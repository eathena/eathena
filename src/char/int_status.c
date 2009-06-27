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


// Deliver status change data.
static void mapif_status_load(int fd, const struct scdata* sc)
{
	WFIFOHEAD(fd, 14 + sc->count * sizeof(struct status_change_data));
	WFIFOW(fd,0) = 0x2b1d;
	WFIFOW(fd,2) = 14 + sc->count * sizeof(struct status_change_data);
	WFIFOL(fd,4) = sc->account_id;
	WFIFOL(fd,8) = sc->char_id;
	WFIFOW(fd,12) = sc->count;
	memcpy(WFIFOP(fd,14), sc->data, sc->count * sizeof(struct status_change_data));
	WFIFOSET(fd, WFIFOW(fd,2));
}


static void mapif_parse_StatusLoad(int fd)
{
	int aid = RFIFOL(fd,2);
	int cid = RFIFOL(fd,6);
	struct scdata sc;

	if( !statuses->load(statuses, &sc, cid) )
		return; // no data

	sc.account_id = aid; // here for compatibility reasons

	mapif_status_load(fd, &sc);

	if( sc.data != NULL )
		aFree(sc.data);
}

static void mapif_parse_StatusSave(int fd)
{
	size_t size = RFIFOW(fd,2) - 14;
	int aid = RFIFOL(fd,4);
	int cid = RFIFOL(fd,8);
	int count = RFIFOW(fd,12);
	uint8* scbuf = RFIFOP(fd,14);
	struct scdata sc;

	if( size != count * sizeof(struct status_change_data) )
	{// size mismatch
		//TODO: error message
		return;
	}

	sc.account_id = aid;
	sc.char_id = cid;
	sc.count = count;
	sc.data = (struct status_change_data*)scbuf;

	statuses->save(statuses, &sc);
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

bool inter_status_delete(int char_id)
{
	return statuses->remove(statuses, char_id);
}
