// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "storagedb.h"
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct StorageDB_SQL
{
	StorageDB vtable;   // public interface

	CharServerDB_SQL* owner;
	Sql* storages;      // SQL storage storage

	// other settings
	const char* storage_db;

} StorageDB_SQL;



static bool mmo_storage_fromsql(StorageDB_SQL* db, struct storage_data* s, int account_id)
{
	Sql* sql_handle = db->storages;
	StringBuf buf;
	struct item* item;
	char* data;
	int i;
	int j;

	memset(s, 0, sizeof(*s)); //clean up memory

	// storage {`account_id`,`id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `account_id`='%d' ORDER BY `nameid`", db->storage_db, account_id);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	for( i = 0; i < MAX_STORAGE && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		item = &s->items[i];
		Sql_GetData(sql_handle, 0, &data, NULL); item->id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); item->nameid = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); item->amount = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); item->equip = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); item->identify = atoi(data);
		Sql_GetData(sql_handle, 5, &data, NULL); item->refine = atoi(data);
		Sql_GetData(sql_handle, 6, &data, NULL); item->attribute = atoi(data);
		for( j = 0; j < MAX_SLOTS; ++j )
		{
			Sql_GetData(sql_handle, 7+j, &data, NULL); item->card[j] = atoi(data);
		}
	}
	s->storage_amount = i;

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}


static bool mmo_storage_tosql(StorageDB_SQL* db, const struct storage_data* s, int account_id)
{
	return memitemdata_to_sql(db->storages, s->items, MAX_STORAGE, account_id, db->storage_db, "account_id");
}


static bool storage_db_sql_init(StorageDB* self)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	db->storages = db->owner->sql_handle;
	return true;
}

static void storage_db_sql_destroy(StorageDB* self)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	db->storages = NULL;
	aFree(db);
}

static bool storage_db_sql_sync(StorageDB* self)
{
	return true;
}

static bool storage_db_sql_remove(StorageDB* self, const int account_id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	Sql* sql_handle = db->storages;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id`='%d'", db->storage_db, account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool storage_db_sql_save(StorageDB* self, const struct storage_data* s, int account_id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return mmo_storage_tosql(db, s, account_id);
}

static bool storage_db_sql_load(StorageDB* self, struct storage_data* s, int account_id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return mmo_storage_fromsql(db, s, account_id);
}


/// Returns an iterator over all storages.
static CSDBIterator* storage_db_sql_iterator(StorageDB* self)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return csdb_sql_iterator(db->storages, db->storage_db, "account_id");
}


/// public constructor
StorageDB* storage_db_sql(CharServerDB_SQL* owner)
{
	StorageDB_SQL* db = (StorageDB_SQL*)aCalloc(1, sizeof(StorageDB_SQL));

	// set up the vtable
	db->vtable.init      = &storage_db_sql_init;
	db->vtable.destroy   = &storage_db_sql_destroy;
	db->vtable.sync      = &storage_db_sql_sync;
	db->vtable.remove    = &storage_db_sql_remove;
	db->vtable.save      = &storage_db_sql_save;
	db->vtable.load      = &storage_db_sql_load;
	db->vtable.iterator  = &storage_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->storages = NULL;

	// other settings
	db->storage_db = db->owner->table_storages;

	return &db->vtable;
}
