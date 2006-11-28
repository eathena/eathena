#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"

#include "basesync.h"
#include "basearray.h"





/* from gcc include/c++/3.4.2/new

 ** These are replaceable signatures:
 *  - normal single new and delete (no arguments, throw @c bad_alloc on error)
 *  - normal array new and delete (same)
 *  - @c nothrow single new and delete (take a @c nothrow argument, return
 *    @c NULL on error)
 *  - @c nothrow array new and delete (same)
 *
 *  Placement new and delete signatures (take a memory address argument,
 *  does nothing) may not be replaced by a user's program.

  nice, but how to check if the non-overloadable stuff exists
  and doing this plattform independend?
  and what if I nevertheless want to overload?
  answer is: not possible and fish around in a bunch of defines

  and "replaceable" only means "replace them with the same declaration"
  but what when I want to throw a different exception
  answer is: "you cannot since the throw definition is part of the declaration"

  so what do I need this if I cannot change it
 ...crap...
*/

// on windows you can (and must) overload and it must be in global namespace
// so we put another define in the header since this here is a temporary workaround
#if !defined(WIN32)	
// these are the scope declarations of incluse/c++/<new> from solaris and two different linux distros, 
// I bet there are more of these, but how was the meaning of "standardisation? everybody makes his own?
#if !defined(_NEW) && !defined(_NEW_)  && !defined(__NEW__) 
///////////////////////////////////////////////////////////////////////////////
// global new/delete operators
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#if defined(WITH_NAMESPACE)
///////////////////////////////////////////////////////////////////////////////
/// standard new operator
extern inline void *operator new (size_t sz) THROW(basics::exception_memory)
{
	void* p=malloc(sz);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw basics::exception_memory("allocation failed");
#endif
	return p;
}
/// standard new[] operator
extern inline void *operator new[] (size_t sz) THROW(basics::exception_memory)
{
	void* p=malloc(sz);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw basics::exception_memory("allocation failed");
#endif
	return p;
}
/// standard delete operator
extern inline void operator delete (void * a) NOTHROW()
{
	if(a) free(a);
}
/// standard delete[] operator
extern inline void operator delete[] (void *a) NOTHROW()
{
	if(a) free(a);
}
/// inplace-new operator
extern inline void *operator new (size_t sz,void* p) THROW(basics::exception_memory)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw basics::exception_memory("allocation failed");
#endif
	return p;
}

/// inplace-new operator
extern inline void *operator new[] (size_t sz, void* p) THROW(basics::exception_memory)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw basics::exception_memory("allocation failed");
#endif
	return p;
}
/// inplace-delete operator
extern inline void operator delete (void * a, void* p) NOTHROW()
{
}
/// inplace-delete operator
extern inline void operator delete[] (void *a, void* p) NOTHROW()
{
}

///////////////////////////////////////////////////////////////////////////////
#else//!defined(WITH_NAMESPACE)
///////////////////////////////////////////////////////////////////////////////
/// standard new operator
extern inline void *operator new (size_t sz) THROW(exception_memory)
{
	void* p=malloc(sz);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}
/// standard new[] operator
extern inline void *operator new[] (size_t sz) THROW(exception_memory)
{
	void* p=malloc(sz);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}
/// standard delete operator
extern inline void operator delete (void * a) NOTHROW()
{
	if(a) free(a);
}
/// standard delete[] operator
extern inline void operator delete[] (void *a) NOTHROW()
{
	if(a) free(a);
}
/// inplace-new operator
extern inline void *operator new (size_t sz,void* p) THROW(exception_memory)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}


/// inplace-new operator
extern inline void *operator new[] (size_t sz, void* p) THROW(exception_memory)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}

/// inplace-delete operator
extern inline void operator delete (void * a, void* p) NOTHROW()
{
}
/// inplace-delete operator
extern inline void operator delete[] (void *a, void* p) NOTHROW()
{
}
///////////////////////////////////////////////////////////////////////////////
#endif//defined(WITH_NAMESPACE)
///////////////////////////////////////////////////////////////////////////////
#endif// !defined _NEW or _NEW_
#endif// !defined(WIN32)


NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
// dynamic reallocation policy for c-style allocation
//////////////////////////////////////////////////////////////////////////
void* memalloc(uint a) 
{
	if (a == 0)
		return NULL;
	else
	{
		void* p = malloc(a);
		if (p == NULL)
			throw exception_memory("Not enough memory");
		return p;
	}
}
void* memrealloc(void* p, uint a) 
{
	if (a == 0)
	{
		memfree(p);
		return NULL;
	}
	else if (p == NULL)
		return memalloc(a);
	else
	{
		p = realloc(p, a);
		if (p == NULL)
			throw exception_memory("Not enough memory");
		return p;
	}
}
void memfree(void* p)
{
	if (p != NULL)
		free(p);
}



const _allocatorbase::size_type _allocatorbase::npos = (_allocatorbase::size_type)-1;



//## TODO copy back c++ memory handler












#if defined(DEBUG)


// msvc does not create seperate functions with empty parameter list
template <class T> int test_memcopy(T dummy)	
{
	T*buf = new T[8388608],
		*t=buf, *s=buf+4194304,
		*target,*source;

	size_t i, cnt=4194304;
	ulong tick;
	static int sz;
	
	memset(buf,0,sizeof(*buf)*8388608);



	tick=clock();
	for(i=0; i<100;++i)
	{	
		target=t, source=s;
		// simple type mover, no checks performed
		memmove(target, source, cnt*sizeof(*target));
	}
	printf("memmove: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;++i)
	{
		target=t, source=s;
		// simple type copy, no checks performed
		memcpy(target, source, cnt*sizeof(*target));
	}
	printf("memcpy: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;++i)
	{
		target=t, source=s;
		// simple type compare, no checks performed
		sz = memcmp(target,source,cnt*sizeof(*target));
	}
	printf("memcmp: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;++i)
	{	
		target=t, source=s;
		if(target>source)
		{	// last to first run
			T* epp=target;
			target+=cnt-1;
			source+=cnt-1;
			while( target>=epp ) *target-- = *source--;
		}
		else if(target<source)
		{	// first to last run
			T* epp=target+cnt;
			while( target< epp ) *target++ = *source++;
		}
		//else identical; no move necessary
	}
	printf("move: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;++i)
	{	
		target=t, source=s;
		T* epp=target+cnt;
		while( target<epp ) *target++ = *source++;
	}
	printf("cpy: %lu\n", clock()-tick);

	tick=clock();

	for(i=0; i<100;++i)
	{	
		target=t, source=s;
		const T* epp=target+cnt;
		for( ; target<epp ; ++target,++source)
		{
			if( target==source || *target==*source )
				continue;
			else if( *target < *source )
			{
				sz = -1;
				break;
			}
			else
			{
				sz = 1;
				break;
			}
		}
	}
	printf("cmp: %lu\n", clock()-tick);

	return 0;
}



int test_memswap()
{
	uchar array[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

	int tpos=11, spos=1, cnt=3;

	if( cnt && ( tpos<spos || tpos>spos+cnt) )
	{
		size_t i;
		uchar tempobj;
		uchar *tptr, *sptr, *kptr, *mptr, *tmp;

		if(tpos<spos)
		{	// downmove	
			tptr = array+tpos;
			sptr = array+spos;
			i    = cnt;
		}
		else
		{	// upmove
			// map it to a downmove				
			tptr = array+spos;
			sptr = array+spos+cnt;
			i = cnt = tpos-spos-cnt+1;
		}

int cpcnt=0;

		tmp = sptr;						// start with first source element
		tempobj = *tmp;					// save the initial element
cpcnt++;
		kptr=tmp;						// set run pointer
		while(i--)
		{							
			mptr = kptr-cnt;			// the target position is cnt elements before
			while(mptr>=tptr)			// go through the length until falling off range
			{
				*kptr=*mptr;			// move elements up
cpcnt++;
				kptr = mptr;			// set the next insert pointer
				mptr-=cnt;				// go to the new element
			}
			// fallen out of the array
			mptr = sptr + (kptr-tptr);	// next insertion element has to come from:
			if(mptr==tmp)				// if we hit the pos where the temp elements belongs
			{
				*kptr = tempobj;		// insert the temp element
cpcnt++;
				tmp++;					// increment
				tempobj = *tmp;			// get the new temp
cpcnt++;
				kptr=tmp;				// set run pointer
			}
			else						// otherwise
			{
				*kptr = *mptr;			// move the selected element down
cpcnt++;
				kptr = mptr;			// set run pointer
			}
		}

		printf("copies for (%lu->%lu)x%lu = %i", (ulong)spos, (ulong)tpos, (ulong)cnt, cpcnt);

	}
	return 0;
}
#endif//DEBUG






static bool printcreate=false;
static int sscount=0;
const int sslimit=100000000;


class simple_class : public defaultcmp, public global
{
public:
	int data;
	int data1;
	int data2;
	int data3;
	int data4;
	int data5;
	int data6;
	int data7;

	void init()
	{
		data1 = rand();
		data2 = rand();
		data3 = rand();
		data4 = rand();
		data5 = rand();
		data6 = rand();
		data7 = rand();
	}

	simple_class() : data(0x1234)
	{
		init();
		if(sscount==sslimit)
			throw "error";
		++sscount;
		if(printcreate) printf("d");
	}
	simple_class(const simple_class& c) : data(c.data)
	{
		init();
		if(sscount==sslimit)
			throw "error";
		++sscount;
		if(printcreate) printf("c");
	}
	const simple_class& operator=(const simple_class& c)
	{
		data  = c.data;
		data1 = c.data1;
		data2 = c.data2;
		data3 = c.data3;
		data4 = c.data4;
		data5 = c.data5;
		data6 = c.data6;
		data7 = c.data7;
		if(printcreate) printf("a");
		return *this;
	}
	simple_class(int v) : data(v)
	{
		init();
		if(sscount==sslimit)
			throw "error";
		++sscount;
		if(printcreate) printf("i");
	}
	~simple_class()
	{
		--sscount;
		data = 0xABCD; 
		if(printcreate) printf("x");
	}
};






void test_memory(void)
{
#if defined(DEBUG)

//	printf("copy chars\n");
//	test_memcopy<char>(1);
//	printf("copy shorts\n");
//	test_memcopy<short>(1);
//	printf("copy longs\n");
//	test_memcopy<long>(1);
//
//	printf("test memswap\n");
//	test_memswap();


#endif//DEBUG

	{
		printcreate = true;
		{
			printf("\nTArray\n");
			TArrayDCT<simple_class> svc;
			svc.append(1);printf("-");
			svc.append(2);printf("-");
			svc.append(3);printf("-");
			svc.append(4);printf("-");
		}		
		{
			printf("\nvector\n");
			vector<simple_class> svc;
			svc.append(1);printf("-");
			svc.append(2);printf("-");
			svc.append(3);printf("-");
			svc.append(4);printf("-");
		}
		printf("\n");
	}

	{
		printcreate = false;
		printf("TArray vs. vector\n");
		size_t runs=100, elems=10000;
		size_t i,k;
		ulong tick;

		TArrayDCT<simple_class> arr;
		vector<simple_class> vec;
/*
		TArrayDCT	<int> arr;
		vector		<int> vec;
*/

		printf("append:\n");

		tick = clock();
		for(k=0; k<runs;k++, arr.clear() )
		for(i=0; i<elems; ++i)
		{
			arr.append( (uchar)i );
			//arr.insert( (uchar)i, 1, 0 );
		}
		printf("tarray %lu  (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);


		tick = clock();
		for(k=0; k<runs;k++, vec.resize(0) )
		for(i=0; i<elems; ++i)
		{			
			vec.append( (uchar)i );
			//vec.insert( (uchar)i, 1, 0 );
		}
		printf("vector: %lu (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);


		printf("insert:\n");
		runs=100, elems=1000;

		tick = clock();
		for(k=0; k<runs;k++, arr.clear() )
		for(i=0; i<elems; ++i)
		{
			//arr.append( (uchar)i );
			arr.insert( (uchar)i, 1, 0 );
		}
		printf("tarray %lu  (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);


		tick = clock();
		for(k=0; k<runs;k++, vec.resize(0) )
		for(i=0; i<elems; ++i)
		{			
			//vec.append( (uchar)i );
			vec.insert( (uchar)i, 1, 0 );
		}
		printf("vector: %lu (%lu,%lu)\n", clock()-tick, (ulong)runs, (ulong)elems);
	}
}


NAMESPACE_END(basics)

