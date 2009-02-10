// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILDSTORAGEDB_H_
#define _GUILDSTORAGEDB_H_

#include "../common/mmo.h" // struct guild_storage

typedef struct GuildStorageDB GuildStorageDB;


struct GuildStorageDB
{
	bool (*init)(GuildStorageDB* self);
	void (*destroy)(GuildStorageDB* self);

	bool (*sync)(GuildStorageDB* self);

	bool (*remove)(GuildStorageDB* self, const int guild_id);

	bool (*save)(GuildStorageDB* self, const struct guild_storage* gs, int guild_id);
	bool (*load)(GuildStorageDB* self, struct guild_storage* gs, int guild_id);
};


#endif /* _GUILDSTORAGEDB_H_ */
