
#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"



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



//!! TODO copy back c++ memory handler












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
	for(i=0; i<100;i++)
	{	
		target=t, source=s;
		// simple type mover, no checks performed
		memmove(target, source, cnt*sizeof(*target));
	}
	printf("memmove: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;i++)
	{
		target=t, source=s;
		// simple type copy, no checks performed
		memcpy(target, source, cnt*sizeof(*target));
	}
	printf("memcpy: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;i++)
	{
		target=t, source=s;
		// simple type compare, no checks performed
		sz = memcmp(target,source,cnt*sizeof(*target));
	}
	printf("memcmp: %lu\n", clock()-tick);

	tick=clock();
	for(i=0; i<100;i++)
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
	for(i=0; i<100;i++)
	{	
		target=t, source=s;
		T* epp=target+cnt;
		while( target<epp ) *target++ = *source++;
	}
	printf("cpy: %lu\n", clock()-tick);

	tick=clock();

	for(i=0; i<100;i++)
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


void test_memory(void)
{
#if defined(DEBUG)

	printf("copy chars\n");
	test_memcopy<char>(1);
	printf("copy shorts\n");
	test_memcopy<short>(1);
	printf("copy longs\n");
	test_memcopy<long>(1);

	printf("test memswap\n");
	test_memswap();

#endif//DEBUG

}
