// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h" // CREATE()
#include "../common/mmo.h" // NAME_LENGTH
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charlogdb.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // strtoul()
#include <string.h>


typedef struct CharLogDB_SQL CharLogDB_SQL;


/// internal structure
struct CharLogDB_SQL
{
	// public interface
	CharLogDB vtable;

	// state
	Sql* sql_handle;
	bool initialized;

	// settings
	char db_hostname[32];
	uint16 db_port;
	char db_username[32];
	char db_password[32];
	char db_database[32];
	char table_charlog[256];
	bool enabled;
};


static bool charlog_db_sql_init(CharLogDB* self)
{
	CharLogDB_SQL* db = (CharLogDB_SQL*)self;

	db->sql_handle = Sql_Malloc();

	if( SQL_ERROR == Sql_Connect(db->sql_handle, db->db_username, db->db_password, db->db_hostname, db->db_port, db->db_database) )
	{
		Sql_ShowDebug(db->sql_handle);
		Sql_Free(db->sql_handle);
		db->sql_handle = NULL;
		db->initialized = false;
		return false;
	}

	db->initialized = true;

	return true;
}


static void charlog_db_sql_destroy(CharLogDB* self)
{
	CharLogDB_SQL* db = (CharLogDB_SQL*)self;
	Sql_Free(db->sql_handle);
	aFree(db);
}


static bool charlog_db_sql_get_property(CharLogDB* self, const char* key, char* buf, size_t buflen)
{
	CharLogDB_SQL* db = (CharLogDB_SQL*)self;

	if( strcmpi(key, "engine.name") == 0 )
		safesnprintf(buf, buflen, "sql");
	else
	if( strcmpi(key, "charlog.sql.table") == 0 )
		safesnprintf(buf, buflen, "%s", db->table_charlog);
	else
		return false;

	return true;
}


static bool charlog_db_sql_set_property(CharLogDB* self, const char* key, const char* value)
{
	CharLogDB_SQL* db = (CharLogDB_SQL*)self;

	if( strcmpi(key, "log_db_ip") == 0 )
		safestrncpy(db->db_hostname, value, sizeof(db->db_hostname));
	else
	if( strcmpi(key, "log_db_port") == 0 )
		db->db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		safestrncpy(db->db_username, value, sizeof(db->db_username));
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		safestrncpy(db->db_password, value, sizeof(db->db_password));
	else
	if( strcmpi(key, "log_db_db") == 0 )
		safestrncpy(db->db_database, value, sizeof(db->db_database));
	else
	if( strcmpi(key, "charlog.sql.table") == 0 )
		safestrncpy(db->table_charlog, value, sizeof(db->table_charlog));
	else
		return false;

	return true;
}


static bool charlog_db_sql_log(CharLogDB* self, int char_id, int account_id, int slot, const char* name, const char* msg, va_list ap)
{
	CharLogDB_SQL* db = (CharLogDB_SQL*)self;

	char message[255+1];
	char esc_message[sizeof(message)*2+1];
	char esc_name[NAME_LENGTH*2+1];

	if( !db->initialized )
		return false;

	// prepare formatted message
	vsnprintf(message, sizeof(message)-1, msg, ap);

	// escape message
	Sql_EscapeStringLen(db->sql_handle, esc_message, message, strnlen(message, sizeof(message)-1));

	// escape name
	Sql_EscapeStringLen(db->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));

	// write log entry
	if( SQL_ERROR == Sql_Query(db->sql_handle,
	    "INSERT INTO `%s` (`time`, `char_id`, `account_id`, `slot`, `name`, `message`) VALUES(NOW(), '%d', '%d', '%d', '%s', '%s')",
		db->table_charlog, char_id, account_id, slot, esc_name, esc_message) )
		Sql_ShowDebug(db->sql_handle);

	return true;
}


CharLogDB* charlog_db_sql(void)
{
	CharLogDB_SQL* db;

	CREATE(db, CharLogDB_SQL, 1);
	db->vtable.init         = charlog_db_sql_init;
	db->vtable.destroy      = charlog_db_sql_destroy;
	db->vtable.get_property = charlog_db_sql_get_property;
	db->vtable.set_property = charlog_db_sql_set_property;
	db->vtable.log          = charlog_db_sql_log;

	// initial state
	db->sql_handle = NULL;
	db->initialized = false;

	// default settings
	safestrncpy(db->db_hostname, "127.0.0.1", sizeof(db->db_hostname));
	db->db_port = 3306;
	safestrncpy(db->db_username, "ragnarok", sizeof(db->db_username));
	safestrncpy(db->db_password, "ragnarok", sizeof(db->db_password));
	safestrncpy(db->db_database, "log", sizeof(db->db_database));
	safestrncpy(db->table_charlog, "charlog", sizeof(db->table_charlog));
	db->enabled = true;

	return &db->vtable;
}
