// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "statusdb.h"
#include <string.h>

// status database
static StatusDB* statuses = NULL;


static void mapif_parse_StatusLoad(int fd)
{
	int cid = RFIFOL(fd,2);
	size_t size = statuses->size(statuses, cid);

	WFIFOHEAD(fd, 10 + size * sizeof(struct status_change_data));
	WFIFOW(fd,0) = 0x2b1d;
	WFIFOW(fd,2) = 10 + size * sizeof(struct status_change_data);
	WFIFOL(fd,4) = cid;
	WFIFOW(fd,8) = size;
	statuses->load(statuses, (struct status_change_data*)WFIFOP(fd,10), size, cid);
	WFIFOSET(fd, WFIFOW(fd,2));
}


static void mapif_parse_StatusSave(int fd)
{
	size_t size = RFIFOW(fd,2) - 10;
	int cid = RFIFOL(fd,4);
	int count = RFIFOW(fd,8);
	struct status_change_data* sc = (struct status_change_data*)RFIFOP(fd,10);

	if( size != count * sizeof(struct status_change_data) )
	{// size mismatch
		//TODO: error message
		return;
	}

	statuses->save(statuses, sc, count, cid);
}


int inter_status_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x30A0: mapif_parse_StatusLoad(fd); break;
	case 0x30A1: mapif_parse_StatusSave(fd); break;
	default:
		return 0;
	}
	return 1;
}


void inter_status_init(StatusDB* db)
{
	statuses = db;
}


void inter_status_final(void)
{
	statuses = NULL;
}
