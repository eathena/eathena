// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h" // ARR_FIND()
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "storagedb.h"
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct StorageDB_SQL
{
	// public interface
	StorageDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* storages;

	// settings
	const char* inventory_db;
	const char* cart_db;
	const char* storage_db;
	const char* guildstorage_db;

} StorageDB_SQL;


/// @private
static const char* type2table(StorageDB_SQL* db, enum storage_type type)
{
	switch( type )
	{
	case STORAGE_INVENTORY: return db->inventory_db;
	case STORAGE_CART     : return db->cart_db;
	case STORAGE_KAFRA    : return db->storage_db;
	case STORAGE_GUILD    : return db->guildstorage_db;
	default:
		return NULL;
	}
}


/// @private
static const char* type2column(StorageDB_SQL* db, enum storage_type type)
{
	switch( type )
	{
	case STORAGE_INVENTORY: return "char_id";
	case STORAGE_CART     : return "char_id";
	case STORAGE_KAFRA    : return "account_id";
	case STORAGE_GUILD    : return "guild_id";
	default:
		return NULL;
	}
}


/// Loads an array of 'item' entries from the specified table.
/// @private
static bool mmo_storage_fromsql(StorageDB_SQL* db, struct item* items, size_t size, enum storage_type type, int id)
{
	Sql* sql_handle = db->storages;
	const char* tablename = type2table(db,type);
	const char* selectoption = type2column(db,type);
	StringBuf buf;
	char* data;
	size_t i, j;

	memset(items, 0, size * sizeof(struct item)); //clean up memory

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`expire_time`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `%s`='%d' ORDER BY `nameid`", tablename, selectoption, id);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	for( i = 0; i < size && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); items[i].id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); items[i].nameid = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); items[i].amount = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); items[i].equip = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); items[i].identify = atoi(data);
		Sql_GetData(sql_handle, 5, &data, NULL); items[i].refine = atoi(data);
		Sql_GetData(sql_handle, 6, &data, NULL); items[i].attribute = atoi(data);
		Sql_GetData(sql_handle, 7, &data, NULL); items[i].expire_time = (unsigned int)atoi(data);
		for( j = 0; j < MAX_SLOTS; ++j )
		{
			Sql_GetData(sql_handle, 8+j, &data, NULL); items[i].card[j] = atoi(data);
		}
	}

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}


/// Saves an array of 'item' entries into the specified table.
/// @private
static bool mmo_storage_tosql(StorageDB_SQL* db, const struct item* items, size_t size, enum storage_type type, int id)
{
	Sql* sql_handle = db->storages;
	const char* tablename = type2table(db,type);
	const char* selectoption = type2column(db,type);
	StringBuf buf;
	SqlStmt* stmt;
	size_t i, j;
	struct item tmp_item; // temp storage variable
	bool* flag; // bit array for inventory matching
	bool found;

	// The following code compares inventory with current database values
	// and performs modification/deletion/insertion only on relevant rows.
	// This approach is more complicated than a trivial delete&insert, but
	// it significantly reduces cpu load on the database server.

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `%s`='%d'", tablename, selectoption, id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return 1;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &tmp_item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,   &tmp_item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,   &tmp_item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,   &tmp_item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,   &tmp_item.expire_time, 0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SqlStmt_BindColumn(stmt, 8+j, SQLDT_SHORT, &tmp_item.card[j], 0, NULL, NULL);

	// bit array indicating which inventory items have already been matched
	flag = (bool*) aCallocA(size, sizeof(bool));

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		found = false;
		// search for the presence of the item in the char's inventory
		for( i = 0; i < size; ++i )
		{
			// skip empty and already matched entries
			if( items[i].nameid == 0 || flag[i] )
				continue;

			if( items[i].nameid == tmp_item.nameid
			&&  items[i].card[0] == tmp_item.card[0]
			&&  items[i].card[2] == tmp_item.card[2]
			&&  items[i].card[3] == tmp_item.card[3]
			) {	//They are the same item.
				ARR_FIND( 0, MAX_SLOTS, j, items[i].card[j] != tmp_item.card[j] );
				if( j == MAX_SLOTS &&
				    items[i].amount == tmp_item.amount &&
				    items[i].equip == tmp_item.equip &&
				    items[i].identify == tmp_item.identify &&
				    items[i].refine == tmp_item.refine &&
				    items[i].attribute == tmp_item.attribute &&
				    items[i].expire_time == tmp_item.expire_time )
				;	//Do nothing.
				else
				{
					// update all fields.
					StringBuf_Clear(&buf);
					StringBuf_Printf(&buf, "UPDATE `%s` SET `amount`='%d', `equip`='%d', `identify`='%d', `refine`='%d',`attribute`='%d', `expire_time`='%u'",
						tablename, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time);
					for( j = 0; j < MAX_SLOTS; ++j )
						StringBuf_Printf(&buf, ", `card%d`=%d", j, items[i].card[j]);
					StringBuf_Printf(&buf, " WHERE `id`='%d' LIMIT 1", tmp_item.id);
					
					if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
						Sql_ShowDebug(sql_handle);
				}

				found = flag[i] = true; //Item dealt with,
				break; //skip to next item in the db.
			}
		}
		if( !found )
		{// Item not present in inventory, remove it.
			if( SQL_ERROR == Sql_Query(sql_handle, "DELETE from `%s` where `id`='%d'", tablename, tmp_item.id) )
				Sql_ShowDebug(sql_handle);
		}
	}
	SqlStmt_Free(stmt);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`", tablename, selectoption);
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_AppendStr(&buf, ") VALUES ");

	found = false;
	// insert non-matched items into the db as new items
	for( i = 0; i < size; ++i )
	{
		// skip empty and already matched entries
		if( items[i].nameid == 0 || flag[i] )
			continue;

		if( found )
			StringBuf_AppendStr(&buf, ",");
		else
			found = true;

		StringBuf_Printf(&buf, "('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u'",
			id, items[i].nameid, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time);
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ", '%d'", items[i].card[j]);
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);
	aFree(flag);

	return true;
}


/// @protected
static bool storage_db_sql_init(StorageDB* self)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	db->storages = db->owner->sql_handle;
	return true;
}


/// @protected
static void storage_db_sql_destroy(StorageDB* self)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	db->storages = NULL;
	aFree(db);
}


/// @protected
static bool storage_db_sql_sync(StorageDB* self, bool force)
{
	return true;
}


/// @protected
static bool storage_db_sql_remove(StorageDB* self, enum storage_type type, const int id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	Sql* sql_handle = db->storages;
	const char* table = type2table(db,type);
	const char* column = type2column(db,type);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `%s`='%d'", table, column, id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// Writes item data to the database.
/// @param self Database
/// @param s Array of item structures
/// @param size Number of fields in the array
/// @param type Type of storage
/// @param id Id of the storage data
/// @protected
static bool storage_db_sql_save(StorageDB* self, const struct item* s, size_t size, enum storage_type type, int id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return mmo_storage_tosql(db, s, size, type, id);
}


/// Loads item data from the database.
/// @param self Database
/// @param s Array of item structures
/// @param size Number of fields in the array
/// @param type Type of storage
/// @param id Id of the storage data
/// @protected
static bool storage_db_sql_load(StorageDB* self, struct item* s, size_t size, enum storage_type type, int id)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return mmo_storage_fromsql(db, s, size, type, id);
}


/// Returns an iterator over all storages.
/// @param self Database
/// @param type Type of storage
/// @return Iterator
/// @protected
static CSDBIterator* storage_db_sql_iterator(StorageDB* self, enum storage_type type)
{
	StorageDB_SQL* db = (StorageDB_SQL*)self;
	return csdb_sql_iterator(db->storages, type2table(db,type), type2column(db,type));
}


/// Constructs a new StorageDB interface.
/// @protected
StorageDB* storage_db_sql(CharServerDB_SQL* owner)
{
	StorageDB_SQL* db = (StorageDB_SQL*)aCalloc(1, sizeof(StorageDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &storage_db_sql_init;
	db->vtable.p.destroy = &storage_db_sql_destroy;
	db->vtable.p.sync    = &storage_db_sql_sync;
	db->vtable.remove    = &storage_db_sql_remove;
	db->vtable.save      = &storage_db_sql_save;
	db->vtable.load      = &storage_db_sql_load;
	db->vtable.iterator  = &storage_db_sql_iterator;

	// initialize state
	db->owner = owner;
	db->storages = NULL;

	// other settings
	db->inventory_db = db->owner->table_inventories;
	db->cart_db = db->owner->table_carts;
	db->storage_db = db->owner->table_storages;
	db->guildstorage_db = db->owner->table_guild_storages;

	return &db->vtable;
}
