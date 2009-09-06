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
#include "petdb.h"
#include <stdio.h>
#include <string.h>


#define START_PET_NUM 1


/// Internal structure.
/// @private
typedef struct PetDB_TXT
{
	// public interface
	PetDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* pets;
	int next_pet_id;
	bool dirty;

	// settings
	const char* pet_db;

} PetDB_TXT;


/// @private
static bool mmo_pet_fromstr(struct s_pet* p, char* str)
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

	return true;
}


/// @private
static bool mmo_pet_tostr(const struct s_pet* p, char* str)
{
	sprintf(str, "%d,%d,%s\t%d,%d,%d,%d,%d,%d,%d,%d,%d",
		p->pet_id, p->class_, p->name, p->account_id, p->char_id, p->level, p->egg_id,
		p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate);

	return true;
}


/// @protected
static bool pet_db_txt_init(PetDB* self)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets;

	char line[8192];
	FILE *fp;

	// create pet database
	if( db->pets == NULL )
		db->pets = idb_alloc(DB_OPT_RELEASE_DATA);
	pets = db->pets;
	db_clear(pets);

	// open data file
	fp = fopen(db->pet_db, "r");
	if( fp == NULL )
	{
		ShowError("Pet file not found: %s.\n", db->pet_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int pet_id, n;
		struct s_pet p;
		struct s_pet* tmp;

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &pet_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( pet_id > db->next_pet_id )
				db->next_pet_id = pet_id;
			continue;
		}

		if( !mmo_pet_fromstr(&p, line) )
		{
			ShowError("pet_db_txt_init: skipping invalid data: %s", line);
			continue;
		}
	
		// record entry in db
		tmp = (struct s_pet*)aMalloc(sizeof(struct s_pet));
		memcpy(tmp, &p, sizeof(struct s_pet));
		idb_put(pets, p.pet_id, tmp);

		if( p.pet_id >= db->next_pet_id )
			db->next_pet_id = p.pet_id + 1;
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void pet_db_txt_destroy(PetDB* self)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets = db->pets;

	// delete pet database
	if( pets != NULL )
	{
		db_destroy(pets);
		db->pets = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool pet_db_txt_sync(PetDB* self, bool force)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->pet_db, &lock);
	if( fp == NULL )
	{
		ShowError("pet_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->pet_db);
		return false;
	}

	iter = db->pets->iterator(db->pets);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct s_pet* pd = (struct s_pet*) data;
		char line[8192];

		mmo_pet_tostr(pd, line);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_pet_id);
	iter->destroy(iter);

	lock_fclose(fp, db->pet_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool pet_db_txt_create(PetDB* self, struct s_pet* pd)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets = db->pets;
	struct s_pet* tmp;

	// decide on the pet id to assign
	int pet_id = ( pd->pet_id != -1 ) ? pd->pet_id : db->next_pet_id;

	// check if the pet id is free
	tmp = idb_get(pets, pet_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("pet_db_txt_create: cannot create pet %d:'%s', this id is already occupied by %d:'%s'!\n", pet_id, pd->name, pet_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct s_pet, 1);
	memcpy(tmp, pd, sizeof(struct s_pet));
	tmp->pet_id = pet_id;
	idb_put(pets, pet_id, tmp);

	// increment the auto_increment value
	if( pet_id >= db->next_pet_id )
		db->next_pet_id = pet_id + 1;

	// write output
	pd->pet_id = pet_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool pet_db_txt_remove(PetDB* self, const int pet_id)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets = db->pets;

	idb_remove(pets, pet_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool pet_db_txt_save(PetDB* self, const struct s_pet* pd)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets = db->pets;
	int pet_id = pd->pet_id;

	// retrieve previous data
	struct s_pet* tmp = idb_get(pets, pet_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, pd, sizeof(struct s_pet));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool pet_db_txt_load(PetDB* self, struct s_pet* pd, int pet_id)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	DBMap* pets = db->pets;

	// retrieve data
	struct s_pet* tmp = idb_get(pets, pet_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(pd, tmp, sizeof(struct s_pet));

	return true;
}


/// Returns an iterator over all pets.
/// @protected
static CSDBIterator* pet_db_txt_iterator(PetDB* self)
{
	PetDB_TXT* db = (PetDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->pets));
}


/// Constructs a new PetDB interface.
/// @protected
PetDB* pet_db_txt(CharServerDB_TXT* owner)
{
	PetDB_TXT* db = (PetDB_TXT*)aCalloc(1, sizeof(PetDB_TXT));

	// set up the vtable
	db->vtable.p.init      = &pet_db_txt_init;
	db->vtable.p.destroy   = &pet_db_txt_destroy;
	db->vtable.p.sync      = &pet_db_txt_sync;
	db->vtable.create    = &pet_db_txt_create;
	db->vtable.remove    = &pet_db_txt_remove;
	db->vtable.save      = &pet_db_txt_save;
	db->vtable.load      = &pet_db_txt_load;
	db->vtable.iterator  = &pet_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->pets = NULL;
	db->next_pet_id = START_PET_NUM;
	db->dirty = false;

	// other settings
	db->pet_db = db->owner->file_pets;

	return &db->vtable;
}
