// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILDSTORAGEDB_H_
#define _GUILDSTORAGEDB_H_

#include "../common/mmo.h" // struct guild_storage

typedef struct GuildStorageDB GuildStorageDB;
typedef struct GuildStorageDBIterator GuildStorageDBIterator;


struct GuildStorageDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(GuildStorageDBIterator* self);

	/// Fetches the next guild data and stores it in 'data'.
	/// @param self Iterator
	/// @param data a guild's storage data
	/// @param key a guild's guild_id
	/// @return true if successful
	bool (*next)(GuildStorageDBIterator* self, struct guild_storage* data, int* key);
};


struct GuildStorageDB
{
	bool (*init)(GuildStorageDB* self);
	void (*destroy)(GuildStorageDB* self);

	bool (*sync)(GuildStorageDB* self);

	bool (*remove)(GuildStorageDB* self, const int guild_id);

	bool (*save)(GuildStorageDB* self, const struct guild_storage* gs, int guild_id);
	bool (*load)(GuildStorageDB* self, struct guild_storage* gs, int guild_id);

	/// Returns an iterator over all guild storages.
	///
	/// @param self Database
	/// @return Iterator
	GuildStorageDBIterator* (*iterator)(GuildStorageDB* self);
};


#endif /* _GUILDSTORAGEDB_H_ */
