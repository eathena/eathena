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
#include "mercdb.h"
#include <stdio.h>
#include <string.h>


/// global defines
#define MERCDB_TXT_DB_VERSION 20090805
#define START_MERCENARY_NUM 1


/// Internal structure.
/// @private
typedef struct MercDB_TXT
{
	// public interface
	MercDB vtable;

	// data provider
	CSDB_TXT* db;

} MercDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool merc_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct s_mercenary* md = (struct s_mercenary*)data;

	*out_size = sizeof(*md);

	if( size < sizeof(*md) )
		return true;

	if( version == 20090805 )
	{
		// zero out the destination first
		memset(md, 0x00, sizeof(*md));

		if( sscanf(str, "%d\t%d,%hd,%d,%d,%u,%u", &md->mercenary_id, &md->char_id, &md->class_, &md->hp, &md->sp, &md->kill_count, &md->life_time) != 7 )
			return false;

		*key = md->mercenary_id;
	}
	else
	{// unmatched row	
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool merc_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	const struct s_mercenary* md = (const struct s_mercenary*)data;

	p += sprintf(p, "%d\t%d,%hd,%d,%d,%u,%u", md->mercenary_id, md->char_id, md->class_, md->hp, md->sp, md->kill_count, md->life_time);

	return true;
}


/// @protected
static bool merc_db_txt_init(MercDB* self)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void merc_db_txt_destroy(MercDB* self)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool merc_db_txt_sync(MercDB* self, bool force)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool merc_db_txt_create(MercDB* self, struct s_mercenary* md)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;

	if( md->mercenary_id == -1 )
		md->mercenary_id = db->next_key(db);

	return db->insert(db, md->mercenary_id, md, sizeof(*md));
}


/// @protected
static bool merc_db_txt_remove(MercDB* self, const int merc_id)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return db->remove(db, merc_id);
}


/// @protected
static bool merc_db_txt_save(MercDB* self, const struct s_mercenary* md)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return db->update(db, md->mercenary_id, md, sizeof(*md));
}


/// @protected
static bool merc_db_txt_load(MercDB* self, struct s_mercenary* md, int merc_id)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return db->load(db, merc_id, md, sizeof(*md), NULL);
}


/// Returns an iterator over all mercs.
/// @protected
static CSDBIterator* merc_db_txt_iterator(MercDB* self)
{
	CSDB_TXT* db = ((MercDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new MercDB interface.
/// @protected
MercDB* merc_db_txt(CharServerDB_TXT* owner)
{
	MercDB_TXT* db = (MercDB_TXT*)aCalloc(1, sizeof(MercDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_mercenaries, MERCDB_TXT_DB_VERSION, START_MERCENARY_NUM);
	db->db->p.fromstr = &merc_db_txt_fromstr;
	db->db->p.tostr   = &merc_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &merc_db_txt_init;
	db->vtable.p.destroy = &merc_db_txt_destroy;
	db->vtable.p.sync    = &merc_db_txt_sync;
	db->vtable.create    = &merc_db_txt_create;
	db->vtable.remove    = &merc_db_txt_remove;
	db->vtable.save      = &merc_db_txt_save;
	db->vtable.load      = &merc_db_txt_load;
	db->vtable.iterator  = &merc_db_txt_iterator;

	return &db->vtable;
}
