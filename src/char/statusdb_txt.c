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
#include "statusdb.h"
#include <stdio.h>
#include <string.h>


/// internal structure
typedef struct StatusDB_TXT
{
	StatusDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* statuses;      // in-memory status storage

	const char* status_db;// status data storage file

} StatusDB_TXT;



static void* create_scdata(DBKey key, va_list args)
{
	struct scdata* sc = (struct scdata*)aMalloc(sizeof(struct scdata));
	sc->account_id = va_arg(args, int);
	sc->char_id = key.i;
	sc->count = 0;
	sc->data = NULL;
	return sc;
}


static int scdata_db_final(DBKey key, void* data, va_list ap)
{
	struct scdata* sc = (struct scdata*)data;
	if (sc->data)
		aFree(sc->data);
	aFree(sc);
	return 0;
}


static bool mmo_status_fromstr(struct scdata* sc, char* str)
{
	int i, len, next;
	
	if( sscanf(str, "%d,%d,%d\t%n", &sc->account_id, &sc->char_id, &sc->count, &next) < 3 )
		return false;

	sc->data = (struct status_change_data*)aCalloc(sc->count, sizeof (struct status_change_data));

	for( i = 0; i < sc->count; i++ )
	{
		if (sscanf(str + next, "%hu,%d,%d,%d,%d,%d\t%n", &sc->data[i].type, &sc->data[i].tick,
			&sc->data[i].val1, &sc->data[i].val2, &sc->data[i].val3, &sc->data[i].val4, &len) < 6)
		{
			aFree(sc->data);
			return false;
		}
		next += len;
	}

	return true;
}


static bool mmo_status_tostr(const struct scdata* sc, char* str)
{
	int i, len;

	len = sprintf(str, "%d,%d,%d\t", sc->account_id, sc->char_id, sc->count);
	for( i = 0; i < sc->count; i++ )
	{
		len += sprintf(str + len, "%d,%d,%d,%d,%d,%d\t", sc->data[i].type, sc->data[i].tick,
			sc->data[i].val1, sc->data[i].val2, sc->data[i].val3, sc->data[i].val4);
	}

	return true;
}


static bool mmo_status_sync(StatusDB_TXT* db)
{
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->status_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_status_sync: can't write [%s] !!! data is lost !!!\n", db->status_db);
		return false;
	}

	iter = db->statuses->iterator(db->statuses);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct scdata* sc = (struct scdata*) data;
		char line[8192];

		if( sc->count == 0 )
			continue;

		mmo_status_tostr(sc, line);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->status_db, &lock);

	return true;
}


static bool status_db_txt_init(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses;

	char line[8192];
	FILE *fp;

	// create pet database
	db->statuses = idb_alloc(DB_OPT_BASE);
	statuses = db->statuses;

	// open data file
	fp = fopen(db->status_db, "r");
	if( fp == NULL )
	{
		ShowError("status_db_txt_init: Cannot open file %s!\n", db->status_db);
		return false;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		struct scdata* sc = (struct scdata*)aCalloc(1, sizeof(struct scdata));

		if( !mmo_status_fromstr(sc, line) )
		{
			ShowError("status_db_txt_init: Broken line data: %s\n", line);
			aFree(sc);
			continue;
		}

		sc = (struct scdata*)idb_put(statuses, sc->char_id, sc);
		if( sc != NULL )
		{
			ShowError("Duplicate entry in %s for character %d\n", db->status_db, sc->char_id);
			if (sc->data) aFree(sc->data);
			aFree(sc);
		}
	}

	fclose(fp);

	return true;
}

static void status_db_txt_destroy(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;

	// write data
	mmo_status_sync(db);

	// delete status database
	statuses->destroy(statuses, scdata_db_final);
	db->statuses = NULL;

	// delete entire structure
	aFree(db);
}

static bool status_db_txt_sync(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	return mmo_status_sync(db);
}

static bool status_db_txt_remove(StatusDB* self, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;

	struct scdata* tmp = (struct scdata*)idb_remove(statuses, char_id);
	if( tmp != NULL )
	{// deallocation
		aFree(tmp->data);
		aFree(tmp);
	}

	return true;
}

static bool status_db_txt_save(StatusDB* self, struct scdata* sc)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;
	struct scdata* tmp;

	if( sc->count > 0 )
	{	// retrieve previous data / allocate new data
		tmp = (struct scdata*)idb_ensure(statuses, sc->char_id, create_scdata, sc->account_id);

		// overwrite with new data
		tmp->account_id = sc->account_id;
		tmp->char_id = sc->char_id;
		tmp->count = sc->count;
		memcpy(tmp->data, sc->data, sc->count * sizeof(struct status_change_data));
	}
	else
	{
		tmp = (struct scdata*)idb_remove(statuses, sc->char_id);
		if( tmp != NULL )
		{
			aFree(tmp->data);
			aFree(tmp);
		}
	}


	return true;
}

static bool status_db_txt_load(StatusDB* self, struct scdata* sc, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;
	struct scdata* tmp;

	tmp = (struct scdata*)idb_get(statuses, char_id);
	if( tmp != NULL )
	{
		//sc->account_id = tmp->account_id;
		sc->char_id = tmp->char_id;
		sc->count = tmp->count;
		sc->data = (struct status_change_data*)aMalloc(sc->count * sizeof(struct status_change_data));
		memcpy(sc->data, tmp->data, sc->count * sizeof(struct status_change_data));
	}
	else
	{
		//sc->account_id = account_id;
		sc->char_id = char_id;
		sc->count = 0;
		sc->data = NULL;
	}

	return true;
}


/// Returns an iterator over all status entries.
static CSDBIterator* status_db_txt_iterator(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->statuses));
}


/// public constructor
StatusDB* status_db_txt(CharServerDB_TXT* owner)
{
	StatusDB_TXT* db = (StatusDB_TXT*)aCalloc(1, sizeof(StatusDB_TXT));

	// set up the vtable
	db->vtable.init      = &status_db_txt_init;
	db->vtable.destroy   = &status_db_txt_destroy;
	db->vtable.sync      = &status_db_txt_sync;
	db->vtable.remove    = &status_db_txt_remove;
	db->vtable.save      = &status_db_txt_save;
	db->vtable.load      = &status_db_txt_load;
	db->vtable.iterator  = &status_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->statuses = NULL;

	// other settings
	db->status_db = db->owner->file_statuses;

	return &db->vtable;
}
