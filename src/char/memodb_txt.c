// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "memodb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define MEMODB_TXT_DB_VERSION 20090825


/// Internal structure.
/// @private
typedef struct MemoDB_TXT
{
	// public interface
	MemoDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* memos;
	bool dirty;

	// settings
	const char* memo_db;

} MemoDB_TXT;


/// @private
static void* create_memolist(DBKey key, va_list args)
{
	return (memolist*)aMalloc(sizeof(memolist));
}


/// @private
static bool mmo_memolist_fromstr(memolist* list, char* str)
{
	const char* p = str;
	bool first = true;
	int n = 0;
	int i;

	memset(list, 0, sizeof(*list));

	for( i = 0; *p != '\0' && *p != '\n' && *p != '\r'; ++i )
	{
		int tmp_int[3];
		char tmp_str[256];

		if( first )
			first = false;
		else
		if( *p == ' ' )
			p++;
		else
			return false;

		if( sscanf(p, "%255[^,],%d,%d%n", tmp_str, &tmp_int[1], &tmp_int[2], &n) != 3 )
			return false;

		tmp_int[0] = mapindex_name2id(tmp_str);


		p += n;

		if( i == MAX_MEMOPOINTS )
			continue; // TODO: warning?

		(*list)[i].map = tmp_int[0];
		(*list)[i].x = tmp_int[1];
		(*list)[i].y = tmp_int[2];
	}

	return true;
}


/// @private
static bool mmo_memolist_tostr(const memolist* list, char* str)
{
	char* p = str;
	bool first = true;
	int i;

	for( i = 0; i < MAX_MEMOPOINTS; ++i )
	{
		if( (*list)[i].map == 0 )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, " ");

		p += sprintf(p, "%s,%d,%d", mapindex_id2name((*list)[i].map), (*list)[i].x, (*list)[i].y);
	}

	*p = '\0';

	return true;
}


/// @protected
static bool memo_db_txt_init(MemoDB* self)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBMap* memos;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create memo database
	if( db->memos == NULL )
		db->memos = idb_alloc(DB_OPT_RELEASE_DATA);
	memos = db->memos;
	db_clear(memos);

	// open data file
	fp = fopen(db->memo_db, "r");
	if( fp == NULL )
	{
		ShowError("Memo file not found: %s.\n", db->memo_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id, n;
		unsigned int v;
		memolist* list;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		list = (memolist*)aCalloc(1, sizeof(memolist));
		if( list == NULL )
		{
			ShowFatalError("memo_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n\t", &char_id, &n) != 1 || line[n] != '\t' )
		{
			aFree(list);
			continue;
		}

		// load memos for this char
		if( !mmo_memolist_fromstr(list, line + n + 1) )
		{
			ShowError("memo_db_txt_init: skipping invalid data: %s", line);
			aFree(list);
			continue;
		}
	
		// record entry in db
		idb_put(memos, char_id, list);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void memo_db_txt_destroy(MemoDB* self)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBMap* memos = db->memos;

	// delete memo database
	if( memos != NULL )
	{
		db_destroy(memos);
		db->memos = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool memo_db_txt_sync(MemoDB* self)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->memo_db, &lock);
	if( fp == NULL )
	{
		ShowError("memo_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->memo_db);
		return false;
	}

	fprintf(fp, "%d\n", MEMODB_TXT_DB_VERSION);

	iter = db->memos->iterator(db->memos);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		memolist* list = (memolist*) data;
		char line[8192];

		mmo_memolist_tostr(list, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->memo_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool memo_db_txt_remove(MemoDB* self, const int char_id)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBMap* memos = db->memos;

	idb_remove(memos, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool memo_db_txt_save(MemoDB* self, const memolist* list, const int char_id)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBMap* memos = db->memos;

	// retrieve previous data / allocate new data
	memolist* tmp = idb_ensure(memos, char_id, create_memolist);
	if( tmp == NULL )
	{// error condition - allocation problem?
		return false;
	}

	// overwrite with new data
	memcpy(tmp, list, sizeof(memolist));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool memo_db_txt_load(MemoDB* self, memolist* list, const int char_id)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	DBMap* memos = db->memos;

	memolist* tmp = idb_get(memos, char_id);

	if( tmp != NULL )
		memcpy(list, tmp, sizeof(memolist));
	else
		memset(list, 0, sizeof(memolist));

	return true;
}


/// Returns an iterator over all memo lists.
/// @protected
static CSDBIterator* memo_db_txt_iterator(MemoDB* self)
{
	MemoDB_TXT* db = (MemoDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->memos));
}


/// Constructs a new MemoDB interface.
/// @protected
MemoDB* memo_db_txt(CharServerDB_TXT* owner)
{
	MemoDB_TXT* db = (MemoDB_TXT*)aCalloc(1, sizeof(MemoDB_TXT));

	// set up the vtable
	db->vtable.init      = &memo_db_txt_init;
	db->vtable.destroy   = &memo_db_txt_destroy;
	db->vtable.sync      = &memo_db_txt_sync;
	db->vtable.remove    = &memo_db_txt_remove;
	db->vtable.save      = &memo_db_txt_save;
	db->vtable.load      = &memo_db_txt_load;
	db->vtable.iterator  = &memo_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->memos = NULL;
	db->dirty = true;

	// other settings
	db->memo_db = db->owner->file_memos;

	return &db->vtable;
}
