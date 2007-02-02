#ifndef __BASEPOOL_H__
#define __BASEPOOL_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"
#include "basesafeptr.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "baseexceptions.h"
#include "basearray.h"


NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
/// a pool provides overlapping free usage of objects.
/// usable types need copy constructor
template <typename T>
class TPool : public global, protected Mutex
{
	ICL_EMPTY_COPYCONSTRUCTOR(TPool)
	//TslistDST<T*> cListAll;
	//TslistDST<T*> cListFree;
	ptrvector<T> cListAll;
	ptrvector<T> cListFree;

public:
	// constructions already create a default T object
	TPool()
	{	// zero parametered objects
		T* obj = new T;
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1>
	TPool(P1& p1)
	{	// one parametered objects
		T* obj = new T(p1);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2>
	TPool(P1& p1, P2& p2)
	{	// two parametered objects
		T* obj = new T(p1,p2);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2, typename P3>
	TPool(P1& p1, P2& p2, P3& p3)
	{	// three parametered objects
		T* obj = new T(p1,p2,p3);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2, typename P3, typename P4>
	TPool(P1& p1, P2& p2, P3& p3, P4& p4)
	{	// four parametered objects
		T* obj = new T(p1,p2,p3,p4);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2, typename P3, typename P4, typename P5>
	TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
	{	// five parametered objects
		T* obj = new T(p1,p2,p3,p4,p5);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
	{	// six parametered objects
		T* obj = new T(p1,p2,p3,p4,p5,p6);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TPool(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
	{	// seven parametered objects
		T* obj = new T(p1,p2,p3,p4,p5,p6,p7);
		cListAll.push(obj);
		cListFree.push(obj);
	}
	~TPool()
	{	// free the pool elements
		size_t i;
		for(i=0; i<cListAll.size(); ++i)
			delete cListAll[i];
	}
	T& aquire()
	{
		ScopeLock sl(*this);
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
		ScopeLock sl(*this);
		cListFree.push(&elem);
	}

	void call( void (T::*func)(void))
	{
		ScopeLock sl(*this);
		size_t i;
		for(i=0; i<cListAll.size(); ++i)
		{	
			(cListAll[i]->*func)();
		}
	}
	template<typename X>
	void call( void (T::*func)(X p1), X p1)
	{
		ScopeLock sl(*this);
		size_t i;
		for(i=0; i<cListAll.size(); ++i)
		{	
			(cListAll[i]->*func)(p1);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
/// object for automated aquire/release of pool objects
template<typename T>
class TPoolObj
{
	TPool<T>& cPool;
	T& cObj;
public:
	TPoolObj<T>(TPool<T>& pool) : cPool(pool), cObj( pool.aquire() )	{}
	~TPoolObj<T>()	{ cPool.release(cObj); }

	operator       T&()			{ return cObj; }
	operator const T&() const 	{ return cObj; }
	      T& operator*()		{ return cObj; }
	const T& operator*() const 	{ return cObj; }
	      T* operator->()		{ return &cObj; }
	const T* operator->() const { return &cObj; }
};

NAMESPACE_END(basics)


#endif//__BASEPOOL_H__
