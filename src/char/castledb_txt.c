// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "castledb.h"
#include "charserverdb_txt.h"
#include <stdio.h>
#include <string.h>

#define START_CASTLE_NUM 1

/// internal structure
typedef struct CastleDB_TXT
{
	CastleDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* castles;       // in-memory castle storage
	int next_castle_id;   // auto_increment

	char castle_db[1024]; // castle data storage file

} CastleDB_TXT;

/// internal functions
static bool castle_db_txt_init(CastleDB* self);
static void castle_db_txt_destroy(CastleDB* self);
static bool castle_db_txt_sync(CastleDB* self);
static bool castle_db_txt_create(CastleDB* self, struct guild_castle* gc);
static bool castle_db_txt_remove(CastleDB* self, const int castle_id);
static bool castle_db_txt_save(CastleDB* self, const struct guild_castle* gc);
static bool castle_db_txt_load_num(CastleDB* self, struct guild_castle* gc, int castle_id);

static bool mmo_castle_fromstr(struct guild_castle* gc, char* str);
static bool mmo_castle_tostr(const struct guild_castle* gc, char* str);
static bool mmo_castle_sync(CastleDB_TXT* db);

/// public constructor
CastleDB* castle_db_txt(CharServerDB_TXT* owner)
{
	CastleDB_TXT* db = (CastleDB_TXT*)aCalloc(1, sizeof(CastleDB_TXT));

	// set up the vtable
	db->vtable.init      = &castle_db_txt_init;
	db->vtable.destroy   = &castle_db_txt_destroy;
	db->vtable.sync      = &castle_db_txt_sync;
	db->vtable.create    = &castle_db_txt_create;
	db->vtable.remove    = &castle_db_txt_remove;
	db->vtable.save      = &castle_db_txt_save;
	db->vtable.load_num  = &castle_db_txt_load_num;

	// initialize to default values
	db->owner = owner;
	db->castles = NULL;
	db->next_castle_id = START_CASTLE_NUM;
	// other settings
	safestrncpy(db->castle_db, "save/castle.txt", sizeof(db->castle_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


static bool castle_db_txt_init(CastleDB* self)
{
	CastleDB_TXT* db = (CastleDB_TXT*)self;
	DBMap* castles;

	char line[16384];
	FILE* fp;
	int count = 0;

	// create castle database
	db->castles = idb_alloc(DB_OPT_RELEASE_DATA);
	castles = db->castles;

	// open data file
	fp = fopen(db->castle_db, "r");
	if( fp == NULL )
		return 1;

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		struct guild_castle* gc = (struct guild_castle *) aCalloc(sizeof(struct guild_castle), 1);

		if( !mmo_castle_fromstr(gc, line) )
		{
			ShowError("castle_db_txt_init: skipping invalid data: %s", line);
			aFree(gc);
			continue;
		}

		// record entry in db
		idb_put(castles, gc->castle_id, gc);

		if( gc->castle_id >= db->next_castle_id )
			db->next_castle_id = gc->castle_id + 1;

		count++;
	}

	// close data file
	fclose(fp);

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

	return true;
}

static void castle_db_txt_destroy(CastleDB* self)
{
	CastleDB_TXT* db = (CastleDB_TXT*)self;
	DBMap* castles = db->castles;
}

static bool castle_db_txt_sync(CastleDB* self)
{
	CastleDB_TXT* db = (CastleDB_TXT*)self;
	return mmo_castle_sync(db);
}

static bool castle_db_txt_create(CastleDB* self, struct guild_castle* gc)
{
/*
*/
}

static bool castle_db_txt_remove(CastleDB* self, const int castle_id)
{
/*
*/
}

static bool castle_db_txt_save(CastleDB* self, const struct guild_castle* gc)
{
/*
*/
}

static bool castle_db_txt_load_num(CastleDB* self, struct guild_castle* gc, int castle_id)
{
/*
*/
}


/// parses the castle data string into a castle data structure
static bool mmo_castle_fromstr(struct guild_castle* gc, char* str)
{
/*
	int castleid, guildid, economy, defense, triggerE, triggerD, nextTime, payTime, createTime, visibleC;
	int guardian[8];
	int dummy;

	memset(gc, 0, sizeof(struct guild_castle));
	// structure of guild castle with the guardian hp included
	if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		&castleid, &guildid, &economy, &defense, &triggerE, &triggerD, &nextTime, &payTime, &createTime, &visibleC,
		&guardian[0], &guardian[1], &guardian[2], &guardian[3], &guardian[4], &guardian[5], &guardian[6], &guardian[7],
		&dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) != 26 )
	// structure of guild castle without the hps (current one)
	if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		&castleid, &guildid, &economy, &defense, &triggerE, &triggerD, &nextTime, &payTime, &createTime, &visibleC,
		&guardian[0], &guardian[1], &guardian[2], &guardian[3], &guardian[4], &guardian[5], &guardian[6], &guardian[7]) != 18 )
		return 1;

	gc->castle_id = castleid;
	gc->guild_id = guildid;
	gc->economy = economy;
	gc->defense = defense;
	gc->triggerE = triggerE;
	gc->triggerD = triggerD;
	gc->nextTime = nextTime;
	gc->payTime = payTime;
	gc->createTime = createTime;
	gc->visibleC = visibleC;
	gc->guardian[0].visible = guardian[0];
	gc->guardian[1].visible = guardian[1];
	gc->guardian[2].visible = guardian[2];
	gc->guardian[3].visible = guardian[3];
	gc->guardian[4].visible = guardian[4];
	gc->guardian[5].visible = guardian[5];
	gc->guardian[6].visible = guardian[6];
	gc->guardian[7].visible = guardian[7];
*/
	return true;
}

/// serializes the castle data structure into the provided string
static bool mmo_castle_tostr(const struct guild_castle* gc, char* str)
{
/*
	int len;

	len = sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	              gc->castle_id, gc->guild_id, gc->economy, gc->defense, gc->triggerE,
	              gc->triggerD, gc->nextTime, gc->payTime, gc->createTime, gc->visibleC,
	              gc->guardian[0].visible, gc->guardian[1].visible, gc->guardian[2].visible, gc->guardian[3].visible,
	              gc->guardian[4].visible, gc->guardian[5].visible, gc->guardian[6].visible, gc->guardian[7].visible);
*/
	return true;
}

static bool mmo_castle_sync(CastleDB_TXT* db)
{
/*
	FILE *fp;
	int lock;
	DBIterator* iter;
	struct guild_castle* gc;

	// save castle data
	if ((fp = lock_fopen(castle_txt,&lock)) == NULL) {
		ShowError("int_guild: can't write [%s] !!! data is lost !!!\n", castle_txt);
		return 1;
	}

	iter = castle_db->iterator(castle_db);
	for( gc = (struct guild_castle*)iter->first(iter,NULL); iter->exists(iter); gc = (struct guild_castle*)iter->next(iter,NULL) )
	{
		char line[16384];
		inter_guildcastle_tostr(line, gc);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, castle_txt, &lock);
*/
}
