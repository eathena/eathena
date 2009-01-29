// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"

#include <string.h>
#include <stdlib.h>



/// Initializes this database engine, making it ready for use.
static bool charserver_db_txt_init(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	if( db->initialized )
		return true;

	// dependencies: charregdb < chardb
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
		db->auctiondb->init(db->auctiondb) &&
		rank_db_txt_init(db->rankdb) &&
		db->maildb->init(db->maildb) &&
		db->statusdb->init(db->statusdb)
	)
		db->initialized = true;

	return db->initialized;
}



/// Destroys this database engine, releasing all allocated memory (including itself).
static void charserver_db_txt_destroy(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

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
	db->auctiondb->destroy(db->auctiondb);
	db->auctiondb = NULL;
	rank_db_txt_destroy(db->rankdb);
	db->rankdb = NULL;
	db->maildb->destroy(db->maildb);
	db->maildb = NULL;
	db->statusdb->destroy(db->statusdb);
	db->statusdb = NULL;
	db->accregdb->destroy(db->accregdb);
	db->accregdb = NULL;
	db->charregdb->destroy(db->charregdb);
	db->charregdb = NULL;

	aFree(db);
}



/// Gets a property from this database engine.
static bool charserver_db_txt_get_property(CharServerDB* self, const char* key, char* buf, size_t buflen)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;
	const char* signature;

	signature = "engine.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "name") == 0 )
			safesnprintf(buf, buflen, "txt");
		else
		if( strcmpi(key, "version") == 0 )
			safesnprintf(buf, buflen, "%d", CHARSERVERDB_TXT_VERSION);
		else
		if( strcmpi(key, "comment") == 0 )
			safesnprintf(buf, buflen, "CharServerDB TXT engine");
		else
			return false;// not found
		return true;
	}

	signature = "txt.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);

		// savefile paths
		if( strcmpi(key, "accreg_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_accregs);
		else
		if( strcmpi(key, "auction_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_auctions);
		else
		if( strcmpi(key, "castle_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_castles);
		else
		if( strcmpi(key, "athena_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_chars);
		else
		if( strcmpi(key, "friends_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_friends);
		else
		if( strcmpi(key, "guild_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_guilds);
		else
		if( strcmpi(key, "guild_storage_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_guild_storages);
		else
		if( strcmpi(key, "homun_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_homuns);
		else
		if( strcmpi(key, "hotkeys_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_hotkeys);
		else
		if( strcmpi(key, "mail_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_mails);
		else
		if( strcmpi(key, "party_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_parties);
		else
		if( strcmpi(key, "pet_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_pets);
		else
		if( strcmpi(key, "quest_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_quests);
		else
		if( strcmpi(key, "scdata_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_statuses);
		else
		if( strcmpi(key, "storage_txt") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_storages);

		else
			return false;// not found
		return true;
	}

	return false;// not found
}



/// Sets a property in this database engine.
static bool charserver_db_txt_set_property(CharServerDB* self, const char* key, const char* value)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;
	const char* signature;


	signature = "txt.";
	if( strncmp(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);

		// savefile paths
		if( strcmpi(key, "accreg_txt") == 0 )
			safestrncpy(db->file_accregs, value, sizeof(db->file_accregs));
		else
		if( strcmpi(key, "auction_txt") == 0 )
			safestrncpy(db->file_auctions, value, sizeof(db->file_auctions));
		else
		if( strcmpi(key, "castle_txt") == 0 )
			safestrncpy(db->file_castles, value, sizeof(db->file_castles));
		else
		if( strcmpi(key, "char_txt") == 0 )
			safestrncpy(db->file_chars, value, sizeof(db->file_chars));
		else
		if( strcmpi(key, "friends_txt") == 0 )
			safestrncpy(db->file_friends, value, sizeof(db->file_friends));
		else
		if( strcmpi(key, "guild_txt") == 0 )
			safestrncpy(db->file_guilds, value, sizeof(db->file_guilds));
		else
		if( strcmpi(key, "guild_storage_txt") == 0 )
			safestrncpy(db->file_guild_storages, value, sizeof(db->file_guild_storages));
		else
		if( strcmpi(key, "homun_txt") == 0 )
			safestrncpy(db->file_homuns, value, sizeof(db->file_homuns));
		else
		if( strcmpi(key, "hotkeys_txt") == 0 )
			safestrncpy(db->file_hotkeys, value, sizeof(db->file_hotkeys));
		else
		if( strcmpi(key, "mail_txt") == 0 )
			safestrncpy(db->file_mails, value, sizeof(db->file_mails));
		else
		if( strcmpi(key, "party_txt") == 0 )
			safestrncpy(db->file_parties, value, sizeof(db->file_parties));
		else
		if( strcmpi(key, "pet_txt") == 0 )
			safestrncpy(db->file_pets, value, sizeof(db->file_pets));
		else
		if( strcmpi(key, "quest_txt") == 0 )
			safestrncpy(db->file_quests, value, sizeof(db->file_quests));
		else
		if( strcmpi(key, "scdata_txt") == 0 )
			safestrncpy(db->file_statuses, value, sizeof(db->file_statuses));
		else
		if( strcmpi(key, "storage_txt") == 0 )
			safestrncpy(db->file_storages, value, sizeof(db->file_storages));

		else
			return false;// not found
		return true;
	}

	return false;// not found
}



/// TODO
static CastleDB* charserver_db_txt_castledb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->castledb;
}



/// TODO
static CharDB* charserver_db_txt_chardb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->chardb;
}



/// TODO
static FriendDB* charserver_db_txt_frienddb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->frienddb;
}



/// TODO
static GuildDB* charserver_db_txt_guilddb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->guilddb;
}



/// TODO
static HomunDB* charserver_db_txt_homundb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->homundb;
}



/// TODO
static HotkeyDB* charserver_db_txt_hotkeydb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->hotkeydb;
}



/// TODO
static PartyDB* charserver_db_txt_partydb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->partydb;
}



/// TODO
static PetDB* charserver_db_txt_petdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->petdb;
}



/// TODO
static QuestDB* charserver_db_txt_questdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->questdb;
}



/// Returns the database interface that handles rankings.
static RankDB* charserver_db_txt_rankdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->rankdb;
}



/// TODO
static MailDB* charserver_db_txt_maildb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->maildb;
}



/// TODO
static AuctionDB* charserver_db_txt_auctiondb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->auctiondb;
}



/// TODO
static StatusDB* charserver_db_txt_statusdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->statusdb;
}



/// TODO
static AccRegDB* charserver_db_txt_accregdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->accregdb;
}



/// TODO
static CharRegDB* charserver_db_txt_charregdb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return db->charregdb;
}



/// constructor
CharServerDB* charserver_db_txt(void)
{
	CharServerDB_TXT* db;

	CREATE(db, CharServerDB_TXT, 1);
	db->vtable.init         = charserver_db_txt_init;
	db->vtable.destroy      = charserver_db_txt_destroy;
	db->vtable.get_property = charserver_db_txt_get_property;
	db->vtable.set_property = charserver_db_txt_set_property;
	db->vtable.castledb     = charserver_db_txt_castledb;
	db->vtable.chardb       = charserver_db_txt_chardb;
	db->vtable.frienddb     = charserver_db_txt_frienddb;
	db->vtable.guilddb      = charserver_db_txt_guilddb;
	db->vtable.homundb      = charserver_db_txt_homundb;
	db->vtable.hotkeydb     = charserver_db_txt_hotkeydb;
	db->vtable.partydb      = charserver_db_txt_partydb;
	db->vtable.petdb        = charserver_db_txt_petdb;
	db->vtable.questdb      = charserver_db_txt_questdb;
	db->vtable.rankdb       = charserver_db_txt_rankdb;
	db->vtable.maildb       = charserver_db_txt_maildb;
	db->vtable.auctiondb    = charserver_db_txt_auctiondb;
	db->vtable.statusdb     = charserver_db_txt_statusdb;
	db->vtable.accregdb     = charserver_db_txt_accregdb;
	db->vtable.charregdb    = charserver_db_txt_charregdb;
	// TODO DB interfaces

	db->castledb = castle_db_txt(db);
	db->chardb = char_db_txt(db);
	db->frienddb = friend_db_txt(db);
	db->guilddb = guild_db_txt(db);
	db->homundb = homun_db_txt(db);
	db->hotkeydb = hotkey_db_txt(db);
	db->partydb = party_db_txt(db);
	db->petdb = pet_db_txt(db);
	db->questdb = quest_db_txt(db);
	db->rankdb = rank_db_txt(db);
	db->maildb = mail_db_txt(db);
	db->auctiondb = auction_db_txt(db);
	db->statusdb = status_db_txt(db);
	db->accregdb = accreg_db_txt(db);
	db->charregdb = charreg_db_txt(db);

	// initialize to default values
	db->initialized = false;

	// other settings
	safestrncpy(db->file_accregs, "save/accreg.txt", sizeof(db->file_accregs));
	safestrncpy(db->file_auctions, "save/auction.txt", sizeof(db->file_auctions));
	safestrncpy(db->file_castles, "save/castle.txt", sizeof(db->file_castles));
	safestrncpy(db->file_chars, "save/athena.txt", sizeof(db->file_chars));
	safestrncpy(db->file_friends, "save/friends.txt", sizeof(db->file_friends));
	safestrncpy(db->file_guilds, "save/guild.txt", sizeof(db->file_guilds));
	safestrncpy(db->file_guild_storages, "save/g_storage.txt", sizeof(db->file_guild_storages));
	safestrncpy(db->file_homuns, "save/homun.txt", sizeof(db->file_homuns));
	safestrncpy(db->file_hotkeys, "save/hotkeys.txt", sizeof(db->file_hotkeys));
	safestrncpy(db->file_mails, "save/mail.txt", sizeof(db->file_mails));
	safestrncpy(db->file_parties, "save/party.txt", sizeof(db->file_parties));
	safestrncpy(db->file_pets, "save/pet.txt", sizeof(db->file_pets));
	safestrncpy(db->file_quests, "save/quest.txt", sizeof(db->file_quests));
	safestrncpy(db->file_statuses, "save/scdata.txt", sizeof(db->file_statuses));
	safestrncpy(db->file_storages, "save/storage.txt", sizeof(db->file_storages));

	return &db->vtable;
}
