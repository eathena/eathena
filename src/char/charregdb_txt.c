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
#include "charregdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define CHARREGDB_TXT_DB_VERSION 20090825


/// Internal structure.
/// @private
typedef struct CharRegDB_TXT
{
	// public interface
	CharRegDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* charregs;
	bool dirty;

	// settings
	const char* charreg_db;

} CharRegDB_TXT;


/// @private
static void* create_charregs(DBKey key, va_list args)
{
	return (struct regs*)aMalloc(sizeof(struct regs));
}


/// @private
static bool mmo_charreg_fromstr(struct regs* reg, const char* str)
{
	int fields[GLOBAL_REG_NUM+1][2];
	int nfields;
	int i;

	if( str[0] == '\0' )
		nfields = 0;
	else
		nfields = sv_parse(str, strlen(str), 0, ' ', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF));

	for( i = 1; i <= nfields; ++i )
	{
		int off[2+1][2];

		if( sv_parse(str, fields[i][1], fields[i][0], ',', (int*)off, 2*ARRAYLENGTH(off), (e_svopt)(SV_ESCAPE_C)) != 2 )
			return false;

		sv_unescape_c(reg->reg[i-1].str, str + off[1][0], off[1][1] - off[1][0]);
		sv_unescape_c(reg->reg[i-1].value, str + off[2][0], off[2][1] - off[2][0]);
	}

	reg->reg_num = nfields;

	return true;
}


/// @private
static bool mmo_charreg_tostr(const struct regs* reg, char* str)
{
	char* p = str;
	bool first = true;
	int i;

	for( i = 0; i < reg->reg_num; ++i )
	{
		char esc_str[4*32+1];
		char esc_value[4*256+1];

		if( reg->reg[i].str[0] == '\0' )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, " ");

			sv_escape_c(esc_str, reg->reg[i].str, strlen(reg->reg[i].str), " ,");
			sv_escape_c(esc_value, reg->reg[i].value, strlen(reg->reg[i].value), " ");
			p += sprintf(p, "%s,%s", esc_str, esc_value);
	}

	*p = '\0';

	return true;
}


/// @protected
static bool charreg_db_txt_init(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs;
	char line[8192];
	FILE* fp;
	unsigned int version = 0;

	// create charreg database
	if( db->charregs == NULL )
		db->charregs = idb_alloc(DB_OPT_RELEASE_DATA);
	charregs = db->charregs;
	db_clear(db->charregs);

	// open data file
	fp = fopen(db->charreg_db, "r");
	if( fp == NULL )
	{
		ShowError("Charreg file not found: %s.\n", db->charreg_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id, n;
		unsigned int v;
		struct regs* reg;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		reg = (struct regs*)aCalloc(1, sizeof(struct regs));
		if( reg == NULL )
		{
			ShowFatalError("charreg_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n\t", &char_id, &n) != 1 || line[n] != '\t' )
		{
			aFree(reg);
			continue;
		}

		// load regs for this character
		if( !mmo_charreg_fromstr(reg, line + n + 1) )
		{
			ShowError("charreg_db_txt_init: broken data [%s] char id %d\n", db->charreg_db, char_id);
			aFree(reg);
			continue;
		}

		// record entry in db
		idb_put(charregs, char_id, reg);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void charreg_db_txt_destroy(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;

	// delete charreg database
	if( charregs != NULL )
	{
		db_destroy(charregs);
		db->charregs = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool charreg_db_txt_sync(CharRegDB* self, bool force)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->charreg_db, &lock);
	if( fp == NULL )
	{
		ShowError("charreg_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->charreg_db);
		return false;
	}

	fprintf(fp, "%d\n", CHARREGDB_TXT_DB_VERSION);

	iter = db->charregs->iterator(db->charregs);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		struct regs* reg = (struct regs*) data;
		char line[8192];

		if( reg->reg_num == 0 )
			continue;

		mmo_charreg_tostr(reg, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->charreg_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool charreg_db_txt_remove(CharRegDB* self, const int char_id)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;

	idb_remove(charregs, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool charreg_db_txt_save(CharRegDB* self, const struct regs* reg, int char_id)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;

	if( reg->reg_num > 0 )
	{
		struct regs* tmp = (struct regs*)idb_ensure(charregs, char_id, create_charregs);
		memcpy(tmp, reg, sizeof(*reg));
	}
	else
	{
		idb_remove(charregs, char_id);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool charreg_db_txt_load(CharRegDB* self, struct regs* reg, int char_id)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;
	struct regs* tmp;

	tmp = (struct regs*)idb_get(charregs, char_id);

	if( tmp != NULL )
		memcpy(reg, tmp, sizeof(*reg));
	else
		memset(reg, 0x00, sizeof(*reg));

	return true;
}


/// Returns an iterator over all character regs.
/// @protected
static CSDBIterator* charreg_db_txt_iterator(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->charregs));
}


/// Constructs a new CharRegDB interface.
/// @protected
CharRegDB* charreg_db_txt(CharServerDB_TXT* owner)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)aCalloc(1, sizeof(CharRegDB_TXT));

	// set up the vtable
	db->vtable.p.init    = &charreg_db_txt_init;
	db->vtable.p.destroy = &charreg_db_txt_destroy;
	db->vtable.p.sync    = &charreg_db_txt_sync;
	db->vtable.remove  = &charreg_db_txt_remove;
	db->vtable.save    = &charreg_db_txt_save;
	db->vtable.load    = &charreg_db_txt_load;
	db->vtable.iterator = &charreg_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->charregs = NULL;
	db->dirty = false;

	// other settings
	db->charreg_db = db->owner->file_charregs;

	return &db->vtable;
}
