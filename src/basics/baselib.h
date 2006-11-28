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
// defines
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
// nothing necessary here
// we change the unix to equal windows style
#else

#  include <dlfcn.h>
/// implement a windows like loading environment
typedef void* HINSTANCE;

extern inline HINSTANCE LoadLibrary(const char* lpLibFileName)
{
	return dlopen(lpLibFileName,RTLD_LAZY);
}
extern inline bool FreeLibrary(HINSTANCE hLibModule)
{
	return 0==dlclose(hLibModule);
}
extern inline void *GetProcAddress(HINSTANCE hModule, const char* lpProcName)
{
	return dlsym(hModule, lpProcName);
}

#endif


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
