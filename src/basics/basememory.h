#ifndef __BASEMEMORY_H__
#define __BASEMEMORY_H__


//## change class hierarchy				(90%, testing phase)
//## moving basic access interface here	(evaluating)


#include "basetypes.h"
#include "baseswap.h"
#include "baseobjects.h"

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_BEGIN(basics)
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/// test function
void test_memory(void);


//////////////////////////////////////////////////////////////////////////
/// memory allocation
//////////////////////////////////////////////////////////////////////////
void* memalloc(uint a);
void* memrealloc(void* p, uint a);
void memfree(void* p);

///////////////////////////////////////////////////////////////////////////////
/// memory quantisizer
///////////////////////////////////////////////////////////////////////////////
extern inline size_t memquantize(size_t sz)
{
	static const size_t quant1 = (size_t)(   64);
	static const size_t qmask1 = (size_t)(  ~63);
	static const size_t quant2 = (size_t)( 4096);
	static const size_t qmask2 = (size_t)(~4095);

	if( sz <= 16 )
		return 16;
	else if( sz <= 32 )
		return 32;
	else if( sz <= 2048 )
		return (sz + quant1 - 1) & qmask1;
	else
		return (sz + quant2 - 1) & qmask2;
}

#ifndef BASICS_HAS_ITERATOR_TAGS
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};
#define BASICS_HAS_ITERATOR_TAGS
#endif//BASICS_HAS_ITERATOR_TAGS


NAMESPACE_BEGIN(elaborator)


template<typename T>
inline T*  intern_move(T* target, const T* source, size_t cnt, bool_true)
{	// simple type mover, no checks performed
	memmove(target, source, cnt*sizeof(T));
	return target+cnt;
}
template<typename T>
inline T*  intern_copy(T* target, const T* source, size_t cnt, bool_true)
{	// simple type copy, no checks performed
	memcpy(target, source, cnt*sizeof(T));
	return target+cnt;
}
template<typename T>
inline int intern_cmp(const T* a, const T* b, size_t cnt, bool_true)
{	// simple type compare, no checks performed
	return memcmp(a,b,cnt*sizeof(T));
}


template<typename T>
inline T* intern_move(T* target, const T* source, size_t cnt, bool_false)
{	
	if(target>source)
	{	// last to first run
		T* epp=target;
		target+=cnt-1;
		source+=cnt-1;
		while( target>=epp ) *target-- = *source--;
		return epp+cnt;
	}
	else if(target<source)
	{	// first to last run
		T* epp=target+cnt;
		while( target< epp ) *target++ = *source++;
		return epp;
	}
	else
		// identical; no move necessary
		return target+cnt;
}
template<typename T>
inline T*  intern_copy(T* target, const T* source, size_t cnt, bool_false)
{	
	T* epp=target+cnt;
	while( target<epp ) *target++ = *source++;
	return epp;
}
template<typename T>
inline int intern_cmp(const T* a, const T* b, size_t cnt, bool_false)
{	
	const T* epp=a+cnt;
	for( ; a<epp ; ++a,++b)
	{
		if( a==b || *a==*b )
			continue;
		else if( *a < *b )
			return -1;
		else
			return  1;
	}
	return 0;
}

template<typename T>
struct is_simple_type
{
	enum _dummy { Result = sizeof(T)<sizeof(void*) };
	typedef typename bool2type<Result>::Type Type;
};

template<typename T>
inline T*  intern_move(T* target, const T* source, size_t cnt)
{	
	typedef typename elaborator::is_simple_type<T>::Type is_simple;
	return intern_move(target, source, cnt, is_simple());
}
template<typename T>
inline T*  intern_copy(T* target, const T* source, size_t cnt)
{
	typedef typename elaborator::is_simple_type<T>::Type is_simple;
	return intern_copy(target, source, cnt, is_simple());
}
template<typename T>
inline int intern_cmp(const T* a, const T* b, size_t cnt)
{
	typedef typename elaborator::is_simple_type<T>::Type is_simple;
	return intern_cmp(a, b, cnt, is_simple());
}

NAMESPACE_END(elaborator)

///////////////////////////////////////////////////////////////////////////////
/// basic allocator interface.
/// for allocating memory in different ways
/// defines basic functions of a storage element
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator : public global, public noncopyable
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator)
public:
	typedef size_t						size_type;		///< type of positions
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ssize_t						difference_type;
	typedef T*							pointer;
	typedef T&							reference;

	typedef const value_type*			const_iterator;		///< stl like pointer iterator
	typedef value_type*					simple_iterator;	///< stl like pointer iterator

	static const size_type				npos;	///< not-valid-position constant

	///////////////////////////////////////////////////////////////////////////
	/// iterator wrapper.
	/// provised iterator interface with additional checkings and conversions
	class iterator
	{
	public:
		typedef random_access_iterator_tag	iterator_category;
		typedef T							value_type;
		typedef size_t						difference_type;
		typedef T*							pointer;
		typedef T&							reference;
	private:
		const allocator* base;
		T* ptr;
	public:
		iterator()
			: base(NULL), ptr(NULL)
		{}
		iterator(const allocator& a)
			: base(&a), ptr(const_cast<T*>(a.begin()))
		{}
		iterator(const allocator& a, const T* p)
			: base(&a), ptr(const_cast<T*>((p && base && p>=base->begin() && p<=base->end())?p:(base->begin()?base->begin()-1:NULL)))
		{}
		~iterator()	{}
		// can use default copy//assignment
		const iterator& operator=(const T* p)
		{
			this->ptr=(p && base && p>=base->begin() && p<=base->end())?const_cast<T*>(p):NULL;
			return *this;
		}
		const iterator& operator=(const T& p)
		{
			this->ptr=(&p && base && &p>=base->begin() && &p<=base->end())?const_cast<T*>(&p):NULL;
			return *this;
		}
		const iterator& operator=(const allocator& a)
		{
			base = &a;
			ptr = const_cast<T*>(a.begin());
			return *this;
		}
		size_t position() const	{ return (this->ptr && this->base && this->ptr>=base->begin() && this->ptr<=base->end())?(this->ptr-base->begin()):(0); }
		bool isvalid() const	{ return (this->ptr && this->base && base->begin()<base->end() && this->ptr>=base->begin() && this->ptr<base->end()); }
		operator bool() const	{ return this->isvalid(); }
		bool operator !() const	{ return !this->isvalid(); }
		T* operator()()			{ return this->isvalid()?this->ptr:NULL; }
		T& operator*()			{ return *this->ptr; }
		T* operator->()			{ return this->isvalid()?this->ptr:NULL; }

		T* operator++()			
		{	// preincrement
			if( base && this->ptr < this->base->end() )
			{
				++this->ptr;
				if( this->ptr < this->base->end() )
					return this->ptr;
			}
			return NULL; 
		}
		T* operator++(int)
		{	// postincrement
			if( base && this->ptr < this->base->end() )
				return this->ptr++;
			return NULL; 
		}
		T* operator--()
		{	// predecrement
			if( base && this->ptr >= this->base->begin() )
			{
				--this->ptr;
				if(this->ptr >= this->base->begin())
					return this->ptr;
			}
			return NULL; 
		}
		T* operator--(int)
		{	// postdecrement
			if( base && this->ptr >= this->base->begin() )
				return this->ptr--;
			return NULL; 
		}
		bool operator==(const T*p) const	{ return this->ptr==p; }
		bool operator!=(const T*p) const	{ return this->ptr!=p; }
		bool operator>=(const T*p) const	{ return this->ptr>=p; }
		bool operator> (const T*p) const	{ return this->ptr> p; }
		bool operator<=(const T*p) const	{ return this->ptr<=p; }
		bool operator< (const T*p) const	{ return this->ptr< p; }

		bool operator==(const iterator&i) const	{ return this->ptr==i.ptr; }
		bool operator!=(const iterator&i) const	{ return this->ptr!=i.ptr; }
		bool operator>=(const iterator&i) const	{ return this->ptr>=i.ptr; }
		bool operator> (const iterator&i) const	{ return this->ptr> i.ptr; }
		bool operator<=(const iterator&i) const	{ return this->ptr<=i.ptr; }
		bool operator< (const iterator&i) const	{ return this->ptr< i.ptr; }

		bool operator&&(const iterator&i) const	{ return this->isvalid() && i.isvalid(); }
		bool operator||(const iterator&i) const	{ return this->isvalid() || i.isvalid(); }
	};


	allocator()				{}
	virtual ~allocator()	{}

	//virtual operator const T*() const =0;
	virtual size_t length()	const =0;
	virtual size_t size() const { return length(); }
	virtual size_t capacity() const =0;
	virtual const T* begin() const=0;
	virtual const T* final() const=0;
	virtual const T* end() const=0;
	virtual       T* begin()=0;
	virtual       T* final()=0;
	virtual       T* end()=0;
};

template <typename T>
const RETURN_TYPENAME allocator<T>::size_type allocator<T>::npos = static_cast<typename allocator<T>::size_type>(-1);



///////////////////////////////////////////////////////////////////////////////
/// allocators for write buffers.
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator_w : public virtual allocator<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_w)
protected:
	mutable T* cBuf;
	T* cEnd;
	T* cWpp;

	const T*& ptrBuf() const	{ return cBuf; }
	      T*& ptrRpp() const	{ return cBuf; }
	      T*& ptrWpp() const	{ return cWpp; }
	const T*& ptrEnd() const	{ return cEnd; }

	// std construct/destruct
	allocator_w()	: cBuf(NULL), cEnd(NULL), cWpp(NULL)				{}
	allocator_w(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cWpp(buf)	{}
	virtual ~allocator_w()	{ cBuf=cEnd=cWpp=NULL; }

	virtual bool checkwrite(size_t addsize)	=0;
public:
	virtual size_t length()	const		{ return (this->cWpp-this->cBuf); }
	virtual size_t capacity() const		{ return (this->cEnd-this->cBuf); }
	virtual const T* begin() const		{ return  this->cBuf; }
	virtual const T* final() const		{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual const T* end() const		{ return  this->cWpp; }
	virtual       T* begin()			{ return  this->cBuf; }
	virtual       T* final()			{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual       T* end()				{ return  this->cWpp; }
};



///////////////////////////////////////////////////////////////////////////////
/// dynamic write-buffer allocator (string version).
/// creates/reallocates storage on its own
/// size is calculated with one element in advance
/// so it can be used for cstring allocation
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator_ws_dy : public allocator_w<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_ws_dy)
protected:
	allocator_ws_dy() : allocator_w<T>()			{ }
	allocator_ws_dy(size_t sz) : allocator_w<T>()	{ checkwrite(sz); }
	allocator_ws_dy(T* buf, size_t sz)				{ }	// no implementation here
	virtual ~allocator_ws_dy()						{ if(this->cBuf) delete[] this->cBuf; }
//## bloat
	virtual bool checkwrite(size_t addsize)
	{
		const size_t used_count = this->cWpp-this->cBuf;
		const size_t alloccount = this->cEnd-this->cBuf;
		const size_t new__count = used_count+addsize  +1 ; //one more than requested
		if( new__count > alloccount )
		{	// allocate new
			const size_t sz = memquantize( new__count );
			T *tmp = new T[sz];
			if(!tmp) return false;
			if(this->cBuf)
			{
				elaborator::intern_copy<T>(tmp, this->cBuf, used_count);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+used_count;
			this->cBuf = tmp;
		}
		return (this->cEnd > this->cBuf);
	}
	// have the EOS be part of the iteration
	virtual const T* end() const		{ return this->cWpp?this->cWpp-1:NULL; }
	virtual       T* end()				{ return this->cWpp?this->cWpp-1:NULL; }
};


///////////////////////////////////////////////////////////////////////////////
/// static write-buffer (string version).
/// is using an external buffer for writing
/// does not resize but just returns false on checkwrite
/// size is calculated with one element in advance
/// so it can be used for cstring allocation
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator_ws_st : public allocator_w<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_ws_st)
protected:
	allocator_ws_st()			{}			// no implementation here
	allocator_ws_st(size_t sz)	{}			// no implementation here
	allocator_ws_st(T* buf, size_t sz) : allocator_w<T>(buf, sz)	{}
	virtual ~allocator_ws_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		return ( this->cWpp && this->cWpp +addsize < this->cEnd );
	}
	// have the EOS be part of the iteration
	virtual const T* final() const		{ return (this->cWpp)?(this->cWpp[-1]?this->cWpp-1:this->cWpp-2):this->cWpp; }
 	virtual const T* end() const		{ return this->cWpp?this->cWpp-1:NULL; }
	virtual       T* final()			{ return (this->cWpp)?(this->cWpp[-1]?this->cWpp-1:this->cWpp-2):this->cWpp; }
	virtual       T* end()				{ return this->cWpp?this->cWpp-1:NULL; }
};

///////////////////////////////////////////////////////////////////////////////
/// dynamic write-buffer allocator.
/// creates/reallocates storage on its own
/// (there is no extra element here, Wpp points outside of the array when full)
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator_w_dy : public allocator_w<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_w_dy)
protected:
	allocator_w_dy() : allocator_w<T>()			{}
	allocator_w_dy(size_t sz) : allocator_w<T>()	{ checkwrite(sz); }
	allocator_w_dy(T* buf, size_t sz)				{}	// no implementation here
	virtual ~allocator_w_dy()						{ if(this->cBuf) delete[] this->cBuf; }
//## bloat
	virtual bool checkwrite(size_t addsize)
	{
		const size_t used_count = this->cWpp-this->cBuf;
		const size_t alloccount = this->cEnd-this->cBuf;
		const size_t new__count = used_count + addsize;

		if( new__count > alloccount )
		{	// allocate new
			const size_t sz = alloccount + ((addsize<alloccount)?(alloccount):(addsize));
			T *tmp = new T[sz];
			if(!tmp) return false;
			if(this->cBuf)
			{
				elaborator::intern_copy<T>(tmp, this->cBuf, used_count);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+used_count;
			this->cBuf = tmp;
		}
		return (this->cEnd > this->cBuf);
	}
};


///////////////////////////////////////////////////////////////////////////////
/// static write-buffer. is using an external buffer for writing
/// does not resize but just returns false on checkwrite
/// (there is no extra element here, Wpp points outside of the array when full)
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class allocator_w_st : public allocator_w<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_w_st)
protected:
	allocator_w_st()			{}			// no implementation here
	allocator_w_st(size_t sz)	{}			// no implementation here
	allocator_w_st(T* buf, size_t sz) : allocator_w<T>(buf, sz)	{}
	virtual ~allocator_w_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		return ( this->cWpp && this->cWpp +addsize <= this->cEnd );
	}
};



///////////////////////////////////////////////////////////////////////////////
/// allocators for read/write buffers.
/// ie. for FIFO implementations
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_rw : public virtual allocator<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_rw)
protected:
	T* cBuf;
	T* cEnd;
	mutable T* cRpp; // read also from const objects
	T* cWpp;

	const T*& ptrBuf() const	{ return cBuf; }
	      T*& ptrRpp() const	{ return cRpp; }
	      T*& ptrWpp() const	{ return cWpp; }
	const T*& ptrEnd() const	{ return cEnd; }

	// std construct/destruct
	allocator_rw()	: cBuf(NULL), cEnd(NULL), cRpp(NULL), cWpp(NULL)				{}
	allocator_rw(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cRpp(buf), cWpp(buf)	{}
	virtual ~allocator_rw()	{ cBuf=cEnd=cRpp=cWpp=NULL; }

	virtual bool checkwrite(size_t addsize)	=0;
	virtual bool checkread(size_t addsize) const =0;
public:
	virtual size_t length()	const		{ return (this->cWpp-this->cRpp); }
	virtual size_t capacity() const		{ return (this->cEnd-this->cBuf); }
	virtual const T* begin() const		{ return  this->cRpp; }
	virtual const T* final() const		{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual const T* end() const		{ return  this->cWpp; }
	virtual       T* begin()			{ return  this->cRpp; }
	virtual       T* final()			{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual       T* end()				{ return  this->cWpp; }
};
///////////////////////////////////////////////////////////////////////////////
/// dynamic read/write-buffer allocator.
/// creates/reallocates storage on its own
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_rw_dy : public allocator_rw<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_rw_dy)
protected:
	allocator_rw_dy() : allocator_rw<T>()			{}
	allocator_rw_dy(size_t sz) : allocator_rw<T>()	{ checkwrite(sz); }
	allocator_rw_dy(T* buf, size_t sz)				{}
	virtual ~allocator_rw_dy()				{ if(this->cBuf) delete[] this->cBuf; }
//## bloat
	virtual bool checkwrite(size_t addsize)
	{
		const size_t used_count = this->cWpp-this->cRpp;
		const size_t alloccount = this->cEnd-this->cBuf;
		const size_t new__count = used_count + addsize;

		if( new__count > alloccount )
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quarter of the buffer for >8k buffers
			const size_t sz = alloccount + ((addsize<alloccount)?(alloccount):(addsize));
			T *tmp = new T[sz];
			if(!tmp) return false;
			if(this->cBuf)
			{
				elaborator::intern_copy(tmp, this->cRpp, used_count);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+used_count;
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->cWpp+addsize > this->cEnd )
		{	// moving the current buffer data is sufficient
			elaborator::intern_move(this->cBuf, this->cRpp, used_count);
			this->cWpp = this->cBuf+used_count;
			this->cRpp = this->cBuf;
		}
		return (this->cEnd > this->cBuf);
	}
	virtual bool checkread(size_t addsize) const
	{
		return ( this->cRpp && this->cRpp + addsize <=this->cWpp );
	}
};

///////////////////////////////////////////////////////////////////////////////
/// static read/write-buffer. is using an external buffer for writing
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_rw_st : public allocator_rw<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_rw_st)
protected:
	allocator_rw_st()			{}
	allocator_rw_st(size_t sz)	{}
	allocator_rw_st(T* buf, size_t sz) : allocator_rw<T>(buf, sz)	{}
protected:
	virtual ~allocator_rw_st()				{}
//## bloat
	virtual bool checkwrite(size_t addsize)
	{
		if( this->cWpp+addsize > this->cEnd && this->cBuf < this->cRpp )
		{	// move the current buffer data when necessary and possible 
			const size_t used_count = this->cWpp-this->cRpp;
			elaborator::intern_move(this->cBuf, this->cRpp, used_count);
			this->cWpp = this->cBuf+used_count;
			this->cRpp = this->cBuf;
		}
		return ( this->cWpp && this->cWpp + addsize <= this->cEnd );
	}
	virtual bool checkread(size_t addsize) const
	{
		return ( this->cRpp && this->cRpp + addsize <=this->cWpp );
	}
};




///////////////////////////////////////////////////////////////////////////////
/// allocators for read buffers with additional scanning.
/// ie. for the parser
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_r : public virtual allocator<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_r)
protected:
	T* cBuf;
	T* cEnd;
	T* cRpp;
	T* cWpp;
	T* cScn;

	const T*& ptrBuf() const	{ return cBuf; }
	      T*& ptrRpp() const	{ return cRpp; }
	      T*& ptrWpp() const	{ return cWpp; }
	const T*& ptrEnd() const	{ return cEnd; }


	// std construct/destruct
	allocator_r()	: cBuf(NULL), cEnd(NULL), cRpp(NULL), cWpp(NULL), cScn(NULL) {}
	allocator_r(T* buf, size_t sz, size_t fill=0) : cBuf(buf), cEnd(buf+sz), cRpp(buf), cWpp(buf+min(sz,fill)), cScn(buf) {}
	virtual ~allocator_r() { cBuf=cEnd=cRpp=cScn=cWpp=NULL; }

	// default read function, reads max sz elements to buf and returns the actual count of read elements
	virtual size_t readdata(T*buf, size_t sz) =0;
	virtual bool checkwrite(size_t addsize)	=0;
	virtual bool checkread(size_t addsize) =0;
public:
	virtual size_t length()	const		{ return (this->cWpp-this->cRpp); }
	virtual size_t capacity() const		{ return (this->cEnd-this->cBuf); }

	// not that usefull here
	virtual const T* begin() const		{ return  this->cRpp; }
	virtual const T* final() const		{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual const T* end() const		{ return  this->cWpp; }
	virtual       T* begin()			{ return  this->cRpp; }
	virtual       T* final()			{ return (this->cWpp)?this->cWpp-1:this->cWpp; }
	virtual       T* end()				{ return  this->cWpp; }
};

///////////////////////////////////////////////////////////////////////////////
/// dynamic read-buffer allocator.
/// data left of cRpp is considered invalid on reallocation
/// and overwritten whenever suitable
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_r_dy : public allocator_r<T>
{
	ICL_EMPTY_COPYCONSTRUCTOR(allocator_r_dy)
protected:
	allocator_r_dy() : allocator_r<T>()				{  }
	allocator_r_dy(size_t sz) : allocator_r<T>()	{ this->checkwrite(sz); }
	virtual ~allocator_r_dy()						{ if(this->cBuf) delete[] this->cBuf; }
//## bloat
	virtual bool checkwrite(size_t addsize)
	{
		const size_t used_count = this->cWpp-this->cRpp;
		const size_t alloccount = this->cEnd-this->cBuf;
		const size_t new__count = used_count + addsize;

		if( new__count > alloccount )
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quarter of the buffer for >8k buffers
			const size_t sz = alloccount + ((addsize<alloccount)?(alloccount):(addsize));
			T *tmp = new T[sz];
			if(!tmp) return false;
			if(this->cBuf)
			{
				elaborator::intern_copy(tmp, this->cRpp, used_count);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+used_count;
			this->cScn = tmp+(this->cScn-this->cRpp);
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->cWpp+addsize > this->cEnd )
		{	// moving the current buffer data is sufficient
			elaborator::intern_move(this->cBuf, this->cRpp, used_count);
			this->cWpp = this->cBuf+used_count;
			this->cScn = this->cBuf+(this->cScn-this->cRpp);
			this->cRpp = this->cBuf;
		}
		return (this->cEnd > this->cBuf);
	}
	virtual bool checkread(size_t addsize)
	{
		T* ptr=(this->cRpp>this->cScn) ? this->cRpp : this->cScn;
		if( !ptr || ptr + addsize >this->cWpp )
		{	
			if( checkwrite( addsize-(this->cWpp-ptr) ) )
			{
				this->cWpp+=readdata(this->cWpp, this->cEnd-this->cWpp);
				return ( ((this->cRpp>this->cScn) ? this->cRpp : this->cScn) + addsize <=this->cWpp );
			}
			return false;
		}
		return true;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// dynamic file read-buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename T=unsigned char>
class allocator_file : public allocator_r_dy<T>
{
	FILE *cFile;
public:
	allocator_file() : allocator_r_dy<T>(1024), cFile(NULL)
	{  }
	allocator_file(const char* name) : allocator_r_dy<T>(1024), cFile(NULL)
	{
		open(name);
	}
	~allocator_file()
	{
		close();
	}
	bool open(const char* name)
	{
		close();
		if(name)
		{
			cFile = fopen(name, "rb");
			this->checkread(1);
		}
		return (NULL!=cFile);
	}
	void close()
	{
		if(cFile)
		{
			fclose(cFile);
			cFile=NULL;
			this->cWpp=this->cRpp=this->cScn=this->cBuf;
		}
	}
protected:
	virtual size_t readdata(T*buf, size_t sz)
	{
		return (cFile)?fread(buf, 1, sz,cFile):0;
	}
};

///////////////////////////////////////////////////////////////////////////////
NAMESPACE_END(basics)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASEMEMORY_H__

