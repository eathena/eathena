// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "inter.h"


extern Sql* sql_handle;



//--------------------------------------------------------
// Save registry to sql
int inter_accreg_tosql(int account_id, int char_id, struct accreg* reg, int type)
{
	struct global_reg* r;
	SqlStmt* stmt;
	int i;

	if( account_id <= 0 )
		return 0;
	reg->account_id = account_id;
	reg->char_id = char_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	switch( type )
	{
	case 3: //Char Reg
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
			Sql_ShowDebug(sql_handle);
		account_id = 0;
		break;
	case 2: //Account Reg
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
			Sql_ShowDebug(sql_handle);
		char_id = 0;
		break;
	case 1: //Account2 Reg
		ShowError("inter_accreg_tosql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
		return 0;
	default:
		ShowError("inter_accreg_tosql: Invalid type %d\n", type);
		return 0;
	}

	if( reg->reg_num <= 0 )
		return 0;

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `char_id`, `str`, `value`) VALUES ('%d','%d','%d',?,?)", reg_db, type, account_id, char_id) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < reg->reg_num; ++i )
	{
		r = &reg->reg[i];
		if( r->str[0] != '\0' && r->value != '\0' )
		{
			// str
			SqlStmt_BindParam(stmt, 0, SQLDT_STRING, r->str, strnlen(r->str, sizeof(r->str)));
			// value
			SqlStmt_BindParam(stmt, 1, SQLDT_STRING, r->value, strnlen(r->value, sizeof(r->value)));

			if( SQL_ERROR == SqlStmt_Execute(stmt) )
				SqlStmt_ShowDebug(stmt);
		}
	}
	SqlStmt_Free(stmt);
	return 1;
}


// Load account_reg from sql (type=2)
int inter_accreg_fromsql(int account_id,int char_id, struct accreg *reg, int type)
{
	struct global_reg* r;
	char* data;
	size_t len;
	int i;

	if( reg == NULL)
		return 0;

	memset(reg, 0, sizeof(struct accreg));
	reg->account_id = account_id;
	reg->char_id = char_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	switch( type )
	{
	case 3: //char reg
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
			Sql_ShowDebug(sql_handle);
		break;
	case 2: //account reg
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
			Sql_ShowDebug(sql_handle);
		break;
	case 1: //account2 reg
		ShowError("inter_accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
		return 0;
	default:
		ShowError("inter_accreg_fromsql: Invalid type %d\n", type);
		return 0;
	}
	for( i = 0; i < MAX_REG_NUM && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		r = &reg->reg[i];
		// str
		Sql_GetData(sql_handle, 0, &data, &len);
		memcpy(r->str, data, min(len, sizeof(r->str)));
		// value
		Sql_GetData(sql_handle, 1, &data, &len);
		memcpy(r->value, data, min(len, sizeof(r->value)));
	}
	reg->reg_num = i;
	Sql_FreeResult(sql_handle);
	return 1;
}



int inter_accreg_init(void)
{
	return 0;
}

int inter_accreg_final(void)
{
	return 0;
}
