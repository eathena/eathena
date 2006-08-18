#ifndef __BASEALGORITHM_H__
#define __BASEALGORITHM_H__

#include "basetypes.h"

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// test function
///////////////////////////////////////////////////////////////////////////////
void test_algo(int scale=1);



///////////////////////////////////////////////////////////////////////////////
/// Simple Binary Search. (asc order)
template<class X, class L> bool BinarySearchS( X const (& elem), L const (& list), size_t sz, size_t startpos, size_t& findpos)
{
	size_t a = startpos-1;
	size_t b = sz;
	size_t c;
	do
	{
		c=(a+b)/2;
		if( list[c].operator==(elem) )
		{
			findpos=c;
			return true;
			break;
		}
		else if( list[c] < elem )
			a=c;
		else
			b=c;
	} while( (a+1) < b );

	return false;
}


///////////////////////////////////////////////////////////////////////////////
/// Binary Search for both orders.
/// checks borders for out-of-bounds first
/// if returning false, the findpos contains the position 
/// where to insert the new element in the fields
/// uses binary operators of the elements
/// works on objects and fields likewise since no pointer arithmetic used
///////////////////////////////////////////////////////////////////////////////
template<class X, class L> bool BinarySearch(X const (& elem), L const (& list), size_t sz, size_t startpos, size_t& findpos, bool asc=true)
{	// do a binary search
	// make some initial stuff
	bool ret = false;
	size_t a= (startpos>=sz) ? 0 : startpos;
	size_t b= sz-1;
	size_t c;

	if( sz < 1)
	{
		findpos = 0;
		ret = false;
	}
	else if( asc )
	{	//smallest element first
		if( elem <= list[a] )
		{	
			if( elem == list[a] ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( elem >= list[b] )
		{	
			if( elem == list[b] )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == list[c] )
				{
					b=c;
					ret = true;
					break;
				}
				else if( elem < list[c] )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next smaller element to the given or the found element
		}
	}
	else // descending
	{	//smallest element last
		if( elem >= list[a] )
		{	
			if( elem == list[a] ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( elem <= list[b] )
		{	
			if( elem == list[b] )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( elem == list[c] )
				{	b=c;
					ret = true;
					break;
				}
				else if( elem > list[c] )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next larger element to the given or the found element
		}
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
/// Binary Search for both orders.
/// checks borders for out-of-bounds first
/// if returning false, the findpos contains the position 
/// where to insert the new element in the fields
/// uses given X member compare function
/// works on objects and fields likewise since no pointer arithmetic used
///////////////////////////////////////////////////////////////////////////////
template<class X, class L, class Y> bool BinarySearchB(X const (& elem), L const (& list), size_t sz, size_t startpos, size_t& findpos, int (Y::*cmp)(const X&) const, bool asc=true)
{	// do a binary search
	// make some initial stuff
	bool ret = false;
	size_t a= (startpos>=sz) ? 0 : startpos;
	size_t b= sz-1;
	size_t c;

	if( sz < 1)
	{
		findpos = 0;
		ret = false;
	}
	else if( asc )
	{	//smallest element first
		if( 0<=(list[a].*cmp)(elem) )
		{	
			if( 0 == (list[a].*cmp)(elem) ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( 0 >= (list[b].*cmp)(elem) )
		{	
			if( 0 == (list[b].*cmp)(elem) )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( 0 == (list[c].*cmp)(elem) )
				{
					b=c;
					ret = true;
					break;
				}
				else if( 0 < (list[c].*cmp)(elem) )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next smaller element to the given or the found element
		}
	}
	else // descending
	{	//smallest element last
		if( 0 >= (list[a].*cmp)(elem) )
		{	
			if( 0 == (list[a].*cmp)(elem) ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( 0 <= (list[b].*cmp)(elem) )
		{	
			if( 0 == (list[b].*cmp)(elem) )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( 0 == (list[c].*cmp)(elem) )
				{	b=c;
					ret = true;
					break;
				}
				else if( 0 > (list[c].*cmp)(elem) )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next larger element to the given or the found element
		}
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/// Binary Search for both orders.
/// checks borders for out-of-bounds first
/// if returning false, the findpos contains the position 
/// where to insert the new element in the fields
/// uses given external compare function
/// works on objects and fields likewise since no pointer arithmetic used
///////////////////////////////////////////////////////////////////////////////
template<class X, class L, class Y> bool BinarySearchC( X const (& elem), L const (& list), size_t sz, size_t startpos, size_t& findpos, int (*cmp)(const X&, const Y&), bool asc=true)
{	// do a binary search
	// make some initial stuff
	bool ret = false;
	size_t a= (startpos>=sz) ? 0 : startpos;
	size_t b= sz-1;
	size_t c;

	if( sz < 1)
	{
		findpos = 0;
		ret = false;
	}
	else if( asc )
	{	//smallest element first
		if( 0>=(*cmp)(elem, list[a]) )
		{	
			if( 0 == (*cmp)(elem, list[a]) ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( 0 <= (*cmp)(elem, list[b]) )
		{	
			if( 0 == (*cmp)(elem, list[b]) )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( 0 == (*cmp)(elem, list[c]) )
				{
					b=c;
					ret = true;
					break;
				}
				else if( 0 > (*cmp)(elem, list[c]) )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next smaller element to the given or the found element
		}
	}
	else // descending
	{	//smallest element last
		if( 0 <= (*cmp)(elem, list[a]) )
		{	
			if( 0 == (*cmp)(elem, list[a]) ) 
			{
				findpos=a;
				ret = true;
			}
			else
			{
				findpos = a;
				ret = false;
			}
		}
		else if( 0 >= (*cmp)(elem, list[b]) )
		{	
			if( 0 == (*cmp)(elem, list[b]) )
			{
				findpos = b;
				ret = true;
			}
			else
			{
				findpos = b+1;
				ret = false;
			}
		}
		else
		{	// binary search
			do
			{
				c=(a+b)/2;
				if( 0 == (*cmp)(elem, list[c]) )
				{	b=c;
					ret = true;
					break;
				}
				else if( 0 < (*cmp)(elem, list[c]) )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;//return the next larger element to the given or the found element
		}
	}
	return ret;
}



///////////////////////////////////////////////////////////////////////////////
/// binarysearch used in pointer vectors
bool BinarySearch(const void* elem, const void* list[], size_t sz, size_t startpos, size_t& findpos, int (*cmp)(const void*a, const void*b, bool asc), bool asc=true);










///////////////////////////////////////////////////////////////////////////////
/// assignment optimized move without overwrite. pointer version
/// moves cnt elements starting from spos to tpos
// 0 1 2 3 4 5 6 7 8 9 ->(1,5,2)-> 0 5 6 1 2 3 4 7 8 9
template <class T>
bool elementmove(T array[], size_t sz, size_t tpos, size_t spos, size_t cnt)
{	
	if( tpos+cnt > sz )
		cnt = tpos-sz;

	if( cnt && tpos<sz && spos <sz && ( tpos<spos || tpos>spos+cnt) )
	{
		size_t i;
		T tempobj;
		T *tptr, *sptr, *kptr, *mptr, *tmp;

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
#ifdef CNTCOPIES
int cpcnt=0;
#endif
		tmp = sptr;						// start with first source element
		tempobj = *tmp;					// save the initial element
#ifdef CNTCOPIES
cpcnt++;
#endif
		kptr=tmp;						// set run pointer
		while(i--)
		{							
			mptr = kptr-cnt;			// the target position is cnt elements before
			while(mptr>=tptr)			// go through the length until falling off range
			{
				*kptr=*mptr;			// move elements up
#ifdef CNTCOPIES
cpcnt++;
#endif
				kptr = mptr;			// set the next insert pointer
				mptr-=cnt;				// go to the new element
			}
			// fallen out of the array
			mptr = sptr + (kptr-tptr);	// next insertion element has to come from:
			if(mptr==tmp)				// if we hit the pos where the temp elements belongs
			{
				*kptr = tempobj;		// insert the temp element
#ifdef CNTCOPIES
cpcnt++;
#endif
				tmp++;					// increment
				tempobj = *tmp;			// get the new temp
#ifdef CNTCOPIES
cpcnt++;
#endif
				kptr=tmp;				// set run pointer
			}
			else						// otherwise
			{
				*kptr = *mptr;			// move the selected element down
#ifdef CNTCOPIES
cpcnt++;
#endif
				kptr = mptr;			// set run pointer
			}
		}
#ifdef CNTCOPIES
printf("copies for (%lu->%lu)x%lu = %i", (ulong)spos, (ulong)tpos, (ulong)cnt, cpcnt);
#endif
		return true;
	}
	else
		return false;
}




// predeclaration
template<class T> class vectorinterface;


///////////////////////////////////////////////////////////////////////////////
/// assignment optimized move without overwrite. array version
/// moves cnt elements starting from spos to tpos
// 0 1 2 3 4 5 6 7 8 9 ->(1,5,2)-> 0 5 6 1 2 3 4 7 8 9
template <class T>
bool elementmove(vectorinterface<T>& array, size_t sz, size_t tpos, size_t spos, size_t cnt)
{	
	if( tpos+cnt > sz )
		cnt = tpos-sz;

	if( cnt && tpos<sz && spos <sz && ( tpos<spos || tpos>spos+cnt) )
	{
		size_t i;
		uchar tempobj;
		//T *tptr, *sptr, *kptr, *mptr, *tmp;
		size_t t, t1, s, k, m, p;

		if(tpos<spos)
		{	// downmove	
			t = tpos;
			s = spos;
			i = cnt;
		}
		else
		{	// upmove
			// map it to a downmove				
			t = spos;
			s = spos+cnt;
			i = cnt = tpos-spos-cnt+1;
		}
#ifdef CNTCOPIES
int cpcnt=0;
#endif
		p = s;							// start with first source element
		tempobj = array[p];				// save the initial element
#ifdef CNTCOPIES
cpcnt++;
#endif
		m = k = p;						// set run pointer
		t1 = t+cnt;						// moved compare threshold
		while(i--)
		{							
			
			while(m>=t1)				// go through the length until falling off range
			{

				m = k-cnt;				// the target position is cnt elements before
				array[k]=array[m];		// move elements up
#ifdef CNTCOPIES
cpcnt++;
#endif
				k = m;					// set the next insert pointer
			}
			// fallen out of the array
			m = s + (k-t);				// next insertion element has to come from:
			if(m==p)					// if we hit the pos where the temp elements belongs
			{
				array[k] = tempobj;		// insert the temp element
#ifdef CNTCOPIES
cpcnt++;
#endif
				p++;					// increment
				tempobj = array[p];		// get the new temp
#ifdef CNTCOPIES
cpcnt++;
#endif
				m = k = p;				// set run pointer
			}
			else						// otherwise
			{
				array[k] = array[m];	// move the selected element down
#ifdef CNTCOPIES
cpcnt++;
#endif
				k = m;					// set run pointer
			}
		}
#ifdef CNTCOPIES
printf("copies for (%lu->%lu)x%lu = %i", (ulong)spos, (ulong)tpos, (ulong)cnt, cpcnt);
#endif
		return true;
	}
	else
		return false;
}




///////////////////////////////////////////////////////////////////////////////
/// partial copy from baserandom, put back when possible

class polygen
{
public:
	void shuffle_02()
	{	// c(x) = x^2 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x0004)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x0003;
	}
	void shuffle_03()
	{	// c(x) = x^3 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x0008)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x0007;
	}
	void shuffle_04()
	{	// c(x) = x^4 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x0010)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x000F;
	}
	void shuffle_05()
	{	// c(x) = x^5 + x^2 + 1
		poly <<= 1;
		if( ((poly&0x0020)!=0) ^ ((poly&0x0004)!=0) )
			poly |= 1;
		poly &= 0x001F;
	}
	void shuffle_06()
	{	// c(x) = x^6 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x0040)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x003F;
	}
	void shuffle_07()
	{	// c(x) = x^7 + x^3 + 1
		poly <<= 1;
		if( ((poly&0x0080)!=0) ^ ((poly&0x0008)!=0) )
			poly |= 1;
		poly &= 0x007F;
	}
	void shuffle_08()
	{	// c(x) = x^8 + x^4 + x^3 + x^2 + 1
		poly <<= 1;
		if( ((poly&0x0100)!=0) ^ ((poly&0x0010)!=0) ^ ((poly&0x0008)!=0) ^ ((poly&0x0004)!=0) )
			poly |= 1;
		poly &= 0x00FF;
	}
	void shuffle_09()
	{	// c(x) = x^9 + x^4 + 1
		poly <<= 1;
		if( ((poly&0x0200)!=0) ^ ((poly&0x0010)!=0) )
			poly |= 1;
		poly &= 0x01FF;
	}
	void shuffle_10()
	{	// c(x) = x^10 + x^3 + 1
		poly <<= 1;
		if( ((poly&0x0400)!=0) ^ ((poly&0x0008)!=0) )
			poly |= 1;
		poly &= 0x03FF;
	}
	void shuffle_11()
	{	// c(x) = x^11 + x^2 + 1
		poly <<= 1;
		if( ((poly&0x0800)!=0) ^ ((poly&0x0004)!=0) )
			poly |= 1;
		poly &= 0x07FF;
	}
	void shuffle_12()
	{	// c(x) = x^12 + x^6 + x^4 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x1000)!=0) ^ ((poly&0x0040)!=0) ^ ((poly&0x0010)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x0FFF;
	}
	void shuffle_13()
	{	// c(x) = x^13 + x^12 + x^9 + x^3 + 1
		poly <<= 1;
		if( ((poly&0x2000)!=0) ^ ((poly&0x1000)!=0) ^ ((poly&0x0200)!=0) ^ ((poly&0x0008)!=0) )
			poly |= 1;
		poly &= 0x1FFF;
	}
	void shuffle_14()
	{	// c(x) = x^14 + x^12 + x^11 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x4000)!=0) ^ ((poly&0x1000)!=0) ^ ((poly&0x0800)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x3FFF;
	}
	void shuffle_15()
	{	// c(x) = x^158 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x8000)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0x7FFF;
	}
	void shuffle_16()
	{	// c(x) = x^16 + x^12 + x^3 + x^1 + 1
		poly <<= 1;
		if( ((poly&0x10000)!=0) ^ ((poly&0x1000)!=0) ^ ((poly&0x0008)!=0) ^ ((poly&0x0002)!=0) )
			poly |= 1;
		poly &= 0xFFFF;
	}
private:
	typedef void (polygen::*shuffle_func)(void);


	shuffle_func fShuffle;
	uint32 poly;
public:

	polygen(shuffle_func s) : fShuffle(s), poly(rand())
	{
		if(!poly) poly=0;
	}
	//polygen(int width);

	int shuffle()
	{
		(this->*fShuffle)();
		return poly;
	}

	operator int()		{ return poly; }
	int operator()()	{ return poly; }
};







NAMESPACE_END(basics)

#endif//__BASEALGORITHM_H__



