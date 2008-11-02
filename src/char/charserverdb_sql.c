// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"

#include <string.h>
#include <stdlib.h>



/// Initializes this database engine, making it ready for use.
static bool charserver_db_sql_init(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;
	Sql* sql_handle;
	const char* username;
	const char* password;
	const char* hostname;
	uint16      port;
	const char* database;
	const char* codepage;

	sql_handle = Sql_Malloc();
	username = db->global_db_username;
	password = db->global_db_password;
	hostname = db->global_db_hostname;
	port     = db->global_db_port;
	database = db->global_db_database;
	codepage = db->global_codepage;

	if( SQL_ERROR == Sql_Connect(sql_handle, username, password, hostname, port, database) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		return false;
	}

	db->sql_handle = sql_handle;
	if( codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, codepage) )
		Sql_ShowDebug(sql_handle);

	// TODO DB interfaces
	return db->chardb->init(db->chardb);
}



/// Destroys this database engine, releasing all allocated memory (including itself).
static void charserver_db_sql_destroy(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	db->chardb->destroy(db->chardb);
	Sql_Free(db->sql_handle);
	db->sql_handle = NULL;
	// TODO DB interfaces
	aFree(db);
}



/// Gets a property from this database engine.
static bool charserver_db_sql_get_property(CharServerDB* self, const char* key, char* buf, size_t buflen)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;
	const char* signature;

	signature = "engine.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "name") == 0 )
			safesnprintf(buf, buflen, "sql");
		else
		if( strcmpi(key, "version") == 0 )
			safesnprintf(buf, buflen, "%d", CHARSERVERDB_SQL_VERSION);
		else
		if( strcmpi(key, "comment") == 0 )
			safesnprintf(buf, buflen, "CharServerDB SQL engine");
		else
			return false;// not found
		return true;
	}

	signature = "sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_hostname);
		else
		if( strcmpi(key, "db_port") == 0 )
			safesnprintf(buf, buflen, "%d", db->global_db_port);
		else
		if( strcmpi(key, "db_username") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_username);
		else
		if(	strcmpi(key, "db_password") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_password);
		else
		if( strcmpi(key, "db_database") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_database);
		else
		if( strcmpi(key, "codepage") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_codepage);
		else
			return false;// not found
		return true;
	}

	// TODO DB interface properties

	return false;// not found
}



/// Sets a property in this database engine.
static bool charserver_db_sql_set_property(CharServerDB* self, const char* key, const char* value)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;
	const char* signature;


	signature = "sql.";
	if( strncmp(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(db->global_db_hostname, value, sizeof(db->global_db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			db->global_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(db->global_db_username, value, sizeof(db->global_db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(db->global_db_password, value, sizeof(db->global_db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(db->global_db_database, value, sizeof(db->global_db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(db->global_codepage, value, sizeof(db->global_codepage));
		else
			return false;// not found
		return true;
	}

	// TODO DB interface properties

	return false;// not found
}



/// TODO
static CharDB* charserver_db_sql_chardb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->chardb;
}



/// constructor
CharServerDB* charserver_db_sql(void)
{
	CharServerDB_SQL* db;

	CREATE(db, CharServerDB_SQL, 1);
	db->vtable.init         = charserver_db_sql_init;
	db->vtable.destroy      = charserver_db_sql_destroy;
	db->vtable.get_property = charserver_db_sql_get_property;
	db->vtable.set_property = charserver_db_sql_set_property;
	db->vtable.chardb       = charserver_db_sql_chardb;
	// TODO DB interfaces

	// initialize to default values
	db->sql_handle = NULL;
	db->chardb = char_db_sql(db);
	// global sql settings
	safestrncpy(db->global_db_hostname, "127.0.0.1", sizeof(db->global_db_hostname));
	db->global_db_port = 3306;
	safestrncpy(db->global_db_username, "ragnarok", sizeof(db->global_db_username));
	safestrncpy(db->global_db_password, "ragnarok", sizeof(db->global_db_password));
	safestrncpy(db->global_db_database, "ragnarok", sizeof(db->global_db_database));
	safestrncpy(db->global_codepage, "", sizeof(db->global_codepage));
	// other settings

	return &db->vtable;
}
