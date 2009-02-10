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
		db->guildstoragedb->init(db->guildstoragedb) &&
		db->homundb->init(db->homundb) &&
		db->hotkeydb->init(db->hotkeydb) &&
		db->partydb->init(db->partydb) &&
		db->petdb->init(db->petdb) &&
		db->questdb->init(db->questdb) &&
		db->auctiondb->init(db->auctiondb) &&
		db->rankdb->init(db->rankdb) &&
		db->maildb->init(db->maildb) &&
		db->statusdb->init(db->statusdb) &&
		db->storagedb->init(db->storagedb)
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
	db->guildstoragedb->destroy(db->guildstoragedb);
	db->guildstoragedb = NULL;
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
	db->auctiondb->destroy(db->auctiondb);
	db->auctiondb = NULL;
	db->rankdb->destroy(db->rankdb);
	db->rankdb = NULL;
	db->maildb->destroy(db->maildb);
	db->maildb = NULL;
	db->statusdb->destroy(db->statusdb);
	db->statusdb = NULL;
	db->storagedb->destroy(db->storagedb);
	db->storagedb= NULL;
	db->accregdb->destroy(db->accregdb);
	db->accregdb = NULL;
	db->charregdb->destroy(db->charregdb);
	db->charregdb = NULL;

	Sql_Free(db->sql_handle);
	db->sql_handle = NULL;
	aFree(db);
}



/// Flushes all in-memory data to secondary storage.
static bool charserver_db_sql_sync(CharServerDB* self)
{
	return true; //not needed for this engine
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

		// table names
		if( strcmpi(key, "auction_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_auctions);
		else
		if( strcmpi(key, "cart_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_carts);
		else
		if( strcmpi(key, "guild_castle_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_castles);
		else
		if( strcmpi(key, "char_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_chars);
		else
		if( strcmpi(key, "friend_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_friends);
		else
		if( strcmpi(key, "guild_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guilds);
		else
		if( strcmpi(key, "guild_alliance_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_alliances);
		else
		if( strcmpi(key, "guild_expulsion_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_expulsions);
		else
		if( strcmpi(key, "guild_member_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_members);
		else
		if( strcmpi(key, "guild_position_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_positions);
		else
		if( strcmpi(key, "guild_skill_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_skills);
		else
		if( strcmpi(key, "guild_storage") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_storages);
		else
		if( strcmpi(key, "homun_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_homuns);
		else
		if( strcmpi(key, "homun_skill_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_homun_skills);
		else
		if( strcmpi(key, "hotkey_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_hotkeys);
		else
		if( strcmpi(key, "inventory_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_inventories);
		else
		if( strcmpi(key, "mail_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_mails);
		else
		if( strcmpi(key, "memo_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_memos);
		else
		if( strcmpi(key, "party_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_parties);
		else
		if( strcmpi(key, "pet_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_pets);
		else
		if( strcmpi(key, "quest_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_quests);
		else
		if( strcmpi(key, "quest_obj_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_quest_objectives);
		else
		if( strcmpi(key, "table_ranks") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_ranks);
		else
		if( strcmpi(key, "reg_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_registry);
		else
		if( strcmpi(key, "scdata_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_statuses);
		else
		if( strcmpi(key, "skill_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_skills);
		else
		if( strcmpi(key, "storage_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_storages);

		else
			return false;// not found
		return true;
	}

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

		// table names
		if( strcmpi(key, "auction_db") == 0 )
			safestrncpy(db->table_auctions, value, sizeof(db->table_auctions));
		else
		if( strcmpi(key, "cart_db") == 0 )
			safestrncpy(db->table_carts, value, sizeof(db->table_carts));
		else
		if( strcmpi(key, "guild_castle_db") == 0 )
			safestrncpy(db->table_castles, value, sizeof(db->table_castles));
		else
		if( strcmpi(key, "char_db") == 0 )
			safestrncpy(db->table_chars, value, sizeof(db->table_chars));
		else
		if( strcmpi(key, "friend_db") == 0 )
			safestrncpy(db->table_friends, value, sizeof(db->table_friends));
		else
		if( strcmpi(key, "guild_db") == 0 )
			safestrncpy(db->table_guilds, value, sizeof(db->table_guilds));
		else
		if( strcmpi(key, "guild_alliance") == 0 )
			safestrncpy(db->table_guild_alliances, value, sizeof(db->table_guild_alliances));
		else
		if( strcmpi(key, "guild_expulsion") == 0 )
			safestrncpy(db->table_guild_expulsions, value, sizeof(db->table_guild_expulsions));
		else
		if( strcmpi(key, "guild_member") == 0 )
			safestrncpy(db->table_guild_members, value, sizeof(db->table_guild_members));
		else
		if( strcmpi(key, "guild_position") == 0 )
			safestrncpy(db->table_guild_positions, value, sizeof(db->table_guild_positions));
		else
		if( strcmpi(key, "guild_skill") == 0 )
			safestrncpy(db->table_guild_skills, value, sizeof(db->table_guild_skills));
		else
		if( strcmpi(key, "guild_storage") == 0 )
			safestrncpy(db->table_guild_storages, value, sizeof(db->table_guild_storages));
		else
		if( strcmpi(key, "homun_db") == 0 )
			safestrncpy(db->table_homuns, value, sizeof(db->table_homuns));
		else
		if( strcmpi(key, "homun_skill_db") == 0 )
			safestrncpy(db->table_homun_skills, value, sizeof(db->table_homun_skills));
		else
		if( strcmpi(key, "hotkey_db") == 0 )
			safestrncpy(db->table_hotkeys, value, sizeof(db->table_hotkeys));
		else
		if( strcmpi(key, "inventory_db") == 0 )
			safestrncpy(db->table_inventories, value, sizeof(db->table_inventories));
		else
		if( strcmpi(key, "mail_db") == 0 )
			safestrncpy(db->table_mails, value, sizeof(db->table_mails));
		else
		if( strcmpi(key, "memo_db") == 0 )
			safestrncpy(db->table_memos, value, sizeof(db->table_memos));
		else
		if( strcmpi(key, "party_db") == 0 )
			safestrncpy(db->table_parties, value, sizeof(db->table_parties));
		else
		if( strcmpi(key, "pet_db") == 0 )
			safestrncpy(db->table_pets, value, sizeof(db->table_pets));
		else
		if( strcmpi(key, "quest_db") == 0 )
			safestrncpy(db->table_quests, value, sizeof(db->table_quests));
		else
		if( strcmpi(key, "quest_obj_db") == 0 )
			safestrncpy(db->table_quest_objectives, value, sizeof(db->table_quest_objectives));
		else
		if( strcmpi(key, "table_ranks") == 0 )
			safestrncpy(db->table_ranks, value, sizeof(db->table_ranks));
		else
		if( strcmpi(key, "reg_db") == 0 )
			safestrncpy(db->table_registry, value, sizeof(db->table_registry));
		else
		if( strcmpi(key, "scdata_db") == 0 )
			safestrncpy(db->table_statuses, value, sizeof(db->table_statuses));
		else
		if( strcmpi(key, "skill_db") == 0 )
			safestrncpy(db->table_skills, value, sizeof(db->table_skills));
		else
		if( strcmpi(key, "storage_db") == 0 )
			safestrncpy(db->table_storages, value, sizeof(db->table_storages));

		else
			return false;// not found
		return true;
	}

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
static GuildStorageDB* charserver_db_sql_guildstoragedb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->guildstoragedb;
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
static AuctionDB* charserver_db_sql_auctiondb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->auctiondb;
}



/// TODO
static StatusDB* charserver_db_sql_statusdb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->statusdb;
}



/// TODO
static StorageDB* charserver_db_sql_storagedb(CharServerDB* self)
{
	CharServerDB_SQL* db = (CharServerDB_SQL*)self;

	return db->storagedb;
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
	db->vtable.sync         = charserver_db_sql_sync;
	db->vtable.get_property = charserver_db_sql_get_property;
	db->vtable.set_property = charserver_db_sql_set_property;
	db->vtable.castledb     = charserver_db_sql_castledb;
	db->vtable.chardb       = charserver_db_sql_chardb;
	db->vtable.frienddb     = charserver_db_sql_frienddb;
	db->vtable.guilddb      = charserver_db_sql_guilddb;
	db->vtable.guildstoragedb = charserver_db_sql_guildstoragedb;
	db->vtable.homundb      = charserver_db_sql_homundb;
	db->vtable.hotkeydb     = charserver_db_sql_hotkeydb;
	db->vtable.partydb      = charserver_db_sql_partydb;
	db->vtable.petdb        = charserver_db_sql_petdb;
	db->vtable.questdb      = charserver_db_sql_questdb;
	db->vtable.rankdb       = charserver_db_sql_rankdb;
	db->vtable.maildb       = charserver_db_sql_maildb;
	db->vtable.auctiondb    = charserver_db_sql_auctiondb;
	db->vtable.statusdb     = charserver_db_sql_statusdb;
	db->vtable.storagedb    = charserver_db_sql_storagedb;
	db->vtable.accregdb     = charserver_db_sql_accregdb;
	db->vtable.charregdb    = charserver_db_sql_charregdb;
	// TODO DB interfaces

	db->castledb = castle_db_sql(db);
	db->chardb = char_db_sql(db);
	db->frienddb = friend_db_sql(db);
	db->guilddb = guild_db_sql(db);
	db->guildstoragedb = guildstorage_db_sql(db);
	db->homundb = homun_db_sql(db);
	db->hotkeydb = hotkey_db_sql(db);
	db->partydb = party_db_sql(db);
	db->petdb = pet_db_sql(db);
	db->questdb = quest_db_sql(db);
	db->rankdb = rank_db_sql(db);
	db->maildb = mail_db_sql(db);
	db->auctiondb = auction_db_sql(db);
	db->statusdb = status_db_sql(db);
	db->storagedb = storage_db_sql(db);
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
	safestrncpy(db->table_auctions, "auction", sizeof(db->table_auctions));
	safestrncpy(db->table_carts, "cart_inventory", sizeof(db->table_carts));
	safestrncpy(db->table_castles, "guild_castle", sizeof(db->table_castles));
	safestrncpy(db->table_chars, "char", sizeof(db->table_chars));
	safestrncpy(db->table_friends, "friends", sizeof(db->table_friends));
	safestrncpy(db->table_guilds, "guild", sizeof(db->table_guilds));
	safestrncpy(db->table_guild_alliances, "guild_alliance", sizeof(db->table_guild_alliances));
	safestrncpy(db->table_guild_expulsions, "guild_expulsion", sizeof(db->table_guild_expulsions));
	safestrncpy(db->table_guild_members, "guild_member", sizeof(db->table_guild_members));
	safestrncpy(db->table_guild_positions, "guild_position", sizeof(db->table_guild_positions));
	safestrncpy(db->table_guild_skills, "guild_skill", sizeof(db->table_guild_skills));
	safestrncpy(db->table_guild_storages, "guild_storage", sizeof(db->table_guild_storages));
	safestrncpy(db->table_homuns, "homunculus", sizeof(db->table_homuns));
	safestrncpy(db->table_homun_skills, "skill_homunculus", sizeof(db->table_homun_skills));
	safestrncpy(db->table_hotkeys, "hotkey", sizeof(db->table_hotkeys));
	safestrncpy(db->table_inventories, "inventory", sizeof(db->table_inventories));
	safestrncpy(db->table_mails, "mail", sizeof(db->table_mails));
	safestrncpy(db->table_memos, "memo", sizeof(db->table_memos));
	safestrncpy(db->table_parties, "party", sizeof(db->table_parties));
	safestrncpy(db->table_pets, "pet", sizeof(db->table_pets));
	safestrncpy(db->table_quests, "quest", sizeof(db->table_quests));
	safestrncpy(db->table_quest_objectives, "quest_objective", sizeof(db->table_quest_objectives));
	safestrncpy(db->table_ranks, "ranks", sizeof(db->table_ranks));
	safestrncpy(db->table_registry, "global_reg_value", sizeof(db->table_registry));
	safestrncpy(db->table_skills, "skill", sizeof(db->table_skills));
	safestrncpy(db->table_statuses, "sc_data", sizeof(db->table_statuses));
	safestrncpy(db->table_storages, "storage", sizeof(db->table_storages));

	return &db->vtable;
}
