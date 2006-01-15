#ifndef __BASEPOOL_H__
#define __BASEPOOL_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"

template <class T> class TPool : public global, public noncopyable
{
	TslistDST<T*> cListAll;
	TslistDST<T*> cListFree;

public:
	// constructions already create a default T object
	TPool()
	{	// zero parametered objects
		T* obj = new T;
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1> TPool(const P1& p1)
	{	// one parametered objects
		T* obj = new T(p1);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2> TPool(P1& p1, P2& p2)
	{	// two parametered objects
		T* obj = new T(p1,p2);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2, class P3> TPool(P1& p1, P2& p2, P3& p3)
	{	// three parametered objects
		T* obj = new T(p1,p2,p3);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2, class P3, class P4> TPool(P1& p1, P2& p2, P3& p3, P4& p4)
	{	// four parametered objects
		T* obj = new T(p1,p2,p3,p4);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2, class P3, class P4, class P5> TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
	{	// five parametered objects
		T* obj = new T(p1,p2,p3,p4,p5);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2, class P3, class P4, class P5, class P6> TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
	{	// six parametered objects
		T* obj = new T(p1,p2,p3,p4,p5,p6);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<class P1, class P2, class P3, class P4, class P5, class P6, class P7> TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
	{	// seven parametered objects
		T* obj = new T(p1,p2,p3,p4,p5,p6,p7);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	~TPool()
	{	// free the pool elements
		size_t i;
		for(i=0; i<cListAll.size(); i++)
			delete cListAll[i];
	}
	T& aquire()
	{
		T* obj;
		if( cListFree.size() == 0 )
		{	// create a new object using copy operator
			obj = new T(*cListAll[0]);
			cListAll.push(obj);
			
		}
		else
		{	// take a free obj
			obj = cListFree.pop();
		}
		return *obj;
	}
	operator T&()	{ return aquire(); }
	void release(T& elem)
	{	// put the released object to the freelist
		cListFree.push(&elem);
	}
};


#endif//__BASEPOOL_H__
