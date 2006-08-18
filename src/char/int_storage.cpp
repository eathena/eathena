// $Id: int_storage.c,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#include "baseio.h"
#include "utils.h"
#include "db.h"
#include "lock.h"
#include "mmo.h"
#include "malloc.h"
#include "showmsg.h"

#include "char.h"
#include "inter.h"
#include "int_storage.h"


CGuildStorageDB	cGDStorageDB;
CPCStorageDB	cPCstorageDB;

//---------------------------------------------------------
// 倉庫データを読み込む
int inter_storage_init()
{
	//!! no config file at the moment, will use parameter class later on, live with the defaults now
	cPCstorageDB.init(CHAR_CONF_NAME);
	cGDStorageDB.init(CHAR_CONF_NAME);
	return 0;
}

void inter_storage_final()
{
	return;
}


// 倉庫データ削除
int inter_storage_delete(uint32 account_id)
{
	return cPCstorageDB.removeStorage(account_id);
}

// ギルド倉庫データ削除
int inter_guild_storage_delete(uint32 guild_id)
{
	return cGDStorageDB.removeStorage(guild_id);
}


int mapif_load_storage(int fd, uint32 account_id)
{
	if( session_isActive(fd) )
	{
		CPCStorage pcstor;
		if( cPCstorageDB.searchStorage(account_id, pcstor) )
		{
			WFIFOW(fd,0)=0x3810;
			WFIFOW(fd,2)=sizeof(struct pc_storage)+8;
			WFIFOL(fd,4)=account_id;
			pc_storage_tobuffer(pcstor, WFIFOP(fd,8));
			WFIFOSET(fd,WFIFOW(fd,2));
		}
	}
	return 0;
}

int mapif_save_storage_ack(int fd, uint32 account_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3811;
		WFIFOL(fd,2)=account_id;
		WFIFOB(fd,6)=0;
		WFIFOSET(fd,7);
	}
	return 0;
}

int mapif_load_guild_storage(int fd, uint32 account_id, uint32 guild_id)
{
	if( session_isActive(fd) )
	{
		CGuildStorage gdstor;
		WFIFOW(fd,0)=0x3818;
		if( cGDStorageDB.searchStorage(guild_id, gdstor) )
		{
			WFIFOW(fd,2)=sizeof(struct guild_storage)+12;
			WFIFOL(fd,4)=account_id;
			WFIFOL(fd,8)=guild_id;
			guild_storage_tobuffer(gdstor, WFIFOP(fd,12));
		}
		else
		{
			WFIFOW(fd,2)=12;
			WFIFOL(fd,4)=account_id;
			WFIFOL(fd,8)=0;
		}
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}
int mapif_save_guild_storage_ack(int fd,uint32 account_id,uint32 guild_id,int fail)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3819;
		WFIFOL(fd,2)=account_id;
		WFIFOL(fd,6)=guild_id;
		WFIFOB(fd,10)=fail;
		WFIFOSET(fd,11);
	}
	return 0;
}

//---------------------------------------------------------
int mapif_parse_LoadStorage(int fd)
{
	if( session_isActive(fd) )
	{
		mapif_load_storage(fd,RFIFOL(fd,2));
	}
	return 0;
}
// 倉庫データ受信＆保存
int mapif_parse_SaveStorage(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 account_id=RFIFOL(fd,4);
		unsigned short len=RFIFOW(fd,2);
		if(sizeof(struct pc_storage)!=len-8)
		{
			ShowMessage("inter storage: data size error %d %d\n",sizeof(struct pc_storage),len-8);
		}
		else
		{
			CPCStorage pcstor;
			pc_storage_frombuffer(pcstor, RFIFOP(fd,8));
			cPCstorageDB.saveStorage(pcstor);

			mapif_save_storage_ack(fd,account_id);
		}
	}
	return 0;
}

int mapif_parse_LoadGuildStorage(int fd)
{
	if( session_isActive(fd) )
	{
		mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	}
	return 0;
}
int mapif_parse_SaveGuildStorage(int fd)
{
	if( session_isActive(fd) )
	{
		int guild_id=RFIFOL(fd,8);
		int len=RFIFOW(fd,2);
		if(sizeof(struct guild_storage)!=len-12)
		{
			ShowMessage("inter storage: data size error %d %d\n",sizeof(struct guild_storage),len-12);
		}
		else
		{
			CGuildStorage gdstor;
			guild_storage_frombuffer(gdstor, RFIFOP(fd,12));
			cGDStorageDB.saveStorage(gdstor);

			mapif_save_guild_storage_ack(fd, RFIFOL(fd,4), guild_id, 0);
		}
	}
	return 0;
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_storage_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOW(fd,0)){
	case 0x3010: mapif_parse_LoadStorage(fd); break;
	case 0x3011: mapif_parse_SaveStorage(fd); break;
	case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
	case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
	default:
		return 0;
	}
	return 1;
}
