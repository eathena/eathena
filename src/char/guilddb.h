// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILDDB_H_
#define _GUILDDB_H_

#include "../common/mmo.h" // struct guild

typedef struct GuildDB GuildDB;

// standard engines
#ifdef WITH_TXT
GuildDB* guild_db_txt(void);
#endif
#ifdef WITH_SQL
GuildDB* guild_db_sql(void);
#endif


struct GuildDB
{
	bool (*init)(GuildDB* self);

	void (*destroy)(GuildDB* self);

	bool (*sync)(GuildDB* self);

	bool (*create)(GuildDB* self, struct guild* g);

	bool (*remove)(GuildDB* self, const int guild_id);

	bool (*save)(GuildDB* self, const struct guild* p);

	bool (*load_num)(GuildDB* self, struct guild* p, int guild_id);

	bool (*name2id)(GuildDB* self, struct guild* p, const char* name);
};


#endif /* _GUILDDB_H_ */
