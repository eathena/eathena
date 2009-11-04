// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_TXT_H_
#define _CHARSERVERDB_TXT_H_
/// \file
/// \extends charserver.h
/// Global header for the TXT database engine and interfaces.
/// Everything exposed by this header has protected access (only the engine and interface code can use it).


#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "charserverdb.h"


/// global defines
#define CHARSERVERDB_TXT_VERSION 20090210 // update this whenever the database format changes
#define CHARSERVERDB_AUTOSAVE_CHANGE_DELAY 1000
#define CHARSERVERDB_AUTOSAVE_RETRY_DELAY 5000
#define CHARSERVERDB_AUTOSAVE_MAX_DELAY 10000


typedef struct CharServerDB_TXT CharServerDB_TXT;


/// internal structure
struct CharServerDB_TXT
{
	CharServerDB vtable;

	/// For use by the database interfaces.
	/// @protected
	struct
	{
		/// Requests a sync.
		/// Called when data is changed in one of the database interfaces.
		///
		/// @param self Database engine
		void (*request_sync)(CharServerDB_TXT* self);
	} p;

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
	MemoDB* memodb;
	AuctionDB* auctiondb;
	StatusDB* statusdb;
	SkillDB* skilldb;
	StorageDB* storagedb;
	AccRegDB* accregdb;
	CharRegDB* charregdb;

	bool initialized;
	unsigned int dirty_tick;//< when it first became dirty
	int sync_timer;//< triggers sync attempts

	// settings
	int autosave_change_delay;
	int autosave_retry_delay;
	int autosave_max_delay;
	char file_accregs[256];
	char file_auctions[256];
	char file_carts[256];
	char file_castles[256];
	char file_chars[256];
	char file_charregs[256];
	char file_friends[256];
	char file_guilds[256];
	char file_guild_storages[256];
	char file_homuns[256];
	char file_hotkeys[256];
	char file_inventories[256];
	char file_mails[256];
	char file_memos[256];
	char file_mercenaries[256];
	char file_parties[256];
	char file_pets[256];
	char file_quests[256];
	char file_skills[256];
	char file_statuses[256];
	char file_storages[256];
	char file_ranks[256];
};


#endif /* _CHARSERVERDB_TXT_H_ */
