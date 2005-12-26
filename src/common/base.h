#ifndef	_BASE_H_
#define _BASE_H_

//////////////////////////////////////////////////////////////////////////
// basic include for all basics
// introduces types and global functions
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// some global switches
//////////////////////////////////////////////////////////////////////////

#define COUNT_GLOBALS		// enables countage of created objects
#define CHECK_BOUNDS		// enables boundary check for arrays and lists
#define CHECK_LOCKS			// enables check of locking/unlocking sync objects
#define SINGLETHREAD		// builds without multithread guards
#define CHECK_EXCEPTIONS	// use exceptions for "exception" handling




//////////////////////////////////////////////////////////////////////////
// setting some defines on platforms
//////////////////////////////////////////////////////////////////////////
#if (defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(_MSC_VER) || defined(__BORLANDC__)) && !defined(WIN32)
#define WIN32
#endif

// __APPLE__ is the only predefined macro on MacOS X
#if defined(__APPLE__)
#define __DARWIN__
#endif

// 64bit OS
#if defined(_M_IA64) || defined(_M_X64) || defined(_WIN64) || defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
#define __64BIT__
#endif


#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif


//////////////////////////////////////////////////////////////////////////
// setting some defines for compile modes
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
// System specific defined
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//////////////////////////////
#define WIN32_LEAN_AND_MEAN	// stuff
#define _WINSOCKAPI_		// prevent inclusion of winsock.h
//#define _USE_32BIT_TIME_T	// use 32 bit time variables on 64bit windows
//#define __USE_W32_SOCKETS
//////////////////////////////
#endif
//////////////////////////////


#undef FD_SETSIZE
#define FD_SETSIZE 4096


//////////////////////////////////////////////////////////////////////////
// includes
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <signal.h>

#include <assert.h>

//////////////////////////////
#ifdef WIN32
//////////////////////////////
#include <windows.h>
#include <winsock2.h>
#include <conio.h>
//#include <crtdbg.h>
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
#include <net/if.h>		// needs but does not include <sys/socket.h>
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
// no c support anymore
//////////////////////////////////////////////////////////////////////////
#ifndef __cplusplus
#  error "this is C++ source"
#endif


//////////////////////////////////////////////////////////////////////////
// c++ header 
//////////////////////////////////////////////////////////////////////////
#include <typeinfo>		//!! this is poisoning the global namespace




//////////////////////////////
#ifdef WIN32
//////////////////////////////
#pragma warning(disable : 4996)	// disable deprecated warnings
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4244) // converting type on return will shorten
#pragma warning(disable : 4310)	// converting constant will shorten
#pragma warning(disable : 4706) // assignment within conditional
#pragma warning(disable : 4127)	// constant assignment
#pragma warning(disable : 4710)	// is no inline function
#pragma warning(disable : 4511)	// no copy constructor
#pragma warning(disable : 4512)	// no assign operator
//////////////////////////////
#endif
//////////////////////////////



//////////////////////////////////////////////////////////////////////////
// useful typedefs
//////////////////////////////////////////////////////////////////////////
typedef   signed int    sint;	// don't use (only for ie. scanf)
typedef unsigned int    uint;	// don't use
typedef   signed long   slong;	// don't use (only for ie. file-io)
typedef unsigned long   ulong;	// don't use
typedef   signed short  sshort;
typedef unsigned short  ushort;
typedef unsigned char   uchar;
typedef   signed char   schar;

typedef char*           pschar;
typedef unsigned char*  puchar;
typedef const char*     pcschar;
typedef void*           ptr;
typedef int*            pint;


//////////////////////////////////////////////////////////////////////////
// integer with exact processor width (and best speed)

//						size_t already defined in stdio.h
#ifdef WIN32 // does not have a signed size_t
#if (defined _WIN64)  // naive 64bit windows platform
typedef __int64			ssize_t;
#else
typedef int				ssize_t;
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// we need to define our own fixed byte types
typedef   signed char	sint08;
typedef unsigned char	uint08;
typedef   signed short	sint16;
typedef unsigned short	uint16;
//////////////////////////////////////////////////////////////////////////
// IMPORTANT: 
// ints beeing regular machine dependend type is over starting with 64bit arch
// 64bit gnu will define ints as 32bit but long as 64bit,
// while 64bit windows will define both int and long as 4 byte long, introducing __int64 as 64bit integer type
// other platforms vary int size dependend on machine width but keep long constant
#ifdef WIN32
typedef long			sint32;
typedef unsigned long	uint32;
#else
typedef int				sint32;
typedef unsigned int	uint32;
#endif


//////////////////////////////////////////////////////////////////////////
// portable 64-bit integers
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) || defined(__BORLANDC__)
 typedef __int64             sint64;
 typedef unsigned __int64    uint64;
#define LLCONST(a) (a##i64)
#else
 typedef long long           sint64;
 typedef unsigned long long  uint64;
#define LLCONST(a) (a##ll)
#endif

#ifndef INT64_MIN
#define INT64_MIN  (LLCONST(-9223372036854775807)-1)
#endif
#ifndef INT64_MAX
#define INT64_MAX  (LLCONST(9223372036854775807))
#endif
#ifndef UINT64_MAX
#define UINT64_MAX (LLCONST(18446744073709551615u))
#endif



//////////////////////////////
#ifdef __cplusplus
//////////////////////////////

// starting over in another project and transfering 
// after coding and testing is finished


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
{	if(i1 > i2) return (T&)i1; else return (T&)i2;
}

#ifdef swap // just to be sure
#undef swap
#endif
template <class T> inline void swap(T &i1, T &i2)
{	T dummy = i1; i1=i2; i2=dummy;
}

//////////////////////////////
#else // not cplusplus
//////////////////////////////

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



//////////////////////////////
// socketlen type definition
//////////////////////////////
#ifndef __socklen_t_defined
#define __socklen_t_defined	// define socklen_t if not already defined

#ifdef WIN32
  typedef int socklen_t;	// type in an int on windows
#elif CYGWIN				// stupid cygwin is not standard conform
  #ifdef socklen_t			// it has a define for this instead a typedef as it should have
  #undef socklen_t
  #endif
  typedef int socklen_t;	// define it pr
#else// normal unix with undefined socklen_t
  typedef unsigned int socklen_t;
#endif
  
#endif//__socklen_t_defined



#ifndef NULL
#define NULL (void *)0
#endif

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define Assert(EX) //assert(EX)



//////////////////////////////
#ifdef WIN32
//////////////////////////////

// keywords
#ifndef __cplusplus
#define inline __inline
#endif

// defines
// txtやlogなどの書き出すファイルの改行コード
#define RETCODE	"\r\n"	// (CR/LF：Windows系)
#define RET RETCODE

#define PATHSEP '\\'



// abnormal function definitions
// no inlining them because of varargs; those might not conflict in defines anyway
#define vsnprintf			_vsnprintf
#define snprintf			_snprintf


#define strcasecmp			stricmp
#define strncasecmp			strnicmp


static inline int read(SOCKET fd, char*buf, int sz)		
{
	return recv(fd,buf,sz,0); 
}
static inline int write(SOCKET fd, char*buf, int sz)	
{
	return send(fd,buf,sz,0); 
}


// missing functions and helpers
static inline int gettimeofday(struct timeval *timenow, void *tz)
{
    if (timenow)
    {
		FILETIME	ft;
		GetSystemTimeAsFileTime(&ft);
		
#if !(defined __64BIT__)	// not a naive 64bit platform
		/////////////////////////////////////////////////////////////////////////////	
		// Apparently Win32 has units of 1e-7 sec (100-nanosecond intervals)
		// 4294967296 is 2^32, to shift high word over
		// 11644473600 is the number of seconds between
		// the Win32 epoch 1601-Jan-01 and the Unix epoch 1970-Jan-01
		// Tests found floating point to be 10x faster than 64bit int math.
		double timed = ((ft.dwHighDateTime * 4294967296e-7) - 11644473600.0) + (ft.dwLowDateTime  * 1e-7);
		
		timenow->tv_sec  = (long) timed;
		timenow->tv_usec = (long) ((timed - timenow->tv_sec) * 1e6);
#else
		/////////////////////////////////////////////////////////////////////////////	
		// and the same with 64bit math
		// which might be faster on a real 64bit platform
		LARGE_INTEGER   li;
		__int64         t;
		static const __i64 EPOCHFILETIME = (116444736000000000i64)
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       // time in 100-nanosecond intervals
        t -= EPOCHFILETIME;     // offset to the epoch time
        t /= 10;                // time in microseconds

        timenow->tv_sec  = (long)(t / 1000000);
        timenow->tv_usec = (long)(t % 1000000);
#endif
    }
	/////////////////////////////////////////////////////////////////////////////////
	/*
	void*tz should be struct timezone *tz
		with
	struct timezone {
		int     tz_minuteswest; // minutes W of Greenwich
		int     tz_dsttime;     // type of dst correction
	};
	but has never been used, because the daylight saving could not be defined by an algorithm

	the code to actually use this structures would be:
    if (tz)
    {
		static int      tzflag=0;
        if (!tzflag)
        {	// run tzset once in application livetime
			// to set _timezone and _daylight according to system specification
            _tzset();
            tzflag++;
        }
		// copy the global values out
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
	*/
	/////////////////////////////////////////////////////////////////////////////////

	return 0;
}

//////////////////////////////
#else/////////////////////////
//////////////////////////////


// keywords

// defines

// txtやlogなどの書き出すファイルの改行コード
#define RETCODE "\n"	// (LF：Unix系）
#define RET RETCODE

#define PATHSEP '/'

// typedefs

typedef int		SOCKET;

// abnormal function definitions

static inline int closesocket(SOCKET fd)		
{
	return close(fd); 
}
static inline int ioctlsocket(SOCKET fd, long cmd, unsigned long *arg)		
{
	return ioctl(fd,cmd,arg); 
}



// missing functions and helpers

// missing TickCount on Unix
static inline unsigned long GetTickCount()
{
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec * 1000 + tval.tv_usec / 1000;
}

static inline unsigned long GetCurrentProcessId()
{	
	return getpid();
}

//////////////////////////////
#endif////////////////////////
//////////////////////////////


extern inline uint sleep(uint milliseconds)
{
#if defined(WIN32)
    Sleep(milliseconds);
#elif defined(__sun__)
    poll(0, 0, milliseconds);
#else
    usleep( ((useconds_t)milliseconds) * 1000 );
#endif
	return milliseconds;
}






//////////////////////////////////////////////////////////////////////////
// seek defines do not always exist
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

namespace eapp
{
//////////////////////////////////////////////////////////////////////////
// wrappers for Character Classification Routines  
//////////////////////////////////////////////////////////////////////////
#ifdef isalpha	// get the function form, not the macro
#undef isalpha
#endif
static inline char isalpha(char val)	{ return (char)::isalpha((int)((unsigned char)val)); }
#ifdef isupper	// get the function form, not the macro
#undef isupper
#endif
static inline char isupper(char val)	{ return isupper((int)((unsigned char)val)); }
#ifdef islower	// get the function form, not the macro
#undef islower
#endif
static inline char islower(char val)	{ return islower((int)((unsigned char)val)); }
#ifdef isdigit	// get the function form, not the macro
#undef isdigit
#endif
static inline char isdigit(char val)	{ return isdigit((int)((unsigned char)val)); }
#ifdef isxdigit	// get the function form, not the macro
#undef isxdigit
#endif
static inline char isxdigit(char val)	{ return isxdigit((int)((unsigned char)val)); }
#ifdef isspace	// get the function form, not the macro
#undef isspace
#endif
static inline char isspace(char val)	{ return isspace((int)((unsigned char)val)); }
#ifdef ispunct	// get the function form, not the macro
#undef ispunct
#endif
static inline char ispunct(char val)	{ return ispunct((int)((unsigned char)val)); }
#ifdef isalnum	// get the function form, not the macro
#undef isalnum
#endif
static inline char isalnum(char val)	{ return isalnum((int)((unsigned char)val)); }
#ifdef isprint	// get the function form, not the macro
#undef isprint
#endif
static inline char isprint(char val)	{ return isprint((int)((unsigned char)val)); }
#ifdef isgraph	// get the function form, not the macro
#undef isgraph
#endif
static inline char isgraph(char val)	{ return isgraph((int)((unsigned char)val)); }
#ifdef iscntrl	// get the function form, not the macro
#undef iscntrl
#endif
static inline char iscntrl(char val)	{ return iscntrl((int)((unsigned char)val)); }
#ifdef toupper	// get the function form, not the macro
#undef toupper
#endif
static inline char toupper(char val)	{ return toupper((int)((unsigned char)val)); }
#ifdef tolower	// get the function form, not the macro
#undef tolower
#endif
static inline char tolower(char val)	{ return tolower((int)((unsigned char)val)); }

}

//////////////////////////////////////////////////////////////////////////
// byte word dword access
//////////////////////////////////////////////////////////////////////////

static inline unsigned char GetByte(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (unsigned char)((val & 0x000000FF)      );
	case 1:
		return (unsigned char)((val & 0x0000FF00)>>0x08);
	case 2:
		return (unsigned char)((val & 0x00FF0000)>>0x10);
	case 3:
		return (unsigned char)((val & 0xFF000000)>>0x18);
	default:
		return 0;	//better throw something here
	}
}
static inline unsigned short GetWord(uint32 val, size_t num)
{
	switch(num)
	{
	case 0:
		return (unsigned short)((val & 0x0000FFFF)      );
	case 1:
		return (unsigned short)((val & 0xFFFF0000)>>0x10);
	default:
		return 0;	//better throw something here
	}	
}
static inline unsigned short MakeWord(unsigned char byte0, unsigned char byte1)
{
	return byte0 | (byte1<<0x08);
}
static inline uint32 MakeDWord(unsigned short word0, unsigned short word1)
{
	return 	  ((uint32)word0)
			| ((uint32)word1<<0x10);
}

static inline uint32 MakeDWord(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3)
{
	return 	  ((uint32)byte0)
			| ((uint32)byte1<<0x08)
			| ((uint32)byte2<<0x10)
			| ((uint32)byte3<<0x18);
}

// Check the byte-order of the CPU.
#define LSB_FIRST        0
#define MSB_FIRST        1
static inline int CheckByteOrder(void)
{
    static short  w = 0x0001;
    static char  *b = (char *) &w;
    return(b[0] ? LSB_FIRST : MSB_FIRST);
}

// Swap the bytes within a 16-bit WORD.
static inline void SwapTwoBytes(char *p)
{	if(p)
	{	char tmp =p[0];
		p[0] = p[1];
		p[1] = tmp;
	}
}
static inline unsigned short SwapTwoBytes(unsigned short w)
{
    return	  ((w & 0x00FF) << 0x08)
			| ((w & 0xFF00) >> 0x08);
}

// Swap the bytes within a 32-bit DWORD.
static inline void SwapFourBytes(char *p)
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
static inline uint32 SwapFourBytes(uint32 w)
{
    return	  ((w & 0x000000FF) << 0x18)
			| ((w & 0x0000FF00) << 0x08)
			| ((w & 0x00FF0000) >> 0x08)
			| ((w & 0xFF000000) >> 0x18);
}


// glibc conflicting define
#ifdef log2
#undef log2
#endif
// Find the log base 2 of an N-bit integer in O(lg(N)) operations
// in this case for 32bit input it would be 11 operations
static inline unsigned long log2(unsigned long  v)
{
//	static const uint32 b[] = 
//		{0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
//	static const uint32 S[] = 
//		{1, 2, 4, 8, 16};
	// result of log2(v) will go here
	register unsigned long c = 0; 
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
// 64bit unix defines long as 64bit
#if (defined __64BIT__)
	if (v & LLCONST(0xFFFFFFFF00000000)) { v >>= 0x20; c |= 0x20; } 
#endif
	if (v & 0xFFFF0000) { v >>= 0x10; c |= 0x10; } 
	if (v & 0x0000FF00) { v >>= 0x08; c |= 0x08; }
	if (v & 0x000000F0) { v >>= 0x04; c |= 0x04; }
	if (v & 0x0000000C) { v >>= 0x02; c |= 0x02; }
	if (v & 0x00000002) { v >>= 0x01; c |= 0x01; }
	return c;
}
#ifdef pow2
#undef pow2
#endif
static inline unsigned long pow2(unsigned long v)
{
	if( v < 8*sizeof(v) )
		return 1<<v;
	return 0;
}

// newton approximation
// starting condition calculated with two approximations
// sqrt(n) = n^1/2 = n / n^1/2 = n / 2^log2(n^1/2) = n / 2^(log2(n)/2)
// and calculating a/2^b with left shift as a>>b
// which results in a larger value than necessary 
// because the integer log2 returns the floored logarism and is smaller than the real log2
// second approximation is
// sqrt(n) = n^1/2 = 2^(log2(n)/2) which is calculated as 1<<(log2(n)/2)
// resulting in a value smaller than necessary because of the integer log2 
// calculation the mean of those two approximations gets closer to the real value, 
#ifdef isqrt
#undef isqrt
#endif
template<class T> static inline T isqrt(T n)
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
// empty base classes
//////////////////////////////////////////////////////////////////////////
class noncopyable
{
// looks like some gcc really insists on having this members readable
// even if not used and not disturbing in any way, 
// stupid move, but ok, go on, read it 
// (but protected declaration will then generate linker errors instead of compiler errors)
#ifdef __GNUC__
protected:
#else
private:
#endif
	noncopyable(const noncopyable&);					// no copy
	const noncopyable& operator=(const noncopyable&);	// no assign
public:
	noncopyable()	{}
};
class global
{
public:
	global()	{}
};



//!!todo!! replace with full headers when switching to multithread

//////////////////////////////////////////////////////////////////////////
// empty sync classes for single thread 
//////////////////////////////////////////////////////////////////////////
class Mutex: public noncopyable
{
public:
    Mutex()					{}
    ~Mutex()				{}
	bool trylock() const	{return false;}
    void enter() const		{}
    void leave() const		{}
    void lock() const 		{}
    void unlock() const		{}
};

class ScopeLock: public noncopyable
{
public:
    ScopeLock(const Mutex& imtx)	{}
    ~ScopeLock()					{}
};







//////////////////////////////////////////////////////////////////////////
// the basic exception class. 
//////////////////////////////////////////////////////////////////////////
class CException : public global
{
protected:
    char *message;
public:
    CException(const char*   e):message(NULL)
	{
		if(e)
		{	// c function will just fail on memory error
			this->message = strdup( e );
		}
	}

    virtual ~CException()		{ free(this->message); }
	operator const char *()		{ return this->message; }
};


//////////////////////////////////////////////////////////////////////////
// exception for 'out of bound array access'
//////////////////////////////////////////////////////////////////////////
class exception_bound : public CException
{
public:
	exception_bound(const char*   e) : CException(e) {}
	virtual ~exception_bound()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'memory allocation failed'
//////////////////////////////////////////////////////////////////////////
class exception_memory : public CException
{
public:
	exception_memory(const char*   e) : CException(e)	{}
	virtual ~exception_memory()							{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'failed conversion'
//////////////////////////////////////////////////////////////////////////
class exception_convert: public CException
{
public:
	exception_convert(const char*   e) : CException(e)	{}
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
    virtual ~exception_variant()						{}
};

//////////////////////////////////////////////////////////////////////////
// exception for 'socket failed'
//////////////////////////////////////////////////////////////////////////
class exception_socket : public CException
{
public:
	exception_socket(const char*   e) : CException(e)	{}
	virtual ~exception_socket()							{}
};












///////////////////////////////////////////////////////////////////////////////
// a simple fixed size buffer
// need size argument at compile time
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TBuffer
{
private:
	T	cField[SZ];
public:
	///////////////////////////////////////////////////////////////////////////
	// access a c style array
	operator T*()	{ return this->cField; }

	///////////////////////////////////////////////////////////////////////////
	// write access to field elements
	T &operator[](size_t inx)
	{
#ifdef CHECK_BOUNDS
		if(inx>SZ)
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TBuffer out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return this->cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// read-only access to field elements
	const T &operator[](size_t inx) const
	{
#ifdef CHECK_BOUNDS
		if(inx>SZ)
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TBuffer out of bound");
#else
			return this->cField[0];
#endif
		}
#endif
		return this->cField[inx];
	}
};


///////////////////////////////////////////////////////////////////////////////
//
// List of Objects / implemented as list of pointers to (single) objects
// it actually owns the pointers and deletes them on exit
//
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDPT
{
	typedef T* pT;

	pT*		cField;
	size_t	cCnt;
	size_t	cSZ;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	TArrayDPT() : cField(NULL),cCnt(0),cSZ(0)			{  }
	TArrayDPT(size_t cnt) : cField(NULL),cCnt(0),cSZ(0)	{ resize(cnt); }
	~TArrayDPT()										{ clear(); }

	///////////////////////////////////////////////////////////////////////////
	// copy/assign
	TArrayDPT(const TArrayDPT& tp) : cField(NULL),cCnt(0),cSZ(0)
	{
		this->copy(tp);
	}

	const TArrayDPT& operator=(const TArrayDPT& tp)
	{
		this->resize(0); 
		this->copy(tp);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// access functions
	void copy(const TArrayDPT& tp)
	{
		if(this!=&tp)
		{
			size_t i;
			realloc(tp.cCnt);
			for(i=0; i<tp.cCnt; i++)
			{
				if(tp.cField[i])
				{	// list element in source list, copy it
					if(!cField[i])
						cField[i] = new T(*tp.cField[i]);
					else
						*cField[i] = *tp.cField[i];
				}
				else
				{	// empty element in source list
					if(cField[i])
					{
						delete cField[i];
						cField[i] = NULL;
					}
				}

			}
			this->cCnt=tp.cCnt;
		}
	}
	void clear()
	{
		if(cField)
		{	
			size_t i;
			for(i=0; i<cSZ; i++)
				if(cField[i]) delete cField[i];
			delete[] cField;
			cField=NULL;
			cCnt=0;
			cSZ=0;
		}
	}
	bool realloc(size_t newsize)
	{
		if(newsize >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(newsize >= sz) sz*=2;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			size_t sz = (cSZ)?cSZ:2;
			while(cCnt+expectaddition >= sz)
				sz+=(growsize)?growsize:sz;
			pT* temp = new pT[sz];
			if(temp)
			{
				memset(temp,0,sz*sizeof(pT));
				if(cField) 
				{
					memcpy(temp,cField,cSZ*sizeof(pT));
					delete[] cField;
				}
				cField = temp;
				cSZ = sz;
			}
		}
		return cField!=NULL;
	}
	virtual bool realloc()
	{
		if(cCnt != cSZ)
		{	
			if(cCnt)
			{
				// need to resize
				cSZ = cCnt;
				pT* newfield = new T[cSZ];
				if(newfield)
				{
					if(cField) 
					{
						memcpy(newfield,cField,cSZ*sizeof(pT));
						delete[] cField;
					}
					cField = newfield;
				}
			}
			else
			{	// just clear all
				if(cField)
				{
					delete[] cField;
					cField=NULL;
					cSZ = 0;
				}
			}
		}
		return cField!=NULL;
	}
	bool resize(size_t cnt)
	{
		if(cnt >= cSZ)
			realloc(cnt);
		cCnt = cnt;
		return NULL!=cField;
	}
	T& operator[](size_t inx)
	{
		// automatic resize on out-of-bound
		if( inx>=cCnt )
		{	
			resize(inx+1);
		}

		if( !cField[inx] )
		{	// create a new element if not exist
			cField[inx] = new T;
		}
		return *cField[inx];
	}
	const T& operator[](size_t inx) const
	{	// throw something here
		if( inx>=cCnt || !cField[inx] )
			throw exception_bound("TPointerList");

		return *cField[inx];
	}

	T& first()
	{
		return this->operator[](0);
	}
	T& last()
	{	
		return this->operator[]((cCnt>0)?(cCnt-1):0);
	}
	size_t size() const			{ return cCnt; }



	bool insert(const T& elem, size_t pos) // insert an element at position pos
	{
		insert(pos);
		if(cField[pos])
			*cField[pos] = elem;
		else
			cField[pos] = new T(elem);
	}
	bool insert(size_t pos) // insert an element at position pos
	{
		if(pos>=cCnt)
		{
			resize(pos+1);
		}
		else
		{
			resize(cCnt+1);
			pT temp =(cField[cCnt]);
			memmove(cField+pos+1,cField+pos,(cCnt-pos)*sizeof(pT));
			cField[pos]=temp;
		}
		return true;
	}
	bool strip(size_t cnt, bool clear=false)
	{
		if(cCnt>=cnt)
		{
			if(clear)
			{	
				while(cnt>0)
				{
					cCnt--;
					if(cField[cCnt])
					{
						delete cField[cCnt];
						cField[cCnt] = NULL;
					}
				}
			}
			else
				cCnt-=cnt;
			return true;
		}
		return false;
	}

	bool removeindex(size_t inx)
	{
		if(inx<cCnt)
		{
			pT temp = cField[inx];
			memmove(cField+inx, cField+inx+1,(cCnt-inx-1)*sizeof(pT));
			cCnt--;
			cField[cCnt] = temp;
		}
		return false;
	}
	bool append(const T& elem)
	{
		realloc(cCnt+1);
		if(cField[cCnt])
			*cField[cCnt] = elem;
		else
			cField[cCnt]=new T(elem);
		cCnt++;
		return true;
	}
	bool append(const T* elem, size_t cnt)
	{
		realloc(cCnt+cnt);
		for(size_t i=0; i<cnt;i++)
		{
			if(cField[cCnt])
				*cField[cCnt] = elem[i];
			else
				cField[cCnt] = new T(elem[i]);
			cCnt++;
		}
		return true;
	}
	bool pop(bool clear=false)
	{
		if(cCnt>0)
		{
			cCnt--;
			if(clear && cField[cCnt])
			{
				delete cField[cCnt];
				cField[cCnt] = NULL;
			}
			return true;
		}
		return false;
	}
	bool push(const T& elem)
	{
		return append(elem);
	}
	bool push(const T* elem, size_t cnt)
	{
		return append(elem, cnt);
	}
};

















///////////////////////////////////////////////////////////////////////////////
// basic interface for arrays
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// basic interface for arrays
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArray : public Mutex, public global
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class String;
	friend class SubString;

protected:
	virtual const T* array() const=0;

protected:
	///////////////////////////////////////////////////////////////////////////
	// copy, move, compare
	virtual void copy(T* tar, const T* src, size_t cnt) = 0;
	virtual void move(T* tar, const T* src, size_t cnt) = 0;
	virtual int  compare(const T& a, const T& b) const = 0;

public:
	virtual bool realloc(size_t newsize) = 0;
	virtual bool realloc(size_t expectaddition, size_t growsize) = 0;
	virtual bool realloc() = 0;

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArray()							{}
	virtual ~TArray()					{}

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer
	virtual const T* getreadbuffer(size_t &maxcnt) const=0;
	virtual T* getwritebuffer(size_t &maxcnt)			=0;

	virtual bool setreadsize(size_t cnt)				=0;
	virtual bool setwritesize(size_t cnt)				=0;

	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)		=0;

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const 	=0;
	virtual T& operator[](size_t inx)		=0;	

	T& first()	{ return this->operator[](0); }
	T& last()	{ return this->operator[]((this->size()>0)?(this->size()-1):0); }


	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			=0;	


	///////////////////////////////////////////////////////////////////////////
	// returns number of elements
	virtual size_t size() const				=0;	
	virtual size_t freesize() const			=0;	


	///////////////////////////////////////////////////////////////////////////
	// push/pop access
	virtual bool push(const T& elem)		{ return append(elem); }
	virtual bool push(const TArray<T>& list){ return append(list); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T& pop()						=0;	
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)				=0;	
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const					=0;	
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const			=0;	


	///////////////////////////////////////////////////////////////////////////
	// add an element at the end
	virtual bool append(const T& elem, size_t cnt=1) =0;	
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt) =0;	

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()					=0;	

	///////////////////////////////////////////////////////////////////////////
	// move an element inside the buffer
	virtual bool move(size_t tarpos, size_t srcpos) = 0;
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0) 		=0;	
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)	=0;	

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)	=0;
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)	=0;

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)	=0;	
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt) 	=0;	


	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const=0;
	virtual int  find(const T& elem, size_t startpos=0) const=0;
};
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// fixed size arrays
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TArrayFST : public TArray<T>
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class String;
	friend class SubString;
	virtual const T* array() const	{return cField;}
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		cField[SZ];	// fixed size array
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int compare(const T& a, const T& b) const
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}
public:
	virtual bool  realloc(size_t newsize) { return false; }
	virtual bool realloc(size_t expectaddition, size_t growsize)	{ return false; }
	virtual bool realloc()	{ return false; }

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayFST() : cCnt(0)	{}
	virtual ~TArrayFST()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayFST(const TArray<T>& arr):cCnt(0)					{ this->copy(arr); }
	const TArrayFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this;}
	TArrayFST(const TArrayFST<T,SZ>& arr):cCnt(0)			{ this->copy(arr); }
	const TArrayFST& operator=(const TArrayFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this;}

	TArrayFST(const T* elem, size_t sz):cCnt(0)				{ this->copy(elem, sz); }

	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer/max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		Mutex::lock();
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		Mutex::unlock();
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		Mutex::lock();
		if( cCnt < SZ )
		{
			maxcnt = SZ-cCnt;
			return const_cast<T*>(cField+cCnt);
		}
		maxcnt = 0;
		Mutex::unlock();
		return NULL;
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt )
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		Mutex::unlock();
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < SZ )
		{
			cCnt += cnt;
			ret = true;
		}
		Mutex::unlock();
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
			Mutex::unlock();
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= SZ )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		ScopeLock scopelock(*this);
		if( cnt < SZ )
		{
			cCnt = cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ ScopeLock scopelock(*this); return cCnt; }	
	virtual size_t freesize() const			{ ScopeLock scopelock(*this); return SZ-cCnt; }


	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from list
	virtual T& pop()
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			cCnt--;
			return cField[cCnt];
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			cCnt--;
			elem = cField[cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// move inside the array
	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		ScopeLock scopelock(*this);
		if(srcpos>cCnt) srcpos=cCnt; 
		if( ( tarpos > srcpos && cCnt+tarpos < SZ+srcpos ) || // enlarge only up to limit
			( tarpos < srcpos                            ) )
		{
			move(cField+tarpos,cField+srcpos,cCnt-srcpos);
			cCnt += tarpos-srcpos;
			return true;
		}	
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		ScopeLock scopelock(*this);
		if(cCnt < SZ)
		{
			while(cnt--) cField[cCnt++] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if( elem && cCnt+cnt < SZ )
		{
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1)
	{
		ScopeLock scopelock(*this);
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		ScopeLock scopelock(*this);
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		ScopeLock scopelock(*this);
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		if( cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos = cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			while(cnt--) cField[pos+cnt] = elem;
			cCnt+=cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		if( elem && cCnt+cnt < SZ )
		{
			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// place cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		ScopeLock scopelock(*this);
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt < SZ )
			{
				copy(cField+pos,elem,cnt);
				cCnt = pos+cnt;
				return true;
			}
		}
		return false;
	}
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{	
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			size_t cnt;
			const T* elem = list.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem && (cCnt+cnt < SZ+poscnt) )
		{
			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		ScopeLock scopelock(*this);
		for(size_t i=startpos; i<cCnt; i++)
		{
			if( elem== cField[i] )
			{	pos = i;
				return true;
			}
		}
		return false;
	}
	virtual int  find(const T& elem, size_t startpos=0) const
	{
		ScopeLock scopelock(*this);
		for(size_t i=startpos; i<cCnt; i++)
		{
			if( elem== cField[i] )
			{	
				return i;
			}
		}
		return -1;
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TfifoFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoFST()	{}
	virtual ~TfifoFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoFST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoFST(const TfifoFST<T,SZ>& arr)					{ this->copy(arr); }
	const TfifoFST& operator=(const TfifoFST<T,SZ>& arr){ this->resize(0); this->copy(arr); return *this; }


	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T& pop()
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			static T elem = this->cField[0];
			this->cCnt--;
			this->move(this->cField+0, this->cField+1, this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return this->cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			this->cCnt--;
			move(this->cField+0, this->cField+1, this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TstackFST : public TArrayFST<T,SZ>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackFST()				{}
	virtual ~TstackFST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackFST(const TArray<T>& arr)							{ this->copy(arr); }
	const TstackFST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TstackFST(const TstackFST<T,SZ>& arr)					{ this->copy(arr); }
	const TstackFST& operator=(const TstackFST<T,SZ>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
// basic interface for sorted lists
///////////////////////////////////////////////////////////////////////////////
template <class T, size_t SZ> class TslistFST : public TfifoFST<T,SZ>
{
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)
	bool cAscending;// sorting order

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistFST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistFST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistFST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)		{ this->copy(arr); }
	const TslistFST& operator=(const TArray<T>& arr)									{ this->resize(0); this->copy(arr); return *this; }
	TslistFST(const TslistFST<T,SZ>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistFST& operator=(const TslistFST<T,SZ>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistFST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)				{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool push(const T& elem) 				{ return insert(elem); }
	virtual bool push(const TArray<T>& list)		{ return insert(list); }
	virtual bool push(const T* elem, size_t cnt)	{ return insert(elem,cnt); }

	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=insert(elem); return ret;}
	virtual bool append(const TArray<T>& list)		{ return insert(list); }
	virtual bool append(const T* elem, size_t cnt)	{ return insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		// ignore position, insert sorted
		if( this->cCnt <SZ )
		{
			bool f = this->find(elem, 0, pos);
			if( !f || cAllowDup )
			{
				move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
				this->cCnt++;
				this->cField[pos] = elem;
			}
			return true;
		}
		// else found
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		for(size_t i=0; i<cnt && this->cCnt<SZ; i++)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			for(size_t i=0; i<list.size() && this->cCnt<SZ; i++)
			{
				this->insert( list[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			this->clear();
			return this->insert(list);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		ScopeLock scopelock(*this);
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		this->removeindex(pos,poscnt);
		return this->insert(list);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		ScopeLock scopelock(*this);
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}
/*
	{	// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1, c;	//smallest first

		pos = 0;
		if( this->cCnt==0 )
		{
			ret = false;
		}
		else if( elem < this->cField[a] )
		{	pos = a;
			ret = false; //less than lower
		}
		else if( elem > this->cField[b] )
		{	pos = b+1;
			ret = false; //larger than upper
		}
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == this->cField[c] )
				{	b=c; // was pos = c;
					ret = true;
					break;
				}
				else if( elem < this->cField[c] )
					b=c;
				else
					a=c;
			}while( (a+1) < b );
			pos = b;//return the next larger element to the given or the found element
		}
		return ret;
	}
*/
	virtual int  find(const T& elem, size_t startpos=0) const
	{
		ScopeLock scopelock(*this);
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}

};
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// fixed size arrays
///////////////////////////////////////////////////////////////////////////////
template <class T> class TArrayDST : public TArray<T>
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class String;
	friend class SubString;
	friend class MiniString;
	virtual const T* array() const	{ return cField; }
protected:
	///////////////////////////////////////////////////////////////////////////
	// data elements
	T		*cField;	// array
	size_t	cSZ;		// allocates array size
	size_t	cCnt;		// used elements

	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		memcpy(tar,src,cnt*sizeof(T));
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		memmove(tar,src,cnt*sizeof(T));
	}
	virtual int  compare(const T& a, const T& b) const	
	{	// dont have a working compare here
		// overload at slist
		return 0;
	}

public:
	virtual bool realloc(size_t newsize)
	{	
		ScopeLock scopelock(*this);

		if(  cSZ < newsize )
		{	// grow rule
			size_t tarsize = newsize;
			newsize = 2;
			while( newsize < tarsize ) newsize *= 2;
		}
		else if( cSZ>8 && cCnt < cSZ/4 && newsize < cSZ/2)
		{	// shrink rule
			newsize = cSZ/2;
		}
		else // no change
			return true;


		T *newfield = new T[newsize];
		if(newfield==NULL)
			throw exception_memory("TArrayDST: memory allocation failed");

		if(cField)
		{
			copy(newfield, cField, cCnt); // between read ptr and write ptr
			delete[] cField;
		}
		cSZ = newsize;
		cField = newfield;
		return cField!=NULL;
	}
	virtual bool realloc(size_t expectaddition, size_t growsize)
	{
		if(cCnt+expectaddition >= cSZ)
		{	// need to enlarge
			if(cSZ==0) cSZ=2;
			while(cCnt+expectaddition >= cSZ)
				cSZ+=(growsize)?growsize:cSZ;
			T* newfield = new T[cSZ];
			if(newfield)
			{
				if(cField) 
				{
					copy(newfield,cField,cCnt);
					delete[] cField;
				}
				cField = newfield;
			}
		}
		return cField!=NULL;
	}
	virtual bool realloc()
	{
		if(cCnt != cSZ)
		{	
			if(cCnt)
			{	// need to resize
				cSZ = cCnt;
				T* newfield = new T[cSZ];
				if(newfield)
				{
					if(cField) 
					{
						copy(newfield,cField,cCnt);
						delete[] cField;
					}
					cField = newfield;
				}
			}
			else
			{	// just clear all
				if(cField)
				{
					delete[] cField;
					cField=NULL;
					cSZ = 0;
				}
			}
		}
		return cField!=NULL;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDST() : cField(NULL),cSZ(0),cCnt(0)	{}
	TArrayDST(size_t sz) : cField(NULL),cSZ(0),cCnt(0)	{ resize(sz); }
	virtual ~TArrayDST()	{ if(cField) delete[] cField; }

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDST(const TArray<T>& arr) : cField(NULL),cSZ(0),cCnt(0)	{ this->copy(arr); }
	const TArrayDST& operator=(const TArray<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }
	TArrayDST(const TArrayDST<T>& arr) : cField(NULL),cSZ(0),cCnt(0){ this->copy(arr); }
	const TArrayDST& operator=(const TArrayDST<T>& arr)				{ this->resize(0); this->copy(arr); return *this; }

	TArrayDST(const T* elem, size_t sz):cField(NULL),cSZ(0),cCnt(0)	{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// put a element to the list
	virtual bool push(const T& elem)			{ return append(elem); }
	virtual bool push(const TArray<T>& list)	{ return append(list); }
	virtual bool push(const T* elem, size_t cnt){ return append(elem,cnt); }
	///////////////////////////////////////////////////////////////////////////
	// return the last element and remove it from list
	virtual T& pop()
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			cCnt--;
			return cField[cCnt];
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			cCnt--;
			elem = cField[cCnt];
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		if( cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
			return const_cast<T&>(cField[0]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		ScopeLock scopelock(*this);
		if( cCnt > 0 )
		{
			elem = cField[0];
			return true;
		}
		return false;
	}


	///////////////////////////////////////////////////////////////////////////
	// direct access to the buffer, return Pointer and max buffer size
	virtual const T* getreadbuffer(size_t &maxcnt) const
	{
		Mutex::lock();
		if( cCnt >0 )
		{
			maxcnt = cCnt;
			// keep the mutex locked
			return const_cast<T*>(cField);
		}
		maxcnt = 0;
		Mutex::unlock();
		return NULL;
	}
	virtual T* getwritebuffer(size_t &maxcnt)
	{
		Mutex::lock();
		if( cCnt+maxcnt > cSZ )
			realloc( maxcnt+cCnt );
		return const_cast<T*>(cField+cCnt);
	}
	virtual bool setreadsize(size_t cnt)
	{
		bool ret = false;
		if( cnt <= cCnt)
		{
			if( cnt >0 )
			{
				move(cField+0, cField+cnt,cCnt-cnt);
				cCnt -= cnt;
			}
			ret = true;
		}
		Mutex::unlock();
		return ret;
	}
	virtual bool setwritesize(size_t cnt)
	{
		bool ret = false;
		if( cCnt+cnt < cSZ )
		{
			cCnt += cnt;
			ret = true;
		}
		Mutex::unlock();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements from list to buf, return number of copied elements
	virtual size_t copytobuffer(T* buf, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if(buf)
		{
			if(cnt>cCnt) cnt = cCnt;
			copy(buf,cField,cnt);
			return cnt;
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	virtual T &operator[](size_t inx)
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( inx >= cCnt )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF out of bound");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return cField[inx];
	}
	///////////////////////////////////////////////////////////////////////////
	// (re)allocates a list of cnt elements [0...cnt-1], 
	// leave new elements uninitialized/default constructed
	virtual bool resize(size_t cnt)			
	{
		ScopeLock scopelock(*this);
		if(cnt > cSZ)
			realloc(cnt);
		cCnt = cnt;
		return NULL!=cField;
	}

	///////////////////////////////////////////////////////////////////////////
	// returns number of used elements
	virtual size_t size() const				{ ScopeLock scopelock(*this); return cCnt; }	
	virtual size_t freesize() const			{ ScopeLock scopelock(*this); return cSZ-cCnt; }	


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T& elem, size_t cnt=1)
	{
		ScopeLock scopelock(*this);
		if(cCnt+cnt > cSZ)
			realloc(cCnt+cnt);
		while(cnt--) cField[cCnt++] = elem;
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const TArray<T>& list)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return append(elem, cnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool append(const T* elem, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if( elem )
		{
			if( cCnt+cnt > cSZ)
				realloc( cCnt+cnt );
			copy(cField+cCnt,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove elements from end of list
	virtual bool strip(size_t cnt=1)
	{
		ScopeLock scopelock(*this);
		if( cnt <= cCnt )
		{
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	virtual bool removeindex(size_t inx)
	{
		ScopeLock scopelock(*this);
		if(inx < cCnt)
		{
			move(cField+inx,cField+inx+1,cCnt-inx-1);
			cCnt--;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove cnt elements starting from inx
	virtual bool removeindex(size_t inx, size_t cnt)
	{
		ScopeLock scopelock(*this);
		if(inx < cCnt)
		{
			if(inx+cnt > cCnt)	cnt = cCnt-inx;
			move(cField+inx,cField+inx+cnt,cCnt-inx-cnt);
			cCnt -= cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		ScopeLock scopelock(*this);
		cCnt = 0;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(size_t pos, size_t cnt=1)
	{
		ScopeLock scopelock(*this);
		if( cCnt+cnt > cSZ )
			realloc(cSZ+cnt);

		if(pos >= cCnt) 
			pos = cCnt;
		else
			move(cField+pos+cnt, cField+pos, cCnt-pos);

		cCnt += cnt;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t cnt=1, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		if( cCnt+cnt > cSZ )
			realloc(cSZ+cnt);

		if(pos >= cCnt) 
			pos = cCnt;
		else
			move(cField+pos+cnt, cField+pos, cCnt-pos);

		cCnt += cnt;
		while(cnt--) cField[pos++] = elem;
		return true;

	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		if( elem )
		{	
			if( cCnt+cnt > cSZ )
				realloc(cCnt+cnt);

			if(pos >= cCnt) 
				pos=cCnt;
			else
				move(cField+pos+cnt, cField+pos, cCnt-pos);
			copy(cField+pos,elem,cnt);
			cCnt += cnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return insert(elem,cnt, pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// copy cnt elements at position pos (at the end by default) overwriting existing elements
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		ScopeLock scopelock(*this);
		if( elem )
		{	
			if(pos > cCnt) pos = cCnt;

			if( pos+cnt > cSZ )
				realloc(pos+cnt);
			copy(cField+pos,elem,cnt);
			cCnt = pos+cnt;
			return true;
		}
		return false;
	}
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{	
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			size_t cnt;
			const T* elem = list.getreadbuffer(cnt);
			return copy(elem,cnt, pos);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// Moving elements inside the buffer
	// always take the elements from 'from' up to 'cElements'

	virtual bool move(size_t tarpos, size_t srcpos)
	{	
		ScopeLock scopelock(*this);
		if(srcpos>cCnt) srcpos=cCnt; 
		if( cCnt+tarpos > cSZ+srcpos )
			realloc(cCnt+tarpos-srcpos);
		move(cField+tarpos,cField+srcpos,cCnt-srcpos);
		cCnt += tarpos-srcpos;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		if(pos > cCnt)
		{
			pos = cCnt;
			poscnt = 0;
		}
		if(pos+poscnt > cCnt) 
		{
			poscnt=cCnt-pos;
		}
		if( elem )
		{
			if( cCnt+cnt > cSZ+poscnt)
				realloc(cCnt+cnt-poscnt);

			move(cField+pos+cnt, cField+pos+poscnt,cCnt-pos-poscnt);
			copy(cField+pos,elem,cnt);
			cCnt += cnt-poscnt;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{	
		ScopeLock scopelock(*this);
		size_t cnt;
		const T* elem = list.getreadbuffer(cnt);
		return replace(elem,cnt, pos, poscnt);
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{
		ScopeLock scopelock(*this);
		for(size_t i=startpos; i<cCnt; i++)
			if( elem == cField[i] )
			{	pos = i;
				return true;
			}
		return false;
	}
	virtual int  find(const T& elem, size_t startpos=0) const
	{
		ScopeLock scopelock(*this);
		for(size_t i=startpos; i<cCnt; i++)
			if( elem == cField[i] )
			{	
				return i;
			}
		return -1;
	}
};
///////////////////////////////////////////////////////////////////////////////
template <class T> class TfifoDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDST()	{}
	virtual ~TfifoDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TfifoDST& operator=(const TArray<T>& arr)		{ this->resize(0); this->copy(arr); return *this; }
	TfifoDST(const TfifoDST<T>& arr)					{ this->copy(arr); }
	const TfifoDST& operator=(const TfifoDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T& pop()
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			static T elem = this->cField[0];
			this->cCnt--;
			this->move(this->cField+0, this->cField+1,this->cCnt);
			return elem;
		}
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
#else
			return this->cField[0];
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			elem = this->cField[0];
			this->cCnt--;
			move(this->cField+0, this->cField+1,this->cCnt);
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
template <class T> class TstackDST : public TArrayDST<T>
{
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDST()				{}
	virtual ~TstackDST()	{}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDST(const TArray<T>& arr)						{ this->copy(arr); }
	const TstackDST& operator=(const TArray<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }
	TstackDST(const TstackDST<T>& arr)					{ this->copy(arr); }
	const TstackDST& operator=(const TstackDST<T>& arr)	{ this->resize(0); this->copy(arr); return *this; }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const
	{
		ScopeLock scopelock(*this);
#ifdef CHECK_BOUNDS
		// check for access to outside memory
		if( this->cCnt == 0 )
		{
#ifdef CHECK_EXCEPTIONS
			throw exception_bound("TArrayF underflow");
#else
			static T dummy;
			return dummy;
#endif
		}
#endif
		return const_cast<T&>(this->cField[this->cCnt]);
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		ScopeLock scopelock(*this);
		if( this->cCnt > 0 )
		{
			elem = this->cField[this->cCnt];
			return true;
		}
		return false;
	}

};
///////////////////////////////////////////////////////////////////////////////
// basic interface for sorted lists
///////////////////////////////////////////////////////////////////////////////
template <class T> class TslistDST : public TfifoDST<T>
{
	bool cAllowDup;	// allow duplicate entries (find might then not find specific elems)
	bool cAscending;// sorting order

	virtual int compare(const T&a, const T&b) const
	{
		if( a>b )		return (cAscending) ?  1:-1;
		else if( a<b )	return (cAscending) ? -1: 1;
		else			return 0;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// destructor
	TslistDST(bool as=true, bool ad=false):cAscending(as),cAllowDup(ad) {}
	virtual ~TslistDST() {}
	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDST(const TArray<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TArray<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }
	TslistDST(const TslistDST<T>& arr,bool ad=false):cAscending(true),cAllowDup(ad)	{ this->copy(arr); }
	const TslistDST& operator=(const TslistDST<T>& arr)								{ this->resize(0); this->copy(arr); return *this; }

	TslistDST(const T* elem, size_t sz):cAscending(true),cAllowDup(false)			{ this->copy(elem, sz); }
	///////////////////////////////////////////////////////////////////////////
	// add an element to the list
	virtual bool push(const T& elem) 				{ return this->insert(elem); }
	virtual bool push(const TArray<T>& list)		{ return this->insert(list); }
	virtual bool push(const T* elem, size_t cnt)	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos at the end by default
	virtual bool append(const T& elem, size_t cnt=1){ bool ret=false; while(cnt--) ret=this->insert(elem); return ret;}
	virtual bool append(const TArray<T>& list) 		{ return this->insert(list); }
	virtual bool append(const T* elem, size_t cnt) 	{ return this->insert(elem,cnt); }


	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T& elem, size_t pos=~0)
	{	// ignore position, insert sorted
		ScopeLock scopelock(*this);
		bool f = find(elem, 0, pos);
		if( !f || cAllowDup )
		{
			if( this->cCnt >= this->cSZ )
				this->realloc(this->cSZ+1);

			this->move(this->cField+pos+1, this->cField+pos, this->cCnt-pos);
			this->cCnt++;
			this->cField[pos] = elem;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add cnt elements at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt, size_t pos=~0)
	{
		ScopeLock scopelock(*this);
		if( this->cCnt+cnt>this->cSZ )
			realloc(this->cCnt+cnt);

		for(size_t i=0; i<cnt; i++)
		{	
			this->insert( elem[i] );
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an list of elements at position pos (at the end by default)
	virtual bool insert(const TArray<T>& list, size_t pos=~0)
	{
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			if( this->cCnt+list.size()>this->cSZ )
				this->realloc(this->cCnt+list.size());

			for(size_t i=0; i<list.size(); i++)
			{
				this->insert( list[i] );
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const TArray<T>& list, size_t pos=0)
	{
		if(this!=&list)
		{
			ScopeLock scopelock(*this);
			this->clear();
			return this->insert(list);
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// copy the given list
	virtual bool copy(const T* elem, size_t cnt, size_t pos=0)
	{
		ScopeLock scopelock(*this);
		this->clear();
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with list
	virtual bool replace(const TArray<T>& list, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		this->removeindex(pos,poscnt);
		return this->insert(list);
	}
	///////////////////////////////////////////////////////////////////////////
	// replace poscnt elements at pos with cnt elements
	virtual bool replace(const T* elem, size_t cnt, size_t pos, size_t poscnt)
	{
		ScopeLock scopelock(*this);
		this->removeindex(pos,poscnt);
		return this->insert(elem, cnt);
	}

	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	virtual bool find(const T& elem, size_t startpos, size_t& pos) const
	{	
		ScopeLock scopelock(*this);
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1;
		size_t c;
		pos = 0;

		if( NULL==this->cField || this->cCnt < 1)
			ret = false;
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else if( cAscending )
		{	//smallest element first
			if( elem < this->cField[a] )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( elem > this->cField[b] )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem < this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( elem > this->cField[a] )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( elem < this->cField[b] )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( elem == this->cField[c] )
					{	b=c;
						ret = true;
						break;
					}
					else if( elem > this->cField[c] )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		return ret;
	}
/*
	{	// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>this->cCnt) ? 0 : startpos;
		size_t b=this->cCnt-1, c;	//smallest first

		pos = 0;
		if( this->cCnt==0 )
		{
			ret = false;
		}
		else if( elem < this->cField[a] )
		{	pos = a;
			ret = false; //less than lower
		}
		else if( elem > this->cField[b] )
		{	pos = b+1;
			ret = false; //larger than upper
		}
		else if( elem == this->cField[a] ) 
		{	pos=a;
			ret = true;
		}
		else if( elem == this->cField[b] )
		{	pos = b;
			ret = true;
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == this->cField[c] )
				{	b=c; // was pos = c;
					ret = true;
					break;
				}
				else if( elem < this->cField[c] )
					b=c;
				else
					a=c;
			}while( (a+1) < b );
			pos = b;//return the next larger element to the given or the found element
		}
		return ret;
	}
*/
	virtual int  find(const T& elem, size_t startpos=0) const
	{
		ScopeLock scopelock(*this);
		size_t pos;
		if( this->find(elem,startpos, pos) )
			return pos;
		return -1;
	}

};
///////////////////////////////////////////////////////////////////////////////


template <class T> class TArrayDCT : public TArrayDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TArrayDCT()				{}
	virtual ~TArrayDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TArrayDCT(const TArray<T>& arr)						{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
	TArrayDCT(const TArrayDCT<T>& arr)					{ TArrayDST<T>::copy(arr); }
	const TArrayDCT& operator=(const TArrayDCT<T>& arr)	{ this->resize(0); TArrayDST<T>::copy(arr); return *this; }
};


template <class T> class TfifoDCT : public TfifoDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TfifoDCT()			{}
	virtual ~TfifoDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TfifoDCT(const TArray<T>& arr)							{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TArray<T>& arr)			{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }
	TfifoDCT(const TfifoDCT<T>& arr)						{ TfifoDST<T>::copy(arr); }
	const TfifoDCT& operator=(const TfifoDCT<T>& arr)		{ this->resize(0); TfifoDST<T>::copy(arr); return *this; }

};
template <class T> class TstackDCT : public TstackDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for simple data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{

		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TstackDCT()				{}
	virtual ~TstackDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TstackDCT(const TArray<T>& arr)						{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }
	TstackDCT(const TstackDCT<T>& arr)					{ TstackDST<T>::copy(arr); }
	const TstackDCT& operator=(const TstackDCT<T>& arr)	{ this->resize(0); TstackDST<T>::copy(arr); return *this; }

};
template <class T> class TslistDCT : public TslistDST<T>
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// copy and move for complex data types
	virtual void copy(T* tar, const T* src, size_t cnt)
	{
		for(size_t i=0; i<cnt; i++)
			tar[i] = src[i];
	}
	virtual void move(T* tar, const T* src, size_t cnt)
	{
		if(tar>src)
		{	// last to first run
			register size_t i=cnt;
			while(i>0)
			{
				i--;
				tar[i] = src[i];
			}
		}
		else if(tar<src)
		{	// first to last run
			register size_t i;
			for(i=0; i<cnt; i++)
				tar[i] = src[i];

		}
		//else identical; no move necessary
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// constructor / destructor
	TslistDCT()				{}
	virtual ~TslistDCT()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy constructor and assign (cannot be derived)
	TslistDCT(const TArray<T>& arr)						{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TArray<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
	TslistDCT(const TslistDCT<T>& arr)					{ TslistDST<T>::copy(arr); }
	const TslistDCT& operator=(const TslistDST<T>& arr)	{ this->resize(0); TslistDST<T>::copy(arr); return *this; }
};


///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using a growing base list and sorted pod lists for storing positions
// base list cannot shrink on delete operations so this is only suitable for static list
// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiList
{

	TArrayDST<T>		cList;
	TArrayDST<size_t>	cIndex[CNT];

public:
	TMultiList()	{}
	~TMultiList()	{}

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t i) const	{ return cList[ cIndex[0][i] ]; }
	T& operator[](size_t i)				{ return cList[ cIndex[0][i] ]; }

	const T& operator()(size_t p,size_t i=0) const	{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }
	T& operator()(size_t p,size_t i=0)				{ return cList[ cIndex[(i<CNT)?i:0][p] ]; }


	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i, pos = cList.size();
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			for(i=0; i<CNT; i++)
			{
				cIndex[i].insert(pos, 1, ipos[i]);
			}
			return cList.append(elem);
		}

		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T& elem = cList[ cIndex[(inx<CNT)?inx:0][pos] ];
		size_t temppos, i;
		pos = cIndex[inx][pos];

		for(i=0; i<CNT; i++)
		{
			if( binsearch(elem, 0, i, temppos, true, &T::compare) )
				cIndex[i].removeindex(temppos);
		}
		return cList.removeindex(pos);
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		size_t i;
		for(i=0; i<CNT; i++)
			cIndex[i].clear();
		return cList.clear();
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][a] ], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)(cList[ cIndex[inx][a] ], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)(cList[ cIndex[inx][b] ], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)(cList[ cIndex[inx][c] ], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};
///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using Pointers to stored objects for internal lists, 
// subsequent insert/delete does new/delete the objects, 
// for performance use a managed memory derived classes
// usable classes need a "int compare(const T& elem, size_t inx) const" member
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListP
{

	TArrayDST<T*>	cIndex[CNT];

public:
	TMultiListP()	{}
	~TMultiListP()	{ clear(); }

	size_t size() const					{ return cIndex[0].size(); }
	const T& operator[](size_t p) const	{ return *cIndex[0][p]; }
	T& operator[](size_t p)				{ return *cIndex[0][p]; }

	const T& operator()(size_t p,size_t i=0) const	{ return *cIndex[(i<CNT)?i:0][p]; }
	T& operator()(size_t p,size_t i=0)				{ return *cIndex[(i<CNT)?i:0][p]; }


	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			T* newelem = new T(elem);
			for(i=0; i<CNT; i++)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}

		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; i++)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				for(i=0; i<CNT; i++)
				{
					ok &= cIndex[i].insert(elem, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		T* elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; i++)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); i++)
			delete cIndex[0][i];
		// clear pointer list
		for(i=0; i<CNT; i++)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};

///////////////////////////////////////////////////////////////////////////////







//////////////////////////////////////////////////////////////////////////
// atomic access functions
// single thread version with removed fastcall
//!! replace with basesync.h
//////////////////////////////////////////////////////////////////////////

inline int atomicincrement(int* target)
{
    return ++(*target);
}

inline int atomicdecrement(int* target)
{
    return --(*target);
}

inline int atomicexchange(int* target, int value)
{
    int r = *target;
    *target = value;
    return r;
}
inline void* _atomicexchange(void** target, void* value)
{
    void* r = *target;
    *target = value;
    return r;
}

inline unsigned int atomicincrement(unsigned int* target)	{return (unsigned int)atomicincrement((int*)target);}
inline unsigned int atomicdecrement(unsigned int* target)	{return (unsigned int)atomicdecrement((int*)target);}

template <class T> inline T* atomicexchange(T** target, T* value)
{	return (T*)_atomicexchange((void**)target, (void*)value); 
}



/////////////////////////////////////////////////////////////////////////////////////////
// basic pointer interface
//!! replace with basesafeprt.h
/////////////////////////////////////////////////////////////////////////////////////////
template <class X> class TPtr : public noncopyable
{
protected:
	TPtr()	{}
public:
	virtual ~TPtr()	{}
	virtual const X* get() const = 0;

	virtual const X& readaccess() const = 0;
	virtual X& writeaccess() = 0;

	virtual bool operator ==(const TPtr& p)	const	{return false;}
	virtual bool operator !=(const TPtr& p)	const	{return true;}
	virtual bool operator ==(void *p) const = 0;
	virtual bool operator !=(void *p) const = 0;
};
/////////////////////////////////////////////////////////////////////////////////////////
// Simple Auto/Copy-Pointer
// it creates a default object on first access if not initializes
// automatic free on object delete
// does not use a counting object
/////////////////////////////////////////////////////////////////////////////////////////
template <class X> class TPtrAuto : public TPtr<X>
{
private:
	X* itsPtr;
	void create()					{ if(!itsPtr) itsPtr = new X; }
	void copy(const TPtr<X>& p)
	{
		if(this->itsPtr)
		{
			delete this->itsPtr;
			this->itsPtr = NULL;
		}
		X* pp=p.get();
		if( pp )
			this->itsPtr = new X(*pp);
	}
public:
	explicit TPtrAuto(X* p = NULL) throw() : itsPtr(p)	{}
	virtual ~TPtrAuto()				{if(this->itsPtr) delete this->itsPtr;}

	TPtrAuto(const TPtr<X>& p) throw() : itsPtr(NULL)		{ this->copy(p); }
	const TPtr<X>& operator=(const TPtr<X>& p)				{ this->copy(p); return *this; }
	TPtrAuto(const TPtrAuto<X>& p) throw() : itsPtr(NULL)	{ this->copy(p); }
	const TPtr<X>& operator=(const TPtrAuto<X>& p)			{ this->copy(p); return *this; }

	virtual const X& readaccess() const			{const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr;}
	virtual X& writeaccess()					{const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr;}
	virtual const X* get() const				{const_cast<TPtrAuto<X>*>(this)->create(); return this->itsPtr;}
	virtual const X& operator*() const throw()	{const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr;}
	virtual const X* operator->() const throw() {const_cast<TPtrAuto<X>*>(this)->create(); return this->itsPtr;}
	X& operator*()	throw()						{const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr;}
	X* operator->()	throw()						{const_cast<TPtrAuto<X>*>(this)->create(); return this->itsPtr;}
	operator const X&() const throw()			{const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr;}


	virtual bool operator ==(void *p) const { return this->itsPtr==p; }
	virtual bool operator !=(void *p) const { return this->itsPtr!=p; }
};


/////////////////////////////////////////////////////////////////////////////////////////
// Count-Pointer
// pointer copies are counted
// when reference counter becomes zero the object is automatically deleted
// if there is no pointer, it will return NULL
/////////////////////////////////////////////////////////////////////////////////////////
template <class X> class TPtrCount : public TPtr<X>
{
protected:
	class CCounter
	{
	public:
		unsigned int	count;
		X*				ptr;

		CCounter(X* p, unsigned c = 1) : ptr(p), count(c) {}
		~CCounter()	{ if(ptr) delete ptr; }

		const CCounter& operator=(X* p)
		{	// take ownership of the given pointer
			if(ptr) delete ptr;
			ptr = p;
			return *this;
		}
		CCounter* aquire()
		{
			atomicincrement( &count );
			return this;
		}
		CCounter* release()
		{
			if( atomicdecrement( &count ) == 0 )
			{
				delete this;
			}
			return NULL;
		}
	}* itsCounter;

	void acquire(const TPtrCount& r) throw()
	{	// check if not already pointing to the same object
		if( this->itsCounter != r.itsCounter ||  NULL==this->itsCounter )
		{	// save the current pointer
			CCounter *old = this->itsCounter;
			// aquite and increment the given pointer
			if( r.itsCounter )
			{
				this->itsCounter = r.itsCounter;
				this->itsCounter->aquire();
			}
			else
			{	// new empty counter to link the pointers
				this->itsCounter = new CCounter(NULL);
				const_cast<TPtrCount&>(r).itsCounter = this->itsCounter;
				this->itsCounter->aquire();
			}
			// release the old thing
			if(old) old->release();
		}
	}
	void release()
	{	// decrement the count, clear the handle
		if(this->itsCounter) itsCounter = itsCounter->release();
	}


public:
	TPtrCount() : itsCounter(NULL)
	{}
	explicit TPtrCount(X* p) : itsCounter(NULL)
	{	// allocate a new counter
		if(p)
			this->itsCounter = new CCounter(p);
	}
	TPtrCount(const TPtrCount& r) : itsCounter(NULL)
	{
		this->acquire(r);
	}
	virtual ~TPtrCount()	
	{
		this->release();
	}
	const TPtrCount& operator=(const TPtrCount& r)
	{
		this->acquire(r);
		return *this;
	}
	TPtrCount& operator=(X* p)
	{	// take ownership of the given pointer
		if( this->itsCounter )
		{	
			make_unique();
			*this->itsCounter = p;
		}
		else
		{
			this->itsCounter = new CCounter(p);
		}
		return *this;
	}

	const size_t getRefCount() const { return (this->itsCounter) ? this->itsCounter->count : 1;}
	bool clear()					{ this->release(); return this->itsCounter==NULL; }
	bool exists() const				{ return NULL!=this->itsCounter && NULL!=this->itsCounter->ptr; }
	bool isunique()	const throw()	{ return (this->itsCounter ? (itsCounter->count == 1):true);}
	bool make_unique()	  throw()
	{
		if( !isunique() )
		{
			CCounter *cnt = new CCounter(NULL);
			// copy the object if exist
			if(this->itsCounter->ptr)
			{
				cnt->ptr = new X(*(this->itsCounter->ptr));
			}
			release();
			itsCounter = cnt;
		}
		return true;
	}
	virtual bool operator ==(const TPtrCount& p) const { return this->itsCounter == p.itsCounter; }
	virtual bool operator !=(const TPtrCount& p) const { return this->itsCounter != p.itsCounter; }
	virtual bool operator ==(void *p) const { return (this->itsCounter) ? (this->itsCounter->ptr==p) : (this->itsCounter==p); }
	virtual bool operator !=(void *p) const { return (this->itsCounter) ? (this->itsCounter->ptr!=p) : (this->itsCounter!=p); }

//	operator bool() const	{ return  itsCounter ?  NULL!=itsCounter->ptr : false; }

	virtual const X& readaccess() const			{ return *this->itsCounter->ptr; }
	virtual X& writeaccess()					{ return *this->itsCounter->ptr; }

	virtual const X* get() const				{ return  this->itsCounter ?  this->itsCounter->ptr : 0; }
	virtual const X& operator*() const throw()	{ return *this->itsCounter->ptr; }
	virtual const X* operator->() const throw()	{ return  this->itsCounter ?  this->itsCounter->ptr : 0; }
	virtual X& operator*() throw()				{ return *this->itsCounter->ptr; }
	virtual X* operator->() throw()				{ return  this->itsCounter ?  this->itsCounter->ptr : 0; }
	virtual operator const X&() const throw()	{ return *this->itsCounter->ptr; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// Count-Pointer
// pointer copies are counted
// when reference counter becomes zero the object is automatically deleted
// creates a default object if not exist
/////////////////////////////////////////////////////////////////////////////////////////
template <class X> class TPtrAutoCount : public TPtrCount<X>
{
	void create()
	{	// check if we have an object to access, create one if not
		if(!this->itsCounter)		this->itsCounter		= new (typename TPtrCount<X>::CCounter)(NULL);
		// usable objects need a default constructor
		if(!this->itsCounter->ptr)	this->itsCounter->ptr	= new X; 
	}

public:
	explicit TPtrAutoCount(X* p = NULL) : TPtrCount<X>(p)		{}
	virtual ~TPtrAutoCount()									{}
	TPtrAutoCount(const TPtrCount<X>& r) : TPtrCount<X>(r)		{}
	const TPtrAutoCount<X>& operator=(const TPtrCount<X>& r)	{ this->acquire(r); return *this; }
	TPtrAutoCount(const TPtrAutoCount<X>& r) : TPtrCount<X>(r)	{}
	const TPtrAutoCount<X>& operator=(const TPtrAutoCount<X>& r){ this->acquire(r); return *this; }

	virtual const X& readaccess() const			{ const_cast<TPtrAutoCount<X>*>(this)->create(); return *this->itsCounter->ptr; }
	virtual X& writeaccess()					{ const_cast<TPtrAutoCount<X>*>(this)->create(); return *this->itsCounter->ptr; }
	virtual const X* get() const				{ const_cast<TPtrAutoCount<X>*>(this)->create(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual const X& operator*() const throw()	{ const_cast<TPtrAutoCount<X>*>(this)->create(); return *this->itsCounter->ptr; }
	virtual const X* operator->() const throw()	{ const_cast<TPtrAutoCount<X>*>(this)->create(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual X& operator*() throw()				{ const_cast<TPtrAutoCount<X>*>(this)->create(); return *this->itsCounter->ptr; }
	virtual X* operator->() throw()				{ const_cast<TPtrAutoCount<X>*>(this)->create(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual operator const X&() const throw()	{ const_cast<TPtrAutoCount<X>*>(this)->create(); return *this->itsCounter->ptr; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// Count-Auto-Pointer with copy-on-write scheme
// pointer copies are counted
// when reference counter becomes zero the object is automatically deleted
// creates a default object if not exist
/////////////////////////////////////////////////////////////////////////////////////////
template <class X> class TPtrAutoRef : public TPtrCount<X>
{
	void create()
	{	// check if we have an object to access, create one if not
		if(!this->itsCounter)		this->itsCounter		= new (typename TPtrCount<X>::CCounter)(NULL);
		// usable objects need a default constructor
		if(!this->itsCounter->ptr)	this->itsCounter->ptr	= new X; 
	}
public:
	explicit TPtrAutoRef(X* p = NULL) : TPtrCount<X>(p)		{}
	virtual ~TPtrAutoRef()									{}
	TPtrAutoRef(const TPtrCount<X>& r) : TPtrCount<X>(r)	{}
	const TPtrAutoRef& operator=(const TPtrCount<X>& r)		{ this->acquire(r); return *this; }
	TPtrAutoRef(const TPtrAutoRef<X>& r) : TPtrCount<X>(r)	{}
	const TPtrAutoRef& operator=(const TPtrAutoRef<X>& r)	{ this->acquire(r); return *this; }

	virtual const X& readaccess() const	
	{ 
		const_cast< TPtrAutoRef* >(this)->create();	
		// no need to aquire, is done on reference creation
		return *this->itsCounter->ptr;
	}
	virtual X& writeaccess()
	{
		(this)->create();
		this->make_unique();
		// no need to aquire, is done on reference creation
		return *this->itsCounter->ptr;
	}
	virtual const X* get() const					{ this->readaccess(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual const X& operator*()	const throw()	{ return this->readaccess(); }
	virtual const X* operator->()	const throw()	{ this->readaccess(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual X& operator*()	throw()					{ return this->writeaccess();}
	virtual X* operator->()	throw()					{ this->writeaccess(); return this->itsCounter ? this->itsCounter->ptr : NULL; }
	virtual operator const X&() const throw()		{ return this->readaccess(); }
};



/////////////////////////////////////////////////////////////////////////////
//
// MiniString
//
/////////////////////////////////////////////////////////////////////////////
class MiniString : public global
{

	TPtrAutoRef< TArrayDST<char> > cStrPtr;

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
				return memcmp(s, cStrPtr->array(), cStrPtr->size());

			if(s.cStrPtr->size()==0 && cStrPtr->size()==0) return 0;
			if(s.cStrPtr->size()==0) return -1;
			return 1;
		}
		return 0;
	}
	int compareTo(const char *c) const 
	{	// compare with memcmp including the end-of-string
		// which is faster than doing a strcmp
		if(c && cStrPtr.exists()) return memcmp(c, cStrPtr->array(), cStrPtr->size());
		if((!c || *c==0) && !cStrPtr.exists()) return 0;
		if((!c || *c==0)) return -1;
		return 1;
	}
public:
	MiniString()						{  }
	MiniString(const char *c)			{ copy(c); }
	MiniString(const char *c, size_t len){ copy(c, len); }
	MiniString(const MiniString &str)	{ cStrPtr = str.cStrPtr; }
	/////////////////////////////////////////////////////////////////
	// a special constructor for creating an addition objects
	/////////////////////////////////////////////////////////////////
	MiniString(const char *c1, const size_t len1, const char *c2, const size_t len2)
	{	// double initialisation to concatenate two strings within the constructor
		// the given len values are only the number of characters without the EOS
		cStrPtr->realloc(len1+len2+1);
		cStrPtr->copy(c1,len1, 0);
		cStrPtr->copy(c2,len2, len1);
		cStrPtr->append(0);
	}

	virtual ~MiniString()				{  }

	const MiniString &operator=(const MiniString &str)
	{
		cStrPtr = str.cStrPtr;
		return *this; 
	}
	const MiniString& operator=(const char *c)	{ copy(c); return *this; }
	const char* get() const						{ return cStrPtr->array(); }
	operator const char*() const				{ return cStrPtr->array(); }

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

	friend bool operator==(const char *a, const MiniString &b) {return (0==b.compareTo(a));}
	friend bool operator!=(const char *a, const MiniString &b) {return (0!=b.compareTo(a));}

	friend int compare(const MiniString &a,const MiniString &b){ return a.compareTo(b); }

	void clear()
	{
		if(cStrPtr.exists())
		{
			cStrPtr->resize(0);
			cStrPtr->append(0);
		}
	}
	bool append(char c)
	{
		if(cStrPtr.exists())
			cStrPtr->strip(1); //strip EOS
		cStrPtr->append(c);
		cStrPtr->append(0);
		return true;
	}
	bool append(const char *c)
	{
		if(c)
		{
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append(c,strlen(c)+1);
		}
		return true;
	}
	bool append(const char *c, size_t len)
	{
		if(c)
		{
			size_t sz = (len) ? min(len,strlen(c)) : (0);
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append(c,sz);
			cStrPtr->append(0);
		}
		return true;
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
	size_t length() const	{ return (cStrPtr.exists() && cStrPtr->size()>0) ? ( cStrPtr->size()-1):0; }



	//////////////////////////////////////////////////////
	// type to string conversions
	MiniString(double v)						{ assign(v); }
	const MiniString &operator=(double v)		{ assign(v); return *this;}

	MiniString(int v)							{ assign(v); }
	const MiniString &operator=(int v)			{ assign(v); return *this;}

	MiniString(unsigned int v)					{ assign(v); }
	const MiniString &operator=(unsigned int v)	{ assign(v); return *this;}

	void assign(double v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%.3lf", v);
		copy(buf, sz);
	}
	void assign(int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%i", v);
		copy(buf, sz);
	}
	void assign(unsigned int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%u", v);
		copy(buf, sz);
	}
	void append(double v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%.3lf", v);
		append(buf, sz);
	}
	void append(int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%i", v);
		append(buf, sz);
	}
	void append(unsigned int v)
	{
		char buf[128];
		size_t sz = snprintf(buf,sizeof(buf), "%u", v);
		append(buf, sz);
	}

	//////////////////////////////////////////////////////
	// string operations
	MiniString& operator+=(const MiniString& str)
	{	// append two strings
		if(str.cStrPtr.exists())
		{
			if(cStrPtr.exists())
				cStrPtr->strip(1); //strip EOS
			cStrPtr->append( *str.cStrPtr );
		}
		return *this;
	}
	MiniString& operator+=(const char* str)
	{	// append two strings
		this->append(str);
		return *this;
	}
	MiniString& operator+=(double v)
	{	// append two strings
		this->append(v);
		return *this;
	}
	MiniString& operator+=(int v)
	{	// append two strings
		this->append(v);
		return *this;
	}
	MiniString& operator+=(unsigned int v)
	{	// append two strings
		this->append(v);
		return *this;
	}

	MiniString operator +(const MiniString &s)
	{
		return MiniString(cStrPtr->array(),length(), s.cStrPtr->array(), s.length());
	}
	MiniString operator +(const char* c)
	{
		if(c)
		{
			return MiniString(cStrPtr->array(),length(), c, strlen(c));
		}
		return *this;
	}
	MiniString operator +(const char ch)
	{
		if(ch)
			return MiniString(cStrPtr->array(),length(), &ch, 1);
		return *this;
	}
	MiniString operator +(double v)
	{
		MiniString s(v);
		return MiniString(cStrPtr->array(),length(), s.cStrPtr->array(), s.length());
	}
	MiniString operator +(int v)
	{
		MiniString s(v);
		return MiniString(cStrPtr->array(),length(), s.cStrPtr->array(), s.length());
	}
	MiniString operator +(unsigned int v)
	{
		MiniString s(v);
		return MiniString(cStrPtr->array(),length(), s.cStrPtr->array(), s.length());
	}

	friend MiniString operator+(const char *c, const MiniString &b)
	{
		return MiniString(c, strlen(c), b.get(),b.length() );
	}
	friend MiniString operator+(const char c, const MiniString &b)
	{
		return MiniString(&c, 1, b.get(),b.length() );
	}
	friend MiniString operator +(double v, const MiniString &b)
	{
		MiniString s(v);
		return MiniString(s.get(), s.length(), b.get(), b.length());
	}
	friend MiniString operator +(int v, const MiniString &b)
	{
		MiniString s(v);
		return MiniString(s.get(), s.length(), b.get(), b.length());
	}
	friend MiniString operator +(unsigned int v, const MiniString &b)
	{
		MiniString s(v);
		return MiniString(s.get(), s.length(), b.get(), b.length());
	}

};


///////////////////////////////////////////////////////////////////////////////
// modifies a value on a scope change (actually on object destruction)
// removes the necessity of writing bunches of value settings before returns
// on spagetti code like frequently found especially here
///////////////////////////////////////////////////////////////////////////////
template <class T> class TScopeChange
{
	T& val;
	T  tar;
public:
	TScopeChange(T& v, const T&t) : val(v), tar(t)	{}
	~TScopeChange()			{ val=tar; }
	void disable()			{ tar = val; }
	void set(const T& t)	{ tar = t; }
};












///////////////////////////////////////////////////////////////////////////////
// Multi-Indexed List Template
// using SavePointers to stored objects for internal lists, 
// subsequent insert/delete does new/delete the objects, 
// for performance use a managed memory derived classes
// usable classes need a "int compare(const T& elem, size_t inx) const" member
//
// needs evaluation
///////////////////////////////////////////////////////////////////////////////
template <class T, int CNT> class TMultiListSP
{
	TArrayDCT< TPtrAutoCount<T> >	cIndex[CNT];
	Mutex							cMx;

public:
	TMultiListSP()	{}
	~TMultiListSP()	{ clear(); }

	size_t size() const												{ return cIndex[0].size(); }
	const TPtrAutoCount<T> operator[](size_t p) const				{ return cIndex[0][p]; }
	TPtrAutoCount<T> operator[](size_t p)							{ return cIndex[0][p]; }

	const TPtrAutoCount<T> operator()(size_t p,size_t i=0) const	{ return cIndex[(i<CNT)?i:0][p]; }
	TPtrAutoCount<T> operator()(size_t p,size_t i=0)				{ return cIndex[(i<CNT)?i:0][p]; }

	///////////////////////////////////////////////////////////////////////////
	// add an element
	virtual bool insert(const T& elem)
	{
		size_t i;
		size_t ipos[CNT];
		bool ok=true;

		for(i=0; i<CNT; i++)
		{
			ok &= !binsearch(elem, 0, i, ipos[i], true, &T::compare);
		}
		if(ok)
		{
			TPtrAutoCount<T> newelem(new T(elem));
			for(i=0; i<CNT; i++)
			{
				ok &= cIndex[i].insert(newelem, 1, ipos[i]);
			}
		}
		return ok;
	}
	///////////////////////////////////////////////////////////////////////////
	// add and take over the element pointer
	virtual bool insert(T* elem)
	{
		bool ok=false;
		if(elem)
		{
			size_t i;
			size_t ipos[CNT];
			ok = true;
			for(i=0; i<CNT; i++)
			{
				ok &= !binsearch(*elem, 0, i, ipos[i], true, &T::compare);
			}
			if(ok)
			{
				TPtrAutoCount<T> xx(elem);
				for(i=0; i<CNT; i++)
				{
					ok &= cIndex[i].insert(xx, 1, ipos[i]);
				}
			}
		}
		return ok;
	}

	///////////////////////////////////////////////////////////////////////////
	// add an element at position pos (at the end by default)
	virtual bool insert(const T* elem, size_t cnt)
	{
		bool ret = false;
		if(elem && cnt)
		{
			size_t i;
			ret = insert(elem[0]);
			for(i=1; i<cnt; i++)
				ret &= insert(elem[i]);
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove element [inx]
	bool removeindex(size_t pos, size_t inx=0)
	{
		TPtrAutoCount<T> elem = cIndex[(inx<CNT)?inx:0][pos];
		size_t temppos, i;
		bool ret = true;

		for(i=0; i<CNT; i++)
		{
			if( binsearch(*elem, 0, i, temppos, true, &T::compare) )
				ret &= cIndex[i].removeindex(temppos);
		}
		// free the removed element
		delete elem;
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	bool clear()
	{
		bool ret = true;
		size_t i;
		// free elements
		for(i=0; i<cIndex[0].size(); i++)
			cIndex[0][i].clear();
		// clear pointer list
		for(i=0; i<CNT; i++)
			ret &= cIndex[i].clear();
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// find an element in the list
	bool find(const T& elem, size_t& pos, size_t inx=0) const
	{
		return binsearch(elem, 0, (inx<CNT)?inx:0, pos, true, &T::compare);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// binsearch
	// with member function parameter might be not necesary in this case
	bool binsearch(const T& elem, size_t startpos, size_t inx, size_t& pos, bool asc, int (T::*cmp)(const T&, size_t) const) const
	{	
		if (inx>=CNT) inx=0;
		// do a binary search
		// make some initial stuff
		bool ret = false;
		size_t a= (startpos>=cIndex[inx].size()) ? 0 : startpos;
		size_t b=cIndex[inx].size()-1;
		size_t c;
		pos = 0;

		if( cIndex[inx].size() < 1)
			ret = false;
		else if( 0 == (elem.*cmp)( *cIndex[inx][a], inx) ) 
		{	pos=a;
			ret = true;
		}
		else if( 0 == (elem.*cmp)( *cIndex[inx][b], inx) )
		{	pos = b;
			ret = true;
		}
		else if( asc )
		{	//smallest element first
			if( 0 > (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //larger than lower
			}
			else if( 0 < (elem.*cmp)( *cIndex[inx][b], inx) )	// v1
			{	pos = b+1;
				ret = false; //less than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 > (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next smaller element to the given or the found element
			}
		}
		else // descending
		{	//smallest element last
			if( 0 < (elem.*cmp)( *cIndex[inx][a], inx) )
			{	pos = a;
				ret = false; //less than lower
			}
			else if( 0 > (elem.*cmp)( *cIndex[inx][b], inx) )
			{	pos = b+1;
				ret = false; //larger than upper
			}
			else
			{	// binary search
				do
				{
					c=(a+b)/2;
					if( 0 == (elem.*cmp)( *cIndex[inx][c], inx) )
					{	b=c;
						ret = true;
						break;
					}
					else if( 0 < (elem.*cmp)( *cIndex[inx][c], inx) )
						b=c;
					else
						a=c;
				}while( (a+1) < b );
				pos = b;//return the next larger element to the given or the found element
			}
		}
		return ret;
	}

};

///////////////////////////////////////////////////////////////////////////////



#endif//_BASE_H_

