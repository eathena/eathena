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


/// internal structure
typedef struct CharRegDB_TXT
{
	CharRegDB vtable;        // public interface

	CharServerDB_TXT* owner;
	DBMap* charregs;         // in-memory charreg storage

} CharRegDB_TXT;


/// internal structure
typedef struct CharRegDBIterator_TXT
{
	CharRegDBIterator vtable;      // public interface
	DBIterator* iter;

} CharRegDBIterator_TXT;


static void* create_charregs(DBKey key, va_list args)
{
	return (struct regs*)aMalloc(sizeof(struct regs));
}


bool mmo_charreg_fromstr(struct regs* reg, const char* str)
{
	//FIXME: no escaping - will break if str/value contains commas or spaces
	//FIXME: doesn't obey size limits
	int i;
	int len;

	for( i = 0; *str && *str != '\t' && *str != '\n' && *str != '\r'; ++i )
	{// global_reg実装以前のathena.txt互換のため一応'\n'チェック
		if( sscanf(str, "%[^,],%[^ ] %n", reg->reg[i].str, reg->reg[i].value, &len) != 2 )
		{ 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if( *str == ',' && sscanf(str, ",%[^ ] %n", reg->reg[i].value, &len) == 1 )
				i--;
			else
				return false;
		}

		str += len;
		if ( *str == ' ' )
			str++;
	}

	reg->reg_num = i;

	return true;
}


//static bool mmo_charreg_tostr(const struct regs* reg, char* str)
bool mmo_charreg_tostr(const struct regs* reg, char* str)
{
	char* p = str;
	int i;

	p[0] = '\0';

	for( i = 0; i < reg->reg_num; ++i )
		if( reg->reg[i].str[0] != '\0' )
			p += sprintf(p, "%s,%s ", reg->reg[i].str, reg->reg[i].value);

	return true;
}


static bool charreg_db_txt_init(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;

	// create charreg database
	db->charregs = idb_alloc(DB_OPT_RELEASE_DATA);

	return true;
}

static void charreg_db_txt_destroy(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;

	// delete charreg database
	charregs->destroy(charregs, NULL);
	db->charregs = NULL;

	// delete entire structure
	aFree(db);
}

static bool charreg_db_txt_sync(CharRegDB* self)
{
	// not applicable
	return true;
}

static bool charreg_db_txt_remove(CharRegDB* self, const int char_id)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;

	idb_remove(charregs, char_id);

	return true;
}

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

	return true;
}

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


/// Destroys this iterator, releasing all allocated memory (including itself).
static void charreg_db_txt_iter_destroy(CharRegDBIterator* self)
{
	CharRegDBIterator_TXT* iter = (CharRegDBIterator_TXT*)self;
	dbi_destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next character reg data.
static bool charreg_db_txt_iter_next(CharRegDBIterator* self, struct regs* data, int* key)
{
	CharRegDBIterator_TXT* iter = (CharRegDBIterator_TXT*)self;
	struct regs* tmp;
	DBKey k;

	while( true )
	{
		tmp = (struct regs*)iter->iter->next(iter->iter, &k);
		if( tmp == NULL )
			return false;// not found

		if( key )
			*key = k.i;
		memcpy(data, tmp, sizeof(*data));
		return true;
	}
}


/// Returns an iterator over all the character regs.
static CharRegDBIterator* charreg_db_txt_iterator(CharRegDB* self)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)self;
	DBMap* charregs = db->charregs;
	CharRegDBIterator_TXT* iter = (CharRegDBIterator_TXT*)aCalloc(1, sizeof(CharRegDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &charreg_db_txt_iter_destroy;
	iter->vtable.next    = &charreg_db_txt_iter_next;

	// fill data
	iter->iter = db_iterator(charregs);

	return &iter->vtable;
}


/// public constructor
CharRegDB* charreg_db_txt(CharServerDB_TXT* owner)
{
	CharRegDB_TXT* db = (CharRegDB_TXT*)aCalloc(1, sizeof(CharRegDB_TXT));

	// set up the vtable
	db->vtable.init    = &charreg_db_txt_init;
	db->vtable.destroy = &charreg_db_txt_destroy;
	db->vtable.sync    = &charreg_db_txt_sync;
	db->vtable.remove  = &charreg_db_txt_remove;
	db->vtable.save    = &charreg_db_txt_save;
	db->vtable.load    = &charreg_db_txt_load;
	db->vtable.iterator = &charreg_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->charregs = NULL;

	return &db->vtable;
}
