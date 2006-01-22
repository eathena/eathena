#ifndef __BASELIB__
#define __BASELIB__

//////////////////////////////////////////////////////////////////////////
// module for loading dynamic libraries
// defines a wrapper for unifying unix/windows style of lib loading
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
// implement a windows like loading environment
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



//////////////////////////////////////////////////////////////////////////
// basic interface for dynamic library loading class
// derive an implementation from here
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






//////////////////////////////////////////////////////////////////////////
// sample implementation of a dynamic loading zlib 
// with static internals, so can create it once and use it anywere
// create with path of zlib library and access the member functions
// as the C functions would be called
// need the zlib headers for compilation and a lib for linking
//////////////////////////////////////////////////////////////////////////

#include <zlib.h>

class Czlib : public CLibraryLoader
{

	//////////////////////////////////////////////////////////////////////
	// internal class that actually holds the libray
	class _CZlib : public CLibraryLoader
	{
	public:
		//////////////////////////////////////////////////////////////////
		// handle to the library
		HINSTANCE czlib;

		//////////////////////////////////////////////////////////////////
		// function pointers
		int (* cinflateInit) (z_streamp strm, const char *version, int stream_size);
		int (* cinflate) (z_streamp strm, int flush);
		int (* cinflateEnd) (z_streamp strm);

		int (* cdeflateInit) (z_streamp strm, int level, const char *version, int stream_size);
		int (* cdeflate) (z_streamp strm, int flush);
		int (* cdeflateEnd) (z_streamp strm);
	
		_CZlib() : czlib(NULL),
		  cinflateInit(NULL),cinflate(NULL),cinflateEnd(NULL),
		  cdeflateInit(NULL),cdeflate(NULL),cdeflateEnd(NULL)
		{}

		virtual ~_CZlib()
		{
			this->FreeLib();
		}

		virtual bool FreeLib();
		virtual bool LoadLib(const char* name);
		virtual bool isOk();
	};

	//////////////////////////////////////////////////////////////////////
	// we need a singleton here 
	// to enable only one instance of the library inside the application
	TPtrAuto<_CZlib> &ptr()	
	{
		static TPtrAuto<_CZlib> p;
		return p;
	}

public:
	//////////////////////////////////////////////////////////////////////
	// construction/destruction
	Czlib()						{}
	Czlib(const char* name)		{ LoadLib(name); }
	virtual ~Czlib()			{}

	//////////////////////////////////////////////////////////////////////
	// implementation of virtual base function
	virtual bool FreeLib()
	{
		return ptr()->FreeLib();
	}

	virtual bool LoadLib(const char* name)
	{
		return ptr()->LoadLib(name);
	}
	virtual bool isOk()
	{
		return ( ptr()->isOk() );
	}

	//////////////////////////////////////////////////////////////////////
	// access functions
	int decode(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
	int encode(unsigned char *dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen); 
};
//////////////////////////////////////////////////////////////////////////




#endif//__BASELIB__
