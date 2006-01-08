#ifndef __BASEEXCEPTIONS_H__
#define __BASEEXCEPTIONS_H__

//////////////////////////////////////////////////////////////////////////
// exceptions
//////////////////////////////////////////////////////////////////////////

#include "baseobjects.h"
#include "basestring.h"


//////////////////////////////////////////////////////////////////////////
// the basic exception class. 
// unfortunately the name "exception" is already in use
//////////////////////////////////////////////////////////////////////////
class CException : public global
{
protected:
    string<> message;
public:
    CException(const char* e) : message(e)
	{ }
    CException(const string<>& e) : message(e)
	{ }

    virtual ~CException()				{ }
    virtual string<>& get_message()		{ return message; }
	operator const char *()				{ return message; }
};


//////////////////////////////////////////////////////////////////////////
// exception for 'out of bound array access'
//////////////////////////////////////////////////////////////////////////
class exception_bound : public CException
{
public:
	exception_bound(const char*   e) : CException(e) {}
	exception_bound(const string<>& e) : CException(e) {}
	virtual ~exception_bound()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'memory allocation failed'
//////////////////////////////////////////////////////////////////////////
class exception_memory : public CException
{
public:
	exception_memory(const char*   e) : CException(e)	{}
	exception_memory(const string<>& e) : CException(e)	{}
	virtual ~exception_memory()							{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'failed conversion'
//////////////////////////////////////////////////////////////////////////
class exception_convert: public CException
{
public:
	exception_convert(const char*   e) : CException(e)	{}
	exception_convert(const string<>& e) : CException(e)	{}
    virtual ~exception_convert()						{}
};



//////////////////////////////////////////////////////////////////////////
// variant exception class; 
// may be thrown when a variant is being typecasted to 32-bit int 
// and the value is out of range
//////////////////////////////////////////////////////////////////////////
class exception_variant: public CException
{
public:
	exception_variant(const char*   e) : CException(e)	{}
	exception_variant(const string<>& e) : CException(e)	{}
    virtual ~exception_variant()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'socket failed'
//////////////////////////////////////////////////////////////////////////
class exception_socket : public CException
{
public:
	exception_socket(const char*   e) : CException(e)	{}
	exception_socket(const string<>& e) : CException(e)	{}
	virtual ~exception_socket()							{}
};





//////////////////////////////////////////////////////////////////////////
// new and delete with exceptions 
// also on compilers that do not support throwing memory
//////////////////////////////////////////////////////////////////////////
#if defined MEMORY_EXCEPTIONS
inline void *operator new (size_t sz)
{
	void *a = malloc(sz);
	if(NULL==a) throw exception_memory;
	return a;
}
inline void *operator new[] (size_t sz)
{
	void *a = malloc(sz);
	if(NULL==a) throw exception_memory;
	return a;
}

inline void operator delete (void * a)
{
	if(a) free(a);
}

inline void operator delete[] (void *a)
{
	if(a) free(a);
}
#endif



#endif//__BASEEXCEPTIONS_H__

