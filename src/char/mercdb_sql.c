// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "charserverdb_sql.h"
#include "petdb.h"
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct MercDB_SQL
{
	// public interface
	MercDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* mercs;

	// settings
	const char* merc_db;
	const char* merc_owner_db;

} MercDB_SQL;


/// @private
static bool mmo_merc_fromsql(MercDB_SQL* db, struct s_mercenary* md, int merc_id)
{
	Sql* sql_handle = db->mercs;
	char* data;

	memset(md, 0, sizeof(*md));

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `mer_id`, `char_id`, `class`, `hp`, `sp`, `kill_counter`, `life_time` FROM `mercenary` WHERE `mer_id` = '%d'", merc_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); md->mercenary_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); md->char_id = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); md->class_ = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); md->hp = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); md->sp = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); md->kill_count = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); md->life_time = atoi(data);
	Sql_FreeResult(sql_handle);
	
	return true;
}


/// @private
static bool mmo_merc_tosql(MercDB_SQL* db, struct s_mercenary* md, bool is_new)
{
	Sql* sql_handle = db->mercs;

	if( is_new )
	{ // Create new DB entry
		if( SQL_ERROR == Sql_Query(sql_handle,
			"INSERT INTO `mercenary` (`char_id`,`class`,`hp`,`sp`,`kill_counter`,`life_time`) VALUES ('%d','%d','%d','%d','%u','%u')",
			md->char_id, md->class_, md->hp, md->sp, md->kill_count, md->life_time) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		else
			md->mercenary_id = (int)Sql_LastInsertId(sql_handle);
	}
	else
	{
		if( SQL_ERROR == Sql_Query(sql_handle,
			"UPDATE `mercenary` SET `char_id` = '%d', `class` = '%d', `hp` = '%d', `sp` = '%d', `kill_counter` = '%u', `life_time` = '%u' WHERE `mer_id` = '%d'",
			md->char_id, md->class_, md->hp, md->sp, md->kill_count, md->life_time, md->mercenary_id) )
		{ // Update DB entry
			Sql_ShowDebug(sql_handle);
			return false;
		}
	}

	return true;
}


/// @protected
static bool merc_db_sql_init(MercDB* self)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	db->mercs = db->owner->sql_handle;
	return true;
}


/// @protected
static void merc_db_sql_destroy(MercDB* self)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	db->mercs = NULL;
	aFree(db);
}


/// @protected
static bool merc_db_sql_sync(MercDB* self, bool force)
{
	return true;
}


/// @protected
static bool merc_db_sql_create(MercDB* self, struct s_mercenary* md)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	return mmo_merc_tosql(db, md, true);
}


/// @protected
static bool merc_db_sql_remove(MercDB* self, const int merc_id)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	Sql* sql_handle = db->mercs;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `mer_id`='%d'", db->merc_db, merc_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool merc_db_sql_save(MercDB* self, const struct s_mercenary* md)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	return mmo_merc_tosql(db, (struct s_mercenary*)md, false);
}


/// @protected
static bool merc_db_sql_load(MercDB* self, struct s_mercenary* md, int merc_id)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	return mmo_merc_fromsql(db, md, merc_id);
}


/// Returns an iterator over all mercs.
/// @protected
static CSDBIterator* merc_db_sql_iterator(MercDB* self)
{
	MercDB_SQL* db = (MercDB_SQL*)self;
	return csdb_sql_iterator(db->mercs, db->merc_db, "mer_id");
}


/// Constructs a new MercDB interface.
/// @protected
MercDB* merc_db_sql(CharServerDB_SQL* owner)
{
	MercDB_SQL* db = (MercDB_SQL*)aCalloc(1, sizeof(MercDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &merc_db_sql_init;
	db->vtable.p.destroy = &merc_db_sql_destroy;
	db->vtable.p.sync    = &merc_db_sql_sync;
	db->vtable.create    = &merc_db_sql_create;
	db->vtable.remove    = &merc_db_sql_remove;
	db->vtable.save      = &merc_db_sql_save;
	db->vtable.load      = &merc_db_sql_load;
	db->vtable.iterator  = &merc_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->mercs = NULL;

	// other settings
	db->merc_db = db->owner->table_mercenaries;
	db->merc_owner_db = db->owner->table_mercenary_owners;

	return &db->vtable;
}
