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
#include "storagedb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct StorageDB_TXT
{
	StorageDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* storages;       // in-memory storage storage

	const char* storage_db;// storage data storage file

} StorageDB_TXT;



static bool mmo_storage_fromstr(struct storage_data* s, const char* str)
{
	int amount;
	int count;
	int fields[1+MAX_STORAGE][2];
	int tmp_int[7+MAX_SLOTS+1];
	int len,i,j;

	// parse amount (currently ignored)
	if( sscanf(str, "%d\t%n", &amount, &len) != 1 )
		return false;

	// extract space-separated item blocks from str
	count = sv_parse(str, strlen(str), len, ' ', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF)) - 1;

	// parse individual item blocks
	for( i = 0; i < count; ++i )
	{
		const char* p = &str[fields[i+1][0]];

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&len) != 7 )
			return false;

		p += len;

		j = 0;
		while( *p != ' ' || *p != '\0' )
		{
			if( sscanf(p, ",%d%n", &tmp_int[7+j], &len) != 1 )
				return false;

			p += len;

			if( j == MAX_SLOTS )
				continue; // discard card slots over max

			j++;
		}

		if( i == MAX_STORAGE )
			continue; // discard items over max

		s->items[i].id = tmp_int[0];
		s->items[i].nameid = tmp_int[1];
		s->items[i].amount = tmp_int[2];
		s->items[i].equip = tmp_int[3];
		s->items[i].identify = tmp_int[4];
		s->items[i].refine = tmp_int[5];
		s->items[i].attribute = tmp_int[6];
		for( j = 0; j < MAX_SLOTS; ++j )
			s->items[i].card[j] = tmp_int[7+j];
	}

	s->storage_amount = amount;

	return true;
}


static bool mmo_storage_tostr(const struct storage_data* s, char* str)
{
	int i,j;
	char* p = str;

	p += sprintf(p, "%d\t", s->storage_amount);

	for( i = 0; i < MAX_STORAGE; i++ )
		if( s->items[i].nameid > 0 && s->items[i].amount > 0 )
		{
			p += sprintf(p, "%d,%d,%d,%d,%d,%d,%d",
				s->items[i].id, s->items[i].nameid, s->items[i].amount, s->items[i].equip,
				s->items[i].identify, s->items[i].refine, s->items[i].attribute);

			for( j = 0; j < MAX_SLOTS; j++ )
				p += sprintf(p, ",%d", s->items[i].card[j]);

			p += sprintf(p, " ");
		}

	*(p++) = '\t';

	*p = '\0';

	return true;
}


static bool mmo_storagedb_sync(StorageDB_TXT* db)
{
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE* fp;
	int lock;

	fp = lock_fopen(db->storage_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_storagedb_sync: can't write [%s] !!! data is lost !!!\n", db->storage_db);
		return false;
	}

	iter = db->storages->iterator(db->storages);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int account_id = key.i;
		struct storage_data* s = (struct storage_data*) data;
		char line[65536];

		if( s->storage_amount == 0 )
			continue;

		mmo_storage_tostr(s, line);
		fprintf(fp, "%d,%s\n", account_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->storage_db, &lock);

	return true;
}


static void* create_storage(DBKey key, va_list args)
{
	return (struct storage_data *) aCalloc(1, sizeof(struct storage_data));
}


static bool storage_db_txt_init(StorageDB* self)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	DBMap* storages;

	char line[65536];
	FILE *fp;

	// create pet database
	db->storages = idb_alloc(DB_OPT_RELEASE_DATA);
	storages = db->storages;

	// open data file
	fp = fopen(db->storage_db, "r");
	if( fp == NULL )
	{
		ShowError("storage_db_txt_init: Cannot open file %s!\n", db->storage_db);
		return false;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		int account_id;
		int n;

		struct storage_data* s = (struct storage_data*)aCalloc(1, sizeof(struct storage_data));
		if( s == NULL )
		{
			ShowFatalError("storage_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load account id
		if( sscanf(line, "%d,%n", &account_id, &n) != 1 )
		{
			aFree(s);
			continue;
		}

		// load storage for this account
		if( !mmo_storage_fromstr(s, line + n) )
		{
			ShowError("storage_db_txt_init: Broken line data: %s\n", line);
			aFree(s);
			continue;
		}

		s = (struct storage_data*)idb_put(storages, account_id, s);
		if( s != NULL )
		{
			ShowError("Duplicate entry in %s for account %d\n", db->storage_db, account_id);
			aFree(s);
		}
	}

	fclose(fp);

	return true;
}

static void storage_db_txt_destroy(StorageDB* self)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	DBMap* storages = db->storages;

	// write data
	mmo_storagedb_sync(db);

	// delete storage database
	storages->destroy(storages, NULL);
	db->storages = NULL;

	// delete entire structure
	aFree(db);
}

static bool storage_db_txt_sync(StorageDB* self)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	return mmo_storagedb_sync(db);
}

static bool storage_db_txt_remove(StorageDB* self, const int account_id)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	DBMap* storages = db->storages;

	struct storage_data* tmp = (struct storage_data*)idb_remove(storages, account_id);
	if( tmp == NULL )
	{// error condition - entry not present
		ShowError("storage_db_txt_remove: no entry for account id %d\n", account_id);
		return false;
	}

	return true;
}

/// Writes provided data into storage cache.
/// If data contains 0 items, any existing entry in cache is destroyed instead.
static bool storage_db_txt_save(StorageDB* self, const struct storage_data* s, int account_id)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	DBMap* storages = db->storages;
	struct storage_data* tmp;

	if( s->storage_amount > 0 )
	{
		tmp = (struct storage_data*)idb_ensure(storages, account_id, create_storage);
		memcpy(tmp, s, sizeof(*s));
	}
	else
	{
		idb_remove(storages, account_id);
	}

	return true;
}

/// Loads storage data into the provided data structure.
/// If data doesn't exist, the destination is zeroed instead.
static bool storage_db_txt_load(StorageDB* self, struct storage_data* s, int account_id)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	DBMap* storages = db->storages;
	struct storage_data* tmp;
	
	tmp = (struct storage_data*)idb_get(storages, account_id);

	if( tmp != NULL )
		memcpy(s, tmp, sizeof(*s));
	else
		memset(s, 0x00, sizeof(*s));

	return true;
}


/// public constructor
StorageDB* storage_db_txt(CharServerDB_TXT* owner)
{
	StorageDB_TXT* db = (StorageDB_TXT*)aCalloc(1, sizeof(StorageDB_TXT));

	// set up the vtable
	db->vtable.init      = &storage_db_txt_init;
	db->vtable.destroy   = &storage_db_txt_destroy;
	db->vtable.sync      = &storage_db_txt_sync;
	db->vtable.remove    = &storage_db_txt_remove;
	db->vtable.save      = &storage_db_txt_save;
	db->vtable.load      = &storage_db_txt_load;

	// initialize to default values
	db->owner = owner;
	db->storages = NULL;

	// other settings
	db->storage_db = db->owner->file_storages;

	return &db->vtable;
}
