#ifndef __BASEARRAY_H__
#define __BASEARRAY_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"


//////////////////////////////////////////////////////////////////////////
// basic lists/arrays
//////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// basic interface for arrays
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArray : public global
{
protected:
	virtual const T* array() const=0;

protected:
	///////////////////////////////////////////////////////////////////////////
	// copy, move, compare
	virtual void copy(T* tar, const T* src, size_t cnt) = 0;
	virtual void move(T* tar, const T* src, size_t cnt) = 0;
	virtual int  compare(const T& a, const T& b) const = 0;

public:
	virtual bool realloc(size_t newsize) = 0;
	virtual bool realloc(size_t expectaddition, size_t growsize) = 0;
	virtual bool realloc() = 0;

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArray()							{}
	virtual ~TArray()					{}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer
	virtual const T* getreadbuffer(size_t &maxcnt) const=0;
	virtual T* getwritebuffer(size_t &maxcnt)			=0;

	virtual bool setreadsize(size_t cnt)				=0;
	virtual bool setwritesize(size_t cnt)				=0;

	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)		=0;

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const 	=0;
	virtual T& operator[](size_t inx)		=0;	

	T& first()	{ return this->operator[](0); }
	T& last()	{ return this->operator[]((this->size()>0)?(this->size()-1):0); }


	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			=0;	


	///////////////////////////////////////////////////////////////////////////
	// returns number of elements
	virtual size_t size() const				=0;	
	virtual size_t freesize() const			=0;	


	///////////////////////////////////////////////////////////////////////////
	// push/pop access
	virtual bool push(const T& elem)		{ return append(elem); }
	virtual bool push(const TArray<T>& list){ return append(list); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T pop()							=0;	
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)				=0;	
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const					=0;	
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const			=0;	


	///////////////////////////////////////////////////////////////////////////
	// add an element at the end
	virtual bool append(const T& elem, size_t cnt=1) =0;	
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt) =0;	

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()					=0;	

	///////////////////////////////////////////////////////////////////////////
	// move an element inside the buffer
	virtual bool move(size_t tarpos, size_t srcpos) = 0;
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)	=0;	

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)	=0;
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)	=0;

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt) 	=0;	


	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const=0;
	virtual ssize_t find(const T& elem, size_t startpos=0) const=0;
};
///////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
// fixed size arrays
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TArrayFST : public TArray<T>
{
	virtual const T* array() const	{return cField;}
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		cField[SZ];	// fixed size array
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int compare(const T& a, const T& b) const
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}
public:
	virtual bool  realloc(size_t newsize) { return false; }
	virtual bool realloc(size_t expectaddition, size_t growsize)	{ return false; }
	virtual bool realloc()	{ return false; }

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayFST() : cCnt(0)	{}
	virtual ~TArrayFST()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayFST(const TArray<T>& arr):cCnt(0)					{ this->copy(arr); }
	const TArrayFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this;}
	TArrayFST(const TArrayFST<T,SZ>& arr):cCnt(0)			{ this->copy(arr); }
	const TArrayFST& operator=(const TArrayFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this;}

	TArrayFST(const T* elem, size_t sz):cCnt(0)				{ this->copy(elem, sz); }

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer/max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		if( cCnt < SZ )
		{
			maxcnt = SZ-cCnt;
			return const_cast<T*>(cField+cCnt);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt )
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < SZ )
		{
			cCnt += cnt;
			ret = true;
		}
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		if( cnt < SZ )
		{
			cCnt = cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ return cCnt; }	
	virtual size_t freesize() const			{ return SZ-cCnt; }


	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from list
	virtual T pop()
	{
		if( cCnt > 0 )
		{
			return cField[--cCnt];
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( cCnt > 0 )
		{
			elem = cField[--cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// move inside the array
	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		if(srcpos>cCnt) srcpos=cCnt; 
		if( ( tarpos > srcpos && cCnt+tarpos < SZ+srcpos ) || // enlarge only up to limit
			( tarpos < srcpos                            ) )
		{
			move(cField+tarpos,cField+srcpos,cCnt-srcpos);
			cCnt += tarpos-srcpos;
			return true;
		}	
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		if(cCnt < SZ)
		{
			while(cnt--) cField[cCnt++] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		if( elem && cCnt+cnt < SZ )
		{
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1)
	{
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		if( cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos = cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			while(cnt--) cField[pos+cnt] = elem;
			cCnt+=cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( elem && cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// place cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt < SZ )
			{
				copy(cField+pos,elem,cnt);
				cCnt = pos+cnt;
				return true;
			}
		}
		return false;
	}
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{	
		if(this!=&list)
		{
			size_t cnt;
			const T* elem = list.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem && (cCnt+cnt < SZ+poscnt) )
		{
			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		for(size_t i=startpos; i<cCnt; i++)
		{
			if( elem== cField[i] )
			{	pos = i;
				return true;
			}
		}
		return false;
	}
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		for(size_t i=startpos; i<cCnt; i++)
		{
			if( elem== cField[i] )
			{	
				return i;
			}
		}
		return -1;
	}
};




///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TfifoFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoFST()	{}
	virtual ~TfifoFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoFST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoFST(const TfifoFST<T,SZ>& arr)					{ this->copy(arr); }
	const TfifoFST& operator=(const TfifoFST<T,SZ>& arr){ this->resize(0); this->copy(arr); return *this; }


	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T pop()
	{
		if( this->cCnt > 0 )
		{
			T elem = this->cField[0];
			this->move(this->cField+0, this->cField+1, --this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return this->cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			move(this->cField+0, this->cField+1, --this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TstackFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackFST()				{}
	virtual ~TstackFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackFST(const TArray<T>& arr)							{ this->copy(arr); }
	const TstackFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TstackFST(const TstackFST<T,SZ>& arr)					{ this->copy(arr); }
	const TstackFST& operator=(const TstackFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
// basic interface for sorted lists
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TslistFST : public TfifoFST<T,SZ>
{
	bool cAscending;// sorting order
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistFST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistFST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistFST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)		{ this->copy(arr); }
	const TslistFST& operator=(const TArray<T>& arr)									{ this->resize(0); this->copy(arr); return *this; }
	TslistFST(const TslistFST<T,SZ>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistFST& operator=(const TslistFST<T,SZ>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistFST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)				{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool push(const T& elem) 				{ return insert(elem); }
	virtual bool push(const TArray<T>& list)		{ return insert(list); }
	virtual bool push(const T* elem, size_t cnt)	{ return insert(elem,cnt); }

	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=insert(elem); return ret;}
	virtual bool append(const TArray<T>& list)		{ return insert(list); }
	virtual bool append(const T* elem, size_t cnt)	{ return insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{
		// ignore position, insert sorted
		if( this->cCnt <SZ )
		{
			bool f = this->find(elem, 0, pos);
			if( !f || cAllowDup )
			{
				move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
				this->cCnt++;
				this->cField[pos] = elem;
			}
			return true;
		}
		// else found
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		for(size_t i=0; i<cnt && this->cCnt<SZ; i++)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{
		if(this!=&list)
		{
			for(size_t i=0; i<list.size() && this->cCnt<SZ; i++)
			{
				this->insert( list[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{
		if(this!=&list)
		{
			this->clear();
			return this->insert(list);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(list);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}
/*
	{	// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1, c;	//smallest first

		pos = 0;
		if( this->cCnt==0 )
		{
			ret = false;
		}
		else if( elem < this->cField[a] )
		{	pos = a;
			ret = false; //less than lower
		}
		else if( elem > this->cField[b] )
		{	pos = b+1;
			ret = false; //larger than upper
		}
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == this->cField[c] )
				{	b=c; // was pos = c;
					ret = true;
					break;
				}
				else if( elem < this->cField[c] )
					b=c;
				else
					a=c;
			}while( (a+1) < b );
			pos = b;//return the next larger element to the given or the found element
		}
		return ret;
	}
*/
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}

};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// fixed size arrays
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDST : public TArray<T>
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class MiniString;
public:
	virtual const T* array() const	{ return cField; }
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		*cField;	// array
	size_t	cSZ;		// allocates array size
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int  compare(const T& a, const T& b) const	
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}

public:
	virtual bool realloc(size_t newsize)
	{	
		if(  cSZ < newsize )
		{	// grow rule
			size_t tarsize = newsize;
			newsize = 2;
			while( newsize < tarsize ) newsize *= 2;
		}
		else if( cSZ>8 && cCnt < cSZ/4 && newsize < cSZ/2)
		{	// shrink rule
			newsize = cSZ/2;
		}
		else // no change
			return true;


		T *newfield = new T[newsize];
		if(newfield==NULL)
			throw exception_memory("TArrayDST: memory allocation failed");

		if(cField)
		{
			copy(newfield, cField, cCnt); // between read ptr and write ptr
			delete[] cField;
		}
		cSZ = newsize;
		cField = newfield;
		return cField!=NULL;
	}
	virtual bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			if(cSZ==0) cSZ=2;
			while(cCnt+expectaddition >= cSZ)
				cSZ+=(growsize)?growsize:cSZ;
			T* newfield = new T[cSZ];
			if(newfield)
			{
				if(cField) 
				{
					copy(newfield,cField,cCnt);
					delete[] cField;
				}
				cField = newfield;
			}
		}
		return cField!=NULL;
	}
	virtual bool realloc()
	{
		if(cCnt != cSZ)
		{	
			if(cCnt)
			{	// need to resize
				cSZ = cCnt;
				T* newfield = new T[cSZ];
				if(newfield)
				{
					if(cField) 
					{
						copy(newfield,cField,cCnt);
						delete[] cField;
					}
					cField = newfield;
				}
			}
			else
			{	// just clear all
				if(cField)
				{
					delete[] cField;
					cField=NULL;
					cSZ = 0;
				}
			}
		}
		return cField!=NULL;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDST() : cField(NULL),cSZ(0),cCnt(0)	{}
	TArrayDST(size_t sz) : cField(NULL),cSZ(0),cCnt(0)	{ resize(sz); }
	virtual ~TArrayDST()	{ if(cField) delete[] cField; }

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDST(const TArray<T>& arr) : cField(NULL),cSZ(0),cCnt(0)	{ this->copy(arr); }
	const TArrayDST& operator=(const TArray<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }
	TArrayDST(const TArrayDST<T>& arr) : cField(NULL),cSZ(0),cCnt(0){ this->copy(arr); }
	const TArrayDST& operator=(const TArrayDST<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }

	TArrayDST(const T* elem, size_t sz):cField(NULL),cSZ(0),cCnt(0)	{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// put a element to the list
	virtual bool push(const T& elem)			{ return append(elem); }
	virtual bool push(const TArray<T>& list)	{ return append(list); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from list
	virtual T pop()
	{
		if( cCnt > 0 )
		{
			return cField[--cCnt];
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( cCnt > 0 )
		{
			elem = cField[--cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}


	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer and max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		if( cCnt+maxcnt > cSZ )
			realloc( maxcnt+cCnt );
		return const_cast<T*>(cField+cCnt);
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt)
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < cSZ )
		{
			cCnt += cnt;
			ret = true;
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		if(cnt > cSZ)
			realloc(cnt);
		cCnt = cnt;
		return NULL!=cField;
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ return cCnt; }	
	virtual size_t freesize() const			{ return cSZ-cCnt; }	


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		if(cCnt+cnt > cSZ)
			realloc(cCnt+cnt);
		while(cnt--) cField[cCnt++] = elem;
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		if( elem )
		{
			if( cCnt+cnt > cSZ)
				realloc( cCnt+cnt );
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1)
	{
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		if( cCnt+cnt > cSZ )
			realloc(cSZ+cnt);

		if(pos >= cCnt) 
			pos = cCnt;
		else
			move(cField+pos+cnt, cField+pos, cCnt-pos);

		cCnt += cnt;
		while(cnt--) cField[pos++] = elem;
		return true;

	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( elem )
		{	
			if( cCnt+cnt > cSZ )
				realloc(cCnt+cnt);

			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt > cSZ )
				realloc(pos+cnt);
			copy(cField+pos,elem,cnt);
			cCnt = pos+cnt;
			return true;
		}
		return false;
	}
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{	
		if(this!=&list)
		{
			size_t cnt;
			const T* elem = list.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// Moving elements inside the buffer
	// always take the elements from 'from' up to 'cElements'

	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		if(srcpos>cCnt) srcpos=cCnt; 
		if( cCnt+tarpos > cSZ+srcpos )
			realloc(cCnt+tarpos-srcpos);
		move(cField+tarpos,cField+srcpos,cCnt-srcpos);
		cCnt += tarpos-srcpos;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem )
		{
			if( cCnt+cnt > cSZ+poscnt)
				realloc(cCnt+cnt-poscnt);

			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{	
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		for(size_t i=startpos; i<cCnt; i++)
			if( elem == cField[i] )
			{	pos = i;
				return true;
			}
		return false;
	}
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		for(size_t i=startpos; i<cCnt; i++)
			if( elem == cField[i] )
			{	
				return i;
			}
		return -1;
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T> class TfifoDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDST()	{}
	virtual ~TfifoDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoDST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoDST(const TfifoDST<T>& arr)					{ this->copy(arr); }
	const TfifoDST& operator=(const TfifoDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T pop()
	{
		if( this->cCnt > 0 )
		{
			T elem = this->cField[0];
			this->move(this->cField+0, this->cField+1,--this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return this->cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			move(this->cField+0, this->cField+1,--this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
template <class T> class TstackDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDST()				{}
	virtual ~TstackDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TstackDST& operator=(const TArray<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }
	TstackDST(const TstackDST<T>& arr)					{ this->copy(arr); }
	const TstackDST& operator=(const TstackDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
// basic interface for sorted lists
///////////////////////////////////////////////////////////////////////////////
template <class T> class TslistDST : public TfifoDST<T>
{
	bool cAscending;// sorting order
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistDST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistDST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TArray<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }
	TslistDST(const TslistDST<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TslistDST<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistDST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)			{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool push(const T& elem) 				{ return this->insert(elem); }
	virtual bool push(const TArray<T>& list)		{ return this->insert(list); }
	virtual bool push(const T* elem, size_t cnt)	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos at the end by default
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=this->insert(elem); return ret;}
	virtual bool append(const TArray<T>& list) 		{ return this->insert(list); }
	virtual bool append(const T* elem, size_t cnt) 	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{	// ignore position, insert sorted
		bool f = find(elem, 0, pos);
		if( !f || cAllowDup )
		{
			if( this->cCnt >= this->cSZ )
				this->realloc(this->cSZ+1);

			this->move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
			this->cCnt++;
			this->cField[pos] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		if( this->cCnt+cnt>this->cSZ )
			realloc(this->cCnt+cnt);

		for(size_t i=0; i<cnt; i++)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{
		if(this!=&list)
		{
			if( this->cCnt+list.size()>this->cSZ )
				this->realloc(this->cCnt+list.size());

			for(size_t i=0; i<list.size(); i++)
			{
				this->insert( list[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{
		if(this!=&list)
		{
			this->clear();
			return this->insert(list);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(list);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}
/*
	{	// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1, c;	//smallest first

		pos = 0;
		if( this->cCnt==0 )
		{
			ret = false;
		}
		else if( elem < this->cField[a] )
		{	pos = a;
			ret = false; //less than lower
		}
		else if( elem > this->cField[b] )
		{	pos = b+1;
			ret = false; //larger than upper
		}
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == this->cField[c] )
				{	b=c; // was pos = c;
					ret = true;
					break;
				}
				else if( elem < this->cField[c] )
					b=c;
				else
					a=c;
			}while( (a+1) < b );
			pos = b;//return the next larger element to the given or the found element
		}
		return ret;
	}
*/
	virtual ssize_t find(const T& elem, size_t startpos=0) const
	{
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}

};
///////////////////////////////////////////////////////////////////////////////


template <class T> class TArrayDCT : public TArrayDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDCT()				{}
	virtual ~TArrayDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDCT(const TArray<T>& arr)						{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
	TArrayDCT(const TArrayDCT<T>& arr)					{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArrayDCT<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
};


template <class T> class TfifoDCT : public TfifoDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDCT()			{}
	virtual ~TfifoDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDCT(const TArray<T>& arr)							{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TArray<T>& arr)			{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }
	TfifoDCT(const TfifoDCT<T>& arr)						{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TfifoDCT<T>& arr)		{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }

};
template <class T> class TstackDCT : public TstackDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDCT()				{}
	virtual ~TstackDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDCT(const TArray<T>& arr)						{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }
	TstackDCT(const TstackDCT<T>& arr)					{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TstackDCT<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }

};
template <class T> class TslistDCT : public TslistDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for complex data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TslistDCT()				{}
	virtual ~TslistDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDCT(const TArray<T>& arr)						{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
	TslistDCT(const TslistDCT<T>& arr)					{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TslistDST<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
};


///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using a growing base list and sorted pod lists for storing positions
// base list cannot shrink on delete operations so this is only suitable for static list
// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiList
{

	TArrayDST<T>		cList;
	TArrayDST<size_t>	cIndex[CNT];

public:
	TMultiList()	{}
	~TMultiList()	{}

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t i) const	{ return cList[ cIndex[0][i] ]; }
	T& operator[](size_t i)				{ return cList[ cIndex[0][i] ]; }

	const T& operator()(size_t p,size_t i=0) const	{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }
	T& operator()(size_t p,size_t i=0)				{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }


	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i, pos = cList.size();
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			for(i=0; i<CNT; i++)
			{
				cIndex[i].insert(pos, 1, ipos[i]);
			}
			return cList.append(elem);
		}

		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T& elem = cList[ cIndex[(inx<CNT)?inx:0][pos] ];
		size_t temppos, i;
		pos = cIndex[inx][pos];

		for(i=0; i<CNT; i++)
		{
			if( binsearch(elem, 0, i, temppos, true, &T::compare) )
				cIndex[i].removeindex(temppos);
		}
		return cList.removeindex(pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		size_t i;
		for(i=0; i<CNT; i++)
			cIndex[i].clear();
		return cList.clear();
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][a] ], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};






///////////////////////////////////////////////////////////////////////////////
//
// List of Objects / implemented as list of pointers to (single) objects
// it actually owns the pointers and deletes them on exit
//
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDPT
{
	typedef T* pT;

	pT*		cField;
	size_t	cCnt;
	size_t	cSZ;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	TArrayDPT() : cField(NULL),cCnt(0),cSZ(0)			{  }
	TArrayDPT(size_t cnt) : cField(NULL),cCnt(0),cSZ(0)	{ resize(cnt); }
	~TArrayDPT()										{ clear(); }

	///////////////////////////////////////////////////////////////////////////
	// copy/assign
	TArrayDPT(const TArrayDPT& tp) : cField(NULL),cCnt(0),cSZ(0)
	{
		this->copy(tp);
	}

	const TArrayDPT& operator=(const TArrayDPT& tp)
	{
		this->resize(0); 
		this->copy(tp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// access functions
	void copy(const TArrayDPT& tp)
	{
		if(this!=&tp)
		{
			size_t i;
			realloc(tp.cCnt);
			for(i=0; i<tp.cCnt; i++)
			{
				if(tp.cField[i])
				{	// list element in source list, copy it
					if(!cField[i])
						cField[i] = new T(*tp.cField[i]);
					else
						*cField[i] = *tp.cField[i];
				}
				else
				{	// empty element in source list
					if(cField[i])
					{
						delete cField[i];
						cField[i] = NULL;
					}
				}

			}
			this->cCnt=tp.cCnt;
		}
	}
	void clear()
	{
		if(cField)
		{	
			size_t i;
			for(i=0; i<cSZ; i++)
				if(cField[i]) delete cField[i];
			delete[] cField;
			cField=NULL;
			cCnt=0;
			cSZ=0;
		}
	}
	bool realloc(size_t newsize)
	{
		if(newsize >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(newsize >= sz) sz*=2;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(cCnt+expectaddition >= sz)
				sz+=(growsize)?growsize:sz;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	bool resize(size_t cnt)
	{
		if(cnt >= cSZ)
			realloc(cnt);
		cCnt = cnt;
		return NULL!=cField;
	}
	T& operator[](size_t inx)
	{
		// automatic resize on out-of-bound
		if( inx>=cCnt )
		{	
			printf("autoresize %lu->%lu\n",(unsigned long)cCnt, (unsigned long)(inx+1));
			resize(inx+1);
		}

		if( !cField[inx] )
		{	// create a new element if not exist
			cField[inx] = new T;
		}
		return *cField[inx];
	}
	const T& operator[](size_t inx) const
	{	// throw something here
		if( inx>=cCnt || !cField[inx] )
			throw exception_bound("TPointerList");

		return *cField[inx];
	}

	T& first()
	{
		return this->operator[](0);
	}
	T& last()
	{	
		return this->operator[]((cCnt>0)?(cCnt-1):0);
	}
	size_t size() const			{ return cCnt; }

	bool insert(const T& elem, size_t pos) // insert an element at position pos
	{
		insert(pos);
		if(cField[pos])
			*cField[pos] = elem;
		else
			cField[pos] = new T(elem);
		return true;
	}
	bool insert(size_t pos) // insert an element at position pos
	{
		if(pos>=cCnt)
		{
			resize(pos+1);
		}
		else
		{
			resize(cCnt+1);
			pT temp =(cField[cCnt]);
			memmove(cField+pos+1,cField+pos,(cCnt-pos)*sizeof(pT));
			cField[pos]=temp;
		}
		return true;
	}
	bool strip(size_t cnt, bool clear=false)
	{
		if(cCnt>=cnt)
		{
			if(clear)
			{	
				while(cnt>0)
				{
					cCnt--;
					if(cField[cCnt])
					{
						delete cField[cCnt];
						cField[cCnt] = NULL;
					}
				}
			}
			else
				cCnt-=cnt;
			return true;
		}
		return false;
	}

	bool removeindex(size_t inx)
	{
		if(inx<cCnt)
		{
			pT temp = cField[inx];
			memmove(cField+inx, cField+inx+1,(cCnt-inx-1)*sizeof(pT));
			cCnt--;
			cField[cCnt] = temp;
		}
		return false;
	}
	bool append(const T& elem)
	{
		realloc(cCnt+1);
		if(cField[cCnt])
			*cField[cCnt] = elem;
		else
			cField[cCnt]=new T(elem);
		cCnt++;
		return true;
	}
	bool append(const T* elem, size_t cnt)
	{
		realloc(cCnt+cnt);
		for(size_t i=0; i<cnt;i++)
		{
			if(cField[cCnt])
				*cField[cCnt] = elem[i];
			else
				cField[cCnt] = new T(elem[i]);
			cCnt++;
		}
		return true;
	}
	bool pop(bool clear=false)
	{
		if(cCnt>0)
		{
			cCnt--;
			if(clear && cField[cCnt])
			{
				delete cField[cCnt];
				cField[cCnt] = NULL;
			}
			return true;
		}
		return false;
	}
	bool push(const T& elem)
	{
		return append(elem);
	}
	bool push(const T* elem, size_t cnt)
	{
		return append(elem, cnt);
	}
};














///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using Pointers to stored objects for internal lists, 
// subsequent insert/delete does new/delete the objects, 
// for performance use a managed memory derived classes
// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListP
{

	TArrayDST<T*>	cIndex[CNT];

public:
	TMultiListP()			{}
	virtual ~TMultiListP()	{ clear(); }

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t p) const	{ return *cIndex[0][p]; }
	T& operator[](size_t p)				{ return *cIndex[0][p]; }

	const T& operator()(size_t p,size_t i=0) const	{ return *cIndex[(i<CNT)?i:0][p]; }
	T& operator()(size_t p,size_t i=0)				{ return *cIndex[(i<CNT)?i:0][p]; }


	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			T* newelem = new T(elem);
			for(i=0; i<CNT; i++)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}

		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; i++)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				for(i=0; i<CNT; i++)
				{
					ok &= cIndex[i].insert(elem, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T* elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; i++)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); i++)
			delete cIndex[0][i];
		// clear pointer list
		for(i=0; i<CNT; i++)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};



///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using SavePointers to stored objects for internal lists, 
// subsequent insert/delete does new/delete the objects, 
// for performance use a managed memory derived classes
// usable classes need a "int compare(const T& elem, size_t inx) const" member
//
// needs evaluation
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListSP
{
	TArrayDCT< TPtrAutoCount<T> >	cIndex[CNT];

public:
	TMultiListSP()	{}
	~TMultiListSP()	{ clear(); }

	size_t size() const												{ return cIndex[0].size(); }
	const TPtrAutoCount<T> operator[](size_t p) const				{ return cIndex[0][p]; }
	TPtrAutoCount<T> operator[](size_t p)							{ return cIndex[0][p]; }

	const TPtrAutoCount<T> operator()(size_t p,size_t i=0) const	{ return cIndex[(i<CNT)?i:0][p]; }
	TPtrAutoCount<T> operator()(size_t p,size_t i=0)				{ return cIndex[(i<CNT)?i:0][p]; }

	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			TPtrAutoCount<T> newelem(new T(elem));
			for(i=0; i<CNT; i++)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}
		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; i++)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				TPtrAutoCount<T> xx(elem);
				for(i=0; i<CNT; i++)
				{
					ok &= cIndex[i].insert(xx, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		TPtrAutoCount<T> elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; i++)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); i++)
			cIndex[0][i].clear();
		// clear pointer list
		for(i=0; i<CNT; i++)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

#endif//__BASEARRAY_H__
