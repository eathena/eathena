// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/txt.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "statusdb.h"
#include <limits.h> // SIZE_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define STATUSDB_TXT_DB_VERSION 20090907


/// Internal structure.
/// @private
typedef struct StatusDB_TXT
{
	// public interface
	StatusDB vtable;

	// data provider
	CSDB_TXT* db;

} StatusDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool status_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct status_change_data* sc = (struct status_change_data*)data;

	if( version == 20090907 )
	{
		Txt* txt;
		int char_id, n, i;
		struct status_change_data tmp;
		bool done = false;

		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		memset(&tmp, 0, sizeof(tmp));
		*out_size = 0;

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 6, ',', ' ', "");
		Txt_Bind(txt, 0, TXTDT_USHORT, &tmp.type, sizeof(tmp.type));
		Txt_Bind(txt, 1, TXTDT_INT, &tmp.tick, sizeof(tmp.tick));
		Txt_Bind(txt, 2, TXTDT_INT, &tmp.val1, sizeof(tmp.val1));
		Txt_Bind(txt, 3, TXTDT_INT, &tmp.val2, sizeof(tmp.val2));
		Txt_Bind(txt, 4, TXTDT_INT, &tmp.val3, sizeof(tmp.val3));
		Txt_Bind(txt, 5, TXTDT_INT, &tmp.val4, sizeof(tmp.val4));

		i = 0;
		while( !done )
		{
			if( Txt_Parse(txt) != TXT_SUCCESS )
				break;

			if( Txt_NumFields(txt) == 0 )
			{// no more data
				done = true;
				break;
			}

			if( Txt_NumFields(txt) != 6 )
				break; // parsing failure

			*out_size += sizeof(struct status_change_data);

			if( (i+1) * sizeof(struct status_change_data) > size )
				continue; // discard entries over max

			memcpy(&sc[i], &tmp, sizeof(tmp));
			memset(&tmp, 0, sizeof(tmp));
			i++;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		*key = char_id;
	}
	else
	if( version == 00000000 )
	{
		int account_id, char_id, count, n, i;
		struct status_change_data tmp;

		if( sscanf(str, "%d,%d,%d%n", &account_id, &char_id, &count, &n) != 3 && str[n] != '\t' )
			return false;

		str += n + 1;

		memset(&tmp, 0, sizeof(tmp));
		*out_size = 0;

		for( i = 0; i < count; i++ )
		{
			if( sscanf(str, "%hu,%d,%d,%d,%d,%d%n", &tmp.type, &tmp.tick, &tmp.val1, &tmp.val2, &tmp.val3, &tmp.val4, &n) < 6 || str[n] != '\t' )
				return false;

			str += n + 1;
			*out_size += sizeof(struct status_change_data);

			if( (i+1) * sizeof(struct status_change_data) > size )
				continue; // discard entries over max

			memcpy(&sc[i], &tmp, sizeof(tmp));
			memset(&tmp, 0, sizeof(tmp));
		}

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
static bool status_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	char* p = str;
	int char_id = key;
	struct status_change_data* sc = (struct status_change_data*)data;
	size_t i;
	int count = 0;
	Txt* txt;

	p += sprintf(p, "%d\t", char_id);

	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 6, ',', ' ', ", ");

	for( i = 0; i < datasize / sizeof(struct status_change_data); i++ )
	{
		if( sc[i].type == (unsigned short)-1 )
			continue;

		Txt_Bind(txt, 0, TXTDT_USHORT, &sc[i].type, sizeof(sc[i].type));
		Txt_Bind(txt, 1, TXTDT_INT, &sc[i].tick, sizeof(sc[i].tick));
		Txt_Bind(txt, 2, TXTDT_INT, &sc[i].val1, sizeof(sc[i].val1));
		Txt_Bind(txt, 3, TXTDT_INT, &sc[i].val2, sizeof(sc[i].val2));
		Txt_Bind(txt, 4, TXTDT_INT, &sc[i].val3, sizeof(sc[i].val3));
		Txt_Bind(txt, 5, TXTDT_INT, &sc[i].val4, sizeof(sc[i].val4));

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
static bool status_db_txt_init(StatusDB* self)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void status_db_txt_destroy(StatusDB* self)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool status_db_txt_sync(StatusDB* self, bool force)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool status_db_txt_remove(StatusDB* self, int char_id)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// @protected
static bool status_db_txt_save(StatusDB* self, const struct status_change_data* sc, int count, int char_id)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	return db->replace(db, char_id, sc, count * sizeof(struct status_change_data));
}


/// @protected
static bool status_db_txt_load(StatusDB* self, struct status_change_data* sc, int count, int char_id)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;

	if( !db->load(db, char_id, sc, count * sizeof(struct status_change_data), NULL) )
		memset(sc, 0, count * sizeof(struct status_change_data));

	return true;
}


/// Gives the number of status changes stored for this character.
/// @protected
static int status_db_txt_count(StatusDB* self, int char_id)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	size_t size;

	if( !db->load(db, char_id, NULL, 0, &size) )
		size = 0;

	return size / sizeof(struct status_change_data);
}


/// Returns an iterator over all status entries.
/// @protected
static CSDBIterator* status_db_txt_iterator(StatusDB* self)
{
	CSDB_TXT* db = ((StatusDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new StatusDB interface.
/// @protected
StatusDB* status_db_txt(CharServerDB_TXT* owner)
{
	StatusDB_TXT* db = (StatusDB_TXT*)aCalloc(1, sizeof(StatusDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_statuses, STATUSDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &status_db_txt_fromstr;
	db->db->p.tostr   = &status_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &status_db_txt_init;
	db->vtable.p.destroy = &status_db_txt_destroy;
	db->vtable.p.sync    = &status_db_txt_sync;
	db->vtable.remove    = &status_db_txt_remove;
	db->vtable.save      = &status_db_txt_save;
	db->vtable.load      = &status_db_txt_load;
	db->vtable.count     = &status_db_txt_count;
	db->vtable.iterator  = &status_db_txt_iterator;

	return &db->vtable;
}
