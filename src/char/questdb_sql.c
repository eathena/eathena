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


/// internal structure
typedef struct QuestDB_SQL
{
	QuestDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* quests;       // SQL quest storage

	// other settings
	const char* quest_db;
	const char* objective_db;

} QuestDB_SQL;



static bool mmo_quests_fromsql(QuestDB_SQL* db, questlog* log, int char_id, int* const count)
{
	Sql* sql_handle = db->quests;
	struct quest tmp_quest;
	struct quest_objective tmp_objective;
	SqlStmt* stmt;
	int i;
	bool result = false;

	memset(&tmp_quest, 0, sizeof(tmp_quest));
	memset(&tmp_objective, 0, sizeof(tmp_objective));
	*count = 0;

	stmt = SqlStmt_Malloc(sql_handle);

	//TODO: transaction

	do
	{

	if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT `quest_id`, `state` FROM `%s` WHERE `char_id`=%d LIMIT %d", db->quest_db, char_id, MAX_QUEST)
	||	SQL_SUCCESS != SqlStmt_Execute(stmt)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &tmp_quest.quest_id, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &tmp_quest.state, 0, NULL, NULL) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	for( i = 0; i < MAX_QUEST && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		(*log)[i].quest_id = tmp_quest.quest_id;
		(*log)[i].state = tmp_quest.state;
	}

	*count = i;

	for( i = 0; i < *count; ++i )
	{
		int j, num;

		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT `num`, `name`, `count` FROM `%s` WHERE `char_id`=%d AND `quest_id`=%d LIMIT %d", db->objective_db, char_id, (*log)[i].quest_id, MAX_QUEST_OBJECTIVES)
		||	SQL_SUCCESS != SqlStmt_Execute(stmt)
		||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &num, 0, NULL, NULL)
		||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &tmp_objective.name, NAME_LENGTH, NULL, NULL)
		||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 2, SQLDT_INT,    &tmp_objective.count, 0, NULL, NULL) )
		{
			SqlStmt_ShowDebug(stmt);
			break;
		}

		for( j = 0; j < MAX_QUEST && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++j )
		{
			safestrncpy((*log)[i].objectives[num].name, tmp_objective.name, NAME_LENGTH);
			(*log)[i].objectives[num].count = tmp_objective.count;
		}

		(*log)[i].num_objectives = j;
	}

	SqlStmt_Free(stmt);

	// if we got here, everything succeeded
	result = true;

	}
	while(0);

	return result;
}


static bool quest_db_sql_init(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	db->quests = db->owner->sql_handle;
	return true;
}

static void quest_db_sql_destroy(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	db->quests = NULL;
	aFree(db);
}

static bool quest_db_sql_sync(QuestDB* self)
{
	return true;
}

static bool quest_db_sql_remove(QuestDB* self, const int char_id)
{

}

static bool quest_db_sql_add(QuestDB* self, const struct quest* qd, const int char_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;
	bool result = false;
	int i;

	//TODO: transaction

	do
	{

	if( SQL_SUCCESS != Sql_Query(sql_handle,
	    "INSERT INTO `%s`(`quest_id`, `char_id`, `state`) VALUES ('%d', '%d', '%d')",
	    db->quest_db, qd->quest_id, char_id, qd->state) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	for( i = 0; i < qd->num_objectives; i++ )
	{
		if( SQL_SUCCESS != Sql_Query(sql_handle,
		    "INSERT INTO `%s`(`quest_id`, `char_id`, `num`, `name`, `count`) VALUES ('%d', '%d', '%d', '%s', '%d')",
			db->objective_db, qd->quest_id, char_id, i, qd->objectives[i].name, qd->objectives[i].count) )
		{
			Sql_ShowDebug(sql_handle);
			break;
		}
	}

	if( i < qd->num_objectives )
		break;

	// if we got here, everything succeeded
	result = true;

	}
	while(0);

	return result;
}

static bool quest_db_sql_del(QuestDB* self, const int char_id, const int quest_id)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	Sql* sql_handle = db->quests;
	bool result = false;

	//TODO: transaction

	do
	{

	if( SQL_SUCCESS != Sql_Query(sql_handle,
		"DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'",
		db->quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( SQL_SUCCESS != Sql_Query(sql_handle,
	    "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'",
		db->objective_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	// if we got here, everything succeeded
	result = true;

	}
	while(0);

	return result;
}

static bool quest_db_sql_load(QuestDB* self, questlog* log, int char_id, int* const count)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return mmo_quests_fromsql(db, log, char_id, count);
}


/// Returns an iterator over all quest entries.
static CSDBIterator* quest_db_sql_iterator(QuestDB* self)
{
	QuestDB_SQL* db = (QuestDB_SQL*)self;
	return csdb_sql_iterator(db->quests, db->quest_db, "char_id");
}


/// public constructor
QuestDB* quest_db_sql(CharServerDB_SQL* owner)
{
	QuestDB_SQL* db = (QuestDB_SQL*)aCalloc(1, sizeof(QuestDB_SQL));

	// set up the vtable
	db->vtable.init      = &quest_db_sql_init;
	db->vtable.destroy   = &quest_db_sql_destroy;
	db->vtable.sync      = &quest_db_sql_sync;
	db->vtable.add       = &quest_db_sql_add;
	db->vtable.del       = &quest_db_sql_del;
	db->vtable.load      = &quest_db_sql_load;
	db->vtable.iterator  = &quest_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->quests = NULL;

	// other settings
	db->quest_db = db->owner->table_quests;
	db->objective_db = db->owner->table_quest_objectives;

	return &db->vtable;
}
