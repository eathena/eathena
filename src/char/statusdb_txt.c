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
#include "statusdb.h"
#include <stdio.h>
#include <string.h>


/// internal structure
typedef struct StatusDB_TXT
{
	StatusDB vtable;      // public interface

	DBMap* statuses;      // in-memory status storage

	char status_db[1024]; // status data storage file

} StatusDB_TXT;

/// internal functions
static bool status_db_txt_init(StatusDB* self);
static void status_db_txt_destroy(StatusDB* self);
static bool status_db_txt_sync(StatusDB* self);
static bool status_db_txt_create(StatusDB* self, struct scdata* sc);
static bool status_db_txt_remove(StatusDB* self, const int char_id);
static bool status_db_txt_save(StatusDB* self, const struct scdata* sc);
static bool status_db_txt_load_num(StatusDB* self, struct scdata* sc, int char_id);

static bool mmo_status_fromstr(struct scdata* sc, char* str);
static bool mmo_status_tostr(const struct scdata* sc, char* str);
static bool mmo_status_sync(StatusDB_TXT* db);
static int scdata_db_final(DBKey k, void* d, va_list ap);

/// public constructor
StatusDB* status_db_txt(void)
{
	StatusDB_TXT* db = (StatusDB_TXT*)aCalloc(1, sizeof(StatusDB_TXT));

	// set up the vtable
	db->vtable.init      = &status_db_txt_init;
	db->vtable.destroy   = &status_db_txt_destroy;
	db->vtable.sync      = &status_db_txt_sync;
	db->vtable.create    = &status_db_txt_create;
	db->vtable.remove    = &status_db_txt_remove;
	db->vtable.save      = &status_db_txt_save;
	db->vtable.load_num  = &status_db_txt_load_num;

	// initialize to default values
	db->statuses = NULL;
	// other settings
	safestrncpy(db->status_db, "save/scdata.txt", sizeof(db->status_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


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
		return;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		struct scdata* sc = (struct scdata*)aCalloc(1, sizeof(struct scdata));

		if( !inter_scdata_fromstr(line, sc) )
		{
			ShowError("status_db_txt_init: Broken line data: %s\n", line);
			aFree(sc);
			continue;
		}

		sc = (struct scdata*)idb_put(scdata_db, sc->char_id, sc);
		if (sc) {
			ShowError("Duplicate entry in %s for character %d\n", filename, sc->char_id);
			if (sc->data) aFree(sc->data);
			aFree(sc);
		}
	}

	fclose(fp);
}

static void status_db_txt_destroy(StatusDB* self)
{
	scdata_db->destroy(scdata_db, scdata_db_final);
}

static bool status_db_txt_sync(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	return mmo_status_sync(db);
}

static bool status_db_txt_create(StatusDB* self, struct scdata* sc)
{
}

static bool status_db_txt_remove(StatusDB* self, const int char_id)
{
	struct scdata* scdata = (struct scdata*)idb_remove(scdata_db, cid);
	if (scdata)
	{
		if (scdata->data)
			aFree(scdata->data);
		aFree(scdata);
	}
}

static bool status_db_txt_save(StatusDB* self, const struct scdata* sc)
{
	struct scdata *data;
	data = (struct scdata*)aCalloc(1, sizeof(struct scdata));
	data->account_id = va_arg(args, int);
	data->char_id = key.i;
	return data;
}

static bool status_db_txt_load_num(StatusDB* self, struct scdata* sc, int char_id)
{
	return (struct scdata*)scdata_db->ensure(scdata_db, i2key(cid), create_scdata, aid);
}


static bool mmo_status_fromstr(struct scdata* sc, char* str)
{
	int i, len, next;
	
	if( sscanf(line, "%d,%d,%d\t%n", &sc->account_id, &sc->char_id, &sc->count, &next) < 3 )
		return false;

	if( sc->count < 1 )
		return false;
	
	sc->data = (struct status_change_data*)aCalloc(sc->count, sizeof (struct status_change_data));

	for( i = 0; i < sc->count; i++ )
	{
		if (sscanf(line + next, "%hu,%d,%d,%d,%d,%d\t%n", &sc->data[i].type, &sc->data[i].tick,
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

	len = sprintf(line, "%d,%d,%d\t", sc->account_id, sc->char_id, sc->count);
	for(i = 0; i < sc->count; i++)
	{
		len += sprintf(line + len, "%d,%d,%d,%d,%d,%d\t", sc->data[i].type, sc->data[i].tick,
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

	fp = lock_fopen(scdata_txt, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_status_sync: can't write [%s] !!! data is lost !!!\n", scdata_txt);
		return false;
	}

	iter = db->statuses->iterator(db->statuses);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct scdata* sc = (struct scdata*) data;
		char line[8192];

		if( sc->count == 0 )
			continue;

		inter_status_tostr(line, sc);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, scdata_txt, &lock);

	return true;
}

static int scdata_db_final(DBKey k, void* d, va_list ap)
{
	struct scdata *data = (struct scdata*)d;
	if (data->data)
		aFree(data->data);
	aFree(data);
	return 0;
}
