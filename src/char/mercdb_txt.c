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
#include "mercdb.h"
#include <stdio.h>
#include <string.h>


/// global defines
#define MERC_TXT_DB_VERSION 20090805
#define START_MERCENARY_NUM 1


/// internal structure
typedef struct MercDB_TXT
{
	MercDB vtable;       // public interface

	CharServerDB_TXT* owner;
	DBMap* mercs;         // in-memory mercenary storage
	int next_merc_id;     // auto_increment
	bool dirty;

	const char* merc_db;  // mercenary data storage file

} MercDB_TXT;



static bool mmo_merc_fromstr(struct s_mercenary* md, char* str, unsigned int version)
{
	// zero out the destination first
	memset(md, 0x00, sizeof(*md));

	if( version == 20090805 )
	{
		if( sscanf(str, "%d\t%d,%hd,%d,%d,%u,%u", &md->mercenary_id, &md->char_id, &md->class_, &md->hp, &md->sp, &md->kill_count, &md->life_time) != 7 )
			return false;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


static bool mmo_merc_tostr(const struct s_mercenary* md, char* str)
{
	char* p = str;

	p += sprintf(p, "%d\t%d,%hd,%d,%d,%u,%u", md->mercenary_id, md->char_id, md->class_, md->hp, md->sp, md->kill_count, md->life_time);

	return true;
}


static bool merc_db_txt_init(MercDB* self)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create mercenary database
	if( db->mercs == NULL )
		db->mercs = idb_alloc(DB_OPT_RELEASE_DATA);
	mercs = db->mercs;
	db_clear(mercs);

	// open data file
	fp = fopen(db->merc_db, "r");
	if( fp == NULL )
	{
		ShowError("Mercenary file not found: %s.\n", db->merc_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int merc_id, n;
		unsigned int v;
		struct s_mercenary p;
		struct s_mercenary* tmp;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &merc_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( merc_id > db->next_merc_id )
				db->next_merc_id = merc_id;
			continue;
		}

		if( !mmo_merc_fromstr(&p, line, version) )
		{
			ShowError("merc_db_txt_init: skipping invalid data: %s", line);
			continue;
		}
	
		// record entry in db
		tmp = (struct s_mercenary*)aMalloc(sizeof(struct s_mercenary));
		memcpy(tmp, &p, sizeof(struct s_mercenary));
		idb_put(mercs, p.mercenary_id, tmp);

		if( p.mercenary_id >= db->next_merc_id )
			db->next_merc_id = p.mercenary_id + 1;
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}

static void merc_db_txt_destroy(MercDB* self)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs = db->mercs;

	// delete mercenary database
	if( mercs != NULL )
	{
		db_destroy(mercs);
		db->mercs = NULL;
	}

	// delete entire structure
	aFree(db);
}

static bool merc_db_txt_sync(MercDB* self)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBIterator* iter;
	void* data;
	FILE* fp;
	int lock;

	fp = lock_fopen(db->merc_db, &lock);
	if( fp == NULL )
	{
		ShowError("merc_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->merc_db);
		return false;
	}

	fprintf(fp, "%d\n", MERC_TXT_DB_VERSION); // savefile version

	iter = db->mercs->iterator(db->mercs);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct s_mercenary* md = (struct s_mercenary*) data;
		char line[8192];

		mmo_merc_tostr(md, line);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_merc_id);
	iter->destroy(iter);

	lock_fclose(fp, db->merc_db, &lock);

	db->dirty = false;
	return true;
}

static bool merc_db_txt_create(MercDB* self, struct s_mercenary* md)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs = db->mercs;
	struct s_mercenary* tmp;

	// decide on the mercenary id to assign
	int merc_id = ( md->mercenary_id != -1 ) ? md->mercenary_id : db->next_merc_id;

	// check if the mercenary id is free
	tmp = idb_get(mercs, merc_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("merc_db_txt_create: cannot create mercenary %d (%d), this id is already occupied by %d (%d)!\n", merc_id, md->char_id, merc_id, tmp->char_id);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct s_mercenary, 1);
	memcpy(tmp, md, sizeof(struct s_mercenary));
	tmp->mercenary_id = merc_id;
	idb_put(mercs, merc_id, tmp);

	// increment the auto_increment value
	if( merc_id >= db->next_merc_id )
		db->next_merc_id = merc_id + 1;

	// write output
	md->mercenary_id = merc_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool merc_db_txt_remove(MercDB* self, const int merc_id)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs = db->mercs;

	idb_remove(mercs, merc_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool merc_db_txt_save(MercDB* self, const struct s_mercenary* md)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs = db->mercs;
	int merc_id = md->mercenary_id;

	// retrieve previous data
	struct s_mercenary* tmp = idb_get(mercs, merc_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, md, sizeof(struct s_mercenary));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool merc_db_txt_load(MercDB* self, struct s_mercenary* md, int merc_id)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	DBMap* mercs = db->mercs;

	// retrieve data
	struct s_mercenary* tmp = idb_get(mercs, merc_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(md, tmp, sizeof(struct s_mercenary));

	return true;
}


/// Returns an iterator over all mercs.
static CSDBIterator* merc_db_txt_iterator(MercDB* self)
{
	MercDB_TXT* db = (MercDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->mercs));
}


/// public constructor
MercDB* merc_db_txt(CharServerDB_TXT* owner)
{
	MercDB_TXT* db = (MercDB_TXT*)aCalloc(1, sizeof(MercDB_TXT));

	// set up the vtable
	db->vtable.init      = &merc_db_txt_init;
	db->vtable.destroy   = &merc_db_txt_destroy;
	db->vtable.sync      = &merc_db_txt_sync;
	db->vtable.create    = &merc_db_txt_create;
	db->vtable.remove    = &merc_db_txt_remove;
	db->vtable.save      = &merc_db_txt_save;
	db->vtable.load      = &merc_db_txt_load;
	db->vtable.iterator  = &merc_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->mercs = NULL;
	db->next_merc_id = START_MERCENARY_NUM;
	db->dirty = false;

	// other settings
	db->merc_db = db->owner->file_mercenaries;

	return &db->vtable;
}
