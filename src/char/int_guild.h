// $Id: int_guild.h,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#ifndef _INT_GUILD_H_
#define _INT_GUILD_H_

int inter_guild_init();
void inter_guild_final();

int inter_guild_parse_frommap(int fd);


int inter_guild_mapif_init(int fd);

int inter_guild_leave(uint32 guild_id,uint32 account_id,uint32 char_id);


#endif
