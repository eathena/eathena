// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include <string.h>
#include <stdlib.h>


/// Internal structure.
/// @private
typedef struct RankDB_SQL
{
	// public interface
	RankDB vtable;

	// state
	CharServerDB_SQL* owner;

	// settings
	const char* table_ranks;

} RankDB_SQL;


/// Gets the top rankers in rank rank_id.
/// @protected
static int rank_db_sql_get_top_rankers(RankDB* self, enum rank_type rank_id, struct fame_list* list, int count)
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
/// @protected
static int rank_db_sql_get_points(RankDB* self, enum rank_type rank_id, int char_id)
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
/// @protected
static void rank_db_sql_set_points(RankDB* self, enum rank_type rank_id, int char_id, int points)
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
static bool rank_db_sql_sync(RankDB* self, bool force)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	return true;
}


/// Internal structure.
/// @private
typedef struct RankDBIterator_SQL
{
	// public interface
	CSDBIterator vtable;

	// state
	RankDB_SQL* db;
	int* ids_arr;
	int ids_num;
	int pos;

} RankDBIterator_SQL;


/// Destroys this iterator, releasing all allocated memory (including itself).
/// @protected
static void rank_db_sql_iter_destroy(CSDBIterator* self)
{
	RankDBIterator_SQL* iter = (RankDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}


/// Fetches the next ranking.
/// @protected
static bool rank_db_sql_iter_next(CSDBIterator* self, int* key)
{
	RankDBIterator_SQL* iter = (RankDBIterator_SQL*)self;
	RankDB_SQL* db = (RankDB_SQL*)iter->db;
	Sql* sql_handle = db->owner->sql_handle;

	if( iter->pos+1 >= iter->ids_num )
		return false;

	++iter->pos;
	if( key )
		*key = iter->ids_arr[iter->pos];
	return true;
}


/// Returns an iterator over all rankings of the specified type.
///
/// @param self Database
/// @param rank_id Rank list id
/// @return Iterator
/// @protected
static CSDBIterator* rank_db_sql_iterator(RankDB* self, enum rank_type rank_id)
{
	RankDB_SQL* db = (RankDB_SQL*)self;
	Sql* sql_handle = db->owner->sql_handle;
	RankDBIterator_SQL* iter = (RankDBIterator_SQL*)aCalloc(1, sizeof(RankDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &rank_db_sql_iter_destroy;
	iter->vtable.next    = &rank_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `rank_id`=%d ORDER BY `char_id` ASC", db->table_ranks, rank_id) )
		Sql_ShowDebug(sql_handle);
	else if( Sql_NumRows(sql_handle) > 0 )
	{
		int i;

		iter->ids_num = (int)Sql_NumRows(sql_handle);
		CREATE(iter->ids_arr, int, iter->ids_num);

		i = 0;
		while( i < iter->ids_num )
		{
			char* data;
			int res = Sql_NextRow(sql_handle);
			if( res == SQL_SUCCESS )
				res = Sql_GetData(sql_handle, 0, &data, NULL);
			if( res == SQL_ERROR )
				Sql_ShowDebug(sql_handle);
			if( res != SQL_SUCCESS )
				break;

			if( data == NULL )
				continue;

			iter->ids_arr[i] = atoi(data);
			++i;
		}
		iter->ids_num = i;
	}
	Sql_FreeResult(sql_handle);

	return &iter->vtable;
}


/// Constructs a new RankDB interface.
/// @protected
RankDB* rank_db_sql(CharServerDB_SQL* owner)
{
	RankDB_SQL* db;

	CREATE(db, RankDB_SQL, 1);
	db->vtable.p.init          = rank_db_sql_init;
	db->vtable.p.destroy       = rank_db_sql_destroy;
	db->vtable.p.sync          = rank_db_sql_sync;
	db->vtable.get_top_rankers = rank_db_sql_get_top_rankers;
	db->vtable.get_points      = rank_db_sql_get_points;
	db->vtable.set_points      = rank_db_sql_set_points;
	db->vtable.iterator        = rank_db_sql_iterator;

	db->owner = owner;
	db->table_ranks = owner->table_ranks;
	return &db->vtable;
}
