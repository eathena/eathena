// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h" // ARR_FIND()
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "questdb.h"
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct QuestDB_SQL
{
	// public interface
	QuestDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* quests;

	// settings
	const char* quest_db;

} QuestDB_SQL;


/// @private
static bool quest_db_sql_add(QuestDB_SQL* db, const struct quest* qd, const int char_id)
{
	if( SQL_SUCCESS != Sql_Query(db->quests,
	    "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `count1`, `count2`, `count3`) "
		"VALUES ('%d', '%d', '%d','%d', '%d', '%d', '%d')",
	    db->quest_db, qd->quest_id, char_id, qd->state, qd->time, qd->count[0], qd->count[1], qd->count[2]) )
	{
		Sql_ShowDebug(db->quests);
		return false;
	}

	return true;
}


/// @private
static bool quest_db_sql_update(QuestDB_SQL* db, const struct quest* qd, const int char_id)
{
	//TODO: support for writing to all columns
	if( SQL_SUCCESS != Sql_Query(db->quests,
	    "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' WHERE `quest_id` = '%d' AND `char_id` = '%d'",
	    db->quest_db, qd->state, qd->count[0], qd->count[1], qd->count[2], qd->quest_id, char_id) )
	{
		Sql_ShowDebug(db->quests);
		return false;
	}

	return true;
}


/// @private
static bool quest_db_sql_del(QuestDB_SQL* db, const int char_id, const int quest_id)
{
	if( SQL_SUCCESS != Sql_Query(db->quests,
		"DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'",
		db->quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(db->quests);
		return false;
	}

	return true;
}


/// @param size Capacity of the array, in fields.
/// @private
static bool mmo_quests_fromsql(QuestDB_SQL* db, struct quest* log, size_t size, int char_id)
{
	Sql* sql_handle = db->quests;
	struct quest tmp_quest;
	SqlStmt* stmt;
	size_t i;
	bool result = false;

	// zero out the destination first
	memset(log, 0, size * sizeof(struct quest));

	memset(&tmp_quest, 0, sizeof(tmp_quest));

	stmt = SqlStmt_Malloc(sql_handle);

	do
	{

	if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT `quest_id`, `state`, `time`, `count1`, `count2`, `count3` FROM `%s` WHERE `char_id`=%d LIMIT %d", db->quest_db, char_id, MAX_QUEST_DB)
	||	SQL_SUCCESS != SqlStmt_Execute(stmt)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &tmp_quest.quest_id, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &tmp_quest.state, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 2, SQLDT_UINT,&tmp_quest.time, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 3, SQLDT_INT, &tmp_quest.count[0], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 4, SQLDT_INT, &tmp_quest.count[1], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &tmp_quest.count[2], 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		break;
	}

	for( i = 0; i < size && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&log[i], &tmp_quest, sizeof(tmp_quest));

	// if we got here, everything succeeded
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);

	return result;
}


/// @param size Number of fields in the array to process.
/// @private
static bool mmo_quests_tosql(QuestDB_SQL* db, const struct quest* log, size_t size, int char_id)
{
	struct quest qd1[MAX_QUEST_DB]; // new quest log, to be saved
	struct quest qd2[MAX_QUEST_DB]; // previous quest log
	int num2;
	int i, j;
	bool result = true;

	size = min(size, MAX_QUEST_DB); // temporary safety check

	memset(qd1, 0, sizeof(qd1));
	memset(qd2, 0, sizeof(qd2));
	memcpy(qd1, log, size * sizeof(struct quest));
	mmo_quests_fromsql(db, qd2, ARRAYLENGTH(qd2), char_id);

	// determine number of entries in array
	ARR_FIND(0, MAX_QUEST_DB, num2, qd2[num2].quest_id == 0);

	for( i = 0; i < ARRAYLENGTH(qd1); i++ )
	{
		if( qd1[i].quest_id == 0 )
			continue;

		ARR_FIND( 0, num2, j, qd1[i].quest_id == qd2[j].quest_id );
		if( j < num2 ) // Update existed quests
		{	// Only states and counts are changable.
			if( qd1[i].state != qd2[j].state || qd1[i].count[0] != qd2[j].count[0] || qd1[i].count[1] != qd2[j].count[1] || qd1[i].count[2] != qd2[j].count[2] )
				result &= quest_db_sql_update(db, &qd1[i], char_id);

			if( j < (--num2) )
			{
				memmove(&qd2[j],&qd2[j+1],sizeof(struct quest)*(num2-j));
				memset(&qd2[num2], 0, sizeof(struct quest));
			}
		}
		else // Add new quests
			result &= quest_db_sql_add(db, &qd1[i], char_id);
	}

	for( i = 0; i < num2; i++ ) // Quests not in qd1 but in qd2 are to be erased.
		result &= quest_db_sql_del(db, char_id, qd2[i].quest_id);

	return result;
}


/// @protected
static bool quest_db_sql_init(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	db->quests = db->owner->sql_handle;
	return true;
}


/// @protected
static void quest_db_sql_destroy(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	db->quests = NULL;
	aFree(db);
}


/// @protected
static bool quest_db_sql_sync(QuestDB* self, bool force)
{
	return true;
}


/// @protected
static bool quest_db_sql_remove(QuestDB* self, const int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'",	db->quest_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool quest_db_sql_save(QuestDB* self, const struct quest* log, size_t size, int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return mmo_quests_tosql(db, log, size, char_id);
}


/// @protected
static bool quest_db_sql_load(QuestDB* self, struct quest* log, size_t size, int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return mmo_quests_fromsql(db, log, size, char_id);
}


/// @protected
static int quest_db_sql_count(QuestDB* self, int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	char* data;
	int result;

	if( SQL_SUCCESS != Sql_Query(db->quests, "SELECT COUNT(*) FROM `%s` WHERE `char_id` = %d", db->quest_db, char_id)
	||  SQL_SUCCESS != Sql_NextRow(db->quests)
	) {
		Sql_ShowDebug(db->quests);
		Sql_FreeResult(db->quests);
		return 0;
	}

	Sql_GetData(db->quests, 0, &data, NULL);
	result = atoi(data);

	return result;
}


/// Returns an iterator over all quest entries.
/// @protected
static CSDBIterator* quest_db_sql_iterator(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return csdb_sql_iterator(db->quests, db->quest_db, "char_id");
}


/// Constructs a new QuestDB interface.
/// @protected
QuestDB* quest_db_sql(CharServerDB_SQL* owner)
{
	QuestDB_SQL* db = (QuestDB_SQL*)aCalloc(1, sizeof(QuestDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &quest_db_sql_init;
	db->vtable.p.destroy = &quest_db_sql_destroy;
	db->vtable.p.sync    = &quest_db_sql_sync;
	db->vtable.remove    = &quest_db_sql_remove;
	db->vtable.save      = &quest_db_sql_save;
	db->vtable.load      = &quest_db_sql_load;
	db->vtable.count     = &quest_db_sql_count;
	db->vtable.iterator  = &quest_db_sql_iterator;

	// initialize state
	db->owner = owner;
	db->quests = NULL;

	// other settings
	db->quest_db = db->owner->table_quests;

	return &db->vtable;
}
