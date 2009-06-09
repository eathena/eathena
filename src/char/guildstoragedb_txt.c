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
#include "guildstoragedb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct GuildStorageDB_TXT
{
	GuildStorageDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* guildstorages;       // in-memory guild storage storage

	const char* guildstorage_db;// guild storage data storage file

} GuildStorageDB_TXT;



static void* create_guildstorage(DBKey key, va_list args)
{
	return (struct guild_storage *) aCalloc(1, sizeof(struct guild_storage));
}


static bool mmo_guildstorage_fromstr(struct guild_storage* gs, char* str)
{
	int tmp_int[256];
	char tmp_str[256];
	int set,next,len,i,j;

	set = sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
	gs->guild_id=tmp_int[0];
	gs->storage_amount=tmp_int[1];

	if(set!=2)
		return 1;
	if(str[next]=='\n' || str[next]=='\r')
		return 0;
	next++;
	for(i=0;str[next] && str[next]!='\t' && i < MAX_GUILD_STORAGE;i++){
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			&tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &len) == 8)
		{
			gs->storage_[i].id = tmp_int[0];
			gs->storage_[i].nameid = tmp_int[1];
			gs->storage_[i].amount = tmp_int[2];
			gs->storage_[i].equip = tmp_int[3];
			gs->storage_[i].identify = tmp_int[4];
			gs->storage_[i].refine = tmp_int[5];
			gs->storage_[i].attribute = tmp_int[6];
			for(j = 0; j < MAX_SLOTS && tmp_str && sscanf(tmp_str, ",%d%[0-9,-]",&tmp_int[0], tmp_str) > 0; j++)
				gs->storage_[i].card[j] = tmp_int[0];
			next += len;
			if (str[next] == ' ')
				next++;
		}
		else return 1;
	}
	if (i >= MAX_GUILD_STORAGE && str[next] && str[next]!='\t')
		ShowWarning("mmo_guildstorage_fromstr: Found a storage line with more items than MAX_GUILD_STORAGE (%d), remaining items have been discarded!\n", MAX_GUILD_STORAGE);

	return true;
}


static bool mmo_guildstorage_tostr(const struct guild_storage* gs, char* str)
{
	int i,j,f=0;
	char* p = str;

	p += sprintf(str, "%d,%d\t", gs->guild_id, gs->storage_amount);

	for( i = 0; i < MAX_GUILD_STORAGE; i++ )
	{
		if( gs->storage_[i].nameid != 0 && gs->storage_[i].amount != 0 )
		{
			p += sprintf(p, "%d,%d,%d,%d,%d,%d,%d",
				gs->storage_[i].id, gs->storage_[i].nameid, gs->storage_[i].amount, gs->storage_[i].equip,
				gs->storage_[i].identify, gs->storage_[i].refine, gs->storage_[i].attribute);

			for( j = 0; j < MAX_SLOTS; j++ )
				p += sprintf(p, ",%d", gs->storage_[i].card[j]);

			p += sprintf(p, " ");

			f++;
		}
	}

	*(p++) = '\t';

	*p = '\0';

	if( f == 0 )
		str[0] = 0;

	return true;
}


static bool mmo_guildstoragedb_sync(GuildStorageDB_TXT* db)
{
	DBIterator* iter;
	void* data;
	FILE* fp;
	int lock;

	fp = lock_fopen(db->guildstorage_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_guildstoragedb_sync: can't write [%s] !!! data is lost !!!\n", db->guildstorage_db);
		return false;
	}

	iter = db->guildstorages->iterator(db->guildstorages);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct guild_storage* gs = (struct guild_storage*) data;
		char line[65536];

		if( gs->storage_amount == 0 )
			continue;

		mmo_guildstorage_tostr(gs, line);

		if( line[0] == '\0' )
			continue; //FIXME: shouldn't this be caught by the check above?

		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->guildstorage_db, &lock);

	return true;
}


static bool guildstorage_db_txt_init(GuildStorageDB* self)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	DBMap* guildstorages;

	char line[65536];
	FILE *fp;

	// create pet database
	db->guildstorages = idb_alloc(DB_OPT_RELEASE_DATA);
	guildstorages = db->guildstorages;

	// open data file
	fp = fopen(db->guildstorage_db, "r");
	if( fp == NULL )
	{
		ShowError("guildstorage_db_txt_init: Cannot open file %s!\n", db->guildstorage_db);
		return false;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		struct guild_storage* gs = (struct guild_storage*)aCalloc(1, sizeof(struct guild_storage));
		if( gs == NULL )
		{
			ShowFatalError("guildstorage_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load storage for this guild
		if( !mmo_guildstorage_fromstr(gs, line) )
		{
			ShowError("guildstorage_db_txt_init: Broken line data: %s\n", line);
			aFree(gs);
			continue;
		}

		gs = (struct guild_storage*)idb_put(guildstorages, gs->guild_id, gs);
		if( gs != NULL )
		{
			ShowError("Duplicate entry in %s for guild %d\n", db->guildstorage_db, gs->guild_id);
			aFree(gs);
		}
	}

	fclose(fp);

	return true;
}

static void guildstorage_db_txt_destroy(GuildStorageDB* self)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	DBMap* guildstorages = db->guildstorages;

	// write data
	mmo_guildstoragedb_sync(db);

	// delete storage database
	guildstorages->destroy(guildstorages, NULL);
	db->guildstorages = NULL;

	// delete entire structure
	aFree(db);
}

static bool guildstorage_db_txt_sync(GuildStorageDB* self)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	return mmo_guildstoragedb_sync(db);
}

static bool guildstorage_db_txt_remove(GuildStorageDB* self, const int guild_id)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	DBMap* guildstorages = db->guildstorages;

	idb_remove(guildstorages, guild_id);

	return true;
}

/// Writes provided data into guild storage cache.
/// If data contains 0 items, any existing entry in cache is destroyed instead.
static bool guildstorage_db_txt_save(GuildStorageDB* self, const struct guild_storage* gs, int guild_id)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	DBMap* guildstorages = db->guildstorages;
	struct guild_storage* tmp;

	if( gs->storage_amount > 0 )
	{
		tmp = (struct guild_storage*)idb_ensure(guildstorages, guild_id, create_guildstorage);
		memcpy(tmp, gs, sizeof(*gs));
	}
	else
	{
		idb_remove(guildstorages, guild_id);
	}

	return true;
}

/// Loads guild storage data into the provided data structure.
/// If data doesn't exist, the destination is zeroed instead.
static bool guildstorage_db_txt_load(GuildStorageDB* self, struct guild_storage* gs, int guild_id)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)self;
	DBMap* guildstorages = db->guildstorages;
	struct guild_storage* tmp;
	
	tmp = (struct guild_storage*)idb_get(guildstorages, guild_id);

	if( tmp != NULL )
		memcpy(gs, tmp, sizeof(*gs));
	else
		memset(gs, 0x00, sizeof(*gs));

	return true;
}


/// public constructor
GuildStorageDB* guildstorage_db_txt(CharServerDB_TXT* owner)
{
	GuildStorageDB_TXT* db = (GuildStorageDB_TXT*)aCalloc(1, sizeof(GuildStorageDB_TXT));

	// set up the vtable
	db->vtable.init      = &guildstorage_db_txt_init;
	db->vtable.destroy   = &guildstorage_db_txt_destroy;
	db->vtable.sync      = &guildstorage_db_txt_sync;
	db->vtable.remove    = &guildstorage_db_txt_remove;
	db->vtable.save      = &guildstorage_db_txt_save;
	db->vtable.load      = &guildstorage_db_txt_load;

	// initialize to default values
	db->owner = owner;
	db->guildstorages = NULL;

	// other settings
	db->guildstorage_db = db->owner->file_guild_storages;

	return &db->vtable;
}
