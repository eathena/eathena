


#include "basetypes.h"


#ifdef DEBUG
#define CNTCOPIES	// debug option
#endif

#include "basealgo.h"
#include "basearray.h"


#if defined(DEBUG)

#define CFIELDS	// test on ordinary carrays



#if defined(CFIELDS)
inline void copy(int*a, const int*b, size_t sz)
{
	memcpy(a,b,sz*sizeof(int));
}
// compare for buildin qsort
static inline int intcompare(const void*a, const void*b)
{
	return *((int*)a) - *((int*)b);
}
#else
inline void copy(TArrayDST<int>& a, const TArrayDST<int>& b, size_t sz)
{
	a=b;
}
#endif





int cmp(const int& a, const char&b)
{
	return a-(int)b;
}


class aaaa
{
	char val;
public:
	aaaa()	{}
	aaaa(int i):val(i)	{}

	int compare(const int& a) const { return val-a; }
};


#endif//DEBUG


void test_algo(int scale)
{
#if defined(DEBUG)
	{
		char buf1[10] = {0,1,2,3,5,6,7,8};
		aaaa buf2[10] = {0,1,2,3,5,6,7,8};
		int a = 4;
		size_t findpos;

		BinarySearchC<int,char*,char>(a, buf1, 8, 0, findpos, cmp);
		BinarySearchB<int,aaaa*,aaaa>(7, buf2, 8, 0, findpos, &aaaa::compare);
	
	
		a = findpos;



		uchar array[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
		TArrayDST<uchar> ta(array,20);
		size_t k,sz = 20;

		size_t tpos=1, spos=5, cnt=8;

		elementmove(array,20,tpos,spos,cnt);
		elementmove(ta, 20, tpos,spos,cnt);

		for(k=0; k<sz; k++)
			printf("%i ", array[k]);
		printf("\n");

		for(k=0; k<sz; k++)
			printf("%i ", ta[k]);
		printf("\n");

	}



	if( scale<1 ) scale=1;
#if defined(CFIELDS)

#else
	scale*=10;
#endif

	uint k;
	const uint CFIELDSIZE = 5000000/scale;
	const uint CBUBBLESIZE = 10000/scale;
	uint elems=0;

	ulong tick;
#if defined(CFIELDS)
	int *array[3];
	array[0]= new int[CFIELDSIZE];
	array[1]= new int[CFIELDSIZE];
	array[2]= new int[CFIELDSIZE];
	printf("testing cfields\n");
#else
	TArrayDST<int> array[3];
	array[0].resize(CFIELDSIZE);
	array[1].resize(CFIELDSIZE);
	array[2].resize(CFIELDSIZE);
	printf("testing arrays\n");

#endif

	for(k=0; k<CFIELDSIZE; k++)
		array[0][k]=array[1][k]=
		rand();						// random data
	//	CFIELDSIZE-k;				// reverse sorted



	///////////////////////////////////////////////////////////////////////////
	// Sort Tests
	///////////////////////////////////////////////////////////////////////////

	printf("work on unsorted fields\n");


#if defined(CFIELDS)
	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	qsort(array[1], elems, sizeof(int), intcompare);
	printf("buildin qsort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<CFIELDSIZE-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }
#endif


	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	BubbleSort<int>(array[1], elems/100, elems-1);
	printf("BubbleSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	BubbleSort<int>(array[1], elems);
	printf("BubbleSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	InsertionSort<int>(array[1], elems/100, elems-1);
	printf("InsertionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	InsertionSort<int>(array[1], elems);
	printf("InsertionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	SelectionSort<int>(array[1], elems/100, elems-1);
	printf("SelectionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	SelectionSort<int>(array[1], elems);
	printf("SelectionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	ShellSort<int>(array[1], elems/100, elems-1);
	printf("ShellSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	ShellSort<int>(array[1], elems);
	printf("ShellSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	MergeSort<int>(array[1], array[2], elems/100, elems-1);
	printf("MergeSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	MergeSort<int>(array[1], array[2], elems);
	printf("MergeSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	CombSort<int>(array[1], elems/100, elems-1);
	printf("CombSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	CombSort<int>(array[1], elems);
	printf("CombSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSort<int>(array[1], elems/100, elems-1);
	printf("HeapSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSort<int>(array[1], elems);
	printf("HeapSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }


	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUDH<int>(array[1], elems/100, elems-1);
	printf("HeapSortBUDH %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUDH<int>(array[1], elems);
	printf("HeapSortBUDH %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUUH<int>(array[1], elems/100, elems-1);
	printf("HeapSortBUUH %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUUH<int>(array[1], elems);
	printf("HeapSortBUUH %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }


	
	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	QuickSortClassic<int>(array[1], elems/100, elems-1);
	printf("classic quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	QuickSortClassic<int>(array[1], elems);
	printf("classic quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	QuickSort<int>(array[1], elems/100, elems-1);
	printf("mod quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }
	
	copy(array[1], array[0], CFIELDSIZE);
	elems = CFIELDSIZE;
	tick = clock();
	QuickSort<int>(array[1], elems);
	printf("mod quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }





	printf("work on sorted fields\n");
#if defined(CFIELDS)
	elems = CFIELDSIZE;
	tick = clock();
	qsort(array[1], elems, sizeof(int), intcompare);
	printf("buildin qsort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<CFIELDSIZE-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }
#endif


	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	BubbleSort<int>(array[1], elems/100, elems-1);
	printf("BubbleSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	BubbleSort<int>(array[1], elems);
	printf("BubbleSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	InsertionSort<int>(array[1], elems/100, elems-1);
	printf("InsertionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	InsertionSort<int>(array[1], elems);
	printf("InsertionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	SelectionSort<int>(array[1], elems/100, elems-1);
	printf("SelectionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = (CFIELDSIZE<CBUBBLESIZE)?CFIELDSIZE:CBUBBLESIZE;
	tick = clock();
	SelectionSort<int>(array[1], elems);
	printf("SelectionSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	ShellSort<int>(array[1], elems/100, elems-1);
	printf("ShellSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	ShellSort<int>(array[1], elems);
	printf("ShellSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	MergeSort<int>(array[1], array[2], elems/100, elems-1);
	printf("MergeSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	MergeSort<int>(array[1], array[2], elems);
	printf("MergeSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	CombSort<int>(array[1], elems/100, elems-1);
	printf("CombSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	CombSort<int>(array[1], elems);
	printf("CombSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	HeapSort<int>(array[1], elems/100, elems-1);
	printf("HeapSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	HeapSort<int>(array[1], elems);
	printf("HeapSort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUDH<int>(array[1], elems/100, elems-1);
	printf("HeapSortBUDH %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUDH<int>(array[1], elems);
	printf("HeapSortBUDH %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUUH<int>(array[1], elems/100, elems-1);
	printf("HeapSortBUUH %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	HeapSortBUUH<int>(array[1], elems);
	printf("HeapSortBUUH %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	QuickSortClassic<int>(array[1], elems/100, elems-1);
	printf("classic quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

	elems = CFIELDSIZE;
	tick = clock();
	QuickSortClassic<int>(array[1], elems);
	printf("classic quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }



	elems = CFIELDSIZE;
	tick = clock();
	QuickSort<int>(array[1], elems/100, elems-1);
	printf("mod quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=elems/100; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }
	
	elems = CFIELDSIZE;
	tick = clock();
	QuickSort<int>(array[1], elems);
	printf("mod quicksort %lu (%i elems)\n", clock()-tick, elems);
	for(k=0; k<elems-1; k++) if( array[1][k]>array[1][k+1]) { printf("error inx %i\n", k); break; }

#if defined(CFIELDS)
	delete[] array[0];
	delete[] array[1];
	delete[] array[2];
#endif

#endif//DEBUG
}


