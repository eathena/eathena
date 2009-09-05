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
#include "hotkeydb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct HotkeyDB_TXT
{
	// public interface
	HotkeyDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* hotkeys;
	bool dirty;

	// settings
	const char* hotkey_db;

} HotkeyDB_TXT;


/// @private
static void* create_hotkeylist(DBKey key, va_list args)
{
	return (hotkeylist*)aMalloc(sizeof(hotkeylist));
}


/// @private
static bool mmo_hotkeylist_fromstr(hotkeylist* list, char* str)
{
	const char* p = str;
	int i, n;

	memset(list, 0, sizeof(*list));

	for( i = 0; i < MAX_HOTKEYS; i++, p += n )
	{
		int type, id, lv;

		if( sscanf(p, ",%d,%d,%d%n", &type, &id, &lv, &n) != 3 )
			return false;

		(*list)[i].type = type;
		(*list)[i].id = id;
		(*list)[i].lv = lv;
	}

	return true;
}


/// @private
static bool mmo_hotkeylist_tostr(const hotkeylist* list, char* str)
{
	int i;
	char* p = str;

	p[0] = '\0';

	for( i = 0; i < MAX_HOTKEYS; i++ )
		p += sprintf(p, ",%d,%d,%d", (*list)[i].type, (*list)[i].id, (*list)[i].lv);

	return true;
}


/// @protected
static bool hotkey_db_txt_init(HotkeyDB* self)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBMap* hotkeys;

	char line[8192];
	FILE *fp;

	// create hotkey database
	if( db->hotkeys == NULL )
		db->hotkeys = idb_alloc(DB_OPT_RELEASE_DATA);
	hotkeys = db->hotkeys;
	db_clear(hotkeys);

	// open data file
	fp = fopen(db->hotkey_db, "r");
	if( fp == NULL )
	{
		ShowError("Hotkey file not found: %s.\n", db->hotkey_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id;
		int n;

		hotkeylist* list = (hotkeylist*)aCalloc(1, sizeof(hotkeylist));
		if( list == NULL )
		{
			ShowFatalError("hotkey_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n", &char_id, &n) != 1 || char_id <= 0 )
		{
			aFree(list);
			continue;
		}

		// load hotkeys for this char
		if( !mmo_hotkeylist_fromstr(list, line + n) )
		{
			ShowError("hotkey_db_txt_init: skipping invalid data: %s", line);
			continue;
		}
	
		// record entry in db
		idb_put(hotkeys, char_id, list);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void hotkey_db_txt_destroy(HotkeyDB* self)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBMap* hotkeys = db->hotkeys;

	// delete hotkey database
	if( hotkeys != NULL )
	{
		db_destroy(hotkeys);
		db->hotkeys = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool hotkey_db_txt_sync(HotkeyDB* self)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->hotkey_db, &lock);
	if( fp == NULL )
	{
		ShowError("hotkey_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->hotkey_db);
		return false;
	}

	iter = db->hotkeys->iterator(db->hotkeys);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		hotkeylist* list = (hotkeylist*) data;
		char line[8192];

		mmo_hotkeylist_tostr(list, line);
		fprintf(fp, "%d%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->hotkey_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool hotkey_db_txt_remove(HotkeyDB* self, const int char_id)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBMap* hotkeys = db->hotkeys;

	idb_remove(hotkeys, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool hotkey_db_txt_save(HotkeyDB* self, const hotkeylist* list, const int char_id)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBMap* hotkeys = db->hotkeys;

	// retrieve previous data / allocate new data
	hotkeylist* tmp = idb_ensure(hotkeys, char_id, create_hotkeylist);
	if( tmp == NULL )
	{// error condition - allocation problem?
		return false;
	}

	// overwrite with new data
	memcpy(tmp, list, sizeof(hotkeylist));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool hotkey_db_txt_load(HotkeyDB* self, hotkeylist* list, const int char_id)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	DBMap* hotkeys = db->hotkeys;

	// retrieve data
	hotkeylist* tmp = idb_get(hotkeys, char_id);

	if( tmp != NULL )
		memcpy(list, tmp, sizeof(hotkeylist));
	else
		memset(list, 0, sizeof(hotkeylist));

	return true;
}


/// Returns an iterator over all hotkey lists.
/// @protected
static CSDBIterator* hotkey_db_txt_iterator(HotkeyDB* self)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->hotkeys));
}


/// Constructs a new HotkeyDB interface.
/// @protected
HotkeyDB* hotkey_db_txt(CharServerDB_TXT* owner)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)aCalloc(1, sizeof(HotkeyDB_TXT));

	// set up the vtable
	db->vtable.init      = &hotkey_db_txt_init;
	db->vtable.destroy   = &hotkey_db_txt_destroy;
	db->vtable.sync      = &hotkey_db_txt_sync;
	db->vtable.remove    = &hotkey_db_txt_remove;
	db->vtable.save      = &hotkey_db_txt_save;
	db->vtable.load      = &hotkey_db_txt_load;
	db->vtable.iterator  = &hotkey_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->hotkeys = NULL;
	db->dirty = true;

	// other settings
	db->hotkey_db = db->owner->file_hotkeys;

	return &db->vtable;
}
