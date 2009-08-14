// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_SQL_H_
#define _CHARSERVERDB_SQL_H_
/// \file
/// \extends charserver.h
/// Global header for the SQL database engine and interfaces.
/// Everything exposed by this header has protected access (only the engine and interface code can use it).

#include "../common/cbasetypes.h"
#include "../common/sql.h"
#include "charserverdb.h"



/// global defines
#define CHARSERVERDB_SQL_VERSION 20090210 // update this whenever the database format changes



typedef struct CharServerDB_SQL CharServerDB_SQL;



/// internal structure
struct CharServerDB_SQL
{
	CharServerDB vtable;

	// TODO DB interfaces
	CastleDB* castledb;
	CharDB* chardb;
	FriendDB* frienddb;
	GuildDB* guilddb;
	HomunDB* homundb;
	MercDB* mercdb;
	HotkeyDB* hotkeydb;
	PartyDB* partydb;
	PetDB* petdb;
	QuestDB* questdb;
	RankDB* rankdb;
	MailDB* maildb;
	AuctionDB* auctiondb;
	StatusDB* statusdb;
	StorageDB* storagedb;
	AccRegDB* accregdb;
	CharRegDB* charregdb;

	Sql* sql_handle;// SQL connection handle
	bool initialized;

	// global sql settings
	char   global_db_hostname[32];
	uint16 global_db_port;
	char   global_db_username[32];
	char   global_db_password[32];
	char   global_db_database[32];
	char   global_codepage[32];

	// settings
	char table_auctions[256];
	char table_carts[256];
	char table_castles[256];
	char table_chars[256];
	char table_friends[256];
	char table_guilds[256];
	char table_guild_alliances[256];
	char table_guild_expulsions[256];
	char table_guild_members[256];
	char table_guild_positions[256];
	char table_guild_skills[256];
	char table_guild_storages[256];
	char table_homuns[256];
	char table_homun_skills[256];
	char table_hotkeys[256];
	char table_inventories[256];
	char table_mails[256];
	char table_memos[256];
	char table_mercenaries[256];
	char table_mercenary_owners[256];
	char table_parties[256];
	char table_pets[256];
	char table_quests[256];
	char table_ranks[256];
	char table_registry[256];
	char table_skills[256];
	char table_statuses[256];
	char table_storages[256];
};



// generic sql db iterator constructor
extern CSDBIterator* csdb_sql_iterator(Sql* sql_handle, const char* sql_table, const char* sql_column);



#endif /* _CHARSERVERDB_SQL_H_ */
