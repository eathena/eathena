#ifndef __BASEZLIB_H__
#define __BASEZLIB_H__

//////////////////////////////////////////////////////////////////////////
/// module for zlib wrapper.
/// using either static link or dynamic load
//////////////////////////////////////////////////////////////////////////

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "baselib.h"

//////////////////////////////////////////////////////////////////////////
/// sample implementation of a dynamic loading zlib.
/// with static internals, so can create it once and use it anywere
/// create with path of zlib library and access the member functions
/// as the C functions would be called
/// need the zlib headers for compilation and a lib for linking 
/// when linking statically (either complete or with dll stub)
//////////////////////////////////////////////////////////////////////////

#include <zlib.h>


NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
/// test function
void test_zib(void);

//////////////////////////////////////////////////////////////////////////
// tells the code to dynamically load an external library via library name
// otherwise link zlib functions statically
//#define BASE_ZLIB_DYNAMIC

//////////////////////////////////////////////////////////////////////////
// convinience addition to select preferend link library
// searchpath to the lib has to be added manually
#ifndef BASE_ZLIB_DYNAMIC
#ifdef _MSC_VER

#pragma comment(lib, "zlib1.lib")			// library stub for zlib1.dll
//#pragma comment(lib, "zlib-asm-mt.lib")	// multithread lib
//#pragma comment(lib, "zlib-asm-md.lib")	// multithread-dll lib
//#pragma comment(lib, "zlib-asm-ml.lib")	// singlethread lib

#endif
#endif


//////////////////////////////////////////////////////////////////////////
/// the zlib wrapper.
class CZlib : public CLibraryLoader
{

#ifdef BASE_ZLIB_DYNAMIC
	//////////////////////////////////////////////////////////////////////
	/// internal class that actually holds the access to the libray
	class _CZlib : public CLibraryLoader
	{
	public:
		//////////////////////////////////////////////////////////////////
		/// handle to the library
		HINSTANCE czlib;

		//////////////////////////////////////////////////////////////////
		/// function pointers
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
	TPtrAutoCount<_CZlib> &ptr() const	
	{
		static TPtrAutoCount<_CZlib> p;
		return p;
	}
#endif

public:
	//////////////////////////////////////////////////////////////////////
	// construction/destruction
	CZlib()						{}
	CZlib(const char* name)		{ LoadLib(name); }
	virtual ~CZlib()			{}

	//////////////////////////////////////////////////////////////////////
	/// implementation of virtual base function.
	/// will be empty stubs on static link
	virtual bool FreeLib()
	{
#ifdef BASE_ZLIB_DYNAMIC
		return ( !ptr().exists() || ptr()->FreeLib() );
#else
		return true;
#endif
	}

	virtual bool LoadLib(const char* name)
	{
#ifdef BASE_ZLIB_DYNAMIC
		return ptr()->LoadLib(name);
#else
		return true;
#endif
	}
	virtual bool isOk()
	{
#ifdef BASE_ZLIB_DYNAMIC
		return ( ptr().exists() && ptr()->isOk() );
#else
		return true;
#endif
	}



	//////////////////////////////////////////////////////////////////////
	/// access functions
	int decode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen);
	int encode(unsigned char *dest, unsigned long& destLen, const unsigned char* source, unsigned long sourceLen); 

	/// deflates from file to memory.
	/// source is the zip-file, filename the file inside the zip archive
	/// calling with dest=NULL only returns the necessary size in destLen
	bool deflate(unsigned char *dest, unsigned long& destLen, const char *source, const char *filename, const char *pass=NULL);

};
//////////////////////////////////////////////////////////////////////////

NAMESPACE_END(basics)


#endif//__BASEZLIB_H__
