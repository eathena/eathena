// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "castledb.h"
#include "charserverdb_sql.h"
#include <stdlib.h>
#include <string.h>

/// internal structure
typedef struct CastleDB_SQL
{
	CastleDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* castles;       // SQL castle storage

	// other settings
	const char* castle_db;

} CastleDB_SQL;

/// internal structure
typedef struct CastleDBIterator_SQL
{
	CastleDBIterator vtable;    // public interface

	CastleDB_SQL* db;
	int last_castle_id;
} CastleDBIterator_SQL;



static bool mmo_castle_fromsql(CastleDB_SQL* db, struct guild_castle* gc, int castle_id)
{
	Sql* sql_handle = db->castles;
	char* data;

	if( gc == NULL )
		return false;
	if( castle_id == -1 ) //TODO: is this needed?
		return false;

	memset(gc,0,sizeof(struct guild_castle));

	// query for castle data
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, "
		"`visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`"
		" FROM `%s` WHERE `castle_id`='%d'", db->castle_db, castle_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); gc->castle_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); gc->guild_id =  atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); gc->economy = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); gc->defense = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); gc->triggerE = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); gc->triggerD = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); gc->nextTime = atoi(data);
	Sql_GetData(sql_handle,  7, &data, NULL); gc->payTime = atoi(data);
	Sql_GetData(sql_handle,  8, &data, NULL); gc->createTime = atoi(data);
	Sql_GetData(sql_handle,  9, &data, NULL); gc->visibleC = atoi(data);
	Sql_GetData(sql_handle, 10, &data, NULL); gc->guardian[0].visible = atoi(data);
	Sql_GetData(sql_handle, 11, &data, NULL); gc->guardian[1].visible = atoi(data);
	Sql_GetData(sql_handle, 12, &data, NULL); gc->guardian[2].visible = atoi(data);
	Sql_GetData(sql_handle, 13, &data, NULL); gc->guardian[3].visible = atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); gc->guardian[4].visible = atoi(data);
	Sql_GetData(sql_handle, 15, &data, NULL); gc->guardian[5].visible = atoi(data);
	Sql_GetData(sql_handle, 16, &data, NULL); gc->guardian[6].visible = atoi(data);
	Sql_GetData(sql_handle, 17, &data, NULL); gc->guardian[7].visible = atoi(data);

	Sql_FreeResult(sql_handle);

	return true;
}


static bool mmo_castle_tosql(CastleDB_SQL* db, const struct guild_castle* gc)
{
	// `guild_castle` (`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`)
	Sql* sql_handle = db->castles;

	if ( gc == NULL )
		return false;

	if( SQL_ERROR == Sql_Query(sql_handle,
		"REPLACE INTO `%s` "
		"(`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`,"
		"`visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`)"
		"VALUES "
		"('%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d')",
		db->castle_db, gc->castle_id, gc->guild_id,  gc->economy, gc->defense,
		gc->triggerE, gc->triggerD, gc->nextTime, gc->payTime, gc->createTime, gc->visibleC,
		gc->guardian[0].visible, gc->guardian[1].visible, gc->guardian[2].visible, gc->guardian[3].visible,
		gc->guardian[4].visible, gc->guardian[5].visible, gc->guardian[6].visible, gc->guardian[7].visible)
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


static bool castle_db_sql_init(CastleDB* self)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	db->castles = db->owner->sql_handle;
	return true;
}

static void castle_db_sql_destroy(CastleDB* self)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	db->castles = NULL;
	aFree(db);
}

static bool castle_db_sql_sync(CastleDB* self)
{
	return true;
}

static bool castle_db_sql_create(CastleDB* self, struct guild_castle* gc)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	return mmo_castle_tosql(db, gc);
}

static bool castle_db_sql_remove(CastleDB* self, const int castle_id)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	Sql* sql_handle = db->castles;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `castle_id` = '%d'", db->castle_db, castle_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}

static bool castle_db_sql_remove_gid(CastleDB* self, const int guild_id)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	Sql* sql_handle = db->castles;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", db->castle_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}

static bool castle_db_sql_save(CastleDB* self, const struct guild_castle* gc)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	return mmo_castle_tosql(db, gc);
}

static bool castle_db_sql_load(CastleDB* self, struct guild_castle* gc, int castle_id)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	return mmo_castle_fromsql(db, gc, castle_id);
}


/// Destroys this iterator, releasing all allocated memory (including itself).
static void castle_db_sql_iter_destroy(CastleDBIterator* self)
{
	CastleDBIterator_SQL* iter = (CastleDBIterator_SQL*)self;
	aFree(iter);
}

/// Fetches the next account in the database.
static bool castle_db_sql_iter_next(CastleDBIterator* self, struct guild_castle* gc)
{
	CastleDBIterator_SQL* iter = (CastleDBIterator_SQL*)self;
	CastleDB_SQL* db = (CastleDB_SQL*)iter->db;
	Sql* sql_handle = db->castles;
	int castle_id;
	char* data;

	// get next castle ID
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `castle_id` FROM `%s` WHERE `castle_id` > '%d' ORDER BY `castle_id` ASC LIMIT 1",
		db->castle_db, iter->last_castle_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) &&
		SQL_SUCCESS == Sql_GetData(sql_handle, 0, &data, NULL) &&
		data != NULL )
	{// get castle data
		castle_id = atoi(data);
		if( mmo_castle_fromsql(db, gc, castle_id) )
		{
			iter->last_castle_id = castle_id;
			Sql_FreeResult(sql_handle);
			return true;
		}
	}
	Sql_FreeResult(sql_handle);
	return false;
}

/// Returns a new forward iterator.
static CastleDBIterator* castle_db_sql_iterator(CastleDB* self)
{
	CastleDB_SQL* db = (CastleDB_SQL*)self;
	CastleDBIterator_SQL* iter = (CastleDBIterator_SQL*)aCalloc(1, sizeof(CastleDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &castle_db_sql_iter_destroy;
	iter->vtable.next    = &castle_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->last_castle_id = -1;

	return &iter->vtable;
}


/// public constructor
CastleDB* castle_db_sql(CharServerDB_SQL* owner)
{
	CastleDB_SQL* db = (CastleDB_SQL*)aCalloc(1, sizeof(CastleDB_SQL));

	// set up the vtable
	db->vtable.init      = &castle_db_sql_init;
	db->vtable.destroy   = &castle_db_sql_destroy;
	db->vtable.sync      = &castle_db_sql_sync;
	db->vtable.create    = &castle_db_sql_create;
	db->vtable.remove    = &castle_db_sql_remove;
	db->vtable.remove_gid= &castle_db_sql_remove_gid;
	db->vtable.save      = &castle_db_sql_save;
	db->vtable.load      = &castle_db_sql_load;
	db->vtable.iterator  = &castle_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->castles = NULL;

	// other settings
	db->castle_db = db->owner->table_castles;

	return &db->vtable;
}
