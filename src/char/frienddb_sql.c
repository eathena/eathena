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


/// internal structure
typedef struct FriendDB_SQL
{
	FriendDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* friends;       // SQL friend storage

	// other settings
	char friend_db[32];

} FriendDB_SQL;

/// internal functions
static bool friend_db_sql_init(FriendDB* self);
static void friend_db_sql_destroy(FriendDB* self);
static bool friend_db_sql_sync(FriendDB* self);
static bool friend_db_sql_remove(FriendDB* self, const int char_id);
static bool friend_db_sql_save(FriendDB* self, const friendlist* list, const int char_id);
static bool friend_db_sql_load(FriendDB* self, friendlist* list, const int char_id);

static bool mmo_friendlist_fromsql(FriendDB_SQL* db, friendlist* list, int char_id);
static bool mmo_friendlist_tosql(FriendDB_SQL* db, const friendlist* list, int char_id);

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

	// initialize to default values
	db->owner = owner;
	db->friends = NULL;
	// other settings
	safestrncpy(db->friend_db, "friends", sizeof(db->friend_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


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
/*
	FriendDB_SQL* db = (FriendDB_SQL*)self;
	Sql* sql_handle = db->friends;

	//TODO: doesn't return proper value

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->friend_db, char_id) )
		Sql_ShowDebug(sql_handle);

	// delete char from other's friend list
	//NOTE: Won't this cause problems for people who are already online? [Skotlex]
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `friend_id` = '%d'", friend_db, char_id) )
		Sql_ShowDebug(sql_handle);
*/
	return true;
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


static bool mmo_friendlist_fromsql(FriendDB_SQL* db, friendlist* list, int char_id)
{
/*
	struct s_friend tmp_friend;

	//`friends` (`char_id`, `friend_account`, `friend_id`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT c.`account_id`, c.`char_id`, c.`name` FROM `%s` c LEFT JOIN `%s` f ON f.`friend_account` = c.`account_id` AND f.`friend_id` = c.`char_id` WHERE f.`char_id`=? LIMIT %d", char_db, friend_db, MAX_FRIENDS)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_friend.account_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT,    &tmp_friend.char_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &tmp_friend.name, sizeof(tmp_friend.name), NULL, NULL) )
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_FRIENDS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->friends[i], &tmp_friend, sizeof(tmp_friend));
*/
}

static bool mmo_friendlist_tosql(FriendDB_SQL* db, const friendlist* list, int char_id)
{
/*
	//Save friends
	ARR_FIND( 0, MAX_FRIENDS, i, p->friends[i].char_id != cp->friends[i].char_id || p->friends[i].account_id != cp->friends[i].account_id );
	if( i < MAX_FRIENDS )
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", friend_db, p->char_id) )
			Sql_ShowDebug(sql_handle);

		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `friend_account`, `friend_id`) VALUES ", friend_db);
		for( i = 0, count = 0; i < MAX_FRIENDS; ++i )
		{
			if( p->friends[i].char_id > 0 )
			{
				if( count != 0 )
					StringBuf_AppendStr(&buf, ",");
				StringBuf_Printf(&buf, "('%d','%d','%d')", p->char_id, p->friends[i].account_id, p->friends[i].char_id);
				count++;
			}
		}
		if( count > 0 )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
				Sql_ShowDebug(sql_handle);
		}
	}
*/
}
