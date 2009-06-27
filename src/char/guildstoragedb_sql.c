// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "char.h" // TABLE_GUILD_STORAGE
#include "charserverdb_sql.h"
#include "guildstoragedb.h"
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct GuildStorageDB_SQL
{
	GuildStorageDB vtable;   // public interface

	CharServerDB_SQL* owner;
	Sql* guildstorages;      // SQL guild storage storage

	// other settings
	const char* guildstorage_db;

} GuildStorageDB_SQL;



static bool mmo_guildstorage_fromsql(GuildStorageDB_SQL* db, struct guild_storage* gs, int guild_id)
{
	Sql* sql_handle = db->guildstorages;
	StringBuf buf;
	struct item* item;
	char* data;
	int i;
	int j;

	memset(gs, 0, sizeof(struct guild_storage)); //clean up memory
	gs->storage_amount = 0;
	gs->guild_id = guild_id;

	// storage {`guild_id`,`id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3`}
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `guild_id`='%d' ORDER BY `nameid`", db->guildstorage_db, guild_id);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	for( i = 0; i < MAX_GUILD_STORAGE && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		item = &gs->storage_[i];
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
	gs->storage_amount = i;

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}


static bool mmo_guildstorage_tosql(GuildStorageDB_SQL* db, const struct guild_storage* gs, int guild_id)
{
	return memitemdata_to_sql(db->guildstorages, gs->storage_, MAX_GUILD_STORAGE, guild_id, db->guildstorage_db, "guild_id");
}


static bool guildstorage_db_sql_init(GuildStorageDB* self)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	db->guildstorages = db->owner->sql_handle;
	return true;
}

static void guildstorage_db_sql_destroy(GuildStorageDB* self)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	db->guildstorages = NULL;
	aFree(db);
}

static bool guildstorage_db_sql_sync(GuildStorageDB* self)
{
	return true;
}

static bool guildstorage_db_sql_remove(GuildStorageDB* self, const int guild_id)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	Sql* sql_handle = db->guildstorages;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d'", db->guildstorage_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool guildstorage_db_sql_save(GuildStorageDB* self, const struct guild_storage* gs, int guild_id)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	return mmo_guildstorage_tosql(db, gs, guild_id);
}

static bool guildstorage_db_sql_load(GuildStorageDB* self, struct guild_storage* gs, int guild_id)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	return mmo_guildstorage_fromsql(db, gs, guild_id);
}


/// Returns an iterator over all guild storages.
static CSDBIterator* guildstorage_db_sql_iterator(GuildStorageDB* self)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)self;
	return csdb_sql_iterator(db->guildstorages, db->guildstorage_db, "guild_id");
}


/// public constructor
GuildStorageDB* guildstorage_db_sql(CharServerDB_SQL* owner)
{
	GuildStorageDB_SQL* db = (GuildStorageDB_SQL*)aCalloc(1, sizeof(GuildStorageDB_SQL));

	// set up the vtable
	db->vtable.init      = &guildstorage_db_sql_init;
	db->vtable.destroy   = &guildstorage_db_sql_destroy;
	db->vtable.sync      = &guildstorage_db_sql_sync;
	db->vtable.remove    = &guildstorage_db_sql_remove;
	db->vtable.save      = &guildstorage_db_sql_save;
	db->vtable.load      = &guildstorage_db_sql_load;
	db->vtable.iterator  = &guildstorage_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->guildstorages = NULL;

	// other settings
	db->guildstorage_db = db->owner->table_guild_storages;

	return &db->vtable;
}
