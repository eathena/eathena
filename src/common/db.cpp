// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder



#include "db.h"
#include "utils.h"
#include "showmsg.h"


#define MALLOC_DBN


#ifdef MALLOC_DBN
#define ROOT_SIZE 4096
static struct dbn *dbn_root[512], *dbn_free;
static size_t dbn_root_inx = 0;
static size_t dbn_root_pos = ROOT_SIZE;


struct dbn* malloc_dbn (void)
{
	struct dbn* ret;

	if (dbn_free == NULL)
	{
		if(dbn_root_pos >= ROOT_SIZE)
		{
			dbn_root[dbn_root_inx] = new struct dbn[ROOT_SIZE];
			memset(dbn_root[dbn_root_inx],0, sizeof(struct dbn)*ROOT_SIZE);
			dbn_root_inx++;
			dbn_root_pos = 0;
		}
		return &(dbn_root[dbn_root_inx-1][dbn_root_pos++]);
	}
	ret = dbn_free;
	dbn_free = ret->parent;
	memset(ret,0,sizeof(struct dbn));
	return ret;
}

void free_dbn (struct dbn *add_dbn)
{
	add_dbn->parent = dbn_free;
	dbn_free = add_dbn;
}

void exit_dbn (void)
{
	size_t i;
	for (i = 0; i < dbn_root_inx; ++i)
		if (dbn_root[i])
		{
			delete[] dbn_root[i];
			dbn_root[i]=NULL;
		}
	dbn_root_pos = dbn_root_inx = 0;
	return;
}
#else

inline struct dbn* malloc_dbn (void)
{
	struct dbn x= new struct dbn;
	memset(x,0, sizeof(struct dbn));
	return x;
}

inline void free_dbn (struct dbn *add_dbn)
{
	delete p;
}

inline void exit_dbn(void)
{  }
#endif


ssize_t strdb_cmp (struct dbt* table, void* a, void* b)
{
	if(table->maxlen)
		return strncmp((char*)a,(char*)b,table->maxlen);
	return strcmp((char*)a,(char*)b);
}


size_t strdb_hash (struct dbt* table, void* a)
{
	size_t h = 0;
	unsigned char *p = (unsigned char*)a;
	int i = table->maxlen;

	if (i == 0) i = 0x7fffffff;
	while (*p && --i >= 0)
		h = (h*33 + *p++) ^ (h>>24);

	return h;
}

struct dbt* strdb_init_ (int maxlen, const char *file, int line)
{
	int i;
	struct dbt* table = new struct dbt;
	memset(table,0,sizeof(struct dbt));

	table->cmp = strdb_cmp;
	table->hashfunc = strdb_hash;
	table->maxlen = maxlen;
	for (i = 0; i < HASH_SIZE; ++i)
		table->ht[i] = NULL;
	table->alloc_file = file;
	table->alloc_line = line;
	table->item_count = 0;

	return table;
}

ssize_t numdb_cmp (struct dbt *table, void *a, void *b)
{
	size_t ia, ib;
	ia = (size_t)a;
	ib = (size_t)b;
	return ia - ib;
}

size_t numdb_hash (struct dbt *table, void *a)
{
	return (size_t)a;
}

struct dbt* numdb_init_(const char *file, int line)
{
	int i;
	struct dbt* table = new struct dbt;
	memset(table,0,sizeof(struct dbt));

	table->cmp=numdb_cmp;
	table->hashfunc=numdb_hash;
	table->maxlen=sizeof(size_t);
	for(i=0;i<HASH_SIZE;++i)
		table->ht[i]=NULL;
	table->alloc_file = file;
	table->alloc_line = line;
	table->item_count = 0;
	return table;
}

void* db_search (struct dbt *table, void  *key)
{
	struct dbn *p = table->ht[table->hashfunc(table,key) % HASH_SIZE];

	while (p) {
		int c = table->cmp(table, key, p->key);
		if (c == 0)
			return p->data;
		if (c < 0)
			p = p->left;
		else
			p = p->right;
	}
	return NULL;
}

void* db_search2 (struct dbt *table, const char *key)
{
	int i, sp;
	struct dbn *p, *pn, *stack[64];
	size_t slen = strlen(key);

	for (i = 0; i < HASH_SIZE; ++i) {
		if ((p = table->ht[i]) == NULL)
			continue;
		sp=0;
		while(1){
			if (strncasecmp(key, (char*)p->key, slen) == 0)
				return p->data;
			if((pn=p->left)!=NULL){
				if (p->right)
					stack[sp++] = p->right;
				p = pn;
			} else {
				if (p->right)
					p = p->right;
				else {
					if (sp == 0)
						break;
					p = stack[--sp];
				}
			}
		}
	}
	return 0;
}

void db_rotate_left (struct dbn *p, struct dbn **root)
{
	struct dbn * y = p->right;
	p->right = y->left;
	if (y->left !=0)
		y->left->parent = p;
	y->parent = p->parent;

	if (p == *root)
		*root = y;
	else if (p == p->parent->left)
		p->parent->left = y;
	else
		p->parent->right = y;
	y->left = p;
	p->parent = y;
}

void db_rotate_right(struct dbn *p,struct dbn **root)
{
	struct dbn * y = p->left;
	p->left = y->right;
	if (y->right != 0)
		y->right->parent = p;
	y->parent = p->parent;

	if (p == *root)
		*root = y;
	else if (p == p->parent->right)
		p->parent->right = y;
	else
		p->parent->left = y;
	y->right = p;
	p->parent = y;
}

void db_rebalance(struct dbn *p,struct dbn **root)
{
	p->color = RED;
	while(p!=*root && p->parent->color==RED){ // rootは必ず黒で親は赤いので親の親は必ず存在する
		if (p->parent == p->parent->parent->left) {
			struct dbn *y = p->parent->parent->right;
			if (y && y->color == RED) {
				p->parent->color = BLACK;
				y->color = BLACK;
				p->parent->parent->color = RED;
				p = p->parent->parent;
			} else {
				if (p == p->parent->right) {
					p = p->parent;
					db_rotate_left(p, root);
				}
				p->parent->color = BLACK;
				p->parent->parent->color = RED;
				db_rotate_right(p->parent->parent, root);
			}
		} else {
			struct dbn* y = p->parent->parent->left;
			if (y && y->color == RED) {
				p->parent->color = BLACK;
				y->color = BLACK;
				p->parent->parent->color = RED;
				p = p->parent->parent;
			} else {
				if (p == p->parent->left) {
					p = p->parent;
					db_rotate_right(p, root);
				}
				p->parent->color = BLACK;
				p->parent->parent->color = RED;
				db_rotate_left(p->parent->parent, root);
			}
		}
	}
	(*root)->color=BLACK;
}

void db_rebalance_erase(struct dbn *z,struct dbn **root)
{
	struct dbn *y = z, *x = NULL, *x_parent = NULL;

	if(!z || !root || !*root)
		return;

	if (y->left == NULL)
		x = y->right;
	else if (y->right == NULL)
		x = y->left;
	else // both are NULL
	{	
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
		if (*root == z)
			*root = y;
		else if (z->parent->left == z)
			z->parent->left = y;
		else
			z->parent->right = y;
		y->parent = z->parent;
		{	// swap colors
			int tmp=y->color; 
			y->color=z->color;
			z->color=tmp;
		}
		y = z;
	}
	else
	{	// どちらか空いていた場合 xをzの位置に持ってきてzを浮かせる
		x_parent = y->parent;
		if (x) x->parent = y->parent;
		if (*root == z)
			*root = x;
		else if (z->parent->left == z)
			z->parent->left = x;
		else
			z->parent->right = x;
	}
	// ここまで色の移動の除いて通常の2分木と同じ
	if (y->color != RED)
	{	// 赤が消える分には影響無し
		while (x != *root && (x == NULL || x->color == BLACK))
		{
			if(x == x_parent->left)
			{
				struct dbn* w = x_parent->right;
				if (w->color == RED)
				{
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_left(x_parent, root);
					w = x_parent->right;
				}
				if( (w->left == NULL  || w->left->color == BLACK) &&
					(w->right == NULL || w->right->color == BLACK))
				{
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				}
				else
				{
					if (w->right == NULL || w->right->color == BLACK)
					{
						if (w->left)
							w->left->color = BLACK;
						w->color = RED;
						db_rotate_right(w, root);
						w = x_parent->right;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->right)
						w->right->color = BLACK;
					db_rotate_left(x_parent, root);
					break;
				}
			}
			else
			{	// same as above, with right <-> left.
				struct dbn* w = x_parent->left;
				if( w->color == RED )
				{
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_right(x_parent, root);
					w = x_parent->left;
				}
				if( (w->right == NULL || w->right->color == BLACK) &&
					(w->left == NULL  || w->left->color == BLACK))
				{
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				}
				else
				{
					if (w->left == NULL || w->left->color == BLACK)
					{
						if (w->right)
							w->right->color = BLACK;
						w->color = RED;
						db_rotate_left(w, root);
						w = x_parent->left;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->left)
						w->left->color = BLACK;
					db_rotate_right(x_parent, root);
					break;
				}
			}
		}
		if (x) x->color = BLACK;
	}
}

void db_free_lock(struct dbt *table)
{
	table->free_lock++;
}

void db_free_unlock(struct dbt *table)
{
	if(table->free_lock>0) table->free_lock--;
	if(table->free_lock == 0)
	{
		int i;
		for(i = 0; i < table->free_count ; ++i)
		{
			db_rebalance_erase(table->free_list[i].z, table->free_list[i].root);
			// made a local copy of the key before
			// need to free it now
			if(table->cmp == strdb_cmp)
			{
				if(table->free_list[i].z->key)
				{
					delete[] ((char*)table->free_list[i].z->key);
					table->free_list[i].z->key=NULL;
				}
			}
			free_dbn(table->free_list[i].z);
			table->item_count--;
		}
		table->free_count = 0;
	}
}

struct dbn* db_insert(struct dbt *table,void* key,void* data)
{

//	if(NULL==key || NULL==data)
//	{
//		ShowMessage("db insert 0: key=%p, data=%p\n", key, data);
//	}
	int c = 0;
	size_t hash = table->hashfunc(table,key) % HASH_SIZE;
	struct dbn *priv = NULL;
	struct dbn *p = table->ht[hash];
	while(p)
	{
		c=table->cmp(table,key,p->key);
		if(c==0)
		{	// replace
			if(table->release)
				table->release(p, 3);
			if(p->deleted)
			{	// 削除されたデータなので、free_list 上の削除予定を消す
				int i;
				for(i = 0; i < table->free_count ; ++i)
				{
					if(table->free_list[i].z == p)
					{
						memmove(
							&table->free_list[i],
							&table->free_list[i+1],
							sizeof(struct db_free)*(table->free_count - i - 1)
							);
						break;
					}
				}
				if(i == table->free_count || table->free_count <= 0) {
					ShowError("db_insert: cannnot find deleted db node.\n");
				} else {
					table->free_count--;
					if(table->cmp == strdb_cmp)
					{
						delete[] ((char*)p->key);
					}
				}
			}
			p->data=data;
			p->key=key;
			p->deleted = 0;
			return p;
		}
		priv=p;
		if(c<0){
			p=p->left;
		} else {
			p=p->right;
		}
	}
	p = malloc_dbn();
	if(p==NULL){
		ShowError("out of memory : db_insert\n");
		return NULL;
	}
	p->parent= NULL;
	p->left  = NULL;
	p->right = NULL;
	p->key   = key;
	p->data  = data;
	p->color = RED;
	p->deleted = 0;
	p->prev = NULL;
	p->next = NULL;

	// double linked list with order of insertion
	if(table->head == NULL)
		table->head = table->tail = p;
	else
	{
		p->prev = table->tail;
		table->tail->next = p;
		table->tail = p;
	}

	if(c==0){ // hash entry is empty
		table->ht[hash] = p;
		p->color = BLACK;
	} else {
		if(c<0){ // left node
			priv->left = p;
			p->parent=priv;
		} else { // right node
			priv->right = p;
			p->parent=priv;
		}
		if(priv->color==RED){ // must rebalance
			db_rebalance(p,&table->ht[hash]);
		}
	}
	table->item_count++;
	return p;
}

void* db_erase(struct dbt *table,void* key)
{
	void *data=NULL;
	if(table)
	{
		int c;
		size_t hash = table->hashfunc(table,key) % HASH_SIZE;
		struct dbn *p = table->ht[hash];
		while(p)
		{
			c=table->cmp(table,key,p->key);
			if(c==0)
				break;
			if(c<0)
				p=p->left;
			else
				p=p->right;
		}
		if(!p || p->deleted)
			return NULL;
		data=p->data;
		if(table->free_lock)
		{
			if(table->free_count == table->free_max)
			{
				const size_t sz = table->free_max + 32;
				struct db_free* tmp = new struct db_free[sz];

				if( table->free_list )
				{
					memcpy(tmp, table->free_list, table->free_max*sizeof(struct db_free));
					delete[] table->free_list;
				}
				memset(tmp+table->free_max,0,32*sizeof(struct db_free));

				table->free_list = tmp;
				table->free_max  = sz;;
	
			}
			table->free_list[table->free_count].z    = p;
			table->free_list[table->free_count].root = &table->ht[hash];
			table->free_count++;
			p->deleted = 1;
			p->data    = NULL;

			// assuming that the key is part of the data,
			// we need to prepare a local copy since the data is gone
			if(table->cmp == strdb_cmp)
			{
				size_t len = 1+((table->maxlen) ? table->maxlen : strlen((const char*)p->key));
				char *key = new char[len];
				memcpy(key,p->key,len);
				key[len-1]=0;
				p->key = key;
			}
		}
		else
		{
			db_rebalance_erase(p,&table->ht[hash]);
			if (p->prev)
				p->prev->next = p->next;
			else
				table->head = p->next;
			if (p->next)
				p->next->prev = p->prev;
			else
				table->tail = p->prev;

			free_dbn(p);
			table->item_count--;
		}
	}
	return data;
}

void db_foreach(struct dbt* table, const CDBProcessor& elem)
{
	if(table)
	{
		int count = table->item_count;
		struct dbn *p = table->head;
		db_free_lock(table);
		while(p)
		{
			if(!p->deleted)
			{	// stop traversal on request
				if( !elem.process(p->key, p->data) )
				{
					count=0;
					break;
				}
			}
			count--;
			p=p->next;
		}
		db_free_unlock(table);
		if(count)
		{
			ShowError("db_foreach : data lost %d item(s) allocated from %s line %d\n",
				count, table->alloc_file, table->alloc_line);
		}
	}
}

void db_final(struct dbt *&table, void (*func)(void*,void*))
{
	if(table)
	{
		struct dbn *pn, *p = table->head;
		db_free_lock(table);
		while(p)
		{
			if(!p->deleted)
			{
				if(func) func(p->key,p->data);
				p->key=NULL;
				p->data=NULL;
			}

			pn=p->next;

			if (p->prev)
				p->prev->next = p->next;
			else
				table->head = p->next;
			if (p->next)
				p->next->prev = p->prev;
			else
				table->tail = p->prev;

			free_dbn(p);
			p=pn;
		}
		db_free_unlock(table);
		if(table->free_list)
		{
			delete table->free_list;
			table->free_list=NULL;
		}
		delete table;
		table=NULL;
	}
}
void db_final(struct dbt*&table, const CDBProcessor& elem)
{
	if(table)
	{
		struct dbn *pn, *p = table->head;
		db_free_lock(table);
		while(p)
		{
			if(!p->deleted)
			{
				// ignore return value
				elem.process(p->key,p->data);
				p->key=NULL;
				p->data=NULL;
			}
			pn = p->next;

			if (p->prev)
				p->prev->next = p->next;
			else
				table->head = p->next;
			if (p->next)
				p->next->prev = p->prev;
			else
				table->tail = p->prev;
			free_dbn(p);
			p=pn;
		}
		db_free_unlock(table);
		if(table->free_list)
		{
			delete table->free_list;
			table->free_list=NULL;
		}

		delete table;
		table=NULL;
	}
}
