// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILDDB_H_
#define _GUILDDB_H_

#include "../common/mmo.h" // struct guild

typedef struct GuildDB GuildDB;
typedef struct GuildDBIterator GuildDBIterator;


// fine-grained guild updating
enum guild_save_flags
{
	GS_BASIC      = 0x0001,
	GS_MEMBER     = 0x0002,
	GS_POSITION   = 0x0004,
	GS_ALLIANCE   = 0x0008,
	GS_EXPULSION  = 0x0010,
	GS_SKILL      = 0x0020,
	GS_EMBLEM     = 0x0040,
	GS_CONNECT    = 0x0080,
	GS_LEVEL      = 0x0100,
	GS_MES        = 0x0200,

	GS_BASIC_MASK = GS_BASIC | GS_EMBLEM | GS_CONNECT | GS_LEVEL | GS_MES,
};


// guild member-related flags
#define GS_MEMBER_UNMODIFIED 0x00
#define GS_MEMBER_MODIFIED 0x01
#define GS_MEMBER_NEW 0x02
#define GS_MEMBER_DELETED 0x04
// guild position-related flags
#define GS_POSITION_UNMODIFIED 0x00
#define GS_POSITION_MODIFIED 0x01


struct GuildDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(GuildDBIterator* self);

	/// Fetches the next guild data and stores it in 'data'.
	/// @param self Iterator
	/// @param data a guild's data
	/// @return true if successful
	bool (*next)(GuildDBIterator* self, struct guild* data);
};


struct GuildDB
{
	bool (*init)(GuildDB* self);

	void (*destroy)(GuildDB* self);

	bool (*sync)(GuildDB* self);

	bool (*create)(GuildDB* self, struct guild* g);

	bool (*remove)(GuildDB* self, const int guild_id);

	bool (*save)(GuildDB* self, const struct guild* p, enum guild_save_flags flag);

	bool (*load)(GuildDB* self, struct guild* p, int guild_id);

	bool (*name2id)(GuildDB* self, const char* name, int* guild_id);

	/// Returns an iterator over all guilds.
	///
	/// @param self Database
	/// @return Iterator
	GuildDBIterator* (*iterator)(GuildDB* self);
};


#endif /* _GUILDDB_H_ */
