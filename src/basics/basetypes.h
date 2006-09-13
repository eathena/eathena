#ifndef __BASETYPES_H__
#define __BASETYPES_H__

//////////////////////////////////////////////////////////////////////////
/// basic include for all basics.
/// introduces types and global functions
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// some global switches
//////////////////////////////////////////////////////////////////////////
#define COUNT_GLOBALS		///< enables countage of created objects
#define CHECK_BOUNDS		///< enables boundary check for arrays and lists
#define CHECK_EXCEPTIONS	///< use exceptions for "exception" handling
#define CHECK_LOCKS			///< enables check of locking/unlocking sync objects
#define SINGLETHREAD		///< builds without multithread guards
//#define MEMORY_EXCEPTIONS	///< use buildin exceptions for out-of-memory handling
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
#else
#define NAMESPACE_BEGIN(x)
#define NAMESPACE_END(x)
#define USING_NAMESPACE(x)
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
/// exeption redefine.
/// after including the c++ header, class name "exception" has been
/// placed unfortunately in global namespace and in libc's 
/// at least for the VisualC on windows platforms
/// to get it working with the our own exception class we do some workaround
/// by just swapping the name with a hard define, 
/// drawback is, that std::exception is in this case also not usable
/// another possibility would be to retreat to our own namespace
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) && !defined(WITH_NAMESPACE)
#define exception CException
#endif
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// no comment
//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable : 4097) // typedef-name used as based class of (...)
#pragma warning(disable : 4099)	// "type name first seen using 'struct' now seen using 'class'"
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4127)	// constant assignment
//#pragma warning(disable : 4200)	//'...'" warning, NULL field in struct
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
 #else
  // gcc v3.0 onwards
  #define TYPEDEF_TYPENAME typename
  #define TEMPLATE_TYPENAME typename
  #define PARAMETER_TYPENAME typename
 #endif
#elif defined(_MSC_VER)
 // Microsoft Visual Studio variants
 #define TYPEDEF_TYPENAME
 #define TEMPLATE_TYPENAME
 #if _MSC_VER < 1300
  // Visual Studio version 6 abd below
  #define PARAMETER_TYPENAME
 #else
  // Visual Studio .NET and above
  #define PARAMETER_TYPENAME typename
 #endif
#endif


////////////////////////////////////////////////////////////////////////////////
/// Problems with the partial template specialisation
////////////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
 // gcc compiler variants
 #if (__GNUC_MINOR__ < 9)  && (__GNUC__ < 3) // below gcc 2.9
  #define TEMPLATE_NO_PARTIAL_SPECIALIZATION
 #endif
#elif defined(_MSC_VER)
 // Microsoft Visual Studio variants
 #if (_MSC_VER <= 1300) 
  // Visual Studio .Net and below
  #define TEMPLATE_NO_PARTIAL_SPECIALIZATION
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
/// Problems with member template keyword
////////////////////////////////////////////////////////////////////////////////
#if (__GNUC__ < 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ < 4))
 // gcc versions before 3.4.0
 #define NO_MEMBER_TEMPLATE_KEYWORD
#elif (_MSC_VER < 1300) || defined (UNDER_CE)
 // including MSVC 6.0
 #define NO_MEMBER_TEMPLATE_KEYWORD
#elif (__SUNPRO_CC < 0x510) || (defined (__SUNPRO_CC_COMPAT) && (__SUNPRO_CC_COMPAT < 5))
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
// template behaviour on microsoft visual c++ is weird
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define HAS_BAD_TEMPLATES
#endif

//////////////////////////////////////////////////////////////////////////
// 
#if defined(_MSC_VER) && _MSC_VER < 1200
 // before VC++ 6.0
 #define TEMPLATES_QUALIFIED_SPECIALIZATION_BUG
#endif


#ifndef PI
#define PI 3.14159265358979323846264338327950288L
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
#else
typedef int				ssize_t;
#endif
//////////////////////////////
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
/// make sure there is a pointer difference type
#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
typedef ssize_t ptrdiff_t;
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
/// min max and swap template.
//////////////////////////////////////////////////////////////////////////
/// The Windoze headers define macros called max/min which conflict with the templates std::max and std::min.
/// So, to avoid conflicts, MS removed the std::max/min rather than fixing the problem!
/// From Visual Studio .NET (SV7, compiler version 13.00) the STL templates have been added correctly.
/// This fix switches off the macros and reinstates the STL templates for earlier versions (SV6).
/// Note that this could break MFC applications that rely on the macros (try it and see).

// For MFC compatibility, only undef min and max in non-MFC programs - some bits of MFC
// use macro min/max in headers. For VC7 both the macros and template functions exist
// so there is no real need for the undefs but do it anyway for consistency. So, if
// using VC6 and MFC then template functions will not exist

#ifdef min // windef has macros for that, kill'em
#undef min
#endif
template <class T> inline T &min(const T &i1, const T &i2)
{	if(i1 < i2) return (T&)i1; else return (T&)i2;
}
#ifdef max // windef has macros for that, kill'em
#undef max
#endif
template <class T> inline T &max(const T &i1, const T &i2)	
{	if(i1 < i2) return (T&)i2; else return (T&)i1;
}
template <class T> inline void swap(T &i1, T &i2)
{	T dummy = i1; i1=i2; i2=dummy;
}
template <class T> inline void minmax(const T &i1, const T &i2, T &minval, T &maxval)
{
	if(i1<i2)
		minval=i1, maxval=i2;
	else
		minval=i2, maxval=i1;
}



///////////////////////////////////////////////////////////////////////////////
/// conversion overloads. to change signed types to the appropriate unsigned
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
// others, just to be complete
inline size_t to_unsigned(int t)
{
	return (unsigned int)(t);
}
inline size_t to_unsigned(unsigned int t)
{
	return (unsigned int)(t);
}
inline size_t to_unsigned(long t)
{
	return (unsigned long)(t);
}
inline size_t to_unsigned(unsigned long t)
{
	return (unsigned long)(t);
}
inline uint64 to_unsigned(int64 t)
{
	return (uint64)(t);
}
inline uint64 to_unsigned(uint64 t)
{
	return (uint64)(t);
}

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// relops.
//template<typename T> inline bool operator!=(const T& x, const T& y) { return !(x == y); }
//template<typename T> inline bool operator> (const T& x, const T& y)	{ return  (y <  x); }
//template<typename T> inline bool operator<=(const T& x, const T& y)	{ return !(y <  x); }
//template<typename T> inline bool operator>=(const T& x, const T& y)	{ return !(x <  y); }



///////////////////////////////////////////////////////////////////////////////
/// wrappers for Character Classification Routines.
/// this also gets rid of the macro definitions
//## TODO: choose a better name for the namespace 
namespace stringcheck
{
#ifdef isalpha	// get the function form, not the macro
#undef isalpha
#endif
template <class T> extern inline bool isalpha(T c) { return 0!=::isalpha( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isalpha(char c)		{ return 0!=::isalpha ( to_unsigned(c) ); }
extern inline bool isalpha(wchar_t c)	{ return 0!=::iswalpha( to_unsigned(c) ); }

#ifdef isupper	// get the function form, not the macro
#undef isupper
#endif
template <class T> extern inline bool isupper(T c) { return ::isupper( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isupper(char c)		{ return 0!=::isupper ( to_unsigned(c) ); }
extern inline bool isupper(wchar_t c)	{ return 0!=::iswupper( to_unsigned(c) ); }

#ifdef islower	// get the function form, not the macro
#undef islower
#endif
template <class T> extern inline bool islower(T c) { return 0!=::islower( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool islower(char c)		{ return 0!=::islower ( to_unsigned(c) ); }
extern inline bool islower(wchar_t c)	{ return 0!=::iswlower( to_unsigned(c) ); }

#ifdef isdigit	// get the function form, not the macro
#undef isdigit
#endif
template <class T> extern inline bool isdigit(T c) { return 0!=::isdigit( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isdigit(char c)		{ return 0!=::isdigit ( to_unsigned(c) ); }
extern inline bool isdigit(wchar_t c)	{ return 0!=::iswdigit( to_unsigned(c) ); }

#ifdef isxdigit	// get the function form, not the macro
#undef isxdigit
#endif
template <class T> extern inline bool isxdigit(T c) { return 0!=::isxdigit( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isxdigit(char c)		{ return 0!=::isxdigit ( to_unsigned(c) ); }
extern inline bool isxdigit(wchar_t c)	{ return 0!=::iswxdigit( to_unsigned(c) ); }

#ifdef isspace	// get the function form, not the macro
#undef isspace
#endif
template <class T> extern inline bool isspace(T c) { return 0!=::isspace( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isspace(char c)		{ return 0!=::isspace ( to_unsigned(c) ); }
extern inline bool isspace(wchar_t c)	{ return 0!=::iswspace( to_unsigned(c) ); }

#ifdef ispunct	// get the function form, not the macro
#undef ispunct
#endif
template <class T> extern inline bool ispunct(T c) { return 0!=::ispunct( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool ispunct(char c)		{ return 0!=::ispunct ( to_unsigned(c) ); }
extern inline bool ispunct(wchar_t c)	{ return 0!=::iswpunct( to_unsigned(c) ); }

#ifdef isalnum	// get the function form, not the macro
#undef isalnum
#endif
template <class T> extern inline bool isalnum(T c) { return 0!=::isalnum( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isalnum(char c)		{ return 0!=::isalnum ( to_unsigned(c) ); }
extern inline bool isalnum(wchar_t c)	{ return 0!=::iswalnum( to_unsigned(c) ); }

#ifdef isprint	// get the function form, not the macro
#undef isprint
#endif
template <class T> extern inline bool isprint(T c) { return 0!=::isprint( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isprint(char c)		{ return 0!=::isprint ( to_unsigned(c) ); }
extern inline bool isprint(wchar_t c)	{ return 0!=::iswprint( to_unsigned(c) ); }

#ifdef isgraph	// get the function form, not the macro
#undef isgraph
#endif
template <class T> extern inline bool isgraph(T c) { return 0!=::isgraph( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool isgraph(char c)		{ return 0!=::isgraph ( to_unsigned(c) ); }
extern inline bool isgraph(wchar_t c)	{ return 0!=::iswgraph( to_unsigned(c) ); }

#ifdef iscntrl	// get the function form, not the macro
#undef iscntrl
#endif
template <class T> extern inline bool iscntrl(T c) { return 0!=::iscntrl( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline bool iscntrl(char c)		{ return 0!=::iscntrl ( to_unsigned(c) ); }
extern inline bool iscntrl(wchar_t c)	{ return 0!=::iswcntrl( to_unsigned(c) ); }

#ifdef toupper	// get the function form, not the macro
#undef toupper
#endif
template <class T> extern inline T toupper(T c) { return ::toupper( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline char toupper(char c)		{ return ::toupper ( to_unsigned(c) ); }
extern inline wchar_t toupper(wchar_t c){ return ::towupper( to_unsigned(c) ); }

#ifdef tolower	// get the function form, not the macro
#undef tolower
#endif
template <class T> extern inline T tolower(T c) { return ::tolower( to_unsigned(c) ); }
// implementation for char (and wchar)
extern inline char tolower(char c)		{ return ::tolower ( to_unsigned(c) ); }
extern inline wchar_t tolower(wchar_t c){ return ::towlower( to_unsigned(c) ); }

};
///////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
/// atomic access functions.
/// FASTCALL define is necessary for _asm atomic functions on windows
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define FASTCALL __fastcall
#else
#define FASTCALL
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
template <class T> inline T* atomicexchange(T** target, T* value)
{
	return (T*)_atomicexchange((void**)target, (void*)value); 
}

size_t FASTCALL atomiccompareexchange(size_t*target, size_t value, size_t comperand);
inline ssize_t atomiccompareexchange(ssize_t*target, ssize_t value, ssize_t comperand)
{	
	return atomiccompareexchange((size_t*)target, (size_t)value, (size_t)comperand);
}

void* FASTCALL _atomiccompareexchange(void** target, void* value, void* comperand);
template <class T> inline T* atomiccompareexchange(T**target, T* value, T* comperand)
{	
	return (T*)_atomiccompareexchange((void**)target, (void*)value, (void*)comperand);
}

//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
//////////////////////////////////////////////////////////////////////////
extern inline uchar GetByte(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (uchar)(val      );
	case 1:
		return (uchar)(val>>0x08);
	case 2:
		return (uchar)(val>>0x10);
	case 3:
		return (uchar)(val>>0x18);
	default:
		return 0;	//better throw something here
	}
}
//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
//////////////////////////////////////////////////////////////////////////
extern inline ushort GetWord(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (ushort)(val      );
	case 1:
		return (ushort)(val>>0x10);
	default:
		return 0;	//better throw something here
	}	
}
//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
//////////////////////////////////////////////////////////////////////////
extern inline ushort MakeWord(uchar byte0, uchar byte1)
{
	return	  (((ushort)byte0)      )
			| (((ushort)byte1)<<0x08);
}
//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
//////////////////////////////////////////////////////////////////////////
extern inline uint32 MakeDWord(ushort word0, ushort word1)
{
	return 	  (((uint32)word0)      )
			| (((uint32)word1)<<0x10);
}
//////////////////////////////////////////////////////////////////////////
/// byte/word/dword access, 32bit limited
//////////////////////////////////////////////////////////////////////////
extern inline uint32 MakeDWord(uchar byte0, uchar byte1, uchar byte2, uchar byte3)
{
	return 	  (((uint32)byte0)      )
			| (((uint32)byte1)<<0x08)
			| (((uint32)byte2)<<0x10)
			| (((uint32)byte3)<<0x18);
}

/// Swap two bytes in a byte stream
extern inline void SwapTwoBytes(char *p)
{	if(p)
	{	char tmp =p[0];
		p[0] = p[1];
		p[1] = tmp;
	}
}

/// Swap the bytes within a 16-bit WORD.
extern inline ushort SwapTwoBytes(ushort w)
{
    return	  ((w & 0x00FF) << 0x08)
			| ((w & 0xFF00) >> 0x08);
}

/// Swap the 4 bytes in a byte stream
extern inline void SwapFourBytes(char *p)
{	if(p)
	{	char tmp;
		tmp  = p[0];
		p[0] = p[3];
		p[3] = tmp;
		tmp  = p[1];
		p[1] = p[2];
		p[2] = tmp;
	}
}
// Swap the 4 bytes within a 32-bit DWORD.
extern inline uint32 SwapFourBytes(uint32 w)
{
    return	  ((w & 0x000000FF) << 0x18)
			| ((w & 0x0000FF00) << 0x08)
			| ((w & 0x00FF0000) >> 0x08)
			| ((w & 0xFF000000) >> 0x18);
}


//////////////////////////////////////////////////////////////////////////
/// Check the byte-order of the CPU.
//////////////////////////////////////////////////////////////////////////
#define LSB_FIRST        0
#define MSB_FIRST        1
extern inline int CheckByteOrder(void)
{
    static short  w = 0x0001;
    static char  *b = (char *) &w;
    return(b[0] ? LSB_FIRST : MSB_FIRST);
}


//////////////////////////////////////////////////////////////////////////
// some bit twiddling
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an N-bit integer in O(lg(N)) operations
// in this case for 32bit input it would be 11 operations
#ifdef log2 //glibc defines this as macro
#undef log2
#endif
inline unsigned long log2(unsigned long v)
{
//	static const unsigned long b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
//	static const unsigned long S[] = {1, 2, 4, 8, 16};
	// result of log2(v) will go here
	register uint32 c = 0; 
//	int i;
//	for (i = 4; i >= 0; i--) 
//	{
//	  if (v & b[i])
//	  {
//		v >>= S[i];
//		c |= S[i];
//	  } 
//	}
	// unroll for speed...
//	if (v & b[4]) { v >>= S[4]; c |= S[4]; } 
//	if (v & b[3]) { v >>= S[3]; c |= S[3]; }
//	if (v & b[2]) { v >>= S[2]; c |= S[2]; }
//	if (v & b[1]) { v >>= S[1]; c |= S[1]; }
//	if (v & b[0]) { v >>= S[0]; c |= S[0]; }
	// put values in for more speed...
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	if (v & ULLCONST(0xFFFFFFFF00000000)) { v >>= 0x20; c |= 0x20; } 
#endif
	if (v & 0xFFFF0000) { v >>= 0x10; c |= 0x10; } 
	if (v & 0x0000FF00) { v >>= 0x08; c |= 0x08; }
	if (v & 0x000000F0) { v >>= 0x04; c |= 0x04; }
	if (v & 0x0000000C) { v >>= 0x02; c |= 0x02; }
	if (v & 0x00000002) { v >>= 0x01; c |= 0x01; }

	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an N-bit integer.
/// if you know it is a power of 2
inline unsigned long log2_(unsigned long v)
{
//	const unsigned long b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
//	register ulong c = ((v & b[0]) != 0);
//	int i;
//	for (i = 4; i >= 1; i--) 
//	{
//	  c |= ((v & b[i]) != 0) << i;
//	}
	// unroll for speed...
//	c |= ((v & b[4]) != 0) << 4;
//	c |= ((v & b[3]) != 0) << 3;
//	c |= ((v & b[2]) != 0) << 2;
//	c |= ((v & b[1]) != 0) << 1;
//	c |= ((v & b[0]) != 0) << 0;
	// unroll for speed...
	// put values in for more speed...
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	register ulong c = ((v & ULLCONST(0xAAAAAAAAAAAAAAAA)) != 0);
	c |= ((v & ULLCONST(0xFFFFFFFF00000000)) != 0) << 5;
	c |= ((v & ULLCONST(0xFFFF0000FFFF0000)) != 0) << 4;
	c |= ((v & ULLCONST(0xFF00FF00FF00FF00)) != 0) << 3;
	c |= ((v & ULLCONST(0xF0F0F0F0F0F0F0F0)) != 0) << 2;
	c |= ((v & ULLCONST(0xCCCCCCCCCCCCCCCC)) != 0) << 1;
	c |= ((v & ULLCONST(0xAAAAAAAAAAAAAAAA)) != 0) << 0;
#else
	register ulong c = ((v & 0xAAAAAAAA) != 0);
	c |= ((v & 0xFFFF0000) != 0) << 4;
	c |= ((v & 0xFF00FF00) != 0) << 3;
	c |= ((v & 0xF0F0F0F0) != 0) << 2;
	c |= ((v & 0xCCCCCCCC) != 0) << 1;
	c |= ((v & 0xAAAAAAAA) != 0) << 0;
#endif
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Find the log base 2 of an integer with a lookup table.
// The lookup table method takes only about 7 operations 
// to find the log of a 32-bit value. 
// extended for 64-bit quantities, it would take roughly 9 operations.
extern inline unsigned long log2t(unsigned long v)
{
	static const unsigned char LogTable256[] = 
	{
	  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	register unsigned long c; // c will be lg(v)
	register unsigned long t;
	register unsigned long tt;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	register unsigned long ttt = (v >> 32);
	if(ttt)
	{
		tt = (ttt >> 16);
		if(tt)
		{
			t = v >> 24;
			c = (t) ? 32+24 + LogTable256[t] : 32+16 + LogTable256[tt & 0xFF];
		}
		else 
		{
			t = v & 0xFF00;
			c = (t) ? 32+8 + LogTable256[t >> 8] : 32+LogTable256[v & 0xFF];
		}
	}
	else 
#endif
	{
		tt = (v >> 16);
		if(tt)
		{
			t = v >> 24;
			c = (t) ? 24 + LogTable256[t] : 16 + LogTable256[tt & 0xFF];
		}
		else 
		{
			t = v & 0xFF00;
			c = (t) ? 8 + LogTable256[t >> 8] : LogTable256[v & 0xFF];
		}
	}

	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, in parallel.
inline unsigned long bit_count(unsigned long v)
{
//	static const ulong S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
//	static const ulong B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
//	v = ((v >> S[0]) & B[0]) + (v & B[0]);
//	v = ((v >> S[1]) & B[1]) + (v & B[1]);
//	v = ((v >> S[2]) & B[2]) + (v & B[2]);
//	v = ((v >> S[3]) & B[3]) + (v & B[3]);
//	v = ((v >> S[4]) & B[4]) + (v & B[4]);
	// put values in
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v = ((v >> 0x01) & ULLCONST(0x5555555555555555)) + (v & ULLCONST(0x5555555555555555));
	v = ((v >> 0x02) & ULLCONST(0x3333333333333333)) + (v & ULLCONST(0x3333333333333333));
	v = ((v >> 0x04) & ULLCONST(0x0F0F0F0F0F0F0F0F)) + (v & ULLCONST(0x0F0F0F0F0F0F0F0F));
	v = ((v >> 0x08) & ULLCONST(0x00FF00FF00FF00FF)) + (v & ULLCONST(0x00FF00FF00FF00FF));
	v = ((v >> 0x10) & ULLCONST(0x0000FFFF0000FFFF)) + (v & ULLCONST(0x0000FFFF0000FFFF));
	v = ((v >> 0x20) & ULLCONST(0x00000000FFFFFFFF)) + (v & ULLCONST(0x00000000FFFFFFFF));
#else
	v = ((v >> 0x01) & 0x55555555) + (v & 0x55555555);
	v = ((v >> 0x02) & 0x33333333) + (v & 0x33333333);
	v = ((v >> 0x04) & 0x0F0F0F0F) + (v & 0x0F0F0F0F);
	v = ((v >> 0x08) & 0x00FF00FF) + (v & 0x00FF00FF);
	v = ((v >> 0x10) & 0x0000FFFF) + (v & 0x0000FFFF);
#endif
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, with table.
inline unsigned long bit_count_t(unsigned long v)
{
	// Counting bits set by lookup table 
	static const unsigned char BitsSetTable256[] = 
	{
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};
	unsigned long c=0; // c is the total bits set in v
	while(v)
	{
		c += BitsSetTable256[v&0xff];
		v >>= NBBY;
	}
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set, Brian Kernighan's way.
inline unsigned long bit_count_k(unsigned long v)
{
	unsigned long c; // c accumulates the total bits set in v
	for (c = 0; v; ++c)
	{	// clear the least significant bit set
		v &= v - 1; 
	}
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 12-bit words using 64-bit instructions.
inline unsigned long bit_count_12_i64(unsigned long v)
{	
	unsigned long c; // c accumulates the total bits set in v
	// option 1, for at most 12-bit values in v:
	c = (v * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f;
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 24-bit words using 64-bit instructions.
inline unsigned long bit_count_24_i64(unsigned long v)
{	// option 2, for at most 24-bit values in v:
	unsigned long c; // c accumulates the total bits set in v
	c =  (v & 0xfff) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421);
	c += ((v & 0xfff000) >> 12) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421);
	c %= 0x1f;
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// Counting bits set in 32-bit words using 64-bit instructions.
inline unsigned long bit_count_32_i64(unsigned long v)
{	// option 3, for at most 32-bit values in v:
	unsigned long c; // c accumulates the total bits set in v
	c = (((v & 0xfff) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) +
		((v & 0xfff000) >> 12) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f; 
	c += ((v >> 24) * ULLCONST(0x1001001001001) & ULLCONST(0x84210842108421)) % 0x1f; 
	// This method requires a 64-bit CPU with fast modulus division to be efficient. 
	// The first option takes only 6 operations; the second option takes 13; 
	// and the third option takes 17. 
	return c;
}

//////////////////////////////////////////////////////////////////////////
/// calculate parity in parallel.
inline bool parity(unsigned long v)
{	// The method above takes around 9 operations for 32-bit words. 
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v ^= v >> 32;
#endif
	v ^= v >> 16;
	v ^= v >> 8;
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}

//////////////////////////////////////////////////////////////////////////
/// calculate parity using Brian Kernigan's bit counting.
inline bool parity_k(unsigned long v)
{	// using Brian Kernigan's bit counting
	// The time it takes is proportional to the number of bits set. 
	bool parity = false;  // parity will be the parity of b
	while (v)
	{
		parity = !parity;
		v = v & (v - 1);
	}
	return parity;
}

//////////////////////////////////////////////////////////////////////////
/// Compute parity of a byte using 64-bit multiply and modulus division.
// The method takes around 7 operations, but only works on bytes. 
inline bool parity_b(unsigned char v)
{	
	return (((v * ULLCONST(0x0101010101010101)) & ULLCONST(0x8040201008040201)) % 0x1FF) & 1;
}

//////////////////////////////////////////////////////////////////////////
/// Compute parity with table.
inline bool parity_t(unsigned long v)
{
	//Thanks to Mathew Hendry for pointing out the shift-lookup idea at 
	// the end on Dec. 15, 2002. That optimization shaves two operations off 
	// using only shifting and XORing to find the parity.
	// Compute parity by lookup table 
	static const bool ParityTable[] = 
	{
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
	};
	bool parity=false;
	while(v)
	{
		parity ^= ParityTable[(v & 0xff)];
		v >>= NBBY;
	}
	return parity;
}

//////////////////////////////////////////////////////////////////////////
/// Reverse the bits in a byte with 7 operations.
inline uchar bit_reverse(uchar b)
{
	return (uchar)(((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
}

//////////////////////////////////////////////////////////////////////////
/// Reverse the bits in a byte with 64bit instructions.
inline uchar bit_reverse_i64(uchar b)
{	// The multiply operation creates five separate copies of the 8-bit byte 
	// pattern to fan-out into a 64-bit value. The and operation selects the bits that 
	// are in the correct (reversed) positions, relative to each 10-bit groups of bits. 
	// The multiply and the and operations copy the bits from the original byte 
	// so they each appear in only one of the 10-bit sets. 
	// The reversed positions of the bits from the original byte coincide with 
	// their relative positions within any 10-bit set. 
	// The last step, which involves modulus division by 2^10 - 1, 
	// has the effect of merging together each set of 10 bits 
	// (from positions 0-9, 10-19, 20-29, ...) in the 64-bit value. 
	// They do not overlap, so the addition steps underlying the modulus division 
	// behave like or operations. 
	return (b * ULLCONST(0x0202020202) & ULLCONST(0x010884422010)) % 1023;
}

//////////////////////////////////////////////////////////////////////////
/// Reverse an N-bit quantity in parallel in 5 * lg(N) operations.
// This method is best suited to situations where N is large.
// Any reasonable optimizing C compiler should treat the dereferences 
// of B, ~B, and S as constants, requiring no evaluation other than perhaps 
// a load operation for some of the B and ~B references.
// See Dr. Dobb's Journal 1983, Edwin Freed's article on Binary Magic Numbers 
// for more information. 
// Anyway I would not count on that, so I put values explicitely.
inline unsigned long bit_reverse(unsigned long v)
{
//	static const ulong S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
//	static const ulong B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
//	v = ((v >> S[0]) & B[0]) | ((v << S[0]) & ~B[0]); // swap odd and even bits
//	v = ((v >> S[1]) & B[1]) | ((v << S[1]) & ~B[1]); // swap consecutive pairs
//	v = ((v >> S[2]) & B[2]) | ((v << S[2]) & ~B[2]); // swap nibbles ...
//	v = ((v >> S[3]) & B[3]) | ((v << S[3]) & ~B[3]);
//	v = ((v >> S[4]) & B[4]) | ((v << S[4]) & ~B[4]);
	// better set it by hand
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v = ((v >> 0x01) & ULLCONST(0x5555555555555555)) | ((v << 0x01) & ~ULLCONST(0x5555555555555555));
	v = ((v >> 0x02) & ULLCONST(0x3333333333333333)) | ((v << 0x02) & ~ULLCONST(0x3333333333333333));
	v = ((v >> 0x04) & ULLCONST(0x0F0F0F0F0F0F0F0F)) | ((v << 0x04) & ~ULLCONST(0x0F0F0F0F0F0F0F0F));
	v = ((v >> 0x08) & ULLCONST(0x00FF00FF00FF00FF)) | ((v << 0x08) & ~ULLCONST(0x00FF00FF00FF00FF));
	v = ((v >> 0x10) & ULLCONST(0x0000FFFF0000FFFF)) | ((v << 0x10) & ~ULLCONST(0x0000FFFF0000FFFF));
	v = ((v >> 0x20) & ULLCONST(0x00000000FFFFFFFF)) | ((v << 0x20) & ~ULLCONST(0x00000000FFFFFFFF));
#else
	v = ((v >> 0x01) & 0x55555555) | ((v << 0x01) & ~0x55555555);
	v = ((v >> 0x02) & 0x33333333) | ((v << 0x02) & ~0x33333333);
	v = ((v >> 0x04) & 0x0F0F0F0F) | ((v << 0x04) & ~0x0F0F0F0F);
	v = ((v >> 0x08) & 0x00FF00FF) | ((v << 0x08) & ~0x00FF00FF);
	v = ((v >> 0x10) & 0x0000FFFF) | ((v << 0x10) & ~0x0000FFFF);
#endif
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// check if a number is power of 2.
/// roughly; if one (and only one) bit is set
extern inline bool isPowerOf2(unsigned long i)
{
	//return (i & (i - 1)) == 0; 
	// with drawback that 0 is incorrectly considered a power of 2
	// therefore:
	//return (i > 0) && (0==(i & (i - 1)));
	// or more short:
	return i && !(i & (i - 1));
}

//////////////////////////////////////////////////////////////////////////
/// round up to the next power of 2.
extern inline unsigned long RoundPowerOf2(unsigned long v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	v |= v >> 32;
#endif
	v++;
	return v;
}

//////////////////////////////////////////////////////////////////////////
/// Compute modulus division by 1 << s without a division operator.
extern inline unsigned long moduloPowerOf2(unsigned long v, unsigned long s)
{	// Most programmers learn this trick early, but it was included for the 
	// sake of completeness. 
	return v & ( (1<<s) - 1); // v % 2^s
}

//////////////////////////////////////////////////////////////////////////
/// Compute modulus division by (1 << s) - 1 without a division operator.
extern inline unsigned long moduloPowerOf2_1(unsigned long v, unsigned long s)
{	// This method of modulus division by an integer that is one less than 
	// a power of 2 takes at most 5 + (4 + 5 * ceil(N / s)) * ceil(lg(N / s)) operations, 
	// where N is the number of bits in the numerator. 
	// In other words, it takes at most O(N * lg(N)) time. 
	const unsigned long d = (1 << s) - 1; // so d is either 1, 3, 7, 15, 31, ...).
	unsigned long m;                      // n % d goes here.
	for (m=v; v>d; v=m)
	for (m=0; v; v >>= s)
	{
		m += v & d;
	}
	// Now m is a value from 0 to d, but since with modulus division
	// we want m to be 0 when it is d.
	m = (m==d) ? 0 : m; // or: ((m + 1) & d) - 1;
	return m;
}

//////////////////////////////////////////////////////////////////////////
/// Determine if a word has a zero byte.
inline bool has_zeros(unsigned long v)
{	// check if any 8-bit byte in it is 0
#if defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
	return (bool)(~((((v & ULLCONST(0x7F7F7F7F7F7F7F7F)) + ULLCONST(0x7F7F7F7F7F7F7F7F)) | v) | ULLCONST(0x7F7F7F7F7F7F7F7F)));
#else
	return (bool)(~((((v & 0x7F7F7F7F) + 0x7F7F7F7F) | v) | 0x7F7F7F7F));
#endif
	// The code works by first zeroing the high bits of the 4 bytes in the word. 
	// Next, it adds a number that will result in an overflow to the high bit of 
	// a byte if any of the low bits were initialy set. 
	// Next the high bits of the original word are ORed with these values; 
	// thus, the high bit of a byte is set if any bit in the byte was set. 
	// Finally, we determine if any of these high bits are zero 
	// by ORing with ones everywhere except the high bits and inverting the result. 

	// The code above may be useful when doing a fast string copy 
	// in which a word is copied at a time; 
	// it uses at most 6 operations (and at least 5). 
	// On the other hand, testing for a null byte in the obvious ways 
	// have at least 7 operations (when counted in the most sparing way), 
	// and at most 12:
	// bool hasNoZeroByte = ((v & 0xff) && (v & 0xff00) && (v & 0xff0000) && (v & 0xff000000))
	// or:
	// unsigned char * p = (unsigned char *) &v;  
	// bool hasNoZeroByte = *p && *(p + 1) && *(p + 2) && *(p + 3);
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) using binary magics.
/// are useful for linearizing 2D integer coordinates, 
/// so x and y are combined into a single number that can be compared 
/// easily and has the property that a number is usually close to another 
/// if their x and y values are close. 
inline unsigned long interleave(unsigned short x, unsigned short y)
{	// Interleave bits by Binary Magic Numbers 
	const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
	const unsigned int S[] = {1, 2, 4, 8};
	x = (x | (x << S[3])) & B[3];
	x = (x | (x << S[2])) & B[2];
	x = (x | (x << S[1])) & B[1];
	x = (x | (x << S[0])) & B[0];
	y = (y | (y << S[3])) & B[3];
	y = (y | (y << S[2])) & B[2];
	y = (y | (y << S[1])) & B[1];
	y = (y | (y << S[0])) & B[0];
	return x | (y << 1);
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) the obvious way.
inline unsigned long interleave_trivial(unsigned short x, unsigned short y)
{	// bits of x are in the even positions and y in the odd;
	unsigned int z = 0; // z gets the resulting 32-bit Morton Number.
	size_t i;
	for(i=0; i<sizeof(x)*NBBY; ++i)// unroll for more speed...
	{
		z |= (x & 1 << i) << i | (y & 1 << i) << (i + 1);
	}
	return z;
}

//////////////////////////////////////////////////////////////////////////
/// Interleaved bits (aka Morton numbers) using a table.
inline unsigned long interleave_t(unsigned short x, unsigned short y)
{
	static const unsigned short MortonTable256[] = 
	{
		0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 
		0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, 
		0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 
		0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, 
		0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 
		0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, 
		0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
		0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
		0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
		0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
		0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
		0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
		0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
		0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
		0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
		0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
		0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
		0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
		0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
		0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
		0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
		0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
		0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
		0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
		0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
		0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
		0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
		0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
		0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 
		0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
		0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
		0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
	};
	unsigned long z;   // gets the resulting 32-bit Morton Number.
	z = (MortonTable256[y & 0xFF] << 1) | MortonTable256[x & 0xFF] |
		(MortonTable256[y >> 8] << 1 | MortonTable256[x >> 8]) << 16;


	z = (MortonTable256[y & 0xFF] << 1) | MortonTable256[x & 0xFF] |
		(MortonTable256[y >> 8] << 1 | MortonTable256[x >> 8]) << 16;
	return z;
}

//////////////////////////////////////////////////////////////////////////
/// square root with newton approximation.
/// starting condition calculated with two approximations
/// sqrt(n) = n^1/2 = n / n^1/2 = n / 2^log2(n^1/2) = n / 2^(log2(n)/2)
/// and calculating a/2^b with left shift as a>>b
/// which results in a larger value than necessary 
/// because the integer log2 returns the floored logarism and is smaller than the real log2
/// second approximation is
/// sqrt(n) = n^1/2 = 2^(log2(n)/2) which is calculated as 1<<(log2(n)/2)
/// resulting in a value smaller than necessary because of the integer log2 
/// calculation the mean of those two approximations gets closer to the real value, 
/// only slightly faster than the buildin double sqrt and therefore useless
#ifdef isqrt
#undef isqrt
#endif
template<class T> static inline T isqrt(const T& n)
{
	if(n>0)
	{
		T q=0, xx = (log2(n)/2), qx = ((n>>xx) + (1<<xx))/2;
		do
		{
			q  = qx;
			qx = (q + n/q)/2;
		}
		while( q!=qx && q+1!=qx );
		return q;
	}
	// should set matherr or throw something when negative
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/// The expression evaluates ie. sign = v >> 31 for 32-bit integers. 
/// This is one operation faster than the obvious way, 
/// sign = -(v > 0). This trick works because when integers are shifted right, 
/// the value of the far left bit is copied to the other bits. 
/// The far left bit is 1 when the value is negative and 0 otherwise; 
template<class T> inline T sign(const T& v)
{
	T sign;   // the result goes here 

	// if v < 0 then -1, else 0
	//sign = v >> (sizeof(T) * NBBY - 1); 

	// if v < 0 then -1, else +1
	//sign = +1 | (v >> (sizeof(int) * 8 - 1));
	
	// Alternatively, for -1, 0, or +1
	sign = (v != 0) | (v >> (sizeof(int) * 8 - 1));  // -1, 0, or +1
	return sign;
	// Caveat: On March 7, 2003, Angus Duggan pointed out that the 1989 ANSI C 
	// specification leaves the result of signed right-shift implementation-defined, 
	// so on some systems this hack might not work. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the integer absolute value (abs) without branching.
template<class T> inline T iabs(const T& v)
{	
	return (+1 | (v >> (sizeof(T) * NBBY - 1))) * v;
	// Some CPUs don't have an integer absolute value instruction 
	// (or the compiler fails to use them). On machines where branching 
	// is expensive, the above expression can be faster than the obvious 
	// approach, r = (v < 0) ? -v : v, even though the number of operations 
	// is the same. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the minimum (min) of two integers without branching.
template<class T> inline T imin(const T& x, const T& y)
{	
	return y + ((x - y) & ((x - y) >> (sizeof(T) * NBBY - 1))); // min(x, y)
	// On machines where branching is expensive, the above expression can be faster 
	// than the obvious approach, r = (x < y) ? x : y, 
	// even though it involves one more instruction. 
	// (Notice that (x - y) only needs to be evaluated once.) It works because 
	// if x < y, then (x - y) >> 31 will be all ones (on a 32-bit integer machine), 
	// so r = y + (x - y) & ~0 = y + x - y = x. 
	// Otherwise, if x >= y, then (x - y) >> 31 will be all zeros, 
	// so r = y + (x - y) & 0 = y. 
}

//////////////////////////////////////////////////////////////////////////
/// Compute the maximum (max) of two integers without branching.
template<class T> inline T imax(const T& x, const T& y)
{	
	return  x - ((x - y) & ((x - y) >> (sizeof(int) * NBBY - 1))); // max(x, y)
	// On machines where branching is expensive, 
	// the above expression can be faster than the obvious approach, 
	// r = (x < y) ? x : y, even though it involves one more instruction. 
	// (Notice that (x - y) only needs to be evaluated once.) It works because 
	// if x < y, then (x - y) >> 31 will be all ones (on a 32-bit integer machine), 
	// so r = y + (x - y) & ~0 = y + x - y = x. 
	// Otherwise, if x >= y, then (x - y) >> 31 will be all zeros, 
	// so r = y + (x - y) & 0 = y. 
}

//////////////////////////////////////////////////////////////////////////
/// calculate pow n on base 2.
#ifdef pow2 // just to be sure
#undef pow2
#endif
extern inline unsigned long pow2(unsigned long v)
{
	if( v < NBBY*sizeof(unsigned long) )
		return 1<<v;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/// calculate 10 to the power of exp using a integer operations with table.
#ifdef pow10
#undef pow10
#endif
inline uint64 pow10(uint exp)
{
	static const uint64 table[] = { ULLCONST(100000000), ULLCONST(10), ULLCONST(100), ULLCONST(1000), 
									ULLCONST(10000), ULLCONST(100000) , ULLCONST(1000000), ULLCONST(10000000)};
	if(!exp)
		return 1;
	else if(exp<8)
		return table[exp&0x7];
	else if(exp>19)
		return UINT64_MAX;

	uint64 res=table[0];
	exp -= 8;
	while(exp)
	{
		if(exp<8)
		{
			res *= table[exp&0x7];
			break;
		}
		else
		{
			res*=table[0];
			exp -= 8;
		}
	}
	return res;
}

NAMESPACE_END(basics)

#endif//__BASETYPES_H__
