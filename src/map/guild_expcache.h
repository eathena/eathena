// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILD_EXPCACHE_H_
#define _GUILD_EXPCACHE_H_


unsigned int guild_addexp(int guild_id, int account_id, int char_id, unsigned int exp);
void do_init_guild_expcache(void);
void do_final_guild_expcache(void);


#endif // _GUILD_EXPCACHE_H_
