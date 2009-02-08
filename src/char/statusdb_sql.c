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

/// internal functions
static bool status_db_sql_init(StatusDB* self);
static void status_db_sql_destroy(StatusDB* self);
static bool status_db_sql_sync(StatusDB* self);
static bool status_db_sql_remove(StatusDB* self, const int char_id);
static bool status_db_sql_save(StatusDB* self, struct scdata* sc);
static bool status_db_sql_load(StatusDB* self, struct scdata* sc, int char_id);

static bool mmo_status_fromsql(StatusDB_SQL* db, struct scdata* sc, int char_id);
static bool mmo_status_tosql(StatusDB_SQL* db, const struct scdata* sc);

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

	// initialize to default values
	db->owner = owner;
	db->statuses = NULL;

	// other settings
	db->status_db = db->owner->table_statuses;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


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

static bool status_db_sql_remove(StatusDB* self, const int char_id)
{
	/*
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, account_id, char_id) )
		Sql_ShowDebug(sql_handle);
	*/
}

static bool status_db_sql_save(StatusDB* self, struct scdata* sc)
{
	//TODO: destructive save

/*
	struct status_change_data data;
	StringBuf buf;
	int i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`account_id`, `char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ", scdata_db);
	for( i = 0; i < count; ++i )
	{
		memcpy (&data, RFIFOP(fd, 14+i*sizeof(struct status_change_data)), sizeof(struct status_change_data));
		if( i > 0 )
			StringBuf_AppendStr(&buf, ", ");
		StringBuf_Printf(&buf, "('%d','%d','%hu','%d','%d','%d','%d','%d')", aid, cid,
			data.type, data.tick, data.val1, data.val2, data.val3, data.val4);
	}
	if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);
	StringBuf_Destroy(&buf);
*/
}

static bool status_db_sql_load(StatusDB* self, struct scdata* sc, int char_id)
{
	//TODO: destructive load

/*
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT type, tick, val1, val2, val3, val4 from `%s` WHERE `account_id` = '%d' AND `char_id`='%d'",
		scdata_db, aid, cid) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}
	if( Sql_NumRows(sql_handle) > 0 )
	{
		struct status_change_data scdata;
		int count;
		char* data;

		WFIFOHEAD(fd,14+50*sizeof(struct status_change_data));
		WFIFOW(fd,0) = 0x2b1d;
		WFIFOL(fd,4) = aid;
		WFIFOL(fd,8) = cid;
		for( count = 0; count < 50 && SQL_SUCCESS == Sql_NextRow(sql_handle); ++count )
		{
			Sql_GetData(sql_handle, 0, &data, NULL); scdata.type = atoi(data);
			Sql_GetData(sql_handle, 1, &data, NULL); scdata.tick = atoi(data);
			Sql_GetData(sql_handle, 2, &data, NULL); scdata.val1 = atoi(data);
			Sql_GetData(sql_handle, 3, &data, NULL); scdata.val2 = atoi(data);
			Sql_GetData(sql_handle, 4, &data, NULL); scdata.val3 = atoi(data);
			Sql_GetData(sql_handle, 5, &data, NULL); scdata.val4 = atoi(data);
			memcpy(WFIFOP(fd, 14+count*sizeof(struct status_change_data)), &scdata, sizeof(struct status_change_data));
		}
		if (count >= 50)
			ShowWarning("Too many status changes for %d:%d, some of them were not loaded.\n", aid, cid);
		if (count > 0)
		{
			WFIFOW(fd,2) = 14 + count*sizeof(struct status_change_data);
			WFIFOW(fd,12) = count;
			WFIFOSET(fd,WFIFOW(fd,2));

			//Clear the data once loaded.
			if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, aid, cid) )
				Sql_ShowDebug(sql_handle);
		}
	}
	Sql_FreeResult(sql_handle);
*/
}


static bool mmo_status_fromsql(StatusDB_SQL* db, struct scdata* sc, int char_id)
{
}

static bool mmo_status_tosql(StatusDB_SQL* db, const struct scdata* sc)
{
}
