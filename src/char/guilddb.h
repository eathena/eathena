// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILDDB_H_
#define _GUILDDB_H_

#include "../common/mmo.h" // struct guild


// fine-grained guild updating
#define GS_BASIC 0x0001
#define GS_MEMBER 0x0002
#define GS_POSITION 0x0004
#define GS_ALLIANCE 0x0008
#define GS_EXPULSION 0x0010
#define GS_SKILL 0x0020
#define GS_EMBLEM 0x0040
#define GS_CONNECT 0x0080
#define GS_LEVEL 0x0100
#define GS_MES 0x0200
#define GS_MASK 0x03FF
#define GS_BASIC_MASK (GS_BASIC | GS_EMBLEM | GS_CONNECT | GS_LEVEL | GS_MES)
// guild member-related flags
#define GS_MEMBER_UNMODIFIED 0x00
#define GS_MEMBER_MODIFIED 0x01
#define GS_MEMBER_NEW 0x02
#define GS_MEMBER_DELETED 0x04
// guild position-related flags
#define GS_POSITION_UNMODIFIED 0x00
#define GS_POSITION_MODIFIED 0x01


typedef struct GuildDB GuildDB;


struct GuildDB
{
	bool (*init)(GuildDB* self);

	void (*destroy)(GuildDB* self);

	bool (*sync)(GuildDB* self);

	bool (*create)(GuildDB* self, struct guild* g);

	bool (*remove)(GuildDB* self, const int guild_id);

	bool (*save)(GuildDB* self, const struct guild* p);

	bool (*load)(GuildDB* self, struct guild* p, int guild_id);

	bool (*name2id)(GuildDB* self, const char* name, int* guild_id);
};


#endif /* _GUILDDB_H_ */
