// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MALLOC_VH_
#define _MALLOC_VH_

#include <stdio.h> // for size_t


/*

#define ALC_MARK __FILE__, __LINE__, __func__






//////////////////////////////////////////////////////////////////////
// Whether to use Athena's built-in Memory Manager (enabled by default)
// To disable just comment the following line
#define USE_MEMMGR


#define LOG_MEMMGR		// Whether to enable Memory Manager's logging
//#define MEMTRACE		// can be used to mark and identify specific memories
//#define MEMCHECKER	// checks validity of all pointers in and out of the memory manager

#ifdef USE_MEMMGR


#ifdef MEMTRACE
#include "base.h"
/////////////////////////////////////////////////////////////////////
// memory tracer to locate conditions of memory leaks
// each alloced memory can be stored in the list with a description
// in case of a leak the description is shown, allowing to find
// the conditions under which the leaking memory was allocated
// (instead of just stating where it was allocated)
// usage CMemDesc::Insert( <alloced memory pointer>, <description> );
/////////////////////////////////////////////////////////////////////
class CMemDesc : private Mutex
{
	/////////////////////////////////////////////////////////////////
	// index structure
	class _key
	{
	public:
		void*	cMem;	// key value
		char	cDesc[64];	// description

		_key(void* m=NULL, const char* d=NULL):cMem(m)
		{
			if(d)
			{
				strncpy(cDesc,d,64);
				cDesc[63]=0;
			}
			else
				*cDesc=0;
		}
		bool operator==(const _key& k) const	{ return cMem==k.cMem; }
		bool operator> (const _key& k) const	{ return cMem> k.cMem; }
		bool operator< (const _key& k) const	{ return cMem< k.cMem; }
	};
	/////////////////////////////////////////////////////////////////
	// class data
	static TslistDST<_key>	cIndex;	// the index
	CMemDesc(const CMemDesc&);
	const CMemDesc& operator=(const CMemDesc&);
public:
	CMemDesc()	{}
	~CMemDesc()	{}

	static void Insert(void* m, const char* d=NULL)
	{
		if(m) cIndex.insert( _key(m,d) );
	}
	static void print(void* m)
	{
		size_t i;
		if( m && cIndex.find( _key(m), 0, i) )
		{
			printf("%p: %s\n", m, cIndex[i].cDesc);
		}
		else
			printf("no description\n");
	}

};
#endif


#	define aMalloc(n)		_mmalloc(n,ALC_MARK)
#	define aMallocA(n)		_mmalloc(n,ALC_MARK)
#	define aCalloc(m,n)		_mcalloc(m,n,ALC_MARK)
#	define aCallocA(m,n)	_mcalloc(m,n,ALC_MARK)
#	define aRealloc(p,n)	_mrealloc(p,n,ALC_MARK)
#	define aStrdup(p)		_mstrdup(p,ALC_MARK)
#	define aFree(p)			_mfree(p,ALC_MARK)

	void* _mmalloc	(size_t, const char *, int, const char *);
	void* _mcalloc	(size_t, size_t, const char *, int, const char *);
	void* _mrealloc	(void *, size_t, const char *, int, const char *);
	char* _mstrdup	(const char *, const char *, int, const char *);
	void  _mfree	(void *, const char *, int, const char *);	

#else

#	define aMalloc(n)		aMalloc_(n,ALC_MARK)
#	define aMallocA(n)		aMallocA_(n,ALC_MARK)
#	define aCalloc(m,n)		aCalloc_(m,n,ALC_MARK)
#	define aCallocA(m,n)	aCallocA_(m,n,ALC_MARK)
#	define aRealloc(p,n)	aRealloc_(p,n,ALC_MARK)
#	define aStrdup(p)		aStrdup_(p,ALC_MARK)
#	define aFree(p)			aFree_(p,ALC_MARK)

	void* aMalloc_	(size_t, const char *, int, const char *);
	void* aMallocA_	(size_t, const char *, int, const char *);
	void* aCalloc_	(size_t, size_t, const char *, int, const char *);
	void* aCallocA_	(size_t, size_t, const char *, int, const char *);
	void* aRealloc_	(void *, size_t, const char *, int, const char *);
	char* aStrdup_	(const char *, const char *, int, const char *);
	void  aFree_	(void *, const char *, int, const char *);

#endif

////////////// Memory Managers //////////////////

#ifdef MEMWATCH

#	include "memwatch.h"
#	define MALLOC(n)	mwMalloc(n,__FILE__, __LINE__)
#	define MALLOCA(n)	mwMalloc(n,__FILE__, __LINE__)
#	define CALLOC(m,n)	mwCalloc(m,n,__FILE__, __LINE__)
#	define CALLOCA(m,n)	mwCalloc(m,n,__FILE__, __LINE__)
#	define REALLOC(p,n)	mwRealloc(p,n,__FILE__, __LINE__)
#	define STRDUP(p)	mwStrdup(p,__FILE__, __LINE__)
#	define FREE(p)		mwFree(p,__FILE__, __LINE__)

#elif defined(DMALLOC)

#	include "dmalloc.h"
#	define MALLOC(n)	dmalloc_malloc(__FILE__, __LINE__, (n), DMALLOC_FUNC_MALLOC, 0, 0)
#	define MALLOCA(n)	dmalloc_malloc(__FILE__, __LINE__, (n), DMALLOC_FUNC_MALLOC, 0, 0)
#	define CALLOC(m,n)	dmalloc_malloc(__FILE__, __LINE__, (p)*(n), DMALLOC_FUNC_CALLOC, 0, 0)
#	define CALLOCA(m,n)	dmalloc_malloc(__FILE__, __LINE__, (p)*(n), DMALLOC_FUNC_CALLOC, 0, 0)
#	define REALLOC(p,n)	dmalloc_realloc(__FILE__, __LINE__, (p), (n), DMALLOC_FUNC_REALLOC, 0)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

#elif defined(GCOLLECT)

#	include "gc.h"
#	define MALLOC(n)	GC_MALLOC(n)
#	define MALLOCA(n)	GC_MALLOC_ATOMIC(n)
#	define CALLOC(m,n)	_bcalloc(m,n)
#	define CALLOCA(m,n)	_bcallocA(m,n)
#	define REALLOC(p,n)	GC_REALLOC(p,n)
#	define STRDUP(p)	_bstrdup(p)
#	define FREE(p)		GC_FREE(p)

	void * _bcalloc(size_t, size_t);
	void * _bcallocA(size_t, size_t);
	char * _bstrdup(const char *);

#elif defined(BCHECK)

#	define MALLOC(n)	malloc(n)
#	define MALLOCA(n)	malloc(n)
#	define CALLOC(m,n)	calloc(m,n)
#	define CALLOCA(m,n)	calloc(m,n)
#	define REALLOC(p,n)	realloc(p,n)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

#else// neither GCOLLECT or BCHECK

#	define MALLOC(n)	malloc(n)
#	define MALLOCA(n)	malloc(n)
#	define CALLOC(m,n)	calloc(m,n)
#	define CALLOCA(m,n)	calloc(m,n)
#	define REALLOC(p,n)	realloc(p,n)
#	define STRDUP(p)	strdup(p)
#	define FREE(p)		free(p)

#endif

////////////// Others //////////////////////////
// should be merged with any of above later
#define CREATE(result, type, number) \
	(result) = (type *) aCalloc ((number), sizeof(type));

#define CREATE_A(result, type, number) \
	(result) = (type *) aCallocA ((number), sizeof(type));

#define RECREATE(result, type, number) \
	(result) = (type *) aRealloc ((result), sizeof(type) * (number)); 	

////////////////////////////////////////////////


*/


template<typename T>
struct simple_buffer
{
	T* array;
	simple_buffer(size_t sz, bool clearmem=false)
		: array(new T[sz])
	{
		if(clearmem)
			memset(this->array,0,sz*sizeof(T));
	}
	~simple_buffer()
	{
		if(array)
			delete[] array;
	}
	T& operator[](int p)
	{
		return array[p];
	}
	const T& operator[](int p) const 
	{
		return array[p];
	}
	operator T*()				{ return array; }
	operator const T*() const	{ return array; }
	T* operator()()				{ return array; }
	const T* operator()() const	{ return array; }
};

/////////////// Buffer Creation /////////////////
// Full credit for this goes to Shinomori [Ajarn]

#if defined(__GNUC__)// GCC has variable length arrays, though they are not ISO C++ conform
	#define CREATE_BUFFER(name, type, size) type name[size]; memset(name,0,size*sizeof(type))
	#define DELETE_BUFFER(name)
#else // others don't, so we emulate them
// c style
	#define CREATE_BUFFER(name, type, size) type *name = (type *) calloc (size, sizeof(type))
	#define DELETE_BUFFER(name) free(name)
// c++ style
// note that only pod's can be used here
// and the buffer cannot be passed through ellipsis
//	#define CREATE_BUFFER(name, type, size) simple_buffer<type> name(size, true);
//	#define DELETE_BUFFER(name)
#endif





// temporary memory realloc replacement using memcpy and memset
// change to containers generally
template<typename T>
size_t new_realloc(T*& pointer, size_t oldsize, size_t addition)
{
	const size_t sz = oldsize+addition;
	T* tmp = new T[sz];
	memset(tmp+oldsize,0, addition*sizeof(T));
	if(pointer)
	{
		memcpy(tmp, pointer, oldsize*sizeof(T));
		delete[] pointer;
	}
	pointer = tmp;
	return sz;
}


int memmgr_init(const char* file);
void memmgr_final(void);


#endif
