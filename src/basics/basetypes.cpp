
#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"		// for mutex


NAMESPACE_BEGIN(basics)

//////////////////////////////////////////////////////////////////////////
// atomic access functions
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// single-threaded versions, all systems
//////////////////////////////////////////////////////////////////////////
#ifdef SINGLETHREAD
//////////////////////////////////////////////////////////////////////////
int FASTCALL atomicexchange(int* target, int value)
{
	int r = *target;
	*target = value;
	return r;
}

void* FASTCALL _atomicexchange(void** target, void* value)
{
	void* r = *target;
	*target = value;
	return r;
}

int FASTCALL atomicincrement(int* target)
{
	return ++(*target);
}

int FASTCALL atomicdecrement(int* target)
{
	return --(*target);
}



//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
// multi-threaded version
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// no posix
//////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__) && (defined(__i386__) || defined(__I386__))
#  define GCC_i386
#elif defined(__GNUC__) && defined(__ppc__)
#  define GCC_PPC
#elif defined(_MSC_VER) && defined(_M_IX86)
#  define MSC_i386
#elif defined(__BORLANDC__) && defined(_M_IX86)
#  define BCC_i386
#endif

//////////////////////////////////////////////////////////////////////////
// atomic operations for Microsoft C and Borland C on Windows
//////////////////////////////////////////////////////////////////////////
#if defined(MSC_i386) || defined(BCC_i386)
//////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#  pragma warning (disable: 4035)	// 'function': no return value
#elif defined(__BORLANDC__)
#  pragma warn -rvl
#endif

//////////////////////////////////////////////////////////////////////////
// !!! NOTE
// the following functions implement atomic exchange/inc/dec on windows. 
// they are dangerous in that they rely on the calling conventions 
// of __fastcall on MSVC and BCC. 
// the first one passes the first two arguments in ECX and EDX, 
// and the second one - in EAX and EDX.
//////////////////////////////////////////////////////////////////////////

int FASTCALL atomicincrement(int*)
{
	__asm
	{
#ifdef BCC_i386
		mov          ecx,eax
#endif
		mov          eax,1;
		lock xadd   [ecx],eax;
		inc          eax
	}
}

int FASTCALL atomicdecrement(int*)
{
	__asm
	{
#ifdef BCC_i386
		mov          ecx,eax
#endif
		mov          eax,-1;
		lock xadd   [ecx],eax;
		dec          eax
	}
}

int FASTCALL atomicexchange(int*, int)
{
	__asm
	{
#ifdef BCC_i386
		xchg        eax,edx;
		lock xchg   eax,[edx];
#else
		mov         eax,edx;
		lock xchg   eax,[ecx];
#endif
	}
}

void* FASTCALL _atomicexchange(void**, void*)
{
	__asm
	{
#ifdef BCC_i386
		xchg        eax,edx;
		lock xchg   eax,[edx];
#else
		mov         eax,edx;
		lock xchg   eax,[ecx];
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
// GNU C compiler on any i386 platform (actually 486+ for xadd)
//////////////////////////////////////////////////////////////////////////
#elif defined(GCC_i386)
//////////////////////////////////////////////////////////////////////////

int FASTCALL atomicexchange(int* target, int value)
{
	__asm__ __volatile ("lock ; xchgl (%1),%0" : "+r" (value) : "r" (target));
	return value;
}

void* FASTCALL _atomicexchange(void** target, void* value)
{
	__asm__ __volatile ("lock ; xchgl (%1),%0" : "+r" (value) : "r" (target));
	return value;
}

int FASTCALL atomicincrement(int* target)
{
	int temp = 1;
	__asm__ __volatile ("lock ; xaddl %0,(%1)" : "+r" (temp) : "r" (target));
	return temp + 1;
}

int FASTCALL atomicdecrement(int* target)
{
	int temp = -1;
	__asm__ __volatile ("lock ; xaddl %0,(%1)" : "+r" (temp) : "r" (target));
	return temp - 1;
}

//////////////////////////////////////////////////////////////////////////
// GNU C compiler on any PPC platform
//////////////////////////////////////////////////////////////////////////
#elif defined(GCC_PPC)
//////////////////////////////////////////////////////////////////////////

int FASTCALL atomicexchange(int* target, int value)
{
	int temp;
	__asm__ __volatile
	(
		"1: lwarx  %0,0,%1\n\
		stwcx. %2,0,%1\n\
		bne-   1b\n\
		isync"
		: "=&r" (temp)
		: "r" (target), "r" (value)
		: "cc", "memory"
	)
	return temp;
}


void* FASTCALL _atomicexchange(void** target, void* value)
{
	void* temp;
	__asm__ __volatile
	(
		"1: lwarx  %0,0,%1\n\
		stwcx. %2,0,%1\n\
		bne-   1b\n\
		isync"
		: "=&r" (temp)
		: "r" (target), "r" (value)
		: "cc", "memory"
	)
	return temp;
}


int FASTCALL atomicincrement(int* target)
{
	int temp;
	__asm__ __volatile
	(
		"1: lwarx  %0,0,%1\n\
		addic  %0,%0,1\n\
		stwcx. %0,0,%1\n\
		bne-   1b\n\
		isync"
		: "=&r" (temp)
		: "r" (target)
		: "cc", "memory"
	)
	return temp;
}


int FASTCALL atomicdecrement(int* target)
{
	int temp;
	__asm__ __volatile
	(
		"1: lwarx  %0,0,%1\n\
		addic  %0,%0,-1\n\
		stwcx. %0,0,%1\n\
		bne-   1b\n\
		isync"
		: "=&r" (temp)
		: "r" (target)
		: "cc", "memory"
	)
	return temp;
}

//////////////////////////////////////////////////////////////////////////
// other platforms have to use mutex locking
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////


int FASTCALL atomicexchange(int* target, int value)
{
	static Mutex m;
	ScopeLock sl(m);
	int r = *target;
	*target = value;
	return r;
}

void* FASTCALL _atomicexchange(void** target, void* value)
{
	static Mutex m;
	ScopeLock sl(m);
	void* r = *target;
	*target = value;
	return r;
}

int FASTCALL atomicincrement(int* target)
{
	static Mutex m;
	ScopeLock sl(m);
	int r = ++(*target);
	return r;
}

int FASTCALL atomicdecrement(int* target)
{
	static Mutex m;
	ScopeLock sl(m);
	int r = --(*target);
	return r;
}

#endif // plattform

#endif// !SINGLETHREAD




#if defined(SINGLETHREAD)

size_t FASTCALL atomiccompareexchange(size_t*target, size_t value, size_t comperand)
{
	int prev = *target;
	if(*target==comperand)
		*target = value;
	return prev;
}
void* FASTCALL _atomiccompareexchange(void** target, void* value, void* comperand)
{
	void* prev = *target;
	if(*target==comperand)
		*target = value;
	return prev;
}

#elif defined(WIN32)

size_t FASTCALL atomiccompareexchange(size_t* target, size_t value, size_t comperand)
{
	return (int)InterlockedCompareExchange((void**)target, (void*)value, (void*)comperand);
}
void* FASTCALL _atomiccompareexchange(void** target, void* value, void* comperand)
{
	return InterlockedCompareExchange(target, value, comperand);
}
//## TODO: add inline asm's
#else

size_t FASTCALL atomiccompareexchange(size_t*target, size_t value, size_t comperand)
{
	static Mutex m;
	ScopeLock sl(m);
	int prev = *target;
	if(*target==comperand)
		*target = value;
	return prev;
}
void* FASTCALL _atomiccompareexchange(void** target, void* value, void* comperand)
{
	static Mutex m;
	ScopeLock sl(m);
	void* prev = *target;
	if(*target==comperand)
		*target = value;
	return prev;
)

#endif




NAMESPACE_END(basics)
