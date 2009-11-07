// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
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
static bool skill_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	questlog* log = (questlog*)data;

	*out_size = sizeof(*log);

	if( size < sizeof(*log) )
		return true;

	if( version == 20090920 )
	{// quest blocks separated by tabs, quest fields separated by commas, 6 fields total
		char* quests[MAX_QUEST_DB+1];
		int char_id, n, i;
		int count;

		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		// zero out the destination first
		memset(log, 0, sizeof(*log));

		// extract tab-separated columns from str
		count = sv_split(str, strlen(str), n + 1, '\t', quests, ARRAYLENGTH(quests), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));

		for( i = 0; i < count; ++i )
		{
			char* fields[6+1];
			if( sv_split(quests[i+1], strlen(quests[i+1]), 0, ',', fields, ARRAYLENGTH(fields), SV_NOESCAPE_NOTERMINATE) != 6 )
				return false;

			(*log)[i].quest_id = strtol(fields[1], NULL, 10);
			(*log)[i].state = strtol(fields[2], NULL, 10);
			(*log)[i].time = strtoul(fields[3], NULL, 10);
			(*log)[i].count[0] = strtol(fields[4], NULL, 10);
			(*log)[i].count[1] = strtol(fields[5], NULL, 10);
			(*log)[i].count[2] = strtol(fields[6], NULL, 10);
		}

		*key = char_id;
	}
	else
	if( version == 20090801 )
	{// quest blocks separated by tabs, quest fields separated by commas, 10 fields total
		char* quests[MAX_QUEST_DB+1];
		int char_id, n, i;
		int count;

		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		// zero out the destination first
		memset(log, 0, sizeof(*log));

		// extract tab-separated columns from str
		count = sv_split(str, strlen(str), n + 1, '\t', quests, ARRAYLENGTH(quests), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));

		for( i = 0; i < count; ++i )
		{
			char* fields[10+1];
			if( sv_split(quests[i+1], strlen(quests[i+1]), 0, ',', fields, ARRAYLENGTH(fields), SV_NOESCAPE_NOTERMINATE) != 10 )
				return false;

			(*log)[i].quest_id = strtol(fields[1], NULL, 10);
			(*log)[i].state = strtol(fields[2], NULL, 10);
			(*log)[i].time = strtoul(fields[3], NULL, 10);
			//(*log)[i].num_objectives = strtol(fields[4], NULL, 10);
			//(*log)[i].mob[0] = strtol(fields[5], NULL, 10);
			(*log)[i].count[0] = strtol(fields[6], NULL, 10);
			//(*log)[i].mob[1] = strtol(fields[7], NULL, 10);
			(*log)[i].count[1] = strtol(fields[8], NULL, 10);
			//(*log)[i].mob[2] = strtol(fields[9], NULL, 10);
			(*log)[i].count[2] = strtol(fields[10], NULL, 10);
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
static bool skill_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int char_id = key;
	questlog* log = (questlog*)data;
	int i;
	int count = 0;
	bool first = true;

	if( size != sizeof(*log) )
		return false;

	p += sprintf(p, "%d\t", char_id);

	for( i = 0; i < MAX_QUEST_DB; ++i )
	{
		const struct quest* qd = &(*log)[i];

		if( qd->quest_id == 0 )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, "\t");

		p += sprintf(p, "%d,%d,%u,%d,%d,%d", qd->quest_id, qd->state, qd->time, qd->count[0], qd->count[1], qd->count[2]);
	
		count++;
	}

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


/// @protected
static bool quest_db_txt_save(QuestDB* self, questlog* log, int char_id)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	int i;

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id > 0);
	if( i < MAX_QUEST_DB )
		return db->replace(db, char_id, log, sizeof(*log));
	else
		return db->remove(db, char_id);
}


/// @protected
static bool quest_db_txt_load(QuestDB* self, questlog* log, int char_id, int* const count)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	int i;

	if( db->load(db, char_id, log, sizeof(*log), NULL) )
	{
		// calculate and update 'count'
		*count = 0;
		for( i = 0; i < MAX_QUEST_DB; ++i )
			if( (*log)[i].quest_id > 0 )
				(*count)++;
	}
	else
	{
		memset(log, 0, sizeof(*log));
		*count = 0;
	}

	return true;
}


/// Returns an iterator over all quest entries.
/// @protected
static CSDBIterator* quest_db_txt_iterator(QuestDB* self)
{
	CSDB_TXT* db = ((QuestDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new QuestDB interface.
/// @protected
QuestDB* quest_db_txt(CharServerDB_TXT* owner)
{
	QuestDB_TXT* db = (QuestDB_TXT*)aCalloc(1, sizeof(QuestDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_quests, QUESTDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &skill_db_txt_fromstr;
	db->db->p.tostr   = &skill_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &quest_db_txt_init;
	db->vtable.p.destroy = &quest_db_txt_destroy;
	db->vtable.p.sync    = &quest_db_txt_sync;
	db->vtable.remove    = &quest_db_txt_remove;
	db->vtable.load      = &quest_db_txt_load;
	db->vtable.save      = &quest_db_txt_save;
	db->vtable.iterator  = &quest_db_txt_iterator;

	return &db->vtable;
}
