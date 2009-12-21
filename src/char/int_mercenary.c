// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/utils.h"
#include "char.h"
#include "mercdb.h"
#include "inter.h"
#include "int_mercenary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// mercenary database
static MercDB* mercs = NULL;


static void mapif_mercenary_send(int fd, struct s_mercenary *merc, unsigned char flag)
{
	int size = sizeof(struct s_mercenary) + 5;

	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x3870;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),merc,sizeof(struct s_mercenary));
	WFIFOSET(fd,size);
}

static void mapif_parse_mercenary_create(int fd, struct s_mercenary* merc)
{
	bool result;
	merc->mercenary_id = -1; // autogenerate
	result = mercs->create(mercs, merc);
	mapif_mercenary_send(fd, merc, result);
}

static void mapif_parse_mercenary_load(int fd, int merc_id, int char_id)
{
	struct s_mercenary merc;
	if( mercs->load(mercs, &merc, merc_id) )
	{
		mapif_mercenary_send(fd, &merc, true);
	}
	else
	{
		memset(&merc, 0x00, sizeof(merc));
		merc.char_id = char_id;
		mapif_mercenary_send(fd, &merc, false);
	}
}

static void mapif_mercenary_deleted(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3871;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

static void mapif_parse_mercenary_delete(int fd, int merc_id)
{
	bool result = mercs->remove(mercs, merc_id);
	mapif_mercenary_deleted(fd, result);
}

static void mapif_mercenary_saved(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3872;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

static void mapif_parse_mercenary_save(int fd, struct s_mercenary* merc)
{
	bool result = mercs->save(mercs, merc);
	mapif_mercenary_saved(fd, result);
}


/// Inter Packets.
int inter_mercenary_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);

	switch( cmd )
	{
		case 0x30C0: mapif_parse_mercenary_create(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		case 0x30C1: mapif_parse_mercenary_load(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
		case 0x30C2: mapif_parse_mercenary_delete(fd, (int)RFIFOL(fd,2)); break;
		case 0x30C3: mapif_parse_mercenary_save(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}

void inter_mercenary_init(MercDB* db)
{
	mercs = db;
}

void inter_mercenary_final()
{
	mercs = NULL;
}
