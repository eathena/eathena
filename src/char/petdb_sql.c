// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "petdb.h"
#include <stdlib.h>


/// internal structure
typedef struct PetDB_SQL
{
	PetDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* pets;       // SQL pet storage

	// other settings
	bool case_sensitive;
	const char* pet_db;

} PetDB_SQL;



static bool mmo_pet_fromsql(PetDB_SQL* db, struct s_pet* pd, int pet_id)
{
/*
	char* data;
	size_t len;

	memset(p, 0, sizeof(struct s_pet));

	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate` FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		p->pet_id = pet_id;
		Sql_GetData(sql_handle,  1, &data, NULL); p->class_ = atoi(data);
		Sql_GetData(sql_handle,  2, &data, &len); memcpy(p->name, data, min(len, NAME_LENGTH));
		Sql_GetData(sql_handle,  3, &data, NULL); p->account_id = atoi(data);
		Sql_GetData(sql_handle,  4, &data, NULL); p->char_id = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); p->level = atoi(data);
		Sql_GetData(sql_handle,  6, &data, NULL); p->egg_id = atoi(data);
		Sql_GetData(sql_handle,  7, &data, NULL); p->equip = atoi(data);
		Sql_GetData(sql_handle,  8, &data, NULL); p->intimate = atoi(data);
		Sql_GetData(sql_handle,  9, &data, NULL); p->hungry = atoi(data);
		Sql_GetData(sql_handle, 10, &data, NULL); p->rename_flag = atoi(data);
		Sql_GetData(sql_handle, 11, &data, NULL); p->incuvate = atoi(data);

		Sql_FreeResult(sql_handle);

		p->hungry = cap_value(p->hungry, 0, 100);
		p->intimate = cap_value(p->intimate, 0, 1000);

		if( save_log )
			ShowInfo("Pet loaded (%d - %s).\n", pet_id, p->name);
	}
	return 0;
*/
}


static bool mmo_pet_tosql(PetDB_SQL* db, const struct s_pet* pd, bool is_new)
{
/*
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	char esc_name[NAME_LENGTH*2+1];// escaped pet name

	Sql_EscapeStringLen(sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
	p->hungry = cap_value(p->hungry, 0, 100);
	p->intimate = cap_value(p->intimate, 0, 1000);

	if( pet_id == -1 )
	{// New pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` "
			"(`class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`) "
			"VALUES ('%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
		p->pet_id = (int)Sql_LastInsertId(sql_handle);
	}
	else
	{// Update pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%d'",
			pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate, p->pet_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
	}

	if (save_log)
		ShowInfo("Pet saved %d - %s.\n", pet_id, p->name);
	return 1;
*/
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
	Sql* sql_handle = db->pets;

	// decide on the pet id to assign
	int pet_id;
	if( pd->pet_id != -1 )
	{// caller specifies it manually
		pet_id = pd->pet_id;
	}
	else
	{// ask the database
		char* data;
		size_t len;

		if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT MAX(`pet_id`)+1 FROM `%s`", db->pet_db) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
		{
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			return false;
		}

		Sql_GetData(sql_handle, 0, &data, &len);
		pet_id = ( data != NULL ) ? atoi(data) : 0;
		Sql_FreeResult(sql_handle);
	}

	// zero value is prohibited
	if( pet_id == 0 )
		return false;

	// insert the data into the database
	pd->pet_id = pet_id;
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
	return mmo_pet_tosql(db, pd, false);
}

static bool pet_db_sql_load_num(PetDB* self, struct s_pet* pd, int pet_id)
{
	PetDB_SQL* db = (PetDB_SQL*)self;
	return mmo_pet_fromsql(db, pd, pet_id);
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
	db->vtable.load_num  = &pet_db_sql_load_num;

	// initialize to default values
	db->owner = owner;
	db->pets = NULL;

	// other settings
	db->case_sensitive = false;
	db->pet_db = db->owner->table_pets;

	return &db->vtable;
}
