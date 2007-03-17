#ifndef __BASESAFEPTR_H__
#define __BASESAFEPTR_H__

#include "basetypes.h"
#include "baseobjects.h"

NAMESPACE_BEGIN(basics)

/////////////////////////////////////////////////////////////////////////////////////////
/// basic pointer interface
/////////////////////////////////////////////////////////////////////////////////////////
template <typename X>
class TPtr : public noncopyable
{
protected:
	TPtr()	{}
public:
	virtual ~TPtr()	{}
	virtual const X* get() const = 0;
	const X* pointer() const						{ return this->get(); }

	///////////////////////////////////////////////////////////////////////////
	/// logical tests to see if there is anything contained in the pointer since it can be null
//	operator bool(void) const						{ return this->exists(); }
	bool operator!(void) const						{ return !this->exists(); }
	bool present(void) const						{ return this->exists(); }
	bool null(void) const							{ return !this->exists(); }

	virtual bool exists() const=0;

	virtual const X& readaccess() const = 0;
	virtual X& writeaccess() = 0;

	virtual const X& operator*() const =0;
	virtual const X* operator->() const =0;
	virtual X& operator*() =0;
	virtual X* operator->() =0;
	virtual operator const X&() const =0;

	virtual bool operator ==(void *p) const			{ return p==(void*)this->get(); }
	virtual bool operator !=(void *p) const			{ return p==(void*)this->get(); }
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Pointer Compares
//## I'm not sure if burdening the compares with the stl-reduced-compare-set is a good idea
template<typename T>
bool operator==(const TPtr<T>& left, const TPtr<T>& right)
{
	// a null is not equal to a non-null but equal to another null
	if(!left || !right) return left.pointer() == right.pointer();
	// shortcut - if the two pointers are equal then the objects must be equal
	if (left.pointer() == right.pointer()) return true;
	// otherwise compare the objects themselves
	return  (*left == *right);
}
template<typename T>
bool operator!=(const TPtr<T>& left, const TPtr<T>& right)
{
	// a null is not equal to a non-null but equal to another null
	if(!left || !right) return left.pointer() != right.pointer();
	// shortcut - if the two pointers are equal then the objects must be equal
	if (left.pointer() == right.pointer()) return false;
	// otherwise compare the objects themselves
	return !(*left == *right);
//	return  (*left != *right);
}

template<typename T>
bool operator< (const TPtr<T>& left, const TPtr<T>& right)
{
	// a null pointer is less than a non-null but equal to another null
	if(!left || !right) return left.pointer() < right.pointer();
	// shortcut - if the two pointers are equal then the comparison must be false
	if (left.pointer() == right.pointer()) return false;
	// otherwise, compare the objects
	return (*left < *right);
}
template<typename T>
bool operator<=(const TPtr<T>& left, const TPtr<T>& right)
{
	// a null pointer is less than a non-null but equal to another null
	if(!left || !right) return left.pointer() <= right.pointer();
	// shortcut - if the two pointers are equal then the comparison must be false
	if (left.pointer() == right.pointer()) return true;
	// otherwise, compare the objects
	return (*left < *right) || (*left == *right);
//	return (*left <=*right);
}
template<typename T>
bool operator> (const TPtr<T>& left, const TPtr<T>& right)
{
	// a null pointer is less than a non-null but equal to another null
	if(!left || !right) return left.pointer() > right.pointer();
	// shortcut - if the two pointers are equal then the comparison must be false
	if (left.pointer() == right.pointer()) return false;
	// otherwise, compare the objects
	return !(*left < *right) && !(*left == *right);
//	return  (*left > *right);
}
template<typename T>
bool operator>=(const TPtr<T>& left, const TPtr<T>& right)
{
	// a null pointer is less than a non-null but equal to another null
	if(!left || !right) return left.pointer() >= right.pointer();
	// shortcut - if the two pointers are equal then the comparison must be false
	if (left.pointer() == right.pointer()) return true;
	// otherwise, compare the objects
	return !(*left < *right);
}



/////////////////////////////////////////////////////////////////////////////////////////
/// Simple Auto/Copy-Pointer.
/// it creates a default object on first access if not initialized
/// automatic free on object delete
/// does not use a counting object
/////////////////////////////////////////////////////////////////////////////////////////
template <typename X>
class TPtrAuto : public TPtr<X>
{
private:
	X* itsPtr;
	void create()					{ if(!itsPtr) itsPtr = new X; }
	void copy(const TPtr<X>& p)
	{
		const X* pp=p.get();
		if( this!=&p && this->itsPtr!=pp )
		{
			if(this->itsPtr)
			{
				delete this->itsPtr;
				this->itsPtr = NULL;
			}
			if( pp )
				this->itsPtr = new X(*pp);
		}
	}
public:
	explicit TPtrAuto(X* p = NULL) : itsPtr(p)				{ }
	virtual ~TPtrAuto()										{ if(this->itsPtr) delete this->itsPtr; }

	TPtrAuto(const TPtr<X>& p) : itsPtr(NULL)				{ this->copy(p); }
	const TPtr<X>& operator=(const TPtr<X>& p)				{ this->copy(p); return *this; }
	TPtrAuto(const TPtrAuto<X>& p): itsPtr(NULL)			{ this->copy(p); }
	const TPtr<X>& operator=(const TPtrAuto<X>& p)			{ this->copy(p); return *this; }

	virtual bool exists() const					{ return this->itsPtr!=NULL; }

	virtual const X& readaccess() const			{ const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr; }
	virtual X& writeaccess()					{ const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr; }
	virtual const X* get() const				{ const_cast<TPtrAuto<X>*>(this)->create(); return  this->itsPtr; }
	virtual const X& operator*() const			{ const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr; }
	virtual const X* operator->() const			{ const_cast<TPtrAuto<X>*>(this)->create(); return  this->itsPtr; }
	virtual X& operator*()						{ const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr; }
	virtual X* operator->()						{ const_cast<TPtrAuto<X>*>(this)->create(); return  this->itsPtr; }
	virtual operator const X&() const			{ const_cast<TPtrAuto<X>*>(this)->create(); return *this->itsPtr; }

	virtual bool operator ==(void *p) const { return this->itsPtr==p; }
	virtual bool operator !=(void *p) const { return this->itsPtr!=p; }
};



/////////////////////////////////////////////////////////////////////////////////////////
/// count object.
/// holds pointers to managed objects
template <typename T>
struct CPtrCounter : public noncopyable
{
	T*				ptr;
	unsigned int	count;

	template <typename P1>
	CPtrCounter(const P1& p1)
		: ptr(new T(p1)), count(1)
	{}
private:
	// (re)forbid copy construction
	CPtrCounter(const CPtrCounter& p1);
public:
	template <typename P1, typename P2>
	CPtrCounter(const P1& p1, const P2& p2)
		: ptr(new T(p1,p2)), count(1)
	{}
	template <typename P1, typename P2, typename P3>
	CPtrCounter(const P1& p1, const P2& p2, const P3& p3)
		: ptr(new T(p1,p2,p3)), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	CPtrCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: ptr(new T(p1,p2,p3,p4)), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	CPtrCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: ptr(new T(p1,p2,p3,p4,p5)), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	CPtrCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: ptr(new T(p1,p2,p3,p4,p5,p6)), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	CPtrCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: ptr(new T(p1,p2,p3,p4,p5,p6,p7)), count(1)
	{}

	CPtrCounter() : ptr(NULL), count(1) {}
	//explicit CCounter(T* p) : ptr(p), count(1) {}
	//explicit CCounter(const T& p) : ptr(new T(p)), count(1) {}
	~CPtrCounter()	{ if(ptr) delete ptr; }

	const CPtrCounter& operator=(T* p)
	{	// take ownership of the given pointer
		if(ptr) delete ptr;
		ptr = p;
		return *this;
	}
	CPtrCounter<T>* aquire()
	{
		atomicincrement( &count );
		return this;
	}
	CPtrCounter<T>* release()
	{
		if( atomicdecrement( &count ) == 0 )
		{
			delete this;
		}
		return NULL;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Count-Pointer.
/// pointer copies are counted
/// when reference counter becomes zero the object is automatically deleted
/// if there is no pointer, it will return NULL
template <typename X>
class TPtrCount : public TPtr<X>
{
protected:
	CPtrCounter<X>* cCntObj;
	void acquire(const TPtrCount<X>& r)
	{	// check if not already pointing to the same object
		if( this->cCntObj != r.cCntObj )
		{	// save the current pointer
			CPtrCounter<X> *old = this->cCntObj;
			// aquite and increment the given pointer
			this->cCntObj = (r.cCntObj)?r.cCntObj->aquire():NULL;
			// release the old thing
			if(old) old->release();
		}
		if( NULL==this->cCntObj )
		{	// new empty counter to link the pointers
			this->cCntObj = new CPtrCounter<X>();
			const_cast<TPtrCount&>(r).cCntObj = this->cCntObj;
			this->cCntObj->aquire();
		}
	}
	void release()
	{	// decrement the count, clear the handle
		if(this->cCntObj) cCntObj = cCntObj->release();
	}
	void checkobject()
	{	// check if we have an object to access, create one if not
		if(!this->cCntObj)
			this->cCntObj = new CPtrCounter<X>();
		// usable objects need a default constructor
		if(!this->cCntObj->ptr)
			this->cCntObj->ptr = new X; 
	}

public:
	template <typename P1>
	TPtrCount<X>(const P1& p1)
		: cCntObj(NULL)
	{
		 this->cCntObj=new CPtrCounter<X>(p1);
	}
	template <typename P1, typename P2>
	TPtrCount<X>(const P1& p1, const P2& p2)
		: cCntObj( new CPtrCounter<X>(p1,p2) )
	{}
	template <typename P1, typename P2, typename P3>
	TPtrCount<X>(const P1& p1, const P2& p2, const P3& p3)
		: cCntObj( new CPtrCounter<X>(p1,p2,p3) )
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TPtrCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: cCntObj( new CPtrCounter<X>(p1,p2,p3,p4) )
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TPtrCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: cCntObj( new CPtrCounter<X>(p1,p2,p3,p4,p5) )
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TPtrCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: cCntObj( new CPtrCounter<X>(p1,p2,p3,p4,p5,p6) )
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TPtrCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: cCntObj( new CPtrCounter<X>(p1,p2,p3,p4,p5,p6,p7) )
	{}

	TPtrCount<X>() : cCntObj(NULL)
	{ }
	explicit TPtrCount<X>(X* p) : cCntObj(p?new CPtrCounter<X>(p):NULL)
	{ }
	TPtrCount<X>(const TPtrCount<X>& r) : cCntObj(NULL)
	{
		this->acquire(r);
	}
	virtual ~TPtrCount<X>()	
	{
		this->release();
	}
	const TPtrCount<X>& operator=(const TPtrCount<X>& r)
	{
		this->acquire(r);
		return *this;
	}
	TPtrCount<X>& operator=(X* p)
	{	// take ownership of the given pointer
		if( this->cCntObj && this->is_unique() )
		{
			*this->cCntObj = p;
		}
		else
		{
			this->release();
			this->cCntObj = new CPtrCounter<X>();//(p);
			this->cCntObj->ptr=p;
		}
		return *this;
	}

	size_t getRefCount() const					{ return (this->cCntObj) ? this->cCntObj->count : 1;}
	bool clear()								{ this->release(); return this->cCntObj==NULL; }
	bool is_unique() const						{ return (this->cCntObj ? (cCntObj->count == 1):true);}
	bool make_unique()		
	{
		if( !is_unique() )
		{
			CPtrCounter<X> *cnt = new CPtrCounter<X>();
			// copy the object if exist
			if(this->cCntObj && this->cCntObj->ptr)
			{
				cnt->ptr = new X(*(this->cCntObj->ptr));
			}
			this->release();
			this->cCntObj = cnt;
		}
		return true;
	}
	virtual const X& readaccess() const			{ return *this->cCntObj->ptr; }
	virtual X& writeaccess()					{ return *this->cCntObj->ptr; }

	virtual bool exists() const					{ return NULL!=this->cCntObj && NULL!=this->cCntObj->ptr; }
	virtual const X* get() const				{ return this->cCntObj ?  this->cCntObj->ptr : 0; }
	virtual const X& operator*() const			{ return *this->cCntObj->ptr; }
	virtual const X* operator->() const			{ return this->cCntObj ?  this->cCntObj->ptr : 0; }
	virtual X& operator*()						{ return *this->cCntObj->ptr; }
	virtual X* operator->()						{ return this->cCntObj ?  this->cCntObj->ptr : 0; }
	virtual operator const X&() const			{ return *this->cCntObj->ptr; }

	virtual bool operator ==(void *p) const		{ return (this->cCntObj) ? (this->cCntObj->ptr==p) : (this->cCntObj==p); }
	virtual bool operator !=(void *p) const		{ return (this->cCntObj) ? (this->cCntObj->ptr!=p) : (this->cCntObj!=p); }

	const TPtrCount<X>& create ()
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>();
		return *this;
	}
	template <typename P1>
	const TPtrCount<X>& create (const P1& p1)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1);
		return *this;
	}
	template <typename P1, typename P2>
	const TPtrCount<X>& create (const P1& p1, const P2& p2)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2);
		return *this;
	}
	template <typename P1, typename P2, typename P3>
	const TPtrCount<X>& create (const P1& p1, const P2& p2, const P3& p3)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2,p3);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4>
	const TPtrCount<X>& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2,p3,p4);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const TPtrCount<X>& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2,p3,p4,p5);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const TPtrCount<X>& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2,p3,p4,p5,p6);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const TPtrCount<X>& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
	{
		this->clear();
		this->cCntObj = new CPtrCounter<X>(p1,p2,p3,p4,p5,p6,p7);
		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Count-Pointer.
/// pointer copies are counted
/// when reference counter becomes zero the object is automatically deleted
/// creates a default object if not exist
/////////////////////////////////////////////////////////////////////////////////////////
template <typename X>
class TPtrAutoCount : public TPtrCount<X>
{
public:
	template <typename P1>
	TPtrAutoCount<X>(const P1& p1)
		: TPtrCount<X>(p1)
	{}
	template <typename P1, typename P2>
	TPtrAutoCount<X>(const P1& p1, const P2& p2)
		: TPtrCount<X>(p1,p2)
	{}
	template <typename P1, typename P2, typename P3>
	TPtrAutoCount<X>(const P1& p1, const P2& p2, const P3& p3)
		: TPtrCount<X>(p1,p2,p3)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TPtrAutoCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: TPtrCount<X>(p1,p2,p3,p4)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TPtrAutoCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: TPtrCount<X>(p1,p2,p3,p4,p5)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TPtrAutoCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TPtrAutoCount<X>(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6,p7)
	{}

	explicit TPtrAutoCount<X>(X* p) : TPtrCount<X>(p)				{}
	TPtrAutoCount<X>()												{}
	virtual ~TPtrAutoCount<X>()										{}
	TPtrAutoCount<X>(const TPtrCount<X>& r) : TPtrCount<X>(r)		{}
	const TPtrAutoCount<X>& operator=(const TPtrCount<X>& r)		{ this->acquire(r); return *this; }
	TPtrAutoCount<X>(const TPtrAutoCount<X>& r) : TPtrCount<X>(r)	{}
	const TPtrAutoCount<X>& operator=(const TPtrAutoCount<X>& r)	{ this->acquire(r); return *this; }

	virtual const X& readaccess() const			{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return *this->cCntObj->ptr; }
	virtual X& writeaccess()					{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return *this->cCntObj->ptr; }
	virtual const X* get() const				{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual const X& operator*() const			{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return *this->cCntObj->ptr; }
	virtual const X* operator->() const			{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual X& operator*()						{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return *this->cCntObj->ptr; }
	virtual X* operator->()						{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual operator const X&() const			{ const_cast<TPtrAutoCount<X>*>(this)->checkobject(); return *this->cCntObj->ptr; }
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Count-Auto-Pointer with copy-on-write scheme.
/// pointer copies are counted
/// when reference counter becomes zero the object is automatically deleted
/// creates a default object if not exist
/////////////////////////////////////////////////////////////////////////////////////////
template <typename X>
class TPtrAutoRef : public TPtrCount<X>
{
public:
	template <typename P1>
	TPtrAutoRef(const P1& p1)
		: TPtrCount<X>(p1)
	{}
	template <typename P1, typename P2>
	TPtrAutoRef(const P1& p1, const P2& p2)
		: TPtrCount<X>(p1,p2)
	{}
	template <typename P1, typename P2, typename P3>
	TPtrAutoRef(const P1& p1, const P2& p2, const P3& p3)
		: TPtrCount<X>(p1,p2,p3)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TPtrAutoRef(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: TPtrCount<X>(p1,p2,p3,p4)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TPtrAutoRef(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: TPtrCount<X>(p1,p2,p3,p4,p5)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TPtrAutoRef(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TPtrAutoRef(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6,p7)
	{}

	explicit TPtrAutoRef(X* p) : TPtrCount<X>(p)		{}
	TPtrAutoRef()										{}
	virtual ~TPtrAutoRef()								{}
	TPtrAutoRef(const TPtrCount<X>& r) : TPtrCount<X>(r){}
	const TPtrAutoRef& operator=(const TPtrCount<X>& r)	{ this->acquire(r); return *this; }
	TPtrAutoRef(const TPtrAutoRef& r) : TPtrCount<X>(r)	{}
	const TPtrAutoRef& operator=(const TPtrAutoRef& r)	{ this->acquire(r); return *this; }

	virtual const X& readaccess() const	
	{ 
		const_cast< TPtrAutoRef* >(this)->checkobject();	
		// no need to aquire, is done on reference creation
		return *this->cCntObj->ptr;
	}
	virtual X& writeaccess()
	{
		this->checkobject();
		this->make_unique();
		// no need to aquire, is done on reference creation
		return *this->cCntObj->ptr;
	}
	virtual const X* get() const					{ this->readaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual const X& operator*() const				{ return this->readaccess(); }
	virtual const X* operator->() const				{ this->readaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual X& operator*()							{ return this->writeaccess();}
	virtual X* operator->()							{ this->writeaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual operator const X&() const				{ return this->readaccess(); }
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Pointer with loadable scheme.
/////////////////////////////////////////////////////////////////////////////////////////
enum POINTER_TYPE
{
	AUTOCOUNT,	// counting and create-on-access
	AUTOREF		// counting, create-on-access and copy-on-write (default)
};

template <typename X>
class TPtrCommon : public TPtrCount<X>
{
	mutable bool cAutoRef : 1;
public:
	template <typename P1>
	TPtrCommon(const P1& p1) : TPtrCount<X>(p1)
	{ }
	template <typename P1, typename P2>
	TPtrCommon(const P1& p1, const P2& p2)
		: TPtrCount<X>(p1,p2)
	{}
	template <typename P1, typename P2, typename P3>
	TPtrCommon(const P1& p1, const P2& p2, const P3& p3)
		: TPtrCount<X>(p1,p2,p3)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: TPtrCount<X>(p1,p2,p3,p4)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: TPtrCount<X>(p1,p2,p3,p4,p5)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: TPtrCount<X>(p1,p2,p3,p4,p5,p6,p7)
	{}

	explicit TPtrCommon(X* p) : TPtrCount<X>(p), cAutoRef(true)			{}
	TPtrCommon() : cAutoRef(true)										{}
	virtual ~TPtrCommon()												{}
	TPtrCommon(const TPtrCount<X>& r) : TPtrCount<X>(r), cAutoRef(true)	{}
	const TPtrCommon& operator=(const TPtrCount<X>& r)					{ this->acquire(r); return *this; }
	TPtrCommon(const TPtrCommon& r) : TPtrCount<X>(r), cAutoRef(true)	{}
	const TPtrCommon& operator=(const TPtrCommon& r)					{ this->acquire(r); return *this; }

	void setaccess(POINTER_TYPE pt)	const
	{ 
		this->cAutoRef = (pt == AUTOREF);
	}
	bool isReference() const	{ return cAutoRef; }

	virtual const X& readaccess() const
	{ 
		const_cast< TPtrCommon<X>* >(this)->checkobject();	
		// no need to aquire, is done on reference creation
		return *this->cCntObj->ptr;
	}
	virtual X& writeaccess()
	{
		this->checkobject();
		if(cAutoRef) this->make_unique();
		// no need to aquire, is done on reference creation
		return *this->cCntObj->ptr;
	}
	virtual const X* get() const					{ readaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual const X& operator*() const				{ return readaccess(); }
	virtual const X* operator->() const				{ readaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual X& operator*()							{ return writeaccess();}
	virtual X* operator->()							{ writeaccess(); return this->cCntObj ? this->cCntObj->ptr : NULL; }
	virtual operator const X&() const				{ return readaccess(); }
};



/////////////////////////////////////////////////////////////////////////////////////////
/// count object.
/// holds objects
template <typename X>
struct CObjCounter : public noncopyable
{
	X				obj;
	unsigned int	count;

	CObjCounter() : count(1)
	{}
	template <typename P1>
	CObjCounter(const P1& p1)
		: obj(p1), count(1)
	{}
private:
	// (re)forbid copy construction
	CObjCounter(const CObjCounter& p1);
public:
	CObjCounter(const X& x)
		: obj(x), count(1)
	{}
	template <typename P1, typename P2>
	CObjCounter(const P1& p1, const P2& p2)
		: obj(p1,p2), count(1)
	{}
	template <typename P1, typename P2, typename P3>
	CObjCounter(const P1& p1, const P2& p2, const P3& p3)
		: obj(p1,p2,p3), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	CObjCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: obj(p1,p2,p3,p4), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	CObjCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: obj(p1,p2,p3,p4,p5), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	CObjCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: obj(p1,p2,p3,p4,p5,p6), count(1)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	CObjCounter(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: obj(p1,p2,p3,p4,p5,p6,p7), count(1)
	{}

	~CObjCounter()
	{}

	operator X&()				{ return obj; }
	operator const X&() const	{ return obj; }

	CObjCounter* aquire()
	{
		atomicincrement( &count );
		return this;
	}
	CObjCounter* release()
	{
		if( atomicdecrement( &count ) == 0 )
		{
			delete this;
		}
		return NULL;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/// smart pointer with embedded data.
/// 
/// TObjPtr implements copy-on-write.
/// TObjPtrCount implements autocreate.
/// TObjPtrCommon is switchable.
///
template <typename X>
class TObjPtr : public TPtr<X>
{
protected:
	CObjCounter<X>* cCntObj;

	void acquire(const TObjPtr& r)
	{	// check if not already pointing to the same object
		if( this->cCntObj != r.cCntObj )
		{	// save the current pointer
			CObjCounter<X> *old = this->cCntObj;
			// aquite and increment the given pointer
			this->cCntObj = (r.cCntObj)?r.cCntObj->aquire():NULL;
			// release the old thing
			if(old) old->release();
		}
		if( NULL==this->cCntObj )
		{	// new empty counter to link the pointers
			this->cCntObj = new CObjCounter<X>();
			const_cast<TObjPtr&>(r).cCntObj = this->cCntObj;
			this->cCntObj->aquire();
		}
	}
	void release()
	{	// decrement the count, clear the handle
		if(this->cCntObj) cCntObj = cCntObj->release();
	}
	void checkobject()
	{	// check if we have an object to access, create one if not
		// usable objects here need a default constructor
		if(!this->cCntObj)
			this->cCntObj = new CObjCounter<X>();
	}

	virtual X& autocreate() const
	{ 
		(const_cast< TObjPtr* >(this))->checkobject();	
		// no need to aquire, is done on reference creation
		return this->cCntObj->obj;
	}
	virtual X& copyonwrite() const
	{
		(const_cast< TObjPtr* >(this))->checkobject();
		(const_cast< TObjPtr* >(this))->make_unique();
		// no need to aquire, is done on reference creation
		return this->cCntObj->obj;
	}

public:
	TObjPtr()
		: cCntObj(NULL)
	{}
//	template <typename P1>
//	TObjPtr(const P1& p1) : cCntObj(NULL)
//	{
//		this->cCntObj=new CObjCounter<X>(p1);
//	}
	TObjPtr(const TObjPtr& r)
		: cCntObj(NULL)
	{
		this->acquire(r);
	}
	TObjPtr(const X& x) : cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(x);
	}
	template <typename P1, typename P2>
	TObjPtr(const P1& p1, const P2& p2)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2);
	}
	template <typename P1, typename P2, typename P3>
	TObjPtr(const P1& p1, const P2& p2, const P3& p3)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2,p3);
	}
	template <typename P1, typename P2, typename P3, typename P4>
	TObjPtr(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2,p3,p4);
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TObjPtr(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2,p3,p4,p5);
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TObjPtr(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2,p3,p4,p5,p6);
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TObjPtr(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: cCntObj(NULL)
	{
		this->cCntObj=new CObjCounter<X>(p1,p2,p3,p4,p5,p6,p7);
	}

	virtual ~TObjPtr()	
	{
		this->release();
	}
	const TObjPtr& operator=(const TObjPtr& r)
	{
		this->acquire(r);
		return *this;
	}
	TObjPtr& operator=(const X& x)
	{	
		if( this->cCntObj )
			this->writeaccess() = x;
		else
			this->cCntObj = new CObjCounter<X>(x);
		return *this;
	}

	size_t getRefCount() const						{ return (this->cCntObj) ? this->cCntObj->count : 1;}
	bool clear()									{ this->release(); return this->cCntObj==NULL; }
	
	bool is_unique() const							{ return (this->cCntObj ? (cCntObj->count == 1):true);}
	bool make_unique()
	{
		if( !this->is_unique() )
		{	// there is an object and the refeence counter is >1
			CObjCounter<X> *cnt = new CObjCounter<X>(this->cCntObj->obj);
			// copy the object if exist
			this->release();
			this->cCntObj = cnt;
		}
		return true;
	}

	virtual bool isCopyonWrite() const				{ return true; }
	virtual bool isAutoCreate() const				{ return false; }
	virtual void setCopyonWrite() const				{ }
	virtual void setAutoCreate() const				{ }

	virtual const X& readaccess() const				{ return this->autocreate(); }
	virtual X& writeaccess()						{ return this->copyonwrite(); }

	virtual bool exists() const						{ return NULL!=this->cCntObj; }
	virtual const X* get() const					{ this->readaccess(); return this->cCntObj ? &this->cCntObj->obj : NULL; }
	virtual const X& operator*() const				{ return this->readaccess(); }
	virtual const X* operator->() const				{ this->readaccess(); return this->cCntObj ? &this->cCntObj->obj : NULL; }
	virtual X& operator*()							{ return this->writeaccess();}
	virtual X* operator->()							{ this->writeaccess(); return this->cCntObj ? &this->cCntObj->obj : NULL; }
	virtual operator const X&() const				{ return this->readaccess(); }

	virtual bool operator ==(void *p) const			{ return this->cCntObj==p; }
	virtual bool operator !=(void *p) const			{ return this->cCntObj!=p; }

	const TObjPtr& create ()
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>( );
		return *this;
	}
	template <typename P1>
	const TObjPtr& create (const P1& p1)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>( p1 );
		return *this;
	}
	template <typename P1, typename P2>
	const TObjPtr& create (const P1& p1, const P2& p2)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2);
		return *this;
	}
	template <typename P1, typename P2, typename P3>
	const TObjPtr& create (const P1& p1, const P2& p2, const P3& p3)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2,p3);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4>
	const TObjPtr& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2,p3,p4);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	const TObjPtr& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2,p3,p4,p5);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	const TObjPtr& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2,p3,p4,p5,p6);
		return *this;
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	const TObjPtr& create (const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
	{
		this->clear();
		this->cCntObj = new CObjCounter<X>(p1,p2,p3,p4,p5,p6,p7);
		return *this;
	}
};


template <typename X>
class TObjPtrCount : public TObjPtr<X>
{
public:
	TObjPtrCount()
	{}
//	template <typename P1>
//	TObjPtrCount(const P1& p1)
//		: TObjPtr<X>(p1)
//	{}
	TObjPtrCount(const TObjPtrCount& r)
	{
		this->acquire(r);
	}
	TObjPtrCount(const TObjPtr<X>& r)
	{
		this->acquire(r);
	}
	TObjPtrCount(const X& p) : TObjPtr<X>(p)
	{}
	template <typename P1, typename P2>
	TObjPtrCount(const P1& p1, const P2& p2)
		: TObjPtr<X>(p1,p2)
	{}
	template <typename P1, typename P2, typename P3>
	TObjPtrCount(const P1& p1, const P2& p2, const P3& p3)
		: TObjPtr<X>(p1,p2,p3)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TObjPtrCount(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: TObjPtr<X>(p1,p2,p3,p4)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TObjPtrCount(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: TObjPtr<X>(p1,p2,p3,p4,p5)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TObjPtrCount(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: TObjPtr<X>(p1,p2,p3,p4,p5,p6)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TObjPtrCount(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: TObjPtr<X>(p1,p2,p3,p4,p5,p6,p7)
	{}

	virtual ~TObjPtrCount()	
	{}

	const TObjPtrCount& operator=(const TObjPtrCount& r)
	{
		this->acquire(r);
		return *this;
	}
	const TObjPtrCount& operator=(const TObjPtr<X>& r)
	{
		this->acquire(r);
		return *this;
	}
	TObjPtrCount& operator=(const X& x)
	{	
		this->TObjPtr<X>::operator=(x);
		return *this;
	}

	virtual bool isCopyonWrite() const	{ return false; }
	virtual bool isAutoCreate() const	{ return true; }
	virtual void setCopyonWrite() const	{  }
	virtual void setAutoCreate() const	{  }

	virtual const X& readaccess() const	{ return this->TObjPtrCount<X>::autocreate(); }
	virtual X& writeaccess()			{ return this->TObjPtrCount<X>::autocreate(); }
};



template <typename X>
class TObjPtrCommon : public TObjPtr<X>
{
protected:
	mutable X& (TObjPtr<X>::*fAccess)(void) const;

public:
		
	TObjPtrCommon()
		: fAccess(&TObjPtrCommon<X>::copyonwrite)	// access protected function through TObjPtrCommon
	{}
//	template <typename P1>
//	TObjPtrCommon(const P1& p1)
//		: TObjPtr<X>(p1), fAccess(&TObjPtr<X>::copyonwrite)
//	{}
	TObjPtrCommon(const TObjPtrCommon& r)
		: fAccess(r.fAccess)
	{
		this->acquire(r);
	}
	TObjPtrCommon(const TObjPtr<X>& r)
		: fAccess(&TObjPtr<X>::copyonwrite)
	{
		this->acquire(r);
	}
	TObjPtrCommon(const TObjPtrCount<X>& r)
		: fAccess(&TObjPtr<X>::autocreate)
	{
		this->acquire(r);
	}
	TObjPtrCommon(const X& x)
		: TObjPtr<X>(x), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2>
	TObjPtrCommon(const P1& p1, const P2& p2)
		: TObjPtr<X>(p1,p2), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2, typename P3>
	TObjPtrCommon(const P1& p1, const P2& p2, const P3& p3)
		: TObjPtr<X>(p1,p2,p3), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2, typename P3, typename P4>
	TObjPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: TObjPtr<X>(p1,p2,p3,p4), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	TObjPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5)
		: TObjPtr<X>(p1,p2,p3,p4,p5), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	TObjPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6)
		: TObjPtr<X>(p1,p2,p3,p4,p5,p6), fAccess(&TObjPtr<X>::copyonwrite)
	{}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	TObjPtrCommon(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6, const P7& p7)
		: TObjPtr<X>(p1,p2,p3,p4,p5,p6,p7), fAccess(&TObjPtr<X>::copyonwrite)
	{}

	explicit TObjPtrCommon(bool cpwr)
		: fAccess(cpwr?&TObjPtrCommon<X>::copyonwrite:&TObjPtrCommon<X>::autocreate)
	{}
	explicit TObjPtrCommon(const TObjPtr<X>& r, bool cpwr)
		: fAccess(cpwr?&TObjPtrCommon<X>::copyonwrite:&TObjPtrCommon<X>::autocreate)
	{
		this->acquire(r);
	}
	TObjPtrCommon(const X& p, bool cpwr)
		: TObjPtr<X>(p), fAccess(cpwr?&TObjPtrCommon::copyonwrite:&TObjPtrCommon::autocreate)
	{}

	virtual ~TObjPtrCommon()	
	{}

	const TObjPtrCommon& operator=(const TObjPtrCommon& r)
	{
		this->fAccess = r.fAccess;
		this->acquire(r);
		return *this;
	}
	const TObjPtrCommon& operator=(const TObjPtr<X>& r)
	{
		this->fAccess = &TObjPtrCommon<X>::copyonwrite;
		this->acquire(r);
		return *this;
	}
	const TObjPtrCommon& operator=(const TObjPtrCount<X>& r)
	{
		this->fAccess = &TObjPtrCommon<X>::autocreate;
		this->acquire(r);
		return *this;
	}
	const TObjPtrCommon& operator=(const X& p)
	{	
		this->TObjPtr<X>::operator=(p);
		return *this;
	}

	virtual bool isCopyonWrite() const	{ return (this->fAccess == &TObjPtrCommon<X>::copyonwrite); }
	virtual bool isAutoCreate() const	{ return (this->fAccess == &TObjPtrCommon<X>::autocreate); }
	virtual void setCopyonWrite() const	{ this->fAccess = &TObjPtrCommon<X>::copyonwrite; }
	virtual void setAutoCreate() const	{ this->fAccess = &TObjPtrCommon<X>::autocreate; }

	virtual const X& readaccess() const	{ return this->autocreate(); }
	virtual X& writeaccess()			{ return (this->*fAccess)(); }
};




/*
/////////////////////////////////////////////////////////////////////
// reference counting pointer template
// counts references on the used data
// automatic cleanup when the last data is destroyed
// implements a copy-on-write scheme
/////////////////////////////////////////////////////////////////////
template<typename T>
class TPtrRef
{
	/////////////////////////////////////////////////////////////////
	// the reference manages refernce counting and automatic cleanup
	// the user data is sored as an member element
	/////////////////////////////////////////////////////////////////
	class CRefCnt
	{
		friend class TPtrRef<T>;
		/////////////////////////////////////////////////////////////
		// class data:
		/////////////////////////////////////////////////////////////
		T		cData;
		size_t	cRefCnt;
		#ifndef SINGLETHREAD
		rwlock	cRWlock;
		#endif
public:
		/////////////////////////////////////////////////////////////
		// Constructors
		// aquire the reference automatically on creation
		// a destructor is not necessary can use defaults
		/////////////////////////////////////////////////////////////
		CRefCnt():cRefCnt(0)					{aquire();}
		CRefCnt(const T& t):cData(t),cRefCnt(0)	{aquire();}
		/////////////////////////////////////////////////////////////
		// aquire a reference 
		// and increment the reference counter
		/////////////////////////////////////////////////////////////
		void aquire()
		{
			atomicincrement(&cRefCnt);
		}
		/////////////////////////////////////////////////////////////
		// release a reference 
		// delete when the last reference is gone
		/////////////////////////////////////////////////////////////
		void release()
		{
			if( 0==atomicdecrement(&cRefCnt) )
				delete this;
		}
		/////////////////////////////////////////////////////////////
		// reference counter access and check funtion
		/////////////////////////////////////////////////////////////
		bool isShared()	const {return (cRefCnt>1);}
		const size_t getRefCount() const {return cRefCnt;}
		/////////////////////////////////////////////////////////////
		// read/write lock for syncronisation on multithread environment
		/////////////////////////////////////////////////////////////
		void ReadLock() 
		{
			#ifndef SINGLETHREAD
			cRWlock.rdlock(); 
			#endif
		}
		void WriteLock()
		{
			#ifndef SINGLETHREAD
			cRWlock.wrlock(); 
			#endif
		}
		void UnLock()	
		{
			#ifndef SINGLETHREAD
			cRWlock.unlock(); 
			#endif
		}
	};
	/////////////////////////////////////////////////////////////////

	CRefCnt	*cRef;
public:
	/////////////////////////////////////////////////////////////////
	// constructors and destructor
	// 
	/////////////////////////////////////////////////////////////////
	TPtrRef() : cRef(NULL)
	{}
	TPtrRef(TPtrRef& ptr) : cRef(ptr.cRef)
	{
		if (cRef != 0) {
			cRef->aquire();
		}
	}
	TPtrRef(const T& obj) : cRef( new CRefCnt(obj) )
	{	// no need to aquire, is done on reference creation
	}
	~TPtrRef()
	{
		if (cRef) cRef->release();
	}
	/////////////////////////////////////////////////////////////////
	// assignment operator
	// release an existing Reference then copy and aquire the given
	/////////////////////////////////////////////////////////////////
	TPtrRef& operator=(const TPtrRef& prt)
	{
		if (cRef != prt.cRef) {
			if (cRef) cRef->release(); // let go the old ref
			cRef = prt.cRef;
			if (cRef) cRef->aquire(); // and aquire the new one
		}
		
		// !!todo!! check if necessary
	//	if(cRef == NULL)  // prt was empty
	//	{	// so we create a new reference
	//		cRef = new CRefCnt;
	//		// do not aquire, the reference creation is already aquiring one time
	//		// and also put it to the calling ptr (cast needed for writing)
	//		((CRefCnt*)prt.cRef) = cRef;
	//		cRef->aquire(); 
	//	}
		return *this;
	}
	/////////////////////////////////////////////////////////////////
	// just for inspection and test purpose
	/////////////////////////////////////////////////////////////////
	const size_t getRefCount() const { return (cRef) ? cRef->getRefCount() : 1;}
	/////////////////////////////////////////////////////////////////
	// compare for 2 pointers if the share an identical reference
	/////////////////////////////////////////////////////////////////
	bool operator ==(const TPtrRef& ref) const 
	{
		return (cRef == ref.cRef);
	}
	bool operator !=(const TPtrRef& ref) const 
	{
		return (cRef != ref.cRef);
	}

	/////////////////////////////////////////////////////////////////
	// returns the data element for read access
	// can be used to write the sharing data
	/////////////////////////////////////////////////////////////////
	void Check()
	{
		if(!cRef) cRef = new CRefCnt;
	}
	T& ReadAccess() const
	{ 
		// need to cast pointers to allow the creation of an empty element if not exist
		const_cast< TPtrRef<T>* >(this)->Check();	
		// no need to aquire, is done on reference creation
		return cRef->cData;
	}
	/////////////////////////////////////////////////////////////////
	// write access function
	// copy the data before returning the element
	/////////////////////////////////////////////////////////////////
	T& WriteAccess()
	{
		if(!cRef) 
			cRef = new CRefCnt;
		else if( cRef->isShared() )
		{	cRef->release();
			cRef = new CRefCnt(cRef->cData);
		}
		// no need to aquire, is done on reference creation
		return cRef->cData;
	}
	/////////////////////////////////////////////////////////////////
	// automatic read/write access function pairs
	// when access is const the compiler choose read access
	// when access is not const it will be write access
	// proper function declarations necessary to identify
	// const and non const access to the data
	/////////////////////////////////////////////////////////////////
	const T* operator->() const
	{	
		return &(this->ReadAccess());
	}
	/////////////////////////////////////////////////////////////////
	T* operator->()
	{
		return &(this->WriteAccess());
	}
	/////////////////////////////////////////////////////////////////
	const T& operator*() const
	{	
		return (this->ReadAccess());
	}
	/////////////////////////////////////////////////////////////////
	T& operator*()
	{
		return (this->WriteAccess());
	}
	/////////////////////////////////////////////////////////////////
	// allow direct access to the data
	// to use for write access to the global onject
	/////////////////////////////////////////////////////////////////
	T* global()
	{
		if(!cRef) 
			cRef = new CRefCnt; // no need to aquire, is done on reference creation			
		return cRef->cData;
	}

	/////////////////////////////////////////////////////////////////
	// read/write lock for syncronisation on multithread environment
	/////////////////////////////////////////////////////////////////
	void ReadLock()	{if(cRef) cRef->ReadLock(); }
	void WriteLock(){if(cRef) cRef->WriteLock(); }
	void UnLock()	{if(cRef) cRef->UnLock(); }
};
*/
/////////////////////////////////////////////////////////////////////




/*
// strong/weak pointer maybe not that usefull
// but I'm still hesitating to remove them
/////////////////////////////////////////////////////////////////////////////////////////
//
// handy helpers
//
/////////////////////////////////////////////////////////////////////////////////////////

struct RefCounts {
	RefCounts () : totalRefs_ (1), strongRefs_ (1) 
	{}

	size_t totalRefs_;
	size_t strongRefs_;
};

enum Null {
	null = 0
};



/////////////////////////////////////////////////////////////////////////////////////////
// Strong and Weak reference pointers
// strong pointers count the reference 
// weak pointers only using the reference to decide if an object exist or not
/////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// forward definitions

template <typename T> class TPtrWeak;
template <typename T> class TPtrStrong;

///////////////////////////////////////////////////////
// TPtrStrong
template <typename T>
class TPtrStrong {
public:
	TPtrStrong () : cPtr (0), cRefCount (0) {
	}

	TPtrStrong (const TPtrStrong<T>& other) {
		cPtr = other.cPtr;
		cRefCount = other.cRefCount;

		if (cPtr != 0) {
			atomicincrement (cRefCount->strongRefs_);
			atomicincrement (cRefCount->totalRefs_);
		}
	}

	template <typename O>
	TPtrStrong (const TPtrWeak<O>& other) {
		cPtr = other.cPtr;
		cRefCount = other.cRefCount;

		if (cPtr != 0) {
			atomicincrement (cRefCount->strongRefs_);
			atomicincrement (cRefCount->totalRefs_);
		}
	}

	TPtrStrong& operator = (const TPtrStrong<T>& other) {
		if (cPtr != other.cPtr) {
			releaseRef ();

			cPtr = other.cPtr;
			cRefCount = other.cRefCount;

			if (cPtr != 0) {
				atomicincrement (cRefCount->strongRefs_);
				atomicincrement (cRefCount->totalRefs_);
			}
		}

		return *this;
	}

	TPtrStrong& operator = (const TPtrWeak<T>& other) {
		if (cPtr != other.cPtr) {
			releaseRef ();

			cPtr = other.cPtr;
			cRefCount = other.cRefCount;

			if (cPtr != 0) {
				atomicincrement (cRefCount->strongRefs_);
				atomicincrement (cRefCount->totalRefs_);
			}
		}

		return *this;
	}

	TPtrStrong& operator = (Null) {
		releaseRef ();

		return *this;
	}

	~TPtrStrong () {
		releaseRef ();
	}

	T& operator * () const {
		assert (cPtr != 0);

		return *cPtr;
	}

	T* operator -> () const {
		assert (cPtr != 0);

		return cPtr;
	}

	void swap (TPtrStrong<T>& other) {
		std::swap (cPtr, other.cPtr);
		std::swap (cRefCount, other.cRefCount);
	}

private:
	TPtrStrong (T* ptr, RefCounts* refCounts) : cPtr (ptr), cRefCount (refCounts) {
	}

	void releaseRef () {
		if (cPtr == 0) {
			return;
		}

		{
			bool release = atomicdecrement (cRefCount->strongRefs_);
			if (release) {
				delete cPtr;
			}

			cPtr = 0;
		}

		{
			bool release = atomicdecrement (cRefCount->totalRefs_);
			if (release) {
				delete cRefCount;
			}

			cRefCount = 0;
		}
	}

	T* cPtr;
	RefCounts* cRefCount;

	template <typename T>
	friend TPtrStrong<T> create ();
	
	template <typename T, typename P1>
	friend TPtrStrong<T> create (P1 p1);

	template <typename T, typename P1, typename P2>
	friend TPtrStrong<T> create (P1 p1, P2 p2);

	template <typename T, typename P1, typename P2, typename P3>
	friend TPtrStrong<T> create (P1 p1, P2 p2, P3 p3);

	template <typename T, typename P1, typename P2, typename P3, typename P4>
	friend TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4);

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
	friend TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	friend TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

	template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	friend TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

	template <typename T>
	friend TPtrStrong<T> wrapPtr (T* t);

	friend class TPtrStrong;
	friend class TPtrWeak;

	template<typename T, typename U>
	friend bool operator == (const TPtrStrong<T>& a, const TPtrStrong<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrStrong<T>& a, const TPtrStrong<U>& b);

	template<typename T>
	friend bool operator == (const TPtrStrong<T>& a, Null);

	template<typename T>
	friend bool operator == (Null, const TPtrStrong<T>& a);

	template<typename T>
	friend bool operator != (const TPtrStrong<T>& a, Null);

	template<typename T>
	friend bool operator != (Null, const TPtrStrong<T>& a);

	template<typename T, typename U>
	friend bool operator == (const TPtrStrong<T>& a, const TPtrWeak<U>& b);

	template<typename T, typename U>
	friend bool operator == (const TPtrWeak<T>& a, const TPtrStrong<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrStrong<T>& a, const TPtrWeak<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrWeak<T>& a, const TPtrStrong<U>& b);

	template <typename T, typename F>
	friend TPtrStrong<T> staticCast (const TPtrStrong<F>& from);

	template <typename T, typename F>
	friend TPtrStrong<T> constCast (const TPtrStrong<F>& from);

	template <typename T, typename F>
	friend TPtrStrong<T> dynamicCast (const TPtrStrong<F>& from);

	template <typename T, typename F>
	friend TPtrStrong<T> checkedCast (const TPtrStrong<F>& from);

	template <typename T, typename F>
	friend TPtrStrong<T> queryCast (const TPtrStrong<F>& from);
};

///////////////////////////////////////////////////////
// TPtrWeak

template <typename T>
class TPtrWeak {
public:
	TPtrWeak () : cPtr (0), cRefCount (0) {
	}

	TPtrWeak (const TPtrWeak<T>& other) {
		cPtr = other.cPtr;
		cRefCount = other.cRefCount;

		if (cPtr != 0) {
			atomicincrement (cRefCount->totalRefs_);
		}
	}

	template <typename T>
	TPtrWeak (const TPtrStrong<T>& other) {
		cPtr = other.cPtr;
		cRefCount = other.cRefCount;

		if (cPtr != 0) {
			atomicincrement (cRefCount->totalRefs_);
		}
	}

	TPtrWeak& operator = (const TPtrWeak<T>& other) {
		if (get () != other.get ()) {
			releaseRef ();

			cPtr = other.cPtr;
			cRefCount = other.cRefCount;

			if (cPtr != 0) {
				atomicincrement (cRefCount->totalRefs_);
			}
		}

		return *this;
	}

	TPtrWeak& operator = (Null) {
		releaseRef ();

		return *this;
	}


	template <typename T>
	TPtrWeak& operator = (const TPtrStrong<T>& other) {
		if (get () != other.cPtr) {
			releaseRef ();

			cPtr = other.cPtr;
			cRefCount = other.cRefCount;

			if (cPtr != 0) {
				atomicincrement (cRefCount->totalRefs_);
			}
		}

		return *this;
	}

	~TPtrWeak () {
		releaseRef ();
	}

	T& operator * () const {
		assert ( (0!=cRefCount->strongRefs_) && (0!=cPtr) );

		return *get ();
	}

	T* operator -> () const {
		assert ( (0!=cRefCount->strongRefs_) && (0!=cPtr) );

		return get ();
	}

	void swap (TPtrWeak<T>& other) {
		std::swap (cPtr, other.cPtr);
		std::swap (cRefCount, other.cRefCount);
	}

private:
	TPtrWeak (T* ptr, RefCounts* refCounts) : cPtr (ptr), cRefCount (refCounts) {
	}

	T* get () const {
		if( (cRefCount == 0) || (0==cRefCount->strongRefs_)) {
			return 0;
		}

		return cPtr;
	}

	bool isNull () const {
		return get () == 0;
	}

	void releaseRef () {
		if (cPtr == 0) {
			return;
		}

		bool release = atomicdecrement (cRefCount->totalRefs_);
		if (release) {
			delete cRefCount;
		}

		cRefCount = 0;
		cPtr = 0;
	}

	T* cPtr;
	RefCounts* cRefCount;

	friend class TPtrWeak;
	friend class SmartPtr;

	template<typename T, typename U>
	friend bool operator == (const TPtrWeak<T>& a, const TPtrWeak<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrWeak<T>& a, const TPtrWeak<U>& b);

	template<typename T>
	friend bool operator == (const TPtrWeak<T>& a, Null);

	template<typename T>
	friend bool operator == (Null, const TPtrWeak<T>& a);

	template<typename T>
	friend bool operator != (const TPtrWeak<T>& a, Null);

	template<typename T>
	friend bool operator != (Null, const TPtrWeak<T>& a);

	template<typename T, typename U>
	friend bool operator == (const TPtrStrong<T>& a, const TPtrWeak<U>& b);

	template<typename T, typename U>
	friend bool operator == (const TPtrWeak<T>& a, const TPtrStrong<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrStrong<T>& a, const TPtrWeak<U>& b);

	template<typename T, typename U>
	friend bool operator != (const TPtrWeak<T>& a, const TPtrStrong<U>& b);

	template <typename T, typename F>
	friend TPtrWeak<T> staticCast (const TPtrWeak<F>& from);

	template <typename T, typename F>
	friend TPtrWeak<T> constCast (const TPtrWeak<F>& from);

	template <typename T, typename F>
	friend TPtrWeak<T> dynamicCast (const TPtrWeak<F>& from);

	template <typename T, typename F>
	friend TPtrWeak<T> checkedCast (const TPtrWeak<F>& from);

	template <typename T, typename F>
	friend TPtrWeak<T> queryCast (const TPtrWeak<F>& from);
};

///////////////////////////////////////////////////////
// globals

template<typename T, typename U>
inline bool operator == (const TPtrStrong<T>& a, const TPtrStrong<U>& b) {
	return a.cPtr == b.cPtr;
}

template<typename T, typename U>
inline bool operator == (const TPtrWeak<T>& a, const TPtrWeak<U>& b) {
	return a.get () == b.get ();
}

template<typename T, typename U>
inline bool operator == (const TPtrStrong<T>& a, const TPtrWeak<U>& b) {
	return a.cPtr == b.get ();
}

template<typename T, typename U>
inline bool operator == (const TPtrWeak<T>& a, const TPtrStrong<U>& b) {
	return a.get () == b.cPtr;
}

template<typename T, typename U>
inline bool operator != (const TPtrStrong<T>& a, const TPtrStrong<U>& b) {
	return a.cPtr != b.cPtr;
}

template<typename T, typename U>
inline bool operator != (const TPtrWeak<T>& a, const TPtrWeak<U>& b) {
	return a.get () != b.get ();
}

template<typename T, typename U>
inline bool operator != (const TPtrStrong<T>& a, const TPtrWeak<U>& b) {
	return a.cPtr != b.get ();
}

template<typename T, typename U>
inline bool operator != (const TPtrWeak<T>& a, const TPtrStrong<U>& b) {
	return a.get () != b.cPtr;
}

template<typename T>
inline bool operator == (const TPtrStrong<T>& a, Null) {
	return a.cPtr == 0;
}

template<typename T>
inline bool operator == (Null, const TPtrStrong<T>& a) {
	return a.cPtr == 0;
}

template<typename T>
inline bool operator != (const TPtrStrong<T>& a, Null) {
	return a.cPtr != 0;
}

template<typename T>
inline bool operator != (Null, const TPtrStrong<T>& a) {
	return a.cPtr != 0;
}

template<typename T>
inline bool operator == (const TPtrWeak<T>& a, Null) {
	return a.isNull ();
}

template<typename T>
inline bool operator == (Null, const TPtrWeak<T>& a) {
	return a.isNull ();
}

template<typename T>
inline bool operator != (const TPtrWeak<T>& a, Null) {
	return !a.isNull ();
}

template<typename T>
inline bool operator != (Null, const TPtrWeak<T>& a) {
	return a.isNull ();
}

//////////////////////////////////////////////////////
// creation functions

template <typename T>
TPtrStrong<T> create () {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T;
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1>
TPtrStrong<T> create (P1 p1) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2>
TPtrStrong<T> create (P1 p1, P2 p2) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2, typename P3>
TPtrStrong<T> create (P1 p1, P2 p2, P3 p3) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2, p3);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2, typename P3, typename P4>
TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2, p3, p4);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2, p3, p4, p5);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2, p3, p4, p5, p6);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
TPtrStrong<T> create (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
	RefCounts* rc = new RefCounts;

	try {
		T* t = new T (p1, p2, p3, p4, p5, p6, p7);
		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete rc;
		throw;
	}
}

template <typename T>
TPtrStrong<T> wrapPtr (T* t) {
	if (t == 0) {
		return TPtrStrong<T> ();
	}

	try {
		RefCounts* rc = new RefCounts;

		return TPtrStrong<T> (t, rc);
	}
	catch (...) {
		delete t;
		throw;
	}
}

///////////////////////////////////////////////////////
// casts

template <typename T, typename F>
TPtrStrong<T> staticCast (const TPtrStrong<F>& from) {
	if (from.cPtr == 0) {
		return TPtrStrong<T>();
	}

	T* ptr = static_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->strongRefs_);
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrStrong<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrStrong<T> constCast (const TPtrStrong<F>& from) {
	if (from.cPtr == 0) {
		return TPtrStrong<T>();
	}

	T* ptr = const_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->strongRefs_);
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrStrong<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrStrong<T> dynamicCast (const TPtrStrong<F>& from) {
	if (from.cPtr == 0) {
		return TPtrStrong<T>();
	}

	T* ptr = &dynamic_cast <T&> (*from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->strongRefs_);
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrStrong<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrStrong<T> queryCast (const TPtrStrong<F>& from) {
	T* ptr = dynamic_cast <T*> (from.cPtr);

	if (ptr == 0) {
		return TPtrStrong<T>();
	}

	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->strongRefs_);
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrStrong<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrStrong<T> checkedCast (const TPtrStrong<F>& from) {
	if (from.cPtr == 0) {
		return TPtrStrong<T>();
	}

	assert (dynamic_cast<T*> (from.cPtr) != 0);

	T* ptr = static_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->strongRefs_);
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrStrong<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrWeak<T> staticCast (const TPtrWeak<F>& from) {
	if (from.get () == 0) {
		return TPtrWeak<T>();
	}

	T* ptr = static_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrWeak<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrWeak<T> constCast (const TPtrWeak<F>& from) {
	if (from.get () == 0) {
		return TPtrWeak<T>();
	}

	T* ptr = const_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrWeak<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrWeak<T> dynamicCast (const TPtrWeak<F>& from) {
	if (from.get () == 0) {
		return TPtrWeak<T>();
	}

	T* ptr = &dynamic_cast <T&> (*from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrWeak<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrWeak<T> queryCast (const TPtrWeak<F>& from) {
	T* ptr = dynamic_cast <T*> (from.get ());

	if (ptr == 0) {
		return TPtrWeak<T>();
	}

	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrWeak<T> (ptr, refCounts);
}

template <typename T, typename F>
TPtrWeak<T> checkedCast (const TPtrWeak<F>& from) {
	if (from.get () == 0) {
		return TPtrWeak<T>();
	}

	assert (dynamic_cast<T*> (from.cPtr) != 0);

	T* ptr = static_cast <T*> (from.cPtr);
	RefCounts* refCounts = from.cRefCount;

	if (ptr != 0) {
		atomicincrement (refCounts->totalRefs_);
	}

	return TPtrWeak<T> (ptr, refCounts);
}


template <typename T> 
inline void swap (TPtrStrong<T>& t1, TPtrStrong<T>& t2) {
	t1.swap (t2);
}

template <typename T> 
inline void swap (TPtrWeak<T>& t1, TPtrWeak<T>& t2) {
	t1.swap (t2);
}


*/
NAMESPACE_END(basics)

#endif//__BASESAFEPTR_H__
