#ifndef __BASETREE_H__
#define __BASETREE_H__

#include "basetypes.h"
#include "basesync.h"


///////////////////////////////////////////////////////////////////////////////
//## TODO: combine interfaces
//## TODO: add b+tree/others

NAMESPACE_BEGIN(basics)


void test_db(void);

///////////////////////////////////////////////////////////////////////////////
/// db rewrite

///////////////////////////////////////////////////////////////////////////////
/// RedBlack tree with hash and double linked list
class CDBBase : public Mutex
{
protected:
	///////////////////////////////////////////////////////////////////////////
	/// hash and balancing
	#define HASH_SIZE (256+27)	// maybe smaller hash field would be also ok
	/// memory layout (512 arrays with 4096 elements each)
	#define ROOT_FIELD 512
	#define ROOT_SIZE 4096

	///////////////////////////////////////////////////////////////////////////
	/// tree node
	class CDBNode : public noncopyable
	{
	protected:
		///////////////////////////////////////////////////
		/// db memory management
		class CDBMemory
		{
			///////////////////////////////////////////////////
			/// data
			CDBNode* cRoot[ROOT_FIELD];
			CDBNode* cFree;
			size_t cInx;
			size_t cPos;
		public:
			///////////////////////////////////////////////////
			// construct/destruct
			CDBMemory() : cFree(NULL), cInx(0), cPos(ROOT_SIZE)
			{}
			~CDBMemory()
			{
				size_t i;
				for (i=0; i<cInx; ++i)
				{
					if(cRoot[i])
					{
						delete[] cRoot[i];
						cRoot[i]=NULL;
					}
				}
				cInx = 0;
				cPos = ROOT_SIZE;
				return;
			}
			///////////////////////////////////////////////////
			/// request a node
			CDBNode* aquire(void)
			{
				CDBNode* ret;
				if(cFree == NULL)
				{	// reuse some freed node
					ret = cFree;
					cFree = ret->parent;	
				}
				else
				{	// get an unused node from allocation array
					if(cPos >= ROOT_SIZE)
					{	// all used up, need to alloc a new array
						if(cInx>=ROOT_FIELD)
						{	// storage full (2'097'152 objects reached)
							printf("Database: no more memory, %i nodes used", ROOT_FIELD*ROOT_SIZE);
							return NULL;
						}
						cRoot[cInx] = new CDBNode[ROOT_SIZE];
						cInx++;
						cPos = 0;
					}
					ret = &(cRoot[cInx-1][cPos++]);
				}
				return ret;
			}
			///////////////////////////////////////////////////
			/// put a node back
			void release(CDBNode* node)
			{
				if(node->parent)
				{
					printf("DB released node not freed properly");
				}
				else
				{
					node->parent = cFree;
					cFree = node;
				}
			}

		};
		// CDBMemory
		///////////////////////////////////////////////////

		// friends
		friend class CDBMemory;
		// only cdbmemory can create nodes
		CDBNode()	{}
		~CDBNode()	{}
	public:
		///////////////////////////////////////////////////
		// node elements
		CDBNode *parent;
		CDBNode *left;
		CDBNode *right;
		// doubled linked list elements
		CDBNode *next;
		CDBNode *prev;
		// balancing
		char isRed;
		// node data
		void *key;
		void *data;

		///////////////////////////////////////////////////
		// memory pool
		static CDBMemory cMem;
		static CDBNode* aquire()			{ return cMem.aquire(); }
		static void release(CDBNode* p)		{ cMem.release(p); }
	};
	// CDBNode
	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class CIterator;

	///////////////////////////////////////////////////////////////////////////
	// tree data
	CDBNode *ht[HASH_SIZE];	// list of tree roots
	CDBNode *head;			// start of linked list
	CDBNode *tail;			// end of linked list

	CDBNode *cFreeList;		// temp storage of deleted elements
	size_t	cFreeLock;		// lock counter

	size_t	item_count;		// item number in tree

	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CDBBase() : head(NULL), tail(NULL), cFreeList(NULL), cFreeLock(0), item_count(0)
	{
		memset(ht,0,sizeof(ht));
	}
	virtual ~CDBBase()
	{	// calling clear from the derived function is not possible
		// since the pure virtual releasenode/key/data cannot be called 
		// after having been destructed
		ScopeLock sl(*this);
		if(head)
		{
			printf("database: memory leak, call clear from derived class destructor\n");
			clear(false);
		}
	}
	void clear(bool clrdata=true)
	{	// clears everything even if used by iterators
		// handle with care
		ScopeLock sl(*this);

		// clear the freelock
		if(cFreeLock)
		{
			cFreeLock=0;
			freeunlock();
		}

		// erase the hashlist
		memset(this->ht,0,sizeof(this->ht));

		// use linked list to release the nodes
		CDBNode *node;
		while(this->head)
		{
			node = this->head->next;

			if(clrdata)	// release the data of the node
				this->releasenode(*head);

			// clear the links
			this->head->next=NULL;
			this->head->prev=NULL;
			this->head->right=NULL;
			this->head->left=NULL;

			// and release the node
			this->head->parent=NULL;
			CDBNode::release(this->head);
			this->item_count--;

			this->head=node;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// locking
	bool freelock()
	{
		ScopeLock sl(*this);
		cFreeLock++;
		return (cFreeLock!=0);
	}
	bool freeunlock()
	{
		ScopeLock sl(*this);
		if(cFreeLock>0) cFreeLock--;
		if(cFreeLock==0)
		{
			CDBNode *nextelem;
			while(cFreeList)
			{	// elements in freelist are out of the tree
				// but have still the data and are
				// in the linked list
				
				nextelem = cFreeList->parent;
	
				// release the data
				this->releasenode(*cFreeList);
				
				// unlink from liked list
				if (cFreeList->prev)
					cFreeList->prev->next = cFreeList->next;
				else
					this->head = cFreeList->next;
				if (cFreeList->next)
					cFreeList->next->prev = cFreeList->prev;
				else
					this->tail = cFreeList->prev;

				// release the element
				cFreeList->parent=NULL;
				CDBNode::release(cFreeList);

				item_count--;
				cFreeList = nextelem;
			}
		}
		return (cFreeLock==0);
	}
	///////////////////////////////////////////////////////////////////////////
	// access function
	void* search(void *key)
	{
		ScopeLock sl(*this);
		ssize_t c;
		CDBNode *p = ht[hash(key) % HASH_SIZE];
		while(p)
		{
			c = cmp(key, p->key);
			if (c == 0)
				return p->data;
			if (c < 0)
				p = p->left;
			else
				p = p->right;
		}
		return NULL;
	}
	bool insert(void* key, void* data)
	{
		ScopeLock sl(*this);
		ssize_t c = 0;
		CDBNode *&root = this->ht[this->hash(key) % HASH_SIZE];
		CDBNode *priv = NULL;
		CDBNode *p = root;
		// find node and position for insertion (priv and c)
		while(p)
		{
			priv=p;
			c=this->cmp(key,p->key);
			if(c==0)
			{	// found it directly
				// release the old data
				CDBNode::release(p);
				// insert the new data
				p->data=data;
				p->key=key;
				return (p!=NULL);
			}
			else if(c<0)
				p=p->left;
			else
				p=p->right;
		}
		// get a new node
		p = CDBNode::aquire();
		if(p)
		{	// set the data
			p->parent= NULL;
			p->left  = NULL;
			p->right = NULL;
			p->key   = key;
			p->data  = data;
			p->isRed = true;
			p->prev = NULL;
			p->next = NULL;

			// set double linked list with order of insertion
			if(this->head == NULL)
				this->head = this->tail = p;
			else
			{
				p->prev = this->tail;
				this->tail->next = p;
				this->tail = p;
			}

			// insert to tree
			if(!root)
			{	// root is empty
				root = p;
				p->isRed = false;
			}
			else
			{
				if(c<0)
				{	// insert as left node
					priv->left = p;
					p->parent=priv;
				}
				else
				{	// insert as right node
					priv->right = p;
					p->parent=priv;
				}
				if(priv->isRed)
				{	// must rebalance
					rebalance(p, root);
				}
			}
			item_count++;
		}
		return (p!=NULL);
	}
	bool erase(void* key)
	{
		ScopeLock sl(*this);
		ssize_t c;
		CDBNode *&root = ht[hash(key) % HASH_SIZE];
		CDBNode *p = root;
		// find the node
		while(p)
		{
			c=this->cmp(key,p->key);
			if(c==0)
				break;
			if(c<0)
				p=p->left;
			else
				p=p->right;
		}
		return erase(p, root);
	}
	bool erase(CDBNode* node, CDBNode *&root)
	{
		if(node)
		{
			// remove the node from the tree
			this->rebalance_erase(node, root);
			// self reference is marking deleted nodes
			node->left = node;
			// and add it to freelist
			node->parent=cFreeList;
			this->cFreeList = node;
			// if not locked, call freeunlock to delete the queued data
			// (note: freeunlock does not count below zero, so this is safe)
			if(0==cFreeLock) freeunlock();
		}
		return (node!=NULL);
	}
	void rotate_left(CDBNode *p, CDBNode* &root)
	{
		if(p->right)
		{
			CDBNode* y = p->right;
			p->right = y->left;
			if (y->left !=0)
				y->left->parent = p;
			y->parent = p->parent;

			if (p == root)
				root = y;
			else if (p == p->parent->left)
				p->parent->left = y;
			else
				p->parent->right = y;
			y->left = p;
			p->parent = y;
		}
	}
	void rotate_right(CDBNode *p, CDBNode *&root)
	{
		if(p->left)
		{
			CDBNode* y = p->left;
			p->left = y->right;
			if (y->right != 0)
				y->right->parent = p;
			y->parent = p->parent;

			if (p == root)
				root = y;
			else if (p == p->parent->right)
				p->parent->right = y;
			else
				p->parent->left = y;
			y->right = p;
			p->parent = y;
		}
	}
	void rebalance(CDBNode *p, CDBNode *&root)
	{
		CDBNode *y;
		p->isRed = true;
		while(p!=root && p->parent->isRed)
		{	// rootは必ず黒で親は赤いので親の親は必ず存在する
			if (p->parent == p->parent->parent->left)
			{
				y = p->parent->parent->right;
				if( y && y->isRed)
				{
					p->parent->isRed = false;
					y->isRed = false;
					p->parent->parent->isRed = true;
					p = p->parent->parent;
				}
				else
				{
					if (p == p->parent->right)
					{
						p = p->parent;
						rotate_left(p, root);
					}
					p->parent->isRed = false;
					p->parent->parent->isRed = true;
					rotate_right(p->parent->parent, root);
				}
			}
			else
			{
				y = p->parent->parent->left;
				if (y && y->isRed)
				{
					p->parent->isRed = false;
					y->isRed = false;
					p->parent->parent->isRed = true;
					p = p->parent->parent;
				}
				else
				{
					if (p == p->parent->left)
					{
						p = p->parent;
						rotate_right(p, root);
					}
					p->parent->isRed = false;
					p->parent->parent->isRed = true;
					rotate_left(p->parent->parent, root);
				}
			}
		}
		root->isRed=false;
	}
	void rebalance_erase(CDBNode *z, CDBNode *&root)
	{
		if(z && root)
		{
			CDBNode *y = z;
			CDBNode *x = NULL;
			CDBNode *x_parent = NULL;

			if (y->left == NULL)
				x = y->right;
			else if (y->right == NULL)
				x = y->left;
			else 
			{	// left and right are used
				// find the following node (leftmost starting from the right)
				y = y->right;
				while (y->left != NULL)
					y = y->left;
				x = y->right;
			}
			if (y != z)
			{	// 左右が両方埋まっていた時 yをzの位置に持ってきてzを浮かせる
				z->left->parent = y;
				y->left = z->left;
				if (y != z->right)
				{
					x_parent = y->parent;
					if (x) x->parent = y->parent;
					y->parent->left = x;
					y->right = z->right;
					z->right->parent = y;
				} else
					x_parent = y;
				if(root == z)
					root = y;
				else if (z->parent->left == z)
					z->parent->left = y;
				else
					z->parent->right = y;
				y->parent = z->parent;
				{	// swap colors
					swap(y->isRed, z->isRed);
				}
				y = z;
			}
			else
			{	// どちらか空いていた場合 xをzの位置に持ってきてzを浮かせる
				x_parent = y->parent;
				if(x) x->parent = y->parent;
				if(root == z)
					root = x;
				else if (z->parent->left == z)
					z->parent->left = x;
				else
					z->parent->right = x;
			}
			// ここまで色の移動の除いて通常の2分木と同じ
			if(!y->isRed)
			{	// 赤が消える分には影響無し
				while( x!=root && (x==NULL || !x->isRed) )
				{
					if(x == x_parent->left)
					{
						CDBNode* w = x_parent->right;
						if (w->isRed)
						{
							w->isRed = false;
							x_parent->isRed = true;
							rotate_left(x_parent, root);
							w = x_parent->right;
						}
						if( (w->left == NULL  || !w->left->isRed) &&
							(w->right == NULL || !w->right->isRed) )
						{
							w->isRed = true;
							x = x_parent;
							x_parent = x_parent->parent;
						}
						else
						{
							if (w->right == NULL || !w->right->isRed)
							{
								if (w->left)
									w->left->isRed = false;
								w->isRed = true;
								rotate_right(w, root);
								w = x_parent->right;
							}
							w->isRed = x_parent->isRed;
							x_parent->isRed = false;
							if (w->right)
								w->right->isRed = false;
							rotate_left(x_parent, root);
							break;
						}
					}
					else
					{	// same as above, with right <-> left.
						CDBNode* w = x_parent->left;
						if( w->isRed)
						{
							w->isRed = false;
							x_parent->isRed = true;
							rotate_right(x_parent, root);
							w = x_parent->left;
						}
						if( (w->right == NULL || !w->right->isRed) &&
							(w->left == NULL  || !w->left->isRed) )
						{
							w->isRed = true;
							x = x_parent;
							x_parent = x_parent->parent;
						}
						else
						{
							if( w->left == NULL || !w->left->isRed )
							{
								if (w->right)
									w->right->isRed = false;
								w->isRed = true;
								rotate_left(w, root);
								w = x_parent->left;
							}
							w->isRed = x_parent->isRed;
							x_parent->isRed = true;
							if (w->left)
								w->left->isRed = true;
							rotate_right(x_parent, root);
							break;
						}
					}
				}
				if (x) x->isRed = false;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// virtual interface for derived classes
	virtual ssize_t cmp(void*k1, void*k2) = 0;		// compares two keys
	virtual size_t hash(void*k) = 0;				// calculate hash
	virtual void releasenode(CDBNode& node) = 0;	// release key/data of a node
public:

};

///////////////////////////////////////////////////////////////////////////////
/// iterator for traversing through the data
/// deletion safe and iteration safe 
class CIterator
{
	CDBBase*			table;
	CDBBase::CDBNode*	curr;
public:
	CIterator(const CIterator& iter) : table(iter.table), curr(iter.curr)
	{
		if(table)
			table->freelock();
	}
	const CIterator& operator=(const CIterator& iter)
	{
		if(table)
			table->freeunlock();
		table = iter.table;
		curr  = iter.curr;
		if(table)
			table->freelock();
		return *this;
	}
	CIterator(CDBBase& t)
	{
		table = &t;
		table->freelock();
		curr = table->head;
	}
	const CIterator& operator=(CDBBase& t)
	{
		if(table)
			table->freeunlock();
		t.freelock();
		table = &t;
		curr  = t.head;
		return *this;
	}
	~CIterator()
	{
		if(table)
			table->freeunlock();
	}
	bool operator==(const CDBBase& t) const	{ return table==&t; }

	bool first()			{ curr=(table) ? table->head : NULL; return NULL!=curr; }
	bool last()				{ curr=(table) ? table->tail : NULL; return NULL!=curr; }
	CIterator  operator++(int)	{ CIterator temp(*this); this->next(); return temp; }
	CIterator& operator++()		{ this->next(); return *this; }
	CIterator  operator--(int)	{ CIterator temp(*this); this->prev(); return temp; }
	CIterator& operator--()		{ this->prev(); return *this; }
	bool next()
	{
		if(curr)
		{	// goto next node
			do
			{
				curr=curr->next;
			} while(curr && curr==curr->left);
			// but skip deleted nodes
			// self reference is marking deleted nodes
		}
		return NULL!=curr; 
	}
	bool prev()
	{
		if(curr)
		{	// goto next node
			do
			{
				curr=curr->prev;
			} while(curr && curr==curr->left);
			// but skip deleted nodes
			// self reference is marking deleted nodes
		}
		return NULL!=curr; 
	}

	operator const bool() const { return NULL!=curr; }
	bool isValid() const		{ return NULL!=curr; }

	void* key() const			{ return ((curr) ? curr->key  : NULL); }
	void* data() const			{ return ((curr) ? curr->data : NULL); }

	bool erase()
	{
		if( table && curr && curr!=curr->left)
			return table->erase(curr->key);
		return false;
	}

};


///////////////////////////////////////////////////////////////////////////////
/// fully inlined type checking db template
/// can be used for automatic object deletion
/// only types with max sizeof(size_t) are usable
template<class K, class T> class CDB : public CDBBase
{
	bool ownkey;
	bool owndata;
protected:
	virtual void releasenode(CDBNode& node)
	{
		if(ownkey)
		{
			this->releasekey((K)((size_t)node.key));
			node.key=0;
		}
		if(owndata)
		{
			this->releasedata((T)node.data);
			node.data=0;
		}
	}
	virtual void releasekey(K key)=0;
	virtual void releasedata(T data)=0;
public:
	CDB(bool ok, bool od) : ownkey(ok), owndata(od)	{}
	virtual ~CDB()	{ this->clear(); }

	T search(K key)				{ return (T)CDBBase::search((void*)key); }
	bool insert(K key, T data)	{ return CDBBase::insert((void*)key,(void*)data); }
	bool erase(K key)			{ return CDBBase::erase((void*)key); }

	K key(CIterator& iter)		{ return (K)((iter==*this)?iter.key():NULL); }
	T data(CIterator& iter)		{ return (T)((iter==*this)?iter.data():NULL); }
};


///////////////////////////////////////////////////////////////////////////////
/// db template with string keys
template<class T> class CStrDB : public CDB<char*, T>
{
protected:
	size_t maxlen;
	virtual ssize_t cmp(void*k1, void*k2)		// compares two keys
	{
		if(maxlen)
			return strncmp((const char*)k1,(const char*)k2,maxlen);
		return strcmp((const char*)k1,(const char*)k2);
	}
	virtual size_t hash(void*k)					// calculate hash
	{	//## check for faster string hash
		size_t h = 0;
		size_t i = (maxlen)?maxlen+1:~0;
		unsigned char *p = (unsigned char*)k;
		while (*p && --i)
			h = (h*33 + *p++) ^ (h>>24);
		return h;
	}
public:
	CStrDB(bool ownkey, bool owndata, size_t max=0) : CDB<char*, T>(ownkey, owndata), maxlen(max)	{}
	virtual ~CStrDB()	{}

	virtual void releasekey(char* key)
	{
		delete[] key;
	}
	virtual void releasedata(T data)
	{
		delete data;
	}
};
///////////////////////////////////////////////////////////////////////////////
/// db template with number keys
template<class T> class CNumDB : public CDB<ssize_t, T>
{
protected:
	virtual ssize_t cmp(void*k1, void*k2)		// compares two keys
	{	
		return ((size_t)k1)-((size_t)k2);
	}
	virtual size_t hash(void*k)					// calculate hash
	{
		return ((size_t)k);
	}
public:
	CNumDB(bool owndata) : CDB<ssize_t, T>(false, owndata)	{}
	virtual ~CNumDB()	{}

	virtual void releasekey(ssize_t key)	{}
	virtual void releasedata(T data)
	{
		delete data;
	}

};














///////////////////////////////////////////////////////////////////////////////
/// AVL tree
/// derived from http://meshula.artistnation.com
/// slightly better balancing then RB trees on the expence of extra data
/// not much tested though
///////////////////////////////////////////////////////////////////////////////
template<class K, class T> class TAVLTree
{
protected:
	///////////////////////////////////////////////////////////////////////////////
	// internal declarations
	class TAVLNode
	{
		/////////////////////////////////////////////////////////////
		// friends
		friend class TAVLTree<K,T>;
		/////////////////////////////////////////////////////////////
		// data
		TAVLNode*	cLeft;
		TAVLNode*	cRight;
		char		cBalance;
		uchar		cDepth;
		K			cKey;
		T			cData;
		/////////////////////////////////////////////////////////////
		// constructor
		TAVLNode(K key, T item) :
			cBalance(0), cDepth(0),
			cKey(key), cData(item),
			cLeft(0), cRight(0)
		{  }
	};
public:
	///////////////////////////////////////////////////////////////////////////
	/// iterator
	class Iterator
	{
	protected:
		/////////////////////////////////////////////////////////////
		// Data
		TPtrCount< TAVLNode >	cRoot;
		TAVLNode*				cStack[64];
		short					cInx;
		TAVLNode*				cNode;
	public:
		/////////////////////////////////////////////////////////////
		// Construction
		Iterator(const Iterator& p)
			:  cRoot(p.cRoot), cNode(p.cNode), cInx(p.cInx)
		{
			for (int i = 0; i < cInx; ++i)
				cStack[i] = p.cStack[i];
		}
		Iterator(TAVLTree<K, T>& p) : cRoot(p.cRoot)
		{
			K key;
			T item;
			GetFirst(key, item);
		}
		/////////////////////////////////////////////////////////////
		// Destruction
		~Iterator() { }

		bool getCurrent(K& key, T& item) const
		{
			if (cNode)
			{
				key = cNode->cKey;
				item = cNode->cData;
				return true;
			}
			else
				return false;
		}

		/// returns false if the tree is empty
		bool getFirst(K& key, T& item)
		{
			cInx = 0;
			if( !cRoot )
			{
				return false;
			}
			else
			{
				TAVLNode* pCurr = cRoot;
				TAVLNode* pPrev;
				while (pCurr)
				{
					pPrev = pCurr;
					if( pCurr->cLeft )
						cStack[cInx++] = pCurr;
					pCurr = pCurr->cLeft;
				}
				cNode = pPrev;
				key = cNode->cKey;
				item = cNode->cData;
				return true;
			}
		}

		bool getNext(K& key, T& item)
		{
			if (!cNode)	// already done?
			{
				return false;
			}
			else
			{
				// start looking to the right
				TAVLNode* pCurr = cNode->cRight;	

				while (true)
				{	// this while forces a traversal as far left as possible
					if (pCurr)	
					{	// if we have a pcurr, push it and go left, and repeat.
						cStack[cInx++] = pCurr;
						pCurr = pCurr->cLeft;
					}
					else
					{	// backtrack
						if (cInx > 0)
						{
							TAVLNode* pCandidate = cStack[--cInx];
							// did we backtrack up a right branch?
							if (cNode == pCandidate->cRight)
							{
								// if there is a parent, return the parent.
								if (cInx > 0)
								{
									cNode = cStack[--cInx];
									key = cNode->cKey;
									item = cNode->cData;
									return true;
								}
								else
								{	// if up a right branch, and no parent, traversal finished
									return false;
								}
							}
							else
							{	// up a left branch, done.
								cNode = pCandidate;
								key = cNode->cKey;
								item = cNode->cData;
								return true;
							}
						}
						else
						{	// totally done
							return false;
						}
					}
				}
			}
		}

		bool find(const K& key, T& item)
		{
			TAVLNode* pCurr = cRoot->cRoot;
			cInx = 0;
			while (true)
			{
				TAVLNode* pPushMe = pCurr;
				if( pCurr->cKey == key )	// already done?
				{
					cNode = pCurr;
					item = cNode->cData;
					return true;
				}
					
				if (pCurr->cKey > key)
					pCurr = pCurr->cLeft;
				else
					pCurr = pCurr->cRight;

				if (pCurr)	// maintain the stack so that GetNext will work.
				{
					cStack[cInx++] = pPushMe;
				}
				else	// couldn't find it.
				{
					return false;
				}
			}
			return true;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class TAVLTree<K, T>::Iterator;

private:
	///////////////////////////////////////////////////////////////////////////
	/// Root Node
	TPtrCount< TAVLNode > cRoot;
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	TAVLTree() : cRoot(NULL) { }
	~TAVLTree() { clear(); }
	
	///////////////////////////////////////////////////////////////////////////
	// access
	bool 	insert(const K& key, const T& item);
	bool 	remove(const K& key);
	bool 	find  (const K& key, T& item);
	ushort	getDepth() const { return cRoot ? cRoot->cDepth : 0; }

	bool	clear()	
	{
		return false; 
	}

protected:
	void _insert        (const K& key, const T& item, TAVLNode*& root);
	bool _remove		(TAVLNode*& root, const K& key);
	bool _removeBoth	(TAVLNode*& root, TAVLNode*& curr);
	bool _find			(const K& key, T& item, TAVLNode* root);
	void compute		(TAVLNode*  root);
	void balance        (TAVLNode*& root);
	void balanceRight	(TAVLNode*& root);
	void balanceLeft	(TAVLNode*& root);
	void rotateLeft		(TAVLNode*& root);
	void rotateRight	(TAVLNode*& root);
};

///////////////////////////////////////////////////////////////////////////////
template<class K, class T> inline bool TAVLTree<K,T>::insert(const K& key, const T& item)
{
	if (cRoot == 0)
		cRoot = new TAVLNode(key, item);
	else
		_Insert(key, item, cRoot);
	return true;
}

template<class K, class T> inline void TAVLTree<K, T>::_insert(const K& key, const T& item, TAVLNode*& root)
{
	if (key < root->cKey)
	{
		if (root->cLeft)
			_Insert(key, item, root->cLeft);
		else
			root->cLeft = new TAVLNode(key, item);
	}
	else if (key > root->cKey)
	{
		if (root->cRight)
			_Insert(key, item, root->cRight);
		else
			root->cRight = new TAVLNode(key, item);
	}
	else
	{	// duplicate keys, erplace the former element
		root->cData = item;
	}
	compute(root);
	balance(root);
}

///////////////////////////////////////////////////////////////////////////////
template<class K, class T> inline bool TAVLTree<K, T>::remove(const K& key)
{
	return _remove(this->cRoot, key);
}

template<class K, class T> inline bool TAVLTree<K, T>::_remove(TAVLNode*& root, const K& key)
{
	bool ret = false;
	if (!root)
	{	// just nothing
	}
	else if (root->cKey > key)	
	{	// go to left subtree
		if( _remove(root->cLeft, key) )
		{
			compute(root);
			balanceRight(root);
			return true;
		}
	}
	else if (root->cKey < key) 
	{	// go to right subtree
		if( _remove(root->cRight, key) )
		{
			compute(root);
			balanceLeft(root);
			return true;
		}
	}
	else	
	{	// node found!
		TAVLNode* pMe = root;
		
		if (!root->cRight)
		{
			root = root->cLeft;
			delete pMe;
			return true;
		}
		else if (!root->cLeft)
		{
			root = root->cRight;
			delete pMe;
			return true;
		}
		else
		{
			if( _removeBoth(root, root->cLeft) )
			{
				compute(root);
				balance(root);
			}
			return true;
		}
	}
	return false;
}

template<class K, class T> inline bool TAVLTree<K, T>::_removeBoth(TAVLNode*& root, TAVLNode*& curr)
{
	if (!curr->cRight)
	{
		root->cKey = curr->cKey;
		root->cData = curr->cData;
		TAVLNode* pMe = curr;
		curr = curr->cLeft;
		delete pMe;
		return true;
	}
	else
	{
		if( _removeBoth(root, curr->cRight) )
		{
			compute(root);
			balance(root);
			return true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
template<class K, class T> inline bool TAVLTree<K, T>::find(const K& key, T& item)
{
	return _find(key, item, cRoot);
}

template<class K, class T> inline bool TAVLTree<K, T>::_find(const K& key, T& item, TAVLNode* root)
{
	if (root)
	{
		if (root->cKey == key)
		{
			item = root->cData;
			return true;
		}
		else if (key < root->cKey)
			return _find(key, item, root->cLeft);
		else
			return _find(key, item, root->cRight);
	}
	else
		return false;
}


///////////////////////////////////////////////////////////////////////////////
template<class K, class T> inline void TAVLTree<K, T>::compute(TAVLNode* root)
{
	if (root)
	{
		short leftDepth  = root->cLeft  ? root->cLeft->cDepth  : 0;
		short rightDepth = root->cRight ? root->cRight->cDepth : 0;

		root->cDepth = 1 + ((leftDepth > rightDepth) ? leftDepth : rightDepth);
		root->cBalance = rightDepth - leftDepth;
	}
}

template<class K, class T> inline void TAVLTree<K, T>::balance(TAVLNode*& root)
{	// AVL trees have the property that no branch is more than 1 longer than its sibling
	if (root->cBalance > 1)
		balanceRight(root);
	if (root->cBalance < -1)
		balanceLeft(root);
}

template<class K, class T> inline void TAVLTree<K, T>::balanceRight(TAVLNode*& root)
{
	if (root->cRight)
	{
		if (root->cRight->cBalance > 0)
		{
			rotateLeft(root);
		}
		else if (root->cRight->cBalance < 0)
		{
			rotateRight(root->cRight);
			rotateLeft(root);
		}
	}
}

template<class K, class T> inline void TAVLTree<K, T>::balanceLeft(TAVLNode*& root)
{
	if (root->cLeft)
	{
		if (root->cLeft->cBalance < 0)
		{
			rotateRight(root);
		}
		else if (root->cLeft->cBalance > 0)
		{
			rotateLeft(root->cLeft);
			rotateRight(root);
		}
	}
}

template<class K, class T> inline void TAVLTree<K, T>::rotateLeft(TAVLNode*& root)
{
	TAVLNode* pTemp = root;
	root = root->cRight;
	pTemp->cRight = root->cLeft;
	root->cLeft = pTemp;

	compute(root->cLeft);
	compute(root->cRight);
	compute(root);
}

template<class K, class T> inline void TAVLTree<K, T>::rotateRight(TAVLNode*& root)
{
	TAVLNode* pTemp = root;
	root = root->cLeft;
	pTemp->cLeft = root->cRight;
	root->cRight = pTemp;

	compute(root->cLeft);
	compute(root->cRight);
	compute(root);
}











///////////////////////////////////////////////////////////////////////////////
/// RedBlackTree
/// derived from 
/// http://www.oopweb.com/Algorithms/Documents/PLDS210/Volume/red_black.html
/// already added smart pointers but the code needs to be revised generally
/// Requirements:
/// The K class must support:
/// 1. < and == operations.
/// 2. Copy construction.
///
/// The T class must support:
/// 1. Copy construction.
/// 2. Assignment operation (=) if SetValue is used

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// class TRBTree. Holder of the tree structure. 
template <class K, class T> class TRBTree
{
	///////////////////////////////////////////////////////////////////////////
	// Private Classes

	///////////////////////////////////////////////////////////////////////////
	/// class TRBNode is the element type of the TRBTree tree. 
	class TRBNode
	{
	private:
		/////////////////////////////////////////////////////////////
		// privates
		TRBNode();		// default constructor disabled

		/////////////////////////////////////////////////////////////
		// Private Members
		TRBNode*	mLeftChild;
		TRBNode*	mRightChild;
		TRBNode*	mParent;

		K			mKey;
		T			mValue;
		bool		mIsRed;

	public:
		/////////////////////////////////////////////////////////////
		// Constructor/Destructor
		TRBNode(const K& k, const T& v)
			: mLeftChild(0), mRightChild(0), mParent(0),
			  mKey(k),mValue(v),
			  mIsRed(true)
		{}
		~TRBNode() {}

		/////////////////////////////////////////////////////////////
		/// Access
		/// Set*Child also updates the the child's parent attribute.
		void SetLeftChild(TRBNode* p)	{ mLeftChild=p; if (p) p->SetParent(this); }
		void SetRightChild(TRBNode* p)	{ mRightChild=p;if (p) p->SetParent(this); }
		void SetParent(TRBNode* p)		{ mParent=p; }

		void SetValue(const T& v)		{ mValue = v; }
		/// Note: Deliberately no SetKey, that could easily screw up the tree.
		/// If a key shall be changed, delete node from tree and insert a new one instead.

		void SetRed()					{ mIsRed = true; }
		void SetBlack()					{ mIsRed = false; }
		/////////////////////////////////////////////////////////////
		// Queries
		TRBNode* GetLeftChild() const	{ return mLeftChild; }
		TRBNode* GetRightChild() const	{ return mRightChild; }
		TRBNode* GetParent() const		{ return mParent; }

		T GetValue() const				{ return mValue; }
		K GetKey() const				{ return mKey; }
		/////////////////////////////////////////////////////////////
		// Some more or less useful queries
		bool IsRoot() const				{ return mParent==0; }
		bool IsLeftChild() const		{ return mParent!=0 && mParent->GetLeftChild()==this; }
		bool IsRightChild() const		{ return mParent!=0 && mParent->GetRightChild()==this; }
		bool IsLeaf() const				{ return mLeftChild==0 && mRightChild==0; }
		unsigned int GetLevel() const	{ if (IsRoot()) return 1; else return GetParent()->GetLevel() + 1; }
		bool IsRed() const				{ return mIsRed; };
		bool IsBlack() const			{ return !mIsRed; };
	};// TRBNode
	///////////////////////////////////////////////////////////////////////////
public:
	///////////////////////////////////////////////////////////////////////////
	// Public Definitions

	///////////////////////////////////////////////////////////////////////////
	// Public Classes

	///////////////////////////////////////////////////////////////////////////
	/// class TRBTree::Iterator.
	/// Regular low->high (++) and high->low (--) iterator
	class Iterator
	{
		/////////////////////////////////////////////////////////////
		// Private Data
		TPtrCount< TRBNode* >	mRoot;
		TRBNode*				mCur;
	public:
		/////////////////////////////////////////////////////////////
		// Construction
		Iterator() : mRoot(0),mCur(0)						{  }
		Iterator(TRBNode& root):mRoot(root)					{ Reset();}
		Iterator(const TRBTree& tree):mRoot(tree.mRoot)		{ Reset();}
		Iterator(const Iterator& s):mRoot(s.mRoot),mCur(s.mCur){}

		/////////////////////////////////////////////////////////////
		// Reset the iterator
		// atLowest : true  - Reset at lowest key value (and then ++ your way through)
		//            false - Reset at highest key value (and then -- your way through)
		void Reset(bool atLowest=true) 
		{ 
			if (atLowest) 
				mCur = Min(mRoot); 
			else 
				mCur = Max(mRoot);
		}

		/////////////////////////////////////////////////////////////
		// Queries

		// Has the iterator reached the end?
		bool atEnd() const		{ return mCur==NULL; }
		TRBNode* GetNode()		{ return mCur;	}

		/////////////////////////////////////////////////////////////
		// Assignment operator
		Iterator& operator=(const Iterator& src) 
		{ 
			mRoot = src.mRoot; 
			mCur = src.mCur; 
			return (*this);
		}

		/////////////////////////////////////////////////////////////
		// Increment operator
		void operator++(int)	{ Inc(); }
		/////////////////////////////////////////////////////////////
		// Decrement operator
		void operator--(int)	{ Dec(); }

		/////////////////////////////////////////////////////////////
		// Access operators
		TRBNode* operator -> () { return GetNode();	}
		TRBNode& operator*   () 
		{ 
			if (atEnd())
			{
				throw "Iterator at end";			
			}
			return *mCur; 
		}

	private:
		/////////////////////////////////////////////////////////////
		// Private Helper Methods

		/////////////////////////////////////////////////////////////
		// Min: Returns node with lowest key from n and down.
		//      Ie the node all down to the left
		TRBNode* Min(TRBNode* n)
		{
			while(n && n->GetLeftChild())
				n = n->GetLeftChild();
			return n;
		}
		/////////////////////////////////////////////////////////////
		// Max: Returns node with highest key from n and down.
		//      Ie the node all down to the right
		TRBNode* Max(TRBNode* n)
		{
			while(n && n->GetRightChild())
				n = n->GetRightChild();
			return n;
		}

		/////////////////////////////////////////////////////////////
		// ++
		void Inc()
		{
			// Already at end?
			if (mCur==0)
				return;

			if (mCur->GetRightChild())
			{
				// If current node has a right child, the next higher node is the 
				// node with lowest key beneath the right child.
				mCur =  Min(mCur->GetRightChild());
			}
			else if (mCur->IsLeftChild())
			{
				// No right child? Well if current node is a left child then
				// the next higher node is the parent
				mCur = mCur->GetParent();
			}
			else
			{
				// Current node neither is left child nor has a right child.
				// Ie it is either right child or root
				// The next higher node is the parent of the first non-right
				// child (ie either a left child or the root) up in the
				// hierarchy. Root's parent is 0.
				while(mCur->IsRightChild())
					mCur = mCur->GetParent();
				mCur =  mCur->GetParent();
			}
		}
		/////////////////////////////////////////////////////////////
		// --
		void Dec()
		{
			// Already at end?
			if (mCur==0)
				return;

			if (mCur->GetLeftChild())
			{
				// If current node has a left child, the next lower node is the 
				// node with highest key beneath the left child.
				mCur =  Max(mCur->GetLeftChild());
			}
			else if (mCur->IsRightChild())
			{
				// No left child? Well if current node is a right child then
				// the next lower node is the parent
				mCur =  mCur->GetParent();
			}
			else
			{
				// Current node neither is right child nor has a left child.
				// Ie it is either left child or root
				// The next higher node is the parent of the first non-left
				// child (ie either a right child or the root) up in the
				// hierarchy. Root's parent is 0.

				while(mCur->IsLeftChild())
					mCur = mCur->GetParent();
				mCur =  mCur->GetParent();
			}
		}
	}; // Iterator
	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	/// class TRBTree::ParentFirstIterator. 
	/// Traverses the tree from top to bottom. Typical usage is 
	/// when storing the tree structure, because when reading it 
	/// later (and inserting elements) the tree structure will
	/// be the same.
	class ParentFirstIterator
	{
		/////////////////////////////////////////////////////////////
		// Private Data
		TPtrCount< TRBNode* >	mRoot;
		TRBNode*				mCur;
	public:

		/////////////////////////////////////////////////////////////
		// constructor
		ParentFirstIterator():mRoot(0),mCur(0){}
		explicit ParentFirstIterator(TRBNode* root):mRoot(root),mCur(0){ Reset(); }

		/////////////////////////////////////////////////////////////
		// Public Methods
		
		void Reset() { mCur = mRoot; };
		/////////////////////////////////////////////////////////////
		// Has the iterator reached the end?
		bool atEnd() const { return mCur==0; }
		TRBNode* GetNode() { return mCur;	}

		/////////////////////////////////////////////////////////////
		// Assignment operator
		ParentFirstIterator& operator=(const ParentFirstIterator& src) 
		{ 
			mRoot = src.mRoot; mCur = src.mCur; return (*this);
		}

		/////////////////////////////////////////////////////////////
		// Increment operator
		void operator++(int) { Inc(); }

		/////////////////////////////////////////////////////////////
		// Access operators
		TRBNode* operator -> () { return GetNode();	}
		TRBNode& operator*   () 
		{ 
			if (atEnd())
				throw "ParentFirstIterator at end";			
			return *GetNode(); 
		}
	private:
		/////////////////////////////////////////////////////////////
		// Private methodsCommands
		
		void Inc()
		{
			// Already at end?
			if (mCur==0)
				return;

			// First we try down to the left
			if (mCur->GetLeftChild())
			{
				mCur =  mCur->GetLeftChild();
			}
			else if (mCur->GetRightChild())
			{
				// No left child? The we go down to the right.
				mCur = mCur->GetRightChild();
			}
			else
			{
				// No children? Move up in the hierarcy until
				// we either reach 0 (and are finished) or
				// find a right uncle.
				while (mCur!=0)
				{
					// But if parent is left child and has a right "uncle" the parent
					// has already been processed but the uncle hasn't. Move to 
					// the uncle.
					if (mCur->IsLeftChild() && mCur->GetParent()->GetRightChild())
					{
						mCur = mCur->GetParent()->GetRightChild();
						return;
					}
					mCur = mCur->GetParent();
				}
			}
		}
	}; // ParentFirstIterator
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	/// class TRBTree::ParentLastIterator. 
	/// Traverse the tree from bottom to top.
	/// Typical usage is when deleting all elements in the tree
	/// because you must delete the children before you delete
	/// their parent.
	class ParentLastIterator
	{
		/////////////////////////////////////////////////////////////
		// Private Data
		TPtrCount< TRBNode* >	mRoot;
		TRBNode*				mCur;
	public:

		/////////////////////////////////////////////////////////////
		// Construction
		ParentLastIterator():mRoot(0),mCur(0){}
		explicit ParentLastIterator(TRBNode* root):mRoot(root),mCur(0){ Reset();}

		/////////////////////////////////////////////////////////////
		//
		void Reset() { mCur = Min(mRoot); }
		/////////////////////////////////////////////////////////////
		// Has the iterator reached the end?
		bool atEnd() const { return mCur==0; }
		TRBNode* GetNode() { return mCur;	}

		/////////////////////////////////////////////////////////////
		// Assignment operator
		ParentLastIterator& operator=(const ParentLastIterator& src) { mRoot = src.mRoot; mCur = src.mCur; return (*this);}

		/////////////////////////////////////////////////////////////
		// Increment operator
		void operator++(int) { Inc(); }

		/////////////////////////////////////////////////////////////
		// Access operators
		TRBNode* operator -> () { return GetNode();	}
		TRBNode& operator*   () 
		{ 
			if (atEnd())
				throw "ParentLastIterator at end";			
			return *GetNode(); 
		}
	private:
		/////////////////////////////////////////////////////////////
		// Min: Returns the node as far down to the left as possible.
		TRBNode* Min(TRBNode* n)
		{
			while(n!=0 && (n->GetLeftChild()!=0 || n->GetRightChild()!=0))
			{
				if (n->GetLeftChild())
					n = n->GetLeftChild();
				else
					n = n->GetRightChild();
			}
			return n;
		}
		void Inc()
		{
			// Already at end?
			if (mCur==0)
				return;

			// Note: Starting point is the node as far down to the left as possible.

			// If current node has an uncle to the right, go to the
			// node as far down to the left from the uncle as possible
			// else just go up a level to the parent.
			if (mCur->IsLeftChild() && mCur->GetParent()->GetRightChild())
			{
				mCur = Min(mCur->GetParent()->GetRightChild());
			}
			else
				mCur = mCur->GetParent();
		}
	}; // ParentLastIterator
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	/// ByLevelIterator. Traverse tree top to bottom, level by level left to right.
	/// Typical usage : I don't know. Perhaps if the tree should be displayed               
	/// in some fancy way.
	/// It is the most memory consuming of all iterators as it will allocate an 
	/// array of (Size()+1)/2 Nodes.
	/// Impl. desc:
	///   mArray[0] is the current node we're looking at, initially set to mRoot.
	///   When ++:ing the children of mArray[0] (if any) are put last in the 
	///   array and the array is shifted down 1 step
	class ByLevelIterator
	{
		/////////////////////////////////////////////////////////////
		// Private Data
		TPtrCount< TRBNode* >	mRoot;
		TRBNode** mArray;
		unsigned int mSize;
		unsigned int mEndPos;
	public:
		//------------------------------
		// Public Construction
		//------------------------------

		// Default constructor
		ByLevelIterator():mRoot(0),mArray(0),mSize(0){}

		// Constructor(treeRoot, elementsInTree)
		ByLevelIterator(TRBNode* root, unsigned int size):mRoot(root),mSize(size),mArray(0){ Reset();}

		// Copy constructor
		ByLevelIterator(const ByLevelIterator& src):mRoot(src.mRoot),mSize(src.mSize),mEndPos(src.mEndPos)
		{
			if (src.mArray!=0)
			{
				mArray = new TRBNode*[(mSize+1)/2];
				memcpy(mArray,src.mArray,sizeof(TRBNode*)*(mSize+1)/2);
			}
			else
				mArray = 0;
		}

		// Destructor
		~ByLevelIterator() 
		{ 
			if (mArray!=0)
			{
				delete [] mArray;
				mArray = 0;
			}
		}

		//------------------------------
		// Public Commands
		//------------------------------
		void Reset() 
		{ 
			if (mSize>0)
			{
				// Only allocate the first time Reset is called
				if (mArray==0)
				{
					// mArray must be able to hold the maximum "width" of the tree which
					// at most can be (NumberOfNodesInTree + 1 ) / 2
					mArray = new TRBNode*[(mSize+1)/2];
				}
				// Initialize the array with 1 element, the mRoot.
				mArray[0] = mRoot;
				mEndPos = 1;
			}
			else 
				mEndPos=0;
		} // Reset

		//------------------------------
		// Public Queries
		//------------------------------

		// Has the iterator reached the end?
		bool atEnd() const { return mEndPos == 0; }
		TRBNode* GetNode() { return mArray[0];	}

		//------------------------------
		// Public Operators
		//------------------------------

		// Assignment Operator
		ByLevelIterator& operator=(const ByLevelIterator& src) 
		{ 
			mRoot = src.mRoot; 
			mSize = src.mSize;
			if (src.mArray!=0)
			{
				mArray = new TRBNode*[(mSize+1)/2];
				memcpy(mArray,src.mArray,sizeof(TRBNode*)*(mSize+1)/2);
			}
			else
				mArray = 0;

			return (*this);
		}

		// Increment operator
		void operator++(int) { Inc(); }

		// Access operators
		TRBNode* operator -> () { return GetNode(); }
		TRBNode& operator*   () 
		{ 
			if (atEnd())
				throw "ParentLastIterator at end";			
			return *GetNode(); 
		}
	private:

		//------------------------------
		// Private Commands
		//------------------------------

		void Inc()
		{
			if (mEndPos == 0)
				return;

			// Current node is mArray[0]
			TRBNode* pNode = mArray[0];

			// Move the array down one notch, ie we have a new current node 
			// (the one than just was mArray[1])
			unsigned int i;
			for(i=0; i<mEndPos; ++i)
			{
				mArray[i] = mArray[i+1];
			}
			mEndPos--;

			TRBNode* pChild=pNode->GetLeftChild();
			if (pChild) // Append the left child of the former current node
			{ 
				mArray[mEndPos] = pChild;
				mEndPos++;
			}

			pChild = pNode->GetRightChild();
			if (pChild) // Append the right child of the former current node
			{ 
				mArray[mEndPos] = pChild;
				mEndPos++;
			}

		}
	}; // ByLevelIterator
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	/// AccessClass is a temparoary class used with the [] operator.
	/// It makes it possible to have different behavior in situations like:
	/// myTree["Foo"] = 32; 
	///   If "Foo" already exist, just update its value else insert a new 
	///   element.
	/// int i = myTree["Foo"]
	/// If "Foo" exists return its value, else throw an exception.
	/// 
	class AccessClass
	{
		// Let TRBTree be the only one who can instantiate this class.
		friend class TRBTree<K, T>;
		//------------------------------
		// Private Members
		//------------------------------
		TRBTree& mTree;
		const K& mKey;
	public:

		// Assignment operator. Handles the myTree["Foo"] = 32; situation
		const T& operator=(const T& value)
		{	// Just use the Set method, it handles already exist/not exist situation
			mTree.Set(mKey, value);
			return value;
		}

		// T operator
		operator T()
		{
			TRBNode* node = mTree.Find(mKey);

			// Not found
			if (node==0)
			{
				throw "Item not found";
			}

			return node->GetValue();
		}
	private:
		AccessClass(TRBTree& tree, const K& key):mTree(tree),mKey(key){}
		AccessClass();
	}; // AccessClass
	///////////////////////////////////////////////////////////////////////////



private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data
	TPtrCount< TRBNode* >	mRoot;	// The top node. 0 if empty.	
	unsigned int			mSize;	// Number of nodes in the tree
public:
	///////////////////////////////////////////////////////////////////////////
	// Constructor/Destructor
	TRBTree():mRoot(NULL),mSize(0){}
	~TRBTree(){ DeleteAll(); }

	///////////////////////////////////////////////////////////////////////////
	bool Insert(const K& keyNew, const T& v)
	{
		// First insert node the "usual" way (no fancy balance logic yet)
		TRBNode* newNode = new TRBNode(keyNew,v);
		if (!Insert(newNode))
		{
			delete newNode;
			return false;
		}

		// Then attend a balancing party
		while  (!newNode->IsRoot() && (newNode->GetParent()->IsRed()))
		{
			if ( newNode->GetParent()->IsLeftChild()) 
			{
				// If newNode is a left child, get its right 'uncle'
				TRBNode* newNodesUncle = newNode->GetParent()->GetParent()->GetRightChild();
				if ( newNodesUncle!=0 && newNodesUncle->IsRed())
				{
					// case 1 - change the colours
					newNode->GetParent()->SetBlack();
					newNodesUncle->SetBlack();
					newNode->GetParent()->GetParent()->SetRed();
					// Move newNode up the tree
					newNode = newNode->GetParent()->GetParent();
				}
				else 
				{
					// newNodesUncle is a black node
					if ( newNode->IsRightChild()) 
					{
						// and newNode is to the right
						// case 2 - move newNode up and rotate
						newNode = newNode->GetParent();
						RotateLeft(newNode);
					}
					// case 3
					newNode->GetParent()->SetBlack();
					newNode->GetParent()->GetParent()->SetRed();
					RotateRight(newNode->GetParent()->GetParent());
				}
			}
			else 
			{
				// If newNode is a right child, get its left 'uncle'
				TRBNode* newNodesUncle = newNode->GetParent()->GetParent()->GetLeftChild();
				if ( newNodesUncle!=0 && newNodesUncle->IsRed())
				{
					// case 1 - change the colours
					newNode->GetParent()->SetBlack();
					newNodesUncle->SetBlack();
					newNode->GetParent()->GetParent()->SetRed();
					// Move newNode up the tree
					newNode = newNode->GetParent()->GetParent();
				}
				else 
				{
					// newNodesUncle is a black node
					if ( newNode->IsLeftChild()) 
					{
						// and newNode is to the left
						// case 2 - move newNode up and rotate
						newNode = newNode->GetParent();
						RotateRight(newNode);
					}
					// case 3
					newNode->GetParent()->SetBlack();
					newNode->GetParent()->GetParent()->SetRed();
					RotateLeft(newNode->GetParent()->GetParent());
				}

			}
		}
		// Color the root black
		mRoot->SetBlack();
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Set. If the key already exist just replace the value
	/// else insert a new element.
	void Set(const K& k, const T& v)
	{
		TRBNode* p = Find(k);
		if (p)
			p->SetValue(v);
		else
			Insert(k,v);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Remove a node. Return true if the node could
	/// be found (and was removed) in the tree.
	bool Delete(const K& k)
	{
		TRBNode* p = Find(k);
		if (p == NULL) return false;

		// Rotate p down to the left until it has no right child, will get there
		// sooner or later.
		while(p->GetRightChild())
		{
			// "Pull up my right child and let it knock me down to the left"
			RotateLeft(p);
		}
		// p now has no right child but might have a left child
		TRBNode* left = p->GetLeftChild(); 

		// Let p's parent point to p's child instead of point to p
		if (p->IsLeftChild())
		{
			p->GetParent()->SetLeftChild(left);
		}
		else if (p->IsRightChild())
		{
			p->GetParent()->SetRightChild(left);
		}
		else
		{
			// p has no parent => p is the root. 
			// Let the left child be the new root.
			SetRoot(left);
		}

		// p is now gone from the tree in the sense that 
		// no one is pointing at it. Let's get rid of it.
		delete p;
			
		mSize--;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Wipe out the entire tree.
	void DeleteAll()
	{
		ParentLastIterator i(GetParentLastIterator());

		while(!i.atEnd())
		{
			TRBNode* p = i.GetNode();
			i++; // Increment it before it is deleted
			     // else iterator will get quite confused.
			delete p;
		}
		mRoot = 0;
		mSize= 0;
	}


	///////////////////////////////////////////////////////////////////////////
	/// Is the tree empty?
	bool IsEmpty() const { return mRoot == 0; }

	///////////////////////////////////////////////////////////////////////////
	/// Search for the node.
	/// Returns 0 if node couldn't be found.
	TRBNode* Find(const K& keyToFind) const
	{
		TRBNode* pNode = mRoot;

		while(pNode!=0)
		{
			K key(pNode->GetKey());

			if (keyToFind == key)
			{
				// Found it! Return it! Wheee!
				return pNode;
			}			
			else if (keyToFind < key)
			{
				pNode = pNode->GetLeftChild();
			}
			else //keyToFind > key
			{
				pNode = pNode->GetRightChild();
			}
		}

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Get the root element. 0 if tree is empty.
	TRBNode* GetRoot() const { return mRoot; }

	///////////////////////////////////////////////////////////////////////////
	/// Number of nodes in the tree.
	unsigned int Size() const { return mSize; }


	///////////////////////////////////////////////////////////////////////////
	/// Public Iterators
	Iterator GetIterator()			 
	{ 
		Iterator it(GetRoot());
		return it; 
	}
	ParentFirstIterator GetParentFirstIterator() 
	{
		ParentFirstIterator it(GetRoot());
		return it; 
	}
	ParentLastIterator GetParentLastIterator()
	{   
		ParentLastIterator it(GetRoot());
		return it;	
	}
	ByLevelIterator GetByLevelIterator()	 
	{ 
		ByLevelIterator it(GetRoot(),Size());
		return it;	
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// operator [] for accesss to elements
	AccessClass operator[](const K& k) 
	{
		return AccessClass(*this, k);
	}
private:


	///////////////////////////////////////////////////////////////////////////
	/// Private Commands
	void SetRoot(TRBNode* newRoot)
	{
		mRoot = newRoot;
		if (mRoot!=0)
			mRoot->SetParent(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Insert a node into the tree without using any fancy balancing logic.
	/// Returns false if that key already exist in the tree.
	bool Insert(TRBNode* newNode)
	{
		bool result=true; // Assume success

		if (mRoot==0)
		{
			SetRoot(newNode);
			mSize = 1;
		}
		else
		{
			TRBNode* pNode = mRoot;
			K keyNew = newNode->GetKey();
			while (pNode)
			{
				K key(pNode->GetKey());

				if (keyNew == key)
				{
					result = false;
					pNode = 0;
				} 
				else if (keyNew < key)
				{
					if (pNode->GetLeftChild()==0)
					{
						pNode->SetLeftChild(newNode);
						pNode = 0;
					}
					else
					{
						pNode = pNode->GetLeftChild();
					}
				} 
				else 
				{
					// keyNew > key
					if (pNode->GetRightChild()==0)
					{
						pNode->SetRightChild(newNode);
						pNode = 0;
					}
					else
					{
						pNode = pNode->GetRightChild();
					}
				}
			}

			if (result)
			{
				mSize++;
			}
		}
		
		return result;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Rotate left.
	/// Pull up node's right child and let it knock node down to the left
	void RotateLeft(TRBNode* p)
	{		
		TRBNode* right = p->GetRightChild();

		p->SetRightChild(right->GetLeftChild());
		
		if (p->IsLeftChild())
			p->GetParent()->SetLeftChild(right);
		else if (p->IsRightChild())
			p->GetParent()->SetRightChild(right);
		else
		{
			SetRoot(right);
		}
		right->SetLeftChild(p);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Rotate right.
	/// Pull up node's left child and let it knock node down to the right
	void RotateRight(TRBNode* p)
	{		

		TRBNode* left = p->GetLeftChild();

		p->SetLeftChild(left->GetRightChild());
		
		if (p->IsLeftChild())
			p->GetParent()->SetLeftChild(left);
		else if (p->IsRightChild())
			p->GetParent()->SetRightChild(left);
		else
		{
			SetRoot(left);
		}
		left->SetRightChild(p);
	}
};


NAMESPACE_END(basics)



#endif//__BASETREE_H__

