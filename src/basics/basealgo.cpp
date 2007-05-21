
#include "basetypes.h"
#include "basealgo.h"
#include "basearray.h"


NAMESPACE_BEGIN(basics)

#ifdef DEBUG
#define CNTCOPIES	// debug option
#endif




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)




class aaaa
{
	char val;
public:
	aaaa()	{}
	aaaa(int i):val(i)	{}

	int compare(const int& a) const { return val-a; }



	struct cmp
	{
		int operator()(const aaaa& a, const char&b) const
		{
			return a.val-(int)((unsigned char)b);
		}
	};
	friend struct cmp;
};




#endif//DEBUG


void test_algo(int scale)
{
#if defined(DEBUG)

	char buf1[10] = {0,1,2,3,5,6,7,8};
	aaaa buf2[10] = {0,1,2,3,5,6,7,8};
	int a = 4;
	size_t findpos;

	BinarySearch(a, buf1, 8, 0, findpos);
	BinarySearch(7, buf2, 8, 0, findpos, aaaa::cmp());


	a = findpos;

	uchar array[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	vector<uchar> ta(array,20);
	size_t k,sz = 20;

	size_t tpos=1, spos=5, cnt=8;

	elementmove(array,20,tpos,spos,cnt);
	elementmove(ta.begin(), ta.size(), tpos,spos,cnt);

	for(k=0; k<sz; ++k)
		printf("%i ", array[k]);
	printf("\n");

	for(k=0; k<sz; ++k)
		printf("%i ", ta[k]);
	printf("\n");

#endif//DEBUG
}


NAMESPACE_END(basics)
