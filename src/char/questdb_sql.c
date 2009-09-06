// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
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
static bool mmo_quests_fromsql(QuestDB_SQL* db, questlog* log, int char_id, int* const count)
{
	Sql* sql_handle = db->quests;
	struct quest tmp_quest;
	SqlStmt* stmt;
	int i;
	bool result = false;

	memset(&tmp_quest, 0, sizeof(tmp_quest));
	*count = 0;

	stmt = SqlStmt_Malloc(sql_handle);

	do
	{

	if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT `quest_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3` FROM `%s` WHERE `char_id`=%d LIMIT %d", db->quest_db, char_id, MAX_QUEST_DB)
	||	SQL_SUCCESS != SqlStmt_Execute(stmt)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &tmp_quest.quest_id, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &tmp_quest.state, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 2, SQLDT_UINT,&tmp_quest.time, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 3, SQLDT_INT, &tmp_quest.mob[0], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 4, SQLDT_INT, &tmp_quest.count[0], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &tmp_quest.mob[1], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 6, SQLDT_INT, &tmp_quest.count[1], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 7, SQLDT_INT, &tmp_quest.mob[2], 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 8, SQLDT_INT, &tmp_quest.count[2], 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		break;
	}

	for( i = 0; i < MAX_QUEST_DB && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		(*log)[i].quest_id = tmp_quest.quest_id;
		(*log)[i].state = tmp_quest.state;
		(*log)[i].time = tmp_quest.time;
		(*log)[i].mob[0] = tmp_quest.mob[0];
		(*log)[i].count[0] = tmp_quest.count[0];
		(*log)[i].mob[1] = tmp_quest.mob[1];
		(*log)[i].count[1] = tmp_quest.count[1];
		(*log)[i].mob[2] = tmp_quest.mob[2];
		(*log)[i].count[2] = tmp_quest.count[2];
		(*log)[i].num_objectives = (tmp_quest.mob[0]) ? 0 : (tmp_quest.mob[1]) ? 1 : (tmp_quest.mob[2]) ? 2 : 3;
	}

	*count = i;

	// if we got here, everything succeeded
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);

	return result;
}


/// @private
static bool mmo_quests_tosql(QuestDB_SQL* db, questlog* log, int char_id)
{
	Sql* sql_handle = db->owner->sql_handle;
	StringBuf buf;
	int i, j;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	StringBuf_Init(&buf);

	// try
	do
	{

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->quest_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`quest_id`, `char_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3`) VALUES ", db->quest_db);

	j = 0; // counter
	for( i = 0; i < MAX_QUEST_DB; ++i )
	{
		const struct quest* qd = &(*log)[i];

		if( qd->quest_id == 0 )
			continue;

		if( j != 0 )
			StringBuf_AppendStr(&buf, ",");

		StringBuf_Printf(&buf, "('%d','%d','%d','%d','%d','%d','%d','%d','%d','%d')", qd->quest_id, char_id, qd->state, qd->time, qd->mob[0], qd->count[0], qd->mob[1], qd->count[1], qd->mob[2], qd->count[2]);

		j++;
	}

	if( j == 0 )
	{// nothing to save
		result = true;
		break;
	}

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	// success
	result = true;

	}
	while(0);
	// finally

	StringBuf_Destroy(&buf);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") )
	{
		Sql_ShowDebug(sql_handle);
		result = false;
	}

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

	if( SQL_SUCCESS != Sql_Query(sql_handle,
		"DELETE FROM `%s` WHERE `char_id` = '%d'",
		db->quest_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool quest_db_sql_add(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;

	if( SQL_SUCCESS != Sql_Query(sql_handle,
	    "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3`) "
		"VALUES ('%d', '%d', '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d')",
	    db->quest_db, qd->quest_id, char_id, qd->state, qd->time, qd->mob[0], qd->count[0], qd->mob[1], qd->count[1], qd->mob[2], qd->count[2]) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool quest_db_sql_update(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;

	//TODO: support for writing to all columns
	if( SQL_SUCCESS != Sql_Query(sql_handle,
	    "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' WHERE `quest_id` = '%d' AND `char_id` = '%d'",
	    db->quest_db, qd->state, qd->count[0], qd->count[1], qd->count[2], qd->quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool quest_db_sql_del(QuestDB* self, const int char_id, const int quest_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;


	if( SQL_SUCCESS != Sql_Query(sql_handle,
		"DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'",
		db->quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool quest_db_sql_load(QuestDB* self, questlog* log, int char_id, int* const count)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return mmo_quests_fromsql(db, log, char_id, count);
}


/// @protected
static bool quest_db_sql_save(QuestDB* self, questlog* log, int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return mmo_quests_tosql(db, log, char_id);
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
	db->vtable.p.init      = &quest_db_sql_init;
	db->vtable.p.destroy   = &quest_db_sql_destroy;
	db->vtable.p.sync      = &quest_db_sql_sync;
	db->vtable.add       = &quest_db_sql_add;
	db->vtable.del       = &quest_db_sql_del;
	db->vtable.update    = &quest_db_sql_update;
	db->vtable.load      = &quest_db_sql_load;
	db->vtable.save      = &quest_db_sql_save;
	db->vtable.iterator  = &quest_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->quests = NULL;

	// other settings
	db->quest_db = db->owner->table_quests;

	return &db->vtable;
}
