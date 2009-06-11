// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "charserverdb_sql.h"
#include "inter.h"
#include "accregdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Maximum number of account ids cached in the iterator.
#define ACCREGDBITERATOR_MAXCACHE 16000


/// internal structure
typedef struct AccRegDB_SQL
{
	AccRegDB vtable;        // public interface

	CharServerDB_SQL* owner;
	Sql* accregs;           // SQL accreg storage
	
	// other settings
	const char* accreg_db;

} AccRegDB_SQL;


/// internal structure
typedef struct AccRegDBIterator_SQL
{
	AccRegDBIterator vtable;    // public interface

	AccRegDB_SQL* db;
	int* ids_arr;
	int ids_num;
	int pos;
	bool has_more;

} AccRegDBIterator_SQL;


static bool mmo_accreg_fromsql(AccRegDB_SQL* db, struct regs* reg, int account_id)
{
	Sql* sql_handle = db->accregs;
	int i;

	memset(reg, 0, sizeof(struct regs));

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%d'", db->accreg_db, account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	for( i = 0; i < MAX_REG_NUM && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		char* data;
		size_t len;
		struct global_reg* r = &reg->reg[i];

		Sql_GetData(sql_handle, 0, &data, &len); memcpy(r->str, data, min(len, sizeof(r->str)));
		Sql_GetData(sql_handle, 1, &data, &len); memcpy(r->value, data, min(len, sizeof(r->value)));
	}
	reg->reg_num = i;
	Sql_FreeResult(sql_handle);

	return true;
}


static bool mmo_accreg_tosql(AccRegDB_SQL* db, const struct regs* reg, int account_id)
{
	Sql* sql_handle = db->accregs;
	StringBuf buf;
	int i, j;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	StringBuf_Init(&buf);

	// try
	do
	{

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'", db->accreg_db, account_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( reg->reg_num <= 0 )
	{// nothing to save
		result = true;
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES ", db->accreg_db);

	j = 0; // counter
	for( i = 0; i < reg->reg_num; ++i )
	{
		const struct global_reg* r = &reg->reg[i];
		char esc_str[2*31+1];
		char esc_value[2*255+1];

		if( r->str[0] == '\0' || r->value[0] == '\0' )
			continue; // should not save these

		Sql_EscapeStringLen(sql_handle, esc_str, r->str, strnlen(r->str, sizeof(r->str)));
		Sql_EscapeStringLen(sql_handle, esc_value, r->value, strnlen(r->value, sizeof(r->value)));

		if( j != 0 )
			StringBuf_AppendStr(&buf, ",");
		StringBuf_Printf(&buf, "(2,%d,'%s','%s')", account_id, esc_str, esc_value);

		j++;
	}

	if( j == 0 )
	{// nothing to save
		result = true;
		break;
	}

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	// success
	result = true;

	}
	while(0);
	// finally

	StringBuf_Destroy(&buf);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") )
	{
		Sql_ShowDebug(sql_handle);
		result = false;
	}

	return result;
}


static bool accreg_db_sql_init(AccRegDB* self)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	db->accregs = db->owner->sql_handle;
	return true;
}

static void accreg_db_sql_destroy(AccRegDB* self)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	db->accregs = NULL;
	aFree(db);
}

static bool accreg_db_sql_sync(AccRegDB* self)
{
	return true;
}

static bool accreg_db_sql_remove(AccRegDB* self, const int account_id)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	Sql* sql_handle = db->accregs;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'", db->accreg_db, account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool accreg_db_sql_save(AccRegDB* self, const struct regs* reg, int account_id)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	return mmo_accreg_tosql(db, reg, account_id);
}

static bool accreg_db_sql_load(AccRegDB* self, struct regs* reg, int account_id)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	return mmo_accreg_fromsql(db, reg, account_id);
}


/// Private. Fills the cache of the iterator with ids.
static void accreg_db_sql_iter_P_fillcache(AccRegDBIterator_SQL* iter)
{
	AccRegDB_SQL* db = iter->db;
	Sql* sql_handle = db->accregs;
	int res;
	int last_id = 0;
	bool has_last_id = false;

	if( iter->ids_num > 0 )
	{
		last_id = iter->ids_arr[iter->ids_num-1];
		has_last_id = true;
	}

	if( has_last_id )
		res = Sql_Query(sql_handle, "SELECT DISTINCT `account_id` FROM `%s` WHERE `account_id`>%d ORDER BY `account_id` ASC LIMIT %d", db->accreg_db, last_id, ACCREGDBITERATOR_MAXCACHE+1);
	else
		res = Sql_Query(sql_handle, "SELECT DISTINCT `account_id` FROM `%s` ORDER BY `account_id` ASC LIMIT %d", db->accreg_db, ACCREGDBITERATOR_MAXCACHE+1);
	if( res == SQL_ERROR )
	{
		Sql_ShowDebug(sql_handle);
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else if( Sql_NumRows(sql_handle) == 0 )
	{
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else
	{
		int i;

		if( Sql_NumRows(sql_handle) > ACCREGDBITERATOR_MAXCACHE )
		{
			iter->has_more = true;
			iter->ids_num = ACCREGDBITERATOR_MAXCACHE;
		}
		else
		{
			iter->has_more = false;
			iter->ids_num = (int)Sql_NumRows(sql_handle);
		}
		if( has_last_id )
		{
			++iter->ids_num;
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->ids_arr[0] = last_id;
			iter->pos = 0;
			i = 1;
		}
		else
		{
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->pos = -1;
			i = 0;
		}

		while( i < iter->ids_num )
		{
			char* data;
			int res = Sql_NextRow(sql_handle);
			if( res == SQL_SUCCESS )
				res = Sql_GetData(sql_handle, 0, &data, NULL);
			if( res == SQL_ERROR )
				Sql_ShowDebug(sql_handle);
			if( res != SQL_SUCCESS )
				break;

			if( data == NULL )
				continue;

			iter->ids_arr[i] = atoi(data);
			++i;
		}
		iter->ids_num = i;
	}
	Sql_FreeResult(sql_handle);
}


/// Destroys this iterator, releasing all allocated memory (including itself).
static void accreg_db_sql_iter_destroy(AccRegDBIterator* self)
{
	AccRegDBIterator_SQL* iter = (AccRegDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}


/// Fetches the next accreg.
static bool accreg_db_sql_iter_next(AccRegDBIterator* self, struct regs* data, int* key)
{
	AccRegDBIterator_SQL* iter = (AccRegDBIterator_SQL*)self;
	AccRegDB_SQL* db = (AccRegDB_SQL*)iter->db;
	Sql* sql_handle = db->accregs;

	while( iter->pos+1 >= iter->ids_num )
	{
		if( !iter->has_more )
			return false;
		accreg_db_sql_iter_P_fillcache(iter);
	}

	++iter->pos;
	if( key )
		*key = iter->ids_arr[iter->pos];
	return mmo_accreg_fromsql(db, data, iter->ids_arr[iter->pos]);
}


/// Returns an iterator over all account regs.
static AccRegDBIterator* accreg_db_sql_iterator(AccRegDB* self)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)self;
	AccRegDBIterator_SQL* iter = (AccRegDBIterator_SQL*)aCalloc(1, sizeof(AccRegDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &accreg_db_sql_iter_destroy;
	iter->vtable.next    = &accreg_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;
	iter->has_more = true;// auto load on next

	return &iter->vtable;
}


/// public constructor
AccRegDB* accreg_db_sql(CharServerDB_SQL* owner)
{
	AccRegDB_SQL* db = (AccRegDB_SQL*)aCalloc(1, sizeof(AccRegDB_SQL));

	// set up the vtable
	db->vtable.init    = &accreg_db_sql_init;
	db->vtable.destroy = &accreg_db_sql_destroy;
	db->vtable.sync    = &accreg_db_sql_sync;
	db->vtable.remove  = &accreg_db_sql_remove;
	db->vtable.save    = &accreg_db_sql_save;
	db->vtable.load    = &accreg_db_sql_load;
	db->vtable.iterator = &accreg_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->accregs = NULL;

	// other settings
	db->accreg_db = db->owner->table_registry;

	return &db->vtable;
}
