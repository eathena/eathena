// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_H_
#define _INTER_H_

#include "charserverdb.h"

void inter_init(CharServerDB* db);
void inter_final(void);
int inter_parse_frommap(int fd);
void inter_mapif_init(int fd);
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);
void log_inter(const char* msg, ...);

// shared inter-server config settings
extern unsigned int party_share_level;
extern bool party_break_without_leader;
extern bool party_auto_reassign_leader;
extern char main_chat_nick[16];

#endif /* _INTER_H_ */
