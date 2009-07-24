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


/// internal structure
typedef struct PetDB_SQL
{
	PetDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* pets;       // SQL pet storage

	// other settings
	const char* pet_db;

} PetDB_SQL;



static bool mmo_pet_fromsql(PetDB_SQL* db, struct s_pet* pd, int pet_id)
{
	Sql* sql_handle = db->pets;
	SqlStmt* stmt;
	StringBuf buf;
	bool result = false;

	memset(pd, 0, sizeof(*pd));

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate` FROM `%s` WHERE `pet_id`='%d'", db->pet_db, pet_id);

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);

	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( SqlStmt_NumRows(stmt) == 0 )
		break; // no pet found

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT, (void*)&pd->pet_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT, (void*)&pd->class_, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, (void*)pd->name, sizeof(pd->name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_INT, (void*)&pd->account_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_INT, (void*)&pd->char_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_SHORT, (void*)&pd->level, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_SHORT, (void*)&pd->egg_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_SHORT, (void*)&pd->equip, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_SHORT, (void*)&pd->intimate, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_SHORT, (void*)&pd->hungry, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_CHAR, (void*)&pd->rename_flag, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 11, SQLDT_CHAR, (void*)&pd->incuvate, 0, NULL, NULL);

	if( SQL_SUCCESS != SqlStmt_NextRow(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	pd->hungry = cap_value(pd->hungry, 0, 100);
	pd->intimate = cap_value(pd->intimate, 0, 1000);

	// success
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


static bool mmo_pet_tosql(PetDB_SQL* db, struct s_pet* pd, bool is_new)
{
	Sql* sql_handle = db->pets;
	StringBuf buf;
	SqlStmt* stmt = NULL;
	bool result = false;

	pd->hungry = cap_value(pd->hungry, 0, 100);
	pd->intimate = cap_value(pd->intimate, 0, 1000);

	StringBuf_Init(&buf);

	if( is_new )
	{
		StringBuf_Printf(&buf,
			"INSERT INTO `%s` "
			"(`class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`,`pet_id`) "
			"VALUES "
			"(?,?,?,?,?,?,?,?,?,?,?,?)"
			, db->pet_db);
	}
	else
	{
		StringBuf_Printf(&buf,
			"UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%d'"
			, db->pet_db);
	}

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_SHORT, (void*)&pd->class_, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)pd->name, strnlen(pd->name, ARRAYLENGTH(pd->name)))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_INT, (void*)&pd->account_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_INT, (void*)&pd->char_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_SHORT, (void*)&pd->level, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 5, SQLDT_SHORT, (void*)&pd->egg_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 6, SQLDT_SHORT, (void*)&pd->equip, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 7, SQLDT_SHORT, (void*)&pd->intimate, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 8, SQLDT_SHORT, (void*)&pd->hungry, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 9, SQLDT_CHAR, (void*)&pd->rename_flag, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_CHAR, (void*)&pd->incuvate, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, (pd->pet_id != -1)?SQLDT_INT:SQLDT_NULL, (void*)&pd->pet_id, 0)
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( is_new )
	{
		// fill in output value
		if( pd->pet_id == -1 )
			pd->pet_id = (int)SqlStmt_LastInsertId(stmt);
	}

	// success
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


static bool pet_db_sql_init(PetDB* self)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	db->pets = db->owner->sql_handle;
	return true;
}

static void pet_db_sql_destroy(PetDB* self)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	db->pets = NULL;
	aFree(db);
}

static bool pet_db_sql_sync(PetDB* self)
{
	return true;
}

static bool pet_db_sql_create(PetDB* self, struct s_pet* pd)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	return mmo_pet_tosql(db, pd, true);
}

static bool pet_db_sql_remove(PetDB* self, const int pet_id)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	Sql* sql_handle = db->pets;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `pet_id`='%d'", db->pet_db, pet_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool pet_db_sql_save(PetDB* self, const struct s_pet* pd)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	return mmo_pet_tosql(db, (struct s_pet*)pd, false);
}

static bool pet_db_sql_load(PetDB* self, struct s_pet* pd, int pet_id)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	return mmo_pet_fromsql(db, pd, pet_id);
}


/// Returns an iterator over all pets.
static CSDBIterator* pet_db_sql_iterator(PetDB* self)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	return csdb_sql_iterator(db->pets, db->pet_db, "pet_id");
}


/// public constructor
PetDB* pet_db_sql(CharServerDB_SQL* owner)
{
	PetDB_SQL* db = (PetDB_SQL*)aCalloc(1, sizeof(PetDB_SQL));

	// set up the vtable
	db->vtable.init      = &pet_db_sql_init;
	db->vtable.destroy   = &pet_db_sql_destroy;
	db->vtable.sync      = &pet_db_sql_sync;
	db->vtable.create    = &pet_db_sql_create;
	db->vtable.remove    = &pet_db_sql_remove;
	db->vtable.save      = &pet_db_sql_save;
	db->vtable.load      = &pet_db_sql_load;
	db->vtable.iterator  = &pet_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->pets = NULL;

	// other settings
	db->pet_db = db->owner->table_pets;

	return &db->vtable;
}
