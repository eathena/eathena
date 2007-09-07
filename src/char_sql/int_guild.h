// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_GUILD_SQL_H_
#define _INT_GUILD_SQL_H_

struct guild;
struct guild_castle;

int inter_guild_parse_frommap(int fd);
int inter_guild_sql_init(void);
void inter_guild_sql_final(void);
int inter_guild_mapif_init(int fd);
int inter_guild_leave(int guild_id,int account_id,int char_id);
int mapif_parse_BreakGuild(int fd,int guild_id);
int inter_guild_broken(int guild_id);
int inter_guild_sex_changed(int guild_id,int account_id,int char_id, int gender);
int inter_guild_CharOnline(int char_id, int guild_id);
int inter_guild_CharOffline(int char_id, int guild_id);

//For the TXT->SQL converter.
int inter_guild_tosql(struct guild *g,int flag);
int inter_guildcastle_tosql(struct guild_castle *gc);

#endif /* _INT_GUILD_SQL_H_ */
