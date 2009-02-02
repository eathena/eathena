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
#include "homundb.h"
#include "hotkeydb.h"
#include "maildb.h"
#include "partydb.h"
#include "petdb.h"
#include "questdb.h"
#include "rankdb.h"
#include "statusdb.h"
// TODO include DB interface headers
// [ data reference ]
// characters : character, char variables, quest data, sc data, hotkeys, inventory (indexed by charid and only affects individual characters)
// storages : account/guild/cart? storages (indexed by type and id)
// ranks : alchemist/blacksmith/taekwon rank (indexed by type and id)
// mails : mails
// auctions : auctions
// pets : pets
// homunculus : homunculus
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

	/// TODO
	CastleDB* (*castledb)(CharServerDB* self);

	CharDB* (*chardb)(CharServerDB* self);

	FriendDB* (*frienddb)(CharServerDB* self);

	GuildDB* (*guilddb)(CharServerDB* self);

	HomunDB* (*homundb)(CharServerDB* self);

	HotkeyDB* (*hotkeydb)(CharServerDB* self);

	PartyDB* (*partydb)(CharServerDB* self);

	PetDB* (*petdb)(CharServerDB* self);

	QuestDB* (*questdb)(CharServerDB* self);

	/// Returns the database interface that handles rankings.
	/// Returns NULL if rankings are not supported.
	///
	/// @param self Database engine
	/// @return Interface for rankings
	RankDB* (*rankdb)(CharServerDB* self);

	MailDB* (*maildb)(CharServerDB* self);

	AuctionDB* (*auctiondb)(CharServerDB* self);

	StatusDB* (*statusdb)(CharServerDB* self);

	AccRegDB* (*accregdb)(CharServerDB* self);

	CharRegDB* (*charregdb)(CharServerDB* self);
	// TODO DB interface accessors
};



#endif /* _CHARSERVERDB_H_ */
