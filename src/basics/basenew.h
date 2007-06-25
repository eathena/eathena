#ifndef __BASENEW_H__
#define __BASENEW_H__

// actually only necessary for placemant new operators
// and as declarative part for basemempool
// might encounter problems on various gcc's
// especially when new/delete are buildin's


///////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)
///////////////////////////////////////////////////////////

#ifndef __PLACEMENT_NEW_INLINE
// windows <new> has a protective macro

extern void *operator new (size_t sz);
extern void operator delete (void * a) NOTHROW();
extern void *operator new[] (size_t sz);
extern void operator delete[] (void * a) NOTHROW();

#define __PLACEMENT_NEW_INLINE
/// inplace-new operator
inline void *operator new (size_t sz,void* p)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("NULL in placement new");
#endif
	return p;
}

/// inplace-new operator
inline void *operator new[] (size_t sz, void* p)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("NULL in placement new[]");
#endif
	return p;
}

/// inplace-delete operator
inline void operator delete (void * a, void* p) NOTHROW()
{}

/// inplace-delete operator
inline void operator delete[] (void *a, void* p) NOTHROW()
{}


// temporary copy
///////////////////////////////////////////////////////////////////////////////
#if !defined(WITH_MEMORYMANAGER)
///////////////////////////////////////////////////////////////////////////////

inline void* operator new(size_t bytes)
{
	void* p=malloc(bytes);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}
inline void operator delete(void* p) NOTHROW()
{
	free(p);
}
inline void* operator new[](size_t bytes)
{
	void* p=malloc(bytes);
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("allocation failed");
#endif
	return p;
}
inline void operator delete[](void* p) NOTHROW()
{
	free(p);
}
///////////////////////////////////////////////////////////////////////////////
#endif// !defined(WITH_MEMORYMANAGER)
///////////////////////////////////////////////////////////////////////////////

#endif//__PLACEMENT_NEW_INLINE
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#else//!defined(WIN32)
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/* from gcc include/c++/3.4.2/new

 ** These are replaceable signatures:
 *  - normal single new and delete (no arguments, throw @c bad_alloc on error)
 *  - normal array new and delete (same)
 *  - @c nothrow single new and delete (take a @c nothrow argument, return
 *    @c NULL on error)
 *  - @c nothrow array new and delete (same)
 *
 *  Placement new and delete signatures (take a memory address argument,
 *  does nothing) may not be replaced by a user's program.

  nice, but how to check if the non-overloadable stuff exists
  and doing this plattform independend?
  and what if I nevertheless want to overload?
  answer is: not possible and fish around in a bunch of defines

  and "replaceable" only means "replace them with the same declaration"
  but what when I want to throw a different exception
  answer is: "you cannot since the throw definition is part of the declaration"


  these are the scope declarations of include/c++/<new> from solaris and two different linux distros, 
  I bet there are more of these, but how was the meaning of "standardisation? everybody makes his own?
  #if !defined(_NEW) && !defined(_NEW_)  && !defined(__NEW__) 
  
  so again: avoid any std:: header and build all necessary functionality yourself
*/
///////////////////////////////////////////////////////////////////////////////

//#include <new>
#if !defined(_NEW) && !defined(_NEW_)  && !defined(__NEW__) 

///////////////////////////////////////////////////////////////////////////////
// global new/delete operators
// will conflict with with previous header includes or compiler buildins
// nevertheless, try using the existing functions for standard new/delete
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// standard new operator.
extern void* operator new(size_t bytes);
///////////////////////////////////////////////////////////////////////////////
/// standard delete operator.
extern void operator delete(void* p) NOTHROW();
///////////////////////////////////////////////////////////////////////////////
/// standard new[] operator.
extern void* operator new[](size_t bytes);
///////////////////////////////////////////////////////////////////////////////
/// standard delete[] operator.
extern void operator delete[](void* p) NOTHROW();


/// inplace-new operator
inline void *operator new (size_t sz,void* p)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("NULL in placement new");
#endif
	return p;
}

/// inplace-new operator
inline void *operator new[] (size_t sz, void* p)
{
#if defined(MEMORY_EXCEPTIONS)
	if(!p) throw exception_memory("NULL in placement new[]");
#endif
	return p;
}
/// inplace-delete operator
inline void operator delete (void * a, void* p) NOTHROW()
{
}
/// inplace-delete operator
inline void operator delete[] (void *a, void* p) NOTHROW()
{
}

#endif// !defined(_NEW) && !defined(_NEW_)  && !defined(__NEW__) 

///////////////////////////////////////////////////////////////////////////////
#endif//!defined(WIN32)
///////////////////////////////////////////////////////////////////////////////

#endif//__BASENEW_H__

