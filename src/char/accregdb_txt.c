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
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
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

	// data provider
	CSDB_TXT* db;

} AccRegDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool accreg_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct regs* reg = (struct regs*)data;

	*out_size = sizeof(*reg);

	if( size < sizeof(*reg) )
		return true;

	if( version == 20090825 )
	{
		struct global_reg tmp;
		int account_id, n, i;
		Txt* txt;
		bool done = false;

		// load account id
		if( sscanf(str, "%d%n", &account_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		// load regs for this account
		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 2, ',', ' ', "");
		Txt_Bind(txt, 0, TXTDT_CSTRING, tmp.str, sizeof(tmp.str));
		Txt_Bind(txt, 1, TXTDT_CSTRING, tmp.value, sizeof(tmp.value));

		i = 0;
		while( Txt_Parse(txt) == TXT_SUCCESS )
		{
			if( Txt_NumFields(txt) == 0 )
			{// no more data
				done = true;
				break;
			}

			if( Txt_NumFields(txt) != 2 )
				break; // parsing failure

			//TODO: size limit check here

			memcpy(&reg->reg[i], &tmp, sizeof(tmp));
			i++;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		reg->reg_num = i;
		*key = account_id;
	}
	else
	if( version == 00000000 )
	{
		int account_id, n, i;

		if( sscanf(str, "%d%n", &account_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		for( i = 0; i < ACCOUNT_REG_NUM; i++, str += n )
			if( sscanf(str, "%[^,],%[^ ] %n", reg->reg[i].str, reg->reg[i].value, &n) != 2 ) 
				break;

		if( *str != '\0' )
			return false;

		reg->reg_num = i;
		*key = account_id;
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool accreg_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int account_id = key;
	struct regs* reg = (struct regs*)data;
	int i;
	int count = 0;
	Txt* txt;

	if( size != sizeof(*reg) )
		return false;

	// write account id
	p += sprintf(p, "%d\t", account_id);

	// write regs for this account
	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 2, ',', ' ', ", \t");

	for( i = 0; i < reg->reg_num; ++i )
	{
		if( reg->reg[i].str[0] == '\0' )
			continue;

		Txt_Bind(txt, 0, TXTDT_CSTRING, reg->reg[i].str, sizeof(reg->reg[i].str));
		Txt_Bind(txt, 1, TXTDT_CSTRING, reg->reg[i].value, sizeof(reg->reg[i].value));
		Txt_Write(txt);

		count++;
	}

	Txt_Free(txt);

	if( count == 0 )
		str[0] = '\0';

	return true;
}


/// @protected
static bool accreg_db_txt_init(AccRegDB* self)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void accreg_db_txt_destroy(AccRegDB* self)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool accreg_db_txt_sync(AccRegDB* self, bool force)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool accreg_db_txt_remove(AccRegDB* self, const int account_id)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;
	return db->remove(db, account_id);
}


/// @protected
static bool accreg_db_txt_save(AccRegDB* self, const struct regs* reg, int account_id)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;

	if( reg->reg_num > 0 )
		return db->replace(db, account_id, reg, sizeof(*reg));
	else
		return db->remove(db, account_id);
}


/// @protected
static bool accreg_db_txt_load(AccRegDB* self, struct regs* reg, int account_id)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;

	if( !db->load(db, account_id, reg, sizeof(*reg), NULL) )
		memset(reg, 0x00, sizeof(*reg));

	return true;
}


/// Returns an iterator over all account regs.
/// @protected
static CSDBIterator* accreg_db_txt_iterator(AccRegDB* self)
{
	CSDB_TXT* db = ((AccRegDB_TXT*)self)->db;
	return csdb_txt_iterator(db->iterator(db));
}


/// Constructs a new AccRegDB interface.
/// @protected
AccRegDB* accreg_db_txt(CharServerDB_TXT* owner)
{
	AccRegDB_TXT* db = (AccRegDB_TXT*)aCalloc(1, sizeof(AccRegDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_accregs, ACCREGDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &accreg_db_txt_fromstr;
	db->db->p.tostr   = &accreg_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &accreg_db_txt_init;
	db->vtable.p.destroy = &accreg_db_txt_destroy;
	db->vtable.p.sync    = &accreg_db_txt_sync;
	db->vtable.remove    = &accreg_db_txt_remove;
	db->vtable.save      = &accreg_db_txt_save;
	db->vtable.load      = &accreg_db_txt_load;
	db->vtable.iterator  = &accreg_db_txt_iterator;

	return &db->vtable;
}
