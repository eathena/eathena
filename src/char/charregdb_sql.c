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


/// internal structure
typedef struct CharRegDB_SQL
{
	CharRegDB vtable;        // public interface

	CharServerDB_SQL* owner;
	Sql* charregs;           // SQL charreg storage
	
	// other settings
	const char* charreg_db;

} CharRegDB_SQL;


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

	// initialize to default values
	db->owner = owner;
	db->charregs = NULL;

	// other settings
	db->charreg_db = db->owner->table_registry;

	return &db->vtable;
}
