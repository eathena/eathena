// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "guild_expcache.h"
#include "guild.h" // struct guild, GMI_EXP, guild_search(), guild_getindex()
#include "intif.h" // intif_guild_change_memberinfo()
#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/ers.h"
#include "../common/timer.h"


struct guild_expcache
{
	int guild_id;
	int account_id;
	int char_id;
	uint64 exp;
};


static struct eri* expcache_ers = NULL; // for handling of guild exp updates
static DBMap* guild_expcache_db = NULL; // int char_id -> struct guild_expcache*
#define GUILD_ADDEXP_INVERVAL 10000 // interval for flushing exp cache


static void* create_expcache(DBKey key, va_list args)
{
	int guild_id = va_arg(args, int);
	int account_id = va_arg(args, int);
	int char_id = va_arg(args, int);

	struct guild_expcache* c = ers_alloc(expcache_ers, struct guild_expcache);
	c->guild_id = guild_id;
	c->account_id = account_id;
	c->char_id = char_id;
	c->exp = 0;
	return c;
}


static int guild_expcache_db_final(DBKey key, void* data, va_list args)
{
	ers_free(expcache_ers, data);
	return 0;
}


// Flush guild exp cache to interserver.
static int guild_addexp_timer_sub(DBKey dataid, void* data, va_list ap)
{
	struct guild_expcache* c = (struct guild_expcache*)data;

	struct guild* g = guild_search(c->guild_id);
	if( g != NULL )
	{
		int i = guild_getindex(g, c->account_id, c->char_id);
		if( i >= 0 )
		{
			if( g->member[i].exp > UINT64_MAX - c->exp )
				g->member[i].exp = UINT64_MAX;
			else
				g->member[i].exp += c->exp;

			intif_guild_change_memberinfo(g->guild_id, c->account_id, c->char_id, GMI_EXP, &g->member[i].exp, sizeof(g->member[i].exp));
			c->exp = 0;
		}
	}

	ers_free(expcache_ers, data);
	return 0;
}


static int guild_addexp_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	guild_expcache_db->clear(guild_expcache_db, guild_addexp_timer_sub);
	return 0;
}


/// Increase this player's exp contribution to his guild.
unsigned int guild_addexp(int guild_id, int account_id, int char_id, unsigned int exp)
{
	struct guild_expcache* c = (struct guild_expcache*)guild_expcache_db->ensure(guild_expcache_db, db_i2key(char_id), create_expcache, guild_id, account_id, char_id);

	if( c->exp > UINT64_MAX - exp )
		c->exp = UINT64_MAX;
	else
		c->exp += exp;

	return exp;
}


void do_init_guild_expcache(void)
{
	guild_expcache_db = idb_alloc(DB_OPT_BASE);
	expcache_ers = ers_new(sizeof(struct guild_expcache));

	add_timer_func_list(guild_addexp_timer, "guild_addexp_timer");
	add_timer_interval(gettick() + GUILD_ADDEXP_INVERVAL, guild_addexp_timer, 0, 0, GUILD_ADDEXP_INVERVAL);
}


void do_final_guild_expcache(void)
{
	guild_expcache_db->destroy(guild_expcache_db, guild_expcache_db_final);
	ers_destroy(expcache_ers);
}
