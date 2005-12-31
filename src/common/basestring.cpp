
#include "basestring.h"

#include <wchar.h>





///////////////////////////////////////////////////////////////////////////////
// conversion overloads to change signed types to the appropriate unsigned
//
///////////////////////////////////////////////////////////////////////////////
inline size_t to_unsigned(char t)
{
	return (unsigned char)(t);
}
inline size_t to_unsigned(unsigned char t)
{
	return (unsigned char)(t);
}
// UCT2
inline size_t to_unsigned(short t)
{
	return (unsigned short)(t);
}
inline size_t to_unsigned(unsigned short t)
{
	return (unsigned short)(t);
}
// UCT4
inline size_t to_unsigned(sint32 t)
{
	return (uint32)(t);
}
inline size_t to_unsigned(uint32 t)
{
	return (uint32)(t);
}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// string conversion/checking
// with specified implementations for certain types
///////////////////////////////////////////////////////////////////////////////

template <class T> inline T locase(T c) { return ::tolower( to_unsigned(c) ); }
inline char locase(char c) { if (c >= 'A' && c <= 'Z') return char(c + 32); return c; }
inline wchar_t locase(wchar_t c) { return ::towlower( to_unsigned(c) ); }

template <class T> inline T upcase(T c) { return ::toupper( to_unsigned(c) ); }
// implementation for char (and wchar)
inline char upcase(char c) { if (c >= 'a' && c <= 'z') return char(c - 32); return c; }
inline wchar_t upcase(wchar_t c) { return ::towupper( to_unsigned(c) ); }

template <class T> inline bool _isspace(T c) { return ::isspace( to_unsigned(c) ); }
// implementation for char (and wchar)
inline bool _isspace(char c) { return (c==0x20) || (c>=0x09 && c<=0x0D) || (c==(char)0xA0); }
inline bool _isspace(wchar_t c) { return 0!=::iswspace( to_unsigned(c) ); }


#define CONSTSTRING(x) x
//#define CONSTSTRING(x) L##x		// unicode string marker


uchar hex4(char c) 
{
    if( c>='a' && c<='f')
        return uchar(c - 'a' + 10);
    else if( c >= 'A' && c <= 'F')
        return uchar(c - 'A' + 10);
    else if( c >= '0' && c <= '9')
        return uchar(c - '0');
	else
		return 0;
}
char hexchar(uchar c)
{
	c &= 0x0f;
    if( c < 10 )
        return char(c + '0');
    else 
        return char(c - 10 + 'a');
}


static const char* _itobase(sint64 value, char* buf, size_t base, size_t& len, bool _signed)
{
    // internal conversion routine: converts the value to a string 
    // at the end of the buffer and returns a pointer to the first
    // character. this is to get rid of copying the string to the 
    // beginning of the buffer, since finally the string is supposed 
    // to be copied to a dynamic string in itostring(). the buffer 
    // must be at least 65 bytes long.

    static const char digits[65] = 
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    const char* pdigits = (base>36) ? digits : digits+2;
    size_t i = 64;
    buf[i] = 0;
    bool neg = false;
    uint64 v = value;
    if (_signed && base==10 && value < 0)
    {
        v = -value;
        // since we can't handle the lowest signed 64-bit value, we just
        // return a built-in string.
        if(sint64(v) < 0)   // the LLONG_MIN negated results in the same value
        {
            len = 20;
            return "-9223372036854775808";
        }
        neg = true;
    }
    do
    {
        buf[--i] = pdigits[uint(v % base)];
        v /= base;
    } while (v > 0);

    if (neg)
        buf[--i] = '-';

    len = 64 - i;
    return buf+i;
}

sint64 stringtoi(const char* p)
{
	sint64 r = 0;
	bool sign=false;
	if( p != 0 &&  *p != 0)
	{
		sint64 t;
		while( _isspace(*p) ) p++;
		sign = (*p=='-');
		if(sign || *p=='+') p++;
		// allow whitespace between sign and number
		while( _isspace(*p) ) p++;
		do
		{
			char c = *p++;
			if (c < '0' || c > '9')	// invalid character
				break;
			t = r * 10;
			if (t < r)	// overflow
				return (sign)?INT64_MIN:INT64_MAX;
			t += c - '0';
			if (t < r)	// overflow
				return (sign)?INT64_MIN:INT64_MAX;
			r = t;
		} while (*p != 0);
	}
    return (sign)?(-r):(r);
}

uint64 stringtoue(const char* str, size_t base)
{
	uint64 result = 0;

    if( str != 0 && *str != 0 && base >= 2 && base <= 64)
	{
		uint64 t;
	    const char* p = str;
		do
		{
			int c = *p++;
			if (c >= 'a')
			{
				// for the numeration bases that use '.', '/', digits and
				// uppercase letters the letter case is insignificant.
				if (base <= 38)
					c -= 'a' - '9' - 1;
				else  // others use both upper and lower case letters
					c -= ('a' - 'Z' - 1) + ('A' - '9' - 1);
			}
			else if (c > 'Z')
				break;
			else if (c >= 'A')
				c -= 'A' - '9' - 1;
			else if (c > '9')
				break;

			c -= (base > 36) ? '.' : '0';
			if (c < 0 || (size_t)c >= base)
				break;

			t = result * base;
			if (t / base != result)
				return INT64_MAX;
			result = t;
			t = result + uint(c);
			if (t < result)
				return INT64_MAX;
			result = t;
		} while (*p != 0);
	}
    return result;
}


sint64 stringtoie(const char* str)
{
    if(str==0 || *str==0)
        return 0;
    bool neg = *str == '-';
    uint64 result = stringtoue(str + int(neg), 10);
    if (result > (uint64(INT64_MAX) + uint(neg)))
        return (neg)?INT64_MIN:INT64_MAX;
    if (neg)
        return -sint64(result);
    else
        return  sint64(result);
}












size_t memquantize(size_t sz)
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


//////////////////////////////////////////////////////////////////////////
// basic allocator interface
//////////////////////////////////////////////////////////////////////////
template <class T=char> class allocator : public global, public noncopyable
{
public:
	virtual ~allocator()	{}
public:
	virtual operator const T*() const =0;
	virtual size_t length()	const =0;
};

//////////////////////////////////////////////////////////////////////////
// basic elaborator interface
//////////////////////////////////////////////////////////////////////////
template <class T=char> class elaborator
{
public:
	virtual ~elaborator()	{}

protected:
	virtual void move(T* target, const T* source, size_t cnt) = 0;
	virtual void copy(T* target, const T* source, size_t cnt) = 0;
	virtual int cmp(const T* a, const T* b, size_t cnt) const = 0;
};


//////////////////////////////////////////////////////////////////////////
// elaborator for simple data types
//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
// elaborator for complex data types
//////////////////////////////////////////////////////////////////////////
template<class T> class elaborator_ct : public elaborator<T>
{
public:
	virtual ~elaborator_ct()	{}

protected:
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
	virtual int cmp(const T* a, const T* b, size_t cnt) const
	{	
		T* epp=a+cnt;
		while( a<epp )
		{
			if( *a == *b )
				;
			if( *a < *b )
				return -1;
			else
				return  1;
			a++;
			b++;
		}
		return 0;
	}
};


//////////////////////////////////////////////////////////////////////////
// allocators for write buffers
//
// dynamic one creates on its own
// static is using an external buffer for writing
//
// size is calculated with one element in advance
// so it can be used for cstring allocation
//////////////////////////////////////////////////////////////////////////
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
template <class T=char> class allocator_w_st : public allocator_w<T>
{
protected:
	allocator_w_st(T* buf, size_t sz) : allocator_w<T>(buf, sz)	{}
	allocator_w_st();						// no implementation here
	allocator_w_st(size_t sz);				// no implementation here
	virtual ~allocator_w_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		return ( this->cPtr && this->cPtr +addsize < this->cEnd );
	}
};

///////////////////////////////////////////////////////////////////////////////
template <class T=char> class allocator_w_dy : public allocator_w<T>
{
protected:
	allocator_w_dy(T* buf, size_t sz);		// no implementation here
	allocator_w_dy() : allocator_w<T>()			{}
	allocator_w_dy(size_t sz) : allocator_w<T>(){ checkwrite(sz); }
	virtual ~allocator_w_dy()				{ if(this->cBuf) delete[] this->cBuf; }
	virtual bool checkwrite(size_t addsize)
	{
		size_t sz = memquantize( this->cPtr-this->cBuf + addsize + 1 );

		if( this->cPtr+addsize >= this->cEnd ||
			(this->cBuf+1024 < this->cEnd && this->cBuf+4*sz <= this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer for >1k strings
			char *tmp = new char[sz];
			if(this->cBuf)
			{
				memcpy(tmp, this->cBuf, this->cPtr-this->cBuf);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cPtr = tmp+(this->cPtr-this->cBuf);
			this->cBuf = tmp;
		}
		return (this->cEnd > this->cBuf);
	}
};



//////////////////////////////////////////////////////////////////////////
// allocators for read/write buffers
//
// dynamic one creates on its own
// static is using an external buffer for writing
//
//////////////////////////////////////////////////////////////////////////
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
template <class T=unsigned char> class allocator_rw_st : public allocator_rw<T>
{
protected:
	allocator_rw_st(T* buf, size_t sz) : allocator_rw<T>(buf, sz)	{}
private:
	allocator_rw_st();						// no implementation here
	allocator_rw_st(size_t sz);				// no implementation here
protected:
	virtual ~allocator_rw_st()				{}
	virtual bool checkwrite(size_t addsize)
	{
		if( this->cWpp+addsize >= this->cEnd && this->cBuf < this->cRpp )
		{	// move the current buffer data when necessary and possible 
			memmove(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
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
template <class T=unsigned char> class allocator_rw_dy : public allocator_rw<T>
{
private:
	allocator_rw_dy(T* buf, size_t sz);		// no implementation here
protected:
	allocator_rw_dy() : allocator_rw<T>()		{}
	allocator_rw_dy(size_t sz) : allocator_rw<T>(){ checkwrite(sz); }
	virtual ~allocator_rw_dy()				{ if(this->cBuf) delete[] this->cBuf; }
	virtual bool checkwrite(size_t addsize)
	{
		size_t sz = memquantize( this->cPtr-this->cBuf + addsize );

		if( this->cPtr+addsize > this->cEnd  ||
			(this->cBuf+8092 < this->cEnd && this->cBuf+4*sz <= this->cEnd))
		{	// allocate new
			// enlarge when not fit
			// shrink when using less than a quater of the buffer for >8k buffers
			char *tmp = new char[sz];
			if(this->cBuf)
			{
				memcpy(tmp, this->cRpp, this->cWpp-this->cRpp);
				delete[] this->cBuf;
			}
			this->cEnd = tmp+sz;
			this->cWpp = tmp+(this->cWpp-this->cRpp);
			this->cRpp = tmp;
			this->cBuf = tmp;
		}
		else if( this->wpp+addsize > this->end )
		{	// moving the current buffer data is sufficient
			memmove(this->cBuf, this->cRpp, this->cWpp-this->cRpp);
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






//////////////////////////////////////////////////////////////////////////
// predeclaration
//////////////////////////////////////////////////////////////////////////

template <class T> class basestring;		// dynamic buffer without smart pointer
template <class T> class staticstring;		// external static buffer
template <class T> class string;			// dynamic buffer with smart pointer, copy-on-write
template <class T> class globalstring;		// dynamic buffer with smart pointer, copy-on-write
template <class T> class substring;			// contains pointer to string, implements allocator interface
template <class T> class patternstring;		// string derived, implements booyer-moore search




//////////////////////////////////////////////////////////////////////////
// basic string interface
//////////////////////////////////////////////////////////////////////////


template <class T=char> class stringinterface : public allocator<T>
{
public:
	stringinterface<T>()			{}
	virtual ~stringinterface<T>()	{}

//	virtual operator const T*() const =0;
	virtual const T* c_str() const =0;
//	virtual size_t length()	const =0;
};



//////////////////////////////////////////////////////////////////////////
// basic string implementation
//////////////////////////////////////////////////////////////////////////
template < class T=char, class A=allocator_w_dy<T>, class E=elaborator_st<T> >
class TString : public A, public E, public stringinterface<T>
{
public:
	TString<T,A,E>() : A()								{}
	TString<T,A,E>(size_t sz) : A(sz)					{}
	TString<T,A,E>(T* buf, size_t sz) : A(buf, sz)		{}
	virtual ~TString<T,A,E>()							{}

	///////////////////////////////////////////////////////////////////////////
	// M$ VisualC does not allow template generated copy/assignment operators
	// but always generating defaults if not explicitely defined
	// mixing template and explicit definition does also not work because
	// it "detects" ambiguities in this case
//	template <class A> TString<T,A,E>(const TString<T, A>& b) : alloc(b.length())
//	{
//		*this<<b;
//	}
	// declare standard copy constructor
	TString<T,A,E>(const TString<T,A,E>& b) : A(b.length())
	{
		*this<<b;
	}
	// declare copy of base class types
	TString<T,A,E>(const stringinterface<T>& t) : A(t.length())
	{
		*this<<t;
	}

	///////////////////////////////////////////////////////////////////////////
	// standard functions
	///////////////////////////////////////////////////////////////////////////
	void clear()			{ this->cPtr=this->cBuf; if(this->cPtr) *this->cPtr=0; }
	bool is_empty() const	{ return this->cPtr==this->cBuf; }
	void truncate(size_t sz)
	{
		if( this->cBuf+sz < this->cPtr )
		{	// truncate
			this->cPtr = this->cBuf+sz;
			*this->cPtr = 0;
		}
		else
		{	// expand by padding spaces
			if( this->checkwrite( sz-(this->cPtr-this->cBuf) ) )
			{
				while( this->cBuf+sz > this->cPtr )
					*this->cPtr++ = (T)(' ');
				*this->cPtr = 0;
			}
		}
	}
	void truncate(size_t ix, size_t sz)
	{
		if(ix==0)
		{	// normal truncate
			truncate(sz);
		}
		else if( this->cBuf+ix > this->cPtr )
		{	// cut all off
			this->cPtr = this->cBuf;
			*this->cPtr = 0;
		}
		else if( this->cBuf+ix+sz <= this->cPtr )  
		{	// move and truncate
			this->move(this->cBuf, this->cBuf+ix, sz);
			this->cPtr = this->cBuf+sz;
			*this->cPtr = 0;
		}
		else
		{	// move and expand
			this->move(this->cBuf, this->cBuf+ix, this->cPtr-this->cBuf-ix);
			this->cPtr -= ix;
			if( this->checkwrite( sz-(this->cPtr-this->cBuf) ) )
			{
				while( this->cBuf+sz > this->cPtr )
					*this->cPtr++ = (T)(' ');
			}
			*this->cPtr = 0;
		}
	}
	void removeindex(size_t inx, size_t len)
	{
		if( this->cBuf+inx < this->cPtr )
		{
			if(this->cBuf+inx+len > this->cPtr)	len = this->cPtr-this->cBuf-inx;
			this->move(this->cBuf+inx, this->cBuf+inx+len, this->cPtr-this->cBuf-inx-len);
			this->cPtr -= len;
			*this->cPtr = 0;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// base class functions
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if(!this->cBuf && const_cast<TString<T,A,E>*>(this)->checkwrite(1))
			*this->cPtr=0; 
		return this->cBuf; 
	}
	virtual operator const T*() const	{ return this->c_str(); }
	virtual size_t length()	const		{ return this->A::length(); }

	///////////////////////////////////////////////////////////////////////////
	// array access
	///////////////////////////////////////////////////////////////////////////
	const T& operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		if( this->cBuf+inx > this->cPtr )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("String out of bound");
#else
			// out of bound
			static T dummy=0;
			return dummy;
#endif
		}
#endif
		return this->cBuf[inx];
	}
	T& operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		if( this->cBuf+inx > this->cPtr )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("String out of bound");
#else
			// out of bound
			static T dummy;
			return dummy=0;
#endif
		}
#endif
		return this->cBuf[inx];
	}
	const T& operator[](int inx) const
	{
		return this->operator[]( (size_t)inx );
	}
	T& operator[](int inx)
	{
		return this->operator[]( (size_t)inx );
	}
	///////////////////////////////////////////////////////////////////////////
	// assignment function
	///////////////////////////////////////////////////////////////////////////
	template<class X> TString<T,A,E>& assign(const X& t)
	{
		this->cPtr = this->cBuf;
		return this->operator<<(t);
	}
	TString<T,A,E>& assign(const T* c, size_t slen)
	{
		this->cPtr = this->cBuf;
		if( c && this->checkwrite(slen) )
		{
			const T* epp=this->cPtr+slen;
			while( *c && this->cPtr<epp )
				*this->cPtr++ = *c++;
		}
		if(*this->cPtr) *this->cPtr=0;
		return *this;
	}
	TString<T,A,E>& assign(const T ch, size_t slen=1)
	{
		this->cPtr = this->cBuf;
		if(ch)
		{	
			if( this->checkwrite(slen) )
			{
				const T* epp=this->cPtr+slen;
				while( this->cPtr < epp )
					*this->cPtr++ = ch;
			}
		}
		if(this->cPtr) *this->cPtr=0;
		return *this;
	}
	TString<T,A,E>& assign(const T* c1, size_t len1, const T* c2, size_t len2)
	{
		if( !c1 || len1 <= 0 )
			assign(c2, len2);
		else if ( !c2 || len2 <= 0 )
			assign(c1, len1);
		else if( this->checkwrite(len1 + len2) )
		{
			this->cPtr = this->cBuf;

			const T* epp=this->cBuf+len1;
			while( *c1 && this->cPtr < epp )
				*this->cPtr++ = *c1++;

			epp=this->cPtr+len2;
			while( *c2 && this->cPtr < epp )
				*this->cPtr++ = *c2++;

			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A,E>& assign_tolower(const T* c, size_t slen=~0)
	{
		this->cPtr = this->cBuf;
		if(c)
		{
			const T* epp = (slen==~0) ? (const T*)(~((size_t)0)) : epp=this->cPtr+slen;
			while(*c && this->cPtr<epp && this->checkwrite(1) )
				*this->cPtr++ = locase(*c++);
		}
		if(this->cPtr) *this->cPtr=0;
		return *this;
	}
	TString<T,A,E>& assign_toupper(const T* c, size_t slen=~0)
	{
		this->cPtr = this->cBuf;
		if(c)
		{
			const T* epp = (slen==~0) ? ~0 : c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cPtr++ = upcase(*c++);
		}
		if(this->cPtr) *this->cPtr=0;
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// append function
	///////////////////////////////////////////////////////////////////////////
	template<class X> TString<T,A,E>& append(const X& t)
	{
		return this->operator<<(t);
	}
	/////////////////////////////////////////////////////////////////
	// append with length
	/////////////////////////////////////////////////////////////////
	TString<T,A,E>& append(const stringinterface<T>& t, size_t slen)
	{
		if( slen > t.length() )
			slen = t.length();
		if(slen)
		{
			if( this->checkwrite( slen ) )
			{
				this->copy(this->cPtr, t, slen);
				this->cPtr += slen;
				*this->cPtr=0;
			}
		}
		return *this;
	}
	TString<T,A,E>& append(const T* c, size_t slen)
	{
		if(c)
		{
			const T* epp = this->cPtr+slen;
			while(*c && this->cPtr<epp && this->checkwrite(1) )
				*this->cPtr++ = *c++;
			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A,E>& append(const T ch, size_t slen)
	{
		if(ch) // dont append a eos
		{
			if( this->checkwrite( slen ) )
			{
				T* epp = this->cPtr+slen;
				while(this->cPtr<epp)
					*this->cPtr++ = ch;
				*this->cPtr = 0;
			}
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// insert
	/////////////////////////////////////////////////////////////////
	TString<T,A,E>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~0)
	{
		if( pos > this->length() )
			return this->append(t, slen);
		else
		{
			if( slen > t.length() )
				slen = t.length();
			if( this->checkwrite( slen ) )
			{
				this->move(this->cBuf+pos+slen, this->cBuf+pos, this->cPtr-this->cBuf-pos);
				this->copy(this->cBuf+pos, t, slen);

				this->cPtr += slen;
				*this->cPtr = 0;
			}
		}
		return *this;
	}
	TString<T,A,E>& insert(size_t pos, const T* c, size_t slen=~0)
	{
		if(c)
		{
			if( pos > this->length() )
				return this->append(c, slen);
			else
			{
				const T* ep =c;
				while(*ep) ep++;
				if( slen > (size_t)(ep-c) )
					slen = ep-c;
				if( this->checkwrite( slen ) )
				{
					this->move(this->cBuf+pos+slen, this->cBuf+pos, this->cPtr-this->cBuf-pos);
					this->copy(this->cBuf+pos, c, this->cPtr-this->cBuf-pos);

					this->cPtr += slen;
					*this->cPtr = 0;
				}
			}
		}
		return *this;
	}
	TString<T,A,E>& insert(size_t pos, const T ch, size_t slen=1)
	{
		if(ch) // dont append a eos
		{
			if( pos > this->length() )
				return this->append(ch, slen);
			else
			{
				if( this->checkwrite( slen ) )
				{
					T* ipp = this->cBuf+pos;
					T* epp = this->cBuf+pos+slen;
					this->move(epp, ipp, this->cPtr-this->cBuf-pos);
					while(ipp<epp)
						*ipp++ = ch;

					this->cPtr += slen;
					*this->cPtr = 0;
				}
			}
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// replace
	/////////////////////////////////////////////////////////////////
	TString<T,A,E>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~0)
	{
		if( pos > this->length() )
			return this->append(t, slen);
		else
		{
			if( pos+tlen > this->length() )
				tlen = this->length()-pos;
			if( slen > t.length() )
				slen = t.length();
			if( slen<=tlen || this->checkwrite( slen-tlen ) )
			{
				this->move(this->cBuf+pos+slen, this->cBuf+pos+tlen, this->cPtr-this->cBuf-pos-tlen);
				this->copy(this->cBuf+pos, t, slen);

				this->cPtr += slen-tlen;
				*this->cPtr = 0;
			}
		}
		return *this;
	}
	TString<T,A,E>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~0)
	{
		if(c)
		{
			if( pos > this->length() )
				return this->append(c, slen);
			else
			{
				if( pos+tlen > this->length() )
					tlen = this->length()-pos;

				const T* ep =c;
				while(*ep) ep++;
				if( slen > (size_t)(ep-c) )
					slen = ep-c;
				
				if( slen<=tlen || this->checkwrite( slen-tlen ) )
				{
					this->move(this->cBuf+pos+slen, this->cBuf+pos+tlen, this->cPtr-this->cBuf-pos-tlen);
					this->copy(this->cBuf+pos, c, slen);

					this->cPtr += slen-tlen;
					*this->cPtr = 0;
				}
			}
		}
		return *this;
	}
	TString<T,A,E>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1)
	{
		if(ch) // dont append a eos
		{
			if( pos > this->length() )
				return this->append(ch, slen);
			else
			{
				if( pos+tlen > this->length() )
					tlen = this->length()-pos;
				
				if( slen<=tlen || this->checkwrite( slen-tlen ) )
				{
					T* ipp = this->cBuf+pos;
					T* epp = this->cBuf+pos+slen;
					this->move(epp, ipp+tlen, this->cPtr-this->cBuf-pos-tlen);
					while(ipp<epp)
						*ipp++ = ch;

					this->cPtr += slen-tlen;
					*this->cPtr = 0;
				}
			}
		}
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// assignment operators
	///////////////////////////////////////////////////////////////////////////
// M$ VisualC does not allow template generated copy/assignment operators
// but always generating defaults if not explicitely defines
// mixing template and explicit definition does also not work because
// it "detects" ambiguities in this case

//	template <class X> const TString<T,A,E>& operator =(const X& t)
//	{
//		this->cPtr = this->cBuf;
//		return *this<<t;
//	}

	// only default type assignment usable
	const TString<T,A,E>& operator=(const TString<T,A,E>& t)	{ this->cPtr = this->cBuf; return this->operator<<(t); }

	const TString<T,A,E>& operator=(const stringinterface<T>& t){ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const char* t)			{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const char t)			{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const int t)			{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const unsigned int t)	{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const long t)			{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const unsigned long t)	{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const ipaddress t)		{ this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A,E>& operator=(const double t)			{ this->cPtr = this->cBuf; return this->operator<<(t); }

	///////////////////////////////////////////////////////////////////////////
	// add-assignment operators
	///////////////////////////////////////////////////////////////////////////
	template <class X> TString<T,A,E>& operator +=(const X& t)
	{
		return this->operator<<(t);
	}
	///////////////////////////////////////////////////////////////////////////
	// add operators
	// always return dynamic allocates buffers
	///////////////////////////////////////////////////////////////////////////
	template <class X> string<T> operator +(const X& t)
	{
		return ( string<T>(*this)<< t );
	}
	// add operators 
	// for left hand side operators, only implement these, 
	// the others would confuse standard operators
	friend string<T> operator +(const T* t, const TString<T,A,E>& str)
	{
		return ( string<T>(t) << str );
	}
	friend string<T> operator +(const T t, const TString<T,A,E>& str)
	{
		return ( string<T>(t) << str );
	}

	///////////////////////////////////////////////////////////////////////////
	// operator realisations for supported types
	///////////////////////////////////////////////////////////////////////////
	template <class X> TString<T,A,E>& operator <<(const TString<T,X>& sb)
	{
		if(sb.length())
		{
			if( this->checkwrite(sb.length()) )
			{
				this->copy(this->cPtr, sb, sb.length());
				this->cPtr += sb.length();
				*this->cPtr=0;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator<<(const stringinterface<T>& t)
	{
		if( t.length() )
		{
			if( this->checkwrite( t.length() ) )
			{
				this->copy(this->cPtr, t, t.length());
				this->cPtr += t.length();
				*this->cPtr=0;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const char* ip)
	{
		if(ip)
		{
			while( *ip && this->checkwrite(1) )
				*this->cPtr++ = *ip++;
			if(this->cPtr) *this->cPtr=0;
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const char ch)
	{
		if( ch && this->checkwrite(1) )
		{
			*this->cPtr++ = ch;
			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const int v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%i", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const unsigned int v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%u", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const long v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%li", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const unsigned long v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%lu", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const ipaddress ip)
	{
		ssize_t sz;
		this->checkwrite(16);
		while(1)
		{
			sz = ip.getstring(this->cPtr, this->cEnd-this->cPtr);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}
	TString<T,A,E>& operator <<(const double v)
	{
		int sz;
		while(1)
		{
			sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%lf", v);
			if(sz<0)
			{	// buffer not sufficient
				if( !this->checkwrite(1+2*(this->cEnd-this->cPtr)) )
				{	// give up
					// should throw something here
					*this->cPtr=0;
					break;
				}
			}
			else
			{
				this->cPtr += sz;
				break;
			}
		}
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// compare functions
	///////////////////////////////////////////////////////////////////////////
	template<class X> int compareTo(const TString<T,X> &s) const 
	{	
		if(s.length()>0 && this->length()>0) 
			return this->cmp(this->cBuf, s.cBuf, 1+this->length());
		else if(s.length()==0 && this->length()==0)
			return 0;
		else if(s.length()==0)
			return 1;
		else
			return -1;
	}
	int compareTo(const T c) const 
	{	
		if(this->length()==1)
			return *this->cBuf - c;
		else if(this->cBuf)
			return 1;
		else 
			return -1;
	}
	int compareTo(const T *c) const 
	{	
		if(c && this->length()>0)
			return this->cmp(this->cBuf, c, 1+this->length());
		else if((!c || *c==0) && this->length()==0)
			return 0;
		else if((!c || *c==0))
			return 1;
		else 
			return -1;
	}
	/////////////////////////////////////////////////////////////////
	// boolean operators
	/////////////////////////////////////////////////////////////////
	template<class X> bool operator ==(const TString<T,X> &s)	const { return (compareTo( s ) == 0); }
	template<class X> bool operator !=(const TString<T,X> &s)	const { return (compareTo( s ) != 0); }
	template<class X> bool operator <=(const TString<T,X> &s)	const { return (compareTo( s ) <= 0); }
	template<class X> bool operator >=(const TString<T,X> &s)	const { return (compareTo( s ) >= 0); }
	template<class X> bool operator < (const TString<T,X> &s)	const { return (compareTo( s ) <  0); }
	template<class X> bool operator > (const TString<T,X> &s)	const { return (compareTo( s ) >  0); }

	bool operator ==(const T c) 	const { return (compareTo( c ) == 0); }
	bool operator !=(const T c) 	const { return (compareTo( c ) != 0); }
	bool operator <=(const T c) 	const { return (compareTo( c ) <= 0); }
	bool operator >=(const T c) 	const { return (compareTo( c ) >= 0); }
	bool operator < (const T c) 	const { return (compareTo( c ) <  0); }
	bool operator > (const T c) 	const { return (compareTo( c ) >  0); }

	bool operator ==(const T *c) 	const { return (compareTo( c ) == 0); }
	bool operator !=(const T *c) 	const { return (compareTo( c ) != 0); }
	bool operator <=(const T *c) 	const { return (compareTo( c ) <= 0); }
	bool operator >=(const T *c) 	const { return (compareTo( c ) >= 0); }
	bool operator < (const T *c) 	const { return (compareTo( c ) <  0); }
	bool operator > (const T *c) 	const { return (compareTo( c ) >  0); }

	friend bool operator ==(const T *c, const TString<T,A,E> &s)	{return (s==c);}
	friend bool operator !=(const T *c, const TString<T,A,E> &s)	{return (s!=c);}
	friend bool operator <=(const T *c, const TString<T,A,E> &s)	{return (s>=c);}
	friend bool operator >=(const T *c, const TString<T,A,E> &s)	{return (s<=c);}
	friend bool operator < (const T *c, const TString<T,A,E> &s)	{return (s> c);}
	friend bool operator > (const T *c, const TString<T,A,E> &s)	{return (s< c);}

	/////////////////////////////////////////////////////////////////
	// change case
	/////////////////////////////////////////////////////////////////
	void tolower()
	{
		if( this->cBuf )
		{
			char* ipp = this->cBuf;
			while( ipp < this->cPtr )
			{
				*ipp = locase(*ipp);
				ipp++;
			}
		}
	}
	void toupper()
	{
		if( this->cBuf )
		{
			char* ipp = this->cBuf;
			while( ipp < this->cPtr )
			{
				*ipp = upcase(*ipp);
				ipp++;
			}
		}
	}
	/////////////////////////////////////////////////////////////////
	// Trim
	/////////////////////////////////////////////////////////////////
	void ltrim()
	{
		if( this->cBuf )
		{
			char* ipp = this->cBuf;
			while( ipp < this->cPtr && _isspace(*ipp) )
				ipp++;
			if(ipp!=this->cBuf)
			{
				this->move(this->cBuf, ipp, this->cPtr-ipp);
				*this->cPtr=0;
			}
		}
	}
	void rtrim()
	{
		if( this->cBuf )
		{
			char* ipp = this->cPtr-1;
			char* kpp = this->cPtr-1;
				
			while( ipp>this->cBuf && _isspace(*ipp) )
				ipp--;
			if( ipp != kpp )
			{
				this->cPtr=ipp+1;
				*this->cPtr=0;
			}
		}
	}
	void trim()
	{
		rtrim();
		ltrim();
	}

	/////////////////////////////////////////////////////////////////
	// Search function
	/////////////////////////////////////////////////////////////////
	bool find(const stringinterface<T>& pattern, size_t &startpos, bool ignorecase=false) const
	{	// modified boyer-moore search
		
		// table size is bound to 8bit values
		size_t	SkipTable[256]; 

		size_t i,k,sp;
		size_t len = pattern.length();

		// initialisation
		for (i=0; i<256; i++)
		{	// skip len+1 string chars if search char was not found
			// not exactly boyer-moore but fastens up the thing
			SkipTable[i] = len+1;
		}
		for (i=0; i<len; i++)
		{	// otherwise skip as only that many 
			// so the next char in the string fits with the one frome the pattern
			SkipTable[ (unsigned char) (pattern[i]) ] = len-i;
		}

		// run first to last / case sensitive
		sp=i=startpos;
		k=0; 
		while( i<this->length() && k<len )
		{
			if( ignorecase ? 
				(tolower((unsigned char)((*this)[i])) != tolower((unsigned char)(pattern[k]))) :
				((*this)[i] != pattern[k]) )
			{	// no match at that position, find the next starting pos
				sp += SkipTable[((unsigned char)((*this)[sp+len]))];
				i=sp;
				k=0;
			}
			else
			{	// check the next char
				i++;
				k++;
			}
		}
		if( k<len ) // not found
			return false;
		startpos = sp;
		return true;
	}
	
};


template <class T=char> class basestring : public TString< T, allocator_w_dy<T>, elaborator_st<T> >
{
public:
	basestring()
	{}
	basestring<T>(const basestring<T>& t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	basestring<T>(const stringinterface<T>& t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	basestring<T>(const char* t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	explicit basestring<T>(char t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	explicit basestring<T>(int t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	explicit basestring<T>(unsigned int t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}
	explicit basestring<T>(double t) : TString< T, allocator_w_dy<T>, elaborator_st<T> >()
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
	}

	virtual ~basestring()	{}

	template<class X> const basestring<T>& operator=(const X& t)
	{
		this->TString< T, allocator_w_dy<T>, elaborator_st<T> >::assign(t);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// add operators
	// always return dynamic allocates buffers
	///////////////////////////////////////////////////////////////////////////
	template <class X> basestring<T> operator +(const X& t)
	{
		return ( (basestring<T>(*this)) << t );
	}
	// add operators 
	// for left hand side operators, only implement these, 
	// the others would confuse standard operators
	friend basestring<T> operator +(const char* t, const basestring<T>& str)
	{
		return ( basestring<T>(t) << str );
	}
	friend basestring<T> operator +(const char t, const basestring<T>& str)
	{
		return ( basestring<T>(t) << str );
	}
};


template<class T=char> class staticstring : public TString< T, allocator_w_st<T>, elaborator_st<T> >
{
	staticstring<T>();
	staticstring<T>(const staticstring<T>&);
public:
	staticstring<T>(T* buf, size_t sz)
		: TString< T, allocator_w_st<T> >(buf, sz)
	{}
	virtual ~staticstring()	{}

	template<class X> const staticstring& operator=(const X& t)
	{
		this->TString< T, allocator_w_st<T> >::assign(t);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if(!this->cBuf && const_cast<staticstring<T>*>(this)->checkwrite(1))
			*this->cPtr=0; 
		return this->cBuf; 
	}

	///////////////////////////////////////////////////////////////////////////
	// add operators
	// always return dynamic allocates buffers
	///////////////////////////////////////////////////////////////////////////
	template <class X> string<T> operator +(const X& t)
	{
		return ((basestring<T>(*this))<< t);
	}
	// add operators 
	// for left hand side operators, only implement these, 
	// the others would confuse standard operators
	friend basestring<T> operator +(const char* t, const staticstring<T>& str)
	{
		return ( basestring<T>(t) << str );
	}
	friend basestring<T> operator +(const char t, const staticstring<T>& str)
	{
		return ( basestring<T>(t) << str );
	}
};





inline size_t strlen(const stringinterface<>& p)	{ return p.length(); }
inline size_t hstrlen(const stringinterface<>& p)	{ return p.length(); }
// some Unix systems do not accept NULL
inline size_t hstrlen(const char* p)		{ return p == NULL ? 0 : strlen(p); }


















template <class T=char> class string : public stringinterface<T>
{
	friend class substring<T>;
private:
	/////////////////////////////////////////////////////////////////
	// smart pointer base class
	/////////////////////////////////////////////////////////////////
	class ptrstring : public basestring<T>
	{
	private:
		friend class string<T>;

		// reference counter
		mutable unsigned int cRefCnt;

		// construction
		ptrstring() : cRefCnt(1)	{}
		ptrstring(const ptrstring& b) : basestring<T>(b), cRefCnt(1)	{}
		ptrstring(const basestring<T>& b) : basestring<T>(b), cRefCnt(1)	{}

	public:
		virtual ~ptrstring()	{}

	private:
		template<class X> const ptrstring& operator=(const X& t)
		{
			this->basestring<T>::operator=(t);
			return *this;
		}

		// access on the smart pointer
		ptrstring* aquire()
		{
#ifdef SINGLETHREAD
			cRefCnt++;
#else
			atomicincrement( &cRefCnt );
#endif
			return this;
		}
		ptrstring* release()
		{
#ifdef SINGLETHREAD
			if( ( --cRefCnt ) == 0 )
#else
			if( atomicdecrement( &cRefCnt ) == 0 )
#endif
				delete this;
			return NULL;
		}
	};
protected:
	/////////////////////////////////////////////////////////////////
	// the smart pointer itself
	/////////////////////////////////////////////////////////////////
	ptrstring *itsCounter;

	/////////////////////////////////////////////////////////////////
	// aquire/release the pointer
	/////////////////////////////////////////////////////////////////
	void acquire(const string<T>& r) throw()
	{	// check if not already pointing to the same object
		if( this->itsCounter != r.itsCounter ||  NULL==this->itsCounter )
		{	// save the current pointer
			ptrstring *old = this->itsCounter;
			// aquite and increment the given pointer
			if( NULL==r.itsCounter )
			{	// new empty counter to link both pointers
				const_cast<string<T>&>(r).itsCounter = new ptrstring;
			}
			this->itsCounter = r.itsCounter->aquire();
			// release the old thing
			if(old) old->release();
		}
	}
	void release()
	{	// decrement the count, clear the handle
		if(this->itsCounter) this->itsCounter = this->itsCounter->release();
	}

	/////////////////////////////////////////////////////////////////
	// virtual access functions to the smart pointer
	/////////////////////////////////////////////////////////////////
	// !!copy-on-write
	virtual const basestring<T>& readaccess() const	
	{ 
		this->checkpointer();
		return *this->itsCounter;
	}
	virtual       basestring<T>& writeaccess(bool keep=true)
	{
		if(!this->itsCounter)
			this->itsCounter = new ptrstring;
		else
			this->make_unique(keep);
		return *this->itsCounter;
	}
	void checkpointer() const
	{
		if(!this->itsCounter)
			const_cast< string* >(this)->itsCounter = new ptrstring;
		// no need to aquire, is done on reference creation
	}

	/////////////////////////////////////////////////////////////////
	// control of the smart pointer
	/////////////////////////////////////////////////////////////////
	bool isunique()	const throw()
	{
		return (this->itsCounter) ? (this->itsCounter->cRefCnt == 1):true;
	}
	bool make_unique(bool keep=true)	  throw()
	{
		if( !isunique() )
		{
			ptrstring *cnt = (itsCounter && keep) ? new ptrstring(*itsCounter) : new ptrstring;
			release();
			itsCounter = cnt;
		}
		return true;
	}
	virtual basestring<T>& operator*()	throw()				{ return this->writeaccess(); }
	virtual operator const basestring<T>&() const throw()	{ return this->readaccess();  }

public:
	/////////////////////////////////////////////////////////////////
	// construction/destruction
	/////////////////////////////////////////////////////////////////
	string<T>() : itsCounter(NULL)					{  }
	string<T>(const string& r) : itsCounter(NULL)	{ this->acquire(r); }
	const string<T>& operator=(const string<T>& r)	{ this->acquire(r); return *this; }
	virtual ~string<T>()							{ this->release(); }

	/////////////////////////////////////////////////////////////////
	// public pointer functions
	/////////////////////////////////////////////////////////////////
	const size_t getRefCount() const
	{
		return (this->itsCounter) ? this->itsCounter->cRefCnt : 1; 
	}
	bool is_sameRef(const string<T>& str) const
	{
		return ( this->itsCounter && this->itsCounter==str.itsCounter );
	}
	bool is_sameRef(const substring<T>& sub) const
	{
		return ( this->itsCounter && sub.cString && this->itsCounter==sub.cString->itsCounter );
	}
	T* unique()
	{
		// force calling string::writeaccess, 
		// which creates a unique object
		return const_cast<T*>(this->string::writeaccess().c_str());
	}

	/////////////////////////////////////////////////////////////////
	// type construction
	/////////////////////////////////////////////////////////////////
	string<T>(const stringinterface<T>& t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const substring<T>& t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const char* t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(char t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(int t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(unsigned int t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(double t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}

	/////////////////////////////////////////////////////////////////
	// assignment operators
	/////////////////////////////////////////////////////////////////
	const string<T>& operator =(const stringinterface<T>& t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(const substring<T>& sub)
	{
		if( this->is_sameRef(sub) )
			this->writeaccess().truncate(sub.cPos, sub.cLen);
		else
			this->writeaccess(false).assign(sub);
		return *this;
	}
	const string<T>& operator =(const char* t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(char t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(int t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(unsigned int t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(double t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// concatination assignment operators
	/////////////////////////////////////////////////////////////////
	const string<T>& operator +=(const stringinterface<T>& t)
	{
		this->writeaccess() += t;
		return *this;
	}
	const string<T>& operator +=(const string<T>& t)
	{
		this->writeaccess() += t.readaccess();
		return *this;
	}
	const string<T>& operator +=(const char* t)
	{
		this->writeaccess() += t;
		return *this;
	}
	const string<T>& operator +=(char t)
	{
		this->writeaccess() += t;
		return *this;
	}
	const string<T>& operator +=(int t)
	{
		this->writeaccess() += t;
		return *this;
	}
	const string<T>& operator +=(unsigned int t)
	{
		this->writeaccess() += t;
		return *this;
	}
	const string<T>& operator +=(double t)
	{
		this->writeaccess() += t;
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// add operators 
	// for right side operations
	/////////////////////////////////////////////////////////////////
	string<T> operator +(const stringinterface<T>& t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	string<T> operator +(const string<T>& t)
	{
		string<T> a(*this);
		a.writeaccess() += t.readaccess();
		return a;
	}
	string<T> operator +(const char* t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	string<T> operator +(char t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	string<T> operator +(int t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	string<T> operator +(unsigned int t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	string<T> operator +(double t)
	{
		string<T> a(*this);
		a.writeaccess() += t;
		return a;
	}
	// add operators 
	// for left hand side operators, only implement these, 
	// the others would confuse standard operators
	friend string<T> operator +(const stringinterface<T>& t, const string<T>& str)
	{
		string<T> a(t);
		a.writeaccess() += str.readaccess();
		return a;
	}
	friend string<T> operator +(const char* t, const string<T>& str)
	{
		string<T> a(t);
		a.writeaccess() += str.readaccess();
		return a;
	}
	friend string<T> operator +(const char t, const string<T>& str)
	{
		string<T> a(t);
		a.writeaccess() += str.readaccess();
		return a;
	}

	/////////////////////////////////////////////////////////////////
	// c++ style piping
	/////////////////////////////////////////////////////////////////
	string<T>& operator <<(const stringinterface<T>& t)
	{
		this->writeaccess() << t;
		return *this;
	}
	string<T>& operator <<(const string<T>& t)
	{
		this->writeaccess() << t.readaccess();
		return *this;
	}
	string<T>& operator <<(const char* t)
	{
		this->writeaccess() << t;
		return *this;
	}
	string<T>& operator <<(char t)
	{
		this->writeaccess() << t;
		return *this;
	}
	string<T>& operator <<(int t)
	{
		this->writeaccess() << t;
		return *this;
	}
	string<T>& operator <<(unsigned int t)
	{
		this->writeaccess() << t;
		return *this;
	}
	string<T>& operator <<(double t)
	{
		this->writeaccess() << t;
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// assignment function
	///////////////////////////////////////////////////////////////////////////
	template<class X> string<T>& assign(const X& t)
	{
		this->writeaccess().assign(t);
		return *this;
	}
	string<T>& assign(const string<T>& s, size_t len)
	{
		if( this->is_sameRef(s) )
			if(len<s.length()) this->writeaccess().truncate(s, len);
		else
			this->writeaccess().assign(s, len);
		return *this;
	}
	string<T>& assign(const T* c, size_t len)
	{
		this->writeaccess().assign(c, len);
		return *this;
	}
	string<T>& assign(const T ch, size_t slen=1)
	{
		this->writeaccess().assign(ch, slen);
		return *this;
	}
	string<T>& assign(const string<T>& s1, size_t len1, const string<T>& s2, size_t len2)
	{
		if( this->is_sameRef(s1) && this->is_sameRef(s2) )
		{	// force a new object in case we do self-assignments
			basestring<T> u(s1);
			this->writeaccess().assign(u, len1, u, len2);
		}
		else if( this->is_sameRef(s1) )
		{
			if(len1<s1.length()) this->writeaccess().truncate(s1, len1);
			this->writeaccess().append(s2, len2);
		}
		else if( this->is_sameRef(s2) )
		{
			if(len2<s2.length()) this->writeaccess().truncate(s2, len2);
			this->writeaccess().insert(0, s1, len1);
		}
		else
		{
			this->writeaccess().assign(s1, len1, s2, len2);
		}
	}
	string<T>& assign(const string<T>& s1, size_t len1, const T* c2, size_t len2)
	{
		if( this->is_sameRef(s1) )
		{
			if(len1<s1.length()) this->writeaccess().truncate(s1, len1);
			this->writeaccess().append(c2, len2);
		}
		else
		{
			this->writeaccess().assign(s1, len1, c2, len2);
		}
	}
	string<T>& assign(const T* c1, size_t len1, const string<T>& s2, size_t len2)
	{
		if( this->is_sameRef(s2) )
		{
			if(len2<s2.length()) this->writeaccess().truncate(s2, len2);
			this->writeaccess().insert(0, c1, len1);
		}
		else
		{
			this->writeaccess().assign(c1, len1, s2, len2);
		}
	}
	string<T>& assign(const T* c1, size_t len1, const T* c2, size_t len2)
	{
		this->writeaccess().assign(c1, len1, c2, len2);
		return *this;
	}
	string<T>& assign_tolower(const T* c, size_t len=~0)
	{
		this->writeaccess().assign_tolower(c, len);
		return *this;
	}
	string<T>& assign_toupper(const T* c, size_t len=~0)
	{
		this->writeaccess().assign_toupper(c, len);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// append function
	///////////////////////////////////////////////////////////////////////////
	template<class X> string<T>& append(const X& t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// append with length
	/////////////////////////////////////////////////////////////////
	string<T>& append(const stringinterface<T>& t, size_t slen=~0)
	{
		this->writeaccess().append(t, slen);
		return *this;
	}
	string<T>& append(const T* c, size_t slen=~0)
	{
		this->writeaccess().append(c, slen);
		return *this;
	}
	string<T>& append(const T ch, size_t slen=1)
	{
		this->writeaccess().append(ch, slen);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	// insert
	/////////////////////////////////////////////////////////////////
	string<T>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~0)
	{
		this->writeaccess().insert(pos, t, slen);
		return *this;
	}
	string<T>& insert(size_t pos, const substring<T>& sub, size_t slen=~0)
	{
		if( this->is_sameRef(sub) )
		{
			basestring<T> a(sub);
			this->writeaccess().insert(pos, a, slen);
		}
		else
			this->writeaccess().insert(pos, sub, slen);
		return *this;
	}
	string<T>& insert(size_t pos, const T* c, size_t slen=~0)
	{
		this->writeaccess().insert(pos, c, slen);
		return *this;
	}
	string<T>& insert(size_t pos, const T ch, size_t slen=1)
	{
		this->writeaccess().insert(pos, ch, slen);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	// replace
	/////////////////////////////////////////////////////////////////
	string<T>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~0)
	{
		this->writeaccess().replace(pos, tlen, t, slen);
		return *this;
	}
	string<T>& replace(size_t pos, size_t tlen, const substring<T>& sub, size_t slen=~0)
	{
		if( this->is_sameRef(sub) )
		{
			basestring<T> a(sub);
			this->writeaccess().replace(pos, tlen, sub, slen);
		}
		else
			this->writeaccess().replace(pos, tlen, sub, slen);
		return *this;
	}
	string<T>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~0)
	{
		this->writeaccess().replace(pos, tlen, c, slen);
		return *this;
	}
	string<T>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1)
	{
		this->writeaccess().replace(pos, tlen, ch, slen);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	// standard string functions
	/////////////////////////////////////////////////////////////////
    virtual operator const T*() const		{ return this->readaccess().c_str(); }
	virtual const T* c_str() const			{ return this->readaccess().c_str(); }
	virtual size_t length()	const			{ return this->readaccess().length(); }
	void clear()							{ this->writeaccess().clear(); }
	void empty()							{ this->writeaccess().clear(); }
	void truncate(size_t sz)				{ this->writeaccess().truncate(sz); }
	void truncate(size_t ix, size_t sz)		{ this->writeaccess().truncate(ix, sz); }
	void removeindex(size_t inx, size_t len){ this->writeaccess().removeindex(inx, len); }

	bool is_empty() const
	{	
		return ( NULL==this->itsCounter || 0==this->readaccess().length() );
	}
	/////////////////////////////////////////////////////////////////
	// Array access
	/////////////////////////////////////////////////////////////////
	substring<T>operator[](size_t inx)
	{
		if( inx < this->length() )
			return substring<T>(this, inx, 1); 
		return substring<T>(this, 0, 0);
	}
	substring<T>operator[](int inx)
	{
		if( inx>0 && (size_t)inx < this->length() )
			return substring<T>(this, inx, 1); 
		return substring<T>(this, 0, 0);
	}
	const char& operator[](size_t inx) const	{ return this->readaccess()[inx]; }
	const char& operator[](int inx) const		{ return this->readaccess()[inx]; }

	/////////////////////////////////////////////////////////////////
	// returns a string to access the string bound to all pointers
	// instead of making it copy on write
	globalstring<T> global()	{ return *this; }

	/////////////////////////////////////////////////////////////////
	// the sub-string operator
	/////////////////////////////////////////////////////////////////
	substring<T> operator()(size_t inx, size_t len=1) const
	{
		if( inx < this->length() )
		{	
			if( inx+len > this->length() )
				len = this->length()-inx;
			return substring<T>(this, inx, len);
		}
		return substring<T>(this, 0, 0);
	}
	string<T> mid(size_t inx, const size_t len=1) const
	{
		if( inx < this->length() )
		{
			if( inx+len > this->length() )
				len = this->length()-inx;
			return string<T>(this->c_str()+inx, len);
		}
		return string<T>();
	}
	/////////////////////////////////////////////////////////////////
	// change case
	/////////////////////////////////////////////////////////////////
	void to_lower()
	{	// this function does practically nothing if the string
		// contains no uppercase characters. once an uppercase character
		// is encountered, the string is copied to another buffer and the 
		// rest is done as usual.

		// a trick to get a non-const pointer without making
		// the string unique
		T* p = const_cast<T*>(this->c_str());
		bool u = false;
		int i = 0;
		while(*p)
		{
			char c = locase(*p);
			// if the character went lowercase...
			if (c != *p)
			{
				// if the resulting string r is not unique yet...
				if (!u)
				{	// ... make it unique and adjust the pointer p accordingly
					// this is done only once.
					p = this->unique() + i;
					u = true;
				}
				*p = c;
			}
			p++;
			i++;
		}
	}
	void to_upper()
	{	// a trick to get a non-const pointer without making
		// the string unique
		T* p = const_cast<T*>(this->c_str());
		bool u = false;
		int i = 0;
		while(*p)
		{
			char c = upcase(*p);
			// if the character went uppercase...
			if (c != *p)
			{
				// if the resulting string r is not unique yet...
				if (!u)
				{	// ... make it unique and adjust the pointer p accordingly
					// this is done only once.
					p = this->unique() + i;
					u = true;
				}
				*p = c;
			}
			p++;
			i++;
		}
	}
	/////////////////////////////////////////////////////////////////
	// Trim
	/////////////////////////////////////////////////////////////////
	void ltrim()
	{	// a trick to get a non-const pointer without making
		// the string unique
		const T* p=this->c_str(), *q = p;
		if(p)
		{
			while( *p && _isspace(*p) )
				p++;
			if(p!=q)
				writeaccess().removeindex(0, p-q);
		}
	}
	void rtrim()
	{	// a trick to get a non-const pointer without making
		// the string unique
		const T* e=this->c_str(), *p = e+this->length()-1, *q=p;
		if(e && p>=e)
		{
			while( p>=e && _isspace(*p) )
				p--;
			if(p!=q)
				writeaccess().truncate(1+p-e);
		}
	}
	void trim()
	{
		rtrim();
		ltrim();
	}


	/////////////////////////////////////////////////////////////////
	// comparisons
	/////////////////////////////////////////////////////////////////

	// compare functions
	int compareTo(const char ch) const
	{
		return this->readaccess().compareTo( ch );
	}
	int compareTo(const string &s) const 
	{	// first check if the pointers are identical
		if( itsCounter != s.itsCounter )
			return this->readaccess().compareTo( s.readaccess() );
		return 0;
	}
	int compareTo(const char *c) const 
	{
		return this->readaccess().compareTo( c );
	}

	// boolean operators
	bool operator ==(const string<T> &s)const { return (compareTo( s ) == 0); }
	bool operator !=(const string<T> &s)const { return (compareTo( s ) != 0); }
	bool operator <=(const string<T> &s)const { return (compareTo( s ) <= 0); }
	bool operator >=(const string<T> &s)const { return (compareTo( s ) >= 0); }
	bool operator < (const string<T> &s)const { return (compareTo( s ) <  0); }
	bool operator > (const string<T> &s)const { return (compareTo( s ) >  0); }

	bool operator ==(const char c) 	const { return (compareTo( c ) == 0); }
	bool operator !=(const char c) 	const { return (compareTo( c ) != 0); }
	bool operator <=(const char c) 	const { return (compareTo( c ) <= 0); }
	bool operator >=(const char c) 	const { return (compareTo( c ) >= 0); }
	bool operator < (const char c) 	const { return (compareTo( c ) <  0); }
	bool operator > (const char c) 	const { return (compareTo( c ) >  0); }
	
	bool operator ==(const char *c) const { return (compareTo( c ) == 0); }
	bool operator !=(const char *c) const { return (compareTo( c ) != 0); }
	bool operator <=(const char *c) const { return (compareTo( c ) <= 0); }
	bool operator >=(const char *c) const { return (compareTo( c ) >= 0); }
	bool operator < (const char *c) const { return (compareTo( c ) <  0); }
	bool operator > (const char *c) const { return (compareTo( c ) >  0); }

	friend bool operator ==(const char *c, const string<T> &s)	{return (s==c);}
	friend bool operator !=(const char *c, const string<T> &s)	{return (s!=c);}
	friend bool operator <=(const char *c, const string<T> &s)	{return (s>=c);}
	friend bool operator >=(const char *c, const string<T> &s)	{return (s<=c);}
	friend bool operator < (const char *c, const string<T> &s)	{return (s> c);}
	friend bool operator > (const char *c, const string<T> &s)	{return (s< c);}

	/////////////////////////////////////////////////////////////////
	// Search function
	/////////////////////////////////////////////////////////////////
	size_t find(const string<T>& pattern, size_t &startpos, bool ignorecase=false) const
	{	// modified boyer-moore search
		
		// table size is bound to 8bit values
		size_t	SkipTable[256]; 

		size_t i,k,sp;
		size_t len = pattern.length();

		// initialisation
		for (i=0; i<256; i++)
		{	// skip len+1 string chars if search char was not found
			// not exactly boyer-moore but fastens up the thing
			SkipTable[i] = len+1;
		}
		for (i=0; i<len; i++)
		{	// otherwise skip as only that many 
			// so the next char in the string fits with the one frome the pattern
			SkipTable[ (unsigned char) (pattern[i]) ] = len-i;
		}

		// run first to last / case sensitive
		sp=i=startpos;
		k=0; 
		while( i<this->length() && k<len )
		{
			if( ignorecase ? 
				(tolower((unsigned char)((*this)[i])) != tolower((unsigned char)(pattern[k]))) :
				((*this)[i] != pattern[k]) )
			{	// no match at that position, find the next starting pos
				sp += SkipTable[((unsigned char)((*this)[sp+len]))];
				i=sp;
				k=0;
			}
			else
			{	// check the next char
				i++;
				k++;
			}
		}
		if( k<len ) // not found
			return 0;
		startpos = sp+1;
		return sp;
	}

	TArrayDST<size_t> findall(const string<T>& pattern, bool ignorecase=false) const
	{	// modified boyer-moore search
		TArrayDST<size_t> poslist;

		// table size is bound to 8bit values
		size_t	SkipTable[256]; 

		size_t i,k,sp;
		size_t len = pattern.length();

		// initialisation
		for (i=0; i<256; i++)
		{	// skip at least len+1 if char was not found
			SkipTable[i] = len+1;
		}
		for (i=0; i<len; i++)
		{	// otherwise skip as only that many 
			// so the char in the string fits with the one frome the pattern
			SkipTable[((unsigned char)(pattern[i]))] = len-i;
		}

		// run first to last / case sensitive
		sp=i=0;
		k=0; 
		while( i<this->length() )
		{
			if( ignorecase ? 
				(tolower((unsigned char)((*this)[i])) != tolower((unsigned char)(pattern[k]))) :
				((*this)[i] != pattern[k]) )
			{	// no match at that position, fine the next starting pos
				sp += SkipTable[((unsigned char)((*this)[sp+len]))];
				i=sp;
				k=0;
			}
			else
			{	// check the next char
				i++;
				k++;
			}
			if(k>=len)
			{	// found
				poslist.push(sp);
				sp++;
			}
		}
		return poslist;
	}
};



template<class T=char> class globalstring : public string<T>
{
protected:
	/////////////////////////////////////////////////////////////////
	// virtual access functions to the smart pointer
	/////////////////////////////////////////////////////////////////
	// !!autocount
	virtual const basestring<T>& readaccess() const
	{
		this->checkpointer();	
		// no need to aquire, is done on reference creation
		return *this->itsCounter;
	}
	virtual       basestring<T>& writeaccess(bool keep=true)
	{
		this->checkpointer();
		// no need to aquire, is done on reference creation
		return *this->itsCounter;
	}

public:
	/////////////////////////////////////////////////////////////////
	// construction/destruction
	/////////////////////////////////////////////////////////////////
	globalstring<T>()											{  }
	const globalstring<T>& operator=(const globalstring<T>& r)
	{
		this->string<T>::operator=(r);
		return *this; 
	}
	const globalstring<T>& operator=(const string<T>& r)
	{
		this->string<T>::operator=(r);
		return *this; 
	}
	virtual ~globalstring<T>()									{  }

	/////////////////////////////////////////////////////////////////
	// type construction
	/////////////////////////////////////////////////////////////////
	globalstring<T>(const globalstring<T>& r) : string<T>(r)	{  }
	globalstring<T>(const string<T>& r) : string<T>(r)			{  }
	globalstring<T>(const substring<T>& t) : string<T>(t)		{  }
	globalstring<T>(const char* t) : string<T>(t)				{  }
	explicit globalstring<T>(char t) : string<T>(t)				{  }
	explicit globalstring<T>(int t) : string<T>(t)				{  }
	explicit globalstring<T>(unsigned int t) : string<T>(t)		{  }
	explicit globalstring<T>(double t) : string<T>(t)			{  }

	/////////////////////////////////////////////////////////////////
	// assignment operators
	/////////////////////////////////////////////////////////////////
	const globalstring<T>& operator =(const stringinterface<T>& t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(const substring<T>& t)
	{
		if( this->is_sameRef(t) )
			this->writeaccess().truncate(t.cPos, t.cLen);
		else
			this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(const char* t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(char t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(int t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(unsigned int t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const globalstring<T>& operator =(double t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	/////////////////////////////////////////////////////////////////
};



template <class T=char> class substring : public stringinterface<T>
{
	friend class string<T>;
	string<T>* cString;
	size_t cPos;
	size_t cLen;
// my gnu ver. 3.4 don't like private constructors
// looks like some gcc really insists on having this members public
#ifdef __GNUC__
public:
#else
private:
#endif
	substring<T>(const substring<T>&);

private:
	substring<T>(const string<T>* str, size_t pos, size_t len)
		: cString(const_cast<string<T>*>(str)), cPos(pos), cLen(len)
	{ }

	int compareTo(const char ch) const
	{
		if(cLen==1)
			return this->c_str()[0] - ch;
		else
			return this->c_str()[1];
	}
	int compareTo(const char *c) const
	{	// cannot compare up to the terminator because Substrings do not have one
		int ret = memcmp(this->c_str(), c, cLen);
		if(ret == 0 ) return (0-c[cLen]);
		return ret;
	}
	int compareTo(const string<T> &s) const
	{	// cannot compare up to the terminator because Substrings do not have one
		int ret = memcmp(this->c_str(), s->c_str(), this->cLen);
		if(ret == 0 ) return (0 - s->c_str()[cLen]);
		return ret;
	}
	int compareTo(const substring<T> &sub) const
	{
		if(this->cLen!=sub.cLen)
			return this->cLen-sub.cLen;
		int ret = memcmp(this->c_str(), sub.c_str(), cLen);
		return ret;
	}
public:
	~substring<T>()	{}

	const size_t getRefCount() const
	{
		return (this->cString) ? this->cString->getRefCount() : 1;
	}

	virtual operator const T() const	{ return (cString) ? this->cString->c_str()[cPos] : 0; }
	virtual operator const T*() const	{ return (cString) ? this->cString->c_str()+cPos  : ""; }
	virtual const T* c_str() const		{ return (cString) ? this->cString->c_str()+cPos  : ""; }
	virtual size_t length() const		{ return cLen; }

	/////////////////////////////////////////////////////////////////
	// Different Assignments to a substring
	/////////////////////////////////////////////////////////////////
	const substring<T>& operator=(const string<T>& str)
	{	
		if(cString)
		{
			if( this->cString->is_sameRef(str) )
			{	// handle the recursive assignment
				basestring<T> x(str);	// make a copy
				this->cString->writeaccess().replace(cPos, cLen, x.c_str(), x.length() );
			}
			else
				this->cString->writeaccess().replace(cPos, cLen, str.c_str(), str.length() );
		}
		return *this;
	}
	const substring<T>& operator=(const substring<T>& sub)
	{
		if(cString)
		{
			if( this->cString->is_sameRef(sub) )
			{	// handle the recursive assignment
				basestring<T> x(sub);	// make a copy
				this->cString->writeaccess().replace(cPos, cLen, x.c_str()+sub.cPos, sub.cLen);
			}
			else
				this->cString->writeaccess().replace(cPos, cLen, sub.c_str()+sub.cPos, sub.cLen);
		}
		return *this;
	}
	const substring<T>& operator=(const char* c)
	{	// we assume the given const char in not taken from the string itself
		if(cString)
		{
			size_t sz = (c) ? strlen(c) : 0;
			if(sz) this->cString->writeaccess().replace(cPos, cLen, c, sz);
		}
		return *this;
	}
	const substring<T>& operator=(const char ch)
	{	// replece the selected string potion with one character
		if(ch && cString) this->cString->writeaccess().replace(cPos, cLen, &ch, 1);
		return *this;
	}

	string<T> operator +(const string<T> &s)
	{
		return string<T>(*this) << s;
	}
	string<T> operator +(const stringinterface<T> &t)
	{
		return string<T>(*this) << t;
	}
	string<T> operator +(const substring<T> &sub)
	{
		return string<T>(*this) << sub;
	}
	string<T> operator +(const char* c)
	{
		return string<T>(*this) << c;
	}
	string<T> operator +(const char ch)
	{
		return string<T>(*this) << ch;
	}

	bool operator ==(const char ch)			const { return (compareTo( ch ) == 0); }
	bool operator !=(const char ch) 		const { return (compareTo( ch ) != 0); }
	bool operator <=(const char ch) 		const { return (compareTo( ch ) <= 0); }
	bool operator >=(const char ch) 		const { return (compareTo( ch ) >= 0); }
	bool operator < (const char ch) 		const { return (compareTo( ch ) <  0); }
	bool operator > (const char ch) 		const { return (compareTo( ch ) >  0); }

	bool operator ==(const char *c)			const { return (compareTo( c ) == 0); }
	bool operator !=(const char *c) 		const { return (compareTo( c ) != 0); }
	bool operator <=(const char *c) 		const { return (compareTo( c ) <= 0); }
	bool operator >=(const char *c) 		const { return (compareTo( c ) >= 0); }
	bool operator < (const char *c) 		const { return (compareTo( c ) <  0); }
	bool operator > (const char *c) 		const { return (compareTo( c ) >  0); }

	bool operator ==(const string<T> &s)	const { return (compareTo( s ) == 0); }
	bool operator !=(const string<T> &s)	const { return (compareTo( s ) != 0); }
	bool operator <=(const string<T> &s)	const { return (compareTo( s ) <= 0); }
	bool operator >=(const string<T> &s)	const { return (compareTo( s ) >= 0); }
	bool operator < (const string<T> &s)	const { return (compareTo( s ) <  0); }
	bool operator > (const string<T> &s)	const { return (compareTo( s ) >  0); }

	bool operator ==(const substring<T> &s)	const { return (compareTo( s ) == 0); }
	bool operator !=(const substring<T> &s)	const { return (compareTo( s ) != 0); }
	bool operator <=(const substring<T> &s)	const { return (compareTo( s ) <= 0); }
	bool operator >=(const substring<T> &s)	const { return (compareTo( s ) >= 0); }
	bool operator < (const substring<T> &s)	const { return (compareTo( s ) <  0); }
	bool operator > (const substring<T> &s)	const { return (compareTo( s ) >  0); }
};







template <class T=char> class patternstring : public string<T>
{
	friend class string<T>;
	// table size is bound to 8bit values
	size_t	SkipTable[256];
public:
	patternstring(const string<T>& pattern) : string<T>(pattern)
	{
		size_t len = pattern.length();
		size_t i;
		// initialisation
		for (i=0; i<256; i++)
		{	// skip len+1 string chars if search char was not found
			// not exactly boyer-moore but fastens up the thing
			SkipTable[i] = len+1;
		}
		for (i=0; i<len; i++)
		{	// otherwise skip as only that many 
			// so the next char in the string fits with the one frome the pattern
			size_t inx = to_unsigned( pattern[i] );
			if( inx < (sizeof(SkipTable)/sizeof(SkipTable[0])) )
				SkipTable[ inx  ] = len-i;
		}
	}

	/////////////////////////////////////////////////////////////////
	// Search function
	/////////////////////////////////////////////////////////////////
	bool find(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase=false) const
	{	// modified boyer-moore search
		size_t sp, i, k, len=this->length();
		// run first to last / case sensitive
		sp=i=startpos;
		k=0; 
		while( i<searchstring.length() && k<len )
		{
			if( ignorecase ? 
				(tolower((unsigned char)(searchstring[sp+len])) != tolower((unsigned char)((*this)[k]))) :
				(searchstring[i] != (*this)[k]) )
			{	// no match at that position, find the next starting pos
				size_t inx = to_unsigned( searchstring[sp+len] );
				sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
				i=sp;
				k=0;
			}
			else
			{	// check the next char
				i++;
				k++;
			}
		}
		if( k<len ) // not found
			return false;
		startpos = sp;
		return true;
	}

	TArrayDST<size_t> findall(const stringinterface<T>& searchstring, bool ignorecase=false) const
	{	// modified boyer-moore search
		TArrayDST<size_t> poslist;
		size_t sp, i, k, len=this->length();
		// run first to last / case sensitive
		sp=i=0;
		k=0; 
		while( i<searchstring.length() )
		{
			if( ignorecase ? 
				(tolower((unsigned char)(searchstring[sp+len])) != tolower((unsigned char)((*this)[k]))) :
				(searchstring[i] != (*this)[k]) )
			{	// no match at that position, find the next starting pos
				size_t inx = to_unsigned( searchstring[sp+len] );
				sp += (inx<(sizeof(SkipTable)/sizeof(SkipTable[0]))) ? SkipTable[inx] : 1;
				i=sp;
				k=0;
			}
			else
			{	// check the next char
				i++;
				k++;
			}
			if(k>=len)
			{	// found
				poslist.push(sp);
				sp++;
			}
		}
		return poslist;
	}
};







/*
    friend void   assign(string& s, const char* buf, int len);
    friend void   clear(string& s);
    friend bool   isempty(const string& s);
    friend char*  setlength(string&, int);
    friend char*  unique(string&);
    friend void   concat(string& s, const char* sc, int catlen);
    friend void   concat(string& s, const char* s1);
    friend void   concat(string& s, char s1);
    friend void   concat(string& s, const string& s1);
    friend string copy(const string& s, int from, int cnt);
    friend string copy(const string& s, int from);
    friend void   ins(const char* s1, int s1len, string& s, int at);
    friend void   ins(const char* s1, string& s, int at);
    friend void   ins(char s1, string& s, int at);
    friend void   ins(const string& s1, string& s, int at);
    friend void   del(string& s, int at, int cnt);
    friend void   del(string& s, int at);
    friend int    pos(const char* s1, const string& s);
    friend int    pos(char s1, const string& s);
    friend int    pos(const string& s1, const string& s);
    friend int    rpos(char s1, const string& s);
    friend bool   contains(const char* s1, int len, const string& s, int at);
    friend bool   contains(const char* s1, const string& s, int at);
    friend bool   contains(char s1, const string& s, int at);
    friend bool   contains(const string& s1, const string& s, int at);
    friend string dup(const string& s);
	friend int compare(const string &a,const string &b){ return strcmp(a,b); }


    friend void initialize(string& s);
    friend void initialize(string& s, const string& s1);
    friend void initialize(string& s, const char* s1);
    friend void finalize(string& s);
*/



template<class T> inline void assign(string<T>& s, const char* buf, int len)	{ s.assign(buf, len); }
template<class T> inline void clear(string<T>& s)								{ s.clear(); }
template<class T> inline bool isempty(const string<T>& s)						{ return s.length() == 0; }
template<class T> inline int  pos(const string<T>& s1, const string<T>& s)		{ return pos(s1.c_str(), s); }

template<class T> string<T> lowercase(const T* p) 
{
    // we rely on the function locase() which converts one single
    // character to lower case. all locale specific things can be
    // settled down in the future releases.
    return string<T>().assign_tolower(p);
}

template<class T> string<T> lowercase(const string<T>& s)
{
	// this function does practically nothing if the string s
	// contains no uppercase characters. once an uppercase character
	// is encountered, the string is copied to another buffer and the 
	// rest is done as usual.
	string<T> r = s;

	// a trick to get a non-const pointer without making
	// the string unique
	char* p = pchar(cchar(r));
	bool u = false;
	int i = 0;
	while (*p != 0)
	{
		char c = tolower( to_unsigned(*p) );
		// if the character went lowercase...
		if (c != *p)
		{
			// if the resulting string r is not unique yet...
			if (!u)
			{	// ... make it unique and adjust the pointer p accordingly
				// this is done only once.
				p = r.unique() + i;
				u = true;
			}
			*p = c;
		}
		p++;
		i++;
	}
	return r;
}








static void _itobase2(string<>& result, sint64 value, size_t base, bool _signed, size_t width=0, char padchar=0)
{
    if(base < 2 || base > 64)
    {
        clear(result);
        return;
    }
    char buf[128];   // the longest possible string is 64 when base=2
    size_t reslen;
    const char* p = _itobase(value, buf, base, reslen, _signed);
    if (width > reslen)
    {
        if (padchar == 0)
        {	// default pad char
            if (base == 10)
                padchar = ' ';
            else if (base > 36)
                padchar = '.';
            else
                padchar = '0';
        }
		bool neg = *p == '-';
		result.clear();
		width -= reslen;
		if( neg && padchar!=' ' )
		{	// no space padding, need to strip the sign
			result.append('-');		// add the sign in front
			p++;					// buffer now starts after the sign
			reslen--;				// and is one element shorter
		}
		result.append(padchar, width);	// padding
		result.append(p, reslen);	// buffer
    }
    else 
        result.assign(p, reslen);
}


string<> itostring(sint64 value, size_t base, size_t width=0, char padchar=0)
{
    string<> result;
    _itobase2(result, value, base, true, width, padchar);
    return result;
}


string<> itostring(uint64 value, size_t base, size_t width=0, char padchar=0)
{
    string<> result;
    _itobase2(result, value, base, false, width, padchar);
    return result;
}


string<> itostring(int value, size_t base, size_t width=0, char padchar=0)
{
    string<> result;
    _itobase2(result, sint64(value), base, true, width, padchar);
    return result;
}


string<> itostring(uint value, size_t base, size_t width=0, char padchar=0)
{
    string<> result;
    _itobase2(result, uint64(value), base, false, width, padchar);
    return result;
}


string<> itostring(sint64 v)	{ return itostring(v, 10, 0, ' '); }
string<> itostring(uint64 v)	{ return itostring(v, 10, 0, ' '); }
string<> itostring(int v)		{ return itostring(sint64(v), 10, 0, ' '); }
string<> itostring(uint v)		{ return itostring(uint64(v), 10, 0, ' '); }


string<> dup(const string<>& s)
{    // dup() only reads the data pointer so it is thread-safe
    return string<>(s);
}


string<> nowstring(const char* fmt, bool utc)
{
    char buf[128];
    time_t longtime;
    time(&longtime);
#if defined(SINGLETHREAD) || defined(WIN32)
    tm* t;
    if (utc)
        t = gmtime(&longtime);
    else
        t = localtime(&longtime);
    int r = strftime(buf, sizeof(buf), fmt, t);
#else
    tm t;
    if (utc)
        gmtime_r(&longtime, &t);
    else
        localtime_r(&longtime, &t);
    int r = strftime(buf, sizeof(buf), fmt, &t);
#endif
    buf[r] = 0;
    return string<>(buf);
}




bool contains(const char* s1, size_t s1len, const string<>& s, size_t at)
{
    return (s1len >= 0) && (at >= 0) && (at + s1len <= hstrlen(s))
        && (s1len == 0 || memcmp(s.c_str() + at, s1, s1len) == 0);
}


bool contains(const char* s1, const string<>& s, size_t at)
{
    return contains(s1, hstrlen(s1), s, at);
}


bool contains(char s1, const string<>& s, size_t at)
{
    return (at >= 0) && (at < hstrlen(s)) && (s.c_str()[at] == s1);
}


bool contains(const string<>& s1, const string<>& s, size_t at)
{
    return contains(s1.c_str(), hstrlen(s1), s, at);
}

template<class T> string<T> copy(const string<T>& s, size_t from, size_t cnt)
{
    string<T> t;
    if( hstrlen(s) > 0 && from >= 0 && from < hstrlen(s)) 
    {
        size_t l = min(cnt, hstrlen(s) - from);
        if (from == 0 && l == hstrlen(s))
            t = s;
        else if (l > 0) 
            t = t(from, cnt);
    }
    return t;
}

string<> copy(const string<>& s, size_t from)
{
    return copy(s, from, INT_MAX);
}

void ins(const char* s1, size_t s1len, string<>& s, size_t at)
{
	s.insert(at, s1, s1len);
}

void ins(const char* sc, string<>& s, int at)
{
	s.insert(at, sc, hstrlen(sc));
}

void ins(char c, string<>& s, int at)
{
	s.insert(at, c);
}

void ins(const string<>& s1, string<>& s, int at)
{
	s.insert(at, s);
}


void del(string<>& s, int from, int cnt)
{
	s.removeindex(from, cnt);
}


void del(string<>& s, int from)
{
	s.removeindex(from, INT_MAX);
}

int pos(const char* sc, const string<>& s)
{
    const char* t = strstr(s.c_str(), sc);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

int pos(char c, const string<>& s)
{
    const char* t = strchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

int rpos(char c, const string<>& s)
{
    const char* t = strrchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

string<> fill(size_t width, char ch)
{
    string<> res;
    if (width > 0)
        res.assign(ch, width);
    return res;
}

string<> pad(const string<>& s, size_t width, char ch, bool left)
{
	size_t len = hstrlen(s);
	if(width > 0 && len < width)
	{
		string<> x(s);
		x.insert( (left)?0:len, ch, width-len);
		return x;
	}
	return s;
}
































class yyytest
{
	char array[16];
public:
	const char* c_str() const	{ return array;}
	yyytest()	{}
};

class xxxtest : public yyytest
{
	
public:
	xxxtest()	{}
	~xxxtest()	{}


	operator const char*()				{ printf("string\n"); return this->c_str(); }


	      char& operator[](int i)		{ printf("array\n");return ((char*)this->c_str())[i]; }
	const char& operator[](int i) const	{ printf("const array\n");return this->c_str()[i]; }
};




int stringbuffer_test()
{
	{

		string<> a = "  12345  12345   ";
		a.trim();
	}




	{
		string<> a;
		const char *c;
		uint b, i;
	
		for(i=0; i<66; i++)
		{
			a = itostring(1234567890, i);
			c = a;
			b = stringtoue(a, i);
			printf("base 10 '%u' / base %2i '%s'\n", b, i, c);
		}
		printf("\n");
	}




	{
		basestring<> a("halloballo");
		basestring<> b(a);
		basestring<> c("11111");
	
		patternstring<> pat("ll");

		size_t pos=0;
		pat.find(a, pos);
		TArrayDST<size_t> list = pat.findall(a);

		char buffer[1024];

		TString< char, allocator_w_dy<char>, elaborator_st<char> > aa;
		TString< char, allocator_w_st<char>, elaborator_st<char> > bb(buffer,1024);

		aa = bb;


		a.append("xxxxx");
		a.insert(5, c, 2);
		a.replace(2, 2, '.', 6);

	}

	{
		string<> a="aa11bb";
		a(2,2) = a;

		a.assign( a(2,4) );
	}

	{



		xxxtest xxx;

		char c = xxx[3];
		if( xxx[5]!='t' )
			xxx[1] = c;

MiniString a; bool bb = true;

a << "hallo" << 1 << bb;


		basestring<> sa, sb, sc;
		globalstring<> ga, gb, gc;
		string<> da, db, dc;

	//	const char c = da[3];
size_t sz=0;
		da = "abcde";
		da.find("cd",sz);
		db = "test";
		dc = da;

		ga = da;

		ga += "1111";
		da += "2222";

		sa = ga;

		gb = sa;

	}







	size_t i, sz;
	char buffer1[1024];

	char buffer2[1024];
	staticstring<> sa(buffer2, sizeof(buffer2));
	basestring<> sb;
	MiniString a;




	sa = "hallo";
	sb = "hallo";
	sa << "test" << 19999;

	sb = sa + 111111;

	ulong tick = gettick();
	for(i=0; i<100000; i++)
		sz = snprintf(buffer1, sizeof(buffer1), "hallo %i ballo %i no %lf", i, i*2/3+i, ((double)i)*1.3);
	printf("sprintf %i\n", gettick()-tick);

	tick = gettick();
	for(i=0; i<100000; i++)
		a = MiniString("hallo ") + (int)i + " ballo " + (int)(i*2/3+i) + " no " +(((double)i)*1.3);
	printf("Ministring %i\n", gettick()-tick);

	tick = gettick();
	for(i=0; i<100000; i++)
	{
		sa.clear();
		sa << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("staticstring %i\n", gettick()-tick);
	
	tick = gettick();
	for(i=0; i<100000; i++)
	{
		sb.clear();
		sb << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("basestring %i\n", gettick()-tick);

	string<> ds;
	tick = gettick();
	for(i=0; i<100000; i++)
	{
		ds.clear();
		ds << "a";
		ds << "hallo " << (int)i << " ballo " << (int)(i*2/3+i) << " no " << (((double)i)*1.3);
	}
	printf("string %i\n", gettick()-tick);

	return 0;
}



// string test functions
#define string string<>
#define substring substring<>



// checkRefCntVal: pass by value, where reference count gets incremented
// by one on entry and decremented by one on exit.
 
void checkRefCntVal(string s, size_t shouldBe, const char *prefix )
{
  if (s.getRefCount() != shouldBe)
    printf("%s refCnt = %d (should be %d)\n", 
           prefix, s.getRefCount(), shouldBe );
} // checkRefCntVal



// checkRefCnt: pass by reference.  The reference count in the string
// object is unchanged, since the object is passed by reference.

void checkRefCnt(string &s, size_t shouldBe, const char *prefix )
{
  if (s.getRefCount() != shouldBe)
    printf("%s refCnt = %d (should be %d)\n", 
           prefix, s.getRefCount(), shouldBe );
} // checkRefCnt



// test_constructors
void test_constructors()
{
  string a( "abcd" );
  string b( a );

  printf("Test string constructors\n");

  if (b.getRefCount() != 2)
    printf("1. Reference count is wrong: refCnt = %d (should be 2)\n",
           b.getRefCount() );

  string c = a;
  if (b.getRefCount() != 3)
    printf("2. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  if (a.getRefCount() != 3)
    printf("3. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  checkRefCntVal( a, 4, "4. ");

  if (a.getRefCount() != 3)
    printf("4. Reference count is wrong: refCnt = %d (should be 3)\n",
           b.getRefCount() );

  checkRefCnt( a, 3, "5. ");

  string d( 'd' );

  // test for construction with an empty string
  string e;
  string f( e );
  checkRefCnt( f, 2, "6. ");
  checkRefCntVal( f, 3, "7. ");

  // test of a pointer containing null
  const char *nullPtr = 0;
  string g( nullPtr );

  // test of a null address
  string h( (const char *)0 );

  string i( '\0' );
  if (i.length() != 0) {
    printf("8. Adding a null character should not increase length\n");
  }
} // test_constructors



// test_char_cast
void test_char_cast()
{
  printf("Test character cast\n");

  const char *testStr = "the quick brown fox";
  string a( testStr );
  const char *tmp = a;

  if (tmp == 0)
    printf("1. error: result of cast is null\n");
  else if (strcmp(testStr, tmp) != 0)
    printf("2. error: strings are not equal\n");

} // test_char_cast



// test_assign
//
// The privious version of the string class created a new copy when
// ever the index operator ([]) was used.  I was surprised to 
// discover that this was the case even with the index operator
// was on the right hand size.  This function includes a test
// which checks that copy on read does not take place in this
// version.
void test_assign()
{
  printf("Test assignment\n");

  const char *testStr = "my girl is the best";
  string a = "abcd";

  const char *tmp = a;

  if (strcmp(tmp, "abcd") != 0)
    printf("1. Assignment in declaration failed\n");

  const char *init_b = "this is not it";
  string b(init_b);
  string original_b;

  original_b = b;

  b = testStr;
  if (b.getRefCount() != 1)
    printf("2. reference count for b is wrong\n");

  tmp = b;
  if (strcmp(tmp, testStr) != 0)
    printf("3. string has incorrect contents\n");

  if (original_b.getRefCount() != 1)
     printf("4. reference count for original_b is wrong\n");

  if (original_b != init_b)
     printf("5. modification of b improperly changed original_b\n");

  string c( testStr );
  c = b;
  if (b.getRefCount() != 2)
    printf("6. reference count is wrong\n");

  const char *nullPtr = 0;
  string d;

  if (d != "") {
    printf("7. comparision to a null string failed\n");
  }

  d = nullPtr;
  if (d != "") {
    printf("8. assignment of a null C-string failed\n");
  }

  d = testStr;
  tmp = d;
  if (strcmp(tmp, testStr) != 0)
    printf("9. string has incorrect contents\n");

  string e = string( testStr );
  tmp = e;
  if (strcmp(tmp, testStr) != 0)
    printf("10. string has incorrect contents\n");

  if (e.getRefCount() != 1)
    printf("11. refCnt is wrong: refCnt = %d, should be 1\n",
           e.getRefCount() );

  const char *constCStr = "1234567890";
  const size_t len = sizeof(constCStr) / sizeof(char);
  string foo = constCStr;
  string bar = foo;
  if (foo.getRefCount() != 2) {
    printf("12. refcnt is wrong: refCnt = %d, should be 2\n",
	   foo.getRefCount() );
  }

  // This makes sure that the [] operator is implemented properly
  // and does not cause a "copy-on-write" on a read operation.
  bool contentOK = true;
  for (size_t i = 0; i < len; i++) {
    if (constCStr[i] != foo[i]) {
      contentOK = false;
      break;
    }
  }
  if (!contentOK) {
    printf("13: content is wrong\n");
  }
  // make sure refCnt is still OK
  if (bar.getRefCount() != 2) {
    printf("14. refcnt is wrong: refCnt = %d, should be 2\n",
	   bar.getRefCount() );
  }

  const char *testStr2 = "null is a lonely number";
  string r = testStr2;
  string r2 = r;
  r = '\0';
  if (r != "") {
    printf("15. assignment of null character did not result in empty str\n");
  }

  if (r2 != testStr2) {
    printf("16. null character assignment changed a shared string\n");
  }

  if (r2.getRefCount() != 1) {
    printf("17. reference count is wrong\n");
  }

  const char *testStr3 = "\"Writing tests is hard!\" said Barbie";
  string s = testStr3;
  string s2 = s;
  s = "";
  if (s != "") {
    printf("18. assignment of empty string did not result in empty str\n");
  }

  if (s2 != testStr3) {
    printf("19. empty string assignment changed a shared string\n");
  }

  if (s2.getRefCount() != 1) {
    printf("17. reference count is wrong\n");
  }

  // Test chained assignment
  const char *testStr4 = "working on the chain gang";
  string w, x, y;
  w = x = y = testStr4;

  if (w != testStr4 ||
      x != testStr4 ||
      y != testStr4) {
    printf("18. chained assignment failed\n");
  }

  if (y.getRefCount() != 3) {
    printf("19. reference count in chained assignment is wrong\n");
  }

  string z = "still working on the gang";
  w = x = y = z;
  if((w != x) ||
     (x != y) ||
     (y != z)) {
    printf("20. chained assignment failed\n");
  }

  if (y.getRefCount() != 4) {
    printf("21. reference count in chained assignment is wrong\n");
  }
} // test_assign



// test_plus_equal
void test_plus_equal()
{
  const char *firstHalf = "abcd";
  const char *secondHalf = " efgh";
  const char *concatStr = "abcd efgh";

  printf("Test += operator\n");

  string a( firstHalf );

  a += secondHalf;

  const char *tmp = a;

  if (strcmp(tmp, concatStr) != 0)
    printf("1. Strings did not match: str = %s (should be [%s]\n",
           tmp, concatStr );

  string b;

  b += firstHalf;
  tmp = b;
  if (strcmp(tmp, firstHalf) != 0)
    printf("2. Strings did not match: str = %s (should be [%s]\n",
           tmp, firstHalf );

  string d, c;

  c += d;
  if (c.getRefCount() != 1)
    printf("3. refCnt should (still) be 1\n");

  if (d != "" || c != "") {
    printf("4. Strings c and d should be the empty string, but are not\n");
  }

  c += secondHalf;
  tmp = c;
  if (strcmp(tmp, secondHalf) != 0)
    printf("5. Strings did not match: str = %s (should be [%s]\n",
           tmp, secondHalf );

  string e("1234");

  for (size_t i = 5; i < 10; i++) {
    e += (char)(i + (char)'0');
  }
  if (e != "123456789") {
    tmp = e;
    printf("6. Character concat failed: d = %s, should be 123456789\n",
           tmp );
  }

  const char *testStr1 = "metal jacket";
  string empty;
  string full(testStr1);

  empty += full;

  if (empty.getRefCount() != 1) {
    printf("7. empty.getRefCount() = %d, should be 1\n", empty.getRefCount() );
  }

  if (empty != testStr1) {
    printf("8. empty string += string failed\n");
  }

  const char *testStr2 = "foo";
  string empty2;
  const char *str = testStr2;

  empty2 += str;
  if (empty2.getRefCount() != 1) {
    printf("9. empty2.getRefCount() = %d, should be 1\n", empty2.getRefCount() );
  }

  if (empty2 != testStr2) {
    printf("10. empty string += C-string failed\n");
  }

  // test chained assignment
  const char *testStr3 = "twas brillig";
  string s1 = "twas ";
  string s2 = "brillig";
  string s3 = s1 += s2;

  if (s3 != s1 && s1 != testStr3) {
    printf("11. chained assignment with += failed\n");
  }

  if (s3.getRefCount() != 2 &&
      s1.getRefCount() != 2 &&
      s2.getRefCount() != 1) {
    printf("12. reference count for chained assignment with += failed\n");
  }
} // test_plus_equal




// test_plus
void test_plus()
{
  printf("Test + operator\n");

  const char *firstHalf = "abcd";
  const char *secondHalf = " efgh";
  const char *concatStr = "abcd efgh";

  //
  // Test string + string
  //
  string t1( firstHalf );
  string t2( secondHalf );
  string a = t1 + t2;
  if (a.getRefCount() != 1) {
    printf("1. refCnt is wrong: refCnt = %d, should be 1\n",
           a.getRefCount() );
  }

  if (strcmp((const char *)a, concatStr) != 0)
    printf("2. string contents are not correct: a = %s (should be [%s])\n",
           (const char *)a, concatStr );

  //
  // Test string + const char *
  //
  string b = t1 + secondHalf;

  if (b.getRefCount() != 1) {
    printf("3. refCnt is wrong: refCnt = %d, should be 1\n",
           b.getRefCount() );
  }

  const char *tmp = b;
  if (strcmp(tmp, concatStr) != 0)
    printf("4. string contents are not correct: b = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // test the global string addition operator const char * + string
  //
  string c = firstHalf + t2;
  tmp = c;
  if (strcmp(tmp, concatStr) != 0)
    printf("5. string contents are not correct: c = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // Make sure that the operands of the addition are not altered by
  // the addition
  //
  string first( firstHalf );
  string second( secondHalf );
  string d = first + second;
  tmp = first;
  if (strcmp(tmp, firstHalf) != 0)
    printf("6. first has been altered: first = %s (should be [%s])\n",
           tmp, firstHalf );

  tmp = second;
  if (strcmp(tmp, secondHalf) != 0)
    printf("7. second has been altered: second = %s (should be [%s])\n",
           tmp, secondHalf );

  tmp = d;
  if (strcmp(tmp, concatStr) != 0)
    printf("8. string contents are not correct: d = %s (should be [%s])\n",
           tmp, concatStr );

  //
  // Test character concatenation.  Here the character operands
  // are converted to string objects.
  //
  string e("12345");
  string f;
  string g;

  f = e + '6' + '7' + '8' + '9';
  g = 'a' + f;

  if (e != "12345")
    printf("9. string e changed\n");

  if (f != "123456789") {
     const char *tmp = f;
     printf("10. string f is %s, it should be \"123456789\"\n", tmp);
  }

  if (g != "a123456789")
    printf("11. g is incorrect\n");

  // Test addition with chained assignment and string operands
  string w( "foo" );
  string x( "bar" );
  string y;
  string z = y = w + x;

  if (y != "foobar") {
     const char *tmp = y;
     printf("12. string y is %s, it should be \"foobar\"\n", tmp );
  }

  if (z != "foobar") {
     const char *tmp = z;
     printf("13. string z is %s, it should be \"foobar\"\n", tmp );
  }

  // The reference counts for w and x should both be 1
  if (w.getRefCount() != 1) {
     printf("14. w.getRefCount() = %d, it should be 1\n", w.getRefCount() );
  }
  if (x.getRefCount() != 1) {
     printf("15. x.getRefCount() = %d, it should be 1\n", x.getRefCount() );
  }

  if (y.getRefCount() != 2) {
     printf("16. y.getRefCount() = %d, it should be 2\n", y.getRefCount() );
  }
  if (z.getRefCount() != 2) {
     printf("17. z.getRefCount() = %d, it should be 2\n", z.getRefCount() );
  }
} // test_plus




// test_relops
//
// Test relational operators

void test_relops()
{
  printf("Test relational operators\n");

  const char *less = "abcd";
  const char *greater = "wxyz";
  const char *equal = "abcd";
  string lessString( less );
  string greaterString( greater );
  string equalString( equal );  // note that equalString == lessString == less
  string same;

  same = less;

  //
  // ==
  //
  // string string
  //
  // Check the case where both strings contain no data
  // (and so are equal).
  string x, y;  // two empty strings
  if (x != y)
    printf("0. empty strings are not equal\n");

  if (x != "") {
    printf("0.5: string not equal to empty C string\n");
  }

  if (! (lessString == equalString))
    printf("1. string == string failed\n");

  // string const char *
  if (! (lessString == equal))
    printf("2. string == const char * failed\n");

  // const char *  string
  if (! (equal == lessString))
    printf("3. const char * == string failed\n");

  if (! (same == less))
    printf("string == string failed for string objs w/same shared data\n");

  //
  // !=
  //
  // string string
  if (! (lessString != greaterString))
    printf("4. string != string failed\n");

  // string const char *
  if (! (lessString != greater))
    printf("5. string != const char * failed\n");

  // const char *  string
  if (! (less != greaterString))
    printf("6. const char * != string failed\n");

  //
  // >=
  //
  // string string
  if (! (greaterString >= lessString))
    printf("7. string >= string failed for >\n");

  if (! (lessString >= equalString))
    printf("8. string >= string failed for ==\n");

  // string const char *
  if (! (greaterString >= less))
    printf("9. string >= const char * failed for >\n");

  if (! (lessString >= equal))
    printf("10. string >= const char * failed ==\n");

  // const char *  string
  if (! (greater >= lessString))
    printf("11. const char * >= string failed for >\n");

  if (! (equal >= lessString))
    printf("12. const char * >= string failed for ==\n");

  //
  // <=
  //
  // string string
  if (! (lessString <= greaterString))
    printf("13. string <= string failed for <\n");

  if (! (lessString <= equalString))
    printf("14. string <= string failed for ==\n");

  // string const char *
  if (! (lessString <= greater))
    printf("15. string <= const char * failed for <\n");

  if (! (lessString <= equal))
    printf("16. string <= const char * failed for ==\n");

  // const char *  string
  if (! (less <= greaterString))
    printf("17. const char * <= string failed for <\n");

  if (! (equal <= lessString))
    printf("18. const char * <= string failed for ==\n");

  //
  // >
  //
  // string string
  if (! (greaterString > lessString))
    printf("19. string > string failed\n");

  // string const char *
  if (! (greaterString > less))
    printf("20. string > const char * failed\n");

  // const char *  string
  if (! (greater > lessString))
    printf("21. const char * > string failed\n");

  //
  // <
  //
  // string string
  if (! (lessString < greaterString))
    printf("22. string < string failed\n");

  // string const char *
  if (! (lessString < greater))
    printf("23. string < const char * failed\n");

  // const char *  string
  if (! (less < greaterString))
    printf("24. const char * < string failed\n");

} // test_relops




// Array reference [] operator tests

// There are two versions: left hand size and right
// hand size array operators.  The right hand side
// version returns a value, the left hand size
// operator returns an address (to which a value
// can be assigned).

void test_arrayop()
{
  const char *jabbar     = "He took his vorpal sword in hand";
  const char *newJabbar1 = "He took her vorpal sword in hand";
  //     index 9   -----------------^
  const char *newJabbar2 = "He took her vorpal ruler in hand";
  //     index 19  ----------------------------^
  const char *newWord = "ruler";

  const size_t len = strlen( jabbar );
  string jabbarString( jabbar );
  string jabbarRefStr = jabbarString;


  printf("test arrayops\n");

  //
  // Make sure that integer operators on the RHS work properly
  //
  if (jabbarString[3] != 't') {
    printf("0. string index failed\n");
  }

  // make sure than in index operation does not cause a copy
  if (jabbarRefStr.getRefCount() != 2) {
    printf("0.5. string index seems to have caused a copy\n");
  }

  for (size_t i = 0; i < len; i++) {
    char lhsCh = jabbarString[i];
    char rhsCh = jabbar[i];
    if (lhsCh != rhsCh) {
      printf("1. mismatch on rhs string index\n");
    }
  }

  // references are jabbarString, jabbarRefStr and now, "a"
  string a = jabbarString;
  if (a.getRefCount() != 3)
    printf("2. reference count is wrong\n");

  a(9) = 'e';
  a(10) = 'r';

  // The string has been changed, so a "copy on write" should 
  // have taken place.  Now there is a single copy, with the
  // change.
  if (a.getRefCount() != 1)
    printf("3. 'a' reference count is wrong\n");

  if (jabbarString.getRefCount() != 2)
    printf("4. jabbarString reference count is wrong\n");
  
  const char *tmp = a;
  if (strcmp(tmp, newJabbar1) != 0) {
    printf("5. strings don't match: a = %s, should be %s\n",
           tmp, newJabbar1 );
  }

  // make sure that the original string is unchanged
  tmp = jabbarString;
  if (strcmp(tmp, jabbar) != 0) {
    printf("6. strings don't match: a = %s, should be %s\n",
           tmp, jabbar );
  }

  // make sure that the original string is unchanged
  a = newJabbar1;

  a(19,5) = newWord;
  tmp = a;
  if (strcmp(tmp, newJabbar2) != 0) {
    printf("7. strings don't match: a = %s, should be %s\n",
           tmp, newJabbar2 );
  }


} // test_arrayop





// test_insert()
//
// In an earlier version of the string object used an explicit insert
// function to insert a string into another string.  This is very much
// like assigning to a SubString.  In the case of an insert the
// length of the section being replaced is 0.
   
// This code attempts to not only make sure that insert works, but
// also that insert works for various corner cases (e.g., insert of an
// empty string or a null C string).

void test_insert()
{
  printf("Test string insert\n");

  const char *origA = "abcdefgh";
  const char *rslt1 = "abcd1234efgh";
  string a(origA);
  string b("1234");
  string c = a;

  // test insert of a string into a string

  // reference count should be 2, since a and c have the
  // same shared data
  if (a.getRefCount() != 2)
    printf("1. reference count is wrong\n");

  a(4,0) = b;
  // make sure a is "abcd1234efgh"
  if (a != rslt1) {
    printf("2. insert failed. a = [%s], should be [%s]\n",
           (const char *)a, rslt1 );
  }

  // a should be unique
  if (a.getRefCount() != 1)
    printf("3. reference count in a is wrong\n");

  // c should be unique
  if (c.getRefCount() != 1)
    printf("4. reference count in c is wrong\n");

  // The contents of c should be unchanged
  if (c != origA)
    printf("5. contents of c is wrong\n");

  // test insert of C-string into a string
  string d = c;
  d(4,0) = "1234";
  // make sure d is "abcd1234efgh"
  if (d != rslt1) {
    printf("6. insert failed. d = [%s], should be [%s]\n",
           (const char *)d, rslt1 );
  }  

  string c_prime = c;  // reference count is 2

  // test insert of a zero length C string into a string
  // (remember, c is still "abcdefgh")
  c(4, 0) = "";
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("7. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }

  // Insert a null string into a string object (should do nothing)
  c_prime(4, 0) = (const char *)0;
  // make sure that reference count is still 2
  if (c.getRefCount() != 2)
    printf("8. c.getRefCount() = %d, should be 2\n", c.getRefCount());

  if (c_prime.getRefCount() != 2)
    printf("9. c_prime.getRefCount() = %d, should be 2\n", c_prime.getRefCount());

  if (c_prime != origA) {
    printf("10. insert failed. c_prime = [%s], should be [%s]\n",
           (const char *)c_prime, origA );
  }

  // test insert of an empty string into "c", an non-empty string.
  string emptyString;

  // try insert at index 0
  c(0,0) = emptyString;
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("11. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }

  // try insert at index 4
  c(4,0) = emptyString;
  // make sure c is "abcdefgh" (e.g., nothing happened)
  if (c != origA) {
    printf("12. insert failed. c = [%s], should be [%s]\n",
           (const char *)c, origA );
  }
} // test_insert


//
void test_substr_func_valarg( const substring &sub, size_t errorNum )
{
  if (sub.getRefCount() != 1) {
    printf("%d. sub.getRefCount() = %d, should be 1\n", 
           errorNum, sub.getRefCount() );
  }

  if (sub != "de") {
    printf("%d. sub.string = %s, should be \"de\"\n",
           errorNum + 1, (const char *)((const string)sub) );
  }
} // test_substr_func_valarg



// test_substring
//
// Test operations on SubStrings via the string() operator.
// These test overlap the SubString class tests.  They
// are here for historical reasons.

void test_substring()
{
  printf("Test sub-string operations\n");
  const char *init_a = "abcdefgh";
  string a(init_a);

  if (a(3, 4).getRefCount() != 1) {
    printf("1. reference count is wrong\n");
  }

  test_substr_func_valarg( a(3, 2), 2 );

  string b = a;

  // b is "abcdefgh
  // insert "1234" at position 3
  b(3, 4) = "1234";

  if (a != init_a) {
    printf("3. \"a\" was altered when b(3, 4) was changed\n");
  }

  if (b != "abc1234h") {
    printf("4. b = %s, should be \"abc1234h\"\n", (const char *)b);
  }

  if (a.getRefCount() != 1 || b.getRefCount() != 1) {
    printf("5. a.getRefCount() = %d, b.getRefCount() = %d (both should be 1)\n",
           a.getRefCount(), b.getRefCount() );
  }

  string c;
  c = b(3, 4);
  if (c != "1234") {
    printf("6. c = %s, should be \"1234\"\n", (const char *)c);
  }

  if (c.length() != 4) {
    printf("7. c.length() = %d, should be 4\n", c.length());
  }

  string d("1234abcdefgh");
  string e;

  e = d(4, 8) + d(0, 4); // e = d << 4
  if (e != "abcdefgh1234") {
    printf("8. e = %s, should be \"abcdefgh1234\"\n", (const char *)e );
  }

  if (d != "1234abcdefgh") {
    printf("9. d was changed by the SubString operations\n");
  }

  if (d.getRefCount() != 1) {
    printf("10. d.getRefCount() = %d, it should be 1\n", d.getRefCount());
  }

  //
  // According to 10.4.10 Temporary Objects in "The C++ Programming
  // Language", Third Edition, by Stroustrup:
  //
  //   Unless bound to a reference or used to initialize a named
  //   object, a temporary object is destroyed at the end of teh full
  //   expression in which it was created.  A full expression is an
  //   expression that is not a subexpression of some other
  //   expression.
  //
  // Stroustrup goes on to provide an example using the Standard
  // Template Library (STL) "string" class.  Here the c_str()
  // function returns a "C" language string.
  //
  //    const char* cs = (s1 + s2).c_str().
  //
  //    A temporary object of class string is created to hold s1+s2.
  //    Next, a pointer to a C-style string is extracted from the
  //    object.  Then - at the end of the expression - the temporary
  //    object is deleted.  Now, where was the C-style string
  //    allocated?  Probably as part of the temporary object holding
  //    s1+s2, and that storage is not guaranteed to exist after that
  //    temporary is destroyted.  Consequently, cs points to
  //    deallocated storage.
  //
  // In the case of the string container, the expression
  //
  //     e = d(4, 8) + d(0, 4);
  // 
  // creates a string object temporary, which is assigned to "e"
  //
  //   <allocate string temporary object CompilerTemp>
  //   <CompilerTemp> = d(4, 8) + d(0, 4);
  //   e = <CompilerTemp>     Note: at this point getRefCount = 2
  //   <destructor called for CompilerTemp> Note: refCnt decremented
  //   next statement
  //
  // By the time we reach "next statement" the destructor is called
  // and the reference counter for "e" is 1, which is what we would
  // expect.
  //
  // If this example does not convience you that C++ is a complicated
  // language, nothing will.
  //
  if (e.getRefCount() != 1) {
    printf("11. e.getRefCount() = %d, it should be 1\n", e.getRefCount());
  }

  //
  // Note that the SubString object created by the () operator is
  // a new object and so has a reference count of 1, although the
  // string associated with it has a reference count of two.
  string z = "lost Z-man";
  string y = z; // refCnt is now 2
  if (z(5, 5).getRefCount() != 1) {
    printf("12. z(8, 4).getRefCount() = %d, should be 1, !!changed behaviour here!!\n", z(8, 4).getRefCount() );
  }

  string f("chopsock");

  f(4, 4) = f(0, 4);
  
  if (f != "chopchop") {
    printf("13. f = %s, should be \"chopchop\"\n", (const char *)f );
  }
} // test_substring



//
// test_resize
//
// Test the string object resize function.

void test_resize()
{
  const char *init_a = "01234567890123456789";
  string a( init_a );
  string b = a;
  const char *tmp;

  printf("Test resize\n");

  b.truncate(10);  // set size of string b to 10
  if (b.length() != 10)
    printf("1. b.length() = %d, should be 10\n", b.length() );

  if (b != "0123456789") {
    tmp = b;
    printf("2. b = %s, should be \"0123456789\"\n", tmp );
  }

  if (a != init_a)
    printf("3. a was improperly modified by resizing b\n");

  if (a.length() != 20)
    printf("4. a.length() = %d, should be 20\n", a.length() );

  b.truncate(20);
  if (b != "0123456789          ") {
    tmp = b;
    printf("5. b = %s, should be \"0123456789          \"\n", tmp );
  }
  if (b.length() != 20)
    printf("6. b.length() = %d, should be 20\n", b.length() );

  if (a != init_a)
    printf("8. resizing b modified a\n");

  b.truncate( 0 );

  string empty;

  if (b != empty)
    printf("9. b is not the same as the empty string\n");

  if (a != init_a)
    printf("10. resizing b modified a\n");

  if (b.length() != 0)
    printf("11. b.length() = %d, should be 0\n", b.length() );

  if (b != "") {
    printf("12. b should be the same as the empty string\n");
  }

} // test_resize


int stringtest()
{
	test_constructors();
	test_char_cast();
	test_assign();
	test_plus_equal();
	test_plus();
	test_relops();
	test_arrayop();
	test_insert();
	test_substring();
	test_resize();
	return 0;
}

