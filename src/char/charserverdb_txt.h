// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_TXT_H_
#define _CHARSERVERDB_TXT_H_
/// \file
/// \extends charserver.h
/// Global header for the TXT database engine and interfaces.
/// Everything exposed by this header has protected access (only the engine and interface code can use it).

// TODO in Aegis when you are ranked and change job, the rank points in that rank remain intact 
//      and are continued whenever we change back to the same kind of job (they are kept separate from char)
// TODO char.fame => famedb.txt {int rank_id, int char_id, int points}.*

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "charserverdb.h"



/// global defines
#define CHARSERVERDB_TXT_VERSION 20081028



typedef struct CharServerDB_TXT CharServerDB_TXT;



/// internal structure
struct CharServerDB_TXT
{
	CharServerDB vtable;

	// TODO DB interfaces
	CastleDB* castledb;
	CharDB* chardb;
	FriendDB* frienddb;
	GuildDB* guilddb;
	HomunDB* homundb;
	HotkeyDB* hotkeydb;
	PartyDB* partydb;
	PetDB* petdb;
	QuestDB* questdb;
	RankDB* rankdb;
	MailDB* maildb;
	AuctionDB* auctiondb;
	StatusDB* statusdb;
	AccRegDB* accregdb;
	CharRegDB* charregdb;

	bool initialized;

	// settings
	char file_accregs[256];
	char file_auctions[256];
	char file_castles[256];
	char file_chars[256];
	char file_friends[256];
	char file_guilds[256];
	char file_guild_storages[256];
	char file_homuns[256];
	char file_hotkeys[256];
	char file_mails[256];
	char file_parties[256];
	char file_pets[256];
	char file_quests[256];
	char file_statuses[256];
	char file_storages[256];
};



CastleDB* castle_db_txt(CharServerDB_TXT* owner);
CharDB* char_db_txt(CharServerDB_TXT* owner);
FriendDB* friend_db_txt(CharServerDB_TXT* owner);
GuildDB* guild_db_txt(CharServerDB_TXT* owner);
HomunDB* homun_db_txt(CharServerDB_TXT* owner);
HotkeyDB* hotkey_db_txt(CharServerDB_TXT* owner);
PartyDB* party_db_txt(CharServerDB_TXT* owner);
PetDB* pet_db_txt(CharServerDB_TXT* owner);
QuestDB* quest_db_txt(CharServerDB_TXT* owner);
MailDB* mail_db_txt(CharServerDB_TXT* owner);
AuctionDB* auction_db_txt(CharServerDB_TXT* owner);
StatusDB* status_db_txt(CharServerDB_TXT* owner);
AccRegDB* accreg_db_txt(CharServerDB_TXT* owner);
CharRegDB* charreg_db_txt(CharServerDB_TXT* owner);

RankDB* rank_db_txt(CharServerDB_TXT* owner);
bool    rank_db_txt_init(RankDB* self);
void    rank_db_txt_destroy(RankDB* self);



#endif /* _CHARSERVERDB_TXT_H_ */
