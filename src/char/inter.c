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
#include "inter.h"
#include "int_auction.h"
#include "int_rank.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_mail.h"
#include "int_mercenary.h"
#include "int_message.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_quest.h"
#include "int_registry.h"
#include "int_status.h"
#include "int_storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static char inter_log_filename[1024] = "log/inter.log";
static bool log_inter_enabled = true; // interserver logging
char main_chat_nick[16] = "Main";
int guild_exp_rate = 100;
unsigned int party_share_level = 10;
bool party_break_without_leader = false;
bool party_auto_reassign_leader = false;


// recv. packet list
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1,13,36, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3000-  unsorted (messages, charreg)
	 0, 0, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,	// 3010-  Guild storage
	-1, 6,-1,14, 14,19, 6,-1, 14,14, 0, 0,  0, 0,  0, 0,	// 3020-  Party
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 18,19,186,-1,	// 3030-  Guild
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3040-  Guild (continued)
	-1,-1,10,10,  0,-1, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3050-  Auction System
	 6,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3060-  Quest system
	 7, 6,10,10, 10,-1, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3070-  Mail system
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3080-  Pet
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3090-  Homunculus
	 6,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 30A0-  Status
	10, 2, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 30B0-  Rank
	-1,10, 6,-1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 30C0-  Mercenary
};

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

		if(strcmpi(w1, "main_chat_nick")==0)
			strcpy(main_chat_nick, w2);
		else
		if( strcmpi(w1, "guild_exp_rate") == 0 )
			guild_exp_rate = atoi(w2);
		else
		if (strcmpi(w1, "party_share_level") == 0)
			party_share_level = atoi(w2);
		else
		if( strcmpi(w1, "party.break_without_leader") == 0 )
			party_break_without_leader = config_switch(w2);
		else
		if( strcmpi(w1, "party.auto_reassign_leader") == 0 )
			party_auto_reassign_leader = config_switch(w2);
		else
		if( strcmpi(w1, "log_inter") == 0 )
			log_inter_enabled = config_switch(w2);
		else
		if( strcmpi(w1, "inter_log_filename") == 0 )
			safestrncpy(inter_log_filename, w2, sizeof(inter_log_filename));
		else
		if (strcmpi(w1, "import") == 0)
			inter_config_read(w2);
	}
	fclose(fp);

	ShowInfo ("done reading %s.\n", cfgName);

	return 0;
}

/// Records an event in the interserver log
void log_inter(const char* fmt, ...)
{
	char timestamp[24+1];
	time_t now;
	FILE* log_fp;
	va_list ap;

	if( !log_inter_enabled )
		return;

	// open log file
	log_fp = fopen(inter_log_filename, "a");
	if( log_fp == NULL )
		return; // failed

	// write timestamp to log file
	time(&now);
	strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&now));
	fprintf(log_fp, "%s\t", timestamp);

	// write formatted message to log file
	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);

	// close log file
	fclose(log_fp);
}

// initialize
void inter_init(CharServerDB* db)
{
	inter_message_init();
	inter_registry_init(db->accregdb(db), db->charregdb(db));
	inter_status_init(db->statusdb(db));
	inter_party_init(db->partydb(db));
	inter_guild_init(db->guilddb(db), db->castledb(db));
	inter_storage_init(db->storagedb(db));
	inter_pet_init(db->petdb(db));
	inter_homun_init(db->homundb(db));
	inter_mercenary_init(db->mercdb(db));
	inter_mail_init(db->maildb(db), db->chardb(db));
	inter_quest_init(db->questdb(db));
	inter_auction_init(db->auctiondb(db), db->maildb(db));
	inter_rank_init(db->rankdb(db));
}

// finalize
void inter_final(void)
{
	inter_message_final();
	inter_registry_final();
	inter_status_final();
	inter_party_final();
	inter_guild_final();
	inter_storage_final();
	inter_pet_final();
	inter_homun_final();
	inter_mercenary_final();
	inter_mail_final();
	inter_quest_final();
	inter_auction_final();
	inter_rank_final();
}

// map server connection
void inter_mapif_init(int fd)
{
	inter_guild_mapif_init(fd);
}


//--------------------------------------------------------
// packets to map-server


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
// received packets from map-server


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
	if (char_config.char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_config.char_name_letters, name[i]) == NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	} else if (char_config.char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_config.char_name_letters, name[i]) != NULL) {
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
	case 0x3006: mapif_parse_NameChangeRequest(fd); break;
	default:
		if(  inter_message_parse_frommap(fd)
		  || inter_storage_parse_frommap(fd)
		  || inter_party_parse_frommap(fd)
		  || inter_guild_parse_frommap(fd)
		  || inter_mail_parse_frommap(fd)
		  || inter_quest_parse_frommap(fd)
		  || inter_auction_parse_frommap(fd)
		  || inter_pet_parse_frommap(fd)
		  || inter_homun_parse_frommap(fd)
		  || inter_mercenary_parse_frommap(fd)
		  || inter_status_parse_frommap(fd)
		  || inter_rank_parse_frommap(fd)
		  || inter_registry_parse_frommap(fd)
		   )
			break;
		else
			return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}
