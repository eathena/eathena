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


/// internal structure
typedef struct StatusDB_SQL
{
	StatusDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* statuses;      // SQL status storage

	// other settings
	const char* status_db;

} StatusDB_SQL;



static bool mmo_status_fromsql(StatusDB_SQL* db, struct scdata* sc, int char_id)
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

	//sc->account_id = account_id;
	sc->char_id = char_id;
	sc->count = (int)Sql_NumRows(sql_handle);
	sc->data = (struct status_change_data*)aMalloc(sc->count * sizeof(struct status_change_data));

	for( i = 0; i < sc->count && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); sc->data[i].type = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); sc->data[i].tick = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); sc->data[i].val1 = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); sc->data[i].val2 = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); sc->data[i].val3 = atoi(data);
		Sql_GetData(sql_handle, 5, &data, NULL); sc->data[i].val4 = atoi(data);
	}

	Sql_FreeResult(sql_handle);

	if( i < sc->count )
	{
		aFree(sc->data);
		return false;
	}

	return true;
}


static bool mmo_status_tosql(StatusDB_SQL* db, const struct scdata* sc)
{
	Sql* sql_handle = db->statuses;
	StringBuf buf;
	int i;
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

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id`='%d' AND `char_id`='%d'", db->status_db, sc->account_id, sc->char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( sc->count <= 0 )
	{// nothing more needed
		result = true;
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`account_id`, `char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ", db->status_db);

	for( i = 0; i < sc->count; ++i )
	{
		if( i > 0 )
			StringBuf_AppendStr(&buf, ", ");

		StringBuf_Printf(&buf, "('%d','%d','%hu','%d','%d','%d','%d','%d')", sc->account_id, sc->char_id,
			sc->data[i].type, sc->data[i].tick, sc->data[i].val1, sc->data[i].val2, sc->data[i].val3, sc->data[i].val4);
	}

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


static bool status_db_sql_init(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	db->statuses = db->owner->sql_handle;
	return true;
}

static void status_db_sql_destroy(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	db->statuses = NULL;
	aFree(db);
}

static bool status_db_sql_sync(StatusDB* self)
{
	return true;
}

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

static bool status_db_sql_save(StatusDB* self, struct scdata* sc)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return mmo_status_tosql(db, sc);
}

static bool status_db_sql_load(StatusDB* self, struct scdata* sc, int char_id)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return mmo_status_fromsql(db, sc, char_id);
}


/// Returns an iterator over all status entries.
static CSDBIterator* status_db_sql_iterator(StatusDB* self)
{
	StatusDB_SQL* db = (StatusDB_SQL*)self;
	return csdb_sql_iterator(db->statuses, db->status_db, "char_id");
}


/// public constructor
StatusDB* status_db_sql(CharServerDB_SQL* owner)
{
	StatusDB_SQL* db = (StatusDB_SQL*)aCalloc(1, sizeof(StatusDB_SQL));

	// set up the vtable
	db->vtable.init      = &status_db_sql_init;
	db->vtable.destroy   = &status_db_sql_destroy;
	db->vtable.sync      = &status_db_sql_sync;
	db->vtable.remove    = &status_db_sql_remove;
	db->vtable.save      = &status_db_sql_save;
	db->vtable.load      = &status_db_sql_load;
	db->vtable.iterator  = &status_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->statuses = NULL;

	// other settings
	db->status_db = db->owner->table_statuses;

	return &db->vtable;
}
