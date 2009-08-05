// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_H_
#define _CHARSERVERDB_H_
/// \file
/// Public interface of the database engine.
/// Includes all the database interfaces.
/// Everything exposed by this header is meant to be used by external code.



#include "accregdb.h"
#include "auctiondb.h"
#include "castledb.h"
#include "chardb.h"
#include "charregdb.h"
#include "frienddb.h"
#include "guilddb.h"
#include "guildstoragedb.h"
#include "homundb.h"
#include "hotkeydb.h"
#include "maildb.h"
#include "mercdb.h"
#include "partydb.h"
#include "petdb.h"
#include "questdb.h"
#include "rankdb.h"
#include "statusdb.h"
#include "storagedb.h"
// TODO include DB interface headers
// [ data reference ]
// characters : character, char variables, quest data, sc data, hotkeys, inventory (indexed by charid and only affects individual characters)
// storages : account/guild/cart? storages (indexed by type and id)
// ranks : alchemist/blacksmith/taekwon rank (indexed by type and id)
// mails : mails
// auctions : auctions
// pets : pets
// homunculus : homunculus
// mercenaries : mercenaries
// groups : families, friends, parties, guilds



typedef struct CharServerDB CharServerDB;



// charserver_db_<engine>
#define CHARSERVERDB_CONSTRUCTOR_(engine) charserver_db_##engine
#define CHARSERVERDB_CONSTRUCTOR(engine) CHARSERVERDB_CONSTRUCTOR_(engine)

// standard engines
#ifdef WITH_TXT
CharServerDB* charserver_db_txt(void);
#endif
#ifdef WITH_SQL
CharServerDB* charserver_db_sql(void);
#endif
// extra engines
#ifdef CHARSERVERDB_ENGINE_0
CharServerDB* CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_0)(void);
#endif
#ifdef CHARSERVERDB_ENGINE_1
CharServerDB* CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_1)(void);
#endif
#ifdef CHARSERVERDB_ENGINE_2
CharServerDB* CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_2)(void);
#endif
#ifdef CHARSERVERDB_ENGINE_3
CharServerDB* CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_3)(void);
#endif
#ifdef CHARSERVERDB_ENGINE_4
CharServerDB* CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_4)(void);
#endif



struct CharServerDB
{
	/// Initializes this database engine, making it ready for use.
	/// Call this after setting the properties.
	///
	/// @param self Database engine
	/// @return true if successful
	bool (*init)(CharServerDB* self);

	/// Destroys this database engine, releasing all allocated memory (including itself).
	///
	/// @param self Database engine
	void (*destroy)(CharServerDB* self);

	/// Writes pending data to permanent storage.
	/// If force is true, writes all cached data even if unchanged.
	///
	/// @param self Database engine
	/// @param force Force write
	/// @return true if successfull
	bool (*sync)(CharServerDB* self, bool force);

	/// Gets a property from this database engine.
	/// These read-only properties must be implemented:
	/// "engine.name" -> "txt", "sql", ...
	/// "engine.version" -> internal version
	/// "engine.comment" -> anything (suggestion: description or specs of the engine)
	///
	/// @param self Database engine
	/// @param key Property name
	/// @param buf Buffer for the value
	/// @param buflen Buffer length
	/// @return true if successful
	bool (*get_property)(CharServerDB* self, const char* key, char* buf, size_t buflen);

	/// Sets a property in this database engine.
	///
	/// @param self Database engine
	/// @param key Property name
	/// @param value Property value
	/// @return true if successful
	bool (*set_property)(CharServerDB* self, const char* key, const char* value);

	/// Returns the database interface that handles account regs.
	///
	/// @param self Database engine
	/// @return Interface for account regs
	AccRegDB* (*accregdb)(CharServerDB* self);

	/// Returns the database interface that handles auctions.
	///
	/// @param self Database engine
	/// @return Interface for auctions
	AuctionDB* (*auctiondb)(CharServerDB* self);

	/// Returns the database interface that handles castles.
	///
	/// @param self Database engine
	/// @return Interface for castles
	CastleDB* (*castledb)(CharServerDB* self);

	/// Returns the database interface that handles characters.
	///
	/// @param self Database engine
	/// @return Interface for characters
	CharDB* (*chardb)(CharServerDB* self);

	/// Returns the database interface that handles character regs.
	///
	/// @param self Database engine
	/// @return Interface for character regs
	CharRegDB* (*charregdb)(CharServerDB* self);

	/// Returns the database interface that handles friends.
	///
	/// @param self Database engine
	/// @return Interface for friends
	FriendDB* (*frienddb)(CharServerDB* self);

	/// Returns the database interface that handles guilds.
	///
	/// @param self Database engine
	/// @return Interface for guilds
	GuildDB* (*guilddb)(CharServerDB* self);

	/// Returns the database interface that handles guild storages.
	///
	/// @param self Database engine
	/// @return Interface for guild storages
	GuildStorageDB* (*guildstoragedb)(CharServerDB* self);

	/// Returns the database interface that handles homuns.
	///
	/// @param self Database engine
	/// @return Interface for homuns
	HomunDB* (*homundb)(CharServerDB* self);

	/// Returns the database interface that handles hotkeys.
	///
	/// @param self Database engine
	/// @return Interface for hotkeys
	HotkeyDB* (*hotkeydb)(CharServerDB* self);

	/// Returns the database interface that handles mails.
	///
	/// @param self Database engine
	/// @return Interface for mails
	MailDB* (*maildb)(CharServerDB* self);

	/// Returns the database interface that handles mercenaries.
	///
	/// @param self Database engine
	/// @return Interface for mercenaries
	MercDB* (*mercdb)(CharServerDB* self);

	/// Returns the database interface that handles parties.
	///
	/// @param self Database engine
	/// @return Interface for parties
	PartyDB* (*partydb)(CharServerDB* self);

	/// Returns the database interface that handles pets.
	///
	/// @param self Database engine
	/// @return Interface for pets
	PetDB* (*petdb)(CharServerDB* self);

	/// Returns the database interface that handles quests.
	///
	/// @param self Database engine
	/// @return Interface for quests
	QuestDB* (*questdb)(CharServerDB* self);

	/// Returns the database interface that handles rankings.
	/// Returns NULL if rankings are not supported.
	///
	/// @param self Database engine
	/// @return Interface for rankings
	RankDB* (*rankdb)(CharServerDB* self);

	/// Returns the database interface that handles statuses.
	/// Returns NULL if rankings are not supported.
	///
	/// @param self Database engine
	/// @return Interface for statuses
	StatusDB* (*statusdb)(CharServerDB* self);

	/// Returns the database interface that handles storages.
	/// Returns NULL if rankings are not supported.
	///
	/// @param self Database engine
	/// @return Interface for storages
	StorageDB* (*storagedb)(CharServerDB* self);
};



#endif /* _CHARSERVERDB_H_ */
