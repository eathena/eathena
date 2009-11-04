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
#include "csdb_txt.h"
#include "hotkeydb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define HOTKEYDB_TXT_DB_VERSION 00000000


/// Internal structure.
/// @private
typedef struct HotkeyDB_TXT
{
	// public interface
	HotkeyDB vtable;

	// data provider
	CSDB_TXT* db;

} HotkeyDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool hotkey_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	hotkeylist* list = (hotkeylist*)data;

	*out_size = sizeof(*list);

	if( size < sizeof(*list) )
		return true;

	if( version == 00000000 )
	{
		const char* p = str;
		int char_id, i, n;

		memset(list, 0, sizeof(*list));

		// load char id
		if( sscanf(p, "%d%n", &char_id, &n) != 1 )
			return false;

		p += n;

		//FIXME: savefile will break if MAX_HOTKEYS changes

		for( i = 0; i < MAX_HOTKEYS; i++, p += n )
		{
			int type, id, lv;

			if( sscanf(p, ",%d,%d,%d%n", &type, &id, &lv, &n) != 3 )
				return false;

			(*list)[i].type = type;
			(*list)[i].id = id;
			(*list)[i].lv = lv;
		}

		*key = char_id;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool hotkey_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int char_id = key;
	hotkeylist* list = (hotkeylist*)data;
	int i;
	int count = 0;

	if( size != sizeof(*list) )
		return false;

	// write char id
	p += sprintf(p, "%d", char_id);

	// write friend list for this char
	for( i = 0; i < MAX_HOTKEYS; i++ )
		p += sprintf(p, ",%d,%d,%d", (*list)[i].type, (*list)[i].id, (*list)[i].lv);

	return true;
}


/// @protected
static bool hotkey_db_txt_init(HotkeyDB* self)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void hotkey_db_txt_destroy(HotkeyDB* self)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool hotkey_db_txt_sync(HotkeyDB* self, bool force)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool hotkey_db_txt_remove(HotkeyDB* self, const int char_id)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// @protected
static bool hotkey_db_txt_save(HotkeyDB* self, const hotkeylist* list, const int char_id)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	return db->replace(db, char_id, list, sizeof(*list));
}


/// @protected
static bool hotkey_db_txt_load(HotkeyDB* self, hotkeylist* list, const int char_id)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;

	if( !db->load(db, char_id, list, sizeof(*list), NULL) )
		memset(list, 0, sizeof(*list));

	return true;
}


/// Returns an iterator over all hotkey lists.
/// @protected
static CSDBIterator* hotkey_db_txt_iterator(HotkeyDB* self)
{
	CSDB_TXT* db = ((HotkeyDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new HotkeyDB interface.
/// @protected
HotkeyDB* hotkey_db_txt(CharServerDB_TXT* owner)
{
	HotkeyDB_TXT* db = (HotkeyDB_TXT*)aCalloc(1, sizeof(HotkeyDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_hotkeys, HOTKEYDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &hotkey_db_txt_fromstr;
	db->db->p.tostr   = &hotkey_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &hotkey_db_txt_init;
	db->vtable.p.destroy = &hotkey_db_txt_destroy;
	db->vtable.p.sync    = &hotkey_db_txt_sync;
	db->vtable.remove    = &hotkey_db_txt_remove;
	db->vtable.save      = &hotkey_db_txt_save;
	db->vtable.load      = &hotkey_db_txt_load;
	db->vtable.iterator  = &hotkey_db_txt_iterator;

	return &db->vtable;
}
