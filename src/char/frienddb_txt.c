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
#include "frienddb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define FRIENDDB_TXT_DB_VERSION 00000000


/// Internal structure.
/// @private
typedef struct FriendDB_TXT
{
	// public interface
	FriendDB vtable;

	// data provider
	CSDB_TXT* db;

} FriendDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool friend_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	friendlist* list = (friendlist*)data;

	*out_size = sizeof(*list);

	if( size < sizeof(*list) )
		return true;

	if( version == 00000000 )
	{
		const char* p = str;
		int char_id, i, n;

		memset(list, 0, sizeof(friendlist));

		// load char id
		if( sscanf(p, "%d%n", &char_id, &n) != 1 )
			return false;

		p += n;

		for( i = 0; i < MAX_FRIENDS; i++, p += n )
		{
			int account_id;
			int char_id;
			char name[NAME_LENGTH];

			if( *p == '\n' || *p == '\r' || *p == '\0' )
				break;

			if( sscanf(p, ",%d,%d,%23[^,\n]%n", &account_id, &char_id, name, &n) != 3 )
				return false;

			(*list)[i].account_id = account_id;
			(*list)[i].char_id = char_id;
			safestrncpy((*list)[i].name, name, NAME_LENGTH);
		}

		*key = char_id;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool friend_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int char_id = key;
	friendlist* list = (friendlist*)data;
	int i;
	int count = 0;

	if( size != sizeof(*list) )
		return false;

	// write char id
	p += sprintf(p, "%d", char_id);

	// write friend list for this char
	for( i = 0; i < MAX_FRIENDS; i++ )
	{
		if( (*list)[i].account_id == 0 || (*list)[i].char_id == 0 || (*list)[i].name[0] == '\0' )
			continue;

		p += sprintf(p, ",%d,%d,%s", (*list)[i].account_id, (*list)[i].char_id, (*list)[i].name);
		count++;
	}

	if( count == 0 )
		str[0] = '\0';

	return true;
}


/// @protected
static bool friend_db_txt_init(FriendDB* self)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void friend_db_txt_destroy(FriendDB* self)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool friend_db_txt_sync(FriendDB* self, bool force)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool friend_db_txt_remove(FriendDB* self, const int char_id)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// @protected
static bool friend_db_txt_save(FriendDB* self, const friendlist* list, const int char_id)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	return db->replace(db, char_id, list, sizeof(*list));
}


/// @protected
static bool friend_db_txt_load(FriendDB* self, friendlist* list, const int char_id)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;

	if( !db->load(db, char_id, list, sizeof(*list), NULL) )
		memset(list, 0, sizeof(*list));

	return true;
}


/// Returns an iterator over all friend lists.
/// @protected
static CSDBIterator* friend_db_txt_iterator(FriendDB* self)
{
	CSDB_TXT* db = ((FriendDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new FriendDB interface.
/// @protected
FriendDB* friend_db_txt(CharServerDB_TXT* owner)
{
	FriendDB_TXT* db = (FriendDB_TXT*)aCalloc(1, sizeof(FriendDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_friends, FRIENDDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &friend_db_txt_fromstr;
	db->db->p.tostr   = &friend_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &friend_db_txt_init;
	db->vtable.p.destroy = &friend_db_txt_destroy;
	db->vtable.p.sync    = &friend_db_txt_sync;
	db->vtable.remove    = &friend_db_txt_remove;
	db->vtable.save      = &friend_db_txt_save;
	db->vtable.load      = &friend_db_txt_load;
	db->vtable.iterator  = &friend_db_txt_iterator;

	return &db->vtable;
}
