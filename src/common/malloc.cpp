// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "malloc.h"
#include "showmsg.h"
#include "utils.h"














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











#ifdef MEMTRACE

TslistDST<CMemDesc::_key>	CMemDesc::cIndex;	// the index

#endif


void* aMalloc_ (size_t size, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	void *ret = MALLOC(size);
#else
	void *ret = mwMalloc(size, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: malloc %d\n",file,line,func,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: malloc error out of memory!\n",file,line,func);
		exit(1);
	}
	return ret;
}
void* aMallocA_ (size_t size, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	void *ret = MALLOCA(size);
#else
	void *ret = mwMalloc(size, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: malloc %d\n",file,line,func,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: malloc error out of memory!\n",file,line,func);
		exit(1);
	}
	return ret;
}
void* aCalloc_ (size_t num, size_t size, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	void *ret = CALLOC(num, size);
#else
	void *ret = mwCalloc(num, size, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: calloc %d %d\n",file,line,func,num,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: calloc error out of memory!\n", file, line, func);
		exit(1);
	}
	return ret;
}
void* aCallocA_ (size_t num, size_t size, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	void *ret = CALLOCA(num, size);
#else
	void *ret = mwCalloc(num, size, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: calloc %d %d\n",file,line,func,num,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: calloc error out of memory!\n",file,line,func);
		exit(1);
	}
	return ret;
}
void* aRealloc_ (void *p, size_t size, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	void *ret = REALLOC(p, size);
#else
	void *ret = mwRealloc(p, size, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: realloc %p %d\n",file,line,func,p,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: realloc error out of memory!\n",file,line,func);
		exit(1);
	}
	return ret;
}
char* aStrdup_ (const char *p, const char *file, int line, const char *func)
{
#ifndef MEMWATCH
	char *ret = STRDUP(p);
#else
	char *ret = mwStrdup(p, file, line);
#endif
	// ShowMessage("%s:%d: in func %s: strdup %p\n",file,line,func,p);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: strdup error out of memory!\n", file, line, func);
		exit(1);
	}
	return ret;
}
void aFree_ (void *p, const char *file, int line, const char *func)
{
	// ShowMessage("%s:%d: in func %s: free %p\n",file,line,func,p);
	if (p)
	#ifndef MEMWATCH
		FREE(p);
	#else
		mwFree(p, file, line);
	#endif
}

#ifdef GCOLLECT

void* _bcallocA(size_t size, size_t cnt)
{
	void *ret = MALLOCA(size * cnt);
	if (ret) memset(ret, 0, size * cnt);
	return ret;
}
void* _bcalloc(size_t size, size_t cnt)
{
	void *ret = MALLOC(size * cnt);
	if (ret) memset(ret, 0, size * cnt);
	return ret;
}
char* _bstrdup(const char *chr)
{
	if(chr)
	{
		int len = strlen(chr);
		char *ret = (char*)MALLOC(len + 1);
		if (ret) memcpy(ret, chr, len + 1);
		return ret;
	}
	return NULL;
}

#endif

#ifdef USE_MEMMGR

// USE_MEMMGR

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
 *     ・ユニット同士はリンクリスト(block_prev,block_next) でつながり、同じサイ
 *       ズを持つユニット同士もリンクリスト(samesize_prev,samesize_nect) でつな
 *       がっています。それにより、不要となったメモリの再利用が効率的に行えます。
 */

// ブロックに入るデータ量
#define BLOCK_DATA_SIZE	80*1024

// 一度に確保するブロックの数。
#define BLOCK_ALLOC		32

// ブロックのアライメント
#define BLOCK_ALIGNMENT	64

// ブロック
struct block
{
	size_t block_no;				// ブロック番号
	struct block* block_prev;		// 前に確保した領域
	struct block* block_next;		// 次に確保した領域
	size_t samesize_no;				// 同じサイズの番号
	struct block* samesize_prev;	// 同じサイズの前の領域
	struct block* samesize_next;	// 同じサイズの次の領域
	size_t unit_size;				// ユニットのバイト数 0=未使用
	size_t unit_hash;				// ユニットのハッシュ
	size_t unit_count;				// ユニットの数
	size_t unit_used;				// 使用済みユニット
	char   data[BLOCK_DATA_SIZE];
};

struct unit_head 
{
	size_t marker;
	struct block* parent;
	size_t size;
	const char* file;
	int line;
};

static struct block* block_first  = NULL;
static struct block* block_last   = NULL;
static struct block* block_unused = NULL;

// ユニットへのハッシュ。80KB/64Byte = 1280個
static struct block* unit_first[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];	// 最初
static struct block* unit_last[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];	// 最後
static struct block* unit_empty[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];	// 埋まってない

// メモリを使い回せない領域用のデータ
struct unit_head_large {
	struct unit_head_large* prev;
	struct unit_head_large* next;
	struct unit_head        unithead;
};
static struct unit_head_large *unit_head_large_first = NULL;

struct block* block_malloc(void);
void   block_free(struct block* p);
void memmgr_info(void);

static char memmer_logfile[128];
static FILE *log_fp=NULL;

static size_t memmgr_usage_bytes = 0;


#ifdef MEMCHECKER
TslistDST<size_t> memlist;
#endif


void* _mmalloc(size_t size, const char *file, int line, const char *func )
{
	size_t i;
	struct block *block;
	size_t size_hash = (size+BLOCK_ALIGNMENT-1) / BLOCK_ALIGNMENT;
	size = size_hash * BLOCK_ALIGNMENT; // アライメントの倍数に切り上げ 

	if(size == 0)
		return NULL;

	memmgr_usage_bytes += size;

	// ブロック長を超える領域の確保には、malloc() を用いる 
	// その際、unit_head.block に NULL を代入して区別する 
	if( size+sizeof(struct unit_head) > BLOCK_DATA_SIZE )
	{
#ifdef MEMWATCH
		struct unit_head_large* p = (struct unit_head_large*)mwMalloc(sizeof(struct unit_head_large) + size,file,line);
#else
		struct unit_head_large* p = (struct unit_head_large*) MALLOC (sizeof(struct unit_head_large) + size);
#endif
		if(p != NULL)
		{
			memset(p,0,sizeof(struct unit_head_large)+size);
			p->unithead.marker = 0xC3C3A5A5;
			p->unithead.parent = NULL;
			p->unithead.size  = size;
			p->unithead.file = file;
			p->unithead.line = line;


			if(unit_head_large_first)
				unit_head_large_first->prev = p;
			p->prev = NULL;
			p->next = unit_head_large_first;
			unit_head_large_first = p;

#ifdef MEMCHECKER
			memlist.insert( (size_t)(((char *)p) + sizeof(struct unit_head_large)) );
#endif
			return ((char *)p) + sizeof(struct unit_head_large);
		}
		else
		{
			ShowFatalError("MEMMGR::memmgr_alloc failed.\n");
			exit(1);
		}
	}

	// 同一サイズのブロックが確保されていない時、新たに確保する
	if(unit_empty[size_hash] == NULL)
	{
		block = block_malloc();
		if(unit_first[size_hash] == NULL)
		{	// 初回確保
			unit_first[size_hash] = block;
			unit_last[size_hash] = block;
			block->samesize_no = 0;
			block->samesize_prev = NULL;
			block->samesize_next = NULL;
		}
		else
		{	// 連結作業
			unit_last[size_hash]->samesize_next = block;
			block->samesize_no   = unit_last[size_hash]->samesize_no + 1;
			block->samesize_prev = unit_last[size_hash];
			block->samesize_next = NULL;
			unit_last[size_hash] = block;
		}
		unit_empty[size_hash] = block;
		block->unit_size  = size + sizeof(struct unit_head);
		block->unit_count = BLOCK_DATA_SIZE / block->unit_size;
		block->unit_used  = 0;
		block->unit_hash  = size_hash;
		// 未使用Flagを立てる
		for(i=0;i<block->unit_count;++i)
		{
			struct unit_head *head = (struct unit_head*)(block->data + i*block->unit_size);
			head->marker = 0xA5A5C3C3;
			head->parent = NULL;
		}
	}
	// ユニット使用個数加算
	block = unit_empty[size_hash];
	block->unit_used++;

	// ユニット内を全て使い果たした
	if(block->unit_used>=block->unit_count)
	{
		do
		{
			unit_empty[size_hash] = unit_empty[size_hash]->samesize_next;
		} while( unit_empty[size_hash] != NULL && 
				 unit_empty[size_hash]->unit_used >= unit_empty[size_hash]->unit_count);
	}

	// ブロックの中の空きユニット捜索
	for(i=0;i<block->unit_count;++i)
	{
		struct unit_head *head = (struct unit_head*)(block->data + i*block->unit_size);
		if(head->marker != 0xA5A5C3C3)
		{
			ShowError("Memory Malloc: block %i,%i was propably overwritten\n", block->block_no, i);
			ShowMessage("old(%d %i %i) new(%d %i %i)\n", 
				head->file, head->line, head->size, file, line, size);
			if(i>0)
			{
				struct unit_head *head2 = (struct unit_head*)(block->data + (i-1)*block->unit_size);
				ShowMessage("previous(%d %i %i)\n", 
					head2->file, head2->line, head2->size);
			}
			memmgr_info();
		}
		else if(head->parent == NULL)
		{
			head->parent = block;
			head->size  = size;
			head->line  = line;
			head->file  = file;
#ifdef MEMCHECKER
			memlist.insert( (size_t)((char *)head) + sizeof(struct unit_head) );
#endif
			return ((char *)head) + sizeof(struct unit_head);
		}
	}
	// ここに来てはいけない。
	ShowFatalError("MEMMGR::memmgr_malloc() serious error.\n");
	memmgr_info();
	exit(1);
//	return NULL;
};

void* _mcalloc(size_t num, size_t size, const char *file, int line, const char *func )
{
	void *p = _mmalloc(num * size,file,line,func);
	memset(p,0,num * size);
	return p;
}

void* _mrealloc(void *memblock, size_t size, const char *file, int line, const char *func )
{
	struct unit_head *head;
	if(memblock == NULL)
	{
		return _mmalloc(size,file,line,func);
	}
#ifdef MEMCHECKER
	size_t pos;
	if( !memlist.find( (size_t)memblock, 0, pos ) )
	{	
		ShowFatalError("Realloc on non-aquired memory");
		// do crash explicitely
		char *p=NULL;
		*p = 0;
	}
#endif
	head = (struct unit_head *)((char *)memblock - sizeof(struct unit_head));
	if(head->marker != 0xA5A5C3C3 && head->marker != 0xC3C3A5A5)
	{
		ShowError("Memory Realloc: pointer %p was propably overwritten\n", memblock);
		ShowMessage("old(%d %i %i) new(%d %i %i)\n", 
			head->file, head->line, head->size, file, line, size);

		memmgr_info();
		// allocate a new block without freeing the overwritten one
		void *p = _mmalloc(size,file,line,func);
		if(p != NULL)
		{
			memcpy(p,memblock,head->size);
		}
		_mfree(memblock,file,line,func);
		return p;
	}
	else if(head->size >= size)
	{	// サイズ縮小 -> そのまま返す（手抜き）
		return memblock;
	}
	else
	{
		// サイズ拡大
		void *p = _mmalloc(size,file,line,func);
		if(p != NULL)
		{
			memcpy(p,memblock,head->size);
		}
		_mfree(memblock,file,line,func);
		return p;
	}
}

char* _mstrdup(const char *p, const char *file, int line, const char *func )
{
	if( NULL!=p )
	{
		size_t  len = 1+strlen(p);
		char *string  = (char *)_mmalloc(len,file,line,func);
		memcpy(string,p,len);
		return string;
	}
	return NULL;
}

void _mfree(void *ptr, const char *file, int line, const char *func )
{
#ifdef MEMCHECKER
	size_t pos;
	if( !memlist.find( (size_t)ptr, 0, pos ) )
	{	
		ShowFatalError("Free on non-aquired memory");
		// do crash explicitely
		char *p=NULL;
		*p = 0;
	}
	else
	{
		memlist.removeindex(pos);
	}
#endif

	struct unit_head *head = (struct unit_head *)((char *)ptr - sizeof(struct unit_head));

	if (ptr == NULL)
	{
		return; 
	}

	if(head->marker != 0xA5A5C3C3 && head->marker != 0xC3C3A5A5)
	{
		ShowError("Memory Free: Pointer %p was propably overwritten", head);
		ShowMessage("old(%d %i %i) new(%d %i)\n", 
			head->file, head->line, head->size, file, line);
		memmgr_info();
	}
	else if( head->parent == NULL && head->size+sizeof(struct unit_head) > BLOCK_DATA_SIZE && head->marker == 0xC3C3A5A5)
	{	// malloc() で直に確保された領域
		struct unit_head_large *head_large = (struct unit_head_large *)((char *)ptr - sizeof(struct unit_head_large));
		if(head_large->prev)
			head_large->prev->next = head_large->next;
		else
			unit_head_large_first  = head_large->next;
		if(head_large->next)
			head_large->next->prev = head_large->prev;
		memmgr_usage_bytes -= head->size;
		FREE (head_large);
		return;
	}
	else if(head->marker == 0xA5A5C3C3)
	{	// ユニット解放
		struct block *block = head->parent;
		if(!block || block->unit_used==0)
		{
			ShowError("Memory manager: args of aFree is freed pointer %s line %d\n", file, line);
			return;
		}
		else
		{
			head->parent = NULL;
			memmgr_usage_bytes -= head->size;
			if(--block->unit_used == 0)
			{	// ブロックの解放
				if(unit_empty[block->unit_hash] == block)
				{	// 空きユニットに指定されている
					do {
						unit_empty[block->unit_hash] = unit_empty[block->unit_hash]->samesize_next;
					} while(
						unit_empty[block->unit_hash] != NULL &&
						unit_empty[block->unit_hash]->unit_count == unit_empty[block->unit_hash]->unit_used
					);
				}
				if(block->samesize_prev == NULL && block->samesize_next == NULL)
				{	// 独立ブロックの解放
					unit_first[block->unit_hash]  = NULL;
					unit_last[block->unit_hash]   = NULL;
					unit_empty[block->unit_hash] = NULL;
				}
				else if(block->samesize_prev == NULL)
				{	// 先頭ブロックの解放
					unit_first[block->unit_hash] = block->samesize_next;
					(block->samesize_next)->samesize_prev = NULL;
				}
				else if(block->samesize_next == NULL)
				{	// 末端ブロックの解放
					unit_last[block->unit_hash] = block->samesize_prev; 
					(block->samesize_prev)->samesize_next = NULL;
				}
				else
				{	// 中間ブロックの解放
					(block->samesize_next)->samesize_prev = block->samesize_prev;
					(block->samesize_prev)->samesize_next = block->samesize_next;
				}
				block_free(block);
			}
			else
			{	// 空きユニットの再設定
				if( unit_empty[block->unit_hash] == NULL ||
					unit_empty[block->unit_hash]->samesize_no > block->samesize_no )
				{
					unit_empty[block->unit_hash] = block;
				}
			}
			ptr = NULL;
		}
	}
}

// 現在の状況を表示する
void memmgr_info(void)
{
	size_t i;
	struct block *p;
	ShowInfo("** Memory Manager Information **\n");
	if(block_first == NULL)
	{
		ShowMessage("Uninitialized.\n");
		return;
	}
	ShowMessage("Blocks: %4u , BlockSize: %6u Byte , Used: %8uKB\n",
		block_last->block_no+1,
		sizeof(struct block),
		(block_last->block_no+1) * sizeof(struct block)/1024 );
	p = block_first;
	for(i=0;i<=block_last->block_no && p;++i)
	{
		ShowMessage("    Block #%4u : ",p->block_no);
		if(p->unit_size == 0)
		{
			ShowMessage("unused.\n");
		}
		else
		{
			ShowMessage("size: %5u byte. used: %4u/%4u prev:",
				p->unit_size - sizeof(struct unit_head),p->unit_used,p->unit_count);
			if(p->samesize_prev == NULL)
				ShowMessage("NULL");
			else
				ShowMessage("%4u", p->samesize_prev->block_no);
			ShowMessage(" next:");
			if(p->samesize_next == NULL)
				ShowMessage("NULL");
			else
				ShowMessage("%4u",p->samesize_next->block_no);
			ShowMessage("\n");
		}
		p = p->block_next;
	}
}

// ブロックを確保する
struct block* block_malloc(void)
{
	if(block_unused != NULL)
	{	// ブロック用の領域は確保済み
		struct block* ret = block_unused;
		do {
			block_unused = block_unused->block_next;
		} while(block_unused != NULL && block_unused->unit_size != 0);
		return ret;
	}
	else
	{	// ブロック用の領域を新たに確保する
		size_t i;
		size_t  block_no;
		struct block* p = (struct block *) CALLOC (sizeof(struct block),BLOCK_ALLOC);
		if(p == NULL)
		{
			ShowFatalError("MEMMGR::block_alloc failed.\n");
			exit(1);
		}
		if(block_first == NULL)
		{	// 初回確保
			block_no     = 0;
			block_first  = p;
		}
		else
		{
			block_no      = block_last->block_no + 1;
			block_last->block_next = p;
			p->block_prev = block_last;
		}
		block_last = &p[BLOCK_ALLOC-1];
		// ブロックを連結させる
		for(i=0;i<BLOCK_ALLOC;++i)
		{
			if(i != 0)
				p[i].block_prev = &p[i-1];
			if(i != BLOCK_ALLOC -1)
				p[i].block_next = &p[i+1];
			p[i].block_no = block_no + i;
		}
		// 未使用ブロックへのポインタを更新
		block_unused = &p[1];
		return p;
	}
}

void block_free(struct block* p)
{	// free() せずに、未使用フラグを付けるだけ
	p->unit_size = 0;
	// 未使用ポインターを更新する
	if(block_unused == NULL)
		block_unused = p;
	else if(block_unused->block_no > p->block_no)
		block_unused = p;
}

size_t memmgr_usage (void)
{
	return memmgr_usage_bytes / 1024;
}


#ifdef LOG_MEMMGR
void memmgr_log (char *buf)
{
	if (!log_fp)
	{
		log_fp = basics::safefopen(memmer_logfile,"w");
		if (!log_fp) log_fp = stdout;
		fprintf(log_fp, "Memory manager: Memory leaks found.\n");
	}
	fprintf(log_fp, buf);
	return;
}
#endif

void memmer_exit(void)
{
	struct block *block,*bltmp;
	struct unit_head_large *large, *large2;
	char *ptr;
	size_t i;

#ifdef LOG_MEMMGR
	int count = 0;
	char buf[128];
#endif
	
	bltmp = block = block_first;
	while (block)
	{
		// prevent references to a previously deleted block
		block_unused = NULL;

		if (block->unit_size)
		{
			for(i=0; i<block->unit_count; ++i)
			{
				struct unit_head *head = (struct unit_head*)(&block->data[block->unit_size * i]);
				if(head->parent != NULL)
				{
				#ifdef LOG_MEMMGR
					snprintf (buf, sizeof(buf),
						"%04d : %s line %d size %ld\n", ++count,
						head->file, head->line, (unsigned long)head->size);
					memmgr_log (buf);
				#endif
					// get block pointer and free it
					ptr = (char *)head + sizeof(struct unit_head);
#ifdef MEMTRACE
	CMemDesc::print(ptr);
#endif
					_mfree (ptr, ALC_MARK);
				}
			}
		}

		block = block->block_next;
		if( !block || (block >= bltmp+BLOCK_ALLOC) || (block < bltmp) )
		{	// reached a new block array
			FREE(bltmp);
			bltmp=block;
		}
	}
	block_first = NULL;

	large = unit_head_large_first;
	while(large)
	{
	#ifdef LOG_MEMMGR
		snprintf (buf, sizeof(buf),
			"%04d : %s line %d size %ld\n", ++count,
			large->unithead.file, large->unithead.line, (unsigned long)large->unithead.size);
		memmgr_log (buf);
	#endif
		
		// we're already quitting, just skip tidying things up ^^
		//if (large->prev) {
		//	large->prev->next = large->next;
		//} else {
		//	unit_head_large_first  = large->next;
		//}
		//if (large->next) {
		//	large->next->prev = large->prev;
		//}
		large2 = large->next;
		FREE (large);
		large = large2;
	}
	unit_head_large_first = NULL;


#ifdef LOG_MEMMGR
	if(count == 0) {
		ShowInfo("memmgr: no memory leaks found.\n");
	} else {
		ShowWarning("Memory manager: Memory leaks found and fixed.\n");
		fclose(log_fp);
	}
#endif
	return;
}

#endif//USE_MEMMGR



int memmgr_init(const char* file)
{
#ifdef GCOLLECT
	GC_enable_incremental();
#endif

#if defined(USE_MEMMGR) && defined(LOG_MEMMGR)
	snprintf(memmer_logfile, sizeof(memmer_logfile), "log/%s.leaks", file);
	ShowStatus("Memory manager initialised: "CL_WHITE"%s"CL_RESET"\n", memmer_logfile);
#endif
	return 0;
}

void memmgr_final()
{
#if defined(USE_MEMMGR)
	memmer_exit();
#endif
}
