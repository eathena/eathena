// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/txt.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "storagedb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define STORAGEDB_TXT_DB_VERSION 20090825


/// Internal structure.
/// @private
typedef struct StorageDB_TXT
{
	// public interface
	StorageDB vtable;

	// data providers
	CSDB_TXT* inventories;
	CSDB_TXT* carts;
	CSDB_TXT* storages;
	CSDB_TXT* guildstorages;

} StorageDB_TXT;


/// @private
static CSDB_TXT* type2db(StorageDB_TXT* db, enum storage_type type)
{
	switch( type )
	{
	case STORAGE_INVENTORY: return db->inventories;
	case STORAGE_CART     : return db->carts;
	case STORAGE_KAFRA    : return db->storages;
	case STORAGE_GUILD    : return db->guildstorages;
	default:
		return NULL;
	}
}


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool storage_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct item* s = (struct item*)data;

	if( version == 20090825 )
	{
		Txt* txt;
		int id, n, i, j;
		struct item it;
		bool done = false;

		if( sscanf(str, "%d%n", &id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		memset(&it, 0, sizeof(it));
		*out_size = 0;

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 7+MAX_SLOTS, ',', ' ', "");
		Txt_Bind(txt, 0, TXTDT_INT, &it.id, sizeof(it.id));
		Txt_Bind(txt, 1, TXTDT_SHORT, &it.nameid, sizeof(it.nameid));
		Txt_Bind(txt, 2, TXTDT_SHORT, &it.amount, sizeof(it.amount));
		Txt_Bind(txt, 3, TXTDT_USHORT, &it.equip, sizeof(it.equip));
		Txt_Bind(txt, 4, TXTDT_CHAR, &it.identify, sizeof(it.identify));
		Txt_Bind(txt, 5, TXTDT_CHAR, &it.refine, sizeof(it.refine));
		Txt_Bind(txt, 6, TXTDT_CHAR, &it.attribute, sizeof(it.attribute));
		for( j = 0; j < MAX_SLOTS; ++j )
			Txt_Bind(txt, 7+j, TXTDT_SHORT, &it.card[j], sizeof(it.card[j]));

		i = 0;
		while( !done )
		{
			if( Txt_Parse(txt) != TXT_SUCCESS )
				break;

			if( Txt_NumFields(txt) == 0 )
			{// no more data
				done = true;
				break;
			}

			if( Txt_NumFields(txt) < 7 )
				break; // parsing failure

			*out_size += sizeof(struct item);

			if( (i + 1) * sizeof(struct item) > size )
				continue; // discard entries over max

			memcpy(&s[i], &it, sizeof(it));
			memset(&it, 0, sizeof(it));
			i++;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		*key = id;
	}
	else
	if( version == 00000000 )
	{
		int tmp_int[7+MAX_SLOTS];
		char tmp_str[256];
		int id, dummy, n;
		int i, j;

		if( sscanf(str, "%d,%d%n", &id, &dummy, &n) != 2 || str[n] != '\t' )
			return false;

		str += n + 1;
		*out_size = 0;

		for( i = 0; *str != '\0' && *str != '\t'; i++ )
		{
			if(sscanf(str, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
				  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				  &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &n) != 8)
				  return false;

			str += n;

			if( *str == ' ' )
				str++;

			*out_size += sizeof(struct item);

			if( (i + 1) * sizeof(struct item) > size )
				continue; // discard entries over max

			s[i].id = tmp_int[0];
			s[i].nameid = tmp_int[1];
			s[i].amount = tmp_int[2];
			s[i].equip = tmp_int[3];
			s[i].identify = tmp_int[4];
			s[i].refine = tmp_int[5];
			s[i].attribute = tmp_int[6];
				
			for( j = 0; j < MAX_SLOTS && tmp_str[0] != '\0' && sscanf(tmp_str, ",%d%[0-9,-]", &tmp_int[7+j], tmp_str) > 0; j++ )
				s[i].card[j] = tmp_int[7+j]; //NOTE: sscanf into same buffer has undefined behavior
		}

		*key = id;
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool storage_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	char* p = str;
	int id = key;
	struct item* s = (struct item*)data;
	size_t i, j;
	int count = 0;
	Txt* txt;

	p += sprintf(p, "%d\t", id);

	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 7+MAX_SLOTS, ',', ' ', ", ");

	for( i = 0; i < datasize / sizeof(struct item); i++ )
	{
		if( s[i].nameid == 0 || s[i].amount == 0 )
			continue;

		Txt_Bind(txt, 0, TXTDT_INT, &s[i].id, sizeof(s[i].id));
		Txt_Bind(txt, 1, TXTDT_SHORT, &s[i].nameid, sizeof(s[i].nameid));
		Txt_Bind(txt, 2, TXTDT_SHORT, &s[i].amount, sizeof(s[i].amount));
		Txt_Bind(txt, 3, TXTDT_USHORT, &s[i].equip, sizeof(s[i].equip));
		Txt_Bind(txt, 4, TXTDT_CHAR, &s[i].identify, sizeof(s[i].identify));
		Txt_Bind(txt, 5, TXTDT_CHAR, &s[i].refine, sizeof(s[i].refine));
		Txt_Bind(txt, 6, TXTDT_CHAR, &s[i].attribute, sizeof(s[i].attribute));
		for( j = 0; j < MAX_SLOTS; j++ )
			Txt_Bind(txt, 7+j, TXTDT_SHORT, &s[i].card[j], sizeof(s[i].card[j]));

		if( Txt_Write(txt) != TXT_SUCCESS )
		{
			Txt_Free(txt);
			return false;
		}

		count++;
	}

	Txt_Free(txt);

	if( count == 0 )
		str[0] = '\0';

	return true;
}


/// @protected
static bool storage_db_txt_init(StorageDB* self)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;

	return ( db->inventories->init(db->inventories)	&&
	         db->carts->init(db->carts) &&
	         db->storages->init(db->storages) &&
	         db->guildstorages->init(db->guildstorages)
	);
}


/// @protected
static void storage_db_txt_destroy(StorageDB* self)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;

	// delete storage databases
	if( db->inventories != NULL )
		db->inventories->destroy(db->inventories);
	if( db->carts != NULL )
		db->carts->destroy(db->carts);
	if( db->storages != NULL )
		db->storages->destroy(db->storages);
	if( db->guildstorages != NULL )
		db->guildstorages->destroy(db->guildstorages);

	// delete entire structure
	aFree(db);
}


/// @protected
static bool storage_db_txt_sync(StorageDB* self, bool force)
{
	StorageDB_TXT* db = (StorageDB_TXT*)self;
	bool result = true;

	if( !db->inventories->sync(db->inventories, force) )
		result = false;
	if( !db->carts->sync(db->carts, force) )
		result = false;
	if( !db->storages->sync(db->storages, force) )
		result = false;
	if( !db->guildstorages->sync(db->guildstorages, force) )
		result = false;

	return result;
}


/// @protected
static bool storage_db_txt_remove(StorageDB* self, enum storage_type type, const int id)
{
	CSDB_TXT* db = type2db((StorageDB_TXT*)self, type);
	return db->remove(db, id);
}


/// Writes provided data into storage cache.
/// @protected
static bool storage_db_txt_save(StorageDB* self, const struct item* s, size_t size, enum storage_type type, int id)
{
	CSDB_TXT* db = type2db((StorageDB_TXT*)self, type);
	return db->replace(db, id, s, size * sizeof(*s));
}


/// Loads storage data into the provided data structure.
/// If data doesn't exist, the destination is zeroed instead.
/// @protected
static bool storage_db_txt_load(StorageDB* self, struct item* s, size_t size, enum storage_type type, int id)
{
	CSDB_TXT* db = type2db((StorageDB_TXT*)self, type);
	size_t insize = size * sizeof(*s);
	size_t outsize;
	int count;

	if( !db->load(db, id, s, insize, &outsize) )
		outsize = 0;

	count = outsize / sizeof(*s);
	memset(&s[count], 0, insize - outsize);

	return true;
}


/// Returns an iterator over all storages.
/// @protected
static CSDBIterator* storage_db_txt_iterator(StorageDB* self, enum storage_type type)
{
	CSDB_TXT* db = type2db((StorageDB_TXT*)self, type);
	return db->iterator(db);
}


/// Constructs a new StorageDB interface.
/// @protected
StorageDB* storage_db_txt(CharServerDB_TXT* owner)
{
	StorageDB_TXT* db = (StorageDB_TXT*)aCalloc(1, sizeof(StorageDB_TXT));

	// call base class constructors and bind abstract methods
	db->inventories              = csdb_txt(owner, owner->file_inventories, STORAGEDB_TXT_DB_VERSION, 0);
	db->inventories->p.fromstr   = &storage_db_txt_fromstr;
	db->inventories->p.tostr     = &storage_db_txt_tostr;
	db->carts                    = csdb_txt(owner, owner->file_carts, STORAGEDB_TXT_DB_VERSION, 0);
	db->carts->p.fromstr         = &storage_db_txt_fromstr;
	db->carts->p.tostr           = &storage_db_txt_tostr;
	db->storages                 = csdb_txt(owner, owner->file_storages, STORAGEDB_TXT_DB_VERSION, 0);
	db->storages->p.fromstr      = &storage_db_txt_fromstr;
	db->storages->p.tostr        = &storage_db_txt_tostr;
	db->guildstorages            = csdb_txt(owner, owner->file_guild_storages, STORAGEDB_TXT_DB_VERSION, 0);
	db->guildstorages->p.fromstr = &storage_db_txt_fromstr;
	db->guildstorages->p.tostr   = &storage_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &storage_db_txt_init;
	db->vtable.p.destroy = &storage_db_txt_destroy;
	db->vtable.p.sync    = &storage_db_txt_sync;
	db->vtable.remove    = &storage_db_txt_remove;
	db->vtable.save      = &storage_db_txt_save;
	db->vtable.load      = &storage_db_txt_load;
	db->vtable.iterator  = &storage_db_txt_iterator;

	return &db->vtable;
}
