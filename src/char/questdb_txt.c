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

#define START_QUEST_NUM 1


/// internal structure
typedef struct QuestDB_TXT
{
	QuestDB vtable;       // public interface

	CharServerDB_TXT* owner;
	DBMap* quests;        // in-memory pet storage

	const char* quest_db; // quest data storage file

} QuestDB_TXT;



static bool mmo_quests_fromstr(questlog* log, char* str)
{
//	const char* p = str;
//	int i, n;

	memset(log, 0, sizeof(questlog));

	//TODO: come up with a good data format

	return true;
}


static bool mmo_quests_tostr(const questlog* log, char* str)
{
	char* p = str;
	int i;

	for( i = 0; i < MAX_QUEST; ++i )
	{
		const struct quest* qd = &(*log)[i];
		if( qd->quest_id == 0 )
			continue;
		
		//TODO: save qd->quest_id and qd->state
		//TODO: for each qd->objectives[0..qd->num_objectives] save name and count
	}

	return true;
}


static bool mmo_questdb_sync(QuestDB_TXT* db)
{
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE* fp;
	int lock;

	fp = lock_fopen(db->quest_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_questdb_sync: can't write [%s] !!! data is lost !!!\n", db->quest_db);
		return false;
	}

	iter = db->quests->iterator(db->quests);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		questlog* log = (questlog*) data;
		char line[8192];

		mmo_quests_tostr(log, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->quest_db, &lock);

	return true;
}


static bool quest_db_txt_init(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests;

	char line[8192];
	FILE *fp;

	// create quest database
	db->quests = idb_alloc(DB_OPT_RELEASE_DATA);
	quests = db->quests;

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

		questlog* log = (questlog*)aCalloc(1, sizeof(questlog));
		if( log == NULL )
		{
			ShowFatalError("quest_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		if( sscanf(line, "%d\t%n", &char_id, &n) != 1 || char_id <= 0 )
		{
			aFree(log);
			continue;
		}

		// load quests for this char
		if( !mmo_quests_fromstr(log, line + n) )
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

static void quest_db_txt_destroy(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;

	// write data
	mmo_questdb_sync(db);

	// delete quest database
	quests->destroy(quests, NULL);
	db->quests = NULL;

	// delete entire structure
	aFree(db);
}

static bool quest_db_txt_sync(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	return mmo_questdb_sync(db);
}

static bool quest_db_txt_remove(QuestDB* self, const int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;

	questlog* tmp = (questlog*)idb_remove(quests, char_id);
	if( tmp == NULL )
	{// error condition - entry not present
		ShowError("quest_db_txt_remove: no such entry for char with id %d\n", char_id);
		return false;
	}

	return true;
}

static bool quest_db_txt_add(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* log = (questlog*)idb_get(quests, char_id);
	if( log == NULL )
	{// entry not found
		return false;
	}

	ARR_FIND(0, MAX_QUEST, i, (*log)[i].quest_id == qd->quest_id);
	if( i < MAX_QUEST )
	{// quest already exists
		return false;
	}

	ARR_FIND(0, MAX_QUEST, i, (*log)[i].quest_id == 0);
	if( i == MAX_QUEST )
	{// no free room for this quest
		return false;
	}

	// write new questlog entry
	memcpy(&(*log)[i], qd, sizeof(struct quest));

	return true;
}

static bool quest_db_txt_del(QuestDB* self, const int char_id, const int quest_id)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	questlog* log = (questlog*)idb_get(quests, char_id);
	if( log == NULL )
	{// entry not found
		return false;
	}

	ARR_FIND(0, MAX_QUEST, i, (*log)[i].quest_id == quest_id);
	if( i == MAX_QUEST )
	{// quest not present in list
		return false;
	}

	// erase questlog entry
	memset(&(*log)[i], 0, sizeof((*log)[i]));

	return true;
}

static bool quest_db_txt_load(QuestDB* self, questlog* log, int char_id, int* const count)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	DBMap* quests = db->quests;
	int i;

	// retrieve data
	questlog* tmp = idb_get(quests, char_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(log, tmp, sizeof(questlog));

	// calculate and update 'count'
	*count = 0;
	for( i = 0; i < MAX_QUEST; ++i )
		if( (*log)[i].quest_id > 0 )
			(*count)++;

	return true;
}


/// Returns an iterator over all quest entries.
static CSDBIterator* quest_db_txt_iterator(QuestDB* self)
{
	QuestDB_TXT* db = (QuestDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->quests));
}


/// public constructor
QuestDB* quest_db_txt(CharServerDB_TXT* owner)
{
	QuestDB_TXT* db = (QuestDB_TXT*)aCalloc(1, sizeof(QuestDB_TXT));

	// set up the vtable
	db->vtable.init      = &quest_db_txt_init;
	db->vtable.destroy   = &quest_db_txt_destroy;
	db->vtable.sync      = &quest_db_txt_sync;
	db->vtable.add       = &quest_db_txt_add;
	db->vtable.del       = &quest_db_txt_del;
	db->vtable.load      = &quest_db_txt_load;
	db->vtable.iterator  = &quest_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->quests = NULL;

	// other settings
	db->quest_db = db->owner->file_quests;

	return &db->vtable;
}
