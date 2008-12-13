// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h>

static char   log_db_hostname[32] = "127.0.0.1";
static uint16 log_db_port = 3306;
static char   log_db_username[32] = "ragnarok";
static char   log_db_password[32] = "ragnarok";
static char   log_db_database[32] = "log";
static char   interlog_table[256] = "interlog";
static bool   log_inter_enabled = true;

static Sql* sql_handle = NULL;
static bool init_done = false;


/*=============================================
 * Records an event in the interserver log
 *---------------------------------------------*/
void interlog_log(const char* msg, ...)
{
	va_list ap;
	char message[255+1];
	char esc_message[255*2+1];

	if( !log_inter_enabled )
		return;
	if( !init_done )
		return;

	// prepare formatted message
	va_start(ap, msg);
	vsnprintf(message, sizeof(message)-1, msg, ap);
	va_end(ap);

	// escape message
	Sql_EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	// write log entry
	if( SQL_ERROR == Sql_Query(sql_handle,
	    "INSERT INTO `%s` (`time`, `log`) "
		"VALUES(NOW(), '%s'",
		interlog_table, esc_message) )
		Sql_ShowDebug(sql_handle);
}


bool interlog_init(void)
{
	sql_handle = Sql_Malloc();

	if( SQL_ERROR == Sql_Connect(sql_handle, log_db_username, log_db_password, log_db_hostname, log_db_port, log_db_database) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	init_done = true;

	return true;
}


bool interlog_final(void)
{
	Sql_Free(sql_handle);
	sql_handle = NULL;
	init_done = false;

	return true;
}


bool interlog_config_read(const char* key, const char* value)
{
	if( strcmpi(key, "log_inter") == 0 )
		log_inter_enabled = atoi(value);
	else
	if( strcmpi(key, "log_db_ip") == 0 )
		safestrncpy(log_db_hostname, value, sizeof(log_db_hostname));
	else
	if( strcmpi(key, "log_db_port") == 0 )
		log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		safestrncpy(log_db_username, value, sizeof(log_db_username));
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		safestrncpy(log_db_password, value, sizeof(log_db_password));
	else
	if( strcmpi(key, "log_db_db") == 0 )
		safestrncpy(log_db_database, value, sizeof(log_db_database));
	else
	if( strcmpi(key, "interlog_db") == 0 )
		safestrncpy(interlog_table, value, sizeof(interlog_table));
	else
		return false;

	return true;
}
