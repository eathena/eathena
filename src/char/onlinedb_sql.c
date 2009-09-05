// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "onlinedb.h"
#include <stdlib.h> // strtoul()
#include <string.h> // strcmpi()


/// Internal structure.
typedef struct OnlineDB_SQL
{
	// public interface
	OnlineDB vtable;

	// state
	Sql* sql_handle;
	bool initialized;

	// settings
	char db_hostname[32];
	uint16 db_port;
	char db_username[32];
	char db_password[32];
	char db_database[32];
	char table_chars[256];

} OnlineDB_SQL;


/// @protected
static bool online_db_sql_init(OnlineDB* self)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;

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


/// @protected
static void online_db_sql_destroy(OnlineDB* self)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;
	Sql_Free(db->sql_handle);
	aFree(db);
}


/// @protected
static bool online_db_sql_sync(OnlineDB* self)
{
	return true;
}


/// @protected
static bool online_db_sql_get_property(OnlineDB* self, const char* key, char* buf, size_t buflen)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;

	if( strcmpi(key, "engine.name") == 0 )
		safesnprintf(buf, buflen, "sql");
	else
		return false;

	return true;
}


/// @protected
static bool online_db_sql_set_property(OnlineDB* self, const char* key, const char* value)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;

	if( strcmpi(key, "sql.db_hostname") == 0 )
		safestrncpy(db->db_hostname, value, sizeof(db->db_hostname));
	else
	if( strcmpi(key, "sql.db_port") == 0 )
		db->db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "sql.db_username") == 0 )
		safestrncpy(db->db_username, value, sizeof(db->db_username));
	else
	if( strcmpi(key, "sql.db_password") == 0 )
		safestrncpy(db->db_password, value, sizeof(db->db_password));
	else
	if( strcmpi(key, "sql.db_database") == 0 )
		safestrncpy(db->db_database, value, sizeof(db->db_database));
	else
	if( strcmpi(key, "sql.char_db") == 0 )
		safestrncpy(db->table_chars, value, sizeof(db->table_chars));
	else
		return false;

	return true;
}


/// @protected
static bool online_db_sql_set_online(OnlineDB* self, int account_id, int char_id)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;

	if( SQL_ERROR == Sql_Query(db->sql_handle, "UPDATE `%s` SET `online`='1' WHERE `char_id`='%d'", db->table_chars, char_id) )
	{
		Sql_ShowDebug(db->sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool online_db_sql_set_offline(OnlineDB* self, int account_id, int char_id)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)self;

	if( char_id != -1 )
	{// just this character
		if( SQL_ERROR == Sql_Query(db->sql_handle, "UPDATE `%s` SET `online`='0' WHERE `char_id`='%d'", db->table_chars, char_id) )
		{
			Sql_ShowDebug(db->sql_handle);
			return false;
		}
	}
	else
	if( account_id != -1 )
	{// all characters on this account
		if( SQL_ERROR == Sql_Query(db->sql_handle, "UPDATE `%s` SET `online`='0' WHERE `account_id`='%d'", db->table_chars, account_id) )
		{
			Sql_ShowDebug(db->sql_handle);
			return false;
		}
	}
	else
	{// whole database
		if( SQL_ERROR == Sql_Query(db->sql_handle, "UPDATE `%s` SET `online`='0'", db->table_chars, char_id) )
		{
			Sql_ShowDebug(db->sql_handle);
			return false;
		}
	}

	return true;
}


/// Constructs a new OnlineDB interface.
/// @protected
OnlineDB* online_db_sql(void)
{
	OnlineDB_SQL* db = (OnlineDB_SQL*)aCalloc(1, sizeof(OnlineDB_SQL));

	// set up the vtable
	db->vtable.init         = &online_db_sql_init;
	db->vtable.destroy      = &online_db_sql_destroy;
	db->vtable.sync         = &online_db_sql_sync;
	db->vtable.get_property = &online_db_sql_get_property;
	db->vtable.set_property = &online_db_sql_set_property;
	db->vtable.set_online   = &online_db_sql_set_online;
	db->vtable.set_offline  = &online_db_sql_set_offline;

	// initial state
	db->sql_handle = NULL;
	db->initialized = false;

	// default settings
	safestrncpy(db->db_hostname, "127.0.0.1", sizeof(db->db_hostname));
	db->db_port = 3306;
	safestrncpy(db->db_username, "ragnarok", sizeof(db->db_username));
	safestrncpy(db->db_password, "ragnarok", sizeof(db->db_password));
	safestrncpy(db->db_database, "ragnarok", sizeof(db->db_database));
	safestrncpy(db->table_chars, "char", sizeof(db->table_chars));

	return &db->vtable;
}
