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

/*
bool mercenary_owner_fromsql(int char_id, struct mmo_charstatus *status)
{
	char* data;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith` FROM `mercenary_owner` WHERE `char_id` = '%d'", char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); status->mer_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); status->arch_calls = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); status->arch_faith = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); status->spear_calls = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); status->spear_faith = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); status->sword_calls = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); status->sword_faith = atoi(data);
	Sql_FreeResult(sql_handle);

	return true;
}

bool mercenary_owner_tosql(int char_id, struct mmo_charstatus *status)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `mercenary_owner` (`char_id`, `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith`) VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
		char_id, status->mer_id, status->arch_calls, status->arch_faith, status->spear_calls, status->spear_faith, status->sword_calls, status->sword_faith) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

bool mercenary_owner_delete(int char_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `mercenary_owner` WHERE `char_id` = '%d'", char_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `mercenary` WHERE `char_id` = '%d'", char_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}
*/

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
	bool result = mercs->save(mercs, merc);
	mapif_mercenary_send(fd, merc, result);
}

static void mapif_parse_mercenary_load(int fd, int merc_id, int char_id)
{
	struct s_mercenary merc;
	bool result = mercs->load(mercs, &merc, merc_id); // FIXME: char_id not used
	mapif_mercenary_send(fd, &merc, result);
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


/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_mercenary_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);

	switch( cmd )
	{
		case 0x3070: mapif_parse_mercenary_create(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		case 0x3071: mapif_parse_mercenary_load(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
		case 0x3072: mapif_parse_mercenary_delete(fd, (int)RFIFOL(fd,2)); break;
		case 0x3073: mapif_parse_mercenary_save(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
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

void inter_mercenary_delete(int char_id)
{
}
