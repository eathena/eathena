// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "statusdb.h"
#include <stdlib.h>


/// Internal structure.
/// @private
typedef struct StatusDB_SQL
{
	// public interface
	StatusDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* statuses;

	// settings
	const char* status_db;

} StatusDB_SQL;


/// @private
static bool mmo_status_fromsql(StatusDB_SQL* db, struct status_change_data* sc, int size, int char_id)
{
	Sql* sql_handle = db->statuses;
	char* data;
	int i;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT type, tick, val1, val2, val3, val4 from `%s` WHERE `char_id`='%d'",
		db->status_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	for( i = 0; i < size && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); sc[i].type = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); sc[i].tick = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); sc[i].val1 = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); sc[i].val2 = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); sc[i].val3 = atoi(data);
		Sql_GetData(sql_handle, 5, &data, NULL); sc[i].val4 = atoi(data);
	}

	Sql_FreeResult(sql_handle);

	return true;
}


/// @private
static bool mmo_status_tosql(StatusDB_SQL* db, const struct status_change_data* sc, int count, int char_id)
{
	Sql* sql_handle = db->statuses;
	StringBuf buf;
	int i;
	bool first = true;
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

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->status_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ", db->status_db);

	for( i = 0; i < count; ++i )
	{
		if( sc[i].type == (unsigned short)-1 )
			continue;

		if( first )
			first = false;
		else
			StringBuf_AppendStr(&buf, " ");

		StringBuf_Printf(&buf, "('%d','%hu','%d','%d','%d','%d','%d')", char_id, sc[i].type, sc[i].tick, sc[i].val1, sc[i].val2, sc[i].val3, sc[i].val4);
	}

	if( !first ) // run query only if there's any data to write
	if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

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
static bool status_db_sql_init(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	db->statuses = db->owner->sql_handle;
	return true;
}


/// @protected
static void status_db_sql_destroy(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	db->statuses = NULL;
	aFree(db);
}


/// @protected
static bool status_db_sql_sync(StatusDB* self, bool force)
{
	return true;
}


/// @protected
static bool status_db_sql_remove(StatusDB* self, int char_id)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	Sql* sql_handle = db->statuses;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->status_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool status_db_sql_save(StatusDB* self, const struct status_change_data* sc, int count, int char_id)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return mmo_status_tosql(db, sc, count, char_id);
}


/// @protected
static bool status_db_sql_load(StatusDB* self, struct status_change_data* sc, int count, int char_id)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return mmo_status_fromsql(db, sc, count, char_id);
}


/// @protected
static int status_db_sql_count(StatusDB* self, int char_id)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	char* data;
	int result;

	if( SQL_SUCCESS != Sql_Query(db->statuses, "SELECT COUNT(*) FROM `%s` WHERE `char_id` = %d", db->status_db, char_id)
	||  SQL_SUCCESS != Sql_NextRow(db->statuses)
	) {
		Sql_ShowDebug(db->statuses);
		Sql_FreeResult(db->statuses);
		return 0;
	}

	Sql_GetData(db->statuses, 0, &data, NULL);
	result = atoi(data);

	return result;
}


/// Returns an iterator over all status entries.
/// @protected
static CSDBIterator* status_db_sql_iterator(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return csdb_sql_iterator(db->statuses, db->status_db, "char_id");
}


/// Constructs a new StatusDB interface.
/// @protected
StatusDB* status_db_sql(CharServerDB_SQL* owner)
{
	StatusDB_SQL* db = (StatusDB_SQL*)aCalloc(1, sizeof(StatusDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &status_db_sql_init;
	db->vtable.p.destroy = &status_db_sql_destroy;
	db->vtable.p.sync    = &status_db_sql_sync;
	db->vtable.remove    = &status_db_sql_remove;
	db->vtable.save      = &status_db_sql_save;
	db->vtable.load      = &status_db_sql_load;
	db->vtable.count     = &status_db_sql_count;
	db->vtable.iterator  = &status_db_sql_iterator;

	// initialize state
	db->owner = owner;
	db->statuses = NULL;

	// other settings
	db->status_db = db->owner->table_statuses;

	return &db->vtable;
}
