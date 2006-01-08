#ifndef __BASEOBJECTS_H__
#define __BASEOBJECTS_H__

#include "basetypes.h"

//////////////////////////////////////////////////////////////////////////
// basic abstract classes
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// base class to prevent copy/assign
//////////////////////////////////////////////////////////////////////////
class noncopyable 
{
// my gnu ver. 3.4 don't like private constructors
// looks like some gcc really insists on having this members readable
// even if not used and not disturbing in any way, 
// stupid move, but ok, go on, read it 
// but the protected declaration will then generate errors in linker instead of compiler 
// and the source is then harder to find
#ifdef __GNUC__
protected:
#else
private:
#endif
	noncopyable(const noncopyable&);
	const noncopyable& operator= (const noncopyable&);
public:
	noncopyable() {}
	~noncopyable() {}
};


//////////////////////////////////////////////////////////////////////////
// base class for objects that can be counted on global scope
//////////////////////////////////////////////////////////////////////////
class global 
{
private:
#ifdef COUNT_GLOBALS
	static int sGlobalCount;
#ifdef DEBUG
	class _globalcount
	{
	public:
		_globalcount();
		~_globalcount();
	};
	static _globalcount gc;
#endif
#endif

public:
#ifdef COUNT_GLOBALS
	global()				{ atomicincrement(&sGlobalCount); }
	virtual ~global()		{ atomicdecrement(&sGlobalCount); }
	static int getcount()	{ return sGlobalCount; }
	static int correct()	{ atomicdecrement(&sGlobalCount); return sGlobalCount; }
#else
	global()				{ }
	virtual ~global()		{ }
	static int getcount()	{ return 0; }
	static int correct()	{ return 0; }
#endif
};

//////////////////////////////////////////////////////////////////////////
// class for correcting global counter
// in case of instanciation of static objects that might get destructed
// after the global class and thus cause incorrect global object counter
//////////////////////////////////////////////////////////////////////////
class globalrecount
{
public:
#ifdef COUNT_GLOBALS
	globalrecount()		{ global::correct(); }
	~globalrecount()	{ }
#else
	globalrecount()		{ }
	~globalrecount()	{ }
#endif
};




//////////////////////////////////////////////////////////////////////////
// 64bit and user save pointer/number sharing structure
// use with care
//!! currently in common/timer.h
/*
typedef struct _numptr
{
	bool	isptr;
	union{
		void *	ptr;
		ssize_t	num;
	};
	_numptr():isptr(false),ptr(NULL)				{}	// clear as default
	explicit _numptr(void*a):isptr(a!=NULL),ptr(a)	{}	// take over the void pointer, block const pointers here to signal the user
	_numptr(ssize_t a):isptr(false),num(a)			{}	// initialisation from numbers !!DONT CAST POINTERS TO INTS!!
} numptr;
*/



///////////////////////////////////////////////////////////////////////////////
// modifies a value on a scope change (actually on object destruction)
// removes the necessity of writing bunches of value settings before returns
// on spagetti code like frequently found especially here
///////////////////////////////////////////////////////////////////////////////
template <class T> class TScopeChange
{
	T& val;
	T tar;
public:
	TScopeChange(T& v, const T&t) : val(v), tar(t)	{}
	~TScopeChange()			{ val=tar; }
	void disable()			{ tar = val; }
	void set(const T& t)	{ tar = t; }
};



///////////////////////////////////////////////////////////////////////////////
// Singleton Templates
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Simple Singleton for default creatable objects only
///////////////////////////////////////////////////////////////////////////////
template <class T> class TSimpleSingleton
{
public:
	TSimpleSingleton()		{ }
	~TSimpleSingleton()		{ }

	static T& aquire()
	{
		static globalrecount grc;	// for reduceing global counter
		static T singleton;
		return singleton;
	}
	operator T&()	{ return TSimpleSingleton::aquire(); }
};

///////////////////////////////////////////////////////////////////////////////
// Singleton with templated create functions
///////////////////////////////////////////////////////////////////////////////
template <class T> class TSingleton
{
	static T*& getpointer()
	{
		static globalrecount grc;	// for reduceing global counter
		static T*singleton_ptr=NULL;
		return singleton_ptr;
	}
public:
	TSingleton()						
	{
		if( NULL==this->getpointer() )
			this->create();
	}
	TSingleton(const TSingleton&)
	{ }
	template <class P1>
	TSingleton(P1& p1)
	{
		this->create(p1);
	}
	template <class P1, class P2>
	TSingleton(P1& p1, P2& p2)
	{
		this->create(p1,p2);
	}
	template <class P1, class P2, class P3>
	TSingleton(P1& p1, P2& p2, P3& p3)
	{
		this->create(p1,p2,p3);
	}
	template <class P1, class P2, class P3, class P4>
	TSingleton(P1& p1, P2& p2, P3& p3, P4& p4)
	{
		this->create(p1,p2,p3,p4);
	}
	template <class P1, class P2, class P3, class P4, class P5>
	TSingleton(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
	{
		this->create(p1,p2,p3,p4,p5);
	}
	template <class P1, class P2, class P3, class P4, class P5, class P6>
	TSingleton(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
	{
		this->create(p1,p2,p3,p4,p5,p6);
	}
	template <class P1, class P2, class P3, class P4, class P5, class P6, class P7>
	TSingleton(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
	{
		this->create(p1,p2,p3,p4,p5,p6,p7);
	}

	~TSingleton()		{ }
	static T& aquire()
	{
		return *(TSingleton::getpointer());
	}
	operator T&()	{ return this->aquire(); }

	bool isvalid()			{ return NULL!=this->getpointer(); }
	bool destroy()			
	{
		T*& ptr= this->getpointer();
		if( ptr )
		{
			delete ptr;
			ptr=NULL;
			return true;
		}
		return false;
	}
	bool create()
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T;
			return true;
		}
		return false;
	}
	template <class P1>
	bool create(P1& p1)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1);
			return true;
		}
		return false;
	}
	template <class P1, class P2>
	bool create(P1& p1, P2& p2)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2);
			return true;
		}
		return false;
	}
	template <class P1, class P2, class P3>
	bool create(P1& p1, P2& p2, P3& p3)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2,p3);
			return true;
		}
		return false;
	}
	template <class P1, class P2, class P3, class P4>
	bool create(P1& p1, P2& p2, P3& p3, P4& p4)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2,p3,p4);
			return true;
		}
		return false;
	}
	template <class P1, class P2, class P3, class P4, class P5>
	bool create(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2,p3,p4,p5);
			return true;
		}
		return false;
	}
	template <class P1, class P2, class P3, class P4, class P5, class P6>
	bool create(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2,p3,p4,p5,p6);
			return true;
		}
		return false;
	}
	template <class P1, class P2, class P3, class P4, class P5, class P6, class P7>
	bool create(P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7)
	{
		T*& ptr= this->getpointer();
		if( !ptr )
		{
			ptr = new T(p1,p2,p3,p4,p5,p6,p7);
			return true;
		}
		return false;
	}
};


#endif//__BASEOBJECTS_H__

