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
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "castledb.h"
#include <stdio.h>
#include <string.h>


/// global defines
#define CASTLEDB_TXT_DB_VERSION 00000000


/// Internal structure.
/// @private
typedef struct CastleDB_TXT
{
	// public interface
	CastleDB vtable;

	// data provider
	CSDB_TXT* db;

} CastleDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool castle_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct guild_castle* gc = (struct guild_castle*)data;

	*out_size = sizeof(*gc);

	if( size < sizeof(*gc) )
		return true;

	if( version == 00000000 )
	{
		int dummy;

		memset(gc, 0, sizeof(*gc));

		// structure of guild castle with the guardian hp included (old one)
		if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&gc->castle_id, &gc->guild_id, &gc->economy, &gc->defense,
			&gc->triggerE, &gc->triggerD, &gc->nextTime, &gc->payTime, &gc->createTime, &gc->visibleC,
			&gc->guardian[0].visible, &gc->guardian[1].visible, &gc->guardian[2].visible, &gc->guardian[3].visible,
			&gc->guardian[4].visible, &gc->guardian[5].visible, &gc->guardian[6].visible, &gc->guardian[7].visible,
			&dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) != 26 )
		// structure of guild castle without the hps (current one)
		if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			&gc->castle_id, &gc->guild_id, &gc->economy, &gc->defense,
			&gc->triggerE, &gc->triggerD, &gc->nextTime, &gc->payTime, &gc->createTime, &gc->visibleC,
			&gc->guardian[0].visible, &gc->guardian[1].visible, &gc->guardian[2].visible, &gc->guardian[3].visible,
			&gc->guardian[4].visible, &gc->guardian[5].visible, &gc->guardian[6].visible, &gc->guardian[7].visible) != 18 )
			return false;

		*key = gc->castle_id;
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool castle_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	struct guild_castle* gc = (struct guild_castle*)data;
	bool result;

	Txt* txt = Txt_Malloc();
	Txt_Init(txt, str, SIZE_MAX, 18, ',', '\0', ",");
	Txt_Bind(txt,  0, TXTDT_INT, &gc->castle_id, sizeof(gc->castle_id));
	Txt_Bind(txt,  1, TXTDT_INT, &gc->guild_id, sizeof(gc->guild_id));
	Txt_Bind(txt,  2, TXTDT_INT, &gc->economy, sizeof(gc->economy));
	Txt_Bind(txt,  3, TXTDT_INT, &gc->defense, sizeof(gc->defense));
	Txt_Bind(txt,  4, TXTDT_INT, &gc->triggerE, sizeof(gc->triggerE));
	Txt_Bind(txt,  5, TXTDT_INT, &gc->triggerD, sizeof(gc->triggerD));
	Txt_Bind(txt,  6, TXTDT_INT, &gc->nextTime, sizeof(gc->nextTime));
	Txt_Bind(txt,  7, TXTDT_INT, &gc->payTime, sizeof(gc->payTime));
	Txt_Bind(txt,  8, TXTDT_INT, &gc->createTime, sizeof(gc->createTime));
	Txt_Bind(txt,  9, TXTDT_INT, &gc->visibleC, sizeof(gc->visibleC));
	Txt_Bind(txt, 10, TXTDT_BOOL, &gc->guardian[0].visible, sizeof(gc->guardian[0].visible));
	Txt_Bind(txt, 11, TXTDT_BOOL, &gc->guardian[1].visible, sizeof(gc->guardian[1].visible));
	Txt_Bind(txt, 12, TXTDT_BOOL, &gc->guardian[2].visible, sizeof(gc->guardian[2].visible));
	Txt_Bind(txt, 13, TXTDT_BOOL, &gc->guardian[3].visible, sizeof(gc->guardian[3].visible));
	Txt_Bind(txt, 14, TXTDT_BOOL, &gc->guardian[4].visible, sizeof(gc->guardian[4].visible));
	Txt_Bind(txt, 15, TXTDT_BOOL, &gc->guardian[5].visible, sizeof(gc->guardian[5].visible));
	Txt_Bind(txt, 16, TXTDT_BOOL, &gc->guardian[6].visible, sizeof(gc->guardian[6].visible));
	Txt_Bind(txt, 17, TXTDT_BOOL, &gc->guardian[7].visible, sizeof(gc->guardian[7].visible));

	result = ( Txt_Write(txt) == TXT_SUCCESS && Txt_NumFields(txt) == 18 );
	Txt_Free(txt);

	return result;
}


/// @protected
static bool castle_db_txt_init(CastleDB* self)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return db->init(db);
/*
	if( count == 0 )
	{// empty castles file, set up a default layout
		int i;
		ShowStatus(" %s - making Default Data...\n", db->castle_db);
		for( i = 0; i < MAX_GUILDCASTLE; ++i )
		{
			struct guild_castle* gc = (struct guild_castle *) aCalloc(sizeof(struct guild_castle), 1);
			gc->castle_id = i;
			idb_put(castles, gc->castle_id, gc);
		}
		ShowStatus(" %s - making done\n", db->castle_db);
	}
*/
}


/// @protected
static void castle_db_txt_destroy(CastleDB* self)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool castle_db_txt_sync(CastleDB* self, bool force)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool castle_db_txt_create(CastleDB* self, struct guild_castle* gc)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;

	if( gc->castle_id == -1 )
		gc->castle_id = db->next_key(db);

	return db->insert(db, gc->castle_id, gc, sizeof(*gc));
}


/// @protected
static bool castle_db_txt_remove(CastleDB* self, const int castle_id)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return db->remove(db, castle_id);
}


/// @protected
static bool castle_db_txt_remove_gid(CastleDB* self, const int guild_id)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	DBIterator* iter = db->iterator(db);
	void* data;

	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct guild_castle* gc = (struct guild_castle*)data;

		if( gc->guild_id == guild_id )
		{
			iter->remove(iter);
			data = NULL; // invalidated
		}
	}

	iter->destroy(iter);

	return true;
}


/// @protected
static bool castle_db_txt_save(CastleDB* self, const struct guild_castle* gc)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return db->replace(db, gc->castle_id, gc, sizeof(*gc));
}


/// @protected
static bool castle_db_txt_load(CastleDB* self, struct guild_castle* gc, int castle_id)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return db->load(db, castle_id, gc, sizeof(*gc), NULL);
}


/// Returns an iterator over all castles.
/// @protected
static CSDBIterator* castle_db_txt_iterator(CastleDB* self)
{
	CSDB_TXT* db = ((CastleDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new CastleDB interface.
/// @protected
CastleDB* castle_db_txt(CharServerDB_TXT* owner)
{
	CastleDB_TXT* db = (CastleDB_TXT*)aCalloc(1, sizeof(CastleDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_castles, CASTLEDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &castle_db_txt_fromstr;
	db->db->p.tostr   = &castle_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &castle_db_txt_init;
	db->vtable.p.destroy = &castle_db_txt_destroy;
	db->vtable.p.sync    = &castle_db_txt_sync;
	db->vtable.create    = &castle_db_txt_create;
	db->vtable.remove    = &castle_db_txt_remove;
	db->vtable.remove_gid= &castle_db_txt_remove_gid;
	db->vtable.save      = &castle_db_txt_save;
	db->vtable.load      = &castle_db_txt_load;
	db->vtable.iterator  = &castle_db_txt_iterator;

	return &db->vtable;
}
