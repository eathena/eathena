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
#include "questdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define QUESTDB_TXT_DB_VERSION 20090920


/// Internal structure.
/// @private
typedef struct QuestDB_TXT
{
	// public interface
	QuestDB vtable;

	// data provider
	CSDB_TXT* db;

} QuestDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool quest_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct quest* log = (struct quest*)data;

	if( version == 20090920 )
	{// quest blocks separated by tabs, quest fields separated by commas, 6 fields total
		Txt* txt;
		int char_id, n, i;
		struct quest qd;
		bool done = false;

		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		memset(&qd, 0, sizeof(qd));
		*out_size = 0;

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 6, ',', '\t', "");
		Txt_Bind(txt, 0, TXTDT_INT, &qd.quest_id, sizeof(qd.quest_id));
		Txt_Bind(txt, 1, TXTDT_ENUM, &qd.state, sizeof(qd.state));
		Txt_Bind(txt, 2, TXTDT_UINT, &qd.time, sizeof(qd.time));
		Txt_Bind(txt, 3, TXTDT_INT, &qd.count[0], sizeof(qd.count[0]));
		Txt_Bind(txt, 4, TXTDT_INT, &qd.count[1], sizeof(qd.count[1]));
		Txt_Bind(txt, 5, TXTDT_INT, &qd.count[2], sizeof(qd.count[2]));

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

			*out_size += sizeof(qd);

			if( (i + 1) * sizeof(qd) > size )
				continue; // discard entries over max

			memcpy(&log[i], &qd, sizeof(qd));
			memset(&qd, 0, sizeof(qd));
			i++;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		*key = char_id;
	}
	else
	if( version == 20090801 )
	{// quest blocks separated by tabs, quest fields separated by commas, 10 fields total
		Txt* txt;
		int char_id, n, i;
		struct quest qd;
		bool done = false;

		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		memset(&qd, 0, sizeof(qd));
		*out_size = 0;

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 6, ',', '\t', "");
		Txt_Bind(txt, 0, TXTDT_INT, &qd.quest_id, sizeof(qd.quest_id));
		Txt_Bind(txt, 1, TXTDT_ENUM, &qd.state, sizeof(qd.state));
		Txt_Bind(txt, 2, TXTDT_UINT, &qd.time, sizeof(qd.time));
		//Txt_Bind(txt, 3, TXTDT_INT, &qd.num_objectives, sizeof(qd.num_objectives));
		//Txt_Bind(txt, 4, TXTDT_INT, &dq.mob[0], sizeof(qd.mob[0]));
		Txt_Bind(txt, 5, TXTDT_INT, &qd.count[0], sizeof(qd.count[0]));
		//Txt_Bind(txt, 6, TXTDT_INT, &dq.mob[1], sizeof(qd.mob[1]));
		Txt_Bind(txt, 7, TXTDT_INT, &qd.count[1], sizeof(qd.count[1]));
		//Txt_Bind(txt, 8, TXTDT_INT, &dq.mob[2], sizeof(qd.mob[2]));
		Txt_Bind(txt, 9, TXTDT_INT, &qd.count[2], sizeof(qd.count[2]));

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

			if( Txt_NumFields(txt) != 10 )
				break; // parsing failure

			*out_size += sizeof(qd);

			if( (i + 1) * sizeof(qd) > size )
				continue; // discard entries over max

			memcpy(&log[i], &qd, sizeof(qd));
			memset(&qd, 0, sizeof(qd));
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
static bool quest_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	char* p = str;
	int char_id = key;
	struct quest* log = (struct quest*)data;
	size_t i;
	int count = 0;
	Txt* txt;

	p += sprintf(p, "%d\t", char_id);

	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 7+MAX_SLOTS, ',', '\t', ",\t");

	for( i = 0; i < datasize / sizeof(*log); ++i )
	{
		if( log[i].quest_id == 0 )
			continue;

		Txt_Bind(txt, 0, TXTDT_INT,  &log[i].quest_id, sizeof(log[i].quest_id));
		Txt_Bind(txt, 1, TXTDT_ENUM, &log[i].state,    sizeof(log[i].state)   );
		Txt_Bind(txt, 2, TXTDT_UINT, &log[i].time,     sizeof(log[i].time)    );
		Txt_Bind(txt, 3, TXTDT_INT,  &log[i].count[0], sizeof(log[i].count[0]));
		Txt_Bind(txt, 4, TXTDT_INT,  &log[i].count[1], sizeof(log[i].count[1]));
		Txt_Bind(txt, 5, TXTDT_INT,  &log[i].count[2], sizeof(log[i].count[2]));

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
static bool quest_db_txt_init(QuestDB* self)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void quest_db_txt_destroy(QuestDB* self)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool quest_db_txt_sync(QuestDB* self, bool force)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool quest_db_txt_remove(QuestDB* self, const int char_id)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// Saves the questlog.
/// @param size Number of fields in the array to process.
/// @protected
static bool quest_db_txt_save(QuestDB* self, const struct quest* log, size_t size, int char_id)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	size_t i;

	ARR_FIND(0, size, i, log[i].quest_id > 0);
	if( i < size )
		return db->replace(db, char_id, log, size * sizeof(*log));
	else
		return db->remove(db, char_id);
}


/// Loads a character's questlog into the provided array.
/// @param size Capacity of the array, in fields.
/// @protected
static bool quest_db_txt_load(QuestDB* self, struct quest* log, size_t size, int char_id)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	size_t insize = size * sizeof(*log);
	size_t outsize;
	int count;

	if( !db->load(db, char_id, log, insize, &outsize) )
		outsize = 0;

	count = outsize / sizeof(*log);
	memset(&log[count], 0, insize - outsize);

	return true;
}


/// Gives the number of questlog entries stored for this character.
/// @protected
static int quest_db_txt_count(QuestDB* self, int char_id)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	size_t size;

	if( !db->load(db, char_id, NULL, 0, &size) )
		size = 0;

	return size / sizeof(struct quest);
}


/// Returns an iterator over all quest entries.
/// @protected
static CSDBIterator* quest_db_txt_iterator(QuestDB* self)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new QuestDB interface.
/// @protected
QuestDB* quest_db_txt(CharServerDB_TXT* owner)
{
	QuestDB_TXT* db = (QuestDB_TXT*)aCalloc(1, sizeof(QuestDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_quests, QUESTDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &quest_db_txt_fromstr;
	db->db->p.tostr   = &quest_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &quest_db_txt_init;
	db->vtable.p.destroy = &quest_db_txt_destroy;
	db->vtable.p.sync    = &quest_db_txt_sync;
	db->vtable.remove    = &quest_db_txt_remove;
	db->vtable.save      = &quest_db_txt_save;
	db->vtable.load      = &quest_db_txt_load;
	db->vtable.count     = &quest_db_txt_count;
	db->vtable.iterator  = &quest_db_txt_iterator;

	return &db->vtable;
}
