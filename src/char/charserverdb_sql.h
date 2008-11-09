// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_SQL_H_
#define _CHARSERVERDB_SQL_H_
/// \file
/// \extends charserver.h
/// Global header for the SQL database engine and interfaces.
/// Everything exposed by this header has protected access (only the engine and interface code can use it).

// TODO in Aegis when you are ranked and change job, the rank points in that rank remain intact 
//      and are continued whenever we change back to the same kind of job (they are kept separate from char)
// TODO char.fame => TABLE `ranks` (`rank_id` INT NOT NULL, `char_id` INT NOT NULL, `points` INT NOT NULL, UNIQUE (`rank_id`,`char_id`))

#include "../common/cbasetypes.h"
#include "../common/sql.h"
#include "charserverdb.h"



/// global defines
#define CHARSERVERDB_SQL_VERSION 20081028



typedef struct CharServerDB_SQL CharServerDB_SQL;



/// internal structure
struct CharServerDB_SQL
{
	CharServerDB vtable;

	Sql* sql_handle;// SQL connection handle
	bool initialized;
	// TODO DB interfaces
	CastleDB* castledb;
	CharDB* chardb;
	GuildDB* guilddb;
	HomunDB* homundb;
	PetDB* petdb;
	RankDB* rankdb;

	// global sql settings
	char   global_db_hostname[32];
	uint16 global_db_port;
	char   global_db_username[32];
	char   global_db_password[32];
	char   global_db_database[32];
	char   global_codepage[32];
	// settings
	char table_chars[256];
};


CastleDB* castle_db_sql(CharServerDB_SQL* owner);
CharDB* char_db_sql(CharServerDB_SQL* owner);
GuildDB* guild_db_sql(CharServerDB_SQL* owner);
HomunDB* homun_db_sql(CharServerDB_SQL* owner);
PetDB* pet_db_sql(CharServerDB_SQL* owner);

RankDB* rank_db_sql(CharServerDB_SQL* owner);
bool    rank_db_sql_init(RankDB* self);
void    rank_db_sql_destroy(RankDB* self);


#endif /* _CHARSERVERDB_SQL_H_ */
