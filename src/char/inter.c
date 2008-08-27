// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "chardb.h"
#include "inter.h"
#include "int_auction.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_mail.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_quest.h"
#include "int_registry.h"
#include "int_status.h"
#include "int_storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// temporary stuff
extern int save_accreg2(unsigned char* buf, int len);
extern int request_accreg2(int account_id, int char_id);

#define WISDATA_TTL (60*1000)	// Expiration time of non-acknowledged whisper data (60 seconds)
#define WISDELLIST_MAX 256   	// Number of elements of Wisp/page data deletion list

#ifdef TXT_ONLY
extern char accreg_txt[1024];
char inter_log_filename[1024] = "log/inter.log";
#else
Sql* sql_handle = NULL;
int char_server_port = 3306;
char char_server_ip[32] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[32] = "ragnarok";
char char_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.
#endif

unsigned int party_share_level = 10;
char main_chat_nick[16] = "Main";

// recv. packet list
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1,13,36, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3000-
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,	// 3010-
	-1, 6,-1,14, 14,19, 6,-1, 14,14, 0, 0,  0, 0,  0, 0,	// 3020-
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,	// 3030-
	 5, 9, 0, 0,  0, 0, 0, 0,  7, 6,10,10, 10,-1,  0, 0,	// 3040-
	-1,-1,10,10,  0,-1, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3050-  Auction System [Zephyrus]
	 6,-1,10, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3060-  Quest system [Kevin]
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3070-
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3080-
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3090-  Homunculus packets [albator]
};

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[512];
};
static DBMap* wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;


/*==========================================
 * read config file
 *------------------------------------------*/
static int inter_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	fp = fopen(cfgName, "r");
	if( fp == NULL )
	{
		ShowError("file not found: %s\n", cfgName);
		return 1;
	}

	ShowInfo("reading file %s...\n", cfgName);

	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

#ifdef TXT_ONLY
		if (strcmpi(w1, "storage_txt") == 0)
			strncpy(storage_txt, w2, sizeof(storage_txt));
		else
		if (strcmpi(w1, "party_txt") == 0)
			strncpy(party_txt, w2, sizeof(party_txt));
		else
		if (strcmpi(w1, "pet_txt") == 0)
			strncpy(pet_txt, w2, sizeof(pet_txt));
		else
		if (strcmpi(w1, "accreg_txt") == 0)
			strncpy(accreg_txt, w2, sizeof(accreg_txt));
		else
		if (strcmpi(w1, "guild_txt") == 0)
			strncpy(guild_txt, w2, sizeof(guild_txt));
		else
		if (strcmpi(w1, "castle_txt") == 0)
			strncpy(castle_txt, w2, sizeof(castle_txt));
		else
		if (strcmpi(w1, "guild_storage_txt") == 0)
			strncpy(guild_storage_txt, w2, sizeof(guild_storage_txt));
		else
		if (strcmpi(w1, "homun_txt") == 0)
			strncpy(homun_txt, w2, sizeof(homun_txt));
		if (strcmpi(w1, "inter_log_filename") == 0)
			strncpy(inter_log_filename, w2, sizeof(inter_log_filename));
#else
		if(!strcmpi(w1,"char_server_ip"))
			strcpy(char_server_ip,w2);
		else
		if(!strcmpi(w1,"char_server_port"))
			char_server_port = atoi(w2);
		else
		if(!strcmpi(w1,"char_server_id"))
			strcpy(char_server_id,w2);
		else
		if(!strcmpi(w1,"char_server_pw"))
			strcpy(char_server_pw,w2);
		else
		if(!strcmpi(w1,"char_server_db"))
			strcpy(char_server_db,w2);
		else
		if(!strcmpi(w1,"default_codepage"))
			strcpy(default_codepage,w2);
#endif
		else
		if(strcmpi(w1,"log_inter")==0)
			log_inter = atoi(w2);
		else
		if(strcmpi(w1, "main_chat_nick")==0)
			strcpy(main_chat_nick, w2);
		else
		if (strcmpi(w1, "party_share_level") == 0)
			party_share_level = atoi(w2);
		else
		if (strcmpi(w1, "import") == 0)
			inter_config_read(w2);
	}
	fclose(fp);

	ShowInfo ("done reading %s.\n", cfgName);

	return 0;
}

// interserver logging
int inter_log(char *fmt,...)
{
#ifdef TXT_ONLY
	FILE *logfp;
	va_list ap;

	va_start(ap,fmt);
	logfp = fopen(inter_log_filename, "a");
	if (logfp) {
		vfprintf(logfp, fmt, ap);
		fclose(logfp);
	}
	va_end(ap);
#else
	char str[255];
	char esc_str[sizeof(str)*2+1];// escaped str
	va_list ap;

	va_start(ap,fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	Sql_EscapeStringLen(sql_handle, esc_str, str, strnlen(str, sizeof(str)));
	if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')", interlog_db, esc_str) )
		Sql_ShowDebug(sql_handle);
#endif
	return 0;
}

#ifdef TXT_ONLY
// セーブ
int inter_save(void)
{
#ifdef ENABLE_SC_SAVING
	inter_status_save();
#endif
	inter_party_save();
	inter_guild_save();
	inter_storage_save();
	inter_guild_storage_save();
	inter_pet_save();
	inter_homun_save();
	inter_accreg_sync();

	return 0;
}
#endif

// initialize
int inter_init(void)
{
	//FIXME: more than one 'inter_config_read' exists
	inter_config_read(INTER_CONF_NAME);

#ifndef TXT_ONLY
	ShowInfo ("interserver initialize...\n");

	//DB connection initialized
	sql_handle = Sql_Malloc();
	ShowInfo("Connect Character DB server.... (Character Server)\n");
	if( SQL_ERROR == Sql_Connect(sql_handle, char_server_id, char_server_pw, char_server_ip, (uint16)char_server_port, char_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( *default_codepage ) {
		if( SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
			Sql_ShowDebug(sql_handle);
	}
#endif
	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);

	inter_accreg_init();
	inter_charreg_init();
	inter_party_init();
	inter_guild_init();
	inter_storage_init();
	inter_pet_init();
	inter_homun_init();
	inter_accreg_init();
#ifndef TXT_ONLY
	inter_mail_init();
	inter_auction_init();
#endif

	return 0;
}

// finalize
void inter_final(void)
{
	inter_accreg_final();
	inter_charreg_final();
	wis_db->destroy(wis_db, NULL);
	inter_party_final();
	inter_guild_final();
	inter_storage_final();
	inter_pet_final();
	inter_homun_final();
#ifndef TXT_ONLY
	inter_mail_final();
	inter_auction_final();
#endif
	return;
}

// map server connection
int inter_mapif_init(int fd)
{
	inter_guild_mapif_init(fd);
	return 0;
}


//--------------------------------------------------------
// packets to map-server

// GM message sending
int mapif_GMmessage(unsigned char *mes, int len, unsigned long color, int sfd)
{
	unsigned char buf[2048];

	if (len > 2048) len = 2047; //Make it fit to avoid crashes. [Skotlex]
	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	WBUFL(buf,4) = color;
	memcpy(WBUFP(buf,8), mes, len - 8);
	mapif_sendallwos(sfd, buf, len);
	return 0;
}

// Whisper sending
static void mapif_wis_message(struct WisData *wd, int fd)
{
	WFIFOHEAD(fd, 56+wd->len);
	WFIFOW(fd, 0) = 0x3801;
	WFIFOW(fd, 2) = 56 + wd->len;
	WFIFOL(fd, 4) = wd->id;
	memcpy(WFIFOP(fd, 8), wd->src, NAME_LENGTH);
	memcpy(WFIFOP(fd,32), wd->dst, NAME_LENGTH);
	memcpy(WFIFOP(fd,56), wd->msg, wd->len);
	wd->count = 1;
	WFIFOSET(fd,WFIFOW(fd,2));
}

// Whisper sending result
// flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
static void mapif_wis_end(int fd, const char* src, int flag)
{
	WFIFOHEAD(fd,27);
	WFIFOW(fd,0) = 0x3802;
	safestrncpy((char*)WFIFOP(fd,2), src, NAME_LENGTH);
	WFIFOB(fd,26) = flag;
	WFIFOSET(fd,27);
}

// Account registry transfer to map-server
static void mapif_account_reg(int fd, unsigned char *src)
{
	WBUFW(src,0) = 0x3804; //NOTE: writing to RFIFO
	mapif_sendallwos(fd, src, WBUFW(src,2));
}

// Send the requested regs
static void mapif_regs_reply(int fd, int account_id, int char_id, int type, const struct regs* reg)
{
	WFIFOHEAD(fd, 13 + ACCOUNT_REG_NUM * 288);
	WFIFOW(fd,0) = 0x3804;
	WFIFOL(fd,4) = account_id;
	WFIFOL(fd,8) = char_id;
	WFIFOB(fd,12) = type;
	WFIFOW(fd,2) = 13 + inter_regs_tobuf(WFIFOP(fd,13), ACCOUNT_REG_NUM * 288, reg);
	WFIFOSET(fd,WFIFOW(fd,2));
}

//Request to kick char from a certain map server. [Skotlex]
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason)
{
	if (fd < 0)
		return -1;

	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x2b1f;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = reason;
	WFIFOSET(fd,7);

	return 0;
}

//--------------------------------------------------------

// Existence check of WISP data
int check_ttl_wisdata_sub(DBKey key, void *data, va_list ap)
{
	unsigned long tick;
	struct WisData *wd = (struct WisData *)data;
	tick = va_arg(ap, unsigned long);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata(void)
{
	unsigned long tick = gettick();
	int i;

	do {
		wis_delnum = 0;
		wis_db->foreach(wis_db, check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)idb_get(wis_db, wis_dellist[i]);
			ShowWarning("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			idb_remove(wis_db, wd->id);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------
// received packets from map-server

// GM message sending
int mapif_parse_GMmessage(int fd)
{
	mapif_GMmessage(RFIFOP(fd,8), RFIFOW(fd,2), RFIFOL(fd,4), fd);
	return 0;
}

static struct WisData* mapif_create_whisper(int fd, char* src, char* dst, char* mes, int meslen)
{
	static int wisid = 0;
	struct WisData* wd = (struct WisData *)aCalloc(sizeof(struct WisData), 1);
	if (wd == NULL){
		ShowFatalError("inter: WisRequest: out of memory !\n");
		return NULL;
	}
	wd->id = ++wisid;
	wd->fd = fd;
	wd->len= meslen;
	memcpy(wd->src, src, NAME_LENGTH);
	memcpy(wd->dst, dst, NAME_LENGTH);
	memcpy(wd->msg, mes, meslen);
	wd->tick = gettick();
	return wd;
}

// Wisp/page request to send
// 3001 <length>.w <src name>.24b <dst name>.24b <message>.?b
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	char name[NAME_LENGTH];
	int fd2;
	int aid, cid;
#ifdef TXT_ONLY
	struct mmo_charstatus* char_status;
#else
	char esc_name[NAME_LENGTH*2+1];// escaped name
	char* data;
	size_t len;
#endif

	if( RFIFOW(fd,2)-52 >= sizeof(wd->msg) )
	{
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	}
	if( RFIFOW(fd,2)-52 <= 0 ) { // normally impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}

	safestrncpy(name, (char*)RFIFOP(fd,28), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]

	// search if character exists before to ask all map-servers
#ifdef TXT_ONLY
	char_status = search_character_byname(name);
	if( char_status == NULL )
	{
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		return 0;
	}
#else
	Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`, `char_id`, `name` FROM `%s` WHERE `name`='%s'", char_db, esc_name) )
		Sql_ShowDebug(sql_handle);

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		Sql_FreeResult(sql_handle);
		return 0;
	}
#endif

	// Character exists. So, ask all map-servers
	// to be sure of the correct name, rewrite it
#ifdef TXT_ONLY
	aid = char_status->account_id;
	cid = char_status->char_id;
	safestrncpy(name, char_status->name, NAME_LENGTH);
#else
	Sql_GetData(sql_handle, 0, &data, &len); aid = atoi(data);
	Sql_GetData(sql_handle, 1, &data, &len); cid = atoi(data);
	Sql_GetData(sql_handle, 2, &data, &len); safestrncpy(name, data, NAME_LENGTH);
	Sql_FreeResult(sql_handle);
#endif

	// if talking to self, don't ask other servers.
	if( strncmp((const char*)RFIFOP(fd,4), name, NAME_LENGTH) == 0 )
	{
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		return 0;
	}

	//Look for online character.
	fd2 = search_character_online(aid, cid);
	if( fd2 < 0 ) 
	{//Character not online.
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		return 0;
	}

	// Whether the failure of previous wisp/page transmission (timeout)
	//FIXME: replace with a periodic timer
	check_ttl_wisdata();

	//Character online, send whisper.
	wd = mapif_create_whisper(fd, (char*)RFIFOP(fd, 4), (char*)RFIFOP(fd,28), (char*)RFIFOP(fd,52), RFIFOW(fd,2)-52);
	if (!wd) return 1;
	idb_put(wis_db, wd->id, wd);
	mapif_wis_message(wd, fd2);
	return 0;
}

// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	int id, flag;
	struct WisData *wd;

	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = (struct WisData*)idb_get(wis_db, id);

	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout or because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd->fd, wd->msg, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B

	ShowDebug("Sent packet back!\n");
	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// save incoming registry
// 3004 <length>.w <aid>.l <cid>.l <type>.b { <str>.s <val>.s }*
int mapif_parse_Registry(int fd)
{
	int length = RFIFOW(fd,2);
	int account_id = RFIFOL(fd,4);
	int char_id = RFIFOL(fd,8);
	int type = RFIFOB(fd,12);
	uint8* buf = RFIFOP(fd,13);

	switch( type )
	{
	case 3: //Character registry
	{
		struct regs reg;
		inter_regs_frombuf(buf, length-13, &reg);
		inter_charreg_save(account_id, &reg);
		mapif_account_reg(fd,RFIFOP(fd,0));	// Send updated accounts to other map servers.
		return 0;
	}
	break;

	case 2: //Account Registry
	{
		struct regs reg;
		inter_regs_frombuf(buf, length-13, &reg);
		inter_accreg_save(char_id, &reg);
		mapif_account_reg(fd,RFIFOP(fd,0));	// Send updated accounts to other map servers.
		return 0;
	}
	break;

	case 1: //Account2 registry, must be sent over to login server.
		return save_accreg2(RFIFOP(fd,4), length-4);
	default: //Error?
		return 1;
	}
}

// Request the value of all registries.
int mapif_parse_RegistryRequest(int fd)
{
	if( RFIFOB(fd,12) )
	{// Load Char Registry
		struct regs charreg;
		inter_charreg_load(RFIFOL(fd,6), &charreg);
		mapif_regs_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),3,&charreg);
	}

	if( RFIFOB(fd,11) )
	{// Load Account Registry
		struct regs accreg;
		inter_accreg_load(RFIFOL(fd,2), &accreg);
		mapif_regs_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),2,&accreg);
	}

	if( RFIFOB(fd,10) )
	{// Ask Login Server for Account2 values.
		request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6));
	}

	return 1;
}

static void mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, char *name)
{
	WFIFOHEAD(fd, NAME_LENGTH+13);
	WFIFOW(fd, 0) = 0x3806;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = type;
	WFIFOB(fd,11) = flag;
	memcpy(WFIFOP(fd, 12), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+13);
}

int mapif_parse_NameChangeRequest(int fd)
{
	int account_id, char_id, type;
	char* name;
	int i;

	account_id = RFIFOL(fd,2);
	char_id = RFIFOL(fd,6);
	type = RFIFOB(fd,10);
	name = (char*)RFIFOP(fd,11);

	// Check Authorised letters/symbols in the name
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) == NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) != NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	}
	//TODO: type holds the type of object to rename.
	//If it were a player, it needs to have the guild information and db information
	//updated here, because changing it on the map won't make it be saved [Skotlex]

	//name allowed.
	mapif_namechange_ack(fd, account_id, char_id, type, 1, name);
	return 0;
}

//--------------------------------------------------------

/// Returns the length of the next complete packet to process,
/// or 0 if no complete packet exists in the queue.
///
/// @param length The minimum allowed length, or -1 for dynamic lookup
int inter_check_length(int fd, int length)
{
	if( length == -1 )
	{// variable-length packet
		if( RFIFOREST(fd) < 4 )
			return 0;
		length = RFIFOW(fd,2);
	}

	if( (int)RFIFOREST(fd) < length )
		return 0;

	return length;
}

// map server からの通信（１パケットのみ解析すること）
// エラーなら0(false)、処理できたなら1、
// パケット長が足りなければ2をかえさなければならない
int inter_parse_frommap(int fd)
{
	int cmd = RFIFOW(fd,0);
	int len = 0;

	// inter鯖管轄かを調べる
	if( cmd < 0x3000 || cmd >= 0x3000 + ARRAYLENGTH(inter_recv_packet_length) )
		return 0;

	//This is necessary, because otherwise we return 2 and the char server will just hang waiting for packets! [Skotlex]
	if (inter_recv_packet_length[cmd-0x3000] == 0)
		return 0;

	// パケット長を調べる
	len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000]);
	if( len == 0 )
		return 2;

	switch(cmd) {
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_Registry(fd); break;
	case 0x3005: mapif_parse_RegistryRequest(fd); break;
	case 0x3006: mapif_parse_NameChangeRequest(fd); break;
	default:
		if(  inter_party_parse_frommap(fd)
		  || inter_guild_parse_frommap(fd)
		  || inter_storage_parse_frommap(fd)
		  || inter_pet_parse_frommap(fd)
		  || inter_homun_parse_frommap(fd)
#ifndef TXT_ONLY
		  || inter_mail_parse_frommap(fd)
		  || inter_auction_parse_frommap(fd)
		  || inter_quest_parse_frommap(fd)
#endif
		   )
			break;
		else
			return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}
