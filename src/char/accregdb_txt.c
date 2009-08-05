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


/// internal structure
typedef struct AccRegDB_TXT
{
	AccRegDB vtable;        // public interface

	CharServerDB_TXT* owner;
	DBMap* accregs;         // in-memory accreg storage
	bool dirty;
	
	const char* accreg_db;  // accreg data storage file

} AccRegDB_TXT;


static void* create_accregs(DBKey key, va_list args)
{
	return (struct regs*)aMalloc(sizeof(struct regs));
}


static bool mmo_accreg_fromstr(struct regs* reg, const char* str)
{
	const char* p = str;
	int i, n;

	//FIXME: no escaping - will break if str/value contains commas or spaces
	for( i = 0; i < ACCOUNT_REG_NUM; i++, p += n )
	{
		if (sscanf(p, "%[^,],%[^ ] %n", reg->reg[i].str, reg->reg[i].value, &n) != 2) 
			break;
	}
	reg->reg_num = i;

	return true;
}


static bool mmo_accreg_tostr(const struct regs* reg, char* str)
{
	char* p = str;
	int i;

	p[0] = '\0';

	for( i = 0; i < reg->reg_num; ++i )
		p += sprintf(p, "%s,%s ", reg->reg[i].str, reg->reg[i].value);

	return true;
}


static bool mmo_accreg_sync(AccRegDB_TXT* db)
{
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->accreg_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_accreg_sync: can't write [%s] !!! data is lost !!!\n", db->accreg_db);
		return false;
	}

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


static bool accreg_db_txt_init(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs;

	char line[8192];
	FILE* fp;

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
		int account_id;
		int n;

		struct regs* reg = (struct regs*)aCalloc(1, sizeof(struct regs));
		if( reg == NULL )
		{
			ShowFatalError("accreg_db_txt_init: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load account id
		if( sscanf(line, "%d\t%n", &account_id, &n) != 1 || account_id <= 0 )
		{
			aFree(reg);
			continue;
		}

		// load regs for this account
		if( !mmo_accreg_fromstr(reg, line + n) )
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

static bool accreg_db_txt_sync(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	return mmo_accreg_sync(db);
}

static bool accreg_db_txt_remove(AccRegDB* self, const int account_id)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	DBMap* accregs = db->accregs;

	idb_remove(accregs, account_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

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
static CSDBIterator* accreg_db_txt_iterator(AccRegDB* self)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->accregs));
}


/// public constructor
AccRegDB* accreg_db_txt(CharServerDB_TXT* owner)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)aCalloc(1, sizeof(AccRegDB_TXT));

	// set up the vtable
	db->vtable.init    = &accreg_db_txt_init;
	db->vtable.destroy = &accreg_db_txt_destroy;
	db->vtable.sync    = &accreg_db_txt_sync;
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
