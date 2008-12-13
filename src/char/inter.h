// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_H_
#define _INTER_H_

#include "charserverdb.h"

int inter_init(CharServerDB* db);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);

extern unsigned int party_share_level;
extern char main_chat_nick[16];

#ifdef TXT_ONLY
int inter_save(void);
//For TXT->SQL conversion
extern char accreg_txt[];
#else
#include "../common/sql.h"
extern Sql* sql_handle;
extern Sql* lsql_handle;

extern char char_db[256];
extern char scdata_db[256];
extern char cart_db[256];
extern char inventory_db[256];
extern char storage_db[256];
extern char reg_db[256];
extern char skill_db[256];
extern char memo_db[256];
extern char guild_db[256];
extern char guild_alliance_db[256];
extern char guild_castle_db[256];
extern char guild_expulsion_db[256];
extern char guild_member_db[256];
extern char guild_position_db[256];
extern char guild_skill_db[256];
extern char guild_storage_db[256];
extern char party_db[256];
extern char friend_db[256];
extern char pet_db[256];
extern char mail_db[256];
extern char hotkey_db[256];
extern char auction_db[256];
extern char quest_db[256];
extern char quest_obj_db[256];

#endif

#endif /* _INTER_H_ */
