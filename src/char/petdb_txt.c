// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "petdb.h"
#include <stdio.h>
#include <string.h>


/// global defines
#define PETDB_TXT_DB_VERSION 00000000
#define START_PET_NUM 1


/// Internal structure.
/// @private
typedef struct PetDB_TXT
{
	// public interface
	PetDB vtable;

	// data provider
	CSDB_TXT* db;

} PetDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool pet_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct s_pet* p = (struct s_pet*)data;

	*out_size = sizeof(*p);

	if( size < sizeof(*p) )
		return true;

	if( version == 00000000 )
	{
		int tmp_int[16];
		char tmp_str[256];

		memset(p, 0, sizeof(p));

		if( sscanf(str, "%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&tmp_int[0], &tmp_int[1], tmp_str, &tmp_int[2],
			&tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10]) != 12 )
			return false;

		p->pet_id = tmp_int[0];
		p->class_ = tmp_int[1];
		memcpy(p->name,tmp_str,NAME_LENGTH);
		p->account_id = tmp_int[2];
		p->char_id = tmp_int[3];
		p->level = tmp_int[4];
		p->egg_id = tmp_int[5];
		p->equip = tmp_int[6];
		p->intimate = tmp_int[7];
		p->hungry = tmp_int[8];
		p->rename_flag = tmp_int[9];
		p->incuvate = tmp_int[10];

		p->hungry = cap_value(p->hungry, 0, 100);
		p->intimate = cap_value(p->intimate, 0, 1000);

		*key = p->pet_id;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool pet_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	const struct s_pet* p = (const struct s_pet*)data;

	sprintf(str, "%d,%d,%s\t%d,%d,%d,%d,%d,%d,%d,%d,%d",
		p->pet_id, p->class_, p->name, p->account_id, p->char_id, p->level, p->egg_id,
		p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate);

	return true;
}


/// @protected
static bool pet_db_txt_init(PetDB* self)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void pet_db_txt_destroy(PetDB* self)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool pet_db_txt_sync(PetDB* self, bool force)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool pet_db_txt_create(PetDB* self, struct s_pet* pd)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;

	if( pd->pet_id == -1 )
		pd->pet_id = db->next_key(db);

	return db->insert(db, pd->pet_id, pd, sizeof(*pd));
}


/// @protected
static bool pet_db_txt_remove(PetDB* self, const int pet_id)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return db->remove(db, pet_id);
}


/// @protected
static bool pet_db_txt_save(PetDB* self, const struct s_pet* pd)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return db->update(db, pd->pet_id, pd, sizeof(*pd));
}


/// @protected
static bool pet_db_txt_load(PetDB* self, struct s_pet* pd, int pet_id)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return db->load(db, pet_id, pd, sizeof(*pd), NULL);
}


/// Returns an iterator over all pets.
/// @protected
static CSDBIterator* pet_db_txt_iterator(PetDB* self)
{
	CSDB_TXT* db = ((PetDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new PetDB interface.
/// @protected
PetDB* pet_db_txt(CharServerDB_TXT* owner)
{
	PetDB_TXT* db = (PetDB_TXT*)aCalloc(1, sizeof(PetDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_pets, PETDB_TXT_DB_VERSION, START_PET_NUM);
	db->db->p.fromstr = &pet_db_txt_fromstr;
	db->db->p.tostr   = &pet_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &pet_db_txt_init;
	db->vtable.p.destroy = &pet_db_txt_destroy;
	db->vtable.p.sync    = &pet_db_txt_sync;
	db->vtable.create    = &pet_db_txt_create;
	db->vtable.remove    = &pet_db_txt_remove;
	db->vtable.save      = &pet_db_txt_save;
	db->vtable.load      = &pet_db_txt_load;
	db->vtable.iterator  = &pet_db_txt_iterator;

	return &db->vtable;
}
