#ifndef __BASETIME_H__
#define __BASETIME_H__

///////////////////////////////////////////////////////////////////////////////
/// time handler.
/// introduces datatime object and access functions
///////////////////////////////////////////////////////////////////////////////
#include "basetypes.h"


///////////////////////////////////////////////////////////////////////////////
/// missing wchar time defines (ie. cygwin)
#if defined(__CYGWIN__) && !defined(_WTIME_DEFINED)

// have a rudimentary definition
inline wchar_t * __cdecl _wasctime(const struct tm *)	{ return NULL; }
inline wchar_t * __cdecl _wctime(const time_t *)		{ return NULL; }
inline size_t __cdecl wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *)	{ return 0; }
inline wchar_t * __cdecl _wstrdate(wchar_t *)		{ return NULL; }
inline wchar_t * __cdecl _wstrtime(wchar_t *)		{ return NULL; }

#define _WTIME_DEFINED
#define _WTIME_DEFINED_WARNING
#endif



///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
///////////////////////////////////////////////////////////////////////////////
/// missing gettimeofday on windows
extern inline int gettimeofday(struct timeval *timenow, void* /*tz*/)
{
	if (timenow)
	{
		FILETIME	ft;
		GetSystemTimeAsFileTime(&ft);
		// while the return value of GetSystemTimeAsFileTime is precise to 100ns
		// it is only updated at each timer tick - approximately 64 times per second 
		// on WinXP, or once every 15.625 milliseconds.
		// so use the returned time value with care
#if !(defined __64BIT__)	// not a naive 64bit platform
		///////////////////////////////////////////////////////////////////////
		// Apparently Win32 has units of 1e-7 sec (100-nanosecond intervals)
		// 4294967296 is 2^32, to shift high word over
		// 11644473600 is the number of seconds between
		// the Win32 epoch 1601-Jan-01 and the Unix epoch 1970-Jan-01
		// Tests found floating point to be 10x faster than 64bit int math.
		double timed = ((ft.dwHighDateTime * 4294967296e-7) - 11644473600.0) + (ft.dwLowDateTime  * 1e-7);
		
		timenow->tv_sec  = (long) timed;
		timenow->tv_usec = (long) ((timed - timenow->tv_sec) * 1e6);
#else
		///////////////////////////////////////////////////////////////////////
		// and the same with 64bit math
		// which might be faster on a real 64bit platform
		LARGE_INTEGER   li;
		__int64		t;
		static const __i64 EPOCHFILETIME = (116444736000000000i64)
		li.LowPart  = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		t  = li.QuadPart;	// time in 100-nanosecond intervals
		t -= EPOCHFILETIME;	// offset to the epoch time
		t /= 10;				// time in microseconds

		timenow->tv_sec  = (long)(t / 1000000);
		timenow->tv_usec = (long)(t % 1000000);
#endif
	}
	///////////////////////////////////////////////////////////////////////////
	/*
	void*tz should be struct timezone *tz
		with
	struct timezone {
		int	tz_minuteswest; // minutes W of Greenwich
		int	tz_dsttime;	// type of dst correction
	};
	but has never been used, because the daylight saving 
	could not be defined by an algorithm

	the code to actually use this structures would be:
	if (tz)
	{
		static int	tzflag=0;
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
	///////////////////////////////////////////////////////////////////////////

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////
/// missing TickCount on Unix.
/// return millisecond precision
extern inline unsigned long GetTickCount()
{
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec * 1000 + tval.tv_usec / 1000;
}
#endif
///////////////////////////////////////////////////////////////////////////////












NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// datetime type.
/// 64-bit int defined as number of milliseconds since midnight 01/01/0001
/// derived from PTypes (C++ Portable Types Library)
///////////////////////////////////////////////////////////////////////////////
typedef int64 datetime;

#define invdatetime LLCONST(-1)

#define _msecsmax 86400000						///< number of milliseconds of one day
#define _daysmax  3652059						///< number of days between 01/01/0001 and 12/31/9999
#define _datetimemax LLCONST(315537897600000)	///< max. allowed number for datetime type
#define _unixepoch LLCONST(62135596800000)		///< difference between time_t and datetime in milliseconds



///////////////////////////////////////////////////////////////////////////////
/// datetime general utilities
///////////////////////////////////////////////////////////////////////////////
inline int days(datetime d)			{ return int(d / _msecsmax); }
inline int msecs(datetime d)		{ return int(d % _msecsmax); }

datetime makedt(int days, int msecs);
bool isvalid(datetime);
datetime now(bool utc = true);
void tzupdate();
int	tzoffset();
datetime utodatetime(time_t u);
struct tm* dttotm(datetime dt, struct tm& t);

///////////////////////////////////////////////////////////////////////////////
/// date/calendar manipulation
///////////////////////////////////////////////////////////////////////////////
bool isleapyear(int year);
int	daysinmonth(int year, int month);
int	daysinyear(int year, int month);
int	dayofweek(datetime);
bool isdatevalid(int year, int month, int day);
datetime encodedate(int year, int month, int day);
bool decodedate(datetime, int& year, int& month, int& day);

///////////////////////////////////////////////////////////////////////////////
/// time manipulation
///////////////////////////////////////////////////////////////////////////////
bool istimevalid(int hour, int min, int sec, int msec = 0);
datetime encodetime(int hour, int min, int sec, int msec = 0);
bool decodetime(datetime, int& hour, int& min, int& sec, int& msec);
bool decodetime(datetime, int& hour, int& min, int& sec);


datetime encodedate(int year, int month, int day, int hour, int min, int sec, int msec = 0);


///////////////////////////////////////////////////////////////////////////////
/// clocks per tick. not very relient esp. on windows
static inline unsigned long clocks_per_sec()
{
#if defined(WIN32)
	return CLOCKS_PER_SEC;
#else
	return sysconf(_SC_CLK_TCK);
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// directly read the processor clock register (using RDTSC command).
/// combined with known processor clock it would allow timers in nanosecond scale
/// unfortunately the later is unlikely to get in necessary precision
/// and might even vary at runtime (dynamic clock scaling)
inline uint64 rdtsc(void)
{
///////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)			// Visual C++
	unsigned long upper, lower;
	__asm
	{
		rdtsc
		mov [lower],eax
		mov [upper],edx
	}
	return (((uint64)upper)<<32) | lower;
///////////////////////////////////////////////////////////////////////////////
#elif defined(__BORLANDC__)		// Borland
	unsigned long upper, lower;
	asm
	{
		db 0x0F,0x31 // instruction RDTSC
		mov [lower],eax
		mov [upper],edx
	}
	return (((uint64)upper)<<32) | lower;
///////////////////////////////////////////////////////////////////////////////
#elif defined(__alpha__)
	uint64 ret;
	__asm__ __volatile__("rpcc %0" : "=r" (ret) : : "memory")
	return ret;
///////////////////////////////////////////////////////////////////////////////
#elif defined(__sun__)
	return (uint64)gethrtime();	// has a nanocecond scale timer
///////////////////////////////////////////////////////////////////////////////
#elif defined(__powerpc__)
	unsigned long upper, lower, tmp;
	__asm__ volatile(
		"loop:				\n"
		"\tmftbu   %0		\n"
		"\tmftb	%1		\n"
		"\tmftbu   %2		\n"
		"\tcmpw	%2,%0		\n"
		"\tbne	loop		\n"
		: "=r"(upper),"=r"(lower),"=r"(tmp)
		);
	return (((uint64)upper)<<32) | lower;
///////////////////////////////////////////////////////////////////////////////
#elif defined(__i386__) || defined(__x86_64__) || defined(linux)
	uint64 x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
//	__asm__ __volatile__ ("rdtsc" : "=A" (x) : : "memory")
	return x;
///////////////////////////////////////////////////////////////////////////////
#else
# error your architecture is not recognized for a valid RDTSC instruction.
#endif
///////////////////////////////////////////////////////////////////////////////
}

NAMESPACE_END(basics)









#endif//__BASETIME_H__
