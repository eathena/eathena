#ifndef __BASELIB__
#define __BASELIB__

//////////////////////////////////////////////////////////////////////////
/// module for loading dynamic libraries.
/// defines a wrapper for unifying unix/windows style of lib loading
//////////////////////////////////////////////////////////////////////////

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"


//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//////////////////////////////////////////////////////////////////////////

// using windows builtins for
//HINSTANCE LoadLibrary(const char* lpLibFileName)
//bool FreeLibrary(HINSTANCE hLibModule)
//FARPROC* GetProcAddress(HINSTANCE hModule, const char* lpProcName)

extern inline FARPROC GetProcFunction(HINSTANCE hModule, const char* lpProcName)
{	// ignore the conversion issue here as M$ is not standard conform anyway
	return GetProcAddress(hModule, lpProcName);
}

extern inline void* GetProcAddress(FARPROC functionpointer)
{	// ignore the conversion issue here as M$ is not standard conform anyway
	return (void*)functionpointer;
}

extern inline FARPROC GetProcFunction(void* pointer)
{	// ignore the conversion issue here as M$ is not standard conform anyway
	return (FARPROC)pointer;
}

//////////////////////////////////////////////////////////////////////////
#else//not WIN32
//////////////////////////////////////////////////////////////////////////

#  include <dlfcn.h>
/// implement a windows like loading environment
typedef void* HINSTANCE;
typedef int (*FARPROC)(void);

extern inline HINSTANCE LoadLibrary(const char* lpLibFileName)
{
	return dlopen(lpLibFileName,RTLD_LAZY);
}

extern inline bool FreeLibrary(HINSTANCE hLibModule)
{
	return 0==dlclose(hLibModule);
}

extern inline void* GetProcAddress(FARPROC functionpointer)
{	// dealing with C++ Standard Core Language Active Issue #195
	// breaking the language rules could be done with:
	// cast via integral, via union or memcpy
	// use the union here
	union
	{
		void *	vptr;
		FARPROC fptr;
	} conv;
	conv.fptr = functionpointer;
	return conv.vptr;
}

extern inline FARPROC GetProcFunction(void* pointer)
{
	union
	{
		void *	vptr;
		FARPROC fptr;
	} conv;
	conv.vptr = pointer;
	return conv.fptr;
}

extern inline void* GetProcAddress(HINSTANCE hModule, const char* lpProcName)
{
	return dlsym(hModule, lpProcName);
}

extern inline FARPROC GetProcFunction(HINSTANCE hModule, const char* lpProcName)
{
	return GetProcFunction( GetProcAddress(hModule, lpProcName) );
}


//////////////////////////////////////////////////////////////////////////
#endif//not WIN32
//////////////////////////////////////////////////////////////////////////


NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
/// basic interface for dynamic library loading class.
/// derive an implementation from here
//////////////////////////////////////////////////////////////////////////

class CLibraryLoader : public noncopyable, public global
{
protected:
	CLibraryLoader()			{}
	virtual ~CLibraryLoader()	{}
public:
	virtual bool FreeLib()	= 0;
	virtual bool LoadLib(const char* name)	= 0;
	virtual bool isOk()	= 0;
};


NAMESPACE_END(basics)


#endif//__BASELIB__
