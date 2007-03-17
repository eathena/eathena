#ifndef __BASETYPES_H__
#define __BASETYPES_H__

//////////////////////////////////////////////////////////////////////////
/// basic include for all basics.
/// introduces types and global functions
//////////////////////////////////////////////////////////////////////////


// add to coding guideline:
//
// forbid incremental assignments with right hand expressions
// because
// incremental assignments are not handled standard conform at gcc
// things like "ip=pointer; *ip++ = (ip=pointer)?value1:value2;"
// are compiled as: "ip=pointer; temp_ip=ip; ++ip; *temp_ip=(ip=pointer)?value1:value2;"


//////////////////////////////////////////////////////////////////////////
// some global switches
//////////////////////////////////////////////////////////////////////////
#define COUNT_GLOBALS		///< enables countage of created objects
#define CHECK_BOUNDS		///< enables boundary check for arrays and lists
#define CHECK_EXCEPTIONS	///< use exceptions for "exception" handling
#define CHECK_LOCKS			///< enables check of locking/unlocking sync objects
#define SINGLETHREAD		///< builds without multithread guards
//#define MEMORY_EXCEPTIONS	///< use buildin exceptions for out-of-memory handling
//#define WITH_MEMORYMANAGER///< enables default and temp memory manager (using chunkalloc)
#define WITH_NAMESPACE		///< go with everything inside a namespace



#ifndef WITH_MYSQL
//#define WITH_MYSQL		///< builds with mysql access wrappers
#endif
//////////////////////////////////////////////////////////////////////////
// no c support anymore
//////////////////////////////////////////////////////////////////////////
#ifndef __cplusplus
#error "this is C++ source"
#endif


//////////////////////////////////////////////////////////////////////////
/// setting some defines on platforms
//////////////////////////////////////////////////////////////////////////
#if (defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) || defined(__BORLANDC__)) && !defined(WIN32)
#define WIN32
#endif

// __APPLE__ is the only predefined macro on MacOS X
#if defined(__APPLE__)
#define __DARWIN__
#endif

// 64bit OS
#if (defined(_M_IA64) || defined(_M_X64) || defined(_WIN64) || defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)) && !defined(__64BIT__)
#define __64BIT__
#endif

#if defined(_ILP64)
#error "this specific 64bit architecture is not supported"
#endif

// debug mode
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

// not building multithread on msvc with setting this to multithread
#if defined(_MSC_VER) && !defined(_MT) && !defined(SINGLETHREAD)
#pragma message ( "INFO: not building multithreaded, defining SINGLETHREAD" )
#define SINGLETHREAD
#endif


//////////////////////////////////////////////////////////////////////////
/// setting some defines for compile modes
//////////////////////////////////////////////////////////////////////////

// check global objects count and array boundaries in debug mode
#if defined(DEBUG) && !defined(COUNT_GLOBALS)
#define COUNT_GLOBALS
#endif
#if defined(DEBUG) && !defined(CHECK_BOUNDS)
#define CHECK_BOUNDS
#endif
#if defined(DEBUG) && !defined(CHECK_LOCKS)
#define CHECK_LOCKS
#endif


//////////////////////////////////////////////////////////////////////////
/// exception macros.
/// disable exceptions on unsupported compilers
#if defined(CHECK_EXCEPTIONS)

#if defined(_MSC_VER) && _MSC_VER <= 1000	// MSVC4 is too old
#undef CHECK_EXCEPTIONS
#endif

#if defined(__BORLANDC__) && !defined(WIN32)
#undef CHECK_EXCEPTIONS
#endif

#if defined(_MSC_VER) && !defined(WIN32)
#undef CHECK_EXCEPTIONS
#endif

#endif

//////////////////////////////////////////////////////////////////////////
/// set some macros for throw declarations
#if defined(CHECK_EXCEPTIONS)
#define NOTHROW()	throw()
#define THROW(a)	throw(a)
#define D_NOTHROW	throw()
#define D_THROW(a)	throw(a)
#else
#define NOTHROW()
#define THROW(a)
#define D_NOTHROW()
#define D_THROW(a)
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1020
#  undef  D_NOTHROW	// MSVC++ 4.0/4.2 does not support 
#  undef  D_THROW	// exception specification in definition
#  define D_NOTHROW
#  define D_THROW(a)
#endif


//////////////////////////////////////////////////////////////////////////
/// namespace setup

#if defined(WITH_NAMESPACE)
#define NAMESPACE_BEGIN(x)	namespace x {
#define NAMESPACE_END(x)	}
#define USING_NAMESPACE(x)	using namespace x;
#define FROM_NAMESPACE(x,y)	x::y

#else
#define NAMESPACE_BEGIN(x)
#define NAMESPACE_END(x)
#define USING_NAMESPACE(x)
#define FROM_NAMESPACE(x,y) y
#endif


//////////////////////////////////////////////////////////////////////////
/// system dependend include switches.
/// that need to be done before start including stuff
//////////////////////////////////////////////////////////////////////////

//////////////////////////////
#ifdef WIN32
//////////////////////////////
#define WIN32_LEAN_AND_MEAN		// stuff
#define _WINSOCKAPI_			// prevent inclusion of winsock.h
//#define _USE_32BIT_TIME_T		// use 32 bit time variables 
//#define __USE_W32_SOCKETS
//////////////////////////////
#endif
//////////////////////////////

//////////////////////////////
// force glibc stuff into namespaces (>=2.3 as far as I've seen)
// still testing, 
// might generally avoid fighting with glibc since forlorn
// if completely hopeless, retreat to an own namespace
// this would also solve the exception problem on windows
//////////////////////////////
//#define _GLIBCPP_USE_NAMESPACES
//////////////////////////////


//////////////////////////////
// global fdsetsize
// override all external settings
#undef FD_SETSIZE
#define FD_SETSIZE 4096
//////////////////////////////


//////////////////////////////
// set low-level fileio to 64bit mode.
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
// hack for 64bit compiling problems with explicit 64bit file-io
// necessary for: procfs.h, sys/swap.h (not used here, just in case)
//#if _FILE_OFFSET_BITS==64
//#undef _FILE_OFFSET_BITS
//#include <procfs.h>
//#define _FILE_OFFSET_BITS 64
//#else
//#include <procfs.h>
//#endif




//////////////////////////////////////////////////////////////////////////
/// standard headers
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <memory.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <assert.h>

//////////////////////////////
/// additional includes for wchar support
#include <wchar.h>
#include <wctype.h>

#if defined (_MSC_VER) && !defined (_NATIVE_WCHAR_T_DEFINED)
#define WCHAR_T_IS_USHORT
#endif



//////////////////////////////
#if defined(linux)
#include <stdint.h>     // for uintptr_t
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
// time
//////////////////////////////////////////////////////////////////////////
#include <time.h>
//////////////////////////////
// altzone fix for Sun/GCC 3.2.
#if defined(__sun__) && defined(__GNUC__) && ( __STDC__ != 0 || defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE) )
extern int cftime(char *, char *, const time_t *);
extern int ascftime(char *, const char *, const struct tm *);
extern long altzone;
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
/// threads & syncronisation
//////////////////////////////////////////////////////////////////////////

//////////////////////////////
#ifdef WIN32
//////////////////////////////
#define _WINSOCKAPI_			// prevent inclusion of winsock.h
								// since winsock2.h is necessary
#define _WIN32_WINNT 0x0400		// need for TryEnterCriticalSection
								// uncomment for use on win98 and lower
								// but remember that mutex::trylock()
								// will always fail then
#include <windows.h>
#include <process.h>
#include <conio.h>
//////////////////////////////
#else
//////////////////////////////
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#ifdef __sun__
#include <poll.h>
#endif
#ifndef __bsdi__
#include <semaphore.h>
#endif
//////////////////////////////
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
/// inet/socket/fileio
//////////////////////////////////////////////////////////////////////////

//////////////////////////////
#ifdef WIN32
//////////////////////////////
#include <winsock2.h>
#include <ws2tcpip.h>
// do not include <io.h> globally
//////////////////////////////
#else
//////////////////////////////
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
//////////////////////////////
// SunOS/Solaris has an incompatibility by defining "struct map *if_memmap"
// as last element of "struct ifnet" and thus blocking the
// use of the name "map" in all other code
// so we temporarily move the name here but leave the struct intact
#if defined(__sun__) || defined(solaris)
#define map _map
#endif
//////////////////////////////
#include <net/if.h>		// needs but does not include <sys/socket.h>
//////////////////////////////
#if defined(__sun__) || defined(solaris)
#undef map
#endif
//////////////////////////////
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <dirent.h>
#ifndef FIONREAD
#include <sys/filio.h>	// FIONREAD on Solaris, might conflict on other systems
#endif
#ifndef SIOCGIFCONF
#include <sys/sockio.h> // SIOCGIFCONF on Solaris, maybe others?
#endif
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif
//////////////////////////////
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
/// c++ header 
//////////////////////////////////////////////////////////////////////////
#include <typeinfo>		//## this is poisoning the global namespace



//////////////////////////////////////////////////////////////////////////
// no comment
//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable : 4097) // typedef-name used as based class of (...)
#pragma warning(disable : 4099)	// "type name first seen using 'struct' now seen using 'class'"
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4127)	// constant assignment
//#pragma warning(disable : 4200)	//'...'" warning, NULL field in struct
#pragma warning(disable : 4201)	// nameless struct/union
#pragma warning(disable : 4213) // type conversion of L-value issued falsly at operator bool
#pragma warning(disable : 4231) // non standard extension : 'extern' before template instanciation
#pragma warning(disable : 4244) // converting type on return will shorten
#pragma warning(disable : 4250) // dominant derive, is only informational
//#pragma warning(disable : 4251)	// disable "class '...' needs to have dll-interface to be used by clients of class '...'", since the compiler may sometimes give this warning incorrectly.
#pragma warning(disable : 4256)	// constructor for class with virtual bases has '...'; calls may not be compatible with older versions of Visual C++
#pragma warning(disable : 4267)	// disable "argument conversion possible loss of data"
#pragma warning(disable : 4275)	// disable VC6 "exported class was derived from a class that was not exported"
#pragma warning(disable : 4284) // disable VC6 for -> operator
#pragma warning(disable : 4786)	// disable VC6 "identifier string exceeded maximum allowable length and was truncated" (only affects debugger)
#pragma warning(disable : 4290)	// ... C++-specification of exception ignored
//   4305 - VC6, identifier type was converted to a smaller type
//   4309 - VC6, type conversion operation caused a constant to exceeded the space allocated for it
#pragma warning(disable : 4310)	// converting constant will shorten
#pragma warning(disable : 4355)	// "'this' : used in base member initializer list"
//   4389  '==' : signed/unsigned mismatch
//   4503 - VC6, decorated name was longer than the maximum the compiler allows (only affects debugger)
#pragma warning(disable : 4511)	// no copy constructor
#pragma warning(disable : 4512)	// no assign operator
#pragma warning(disable : 4514)	// unreferenced inline function has been removed
#if (_MSC_VER < 1200) // VC5 and earlier
#pragma warning(disable : 4521)	// multiple copy constructors/assignment operators specified,
#pragma warning(disable : 4522)	// with member templates are bogus...
#endif
#pragma warning(disable : 4571) // catch(...) blocks compiled with /EHs do not catch or re-throw structured exceptions
#pragma warning(disable : 4660) // template-class specialization '...' is already instantiated
#pragma warning(disable : 4663) // new syntax for explicit template specification (comes from Microsofts own c++ headers along with a bunch of sign warnings)
//   4675 - VC7.1, "change" in function overload resolution _might_ have altered program
#pragma warning(disable : 4702) // disable "unreachable code" warning for throw (known compiler bug)
#pragma warning(disable : 4706) // assignment within conditional
#pragma warning(disable : 4710)	// is no inline function
#pragma warning(disable : 4711)	// '..' choosen for inline extension
#pragma warning(disable : 4786)	// shortened identifier to 255 chars in browse information
#pragma warning(disable : 4800)	// disable VC6 "forcing value to bool 'true' or 'false'" (performance warning)
#pragma warning(disable : 4996)	// disable deprecated warnings

#endif


#ifdef __ICL
#pragma warning(disable :  279) // controlling expression is constant
#pragma warning(disable :  304) // access control not specified
#pragma warning(disable :  383) // value copied to temporary, reference to temporary used
#pragma warning(disable :  424) // extra ";" ignored
#pragma warning(disable :  444) // destructor for base class is not virtual
#pragma warning(disable :  869) // parameter was never referenced
#pragma warning(disable :  981) // operands are evaluated in unspecified order
#pragma warning(disable : 1418) // external function definition with no prior declaration 
#pragma warning(disable : 1572) // floating-point equality and inequality comparisons are unreliable
#pragma warning(disable : 1418) // external function definition with no prior declaration
#pragma warning(disable : 1419) // external declaration in primary source file

// possible problems
// it also seems 373 cannot be disabled
#pragma warning(disable :  373) // "copy constructor of copy forbidden classes" is inaccessible (which is actually exact what we intended)
#pragma warning(disable :  522) // function redeclared "inline" after being called

////////////////////////////////////////////////////////////////////////////////
/// Problems with ICL which is not possible to disable remark 373
/// which is spamming idiotic stuff
#define ICL_DUMMY_COPYCONSTRUCTOR(a)	a(const a&) {}
#define ICL_EMPTY_COPYCONSTRUCTOR(a)	a(const a&);
#else
#define ICL_DUMMY_COPYCONSTRUCTOR(a)
#define ICL_EMPTY_COPYCONSTRUCTOR(a)
#endif


#if defined(__BORLANDC__)
// Borland
// Shut up the following irritating warnings
#pragma warn -8022
//   8022 - A virtual function in a base class is usually overridden by a
//          declaration in a derived class.
//          In this case, a declaration with the same name but different
//          argument types makes the virtual functions inaccessible to further
//          derived classes
#pragma warn -8008
//   8008 - Condition is always true.
//          Whenever the compiler encounters a constant comparison that (due to
//          the nature of the value being compared) is always true or false, it
//          issues this warning and evaluates the condition at compile time.
#pragma warn -8060
//   8060 - Possibly incorrect assignment.
//          This warning is generated when the compiler encounters an assignment
//          operator as the main operator of a conditional expression (part of
//          an if, while, or do-while statement). This is usually a
//          typographical error for the equality operator.
#pragma warn -8066
//   8066 - Unreachable code.
//          A break, continue, goto, or return statement was not followed by a
//          label or the end of a loop or function. The compiler checks while,
//          do, and for loops with a constant test condition, and attempts to
//          recognize loops that can't fall through.

#endif


//////////////////////////////////////////////////////////////////////////
// just to be complete, nobody should use that old thing
#if defined(_MSC_VER) && _MSC_VER <= 1000
typedef int bool;
#if !defined(false)
#define false  0
#endif
#if !defined(true)
#define true   1
#endif
#endif


//////////////////////////////////////////////////////////////////////////
/// useful defines
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
// Windows compilers before VC7 don't have __FUNCTION__.
# if _MSC_VER < 1300
#  define __FUNCTION__ "<unknown>"
# endif
# define __func__ __FUNCTION__
#elif defined(__BORLANDC__)
# define __FUNCTION__ __FUNC__
# define __func__ __FUNCTION__
#elif defined(__sgi) && !defined(__GNUC__) && defined(__c99)
# define __FUNCTION__ __func__
#elif !defined(__NETBSD__)
# if __STDC_VERSION__ < 199901L
#  if __GNUC__ >= 2
#   define __func__ __FUNCTION__
#  else
#   define __FUNCTION__ "<unknown>"
#   define __PRETTY_FUNCTION__ __FUNCTION__
#   define __func__ __FUNCTION__
#  endif
# endif
#endif



/// disable attributed stuff on non-GNU
#ifndef __GNUC__
#  define  __attribute__(x)
#endif



////////////////////////////////////////////////////////////////////////////////
/// Problems with the typename keyword.
////////////////////////////////////////////////////////////////////////////////
// There are problems with using the 'typename' keyword. Technically, if you
// use a typedef member of a template class, you need to tell the compiler
// that it is a type name. This is because the compiler cannot work out
// whether a member is a type, a method or a data field at compile time.
// However, support for the typename keyword has traditionally been incomplete
// in both gcc and Visual Studio. I have used macros to try to resolve this
// issue. The macros add the keyword for compiler versions that require it and
// omit it for compiler versions that do not support it
// Typedefs:
// GCC pre-version 3 didn't handle typename in typedefs
//     after version 3, typename is required for a typedef in a template function
// Visual Studio
//     these cases are handled by the definition of the TYPEDEF_TYPENAME macro
// Function Parameters:
// Visual Studio version 7.1 requires a typename in a parameter specification in similarly obscure situations
//     this appears to be specific to VC7.1 (.NET 2003) and after and is not compatible with any gcc version
//     this case is handled by the definition of the PARAMETER_TYPENAME macro
// Template Instantiation Parameters:
// Visual studio cannot hack typename within a template instantiation parameter list
//     this is required by gcc v3.4 and optional before that
//     this case is handled by the definition of the TEMPLATE_TYPENAME macro
#if defined(__GNUC__)
 // gcc compiler variants
 #if __GNUC__ < 3
  // gcc prior to v3
  #define TYPEDEF_TYPENAME
  #define TEMPLATE_TYPENAME
  #define PARAMETER_TYPENAME
  #define RETURN_TYPENAME
 #else
  // gcc v3.0 onwards
  #define TYPEDEF_TYPENAME typename
  #define TEMPLATE_TYPENAME typename
  #define PARAMETER_TYPENAME typename
  #define RETURN_TYPENAME typename
 #endif
#elif defined(_MSC_VER)
 // Microsoft Visual Studio variants
 #define TYPEDEF_TYPENAME
 #define TEMPLATE_TYPENAME
 #if _MSC_VER < 1300
  // Visual Studio version 6 and below
  #define PARAMETER_TYPENAME
  #define RETURN_TYPENAME
 #else
  // Visual Studio .NET and above
  #define PARAMETER_TYPENAME typename
  #define RETURN_TYPENAME typename
 #endif
#else// assume C99 compilant
  #define TYPEDEF_TYPENAME typename
  #define TEMPLATE_TYPENAME typename
  #define PARAMETER_TYPENAME typename
  #define RETURN_TYPENAME typename
#endif


////////////////////////////////////////////////////////////////////////////////
/// Problems with the partial template specialisation
////////////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
 // gcc compiler variants
 #if (__GNUC_MINOR__ < 9)  && (__GNUC__ < 3) // below gcc 2.9
  #define TEMPLATE_NO_PARTIAL_SPECIALIZATION
 #endif
#elif defined(__ICL)
// intel compiler
#elif defined(_MSC_VER)
 // Microsoft Visual Studio variants
 #if (_MSC_VER <= 1300) 
  // Visual Studio .Net and below
  #define TEMPLATE_NO_PARTIAL_SPECIALIZATION
  #define TEMPLATE_NO_CV_VOID_SPECIALIZATION
 #endif
#elif defined(__BORLANDC__)
 #if (__BORLANDC__ <= 0x551)
  #define TEMPLATE_NO_CV_VOID_SPECIALIZATION
 #endif
#endif

////////////////////////////////////////////////////////////////////////////////
/// Problems with templated copy/assignment.
////////////////////////////////////////////////////////////////////////////////
// microsoft does not understand templated copy/assign together with default copy/assign
// but gnu needs them both seperated
// otherwise generates an own default copy/assignment which causes trouble then
// so could add all constructors and seperate the standard one with
// #if defined(__GNU__) or #if !defined(_MSC_VER) / #endif
// another workaround is to have a baseclass to derive the hierarchy from 
// and have templated copy/assignment refering the baseclass beside standard copy/assignment
#if defined(__GNUC__)
 // gcc compiler variants
#elif defined(_MSC_VER)
 // Microsoft Visual Studio variants
 #if _MSC_VER < 1300
  // Visual Studio version 6 and below
  #define NO_TEMPLATED_COPY_ASSIGN
 #else
  // Visual Studio .NET
 #endif
#endif


////////////////////////////////////////////////////////////////////////////////
/// Problems with member template keyword and rebind
////////////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__) && ((__GNUC__ < 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ < 4)))
 // gcc versions before 3.4.0
 #define NO_MEMBER_TEMPLATE_KEYWORD
#elif defined(_MSC_VER) && (_MSC_VER < 1300) || defined (UNDER_CE)
 // including MSVC 6.0
 #define NO_MEMBER_TEMPLATE_KEYWORD
 #define DONT_SUPPORT_REBIND
 #define HAS_BAD_TEMPLATES
#elif (defined(__SUNPRO_CC) && (__SUNPRO_CC < 0x510)) || (defined (__SUNPRO_CC_COMPAT) && (__SUNPRO_CC_COMPAT < 5))
 #define NO_MEMBER_TEMPLATE_KEYWORD
#endif

#if defined(NO_MEMBER_TEMPLATE_KEYWORD)
#  define TEMPLATE
#else
#  define TEMPLATE template
#endif


////////////////////////////////////////////////////////////////////////////////
// friend template brackets
////////////////////////////////////////////////////////////////////////////////
#if defined(__GNUG__) || defined(__MWERKS__) || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x540))
#define FRIEND_TEMPLATE <>
#else
#define FRIEND_TEMPLATE
#endif


//////////////////////////////////////////////////////////////////////////
// 
#if defined(_MSC_VER) && (_MSC_VER < 1200)
 // before VC++ 6.0
 #define TEMPLATES_QUALIFIED_SPECIALIZATION_BUG
#endif





//////////////////////////////////////////////////////////////////////////
/// useful typedefs.
//////////////////////////////////////////////////////////////////////////
#if !defined(__GLIBC__) || !defined(__USE_MISC)// glibc again
typedef unsigned int	uint;	// don't use
typedef unsigned long	ulong;	// don't use
typedef unsigned short	ushort;
#endif
typedef unsigned char	uchar;

typedef   signed int	sint;	// don't use (only for ie. scanf/file-io)
typedef   signed long	slong;	// don't use
typedef   signed short	sshort;
typedef   signed char	schar;

typedef char*           pchar;
typedef const char*     pcchar;
typedef unsigned char*	puchar;
typedef void*			ptr;
typedef int*			pint;


//////////////////////////////////////////////////////////////////////////
// pointer type
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
typedef uint         intptr;
#else
typedef uintptr_t    intptr;
#endif


//////////////////////////////////////////////////////////////////////////
// typedefs to compensate type size change from 32bit to 64bit
// MS implements LLP64 model, normal unix does LP64,
// only Silicon Graphics/Cray goes ILP64 so don't care (and don't support)
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// Integers with guaranteed _exact_ size.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////
#ifdef WIN32
//////////////////////////////
typedef          __int8		int8;
typedef signed   __int8		sint8;
typedef unsigned __int8		uint8;


typedef          __int16	int16;
typedef signed   __int16	sint16;
typedef unsigned __int16	uint16;

// seperated typedef makes problems with type indication
//typedef          __int32	int32;	
//typedef signed   __int32	sint32;
//typedef unsigned __int32	uint32;
typedef          int		int32;
typedef signed   int		sint32;
typedef unsigned int		uint32;


//////////////////////////////
#else // GNU
//////////////////////////////
typedef char				int8;
typedef short				int16;
typedef int					int32;

typedef signed char			sint8;
typedef signed short		sint16;
typedef signed int			sint32;

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
//////////////////////////////
#endif
//////////////////////////////

#ifndef INT8_MIN
#define INT8_MIN		(int8(0x80))
#endif
#ifndef INT8_MAX
#define INT8_MAX		(int8(0x7f))
#endif
#ifndef UINT8_MIN
#define UINT8_MIN		(uint8(0x00))
#endif

#ifndef UINT8_MAX
#define UINT8_MAX		(uint8(0xff))
#endif

#ifndef INT16_MIN
#define INT16_MIN		(int16(0x8000))
#endif
#ifndef INT16_MAX
#define INT16_MAX		(int16(0x7fff))
#endif
#ifndef UINT16_MIN
#define UINT16_MIN		(uint16(0x0000))
#endif
#ifndef UINT16_MAX
#define UINT16_MAX		(uint16(0xffff))
#endif

#ifndef INT32_MIN
#define INT32_MIN		(int32(0x80000000))
#endif
#ifndef INT32_MAX
#define INT32_MAX		(int32(0x7fffffff))
#endif
#ifndef UINT32_MIN
#define UINT32_MIN		(uint32(0x00000000))
#endif
#ifndef UINT32_MAX
#define UINT32_MAX		(uint32(0xffffffff))
#endif


//////////////////////////////////////////////////////////////////////////
/// Integers with guaranteed _minimum_ size.
/// These could be larger than you expect,
/// they are designed for speed.
//////////////////////////////////////////////////////////////////////////
typedef          long int      ppint;
typedef unsigned long int      ppuint;

typedef          long int      ppint8;
typedef          long int      ppint16;
typedef          long int      ppint32;

typedef unsigned long int      ppuint8;
typedef unsigned long int      ppuint16;
typedef unsigned long int      ppuint32;


//////////////////////////////////////////////////////////////////////////
/// integer with exact processor width. (and best speed)
///						size_t already defined in stdio.h
//////////////////////////////
#ifdef WIN32 // does not have a signed size_t
//////////////////////////////
#if defined(_WIN64)	// naive 64bit windows platform
typedef __int64			ssize_t;
#elif !defined(__GNUC__)
typedef int				ssize_t;
#endif
//////////////////////////////
#endif
//////////////////////////////



//////////////////////////////////////////////////////////////////////////
/// make sure there is a pointer difference type
#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
#if !defined(__PTRDIFF_TYPE__) //mingw typedefs ptrdiff_t without telling properly
typedef ssize_t ptrdiff_t;
#endif
#endif

//////////////////////////////////////////////////////////////////////////
/// portable 64-bit integers
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64				int64;
typedef signed __int64		sint64;
typedef unsigned __int64	uint64;
#define LLCONST(a)			(a##i64)
#define ULLCONST(a)			(a##ui64)
#else //elif HAVE_LONG_LONG
typedef long long			int64;
typedef signed long long	sint64;
typedef unsigned long long	uint64;
#define LLCONST(a)			(a##ll)
#define ULLCONST(a)			(a##ull)
#endif

#ifndef INT64_MIN
#define INT64_MIN  (LLCONST(-9223372036854775807)-1)
#endif
#ifndef INT64_MAX
#define INT64_MAX  (LLCONST(9223372036854775807))
#endif
#ifndef UINT64_MAX
#define UINT64_MAX (ULLCONST(18446744073709551615))
#endif


//////////////////////////////////////////////////////////////////////////
/// some redefine of function redefines for some Compilers
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) || defined(__BORLANDC__)
#define strcasecmp			stricmp
#define strncasecmp			strnicmp
#define snprintf			_snprintf
#define vsnprintf			_vsnprintf
#define vswprintf			_vsnwprintf
#endif


//////////////////////////////////////////////////////////////////////////
// glibc has become a major bitch
//////////////////////////////////////////////////////////////////////////
#if defined(__GLIBC__) && ( (__GLIBC__ >2) || (__GLIBC__ ==2 && __GLIBC_MINOR__>=3) )
// unfold hidden libc proto's
#endif


//////////////////////////////
#ifndef __cplusplus	// not cplusplus cannot happen, just to store it here
//////////////////////////////

// keyword replacement in windows
#ifdef _WIN32
#define inline __inline
#endif

// boolean types for C
typedef int bool;
#define false	(1==0)
#define true	(1==1)

#ifdef swap // just to be sure
#undef swap
#endif
// hmm only ints?
//#define swap(a,b) { int temp=a; a=b; b=temp;} 
// if using macros then something that is type independent
#define swap(a,b) ((a == b) || ((a ^= b), (b ^= a), (a ^= b)))

//////////////////////////////
#endif // not cplusplus
//////////////////////////////

//////////////////////////////////////////////////////////////////////////
// should not happen
#ifndef NULL
#define NULL (void *)0
#endif

//////////////////////////////////////////////////////////////////////////
/// number of bits in a byte
#ifndef NBBY
#define	NBBY 8
#endif



NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
/// atomic access functions.
/// FASTCALL define is necessary for _asm atomic functions on windows
#ifdef WIN32
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif
#if defined(_MSVC) || defined(__BORLANDC__)
#define CCDECL __cdecl
#else
#define CCDECL
#endif


int FASTCALL atomicincrement(int* target);
inline unsigned int FASTCALL atomicincrement(unsigned int* target)
{
	return (unsigned int)atomicincrement((int*)target);
}

int FASTCALL atomicdecrement(int* target);
inline unsigned int FASTCALL atomicdecrement(unsigned int* target)
{
	return (unsigned int)atomicdecrement((int*)target);
}

int FASTCALL atomicexchange(int* target, int value);
inline unsigned int FASTCALL atomicexchange(unsigned int* target, unsigned int value)
{
	return (unsigned int)atomicexchange((int*)target, (int)value);
}

void* FASTCALL _atomicexchange(void** target, void* value);
template
<typename T> inline T* atomicexchange(T** target, T* value)
{
	return (T*)_atomicexchange((void**)target, (void*)value); 
}

size_t FASTCALL atomiccompareexchange(size_t*target, size_t value, size_t comperand);
inline ssize_t atomiccompareexchange(ssize_t*target, ssize_t value, ssize_t comperand)
{	
	return atomiccompareexchange((size_t*)target, (size_t)value, (size_t)comperand);
}

void* FASTCALL _atomiccompareexchange(void** target, void* value, void* comperand);
template <typename T>
inline T* atomiccompareexchange(T**target, T* value, T* comperand)
{	
	return (T*)_atomiccompareexchange((void**)target, (void*)value, (void*)comperand);
}


NAMESPACE_END(basics)

#endif//__BASETYPES_H__
