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
#include "frienddb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct FriendDB_TXT
{
	// public interface
	FriendDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* friends;
	bool dirty;

	// settings
	const char* friend_db;

} FriendDB_TXT;



/// @private
static void* create_friendlist(DBKey key, va_list args)
{
	return (friendlist*)aMalloc(sizeof(friendlist));
}


/// @private
static bool mmo_friendlist_fromstr(friendlist* list, char* str)
{
	const char* p = str;
	int i, n;

	memset(list, 0, sizeof(friendlist));

	for( i = 0; i < MAX_FRIENDS; i++, p += n )
	{
		int account_id;
		int char_id;
		char name[NAME_LENGTH];

		if( *p == '\n' || *p == '\0' )
			break;

		if( sscanf(p, ",%d,%d,%23[^,\n]%n", &account_id, &char_id, name, &n) != 3 )
			return false;

		(*list)[i].account_id = account_id;
		(*list)[i].char_id = char_id;
		safestrncpy((*list)[i].name, name, NAME_LENGTH);
	}
	
	return true;
}


/// @private
static bool mmo_friendlist_tostr(const friendlist* list, char* str)
{
	int i;
	char* p = str;

	p[0] = '\0';

	for( i = 0; i < MAX_FRIENDS; i++ )
		if( (*list)[i].account_id > 0 && (*list)[i].char_id > 0 && (*list)[i].name[0] != '\0' )
			p += sprintf(p, ",%d,%d,%s", (*list)[i].account_id, (*list)[i].char_id, (*list)[i].name);

	return true;
}


/// @protected
static bool friend_db_txt_init(FriendDB* self)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBMap* friends = db->friends;
	char line[8192];
	FILE *fp;

	db_clear(friends);

	// open data file
	fp = fopen(db->friend_db, "r");
	if( fp == NULL )
	{
		ShowError("Friend file not found: %s.\n", db->friend_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id;
		int n;

		friendlist* list = (friendlist*)aCalloc(1, sizeof(friendlist));
		if( list == NULL )
		{
			ShowFatalError("friend_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n", &char_id, &n) != 1 || char_id <= 0 )
		{
			aFree(list);
			continue;
		}

		// load friends for this char
		if( !mmo_friendlist_fromstr(list, line + n) )
		{
			ShowError("friend_db_txt_init: skipping invalid data: %s", line);
			continue;
		}
	
		// record entry in db
		idb_put(friends, char_id, list);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void friend_db_txt_destroy(FriendDB* self)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBMap* friends = db->friends;

	// delete friend database
	db_destroy(friends);
	db->friends = NULL;

	// delete entire structure
	aFree(db);
}


/// @protected
static bool friend_db_txt_sync(FriendDB* self)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->friend_db, &lock);
	if( fp == NULL )
	{
		ShowError("friend_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->friend_db);
		return false;
	}

	iter = db->friends->iterator(db->friends);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		friendlist* list = (friendlist*) data;
		char line[8192];

		mmo_friendlist_tostr(list, line);
		fprintf(fp, "%d%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->friend_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool friend_db_txt_remove(FriendDB* self, const int char_id)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBMap* friends = db->friends;

	idb_remove(friends, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool friend_db_txt_save(FriendDB* self, const friendlist* list, const int char_id)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBMap* friends = db->friends;

	// retrieve previous data / allocate new data
	friendlist* tmp = idb_ensure(friends, char_id, create_friendlist);
	if( tmp == NULL )
	{// error condition - allocation problem?
		return false;
	}

	// overwrite with new data
	memcpy(tmp, list, sizeof(friendlist));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool friend_db_txt_load(FriendDB* self, friendlist* list, const int char_id)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	DBMap* friends = db->friends;

	// retrieve data
	friendlist* tmp = idb_get(friends, char_id);
	if( tmp == NULL )
	{// if no data, just fake it
		memset(list, 0, sizeof(friendlist));
		return true;
	}

	// store it
	memcpy(list, tmp, sizeof(friendlist));

	return true;
}


/// Returns an iterator over all friend lists.
/// @protected
static CSDBIterator* friend_db_txt_iterator(FriendDB* self)
{
	FriendDB_TXT* db = (FriendDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->friends));
}


/// Constructs a new FriendDB interface.
/// @protected
FriendDB* friend_db_txt(CharServerDB_TXT* owner)
{
	FriendDB_TXT* db = (FriendDB_TXT*)aCalloc(1, sizeof(FriendDB_TXT));

	// set up the vtable
	db->vtable.init      = &friend_db_txt_init;
	db->vtable.destroy   = &friend_db_txt_destroy;
	db->vtable.sync      = &friend_db_txt_sync;
	db->vtable.remove    = &friend_db_txt_remove;
	db->vtable.save      = &friend_db_txt_save;
	db->vtable.load      = &friend_db_txt_load;
	db->vtable.iterator  = &friend_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->friends = idb_alloc(DB_OPT_RELEASE_DATA);
	db->dirty = false;

	// other settings
	db->friend_db = db->owner->file_friends;

	return &db->vtable;
}
