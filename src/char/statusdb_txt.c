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
#include <limits.h> // SIZE_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define STATUSDB_TXT_DB_VERSION 20090907


/// Internal structure.
/// @private
typedef struct StatusDB_TXT
{
	// public interface
	StatusDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* statuses;
	bool dirty;

	// settings
	const char* status_db;

} StatusDB_TXT;


/// @private
static bool mmo_status_fromstr(struct status_change_data* sc, size_t size, char* str, unsigned int version)
{
	if( version == 20090907 )
	{
		const char* p = str;
		int n;
		size_t i;
		
		for( i = 0; i < size && *p != '\0' && *p != '\r' && *p != '\n'; i++ )
		{
			if (sscanf(p, " %hu,%d,%d,%d,%d,%d%n", &sc[i].type, &sc[i].tick, &sc[i].val1, &sc[i].val2, &sc[i].val3, &sc[i].val4, &n) < 6)
				return false;

			p += n;
		}

		for( i = i; i < size; ++i )
		{
			memset(sc, 0, sizeof(*sc));
			sc[i].type = -1;
		}
	}
	else
	if( version == 00000000 )
	{
		const char* p = str;
		int n;
		size_t i;
		
		for( i = 0; i < size && *p != '\0' && *p != '\r' && *p != '\n'; i++ )
		{
			if (sscanf(p, "%hu,%d,%d,%d,%d,%d\t%n", &sc[i].type, &sc[i].tick, &sc[i].val1, &sc[i].val2, &sc[i].val3, &sc[i].val4, &n) < 6)
				return false;

			p += n;
		}

		for( i = i; i < size; ++i )
		{
			memset(sc, 0, sizeof(*sc));
			sc[i].type = -1;
		}
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// @private
static bool mmo_status_tostr(const struct status_change_data* sc, size_t size, char* str)
{
	char* p = str;
	bool first = true;
	size_t i;

	for( i = 0; i < size; i++ )
	{
		if( sc[i].type == (unsigned short)-1 )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, " ");

		p += sprintf(p, "%d,%d,%d,%d,%d,%d", sc[i].type, sc[i].tick, sc[i].val1, sc[i].val2, sc[i].val3, sc[i].val4);
	}

	return true;
}


/// @protected
static bool status_db_txt_init(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create pet database
	if( db->statuses == NULL )
		db->statuses = idb_alloc(DB_OPT_RELEASE_DATA);
	statuses = db->statuses;
	statuses->clear(statuses, NULL);

	// open data file
	fp = fopen(db->status_db, "r");
	if( fp == NULL )
	{
		ShowError("status_db_txt_init: Cannot open file %s!\n", db->status_db);
		return false;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		int char_id, n;
		unsigned int v;
		struct status_change_data* sc;
		int count;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		// load char id and determine size
		n = 0;
		if( version == 20090907 && sscanf(line, "%d%n", &char_id, &n) == 1 && line[n] == '\t' )
			count = sv_parse(line, strlen(line), n + 1, ' ', NULL, 0, (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF));
		else
		if( version == 00000000 && sscanf(line, "%*d,%d,%*d%n", &char_id, &n) == 1 && line[n] == '\t' )
			count = sv_parse(line, strlen(line), n + 1, '\t', NULL, 0, (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF)) - 1;
		else
		{
			ShowError("status_db_txt_init: File %s, broken line data: %s\n", db->status_db, line);
			continue;
		}

		// allocate space
		sc = (struct status_change_data*)aMalloc(count * sizeof(struct status_change_data));
		if( sc == NULL )
		{
			ShowFatalError("status_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load data
		if( !mmo_status_fromstr(sc, count, line + n + 1, version) )
		{
			ShowError("status_db_txt_init: Broken line data: %s\n", line);
			aFree(sc);
			continue;
		}

		db->vtable.save(&db->vtable, sc, count, char_id);
		aFree(sc);
	}

	fclose(fp);

	return true;
}


/// @protected
static void status_db_txt_destroy(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;

	// delete status database
	if( statuses != NULL )
	{
		statuses->destroy(statuses, NULL);
		db->statuses = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool status_db_txt_sync(StatusDB* self, bool force)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->status_db, &lock);
	if( fp == NULL )
	{
		ShowError("status_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->status_db);
		return false;
	}

	fprintf(fp, "%d\n", STATUSDB_TXT_DB_VERSION);

	iter = db->statuses->iterator(db->statuses);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		struct status_change_data* sc = (struct status_change_data*) data;
		char line[8192];
		size_t size;

		ARR_FIND(0, SIZE_MAX, size, sc[size].type == (unsigned short)-1); // determine size
		mmo_status_tostr(sc, size, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->status_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool status_db_txt_remove(StatusDB* self, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;

	idb_remove(statuses, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool status_db_txt_save(StatusDB* self, const struct status_change_data* sc, size_t size, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;
	struct status_change_data* tmp;
	size_t i, k;

	if( sc == NULL )
		size = 0;

	// remove old status list
	idb_remove(statuses, char_id);

	// fill list (compact array + append zero terminator)
	tmp = (struct status_change_data*)aMalloc((size+1)*sizeof(*sc));
	k = 0;
	for( i = 0; i < size; ++i )
	{
		if( sc[i].type == (unsigned short)-1 )
			continue;
		memcpy(&tmp[k], &sc[i], sizeof(*sc));
		++k;
	}

	if( k > 0 )
	{// has entries
		if( k != size )
			tmp = (struct status_change_data*)aRealloc(tmp, (k+1)*sizeof(*sc));
		memset(tmp+k, 0x00, sizeof(*sc));
		tmp[k].type = -1; // terminate array
		idb_put(statuses, char_id, tmp);
	}
	else
	{// no data to store
		aFree(tmp);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool status_db_txt_load(StatusDB* self, struct status_change_data* sc, size_t size, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	DBMap* statuses = db->statuses;
	struct status_change_data* tmp;
	size_t i = 0;

	tmp = (struct status_change_data*)idb_get(statuses, char_id);

	if( tmp != NULL )
		ARR_FIND(0, size, i, tmp[i].type == (unsigned short)-1);
	if( i > 0 )
		memcpy(sc, tmp, i*sizeof(*sc));
	if( i != size )
	{
		size_t j;
		for( j = i; j < size; ++j )
		{
			memset(sc+j, 0x00, sizeof(*sc));
			sc[j].type = -1;
		}
	}

	return true;
}


/// @protected
static size_t status_db_txt_size(StatusDB* self, int char_id)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	struct status_change_data* tmp;
	size_t result;
	size_t i = 0;

	tmp = (struct status_change_data*)idb_get(db->statuses, char_id);
	if( tmp != NULL )
		ARR_FIND(0, SIZE_MAX, i, tmp[i].type == (unsigned short)-1);
	result = i;

	return result;
}


/// Returns an iterator over all status entries.
/// @protected
static CSDBIterator* status_db_txt_iterator(StatusDB* self)
{
	StatusDB_TXT* db = (StatusDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->statuses));
}


/// Constructs a new StatusDB interface.
/// @protected
StatusDB* status_db_txt(CharServerDB_TXT* owner)
{
	StatusDB_TXT* db = (StatusDB_TXT*)aCalloc(1, sizeof(StatusDB_TXT));

	// set up the vtable
	db->vtable.p.init    = &status_db_txt_init;
	db->vtable.p.destroy = &status_db_txt_destroy;
	db->vtable.p.sync    = &status_db_txt_sync;
	db->vtable.remove    = &status_db_txt_remove;
	db->vtable.save      = &status_db_txt_save;
	db->vtable.load      = &status_db_txt_load;
	db->vtable.size      = &status_db_txt_size;
	db->vtable.iterator  = &status_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->statuses = NULL;
	db->dirty = false;

	// other settings
	db->status_db = db->owner->file_statuses;

	return &db->vtable;
}
