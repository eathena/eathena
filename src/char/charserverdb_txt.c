// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "charserverdb_txt.h"

#include <string.h>
#include <stdlib.h>


// Constructors for the individual DBs
extern AccRegDB* accreg_db_txt(CharServerDB_TXT* owner);
extern AuctionDB* auction_db_txt(CharServerDB_TXT* owner);
extern CastleDB* castle_db_txt(CharServerDB_TXT* owner);
extern CharDB* char_db_txt(CharServerDB_TXT* owner);
extern CharRegDB* charreg_db_txt(CharServerDB_TXT* owner);
extern FriendDB* friend_db_txt(CharServerDB_TXT* owner);
extern GuildDB* guild_db_txt(CharServerDB_TXT* owner);
extern HomunDB* homun_db_txt(CharServerDB_TXT* owner);
extern HotkeyDB* hotkey_db_txt(CharServerDB_TXT* owner);
extern MailDB* mail_db_txt(CharServerDB_TXT* owner);
extern MemoDB* memo_db_txt(CharServerDB_TXT* owner);
extern MercDB* merc_db_txt(CharServerDB_TXT* owner);
extern PartyDB* party_db_txt(CharServerDB_TXT* owner);
extern PetDB* pet_db_txt(CharServerDB_TXT* owner);
extern QuestDB* quest_db_txt(CharServerDB_TXT* owner);
extern RankDB* rank_db_txt(CharServerDB_TXT* owner);
extern SkillDB* skill_db_txt(CharServerDB_TXT* owner);
extern StatusDB* status_db_txt(CharServerDB_TXT* owner);
extern StorageDB* storage_db_txt(CharServerDB_TXT* owner);


// forward declarations
static int charserver_db_txt_sync_timer(int tid, unsigned int tick, int id, intptr data);



/// Schedules a sync operation with the specified delay.
/// If already scheduled, it uses the farthest sync time.
/// @private
static void charserver_db_txt_scheduleSync(CharServerDB_TXT* db, int delay)
{
	static bool registered = false;
	if( !registered )
	{
		add_timer_func_list(charserver_db_txt_sync_timer, "charserver_db_txt_sync_timer");
		registered = true;
	}

	if( db->sync_timer == INVALID_TIMER )
	{
		if( delay > db->autosave_max_delay )
			delay = db->autosave_max_delay;
		db->dirty_tick = gettick();
		db->sync_timer = add_timer(db->dirty_tick + delay, charserver_db_txt_sync_timer, 0, (intptr)db);
	}
	else
	{
		unsigned int maxtick = db->dirty_tick + db->autosave_max_delay;
		unsigned int newtick = gettick() + delay;
		if( DIFF_TICK(newtick, maxtick) > 0 )
			newtick = maxtick;
		if( DIFF_TICK(newtick, get_timer(db->sync_timer)->tick) > 0 )
			settick_timer(db->sync_timer, newtick);
	}
}



/// Timer function.
/// Triggers a sync.
/// Reschedules the sync if the operation failed.
/// @private
static int charserver_db_txt_sync_timer(int tid, unsigned int tick, int id, intptr data)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)data;

	if( db && db->sync_timer == tid )
	{
		db->sync_timer = INVALID_TIMER;
		if( !(
			db->chardb->sync(db->chardb) &&
			db->frienddb->sync(db->frienddb) &&
			db->hotkeydb->sync(db->hotkeydb) &&
			db->partydb->sync(db->partydb) &&
			db->guilddb->sync(db->guilddb) &&
			db->castledb->sync(db->castledb) &&
			db->petdb->sync(db->petdb) &&
			db->homundb->sync(db->homundb) &&
			db->mercdb->sync(db->mercdb) &&
			db->accregdb->sync(db->accregdb) &&
			db->charregdb->sync(db->charregdb) &&
			db->skilldb->sync(db->skilldb) &&
			db->statusdb->sync(db->statusdb) &&
			db->storagedb->sync(db->storagedb) &&
			db->maildb->sync(db->maildb) &&
			db->memodb->sync(db->memodb) &&
			db->questdb->sync(db->questdb) &&
			db->rankdb->sync(db->rankdb) &&
			db->auctiondb->sync(db->auctiondb) )
		)
			charserver_db_txt_scheduleSync(db, db->autosave_retry_delay);
	}
	return 0;
}



/// Initializes this database engine, making it ready for use.
static bool charserver_db_txt_init(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	if( db->initialized )
		return true;

	if(
		db->accregdb->init(db->accregdb) &&
		db->charregdb->init(db->charregdb) &&
		db->castledb->init(db->castledb) &&
		db->storagedb->init(db->storagedb) &&
		db->chardb->init(db->chardb) &&
		db->frienddb->init(db->frienddb) &&
		db->guilddb->init(db->guilddb) &&
		db->homundb->init(db->homundb) &&
		db->mercdb->init(db->mercdb) &&
		db->hotkeydb->init(db->hotkeydb) &&
		db->partydb->init(db->partydb) &&
		db->petdb->init(db->petdb) &&
		db->questdb->init(db->questdb) &&
		db->auctiondb->init(db->auctiondb) &&
		db->rankdb->init(db->rankdb) &&
		db->maildb->init(db->maildb) &&
		db->memodb->init(db->memodb) &&
		db->skilldb->init(db->skilldb) &&
		db->statusdb->init(db->statusdb)
	)
		db->initialized = true;

	return db->initialized;
}



/// Destroys this database engine, releasing all allocated memory (including itself).
static void charserver_db_txt_destroy(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	// write cached data
	if( db->initialized && !self->sync(self, true) )
	{
		ShowError("charserver_db_txt_destroy: failed to write cached data, possible data loss\n");
		delete_timer(db->sync_timer, charserver_db_txt_sync_timer);
		db->sync_timer = INVALID_TIMER;
	}

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
	db->mercdb->destroy(db->mercdb);
	db->mercdb = NULL;
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
	db->memodb->destroy(db->memodb);
	db->memodb = NULL;
	db->skilldb->destroy(db->skilldb);
	db->skilldb = NULL;
	db->statusdb->destroy(db->statusdb);
	db->statusdb = NULL;
	db->storagedb->destroy(db->storagedb);
	db->storagedb = NULL;
	db->accregdb->destroy(db->accregdb);
	db->accregdb = NULL;
	db->charregdb->destroy(db->charregdb);
	db->charregdb = NULL;

	aFree(db);
}



/// Writes pending data to permanent storage.
/// If force is true, writes all cached data even if unchanged.
static bool charserver_db_txt_sync(CharServerDB* self, bool force)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	if( db->sync_timer != INVALID_TIMER )
	{
		delete_timer(db->sync_timer, charserver_db_txt_sync_timer);
		db->sync_timer = INVALID_TIMER;
	}
	charserver_db_txt_sync_timer(INVALID_TIMER, gettick(), 0, (intptr)self);
	return (db->sync_timer == INVALID_TIMER);
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
		if( strcmpi(key, "autosave.change_delay") == 0 )
			safesnprintf(buf, buflen, "%d", db->autosave_change_delay);
		else
		if( strcmpi(key, "autosave.retry_delay") == 0 )
			safesnprintf(buf, buflen, "%d", db->autosave_retry_delay);
		else
		if( strcmpi(key, "autosave.max_delay") == 0 )
			safesnprintf(buf, buflen, "%d", db->autosave_max_delay);
		else
		if( strcmpi(key, "file_accregs") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_accregs);
		else
		if( strcmpi(key, "file_auctions") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_auctions);
		else
		if( strcmpi(key, "file_carts") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_carts);
		else
		if( strcmpi(key, "file_castles") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_castles);
		else
		if( strcmpi(key, "file_chars") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_chars);
		else
		if( strcmpi(key, "file_charregs") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_charregs);
		else
		if( strcmpi(key, "file_friends") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_friends);
		else
		if( strcmpi(key, "file_guilds") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_guilds);
		else
		if( strcmpi(key, "file_guild_storages") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_guild_storages);
		else
		if( strcmpi(key, "file_homuns") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_homuns);
		else
		if( strcmpi(key, "file_hotkeys") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_hotkeys);
		else
		if( strcmpi(key, "file_inventories") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_inventories);
		else
		if( strcmpi(key, "file_mails") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_mails);
		else
		if( strcmpi(key, "file_memos") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_memos);
		else
		if( strcmpi(key, "file_mercenaries") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_mercenaries);
		else
		if( strcmpi(key, "file_parties") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_parties);
		else
		if( strcmpi(key, "file_pets") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_pets);
		else
		if( strcmpi(key, "file_quests") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_quests);
		else
		if( strcmpi(key, "file_ranks") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_ranks);
		else
		if( strcmpi(key, "file_skills") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_skills);
		else
		if( strcmpi(key, "file_statuses") == 0 )
			safesnprintf(buf, buflen, "%s", db->file_statuses);
		else
		if( strcmpi(key, "file_storages") == 0 )
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
		if( strcmpi(key, "autosave.change_delay") == 0 )
			db->autosave_change_delay = atoi(value);
		else
		if( strcmpi(key, "autosave.retry_delay") == 0 )
			db->autosave_retry_delay = atoi(value);
		else
		if( strcmpi(key, "autosave.max_delay") == 0 )
			db->autosave_max_delay = atoi(value);
		else
		if( strcmpi(key, "file_accregs") == 0 )
			safestrncpy(db->file_accregs, value, sizeof(db->file_accregs));
		else
		if( strcmpi(key, "file_auctions") == 0 )
			safestrncpy(db->file_auctions, value, sizeof(db->file_auctions));
		else
		if( strcmpi(key, "file_carts") == 0 )
			safestrncpy(db->file_carts, value, sizeof(db->file_carts));
		else
		if( strcmpi(key, "file_castles") == 0 )
			safestrncpy(db->file_castles, value, sizeof(db->file_castles));
		else
		if( strcmpi(key, "file_chars") == 0 )
			safestrncpy(db->file_chars, value, sizeof(db->file_chars));
		else
		if( strcmpi(key, "file_charregs") == 0 )
			safestrncpy(db->file_charregs, value, sizeof(db->file_charregs));
		else
		if( strcmpi(key, "file_friends") == 0 )
			safestrncpy(db->file_friends, value, sizeof(db->file_friends));
		else
		if( strcmpi(key, "file_guilds") == 0 )
			safestrncpy(db->file_guilds, value, sizeof(db->file_guilds));
		else
		if( strcmpi(key, "file_guild_storages") == 0 )
			safestrncpy(db->file_guild_storages, value, sizeof(db->file_guild_storages));
		else
		if( strcmpi(key, "file_homuns") == 0 )
			safestrncpy(db->file_homuns, value, sizeof(db->file_homuns));
		else
		if( strcmpi(key, "file_hotkeys") == 0 )
			safestrncpy(db->file_hotkeys, value, sizeof(db->file_hotkeys));
		else
		if( strcmpi(key, "file_inventories") == 0 )
			safestrncpy(db->file_inventories, value, sizeof(db->file_inventories));
		else
		if( strcmpi(key, "file_mails") == 0 )
			safestrncpy(db->file_mails, value, sizeof(db->file_mails));
		else
		if( strcmpi(key, "file_memos") == 0 )
			safestrncpy(db->file_memos, value, sizeof(db->file_memos));
		else
		if( strcmpi(key, "file_mercenaries") == 0 )
			safestrncpy(db->file_mercenaries, value, sizeof(db->file_mercenaries));
		else
		if( strcmpi(key, "file_parties") == 0 )
			safestrncpy(db->file_parties, value, sizeof(db->file_parties));
		else
		if( strcmpi(key, "file_pets") == 0 )
			safestrncpy(db->file_pets, value, sizeof(db->file_pets));
		else
		if( strcmpi(key, "file_quests") == 0 )
			safestrncpy(db->file_quests, value, sizeof(db->file_quests));
		else
		if( strcmpi(key, "file_ranks") == 0 )
			safestrncpy(db->file_ranks, value, sizeof(db->file_ranks));
		else
		if( strcmpi(key, "file_skills") == 0 )
			safestrncpy(db->file_skills, value, sizeof(db->file_skills));
		else
		if( strcmpi(key, "file_statuses") == 0 )
			safestrncpy(db->file_statuses, value, sizeof(db->file_statuses));
		else
		if( strcmpi(key, "file_storages") == 0 )
			safestrncpy(db->file_storages, value, sizeof(db->file_storages));

		else
			return false;// not found
		return true;
	}

	return false;// not found
}



// Accessors for the various DB interfaces.
static AccRegDB*       charserver_db_txt_accregdb      (CharServerDB* self) { return ((CharServerDB_TXT*)self)->accregdb;       }
static AuctionDB*      charserver_db_txt_auctiondb     (CharServerDB* self) { return ((CharServerDB_TXT*)self)->auctiondb;      }
static CastleDB*       charserver_db_txt_castledb      (CharServerDB* self) { return ((CharServerDB_TXT*)self)->castledb;       }
static CharDB*         charserver_db_txt_chardb        (CharServerDB* self) { return ((CharServerDB_TXT*)self)->chardb;         }
static CharRegDB*      charserver_db_txt_charregdb     (CharServerDB* self) { return ((CharServerDB_TXT*)self)->charregdb;      }
static FriendDB*       charserver_db_txt_frienddb      (CharServerDB* self) { return ((CharServerDB_TXT*)self)->frienddb;       }
static GuildDB*        charserver_db_txt_guilddb       (CharServerDB* self) { return ((CharServerDB_TXT*)self)->guilddb;        }
static HomunDB*        charserver_db_txt_homundb       (CharServerDB* self) { return ((CharServerDB_TXT*)self)->homundb;        }
static HotkeyDB*       charserver_db_txt_hotkeydb      (CharServerDB* self) { return ((CharServerDB_TXT*)self)->hotkeydb;       }
static MailDB*         charserver_db_txt_maildb        (CharServerDB* self) { return ((CharServerDB_TXT*)self)->maildb;         }
static MemoDB*         charserver_db_txt_memodb        (CharServerDB* self) { return ((CharServerDB_TXT*)self)->memodb;         }
static MercDB*         charserver_db_txt_mercdb        (CharServerDB* self) { return ((CharServerDB_TXT*)self)->mercdb;         }
static PartyDB*        charserver_db_txt_partydb       (CharServerDB* self) { return ((CharServerDB_TXT*)self)->partydb;        }
static PetDB*          charserver_db_txt_petdb         (CharServerDB* self) { return ((CharServerDB_TXT*)self)->petdb;          }
static QuestDB*        charserver_db_txt_questdb       (CharServerDB* self) { return ((CharServerDB_TXT*)self)->questdb;        }
static RankDB*         charserver_db_txt_rankdb        (CharServerDB* self) { return ((CharServerDB_TXT*)self)->rankdb;         }
static SkillDB*        charserver_db_txt_skilldb       (CharServerDB* self) { return ((CharServerDB_TXT*)self)->skilldb;        }
static StatusDB*       charserver_db_txt_statusdb      (CharServerDB* self) { return ((CharServerDB_TXT*)self)->statusdb;       }
static StorageDB*      charserver_db_txt_storagedb     (CharServerDB* self) { return ((CharServerDB_TXT*)self)->storagedb;      }



/// Requests a sync.
/// Called when data is changed in one of the database interfaces.
/// @protected
void charserver_db_txt_request_sync(CharServerDB_TXT* db)
{
	charserver_db_txt_scheduleSync(db, db->autosave_change_delay);
}



/// constructor
CharServerDB* charserver_db_txt(void)
{
	CharServerDB_TXT* db;

	CREATE(db, CharServerDB_TXT, 1);
	db->vtable.init         = charserver_db_txt_init;
	db->vtable.destroy      = charserver_db_txt_destroy;
	db->vtable.sync         = charserver_db_txt_sync;
	db->vtable.get_property = charserver_db_txt_get_property;
	db->vtable.set_property = charserver_db_txt_set_property;
	db->vtable.castledb     = charserver_db_txt_castledb;
	db->vtable.chardb       = charserver_db_txt_chardb;
	db->vtable.frienddb     = charserver_db_txt_frienddb;
	db->vtable.guilddb      = charserver_db_txt_guilddb;
	db->vtable.homundb      = charserver_db_txt_homundb;
	db->vtable.mercdb       = charserver_db_txt_mercdb;
	db->vtable.hotkeydb     = charserver_db_txt_hotkeydb;
	db->vtable.partydb      = charserver_db_txt_partydb;
	db->vtable.petdb        = charserver_db_txt_petdb;
	db->vtable.questdb      = charserver_db_txt_questdb;
	db->vtable.rankdb       = charserver_db_txt_rankdb;
	db->vtable.maildb       = charserver_db_txt_maildb;
	db->vtable.memodb       = charserver_db_txt_memodb;
	db->vtable.auctiondb    = charserver_db_txt_auctiondb;
	db->vtable.skilldb      = charserver_db_txt_skilldb;
	db->vtable.statusdb     = charserver_db_txt_statusdb;
	db->vtable.storagedb    = charserver_db_txt_storagedb;
	db->vtable.accregdb     = charserver_db_txt_accregdb;
	db->vtable.charregdb    = charserver_db_txt_charregdb;
	db->p.request_sync      = charserver_db_txt_request_sync;
	// TODO DB interfaces

	db->castledb = castle_db_txt(db);
	db->chardb = char_db_txt(db);
	db->frienddb = friend_db_txt(db);
	db->guilddb = guild_db_txt(db);
	db->homundb = homun_db_txt(db);
	db->mercdb = merc_db_txt(db);
	db->hotkeydb = hotkey_db_txt(db);
	db->partydb = party_db_txt(db);
	db->petdb = pet_db_txt(db);
	db->questdb = quest_db_txt(db);
	db->rankdb = rank_db_txt(db);
	db->maildb = mail_db_txt(db);
	db->memodb = memo_db_txt(db);
	db->auctiondb = auction_db_txt(db);
	db->skilldb = skill_db_txt(db);
	db->statusdb = status_db_txt(db);
	db->storagedb = storage_db_txt(db);
	db->accregdb = accreg_db_txt(db);
	db->charregdb = charreg_db_txt(db);

	// initialize to default values
	db->initialized = false;
	db->dirty_tick = gettick();
	db->sync_timer = INVALID_TIMER;

	// other settings
	db->autosave_change_delay = CHARSERVERDB_AUTOSAVE_CHANGE_DELAY;
	db->autosave_retry_delay = CHARSERVERDB_AUTOSAVE_RETRY_DELAY;
	db->autosave_max_delay = CHARSERVERDB_AUTOSAVE_MAX_DELAY;
	safestrncpy(db->file_accregs, "save/accreg.txt", sizeof(db->file_accregs));
	safestrncpy(db->file_auctions, "save/auction.txt", sizeof(db->file_auctions));
	safestrncpy(db->file_carts, "save/cart.txt", sizeof(db->file_carts));
	safestrncpy(db->file_castles, "save/castle.txt", sizeof(db->file_castles));
	safestrncpy(db->file_chars, "save/char.txt", sizeof(db->file_chars));
	safestrncpy(db->file_friends, "save/friends.txt", sizeof(db->file_friends));
	safestrncpy(db->file_guilds, "save/guild.txt", sizeof(db->file_guilds));
	safestrncpy(db->file_guild_storages, "save/g_storage.txt", sizeof(db->file_guild_storages));
	safestrncpy(db->file_homuns, "save/homun.txt", sizeof(db->file_homuns));
	safestrncpy(db->file_hotkeys, "save/hotkey.txt", sizeof(db->file_hotkeys));
	safestrncpy(db->file_inventories, "save/inventory.txt", sizeof(db->file_inventories));
	safestrncpy(db->file_mails, "save/mail.txt", sizeof(db->file_mails));
	safestrncpy(db->file_memos, "save/memo.txt", sizeof(db->file_memos));
	safestrncpy(db->file_mercenaries, "save/mercenary.txt", sizeof(db->file_mercenaries));
	safestrncpy(db->file_parties, "save/party.txt", sizeof(db->file_parties));
	safestrncpy(db->file_pets, "save/pet.txt", sizeof(db->file_pets));
	safestrncpy(db->file_quests, "save/quest.txt", sizeof(db->file_quests));
	safestrncpy(db->file_ranks, "save/ranks.txt", sizeof(db->file_ranks));
	safestrncpy(db->file_skills, "save/skill.txt", sizeof(db->file_skills));
	safestrncpy(db->file_statuses, "save/scdata.txt", sizeof(db->file_statuses));
	safestrncpy(db->file_storages, "save/storage.txt", sizeof(db->file_storages));

	return &db->vtable;
}
