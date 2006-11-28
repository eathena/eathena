#ifndef __BASEMINISTRING_H__
#define __BASEMINISTRING_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"

NAMESPACE_BEGIN(basics)

/////////////////////////////////////////////////////////////////////////////
/// MiniString.
/// basically a downsized version of string<char>
/// mainly for testing purpose
/////////////////////////////////////////////////////////////////////////////
class MiniString : public global
{
	//TPtrAutoRef< TArrayDST<char> > cStrPtr;
	//TPtrAutoRef< vector<char> > cStrPtr;
	TObjPtr< vector<char> > cStrPtr;

	void copy(const char *c, size_t len=~0)
	{	
		size_t sz = (len&&c)?min(len,strlen(c)):(0);
		if( sz<1 )
		{
			clear();
		}
		else
		{
			cStrPtr->resize(0);
			cStrPtr->copy(c,sz,0);
		}
		cStrPtr->append(0);
	}
protected:
	int compareTo(const MiniString &s) const 
	{	// compare with memcmp including the End-of-String
		// which is faster than doing a strcmp
		if( cStrPtr != s.cStrPtr )
		{
			if(s.cStrPtr->size()>1 && cStrPtr->size()>1) 
				return memcmp(s, cStrPtr->begin(), cStrPtr->size());

			if(s.cStrPtr->size()==0 && cStrPtr->size()==0) return 0;
			if(s.cStrPtr->size()==0) return -1;
			return 1;
		}
		return 0;
	}
	int compareTo(const char *c) const 
	{	// compare with memcmp including the end-of-string
		// which is faster than doing a strcmp
		if(c && cStrPtr.exists()) return memcmp(c, cStrPtr->begin(), cStrPtr->size());
		if((!c || *c==0) && !cStrPtr.exists()) return 0;
		if((!c || *c==0)) return -1;
		return 1;
	}
public:
	MiniString()							{  }

	/////////////////////////////////////////////////////////////////
	// copy/assignment
	MiniString(const MiniString &str)		{ cStrPtr = str.cStrPtr; }
	const MiniString &operator=(const MiniString &str)
	{
		cStrPtr = str.cStrPtr;
		return *this; 
	}

	/////////////////////////////////////////////////////////////////
	/// a special constructor for creating an addition objects
	MiniString(const char *c1, const size_t len1, const char *c2, const size_t len2)
	{	// double initialisation to concatenate two strings within the constructor
		// the given len values are only the number of characters without the EOS
		cStrPtr->realloc(len1+len2+1);
		cStrPtr->copy(c1,len1, 0);
		cStrPtr->copy(c2,len2, cStrPtr->size());
		cStrPtr->append(0);
	}

	//////////////////////////////////////////////////////
	// type to string conversions
	MiniString(const char *c)					{ assign(c); }
	MiniString(const char *c, size_t len)		{ assign(c, len); }
	const MiniString& operator=(const char *c)	{ assign(c); return *this; }

	explicit MiniString(const char v)			{ assign(v); }
	const MiniString &operator=(const char v)	{ assign(v); return *this;}

	explicit MiniString(int v)					{ assign(v); }
	const MiniString &operator=(int v)			{ assign(v); return *this;}

	explicit MiniString(unsigned int v)			{ assign(v); }
	const MiniString &operator=(unsigned int v)	{ assign(v); return *this;}

	explicit MiniString(long v)					{ assign(v); }
	const MiniString &operator=(long v)			{ assign(v); return *this;}

	explicit MiniString(unsigned long v)		{ assign(v); }
	const MiniString &operator=(unsigned long v){ assign(v); return *this;}

	explicit MiniString(double v)				{ assign(v); }
	const MiniString &operator=(double v)		{ assign(v); return *this;}

	explicit MiniString(bool v)					{ assign(v); }
	const MiniString &operator=(bool v)			{ assign(v); return *this;}

	//////////////////////////////////////////////////////
	// destruct
	virtual ~MiniString()				{  }

	//////////////////////////////////////////////////////
	// 
	const char* get() const						{ return cStrPtr->begin(); }
	const char* c_str() const					{ return cStrPtr->begin(); }
	operator const char*() const				{ return cStrPtr->begin(); }
	size_t length() const	{ return (cStrPtr.exists() && cStrPtr->size()>0) ? ( cStrPtr->size()-1):0; }

	void clear()
	{
		if(cStrPtr.exists())
		{
			cStrPtr->resize(0);
			cStrPtr->append(0);
		}
	}

	bool resize(size_t sz)
	{
		if(cStrPtr.exists())
			cStrPtr->strip(1); //strip EOS
		if(cStrPtr->size() > sz)
		{
			cStrPtr->resize(sz);
		}
		else
		{
			while(cStrPtr->size() < sz)
				cStrPtr->append(' '); // append spaces
		}
		cStrPtr->append(0);
		return true;
	}

	//////////////////////////////////////////////////////
	// string operations
	MiniString& assign(const MiniString& str)
	{	
		cStrPtr = str.cStrPtr;
		return *this;
	}
	MiniString& assign(const char *c, size_t sz)
	{
		copy(c, sz);
		return *this;
	}
	MiniString& assign(const char* c)
	{	
		copy(c);
		return *this;
	}
	MiniString& assign(char c)
	{
		if(cStrPtr.exists())
			cStrPtr->strip(1); //strip EOS
		cStrPtr->append(c);
		cStrPtr->append(0);
		return *this;
	}
	MiniString& assign(int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%i", v);
		return assign(buf, sz);
	}
	MiniString& assign(unsigned int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%u", v);
		return assign(buf, sz);
	}
 	MiniString& assign(long v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf),"%li",v);
		return assign(buf,sz);
	}
 	MiniString& assign(unsigned long int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf),"%lu",v);
		return assign(buf,sz);
	}
	MiniString& assign(double v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%.3lf", v);
		return assign(buf, sz);
	}
	MiniString& assign(bool b)
	{
		return assign((b)?"true":"false", (b)?4:5);
	}

	//////////////////////////////////////////////////////
	MiniString& append(const MiniString& str)
	{	// append two strings
		if(str.cStrPtr.exists())
		{
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append( *str.cStrPtr );
		}
		return *this;
	}
	MiniString& append(const char *c, size_t len)
	{
		if(c)
		{
			size_t sz = (len) ? min(len,strlen(c)) : (0);
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append(c,sz);
			cStrPtr->append(0);
		}
		return *this;
	}
	MiniString& append(const char* c)
	{	// append two strings
		if(c)
		{
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append(c,strlen(c)+1);
		}
		return *this;
	}
	MiniString& append(char c)
	{
		if(cStrPtr.exists())
			cStrPtr->strip(1); //strip EOS
		cStrPtr->append(c);
		cStrPtr->append(0);
		return *this;
	}
	MiniString& append(int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%i", v);
		return append(buf, sz);
	}
	MiniString& append(unsigned int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%u", v);
		return append(buf, sz);
	}
	MiniString& append(long v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%li", v);
		return append(buf, sz);
	}
	MiniString& append(unsigned long v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%lu", v);
		return append(buf, sz);
	}
	MiniString& append(double v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%.3lf", v);
		return append(buf, sz);
	}
	MiniString& append(bool b)
	{
		return append((b)?"true":"false", (b)?4:5);
	}
	//////////////////////////////////////////////////////
	template<class X> const MiniString& operator+=(const X& x)
	{	
		return this->append(x);
	}

	//////////////////////////////////////////////////////
	template<class X> MiniString& operator<<(const X& x)
	{	
		return this->append(x);
	}

	//////////////////////////////////////////////////////
	MiniString operator +(const MiniString &s)
	{
		return MiniString(cStrPtr->begin(),length(), s.cStrPtr->begin(), s.length());
	}
	MiniString operator +(const char* c)
	{
		if(c)
		{
			return MiniString(cStrPtr->begin(),length(), c, strlen(c));
		}
		return *this;
	}
	MiniString operator +(const char ch)
	{
		if(ch)
			return MiniString(cStrPtr->begin(),length(), &ch, 1);
		return *this;
	}
	MiniString operator +(int v)
	{
		MiniString s(*this);
		return s.append(v);
	}
	MiniString operator +(unsigned int v)
	{
		MiniString s(*this);
		return s.append(v);
	}
	MiniString operator +(long v)
	{
		MiniString s(*this);
		return s.append(v);
	}
	MiniString operator +(unsigned long v)
	{
		MiniString s(*this);
		return s.append(v);
	}
	MiniString operator +(double v)
	{
		MiniString s(*this);
		return s.append(v);
	}
	MiniString operator +(bool v)
	{
		MiniString s(*this);
		return s.append(v);
	}

	//////////////////////////////////////////////////////
	bool operator==(const char *b) const		{return (0==compareTo(b));}
	bool operator==(const MiniString &b) const	{return (0==compareTo(b));}
	bool operator!=(const char *b) const		{return (0!=compareTo(b));}
	bool operator!=(const MiniString &b) const	{return (0!=compareTo(b));}
	bool operator> (const char *b) const		{return (0> compareTo(b));}
	bool operator> (const MiniString &b) const	{return (0> compareTo(b));}
	bool operator< (const char *b) const		{return (0< compareTo(b));}
	bool operator< (const MiniString &b) const	{return (0< compareTo(b));}
	bool operator>=(const char *b) const		{return (0>=compareTo(b));}
	bool operator>=(const MiniString &b) const	{return (0>=compareTo(b));}
	bool operator<=(const char *b) const		{return (0<=compareTo(b));}
	bool operator<=(const MiniString &b) const	{return (0<=compareTo(b));}

	//////////////////////////////////////////////////////
	friend bool operator==(const char *a, const MiniString &b) {return (0==b.compareTo(a));}
	friend bool operator!=(const char *a, const MiniString &b) {return (0!=b.compareTo(a));}

	//////////////////////////////////////////////////////
	friend int compare(const MiniString &a,const MiniString &b){ return a.compareTo(b); }
};


template<class T> inline stringoperator<T>& operator <<(stringoperator<T>& s, const MiniString& t)	{ s.append(t.c_str(), t.length()); return s;}
template<class T> inline string<T>& operator <<(string<T>& s, const MiniString& t)					{ s->append(t.c_str(), t.length()); return s; }

NAMESPACE_END(basics)

#endif//__BASEMINISTRING_H__
