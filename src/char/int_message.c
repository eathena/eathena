// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "if_map.h" // server[]
#include "int_message.h"
#include "charserverdb.h"
#include "online.h"
#include <string.h>

// temporary stuff
extern CharServerDB* charserver;

#define WISDATA_TTL (60*1000)	// Expiration time of non-acknowledged whisper data (60 seconds)
#define WISDELLIST_MAX 256   	// Number of elements of Wisp/page data deletion list

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[512];
};

static DBMap* wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;


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


// GM message sending
int mapif_parse_GMmessage(int fd)
{
	mapif_GMmessage(RFIFOP(fd,8), RFIFOW(fd,2), RFIFOL(fd,4), fd);
	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
// 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[2048];
	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf,0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// Wisp/page request to send
// 3001 <length>.w <src name>.24b <dst name>.24b <message>.?b
int mapif_parse_WisRequest(int fd)
{
	CharDB* chars = charserver->chardb(charserver);
	struct WisData* wd;
	char name[NAME_LENGTH];
	int fd2;
	struct online_char_data* character;
	int aid, cid;
	unsigned int n;

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
	if( !chars->name2id(chars, name, true, &cid, &aid, &n) &&// not exact
		!(!char_config.character_name_case_sensitive && chars->name2id(chars, name, false, &cid, &aid, &n) && n == 1) )// not unique
	{
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		return 0;
	}

	// if talking to self, don't ask other servers.
	if( strncmp((const char*)RFIFOP(fd,4), name, NAME_LENGTH) == 0 )
	{
		mapif_wis_end(fd, (char*)RFIFOP(fd,4), 1);
		return 0;
	}

	//Look for online character.
	character = onlinedb_get(aid);
	fd2 = ( character && character->char_id == cid && character->server > -1 ) ? server[character->server].fd : -1;
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


int inter_message_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	default:
		return 0;
	}
	return 1;
}

void inter_message_init(void)
{
	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);
}

void inter_message_final(void)
{
	wis_db->destroy(wis_db, NULL);
}
