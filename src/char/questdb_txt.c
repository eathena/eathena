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
#include "questdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define QUEST_TXT_DB_VERSION 20090801


/// Internal structure.
/// @private
typedef struct QuestDB_TXT
{
	// public interface
	QuestDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* quests;
	bool dirty;

	// settings
	const char* quest_db;

} QuestDB_TXT;


/// @private
static void* create_questlog(DBKey key, va_list args)
{
	return (questlog*)aCalloc(1, sizeof(questlog));
}


/// @private
static bool mmo_quests_fromstr(questlog* log, char* str, unsigned int version)
{
	// zero out the destination first
	memset(log, 0, sizeof(*log));

	if( version == 20090801 )
	{// quest blocks separated by tabs, quest fields separated by commas, 10 fields total
		char* quests[MAX_QUEST_DB+1];
		int count;
		int i;

		// extract tab-separated columns from str
		count = sv_split(str, strlen(str), 0, '\t', quests, ARRAYLENGTH(quests), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));

		for( i = 0; i < count; ++i )
		{
			char* fields[10+1];
			if( sv_split(quests[i+1], strlen(quests[i+1]), 0, ',', fields, ARRAYLENGTH(fields), SV_NOESCAPE_NOTERMINATE) != 10 )
				return false;

			(*log)[i].quest_id = strtol(fields[1], NULL, 10);
			(*log)[i].state = strtol(fields[2], NULL, 10);
			(*log)[i].time = strtoul(fields[3], NULL, 10);
			(*log)[i].num_objectives = strtol(fields[4], NULL, 10);
			(*log)[i].mob[0] = strtol(fields[5], NULL, 10);
			(*log)[i].count[0] = strtol(fields[6], NULL, 10);
			(*log)[i].mob[1] = strtol(fields[7], NULL, 10);
			(*log)[i].count[1] = strtol(fields[8], NULL, 10);
			(*log)[i].mob[2] = strtol(fields[9], NULL, 10);
			(*log)[i].count[2] = strtol(fields[10], NULL, 10);
		}
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// @private
static bool mmo_quests_tostr(const questlog* log, char* str)
{
	char* p = str;
	int i;

	for( i = 0; i < MAX_QUEST_DB; ++i )
	{
		const struct quest* qd = &(*log)[i];
		if( qd->quest_id == 0 )
			continue;

		if( i != 0 )
			p += sprintf(p, "\t");

		p += sprintf(p, "%d,%d,%u,%d,", qd->quest_id, qd->state, qd->time, qd->num_objectives);
		p += sprintf(p, "%d,%d,%d,%d,%d,%d", qd->mob[0], qd->count[0], qd->mob[1], qd->count[1], qd->mob[2], qd->count[2]);

	}

	return true;
}


/// @protected
static bool quest_db_txt_init(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create quest database
	if( db->quests == NULL )
		db->quests = idb_alloc(DB_OPT_RELEASE_DATA);
	quests = db->quests;
	db_clear(quests);

	// open data file
	fp = fopen(db->quest_db, "r");
	if( fp == NULL )
	{
		ShowError("Quest file not found: %s.\n", db->quest_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id;
		int n;
		unsigned int v;
		questlog* log;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		log = (questlog*)aCalloc(1, sizeof(questlog));
		if( log == NULL )
		{
			ShowFatalError("quest_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n\t", &char_id, &n) != 1 || line[n] != '\t' )
		{
			aFree(log);
			continue;
		}

		// load quests for this char
		if( !mmo_quests_fromstr(log, line + n + 1, version) )
		{
			ShowError("quest_db_txt_init: skipping invalid data: %s", line);
			continue;
		}

		// record entry in db
		idb_put(quests, char_id, log);
	}

	// close data file
	fclose(fp);

	return true;
}


/// @protected
static void quest_db_txt_destroy(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;

	// delete quest database
	if( quests != NULL )
	{
		db_destroy(quests);
		db->quests = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool quest_db_txt_sync(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE* fp;
	int lock;

	fp = lock_fopen(db->quest_db, &lock);
	if( fp == NULL )
	{
		ShowError("quest_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->quest_db);
		return false;
	}

	fprintf(fp, "%d\n", QUEST_TXT_DB_VERSION); // savefile version

	iter = db->quests->iterator(db->quests);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		questlog* log = (questlog*) data;
		char line[8192]; //FIXME: not nearly enough space for MAX_QUEST_DB entries

		mmo_quests_tostr(log, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->quest_db, &lock);

	return true;
}


/// @protected
static bool quest_db_txt_remove(QuestDB* self, const int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;

	idb_remove(quests, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool quest_db_txt_add(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* log = (questlog*)idb_ensure(quests, char_id, create_questlog);
	if( log == NULL )
	{// allocation failure <_<
		return false;
	}

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id == qd->quest_id);
	if( i < MAX_QUEST_DB )
	{// quest already exists
		return false;
	}

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id == 0);
	if( i == MAX_QUEST_DB )
	{// no free room for this quest
		return false;
	}

	// write new questlog entry
	memcpy(&(*log)[i], qd, sizeof(struct quest));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool quest_db_txt_update(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* log = (questlog*)idb_get(quests, char_id);
	if( log == NULL )
	{// empty quest log
		return false;
	}

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id == qd->quest_id);
	if( i == MAX_QUEST_DB )
	{// quest not found
		return false;
	}

	// update questlog entry
	memcpy(&(*log)[i], qd, sizeof(struct quest));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool quest_db_txt_del(QuestDB* self, const int char_id, const int quest_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* log = (questlog*)idb_get(quests, char_id);
	if( log == NULL )
	{// no quests, nothing to delete
		return true;
	}

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id == quest_id);
	if( i == MAX_QUEST_DB )
	{// quest not present in list, nothing to delete
		return true;
	}

	// erase questlog entry
	memset(&(*log)[i], 0, sizeof((*log)[i]));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool quest_db_txt_load(QuestDB* self, questlog* log, int char_id, int* const count)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* tmp = idb_get(quests, char_id);

	if( tmp != NULL )
	{
		// store it
		memcpy(log, tmp, sizeof(questlog));

		// calculate and update 'count'
		*count = 0;
		for( i = 0; i < MAX_QUEST_DB; ++i )
			if( (*log)[i].quest_id > 0 )
				(*count)++;
	}
	else
	{
		memset(log, 0x00, sizeof(*log));
		*count = 0;
	}

	return true;
}


/// @protected
static bool quest_db_txt_save(QuestDB* self, questlog* log, int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	ARR_FIND(0, MAX_QUEST_DB, i, (*log)[i].quest_id > 0);
	if( i < MAX_QUEST_DB )
	{
		questlog* tmp = (questlog*)idb_ensure(quests, char_id, create_questlog);
		memcpy(tmp, log, sizeof(*log));
	}
	else
	{
		idb_remove(quests, char_id);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// Returns an iterator over all quest entries.
/// @protected
static CSDBIterator* quest_db_txt_iterator(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->quests));
}


/// Constructs a new QuestDB interface.
/// @protected
QuestDB* quest_db_txt(CharServerDB_TXT* owner)
{
	QuestDB_TXT* db = (QuestDB_TXT*)aCalloc(1, sizeof(QuestDB_TXT));

	// set up the vtable
	db->vtable.init      = &quest_db_txt_init;
	db->vtable.destroy   = &quest_db_txt_destroy;
	db->vtable.sync      = &quest_db_txt_sync;
	db->vtable.add       = &quest_db_txt_add;
	db->vtable.del       = &quest_db_txt_del;
	db->vtable.update    = &quest_db_txt_update;
	db->vtable.load      = &quest_db_txt_load;
	db->vtable.save      = &quest_db_txt_save;
	db->vtable.iterator  = &quest_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->quests = NULL;
	db->dirty = false;

	// other settings
	db->quest_db = db->owner->file_quests;

	return &db->vtable;
}
