// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "homundb.h"
#include <stdio.h>
#include <string.h>


/// global defines
#define HOMUNDB_TXT_DB_VERSION 00000000
#define START_HOMUN_NUM 1


/// Internal structure.
/// @private
typedef struct HomunDB_TXT
{
	// public interface
	HomunDB vtable;

	// data provider
	CSDB_TXT* db;

} HomunDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool homun_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct s_homunculus* hd = (struct s_homunculus*)data;

	*out_size = sizeof(*hd);

	if( size < sizeof(*hd) )
		return true;

	if( version == 00000000 )
	{
		int next, len;

		memset(hd, 0, sizeof(*hd));

		if( sscanf(str,
			"%d,%hd\t%23[^\t]\t%d,%d,%d,%d,%d,"
			"%u,%hd,%hd,%hd,"
			"%u,%hd,%hd,"
			"%d,%d,%d,%d,%d,%d\t%n",
			&hd->hom_id, &hd->class_, hd->name,
			&hd->char_id, &hd->hp, &hd->max_hp, &hd->sp, &hd->max_sp,
			&hd->intimacy, &hd->hunger, &hd->skillpts, &hd->level,
			&hd->exp, &hd->rename_flag, &hd->vaporize,
			&hd->str, &hd->agi, &hd->vit, &hd->int_, &hd->dex, &hd->luk,
			&next) != 21 )
			return false;

		//Read skills.
		while( str[next] != '\0' && str[next] != '\n' && str[next] != '\r' )
		{
			int id, lv;

			if( sscanf(str+next, "%d,%d,%n", &id, &lv, &len) != 2 )
				return false;

			if( id >= HM_SKILLBASE && id < HM_SKILLBASE+MAX_HOMUNSKILL )
			{
				int i = id - HM_SKILLBASE;
				hd->hskill[i].id = id;
				hd->hskill[i].lv = lv;
			} else
				ShowError("Read Homun: Unsupported Skill ID %d for homunculus (Homun ID=%d)\n", id, hd->hom_id);

			next += len;
			if( str[next] == ' ' )
				next++;
		}

		*key = hd->hom_id;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool homun_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	const struct s_homunculus* hd = (const struct s_homunculus*)data;
	int i;

	str += sprintf(str,
		"%d,%d\t%s\t%d,%d,%d,%d,%d,"
		"%u,%d,%d,%d,"
		"%u,%d,%d,"
		"%d,%d,%d,%d,%d,%d\t",
		hd->hom_id, hd->class_, hd->name,
		hd->char_id, hd->hp, hd->max_hp, hd->sp, hd->max_sp,
	  	hd->intimacy, hd->hunger, hd->skillpts, hd->level,
		hd->exp, hd->rename_flag, hd->vaporize,
		hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk);

	for( i = 0; i < MAX_HOMUNSKILL; i++ )
	{
		if( hd->hskill[i].id && !hd->hskill[i].flag )
			str += sprintf(str, "%d,%d,", hd->hskill[i].id, hd->hskill[i].lv);
	}

	return true;
}


/// @protected
static bool homun_db_txt_init(HomunDB* self)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void homun_db_txt_destroy(HomunDB* self)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool homun_db_txt_sync(HomunDB* self, bool force)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool homun_db_txt_create(HomunDB* self, struct s_homunculus* hd)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;

	if( hd->hom_id == -1 )
		hd->hom_id = db->next_key(db);

	return db->insert(db, hd->hom_id, hd, sizeof(*hd));
}


/// @protected
static bool homun_db_txt_remove(HomunDB* self, int homun_id)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return db->remove(db, homun_id);
}


/// @protected
static bool homun_db_txt_save(HomunDB* self, const struct s_homunculus* hd)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return db->update(db, hd->hom_id, hd, sizeof(*hd));
}


/// @protected
static bool homun_db_txt_load(HomunDB* self, struct s_homunculus* hd, int homun_id)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return db->load(db, homun_id, hd, sizeof(*hd), NULL);
}


/// Returns an iterator over all homunculi.
/// @protected
static CSDBIterator* homun_db_txt_iterator(HomunDB* self)
{
	CSDB_TXT* db = ((HomunDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new HomunDB interface.
/// @protected
HomunDB* homun_db_txt(CharServerDB_TXT* owner)
{
	HomunDB_TXT* db = (HomunDB_TXT*)aCalloc(1, sizeof(HomunDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_homuns, HOMUNDB_TXT_DB_VERSION, START_HOMUN_NUM);
	db->db->p.fromstr = &homun_db_txt_fromstr;
	db->db->p.tostr   = &homun_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &homun_db_txt_init;
	db->vtable.p.destroy = &homun_db_txt_destroy;
	db->vtable.p.sync    = &homun_db_txt_sync;
	db->vtable.create    = &homun_db_txt_create;
	db->vtable.remove    = &homun_db_txt_remove;
	db->vtable.save      = &homun_db_txt_save;
	db->vtable.load      = &homun_db_txt_load;
	db->vtable.iterator  = &homun_db_txt_iterator;

	return &db->vtable;
}
