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
}

static bool mmo_friendlist_tosql(FriendDB_SQL* db, const friendlist* list, int char_id)
{
}
