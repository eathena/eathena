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


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test function
void test_heaps(int scale=1);




///////////////////////////////////////////////////////////////////////////////
/// Binary Heap class.
/// Insert appends the element at the end and upheaps it
/// Delete swaps the last element in front and downheaps it until a leaf is reached
/// about 20% slower than method in the second implementation
template <typename T>
class BinaryHeapDH : private TArrayDCT<T>
{

	friend void test_heaps(int scale);
public:
	BinaryHeapDH()	{}
	BinaryHeapDH(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); ++i)
			this->BinaryHeapDH::insert(arr[i]);
	}
	virtual ~BinaryHeapDH()	{}

	///////////////////////////////////////////////////////////////////////////
	/// check if empty
	bool isEmpty( ) const
	{
		return this->cCnt == 0;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()
	{
		this->TArrayDST<T>::resize(0);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// returns number of elements
	virtual size_t size() const						{ return this->TArrayDCT<T>::size(); }
	virtual size_t length() const					{ return this->TArrayDCT<T>::size(); }

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx]
	virtual const T& operator[](size_t inx) const 	{ return this->TArrayDCT<T>::operator[](inx); }
	// not define write access to the heap elements
	const T& first()								{ return this->TArrayDCT<T>::first(); }
	const T& last()									{ return this->TArrayDCT<T>::last(); }

	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)				{ return this->BinaryHeapDH<T>::insert(elem); }
	virtual bool push(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); ++i)
			this->BinaryHeapDH<T>::insert(arr[i]);
		return true;
	}
	virtual bool push(const T* elem, size_t cnt)
	{
		size_t i;
		for(i=0; i<cnt; ++i)
			this->BinaryHeapDH<T>::insert(elem[i]);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from list
	/// throw underflow when empty
	virtual T pop()
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if(this->cCnt<=0)
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		// take out the first element
		T& minItem = this->cField[0];
		// put the last element to the top
		this->cField[0] = this->cField[ --this->cCnt ];
		// and percolate it down the heap
		this->percolateDown(0);
		return minItem;
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if( this->cCnt )
		{	// take out the first element
			elem = this->cField[0];
			// put the last element to the top
			this->cField[0] = this->cField[ --this->cCnt ];
			// and percolate it down the heap
			this->percolateDown(0);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from list
	/// throw underflow when empty
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if(this->cCnt<=0)
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		return this->cField[0];
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if( this->cCnt )
		{
			elem = this->cField[0];
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Insert item x into the priority queue, maintaining heap order.
	/// Duplicates are allowed.
	virtual bool insert( const T& e )
	{
		// Percolate up
		size_t hole = this->cCnt++;
		this->TArrayDST<T>::realloc(this->cCnt);

		for( ; hole>0 && e<this->cField[(hole-1)/2]; hole = (hole-1)/2 )
			this->cField[hole] = this->cField[(hole-1)/2];
		this->cField[hole] = e;
		return true;
	}
	bool checkHeap()
	{
		size_t i;
		for(i=0; i<this->cCnt/2; ++i)
		{
			if(  this->cField[i] > this->cField[2*i+1] ||
				(this->cField[i] > this->cField[2*i+2] && (2*i+2)<this->cCnt) )
				return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Establish heap order property from an arbitrary
	/// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		size_t i=this->cCnt/2;
		while(i>0)
		{
			--i;
			this->percolateDown(i);
		}
	}
private:
	/// Internal method to percolate down in the heap.
	/// hole is the index at which the percolate begins.
	void percolateDown( size_t hole )
	{
		if(this->cCnt>0)
		{
			size_t child;
			T tmp = this->cField[hole];
			for( ; hole*2+1 < this->cCnt; hole = child )
			{
				child = 2*hole+1;
				if( child+1 < this->cCnt && this->cField[child+1] < this->cField[child] )
					child++;
				if( this->cField[child] < tmp )
					this->cField[hole] = this->cField[child];
				else
					break;
			}
			this->cField[hole] = tmp;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// Binary Heap using UpDown-movement.
/// Insert appends the element at the end and upheaps it
/// Delete downheaps the hole until a leaf is reached, 
///    then replaces the hole with the last element and upheaps it
template <typename T>
class BinaryHeap : public TArrayDCT<T>
{
	friend void test_heaps(int scale);
public:
	BinaryHeap()	
	{
	}
	BinaryHeap(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); ++i)
			this->BinaryHeap::insert(arr[i]);
	}
	virtual ~BinaryHeap()	{}
	///////////////////////////////////////////////////////////////////////////
	/// check if empty
	bool isEmpty( ) const
	{
		return this->cCnt == 0;
	}
	///////////////////////////////////////////////////////////////////////////
	/// remove all elements
	virtual bool clear()
	{
		TArrayDST<T>::resize(0);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// returns number of elements
	virtual size_t size() const						{ return this->TArrayDST<T>::size(); }
	virtual size_t length() const					{ return this->TArrayDST<T>::size(); }

	///////////////////////////////////////////////////////////////////////////
	/// access to element[inx]
	virtual const T& operator[](size_t inx) const 	{ return this->TArrayDST<T>::operator[](inx); }
	// not define write access to the heap elements
	const T& first()								{ return this->TArrayDST<T>::first(); }
	const T& last()									{ return this->TArrayDST<T>::last(); }

	///////////////////////////////////////////////////////////////////////////
	/// push/pop access
	virtual bool push(const T& elem)				{ return this->BinaryHeap<T>::insert(elem); }
	virtual bool push(const TArray<T>& arr)
	{
		size_t i;
		for(i=0; i<arr.size(); ++i)
			this->BinaryHeap<T>::insert(arr[i]);
		return true;
	}
	virtual bool push(const T* elem, size_t cnt)
	{
		size_t i;
		for(i=0; i<cnt; ++i)
			this->BinaryHeap<T>::insert(elem[i]);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and remove it from list
	virtual T pop()
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if( 0==this->cCnt )
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		T tmp;
		this->BinaryHeap<T>::pop( tmp );
		return tmp;
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool pop(T& elem)
	{
		if(this->cCnt<=0)
			return false;

		size_t i,h,k;
		elem = this->cField[0];
	
		this->cCnt--;
		// downheap 
		for(h=0, k=2; k<this->cCnt; h=k, k=k*2+2)
		{
			if( this->cField[k] > this->cField[k-1] )
				k--;
			this->cField[h]=this->cField[k];
		}
		if( k==this->cCnt )
		{
			--k;
			this->cField[h]=this->cField[k];
			h = k;
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
	virtual bool pop( T & minItem )
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
	///////////////////////////////////////////////////////////////////////////
	/// return the first element and do not remove it from list
	/// throw Underflow if empty.
	virtual T& top() const
	{
#ifdef CHECK_BOUNDS
#ifdef CHECK_EXCEPTIONS
		if(this->cCnt<=0)
			throw exception_bound("BinaryHeap: underflow");
#endif
#endif
		return this->cField[0];
	}
	///////////////////////////////////////////////////////////////////////////
	/// as above but with check if element exist
	virtual bool top(T& elem) const
	{
		if(this->cCnt>0)
		{
			elem = this->cField[0];
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Insert item x into the priority queue, maintaining heap order.
	/// Duplicates are allowed.
	virtual bool insert( const T& e )
	{
		// Percolate up
		size_t hole = this->cCnt++;
		this->TArrayDST<T>::realloc(this->cCnt);

		for( ; hole>0 && e<this->cField[(hole-1)/2]; hole = (hole-1)/2 )
			this->cField[hole] = this->cField[(hole-1)/2];
		this->cField[hole] = e;
		return true;
	}
	bool checkHeap()
	{
		size_t i;
		for(i=0; i<this->cCnt/2; ++i)
		{
			if(  this->cField[i] > this->cField[2*i+1] ||
				(this->cField[i] > this->cField[2*i+2] && (2*i+2)<this->cCnt) )
				return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Establish heap order property from an arbitrary
	/// arrangement of items. Runs in linear time.
	void restoreHeap( )
	{
		T tmp;
		ssize_t i;
		size_t h,k;
		for(i=this->cCnt/2-1; i>=0; --i)
		{
			tmp = this->cField[i];
			for( h=i,k=h*2+2 ; k<this->cCnt; h=k, k=2*k+2 )
			{
				if( this->cField[k-1] < this->cField[k] )
					k--;
				if( this->cField[k] < tmp )
					this->cField[h] = this->cField[k];
				else
					break;
			}
			if(k==this->cCnt && this->cField[k-1] < tmp )
			{
				this->cField[h] = this->cField[k-1];
				h = k-1;
			}
			this->cField[h] = tmp;
		}
	}
};



NAMESPACE_END(basics)

#endif//__BASEHEAPS_H__

