#include "baseheaps.h"

#define CFIELDS

void test_heaps(int scale)
{

	if( scale<1 ) scale=1;
#if defined(CFIELDS)

#else
	scale*=10;
#endif

	size_t k;
	const size_t CFIELDSIZE = 5000000/scale;
	size_t elems=0;

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
	// Binary Heap Test
	///////////////////////////////////////////////////////////////////////////

	BinaryHeap<int> bhtest;

	elems = CFIELDSIZE;

	bhtest.clear();
	for(k=0; k<elems; k++)
		bhtest.append(array[0][k]);

	tick = clock();
	bhtest.restoreHeap();
	printf("restoreHeap %lu (%i elems)\n", clock()-tick, elems);


	BinaryHeapDH<int> bh1;
	BinaryHeap<int> bh2;
	size_t i;
	int val, val2;

	///////////////////////////////////////////////////////////////////////////
	bh1.insert(5);
	bh1.insert(2);
	bh1.insert(1);
	bh1.insert(4);
	bh1.insert(8);

	bh1.deleteMin(val);
	bh1.deleteMin(val);
	bh1.deleteMin(val);
	bh1.deleteMin(val);
	bh1.deleteMin(val);

	bh2.insert(5);
	bh2.insert(2);
	bh2.insert(1);
	bh2.insert(4);
	bh2.insert(8);

	bh2.deleteMin(val);
	bh2.deleteMin(val);
	bh2.deleteMin(val);
	bh2.deleteMin(val);
	bh2.deleteMin(val);

	///////////////////////////////////////////////////////////////////////////
	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; i++)
	{
		bh2.insert( array[0][i] );
	}
	printf("binary heap (updown) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; i++)
	{
		bh2.deleteMin( val );
		if( val<val2 )
		{
			printf("error1\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (updown) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);


	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; i++)
	{
		bh1.insert( array[0][i] );
	}
	printf("binary heap (downmove) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; i++)
	{
		bh1.deleteMin( val );
		if( val<val2 )
		{
			printf("error3\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (downmove) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);

}


/*
// implementation of a van Emde Boas max-heap, 
// capable of O(lglg n) insert/delete times.
template <class T> class VEBHeap
{
public:
    const size_t kMaxLeaves;

	// Child heaps
    VEBHeap *fChildren;
	// Side heap, tracks indices of maximum-value subheaps
	VEBHeap *fSideHeap;
	// Values stored in leaf nodes only
	T* fLeafValues;
	
	// Bookkeeping:
    int fCounter;
    int fLength;
    int fLowest;
    int fRootCeiling;
	
	// Values stored for fast retrieval:
    T fMaxTreeValue;

	// Constructs a new VEB-heap object to hold 'length' distinct
	// values beginning with 'start'.  For example, the following
	// call would create a (tiny) heap for the values 5 through 14:
	//
	//     VEBHeap heap = new VEBHeap(5, 10)
	//
	// @param start beginning of value range for the heap
	// @param length length of the heap
    VEBHeap(const T& start, size_t length)
		: fMaxTreeValue(INT_MIN),kMaxLeaves(2), fChildren(NULL), fSideHeap(NULL), fLeafValues(NULL)
	{
		fLowest  = start;
		fLength  = length;
		
		if(length > kMaxLeaves)
		{
            fRootCeiling = (int)sqrt((double)length);

            int highest  = (start + length) - 1;
            int low      = fLowest;
            int k        = 0;
            fChildren    = new VEBHeap[fRootCeiling];

            // Generate child heaps:
            while (low <= highest)
            {
                int high     = min(highest, low + fRootCeiling - 1);
                int num      = high - low + 1;
                fChildren[k] = new VEBHeap(low, num);

                low += fRootCeiling;
                k++;
            }

            // Create side heap
            fSideHeap = new VEBHeap(0, fRootCeiling);
        }
        else
        {
            fLeafValues = new T[length];
        }
    }
	~VEBHeap()
	{
		if(fChildren) delete[] fChildren;
		if(fSideHeap) delete   fSideHeap;
		if(fLeafValues) delete[] fLeafValues;
	}

	// @return true if the node is pristine; false otherwise
    bool isPristine()
    {
        return (fCounter == 0);
    }

	// @return true if the node is a singleton; false otherwise
    bool isSingleton()
    {
        return (fCounter == 1);
    }

	// Removes the maximum element from the heap.
	// @return maximum element in the heap
	// @throws Exception if the heap is empty. 
    T extractMax()
    {
        if (fCounter <= 0)
            throw exception_bound("VEBHeap: underflow");

        fCounter--;
        T result = fMaxTreeValue;

        if( isLeaf() )
        {
            T i = fMaxTreeValue - fLowest;
            fLeafValues[i]--;

            
            // * If the value counter drops to 0, but the overall count is
            // * nonzero, the current counter must have been higher.
            // * If the overall count is 0, the leaf is empty in either case.
            if (fLeafValues[i] == 0)
            {
                fMaxTreeValue = (fCounter > 0) ? fLowest : INT_MIN;
            }
        }
        else if (fCounter > 0)
        {
            int i  = fSideHeap.findMax();
            result = fChildren[i].extractMax();

            if (fChildren[i].isPristine())
            {
                fSideHeap.extractMax();
            }

            // * Update the current max value.  Note that this causes
            // * the last singleton child to become pristine.
            fMaxTreeValue = (isSingleton())
                ? fChildren[fSideHeap.extractMax()].extractMax()
                : fChildren[fSideHeap.findMax()].findMax();
        }

        return result;
    }


//     * Inserts a value into the heap.
//     *
//     * If the node is a leaf, it simply updates the counter for the value.
//     *
//     * If the node is not a leaf, then:
//     *   1. If it is pristine, store the value as the max and return
//     *   2. If it is a singleton:
//     *       a. for the same child, update the child
//     *          twice and notify the side heap
//     *       b. for different children, update each child once
//     *          and notify the side heap twice
//     *       c. either way it is two O(1) calls and one O(lglgn) call
//     *          so the total is O(lglgn)
//     *   3. If the heap has more than two children:
//     *       a. update the child
//     *       b. if the child is now a singleton, update the side heap
//     *       c. either way it is O(lglgn)
//     *
//     * @param  val value to insert into the heap
//     * @throws Exception if the value is out of range
    void insert(T val)
    {
        if ((val < fLowest) || (val >= fLowest + fLength))
			throw exception_bound("VEBHeap: out of bound");

        if (isLeaf())
        {
            fLeafValues[val - fLowest]++;
        }
        else if (isPristine())
        {
            // no-op
        }
        else if (isSingleton())
        {
            int i = childIndex(val);
            int j = childIndex(fMaxTreeValue);
            if (i != j)
            {
                fSideHeap.insert(i);
                fSideHeap.insert(j);
            }
            else
            {
                fSideHeap.insert(i);
            }
            fChildren[i].insert(val);
            fChildren[j].insert(fMaxTreeValue);
        }
        else
        {
            int i = childIndex(val);
            fChildren[i].insert(val);

            if (fChildren[i].isSingleton())
            {
                fSideHeap.insert(i);
            }
        }

        fCounter++;
        fMaxTreeValue = Math.max(val, fMaxTreeValue);
    }


    // @return the maximum value in the heap
    int findMax()
    {
        return fMaxTreeValue;
    }


    // Convenience method that displays the entire heap.
    void showHeap(const char* prefix)
    {
        printf(prefix);
		toString()
		printf(": ");
        if (isLeaf())
			printf("(leaf)");
        if (isPristine())
        {
            printf("pristine");
        }
        else if (isSingleton())
        {
            printf("singleton-" + fMaxTreeValue);
        }
        else
        {
            printf("count = %i; max = %i", fCounter, fMaxTreeValue);
        }
		printf("\n");

        if (! isLeaf())
        {
            char nextLevel[128];
			snprintf(nextLevel, sizeof(nextLevel), "%s    ", prefix);
            for (int i=0; i<fChildren.length; i++)
            {
                if (fChildren[i] != null) fChildren[i].showHeap(nextLevel);
            }
			snprintf(nextLevel, sizeof(nextLevel), "%s%s/side", prefix, toString());
            fSideHeap.showHeap(nextLevel);
        }
    }

    // @return simple string representation of heap information
    const char* toString()
    {
		static char buf[128];
		snprintf(buf, sizeof(buf), " (%i-%i)", fLowest, ((fLowest + fLength) - 1));
        return buf;
    }
    bool isLeaf()
    {
        return (fLength <= kMaxLeaves);
    }
    int childIndex(int val)
    {
        return (val - fLowest) / fRootCeiling;
    }
};


*/

