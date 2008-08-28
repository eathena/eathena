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


/////////////////////////////
/// Account reg manipulation

int inter_accreg_tosql(int account_id, struct regs* reg)
{
	struct global_reg* r;
	SqlStmt* stmt;
	int i;

	if( account_id <= 0 )
		return 0;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
		Sql_ShowDebug(sql_handle);

	if( reg->reg_num <= 0 )
		return 0;

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES (2,'%d',?,?)", reg_db, account_id) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < reg->reg_num; ++i )
	{
		r = &reg->reg[i];
		if( r->str[0] == '\0' || r->value[0] == '\0' )
			continue; // should not save these

		SqlStmt_BindParam(stmt, 0, SQLDT_STRING, r->str, strnlen(r->str, sizeof(r->str)));
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, r->value, strnlen(r->value, sizeof(r->value)));

		if( SQL_ERROR == SqlStmt_Execute(stmt) )
			SqlStmt_ShowDebug(stmt);
	}
	SqlStmt_Free(stmt);
	return 1;
}


int inter_accreg_fromsql(int account_id, struct regs* reg)
{
	struct global_reg* r;
	char* data;
	size_t len;
	int i;

	if( reg == NULL)
		return 0;

	memset(reg, 0, sizeof(struct regs));

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < MAX_REG_NUM && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		r = &reg->reg[i];
		Sql_GetData(sql_handle, 0, &data, &len); memcpy(r->str, data, min(len, sizeof(r->str)));
		Sql_GetData(sql_handle, 1, &data, &len); memcpy(r->value, data, min(len, sizeof(r->value)));
	}
	reg->reg_num = i;
	Sql_FreeResult(sql_handle);
	return 1;
}

bool inter_accreg_load(int account_id, struct regs* reg)
{
	return ( inter_accreg_fromsql(account_id, reg) == true );
}

bool inter_accreg_save(int account_id, struct regs* reg)
{
	return( inter_accreg_tosql(account_id, reg) == true );
}


///////////////////////////////
/// Character reg manipulation

int inter_charreg_tosql(int char_id, struct regs* reg)
{
	struct global_reg* r;
	SqlStmt* stmt;
	int i;

	if( char_id <= 0 )
		return 0;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
		Sql_ShowDebug(sql_handle);

	if( reg->reg_num <= 0 )
		return 0;

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `char_id`, `str`, `value`) VALUES (3,'%d',?,?)", reg_db, char_id) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < reg->reg_num; ++i )
	{
		r = &reg->reg[i];
		if( r->str[0] == '\0' || r->value[0] == '\0' )
			continue; // should not save these
		
		SqlStmt_BindParam(stmt, 0, SQLDT_STRING, r->str, strnlen(r->str, sizeof(r->str)));
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, r->value, strnlen(r->value, sizeof(r->value)));

		if( SQL_ERROR == SqlStmt_Execute(stmt) )
			SqlStmt_ShowDebug(stmt);
	}
	SqlStmt_Free(stmt);
	return 1;
}

int inter_charreg_fromsql(int char_id, struct regs* reg)
{
	struct global_reg* r;
	char* data;
	size_t len;
	int i;

	if( reg == NULL)
		return 0;

	memset(reg, 0, sizeof(struct regs));

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < MAX_REG_NUM && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		r = &reg->reg[i];
		Sql_GetData(sql_handle, 0, &data, &len); memcpy(r->str, data, min(len, sizeof(r->str)));
		Sql_GetData(sql_handle, 1, &data, &len); memcpy(r->value, data, min(len, sizeof(r->value)));
	}
	reg->reg_num = i;
	Sql_FreeResult(sql_handle);
	return 1;
}

bool inter_charreg_load(int char_id, struct regs* reg)
{
	return ( inter_charreg_fromsql(char_id, reg) == true );
}

bool inter_charreg_save(int char_id, struct regs* reg)
{
	return( inter_charreg_tosql(char_id, reg) == true );
}


int inter_registry_init(void)
{
	return 0;
}

int inter_registry_final(void)
{
	return 0;
}
