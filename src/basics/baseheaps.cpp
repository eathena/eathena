#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"
#include "baseheaps.h"


NAMESPACE_BEGIN(basics)



///////////////////////////////////////////////////////////////////////////////
// BinaryHeap class
// Insert appends the element at the end and upheaps it
// Delete swaps the last element in front and downheaps it until a leaf is reached
// about 20% slower than method in the second implementation
template <class T> class _BinaryHeapDH : private allocator_w_dy<T>, private elaborator_ct<T> 
{
	friend void test_heaps(int scale);
	bool append(const T&e)
	{
		if( this->cWpp<this->cEnd || this->checkwrite(1) )
		{
			*this->cWpp++ = e;
			return true;
		}
		return false;
	}
public:
	_BinaryHeapDH()							{}
	_BinaryHeapDH(const allocator<T>& a)	{ this->insert(a); }
	virtual ~_BinaryHeapDH()				{}

	///////////////////////////////////////////////////////////////////////////
	// returns number of elements
	virtual size_t size() const		{ return this->cWpp-this->cBuf; }
	virtual size_t length() const	{ return this->cWpp-this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// check if empty
	bool is_empty( ) const			{ return this->cWpp==this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// 
	bool realloc(size_t sz)
	{
		if( sz>this->size() )
			return this->checkwrite( sz-this->size() );
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()			{ return this->cWpp=this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const 	
	{
		if( inx>this->size() )
		{
			vector_error("heap: out of bound");
		}
		return this->cBuf[inx]; 
	}
	// not define write access to the heap elements
	const T& first()								
	{ 
		return this->operator[](0);
	}
	const T& last()									
	{
		return this->operator[](this->size()-1);
	}

	///////////////////////////////////////////////////////////////////////////
	// push/pop access
	virtual bool push(const T& e)				{ return this->insert(e); }
	virtual bool push(const allocator<T>& a)	{ return this->insert(a); }
	virtual bool push(const T* elem, size_t cnt){ return this->insert(elem, cnt); }

	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	// throw underflow when empty
	virtual T pop()
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if( this->cBuf == this->cWpp )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		// take out the first element
		T& minItem = *this->cBuf;
		// put the last element to the top
		*this->cBuf = *(--this->cWpp);
		// and percolate it down the heap
		this->percolateDown(0);
		return minItem;
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cBuf < this->cWpp )
		{	// take out the first element
			elem = *this->cBuf;
			// put the last element to the top
			*this->cBuf = *(--this->cWpp);
			// and percolate it down the heap
			this->percolateDown(0);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	// throw underflow when empty
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if( this->cBuf == this->cWpp )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		return *this->cBuf;
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cBuf < this->cWpp )
		{
			elem = *this->cBuf;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// Insert item x into the priority queue, maintaining heap order.
	// Duplicates are allowed.
	virtual bool insert(const allocator<T>& a)
	{
		typename allocator<T>::iterator iter(a);
		if( this->cWpp+a.size()<=this->cEnd || this->checkwrite(a.size()) )
		{
			while(iter)
				this->insert( *iter++ );
			return true;
		}
		return false;
	}
	virtual bool insert( const T* elem, size_t cnt )
	{
		const T* end = elem+cnt;
		if( this->cWpp+cnt<=this->cEnd || this->checkwrite(cnt) )
		{
			while(elem < end )
				this->insert( *elem++ );
			return true;
		}
		return false;
	}
	virtual bool insert( const T& e )
	{	// Percolate up
		if( this->cWpp<this->cEnd || this->checkwrite(1) )
		{
			size_t hole = (this->cWpp-this->cBuf);
			this->cWpp++;
			for( ; hole>0 && e<this->cBuf[(hole-1)/2]; hole = (hole-1)/2 )
				this->cBuf[hole] = this->cBuf[(hole-1)/2];
			this->cBuf[hole] = e;
			return true;
		}
		return false;
	}
	bool checkHeap()
	{
		size_t i;
		for(i=0; i<this->size()/2; ++i)
		{
			if(  this->cBuf[i] > this->cBuf[2*i+1] ||
				(this->cBuf[i] > this->cBuf[2*i+2] && (2*i+2)<this->size()) )
				return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// Establish heap order property from an arbitrary
	// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		size_t i=this->size()/2;
		while(i>0)
		{
			--i;
			this->percolateDown(i);
		}
	}
private:
	// Internal method to percolate down in the heap.
	// hole is the index at which the percolate begins.
	void percolateDown( size_t hole )
	{
		size_t child, sz=this->size();
		if(sz>0)
		{
			T tmp = this->cBuf[hole];
			for( ; hole*2+1 < sz; hole = child )
			{
				child = 2*hole+1;
				if( child+1 < sz && this->cBuf[child+1] < this->cBuf[child] )
					child++;
				if( this->cBuf[child] < tmp )
					this->cBuf[hole] = this->cBuf[child];
				else
					break;
			}
			this->cBuf[hole] = tmp;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Insert appends the element at the end and upheaps it
// Delete downheaps the hole until a leaf is reached, 
//    then replaces the hole with the last element and upheaps it
template <class T> class _BinaryHeap : private allocator_w_dy<T>, private elaborator_ct<T> 
{
	friend void test_heaps(int scale);

	bool append(const T&e)
	{
		if( this->cWpp<this->cEnd || this->checkwrite(1) )
		{
			*this->cWpp++ = e;
			return true;
		}
		return false;
	}

public:
	_BinaryHeap()						{}
	_BinaryHeap(const allocator<T>& a)	{ this->insert(a); }
	virtual ~_BinaryHeap()				{}

	///////////////////////////////////////////////////////////////////////////
	// returns number of elements
	virtual size_t size() const		{ return this->cWpp-this->cBuf; }
	virtual size_t length() const	{ return this->cWpp-this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// check if empty
	bool is_empty( ) const			{ return this->cWpp==this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// 
	bool realloc(size_t sz)
	{
		if( sz>this->size() )
			return this->checkwrite( sz-this->size() );
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()			{ return this->cWpp=this->cBuf; }

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const 	
	{
		if( inx>this->size() )
		{
			vector_error("heap: out of bound");
		}
		return this->cBuf[inx]; 
	}
	// not define write access to the heap elements
	const T& first()								
	{ 
		return this->operator[](0);
	}
	const T& last()									
	{
		return this->operator[](this->size()-1);
	}

	///////////////////////////////////////////////////////////////////////////
	// push/pop access
	virtual bool push(const T& e)				{ return this->insert(e); }
	virtual bool push(const allocator<T>& a)	{ return this->insert(a); }
	virtual bool push(const T* elem, size_t cnt){ return this->insert(elem, cnt); }


	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T pop()
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if( this->cBuf==this->cWpp )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		T tmp;
		this->pop( tmp );
		return tmp;
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cBuf==this->cWpp )
			return false;

		size_t i,h,k;
		elem = this->cBuf[0];
		--this->cWpp;
		size_t sz = this->size();
		// downheap 
		for(h=0, k=2; k<=sz; h=k, k=k*2+2)
		{
			if( this->cBuf[k] > this->cBuf[k-1] )
				--k;
			this->cBuf[h]=this->cBuf[k];
		}
		if(h < sz)
		{
			// insert the last element at the hole
			// and upheap it beginning from h
			for( i = (h-1)/2;
				(h > 0) && this->cBuf[sz]<this->cBuf[i];
				h=i, i=(h-1)/2)
			{
				this->cBuf[h] = this->cBuf[i];
			}
			this->cBuf[h]=this->cBuf[sz];
		}
		return true;
	}
	/*
	// worse performance
	virtual bool pop( T & minItem )
	{
		if( this->cBuf==this->cWpp )
			return false;

		minItem = *this->cBuf;
		// we now have a hole at position 0 which needs to be filled
		--this->cWpp;

		size_t h=0, i=1; // i=2*h+1, for the general case
		size_t sz = this->size();
		while(i<=sz)
		{
			// check if there are two children and the second is the larger one
////////////// this check can be avoided, see implementation above
			if( i<sz && this->cBuf[i]>this->cBuf[i+1] ) i++;	// select the second
			// move the children up
			this->cBuf[h] = this->cBuf[i];
			// and continue with the downmoved hole
			h=i;
			i = 2*h+1;
		}
		if(h<sz)
		{	// single leaf but not at the end
			// fill with last node from the field
			// and move the node up if necessary
			while (h>0)
			{
				i=(h-1)/2;    // predecessor
				if( this->cBuf[sz]>=this->cBuf[i] ) 
					break;
				// otherwise swap nodes
				this->cBuf[h] = this->cBuf[i];
				h=i;
			}
			this->cBuf[h] = this->cBuf[sz];
		}
		return true;
	}
	*/
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	// throw Underflow if empty.
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if(this->cBuf == this->cWpp )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		return *this->cBuf;
	}
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if(this->cBuf == this->cWpp )
		{
			elem = *this->cBuf;
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// Insert item x into the priority queue, maintaining heap order.
	// Duplicates are allowed.
	virtual bool insert(const allocator<T>& a)
	{
		typename allocator<T>::iterator iter(a);
		if( this->cWpp+a.size()<=this->cEnd || this->checkwrite(a.size()) )
		{
			while(iter)
				this->insert( *iter++ );
			return true;
		}
		return false;
	}
	virtual bool insert( const T* elem, size_t cnt )
	{
		const T* end = elem+cnt;
		if( this->cWpp+cnt<=this->cEnd || this->checkwrite(cnt) )
		{
			while(elem < end )
				this->insert( *elem++ );
			return true;
		}
		return false;
	}
	bool insert( const T& e )
	{
		// Percolate up
		if( this->cWpp<this->cEnd || this->checkwrite(1) )
		{
			size_t hole = (this->cWpp-this->cBuf);
			this->cWpp++;
			for( ; hole>0 && e<this->cBuf[(hole-1)/2]; hole = (hole-1)/2 )
				this->cBuf[hole] = this->cBuf[(hole-1)/2];
			this->cBuf[hole] = e;
			return true;
		}
		return false;
	}
	bool checkHeap()
	{
		size_t i;
		for(i=0; i<this->size()/2; ++i)
		{
			if(  this->cBuf[i] > this->cBuf[2*i+1] ||
				(this->cBuf[i] > this->cBuf[2*i+2] && (2*i+2)<this->size()) )
				return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// Establish heap order property from an arbitrary
	// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		T tmp;
		ssize_t i;
		size_t h,k, sz=this->size();
		for(i=sz/2-1; i>=0; --i)
		{
			tmp = this->cBuf[i];
			for( h=i,k=h*2+2 ; k<sz; h=k, k=2*k+2 )
			{
				if( this->cBuf[k-1] < this->cBuf[k] )
					k--;
				if( this->cBuf[k] < tmp )
					this->cBuf[h] = this->cBuf[k];
				else
					break;
			}
			if(k==sz && this->cBuf[k-1] < tmp )
			{
				this->cBuf[h] = this->cBuf[k-1];
				h = k-1;
			}
			this->cBuf[h] = tmp;
		}
	}
};






void test_heaps(int scale)
{
#if defined(DEBUG)

	if( scale<1 ) scale=1;

	uint k;
	const uint CFIELDSIZE = 5000000/scale;
	uint elems=0;

	ulong tick;
	int *array[3];
	array[0]= new int[CFIELDSIZE];
	array[1]= new int[CFIELDSIZE];
	array[2]= new int[CFIELDSIZE];

	
	srand( time(NULL) );
	for(k=0; k<CFIELDSIZE; ++k)
		array[0][k]=array[1][k]=
		rand();						// random data
	//	k;							// sorted
	//	CFIELDSIZE-k;				// reverse sorted



{
	_BinaryHeapDH<int> bhtest1;

	elems = CFIELDSIZE;

	bhtest1.clear();
	bhtest1.realloc(elems);
	for(k=0; k<elems; ++k)
		bhtest1.append(array[0][k]);

	tick = clock();
	bhtest1.restoreHeap();
	printf("restoreHeapDH %lu (%i elems)\n", clock()-tick, elems);
	if( !bhtest1.checkHeap() )
		printf("restoreHeapDH failed\n");

	_BinaryHeap<int> bhtest2;

	elems = CFIELDSIZE;

	bhtest2.clear();
	bhtest2.realloc(elems);
	for(k=0; k<elems; ++k)
		bhtest2.append(array[0][k]);

	tick = clock();
	bhtest2.restoreHeap();
	printf("restoreHeap %lu (%i elems)\n", clock()-tick, elems);
	if( !bhtest2.checkHeap() )
		printf("restoreHeap failed\n");


	_BinaryHeapDH<int> bh1;
	_BinaryHeap<int> bh2;
	size_t i;
	int val=0, val2=0;

	///////////////////////////////////////////////////////////////////////////
	bh1.append(5);
	bh1.append(4);
	bh1.append(8);
	bh1.append(3);
	bh1.append(6);
	bh1.append(2);
	bh1.append(1);
	bh1.restoreHeap();


	bh1.pop(val); if( val!=1 ) printf("bheap 1 pop failed (%i!=1)\n", val);
	bh1.pop(val); if( val!=2 ) printf("bheap 1 pop failed (%i!=2)\n", val);
	bh1.pop(val); if( val!=3 ) printf("bheap 1 pop failed (%i!=3)\n", val);
	bh1.pop(val); if( val!=4 ) printf("bheap 1 pop failed (%i!=4)\n", val);
	bh1.pop(val); if( val!=5 ) printf("bheap 1 pop failed (%i!=5)\n", val);

	bh2.append(5);
	bh2.append(4);
	bh2.append(8);
	bh2.append(3);
	bh2.append(6);
	bh2.append(2);
	bh2.append(1);
	bh2.restoreHeap();

	bh2.pop(val); if( val!=1 ) printf("bheap 2 pop failed (%i!=1)\n", val);
	bh2.pop(val); if( val!=2 ) printf("bheap 2 pop failed (%i!=2)\n", val);
	bh2.pop(val); if( val!=3 ) printf("bheap 2 pop failed (%i!=3)\n", val);
	bh2.pop(val); if( val!=4 ) printf("bheap 2 pop failed (%i!=4)\n", val);
	bh2.pop(val); if( val!=5 ) printf("bheap 2 pop failed (%i!=5)\n", val);


	bh1.clear();
	bh2.clear();
	bh1.realloc(elems);
	bh2.realloc(elems);

	///////////////////////////////////////////////////////////////////////////

	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; ++i)
	{
		bh1.insert( array[0][i] );
	}
	printf("binary heap (downmove) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; ++i)
	{
		bh1.pop( val );
		if( val<val2 )
		{
			printf("error3\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (downmove) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);
	
	
	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; ++i)
	{
		bh2.insert( array[0][i] );
	}
	printf("binary heap (updown) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; ++i)
	{
		bh2.pop( val );
		if( val<val2 )
		{
			printf("error1\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (updown) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);



}

	///////////////////////////////////////////////////////////////////////////
	// Binary Heap Test
	///////////////////////////////////////////////////////////////////////////

	BinaryHeapDH<int> bhtest1;

	elems = CFIELDSIZE;

	bhtest1.clear();
	for(k=0; k<elems; ++k)
		bhtest1.append(array[0][k]);

	tick = clock();
	bhtest1.restoreHeap();
	printf("restoreHeapDH %lu (%i elems)\n", clock()-tick, elems);
	if( !bhtest1.checkHeap() )
		printf("restoreHeapDH failed\n");

	BinaryHeap<int> bhtest2;

	elems = CFIELDSIZE;

	bhtest2.clear();
	for(k=0; k<elems; ++k)
		bhtest2.append(array[0][k]);

	tick = clock();
	bhtest2.restoreHeap();
	printf("restoreHeap %lu (%i elems)\n", clock()-tick, elems);
	if( !bhtest2.checkHeap() )
		printf("restoreHeap failed\n");












	BinaryHeapDH<int> bh1;
	BinaryHeap<int> bh2;
	size_t i;
	int val, val2;

	///////////////////////////////////////////////////////////////////////////
	bh1.append(5);
	bh1.append(4);
	bh1.append(8);
	bh1.append(3);
	bh1.append(6);
	bh1.append(2);
	bh1.append(1);
	bh1.restoreHeap();


	bh1.pop(val); if( val!=1 ) printf("bheap 1 pop failed (%i!=1)\n", val);
	bh1.pop(val); if( val!=2 ) printf("bheap 1 pop failed (%i!=2)\n", val);
	bh1.pop(val); if( val!=3 ) printf("bheap 1 pop failed (%i!=3)\n", val);
	bh1.pop(val); if( val!=4 ) printf("bheap 1 pop failed (%i!=4)\n", val);
	bh1.pop(val); if( val!=5 ) printf("bheap 1 pop failed (%i!=5)\n", val);

	bh2.append(5);
	bh2.append(4);
	bh2.append(8);
	bh2.append(3);
	bh2.append(6);
	bh2.append(2);
	bh2.append(1);
	bh2.restoreHeap();

	bh2.pop(val); if( val!=1 ) printf("bheap 2 pop failed (%i!=1)\n", val);
	bh2.pop(val); if( val!=2 ) printf("bheap 2 pop failed (%i!=2)\n", val);
	bh2.pop(val); if( val!=3 ) printf("bheap 2 pop failed (%i!=3)\n", val);
	bh2.pop(val); if( val!=4 ) printf("bheap 2 pop failed (%i!=4)\n", val);
	bh2.pop(val); if( val!=5 ) printf("bheap 2 pop failed (%i!=5)\n", val);


	bh1.clear();
	bh2.clear();

	///////////////////////////////////////////////////////////////////////////

	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; ++i)
	{
		bh1.insert( array[0][i] );
	}
	printf("binary heap (downmove) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; ++i)
	{
		bh1.pop( val );
		if( val<val2 )
		{
			printf("error3\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (downmove) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);
	
	
	elems = CFIELDSIZE;
	tick = clock();
	for(i=0; i<elems; ++i)
	{
		bh2.insert( array[0][i] );
	}
	printf("binary heap (updown) insert %lu (%i elems)\n", clock()-tick, elems);

	val2=-1;
	tick = clock();
	for(i=0; i<CFIELDSIZE; ++i)
	{
		bh2.pop( val );
		if( val<val2 )
		{
			printf("error1\n");
			break;
		}
		val2=val;
	}
	printf("binary heap (updown) delete (+1 compare&assign) %lu (%i elems)\n", clock()-tick, elems);


#endif//DEBUG
}
NAMESPACE_END(basics)

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
            for (int i=0; i<fChildren.length; ++i)
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


