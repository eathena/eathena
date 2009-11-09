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
#include "../common/txt.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
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

	// data provider
	CSDB_TXT* db;

} MemoDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool memo_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	memolist* list = (memolist*)data;

	*out_size = sizeof(*list);

	if( size < sizeof(*list) )
		return true;

	if( version == 20090825 )
	{
		const char* p = str;
		int char_id, i, n;
		char memo_map[256];
		int memo_x, memo_y;
		Txt* txt;
		bool done = false;

		// load char id
		if( sscanf(p, "%d%n", &char_id, &n) != 1 || p[n] != '\t' )
			return false;

		p += n + 1;
		memset(list, 0, sizeof(*list));

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)p, strlen(p), 3, ',', ' ', "");
		Txt_Bind(txt, 0, TXTDT_CSTRING, memo_map, sizeof(memo_map));
		Txt_Bind(txt, 1, TXTDT_INT, &memo_x, sizeof(memo_x));
		Txt_Bind(txt, 2, TXTDT_INT, &memo_y, sizeof(memo_y));

		i = 0;
		while( Txt_Parse(txt) == TXT_SUCCESS )
		{
			if( Txt_NumFields(txt) == 0 )
			{// no more data
				done = true;
				break;
			}

			if( Txt_NumFields(txt) != 3 )
				break; // parsing failure

			if( i >= MAX_MEMOPOINTS )
				continue; // TODO: warning?

			(*list)[i].map = mapindex_name2id(memo_map);
			(*list)[i].x = memo_x;
			(*list)[i].y = memo_y;
			
			i++;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		*key = char_id;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool memo_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int char_id = key;
	memolist* list = (memolist*)data;
	int i;
	int count = 0;
	Txt* txt;

	// write char id
	p += sprintf(p, "%d\t", char_id);

	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 3, ',', ' ', ", ");

	for( i = 0; i < MAX_MEMOPOINTS; ++i )
	{
		char memo_map[MAP_NAME_LENGTH];

		if( (*list)[i].map == 0 )
			continue;

		safestrncpy(memo_map, mapindex_id2name((*list)[i].map), sizeof(memo_map));
		Txt_Bind(txt, 0, TXTDT_CSTRING, memo_map, sizeof(memo_map));
		Txt_Bind(txt, 1, TXTDT_SHORT, &(*list)[i].x, sizeof((*list)[i].x));
		Txt_Bind(txt, 2, TXTDT_SHORT, &(*list)[i].y, sizeof((*list)[i].y));

		if( Txt_Write(txt) != TXT_SUCCESS )
		{
			Txt_Free(txt);
			return false;
		}

		count++;
	}

	Txt_Free(txt);

	if( count == 0 )
		str[0] = '\0';

	return true;
}


/// @protected
static bool memo_db_txt_init(MemoDB* self)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void memo_db_txt_destroy(MemoDB* self)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool memo_db_txt_sync(MemoDB* self, bool force)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool memo_db_txt_remove(MemoDB* self, const int char_id)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// @protected
static bool memo_db_txt_save(MemoDB* self, const memolist* list, const int char_id)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	return db->replace(db, char_id, list, sizeof(*list));
}


/// @protected
static bool memo_db_txt_load(MemoDB* self, memolist* list, const int char_id)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;

	if( !db->load(db, char_id, list, sizeof(*list), NULL) )
		memset(list, 0, sizeof(*list));

	return true;
}


/// Returns an iterator over all memo lists.
/// @protected
static CSDBIterator* memo_db_txt_iterator(MemoDB* self)
{
	CSDB_TXT* db = ((MemoDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new MemoDB interface.
/// @protected
MemoDB* memo_db_txt(CharServerDB_TXT* owner)
{
	MemoDB_TXT* db = (MemoDB_TXT*)aCalloc(1, sizeof(MemoDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_memos, MEMODB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &memo_db_txt_fromstr;
	db->db->p.tostr   = &memo_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &memo_db_txt_init;
	db->vtable.p.destroy = &memo_db_txt_destroy;
	db->vtable.p.sync    = &memo_db_txt_sync;
	db->vtable.remove    = &memo_db_txt_remove;
	db->vtable.save      = &memo_db_txt_save;
	db->vtable.load      = &memo_db_txt_load;
	db->vtable.iterator  = &memo_db_txt_iterator;

	return &db->vtable;
}
