// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "timer.h"
#include "db.h"
#include "showmsg.h"
#include "utils.h"
#include "malloc.h"
#include "lock.h"

#include "char.h"
#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_storage.h"
#include "int_pet.h"

#define WISDATA_TTL (60*1000)	// Existence time of Wisp/page data (60 seconds)
                             	// that is the waiting time of answers of all map-servers

basics::CParam< basics::string<> > inter_log_filename("inter_log_filename", "log/inter.log");
basics::CParam< basics::string<> > accreg_txt("accreg_txt", "save/accreg.txt");

static struct dbt *accreg_db = NULL;

struct accreg
{
	uint32 account_id;
	size_t reg_num;
	struct global_reg reg[ACCOUNT_REG_NUM];

	accreg() : 
		account_id(0),
		reg_num(0)
	{}
	explicit accreg(uint32 id) : 
		account_id(id),
		reg_num(0)
	{}
};



// 送信パケット長リスト
int inter_send_packet_length[] = {
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0,
	35,-1,11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,
	 9, 9,-1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};
// 受信パケット長リスト
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1, 6, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3000
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,	// 0x3010
	74, 6,52,14, 10,29, 6,-1, 34, 0, 0, 0,  0, 0,  0, 0,	// 0x3020
	-1, 6,-1, 0, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,	// 0x3030
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3040
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3050
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3060
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 0x3070
	48,14,-1, 6,  0, 0, 0, 0, -1,14,-1,14,  0, 0,  0, 0,	// 0x3080
};

struct WisData
{
	uint32 id;
	int fd;
	int count;
	uint32 len;
	unsigned long tick;
	char src[24];
	char dst[24];
	char msg[512];
};
static struct dbt * wis_db = NULL;


//--------------------------------------------------------

// アカウント変数を文字列へ変換
int inter_accreg_tostr(char *str, struct accreg *reg) {
	size_t j;
	char *p = str;

	p += sprintf(p, "%lu\t", (unsigned long)reg->account_id);
	for(j = 0; j < reg->reg_num; ++j) {
		p += sprintf(p,"%s,%ld ", reg->reg[j].str, (long)reg->reg[j].value);
	}

	return 0;
}

// アカウント変数を文字列から変換
int inter_accreg_fromstr(const char *str, struct accreg *reg) {
	int j, v, n;
	char buf[128];
	const char *p = str;
	int accid;

	if (sscanf(p, "%d\t%n", &accid, &n ) != 1 || accid <= 0)
		return 1;

	reg->account_id = accid;
	for(j = 0, p += n; j < ACCOUNT_REG_NUM; j++, p += n) {
		if (sscanf(p, "%128[^,],%d %n", buf, &v, &n) != 2)
			break;
		memcpy(reg->reg[j].str, buf, 32);
		reg->reg[j].value = v;
	}
	reg->reg_num = j;

	return 0;
}

// アカウント変数の読み込み
int inter_accreg_init(void)
{
	char line[8192];
	FILE *fp;
	int c = 0;
	struct accreg *reg;

	accreg_db = numdb_init();

	if( (fp = basics::safefopen(accreg_txt(), "r")) == NULL)
		return 1;
	while(fgets(line, sizeof(line), fp)){
		line[sizeof(line)-1] = '\0';

		reg = new struct accreg;
		if (inter_accreg_fromstr(line, reg) == 0 && reg->account_id > 0)
		{
			numdb_insert(accreg_db, reg->account_id, reg);
		}
		else
		{
			ShowMessage("inter: accreg: broken data [%s] line %d\n", (const char*)accreg_txt(), c);
			delete reg;
		}
		c++;
	}
	fclose(fp);
	//ShowStatus("inter: %s read done (%d)\n", (const char*)accreg_txt(), c);
	return 0;
}

// アカウント変数のセーブ
int inter_accreg_save(void)
{
	int lock;
	FILE *fp = lock_fopen(accreg_txt(), lock);
	if(!fp)
	{
		ShowError("int_accreg: cant write [%s] !!! data is lost !!!\n", (const char*)accreg_txt());
		return 1;
	}
	else
	{
		db_iterator<size_t,struct accreg *> iter(accreg_db);
		char line[8192];
		accreg *reg;
		for(; iter; ++iter)
		{
			reg = iter.data();
			if(reg && reg->reg_num > 0)
			{
				inter_accreg_tostr(line, reg);
				fprintf(fp, "%s" RETCODE, line);
			}
		}
		lock_fclose(fp, accreg_txt(), lock);
		//ShowStatus("inter: %s saved.\n", (const char*)accreg_txt());
	}
	return 0;
}

//--------------------------------------------------------

// ログ書き出し
int inter_log(char *fmt,...)
{
	FILE *logfp = basics::safefopen(inter_log_filename(), "a");
	if (logfp && fmt)
	{
		va_list ap;
		va_start(ap,fmt);
		vfprintf(logfp, fmt, ap);
		va_end(ap);

		fclose(logfp);
	}
	return 0;
}

// セーブ
int inter_save(void) 
{
	inter_accreg_save();
	return 0;
}

// 初期化
int inter_init(void)
{
	wis_db = numdb_init();

	inter_party_init();
	inter_guild_init();
	inter_storage_init();
	inter_pet_init();
	inter_accreg_init();

	return 0;
}

// finalize
void accreg_db_final (void *k, void *data)
{	
	struct accreg *p = (struct accreg *) data;
	if (p) delete p;
}
void wis_db_final (void *k, void *data)
{
	struct WisData *p = (struct WisData *) data;
	if(p) delete p;
}
void inter_final()
{
	if(accreg_db)
	{
		numdb_final(accreg_db, accreg_db_final);
		accreg_db=NULL;
	}
	if(wis_db)
	{
		numdb_final(wis_db, wis_db_final);
		wis_db=NULL;
	}

	inter_party_final();
	inter_guild_final();
	inter_storage_final();
	inter_pet_final();
	return;
}

// マップサーバー接続
int inter_mapif_init(int fd) {
	inter_guild_mapif_init(fd);

	return 0;
}

//--------------------------------------------------------
// sended packets to map-server

// GMメッセージ送信
int mapif_GMmessage(unsigned char *mes, int len, int sfd)
{
	CREATE_BUFFER(buf,unsigned char,len);

	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	memcpy(WBUFP(buf,4), mes, len - 4);
	mapif_sendallwos(sfd, buf, len);
//	ShowMessage("inter server: GM:%d %s\n", len, mes);

	DELETE_BUFFER(buf);
	return 0;
}

// Wisp/page transmission to all map-server
int mapif_wis_message(struct WisData *wd)
{
	CREATE_BUFFER(buf,unsigned char,56 + wd->len);

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 + wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, 24);
	memcpy(WBUFP(buf,32), wd->dst, 24);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif_sendall(buf, 56 + wd->len);

	DELETE_BUFFER(buf);
	return 0;
}

// Wisp/page transmission result to map-server
int mapif_wis_end(struct WisData *wd, int flag) {
	unsigned char buf[27];

	WBUFW(buf, 0) = 0x3802;
	memcpy(WBUFP(buf, 2), wd->src, 24);
	WBUFB(buf,26) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	mapif_send(wd->fd, buf, 27);
//	ShowMessage("inter server wis_end: flag: %d\n", flag);

	return 0;
}

// アカウント変数送信
int mapif_account_reg(int fd, unsigned char *src)
{
	CREATE_BUFFER(buf,unsigned char, (unsigned short)WBUFW(src,2));

	memcpy(WBUFP(buf,0),src,WBUFW(src,2));
	WBUFW(buf, 0) = 0x3804;
	mapif_sendallwos(fd, buf, WBUFW(buf,2));

	DELETE_BUFFER(buf);
	return 0;
}

// アカウント変数要求返信
int mapif_account_reg_reply(int fd, uint32 account_id) {
	struct accreg *reg = (struct accreg*)numdb_search(accreg_db,account_id);
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x3804;
	WFIFOL(fd,4) = account_id;
	if (reg == NULL) {
		WFIFOW(fd,2) = 8;
	} else {
		size_t j;
		unsigned short p;
		for(j = 0, p = 8; j < reg->reg_num; j++, p += 36) {
			memcpy(WFIFOP(fd,p), reg->reg[j].str, 32);
			WFIFOL(fd,p+32) = reg->reg[j].value;
		}
		WFIFOW(fd,2) = p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// check ttl of wisper messages
///
int check_ttl_wisdata(void)
{
	unsigned long tick = gettick();
	db_iterator<size_t, WisData*> iter(wis_db);
	for(; iter; ++iter)
	{
		WisData *wd = iter.data();
		if( wd && DIFF_TICK(tick, wd->tick) > WISDATA_TTL )
		{
			ShowMessage("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			mapif_wis_end(wd, 1); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target

			delete wd;
			numdb_erase(wis_db, iter.key());
		}
	}
	return 0;
}

//--------------------------------------------------------
// received packets from map-server

// GMメッセージ送信
int mapif_parse_GMmessage(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	return mapif_GMmessage(RFIFOP(fd,4), RFIFOW(fd,2), fd);
}

// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	static int wisid = 0;

	if( !session_isActive(fd) )
		return 0;

	if ((size_t)RFIFOW(fd,2) >= sizeof(wd->msg)+52) {
		ShowMessage("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2) <= 52) { // normaly, impossible, but who knows...
		ShowMessage("inter: Wis message doesn't exist.\n");
		return 0;
	}

	// search if character exists before to ask all map-servers
	if( !char_exist((char*)RFIFOP(fd,28)) )
	{
		unsigned char buf[27];
		WBUFW(buf, 0) = 0x3802;
		memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
		WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		mapif_send(fd, buf, 27);
	
	}
	else
	{	// Character exists. So, ask all map-servers
		// to be sure of the correct name, rewrite it
		//!!memcpy((char*)RFIFOP(fd,28), search_character_name(index), 24);
		// if source is destination, don't ask other servers.
		if (strcmp((char*)RFIFOP(fd,4),(char*)RFIFOP(fd,28)) == 0) {
			unsigned char buf[27];
			WBUFW(buf, 0) = 0x3802;
			memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
			WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			mapif_send(fd, buf, 27);
		} else {


			// Whether the failure of previous wisp/page transmission (timeout)
			check_ttl_wisdata();

			wd = new struct WisData;

			wd->id = ++wisid;
			wd->fd = fd;
			wd->count= 0;
			wd->len= RFIFOW(fd,2)-52;
			safestrcpy(wd->src, sizeof(wd->src), (char*)RFIFOP(fd, 4));
			safestrcpy(wd->dst, sizeof(wd->dst), (char*)RFIFOP(fd,28));
			safestrcpy(wd->msg, sizeof(wd->msg), (char*)RFIFOP(fd,52));
			wd->tick = gettick();
			numdb_insert(wis_db, wd->id, wd);
			mapif_wis_message(wd);
		}
	}

	return 0;
}

// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	int id = RFIFOL(fd,2);
	int flag = RFIFOB(fd,6);
	struct WisData *wd = (struct WisData*)numdb_search(wis_db, id);

	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		numdb_erase(wis_db, id);
		delete wd;
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[65536];
	if( !session_isActive(fd) )
		return 0;

	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// アカウント変数保存要求
int mapif_parse_AccReg(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	int j, p;
	struct accreg *reg = (struct accreg *)numdb_search(accreg_db, (uint32)RFIFOL(fd,4));
	if (reg == NULL)
	{	// create new
		reg = new struct accreg(RFIFOL(fd,4));
		numdb_insert(accreg_db, reg->account_id, reg);
	}
	for(j = 0, p = 8; j < ACCOUNT_REG_NUM && p < RFIFOW(fd,2); j++, p += 36)
	{
		safestrcpy(reg->reg[j].str, sizeof(reg->reg[j].str), (char*)RFIFOP(fd,p));
		reg->reg[j].value = RFIFOL(fd, p + 32);
	}
	reg->reg_num = j;

	mapif_account_reg(fd, RFIFOP(fd,0));	// 他のMAPサーバーに送信

	return 0;
}

// アカウント変数送信要求
int mapif_parse_AccRegRequest(int fd)
{
	if( !session_isActive(fd) )
		return 0;

//	ShowMessage("mapif: accreg request\n");
	return mapif_account_reg_reply(fd, RFIFOL(fd,2));
}

//--------------------------------------------------------

// map server からの通信（１パケットのみ解析すること）
// エラーなら0(false)、処理できたなら1、
// パケット長が足りなければ2をかえさなければならない
int inter_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	unsigned short cmd = RFIFOW(fd,0);
	int len = 0;

	// inter鯖管轄かを調べる
	if (cmd < 0x3000 || cmd >= 0x3000 + (sizeof(inter_recv_packet_length) / sizeof(inter_recv_packet_length[0])))
		return 0;

	// パケット長を調べる #1
	if ((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 0;
	// パケット長を調べる #2
	if(len>0 && RFIFOREST(fd) < len)
		return 2;

	switch(cmd) {
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_AccReg(fd); break;
	case 0x3005: mapif_parse_AccRegRequest(fd); break;
	default:
		if (inter_party_parse_frommap(fd))
			break;
		if (inter_guild_parse_frommap(fd))
			break;
		if (inter_storage_parse_frommap(fd))
			break;
		if (inter_pet_parse_frommap(fd))
			break;
		return 0;
	}
	RFIFOSKIP(fd, len);

	return 1;
}

// RFIFOのパケット長確認
// 必要パケット長があればパケット長、まだ足りなければ0
int inter_check_length(int fd, int length)
{
	if( !session_isActive(fd) )
		return 0;

	if (length == -1) {	// 可変パケット長
		if (RFIFOREST(fd) < 4)	// パケット長が未着
			return 0;
		length = RFIFOW(fd,2);
	}

	if (RFIFOREST(fd) < length)	// パケットが未着
		return 0;

	return length;
}
