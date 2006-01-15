#ifndef __BASEEXCEPTIONS_H__
#define __BASEEXCEPTIONS_H__

//////////////////////////////////////////////////////////////////////////
// exceptions
//////////////////////////////////////////////////////////////////////////

#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"





class exception : public global
{
protected:
    string<> message;
public:
    exception(const char* e) : message(e)
	{ }
    exception(const string<>& e) : message(e)
	{ }

    virtual ~exception()				{ }
    virtual string<>& get_message()		{ return message; }
	operator const char *()				{ return message; }
	const char *what()					{ return message; }
};

//////////////////////////////////////////////////////////////////////////
// exception for 'out of bound array access'
//////////////////////////////////////////////////////////////////////////
class exception_bound : public exception
{
public:
	exception_bound(const char*   e) : exception(e) {}
	exception_bound(const string<>& e) : exception(e) {}
	virtual ~exception_bound()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'memory allocation failed'
//////////////////////////////////////////////////////////////////////////
class exception_memory : public exception
{
public:
	exception_memory(const char*   e) : exception(e)	{}
	exception_memory(const string<>& e) : exception(e)	{}
	virtual ~exception_memory()							{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'failed conversion'
//////////////////////////////////////////////////////////////////////////
class exception_convert: public exception
{
public:
	exception_convert(const char*   e) : exception(e)	{}
	exception_convert(const string<>& e) : exception(e)	{}
    virtual ~exception_convert()						{}
};



//////////////////////////////////////////////////////////////////////////
// variant exception class; 
// may be thrown when a variant is being typecasted to 32-bit int 
// and the value is out of range
//////////////////////////////////////////////////////////////////////////
class exception_variant: public exception
{
public:
	exception_variant(const char*   e) : exception(e)	{}
	exception_variant(const string<>& e) : exception(e)	{}
    virtual ~exception_variant()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'socket failed'
//////////////////////////////////////////////////////////////////////////
class exception_socket : public exception
{
public:
	exception_socket(const char*   e) : exception(e)	{}
	exception_socket(const string<>& e) : exception(e)	{}
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

