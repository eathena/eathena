#ifndef __BASESTRING_H__
#define __BASESTRING_H__

//!! bloat marker included
//!! move back included

#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"

///////////////////////////////////////////////////////////////////////////////
// additional includes for wchar support
#include <wchar.h>
#include <wctype.h>


///////////////////////////////////////////////////////////////////////////////
// we need this predeclaration here
// templates are implemented in basearray.h
template <class T> class TArrayDST;
template <class T> class TArrayDCT;


///////////////////////////////////////////////////////////////////////////////
// test functions
///////////////////////////////////////////////////////////////////////////////
int stringtest();
int stringbuffer_test();


///////////////////////////////////////////////////////////////////////////////
// conversion overloads to change signed types to the appropriate unsigned
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
// character conversion/checking
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





///////////////////////////////////////////////////////////////////////////////
//!! TODO: check for integration to the template generation
//         (currently implemented as external stringtable)
#define CONSTSTRING(x) x
//#define CONSTSTRING(x) L##x		// unicode string marker




///////////////////////////////////////////////////////////////////////////////
// cstring to number
///////////////////////////////////////////////////////////////////////////////
template <class T> uint64 stringtoue(const T* str, size_t base);
template <class T> sint64 stringtoie(const T* str);
template <class T> sint64 stringtoi(const T* p);




///////////////////////////////////////////////////////////////////////////////
// number conversion core
// don't use directly
///////////////////////////////////////////////////////////////////////////////
template <class T> const T* _itobase(sint64 value, T* buf, size_t base, size_t& len, bool _signed);






///////////////////////////////////////////////////////////////////////////////
// predeclaration
///////////////////////////////////////////////////////////////////////////////

template <class T> class basestring;		// dynamic buffer without smart pointer
template <class T> class staticstring;		// external static buffer
template <class T> class string;			// dynamic buffer with smart pointer, copy-on-write
template <class T> class globalstring;		// dynamic buffer with smart pointer, copy-on-write
template <class T> class substring;			// contains pointer to string, implements allocator interface
template <class T> class patternstring;		// string derived, implements booyer-moore search



///////////////////////////////////////////////////////////////////////////////
// basic string interface
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class stringinterface : public allocator<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// internal StringConstant aquire
	const T*getStringConstant(size_t i) const;

	///////////////////////////////////////////////////////////////////////////
	// provide an exception interface without including 
	void throw_bound() const;
public:
	stringinterface<T>()			{}
	virtual ~stringinterface<T>()	{}

	virtual const T* c_str() const =0;
	///////////////////////////////////////////////////////////////////////////
	// search functions
	///////////////////////////////////////////////////////////////////////////
	bool				findnext(const stringinterface<T>& pattern, size_t &startpos, bool ignorecase=false) const;
	TArrayDST<size_t>	findall (const stringinterface<T>& pattern, bool ignorecase=false) const;


	/////////////////////////////////////////////////////////////////
	// change case
	/////////////////////////////////////////////////////////////////
	virtual void tolower() =0;
	virtual void toupper() =0;
	///////////////////////////////////////////////////////////////////////////
	// Trim's
	///////////////////////////////////////////////////////////////////////////
	virtual void ltrim() =0;
	virtual void rtrim() =0;
	virtual void trim() =0;
	virtual void itrim(bool removeall=false) =0;
};


///////////////////////////////////////////////////////////////////////////////
// basic string functions
///////////////////////////////////////////////////////////////////////////////
template<class T> inline size_t strlen(const stringinterface<T>& p)	{ return p.length(); }
template<class T> inline size_t hstrlen(const stringinterface<T>& p){ return p.length(); }

template<class T> const stringinterface<T>& toupper(stringinterface<T>& str)	{ return str.toupper(); return str; }
template<class T> const stringinterface<T>& tolower(stringinterface<T>& str)	{ return str.tolower(); return str; }
template<class T> const stringinterface<T>& ltrim(stringinterface<T>& str)		{ return str.ltrim(); return str; }
template<class T> const stringinterface<T>& rtrim(stringinterface<T>& str)		{ return str.rtrim(); return str; }
template<class T> const stringinterface<T>& trim(stringinterface<T>& str)		{ return str.trim(); return str; }
template<class T> const stringinterface<T>& itrim(stringinterface<T>& str)		{ return str.itrim(); return str; }



///////////////////////////////////////////////////////////////////////////////
// basic string functions
///////////////////////////////////////////////////////////////////////////////
// some Unix systems do not accept NULL
inline size_t hstrlen(const char* p)		{ return p == NULL ? 0 : strlen(p); }
inline size_t hstrlen(const wchar_t* p)		{ return p == NULL ? 0 : wcslen(p); }

template<class T> const T* toupper(T* str);
template<class T> const T* tolower(T* str);
template<class T> const T* ltrim(T* str);
template<class T> const T* rtrim(T* str);
template<class T> const T* trim(T* str);
template<class T> const T* itrim(T* str, bool removeall=false);




///////////////////////////////////////////////////////////////////////////////
// basic string template implementation
// don't use this but the derived classes
///////////////////////////////////////////////////////////////////////////////
template < class T=char, class A=allocator_w_dy<T,elaborator_st<T> > >
class TString : public A, public stringinterface<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// string internal part of the number2string conversion
	///////////////////////////////////////////////////////////////////////////
	size_t _itobase2(sint64 value, size_t base, bool _signed, size_t width=0, T padchar=0)
	{
		size_t reslen, retval=0;
		if(base >= 2 && base <= 64)
		{
			T buf[128];   // the longest possible string is 64 when base=2
			const T* p = _itobase<T>(value, buf, base, reslen, _signed);
			retval = reslen;
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
				width -= reslen;
				if( neg && padchar!=' ' )
				{	// no space padding, need to strip the sign
					this->append('-');		// add the sign in front
					p++;					// buffer now starts after the sign
					reslen--;				// and is one element shorter
				}
				this->append(padchar, width);	// padding
			}
			this->append(p, reslen);			// buffer
		}
		return retval;
	}
	///////////////////////////////////////////////////////////////////////////
	// internal number2string calls
	///////////////////////////////////////////////////////////////////////////
	size_t itostring(sint64 v)	{ return _itobase2(sint64(v), 10, true,  0, ' '); }
	size_t itostring(uint64 v)	{ return _itobase2(uint64(v), 10, false, 0, ' '); }
	size_t itostring(long v)	{ return _itobase2(sint64(v), 10, true,  0, ' '); }
	size_t itostring(ulong v)	{ return _itobase2(uint64(v), 10, false, 0, ' '); }
	size_t itostring(int v)		{ return _itobase2(sint64(v), 10, true,  0, ' '); }
	size_t itostring(uint v)	{ return _itobase2(uint64(v), 10, false, 0, ' '); }

public:
	TString<T,A>() : A()							{}
	TString<T,A>(size_t sz) : A(sz)					{}
	TString<T,A>(T* buf, size_t sz) : A(buf, sz)	{}
	virtual ~TString<T,A>()							{}

	///////////////////////////////////////////////////////////////////////////
	// M$ VisualC does not allow template generated copy/assignment operators
	// but always generating defaults if not explicitely defined
	// mixing template and explicit definition does also not work because
	// it "detects" ambiguities in this case
//	template <class A> TString<T,A>(const TString<T, A>& b) : alloc(b.length())
//	{
//		*this<<b;
//	}
	// declare standard copy constructor
	TString<T,A>(const TString<T,A>& b) : A(b.length())
	{
		*this<<b;
	}
	// declare copy of base class types
	TString<T,A>(const stringinterface<T>& t) : A(t.length())
	{
		*this<<t;
	}

	///////////////////////////////////////////////////////////////////////////
	// standard functions
	///////////////////////////////////////////////////////////////////////////
	void clear()				{ if(this->cPtr<this->cEnd) { this->cPtr=this->cBuf; *this->cPtr=0; } }
	void empty()				{ this->clear(); }
	// remove everything from ix for sz length
	void clear(size_t inx, size_t len)
	{
		if( this->cEnd < this->cPtr )	// const string
			;
		else if( this->cBuf+inx < this->cPtr )
		{
			if(this->cBuf+inx+len > this->cPtr)	len = this->cPtr-this->cBuf-inx;
			this->move(this->cBuf+inx, this->cBuf+inx+len, this->cPtr-this->cBuf-inx-len);
			this->cPtr -= len;
			*this->cPtr = 0;
		}
	}
	bool is_empty() const		{ return this->cPtr==this->cBuf; }
	// remove everything exept from 0 for sz length
	void truncate(size_t sz)
	{
		if( this->cEnd < this->cPtr )	// const string
			;
		else if( this->cBuf+sz < this->cPtr )
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
	// remove everything exept from ix for sz length
	void truncate(size_t ix, size_t sz)
	{
		if( this->cEnd < this->cPtr )	// const string
			;
		else if(ix==0)
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
	bool checksize(size_t sz)	{ return (this->cBuf+sz>this->cEnd) ? checkwrite(sz-(this->cEnd-this->cBuf)) : true; }
	///////////////////////////////////////////////////////////////////////////
	// base class functions
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if(!this->cBuf && const_cast<TString<T,A>*>(this)->checkwrite(1))
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
			this->throw_bound();
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
		// merge with exeption code
		if( this->cEnd < this->cPtr )	// const string
		{
			static T dummy;
			return dummy=0;
		}
	#ifdef CHECK_BOUNDS
		if( this->cBuf+inx > this->cPtr )
		{
	#ifdef CHECK_EXCEPTIONS
			this->throw_bound();
	#else
			// out of bound
			static T dummy;
			return dummy=0;
	#endif
		}
	#endif
		return this->cBuf[inx];
	}
	const T& operator[](int inx) const	{ return this->operator[]( (size_t)inx ); }
	T& operator[](int inx)				{ return this->operator[]( (size_t)inx ); }
	///////////////////////////////////////////////////////////////////////////
	// assignment function
	///////////////////////////////////////////////////////////////////////////
	template<class X> TString<T,A>& assign(const X& t)
	{
		if( this->cPtr<this->cEnd ) // not a const string
			this->cPtr = this->cBuf;
		return this->operator<<(t);
	}
	TString<T,A>& assign(const T* c, size_t slen)
	{
		if( this->cPtr<this->cEnd ) // not a const string
			this->cPtr = this->cBuf;
		if( c && this->checkwrite(slen) )
		{
			const T* epp=c+slen;
			while( *c && c<epp )
				*this->cPtr++ = *c++;
		}
		if( this->cPtr<this->cEnd ) *this->cPtr=0;
		return *this;
	}
	TString<T,A>& assign(const T ch, size_t slen=1)
	{
		if( this->cPtr<this->cEnd ) // not a const string
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
		if(this->cPtr<this->cEnd) *this->cPtr=0;
		return *this;
	}
	TString<T,A>& assign(const T* c1, size_t len1, const T* c2, size_t len2)
	{
		if( !c1 || len1 <= 0 )
			assign(c2, len2);
		else if ( !c2 || len2 <= 0 )
			assign(c1, len1);
		else if( this->checkwrite(len1 + len2) )
		{
			this->cPtr = this->cBuf;

			const T* epp=c1+len1;
			while( *c1 && c1 < epp )
				*this->cPtr++ = *c1++;

			epp=c2+len2;
			while( *c2 && c2 < epp )
				*this->cPtr++ = *c2++;

			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A>& assign_tolower(const T* c, size_t slen=~0)
	{
		if( this->cPtr<this->cEnd ) // not a const string
			this->cPtr = this->cBuf;
		if(c)
		{
			const T* epp = (slen==~0) ? (const T*)(~((size_t)0)) : epp=c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cPtr++ = locase(*c++);
		}
		if( this->cPtr<this->cEnd ) *this->cPtr=0;
		return *this;
	}
	TString<T,A>& assign_toupper(const T* c, size_t slen=~0)
	{
		if( this->cPtr<this->cEnd ) // not a const string
			this->cPtr = this->cBuf;
		if(c)
		{
			const T* epp = (slen==~0) ? (const T*)(~((size_t)0)) : epp=c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cPtr++ = upcase(*c++);
		}
		if(this->cPtr<this->cEnd) *this->cPtr=0;
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	// append function
	///////////////////////////////////////////////////////////////////////////
	template<class X> TString<T,A>& append(const X& t)
	{
		return this->operator<<(t);
	}
	///////////////////////////////////////////////////////////////////////////
	// append with length
	///////////////////////////////////////////////////////////////////////////
	TString<T,A>& append(const stringinterface<T>& t, size_t slen)
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
	TString<T,A>& append(const T* c, size_t slen)
	{
		if(c)
		{
			const T* epp = c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cPtr++ = *c++;
			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A>& append(const T ch, size_t slen)
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
	TString<T,A>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~0)
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
	TString<T,A>& insert(size_t pos, const T* c, size_t slen=~0)
	{
		if(c)
		{
			if( pos > this->length() )
				return this->append(c, slen);
			else if( this->cPtr<this->cEnd ) // not a const string
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
	TString<T,A>& insert(size_t pos, const T ch, size_t slen=1)
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
	TString<T,A>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~0)
	{
		if( pos > this->length() )
			return this->append(t, slen);
		else if( this->cPtr<this->cEnd ) // not a const string
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
	TString<T,A>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~0)
	{
		if(c)
		{
			if( pos > this->length() )
				return this->append(c, slen);
			else if( this->cPtr<this->cEnd ) // not a const string
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
	TString<T,A>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1)
	{
		if(ch) // dont append a eos
		{
			if( pos > this->length() )
				return this->append(ch, slen);
			else if( this->cPtr<this->cEnd ) // not a const string
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
//	template <class X> const TString<T,A>& operator =(const X& t)
//	{
//		this->cPtr = this->cBuf;
//		return *this<<t;
//	}

	// only default copy assignment usable
	const TString<T,A>& operator=(const TString<T,A>& t)	{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }

	const TString<T,A>& operator=(const stringinterface<T>& t){ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const T* t)				{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const T t)				{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const int t)				{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const unsigned int t)		{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const long t)				{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const unsigned long t)	{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }
	const TString<T,A>& operator=(const double t)			{ if( this->cPtr<this->cEnd ) this->cPtr = this->cBuf; return this->operator<<(t); }

	///////////////////////////////////////////////////////////////////////////
	// add-assignment operators
	///////////////////////////////////////////////////////////////////////////
	template <class X> TString<T,A>& operator +=(const X& t)
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
	friend string<T> operator +(const T* t, const TString<T,A>& str)
	{
		return ( string<T>(t) << str );
	}
	friend string<T> operator +(const T t, const TString<T,A>& str)
	{
		return ( string<T>(t) << str );
	}

	///////////////////////////////////////////////////////////////////////////
	// operator realisations for supported types
	///////////////////////////////////////////////////////////////////////////
	template <class X> TString<T,A>& operator <<(const TString<T,X>& sb)
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
	TString<T,A>& operator <<(const stringinterface<T>& t)
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
	TString<T,A>& operator <<(const T* ip)
	{
		if(ip)
		{
			while( *ip && this->checkwrite(1) )
				*this->cPtr++ = *ip++;
			if(this->cPtr<this->cEnd) *this->cPtr=0;
		}
		return *this;
	}
	TString<T,A>& operator <<(const T ch)
	{
		if( ch && this->checkwrite(1) )
		{
			*this->cPtr++ = ch;
			*this->cPtr=0;
		}
		return *this;
	}
	TString<T,A>& operator <<(const int v)
	{
	/*	int sz;
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
	*/	this->itostring(v);
		return *this;
	}
	TString<T,A>& operator <<(const unsigned int v)
	{
	/*	int sz;
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
	*/	this->itostring(v);
		return *this;
	}
	TString<T,A>& operator <<(const long v)
	{
	/*	int sz;
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
	*/	this->itostring(v);
		return *this;
	}
	TString<T,A>& operator <<(const unsigned long v)
	{
	/*	int sz;
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
	*/	this->itostring(v);
		return *this;
	}

	
private:	
	ssize_t printdouble(char* buf, size_t sz, double v)
	{
		return snprintf(buf, sz, "%lf", v);
	}
	ssize_t printdouble(wchar_t* buf, size_t sz, double v)
	{	// function not available, just skip it for now
		//!! add extracted printlib
		*buf=0;
		return 0; //wsnprintf(buf, sz, L"%lf", v);
	}
public:
	TString<T,A>& operator <<(const double v)
	{
		ssize_t sz;
		while(this->cEnd>this->cPtr)
		{
			//sz = snprintf(this->cPtr, this->cEnd-this->cPtr, "%lf", v);
			sz = printdouble(this->cPtr, this->cEnd-this->cPtr, v);
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
	int compareTo(const stringinterface<T>& s) const
	{	
		if(s.length()>0 && this->length()>0) 
			// compare including eos
			return this->cmp(this->cBuf, s.c_str(), 1+this->length());
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

	friend bool operator ==(const T *c, const TString<T,A> &s)	{return (s==c);}
	friend bool operator !=(const T *c, const TString<T,A> &s)	{return (s!=c);}
	friend bool operator <=(const T *c, const TString<T,A> &s)	{return (s>=c);}
	friend bool operator >=(const T *c, const TString<T,A> &s)	{return (s<=c);}
	friend bool operator < (const T *c, const TString<T,A> &s)	{return (s> c);}
	friend bool operator > (const T *c, const TString<T,A> &s)	{return (s< c);}

	///////////////////////////////////////////////////////////////////////////
	// change case
	///////////////////////////////////////////////////////////////////////////
	virtual void tolower()
	{
		if( this->cPtr<this->cEnd )
		{
			T* ipp = this->cBuf;
			while( ipp < this->cPtr )
			{
				*ipp = locase(*ipp);
				ipp++;
			}
		}
	}
	virtual void toupper()
	{
		if( this->cPtr<this->cEnd )
		{
			T* ipp = this->cBuf;
			while( ipp < this->cPtr )
			{
				*ipp = upcase(*ipp);
				ipp++;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// trim's
	///////////////////////////////////////////////////////////////////////////
	// remove whitespaces from left side
	virtual void ltrim()
	{
		if( this->cPtr<this->cEnd )
		{
			T* ipp = this->cBuf;
			while( ipp < this->cPtr && _isspace(*ipp) )
				ipp++;
			if(ipp!=this->cBuf)
			{
				this->move(this->cBuf, ipp, this->cPtr-ipp);
				*this->cPtr=0;
			}
		}
	}
	// remove whitespaces from right side
	virtual void rtrim()
	{
		if( this->cPtr<this->cEnd )
		{
			T* ipp = this->cPtr-1;
			T* kpp = this->cPtr-1;
				
			while( ipp>this->cBuf && _isspace(*ipp) )
				ipp--;
			if( ipp != kpp )
			{
				this->cPtr=ipp+1;
				*this->cPtr=0;
			}
		}
	}
	// remove whitespaces from both sides
	virtual void trim()
	{
		if( this->cPtr<this->cEnd )
		{
			T *src=this->cBuf, *tar=this->cBuf, *mk=NULL;
			while(*src && _isspace(*src) )
				src++;
			while(*src)
			{
				mk = ( _isspace(*src) )?mk?mk:tar:NULL;
				*tar++ = *src++;
			}
			this->cPtr = (mk) ? mk : tar;
			*this->cPtr=0;
		}
	}
	// remove whitespaces from both sides, combine or remove speces inside
	virtual void itrim(bool removeall=false)
	{
		if( this->cPtr<this->cEnd )
		{
			T *src=this->cBuf, *tar=this->cBuf, mk=0;
			while(*src && _isspace(*src) )
				src++;
			while(*src)
			{
				if( _isspace(*src) )
					mk=*src, src++;
				else
				{
					if( mk && !removeall )
						*tar++=mk, mk=0;
					*tar++ = *src++;
				}
			}
			*tar=0;
			this->cPtr = tar;
		}
	}
};


///////////////////////////////////////////////////////////////////////////////
// basestring class
// implements a dynamic in-place buffer
// type construction always copies the  data to a newly created buffer
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class basestring : public TString< T, allocator_w_dy<T, elaborator_st<T> > >
{
public:
	basestring()
	{}
	basestring<T>(const basestring<T>& t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	basestring<T>(const stringinterface<T>& t) : TString< T, allocator_w_dy<T,elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	basestring<T>(const char* t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	explicit basestring<T>(char t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	explicit basestring<T>(int t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	explicit basestring<T>(unsigned int t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}
	explicit basestring<T>(double t) : TString< T, allocator_w_dy<T, elaborator_st<T> > >()
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
	}

	virtual ~basestring()	{}

	template<class X> const basestring<T>& operator=(const X& t)
	{
		this->TString< T, allocator_w_dy<T, elaborator_st<T> > >::assign(t);
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

///////////////////////////////////////////////////////////////////////////////
// staticstring class
// implements an external static buffer
// type construction only possible with either a buffer which is used for writing
// or with a const cstring which is then used as a constant without write possibility
///////////////////////////////////////////////////////////////////////////////
template<class T=char> class staticstring : public TString< T, allocator_w_st<T, elaborator_st<T> > >
{
	staticstring<T>();
	staticstring<T>(const staticstring<T>&);
public:
	staticstring<T>(T* buf, size_t sz)
		: TString< T, allocator_w_st<T> >(buf, sz)
	{}
	staticstring<T>(const T* cstr)
		: TString< T, allocator_w_st<T> >(const_cast<T*>(cstr), 0)
	{	// set up a zero-length buffer, 
		// where the length pointer is set at position
		this->cPtr+=hstrlen(cstr);
	}
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


///////////////////////////////////////////////////////////////////////////////
// main string class
// derives internally from basestring and inherits it's dynamic buffer
// additionally it implements a smart pointer with copy-on-write semantic
// so that string copy operations are basically reduced to copying a pointer
///////////////////////////////////////////////////////////////////////////////
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
			atomicincrement( &cRefCnt );
			return this;
		}
		ptrstring* release()
		{
			if( atomicdecrement( &cRefCnt ) == 0 )
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
	// allocation of the pointer
	/////////////////////////////////////////////////////////////////
	virtual ptrstring *createpointer() const throw()
	{
		return new ptrstring;
	}
	virtual ptrstring *createpointer(const ptrstring& old) const throw()
	{
		return new ptrstring(old);
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
			this->itsCounter = createpointer();
		else
			this->make_unique(keep);
		return *this->itsCounter;
	}
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
				const_cast<string<T>&>(r).itsCounter = this->createpointer();
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
	// control of the smart pointer
	/////////////////////////////////////////////////////////////////
	void checkpointer() const
	{
		if(!this->itsCounter)
			const_cast< string* >(this)->itsCounter = this->createpointer();
		// no need to aquire, is done on reference creation
	}
	bool isunique()	const throw()
	{
		return (this->itsCounter) ? (this->itsCounter->cRefCnt == 1):true;
	}
	bool make_unique(bool keep=true)
	{
		if( !this->isunique() )
		{
			ptrstring *cnt = (this->itsCounter && keep) ? 
								this->createpointer(*this->itsCounter) : 
								this->createpointer();
			this->release();
			this->itsCounter = cnt;
		}
		return true;
	}

public:
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
		// we then can return also a writable pointer to the string data
		// but don't try to change the size of the string by modifying the eos!!
		return const_cast<T*>(this->string::writeaccess().c_str());
	}
	/////////////////////////////////////////////////////////////////
	// direct access to the underlying class
	virtual basestring<T>& operator*()	throw()				{ return this->writeaccess(); }
	virtual operator const basestring<T>&() const throw()	{ return this->readaccess();  }


	/////////////////////////////////////////////////////////////////
	// construction/destruction
	/////////////////////////////////////////////////////////////////
	string<T>() : itsCounter(NULL)					{  }
	string<T>(const string& r) : itsCounter(NULL)	{ this->acquire(r); }
	const string<T>& operator=(const string<T>& r)	{ this->acquire(r); return *this; }
	virtual ~string<T>()							{ this->release(); }

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
	string<T>(const T* t) : itsCounter(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const T* t, size_t sz) : itsCounter(NULL)
	{
		this->writeaccess().assign(t, sz);
	}
	explicit string<T>(T t) : itsCounter(NULL)
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
		{	// need an explicit copy
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
		{	// need an explicit copy
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
    virtual operator const T*() const	{ return this->readaccess().c_str(); }
	virtual const T* c_str() const		{ return this->readaccess().c_str(); }
	virtual size_t length()	const		{ return this->readaccess().length(); }
	void clear()						{ this->writeaccess().clear(); }
	void empty()						{ this->writeaccess().clear(); }
	void clear(size_t inx, size_t len)	{ this->writeaccess().clear(inx, len); }
	bool is_empty() const
	{	
		return ( NULL==this->itsCounter || 0==this->readaccess().length() );
	}
	void truncate(size_t sz)			{ this->writeaccess().truncate(sz); }
	void truncate(size_t ix, size_t sz)	{ this->writeaccess().truncate(ix, sz); }
	bool checksize(size_t sz)			{ return this->writeaccess().checksize(sz); }

	/////////////////////////////////////////////////////////////////
	// Array access
	// did not find a way to force the compiler to use the readonly [] operator
	// in any case where it's not written to the object; instead they always
	// use the nonconst operator exept the whole object is const,
	// so we actually do some detour and return a substring which will either 
	// just return the char on readaccess or call the copy-on-write when writing
	// unfortunately the cost of this detour is somewhat higher
	/////////////////////////////////////////////////////////////////
	substring<T>operator[](size_t inx)
	{
		if( inx >= this->length() )
			return substring<T>(this, this->length()-1, 1);
		else
			return substring<T>(this, inx, 1);
	}
	substring<T>operator[](int inx)
	{
		if( inx<=0 )
			return substring<T>(this, 0, 0);
		else if( (size_t)inx >= this->length() )
			return substring<T>(this, this->length()-1, 1);
		else
			return substring<T>(this, inx, 1);
	}
	const T& operator[](size_t inx) const	{ return this->readaccess()[inx]; }
	const T& operator[](int inx) const		{ return this->readaccess()[inx]; }

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
	// comparisons
	/////////////////////////////////////////////////////////////////

	// compare functions
	int compareTo(const string<T> &s) const 
	{	// first check if the pointers are identical
		if( itsCounter != s.itsCounter )
			return this->readaccess().compareTo( s.readaccess() );
		return 0;
	}
	int compareTo(const char *c) const 
	{
		return this->readaccess().compareTo( c );
	}
	int compareTo(const char ch) const
	{
		return this->readaccess().compareTo( ch );
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

	//////////////////////////////////////////////////////
	friend int compare(const string<T> &a,const string<T> &b){ return a.compareTo(b); }


	///////////////////////////////////////////////////////////////////////////
	// search functions
	///////////////////////////////////////////////////////////////////////////
	bool				findnext(const string<T>& pattern, size_t &startpos, bool ignorecase=false) const
	{
		return stringinterface<T>::findnext(pattern,startpos,ignorecase);
	}
	TArrayDST<size_t>	findall (const string<T>& pattern, bool ignorecase=false) const
	{
		return stringinterface<T>::findnext(pattern,ignorecase);
	}
	bool				findnext(const T* pattern, size_t &startpos, bool ignorecase=false) const
	{
		staticstring<T> tmp(pattern);
		return stringinterface<T>::findnext(tmp,startpos,ignorecase);
	}
	TArrayDST<size_t>	findall (const T* pattern, bool ignorecase=false) const
	{
		staticstring<T> tmp(pattern);
		return stringinterface<T>::findnext(tmp,ignorecase);
	}
	/////////////////////////////////////////////////////////////////
	// change case
	/////////////////////////////////////////////////////////////////
	virtual void tolower()
	{	// this function does practically nothing if the string
		// contains no uppercase characters. once an uppercase character
		// is encountered, the string is copied to another buffer for copy-on-write strings
		// and the rest is done as usual.

		// a trick to get a non-const pointer without making
		// the string unique
		T *p, *cp, c;
		p = cp = const_cast<T*>(this->c_str());
		bool u = false;
		if(p)
		{
			while(*p)
			{
				c = locase(*p);
				// if the character went lowercase...
				if (c != *p)
				{
					// if the resulting string r is not unique yet...
					if (!u)
					{	// ... do a writeaccess to copy the buffer 
						// and adjust the pointer p accordingly, this is done only once.
						this->writeaccess();
						p = const_cast<T*>(this->c_str()) + (p-cp);
						u = true;
					}
					*p = c;
				}
				p++;
			}
		}
	}
	virtual void toupper()
	{	// a trick to get a non-const pointer without making
		// the string unique
		T *p, *cp, c;
		p = cp = const_cast<T*>(this->c_str());
		bool u = false;
		if(p)
		{
			while(*p)
			{
				c = upcase(*p);
				// if the character went uppercase...
				if (c != *p)
				{
					// if the resulting string r is not unique yet...
					if (!u)
					{	// ... do a writeaccess to copy the buffer 
						// and adjust the pointer p accordingly, this is done only once.
						this->writeaccess();
						p = const_cast<T*>(this->c_str()) + (p-cp);
						u = true;
					}
					*p = c;
				}
				p++;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// trim's
	///////////////////////////////////////////////////////////////////////////
	// remove whitespaces from left side
	virtual void ltrim()
	{	// a trick to get a non-const pointer without making
		// the string unique
		const T* p=this->c_str(), *q = p;
		if(p)
		{
			while( *p && _isspace(*p) )
				p++;
			if(p!=q)
				writeaccess().clear(0, p-q);
		}
	}
	// remove whitespaces from right side
	virtual void rtrim()
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
	// remove whitespaces from both sides
	virtual void trim()
	{
		rtrim();
		ltrim();
	}
	// remove whitespaces from both sides, combine or remove speces inside
	virtual void itrim(bool removeall=false)
	{
		this->writeaccess().itrim(removeall);
	}
};


///////////////////////////////////////////////////////////////////////////////
// globalstring
// practically same as string
// but the smart pointer behaviour does not do copy-on-write
// can be used to globally change all strings connected to the same buffer
// use with care in multithread environment
///////////////////////////////////////////////////////////////////////////////
template<class T=char> class globalstring : public string<T>
{
protected:
	/////////////////////////////////////////////////////////////////
	// virtual access functions to the smart pointer
	/////////////////////////////////////////////////////////////////
	// !!autocount behaviour
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
	explicit globalstring<T>(long t) : string<T>(t)				{  }
	explicit globalstring<T>(unsigned long t) : string<T>(t)	{  }
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


///////////////////////////////////////////////////////////////////////////////
// substring
// result of string::operator()
// can only be created from inside a string and not stored, 
// so it is safe to just contain and work on a pointer to the calling string
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class substring : public stringinterface<T>
{
	friend class string<T>;
	string<T>* cString;
	size_t cPos;
	size_t cLen;

// my gnu ver. 3.4 don't like private constructors
// looks like some gcc really insists on having this members public
// even if it's not used and just there to block the default generated copy constructor
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
		int ret = memcmp(this->c_str(), s.c_str(), this->cLen);
		if(ret == 0 ) return (0 - s.c_str()[cLen]);
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
	virtual operator const T*() const	{ return (cString) ? this->cString->c_str()+cPos  : this->getStringConstant(0); }
	virtual const T* c_str() const		{ return (cString) ? this->cString->c_str()+cPos  : this->getStringConstant(0); }
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
	/////////////////////////////////////////////////////////////////
	// change case
	/////////////////////////////////////////////////////////////////
	virtual void tolower()
	{
		string<T> sel = *this;
		sel.tolower();
		if(*this!=sel)
			*this=sel;

	}
	virtual void toupper()
	{
		string<T> sel = *this;
		sel.toupper();
		if(*this!=sel)
			*this=sel;
	}
	///////////////////////////////////////////////////////////////////////////
	// trim's
	///////////////////////////////////////////////////////////////////////////
	// remove whitespaces from left side
	virtual void ltrim()
	{	
		string<T> sel = *this;
		sel.ltrim();
		if(*this!=sel)
			*this=sel;
	}
	// remove whitespaces from right side
	virtual void rtrim()
	{
		string<T> sel = *this;
		sel.rtrim();
		if(*this!=sel)
			*this=sel;
	}
	// remove whitespaces from both sides
	virtual void trim()
	{
		string<T> sel = *this;
		sel.trim();
		if(*this!=sel)
			*this=sel;
	}
	// remove whitespaces from both sides, combine or remove speces inside
	virtual void itrim(bool removeall=false)
	{
		string<T> sel = *this;
		sel.itrim(removeall);
		if(*this!=sel)
			*this=sel;
	}
};


///////////////////////////////////////////////////////////////////////////////
// patternstring
// derived from string
// additionally generates the pattern skip table to be used in booyer-moore search
// fastens up continious searches of the same pattern in different strings
///////////////////////////////////////////////////////////////////////////////
template <class T=char> class patternstring : public string<T>
{
	friend class string<T>;
	// table size is bound to 8bit values
	size_t	SkipTable[256];
public:
	patternstring(const string<T>& pattern);
	///////////////////////////////////////////////////////////////////////////////
	// Search function
	///////////////////////////////////////////////////////////////////////////////
	bool findnext(const stringinterface<T>& searchstring, size_t &startpos, bool ignorecase=false) const;
	TArrayDST<size_t> findall(const stringinterface<T>& searchstring, bool ignorecase=false) const;

};

///////////////////////////////////////////////////////////////////////////////
// dbstring
// class itself is not usable
// it just contains a simple singleton with a list of all stored strings
// so it can be used to retrieve same stringbuffers throughout the application
///////////////////////////////////////////////////////////////////////////////
template<class T=char> class dbstring
{
private:
	// simple Singleton
	static TArrayDCT< string<T> >& getStringList()
	{
		static TArrayDCT< string<T> > cStringList;
		return cStringList;
	}
	dbstring()	{}
	~dbstring()	{}
public:	
	static const string<T>& get(const string<T>& str)
	{
		size_t pos;
		TArrayDCT< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast< string<T>* >(list.array()), list.size());
		if( !BinarySearch(str, list.array(), list.size(), 0, pos) )
		{
			list.insert(str,1,pos);
		}
		return list[pos];
	}
	static const string<T>& get(const char* c)
	{
		size_t pos;
		TArrayDCT< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast< string<T>* >(list.array()), list.size());
		if( !BinarySearch(c, list.array(), list.size(), 0, pos) )
		{
			string<T> s(c);
			list.insert(s,1,pos);
		}
		return list[pos];
	}
	static const string<T>& get(const stringinterface<T>& si)
	{
		size_t pos;
		TArrayDCT< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast< string<T>* >(list.array()), list.size());
		if( !BinarySearch(si, list.array(), list.size(), 0, pos) )
		{
			string<T> s(si);
			list.insert(s,1,pos);
			return list[pos];
		}
		return list[pos];
	}
};


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
template<class T> inline void assign(string<T>& s, const char* buf, int len)
{
	s.assign(buf, len);
}
template<class T> inline void clear(string<T>& s)
{
	s.clear();
}
template<class T> inline bool isempty(const stringinterface<T>& s)
{
	return s.length() == 0;
}
template<class T> inline int  pos(const string<T>& s1, const string<T>& s)
{
	return pos(s1.c_str(), s);
}

template<class T> inline string<T> lowercase(const T* p) 
{
    return string<T>().assign_tolower(p);
}

template<class T> inline string<T> lowercase(const string<T>& s)
{
	string<T> r = s;
	r.to_lower();
	return r;
}

template<class T> inline string<T> dup(const string<T>& s)
{    // dup() only reads the data pointer so it is thread-safe
    return string<T>(s);
}

template<class T> inline bool contains(const T* s1, size_t s1len, const string<T>& s, size_t at)
{
	return (s1len >= 0) && (at >= 0) && (at+s1len <= hstrlen(s))
        && (s1len == 0 || memcmp(s.c_str()+at, s1, s1len*sizeof(T)) == 0);
}

template<class T> inline bool contains(const T* s1, const string<T>& s, size_t at)
{
    return contains<T>(s1, hstrlen(s1), s, at);
}

template<class T> inline bool contains(const T s1, const string<T>& s, size_t at)
{
    return (at<hstrlen(s)) && (s.c_str()[at] == s1);
}

template<class T> inline bool contains(const string<T>& s1, const string<T>& s, size_t at)
{
    return contains<T>(s1.c_str(), hstrlen(s1), s, at);
}

template<class T> inline string<T> copy(const string<T>& s, size_t from, size_t cnt)
{
    string<T> t;
    if( hstrlen(s)>0 && from<hstrlen(s)) 
    {
        size_t l = min(cnt, hstrlen(s)-from);
        if( from==0 && l==hstrlen(s) )
			t = s;
		else if(l>0)
			t = t(from, cnt);
	}
	return t;
}

template<class T> inline string<T> copy(const string<T>& s, size_t from)
{
	return copy(s, from, INT_MAX);
}

template<class T> inline void ins(const T* s1, size_t s1len, string<T>& s, size_t at)
{
	s.insert(at, s1, s1len);
}

template<class T> inline void ins(const T* sc, string<T>& s, size_t at)
{
	s.insert(at, sc);
}

template<class T> inline void ins(const T c, string<T>& s, size_t at)
{
	s.insert(at, c);
}

template<class T> inline void ins(const string<T>& s1, string<T>& s, size_t at)
{
	s.insert(at, s);
}

template<class T> inline void del(string<T>& s, size_t from, size_t cnt)
{
	s.clear(from, cnt);
}

template<class T> inline void del(string<T>& s, size_t from)
{
	s.clear(from, INT_MAX);
}

template<class T> inline int pos(const T* sc, const string<T>& s)
{
    const T* t = sstrstr(s.c_str(), sc);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<class T> inline int pos(T c, const string<T>& s)
{
    const T* t = strchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<class T> inline int rpos(T c, const string<T>& s)
{
    const T* t = strrchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<class T> inline string<T>& fill(string<T>& str, size_t width, T ch)
{
    if( width > 0 )
        str.assign(ch, width);
	else
		str.clear();
    return str;
}

template<class T> inline string<T>& pad(string<T>& s, size_t width, T ch, bool left)
{
	size_t len = hstrlen(s);
	if( width > 0 && len < width )
		s.insert( (left)?0:len, ch, width-len);
	return s;
}




///////////////////////////////////////////////////////////////////////////////
// default number to string conversions
// using string default type
///////////////////////////////////////////////////////////////////////////////
string<> itostring(sint64 value, size_t base, size_t width=0, char padchar=0);
string<> itostring(uint64 value, size_t base, size_t width=0, char padchar=0);
string<> itostring(int value, size_t base, size_t width=0, char padchar=0);
string<> itostring(uint value, size_t base, size_t width=0, char padchar=0);

inline string<> itostring(sint64 v)	{ return itostring(sint64(v), 10, 0, ' '); }
inline string<> itostring(uint64 v)	{ return itostring(uint64(v), 10, 0, ' '); }
inline string<> itostring(int v)	{ return itostring(sint64(v), 10, 0, ' '); }
inline string<> itostring(uint v)	{ return itostring(uint64(v), 10, 0, ' '); }

///////////////////////////////////////////////////////////////////////////////
// timestamp string
///////////////////////////////////////////////////////////////////////////////
template<class T> string<T> nowstring(const T* fmt, bool utc=true);
template<class T> string<T> dttostring(datetime, const T* fmt);



extern string<> nullstring;


#endif//__BASESTRING_H__
