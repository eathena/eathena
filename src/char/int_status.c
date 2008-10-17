// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "statusdb.h"
#include <string.h>

// status database
StatusDB* statuses = NULL;


void inter_status_init(void)
{
#ifdef TXT_ONLY
	statuses = status_db_txt();
#else
	statuses = status_db_sql();
#endif

	statuses->init(statuses);
}

void inter_status_final(void)
{
	statuses->destroy(statuses);
}

void inter_status_sync(void)
{
	statuses->sync(statuses);
}

bool inter_status_delete(int char_id)
{
	return statuses->remove(statuses, char_id);
}

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

int inter_status_parse_frommap(int fd)
{
}
