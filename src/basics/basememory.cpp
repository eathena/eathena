#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"

#include "basesync.h"
#include "basearray.h"


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
template <typename T>
int test_memcopy(T dummy)	
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

