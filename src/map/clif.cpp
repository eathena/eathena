// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

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
#include "flooritem.h"
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
#include "intif.h"
#include "battle.h"
#include "mob.h"
#include "party.h"
#include "guild.h"
#include "vending.h"
#include "pet.h"
#include "log.h"
#include "packetdb.h"




// local define
typedef enum
{
	ALL_CLIENT=0,
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
} send_t;

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
inline void WBUFPOS2(unsigned char* p, size_t pos, unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned char sx0, unsigned char sy0)
{
	if(p)
	{
		// client-side:
		//   x0 += 0.0625*sx0 - 0.5
		//   y0 += 0.0625*sy0 - 0.5
		p += pos;
		*p++ = (unsigned char)(x0>>2);
		*p++ = (unsigned char)((x0<<6) | ((y0>>4)&0x3f));
		*p++ = (unsigned char)((y0<<4) | ((x1>>6)&0x0f));
		*p++ = (unsigned char)((x1<<2) | ((y1>>8)&0x03));
		*p++ = (unsigned char)(y1); 
		*p   = (unsigned char)((sx0<<4) | (sy0&0x0f));
	}
}

inline void WFIFOPOS(int fd, size_t pos, unsigned short x, unsigned short y, unsigned char d)
{
	WBUFPOS(WFIFOP(fd,pos),0,x,y,d); 
}
inline void WFIFOPOS2(int fd, size_t pos, unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned char sx0, unsigned char sy0)
{
	WBUFPOS2(WFIFOP(fd,pos),0,x0,y0,x1,y1,sx0,sy0); 
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
			fprintf(fp, "%sPlayer with account ID %lu (character ID %lu, player name %s) sent wrong packet:\n",
					asctime(localtime(&now)), (unsigned long)sd->status.account_id, (unsigned long)sd->status.char_id, sd->status.name);
		else
			fprintf(fp, "%sPlayer with account ID %lu sent wrong packet:\n",
					asctime(localtime(&now)), (unsigned long)sd->block_list::id);
	}
	else if (sd) // not authentified! (refused by char-server or disconnect before to be authentified)
		fprintf(fp, "%sPlayer with account ID %lu sent wrong packet:\n", 
				asctime(localtime(&now)), (unsigned long)sd->block_list::id);
	
	dump(RFIFOP(fd,0), packet_len, fp);

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

	dump(RFIFOP(fd,0), packet_len, stdout);
#endif
}



// globals for fake and weather objects
uint32 server_char_id	= 0;
uint32 server_mob_id	= 0;

void clif_ban_player(const map_session_data &sd, uint32 banoption, const char* reason)
{
	char message[1024];
	snprintf(message, sizeof(message), "Character '%s' (account: %lu, charid: %lu) %s", 
		sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, reason);
	intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message);

	if( banoption == 0)
	{	// ban/block disabled
		snprintf(message, sizeof(message),msg_txt(MSG_BAN_DISABLED));
	}
	else if( config.ban_bot & 0x80000000 )
	{	// value was negative -> block
		chrif_char_ask_name(-1, sd.status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
		session_Remove(sd.fd); // forced to disconnect because of the hack
		// message about the ban
		snprintf(message, sizeof(message),msg_txt(MSG_BLOCKED), config.ban_spoof_namer);
	}
	else
	{	// positive value -> gives minutes of ban time
		chrif_char_ask_name(-1, sd.status.name, 2, 0, 0, 0, 0, config.ban_bot, 0); // type: 2 - ban (year, month, day, hour, minute, second)
		session_Remove(sd.fd); // forced to disconnect because of the hack
		// message about the ban
		snprintf(message, sizeof(message),msg_txt(MSG_BANNED_D_MINUTES), config.ban_spoof_namer);
	}
	intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, message);
}



///////////////////////////////////////////////////////////////////////////////
// freya anti-bot system
inline unsigned short get_fakemob_id(map_session_data &sd)
{
	// base randomizer; the other variables are known by the client
	static const int base = rand();
	// choose a mob which the client cannot trace back
	static unsigned short fake_mob_list[] =
	{	// set here mobs that do not sound when they don't move
		1001, // scorpion
		1002, // poring
		1007, // fabre
		1031, // poporing
		1022  // werewolf
	};
	unsigned short id=fake_mob_list[(base + sd.block_list::m + sd.fd + sd.status.char_id) % (sizeof(fake_mob_list) / sizeof(fake_mob_list[0]))];
	return ( mobdb_checkid(id) >0 ) ? id : fake_mob_list[0];
}

void check_fake_id(int fd, map_session_data &sd, uint32 target_id)
{
	// if player asks for the fake player (only bot and modified client can see a hiden player)
	if (target_id == server_char_id)
	{
		clif_ban_player(sd, config.ban_bot, "can see fake objects");

		// send this info cause the bot ask until get an answer, damn spam
		memset(WFIFOP(fd,0), 0, packet(sd.packet_ver,0x95).len);
		WFIFOW(fd,0) = 0x95;
		WFIFOL(fd,2) = server_char_id;
		memcpy(WFIFOP(fd,6), sd.status.name, 24);
		WFIFOSET(fd, packet(sd.packet_ver,0x95).len);

		// take fake player out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_char_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet(sd.packet_ver,0x80).len);
		// take fake mob out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_mob_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet(sd.packet_ver,0x80).len);
		return;
	}

	// if player asks for the fake mob (only bot and modified client can see a hiden mob)
	if (target_id == server_mob_id)
	{
		clif_ban_player(sd, config.ban_bot, "can see fake objects");

		// send this info cause the bot ask until get an answer, damn spam
		memset(WFIFOP(fd,0), 0, packet(sd.packet_ver,0x95).len);
		WFIFOW(fd,0) = 0x95;
		WFIFOL(fd,2) = server_mob_id;
		memcpy(WFIFOP(fd,6), mob_db[ get_fakemob_id(sd) ].name, 24);
		WFIFOSET(fd, packet(sd.packet_ver,0x95).len);

		// take fake mob out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_mob_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet(sd.packet_ver,0x80).len);
		// take fake player out of screen
		WFIFOW(fd,0) = 0x80;
		WFIFOL(fd,2) = server_char_id;
		WFIFOB(fd,6) = 0;
		WFIFOSET(fd, packet(sd.packet_ver,0x80).len);
		return;
	}
	
}
void send_fake_id(int fd, map_session_data &sd)
{	// if we try to detect bot
	if (config.ban_bot)
	{	// don't send fake player if we would not detect bot
		// send fake player (exactly same of the player, with HIDE option)

		// randomize positions of fake objects within a 2x2 area (prevents starting walk glitch)
		const uint32 r = rand();
		const ushort x1 = sd.block_list::x + ((((r&0x0003)    )>1)?((r&0x0003)    )-4:((r&0x0003)    )+1);
		const ushort y1 = sd.block_list::y + ((((r&0x000C)>> 2)>1)?((r&0x000C)>> 2)-4:((r&0x000C)>> 2)+1);
		const ushort d1 = ((r&0x0070)>> 4);
		const ushort x2 = sd.block_list::x + ((((r&0x0300)>> 8)>1)?((r&0x0300)>> 8)-4:((r&0x0300)>> 8)+1);
		const ushort y2 = sd.block_list::y + ((((r&0x0C00)>>10)>1)?((r&0x0C00)>>10)-4:((r&0x0C00)>>10)+1);
		const ushort d2 = ((r&0x7000)>>12);

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
		WFIFOPOS(fd, 46, x1, y1, d1);
		WFIFOB(fd,49) = 5;
		WFIFOB(fd,50) = 5;
		WFIFOB(fd,51) = sd.state.dead_sit;
		WFIFOW(fd,52) = (sd.status.base_level > config.max_lv) ? config.max_lv : sd.status.base_level;
		WFIFOSET(sd.fd, packet(sd.packet_ver,0x78).len);
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
		WFIFOPOS(fd, 46, x1, y1, d1);
		WFIFOB(fd,49) = 5;
		WFIFOB(fd,50) = 5;
		WFIFOB(fd,51) = sd.state.dead_sit;
		WFIFOW(fd,52) = ((level = sd.get_lv()) > config.max_job_level) ? config.max_job_level : level;
		WFIFOSET(sd.fd, packet(sd.packet_ver,0x1d8).len);
#endif
		// send a fake monster
		memset(WFIFOP(fd, 0), 0, packet(sd.packet_ver,0x7c).len);
		WFIFOW(fd, 0) = 0x7c;
		WFIFOL(fd, 2) = server_mob_id;
		WFIFOW(fd, 6) = sd.speed;
		WFIFOW(fd, 8) = 0;
		WFIFOW(fd,10) = 0;
		WFIFOW(fd,12) = OPTION_HIDE;
		WFIFOW(fd,20) = get_fakemob_id(sd);
		WFIFOPOS(fd, 36, x2, y2, d2);
		WFIFOSET(sd.fd, packet(sd.packet_ver,0x7c).len);
	}
}





///////////////////////////////////////////////////////////////////////////////
/// send skill failed message.
/// スキル詠唱失敗.
int clif_skill_failed(map_session_data& sd, ushort skill_id, skillfail_t type)
{
	//, uchar type, ushort btype
	// only when type==0:
	//  if(skillid==NV_BASIC)
	//   btype==0 "skill failed" MsgStringTable[159]
	//   btype==1 "no emotions" MsgStringTable[160]
	//   btype==2 "no sit" MsgStringTable[161]
	//   btype==3 "no chat" MsgStringTable[162]
	//   btype==4 "no party" MsgStringTable[163]
	//   btype==5 "no shout" MsgStringTable[164]
	//   btype==6 "no PKing" MsgStringTable[165]
	//   btype==7 "no alligning" MsgStringTable[383]
	//   btype>=8: ignored
	//  if(skillid==AL_WARP) "not enough skill level" MsgStringTable[214]
	//  if(skillid==TF_STEAL) "steal failed" MsgStringTable[205]
	//  if(skillid==TF_POISON) "envenom failed" MsgStringTable[207]
	//  otherwise "skill failed" MsgStringTable[204]
	// btype irrelevant
	//  type==1 "insufficient SP" MsgStringTable[202]
	//  type==2 "insufficient HP" MsgStringTable[203]
	//  type==3 "insufficient materials" MsgStringTable[808]
	//  type==4 "there is a delay after using a skill" MsgStringTable[219]
	//  type==5 "insufficient zeny" MsgStringTable[233]
	//  type==6 "wrong weapon" MsgStringTable[239]
	//  type==7 "red jemstone needed" MsgStringTable[246]
	//  type==8 "blue jemstone needed" MsgStringTable[247]
	//  type==9 "overweight" MsgStringTable[580]
	//  type==10 "skill failed" MsgStringTable[285]
	//  type>=11 ignored

	// if(success!=0) doesn't display any of the previous messages
	// Note: when this packet is received an unknown flag is always set to 0,
	// suggesting this is an ACK packet for the UseSkill packets and should be sent on success too [FlavioJS]

	// reset all variables [celest]
	sd.skillx    = sd.skilly      = 0xFFFF;
	sd.skillid   = sd.skilllv     = 0xFFFF;
	sd.skillitem = sd.skillitemlv = 0xFFFF;

	if( session_isActive(sd.fd) &&
		(type!=SF_DELAY || (config.display_delay_skill_fail && !sd.state.nodelay)) )
	{
		WFIFOW(sd.fd,0) = 0x110;
		WFIFOW(sd.fd,2) = skill_id;
		WFIFOL(sd.fd,4) = (type>>8)&0xFF;//btype;
		WFIFOB(sd.fd,8) = 0;// success;
		WFIFOB(sd.fd,9) = (type   )&0xFF;//type;
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x110).len);
	}
	return 0;
}












bool clif_packetsend(int fd, map_session_data &sd, unsigned short cmd, int info[], size_t sz)
{
	if(cmd <= MAX_PACKET_DB)
	{
		size_t i;
		switch (cmd)
		{
		case 0x209:
			WFIFOW(fd,0) = 0x209;
			WFIFOW(fd,2) = 2;
			memcpy(WFIFOP(fd, 12), sd.status.name, 24);
			WFIFOSET(fd, packet(sd.packet_ver,cmd).len);
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
			WFIFOSET(fd, packet(sd.packet_ver,cmd).len);
			break;
		}
		return true;
	}
	return false;
}



/*==========================================
 * 全てのclientに対してfunc()実行
 *------------------------------------------
 */
int clif_foreachclient(const CClifProcessor& elem)
{
	int returnvalue = 0;
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if( sd )
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

	virtual int process(block_list& bl) const
	{
		map_session_data& sd = (map_session_data&)bl;
		if( bl==BL_PC && session_isActive(sd.fd) )
		{
			switch(type) {
			case AREA_WOS:
			case AREA_CHAT_WOC:
				if(sd.block_list::id == src_bl.id)
					return 0;
				break;
			case AREA_WOC:
				if (sd.chat || sd.block_list::id == src_bl.id)
					return 0;
				break;
			case AREA_WOSC:
			{
				const map_session_data *ssd = src_bl.get_sd();
				if(ssd && sd.chat && sd.chat == ssd->chat)
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
				if(packet(sd.packet_ver,RBUFW(buf,0)).len)
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
	const map_session_data *sd = NULL;
	struct party *p = NULL;
	struct guild *g = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	if (type != ALL_CLIENT)
	{
		if(!bl)
		{
			printf("clif_send nullpo, head: %02X%02X%02X%02X type %i, len %lu\n", buf[0], buf[1], buf[2], buf[3], type, (unsigned long)len);
			return 0;
		}
		sd = bl->get_sd();
	}

	switch(type) {
	case ALL_CLIENT: // 全クライアントに送信
	{
		map_session_data::iterator iter(map_session_data::nickdb());
		for(; iter; ++iter)
		{
			sd = iter.data();
			if( sd )
			{
				if( packet(sd->packet_ver,RBUFW(buf,0)).len)
				{	// packet must exist for the client version
					memcpy(WFIFOP(sd->fd,0), buf, len);
					WFIFOSET(sd->fd,len);
				}
			}
		}
		break;
	}
	case ALL_SAMEMAP: // 同じマップの全クライアントに送信
	{
		map_session_data::iterator iter(map_session_data::nickdb());
		for(; iter; ++iter)
		{
			sd = iter.data();
			if( sd &&
				sd->block_list::m == bl->m )
			{
				if( packet(sd->packet_ver,RBUFW(buf,0)).len )
				{	// packet must exist for the client version
					memcpy(WFIFOP(sd->fd,0), buf, len);
					WFIFOSET(sd->fd,len);
				}
			}
		}
		break;
	}
	case AREA:
	case AREA_WOS:
	case AREA_WOC:
	case AREA_WOSC:
		block_list::foreachinarea( CClifSend(buf, len, *bl, type),
			bl->m, ((int)bl->x)-AREA_SIZE, ((int)bl->y)-AREA_SIZE, ((int)bl->x)+AREA_SIZE, ((int)bl->y)+AREA_SIZE,BL_PC);
//		map_foreachinarea(clif_send_sub, 
//			bl->m, ((int)bl->x)-AREA_SIZE, ((int)bl->y)-AREA_SIZE, ((int)bl->x)+AREA_SIZE, ((int)bl->y)+AREA_SIZE,BL_PC, 
//			buf, len, bl, type);
		break;
	case AREA_CHAT_WOC:
		block_list::foreachinarea( CClifSend(buf, len, *bl, type),
			bl->m, ((int)bl->x)-(AREA_SIZE-5), ((int)bl->y)-(AREA_SIZE-5),((int)bl->x)+(AREA_SIZE-5), ((int)bl->y)+(AREA_SIZE-5), BL_PC);
//		map_foreachinarea(clif_send_sub, 
//			bl->m, ((int)bl->x)-(AREA_SIZE-5), ((int)bl->y)-(AREA_SIZE-5),((int)bl->x)+(AREA_SIZE-5), ((int)bl->y)+(AREA_SIZE-5), BL_PC, 
//			buf, len, bl, AREA_WOC);
		break;
	case CHAT:
	case CHAT_WOS:
		{
			const chat_data *cd = (sd) ? sd->chat : bl->get_cd();
			if(cd)
			{		
				size_t i;
				for(i=0; i<cd->users; ++i)
				{
					if (type == CHAT_WOS && cd->usersd[i] == sd)
						continue;
					if( session_isActive(cd->usersd[i]->fd) && packet(cd->usersd[i]->packet_ver,RBUFW(buf,0)).len)
					{
						memcpy(WFIFOP(cd->usersd[i]->fd,0), buf, len);
						WFIFOSET(cd->usersd[i]->fd,len);
					}
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
		if (p)
		{
			size_t i;
			for(i=0;i<MAX_PARTY; ++i)
			{
				if ((sd = p->member[i].sd) != NULL)
				{
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
//					if(config.etc_log)
//						ShowMessage("send party %d %d %d\n",p->party_id,i,flag)
				}
			}

			map_session_data::iterator iter(map_session_data::nickdb());
			for(; iter; ++iter)
			{
				sd = iter.data();
				if( sd &&
					sd->partyspy == p->party_id)
				{
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
		}
		break;
	case SELF:
		if (sd && session_isActive(sd->fd) && session[sd->fd]->user_session && packet(sd->packet_ver,RBUFW(buf,0)).len ) { // packet must exist for the client version
			memcpy(WFIFOP(sd->fd,0), buf, len);
			WFIFOSET(sd->fd,len);
		}
		break;

// New definitions for guilds [Valaris]	

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
			size_t i;
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}

			map_session_data::iterator iter(map_session_data::nickdb());
			for(; iter; ++iter)
			{
				sd = iter.data();
				if( sd &&
					sd->guildspy == g->guild_id )
				{
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
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
		if (g)
		{
			size_t i;
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len) { // packet must exist for the client version
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
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if( sd )
		{
			if( packet(sd->packet_ver,RBUFW(buf,0)).len &&
				session_isActive(sd->fd) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(sd->fd,0), buf, len);
				WFIFOSET(sd->fd,len);
			}
		}
	}
}
void clif_send_same_map(const block_list& bl, unsigned char *buf, size_t len)
{
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if( sd && sd->block_list::m == bl.m )
		{
			if( packet(sd->packet_ver,RBUFW(buf,0)).len &&
				session_isActive(sd->fd) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(sd->fd,0), buf, len);
				WFIFOSET(sd->fd,len);
			}
		}
	}
}

void clif_send_self(const block_list& bl, unsigned char *buf, size_t len)
{
	const map_session_data *sd = bl.get_sd();
	if( sd && session_isActive(sd->fd) && session[sd->fd]->user_session && 
		packet(sd->packet_ver,RBUFW(buf,0)).len )
	{
		memcpy(WFIFOP(sd->fd,0), buf, len);
		WFIFOSET(sd->fd,len);
	}
}
void clif_send_chat(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	size_t i;
	const map_session_data *psd = bl.get_sd();
	const chat_data *pcd = (psd) ? psd->chat : bl.get_cd();

	if( pcd )
	{
		for(i=0; i<pcd->users; ++i)
		{
			if( type==CHAT_WOS && pcd->usersd[i]==psd )
				continue;
			if( packet(pcd->usersd[i]->packet_ver,RBUFW(buf,0)).len &&
				session_isActive(pcd->usersd[i]->fd) )
			{	// packet must exist for the client version
				memcpy(WFIFOP(pcd->usersd[i]->fd,0), buf, len);
				WFIFOSET(pcd->usersd[i]->fd,len);
			}
		}
	}
}
void clif_send_area(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	block_list::foreachinarea( CClifSend(buf, len, bl, type),
		bl.m, ((int)bl.x)-AREA_SIZE, ((int)bl.y)-AREA_SIZE, ((int)bl.x)+AREA_SIZE, ((int)bl.y)+AREA_SIZE, BL_PC);
}
void clif_send_chatarea(const block_list& bl, unsigned char *buf, size_t len, int type)
{
	block_list::foreachinarea( CClifSend(buf, len, bl, type),
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len) { // packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
//					if(config.etc_log)
//						ShowMessage("send party %d %d %d\n",p->party_id,i,flag)

				}
			}

			map_session_data::iterator iter(map_session_data::nickdb());
			for(; iter; ++iter)
			{
				sd = iter.data();
				if( sd &&
					sd->partyspy == p->party_id)
				{
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
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
	map_session_data *sd = NULL;
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
			sd = (map_session_data *)bl;
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
			map_session_data::iterator iter(map_session_data::nickdb());
			for(; iter; ++iter)
			{
				sd = iter.data();
				if( sd &&
					sd->guildspy == g->guild_id)
				{
					if (packet(sd->packet_ver,RBUFW(buf,0)).len)
					{	// packet must exist for the client version
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
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
					if (packet(sd->packet_ver,RBUFW(buf,0)).len) { // packet must exist for the client version
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









///////////////////////////////////////////////////////////////////////////////
///
/// activate a effect on the id object for the specified session
///
int clif_seteffect(const map_session_data& sd, uint32 id, uint32 effect)
{
	if( session_isActive(sd.fd) )
	{
		WFIFOW(sd.fd,0) = 0x1f3;
		WFIFOL(sd.fd,2) = id;
		WFIFOL(sd.fd,6) = effect;
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x1f3).len);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///
/// activate a effect on the object for session inside the area
///
int clif_setareaeffect(const block_list &bl, uint32 effect)
{
	unsigned char buf[24];

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = effect;

	return clif_send(buf, packet(0,0x1f3).len, &bl, AREA);
}


///////////////////////////////////////////////////////////////////////////////
///
/// set the weather object for a specific session
///
int clif_setweather(const map_session_data& sd)
{
	if( session_isActive(sd.fd) )
	{
		// get a static id for the weather object
		static uint32 server_weather_id = npc_get_new_npc_id();

		// remove the weather object,
		// weather effects are cleared
		WFIFOW(sd.fd,0) = 0x80;
		WFIFOL(sd.fd,2) = server_weather_id;
		WFIFOB(sd.fd,6) = 0;
		WFIFOSET(sd.fd, packet(sd.packet_ver,0x80).len);

		
		if( !maps[sd.block_list::m].flag.indoors && (
			maps[sd.block_list::m].flag.snow || 
			maps[sd.block_list::m].flag.clouds || 
			maps[sd.block_list::m].flag.clouds2 || 
			maps[sd.block_list::m].flag.fog || 
			maps[sd.block_list::m].flag.fireworks ||
			maps[sd.block_list::m].flag.sakura || 
			maps[sd.block_list::m].flag.leaves || 
			maps[sd.block_list::m].flag.rain ) )
		{
			// spawn the weather object, when there is weather
			WFIFOW(sd.fd,0)=0x7c;
			WFIFOL(sd.fd,2)=server_weather_id;
			WFIFOW(sd.fd,6)=0;
			WFIFOW(sd.fd,8)=0;
			WFIFOW(sd.fd,10)=0;
			WFIFOW(sd.fd,12)=OPTION_HIDE;
			WFIFOW(sd.fd,20)=100;
			WFIFOPOS(sd.fd,36,0,0,0);
			WFIFOSET(sd.fd,packet(sd.packet_ver,0x7c).len);

			// and add the effects
			if (maps[sd.block_list::m].flag.snow)
				clif_seteffect(sd, server_weather_id, EFFECT_SNOW);

			if (maps[sd.block_list::m].flag.clouds)
				clif_seteffect(sd, server_weather_id, EFFECT_CLOUDS);

			if (maps[sd.block_list::m].flag.clouds2)
				clif_seteffect(sd, server_weather_id, EFFECT_CLOUDS2);

			if (maps[sd.block_list::m].flag.fog)
				clif_seteffect(sd, server_weather_id, EFFECT_FOG);

			if (maps[sd.block_list::m].flag.fireworks)
			{
				clif_seteffect(sd, server_weather_id, EFFECT_FIRE1);
				clif_seteffect(sd, server_weather_id, EFFECT_FIRE2);
				clif_seteffect(sd, server_weather_id, EFFECT_FIRE3);
			}
			if (maps[sd.block_list::m].flag.sakura)
				clif_seteffect(sd, server_weather_id, EFFECT_SAKURA);

			if (maps[sd.block_list::m].flag.leaves)
				clif_seteffect(sd, server_weather_id, EFFECT_LEAVES);

			if (maps[sd.block_list::m].flag.rain)
				clif_seteffect(sd, server_weather_id, EFFECT_RAIN);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///
/// updated the weather object for a specific map
///
int clif_updateweather(unsigned short m)
{
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		const map_session_data *sd = iter.data();
		if(sd && sd->block_list::m == m)
		{
			clif_setweather(*sd);
		}
	}
	return 0;
}



















//
// パケット作って送信
//
/*==========================================
 *
 *------------------------------------------
 */
int clif_authok(map_session_data &sd)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) = 0x73;
	WFIFOL(fd, 2) = gettick();
	WFIFOPOS(fd, 6, sd.block_list::x, sd.block_list::y, sd.dir);
	WFIFOB(fd, 9) = 5;
	WFIFOB(fd,10) = 5;
	WFIFOSET(fd,packet(sd.packet_ver,0x73).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse(int fd);
int clif_authfail(map_session_data &sd, uint32 type)
{
	int fd = sd.fd;
	if( !session_isActive(fd) || session[fd]->func_parse!=clif_parse)
		return 0;

	WFIFOW(fd,0) = 0x81;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd,packet(sd.packet_ver,0x81).len);

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
	map_session_data *sd=map_session_data::from_blid(id);
	int fd;

	if( sd == NULL)
		return 1;

	fd = sd->fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xb3;
	WFIFOB(fd,2) = 1;
	WFIFOSET(fd,packet(sd->packet_ver,0xb3).len);

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

	return clif_send(buf, packet(0,0x9e).len, &fitem, AREA);
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

	if (fd == 0)
	{
		clif_send(buf, packet(0,0xa1).len, &fitem, AREA);
	}
	else if( session_isActive(fd) )
	{
		memcpy(WFIFOP(fd,0), buf, 6);
		WFIFOSET(fd,packet(0,0xa1).len);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchar(block_list &bl, unsigned char type)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl.id;
	WBUFB(buf,6) = type;
	clif_send(buf, packet(0,0x80).len, &bl, (type&1) ? AREA : AREA_WOS);


	const map_session_data *sd = bl.get_sd();
	if( sd && sd->disguise_id )
	{
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		clif_send(buf, packet(0,0x80).len, &bl, AREA);
	}

	return 0;
}
int clif_clearchar(map_session_data &sd, block_list &bl)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl.id;
	WBUFB(buf,6) = 0;
	clif_send_self(sd,buf, packet(sd.packet_ver,0x80).len);

	const map_session_data *tsd = bl.get_sd();
	if( tsd && tsd->disguise_id )
	{
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		clif_send_self(sd,buf, packet(sd.packet_ver,0x80).len);
	}

	return 0;
}


int clif_clearchar_delay_sub(int tid, unsigned long tick, int id, basics::numptr data) 
{
	block_list *bl = (block_list *)data.ptr;
	if(bl)
	{
		clif_clearchar(*bl,id);
		bl->freeblock();
		// clear the pointer transfering timer
		get_timer(tid)->data=0;
	}

	return 0;
}

int clif_clearchar_delay(unsigned long tick, block_list &bl, int type) 
{
	block_list *tmpbl = new block_list(bl);
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
	WFIFOSET(sd.fd, packet(sd.packet_ver,0x80).len);
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
		memset(buf, 0, packet(sd.packet_ver,0x78).len);
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
		WBUFW(buf,52) = ((level = sd.get_lv()) > config.max_base_level) ? config.max_base_level : level;

		return packet(sd.packet_ver,0x78).len;
	}

#if PACKETVER < 4
	memset(buf, 0, packet(sd.packet_ver,0x78).len);
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

	return packet(sd.packet_ver,0x78).len;
#else
	memset(buf, 0, packet(sd.packet_ver,0x1d8).len);
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

	return packet(sd.packet_ver,0x1d8).len;
#endif
}

// non-moving function for disguises [Valaris]
size_t clif_dis0078(map_session_data &sd, unsigned char *buf)
{
	memset(buf,0,packet(sd.packet_ver,0x78).len);
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

	return (packet(sd.packet_ver,0x78).len>0)?packet(sd.packet_ver,0x78).len:0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set007b(const map_session_data &sd,unsigned char *buf)
{

#if PACKETVER < 4
	memset(buf, 0, packet(sd.packet_ver,0x7b).len);
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
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.target.x,sd.target.y,8,8);
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd.status.base_level>config.max_lv)?config.max_lv:sd.status.base_level;

	return packet(sd.packet_ver,0x7b).len;
#else
	memset(buf, 0, packet(sd.packet_ver,0x1da).len);
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
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y,8,8);
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd.status.base_level>config.max_base_level)?config.max_base_level:sd.status.base_level;

	return packet(sd.packet_ver,0x1da).len;
#endif
}

// moving function for disguises [Valaris]
int clif_dis007b(const map_session_data &sd,unsigned char *buf)
{
	memset(buf,0,packet(sd.packet_ver,0x7b).len);
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
	WBUFPOS2(buf,50,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y,8,8);
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=0;

	return packet(sd.packet_ver,0x7b).len;
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
		clif_send(buf,packet(0,0x1b0).len,&bl,AREA);
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

		clif_send(buf,packet(0,0x1b0).len,&md,AREA);
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

	clif_send(buf,packet(0,0x1a4).len,&md,AREA);

	return 0;
}

/*==========================================
 * MOB表示1
 *------------------------------------------
 */
int clif_mob0078(const mob_data &md, unsigned char *buf)
{
	unsigned short level=md.get_lv();
	int id = md.get_viewclass();

	if( id <= 23 || id >= 4001)
	{	// Use 0x1d8 packet for monsters with player sprites [Valaris]
		memset(buf,0,packet(0,0x1d8).len);

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

		return packet(0,0x1d8).len;
	}
	else
	{	// Use 0x78 packet for monsters sprites [Valaris]
		memset(buf,0,packet(0,0x78).len);

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
		level = md.get_lv();
		WBUFW(buf,52)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x78).len;
	}
}



/*==========================================
 * MOB表示2
 *------------------------------------------
 */
int clif_mob007b(const mob_data &md, unsigned char *buf)
{
	unsigned short level = md.get_lv();
	int id = md.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1da packet for monsters with player sprites [Valaris]
		memset(buf,0,packet(0,0x1da).len);

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
		WBUFPOS2(buf,50,md.block_list::x,md.block_list::y,md.walktarget.x,md.walktarget.y,8,8);
		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x1da).len;
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet(0,0x7b).len);
	
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

		WBUFPOS2(buf,50,md.block_list::x,md.block_list::y,md.walktarget.x,md.walktarget.y,8,8);
		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		level = md.get_lv();
		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x7b).len;
	}
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_npc0078(const npc_data &nd, unsigned char *buf)
{
	

	memset(buf,0,packet(0,0x78).len);

	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,14)=nd.class_;

	const npcscript_data *sc;
	const guild *g;
	if( (nd.class_ == 722) && 
		(sc = nd.get_script()) && 
		sc->guild_id && 
		(g=guild_search(sc->guild_id)) )
	{
		WBUFL(buf,34)=g->emblem_id;
		WBUFL(buf,38)=g->guild_id;
	}
	WBUFPOS(buf,46,nd.block_list::x,nd.block_list::y,nd.get_dir());
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;

	return packet(0,0x78).len;
}

// NPC Walking [Valaris]
int clif_npc007b(const npc_data &nd, unsigned char *buf)
{
	memset(buf,0,packet(0,0x7b).len);

	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,14)=nd.class_;

	const npcscript_data *sc;
	const guild *g;
	if( (nd.class_ == 722) && 
		(sc = nd.get_script()) && 
		sc->guild_id && 
		(g=guild_search(sc->guild_id)) )
	{
		WBUFL(buf,38)=g->emblem_id;
		WBUFL(buf,42)=g->guild_id;
	}

	WBUFL(buf,22)=gettick();
	WBUFPOS2(buf,50,nd.block_list::x,nd.block_list::y,nd.walktarget.x,nd.walktarget.y,8,8);
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;

	return packet(0,0x7b).len;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pet0078(const pet_data &pd, unsigned char *buf)
{
	int view;
	unsigned short level = pd.get_lv();
	int id = pd.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1d8 packet for pets with player sprites [Valaris]
		memset(buf,0,packet(0,0x1d8).len);

		WBUFW(buf,0)=0x1d8;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,8)=0; // opt1
		WBUFW(buf,10)=0; // opt2
		WBUFW(buf,12)=mob_db[pd.pet.class_].option;
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
		level = pd.get_lv();
		WBUFW(buf,52)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x1d8).len;
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet(0,0x78).len);

		WBUFW(buf,0)=0x78;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,14)=pd.get_viewclass();
		WBUFW(buf,16)=config.pet_hair_style;
		if((view = itemdb_viewid(pd.pet.equip_id)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd.pet.equip_id;

		WBUFPOS(buf,46,pd.block_list::x,pd.block_list::y,pd.dir);
		WBUFB(buf,49)=0;
		WBUFB(buf,50)=0;
		level = pd.get_lv();
		WBUFW(buf,52)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x78).len;
	}
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_pet007b(const pet_data &pd, unsigned char *buf)
{
	int view;
	unsigned short level = pd.get_lv();
	int id = pd.get_viewclass();

	if(id <= 23 || id >= 4001)
	{	// Use 0x1da packet for monsters with player sprites [Valaris]
		memset(buf,0,packet(0,0x1da).len);

		WBUFW(buf,0)=0x1da;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,8)=0; // opt1
		WBUFW(buf,10)=0; // opt2
		WBUFW(buf,12)=mob_db[pd.pet.class_].option;
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
		WBUFPOS2(buf,50,pd.block_list::x,pd.block_list::y,pd.walktarget.x,pd.walktarget.y,8,8);
		WBUFB(buf,56)=0;
		WBUFB(buf,57)=0;
		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x1da).len;
	}
	else
	{	// Use 0x7b packet for monsters sprites [Valaris]
		memset(buf,0,packet(0,0x7b).len);

		WBUFW(buf,0)=0x7b;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.speed;
		WBUFW(buf,14)=pd.get_viewclass();
		WBUFW(buf,16)=config.pet_hair_style;
		if ((view = itemdb_viewid(pd.pet.equip_id)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd.pet.equip_id;
		WBUFL(buf,22)=gettick();

		WBUFPOS2(buf,50,pd.block_list::x,pd.block_list::y,pd.walktarget.x,pd.walktarget.y,8,8);
		WBUFB(buf,56)=0;
		WBUFB(buf,57)=0;

		WBUFW(buf,58)=(level>config.max_base_level)? config.max_base_level:level;

		return packet(0,0x7b).len;
	}
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_hom0078(const homun_data &hd, unsigned char *buf)
{
	memset(buf, 0, packet(0,0x78).len);

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
	WBUFW(buf,52)=hd.get_lv();

	return packet(0,0x78).len;
}
int clif_hom007b(const homun_data &hd, unsigned char *buf)
{
	int view,level;

	memset(buf,0,packet(0,0x7b).len);

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

	WBUFPOS2(buf,50,hd.block_list::x,hd.block_list::y,hd.walktarget.x,hd.walktarget.y,8,8);
	WBUFB(buf,56)=0;
	WBUFB(buf,57)=0;
	level = hd.get_lv();
	WBUFW(buf,58)=(level>99)? 99:level;

	return packet(0,0x7b).len;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_set01e1(map_session_data &sd, unsigned char *buf)
{
	WBUFW(buf,0)=0x1e1;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.spiritball;

	return packet(sd.packet_ver,0x1e1).len;
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
	mapname2buffer(WFIFOP(fd,8), 16, maps[m].mapname);
	WFIFOSET(fd,packet(0,0x192).len);

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
int clif_spawnpc(map_session_data &sd)
{
	unsigned char buf[128];

	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	clif_set0078(sd, buf);

#if PACKETVER < 4
	WBUFW(buf, 0) = 0x79;
	WBUFW(buf,51) = (sd.status.base_level > config.max_lv) ? config.max_lv : sd.status.base_level;
	clif_send(buf, packet(sd.packet_ver,0x79).len, &sd, AREA_WOS);
#else
	WBUFW(buf, 0) = 0x1d9;
	WBUFW(buf,51) = (sd.status.base_level > config.max_base_level) ? config.max_base_level : sd.status.base_level;
	clif_send(buf, packet(sd.packet_ver,0x1d9).len, &sd, AREA_WOS);
#endif

	if(sd.disguise_id > 0)
	{
		memset(buf,0,packet(sd.packet_ver,0x7c).len);
		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=sd.block_list::id|FLAG_DISGUISE;
		WBUFW(buf,6)=sd.get_speed();
		WBUFW(buf,12)=sd.status.option;
		WBUFW(buf,20)=sd.disguise_id;
		WBUFPOS(buf,36,sd.block_list::x,sd.block_list::y,sd.dir);
		clif_send(buf,packet(sd.packet_ver,0x7c).len,&sd,AREA);

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

	if ( (sd.is_riding() && sd.skill_check(KN_RIDING)>0) && 
		(sd.status.class_==7 || sd.status.class_==14 || sd.status.class_==4008 || sd.status.class_==4015 || sd.status.class_==4030 || sd.status.class_==4037))
		pc_setriding(sd); // update peco riders for people upgrading athena [Valaris]

	clif_setweather(sd);

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
int clif_spawnnpc(npc_data &nd)
{
	unsigned char buf[64];
	int len;

	if(nd.class_ < 0 || nd.invalid || nd.class_ == INVISIBLE_CLASS)
		return 0;

	memset(buf,0,packet(0,0x7c).len);

	WBUFW(buf,0)=0x7c;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,20)=nd.class_;
	WBUFPOS(buf,36,nd.block_list::x,nd.block_list::y,nd.get_dir());

	clif_send(buf,packet(0,0x7c).len,&nd,AREA);

	len = clif_npc0078(nd,buf);
	clif_send(buf,len,&nd,AREA);

	return 0;
}
/// spawn npc only on given client
int clif_spawnnpc(map_session_data &sd, npc_data &nd)
{
	unsigned char buf[64];
	int len;

	if(nd.class_ < 0 || nd.invalid || nd.class_ == INVISIBLE_CLASS)
		return 0;

	memset(buf,0,packet(sd.packet_ver,0x7c).len);

	WBUFW(buf,0)=0x7c;
	WBUFL(buf,2)=nd.block_list::id;
	WBUFW(buf,6)=nd.get_speed();
	WBUFW(buf,20)=nd.class_;
	WBUFPOS(buf,36,nd.block_list::x,nd.block_list::y,nd.get_dir());

	clif_send_self(sd,buf,packet(sd.packet_ver,0x7c).len);

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
		memset(buf,0,packet(0,0x7c).len);
		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=md.block_list::id;
		WBUFW(buf,6)=md.get_speed();
		WBUFW(buf,8)=md.opt1;
		WBUFW(buf,10)=md.opt2;
		WBUFW(buf,12)=md.option;
		WBUFW(buf,20)=viewclass;
		WBUFPOS(buf,36,md.block_list::x,md.block_list::y,md.dir);
		clif_send(buf,packet(0,0x7c).len,&md,AREA);
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
		memset(buf,0,packet(0,0x7c).len);

		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=pd.block_list::id;
		WBUFW(buf,6)=pd.get_speed();
		WBUFW(buf,20)=pd.get_viewclass();
		WBUFPOS(buf,36,pd.block_list::x,pd.block_list::y,pd.dir);

		clif_send(buf,packet(0,0x7c).len,&pd,AREA);
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

	memset(buf,0,packet(19,0x7c).len);
	
	WBUFW(buf,0) = 0x7c;
	WBUFL(buf,2) = hd.block_list::id;
	WBUFW(buf,6) = hd.speed;
	if( hd.view_class )
		WBUFW(buf,20) = hd.view_class;
	else
		WBUFW(buf,20) = hd.status.class_;
	WBUFB(buf,28)=8;		// 調べた限り固定
	WBUFPOS(buf,36,hd.block_list::x,hd.block_list::y, hd.get_dir());
	clif_send(buf, packet(19,0x7c).len, &hd, AREA);
	
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
int clif_send_homdata(const map_session_data &sd, unsigned short type, uint32 param)
{
	int fd = sd.fd;
	if( !session_isActive(fd) || !sd.hd)
		return 0;

	WFIFOW(fd,0) = 0x230;
	WFIFOW(fd,2) = type;
	WFIFOL(fd,4) = sd.hd->block_list::id;
	WFIFOL(fd,8) = param;
	WFIFOSET(fd, packet(sd.packet_ver,0x230).len);
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

int clif_send_homstatus(const map_session_data &sd, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) || !sd.hd )
		return 0;
	
	WFIFOW(fd,0)=0x22e;
	memcpy(WFIFOP(fd,2), sd.hd->status.name, 24);
	WFIFOB(fd,26) = sd.hd->status.rename_flag*2;	// 名前付けたフラグ 2で変更不可
	WFIFOW(fd,27) = sd.hd->status.base_level;	// Lv
	WFIFOW(fd,29) = sd.hd->status.hungry;		// 満腹度
	WFIFOW(fd,31) = sd.hd->intimate/100;	// 新密度
	WFIFOW(fd,33) = sd.hd->status.equip;			// equip id
	WFIFOW(fd,35) = sd.hd->atk;					// Atk
	WFIFOW(fd,37) = sd.hd->matk;					// MAtk
	WFIFOW(fd,39) = sd.hd->hit;					// Hit
	WFIFOW(fd,41) = sd.hd->critical;				// Cri
	WFIFOW(fd,43) = sd.hd->def;					// Def
	WFIFOW(fd,45) = sd.hd->mdef;					// Mdef
	WFIFOW(fd,47) = sd.hd->flee;					// Flee
	WFIFOW(fd,49) =(flag)?0:status_get_amotion(sd.hd)+200;	// Aspd
	WFIFOW(fd,51) = sd.hd->status.hp;			// HP
	WFIFOW(fd,53) = sd.hd->max_hp;		// MHp
	WFIFOW(fd,55) = sd.hd->status.sp;			// SP
	WFIFOW(fd,57) = sd.hd->max_sp;		// MSP
	WFIFOL(fd,59) = sd.hd->status.base_exp;		// Exp
	WFIFOL(fd,63) = sd.hd->next_baseexp();	// NextExp
	WFIFOW(fd,67) = sd.hd->status.skill_point;	// skill point
	WFIFOW(fd,69) = sd.hd->atackable;			// 攻撃可否フラグ	0:不可/1:許可
	WFIFOSET(fd,packet(sd.packet_ver,0x22e).len);

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
	WFIFOSET(fd,packet(sd.packet_ver,0x22f).len);
	return 0;
}
/*==========================================
 * ホムのスキルリストを送信する
 *------------------------------------------
 */
int clif_homskillinfoblock(const map_session_data &sd)
{
	int fd=sd.fd;
	int i,c,len=4,id,range,skill_lv;
	if( !session_isActive(fd) || !sd.hd)
		return 0;
	
	WFIFOW(fd,0)=0x235;
	for ( i=c=0; i<MAX_HOMSKILL; ++i)
	{
		id=sd.hd->status.skill[i].id;
		if( id && sd.hd->status.skill[i].lv )
		{
			WFIFOW(fd,len  ) = id;
			WFIFOL(fd,len+2) = skill_get_inf(id);
			skill_lv = sd.hd->status.skill[i].lv;
			WFIFOW(fd,len+6) = skill_lv;
			WFIFOW(fd,len+8) = skill_get_sp(id,skill_lv);
			range = skill_get_range(id,skill_lv);
			if(range < 0)
				range = sd.hd->get_range() - (range + 1);
			WFIFOW(fd,len+10)= range;
			memset(WFIFOP(fd,len+12),0,24);
			if(!(skill_get_inf2(id)&INF2_QUEST_SKILL))
				WFIFOB(fd,len+36)= (skill_lv < skill_get_max(id) && sd.hd->status.skill[i].flag ==0 )? 1:0;
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
int clif_homskillup(const map_session_data &sd, unsigned short skill_num)
{
	int range, fd=sd.fd;
	unsigned short skillid = skill_num-HOM_SKILLID;
	if( !session_isActive(fd) || !sd.hd)
		return 0;
	
	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skill_num;
	WFIFOW(fd,4) = sd.hd->status.skill[skillid].lv;
	WFIFOW(fd,6) = skill_get_sp(skill_num, sd.hd->status.skill[skillid].lv);
	range = skill_get_range(skill_num, sd.hd->status.skill[skillid].lv);
	if(range < 0)
		range = sd.hd->get_range() - (range + 1);
	WFIFOW(fd,8) = range;
	WFIFOB(fd,10) = (sd.hd->status.skill[skillid].lv < skill_get_max(sd.hd->status.skill[skillid].id)) ? 1 : 0;
	WFIFOSET(fd,packet(sd.packet_ver,0x10e).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_servertick(map_session_data &sd, unsigned long tick)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x7f;
	WFIFOL(fd,2)=tick;
	WFIFOSET(fd,packet(sd.packet_ver,0x7f).len);

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
	WFIFOPOS2(fd,6,sd.block_list::x,sd.block_list::y,sd.walktarget.x,sd.walktarget.y,8,8);
	WFIFOSET(fd,packet(sd.packet_ver,0x87).len);
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

		clif_walkok(sd);

		len = clif_set007b(sd, buf);
		clif_send(buf, len, &sd, AREA_WOS);

		if(sd.disguise_id)
		{
			len = clif_dis007b(sd, buf);
			clif_send(buf, len, &sd, AREA);
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
int clif_changemap(map_session_data &sd, const char *mapname, unsigned short x, unsigned short y)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
	return 0;

	WFIFOW(fd,0) = 0x91;
	mapname2buffer(WFIFOP(fd,2), 16, mapname);
	WFIFOW(fd,18) = x;
	WFIFOW(fd,20) = y;
	WFIFOSET(fd, packet(sd.packet_ver,0x91).len);

	return 0;
}
// refresh the client's screen, getting rid of any effects
int clif_refresh(map_session_data &sd)
{
	clif_changemap(sd,sd.mapname,sd.block_list::x,sd.block_list::y);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapserver(map_session_data &sd, const char *mapname, unsigned short x, unsigned short y, basics::ipaddress ip, unsigned short port)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x92;
	mapname2buffer(WFIFOP(fd,2), 16, mapname);
	WFIFOW(fd,18) = x;
	WFIFOW(fd,20) = y;
	WFIFOLIP(fd,22) = ip;
	WFIFOW(fd,26) = port;
	WFIFOSET(fd, packet(sd.packet_ver,0x92).len);

	session_SetWaitClose(fd, 1000);
	return 0;
}



/*==========================================
 *
 *------------------------------------------
 */
int clif_npcbuysell(map_session_data& sd, uint32 id)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xc4;
	WFIFOL(fd,2)=id;
	WFIFOSET(fd,packet(sd.packet_ver,0xc4).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_buylist(map_session_data &sd, npc_data &nd)
{
	const npcshop_data *sh = nd.get_shop();
	if( sh && session_isActive(sd.fd) )
	{
		const item_data *id;
		int i,c,val;
		int fd=sd.fd;

		WFIFOW(fd,0)=0xc6;
		for(i=0, c=0; i<sh->shop_item_cnt; ++i)
		{
			if( sh->shop_item_lst[i].nameid )
			{
				id = itemdb_search(sh->shop_item_lst[i].nameid);
				val= sh->shop_item_lst[i].price;

				WFIFOL(fd,4+c*11)=val;
				if (!id->flag.value_notdc)
					val=pc_modifybuyvalue(sd,val);
				WFIFOL(fd,8+c*11)=val;
				WFIFOB(fd,12+c*11)=id->getType();
				if (id->view_id > 0)
					WFIFOW(fd,13+c*11)=id->view_id;
				else
					WFIFOW(fd,13+c*11)=sh->shop_item_lst[i].nameid;
				++c;
			}
		}
		WFIFOW(fd,2)=c*11+4;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_selllist(map_session_data &sd)
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
int clif_scriptmes(map_session_data &sd, uint32 npcid, const char *mes)
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
int clif_scriptnext(map_session_data &sd,uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xb5;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet(sd.packet_ver,0xb5).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptclose(map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xb6;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet(sd.packet_ver,0xb6).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptmenu(map_session_data &sd, uint32 npcid, const char *mes)
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
int clif_scriptinput(map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x142;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet(sd.packet_ver,0x142).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptinputstr(map_session_data &sd, uint32 npcid)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1d4;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet(sd.packet_ver,0x1d4).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_viewpoint(map_session_data &sd, uint32 npc_id, uint32 id, uint32 x, uint32 y, unsigned char type, uint32 color)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x144).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_cutin(map_session_data &sd, const char *image, unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(image)
	{	
		WFIFOW(fd,0)=0x1b3;
		safestrcpy((char*)WFIFOP(fd,2),64,image);
		WFIFOB(fd,66)=type;
		WFIFOSET(fd,packet(sd.packet_ver,0x1b3).len);
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
int clif_additem(map_session_data &sd, unsigned short n, unsigned short amount, unsigned char fail)
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

	WFIFOSET(fd,packet(sd.packet_ver,0xa0).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_delitem(map_session_data &sd,unsigned short n,unsigned short amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xaf;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=amount;

	WFIFOSET(fd,packet(sd.packet_ver,0xaf).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_itemlist(map_session_data &sd)
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
int clif_equiplist(map_session_data &sd)
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
int clif_storageitemlist(map_session_data &sd,struct pc_storage &stor)
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
int clif_storageequiplist(map_session_data &sd,struct pc_storage &stor)
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
int clif_guildstorageitemlist(map_session_data &sd,struct guild_storage &stor)
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
int clif_guildstorageequiplist(map_session_data &sd,struct guild_storage &stor)
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
int clif_guild_xy(map_session_data &sd)
{
	unsigned char buf[10];
	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=sd.block_list::x;
	WBUFW(buf,8)=sd.block_list::y;
	clif_send(buf,packet(sd.packet_ver,0x1eb).len,&sd,GUILD_SAMEMAP_WOS);
	return 0;
}
int clif_guild_xy_remove(map_session_data &sd)
{
	unsigned char buf[10];
	WBUFW(buf,0)=0x1eb;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=0xFFFF;
	WBUFW(buf,8)=0xFFFF;
	clif_send(buf,packet(sd.packet_ver,0x1eb).len,&sd,GUILD_SAMEMAP_WOS);
	return 0;
}

/*==========================================
 * ステータスを送りつける
 * 表示専用数字はこの中で計算して送る
 *------------------------------------------
 */
int clif_updatestatus(map_session_data &sd,unsigned short type)
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
int clif_changestatus(block_list &bl, unsigned short type, uint32 val)
{
	unsigned char buf[12];
	if( bl == BL_PC )
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
		clif_send(buf,packet(0,0x1ab).len,&bl,AREA_WOS);
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
		map_session_data &sd = (map_session_data &)bl;
		if( (type == LOOK_WEAPON || type == LOOK_SHIELD) && sd.view_class == 22 )
			val =0;
		WBUFW(buf,0)=0xc3;
		WBUFL(buf,2)=bl.id;
		WBUFB(buf,6)=type;
		WBUFB(buf,7)=val;
		return clif_send(buf,packet(sd.packet_ver,0xc3).len,&bl,AREA);
	}
	return 0;

#else

	if(bl == BL_PC)
	{
		map_session_data &sd = (map_session_data &)bl;

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
			return clif_send(buf,packet(sd.packet_ver,0x1d7).len,&bl,AREA);
		}
		else if( (type == LOOK_BASE) && (val > 255) )
		{
			WBUFW(buf,0)=0x1d7;
			WBUFL(buf,2)=bl.id;
			WBUFB(buf,6)=type;
			WBUFW(buf,7)=val;
			WBUFW(buf,9)=0;
			return clif_send(buf,packet(sd.packet_ver,0x1d7).len,&bl,AREA);
		}

	}
	WBUFW(buf,0)=0xc3;
	WBUFL(buf,2)=bl.id;
	WBUFB(buf,6)=type;
	WBUFB(buf,7)=val;
	return clif_send(buf,packet(0,0xc3).len,&bl,AREA);

#endif

}

/*==========================================
 *
 *------------------------------------------
 */
int clif_initialstatus(map_session_data &sd)
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

	WFIFOSET(fd,packet(sd.packet_ver,0xbd).len);

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
int clif_arrowequip(map_session_data &sd,unsigned short val)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if(sd.target_id)
		sd.target_id = 0;

	WFIFOW(fd,0)=0x013c;
	WFIFOW(fd,2)=val+2;//矢のアイテムID

	WFIFOSET(fd,packet(sd.packet_ver,0x013c).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_arrow_fail(map_session_data &sd,unsigned short type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x013b;
	WFIFOW(fd,2)=type;

	WFIFOSET(fd,packet(sd.packet_ver,0x013b).len);

	return 0;
}

/*==========================================
 * 作成可能 矢リスト送信
 *------------------------------------------
 */
int clif_arrow_create_list(map_session_data &sd)
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
int clif_statusupack(map_session_data &sd,unsigned short type,unsigned char ok,unsigned char val)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xbc;
	WFIFOW(fd,2)=type;
	WFIFOB(fd,4)=ok;
	WFIFOB(fd,5)=val;
	WFIFOSET(fd,packet(sd.packet_ver,0xbc).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_equipitemack(map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xaa;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet(sd.packet_ver,0xaa).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_unequipitemack(map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xac;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet(sd.packet_ver,0xac).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_misceffect(block_list &bl, uint32 type)
{
	unsigned char buf[32];

	WBUFW(buf,0) = 0x19b;
	WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = type;

	return clif_send(buf,packet(0,0x19b).len,&bl,AREA);
}


/*==========================================
 * 表示オプション変更
 *------------------------------------------
 */
int clif_changeoption(block_list &bl)
{
	static const int omask[]={ 0x10,0x20 };
	static const status_t scnum[]={ SC_FALCON, SC_RIDING };
	unsigned char buf[32];
	short option;
	size_t i;
	option = *status_get_option(&bl);


	WBUFW(buf,0) = 0x119;
	if(bl==BL_PC && ((map_session_data &)bl).disguise_id)
	{
		WBUFL(buf,2) = bl.id;
		WBUFW(buf,6) = 0;
		WBUFW(buf,8) = 0;
		WBUFW(buf,10) = OPTION_HIDE;
		WBUFB(buf,12) = 0;
		clif_send(buf,packet(0,0x119).len,&bl,AREA);
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
		WBUFW(buf,6) = 0;
		WBUFW(buf,8) = 0;
		WBUFW(buf,10) = option;
		WBUFB(buf,12) = 0;
		clif_send(buf,packet(0,0x119).len,&bl,AREA);
	}
	else
	{
		WBUFL(buf,2) = bl.id;
		WBUFW(buf,6) = *status_get_opt1(&bl);
		WBUFW(buf,8) = *status_get_opt2(&bl);
		WBUFW(buf,10) = option;
		WBUFB(buf,12) = 0;	// ??
		clif_send(buf,packet(0,0x119).len,&bl,AREA);
	}

	// アイコンの表示
	for(i=0;i<sizeof(omask)/sizeof(omask[0]); ++i)
	{
		if( option&omask[i] )
		{
			if( !bl.has_status(scnum[i]) )
				status_change_start(&bl,scnum[i],0,0,0,0,0,0);
		}
		else
		{
			status_change_end(&bl,scnum[i],-1);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_useitemack(map_session_data &sd,unsigned short index,unsigned short amount,unsigned char ok)
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
		WFIFOSET(fd,packet(sd.packet_ver,0xa8).len);
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
		WFIFOSET(fd,packet(sd.packet_ver,0xa8).len);
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
		clif_send(buf,packet(sd.packet_ver,0x1c8).len,&sd,AREA);
#endif
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_createchat(map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd6;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0xd6).len);

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
		memcpy(WFIFOP(fd,0),buf,packet(0,0xd8).len);
		WFIFOSET(fd,packet(0,0xd8).len);
	}
	else
	{
		clif_send(buf,packet(0,0xd8).len,cd.owner,AREA_WOSC);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_joinchatfail(map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xda;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0xda).len);

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
		WFIFOL(fd, 8+i*28) = (i!=0) || (cd.owner && *cd.owner == BL_NPC);
		memcpy(WFIFOP(fd, 8+i*28+4), cd.usersd[i]->status.name, 24);
	}
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_addchat(chat_data &cd,map_session_data &sd)
{
	unsigned char buf[32];
	WBUFW(buf, 0) = 0x0dc;
	WBUFW(buf, 2) = cd.users;
	memcpy(WBUFP(buf, 4),sd.status.name,24);
	return clif_send(buf,packet(sd.packet_ver,0xdc).len,&sd,CHAT_WOS);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changechatowner(chat_data &cd,map_session_data &sd)
{
	unsigned char buf[64];

	WBUFW(buf, 0) = 0xe1;
	WBUFL(buf, 2) = 1;
	if(cd.usersd[0]) memcpy(WBUFP(buf,6), cd.usersd[0]->status.name,24);
	WBUFW(buf,30) = 0xe1;
	WBUFL(buf,32) = 0;
	memcpy(WBUFP(buf,36),sd.status.name,24);

	return clif_send(buf,packet(sd.packet_ver,0xe1).len*2,&sd,CHAT);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_leavechat(chat_data &cd,map_session_data &sd)
{
	unsigned char buf[32];

	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = cd.users-1;
	memcpy(WBUFP(buf,4),sd.status.name,24);
	WBUFB(buf,28) = 0;

	return clif_send(buf,packet(sd.packet_ver,0xdd).len,&sd,CHAT);
}

/*==========================================
 * 取り引き要請受け
 *------------------------------------------
 */
int clif_traderequest(map_session_data &sd,const char *name)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xe5;

	strcpy((char*)WFIFOP(fd,2),name);
	
	WFIFOSET(fd,packet(sd.packet_ver,0xe5).len);

	return 0;
}

/*==========================================
 * 取り引き要求応答
 *------------------------------------------
 */
int clif_tradestart(map_session_data &sd,unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xe7;
	WFIFOB(fd,2)=type;
	WFIFOSET(fd,packet(sd.packet_ver,0xe7).len);

	return 0;
}

/*==========================================
 * 相手方からのアイテム追加
 *------------------------------------------
 */
int clif_tradeadditem(map_session_data &sd, map_session_data &tsd,unsigned short index, uint32 amount)
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
	WFIFOSET(fd,packet(sd.packet_ver,0xe9).len);

	return 0;
}

/*==========================================
 * アイテム追加成功/失敗
 *------------------------------------------
 */
int clif_tradeitemok(map_session_data &sd,unsigned short index,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xea;
	WFIFOW(fd,2)=index;
	WFIFOB(fd,4)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0xea).len);

	return 0;
}

/*==========================================
 * 取り引きok押し
 *------------------------------------------
 */
int clif_tradedeal_lock(map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xec;
	WFIFOB(fd,2)=fail; // 0=you 1=the other person
	WFIFOSET(fd,packet(sd.packet_ver,0xec).len);

	return 0;
}

/*==========================================
 * 取り引きがキャンセルされました
 *------------------------------------------
 */
int clif_tradecancelled(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xee;
	WFIFOSET(fd,packet(sd.packet_ver,0xee).len);

	return 0;
}

/*==========================================
 * 取り引き完了
 *------------------------------------------
 */
int clif_tradecompleted(map_session_data &sd,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf0;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0xf0).len);

	return 0;
}

/*==========================================
 * カプラ倉庫のアイテム数を更新
 *------------------------------------------
 */
int clif_updatestorageamount(map_session_data &sd,struct pc_storage &stor)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor.storage_amount;  //items
	WFIFOW(fd,4) = MAX_STORAGE; //items max
	WFIFOSET(fd,packet(sd.packet_ver,0xf2).len);

	return 0;
}

/*==========================================
 * カプラ倉庫にアイテムを追加する
 *------------------------------------------
 */
int clif_storageitemadded(map_session_data &sd,struct pc_storage &stor,unsigned short index,uint32 amount)
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

	WFIFOSET(fd,packet(sd.packet_ver,0xf4).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_updateguildstorageamount(map_session_data &sd,struct guild_storage &stor)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor.storage_amount;  //items
	WFIFOW(fd,4) = MAX_GUILD_STORAGE; //items max
	WFIFOSET(fd,packet(sd.packet_ver,0xf2).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageitemadded(map_session_data &sd,struct guild_storage &stor,unsigned short index,uint32 amount)
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

	WFIFOSET(fd,packet(sd.packet_ver,0xf4).len);

	return 0;
}

/*==========================================
 * カプラ倉庫からアイテムを取り去る
 *------------------------------------------
 */
int clif_storageitemremoved(map_session_data &sd,unsigned short index,uint32 amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf6; // Storage item removed
	WFIFOW(fd,2)=index+1;
	WFIFOL(fd,4)=amount;
	WFIFOSET(fd,packet(sd.packet_ver,0xf6).len);

	return 0;
}

/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
int clif_storageclose(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xf8; // Storage Closed
	WFIFOSET(fd,packet(sd.packet_ver,0xf8).len);

	return 0;
}

//
// callback系 ?
//
/*==========================================
 * PC表示
 *------------------------------------------
 */
int clif_getareachar_pc(map_session_data &sd, map_session_data &dstsd)
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


	if(dstsd.chat)
	{
		chat_data *cd = dstsd.chat;
		if(cd->usersd[0]==&dstsd)
			clif_dispchat(*cd,sd.fd);
	}
	if(dstsd.vender_id){
		clif_showvendingboard(dstsd, dstsd.message, sd.fd);
	}
	if(dstsd.spiritball > 0) {
		clif_set01e1(dstsd,WFIFOP(sd.fd,0));
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x1e1).len);
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
int clif_getareachar_npc(map_session_data &sd,npc_data &nd)
{
	int len;
	if( !session_isActive(sd.fd) )
		return 0;

	if(nd.class_ < 0 || nd.invalid || nd.class_ == INVISIBLE_CLASS)
		return 0;
	if(nd.is_walking() )
		len = clif_npc007b(nd,WFIFOP(sd.fd,0));
	else
		len = clif_npc0078(nd,WFIFOP(sd.fd,0));
	WFIFOSET(sd.fd,len);

	npcscript_data *sc = nd.get_script();
	if(sc && sc->chat)
	{
		clif_dispchat(*sc->chat, sd.fd);
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

	clif_send(buf, packet(0,0x88).len, &bl, AREA);

	const map_session_data *sd=bl.get_sd();
	if(sd && sd->disguise_id)
	{	// reusing the buffer
		WBUFL(buf,2)=bl.id|FLAG_DISGUISE;
		//WBUFW(buf,6)=bl.x;
		//WBUFW(buf,8)=bl.y;
		clif_send(buf, packet(0,0x88).len, &bl, AREA);
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
int clif_damage(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,unsigned short damage,unsigned short div,unsigned char type,unsigned short damage2)
{
	unsigned char buf[256];

	if(type != 4 && dst==BL_PC && ((map_session_data &)dst).state.infinite_endure)
		type = 9;

	if( type != 4 && dst.has_status(SC_ENDURE) && 
		dst==BL_PC && !maps[dst.m].flag.gvg )
		type = 9;
	if( dst.has_status(SC_HALLUCINATION) )
	{
		if(damage > 0)
			damage = damage*(5+dst.get_statusvalue1(SC_HALLUCINATION).integer()) + rand()%100;
		if(damage2 > 0)
			damage2 = damage2*(5+dst.get_statusvalue1(SC_HALLUCINATION).integer()) + rand()%100;
	}
	
	WBUFW(buf,0)=0x8a;
	if(src==BL_PC && ((map_session_data &)src).disguise_id)
		WBUFL(buf,2)=src.id|FLAG_DISGUISE;
	else 
		WBUFL(buf,2)=src.id;
	if(dst==BL_PC && ((map_session_data &)dst).disguise_id)
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
	clif_send(buf,packet(0,0x8a).len,&src,AREA);

	if( (src==BL_PC && ((map_session_data &)src).disguise_id) || 
		(dst==BL_PC && ((map_session_data &)dst).disguise_id) )
	{
		WBUFW(buf,0)=0x8a;
		if(src==BL_PC && ((map_session_data &)src).disguise_id)
			WBUFL(buf,2)=src.id|FLAG_DISGUISE;
		else 
			WBUFL(buf,2)=src.id;
		if(dst==BL_PC && ((map_session_data &)dst).disguise_id)
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
		clif_send(buf,packet(0,0x8a).len,&src,AREA);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar_mob(map_session_data &sd,struct mob_data &md)
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
int clif_getareachar_pet(map_session_data &sd,struct pet_data &pd)
{
	size_t len;

	if( !session_isActive(sd.fd) )
		return 0;

	len = pd.is_walking() ? clif_pet007b(pd,WFIFOP(sd.fd,0)) :  clif_pet0078(pd,WFIFOP(sd.fd,0));
	WFIFOSET(sd.fd,len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_getareachar(map_session_data &sd, struct homun_data &hd)
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
int clif_getareachar_item(map_session_data &sd, flooritem_data &fitem)
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

	WFIFOSET(fd,packet(sd.packet_ver,0x9d).len);
	return 0;
}
/*==========================================
 * 場所スキルエフェクトが視界に入る
 *------------------------------------------
 */
int clif_getareachar_skillunit(map_session_data &sd,struct skill_unit &unit)
{

	block_list *bl;
	int fd=sd.fd;

	if( !session_isActive(fd) || !unit.group )
			return 0;

	bl=block_list::from_blid(unit.group->src_id);
#if PACKETVER < 3
	memset(WFIFOP(fd,0),0,packet(sd.packet_ver,0x11f).len);
	WFIFOW(fd, 0)=0x11f;
	WFIFOL(fd, 2)=unit.block_list::id;
	WFIFOL(fd, 6)=unit.group->src_id;
	WFIFOW(fd,10)=unit.block_list::x;
	WFIFOW(fd,12)=unit.block_list::y;
	WFIFOB(fd,14)=unit.group->unit_id;
	WFIFOB(fd,15)=0;
	WFIFOSET(fd,packet(sd.packet_ver,0x11f).len);
#else
	memset(WFIFOP(fd,0),0,packet(sd.packet_ver,0x1c9).len);
	WFIFOW(fd, 0)=0x1c9;
	WFIFOL(fd, 2)=unit.block_list::id;
	WFIFOL(fd, 6)=unit.group->src_id;
	WFIFOW(fd,10)=unit.block_list::x;
	WFIFOW(fd,12)=unit.block_list::y;
	WFIFOB(fd,14)=unit.group->unit_id;
	//	Graffiti [Valaris]	
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
	WFIFOSET(fd,packet(sd.packet_ver,0x1c9).len);
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
	WFIFOSET(fd,packet(0,0x120).len);
	if(unit.group && unit.group->skill_id == WZ_ICEWALL)
		clif_set0192(fd,unit.block_list::m,unit.block_list::x,unit.block_list::y,unit.val2);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_01ac(block_list &bl)
{
	unsigned char buf[32];
	WBUFW(buf, 0) = 0x1ac;
	WBUFL(buf, 2) = bl.id;
	return clif_send(buf,packet(0,0x1ac).len,&bl,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
class CClifGetAreaChar : public CMapProcessor
{
	ICL_EMPTY_COPYCONSTRUCTOR(CClifGetAreaChar)
	map_session_data &sd;
public:

	CClifGetAreaChar(map_session_data &s) : sd(s)	{}
	~CClifGetAreaChar()	{}

	virtual int process(block_list& bl) const
	{
		if( session_isActive(sd.fd) )
		{
			if(bl==BL_PC)
			{
				if(sd.block_list::id != bl.id)
					clif_getareachar_pc(sd, ((map_session_data&)bl));
			}
			else if(bl==BL_NPC)
			{
				clif_getareachar_npc(sd, ((npc_data&) bl));
			}
			else if(bl==BL_MOB)
			{
				clif_getareachar_mob(sd,((struct mob_data&) bl));
			}
			else if(bl==BL_PET)
			{
				clif_getareachar_pet(sd, ((struct pet_data&) bl));
			}
			else if(bl==BL_HOM)
			{
				clif_getareachar(sd, ((struct homun_data&) bl));
			}
			else if(bl==BL_ITEM)
			{
				clif_getareachar_item(sd, ((flooritem_data&) bl));
			}
			else if(bl==BL_SKILL)
			{
				clif_getareachar_skillunit(sd, ((struct skill_unit&)bl));
			}
			else
			{
				if(config.error_log)
					ShowMessage("get area char usupportd type\n");
			}
		}
		return 0;
	}
};










int CClifInsight::process(block_list& bl) const
{
	if (bl.id == tbl.id)
		return 0;

	map_session_data *sd = (tsd) ? tsd : bl.get_sd();
	block_list* object   = (tsd) ? &bl : &tbl;

	if(!tsd && object->get_md() )
		object->get_md();

	if( sd && session_isActive(sd->fd) )
	{	//Tell sd that object entered into his view
		if(*object==BL_PC)
		{
			if(sd->block_list::id != object->id)
			{
				clif_getareachar_pc(*sd, *object->get_sd());
				clif_getareachar_pc(*object->get_sd(), *sd);
			}
		}
		else if(*object==BL_MOB)
		{
			clif_getareachar_mob(*sd,*object->get_md());
		}
		else if(*object==BL_NPC)
		{
			clif_getareachar_npc(*sd,*object->get_nd());
		}
		else if(*object==BL_ITEM)
		{
			clif_getareachar_item(*sd,*object->get_fd());
		}
		else if(*object==BL_SKILL)
		{
			clif_getareachar_skillunit(*sd,*object->get_sk());
		}
		else if(*object==BL_PET)
		{
			clif_getareachar_pet(*sd,*object->get_pd());
		}
		else if(*object==BL_HOM)
		{
			clif_getareachar(*sd,*object->get_hd());
		}

	}
	return 0;
}
int CClifOutsight::process(block_list& bl) const
{
	if(bl.id == tbl.id)
		return 0;

	map_session_data *sd = (tsd) ? tsd : bl.get_sd();
	block_list* object   = (tsd) ? &bl : &tbl;

	if( sd && session_isActive(sd->fd) )
	{	//sd has lost sight of object
		if( *object==BL_PC )
		{
			map_session_data *osd = object->get_sd();
			clif_clearchar_id(*osd,  sd->block_list::id, 0);
			clif_clearchar_id( *sd, osd->block_list::id, 0);

			if( osd->disguise_id )
				clif_clearchar_id( *sd, osd->block_list::id|FLAG_DISGUISE, 0);
			if( sd->disguise_id )
				clif_clearchar_id(*osd,  sd->block_list::id|FLAG_DISGUISE, 0);

			if(sd->chat)
			{
				struct chat_data *cd = sd->chat;
				if(cd->usersd[0]==osd)
					clif_dispchat(*cd,osd->fd);
			}
			if(osd->chat)
			{
				struct chat_data *cd= osd->chat;
				if(cd->usersd[0]==sd)
					clif_dispchat(*cd,sd->fd);
			}

			if(sd->vender_id)
				clif_closevendingboard(*sd,tsd->fd);
			if(osd->vender_id)
				clif_closevendingboard(*osd,sd->fd);
		}
		else if( *object==BL_ITEM )
		{
			clif_clearflooritem( *object->get_fd(),sd->fd);
		}
		else if( *object==BL_SKILL )
		{
			clif_clearchar_skillunit( *object->get_sk(),sd->fd);
		}
		else
		{	// default
			clif_clearchar_id(*sd, object->id, 0);
		}
	}

	return 0;
}










/*==========================================
 *
 *------------------------------------------
 */
int clif_skillinfo(map_session_data &sd, unsigned short skillid, short type, short range)
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
			range = sd.get_range() - (range + 1);
	}
	WFIFOW(fd,12)= range;

	memset(WFIFOP(fd,14), 0, 24);
	//safestrcpy((char*)WFIFOP(fd,14), 24, skill_get_name(id));
	inf2 = skill_get_inf2(id);
	if( ((!(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) && !(inf2&INF2_WEDDING_SKILL)) ||
		(config.gm_allskill && sd.isGM() >= config.gm_allskill) )
		//WFIFOB(fd,38)= (sd.status.skill[skillid].lv < skill_get_max(id) && sd.status.skill[skillid].flag ==0 )? 1:0;
		WFIFOB(fd,38)= (sd.status.skill[skillid].lv < skill_tree_get_max(id, sd.status.class_) && sd.status.skill[skillid].flag ==0 )? 1:0;
	else
		WFIFOB(fd,38) = 0;
	WFIFOSET(fd,packet(sd.packet_ver,0x147).len);

	return 0;
}

/*==========================================
 * スキルリストを送信する
 *------------------------------------------
 */
int clif_skillinfoblock(map_session_data &sd)
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
				range = sd.get_range() - (range + 1);
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
int clif_skillup(map_session_data &sd, unsigned short skill_num)
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
		range = sd.get_range() - (range + 1);
	WFIFOW(fd,8) = range;
	//WFIFOB(fd,10) = (sd.status.skill[skill_num].lv < skill_get_max(sd.status.skill[skill_num].id)) ? 1 : 0;
	WFIFOB(fd,10) = (sd.status.skill[skill_num].lv < skill_tree_get_max(sd.status.skill[skill_num].id, sd.status.class_)) ? 1 : 0;
	WFIFOSET(fd,packet(sd.packet_ver,0x10e).len);

	return 0;
}

/*==========================================
 * スキル詠唱エフェクトを送信する
 *------------------------------------------
 */
int clif_skillcasting(block_list &bl,uint32 src_id,uint32 dst_id,unsigned short dst_x,unsigned short dst_y,unsigned short skill_id,uint32 casttime)
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
	return clif_send(buf,packet(0,0x13e).len, &bl, AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_skillcastcancel(block_list &bl)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x1b9;
	WBUFL(buf,2) = bl.id;
	return clif_send(buf,packet(0,0x1b9).len, &bl, AREA);
}



/*==========================================
 * スキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type)
{
	unsigned char buf[64];

	if(type != 5 && dst==BL_PC && ((map_session_data &)dst).state.infinite_endure)
		type = 9;
	if(type != 5 && dst.has_status(SC_ENDURE) )
		type = 9;
	if(dst.has_status(SC_HALLUCINATION) && damage > 0)
		damage = damage*(5+dst.get_statusvalue1(SC_HALLUCINATION).integer()) + rand()%100;


#if PACKETVER < 3
	WBUFW(buf,0)=0x114;
	WBUFW(buf,2)=skill_id;
	if(src->type==BL_PC && ((map_session_data &)src).disguise_id)
		WBUFL(buf,4)=src.id | FLAG_DISGUISE;
	else 
		WBUFL(buf,4)=src.id;
	if(dst->type==BL_PC && ((map_session_data *)dst)->disguise_id)
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
	return clif_send(buf,packet(sd.packet_ver,0x114).len,&src,AREA);
#else
	WBUFW(buf,0)=0x1de;
	WBUFW(buf,2)=skill_id;
	if(src==BL_PC && ((map_session_data &)src).disguise_id)
		WBUFL(buf,4)=src.id|FLAG_DISGUISE;
	else 
		WBUFL(buf,4)=src.id;
	if(dst==BL_PC && ((map_session_data &)dst).disguise_id)
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
	return clif_send(buf,packet(0,0x1de).len,&src,AREA);
#endif
}

/*==========================================
 * 吹き飛ばしスキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage2(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type)
{
	unsigned char buf[64];

	if(type != 5 && dst == BL_PC && ((map_session_data &)dst).state.infinite_endure)
		type = 9;
	if(type != 5 && dst.has_status(SC_ENDURE))
		type = 9;
	if(dst.has_status(SC_HALLUCINATION) && damage > 0)
		damage = damage*(5+dst.get_statusvalue1(SC_HALLUCINATION).integer()) + rand()%100;

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
	return clif_send(buf,packet(0,0x115).len,&src,AREA);
}

/*==========================================
 * 支援/回復スキルエフェクト
 *------------------------------------------
 */
int clif_skill_nodamage(block_list &src,block_list &dst,unsigned short skill_id,unsigned short heal,unsigned char fail)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x11a;
	WBUFW(buf,2)=skill_id;
	WBUFW(buf,4)=(heal > 0x7fff)? 0x7fff:heal;
	WBUFL(buf,6)=dst.id;
	WBUFL(buf,10)=src.id;
	WBUFB(buf,14)=fail;
	return clif_send(buf,packet(0,0x11a).len,&src,AREA);
}

/*==========================================
 * 場所スキルエフェクト
 *------------------------------------------
 */
int clif_skill_poseffect(block_list &src,unsigned short skill_id,unsigned short val,unsigned short x,unsigned short y,unsigned long tick)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x117;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src.id;
	WBUFW(buf,8)=val;
	WBUFW(buf,10)=x;
	WBUFW(buf,12)=y;
	WBUFL(buf,14)=tick;
	return clif_send(buf,packet(0,0x117).len,&src,AREA);
}

/*==========================================
 * 場所スキルエフェクト表示
 *------------------------------------------
 */
int clif_skill_setunit(struct skill_unit &unit)
{
	unsigned char buf[128];
	block_list *bl = (unit.group) ? block_list::from_blid(unit.group->src_id) : NULL;

#if PACKETVER < 3
	memset(WBUFP(buf, 0),0,packet(sd.packet_ver,0x11f).len);
	WBUFW(buf, 0)=0x11f;
	WBUFL(buf, 2)=unit.block_list::id;
	WBUFL(buf, 6)=(unit.group) ? unit.group->src_id : unit.block_list::id;
	WBUFW(buf,10)=unit.block_list::x;
	WBUFW(buf,12)=unit.block_list::y;
	WBUFB(buf,14)=(unit.group) ? unit.group->unit_id : unit.block_list::id;
	WBUFB(buf,15)=0;
	return clif_send(buf,packet(sd.packet_ver,0x11f).len,&unit->bl,AREA);
#else
	memset(WBUFP(buf, 0),0,packet(0,0x1c9).len);
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
	return clif_send(buf,packet(0,0x1c9).len,&unit,AREA);
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
	return clif_send(buf,packet(0,0x120).len,&unit,AREA);
}
/*==========================================
 * ワープ場所選択
 *------------------------------------------
 */
int clif_skill_warppoint(map_session_data &sd,unsigned short skill_id,
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
	WFIFOSET(fd,packet(sd.packet_ver,0x11c).len);
	return 0;
}
/*==========================================
 * メモ応答
 *------------------------------------------
 */
int clif_skill_memo(map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x11e;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x11e).len);
	return 0;
}
int clif_skill_teleportmessage(map_session_data &sd,unsigned short flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
			return 0;

	WFIFOW(fd,0)=0x189;
	WFIFOW(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x189).len);
	return 0;
}

/*==========================================
 * モンスター情報
 *------------------------------------------
 */
int clif_skill_estimation(const map_session_data &sd, const mob_data &md)
{
	unsigned char buf[64];
	int i;

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

		return clif_send(buf,packet(sd.packet_ver,0x18c).len,&sd,PARTY_AREA);
	
	else if( session_isActive(sd.fd) )
	{
		memcpy(WFIFOP(sd.fd,0),buf,packet(sd.packet_ver,0x18c).len);
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x18c).len);
	}
	return 0;
}
/*==========================================
 * アイテム合成可能リスト
 *------------------------------------------
 */
int clif_skill_produce_mix_list(map_session_data &sd,int trigger)
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
int clif_status_change(block_list &bl,unsigned short type,unsigned char flag)
{
	unsigned char buf[16];
	if (type >= MAX_STATUSCHANGE) //Status changes above this value are not displayed on the client. [Skotlex]
		return 0;
	WBUFW(buf,0)=0x0196;
	WBUFW(buf,2)=type;
	WBUFL(buf,4)=bl.id;
	WBUFB(buf,8)=flag;
	return clif_send(buf,packet(0,0x196).len,&bl,AREA);
}

/*==========================================
 * Send message (modified by [Yor])
 *------------------------------------------
 */
int clif_displaymessage(int fd, const char* mes)
{
	if(mes)
	{
		//Console [Wizputer]
		if (fd == 0)
			ShowConsole(CL_BOLD"%s\n"CL_NORM, mes);

		else if(*mes && session_isActive(fd) )
		{	// don't send a void message (it's not displaying on the client chat). @help can send void line.
			size_t len=strlen(mes);
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
int clif_GMmessage(block_list *bl, const char* mes, size_t len, int flag)
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
int clif_GlobalMessage(block_list &bl,const char *message, size_t len)
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
int clif_heal(int fd, unsigned short type, unsigned short val)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x13d;
	WFIFOW(fd,2)=type;
	WFIFOW(fd,4)=val;
	WFIFOSET(fd,packet(0,0x13d).len);

	return 0;
}

/*==========================================
 * 復活する
 *------------------------------------------
 */
int clif_resurrection(block_list &bl,unsigned short type)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x148;
	WBUFL(buf,2)=bl.id;
	WBUFW(buf,6)=type;

	clif_send(buf,packet(0,0x148).len,&bl,type==1 ? AREA : AREA_WOS);

	map_session_data *sd = bl.get_sd();
	if(sd && sd->disguise_id )
		clif_spawnpc( *sd );

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
	WFIFOSET(fd,packet(0,0x199).len);

	return 0;
}

/*==========================================
 * PVP実装？(仮)
 *------------------------------------------
 */
int clif_pvpset(map_session_data &sd,uint32 pvprank,uint32 pvpnum,int type)
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
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x19a).len);
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
			clif_send(buf,packet(sd.packet_ver,0x19a).len,&sd,AREA);
		else
			clif_send_same_map(sd,buf,packet(sd.packet_ver,0x19a).len);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_send0199(unsigned short map,unsigned short type)
{
	block_list bl;
	unsigned char buf[16];

	bl.m = map;
	WBUFW(buf,0)=0x199;
	WBUFW(buf,2)=type;
	clif_send_same_map(bl,buf,packet(0,0x199).len);
	return 0;
}

/*==========================================
 * 精錬エフェクトを送信する
 *------------------------------------------
 */
int clif_refine(int fd, map_session_data &sd,unsigned short fail,unsigned short index,unsigned short val)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x188;
	WFIFOW(fd,2)=fail;
	WFIFOW(fd,4)=index+2;
	WFIFOW(fd,6)=val;
	WFIFOSET(fd,packet(sd.packet_ver,0x188).len);

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
	WFIFOSET(fd,packet(0,0x98).len);
	return 0;
}

/*==========================================
 * キャラID名前引き結果を送信する
 *------------------------------------------
 */
int clif_solved_charname(const map_session_data &sd, uint32 char_id, const char* name)
{
	if( session_isActive(sd.fd) )
	{
		WFIFOW(sd.fd,0)=0x194;
		WFIFOL(sd.fd,2)=char_id;
		memcpy(WFIFOP(sd.fd,6), name, 24);
		WFIFOSET(sd.fd,packet(sd.packet_ver,0x194).len);
	}
	return 0;
}

/*==========================================
 * カードの挿入可能リストを返す
 *------------------------------------------
 */
int clif_use_card(map_session_data &sd,unsigned short idx)
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
int clif_insert_card(map_session_data &sd,unsigned short idx_equip,unsigned short idx_card,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x17d;
	WFIFOW(fd,2)=idx_equip+2;
	WFIFOW(fd,4)=idx_card+2;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x17d).len);
	return 0;
}

/*==========================================
 * 鑑定可能アイテムリスト送信
 *------------------------------------------
 */
int clif_item_identify_list(map_session_data &sd)
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
int clif_item_identified(map_session_data &sd,unsigned short idx,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x179;
	WFIFOW(fd, 2)=idx+2;
	WFIFOB(fd, 4)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x179).len);
	return 0;
}

/*==========================================
 * 修理可能アイテムリスト送信
 *------------------------------------------
 */
int clif_item_repair_list(map_session_data &sd)
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
		sd.skill_failed(sd.skillid);

	return 0;
}

int clif_repaireffect(map_session_data &sd, unsigned short nameid, unsigned char flag)
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
int clif_item_refine_list(map_session_data &sd)
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

	skilllv = sd.skill_check(WS_WEAPONREFINE);

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
int clif_item_skill(map_session_data &sd,unsigned short skillid,unsigned short skilllv,const char *name)
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
		range = sd.get_range() - (range + 1);
	WFIFOW(fd,12)=range;
	safestrcpy((char*)WFIFOP(fd,14),24,name);
	WFIFOB(fd,38)=0;
	WFIFOSET(fd,packet(sd.packet_ver,0x147).len);
	return 0;
}

/*==========================================
 * カートにアイテム追加
 *------------------------------------------
 */
int clif_cart_additem(map_session_data &sd,unsigned short n,uint32 amount,unsigned char fail)
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

	WFIFOSET(fd,packet(sd.packet_ver,0x124).len);
	return 0;
}

/*==========================================
 * カートからアイテム削除
 *------------------------------------------
 */
int clif_cart_delitem(map_session_data &sd,unsigned short n,uint32 amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x125;
	WFIFOW(fd,2)=n+2;
	WFIFOL(fd,4)=amount;

	WFIFOSET(fd,packet(sd.packet_ver,0x125).len);

	return 0;
}

/*==========================================
 * カートのアイテムリスト
 *------------------------------------------
 */
int clif_cart_itemlist(map_session_data &sd)
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
int clif_cart_equiplist(map_session_data &sd)
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
int clif_openvendingreq(map_session_data &sd, unsigned short num)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x12d;
	WFIFOW(fd,2)=num;
	WFIFOSET(fd,packet(sd.packet_ver,0x12d).len);

	return 0;
}

/*==========================================
 * 露店看板表示
 *------------------------------------------
 */
int clif_showvendingboard(block_list &bl,const char *message,int fd)
{
	unsigned char buf[128];

	WBUFW(buf,0)=0x131;
	WBUFL(buf,2)=bl.id;
	memcpy((char*)WBUFP(buf,6),message,80);
	if(fd && session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0),buf,packet(0,0x131).len);
		WFIFOSET(fd,packet(0,0x131).len);
	}else{
		clif_send(buf,packet(0,0x131).len,&bl,AREA_WOS);
	}
	return 0;
}

/*==========================================
 * 露店看板消去
 *------------------------------------------
 */
int clif_closevendingboard(block_list &bl,int fd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x132;
	WBUFL(buf,2)=bl.id;
	if(fd && session_isActive(fd) ) {
		memcpy(WFIFOP(fd,0),buf,packet(0,0x132).len);
		WFIFOSET(fd,packet(0,0x132).len);
	}else{
		clif_send(buf,packet(0,0x132).len,&bl,AREA_WOS);
	}

	return 0;
}
/*==========================================
 * 露店アイテムリスト
 *------------------------------------------
 */
int clif_vendinglist(map_session_data &sd, uint32 id, vending_element vend_list[])
{
	struct item_data *data;
	int i,n,index,fd;
	map_session_data *vsd;
	unsigned char *buf;

	nullpo_retr(0, vsd=map_session_data::from_blid(id));

	fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;
	
	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0x133;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<vsd->vend_num; ++i)
	{
		if(vend_list[i].amount<=0)
			continue;
		WBUFL(buf,8+n*22)=vend_list[i].value;
		WBUFW(buf,12+n*22)=vend_list[i].amount;
		WBUFW(buf,14+n*22)=(index=vend_list[i].index)+2;
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
	if(n > 0)
	{
		WBUFW(buf,2)=8+n*22;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * 露店アイテム購入失敗
 *------------------------------------------
*/
int clif_buyvending(map_session_data &sd,unsigned short index,unsigned short amount,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x135;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOB(fd,6)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0x135).len);

	return 0;
}

/*==========================================
 * 露店開設成功
 *------------------------------------------
*/
int clif_openvending(map_session_data &sd,uint32 id, vending_element vend_list[])
{
	struct item_data *data;
	int i,n,index,fd;
	unsigned char *buf;

	fd=sd.fd;
	if( !session_isActive(fd) || !vend_list)
		return 0;

	buf = WFIFOP(fd,0);

	WBUFW(buf,0)=0x136;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<sd.vend_num; ++i)
	{
		if (sd.vend_num > 2+sd.skill_check(MC_VENDING)) return 0;
		WBUFL(buf,8+n*22)=vend_list[i].value;
		WBUFW(buf,12+n*22)=(index=vend_list[i].index)+2;
		WBUFW(buf,14+n*22)=vend_list[i].amount;
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
int clif_vendingreport(map_session_data &sd,unsigned short index,unsigned short amount)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x137;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOSET(fd,packet(sd.packet_ver,0x137).len);

	return 0;
}

/*==========================================
 * パーティ作成完了
 *------------------------------------------
 */
int clif_party_created(map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xfa;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0xfa).len);
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
	map_session_data *sd=NULL;

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
			mapname2buffer(WBUFP(buf,28+c*46+28),16,m.mapname);
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
int clif_party_invite(map_session_data &sd,map_session_data &tsd)
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
	WFIFOSET(fd,packet(sd.packet_ver,0xfe).len);
	return 0;
}

/*==========================================
 * パーティ勧誘結果
 *------------------------------------------
 */
int clif_party_inviteack(map_session_data &sd,const char *nick,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xfd;
	memcpy(WFIFOP(fd,2),nick,24);
	WFIFOB(fd,26)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0xfd).len);
	return 0;
}

/*==========================================
 * パーティ設定送信
 * flag & 0x001=exp変更ミス
 *        0x010=item変更ミス
 *        0x100=一人にのみ送信
 *------------------------------------------
 */
int clif_party_option(struct party &p,map_session_data *sd, int flag)
{
	unsigned char buf[16];
//	if(config.etc_log)
//		ShowMessage("clif_party_option: %d %d %d\n",p->exp,p->item,flag);
	if(sd==NULL && flag==0){
		int i;
		for(i=0;i<MAX_PARTY; ++i)
			if((sd=map_session_data::from_blid(p.member[i].account_id))!=NULL)
				break;
	}
	WBUFW(buf,0)=0x101;
	WBUFW(buf,2)=((flag&0x01)?2:p.expshare);
	WBUFW(buf,4)=((flag&0x10)?2:p.itemshare);
	if(flag==0)
		clif_send(buf, packet(0,0x101).len, sd, PARTY);
	else if( sd && session_isActive(sd->fd) ) {
		memcpy(WFIFOP(sd->fd,0),buf,packet(0,0x101).len);
		WFIFOSET(sd->fd,packet(0,0x101).len);
	}
	return 0;
}
/*==========================================
 * パーティ脱退（脱退前に呼ぶこと）
 *------------------------------------------
 */
int clif_party_leaved(struct party &p,map_session_data *sd,uint32 account_id,const char *name,unsigned char flag)
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
			clif_send(buf,packet(0,0x105).len, sd,PARTY);
	}
	else if( sd && session_isActive(sd->fd) )
	{
		memcpy(WFIFOP(sd->fd,0),buf,packet(sd->packet_ver,0x105).len);
		WFIFOSET(sd->fd,packet(sd->packet_ver,0x105).len);
	}
	return 0;
}
/*==========================================
 * パーティメッセージ送信
 *------------------------------------------
 */
int clif_party_message(struct party &p,uint32 account_id, const char *mes,size_t len)
{
	map_session_data *sd=NULL;
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
int clif_party_xy(struct party &p,map_session_data &sd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=sd.block_list::x;
	WBUFW(buf,8)=sd.block_list::y;
	return clif_send(buf,packet(sd.packet_ver,0x107).len,&sd,PARTY_SAMEMAP_WOS);
}
/*==========================================
 * Remove dot from minimap 
 *------------------------------------------
*/
int clif_party_xy_remove(map_session_data &sd)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd.status.account_id;
	WBUFW(buf,6)=0xFFFF;
	WBUFW(buf,8)=0xFFFF;
	clif_send(buf, packet(sd.packet_ver,0x107).len, &sd, PARTY_SAMEMAP_WOS);
	return 0;
}


/*==========================================
 * パーティHP通知
 *------------------------------------------
 */
int clif_party_hp(struct party &p,map_session_data &sd)
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
	return clif_send(buf,packet(sd.packet_ver,0x106).len,&sd,PARTY_AREA_WOS);
}
/*==========================================
 * GMへ場所とHP通知
 *------------------------------------------
 */
class CHPDisplay : public CMapProcessor
{
	const map_session_data &sd;
	const unsigned char *buf1;
	const unsigned char *buf2;
public:
	CHPDisplay(const map_session_data &s, const unsigned char *b1, const unsigned char *b2)
		: sd(s),buf1(b1),buf2(b2)
	{}
	~CHPDisplay()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd2 = bl.get_sd();
		if( sd2 &&
			sd2->isGM() >= config.disp_hpmeter &&
			sd2->isGM() >= sd.isGM() &&
			&sd != sd2 && 
			sd2->state.auth)
		{
			memcpy(WFIFOP(sd2->fd,0), buf1, packet(sd.packet_ver,0x107).len);
			WFIFOSET (sd2->fd, packet(sd.packet_ver,0x107).len);

			memcpy (WFIFOP(sd2->fd,0), buf2, packet(sd.packet_ver,0x106).len);
			WFIFOSET (sd2->fd, packet(sd.packet_ver,0x106).len);
		}
		return 0;
	}
};
int clif_hpmeter(map_session_data &sd)
{
	unsigned char buf1[16];
	unsigned char buf2[16];
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

	block_list::foreachinarea(CHPDisplay(sd,buf1,buf2), 
		sd.block_list::m, 
		sd.block_list::x - AREA_SIZE,sd.block_list::y - AREA_SIZE,
		sd.block_list::x + AREA_SIZE,sd.block_list::y + AREA_SIZE,
		BL_PC);

/*
	map_session_data *sd2;
	size_t i;
	int x0, y0, x1, y1;

	// some kind of self written foreach_client
	x0 = sd.block_list::x - AREA_SIZE;
	y0 = sd.block_list::y - AREA_SIZE;
	x1 = sd.block_list::x + AREA_SIZE;
	y1 = sd.block_list::y + AREA_SIZE;

	for (i = 0; i < fd_max; ++i)
	{
		if( session[i] && 
			(sd2 = (map_session_data*)session[i]->user_session) &&  
			sd2->block_list::m == sd.block_list::m &&
			sd2->block_list::x > x0 && sd2->block_list::x < x1 &&
			sd2->block_list::y > y0 && sd2->block_list::y < y1 &&
			sd2->isGM() >= config.disp_hpmeter &&
			sd2->isGM() >= sd.isGM() &&
			&sd != sd2 && 
			sd2->state.auth)
		{
			memcpy(WFIFOP(i,0), buf1, packet(sd.packet_ver,0x107).len);
			WFIFOSET (i, packet(sd.packet_ver,0x107).len);

			memcpy (WFIFOP(i,0), buf2, packet(sd.packet_ver,0x106).len);
			WFIFOSET (i, packet(sd.packet_ver,0x106).len);
		}
	}
*/
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
	snprintf(mobhp, sizeof(mobhp), "hp: %lu/%lu", (unsigned long)md.hp, (unsigned long)mob_db[md.class_].max_hp);

	WBUFW(buf, 0) = 0x195;
	memcpy(WBUFP(buf,30), mobhp, 24);
	WBUFL(buf,54) = 0;
	WBUFL(buf,78) = 0;
	return clif_send(buf,packet(0,0x195).len,&md,AREA);
}
/*==========================================
 * パーティ場所移動（未使用）
 *------------------------------------------
 */
int clif_party_move(struct party &p,map_session_data &sd,unsigned char online)
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
	mapname2buffer(WBUFP(buf,63),16,maps[sd.block_list::m].mapname);
	return clif_send(buf,packet(sd.packet_ver,0x104).len,&sd,PARTY);
}
/*==========================================
 * 攻撃するために移動が必要
 *------------------------------------------
 */
int clif_movetoattack(map_session_data &sd,block_list &bl)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x139).len);
	return 0;
}
/*==========================================
 * 製造エフェクト
 *------------------------------------------
 */
int clif_produceeffect(map_session_data &sd,unsigned short flag,unsigned short nameid)
{
	unsigned short view;
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	// 名前の登録と送信を先にしておく
	map_add_namedb(sd.status.char_id,sd.status.name);
	clif_solved_charname(sd, sd.status.char_id, sd.status.name);

	WFIFOW(fd, 0)=0x18f;
	WFIFOW(fd, 2)=flag;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd, 4)=view;
	else
		WFIFOW(fd, 4)=nameid;
	WFIFOSET(fd,packet(sd.packet_ver,0x18f).len);
	return 0;
}

// pet
int clif_catch_process(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x19e;
	WFIFOSET(fd,packet(sd.packet_ver,0x19e).len);

	return 0;
}

int clif_pet_rulet(map_session_data &sd,unsigned char data)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1a0;
	WFIFOB(fd,2)=data;
	WFIFOSET(fd,packet(sd.packet_ver,0x1a0).len);

	return 0;
}

/*==========================================
 * pet卵リスト作成
 *------------------------------------------
 */
int clif_sendegg(map_session_data &sd)
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

int clif_send_petdata(map_session_data &sd,unsigned char type,uint32 param)
{
	int fd=sd.fd;
	if( !session_isActive(fd) || !sd.pd)
		return 0;

	WFIFOW(fd,0)=0x1a4;
	WFIFOB(fd,2)=type;
	WFIFOL(fd,3)=sd.pd->block_list::id;
	WFIFOL(fd,7)=param;
	WFIFOSET(fd,packet(sd.packet_ver,0x1a4).len);

	return 0;
}

int clif_send_petstatus(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) || !sd.pd )
		return 0;

	WFIFOW(fd,0)=0x1a2;
	memcpy(WFIFOP(fd,2),sd.pd->pet.name,24);
	WFIFOB(fd,26)=(config.pet_rename == 1)? 0:sd.pd->pet.rename_flag;
	WFIFOW(fd,27)=sd.pd->pet.level;
	WFIFOW(fd,29)=sd.pd->pet.hungry;
	WFIFOW(fd,31)=sd.pd->pet.intimate;
	WFIFOW(fd,33)=sd.pd->pet.equip_id;
	WFIFOSET(fd,packet(sd.packet_ver,0x1a2).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pet_emotion(struct pet_data &pd, uint32 param)
{
	unsigned char buf[16];
	memset(buf,0,packet(0,0x1aa).len);

	WBUFW(buf,0)=0x1aa;
	WBUFL(buf,2)=pd.block_list::id;
	if( param >= 100 && pd.petDB.talk_convert_class)
	{
		if(pd.petDB.talk_convert_class < 0)
			return 0;
		else if(pd.petDB.talk_convert_class > 0)
		{
			param -= (pd.pet.class_ - 100)*100;
			param += (pd.petDB.talk_convert_class - 100)*100;
		}
	}
	WBUFL(buf,6)=param;

	return clif_send(buf,packet(0,0x1aa).len,&pd,AREA);
}

int clif_pet_performance(block_list &bl,uint32 param)
{
	unsigned char buf[16];

	memset(buf,0,packet(0,0x1a4).len);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=4;
	WBUFL(buf,3)=bl.id;
	WBUFL(buf,7)=param;

	return clif_send(buf,packet(0,0x1a4).len,&bl,AREA);
}

int clif_pet_equip(struct pet_data &pd)
{
	unsigned char buf[16];
	unsigned short view;

	memset(buf,0,packet(0,0x1a4).len);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=3;
	WBUFL(buf,3)=pd.block_list::id;
	if((view = itemdb_viewid(pd.pet.equip_id)) > 0)
		WBUFL(buf,7)=view;
	else
		WBUFL(buf,7)=pd.pet.equip_id;

	return clif_send(buf,packet(0,0x1a4).len,&pd,AREA);
}

int clif_pet_food(map_session_data &sd,unsigned short foodid,unsigned char fail)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1a3;
	WFIFOB(fd,2)=fail;
	WFIFOW(fd,3)=foodid;
	WFIFOSET(fd,packet(sd.packet_ver,0x1a3).len);

	return 0;
}

/*==========================================
 * オートスペル リスト送信
 *------------------------------------------
 */
int clif_autospell(map_session_data &sd,unsigned short skilllv)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0)=0x1cd;

	if(skilllv>0 && sd.skill_check(MG_NAPALMBEAT)>0)
		WFIFOL(fd,2)= MG_NAPALMBEAT;
	else
		WFIFOL(fd,2)= 0x00000000;
	if(skilllv>1 && sd.skill_check(MG_COLDBOLT)>0)
		WFIFOL(fd,6)= MG_COLDBOLT;
	else
		WFIFOL(fd,6)= 0x00000000;
	if(skilllv>1 && sd.skill_check(MG_FIREBOLT)>0)
		WFIFOL(fd,10)= MG_FIREBOLT;
	else
		WFIFOL(fd,10)= 0x00000000;
	if(skilllv>1 && sd.skill_check(MG_LIGHTNINGBOLT)>0)
		WFIFOL(fd,14)= MG_LIGHTNINGBOLT;
	else
		WFIFOL(fd,14)= 0x00000000;
	if(skilllv>4 && sd.skill_check(MG_SOULSTRIKE)>0)
		WFIFOL(fd,18)= MG_SOULSTRIKE;
	else
		WFIFOL(fd,18)= 0x00000000;
	if(skilllv>7 && sd.skill_check(MG_FIREBALL)>0)
		WFIFOL(fd,22)= MG_FIREBALL;
	else
		WFIFOL(fd,22)= 0x00000000;
	if(skilllv>9 && sd.skill_check(MG_FROSTDIVER)>0)
		WFIFOL(fd,26)= MG_FROSTDIVER;
	else
		WFIFOL(fd,26)= 0x00000000;

	WFIFOSET(fd,packet(sd.packet_ver,0x1cd).len);
	return 0;
}

/*==========================================
 * ディボーションの青い糸
 *------------------------------------------
 */
int clif_devotion(map_session_data &sd,uint32 target_id)
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

	return clif_send(buf,packet(sd.packet_ver,0x1cf).len,&sd,AREA);
}
int clif_marionette(block_list &src, block_list *target)
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

	clif_send(buf,packet(0,0x1cf).len,&src,AREA);
	return 0;
}

/*==========================================
 * 氣球
 *------------------------------------------
 */
int clif_spiritball(map_session_data &sd)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x1d0;
	WBUFL(buf,2)=sd.block_list::id;
	WBUFW(buf,6)=sd.spiritball;
	return clif_send(buf,packet(sd.packet_ver,0x1d0).len,&sd,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_combo_delay(block_list &bl,uint32 wait)
{
	unsigned char buf[32];


	WBUFW(buf,0)=0x1d2;
	WBUFL(buf,2)=bl.id;
	WBUFL(buf,6)=wait;
	return clif_send(buf,packet(0,0x1d2).len,&bl,AREA);
}
/*==========================================
 *白刃取り
 *------------------------------------------
 */
int clif_bladestop(block_list &src,block_list &dst, uint32 _bool)
{
	unsigned char buf[32];

	WBUFW(buf,0)=0x1d1;
	WBUFL(buf,2)=src.id;
	WBUFL(buf,6)=dst.id;
	WBUFL(buf,10)=_bool;

	return clif_send(buf,packet(0,0x1d1).len,&src,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapcell(unsigned short m,unsigned short x,unsigned short y,unsigned short cell_type,int type)
{
	block_list bl;
	unsigned char buf[32];

	bl.m = m;
	bl.x = x;
	bl.y = y;
	WBUFW(buf,0) = 0x192;
	WBUFW(buf,2) = x;
	WBUFW(buf,4) = y;
	WBUFW(buf,6) = cell_type;
	mapname2buffer(WBUFP(buf,8),16,maps[m].mapname);
	if(!type)
		clif_send(buf,packet(0,0x192).len,&bl, AREA);
	else
		clif_send_same_map(bl,buf,packet(0,0x192).len);
	return 0;
}

/*==========================================
 * MVPエフェクト
 *------------------------------------------
 */
int clif_mvp_effect(map_session_data &sd)
{
	unsigned char buf[16];

	WBUFW(buf,0)=0x10c;
	WBUFL(buf,2)=sd.block_list::id;
	return clif_send(buf,packet(sd.packet_ver,0x10c).len,&sd,AREA);
}
/*==========================================
 * MVPアイテム所得
 *------------------------------------------
 */
int clif_mvp_item(map_session_data &sd,unsigned short nameid)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x10a).len);
	return 0;
}
/*==========================================
 * MVP経験値所得
 *------------------------------------------
 */
int clif_mvp_exp(map_session_data &sd, uint32 exp)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x10b;
	WFIFOL(fd,2)=exp;
	WFIFOSET(fd,packet(sd.packet_ver,0x10b).len);
	return 0;
}

/*==========================================
 * ギルド作成可否通知
 *------------------------------------------
 */
int clif_guild_created(map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x167;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x167).len);
	return 0;
}
/*==========================================
 * ギルド所属通知
 *------------------------------------------
 */
int clif_guild_belonginfo(map_session_data &sd,struct guild &g)
{
	int ps,fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	ps=guild_getposition(sd,g);

	memset(WFIFOP(fd,0),0,packet(sd.packet_ver,0x16c).len);
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=g.guild_id;
	WFIFOL(fd,6)=g.emblem_id;
	WFIFOL(fd,10)=g.position[ps].mode;
	memcpy(WFIFOP(fd,19),g.name,24);
	WFIFOSET(fd,packet(sd.packet_ver,0x16c).len);
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
		map_session_data *sd=guild_getavailablesd(g);
		if(sd!=NULL)
			clif_send(buf,packet(0,0x16d).len,sd,GUILD);
	}else
		clif_send(buf,packet(0,0x16d).len, g.member[idx].sd, GUILD_WOS);
	return 0;
}
/*==========================================
 * ギルドマスター通知(14dへの応答)
 *------------------------------------------
 */
int clif_guild_masterormember(map_session_data &sd)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x14e).len);
	return 0;
}
/*==========================================
 * Basic Info (Territories [Valaris])
 *------------------------------------------
 */
int clif_guild_basicinfo(map_session_data &sd)
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
			map_session_data *psd = map_session_data::from_blid(g->member[i].account_id);
			if (psd && psd->status.char_id == g->member[i].char_id &&
				psd->status.guild_id == g->guild_id &&
				!psd->state.waitingdisconnect)
			{
				chaos  += psd->status.chaos; //
				honour -= psd->status.karma; // carma is counted invers, honour not
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

	if     (t==1)  safestrcpy((char*)WFIFOP(fd,94),20,"One Castle");
	else if(t==2)  safestrcpy((char*)WFIFOP(fd,94),20,"Two Castles");
	else if(t==3)  safestrcpy((char*)WFIFOP(fd,94),20,"Three Castles");
	else if(t==4)  safestrcpy((char*)WFIFOP(fd,94),20,"Four Castles");
	else if(t==5)  safestrcpy((char*)WFIFOP(fd,94),20,"Five Castles");
	else if(t==6)  safestrcpy((char*)WFIFOP(fd,94),20,"Six Castles");
	else if(t==7)  safestrcpy((char*)WFIFOP(fd,94),20,"Seven Castles");
	else if(t==8)  safestrcpy((char*)WFIFOP(fd,94),20,"Eight Castles");
	else if(t==9)  safestrcpy((char*)WFIFOP(fd,94),20,"Nine Castles");
	else if(t==10) safestrcpy((char*)WFIFOP(fd,94),20,"Ten Castles");
	else if(t==11) safestrcpy((char*)WFIFOP(fd,94),20,"Eleven Castles");
	else if(t==12) safestrcpy((char*)WFIFOP(fd,94),20,"Twelve Castles");
	else if(t==13) safestrcpy((char*)WFIFOP(fd,94),20,"Thirteen Castles");
	else if(t==14) safestrcpy((char*)WFIFOP(fd,94),20,"Fourteen Castles");
	else if(t==15) safestrcpy((char*)WFIFOP(fd,94),20,"Fifteen Castles");
	else if(t==16) safestrcpy((char*)WFIFOP(fd,94),20,"Sixteen Castles");
	else if(t==17) safestrcpy((char*)WFIFOP(fd,94),20,"Seventeen Castles");
	else if(t==18) safestrcpy((char*)WFIFOP(fd,94),20,"Eighteen Castles");
	else if(t==19) safestrcpy((char*)WFIFOP(fd,94),20,"Nineteen Castles");
	else if(t==20) safestrcpy((char*)WFIFOP(fd,94),20,"Twenty Castles");
	else if(t==21) safestrcpy((char*)WFIFOP(fd,94),20,"Twenty One Castles");
	else if(t==22) safestrcpy((char*)WFIFOP(fd,94),20,"Twenty Two Castles");
	else if(t==23) safestrcpy((char*)WFIFOP(fd,94),20,"Twenty Three Castles");
	else if(t==24) safestrcpy((char*)WFIFOP(fd,94),20,"Twenty Four Castles");
	else if(t==MAX_GUILDCASTLE) safestrcpy((char*)WFIFOP(fd,94),20,"Total Domination");
	else safestrcpy((char*)WFIFOP(fd,94),20,"None Taken");

	WFIFOSET(fd,packet(sd.packet_ver,0x1b6).len);
	clif_guild_emblem(sd,*g);	// Guild emblem vanish fix [Valaris]
	return 0;
}

/*==========================================
 * ギルド同盟/敵対情報
 *------------------------------------------
 */
int clif_guild_allianceinfo(map_session_data &sd)
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
int clif_guild_memberlist(map_session_data &sd)
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
int clif_guild_positionnamelist(map_session_data &sd)
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
int clif_guild_positioninfolist(map_session_data &sd)
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
	map_session_data *sd;
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
	map_session_data *sd;
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
int clif_guild_emblem(map_session_data &sd,struct guild &g)
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
int clif_guild_skillinfo(map_session_data &sd)
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
int clif_guild_notice(map_session_data &sd,struct guild &g)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;
 
	if(*g.mes1==0 && *g.mes2==0)
		return 0;
	WFIFOW(fd,0)=0x16f;
	memcpy(WFIFOP(fd,2),g.mes1,60);
	memcpy(WFIFOP(fd,62),g.mes2,120);
	WFIFOSET(fd,packet(sd.packet_ver,0x16f).len);
	return 0;
}

/*==========================================
 * ギルドメンバ勧誘
 *------------------------------------------
 */
int clif_guild_invite(map_session_data &sd,struct guild &g)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x16a;
	WFIFOL(fd,2)=g.guild_id;
	memcpy(WFIFOP(fd,6),g.name,24);
	WFIFOSET(fd,packet(sd.packet_ver,0x16a).len);
	return 0;
}
/*==========================================
 * ギルドメンバ勧誘結果
 *------------------------------------------
 */
int clif_guild_inviteack(map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x169;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x169).len);
	return 0;
}
/*==========================================
 * ギルドメンバ脱退通知
 *------------------------------------------
 */
int clif_guild_leave(map_session_data &sd,const char *name,const char *mes)
{
	unsigned char buf[128];

	WBUFW(buf, 0)=0x15a;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	return clif_send(buf,packet(sd.packet_ver,0x15a).len,&sd,GUILD);
}
/*==========================================
 * ギルドメンバ追放通知
 *------------------------------------------
 */
int clif_guild_explusion(map_session_data &sd,const char *name,const char *mes,uint32 account_id)
{
	unsigned char buf[128];

	WBUFW(buf, 0)=0x15c;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	memcpy(WBUFP(buf,66),"dummy",24);
	return clif_send(buf,packet(sd.packet_ver,0x15c).len,&sd,GUILD);
}
/*==========================================
 * ギルド追放メンバリスト
 *------------------------------------------
 */
int clif_guild_explusionlist(map_session_data &sd)
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
	map_session_data *sd;
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
int clif_guild_skillup(map_session_data &sd,unsigned short skillid,unsigned short skilllv)
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
int clif_guild_reqalliance(map_session_data &sd,uint32 account_id,const char *name)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x171;
	WFIFOL(fd,2)=account_id;
	memcpy(WFIFOP(fd,6),name,24);
	WFIFOSET(fd,packet(sd.packet_ver,0x171).len);
	return 0;
}
/*==========================================
 * ギルド同盟結果
 *------------------------------------------
 */
int clif_guild_allianceack(map_session_data &sd,uint32 flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x173;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x173).len);
	return 0;
}
/*==========================================
 * ギルド関係解消通知
 *------------------------------------------
 */
int clif_guild_delalliance(map_session_data &sd,uint32 guild_id,uint32 flag)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x184;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x184).len);
	return 0;
}
/*==========================================
 * ギルド敵対結果
 *------------------------------------------
 */
int clif_guild_oppositionack(map_session_data &sd,unsigned char flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x181;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x181).len);
	return 0;
}

/*==========================================
 * ギルド解散通知
 *------------------------------------------
 */
int clif_guild_broken(map_session_data &sd,uint32 flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x15e;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x15e).len);
	return 0;
}

/*==========================================
 * エモーション
 *------------------------------------------
 */
int clif_emotion(block_list &bl,unsigned char type)
{
	unsigned char buf[8];

	WBUFW(buf,0)=0xc0;
	WBUFL(buf,2)=bl.id;
	WBUFB(buf,6)=type;
	return clif_send(buf,packet(0,0xc0).len,&bl,AREA);
}

/*==========================================
 * トーキーボックス
 *------------------------------------------
 */
int clif_talkiebox(block_list &bl, const char* talkie)
{
	unsigned char buf[86];

	WBUFW(buf,0)=0x191;
	WBUFL(buf,2)=bl.id;
	memcpy(WBUFP(buf,6),talkie,80);
	return clif_send(buf,packet(0,0x191).len,&bl,AREA);
}

/*==========================================
 * 結婚エフェクト
 *------------------------------------------
 */
int clif_wedding_effect(block_list &bl)
{
	unsigned char buf[6];

	WBUFW(buf,0) = 0x1ea;
	WBUFL(buf,2) = bl.id;
	return clif_send(buf, packet(0,0x1ea).len, &bl, AREA);
}
/*==========================================
 * あなたに逢いたい使用時名前叫び
 *------------------------------------------

int clif_callpartner(map_session_data &sd)
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
		clif_send(buf,packet(sd.packet_ver,0x1e6).len&sd,AREA);
	}
	return 0;
}
*/
/*==========================================
 * Adopt baby [Celest]
 *------------------------------------------
 */
int clif_adopt_process(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1f8;
	WFIFOSET(fd,packet(sd.packet_ver,0x1f8).len);
	return 0;
}
/*==========================================
 * Marry [DracoRPG]
 *------------------------------------------
 */
void clif_marriage_process(map_session_data &sd)
{
	int fd=sd.fd;
	WFIFOW(fd,0)=0x1e4;
	WFIFOSET(fd,packet(sd.packet_ver,0x1e4).len);
}


/*==========================================
 * Notice of divorce
 *------------------------------------------
 */
int clif_divorced(map_session_data &sd, const char *name)
{
	int fd=sd.fd;
	WFIFOW(fd,0)=0x205;
	memcpy(WFIFOP(fd,2), name, 24);
	WFIFOSET(fd, packet(sd.packet_ver,0x205).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ReqAdopt(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1f6;
	WFIFOSET(fd, packet(sd.packet_ver,0x1f6).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ReqMarriage(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	WFIFOW(fd,0)=0x1e2;
	WFIFOSET(fd, packet(sd.packet_ver,0x1e2).len);
	return 0;
}

/*==========================================
 * 座る
 *------------------------------------------
 */
int clif_sitting(map_session_data &sd)
{
	unsigned char buf[64];

	WBUFW(buf, 0) = 0x8a;
	WBUFL(buf, 2) = sd.block_list::id;
	WBUFB(buf,26) = 2;
	clif_send(buf, packet(sd.packet_ver,0x8a).len, &sd, AREA);

	if(sd.disguise_id)
	{
		WBUFL(buf, 2) = sd.block_list::id|FLAG_DISGUISE;
		clif_send(buf, packet(sd.packet_ver,0x8a).len, &sd, AREA);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_disp_onlyself(map_session_data &sd, const char *mes)
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

int clif_GM_kickack(map_session_data &sd, uint32 id)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xcd;
	WFIFOL(fd,2) = id;
	WFIFOSET(fd, packet(sd.packet_ver,0xcd).len);
	return 0;
}

int clif_GM_kick(map_session_data &sd,map_session_data &tsd,int type)
{
	if(type)
		clif_GM_kickack(sd,tsd.status.account_id);
	tsd.opt1 = tsd.opt2 = 0;

	if( !session_isActive(tsd.fd) )
	return 0;

	WFIFOW(tsd.fd,0) = 0x18b;
	WFIFOW(tsd.fd,2) = 0;
	WFIFOSET(tsd.fd,packet(sd.packet_ver,0x18b).len);
	session_SetWaitClose(tsd.fd, 5000);

	return 0;
}

int clif_GM_silence(map_session_data &sd, map_session_data &tsd, int type)
{
	int fd = tsd.fd;
	if( !session_isActive(fd) )
		return 0;
	
	WFIFOW(fd,0) = 0x14b;
	WFIFOB(fd,2) = 0;
	memcpy(WFIFOP(fd,3), sd.status.name, 24);
	WFIFOSET(fd, packet(sd.packet_ver,0x14b).len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */

int clif_timedout(map_session_data &sd)
{
	ShowInfo("%sCharacter with Account ID '"CL_WHITE"%d"CL_RESET"' timed out.\n", (sd.isGM())?"GM ":"", sd.block_list::id);
	sd.map_quit();
	clif_authfail(sd,3); // Even if player is not on we still send anyway
	session_Remove(sd.fd); // Set session to EOF
	return 0;
}

/*==========================================
 * Wis拒否許可応答
 *------------------------------------------
 */
int clif_wisexin(map_session_data &sd,int type,int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd1;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0xd1).len);

	return 0;
}
/*==========================================
 * Wis全拒否許可応答
 *------------------------------------------
 */
int clif_wisall(map_session_data &sd,int type,int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0xd2;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet(sd.packet_ver,0xd2).len);

	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------
 */
int clif_soundeffect(map_session_data &sd,block_list &bl,const char *name,unsigned char type)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x1d3;
	memcpy(WFIFOP(fd,2),name,24);
	WFIFOB(fd,26)=type;
	WFIFOL(fd,27)=0;
	WFIFOL(fd,31)=bl.id;
	WFIFOSET(fd,packet(sd.packet_ver,0x1d3).len);

	return 0;
}

int clif_soundeffectall(block_list &bl, const char *name, unsigned char type)
{
	unsigned char buf[64];

	WBUFW(buf,0)=0x1d3;
	memcpy(WBUFP(buf,2), name, 24);
	WBUFB(buf,26)=type;
	WBUFL(buf,27)=0;
	WBUFL(buf,31)=bl.id;
	return clif_send(buf, packet(0,0x1d3).len, &bl, AREA);
}

// displaying special effects (npcs, weather, etc) [Valaris]
int clif_specialeffect(const block_list &bl, uint32 type, int flag)
{
	unsigned char buf[64];
	memset(buf, 0, packet(0,0x1f3).len);

	WBUFW(buf,0) = 0x1f3;
	if(bl==BL_PC && ((map_session_data &)bl).disguise_id)
		WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
	else
		WBUFL(buf,2) = bl.id;
	WBUFL(buf,6) = type;

	switch (flag) {
	case 3:
		clif_send_all(buf, packet(0,0x1f3).len);
		break;
	case 2:
		clif_send_same_map(bl, buf, packet(0,0x1f3).len);
		break;
	case 1:
		clif_send_self(bl, buf, packet(0,0x1f3).len);
		break;
	default:
		clif_send(buf, packet(0,0x1f3).len, &bl, AREA);
	}

	return 0;
}


/*------------------------------------------
 * @me command by lordalfa
 *------------------------------------------
*/
int clif_disp_overhead(map_session_data &sd, const char* mes)
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
int clif_charnameack(int fd, block_list &bl, bool clear)
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

	if(bl==BL_PC)
	{
		map_session_data &sd = (map_session_data &)bl;

		if( session_isActive(fd) && session[fd]->user_session)
		{	// first system to detect bot usage
			map_session_data *psd = (map_session_data *)session[fd]->user_session;
			if( psd->block_list::id != sd.block_list::id &&		// not the same player
				sd.is_invisible() &&							// and can see hidden players
				psd->status.gm_level <= sd.status.gm_level )	// and less gm status
			{	// not necessary to send message if GM can do nothing
				// we can not ban automaticly, because if there is lag, hidden player could be not hidden when other player ask for name.
				char message_to_gm[1024];
				snprintf(message_to_gm, sizeof(message_to_gm), "Possible use of BOT (99%% of chance) or modified client by '%s' (account: %lu, char_id: %lu)."
					" This player ask your name when you are hidden.", sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id);
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
	}
	else if(bl==BL_PET)
	{
		struct pet_data& pd = (struct pet_data&)bl;
		safestrcpy((char*)WBUFP(buf,6), 24, pd.pet.name);
		char nameextra[32];
		safestrcpy(nameextra, 22, pd.msd.status.name);
		// need 2 extra chars for the attachment
		strcat(nameextra, "'s");
		cmd = 0x195;
		WBUFB(buf,54) = 0;
		safestrcpy((char*)WBUFP(buf,78), 24, nameextra);
	}
	else if(bl==BL_HOM)
	{
		safestrcpy((char*)WBUFP(buf,6), 24, ((homun_data&)bl).status.name);
	}
	else if(bl==BL_NPC)
	{
		safestrcpy((char*)WBUFP(buf,6), 24, ((npc_data&)bl).name);
	}
	else if(bl==BL_MOB)
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
				safestrcpy((char*)WBUFP(buf,54), 24, g->name);
				safestrcpy((char*)WBUFP(buf,78), 24, gc->castle_name);
			}
		}
		else if(config.show_mob_hp)
		{
			char mobhp[64];
			cmd = 0x195;
			snprintf(mobhp, sizeof(mobhp), "hp: %lu/%lu", (unsigned long)md.hp, (unsigned long)md.max_hp);
			safestrcpy((char*)WBUFP(buf,30), 24, mobhp);
			WBUFB(buf,54) = 0;
			WBUFB(buf,78) = 0;
		}
	}
	else if(bl==BL_CHAT)
	{	//FIXME: Clients DO request this... what should be done about it? The chat's title may not fit... [Skotlex]
		safestrcpy((char*)WBUFP(buf,6), 24, ((chat_data&)bl).title);
	}
	else
	{
		if (config.error_log)
			ShowError("clif_parse_GetCharNameRequest : unknown type (id=%ld)\n", (unsigned long)bl.id);
		return 0;
	}

	WBUFW(buf,0) = cmd;
	WBUFL(buf,2) = bl.id;

	// if no receipient specified just update nearby clients
	if( !session_isActive(fd) )
		clif_send(buf, packet(0,cmd).len, &bl, AREA);
	else
	{
		memcpy(WFIFOP(fd, 0), buf, packet(0,cmd).len);
		WFIFOSET(fd, packet(0,cmd).len);
	}

	return 0;
}





// -- 友達リスト関連

/*==========================================
 * 友達リストの情報通知
 *------------------------------------------
 */
int clif_friend_send_info(map_session_data &sd)
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
int clif_friend_send_online(map_session_data &sd, uint32 account_id, uint32 char_id, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x206;
	WFIFOL( fd, 2 ) = account_id;
	WFIFOL( fd, 6 ) = char_id;
	WFIFOB( fd,10 ) = flag;
	WFIFOSET( fd, packet(sd.packet_ver,0x206).len );
	return 0;
}

/*==========================================
 * 友達リスト追加要請
 *------------------------------------------
 */
int clif_friend_add_request(map_session_data &sd, map_session_data &from_sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x207;
	WFIFOL( fd, 2 ) = from_sd.block_list::id;
	WFIFOL( fd, 6 ) = from_sd.status.char_id;
	memcpy(WFIFOP( fd, 10 ), from_sd.status.name, 24 );
	WFIFOSET( fd, packet(sd.packet_ver,0x207).len );

	return 0;
}

/*==========================================
 * 友達リスト追加要請返答
 *------------------------------------------
 */
int clif_friend_add_ack(map_session_data &sd, uint32 account_id, uint32 char_id, const char* name, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x209;
	WFIFOW( fd, 2 ) = flag;
	WFIFOL( fd, 4 ) = account_id;
	WFIFOL( fd, 8 ) = char_id;
	memcpy(WFIFOP( fd, 12 ), name, 24 );
	WFIFOSET( fd, packet(sd.packet_ver,0x209).len );
	return 0;
}

/*==========================================
 * 友達リスト追加削除通知
 *------------------------------------------
 */
int clif_friend_del_ack(map_session_data &sd, uint32 account_id, uint32 char_id )
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW( fd, 0 ) = 0x20a;
	WFIFOL( fd, 2 ) = account_id;
	WFIFOL( fd, 6 ) = char_id;
	WFIFOSET( fd, packet(sd.packet_ver,0x20a).len );
	return 0;
}








// ランキング表示関連
/*==========================================
 * BSランキング
 *------------------------------------------
 */
int clif_blacksmith_fame(map_session_data &sd, const uint32 total, int delta)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x21b;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd, packet(sd.packet_ver,0x21b).len);

	return 0;
}
int clif_blacksmith_ranking(map_session_data &sd, const CFameList &fl)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x219).len);
	return 0;
}
/*==========================================
 * アルケミランキング
 *------------------------------------------
 */
int clif_alchemist_fame(map_session_data &sd, const uint32 total, int delta)
{
	int fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x21c;
	WFIFOL(fd,2) = delta;
	WFIFOL(fd,6) = total;
	WFIFOSET(fd, packet(sd.packet_ver,0x21c).len);
	
	return 0;
}
int clif_alchemist_ranking(map_session_data &sd, const CFameList &fl)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x21a).len);
	return 0;
}
/*==========================================
 * テコンランキング
 *------------------------------------------
 */
int clif_taekwon_fame(const map_session_data &sd, const uint32 total, int delta)
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
int clif_taekwon_ranking(map_session_data &sd, const CFameList &fl)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x226).len);
	return 0;
}
/*==========================================
 * 虐殺者ランキング
 *------------------------------------------
 */
int clif_pk_fame(map_session_data &sd, const uint32 total, int delta)
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
int clif_pk_ranking(map_session_data &sd, const CFameList &fl)
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
	WFIFOSET(fd,packet(sd.packet_ver,0x238).len);
	return 0;
}
/*==========================================
 * Info about Star Glaldiator save map [Komurka]
 *------------------------------------------
 */
int clif_feel_info(map_session_data &sd, int feel_level)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;
	//memcpy(WFIFOP(fd,2),mapindex_id2name(sd->feel_map[feel_level].index), MAP_NAME_LENGTH);
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=0x100+feel_level;
	WFIFOSET(fd, packet(sd.packet_ver,0x20e).len);
	return 0;
}

/*==========================================
 * Info about Star Glaldiator hate mob [Komurka]
 *------------------------------------------
 */
int clif_hate_mob(map_session_data &sd, unsigned short skilllv, unsigned short  mob_id)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;

	if( 0!=strcmp(job_name(mob_id),"Unknown Job") )
		safestrcpy((char*)WFIFOP(fd,2), 24, job_name(mob_id));
	else if( mobdb_checkid(mob_id) )
		safestrcpy((char*)WFIFOP(fd,2), 24, mob_db[mob_id].jname);
	else 
		WFIFOB(fd,2) = 0;
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=0xa00+skilllv-1;
	WFIFOSET(fd, packet(sd.packet_ver,0x20e).len);
	return 0;
}

/*==========================================
 * Info about TaeKwon Do TK_MISSION mob [Skotlex]
 *------------------------------------------
 */
int clif_mission_mob(map_session_data &sd, unsigned short mob_id, unsigned short progress)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x20e;
	safestrcpy((char*)WFIFOP(fd,2), 24, (mobdb_checkid(mob_id))?mob_db[mob_id].jname:"");
	WFIFOL(fd,26)=mob_id;
	WFIFOW(fd,30)=0x1400+progress; //Message to display
	WFIFOSET(fd, packet(sd.packet_ver,0x20e).len);
	return 0;
}




/*==========================================
 * メールBOXを表示させる
 *------------------------------------------
 */
int clif_openmailbox(map_session_data &sd)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x260;
	WFIFOL(fd,2) = 0;
	WFIFOSET(fd,packet(sd.packet_ver,0x260).len);
	return 0;
}
/*==========================================
 * メール一覧表（BOXを開いている時に蔵へ送信）
 *  0x23fの応答
 *------------------------------------------
 */
int clif_send_mailbox(map_session_data &sd, uint32 count, const unsigned char* buffer)
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

				snprintf(message, sizeof(message), "%c %-8lu %4i/%02i/%02i %2i:%02i:%02i %-24s %s",
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
int clif_res_sendmail(map_session_data &sd, bool ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x249;
	WFIFOB(fd,2) = ok?0:1;
	WFIFOSET(fd,packet(sd.packet_ver,0x249).len);
	return 0;
}
/*==========================================
 * アイテムが添付できましたとかできませんとか
 *------------------------------------------
 */
int clif_res_sendmail_setappend(map_session_data &sd, int flag)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x245;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,packet(sd.packet_ver,0x245).len);
	return 0;
}
/*==========================================
 * 新着メールが届きました
 *------------------------------------------
 */
int clif_arrive_newmail(map_session_data &sd, const CMailHead& mh)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x24a;
	WFIFOL(fd,2) = mh.msgid;
	memcpy(WFIFOP(fd, 6),mh.name, 24);
	memcpy(WFIFOP(fd,30),mh.head, 40);
	WFIFOSET(fd,packet(sd.packet_ver,0x24a).len);
	return 0;
}
/*==========================================
 * メールを選択受信	Inter→本人へ
 *------------------------------------------
 */
int clif_receive_mail(map_session_data &sd, const CMail& md)
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

			snprintf(message, sizeof(message), "%c %-8lu %4i/%02i/%02i %2i:%02i:%02i %-24s %s",
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
int clif_deletemail_res(map_session_data &sd, uint32 msgid, bool ok)
{
	int fd=sd.fd;
	if( !session_isActive(fd) )
		return 0;

	if( sd.packet_ver > 18 )
	{	// use client-side mail when supported

		WFIFOW(fd,0) = 0x257;
		WFIFOL(fd,2) = msgid;
		WFIFOL(fd,6) = ok?0:1;
		WFIFOSET(fd,packet(sd.packet_ver,0x257).len);
	}
	else
	{
		char message[512];
		snprintf(message, sizeof(message), "mail %u delet%s.", msgid, (ok?"ed":"ion failed"));
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
		
		for(i=MAX_PACKET_VER; i>0; --i)
		{
			if( cmd==packet_connect(i) &&
				getAthentification( RFIFOL(fd, packet(i,cmd).pos[0]) ,auth) &&
				// explicitely tested above
				//auth.account_id == RFIFOL(fd, packet(i,cmd).pos[0])
				auth.login_id1  == RFIFOL(fd, packet(i,cmd).pos[2]) &&
				auth.client_ip  == session[fd]->client_ip &&
				RFIFOREST(fd) >= packet(i,cmd).len &&
				(RFIFOB(fd, packet(i,cmd).pos[4]) == 0 ||	// 00 = Female
				 RFIFOB(fd, packet(i,cmd).pos[4]) == 1) )	// 01 = Male
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
int clif_parse_WantToConnection(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	map_session_data *old_sd;
	unsigned short cmd	= RFIFOW(fd,0);
	uint32 account_id	= RFIFOL(fd, packet(sd.packet_ver,cmd).pos[0]);
	uint32 char_id		= RFIFOL(fd, packet(sd.packet_ver,cmd).pos[1]);
	uint32 login_id1	= RFIFOL(fd, packet(sd.packet_ver,cmd).pos[2]);
	uint32 client_tick	= RFIFOL(fd, packet(sd.packet_ver,cmd).pos[3]);
	unsigned char sex	= RFIFOB(fd, packet(sd.packet_ver,cmd).pos[4]);
	CAuth auth;

	//ShowMessage("WantToConnection: Received %d bytes with packet 0x%X -> ver %i\n", RFIFOREST(fd), cmd, sd.packet_ver);

	// if same account already connected, we disconnect the 2 sessions
	if((old_sd = map_session_data::from_blid(account_id)) != NULL)
	{
		clif_authfail(sd, 8); // still recognizes last connection
		clif_authfail(*old_sd, 2); // same id

		session_Remove(fd); // Set session to EOF
		session_Remove(old_sd->fd); // Set session to EOF
	}
	else
	{
		
		getAthentification(account_id, auth);
		map_session_data *plsd = new map_session_data(fd, sd.packet_ver, account_id, char_id, login_id1, auth.login_id2, client_tick, sex);

		session[fd]->user_session = plsd;
		plsd->fd = fd;
		plsd->packet_ver = sd.packet_ver;

		plsd->ScriptEngine.temporaty_init(); //!! call constructor explicitely until switched to c++ allocation

		WFIFOL(fd,0) = plsd->block_list::id;
		WFIFOSET(fd,4);

		plsd->register_id(plsd->block_list::id);
		chrif_authreq(*plsd);
	}

	return 0;
}

/*==========================================
 * 007d クライアント側マップ読み込み完了
 * map侵入時に必要なデータを全て送りつける
 *------------------------------------------
 */
int clif_parse_LoadEndAck(int fd, map_session_data &sd)
{
	int i;

	if( !session_isActive(fd) )
		return 0;

	if( sd.is_on_map() )
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
	if( sd.is_carton() )
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

	if(config.pc_invincible_time > 0)
	{
		if(maps[sd.block_list::m].flag.gvg)
			pc_setinvincibletimer(sd,config.pc_invincible_time<<1);
		else
			pc_setinvincibletimer(sd,config.pc_invincible_time);
	}

	sd.addblock();	// ブロック登録
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
	if(maps[sd.block_list::m].flag.pvp)
	{
		if(!config.pk_mode)
		{	// remove pvp stuff for pk_mode [Valaris]
			sd.pvp_timer=add_timer(gettick()+200,pc_calc_pvprank_timer,sd.block_list::id,0);
			sd.pvp_rank=0;
			sd.pvp_lastusers=0;
			sd.pvp_point=5;
			sd.pvp_won=0;
			sd.pvp_lost=0;
		}
		clif_set0199(sd.fd,1);
	}
	else
	{
		sd.pvp_timer=-1;
	}
	if(maps[sd.block_list::m].flag.gvg)
	{
		clif_set0199(sd.fd,3);
	}

	// pet
	if(sd.status.pet_id > 0 && sd.pd && sd.pd->pet.intimate > 0)
	{
		sd.pd->addblock();
		clif_spawnpet(*sd.pd);
		clif_send_petdata(sd,0,0);
		clif_send_petdata(sd,5,config.pet_hair_style);
		clif_send_petstatus(sd);
	}
	if(sd.hd)
	{
		sd.hd->delblock();
		sd.hd->addblock();
		clif_spawnhom(*sd.hd);
		clif_send_homdata(sd,0,0);
		clif_send_homstatus(sd,1);
		clif_send_homstatus(sd,0);
	}
	if(sd.state.connect_new)
	{
		sd.state.connect_new = 0;
		if(sd.status.class_ != sd.view_class)
			clif_changelook(sd,LOOK_BASE,sd.view_class);
		if(sd.status.pet_id > 0 && sd.pd && sd.pd->pet.intimate > 900)
			clif_pet_emotion(*sd.pd,(sd.pd->pet.class_ - 100)*100 + 50 + pet_hungry_val(sd));

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

	//if(sd.status.hp<sd.status.max_hp>>2 && sd.skill_check(SM_AUTOBERSERK)>0 &&
	if(sd.status.hp<sd.status.max_hp>>2 && sd.has_status(SC_AUTOBERSERK) &&
		( !sd.has_status(SC_PROVOKE) || sd.get_statusvalue2(SC_PROVOKE).integer()==0 ))
		// オートバーサーク発動
		status_change_start(&sd,SC_PROVOKE,10,1,0,0,0,0);

//	if(time(&timer) < ((weddingtime=pc_readglobalreg(sd,"PC_WEDDING_TIME")) + 3600))
//		status_change_start(&sd,SC_WEDDING,0,weddingtime,0,0,36000,0);

	if(config.muting_players && sd.status.manner < 0)
		status_change_start(&sd,SC_NOCHAT,0,0,0,0,0,0);

	if (daynight_flag && !maps[sd.block_list::m].flag.indoors)
		clif_seteffect(sd, sd.block_list::id, 474 + config.night_darkness_level);


	// option
	clif_changeoption(sd);
	if( sd.has_status(SC_TRICKDEAD) )
		status_change_end(&sd,SC_TRICKDEAD,-1);
	if( sd.has_status(SC_SIGNUMCRUCIS) && !sd.is_undead() )
		status_change_end(&sd,SC_SIGNUMCRUCIS,-1);
	if( sd.state.infinite_endure && !sd.has_status(SC_ENDURE) )
		status_change_start(&sd,SC_ENDURE,10,1,0,0,0,0);
	for(i=0;i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].equip && sd.status.inventory[i].equip & 0x0002 && sd.status.inventory[i].attribute==1)
			status_change_start(&sd,SC_BROKNWEAPON,0,0,0,0,0,0);
		if(sd.status.inventory[i].equip && sd.status.inventory[i].equip & 0x0010 && sd.status.inventory[i].attribute==1)
			status_change_start(&sd,SC_BROKNARMOR,0,0,0,0,0,0);
	}

	block_list::foreachinarea( CClifGetAreaChar(sd),
		sd.block_list::m,((int)sd.block_list::x)-AREA_SIZE,((int)sd.block_list::y)-AREA_SIZE,((int)sd.block_list::x)+AREA_SIZE,((int)sd.block_list::y)+AREA_SIZE,BL_ALL);


	
	if(!sd.state.event_onconnect)
	{
		npc_data::event("OnConnect", sd);
		sd.state.event_onconnect=1;
	}
	npc_data::event("OnPCLoadMapEvent", script_config.mapload_event_name, sd);

	send_fake_id(fd,sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_TickSend(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	sd.client_tick=RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	return clif_servertick(sd, gettick());
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_WalkToXY(int fd, map_session_data &sd)
{
	int x, y;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.is_dead() )
		return clif_clearchar_area(sd, 1);

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.chat || sd.is_sitting() )
		return 0;

	if (sd.skilltimer != -1 && sd.skill_check( SA_FREECAST) <= 0) // フリーキャスト
		return 0;

	if (sd.canmove_tick > gettick())
		return 0;

	// ステータス異常やハイディング中(トンネルドライブ無)で動けない
	if( (sd.opt1 > 0 && sd.opt1 != 6) ||
	     sd.has_status(SC_ANKLE) || //アンクルスネア
	     sd.has_status(SC_AUTOCOUNTER) || //オートカウンター
	     sd.has_status(SC_TRICKDEAD) || //死んだふり
	     sd.has_status(SC_BLADESTOP) || //白刃取り
	     sd.has_status(SC_SPIDERWEB) || //スパイダーウェッブ
	     (sd.has_status(SC_DANCING) && sd.get_statusvalue4(SC_DANCING).integer()) || //合奏スキル演奏中は動けない
		 (sd.has_status(SC_GOSPEL) && sd.get_statusvalue4(SC_GOSPEL).integer() == BCT_SELF) ||	// cannot move while gospel is in effect
		 (sd.has_status(SC_DANCING) && sd.get_statusvalue1(SC_DANCING).integer() == CG_HERMODE)  //cannot move while Hermod is active.
		)
		return 0;
	if ((sd.status.option & 2) && sd.skill_check( RG_TUNNELDRIVE) <= 0)
		return 0;

	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);

	sd.stop_attack();

	
	int cmd = RFIFOW(fd,0);
	x = RFIFOB(fd,packet(sd.packet_ver,cmd).pos[0]) * 4 +
		(RFIFOB(fd,packet(sd.packet_ver,cmd).pos[0] + 1) >> 6);
	y = ((RFIFOB(fd,packet(sd.packet_ver,cmd).pos[0]+1) & 0x3f) << 4) +
		(RFIFOB(fd,packet(sd.packet_ver,cmd).pos[0] + 2) >> 4);

	sd.walktoxy(x,y);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_QuitGame(int fd, map_session_data &sd)
{
	unsigned long tick=gettick();
	struct skill_unit_group* sg;

	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x18b;
	if( (!sd.is_dead() && (sd.opt1 || (sd.opt2 && !(daynight_flag && sd.opt2 == STATE_BLIND)))) ||
	    sd.skilltimer != -1 ||
	    (DIFF_TICK(tick, sd.canact_tick) < 0) ||
	    (sd.has_status(SC_DANCING) && (sg=(struct skill_unit_group *)sd.get_statusvalue2(SC_DANCING).pointer()) && sg->src_id == sd.block_list::id) ||
		(config.prevent_logout && sd.is_dead() && DIFF_TICK(tick,sd.canlog_tick) < 10000) )
	{	// fail
		WFIFOW(fd,2)=1;
	}
	else
	{	// ok
		session_SetWaitClose(fd, 1000);
		WFIFOW(fd,2)=0;
	}
	WFIFOSET(fd,packet(sd.packet_ver,0x18b).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_GetCharNameRequest(int fd, map_session_data &sd)
{
	block_list* bl;
	uint32 account_id;
	if( !session_isActive(fd) )
		return 0;
	
	account_id = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);

	if(account_id & FLAG_DISGUISE) // for disguises [Valaris]
		account_id &= ~FLAG_DISGUISE;

	if ((bl = block_list::from_blid(account_id)) != NULL)	
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
int clif_parse_GlobalMessage(int fd, map_session_data &sd)
{	// S 008c <len>.w <str>.?B

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	size_t buffersize = RFIFOW(fd,2);
	if( buffersize > (size_t)RFIFOREST(fd) )
	{	// some serious error
		ShowError("clif_parse_GlobalMessage: size marker outside buffer %i > %i, (connection %i=%i,%p=%p)", 
			buffersize, RFIFOREST(fd), fd,sd.fd, session[fd]->user_session, &sd);
		return 0;
	}
	// add an eof marker in the buffer (really inside, not extending the used buffer)
	RFIFOB(fd,buffersize-1)=0;

	const char* message = (const char*)RFIFOP(fd,4);

	//ShowMessage("clif_parse_GlobalMessage: message: '%s'.\n", message);

	if( CommandInfo::is_command(fd, sd, message) ||
	    sd.has_status(SC_BERSERK) || //バーサーク時は会話も不可
		sd.has_status(SC_NOCHAT) ) //チャット禁止
		return 0;
	
	// send message to others
	unsigned char buf[65536];
	WBUFW(buf,0) = 0x8d;
	WBUFW(buf,2) = buffersize + 4; // len of message - 4 + 8
	WBUFL(buf,4) = sd.block_list::id;
	memcpy(WBUFP(buf,8), message, buffersize - 4);
	clif_send(buf, buffersize + 4, &sd, sd.chat ? CHAT_WOS : AREA_CHAT_WOC);
	
	// send back message to the speaker
	memcpy(WFIFOP(fd,0), RFIFOP(fd,0), buffersize);
	WFIFOW(fd,0) = 0x8e;
	WFIFOSET(fd, buffersize);

	// execute npcchats in the area
	block_list::foreachinarea( CNpcChat(message, sd),
		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE, ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_NPC);

	// novice message induced automata
	if( pc_calc_base_job2 (sd.status.class_) == 23 )
	{
		int next = pc_nextbaseexp(sd) > 0 ? pc_nextbaseexp(sd) : sd.status.base_exp;
		if (next > 0 && (sd.status.base_exp * 100 / next) % 10 == 0)
		{
			if (sd.state.snovice_flag == 0 && strstr(message, msg_txt(MSG_GUARDIAN_ANGEL)))
				sd.state.snovice_flag = 1;
			else if (sd.state.snovice_flag == 1)
			{
				char output[512];
				snprintf(output, sizeof(output), msg_txt(MSG_NAME_IS_S_AND_SUPER_NOVICE), sd.status.name);
				if( strstr(message, output) )
					sd.state.snovice_flag = 2;
			}
			else if (sd.state.snovice_flag == 2 && strstr(message, msg_txt(MSG_PLEASE_HELP_ME)))
			{
				sd.state.snovice_flag = 3;
			}
			else if (sd.state.snovice_flag == 3)
			{
				clif_skill_nodamage(sd,sd,MO_EXPLOSIONSPIRITS,0xFFFF,1);
				status_change_start(&sd,(status_t)SkillStatusChangeTable[MO_EXPLOSIONSPIRITS],
						17,0,0,0,skill_get_time(MO_EXPLOSIONSPIRITS,1),0 ); //Lv17-> +50 critical (noted by Poki) [Skotlex]
				sd.state.snovice_flag = 0;
			}
		}
	}
	return 0;
}

int clif_message(block_list &bl, const char* msg)
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
int clif_parse_MapMove(int fd, map_session_data &sd)
{	// /m /mapmove (as @rura GM command)
	char output[32];
	char mapname[32], *ip;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_mapmove) )
	{
		safestrcpy(mapname, sizeof(mapname), (const char*)RFIFOP(fd,2));
		ip = strchr(mapname, '.');
		if(ip) *ip=0;
		snprintf(output, sizeof(output), "%s %d %d", mapname, (unsigned char)RFIFOW(fd,18), (unsigned char)RFIFOW(fd,20));
		command_mapmove(fd, sd, "@mapmove", basics::CParameterList(output));
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changed_dir(block_list &bl)
{
	if( bl.is_on_map() )
	{
		unsigned char buf[64];

		WBUFW(buf,0) = 0x9c;
		WBUFL(buf,2) = bl.id;
		WBUFW(buf,6) = bl.get_headdir();
		WBUFB(buf,8) = bl.get_bodydir();

		clif_send(buf, packet(0,0x9c).len, &bl, AREA_WOS);

		map_session_data *sd = bl.get_sd();
		if(sd)
		{
			if(sd->disguise_id)
			{
				WBUFL(buf,2) = bl.id|FLAG_DISGUISE;
				clif_send(buf, packet(0,0x9c).len, &bl, AREA);
			}
		}
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChangeDir(int fd, map_session_data &sd)
{
	dir_t bodydir, headdir;

	if( !session_isActive(fd)  || !sd.is_on_map() )
		return 0;

	headdir = (dir_t)(((unsigned char)RFIFOB(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]))&0x07);
	bodydir = (dir_t)(((unsigned char)RFIFOB(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]))&0x07);
	sd.set_dir(bodydir, headdir);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_Emotion(int fd, map_session_data &sd)
{
	unsigned char buf[64];
	if( !session_isActive(fd)  || !sd.is_on_map() )
		return 0;

	if(config.basic_skill_check == 0 || sd.skill_check( NV_BASIC) >= 2)
	{
		if(RFIFOB(fd,2) == 34)
		{	// prevent use of the mute emote [Valaris]
			sd.skill_failed(1, SF_NOEMOTION);
			return 0;
		}
		// fix flood of emotion icon (ro-proxy): flood only the hacker player
		if(sd.emotionlasttime >= time(NULL))
		{
			sd.emotionlasttime = time(NULL) + 1; // not more than 1 every second (normal client is every 3-4 seconds)
			sd.skill_failed( 1, SF_NOEMOTION);
			return 0;
		}
		sd.emotionlasttime = time(NULL) + 1; // not more than 1 every second (normal client is every 3-4 seconds)

		WBUFW(buf,0) = 0xc0;
		WBUFL(buf,2) = sd.block_list::id;
		WBUFB(buf,6) = RFIFOB(fd,2);
		return clif_send(buf, packet(sd.packet_ver,0xc0).len, &sd, AREA);
	} else
		sd.skill_failed( 1, SF_NOEMOTION);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int  clif_parse_HowManyConnections(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0xc2;
	WFIFOL(fd,2) = map_getusers();
	WFIFOSET(fd,packet(sd.packet_ver,0xc2).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ActionRequest(int fd, map_session_data &sd)
{
	unsigned long tick;
	unsigned char buf[64];
	int action_type;
	uint32 target_id;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.is_dead() )
	{
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.opt1 > 0 || sd.status.option & 2 ||
	     (sd.has_status(SC_TRICKDEAD) ||
		  sd.has_status(SC_AUTOCOUNTER) || //オートカウンター
	      sd.has_status(SC_BLADESTOP) || //白刃取り
	      sd.has_status(SC_DANCING)) ) //ダンス中
		return 0;

	tick = gettick();

	sd.stop_walking(0);
	sd.stop_attack();

	target_id = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	action_type = RFIFOB(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);
	
	switch(action_type) {
	case 0x00: // once attack
	case 0x07: // continuous attack


		if(sd.has_status(SC_WEDDING) || sd.view_class==22)
			return 0;
		if (sd.vender_id != 0)
			return 0;
		if (!config.skill_delay_attack_enable && sd.skill_check( SA_FREECAST) <= 0) {
			if (DIFF_TICK(tick, sd.canact_tick) < 0) {
				sd.skill_failed( 1, SF_DELAY);
				return 0;
			}
		}
		if (sd.invincible_timer != -1)
			pc_delinvincibletimer(sd);
		if (sd.target_id) // [Valaris]
			sd.target_id = 0;
		pc_attack(sd, target_id, action_type != 0);
		break;
	case 0x02: // sitdown
		if (config.basic_skill_check == 0 || sd.skill_check( NV_BASIC) >= 3) {
			if (sd.skilltimer != -1) //No sitting while casting :P
				break;
			sd.stop_attack();
			sd.stop_walking(1);
			sd.set_sit();
			skill_gangsterparadise(&sd, 1); // ギャングスターパラダイス設定 fixed Valaris
			skill_rest(sd, 1); // TK_HPTIME sitting down mode [Dralnu]
			clif_sitting(sd);
		} else
			sd.skill_failed( 1, SF_NOSIT);
		break;
	case 0x03: // standup
		sd.set_stand();
		skill_gangsterparadise(&sd, 0); // ギャングスターパラダイス解除 fixed Valaris
		skill_rest(sd, 0); // TK_HPTIME standing up mode [Dralnu]
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = sd.block_list::id;
		WBUFB(buf,26) = 3;
		clif_send(buf, packet(sd.packet_ver,0x8a).len, &sd, AREA);
		if(sd.disguise_id) {
			WBUFL(buf, 2) = sd.block_list::id|FLAG_DISGUISE;
			clif_send(buf, packet(sd.packet_ver,0x8a).len, &sd, AREA);
		}
		break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_Restart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOB(fd,2))
	{
	case 0x00:
	{
		if( sd.is_dead() )
		{
			sd.set_stand();
			pc_setrestartvalue(sd, 3);
			pc_setpos(sd, sd.status.save_point.mapname, sd.status.save_point.x, sd.status.save_point.y, 2);
		}
		// in case the player's status somehow wasn't updated yet [Celest]
		else if (sd.status.hp <= 0)
			sd.set_dead();
		break;
	}
	case 0x01:
	{
		struct skill_unit_group* sg;
		unsigned long tick = gettick();
		if( (!sd.is_dead() && (sd.opt1 || (sd.opt2 && !(daynight_flag && sd.opt2 == STATE_BLIND)))) ||
			sd.skilltimer != -1 ||
			(DIFF_TICK(tick, sd.canact_tick) < 0) ||
			(sd.has_status(SC_DANCING) && (sg=(struct skill_unit_group *)sd.get_statusvalue2(SC_DANCING).pointer()) && sg->src_id == sd.block_list::id) ||
			(config.prevent_logout && sd.is_dead() && DIFF_TICK(tick,sd.canlog_tick) < 10000) )
		{	// fail
			WFIFOW(fd,0)=0x18b;
			WFIFOW(fd,2)=1;
			WFIFOSET(fd,packet(sd.packet_ver,0x018b).len);
		}
		else
		{	// ok
			chrif_charselectreq(sd);
			session_SetWaitClose(fd, 2000);
			sd.map_quit();
		}
		break;
	}
	}
	return 0;
}


/*==========================================
 * Transmission of a wisp (S 0096 <len>.w <nick>.24B <message>.?B)
 *------------------------------------------
 */
int clif_parse_Wis(int fd, map_session_data &sd)
{	// S 0096 <len>.w <nick>.24B <message>.?B // rewritten by [Yor]
	if( !session_isActive(fd) || !sd.is_on_map() ||
		sd.has_status(SC_BERSERK) || //バーサーク時は会話も不可
		sd.has_status(SC_NOCHAT) ) //チャット禁止
		return 0;

	size_t buffersize = RFIFOW(fd,2);
	if( buffersize > (size_t)RFIFOREST(fd) )
	{	// some serious error
		ShowError("clif_parse_Wis: size marker outside buffer %i > %i, (connection %i=%i,%p=%p)", 
			buffersize, RFIFOREST(fd), fd,sd.fd, session[fd]->user_session, &sd);
		return 0;
	}
	// terminate the name inside the buffer
	RFIFOB(fd,27)=0;
	// add an eof marker in the buffer (really inside, not extending the used buffer)
	RFIFOB(fd,buffersize-1)=0;

	const char *target  = (const char*)RFIFOP(fd, 4);
	const char *message = (const char*)RFIFOP(fd,28);

	//ShowMessage("clif_parse_Wis: message to %s: '%s'.\n", target, message);

	basics::string<> gm_command;

	gm_command << sd.status.name << " : " << message;
	if( CommandInfo::is_command(fd, sd, gm_command) )
	{
		return 0;
	}


	//Chat Logging type 'W' / Whisper
	if(log_config.chat&1 //we log everything then
		|| ( log_config.chat&2 //if Whisper bit is on
		&& ( !agit_flag || !(log_config.chat&16) ))) //if WOE ONLY flag is off or AGIT is OFF
		log_chat("W", 0, sd.status.char_id, sd.status.account_id, sd.mapname, sd.block_list::x, sd.block_list::y, target, message);


	//-------------------------------------------------------//
	//   Lordalfa - Paperboy - To whisper NPC commands       //
	//-------------------------------------------------------//
	npcscript_data *npc = npcscript_data::from_name(target);
	if( npc )
	{
		char tempmes[128];
		char *ip=(char*)message, *kp=NULL;
		size_t i;
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
			snprintf(tempmes, sizeof(tempmes),"@whispervar%lu$", (unsigned long)i);
			set_var(sd,tempmes,kp);
		}//Sets Variables to use in the NPC
		npc_data::event("OnWhisperGlobal", *npc, sd); // Calls the NPC label
	}
	//-------------------------------------------------------//
	//  Lordalfa - Paperboy - END - NPC Whisper Commands     //
	//-------------------------------------------------------//

	// searching destination character
	map_session_data *dstsd = map_session_data::nick2sd(target);
	// player is not on this map-server
	if (dstsd == NULL ||
	// At this point, don't send wisp/page if it's not exactly the same name, because (example)
	// if there are 'Test' player on an other map-server and 'test' player on this map-server,
	// and if we ask for 'Test', we must not contact 'test' player
	// so, we send information to inter-server, which is the only one which decide (and copy correct name).
	    strcmp(dstsd->status.name, target) != 0) // not exactly same name
	{	// send message to inter-server
		intif_wis_message(sd, target, message, buffersize-28);
	}
	else
	{	// player is on this map-server
		if(dstsd->fd == fd)
		{	// prevent sending to yourself
			clif_wis_message(fd, wisp_server_name, "You can not page yourself. Sorry.", 1+strlen("You can not page yourself. Sorry."));
		}
		else
		{	// otherwise, send message and answer immediatly
			if(dstsd->state.ignoreAll == 1)
			{
				if (dstsd->status.option & OPTION_HIDE && sd.isGM() < dstsd->isGM())
					clif_wis_end(fd, 1); // 1: target character is not loged in
				else
					clif_wis_end(fd, 3); // 3: everyone ignored by target
			}
			else
			{
				// check ignore list
				size_t i;
				for(i=0; i<MAX_IGNORE_LIST; ++i)
				{
					if( 0==strcmp(dstsd->ignore[i].name, sd.status.name) )
					{
						clif_wis_end(fd, 2);	// 2: ignored by target
						break;
					}
				}
				// if source player not found in ignore list
				if(i>=MAX_IGNORE_LIST)
				{
					clif_wis_message(dstsd->fd, sd.status.name, message, buffersize-28);
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
int clif_parse_GMmessage(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_broadcast) )
		intif_GMmessage((char*)RFIFOP(fd,4),RFIFOW(fd,2)-4, 0);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_TakeItem(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.is_dead() )
		return clif_clearchar_area(sd, 1);

	uint32 map_object_id = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	flooritem_data *fitem = flooritem_data::from_blid(map_object_id);

	if (fitem == NULL || fitem->block_list::m != sd.block_list::m)
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0 ||
		sd.is_cloaking() || sd.is_chasewalk() || //Disable cloaking/chasewalking characters from looting [Skotlex]
		(sd.has_status(SC_TRICKDEAD) || //死んだふり
		 sd.has_status(SC_BLADESTOP) || //白刃取り
		 sd.has_status(SC_BERSERK) ||	//バーサーク
		 sd.has_status(SC_CHASEWALK) ||  //Chasewalk [Aru]
		 sd.has_status(SC_NOCHAT)) )	//会話禁止
	{
		clif_additem(sd,0,0,6); // send fail packet! [Valaris]
		return 0;
	}

	pc_takeitem(sd, *fitem);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_DropItem(int fd, map_session_data &sd)
{
	int item_index, item_amount;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if (sd.is_dead()) {
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0 ||
		(sd.has_status(SC_AUTOCOUNTER) || //オートカウンター
		sd.has_status(SC_BLADESTOP) || //白刃取り
		sd.has_status(SC_BERSERK)) ) //バーサーク
		return 0;

	item_index = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0])-2;
	item_amount = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);

	pc_dropitem(sd, item_index, item_amount);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_UseItem(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if (sd.is_dead()) {
		return clif_clearchar_area(sd, 1);
	}
	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || (sd.opt1 > 0 && sd.opt1 != 6) ||
	    (sd.has_status(SC_TRICKDEAD) || //死んだふり
	     sd.has_status(SC_BLADESTOP) || //白刃取り
		sd.has_status(SC_BERSERK) ||	//バーサーク
		sd.has_status(SC_NOCHAT) ||
		sd.has_status(SC_GRAVITATION)) )	//会話禁止
		return 0;

	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);

	pc_useitem(sd,RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0])-2);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_EquipItem(int fd, map_session_data &sd)
{
	unsigned short index;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.is_dead()) {
		return clif_clearchar_area(sd, 1);
	}
	index = RFIFOW(fd,2)-2;
	if( index >= MAX_INVENTORY || sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner != 0)
		return 0;
	if( sd.has_status(SC_BLADESTOP) || sd.has_status(SC_BERSERK) ) 
		return 0;

	if(	sd.status.inventory[index].identify != 1) {		// 未鑑定
		return clif_equipitemack(sd,index,0,0);	// fail
	}
	//ペット用装備であるかないか
	if(sd.inventory_data[index])
	{
		if(sd.inventory_data[index]->type != 8)
		{
			if(sd.inventory_data[index]->type == 10)
				RFIFOW(fd,4)=0x8000;	// 矢を無理やり装備できるように（－－；
			pc_equipitem(sd,index,RFIFOW(fd,4));
		}
		else
			pet_equipitem(sd,index);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_UnequipItem(int fd, map_session_data &sd)
{
	int index;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.is_dead()) {
		return clif_clearchar_area(sd, 1);
	}
	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0 || sd.opt1 > 0)
		return 0;
	index = RFIFOW(fd,2)-2;

	/*if( sd.status.inventory[index].attribute == 1 && sd.has_status(SC_BROKNWEAPON) )
		status_change_end(&sd,SC_BROKNWEAPON,-1);
	if( sd.status.inventory[index].attribute == 1 && sd.has_status(SC_BROKNARMOR) )
		status_change_end(&sd,SC_BROKNARMOR,-1);
	if( sd.has_status(SC_BLADESTOP) || sd.has_status(SC_BERSERK) )
		return 0;*/

	pc_unequipitem(sd,index,1);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcClicked(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.is_dead()) {
		return clif_clearchar_area(sd, 1);
	}
	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.trade_partner || RFIFOL(fd,2)&FLAG_DISGUISE)
		return 0;
	npc_data::click(sd,RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcBuySellSelected(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;

	npcshop_data *sh= npcshop_data::from_blid( RFIFOL(fd,2) );
	if( sh ) 
	{
		if( 0==RFIFOB(fd,6) )
			sh->buywindow(sd);
		else
			sh->sellwindow(sd);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcBuyListSend(int fd, map_session_data &sd)
{
	int fail=1;
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.vender_id == 0  && sd.trade_partner == 0)
		fail = npc_buylist(sd,(RFIFOW(fd,2)-4)/4, RFIFOP(fd,4));

	// fail=0 - success         - MsgStringTable[54]
	// fail=1 - not enough zeny - MsgStringTable[55]
	// fail=2 - overwheight     - MsgStringTable[56]
	// fail=3 - inventory full  - MsgStringTable[221]
	// fail=other - no message
	WFIFOW(fd,0)=0x00ca;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0x00ca).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcSellListSend(int fd, map_session_data &sd)
{
	int fail=1;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.vender_id == 0  && sd.trade_partner == 0)
		fail = npc_selllist(sd, (RFIFOW(fd,2)-4)/4, RFIFOP(fd,4));

	WFIFOW(fd,0)=0xcb;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet(sd.packet_ver,0xcb).len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_CreateChatRoom(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.basic_skill_check && sd.skill_check(NV_BASIC) < 4) || 
		!sd.createchat(RFIFOW(fd,4), RFIFOB(fd,6), (const char*)RFIFOP(fd,7), (const char*)RFIFOP(fd,15)) )
		sd.skill_failed(1,SF_NOCHAT);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatAddMember(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	chat_data::join(sd, RFIFOL(fd,2), (char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatRoomStatusChange(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(sd.chat)
	{	// force string termination inside the buffer
		RFIFOB(fd, 14) = 0;
		RFIFOB(fd, RFIFOW(fd,2)-1) = 0;
		sd.chat->change_status(sd, RFIFOW(fd,4), RFIFOB(fd,6),(const char*)RFIFOP(fd,7), (const char*)RFIFOP(fd,15));
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChangeChatOwner(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(sd.chat)
		sd.chat->change_owner(sd, (const char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_KickFromChat(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.chat && sd.chat->is_owner(sd) )
		sd.chat->kick((const char*)RFIFOP(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_ChatLeave(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	sd.leavechat();
	return 0;
}

/*==========================================
 * 取引要請を相手に送る
 *------------------------------------------
 */
int clif_parse_TradeRequest(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(config.basic_skill_check == 0 || sd.skill_check(NV_BASIC) >= 1){
		trade_traderequest(sd,RFIFOL(sd.fd,2));
	} else
		sd.skill_failed(1);
	return 0;
}

/*==========================================
 * 取引要請
 *------------------------------------------
 */
int clif_parse_TradeAck(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	trade_tradeack(sd,RFIFOB(sd.fd,2));
	return 0;
}

/*==========================================
 * アイテム追加
 *------------------------------------------
 */
int clif_parse_TradeAddItem(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	trade_tradeadditem(sd,RFIFOW(sd.fd,2),RFIFOL(sd.fd,4));
	return 0;
}

/*==========================================
 * アイテム追加完了(ok押し)
 *------------------------------------------
 */
int clif_parse_TradeOk(int fd, map_session_data &sd)
{
	trade_tradeok(sd);
	return 0;
}

/*==========================================
 * 取引キャンセル
 *------------------------------------------
 */
int clif_parse_TradeCancel(int fd, map_session_data &sd)
{
	trade_tradecancel(sd);
	return 0;
}

/*==========================================
 * 取引許諾(trade押し)
 *------------------------------------------
 */
int clif_parse_TradeCommit(int fd, map_session_data &sd)
{
	trade_tradecommit(sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_StopAttack(int fd, map_session_data &sd)
{
	sd.stop_attack();
	return 0;
}

/*==========================================
 * カートへアイテムを移す
 *------------------------------------------
 */
int clif_parse_PutItemToCart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_GetItemFromCart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_RemoveOption(int fd, map_session_data &sd)
{
	if( sd.is_riding() )
	{	// jobchange when removing peco [Valaris]
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
int clif_parse_ChangeCart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	pc_setcart(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ステータスアップ
 *------------------------------------------
 */
int clif_parse_StatusUp(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	pc_statusup(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * スキルレベルアップ
 *------------------------------------------
 */
int clif_parse_SkillUp(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	pc_skillup(sd,RFIFOW(fd,2));
	return 0;
}

/*==========================================
 * スキル使用（ID指定）
 *------------------------------------------
 */
int clif_parse_UseSkillToId(int fd, map_session_data &sd) {
	unsigned short skillnum, skilllv, lv;
	uint32 target_id;
	unsigned long tick = gettick();

	if( !session_isActive(fd) )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.chat || sd.vender_id != 0 || sd.is_sitting() || sd.is_dead())
		return 0;

	skilllv = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	if (skilllv < 1) skilllv = 1; //No clue, I have seen the client do this with guild skills :/ [Skotlex]
	skillnum = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);
	target_id = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[2]);


	if (skill_not_ok(skillnum, sd))
		return 0;

	if (sd.skilltimer != -1)
	{
		if (skillnum != SA_CASTCANCEL)
			return 0;
	}
	else if( (sd.has_status(SC_TRICKDEAD) && skillnum != NV_TRICKDEAD) ||
		sd.has_status(SC_BERSERK) ||
		sd.has_status(SC_NOCHAT) ||
		sd.has_status(SC_WEDDING) ||
		sd.view_class == 22)
		return 0;

	if(target_id & FLAG_DISGUISE) // for disguises [Valaris]
		target_id &= ~FLAG_DISGUISE;

	if( sd.skillitem == skillnum )
		skilllv = sd.skillitemlv;
	else if( skilllv > (lv=sd.skill_check(skillnum)) )
		skilllv = lv;

	//////////////////////////////////////////////////////////////
	// REF target entry point for the new skill code (targetskill)
	// sd.start_skill(skillnum, skilllv, target_id);

	if (DIFF_TICK(tick, sd.canact_tick) < 0 &&
		// allow monk combos to ignore this delay [celest]
		!(sd.has_status(SC_COMBO) &&
		(skillnum == MO_EXTREMITYFIST ||
		skillnum == MO_CHAINCOMBO ||
		skillnum == MO_COMBOFINISH ||
		skillnum == CH_PALMSTRIKE ||
		skillnum == CH_TIGERFIST ||
		skillnum == CH_CHAINCRUSH)))
	{
		sd.skill_failed( skillnum, SF_DELAY);
		return 0;
	}

	if (sd.invincible_timer != -1)//## TODO where to put this (start_skill?) [FlavioJS]
		pc_delinvincibletimer(sd);

	if(sd.skillitem == skillnum)
	{
		skill_use_id(&sd, target_id, skillnum, skilllv);
	}
	else
	{
		//## TODO what's happening here and where to put it [FlavioJS]
		sd.skillitem = sd.skillitemlv = 0xFFFF;
		if (skillnum == MO_EXTREMITYFIST)
		{
			if ((!sd.has_status(SC_COMBO) ||
				(sd.get_statusvalue1(SC_COMBO).integer() != MO_COMBOFINISH &&
				 sd.get_statusvalue1(SC_COMBO).integer() != CH_TIGERFIST &&
				 sd.get_statusvalue1(SC_COMBO).integer() != CH_CHAINCRUSH)) )
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
		}
		else if (skillnum == CH_TIGERFIST)
		{
			if ( !sd.has_status(SC_COMBO) || sd.get_statusvalue1(SC_COMBO).integer() != MO_COMBOFINISH)
			{
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
		if (skilllv > 0) {
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
int clif_parse_UseSkillToPos(int fd, map_session_data &sd)
{
	unsigned short skillnum, skilllv, lv, x, y;
	unsigned long tick = gettick();

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.ScriptEngine.isRunning() || sd.vender_id != 0 || sd.chat || sd.is_sitting() || sd.is_dead() )
		return 0;

	skilllv = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	skillnum = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);
	x = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[2]);
	y = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[3]);
	if (skill_not_ok(skillnum, sd))
		return 0;

	if(packet(sd.packet_ver,RFIFOW(fd,0)).pos[4] > 0)
	{
		int skillmoreinfo = packet(sd.packet_ver,RFIFOW(fd,0)).pos[4];
		if( sd.is_sitting() )
		{
			sd.skill_failed(skillnum,SF_NOSIT);
			return 0;
		}
		safestrcpy(talkie_mes, sizeof(talkie_mes), (char*)RFIFOP(fd,skillmoreinfo));
	}

	if (sd.skilltimer != -1)
		return 0;
	else if( DIFF_TICK(tick, sd.canact_tick) < 0 &&
		// allow monk combos to ignore this delay [celest]
		!( sd.has_status(SC_COMBO) &&
		 (skillnum == MO_EXTREMITYFIST || skillnum == MO_CHAINCOMBO || skillnum == MO_COMBOFINISH ||
		  skillnum == CH_PALMSTRIKE || skillnum == CH_TIGERFIST || skillnum == CH_CHAINCRUSH)) )
	{
		sd.skill_failed( skillnum, SF_DELAY);
		return 0;
	}

	if( (sd.has_status(SC_TRICKDEAD) && skillnum != NV_TRICKDEAD) ||
	    sd.has_status(SC_BERSERK) ||
		sd.has_status(SC_NOCHAT) ||
	    sd.has_status(SC_WEDDING) ||
		sd.view_class == 22)
		return 0;
	if (sd.invincible_timer != -1)
		pc_delinvincibletimer(sd);
	if(sd.skillitem == skillnum)
	{
		if (skilllv != sd.skillitemlv)
			skilllv = sd.skillitemlv;
		skill_use_pos(&sd, x, y, skillnum, skilllv);
	}
	else
	{
		sd.skillitem = sd.skillitemlv = 0xFFFF;
		if ((lv = sd.skill_check( skillnum)) > 0)
		{
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
int clif_parse_UseSkillMap(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.chat)
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0 || 
		sd.has_status(SC_TRICKDEAD) || sd.has_status(SC_BERSERK) ||
		sd.has_status(SC_NOCHAT) || sd.has_status(SC_WEDDING) ||
		sd.view_class==22)
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
int clif_parse_RequestMemo(int fd, map_session_data &sd)
{
	if( !sd.is_dead() || !sd.is_on_map() )
		pc_memo(sd,-1);
	return 0;
}
/*==========================================
 * アイテム合成
 *------------------------------------------
 */
int clif_parse_ProduceMix(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	return skill_produce_mix(sd,RFIFOW(fd,2),RFIFOW(fd,4),RFIFOW(fd,6),RFIFOW(fd,8));
}
/*==========================================
 * 武器修理
 *------------------------------------------
 */
int clif_parse_RepairItem(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	return pc_item_repair(sd, RFIFOW(fd,2));
}

int clif_parse_WeaponRefine(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	return pc_item_refine(sd, RFIFOW(fd, packet(sd.packet_ver,RFIFOW(fd,0)).pos[0])-2);
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcSelectMenu(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	sd.ScriptEngine.cExtData = RFIFOB(fd,6);
	sd.ScriptEngine.restart(RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcNextClicked(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	sd.ScriptEngine.cExtData = 0;
	sd.ScriptEngine.restart(RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcAmountInput(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	sd.ScriptEngine.cExtData = RFIFOL(fd,6);
	sd.ScriptEngine.restart(RFIFOL(fd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcStringInput(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	sd.ScriptEngine.cExtData = (char*)RFIFOP(fd,8);
	sd.ScriptEngine.restart(RFIFOL(fd,4));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_parse_NpcCloseClicked(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	sd.ScriptEngine.cExtData = 1;
	sd.ScriptEngine.restart(RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * アイテム鑑定
 *------------------------------------------
 */
int clif_parse_ItemIdentify(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	pc_item_identify(sd,RFIFOW(fd,2)-2);
	return 0;
}
/*==========================================
 * 矢作成
 *------------------------------------------
 */
int clif_parse_SelectArrow(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	return skill_arrow_create(&sd,RFIFOW(fd,2));
}
/*==========================================
 * オートスペル受信
 *------------------------------------------
 */
int clif_parse_AutoSpell(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	skill_autospell(&sd,RFIFOW(fd,2));
	return 0;
}
/*==========================================
 * カード使用
 *------------------------------------------
 */
int clif_parse_UseCard(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_InsertCard(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_SolveCharName(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	const uint32 charid = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	return map_req_namedb(sd, charid);
}

/*==========================================
 * 0197 /resetskill /resetstate
 *------------------------------------------
 */
int clif_parse_ResetChar(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) )
	{
		switch(RFIFOW(fd,2))
		{
		case 0:
			if( sd.isGM() >= CommandInfo::get_level(command_statusreset) )
				pc_resetstate(sd);
			break;
		case 1:
			if( sd.isGM() >= CommandInfo::get_level(command_skillreset) )
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
int clif_parse_LGMmessage(int fd, map_session_data &sd) {
	unsigned char buf[512];

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_localbroadcast) )
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
int clif_parse_MoveToKafra(int fd, map_session_data &sd) {
	int item_index, item_amount;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;

	item_index = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0])-2;
	item_amount = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);


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
int clif_parse_MoveFromKafra(int fd, map_session_data &sd) {
	int item_index, item_amount;

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( sd.ScriptEngine.isRunning() || sd.vender_id != 0  || sd.trade_partner != 0)
		return 0;

	item_index = RFIFOW(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0])-1;
	item_amount = RFIFOL(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[1]);

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
int clif_parse_MoveToKafraFromCart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_MoveFromKafraToCart(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_CloseKafra(int fd, map_session_data &sd)
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
int clif_parse_CreateParty(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if (config.basic_skill_check == 0 || sd.skill_check(NV_BASIC) >= 7) {
		party_create(sd,(const char*)RFIFOP(fd,2),0,0);
	} else
		sd.skill_failed(1,SF_NOPARTY);
	return 0;
}

/*==========================================
 * パーティを作る
 *------------------------------------------
 */
int clif_parse_CreateParty2(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if (config.basic_skill_check == 0 || sd.skill_check(NV_BASIC) >= 7){
		party_create(sd,(const char*)RFIFOP(fd,2),RFIFOB(fd,26),RFIFOB(fd,27));
	} else
		sd.skill_failed(1,SF_NOPARTY);
	return 0;
}

/*==========================================
 * パーティに勧誘
 *------------------------------------------
 */
int clif_parse_PartyInvite(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	party_invite(sd, RFIFOL(fd,2));
	return 0;
}

/*==========================================
 * パーティ勧誘返答
 *------------------------------------------
 */
int clif_parse_ReplyPartyInvite(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(config.basic_skill_check == 0 || sd.skill_check(NV_BASIC) >= 5){
		party_reply_invite(sd,RFIFOL(fd,2),RFIFOL(fd,6));
	} else {
		party_reply_invite(sd,RFIFOL(fd,2),-1);
		sd.skill_failed(1,SF_NOPARTY);
	}
	return 0;
}

/*==========================================
 * パーティ脱退要求
 *------------------------------------------
 */
int clif_parse_LeaveParty(int fd, map_session_data &sd)
{
	party_leave(sd);
	return 0;
}

/*==========================================
 * パーティ除名要求
 *------------------------------------------
 */
int clif_parse_RemovePartyMember(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	party_removemember(sd,RFIFOL(fd,2),(char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 * パーティ設定変更要求
 *------------------------------------------
 */
int clif_parse_PartyChangeOption(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	party_changeoption(sd, RFIFOW(fd,2), RFIFOW(fd,4));
	return 0;
}

/*==========================================
 * パーティメッセージ送信要求
 *------------------------------------------
 */
int clif_parse_PartyMessage(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	size_t buffersize = RFIFOW(fd,2);
	if( buffersize > (size_t)RFIFOREST(fd) )
	{	// some serious error
		ShowError("clif_parse_PartyMessage: size marker outside buffer %i > %i, (connection %i=%i,%p=%p)", 
			buffersize, RFIFOREST(fd), fd,sd.fd, session[fd]->user_session, &sd);
		return 0;
	}
	// add an eof marker in the buffer (really inside, not extending the used buffer)
	RFIFOB(fd,buffersize-1)=0;

	const char* message = (const char*)RFIFOP(fd,4);


	if( CommandInfo::is_command(fd, sd, message) ||
		sd.has_status(SC_BERSERK) ||	//バーサーク時は会話も不可
		sd.has_status(SC_NOCHAT) )		//チャット禁止
		return 0;

	party_send_message(sd, message, buffersize-4);
	return 0;
}

/*==========================================
 * 露店閉鎖
 *------------------------------------------
 */
int clif_parse_CloseVending(int fd, map_session_data &sd)
{
	vending_closevending(sd);
	return 0;
}

/*==========================================
 * 露店アイテムリスト要求
 *------------------------------------------
 */
int clif_parse_VendingListReq(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_PurchaseReq(int fd, map_session_data &sd)
 {
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(sd.trade_partner == 0)
		vending_purchasereq(sd, RFIFOW(fd,2), RFIFOL(fd,4), RFIFOP(fd,8));
	return 0;
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
int clif_parse_OpenVending(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(sd.vender_id == 0  && sd.trade_partner == 0)
		vending_openvending(sd, RFIFOW(fd,2), (char*)RFIFOP(fd,4), RFIFOB(fd,84), RFIFOP(fd,85));
	return 0;
}

/*==========================================
 * /monster /item
 *------------------------------------------
 */
int clif_parse_GM_Monster_Item(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	if(config.atc_gmonly == 0 || sd.isGM())
	{
		char *monster_item_name = (char*)RFIFOP(fd,2);
		monster_item_name[23] = 0;
		if(mobdb_searchname(monster_item_name) != 0)
		{
			if(sd.isGM() >= CommandInfo::get_level(command_spawn))
				command_spawn(fd, sd, "@spawn", basics::CParameterList(monster_item_name)); // as @spawn
		}
		else if(itemdb_searchname(monster_item_name) != NULL)
		{
			if(sd.isGM() >= CommandInfo::get_level(command_item))
				command_item(fd, sd, "@item", basics::CParameterList(monster_item_name)); // as @item
		}

	}
	return 0;
}

/*==========================================
 * ギルドを作る
 *------------------------------------------
 */
int clif_parse_CreateGuild(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	guild_create(sd, (char*)RFIFOP(fd,6));
	return 0;
}

/*==========================================
 * ギルドマスターかどうか確認
 *------------------------------------------
 */
int clif_parse_GuildCheckMaster(int fd, map_session_data &sd)
{
	clif_guild_masterormember(sd);
	return 0;
}

/*==========================================
 * ギルド情報要求
 *------------------------------------------
 */
int clif_parse_GuildRequestInfo(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_GuildChangePositionInfo(int fd, map_session_data &sd)
{
	int i;
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_GuildChangeMemberPosition(int fd, map_session_data &sd)
{
	int i;
	if( !session_isActive(fd) || !sd.is_on_map() )
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
int clif_parse_GuildRequestEmblem(int fd, map_session_data &sd)
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
int clif_parse_GuildChangeEmblem(int fd, map_session_data &sd)
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
int clif_parse_GuildChangeNotice(int fd, map_session_data &sd)
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
int clif_parse_GuildInvite(int fd, map_session_data &sd)
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
int clif_parse_GuildReplyInvite(int fd, map_session_data &sd)
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
int clif_parse_GuildLeave(int fd, map_session_data &sd)
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
int clif_parse_GuildExplusion(int fd, map_session_data &sd)
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
int clif_parse_GuildMessage(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	size_t buffersize = RFIFOW(fd,2);
	if( buffersize > (size_t)RFIFOREST(fd) )
	{	// some serious error
		ShowError("clif_parse_GuildMessage: size marker outside buffer %i > %i, (connection %i=%i,%p=%p)", 
			buffersize, RFIFOREST(fd), fd,sd.fd, session[fd]->user_session, &sd);
		return 0;
	}
	// add an eof marker in the buffer (really inside, not extending the used buffer)
	RFIFOB(fd,buffersize-1)=0;

	const char* message = (const char*)RFIFOP(fd,4);


	if( CommandInfo::is_command(fd, sd, message) ||
		sd.has_status(SC_BERSERK) ||	//バーサーク時は会話も不可
		sd.has_status(SC_NOCHAT) )		//チャット禁止
		return 0;

	guild_send_message(sd, message, buffersize-4);
	return 0;
}

/*==========================================
 * ギルド同盟要求
 *------------------------------------------
 */
int clif_parse_GuildRequestAlliance(int fd, map_session_data &sd)
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
int clif_parse_GuildReplyAlliance(int fd, map_session_data &sd)
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
int clif_parse_GuildDelAlliance(int fd, map_session_data &sd)
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
int clif_parse_GuildOpposition(int fd, map_session_data &sd)
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
int clif_parse_GuildBreak(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	guild_break(sd,(char*)RFIFOP(fd,2));
	return 0;
}

// pet
int clif_parse_PetMenu(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.pd) sd.pd->menu(RFIFOB(fd,2));
	return 0;
}

int clif_parse_CatchPet(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	pet_catch_process2(sd,RFIFOL(fd,2));
	return 0;
}

int clif_parse_SelectEgg(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	pet_select_egg(sd,RFIFOW(fd,2)-2);
	return 0;
}

int clif_parse_SendEmotion(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.pd)
		clif_pet_emotion(*sd.pd,RFIFOL(fd,2));
	return 0;
}

int clif_parse_ChangePetName(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(sd.pd) sd.pd->change_name( (const char*)RFIFOP(fd,2) );
	return 0;
}

// Kick (right click menu for GM "(name) force to quit")
int clif_parse_GMKick(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) && sd.isGM() >= CommandInfo::get_level(command_kick) )
	{
		map_session_data *tsd;
		mob_data *tmd;
		int tid = RFIFOL(fd,2);

		if( (tsd = map_session_data::from_blid(tid)) )
		{
			if( sd.isGM() > tsd->isGM() )
				clif_GM_kick(sd, *tsd, 1);
			else
				clif_GM_kickack(sd, 0);
		}
		else if( (tmd = mob_data::from_blid(tid)) )
		{
			sd.state.attack_type = 0;
			mob_damage(*tmd, tmd->hp, 2, &sd);
		}
		else
			clif_GM_kickack(sd, 0);
	}
	return 0;
}

/*==========================================
 * /shift
 *------------------------------------------
 */
int clif_parse_Shift(int fd, map_session_data &sd)
{
	char player_name[32]="";

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_jumpto) )
	{
		safestrcpy(player_name, sizeof(player_name), (char*)RFIFOP(fd,2));
		command_jumpto(fd, sd, "@jumpto", basics::CParameterList(player_name)); // as @jumpto
	}

	return 0;
}

/*==========================================
 * /recall
 *------------------------------------------
 */
int clif_parse_Recall(int fd, map_session_data &sd)
{
	char player_name[32];

	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if( (config.atc_gmonly == 0 || sd.isGM()) &&
		sd.isGM() >= CommandInfo::get_level(command_recall) )
	{
		safestrcpy(player_name, sizeof(player_name), (char*)RFIFOP(fd,2));
		command_recall(fd, sd, "@recall", basics::CParameterList(player_name)); // as @recall
	}

	return 0;
}

int clif_parse_GMHide(int fd, map_session_data &sd)
{
	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_hide) )
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
int clif_parse_GMReqNoChat(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	if(!config.muting_players)
	{
		clif_displaymessage(fd, "Muting is disabled.");
		return 0;
	}


	ushort cmd = RFIFOW(fd,0);
	uint32 tid = RFIFOL(fd,packet(sd.packet_ver,cmd).pos[0]);
	int type   = RFIFOB(fd,packet(sd.packet_ver,cmd).pos[1]);
	int limit  = RFIFOW(fd,packet(sd.packet_ver,cmd).pos[2]);
	map_session_data *dstsd = map_session_data::from_blid(tid);

	if( dstsd && sd.block_list::id != dstsd->block_list::id &&
		sd.isGM()>dstsd->isGM() && sd.isGM() >= CommandInfo::get_level(command_mute) )
	{
		int dstfd = dstsd->fd;
		if( session_isActive(dstfd) )
		{
			WFIFOW(dstfd,0)=0x14b;
			WFIFOB(dstfd,2)=(type==2)?1:type;
			memcpy(WFIFOP(dstfd,3),sd.status.name,24);
			WFIFOSET(dstfd,packet(sd.packet_ver,0x14b).len);
		}

		dstsd->status.manner -= (type == 0)?-limit:+limit;

		if(dstsd->status.manner < 0)
			status_change_start(dstsd,SC_NOCHAT,0,0,0,0,0,0);
		else
		{
			dstsd->status.manner = 0;
			status_change_end(dstsd,SC_NOCHAT,-1);
		}
		ShowMessage("name:%s type:%d limit:%d manner:%d\n",dstsd->status.name,type,limit,dstsd->status.manner);
	}
	return 0;
}
/*==========================================
 * GMによるチャット禁止時間参照（？）
 *------------------------------------------
 */
int clif_parse_GMReqNoChatCount(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	int tid = RFIFOL(fd,2);

	WFIFOW(fd,0) = 0x1e0;
	WFIFOL(fd,2) = tid;
	snprintf((char*)WFIFOP(fd,6),24,"%d",tid);
//	memcpy(WFIFOP(fd,6), "TESTNAME", 24);
	WFIFOSET(fd, packet(sd.packet_ver,0x1e0).len);

	return 0;
}

int clif_parse_PMIgnore(int fd, map_session_data &sd)
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
		WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
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
			for(i=0; i<MAX_IGNORE_LIST; ++i)
			{
				if( strcmp(sd.ignore[i].name, nick) == 0 ||
					sd.ignore[i].name[0] == '\0' )
					break;
			}
			if( i<MAX_IGNORE_LIST && sd.ignore[i].name[0] != '\0' )
			{	// failed, found it
				WFIFOB(fd,3) = 1; // fail
				WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
				clif_wis_message(fd, wisp_server_name, "This player is already blocked.", strlen("This player is already blocked.") + 1);
				if (strcmp(wisp_server_name, nick) == 0)
				{	// to find possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %lu) has tried AGAIN to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
					intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
				}
			}
			else if( i>=MAX_IGNORE_LIST )
			{	// failed, list full
				WFIFOB(fd,3) = 1; 
				WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
				clif_wis_message(fd, wisp_server_name, "You can not block more people.", strlen("You can not block more people.") + 1);
				if (strcmp(wisp_server_name, nick) == 0) { // to found possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %lu) has tried to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
					intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
				}
			}
			else
			{	// success
				memcpy(sd.ignore[i].name, nick, 24);
				WFIFOB(fd,3) = 0; 
				WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
				if (strcmp(wisp_server_name, nick) == 0)
				{	// to find possible bot users who automaticaly ignore people.
					snprintf(output, sizeof(output), "Character '%s' (account: %lu) has tried to block wisps from '%s' (wisp name of the server). Bot user?", sd.status.name, (unsigned long)sd.status.account_id, wisp_server_name);
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
				WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
				clif_wis_message(fd, wisp_server_name, "This player is not blocked by you.", strlen("This player is not blocked by you.") + 1);
			}
			else
			{	// move elements from i+1...MAX_IGNORE_LIST to i and clear the last
				memmove(sd.ignore+i, sd.ignore+i+1, (MAX_IGNORE_LIST-i-1)*sizeof(sd.ignore[0]));
				sd.ignore[MAX_IGNORE_LIST-1].name[0]=0;
				WFIFOB(fd,3) = 0; // success
				WFIFOSET(fd, packet(sd.packet_ver,0x0d1).len);
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

int clif_parse_PMIgnoreAll(int fd, map_session_data &sd)
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
			WFIFOSET(fd, packet(sd.packet_ver,0x0d2).len);
		} else {
			WFIFOB(fd,3) = 1; // fail
			WFIFOSET(fd, packet(sd.packet_ver,0x0d2).len);
			clif_wis_message(fd, wisp_server_name, "You already block everyone.", strlen("You already block everyone.") + 1);
		}
	} else {
		WFIFOW(fd,0) = 0x0d2; // R 00d2 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
		WFIFOB(fd,2) = 1;
		if(sd.state.ignoreAll == 1) {
			sd.state.ignoreAll = 0;
			WFIFOB(fd,3) = 0; // success
			WFIFOSET(fd, packet(sd.packet_ver,0x0d2).len);
		} else {
			WFIFOB(fd,3) = 1; // fail
			WFIFOSET(fd, packet(sd.packet_ver,0x0d2).len);
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
int clif_parse_PMIgnoreList(int fd, map_session_data &sd)
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
int clif_parse_NoviceDoriDori(int fd, map_session_data &sd)
{
	sd.doridori_counter = 1;
	return 0;
}
/*==========================================
 * スパノビの爆裂波動
 *------------------------------------------
 */
int clif_parse_NoviceExplosionSpirits(int fd, map_session_data &sd)
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
		status_change_start(&sd,(status_t)SkillStatusChangeTable[MO_EXPLOSIONSPIRITS],5,0,0,0,skill_get_time(MO_EXPLOSIONSPIRITS,5),0 );
		}
	return 0;
	}

// random notes:
// 0x214: forging chance?

/*==========================================
 * Friends List
 *------------------------------------------
 */
int clif_friendslist_send(map_session_data &sd)
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
			WFIFOL(sd.fd, n+0) = (map_session_data::charid2sd(sd.status.friendlist[i].friend_id) != NULL);
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
int clif_friendslist_reqack(map_session_data &sd, char *name, unsigned short type)
{
	int fd;

	fd = sd.fd;
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x209;
	WFIFOW(fd,2) = type;
	if (type != 2)
		memcpy(WFIFOP(fd, 12), name, 24);
	WFIFOSET(fd, packet(sd.packet_ver,0x209).len);
	return 0;
}

int clif_parse_FriendsListAdd(int fd, map_session_data &sd)
{
	map_session_data *f_sd;
	size_t i, count;
	int f_fd;

	if( !session_isActive(fd) )
		return 0;

	f_sd = map_session_data::nick2sd((char*)RFIFOP(fd,2));

	if(f_sd == NULL)
	{	// Friend doesn't exist (no player with this name)
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND));
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
	WFIFOSET(f_fd, packet(sd.packet_ver,0x207).len);
}
	return 0;
}

int clif_parse_FriendsListReply(int fd, map_session_data &sd)
{	//<W: id> <L: Player 1 chara ID> <L: Player 1 AID> <B: Response>
	map_session_data *f_sd;
	uint32 id;
	char reply;

	if( !session_isActive(fd) )
		return 0;

//	uint32 char_id = RFIFOL(fd,2);
	id = RFIFOL(fd,6);
	reply = RFIFOB(fd,10);
//	ShowMessage ("reply: %d %d %d\n", char_id, id, reply);

	f_sd = map_session_data::from_blid(id);
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

int clif_parse_FriendsListRemove(int fd, map_session_data &sd)
{	// 0x203 </o> <ID to be removed W 4B>
	int i, j;

	if( !session_isActive(fd) )
		return 0;

	uint32 id = RFIFOL(fd,6);
	map_session_data *f_sd = map_session_data::charid2sd(id);

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
			WFIFOSET(fd, packet(sd.packet_ver,0x20a).len);
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
int clif_parse_GMKillAll(int fd, map_session_data &sd)
{
	
	if( (config.atc_gmonly == 0 || sd.isGM()) &&
	    sd.isGM() >= CommandInfo::get_level(command_kickall) )
	{
		command_kickall(fd, sd, "kickall", basics::CParameterList());
	}
	return 0;
}

/*==========================================
 * /pvpinfo
 *------------------------------------------
 */
int clif_parse_PVPInfo(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	WFIFOW(fd,0) = 0x210;
	//WFIFOL(fd,2) = 0;	// not sure what for yet
	//WFIFOL(fd,6) = 0;
	WFIFOL(fd,10) = sd.pvp_won;	// times won
	WFIFOL(fd,14) = sd.pvp_lost;	// times lost
	WFIFOL(fd,18) = sd.pvp_point;
	WFIFOSET(fd, packet(sd.packet_ver,0x210).len);

	return 0;
}

/*==========================================
 * /blacksmith
 *------------------------------------------
 */
int clif_parse_Blacksmith(int fd, map_session_data &sd)
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

	WFIFOSET(fd, packet(sd.packet_ver,0x219).len);
	return 0;
}

/*==========================================
 * /alchemist
 *------------------------------------------
 */
int clif_parse_Alchemist(int fd, map_session_data &sd)
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

	WFIFOSET(fd, packet(sd.packet_ver,0x21a).len);
	return 0;
}

/*==========================================
 * /taekwon?
 *------------------------------------------
 */
int clif_parse_Taekwon(int fd, map_session_data &sd)
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
	WFIFOSET(fd, packet(sd.packet_ver,0x226).len);
	return 0;
}

/*==========================================
 * PK Ranking table?
 *------------------------------------------
 */
int clif_parse_RankingPk(int fd, map_session_data &sd)
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

	WFIFOSET(fd, packet(sd.packet_ver,0x238).len);
	return 0;
}





/*==========================================
 * メール送信→Interへ
 *------------------------------------------
 */
int clif_parse_SendMail(int fd, map_session_data &sd)
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
	map_session_data *rd = map_session_data::nick2sd(targetname);
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
int clif_parse_ReadMail(int fd, map_session_data &sd)
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
int clif_parse_MailGetAppend(int fd, map_session_data &sd)
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
int clif_parse_MailWinOpen(int fd, map_session_data &sd)
{
	int flag;
	unsigned short cmd	= RFIFOW(fd,0);

	if( !session_isActive(fd) )
		return 0;

	flag = RFIFOL(fd, packet(sd.packet_ver,cmd).pos[0]);
	chrif_mail_removeitem(sd, (flag==0)?3:flag );
	return 0;
}
/*==========================================
 * メールBOXの更新要求
 *------------------------------------------
 */
int clif_parse_RefreshMailBox(int fd, map_session_data &sd)
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
int clif_parse_SendMailSetAppend(int fd, map_session_data &sd)
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
int clif_parse_DeleteMail(int fd, map_session_data &sd)
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
int clif_parse_HomMenu(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.hd )
		return 0;
	unsigned short cmd	= RFIFOW(fd,0);
	sd.hd->menu(RFIFOB(fd,packet(sd.packet_ver,cmd).pos[0]));
	return 0;
}
int clif_parse_HomWalkMaster(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.hd )
		return 0;

	uint32 id = RFIFOL(fd,2);
	if( id != sd.status.homun_id )
		printf("clif_parse_HomWalkMaster: %lu %lu", (ulong)id, (ulong)sd.status.homun_id);

	sd.hd->return_to_master();
	return 0;
}
int clif_parse_HomWalkToXY(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if( sd.hd )
	{
		const unsigned short cmd = RFIFOW(fd,0);
		uint32 id = RFIFOL(fd,2);
		if( id != sd.hd->status.homun_id )
			printf("clif_parse_HomWalkToXY: %lu %lu", (ulong)id, (ulong)sd.hd->status.homun_id);

		const uchar v0 = RFIFOB(fd,0+packet(sd.packet_ver,cmd).pos[0]);
		const uchar v1 = RFIFOB(fd,1+packet(sd.packet_ver,cmd).pos[0]);
		const uchar v2 = RFIFOB(fd,2+packet(sd.packet_ver,cmd).pos[0]);

		const int x = ( v0      <<2) | (v1>>6);	// inverse to position encoding
		const int y = ((v1&0x3f)<<4) | (v2>>4);

		sd.hd->walktoxy(x,y);
	}
	return 0;
}
int clif_parse_HomActionRequest(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;

	if(sd.hd)
	{
		if( sd.hd->is_dead() )
		{
			clif_clearchar_area(*sd.hd,1);
			return 0;
		}
		if(sd.hd->status.option&2)
			return 0;

		const unsigned short cmd = RFIFOW(fd,0);

		uint32 id = RFIFOL(fd,2);

		if( id != sd.hd->status.homun_id )
			printf("clif_parse_HomActionRequest: %lu %lu", (ulong)id, (ulong)sd.hd->status.homun_id);

		uint32 target_id = RFIFOL(fd,packet(sd.packet_ver,cmd).pos[0]);	// pos 6
		int action_type = RFIFOB(fd,packet(sd.packet_ver,cmd).pos[1]);	// pos 10 -> len 11
		
		// decode for jRO 2005-05-09dRagexe
		if( packet(sd.packet_ver,cmd).pos[0]==0 )
		{
			int packet_len = packet(sd.packet_ver,cmd).len;
			int t1[]={ 88, 37 }, t2[]={ 80, 4 };
			int pos = ( ( packet_len - t1[packet_len&1] ) >> 1 ) + t2[packet_len&1];
			target_id = RFIFOL(fd,pos);
		}

		sd.hd->stop_walking(1);
		sd.hd->stop_attack();
		
		// end decode
		switch(action_type)
		{
		case 0x00:	// once attack
		case 0x07:	// continuous attack
			{
				block_list*bl=block_list::from_blid(target_id);
				if(bl && !mob_gvmobcheck(sd,*bl))
					return 0;
				sd.hd->start_attack(target_id, action_type!=0);
			}
			break;
		}
	}
	return 0;
}
int clif_parse_ChangeHomName(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) )
		return 0;
	if(sd.hd) sd.hd->change_name( (const char*)RFIFOP(fd,packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]));
	return 0;
}

/*==========================================
 *	養子ターゲット表示
 *------------------------------------------
 */
int clif_baby_target_display(map_session_data &sd)
{
	if( !session_isActive(sd.fd) )
		return 0;

	unsigned char buf[2];
	WBUFW(buf,0) = 0x01f8;
	WFIFOSET(sd.fd,packet(sd.packet_ver,0x1f8).len);

	return 0;
}
/*==========================================
 * 養子要求
 *------------------------------------------
 */
int clif_parse_BabyRequest(int fd, map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	uint32 char_id = RFIFOL(fd, packet(sd.packet_ver,RFIFOW(fd,0)).pos[0]);
	map_session_data* tsd = map_session_data::from_blid(char_id);
	if(tsd)
		clif_baby_target_display(*tsd);
	return 0;
}


/*==========================================
 * SG Feel save OK [Komurka]
 *------------------------------------------
 */
int clif_request_feel(map_session_data &sd, unsigned short skilllv)
{
	if( !session_isActive(sd.fd) )
		return 0;
	pc_setglobalreg(sd, "PC_SG_FEEL", skilllv);
	WFIFOW(sd.fd,0)=0x253;
	WFIFOSET(sd.fd, packet(sd.packet_ver,0x253).len);

	return 0;
}

int clif_parse_FeelSaveOk(int fd,map_session_data &sd)
{
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;

	unsigned short i = pc_readglobalreg(sd,"PC_SG_FEEL");
	pc_setglobalreg(sd, "PC_SG_FEEL", 0);
	if(i<1 || i > 3)
		return 0; //Bug?

	static const char *feel_var[3] = {"PC_FEEL_SUN","PC_FEEL_MOON","PC_FEEL_STAR"};
	pc_setglobalreg(sd, feel_var[i-1], sd.block_list::m );

	WFIFOW(fd,0)=0x20e;
	mapname2buffer(WFIFOP(fd,2), 16, maps[sd.block_list::m].mapname);
	WFIFOL(fd,26)=sd.block_list::id;
	WFIFOW(fd,30)=i-1;
	WFIFOSET(fd, packet(sd.packet_ver,0x20e).len);
	
	clif_skill_nodamage(sd,sd,SG_FEEL,i,1);
	return 0;
}

int clif_parse_AdoptRequest(int fd, map_session_data &sd)
{
	//TODO: add somewhere the adopt code, checks for exploits, etc, etc.
	//Missing packets are the client's reply packets to the adopt request one. 
	//[Skotlex]
	if( !session_isActive(fd) || !sd.is_on_map() )
		return 0;
	
	int account_id = RFIFOL(fd,2);
	map_session_data *sd2 = map_session_data::from_blid(account_id);
	if( sd2 && session_isActive(sd2->fd) && 
		sd2->block_list::id != sd.block_list::id && 
		sd2->status.party_id == sd.status.party_id)
	{	//FIXME: No checks whatsoever are in place yet!
		WFIFOW(sd2->fd,0)=0x1f9;
		WFIFOSET(sd2->fd, packet(sd.packet_ver,0x1f9).len);
	}
	return 0;
}
/*==========================================
 * パケットデバッグ
 *------------------------------------------
 */
int clif_parse_debug(int fd, map_session_data &sd)
{
	int i, cmd;

	if( !session_isActive(fd) )
		return 0;

	cmd = RFIFOW(fd,0);

	ShowMessage("packet debug 0x%4X\n",cmd);
//	ShowMessage("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i=0;i<packet(sd.packet_ver,cmd).len; ++i){
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
int clif_parse_clientsetting(int fd, map_session_data &sd)
{
	// RFIFOB(fd,0)	effectを切ってるかどうか
	return 0;
}

int clif_parse_dummy(int fd, map_session_data &sd)
{
	int i, cmd;

	if( !session_isActive(fd) )
		return 0;

	cmd = RFIFOW(fd,0);

	ShowMessage("unimplemented packet debug 0x%4X\n",cmd);
//	ShowMessage("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i=0;i<packet(sd.packet_ver,cmd).len; ++i){
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
		map_session_data *sd = (map_session_data*)session[fd]->user_session;
		if( sd!=NULL )
		{
			if( sd->state.event_disconnect )
				npc_data::event("OnDisconnect", *sd);

			clif_clearchar(*sd, 0);
			if(sd->state.auth) 
			{	// the function doesn't send to inter-server/char-server ifit is not connected [Yor]
				sd->map_quit();
				if(sd->status.name != NULL)
					ShowInfo("%sCharacter '"CL_WHITE"%s"CL_RESET"' logged off.\n", (sd->isGM())?"GM ":"",sd->status.name); // Player logout display [Valaris]
				else
					ShowInfo("%sCharacter with Account ID '"CL_WHITE"%d"CL_RESET"' logged off.\n", (sd->isGM())?"GM ":"", sd->block_list::id); // Player logout display [Yor]
			}
			else 
			{	// not authentified! (refused by char-server or disconnect before to be authentified)
				ShowInfo("Player not authenticated with Account ID '"CL_WHITE"%d"CL_RESET"' logged off.\n", sd->block_list::id); // Player logout display [Yor]
				sd->unregister_id(); // account_id has been included in the DB before auth answer [Yor]
			} 
			chrif_char_offline(*sd);

			sd->freeblock();
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

///////////////////////////////////////////////////////////////////////////////
/// Packets that are unused/unknown/under investigation
#if 0

///////////////////////////////////////////////////////////////////////////////
/// what is truncated at length 5000 client-side.
/// The string in the packet doesn't need to be null terminated.
/// Displays in the game console:
///		=========== HUNTING LIST =============
///		<what>
///		========================================
int clif_027A(int fd, const char* what)
{
	size_t len = strlen(what);
	WFIFOW(fd,0) = 0x27a;
	WFIFOW(fd,2) = 4+len;
	memcpy(WFIFOP(fd,4), what, len);
	WFIFOSET(fd,packet(sd.packet_ver,0x27a).len);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Displays in the game console:
///		"             STR=%3d      AGI=%3d      VIT=%3d      INT=%3d      DEX=%3d      LUK=%d"
///		"standard STR=%3d      AGI=%3d      VIT=%3d      INT=%3d      DEX=%3d      LUK=%2d"
///		"    attPower=%3d    refiningPow=%3d    MAXmatPow=%3d    MINmatPower=%3d       ASPD=%3d"
///		"itemdefPow=%3d     plusdefPow=%3d    mdefPower=%3d    plusmdefPow=%3d    plusASPD=%3d"
///		"hitSuccessVal=%3d    avoidSuccessVal=%3d    plusAvoidSuccessValue=%3d"
int clif_0214(int fd, const char* what)
{
	WFIFOW(fd,0) = 0x214;
	WFIFOB(fd,2) = 0; // STR
	WFIFOB(fd,3) = 0; // standard STR
	WFIFOB(fd,4) = 0; // AGI
	WFIFOB(fd,5) = 0; // standard AGI
	WFIFOB(fd,6) = 0; // VIT
	WFIFOB(fd,7) = 0; // standard VIT
	WFIFOB(fd,8) = 0; // INT
	WFIFOB(fd,9) = 0; // standard INT
	WFIFOB(fd,10) = 0; // DEX
	WFIFOB(fd,11) = 0; // standard DEX
	WFIFOB(fd,12) = 0; // LUK
	WFIFOB(fd,13) = 0; // standard LUK
	WFIFOW(fd,14) = 0; // attPower
	WFIFOW(fd,16) = 0; // refiningPow
	WFIFOW(fd,18) = 0; // MAXmatPow
	WFIFOW(fd,20) = 0; // MINmatPower
	WFIFOW(fd,22) = 0; // itemdefPow
	WFIFOW(fd,24) = 0; // plusdefPow
	WFIFOW(fd,26) = 0; // mdefPower
	WFIFOW(fd,28) = 0; // plusmdefPow
	WFIFOW(fd,30) = 0; // hitSuccessVal
	WFIFOW(fd,32) = 0; // avoidSuccessVal
	WFIFOW(fd,34) = 0; // plusAvoidSuccessValue
	WFIFOW(fd,36) = 0; // ???
	WFIFOW(fd,38) = 0; // ASPD
	WFIFOW(fd,40) = 0; // plusASPD
	WFIFOSET(fd,packet(sd.packet_ver,0x214).len);
	return 0;
}

#endif

/*==========================================
 * クライアントからのパケット解析
 * socket.cのdo_parsepacketから呼び出される
 *------------------------------------------
 */
int clif_parse(int fd)
{
	int packet_len = 0, packet_ver;
	unsigned short cmd;
	map_session_data *sd;

	if( !session_isValid(fd) )
		return 0;

	sd = (map_session_data*)session[fd]->user_session;

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
				(!sd && packet(packet_ver,cmd).func != clif_parse_WantToConnection) || // should not happen

				// check client version
				(config.packet_ver_flag &&
				((packet_ver <=  5 && (config.packet_ver_flag & 0x00000001) == 0) ||
				 (packet_ver ==  6 && (config.packet_ver_flag & 0x00000002) == 0) ||	
				 (packet_ver ==  7 && (config.packet_ver_flag & 0x00000004) == 0) ||	
				 (packet_ver ==  8 && (config.packet_ver_flag & 0x00000008) == 0) ||	
				 (packet_ver ==  9 && (config.packet_ver_flag & 0x00000010) == 0) ||
				 (packet_ver == 10 && (config.packet_ver_flag & 0x00000020) == 0) ||
				 (packet_ver == 11 && (config.packet_ver_flag & 0x00000040) == 0) ||
				 (packet_ver == 12 && (config.packet_ver_flag & 0x00000080) == 0) ||
				 (packet_ver == 13 && (config.packet_ver_flag & 0x00000100) == 0) ||
				 (packet_ver == 14 && (config.packet_ver_flag & 0x00000200) == 0) ||
				 (packet_ver == 15 && (config.packet_ver_flag & 0x00000400) == 0) ||
				 (packet_ver == 16 && (config.packet_ver_flag & 0x00000800) == 0) ||
				 (packet_ver == 17 && (config.packet_ver_flag & 0x00001000) == 0) ||
				 (packet_ver == 18 && (config.packet_ver_flag & 0x00002000) == 0) ||
				 (packet_ver == 19 && (config.packet_ver_flag & 0x00004000) == 0) ||
				 (packet_ver == 20 && (config.packet_ver_flag & 0x00008000) == 0) ||
				 (packet_ver == 21 && (config.packet_ver_flag & 0x00010000) == 0) ||
				 (packet_ver == 22 && (config.packet_ver_flag & 0x00020000) == 0) ||
				 (packet_ver == 23 && (config.packet_ver_flag & 0x00040000) == 0) ||
				 (packet_ver == 24 && (config.packet_ver_flag & 0x00080000) == 0)) ) )
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
		if(cmd > MAX_PACKET_DB || packet_ver>MAX_PACKET_VER || packet(packet_ver,cmd).len == 0)
		{	// packet is not inside these values: session is incorrect?? or auth packet is unknown
			ShowMessage("clif_parse: session #%d, packet 0x%x ver. %i (%d bytes received) -> disconnected (unknown command).\n", fd, cmd, packet_ver, RFIFOREST(fd));
			session_Remove(fd);
			return 0;
		}
		
		// パケット長を計算
		packet_len = packet(packet_ver,cmd).len;
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
		else if (packet(packet_ver,cmd).func)
		{	// packet version 5-6-7 use same functions, but size are different
			// パケット処理

			if( sd && packet(packet_ver,cmd).func==clif_parse_WantToConnection )
			{
				if (config.error_log)
					ShowMessage("clif_parse_WantToConnection : invalid request?\n");
			}
			else
			{	
				if( packet(packet_ver,cmd).func==clif_parse_WantToConnection )
				{	// have a dummy session data to call with WantToConnection
					static map_session_data dummy(0,0,0,0,0,0,0,0);
					dummy.packet_ver = packet_ver;
					dummy.fd = fd;
					clif_parse_WantToConnection(fd, dummy);
					// we come out of WantToConnection and have a valid sd for the next run or NULL if not
					sd = (session[fd])?((map_session_data *)session[fd]->user_session):NULL;
				}
				else if(sd)
					packet(packet_ver,cmd).func(fd, *sd);
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
	server_mob_id     = npc_get_new_npc_id();
	
	packetdb_readdb();
	set_defaultparse(clif_parse);

	add_timer_func_list(clif_clearchar_delay_sub, "clif_clearchar_delay_sub");
	return 0;
}




