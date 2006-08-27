// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_GUILD_H_
#define _INT_GUILD_H_

int inter_guild_init();
void inter_guild_final();

int inter_guild_parse_frommap(int fd);


int inter_guild_mapif_init(int fd);

int inter_guild_leave(uint32 guild_id,uint32 account_id,uint32 char_id);


#endif
