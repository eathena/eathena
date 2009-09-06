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
#include "accregdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define ACCREGDB_TXT_DB_VERSION 20090825


/// Internal structure.
/// @private
typedef struct AccRegDB_TXT
{
	// public interface
	AccRegDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* accregs;
	bool dirty;

	// settings
	const char* accreg_db;

} AccRegDB_TXT;


/// @private
static void* create_accregs(DBKey key, va_list args)
{
	return (struct regs*)aMalloc(sizeof(struct regs));
}


/// @private
static bool mmo_accreg_fromstr(struct regs* reg, const char* str)
{
	int fields[ACCOUNT_REG_NUM+1][2];
	int nfields;
	int i;

	if( str[0] == '\0' )
		nfields = 0;
	else
		nfields = sv_parse(str, strlen(str), 0, ' ', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_ESCAPE_C, SV_TERMINATE_LF|SV_TERMINATE_CRLF));

	for( i = 1; i <= nfields; ++i )
	{
		int off[2+1][2];

		if( i == nfields && fields[i][0] == fields[i][1] )
		{// don't count trailing space as block
			nfields--;
			break;
		}

		if( sv_parse(str, fields[i][1], fields[i][0], ',', (int*)off, 2*ARRAYLENGTH(off), (e_svopt)(SV_ESCAPE_C)) != 2 )
			return false;

		sv_unescape_c(reg->reg[i-1].str, str + off[1][0], off[1][1] - off[1][0]);
		sv_unescape_c(reg->reg[i-1].value, str + off[2][0], off[2][1] - off[2][0]);
	}

	reg->reg_num = nfields;

	return true;
}


/// @private
static bool mmo_accreg_tostr(const struct regs* reg, char* str)
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
static bool accreg_db_txt_init(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs;
	char line[8192];
	FILE* fp;
	unsigned int version = 0;

	// create accreg database
	if( db->accregs == NULL )
		db->accregs = idb_alloc(DB_OPT_RELEASE_DATA);
	accregs = db->accregs;
	db_clear(accregs);

	// open data file
	fp = fopen(db->accreg_db, "r");
	if( fp == NULL )
	{
		ShowError("Accreg file not found: %s.\n", db->accreg_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int account_id, n;
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
			ShowFatalError("accreg_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load account id
		n = 0;
		if( sscanf(line, "%d%n\t", &account_id, &n) != 1 || line[n] != '\t' )
		{
			aFree(reg);
			continue;
		}

		// load regs for this account
		if( !mmo_accreg_fromstr(reg, line + n + 1) )
		{
			ShowError("accreg_db_txt_init: broken data [%s] account id %d\n", db->accreg_db, account_id);
			aFree(reg);
			continue;
		}

		// record entry in db
		idb_put(accregs, account_id, reg);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void accreg_db_txt_destroy(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs = db->accregs;

	// delete accreg database
	if( accregs != NULL )
	{
		db_destroy(accregs);
		db->accregs = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool accreg_db_txt_sync(AccRegDB* self, bool force)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->accreg_db, &lock);
	if( fp == NULL )
	{
		ShowError("accreg_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->accreg_db);
		return false;
	}

	fprintf(fp, "%d\n", ACCREGDB_TXT_DB_VERSION);

	iter = db->accregs->iterator(db->accregs);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int account_id = key.i;
		struct regs* reg = (struct regs*) data;
		char line[8192];

		if( reg->reg_num == 0 )
			continue;

		mmo_accreg_tostr(reg, line);
		fprintf(fp, "%d\t%s\n", account_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->accreg_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool accreg_db_txt_remove(AccRegDB* self, const int account_id)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs = db->accregs;

	idb_remove(accregs, account_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool accreg_db_txt_save(AccRegDB* self, const struct regs* reg, int account_id)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs = db->accregs;

	if( reg->reg_num > 0 )
	{
		struct regs* tmp = (struct regs*)idb_ensure(accregs, account_id, create_accregs);
		memcpy(tmp, reg, sizeof(*reg));
	}
	else
	{
		idb_remove(accregs, account_id);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool accreg_db_txt_load(AccRegDB* self, struct regs* reg, int account_id)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs = db->accregs;
	struct regs* tmp;

	tmp = (struct regs*)idb_get(accregs, account_id);

	if( tmp != NULL )
		memcpy(reg, tmp, sizeof(*reg));
	else
		memset(reg, 0x00, sizeof(*reg));

	return true;
}


/// Returns an iterator over all account regs.
/// @protected
static CSDBIterator* accreg_db_txt_iterator(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->accregs));
}


/// Constructs a new AccRegDB interface.
/// @protected
AccRegDB* accreg_db_txt(CharServerDB_TXT* owner)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)aCalloc(1, sizeof(AccRegDB_TXT));

	// set up the vtable
	db->vtable.p.init    = &accreg_db_txt_init;
	db->vtable.p.destroy = &accreg_db_txt_destroy;
	db->vtable.p.sync    = &accreg_db_txt_sync;
	db->vtable.remove  = &accreg_db_txt_remove;
	db->vtable.save    = &accreg_db_txt_save;
	db->vtable.load    = &accreg_db_txt_load;
	db->vtable.iterator = &accreg_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->accregs = NULL;
	db->dirty = false;

	// other settings
	db->accreg_db = db->owner->file_accregs;

	return &db->vtable;
}
