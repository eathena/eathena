// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "char.h"
#include "inter.h"
#include "int_storage.h"
#include "int_pet.h"
#include "int_guild.h"
#include "storagedb.h"
#include "guildstoragedb.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static StorageDB* storages = NULL;
static GuildStorageDB* guildstorages = NULL;


//---------------------------------------------------------
// char -> map packets

static int mapif_load_guild_storage(int fd,int account_id,int guild_id)
{
	WFIFOHEAD(fd, sizeof(struct guild_storage)+12);
	WFIFOW(fd,0) = 0x3818;

	//NOTE: writes directly to socket buffer
	if( guildstorages->load(guildstorages, (struct guild_storage*)WFIFOP(fd,12), guild_id) )
	{
		WFIFOW(fd,2) = 12 + sizeof(struct guild_storage);
		WFIFOL(fd,4) = account_id;
		WFIFOL(fd,8) = guild_id;
	}
	else
	{// data failed to load
		WFIFOW(fd,2) = 12;
		WFIFOL(fd,4) = account_id;
		WFIFOL(fd,8) = 0;
	}

	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

static int mapif_save_guild_storage_ack(int fd,int account_id,int guild_id,int fail)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0)=0x3819;
	WFIFOL(fd,2)=account_id;
	WFIFOL(fd,6)=guild_id;
	WFIFOB(fd,10)=fail;
	WFIFOSET(fd,11);
	return 0;
}

//---------------------------------------------------------
// map -> char packets

static void mapif_parse_LoadGuildStorage(int fd)
{
	int account_id = RFIFOL(fd,2);
	int guild_id = RFIFOL(fd,6);

	mapif_load_guild_storage(fd,account_id,guild_id);
}

static void mapif_parse_SaveGuildStorage(int fd)
{
	int len = RFIFOW(fd,2);
	int account_id = RFIFOL(fd,4);
	int guild_id = RFIFOL(fd,8);
	struct guild_storage* gs = (struct guild_storage*)RFIFOP(fd,12);

	if( sizeof(struct guild_storage) != len - 12 )
	{
		ShowError("inter storage: data size error %d != %d\n", sizeof(struct guild_storage), len - 12);
		mapif_save_guild_storage_ack(fd, account_id, guild_id, 1);
		return;
	}

	if( guildstorages->save(guildstorages, gs, guild_id) )
		mapif_save_guild_storage_ack(fd,account_id,guild_id,0);
	else
		mapif_save_guild_storage_ack(fd,account_id,guild_id,1);
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_storage_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
	case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
	default:
		return 0;
	}
	return 1;
}

void inter_storage_init(StorageDB* sdb, GuildStorageDB* gsdb)
{
	storages = sdb;
	guildstorages = gsdb;
}

void inter_storage_final(void)
{
	storages = NULL;
	guildstorages = NULL;
}

bool inter_storage_delete(int account_id)
{
	struct storage_data s;
	int i;

	if( !storages->load(storages, &s, account_id) )
		return false;

	for( i = 0; i < s.storage_amount; i++ )
		if( s.items[i].card[0] == (short)0xff00 )
			inter_pet_delete( MakeDWord(s.items[i].card[1],s.items[i].card[2]) );

	storages->remove(storages, account_id);

	return true;
}

int inter_guild_storage_delete(int guild_id)
{
	struct guild_storage gs;
	int i;

	if( !guildstorages->load(guildstorages, &gs, guild_id) )
		return false;

	for( i = 0; i < gs.storage_amount; i++ )
		if( gs.storage_[i].card[0] == (short)0xff00 )
			inter_pet_delete( MakeDWord(gs.storage_[i].card[1],gs.storage_[i].card[2]) );

	guildstorages->remove(guildstorages, guild_id);

	return true;
}
