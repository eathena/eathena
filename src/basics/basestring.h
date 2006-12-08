#ifndef __BASESTRING_H__
#define __BASESTRING_H__

//## bloat marker included
//## move back included

#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basearray.h"

#include "basestrformat.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// test functions
///////////////////////////////////////////////////////////////////////////////
void test_strings(void);
void test_stringbuffer(void);




///////////////////////////////////////////////////////////////////////////////
/// character conversion/checking.
/// with specified implementations for certain types
///////////////////////////////////////////////////////////////////////////////
template <typename T> inline T locase(T c) { return ::tolower( to_unsigned(c) ); }
inline char locase(char c) { if (c >= 'A' && c <= 'Z') return char(c + 32); return c; }
inline wchar_t locase(wchar_t c) { return ::towlower( to_unsigned(c) ); }

template <typename T> inline T upcase(T c) { return ::toupper( to_unsigned(c) ); }
// implementation for char (and wchar)
inline char upcase(char c) { if (c >= 'a' && c <= 'z') return char(c - 32); return c; }
inline wchar_t upcase(wchar_t c) { return ::towupper( to_unsigned(c) ); }






///////////////////////////////////////////////////////////////////////////////
//## TODO: check for integration to the template generation
//         (currently implemented as external stringtable)
#define CONSTSTRING(x) x
//#define CONSTSTRING(x) L##x		// unicode string marker






///////////////////////////////////////////////////////////////////////////////
/// cstring to number
///////////////////////////////////////////////////////////////////////////////
template <typename T> uint64 stringtoue(const T* str, size_t base);
template <typename T> sint64 stringtoie(const T* str);
template <typename T> sint64 stringtoi(const T* p);
template <typename T> double stringtod(const T* p);

///////////////////////////////////////////////////////////////////////////////
/// number conversion core.
/// don't use directly
///////////////////////////////////////////////////////////////////////////////
template <typename T> const T* _itobase(sint64 value, T* buf, size_t base, size_t& len, bool _signed, T pluschar);






///////////////////////////////////////////////////////////////////////////////
/// predeclaration
///////////////////////////////////////////////////////////////////////////////
template <typename T> class stringinterface;
template <typename T> class stringoperator;
template <typename T> class basestring;			// dynamic buffer without smart pointer
template <typename T> class staticstring;			// external static buffer
template <typename T> class string;				// dynamic buffer with smart pointer, copy-on-write
template <typename T> class globalstring;			// dynamic buffer with smart pointer, copy-on-write
template <typename T> class substring;				// contains pointer to string, implements allocator interface


template <typename T, typename X> class formatstr;
template <typename X>             class formatobj;




///////////////////////////////////////////////////////////////////////////////
/// error message.
void string_error(const char*errmsg);


class stringconfig
{
public:
	static int  default_precision;
	static int  default_width;
	static bool default_left_allign;
	static bool default_double_alternate;
	static bool default_hexoct_alternate;
};

///////////////////////////////////////////////////////////////////////////////
/// basic string interface.
/// defines all reading access on a string object
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class stringinterface : public allocator<T>, public elaborator_st<T>, public stringconfig
{
protected:
	///////////////////////////////////////////////////////////////////////////
	/// internal StringConstant aquire
	const T*getStringConstant(size_t i) const;

	stringinterface<T>()			{}
public:
	virtual ~stringinterface<T>()	{}


	virtual operator const T*() const =0;
	virtual const T* c_str() const =0;
	///////////////////////////////////////////////////////////////////////////
	/// search functions
	///////////////////////////////////////////////////////////////////////////
	bool			findnext(const stringinterface<T>& pattern, size_t &startpos, bool ignorecase=false) const;
	vector<size_t>	findall (const stringinterface<T>& pattern, bool ignorecase=false) const;


	///////////////////////////////////////////////////////////////////////////
	/// readable array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T operator[](size_t inx) const = 0;
	virtual const T operator[](int inx) const=0;



	///////////////////////////////////////////////////////////////////////////
	/// compare functions
	///////////////////////////////////////////////////////////////////////////
	virtual int compareTo(const stringinterface<T>& s) const=0;
	virtual int compareTo(const T*c) const=0;
	virtual int compareTo(const T c) const=0;

	/////////////////////////////////////////////////////////////////
	/// boolean operators
	/////////////////////////////////////////////////////////////////
	virtual bool operator ==(const stringinterface<T> &s)	const { return (this->compareTo( s ) == 0); }
	virtual bool operator !=(const stringinterface<T> &s)	const { return (this->compareTo( s ) != 0); }
	virtual bool operator <=(const stringinterface<T> &s)	const { return (this->compareTo( s ) <= 0); }
	virtual bool operator >=(const stringinterface<T> &s)	const { return (this->compareTo( s ) >= 0); }
	virtual bool operator < (const stringinterface<T> &s)	const { return (this->compareTo( s ) <  0); }
	virtual bool operator > (const stringinterface<T> &s)	const { return (this->compareTo( s ) >  0); }

	virtual bool operator ==(const T c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const T *c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T *c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T *c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T *c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T *c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T *c) 	const { return (this->compareTo( c ) >  0); }

	friend bool operator ==(const T *c, const stringinterface<T> &s)	{return (s==c);}
	friend bool operator !=(const T *c, const stringinterface<T> &s)	{return (s!=c);}
	friend bool operator <=(const T *c, const stringinterface<T> &s)	{return (s>=c);}
	friend bool operator >=(const T *c, const stringinterface<T> &s)	{return (s<=c);}
	friend bool operator < (const T *c, const stringinterface<T> &s)	{return (s> c);}
	friend bool operator > (const T *c, const stringinterface<T> &s)	{return (s< c);}
};


///////////////////////////////////////////////////////////////////////////////
/// basic string functions
///////////////////////////////////////////////////////////////////////////////

/// some Unix systems do not accept NULL
inline size_t hstrlen(const char* p)		{ return p == NULL ? 0 : strlen(p); }
inline size_t hstrlen(const wchar_t* p)		{ return p == NULL ? 0 : wcslen(p); }
template<typename T> inline size_t hstrlen(const stringinterface<T>& p)	{ return p.length(); }






///////////////////////////////////////////////////////////////////////////////
/// basic string operator.
/// defines write access on a string object
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class stringoperator : public stringinterface<T>
{
protected:
	stringoperator<T>()				{}
public:
	virtual ~stringoperator<T>()	{}


	///////////////////////////////////////////////////////////////////////////
	/// standard functions
	///////////////////////////////////////////////////////////////////////////
	virtual void clear() =0;
	virtual void empty() =0;
	/// remove everything from ix for sz length
	virtual void clear(size_t inx, size_t len) =0;
	/// remove everything exept from 0 for sz length
	virtual void truncate(size_t sz) =0;
	/// remove everything exept from ix for sz length
	virtual void truncate(size_t ix, size_t sz) =0;
	/// removes sz elements from the end
	virtual void strip(size_t sz) =0;
	/// removes sz elements from the start
	void skip(size_t sz)	{ clear(0,sz); }

	/// check if requestes overall size is available
	virtual bool checksize(size_t sz) =0;


	///////////////////////////////////////////////////////////////////////////
	/// templated single element append function
	///////////////////////////////////////////////////////////////////////////
	template<typename X>
	const stringoperator<T>& assign(const X& t)
	{
		this->clear();
		return this->append(t);
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated add-assignment operators
	///////////////////////////////////////////////////////////////////////////
	template <typename X>
	const stringoperator<T>& operator +=(const X& t)
	{
		return this->append(t);
	}


	///////////////////////////////////////////////////////////////////////////
	/// assignment function
	///////////////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& assign(const stringinterface<T>& t, size_t slen) = 0;
	virtual const stringoperator<T>& assign(const T* c, size_t slen) = 0;
	virtual const stringoperator<T>& assign(const T& c, size_t slen) = 0;
	virtual const stringoperator<T>& assign(const T ch)=0;
	virtual const stringoperator<T>& assign(const T* c1, size_t len1, const T* c2, size_t len2) = 0;
	virtual const stringoperator<T>& assign_tolower(const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& assign_toupper(const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& assign_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL) = 0;

	///////////////////////////////////////////////////////////////////////////
	/// single element append function
	///////////////////////////////////////////////////////////////////////////
	virtual stringoperator<T>& append(const stringinterface<T>& t) =0;
	virtual stringoperator<T>& append(const T* ip) =0;
	virtual stringoperator<T>& append(const T ch) =0;
	virtual stringoperator<T>& append(const int v) =0;
	virtual stringoperator<T>& append(const unsigned int v) =0;
	virtual stringoperator<T>& append(const long v) =0;
	virtual stringoperator<T>& append(const unsigned long v) =0;
	virtual stringoperator<T>& append(const int64 v) =0;
	virtual stringoperator<T>& append(const uint64 v) =0;
	virtual stringoperator<T>& append(const double v) =0;

	template<typename X>
	stringoperator<T>& append(const formatstr<T,X>& t)
	{
		dsprintf<T>(*this, t.fmt(), t.val());
		return *this;
	}
	template<typename X>
	stringoperator<T>& append(const formatobj<X>& t)
	{
		t.print( *this );
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// append function
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& append(const stringinterface<T>& t, size_t slen) = 0;
	virtual const stringoperator<T>& append(const T* c, size_t slen) = 0;
	virtual const stringoperator<T>& append(const T& c, size_t slen) = 0;
	virtual const stringoperator<T>& append_tolower(const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& append_toupper(const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& append_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL) = 0;

	/////////////////////////////////////////////////////////////////
	/// insert function
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& insert(size_t pos, const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& insert(size_t pos, const T& c, size_t slen=1) = 0;

	/////////////////////////////////////////////////////////////////
	/// replace
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~((size_t)0)) = 0;
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1) = 0;

	/////////////////////////////////////////////////////////////////
	/// change case
	/////////////////////////////////////////////////////////////////
	virtual void tolower() =0;
	virtual void toupper() =0;
	///////////////////////////////////////////////////////////////////////////
	/// Trim's
	///////////////////////////////////////////////////////////////////////////
	virtual void ltrim() =0;
	virtual void rtrim() =0;
	virtual void trim() =0;
	virtual void itrim(bool removeall=false) =0;
};

///////////////////////////////////////////////////////////////////////////////
/// case and trim
///////////////////////////////////////////////////////////////////////////////
template<typename T> const T* toupper(T* str);
template<typename T> const T* tolower(T* str);
template<typename T> const T* ltrim(T* str);
template<typename T> const T* rtrim(T* str);
template<typename T> const T* trim(T* str);
template<typename T> const T* itrim(T* str, bool removeall=false);

template<typename T> const stringoperator<T>& toupper(stringoperator<T>& str)	{ return str.toupper(); return str; }
template<typename T> const stringoperator<T>& tolower(stringoperator<T>& str)	{ return str.tolower(); return str; }
template<typename T> const stringoperator<T>& ltrim(stringoperator<T>& str)	{ return str.ltrim(); return str; }
template<typename T> const stringoperator<T>& rtrim(stringoperator<T>& str)	{ return str.rtrim(); return str; }
template<typename T> const stringoperator<T>& trim(stringoperator<T>& str)		{ return str.trim(); return str; }
template<typename T> const stringoperator<T>& itrim(stringoperator<T>& str)	{ return str.itrim(); return str; }


///////////////////////////////////////////////////////////////////////////////
/// basic number conversion
///////////////////////////////////////////////////////////////////////////////
template <typename T> void _itostring(stringoperator<T>& result, sint64 value, size_t base, bool _signed, size_t width=0, bool left=true, T padchar=0, T pluschar=0);
template <typename T> void _hextostring(stringoperator<T>& result, uint64 value, size_t width=0, bool left=true, T padchar=0, bool alt=false, bool low=true);
template <typename T> void _octtostring(stringoperator<T>& result, uint64 value, size_t width=0, bool left=true, T padchar=0, bool alt=false);
template <typename T> void _ftostring(stringoperator<T>& result, double value, int prec, T format='g', size_t width=0, bool left=true, T padchar=0, T pluschar=0, bool alt=false);






///////////////////////////////////////////////////////////////////////////////
/// basic string template implementations.
/// writable part
///////////////////////////////////////////////////////////////////////////////
template <typename T=char, typename A=allocator_ws_dy<T> >
class TString : public A, public stringoperator<T>
{
	friend void test_stringbuffer(void);
protected:
	///////////////////////////////////////////////////////////////////////////
	/// protected constructors, only derived can create
	///////////////////////////////////////////////////////////////////////////
	TString<T,A>()									{}
	TString<T,A>(size_t sz) : A(sz)					{}
	TString<T,A>(T* buf, size_t sz) : A(buf, sz)	{}

	///////////////////////////////////////////////////////////////////////////
	// M$ VisualC does not allow template generated copy/assignment operators
	// but always generating defaults if not explicitely defined
	// mixing template and explicit definition does also not work because
	// it "detects" ambiguities in this case
//	template <typename A>
//	TString<T,A>(const TString<T, A>& b) : alloc(b.length())
//	{
//		this->append(b);
//	}
	/// declare standard copy constructor
	TString<T,A>(const TString<T,A>& b) : A(b.length())
	{
		this->append(b);
	}
	/// declare copy of base class types
	TString<T,A>(const stringinterface<T>& t) : A(t.length())
	{
		this->append(t);
	}

public:
	virtual ~TString<T,A>()											{}


	///////////////////////////////////////////////////////////////////////////
	/// assignment operators
	///////////////////////////////////////////////////////////////////////////
// M$ VisualC does not allow template generated copy/assignment operators
// but always generating defaults if not explicitely defines
// mixing template and explicit definition does also not work because
// it "detects" ambiguities in this case
//	template <typename X>
//	const TString<T,A>& operator =(const X& t)
//	{
//		this->cWpp = this->cBuf;
//		return *this<<t;
//	}

	// only default copy assignment usable
	const TString<T,A>& operator=(const TString<T,A>& t)	{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const stringinterface<T>& t){ this->cWpp = this->cBuf; this->append(t); return *this; }

	const TString<T,A>& operator=(const T* t)				{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const T t)				{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const int t)				{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const unsigned int t)		{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const long t)				{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const unsigned long t)	{ this->cWpp = this->cBuf; this->append(t); return *this; }
	const TString<T,A>& operator=(const double t)			{ this->cWpp = this->cBuf; this->append(t); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// base class functions
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if( !this->cBuf && const_cast<TString<T,A>*>(this)->checkwrite(1) )
			*(const_cast<TString<T,A>*>(this)->cWpp)=0; 
		return this->cBuf; 
	}
	virtual operator const T*() const	{ return this->c_str(); }
	virtual size_t length()	const		{ return this->A::length(); }
	virtual size_t capacity() const		{ return this->A::capacity(); }
	virtual const T* begin() const		{ return this->A::begin(); }
	virtual const T* end() const		{ return this->A::end(); }
	virtual const T* final() const		{ return this->A::final(); }
	virtual       T* begin()			{ return this->A::begin(); }
	virtual       T* end()				{ return this->A::end(); }
	virtual       T* final()			{ return this->A::final(); }

	///////////////////////////////////////////////////////////////////////////
	/// standard functions
	///////////////////////////////////////////////////////////////////////////
	bool is_empty() const
	{
		return this->cWpp==this->cBuf; 
	}

	///////////////////////////////////////////////////////////////////////////
	/// readable array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T operator[](size_t inx) const
	{
	#ifdef CHECK_BOUNDS
		if( this->cBuf+inx > this->cWpp )
			// don't throw here, just return EOS on out of bounds
			return '\0';
	#endif
		return this->cBuf[inx];
	}
	virtual const T operator[](int inx) const	{ return this->operator[]( (size_t)inx ); }


	///////////////////////////////////////////////////////////////////////////
	/// compare functions
	///////////////////////////////////////////////////////////////////////////
	virtual int compareTo(const stringinterface<T>& s) const
	{	
		if(s.c_str() == this->c_str() )
			return 0;
		else if(s.length()>0 && this->length()>0) 
			// compare including eos
			return this->intern_cmp(this->c_str(), s.c_str(), 1+this->length());
		else if(s.length()==0 && this->length()==0)
			return 0;
		else if(s.length()==0)
			return 1;
		else
			return -1;
	}
	virtual int compareTo(const T c) const
	{	
		if(this->length()==1)
			return *this->c_str() - c;
		else if(this->c_str())
			return 1;
		else 
			return -1;
	}
	virtual int compareTo(const T *c) const
	{	
		if(c == this->c_str() )
			return 0;
		else if(c && this->length()>0)
			return this->intern_cmp(this->c_str(), c, 1+this->length());
		else if((!c || *c==0) && this->length()==0)
			return 0;
		else if((!c || *c==0))
			return 1;
		else 
			return -1;
	}

	///////////////////////////////////////////////////////////////////////////
	/// finds
	///////////////////////////////////////////////////////////////////////////
	size_t find_first_of(const T& ch) const
	{
		const T *ptr  = this->begin();
		const T *eptr = this->end();
		if(ptr)
		while(ptr<=eptr)
		{
			if( *ptr == ch )
				return ptr-this->begin();
			++ptr;
		}
		return this->npos;
	}
	size_t find_last_of(const T& ch) const
	{
		const T *ptr  = this->end();
		const T *eptr = this->begin();
		if(ptr)
		while(ptr>=eptr)
		{
			if( *ptr == ch )
				return ptr-this->begin();
			--ptr;
		}
		return this->npos;
	}

	///////////////////////////////////////////////////////////////////////////
	/// hash
	///////////////////////////////////////////////////////////////////////////
	size_t hash() const
	{	
		unsigned long h = 0;
		T*ptr = this->cBuf;
		while(ptr<this->cWpp)
		{
			// hash from stl
			h = 5*h + *ptr++;
			// hash used in ea
			//h = (h*33 + *ptr++) ^ (h>>24);
		}
		return size_t(h);
	}


	///////////////////////////////////////////////////////////////////////////
	/// standard functions
	///////////////////////////////////////////////////////////////////////////
	virtual void clear()				
	{
		this->cWpp=this->cBuf; 
		if(this->cWpp) *this->cWpp=0;
	}
	virtual void empty()
	{
		this->clear(); 
	}
	/// remove everything from ix for sz length
	virtual void clear(size_t inx, size_t len)
	{
		if( this->cBuf+inx < this->cWpp )
		{
			if(this->cBuf+inx+len >= this->cWpp)
				len = this->cWpp-this->cBuf-inx;
			else
				this->intern_move(this->cBuf+inx, this->cBuf+inx+len, this->cWpp-this->cBuf-inx-len);
			this->cWpp -= len;
			*this->cWpp = 0;
		}
	}
	/// remove everything exept from 0 for sz length
	virtual void truncate(size_t sz)
	{
		if( this->cBuf+sz < this->cWpp )
		{	// truncate
			this->cWpp = this->cBuf+sz;
			*this->cWpp = 0;
		}
		else
		{	// expand by padding spaces
			if( this->checkwrite( sz-(this->cWpp-this->cBuf) ) )
			{
				while( this->cBuf+sz > this->cWpp )
					*this->cWpp++ = (T)(' ');
				if(this->cWpp) *this->cWpp = 0;
			}
		}
	}
	/// remove everything exept from ix for sz length
	virtual void truncate(size_t ix, size_t sz)
	{
		if(ix==0)
		{	// normal truncate
			truncate(sz);
		}
		else if( ix > (size_t)(this->cWpp-this->cBuf) )
		{	// cut all off
			this->cWpp = this->cBuf;
			*this->cWpp = 0;
		}
		else if( ix+sz <= (size_t)(this->cWpp-this->cBuf) )
		{	// move and truncate
			this->intern_move(this->cBuf, this->cBuf+ix, sz);
			this->cWpp = this->cBuf+sz;
			*this->cWpp = 0;
		}
		else
		{	// move and expand
			this->intern_move(this->cBuf, this->cBuf+ix, this->cWpp-this->cBuf-ix);
			this->cWpp -= ix;
			if( this->checkwrite( sz-(this->cWpp-this->cBuf) ) )
			{
				while( this->cBuf+sz > this->cWpp )
					*this->cWpp++ = (T)(' ');
			}
			if(this->cWpp) *this->cWpp = 0;
		}
	}
	/// removes sz elements from the end
	virtual void strip(size_t sz)
	{
		if( sz > (size_t)(this->cWpp-this->cBuf) )
		{	// cut all off
			this->cWpp = this->cBuf;
			if(this->cWpp) *this->cWpp = 0;
		}
		else
		{	
			this->cWpp-=sz;
			*this->cWpp = 0;
		}
	}
	/// checks if overall site can be granted
	virtual bool checksize(size_t sz)	
	{
		return (sz>(size_t)(this->cEnd-this->cBuf)) ? checkwrite(sz-(this->cEnd-this->cBuf)) : true; 
	}

	///////////////////////////////////////////////////////////////////////////
	/// writable array access
	///////////////////////////////////////////////////////////////////////////
	T& operator[](size_t inx)
	{
		// merge with exeption code
		if( this->cEnd < this->cWpp )	// const string
		{
			static T dummy;
			return dummy=0;
		}
	#ifdef CHECK_BOUNDS
		if( inx > (size_t)(this->cWpp-this->cBuf) )
		{
			string_error("string: access out of bound");
	#ifndef CHECK_EXCEPTIONS
			// provide some fallback in case of out of bound
			static T dummy;
			return dummy=0;
	#endif
		}
	#endif
		return this->cBuf[inx];
	}




	///////////////////////////////////////////////////////////////////////////
	/// templated single element append function
	///////////////////////////////////////////////////////////////////////////
	template<typename X>
	const stringoperator<T>& assign(const X& t)
	{
		this->clear();
		return this->append(t);
	}
	///////////////////////////////////////////////////////////////////////////
	/// templated add-assignment operators
	///////////////////////////////////////////////////////////////////////////
	template <typename X>
	const stringoperator<T>& operator +=(const X& t)
	{
		return this->append(t);
	}

	///////////////////////////////////////////////////////////////////////////
	/// assignment function
	///////////////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& assign(const stringinterface<T>& t, size_t slen)
	{
		this->cWpp = this->cBuf;
		return this->append(t,slen);
	}
	virtual const stringoperator<T>& assign(const T* c, size_t slen)
	{
		this->cWpp = this->cBuf;
		return this->append(c,slen);
	}
	virtual const stringoperator<T>& assign(const T& ch, size_t slen)
	{
		this->cWpp = this->cBuf;
		return this->append(ch,slen);
	}
	virtual const stringoperator<T>& assign(const T ch)
	{
		this->cWpp = this->cBuf;
		return this->append(ch);
	}
	virtual const stringoperator<T>& assign(const T* c1, size_t len1, const T* c2, size_t len2)
	{
		if( !c1 || len1 <= 0 )
			this->assign(c2, len2);
		else if ( !c2 || len2 <= 0 )
			this->assign(c1, len1);
		else if( this->checkwrite(len1 + len2) )
		{
			this->cWpp = this->cBuf;

			const T* epp=c1+len1;
			while( *c1 && c1 < epp )
				*this->cWpp++ = *c1++;

			epp=c2+len2;
			while( *c2 && c2 < epp )
				*this->cWpp++ = *c2++;

			if(this->cWpp) *this->cWpp=0;
		}
		return *this;
	}
	virtual const stringoperator<T>& assign_tolower(const T* c, size_t slen=~((size_t)0))
	{
		this->cWpp = this->cBuf;
		return this->append_tolower(c,slen);
	}
	virtual const stringoperator<T>& assign_toupper(const T* c, size_t slen=~((size_t)0))
	{
		this->cWpp = this->cBuf;
		return this->append_toupper(c,slen);
	}
	virtual const stringoperator<T>& assign_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL)
	{
		this->cWpp = this->cBuf;
		return this->assign_process(c,slen);
	}

	///////////////////////////////////////////////////////////////////////////
	/// single element append function
	///////////////////////////////////////////////////////////////////////////
	virtual stringoperator<T>& append(const stringinterface<T>& t)
	{
		if( t.length() )
		{
			if( t.length()<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( t.length() ) )
			{
				this->intern_copy(this->cWpp, t, t.length());
				this->cWpp += t.length();
				if(this->cWpp) *this->cWpp=0;
			}
		}
		return *this;
	}
	virtual stringoperator<T>& append(const T* ip)
	{
		if(ip)
		{
			while( *ip && this->checkwrite(1) )
				*this->cWpp++ = *ip++;
			if(this->cWpp) *this->cWpp=0;
		}
		return *this;
	}
	virtual stringoperator<T>&  append(const T ch)
	{
		if(ch) // dont append a eos
		{
			if( this->cWpp+1<this->cEnd || this->checkwrite( 1 ) )
			{
				*this->cWpp++ = ch;
				*this->cWpp = 0;
			}
		}
		return *this;
	}
	virtual stringoperator<T>& append(const int v)
	{
		_itostring<T>(*this, sint64(v), 10, true,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const unsigned int v)
	{
		_itostring<T>(*this, uint64(v), 10, false,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const long v)
	{
		_itostring<T>(*this, sint64(v), 10, true,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const unsigned long v)
	{
		_itostring<T>(*this, uint64(v), 10, false,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const int64 v)
	{
		_itostring<T>(*this, sint64(v), 10, true,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const uint64 v)
	{
		_itostring<T>(*this, uint64(v), 10, false,  0, true, ' ', '\0');
		return *this;
	}
	virtual stringoperator<T>& append(const double v)
	{
		_ftostring<T>(*this, v, -1, 'g', 0, true, ' ', '\0', stringconfig::default_double_alternate);
		return *this;
	}
	template<typename X>
	stringoperator<T>& append(const formatstr<T,X>& t)
	{
		dsprintf<T>(*this, t.fmt(), t.val());
		return *this;
	}
	template<typename X>
	stringoperator<T>& append(const formatobj<X>& t)
	{
		t.print( *this );
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// append with length
	///////////////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& append(const stringinterface<T>& t, size_t slen)
	{
		if( slen > t.length() )
			slen = t.length();
		if(slen)
		{
			if( slen<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( slen ) )
			{
				this->intern_copy(this->cWpp, t, slen);
				this->cWpp += slen;
				if(this->cWpp) *this->cWpp=0;
			}
		}
		return *this;
	}
	virtual const stringoperator<T>& append(const T* c, size_t slen)
	{
		if(c)
		{
			const T* epp = (c+slen<c)?(T*)(~((size_t)0)):c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cWpp++ = *c++;
			if(this->cWpp) *this->cWpp=0;
		}
		return *this;
	}
	virtual const stringoperator<T>& append(const T& ch, size_t slen)
	{
		if(ch) // dont append a eos
		{
			if( slen<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( slen ) )
			{
				T* epp = this->cWpp+slen;
				while(this->cWpp<epp)
					*this->cWpp++ = ch;
				if(this->cWpp) *this->cWpp = 0;
			}
		}
		return *this;
	}
	virtual const stringoperator<T>& append_tolower(const T* c, size_t slen=~((size_t)0))
	{
		if(c)
		{
			const T* epp = (c+slen<c)?(T*)(~((size_t)0)):c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cWpp++ = locase(*c++);
		}
		if( this->cWpp ) *this->cWpp=0;
		return *this;
	}
	virtual const stringoperator<T>& append_toupper(const T* c, size_t slen=~((size_t)0))
	{
		if(c)
		{
			const T* epp = (c+slen<c)?(T*)(~((size_t)0)):c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cWpp++ = upcase(*c++);
		}
		if( this->cWpp ) *this->cWpp=0;
		return *this;
	}
	virtual const stringoperator<T>& append_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL)
	{
		if(!func)
			return this->assign(c,slen);
		else if(c)
		{
			const T* epp = (c+slen<c)?(T*)(~((size_t)0)):c+slen;
			while(*c && c<epp && this->checkwrite(1) )
				*this->cWpp++ = func(*c++);
		}
		if( this->cWpp ) *this->cWpp=0;
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	/// insert
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~((size_t)0))
	{
		if( pos > this->length() )
		{
			this->append(t, slen);
			return *this;
		}
		else
		{
			if( slen > t.length() )
				slen = t.length();
			if( slen<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( slen ) )
			{
				this->intern_move(this->cBuf+pos+slen, this->cBuf+pos, this->cWpp-this->cBuf-pos);
				this->intern_copy(this->cBuf+pos, t, slen);

				this->cWpp += slen;
				*this->cWpp = 0;
			}
		}
		return *this;
	}
	
	virtual const stringoperator<T>& insert(size_t pos, const T* c, size_t slen=~((size_t)0))
	{
		if(c)
		{
			if( pos > this->length() )
			{
				this->append(c, slen);
			}
			else
			{
				const T* ep =c;
				while(*ep) ep++;
				if( slen > (size_t)(ep-c) )
					slen = ep-c;
				if( slen<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( slen ) )
				{
					this->intern_move(this->cBuf+pos+slen, this->cBuf+pos, this->cWpp-this->cBuf-pos);
					this->intern_copy(this->cBuf+pos, c, this->cWpp-this->cBuf-pos);

					this->cWpp += slen;
					*this->cWpp = 0;
				}
			}
		}
		return *this;
	}
	virtual const stringoperator<T>& insert(size_t pos, const T& ch, size_t slen=1)
	{
		if(ch) // dont append a eos
		{
			if( pos > this->length() )
			{
				this->append(ch, slen);
			}
			else
			{
				if( slen<(size_t)(this->cEnd-this->cWpp) || this->checkwrite( slen ) )
				{
					T* ipp = this->cBuf+pos;
					T* epp = this->cBuf+pos+slen;
					this->intern_move(epp, ipp, this->cWpp-this->cBuf-pos);
					while(ipp<epp)
						*ipp++ = ch;

					this->cWpp += slen;
					*this->cWpp = 0;
				}
			}
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	/// replace
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~((size_t)0))
	{
		if( pos > this->length() )
		{
			this->append(t, slen);
		}
		else
		{
			if( pos+tlen > this->length() )
				tlen = this->length()-pos;
			if( slen > t.length() )
				slen = t.length();
			if( slen<=tlen || this->checkwrite( slen-tlen ) )
			{
				this->intern_move(this->cBuf+pos+slen, this->cBuf+pos+tlen, this->cWpp-this->cBuf-pos-tlen);
				this->intern_copy(this->cBuf+pos, t, slen);

				this->cWpp += slen-tlen;
				if(this->cWpp) *this->cWpp = 0;
			}
		}
		return *this;
	}
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~((size_t)0))
	{
		if(c)
		{
			if( pos > this->length() )
			{
				this->append(c, slen);
			}
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
					this->intern_move(this->cBuf+pos+slen, this->cBuf+pos+tlen, this->cWpp-this->cBuf-pos-tlen);
					this->intern_copy(this->cBuf+pos, c, slen);

					this->cWpp += slen-tlen;
					if(this->cWpp) *this->cWpp = 0;
				}
			}
		}
		return *this;
	}
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1)
	{
		if(ch) // dont append a eos
		{
			if( pos > this->length() )
			{
				this->append(ch, slen);
			}
			else
			{
				if( pos+tlen > this->length() )
					tlen = this->length()-pos;
				
				if( slen<=tlen || this->checkwrite( slen-tlen ) )
				{
					T* ipp = this->cBuf+pos;
					T* epp = this->cBuf+pos+slen;
					this->intern_move(epp, ipp+tlen, this->cWpp-this->cBuf-pos-tlen);
					while(ipp<epp)
						*ipp++ = ch;

					this->cWpp += slen-tlen;
					if(this->cWpp) *this->cWpp = 0;
				}
			}
		}
		return *this;
	}



	///////////////////////////////////////////////////////////////////////////
	/// change case
	///////////////////////////////////////////////////////////////////////////
	virtual void tolower()
	{
		T* ipp = this->cBuf;
		while( ipp < this->cWpp )
		{
			*ipp = locase(*ipp);
			ipp++;
		}
	}
	virtual void toupper()
	{
		T* ipp = this->cBuf;
		while( ipp < this->cWpp )
		{
			*ipp = upcase(*ipp);
			ipp++;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// trim's
	///////////////////////////////////////////////////////////////////////////
	/// remove whitespaces from left side
	virtual void ltrim()
	{
		T* ipp = this->cBuf;
		while( ipp < this->cWpp && stringcheck::isspace(*ipp) )
			ipp++;
		if(ipp!=this->cBuf)
		{
			this->intern_move(this->cBuf, ipp, this->cWpp-ipp);
			*this->cWpp=0;
		}
	}
	/// remove whitespaces from right side
	virtual void rtrim()
	{
		T* ipp = this->cWpp-1;
		T* kpp = this->cWpp-1;
			
		while( ipp>this->cBuf && stringcheck::isspace(*ipp) )
			ipp--;
		if( ipp != kpp )
		{
			this->cWpp=ipp+1;
			*this->cWpp=0;
		}
	}
	/// remove whitespaces from both sides
	virtual void trim()
	{
		T *src=this->cBuf, *tar=this->cBuf, *mk=NULL;
		while(*src && stringcheck::isspace(*src) )
			src++;
		while(*src)
		{
			mk = ( stringcheck::isspace(*src) )?mk?mk:tar:NULL;
			*tar++ = *src++;
		}
		this->cWpp = (mk) ? mk : tar;
		if(this->cWpp) *this->cWpp=0;
	}
	/// remove whitespaces from both sides, combine or remove speces inside
	virtual void itrim(bool removeall=false)
	{
		T *src=this->cBuf, *tar=this->cBuf, mk=0;
		while(*src && stringcheck::isspace(*src) )
			src++;
		while(*src)
		{
			if( stringcheck::isspace(*src) )
				mk=*src, src++;
			else
			{
				if( mk && !removeall )
					*tar++=mk, mk=0;
				*tar++ = *src++;
			}
		}
		this->cWpp = tar;
		if(this->cWpp) *this->cWpp=0;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// basestring class.
/// implements a dynamic in-place buffer
/// type construction always copies the data to a newly created buffer
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class basestring : public TString< T, allocator_ws_dy<T> >
{
public:
	basestring()
	{}
	basestring<T>(const basestring<T>& t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	basestring<T>(const stringinterface<T>& t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	basestring<T>(const T* t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	basestring<T>(const T* t, size_t sz) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t, sz);
	}
	explicit basestring<T>(T t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	explicit basestring<T>(int t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	explicit basestring<T>(unsigned int t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}
	explicit basestring<T>(double t) : TString< T, allocator_ws_dy<T> >()
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
	}

	virtual ~basestring()	{}


	///////////////////////////////////////////////////////////////////////////
	/// assignments
	///////////////////////////////////////////////////////////////////////////
	template<typename X>
	const basestring<T>& operator=(const X& t)
	{
		this->TString< T, allocator_ws_dy<T> >::assign(t);
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// staticstring class.
/// implements an external static buffer
/// type construction only possible with either a buffer which is used for writing
/// or with a const cstring which is then used as a constant without write possibility
///////////////////////////////////////////////////////////////////////////////
template<typename T=char>
class staticstring : public TString< T, allocator_ws_st<T> >
{
	staticstring<T>();
	staticstring<T>(const staticstring<T>&);
	const staticstring& operator=(const staticstring& t);
public:
	staticstring<T>(T* buf, size_t sz)
		: TString< T, allocator_ws_st<T> >(buf, sz)
	{}
	virtual ~staticstring()	{}

	///////////////////////////////////////////////////////////////////////////
	/// array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if(!this->cBuf && const_cast<staticstring<T>*>(this)->checkwrite(1))
			*this->cWpp=0; 
		return this->cBuf; 
	}

	///////////////////////////////////////////////////////////////////////////
	/// assignments
	///////////////////////////////////////////////////////////////////////////
	template<typename X>
	const staticstring<T>& operator=(const X& t)
	{
		this->TString< T, allocator_ws_st<T> >::assign(t);
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// constant string class.
/// implements an external static buffer
/// type construction only possible with either a buffer which is used for writing
/// or with a const cstring which is then used as a constant without write possibility
///////////////////////////////////////////////////////////////////////////////
template<typename T=char>
class conststring : public stringinterface<T>, public allocator_ws_st<T>
{
	// prevent default construction
	conststring<T>();
public:
	explicit conststring<T>(const T* cstr)
		: allocator_ws_st<T>(const_cast<T*>(cstr), 0)
	{	// set up a zero-length buffer, 
		// where the length pointer is set at position
		if(cstr) this->cWpp+=hstrlen(cstr);
	}
	
	virtual ~conststring()	{}

	/// copy/assign
	conststring<T>(const conststring<T>& c)
	{
		this->cBuf= c.cBuf;
		this->cWpp= c.cWpp;
		this->cEnd= c.cEnd;
	}
	const conststring& operator=(const conststring& c)
	{
		this->cBuf= c.cBuf;
		this->cWpp= c.cWpp;
		this->cEnd= c.cEnd;
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// supported assignments.
	/// be careful with assigning stuff here
	///////////////////////////////////////////////////////////////////////////
	const conststring& operator=(const char* t)
	{	// set up a zero-length buffer, 
		this->cBuf = this->cWpp = this->cEnd = const_cast<T*>(t);
		if(t) this->cWpp+=hstrlen(t);
		return *this;
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// prevent direct asignment of stringclass content
	const conststring& operator=(const stringinterface<T>& t)
	{ return *this; }
public:

	///////////////////////////////////////////////////////////////////////////
	/// base class functions
	///////////////////////////////////////////////////////////////////////////
	virtual const T* c_str() const		
	{
		if(!this->cBuf)
			const_cast<conststring<T>*>(this)->cBuf=
			const_cast<conststring<T>*>(this)->cWpp=
			const_cast<conststring<T>*>(this)->cEnd=
			const_cast<T*>(this->getStringConstant(0));
		return this->cBuf;
	}
	virtual operator const T*() const	{ return this->c_str(); }
	virtual size_t length()	const		{ return this->allocator_ws_st<T>::length(); }
	virtual size_t capacity() const		{ return this->allocator_ws_st<T>::capacity(); }
	virtual const T* begin() const		{ return this->allocator_ws_st<T>::begin(); }
	virtual const T* end() const		{ return this->allocator_ws_st<T>::end(); }
	virtual const T* final() const		{ return this->allocator_ws_st<T>::final(); }
	virtual       T* begin()			{ return this->allocator_ws_st<T>::begin(); }
	virtual       T* end()				{ return this->allocator_ws_st<T>::end(); }
	virtual       T* final()			{ return this->allocator_ws_st<T>::final(); }

	///////////////////////////////////////////////////////////////////////////
	/// standard functions
	///////////////////////////////////////////////////////////////////////////
	virtual void clear()
	{
		this->cWpp=this->cBuf=this->cEnd=NULL; 
	}
	virtual bool is_empty() const
	{
		return this->cWpp==this->cBuf; 
	}

	///////////////////////////////////////////////////////////////////////////
	/// readable array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T operator[](size_t inx) const
	{
	#ifdef CHECK_BOUNDS
		if( this->cBuf+inx > this->cWpp )
			// don't throw here, just return EOS on out of bounds
			return '\0';
	#endif
		return this->cBuf[inx];
	}
	virtual const T operator[](int inx) const	{ return this->operator[]( (size_t)inx ); }




	///////////////////////////////////////////////////////////////////////////
	/// compare functions
	///////////////////////////////////////////////////////////////////////////
	virtual int compareTo(const stringinterface<T>& s) const
	{	
		if(s.c_str() == this->c_str() )
			return 0;
		else if(s.length()>0 && this->length()>0) 
			// compare including eos
			return this->intern_cmp(this->c_str(), s.c_str(), 1+this->length());
		else if(s.length()==0 && this->length()==0)
			return 0;
		else if(s.length()==0)
			return 1;
		else
			return -1;
	}
	virtual int compareTo(const T c) const
	{	
		if(this->length()==1)
			return *this->c_str() - c;
		else if(this->c_str())
			return 1;
		else 
			return -1;
	}
	virtual int compareTo(const T *c) const
	{	
		if(c == this->c_str() )
			return 0;
		else if(c && this->length()>0)
			return this->intern_cmp(this->c_str(), c, 1+this->length());
		else if((!c || *c==0) && this->length()==0)
			return 0;
		else if((!c || *c==0))
			return 1;
		else 
			return -1;
	}

	///////////////////////////////////////////////////////////////////////////
	/// finds
	///////////////////////////////////////////////////////////////////////////
	size_t find_first_of(const T& ch) const
	{
		const T *ptr  = this->begin();
		const T *eptr = this->end();
		if(ptr)
		while(ptr<=eptr)
		{
			if( *ptr == ch )
				return ptr-this->begin();
			++ptr;
		}
		return this->npos;
	}
	size_t find_last_of(const T& ch) const
	{
		const T *ptr  = this->end();
		const T *eptr = this->begin();
		if(ptr)
		while(ptr>=eptr)
		{
			if( *ptr == ch )
				return ptr-this->begin();
			--ptr;
		}
		return this->npos;
	}

	///////////////////////////////////////////////////////////////////////////
	/// hash
	///////////////////////////////////////////////////////////////////////////
	size_t hash() const
	{	
		unsigned long h = 0;
		T*ptr = this->cBuf;
		while(ptr<this->cWpp)
		{
			// hash from stl
			h = 5*h + *ptr++;
			// hash used in ea
			//h = (h*33 + *ptr++) ^ (h>>24);
		}
		return size_t(h);
	}

};




///////////////////////////////////////////////////////////////////////////////
/// main string class.
/// derives internally from basestring and inherits it's dynamic buffer
/// additionally it implements a smart pointer with copy-on-write semantic
/// so that string copy operations are basically reduced to copying a pointer
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class string : public stringoperator<T>
{
	friend class substring<T>;
private:
	/////////////////////////////////////////////////////////////////
	/// internal string smart pointer object
	/////////////////////////////////////////////////////////////////
	class ptrstring : public basestring<T>
	{
	private:
		friend class string<T>;

		/// reference counter
		unsigned int cRefCnt;

		/// construction
		ptrstring() : cRefCnt(1)	{}
		ptrstring(const ptrstring& b) : basestring<T>(b), cRefCnt(1)	{}
		ptrstring(const basestring<T>& b) : basestring<T>(b), cRefCnt(1)	{}

	public:
		virtual ~ptrstring()	{}

	private:
		template<typename X>
		const ptrstring& operator=(const X& t)
		{
			this->basestring<T>::operator=(t);
			return *this;
		}

		/// access on the smart pointer
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
	/// the smart pointer itself
	/////////////////////////////////////////////////////////////////
	ptrstring *cCountObj;

	/////////////////////////////////////////////////////////////////
	/// allocation of the pointer
	/////////////////////////////////////////////////////////////////
	virtual ptrstring *createpointer() const throw()
	{
		return new ptrstring();
	}
	virtual ptrstring *createpointer(const ptrstring& old) const throw()
	{
		return new ptrstring(old);
	}
	/////////////////////////////////////////////////////////////////
	/// virtual access functions to the smart pointer
	/////////////////////////////////////////////////////////////////
	// !!copy-on-write
	virtual const basestring<T>& readaccess() const	
	{ 
		this->checkpointer();
		return *this->cCountObj;
	}
	virtual       basestring<T>& writeaccess(bool keep=true)
	{
		if(this->cCountObj)
			this->make_unique(keep);
		else
			this->cCountObj = createpointer();	
		return *this->cCountObj;
	}
	/////////////////////////////////////////////////////////////////
	/// aquire/release the pointer
	/////////////////////////////////////////////////////////////////
	void acquire(const string<T>& r) throw()
	{	// check if not already pointing to the same object
		if( this->cCountObj != r.cCountObj ||  NULL==this->cCountObj )
		{	// save the current pointer
			ptrstring *old = this->cCountObj;
			// aquite and increment the given pointer
			if( NULL==r.cCountObj )
			{	// new empty counter to link both pointers
				const_cast<string<T>&>(r).cCountObj = this->createpointer();
			}
			this->cCountObj = r.cCountObj->aquire();
			// release the old thing
			if(old) old->release();
		}
	}
	void release()
	{	// decrement the count, clear the handle
		if(this->cCountObj) this->cCountObj = this->cCountObj->release();
	}
	/////////////////////////////////////////////////////////////////
	/// control of the smart pointer
	/////////////////////////////////////////////////////////////////
	void checkpointer() const
	{
		if(!this->cCountObj)
			const_cast< string* >(this)->cCountObj = this->createpointer();
		// no need to aquire, is done on reference creation
	}
	bool isunique()	const throw()
	{
		return (this->cCountObj) ? (this->cCountObj->cRefCnt == 1):true;
	}
	bool make_unique(bool keep=true)
	{
		if( !this->isunique() )
		{
			ptrstring *cnt = (this->cCountObj && keep) ? 
								this->createpointer(*this->cCountObj) : 
								this->createpointer();
			this->release();
			this->cCountObj = cnt;
		}
		return true;
	}

public:
	/////////////////////////////////////////////////////////////////
	/// public pointer functions
	/////////////////////////////////////////////////////////////////
	const size_t getRefCount() const
	{
		return (this->cCountObj) ? this->cCountObj->cRefCnt : 1; 
	}
	bool is_sameRef(const string<T>& str) const
	{
		return ( this->cCountObj && this->cCountObj==str.cCountObj );
	}
	bool is_sameRef(const substring<T>& sub) const
	{
		return ( this->cCountObj && sub.cString && this->cCountObj==sub.cString->cCountObj );
	}
	T* unique()
	{
		// force calling string::writeaccess, 
		// which creates a unique object
		// we then can return also a writable pointer to the string data
		// but don't try to change the size of the string by modifying the eos!!
		return const_cast<T*>(this->string<T>::writeaccess().c_str());
	}
	/////////////////////////////////////////////////////////////////
	/// direct access to the underlying class
	virtual basestring<T>& operator*()	throw()				{ return this->writeaccess(); }
	virtual basestring<T>* operator->()	throw()				{ return &this->writeaccess(); }
	virtual operator const basestring<T>&() const throw()	{ return this->readaccess();  }


	/////////////////////////////////////////////////////////////////
	/// construction/destruction
	/////////////////////////////////////////////////////////////////
	string<T>() : cCountObj(NULL)					{  }
	string<T>(const string& r) : cCountObj(NULL)	{ this->acquire(r); }
	const string<T>& operator=(const string<T>& r)	{ this->acquire(r); return *this; }
	virtual ~string<T>()							{ this->release(); }

	/////////////////////////////////////////////////////////////////
	/// type construction
	/////////////////////////////////////////////////////////////////
	string<T>(const stringinterface<T>& t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const substring<T>& t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const T* t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	string<T>(const T* t, size_t sz) : cCountObj(NULL)
	{
		this->writeaccess().assign(t, sz);
	}
	string<T>(T t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(int t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(unsigned int t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}
	explicit string<T>(double t) : cCountObj(NULL)
	{
		this->writeaccess().assign(t);
	}

	/////////////////////////////////////////////////////////////////
	/// assignment operators
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
	const string<T>& operator =(long t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(unsigned long t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	const string<T>& operator =(double t)
	{
		this->writeaccess(false).assign(t);
		return *this;
	}
	template<typename X>
	const string<T>& operator=(const formatstr<T,X>& t)
	{
		(this->writeaccess(false)) = t;
		return *this;
	}
	template<typename X>
	const string<T>& operator=(const formatobj<X>& t)
	{
		(this->writeaccess(false)) = t;
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// templated add-assignment operators
	///////////////////////////////////////////////////////////////////////////
	template <typename X>
	const stringoperator<T>& operator +=(const X& t)
	{
		return this->writeaccess().append(t);
	}
	/////////////////////////////////////////////////////////////////
	/// add operators.
	/// for right side operations
	/////////////////////////////////////////////////////////////////
	template <typename X>
	string<T> operator +(const X& t)
	{
		string<T> a(*this);
		a.writeaccess().append(t);
		return a;
	}

	///////////////////////////////////////////////////////////////////////////
	/// assignment function
	///////////////////////////////////////////////////////////////////////////
	template<typename X>
	string<T>& assign(const X& t)
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
	virtual const stringoperator<T>& assign(const stringinterface<T>& t, size_t slen)
	{
		this->writeaccess().assign(t, slen);
		return *this;
	}
	virtual const stringoperator<T>& assign(const T* c, size_t len)
	{
		this->writeaccess().assign(c, len);
		return *this;
	}
	virtual const stringoperator<T>& assign(const T& ch, size_t slen)
	{
		this->writeaccess().assign(ch, slen);
		return *this;
	}
	virtual const stringoperator<T>& assign(const T ch)
	{
		this->writeaccess().assign(ch);
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
		return *this;
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
		return *this;
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
		return *this;
	}
	virtual const stringoperator<T>& assign(const T* c1, size_t len1, const T* c2, size_t len2)
	{
		this->writeaccess().assign(c1, len1, c2, len2);
		return *this;
	}
	virtual const stringoperator<T>& assign_tolower(const T* c, size_t len=~((size_t)0))
	{
		this->writeaccess().assign_tolower(c, len);
		return *this;
	}
	virtual const stringoperator<T>& assign_toupper(const T* c, size_t len=~((size_t)0))
	{
		this->writeaccess().assign_toupper(c, len);
		return *this;
	}
	virtual const stringoperator<T>& assign_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL)
	{
		this->writeaccess().assign_process(c,slen,func);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	/// append function
	///////////////////////////////////////////////////////////////////////////
	string<T>& append(const string<T>& t)
	{
		this->writeaccess().append(t.readaccess());
		return *this;
	}
	virtual stringoperator<T>& append(const stringinterface<T>& t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const T* t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const T t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const int t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const unsigned int t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const long t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const unsigned long t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const int64 t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const uint64 t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	virtual stringoperator<T>& append(const double t)
	{
		this->writeaccess().append(t);
		return *this;
	}
	template<typename X>
	string<T>& append(const formatstr<T,X>& t)
	{
		dsprintf<T>(this->writeaccess(), t.fmt(), t.val());
		return *this;
	}
	template<typename X>
	string<T>& append(const formatobj<X>& t)
	{
		t.print( this->writeaccess() );
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// append with length
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& append(const stringinterface<T>& t, size_t slen)
	{
		this->writeaccess().append(t, slen);
		return *this;
	}
	virtual const stringoperator<T>& append(const T* c, size_t slen)
	{
		this->writeaccess().append(c, slen);
		return *this;
	}
	virtual const stringoperator<T>& append(const T& ch, size_t slen)
	{
		this->writeaccess().append(ch, slen);
		return *this;
	}
	virtual const stringoperator<T>& append_tolower(const T* c, size_t slen=~((size_t)0))
	{
		this->writeaccess().append_tolower(c,slen);
		return *this;
	}
	virtual const stringoperator<T>& append_toupper(const T* c, size_t slen=~((size_t)0))
	{
		this->writeaccess().append_toupper(c,slen);
		return *this;
	}
	virtual const stringoperator<T>& append_process(const T* c, size_t slen=~((size_t)0), T (*func)(T)=NULL)
	{
		this->writeaccess().append_process(c,slen,func);
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	/// insert
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& insert(size_t pos, const stringinterface<T>& t, size_t slen=~((size_t)0))
	{
		this->writeaccess().insert(pos, t, slen);
		return *this;
	}
	string<T>& insert(size_t pos, const substring<T>& sub, size_t slen=~((size_t)0))
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
	virtual const stringoperator<T>& insert(size_t pos, const T* c, size_t slen=~((size_t)0))
	{
		this->writeaccess().insert(pos, c, slen);
		return *this;
	}
	virtual const stringoperator<T>& insert(size_t pos, const T& ch, size_t slen=1)
	{
		this->writeaccess().insert(pos, ch, slen);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// replace
	/////////////////////////////////////////////////////////////////
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const stringinterface<T>& t, size_t slen=~((size_t)0))
	{
		this->writeaccess().replace(pos, tlen, t, slen);
		return *this;
	}
	string<T>& replace(size_t pos, size_t tlen, const substring<T>& sub, size_t slen=~((size_t)0))
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
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T* c, size_t slen=~((size_t)0))
	{
		this->writeaccess().replace(pos, tlen, c, slen);
		return *this;
	}
	virtual const stringoperator<T>& replace(size_t pos, size_t tlen, const T ch, size_t slen=1)
	{
		this->writeaccess().replace(pos, tlen, ch, slen);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// standard string functions
	/////////////////////////////////////////////////////////////////
    virtual operator const T*() const	{ return this->readaccess().c_str(); }
	virtual const T* c_str() const		{ return this->readaccess().c_str(); }
	virtual size_t length()	const		{ return this->readaccess().length(); }
	virtual size_t capacity() const		{ return this->readaccess().capacity(); }
	virtual const T* begin() const		{ return this->readaccess().begin(); }
	virtual const T* end() const		{ return this->readaccess().end(); }
	virtual const T* final() const		{ return this->readaccess().final(); }
	virtual       T* begin()			{ return this->writeaccess().begin(); }
	virtual       T* end()				{ return this->writeaccess().end(); }
	virtual       T* final()			{ return this->writeaccess().final(); }


	virtual void clear()						{ this->writeaccess().clear(); }
	virtual void empty()						{ this->writeaccess().clear(); }
	virtual void clear(size_t inx, size_t len)	{ this->writeaccess().clear(inx, len); }
	virtual bool is_empty() const
	{	
		return ( NULL==this->cCountObj || 0==this->readaccess().length() );
	}
	virtual void truncate(size_t sz)			{ this->writeaccess().truncate(sz); }
	virtual void truncate(size_t ix, size_t sz)	{ this->writeaccess().truncate(ix, sz); }
	virtual void strip(size_t sz)				{ this->writeaccess().strip(sz); }
	virtual bool checksize(size_t sz)			{ return this->writeaccess().checksize(sz); }

	/////////////////////////////////////////////////////////////////
	/// Array access.
	/// did not find a way to force the compiler to use the readonly [] operator
	/// in any case where it's not written to the object; instead they always
	/// use the nonconst operator exept the whole object is const,
	/// so we actually do some detour and return a substring which will either 
	/// just return the char on readaccess or call the copy-on-write when writing
	/// unfortunately the cost of this detour is somewhat higher
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
		if( inx<0 )
			return substring<T>(this, 0, 0);
		else if( (size_t)inx >= this->length() )
			return substring<T>(this, this->length()-1, 1);
		else
			return substring<T>(this, inx, 1);
	}
	///////////////////////////////////////////////////////////////////////////
	/// readable array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T operator[](size_t inx) const	{ return this->readaccess()[inx]; }
	virtual const T operator[](int inx) const		{ return this->readaccess()[inx]; }



	/////////////////////////////////////////////////////////////////
	/// returns a string to access the string bound to all pointers
	/// instead of making it copy on write
	globalstring<T> global()	{ return *this; }

	/////////////////////////////////////////////////////////////////
	/// the sub-string operator
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
	string<T> substr(size_t inx, size_t len=1) const
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
	/// comparisons
	/////////////////////////////////////////////////////////////////
	virtual int compareTo(const string<T> &s) const 
	{	// first check if the pointers are identical
		if( cCountObj != s.cCountObj )
			return this->readaccess().compareTo( s.readaccess() );
		return 0;
	}
	virtual int compareTo(const stringinterface<T>& s) const
	{	
		return this->readaccess().compareTo( s );
	}
	virtual int compareTo(const T *c) const
	{	
		return this->readaccess().compareTo( c );
	}
	virtual int compareTo(const T ch) const
	{	
		return this->readaccess().compareTo( ch );
	}

	/////////////////////////////////////////////////////////////////
	/// booleans
	/////////////////////////////////////////////////////////////////
	virtual bool operator ==(const string<T> &s)const { return (this->compareTo( s ) == 0); }
	virtual bool operator !=(const string<T> &s)const { return (this->compareTo( s ) != 0); }
	virtual bool operator <=(const string<T> &s)const { return (this->compareTo( s ) <= 0); }
	virtual bool operator >=(const string<T> &s)const { return (this->compareTo( s ) >= 0); }
	virtual bool operator < (const string<T> &s)const { return (this->compareTo( s ) <  0); }
	virtual bool operator > (const string<T> &s)const { return (this->compareTo( s ) >  0); }

	virtual bool operator ==(const stringinterface<T> &s)	const { return (this->compareTo( s ) == 0); }
	virtual bool operator !=(const stringinterface<T> &s)	const { return (this->compareTo( s ) != 0); }
	virtual bool operator <=(const stringinterface<T> &s)	const { return (this->compareTo( s ) <= 0); }
	virtual bool operator >=(const stringinterface<T> &s)	const { return (this->compareTo( s ) >= 0); }
	virtual bool operator < (const stringinterface<T> &s)	const { return (this->compareTo( s ) <  0); }
	virtual bool operator > (const stringinterface<T> &s)	const { return (this->compareTo( s ) >  0); }

	virtual bool operator ==(const T c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const T *c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T *c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T *c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T *c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T *c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T *c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const basestring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const basestring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const basestring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const basestring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const basestring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const basestring<T>& c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const staticstring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const staticstring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const staticstring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const staticstring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const staticstring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const staticstring<T>& c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const conststring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const conststring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const conststring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const conststring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const conststring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const conststring<T>& c) 	const { return (this->compareTo( c ) >  0); }


	//////////////////////////////////////////////////////
	friend int compare(const string<T> &a,const string<T> &b){ return a.compareTo(b); }


	///////////////////////////////////////////////////////////////////////////
	/// search functions
	///////////////////////////////////////////////////////////////////////////
	bool			findnext(const string<T>& pattern, size_t &startpos, bool ignorecase=false) const
	{
		return this->readaccess().findnext(pattern,startpos,ignorecase);
	}
	vector<size_t>	findall (const string<T>& pattern, bool ignorecase=false) const
	{
		return this->readaccess().findall(pattern,ignorecase);
	}
	bool			findnext(const T* pattern, size_t &startpos, bool ignorecase=false) const
	{
		conststring<T> tmp(pattern);
		return this->readaccess().findnext(tmp,startpos,ignorecase);
	}
	vector<size_t>	findall (const T* pattern, bool ignorecase=false) const
	{
		conststring<T> tmp(pattern);
		return this->readaccess().findall(tmp,ignorecase);
	}

	size_t find_first_of(const T& ch) const	{ return this->readaccess().find_first_of( ch ); }
	size_t find_last_of(const T& ch) const	{ return this->readaccess().find_last_of( ch ); }

	inline vector< string<T> > split(const T& splitter) const
	{
		vector< string<T> > ret;
		const T *ip, *cp;
		ip = cp = this->c_str();
		if(ip)
		{
			while(*ip)
			{
				if( *ip == splitter)
				{
					ret.push( string<T>(cp, ip-cp) );
					cp = ip = ip+1;
				}
				else
					ip++;				
			}
			if(ip!=cp)
				ret.push( string<T>(cp, ip-cp) );
		}		
		return ret;
	}
	inline vector< string<T> > split(const string<T>& splitter) const
	{
		vector< string<T> > ret;
		const T *ip, *cp;
		ip = cp = this->c_str();
		if(ip)
		{
			while(*ip)
			{
				if( splitter.find_first_of(*ip) != string<>::npos )
				{
					ret.push( string<T>(cp, ip-cp) );
					cp = ip = ip+1;
				}
				else
					ip++;				
			}
			if(ip!=cp)
				ret.push( string<T>(cp, ip-cp) );
		}		
		return ret;
	}


	/////////////////////////////////////////////////////////////////
	/// change case
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
	/// remove whitespaces from left side
	virtual void ltrim()
	{	// a trick to get a non-const pointer without making
		// the string unique
		const T* p=this->c_str(), *q = p;
		if(p)
		{
			while( *p && stringcheck::isspace(*p) )
				p++;
			if(p!=q)
				writeaccess().clear(0, p-q);
		}
	}
	/// remove whitespaces from right side
	virtual void rtrim()
	{	// a trick to get a non-const pointer without making
		// the string unique
		const T* e=this->c_str(), *p = e+this->length()-1, *q=p;
		if(e && p>=e)
		{
			while( p>=e && stringcheck::isspace(*p) )
				p--;
			if(p!=q)
				writeaccess().truncate(1+p-e);
		}
	}
	/// remove whitespaces from both sides
	virtual void trim()
	{
		rtrim();
		ltrim();
	}
	/// remove whitespaces from both sides, combine or remove speces inside
	virtual void itrim(bool removeall=false)
	{
		this->writeaccess().itrim(removeall);
	}
	size_t hash() const
	{
		return this->readaccess().hash();
	}
};


///////////////////////////////////////////////////////////////////////////////
/// globalstring.
/// practically same as string
/// but the smart pointer behaviour does not do copy-on-write
/// can be used to globally change all strings connected to the same buffer
/// use with care in multithread environment
///////////////////////////////////////////////////////////////////////////////
template<typename T=char>
class globalstring : public string<T>
{
protected:
	/////////////////////////////////////////////////////////////////
	/// virtual access functions to the smart pointer
	/// autocount behaviour
	/////////////////////////////////////////////////////////////////
	virtual const basestring<T>& readaccess() const
	{
		this->checkpointer();	
		// no need to aquire, is done on reference creation
		return *this->cCountObj;
	}
	virtual       basestring<T>& writeaccess(bool keep=true)
	{
		this->checkpointer();
		// no need to aquire, is done on reference creation
		return *this->cCountObj;
	}

public:
	/////////////////////////////////////////////////////////////////
	/// construction/destruction
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
	/// type construction
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
	/// assignment operators
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
/// substring.
/// result of string::operator()
/// can only be created from inside a string and not stored, 
/// so it is safe to just contain and work on a pointer to the calling string
///////////////////////////////////////////////////////////////////////////////
template <typename T=char>
class substring : public stringinterface<T>
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
	virtual size_t capacity() const		{ return (cString) ? this->cString->capacity() : 0; }
	virtual const T* begin() const		{ return const_cast<T*>( this->c_str() ); }
	virtual const T* end() const		{ return const_cast<T*>( this->c_str()+cLen ); }
	virtual const T* final() const		{ return const_cast<T*>( this->c_str()+cLen ); }
	virtual       T* begin()			{ return const_cast<T*>( this->c_str() ); }
	virtual       T* end()				{ return const_cast<T*>( this->c_str()+cLen ); }
	virtual       T* final()			{ return const_cast<T*>( this->c_str()+cLen ); }


	/////////////////////////////////////////////////////////////////
	/// Different Assignments to a substring
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
		string<T> a(*this);
		return  (a << s);
	}
	string<T> operator +(const stringinterface<T> &t)
	{
		string<T> a(*this);
		return  (a << t);

	}
	string<T> operator +(const substring<T> &sub)
	{
		string<T> a(*this);
		return  (a << sub);
	}
	string<T> operator +(const char* c)
	{
		string<T> a(*this);
		return  (a << c);
	}
	string<T> operator +(const char ch)
	{
		string<T> a(*this);
		return  (a << ch);
	}


	///////////////////////////////////////////////////////////////////////////
	/// readable array access
	///////////////////////////////////////////////////////////////////////////
	virtual const T operator[](size_t inx) const	{ return (this->cString)?this->cString->operator[](cLen+inx):0; }
	virtual const T operator[](int inx) const		{ return (this->cString)?this->cString->operator[](cLen+inx):0; }


	/////////////////////////////////////////////////////////////////
	/// compares
	/////////////////////////////////////////////////////////////////
	virtual int compareTo(const substring<T> &sub) const
	{
		if(this->cLen!=sub.cLen)
			return this->cLen-sub.cLen;
		int ret = memcmp(this->c_str(), sub.c_str(), cLen*sizeof(T));
		return ret;
	}
	virtual int compareTo(const string<T> &s) const
	{	// cannot compare up to the terminator because Substrings do not have one
		int ret = memcmp(this->c_str(), s.c_str(), this->cLen*sizeof(T));
		if(ret == 0 ) return (0 - s.c_str()[cLen]);
		return ret;
	}
	virtual int compareTo(const stringinterface<T>& s) const
	{	// cannot compare up to the terminator because Substrings do not have one
		int ret = memcmp(this->c_str(), s.c_str(), cLen*sizeof(T));
		if(ret == 0 ) return (0-s[cLen]);
		return ret;
	}
	virtual int compareTo(const T* c) const
	{	// cannot compare up to the terminator because Substrings do not have one
		int ret = memcmp(this->c_str(), c, cLen*sizeof(T));
		if(ret == 0 ) return (0-c[cLen]);
		return ret;
	}
	virtual int compareTo(const T ch) const
	{
		if(cLen==1)
			return this->c_str()[0] - ch;
		else
			return this->c_str()[1];
	}
	/////////////////////////////////////////////////////////////////
	/// change case
	/////////////////////////////////////////////////////////////////
	virtual bool operator ==(const substring<T> &s)	const { return (compareTo( s ) == 0); }
	virtual bool operator !=(const substring<T> &s)	const { return (compareTo( s ) != 0); }
	virtual bool operator <=(const substring<T> &s)	const { return (compareTo( s ) <= 0); }
	virtual bool operator >=(const substring<T> &s)	const { return (compareTo( s ) >= 0); }
	virtual bool operator < (const substring<T> &s)	const { return (compareTo( s ) <  0); }
	virtual bool operator > (const substring<T> &s)	const { return (compareTo( s ) >  0); }

	virtual bool operator ==(const string<T> &s)	const { return (compareTo( s ) == 0); }
	virtual bool operator !=(const string<T> &s)	const { return (compareTo( s ) != 0); }
	virtual bool operator <=(const string<T> &s)	const { return (compareTo( s ) <= 0); }
	virtual bool operator >=(const string<T> &s)	const { return (compareTo( s ) >= 0); }
	virtual bool operator < (const string<T> &s)	const { return (compareTo( s ) <  0); }
	virtual bool operator > (const string<T> &s)	const { return (compareTo( s ) >  0); }

	virtual bool operator ==(const stringinterface<T> &s)	const { return (this->compareTo( s ) == 0); }
	virtual bool operator !=(const stringinterface<T> &s)	const { return (this->compareTo( s ) != 0); }
	virtual bool operator <=(const stringinterface<T> &s)	const { return (this->compareTo( s ) <= 0); }
	virtual bool operator >=(const stringinterface<T> &s)	const { return (this->compareTo( s ) >= 0); }
	virtual bool operator < (const stringinterface<T> &s)	const { return (this->compareTo( s ) <  0); }
	virtual bool operator > (const stringinterface<T> &s)	const { return (this->compareTo( s ) >  0); }

	virtual bool operator ==(const T c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const T *c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const T *c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const T *c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const T *c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const T *c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const T *c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const basestring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const basestring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const basestring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const basestring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const basestring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const basestring<T>& c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const staticstring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const staticstring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const staticstring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const staticstring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const staticstring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const staticstring<T>& c) 	const { return (this->compareTo( c ) >  0); }

	virtual bool operator ==(const conststring<T>& c) 	const { return (this->compareTo( c ) == 0); }
	virtual bool operator !=(const conststring<T>& c) 	const { return (this->compareTo( c ) != 0); }
	virtual bool operator <=(const conststring<T>& c) 	const { return (this->compareTo( c ) <= 0); }
	virtual bool operator >=(const conststring<T>& c) 	const { return (this->compareTo( c ) >= 0); }
	virtual bool operator < (const conststring<T>& c) 	const { return (this->compareTo( c ) <  0); }
	virtual bool operator > (const conststring<T>& c) 	const { return (this->compareTo( c ) >  0); }


	/////////////////////////////////////////////////////////////////
	/// basic assign
	/////////////////////////////////////////////////////////////////
	const substring<T>& assign(const T* c, size_t slen)
	{
		if(cString)
		{
			this->cString->writeaccess().replace(cPos, cLen, c, slen );
		}
		return *this;
	}
	const substring<T>& assign(const T& c, size_t slen)
	{	// replece the selected string potion with one character
		if(c && cString) this->cString->writeaccess().replace(cPos, cLen, &c, 1);
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// basic append
	/////////////////////////////////////////////////////////////////
	const substring<T>& append(const T* c, size_t slen)
	{
		if(cString)
		{	// append to the substring == insert after + resize
			size_t sz = this->cString->size();
			this->cString->writeaccess().insert(cPos+cLen, c, slen);
			cLen += this->cString->size() - sz;
			//## TODO change on modified access interface
		}
		return *this;
	}
	const substring<T>& append(const T& c, size_t slen)
	{
		if(cString)
		{	// append to the substring == insert after + resize
			size_t sz = this->cString->size();
			this->cString->writeaccess().insert(cPos+cLen, c, slen);
			cLen += this->cString->size() - sz;
			//## TODO change on modified access interface
		}
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	/// basic insert
	/////////////////////////////////////////////////////////////////
	const substring<T>& insert(size_t pos, const T* c, size_t slen)
	{
		if(cString)
		{	// insert within substring == insert + resize
			if(pos>this->cLen) pos=this->cLen;
			size_t sz = this->cString->size();
			this->cString->writeaccess().insert(cPos+pos, c, slen);
			cLen += this->cString->size() - sz;
			//## TODO change on modified access interface
		}
		return *this;
	}
	const substring<T>& insert(size_t pos, const T& c, size_t slen)
	{
		if(cString)
		{	// insert within substring == insert + resize
			if(pos>this->cLen) pos=this->cLen;
			size_t sz = this->cString->size();
			this->cString->writeaccess().insert(cPos+pos, c, slen);
			cLen += this->cString->size() - sz;
			//## TODO change on modified access interface
		}
		return *this;
	}

	/////////////////////////////////////////////////////////////////
	/// change case
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
	/// trim's
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
	// remove whitespaces from both sides, combine or remove spaces inside
	virtual void itrim(bool removeall=false)
	{
		string<T> sel = *this;
		sel.itrim(removeall);
		if(*this!=sel)
			*this=sel;
	}
};



///////////////////////////////////////////////////////////////////////////////
/// dbstring.
/// class itself is not usable
/// it just contains a simple singleton with a list of all stored strings
/// so it can be used to retrieve same stringbuffers throughout the application
///////////////////////////////////////////////////////////////////////////////
template<typename T=char>
class dbstring
{
private:
	// simple Singleton
	static vector< string<T> >& getStringList()
	{
		static vector< string<T> > cStringList;
		return cStringList;
	}
	dbstring()	{}
	~dbstring()	{}
public:	
	static const string<T>& get(const string<T>& str)
	{
		size_t pos;
		vector< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast<string<T>*>(list.begin()), list.size() );
		if( !BinarySearch(str, list.begin(), list.size(), 0, pos) )
		{
			list.insert(str,1,pos);
		}
		return list[pos];
	}
	static const string<T>& get(const char* c)
	{
		size_t pos;
		vector< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast<string<T>*>(list.begin()), list.size() );
		if( !BinarySearch(c, list.begin(), list.size(), 0, pos) )
		{
			string<T> s(c);
			list.insert(s,1,pos);
		}
		return list[pos];
	}
	static const string<T>& get(const stringinterface<T>& si)
	{
		size_t pos;
		vector< string<T> > &list = getStringList();
		QuickSort< string<T> >( const_cast<string<T>*>(list.begin()), list.size() );
		if( !BinarySearch(si, list.begin(), list.size(), 0, pos) )
		{
			string<T> s(si);
			list.insert(s,1,pos);
			return list[pos];
		}
		return list[pos];
	}
};





///////////////////////////////////////////////////////////////////////////////
/// pipe operators 
///////////////////////////////////////////////////////////////////////////////
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const stringinterface<T>& t)	{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const string<T>& t)			{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const T* t)					{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const T t)					{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const int t)					{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const unsigned int t)			{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const long t)					{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const unsigned long t)		{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const int64 t)				{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const uint64 t)				{ return s.append(t); }
template<typename T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const double t)				{ return s.append(t); }
template<typename T, typename X> inline stringoperator<T>& operator <<(stringoperator<T>& s, const formatstr<T,X>& t)	{ return s.append(t); }
template<typename T, typename X> inline stringoperator<T>& operator <<(stringoperator<T>& s, const formatobj<X>& t)	{ return s.append(t); }

template<typename T> inline string<T>& operator <<(string<T>& s, const stringinterface<T>& t)		{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const string<T>& t)				{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const T* t)						{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const T t)						{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const int t)						{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const unsigned int t)				{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const long t)						{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const unsigned long t)			{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const int64 t)					{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const uint64 t)					{ s->append(t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const double t)					{ s->append(t); return s; }
template<typename T, typename X> inline string<T>& operator <<(string<T>& s, const formatstr<T,X>& t)	{ s->append(t); return s; } 
template<typename T, typename X> inline string<T>& operator <<(string<T>& s, const formatobj<X>& t)	{ s->append(t); return s; }


// have type conversions for extra windows buildin types for MSVC versions that have them seperated
#if defined(_MSC_VER) && _MSC_VER <= 1200
// take __int8 and __int16 as numbers, not as char/wchar
template<typename T> inline string<T>& operator <<(string<T>& s, const          __int8  t)			{ s->append((         int)t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const unsigned __int8  t)			{ s->append((unsigned int)t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const          __int16 t)			{ s->append((         int)t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const unsigned __int16 t)			{ s->append((unsigned int)t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const          __int32 t)			{ s->append((         long)t); return s; }
template<typename T> inline string<T>& operator <<(string<T>& s, const unsigned __int32 t)			{ s->append((unsigned long)t); return s; }
// (__int64 already handled)
// (no use for __int128 currently)
#endif



///////////////////////////////////////////////////////////////////////////////
/// add operators.
/// for left hand side operators, only implement these, 
/// the others would confuse standard operators
///////////////////////////////////////////////////////////////////////////////
template<typename T> string<T>
operator +(const T* t, const stringinterface<T>& str)
{
	string<T> a(t);
	return ( a << str );
}
template<typename T> string<T>
operator +(const T t, const stringinterface<T>& str)
{
	string<T> a(t);
	return ( a << str );
}
template<typename T> string<T>
operator +(const T* t, const string<T>& str)
{
	string<T> a(t);
	return ( a << str );
}
template<typename T> string<T>
operator +(const T t, const string<T>& str)
{
	string<T> a(t);
	return ( a << str );
}

template<typename T, typename X>
string<T> operator +(const stringinterface<T>& t, const X& v)
{
	string<T> a(t);
	return ( a << v );
}
template<typename T, typename X>
string<T> operator +(const string<T>& t, const X& v)
{
	string<T> a(t);
	return ( a << v );
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
template<typename T> inline void assign(string<T>& s, const char* buf, int len)
{
	s.assign(buf, len);
}
template<typename T> inline void clear(string<T>& s)
{
	s.clear();
}
template<typename T> inline bool isempty(const stringinterface<T>& s)
{
	return s.length() == 0;
}
template<typename T> inline int  pos(const string<T>& s1, const string<T>& s)
{
	return pos(s1.c_str(), s);
}

template<typename T> inline string<T> lowercase(const T* p) 
{
    return string<T>().assign_tolower(p);
}

template<typename T> inline string<T> lowercase(const string<T>& s)
{
	string<T> r = s;
	r.tolower();
	return r;
}

template<typename T> inline string<T> uppercase(const T* p) 
{
    return string<T>().assign_toupper(p);
}

template<typename T> inline string<T> uppercase(const string<T>& s)
{
	string<T> r = s;
	r.toupper();
	return r;
}


template<typename T> inline string<T> dup(const string<T>& s)
{    // dup() only reads the data pointer so it is thread-safe
    return string<T>(s);
}

template<typename T> inline bool contains(const T* s1, size_t s1len, const string<T>& s, size_t at)
{
	return (s1len >= 0) && (at >= 0) && (at+s1len <= hstrlen(s))
        && (s1len == 0 || memcmp(s.c_str()+at, s1, s1len*sizeof(T)) == 0);
}

template<typename T> inline bool contains(const T* s1, const string<T>& s, size_t at)
{
    return contains<T>(s1, hstrlen(s1), s, at);
}

template<typename T> inline bool contains(const T s1, const string<T>& s, size_t at)
{
    return (at<hstrlen(s)) && (s.c_str()[at] == s1);
}

template<typename T> inline bool contains(const string<T>& s1, const string<T>& s, size_t at)
{
    return contains<T>(s1.c_str(), hstrlen(s1), s, at);
}

template<typename T> inline string<T> copy(const string<T>& s, size_t from, size_t cnt)
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

template<typename T> inline string<T> copy(const string<T>& s, size_t from)
{
	return copy(s, from, INT_MAX);
}

template<typename T> inline void ins(const T* s1, size_t s1len, string<T>& s, size_t at)
{
	s.insert(at, s1, s1len);
}

template<typename T> inline void ins(const T* sc, string<T>& s, size_t at)
{
	s.insert(at, sc);
}

template<typename T> inline void ins(const T c, string<T>& s, size_t at)
{
	s.insert(at, c);
}

template<typename T> inline void ins(const string<T>& s1, string<T>& s, size_t at)
{
	s.insert(at, s);
}

template<typename T> inline void del(string<T>& s, size_t from, size_t cnt)
{
	s.clear(from, cnt);
}

template<typename T> inline void del(string<T>& s, size_t from)
{
	s.clear(from, INT_MAX);
}

template<typename T> inline int pos(const T* sc, const string<T>& s)
{
    const T* t = sstrstr(s.c_str(), sc);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<typename T> inline int pos(T c, const string<T>& s)
{
    const T* t = strchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<typename T> inline int rpos(T c, const string<T>& s)
{
    const T* t = strrchr(s.c_str(), c);
    return (t == NULL ? (-1) : (t - s.c_str()));
}

template<typename T> inline string<T>& fill(string<T>& str, size_t width, T ch)
{
    if( width > 0 )
        str.assign(ch, width);
	else
		str.clear();
    return str;
}

template<typename T> inline string<T>& pad(string<T>& s, size_t width, T ch, bool left)
{
	size_t len = hstrlen(s);
	if( width > 0 && len < width )
		s.insert( (left)?0:len, ch, width-len);
	return s;
}

template<typename T> inline vector< string<T> > split(const string<T>& s, const T& splitter)
{
	return s.split(splitter);
}
template<typename T> inline vector< string<T> > split(const string<T>& s, const T* splitter)
{
	return s.split(splitter);
}
template<typename T> inline vector< string<T> > split(const string<T>& s, const string<T>& splitter)
{
	return s.split(splitter);
}





///////////////////////////////////////////////////////////////////////////////
/// default number to string conversions.
/// using string default type
///////////////////////////////////////////////////////////////////////////////
string<> itostring(sint64 value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> itostring(uint64 value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> itostring(long value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> itostring(ulong value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> itostring(int value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> itostring(uint value, size_t base, size_t width=0, bool left=true, char padchar=0, char pluschar=0);
string<> ftostring(double value, int prec, char format='g', size_t width=0, bool left=true, char padchar=0, char pluschar=0, bool alt=false);


inline string<> tostring(sint64 v)	{ return itostring(sint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(uint64 v)	{ return itostring(uint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(long v)	{ return itostring(sint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(ulong v)	{ return itostring(uint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(int v)		{ return itostring(sint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(uint v)	{ return itostring(uint64(v), 10, 0, true, ' ', '\0'); }
inline string<> tostring(double v)	{ return ftostring(v, 6, 'g', 0, true, ' ', '\0', false); }


///////////////////////////////////////////////////////////////////////////////
/// timestamp string
///////////////////////////////////////////////////////////////////////////////
template<typename T> string<T> nowstring(const T* fmt, bool utc=true);
template<typename T> string<T> dttostring(const datetime& dt, const T* fmt);
string<> longtimestring(ulong seconds);
template<typename T> stringoperator<T>& operator <<(stringoperator<T>& str, const datetime& dt);
template<typename T> string<T>& operator <<(string<T>& str, const datetime& dt)	{ *str << dt; return str; }

////////////////////////////////////////////////////////////////////////////////
// converts given number to string with k/M/G.. units with base 1024
string<> bytestostring(long bytes);


extern string<> nullstring;













///////////////////////////////////////////////////////////////////////////////
/// object for sprintf compatible formating.
/// though supports only one printing element per object
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename X>
class formatstr
{
	const T*			cfmt;
	mutable const X&	cval;
	bool				cconst;
public:
	formatstr(const T* f, const X& v) : cfmt(f), cval(v), cconst(true)	{}
	formatstr(const T* f, X& v) : cfmt(f), cval(v), cconst(false)		{}
	~formatstr() {}

	const T* fmt() const		{ return cfmt; }
	const X& val() const		{ return cval; }
	//operator const X&() const	{ return cval; }
	//operator X&()				{ return const_cast<X&>(cval); }

	operator string<T>() const	{ string<T> str; str<<*this; return str;}

	const formatstr<T,X>& operator=(const X& v) const
	{
		if(!cconst)
			const_cast<X&>(cval) = v;
		return *this;
	}
};


///////////////////////////////////////////////////////////////////////////////
/// object for formating.
///////////////////////////////////////////////////////////////////////////////
template <typename X>
class formatobj
{
	mutable const X&	cval;
	bool				cconst;

	int			cWidth;		//	width
	int			cPrec;		//	precision ( digits after '.' on floating points)
	char		cType;		//	'i','d','u','x','X','o','f','g','G','e','E','s'
	char		cPad;		//	pad char ('\0'/' '/'+')
	char		cPlus;		//	plus char ('\0'/' '/'+')
	uint		cLeft : 1;	//	left aligned
	uint		cAlt : 1;	//	alternate format (used on 'x','X','o','f','g','G','e','E')
	uint		_dummy : 6;

public:
	formatobj(const X& v, 
		char type, int prec=0, int width=0, bool left=true, char pad='\0', char plus='\0', bool alt=false)
		: cval(v), cconst(true),
		cWidth(width), cPrec(prec), cType(type), cPad(pad), cPlus(plus), cLeft(left), cAlt(alt)
	{
		check();
	}
/*	formatobj(X& v, 
		char type, int prec=0, int width=0, bool left=true, char pad='\0', char plus='\0', bool alt=false)
		: cval(v), cconst(false),
		cWidth(width), cPrec(prec), cType(type), cPad(pad), cPlus(plus), cLeft(left), cAlt(alt)
	{
		check();
	}
*/	~formatobj() {}

	const X& val() const		{ return cval; }
	//operator const X&() const	{ return cval; }
	//operator X&()				{ return const_cast<X&>(cval); }

	template<typename T> operator string<T>() const	{ string<T> str; this->print(str); return str;}

	const formatobj<X>& operator=(const X& v) const
	{
		if(!cconst)
			const_cast<X&>(cval) = v;
		return *this;
	}
	
	void check()
	{
		this->checktype();
		if(cWidth<0)
		{
			cWidth=-cWidth;
			cLeft=true;
		}
		if(cPrec<0)
		{
			cPrec=-cPrec;
			cLeft=true;
		}
	}
	void checktype();
	ssize_t print(stringoperator<char>& str) const;
	ssize_t print(stringoperator<wchar_t>& str) const;
};


///////////////////////////////////////////////////////////////////////////////
/// format object.
/// type depending inplementations
///////////////////////////////////////////////////////////////////////////////

template <typename X>
inline formatstr<char   ,X> format(const char   *f, const X& v)
{
	return formatstr<char   ,X>(f,v);
}
template <typename X>
inline formatstr<wchar_t,X> format(const wchar_t*f, const X& v)
{
	return formatstr<wchar_t,X>(f,v);
}
template <typename X>
inline formatstr<char   ,X> format(const char   *f, X& v)
{
	return formatstr<char   ,X>(f,v);
}
template <typename X>
inline formatstr<wchar_t,X> format(const wchar_t*f, X& v)
{
	return formatstr<wchar_t,X>(f,v);
}


template <typename X>
inline formatobj<X> format(const X& v, int prec, int width, char type='a', bool left=true, char pad='\0', char plus='\0', bool alt=false)
{
	return formatobj<X>(v, type, prec, width, left, pad, plus, alt);
}


NAMESPACE_END(basics)


#endif//__BASESTRING_H__





/*
#include <locale.h>
char *setlocale(int category, const char *locale);
LC_CTYPE

  mbstowcs(), mbsrtowcs()


  mbtowc(), wctomb()
function can be used safely in a  multithreaded application, 
as long as setlocale(3C) is not being called to change the locale

  mbrtowc/wcrtomb



///////////////////////////////////////////////////////////////////////////////
/// Convert utf8-string to ucs1-string.
///////////////////////////////////////////////////////////////////////////////
char* utf2ucs(const char *utf8, char *ucs, size_t n)
{
	const char *pin;
	char *pout;
	unsigned char current, next;
	int i;
	for(i=0, pin=utf8, pout=ucs; i<n && *pin; ++i,++pin,++pout)
	{
		current = *pin;
		if (current >= 0xE0)
		{	// support only two-byte utf8
			return NULL;
		}
		else if( (current & 0x80) == 0 )
		{	// one-byte utf8
			*pout = current;
		}
		else
		{ // two-byte utf8
			next = *(++pin);
			if(next >= 0xC0)
			{	// illegal coding
				return NULL;
			}
			*pout = ((current & 3) << 6) +	// first two bits of first byte
					(next & 63);			// last six bits of second byte
		}
	}
	if (i < n)
		*pout = '\0';
	return ucs;
///////////////////////////////////////////////////////////////////////////////
/// Convert ucs1-string to utf8-string.
///////////////////////////////////////////////////////////////////////////////
char* ucs2utf(const char *ucs, char *utf8, size_t n)
{
	const char *pin;
	char *pout;
	unsigned char current;
	int i;
	for(i=0,pin=ucs,pout=utf8; i < n && *pin; ++i,++pin,++pout)
	{
		current = *pin;
		if (current < 0x80)
		{	// one-byte utf8
			*pout = current;
		}
		else
		{	// two-byte utf8
			*pout = 0xC0 + (current>>6);	// first two bits
			pout++, i++;					// examine second byte
			if (i>=n)
			{	// cannot convert last byte
				*(--pout) = '\0';
				return utf8;
			}
			*pout = 0x80 + (current & 63);	// last six bits
		}
	}
	if (i < n)
		*pout = '\0';
	return utf8;
}

/// direct mapping from 31 bit UCS-4 to UTF-8.
size_t ucs4toutf8( unsigned long value, unsigned char *buf )
{
	if( value <=      0x0000007F )
	{
		buf[0] = (unsigned char)value;
		return 1;
	}
	else if( value <= 0x000007FF )
	{
		buf[1] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[0] = (unsigned char)(value & 0x1F | 0xC0);
		return 2;
	}
	else if( value <= 0x0000FFFF )
	{
		buf[2] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[1] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[0] = (unsigned char)(value & 0x0F | 0xE0);
		return 3;
	}
	else if( value <= 0x001FFFFF )
	{
		buf[3] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[2] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[1] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[0] = (unsigned char)(value & 0x07 | 0xF0);
		return 4;
	}
	else if( value <= 0x03FFFFFF )
	{
		buf[4] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[3] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[2] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[1] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[0] = (unsigned char)(value & 0x03 | 0xF8);
		return 5;
	}
	else if( value <= 0x7FFFFFFF )
	{
		buf[5] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[4] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[3] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[2] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[1] = (unsigned char)(value & 0x3F | 0x80);
		value>>=6;
		buf[0] = (unsigned char)(value & 0x01 | 0xFC);
		return 6;
	}
	return 0;
}

*/


