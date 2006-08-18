
#include "basetypes.h"
#include "basealgo.h"
#include "basearray.h"


NAMESPACE_BEGIN(basics)

#ifdef DEBUG
#define CNTCOPIES	// debug option
#endif




///////////////////////////////////////////////////////////////////////////////
// Binary Search used in pointer vectors
bool BinarySearch(const void* elem, const void* list[], size_t sz, size_t startpos, size_t& findpos, int (*cmp)(const void* a, const void* b, bool asc), bool asc)
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
	else
	{	
		if( 0>=(*cmp)(elem, list[a], asc) )
		{	
			if( 0 == (*cmp)(elem, list[a], asc) ) 
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
		else if( 0 <= (*cmp)(elem, list[b], asc) )
		{	
			if( 0 == (*cmp)(elem, list[b], asc) )
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
				if( 0 == (*cmp)(elem, list[c], asc) )
				{
					b=c;
					ret = true;
					break;
				}
				else if( 0 > (*cmp)(elem, list[c], asc) )
					b=c;
				else
					a=c;
			} while( (a+1) < b );
			findpos = b;
		}
	}
	return ret;
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)

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

	char buf1[10] = {0,1,2,3,5,6,7,8};
	aaaa buf2[10] = {0,1,2,3,5,6,7,8};
	int a = 4;
	size_t findpos;

	BinarySearchC<int,char*,char>(a, buf1, 8, 0, findpos, cmp);
	BinarySearchB<int,aaaa*,aaaa>(7, buf2, 8, 0, findpos, &aaaa::compare);


	a = findpos;

	uchar array[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	vector<uchar> ta(array,20);
	size_t k,sz = 20;

	size_t tpos=1, spos=5, cnt=8;

	elementmove(array,20,tpos,spos,cnt);
	elementmove(ta, 20, tpos,spos,cnt);

	for(k=0; k<sz; ++k)
		printf("%i ", array[k]);
	printf("\n");

	for(k=0; k<sz; ++k)
		printf("%i ", ta[k]);
	printf("\n");

#endif//DEBUG
}


NAMESPACE_END(basics)
