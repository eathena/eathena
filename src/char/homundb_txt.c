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
#include "homundb.h"
#include <stdio.h>
#include <string.h>

#define START_HOMUN_NUM 1

/// internal structure
typedef struct HomunDB_TXT
{
	HomunDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* homuns;       // in-memory homun storage
	int next_homun_id;   // auto_increment
	bool dirty;

	const char* homun_db; // homun data storage file

} HomunDB_TXT;



static bool mmo_homun_fromstr(struct s_homunculus* hd, char* str)
{
	int next, len;

	memset(hd, 0, sizeof(struct s_homunculus));

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

	return true;
}


static bool mmo_homun_tostr(const struct s_homunculus* hd, char* str)
{
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


static bool homun_db_txt_init(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns;
	char line[8192];
	FILE* fp;

	// create chars database
	if( db->homuns == NULL )
		db->homuns = idb_alloc(DB_OPT_RELEASE_DATA);
	homuns = db->homuns;
	db_clear(homuns);

	// open data file
	fp = fopen(db->homun_db, "r");
	if( fp == NULL )
	{
		ShowError("Homun file not found: %s.\n", db->homun_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int homun_id, n;
		struct s_homunculus* hd;

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &homun_id, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( homun_id > db->next_homun_id )
				db->next_homun_id = homun_id;
			continue;
		}

		hd = (struct s_homunculus*)aCalloc(sizeof(struct s_homunculus), 1);

		if( !mmo_homun_fromstr(hd, line) )
		{
			ShowError("homun_db_txt_init: skipping invalid data: %s", line);
			aFree(hd);
			continue;
		}

		// record entry in db
		idb_put(homuns, hd->hom_id, hd);

		if( hd->hom_id >= db->next_homun_id )
			db->next_homun_id = hd->hom_id + 1;
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}

static void homun_db_txt_destroy(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;

	// delete homun database
	if( homuns != NULL )
	{
		db_destroy(homuns);
		db->homuns = NULL;
	}

	// delete entire structure
	aFree(db);
}

static bool homun_db_txt_sync(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->homun_db, &lock);
	if( fp == NULL )
	{
		ShowError("homun_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->homun_db);
		return false;
	}

	iter = db->homuns->iterator(db->homuns);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct s_homunculus* hd = (struct s_homunculus*) data;
		char line[8192];

		mmo_homun_tostr(hd, line);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_homun_id);
	iter->destroy(iter);

	lock_fclose(fp, db->homun_db, &lock);

	db->dirty = false;
	return true;
}

static bool homun_db_txt_create(HomunDB* self, struct s_homunculus* hd)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;
	struct s_homunculus* tmp;

	// decide on the homun id to assign
	int homun_id = ( hd->hom_id != -1 ) ? hd->hom_id : db->next_homun_id;

	// check if the homun is free
	tmp = idb_get(homuns, homun_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("homun_db_txt_create: cannot create homunculus %d:'%s', this id is already occupied by %d:'%s'!\n", homun_id, hd->name, homun_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct s_homunculus, 1);
	memcpy(tmp, hd, sizeof(struct s_homunculus));
	tmp->hom_id = homun_id;
	idb_put(homuns, homun_id, tmp);

	// increment the auto_increment value
	if( homun_id >= db->next_homun_id )
		db->next_homun_id = homun_id + 1;

	// write output
	hd->hom_id = homun_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool homun_db_txt_remove(HomunDB* self, int homun_id)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;

	idb_remove(homuns, homun_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool homun_db_txt_save(HomunDB* self, const struct s_homunculus* hd)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;
	int homun_id = hd->hom_id;

	// retrieve previous data
	struct s_homunculus* tmp = idb_get(homuns, homun_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, hd, sizeof(struct s_homunculus));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool homun_db_txt_load(HomunDB* self, struct s_homunculus* hd, int homun_id)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;

	// retrieve data
	struct s_homunculus* tmp = idb_get(homuns, homun_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(hd, tmp, sizeof(struct s_homunculus));

	return true;
}


/// Returns an iterator over all homunculi.
static CSDBIterator* homun_db_txt_iterator(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->homuns));
}


/// public constructor
HomunDB* homun_db_txt(CharServerDB_TXT* owner)
{
	HomunDB_TXT* db = (HomunDB_TXT*)aCalloc(1, sizeof(HomunDB_TXT));

	// set up the vtable
	db->vtable.init      = &homun_db_txt_init;
	db->vtable.destroy   = &homun_db_txt_destroy;
	db->vtable.sync      = &homun_db_txt_sync;
	db->vtable.create    = &homun_db_txt_create;
	db->vtable.remove    = &homun_db_txt_remove;
	db->vtable.save      = &homun_db_txt_save;
	db->vtable.load      = &homun_db_txt_load;
	db->vtable.iterator  = &homun_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->homuns = NULL;
	db->next_homun_id = START_HOMUN_NUM;
	db->dirty = false;

	// other settings
	db->homun_db = db->owner->file_homuns;

	return &db->vtable;
}
