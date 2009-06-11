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
#include "charregdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Maximum number of char ids cached in the iterator.
#define CHARREGDBITERATOR_MAXCACHE 16000


/// internal structure
typedef struct CharRegDB_SQL
{
	CharRegDB vtable;        // public interface

	CharServerDB_SQL* owner;
	Sql* charregs;           // SQL charreg storage
	
	// other settings
	const char* charreg_db;

} CharRegDB_SQL;


/// internal structure
typedef struct CharRegDBIterator_SQL
{
	CharRegDBIterator vtable;    // public interface

	CharRegDB_SQL* db;
	int* ids_arr;
	int ids_num;
	int pos;
	bool has_more;

} CharRegDBIterator_SQL;


static bool mmo_charreg_fromsql(CharRegDB_SQL* db, struct regs* reg, int char_id)
{
	Sql* sql_handle = db->charregs;
	int i;

	memset(reg, 0, sizeof(struct regs));

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'", db->charreg_db, char_id) )
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


static bool mmo_charreg_tosql(CharRegDB_SQL* db, const struct regs* reg, int char_id)
{
	Sql* sql_handle = db->charregs;
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
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'", db->charreg_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( reg->reg_num <= 0 )
	{// nothing more needed
		result = true;
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`type`, `char_id`, `str`, `value`) VALUES ", db->charreg_db);

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
		StringBuf_Printf(&buf, "(3,%d,'%s','%s')", char_id, esc_str, esc_value);

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


static bool charreg_db_sql_init(CharRegDB* self)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	db->charregs = db->owner->sql_handle;
	return true;
}

static void charreg_db_sql_destroy(CharRegDB* self)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	db->charregs = NULL;
	aFree(db);
}

static bool charreg_db_sql_sync(CharRegDB* self)
{
	// not applicable
	return true;
}

static bool charreg_db_sql_remove(CharRegDB* self, const int char_id)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	Sql* sql_handle = db->charregs;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `account_id`='%d'", db->charreg_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool charreg_db_sql_save(CharRegDB* self, const struct regs* reg, int char_id)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	return mmo_charreg_tosql(db, reg, char_id);
}

static bool charreg_db_sql_load(CharRegDB* self, struct regs* reg, int char_id)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	return mmo_charreg_fromsql(db, reg, char_id);
}


/// Private. Fills the cache of the iterator with ids.
static void charreg_db_sql_iter_P_fillcache(CharRegDBIterator_SQL* iter)
{
	CharRegDB_SQL* db = iter->db;
	Sql* sql_handle = db->charregs;
	int res;
	int last_id = 0;
	bool has_last_id = false;

	if( iter->ids_num > 0 )
	{
		last_id = iter->ids_arr[iter->ids_num-1];
		has_last_id = true;
	}

	if( has_last_id )
		res = Sql_Query(sql_handle, "SELECT DISTINCT `char_id` FROM `%s` WHERE `char_id`>%d ORDER BY `char_id` ASC LIMIT %d", db->charreg_db, last_id, CHARREGDBITERATOR_MAXCACHE+1);
	else
		res = Sql_Query(sql_handle, "SELECT DISTINCT `char_id` FROM `%s` ORDER BY `char_id` ASC LIMIT %d", db->charreg_db, CHARREGDBITERATOR_MAXCACHE+1);
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

		if( Sql_NumRows(sql_handle) > CHARREGDBITERATOR_MAXCACHE )
		{
			iter->has_more = true;
			iter->ids_num = CHARREGDBITERATOR_MAXCACHE;
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
static void charreg_db_sql_iter_destroy(CharRegDBIterator* self)
{
	CharRegDBIterator_SQL* iter = (CharRegDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}


/// Fetches the next accreg.
static bool charreg_db_sql_iter_next(CharRegDBIterator* self, struct regs* data, int* key)
{
	CharRegDBIterator_SQL* iter = (CharRegDBIterator_SQL*)self;
	CharRegDB_SQL* db = (CharRegDB_SQL*)iter->db;
	Sql* sql_handle = db->charregs;

	while( iter->pos+1 >= iter->ids_num )
	{
		if( !iter->has_more )
			return false;
		charreg_db_sql_iter_P_fillcache(iter);
	}

	++iter->pos;
	if( key )
		*key = iter->ids_arr[iter->pos];
	return mmo_charreg_fromsql(db, data, iter->ids_arr[iter->pos]);
}


/// Returns an iterator over all character regs.
static CharRegDBIterator* charreg_db_sql_iterator(CharRegDB* self)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)self;
	CharRegDBIterator_SQL* iter = (CharRegDBIterator_SQL*)aCalloc(1, sizeof(CharRegDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &charreg_db_sql_iter_destroy;
	iter->vtable.next    = &charreg_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;
	iter->has_more = true;// auto load on next

	return &iter->vtable;
}


/// public constructor
CharRegDB* charreg_db_sql(CharServerDB_SQL* owner)
{
	CharRegDB_SQL* db = (CharRegDB_SQL*)aCalloc(1, sizeof(CharRegDB_SQL));

	// set up the vtable
	db->vtable.init    = &charreg_db_sql_init;
	db->vtable.destroy = &charreg_db_sql_destroy;
	db->vtable.sync    = &charreg_db_sql_sync;
	db->vtable.remove  = &charreg_db_sql_remove;
	db->vtable.save    = &charreg_db_sql_save;
	db->vtable.load    = &charreg_db_sql_load;
 	db->vtable.iterator = &charreg_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->charregs = NULL;

	// other settings
	db->charreg_db = db->owner->table_registry;

	return &db->vtable;
}
