// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "frienddb.h"
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct FriendDB_SQL
{
	FriendDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* friends;       // SQL friend storage

	// other settings
	const char* friend_db;
	const char* char_db;

} FriendDB_SQL;



static bool mmo_friendlist_fromsql(FriendDB_SQL* db, friendlist* list, int char_id)
{
	Sql* sql_handle = db->friends;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	struct s_friend tmp_friend;
	bool result = false;
	int i;

	memset(list, 0, sizeof(friendlist));

	do
	{

	//`friends` (`char_id`, `friend_account`, `friend_id`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT c.`account_id`, c.`char_id`, c.`name` FROM `%s` c LEFT JOIN `%s` f ON f.`friend_account` = c.`account_id` AND f.`friend_id` = c.`char_id` WHERE f.`char_id`=%d LIMIT %d", db->char_db, db->friend_db, char_id, MAX_FRIENDS)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_friend.account_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT,    &tmp_friend.char_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &tmp_friend.name, sizeof(tmp_friend.name), NULL, NULL) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	for( i = 0; i < MAX_FRIENDS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&(*list)[i], &tmp_friend, sizeof(tmp_friend));

	result = true;

	}
	while(0);

	return result;
}


static bool mmo_friendlist_tosql(FriendDB_SQL* db, const friendlist* list, int char_id)
{
	Sql* sql_handle = db->friends;
	StringBuf buf;
	int i, count;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	// try
	do
	{

	StringBuf_Init(&buf);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->friend_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `friend_account`, `friend_id`) VALUES ", db->friend_db);
	for( i = 0, count = 0; i < MAX_FRIENDS; ++i )
	{
		if( (*list)[i].char_id == 0 )
			continue;

		if( count != 0 )
			StringBuf_AppendStr(&buf, ",");

		StringBuf_Printf(&buf, "('%d','%d','%d')", char_id, (*list)[i].account_id, (*list)[i].char_id);
		count++;
	}

	if( count > 0 )
	{
		if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		{
			Sql_ShowDebug(sql_handle);
			break;
		}
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


static bool friend_db_sql_init(FriendDB* self)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	db->friends = db->owner->sql_handle;
	return true;
}

static void friend_db_sql_destroy(FriendDB* self)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	db->friends = NULL;
	aFree(db);
}

static bool friend_db_sql_sync(FriendDB* self)
{
	return true;
}

static bool friend_db_sql_remove(FriendDB* self, const int char_id)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	Sql* sql_handle = db->friends;
	bool result = false;

	do
	{

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->friend_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	// delete char from other's friend list
	//NOTE: Won't this cause problems for people who are already online? [Skotlex]
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `friend_id` = '%d'", db->friend_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	result = true;

	}
	while(0);

	return result;
}

static bool friend_db_sql_save(FriendDB* self, const friendlist* list, const int char_id)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	return mmo_friendlist_tosql(db, list, char_id);
}

static bool friend_db_sql_load(FriendDB* self, friendlist* list, const int char_id)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	return mmo_friendlist_fromsql(db, list, char_id);
}


/// Returns an iterator over all friend lists.
static CSDBIterator* friend_db_sql_iterator(FriendDB* self)
{
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	return csdb_sql_iterator(db->friends, db->friend_db, "char_id");
}


/// public constructor
FriendDB* friend_db_sql(CharServerDB_SQL* owner)
{
	FriendDB_SQL* db = (FriendDB_SQL*)aCalloc(1, sizeof(FriendDB_SQL));

	// set up the vtable
	db->vtable.init      = &friend_db_sql_init;
	db->vtable.destroy   = &friend_db_sql_destroy;
	db->vtable.sync      = &friend_db_sql_sync;
	db->vtable.remove    = &friend_db_sql_remove;
	db->vtable.save      = &friend_db_sql_save;
	db->vtable.load      = &friend_db_sql_load;
	db->vtable.iterator  = &friend_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->friends = NULL;

	// other settings
	db->friend_db = db->owner->table_friends;
	db->char_db = db->owner->table_chars;

	return &db->vtable;
}
