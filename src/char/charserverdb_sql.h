// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_SQL_H_
#define _CHARSERVERDB_SQL_H_

#include "../common/cbasetypes.h"
#include "../common/sql.h"
#include "charserverdb.h"



/// global defines
#define CHARSERVERDB_SQL_VERSION 20081028



typedef struct CharServerDB_SQL CharServerDB_SQL;
typedef struct CharDB_SQL CharDB_SQL;



/// internal structure
struct CharServerDB_SQL
{
	CharServerDB vtable;

	Sql* sql_handle;// SQL connection handle
	// TODO DB interfaces
	CharDB_SQL* chardb;

	// global sql settings
	char   global_db_hostname[32];
	uint16 global_db_port;
	char   global_db_username[32];
	char   global_db_password[32];
	char   global_db_database[32];
	char   global_codepage[32];
};

/// internal structure
struct CharDB_SQL
{
	CharDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* chars;       // SQL character storage

	// other settings
	bool case_sensitive;
	char char_db[32];

};



CharDB_SQL* char_db_sql(CharServerDB_SQL* owner);
bool char_db_sql_init(CharDB_SQL* self);
void char_db_sql_destroy(CharDB_SQL* self);



#endif /* _CHARSERVERDB_SQL_H_ */
