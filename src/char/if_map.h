// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _IF_MAP_H_
#define _IF_MAP_H_

#include "../common/mmo.h"
#include "../common/cookie.h" // struct s_cookie

#define MAX_MAP_SERVERS 30

#define MAPSERVER_TIMEOUT 60000
struct mmo_map_server
{
	int fd;
	uint32 ip;
	uint16 port;
	int users;
	unsigned short map[MAX_MAP_PER_SERVER];
	struct s_cookie cookie;
};

extern struct mmo_map_server server[MAX_MAP_SERVERS];

int parse_frommap(int fd);
int mapif_send(int fd, const void* buf, unsigned int len);
int mapif_sendallwos(int fd, const void* buf, unsigned int len);
int mapif_sendall(const void* buf, unsigned int len);

void mapif_server_init(int id);
void mapif_server_destroy(int id);
void mapif_server_reset(int id);
void mapif_cookie_generate(int id);
void mapif_cookie_clear(int id);

void do_init_mapif(void);
void do_final_mapif(void);


#endif /* _IF_MAP_H_ */
