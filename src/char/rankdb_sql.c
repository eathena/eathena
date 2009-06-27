// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"

#include <string.h>
#include <stdlib.h>



typedef struct RankDB_SQL RankDB_SQL;



/// private
struct RankDB_SQL
{
	RankDB vtable;

	CharServerDB_SQL* owner;
	const char* table_ranks;
};



/// Gets the top rankers in rank rank_id.
static int rank_db_sql_get_top_rankers(RankDB* self, int rank_id, struct fame_list* list, int count)
{
	RankDB_SQL* db = (RankDB_SQL*)self;
	const char* table_ranks = db->table_ranks;
	const char* table_chars = db->owner->table_chars;
	SqlStmt* stmt;
	struct fame_list entry;
	int i;

	if( list == NULL || count <= 0 )
		return 0;// nothing to do
	memset(list, 0, count*sizeof(list[0]));

	stmt = SqlStmt_Malloc(db->owner->sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt,
		"SELECT r.`rank_id`, r.`points`, IFNULL(c.`name`,'')"
		" FROM `%s` AS r LEFT JOIN `%s` AS c USING(`char_id`)"
		" WHERE r.`rank_id`=%d AND r.`points`>0"
		" ORDER BY r.`points` DESC LIMIT 0,%d",
		table_ranks, table_chars, rank_id, count)
	|| SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 0;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &entry.id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &entry.fame, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_CHAR, &entry.name, sizeof(entry.name), NULL, NULL);

	for( i = 0; i < count; ++i )
	{
		int res = SqlStmt_NextRow(stmt);
		if( res != SQL_SUCCESS )
		{// no more entries or error
			if( res == SQL_ERROR )
				SqlStmt_ShowDebug(stmt);
			break;
		}
		memcpy(&list[i], &entry, sizeof(entry));
	}
	SqlStmt_Free(stmt);
	return i;
}



/// Returns the number of points character char_id has in rank rank_id.
/// Returns 0 if not found.
static int rank_db_sql_get_points(RankDB* self, int rank_id, int char_id)
{
	RankDB_SQL* db = (RankDB_SQL*)self;
	Sql* sql_handle = db->owner->sql_handle;
	const char* table_ranks = db->table_ranks;
	int points = 0;

	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `points` FROM `%s`"
		" WHERE `rank_id`=%d AND `char_id`=%d",
		table_ranks, rank_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}
	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;
		Sql_GetData(sql_handle, 0, &data, NULL); points = atoi(data);
	}
	Sql_FreeResult(sql_handle);
	return points;
}



/// Sets the number of points character char_id has in rank rank_id.
static void rank_db_sql_set_points(RankDB* self, int rank_id, int char_id, int points)
{
	RankDB_SQL* db = (RankDB_SQL*)self;
	Sql* sql_handle = db->owner->sql_handle;
	const char* table_ranks = db->table_ranks;

	if( SQL_ERROR == Sql_Query(sql_handle,
		"REPLACE INTO `%s`(`rank_id`, `char_id`, `points`)"
		" VALUES(%d, %d, %d)",
		table_ranks, rank_id, char_id, points) )
	{
		Sql_ShowDebug(sql_handle);
	}
}



/// Initializes this RankDB interface.
/// @protected
static bool rank_db_sql_init(RankDB* self)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	return true;
}



/// Destroys this RankDB interface.
/// @protected
static void rank_db_sql_destroy(RankDB* self)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	db->owner = NULL;
	aFree(db);
}



/// Saves any pending data.
/// @protected
static bool rank_db_sql_save(RankDB* self)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	return true;
}



/// Constructs a new RankDB interface.
/// @protected
RankDB* rank_db_sql(CharServerDB_SQL* owner)
{
	RankDB_SQL* db;

	CREATE(db, RankDB_SQL, 1);
	db->vtable.init            = rank_db_sql_init;
	db->vtable.destroy         = rank_db_sql_destroy;
	db->vtable.sync            = rank_db_sql_save;
	db->vtable.get_top_rankers = rank_db_sql_get_top_rankers;
	db->vtable.get_points      = rank_db_sql_get_points;
	db->vtable.set_points      = rank_db_sql_set_points;
//	db->vtable.iterator        = rank_db_sql_iterator;

	db->owner = owner;
	db->table_ranks = owner->table_ranks;
	return &db->vtable;
}
