// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "memodb.h"
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct MemoDB_SQL
{
	// public interface
	MemoDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* memos;

	// settings
	const char* memo_db;

} MemoDB_SQL;


/// @private
static bool mmo_memolist_fromsql(MemoDB_SQL* db, memolist* list, int char_id)
{
	Sql* sql_handle = db->memos;
	SqlStmt* stmt = NULL;
	struct point tmp_memo;
	char memo_map[MAP_NAME_LENGTH_EXT];
	bool result = false;
	int i;

	memset(list, 0, sizeof(*list));

	do
	{

	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`=? ORDER by `memo_id` LIMIT %d", db->memo_db, MAX_MEMOPOINTS)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &memo_map, sizeof(memo_map), NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_memo.x, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_memo.y, 0, NULL, NULL) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	for( i = 0; i < MAX_MEMOPOINTS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		tmp_memo.map = mapindex_name2id(memo_map);
		memcpy(&(*list)[i], &tmp_memo, sizeof(tmp_memo));
	}

	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	return result;
}


/// @private
static bool mmo_memolist_tosql(MemoDB_SQL* db, const memolist* list, int char_id)
{
	Sql* sql_handle = db->memos;
	StringBuf buf;
	bool result = false;
	int i, count;
	char esc_mapname[NAME_LENGTH*2+1];

	StringBuf_Init(&buf);

	do
	{

	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->memo_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ", db->memo_db);
	for( i = 0, count = 0; i < MAX_MEMOPOINTS; ++i )
	{
		if( (*list)[i].map == 0 )
			continue;

		if( count != 0 )
			StringBuf_AppendStr(&buf, ",");

		Sql_EscapeString(sql_handle, esc_mapname, mapindex_id2name((*list)[i].map));
		StringBuf_Printf(&buf, "('%d', '%s', '%d', '%d')", char_id, esc_mapname, (*list)[i].x, (*list)[i].y);
		++count;
	}

	if( count > 0 )
	{
		if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		{
			Sql_ShowDebug(sql_handle);
			break;
		}
	}

	result = true;

	}
	while(0);

	StringBuf_Destroy(&buf);
	return result;
}


/// @protected
static bool memo_db_sql_init(MemoDB* self)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	db->memos = db->owner->sql_handle;
	return true;
}


/// @protected
static void memo_db_sql_destroy(MemoDB* self)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	db->memos = NULL;
	aFree(db);
}


/// @protected
static bool memo_db_sql_sync(MemoDB* self, bool force)
{
	return true;
}


/// @protected
static bool memo_db_sql_remove(MemoDB* self, const int char_id)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	Sql* sql_handle = db->memos;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->memo_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool memo_db_sql_save(MemoDB* self, const memolist* list, const int char_id)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	return mmo_memolist_tosql(db, list, char_id);
}


/// @protected
static bool memo_db_sql_load(MemoDB* self, memolist* list, const int char_id)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	return mmo_memolist_fromsql(db, list, char_id);
}


/// Returns an iterator over all memo lists.
/// @protected
static CSDBIterator* memo_db_sql_iterator(MemoDB* self)
{
	MemoDB_SQL* db = (MemoDB_SQL*)self;
	return csdb_sql_iterator(db->memos, db->memo_db, "char_id");
}


/// Constructs a new MemoDB interface.
/// @protected
MemoDB* memo_db_sql(CharServerDB_SQL* owner)
{
	MemoDB_SQL* db = (MemoDB_SQL*)aCalloc(1, sizeof(MemoDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &memo_db_sql_init;
	db->vtable.p.destroy = &memo_db_sql_destroy;
	db->vtable.p.sync    = &memo_db_sql_sync;
	db->vtable.remove    = &memo_db_sql_remove;
	db->vtable.save      = &memo_db_sql_save;
	db->vtable.load      = &memo_db_sql_load;
	db->vtable.iterator  = &memo_db_sql_iterator;

	// initialize state
	db->owner = owner;
	db->memos = NULL;

	// other settings
	db->memo_db = db->owner->table_memos;

	return &db->vtable;
}
