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
};



static bool rankid2classes(char* buffer, size_t buflen, int rank_id)
{
	switch( rank_id )
	{
	case RANK_BLACKSMITH:
		safesnprintf(buffer, buflen, "(`class`=%d OR `class`=%d OR `class`=%d)", JOB_BLACKSMITH, JOB_WHITESMITH, JOB_BABY_BLACKSMITH);
		break;
	case RANK_ALCHEMIST:
		safesnprintf(buffer, buflen, "(`class`=%d OR `class`=%d OR `class`=%d)", JOB_ALCHEMIST, JOB_CREATOR, JOB_BABY_ALCHEMIST);
		break;
	case RANK_TAEKWON:
		safesnprintf(buffer, buflen, "`class`=%d", JOB_TAEKWON);
		break;
	default:
		return false;
	}
	return true;
}



/// Gets the top rankers in rank rank_id.
static int rank_db_sql_get_top_rankers(RankDB* self, int rank_id, struct fame_list* list, int count)
{
	RankDB_SQL* db = (RankDB_SQL*)self;
	char* table_chars = db->owner->table_chars;
	SqlStmt* stmt;
	struct fame_list entry;
	int i;
	char restrict_class[256];

	memset(list, 0, count*sizeof(list[0]));
	if( !rankid2classes(restrict_class, sizeof(restrict_class), rank_id) )
	{
		ShowError("rank_db_sql_get_top_rankers: unsupported rank_id %d.\n", rank_id);
		return 0;
	}

	stmt = SqlStmt_Malloc(db->owner->sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt,
		"SELECT `char_id`,`fame`,`name` FROM `%s`"
		" WHERE `fame`>0 AND %s"
		" ORDER BY `fame` DESC LIMIT 0,%d",
		table_chars, restrict_class, count)
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
	char* table_chars = db->owner->table_chars;
	int points = 0;
	char restrict_class[256];

	if( !rankid2classes(restrict_class, sizeof(restrict_class), rank_id) )
	{
		ShowError("rank_db_sql_get_points: unsupported rank_id %d.\n", rank_id);
		return 0;
	}

	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `fame` FROM `%s`"
		" WHERE `rank_id`=%d AND `char_id`=%d",
		table_chars, points, rank_id, char_id) )
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
	char* table_chars = db->owner->table_chars;
	char restrict_class[256];

	if( !rankid2classes(restrict_class, sizeof(restrict_class), rank_id) )
	{
		ShowError("rank_db_sql_set_points: unsupported rank_id %d.\n", rank_id);
		return;
	}

	if( SQL_ERROR == Sql_Query(sql_handle,
		"UPDATE `%s` SET `fame`=%d"
		" WHERE `char_id`=%d AND %s",
		table_chars, points, char_id, restrict_class) )
	{
		Sql_ShowDebug(sql_handle);
	}
}



/// Constructs a new RankDB interface.
/// @protected
RankDB* rank_db_sql(CharServerDB_SQL* owner)
{
	RankDB_SQL* db;

	CREATE(db, RankDB_SQL, 1);
	db->vtable.get_top_rankers = rank_db_sql_get_top_rankers;
	db->vtable.get_points      = rank_db_sql_get_points;
	db->vtable.set_points      = rank_db_sql_set_points;

	db->owner = owner;
	return &db->vtable;
}



/// Initializes this RankDB interface.
/// @protected
bool rank_db_sql_init(RankDB* self)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	return true;
}



/// Destroys this RankDB interface.
/// @protected
void rank_db_sql_destroy(RankDB* self)
{
	RankDB_SQL* db = (RankDB_SQL*)self;

	db->owner = NULL;
	aFree(db);
}
