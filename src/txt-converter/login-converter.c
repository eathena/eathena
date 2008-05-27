// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/malloc.h"
#include "../common/strlib.h"

#include "../login/account.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char account_db[256]="login";
char globalreg_db[256]="global_reg_value";

int db_server_port = 3306;
char db_server_ip[32] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";

#define INTER_CONF_NAME "conf/inter_athena.conf"
#define ACCOUNT_TXT_NAME "save/account.txt"
//--------------------------------------------------------

/// Loads 'acc' from 'str'.
/// Only supports latest version.
/// Copypasted from ../login/account_txt.c.
bool acc_fromtxt(struct mmo_account* a, char* str)
{
	char* fields[32];
	int count;
	char* regs;
	int i, n;

	// zero out the destination first
	memset(a, 0x00, sizeof(struct mmo_account));

	// extract tab-separated columns from line
	count = sv_split(str, strlen(str), 0, '\t', fields, ARRAYLENGTH(fields), SV_NOESCAPE_NOTERMINATE);

	if( count != 13 )
		return false;

	a->account_id = strtol(fields[1], NULL, 10);
	safestrncpy(a->userid, fields[2], sizeof(a->userid));
	safestrncpy(a->pass, fields[3], sizeof(a->pass));
	a->sex = fields[4][0];
	safestrncpy(a->email, fields[5], sizeof(a->email));
	a->level = strtoul(fields[6], NULL, 10);
	a->state = strtoul(fields[7], NULL, 10);
	a->unban_time = strtol(fields[8], NULL, 10);
	a->expiration_time = strtol(fields[9], NULL, 10);
	a->logincount = strtoul(fields[10], NULL, 10);
	safestrncpy(a->lastlogin, fields[11], sizeof(a->lastlogin));
	safestrncpy(a->last_ip, fields[12], sizeof(a->last_ip));
	regs = fields[13];

	// extract account regs
	// {reg name<COMMA>reg value<SPACE>}*
	n = 0;
	for( i = 0; i < ACCOUNT_REG2_NUM; ++i )
	{
		char key[32];
		char value[256];
	
		regs += n;

		if (sscanf(regs, "%31[^\t,],%255[^\t ] %n", key, value, &n) != 2)
		{
			// We must check if a str is void. If it's, we can continue to read other REG2.
			// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
			if (regs[0] == ',' && sscanf(regs, ",%[^\t ] %n", value, &n) == 1) { 
				i--;
				continue;
			} else
				break;
		}
		
		safestrncpy(a->account_reg2[i].str, key, 32);
		safestrncpy(a->account_reg2[i].value, value, 256);
	}
	a->account_reg2_num = i;

	return true;
}

/// Saves 'acc' using 'sql_handle'.
/// Copypasted from ../login/account_sql.c.
bool acc_tosql(const struct mmo_account* acc, Sql* sql_handle)
{
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	bool result = false;
	int i;

	// try
	do
	{

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	{// insert into account table
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt,
			"INSERT INTO `%s` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `expiration_time`,`last_ip`) VALUES (?, ?, ?, ?, ?, ?, ?)",
			account_db)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_INT,    (void*)&acc->account_id, sizeof(acc->account_id))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)acc->userid, strlen(acc->userid))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void*)acc->pass, strlen(acc->pass))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_ENUM,   (void*)&acc->sex, sizeof(acc->sex))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_STRING, (void*)&acc->email, strlen(acc->email))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 5, SQLDT_INT,    (void*)&acc->expiration_time, sizeof(acc->expiration_time))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 6, SQLDT_STRING, (void*)&acc->last_ip, strlen(acc->last_ip))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}

	// insert new account regs
	if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , ? , ? );",  globalreg_db, acc->account_id) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}
	for( i = 0; i < acc->account_reg2_num; ++i )
	{
		if( SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (void*)acc->account_reg2[i].str, strlen(acc->account_reg2[i].str))
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)acc->account_reg2[i].value, strlen(acc->account_reg2[i].value))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}
	if( i < acc->account_reg2_num )
	{
		result = false;
		break;
	}

	// if we got this far, everything was successful
	result = true;

	} while(0);
	// finally

	result = ( SQL_SUCCESS == Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") );
	SqlStmt_Free(stmt);

	return result;
}

int convert_login(void)
{
	Sql* mysql_handle;
	int line_counter = 0;
	FILE *fp;
	char line[2048];
	struct mmo_account acc;

	mysql_handle = Sql_Malloc();
	if ( SQL_ERROR == Sql_Connect(mysql_handle, db_server_id, db_server_pw, db_server_ip, db_server_port, db_server_logindb) )
	{
		Sql_ShowDebug(mysql_handle);
		Sql_Free(mysql_handle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect: Success!\n");
	
	ShowStatus("Convert start...\n");
	fp = fopen(ACCOUNT_TXT_NAME,"r");
	if(fp == NULL)
		return 0;

	while( fgets(line,sizeof(line),fp) != NULL )
	{
		line_counter++;
		if(line[0]=='/' && line[1]=='/')
			continue;

		if( !acc_fromtxt(&acc, line) )
		{
			ShowWarning("Skipping incompatible data on line %d\n", line_counter);
			continue;
		}

		ShowInfo("Converting user (id: %d, name: %s, gm level: %d)\n", acc.account_id, acc.userid, acc.level);

		if( acc.account_id == 0 )
		{
			ShowError("Accounts with id '0' can not be converted to SQL!\n");
			continue;
		}

		if( !acc_tosql(&acc, mysql_handle) )
			ShowError("Conversion failed.\n");
	}

	//TODO: perhaps record the auto-increment value?

	fclose(fp);
	Sql_Free(mysql_handle);

	ShowStatus("Convert end...\n");

	return 0;
}

int login_config_read(const char* cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowStatus("Start reading interserver configuration: %s\n", cfgName);

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		i=sscanf(line,"%[^:]:%s", w1, w2);
		if(i!=2)
			continue;

		//add for DB connection
		if(strcmpi(w1,"db_server_ip")==0){
			strcpy(db_server_ip, w2);
			ShowStatus("set db_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_port")==0){
			db_server_port=atoi(w2);
			ShowStatus("set db_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_id")==0){
			strcpy(db_server_id, w2);
			ShowStatus("set db_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_pw")==0){
			strcpy(db_server_pw, w2);
			ShowStatus("set db_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_logindb")==0){
			strcpy(db_server_logindb, w2);
			ShowStatus("set db_server_logindb : %s\n",w2);
		}
		//support the import command, just like any other config
		else if(strcmpi(w1,"import")==0){
			login_config_read(w2);
		}
	}
	fclose(fp);
	ShowStatus("End reading interserver configuration...\n");
	return 0;
}

int do_init(int argc, char** argv)
{
	int input;
	login_config_read( (argc > 1) ? argv[1] : INTER_CONF_NAME );

	ShowInfo("\nWarning : Make sure you backup your databases before continuing!\n");
	ShowInfo("\nDo you wish to convert your Login Database to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y')
		convert_login();
	return 0;
}

void do_final(void)
{
}
