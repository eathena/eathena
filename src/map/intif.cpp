// $Id: intif.c,v 1.2 2004/09/25 05:32:18 MouseJstr Exp $
#include "socket.h"
#include "timer.h"
#include "map.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "intif.h"
#include "storage.h"
#include "party.h"
#include "guild.h"
#include "pet.h"
#include "homun.h"
#include "nullpo.h"
#include "showmsg.h"
#include "utils.h"
#include "malloc.h"
static const int packet_len_table[]={
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3000-
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0,	// 0x3010-
	35,-1,11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3020-
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,	// 0x3030-
	 9, 9,-1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3040-
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3050-
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3060-
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3070-
	11,-1, 7, 3,  0, 0, 0, 0,  0,-1, 3, 3,  0, 0,  0, 0,	// 0x3080-
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3090-
};


extern int char_fd;		// inter serverのfdはchar_fdを使う

//-----------------------------------------------------------------
// inter serverへの送信


// pet
int intif_create_pet(uint32 account_id,uint32 char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incuvate,const char *pet_name)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3080;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOW(char_fd,10) = pet_class;
	WFIFOW(char_fd,12) = pet_lv;
	WFIFOW(char_fd,14) = pet_egg_id;
	WFIFOW(char_fd,16) = pet_equip;
	WFIFOW(char_fd,18) = intimate;
	WFIFOW(char_fd,20) = hungry;
	WFIFOB(char_fd,22) = rename_flag;
	WFIFOB(char_fd,23) = incuvate;
	memcpy(WFIFOP(char_fd,24),pet_name,24);
	WFIFOSET(char_fd,48);

	return 0;
}

int intif_request_petdata(uint32 account_id,uint32 char_id,uint32 pet_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3081;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOL(char_fd,10) = pet_id;
	WFIFOSET(char_fd,14);

	return 0;
}

int intif_save_petdata(uint32 account_id,struct petstatus &pet)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3082;
	WFIFOW(char_fd,2) = sizeof(struct petstatus) + 8;
	WFIFOL(char_fd,4) = account_id;
	//memcpy(WFIFOP(char_fd,8),p,sizeof(struct petstatus));
	s_pet_tobuffer(pet, WFIFOP(char_fd,8));
	WFIFOSET(char_fd,WFIFOW(char_fd,2));

	return 0;
}

int intif_delete_petdata(uint32 pet_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3083;
	WFIFOL(char_fd,2) = pet_id;
	WFIFOSET(char_fd,6);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// homunculus
///////////////////////////////////////////////////////////////////////////////
int intif_create_homdata(uint32 account_id, uint32 char_id, const homun_data &hd)
{
	WFIFOW(char_fd,0) = 0x3088;
	WFIFOW(char_fd,2) = sizeof(struct homunstatus) + 12;
	WFIFOL(char_fd,4) = account_id;
	WFIFOL(char_fd,8) = char_id;
	homun_tobuffer(hd.status, WFIFOP(char_fd,12));
	WFIFOSET(char_fd,sizeof(struct homunstatus) + 12);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
int intif_request_homdata(uint32 account_id,uint32 char_id,uint32 homun_id)
{
	WFIFOW(char_fd,0) = 0x3089;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOL(char_fd,10)= homun_id;
	WFIFOSET(char_fd,14);

	return 0;
}
int intif_parse_RecvHomun(int fd)
{
	int len=RFIFOW(fd,2);
	if( 5 == len || 1==RFIFOB(fd,4) )
	{
		if(config.etc_log)
			ShowError("intif: hom data create/request failed\n");

	}
	else if( 5+sizeof(struct homunstatus) != len )
	{
		if(config.etc_log)
			ShowError("intif: hom data: data size error %d %d\n",sizeof(struct homunstatus),len-5);
	}
	else
	{
		struct homunstatus tmp;
		homun_frombuffer(tmp, RFIFOP(fd,5));
		homun_data::recv_homunculus(tmp, 0);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int intif_save_homdata(uint32 account_id, uint32 char_id, const homun_data &hd)
{
	WFIFOW(char_fd,0) = 0x308a;
	WFIFOW(char_fd,2) = sizeof(struct homunstatus) + 12;
	WFIFOL(char_fd,4) = account_id;
	WFIFOL(char_fd,8) = char_id;
	homun_tobuffer(hd.status, WFIFOP(char_fd,12));
	WFIFOSET(char_fd,sizeof(struct homunstatus) + 12);

	return 0;
}
int intif_parse_SaveHomun(int fd)
{
	if(RFIFOB(fd,2) == 1)
	{
		if(config.error_log)
			ShowError("hom data saving failed\n");
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int intif_delete_homdata(uint32 account_id,uint32 char_id,uint32 homun_id)
{
	WFIFOW(char_fd,0) = 0x308b;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = char_id;
	WFIFOL(char_fd,10) = homun_id;
	WFIFOSET(char_fd,14);

	return 0;
}
int intif_parse_DeleteHomun(int fd)
{
	if(RFIFOB(fd,2) == 1)
	{
		if(config.error_log)
			ShowError("hom data deletion failed\n");
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////


// GMメッセージを送信
int intif_GMmessage(const char* mes, size_t len, int flag)
{
	if(mes)
	{
		int lp = (flag&0x10) ? 8 : 4;
		if( !session_isActive(char_fd) )
			return 0;
		WFIFOW(char_fd,0) = 0x3000;
		WFIFOW(char_fd,2) = lp + len;
		WFIFOL(char_fd,4) = 0x65756c62;
		memcpy(WFIFOP(char_fd,lp), mes, len);
		WFIFOSET(char_fd, lp + len);

		// Send to the local players
		clif_GMmessage(NULL, mes, len, flag);
	}
	return 0;
}

// The transmission of Wisp/Page to inter-server (player not found on this server)
int intif_wis_message(struct map_session_data &sd, const char *nick, const char *mes, size_t len)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3001;
	WFIFOW(char_fd,2) = len + 52;
	memcpy(WFIFOP(char_fd,4), sd.status.name, 24);
	memcpy(WFIFOP(char_fd,28), nick, 24);
	memcpy(WFIFOP(char_fd,52), mes, len);
	WFIFOB(char_fd,52+len-1) = 0;
	WFIFOSET(char_fd, len + 52);

	if(config.etc_log)
		ShowMessage("intif_wis_message from %s to %s (message: '%s')\n", sd.status.name, nick, mes);

	return 0;
}

// The reply of Wisp/page
int intif_wis_replay(int id, int flag)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3002;
	WFIFOL(char_fd,2) = id;
	WFIFOB(char_fd,6) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	WFIFOSET(char_fd,7);

	if(config.etc_log)
		ShowMessage("intif_wis_replay: id: %d, flag:%d\n", id, flag);

	return 0;
}

// The transmission of GM only Wisp/Page from server to inter-server
int intif_wis_message_to_gm(const char *Wisp_name, int min_gm_level, const char *mes)
{
	int mes_len;
	if( !session_isActive(char_fd) )
		return 0;
	mes_len = strlen(mes) + 1; // + null
	WFIFOW(char_fd,0) = 0x3003;
	WFIFOW(char_fd,2) = mes_len + 30;
	memcpy(WFIFOP(char_fd,4), Wisp_name, 24);
	WFIFOW(char_fd,28) = (short)min_gm_level;
	memcpy(WFIFOP(char_fd,30), mes, mes_len);
	WFIFOSET(char_fd, WFIFOW(char_fd,2));

	if(config.etc_log)
		ShowMessage("intif_wis_message_to_gm: from: '%s', min level: %d, message: '%s'.\n", Wisp_name, min_gm_level, mes);

	return 0;
}

// アカウント変数送信
int intif_saveaccountreg(struct map_session_data &sd)
{
	size_t j;
	unsigned short p;
	if( !session_isActive(char_fd) )
		return 0;

	if( sd.status.account_reg_num == 0xFFFFFFFF )
		return 0;

	WFIFOW(char_fd,0) = 0x3004;
	WFIFOL(char_fd,4) = sd.block_list::id;
	for(j=0,p=8;j<sd.status.account_reg_num;j++,p+=36){
		memcpy(WFIFOP(char_fd,p),sd.status.account_reg[j].str,32);
		WFIFOL(char_fd,p+32)=sd.status.account_reg[j].value;
	}
	WFIFOW(char_fd,2)=p;
	WFIFOSET(char_fd,p);
	return 0;
}
// アカウント変数要求
int intif_request_accountreg(struct map_session_data &sd)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3005;
	WFIFOL(char_fd,2) = sd.block_list::id;
	WFIFOSET(char_fd,6);

	sd.status.account_reg_num = 0xFFFFFFFF;

	return 0;
}

// 倉庫データ要求
int intif_request_storage(uint32 account_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3010;
	WFIFOL(char_fd,2) = account_id;
	WFIFOSET(char_fd,6);
	return 0;
}
// 倉庫データ送信
int intif_send_storage(struct pc_storage &stor)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3011;
	WFIFOW(char_fd,2) = sizeof(struct pc_storage)+8;
	WFIFOL(char_fd,4) = stor.account_id;
	pc_storage_tobuffer(stor, WFIFOP(char_fd,8));
	WFIFOSET(char_fd,WFIFOW(char_fd,2));

	return 0;
}

int intif_request_guild_storage(uint32 account_id,uint32 guild_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3018;
	WFIFOL(char_fd,2) = account_id;
	WFIFOL(char_fd,6) = guild_id;
	WFIFOSET(char_fd,10);
	return 0;
}
int intif_send_guild_storage(uint32 account_id,struct guild_storage &gstor)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3019;
	WFIFOW(char_fd,2) = sizeof(struct guild_storage)+12;
	WFIFOL(char_fd,4) = account_id;
	WFIFOL(char_fd,8) = gstor.guild_id;
	guild_storage_tobuffer(gstor, WFIFOP(char_fd,12));
	WFIFOSET(char_fd,WFIFOW(char_fd,2));

	return 0;
}

// パーティ作成要求
int intif_create_party(struct map_session_data &sd,const char *name,int item,int item2)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3020;
	WFIFOL(char_fd,2) = sd.status.account_id;
	memcpy(WFIFOP(char_fd, 6),name,24);
	memcpy(WFIFOP(char_fd,30),sd.status.name,24);
	memcpy(WFIFOP(char_fd,54),maps[sd.block_list::m].mapname,16);
	WFIFOW(char_fd,70)= sd.status.base_level;
	WFIFOB(char_fd,72)= item;
	WFIFOB(char_fd,73)= item2;
	WFIFOSET(char_fd,74);
//	if(config.etc_log)
//		ShowMessage("intif: create party\n");
	return 0;
}
// パーティ情報要求
int intif_request_partyinfo(uint32 party_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3021;
	WFIFOL(char_fd,2) = party_id;
	WFIFOSET(char_fd,6);
//	if(config.etc_log)
//		ShowMessage("intif: request party info\n");
	return 0;
}
// パーティ追加要求
int intif_party_addmember(uint32 party_id,uint32 account_id)
{
	struct map_session_data *sd;
	if( !session_isActive(char_fd) )
		return 0;
	sd=map_id2sd(account_id);
//	if(config.etc_log)
//		ShowMessage("intif: party add member %d %d\n",party_id,account_id);
	if(sd!=NULL){
		WFIFOW(char_fd,0)=0x3022;
		WFIFOL(char_fd,2)=party_id;
		WFIFOL(char_fd,6)=account_id;
		memcpy(WFIFOP(char_fd,10),sd->status.name,24);
		memcpy(WFIFOP(char_fd,34),maps[sd->block_list::m].mapname,16);
		WFIFOW(char_fd,50)=sd->status.base_level;
		WFIFOSET(char_fd,52);
	}
	return 0;
}
// パーティ設定変更
int intif_party_changeoption(uint32 party_id,uint32 account_id,unsigned short expshare,unsigned short itemshare)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x3023;
	WFIFOL(char_fd,2)=party_id;
	WFIFOL(char_fd,6)=account_id;
	WFIFOW(char_fd,10)=expshare;
	WFIFOW(char_fd,12)=itemshare;
	WFIFOSET(char_fd,14);
	return 0;
}
// パーティ脱退要求
int intif_party_leave(uint32 party_id,uint32 account_id)
{
	if( !session_isActive(char_fd) )
		return 0;
//	if(config.etc_log)
//		ShowMessage("intif: party leave %d %d\n",party_id,account_id);
	WFIFOW(char_fd,0)=0x3024;
	WFIFOL(char_fd,2)=party_id;
	WFIFOL(char_fd,6)=account_id;
	WFIFOSET(char_fd,10);
	return 0;
}
// パーティ移動要求
int intif_party_changemap(struct map_session_data *sd,int online)
{
	if( !session_isActive(char_fd) )
		return 0;
	if(sd!=NULL){
	    WFIFOW(char_fd,0)=0x3025;
	    WFIFOL(char_fd,2)=sd->status.party_id;
	    WFIFOL(char_fd,6)=sd->status.account_id;
	    memcpy(WFIFOP(char_fd,10),maps[sd->block_list::m].mapname,16);
	    WFIFOB(char_fd,26)=online;
	    WFIFOW(char_fd,27)=sd->status.base_level;
	    WFIFOSET(char_fd,29);
	}
//	if(config.etc_log)
//		ShowMessage("party: change map\n");
	return 0;
}
// パーティー解散要求
int intif_break_party(int party_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x3026;
	WFIFOL(char_fd,2)=party_id;
	WFIFOSET(char_fd,6);
	return 0;
}
// パーティ会話送信
int intif_party_message(uint32 party_id,uint32 account_id,const char *mes,size_t len)
{
	if( !session_isActive(char_fd) )
		return 0;
//	if(config.etc_log)
//		ShowMessage("intif_party_message: %s\n",mes);
	if( (party_id > 0) && (party_search(party_id) != NULL) && 
		(mes != NULL) && (len > 0) )
	{ 
		WFIFOW(char_fd,0)=0x3027;
		WFIFOW(char_fd,2)=len+12;
		WFIFOL(char_fd,4)=party_id;
		WFIFOL(char_fd,8)=account_id;
		memcpy(WFIFOP(char_fd,12),mes,len);
		WFIFOSET(char_fd,len+12);
	}
	return 0;
}
// パーティ競合チェック要求
int intif_party_checkconflict(uint32 party_id,uint32 account_id,char *nick)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x3028;
	WFIFOL(char_fd,2)=party_id;
	WFIFOL(char_fd,6)=account_id;
	memcpy(WFIFOP(char_fd,10),nick,24);
	WFIFOSET(char_fd,34);
	return 0;
}

// ギルド作成要求
int intif_guild_create(const char *name,const struct guild_member &master)
{
	if( !session_isActive(char_fd) )
		return 0;
	
	WFIFOW(char_fd,0)=0x3030;
	WFIFOW(char_fd,2)=sizeof(struct guild_member)+32;
	WFIFOL(char_fd,4)=master.account_id;
	memcpy(WFIFOP(char_fd,8),name,24);

	guild_member_tobuffer(master, WFIFOP(char_fd,32));

	WFIFOSET(char_fd,WFIFOW(char_fd,2));

	return 0;
}
// ギルド情報要求
int intif_guild_request_info(uint32 guild_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0) = 0x3031;
	WFIFOL(char_fd,2) = guild_id;
	WFIFOSET(char_fd,6);
	return 0;
}
// ギルドメンバ追加要求
int intif_guild_addmember(uint32 guild_id,struct guild_member &member)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd,0) = 0x3032;
	WFIFOW(char_fd,2) = sizeof(struct guild_member)+8;
	WFIFOL(char_fd,4) = guild_id;
	//memcpy(WFIFOP(char_fd,8),member,sizeof(struct guild_member));
	guild_member_tobuffer(member, WFIFOP(char_fd,8));

	WFIFOSET(char_fd,WFIFOW(char_fd,2));

	return 0;
}
// ギルドメンバ脱退/追放要求
int intif_guild_leave(uint32 guild_id,uint32 account_id,uint32 char_id,int flag,const char *mes)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0) = 0x3034;
	WFIFOL(char_fd, 2) = guild_id;
	WFIFOL(char_fd, 6) = account_id;
	WFIFOL(char_fd,10) = char_id;
	WFIFOB(char_fd,14) = flag;
	memcpy(WFIFOP(char_fd,15),mes,40);
	WFIFOSET(char_fd,55);
	return 0;
}
// ギルドメンバのオンライン状況/Lv更新要求
int intif_guild_memberinfoshort(uint32 guild_id, uint32 account_id,uint32 char_id,int online,int lv,int class_)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0) = 0x3035;
	WFIFOL(char_fd, 2) = guild_id;
	WFIFOL(char_fd, 6) = account_id;
	WFIFOL(char_fd,10) = char_id;
	WFIFOB(char_fd,14) = online;
	WFIFOW(char_fd,15) = lv;
	WFIFOW(char_fd,17) = class_;
	WFIFOSET(char_fd,19);
	return 0;
}
// ギルド解散通知
int intif_guild_break(uint32 guild_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0) = 0x3036;
	WFIFOL(char_fd, 2) = guild_id;
	WFIFOSET(char_fd,6);
	return 0;
}
// ギルド会話送信
int intif_guild_message(uint32 guild_id,uint32 account_id,const char *mes,size_t len)
{
	if( !session_isActive(char_fd) )
		return 0;
	if((guild_search(guild_id) != NULL) && (mes != NULL) && (len > 0))
	{
		WFIFOW(char_fd,0)=0x3037;
		WFIFOW(char_fd,2)=len+12;
		WFIFOL(char_fd,4)=guild_id;
		WFIFOL(char_fd,8)=account_id;
		memcpy(WFIFOP(char_fd,12),mes,len);
		WFIFOSET(char_fd,len+12);
	}

	return 0;
}
// ギルド競合チェック要求
int intif_guild_checkconflict(uint32 guild_id,uint32 account_id,uint32 char_id)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0)=0x3038;
	WFIFOL(char_fd, 2)=guild_id;
	WFIFOL(char_fd, 6)=account_id;
	WFIFOL(char_fd,10)=char_id;
	WFIFOSET(char_fd,14);
	return 0;
}
// ギルド基本情報変更要求
int intif_guild_change_basicinfo(uint32 guild_id,int type, uint32 data)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0)=0x3039;
	WFIFOW(char_fd, 2)=14;
	WFIFOL(char_fd, 4)=guild_id;
	WFIFOW(char_fd, 8)=type;
	WFIFOL(char_fd,10)=data;
	WFIFOSET(char_fd,14);
	return 0;
}
// ギルドメンバ情報変更要求
int intif_guild_change_memberinfo(uint32 guild_id,uint32 account_id,uint32 char_id, unsigned short type, uint32 data)
{
	if( !session_isActive(char_fd) )
		return 0;

	WFIFOW(char_fd, 0)=0x303a;
	WFIFOW(char_fd, 2)=22;
	WFIFOL(char_fd, 4)=guild_id;
	WFIFOL(char_fd, 8)=account_id;
	WFIFOL(char_fd,12)=char_id;
	WFIFOW(char_fd,16)=type;
	WFIFOL(char_fd,18)=data;
	WFIFOSET(char_fd,22);
	return 0;
}
// ギルド役職変更要求
int intif_guild_position(uint32 guild_id,uint32 idx,struct guild_position &pos)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x303b;
	WFIFOW(char_fd,2)=sizeof(struct guild_position)+12;
	WFIFOL(char_fd,4)=guild_id;
	WFIFOL(char_fd,8)=idx;
	guild_position_tobuffer(pos, WFIFOP(char_fd,12));
	WFIFOSET(char_fd, sizeof(struct guild_position)+12);
	return 0;
}
// ギルドスキルアップ要求
int intif_guild_skillup(uint32 guild_id,unsigned short skillid,uint32 account_id,int flag)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0)=0x303c;
	WFIFOL(char_fd, 2)=guild_id;
	WFIFOL(char_fd, 6)=skillid;
	WFIFOL(char_fd,10)=account_id;
	//WFIFOL(char_fd,14)=flag; // not used
	WFIFOSET(char_fd,14);
	return 0;
}
// ギルド同盟/敵対要求
int intif_guild_alliance(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2,int flag)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd, 0)=0x303d;
	WFIFOL(char_fd, 2)=guild_id1;
	WFIFOL(char_fd, 6)=guild_id2;
	WFIFOL(char_fd,10)=account_id1;
	WFIFOL(char_fd,14)=account_id2;
	WFIFOB(char_fd,18)=flag;
	WFIFOSET(char_fd,19);
	return 0;
}
// ギルド告知変更要求
int intif_guild_notice(uint32 guild_id,const char *mes1,const char *mes2)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x303e;
	WFIFOL(char_fd,2)=guild_id;
	memcpy(WFIFOP(char_fd,6),mes1,60);
	memcpy(WFIFOP(char_fd,66),mes2,120);
	WFIFOSET(char_fd,186);
	return 0;
}
// ギルドエンブレム変更要求
int intif_guild_emblem(uint32 guild_id,const unsigned char *data, size_t len)
{
	if( !session_isActive(char_fd) )
		return 0;
	if(guild_id<=0 || len>2000)
		return 0;
	WFIFOW(char_fd,0)=0x303f;
	WFIFOW(char_fd,2)=len+12;
	WFIFOL(char_fd,4)=guild_id;
	WFIFOL(char_fd,8)=0;
	memcpy(WFIFOP(char_fd,12),data,len);
	WFIFOSET(char_fd,len+12);
	return 0;
}
//現在のギルド城占領ギルドを調べる
int intif_guild_castle_dataload(unsigned short castle_id,int index)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x3040;
	WFIFOW(char_fd,2)=castle_id;
	WFIFOB(char_fd,4)=index;
	WFIFOSET(char_fd,5);
	return 0;
}

//ギルド城占領ギルド変更要求
int intif_guild_castle_datasave(unsigned short castle_id,int index, int value)
{
	if( !session_isActive(char_fd) )
		return 0;
	WFIFOW(char_fd,0)=0x3041;
	WFIFOW(char_fd,2)=castle_id;
	WFIFOB(char_fd,4)=index;
	WFIFOL(char_fd,5)=value;
	WFIFOSET(char_fd,9);
	return 0;
}

//-----------------------------------------------------------------
// Packets receive from inter server

// Wisp/Page reception
int intif_parse_WisMessage(int fd)
{	// rewritten by [Yor]
	struct map_session_data* sd;
	int id=RFIFOL(fd,4);
	int i=0;

	if(config.etc_log)
		ShowMessage("intif_parse_wismessage: %d %s %s %s\n",id,RFIFOP(fd,6),RFIFOP(fd,30),RFIFOP(fd,54) );
	
	sd=map_nick2sd((char*)RFIFOP(fd,32));	// 送信先を探す
	if(sd!=NULL && strcmp(sd->status.name, (char*)RFIFOP(fd,32)) == 0){
		if(sd->state.ignoreAll == 1){
			intif_wis_replay(id,2);	// 受信拒否
		}else{
			for(i=0; i<MAX_IGNORE_LIST; ++i){   //拒否リストに名前があるかどうか判定してあれば拒否
				if(strcmp(sd->ignore[i].name,(char*)RFIFOP(fd,8))==0){
					break;
				}
			}
			if(i==MAX_IGNORE_LIST) // run out of list, so we are not ignored
			{
				clif_wis_message(sd->fd,(char*)RFIFOP(fd,8),(char*)RFIFOP(fd,56),RFIFOW(fd,2)-56);
				intif_wis_replay(id,0);   // 送信成功
			}
			else
				intif_wis_replay(id, 2);   // 受信拒否
		}
	}else
		intif_wis_replay(id,1);	// そんな人いません
	return 0;
}

// Wisp/page transmission result reception
int intif_parse_WisEnd(int fd)
{
	struct map_session_data* sd;

	if(config.etc_log)
		ShowMessage("intif_parse_wisend: player: %s, flag: %d\n", (char*)RFIFOP(fd,2), (unsigned char)RFIFOB(fd,26)); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	sd = map_nick2sd((char*)RFIFOP(fd,2));
	if(sd != NULL)
		clif_wis_end(sd->fd, RFIFOB(fd,26));

	return 0;
}

// Received wisp message from map-server via char-server for ALL gm
int mapif_parse_WisToGM(int fd)
{	// 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B
	size_t i;
	int min_gm_level;
	struct map_session_data *pl_sd;
	char Wisp_name[32];
	char mbuf[256];

	char *message = ( (size_t)RFIFOW(fd,2) >= 30+sizeof(mbuf)) ? new char[(RFIFOW(fd,2) - 30)] : mbuf;

	min_gm_level = (int)RFIFOW(fd,28);
	memcpy(Wisp_name, RFIFOP(fd,4), 24);
	Wisp_name[23] = '\0';
	memcpy(message, RFIFOP(fd,30), RFIFOW(fd,2) - 30);
	message[sizeof(message) - 1] = '\0';
	// information is sended to all online GM
	for (i = 0; i < fd_max; ++i)
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
			if(pl_sd->isGM() >= min_gm_level)
				clif_wis_message(i, Wisp_name, message, strlen(message) + 1);

        if(message != mbuf)
            delete[] message;

	return 0;
}

// アカウント変数通知
int intif_parse_AccountReg(int fd) {
	int j,p;
	struct map_session_data *sd;

	if( (sd=map_id2sd(RFIFOL(fd,4)))==NULL )
		return 1;
	for(p=8,j=0; p<RFIFOW(fd,2) && j<ACCOUNT_REG_NUM; p+=36,++j){
		memcpy(sd->status.account_reg[j].str,RFIFOP(fd,p),32);
		sd->status.account_reg[j].value=RFIFOL(fd,p+32);
	}
	sd->status.account_reg_num = j;
//	ShowMessage("intif: accountreg\n");

	return 0;
}

// 倉庫データ受信
int intif_parse_LoadStorage(int fd) {
	struct pc_storage *stor;
	struct map_session_data *sd;

	stor = account2storage( RFIFOL(fd,4));

	if(stor)
	{
		if(stor->storage_status == 1) { // Already open.. lets ignore this update
			if(config.error_log)
				ShowMessage("intif_parse_LoadStorage: storage received for a client already open\n");
			return 0;
		}

		if(RFIFOW(fd,2)-8 != sizeof(struct pc_storage)) {
			if(config.error_log)
				ShowMessage("intif_parse_LoadStorage: data size error %d %d\n", RFIFOW(fd,2)-8, sizeof(struct pc_storage));
			return 1;
		}
		sd=map_id2sd( RFIFOL(fd,4) );
		if(sd==NULL){
			if(config.error_log)
				ShowError("intif_parse_LoadStorage: user not found %ld\n",(unsigned long)RFIFOL(fd,4));
			return 1;
		}
		if(config.save_log)
			ShowMessage("intif_openstorage: %ld\n",(unsigned long)RFIFOL(fd,4) );

	//	memcpy(stor,RFIFOP(fd,8),sizeof(struct pc_storage));
		pc_storage_frombuffer(*stor, RFIFOP(fd,8));
		stor->dirty=0;
		stor->storage_status=1;
		sd->state.storage_flag = 0;
		clif_storageitemlist(*sd,*stor);
		clif_storageequiplist(*sd,*stor);
		clif_updatestorageamount(*sd,*stor);
	}

	return 0;
}

// 倉庫データ送信成功
int intif_parse_SaveStorage(int fd)
{
	if(config.save_log)
		ShowMessage("intif_savestorage: done %ld %d\n",(unsigned long)RFIFOL(fd,2),(unsigned char)RFIFOB(fd,6) );
	return 0;
}

int intif_parse_LoadGuildStorage(int fd)
{
	struct guild_storage *gstor;
	struct map_session_data *sd;

	int guild_id = RFIFOL(fd,8);
	if(guild_id > 0) {
		gstor=guild2storage(guild_id);
		if(!gstor) {
			if(config.error_log)
				ShowMessage("intif_parse_LoadGuildStorage: error guild_id %d not exist\n",guild_id);
			return 1;
		}
		if( RFIFOW(fd,2)-12 != sizeof(struct guild_storage) ){
			gstor->storage_status = 0;
			if(config.error_log)
				ShowMessage("intif_parse_LoadGuildStorage: data size error %d %d\n",RFIFOW(fd,2)-12 , sizeof(struct guild_storage));
			return 1;
		}
		sd=map_id2sd( RFIFOL(fd,4) );
		if(sd==NULL){
			if(config.error_log)
				ShowError("intif_parse_LoadGuildStorage: user not found %ld\n",(unsigned long)RFIFOL(fd,4));
			return 1;
		}
		if(config.save_log)
			ShowMessage("intif_open_guild_storage: %ld\n",(unsigned long)RFIFOL(fd,4) );

		guild_storage_frombuffer(*gstor, RFIFOP(fd,12));

		gstor->storage_status = 1;
		sd->state.storage_flag = 1;
		clif_guildstorageitemlist(*sd,*gstor);
		clif_guildstorageequiplist(*sd,*gstor);
		clif_updateguildstorageamount(*sd,*gstor);
	}
	return 0;
}
int intif_parse_SaveGuildStorage(int fd)
{
	if(config.save_log) {
		ShowMessage("intif_save_guild_storage: done %ld %ld %d\n",(unsigned long)RFIFOL(fd,2),(unsigned long)RFIFOL(fd,6),(unsigned char)RFIFOB(fd,10) );
	}
	return 0;
}

// パーティ作成可否
int intif_parse_PartyCreated(int fd)
{
	if(config.etc_log)
		ShowInfo("intif: party created by account %d\n\n", (uint32)RFIFOL(fd,2));
	party_created(RFIFOL(fd,2),RFIFOB(fd,6),RFIFOL(fd,7),(char*)RFIFOP(fd,11));
	return 0;
}
// パーティ情報
int intif_parse_PartyInfo(int fd)
{
	if( RFIFOW(fd,2)==sizeof(struct party)+4 )
	{
		struct party par;
		party_frombuffer(par, RFIFOP(fd,4));
		party_recv_info(par);
	}
	else if( RFIFOW(fd,2)==8){
		if(config.error_log)
			ShowMessage("intif: party noinfo %ld\n",(unsigned long)RFIFOL(fd,4));
		party_recv_noinfo(RFIFOL(fd,4));
		return 0;
	}
	else
	{
		if(config.error_log)
			ShowMessage("intif: party info : data size error %ld %d %d\n",(unsigned long)RFIFOL(fd,4),(unsigned short)RFIFOW(fd,2),sizeof(struct party)+4);
	}
	return 0;
}
// パーティ追加通知
int intif_parse_PartyMemberAdded(int fd)
{
	if(config.etc_log)
		ShowMessage("intif: party member added %ld %ld %d\n",(unsigned long)RFIFOL(fd,2),(unsigned long)RFIFOL(fd,6),(unsigned char)RFIFOB(fd,10));
	party_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOB(fd,10));
	return 0;
}
// パーティ設定変更通知
int intif_parse_PartyOptionChanged(int fd)
{
	party_optionchanged(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOB(fd,14));
	return 0;
}
// パーティ脱退通知
int intif_parse_PartyMemberLeaved(int fd)
{
	if(config.etc_log)
		ShowMessage("intif: party member leaved %ld %ld %s\n",(unsigned long)RFIFOL(fd,2),(unsigned long)RFIFOL(fd,6),RFIFOP(fd,10));
	party_member_leaved(RFIFOL(fd,2),RFIFOL(fd,6),(char*)RFIFOP(fd,10));
	return 0;
}
// パーティ解散通知
int intif_parse_PartyBroken(int fd)
{
	party_broken(RFIFOL(fd,2));
	return 0;
}
// パーティ移動通知
int intif_parse_PartyMove(int fd)
{
//	if(config.etc_log)
//		ShowMessage("intif: party move %ld %ld %s %d %d\n",(unsigned long)RFIFOL(fd,2),(unsigned long)RFIFOL(fd,6),RFIFOP(fd,10),(unsigned char)RFIFOB(fd,26),(unsigned short)RFIFOW(fd,27));
	party_recv_movemap(RFIFOL(fd,2),RFIFOL(fd,6),(char*)RFIFOP(fd,10),(unsigned char)RFIFOB(fd,26),(unsigned short)RFIFOW(fd,27));
	return 0;
}
// パーティメッセージ
int intif_parse_PartyMessage(int fd)
{
//	if(config.etc_log)
//		ShowMessage("intif_parse_PartyMessage: %s\n",RFIFOP(fd,12));
	party_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char*)RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 0;
}

// ギルド作成可否
int intif_parse_GuildCreated(int fd)
{
	guild_created(RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}
// ギルド情報
int intif_parse_GuildInfo(int fd)
{
	if( RFIFOW(fd,2)==4+sizeof(struct guild) )
	{
		struct guild g;
		guild_frombuffer(g, RFIFOP(fd,4));
		guild_recv_info(g);
	}
	else if( RFIFOW(fd,2)==8)
	{
		if(config.error_log)
			ShowMessage("intif: guild noinfo %ld\n",(unsigned long)RFIFOL(fd,4));
		guild_recv_noinfo(RFIFOL(fd,4));
		return 0;
	}
	else
	{
		if(config.error_log)
			ShowMessage("intif: guild info : data size error %ld %d %d\n", (unsigned long)RFIFOL(fd,4),(unsigned short)RFIFOW(fd,2),4+sizeof(struct guild));
	}
	return 0;
}
// ギルドメンバ追加通知
int intif_parse_GuildMemberAdded(int fd)
{
	if(config.etc_log)
		ShowMessage("intif: guild member added %ld %ld %ld %d\n",(unsigned long)RFIFOL(fd,2),(unsigned long)RFIFOL(fd,6),(unsigned long)RFIFOL(fd,10),(unsigned char)RFIFOB(fd,14));
	guild_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	return 0;
}
// ギルドメンバ脱退/追放通知
int intif_parse_GuildMemberLeaved(int fd)
{
	guild_member_leaved(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),
		(char*)RFIFOP(fd,55),(char*)RFIFOP(fd,15));
	return 0;
}

// ギルドメンバオンライン状態/Lv変更通知
int intif_parse_GuildMemberInfoShort(int fd)
{
	guild_recv_memberinfoshort(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17));
	return 0;
}
// ギルド解散通知
int intif_parse_GuildBroken(int fd)
{
	guild_broken(RFIFOL(fd,2),RFIFOB(fd,6));
	return 0;
}

// ギルド基本情報変更通知
int intif_parse_GuildBasicInfoChanged(int fd)
{
	int type=RFIFOW(fd,8);
	int guild_id=RFIFOL(fd,4);
//	void *data=RFIFOP(fd,10);
	struct guild *g=guild_search(guild_id);
	short dw=RFIFOW(fd,10);//!*!((short !*!)data);
	int dd=RFIFOL(fd,10);//!*!((int !*!)data);
	if( g==NULL )
		return 0;
	switch(type){
	case GBI_EXP:			g->exp=dd; break;
	case GBI_GUILDLV:		g->guild_lv=dw; break;
	case GBI_SKILLPOINT:	g->skill_point=dd; break;
	}
	return 0;
}
// ギルドメンバ情報変更通知
int intif_parse_GuildMemberInfoChanged(int fd)
{
	unsigned short type=RFIFOW(fd,16);
	uint32 guild_id=RFIFOL(fd,4);
	uint32 account_id=RFIFOL(fd,8);
	uint32 char_id=RFIFOL(fd,12);
	uint32 dd = RFIFOL(fd,18);
	struct guild *g=guild_search(guild_id);
	int idx;
	if( g )
	{
		idx=guild_getindex(*g,account_id,char_id);
		switch(type){
		case GMI_POSITION:
			g->member[idx].position=dd;
			guild_memberposition_changed(*g,idx,dd);
			break;
		case GMI_EXP:
			g->member[idx].exp=dd;
			break;
		}
	}
	return 0;
}

// ギルド役職変更通知
int intif_parse_GuildPosition(int fd)
{
	if( RFIFOW(fd,2)==sizeof(struct guild_position)+12 )
	{
		struct guild_position pos;
		guild_position_frombuffer(pos, RFIFOP(fd,12));
		guild_position_changed(RFIFOL(fd,4), RFIFOL(fd,8), pos);
	}
	else
	{
		if(config.error_log)
			ShowMessage("intif: guild position : data size error\n %ld %d %d",(unsigned long)RFIFOL(fd,4),(unsigned short)RFIFOW(fd,2),sizeof(struct guild_position)+12);
	}
	return 0;
}
// ギルドスキル割り振り通知
int intif_parse_GuildSkillUp(int fd)
{
	guild_skillupack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 0;
}
// ギルド同盟/敵対通知
int intif_parse_GuildAlliance(int fd)
{
	guild_allianceack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),
		RFIFOB(fd,18),(char*)RFIFOP(fd,19),(char*)RFIFOP(fd,43));
	return 0;
}
// ギルド告知変更通知
int intif_parse_GuildNotice(int fd)
{
	guild_notice_changed(RFIFOL(fd,2),(char*)RFIFOP(fd,6),(char*)RFIFOP(fd,66));
	return 0;
}
// ギルドエンブレム変更通知
int intif_parse_GuildEmblem(int fd)
{
	guild_emblem_changed(RFIFOW(fd,2)-12,RFIFOL(fd,4),RFIFOL(fd,8),RFIFOP(fd,12));
	return 0;
}
// ギルド会話受信
int intif_parse_GuildMessage(int fd)
{
	guild_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char*)RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 0;
}
// ギルド城データ要求返信
int intif_parse_GuildCastleDataLoad(int fd)
{
	return guild_castledataloadack(RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5));
}
// ギルド城データ変更通知
int intif_parse_GuildCastleDataSave(int fd)
{
	return guild_castledatasaveack(RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5));
}

// ギルド城データ一括受信(初期化時)
int intif_parse_GuildCastleAllDataLoad(int fd)
{
	return guild_castlealldataload(RFIFOW(fd,2),RFIFOP(fd,4)); // !!BAD!!
}

// pet
int intif_parse_CreatePet(int fd)
{
	pet_get_egg(RFIFOL(fd,2),RFIFOL(fd,7),RFIFOB(fd,6));
	return 0;
}

int intif_parse_RecvPetData(int fd)
{
	int len=RFIFOW(fd,2);
	if(sizeof(struct petstatus)!=len-9)
	{
		if(config.etc_log)
			ShowMessage("intif: pet data: data size error %d %d\n",sizeof(struct petstatus),len-9);
	}
	else
	{
		struct petstatus pet;
		s_pet_frombuffer(pet, RFIFOP(fd,9));
		pet_recv_petdata(RFIFOL(fd,4), pet, RFIFOB(fd,8));
	}
	return 0;
}
int intif_parse_SavePetOk(int fd)
{
	if(RFIFOB(fd,6) == 1) {
		if(config.error_log)
			ShowMessage("pet data save failure\n");
	}

	return 0;
}

int intif_parse_DeletePetOk(int fd)
{
	if(RFIFOB(fd,2) == 1) {
		if(config.error_log)
			ShowMessage("pet data delete failure\n");
	}

	return 0;
}
//-----------------------------------------------------------------
// inter serverからの通信
// エラーがあれば0(false)を返すこと
// パケットが処理できれば1,パケット長が足りなければ2を返すこと
int intif_parse(int fd)
{
	int packet_len;
	unsigned short cmd = RFIFOW(fd,0);
	// パケットのID確認
	if(cmd<0x3800 || cmd>=0x3800+(sizeof(packet_len_table)/sizeof(packet_len_table[0])) ||
	   packet_len_table[cmd-0x3800]==0){
	   	return 0;
	}
	// パケットの長さ確認
	packet_len = packet_len_table[cmd-0x3800];
	if(packet_len < 0){
		if(RFIFOREST(fd)<4)
			return 2;
		packet_len = RFIFOW(fd,2);
	}
//	if(config.etc_log)
//		ShowMessage("intif_parse %d %x %d %d\n",fd,cmd,packet_len,RFIFOREST(fd));
	if(RFIFOREST(fd)<packet_len){
		return 2;
	}
	// 処理分岐
	switch(cmd){
	case 0x3800:	clif_GMmessage(NULL,(char*)RFIFOP(fd,4),packet_len-4, 0); break;
	case 0x3801:	intif_parse_WisMessage(fd); break;
	case 0x3802:	intif_parse_WisEnd(fd); break;
	case 0x3803:	mapif_parse_WisToGM(fd); break;
	case 0x3804:	intif_parse_AccountReg(fd); break;
	case 0x3810:	intif_parse_LoadStorage(fd); break;
	case 0x3811:	intif_parse_SaveStorage(fd); break;
	case 0x3818:	intif_parse_LoadGuildStorage(fd); break;
	case 0x3819:	intif_parse_SaveGuildStorage(fd); break;
	case 0x3820:	intif_parse_PartyCreated(fd); break;
	case 0x3821:	intif_parse_PartyInfo(fd); break;
	case 0x3822:	intif_parse_PartyMemberAdded(fd); break;
	case 0x3823:	intif_parse_PartyOptionChanged(fd); break;
	case 0x3824:	intif_parse_PartyMemberLeaved(fd); break;
	case 0x3825:	intif_parse_PartyMove(fd); break;
	case 0x3826:	intif_parse_PartyBroken(fd); break;
	case 0x3827:	intif_parse_PartyMessage(fd); break;
	case 0x3830:	intif_parse_GuildCreated(fd); break;
	case 0x3831:	intif_parse_GuildInfo(fd); break;
	case 0x3832:	intif_parse_GuildMemberAdded(fd); break;
	case 0x3834:	intif_parse_GuildMemberLeaved(fd); break;
	case 0x3835:	intif_parse_GuildMemberInfoShort(fd); break;
	case 0x3836:	intif_parse_GuildBroken(fd); break;
	case 0x3837:	intif_parse_GuildMessage(fd); break;
	case 0x3839:	intif_parse_GuildBasicInfoChanged(fd); break;
	case 0x383a:	intif_parse_GuildMemberInfoChanged(fd); break;
	case 0x383b:	intif_parse_GuildPosition(fd); break;
	case 0x383c:	intif_parse_GuildSkillUp(fd); break;
	case 0x383d:	intif_parse_GuildAlliance(fd); break;
	case 0x383e:	intif_parse_GuildNotice(fd); break;
	case 0x383f:	intif_parse_GuildEmblem(fd); break;
	case 0x3840:	intif_parse_GuildCastleDataLoad(fd); break;
	case 0x3841:	intif_parse_GuildCastleDataSave(fd); break;
	case 0x3842:	intif_parse_GuildCastleAllDataLoad(fd); break;
	case 0x3880:	intif_parse_CreatePet(fd); break;
	case 0x3881:	intif_parse_RecvPetData(fd); break;
	case 0x3882:	intif_parse_SavePetOk(fd); break;
	case 0x3883:	intif_parse_DeletePetOk(fd); break;

	case 0x3889:	intif_parse_RecvHomun(fd); break;
	case 0x388a:	intif_parse_SaveHomun(fd); break;
	case 0x388b:	intif_parse_DeleteHomun(fd); break;

	default:
		if(config.error_log)
			ShowMessage("intif_parse : unknown packet %d %x\n",fd,(unsigned short)RFIFOW(fd,0));
		return 0;
	}
	// パケット読み飛ばし
	RFIFOSKIP(fd,packet_len);
	return 1;
}
