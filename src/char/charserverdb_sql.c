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

	if( db->initialized )
		return true;// already initialized

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

	if(
		db->accregdb->init(db->accregdb) &&
		db->charregdb->init(db->charregdb) &&
		db->castledb->init(db->castledb) &&
		db->chardb->init(db->chardb) &&
		db->frienddb->init(db->frienddb) &&
		db->guilddb->init(db->guilddb) &&
		db->homundb->init(db->homundb) &&
		db->hotkeydb->init(db->hotkeydb) &&
		db->partydb->init(db->partydb) &&
		db->petdb->init(db->petdb) &&
		db->questdb->init(db->questdb) &&
		rank_db_sql_init(db->rankdb) &&
		db->maildb->init(db->maildb) &&
		db->statusdb->init(db->statusdb)
	)
		db->initialized = true;

	return db->initialized;
}



/// Destroys this database engine, releasing all allocated memory (including itself).
static void charserver_db_sql_destroy(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	db->castledb->destroy(db->castledb);
	db->castledb = NULL;
	db->chardb->destroy(db->chardb);
	db->chardb = NULL;
	db->frienddb->destroy(db->frienddb);
	db->frienddb = NULL;
	db->guilddb->destroy(db->guilddb);
	db->guilddb = NULL;
	db->homundb->destroy(db->homundb);
	db->homundb = NULL;
	db->hotkeydb->destroy(db->hotkeydb);
	db->hotkeydb = NULL;
	db->partydb->destroy(db->partydb);
	db->partydb = NULL;
	db->petdb->destroy(db->petdb);
	db->petdb = NULL;
	db->questdb->destroy(db->questdb);
	db->questdb = NULL;
	rank_db_sql_destroy(db->rankdb);
	db->rankdb = NULL;
	db->maildb->destroy(db->maildb);
	db->maildb = NULL;
	db->statusdb->destroy(db->statusdb);
	db->statusdb = NULL;
	db->accregdb->destroy(db->accregdb);
	db->accregdb = NULL;
	db->charregdb->destroy(db->charregdb);
	db->charregdb = NULL;

	Sql_Free(db->sql_handle);
	db->sql_handle = NULL;
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
static CastleDB* charserver_db_sql_castledb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->castledb;
}



/// TODO
static CharDB* charserver_db_sql_chardb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->chardb;
}



/// TODO
static FriendDB* charserver_db_sql_frienddb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->frienddb;
}



/// TODO
static GuildDB* charserver_db_sql_guilddb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->guilddb;
}



/// TODO
static HomunDB* charserver_db_sql_homundb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->homundb;
}



/// TODO
static HotkeyDB* charserver_db_sql_hotkeydb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->hotkeydb;
}



/// TODO
static PartyDB* charserver_db_sql_partydb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->partydb;
}



/// TODO
static PetDB* charserver_db_sql_petdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->petdb;
}



/// TODO
static QuestDB* charserver_db_sql_questdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->questdb;
}



/// Returns the database interface that handles rankings.
static RankDB* charserver_db_sql_rankdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->rankdb;
}



/// TODO
static MailDB* charserver_db_sql_maildb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->maildb;
}



/// TODO
static StatusDB* charserver_db_sql_statusdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->statusdb;
}



/// TODO
static AccRegDB* charserver_db_sql_accregdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->accregdb;
}



/// TODO
static CharRegDB* charserver_db_sql_charregdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->charregdb;
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
	db->vtable.castledb     = charserver_db_sql_castledb;
	db->vtable.chardb       = charserver_db_sql_chardb;
	db->vtable.frienddb     = charserver_db_sql_frienddb;
	db->vtable.guilddb      = charserver_db_sql_guilddb;
	db->vtable.homundb      = charserver_db_sql_homundb;
	db->vtable.hotkeydb     = charserver_db_sql_hotkeydb;
	db->vtable.partydb      = charserver_db_sql_partydb;
	db->vtable.petdb        = charserver_db_sql_petdb;
	db->vtable.questdb      = charserver_db_sql_questdb;
	db->vtable.rankdb       = charserver_db_sql_rankdb;
	db->vtable.maildb       = charserver_db_sql_maildb;
	db->vtable.statusdb     = charserver_db_sql_statusdb;
	db->vtable.accregdb     = charserver_db_sql_accregdb;
	db->vtable.charregdb    = charserver_db_sql_charregdb;
	// TODO DB interfaces

	db->castledb = castle_db_sql(db);
	db->chardb = char_db_sql(db);
	db->frienddb = friend_db_sql(db);
	db->guilddb = guild_db_sql(db);
	db->homundb = homun_db_sql(db);
	db->hotkeydb = hotkey_db_sql(db);
	db->partydb = party_db_sql(db);
	db->petdb = pet_db_sql(db);
	db->questdb = quest_db_sql(db);
	db->rankdb = rank_db_sql(db);
	db->maildb = mail_db_sql(db);
	db->statusdb = status_db_sql(db);
	db->accregdb = accreg_db_sql(db);
	db->charregdb = charreg_db_sql(db);

	// initialize to default values
	db->sql_handle = NULL;
	db->initialized = false;

	// global sql settings
	safestrncpy(db->global_db_hostname, "127.0.0.1", sizeof(db->global_db_hostname));
	db->global_db_port = 3306;
	safestrncpy(db->global_db_username, "ragnarok", sizeof(db->global_db_username));
	safestrncpy(db->global_db_password, "ragnarok", sizeof(db->global_db_password));
	safestrncpy(db->global_db_database, "ragnarok", sizeof(db->global_db_database));
	safestrncpy(db->global_codepage, "", sizeof(db->global_codepage));
	// other settings
	safestrncpy(db->table_chars, "char", sizeof(db->table_chars));

	return &db->vtable;
}
