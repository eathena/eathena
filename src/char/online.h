// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ONLINE_H_
#define _ONLINE_H_

#include "../common/db.h"
#include "onlinedb.h"


// online user tracking system
struct online_char_data
{
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect; // timer id
	short server; // -2: unknown server, -1: not connected, 0+: id of server
};


void onlinedb_create(void);
void onlinedb_init(void);
void onlinedb_final(void);
void onlinedb_sync(void);
void onlinedb_config_read(const char* w1, const char* w2);
unsigned int onlinedb_size(void);
struct online_char_data* onlinedb_get(int account_id);
struct online_char_data* onlinedb_ensure(int account_id);
DBIterator* onlinedb_iterator(void);
void onlinedb_mapserver_offline(int map_id);
void onlinedb_mapserver_unknown(int map_id);

void set_all_offline(void);
void set_char_online(int map_id, int char_id, int account_id);
void set_char_offline(int char_id, int account_id);
void set_char_charselect(int account_id);
void set_char_waitdisconnect(int account_id, unsigned int time);


#endif /* _ONLINE_H_ */
