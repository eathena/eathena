#ifndef __BASEMEMORY_H__
#define __BASEMEMORY_H__

#include "basetypes.h"
#include "baseobjects.h"

//////////////////////////////////////////////////////////////////////////
// memory allocation
//////////////////////////////////////////////////////////////////////////
void* memalloc(uint a);
void* memrealloc(void* p, uint a);
void memfree(void* p);
///////////////////////////////////////////////////////////////////////////////
// memory quantisizer
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




///////////////////////////////////////////////////////////////////////////////
// basic elaborator interface
// for objects working on distinct amount of memory
// defines basic functions to modify storages
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class elaborator
{
public:
	virtual ~elaborator()	{}

protected:
	virtual void move(T* target, const T* source, size_t cnt) = 0;
	virtual void copy(T* target, const T* source, size_t cnt) = 0;
	virtual int cmp(const T* a, const T* b, size_t cnt) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
// elaborator for simple data types
// therefore can use memcopy/move/cmp
///////////////////////////////////////////////////////////////////////////////
template<class T> class elaborator_st : public elaborator<T>
{
public:
	virtual ~elaborator_st()	{}

protected:
	virtual void move(T* target, const T* source, size_t cnt)
	{	// simple type mover, no checks performed
		memmove(target, source, cnt*sizeof(T));
	}
	virtual void copy(T* target, const T* source, size_t cnt)
	{	// simple type copy, no checks performed
		memcpy(target, source, cnt*sizeof(T));
	}
	virtual int cmp(const T* a, const T* b, size_t cnt) const
	{	// simple type compare, no checks performed
		return memcmp(a,b,cnt*sizeof(T));
	}
};

///////////////////////////////////////////////////////////////////////////////
// elaborator for complex data types
// using assignment and compare operators ('=', '==' and '<' mandatory)
///////////////////////////////////////////////////////////////////////////////
template<class T> class elaborator_ct : public elaborator<T>
{
public:
	virtual ~elaborator_ct()	{}

protected:
//!! bloat
	virtual void move(T* target, const T* source, size_t cnt)
	{	
		if(target>source)
		{	// last to first run
			T* epp=target;
			target+=cnt-1;
			source+=cnt-1;
			while( target>=epp ) *target-- = source--;
		}
		else if(target<source)
		{	// first to last run
			T* epp=target+cnt;
			while( target< epp ) *target++ =*source++;
		}
		//else identical; no move necessary
	}
	virtual void copy(T* target, const T* source, size_t cnt)
	{	
		T* epp=target+cnt;
		while( target<epp ) *target++ =*source++;
	}
//!! bloat
	virtual int cmp(const T* a, const T* b, size_t cnt) const
	{	
		T* epp=a+cnt;
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
};






///////////////////////////////////////////////////////////////////////////////
// basic allocator interface
// for allocating memory in different ways
// defines basic functions of a storage element
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class allocator : public global, public noncopyable
{
public:
	virtual ~allocator()	{}
public:
	virtual operator const T*() const =0;
	virtual size_t length()	const =0;
};


///////////////////////////////////////////////////////////////////////////////
// allocators for write buffers
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class allocator_w : public allocator<T>
{
protected:
	T* cBuf;
	T* cEnd;
	T* cPtr;

	// std construct/destruct
	allocator_w()	: cBuf(NULL), cEnd(NULL), cPtr(NULL)				{}
	allocator_w(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cPtr(buf)	{}
	virtual ~allocator_w()	{ cBuf=cEnd=cPtr=NULL; }

	virtual bool checkwrite(size_t addsize)	=0;
public:
	virtual operator const T*() const	{ return this->cBuf; }
	virtual size_t length()	const		{ return (this->cPtr-this->cBuf); }
};



///////////////////////////////////////////////////////////////////////////////
// dynamic write-buffer allocator
// creates/reallocates storage on its own
//
// size is calculated with one element in advance
// so it can be used for cstring allocation
///////////////////////////////////////////////////////////////////////////////
template < class T=char, class E=elaborator_st<T> > class allocator_w_dy : public allocator_w<T>, public E
{
protected:
	allocator_w_dy() : allocator_w<T>()			{}
	allocator_w_dy(size_t sz) : allocator_w<T>(){ checkwrite(sz); }
	allocator_w_dy(T* buf, size_t sz)			{}	// no implementation here
	virtual ~allocator_w_dy()					{ if(this->cBuf) delete[] this->cBuf; }
//!! bloat
	virtual bool checkwrite(size_t addsize)
	{
		size_t sz = memquantize( this->cPtr-this->cBuf + addsize + 1 );

		if( this->cPtr+addsize >= this->cEnd ||
			(this->cBuf+1024 < this->cEnd && this->cBuf+4*sz <= this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quarter of the buffer for >1k elements
			T *tmp = new T[sz];
			if(this->cBuf)
			{
				this->copy(tmp, this->cBuf, this->cPtr-this->cBuf);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cPtr = tmp+(this->cPtr-this->cBuf);
			this->cBuf = tmp;
		}
		return (this->cEnd > this->cBuf);
	}
};


///////////////////////////////////////////////////////////////////////////////
// static write-buffer is using an external buffer for writing
// does not resize but just returns false on checkwrite
// size is calculated with one element in advance
// so it can be used for cstring allocation
///////////////////////////////////////////////////////////////////////////////
template < class T=char, class E=elaborator_st<T> > class allocator_w_st : public allocator_w<T>, public E
{
protected:
	allocator_w_st()			{}			// no implementation here
	allocator_w_st(size_t sz)	{}			// no implementation here
	allocator_w_st(T* buf, size_t sz) : allocator_w<T>(buf, sz)	{}
	virtual ~allocator_w_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		return ( this->cPtr && this->cPtr +addsize < this->cEnd );
	}
};



///////////////////////////////////////////////////////////////////////////////
// allocators for read/write buffers
// ie. for FIFO implementations
///////////////////////////////////////////////////////////////////////////////
template <class T=unsigned char> class allocator_rw : public allocator<T>
{
protected:
	T* cBuf;
	T* cEnd;
	T* cRpp;
	T* cWpp;

	// std construct/destruct
	allocator_rw()	: cBuf(NULL), cEnd(NULL), cRpp(NULL), cWpp(NULL)				{}
	allocator_rw(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cRpp(buf), cWpp(buf)	{}
	virtual ~allocator_rw()	{ cBuf=cEnd=cRpp=cWpp=NULL; }

	virtual bool checkwrite(size_t addsize)	=0;
	virtual bool checkread(size_t addsize) const =0;
public:
	virtual operator const T*() const	{ return this->cRpp; }
	virtual size_t length()	const		{ return (this->cWpp-this->cRpp); }
};
///////////////////////////////////////////////////////////////////////////////
// dynamic read/write-buffer allocator
// creates/reallocates storage on its own
///////////////////////////////////////////////////////////////////////////////
template < class T=unsigned char, class E=elaborator_st<T> > class allocator_rw_dy : public allocator_rw<T>, public E
{
protected:
	allocator_rw_dy() : allocator_rw<T>()			{}
	allocator_rw_dy(size_t sz) : allocator_rw<T>()	{ checkwrite(sz); }
	allocator_rw_dy(T* buf, size_t sz)				{}
	virtual ~allocator_rw_dy()				{ if(this->cBuf) delete[] this->cBuf; }
//!! bloat
	virtual bool checkwrite(size_t addsize)
	{
		size_t sz = memquantize( this->cWpp-this->cRpp + addsize );

		if( this->cBuf+sz > this->cEnd  ||
			(this->cBuf+8192 < this->cEnd && this->cBuf+4*sz <= this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quarter of the buffer for >8k buffers
			T *tmp = new T[sz];
			if(this->cBuf)
			{
				this->copy(tmp, this->cRpp, this->cWpp-this->cRpp);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+(this->cWpp-this->cRpp);
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->wpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			this->move(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->cBuf+(this->cWpp-this->cRpp);
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
// static read/write-buffer is using an external buffer for writing
///////////////////////////////////////////////////////////////////////////////
template < class T=unsigned char, class E=elaborator_st<T> > class allocator_rw_st : public allocator_rw<T>, public E
{
protected:
	allocator_rw_st()			{}
	allocator_rw_st(size_t sz)	{}
	allocator_rw_st(T* buf, size_t sz) : allocator_rw<T>(buf, sz)	{}
protected:
	virtual ~allocator_rw_st()				{}
//!! bloat
	virtual bool checkwrite(size_t addsize)
	{
		if( this->cWpp+addsize >= this->cEnd && this->cBuf < this->cRpp )
		{	// move the current buffer data when necessary and possible 
			this->move(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->cBuf+(this->cWpp-this->cRpp);
			this->cRpp = this->cBuf;
		}
		return ( this->cWpp && this->cWpp + addsize < this->cEnd );
	}
	virtual bool checkread(size_t addsize) const
	{
		return ( this->cRpp && this->cRpp + addsize <=this->cWpp );
	}
};




///////////////////////////////////////////////////////////////////////////////
// allocators for read buffers with additional scanning
// ie. for the parser
///////////////////////////////////////////////////////////////////////////////
template <class T=unsigned char> class allocator_r : public allocator<T>
{
protected:
	T* cBuf;
	T* cEnd;
	T* cRpp;
	T* cScn;
	T* cWpp;

	// std construct/destruct
	allocator_r()	: cBuf(NULL), cEnd(NULL), cRpp(NULL), cScn(NULL), cWpp(NULL)				{}
	allocator_r(T* buf, size_t sz) : cBuf(buf), cEnd(buf+sz), cRpp(buf), cScn(buf), cWpp(buf+z)	{}
	virtual ~allocator_r()	{ cBuf=cEnd=cRpp=cScn=cWpp=NULL; }

	// default read function, reads max sz elements to buf and returns the actual count of read elements
	virtual size_t readdata(T*buf, size_t sz) =0;
	virtual bool checkwrite(size_t addsize)	=0;
	virtual bool checkread(size_t addsize) =0;
public:
	virtual operator const T*() const	{ return this->cRpp; }
	virtual size_t length()	const		{ return (this->cWpp-this->cRpp); }
};

///////////////////////////////////////////////////////////////////////////////
// dynamic read-buffer allocator
///////////////////////////////////////////////////////////////////////////////
template < class T=unsigned char, class E=elaborator_st<T> > class allocator_r_dy : public allocator_r<T>, public E
{
protected:
	allocator_r_dy() : allocator_r<T>()				{  }
	allocator_r_dy(size_t sz) : allocator_r<T>()	{ this->checkwrite(sz); }
	virtual ~allocator_r_dy()						{ if(this->cBuf) delete[] this->cBuf; }
//!! bloat
	virtual bool checkwrite(size_t addsize)
	{
		size_t sz = memquantize( this->cWpp-this->cRpp + addsize );

		if( this->cBuf+sz > this->cEnd  ||
			(this->cBuf+8192 < this->cEnd && this->cBuf+4*sz <= this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quarter of the buffer for >8k buffers
			T *tmp = new T[sz];
			if(this->cBuf)
			{
				this->copy(tmp, this->cRpp, this->cWpp-this->cRpp);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+(this->cWpp-this->cRpp);
			this->cScn = tmp+(this->cScn-this->cRpp);
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->cWpp+addsize > this->cEnd )
		{	// moving the current buffer data is sufficient
			this->move(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
			this->cWpp = this->cBuf+(this->cWpp-this->cRpp);
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
// dynamic file read-buffer
///////////////////////////////////////////////////////////////////////////////
template <class T=unsigned char> class allocator_file : public allocator_r_dy< T, elaborator_st<T> >
{
	FILE *cFile;
public:
	allocator_file() : allocator_r_dy< T, elaborator_st<T> >(1024), cFile(NULL)
	{  }
	allocator_file(const char* name) : allocator_r_dy< T, elaborator_st<T> >(1024), cFile(NULL)
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
			checkread(1);
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



#endif//__BASEMEMORY_H__

