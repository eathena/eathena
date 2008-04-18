// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/md5calc.h"
#include "../common/sql.h"
#include "account.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for stat/lstat/fstat


// SQL-specifics
Sql* sql_handle;

// database parameters
uint16 login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";
char default_codepage[32] = "";

char login_db[256] = "login";
char loginlog_db[256] = "loginlog";

// temporary external imports
extern struct Login_Config login_config;


/*=============================================
 * Records an event in the login log
 *---------------------------------------------*/
void login_log(uint32 ip, const char* username, int rcode, const char* message)
{
	char esc_username[NAME_LENGTH*2+1];
	char esc_message[255*2+1];
	int retcode;

	if( !login_config.log_login )
		return;

	Sql_EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = Sql_Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		loginlog_db, ip2str(ip,NULL), esc_username, rcode, message);

	if( retcode != SQL_SUCCESS )
		Sql_ShowDebug(sql_handle);
}

/*=============================================
 * Does a mysql_ping to all connection handles
 *---------------------------------------------*/
int login_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	Sql_Ping(sql_handle);
	return 0;
}

int sql_ping_init(void)
{
	uint32 connection_timeout, connection_ping_interval;

	// set a default value first
	connection_timeout = 28800; // 8 hours

	// ask the mysql server for the timeout value
	if( SQL_SUCCESS == Sql_GetTimeout(sql_handle, &connection_timeout) && connection_timeout < 60 )
		connection_timeout = 60;

	// establish keepalive
	connection_ping_interval = connection_timeout - 30; // 30-second reserve
	add_timer_func_list(login_sql_ping, "login_sql_ping");
	add_timer_interval(gettick() + connection_ping_interval*1000, login_sql_ping, 0, 0, connection_ping_interval*1000);

	return 0;
}

//-----------------------------------------------------
// Initialize database connection
//-----------------------------------------------------
int mmo_db_init(void)
{
	ShowStatus("Login server init....\n");

	sql_handle = Sql_Malloc();

	// DB connection start
	ShowStatus("Connect Login Database Server....\n");
	if( SQL_ERROR == Sql_Connect(sql_handle, login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	else
	{
		ShowStatus("Connect success!\n");
	}

	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
		Sql_ShowDebug(sql_handle);

	sql_ping_init();

	return 0;
}


//-----------------------------------------------------
// close DB
//-----------------------------------------------------
void mmo_db_close(void)
{
	Sql_Free(sql_handle);
	sql_handle = NULL;
	ShowStatus("close DB connect....\n");
}


//-----------------------------------------------------
// IP-ban system
//-----------------------------------------------------
int login_ip_ban_check(uint32 ip)
{
	uint8* p = (uint8*)&ip;
	char* data = NULL;
	int matches;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `ipbanlist` WHERE `list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u'",
		p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
	{
		Sql_ShowDebug(sql_handle);
		// close connection because we can't verify their connectivity.
		return 1;
	}

	if( SQL_ERROR == Sql_NextRow(sql_handle) )
		return 1;// Shouldn't happen, but just in case...

	Sql_GetData(sql_handle, 0, &data, NULL);
	matches = atoi(data);
	Sql_FreeResult(sql_handle);

	if( matches == 0 )
		return 0;// No ban

	// ip ban ok.
	ShowInfo("Packet from banned ip : %u.%u.%u.%u\n", CONVIP(ip));
	login_log(ip, "unknown", -3, "ip banned");

	return 1;
}

void login_dynamic_ipban(const char* userid, int ip)
{
	if( login_config.dynamic_pass_failure_ban && login_config.log_login )
	{
		unsigned long failures = 0;

		char esc_userid[NAME_LENGTH*2+1];
		Sql_EscapeStringLen(sql_handle, esc_userid, userid, strlen(userid));

		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%u' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
			loginlog_db, ip, login_config.dynamic_pass_failure_ban_interval) )// how many times failed account? in one ip.
			Sql_ShowDebug(sql_handle);

		//check query result
		if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
		{
			char* data;
			Sql_GetData(sql_handle, 0, &data, NULL);
			failures = strtoul(data, NULL, 10);
			Sql_FreeResult(sql_handle);
		}
		if( failures >= login_config.dynamic_pass_failure_ban_limit )
		{
			uint8* p = (uint8*)&ip;
			if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%u.%u.%u.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration, esc_userid) )
				Sql_ShowDebug(sql_handle);
		}
	}
}

int ip_ban_flush(int tid, unsigned int tick, int id, int data)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()") )
		Sql_ShowDebug(sql_handle);

	return 0;
}


bool login_config_read_sql(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "ipban"))
		login_config.ipban = (bool)config_switch(w2);
	else if(!strcmpi(w1, "dynamic_pass_failure_ban"))
		login_config.dynamic_pass_failure_ban = (bool)config_switch(w2);
	else if(!strcmpi(w1, "dynamic_pass_failure_ban_interval"))
		login_config.dynamic_pass_failure_ban_interval = atoi(w2);
	else if(!strcmpi(w1, "dynamic_pass_failure_ban_limit"))
		login_config.dynamic_pass_failure_ban_limit = atoi(w2);
	else if(!strcmpi(w1, "dynamic_pass_failure_ban_duration"))
		login_config.dynamic_pass_failure_ban_duration = atoi(w2);
	else
		return false;

	return true;
}

bool inter_config_read_sql(const char* w1, const char* w2)
{
	if (!strcmpi(w1, "login_db"))
		strcpy(login_db, w2);
	else if (!strcmpi(w1, "login_server_ip"))
		strcpy(login_server_ip, w2);
	else if (!strcmpi(w1, "login_server_port"))
		login_server_port = (uint16)atoi(w2);
	else if (!strcmpi(w1, "login_server_id"))
		strcpy(login_server_id, w2);
	else if (!strcmpi(w1, "login_server_pw"))
		strcpy(login_server_pw, w2);
	else if (!strcmpi(w1, "login_server_db"))
		strcpy(login_server_db, w2);
	else if (!strcmpi(w1, "default_codepage"))
		strcpy(default_codepage, w2);
	else if (!strcmpi(w1, "loginlog_db"))
		strcpy(loginlog_db, w2);
	else
		return false;

	return true;
}
