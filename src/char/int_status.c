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


int inter_status_tobuf(uint8* buf, size_t size, const struct scdata* sc)
{
	int i;
	size_t p;

	if( size < sc->count * sizeof(struct status_change_data) )
	{// not enough space
		//TODO: error message
		return 0;
	}

	for( i = 0, p = 0; i < sc->count && p < size; i++, p += sizeof(struct status_change_data) )
		memcpy(buf + p, &sc->data[i], sizeof(sc->data[i]));

	return p;
}

bool inter_status_frombuf(uint8* buf, size_t size, struct scdata* sc)
{
	int i;
	size_t p;
	int count = size / sizeof(struct status_change_data);
	
	if( size != count * sizeof(struct status_change_data) )
	{// size mismatch
		//TODO: error message
		return false;
	}

	sc->data = (struct status_change_data*)aMalloc(count*sizeof(struct status_change_data));
	sc->count = count;

	for( i = 0, p = 0; i < count && p < size; i++, p += sizeof(struct status_change_data) )
		memcpy(&sc->data[i], buf + p, sizeof(sc->data[i]));
	
	return true;
}


// Deliver status change data.
void mapif_status_load(int fd, const struct scdata* sc)
{
	WFIFOHEAD(fd, 14 + sc->count * sizeof(struct status_change_data));
	WFIFOW(fd,0) = 0x2b1d;
	WFIFOW(fd,2) = 14 + sc->count * sizeof(struct status_change_data);
	WFIFOL(fd,4) = sc->account_id;
	WFIFOL(fd,8) = sc->char_id;
	WFIFOW(fd,12) = sc->count;
	inter_status_tobuf(WFIFOP(fd,14), sc->count * sizeof(struct status_change_data), sc);
	WFIFOSET(fd, WFIFOW(fd,2));
}


void mapif_parse_StatusLoad(int fd)
{
	int aid = RFIFOL(fd,2);
	int cid = RFIFOL(fd,6);
	struct scdata sc;

	//NOTE: destructive read
	if( !statuses->load(statuses, &sc, cid) )
		return; // no data

	mapif_status_load(fd, &sc);

	aFree(sc.data);
}

void mapif_parse_StatusSave(int fd)
{
	size_t size = RFIFOW(fd,2) - 14;
	int aid = RFIFOL(fd,4);
	int cid = RFIFOL(fd,8);
	int count = RFIFOW(fd,12);
	uint8* scbuf = RFIFOP(fd,14);

	struct scdata sc;
	sc.account_id = aid;
	sc.char_id = cid;
	if( !inter_status_frombuf(scbuf, size, &sc) )
	{// invalid data
		//TODO: error message
		return;
	}

	//NOTE: destructive write
	statuses->save(statuses, &sc);

	aFree(sc.data);
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
