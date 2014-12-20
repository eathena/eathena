// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cbasetypes.h"
#include "core.h"
#include "malloc.h"
#include "showmsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////// Memory Libraries //////////////////

#if defined(MEMWATCH)

#	include <string.h> 
#	include "memwatch.h"
#	define MALLOC(n,file,line,func)     mwMalloc((n),(file),(line))
#	define CALLOC(m,n,file,line,func)   mwCalloc((m),(n),(file),(line))
#	define REALLOC(p,n,file,line,func)  mwRealloc((p),(n),(file),(line))
#	define STRDUP(p,file,line,func)     mwStrdup((p),(file),(line))
#	define FREE(p,file,line,func)       mwFree((p),(file),(line))
#	define MEMORY_USAGE()               0
#	define MEMORY_VERIFY(ptr)           mwIsSafeAddr(ptr, 1)
#	define MEMORY_CHECK()               CHECK()

#elif defined(DMALLOC)

#	include <string.h>
#	include <stdlib.h>
#	include "dmalloc.h"
#	define MALLOC(n,file,line,func)     dmalloc_malloc((file),(line),(n),DMALLOC_FUNC_MALLOC,0,0)
#	define CALLOC(m,n,file,line,func)   dmalloc_malloc((file),(line),(m)*(n),DMALLOC_FUNC_CALLOC,0,0)
#	define REALLOC(p,n,file,line,func)  dmalloc_realloc((file),(line),(p),(n),DMALLOC_FUNC_REALLOC,0)
#	define STRDUP(p,file,line,func)     strdup(p)
#	define FREE(p,file,line,func)       free(p)
#	define MEMORY_USAGE()               dmalloc_memory_allocated()
#	define MEMORY_VERIFY(ptr)           (dmalloc_verify(ptr) == DMALLOC_VERIFY_NOERROR)
#	define MEMORY_CHECK()               dmalloc_log_stats(); dmalloc_log_unfreed()

#elif defined(GCOLLECT)

#	include "gc.h"
#	ifdef GC_ADD_CALLER
#		define RETURN_ADDR 0,
#	else
#		define RETURN_ADDR
#	endif
#	define MALLOC(n,file,line,func)     GC_debug_malloc((n), RETURN_ADDR (file),(line))
#	define CALLOC(m,n,file,line,func)   GC_debug_malloc((m)*(n), RETURN_ADDR (file),(line))
#	define REALLOC(p,n,file,line,func)  GC_debug_realloc((p),(n), RETURN_ADDR (file),(line))
#	define STRDUP(p,file,line,func)     GC_debug_strdup((p), RETURN_ADDR (file),(line))
#	define FREE(p,file,line,func)       GC_debug_free(p)
#	define MEMORY_USAGE()               GC_get_heap_size()
#	define MEMORY_VERIFY(ptr)           (GC_base(ptr) != NULL)
#	define MEMORY_CHECK()               GC_gcollect()

#else

#	define MALLOC(n,file,line,func)     malloc(n)
#	define CALLOC(m,n,file,line,func)   calloc((m),(n))
#	define REALLOC(p,n,file,line,func)  realloc((p),(n))
#	define STRDUP(p,file,line,func)     strdup(p)
#	define FREE(p,file,line,func)       free(p)
#	define MEMORY_USAGE()               0
#	define MEMORY_VERIFY(ptr)           true
#	define MEMORY_CHECK()

#endif


void* aMalloc_(size_t size, const char* file, int line, const char* func)
{
	void* ret = MALLOC(size, file, line, func);

	if( ret == NULL )
	{
		ShowFatalError("%s:%d: in func %s: aMalloc error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}

	return ret;
}


void* aCalloc_(size_t num, size_t size, const char* file, int line, const char* func)
{
	void* ret = CALLOC(num, size, file, line, func);

	if( ret == NULL )
	{
		ShowFatalError("%s:%d: in func %s: aCalloc error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}

	return ret;
}


void* aRealloc_(void* p, size_t size, const char* file, int line, const char* func)
{
	void* ret = REALLOC(p, size, file, line, func);

	if( ret == NULL )
	{
		ShowFatalError("%s:%d: in func %s: aRealloc error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}

	return ret;
}


char* aStrdup_(const char* p, const char* file, int line, const char* func)
{
	char *ret = STRDUP(p, file, line, func);

	if( ret == NULL )
	{
		ShowFatalError("%s:%d: in func %s: aStrdup error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}

	return ret;
}


void aFree_(void* p, const char* file, int line, const char* func)
{
	if( p )
	{
		FREE(p, file, line, func);
	}
}


#ifdef USE_MEMMGR

#if defined(DEBUG)
#define DEBUG_MEMMGR
#endif

/* USE_MEMMGR */

/*
 * メモリマネージャ
 *     malloc , free の処理を効率的に出来るようにしたもの。
 *     複雑な処理を行っているので、若干重くなるかもしれません。
 *
 * データ構造など（説明下手ですいません^^; ）
 *     ・メモリを複数の「ブロック」に分けて、さらにブロックを複数の「ユニット」
 *       に分けています。ユニットのサイズは、１ブロックの容量を複数個に均等配分
 *       したものです。たとえば、１ユニット32KBの場合、ブロック１つは32Byteのユ
 *       ニットが、1024個集まって出来ていたり、64Byteのユニットが 512個集まって
 *       出来ていたりします。（padding,unit_head を除く）
 *
 *     ・ブロック同士はリンクリスト(block_prev,block_next) でつながり、同じサイ
 *       ズを持つブロック同士もリンクリスト(hash_prev,hash_nect) でつな
 *       がっています。それにより、不要となったメモリの再利用が効率的に行えます。
 */

/* ブロックのアライメント */
#define BLOCK_ALIGNMENT1	16
#define BLOCK_ALIGNMENT2	64

/* ブロックに入るデータ量 */
#define BLOCK_DATA_COUNT1	128
#define BLOCK_DATA_COUNT2	608

/* ブロックの大きさ: 16*128 + 64*608 = 40KB */
#define BLOCK_DATA_SIZE1	( BLOCK_ALIGNMENT1 * BLOCK_DATA_COUNT1 )
#define BLOCK_DATA_SIZE2	( BLOCK_ALIGNMENT2 * BLOCK_DATA_COUNT2 )
#define BLOCK_DATA_SIZE		( BLOCK_DATA_SIZE1 + BLOCK_DATA_SIZE2 )

/* 一度に確保するブロックの数。 */
#define BLOCK_ALLOC		104

/* check value used for buffer overflow detection */
#define TAILCHECK_VALUE (0xdeadbeafL)

/* ブロック */
struct block
{
	struct block* block_next;		/* 次に確保した領域 */
	struct block* unfill_prev;		/* 次の埋まっていない領域 */
	struct block* unfill_next;		/* 次の埋まっていない領域 */
	unsigned short unit_size;		/* ユニットの大きさ */
	unsigned short unit_hash;		/* ユニットのハッシュ */
	unsigned short unit_count;		/* ユニットの個数 */
	unsigned short unit_used;		/* 使用ユニット数 */
	unsigned short unit_unfill;		/* 未使用ユニットの場所 */
	unsigned short unit_maxused;	/* 使用ユニットの最大値 */
	char   data[ BLOCK_DATA_SIZE ];
};

struct unit_head
{
	struct block   *block;
	const  char*   file;
	unsigned short line;
	unsigned short size;
	long           checksum;    /* placeholder for memory's tail check value */
};

static struct block* hash_unfill[BLOCK_DATA_COUNT1 + BLOCK_DATA_COUNT2 + 1];
static struct block* block_first, *block_last, block_head;

/* メモリを使い回せない領域用のデータ */
struct unit_head_large
{
	size_t                  size;
	struct unit_head_large* prev;
	struct unit_head_large* next;
	struct unit_head        unit_head;
};

static struct unit_head_large *unit_head_large_first = NULL;

static struct block* block_malloc(unsigned short hash);
static void          block_free(struct block* p);
static size_t        memmgr_usage_bytes;

#define memmgr_assert(v) do { if(!(v)) { ShowError("Memory manager: assertion '" #v "' failed!\n"); } } while(0)

static inline struct unit_head* block2unit(struct block* p, unsigned short n)
{
	return (struct unit_head*)(&p->data[p->unit_size*n]);
}

static inline void memmgr_usage_increase(size_t delta)
{
	memmgr_assert( SIZE_MAX-memmgr_usage_bytes >= delta );

	memmgr_usage_bytes+= delta;
}

static inline void memmgr_usage_decrease(size_t delta)
{
	memmgr_assert( memmgr_usage_bytes >= delta );

	memmgr_usage_bytes-= delta;
}

static inline long* memmgr_unit_tail_large(struct unit_head_large* large)
{
	return (long*)(((char*)&large->unit_head.checksum) + large->size);
}

static inline long* memmgr_unit_tail(struct unit_head* head)
{
	return (long*)(((char*)&head->checksum) + head->size);
}

static inline struct unit_head_large* memmgr_memblock2unit_head_large(void* ptr)
{
	struct unit_head_large* large = NULL;  // dummy for offset calculation that takes alignment into account

	return (struct unit_head_large*)( (char*)ptr - ( (uintptr_t)&large->unit_head.checksum - (uintptr_t)large ) );
}

static inline struct unit_head* memmgr_memblock2unit_head(void* ptr)
{
	struct unit_head* head = NULL;  // dummy for offset calculation that takes alignment into account

	return (struct unit_head*)( (char*)ptr - ( (uintptr_t)&head->checksum - (uintptr_t)head ) );
}

static unsigned short size2hash( size_t size )
{
	if( size <= BLOCK_DATA_SIZE1 )
	{
		return (unsigned short)(size + BLOCK_ALIGNMENT1 - 1) / BLOCK_ALIGNMENT1;
	}
	else if( size <= BLOCK_DATA_SIZE )
	{
		return (unsigned short)(size - BLOCK_DATA_SIZE1 + BLOCK_ALIGNMENT2 - 1) / BLOCK_ALIGNMENT2 + BLOCK_DATA_COUNT1;
	}
	else
	{
		return 0xffff;  // too large chunk of memory to fit a block
	}
}

static size_t hash2size( unsigned short hash )
{
	if( hash <= BLOCK_DATA_COUNT1 )
	{
		return hash * BLOCK_ALIGNMENT1;
	}
	else
	{
		return (hash - BLOCK_DATA_COUNT1) * BLOCK_ALIGNMENT2 + BLOCK_DATA_SIZE1;
	}
}

void* _mmalloc(size_t size, const char *file, int line, const char *func )
{
	struct block *block;
	short size_hash = size2hash( size );
	struct unit_head *head;

	if( size >= 0x8000000 )
	{// 2GB
		ShowWarning("_mmalloc: Possible ill-allocation (%u bytes at %s:%d).\n", size, file, line);
	}

	if( size == 0 )
	{
		ShowError("Memory manager: Attempted to allocate zero-sized buffer (%s:%d in %s).\n", file, line, func);
		return NULL;
	}

	memmgr_usage_increase(size);

	/* ブロック長を超える領域の確保には、malloc() を用いる */
	/* その際、unit_head.block に NULL を代入して区別する */
	if( hash2size(size_hash) > BLOCK_DATA_SIZE - sizeof(struct unit_head) )
	{
		struct unit_head_large* p = (struct unit_head_large*)MALLOC(sizeof(struct unit_head_large)+size, file, line, func);

		if( p != NULL )
		{
			p->size            = size;
			p->unit_head.block = NULL;
			p->unit_head.size  = 0;
			p->unit_head.file  = file;
			p->unit_head.line  = line;
			p->prev = NULL;

			if( unit_head_large_first == NULL )
			{
				p->next = NULL;
			}
			else
			{
				unit_head_large_first->prev = p;
				p->next = unit_head_large_first;
			}

			unit_head_large_first = p;

#ifdef DEBUG_MEMMGR
			memset(&p->unit_head.checksum, 0xcd, size);
#endif

			memmgr_unit_tail_large(p)[0] = TAILCHECK_VALUE;

			return &p->unit_head.checksum;
		}
		else
		{
			ShowFatalError("Memory manager::memmgr_alloc failed (allocating %u+%u bytes at %s:%d).\n", sizeof(struct unit_head_large), size, file, line);
			exit(EXIT_FAILURE);
		}
	}

	/* 同一サイズのブロックが確保されていない時、新たに確保する */
	if( hash_unfill[size_hash] )
	{
		block = hash_unfill[size_hash];
	}
	else
	{
		block = block_malloc(size_hash);
	}

	if( block->unit_unfill == 0xFFFF )
	{
		// free済み領域が残っていない
		memmgr_assert(block->unit_used <  block->unit_count);
		memmgr_assert(block->unit_used == block->unit_maxused);
		head = block2unit(block, block->unit_maxused);
		block->unit_used++;
		block->unit_maxused++;
	}
	else
	{
		head = block2unit(block, block->unit_unfill);
		block->unit_unfill = head->size;
		block->unit_used++;
	}

	if( block->unit_unfill == 0xFFFF && block->unit_maxused >= block->unit_count )
	{
		// ユニットを使い果たしたので、unfillリストから削除
		if( block->unfill_prev == &block_head )
		{
			hash_unfill[ size_hash ] = block->unfill_next;
		}
		else
		{
			block->unfill_prev->unfill_next = block->unfill_next;
		}
		if( block->unfill_next )
		{
			block->unfill_next->unfill_prev = block->unfill_prev;
		}
		block->unfill_prev = NULL;
	}

#ifdef DEBUG_MEMMGR
	{
		unsigned char* data = (unsigned char*)&head->checksum;
		size_t i, sz = hash2size( size_hash );

		for( i = 0; i < sz; i++ )
		{
			if( data[i] != 0xfd )
			{
				if( head->line != 0xfdfd )
				{
					ShowError("Memory manager: freed-data is changed. (freed in %s line %d)\n", head->file, head->line);
				}
				else
				{
					ShowError("Memory manager: not-allocated-data is changed.\n");
				}
				break;
			}
		}

		memset(data, 0xcd, sz);
	}
#endif

	head->block = block;
	head->file  = file;
	head->line  = line;
	head->size  = (unsigned short)size;

	memmgr_unit_tail(head)[0] = TAILCHECK_VALUE;

	return &head->checksum;
};

void* _mcalloc(size_t num, size_t size, const char* file, int line, const char* func)
{
	void* p;

	memmgr_assert( num && SIZE_MAX/num > size );

	p = _mmalloc(num * size, file, line, func);
	memset(p, 0, num * size);

	return p;
}

void* _mrealloc(void* memblock, size_t size, const char* file, int line, const char* func)
{
	size_t old_size;

	if( memblock == NULL )
	{
		return _mmalloc(size, file, line, func);
	}

	old_size = memmgr_memblock2unit_head(memblock)->size;

	if( old_size == 0 )
	{
		old_size = memmgr_memblock2unit_head_large(memblock)->size;
	}

	if( old_size > size )
	{
		// smaller -> keep unchanged (lazy)
		// TODO: This improves performance, but may waste memory. [Ai4rei]
		return memblock;
	}
	else
	{
		// grow
		void* p = _mmalloc(size, file, line, func);

		if( p != NULL )
		{
			memcpy(p, memblock, old_size);
		}

		_mfree(memblock, file, line, func);
		return p;
	}
}

char* _mstrdup(const char* p, const char* file, int line, const char* func)
{
	if( p == NULL )
	{
		return NULL;
	}
	else
	{
		size_t len = strlen(p);
		char *string  = (char *)_mmalloc(len + 1, file, line, func);

		memcpy(string, p, len+1);

		return string;
	}
}

void _mfree(void* ptr, const char* file, int line, const char* func)
{
	struct unit_head* head;

	if( ptr == NULL )
		return; 

	head = memmgr_memblock2unit_head(ptr);

	if( head->size == 0 )
	{
		/* malloc() で直に確保された領域 */
		struct unit_head_large* head_large = memmgr_memblock2unit_head_large(ptr);

		if( memmgr_unit_tail_large(head_large)[0] != TAILCHECK_VALUE )
		{
			ShowError("Memory manager: args of aFree 0x%p is overflowed pointer %s line %d\n", ptr, file, line);
		}
		else
		{
			head->size = 0xFFFF;

			if( head_large->prev )
			{
				head_large->prev->next = head_large->next;
			}
			else
			{
				unit_head_large_first  = head_large->next;
			}
			if( head_large->next )
			{
				head_large->next->prev = head_large->prev;
			}

			memmgr_usage_decrease(head_large->size);

#ifdef DEBUG_MEMMGR
			// set freed memory to 0xfd
			memset(ptr, 0xfd, head_large->size);
#endif

			FREE(head_large, file, line, func);
		}
	}
	else
	{
		/* ユニット解放 */
		struct block *block = head->block;

		if( (char*)head - (char*)block > sizeof(struct block) )
		{
			ShowError("Memory manager: args of aFree 0x%p is invalid pointer %s line %d\n", ptr, file, line);
		}
		else if( head->block == NULL )
		{
			ShowError("Memory manager: args of aFree 0x%p is freed pointer %s:%d@%s\n", ptr, file, line, func);
		}
		else if( memmgr_unit_tail(head)[0] != TAILCHECK_VALUE )
		{
			ShowError("Memory manager: args of aFree 0x%p is overflowed pointer %s line %d\n", ptr, file, line);
		}
		else
		{
			memmgr_usage_decrease(head->size);

			head->block = NULL;

#ifdef DEBUG_MEMMGR
			memset(ptr, 0xfd, block->unit_size - ( (uintptr_t)&head->checksum - (uintptr_t)head ) );
			head->file = file;
			head->line = line;
#endif

			memmgr_assert( block->unit_used > 0 );

			if( --block->unit_used == 0 )
			{
				/* ブロックの解放 */
				block_free(block);
			}
			else
			{
				if( block->unfill_prev == NULL )
				{
					// unfill リストに追加
					if( hash_unfill[ block->unit_hash ] )
					{
						hash_unfill[ block->unit_hash ]->unfill_prev = block;
					}
					block->unfill_prev = &block_head;
					block->unfill_next = hash_unfill[ block->unit_hash ];
					hash_unfill[ block->unit_hash ] = block;
				}
				head->size     = block->unit_unfill;
				block->unit_unfill = (unsigned short)(((uintptr_t)head - (uintptr_t)block->data) / block->unit_size);
			}
		}
	}
}

/* ブロックを確保する */
static struct block* block_malloc(unsigned short hash)
{
	int i;
	struct block *p;

	if( hash_unfill[0] != NULL )
	{
		/* ブロック用の領域は確保済み */
		p = hash_unfill[0];
		hash_unfill[0] = hash_unfill[0]->unfill_next;
	}
	else
	{
		/* ブロック用の領域を新たに確保する */
		p = (struct block*)MALLOC(sizeof(struct block) * (BLOCK_ALLOC), __FILE__, __LINE__, __func__ );

		if( p == NULL )
		{
			ShowFatalError("Memory manager::block_alloc failed.\n");
			exit(EXIT_FAILURE);
		}

		if( block_first == NULL )
		{
			/* 初回確保 */
			block_first = p;
		}
		else
		{
			block_last->block_next = p;
		}

		block_last = &p[BLOCK_ALLOC - 1];
		block_last->block_next = NULL;

		/* ブロックを連結させる */
		for( i = 0; i < BLOCK_ALLOC; i++ )
		{
			if( i != 0 )
			{
				// p[0] はこれから使うのでリンクには加えない
				p[i].unfill_next = hash_unfill[0];
				hash_unfill[0]   = &p[i];
				p[i].unfill_prev = NULL;
				p[i].unit_used = 0;
			}
			if( i != BLOCK_ALLOC-1 )
			{
				p[i].block_next = &p[i+1];
			}
		}
	}

	// unfill に追加
	memmgr_assert(hash_unfill[ hash ] == NULL);
	hash_unfill[ hash ] = p;
	p->unfill_prev  = &block_head;
	p->unfill_next  = NULL;
	p->unit_size    = (unsigned short)(hash2size( hash ) + sizeof(struct unit_head));
	p->unit_hash    = hash;
	p->unit_count   = BLOCK_DATA_SIZE / p->unit_size;
	p->unit_used    = 0;
	p->unit_unfill  = 0xFFFF;
	p->unit_maxused = 0;

#ifdef DEBUG_MEMMGR
	memset( p->data, 0xfd, sizeof(p->data) );
#endif

	return p;
}

static void block_free(struct block* p)
{
	if( p->unfill_prev )
	{
		if( p->unfill_prev == &block_head )
		{
			hash_unfill[ p->unit_hash ] = p->unfill_next;
		}
		else
		{
			p->unfill_prev->unfill_next = p->unfill_next;
		}
		if( p->unfill_next )
		{
			p->unfill_next->unfill_prev = p->unfill_prev;
		}
		p->unfill_prev = NULL;
	}

	p->unfill_next = hash_unfill[0];
	hash_unfill[0] = p;
}

size_t memmgr_usage(void)
{
	return memmgr_usage_bytes / 1024;
}

#ifdef LOG_MEMMGR
static char memmer_logfile[128];
static FILE *log_fp;

static void memmgr_log(const char* buf, ...)
{
	va_list args;

	if( !log_fp )
	{
		time_t raw;
		struct tm* t;

		log_fp = fopen(memmer_logfile, "at");

		if( !log_fp )
			log_fp = stdout;

		time(&raw);
		t = localtime(&raw);
		fprintf(log_fp, "\nMemory manager: Memory leaks found at %d/%02d/%02d %02dh%02dm%02ds (Revision %s).\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, get_svn_revision());
	}

	va_start(args, buf);
	vfprintf(log_fp, buf, args);
	va_end(args);
}
#endif /* LOG_MEMMGR */

/// Returns true if the memory location is active.
/// Active means it is allocated and points to a usable part.
///
/// @param ptr Pointer to the memory
/// @return true if the memory is active
bool memmgr_verify(void* ptr)
{
	struct block* block = block_first;
	struct unit_head_large* large = unit_head_large_first;

	if( ptr == NULL )
		return false;// never valid

	// search small blocks
	while( block )
	{
		if( (char*)ptr >= (char*)block && (char*)ptr < ((char*)block) + sizeof(struct block) )
		{// found memory block
			if( block->unit_used && (char*)ptr >= block->data )
			{// memory block is being used and ptr points to a sub-unit
				size_t i = (size_t)((char*)ptr - block->data)/block->unit_size;
				struct unit_head* head = block2unit(block, i);
				if( i < block->unit_maxused && head->block != NULL )
				{// memory unit is allocated, check if ptr points to the usable part
					return ( (char*)ptr >= (char*)&head->checksum
						&& (char*)ptr < (char*)memmgr_unit_tail(head) );
				}
			}
			return false;
		}
		block = block->block_next;
	}

	// search large blocks
	while( large )
	{
		if( (char*)ptr >= (char*)large && (char*)ptr < ((char*)large) + large->size )
		{// found memory block, check if ptr points to the usable part
			return ( (char*)ptr >= (char*)&large->unit_head.checksum
				&& (char*)ptr < (char*)memmgr_unit_tail_large(large) );
		}
		large = large->next;
	}
	return false;
}

static void memmgr_final(void)
{
	struct block *block = block_first;

#ifdef LOG_MEMMGR
	int count = 0;
#endif /* LOG_MEMMGR */

	while( block )
	{
		if( block->unit_used )
		{
			int i;

			for( i = 0; i < block->unit_maxused; i++ )
			{
				struct unit_head *head = block2unit(block, i);

				if( head->block != NULL )
				{
					void* ptr = &head->checksum;
#ifdef LOG_MEMMGR
					memmgr_log("%04d : %s line %d size %lu address 0x%p\n", ++count, head->file, head->line, (unsigned long)head->size, ptr);
#endif /* LOG_MEMMGR */
					// get block pointer and free it [celest]
					_mfree(ptr, ALC_MARK);
				}
			}
		}
		block = block->block_next;
	}

	while( unit_head_large_first )
	{
		struct unit_head_large* large = unit_head_large_first;

#ifdef LOG_MEMMGR
		memmgr_log("%04d : %s line %d size %lu address 0x%p\n", ++count, large->unit_head.file, large->unit_head.line, (unsigned long)large->size, &large->unit_head.checksum);
#endif /* LOG_MEMMGR */

		_mfree(large, ALC_MARK);
	}

#ifdef LOG_MEMMGR
	if( count == 0 )
	{
		ShowInfo("Memory manager: No memory leaks found.\n");
	}
	else
	{
		ShowWarning("Memory manager: Memory leaks found and fixed.\n");
		fclose(log_fp);
	}
#endif /* LOG_MEMMGR */
}

static void memmgr_init(void)
{
#ifdef LOG_MEMMGR
	memset(hash_unfill, 0, sizeof(hash_unfill));
	sprintf(memmer_logfile, "log/%s.leaks", SERVER_NAME);
	ShowStatus("Memory manager initialised: "CL_WHITE"%s"CL_RESET"\n", memmer_logfile);
#endif /* LOG_MEMMGR */
}
#endif /* USE_MEMMGR */


/*======================================
 * Initialise
 *--------------------------------------
 */


/// Tests the memory for errors and memory leaks.
void malloc_memory_check(void)
{
	MEMORY_CHECK();
}


/// Returns true if a pointer is valid.
/// The check is best-effort, false positives are possible.
bool malloc_verify_ptr(void* ptr)
{
#ifdef USE_MEMMGR
	return memmgr_verify(ptr) && MEMORY_VERIFY(ptr);
#else
	return MEMORY_VERIFY(ptr);
#endif
}


size_t malloc_usage(void)
{
#ifdef USE_MEMMGR
	return memmgr_usage();
#else
	return MEMORY_USAGE();
#endif
}

void malloc_final(void)
{
#ifdef USE_MEMMGR
	memmgr_final();
#endif
	MEMORY_CHECK();
}

void malloc_init(void)
{
#if defined(DMALLOC) && defined(CYGWIN)
	// http://dmalloc.com/docs/latest/online/dmalloc_19.html
	dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));
#endif
#ifdef GCOLLECT
	// don't garbage collect, only report inaccessible memory that was not deallocated
	GC_find_leak = 1;
	GC_INIT();
#endif
#ifdef USE_MEMMGR
	memmgr_init();
#endif
}
