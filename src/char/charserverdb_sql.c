// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include <string.h>
#include <stdlib.h>


// Constructors for the individual DBs
extern AccRegDB* accreg_db_sql(CharServerDB_SQL* owner);
extern AuctionDB* auction_db_sql(CharServerDB_SQL* owner);
extern CastleDB* castle_db_sql(CharServerDB_SQL* owner);
extern CharDB* char_db_sql(CharServerDB_SQL* owner);
extern CharRegDB* charreg_db_sql(CharServerDB_SQL* owner);
extern FriendDB* friend_db_sql(CharServerDB_SQL* owner);
extern GuildDB* guild_db_sql(CharServerDB_SQL* owner);
extern HomunDB* homun_db_sql(CharServerDB_SQL* owner);
extern HotkeyDB* hotkey_db_sql(CharServerDB_SQL* owner);
extern MailDB* mail_db_sql(CharServerDB_SQL* owner);
extern MemoDB* memo_db_sql(CharServerDB_SQL* owner);
extern MercDB* merc_db_sql(CharServerDB_SQL* owner);
extern PartyDB* party_db_sql(CharServerDB_SQL* owner);
extern PetDB* pet_db_sql(CharServerDB_SQL* owner);
extern QuestDB* quest_db_sql(CharServerDB_SQL* owner);
extern RankDB* rank_db_sql(CharServerDB_SQL* owner);
extern SkillDB* skill_db_sql(CharServerDB_SQL* owner);
extern StatusDB* status_db_sql(CharServerDB_SQL* owner);
extern StorageDB* storage_db_sql(CharServerDB_SQL* owner);


/// Initializes this database engine, making it ready for use.
/// @protected
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
		db->statusdb->init(db->statusdb) &&
		db->storagedb->init(db->storagedb)
	)
		db->initialized = true;

	return db->initialized;
}


/// Destroys this database engine, releasing all allocated memory (including itself).
/// @protected
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
	db->storagedb= NULL;
	db->accregdb->destroy(db->accregdb);
	db->accregdb = NULL;
	db->charregdb->destroy(db->charregdb);
	db->charregdb = NULL;

	Sql_Free(db->sql_handle);
	db->sql_handle = NULL;
	aFree(db);
}


/// Writes pending data to permanent storage.
/// If force is true, writes all cached data even if unchanged.
/// @protected
static bool charserver_db_sql_sync(CharServerDB* self, bool force)
{
	return true; //not needed for this engine
}


/// Gets a property from this database engine.
/// @protected
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
		if( strcmpi(key, "table_auctions") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_auctions);
		else
		if( strcmpi(key, "table_carts") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_carts);
		else
		if( strcmpi(key, "table_castles") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_castles);
		else
		if( strcmpi(key, "table_chars") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_chars);
		else
		if( strcmpi(key, "table_friends") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_friends);
		else
		if( strcmpi(key, "table_guilds") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guilds);
		else
		if( strcmpi(key, "table_guild_alliances") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_alliances);
		else
		if( strcmpi(key, "table_guild_expulsions") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_expulsions);
		else
		if( strcmpi(key, "table_guild_members") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_members);
		else
		if( strcmpi(key, "table_guild_positions") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_positions);
		else
		if( strcmpi(key, "table_guild_skills") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_skills);
		else
		if( strcmpi(key, "table_guild_storages") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_guild_storages);
		else
		if( strcmpi(key, "table_homuns") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_homuns);
		else
		if( strcmpi(key, "table_homun_skills") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_homun_skills);
		else
		if( strcmpi(key, "table_hotkeys") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_hotkeys);
		else
		if( strcmpi(key, "table_inventories") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_inventories);
		else
		if( strcmpi(key, "table_mails") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_mails);
		else
		if( strcmpi(key, "table_memos") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_memos);
		else
		if( strcmpi(key, "table_mercenaries") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_mercenaries);
		else
		if( strcmpi(key, "table_mercenary_owners") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_mercenary_owners);
		else
		if( strcmpi(key, "table_parties") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_parties);
		else
		if( strcmpi(key, "table_pets") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_pets);
		else
		if( strcmpi(key, "table_quests") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_quests);
		else
		if( strcmpi(key, "table_ranks") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_ranks);
		else
		if( strcmpi(key, "table_registry") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_registry);
		else
		if( strcmpi(key, "table_statuses") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_statuses);
		else
		if( strcmpi(key, "table_skills") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_skills);
		else
		if( strcmpi(key, "table_storages") == 0 )
			safesnprintf(buf, buflen, "%s", db->table_storages);

		else
			return false;// not found
		return true;
	}

	return false;// not found
}


/// Sets a property in this database engine.
/// @protected
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
		if( strcmpi(key, "table_auctions") == 0 )
			safestrncpy(db->table_auctions, value, sizeof(db->table_auctions));
		else
		if( strcmpi(key, "table_carts") == 0 )
			safestrncpy(db->table_carts, value, sizeof(db->table_carts));
		else
		if( strcmpi(key, "table_castles") == 0 )
			safestrncpy(db->table_castles, value, sizeof(db->table_castles));
		else
		if( strcmpi(key, "table_chars") == 0 )
			safestrncpy(db->table_chars, value, sizeof(db->table_chars));
		else
		if( strcmpi(key, "table_friends") == 0 )
			safestrncpy(db->table_friends, value, sizeof(db->table_friends));
		else
		if( strcmpi(key, "table_guilds") == 0 )
			safestrncpy(db->table_guilds, value, sizeof(db->table_guilds));
		else
		if( strcmpi(key, "table_guild_alliances") == 0 )
			safestrncpy(db->table_guild_alliances, value, sizeof(db->table_guild_alliances));
		else
		if( strcmpi(key, "table_guild_expulsions") == 0 )
			safestrncpy(db->table_guild_expulsions, value, sizeof(db->table_guild_expulsions));
		else
		if( strcmpi(key, "table_guild_members") == 0 )
			safestrncpy(db->table_guild_members, value, sizeof(db->table_guild_members));
		else
		if( strcmpi(key, "table_guild_positions") == 0 )
			safestrncpy(db->table_guild_positions, value, sizeof(db->table_guild_positions));
		else
		if( strcmpi(key, "table_guild_skills") == 0 )
			safestrncpy(db->table_guild_skills, value, sizeof(db->table_guild_skills));
		else
		if( strcmpi(key, "table_guild_storages") == 0 )
			safestrncpy(db->table_guild_storages, value, sizeof(db->table_guild_storages));
		else
		if( strcmpi(key, "table_homuns") == 0 )
			safestrncpy(db->table_homuns, value, sizeof(db->table_homuns));
		else
		if( strcmpi(key, "table_homun_skills") == 0 )
			safestrncpy(db->table_homun_skills, value, sizeof(db->table_homun_skills));
		else
		if( strcmpi(key, "table_hotkeys") == 0 )
			safestrncpy(db->table_hotkeys, value, sizeof(db->table_hotkeys));
		else
		if( strcmpi(key, "table_inventories") == 0 )
			safestrncpy(db->table_inventories, value, sizeof(db->table_inventories));
		else
		if( strcmpi(key, "table_mails") == 0 )
			safestrncpy(db->table_mails, value, sizeof(db->table_mails));
		else
		if( strcmpi(key, "table_memos") == 0 )
			safestrncpy(db->table_memos, value, sizeof(db->table_memos));
		else
		if( strcmpi(key, "table_mercenaries") == 0 )
			safestrncpy(db->table_mercenaries, value, sizeof(db->table_mercenaries));
		else
		if( strcmpi(key, "table_mercenary_owners") == 0 )
			safestrncpy(db->table_mercenary_owners, value, sizeof(db->table_mercenary_owners));
		else
		if( strcmpi(key, "table_parties") == 0 )
			safestrncpy(db->table_parties, value, sizeof(db->table_parties));
		else
		if( strcmpi(key, "table_pets") == 0 )
			safestrncpy(db->table_pets, value, sizeof(db->table_pets));
		else
		if( strcmpi(key, "table_quests") == 0 )
			safestrncpy(db->table_quests, value, sizeof(db->table_quests));
		else
		if( strcmpi(key, "table_ranks") == 0 )
			safestrncpy(db->table_ranks, value, sizeof(db->table_ranks));
		else
		if( strcmpi(key, "table_registry") == 0 )
			safestrncpy(db->table_registry, value, sizeof(db->table_registry));
		else
		if( strcmpi(key, "table_statuses") == 0 )
			safestrncpy(db->table_statuses, value, sizeof(db->table_statuses));
		else
		if( strcmpi(key, "table_skills") == 0 )
			safestrncpy(db->table_skills, value, sizeof(db->table_skills));
		else
		if( strcmpi(key, "table_storages") == 0 )
			safestrncpy(db->table_storages, value, sizeof(db->table_storages));

		else
			return false;// not found
		return true;
	}

	return false;// not found
}


// Accessors for the various DB interfaces.
static AccRegDB*       charserver_db_sql_accregdb      (CharServerDB* self) { return ((CharServerDB_SQL*)self)->accregdb;       }
static AuctionDB*      charserver_db_sql_auctiondb     (CharServerDB* self) { return ((CharServerDB_SQL*)self)->auctiondb;      }
static CastleDB*       charserver_db_sql_castledb      (CharServerDB* self) { return ((CharServerDB_SQL*)self)->castledb;       }
static CharDB*         charserver_db_sql_chardb        (CharServerDB* self) { return ((CharServerDB_SQL*)self)->chardb;         }
static CharRegDB*      charserver_db_sql_charregdb     (CharServerDB* self) { return ((CharServerDB_SQL*)self)->charregdb;      }
static FriendDB*       charserver_db_sql_frienddb      (CharServerDB* self) { return ((CharServerDB_SQL*)self)->frienddb;       }
static GuildDB*        charserver_db_sql_guilddb       (CharServerDB* self) { return ((CharServerDB_SQL*)self)->guilddb;        }
static HomunDB*        charserver_db_sql_homundb       (CharServerDB* self) { return ((CharServerDB_SQL*)self)->homundb;        }
static HotkeyDB*       charserver_db_sql_hotkeydb      (CharServerDB* self) { return ((CharServerDB_SQL*)self)->hotkeydb;       }
static MailDB*         charserver_db_sql_maildb        (CharServerDB* self) { return ((CharServerDB_SQL*)self)->maildb;         }
static MemoDB*         charserver_db_sql_memodb        (CharServerDB* self) { return ((CharServerDB_SQL*)self)->memodb;         }
static MercDB*         charserver_db_sql_mercdb        (CharServerDB* self) { return ((CharServerDB_SQL*)self)->mercdb;         }
static PartyDB*        charserver_db_sql_partydb       (CharServerDB* self) { return ((CharServerDB_SQL*)self)->partydb;        }
static PetDB*          charserver_db_sql_petdb         (CharServerDB* self) { return ((CharServerDB_SQL*)self)->petdb;          }
static QuestDB*        charserver_db_sql_questdb       (CharServerDB* self) { return ((CharServerDB_SQL*)self)->questdb;        }
static RankDB*         charserver_db_sql_rankdb        (CharServerDB* self) { return ((CharServerDB_SQL*)self)->rankdb;         }
static SkillDB*        charserver_db_sql_skilldb       (CharServerDB* self) { return ((CharServerDB_SQL*)self)->skilldb;        }
static StatusDB*       charserver_db_sql_statusdb      (CharServerDB* self) { return ((CharServerDB_SQL*)self)->statusdb;       }
static StorageDB*      charserver_db_sql_storagedb     (CharServerDB* self) { return ((CharServerDB_SQL*)self)->storagedb;      }


/// Constructs a new CharServerDB interface.
/// @public
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
	db->vtable.homundb      = charserver_db_sql_homundb;
	db->vtable.mercdb       = charserver_db_sql_mercdb;
	db->vtable.hotkeydb     = charserver_db_sql_hotkeydb;
	db->vtable.partydb      = charserver_db_sql_partydb;
	db->vtable.petdb        = charserver_db_sql_petdb;
	db->vtable.questdb      = charserver_db_sql_questdb;
	db->vtable.rankdb       = charserver_db_sql_rankdb;
	db->vtable.maildb       = charserver_db_sql_maildb;
	db->vtable.memodb       = charserver_db_sql_memodb;
	db->vtable.auctiondb    = charserver_db_sql_auctiondb;
	db->vtable.skilldb      = charserver_db_sql_skilldb;
	db->vtable.statusdb     = charserver_db_sql_statusdb;
	db->vtable.storagedb    = charserver_db_sql_storagedb;
	db->vtable.accregdb     = charserver_db_sql_accregdb;
	db->vtable.charregdb    = charserver_db_sql_charregdb;
	// TODO DB interfaces

	db->castledb = castle_db_sql(db);
	db->chardb = char_db_sql(db);
	db->frienddb = friend_db_sql(db);
	db->guilddb = guild_db_sql(db);
	db->homundb = homun_db_sql(db);
	db->mercdb = merc_db_sql(db);
	db->hotkeydb = hotkey_db_sql(db);
	db->partydb = party_db_sql(db);
	db->petdb = pet_db_sql(db);
	db->questdb = quest_db_sql(db);
	db->rankdb = rank_db_sql(db);
	db->maildb = mail_db_sql(db);
	db->memodb = memo_db_sql(db);
	db->auctiondb = auction_db_sql(db);
	db->skilldb = skill_db_sql(db);
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
	safestrncpy(db->table_mercenaries, "mercenary", sizeof(db->table_mercenaries));
	safestrncpy(db->table_mercenary_owners, "mercenary_owner", sizeof(db->table_mercenary_owners));
	safestrncpy(db->table_memos, "memo", sizeof(db->table_memos));
	safestrncpy(db->table_parties, "party", sizeof(db->table_parties));
	safestrncpy(db->table_pets, "pet", sizeof(db->table_pets));
	safestrncpy(db->table_quests, "quest", sizeof(db->table_quests));
	safestrncpy(db->table_ranks, "ranks", sizeof(db->table_ranks));
	safestrncpy(db->table_registry, "global_reg_value", sizeof(db->table_registry));
	safestrncpy(db->table_skills, "skill", sizeof(db->table_skills));
	safestrncpy(db->table_statuses, "sc_data", sizeof(db->table_statuses));
	safestrncpy(db->table_storages, "storage", sizeof(db->table_storages));

	return &db->vtable;
}
