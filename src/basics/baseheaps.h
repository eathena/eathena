#ifndef __BASEHEAPS_H__
#define __BASEHEAPS_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"


///////////////////////////////////////////////////////////////////////////////
// test function
void test_heaps(int scale);



///////////////////////////////////////////////////////////////////////////////
// BinaryHeap classes
// ******************PUBLIC OPERATIONS*********************
// void insert( x )       --> Insert x
// deleteMin( minItem )   --> Remove (and optionally return) smallest item
// Comparable findMin( )  --> Return smallest item
// bool isEmpty( )        --> Return true if empty; else false
// bool isFull( )         --> Return true if full; else false
// void makeEmpty( )      --> Remove all items
// ******************ERRORS********************************
// Throws Underflow when compiled with exceptions
///////////////////////////////////////////////////////////////////////////////
// Insert appends the element at the end and upheaps it
// Delete removes the first element, replaces it with the last and downheaps it
template <class T> class BinaryHeapDH : private TArrayDST<T>
{
public:
	BinaryHeapDH()	{}
	BinaryHeapDH(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); i++)
			this->BinaryHeapDH::insert(arr[i]);
	}

	bool isEmpty( ) const
	{
		return this->cCnt == 0;
	}
	// Insert item x into the priority queue, maintaining heap order.
	// Duplicates are allowed.
	// Throw Overflow if container is full.
	virtual void insert( const T& x )
	{
		// Percolate up
		size_t hole = this->cCnt++;
		TArrayDST<T>::realloc(this->cCnt);

		for( ; hole>0 && x<this->cField[(hole-1)/2]; hole = (hole-1)/2 )
			this->cField[hole] = this->cField[(hole-1)/2];
		this->cField[hole] = x;
	}
	// Find the smallest item in the priority queue.
	// Return the smallest item, or throw Underflow if empty.
	const T& findMin( ) const
	{
		if( 0<=this->cCnt )
			throw exception_bound("BinaryHeap: underflow");
		return this->cField[0];
	}
	// Remove the smallest item from the priority queue.
	// Throw Underflow if empty.
	void deleteMin( )
	{
		if( 0==this->cCnt )
			throw exception_bound("BinaryHeap: underflow");
		// put the last element to the top
		this->cField[0] = this->cField[ --this->cCnt ];
		// and percolate it down the heap
		percolateDown(0);
	}
	// Remove the smallest item from the priority queue
	// and place it in minItem. Throw Underflow if empty.
	void deleteMin( T & minItem )
	{
		if( 0==this->cCnt )
			throw exception_bound("BinaryHeap: underflow");
		// take out the first element
		minItem = this->cField[0];
		// put the last element to the top
		this->cField[0] = this->cField[ --this->cCnt ];
		// and percolate it down the heap
		percolateDown(0);
	}
	// Make the priority queue logically empty.
	void makeEmpty( )
	{
		TArrayDST<T>::resize(0);
	}
	// Establish heap order property from an arbitrary
	// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		size_t i=TArrayDST<T>::cCnt;
		while(i>1)
		{
			--i;
			percolateDown(i);
		}
	}
private:
	// Internal method to percolate down in the heap.
	// hole is the index at which the percolate begins.
	void percolateDown( size_t hole )
	{
		if(this->cCnt>0)
		{
			size_t child;
			T tmp = (*this)[hole];
			for( ; hole*2+1 < this->cCnt; hole = child )
			{
				child = 2*hole+1;
				if( child+1 < TArrayDST<T>::cCnt && (*this)[child+1] < (*this)[child] )
					child++;
				if( (*this)[child] < tmp )
					(*this)[hole] = (*this)[child];
				else
					break;
			}
			(*this)[hole] = tmp;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// Insert appends the element at the end and upheaps it
// Delete downheaps the hole until a leaf is reached, 
//    then replaces the hole with the last element and upheaps it
template <class T> class BinaryHeap : public TArrayDST<T>
{
	friend void test_algo(int scale);
public:
	BinaryHeap()	
	{
	}
	BinaryHeap(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); i++)
			this->BinaryHeap::insert(arr[i]);
	}
	///////////////////////////////////////////////////////////////////////////
	// check if empty
	bool isEmpty( ) const
	{
		return this->cCnt == 0;
	}
	///////////////////////////////////////////////////////////////////////////
	// remove all elements
	virtual bool clear()
	{
		TArrayDST<T>::resize(0);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// returns number of elements
	virtual size_t size() const						{ return TArrayDST<T>::size(); }
	virtual size_t length() const					{ return TArrayDST<T>::size(); }

	///////////////////////////////////////////////////////////////////////////
	// access to element[inx]
	virtual const T& operator[](size_t inx) const 	{ return TArrayDST<T>::operator[](inx); }
	// not define write access to the heap elements
	const T& first()								{ return TArrayDST<T>::first(); }
	const T& last()									{ return TArrayDST<T>::last(); }

	///////////////////////////////////////////////////////////////////////////
	// push/pop access
	virtual bool push(const T& elem)				{ return insert(elem); }
	virtual bool push(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); i++)
			this->BinaryHeap::insert(arr[i]);
		return true;
	}
	virtual bool push(const T* elem, size_t cnt)
	{
		size_t i;
		for(i=0; i<cnt; i++)
			this->BinaryHeap::insert(elem[i]);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// return the first element and remove it from list
	virtual T pop()							{ return deleteMin(); }
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool pop(T& elem)				{ return deleteMin(elem); }
	///////////////////////////////////////////////////////////////////////////
	// return the first element and do not remove it from list
	virtual T& top() const					{ return findMin(); }
	///////////////////////////////////////////////////////////////////////////
	// as above but with check if element exist
	virtual bool top(T& elem) const			{ return findMin(elem); }

	// Insert item x into the priority queue, maintaining heap order.
	// Duplicates are allowed.
	bool insert( const T& x )
	{
		// Percolate up
		size_t i, h = this->cCnt++;
		TArrayDST<T>::realloc(this->cCnt);
		// upheap starting from last element
		for( i = (h-1)/2;
			(h > 0) && x<this->cField[i];
			h = i, i = (h-1)/2)
		{
			this->cField[h] = this->cField[i];
		}
		this->cField[h]=x;
		return true;
	}
	// Find the smallest item in the priority queue.
	// Return the smallest item, or throw Underflow if empty.
	T& findMin( ) const
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if(this->cCnt<=0)
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		return this->cField[0];
	}
	bool findMin(T & minItem ) const
	{
		if(this->cCnt>0)
		{
			minItem = this->cField[0];
			return true;
		}
		return false;
	}
	// Remove the smallest item from the priority queue.
	// Throw Underflow if empty.
	T deleteMin( )
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if( 0==this->cCnt )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		T tmp;
		deleteMin( tmp );
		return tmp;
	}
	// Remove the smallest item from the priority queue
	// and place it in minItem. Throw Underflow if empty.
	bool deleteMin( T & minItem )
	{
		if(this->cCnt<=0)
			return false;

		size_t i,h,k;
		minItem = this->cField[0];
	
		this->cCnt--;
		// downheap 
		for(h=0, k=2; k<=this->cCnt; h=k, k=k*2+2)
		{
			if( this->cField[k] > this->cField[k-1] )
				k--;
			this->cField[h]=this->cField[k];
		}
		if(h < this->cCnt)
		{
			// insert the last element at the hole
			// and upheap it beginning from h
			for( i = (h-1)/2;
				(h > 0) && this->cField[this->cCnt]<this->cField[i];
				h=i, i=(h-1)/2)
			{
				this->cField[h] = this->cField[i];
			}
			this->cField[h]=this->cField[this->cCnt];
		}
		return true;
	}
	/*
	// worse performance
	bool deleteMin( T & minItem )
	{
		if(this->cCnt<=0)
			return false;

		minItem = (*this)[0];
		// we now have a hole at position 0 which needs to be filled
		--this->cCnt;

		size_t h=0, i=1; // i=2*h+1, for the general case
		while(i<=this->cCnt)
		{
			// check if there are two children and the second is the larger one
////////////// this check can be avoided, see implementation above
			if( i<this->cCnt && this->cField[i]>this->cField[i+1] ) i++;	// select the second
			// move the children up
			this->cField[h] = this->cField[i];
			// and continue with the downmoved hole
			h=i;
			i = 2*h+1;
		}
		if(h<this->cCnt)
		{	// single leaf but not at the end
			// fill with last node from the field
			// and move the node up if necessary
			while (h>0)
			{
				i=(h-1)/2;    // predecessor
				if( this->cField[this->cCnt]>=(*this)[i] ) 
					break;
				// otherwise swap nodes
				this->cField[h] = this->cField[i];
				h=i;
			}
			this->cField[h] = this->cField[this->cCnt];
		}
		return true;
	}
	*/

	// Establish heap order property from an arbitrary
	// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		T x;
		ssize_t i,j,k;
		// Build the heap using shiftup.
		// same routine as in the bottomup Heapsorts but with reversed compares
		// and includeing k=0 to build the heap completely
		for(i=k=this->cCnt/2-1; k>=0; i=--k)
		{
			x = this->cField[i];
			j = 2*i+2;
			while( (size_t)j<this->cCnt )
			{
				if( this->cField[j]>this->cField[j-1] )
					--j;
				this->cField[i] = this->cField[j];
				i = j; 
				j = 2*i+2;
			}
			if((size_t)j==this->cCnt)
			{
				this->cField[i] = this->cField[this->cCnt-1];
				i = this->cCnt-1; 
			}
			j = (i+1)/2-1;
			while(j>=k)
			{
				if( this->cField[j]>x )
				{
					this->cField[i] = this->cField[j];
					i = j;
					j = (i+1)/2-1;
				}
				else
					break;
			}
			///////////////
			this->cField[i] = x;
		}
	}
};





#endif//__BASEHEAPS_H__

