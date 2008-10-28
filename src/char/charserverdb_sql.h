// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_SQL_H_
#define _CHARSERVERDB_SQL_H_

#include "charserverdb.h"
#include "../common/sql.h"



/// global defines
#define CHARSERVERDB_SQL_VERSION 20081028



typedef struct CharServerDB_SQL CharServerDB_SQL;



/// internal structure
struct CharServerDB_SQL
{
	CharServerDB vtable;

	Sql* handle;// SQL connection handle

	// global sql settings
	char   global_db_hostname[32];
	uint16 global_db_port;
	char   global_db_username[32];
	char   global_db_password[32];
	char   global_db_database[32];
	char   global_codepage[32];
};

#endif /* _CHARSERVERDB_SQL_H_ */
