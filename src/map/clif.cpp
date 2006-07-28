// $Id: clif.c 2200 2004-11-07 11:49:58Z Yor $

#define DUMP_UNKNOWN_PACKET
//#define	DUMP_ALL_PACKETS
#include "socket.h"
#include "timer.h"
#include "malloc.h"
#include "version.h"
#include "nullpo.h"
#include "showmsg.h"
#include "utils.h"

#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "status.h"
#include "npc.h"
#include "itemdb.h"
#include "homun.h"
#include "chat.h"
#include "trade.h"
#include "storage.h"
#include "script.h"
#include "skill.h"
#include "atcommand.h"
#include "charcommand.h"
#include "intif.h"
#include "battle.h"
#include "mob.h"
#include "party.h"
#include "guild.h"
#include "vending.h"
#include "pet.h"
#include "log.h"



// new packetdb needs different sections for sending and receiving
// receiving packets are keyed by command and contain format and (recv)processor
// sending packets are keyed by (send)processor and contain format and command
// this would elliminate the necessity of any PACKETVER define and packet_len_table
// (which is useless without the format)


// protocol version
#define PACKETVER			6

// packet DB
#define MAX_PACKET_DB		0x25f
#define MAX_PACKET_VER		19

class packet_cmd
{
public:
	short len;
	int (*func)(int,struct map_session_data &);
	short pos[20];

	packet_cmd()	{}
	~packet_cmd()	{}
	packet_cmd(short l, int (*f)(int,struct map_session_data &)=NULL, short p00=0, short p01=0, short p02=0, short p03=0, short p04=0, short p05=0, short p06=0, short p07=0, short p08=0, short p09=0, short p10=0, short p11=-11, short p12=0, short p13=0, short p14=0, short p15=0, short p16=0, short p17=0, short p18=0, short p19=0)
		: len(l),
		  func(f)
	{
		pos[0]=p00;
		pos[1]=p01;
		pos[2]=p02;
		pos[3]=p03;
		pos[4]=p04;
		pos[5]=p05;
		pos[6]=p06;
		pos[7]=p07;
		pos[8]=p08;
		pos[9]=p09;
		pos[10]=p10;
		pos[11]=p11;
		pos[12]=p12;
		pos[13]=p13;
		pos[14]=p14;
		pos[15]=p15;
		pos[16]=p16;
		pos[17]=p17;
		pos[18]=p18;
		pos[19]=p19;
	}
};

class packet_ver
{
	packet_cmd		cmd[MAX_PACKET_DB];
public:
	unsigned short	connect_cmd;

	packet_ver()	{}
	~packet_ver()	{}

	packet_cmd& operator[](size_t i)
	{
		if(i<MAX_PACKET_DB)
			return cmd[i];
		return cmd[0];
	}
};
class _packet_db
{
	packet_ver	vers[MAX_PACKET_VER+1];
public:
	int default_ver;

	_packet_db()
		: default_ver(MAX_PACKET_VER)
	{
		 memset(vers,0,sizeof(vers)); //!!
	}

	~_packet_db()	{}

	packet_ver& operator[](size_t i)
	{
		if(i<MAX_PACKET_VER+1)
			return vers[i];
		return vers[default_ver];
	}
} packet_db;




static const int packet_len_table[MAX_PACKET_DB] = {
   10,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
//#0x0040
    0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0, 55, 17,  3, 37,  46, -1, 23, -1,  3,108,  3,  2,
#if PACKETVER < 2
    3, 28, 19, 11,  3, -1,  9,  5,  52, 51, 56, 58, 41,  2,  6,  6,
#else	// 78-7b 亀島以降 lv99エフェクト用
    3, 28, 19, 11,  3, -1,  9,  5,  54, 53, 58, 60, 41,  2,  6,  6,
#endif
//#0x0080
    7,  3,  2,  2,  2,  5, 16, 12,  10,  7, 29,  2, -1, -1, -1,  0, // 0x8b changed to 2 (was 23)
    7, 22, 28,  2,  6, 30, -1, -1,   3, -1, -1,  5,  9, 17, 17,  6,
   23,  6,  6, -1, -1, -1, -1,  8,   7,  6,  7,  4,  7,  0, -1,  6,
    8,  8,  3,  3, -1,  6,  6, -1,   7,  6,  2,  5,  6, 44,  5,  3,
//#0x00C0
    7,  2,  6,  8,  6,  7, -1, -1,  -1, -1,  3,  3,  6,  3,  2, 27, // 0xcd change to 3 (was 6)
    3,  4,  4,  2, -1, -1,  3, -1,   6, 14,  3, -1, 28, 29, -1, -1,
   30, 30, 26,  2,  6, 26,  3,  3,   8, 19,  5,  2,  3,  2,  2,  2,
    3,  2,  6,  8, 21,  8,  8,  2,   2, 26,  3, -1,  6, 27, 30, 10,

//#0x0100
    2,  6,  6, 30, 79, 31, 10, 10,  -1, -1,  4,  6,  6,  2, 11, -1,
   10, 39,  4, 10, 31, 35, 10, 18,   2, 13, 15, 20, 68,  2,  3, 16,
    6, 14, -1, -1, 21,  8,  8,  8,   8,  8,  2,  2,  3,  4,  2, -1,
    6, 86,  6, -1, -1,  7, -1,  6,   3, 16,  4,  4,  4,  6, 24, 26,
//#0x0140
   22, 14,  6, 10, 23, 19,  6, 39,   8,  9,  6, 27, -1,  2,  6,  6,
  110,  6, -1, -1, -1, -1, -1,  6,  -1, 54, 66, 54, 90, 42,  6, 42,
   -1, -1, -1, -1, -1, 30, -1,  3,  14,  3, 30, 10, 43, 14,186,182,
   14, 30, 10,  3, -1,  6,106, -1,   4,  5,  4, -1,  6,  7, -1, -1,
//#0x0180
    6,  3,106, 10, 10, 34,  0,  6,   8,  4,  4,  4, 29, -1, 10,  6,
#if PACKETVER < 1
   90, 86, 24,  6, 30,102,  8,  4,   8,  4, 14, 10, -1,  6,  2,  6,
#else	// 196 comodo以降 状態表示アイコン用
   90, 86, 24,  6, 30,102,  9,  4,   8,  4, 14, 10, -1,  6,  2,  6,
#endif
    3,  3, 35,  5, 11, 26, -1,  4,   4,  6, 10, 12,  6, -1,  4,  4,
   11,  7, -1, 67, 12, 18,114,  6,   3,  6, 26, 26, 26, 26,  2,  3,
//#0x01C0,   Set 0x1d5=-1
    2, 14, 10, -1, 22, 22,  4,  2,  13, 97,  3,  9,  9, 30,  6, 28,
    8, 14, 10, 35,  6, -1,  4, 11,  54, 53, 60,  2, -1, 47, 33,  6,
   30,  8, 34, 14,  2,  6, 26,  2,  28, 81,  6, 10, 26,  2, -1, -1,
   -1, -1, 20, 10, 32,  9, 34, 14,   2,  6, 48, 56, -1,  4,  5, 10,
//#0x200
   26, -1,  26, 10, 18, 26, 11, 34,  14, 36, 10, 0,  0, -1, 24, 10, // 0x20c change to 0 (was 19)
   22,  0,  26, 26, 42, -1, -1,  2,   2,282,282,10, 10, -1, -1, 66,
   10, -1,  -1,  8, 10,  2,282, 18,  18, 15, 58, 57, 64, 5, 69,  0,
   12, 26,   9, 11, -1, -1, 10,  2, 282, 11,  4, 36, -1,-1,  4,  2,
   -1, -1,  -1, -1, -1,  3,  4,  8,  -1,  3, 70,  4,  8,12,  4, 10,
    3, 32,  -1,  3,  3,  5,  5,  8,   2,  3, -1, -1,  4,-1,  4
};


// local define
enum {
	ALL_CLIENT,
	ALL_SAMEMAP,
	AREA,
	AREA_WOS,
	AREA_WOC,
	AREA_WOSC,
	AREA_CHAT_WOC,
	CHAT,
	CHAT_WOS,
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,	// [Valaris]
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,	// end additions [Valaris]
	SELF
};

basics::netaddress	charaddress(basics::ipaddress::GetSystemIP(0), 6121);
basics::ipset		mapaddress(5121);

int map_fd=-1;
char talkie_mes[80];

basics::netaddress& getcharaddress()	{ return charaddress; }
basics::ipset& getmapaddress()			{ return mapaddress; }




inline void WBUFPOS(unsigned char* p, size_t pos, unsigned short x, unsigned short y, unsigned char d)
{
	if(p)
	{
		p += pos;
		*p++ = (unsigned char)(x>>2);
		*p++ = (unsigned char)((x<<6) | ((y>>4)&0x3f));
		*p   = (unsigned char)(y<<4) | d&0xF;
	}
}
inline void WBUFPOS2(unsigned char* p, size_t pos, unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)
{
	if(p)
	{
		p += pos;
		*p++ = (unsigned char)(x0>>2);
		*p++ = (unsigned char)((x0<<6) | ((y0>>4)&0x3f));
		*p++ = (unsigned char)((y0<<4) | ((x1>>6)&0x0f));
		*p++ = (unsigned char)((x1<<2) | ((y1>>8)&0x03));
		*p   = (unsigned char)(y1); 
	}
}

inline void WFIFOPOS(int fd, size_t pos, unsigned short x, unsigned short y, unsigned char d)
{
	WBUFPOS(WFIFOP(fd,pos),0,x,y,d); 
}
inline void WFIFOPOS2(int fd, size_t pos, unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)
{
	WBUFPOS2(WFIFOP(fd,pos),0,x0,y0,x1,y1); 
}





void dump_packet(FILE *fp, int fd, int packet_len)
{	// 不明なパケット
#if defined(DUMP_UNKNOWN_PACKET) || defined(DUMP_ALL_PACKETS)
	int i;
	fprintf(fp, "\t---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i = 0; i < packet_len; ++i)
	{
		if ((i & 15) == 0)
			fprintf(fp, "\n\t%04X ", i);
		fprintf(fp, "%02X ", (unsigned char)RFIFOB(fd,i));
	}
	fprintf(fp, "\n\n");
#endif
}

void dump_packet(int fd, int packet_len, map_session_data *sd)
{	// 不明なパケット
#if defined(DUMP_UNKNOWN_PACKET)

	static basics::CParam< basics::string<> > packet_txt("dump_packet_file", "save/packet.txt");
	FILE *fp = basics::safefopen(packet_txt(), "a");
	time_t now;
	if(fp == NULL)
	{
		ShowError("clif.c: cant write [%s] !!! data is lost !!!\n", (const char*)packet_txt());
		fp = stdout;
	}
	time(&now);
	if (sd && sd->state.auth)
	{
		if (sd->status.name != NULL)
			fprintf(fp, "%sPlayer with account ID %ld (character ID %ld, player name %s) sent wrong packet:\n",
					asctime(localtime(&now)), (unsigned long)sd->status.account_id, (unsigned long)sd->status.char_id, sd->status.name);
		else
			fprintf(fp, "%sPlayer with account ID %ld sent wrong packet:\n",
					asctime(localtime(&now)), (unsigned long)sd->block_list::id);
	}
	else if (sd) // not authentified! (refused by char-server or disconnect before to be authentified)
		fprintf(fp, "%sPlayer with account ID %ld sent wrong packet:\n", 
				asctime(localtime(&now)), (unsigned long)sd->block_list::id);
	
	dump_packet(fp, fd, packet_len);

	if(fp != stdout)
		fclose(fp);
#endif
}

void print_packet(int fd, int packet_len, map_session_data *sd)
{
#ifdef DUMP_ALL_PACKETS
	int i;
	if (fd)
		ShowMessage("\nclif_parse: session #%d, packet 0x%x, lenght %d\n", fd, cmd, packet_len);

	if (sd && sd->state.auth)
	{
		if (sd->status.name != NULL)
			ShowMessage("\nAccount ID %d, character ID %d, player name %s.\n",
					sd->status.account_id, sd->status.char_id, sd->status.name);
		else
			ShowMessage("\nAccount ID %d.\n", sd->block_list::id);
	}
	else if (sd) // not authentified! (refused by char-server or disconnect before to be authentified)
		ShowMessage("\nAccount ID %d.\n", sd->block_list::id);

	dump_packet(stdout, fd);
#endif
}







///////////////////////////////////////////////////////////////////////////////
// freya anti-bot system
unsigned short fake_mob_list[] =
{	// set here mobs that do not sound when they don't move
	1001, // scorpion
	1002, // poring
	1007, // fabre
	1031, // poporing
	1022  // werewolf
};
inline unsigned short get_fakemob_id(map_session_data &sd)
{	// base randomizer; the other wariables are known by the client
	const static int base = rand();	
	 // choose a mob which the client cannot trace back
	unsigned short id=fake_mob_list[(base + sd.block_list::m + sd.fd + sd.status.char_id) % (sizeof(fake_mob_list) / sizeof(fake_mob_list[0]))];
	return ( mobdb_checkid(id) >0 ) ? id : 1002; // poring in case of error
}

uint32 server_char_id    = 0;
uint32 server_fake_mob_id= 0;


void check_fake_id(int fd, struct map_session_data &sd, uint32 target_id)
{

	// if player asks for the fake player (only bot and modified client can see a hiden player)
	if (target_id == server_char_id)
	{
		char message_to_gm[1024];
		snprintf(message_to_gm, sizeof(message_to_gm), "Character '%s' (account: %d) try to use a bot", sd.status.name, sd.status.account_id);
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message_to_gm);
		
		// if we block people
		if (config.ban_bot < 0)
		{
			chrif_char_ask_name(-1, sd.status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
			session_SetWaitClose(sd.fd, 1000); // forced to disconnect because of the hack
			// message about the ban
		}
		// if we ban people
		else if (config.ban_bot > 0)
		{
			chrif_char_ask_name(-1, sd.status.name, 2, 0, 0, 0, 0, config.ban_bot, 0); // type: 2 - ban (year, month, day, hour, minute, second)
			session_SetWaitClose(sd.fd, 1000); // forced to disconnect because of the hack
			// message about the ban
		}
		else
		{	// impossible to display: we don't send fake player if config.ban_bot is == 0
			// message about the ban
		}
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message_to_gm);

		// send this info cause the bot ask until get an answer, damn spam
		memset(WFIFOP(fd,0), 0, packet_db[sd.packet_ver][0x95].len);
		WFIFOW(fd,0) = 0x95;
		WFIFOL(fd,2) = server_char_id;
		memcpy(WFIFOP(fd,6), sd.status.name, 24);
		WFIFOSET(fd, packet_db[sd.packet_ver][0x95].len);

		// take fake player out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_char_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet_db[sd.packet_ver][0x80].len);
		// take fake mob out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_fake_mob_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet_db[sd.packet_ver][0x80].len);
		return;
	}

	// if player asks for the fake mob (only bot and modified client can see a hiden mob)
	if (target_id == server_fake_mob_id)
	{
		char message_to_gm[1024];
		snprintf(message_to_gm, sizeof(message_to_gm), "Character '%s' (account: %d) try to use a bot", sd.status.name, sd.status.account_id);
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message_to_gm);
		// if we block people
		if (config.ban_bot < 0)
		{
			chrif_char_ask_name(-1, sd.status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
			session_SetWaitClose(sd.fd, 1000); // forced to disconnect because of the hack
			// message about the ban
		}
		// if we ban people
		else if (config.ban_bot > 0)
		{
			chrif_char_ask_name(-1, sd.status.name, 2, 0, 0, 0, 0, config.ban_bot, 0); // type: 2 - ban (year, month, day, hour, minute, second)
			session_SetWaitClose(sd.fd, 1000); // forced to disconnect because of the hack
			// message about the ban
		}
		else
		{	// impossible to display: we don't send fake player if config.ban_bot is == 0
			// message about the ban
		}
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message_to_gm);

		// send this info cause the bot ask until get an answer, damn spam
		memset(WFIFOP(fd,0), 0, packet_db[sd.packet_ver][0x95].len);
		WFIFOW(fd,0) = 0x95;
		WFIFOL(fd,2) = server_fake_mob_id;
		memcpy(WFIFOP(fd,6), mob_db[ get_fakemob_id(sd) ].name, 24);
		WFIFOSET(fd, packet_db[sd.packet_ver][0x95].len);

		// take fake mob out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_fake_mob_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet_db[sd.packet_ver][0x80].len);
		// take fake player out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_char_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet_db[sd.packet_ver][0x80].len);
		return;
	}
	
}
void send_fake_id(int fd, struct map_session_data &sd)
{	// if we try to detect bot
	if (config.ban_bot)
	{	// don't send fake player if we would not detect bot
		// send fake player (exactly same of the player, with HIDE option and char_id of the wisp server name)
#if PACKETVER < 4
		WFIFOW(fd, 0)= 0x78;
		WFIFOL(fd, 2)= server_char_id;
		WFIFOW(fd, 6)= sd.speed;
		WFIFOW(fd, 8)= sd.opt1;
		WFIFOW(fd,10)= sd.opt2;
		WFIFOW(fd,12)= OPTION_HIDE;
		WFIFOW(fd,14)= sd.view_class;
		WFIFOW(fd,16)= sd.status.hair;
		WFIFOW(fd,18) = (sd.view_class != 22) ? sd.status.weapon:0;
		WFIFOW(fd,20) = sd.status.head_bottom;
		WFIFOW(fd,22) = sd.status.shield;
		WFIFOW(fd,24) = sd.status.head_top;
		WFIFOW(fd,26) = sd.status.head_mid;
		WFIFOW(fd,28) = sd.status.hair_color;
		WFIFOW(fd,30) = sd.status.clothes_color;
		WFIFOW(fd,32) = sd.head_dir;
		WFIFOL(fd,34) = sd.status.guild_id;
		WFIFOL(fd,38) = sd.guild_emblem_id;
		WFIFOW(fd,42) = sd.status.manner;
		WFIFOB(fd,44) = sd.status.karma;
		WFIFOB(fd,45) = sd.status.sex;
		WFIFOPOS(fd, 46, sd.block_list::x, sd.block_list::y, sd.dir);
		WFIFOB(fd,49) = 5;
		WFIFOB(fd,50) = 5;
		WFIFOB(fd,51) = sd.state.dead_sit;
		WFIFOW(fd,52) = (sd.status.base_level > config.max_lv) ? config.max_lv : sd.status.base_level;
		SENDPACKET(sd.fd, packet_db[sd.packet_ver][0x78].len);
#else
		unsigned short level;
		WFIFOW(fd, 0) = 0x1d8;
		WFIFOL(fd, 2) = server_char_id;
		WFIFOW(fd, 6) = sd.speed;
		WFIFOW(fd, 8) = sd.opt1;
		WFIFOW(fd,10) = sd.opt2;
		WFIFOW(fd,12) = OPTION_HIDE;
		WFIFOW(fd,14) = sd.view_class;
		WFIFOW(fd,16) = sd.status.hair;
		if(sd.equip_index[9] < MAX_INVENTORY && sd.inventory_data[sd.equip_index[9]] && sd.view_class != 22) {
			if (sd.inventory_data[sd.equip_index[9]]->view_id > 0)
				WFIFOW(fd,18) = sd.inventory_data[sd.equip_index[9]]->view_id;
			else
				WFIFOW(fd,18) = sd.status.inventory[sd.equip_index[9]].nameid;
		} else
			WFIFOW(fd,18) = 0;
		if (sd.equip_index[8] < MAX_INVENTORY && sd.equip_index[8] != sd.equip_index[9] && sd.inventory_data[sd.equip_index[8]] && sd.view_class != 22) {
			if (sd.inventory_data[sd.equip_index[8]]->view_id > 0)
				WFIFOW(fd,20) = sd.inventory_data[sd.equip_index[8]]->view_id;
			else
				WFIFOW(fd,20) = sd.status.inventory[sd.equip_index[8]].nameid;
		} else
			WFIFOW(fd,20) = 0;
		WFIFOW(fd,22) = sd.status.head_bottom;
		WFIFOW(fd,24) = sd.status.head_top;
		WFIFOW(fd,26) = sd.status.head_mid;
		WFIFOW(fd,28) = sd.status.hair_color;
		WFIFOW(fd,30) = sd.status.clothes_color;
		WFIFOW(fd,32) = sd.head_dir;
		WFIFOL(fd,34) = sd.status.guild_id;
		WFIFOW(fd,38) = sd.guild_emblem_id;
		WFIFOW(fd,40) = sd.status.manner;
		WFIFOW(fd,42) = sd.opt3;
		WFIFOB(fd,44) = sd.status.karma;
		WFIFOB(fd,45) = sd.status.sex;
		WFIFOPOS(fd, 46, sd.block_list::x, sd.block_list::y, sd.dir);
		WFIFOB(fd,49) = 5;
		WFIFOB(fd,50) = 5;
		WFIFOB(fd,51) = sd.state.dead_sit;
		WFIFOW(fd,52) = ((level = status_get_lv(&sd)) > config.max_job_level) ? config.max_job_level : level;
		WFIFOSET(sd.fd, packet_db[sd.packet_ver][0x1d8].len);
#endif
		// send a fake monster
		memset(WFIFOP(fd, 0), 0, packet_db[sd.packet_ver][0x7c].len);
		WFIFOW(fd, 0) = 0x7c;
		WFIFOL(fd, 2) = server_fake_mob_id;
		WFIFOW(fd, 6) = sd.speed;
		WFIFOW(fd, 8) = 0;
		WFIFOW(fd,10) = 0;
		WFIFOW(fd,12) = OPTION_HIDE;
		WFIFOW(fd,20) = get_fakemob_id(sd);
		WFIFOPOS(fd, 36, sd.block_list::x, sd.block_list::y, sd.dir);
		WFIFOSET(sd.fd, packet_db[sd.packet_ver][0x7c].len);
	}
}

bool clif_packetsend(int fd, struct map_session_data &sd, unsigned short cmd, int info[], size_t sz)
{
	if(cmd < MAX_PACKET_DB)
	{
		size_t i;
		switch (cmd)
		{
		case 0x209:
			WFIFOW(fd,0) = 0x209;
			WFIFOW(fd,2) = 2;
			memcpy(WFIFOP(fd, 12), sd.status.name, 24);
			WFIFOSET(fd, packet_db[sd.packet_ver][cmd].len);
			break;
		case 0x1b1:
		case 0x1c2:
		//case xxx:
		//	add others here
		//	break;
		default:
			WFIFOW(fd,0) = cmd;
			for(i=1;i<=sz; ++i)
				if(info[i])
					WFIFOW(fd,i) = info[i];
			WFIFOSET(fd, packet_db[sd.packet_ver][cmd].len);
			break;
		}
		return true;
	}
	return false;
}



/*==========================================
 *
 *------------------------------------------
 */
int clif_countusers(void)
{
	size_t users = 0, i;
	struct map_session_data *sd;

	for(i = 0; i < fd_max; ++i)
	{
		if( session[i] && 
			(sd = (struct map_session_data*)session[i]->user_session) && 
			sd->state.auth &&
		    !(config.hide_GM_session && sd->isGM()) )
			users++;
	}
	return users;
}

/*==========================================
 * 全てのclientに対してfunc()実行
 *------------------------------------------
 */
int clif_foreachclient(const CClifProcessor& elem)
{
	int returnvalue = 0;
	size_t i;
	struct map_session_data *sd;
	for(i=0; i<fd_max; ++i)
	{
		if(session[i] && (sd = (struct map_session_data*)session[i]->user_session) && sd->state.auth)
		{
			returnvalue += elem.process(*sd);
		}
	}
	return returnvalue;
}
/*==========================================
 * clif_sendでAREA*指定時用
 *------------------------------------------
 */
class CClifSend : public CMapProcessor
{
	unsigned char *&buf;
	size_t len;
	const block_list &src_bl;
	int type;
public:

	CClifSend(unsigned char *&b, size_t l, const block_list &bl, int ty)
		: buf(b), len(l), src_bl(bl), type(ty)
	{}
	~CClifSend()	{}

	virtual int process(struct block_list& bl) const
	{
		struct map_session_data& sd = (struct map_session_data&)bl;
		if(bl.type==BL_PC && session_isActive(sd.fd) )
		{
			switch(type) {
			case AREA_WOS:
			case AREA_CHAT_WOC:
				if(sd.block_list::id == src_bl.id)
					return 0;
				break;
			case AREA_WOC:
				if (sd.chatID || sd.block_list::id == src_bl.id)
					return 0;
				break;
			case AREA_WOSC:
			{
				struct map_session_data &ssd = (struct map_session_data &)(src_bl);
				if(src_bl.type==BL_PC && sd.chatID && sd.chatID == ssd.chatID)
					return 0;
				break;
			}
			}
			if(WFIFOP(sd.fd,0) == buf)
			{
				ShowMessage("WARNING: Invalid use of clif_send function\n");
				ShowMessage("         Packet x%4x use a WFIFO of a player instead of to use a buffer.\n", (unsigned short)WBUFW(buf,0));
				ShowMessage("         Please correct your code.\n");
				// don't send to not move the pointer of the packet for next sessions in the loop
			}
			else
			{
				if(packet_db[sd.packet_ver][RBUFW(buf,0)].len)
				{	// packet must exist for the client version
					memcpy(WFIFOP(sd.fd,0), buf, len);
					WFIFOSET(sd.fd,len);
				}
			}
		}
		return 0;
	}
};
/*==========================================
 * seperate this stupid thing, 
 * just use single functions for now and 
 * change it to the prepared packet sender 
 * once the packet objects got implemented
 *------------------------------------------
 */
int clif_send (unsigned char *buf, size_t len, const block_list *bl, int type)
{
	size_t i;
	struct map_session_data *sd = NULL;
	struct party *p = NULL;
	struct guild *g = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	if (type != ALL_CLIENT)
	{
		if(!bl)
		{
			printf("clif_send nullpo, head: %02X%02X%02X%02X type %i, len %li\n", buf[0], buf[1], buf[2], buf[3], type, (unsigned long)len);
			return 0;
		}
		if(bl->type == BL_PC)
			sd = (struct map_session_data *)bl;
	}

	switch(type) {
	case ALL_CLIENT: // 全クライアントに送信
		for (i = 0; i < fd_max; ++i) {
			if (session[i] && (sd = (struct map_session_data *)session[i]->user_session) != NULL && sd->state.auth) {
				if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
					memcpy(WFIFOP(i,0), buf, len);
					WFIFOSET(i,len);
				}
			}
		}
		break;
	case ALL_SAMEMAP: // 同じマップの全クライアントに送信
		for(i = 0; i < fd_max; ++i) {
			if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL &&
				sd->state.auth && sd->block_list::m == bl->m) {
				if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
					memcpy(WFIFOP(i,0), buf, len);
					WFIFOSET(i,len);
				}
			}
		}
		break;
	case AREA:
	case AREA_WOS:
	case AREA_WOC:
	case AREA_WOSC:
		CMap::foreachinarea( CClifSend(buf, len, *bl, type),
			bl->m, ((int)bl->x)-AREA_SIZE, ((int)bl->y)-AREA_SIZE, ((int)bl->x)+AREA_SIZE, ((int)bl->y)+AREA_SIZE,BL_PC);
//		map_foreachinarea(clif_send_sub, 
//			bl->m, ((int)bl->x)-AREA_SIZE, ((int)bl->y)-AREA_SIZE, ((int)bl->x)+AREA_SIZE, ((int)bl->y)+AREA_SIZE,BL_PC, 
//			buf, len, bl, type);
		break;
	case AREA_CHAT_WOC:
		CMap::foreachinarea( CClifSend(buf, len, *bl, type),
			bl->m, ((int)bl->x)-(AREA_SIZE-5), ((int)bl->y)-(AREA_SIZE-5),((int)bl->x)+(AREA_SIZE-5), ((int)bl->y)+(AREA_SIZE-5), BL_PC);
//		map_foreachinarea(clif_send_sub, 
//			bl->m, ((int)bl->x)-(AREA_SIZE-5), ((int)bl->y)-(AREA_SIZE-5),((int)bl->x)+(AREA_SIZE-5), ((int)bl->y)+(AREA_SIZE-5), BL_PC, 
//			buf, len, bl, AREA_WOC);
		break;
	case CHAT:
	case CHAT_WOS:
		{
			chat_data *cd;
			if (sd) {
				cd = (chat_data*)map_id2bl(sd->chatID);
			} else if (bl->type == BL_CHAT) {
				cd = (chat_data*)bl;
			} else break;
			if (cd == NULL)
				break;
			for(i = 0; i < cd->users; ++i) {
				if (type == CHAT_WOS && cd->usersd[i] == sd)
					continue;
				if (packet_db[cd->usersd[i]->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
					if (cd->usersd[i]->fd >=0 && session[cd->usersd[i]->fd]) // Added check to see if session exists [PoW]
						memcpy(WFIFOP(cd->usersd[i]->fd,0), buf, len);
					WFIFOSET(cd->usersd[i]->fd,len);
				}
			}
		}
		break;

	case PARTY_AREA:		// 同じ画面内の全パーティーメンバに送信
	case PARTY_AREA_WOS:	// 自分以外の同じ画面内の全パーティーメンバに送信
		x0 = ((int)bl->x) - AREA_SIZE;
		y0 = ((int)bl->y) - AREA_SIZE;
		x1 = ((int)bl->x) + AREA_SIZE;
		y1 = ((int)bl->y) + AREA_SIZE;
	case PARTY:				// 全パーティーメンバに送信
	case PARTY_WOS:			// 自分以外の全パーティーメンバに送信
	case PARTY_SAMEMAP:		// 同じマップの全パーティーメンバに送信
	case PARTY_SAMEMAP_WOS:	// 自分以外の同じマップの全パーティーメンバに送信
		if (sd) {
			if (sd->partyspy > 0) {
				p = party_search(sd->partyspy);
			} else if (sd->status.party_id > 0) {
				p = party_search(sd->status.party_id);
			}
		}
		if (p) {
			for(i=0;i<MAX_PARTY; ++i){
				if ((sd = p->member[i].sd) != NULL) {
					if ((session[sd->fd] == NULL) || (session[sd->fd]->user_session == NULL))
						continue;
					if (sd->block_list::id == bl->id && (type == PARTY_WOS ||
					    type == PARTY_SAMEMAP_WOS || type == PARTY_AREA_WOS))
						continue;
					if (type != PARTY && type != PARTY_WOS && bl->m != sd->block_list::m) // マップチェック
						continue;
					if ((type == PARTY_AREA || type == PARTY_AREA_WOS) &&
					    (sd->block_list::x < x0 || sd->block_list::y < y0 ||
					     sd->block_list::x > x1 || sd->block_list::y > y1))
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
//					if(config.etc_log)
//						ShowMessage("send party %d %d %d\n",p->party_id,i,flag)

				}
			}
			for (i = 0; i < fd_max; ++i){
				if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL && sd->state.auth) {
					if (sd->partyspy == p->party_id) {
						if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
							memcpy(WFIFOP(sd->fd,0), buf, len);
							WFIFOSET(sd->fd,len);
						}
					}
				}
			}
		}
		break;
	case SELF:
		if (sd && session_isActive(sd->fd) && session[sd->fd]->user_session && packet_db[sd->packet_ver][RBUFW(buf,0)].len ) { // packet must exist for the client version
			memcpy(WFIFOP(sd->fd,0), buf, len);
			WFIFOSET(sd->fd,len);
		}
		break;

/* New definitions for guilds [Valaris]	*/

	case GUILD_AREA:
	case GUILD_AREA_WOS:
		x0 = ((int)bl->x) - AREA_SIZE;
		y0 = ((int)bl->y) - AREA_SIZE;
		x1 = ((int)bl->x) + AREA_SIZE;
		y1 = ((int)bl->y) + AREA_SIZE;
	case GUILD:
	case GUILD_WOS:
		if (sd) { // guildspy [Syrus22]
			if (sd->guildspy > 0) {
				g = guild_search(sd->guildspy);
			} else if (sd->status.guild_id > 0) {
				g = guild_search(sd->status.guild_id);
			}
		}
		if (g)
		{
			for(i = 0; i < g->max_member; ++i)
			{
				if ((sd = g->member[i].sd) != NULL)
				{
					if (session[sd->fd] == NULL || sd->state.auth == 0 || session[sd->fd]->user_session == NULL)
						continue;
					if (type == GUILD_WOS && sd->block_list::id == bl->id)
						continue;
					if (sd->packet_ver > MAX_PACKET_VER)
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
			for (i = 0; i < fd_max; ++i)
			{
				if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL && sd->state.auth)
				{
					if (sd->guildspy == g->guild_id)
					{
						if (packet_db[sd->packet_ver][RBUFW(buf,0)].len)
						{	// packet must exist for the client version
							memcpy(WFIFOP(sd->fd,0), buf, len);
							WFIFOSET(sd->fd,len);
						}
					}
				}
			}
		}
		break;
	case GUILD_SAMEMAP:
	case GUILD_SAMEMAP_WOS:
		if (sd && sd->status.guild_id > 0) {
			g = guild_search(sd->status.guild_id);
		}
		if (g) {
			for(i = 0; i < g->max_member; ++i) {
				if ((sd = g->member[i].sd) != NULL) {
					if (sd->block_list::id == bl->id && (type == GUILD_WOS ||
					    type == GUILD_SAMEMAP_WOS || type == GUILD_AREA_WOS))
						continue;
					if (type != GUILD && type != GUILD_WOS && bl->m != sd->block_list::m) // マップチェック
						continue;
					if ((type == GUILD_AREA || type == GUILD_AREA_WOS) &&
					    (sd->block_list::x < x0 || sd->block_list::y < y0 ||
					     sd->block_list::x > x1 || sd->block_list::y > y1))
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
		}
		break;
	default:
		if (config.error_log)
			ShowMessage("clif_send まだ作ってないよー\n");
		return -1;
	}

	return 0;
}




void clif_send_all(unsigned char *buf, size_t len)
{
	size_t i;
	struct map_session_data *sd;
	for (i=0; i<fd_max; ++i)
	{
		if( session[i] && (sd = (struct map_session_data *)session[i]->user_session) != NULL && 
			sd->state.auth )
		{
			if( packet_db[sd->packet_ver][RBUFW(buf,0)].len &&
				session_isActive(i) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(i,0), buf, len);
				WFIFOSET(i,len);
			}
		}
	}
}
void clif_send_same_map(const block_list& bl, unsigned char *buf, size_t len)
{
	size_t i;
	struct map_session_data *sd;
	for(i=0; i<fd_max; ++i)
	{
		if( session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL &&
			sd->state.auth && sd->block_list::m == bl.m)
		{
			if( packet_db[sd->packet_ver][RBUFW(buf,0)].len &&
				session_isActive(i) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(i,0), buf, len);
				WFIFOSET(i,len);
			}
		}
	}
}

void clif_send_self(const block_list& bl, unsigned char *buf, size_t len)
{
	const struct map_session_data *sd = bl.get_sd();
	if( sd && session_isActive(sd->fd) && session[sd->fd]->user_session && 
		packet_db[sd->packet_ver][RBUFW(buf,0)].len )
	{
		memcpy(WFIFOP(sd->fd,0), buf, len);
		WFIFOSET(sd->fd,len);
	}
}
void clif_send_chat(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	size_t i;
	const chat_data *cd = bl.get_cd();
	const map_session_data *sd = bl.get_sd();

	if (sd)
		cd = (const chat_data*)map_id2bl(sd->chatID);

	if( cd )
	{
		for(i=0; i<cd->users; ++i)
		{
			if( type==CHAT_WOS && cd->usersd[i]==sd )
				continue;
			if( packet_db[cd->usersd[i]->packet_ver][RBUFW(buf,0)].len &&
				session_isActive(cd->usersd[i]->fd) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(cd->usersd[i]->fd,0), buf, len);
				WFIFOSET(cd->usersd[i]->fd,len);
			}
		}
	}
}
void clif_send_area(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	CMap::foreachinarea( CClifSend(buf, len, bl, type),
		bl.m, ((int)bl.x)-AREA_SIZE, ((int)bl.y)-AREA_SIZE, ((int)bl.x)+AREA_SIZE, ((int)bl.y)+AREA_SIZE, BL_PC);
}
void clif_send_chatarea(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	CMap::foreachinarea( CClifSend(buf, len, bl, type),
		bl.m, ((int)bl.x)-(AREA_SIZE-5), ((int)bl.y)-(AREA_SIZE-5), ((int)bl.x)+(AREA_SIZE-5), ((int)bl.y)+(AREA_SIZE-5), BL_PC);
}


void clif_send_party(const block_list& bl, unsigned char *buf, size_t len, int type)
{
/*
	case PARTY_AREA:		// 同じ画面内の全パーティーメンバに送信
	case PARTY_AREA_WOS:	// 自分以外の同じ画面内の全パーティーメンバに送信
		x0 = ((int)bl->x) - AREA_SIZE;
		y0 = ((int)bl->y) - AREA_SIZE;
		x1 = ((int)bl->x) + AREA_SIZE;
		y1 = ((int)bl->y) + AREA_SIZE;
	case PARTY:				// 全パーティーメンバに送信
	case PARTY_WOS:			// 自分以外の全パーティーメンバに送信
	case PARTY_SAMEMAP:		// 同じマップの全パーティーメンバに送信
	case PARTY_SAMEMAP_WOS:	// 自分以外の同じマップの全パーティーメンバに送信
		if (sd) {
			if (sd->partyspy > 0) {
				p = party_search(sd->partyspy);
			} else if (sd->status.party_id > 0) {
				p = party_search(sd->status.party_id);
			}
		}
		if (p) {
			for(i=0;i<MAX_PARTY; ++i){
				if ((sd = p->member[i].sd) != NULL) {
					if ((session[sd->fd] == NULL) || (session[sd->fd]->user_session == NULL))
						continue;
					if (sd->block_list::id == bl->id && (type == PARTY_WOS ||
					    type == PARTY_SAMEMAP_WOS || type == PARTY_AREA_WOS))
						continue;
					if (type != PARTY && type != PARTY_WOS && bl->m != sd->block_list::m) // マップチェック
						continue;
					if ((type == PARTY_AREA || type == PARTY_AREA_WOS) &&
					    (sd->block_list::x < x0 || sd->block_list::y < y0 ||
					     sd->block_list::x > x1 || sd->block_list::y > y1))
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
//					if(config.etc_log)
//						ShowMessage("send party %d %d %d\n",p->party_id,i,flag)

				}
			}
			for (i = 0; i < fd_max; ++i){
				if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL && sd->state.auth) {
					if (sd->partyspy == p->party_id) {
						if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
							memcpy(WFIFOP(sd->fd,0), buf, len);
							WFIFOSET(sd->fd,len);
						}
					}
				}
			}
		}
*/
}
void clif_send_guild(const block_list& bl, unsigned char *buf, size_t len, int type)
{
/*
	size_t i;
	struct map_session_data *sd = NULL;
	struct party *p = NULL;
	struct guild *g = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	if (type != ALL_CLIENT)
	{
		if(!bl)
		{
			printf("clif_send nullpo, head: %02X%02X%02X%02X type %i, len %li\n", buf[0], buf[1], buf[2], buf[3], type, (unsigned long)len);
			return 0;
		}

//		nullpo_retr(0, bl);
		if(bl->type == BL_PC)
			sd = (struct map_session_data *)bl;
	}

	switch(type) {



	case GUILD_AREA:
	case GUILD_AREA_WOS:
		x0 = ((int)bl->x) - AREA_SIZE;
		y0 = ((int)bl->y) - AREA_SIZE;
		x1 = ((int)bl->x) + AREA_SIZE;
		y1 = ((int)bl->y) + AREA_SIZE;
	case GUILD:
	case GUILD_WOS:
		if (sd) { // guildspy [Syrus22]
			if (sd->guildspy > 0) {
				g = guild_search(sd->guildspy);
			} else if (sd->status.guild_id > 0) {
				g = guild_search(sd->status.guild_id);
			}
		}
		if (g)
		{
			for(i = 0; i < g->max_member; ++i)
			{
				if ((sd = g->member[i].sd) != NULL)
				{
					if (session[sd->fd] == NULL || sd->state.auth == 0 || session[sd->fd]->user_session == NULL)
						continue;
					if (type == GUILD_WOS && sd->block_list::id == bl->id)
						continue;
					if (sd->packet_ver > MAX_PACKET_VER)
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
			for (i = 0; i < fd_max; ++i)
			{
				if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) != NULL && sd->state.auth)
				{
					if (sd->guildspy == g->guild_id)
					{
						if (packet_db[sd->packet_ver][RBUFW(buf,0)].len)
						{	// packet must exist for the client version
							memcpy(WFIFOP(sd->fd,0), buf, len);
							WFIFOSET(sd->fd,len);
						}
					}
				}
			}
		}
		break;
	case GUILD_SAMEMAP:
	case GUILD_SAMEMAP_WOS:
		if (sd && sd->status.guild_id > 0) {
			g = guild_search(sd->status.guild_id);
		}
		if (g) {
			for(i = 0; i < g->max_member; ++i) {
				if ((sd = g->member[i].sd) != NULL) {
					if (sd->block_list::id == bl->id && (type == GUILD_WOS ||
					    type == GUILD_SAMEMAP_WOS || type == GUILD_AREA_WOS))
						continue;
					if (type != GUILD && type != GUILD_WOS && bl->m != sd->block_list::m) // マップチェック
						continue;
					if ((type == GUILD_AREA || type == GUILD_AREA_WOS) &&
					    (sd->block_list::x < x0 || sd->block_list::y < y0 ||
					     sd->block_list::x > x1 || sd->block_list::y > y1))
						continue;
					if (packet_db[sd->packet_ver][RBUFW(buf,0)].len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
		}
		break;
	default:
		if (config.error_log)
			ShowMessage("clif_send まだ作ってないよー\n");
		return -1;
	}
*/
}





//
// パケット作って送信
//
/*==========================================
 *
 *------------------------------------------
 */
int clif_authok(struct map_session_data &sd)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) = 0x73;
	WFIFOL(fd, 2) = gettick();
	WFIFOPOS(fd, 6, sd.block_list::x, sd.block_list::y, sd.dir);
	WFIFOB(fd, 9) = 5;
	WFIFOB(fd,10) = 5;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x73].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse(int fd);
int clif_authfail(struct map_session_data &sd, uint32 type)
{
	int fd = sd.fd;
	if( !session_isActive(fd) || session[fd]->func_parse!=clif_parse)
		return 0;

	WFIFOW(fd,0) = 0x81;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x81].len);

	ShowDebug("clif_authfail: Disconnecting session #%d\n", fd);
	session_SetWaitClose(fd, 5000);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_charselectok(uint32 id)
{
	struct map_session_data *sd=map_id2sd(id);
	int fd;

	if( sd == NULL)
		return 1;

	fd = sd->fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xb3;
	WFIFOB(fd,2) = 1;
	WFIFOSET(fd,packet_db[sd->packet_ver][0xb3].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_dropflooritem(flooritem_data &fitem)
{
	unsigned char buf[64];
	int view;

	if(fitem.item_data.nameid <= 0)
		return 0;

	WBUFW(buf, 0) = 0x9e;
	WBUFL(buf, 2) = fitem.block_list::id;
	if ((view = itemdb_viewid(fitem.item_data.nameid)) > 0)
		WBUFW(buf, 6) = view;
	else
		WBUFW(buf, 6) = fitem.item_data.nameid;
	WBUFB(buf, 8) = fitem.item_data.identify;
	WBUFW(buf, 9) = fitem.block_list::x;
	WBUFW(buf,11) = fitem.block_list::y;
	WBUFB(buf,13) = fitem.subx;
	WBUFB(buf,14) = fitem.suby;
	WBUFW(buf,15) = fitem.item_data.amount;

	return clif_send(buf, packet_len_table[0x9e], &fitem, AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearflooritem(flooritem_data &fitem, int fd)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0xa1;
	WBUFL(buf,2) = fitem.block_list::id;

	if (fd == 0) {
		clif_send(buf, packet_len_table[0xa1], &fitem, AREA);
	} else if( session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0), buf, 6);
		WFIFOSET(fd,packet_len_table[0xa1]);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchar(struct block_list &bl, unsigned char type)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl.id;
	WBUFB(buf,6) = type;
	clif_send(buf, packet_len_table[0x80], &bl, (type&1) ? AREA : AREA_WOS);

	if(bl.type==BL_PC && ((struct map_session_data &)bl).disguise_id)
	{
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		clif_send(buf, packet_len_table[0x80], &bl, AREA);
	}

	return 0;
}
int clif_clearchar(struct map_session_data &sd, struct block_list &bl)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl.id;
	WBUFB(buf,6) = 0;
	clif_send_self(sd,buf, packet_db[sd.packet_ver][0x80].len);

	if(bl.type==BL_PC && ((struct map_session_data &)bl).disguise_id)
	{
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		clif_send_self(sd,buf, packet_db[sd.packet_ver][0x80].len);
	}

	return 0;
}


int clif_clearchar_delay_sub(int tid, unsigned long tick, int id, basics::numptr data) 
{
	struct block_list *bl = (struct block_list *)data.ptr;
	if(bl)
	{
		clif_clearchar(*bl,id);
		bl->map_freeblock();
		// clear the pointer transfering timer
		get_timer(tid)->data=0;
	}

	return 0;
}

int clif_clearchar_delay(unsigned long tick, struct block_list &bl, int type) 
{
	struct block_list *tmpbl = new struct block_list(bl);
	add_timer(tick, clif_clearchar_delay_sub, type, basics::numptr(tmpbl), false);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchar_id(const map_session_data& sd, uint32 id, unsigned char type)
{
	if( !session_isActive(sd.fd) )
		return 0;

	WFIFOW(sd.fd,0) = 0x80;
	WFIFOL(sd.fd,2) = id;
	WFIFOB(sd.fd,6) = type;
	WFIFOSET(sd.fd, packet_db[sd.packet_ver][0x80].len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set0078(const map_session_data &sd, unsigned char *buf)
{
	unsigned int level;

	if(sd.disguise_id > 23 && sd.disguise_id < 4001)
	{	// mob disguises [Valaris]
		memset(buf, 0, packet_len_table[0x78]);
		WBUFW(buf,0) = 0x78;
		WBUFL(buf,2) = sd.block_list::id;
		WBUFW(buf,6) = sd.get_speed();
		WBUFW(buf,8) = sd.opt1;
		WBUFW(buf,10) = sd.opt2;
		WBUFW(buf,12) = sd.status.option;
		WBUFW(buf,14) = sd.disguise_id;
		WBUFW(buf,42) = 0;
		WBUFB(buf,44) = 0;
		WBUFPOS(buf, 46, sd.block_list::x, sd.block_list::y, sd.dir);
		WBUFB(buf,49) = 5;
		WBUFB(buf,50) = 5;
		WBUFB(buf,51) = 0;
		WBUFW(buf,52) = ((level = status_get_lv(&sd)) > config.max_base_level) ? config.max_base_level : level;

		return packet_len_table[0x78];
	}

#if PACKETVER < 4
	memset(buf, 0, packet_len_table[0x78]);
	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.get_speed();
	WBUFW(buf,8)=sd.opt1;
	WBUFW(buf,10)=sd.opt2;
	if(sd->disguise_id) {
		WBUFW(buf,12)=OPTION_HIDE;
	} else {
		WBUFW(buf,12)=sd.status.option;
	}	
	WBUFW(buf,14)=sd.view_class;
	WBUFW(buf,16)=sd.status.hair;
	if (sd.view_class != 22)
		WBUFW(buf,18) = sd.status.weapon;
	else
		WBUFW(buf,18)=0;
	WBUFW(buf,20)=sd.status.head_bottom;
	WBUFW(buf,22)=sd.status.shield;
	WBUFW(buf,24)=sd.status.head_top;
	WBUFW(buf,26)=sd.status.head_mid;
	WBUFW(buf,28)=sd.status.hair_color;
	WBUFW(buf,30)=sd.status.clothes_color;
	WBUFW(buf,32)=sd.head_dir;
	WBUFL(buf,34)=sd.status.guild_id;
	WBUFL(buf,38)=sd.guild_emblem_id;
	WBUFW(buf,42)=sd.status.manner;
	WBUFB(buf,44)=sd.status.karma;
	WBUFB(buf,45)=sd.status.sex;
	WBUFPOS(buf,46,sd.block_list::x,sd.block_list::y, sd.dir);
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFB(buf,51)= sd.state.dead_sit;
	WBUFW(buf,52)=(sd.status.base_level>config.max_lv)?config.max_lv:sd.status.base_level;

	return packet_len_table[0x78];
#else
	memset(buf, 0, packet_len_table[0x1d8]);
	WBUFW(buf,0)=0x1d8;
	WBUFL(buf,2) = sd.block_list::id;
	WBUFW(buf,6) = sd.get_speed();
	WBUFW(buf,8) = sd.opt1;
	WBUFW(buf,10) = sd.opt2;
	if(sd.disguise_id)
	{
		WBUFW(buf,12)=OPTION_HIDE;
	}
	else
	{
		WBUFW(buf,12)=sd.status.option;
	}
	WBUFW(buf,14)=sd.view_class;
	WBUFW(buf,16)=sd.status.hair;
	if (sd.equip_index[9] < MAX_INVENTORY && sd.inventory_data[sd.equip_index[9]] && sd.view_class != 22) {
		if (sd.inventory_data[sd.equip_index[9]]->view_id > 0)
			WBUFW(buf,18) = sd.inventory_data[sd.equip_index[9]]->view_id;
		else
			WBUFW(buf,18) = sd.status.inventory[sd.equip_index[9]].nameid;
	} else
		WBUFW(buf,18) = 0;
	if (sd.equip_index[8] < MAX_INVENTORY && sd.equip_index[8] != sd.equip_index[9] && sd.inventory_data[sd.equip_index[8]] && sd.view_class != 22) {
		if (sd.inventory_data[sd.equip_index[8]]->view_id > 0)
			WBUFW(buf,20) = sd.inventory_data[sd.equip_index[8]]->view_id;
		else
			WBUFW(buf,20) = sd.status.inventory[sd.equip_index[8]].nameid;
	} else
		WBUFW(buf,20) = 0;
	WBUFW(buf,22) = sd.status.head_bottom;
	WBUFW(buf,24) = sd.status.head_top;
	WBUFW(buf,26) = sd.status.head_mid;
	WBUFW(buf,28) = sd.status.hair_color;
	WBUFW(buf,30) = sd.status.clothes_color;
	WBUFW(buf,32) = sd.head_dir;
	WBUFL(buf,34) = sd.status.guild_id;
	WBUFW(buf,38) = sd.guild_emblem_id;
	WBUFW(buf,40) = sd.status.manner;
	WBUFW(buf,42) = sd.opt3;
	WBUFB(buf,44) = sd.status.karma;
	WBUFB(buf,45) = sd.status.sex;
	WBUFPOS(buf, 46, sd.block_list::x, sd.block_list::y, sd.dir);
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFB(buf,51) = sd.state.dead_sit;
	WBUFW(buf,52)=(sd.status.base_level>config.max_base_level)?config.max_base_level:sd.status.base_level;

	return packet_len_table[0x1d8];
#endif
}

// non-moving function for disguises [Valaris]
size_t clif_dis0078(struct map_session_data &sd, unsigned char *buf)
{
	memset(buf,0,packet_len_table[0x78]);
	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=sd.block_list::id|FLAG_DISGUISE;
	WBUFW(buf,6)=sd.get_speed();
	WBUFW(buf,8)=0;
	WBUFW(buf,10)=0;
	WBUFW(buf,12)=sd.status.option;
	WBUFW(buf,14)=sd.disguise_id;
	//WBUFL(buf,34)=sd->status.guild_id;
	//WBUFL(buf,38)=sd->guild_emblem_id;
	WBUFW(buf,42)=0;
	WBUFB(buf,44)=0;
	WBUFPOS(buf,46,sd.block_list::x,sd.block_list::y, sd.dir);
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFB(buf,51)=sd.state.dead_sit;
	WBUFW(buf,52)=0;

	return (packet_len_table[0x78]>0)?packet_len_table[0x78]:0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set007b(const map_session_data &sd,unsigned char *buf)
{

#if PACKETVER < 4
	memset(buf, 0, packet_len_table[0x7b]);
	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.get_speed();
	WBUFW(buf,8)=sd.opt1;
	WBUFW(buf,10)=sd.opt2;
	if(sd.disguise_id) {
		WBUFW(buf,12)=OPTION_HIDE;
	} else {
		WBUFW(buf,12)=sd.status.option;
	}
	WBUFW(buf,14)=sd.view_class;
	WBUFW(buf,16)=sd.status.hair;
	if(sd.view_class != 22)
		WBUFW(buf,18)=sd.status.weapon;
	else
		WBUFW(buf,18)=0;
	WBUFW(buf,20)=sd.status.head_bottom;
	WBUFL(buf,22)=gettick();
	WBUFW(buf,26)=sd.status.shield;
	WBUFW(buf,28)=sd.status.head_top;
	WBUFW(buf,30)=sd.status.head_mid;
	WBUFW(buf,32)=sd.status.hair_color;
	WBUFW(buf,34)=sd.status.clothes_color;
	WBUFW(buf,36)=sd.head_dir;
	WBUFL(buf,38)=sd.status.guild_id;
	WBUFL(buf,42)=sd.guild_emblem_id;
	WBUFW(buf,46)=sd.status.manner;
	WBUFB(buf,48)=sd.status.karma;
	WBUFB(buf,49)=sd.sex;
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.target.x,sd.target.y);
	WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd.status.base_level>config.max_lv)?config.max_lv:sd.status.base_level;

	return packet_len_table[0x7b];
#else
	memset(buf, 0, packet_len_table[0x1da]);
	WBUFW(buf,0)=0x1da;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.get_speed();
	WBUFW(buf,8)=sd.opt1;
	WBUFW(buf,10)=sd.opt2;
	if(sd.disguise_id) {
		WBUFW(buf,12)=OPTION_HIDE;
	} else {
		WBUFW(buf,12)=sd.status.option;
	}
	WBUFW(buf,14)=sd.view_class;
	WBUFW(buf,16)=sd.status.hair;
	if(sd.equip_index[9] < MAX_INVENTORY && sd.inventory_data[sd.equip_index[9]] && sd.view_class != 22) {
		if(sd.inventory_data[sd.equip_index[9]]->view_id > 0)
			WBUFW(buf,18)=sd.inventory_data[sd.equip_index[9]]->view_id;
		else
			WBUFW(buf,18)=sd.status.inventory[sd.equip_index[9]].nameid;
	}
	else
		WBUFW(buf,18)=0;
	if(sd.equip_index[8] < MAX_INVENTORY && sd.equip_index[8] != sd.equip_index[9] && sd.inventory_data[sd.equip_index[8]] && sd.view_class != 22) {
		if(sd.inventory_data[sd.equip_index[8]]->view_id > 0)
			WBUFW(buf,20)=sd.inventory_data[sd.equip_index[8]]->view_id;
		else
			WBUFW(buf,20)=sd.status.inventory[sd.equip_index[8]].nameid;
	}
	else
		WBUFW(buf,20)=0;
	WBUFW(buf,22)=sd.status.head_bottom;
	WBUFL(buf,24)=gettick();
	WBUFW(buf,28)=sd.status.head_top;
	WBUFW(buf,30)=sd.status.head_mid;
	WBUFW(buf,32)=sd.status.hair_color;
	WBUFW(buf,34)=sd.status.clothes_color;
	WBUFW(buf,36)=sd.head_dir;
	WBUFL(buf,38)=sd.status.guild_id;
	WBUFW(buf,42)=sd.guild_emblem_id;
	WBUFW(buf,44)=sd.status.manner;
	WBUFW(buf,46)=sd.opt3;
	WBUFB(buf,48)=sd.status.karma;
	WBUFB(buf,49)=sd.status.sex;
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y);
	WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd.status.base_level>config.max_base_level)?config.max_base_level:sd.status.base_level;

	return packet_len_table[0x1da];
#endif
}

// moving function for disguises [Valaris]
int clif_dis007b(const map_session_data &sd,unsigned char *buf)
{
	memset(buf,0,packet_len_table[0x7b]);
	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=sd.block_list::id|FLAG_DISGUISE;
	WBUFW(buf,6)=sd.get_speed();
	WBUFW(buf,8)=0;
	WBUFW(buf,10)=0;
	WBUFW(buf,12)=sd.status.option;
	WBUFW(buf,14)=sd.disguise_id;
	WBUFL(buf,22)=gettick();
	//WBUFL(buf,38)=sd.status.guild_id;
	//WBUFL(buf,42)=sd.guild_emblem_id;
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y);
	WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=0;

	return packet_len_table[0x7b];
}

/*==========================================
 * クラスチェンジ typeはMobの場合は1で他は0？
 *------------------------------------------
 */
int clif_class_change(const block_list &bl,unsigned short class_,unsigned char type)
{
	unsigned char buf[16];
	if(class_ >= MAX_PC_CLASS)
	{
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=bl.id;
		WBUFB(buf,6)=type;
		WBUFL(buf,7)=class_;
		clif_send(buf,packet_len_table[0x1b0],&bl,AREA);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_mob_class_change(const mob_data &md)
{
	unsigned char buf[16];
	int view = md.get_viewclass();
	if(view >= MAX_PC_CLASS)
	{
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=md.block_list::id;
		WBUFB(buf,6)=1;
		WBUFL(buf,7)=view;

		clif_send(buf,packet_len_table[0x1b0],&md,AREA);
	}
	return 0;
}
// mob equipment [Valaris]

int clif_mob_equip(const mob_data &md, unsigned short nameid)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=3;
	WBUFL(buf,3)=md.block_list::id;
	WBUFL(buf,7)=nameid;

	clif_send(buf,packet_len_table[0x1a4],&md,AREA);

	return 0;
}

/*==========================================
 * MOB表示1
 *------------------------------------------
 */
int clif_mob0078(const mob_data &md, unsigned char *buf)
{
	unsigned short level=status_get_lv(&md);
	int id = md.get_viewclass();

	if( id <= 23 || id >= 4001)
	{	// Use 0x1d8 packet for monsters with player sprites [Valaris]
		memset(buf,0,packet_len_table[0x1d8]);

		WBUFW(buf,0)=0x1d8;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,14)=md.get_viewclass();
		WBUFW(buf,16)=md.get_hair();
		WBUFW(buf,18)=md.get_weapon();
		WBUFW(buf,20)=md.get_shield();
		WBUFW(buf,22)=md.get_head_buttom();
		WBUFW(buf,24)=md.get_head_top();
		WBUFW(buf,26)=md.get_head_mid();
		WBUFW(buf,28)=md.get_hair_color();
		WBUFW(buf,30)=md.get_clothes_color();
		WBUFW(buf,32)=md.dir; // head direction
		WBUFL(buf,34)=0; // guild id
		WBUFW(buf,38)=0; // emblem id
		WBUFW(buf,40)=0; // manner
		WBUFW(buf,42)=md.opt3;
		WBUFB(buf,44)=0; // karma
		WBUFB(buf,45)=md.get_sex();
		WBUFPOS(buf,46,md.block_list::x,md.block_list::y,md.dir);
		WBUFB(buf,49)=5;
		WBUFB(buf,50)=5;
		WBUFB(buf,51)=0; // dead or sit state
		WBUFW(buf,52)=(level>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x1d8];
	}
	else
	{	// Use 0x78 packet for monsters sprites [Valaris]
		memset(buf,0,packet_len_table[0x78]);

		WBUFW(buf,0)=0x78;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,14)=md.get_viewclass();

		if (md.class_ >= 1285 && md.class_ <= 1287 && md.guild_id) {	// Added guardian emblems [Valaris]
			struct guild *g;
			struct guild_castle *gc=guild_mapname2gc(maps[md.block_list::m].mapname);
			if (gc && gc->guild_id > 0) {
				g=guild_search(gc->guild_id);
				if (g) {
					WBUFL(buf,34)=g->emblem_id;
					WBUFL(buf,38)=gc->guild_id;
				}
			}
		}	// End addition

		WBUFPOS(buf,46,md.block_list::x,md.block_list::y,md.dir);
		WBUFB(buf,49)=5;
		WBUFB(buf,50)=5;
		level = status_get_lv(&md);
		WBUFW(buf,52)=((level = status_get_lv(&md))>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x78];
	}
}



/*==========================================
 * MOB表示2
 *------------------------------------------
 */
int clif_mob007b(const mob_data &md, unsigned char *buf)
{
	unsigned short level = status_get_lv(&md);
	int id = md.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1da packet for monsters with player sprites [Valaris]
		memset(buf,0,packet_len_table[0x1da]);

		WBUFW(buf,0)=0x1da;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,14)=md.get_viewclass();
		WBUFW(buf,16)=md.get_hair();
		WBUFW(buf,18)=md.get_weapon();
		WBUFW(buf,20)=md.get_shield();
		WBUFW(buf,22)=md.get_head_buttom();
		WBUFL(buf,24)=gettick();
		WBUFW(buf,28)=md.get_head_top();
		WBUFW(buf,30)=md.get_head_mid();
		WBUFW(buf,32)=md.get_hair_color();
		WBUFW(buf,34)=md.get_clothes_color();
		WBUFW(buf,36)=md.dir&0x0f; // head direction
		WBUFL(buf,38)=0; // guild id
		WBUFW(buf,42)=0; // emblem id
		WBUFW(buf,44)=0; // manner
		WBUFW(buf,46)=md.opt3;
		WBUFB(buf,48)=0; // karma
		WBUFB(buf,49)=md.get_sex();
		WBUFPOS2(buf,50,md.block_list::x,md.block_list::y,md.walktarget.x,md.walktarget.y);
		WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x1da];
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet_len_table[0x7b]);
	
		WBUFW(buf,0)=0x7b;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,14)=md.get_viewclass();
		WBUFL(buf,22)=gettick();

		if(md.class_ >= 1285 && md.class_ <= 1287 && md.guild_id)
		{	// Added guardian emblems [Valaris]
			struct guild *g;
			struct guild_castle *gc=guild_mapname2gc(maps[md.block_list::m].mapname);
			if(gc && gc->guild_id > 0)
			{
				g=guild_search(gc->guild_id);
				if(g)
				{
					WBUFL(buf,38)=gc->guild_id;
					WBUFL(buf,42)=g->emblem_id;
				}
			}
		} // End addition

		WBUFPOS2(buf,50,md.block_list::x,md.block_list::y,md.walktarget.x,md.walktarget.y);
		WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		level = status_get_lv(&md);
		WBUFW(buf,58)=((level = status_get_lv(&md))>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x7b];
	}
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_npc0078(const npc_data &nd, unsigned char *buf)
{
	struct guild *g;

	memset(buf,0,packet_len_table[0x78]);

	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,14)=nd.class_;
	if ((nd.class_ == 722) && (nd.u.scr.guild_id > 0) && ((g=guild_search(nd.u.scr.guild_id)) != NULL)) {
		WBUFL(buf,34)=g->emblem_id;
		WBUFL(buf,38)=g->guild_id;
	}
	WBUFPOS(buf,46,nd.block_list::x,nd.block_list::y,nd.dir);
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;

	return packet_len_table[0x78];
}

// NPC Walking [Valaris]
int clif_npc007b(const npc_data &nd, unsigned char *buf)
{
	struct guild *g;

	memset(buf,0,packet_len_table[0x7b]);

	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,14)=nd.class_;
	if ((nd.class_ == 722) && (nd.u.scr.guild_id > 0) && ((g=guild_search(nd.u.scr.guild_id)) != NULL))
	{
		WBUFL(buf,38)=g->emblem_id;
		WBUFL(buf,42)=g->guild_id;
	}

	WBUFL(buf,22)=gettick();
	WBUFPOS2(buf,50,nd.block_list::x,nd.block_list::y,nd.walktarget.x,nd.walktarget.y);
	WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;

	return packet_len_table[0x7b];
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pet0078(const pet_data &pd, unsigned char *buf)
{
	int view;
	unsigned short level = status_get_lv(&pd);
	int id = pd.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1d8 packet for pets with player sprites [Valaris]
		memset(buf,0,packet_len_table[0x1d8]);

		WBUFW(buf,0)=0x1d8;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,8)=0; // opt1
		WBUFW(buf,10)=0; // opt2
		WBUFW(buf,12)=mob_db[pd.class_].option;
		WBUFW(buf,14)=pd.get_viewclass();


		WBUFW(buf,16)=pd.get_hair();
		WBUFW(buf,18)=pd.get_weapon();
		WBUFW(buf,20)=pd.get_shield();
		WBUFW(buf,22)=pd.get_head_buttom();
		WBUFW(buf,24)=pd.get_head_top();
		WBUFW(buf,26)=pd.get_head_mid();
		WBUFW(buf,28)=pd.get_hair_color();
		WBUFW(buf,30)=pd.get_clothes_color();
		WBUFW(buf,32)=pd.dir; // head direction
		WBUFL(buf,34)=0; // guild id
		WBUFW(buf,38)=0; // emblem id
		WBUFW(buf,40)=0; // manner
		WBUFW(buf,42)=0; // opt3
		WBUFB(buf,44)=0; // karma
		WBUFB(buf,45)=pd.get_sex();
		WBUFPOS(buf,46,pd.block_list::x,pd.block_list::y,pd.dir);
		WBUFB(buf,49)=5;
		WBUFB(buf,50)=5;
		WBUFB(buf,51)=0; // dead or sit state
		WBUFW(buf,52)=((level = status_get_lv(&pd))>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x1d8];
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet_len_table[0x78]);

		WBUFW(buf,0)=0x78;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,14)=pd.get_viewclass();
		WBUFW(buf,16)=config.pet_hair_style;
		if((view = itemdb_viewid(pd.equip_id)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd.equip_id;

		WBUFPOS(buf,46,pd.block_list::x,pd.block_list::y,pd.dir);
		WBUFB(buf,49)=0;
		WBUFB(buf,50)=0;

		WBUFW(buf,52)=((level = status_get_lv(&pd))>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x78];
	}
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_pet007b(const pet_data &pd, unsigned char *buf)
{
	int view;
	unsigned short level = status_get_lv(&pd);
	int id = pd.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1da packet for monsters with player sprites [Valaris]
		memset(buf,0,packet_len_table[0x1da]);

		WBUFW(buf,0)=0x1da;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,8)=0; // opt1
		WBUFW(buf,10)=0; // opt2
		WBUFW(buf,12)=mob_db[pd.class_].option;
		WBUFW(buf,14)=pd.get_viewclass();


		WBUFW(buf,16)=pd.get_hair();
		WBUFW(buf,18)=pd.get_weapon();
		WBUFW(buf,20)=pd.get_shield();
		WBUFW(buf,22)=pd.get_head_buttom();
		WBUFL(buf,24)=gettick();
		WBUFW(buf,28)=pd.get_head_top();
		WBUFW(buf,30)=pd.get_head_mid();
		WBUFW(buf,32)=pd.get_hair_color();
		WBUFW(buf,34)=pd.get_clothes_color();
		WBUFW(buf,36)=pd.dir; // head direction
		WBUFL(buf,38)=0; // guild id
		WBUFW(buf,42)=0; // emblem id
		WBUFW(buf,44)=0; // manner
		WBUFW(buf,46)=0; // opt3
		WBUFB(buf,48)=0; // karma
		WBUFB(buf,49)=pd.get_sex();
		WBUFPOS2(buf,50,pd.block_list::x,pd.block_list::y,pd.walktarget.x,pd.walktarget.y);
		WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
		WBUFB(buf,56)=0;
		WBUFB(buf,57)=0;
		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x1da];
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet_len_table[0x7b]);

		WBUFW(buf,0)=0x7b;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,14)=pd.get_viewclass();
		WBUFW(buf,16)=config.pet_hair_style;
		if ((view = itemdb_viewid(pd.equip_id)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd.equip_id;
		WBUFL(buf,22)=gettick();

		WBUFPOS2(buf,50,pd.block_list::x,pd.block_list::y,pd.walktarget.x,pd.walktarget.y);
		WBUFB(buf,55)=0x88; // Deals with acceleration in directions. [Valaris]
		WBUFB(buf,56)=0;
		WBUFB(buf,57)=0;

		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet_len_table[0x7b];
	}
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_hom0078(const homun_data &hd, unsigned char *buf)
{
	memset(buf, 0, packet_len_table[0x78]);

	WBUFW(buf,0) =0x78;
	WBUFL(buf,2) =hd.block_list::id;
	WBUFW(buf,6) =hd.get_speed();
	if(hd.view_class)
		WBUFW(buf,14)=hd.view_class;
	else
		WBUFW(buf,14)=hd.status.class_;

	WBUFW(buf,16)=24;//config.pet0078_hair_id;
	WBUFW(buf,20)=0;

	WBUFPOS(buf,46,hd.block_list::x,hd.block_list::y,hd.get_dir());
	WBUFB(buf,49)=0;
	WBUFB(buf,50)=0;
	WBUFW(buf,52)=status_get_lv(&hd);

	return packet_len_table[0x78];
}
int clif_hom007b(const homun_data &hd, unsigned char *buf)
{
	int view,level;

	memset(buf,0,packet_len_table[0x7b]);

	WBUFW(buf,0) =0x7b;
	WBUFL(buf,2) =hd.block_list::id;
	WBUFW(buf,6) =hd.speed;
	if(hd.view_class)
		WBUFW(buf,14)=hd.view_class;
	else
		WBUFW(buf,14)=hd.status.class_;
	WBUFW(buf,16)=24;//config.pet0078_hair_id;
	if((view = itemdb_viewid(hd.equip)) > 0)
		WBUFW(buf,20)=view;
	else
		WBUFW(buf,20)=hd.equip;
	WBUFL(buf,22)=gettick();

	WBUFPOS2(buf,50,hd.block_list::x,hd.block_list::y,hd.walktarget.x,hd.walktarget.y);
	WBUFB(buf,56)=0;
	WBUFB(buf,57)=0;
	level = status_get_lv(&hd);
	WBUFW(buf,58)=(level>99)? 99:level;

	return packet_len_table[0x7b];
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set01e1(struct map_session_data &sd, unsigned char *buf)
{
	WBUFW(buf,0)=0x1e1;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.spiritball;

	return packet_len_table[0x1e1];
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set0192(int fd, unsigned short m, unsigned short x, unsigned short y, unsigned short type)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x192;
	WFIFOW(fd,2) = x;
	WFIFOW(fd,4) = y;
	WFIFOW(fd,6) = type;
	mapname2buffer(WFIFOP(fd,8), maps[m].mapname, 16);
	WFIFOSET(fd,packet_len_table[0x192]);

	return 0;
}

// new and improved weather display [Valaris]
int clif_weather1(int fd, int type)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x1f3;
	WFIFOL(fd,2) = 0xFFFFFFF6;//-10;
	WFIFOL(fd,6) = type;
	WFIFOSET(fd,packet_len_table[0x1f3]);

	return 0;
}

int clif_weather2(unsigned short m, int type)
{
	size_t i;	
	struct map_session_data *sd=NULL;
	for(i = 0; i < fd_max; ++i)
	{
		if( session[i] && (sd = (struct map_session_data *)session[i]->user_session)!=NULL && session_isActive(sd->fd) && sd->state.auth && sd->block_list::m == m)
		{
			WFIFOW(sd->fd,0) = 0x1f3;
			WFIFOL(sd->fd,2) = 0xFFFFFFF6;//-10;
			WFIFOL(sd->fd,6) = type;
			WFIFOSET(sd->fd,packet_len_table[0x1f3]);
		}
	}

	return 0;
}

int clif_clearweather(unsigned short m)
{
	size_t i;

	struct map_session_data *sd=NULL;

	for(i = 0; i < fd_max; ++i)
	{
		if( session[i] && (sd = (struct map_session_data *)session[i]->user_session)!=NULL && session_isActive(sd->fd) && sd->state.auth && sd->block_list::m == m)
		{
			WFIFOW(sd->fd,0) = 0x80;
			WFIFOL(sd->fd,2) = 0xFFFFFFF6;//-10;
			WFIFOB(sd->fd,6) = 0;
			WFIFOSET(sd->fd,packet_len_table[0x80]);

			
			if( maps[sd->block_list::m].flag.snow || 
				maps[sd->block_list::m].flag.clouds || 
				maps[sd->block_list::m].flag.clouds2 || 
				maps[sd->block_list::m].flag.fog || 
				maps[sd->block_list::m].flag.fireworks ||
				maps[sd->block_list::m].flag.sakura || 
				maps[sd->block_list::m].flag.leaves || 
				maps[sd->block_list::m].flag.rain )
			{
				WFIFOW(sd->fd,0)=0x7c;
				WFIFOL(sd->fd,2)=0xFFFFFFF6;//-10;
				WFIFOW(sd->fd,6)=0;
				WFIFOW(sd->fd,8)=0;
				WFIFOW(sd->fd,10)=0;
				WFIFOW(sd->fd,12)=OPTION_HIDE;
				WFIFOW(sd->fd,20)=100;
				WFIFOPOS(sd->fd,36,sd->block_list::x,sd->block_list::y, sd->dir);
				WFIFOSET(sd->fd,packet_len_table[0x7c]);

				if (maps[sd->block_list::m].flag.snow)
					clif_weather1(sd->fd, EFFECT_SNOW);
				if (maps[sd->block_list::m].flag.clouds)
					clif_weather1(sd->fd, EFFECT_CLOUDS);
				if (maps[sd->block_list::m].flag.clouds2)
					clif_weather1(sd->fd, EFFECT_CLOUDS2);
				if (maps[sd->block_list::m].flag.fog)
					clif_weather1(sd->fd, EFFECT_FOG);
				if (maps[sd->block_list::m].flag.fireworks) {
					clif_weather1(sd->fd, EFFECT_FIRE1);
					clif_weather1(sd->fd, EFFECT_FIRE2);
					clif_weather1(sd->fd, EFFECT_FIRE3);
				}
				if (maps[sd->block_list::m].flag.sakura)
					clif_weather1(sd->fd, EFFECT_SAKURA);
				if (maps[sd->block_list::m].flag.leaves)
					clif_weather1(sd->fd, EFFECT_LEAVES);
				if (maps[sd->block_list::m].flag.rain)
					clif_weather1(sd->fd, EFFECT_RAIN);
			}
		}
	}
	return 0;
}

/// make it a overloaded function
bool clif_spawn(block_list& bl)
{
	if( bl.get_sd() )
		return clif_spawnpc( *bl.get_sd() );
	if( bl.get_nd() )
		return clif_spawnnpc( *bl.get_nd() );
	if( bl.get_md() )
		return clif_spawnmob( *bl.get_md() );
	if( bl.get_pd() )
		return clif_spawnpet( *bl.get_pd() );
	if( bl.get_hd() )
		return clif_spawnhom( *bl.get_hd() );
	return false;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnpc(struct map_session_data &sd)
{
	unsigned char buf[128];

	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	clif_set0078(sd, buf);

#if PACKETVER < 4
	WBUFW(buf, 0) = 0x79;
	WBUFW(buf,51) = (sd.status.base_level > config.max_lv) ? config.max_lv : sd.status.base_level;
	clif_send(buf, packet_len_table[0x79], &sd, AREA_WOS);
#else
	WBUFW(buf, 0) = 0x1d9;
	WBUFW(buf,51) = (sd.status.base_level > config.max_base_level) ? config.max_base_level : sd.status.base_level;
	clif_send(buf, packet_len_table[0x1d9], &sd, AREA_WOS);
#endif

	if(sd.disguise_id > 0)
	{
		memset(buf,0,packet_len_table[0x7c]);
		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=sd.block_list::id|FLAG_DISGUISE;
		WBUFW(buf,6)=sd.get_speed();
		WBUFW(buf,12)=sd.status.option;
		WBUFW(buf,20)=sd.disguise_id;
		WBUFPOS(buf,36,sd.block_list::x,sd.block_list::y,sd.dir);
		clif_send(buf,packet_len_table[0x7c],&sd,AREA);

		size_t len = clif_dis0078(sd,buf);
		clif_send(buf,len,&sd,AREA);
	}

	if (sd.spiritball > 0)
		clif_spiritball(sd);

	if (sd.status.guild_id > 0) { // force display of guild emblem [Valaris]
		struct guild *g = guild_search(sd.status.guild_id);
		if (g)
			clif_guild_emblem(sd,*g);
	}	// end addition [Valaris]

	if (!sd.disguise_id && (sd.status.class_==13 || sd.status.class_==21 || sd.status.class_==4014 || sd.status.class_==4022 || sd.status.class_==4036 || sd.status.class_==4044))
		pc_setoption(sd,sd.status.option|0x0020); // [Valaris]

	if ((pc_isriding(sd) && pc_checkskill(sd,KN_RIDING)>0) && 
		(sd.status.class_==7 || sd.status.class_==14 || sd.status.class_==4008 || sd.status.class_==4015 || sd.status.class_==4030 || sd.status.class_==4037))
		pc_setriding(sd); // update peco riders for people upgrading athena [Valaris]

	if( maps[sd.block_list::m].flag.snow || 
		maps[sd.block_list::m].flag.clouds || 
		maps[sd.block_list::m].flag.clouds2 || 
		maps[sd.block_list::m].flag.fog || 
		maps[sd.block_list::m].flag.fireworks ||
		maps[sd.block_list::m].flag.sakura || 
		maps[sd.block_list::m].flag.leaves || 
		maps[sd.block_list::m].flag.rain )
	{
		WFIFOW(fd,0)=0x7c;
		WFIFOL(fd,2)=0xFFFFFFF6;//-10;
		WFIFOW(fd,6)=0;
		WFIFOW(fd,8)=0;
		WFIFOW(fd,10)=0;
		WFIFOW(fd,12)=OPTION_HIDE;
		WFIFOW(fd,20)=100;
		WFIFOPOS(fd,36,sd.block_list::x,sd.block_list::y,sd.dir);
		WFIFOSET(fd,packet_len_table[0x7c]);

		if (maps[sd.block_list::m].flag.snow)
			clif_weather1(fd, EFFECT_SNOW);
		if (maps[sd.block_list::m].flag.clouds)
			clif_weather1(fd, EFFECT_CLOUDS);
		if (maps[sd.block_list::m].flag.clouds2)
			clif_weather1(fd, EFFECT_CLOUDS2);
		if (maps[sd.block_list::m].flag.fog)
			clif_weather1(fd, EFFECT_FOG);
		if (maps[sd.block_list::m].flag.fireworks) {
			clif_weather1(fd, EFFECT_FIRE1);
			clif_weather1(fd, EFFECT_FIRE2);
			clif_weather1(fd, EFFECT_FIRE3);
		}
		if (maps[sd.block_list::m].flag.sakura)
			clif_weather1(fd, EFFECT_SAKURA);
		if (maps[sd.block_list::m].flag.leaves)
			clif_weather1(fd, EFFECT_LEAVES);
		if (maps[sd.block_list::m].flag.rain)
			clif_weather1(fd, EFFECT_RAIN);
	}

	if(sd.state.viewsize==2) // tiny/big players [Valaris]
		clif_specialeffect(sd,EFFECT_BIG,0);
	else if(sd.state.viewsize==1)
		clif_specialeffect(sd,EFFECT_TINY,0);
		
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
/// spawn npc on all clients in the area
int clif_spawnnpc(struct npc_data &nd)
{
	unsigned char buf[64];
	int len;

	if(nd.class_ < 0 || nd.flag&1 || nd.class_ == INVISIBLE_CLASS)
		return 0;

	memset(buf,0,packet_len_table[0x7c]);

	WBUFW(buf,0)=0x7c;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,20)=nd.class_;
	WBUFPOS(buf,36,nd.block_list::x,nd.block_list::y,nd.dir);

	clif_send(buf,packet_len_table[0x7c],&nd,AREA);

	len = clif_npc0078(nd,buf);
	clif_send(buf,len,&nd,AREA);

	return 0;
}
/// spawn npc only on given client
int clif_spawnnpc(struct map_session_data &sd, struct npc_data &nd)
{
	unsigned char buf[64];
	int len;

	if(nd.class_ < 0 || nd.flag&1 || nd.class_ == INVISIBLE_CLASS)
		return 0;

	memset(buf,0,packet_len_table[0x7c]);

	WBUFW(buf,0)=0x7c;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,20)=nd.class_;
	WBUFPOS(buf,36,nd.block_list::x,nd.block_list::y,nd.dir);

	clif_send_self(sd,buf,packet_len_table[0x7c]);

	len = clif_npc0078(nd,buf);
	clif_send_self(sd,buf,len);

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnmob(struct mob_data &md)
{
	unsigned char buf[64];
	size_t len;
	unsigned short viewclass = md.get_viewclass();

	if (viewclass > 23 && viewclass < 4000)
	{
		memset(buf,0,packet_len_table[0x7c]);
		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,20)=viewclass;
		WBUFPOS(buf,36,md.block_list::x,md.block_list::y,md.dir);
		clif_send(buf,packet_len_table[0x7c],&md,AREA);
	}

	len = clif_mob0078(md,buf);
	clif_send(buf,len,&md,AREA);

	if( md.get_equip() > 0) // mob equipment [Valaris]
		clif_mob_equip(md,md.get_equip());

	if(md.state.size==2) // tiny/big mobs [Valaris]
		clif_specialeffect(md,EFFECT_BIG,0);
	else if(md.state.size==1)
		clif_specialeffect(md,EFFECT_TINY,0);

	return 0;
}

// pet

/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnpet(struct pet_data &pd)
{
	unsigned char buf[64];
	int len;

	if( pd.get_viewclass() >= MAX_PC_CLASS )
	{
		memset(buf,0,packet_len_table[0x7c]);

		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.get_speed();
		WBUFW(buf,20)=pd.get_viewclass();
		WBUFPOS(buf,36,pd.block_list::x,pd.block_list::y,pd.dir);

		clif_send(buf,packet_len_table[0x7c],&pd,AREA);
	}

	len = clif_pet0078(pd,buf);
	clif_send(buf,len,&pd,AREA);

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnhom(const homun_data &hd)
{
	unsigned char buf[64];

	memset(buf,0,packet_db[19][0x7c].len);
	
	WBUFW(buf,0) = 0x7c;
	WBUFL(buf,2) = hd.block_list::id;
	WBUFW(buf,6) = hd.speed;
	if( hd.view_class )
		WBUFW(buf,20) = hd.view_class;
	else
		WBUFW(buf,20) = hd.status.class_;
	WBUFB(buf,28)=8;		// 調べた限り固定
	WBUFPOS(buf,36,hd.block_list::x,hd.block_list::y, hd.get_dir());
	clif_send(buf, packet_db[19][0x7c].len, &hd, AREA);
	
//	if( hd.view_size!=0 )
//		clif_misceffect2(hd, 422+hd.view_size);
	if(hd.view_size==2)
		clif_specialeffect(hd,EFFECT_BIG,0);
	else if(hd.view_size==1)
		clif_specialeffect(hd,EFFECT_TINY,0);
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_send_homdata(const homun_data &hd, const map_session_data &sd, unsigned short type, uint32 param)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x230;
	WFIFOW(fd,2) = type;
	WFIFOL(fd,4) = hd.block_list::id;
	WFIFOL(fd,8) = param;
	WFIFOSET(fd, packet_db[sd.packet_ver][0x230].len);
	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int clif_movehom(const homun_data &hd)
{
	unsigned char buf[256];
	int len = clif_hom007b(hd,buf);
	clif_send(buf,len, &hd, AREA);
	return 0;
}

int clif_send_homstatus(const homun_data &hd, const map_session_data &sd, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	
	WFIFOW(fd,0)=0x22e;
	memcpy(WFIFOP(fd,2), hd.status.name, 24);
	WFIFOB(fd,26) = hd.status.rename_flag*2;	// 名前付けたフラグ 2で変更不可
	WFIFOW(fd,27) = hd.status.base_level;	// Lv
	WFIFOW(fd,29) = hd.status.hungry;		// 満腹度
	WFIFOW(fd,31) = hd.intimate/100;	// 新密度
	WFIFOW(fd,33) = hd.status.equip;			// equip id
	WFIFOW(fd,35) = hd.atk;					// Atk
	WFIFOW(fd,37) = hd.matk;					// MAtk
	WFIFOW(fd,39) = hd.hit;					// Hit
	WFIFOW(fd,41) = hd.critical;				// Cri
	WFIFOW(fd,43) = hd.def;					// Def
	WFIFOW(fd,45) = hd.mdef;					// Mdef
	WFIFOW(fd,47) = hd.flee;					// Flee
	WFIFOW(fd,49) =(flag)?0:status_get_amotion(&hd)+200;	// Aspd
	WFIFOW(fd,51) = hd.status.hp;			// HP
	WFIFOW(fd,53) = hd.max_hp;		// MHp
	WFIFOW(fd,55) = hd.status.sp;			// SP
	WFIFOW(fd,57) = hd.max_sp;		// MSP
	WFIFOL(fd,59) = hd.status.base_exp;		// Exp
	WFIFOL(fd,63) = hd.next_baseexp();	// NextExp
	WFIFOW(fd,67) = hd.status.skill_point;	// skill point
	WFIFOW(fd,69) = hd.atackable;			// 攻撃可否フラグ	0:不可/1:許可
	WFIFOSET(fd,packet_db[sd.packet_ver][0x22e].len);

	return 0;
}
int clif_hom_food(const map_session_data &sd, unsigned short foodid, int fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x22f;
	WFIFOB(fd,2)=fail;
	WFIFOW(fd,3)=foodid;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x22f].len);
	return 0;
}
/*==========================================
 * ホムのスキルリストを送信する
 *------------------------------------------
 */
int clif_homskillinfoblock(const homun_data &hd, const map_session_data &sd)
{
	int fd=sd.fd;
	int i,c,len=4,id,range,skill_lv;
	if( !session_isActive(fd) )
		return 0;
	
	WFIFOW(fd,0)=0x235;
	for ( i=c=0; i<MAX_HOMSKILL; ++i)
	{
		id=hd.status.skill[i].id;
		if( id && hd.status.skill[i].lv )
		{
			WFIFOW(fd,len  ) = id;
			WFIFOL(fd,len+2) = skill_get_inf(id);
			skill_lv = hd.status.skill[i].lv;
			WFIFOW(fd,len+6) = skill_lv;
			WFIFOW(fd,len+8) = skill_get_sp(id,skill_lv);
			range = skill_get_range(id,skill_lv);
			if(range < 0)
				range = status_get_range(&hd) - (range + 1);
			WFIFOW(fd,len+10)= range;
			memset(WFIFOP(fd,len+12),0,24);
			if(!(skill_get_inf2(id)&0x01))
				WFIFOB(fd,len+36)= (skill_lv < skill_get_max(id) && hd.status.skill[i].flag ==0 )? 1:0;
			else
				WFIFOB(fd,len+36) = 0;

			len+=37;
			++c;
		}
	}
	WFIFOW(fd,2)=len;
	WFIFOSET(fd,len);
	return 0;
}
/*==========================================
 * スキル割り振り通知
 *------------------------------------------
 */
int clif_homskillup(const homun_data &hd, const map_session_data &sd, unsigned short skill_num)
{
	int range, fd=sd.fd;
	unsigned short skillid = skill_num-HOM_SKILLID;
	if( !session_isActive(fd) )
		return 0;
	
	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skill_num;
	WFIFOW(fd,4) = hd.status.skill[skillid].lv;
	WFIFOW(fd,6) = skill_get_sp(skill_num, hd.status.skill[skillid].lv);
	range = skill_get_range(skill_num,hd.status.skill[skillid].lv);
	if(range < 0)
		range = status_get_range(&hd) - (range + 1);
	WFIFOW(fd,8) = range;
	WFIFOB(fd,10) = (hd.status.skill[skillid].lv < skill_get_max(hd.status.skill[skillid].id)) ? 1 : 0;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x10e].len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_servertick(struct map_session_data &sd, unsigned long tick)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x7f;
	WFIFOL(fd,2)=tick;
	WFIFOSET(fd,packet_len_table[0x7f]);

	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int clif_walkok(const map_session_data& sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x87;
	WFIFOL(fd,2)=gettick();
	WFIFOPOS2(fd,6,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y);
	WFIFOB(fd,11)=0x88;
	WFIFOSET(fd,packet_len_table[0x87]);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_moveobject(const block_list& bl)
{
	if( bl.get_sd() )
	{
		const map_session_data& sd = *bl.get_sd();
		size_t len;
		unsigned char buf[256];
		unsigned char buf2[256];
		unsigned char buf3[256];

		clif_walkok(sd);

		len = clif_set007b(sd, buf);
		clif_send(buf, len, &sd, AREA_WOS);


		if( maps[sd.block_list::m].flag.snow || 
			maps[sd.block_list::m].flag.clouds || 
			maps[sd.block_list::m].flag.clouds2 || 
			maps[sd.block_list::m].flag.fog || 
			maps[sd.block_list::m].flag.fireworks ||
			maps[sd.block_list::m].flag.sakura || 
			maps[sd.block_list::m].flag.leaves || 
			maps[sd.block_list::m].flag.rain )
		{
			memset(buf2,0,packet_len_table[0x7b]);
			WBUFW(buf2,0)=0x7b;
			WBUFL(buf2,2)=0xFFFFFFF6;//-10;
			WBUFW(buf2,6)=sd.get_speed();
			WBUFW(buf2,8)=0;
			WBUFW(buf2,10)=0;
			WBUFW(buf2,12)=OPTION_HIDE;
			WBUFW(buf2,14)=100;
			WBUFL(buf2,22)=gettick();
			WBUFPOS2(buf2,50,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y);
			WBUFB(buf2,56)=5;
			WBUFB(buf2,57)=5;
			clif_send_self(sd, buf2, len);
		}

		if(sd.disguise_id)
		{
			len = clif_dis007b(sd, buf3);
			clif_send(buf3, len, &sd, AREA);
			return 0;
		}

		//Stupid client that needs this resent every time someone walks :X
		if(config.save_clothcolor &&
			sd.status.clothes_color > 0 &&
			(sd.view_class != 22 || !config.wedding_ignorepalette) )
			clif_changelook(sd, LOOK_CLOTHES_COLOR, sd.status.clothes_color);

		if(sd.state.viewsize==2) // tiny/big players [Valaris]
			clif_specialeffect(sd,EFFECT_BIG,0);
		else if(sd.state.viewsize==1)
			clif_specialeffect(sd,EFFECT_TINY,0);
	}
	else if( bl.get_md() )
	{
		const mob_data &md = *bl.get_md();
		unsigned char buf[256];
		int len;

		len = clif_mob007b(md,buf);
		clif_send(buf,len,&md,AREA);

		if(md.get_equip() > 0) // mob equipment [Valaris]
			clif_mob_equip(md,md.get_equip());

		if(md.state.size==2) // tiny/big mobs [Valaris]
			clif_specialeffect(md,EFFECT_BIG,0);
		else if(md.state.size==1)
			clif_specialeffect(md,EFFECT_TINY,0);
	}
	else if( bl.get_pd() )
	{
		const pet_data &pd = *bl.get_pd();
		unsigned char buf[256];
		int len;

		len = clif_pet007b(pd,buf);
		clif_send(buf,len,&pd,AREA);
	}
	else if( bl.get_nd() )
	{
		const npc_data &nd = *bl.get_nd();
		unsigned char buf[256];

		int len = clif_npc007b(nd,buf);
		clif_send(buf,len,&nd,AREA);
	}
	else if( bl.get_hd() )
	{
		const homun_data &hd = *bl.get_hd();
		unsigned char buf[256];

		int len = clif_hom007b(hd,buf);
		clif_send(buf,len,&hd,AREA);
	}
	return 0;
}





/*==========================================
 *
 *------------------------------------------
 */
int clif_changemap(struct map_session_data &sd, const char *mapname, unsigned short x, unsigned short y)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
	return 0;

	WFIFOW(fd,0) = 0x91;
	mapname2buffer(WFIFOP(fd,2), mapname, 16);
	WFIFOW(fd,18) = x;
	WFIFOW(fd,20) = y;
	WFIFOSET(fd, packet_len_table[0x91]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapserver(struct map_session_data &sd, const char *mapname, unsigned short x, unsigned short y, basics::ipaddress ip, unsigned short port)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x92;
	mapname2buffer(WFIFOP(fd,2), mapname, 16);
	WFIFOW(fd,18) = x;
	WFIFOW(fd,20) = y;
	WFIFOLIP(fd,22) = ip;
	WFIFOW(fd,26) = port;
	WFIFOSET(fd, packet_len_table[0x92]);

	session_SetWaitClose(fd, 1000);
	return 0;
}



/*==========================================
 *
 *------------------------------------------
 */
int clif_npcbuysell(struct map_session_data& sd, uint32 id)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xc4;
	WFIFOL(fd,2)=id;
	WFIFOSET(fd,packet_len_table[0xc4]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_buylist(struct map_session_data &sd, struct npc_data &nd)
{
	struct item_data *id;
	int i,val;

	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xc6;
	for(i=0;nd.u.shop_item[i].nameid > 0; ++i){
		id = itemdb_search(nd.u.shop_item[i].nameid);
		val=nd.u.shop_item[i].value;
		WFIFOL(fd,4+i*11)=val;
		if (!id->flag.value_notdc)
			val=pc_modifybuyvalue(sd,val);
		WFIFOL(fd,8+i*11)=val;
		WFIFOB(fd,12+i*11)=id->getType();
		if (id->view_id > 0)
			WFIFOW(fd,13+i*11)=id->view_id;
		else
			WFIFOW(fd,13+i*11)=nd.u.shop_item[i].nameid;
	}
	WFIFOW(fd,2)=i*11+4;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_selllist(struct map_session_data &sd)
{
	int fd,i,c=0,val;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xc7;
	for(i=0;i<MAX_INVENTORY; ++i)
	{
		if(sd.status.inventory[i].nameid > 0 && sd.inventory_data[i])
		{
			if (!itemdb_cansell(sd.status.inventory[i].nameid, sd.isGM()))
				continue;

			val=sd.inventory_data[i]->value_sell;
			if(val >= 0)
			{
			WFIFOW(fd,4+c*10)=i+2;
			WFIFOL(fd,6+c*10)=val;
				if (!sd.inventory_data[i]->flag.value_notoc)
				val=pc_modifysellvalue(sd,val);
			WFIFOL(fd,10+c*10)=val;
			c++;
		}
	}
	}
	WFIFOW(fd,2)=c*10+4;
	WFIFOSET(fd,c*10+4);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptmes(struct map_session_data &sd, uint32 npcid, const char *mes)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	size_t len = (mes) ? 1+strlen(mes) : 0;
	WFIFOW(fd,0)=0xb4;
	WFIFOW(fd,2)=len+8;
	WFIFOL(fd,4)=npcid;
	memcpy(WFIFOP(fd,8),mes,len);
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptnext(struct map_session_data &sd,uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xb5;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len_table[0xb5]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptclose(struct map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xb6;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len_table[0xb6]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptmenu(struct map_session_data &sd, uint32 npcid, const char *mes)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	size_t len = (mes) ? 1+strlen(mes) : 0;

	WFIFOW(fd,0)=0xb7;
	WFIFOW(fd,2)=len+8;
	WFIFOL(fd,4)=npcid;
	memcpy(WFIFOP(fd,8),mes,len);
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptinput(struct map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x142;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len_table[0x142]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptinputstr(struct map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1d4;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_len_table[0x1d4]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_viewpoint(struct map_session_data &sd, uint32 npc_id, uint32 id, uint32 x, uint32 y, unsigned char type, uint32 color)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x144;
	WFIFOL(fd,2)=npc_id;
	WFIFOL(fd,6)=id;
	WFIFOL(fd,10)=x;
	WFIFOL(fd,14)=y;
	WFIFOB(fd,18)=type;
	WFIFOL(fd,19)=color;
	WFIFOSET(fd,packet_len_table[0x144]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_cutin(struct map_session_data &sd, const char *image, unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(image)
	{	
		WFIFOW(fd,0)=0x1b3;
		safestrcpy((char*)WFIFOP(fd,2),image,64);
		WFIFOB(fd,66)=type;
		WFIFOSET(fd,packet_len_table[0x1b3]);
	}

	return 0;
}
/*==========================================
 * Fills in card data from the given item and into the buffer. [Skotlex]
 *------------------------------------------
 */
void clif_cards_to_buffer(unsigned char* buf, struct item& item)
{
	unsigned j;
	if(item.card[0]==0xff00)
	{	//pet eggs
		WBUFW(buf,0)=0;
		WBUFW(buf,2)=0;
		WBUFW(buf,4)=0;
		WBUFW(buf,6)=item.card[3]; //Pet renamed flag.
	}
	else if(item.card[0]==0x00ff || item.card[0]==0x00fe)
	{	//Forged/created items
		WBUFW(buf,0)=item.card[0];
		WBUFW(buf,2)=item.card[1];
		WBUFW(buf,4)=item.card[2];
		WBUFW(buf,6)=item.card[3];
	}
	else
	{	//Normal items.
		if (item.card[0] > 0 && (j=itemdb_viewid(item.card[0])) > 0)
			WBUFW(buf,0)=j;
		else
			WBUFW(buf,0)= item.card[0];

		if (item.card[1] > 0 && (j=itemdb_viewid(item.card[1])) > 0)
			WBUFW(buf,2)=j;
		else
			WBUFW(buf,2)=item.card[1];

		if (item.card[2] > 0 && (j=itemdb_viewid(item.card[2])) > 0)
			WBUFW(buf,4)=j;
		else
			WBUFW(buf,4)=item.card[2];

		if (item.card[3] > 0 && (j=itemdb_viewid(item.card[3])) > 0)
			WBUFW(buf,6)=j;
		else
			WBUFW(buf,6)=item.card[3];
	}
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_additem(struct map_session_data &sd, unsigned short n, unsigned short amount, unsigned char fail)
{
	int fd;
	unsigned char *buf;

	fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
	if(fail)
	{
		WBUFW(buf,0)=0xa0;
		WBUFW(buf,2)=n+2;
		WBUFW(buf,4)=amount;
		WBUFW(buf,6)=0;
		WBUFB(buf,8)=0;
		WBUFB(buf,9)=0;
		WBUFB(buf,10)=0;
		WBUFW(buf,11)=0;
		WBUFW(buf,13)=0;
		WBUFW(buf,15)=0;
		WBUFW(buf,17)=0;
		WBUFW(buf,19)=0;
		WBUFB(buf,21)=0;
		WBUFB(buf,22)=fail;
	}
	else
	{
		if( n>=MAX_INVENTORY || sd.status.inventory[n].nameid <=0 || sd.inventory_data[n] == NULL)
			return 1;

		WBUFW(buf,0)=0xa0;
		WBUFW(buf,2)=n+2;
		WBUFW(buf,4)=amount;
		if (sd.inventory_data[n]->view_id > 0)
			WBUFW(buf,6)=sd.inventory_data[n]->view_id;
		else
			WBUFW(buf,6)=sd.status.inventory[n].nameid;
		WBUFB(buf,8)=sd.status.inventory[n].identify;
		WBUFB(buf,9)=sd.status.inventory[n].attribute;
		WBUFB(buf,10)=sd.status.inventory[n].refine;
		clif_cards_to_buffer( WBUFP(buf,11), sd.status.inventory[n]);

		WBUFW(buf,19)=pc_equippoint(sd,n);
		WBUFB(buf,21)=sd.inventory_data[n]->getType();
		WBUFB(buf,22)=fail;
	}

	WFIFOSET(fd,packet_len_table[0xa0]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_delitem(struct map_session_data &sd,unsigned short n,unsigned short amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xaf;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=amount;

	WFIFOSET(fd,packet_len_table[0xaf]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_itemlist(struct map_session_data &sd)
{
	int i,n,fd,arrow=-1;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0xa3;
	for(i=0,n=0;i<MAX_INVENTORY; ++i){
		if (sd.status.inventory[i].nameid <=0 || sd.inventory_data[i] == NULL || itemdb_isSingleStorage(*sd.inventory_data[i]))
			continue;
		WBUFW(buf,n*10+4)=i+2;
		if (sd.inventory_data[i]->view_id > 0)
			WBUFW(buf,n*10+6)=sd.inventory_data[i]->view_id;
		else
			WBUFW(buf,n*10+6)=sd.status.inventory[i].nameid;
		WBUFB(buf,n*10+8)=sd.inventory_data[i]->getType();
		WBUFB(buf,n*10+9)=sd.status.inventory[i].identify;
		WBUFW(buf,n*10+10)=sd.status.inventory[i].amount;
		if (sd.inventory_data[i]->equip == 0x8000) {
			WBUFW(buf,n*10+12)=0x8000;
			if (sd.status.inventory[i].equip)
				arrow=i;	// ついでに矢装備チェック
		} else
			WBUFW(buf,n*10+12)=0;
		n++;
	}
	if (n) {
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1ee;
	for(i=0,n=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].nameid <=0 || sd.inventory_data[i] == NULL || itemdb_isSingleStorage(*sd.inventory_data[i]))
			continue;
		WBUFW(buf,n*18+4)=i+2;
		if(sd.inventory_data[i]->view_id > 0)
			WBUFW(buf,n*18+6)=sd.inventory_data[i]->view_id;
		else
			WBUFW(buf,n*18+6)=sd.status.inventory[i].nameid;
		WBUFB(buf,n*18+8)=sd.inventory_data[i]->getType();
		WBUFB(buf,n*18+9)=sd.status.inventory[i].identify;
		WBUFW(buf,n*18+10)=sd.status.inventory[i].amount;
		if (sd.inventory_data[i]->equip == 0x8000) {
			WBUFW(buf,n*18+12)=0x8000;
			if(sd.status.inventory[i].equip)
				arrow=i;	// ついでに矢装備チェック
		} else
			WBUFW(buf,n*18+12)=0;
		clif_cards_to_buffer( WBUFP(buf,n*18+14), sd.status.inventory[i]);
		n++;
	}
	if (n) {
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	if(arrow >= 0)
		clif_arrowequip(sd,arrow);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_equiplist(struct map_session_data &sd)
{
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0xa4;
	for(i=0,n=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].nameid<=0 || sd.inventory_data[i] == NULL || !itemdb_isSingleStorage(*sd.inventory_data[i]))
			continue;
		WBUFW(buf,n*20+4)=i+2;
		if(sd.inventory_data[i]->view_id > 0)
			WBUFW(buf,n*20+6)=sd.inventory_data[i]->view_id;
		else
			WBUFW(buf,n*20+6)=sd.status.inventory[i].nameid;
		WBUFB(buf,n*20+8)=sd.inventory_data[i]->getType();
		WBUFB(buf,n*20+9)=sd.status.inventory[i].identify;
		WBUFW(buf,n*20+10)=pc_equippoint(sd,i);
		WBUFW(buf,n*20+12)=sd.status.inventory[i].equip;
		WBUFB(buf,n*20+14)=sd.status.inventory[i].attribute;
		WBUFB(buf,n*20+15)=sd.status.inventory[i].refine;
		clif_cards_to_buffer( WBUFP(buf,n*20+16), sd.status.inventory[i]);
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * カプラさんに預けてある消耗品&収集品リスト
 *------------------------------------------
 */
int clif_storageitemlist(struct map_session_data &sd,struct pc_storage &stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0xa5;
	for(i=0,n=0;i<MAX_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if( !id || itemdb_isSingleStorage(*id) )
			continue;

		WBUFW(buf,n*10+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=stor.storage[i].nameid;
		WBUFB(buf,n*10+8)=id->getType();
		WBUFB(buf,n*10+9)=stor.storage[i].identify;
		WBUFW(buf,n*10+10)=stor.storage[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1f0;
	for(i=0,n=0;i<MAX_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if(!id || itemdb_isSingleStorage(*id))
			continue;

		WBUFW(buf,n*18+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=stor.storage[i].nameid;
		WBUFB(buf,n*18+8)=id->getType();
		WBUFB(buf,n*18+9)=stor.storage[i].identify;
		WBUFW(buf,n*18+10)=stor.storage[i].amount;
		WBUFW(buf,n*18+12)=0;
		clif_cards_to_buffer( WBUFP(buf,n*18+14), stor.storage[i]);
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 * カプラさんに預けてある装備リスト
 *------------------------------------------
 */
int clif_storageequiplist(struct map_session_data &sd,struct pc_storage &stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0xa6;
	for(i=0,n=0;i<MAX_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if( !id || !itemdb_isSingleStorage(*id) )
			continue;
		WBUFW(buf,n*20+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=stor.storage[i].nameid;
		WBUFB(buf,n*20+8)=id->getType();
		WBUFB(buf,n*20+9)=stor.storage[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=stor.storage[i].equip;
		WBUFB(buf,n*20+14)=stor.storage[i].attribute;
		WBUFB(buf,n*20+15)=stor.storage[i].refine;
		clif_cards_to_buffer( WBUFP(buf,n*20+16), stor.storage[i]);
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageitemlist(struct map_session_data &sd,struct guild_storage &stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf=WFIFOP(fd,0);

#if PACKETVER < 5
	WBUFW(buf,0)=0xa5;
	for(i=0,n=0;i<MAX_GUILD_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if(!id || itemdb_isSingleStorage(*id))
			continue;

		WBUFW(buf,n*10+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=stor.storage[i].nameid;
		WBUFB(buf,n*10+8)=id->getType();
		WBUFB(buf,n*10+9)=stor.storage[i].identify;
		WBUFW(buf,n*10+10)=stor.storage[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1f0;
	for(i=0,n=0;i<MAX_GUILD_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if( !id || itemdb_isSingleStorage(*id) )
			continue;

		WBUFW(buf,n*18+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=stor.storage[i].nameid;
		WBUFB(buf,n*18+8)=id->getType();
		WBUFB(buf,n*18+9)=stor.storage[i].identify;
		WBUFW(buf,n*18+10)=stor.storage[i].amount;
		WBUFW(buf,n*18+12)=0;
		clif_cards_to_buffer( WBUFP(buf,n*18+14), stor.storage[i]);
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageequiplist(struct map_session_data &sd,struct guild_storage &stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf=WFIFOP(fd,0);

	WBUFW(buf,0)=0xa6;
	for(i=0,n=0;i<MAX_GUILD_STORAGE; ++i){
		if(stor.storage[i].nameid<=0)
			continue;
		id = itemdb_exists(stor.storage[i].nameid);
		if(!id || !itemdb_isSingleStorage(*id))
			continue;
		WBUFW(buf,n*20+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=stor.storage[i].nameid;
		WBUFB(buf,n*20+8)=id->getType();
		WBUFB(buf,n*20+9)=stor.storage[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=stor.storage[i].equip;
		WBUFB(buf,n*20+14)=stor.storage[i].attribute;
		WBUFB(buf,n*20+15)=stor.storage[i].refine;
		clif_cards_to_buffer( WBUFP(buf,n*20+16), stor.storage[i]);

		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}



// Guild XY locators [Valaris]
int clif_guild_xy(struct map_session_data &sd)
{
	unsigned char buf[10];
	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=sd.block_list::x;
	WBUFW(buf,8)=sd.block_list::y;
	clif_send(buf,packet_len_table[0x1eb],&sd,GUILD_SAMEMAP_WOS);
	return 0;
}
int clif_guild_xy_remove(struct map_session_data &sd)
{
	unsigned char buf[10];
	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=0xFFFF;
	WBUFW(buf,8)=0xFFFF;
	clif_send(buf,packet_len_table[0x1eb],&sd,GUILD_SAMEMAP_WOS);
	return 0;
}

/*==========================================
 * ステータスを送りつける
 * 表示専用数字はこの中で計算して送る
 *------------------------------------------
 */
int clif_updatestatus(struct map_session_data &sd,unsigned short type)
{
	int fd,len=8;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

 	// send stuff with other functions
	if(type==SP_MANNER)
		clif_changestatus(sd,SP_MANNER,sd.status.manner);
	else if(type==SP_WEIGHT)
		pc_checkweighticon(sd);
	else if(type==SP_HP && config.disp_hpmeter)
		clif_hpmeter(sd);

	// send update things
	WFIFOW(fd,0)=0xb0;
	WFIFOW(fd,2)=type;
	switch(type){
		// 00b0
	case SP_WEIGHT:
		WFIFOL(fd,4)=sd.weight;
		break;
	case SP_MAXWEIGHT:
		WFIFOL(fd,4)=sd.max_weight;
		break;
	case SP_SPEED:
		WFIFOL(fd,4)=sd.get_speed();
		break;
	case SP_BASELEVEL:
		WFIFOL(fd,4)=sd.status.base_level;
		break;
	case SP_JOBLEVEL:
		WFIFOL(fd,4)=sd.status.job_level;
		break;
	case SP_MANNER:
		WFIFOL(fd,4)=sd.status.manner;
		break;
	case SP_STATUSPOINT:
		WFIFOL(fd,4)=sd.status.status_point;
		break;
	case SP_SKILLPOINT:
		WFIFOL(fd,4)=sd.status.skill_point;
		break;
	case SP_HIT:
		WFIFOL(fd,4)=sd.hit;
		break;
	case SP_FLEE1:
		WFIFOL(fd,4)=sd.flee;
		break;
	case SP_FLEE2:
		WFIFOL(fd,4)=sd.flee2/10;
		break;
	case SP_MAXHP:
		WFIFOL(fd,4)=sd.status.max_hp;
		break;
	case SP_MAXSP:
		WFIFOL(fd,4)=sd.status.max_sp;
		break;
	case SP_HP:
		WFIFOL(fd,4)=sd.status.hp;
		break;
	case SP_SP:
		WFIFOL(fd,4)=sd.status.sp;
		break;
	case SP_ASPD:
		WFIFOL(fd,4)=sd.aspd;
		break;
	case SP_ATK1:
		WFIFOL(fd,4)=sd.base_atk+sd.right_weapon.watk;
		break;
	case SP_DEF1:
		WFIFOL(fd,4)=sd.def;
		break;
	case SP_MDEF1:
		WFIFOL(fd,4)=sd.mdef;
		break;
	case SP_ATK2:
		WFIFOL(fd,4)=sd.right_weapon.watk2;
		break;
	case SP_DEF2:
		WFIFOL(fd,4)=sd.def2;
		break;
	case SP_MDEF2:
		WFIFOL(fd,4)=sd.mdef2;
		break;
	case SP_CRITICAL:
		WFIFOL(fd,4)=sd.critical/10;
		break;
	case SP_MATK1:
		WFIFOL(fd,4)=sd.matk1;
		break;
	case SP_MATK2:
		WFIFOL(fd,4)=sd.matk2;
		break;
	case SP_ZENY:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=sd.status.zeny;
		break;
	case SP_BASEEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=sd.status.base_exp;
		break;
	case SP_JOBEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=sd.status.job_exp;
		break;
	case SP_NEXTBASEEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=pc_nextbaseexp(sd);
		break;
	case SP_NEXTJOBEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=pc_nextjobexp(sd);
		break;
		// 00be 終了
	case SP_USTR:
	case SP_UAGI:
	case SP_UVIT:
	case SP_UINT:
	case SP_UDEX:
	case SP_ULUK:
		WFIFOW(fd,0)=0xbe;
		WFIFOB(fd,4)=pc_need_status_point(sd,type-SP_USTR+SP_STR);
		len=5;
		break;

		// 013a 終了
	case SP_ATTACKRANGE:
		WFIFOW(fd,0)=0x13a;
		WFIFOW(fd,2)=sd.attackrange;
		len=4;
		break;

		// 0141 終了
	case SP_STR:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.str;
		WFIFOL(fd,10)=sd.paramb[0] + sd.parame[0];
		len=14;
		break;
	case SP_AGI:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.agi;
		WFIFOL(fd,10)=sd.paramb[1] + sd.parame[1];
		len=14;
		break;
	case SP_VIT:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.vit;
		WFIFOL(fd,10)=sd.paramb[2] + sd.parame[2];
		len=14;
		break;
	case SP_INT:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.int_;
		WFIFOL(fd,10)=sd.paramb[3] + sd.parame[3];
		len=14;
		break;
	case SP_DEX:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.dex;
		WFIFOL(fd,10)=sd.paramb[4] + sd.parame[4];
		len=14;
		break;
	case SP_LUK:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd.status.luk;
		WFIFOL(fd,10)=sd.paramb[5] + sd.parame[5];
		len=14;
		break;

	case SP_CARTINFO:
		WFIFOW(fd,0)=0x121;
		WFIFOW(fd,2)=sd.cart_num;
		WFIFOW(fd,4)=sd.cart_max_num;
		WFIFOL(fd,6)=sd.cart_weight;
		WFIFOL(fd,10)=sd.cart_max_weight;
		len=14;
		break;

	default:
		if(config.error_log)
			ShowMessage("clif_updatestatus : make %d routine\n",type);
		return 1;
	}
	WFIFOSET(fd,len);

	return 0;
}
int clif_changestatus(struct block_list &bl, unsigned short type, uint32 val)
{
	unsigned char buf[12];
	if(bl.type == BL_PC)
	{
		WBUFW(buf,0)=0x1ab;
		WBUFL(buf,2)=bl.id;
		WBUFW(buf,6)=type;
		switch(type){
		case SP_MANNER:
			WBUFL(buf,8)=val;
			break;
		default:
			if(config.error_log)
				ShowMessage("clif_changestatus : make %d routine\n",type);
			return 1;
		}
		clif_send(buf,packet_len_table[0x1ab],&bl,AREA_WOS);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_changelook(const block_list &bl,unsigned char type, unsigned short val)
{
	unsigned char buf[32];

#if PACKETVER < 4

	if(bl->type == BL_PC)
	{
		struct map_session_data &sd = (struct map_session_data &)bl;
		if( (type == LOOK_WEAPON || type == LOOK_SHIELD) && sd.view_class == 22 )
			val =0;
		WBUFW(buf,0)=0xc3;
		WBUFL(buf,2)=bl.id;
		WBUFB(buf,6)=type;
		WBUFB(buf,7)=val;
		return clif_send(buf,packet_len_table[0xc3],&bl,AREA);
	}
	return 0;

#else

	if(bl.type == BL_PC)
	{
		struct map_session_data &sd = (struct map_session_data &)bl;

		if( type == LOOK_WEAPON || type == LOOK_SHIELD || type == LOOK_SHOES )
		{
			WBUFW(buf,0)=0x1d7;
			WBUFL(buf,2)=bl.id;
			if(type == LOOK_SHOES)
			{
				WBUFB(buf,6)=9;
				if(sd.equip_index[2] < MAX_INVENTORY && sd.inventory_data[sd.equip_index[2]])
				{
					if(sd.inventory_data[sd.equip_index[2]]->view_id > 0)
						WBUFW(buf,7)=sd.inventory_data[sd.equip_index[2]]->view_id;
				else
						WBUFW(buf,7)=sd.status.inventory[sd.equip_index[2]].nameid;
				}
				else
				WBUFW(buf,7)=0;
				WBUFW(buf,9)=0;
			}
			else
			{
				WBUFB(buf,6)=2;
				if(sd.equip_index[9] < MAX_INVENTORY && sd.inventory_data[sd.equip_index[9]] && sd.view_class != 22)
				{
					if(sd.inventory_data[sd.equip_index[9]]->view_id > 0)
						WBUFW(buf,7)=sd.inventory_data[sd.equip_index[9]]->view_id;
					else
						WBUFW(buf,7)=sd.status.inventory[sd.equip_index[9]].nameid;
				}
				else
					WBUFW(buf,7)=0;
				if(sd.equip_index[8] < MAX_INVENTORY && sd.equip_index[8] != sd.equip_index[9] && 
					sd.inventory_data[sd.equip_index[8]] && sd.view_class != 22)
				{
					if(sd.inventory_data[sd.equip_index[8]]->view_id > 0)
						WBUFW(buf,9)=sd.inventory_data[sd.equip_index[8]]->view_id;
					else
						WBUFW(buf,9)=sd.status.inventory[sd.equip_index[8]].nameid;
				}
				else
					WBUFW(buf,9)=0;
			}
			return clif_send(buf,packet_len_table[0x1d7],&bl,AREA);
		}
		else if( (type == LOOK_BASE) && (val > 255) )
		{
			WBUFW(buf,0)=0x1d7;
			WBUFL(buf,2)=bl.id;
			WBUFB(buf,6)=type;
			WBUFW(buf,7)=val;
			WBUFW(buf,9)=0;
			return clif_send(buf,packet_len_table[0x1d7],&bl,AREA);
		}

	}
	WBUFW(buf,0)=0xc3;
	WBUFL(buf,2)=bl.id;
	WBUFB(buf,6)=type;
	WBUFB(buf,7)=val;
	return clif_send(buf,packet_len_table[0xc3],&bl,AREA);

#endif

}

/*==========================================
 *
 *------------------------------------------
 */
int clif_initialstatus(struct map_session_data &sd)
{
	int fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf=WFIFOP(fd,0);

	WBUFW(buf,0)=0xbd;
	WBUFW(buf,2)=sd.status.status_point;
	WBUFB(buf,4)=(sd.status.str > 255)? 255:sd.status.str;
	WBUFB(buf,5)=pc_need_status_point(sd,SP_STR);
	WBUFB(buf,6)=(sd.status.agi > 255)? 255:sd.status.agi;
	WBUFB(buf,7)=pc_need_status_point(sd,SP_AGI);
	WBUFB(buf,8)=(sd.status.vit > 255)? 255:sd.status.vit;
	WBUFB(buf,9)=pc_need_status_point(sd,SP_VIT);
	WBUFB(buf,10)=(sd.status.int_ > 255)? 255:sd.status.int_;
	WBUFB(buf,11)=pc_need_status_point(sd,SP_INT);
	WBUFB(buf,12)=(sd.status.dex > 255)? 255:sd.status.dex;
	WBUFB(buf,13)=pc_need_status_point(sd,SP_DEX);
	WBUFB(buf,14)=(sd.status.luk > 255)? 255:sd.status.luk;
	WBUFB(buf,15)=pc_need_status_point(sd,SP_LUK);

	WBUFW(buf,16) = sd.base_atk + sd.right_weapon.watk;
	WBUFW(buf,18) = sd.right_weapon.watk2; //atk bonus
	WBUFW(buf,20) = sd.matk1;
	WBUFW(buf,22) = sd.matk2;
	WBUFW(buf,24) = sd.def; // def
	WBUFW(buf,26) = sd.def2;
	WBUFW(buf,28) = sd.mdef; // mdef
	WBUFW(buf,30) = sd.mdef2;
	WBUFW(buf,32) = sd.hit;
	WBUFW(buf,34) = sd.flee;
	WBUFW(buf,36) = sd.flee2/10;
	WBUFW(buf,38) = sd.critical/10;
	WBUFW(buf,40) = sd.status.karma;
	WBUFW(buf,42) = sd.status.manner;

	WFIFOSET(fd,packet_len_table[0xbd]);

	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);

	clif_updatestatus(sd,SP_ATTACKRANGE);
	clif_updatestatus(sd,SP_ASPD);

	return 0;
}

/*==========================================
 *矢装備
 *------------------------------------------
 */
int clif_arrowequip(struct map_session_data &sd,unsigned short val)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(sd.target_id)
		sd.target_id = 0;

	WFIFOW(fd,0)=0x013c;
	WFIFOW(fd,2)=val+2;//矢のアイテムID

	WFIFOSET(fd,packet_len_table[0x013c]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_arrow_fail(struct map_session_data &sd,unsigned short type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x013b;
	WFIFOW(fd,2)=type;

	WFIFOSET(fd,packet_len_table[0x013b]);

	return 0;
}

/*==========================================
 * 作成可能 矢リスト送信
 *------------------------------------------
 */
int clif_arrow_create_list(struct map_session_data &sd)
{
	int i, c, j;
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x1ad;

	for (i = 0, c = 0; i < MAX_SKILL_ARROW_DB; ++i) {
		if (skill_arrow_db[i].nameid > 0 &&
			(j = pc_search_inventory(sd, skill_arrow_db[i].nameid)) >= 0 &&
			!sd.status.inventory[j].equip)
		{
			if ((j = itemdb_viewid(skill_arrow_db[i].nameid)) > 0)
				WFIFOW(fd,c*2+4) = j;
			else
				WFIFOW(fd,c*2+4) = skill_arrow_db[i].nameid;
			c++;
		}
	}
	WFIFOW(fd,2) = c*2+4;
	WFIFOSET(fd, WFIFOW(fd,2));
	if (c > 0) sd.state.produce_flag = 1;

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_statusupack(struct map_session_data &sd,unsigned short type,unsigned char ok,unsigned char val)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xbc;
	WFIFOW(fd,2)=type;
	WFIFOB(fd,4)=ok;
	WFIFOB(fd,5)=val;
	WFIFOSET(fd,packet_len_table[0xbc]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_equipitemack(struct map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xaa;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet_len_table[0xaa]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_unequipitemack(struct map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xac;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet_len_table[0xac]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_misceffect(struct block_list &bl, uint32 type)
{
	unsigned char buf[32];

	WBUFW(buf,0) = 0x19b;
	WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = type;

	return clif_send(buf,packet_len_table[0x19b],&bl,AREA);
}

int clif_misceffect2(struct block_list &bl, uint32 type)
{
	unsigned char buf[24];
	memset(buf, 0, packet_len_table[0x1f3]);

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = type;

	return clif_send(buf, packet_len_table[0x1f3], &bl, AREA);
}
/*==========================================
 * 表示オプション変更
 *------------------------------------------
 */
int clif_changeoption(struct block_list &bl)
{
	unsigned char buf[32];
	short option;
	struct status_change *sc_data;
	static const int omask[]={ 0x10,0x20 };
	static const int scnum[]={ SC_FALCON, SC_RIDING };
	size_t i;

	option = *status_get_option(&bl);
	sc_data = status_get_sc_data(&bl);

	WBUFW(buf,0) = 0x119;
	if(bl.type==BL_PC && ((struct map_session_data &)bl).disguise_id)
	{
		WBUFL(buf,2) = bl.id;
		WBUFW(buf,6) = 0;
		WBUFW(buf,8) = 0;
		WBUFW(buf,10) = OPTION_HIDE;
		WBUFB(buf,12) = 0;
		clif_send(buf,packet_len_table[0x119],&bl,AREA);
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		WBUFW(buf,6) = 0;
		WBUFW(buf,8) = 0;
		WBUFW(buf,10) = option;
		WBUFB(buf,12) = 0;
		clif_send(buf,packet_len_table[0x119],&bl,AREA);
	}
	else
	{
		WBUFL(buf,2) = bl.id;
		WBUFW(buf,6) = *status_get_opt1(&bl);
		WBUFW(buf,8) = *status_get_opt2(&bl);
		WBUFW(buf,10) = option;
		WBUFB(buf,12) = 0;	// ??
		clif_send(buf,packet_len_table[0x119],&bl,AREA);
	}

	// アイコンの表示
	for(i=0;i<sizeof(omask)/sizeof(omask[0]); ++i){
		if( option&omask[i] ){
			if( sc_data[scnum[i]].timer==-1)
				status_change_start(&bl,scnum[i],0,0,0,0,0,0);
		} else {
			status_change_end(&bl,scnum[i],-1);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_useitemack(struct map_session_data &sd,unsigned short index,unsigned short amount,unsigned char ok)
{
	if(!ok)
	{
		int fd=sd.fd;
		if( !session_isActive(fd) )
			return 0;

		WFIFOW(fd,0)=0xa8;
		WFIFOW(fd,2)=index+2;
		WFIFOW(fd,4)=amount;
		WFIFOB(fd,6)=ok;
		WFIFOSET(fd,packet_len_table[0xa8]);
	}
	else {
#if PACKETVER < 3
		int fd=sd.fd;
		if( !session_isActive(fd) )
			return 0;
		
		WFIFOW(fd,0)=0xa8;
		WFIFOW(fd,2)=index+2;
		WFIFOW(fd,4)=amount;
		WFIFOB(fd,6)=ok;
		WFIFOSET(fd,packet_len_table[0xa8]);
#else
		unsigned char buf[32];

		WBUFW(buf,0)=0x1c8;
		WBUFW(buf,2)=index+2;
		if(sd.inventory_data[index] && sd.inventory_data[index]->view_id > 0)
			WBUFW(buf,4)=sd.inventory_data[index]->view_id;
		else
			WBUFW(buf,4)=sd.status.inventory[index].nameid;
		WBUFL(buf,6)=sd.block_list::id;
		WBUFW(buf,10)=amount;
		WBUFB(buf,12)=ok;
		clif_send(buf,packet_len_table[0x1c8],&sd,AREA);
#endif
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_createchat(struct map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd6;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_len_table[0xd6]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_dispchat(chat_data &cd,int fd)
{
	unsigned char buf[128];	// 最大title(60バイト)+17

	if(cd.owner==NULL)
		return 1;

	WBUFW(buf,0)=0xd7;
	WBUFW(buf,2)=strlen((char*)(cd.title))+17;
	WBUFL(buf,4)=cd.owner->id;
	WBUFL(buf,8)=cd.block_list::id;
	WBUFW(buf,12)=cd.limit;
	WBUFW(buf,14)=cd.users;
	WBUFB(buf,16)=cd.pub;
	strcpy((char*)WBUFP(buf,17),cd.title);

	if(fd && session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0),buf,WBUFW(buf,2));
		WFIFOSET(fd,WBUFW(buf,2));
	} else {
		clif_send(buf,WBUFW(buf,2),cd.owner,AREA_WOSC);
	}

	return 0;
}

/*==========================================
 * chatの状態変更成功
 * 外部の人用と命令コード(d7->df)が違うだけ
 *------------------------------------------
 */
int clif_changechatstatus(chat_data &cd)
{
	unsigned char buf[128];	// 最大title(60バイト)+17

	if(cd.usersd[0]==NULL)
		return 1;

	WBUFW(buf,0)=0xdf;
	WBUFW(buf,2)=strlen(cd.title)+17;
	WBUFL(buf,4)=cd.usersd[0]->block_list::id;
	WBUFL(buf,8)=cd.block_list::id;
	WBUFW(buf,12)=cd.limit;
	WBUFW(buf,14)=cd.users;
	WBUFB(buf,16)=cd.pub;
	strcpy((char*)WBUFP(buf,17),cd.title);
	clif_send(buf,strlen(cd.title)+17, cd.usersd[0], CHAT);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchat(chat_data &cd,int fd)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0xd8;
	WBUFL(buf,2)=cd.block_list::id;

	if( fd && session_isActive(fd) )
	{
		memcpy(WFIFOP(fd,0),buf,packet_len_table[0xd8]);
		WFIFOSET(fd,packet_len_table[0xd8]);
	}
	else
	{
		clif_send(buf,packet_len_table[0xd8],cd.owner,AREA_WOSC);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_joinchatfail(struct map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xda;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_len_table[0xda]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_joinchatok(map_session_data &sd,struct chat_data &cd)
{
	int i;
	int fd = sd.fd;
	if (!session_isActive(fd))
		return 0;

	WFIFOW(fd, 0) = 0xdb;
	WFIFOW(fd, 2) = 8 + (28*cd.users);
	WFIFOL(fd, 4) = cd.block_list::id;
	for (i = 0; i < cd.users; ++i)
	{
		WFIFOL(fd, 8+i*28) = (i!=0) || (cd.owner->type == BL_NPC);
		memcpy(WFIFOP(fd, 8+i*28+4), cd.usersd[i]->status.name, 24);
	}
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_addchat(chat_data &cd,struct map_session_data &sd)
{
	unsigned char buf[32];
	WBUFW(buf, 0) = 0x0dc;
	WBUFW(buf, 2) = cd.users;
	memcpy(WBUFP(buf, 4),sd.status.name,24);
	return clif_send(buf,packet_len_table[0xdc],&sd,CHAT_WOS);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changechatowner(chat_data &cd,struct map_session_data &sd)
{
	unsigned char buf[64];

	WBUFW(buf, 0) = 0xe1;
	WBUFL(buf, 2) = 1;
	if(cd.usersd[0]) memcpy(WBUFP(buf,6), cd.usersd[0]->status.name,24);
	WBUFW(buf,30) = 0xe1;
	WBUFL(buf,32) = 0;
	memcpy(WBUFP(buf,36),sd.status.name,24);

	return clif_send(buf,packet_len_table[0xe1]*2,&sd,CHAT);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_leavechat(chat_data &cd,struct map_session_data &sd)
{
	unsigned char buf[32];

	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = cd.users-1;
	memcpy(WBUFP(buf,4),sd.status.name,24);
	WBUFB(buf,28) = 0;

	return clif_send(buf,packet_len_table[0xdd],&sd,CHAT);
}

/*==========================================
 * 取り引き要請受け
 *------------------------------------------
 */
int clif_traderequest(struct map_session_data &sd,const char *name)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xe5;

	strcpy((char*)WFIFOP(fd,2),name);
	
	WFIFOSET(fd,packet_len_table[0xe5]);

	return 0;
}

/*==========================================
 * 取り引き要求応答
 *------------------------------------------
 */
int clif_tradestart(struct map_session_data &sd,unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xe7;
	WFIFOB(fd,2)=type;
	WFIFOSET(fd,packet_len_table[0xe7]);

	return 0;
}

/*==========================================
 * 相手方からのアイテム追加
 *------------------------------------------
 */
int clif_tradeadditem(struct map_session_data &sd, struct map_session_data &tsd,unsigned short index, uint32 amount)
{
	int fd=tsd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xe9;
	WFIFOL(fd,2)=amount;
	index -= 2;
	// will cover all invalid states of variable 'index '
	if(index >= MAX_INVENTORY)
	{
		WFIFOW(fd,6) = 0; // type id
		WFIFOB(fd,8) = 0; //identify flag
		WFIFOB(fd,9) = 0; // attribute
		WFIFOB(fd,10)= 0; //refine
		WFIFOW(fd,11)= 0; //card (4w)
		WFIFOW(fd,13)= 0; //card (4w)
		WFIFOW(fd,15)= 0; //card (4w)
		WFIFOW(fd,17)= 0; //card (4w)
	}
		else
	{
		if(sd.inventory_data[index] && sd.inventory_data[index]->view_id > 0)
			WFIFOW(fd,6) = sd.inventory_data[index]->view_id;
		else
			WFIFOW(fd,6) = sd.status.inventory[index].nameid; // type id
		WFIFOB(fd,8) = sd.status.inventory[index].identify; //identify flag
		WFIFOB(fd,9) = sd.status.inventory[index].attribute; // attribute
		WFIFOB(fd,10)= sd.status.inventory[index].refine; //refine
		clif_cards_to_buffer( WFIFOP(fd,11), sd.status.inventory[index]);

	}
	WFIFOSET(fd,packet_len_table[0xe9]);

	return 0;
}

/*==========================================
 * アイテム追加成功/失敗
 *------------------------------------------
 */
int clif_tradeitemok(struct map_session_data &sd,unsigned short index,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xea;
	WFIFOW(fd,2)=index;
	WFIFOB(fd,4)=fail;
	WFIFOSET(fd,packet_len_table[0xea]);

	return 0;
}

/*==========================================
 * 取り引きok押し
 *------------------------------------------
 */
int clif_tradedeal_lock(struct map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xec;
	WFIFOB(fd,2)=fail; // 0=you 1=the other person
	WFIFOSET(fd,packet_len_table[0xec]);

	return 0;
}

/*==========================================
 * 取り引きがキャンセルされました
 *------------------------------------------
 */
int clif_tradecancelled(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xee;
	WFIFOSET(fd,packet_len_table[0xee]);

	return 0;
}

/*==========================================
 * 取り引き完了
 *------------------------------------------
 */
int clif_tradecompleted(struct map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf0;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_len_table[0xf0]);

	return 0;
}

/*==========================================
 * カプラ倉庫のアイテム数を更新
 *------------------------------------------
 */
int clif_updatestorageamount(struct map_session_data &sd,struct pc_storage &stor)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor.storage_amount;  //items
	WFIFOW(fd,4) = MAX_STORAGE; //items max
	WFIFOSET(fd,packet_len_table[0xf2]);

	return 0;
}

/*==========================================
 * カプラ倉庫にアイテムを追加する
 *------------------------------------------
 */
int clif_storageitemadded(struct map_session_data &sd,struct pc_storage &stor,unsigned short index,uint32 amount)
{
	int view,fd;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) =0xf4; // Storage item added
	WFIFOW(fd,2) =index+1; // index
	WFIFOL(fd,4) =amount; // amount
	if((view = itemdb_viewid(stor.storage[index].nameid)) > 0)
		WFIFOW(fd,8) =view;
	else
		WFIFOW(fd,8) =stor.storage[index].nameid; // id
	WFIFOB(fd,10)=stor.storage[index].identify; //identify flag
	WFIFOB(fd,11)=stor.storage[index].attribute; // attribute
	WFIFOB(fd,12)=stor.storage[index].refine; //refine

	clif_cards_to_buffer( WFIFOP(fd,13), stor.storage[index] );

	WFIFOSET(fd,packet_len_table[0xf4]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_updateguildstorageamount(struct map_session_data &sd,struct guild_storage &stor)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor.storage_amount;  //items
	WFIFOW(fd,4) = MAX_GUILD_STORAGE; //items max
	WFIFOSET(fd,packet_len_table[0xf2]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageitemadded(struct map_session_data &sd,struct guild_storage &stor,unsigned short index,uint32 amount)
{
	int view,fd;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) =0xf4; // Storage item added
	WFIFOW(fd,2) =index+1; // index
	WFIFOL(fd,4) =amount; // amount
	if((view = itemdb_viewid(stor.storage[index].nameid)) > 0)
		WFIFOW(fd,8) =view;
	else
		WFIFOW(fd,8) =stor.storage[index].nameid; // id
	WFIFOB(fd,10)=stor.storage[index].identify; //identify flag
	WFIFOB(fd,11)=stor.storage[index].attribute; // attribute
	WFIFOB(fd,12)=stor.storage[index].refine; //refine

	clif_cards_to_buffer( WFIFOP(fd,13), stor.storage[index] );

	WFIFOSET(fd,packet_len_table[0xf4]);

	return 0;
}

/*==========================================
 * カプラ倉庫からアイテムを取り去る
 *------------------------------------------
 */
int clif_storageitemremoved(struct map_session_data &sd,unsigned short index,uint32 amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf6; // Storage item removed
	WFIFOW(fd,2)=index+1;
	WFIFOL(fd,4)=amount;
	WFIFOSET(fd,packet_len_table[0xf6]);

	return 0;
}

/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
int clif_storageclose(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf8; // Storage Closed
	WFIFOSET(fd,packet_len_table[0xf8]);

	return 0;
}

//
// callback系 ?
//
/*==========================================
 * PC表示
 *------------------------------------------
 */
int clif_getareachar_pc(struct map_session_data &sd, struct map_session_data &dstsd)
{
	size_t len;
	if( !session_isActive(sd.fd) )
		return 0;

	if( dstsd.is_walking() )
	{
		len = clif_set007b(dstsd,WFIFOP(sd.fd,0));
		WFIFOSET(sd.fd,len);
		if(dstsd.disguise_id)
		{
			len = clif_dis007b(dstsd,WFIFOP(sd.fd,0));
			WFIFOSET(sd.fd,len);
		}
	}
	else
	{
		len = clif_set0078(dstsd,WFIFOP(sd.fd,0));
		WFIFOSET(sd.fd,len);
		if(dstsd.disguise_id)
		{
			len = clif_dis0078(dstsd,WFIFOP(sd.fd,0));
			WFIFOSET(sd.fd,len);
		}
	}


	if(dstsd.chatID)
	{
		chat_data *cd;
		cd=(chat_data*)map_id2bl(dstsd.chatID);
		if(cd->usersd[0]==&dstsd)
			clif_dispchat(*cd,sd.fd);
	}
	if(dstsd.vender_id){
		clif_showvendingboard(dstsd, dstsd.message, sd.fd);
	}
	if(dstsd.spiritball > 0) {
		clif_set01e1(dstsd,WFIFOP(sd.fd,0));
		WFIFOSET(sd.fd,packet_len_table[0x1e1]);
	}
	if( config.save_clothcolor && dstsd.status.clothes_color > 0 &&
		(dstsd.view_class != 22 || !config.wedding_ignorepalette) )
		clif_changelook(dstsd,LOOK_CLOTHES_COLOR,dstsd.status.clothes_color);
	if(sd.status.manner < 0)
		clif_changestatus(sd,SP_MANNER,sd.status.manner);
		
	if(sd.state.viewsize==2) // tiny/big players [Valaris]
		clif_specialeffect(sd,EFFECT_BIG,0);
	else if(sd.state.viewsize==1)
		clif_specialeffect(sd,EFFECT_TINY,0);

	return 0;
}

/*==========================================
 * NPC表示
 *------------------------------------------
 */
//fixed by Valaris
int clif_getareachar_npc(struct map_session_data &sd,struct npc_data &nd)
{
	int len;
	if( !session_isActive(sd.fd) )
		return 0;

	if(nd.class_ < 0 || nd.flag&1 || nd.class_ == INVISIBLE_CLASS)
		return 0;
	if(nd.is_walking() )
		len = clif_npc007b(nd,WFIFOP(sd.fd,0));
	else
		len = clif_npc0078(nd,WFIFOP(sd.fd,0));
	WFIFOSET(sd.fd,len);

	if(nd.chat_id)
	{
		chat_data*ct = (chat_data*)map_id2bl(nd.chat_id);
		if(ct) clif_dispchat(*ct,sd.fd);
	}
	return 0;
}






int clif_fixpos(const block_list &bl)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x88;
	WBUFL(buf,2)=bl.id;
	WBUFW(buf,6)=bl.x;
	WBUFW(buf,8)=bl.y;

	clif_send(buf, packet_len_table[0x88], &bl, AREA);

	if(bl.type==BL_PC)
	{
		struct map_session_data &sd=(struct map_session_data &)bl;

		WBUFL(buf,2)=0xFFFFFFF6;//-10;
		WBUFW(buf,6)=bl.x;
		WBUFW(buf,8)=bl.y;
		clif_send_self(sd, buf, packet_len_table[0x88]);
		
		if(sd.disguise_id)
		{	// reusing the buffer
			WBUFL(buf,2)=bl.id|FLAG_DISGUISE;
			WBUFW(buf,6)=bl.x;
			WBUFW(buf,8)=bl.y;
			clif_send(buf, packet_len_table[0x88], &bl, AREA);
		}
	}

	return 0;
}

/// fix object
int clif_fixobject(const block_list &bl)
{
	unsigned char buf[256];
	int len=0;

	clif_fixpos(bl);

	if( bl.get_sd() )
	{
		const map_session_data &sd = *bl.get_sd();
		if( sd.is_walking() )
			len = clif_set007b(sd,buf);
		else
			len = clif_set0078(sd,buf);
	}
	if( bl.get_md() )
	{
		const mob_data &md = *bl.get_md();
		
		if( md.is_walking() )
			len = clif_mob007b(md,buf);
		else
			len = clif_mob0078(md,buf);
	}
	else if( bl.get_pd() )
	{
		const pet_data &pd = *bl.get_pd();
		
		if( pd.is_walking() )
			len = clif_pet007b(pd,buf);
		else
			len = clif_pet0078(pd,buf);
	}
	else if( bl.get_nd() )
	{
		const npc_data &nd = *bl.get_nd();
		
		if( nd.is_walking() )
			len = clif_npc007b(nd,buf);
		else
			len = clif_npc0078(nd,buf);
	}
	else
	{
		return 0;
	}
	return clif_send(buf,len,&bl,AREA);
}


/*==========================================
 * 通常攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_damage(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,unsigned short damage,unsigned short div,unsigned char type,unsigned short damage2)
{
	unsigned char buf[256];
	struct status_change *sc_data;

	sc_data = status_get_sc_data(&dst);

	if(type != 4 && dst.type == BL_PC && ((struct map_session_data &)dst).state.infinite_endure)
		type = 9;
	if(sc_data)
	{
		if( type != 4 && sc_data[SC_ENDURE].timer != -1 && 
			dst.type == BL_PC && !maps[dst.m].flag.gvg )
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1)
		{
			if(damage > 0)
				damage = damage*(5+sc_data[SC_HALLUCINATION].val1.num) + rand()%100;
			if(damage2 > 0)
				damage2 = damage2*(5+sc_data[SC_HALLUCINATION].val1.num) + rand()%100;
		}
	}
	
	WBUFW(buf,0)=0x8a;
	if(src.type==BL_PC && ((struct map_session_data &)src).disguise_id)
		WBUFL(buf,2)=src.id|FLAG_DISGUISE;
	else 
		WBUFL(buf,2)=src.id;
	if(dst.type==BL_PC && ((struct map_session_data &)dst).disguise_id)
		WBUFL(buf,6)=dst.id|FLAG_DISGUISE;
	else
		WBUFL(buf,6)=dst.id;
	WBUFL(buf,10)=tick;
	WBUFL(buf,14)=sdelay;
	WBUFL(buf,18)=ddelay;
	WBUFW(buf,22)=(damage > 0x7fff)?0x7fff:damage;
	WBUFW(buf,24)=div;
	WBUFB(buf,26)=type;
	WBUFW(buf,27)=damage2;
	clif_send(buf,packet_len_table[0x8a],&src,AREA);

	if( (src.type==BL_PC && ((struct map_session_data &)src).disguise_id) || 
		(dst.type==BL_PC && ((struct map_session_data &)dst).disguise_id) )
	{
		WBUFW(buf,0)=0x8a;
		if(src.type==BL_PC && ((struct map_session_data &)src).disguise_id)
			WBUFL(buf,2)=src.id|FLAG_DISGUISE;
		else 
			WBUFL(buf,2)=src.id;
		if(dst.type==BL_PC && ((struct map_session_data &)dst).disguise_id)
			WBUFL(buf,6)=dst.id|FLAG_DISGUISE;
		else 
			WBUFL(buf,6)=dst.id;
		WBUFL(buf,10)=tick;
		WBUFL(buf,14)=sdelay;
		WBUFL(buf,18)=ddelay;
		if(damage > 0)
			WBUFW(buf,22)=0xFFFF;
		else
			WBUFW(buf,22)=0;
		WBUFW(buf,24)=div;
		WBUFB(buf,26)=type;
		WBUFW(buf,27)=0;
		clif_send(buf,packet_len_table[0x8a],&src,AREA);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar_mob(struct map_session_data &sd,struct mob_data &md)
{
	int len;

	if( !session_isActive(sd.fd) )
		return 0;

	if( md.is_walking() )
		len = clif_mob007b(md,WFIFOP(sd.fd,0));
	else
		len = clif_mob0078(md,WFIFOP(sd.fd,0));
	WFIFOSET(sd.fd,len);

	if(md.get_equip() > 0) // mob equipment [Valaris]
		clif_mob_equip(md,md.get_equip());

	if(md.state.size==2) // tiny/big mobs [Valaris]
		clif_specialeffect(md,EFFECT_BIG,0);
	else if(md.state.size==1)
		clif_specialeffect(md,EFFECT_TINY,0);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar_pet(struct map_session_data &sd,struct pet_data &pd)
{
	size_t len;

	if( !session_isActive(sd.fd) )
		return 0;

	if(pd.state.state == MS_WALK){
		len = clif_pet007b(pd,WFIFOP(sd.fd,0));
	} else {
		len = clif_pet0078(pd,WFIFOP(sd.fd,0));
	}
	WFIFOSET(sd.fd,len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar(struct map_session_data &sd, struct homun_data &hd)
{
	size_t len;

	if( !session_isActive(sd.fd) )
		return 0;

	if( hd.is_walking() ){
		len = clif_hom007b(hd,WFIFOP(sd.fd,0));
	} else {
		len = clif_hom0078(hd,WFIFOP(sd.fd,0));
	}
	WFIFOSET(sd.fd,len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar_item(struct map_session_data &sd, flooritem_data &fitem)
{
	int view,fd=sd.fd;

	if( !session_isActive(fd) )
			return 0;

	//009d <ID>.l <item ID>.w <identify flag>.B <X>.w <Y>.w <amount>.w <subX>.B <subY>.B
	WFIFOW(fd,0)=0x9d;
	WFIFOL(fd,2)=fitem.block_list::id;
	if((view = itemdb_viewid(fitem.item_data.nameid)) > 0)
		WFIFOW(fd,6)=view;
	else
		WFIFOW(fd,6)=fitem.item_data.nameid;
	WFIFOB(fd,8)=fitem.item_data.identify;
	WFIFOW(fd,9)=fitem.block_list::x;
	WFIFOW(fd,11)=fitem.block_list::y;
	WFIFOW(fd,13)=fitem.item_data.amount;
	WFIFOB(fd,15)=fitem.subx;
	WFIFOB(fd,16)=fitem.suby;

	WFIFOSET(fd,packet_len_table[0x9d]);
	return 0;
}
/*==========================================
 * 場所スキルエフェクトが視界に入る
 *------------------------------------------
 */
int clif_getareachar_skillunit(struct map_session_data &sd,struct skill_unit &unit)
{

	struct block_list *bl;
	int fd=sd.fd;

	if( !session_isActive(fd) || !unit.group )
			return 0;

	bl=map_id2bl(unit.group->src_id);
#if PACKETVER < 3
	memset(WFIFOP(fd,0),0,packet_len_table[0x11f]);
	WFIFOW(fd, 0)=0x11f;
	WFIFOL(fd, 2)=unit.block_list::id;
	WFIFOL(fd, 6)=unit.group->src_id;
	WFIFOW(fd,10)=unit.block_list::x;
	WFIFOW(fd,12)=unit.block_list::y;
	WFIFOB(fd,14)=unit.group->unit_id;
	WFIFOB(fd,15)=0;
	WFIFOSET(fd,packet_len_table[0x11f]);
#else
	memset(WFIFOP(fd,0),0,packet_len_table[0x1c9]);
	WFIFOW(fd, 0)=0x1c9;
	WFIFOL(fd, 2)=unit.block_list::id;
	WFIFOL(fd, 6)=unit.group->src_id;
	WFIFOW(fd,10)=unit.block_list::x;
	WFIFOW(fd,12)=unit.block_list::y;
	WFIFOB(fd,14)=unit.group->unit_id;
	/*	Graffiti [Valaris]	*/
	if(unit.group->unit_id==0xb0)
	{
		WFIFOL(fd,15)=1;
		WFIFOL(fd,16)=1;
		memcpy(WFIFOP(fd,17),unit.group->valstring.c_str(),80);
	}
	else
	{
		WFIFOB(fd,15)=1;
		WFIFOL(fd,15+1)=0;						//1-4調べた限り固定
		WFIFOL(fd,15+5)=0;						//5-8調べた限り固定
												//9-12マップごとで一定の77-80とはまた違う4バイトのかなり大きな数字
		WFIFOL(fd,15+13)=unit.block_list::y - 0x12;		//13-16ユニットのY座標-18っぽい(Y:17でFF FF FF FF)
		WFIFOL(fd,15+17)=0x004f37dd;			//17-20調べた限り固定
		WFIFOL(fd,15+21)=0x0012f674;			//21-24調べた限り固定
		WFIFOL(fd,15+25)=0x0012f664;			//25-28調べた限り固定
		WFIFOL(fd,15+29)=0x0012f654;			//29-32調べた限り固定
		WFIFOL(fd,15+33)=0x77527bbc;			//33-36調べた限り固定
												//37-39
		WFIFOB(fd,15+40)=0x2d;					//40調べた限り固定
		WFIFOL(fd,15+41)=0;						//41-44調べた限り0固定
		WFIFOL(fd,15+45)=0;						//45-48調べた限り0固定
		WFIFOL(fd,15+49)=0;						//49-52調べた限り0固定
		WFIFOL(fd,15+53)=0x0048d919;			//53-56調べた限り固定
		WFIFOL(fd,15+57)=0x0000003e;			//57-60調べた限り固定
		WFIFOL(fd,15+61)=0x0012f66c;			//61-64調べた限り固定
												//65-68
												//69-72
		if(bl) WFIFOL(fd,15+73)=bl->y;			//73-76術者のY座標
		WFIFOL(fd,15+77)=unit.block_list::m;	//77-80マップIDかなぁ？かなり2バイトで足りそうな数字
		WFIFOB(fd,15+81)=0xaa;					//81終端文字0xaa
	}
	WFIFOSET(fd,packet_len_table[0x1c9]);
#endif
	if(unit.group && unit.group->skill_id == WZ_ICEWALL)
		clif_set0192(fd,unit.block_list::m,unit.block_list::x,unit.block_list::y,5);

	return 0;
}
/*==========================================
 * 場所スキルエフェクトが視界から消える
 *------------------------------------------
 */
int clif_clearchar_skillunit(struct skill_unit &unit, int fd)
{
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd, 0)=0x120;
	WFIFOL(fd, 2)=unit.block_list::id;
	WFIFOSET(fd,packet_len_table[0x120]);
	if(unit.group && unit.group->skill_id == WZ_ICEWALL)
		clif_set0192(fd,unit.block_list::m,unit.block_list::x,unit.block_list::y,unit.val2);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_01ac(struct block_list &bl)
{
	unsigned char buf[32];
	WBUFW(buf, 0) = 0x1ac;
	WBUFL(buf, 2) = bl.id;
	return clif_send(buf,packet_len_table[0x1ac],&bl,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
class CClifGetAreaChar : public CMapProcessor
{
	struct map_session_data &sd;
public:

	CClifGetAreaChar(struct map_session_data &s) : sd(s)	{}
	~CClifGetAreaChar()	{}

	virtual int process(struct block_list& bl) const
	{
		if( session_isActive(sd.fd) )
		{
			switch(bl.type)
			{
			case BL_PC:
				if(sd.block_list::id == bl.id)
					break;
				clif_getareachar_pc(sd, ((struct map_session_data&)bl));
				break;
			case BL_NPC:
				clif_getareachar_npc(sd, ((struct npc_data&) bl));
				break;
			case BL_MOB:
				clif_getareachar_mob(sd,((struct mob_data&) bl));
				break;
			case BL_PET:
				clif_getareachar_pet(sd, ((struct pet_data&) bl));
				break;
			case BL_HOM:
				clif_getareachar(sd, ((struct homun_data&) bl));
				break;
			case BL_ITEM:
				clif_getareachar_item(sd, ((flooritem_data&) bl));
				break;
			case BL_SKILL:
				clif_getareachar_skillunit(sd, ((struct skill_unit&)bl));
				break;
			default:
				if(config.error_log)
					ShowMessage("get area char ??? (bl.type=%d)\n",bl.type);
				break;
			}
		}
		return 0;
	}
};

/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_pcinsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data *sd;

	nullpo_retr(0, ap);
	nullpo_retr(0, sd=va_arg(ap,struct map_session_data*));

	switch(bl.type){
	case BL_PC:
	{
		struct map_session_data &dstsd=(struct map_session_data&)bl;
		if(sd->block_list::id != dstsd.block_list::id) {
			clif_getareachar_pc(*sd, dstsd);
			clif_getareachar_pc(dstsd, *sd);
		}
		break;
	}
	case BL_NPC:
		clif_getareachar_npc(*sd, ((struct npc_data&)bl));
		break;
	case BL_MOB:
		clif_getareachar_mob(*sd, ((struct mob_data&)bl));
		break;
	case BL_PET:
		clif_getareachar_pet(*sd, ((struct pet_data&)bl));
		break;
	case BL_ITEM:
		clif_getareachar_item(*sd, ((flooritem_data&)bl));
		break;
	case BL_SKILL:
		clif_getareachar_skillunit(*sd, ((struct skill_unit&)bl));
		break;
	}

	return 0;
}
*/
int CClifPCInsight::process(struct block_list& bl) const
{
	switch(bl.type)
	{
	case BL_PC:
	{
		struct map_session_data &dstsd=(struct map_session_data&)bl;
		if(sd.block_list::id != dstsd.block_list::id)
		{
			clif_getareachar_pc(sd, dstsd);
			clif_getareachar_pc(dstsd, sd);
		}
		break;
	}
	case BL_NPC:
		clif_getareachar_npc(sd, ((struct npc_data&)bl));
		break;
	case BL_MOB:
		clif_getareachar_mob(sd, ((struct mob_data&)bl));
		break;
	case BL_PET:
		clif_getareachar_pet(sd, ((struct pet_data&)bl));
		break;
	case BL_HOM:
		clif_getareachar(sd, ((struct homun_data&) bl));
		break;
	case BL_ITEM:
		clif_getareachar_item(sd, ((flooritem_data&)bl));
		break;
	case BL_SKILL:
		clif_getareachar_skillunit(sd, ((struct skill_unit&)bl));
		break;
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_pcoutsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data *sd;

	nullpo_retr(0, ap);
	nullpo_retr(0, sd=va_arg(ap,struct map_session_data*));

	switch(bl.type)
	{
	case BL_PC:
	{
		struct map_session_data &dstsd = (struct map_session_data&)bl;
		if(sd->block_list::id != dstsd.block_list::id)
		{
			clif_clearchar_id(*sd, dstsd.block_list::id, 0);
			clif_clearchar_id(dstsd, sd->block_list::id, 0);

			if(dstsd.disguise_id || sd->disguise_id)
			{
				clif_clearchar_id(*sd, dstsd.block_list::id|FLAG_DISGUISE, 0);
				clif_clearchar_id(dstsd, sd->block_list::id|FLAG_DISGUISE, 0);
			}


			if(dstsd.chatID){
				chat_data *cd;
				cd=(chat_data*)map_id2bl(dstsd.chatID);
				if(cd->usersd[0] && cd->usersd[0]->block_list::id==dstsd.block_list::id)
					clif_dispchat(*cd,sd->fd);
			}
			if(dstsd.vender_id)
			{
				clif_closevendingboard(dstsd.bl,sd->fd);
			}
		}
		break;
	}
	case BL_NPC:
		if( ((struct npc_data &)bl).class_ != INVISIBLE_CLASS )
			clif_clearchar_id(*sd,bl.id,0);
		break;
	case BL_MOB:
	case BL_PET:
		clif_clearchar_id(*sd,bl.id,0);
		break;
	case BL_ITEM:
		clif_clearflooritem( ((flooritem_data&)bl),sd->fd);
		break;
	case BL_SKILL:
		clif_clearchar_skillunit( ((struct skill_unit&)bl),sd->fd);
		break;
	}
	return 0;
}
*/
int CClifPCOutsight::process(struct block_list& bl) const
{
	switch(bl.type)
	{
	case BL_PC:
	{
		struct map_session_data &dstsd = (struct map_session_data&)bl;
		if(sd.block_list::id != dstsd.block_list::id)
		{
			clif_clearchar_id(sd, dstsd.block_list::id, 0);
			clif_clearchar_id(dstsd, sd.block_list::id, 0);

			if(dstsd.disguise_id || sd.disguise_id)
			{
				clif_clearchar_id(sd, dstsd.block_list::id|FLAG_DISGUISE, 0);
				clif_clearchar_id(dstsd, sd.block_list::id|FLAG_DISGUISE, 0);
			}

			if(dstsd.chatID){
				chat_data *cd;
				cd=(chat_data*)map_id2bl(dstsd.chatID);
				if(cd->usersd[0] && cd->usersd[0]->block_list::id==dstsd.block_list::id)
					clif_dispchat(*cd,sd.fd);
			}
			if(dstsd.vender_id)
			{
				clif_closevendingboard(dstsd,sd.fd);
			}
		}
		break;
	}
	case BL_NPC:
		if( ((struct npc_data &)bl).class_ != INVISIBLE_CLASS )
			clif_clearchar_id(sd,bl.id,0);
		break;
	case BL_MOB:
	case BL_PET:
	case BL_HOM:
		clif_clearchar_id(sd,bl.id,0);
		break;
	case BL_ITEM:
		clif_clearflooritem( ((flooritem_data&)bl),sd.fd);
		break;
	case BL_SKILL:
		clif_clearchar_skillunit( ((struct skill_unit&)bl),sd.fd);
		break;
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_mobinsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	struct mob_data *md;

	nullpo_retr(0, ap);

	md=va_arg(ap,struct mob_data*);
	if(bl.type==BL_PC && session[sd.fd] != NULL) {
		clif_getareachar_mob(sd,*md);
	}

	return 0;
}
*/
int CClifMobInsight::process(struct block_list& bl) const
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	if(bl.type==BL_PC && session_isActive(sd.fd))
		clif_getareachar_mob(sd, md);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_moboutsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd = (struct map_session_data&)bl;
	struct mob_data *md;

	nullpo_retr(0, ap);
	nullpo_retr(0, md=va_arg(ap,struct mob_data*));

	if(bl.type==BL_PC && session[sd.fd] != NULL)
	{
		clif_clearchar_id(sd,md->block_list::id,0);
	}

	return 0;
}
*/
int CClifMobOutsight::process(struct block_list& bl) const
{
	struct map_session_data &sd = (struct map_session_data&)bl;
	if(bl.type==BL_PC && session_isActive(sd.fd) )
		clif_clearchar_id(sd, md.block_list::id, 0);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_petinsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	struct pet_data *pd;

	nullpo_retr(0, ap);

	pd=va_arg(ap,struct pet_data*);
	if(bl.type==BL_PC && session[sd.fd] != NULL) {
		clif_getareachar_pet(sd,*pd);
	}

	return 0;
}
*/
int CClifPetInsight::process(struct block_list& bl) const
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	if( bl.type==BL_PC && session_isActive(sd.fd) )
		clif_getareachar_pet(sd,pd);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
/*
int clif_petoutsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	struct pet_data *pd;

	nullpo_retr(0, ap);
	nullpo_retr(0, pd=va_arg(ap,struct pet_data*));

	if(bl.type==BL_PC && session[sd.fd] != NULL) {
		clif_clearchar_id(sd,pd->block_list::id,0);
	}
	return 0;
}
*/
int CClifPetOutsight::process(struct block_list& bl) const
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	if( bl.type==BL_PC && session_isActive(sd.fd) )
		clif_clearchar_id(sd, pd.block_list::id,0);
	return 0;
}



/*==========================================
// npc walking [Valaris]
 *------------------------------------------
 */
/*
int clif_npcinsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	struct npc_data *nd;

	nullpo_retr(0, ap);

	nd=va_arg(ap,struct npc_data*);
	if(bl.type==BL_PC && session[sd.fd] != NULL) {
		clif_getareachar_npc(sd,*nd);
	}

	return 0;
}
*/
int CClifNpcInsight::process(struct block_list& bl) const
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	if( bl.type==BL_PC && session_isActive(sd.fd) )
		clif_getareachar_npc(sd,nd);
	return 0;
}
/*==========================================
// npc walking [Valaris]
 *------------------------------------------
 */
/*
int clif_npcoutsight(struct block_list &bl,va_list &ap)
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	struct npc_data *nd;

	nullpo_retr(0, ap);
	nullpo_retr(0, nd=va_arg(ap,struct npc_data*));

	if(bl.type==BL_PC && session[sd.fd] != NULL) {
		clif_clearchar_id(sd,nd->block_list::id,0);
	}

	return 0;
}
*/
int CClifNpcOutsight::process(struct block_list& bl) const
{
	struct map_session_data &sd=(struct map_session_data&)bl;
	if( bl.type==BL_PC && session_isActive(sd.fd) )
		clif_clearchar_id(sd, nd.block_list::id,0);
	return 0;
}






int CClifInsight::process(struct block_list& bl) const
{
	if (bl.id == tbl.id)
		return 0;

	map_session_data *sd = (tsd) ? tsd : bl.get_sd();
	block_list* object   = (tsd) ? &bl : &tbl;

	if( sd && session_isActive(sd->fd) )
	{	//Tell sd that object entered into his view
		switch(object->type)
		{
		case BL_ITEM:
			clif_getareachar_item(*sd,*(struct flooritem_data*)object);
			break;
		case BL_SKILL:
			clif_getareachar_skillunit(*sd,*(skill_unit*)object);
			break;
		case BL_NPC:
			clif_getareachar_npc(*sd,*(npc_data*)object);
			break;
		case BL_PET:
			clif_getareachar_pet(*sd,*(pet_data*)object);
			break;
		case BL_HOM:
			clif_getareachar(*sd,*(homun_data*)object);
			break;
		case BL_MOB:
			clif_getareachar_mob(*sd,*(mob_data*)object);
			break;
		case BL_PC:
			if(sd->block_list::id != object->id)
			{
				clif_getareachar_pc(*sd, *(map_session_data*)object);
				clif_getareachar_pc(*(map_session_data*)object, *sd);
			}
			break;
		}
	}
	return 0;
}
int CClifOutsight::process(struct block_list& bl) const
{
	if(bl.id == tbl.id)
		return 0;

	map_session_data *sd = (tsd) ? tsd : bl.get_sd();
	block_list* object   = (tsd) ? &bl : &tbl;

	if( sd && session_isActive(sd->fd) )
	{	//sd has lost sight of object
		switch(object->type)
		{
		case BL_PC:
		{
			map_session_data *osd = object->get_sd();
			clif_clearchar_id(*osd,  sd->block_list::id, 0);
			clif_clearchar_id( *sd, osd->block_list::id, 0);

			if( osd->disguise_id )
				clif_clearchar_id( *sd, osd->block_list::id|FLAG_DISGUISE, 0);
			if( sd->disguise_id )
				clif_clearchar_id(*osd,  sd->block_list::id|FLAG_DISGUISE, 0);

			if(sd->chatID)
			{
				struct chat_data *cd;
				cd=(struct chat_data*)map_id2bl(sd->chatID);
				if(cd->usersd[0]==osd)
					clif_dispchat(*cd,osd->fd);
			}
			if(osd->chatID)
			{
				struct chat_data *cd=(struct chat_data*)map_id2bl(osd->chatID);
				if(cd->usersd[0]==sd)
					clif_dispchat(*cd,sd->fd);
			}

			if(sd->vender_id)
				clif_closevendingboard(*sd,tsd->fd);
			if(osd->vender_id)
				clif_closevendingboard(*osd,sd->fd);
		
			break;
		}
		case BL_ITEM:
			clif_clearflooritem( *((struct flooritem_data*)object),sd->fd);
			break;
		case BL_SKILL:
			clif_clearchar_skillunit( *((struct skill_unit*)object),sd->fd);
			break;
		default:
			clif_clearchar_id(*sd, object->id, 0);
			break;
		}
	}

	return 0;
}










/*==========================================
 *
 *------------------------------------------
 */
int clif_skillinfo(struct map_session_data &sd, unsigned short skillid, short type, short range)
{
	int fd, inf2;
	unsigned short id;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	id=sd.status.skill[skillid].id;

	WFIFOW(fd,0)=0x147;
	WFIFOW(fd,2) = id;
	if(type < 0)
		WFIFOW(fd,4) = skill_get_inf(id);
	else
		WFIFOW(fd,4) = type;
	WFIFOW(fd,6) = 0;
	WFIFOW(fd,8) = sd.status.skill[skillid].lv;
	WFIFOW(fd,10) = skill_get_sp(id,sd.status.skill[skillid].lv);
	if(range < 0) {
		range = skill_get_range(id,sd.status.skill[skillid].lv);
		if(range < 0)
			range = status_get_range(&sd) - (range + 1);
	}
	WFIFOW(fd,12)= range;

	memset(WFIFOP(fd,14), 0, 24);
	//safestrcpy((char*)WFIFOP(fd,14), skill_get_name(id), 24);
	inf2 = skill_get_inf2(id);
	if( ((!(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) && !(inf2&INF2_WEDDING_SKILL)) ||
		(config.gm_allskill && sd.isGM() >= config.gm_allskill) )
		//WFIFOB(fd,38)= (sd.status.skill[skillid].lv < skill_get_max(id) && sd.status.skill[skillid].flag ==0 )? 1:0;
		WFIFOB(fd,38)= (sd.status.skill[skillid].lv < skill_tree_get_max(id, sd.status.class_) && sd.status.skill[skillid].flag ==0 )? 1:0;
	else
		WFIFOB(fd,38) = 0;
	WFIFOSET(fd,packet_len_table[0x147]);

	return 0;
}

/*==========================================
 * スキルリストを送信する
 *------------------------------------------
 */
int clif_skillinfoblock(struct map_session_data &sd)
{
	int fd;
	int i,c,len=4,id,range, inf2;

	fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x10f;
	for ( i = c = 0; i < MAX_SKILL; ++i){
		if( (id=sd.status.skill[i].id)!=0 ){
			WFIFOW(fd,len  ) = id;
			WFIFOW(fd,len+2) = skill_get_inf(id);
			WFIFOW(fd,len+4) = 0;
			WFIFOW(fd,len+6) = sd.status.skill[i].lv;
			WFIFOW(fd,len+8) = skill_get_sp(id,sd.status.skill[i].lv);
			range = skill_get_range(id,sd.status.skill[i].lv);
			if(range < 0)
				range = status_get_range(&sd) - (range + 1);
			WFIFOW(fd,len+10)= range;
			memset(WFIFOP(fd,len+12),0,24);
			inf2 = skill_get_inf2(id);
			if( ((!(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) && !(inf2&INF2_WEDDING_SKILL)) ||
				(config.gm_allskill && sd.isGM() >= config.gm_allskill) )
				//WFIFOB(fd,len+36)= (sd.status.skill[i].lv < skill_get_max(id) && sd.status.skill[i].flag ==0 )? 1:0;
				WFIFOB(fd,len+36)= (sd.status.skill[i].lv < skill_tree_get_max(id, sd.status.class_) && sd.status.skill[i].flag ==0 )? 1:0;
			else
				WFIFOB(fd,len+36) = 0;
			len+=37;
			c++;
		}
	}
	WFIFOW(fd,2)=len;
	WFIFOSET(fd,len);

	return 0;
}

/*==========================================
 * スキル割り振り通知
 *------------------------------------------
 */
int clif_skillup(struct map_session_data &sd, unsigned short skill_num)
{
	int range,fd;

	fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skill_num;
	WFIFOW(fd,4) = sd.status.skill[skill_num].lv;
	WFIFOW(fd,6) = skill_get_sp(skill_num,sd.status.skill[skill_num].lv);
	range = skill_get_range(skill_num,sd.status.skill[skill_num].lv);
	if(range < 0)
		range = status_get_range(&sd) - (range + 1);
	WFIFOW(fd,8) = range;
	//WFIFOB(fd,10) = (sd.status.skill[skill_num].lv < skill_get_max(sd.status.skill[skill_num].id)) ? 1 : 0;
	WFIFOB(fd,10) = (sd.status.skill[skill_num].lv < skill_tree_get_max(sd.status.skill[skill_num].id, sd.status.class_)) ? 1 : 0;
	WFIFOSET(fd,packet_len_table[0x10e]);

	return 0;
}

/*==========================================
 * スキル詠唱エフェクトを送信する
 *------------------------------------------
 */
int clif_skillcasting(struct block_list &bl,uint32 src_id,uint32 dst_id,unsigned short dst_x,unsigned short dst_y,unsigned short skill_id,uint32 casttime)
{
	unsigned char buf[32];
	WBUFW(buf,0) = 0x13e;
	WBUFL(buf,2) = src_id;
	WBUFL(buf,6) = dst_id;
	WBUFW(buf,10) = dst_x;
	WBUFW(buf,12) = dst_y;
	WBUFW(buf,14) = skill_id;//魔法詠唱スキル
	WBUFL(buf,16) = skill_get_pl(skill_id);//属性
	WBUFL(buf,20) = casttime;//skill詠唱時間
	return clif_send(buf,packet_len_table[0x13e], &bl, AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_skillcastcancel(struct block_list &bl)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x1b9;
	WBUFL(buf,2) = bl.id;
	return clif_send(buf,packet_len_table[0x1b9], &bl, AREA);
}

/*==========================================
 * スキル詠唱失敗
 *------------------------------------------
 */
int clif_skill_fail(struct map_session_data &sd,unsigned short skill_id,unsigned char type,unsigned short btype)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	// reset all variables [celest]
	sd.skillx    = sd.skilly      = 0xFFFF;
	sd.skillid   = sd.skilllv     = 0xFFFF;
	sd.skillitem = sd.skillitemlv = 0xFFFF;

	if(type==0x4 && (config.display_delay_skill_fail==0 || sd.state.nodelay)){
		return 0;
	}

	WFIFOW(fd,0) = 0x110;
	WFIFOW(fd,2) = skill_id;
	WFIFOW(fd,4) = btype;
	WFIFOW(fd,6) = 0;
	WFIFOB(fd,8) = 0;
	WFIFOB(fd,9) = type;
	WFIFOSET(fd,packet_len_table[0x110]);

	return 0;
}

/*==========================================
 * スキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type)
{
	unsigned char buf[64];
	struct status_change *sc_data;

	sc_data = status_get_sc_data(&dst);

	if(type != 5 && dst.type == BL_PC && ((struct map_session_data &)dst).state.infinite_endure)
		type = 9;
	if(sc_data) {
		if(type != 5 && sc_data[SC_ENDURE].timer != -1)
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1 && damage > 0)
			damage = damage*(5+sc_data[SC_HALLUCINATION].val1.num) + rand()%100;
	}

#if PACKETVER < 3
	WBUFW(buf,0)=0x114;
	WBUFW(buf,2)=skill_id;
	if(src->type==BL_PC && ((struct map_session_data &)src).disguise_id)
		WBUFL(buf,4)=src.id | FLAG_DISGUISE;
	else 
		WBUFL(buf,4)=src.id;
	if(dst->type==BL_PC && ((struct map_session_data *)dst)->disguise_id)
		WBUFL(buf,8)=dst.id | FLAG_DISGUISE;
	else
		WBUFL(buf,8)=dst.id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFW(buf,24)=damage;
	WBUFW(buf,26)=skill_lv;
	WBUFW(buf,28)=div;
	WBUFB(buf,30)=(type>0)?type:skill_get_hit(skill_id);
	return clif_send(buf,packet_len_table[0x114],&src,AREA);
#else
	WBUFW(buf,0)=0x1de;
	WBUFW(buf,2)=skill_id;
	if(src.type==BL_PC && ((struct map_session_data &)src).disguise_id)
		WBUFL(buf,4)=src.id|FLAG_DISGUISE;
	else 
		WBUFL(buf,4)=src.id;
	if(dst.type==BL_PC && ((struct map_session_data &)dst).disguise_id)
		WBUFL(buf,8)=dst.id|FLAG_DISGUISE;
	else
		WBUFL(buf,8)=dst.id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFL(buf,24)=damage;
	WBUFW(buf,28)=skill_lv;
	WBUFW(buf,30)=div;
	WBUFB(buf,32)=(type>0)?type:skill_get_hit(skill_id);
	return clif_send(buf,packet_len_table[0x1de],&src,AREA);
#endif
}

/*==========================================
 * 吹き飛ばしスキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage2(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type)
{
	unsigned char buf[64];
	struct status_change *sc_data;

	sc_data = status_get_sc_data(&dst);

	if(type != 5 && dst.type == BL_PC && ((struct map_session_data &)dst).state.infinite_endure)
		type = 9;
	if(sc_data) {
		if(type != 5 && sc_data[SC_ENDURE].timer != -1)
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1 && damage > 0)
			damage = damage*(5+sc_data[SC_HALLUCINATION].val1.num) + rand()%100;
	}

	WBUFW(buf,0)=0x115;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src.id;
	WBUFL(buf,8)=dst.id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFW(buf,24)=dst.x;
	WBUFW(buf,26)=dst.y;
	WBUFW(buf,28)=damage;
	WBUFW(buf,30)=skill_lv;
	WBUFW(buf,32)=div;
	WBUFB(buf,34)=(type>0)?type:skill_get_hit(skill_id);
	return clif_send(buf,packet_len_table[0x115],&src,AREA);
}

/*==========================================
 * 支援/回復スキルエフェクト
 *------------------------------------------
 */
int clif_skill_nodamage(struct block_list &src,struct block_list &dst,unsigned short skill_id,unsigned short heal,unsigned char fail)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x11a;
	WBUFW(buf,2)=skill_id;
	WBUFW(buf,4)=(heal > 0x7fff)? 0x7fff:heal;
	WBUFL(buf,6)=dst.id;
	WBUFL(buf,10)=src.id;
	WBUFB(buf,14)=fail;
	return clif_send(buf,packet_len_table[0x11a],&src,AREA);
}

/*==========================================
 * 場所スキルエフェクト
 *------------------------------------------
 */
int clif_skill_poseffect(struct block_list &src,unsigned short skill_id,unsigned short val,unsigned short x,unsigned short y,unsigned long tick)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x117;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src.id;
	WBUFW(buf,8)=val;
	WBUFW(buf,10)=x;
	WBUFW(buf,12)=y;
	WBUFL(buf,14)=tick;
	return clif_send(buf,packet_len_table[0x117],&src,AREA);
}

/*==========================================
 * 場所スキルエフェクト表示
 *------------------------------------------
 */
int clif_skill_setunit(struct skill_unit &unit)
{
	unsigned char buf[128];
	struct block_list *bl = (unit.group) ? map_id2bl(unit.group->src_id) : NULL;

#if PACKETVER < 3
	memset(WBUFP(buf, 0),0,packet_len_table[0x11f]);
	WBUFW(buf, 0)=0x11f;
	WBUFL(buf, 2)=unit.block_list::id;
	WBUFL(buf, 6)=(unit.group) ? unit.group->src_id : unit.block_list::id;
	WBUFW(buf,10)=unit.block_list::x;
	WBUFW(buf,12)=unit.block_list::y;
	WBUFB(buf,14)=(unit.group) ? unit.group->unit_id : unit.block_list::id;
	WBUFB(buf,15)=0;
	return clif_send(buf,packet_len_table[0x11f],&unit->bl,AREA);
#else
	memset(WBUFP(buf, 0),0,packet_len_table[0x1c9]);
	WBUFW(buf, 0)=0x1c9;
	WBUFL(buf, 2)=unit.block_list::id;
	WBUFL(buf, 6)=(unit.group) ? unit.group->src_id : unit.block_list::id;
	WBUFW(buf,10)=unit.block_list::x;
	WBUFW(buf,12)=unit.block_list::y;
	WBUFB(buf,14)=(unit.group) ? unit.group->unit_id : unit.block_list::id;
	if(unit.group && unit.group->unit_id==0xb0)
	{
		WBUFL(buf,15)=1;
		WBUFL(buf,16)=1;
		memcpy(WBUFP(buf,17),unit.group->valstring.c_str(),80);
	}
	else
	{
		WBUFB(buf,15)=1;
		WBUFL(buf,15+1)=0;						//1-4調べた限り固定
		WBUFL(buf,15+5)=0;						//5-8調べた限り固定
												//9-12マップごとで一定の77-80とはまた違う4バイトのかなり大きな数字
		WBUFL(buf,15+13)=unit.block_list::y - 0x12;		//13-16ユニットのY座標-18っぽい(Y:17でFF FF FF FF)
		WBUFL(buf,15+17)=0x004f37dd;			//17-20調べた限り固定(0x1b2で0x004fdbddだった)
		WBUFL(buf,15+21)=0x0012f674;			//21-24調べた限り固定
		WBUFL(buf,15+25)=0x0012f664;			//25-28調べた限り固定
		WBUFL(buf,15+29)=0x0012f654;			//29-32調べた限り固定
		WBUFL(buf,15+33)=0x77527bbc;			//33-36調べた限り固定
												//37-39
		WBUFB(buf,15+40)=0x2d;					//40調べた限り固定
		WBUFL(buf,15+41)=0;						//41-44調べた限り0固定
		WBUFL(buf,15+45)=0;						//45-48調べた限り0固定
		WBUFL(buf,15+49)=0;						//49-52調べた限り0固定
		WBUFL(buf,15+53)=0x0048d919;			//53-56調べた限り固定(0x01b2で0x00495119だった)
		WBUFL(buf,15+57)=0x0000003e;			//57-60調べた限り固定
		WBUFL(buf,15+61)=0x0012f66c;			//61-64調べた限り固定
												//65-68
												//69-72
		if(bl) WBUFL(buf,15+73)=bl->y;			//73-76術者のY座標
		WBUFL(buf,15+77)=unit.block_list::m;				//77-80マップIDかなぁ？かなり2バイトで足りそうな数字
		WBUFB(buf,15+81)=0xaa;					//81終端文字0xaa
	}
	return clif_send(buf,packet_len_table[0x1c9],&unit,AREA);
#endif
}
/*==========================================
 * 場所スキルエフェクト削除
 *------------------------------------------
 */
int clif_skill_delunit(struct skill_unit &unit)
{
	unsigned char buf[16];
	WBUFW(buf, 0)=0x120;
	WBUFL(buf, 2)=unit.block_list::id;
	return clif_send(buf,packet_len_table[0x120],&unit,AREA);
}
/*==========================================
 * ワープ場所選択
 *------------------------------------------
 */
int clif_skill_warppoint(struct map_session_data &sd,unsigned short skill_id,
	const char *map1,const char *map2,const char *map3,const char *map4)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x11c;
	WFIFOW(fd,2)=skill_id;
	memcpy(WFIFOP(fd, 4),map1,16);
	memcpy(WFIFOP(fd,20),map2,16);
	memcpy(WFIFOP(fd,36),map3,16);
	memcpy(WFIFOP(fd,52),map4,16);
	WFIFOSET(fd,packet_len_table[0x11c]);
	return 0;
}
/*==========================================
 * メモ応答
 *------------------------------------------
 */
int clif_skill_memo(struct map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x11e;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x11e]);
	return 0;
}
int clif_skill_teleportmessage(struct map_session_data &sd,unsigned short flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x189;
	WFIFOW(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x189]);
	return 0;
}

/*==========================================
 * モンスター情報
 *------------------------------------------
 */
int clif_skill_estimation(struct map_session_data &sd,struct block_list &dst)
{
	struct mob_data &md = (struct mob_data &)dst;
	unsigned char buf[64];
	int i;

	if(dst.type!=BL_MOB )
		return 0;

	WBUFW(buf, 0)=0x18c;
	WBUFW(buf, 2)=md.get_viewclass();
	WBUFW(buf, 4)=md.level;
	WBUFW(buf, 6)=mob_db[md.class_].size;
	WBUFL(buf, 8)=md.hp;
	WBUFW(buf,12)=status_get_def2(&md);
	WBUFW(buf,14)=mob_db[md.class_].race;
	WBUFW(buf,16)=status_get_mdef2(&md) - (mob_db[md.class_].vit>>1);
	WBUFW(buf,18)=status_get_elem_type(&md);
	for(i=0;i<9; ++i)
		WBUFB(buf,20+i)= battle_attr_fix(100,i+1,md.def_ele);

	if(sd.status.party_id>0)

		return clif_send(buf,packet_len_table[0x18c],&sd,PARTY_AREA);
	
	else if( session_isActive(sd.fd) )
	{
		memcpy(WFIFOP(sd.fd,0),buf,packet_len_table[0x18c]);
		WFIFOSET(sd.fd,packet_len_table[0x18c]);
	}
	return 0;
}
/*==========================================
 * アイテム合成可能リスト
 *------------------------------------------
 */
int clif_skill_produce_mix_list(struct map_session_data &sd,int trigger)
{
	int i,c,view;
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd, 0)=0x18d;

	for(i=0,c=0;i<MAX_SKILL_PRODUCE_DB; ++i){
		if( skill_can_produce_mix(sd,skill_produce_db[i].nameid,trigger) ){
			if((view = itemdb_viewid(skill_produce_db[i].nameid)) > 0)
				WFIFOW(fd,c*8+ 4)= view;
			else
				WFIFOW(fd,c*8+ 4)= skill_produce_db[i].nameid;
			WFIFOW(fd,c*8+ 6)= 0x0012;
			WFIFOL(fd,c*8+ 8)= sd.status.char_id;
			c++;
		}
	}
	WFIFOW(fd, 2)=c*8+8;
	WFIFOSET(fd,WFIFOW(fd,2));
	if(c > 0) sd.state.produce_flag = 1;
	return 0;
}

/*==========================================
 * 状態異常アイコン/メッセージ表示
 *------------------------------------------
 */
int clif_status_change(struct block_list &bl,unsigned short type,unsigned char flag)
{
	unsigned char buf[16];
	if (type >= MAX_STATUSCHANGE) //Status changes above this value are not displayed on the client. [Skotlex]
		return 0;
	WBUFW(buf,0)=0x0196;
	WBUFW(buf,2)=type;
	WBUFL(buf,4)=bl.id;
	WBUFB(buf,8)=flag;
	return clif_send(buf,packet_len_table[0x196],&bl,AREA);
}

/*==========================================
 * Send message (modified by [Yor])
 *------------------------------------------
 */
int clif_displaymessage(int fd, const char* mes)
{
	// invalid pointer?
	nullpo_retr(-1, mes);
	
	//Console [Wizputer]
	if (fd == 0)
		ShowConsole(CL_BOLD"%s\n"CL_NORM, mes);

	else if(mes && session_isActive(fd) ) {

		size_t len=strlen(mes);
		if( len > 0 ) { // don't send a void message (it's not displaying on the client chat). @help can send void line.
			WFIFOW(fd,0) = 0x8e;
			WFIFOW(fd,2) = 4 + len+1; // 4 + len + NULL terminate
			memcpy(WFIFOP(fd,4), mes, len+1);
			WFIFOSET(fd, 4 + len+1);
		}
	}
	return 0;
}

/*==========================================
 * 天の声を送信する
 *------------------------------------------
 */
int clif_GMmessage(struct block_list *bl, const char* mes, size_t len, int flag)
{
	if(mes && *mes && len)
	{
		unsigned char buf[512];
		size_t lp = (flag & 0x10) ? 8 : 4;
		if( len > 512-lp ) len = 512-lp;
		
		WBUFW(buf,0) = 0x9a;
		WBUFW(buf,2) = len + lp;
		WBUFL(buf,4) = 0x65756c62;
		memcpy(WBUFP(buf,lp), mes, len);
		buf[511] = 0; //force EOS
		flag &= 0x07;
		flag =	(flag == 1) ? ALL_SAMEMAP : 
				(flag == 2) ? AREA :
				(flag == 3) ? SELF :
				ALL_CLIENT;
		if( flag==ALL_CLIENT || bl )
			clif_send(buf, len + lp, bl,flag);
	}
	return 0;
}

/*==========================================
 * グローバルメッセージ
 *------------------------------------------
 */
int clif_GlobalMessage(struct block_list &bl,const char *message, size_t len)
{
	if(message && *message)
	{
		unsigned char buf[128];
		len++;
		if(len>119) len=119;

		WBUFW(buf,0)=0x8d;
		WBUFW(buf,2)=len+9;
		WBUFL(buf,4)=bl.id;
		memcpy(WBUFP(buf,8),message,len+1);
		buf[sizeof(buf)-1]=0;// force eos
		return clif_send(buf,len+9,&bl,AREA_CHAT_WOC);
	}
	return 0;
}

/*==========================================
 * HPSP回復エフェクトを送信する
 *------------------------------------------
 */
int clif_heal(int fd,unsigned short type,unsigned short val)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x13d;
	WFIFOW(fd,2)=type;
	WFIFOW(fd,4)=val;
	WFIFOSET(fd,packet_len_table[0x13d]);

	return 0;
}

/*==========================================
 * 復活する
 *------------------------------------------
 */
int clif_resurrection(struct block_list &bl,unsigned short type)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x148;
	WBUFL(buf,2)=bl.id;
	WBUFW(buf,6)=type;

	clif_send(buf,packet_len_table[0x148],&bl,type==1 ? AREA : AREA_WOS);

	if(bl.type==BL_PC && ((struct map_session_data &)bl).disguise_id)
		clif_spawnpc( ((struct map_session_data &)bl) );

	return 0;
}

/*==========================================
 * PVP実装？（仮）
 *------------------------------------------
 */
int clif_set0199(int fd,unsigned short type)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x199;
	WFIFOW(fd,2)=type;
	WFIFOSET(fd,packet_len_table[0x199]);

	return 0;
}

/*==========================================
 * PVP実装？(仮)
 *------------------------------------------
 */
int clif_pvpset(struct map_session_data &sd,uint32 pvprank,uint32 pvpnum,int type)
{
	if( !session_isActive(sd.fd) )
		return 0;

	if(maps[sd.block_list::m].flag.nopvp)
		return 0;

	if(type == 2) {
		WFIFOW(sd.fd,0) = 0x19a;
		WFIFOL(sd.fd,2) = sd.block_list::id;
		if(pvprank<=0)
			pc_calc_pvprank(sd);
		WFIFOL(sd.fd,6) = pvprank;
		WFIFOL(sd.fd,10) = pvpnum;
		WFIFOSET(sd.fd,packet_len_table[0x19a]);
	} else {
		unsigned char buf[32];

		WBUFW(buf,0) = 0x19a;
		WBUFL(buf,2) = sd.block_list::id;
		if(sd.status.option&0x46)
			WBUFL(buf,6) = 0xFFFFFFFF;
		else
			if(pvprank<=0)
				pc_calc_pvprank(sd);
			WBUFL(buf,6) = pvprank;
		WBUFL(buf,10) = pvpnum;
		if(!type)
			clif_send(buf,packet_len_table[0x19a],&sd,AREA);
		else
			clif_send_same_map(sd,buf,packet_len_table[0x19a]);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_send0199(unsigned short map,unsigned short type)
{
	struct block_list bl;
	unsigned char buf[16];

	bl.m = map;
	WBUFW(buf,0)=0x199;
	WBUFW(buf,2)=type;
	clif_send_same_map(bl,buf,packet_len_table[0x199]);
	return 0;
}

/*==========================================
 * 精錬エフェクトを送信する
 *------------------------------------------
 */
int clif_refine(int fd, struct map_session_data &sd,unsigned short fail,unsigned short index,unsigned short val)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x188;
	WFIFOW(fd,2)=fail;
	WFIFOW(fd,4)=index+2;
	WFIFOW(fd,6)=val;
	WFIFOSET(fd,packet_len_table[0x188]);

	return 0;
}

/*==========================================
 * Wisp/page is transmitted to the destination player
 *------------------------------------------
 */
int clif_wis_message(int fd, const char *nick, const char *mes, size_t mes_len)
{	 // R 0097 <len>.w <nick>.24B <message>.?B
//	ShowMessage("clif_wis_message(%d, %s, %s)\n", fd, nick, mes);
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x97;
	WFIFOW(fd,2) = mes_len + 24 + 4;
	memcpy(WFIFOP(fd,4), nick, 24);
	memcpy(WFIFOP(fd,28), mes, mes_len);
	WFIFOSET(fd,mes_len + 24 + 4);
	return 0;
}

/*==========================================
 * The transmission result of Wisp/page is transmitted to the source player
 *------------------------------------------
 */
int clif_wis_end(int fd, unsigned short flag)
{	 // R 0098 <type>.B: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x98;
	WFIFOW(fd,2) = flag;
	WFIFOSET(fd,packet_len_table[0x98]);
	return 0;
}

/*==========================================
 * キャラID名前引き結果を送信する
 *------------------------------------------
 */
int clif_solved_charname(struct map_session_data &sd, uint32 char_id)
{
	char *p= map_charid2nick(char_id);
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(p!=NULL){
		WFIFOW(fd,0)=0x194;
		WFIFOL(fd,2)=char_id;
		memcpy(WFIFOP(fd,6), p,24 );
		WFIFOSET(fd,packet_len_table[0x194]);
	}else{
		map_reqchariddb(sd,char_id);
		chrif_searchcharid(char_id);
	}
	return 0;
}

/*==========================================
 * カードの挿入可能リストを返す
 *------------------------------------------
 */
int clif_use_card(struct map_session_data &sd,unsigned short idx)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if( idx<MAX_INVENTORY && sd.inventory_data[idx])
	{
		size_t i,j;
		unsigned short c;
		unsigned short ep=sd.inventory_data[idx]->equip;

		for(i=c=0;i<MAX_INVENTORY; ++i)
		{
			if(sd.inventory_data[i] == NULL)
				continue;
			if(sd.inventory_data[i]->type!=4 && sd.inventory_data[i]->type!=5)	// 武器防具じゃない
				continue;
			if(sd.status.inventory[i].card[0]==0x00ff)	// 製造武器
				continue;
			if(sd.status.inventory[i].card[0]==0xff00 || sd.status.inventory[i].card[0]==0x00fe)
				continue;
			if(sd.status.inventory[i].identify==0 )	// 未鑑定
				continue;

			if((sd.inventory_data[i]->equip & ep)==0)	// 装備個所が違う
				continue;
			if(sd.inventory_data[i]->type==4 && ep==32)	// 盾カードと両手武器
				continue;

			for(j=0;j<sd.inventory_data[i]->flag.slot; ++j){
				if( sd.status.inventory[i].card[j]==0 )
					break;
			}
			if(j>=sd.inventory_data[i]->flag.slot)	// すでにカードが一杯
				continue;

			WFIFOW(fd,4+c*2)=i+2;
			c++;
		}
		WFIFOW(fd,0)=0x017b;
		WFIFOW(fd,2)=4+c*2;
		WFIFOSET(fd,4+c*2);
	}

	return 0;
}
/*==========================================
 * カードの挿入終了
 *------------------------------------------
 */
int clif_insert_card(struct map_session_data &sd,unsigned short idx_equip,unsigned short idx_card,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x17d;
	WFIFOW(fd,2)=idx_equip+2;
	WFIFOW(fd,4)=idx_card+2;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,packet_len_table[0x17d]);
	return 0;
}

/*==========================================
 * 鑑定可能アイテムリスト送信
 *------------------------------------------
 */
int clif_item_identify_list(struct map_session_data &sd)
{
	size_t i,c;

	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x177;
	for(i=c=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].nameid > 0 && sd.status.inventory[i].identify!=1)
		{
			WFIFOW(fd,c*2+4)=i+2;
			c++;
		}
	}
	if(c > 0)
	{
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,c*2+4);
	}
	return 0;
}

/*==========================================
 * 鑑定結果
 *------------------------------------------
 */
int clif_item_identified(struct map_session_data &sd,unsigned short idx,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x179;
	WFIFOW(fd, 2)=idx+2;
	WFIFOB(fd, 4)=flag;
	WFIFOSET(fd,packet_len_table[0x179]);
	return 0;
}

/*==========================================
 * 修理可能アイテムリスト送信
 *------------------------------------------
 */
int clif_item_repair_list(struct map_session_data &sd)
{
	int i, c;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x1fc;
	for(i=c=0;i<MAX_INVENTORY && c<0xFF; ++i)
	{
		//if ((nameid = dstsd->status.inventory[i].nameid) > 0 && dstsd->status.inventory[i].attribute == 1){
		if( sd.status.inventory[i].nameid > 0 && sd.status.inventory[i].attribute == 1)
		{
			WFIFOW(fd,c*13+4) = i;	// これが1fdで返ってくる･･･コレしか返ってこない！！
			WFIFOW(fd,c*13+6) = sd.status.inventory[i].nameid;
			WFIFOL(fd,c*13+8) = sd.status.char_id;
			WFIFOL(fd,c*13+12)= sd.status.char_id;
			WFIFOB(fd,c*13+16)= (unsigned char)c;
			c++;
		}
	}
	if(c > 0)
	{
		WFIFOW(fd,2) = c*13+4;
		WFIFOSET(fd,c*13+4);
		sd.state.produce_flag = 1;
	} else
		clif_skill_fail(sd,sd.skillid,0,0);

	return 0;
}

int clif_repaireffect(struct map_session_data &sd, unsigned short nameid, unsigned char flag)
{
	unsigned short view;

	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) = 0x1fe;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd, 2) = view;
	else
		WFIFOW(fd, 2) = nameid;
	WFIFOB(fd, 4) = flag;
	WFIFOSET(fd, 5);

	//if(sd->repair_target && sd != sd->repair_target && flag==0){	// 成功したら相手にも通知
	//	clif_repaireffect(sd->repair_target,nameid,flag);
	//	sd->repair_target=NULL;
	//}

	return 0;
}

/*==========================================
 * Weapon Refining - Taken from jAthena
 *------------------------------------------
 */
int clif_item_refine_list(struct map_session_data &sd)
{
/*	int i,c;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x177; // temporarily use same packet as clif_item_identify
	for(i=c=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].nameid > 0 && itemdb_type(sd.status.inventory[i].nameid)==4){
			WFIFOW(fd,c*2+4)=i+2;
			c++;
		}
	}
	if(c > 0) {
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
*/
	int i,c;
	int fd=sd.fd;
	int skilllv;
	int wlv;
	int refine_item[5];

	skilllv = pc_checkskill(sd,WS_WEAPONREFINE);

	refine_item[0] = -1;
	refine_item[1] = pc_search_inventory(sd,1010);
	refine_item[2] = pc_search_inventory(sd,1011);
	refine_item[3] = refine_item[4] = pc_search_inventory(sd,984);

	WFIFOW(fd,0)=0x221;
	for(i=c=0;i<MAX_INVENTORY; ++i)
	{
		if( sd.status.inventory[i].nameid > 0 && sd.status.inventory[i].refine < skilllv &&
			sd.status.inventory[i].identify==1 && (wlv=itemdb_wlv(sd.status.inventory[i].nameid)) >=1 &&
			refine_item[wlv]!=-1 && !(sd.status.inventory[i].equip&0x0022))
		{
			WFIFOW(fd,c*13+ 4)=i+2;
			WFIFOW(fd,c*13+ 6)=sd.status.inventory[i].nameid;
			WFIFOW(fd,c*13+ 8)=0; //TODO: Wonder what are these for? Perhaps ID of weapon's crafter if any?
			WFIFOW(fd,c*13+10)=0;
			WFIFOB(fd,c*13+12)=c;
			c++;
		}
	}
	WFIFOW(fd,2)=c*13+4;

	WFIFOSET(fd,WFIFOW(fd,2));
	sd.state.produce_flag = 1;

	return 0;
}
/*==========================================
 * アイテムによる一時的なスキル効果
 *------------------------------------------
 */
int clif_item_skill(struct map_session_data &sd,unsigned short skillid,unsigned short skilllv,const char *name)
{
	int range,fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x147;
	WFIFOW(fd, 2)=skillid;
	WFIFOW(fd, 4)=skill_get_inf(skillid);
	WFIFOW(fd, 6)=0;
	WFIFOW(fd, 8)=skilllv;
	WFIFOW(fd,10)=skill_get_sp(skillid,skilllv);
	range = skill_get_range(skillid,skilllv);
	if(range < 0)
		range = status_get_range(&sd) - (range + 1);
	WFIFOW(fd,12)=range;
	safestrcpy((char*)WFIFOP(fd,14),name,24);
	WFIFOB(fd,38)=0;
	WFIFOSET(fd,packet_len_table[0x147]);
	return 0;
}

/*==========================================
 * カートにアイテム追加
 *------------------------------------------
 */
int clif_cart_additem(struct map_session_data &sd,unsigned short n,uint32 amount,unsigned char fail)
{
	int view,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf=WFIFOP(fd,0);
	if(n>=MAX_CART || sd.status.cart[n].nameid>=MAX_ITEMS)
		return 1;

	WBUFW(buf,0)=0x124;
	WBUFW(buf,2)=n+2;
	WBUFL(buf,4)=amount;
	if((view = itemdb_viewid(sd.status.cart[n].nameid)) > 0)
		WBUFW(buf,8)=view;
	else
		WBUFW(buf,8)=sd.status.cart[n].nameid;
	WBUFB(buf,10)=sd.status.cart[n].identify;
	WBUFB(buf,11)=sd.status.cart[n].attribute;
	WBUFB(buf,12)=sd.status.cart[n].refine;

	clif_cards_to_buffer( WBUFP(buf,13), sd.status.cart[n] );

	WFIFOSET(fd,packet_len_table[0x124]);
	return 0;
}

/*==========================================
 * カートからアイテム削除
 *------------------------------------------
 */
int clif_cart_delitem(struct map_session_data &sd,unsigned short n,uint32 amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x125;
	WFIFOW(fd,2)=n+2;
	WFIFOL(fd,4)=amount;

	WFIFOSET(fd,packet_len_table[0x125]);

	return 0;
}

/*==========================================
 * カートのアイテムリスト
 *------------------------------------------
 */
int clif_cart_itemlist(struct map_session_data &sd)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0x123;
	for(i=0,n=0;i<MAX_CART; ++i){
		if(sd.status.cart[i].nameid<=0)
			continue;
		id = itemdb_exists(sd.status.cart[i].nameid);
		if(!id || itemdb_isSingleStorage(*id))
			continue;
		WBUFW(buf,n*10+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=sd.status.cart[i].nameid;
		WBUFB(buf,n*10+8)=id->getType();
		WBUFB(buf,n*10+9)=sd.status.cart[i].identify;
		WBUFW(buf,n*10+10)=sd.status.cart[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1ef;
	for(i=0,n=0;i<MAX_CART; ++i){
		if(sd.status.cart[i].nameid<=0)
			continue;
		id = itemdb_exists(sd.status.cart[i].nameid);
		if(!id || itemdb_isSingleStorage(*id))
			continue;
		WBUFW(buf,n*18+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=sd.status.cart[i].nameid;
		WBUFB(buf,n*18+8)=id->getType();
		WBUFB(buf,n*18+9)=sd.status.cart[i].identify;
		WBUFW(buf,n*18+10)=sd.status.cart[i].amount;
		WBUFW(buf,n*18+12)=0;
		clif_cards_to_buffer( WBUFP(buf,n*18+14), sd.status.cart[i] );
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 * カートの装備品リスト
 *------------------------------------------
 */
int clif_cart_equiplist(struct map_session_data &sd)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0x122;
	for(i=0,n=0;i<MAX_INVENTORY; ++i){
		if(sd.status.cart[i].nameid<=0)
			continue;
		id = itemdb_exists(sd.status.cart[i].nameid);
		if(!id || !itemdb_isSingleStorage(*id))
			continue;
		WBUFW(buf,n*20+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=sd.status.cart[i].nameid;
		WBUFB(buf,n*20+8)=id->getType();
		WBUFB(buf,n*20+9)=sd.status.cart[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=sd.status.cart[i].equip;
		WBUFB(buf,n*20+14)=sd.status.cart[i].attribute;
		WBUFB(buf,n*20+15)=sd.status.cart[i].refine;
		clif_cards_to_buffer( WBUFP(buf,n*20+16), sd.status.cart[i] );
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
int clif_openvendingreq(struct map_session_data &sd, unsigned short num)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x12d;
	WFIFOW(fd,2)=num;
	WFIFOSET(fd,packet_len_table[0x12d]);

	return 0;
}

/*==========================================
 * 露店看板表示
 *------------------------------------------
 */
int clif_showvendingboard(struct block_list &bl,const char *message,int fd)
{
	unsigned char buf[128];

	WBUFW(buf,0)=0x131;
	WBUFL(buf,2)=bl.id;
	memcpy((char*)WBUFP(buf,6),message,80);
	if(fd && session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0),buf,packet_len_table[0x131]);
		WFIFOSET(fd,packet_len_table[0x131]);
	}else{
		clif_send(buf,packet_len_table[0x131],&bl,AREA_WOS);
	}
	return 0;
}

/*==========================================
 * 露店看板消去
 *------------------------------------------
 */
int clif_closevendingboard(struct block_list &bl,int fd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x132;
	WBUFL(buf,2)=bl.id;
	if(fd && session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0),buf,packet_len_table[0x132]);
		WFIFOSET(fd,packet_len_table[0x132]);
	}else{
		clif_send(buf,packet_len_table[0x132],&bl,AREA_WOS);
	}

	return 0;
}
/*==========================================
 * 露店アイテムリスト
 *------------------------------------------
 */
int clif_vendinglist(struct map_session_data &sd,uint32 id,struct vending vending[])
{
	struct item_data *data;
	int i,n,index,fd;
	struct map_session_data *vsd;
	unsigned char *buf;

	nullpo_retr(0, vsd=map_id2sd(id));

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	
	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0x133;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<vsd->vend_num; ++i){
		if(vending[i].amount<=0)
			continue;
		WBUFL(buf,8+n*22)=vending[i].value;
		WBUFW(buf,12+n*22)=vending[i].amount;
		WBUFW(buf,14+n*22)=(index=vending[i].index)+2;
		if(vsd->status.cart[index].nameid <= 0 || vsd->status.cart[index].amount <= 0)
			continue;
		data = itemdb_search(vsd->status.cart[index].nameid);
		WBUFB(buf,16+n*22)=data->getType();
		if(data->view_id > 0)
			WBUFW(buf,17+n*22)=data->view_id;
		else
			WBUFW(buf,17+n*22)=vsd->status.cart[index].nameid;
		WBUFB(buf,19+n*22)=vsd->status.cart[index].identify;
		WBUFB(buf,20+n*22)=vsd->status.cart[index].attribute;
		WBUFB(buf,21+n*22)=vsd->status.cart[index].refine;
		clif_cards_to_buffer( WBUFP(buf,n*22+22), vsd->status.cart[index] );
		n++;
	}
	if(n > 0){
		WBUFW(buf,2)=8+n*22;
		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return 0;
}

/*==========================================
 * 露店アイテム購入失敗
 *------------------------------------------
*/
int clif_buyvending(struct map_session_data &sd,unsigned short index,unsigned short amount,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x135;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOB(fd,6)=fail;
	WFIFOSET(fd,packet_len_table[0x135]);

	return 0;
}

/*==========================================
 * 露店開設成功
 *------------------------------------------
*/
int clif_openvending(struct map_session_data &sd,uint32 id,struct vending vending[])
{
	struct item_data *data;
	int i,n,index,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) || !vending)
		return 0;

	buf = WFIFOP(fd,0);

	WBUFW(buf,0)=0x136;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<sd.vend_num; ++i){
		if (sd.vend_num > 2+pc_checkskill(sd,MC_VENDING)) return 0;
		WBUFL(buf,8+n*22)=vending[i].value;
		WBUFW(buf,12+n*22)=(index=vending[i].index)+2;
		WBUFW(buf,14+n*22)=vending[i].amount;
		if(sd.status.cart[index].nameid <= 0 || sd.status.cart[index].amount <= 0 || sd.status.cart[index].identify==0 ||
			sd.status.cart[index].attribute==1) // Prevent unidentified and broken items from being sold [Valaris]
			continue;
		data = itemdb_search(sd.status.cart[index].nameid);
		WBUFB(buf,16+n*22)=data->getType();
		if(data->view_id > 0)
			WBUFW(buf,17+n*22)=data->view_id;
		else
			WBUFW(buf,17+n*22)=sd.status.cart[index].nameid;
		WBUFB(buf,19+n*22)=sd.status.cart[index].identify;
		WBUFB(buf,20+n*22)=sd.status.cart[index].attribute;
		WBUFB(buf,21+n*22)=sd.status.cart[index].refine;
		clif_cards_to_buffer( WBUFP(buf,n*22+22), sd.status.cart[index] );
		n++;
	}
	if(n > 0){
		WBUFW(buf,2)=8+n*22;
		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return n;
}

/*==========================================
 * 露店アイテム販売報告
 *------------------------------------------
*/
int clif_vendingreport(struct map_session_data &sd,unsigned short index,unsigned short amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x137;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOSET(fd,packet_len_table[0x137]);

	return 0;
}

/*==========================================
 * パーティ作成完了
 *------------------------------------------
 */
int clif_party_created(struct map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xfa;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0xfa]);
	return 0;
}
/*==========================================
 * パーティ情報送信
 *------------------------------------------
 */
int clif_party_info(struct party &p,int fd)
{
	unsigned char buf[1024];
	int i,c, size;
	struct map_session_data *sd=NULL;

	WBUFW(buf,0)=0xfb;
	memcpy(WBUFP(buf,4),p.name,24);
	for(i=c=0;i<MAX_PARTY; ++i)
	{
		struct party_member &m = p.member[i];
		if(m.account_id)
		{
			if(sd==NULL) sd=m.sd;
			WBUFL(buf,28+c*46)=m.account_id;
			memcpy(WBUFP(buf,28+c*46+ 4),m.name,24);
			mapname2buffer(WBUFP(buf,28+c*46+28),m.mapname,16);
			WBUFB(buf,28+c*46+44)=(m.leader)?0:1;
			WBUFB(buf,28+c*46+45)=(m.online)?0:1;
			c++;
		}
	}
	size = 28+c*46;
	WBUFW(buf,2)=size;
	if(fd>=0 && session_isActive(fd) )
	{	// fdが設定されてるならそれに送る
		memcpy(WFIFOP(fd,0),buf,size);
		WFIFOSET(fd,size);
		return 0;
	}
	if(sd!=NULL)
		clif_send(buf, size, sd, PARTY);
	return 0;
}
/*==========================================
 * パーティ勧誘
 *------------------------------------------
 */
int clif_party_invite(struct map_session_data &sd,struct map_session_data &tsd)
{
	int fd;
	struct party *p;

	fd=tsd.fd;
	if( !session_isActive(fd) )
		return 0;

	if( (p=party_search(sd.status.party_id))==NULL )
		return 0;

	WFIFOW(fd,0)=0xfe;
	WFIFOL(fd,2)=sd.status.account_id;
	memcpy(WFIFOP(fd,6),p->name,24);
	WFIFOSET(fd,packet_len_table[0xfe]);
	return 0;
}

/*==========================================
 * パーティ勧誘結果
 *------------------------------------------
 */
int clif_party_inviteack(struct map_session_data &sd,const char *nick,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xfd;
	memcpy(WFIFOP(fd,2),nick,24);
	WFIFOB(fd,26)=flag;
	WFIFOSET(fd,packet_len_table[0xfd]);
	return 0;
}

/*==========================================
 * パーティ設定送信
 * flag & 0x001=exp変更ミス
 *        0x010=item変更ミス
 *        0x100=一人にのみ送信
 *------------------------------------------
 */
int clif_party_option(struct party &p,struct map_session_data *sd, int flag)
{
	unsigned char buf[16];
//	if(config.etc_log)
//		ShowMessage("clif_party_option: %d %d %d\n",p->exp,p->item,flag);
	if(sd==NULL && flag==0){
		int i;
		for(i=0;i<MAX_PARTY; ++i)
			if((sd=map_id2sd(p.member[i].account_id))!=NULL)
				break;
	}
	WBUFW(buf,0)=0x101;
	WBUFW(buf,2)=((flag&0x01)?2:p.expshare);
	WBUFW(buf,4)=((flag&0x10)?2:p.itemshare);
	if(flag==0)
		clif_send(buf, packet_len_table[0x101], sd, PARTY);
	else if( sd && session_isActive(sd->fd) ) {
		memcpy(WFIFOP(sd->fd,0),buf,packet_len_table[0x101]);
		WFIFOSET(sd->fd,packet_len_table[0x101]);
	}
	return 0;
}
/*==========================================
 * パーティ脱退（脱退前に呼ぶこと）
 *------------------------------------------
 */
int clif_party_leaved(struct party &p,struct map_session_data *sd,uint32 account_id,const char *name,unsigned char flag)
{
	unsigned char buf[64];
	int i;

	WBUFW(buf,0)=0x105;
	WBUFL(buf,2)=account_id;
	memcpy(WBUFP(buf,6),name,24);
	WBUFB(buf,30)=flag&0x0f;

	if(sd==NULL && (flag&0xf0)==0)
	{
		for(i=0;i<MAX_PARTY; ++i)
			if((sd=p.member[i].sd)!=NULL)
				break;
		if(flag==0)
			clif_send(buf,packet_len_table[0x105], sd,PARTY);
	}
	else if( sd && session_isActive(sd->fd) )
	{
		memcpy(WFIFOP(sd->fd,0),buf,packet_len_table[0x105]);
		WFIFOSET(sd->fd,packet_len_table[0x105]);
	}
	return 0;
}
/*==========================================
 * パーティメッセージ送信
 *------------------------------------------
 */
int clif_party_message(struct party &p,uint32 account_id, const char *mes,size_t len)
{
	struct map_session_data *sd=NULL;
	int i;

	// test for someone in the party
	for(i=0;i<MAX_PARTY; ++i)
		if((sd=p.member[i].sd)!=NULL)
			break;

	if(sd!=NULL)
	{
		unsigned char buf[1024];
		WBUFW(buf,0)=0x109;
		WBUFW(buf,2)=len+8;
		WBUFL(buf,4)=account_id;
		memcpy(WBUFP(buf,8),mes,len);
		clif_send(buf,len+8,sd,PARTY);
	}
	return 0;
}
/*==========================================
 * パーティ座標通知
 *------------------------------------------
 */
int clif_party_xy(struct party &p,struct map_session_data &sd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=sd.block_list::x;
	WBUFW(buf,8)=sd.block_list::y;
	return clif_send(buf,packet_len_table[0x107],&sd,PARTY_SAMEMAP_WOS);
}
/*==========================================
 * Remove dot from minimap 
 *------------------------------------------
*/
int clif_party_xy_remove(struct map_session_data &sd)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=0xFFFF;
	WBUFW(buf,8)=0xFFFF;
	clif_send(buf, packet_len_table[0x107], &sd, PARTY_SAMEMAP_WOS);
	return 0;
}


/*==========================================
 * パーティHP通知
 *------------------------------------------
 */
int clif_party_hp(struct party &p,struct map_session_data &sd)
{
	unsigned char buf[16];
	uint32 max_hp = sd.status.max_hp;
	uint32 cur_hp = (sd.status.hp>sd.status.max_hp) ? sd.status.max_hp : sd.status.hp;
	while(max_hp > 0x7fff)
		(max_hp >>= 1), (cur_hp<=1) || (cur_hp >>= 1);
	if(!cur_hp && sd.status.hp) cur_hp=1;
	WBUFW(buf,0)=0x106;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=(unsigned short)cur_hp;
	WBUFW(buf,8)=(unsigned short)max_hp;
	return clif_send(buf,packet_len_table[0x106],&sd,PARTY_AREA_WOS);
}
/*==========================================
 * GMへ場所とHP通知
 *------------------------------------------
 */
int clif_hpmeter(struct map_session_data &sd)
{
	struct map_session_data *sd2;
	unsigned char buf1[16];
	unsigned char buf2[16];
	size_t i;
	int x0, y0, x1, y1;
	uint32 max_hp = sd.status.max_hp;
	uint32 cur_hp = (sd.status.hp>sd.status.max_hp) ? sd.status.max_hp : sd.status.hp;
	while(max_hp > 0x7fff)
		(max_hp >>= 1), (cur_hp<=1) || (cur_hp >>= 1);
	WBUFW(buf1,0) = 0x107;
	WBUFL(buf1,2) = sd.block_list::id;
	WBUFW(buf1,6) = sd.block_list::x;
	WBUFW(buf1,8) = sd.block_list::y;

	WBUFW(buf2,0) = 0x106;
	WBUFL(buf2,2) = sd.status.account_id;
	WBUFW(buf2,6) = (unsigned short)cur_hp;
	WBUFW(buf2,8) = (unsigned short)max_hp;


	// some kind of self written foreach_client
	x0 = sd.block_list::x - AREA_SIZE;
	y0 = sd.block_list::y - AREA_SIZE;
	x1 = sd.block_list::x + AREA_SIZE;
	y1 = sd.block_list::y + AREA_SIZE;

	for (i = 0; i < fd_max; ++i)
	{
		if( session[i] && 
			(sd2 = (struct map_session_data*)session[i]->user_session) &&  
			sd2->block_list::m == sd.block_list::m &&
			sd2->block_list::x > x0 && sd2->block_list::x < x1 &&
			sd2->block_list::y > y0 && sd2->block_list::y < y1 &&
			sd2->isGM() >= config.disp_hpmeter &&
			sd2->isGM() >= sd.isGM() &&
			&sd != sd2 && 
			sd2->state.auth)
		{
			memcpy(WFIFOP(i,0), buf1, packet_len_table[0x107]);
			WFIFOSET (i, packet_len_table[0x107]);

			memcpy (WFIFOP(i,0), buf2, packet_len_table[0x106]);
			WFIFOSET (i, packet_len_table[0x106]);
		}
	}
	return 0;
}
/*==================================================
 * Update monster hp view if it has changed [Celest]
 *--------------------------------------------------
 */
int clif_update_mobhp(struct mob_data &md)
{
	unsigned char buf[128];
	char mobhp[64];

	memset(buf,0,102);
	WBUFW(buf,0) = 0x95;
	WBUFL(buf,2) = md.block_list::id;

	memcpy(WBUFP(buf,6), md.name, 24);
	snprintf(mobhp, sizeof(mobhp), "hp: %ld/%ld", (unsigned long)md.hp, (unsigned long)mob_db[md.class_].max_hp);

	WBUFW(buf, 0) = 0x195;
	memcpy(WBUFP(buf,30), mobhp, 24);
	WBUFL(buf,54) = 0;
	WBUFL(buf,78) = 0;
	return clif_send(buf,packet_len_table[0x195],&md,AREA);
}
/*==========================================
 * パーティ場所移動（未使用）
 *------------------------------------------
 */
int clif_party_move(struct party &p,struct map_session_data &sd,unsigned char online)
{
	unsigned char buf[128];

	WBUFW(buf, 0) = 0x104;
	WBUFL(buf, 2) = sd.status.account_id;
	WBUFL(buf, 6) = 0;
	WBUFW(buf,10) = sd.block_list::x;
	WBUFW(buf,12) = sd.block_list::y;
	WBUFB(buf,14) = !online;
	memcpy(WBUFP(buf,15),p.name,24);
	memcpy(WBUFP(buf,39),sd.status.name,24);
	mapname2buffer(WBUFP(buf,63),maps[sd.block_list::m].mapname,16);
	return clif_send(buf,packet_len_table[0x104],&sd,PARTY);
}
/*==========================================
 * 攻撃するために移動が必要
 *------------------------------------------
 */
int clif_movetoattack(struct map_session_data &sd,struct block_list &bl)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x139;
	WFIFOL(fd, 2)=bl.id;
	WFIFOW(fd, 6)=bl.x;
	WFIFOW(fd, 8)=bl.y;
	WFIFOW(fd,10)=sd.block_list::x;
	WFIFOW(fd,12)=sd.block_list::y;
	WFIFOW(fd,14)=sd.attackrange;
	WFIFOSET(fd,packet_len_table[0x139]);
	return 0;
}
/*==========================================
 * 製造エフェクト
 *------------------------------------------
 */
int clif_produceeffect(struct map_session_data &sd,unsigned short flag,unsigned short nameid)
{
	unsigned short view;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	// 名前の登録と送信を先にしておく
	if( map_charid2nick(sd.status.char_id)==NULL )
		map_addchariddb(sd.status.char_id,sd.status.name);
	clif_solved_charname(sd,sd.status.char_id);

	WFIFOW(fd, 0)=0x18f;
	WFIFOW(fd, 2)=flag;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd, 4)=view;
	else
		WFIFOW(fd, 4)=nameid;
	WFIFOSET(fd,packet_len_table[0x18f]);
	return 0;
}

// pet
int clif_catch_process(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x19e;
	WFIFOSET(fd,packet_len_table[0x19e]);

	return 0;
}

int clif_pet_rulet(struct map_session_data &sd,unsigned char data)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1a0;
	WFIFOB(fd,2)=data;
	WFIFOSET(fd,packet_len_table[0x1a0]);

	return 0;
}

/*==========================================
 * pet卵リスト作成
 *------------------------------------------
 */
int clif_sendegg(struct map_session_data &sd)
{
	//R 01a6 <len>.w <index>.w*
	int i,n=0,fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(agit_flag && config.pet_no_gvg && maps[sd.block_list::m].flag.gvg)
	{	//Disable pet hatching in GvG grounds during Guild Wars [Skotlex]
		clif_displaymessage(fd, "Pets are not allowed in Guild Wars.");
		return 0;
	}

	WFIFOW(fd,0)=0x1a6;
	if(sd.status.pet_id <= 0) {
		for(i=0,n=0;i<MAX_INVENTORY; ++i){
			if(sd.status.inventory[i].nameid<=0 || sd.inventory_data[i] == NULL ||
			   sd.inventory_data[i]->type!=7 ||
			   sd.status.inventory[i].amount<=0)
				continue;
			WFIFOW(fd,n*2+4)=i+2;
			n++;
		}
	}
	WFIFOW(fd,2)=4+n*2;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int clif_send_petdata(struct map_session_data &sd,unsigned char type,uint32 param)
{
	int fd=sd.fd;
	if( !session_isActive(fd) || !sd.pd)
		return 0;

	WFIFOW(fd,0)=0x1a4;
	WFIFOB(fd,2)=type;
	WFIFOL(fd,3)=sd.pd->block_list::id;
	WFIFOL(fd,7)=param;
	WFIFOSET(fd,packet_len_table[0x1a4]);

	return 0;
}

int clif_send_petstatus(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1a2;
	memcpy(WFIFOP(fd,2),sd.pet.name,24);
	WFIFOB(fd,26)=(config.pet_rename == 1)? 0:sd.pet.rename_flag;
	WFIFOW(fd,27)=sd.pet.level;
	WFIFOW(fd,29)=sd.pet.hungry;
	WFIFOW(fd,31)=sd.pet.intimate;
	WFIFOW(fd,33)=sd.pet.equip_id;
	WFIFOSET(fd,packet_len_table[0x1a2]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pet_emotion(struct pet_data &pd, uint32 param)
{
	unsigned char buf[16];
	memset(buf,0,packet_len_table[0x1aa]);

	WBUFW(buf,0)=0x1aa;
	WBUFL(buf,2)=pd.block_list::id;
	if( param >= 100 && pd.petDB.talk_convert_class) {
		if(pd.petDB.talk_convert_class < 0)
			return 0;
		else if(pd.petDB.talk_convert_class > 0) {
			param -= (pd.class_ - 100)*100;
			param += (pd.petDB.talk_convert_class - 100)*100;
		}
	}
	WBUFL(buf,6)=param;

	return clif_send(buf,packet_len_table[0x1aa],&pd,AREA);
}

int clif_pet_performance(struct block_list &bl,uint32 param)
{
	unsigned char buf[16];

	memset(buf,0,packet_len_table[0x1a4]);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=4;
	WBUFL(buf,3)=bl.id;
	WBUFL(buf,7)=param;

	return clif_send(buf,packet_len_table[0x1a4],&bl,AREA);
}

int clif_pet_equip(struct pet_data &pd,unsigned short nameid)
{
	unsigned char buf[16];
	unsigned short view;

	memset(buf,0,packet_len_table[0x1a4]);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=3;
	WBUFL(buf,3)=pd.block_list::id;
	if((view = itemdb_viewid(nameid)) > 0)
		WBUFL(buf,7)=view;
	else
		WBUFL(buf,7)=nameid;

	return clif_send(buf,packet_len_table[0x1a4],&pd,AREA);
}

int clif_pet_food(struct map_session_data &sd,unsigned short foodid,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1a3;
	WFIFOB(fd,2)=fail;
	WFIFOW(fd,3)=foodid;
	WFIFOSET(fd,packet_len_table[0x1a3]);

	return 0;
}

/*==========================================
 * オートスペル リスト送信
 *------------------------------------------
 */
int clif_autospell(struct map_session_data &sd,unsigned short skilllv)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x1cd;

	if(skilllv>0 && pc_checkskill(sd,MG_NAPALMBEAT)>0)
		WFIFOL(fd,2)= MG_NAPALMBEAT;
	else
		WFIFOL(fd,2)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_COLDBOLT)>0)
		WFIFOL(fd,6)= MG_COLDBOLT;
	else
		WFIFOL(fd,6)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_FIREBOLT)>0)
		WFIFOL(fd,10)= MG_FIREBOLT;
	else
		WFIFOL(fd,10)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_LIGHTNINGBOLT)>0)
		WFIFOL(fd,14)= MG_LIGHTNINGBOLT;
	else
		WFIFOL(fd,14)= 0x00000000;
	if(skilllv>4 && pc_checkskill(sd,MG_SOULSTRIKE)>0)
		WFIFOL(fd,18)= MG_SOULSTRIKE;
	else
		WFIFOL(fd,18)= 0x00000000;
	if(skilllv>7 && pc_checkskill(sd,MG_FIREBALL)>0)
		WFIFOL(fd,22)= MG_FIREBALL;
	else
		WFIFOL(fd,22)= 0x00000000;
	if(skilllv>9 && pc_checkskill(sd,MG_FROSTDIVER)>0)
		WFIFOL(fd,26)= MG_FROSTDIVER;
	else
		WFIFOL(fd,26)= 0x00000000;

	WFIFOSET(fd,packet_len_table[0x1cd]);
	return 0;
}

/*==========================================
 * ディボーションの青い糸
 *------------------------------------------
 */
int clif_devotion(struct map_session_data &sd,uint32 target_id)
{
	unsigned char buf[56];
	int n;

	WBUFW(buf,0)=0x1cf;
	WBUFL(buf,2)=sd.block_list::id;
//	WBUFL(buf,6)=target;
	for(n=0;n<5;++n)
		WBUFL(buf,6+4*n)=sd.dev.val2[n];
//		WBUFL(buf,10+4*n)=0;
	WBUFB(buf,26)=8;
	WBUFB(buf,27)=0;

	return clif_send(buf,packet_len_table[0x1cf],&sd,AREA);
}
int clif_marionette(struct block_list &src, struct block_list *target)
{
	unsigned char buf[64];
	int n;

	WBUFW(buf,0)=0x1cf;
	WBUFL(buf,2)=src.id;
	for(n=0;n<5;++n)
		WBUFL(buf,6+4*n)=0;
	if (target) //The target goes on the second slot.
		WBUFL(buf,6+4) = target->id;
	WBUFB(buf,26)=8;
	WBUFB(buf,27)=0;

	clif_send(buf,packet_len_table[0x1cf],&src,AREA);
	return 0;
}

/*==========================================
 * 氣球
 *------------------------------------------
 */
int clif_spiritball(struct map_session_data &sd)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x1d0;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.spiritball;
	return clif_send(buf,packet_len_table[0x1d0],&sd,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_combo_delay(struct block_list &bl,uint32 wait)
{
	unsigned char buf[32];


	WBUFW(buf,0)=0x1d2;
	WBUFL(buf,2)=bl.id;
	WBUFL(buf,6)=wait;
	return clif_send(buf,packet_len_table[0x1d2],&bl,AREA);
}
/*==========================================
 *白刃取り
 *------------------------------------------
 */
int clif_bladestop(struct block_list &src,struct block_list &dst, uint32 _bool)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x1d1;
	WBUFL(buf,2)=src.id;
	WBUFL(buf,6)=dst.id;
	WBUFL(buf,10)=_bool;

	return clif_send(buf,packet_len_table[0x1d1],&src,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapcell(unsigned short m,unsigned short x,unsigned short y,unsigned short cell_type,int type)
{
	struct block_list bl;
	unsigned char buf[32];

	bl.m = m;
	bl.x = x;
	bl.y = y;
	WBUFW(buf,0) = 0x192;
	WBUFW(buf,2) = x;
	WBUFW(buf,4) = y;
	WBUFW(buf,6) = cell_type;
	mapname2buffer(WBUFP(buf,8),maps[m].mapname,16);
	if(!type)
		clif_send(buf,packet_len_table[0x192],&bl, AREA);
	else
		clif_send_same_map(bl,buf,packet_len_table[0x192]);
	return 0;
}

/*==========================================
 * MVPエフェクト
 *------------------------------------------
 */
int clif_mvp_effect(struct map_session_data &sd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x10c;
	WBUFL(buf,2)=sd.block_list::id;
	return clif_send(buf,packet_len_table[0x10c],&sd,AREA);
}
/*==========================================
 * MVPアイテム所得
 *------------------------------------------
 */
int clif_mvp_item(struct map_session_data &sd,unsigned short nameid)
{
	unsigned short view;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x10a;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd,2)=view;
	else
		WFIFOW(fd,2)=nameid;
	WFIFOSET(fd,packet_len_table[0x10a]);
	return 0;
}
/*==========================================
 * MVP経験値所得
 *------------------------------------------
 */
int clif_mvp_exp(struct map_session_data &sd, uint32 exp)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x10b;
	WFIFOL(fd,2)=exp;
	WFIFOSET(fd,packet_len_table[0x10b]);
	return 0;
}

/*==========================================
 * ギルド作成可否通知
 *------------------------------------------
 */
int clif_guild_created(struct map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x167;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x167]);
	return 0;
}
/*==========================================
 * ギルド所属通知
 *------------------------------------------
 */
int clif_guild_belonginfo(struct map_session_data &sd,struct guild &g)
{
	int ps,fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	ps=guild_getposition(sd,g);

	memset(WFIFOP(fd,0),0,packet_len_table[0x16c]);
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=g.guild_id;
	WFIFOL(fd,6)=g.emblem_id;
	WFIFOL(fd,10)=g.position[ps].mode;
	memcpy(WFIFOP(fd,19),g.name,24);
	WFIFOSET(fd,packet_len_table[0x16c]);
	return 0;
}
/*==========================================
 * ギルドメンバログイン通知
 *------------------------------------------
 */
int clif_guild_memberlogin_notice(struct guild &g,uint32 idx,uint32 flag)
{
	unsigned char buf[64];

	if( idx>=MAX_GUILD )
		return 0;

	WBUFW(buf, 0)=0x16d;
	WBUFL(buf, 2)=g.member[idx].account_id;
	WBUFL(buf, 6)=g.member[idx].char_id;
	WBUFL(buf,10)=flag;
	if(g.member[idx].sd==NULL)
	{
		struct map_session_data *sd=guild_getavailablesd(g);
		if(sd!=NULL)
			clif_send(buf,packet_len_table[0x16d],sd,GUILD);
	}else
		clif_send(buf,packet_len_table[0x16d], g.member[idx].sd, GUILD_WOS);
	return 0;
}
/*==========================================
 * ギルドマスター通知(14dへの応答)
 *------------------------------------------
 */
int clif_guild_masterormember(struct map_session_data &sd)
{
	int type=0x57; // member
	struct guild *g;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g!=NULL && strcmp(g->master,sd.status.name)==0)
		type=0xd7; // master
	WFIFOW(fd,0)=0x14e;
	WFIFOL(fd,2)=type;
	WFIFOSET(fd,packet_len_table[0x14e]);
	return 0;
}
/*==========================================
 * Basic Info (Territories [Valaris])
 *------------------------------------------
 */
int clif_guild_basicinfo(struct map_session_data &sd)
{
	int fd,i,t, honour, chaos;
	struct guild *g;
	struct guild_castle *gc=NULL;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;

	// recalculate guild honour and chaos
	for(i=0,honour=0,chaos=0,t=0;i<g->max_member; ++i)
	{	// sdの設定と人数の確認
		if(g->member[i].account_id>0)
		{
			struct map_session_data *sd = map_id2sd(g->member[i].account_id);
			if (sd && sd->status.char_id == g->member[i].char_id &&
				sd->status.guild_id == g->guild_id &&
				!sd->state.waitingdisconnect)
			{
				chaos  += sd->status.chaos; //
				honour -= sd->status.karma;	// carma is counted invers, honour not
				t++;
			}
		}
	}
	g->chaos = (t) ? chaos/t : 0;
	g->honour= (t) ? honour/t : 0;


	WFIFOW(fd, 0)=0x1b6;//0x150;
	WFIFOL(fd, 2)=g->guild_id;
	WFIFOL(fd, 6)=g->guild_lv;
	WFIFOL(fd,10)=g->connect_member;
	WFIFOL(fd,14)=g->max_member;
	WFIFOL(fd,18)=g->average_lv;
	WFIFOL(fd,22)=g->exp;
	WFIFOL(fd,26)=g->next_exp;
	WFIFOL(fd,30)=0;		// tax // 上納
	WFIFOL(fd,34)=g->chaos;	// vw axis // VW（性格の悪さ？：性向グラフ左右）
	WFIFOL(fd,38)=g->honour;// rf axis // RF（正義の度合い？：性向グラフ上下）
	WFIFOL(fd,42)=0;		// number of ppl? // 人数？
	memcpy(WFIFOP(fd,46),g->name,24);
	memcpy(WFIFOP(fd,70),g->master,24);

	for(i=0,t=0;i<MAX_GUILDCASTLE; ++i){
		gc=guild_castle_search(i);
		if(!gc) continue;
			if(g->guild_id == gc->guild_id)	t++;
	}

	if     (t==1)  safestrcpy((char*)WFIFOP(fd,94),"One Castle",20);
	else if(t==2)  safestrcpy((char*)WFIFOP(fd,94),"Two Castles",20);
	else if(t==3)  safestrcpy((char*)WFIFOP(fd,94),"Three Castles",20);
	else if(t==4)  safestrcpy((char*)WFIFOP(fd,94),"Four Castles",20);
	else if(t==5)  safestrcpy((char*)WFIFOP(fd,94),"Five Castles",20);
	else if(t==6)  safestrcpy((char*)WFIFOP(fd,94),"Six Castles",20);
	else if(t==7)  safestrcpy((char*)WFIFOP(fd,94),"Seven Castles",20);
	else if(t==8)  safestrcpy((char*)WFIFOP(fd,94),"Eight Castles",20);
	else if(t==9)  safestrcpy((char*)WFIFOP(fd,94),"Nine Castles",20);
	else if(t==10) safestrcpy((char*)WFIFOP(fd,94),"Ten Castles",20);
	else if(t==11) safestrcpy((char*)WFIFOP(fd,94),"Eleven Castles",20);
	else if(t==12) safestrcpy((char*)WFIFOP(fd,94),"Twelve Castles",20);
	else if(t==13) safestrcpy((char*)WFIFOP(fd,94),"Thirteen Castles",20);
	else if(t==14) safestrcpy((char*)WFIFOP(fd,94),"Fourteen Castles",20);
	else if(t==15) safestrcpy((char*)WFIFOP(fd,94),"Fifteen Castles",20);
	else if(t==16) safestrcpy((char*)WFIFOP(fd,94),"Sixteen Castles",20);
	else if(t==17) safestrcpy((char*)WFIFOP(fd,94),"Seventeen Castles",20);
	else if(t==18) safestrcpy((char*)WFIFOP(fd,94),"Eighteen Castles",20);
	else if(t==19) safestrcpy((char*)WFIFOP(fd,94),"Nineteen Castles",20);
	else if(t==20) safestrcpy((char*)WFIFOP(fd,94),"Twenty Castles",20);
	else if(t==21) safestrcpy((char*)WFIFOP(fd,94),"Twenty One Castles",20);
	else if(t==22) safestrcpy((char*)WFIFOP(fd,94),"Twenty Two Castles",20);
	else if(t==23) safestrcpy((char*)WFIFOP(fd,94),"Twenty Three Castles",20);
	else if(t==24) safestrcpy((char*)WFIFOP(fd,94),"Twenty Four Castles",20);
	else if(t==MAX_GUILDCASTLE) safestrcpy((char*)WFIFOP(fd,94),"Total Domination",20);
	else safestrcpy((char*)WFIFOP(fd,94),"None Taken",20);

	WFIFOSET(fd,packet_len_table[WFIFOW(fd,0)]);
	clif_guild_emblem(sd,*g);	// Guild emblem vanish fix [Valaris]
	return 0;
}

/*==========================================
 * ギルド同盟/敵対情報
 *------------------------------------------
 */
int clif_guild_allianceinfo(struct map_session_data &sd)
{
	int fd,i,c;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x14c;
	for(i=c=0;i<MAX_GUILDALLIANCE; ++i){
		struct guild_alliance *a=&g->alliance[i];
		if(a->guild_id>0){
			WFIFOL(fd,c*32+4)=a->opposition;
			WFIFOL(fd,c*32+8)=a->guild_id;
			memcpy(WFIFOP(fd,c*32+12),a->name,24);
			c++;
		}
	}
	WFIFOW(fd, 2)=c*32+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ギルドメンバーリスト
 *------------------------------------------
 */
int clif_guild_memberlist(struct map_session_data &sd)
{
	int fd;
	int i,c;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;

	WFIFOW(fd, 0)=0x154;
	for(i=0,c=0;i<g->max_member; ++i){
		struct guild_member *m=&g->member[i];
		if(m->account_id==0)
			continue;
		WFIFOL(fd,c*104+ 4)=m->account_id;
		WFIFOL(fd,c*104+ 8)=m->char_id;
		WFIFOW(fd,c*104+12)=m->hair;
		WFIFOW(fd,c*104+14)=m->hair_color;
		WFIFOW(fd,c*104+16)=m->gender;
		WFIFOW(fd,c*104+18)=m->class_;
		WFIFOW(fd,c*104+20)=m->lv;
		WFIFOL(fd,c*104+22)=m->exp;
		WFIFOL(fd,c*104+26)=m->online;
		WFIFOL(fd,c*104+30)=m->position;
		memset(WFIFOP(fd,c*104+34),0,50);	// メモ？
		memcpy(WFIFOP(fd,c*104+84),m->name,24);
		c++;
	}
	WFIFOW(fd, 2)=c*104+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職名リスト
 *------------------------------------------
 */
int clif_guild_positionnamelist(struct map_session_data &sd)
{
	int i,fd;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x166;
	for(i=0;i<MAX_GUILDPOSITION; ++i){
		WFIFOL(fd,i*28+4)=i;
		memcpy(WFIFOP(fd,i*28+8),g->position[i].name,24);
	}
	WFIFOW(fd,2)=i*28+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職情報リスト
 *------------------------------------------
 */
int clif_guild_positioninfolist(struct map_session_data &sd)
{
	int i,fd;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x160;
	for(i=0;i<MAX_GUILDPOSITION; ++i){
		struct guild_position *p=&g->position[i];
		WFIFOL(fd,i*16+ 4)=i;
		WFIFOL(fd,i*16+ 8)=p->mode;
		WFIFOL(fd,i*16+12)=i;
		WFIFOL(fd,i*16+16)=p->exp_mode;
	}
	WFIFOW(fd, 2)=i*16+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職変更通知
 *------------------------------------------
 */
int clif_guild_positionchanged(struct guild &g,uint32 idx)
{
	struct map_session_data *sd;
	unsigned char buf[128];

	if( idx>=MAX_GUILD )
		return 0;

	WBUFW(buf, 0)=0x174;
	WBUFW(buf, 2)=44;
	WBUFL(buf, 4)=idx;
	WBUFL(buf, 8)=g.position[idx].mode;
	WBUFL(buf,12)=idx;
	WBUFL(buf,16)=g.position[idx].exp_mode;
	memcpy(WBUFP(buf,20),g.position[idx].name,24);
	if( (sd=guild_getavailablesd(g))!=NULL )
	{
		clif_send(buf,WBUFW(buf,2),sd,GUILD);
	}
	return 0;
}
/*==========================================
 * ギルドメンバ変更通知
 *------------------------------------------
 */
int clif_guild_memberpositionchanged(struct guild &g,uint32 idx)
{
	struct map_session_data *sd;
	unsigned char buf[64];

	if( idx>=MAX_GUILD )
		return 0;
	WBUFW(buf, 0)=0x156;
	WBUFW(buf, 2)=16;
	WBUFL(buf, 4)=g.member[idx].account_id;
	WBUFL(buf, 8)=g.member[idx].char_id;
	WBUFL(buf,12)=g.member[idx].position;
	if( (sd=guild_getavailablesd(g))!=NULL )
		clif_send(buf,WBUFW(buf,2),sd,GUILD);
	return 0;
}
/*==========================================
 * ギルドエンブレム送信
 *------------------------------------------
 */
int clif_guild_emblem(struct map_session_data &sd,struct guild &g)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(g.emblem_len<=0)
		return 0;
	WFIFOW(fd,0)=0x152;
	WFIFOW(fd,2)=g.emblem_len+12;
	WFIFOL(fd,4)=g.guild_id;
	WFIFOL(fd,8)=g.emblem_id;
	memcpy(WFIFOP(fd,12),g.emblem_data,g.emblem_len);
	WFIFOSET(fd,g.emblem_len+12);
	return 0;
}
/*==========================================
 * ギルドスキル送信
 *------------------------------------------
 */
int clif_guild_skillinfo(struct map_session_data &sd)
{
	int fd;
	int i,id,c,up=1;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd,0)=0x0162;
	WFIFOW(fd,4)=g->skill_point;
	for(i=c=0;i<MAX_GUILDSKILL; ++i){
		if(g->skill[i].id>0){
			WFIFOW(fd,c*37+ 6) = id = g->skill[i].id;
			WFIFOW(fd,c*37+ 8) = guild_skill_get_inf(id);
			WFIFOW(fd,c*37+10) = 0;
			WFIFOW(fd,c*37+12) = g->skill[i].lv;
			WFIFOW(fd,c*37+14) = skill_get_sp(id,g->skill[i].lv);
			WFIFOW(fd,c*37+16) = skill_get_range(id,g->skill[i].lv);
			memset(WFIFOP(fd,c*37+18),0,24);
			if(g->skill[i].lv < guild_skill_get_max(id)) {
				//Kafra and Guardian changed to require Approval [Sara]
				switch (g->skill[i].id)
				{
					case GD_KAFRACONTRACT:
					case GD_GUARDIANRESEARCH:
					case GD_GUARDUP:
					case GD_DEVELOPMENT:
						up = guild_checkskill(*g,GD_APPROVAL) > 0;
						break;
					case GD_LEADERSHIP:
						//Glory skill requirements -- Pretty sure correct [Sara]
						up = (config.require_glory_guild) ?
							guild_checkskill(*g,GD_GLORYGUILD) > 0 : 1;
						// what skill does it need now that glory guild was removed? [celest]
						break;
					case GD_GLORYWOUNDS:
						up = (config.require_glory_guild) ?
							guild_checkskill(*g,GD_GLORYGUILD) > 0 : 1;
						break;
					case GD_SOULCOLD:
						up = guild_checkskill(*g,GD_GLORYWOUNDS) > 0;
						break;
					case GD_HAWKEYES:
						up = guild_checkskill(*g,GD_LEADERSHIP) > 0;
						break;
					case GD_BATTLEORDER:
						up = guild_checkskill(*g,GD_APPROVAL) > 0 &&
							guild_checkskill(*g,GD_EXTENSION) >= 2;
						break;
					case GD_REGENERATION:
						up = guild_checkskill(*g,GD_EXTENSION) >= 5 &&
							guild_checkskill(*g,GD_BATTLEORDER) > 0;
						break;
					case GD_RESTORE:
						up = guild_checkskill(*g,GD_REGENERATION) >= 2;
						break;
					case GD_EMERGENCYCALL:
						up = guild_checkskill(*g,GD_GUARDIANRESEARCH) > 0 &&
							guild_checkskill(*g,GD_REGENERATION) > 0;
						break;
					case GD_GLORYGUILD:
						up = (config.require_glory_guild) ? 1 : 0;
						break;
					default:
						up = 1;
				}
			}
			else {
				up = 0;
			}
			WFIFOB(fd,c*37+42)= up;
			c++;
		}
	}
	WFIFOW(fd,2)=c*37+6;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド告知送信
 *------------------------------------------
 */
int clif_guild_notice(struct map_session_data &sd,struct guild &g)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;
 
	if(*g.mes1==0 && *g.mes2==0)
		return 0;
	WFIFOW(fd,0)=0x16f;
	memcpy(WFIFOP(fd,2),g.mes1,60);
	memcpy(WFIFOP(fd,62),g.mes2,120);
	WFIFOSET(fd,packet_len_table[0x16f]);
	return 0;
}

/*==========================================
 * ギルドメンバ勧誘
 *------------------------------------------
 */
int clif_guild_invite(struct map_session_data &sd,struct guild &g)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x16a;
	WFIFOL(fd,2)=g.guild_id;
	memcpy(WFIFOP(fd,6),g.name,24);
	WFIFOSET(fd,packet_len_table[0x16a]);
	return 0;
}
/*==========================================
 * ギルドメンバ勧誘結果
 *------------------------------------------
 */
int clif_guild_inviteack(struct map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x169;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x169]);
	return 0;
}
/*==========================================
 * ギルドメンバ脱退通知
 *------------------------------------------
 */
int clif_guild_leave(struct map_session_data &sd,const char *name,const char *mes)
{
	unsigned char buf[128];

	WBUFW(buf, 0)=0x15a;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	return clif_send(buf,packet_len_table[0x15a],&sd,GUILD);
}
/*==========================================
 * ギルドメンバ追放通知
 *------------------------------------------
 */
int clif_guild_explusion(struct map_session_data &sd,const char *name,const char *mes,uint32 account_id)
{
	unsigned char buf[128];

	WBUFW(buf, 0)=0x15c;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	memcpy(WBUFP(buf,66),"dummy",24);
	return clif_send(buf,packet_len_table[0x15c],&sd,GUILD);
}
/*==========================================
 * ギルド追放メンバリスト
 *------------------------------------------
 */
int clif_guild_explusionlist(struct map_session_data &sd)
{
	int fd;
	int i,c;
	struct guild *g;

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd,0)=0x163;
	for(i=c=0;i<MAX_GUILDEXPLUSION; ++i){
		struct guild_explusion *e=&g->explusion[i];
		if(e->account_id>0){
			memcpy(WFIFOP(fd,c*88+ 4),e->name,24);
			memcpy(WFIFOP(fd,c*88+28),e->acc,24);
			memcpy(WFIFOP(fd,c*88+52),e->mes,44);
			c++;
		}
	}
	WFIFOW(fd,2)=c*88+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ギルド会話
 *------------------------------------------
 */
int clif_guild_message(struct guild &g,uint32 account_id,const char *mes,size_t len)
{
	struct map_session_data *sd;
	unsigned char buf[512];

	if( mes && ((sd = guild_getavailablesd(g)) != NULL) )
	{
		if(len>512-4) len=512-4; // max buffer limit

	WBUFW(buf, 0) = 0x17f;
	WBUFW(buf, 2) = len + 4;
	memcpy(WBUFP(buf,4), mes, len);
		return clif_send(buf, len + 4, sd, GUILD);
	}
	return 0;
}
/*==========================================
 * ギルドスキル割り振り通知
 *------------------------------------------
 */
int clif_guild_skillup(struct map_session_data &sd,unsigned short skillid,unsigned short skilllv)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skillid;
	WFIFOW(fd,4) = skilllv;
	WFIFOW(fd,6) = skill_get_sp(skillid,skilllv);
	WFIFOW(fd,8) = skill_get_range(skillid,skilllv);
	WFIFOB(fd,10) = 1;
	WFIFOSET(fd,11);
	return 0;
}
/*==========================================
 * ギルド同盟要請
 *------------------------------------------
 */
int clif_guild_reqalliance(struct map_session_data &sd,uint32 account_id,const char *name)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x171;
	WFIFOL(fd,2)=account_id;
	memcpy(WFIFOP(fd,6),name,24);
	WFIFOSET(fd,packet_len_table[0x171]);
	return 0;
}
/*==========================================
 * ギルド同盟結果
 *------------------------------------------
 */
int clif_guild_allianceack(struct map_session_data &sd,uint32 flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x173;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x173]);
	return 0;
}
/*==========================================
 * ギルド関係解消通知
 *------------------------------------------
 */
int clif_guild_delalliance(struct map_session_data &sd,uint32 guild_id,uint32 flag)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x184;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=flag;
	WFIFOSET(fd,packet_len_table[0x184]);
	return 0;
}
/*==========================================
 * ギルド敵対結果
 *------------------------------------------
 */
int clif_guild_oppositionack(struct map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x181;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x181]);
	return 0;
}

/*==========================================
 * ギルド解散通知
 *------------------------------------------
 */
int clif_guild_broken(struct map_session_data &sd,uint32 flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x15e;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_len_table[0x15e]);
	return 0;
}

/*==========================================
 * エモーション
 *------------------------------------------
 */
int clif_emotion(struct block_list &bl,unsigned char type)
{
	unsigned char buf[8];

	WBUFW(buf,0)=0xc0;
	WBUFL(buf,2)=bl.id;
	WBUFB(buf,6)=type;
	return clif_send(buf,packet_len_table[0xc0],&bl,AREA);
}

/*==========================================
 * トーキーボックス
 *------------------------------------------
 */
int clif_talkiebox(struct block_list &bl, const char* talkie)
{
	unsigned char buf[86];

	WBUFW(buf,0)=0x191;
	WBUFL(buf,2)=bl.id;
	memcpy(WBUFP(buf,6),talkie,80);
	return clif_send(buf,packet_len_table[0x191],&bl,AREA);
}

/*==========================================
 * 結婚エフェクト
 *------------------------------------------
 */
int clif_wedding_effect(struct block_list &bl)
{
	unsigned char buf[6];

	WBUFW(buf,0) = 0x1ea;
	WBUFL(buf,2) = bl.id;
	return clif_send(buf, packet_len_table[0x1ea], &bl, AREA);
}
/*==========================================
 * あなたに逢いたい使用時名前叫び
 *------------------------------------------

int clif_callpartner(struct map_session_data &sd)
{
	unsigned char buf[26];
	char *p;

	if(sd.status.partner_id){
		WBUFW(buf,0)=0x1e6;
		p = map_charid2nick(sd.status.partner_id);
		if(p){
			memcpy(WBUFP(buf,2),p,24);
		}else{
			map_reqchariddb(*sd,sd.status.partner_id);
			chrif_searchcharid(sd.status.partner_id);
			WBUFB(buf,2) = 0;
		}
		clif_send(buf,packet_len_table[0x1e6]&sd,AREA);
	}
	return 0;
}
*/
/*==========================================
 * Adopt baby [Celest]
 *------------------------------------------
 */
int clif_adopt_process(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1f8;
	WFIFOSET(fd,packet_len_table[0x1f8]);
	return 0;
}
/*==========================================
 * Marry [DracoRPG]
 *------------------------------------------
 */
void clif_marriage_process(struct map_session_data &sd)
{
	int fd=sd.fd;
	WFIFOW(fd,0)=0x1e4;
	WFIFOSET(fd,packet_len_table[0x1e4]);
}


/*==========================================
 * Notice of divorce
 *------------------------------------------
 */
int clif_divorced(struct map_session_data &sd, const char *name)
{
	int fd=sd.fd;
	WFIFOW(fd,0)=0x205;
	memcpy(WFIFOP(fd,2), name, 24);
	WFIFOSET(fd, packet_len_table[0x205]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ReqAdopt(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1f6;
	WFIFOSET(fd, packet_len_table[0x1f6]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ReqMarriage(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	WFIFOW(fd,0)=0x1e2;
	WFIFOSET(fd, packet_len_table[0x1e2]);
	return 0;
}

/*==========================================
 * 座る
 *------------------------------------------
 */
int clif_sitting(struct map_session_data &sd)
{
	unsigned char buf[64];

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = sd.block_list::id;
	WBUFB(buf,26) = 2;
	clif_send(buf, packet_len_table[0x8a], &sd, AREA);

	if(sd.disguise_id)
	{
		WBUFL(buf, 2) = sd.block_list::id|FLAG_DISGUISE;
		clif_send(buf, packet_len_table[0x8a], &sd, AREA);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_disp_onlyself(struct map_session_data &sd, const char *mes)
{
	if(mes)
	{
		unsigned char buf[512];
		int len = strlen(mes)+1;
		if(len>512-4) len=512-4; // max buffer limit

		WBUFW(buf, 0) = 0x17f;
		WBUFW(buf, 2) = len+4;
		memcpy(WBUFP(buf,4), mes, len);

		clif_send_self(sd,buf, len+4);
	}
	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */

int clif_GM_kickack(struct map_session_data &sd, uint32 id)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xcd;
	WFIFOL(fd,2) = id;
	WFIFOSET(fd, packet_len_table[0xcd]);
	return 0;
}

int clif_GM_kick(struct map_session_data &sd,struct map_session_data &tsd,int type)
{
	if(type)
		clif_GM_kickack(sd,tsd.status.account_id);
	tsd.opt1 = tsd.opt2 = 0;

	if( !session_isActive(tsd.fd) )
	return 0;

	WFIFOW(tsd.fd,0) = 0x18b;
	WFIFOW(tsd.fd,2) = 0;
	WFIFOSET(tsd.fd,packet_len_table[0x18b]);
	session_SetWaitClose(tsd.fd, 5000);

	return 0;
}

int clif_GM_silence(struct map_session_data &sd, struct map_session_data &tsd, int type)
{
	int fd = tsd.fd;
	if( !session_isActive(fd) )
		return 0;
	
	WFIFOW(fd,0) = 0x14b;
	WFIFOB(fd,2) = 0;
	memcpy(WFIFOP(fd,3), sd.status.name, 24);
	WFIFOSET(fd, packet_len_table[0x14b]);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */

int clif_timedout(struct map_session_data &sd)
{
	ShowInfo("%sCharacter with Account ID '"CL_WHITE"%d"CL_RESET"' timed out.\n", (sd.isGM())?"GM ":"", sd.block_list::id);
	map_quit(sd);
	clif_authfail(sd,3); // Even if player is not on we still send anyway
	session_Remove(sd.fd); // Set session to EOF
	return 0;
}

/*==========================================
 * Wis拒否許可応答
 *------------------------------------------
 */
int clif_wisexin(struct map_session_data &sd,int type,int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd1;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len_table[0xd1]);

	return 0;
}
/*==========================================
 * Wis全拒否許可応答
 *------------------------------------------
 */
int clif_wisall(struct map_session_data &sd,int type,int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd2;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_len_table[0xd2]);

	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------
 */
int clif_soundeffect(struct map_session_data &sd,struct block_list &bl,const char *name,unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1d3;
	memcpy(WFIFOP(fd,2),name,24);
	WFIFOB(fd,26)=type;
	WFIFOL(fd,27)=0;
	WFIFOL(fd,31)=bl.id;
	WFIFOSET(fd,packet_len_table[0x1d3]);

	return 0;
}

int clif_soundeffectall(struct block_list &bl, const char *name, unsigned char type)
{
	unsigned char buf[64];

	WBUFW(buf,0)=0x1d3;
	memcpy(WBUFP(buf,2), name, 24);
	WBUFB(buf,26)=type;
	WBUFL(buf,27)=0;
	WBUFL(buf,31)=bl.id;
	return clif_send(buf, packet_len_table[0x1d3], &bl, AREA);
}

// displaying special effects (npcs, weather, etc) [Valaris]
int clif_specialeffect(const block_list &bl, uint32 type, int flag)
{
	unsigned char buf[64];
	memset(buf, 0, packet_len_table[0x1f3]);

	WBUFW(buf,0) = 0x1f3;
	if(bl.type==BL_PC && ((struct map_session_data &)bl).disguise_id)
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
	else
		WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = type;

	switch (flag) {
	case 3:
		clif_send_all(buf, packet_len_table[0x1f3]);
		break;
	case 2:
		clif_send_same_map(bl, buf, packet_len_table[0x1f3]);
		break;
	case 1:
		clif_send_self(bl, buf, packet_len_table[0x1f3]);
		break;
	default:
		clif_send(buf, packet_len_table[0x1f3], &bl, AREA);
	}

	return 0;
}

// refresh the client's screen, getting rid of any effects
int clif_refresh(struct map_session_data &sd)
{
	clif_changemap(sd,sd.mapname,sd.block_list::x,sd.block_list::y);
	return 0;
}
/*------------------------------------------
 * @me command by lordalfa
 *------------------------------------------
*/
int clif_disp_overhead(struct map_session_data &sd, const char* mes)
{
	if(mes && *mes)
	{
		unsigned char buf[1024];
		size_t len = 1+strlen(mes);
		if(len+8>sizeof(buf)) len = sizeof(buf)-8;

		WBUFW(buf, 0) = 0x08d; //Speech to others
		WBUFW(buf, 2) = len+8;
		WBUFL(buf,4) = sd.block_list::id;
		memcpy(WBUFP(buf,8), mes, len);
		clif_send(buf, len+8, &sd, AREA_CHAT_WOC); //Sends self speech to Area

		WBUFW(buf, 0) = 0x08e; //SelfSpeech
		WBUFW(buf, 2) = len+4;
		memcpy(WBUFP(buf,4), mes, len);
		clif_send_self(sd, buf, len+4);
	}
	return 0;
}

// updates the object's (bl) name on client
int clif_charnameack(int fd, struct block_list &bl, bool clear)
{
	unsigned char buf[128];
	unsigned short cmd;
	
	if(clear)
	{
		cmd = 0x195;
		WBUFB(buf,30) = 0;
		WBUFB(buf,54) = 0;
		WBUFB(buf,78) = 0;
	}
	else
	{	// default sending 0x95 and change to 0x195 when necessary
		cmd = 0x95; 
	}
	switch(bl.type)
	{
	case BL_PC:
	{
		struct map_session_data &sd = (struct map_session_data &)bl;

		if( session_isActive(fd) && session[fd]->user_session)
		{	// first system to detect bot usage
			struct map_session_data *psd = (struct map_session_data *)session[fd]->user_session;
			if( psd->block_list::id != sd.block_list::id &&						// not the same player
				pc_isinvisible(sd) &&							// and can see hidden players
				psd->status.gm_level <= sd.status.gm_level )	// and less gm status
			{	// not necessary to send message if GM can do nothing
				// we can not ban automaticly, because if there is lag, hidden player could be not hidden when other player ask for name.
				char message_to_gm[1024];
				snprintf(message_to_gm, sizeof(message_to_gm), "Possible use of BOT (99%% of chance) or modified client by '%s' (account: %ld, char_id: %ld). This player ask your name when you are hidden.", sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id);
				intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message_to_gm);
			}
		}

		if( *sd.fakename )
		{
			memcpy(WBUFP(buf,6), sd.fakename, 24);
		}
		else
		{
			size_t i;
			struct party *p = NULL;
			struct guild *g = NULL;
			memcpy(WBUFP(buf,6), sd.status.name, 24);
			if( sd.status.guild_id && NULL!=(g = guild_search(sd.status.guild_id)) )
			{	// ギルド所属ならパケット0195を返す
				for(i = 0; i < g->max_member; ++i)
				{
					if( g->member[i].account_id == sd.status.account_id &&
						g->member[i].char_id == sd.status.char_id )
					{
						break;
					}
				}
				if(i < g->max_member)
				{
					unsigned short ps = g->member[i].position;
					cmd = 0x195;
					memcpy(WBUFP(buf,54), g->name,24);
					memcpy(WBUFP(buf,78), g->position[ps].name, 24);
				}
			}
			if( sd.status.party_id && NULL!=(p=party_search(sd.status.party_id)) )
			{
				if(g)
					memcpy(WBUFP(buf,30), p->name, 24);
				else
				{
					for(i=0; i<MAX_PARTY; ++i)
					{
						if( p->member[i].account_id==sd.status.account_id &&
							0==strcmp(p->member[i].name, sd.status.name) )
						{
							break;
						}
					}
					cmd = 0x195;
					if(i<MAX_PARTY && p->member[i].leader)
						memcpy(WBUFP(buf,54), "Leader", 7);
					else
						WBUFB(buf,54) = 0;
					memcpy(WBUFP(buf,78), p->name,24);
					WBUFB(buf,30) = 0;
				}
			}
			else
				WBUFB(buf,30) = 0;
		}
		break;
	}
	case BL_PET:
	{
		struct pet_data& pd = (struct pet_data&)bl;
		memcpy(WBUFP(buf,6), pd.namep, 24);
		if(pd.msd)
		{
			char nameextra[32];
			memcpy(nameextra, pd.msd->status.name, 24);
			nameextra[21]=0; // need 2 extra chars for the attachment
			strcat(nameextra, "'s");

			cmd = 0x195;
			WBUFB(buf,54) = 0;
			memcpy(WBUFP(buf,78), nameextra,24);
		}

		break;
	}
	case BL_HOM:
	{
		memcpy(WBUFP(buf,6), ((homun_data&)bl).status.name, 24);
		break;
	}
	case BL_NPC:
	{
		memcpy(WBUFP(buf,6), ((struct npc_data&)bl).name, 24);
		break;
	}
	case BL_MOB:
		{
		struct mob_data &md = (struct mob_data&)bl;

		memcpy(WBUFP(buf,6), md.name, 24);
		if (md.class_ >= 1285 && md.class_ <= 1288 && md.guild_id)
		{
			struct guild *g;
			struct guild_castle *gc = guild_mapname2gc(maps[md.block_list::m].mapname);
			if (gc && gc->guild_id > 0 && (g = guild_search(gc->guild_id)) != NULL)
			{
				cmd = 0x195;
				WBUFB(buf,30) = 0;
				memcpy(WBUFP(buf,54), g->name, 24);
				memcpy(WBUFP(buf,78), gc->castle_name, 24);
			}
		}
		else if(config.show_mob_hp)
		{
			char mobhp[64];
			cmd = 0x195;
			snprintf(mobhp, sizeof(mobhp), "hp: %ld/%ld", (unsigned long)md.hp, (unsigned long)md.max_hp);
			memcpy(WBUFP(buf,30), mobhp, 24);
			WBUFB(buf,54) = 0;
			WBUFB(buf,78) = 0;
		}
		break;
		}
	case BL_CHAT:	
	{	//FIXME: Clients DO request this... what should be done about it? The chat's title may not fit... [Skotlex]
		safestrcpy((char*)WBUFP(buf,6), ((chat_data&)bl).title, 24);
		break;
	}
	default:
		if (config.error_log)
			ShowError("clif_parse_GetCharNameRequest : bad type %d(%ld)\n", bl.type, (unsigned long)bl.id);
		return 0;
	}

	WBUFW(buf,0) = cmd;
	WBUFL(buf,2) = bl.id;

	// if no receipient specified just update nearby clients
	if( !session_isActive(fd) )
		clif_send(buf, packet_len_table[cmd], &bl, AREA);
	else
	{
		memcpy(WFIFOP(fd, 0), buf, packet_len_table[cmd]);
		WFIFOSET(fd, packet_len_table[cmd]);
	}

	return 0;
}





// -- 友達リスト関連

/*==========================================
 * 友達リストの情報通知
 *------------------------------------------
 */
int clif_friend_send_info(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	size_t len,i;

	WFIFOW(fd, 0) = 0x201;
	for(i=0, len=4; i<MAX_FRIENDLIST; ++i)
	{
		struct friends &frd = sd.status.friendlist[i];
		if( frd.friend_id )
		{
			WFIFOL(fd, len  ) = frd.friend_id;
			WFIFOL(fd, len+4) = frd.friend_id;
			memcpy( WFIFOP( fd, len+8 ), frd.friend_name, 24 );
			len+=32;
		}
	}
	WFIFOW(fd, 2) = len;
	WFIFOSET(fd, len);
	return 0;
}

/*==========================================
 * 友達リストのオンライン情報通知
 *------------------------------------------
 */
int clif_friend_send_online(struct map_session_data &sd, uint32 account_id, uint32 char_id, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x206;
	WFIFOL( fd, 2 ) = account_id;
	WFIFOL( fd, 6 ) = char_id;
	WFIFOB( fd,10 ) = flag;
	WFIFOSET( fd, packet_db[sd.packet_ver][0x206].len );
	return 0;
}

/*==========================================
 * 友達リスト追加要請
 *------------------------------------------
 */
int clif_friend_add_request(struct map_session_data &sd, struct map_session_data &from_sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x207;
	WFIFOL( fd, 2 ) = from_sd.block_list::id;
	WFIFOL( fd, 6 ) = from_sd.status.char_id;
	memcpy(WFIFOP( fd, 10 ), from_sd.status.name, 24 );
	WFIFOSET( fd, packet_db[sd.packet_ver][0x207].len );

	return 0;
}

/*==========================================
 * 友達リスト追加要請返答
 *------------------------------------------
 */
int clif_friend_add_ack(struct map_session_data &sd, uint32 account_id, uint32 char_id, const char* name, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x209;
	WFIFOW( fd, 2 ) = flag;
	WFIFOL( fd, 4 ) = account_id;
	WFIFOL( fd, 8 ) = char_id;
	memcpy(WFIFOP( fd, 12 ), name, 24 );
	WFIFOSET( fd, packet_db[sd.packet_ver][0x209].len );
	return 0;
}

/*==========================================
 * 友達リスト追加削除通知
 *------------------------------------------
 */
int clif_friend_del_ack(struct map_session_data &sd, uint32 account_id, uint32 char_id )
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x20a;
	WFIFOL( fd, 2 ) = account_id;
	WFIFOL( fd, 6 ) = char_id;
	WFIFOSET( fd, packet_db[sd.packet_ver][0x20a].len );
	return 0;
}








// ランキング表示関連
/*==========================================
 * BSランキング
 *------------------------------------------
 */
int clif_blacksmith_fame(struct map_session_data &sd, const uint32 total, int delta)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x21b;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd, packet_len_table[0x21b]);

	return 0;
}
int clif_blacksmith_ranking(struct map_session_data &sd, const CFameList &fl)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	int i;
	WFIFOW(fd,0) = 0x219;
	for(i=0;i<10;i++)
	{
		memcpy(WFIFOP(fd,i*24+2), fl[i].name, 24 );
		WFIFOL(fd,i*4+242) = fl[i].fame_points;
	}
	WFIFOSET(fd,packet_db[sd.packet_ver][0x219].len);
	return 0;
}
/*==========================================
 * アルケミランキング
 *------------------------------------------
 */
int clif_alchemist_fame(struct map_session_data &sd, const uint32 total, int delta)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x21c;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd, packet_len_table[0x21c]);
	
	return 0;
}
int clif_alchemist_ranking(struct map_session_data &sd, const CFameList &fl)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	int i;
	WFIFOW(fd,0) = 0x21a;
	for(i=0;i<MAX_FAMELIST;++i)
	{
		memcpy(WFIFOP(fd,i*24+2), fl[i].name, 24 );
		WFIFOL(fd,i*4+242) = fl[i].fame_points;
	}
	WFIFOSET(fd,packet_db[sd.packet_ver][0x21a].len);
	return 0;
}
/*==========================================
 * テコンランキング
 *------------------------------------------
 */
int clif_taekwon_fame(const struct map_session_data &sd, const uint32 total, int delta)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	WFIFOW(fd,0) = 0x224;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd,10);
	return 0;
}
int clif_taekwon_ranking(struct map_session_data &sd, const CFameList &fl)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	int i;
	WFIFOW(fd,0) = 0x226;
	for(i=0;i<MAX_FAMELIST;++i)
	{
		memcpy(WFIFOP(fd,i*24+2), fl[i].name, 24 );
		WFIFOL(fd,i*4+242) = fl[i].fame_points;
	}
	WFIFOSET(fd,packet_db[sd.packet_ver][0x226].len);
	return 0;
}
/*==========================================
 * 虐殺者ランキング
 *------------------------------------------
 */
int clif_pk_fame(struct map_session_data &sd, const uint32 total, int delta)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	WFIFOW(fd,0) = 0x236;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd,10);
	return 0;
}
int clif_pk_ranking(struct map_session_data &sd, const CFameList &fl)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	int i;
	WFIFOW(fd,0) = 0x238;
	for(i=0;i<MAX_FAMELIST;++i)
	{
		memcpy(WFIFOP(fd,i*24+2), fl[i].name, 24 );
		WFIFOL(fd,i*4+242) = fl[i].fame_points;
	}
	WFIFOSET(fd,packet_db[sd.packet_ver][0x238].len);
	return 0;
}
/*==========================================
 * Info about Star Glaldiator save map [Komurka]
 *------------------------------------------
 */
int clif_feel_info(struct map_session_data &sd, int feel_level)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;
	//memcpy(WFIFOP(fd,2),mapindex_id2name(sd->feel_map[feel_level].index), MAP_NAME_LENGTH);
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=0x100+feel_level;
	WFIFOSET(fd, packet_len_table[0x20e]);
	return 0;
}

/*==========================================
 * Info about Star Glaldiator hate mob [Komurka]
 *------------------------------------------
 */
int clif_hate_mob(struct map_session_data &sd, unsigned short skilllv, unsigned short  mob_id)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;

	if( 0!=strcmp(job_name(mob_id),"Unknown Job") )
		safestrcpy((char*)WFIFOP(fd,2),job_name(mob_id), 24);
	else if( mobdb_checkid(mob_id) )
		safestrcpy((char*)WFIFOP(fd,2),mob_db[mob_id].jname, 24);
	else 
		WFIFOB(fd,2) = 0;
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=0xa00+skilllv-1;
	WFIFOSET(fd, packet_len_table[0x20e]);
	return 0;
}

/*==========================================
 * Info about TaeKwon Do TK_MISSION mob [Skotlex]
 *------------------------------------------
 */
int clif_mission_mob(struct map_session_data &sd, unsigned short mob_id, unsigned short progress)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;
	safestrcpy((char*)WFIFOP(fd,2), (mobdb_checkid(mob_id))?mob_db[mob_id].jname:"", 24);
	WFIFOL(fd,26)=mob_id;
	WFIFOW(fd,30)=0x1400+progress; //Message to display
	WFIFOSET(fd, packet_len_table[0x20e]);
	return 0;
}




/*==========================================
 * メールBOXを表示させる
 *------------------------------------------
 */
int clif_openmailbox(struct map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x260;
	WFIFOL(fd,2) = 0;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x260].len);
	return 0;
}
/*==========================================
 * メール一覧表（BOXを開いている時に蔵へ送信）
 *  0x23fの応答
 *------------------------------------------
 */
int clif_send_mailbox(struct map_session_data &sd, uint32 count, const unsigned char* buffer)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	if( count )
	{
		if( sd.packet_ver > 18 )
		{	// use client-side mail when supported
			size_t len=8+73*count;

			WFIFOW(fd,0)  = 0x240;
			WFIFOW(fd,2)  = len;
			WFIFOL(fd,4)  = count;
			memcpy( WFIFOP(fd,8), buffer, len-8);	// data is already alligned
			WFIFOSET(fd,len);
		}
		else
		{	// display in message window
			char message[512];
			CMailHead head;
			size_t i;
			for(i=1; i<=count; ++i, buffer+=head.size())
			{
				head.frombuffer( buffer );
				// display format is: <read> <id> <date> <from_name> <title>

				struct tm t;
				basics::dttotm(basics::utodatetime(head.sendtime), t);

				snprintf(message, sizeof(message), "%c %-8lu %4u/%02u/%02u %2u:%02u:%02u %-24s %s",
					head.read?' ':'*', (unsigned long)head.msgid, 
					t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour,t.tm_min, t.tm_sec,
					head.name, head.head);
				clif_disp_onlyself(sd, message);
			}
		}
	}
	else
	{
		clif_disp_onlyself(sd, "no mails");
	}
	return 0;
}
/*==========================================
 * メールが送信できましたとかできませんとか
 *------------------------------------------
 */
int clif_res_sendmail(struct map_session_data &sd, bool ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x249;
	WFIFOB(fd,2) = ok?0:1;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x249].len);
	return 0;
}
/*==========================================
 * アイテムが添付できましたとかできませんとか
 *------------------------------------------
 */
int clif_res_sendmail_setappend(struct map_session_data &sd, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x245;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet_db[sd.packet_ver][0x245].len);
	return 0;
}
/*==========================================
 * 新着メールが届きました
 *------------------------------------------
 */
int clif_arrive_newmail(struct map_session_data &sd, const CMailHead& mh)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x24a;
	WFIFOL(fd,2) = mh.msgid;
	memcpy(WFIFOP(fd, 6),mh.name, 24);
	memcpy(WFIFOP(fd,30),mh.head, 40);
	WFIFOSET(fd,packet_db[sd.packet_ver][0x24a].len);
	return 0;
}
/*==========================================
 * メールを選択受信	Inter→本人へ
 *------------------------------------------
 */
int clif_receive_mail(struct map_session_data &sd, const CMail& md)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if( sd.packet_ver > 18 )
	{	// use client-side mail when supported
		struct item_data *data;
		const size_t len=99+1+strlen(md.body);

		WFIFOW(fd,0)=0x242;
		WFIFOW(fd,2)=len;
		WFIFOL(fd,4)=md.msgid;
		memcpy(WFIFOP(fd, 8),md.head, 40);
		memcpy(WFIFOP(fd,48),md.name, 24);
		WFIFOL(fd,72)=0x22;	// 境界？
		WFIFOL(fd,76)=md.zeny;
		WFIFOL(fd,80)=md.item.amount;
		data = itemdb_search(md.item.nameid);
		if(data && data->view_id > 0)
			WFIFOW(fd,84)=data->view_id;
		else
			WFIFOW(fd,84)=md.item.nameid;
		WFIFOW(fd,86)=0;
		WFIFOB(fd,88)=md.item.identify;
		WFIFOB(fd,89)=md.item.attribute;
		// refineやcardを入れても認識してくれない？
		WFIFOW(fd,90)=md.item.card[0];	// 謎
		WFIFOW(fd,92)=md.item.card[1];	// 謎
		WFIFOW(fd,94)=md.item.card[2];	// 謎
		WFIFOW(fd,96)=md.item.card[3];	// 謎
		WFIFOB(fd,98)=0x22;
		memcpy(WFIFOP(fd,99),md.body, 1+strlen(md.body));
		WFIFOSET(fd,len);
	}
	else
	{	// display in window
		if( *md.body )
		{	
			char message[1024];
			struct tm t;
			basics::dttotm(basics::utodatetime(md.sendtime), t);

			snprintf(message, sizeof(message), "%c %-8lu %4u/%02u/%02u %2u:%02u:%02u %-24s %s",
				md.read?' ':'*', (unsigned long)md.msgid, 
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour,t.tm_min, t.tm_sec,
				md.name, md.head);
			clif_disp_onlyself(sd, message);

			// linewise print of mail body
			char *kp, *ip = (char*)md.body;
			while(ip)
			{
				kp = strchr(ip, '\n');
				if(kp) *kp++=0;
				clif_disp_onlyself(sd, ip);
				ip = kp;
			}
		}
		else
			clif_disp_onlyself(sd, "mail not found");	
	}
	return 0;
}

/*==========================================
 * メールが削除できましたとかできませんとか
 *------------------------------------------
 */
int clif_deletemail_res(struct map_session_data &sd, uint32 msgid, bool ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if( sd.packet_ver > 18 )
	{	// use client-side mail when supported

		WFIFOW(fd,0) = 0x257;
		WFIFOL(fd,2) = msgid;
		WFIFOL(fd,6) = ok?0:1;
		WFIFOSET(fd,packet_db[sd.packet_ver][0x257].len);
	}
	else
	{
		char message[512];
		snprintf(message, sizeof(message), "mail %i delet%s.", msgid, (ok?"ed":"ion failed"));
		clif_disp_onlyself(sd, message);
	}
	return 0;
}







///////////////////////////////////////////////////////////////////////////////
int clif_getPacketVer(int fd)
{
	int i;
	if( session_isActive(fd) )
	{
		unsigned short cmd = RFIFOW(fd,0);
		CAuth auth;
		
		for(i=MAX_PACKET_VER; i>0; i--)
		{
			if( cmd==packet_db[i].connect_cmd &&
				getAthentification( RFIFOL(fd, packet_db[i][cmd].pos[0]) ,auth) &&
				// explicitely tested above
				//auth.account_id == RFIFOL(fd, packet_db[i][cmd].pos[0])
				auth.login_id1  == RFIFOL(fd, packet_db[i][cmd].pos[2]) &&
				auth.client_ip  == session[fd]->client_ip &&
				RFIFOREST(fd) >= packet_db[i][cmd].len &&
				(RFIFOB(fd, packet_db[i][cmd].pos[4]) == 0 ||	// 00 = Female
				 RFIFOB(fd, packet_db[i][cmd].pos[4]) == 1) )	// 01 = Male
			{
				return i;
			}
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
// clif_parse_*
// パケット読み取って色々操作
int clif_parse_WantToConnection(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	struct map_session_data *old_sd;
	unsigned short cmd	= RFIFOW(fd,0);
	uint32 account_id	= RFIFOL(fd, packet_db[sd.packet_ver][cmd].pos[0]);
	uint32 char_id		= RFIFOL(fd, packet_db[sd.packet_ver][cmd].pos[1]);
	uint32 login_id1	= RFIFOL(fd, packet_db[sd.packet_ver][cmd].pos[2]);
	uint32 client_tick	= RFIFOL(fd, packet_db[sd.packet_ver][cmd].pos[3]);
	unsigned char sex	= RFIFOB(fd, packet_db[sd.packet_ver][cmd].pos[4]);

	//ShowMessage("WantToConnection: Received %d bytes with packet 0x%X -> ver %i\n", RFIFOREST(fd), cmd, sd.packet_ver);

	// if same account already connected, we disconnect the 2 sessions
	if((old_sd = map_id2sd(account_id)) != NULL)
	{
		clif_authfail(sd, 8); // still recognizes last connection
		clif_authfail(*old_sd, 2); // same id

		session_Remove(fd); // Set session to EOF
		session_Remove(old_sd->fd); // Set session to EOF
	}
	else
	{

		struct map_session_data *plsd = new struct map_session_data(fd, account_id, char_id, login_id1, client_tick, sex);

		session[fd]->user_session = plsd;
		plsd->fd = fd;
		plsd->packet_ver = sd.packet_ver;

		plsd->ScriptEngine.temporaty_init(); //!! call constructor explicitely until switched to c++ allocation

		pc_setnewpc(fd, *plsd, account_id, char_id, login_id1, client_tick, sex);
		WFIFOL(fd,0) = plsd->block_list::id;
		WFIFOSET(fd,4);

		plsd->map_addiddb();
		chrif_authreq(*plsd);
	}

	return 0;
}

/*==========================================
 * 007d クライアント側マップ読み込み完了
 * map侵入時に必要なデータを全て送りつける
 *------------------------------------------
 */
int clif_parse_LoadEndAck(int fd, struct map_session_data &sd)
{
	int i;

	if( !session_isActive(fd) )
		return 0;

	if(sd.block_list::prev != NULL)
		return 0;

	// 接続ok時
	//clif_authok();
	//if(sd.npc_id) npc_event_dequeue(sd);
	clif_skillinfoblock(sd);
	pc_checkitem(sd);
	//guild_info();

	// loadendack時
	// next exp
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	// skill point
	clif_updatestatus(sd,SP_SKILLPOINT);
	// item
	clif_itemlist(sd);
	clif_equiplist(sd);
	// cart
	if( pc_iscarton(sd) )
	{
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd,SP_CARTINFO);
	}
	// param all
	clif_initialstatus(sd);
	// party
	party_send_movemap(sd);
	// guild
	guild_send_memberinfoshort(sd,1);
	// 119
	// 78

	if(config.pc_invincible_time > 0) {
		if(maps[sd.block_list::m].flag.gvg)
			pc_setinvincibletimer(sd,config.pc_invincible_time<<1);
		else
			pc_setinvincibletimer(sd,config.pc_invincible_time);
	}

	sd.map_addblock();	// ブロック登録
	clif_spawnpc(sd);	// spawn

	// weight max , now
	clif_updatestatus(sd,SP_MAXWEIGHT);
	clif_updatestatus(sd,SP_WEIGHT);

	// pvp
	if(sd.pvp_timer!=-1 && !config.pk_mode)
	{
		delete_timer(sd.pvp_timer,pc_calc_pvprank_timer);
		sd.pvp_timer = -1;
	}
	if(maps[sd.block_list::m].flag.pvp){
		if(!config.pk_mode) { // remove pvp stuff for pk_mode [Valaris]
			sd.pvp_timer=add_timer(gettick()+200,pc_calc_pvprank_timer,sd.block_list::id,0);
			sd.pvp_rank=0;
			sd.pvp_lastusers=0;
			sd.pvp_point=5;
			sd.pvp_won=0;
			sd.pvp_lost=0;
		}
		clif_set0199(sd.fd,1);
	} else {
		sd.pvp_timer=-1;
	}
	if(maps[sd.block_list::m].flag.gvg) {
		clif_set0199(sd.fd,3);
	}

	// pet
	if(sd.status.pet_id > 0 && sd.pd && sd.pet.intimate > 0) {
		sd.pd->map_addblock();
		clif_spawnpet(*sd.pd);
		clif_send_petdata(sd,0,0);
		clif_send_petdata(sd,5,config.pet_hair_style);
		clif_send_petstatus(sd);
	}

	if(sd.state.connect_new)
	{
		sd.state.connect_new = 0;
		if(sd.status.class_ != sd.view_class)
			clif_changelook(sd,LOOK_BASE,sd.view_class);
		if(sd.status.pet_id > 0 && sd.pd && sd.pet.intimate > 900)
			clif_pet_emotion(*sd.pd,(sd.pd->class_ - 100)*100 + 50 + pet_hungry_val(sd));

		// Stop players from spawning inside castles [Valaris]
		struct guild_castle *gc=guild_mapname2gc(maps[sd.block_list::m].mapname);
			if (gc)
			pc_setpos(sd,sd.status.save_point.mapname,sd.status.save_point.x,sd.status.save_point.y,2);
		// End Addition [Valaris]
			}
	// view equipment item
#if PACKETVER < 4
	clif_changelook(sd,LOOK_WEAPON,sd.status.weapon);
	clif_changelook(sd,LOOK_SHIELD,sd.status.shield);
#else
	clif_changelook(sd,LOOK_WEAPON,0);
#endif
	if(config.save_clothcolor &&
		sd.status.clothes_color > 0 &&
		(sd.view_class != 22 || !config.wedding_ignorepalette) )
		clif_changelook(sd,LOOK_CLOTHES_COLOR,sd.status.clothes_color);

	//if(sd.status.hp<sd.status.max_hp>>2 && pc_checkskill(sd,SM_AUTOBERSERK)>0 &&
	if(sd.status.hp<sd.status.max_hp>>2 && sd.sc_data[SC_AUTOBERSERK].timer != -1 &&
		(sd.sc_data[SC_PROVOKE].timer==-1 || sd.sc_data[SC_PROVOKE].val2.num==0 ))
		// オートバーサーク発動
		status_change_start(&sd,SC_PROVOKE,10,1,0,0,0,0);

//	if(time(&timer) < ((weddingtime=pc_readglobalreg(sd,"PC_WEDDING_TIME")) + 3600))
//		status_change_start(&sd,SC_WEDDING,0,weddingtime,0,0,36000,0);

	if(config.muting_players && sd.status.manner < 0)
		status_change_start(&sd,SC_NOCHAT,0,0,0,0,0,0);

	if (night_flag && !maps[sd.block_list::m].flag.indoors)
		clif_weather1(sd.fd, 474 + config.night_darkness_level);

	// option
	clif_changeoption(sd);
	if(sd.sc_data[SC_TRICKDEAD].timer != -1)
		status_change_end(&sd,SC_TRICKDEAD,-1);
	if(sd.sc_data[SC_SIGNUMCRUCIS].timer != -1 && !battle_check_undead(7,sd.def_ele))
		status_change_end(&sd,SC_SIGNUMCRUCIS,-1);
	if(sd.state.infinite_endure && sd.sc_data[SC_ENDURE].timer == -1)
		status_change_start(&sd,SC_ENDURE,10,1,0,0,0,0);
	for(i=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].equip && sd.status.inventory[i].equip & 0x0002 && sd.status.inventory[i].attribute==1)
			status_change_start(&sd,SC_BROKNWEAPON,0,0,0,0,0,0);
		if(sd.status.inventory[i].equip && sd.status.inventory[i].equip & 0x0010 && sd.status.inventory[i].attribute==1)
			status_change_start(&sd,SC_BROKNARMOR,0,0,0,0,0,0);
	}

	CMap::foreachinarea( CClifGetAreaChar(sd),
		sd.block_list::m,((int)sd.block_list::x)-AREA_SIZE,((int)sd.block_list::y)-AREA_SIZE,((int)sd.block_list::x)+AREA_SIZE,((int)sd.block_list::y)+AREA_SIZE,0);
//	map_foreachinarea(clif_getareachar,
//		sd.block_list::m,((int)sd.block_list::x)-AREA_SIZE,((int)sd.block_list::y)-AREA_SIZE,((int)sd.block_list::x)+AREA_SIZE,((int)sd.block_list::y)+AREA_SIZE,0,
//		&sd);

	// ============================================
	// ADDITION Qamera death/disconnect/connect event mod
	if(!sd.state.event_onconnect){
		npc_event_doall_attached("OnConnect",sd);
		sd.state.event_onconnect=1;
	}
	// ============================================ 
	// Lance
	if( script_config.event_script_type == 0 )
	{
		struct npc_data *npc= npc_name2id(script_config.mapload_event_name);
		if(npc && npc->u.scr.ref && (npc->block_list::m==0xFFFF || npc->block_list::m==sd.block_list::m) )
		{
			CScriptEngine::run(npc->u.scr.ref->script, 0, sd.block_list::id, npc->block_list::id);
			ShowStatus("Event '"CL_WHITE"%s"CL_RESET"' executed.\n", script_config.mapload_event_name);
		}
	}
	else
	{
		int evt = npc_event_doall_id("OnPCLoadMapEvent", sd.block_list::id, sd.block_list::m);
		if(evt) ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n", evt, "OnPCLoadMapEvent");
		// ============================================ 
	}

	send_fake_id(fd,sd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_TickSend(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	sd.client_tick=RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	return clif_servertick(sd, gettick());
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_WalkToXY(int fd, struct map_session_data &sd)
{
	int x, y;

	if( !session_isActive(fd) )
		return 0;

	if( pc_isdead(sd) )
		return clif_clearchar_area(sd, 1);

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.chatID || pc_issit(sd) )
		return 0;

	if (sd.skilltimer != -1 && pc_checkskill(sd, SA_FREECAST) <= 0) // フリーキャスト
		return 0;

	if (sd.canmove_tick > gettick())
		return 0;

	// ステータス異常やハイディング中(トンネルドライブ無)で動けない
	if( (sd.opt1 > 0 && sd.opt1 != 6) ||
	     sd.sc_data[SC_ANKLE].timer !=-1 || //アンクルスネア
	     sd.sc_data[SC_AUTOCOUNTER].timer !=-1 || //オートカウンター
	     sd.sc_data[SC_TRICKDEAD].timer !=-1 || //死んだふり
	     sd.sc_data[SC_BLADESTOP].timer !=-1 || //白刃取り
	     sd.sc_data[SC_SPIDERWEB].timer !=-1 || //スパイダーウェッブ
	     (sd.sc_data[SC_DANCING].timer !=-1 && sd.sc_data[SC_DANCING].val4.num) || //合奏スキル演奏中は動けない
		 (sd.sc_data[SC_GOSPEL].timer !=-1 && sd.sc_data[SC_GOSPEL].val4.num == BCT_SELF) ||	// cannot move while gospel is in effect
		 (sd.sc_data[SC_DANCING].timer !=-1 && sd.sc_data[SC_DANCING].val1.num == CG_HERMODE)  //cannot move while Hermod is active.
		)
		return 0;
	if ((sd.status.option & 2) && pc_checkskill(sd, RG_TUNNELDRIVE) <= 0)
		return 0;

	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);

	sd.stop_attack();

	
	int cmd = RFIFOW(fd,0);
	x = RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[0]) * 4 +
		(RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[0] + 1) >> 6);
	y = ((RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[0]+1) & 0x3f) << 4) +
		(RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[0] + 2) >> 4);

	sd.walktoxy(x,y);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_QuitGame(int fd, struct map_session_data &sd)
{
	unsigned long tick=gettick();
	struct skill_unit_group* sg;

	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x18b;
	if ((!pc_isdead(sd) && (sd.opt1 || (sd.opt2 && !(night_flag == 1 && sd.opt2 == STATE_BLIND)))) ||
	    sd.skilltimer != -1 ||
	    (DIFF_TICK(tick, sd.canact_tick) < 0) ||
	    (sd.sc_data && sd.sc_data[SC_DANCING].timer!=-1 && sd.sc_data[SC_DANCING].val2.isptr && (sg=(struct skill_unit_group *)sd.sc_data[SC_DANCING].val2.ptr) && sg->src_id == sd.block_list::id))
	{
		WFIFOW(fd,2)=1;
		WFIFOSET(fd,packet_len_table[0x18b]);
		return 0;
	}

	/*	Rovert's prevent logout option fixed [Valaris]	*/
	if( ((gettick() - sd.canlog_tick) >= 10000) || (!config.prevent_logout) || !pc_isdead(sd))
	{
		session_SetWaitClose(fd, 1000);
		WFIFOW(fd,2)=0;
	}
	else
	{
		WFIFOW(fd,2)=1;
	}
	WFIFOSET(fd,packet_len_table[0x18b]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_GetCharNameRequest(int fd, struct map_session_data &sd)
{
	struct block_list* bl;
	uint32 account_id;
	if( !session_isActive(fd) )
		return 0;
	
	account_id = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);

	if(account_id & FLAG_DISGUISE) // for disguises [Valaris]
		account_id &= ~FLAG_DISGUISE;

	if ((bl = map_id2bl(account_id)) != NULL)	
		clif_charnameack(fd, *bl);
	else
	{	// if player asks for the fake mob/player (only bot and modified client can see a hiden mob/player)
		check_fake_id(fd, sd, account_id);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_GlobalMessage(int fd, struct map_session_data &sd)
{	// S 008c <len>.w <str>.?B
	char message[1024];
	unsigned char buf[65536];
	size_t size;

	if( !session_isActive(fd) )
		return 0;

	size = RFIFOW(fd,2);
	if( size > (size_t)RFIFOREST(fd) )
	{	// some serious error
		ShowError("clif_parse_GlobalMessage: size marker outside buffer %i > %i, (connection %i=%i,%p=%p)", 
			size, RFIFOREST(fd), fd,sd.fd, session[fd]->user_session, &sd);
		return 0;
	}
	RFIFOB(fd,size-1)=0; // add an eof marker in the buffer

	//ShowMessage("clif_parse_GlobalMessage: message: '%s'.\n", RFIFOP(fd,4));
	if( strncmp((char*)RFIFOP(fd,4), sd.status.name, strlen(sd.status.name)) != 0 )
	{
		snprintf(message,size+16, "Hack on global message (normal message): character '%s' (account: %ld) uses another name.", sd.status.name, (unsigned long)sd.status.account_id);
		// information is send to all online GM
		ShowMessage(message);
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message);

		if( strlen((char*)RFIFOP(fd,4)) == 0 )
			snprintf(message,sizeof(message), " Player sends a void name and a void message.");
		else
			snprintf(message,sizeof(message), " Player sends (name:message): '%s'.", RFIFOP(fd,4));
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message);

		// message about the ban
		if (config.ban_spoof_namer > 0)
			snprintf(message,sizeof(message), " Player has been banned for %ld minute(s).", (unsigned long)config.ban_spoof_namer);
		else
			snprintf(message,sizeof(message), " Player hasn't been banned (Ban option is disabled).");
		intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message);

		// if we ban people
		if (config.ban_spoof_namer > 0) {
			chrif_char_ask_name(-1, sd.status.name, 2, 0, 0, 0, 0, config.ban_spoof_namer, 0); // type: 2 - ban (year, month, day, hour, minute, second)
			session_SetWaitClose(fd, 1000); // forced to disconnect because of the hack
		}
		// but for the hacker, we display on his screen (he see/look no difference).
		return 0;
	}

	if( (is_atcommand(fd, sd, (char*)RFIFOP(fd,4), 0) != AtCommand_None) ||
        (is_charcommand(fd, sd, (char*)RFIFOP(fd,4),0)!= CharCommand_None) ||
	    sd.sc_data[SC_BERSERK].timer != -1 || //バーサーク時は会話も不可
		sd.sc_data[SC_NOCHAT].timer != -1 ) //チャット禁止
		return 0;

	
	// send message to others
	WBUFW(buf,0) = 0x8d;
	WBUFW(buf,2) = size + 4; // len of message - 4 + 8
	WBUFL(buf,4) = sd.block_list::id;
	memcpy(WBUFP(buf,8), RFIFOP(fd,4), size - 4);
	clif_send(buf, size + 4, &sd, sd.chatID ? CHAT_WOS : AREA_CHAT_WOC);
	
	// send back message to the speaker
	memcpy(WFIFOP(fd,0), RFIFOP(fd,0), size);
	WFIFOW(fd,0) = 0x8e;
	WFIFOSET(fd, size);

	CMap::foreachinarea( CNpcChat(((char*)RFIFOP(fd,4)), strlen((char*)RFIFOP(fd,4)), sd),
		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE, ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_NPC);
//	map_foreachinarea(npc_chat_sub, 
//		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE, ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_NPC, 
//		RFIFOP(fd,4), strlen((char*)RFIFOP(fd,4)), &sd);

	// Celest
	if (pc_calc_base_job2 (sd.status.class_) == 23 ) {
		int next = pc_nextbaseexp(sd) > 0 ? pc_nextbaseexp(sd) : sd.status.base_exp;
		char *rfifo = (char*)RFIFOP(fd,4);
		if (next > 0 && (sd.status.base_exp * 100 / next) % 10 == 0) {
			if (sd.state.snovice_flag == 0 && strstr(rfifo, msg_txt(504)))
				sd.state.snovice_flag = 1;
			else if (sd.state.snovice_flag == 1) {
				snprintf(message, sizeof(message), msg_txt(505), sd.status.name);
				if (strstr(rfifo, message))
					sd.state.snovice_flag = 2;
			}
			else if (sd.state.snovice_flag == 2 && strstr(rfifo, msg_txt(506)))
				sd.state.snovice_flag = 3;
			else if (sd.state.snovice_flag == 3) {
				clif_skill_nodamage(sd,sd,MO_EXPLOSIONSPIRITS,0xFFFF,1);
				status_change_start(&sd,SkillStatusChangeTable[MO_EXPLOSIONSPIRITS],
						17,0,0,0,skill_get_time(MO_EXPLOSIONSPIRITS,1),0 ); //Lv17-> +50 critical (noted by Poki) [Skotlex]
				sd.state.snovice_flag = 0;
			}
		}
	}
	return 0;
}

int clif_message(struct block_list &bl, const char* msg)
{
	if(msg)
	{
	unsigned short msg_len = strlen(msg) + 1;
	unsigned char buf[256];
		if(msg_len>256-8)	msg_len=256-8;

	WBUFW(buf, 0) = 0x8d;
	WBUFW(buf, 2) = msg_len + 8;
		WBUFL(buf, 4) = bl.id;
	memcpy(WBUFP(buf, 8), msg, msg_len);
		buf[255] = 0;// force EOS
		clif_send(buf, WBUFW(buf,2), &bl, AREA_CHAT_WOC);	// by Gengar
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_MapMove(int fd, struct map_session_data &sd)
{	// /m /mapmove (as @rura GM command)
	char output[32];
	char mapname[32], *ip;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= get_atcommand_level(AtCommand_MapMove) )
	{
		safestrcpy(mapname, (const char*)RFIFOP(fd,2), sizeof(mapname));
		ip = strchr(mapname, '.');
		if(ip) *ip=0;
		snprintf(output, sizeof(output), "%s %d %d", mapname, (unsigned char)RFIFOW(fd,18), (unsigned char)RFIFOW(fd,20));
		atcommand_rura(fd, sd, "@rura", output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changed_dir(struct block_list &bl)
{
	unsigned char buf[64];

	WBUFW(buf,0) = 0x9c;
	WBUFL(buf,2) = bl.id;
	WBUFW(buf,6) = bl.get_headdir();
	WBUFB(buf,8) = bl.get_bodydir();

	clif_send(buf, packet_len_table[0x9c], &bl, AREA_WOS);

	if(bl.type == BL_PC)
	{
		struct map_session_data &sd = (struct map_session_data &)bl;
		if(sd.disguise_id)
		{
			WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
			clif_send(buf, packet_len_table[0x9c], &bl, AREA);
		}
	}

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChangeDir(int fd, struct map_session_data &sd)
{
	dir_t bodydir, headdir;

	if( !session_isActive(fd) )
		return 0;

	headdir = (dir_t)(unsigned char)RFIFOB(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	bodydir = (dir_t)(unsigned char)RFIFOB(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);

	sd.set_dir(bodydir, headdir);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_Emotion(int fd, struct map_session_data &sd)
{
	unsigned char buf[64];
	if( !session_isActive(fd) )
		return 0;

	if(config.basic_skill_check == 0 || pc_checkskill(sd, NV_BASIC) >= 2)
	{
		if(RFIFOB(fd,2) == 34)
		{	// prevent use of the mute emote [Valaris]
			return clif_skill_fail(sd, 1, 0, 1);
		}
		// fix flood of emotion icon (ro-proxy): flood only the hacker player
		if(sd.emotionlasttime >= time(NULL))
		{
			sd.emotionlasttime = time(NULL) + 1; // not more than 1 every second (normal client is every 3-4 seconds)
			return clif_skill_fail(sd, 1, 0, 1);
		}
		sd.emotionlasttime = time(NULL) + 1; // not more than 1 every second (normal client is every 3-4 seconds)

		WBUFW(buf,0) = 0xc0;
		WBUFL(buf,2) = sd.block_list::id;
		WBUFB(buf,6) = RFIFOB(fd,2);
		return clif_send(buf, packet_len_table[0xc0], &sd, AREA);
	} else
		return clif_skill_fail(sd, 1, 0, 1);
}

/*==========================================
 *
 *------------------------------------------
 */
int  clif_parse_HowManyConnections(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xc2;
	WFIFOL(fd,2) = map_getusers();
	WFIFOSET(fd,packet_len_table[0xc2]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ActionRequest(int fd, struct map_session_data &sd)
{
	unsigned long tick;
	unsigned char buf[64];
	int action_type;
	uint32 target_id;

	if( !session_isActive(fd) )
		return 0;

	if (pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.opt1 > 0 || sd.status.option & 2 ||
	    (sd.sc_data &&
	     (sd.sc_data[SC_TRICKDEAD].timer != -1 ||
		  sd.sc_data[SC_AUTOCOUNTER].timer != -1 || //オートカウンター
	      sd.sc_data[SC_BLADESTOP].timer != -1 || //白刃取り
	      sd.sc_data[SC_DANCING].timer != -1)) ) //ダンス中
		return 0;

	tick = gettick();

	sd.stop_walking(0);
	sd.stop_attack();

	target_id = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	action_type = RFIFOB(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);
	
	switch(action_type) {
	case 0x00: // once attack
	case 0x07: // continuous attack


		if(sd.sc_data[SC_WEDDING].timer != -1 || sd.view_class==22)
			return 0;
		if (sd.vender_id != 0)
			return 0;
		if (!config.skill_delay_attack_enable && pc_checkskill(sd, SA_FREECAST) <= 0) {
			if (DIFF_TICK(tick, sd.canact_tick) < 0) {
				return clif_skill_fail(sd, 1, 4, 0);
			}
		}
		if (sd.invincible_timer != -1)
			pc_delinvincibletimer(sd);
		if (sd.target_id) // [Valaris]
			sd.target_id = 0;
		pc_attack(sd, target_id, action_type != 0);
		break;
	case 0x02: // sitdown
		if (config.basic_skill_check == 0 || pc_checkskill(sd, NV_BASIC) >= 3) {
			if (sd.skilltimer != -1) //No sitting while casting :P
				break;
			sd.stop_attack();
			sd.stop_walking(1);
			pc_setsit(sd);
			skill_gangsterparadise(&sd, 1); // ギャングスターパラダイス設定 fixed Valaris
			skill_rest(sd, 1); // TK_HPTIME sitting down mode [Dralnu]
			clif_sitting(sd);
		} else
			clif_skill_fail(sd, 1, 0, 2);
		break;
	case 0x03: // standup
		pc_setstand(sd);
		skill_gangsterparadise(&sd, 0); // ギャングスターパラダイス解除 fixed Valaris
		skill_rest(sd, 0); // TK_HPTIME standing up mode [Dralnu]
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = sd.block_list::id;
		WBUFB(buf,26) = 3;
		clif_send(buf, packet_len_table[0x8a], &sd, AREA);
		if(sd.disguise_id) {
			WBUFL(buf, 2) = sd.block_list::id|FLAG_DISGUISE;
			clif_send(buf, packet_len_table[0x8a], &sd, AREA);
		}
		break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_Restart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOB(fd,2)) {
	case 0x00:
		if (pc_isdead(sd)) {
			pc_setstand(sd);
			pc_setrestartvalue(sd, 3);
			pc_setpos(sd, sd.status.save_point.mapname, sd.status.save_point.x, sd.status.save_point.y, 2);
		}
		// in case the player's status somehow wasn't updated yet [Celest]
		else if (sd.status.hp <= 0)
			pc_setdead(sd);
		break;
	case 0x01:
		if(!pc_isdead(sd) && (sd.opt1 || (sd.opt2 && !(night_flag == 1 && sd.opt2 == STATE_BLIND))))
			return 0;

		/*	Rovert's Prevent logout option - Fixed [Valaris]	*/
		if (!config.prevent_logout ||
			(gettick() - sd.canlog_tick) >= 10000 ||
			pc_isdead(sd))	//Allow dead characters to logout [Skotlex]
		{
			map_quit(sd);
			chrif_charselectreq(sd);
			session_SetWaitClose(fd, 2000);
		} else {
			WFIFOW(fd,0)=0x18b;
			WFIFOW(fd,2)=1;
			WFIFOSET(fd,packet_len_table[0x018b]);
		}
		break;
	}
	return 0;
}


/*==========================================
 * Transmission of a wisp (S 0096 <len>.w <nick>.24B <message>.?B)
 *------------------------------------------
 */
int clif_parse_Wis(int fd, struct map_session_data &sd)
{	// S 0096 <len>.w <nick>.24B <message>.?B // rewritten by [Yor]
	char *gm_command;
	struct map_session_data *dstsd;
	int i;

	if( !session_isActive(fd) )
		return 0;

	//ShowMessage("clif_parse_Wis: message: '%s'.\n", RFIFOP(fd,28));

	// 24+3+(RFIFOW(fd,2)-28)+1 or 24+3+(strlen(RFIFOP(fd,28))+1 (size can be wrong with hacker)

	size_t sz = RFIFOREST(fd);
	if( sz > (size_t)RFIFOW(fd,2) )
		sz = RFIFOW(fd,2);
	// force termination
	RFIFOB(fd,sz)=0; 

	gm_command = new char[sz+28]; // 24 (for name) + 3 (for " : ") + 1 (for eos)
	sprintf(gm_command, "%s : %s", sd.status.name, RFIFOP(fd,28));

	if ((is_charcommand(fd, sd, gm_command, 0) != CharCommand_None) ||
		(is_atcommand(fd, sd, gm_command, 0) != AtCommand_None) ||
	    (sd.sc_data &&
	     (sd.sc_data[SC_BERSERK].timer!=-1 || //バーサーク時は会話も不可
	      sd.sc_data[SC_NOCHAT].timer != -1))) //チャット禁止
	{
		if(gm_command) delete[] gm_command;
		return 0;
	}

	if(gm_command) delete[] gm_command;

	//Chat Logging type 'W' / Whisper
	if(log_config.chat&1 //we log everything then
		|| ( log_config.chat&2 //if Whisper bit is on
		&& ( !agit_flag || !(log_config.chat&16) ))) //if WOE ONLY flag is off or AGIT is OFF
		log_chat("W", 0, sd.status.char_id, sd.status.account_id, (char*)sd.mapname, sd.block_list::x, sd.block_list::y, (char*)RFIFOP(fd, 4), (char*)RFIFOP(fd, 28));

	//-------------------------------------------------------//
	//   Lordalfa - Paperboy - To whisper NPC commands       //
	//-------------------------------------------------------//
	struct npc_data *npc;
	if ((npc = npc_name2id((const char*)RFIFOP(fd,4))))
	{
		char tempmes[128];
		char *buffer = (char*)RFIFOP(fd,28);
		size_t i;
		char *ip=buffer, *kp=NULL;
		for(i=0; i<10; ++i)
		{
			if(ip)
			{
				kp = ip;
				ip = strchr(ip,',');
				if(ip) *ip++=0;
			}
			else
				kp = "";
			snprintf(tempmes, sizeof(tempmes),"@whispervar%ld$", (unsigned long)i);
			set_var(sd,tempmes,kp);
		}//Sets Variables to use in the NPC
		
		snprintf(tempmes, sizeof(tempmes), "%s::OnWhisperGlobal", npc->name);
		if (npc_event(sd,tempmes,0))
			return 0;	// Calls the NPC label
	}
	//-------------------------------------------------------//
	//  Lordalfa - Paperboy - END - NPC Whisper Commands     //
	//-------------------------------------------------------//


	// searching destination character
	dstsd = map_nick2sd((char*)RFIFOP(fd,4));
	// player is not on this map-server
	if (dstsd == NULL ||
	// At this point, don't send wisp/page if it's not exactly the same name, because (example)
	// if there are 'Test' player on an other map-server and 'test' player on this map-server,
	// and if we ask for 'Test', we must not contact 'test' player
	// so, we send information to inter-server, which is the only one which decide (and copy correct name).
	    strcmp(dstsd->status.name, (char*)RFIFOP(fd,4)) != 0) // not exactly same name
		// send message to inter-server
		intif_wis_message(sd, (char*)RFIFOP(fd,4), (char*)RFIFOP(fd,28), RFIFOW(fd,2)-28);
	// player is on this map-server
	else {
		// if you send to your self, don't send anything to others
		if (dstsd->fd == fd) // but, normaly, it's impossible!
			clif_wis_message(fd, wisp_server_name, "You can not page yourself. Sorry.", strlen("You can not page yourself. Sorry.") + 1);
		// otherwise, send message and answer immediatly
		else {
			if(dstsd->state.ignoreAll == 1) {
				if (dstsd->status.option & OPTION_HIDE && sd.isGM() < dstsd->isGM())
					clif_wis_end(fd, 1); // 1: target character is not loged in
				else
					clif_wis_end(fd, 3); // 3: everyone ignored by target
			} else {
				// if player ignore the source character
				for(i = 0; i < MAX_IGNORE_LIST; ++i)
					if (strcmp(dstsd->ignore[i].name, sd.status.name) == 0) {
						clif_wis_end(fd, 2);	// 2: ignored by target
						break;
					}
				// if source player not found in ignore list
				if (i == MAX_IGNORE_LIST) {
					clif_wis_message(dstsd->fd, sd.status.name, (char*)RFIFOP(fd,28), RFIFOW(fd,2) - 28);
					clif_wis_end(fd, 0); // 0: success to send wisper
				}
			}
		}
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_GMmessage(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= get_atcommand_level(AtCommand_Broadcast) )
		intif_GMmessage((char*)RFIFOP(fd,4),RFIFOW(fd,2)-4, 0);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_TakeItem(int fd, struct map_session_data &sd)
{
	flooritem_data *fitem;
	int map_object_id;

	if( !session_isActive(fd) )
		return 0;

	map_object_id = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);


	fitem = (flooritem_data*)map_id2bl(map_object_id);

	if (pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0 ||
		pc_iscloaking(sd) || pc_ischasewalk(sd) || //Disable cloaking/chasewalking characters from looting [Skotlex]
		(sd.sc_data && (sd.sc_data[SC_TRICKDEAD].timer != -1 || //死んだふり
		 sd.sc_data[SC_BLADESTOP].timer != -1 || //白刃取り
		 sd.sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク
		 sd.sc_data[SC_CHASEWALK].timer!=-1 ||  //Chasewalk [Aru]
		 sd.sc_data[SC_NOCHAT].timer!=-1 )) )	//会話禁止
	{
		clif_additem(sd,0,0,6); // send fail packet! [Valaris]
		return 0;
	}
		
	if (fitem == NULL || fitem->block_list::type != BL_ITEM || fitem->block_list::m != sd.block_list::m)
		return 0;

	pc_takeitem(sd, *fitem);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_DropItem(int fd, struct map_session_data &sd)
{
	int item_index, item_amount;

	if( !session_isActive(fd) )
		return 0;

	if (pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0 ||
		(sd.sc_data && (sd.sc_data[SC_AUTOCOUNTER].timer != -1 || //オートカウンター
		sd.sc_data[SC_BLADESTOP].timer != -1 || //白刃取り
		sd.sc_data[SC_BERSERK].timer != -1)) ) //バーサーク
		return 0;

	item_index = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0])-2;
	item_amount = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);

	pc_dropitem(sd, item_index, item_amount);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_UseItem(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if (pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || (sd.opt1 > 0 && sd.opt1 != 6) ||
	    (sd.sc_data && (sd.sc_data[SC_TRICKDEAD].timer != -1 || //死んだふり
	     sd.sc_data[SC_BLADESTOP].timer != -1 || //白刃取り
		sd.sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク
		sd.sc_data[SC_NOCHAT].timer!=-1 ||
		sd.sc_data[SC_GRAVITATION].timer!=-1)) )	//会話禁止
		return 0;

	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);

	pc_useitem(sd,RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0])-2);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_EquipItem(int fd, struct map_session_data &sd)
{
	unsigned short index;

	if( !session_isActive(fd) )
		return 0;

	if(pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	index = RFIFOW(fd,2)-2;
	if( index >= MAX_INVENTORY || sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0)
		return 0;
	if(sd.sc_data && ( sd.sc_data[SC_BLADESTOP].timer!=-1 || sd.sc_data[SC_BERSERK].timer!=-1 )) 
		return 0;

	if(	sd.status.inventory[index].identify != 1) {		// 未鑑定
		return clif_equipitemack(sd,index,0,0);	// fail
	}
	//ペット用装備であるかないか
	if(sd.inventory_data[index]) {
		if(sd.inventory_data[index]->type != 8){
			if(sd.inventory_data[index]->type == 10)
				RFIFOW(fd,4)=0x8000;	// 矢を無理やり装備できるように（－－；
			pc_equipitem(sd,index,RFIFOW(fd,4));
		} else
			pet_equipitem(sd,index);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_UnequipItem(int fd, struct map_session_data &sd)
{
	int index;

	if( !session_isActive(fd) )
		return 0;

	if(pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0)
		return 0;
	index = RFIFOW(fd,2)-2;

	/*if(sd.status.inventory[index].attribute == 1 && sd.sc_data && sd.sc_data[SC_BROKNWEAPON].timer!=-1)
		status_change_end(&sd,SC_BROKNWEAPON,-1);
	if(sd.status.inventory[index].attribute == 1 && sd.sc_data && sd.sc_data[SC_BROKNARMOR].timer!=-1)
		status_change_end(&sd,SC_BROKNARMOR,-1);
	if(sd.sc_count && ( sd.sc_data[SC_BLADESTOP].timer!=-1 || sd.sc_data[SC_BERSERK].timer!=-1 ))
		return 0;*/

	pc_unequipitem(sd,index,1);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcClicked(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(pc_isdead(sd)) {
		return clif_clearchar_area(sd, 1);
	}
	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner || RFIFOL(fd,2)&FLAG_DISGUISE)
		return 0;
	npc_click(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcBuySellSelected(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if(sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;
	npc_buysellsel(sd,RFIFOL(fd,2),RFIFOB(fd,6));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcBuyListSend(int fd, struct map_session_data &sd)
{
	int fail=1;
	if( !session_isActive(fd) )
		return 0;

	if(sd.vender_id == 0  && sd.trade_partner == 0)
		fail = npc_buylist(sd,(RFIFOW(fd,2)-4)/4, RFIFOP(fd,4));

	WFIFOW(fd,0)=0xca;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_len_table[0xca]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcSellListSend(int fd, struct map_session_data &sd)
{
	int fail=1;

	if( !session_isActive(fd) )
		return 0;

	if(sd.vender_id == 0  && sd.trade_partner == 0)
		fail = npc_selllist(sd, (RFIFOW(fd,2)-4)/4, RFIFOP(fd,4));

	WFIFOW(fd,0)=0xcb;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_len_table[0xcb]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_CreateChatRoom(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 4){
		chat_createchat(sd,RFIFOW(fd,4),RFIFOB(fd,6),(char*)RFIFOP(fd,7),(char*)RFIFOP(fd,15),RFIFOW(fd,2)-15);
	} else
		clif_skill_fail(sd,1,0,3);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatAddMember(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chat_joinchat(sd,RFIFOL(fd,2),(char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatRoomStatusChange(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chat_changechatstatus(sd,RFIFOW(fd,4),RFIFOB(fd,6),(char*)RFIFOP(fd,7),(char*)RFIFOP(fd,15),RFIFOW(fd,2)-15);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChangeChatOwner(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chat_changechatowner(sd,(char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_KickFromChat(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chat_kickchat(sd,(char*)RFIFOP(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatLeave(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chat_leavechat(sd);
	return 0;
}

/*==========================================
 * 取引要請を相手に送る
 *------------------------------------------
 */
int clif_parse_TradeRequest(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 1){
		trade_traderequest(sd,RFIFOL(sd.fd,2));
	} else
		clif_skill_fail(sd,1,0,0);
	return 0;
}

/*==========================================
 * 取引要請
 *------------------------------------------
 */
int clif_parse_TradeAck(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	trade_tradeack(sd,RFIFOB(sd.fd,2));
	return 0;
}

/*==========================================
 * アイテム追加
 *------------------------------------------
 */
int clif_parse_TradeAddItem(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	trade_tradeadditem(sd,RFIFOW(sd.fd,2),RFIFOL(sd.fd,4));
	return 0;
}

/*==========================================
 * アイテム追加完了(ok押し)
 *------------------------------------------
 */
int clif_parse_TradeOk(int fd, struct map_session_data &sd)
{
	trade_tradeok(sd);
	return 0;
}

/*==========================================
 * 取引キャンセル
 *------------------------------------------
 */
int clif_parse_TradeCancel(int fd, struct map_session_data &sd)
{
	trade_tradecancel(sd);
	return 0;
}

/*==========================================
 * 取引許諾(trade押し)
 *------------------------------------------
 */
int clif_parse_TradeCommit(int fd, struct map_session_data &sd)
{
	trade_tradecommit(sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_StopAttack(int fd, struct map_session_data &sd)
{
	sd.stop_attack();
	return 0;
}

/*==========================================
 * カートへアイテムを移す
 *------------------------------------------
 */
int clif_parse_PutItemToCart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0)
		return 0;
	pc_putitemtocart(sd,RFIFOW(fd,2)-2,RFIFOL(fd,4));
	return 0;
}
/*==========================================
 * カートからアイテムを出す
 *------------------------------------------
 */
int clif_parse_GetItemFromCart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0)
		return 0;
	pc_getitemfromcart(sd,RFIFOW(fd,2)-2,RFIFOL(fd,4));
	return 0;
}

/*==========================================
 * 付属品(鷹,ペコ,カート)をはずす
 *------------------------------------------
 */
int clif_parse_RemoveOption(int fd, struct map_session_data &sd)
{
	if(pc_isriding(sd)) {	// jobchange when removing peco [Valaris]
		if(sd.status.class_==13)
			sd.status.class_=sd.view_class=7;

		if(sd.status.class_==21)
			sd.status.class_=sd.view_class=14;

		if(sd.status.class_==4014)
			sd.status.class_=sd.view_class=4008;

		if(sd.status.class_==4022)
			sd.status.class_=sd.view_class=4015;

		if(sd.status.class_==4036)
			sd.status.class_=sd.view_class=4030;

		if(sd.status.class_==4044)
			sd.status.class_=sd.view_class=4037;
	}

	pc_setoption(sd,0);
	return 0;
}

/*==========================================
 * チェンジカート
 *------------------------------------------
 */
int clif_parse_ChangeCart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pc_setcart(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ステータスアップ
 *------------------------------------------
 */
int clif_parse_StatusUp(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	pc_statusup(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * スキルレベルアップ
 *------------------------------------------
 */
int clif_parse_SkillUp(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pc_skillup(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * スキル使用（ID指定）
 *------------------------------------------
 */
int clif_parse_UseSkillToId(int fd, struct map_session_data &sd) {
	unsigned short skillnum, skilllv, lv;
	uint32 target_id;
	unsigned long tick = gettick();

	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.chatID || sd.vender_id != 0 || pc_issit(sd) || pc_isdead(sd))
		return 0;

	skilllv = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	if (skilllv < 1) skilllv = 1; //No clue, I have seen the client do this with guild skills :/ [Skotlex]
	skillnum = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);
	target_id = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[2]);


	if (skillnotok(skillnum, sd))
		return 0;

	if (sd.skilltimer != -1)
	{
		if (skillnum != SA_CASTCANCEL)
			return 0;
	}
	else if (DIFF_TICK(tick, sd.canact_tick) < 0 &&
		// allow monk combos to ignore this delay [celest]
		!(sd.sc_data[SC_COMBO].timer!=-1 &&
		(skillnum == MO_EXTREMITYFIST ||
		skillnum == MO_CHAINCOMBO ||
		skillnum == MO_COMBOFINISH ||
		skillnum == CH_PALMSTRIKE ||
		skillnum == CH_TIGERFIST ||
		skillnum == CH_CHAINCRUSH)))
	{
		clif_skill_fail(sd, skillnum, 4, 0);
		return 0;
	}

	if ((sd.sc_data[SC_TRICKDEAD].timer != -1 && skillnum != NV_TRICKDEAD) ||
	    sd.sc_data[SC_BERSERK].timer != -1 || sd.sc_data[SC_NOCHAT].timer != -1 ||
	    sd.sc_data[SC_WEDDING].timer != -1 || sd.view_class == 22)
		return 0;
	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);
	
	if(target_id & FLAG_DISGUISE) // for disguises [Valaris]
		target_id &= ~FLAG_DISGUISE;
		
	if(sd.skillitem == skillnum)
	{
		if (skilllv != sd.skillitemlv)
			skilllv = sd.skillitemlv;
		skill_use_id(&sd, target_id, skillnum, skilllv);
	}
	else
	{
		sd.skillitem = sd.skillitemlv = 0xFFFF;
		if (skillnum == MO_EXTREMITYFIST) {
			if ((sd.sc_data[SC_COMBO].timer == -1 ||
				(sd.sc_data[SC_COMBO].val1.num != MO_COMBOFINISH &&
				 sd.sc_data[SC_COMBO].val1.num != CH_TIGERFIST &&
				 sd.sc_data[SC_COMBO].val1.num != CH_CHAINCRUSH)) )
			{
				if (!sd.state.skill_flag )
				{
					sd.state.skill_flag = 1;
					clif_skillinfo(sd, MO_EXTREMITYFIST, 1, -1);
					return 0;
				}
				else if (sd.block_list::id == target_id)
				{
					clif_skillinfo(sd, MO_EXTREMITYFIST, 1, -1);
					return 0;
				}
			}
		} else if (skillnum == CH_TIGERFIST) {
			if (sd.sc_data[SC_COMBO].timer == -1 || sd.sc_data[SC_COMBO].val1.num != MO_COMBOFINISH) {
				if (!sd.state.skill_flag ) {
					sd.state.skill_flag = 1;
					if (!sd.target_id) {
						clif_skillinfo(sd, CH_TIGERFIST, 1, -2);
						return 0;
					} else
						target_id = sd.target_id;
				} else if (sd.block_list::id == target_id) {
					clif_skillinfo(sd, CH_TIGERFIST, 1, -2);
					return 0;
				}
			}
		}
		if ((lv = pc_checkskill(sd, skillnum)) > 0) {
			if (skilllv > lv)
				skilllv = lv;
			skill_use_id(&sd, target_id, skillnum, skilllv);
			if (sd.state.skill_flag)
				sd.state.skill_flag = 0;
		}
	}
	return 0;
}

/*==========================================
 * スキル使用（場所指定）
 *------------------------------------------
 */
int clif_parse_UseSkillToPos(int fd, struct map_session_data &sd)
{
	unsigned short skillnum, skilllv, lv, x, y;
	unsigned long tick = gettick();

	if( !session_isActive(fd) )
		return 0;

	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.chatID || pc_issit(sd) || pc_isdead(sd))
		return 0;

	skilllv = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	skillnum = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);
	x = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[2]);
	y = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[3]);
	if (skillnotok(skillnum, sd))
		return 0;

	if(packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[4] > 0)
	{
		int skillmoreinfo = packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[4];
		if( pc_issit(sd) )
		{
			clif_skill_fail(sd, skillnum, 0, 0);
			return 0;
		}
		memcpy(talkie_mes, RFIFOP(fd,skillmoreinfo), 80);
		talkie_mes[79]=0;
	}

	if (sd.skilltimer != -1)
		return 0;
	else if( DIFF_TICK(tick, sd.canact_tick) < 0 &&
		// allow monk combos to ignore this delay [celest]
		!( sd.sc_data[SC_COMBO].timer!=-1 &&
		 (skillnum == MO_EXTREMITYFIST || skillnum == MO_CHAINCOMBO || skillnum == MO_COMBOFINISH ||
		  skillnum == CH_PALMSTRIKE || skillnum == CH_TIGERFIST || skillnum == CH_CHAINCRUSH)) )
	{
		clif_skill_fail(sd, skillnum, 4, 0);
		return 0;
	}

	if ((sd.sc_data[SC_TRICKDEAD].timer != -1 && skillnum != NV_TRICKDEAD) ||
	    sd.sc_data[SC_BERSERK].timer != -1 || sd.sc_data[SC_NOCHAT].timer != -1 ||
	    sd.sc_data[SC_WEDDING].timer != -1 || sd.view_class == 22)
		return 0;
	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);
	if(sd.skillitem == skillnum) {
		if (skilllv != sd.skillitemlv)
			skilllv = sd.skillitemlv;
		skill_use_pos(&sd, x, y, skillnum, skilllv);
	} else {
		sd.skillitem = sd.skillitemlv = 0xFFFF;
		if ((lv = pc_checkskill(sd, skillnum)) > 0) {
			if (skilllv > lv)
				skilllv = lv;
			skill_use_pos(&sd, x, y, skillnum,skilllv);
		}
	}
	return 0;
}

/*==========================================
 * スキル使用（map指定）
 *------------------------------------------
 */
int clif_parse_UseSkillMap(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(sd.chatID) return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || (sd.sc_data &&
		(sd.sc_data[SC_TRICKDEAD].timer != -1 ||
		sd.sc_data[SC_BERSERK].timer!=-1 ||
		sd.sc_data[SC_NOCHAT].timer!=-1 ||
		sd.sc_data[SC_WEDDING].timer!=-1 ||
		sd.view_class==22)))
		return 0;

	if(sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);

	skill_castend_map(&sd,RFIFOW(fd,2),(char*)RFIFOP(fd,4));
	return 0;
}
/*==========================================
 * メモ要求
 *------------------------------------------
 */
int clif_parse_RequestMemo(int fd, struct map_session_data &sd)
{
	if (!pc_isdead(sd))
		pc_memo(sd,-1);
	return 0;
}
/*==========================================
 * アイテム合成
 *------------------------------------------
 */
int clif_parse_ProduceMix(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	return skill_produce_mix(sd,RFIFOW(fd,2),RFIFOW(fd,4),RFIFOW(fd,6),RFIFOW(fd,8));
}
/*==========================================
 * 武器修理
 *------------------------------------------
 */
int clif_parse_RepairItem(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	return pc_item_repair(sd, RFIFOW(fd,2));
}

int clif_parse_WeaponRefine(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	return pc_item_refine(sd, RFIFOW(fd, packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0])-2);
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcSelectMenu(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	sd.ScriptEngine.cExtData = RFIFOB(fd,6);
	npc_scriptcont(sd, RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcNextClicked(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	sd.ScriptEngine.cExtData = 0;
	npc_scriptcont(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcAmountInput(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	sd.ScriptEngine.cExtData = RFIFOL(fd,6);
	npc_scriptcont(sd, RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcStringInput(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	sd.ScriptEngine.cExtData = (char*)RFIFOP(fd,8);
	npc_scriptcont(sd,RFIFOL(fd,4));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcCloseClicked(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	sd.ScriptEngine.cExtData = 1;
	npc_scriptcont(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * アイテム鑑定
 *------------------------------------------
 */
int clif_parse_ItemIdentify(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pc_item_identify(sd,RFIFOW(fd,2)-2);
	return 0;
}
/*==========================================
 * 矢作成
 *------------------------------------------
 */
int clif_parse_SelectArrow(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	return skill_arrow_create(&sd,RFIFOW(fd,2));
}
/*==========================================
 * オートスペル受信
 *------------------------------------------
 */
int clif_parse_AutoSpell(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	skill_autospell(&sd,RFIFOW(fd,2));
	return 0;
}
/*==========================================
 * カード使用
 *------------------------------------------
 */
int clif_parse_UseCard(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if (sd.trade_partner != 0)
		return 0;
	clif_use_card(sd,RFIFOW(fd,2)-2);
	return 0;
}
/*==========================================
 * カード挿入装備選択
 *------------------------------------------
 */
int clif_parse_InsertCard(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if (sd.trade_partner != 0)
		return 0;
	pc_insert_card(sd,RFIFOW(fd,2)-2,RFIFOW(fd,4)-2);
	return 0;
}

/*==========================================
 * 0193 キャラID名前引き
 *------------------------------------------
 */
int clif_parse_SolveCharName(int fd, struct map_session_data &sd) {
	int char_id;
	if( !session_isActive(fd) )
		return 0;

	char_id = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);

	return clif_solved_charname(sd, char_id);
}

/*==========================================
 * 0197 /resetskill /resetstate
 *------------------------------------------
 */
int clif_parse_ResetChar(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
		sd.isGM() >= get_atcommand_level(AtCommand_ResetState) )
	{
		switch(RFIFOW(fd,2)){
		case 0:
			pc_resetstate(sd);
			break;
		case 1:
			pc_resetskill(sd);
			break;
		}
	}
	return 0;
}

/*==========================================
 * 019c /lb等
 *------------------------------------------
 */
int clif_parse_LGMmessage(int fd, struct map_session_data &sd) {
	unsigned char buf[512];

	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= get_atcommand_level(AtCommand_LocalBroadcast) )
	{
		WBUFW(buf,0) = 0x9a;
		WBUFW(buf,2) = RFIFOW(fd,2);
		memcpy(WBUFP(buf,4), RFIFOP(fd,4), RFIFOW(fd,2) - 4);
		clif_send_same_map(sd, buf, RFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * カプラ倉庫へ入れる
 *------------------------------------------
 */
int clif_parse_MoveToKafra(int fd, struct map_session_data &sd) {
	int item_index, item_amount;

	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;

	item_index = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0])-2;
	item_amount = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);


	if (item_index < 0 || item_index >= MAX_INVENTORY)
		return 0;

	if (sd.state.storage_flag)
		storage_guild_storageadd(sd, item_index, item_amount);
	else
		storage_storageadd(sd, item_index, item_amount);
	return 0;
}

/*==========================================
 * カプラ倉庫から出す
 *------------------------------------------
 */
int clif_parse_MoveFromKafra(int fd, struct map_session_data &sd) {
	int item_index, item_amount;

	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;

	item_index = RFIFOW(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0])-1;
	item_amount = RFIFOL(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[1]);

	if (sd.state.storage_flag)
		storage_guild_storageget(sd, item_index, item_amount);
	else
		storage_storageget(sd, item_index, item_amount);
	return 0;
}

/*==========================================
 * カプラ倉庫へカートから入れる
 *------------------------------------------
 */
int clif_parse_MoveToKafraFromCart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0 )
		return 0;
	if( sd.state.storage_flag )
		storage_guild_storageaddfromcart(sd, RFIFOW(fd,2) - 2, RFIFOL(fd,4));
	else
		storage_storageaddfromcart(sd, RFIFOW(fd,2) - 2, RFIFOL(fd,4));
	return 0;
}

/*==========================================
 * カプラ倉庫から出す
 *------------------------------------------
 */
int clif_parse_MoveFromKafraToCart(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0 )
		return 0;
	if( sd.state.storage_flag )
		storage_guild_storagegettocart(sd, RFIFOW(fd,2)-1, RFIFOL(fd,4));
	else
		storage_storagegettocart(sd, RFIFOW(fd,2)-1, RFIFOL(fd,4));
	return 0;
}

/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
int clif_parse_CloseKafra(int fd, struct map_session_data &sd)
{

	if (sd.state.storage_flag)
		storage_guild_storageclose(sd);
	else
		storage_storageclose(sd);
	return 0;
}

/*==========================================
 * パーティを作る
 *------------------------------------------
 */
int clif_parse_CreateParty(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if (config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 7) {
		party_create(sd,(const char*)RFIFOP(fd,2),0,0);
	} else
		clif_skill_fail(sd,1,0,4);
	return 0;
}

/*==========================================
 * パーティを作る
 *------------------------------------------
 */
int clif_parse_CreateParty2(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if (config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 7){
		party_create(sd,(const char*)RFIFOP(fd,2),RFIFOB(fd,26),RFIFOB(fd,27));
	} else
		clif_skill_fail(sd,1,0,4);
	return 0;
}

/*==========================================
 * パーティに勧誘
 *------------------------------------------
 */
int clif_parse_PartyInvite(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	party_invite(sd, RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * パーティ勧誘返答
 *------------------------------------------
 */
int clif_parse_ReplyPartyInvite(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 5){
		party_reply_invite(sd,RFIFOL(fd,2),RFIFOL(fd,6));
	} else {
		party_reply_invite(sd,RFIFOL(fd,2),-1);
		clif_skill_fail(sd,1,0,4);
	}
	return 0;
}

/*==========================================
 * パーティ脱退要求
 *------------------------------------------
 */
int clif_parse_LeaveParty(int fd, struct map_session_data &sd)
{
	party_leave(sd);
	return 0;
}

/*==========================================
 * パーティ除名要求
 *------------------------------------------
 */
int clif_parse_RemovePartyMember(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	party_removemember(sd,RFIFOL(fd,2),(char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 * パーティ設定変更要求
 *------------------------------------------
 */
int clif_parse_PartyChangeOption(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	party_changeoption(sd, RFIFOW(fd,2), RFIFOW(fd,4));
	return 0;
}

/*==========================================
 * パーティメッセージ送信要求
 *------------------------------------------
 */
int clif_parse_PartyMessage(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;


	if (is_charcommand(fd, sd, (char*)RFIFOP(fd,4), 0) != CharCommand_None ||
		is_atcommand(fd, sd, (char*)RFIFOP(fd,4), 0) != AtCommand_None ||
		(sd.sc_data && 
		(sd.sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd.sc_data[SC_NOCHAT].timer!=-1)))		//チャット禁止
		return 0;

	party_send_message(sd, (const char*)RFIFOP(fd,4), RFIFOW(fd,2)-4);
	return 0;
}

/*==========================================
 * 露店閉鎖
 *------------------------------------------
 */
int clif_parse_CloseVending(int fd, struct map_session_data &sd)
{
	vending_closevending(sd);
	return 0;
}

/*==========================================
 * 露店アイテムリスト要求
 *------------------------------------------
 */
int clif_parse_VendingListReq(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if(sd.trade_partner == 0)
		vending_vendinglistreq(sd,RFIFOL(fd,2));
//	if( sd.ScriptEngine.isRunning() )
//		npc_event_dequeue(sd);
	return 0;
}

/*==========================================
 * 露店アイテム購入
 *------------------------------------------
 */
int clif_parse_PurchaseReq(int fd, struct map_session_data &sd)
 {
	if( !session_isActive(fd) )
		return 0;
	if(sd.trade_partner == 0)
		vending_purchasereq(sd, RFIFOW(fd,2), RFIFOL(fd,4), RFIFOP(fd,8));
	return 0;
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
int clif_parse_OpenVending(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if(sd.vender_id == 0  && sd.trade_partner == 0)
		vending_openvending(sd, RFIFOW(fd,2), (char*)RFIFOP(fd,4), RFIFOB(fd,84), RFIFOP(fd,85));
	return 0;
}

/*==========================================
 * /monster /item rewriten by [Yor]
 *------------------------------------------
 */
int clif_parse_GM_Monster_Item(int fd, struct map_session_data &sd)
{
	char monster_item_name[32]="";

	if( !session_isActive(fd) )
		return 0;
	if(config.atc_gmonly == 0 || sd.isGM())
	{
		memcpy(monster_item_name, RFIFOP(fd,2), 24);
		if(mobdb_searchname(monster_item_name) != 0) {
			if(sd.isGM() >= get_atcommand_level(AtCommand_Monster))
				atcommand_spawn(fd, sd, "@spawn", monster_item_name); // as @spawn
		} else if(itemdb_searchname(monster_item_name) != NULL) {
			if(sd.isGM() >= get_atcommand_level(AtCommand_Item))
				atcommand_item(fd, sd, "@item", monster_item_name); // as @item
		}

	}
	return 0;
}

/*==========================================
 * ギルドを作る
 *------------------------------------------
 */
int clif_parse_CreateGuild(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_create(sd, (char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 * ギルドマスターかどうか確認
 *------------------------------------------
 */
int clif_parse_GuildCheckMaster(int fd, struct map_session_data &sd)
{
	clif_guild_masterormember(sd);
	return 0;
}

/*==========================================
 * ギルド情報要求
 *------------------------------------------
 */
int clif_parse_GuildRequestInfo(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOL(fd,2)){
	case 0:	// ギルド基本情報、同盟敵対情報
		clif_guild_basicinfo(sd);
		clif_guild_allianceinfo(sd);
		break;
	case 1:	// メンバーリスト、役職名リスト
		clif_guild_positionnamelist(sd);
		clif_guild_memberlist(sd);
		break;
	case 2:	// 役職名リスト、役職情報リスト
		clif_guild_positionnamelist(sd);
		clif_guild_positioninfolist(sd);
		break;
	case 3:	// スキルリスト
		clif_guild_skillinfo(sd);
		break;
	case 4:	// 追放リスト
		clif_guild_explusionlist(sd);
		break;
	default:
		if (config.error_log)
			ShowMessage("clif: guild request info: unknown type %ld\n", (unsigned long)RFIFOL(fd,2));
		break;
	}
	return 0;
}

/*==========================================
 * ギルド役職変更
 *------------------------------------------
 */
int clif_parse_GuildChangePositionInfo(int fd, struct map_session_data &sd)
{
	int i;
	if( !session_isActive(fd) )
		return 0;
	for(i = 4; i < RFIFOW(fd,2); i += 40 ){
		guild_change_position(sd, RFIFOL(fd,i), RFIFOL(fd,i+4), RFIFOL(fd,i+12), (char*)RFIFOP(fd,i+16));
	}
	return 0;
}

/*==========================================
 * ギルドメンバ役職変更
 *------------------------------------------
 */
int clif_parse_GuildChangeMemberPosition(int fd, struct map_session_data &sd)
{
	int i;
	if( !session_isActive(fd) )
		return 0;
	for(i=4;i<RFIFOW(fd,2);i+=12){
		guild_change_memberposition(sd.status.guild_id,RFIFOL(fd,i),RFIFOL(fd,i+4),RFIFOL(fd,i+8));
	}
	return 0;
}

/*==========================================
 * ギルドエンブレム要求
 *------------------------------------------
 */
int clif_parse_GuildRequestEmblem(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	struct guild *g=guild_search(RFIFOL(fd,2));
	if(g!=NULL)
		clif_guild_emblem(sd,*g);
	return 0;
}

/*==========================================
 * ギルドエンブレム変更
 *------------------------------------------
 */
int clif_parse_GuildChangeEmblem(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_change_emblem(sd,RFIFOW(fd,2)-4,RFIFOP(fd,4));
	return 0;
}

/*==========================================
 * ギルド告知変更
 *------------------------------------------
 */
int clif_parse_GuildChangeNotice(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_change_notice(sd,RFIFOL(fd,2),(char*)RFIFOP(fd,6),(char*)RFIFOP(fd,66));
	return 0;
}

/*==========================================
 * ギルド勧誘
 *------------------------------------------
 */
int clif_parse_GuildInvite(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_invite(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * ギルド勧誘返信
 *------------------------------------------
 */
int clif_parse_GuildReplyInvite(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_reply_invite(sd,RFIFOL(fd,2),RFIFOB(fd,6));
	return 0;
}

/*==========================================
 * ギルド脱退
 *------------------------------------------
 */
int clif_parse_GuildLeave(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_leave(sd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),(char*)RFIFOP(fd,14));
	return 0;
}

/*==========================================
 * ギルド追放
 *------------------------------------------
 */
int clif_parse_GuildExplusion(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_explusion(sd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),(char*)RFIFOP(fd,14));
	return 0;
}

/*==========================================
 * ギルド会話
 *------------------------------------------
 */
int clif_parse_GuildMessage(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;


	if (is_charcommand(fd, sd, (char*)RFIFOP(fd, 4), 0) != CharCommand_None ||
		is_atcommand(fd, sd, (char*)RFIFOP(fd, 4), 0) != AtCommand_None ||
		(sd.sc_data &&
		(sd.sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd.sc_data[SC_NOCHAT].timer!=-1)))		//チャット禁止
		return 0;

	guild_send_message(sd, (char*)RFIFOP(fd,4), RFIFOW(fd,2)-4);
	return 0;
}

/*==========================================
 * ギルド同盟要求
 *------------------------------------------
 */
int clif_parse_GuildRequestAlliance(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_reqalliance(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * ギルド同盟要求返信
 *------------------------------------------
 */
int clif_parse_GuildReplyAlliance(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_reply_reqalliance(sd,RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}

/*==========================================
 * ギルド関係解消
 *------------------------------------------
 */
int clif_parse_GuildDelAlliance(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_delalliance(sd,RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}

/*==========================================
 * ギルド敵対
 *------------------------------------------
 */
int clif_parse_GuildOpposition(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_opposition(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * ギルド解散
 *------------------------------------------
 */
int clif_parse_GuildBreak(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_break(sd,(char*)RFIFOP(fd,2));
	return 0;
}

// pet
int clif_parse_PetMenu(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pet_menu(sd,RFIFOB(fd,2));
	return 0;
}

int clif_parse_CatchPet(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pet_catch_process2(sd,RFIFOL(fd,2));
	return 0;
}

int clif_parse_SelectEgg(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pet_select_egg(sd,RFIFOW(fd,2)-2);
	return 0;
}

int clif_parse_SendEmotion(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(sd.pd)
		clif_pet_emotion(*sd.pd,RFIFOL(fd,2));
	return 0;
}

int clif_parse_ChangePetName(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	pet_change_name(sd,(const char*)RFIFOP(fd,2));
	return 0;
}

// Kick (right click menu for GM "(name) force to quit")
int clif_parse_GMKick(int fd, struct map_session_data &sd)
{
	struct block_list *target;

	if( !session_isActive(fd) )
		return 0;

	int tid = RFIFOL(fd,2);

	if ((config.atc_gmonly == 0 || sd.isGM()) &&
	    (sd.isGM() >= get_atcommand_level(AtCommand_Kick))) {
		target = map_id2bl(tid);
		if (target) {
			if (target->type == BL_PC) {
				struct map_session_data *tsd = (struct map_session_data *)target;
				if (sd.isGM() > tsd->isGM())
					clif_GM_kick(sd, *tsd, 1);
				else
					clif_GM_kickack(sd, 0);
			} else if (target->type == BL_MOB) {
				struct mob_data *md = (struct mob_data *)target;
				sd.state.attack_type = 0;
				mob_damage(*md, md->hp, 2, &sd);
			} else
				clif_GM_kickack(sd, 0);
		} else
			clif_GM_kickack(sd, 0);
	}
	return 0;
}

/*==========================================
 * /shift
 *------------------------------------------
 */
int clif_parse_Shift(int fd, struct map_session_data &sd)
{
	char player_name[32]="";

	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= get_atcommand_level(AtCommand_JumpTo) )
	{
		memcpy(player_name, RFIFOP(fd,2), 24);
		atcommand_jumpto(fd, sd, "@jumpto", player_name); // as @jumpto
	}

	return 0;
}

/*==========================================
 * /recall
 *------------------------------------------
 */
int clif_parse_Recall(int fd, struct map_session_data &sd)
{
	char player_name[32];

	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
		sd.isGM() >= get_atcommand_level(AtCommand_Recall) )
	{
		memcpy(player_name, RFIFOP(fd,2), 24);
		atcommand_recall(fd, sd, "@recall", player_name); // as @recall
	}

	return 0;
}

int clif_parse_GMHide(int fd, struct map_session_data &sd)
{	// Modified by [Yor]

	//ShowMessage("%2x %2x %2x\n", (unsigned short)RFIFOW(fd,0), (unsigned short)RFIFOW(fd,2), (unsigned short)RFIFOW(fd,4)); // R 019d <Option_value>.2B <flag>.2B
	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= get_atcommand_level(AtCommand_Hide) )
	{
		if (sd.status.option & OPTION_HIDE) { // OPTION_HIDE = 0x40
			sd.status.option &= ~OPTION_HIDE; // OPTION_HIDE = 0x40
			clif_displaymessage(fd, "Invisible: Off.");
		} else {
			sd.status.option |= OPTION_HIDE; // OPTION_HIDE = 0x40
			clif_displaymessage(fd, "Invisible: On.");
		}
		clif_changeoption(sd);
	}
	return 0;
}

/*==========================================
 * GMによるチャット禁止時間付与
 *------------------------------------------
 */
int clif_parse_GMReqNoChat(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	ushort cmd = RFIFOW(fd,0);
	uint32 tid = RFIFOL(fd,packet_db[sd.packet_ver][cmd].pos[0]);
	int type   = RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[1]);
	int limit  = RFIFOW(fd,packet_db[sd.packet_ver][cmd].pos[2]);
	struct block_list *bl = map_id2bl(tid);
	
	if( !session_isActive(fd) )
		return 0;

	if(!config.muting_players) {
		clif_displaymessage(fd, "Muting is disabled.");
		return 0;
	}

	if( bl && bl->type == BL_PC && tid==bl->id && sd.block_list::id != bl->id )
	{
		struct map_session_data *dstsd =(struct map_session_data *)bl;
		if( sd.isGM()>dstsd->isGM() && sd.isGM() >= get_atcommand_level(AtCommand_Mute) )
		{
			int dstfd = dstsd->fd;
			if( session_isActive(dstfd) )
			{
				WFIFOW(dstfd,0)=0x14b;
				WFIFOB(dstfd,2)=(type==2)?1:type;
				memcpy(WFIFOP(dstfd,3),sd.status.name,24);
				WFIFOSET(dstfd,packet_len_table[0x14b]);
			}

			dstsd->status.manner -= (type == 0)?-limit:+limit;

			if(dstsd->status.manner < 0)
				status_change_start(bl,SC_NOCHAT,0,0,0,0,0,0);
			else
			{
				dstsd->status.manner = 0;
				status_change_end(bl,SC_NOCHAT,-1);
			}
			ShowMessage("name:%s type:%d limit:%d manner:%d\n",dstsd->status.name,type,limit,dstsd->status.manner);
		}
	}
	return 0;
}
/*==========================================
 * GMによるチャット禁止時間参照（？）
 *------------------------------------------
 */
int clif_parse_GMReqNoChatCount(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	int tid = RFIFOL(fd,2);

	WFIFOW(fd,0) = 0x1e0;
	WFIFOL(fd,2) = tid;
	snprintf((char*)WFIFOP(fd,6),24,"%d",tid);
//	memcpy(WFIFOP(fd,6), "TESTNAME", 24);
	WFIFOSET(fd, packet_len_table[0x1e0]);

	return 0;
}

int clif_parse_PMIgnore(int fd, struct map_session_data &sd)
{
	char output[512]="";
	char *nick; // S 00cf <nick>.24B <type>.B: 00 (/ex nick) deny speech from nick, 01 (/in nick) allow speech from nick
	int i;

	if( !session_isActive(fd) )
		return 0;

	nick = (char*)RFIFOP(fd,2);
	nick[23] = '\0'; // to be sure that the player name have at maximum 23 characters
	ShowDebug("Ignore: char '%s' state: %d\n", nick, (unsigned char)RFIFOB(fd,26));

	WFIFOW(fd,0) = 0x0d1; // R 00d1 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
	WFIFOB(fd,2) = RFIFOB(fd,26);
	// do nothing only if nick can not exist
	if( strlen(nick) < 4 )
	{
		WFIFOB(fd,3) = 1; // fail
		WFIFOSET(fd, packet_len_table[0x0d1]);
		if (RFIFOB(fd,26) == 0) // type
			clif_wis_message(fd, wisp_server_name, "It's impossible to block this player.", strlen("It's impossible to block this player.") + 1);
		else
			clif_wis_message(fd, wisp_server_name, "It's impossible to unblock this player.", strlen("It's impossible to unblock this player.") + 1);
	}
	else
	{
		// deny action (we add nick only if it's not already exist
		if (RFIFOB(fd,26) == 0)
		{	
			for(i = 0; i < MAX_IGNORE_LIST; ++i)
			{
				if( strcmp(sd.ignore[i].name, nick) == 0 ||
					sd.ignore[i].name[0] == '\0' )
					break;
			}
			if( i<MAX_IGNORE_LIST && sd.ignore[i].name[0] != '\0' )
			{	// failed, found it
				WFIFOB(fd,3) = 1; // fail
				WFIFOSET(fd, packet_len_table[0x0d1]);
				clif_wis_message(fd, wisp_server_name, "This player is already blocked.", strlen("This player is already blocked.") + 1);
				if (strcmp(wisp_server_name, nick) == 0)
				{	// to find possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %ld) has tried AGAIN to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
					intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
				}
			}
			else if( i>=MAX_IGNORE_LIST )
			{	// failed, list full
				WFIFOB(fd,3) = 1; 
				WFIFOSET(fd, packet_len_table[0x0d1]);
				clif_wis_message(fd, wisp_server_name, "You can not block more people.", strlen("You can not block more people.") + 1);
				if (strcmp(wisp_server_name, nick) == 0) { // to found possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %ld) has tried to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
					intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
				}
			}
			else
			{	// success
				memcpy(sd.ignore[i].name, nick, 24);
				WFIFOB(fd,3) = 0; 
				WFIFOSET(fd, packet_len_table[0x0d1]);
				if (strcmp(wisp_server_name, nick) == 0)
				{	// to find possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %ld) has tried to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
					intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
					// send something to be inform and force bot to ignore twice... ifGM receiving block + block again, it's a bot :)
					clif_wis_message(fd, wisp_server_name, "Add me in your ignore list, doesn't block my wisps.", 1+strlen("Add me in your ignore list, doesn't block my wisps."));
				}
			}
		}
		else
		{	// allow action (we remove all same nicks if they exist)
			for(i=0; i<MAX_IGNORE_LIST; ++i)
			{
				if( strcmp(sd.ignore[i].name, nick) == 0 ||
					sd.ignore[i].name[0] == '\0' )
					break;
			}
			if( i>=MAX_IGNORE_LIST || sd.ignore[i].name[0] == '\0' )
			{	// fail, not found
				WFIFOB(fd,3) = 1; 
				WFIFOSET(fd, packet_len_table[0x0d1]);
				clif_wis_message(fd, wisp_server_name, "This player is not blocked by you.", strlen("This player is not blocked by you.") + 1);
			}
			else
			{	// move elements from i+1...MAX_IGNORE_LIST to i and clear the last
				memmove(sd.ignore+i, sd.ignore+i+1, (MAX_IGNORE_LIST-i-1)*sizeof(sd.ignore[0]));
				sd.ignore[MAX_IGNORE_LIST-1].name[0]=0;
				WFIFOB(fd,3) = 0; // success
				WFIFOSET(fd, packet_len_table[0x0d1]);
			}
		}
#ifdef DEBUG
		for(i = 0; i < MAX_IGNORE_LIST; ++i) // for debug only
			if (sd.ignore[i].name[0] != '\0')
				ShowDebug("Ignored player: '%s'\n", sd.ignore[i].name);
#endif
	}
	return 0;
}

int clif_parse_PMIgnoreAll(int fd, struct map_session_data &sd)
{ // Rewritten by [Yor]
	//ShowMessage("Ignore all: state: %d\n", (unsigned char)RFIFOB(fd,2));
	if( !session_isActive(fd) )
		return 0;

	if (RFIFOB(fd,2) == 0) {// S 00d0 <type>len.B: 00 (/exall) deny all speech, 01 (/inall) allow all speech
		WFIFOW(fd,0) = 0x0d2; // R 00d2 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
		WFIFOB(fd,2) = 0;
		if(sd.state.ignoreAll == 0) {
			sd.state.ignoreAll = 1;
			WFIFOB(fd,3) = 0; // success
			WFIFOSET(fd, packet_len_table[0x0d2]);
		} else {
			WFIFOB(fd,3) = 1; // fail
			WFIFOSET(fd, packet_len_table[0x0d2]);
			clif_wis_message(fd, wisp_server_name, "You already block everyone.", strlen("You already block everyone.") + 1);
		}
	} else {
		WFIFOW(fd,0) = 0x0d2; // R 00d2 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
		WFIFOB(fd,2) = 1;
		if(sd.state.ignoreAll == 1) {
			sd.state.ignoreAll = 0;
			WFIFOB(fd,3) = 0; // success
			WFIFOSET(fd, packet_len_table[0x0d2]);
		} else {
			WFIFOB(fd,3) = 1; // fail
			WFIFOSET(fd, packet_len_table[0x0d2]);
			clif_wis_message(fd, wisp_server_name, "You already allow everyone.", strlen("You already allow everyone.") + 1);
		}
	}
	return 0;
}

/*==========================================
 * Wis拒否リスト
 *------------------------------------------
 */
int pstrcmp(const void *a, const void *b)
{
	return strcmp((char *)a, (char *)b);
}
int clif_parse_PMIgnoreList(int fd, struct map_session_data &sd)
{
	int i,j=0,count=0;
	if( !session_isActive(fd) )
		return 0;

	qsort (sd.ignore[0].name, MAX_IGNORE_LIST, sizeof(sd.ignore[0].name), pstrcmp);

	for(i = 0; i < MAX_IGNORE_LIST; ++i){	//中身があるのを数える
		if(sd.ignore[i].name[0] != 0)
			count++;
	}
	WFIFOW(fd,0) = 0xd4;
	WFIFOW(fd,2) = 4 + (24 * count);
	for(i = 0; i < MAX_IGNORE_LIST; ++i){
		if(sd.ignore[i].name[0] != 0){
			memcpy(WFIFOP(fd, 4 + j * 24),sd.ignore[i].name, 24);
			j++;
		}
	}
	WFIFOSET(fd, WFIFOW(fd,2));
	if(count >= MAX_IGNORE_LIST)	//満タンなら最後の1個を消す
		sd.ignore[MAX_IGNORE_LIST - 1].name[0] = 0;

	return 0;
}

/*==========================================
 * スパノビの/doridoriによるSPR2倍
 *------------------------------------------
 */
int clif_parse_NoviceDoriDori(int fd, struct map_session_data &sd)
{
	sd.doridori_counter = 1;
	return 0;
}
/*==========================================
 * スパノビの爆裂波動
 *------------------------------------------
 */
int clif_parse_NoviceExplosionSpirits(int fd, struct map_session_data &sd)
{
		int nextbaseexp=pc_nextbaseexp(sd);
	struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);
		if (config.etc_log){
			if(nextbaseexp != 0)
			ShowMessage("SuperNovice explosionspirits!! %d %d %d %d\n",sd.block_list::id,s_class.job,sd.status.base_exp,1000*sd.status.base_exp/nextbaseexp);
			else
			ShowMessage("SuperNovice explosionspirits!! %d %d %d 000\n",sd.block_list::id,s_class.job,sd.status.base_exp);
		}
	if(s_class.job == 23 && sd.status.base_exp > 0 && nextbaseexp > 0 && (1000*sd.status.base_exp/nextbaseexp)%100==0){
		clif_skill_nodamage(sd,sd,MO_EXPLOSIONSPIRITS,5,1);
		status_change_start(&sd,SkillStatusChangeTable[MO_EXPLOSIONSPIRITS],5,0,0,0,skill_get_time(MO_EXPLOSIONSPIRITS,5),0 );
		}
	return 0;
	}

// random notes:
// 0x214: forging chance?

/*==========================================
 * Friends List
 *------------------------------------------
 */
int clif_friendslist_send(struct map_session_data &sd)
{
	size_t i, n;

	if( !session_isActive(sd.fd) )
		return 0;

	// Send friends list
	WFIFOW(sd.fd, 0) = 0x201;
	for(i=0,n=4; i<MAX_FRIENDLIST; ++i)
	{
		if(sd.status.friendlist[i].friend_id)
		{
			WFIFOL(sd.fd, n+0) = (map_charid2sd(sd.status.friendlist[i].friend_id) != NULL);
			WFIFOL(sd.fd, n+4) = sd.status.friendlist[i].friend_id;
			memcpy(WFIFOP(sd.fd, n+8), &sd.status.friendlist[i].friend_name, 24);
			n += 32;
		}
}
	WFIFOW(sd.fd,2) = n;
	WFIFOSET(sd.fd, WFIFOW(sd.fd,2));
	return 0;
}

// Status for adding friend - 0: successfull 1: not exist/rejected 2: over limit
int clif_friendslist_reqack(struct map_session_data &sd, char *name, unsigned short type)
{
	int fd;

	fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x209;
	WFIFOW(fd,2) = type;
	if (type != 2)
		memcpy(WFIFOP(fd, 12), name, 24);
	WFIFOSET(fd, packet_len_table[0x209]);
	return 0;
}

int clif_parse_FriendsListAdd(int fd, struct map_session_data &sd)
{
	struct map_session_data *f_sd;
	size_t i, count;
	int f_fd;

	if( !session_isActive(fd) )
		return 0;

	f_sd = map_nick2sd((char*)RFIFOP(fd,2));

	if(f_sd == NULL)
	{	// Friend doesn't exist (no player with this name)
		clif_displaymessage(fd, msg_txt(3));
	}
	else
	{
		for(i=0, count=0; i < MAX_FRIENDLIST; ++i)
		{
			if (sd.status.friendlist[i].friend_id != 0)
			count++;
				if( sd.status.friendlist[i].friend_id == f_sd->status.char_id )
				{	// Friend already exists
					return clif_displaymessage(fd, "Friend already exists.");
		}
	}
	f_fd = f_sd->fd;
	WFIFOW(f_fd,0) = 0x207;
	WFIFOL(f_fd,2) = sd.status.char_id;
	WFIFOL(f_fd,6) = sd.block_list::id;
	memcpy(WFIFOP(f_fd,10), sd.status.name, 24);
	WFIFOSET(f_fd, packet_len_table[0x207]);
}
	return 0;
}

int clif_parse_FriendsListReply(int fd, struct map_session_data &sd)
{	//<W: id> <L: Player 1 chara ID> <L: Player 1 AID> <B: Response>
	struct map_session_data *f_sd;
	uint32 char_id, id;
	char reply;

	if( !session_isActive(fd) )
		return 0;

	char_id = RFIFOL(fd,2);
	id = RFIFOL(fd,6);
	reply = RFIFOB(fd,10);
//	ShowMessage ("reply: %d %d %d\n", char_id, id, reply);

	f_sd = map_id2sd(id);
	if (f_sd == NULL)
		return 0;

	if (reply == 0)
		clif_friendslist_reqack(*f_sd, sd.status.name, 1);
	else
	{
		size_t i;
		// Find an empty slot
		for (i = 0; i < MAX_FRIENDLIST; ++i)
			if (f_sd->status.friendlist[i].friend_id == 0)
				break;
		if(i == MAX_FRIENDLIST)
		{
			clif_friendslist_reqack(*f_sd, sd.status.name, 2);
		}
		else
		{
			f_sd->status.friendlist[i].friend_id = sd.status.char_id;
			memset(f_sd->status.friendlist[i].friend_name, 0, sizeof(f_sd->status.friendlist[i].friend_name));
			memcpy(f_sd->status.friendlist[i].friend_name, sd.status.name, 23);
			clif_friendslist_reqack(*f_sd, sd.status.name, 0);
			clif_friendslist_send(sd);
		}
	}
	return 0;
}

int clif_parse_FriendsListRemove(int fd, struct map_session_data &sd)
{	// 0x203 </o> <ID to be removed W 4B>
	int i, j;

	if( !session_isActive(fd) )
		return 0;

	uint32 id = RFIFOL(fd,6);
	struct map_session_data *f_sd = map_charid2sd(id);

	// Search friend
	for (i = 0; i < MAX_FRIENDLIST; ++i)
	{
		if(sd.status.friendlist[i].friend_id == id)
		{
			// move all chars down
			for(j = i + 1; j < MAX_FRIENDLIST; ++j)
			{
				sd.status.friendlist[j-1].friend_id = sd.status.friendlist[j].friend_id;
				memcpy(sd.status.friendlist[j-1].friend_name, sd.status.friendlist[j].friend_name, sizeof(sd.status.friendlist[j].friend_name));
			}
			// clear the last one
			sd.status.friendlist[MAX_FRIENDLIST-1].friend_id = 0;
			memset(sd.status.friendlist[MAX_FRIENDLIST-1].friend_name, 0, sizeof(sd.status.friendlist[MAX_FRIENDLIST-1].friend_name));
			clif_displaymessage(fd, "Friend removed");
			WFIFOW(fd,0) = 0x20a;
			WFIFOL(fd,2) = (f_sd) ? f_sd->block_list::id : 0;	//account id;
			WFIFOL(fd,6) = id;
			WFIFOSET(fd, packet_len_table[0x20a]);
			clif_friendslist_send(sd);
			break;
		}
	}

	if (i == 20)
		return clif_displaymessage(fd, "Name not found in list.");

	return 0;
}

/*==========================================
 * /killall
 *------------------------------------------
 */
int clif_parse_GMKillAll(int fd, struct map_session_data &sd)
{
	char message[50];

	memcpy(message,sd.status.name, 24);
	is_atcommand(fd, sd, strcat(message," : @kickall"),0);

	return 0;
}

/*==========================================
 * /pvpinfo
 *------------------------------------------
 */
int clif_parse_PVPInfo(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x210;
	//WFIFOL(fd,2) = 0;	// not sure what for yet
	//WFIFOL(fd,6) = 0;
	WFIFOL(fd,10) = sd.pvp_won;	// times won
	WFIFOL(fd,14) = sd.pvp_lost;	// times lost
	WFIFOL(fd,18) = sd.pvp_point;
	WFIFOSET(fd, packet_db[sd.packet_ver][0x210].len);

	return 0;
}

/*==========================================
 * /blacksmith
 *------------------------------------------
 */
int clif_parse_Blacksmith(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	const CFameList &fl = chrif_getfamelist(FAME_SMITH);
	size_t i;

	WFIFOW(fd,0) = 0x219;
	for (i=0; i<fl.count() && i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), fl[i].name, 24);
		WFIFOL(fd, 242 + i * 4) = fl[i].fame_points;
	}
	for ( ; i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), "None", 24);
		WFIFOL(fd, 242 + i * 4) = 0;
	}

	WFIFOSET(fd, packet_db[sd.packet_ver][0x219].len);
	return 0;
}

/*==========================================
 * /alchemist
 *------------------------------------------
 */
int clif_parse_Alchemist(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	const CFameList &fl = chrif_getfamelist(FAME_CHEM);
	size_t i;

	WFIFOW(fd,0) = 0x21a;
	for (i=0; i<fl.count() && i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), fl[i].name, 24);
		WFIFOL(fd, 242 + i * 4) = fl[i].fame_points;
	}
	for ( ; i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), "None", 24);
		WFIFOL(fd, 242 + i * 4) = 0;
	}

	WFIFOSET(fd, packet_db[sd.packet_ver][0x21a].len);
	return 0;
}

/*==========================================
 * /taekwon?
 *------------------------------------------
 */
int clif_parse_Taekwon(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	const CFameList &fl = chrif_getfamelist(FAME_TEAK);
	size_t i;

	WFIFOW(fd,0) = 0x226;
	for (i=0; i<fl.count() && i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), fl[i].name, 24);
		WFIFOL(fd, 242 + i * 4) = fl[i].fame_points;
	}
	for ( ; i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), "None", 24);
		WFIFOL(fd, 242 + i * 4) = 0;
	}
	WFIFOSET(fd, packet_db[sd.packet_ver][0x226].len);
	return 0;
}

/*==========================================
 * PK Ranking table?
 *------------------------------------------
 */
int clif_parse_RankingPk(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	const CFameList &fl = chrif_getfamelist(FAME_PK);
	size_t i;

	WFIFOW(fd,0) = 0x238;
	for (i=0; i<fl.count() && i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), fl[i].name, 24);
		WFIFOL(fd, 242 + i * 4) = fl[i].fame_points;
	}
	for ( ; i<MAX_FAMELIST; ++i)
	{
		memcpy(WFIFOP(fd, 2 + 24 * i), "None", 24);
		WFIFOL(fd, 242 + i * 4) = 0;
	}

	WFIFOSET(fd, packet_db[sd.packet_ver][0x238].len);
	return 0;
}





/*==========================================
 * メール送信→Interへ
 *------------------------------------------
 */
int clif_parse_SendMail(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(!config.mailsystem)
	{	
		clif_res_sendmail(sd, false);
		return 0;
	}

	if(RFIFOW(fd,2)<70)
		return 0;

	size_t bodylen = RFIFOW(fd,2)-69;
	char* targetname = (char*)RFIFOP(fd,4);
	char* header     = (char*)RFIFOP(fd,28);
	char* body       = (char*)RFIFOP(fd,68);

	// add termination, working on body is a bit dangerous since length is not validated
	targetname[23] = 0;
	header[39] = 0;
	if(bodylen>512) bodylen=512;
	body[bodylen-1] = 0;

	// prevent self mails
	struct map_session_data *rd = map_nick2sd(targetname);
	if(rd && rd->block_list::id==sd.block_list::id)
	{	// self mail
		clif_res_sendmail(sd, false);
		chrif_mail_cancel(sd);
	}
	else
	{
		chrif_mail_send(sd, targetname, header, body);
	}
	return 0;
}
/*==========================================
 * メールの選択受信
 *------------------------------------------
 */
int clif_parse_ReadMail(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	
	chrif_mail_read(sd, RFIFOL(fd,2));
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_MailGetAppend(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	chrif_mail_getappend(sd, RFIFOL(fd,2));
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_MailWinOpen(int fd, struct map_session_data &sd)
{
	int flag;
	unsigned short cmd	= RFIFOW(fd,0);

	if( !session_isActive(fd) )
		return 0;

	flag = RFIFOL(fd, packet_db[sd.packet_ver][cmd].pos[0]);
	chrif_mail_removeitem(sd, (flag==0)?3:flag );
	return 0;
}
/*==========================================
 * メールBOXの更新要求
 *------------------------------------------
 */
int clif_parse_RefreshMailBox(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	chrif_mail_fetch(sd, false);
	return 0;
}
/*==========================================
 * メールに添付
 *------------------------------------------
 */
int clif_parse_SendMailSetAppend(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	int idx   = RFIFOW(fd,2);
	int amount= RFIFOL(fd,4);
	if( idx >= 0 && amount >= 0 && !chrif_mail_setitem(sd, idx, amount) )
		clif_res_sendmail_setappend(sd, -1);
	// possible failure codes from jA
	// -1 general failure
	//  1 invalid amount 
	//  2 item not dropable
	return 0;
}
/*==========================================
 * メール削除
 *------------------------------------------
 */
int clif_parse_DeleteMail(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	chrif_mail_delete(sd, RFIFOL(fd,2));
	return 0;
}










/*==========================================
 * ホムンクルス
 *------------------------------------------
 */
int clif_parse_HomMenu(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	unsigned short cmd	= RFIFOW(fd,0);
	homun_data::menu(sd,RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[0]));
	return 0;
}
int clif_parse_HomWalkMaster(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	uint32 id = RFIFOL(fd,2);
	if( id != sd.status.homun_id )
		printf("clif_parse_HomWalkToXY: %lu %lu", (ulong)id, (ulong)sd.status.homun_id);

	homun_data::return_to_master(sd);
	return 0;
}
int clif_parse_HomWalkToXY(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	//## change to virtual overloads for the walk interface
	homun_data *hd = homun_data::get_homunculus(sd);
	if( hd )
	{
		const unsigned short cmd = RFIFOW(fd,0);
		uint32 id = RFIFOL(fd,2);
		if( id != hd->status.homun_id )
			printf("clif_parse_HomWalkToXY: %lu %lu", (ulong)id, (ulong)hd->status.homun_id);

		const uchar v0 = RFIFOB(fd,0+packet_db[sd.packet_ver][cmd].pos[0]);
		const uchar v1 = RFIFOB(fd,1+packet_db[sd.packet_ver][cmd].pos[0]);
		const uchar v2 = RFIFOB(fd,2+packet_db[sd.packet_ver][cmd].pos[0]);

		const int x = ( v0      <<2) | (v1>>6);	// inverse to position encoding
		const int y = ((v1&0x3f)<<4) | (v2>>4);

		hd->walktoxy(x,y);
	}
	return 0;
}
int clif_parse_HomActionRequest(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	homun_data *hd = homun_data::get_homunculus(sd);
	if(hd)
	{
		if( hd->is_dead() )
		{
			clif_clearchar_area(*hd,1);
			return 0;
		}
		if(hd->status.option&2)
			return 0;

		const unsigned short cmd = RFIFOW(fd,0);

		//nt32 homun_id = RFIFOL(fd,2);
		uint32 target_id = RFIFOL(fd,packet_db[sd.packet_ver][cmd].pos[0]);	// pos 6
		int action_type = RFIFOB(fd,packet_db[sd.packet_ver][cmd].pos[1]);	// pos 10 -> len 11
		
		// decode for jRO 2005-05-09dRagexe
		if( packet_db[sd.packet_ver][cmd].pos[0]==0 )
		{
			int packet_len = packet_db[sd.packet_ver][cmd].len;
			int t1[]={ 88, 37 }, t2[]={ 80, 4 };
			int pos = ( ( packet_len - t1[packet_len&1] ) >> 1 ) + t2[packet_len&1];
			target_id = RFIFOL(fd,pos);
		}

		hd->stop_walking(1);
		hd->stop_attack();
		
		// end decode
		switch(action_type)
		{
		case 0x00:	// once attack
		case 0x07:	// continuous attack
			{
				block_list*bl=map_id2bl(target_id);
				if(bl && !mob_gvmobcheck(sd,*bl))
					return 0;
				hd->start_attack(target_id, action_type!=0);
			}
			break;
		}
	}
	return 0;
}
int clif_parse_ChangeHomName(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	homun_data::change_name(sd,(const char*)RFIFOP(fd,packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]));
	return 0;
}

/*==========================================
 *	養子ターゲット表示
 *------------------------------------------
 */
int clif_baby_target_display(struct map_session_data &sd)
{
	if( !session_isActive(sd.fd) )
		return 0;

	unsigned char buf[2];
	WBUFW(buf,0) = 0x01f8;
	WFIFOSET(sd.fd,packet_db[sd.packet_ver][0x1f8].len);

	return 0;
}
/*==========================================
 * 養子要求
 *------------------------------------------
 */
int  clif_parse_BabyRequest(int fd, struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	uint32 char_id = RFIFOL(fd, packet_db[sd.packet_ver][RFIFOW(fd,0)].pos[0]);
	struct map_session_data* tsd = (struct map_session_data*)map_id2sd(char_id);
	if(tsd && tsd->block_list::type == BL_PC)
		clif_baby_target_display(*tsd);
	return 0;
}


/*==========================================
 * SG Feel save OK [Komurka]
 *------------------------------------------
 */
int clif_request_feel(struct map_session_data &sd, unsigned short skilllv)
{
	if( !session_isActive(sd.fd) )
		return 0;
	pc_setglobalreg(sd, "PC_SG_FEEL", skilllv);
	WFIFOW(sd.fd,0)=0x253;
	WFIFOSET(sd.fd, packet_db[sd.packet_ver][0x253].len);

	return 0;
}

int clif_parse_FeelSaveOk(int fd,struct map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	unsigned short i = pc_readglobalreg(sd,"PC_SG_FEEL");
	pc_setglobalreg(sd, "PC_SG_FEEL", 0);
	if(i<1 || i > 3)
		return 0; //Bug?

	static const char *feel_var[3] = {"PC_FEEL_SUN","PC_FEEL_MOON","PC_FEEL_STAR"};
	pc_setglobalreg(sd, feel_var[i-1], sd.block_list::m );

	WFIFOW(fd,0)=0x20e;
	mapname2buffer(WFIFOP(fd,2), maps[sd.block_list::m].mapname, 16);
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=i-1;
	WFIFOSET(fd, packet_db[sd.packet_ver][0x20e].len);
	
	clif_skill_nodamage(sd,sd,SG_FEEL,i,1);
	return 0;
}

int clif_parse_AdoptRequest(int fd, map_session_data &sd)
{
	//TODO: add somewhere the adopt code, checks for exploits, etc, etc.
	//Missing packets are the client's reply packets to the adopt request one. 
	//[Skotlex]
	if( !session_isActive(fd) )
		return 0;
	
	int account_id = RFIFOL(fd,2);
	struct map_session_data *sd2 = map_id2sd(account_id);
	if( sd2 && session_isActive(sd2->fd) && 
		sd2->block_list::id != sd.block_list::id && 
		sd2->status.party_id == sd.status.party_id)
	{	//FIXME: No checks whatsoever are in place yet!
		WFIFOW(sd2->fd,0)=0x1f9;
		WFIFOSET(sd2->fd, packet_len_table[0x1f9]);
	}
	return 0;
}
/*==========================================
 * パケットデバッグ
 *------------------------------------------
 */
int clif_parse_debug(int fd, struct map_session_data &sd)
{
	int i, cmd;

	if( !session_isActive(fd) )
		return 0;

	cmd = RFIFOW(fd,0);

	ShowMessage("packet debug 0x%4X\n",cmd);
//	ShowMessage("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i=0;i<packet_db[sd.packet_ver][cmd].len; ++i){
		if((i&15)==0)
			ShowMessage("\n%04X ",i);
		ShowMessage("%02X ",(unsigned char)RFIFOB(fd,i));
	}
	ShowMessage("\n");
	return 0;
}
/*==========================================
 * /effectとか
 *------------------------------------------
 */
int clif_parse_clientsetting(int fd, struct map_session_data &sd)
{
	// RFIFOB(fd,0)	effectを切ってるかどうか
	return 0;
}

int clif_parse_dummy(int fd, struct map_session_data &sd)
{
	int i, cmd;

	if( !session_isActive(fd) )
		return 0;

	cmd = RFIFOW(fd,0);

	ShowMessage("unimplemented packet debug 0x%4X\n",cmd);
//	ShowMessage("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i=0;i<packet_db[sd.packet_ver][cmd].len; ++i){
		if((i&15)==0)
			ShowMessage("\n%04X ",i);
		ShowMessage("%02X ",(unsigned char)RFIFOB(fd,i));
	}
	ShowMessage("\n");
	return 0;
}
/*==========================================
 * terminate function on logout ans session delete
 * 
 *------------------------------------------
 */
int clif_terminate(int fd)
{
	if( session_isRemoved(fd) )
	{	
		struct map_session_data *sd = (struct map_session_data*)session[fd]->user_session;
		if( sd!=NULL )
		{
			if( sd->state.event_disconnect )
				npc_event_doall_attached("OnDisconnect",*sd);   

			clif_clearchar(*sd, 0);
			if(sd->state.auth) 
			{	// the function doesn't send to inter-server/char-server ifit is not connected [Yor]
				map_quit(*sd);
				if(sd->status.name != NULL)
					ShowInfo("%sCharacter '"CL_WHITE"%s"CL_RESET"' logged off.\n", (sd->isGM())?"GM ":"",sd->status.name); // Player logout display [Valaris]
				else
					ShowInfo("%sCharacter with Account ID '"CL_WHITE"%d"CL_RESET"' logged off.\n", (sd->isGM())?"GM ":"", sd->block_list::id); // Player logout display [Yor]
			}
			else 
			{	// not authentified! (refused by char-server or disconnect before to be authentified)
				ShowInfo("Player not authenticated with Account ID '"CL_WHITE"%d"CL_RESET"' logged off.\n", sd->block_list::id); // Player logout display [Yor]
				sd->map_deliddb(); // account_id has been included in the DB before auth answer [Yor]
			} 
			chrif_char_offline(*sd);

			delete sd;
			session[fd]->user_session = NULL;
		} 
		else
		{
			uint32 ip = session[fd]->client_ip;
			ShowWarning("Player not identified with IP '"CL_WHITE"%d.%d.%d.%d"CL_RESET"' logged off.\n", (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
		}
	}
	return 0;
}

/*==========================================
 * クライアントからのパケット解析
 * socket.cのdo_parsepacketから呼び出される
 *------------------------------------------
 */
int clif_parse(int fd)
{
	int packet_len = 0, packet_ver;
	unsigned short cmd;
	struct map_session_data *sd;

	if( !session_isValid(fd) )
		return 0;

	sd = (struct map_session_data*)session[fd]->user_session;

	// 接続が切れてるので後始末
	// char鯖に繋がってない間は接続禁止 (!chrif_isconnect())
	if( !session_isActive(fd) )
	{ 
		// wait some time (10sec) before removing the pc after a forced logout
		// other disconnections got a different WaitClose time before comming here
		if( session_isMarked(fd) || !sd || sd->state.waitingdisconnect )
			// removing marked sessons now might be no thread 
			// even if the timer is not removed and called on an empty or reused session
			session_Remove(fd); 
		else
			session_SetWaitClose(fd, 10000);
		return 0;
	}

	while(session_isActive(fd) && RFIFOREST(fd) >= 2)
	{
		cmd = RFIFOW(fd,0);
		//ShowMessage("clif_parse: connection #%d, packet: 0x%x (with being read: %d bytes(%i)).\n", fd, cmd, RFIFOREST(fd), (unsigned short)RFIFOW(fd,2));
		// 管理用パケット処理
		if (cmd >= 30000)
		{
			switch(cmd)
			{
			case 0x7530: // Athena情報所得
				WFIFOW(fd,0) = 0x7531;
				WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
				WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
				WFIFOB(fd,4) = ATHENA_REVISION;
				WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
				WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
				WFIFOB(fd,7) = ATHENA_SERVER_MAP;
				WFIFOW(fd,8) = ATHENA_MOD_VERSION;
				WFIFOSET(fd,10);
				RFIFOSKIP(fd,2);
				break;
			case 0x7532: // 接続の切断
				session_Remove(fd);
				ShowWarning("clif_parse: session #%d, packet 0x%x received -> disconnected.\n", fd, cmd);
				return 0;
			}
		}

		if(sd)
			packet_ver = sd->packet_ver;
		else
		{
			packet_ver = clif_getPacketVer(fd);
			// check if version is accepted
			if( packet_ver < 0 || packet_ver > MAX_PACKET_VER || // reject unknown clients
				(!sd && packet_db[packet_ver][cmd].func != clif_parse_WantToConnection) || // should not happen

				// check client version
				(config.packet_ver_flag &&
				((packet_ver <=  5 && (config.packet_ver_flag & 0x00001) == 0) ||
				(packet_ver ==  6 && (config.packet_ver_flag & 0x00002) == 0) ||	
				(packet_ver ==  7 && (config.packet_ver_flag & 0x00004) == 0) ||	
				(packet_ver ==  8 && (config.packet_ver_flag & 0x00008) == 0) ||	
				(packet_ver ==  9 && (config.packet_ver_flag & 0x00010) == 0) ||
				(packet_ver == 10 && (config.packet_ver_flag & 0x00020) == 0) ||
				(packet_ver == 11 && (config.packet_ver_flag & 0x00040) == 0) ||
				(packet_ver == 12 && (config.packet_ver_flag & 0x00080) == 0) ||
				(packet_ver == 13 && (config.packet_ver_flag & 0x00100) == 0) ||
				(packet_ver == 14 && (config.packet_ver_flag & 0x00200) == 0) ||
				(packet_ver == 15 && (config.packet_ver_flag & 0x00400) == 0) ||
				(packet_ver == 16 && (config.packet_ver_flag & 0x00800) == 0) ||
				(packet_ver == 17 && (config.packet_ver_flag & 0x01000) == 0)) ) )
			{
				ShowMessage("clif_parse: session #%d, packet 0x%x ver. %i (%d bytes received) -> disconnected (unknown packetver).\n", fd, cmd, packet_ver, RFIFOREST(fd));
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = 5; // 05 = Game's EXE is not the latest version
				WFIFOSET(fd,23);
				RFIFOSKIP(fd, RFIFOREST(fd));
				session_SetWaitClose(fd, 5000);
				return 0;
			}
		}
		
		// ゲーム用以外パケットか、認証を終える前に0072以外が来たら、切断する
		if(cmd >= MAX_PACKET_DB || packet_ver>MAX_PACKET_VER || packet_db[packet_ver][cmd].len == 0)
		{	// packet is not inside these values: session is incorrect?? or auth packet is unknown
			ShowMessage("clif_parse: session #%d, packet 0x%x ver. %i (%d bytes received) -> disconnected (unknown command).\n", fd, cmd, packet_ver, RFIFOREST(fd));
			session_Remove(fd);
			return 0;
		}
		
		// パケット長を計算
		packet_len = packet_db[packet_ver][cmd].len;
		if (packet_len < 0)
		{	// 可変長パケットで長さの所までデータが来てない
			if (RFIFOREST(fd) < 4)
				return 0; 
			packet_len = RFIFOW(fd,2);
			if (packet_len < 4 || packet_len > 32768)
			{
				session_Remove(fd);
				ShowWarning("clif_parse: session #%d, packet 0x%x invalid packet_len (%d bytes received) -> disconnected.\n", fd, cmd, packet_len);
				return 0;
			}
		}
		// まだ1パケット分データが揃ってない
		if (RFIFOREST(fd) < packet_len)
			return 0;

		if (sd && sd->state.auth == 1 && sd->state.waitingdisconnect == 1)
		{	// 切断待ちの場合パケットを処理しない
		}
		else if (packet_db[packet_ver][cmd].func)
		{	// packet version 5-6-7 use same functions, but size are different
			// パケット処理

			if( sd && packet_db[packet_ver][cmd].func==clif_parse_WantToConnection )
			{
				if (config.error_log)
					ShowMessage("clif_parse_WantToConnection : invalid request?\n");
			}
			else
			{	// have a dummy session data to call with WantToConnection
				if( packet_db[packet_ver][cmd].func==clif_parse_WantToConnection )
				{	static map_session_data dummy;
					dummy.packet_ver = packet_ver;
					dummy.fd = fd;
					packet_db[packet_ver][cmd].func(fd, dummy);
					// we come out of WantToConnection and have a valid sd for the next run or NULL if not
					sd = (session[fd])?((struct map_session_data *)session[fd]->user_session):NULL;
				}
				else if(sd)
					packet_db[packet_ver][cmd].func(fd, *sd);
			}
		}
#ifdef DUMP_UNKNOWN_PACKET
		else
		{	
			dump_packet(fd, packet_len, sd);
		}
#endif
#ifdef DUMP_ALL_PACKETS
		print_packet(fd, packet_len, sd);
#endif
		RFIFOSKIP(fd, packet_len);
	}// end while
	return 0;
}



/*==========================================
 * パケットデータベース読み込み
 *------------------------------------------
 */
int packetdb_readdb(void)
{	// set defaults for packet_db ver 0..18

	///////////////////////////////////////////////////////////////////////////
	// the default packet used for connecting to the server
	packet_db[0].connect_cmd = 0x72;
	packet_db[0][0x0000] = packet_cmd(10);
	packet_db[0][0x0064] = packet_cmd(55);
	packet_db[0][0x0065] = packet_cmd(17);
	packet_db[0][0x0066] = packet_cmd(3);
	packet_db[0][0x0067] = packet_cmd(37);
	packet_db[0][0x0068] = packet_cmd(46);
	packet_db[0][0x0069] = packet_cmd(-1);
	packet_db[0][0x006a] = packet_cmd(23);
	packet_db[0][0x006b] = packet_cmd(-1);
	packet_db[0][0x006c] = packet_cmd(3);
	packet_db[0][0x006d] = packet_cmd(108);
	packet_db[0][0x006e] = packet_cmd(3);
	packet_db[0][0x006f] = packet_cmd(2);
	packet_db[0][0x0070] = packet_cmd(3);
	packet_db[0][0x0071] = packet_cmd(28);
	packet_db[0][0x0072] = packet_cmd(19,clif_parse_WantToConnection,2,6,10,14,18);
	packet_db[0][0x0073] = packet_cmd(11);
	packet_db[0][0x0074] = packet_cmd(3);
	packet_db[0][0x0075] = packet_cmd(-1);
	packet_db[0][0x0076] = packet_cmd(9);
	packet_db[0][0x0077] = packet_cmd(5);
	packet_db[0][0x0078] = packet_cmd(52);
	packet_db[0][0x0079] = packet_cmd(51);
	packet_db[0][0x007a] = packet_cmd(56);
	packet_db[0][0x007b] = packet_cmd(58);
	packet_db[0][0x007c] = packet_cmd(41);
	packet_db[0][0x007d] = packet_cmd(2,clif_parse_LoadEndAck,0);
	packet_db[0][0x007e] = packet_cmd(6,clif_parse_TickSend,2);
	packet_db[0][0x007f] = packet_cmd(6);
	packet_db[0][0x0080] = packet_cmd(7);
	packet_db[0][0x0081] = packet_cmd(3);
	packet_db[0][0x0082] = packet_cmd(2);
	packet_db[0][0x0083] = packet_cmd(2);
	packet_db[0][0x0084] = packet_cmd(2);
	packet_db[0][0x0085] = packet_cmd(5,clif_parse_WalkToXY,2);
	packet_db[0][0x0086] = packet_cmd(16);
	packet_db[0][0x0087] = packet_cmd(12);
	packet_db[0][0x0088] = packet_cmd(10);
	packet_db[0][0x0089] = packet_cmd(7,clif_parse_ActionRequest,2,6);
	packet_db[0][0x008a] = packet_cmd(29);
	packet_db[0][0x008b] = packet_cmd(2);
	packet_db[0][0x008c] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[0][0x008d] = packet_cmd(-1);
	packet_db[0][0x008e] = packet_cmd(-1);
	packet_db[0][0x008f] = packet_cmd(0);
	packet_db[0][0x0090] = packet_cmd(7,clif_parse_NpcClicked,2);
	packet_db[0][0x0091] = packet_cmd(22);
	packet_db[0][0x0092] = packet_cmd(28);
	packet_db[0][0x0093] = packet_cmd(2);
	packet_db[0][0x0094] = packet_cmd(6,clif_parse_GetCharNameRequest,2);
	packet_db[0][0x0095] = packet_cmd(30);
	packet_db[0][0x0096] = packet_cmd(-1,clif_parse_Wis,2,4,28);
	packet_db[0][0x0097] = packet_cmd(-1);
	packet_db[0][0x0098] = packet_cmd(3);
	packet_db[0][0x0099] = packet_cmd(-1,clif_parse_GMmessage,2,4);
	packet_db[0][0x009a] = packet_cmd(-1);
	packet_db[0][0x009b] = packet_cmd(5,clif_parse_ChangeDir,2,4);
	packet_db[0][0x009c] = packet_cmd(9);
	packet_db[0][0x009d] = packet_cmd(17);
	packet_db[0][0x009e] = packet_cmd(17);
	packet_db[0][0x009f] = packet_cmd(6,clif_parse_TakeItem,2);
	packet_db[0][0x00a0] = packet_cmd(23);
	packet_db[0][0x00a1] = packet_cmd(6);
	packet_db[0][0x00a2] = packet_cmd(6,clif_parse_DropItem,2,4);
	packet_db[0][0x00a3] = packet_cmd(-1);
	packet_db[0][0x00a4] = packet_cmd(-1);
	packet_db[0][0x00a5] = packet_cmd(-1);
	packet_db[0][0x00a6] = packet_cmd(-1);
	packet_db[0][0x00a7] = packet_cmd(8,clif_parse_UseItem,2,4);
	packet_db[0][0x00a8] = packet_cmd(7);
	packet_db[0][0x00a9] = packet_cmd(6,clif_parse_EquipItem,2,4);
	packet_db[0][0x00aa] = packet_cmd(7);
	packet_db[0][0x00ab] = packet_cmd(4,clif_parse_UnequipItem,2);
	packet_db[0][0x00ac] = packet_cmd(7);
	packet_db[0][0x00ad] = packet_cmd(0);
	packet_db[0][0x00ae] = packet_cmd(-1);
	packet_db[0][0x00af] = packet_cmd(6);
	packet_db[0][0x00b0] = packet_cmd(8);
	packet_db[0][0x00b1] = packet_cmd(8);
	packet_db[0][0x00b2] = packet_cmd(3,clif_parse_Restart,2);
	packet_db[0][0x00b3] = packet_cmd(3);
	packet_db[0][0x00b4] = packet_cmd(-1);
	packet_db[0][0x00b5] = packet_cmd(6);
	packet_db[0][0x00b6] = packet_cmd(6);
	packet_db[0][0x00b7] = packet_cmd(-1);
	packet_db[0][0x00b8] = packet_cmd(7,clif_parse_NpcSelectMenu,2,6);
	packet_db[0][0x00b9] = packet_cmd(6,clif_parse_NpcNextClicked,2);
	packet_db[0][0x00ba] = packet_cmd(2);
	packet_db[0][0x00bb] = packet_cmd(5,clif_parse_StatusUp,2,4);
	packet_db[0][0x00bc] = packet_cmd(6);
	packet_db[0][0x00bd] = packet_cmd(44);
	packet_db[0][0x00be] = packet_cmd(5);
	packet_db[0][0x00bf] = packet_cmd(3,clif_parse_Emotion,2);
	packet_db[0][0x00c0] = packet_cmd(7);
	packet_db[0][0x00c1] = packet_cmd(2,clif_parse_HowManyConnections,0);
	packet_db[0][0x00c2] = packet_cmd(6);
	packet_db[0][0x00c3] = packet_cmd(8);
	packet_db[0][0x00c4] = packet_cmd(6);
	packet_db[0][0x00c5] = packet_cmd(7,clif_parse_NpcBuySellSelected,2,6);
	packet_db[0][0x00c6] = packet_cmd(-1);
	packet_db[0][0x00c7] = packet_cmd(-1);
	packet_db[0][0x00c8] = packet_cmd(-1,clif_parse_NpcBuyListSend,2,4);
	packet_db[0][0x00c9] = packet_cmd(-1,clif_parse_NpcSellListSend,2,4);
	packet_db[0][0x00ca] = packet_cmd(3);
	packet_db[0][0x00cb] = packet_cmd(3);
	packet_db[0][0x00cc] = packet_cmd(6,clif_parse_GMKick,2);
	packet_db[0][0x00cd] = packet_cmd(3);
	packet_db[0][0x00ce] = packet_cmd(2,clif_parse_GMKillAll,0);
	packet_db[0][0x00cf] = packet_cmd(27,clif_parse_PMIgnore,2,26);
	packet_db[0][0x00d0] = packet_cmd(3,clif_parse_PMIgnoreAll,2);
	packet_db[0][0x00d1] = packet_cmd(4);
	packet_db[0][0x00d2] = packet_cmd(4);
	packet_db[0][0x00d3] = packet_cmd(2,clif_parse_PMIgnoreList,0);
	packet_db[0][0x00d4] = packet_cmd(-1);
	packet_db[0][0x00d5] = packet_cmd(-1,clif_parse_CreateChatRoom,2,4,6,7,15);
	packet_db[0][0x00d6] = packet_cmd(3);
	packet_db[0][0x00d7] = packet_cmd(-1);
	packet_db[0][0x00d8] = packet_cmd(6);
	packet_db[0][0x00d9] = packet_cmd(14,clif_parse_ChatAddMember,2,6);
	packet_db[0][0x00da] = packet_cmd(3);
	packet_db[0][0x00db] = packet_cmd(-1);
	packet_db[0][0x00dc] = packet_cmd(28);
	packet_db[0][0x00dd] = packet_cmd(29);
	packet_db[0][0x00de] = packet_cmd(-1,clif_parse_ChatRoomStatusChange,2,4,6,7,15);
	packet_db[0][0x00df] = packet_cmd(-1);
	packet_db[0][0x00e0] = packet_cmd(30,clif_parse_ChangeChatOwner,2,6);
	packet_db[0][0x00e1] = packet_cmd(30);
	packet_db[0][0x00e2] = packet_cmd(26,clif_parse_KickFromChat,2);
	packet_db[0][0x00e3] = packet_cmd(2,clif_parse_ChatLeave,0);
	packet_db[0][0x00e4] = packet_cmd(6,clif_parse_TradeRequest,2);
	packet_db[0][0x00e5] = packet_cmd(26);
	packet_db[0][0x00e6] = packet_cmd(3,clif_parse_TradeAck,2);
	packet_db[0][0x00e7] = packet_cmd(3);
	packet_db[0][0x00e8] = packet_cmd(8,clif_parse_TradeAddItem,2,4);
	packet_db[0][0x00e9] = packet_cmd(19);
	packet_db[0][0x00ea] = packet_cmd(5);
	packet_db[0][0x00eb] = packet_cmd(2,clif_parse_TradeOk,0);
	packet_db[0][0x00ec] = packet_cmd(3);
	packet_db[0][0x00ed] = packet_cmd(2,clif_parse_TradeCancel,0);
	packet_db[0][0x00ee] = packet_cmd(2);
	packet_db[0][0x00ef] = packet_cmd(2,clif_parse_TradeCommit,0);
	packet_db[0][0x00f0] = packet_cmd(3);
	packet_db[0][0x00f1] = packet_cmd(2);
	packet_db[0][0x00f2] = packet_cmd(6);
	packet_db[0][0x00f3] = packet_cmd(8,clif_parse_MoveToKafra,2,4);
	packet_db[0][0x00f4] = packet_cmd(21);
	packet_db[0][0x00f5] = packet_cmd(8,clif_parse_MoveFromKafra,2,4);
	packet_db[0][0x00f6] = packet_cmd(8);
	packet_db[0][0x00f7] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[0][0x00f8] = packet_cmd(2);
	packet_db[0][0x00f9] = packet_cmd(26,clif_parse_CreateParty,2);
	packet_db[0][0x00fa] = packet_cmd(3);
	packet_db[0][0x00fb] = packet_cmd(-1);
	packet_db[0][0x00fc] = packet_cmd(6,clif_parse_PartyInvite,2);
	packet_db[0][0x00fd] = packet_cmd(27);
	packet_db[0][0x00fe] = packet_cmd(30);
	packet_db[0][0x00ff] = packet_cmd(10,clif_parse_ReplyPartyInvite,2,6);
	packet_db[0][0x0100] = packet_cmd(2,clif_parse_LeaveParty,0);
	packet_db[0][0x0101] = packet_cmd(6);
	packet_db[0][0x0102] = packet_cmd(6,clif_parse_PartyChangeOption,2,4);
	packet_db[0][0x0103] = packet_cmd(30,clif_parse_RemovePartyMember,2,6);
	packet_db[0][0x0104] = packet_cmd(79);
	packet_db[0][0x0105] = packet_cmd(31);
	packet_db[0][0x0106] = packet_cmd(10);
	packet_db[0][0x0107] = packet_cmd(10);
	packet_db[0][0x0108] = packet_cmd(-1,clif_parse_PartyMessage,2,4);
	packet_db[0][0x0109] = packet_cmd(-1);
	packet_db[0][0x010a] = packet_cmd(4);
	packet_db[0][0x010b] = packet_cmd(6);
	packet_db[0][0x010c] = packet_cmd(6);
	packet_db[0][0x010d] = packet_cmd(2);
	packet_db[0][0x010e] = packet_cmd(11);
	packet_db[0][0x010f] = packet_cmd(-1);
	packet_db[0][0x0110] = packet_cmd(10);
	packet_db[0][0x0111] = packet_cmd(39);
	packet_db[0][0x0112] = packet_cmd(4,clif_parse_SkillUp,2);
	packet_db[0][0x0113] = packet_cmd(10,clif_parse_UseSkillToId,2,4,6);
	packet_db[0][0x0114] = packet_cmd(31);
	packet_db[0][0x0115] = packet_cmd(35);
	packet_db[0][0x0116] = packet_cmd(10,clif_parse_UseSkillToPos,2,4,6,8);
	packet_db[0][0x0117] = packet_cmd(18);
	packet_db[0][0x0118] = packet_cmd(2,clif_parse_StopAttack,0);
	packet_db[0][0x0119] = packet_cmd(13);
	packet_db[0][0x011a] = packet_cmd(15);
	packet_db[0][0x011b] = packet_cmd(20,clif_parse_UseSkillMap,2,4);
	packet_db[0][0x011c] = packet_cmd(68);
	packet_db[0][0x011d] = packet_cmd(2,clif_parse_RequestMemo,0);
	packet_db[0][0x011e] = packet_cmd(3);
	packet_db[0][0x011f] = packet_cmd(16);
	packet_db[0][0x0120] = packet_cmd(6);
	packet_db[0][0x0121] = packet_cmd(14);
	packet_db[0][0x0122] = packet_cmd(-1);
	packet_db[0][0x0123] = packet_cmd(-1);
	packet_db[0][0x0124] = packet_cmd(21);
	packet_db[0][0x0125] = packet_cmd(8);
	packet_db[0][0x0126] = packet_cmd(8,clif_parse_PutItemToCart,2,4);
	packet_db[0][0x0127] = packet_cmd(8,clif_parse_GetItemFromCart,2,4);
	packet_db[0][0x0128] = packet_cmd(8,clif_parse_MoveFromKafraToCart,2,4);
	packet_db[0][0x0129] = packet_cmd(8,clif_parse_MoveToKafraFromCart,2,4);
	packet_db[0][0x012a] = packet_cmd(2,clif_parse_RemoveOption,0);
	packet_db[0][0x012b] = packet_cmd(2);
	packet_db[0][0x012c] = packet_cmd(3);
	packet_db[0][0x012d] = packet_cmd(4);
	packet_db[0][0x012e] = packet_cmd(2,clif_parse_CloseVending,0);
	packet_db[0][0x012f] = packet_cmd(-1);
	packet_db[0][0x0130] = packet_cmd(6,clif_parse_VendingListReq,2);
	packet_db[0][0x0131] = packet_cmd(86);
	packet_db[0][0x0132] = packet_cmd(6);
	packet_db[0][0x0133] = packet_cmd(-1);
	packet_db[0][0x0134] = packet_cmd(-1,clif_parse_PurchaseReq,2,4,8);
	packet_db[0][0x0135] = packet_cmd(7);
	packet_db[0][0x0136] = packet_cmd(-1);
	packet_db[0][0x0137] = packet_cmd(6);
	packet_db[0][0x0138] = packet_cmd(3);
	packet_db[0][0x0139] = packet_cmd(16);
	packet_db[0][0x013a] = packet_cmd(4);
	packet_db[0][0x013b] = packet_cmd(4);
	packet_db[0][0x013c] = packet_cmd(4);
	packet_db[0][0x013d] = packet_cmd(6);
	packet_db[0][0x013e] = packet_cmd(24);
	packet_db[0][0x013f] = packet_cmd(26,clif_parse_GM_Monster_Item,2);
	packet_db[0][0x0140] = packet_cmd(22,clif_parse_MapMove,2,18,20);
	packet_db[0][0x0141] = packet_cmd(14);
	packet_db[0][0x0142] = packet_cmd(6);
	packet_db[0][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[0][0x0144] = packet_cmd(23);
	packet_db[0][0x0145] = packet_cmd(19);
	packet_db[0][0x0146] = packet_cmd(6,clif_parse_NpcCloseClicked,2);
	packet_db[0][0x0147] = packet_cmd(39);
	packet_db[0][0x0148] = packet_cmd(8);
	packet_db[0][0x0149] = packet_cmd(9,clif_parse_GMReqNoChat,2,6,7);
	packet_db[0][0x014a] = packet_cmd(6);
	packet_db[0][0x014b] = packet_cmd(27);
	packet_db[0][0x014c] = packet_cmd(-1);
	packet_db[0][0x014d] = packet_cmd(2,clif_parse_GuildCheckMaster,0);
	packet_db[0][0x014e] = packet_cmd(6);
	packet_db[0][0x014f] = packet_cmd(6,clif_parse_GuildRequestInfo,2);
	packet_db[0][0x0150] = packet_cmd(110);
	packet_db[0][0x0151] = packet_cmd(6,clif_parse_GuildRequestEmblem,2);
	packet_db[0][0x0152] = packet_cmd(-1);
	packet_db[0][0x0153] = packet_cmd(-1,clif_parse_GuildChangeEmblem,2,4);
	packet_db[0][0x0154] = packet_cmd(-1);
	packet_db[0][0x0155] = packet_cmd(-1,clif_parse_GuildChangeMemberPosition,2);
	packet_db[0][0x0156] = packet_cmd(-1);
	packet_db[0][0x0157] = packet_cmd(6);
	packet_db[0][0x0158] = packet_cmd(-1);
	packet_db[0][0x0159] = packet_cmd(54,clif_parse_GuildLeave,2,6,10,14);
	packet_db[0][0x015a] = packet_cmd(66);
	packet_db[0][0x015b] = packet_cmd(54,clif_parse_GuildExplusion,2,6,10,14);
	packet_db[0][0x015c] = packet_cmd(90);
	packet_db[0][0x015d] = packet_cmd(42,clif_parse_GuildBreak,2);
	packet_db[0][0x015e] = packet_cmd(6);
	packet_db[0][0x015f] = packet_cmd(42);
	packet_db[0][0x0160] = packet_cmd(-1);
	packet_db[0][0x0161] = packet_cmd(-1,clif_parse_GuildChangePositionInfo,2);
	packet_db[0][0x0162] = packet_cmd(-1);
	packet_db[0][0x0163] = packet_cmd(-1);
	packet_db[0][0x0164] = packet_cmd(-1);
	packet_db[0][0x0165] = packet_cmd(30,clif_parse_CreateGuild,6);
	packet_db[0][0x0166] = packet_cmd(-1);
	packet_db[0][0x0167] = packet_cmd(3);
	packet_db[0][0x0168] = packet_cmd(14,clif_parse_GuildInvite,2);
	packet_db[0][0x0169] = packet_cmd(3);
	packet_db[0][0x016a] = packet_cmd(30);
	packet_db[0][0x016b] = packet_cmd(10,clif_parse_GuildReplyInvite,2,6);
	packet_db[0][0x016c] = packet_cmd(43);
	packet_db[0][0x016d] = packet_cmd(14);
	packet_db[0][0x016e] = packet_cmd(186,clif_parse_GuildChangeNotice,2,6,66);
	packet_db[0][0x016f] = packet_cmd(182);
	packet_db[0][0x0170] = packet_cmd(14,clif_parse_GuildRequestAlliance,2);
	packet_db[0][0x0171] = packet_cmd(30);
	packet_db[0][0x0172] = packet_cmd(10,clif_parse_GuildReplyAlliance,2,6);
	packet_db[0][0x0173] = packet_cmd(3);
	packet_db[0][0x0174] = packet_cmd(-1);
	packet_db[0][0x0175] = packet_cmd(6);
	packet_db[0][0x0176] = packet_cmd(106);
	packet_db[0][0x0177] = packet_cmd(-1);
	packet_db[0][0x0178] = packet_cmd(4,clif_parse_ItemIdentify,2);
	packet_db[0][0x0179] = packet_cmd(5);
	packet_db[0][0x017a] = packet_cmd(4,clif_parse_UseCard,2);
	packet_db[0][0x017b] = packet_cmd(-1);
	packet_db[0][0x017c] = packet_cmd(6,clif_parse_InsertCard,2,4);
	packet_db[0][0x017d] = packet_cmd(7);
	packet_db[0][0x017e] = packet_cmd(-1,clif_parse_GuildMessage,2,4);
	packet_db[0][0x017f] = packet_cmd(-1);
	packet_db[0][0x0180] = packet_cmd(6,clif_parse_GuildOpposition,2);
	packet_db[0][0x0181] = packet_cmd(3);
	packet_db[0][0x0182] = packet_cmd(106);
	packet_db[0][0x0183] = packet_cmd(10,clif_parse_GuildDelAlliance,2,6);
	packet_db[0][0x0184] = packet_cmd(10);
	packet_db[0][0x0185] = packet_cmd(34);
	packet_db[0][0x0186] = packet_cmd(0);
	packet_db[0][0x0187] = packet_cmd(6);
	packet_db[0][0x0188] = packet_cmd(8);
	packet_db[0][0x0189] = packet_cmd(4);
	packet_db[0][0x018a] = packet_cmd(4,clif_parse_QuitGame,0);
	packet_db[0][0x018b] = packet_cmd(4);
	packet_db[0][0x018c] = packet_cmd(29);
	packet_db[0][0x018d] = packet_cmd(-1);
	packet_db[0][0x018e] = packet_cmd(10,clif_parse_ProduceMix,2,4,6,8);
	packet_db[0][0x018f] = packet_cmd(6);
	packet_db[0][0x0190] = packet_cmd(90,clif_parse_UseSkillToPos,2,4,6,8,10);
	packet_db[0][0x0191] = packet_cmd(86);
	packet_db[0][0x0192] = packet_cmd(24);
	packet_db[0][0x0193] = packet_cmd(6,clif_parse_SolveCharName,2);
	packet_db[0][0x0194] = packet_cmd(30);
	packet_db[0][0x0195] = packet_cmd(102);
	packet_db[0][0x0196] = packet_cmd(8);
	packet_db[0][0x0197] = packet_cmd(4,clif_parse_ResetChar,2);
	packet_db[0][0x0198] = packet_cmd(8);
	packet_db[0][0x0199] = packet_cmd(4);
	packet_db[0][0x019a] = packet_cmd(14);
	packet_db[0][0x019b] = packet_cmd(10);
	packet_db[0][0x019c] = packet_cmd(-1,clif_parse_LGMmessage,2,4);
	packet_db[0][0x019d] = packet_cmd(6,clif_parse_GMHide,0);
	packet_db[0][0x019e] = packet_cmd(2);
	packet_db[0][0x019f] = packet_cmd(6,clif_parse_CatchPet,2);
	packet_db[0][0x01a0] = packet_cmd(3);
	packet_db[0][0x01a1] = packet_cmd(3,clif_parse_PetMenu,2);
	packet_db[0][0x01a2] = packet_cmd(35);
	packet_db[0][0x01a3] = packet_cmd(5);
	packet_db[0][0x01a4] = packet_cmd(11);
	packet_db[0][0x01a5] = packet_cmd(26,clif_parse_ChangePetName,2);
	packet_db[0][0x01a6] = packet_cmd(-1);
	packet_db[0][0x01a7] = packet_cmd(4,clif_parse_SelectEgg,2);
	packet_db[0][0x01a8] = packet_cmd(4);
	packet_db[0][0x01a9] = packet_cmd(6,clif_parse_SendEmotion,2);
	packet_db[0][0x01aa] = packet_cmd(10);
	packet_db[0][0x01ab] = packet_cmd(12);
	packet_db[0][0x01ac] = packet_cmd(6);
	packet_db[0][0x01ad] = packet_cmd(-1);
	packet_db[0][0x01ae] = packet_cmd(4,clif_parse_SelectArrow,2);
	packet_db[0][0x01af] = packet_cmd(4,clif_parse_ChangeCart,2);
	packet_db[0][0x01b0] = packet_cmd(11);
	packet_db[0][0x01b1] = packet_cmd(7);
	packet_db[0][0x01b2] = packet_cmd(-1,clif_parse_OpenVending,2,4,84,85);
	packet_db[0][0x01b3] = packet_cmd(67);
	packet_db[0][0x01b4] = packet_cmd(12);
	packet_db[0][0x01b5] = packet_cmd(18);
	packet_db[0][0x01b6] = packet_cmd(114);
	packet_db[0][0x01b7] = packet_cmd(6);
	packet_db[0][0x01b8] = packet_cmd(3);
	packet_db[0][0x01b9] = packet_cmd(6);
	packet_db[0][0x01ba] = packet_cmd(26);
	packet_db[0][0x01bb] = packet_cmd(26,clif_parse_Shift,2);
	packet_db[0][0x01bc] = packet_cmd(26);
	packet_db[0][0x01bd] = packet_cmd(26,clif_parse_Recall,2);
	packet_db[0][0x01be] = packet_cmd(2);
	packet_db[0][0x01bf] = packet_cmd(3);
	packet_db[0][0x01c0] = packet_cmd(2);
	packet_db[0][0x01c1] = packet_cmd(14);
	packet_db[0][0x01c2] = packet_cmd(10);
	packet_db[0][0x01c3] = packet_cmd(-1);
	packet_db[0][0x01c4] = packet_cmd(22);
	packet_db[0][0x01c5] = packet_cmd(22);
	packet_db[0][0x01c6] = packet_cmd(4);
	packet_db[0][0x01c7] = packet_cmd(2);
	packet_db[0][0x01c8] = packet_cmd(13);
	packet_db[0][0x01c9] = packet_cmd(97);
	packet_db[0][0x01ca] = packet_cmd(3);
	packet_db[0][0x01cb] = packet_cmd(9);
	packet_db[0][0x01cc] = packet_cmd(9);
	packet_db[0][0x01cd] = packet_cmd(30);
	packet_db[0][0x01ce] = packet_cmd(6,clif_parse_AutoSpell,2);
	packet_db[0][0x01cf] = packet_cmd(28);
	packet_db[0][0x01d0] = packet_cmd(8);
	packet_db[0][0x01d1] = packet_cmd(14);
	packet_db[0][0x01d2] = packet_cmd(10);
	packet_db[0][0x01d3] = packet_cmd(35);
	packet_db[0][0x01d4] = packet_cmd(6);
	packet_db[0][0x01d5] = packet_cmd(-1,clif_parse_NpcStringInput,2,4,8);
	packet_db[0][0x01d6] = packet_cmd(4);
	packet_db[0][0x01d7] = packet_cmd(11);
	packet_db[0][0x01d8] = packet_cmd(54);
	packet_db[0][0x01d9] = packet_cmd(53);
	packet_db[0][0x01da] = packet_cmd(60);
	packet_db[0][0x01db] = packet_cmd(2);
	packet_db[0][0x01dc] = packet_cmd(-1);
	packet_db[0][0x01dd] = packet_cmd(47);
	packet_db[0][0x01de] = packet_cmd(33);
	packet_db[0][0x01df] = packet_cmd(6,clif_parse_GMReqNoChatCount,2);
	packet_db[0][0x01e0] = packet_cmd(30);
	packet_db[0][0x01e1] = packet_cmd(8);
	packet_db[0][0x01e2] = packet_cmd(34);
	packet_db[0][0x01e3] = packet_cmd(14);
	packet_db[0][0x01e4] = packet_cmd(2);
	packet_db[0][0x01e5] = packet_cmd(6);
	packet_db[0][0x01e6] = packet_cmd(26);
	packet_db[0][0x01e7] = packet_cmd(2,clif_parse_NoviceDoriDori,0);
	packet_db[0][0x01e8] = packet_cmd(28,clif_parse_CreateParty2,2);
	packet_db[0][0x01e9] = packet_cmd(81);
	packet_db[0][0x01ea] = packet_cmd(6);
	packet_db[0][0x01eb] = packet_cmd(10);
	packet_db[0][0x01ec] = packet_cmd(26);
	packet_db[0][0x01ed] = packet_cmd(2,clif_parse_NoviceExplosionSpirits,0);
	packet_db[0][0x01ee] = packet_cmd(-1);
	packet_db[0][0x01ef] = packet_cmd(-1);
	packet_db[0][0x01f0] = packet_cmd(-1);
	packet_db[0][0x01f1] = packet_cmd(-1);
	packet_db[0][0x01f2] = packet_cmd(20);
	packet_db[0][0x01f3] = packet_cmd(10);
	packet_db[0][0x01f4] = packet_cmd(32);
	packet_db[0][0x01f5] = packet_cmd(9);
	packet_db[0][0x01f6] = packet_cmd(34);
	packet_db[0][0x01f7] = packet_cmd(14);
	packet_db[0][0x01f8] = packet_cmd(2);
	packet_db[0][0x01f9] = packet_cmd(6,clif_parse_ReqAdopt,5);
	packet_db[0][0x01fa] = packet_cmd(48);
	packet_db[0][0x01fb] = packet_cmd(56);
	packet_db[0][0x01fc] = packet_cmd(-1);
	packet_db[0][0x01fd] = packet_cmd(4,clif_parse_RepairItem,2);
	packet_db[0][0x01fe] = packet_cmd(5);
	packet_db[0][0x01ff] = packet_cmd(10);
	packet_db[0][0x0200] = packet_cmd(26);
	packet_db[0][0x0201] = packet_cmd(-1);
	packet_db[0][0x0202] = packet_cmd(26,clif_parse_FriendsListAdd,2);
	packet_db[0][0x0203] = packet_cmd(10,clif_parse_FriendsListRemove,2,6);
	packet_db[0][0x0204] = packet_cmd(18);
	packet_db[0][0x0205] = packet_cmd(26);
	packet_db[0][0x0206] = packet_cmd(11);
	packet_db[0][0x0207] = packet_cmd(34);
	packet_db[0][0x0208] = packet_cmd(14,clif_parse_FriendsListReply,2,6,10);
	packet_db[0][0x0209] = packet_cmd(36);
	packet_db[0][0x020a] = packet_cmd(10);
	packet_db[0][0x020b] = packet_cmd(0);
	packet_db[0][0x020c] = packet_cmd(0);
	packet_db[0][0x020d] = packet_cmd(-1);
	packet_db[0][0x020e] = packet_cmd(24);
	packet_db[0][0x020f] = packet_cmd(10);
	packet_db[0][0x0210] = packet_cmd(22);
	packet_db[0][0x0211] = packet_cmd(0);
	packet_db[0][0x0212] = packet_cmd(26);
	packet_db[0][0x0213] = packet_cmd(26);
	packet_db[0][0x0214] = packet_cmd(42);
	packet_db[0][0x0215] = packet_cmd(-1);
	packet_db[0][0x0216] = packet_cmd(-1);
	packet_db[0][0x0217] = packet_cmd(2);
	packet_db[0][0x0218] = packet_cmd(2);
	packet_db[0][0x0219] = packet_cmd(282);
	packet_db[0][0x021a] = packet_cmd(282);
	packet_db[0][0x021b] = packet_cmd(10);
	packet_db[0][0x021c] = packet_cmd(10);
	packet_db[0][0x021d] = packet_cmd(-1);
	packet_db[0][0x021e] = packet_cmd(-1);
	packet_db[0][0x021f] = packet_cmd(66);
	packet_db[0][0x0220] = packet_cmd(10);
	packet_db[0][0x0221] = packet_cmd(-1);
	packet_db[0][0x0222] = packet_cmd(-1);
	packet_db[0][0x0223] = packet_cmd(8);
	packet_db[0][0x0224] = packet_cmd(10);
	packet_db[0][0x0225] = packet_cmd(2);
	packet_db[0][0x0226] = packet_cmd(282);
	packet_db[0][0x0227] = packet_cmd(18);
	packet_db[0][0x0228] = packet_cmd(18);
	packet_db[0][0x0229] = packet_cmd(15);
	packet_db[0][0x022a] = packet_cmd(58);
	packet_db[0][0x022b] = packet_cmd(57);
	packet_db[0][0x022c] = packet_cmd(64);
	packet_db[0][0x022d] = packet_cmd(5);
	packet_db[0][0x022e] = packet_cmd(69);
	packet_db[0][0x022f] = packet_cmd(0);
	packet_db[0][0x0230] = packet_cmd(12);
	packet_db[0][0x0231] = packet_cmd(26);
	packet_db[0][0x0232] = packet_cmd(9);
	packet_db[0][0x0233] = packet_cmd(11);
	packet_db[0][0x0234] = packet_cmd(-1);
	packet_db[0][0x0235] = packet_cmd(-1);
	packet_db[0][0x0236] = packet_cmd(10);
	packet_db[0][0x0237] = packet_cmd(2, clif_parse_RankingPk, 0);	// shows top 10 slayers in the server
	packet_db[0][0x0238] = packet_cmd(282);
	packet_db[0][0x0239] = packet_cmd(11);
	packet_db[0][0x023d] = packet_cmd(-1);
	packet_db[0][0x023e] = packet_cmd(4);
	packet_db[0][0x023f] = packet_cmd(2, clif_parse_RefreshMailBox, 0);
	packet_db[0][0x0240] = packet_cmd(-1);
	packet_db[0][0x0241] = packet_cmd(6, clif_parse_ReadMail, 2);
	packet_db[0][0x0242] = packet_cmd(-1);
	packet_db[0][0x0243] = packet_cmd(6, clif_parse_DeleteMail, 2);
	packet_db[0][0x0244] = packet_cmd(6, clif_parse_MailGetAppend, 2);
	packet_db[0][0x0245] = packet_cmd(3);
	packet_db[0][0x0246] = packet_cmd(4, clif_parse_MailWinOpen, 2);
	packet_db[0][0x0247] = packet_cmd(8, clif_parse_SendMailSetAppend, 2, 4);
	packet_db[0][0x0248] = packet_cmd(-1,clif_parse_SendMail,2,4,28,69);
	packet_db[0][0x0249] = packet_cmd(3);
	packet_db[0][0x024a] = packet_cmd(70);
	packet_db[0][0x024b] = packet_cmd(4);
	packet_db[0][0x024c] = packet_cmd(8);
	packet_db[0][0x024d] = packet_cmd(12);
	packet_db[0][0x024e] = packet_cmd(4);
	packet_db[0][0x024f] = packet_cmd(10);
	packet_db[0][0x0250] = packet_cmd(3);
	packet_db[0][0x0251] = packet_cmd(32);
	packet_db[0][0x0252] = packet_cmd(-1);
	packet_db[0][0x0253] = packet_cmd(3);
	packet_db[0][0x0254] = packet_cmd(3);//feelsaveok,0
	packet_db[0][0x0255] = packet_cmd(5);
	packet_db[0][0x0256] = packet_cmd(5);
	packet_db[0][0x0257] = packet_cmd(8);
	packet_db[0][0x0258] = packet_cmd(2);
	packet_db[0][0x0259] = packet_cmd(3);
	packet_db[0][0x025a] = packet_cmd(-1);
	packet_db[0][0x025b] = packet_cmd(-1);
	packet_db[0][0x025c] = packet_cmd(4);
	packet_db[0][0x025d] = packet_cmd(-1);
	packet_db[0][0x025e] = packet_cmd(4);

	//0x025f max
	///////////////////////////////////////////////////////////////////////////
	// init packet version 5 and lower
	packet_db[1] = packet_db[0];
	packet_db[1][0x0196] = packet_cmd(9);
	///////////////////////////////////////////////////////////////////////////
	packet_db[2] = packet_db[1];
	packet_db[0][0x0078] = packet_cmd(54);
	packet_db[0][0x0079] = packet_cmd(53);
	packet_db[0][0x007a] = packet_cmd(58);
	packet_db[0][0x007b] = packet_cmd(60);
	///////////////////////////////////////////////////////////////////////////
	packet_db[3] = packet_db[2];
	///////////////////////////////////////////////////////////////////////////
	packet_db[4] = packet_db[3];
	///////////////////////////////////////////////////////////////////////////
	packet_db[5] = packet_db[4];
	///////////////////////////////////////////////////////////////////////////
	// packet version 6 (2004-07-07)
	packet_db[6] = packet_db[5];
	packet_db[6].connect_cmd = 0x72;
	packet_db[6][0x0072] = packet_cmd(22,clif_parse_WantToConnection,5,9,13,17,21);
	packet_db[6][0x0085] = packet_cmd(8,clif_parse_WalkToXY,5);
	packet_db[6][0x00a7] = packet_cmd(13,clif_parse_UseItem,5,9);
	packet_db[6][0x0113] = packet_cmd(15,clif_parse_UseSkillToId,4,9,11);
	packet_db[6][0x0116] = packet_cmd(15,clif_parse_UseSkillToPos,4,9,11,13);
	packet_db[6][0x0190] = packet_cmd(95,clif_parse_UseSkillToPos,4,9,11,13,15);
	///////////////////////////////////////////////////////////////////////////
	// packet version 7 (2004-07-13)
	packet_db[7] = packet_db[6];
	packet_db[7].connect_cmd = 0x72;
	packet_db[7][0x0072] = packet_cmd(39,clif_parse_WantToConnection,12,22,30,34,38);
	packet_db[7][0x0085] = packet_cmd(9,clif_parse_WalkToXY,6);
	packet_db[7][0x009b] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[7][0x009f] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[7][0x00a7] = packet_cmd(17,clif_parse_UseItem,6,13);
	packet_db[7][0x0113] = packet_cmd(19,clif_parse_UseSkillToId,7,9,15);
	packet_db[7][0x0116] = packet_cmd(19,clif_parse_UseSkillToPos,7,9,15,17);
	packet_db[7][0x0190] = packet_cmd(99,clif_parse_UseSkillToPos,7,9,15,17,19);
	///////////////////////////////////////////////////////////////////////////
	// packet version 8 (2004-07-26)
	packet_db[8] = packet_db[7];
	packet_db[8].connect_cmd = 0x7e;
	packet_db[8][0x0072] = packet_cmd(14,clif_parse_DropItem,5,12);
	packet_db[8][0x007e] = packet_cmd(33,clif_parse_WantToConnection,12,18,24,28,32);
	packet_db[8][0x0085] = packet_cmd(20,clif_parse_UseSkillToId,7,12,16);
	packet_db[8][0x0089] = packet_cmd(15,clif_parse_GetCharNameRequest,11);
	packet_db[8][0x008c] = packet_cmd(23,clif_parse_UseSkillToPos,3,6,17,21);
	packet_db[8][0x0094] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[8][0x009b] = packet_cmd(6,clif_parse_WalkToXY,3);
	packet_db[8][0x009f] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[8][0x00a2] = packet_cmd(103,clif_parse_UseSkillToPos,3,6,17,21,23);
	packet_db[8][0x00a7] = packet_cmd(12,clif_parse_SolveCharName,8);
	packet_db[8][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[8][0x00f5] = packet_cmd(17,clif_parse_UseItem,6,12);
	packet_db[8][0x00f7] = packet_cmd(10,clif_parse_TickSend,6);
	packet_db[8][0x0113] = packet_cmd(16,clif_parse_MoveToKafra,5,12);
	packet_db[8][0x0116] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[8][0x0190] = packet_cmd(26,clif_parse_MoveFromKafra,10,22);
	packet_db[8][0x0193] = packet_cmd(9,clif_parse_ActionRequest,3,8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 9 (2004-08-09)(2004-08-16)(2004-08-17)
	packet_db[9] = packet_db[8];
	packet_db[9].connect_cmd = 0x7e;
	packet_db[9][0x0072] = packet_cmd(17,clif_parse_DropItem,8,15);
	packet_db[9][0x007e] = packet_cmd(37,clif_parse_WantToConnection,9,21,28,32,36);
	packet_db[9][0x0085] = packet_cmd(26,clif_parse_UseSkillToId,11,18,22);
	packet_db[9][0x0089] = packet_cmd(12,clif_parse_GetCharNameRequest,8);
	packet_db[9][0x008c] = packet_cmd(40,clif_parse_UseSkillToPos,5,15,29,38);
	packet_db[9][0x0094] = packet_cmd(13,clif_parse_TakeItem,9);
	packet_db[9][0x009b] = packet_cmd(15,clif_parse_WalkToXY,12);
	packet_db[9][0x009f] = packet_cmd(12,clif_parse_ChangeDir,7,11);
	packet_db[9][0x00a2] = packet_cmd(120,clif_parse_UseSkillToPos,5,15,29,38,40);
	packet_db[9][0x00a7] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[9][0x00f5] = packet_cmd(24,clif_parse_UseItem,9,20);
	packet_db[9][0x00f7] = packet_cmd(13,clif_parse_TickSend,9);
	packet_db[9][0x0113] = packet_cmd(23,clif_parse_MoveToKafra,5,19);
	packet_db[9][0x0190] = packet_cmd(26,clif_parse_MoveFromKafra,11,22);
	packet_db[9][0x0193] = packet_cmd(18,clif_parse_ActionRequest,7,17);
	packet_db[9][0x0211] = packet_cmd(0);
	packet_db[9][0x0212] = packet_cmd(26);
	packet_db[9][0x0213] = packet_cmd(26);
	packet_db[9][0x0214] = packet_cmd(42);
	packet_db[9][0x020f] = packet_cmd(10);
	packet_db[9][0x0210] = packet_cmd(22);
	///////////////////////////////////////////////////////////////////////////
	// packet version 10 (2004-09-06)
	packet_db[10] = packet_db[9];
	packet_db[10].connect_cmd = 0xf5;
	packet_db[10][0x0072] = packet_cmd(20,clif_parse_UseItem,9,20);
	packet_db[10][0x007e] = packet_cmd(19,clif_parse_MoveToKafra,3,15);
	packet_db[10][0x0085] = packet_cmd(23,clif_parse_ActionRequest,9,22);
	packet_db[10][0x0089] = packet_cmd(9,clif_parse_WalkToXY,6);
	packet_db[10][0x008c] = packet_cmd(105,clif_parse_UseSkillToPos,10,14,18,23,25);
	packet_db[10][0x0094] = packet_cmd(17,clif_parse_DropItem,6,15);
	packet_db[10][0x009b] = packet_cmd(14,clif_parse_GetCharNameRequest,10);
	packet_db[10][0x009f] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[10][0x00a2] = packet_cmd(14,clif_parse_SolveCharName,10);
	packet_db[10][0x00a7] = packet_cmd(25,clif_parse_UseSkillToPos,10,14,18,23);
	packet_db[10][0x00f3] = packet_cmd(10,clif_parse_ChangeDir,4,9);
	packet_db[10][0x00f5] = packet_cmd(34,clif_parse_WantToConnection,7,15,25,29,33);
	packet_db[10][0x00f7] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[10][0x0113] = packet_cmd(11,clif_parse_TakeItem,7);
	packet_db[10][0x0116] = packet_cmd(11,clif_parse_TickSend,7);
	packet_db[10][0x0190] = packet_cmd(22,clif_parse_UseSkillToId,9,15,18);
	packet_db[10][0x0193] = packet_cmd(17,clif_parse_MoveFromKafra,3,13);
	///////////////////////////////////////////////////////////////////////////
	// packet version 11 (2004-09-21)
	packet_db[11] = packet_db[10];
	packet_db[11].connect_cmd = 0xf5;
	packet_db[11][0x0072] = packet_cmd(18,clif_parse_UseItem,10,14);
	packet_db[11][0x007e] = packet_cmd(25,clif_parse_MoveToKafra,6,21);
	packet_db[11][0x0085] = packet_cmd(9,clif_parse_ActionRequest,3,8);
	packet_db[11][0x0089] = packet_cmd(14,clif_parse_WalkToXY,11);
	packet_db[11][0x008c] = packet_cmd(109,clif_parse_UseSkillToPos,16,20,23,27,29);
	packet_db[11][0x0094] = packet_cmd(19,clif_parse_DropItem,12,17);
	packet_db[11][0x00a2] = packet_cmd(10,clif_parse_SolveCharName,6);
	packet_db[11][0x00a7] = packet_cmd(29,clif_parse_UseSkillToPos,6,20,23,27);
	packet_db[11][0x00f3] = packet_cmd(18,clif_parse_ChangeDir,8,17);
	packet_db[11][0x00f5] = packet_cmd(32,clif_parse_WantToConnection,10,17,23,27,31);
	packet_db[11][0x009b] = packet_cmd(10,clif_parse_GetCharNameRequest,6);
	packet_db[11][0x0113] = packet_cmd(14,clif_parse_TakeItem,10);
	packet_db[11][0x0116] = packet_cmd(14,clif_parse_TickSend,10);
	packet_db[11][0x0190] = packet_cmd(14,clif_parse_UseSkillToId,4,7,10);
	packet_db[11][0x0193] = packet_cmd(12,clif_parse_MoveFromKafra,4,8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 12 (2004-12-18)
	packet_db[12] = packet_db[11];
	packet_db[12].connect_cmd = 0xf5;
	packet_db[12][0x0072] = packet_cmd(17,clif_parse_UseItem,6,13);
	packet_db[12][0x007e] = packet_cmd(16,clif_parse_MoveToKafra,5,12);
	packet_db[12][0x0089] = packet_cmd(6,clif_parse_WalkToXY,3);
	packet_db[12][0x008c] = packet_cmd(103,clif_parse_UseSkillToPos,2,6,17,21,23);
	packet_db[12][0x0094] = packet_cmd(14,clif_parse_DropItem,5,12);
	packet_db[12][0x009b] = packet_cmd(15,clif_parse_GetCharNameRequest,11);
	packet_db[12][0x00a2] = packet_cmd(12,clif_parse_SolveCharName,8);
	packet_db[12][0x00a7] = packet_cmd(23,clif_parse_UseSkillToPos,3,6,17,21);
	packet_db[12][0x00f3] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[12][0x00f5] = packet_cmd(33,clif_parse_WantToConnection,12,18,24,28,32);
	packet_db[12][0x0113] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[12][0x0116] = packet_cmd(10,clif_parse_TickSend,6);
	packet_db[12][0x0190] = packet_cmd(20,clif_parse_UseSkillToId,7,12,16);
	packet_db[12][0x0193] = packet_cmd(26,clif_parse_MoveFromKafra,10,22);
	///////////////////////////////////////////////////////////////////////////
	// packet version 13 (2004-10-25)
	packet_db[13] = packet_db[12];
	packet_db[13].connect_cmd = 0xf5;
	packet_db[13][0x0072] = packet_cmd(13,clif_parse_UseItem,5,9);
	packet_db[13][0x007e] = packet_cmd(13,clif_parse_MoveToKafra,6,9);
	packet_db[13][0x0085] = packet_cmd(15,clif_parse_ActionRequest,4,14);
	packet_db[13][0x008c] = packet_cmd(108,clif_parse_UseSkillToPos,6,9,23,26,28);
	packet_db[13][0x0094] = packet_cmd(12,clif_parse_DropItem,6,10);
	packet_db[13][0x009b] = packet_cmd(10,clif_parse_GetCharNameRequest,6);
	packet_db[13][0x00a2] = packet_cmd(16,clif_parse_SolveCharName,12);
	packet_db[13][0x00a7] = packet_cmd(28,clif_parse_UseSkillToPos,6,9,23,26);
	packet_db[13][0x00f3] = packet_cmd(15,clif_parse_ChangeDir,6,14);
	packet_db[13][0x00f5] = packet_cmd(29,clif_parse_WantToConnection,5,14,20,24,28);
	packet_db[13][0x0113] = packet_cmd(9,clif_parse_TakeItem,5);
	packet_db[13][0x0116] = packet_cmd(9,clif_parse_TickSend,5);
	packet_db[13][0x0190] = packet_cmd(26,clif_parse_UseSkillToId,4,10,22);
	packet_db[13][0x0193] = packet_cmd(22,clif_parse_MoveFromKafra,12,18);
	///////////////////////////////////////////////////////////////////////////
	// packet version 14 (2004-11-01)
	packet_db[14] = packet_db[13];
	packet_db[14].connect_cmd = 0;// packet version disabled
	packet_db[14][0x0143] = packet_cmd(23,clif_parse_NpcAmountInput,2,6);
	packet_db[14][0x0215] = packet_cmd(6);
	packet_db[14][0x0216] = packet_cmd(6);
	///////////////////////////////////////////////////////////////////////////
	// packet version 15 (2004-12-04)
	packet_db[15] = packet_db[14];
	packet_db[15].connect_cmd = 0xf5;
	packet_db[15][0x0190] = packet_cmd(15,clif_parse_UseItem,3,11);
	packet_db[15][0x0094] = packet_cmd(14,clif_parse_MoveToKafra,4,10);
	packet_db[15][0x009f] = packet_cmd(18,clif_parse_ActionRequest,6,17);
	packet_db[15][0x00a7] = packet_cmd(7,clif_parse_WalkToXY,4);
	packet_db[15][0x007e] = packet_cmd(30,clif_parse_UseSkillToPos,4,9,22,28);
	packet_db[15][0x0116] = packet_cmd(12,clif_parse_DropItem,4,10);
	packet_db[15][0x008c] = packet_cmd(13,clif_parse_GetCharNameRequest,9);
	packet_db[15][0x0085] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[15][0x00f7] = packet_cmd(14,clif_parse_SolveCharName,10);
	packet_db[15][0x0113] = packet_cmd(110,clif_parse_UseSkillToPos,4,9,22,28,30);
	packet_db[15][0x00f3] = packet_cmd(8,clif_parse_ChangeDir,3,7);
	packet_db[15][0x00f5] = packet_cmd(29,clif_parse_WantToConnection,3,10,20,24,28);
	packet_db[15][0x00a2] = packet_cmd(7,clif_parse_TakeItem,3);
	packet_db[15][0x0089] = packet_cmd(7,clif_parse_TickSend,3);
	packet_db[15][0x0072] = packet_cmd(22,clif_parse_UseSkillToId,8,12,18);
	packet_db[15][0x0193] = packet_cmd(21,clif_parse_MoveFromKafra,4,17);
	packet_db[15][0x009b] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[15][0x0217] = packet_cmd(2,clif_parse_Blacksmith,0);
	packet_db[15][0x0218] = packet_cmd(2,clif_parse_Alchemist,0);
	packet_db[15][0x0219] = packet_cmd(282);
	packet_db[15][0x021a] = packet_cmd(282);
	packet_db[15][0x021b] = packet_cmd(10);
	packet_db[15][0x021c] = packet_cmd(10);
	packet_db[15][0x021d] = packet_cmd(6);
	packet_db[15][0x021e] = packet_cmd(6);
	packet_db[15][0x0221] = packet_cmd(-1);
	packet_db[15][0x0222] = packet_cmd(6,clif_parse_WeaponRefine,2);
	packet_db[15][0x0223] = packet_cmd(8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 16 (2005-01-10)
	packet_db[16] = packet_db[15];
	packet_db[16].connect_cmd = 0x9b;
	packet_db[16][0x009b] = packet_cmd(32,clif_parse_WantToConnection,3,12,23,27,31);
	packet_db[16][0x0089] = packet_cmd(9,clif_parse_TickSend,5);
	packet_db[16][0x00a7] = packet_cmd(13,clif_parse_WalkToXY,10);
	packet_db[16][0x0190] = packet_cmd(20,clif_parse_ActionRequest,9,19);
	packet_db[16][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[16][0x008c] = packet_cmd(8,clif_parse_GetCharNameRequest,4);
	packet_db[16][0x0085] = packet_cmd(23,clif_parse_ChangeDir,12,22);
	packet_db[16][0x0094] = packet_cmd(20,clif_parse_MoveToKafra,10,16);
	packet_db[16][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[16][0x00f7] = packet_cmd(21,clif_parse_MoveFromKafra,11,17);
	packet_db[16][0x009f] = packet_cmd(17,clif_parse_UseItem,5,13);
	packet_db[16][0x0116] = packet_cmd(20,clif_parse_DropItem,15,18);
	packet_db[16][0x00f5] = packet_cmd(9,clif_parse_TakeItem,5);
	packet_db[16][0x0113] = packet_cmd(34,clif_parse_UseSkillToPos,10,18,22,32);
	packet_db[16][0x0072] = packet_cmd(26,clif_parse_UseSkillToId,8,16,22);
	packet_db[16][0x007e] = packet_cmd(114,clif_parse_UseSkillToPos,10,18,22,32,34);
	packet_db[16][0x00a2] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[16][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[16][0x021f] = packet_cmd(66);
	packet_db[16][0x0220] = packet_cmd(10);
	///////////////////////////////////////////////////////////////////////////
	// packet version 17 (2005-05-10)
	packet_db[17] = packet_db[16];
	packet_db[17].connect_cmd = 0x9b;
	packet_db[17][0x009b] = packet_cmd(26,clif_parse_WantToConnection,4,9,17,18,25);
	packet_db[17][0x0089] = packet_cmd(8,clif_parse_TickSend,4);
	packet_db[17][0x00a7] = packet_cmd(8,clif_parse_WalkToXY,5);
	packet_db[17][0x0190] = packet_cmd(19,clif_parse_ActionRequest,5,18);
	packet_db[17][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[17][0x008c] = packet_cmd(11,clif_parse_GetCharNameRequest,7);
	packet_db[17][0x0085] = packet_cmd(11,clif_parse_ChangeDir,7,10);
	packet_db[17][0x0094] = packet_cmd(14,clif_parse_MoveToKafra,7,10);
	packet_db[17][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[17][0x00f7] = packet_cmd(22,clif_parse_MoveFromKafra,14,18);
	packet_db[17][0x009f] = packet_cmd(14,clif_parse_UseItem,4,10);
	packet_db[17][0x0116] = packet_cmd(10,clif_parse_DropItem,5,8);
	packet_db[17][0x00f5] = packet_cmd(8,clif_parse_TakeItem,4);
	packet_db[17][0x0113] = packet_cmd(22,clif_parse_UseSkillToPos,5,9,12,20);
	packet_db[17][0x0072] = packet_cmd(25,clif_parse_UseSkillToId,6,10,21);
	packet_db[17][0x007e] = packet_cmd(102,clif_parse_UseSkillToPos,5,9,12,20,22);
	packet_db[17][0x00a2] = packet_cmd(15,clif_parse_SolveCharName,11);
	packet_db[17][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[17][0x0224] = packet_cmd(10);
	packet_db[17][0x0225] = packet_cmd(2);
	packet_db[17][0x0226] = packet_cmd(282);
	packet_db[17][0x0227] = packet_cmd(18);
	packet_db[17][0x0228] = packet_cmd(18);
	packet_db[17][0x0229] = packet_cmd(15);
	packet_db[17][0x022a] = packet_cmd(58);
	packet_db[17][0x022b] = packet_cmd(57);
	packet_db[17][0x022c] = packet_cmd(64);
	packet_db[17][0x022d] = packet_cmd(5);
	packet_db[17][0x0232] = packet_cmd(9);
	packet_db[17][0x0233] = packet_cmd(11);
	packet_db[17][0x0234] = packet_cmd(-1);
	///////////////////////////////////////////////////////////////////////////
	// packet version 18 (2005-06-28)
	packet_db[18] = packet_db[17];
	packet_db[18].connect_cmd = 0x9b;
	packet_db[18][0x009b] = packet_cmd(32,clif_parse_WantToConnection,9,15,23,30,31);
	packet_db[18][0x0089] = packet_cmd(13,clif_parse_TickSend,9);
	packet_db[18][0x00a7] = packet_cmd(11,clif_parse_WalkToXY,8);
	packet_db[18][0x0190] = packet_cmd(24,clif_parse_ActionRequest,11,23);
	packet_db[18][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[18][0x008c] = packet_cmd(8,clif_parse_GetCharNameRequest,4);
	packet_db[18][0x0085] = packet_cmd(17,clif_parse_ChangeDir,8,16);
	packet_db[18][0x0094] = packet_cmd(31,clif_parse_MoveToKafra,16,27);
	packet_db[18][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[18][0x00f7] = packet_cmd(18,clif_parse_MoveFromKafra,11,14);
	packet_db[18][0x009f] = packet_cmd(19,clif_parse_UseItem,9,15);
	packet_db[18][0x0116] = packet_cmd(12,clif_parse_DropItem,3,10);
	packet_db[18][0x00f5] = packet_cmd(13,clif_parse_TakeItem,9);
	packet_db[18][0x0113] = packet_cmd(33,clif_parse_UseSkillToPos,12,15,18,31);
	packet_db[18][0x0072] = packet_cmd(34,clif_parse_UseSkillToId,6,17,30);
	packet_db[18][0x007e] = packet_cmd(113,clif_parse_UseSkillToPos,12,15,18,31,33);
	packet_db[18][0x00a2] = packet_cmd(9,clif_parse_SolveCharName,5);
	packet_db[18][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[18][0x020e] = packet_cmd(10);
	packet_db[18][0x022e] = packet_cmd(71);
	packet_db[18][0x0235] = packet_cmd(115);
	packet_db[18][0x0248] = packet_cmd(116);
	///////////////////////////////////////////////////////////////////////////
	// packet version 19 (2005-06-28)
	packet_db[19] = packet_db[18];
	packet_db[19].connect_cmd = 0x9b;
	packet_db[19][0x009b] = packet_cmd(37,clif_parse_WantToConnection,9,21,28,32,36);
	packet_db[19][0x00a2] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[19][0x008c] = packet_cmd(12,clif_parse_GetCharNameRequest,8);
	packet_db[19][0x00a7] = packet_cmd(15,clif_parse_WalkToXY,12);
	packet_db[19][0x0116] = packet_cmd(17,clif_parse_DropItem,8,15);
	packet_db[19][0x009f] = packet_cmd(24,clif_parse_UseItem,9,20);
	packet_db[19][0x0072] = packet_cmd(26,clif_parse_UseSkillToId,11,18,22);
	packet_db[19][0x0113] = packet_cmd(40,clif_parse_UseSkillToPos,5,15,29,38);
	packet_db[19][0x007e] = packet_cmd(120,clif_parse_UseSkillToPos,5,15,29,38,40);
	packet_db[19][0x0085] = packet_cmd(12,clif_parse_ChangeDir,7,11);
	packet_db[19][0x0094] = packet_cmd(23,clif_parse_MoveToKafra,5,19);
	packet_db[19][0x00f7] = packet_cmd(26,clif_parse_MoveFromKafra,11,22);
	packet_db[19][0x0190] = packet_cmd(18,clif_parse_ActionRequest,7,17);


	///////////////////////////////////////////////////////////////////////////
	size_t i;
	for(i=20; i<=MAX_PACKET_VER; ++i)
		packet_db[i] = packet_db[i-1];
	///////////////////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////
	// read in packet_db from file
	FILE *fp;
	char line[1024];
	int ln=0;
	int cmd;
	size_t j, packet_ver;
	int k;
	char *str[64],*p,*str2[64],*p2,w1[64],w2[64];

	static const struct {
		int (*func)(int, struct map_session_data &);
		char *name;
	} clif_parse_func[]={
		{clif_parse_WantToConnection,"wanttoconnection"},
		{clif_parse_LoadEndAck,"loadendack"},
		{clif_parse_TickSend,"ticksend"},
		{clif_parse_WalkToXY,"walktoxy"},
		{clif_parse_QuitGame,"quitgame"},
		{clif_parse_GetCharNameRequest,"getcharnamerequest"},
		{clif_parse_GlobalMessage,"globalmessage"},
		{clif_parse_MapMove,"mapmove"},
		{clif_parse_ChangeDir,"changedir"},
		{clif_parse_Emotion,"emotion"},
		{clif_parse_HowManyConnections,"howmanyconnections"},
		{clif_parse_ActionRequest,"actionrequest"},
		{clif_parse_Restart,"restart"},
		{clif_parse_Wis,"wis"},
		{clif_parse_GMmessage,"gmmessage"},
		{clif_parse_TakeItem,"takeitem"},
		{clif_parse_DropItem,"dropitem"},
		{clif_parse_UseItem,"useitem"},
		{clif_parse_EquipItem,"equipitem"},
		{clif_parse_UnequipItem,"unequipitem"},
		{clif_parse_NpcClicked,"npcclicked"},
		{clif_parse_NpcBuySellSelected,"npcbuysellselected"},
		{clif_parse_NpcBuyListSend,"npcbuylistsend"},
		{clif_parse_NpcSellListSend,"npcselllistsend"},
		{clif_parse_CreateChatRoom,"createchatroom"},
		{clif_parse_ChatAddMember,"chataddmember"},
		{clif_parse_ChatRoomStatusChange,"chatroomstatuschange"},
		{clif_parse_ChangeChatOwner,"changechatowner"},
		{clif_parse_KickFromChat,"kickfromchat"},
		{clif_parse_ChatLeave,"chatleave"},
		{clif_parse_TradeRequest,"traderequest"},
		{clif_parse_TradeAck,"tradeack"},
		{clif_parse_TradeAddItem,"tradeadditem"},
		{clif_parse_TradeOk,"tradeok"},
		{clif_parse_TradeCancel,"tradecancel"},
		{clif_parse_TradeCommit,"tradecommit"},
		{clif_parse_StopAttack,"stopattack"},
		{clif_parse_PutItemToCart,"putitemtocart"},
		{clif_parse_GetItemFromCart,"getitemfromcart"},
		{clif_parse_RemoveOption,"removeoption"},
		{clif_parse_ChangeCart,"changecart"},
		{clif_parse_StatusUp,"statusup"},
		{clif_parse_SkillUp,"skillup"},
		{clif_parse_UseSkillToId,"useskilltoid"},
		{clif_parse_UseSkillToPos,"useskilltopos"},
		{clif_parse_UseSkillToPos,"useskilltoposinfo"},
//		{clif_parse_UseSkillToPosMoreInfo,"useskilltoposinfo"},
		{clif_parse_UseSkillMap,"useskillmap"},
		{clif_parse_RequestMemo,"requestmemo"},
		{clif_parse_ProduceMix,"producemix"},
		{clif_parse_NpcSelectMenu,"npcselectmenu"},
		{clif_parse_NpcNextClicked,"npcnextclicked"},
		{clif_parse_NpcAmountInput,"npcamountinput"},
		{clif_parse_NpcStringInput,"npcstringinput"},
		{clif_parse_NpcCloseClicked,"npccloseclicked"},
		{clif_parse_ItemIdentify,"itemidentify"},
		{clif_parse_SelectArrow,"selectarrow"},
		{clif_parse_AutoSpell,"autospell"},
		{clif_parse_UseCard,"usecard"},
		{clif_parse_InsertCard,"insertcard"},
		{clif_parse_RepairItem,"repairitem"},
		{clif_parse_WeaponRefine,"weaponrefine"},
		{clif_parse_SolveCharName,"solvecharname"},
		{clif_parse_ResetChar,"resetchar"},
		{clif_parse_LGMmessage,"lgmmessage"},
		{clif_parse_MoveToKafra,"movetokafra"},
		{clif_parse_MoveFromKafra,"movefromkafra"},
		{clif_parse_MoveToKafraFromCart,"movetokafrafromcart"},
		{clif_parse_MoveFromKafraToCart,"movefromkafratocart"},
		{clif_parse_CloseKafra,"closekafra"},
		{clif_parse_CreateParty,"createparty"},
		{clif_parse_CreateParty2,"createparty2"},
		{clif_parse_PartyInvite,"partyinvite"},
		{clif_parse_ReplyPartyInvite,"replypartyinvite"},
		{clif_parse_LeaveParty,"leaveparty"},
		{clif_parse_RemovePartyMember,"removepartymember"},
		{clif_parse_PartyChangeOption,"partychangeoption"},
		{clif_parse_PartyMessage,"partymessage"},
		{clif_parse_CloseVending,"closevending"},
		{clif_parse_VendingListReq,"vendinglistreq"},
		{clif_parse_PurchaseReq,"purchasereq"},
		{clif_parse_OpenVending,"openvending"},
		{clif_parse_CreateGuild,"createguild"},
		{clif_parse_GuildCheckMaster,"guildcheckmaster"},
		{clif_parse_GuildRequestInfo,"guildrequestinfo"},
		{clif_parse_GuildChangePositionInfo,"guildchangepositioninfo"},
		{clif_parse_GuildChangeMemberPosition,"guildchangememberposition"},
		{clif_parse_GuildRequestEmblem,"guildrequestemblem"},
		{clif_parse_GuildChangeEmblem,"guildchangeemblem"},
		{clif_parse_GuildChangeNotice,"guildchangenotice"},
		{clif_parse_GuildInvite,"guildinvite"},
		{clif_parse_GuildReplyInvite,"guildreplyinvite"},
		{clif_parse_GuildLeave,"guildleave"},
		{clif_parse_GuildExplusion,"guildexplusion"},
		{clif_parse_GuildMessage,"guildmessage"},
		{clif_parse_GuildRequestAlliance,"guildrequestalliance"},
		{clif_parse_GuildReplyAlliance,"guildreplyalliance"},
		{clif_parse_GuildDelAlliance,"guilddelalliance"},
		{clif_parse_GuildOpposition,"guildopposition"},
		{clif_parse_GuildBreak,"guildbreak"},
		{clif_parse_PetMenu,"petmenu"},
		{clif_parse_CatchPet,"catchpet"},
		{clif_parse_SelectEgg,"selectegg"},
		{clif_parse_SendEmotion,"sendemotion"},
		{clif_parse_ChangePetName,"changepetname"},
		{clif_parse_GMKick,"gmkick"},
		{clif_parse_GMHide,"gmhide"},
		{clif_parse_GMReqNoChat,"gmreqnochat"},
		{clif_parse_GMReqNoChatCount,"gmreqnochatcount"},
		{clif_parse_NoviceDoriDori,"sndoridori"},
		{clif_parse_NoviceExplosionSpirits,"snexplosionspirits"},
		{clif_parse_PMIgnore,"wisexin"},
		{clif_parse_PMIgnoreList,"wisexlist"},
		{clif_parse_PMIgnoreAll,"wisall"},
		{clif_parse_GMKillAll,"killall"},
		{clif_parse_Recall,"summon"},
		{clif_parse_GM_Monster_Item,"itemmonster"},
		{clif_parse_Shift,"shift"},

		{clif_parse_FriendsListAdd,"friendslistadd"},
		{clif_parse_FriendsListAdd,"friendaddrequest"},
		{clif_parse_FriendsListRemove,"friendslistremove"},
		{clif_parse_FriendsListRemove,"frienddeleterequest"},
		{clif_parse_FriendsListReply,"friendslistreply"},
		{clif_parse_FriendsListReply,"friendaddreply"},
		
		{clif_parse_Blacksmith,"blacksmith"},
		{clif_parse_Blacksmith,"rankingblacksmith"},
		{clif_parse_Alchemist,"alchemist"},
		{clif_parse_Alchemist,"rankingalchemist"},
		{clif_parse_Taekwon,"taekwon"},
		{clif_parse_Taekwon,"rankingtaekwon"},
		{clif_parse_RankingPk,"rankingpk"},
		{clif_parse_BabyRequest,"babyrequest"},

		{clif_parse_HomMenu,"hommenu"},
		{clif_parse_HomWalkMaster,"homwalkmaster"},
		{clif_parse_HomWalkToXY,"homwalktoxy"},
		{clif_parse_HomActionRequest,"homactionrequest"},
		{clif_parse_ChangeHomName,"changehomname"},

		{clif_parse_MailWinOpen,"mailwinopen"},
		{clif_parse_ReadMail,"readmail"},
		{clif_parse_MailGetAppend,"mailgetappend"},
		{clif_parse_SendMail,"sendmail"},
		{clif_parse_RefreshMailBox,"refleshmailbox"},
		{clif_parse_SendMailSetAppend,"sendmailsetappend"},
		{clif_parse_DeleteMail,"deletemail"},

		{clif_parse_FeelSaveOk,"feelsaveok"},
		{clif_parse_AdoptRequest,"adopt"},
		
		{clif_parse_clientsetting,"clientsetting"},
		{clif_parse_debug,"debug"},
		{NULL,NULL}
	};

	if( (fp=basics::safefopen("db/packet_db.txt","r"))==NULL )
	{
		ShowWarning("can't read db/packet_db.txt, using defaults\n");		
		return 1;
	}

	packet_ver = MAX_PACKET_VER;
	while( fgets(line,sizeof(line),fp) )
	{
		ln++;
		if( !get_prepared_line(line) )
			continue;

		if (sscanf(line,"%[^:]: %[^\r\n]",w1,w2) == 2)
		{
			if(strcasecmp(w1,"packet_ver")==0)
			{	// start of a new version
				size_t i, prev_ver = packet_ver;
				packet_ver = atoi(w2);
				if (packet_ver > MAX_PACKET_VER)
				{	//Check to avoid overflowing. [Skotlex]
					ShowWarning("The packet_db table only has support up to version %d\n", MAX_PACKET_VER);
					break;
				}
				// copy from previous version into new version and continue
				// - indicating all following packets should be read into the newer version
				for(i=prev_ver+1; i<=packet_ver; ++i)
					packet_db[i]=packet_db[prev_ver];
				continue;
			}
			else if(strcasecmp(w1,"enable_packet_db")==0)
			{	// only working when found in the beginning of the packet_db file
				if( !config_switch(w2) )
					return 0;
				continue;
			}
			else if(strcasecmp(w1,"default_packet_ver")==0)
			{	// use this when in daubt, but pretty useless
				packet_db.default_ver = config_switch(w2);
				// check for invalid version
				if( packet_db.default_ver > MAX_PACKET_VER ||
					packet_db.default_ver < 0 )
					packet_db.default_ver = MAX_PACKET_VER;
				continue;
			}
		}

		if(packet_ver<=MAX_PACKET_VER)
		{	// only read valid packet_ver's
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<4 && p; ++j)
			{
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}
			if(str[0]==NULL)
				continue;
			cmd=strtol(str[0],(char **)NULL,0);
			if(cmd<=0 || cmd>=MAX_PACKET_DB)
				continue;
			if(str[1]==NULL)
			{
				ShowError("packet_db: packet len error (line %i\n)", ln);
				continue;
			}
			k = atoi(str[1]);
			packet_db[packet_ver][cmd].len = k;

			if(str[2]==NULL){
				continue;
			}
			for(j=0;j<sizeof(clif_parse_func)/sizeof(clif_parse_func[0]); ++j)
			{
				if( clif_parse_func[j].name != NULL &&
					strcmp(str[2],clif_parse_func[j].name)==0)
				{
					// if (packet_db[packet_ver][cmd].func != clif_parse_func[j].func && !clif_config.prefer_packet_db)
					//	break;	// not used for now
					packet_db[packet_ver][cmd].func = clif_parse_func[j].func;
					break;
				}
			}
			if( j>=sizeof(clif_parse_func)/sizeof(clif_parse_func[0]) )
			{
				ShowError("packet_db (line %i): parse command '%s' not found\n", ln, str[2]);
			}
			// set the identifying cmd for the packet_db version
			if(strcasecmp(str[2],"wanttoconnection")==0)
			{
				packet_db[packet_ver].connect_cmd = cmd;
			}
			if(str[3]==NULL)
			{
				ShowError("packet_db (line %i): no positions\n", ln);
			}
			for(j=0,p2=str[3];p2; ++j){
				str2[j]=p2;
				p2=strchr(p2,':');
				if(p2) *p2++=0;
				k = atoi(str2[j]);
				packet_db[packet_ver][cmd].pos[j] = k;
			}
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/packet_db.txt");
	return 0;
}

int check_connect_map_port()
{
	if( !session_isActive(map_fd) )
	{	// the listen port was dropped, open it new
		map_fd = make_listen(mapaddress.LANIP(), mapaddress.LANPort());
		if(map_fd>=0)
			ShowStatus("Server is '"CL_BT_GREEN"ready"CL_RESET"' and listening on '"CL_WHITE"%s:%d"CL_RESET"'.\n\n", getmapaddress().LANIP().tostring(NULL), getmapaddress().LANPort());
		else
			ShowError("open listening socket on '"CL_WHITE"%s:%d"CL_RESET"' failed.\n\n", getmapaddress().LANIP().tostring(NULL), getmapaddress().LANPort());
	}
	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int do_init_clif(void)
{
	basics::CParam<bool> automatic_wan_setup("automatic_wan_setup",false);
	if( automatic_wan_setup )
		initialize_WAN(mapaddress, 5121);
	
	server_char_id    = npc_get_new_npc_id();
	server_fake_mob_id= npc_get_new_npc_id();
	
	packetdb_readdb();
	set_defaultparse(clif_parse);

	add_timer_func_list(clif_clearchar_delay_sub, "clif_clearchar_delay_sub");
	return 0;
}




