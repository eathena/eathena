// $Id: mmo.h,v 1.3 2004/09/25 20:12:25 PoW Exp $
// Original : mmo.h 2003/03/14 12:07:02 Rev.1.7

#ifndef	_MMO_H_
#define	_MMO_H_

#include "base.h"
#include "socket.h"
#include "utils.h"

	
#define FIFOSIZE_SERVERLINK	128*1024

// set to 0 to not check IP of player between each server.
// set to another value if you want to check (1)
#define CMP_AUTHFIFO_IP 1
#define CMP_AUTHFIFO_LOGIN2 1


#define MAX_MAP_PER_SERVER 1024
#define MAX_INVENTORY 100
#define MAX_ITEMS 20000
#define MAX_EQUIP 11
#define MAX_AMOUNT 30000
#define MAX_ZENY 1000000000	// 1G zeny
#define MAX_FAME 1000000000 // 1G fame point [DracoRPG]
#define MAX_FAMELIST 10
#define MAX_CART 100
#define MAX_SKILL 650
#define MAX_REFINE 10
#define MAX_REFINE_BONUS 5
#define MAX_MEMO 10
#define MAX_GUARDIAN 8
#define GLOBAL_REG_NUM 96
#define ACCOUNT_REG_NUM 32
#define ACCOUNT_REG2_NUM 32
#define DEFAULT_WALK_SPEED 150
#define MIN_WALK_SPEED 0
#define MAX_WALK_SPEED 1000
#define MAX_STORAGE 300
#define MAX_GUILD_STORAGE 1000
#define MAX_PARTY 12
#define MAX_GUILD 16+10*6	// increased max guild members to accomodate for +6 increase for extension levels [Lupus]
#define MAX_GUILDPOSITION 20	// increased max guild positions to accomodate for all members [Valaris] (removed) [PoW]
#define MAX_GUILDEXPLUSION 32
#define MAX_GUILDALLIANCE 16
#define MAX_GUILDSKILL	15 // increased max guild skills because of new skills [Sara-chan]
#define MAX_GUILDCASTLE 24	// increased to include novice castles [Valaris]
#define MAX_GUILDLEVEL 50
#define MAX_FRIENDLIST 40

#define MIN_HAIR_STYLE battle_config.min_hair_style
#define MAX_HAIR_STYLE battle_config.max_hair_style
#define MIN_HAIR_COLOR battle_config.min_hair_color
#define MAX_HAIR_COLOR battle_config.max_hair_color
#define MIN_CLOTH_COLOR battle_config.min_cloth_color
#define MAX_CLOTH_COLOR battle_config.max_cloth_color

// for produce
#define MIN_ATTRIBUTE 0
#define MAX_ATTRIBUTE 4
#define ATTRIBUTE_NORMAL 0
#define MIN_STAR 0
#define MAX_STAR 3

#define MIN_PORTAL_MEMO 0
#define MAX_PORTAL_MEMO 2

#define MAX_STATUS_TYPE 5

#define WEDDING_RING_M 2634
#define WEDDING_RING_F 2635

#define CHAR_CONF_NAME  "conf/char_athena.conf"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

enum
{
	GBI_EXP			=1,		// ギルドのEXP
	GBI_GUILDLV		=2,		// ギルドのLv
	GBI_SKILLPOINT	=3,		// ギルドのスキルポイント
	GBI_SKILLLV		=4,		// ギルドスキルLv

	GMI_POSITION	=0,		// メンバーの役職変更
	GMI_EXP			=1,		// メンバーのEXP

};

enum
{
	GD_SKILLBASE=10000,
	GD_APPROVAL=10000,
	GD_KAFRACONTRACT=10001,
	GD_GUARDIANRESEARCH=10002,
	GD_CHARISMA=10003,
	GD_GUARDUP=10003,
	GD_EXTENSION=10004,
	GD_GLORYGUILD=10005,
	GD_LEADERSHIP=10006,
	GD_GLORYWOUNDS=10007,
	GD_SOULCOLD=10008,
	GD_HAWKEYES=10009,
	GD_BATTLEORDER=10010,
	GD_REGENERATION=10011,
	GD_RESTORE=10012,
	GD_EMERGENCYCALL=10013,
	GD_DEVELOPMENT=10014,
};


struct mmo_server {
public:
	int fd;
	ipset address;
	size_t users;
};



/////////////////////////////////////////////////////////////////////////////
// char server definition for incoming map server connections

// currently only lanip/lanport are used for storing the ip/port 
// which are sent in connect packet
// maybe it is not necessary to store extra lan/wan ip
// since there is already the client ip in session_data

struct mmo_map_server : public mmo_server
{
public:
	uint32	maps;
	char map[MAX_MAP_PER_SERVER][16];
};

/////////////////////////////////////////////////////////////////////////////
// login server definition for incoming char server connections
struct mmo_char_server : public mmo_server
{
public:
	char name[20];
	unsigned short maintenance;
	unsigned short new_display;
};





/////////////////////////////////////////////////////////////////////////////
// simplified buffer functions with moving buffer pointer 
// to enable secure continous inserting of data to a buffer
// no pointer checking is implemented here, so make sure the calls are correct
/////////////////////////////////////////////////////////////////////////////

extern inline void _L_tobuffer(const uint32 &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0x000000FF)        );
	*buf++ = (uchar)((valin & 0x0000FF00)>> 0x08 );
	*buf++ = (uchar)((valin & 0x00FF0000)>> 0x10 );
	*buf++ = (uchar)((valin & 0xFF000000)>> 0x18 );
}
extern inline void _L_tobuffer(const sint32 &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0x000000FF)        );
	*buf++ = (uchar)((valin & 0x0000FF00)>> 0x08 );
	*buf++ = (uchar)((valin & 0x00FF0000)>> 0x10 );
	*buf++ = (uchar)((valin & 0xFF000000)>> 0x18 );
}

extern inline void _W_tobuffer(const unsigned short &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0x00FF)        );
	*buf++ = (uchar)((valin & 0xFF00)>> 0x08 );
}
extern inline void _W_tobuffer(const signed short &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0x00FF)        );
	*buf++ = (uchar)((valin & 0xFF00)>> 0x08 );
}

extern inline void _B_tobuffer(const unsigned char &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0xFF)        );
}
extern inline void _B_tobuffer(const signed char &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0xFF)        );
}

extern inline void _S_tobuffer(const char *valin, uchar *&buf, const size_t sz)
{	
	strncpy((char*)buf, valin, sz);
	buf[sz-1]=0;
	buf += sz;
}
extern inline void S_tobuffer(const char *valin, uchar *&buf, const size_t sz)
{	
	strncpy((char*)buf, (char*)valin, sz);
	buf[sz-1]=0;
}
extern inline void _X_tobuffer(const uchar *valin, uchar *&buf, const size_t sz)
{	
	memcpy(buf, valin, sz);
	buf += sz;
}
extern inline void X_tobuffer(const uchar *valin, uchar *&buf, const size_t sz)
{	
	memcpy(buf, valin, sz);
}

extern inline void _L_frombuffer(uint32 &valin, const uchar *&buf)
{	
	valin =  ( ((uint32)(buf[0]))        )
			|( ((uint32)(buf[1])) << 0x08)
			|( ((uint32)(buf[2])) << 0x10)
			|( ((uint32)(buf[3])) << 0x18);
	buf += 4;
}
extern inline void _L_frombuffer(sint32 &valin, const uchar *&buf)
{	
	valin =  ( ((uint32)(buf[0]))        )
			|( ((uint32)(buf[1])) << 0x08)
			|( ((uint32)(buf[2])) << 0x10)
			|( ((uint32)(buf[3])) << 0x18);
	buf += 4;
}

extern inline void _W_frombuffer(unsigned short &valin, const uchar *&buf)
{
	valin =  ( ((unsigned short)(buf[0]))        )
			|( ((unsigned short)(buf[1])) << 0x08);
	buf += 2;
}
extern inline void _W_frombuffer(signed short &valin, const uchar *&buf)
{	
	valin =  ( ((unsigned short)(buf[0]))        )
			|( ((unsigned short)(buf[1])) << 0x08);
	buf += 2;
}

extern inline void _B_frombuffer(unsigned char &valin, const uchar *&buf)
{	
	valin	= *buf++;
}
extern inline void _B_frombuffer(signed char &valin, const uchar *&buf)
{	
	valin	= (char)(*buf++);
}

extern inline void _S_frombuffer(char *valin, const uchar *&buf, const size_t sz)
{	
	strncpy(valin, (char*)buf, sz);
	valin[sz-1]=0;
	buf += sz;
}
extern inline void S_frombuffer(char *valin, const uchar *buf, const size_t sz)
{	
	strncpy(valin, (char*)buf, sz);
	valin[sz-1]=0;
}
extern inline void _X_frombuffer(uchar *valin, const uchar *&buf, const size_t sz)
{	
	memcpy(valin, buf, sz);
	buf += sz;
}
extern inline void X_frombuffer(uchar *valin, const uchar *buf, const size_t sz)
{	
	memcpy(valin, buf, sz);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// predeclaration
struct map_session_data;




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct sc_data
{
	ushort type; //SC_type
	uint32 val1;
	uint32 val2;
	uint32 val3;
	uint32 val4;
	uint32 tick; //Remaining duration (only 32bits used)
};

extern inline void _scdata_tobuffer(const struct sc_data &p, uchar *&buf)
{
	if(NULL==buf )	return;
	_W_tobuffer( (p.type),			buf);
	_L_tobuffer( (p.val1),			buf);
	_L_tobuffer( (p.val2),			buf);
	_L_tobuffer( (p.val3),			buf);
	_L_tobuffer( (p.val4),			buf);
	_L_tobuffer( (p.tick),			buf);
}
extern inline void scdata_tobuffer(const struct sc_data &p, uchar *buf)
{
	_scdata_tobuffer(p, buf);
}

extern inline void _scdata_frombuffer(struct sc_data &p, const uchar *&buf)
{
	if(NULL==buf )	return;
	_W_frombuffer( (p.type),		buf);
	_L_frombuffer( (p.val1),		buf);
	_L_frombuffer( (p.val2),		buf);
	_L_frombuffer( (p.val3),		buf);
	_L_frombuffer( (p.val4),		buf);
	_L_frombuffer( (p.tick),		buf);
}
extern inline void scdata_frombuffer(struct sc_data &p, const uchar *buf)
{
	_scdata_frombuffer(p, buf);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct item
{
	unsigned short	id;				// 
	unsigned short	nameid;			// nameid of the corrosponding item_data

	unsigned short	amount;			// number of items in this stash
	unsigned short	equip;			//-> remove, make a better equip system

	uchar	identify;		// :1; used as boolean only 
	uchar	refine;			// :4; stores number of refines (max 10)
	uchar	attribute;		// 

// -> introduce, if it should be possible to introduce production of slotted items
//	uint32	producer_id;	// the id of the producer or zero


	unsigned short	card[4];		// cards
// current usage of the cards:
// card[0] == 0xff00	Petegg
//					with
// (card[1], card[2]) = pet_id

// card[0] == 0x00ff	forged weapon
//					with
//	hibyte(card[1]>>8)  = star
//	card[1]&0x000f)		= wele (weapon_element, I guess)
// (card[2], card[3]) = producer_id

// card[0] == 0x00fe	forged something
//					with
// card[1] =0	
// (card[2], card[3]) = producer_id

/*
	not that good for consistent socket transfer
	but structural better might be:
	union
	{
		struct
		{
			unsigned short card[4];
		};
		struct
		{
			unsigned short itemtype;	// 0xff00 Petegg
										// 0x00ff forged weapon
										// 0x00fe forged something
			unsigned short info;
			uint32 nameid[4];
		};
	}
*/
};

extern inline void _item_tobuffer(const struct item &p, uchar *&buf)
{
	if(NULL==buf )	return;
	_W_tobuffer( (p.id),			buf);
	_W_tobuffer( (p.nameid),		buf);
	_W_tobuffer( (p.amount),		buf);
	_W_tobuffer( (p.equip),			buf);
	_B_tobuffer( (p.identify),		buf);
	_B_tobuffer( (p.refine),		buf);
	_B_tobuffer( (p.attribute),		buf);
	_W_tobuffer( (p.card[0]),		buf);
	_W_tobuffer( (p.card[1]),		buf);
	_W_tobuffer( (p.card[2]),		buf);
	_W_tobuffer( (p.card[3]),		buf);
}
extern inline void item_tobuffer(const struct item &p, uchar *buf)
{
	_item_tobuffer(p, buf);
}

extern inline void _item_frombuffer(struct item &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer( (p.id),			buf);
	_W_frombuffer( (p.nameid),		buf);
	_W_frombuffer( (p.amount),		buf);
	_W_frombuffer( (p.equip),		buf);
	_B_frombuffer( (p.identify),	buf);
	_B_frombuffer( (p.refine),		buf);
	_B_frombuffer( (p.attribute),	buf);
	_W_frombuffer( (p.card[0]),		buf);
	_W_frombuffer( (p.card[1]),		buf);
	_W_frombuffer( (p.card[2]),		buf);
	_W_frombuffer( (p.card[3]),		buf);
}
extern inline void item_frombuffer(struct item &p, const uchar *buf)
{
	_item_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct point
{
	char mapname[24];
	signed short x;
	signed short y;
};
extern inline void _point_tobuffer(const struct point &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer( (p.mapname),	buf, 24);
	_W_tobuffer( (p.x),			buf);
	_W_tobuffer( (p.y),			buf);
}
extern inline void point_tobuffer(const struct point &p, uchar *buf)
{
	_point_tobuffer(p, buf);
}

extern inline void _point_frombuffer(struct point &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer( (p.mapname),	buf, 24);
	char*ip = strchr(p.mapname,'.');
	if(ip) *ip=0;

	_W_frombuffer( (p.x),		buf);
	_W_frombuffer( (p.y),		buf);
}
extern inline void point_frombuffer(struct point &p, const uchar *buf)
{
	_point_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct skill
{
	unsigned short id;
	unsigned short lv;
	unsigned short flag;
};
extern inline void _skill_tobuffer(const struct skill &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_W_tobuffer( (p.id),		buf);
	_W_tobuffer( (p.lv),		buf);
	_W_tobuffer( (p.flag),		buf);
}
extern inline void skill_tobuffer(const struct skill &p, uchar *buf)
{
	_skill_tobuffer(p, buf);
}

extern inline void _skill_frombuffer(struct skill &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer( (p.id),		buf);
	_W_frombuffer( (p.lv),		buf);
	_W_frombuffer( (p.flag),	buf);
}
extern inline void skill_frombuffer(struct skill &p, const uchar *buf)
{
	_skill_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct global_reg
{
	char str[32];
	sint32 value;
};
extern inline void _global_reg_tobuffer(const struct global_reg &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer( (p.str),		buf, 32);
	_L_tobuffer( (p.value),		buf);
}
extern inline void global_reg_tobuffer(const struct global_reg &p, uchar *buf)
{
	_global_reg_tobuffer(p, buf);
}

extern inline void _global_reg_frombuffer(struct global_reg &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer( (p.str),		buf, 32);
	_L_frombuffer( (p.value),	buf);
}
extern inline void global_reg_frombuffer(struct global_reg &p, const uchar *buf)
{
	_global_reg_frombuffer(p, buf);
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct s_pet
{
	uint32 account_id;
	uint32 char_id;
	uint32 pet_id;
	signed short class_;
	signed short level;
	signed short egg_id;//pet egg id
	unsigned short equip_id;		//pet equip name_id
	signed short intimate;//pet friendly
	signed short hungry;//pet hungry
	char name[24];
	unsigned char rename_flag;
	unsigned char incuvate;
};
extern inline void _s_pet_tobuffer(const struct s_pet &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer( (p.account_id),		buf);
	_L_tobuffer( (p.char_id),			buf);
	_L_tobuffer( (p.pet_id),			buf);
	_W_tobuffer( (p.class_),			buf);
	_W_tobuffer( (p.level),				buf);
	_W_tobuffer( (p.egg_id),			buf);
	_W_tobuffer( (p.equip_id),			buf);
	_W_tobuffer( (p.intimate),			buf);
	_W_tobuffer( (p.hungry),			buf);
	_S_tobuffer( (p.name),				buf, 24);
	_B_tobuffer( (p.rename_flag),		buf);
	_B_tobuffer( (p.incuvate),			buf);
}
extern inline void s_pet_tobuffer(const struct s_pet &p, uchar *buf)
{
	_s_pet_tobuffer(p, buf);
}
extern inline void _s_pet_frombuffer(struct s_pet &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer( (p.account_id),		buf);
	_L_frombuffer( (p.char_id),			buf);
	_L_frombuffer( (p.pet_id),			buf);
	_W_frombuffer( (p.class_),			buf);
	_W_frombuffer( (p.level),			buf);
	_W_frombuffer( (p.egg_id),			buf);
	_W_frombuffer( (p.equip_id),		buf);
	_W_frombuffer( (p.intimate),		buf);
	_W_frombuffer( (p.hungry),			buf);
	_S_frombuffer( (p.name),			buf, 24);
	_B_frombuffer( (p.rename_flag),		buf);
	_B_frombuffer( (p.incuvate),		buf);
}
extern inline void s_pet_frombuffer(struct s_pet &p, const uchar *buf)
{
	_s_pet_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct friends
{	// Friends list vars
	uint32 friend_id;
	char friend_name[24];
};


struct mmo_charstatus
{
	uint32 char_id;
	uint32 account_id;
	uint32 partner_id;
	uint32 father_id;
	uint32 mother_id;
	uint32 child_id;

	uint32 base_exp;
	uint32 job_exp;
	uint32 zeny;

	unsigned short class_;
	unsigned short status_point;
	unsigned short skill_point;

	sint32 hp;
	sint32 max_hp;
	sint32 sp;
	sint32 max_sp;

	signed char karma;		// good<->evil
	signed char chaos;		// chaotic<->lawful
	signed short manner;	// stores negative values of mute
	signed short option;
	unsigned short hair;
	unsigned short hair_color;
	unsigned short clothes_color;

	uint32 party_id;
	uint32 guild_id;
	uint32 pet_id;
	uint32 fame_points;

	unsigned short weapon;
	unsigned short shield;
	unsigned short head_top;
	unsigned short head_mid;
	unsigned short head_bottom;

	char name[24];
	unsigned short base_level;
	unsigned short job_level;
	unsigned short str;
	unsigned short agi;
	unsigned short vit;
	unsigned short int_;
	unsigned short dex;
	unsigned short luk;
	uchar slot;
	uchar sex;
	uchar gm_level;

	uint32 mapip;
	unsigned short mapport;

	struct point last_point;
	struct point save_point;
	struct point memo_point[MAX_MEMO];
	struct item inventory[MAX_INVENTORY];

	struct item equipment[MAX_EQUIP];

	struct item cart[MAX_CART];
	struct skill skill[MAX_SKILL];
	uint32 global_reg_num;
	struct global_reg global_reg[GLOBAL_REG_NUM];
	uint32 account_reg_num;
	struct global_reg account_reg[ACCOUNT_REG_NUM];
	uint32 account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

	// Friends list vars
	struct friends	friendlist[MAX_FRIENDLIST];
};

extern inline void _mmo_charstatus_tobuffer(const struct mmo_charstatus &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.char_id),			buf);
	_L_tobuffer( (p.account_id),		buf);
	_L_tobuffer( (p.partner_id),		buf);
	_L_tobuffer( (p.father_id),		buf);
	_L_tobuffer( (p.mother_id),		buf);
	_L_tobuffer( (p.child_id),		buf);

	_L_tobuffer( (p.base_exp),		buf);
	_L_tobuffer( (p.job_exp),			buf);
	_L_tobuffer( (p.zeny),			buf);

	_W_tobuffer( (p.class_),			buf);
	_W_tobuffer( (p.status_point),	buf);
	_W_tobuffer( (p.skill_point),		buf);

	_L_tobuffer( (p.hp),				buf);
	_L_tobuffer( (p.max_hp),			buf);
	_L_tobuffer( (p.sp),				buf);
	_L_tobuffer( (p.max_sp),			buf);

	_B_tobuffer( (p.karma),			buf);
	_B_tobuffer( (p.chaos),			buf);
	_W_tobuffer( (p.manner),			buf);
	_W_tobuffer( (p.option),			buf);
	_W_tobuffer( (p.hair),			buf);
	_W_tobuffer( (p.hair_color),		buf);
	_W_tobuffer( (p.clothes_color),	buf);

	_L_tobuffer( (p.party_id),		buf);
	_L_tobuffer( (p.guild_id),		buf);
	_L_tobuffer( (p.pet_id),			buf);

	_L_tobuffer( (p.fame_points),			buf);

	_W_tobuffer( (p.weapon),			buf);
	_W_tobuffer( (p.shield),			buf);
	_W_tobuffer( (p.head_top),		buf);
	_W_tobuffer( (p.head_mid),		buf);
	_W_tobuffer( (p.head_bottom),		buf);

	_S_tobuffer( p.name,				buf, 24);

	_W_tobuffer( (p.base_level),		buf);
	_W_tobuffer( (p.job_level),		buf);
	_W_tobuffer( (p.str),			buf);
	_W_tobuffer( (p.agi),			buf);
	_W_tobuffer( (p.vit),			buf);
	_W_tobuffer( (p.int_),			buf);
	_W_tobuffer( (p.dex),			buf);
	_W_tobuffer( (p.luk),			buf);

	_B_tobuffer( (p.slot),		buf);
	_B_tobuffer( (p.sex),			buf);
	_B_tobuffer( (p.gm_level),		buf);

	_L_tobuffer( (p.mapip),			buf);
	_W_tobuffer( (p.mapport),		buf);

	_point_tobuffer(             (p.last_point),		buf);
	_point_tobuffer(             (p.save_point),		buf);
	for(i=0; i<MAX_MEMO; i++)
		_point_tobuffer(           p.memo_point[i],		buf);

	for(i=0; i<MAX_INVENTORY; i++)
		_item_tobuffer(            p.inventory[i],		buf);

	for(i=0; i<MAX_CART; i++)
		_item_tobuffer(            p.cart[i],			buf);

	for(i=0; i<MAX_SKILL; i++)
		_skill_tobuffer(           p.skill[i],			buf);

	_L_tobuffer(  (p.global_reg_num),	buf);
	for(i=0; i<GLOBAL_REG_NUM; i++)
		_global_reg_tobuffer(      p.global_reg[i],		buf);


	_L_tobuffer(  (p.account_reg_num),	buf);
	for(i=0; i<ACCOUNT_REG_NUM; i++)
		_global_reg_tobuffer(      p.account_reg[i],	buf);

	_L_tobuffer(  (p.account_reg2_num),buf);
	for(i=0; i<ACCOUNT_REG2_NUM; i++)
		_global_reg_tobuffer(      p.account_reg2[i],	buf);

	// Friends list vars
	for(i=0; i<MAX_FRIENDLIST; i++)
		_L_tobuffer( (p.friendlist[i].friend_id),	buf);
	for(i=0; i<MAX_FRIENDLIST; i++)
		_S_tobuffer(               p.friendlist[i].friend_name,	buf, 24);
}
extern inline void mmo_charstatus_tobuffer(const struct mmo_charstatus &p, uchar *buf)
{
	_mmo_charstatus_tobuffer(p, buf);
}
extern inline void _mmo_charstatus_frombuffer(struct mmo_charstatus &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer( (p.char_id),			buf);
	_L_frombuffer( (p.account_id),		buf);
	_L_frombuffer( (p.partner_id),		buf);
	_L_frombuffer( (p.father_id),		buf);
	_L_frombuffer( (p.mother_id),		buf);
	_L_frombuffer( (p.child_id),		buf);

	_L_frombuffer( (p.base_exp),		buf);
	_L_frombuffer( (p.job_exp),			buf);
	_L_frombuffer( (p.zeny),			buf);

	_W_frombuffer( (p.class_),			buf);
	_W_frombuffer( (p.status_point),	buf);
	_W_frombuffer( (p.skill_point),		buf);

	_L_frombuffer( (p.hp),				buf);
	_L_frombuffer( (p.max_hp),			buf);
	_L_frombuffer( (p.sp),				buf);
	_L_frombuffer( (p.max_sp),			buf);

	_B_frombuffer( (p.karma),			buf);
	_B_frombuffer( (p.chaos),			buf);
	_W_frombuffer( (p.manner),			buf);
	_W_frombuffer( (p.option),			buf);
	_W_frombuffer( (p.hair),			buf);
	_W_frombuffer( (p.hair_color),		buf);
	_W_frombuffer( (p.clothes_color),	buf);

	_L_frombuffer( (p.party_id),		buf);
	_L_frombuffer( (p.guild_id),		buf);
	_L_frombuffer( (p.pet_id),			buf);

	_L_frombuffer( (p.fame_points),		buf);

	_W_frombuffer( (p.weapon),			buf);
	_W_frombuffer( (p.shield),			buf);
	_W_frombuffer( (p.head_top),		buf);
	_W_frombuffer( (p.head_mid),		buf);
	_W_frombuffer( (p.head_bottom),		buf);

	_S_frombuffer( p.name,				buf, 24);

	_W_frombuffer( (p.base_level),		buf);
	_W_frombuffer( (p.job_level),		buf);
	_W_frombuffer( (p.str),				buf);
	_W_frombuffer( (p.agi),				buf);
	_W_frombuffer( (p.vit),				buf);
	_W_frombuffer( (p.int_),			buf);
	_W_frombuffer( (p.dex),				buf);
	_W_frombuffer( (p.luk),				buf);

	_B_frombuffer( (p.slot),			buf);
	_B_frombuffer( (p.sex),				buf);
	_B_frombuffer( (p.gm_level),		buf);

	_L_frombuffer( (p.mapip),			buf);
	_W_frombuffer( (p.mapport),			buf);


	_point_frombuffer(             (p.last_point),	buf);
	_point_frombuffer(             (p.save_point),	buf);
	for(i=0; i<MAX_MEMO; i++)
		_point_frombuffer(           p.memo_point[i],	buf);

	for(i=0; i<MAX_INVENTORY; i++)
		_item_frombuffer(            p.inventory[i],	buf);

	for(i=0; i<MAX_CART; i++)
		_item_frombuffer(            p.cart[i],			buf);

	for(i=0; i<MAX_SKILL; i++)
		_skill_frombuffer(           p.skill[i],		buf);

	_L_frombuffer(  (p.global_reg_num),buf);
	for(i=0; i<GLOBAL_REG_NUM; i++)
		_global_reg_frombuffer(      p.global_reg[i],	buf);


	_L_frombuffer(  (p.account_reg_num),buf);
	for(i=0; i<ACCOUNT_REG_NUM; i++)
		_global_reg_frombuffer(      p.account_reg[i],	buf);

	_L_frombuffer(  (p.account_reg2_num),buf);
	for(i=0; i<ACCOUNT_REG2_NUM; i++)
		_global_reg_frombuffer(      p.account_reg2[i],	buf);

	// Friends list vars
	for(i=0; i<MAX_FRIENDLIST; i++)
		_L_frombuffer( (p.friendlist[i].friend_id),buf);
	for(i=0; i<MAX_FRIENDLIST; i++)
		_S_frombuffer(               p.friendlist[i].friend_name,	buf, 24);
}
extern inline void mmo_charstatus_frombuffer(struct mmo_charstatus &p, const uchar *buf)
{
	_mmo_charstatus_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct pc_storage {
	uint32 dirty;
	uint32 account_id;
	signed short storage_status;
	signed short storage_amount;
	struct item storage[MAX_STORAGE];
};
extern inline void _pc_storage_tobuffer(const struct pc_storage &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.dirty),				buf);
	_L_tobuffer( (p.account_id),		buf);
	_W_tobuffer( (p.storage_status),	buf);
	_W_tobuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_tobuffer(p.storage[i],	buf);
}
extern inline void pc_storage_tobuffer(const struct pc_storage &p, uchar *buf)
{
	_pc_storage_tobuffer(p, buf);
}
extern inline void _pc_storage_frombuffer(struct pc_storage &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer( (p.dirty),			buf);
	_L_frombuffer( (p.account_id),		buf);
	_W_frombuffer( (p.storage_status),	buf);
	_W_frombuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_frombuffer(p.storage[i],	buf);
}
extern inline void pc_storage_frombuffer(struct pc_storage &p, const uchar *buf)
{
	_pc_storage_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_storage {
	uint32 guild_id;
	signed short storage_status;
	signed short storage_amount;
	struct item storage[MAX_GUILD_STORAGE];
};
extern inline void _guild_storage_tobuffer(const struct guild_storage &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.guild_id),			buf);
	_W_tobuffer( (p.storage_status),	buf);
	_W_tobuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_tobuffer(p.storage[i],		buf);
}
extern inline void guild_storage_tobuffer(const struct guild_storage &p, uchar *buf)
{
	_guild_storage_tobuffer(p, buf);
}
extern inline void _guild_storage_frombuffer(struct guild_storage &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer( (p.guild_id),		buf);
	_W_frombuffer( (p.storage_status),	buf);
	_W_frombuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_frombuffer(p.storage[i],		buf);
}
extern inline void guild_storage_frombuffer(struct guild_storage &p, const uchar *buf)
{
	_guild_storage_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct gm_account {
	uint32 account_id;
	uchar level;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct party_member {
	uint32 account_id;
	char name[24];
	char mapname[24];
	uchar leader;
	uchar online;
	unsigned short lv;
	struct map_session_data *sd;
};
extern inline void _party_member_tobuffer(const struct party_member &p, uchar *&buf)
{	
	if( NULL==buf )	return;
	_L_tobuffer( (p.account_id),	buf);
	_S_tobuffer( (p.name),			buf, 24);
	_S_tobuffer( (p.mapname),		buf, 24);
	_B_tobuffer( (p.leader),		buf);
	_B_tobuffer( (p.online),		buf);
	_W_tobuffer( (p.lv),			buf);
	//_L_tobuffer( &(p.sd),			buf);
	buf+=sizeof(struct map_session_data *);
	// skip the map_session_data *
}
extern inline void party_member_tobuffer(const struct party_member &p, uchar *buf)
{
	_party_member_tobuffer(p, buf);
}
extern inline void _party_member_frombuffer(struct party_member &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer( (p.account_id),	buf);
	_S_frombuffer( (p.name),		buf, 24);
	_S_frombuffer( (p.mapname),		buf, 24);
	char*ip = strchr(p.mapname,'.');
	if(ip) *ip=0;

	_B_frombuffer( (p.leader),		buf);
	_B_frombuffer( (p.online),		buf);
	_W_frombuffer( (p.lv),			buf);
	//_L_frombuffer( (p.sd),		buf);
	buf+=sizeof(struct map_session_data *); 
	p.sd = NULL; 
	// skip the map_session_data *
}
extern inline void party_member_frombuffer(struct party_member &p, const uchar *buf)
{
	_party_member_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct party {
	uint32 party_id;
	char name[24];
	unsigned short expshare;
	unsigned short itemshare;
	unsigned short itemc;
	struct party_member member[MAX_PARTY];
};
extern inline void _party_tobuffer(const struct party &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.party_id),		buf);
	_S_tobuffer( (p.name),			buf, 24);
	_W_tobuffer( (p.expshare),		buf);
	_W_tobuffer( (p.itemshare),		buf);
	_W_tobuffer( (p.itemc),			buf);
	for(i=0; i< MAX_PARTY; i++)
		_party_member_tobuffer(p.member[i], buf);
}
extern inline void party_tobuffer(const struct party &p, uchar *buf)
{
	_party_tobuffer(p, buf);
}
extern inline void _party_frombuffer(struct party &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer( (p.party_id),	buf);
	_S_frombuffer( (p.name),		buf, 24);
	_W_frombuffer( (p.expshare),	buf);
	_W_frombuffer( (p.itemshare),	buf);
	_W_frombuffer( (p.itemc),		buf);
	for(i=0; i< MAX_PARTY; i++)
		_party_member_frombuffer(p.member[i], buf);
}
extern inline void party_frombuffer(struct party &p, const uchar *buf)
{
	_party_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_member {
	uint32 account_id;
	uint32 char_id;
	unsigned short hair;
	unsigned short hair_color;
	uchar gender;
	unsigned short class_;
	unsigned short lv;
	uint32 exp;
	uint32 exp_payper;
	unsigned short online;
	unsigned short position;
	uint32 rsv1;
	uint32 rsv2;
	char name[24];
	struct map_session_data *sd;
};
extern inline void _guild_member_tobuffer(const struct guild_member &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer( (p.account_id),	buf);
	_L_tobuffer( (p.char_id),		buf);
	_W_tobuffer( (p.hair),			buf);
	_W_tobuffer( (p.hair_color),	buf);
	_B_tobuffer( (p.gender),		buf);
	_W_tobuffer( (p.class_),		buf);
	_W_tobuffer( (p.lv),			buf);
	_L_tobuffer( (p.exp),			buf);
	_L_tobuffer( (p.exp_payper),	buf);
	_W_tobuffer( (p.online),		buf);
	_W_tobuffer( (p.position),		buf);
	_L_tobuffer( (p.rsv1),			buf);
	_L_tobuffer( (p.rsv2),			buf);
	_S_tobuffer( (p.name),			buf, 24);
	//_L_tobuffer( &(p.sd),			buf);
	buf+=sizeof(struct map_session_data *);
	// skip the struct map_session_data *
}
extern inline void guild_member_tobuffer(const struct guild_member &p, uchar *buf)
{
	_guild_member_tobuffer(p, buf);
}
extern inline void _guild_member_frombuffer(struct guild_member &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer( (p.account_id),	buf);
	_L_frombuffer( (p.char_id),		buf);
	_W_frombuffer( (p.hair),		buf);
	_W_frombuffer( (p.hair_color),	buf);
	_B_frombuffer( (p.gender),		buf);
	_W_frombuffer( (p.class_),		buf);
	_W_frombuffer( (p.lv),			buf);
	_L_frombuffer( (p.exp),			buf);
	_L_frombuffer( (p.exp_payper),	buf);
	_W_frombuffer( (p.online),		buf);
	_W_frombuffer( (p.position),	buf);
	_L_frombuffer( (p.rsv1),		buf);
	_L_frombuffer( (p.rsv2),		buf);
	_S_frombuffer( (p.name),		buf, 24);
	//_L_frombuffer( &(p.sd),		buf);
	buf+=sizeof(struct map_session_data *);
	p.sd = NULL; 
	// skip the struct map_session_data *
}
extern inline void guild_member_frombuffer(struct guild_member &p, const uchar *buf)
{
	_guild_member_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_position {
	char name[24];
	uint32 mode;
	uint32 exp_mode;
};
extern inline void _guild_position_tobuffer(const struct guild_position &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer( (p.name),			buf, 24);
	_L_tobuffer( (p.mode),			buf);
	_L_tobuffer( (p.exp_mode),		buf);
}
extern inline void guild_position_tobuffer(const struct guild_position &p, uchar *buf)
{
	_guild_position_tobuffer(p, buf);
}
extern inline void _guild_position_frombuffer(struct guild_position &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer( (p.name),		buf, 24);
	_L_frombuffer( (p.mode),		buf);
	_L_frombuffer( (p.exp_mode),	buf);
}
extern inline void guild_position_frombuffer(struct guild_position &p, const uchar *buf)
{
	_guild_position_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_alliance {
	sint32 opposition;
	uint32 guild_id;
	char name[24];
};
extern inline void _guild_alliance_tobuffer(const struct guild_alliance &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer( (p.opposition),	buf);
	_L_tobuffer( (p.guild_id),		buf);
	_S_tobuffer( (p.name),			buf, 24);
}
extern inline void guild_alliance_tobuffer(const struct guild_alliance &p, uchar *buf)
{
	_guild_alliance_tobuffer(p, buf);
}
extern inline void _guild_alliance_frombuffer(struct guild_alliance &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer( (p.opposition),	buf);
	_L_frombuffer( (p.guild_id),	buf);
	_S_frombuffer( (p.name),		buf, 24);
}
extern inline void guild_alliance_frombuffer(struct guild_alliance &p, const uchar *buf)
{
	_guild_alliance_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_explusion {
	char name[24];
	char mes[40];
	char acc[40];
	uint32 account_id;
	uint32 char_id;
	uint32 rsv1;
	uint32 rsv2;
	uint32 rsv3;
};
extern inline void _guild_explusion_tobuffer(const struct guild_explusion &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer( (p.name),			buf, 24);
	_S_tobuffer( (p.mes),			buf, 40);
	_S_tobuffer( (p.acc),			buf, 40);
	_L_tobuffer( (p.account_id),	buf);
	_L_tobuffer( (p.char_id),		buf);
	_L_tobuffer( (p.rsv1),			buf);
	_L_tobuffer( (p.rsv2),			buf);
	_L_tobuffer( (p.rsv3),			buf);
}
extern inline void guild_explusion_tobuffer(const struct guild_explusion &p, uchar *buf)
{
	_guild_explusion_tobuffer(p, buf);
}
extern inline void _guild_explusion_frombuffer(struct guild_explusion &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer( (p.name),		buf, 24);
	_S_frombuffer( (p.mes),			buf, 40);
	_S_frombuffer( (p.acc),			buf, 40);
	_L_frombuffer( (p.account_id),	buf);
	_L_frombuffer( (p.char_id),		buf);
	_L_frombuffer( (p.rsv1),		buf);
	_L_frombuffer( (p.rsv2),		buf);
	_L_frombuffer( (p.rsv3),		buf);
}
extern inline void guild_explusion_frombuffer(struct guild_explusion &p, const uchar *buf)
{
	_guild_explusion_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_skill {
	unsigned short id;
	unsigned short lv;
};
extern inline void _guild_skill_tobuffer(const struct guild_skill &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_W_tobuffer(  (p.id),	buf);
	_W_tobuffer(  (p.lv),	buf);
}
extern inline void guild_skill_tobuffer(const struct guild_skill &p, uchar *buf)
{
	_guild_skill_tobuffer(p, buf);
}
extern inline void _guild_skill_frombuffer(struct guild_skill &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer(  (p.id),buf);
	_W_frombuffer(  (p.lv),buf);
}
extern inline void guild_skill_frombuffer(struct guild_skill &p, const uchar *buf)
{
	_guild_skill_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild {
	uint32 guild_id;
	unsigned short guild_lv;
	unsigned short connect_member;
	unsigned short max_member; 
	unsigned short average_lv;
	uint32 exp;
	uint32 next_exp;
	unsigned short skill_point;
	char name[24];
	char master[24];
	struct guild_member member[MAX_GUILD];
	struct guild_position position[MAX_GUILDPOSITION];
	char mes1[60];
	char mes2[120];
	uint32 emblem_id;
	uchar emblem_data[2048];
	unsigned short emblem_len;
	signed char		chaos;	// average chaos of guild members
	signed char		honour;	// average honour of guild members (not saved calculated on the fly)
	struct guild_alliance alliance[MAX_GUILDALLIANCE];
	struct guild_explusion explusion[MAX_GUILDEXPLUSION];
	struct guild_skill skill[MAX_GUILDSKILL];
//#ifndef TXT_ONLY
	uchar save_flag;
	int save_timer;
//#endif
};
extern inline void _guild_tobuffer(const struct guild &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.guild_id),			buf);
	_W_tobuffer( (p.guild_lv),			buf);
	_W_tobuffer( (p.connect_member),	buf);
	_W_tobuffer( (p.max_member),		buf);
	_W_tobuffer( (p.average_lv),		buf);
	_L_tobuffer( (p.exp),				buf);
	_L_tobuffer( (p.next_exp),			buf);
	_W_tobuffer( (p.skill_point),		buf);
	_S_tobuffer( (p.name),				buf, 24);
	_S_tobuffer( (p.master),			buf, 24);
	for(i=0; i< MAX_GUILD; i++)
		_guild_member_tobuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; i++)
		_guild_position_tobuffer(p.position[i],	buf);
	_S_tobuffer( (p.mes1),				buf, 60);
	_S_tobuffer( (p.mes2),				buf, 120);
	_L_tobuffer( (p.emblem_id),			buf);
	_W_tobuffer( (p.emblem_len),		buf);
	_X_tobuffer( (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; i++)
		_guild_alliance_tobuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; i++)
		_guild_explusion_tobuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; i++)
		_guild_skill_tobuffer(p.skill[i],		buf);
}
extern inline void guild_tobuffer(const struct guild &p, uchar *buf)
{
	_guild_tobuffer(p, buf);
}
extern inline void _guild_frombuffer(struct guild &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer( (p.guild_id),		buf);
	_W_frombuffer( (p.guild_lv),		buf);
	_W_frombuffer( (p.connect_member),	buf);
	_W_frombuffer( (p.max_member),		buf);
	_W_frombuffer( (p.average_lv),		buf);
	_L_frombuffer( (p.exp),				buf);
	_L_frombuffer( (p.next_exp),		buf);
	_W_frombuffer( (p.skill_point),		buf);
	_S_frombuffer( (p.name),			buf, 24);
	_S_frombuffer( (p.master),			buf, 24);
	for(i=0; i< MAX_GUILD; i++)
		_guild_member_frombuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; i++)
		_guild_position_frombuffer(p.position[i],	buf);
	_S_frombuffer( (p.mes1),			buf, 60);
	_S_frombuffer( (p.mes2),			buf, 120);
	_L_frombuffer( (p.emblem_id),		buf);
	_W_frombuffer( (p.emblem_len),		buf);
	_X_frombuffer( (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; i++)
		_guild_alliance_frombuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; i++)
		_guild_explusion_frombuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; i++)
		_guild_skill_frombuffer(p.skill[i],		buf);
}
extern inline void guild_frombuffer(struct guild &p, const uchar *buf)
{
	_guild_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_castle_guardian
{
	uint32	guardian_id;
	uint32	guardian_hp;
	uchar	visible;
};
extern inline void _guild_castle_guardian_tobuffer(const struct guild_castle_guardian &p, uchar *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer( (p.guardian_id),		buf);
	_L_tobuffer( (p.guardian_hp),		buf);
	_B_tobuffer( (p.visible),			buf);
}
extern inline void guild_castle_guardian_tobuffer(const struct guild_castle_guardian &p, uchar *buf)
{
	_guild_castle_guardian_tobuffer(p, buf);
}
extern inline void _guild_castle_guardian_frombuffer(struct guild_castle_guardian &p, const uchar *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer( (p.guardian_id),		buf);
	_L_frombuffer( (p.guardian_hp),		buf);
	_B_frombuffer( (p.visible),			buf);
}
extern inline void guild_castle_guardiane_frombuffer(struct guild_castle_guardian &p, const uchar *buf)
{
	_guild_castle_guardian_frombuffer(p, buf);
}
struct guild_castle
{
	unsigned short castle_id;
	char mapname[24];
	char castle_name[24];
	char castle_event[24];
	uint32 guild_id;
	uint32 economy;
	uint32 defense;
	uint32 triggerE;
	uint32 triggerD;
	uint32 nextTime;
	uint32 payTime;
	uint32 createTime;
	uint32 visibleC;
	struct guild_castle_guardian guardian[MAX_GUARDIAN];
};
extern inline void _guild_castle_tobuffer(const struct guild_castle &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_W_tobuffer( (p.castle_id),		buf);
	_S_tobuffer( (p.mapname),		buf, 24);
	_S_tobuffer( (p.castle_name),	buf, 24);
	_S_tobuffer( (p.castle_event),	buf, 24);
	_L_tobuffer( (p.guild_id),		buf);
	_L_tobuffer( (p.economy),		buf);
	_L_tobuffer( (p.defense),		buf);
	_L_tobuffer( (p.triggerE),		buf);
	_L_tobuffer( (p.triggerD),		buf);
	_L_tobuffer( (p.nextTime),		buf);
	_L_tobuffer( (p.payTime),		buf);
	_L_tobuffer( (p.createTime),	buf);
	_L_tobuffer( (p.visibleC),		buf);

	for(i=0; i<MAX_GUARDIAN; i++)
		_guild_castle_guardian_tobuffer(p.guardian[i],buf);
}
extern inline void guild_castle_tobuffer(const struct guild_castle &p, uchar *buf)
{
	_guild_castle_tobuffer(p, buf);
}
extern inline void _guild_castle_frombuffer(struct guild_castle &p, const uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_W_frombuffer( (p.castle_id),	buf);
	_S_frombuffer( (p.mapname),		buf, 24);
	char*ip = strchr(p.mapname,'.');
	if(ip) *ip=0;

	_S_frombuffer( (p.castle_name),	buf, 24);
	_S_frombuffer( (p.castle_event),buf, 24);
	_L_frombuffer( (p.guild_id),	buf);
	_L_frombuffer( (p.economy),		buf);
	_L_frombuffer( (p.defense),		buf);
	_L_frombuffer( (p.triggerE),	buf);
	_L_frombuffer( (p.triggerD),	buf);
	_L_frombuffer( (p.nextTime),	buf);
	_L_frombuffer( (p.payTime),		buf);
	_L_frombuffer( (p.createTime),	buf);
	_L_frombuffer( (p.visibleC),	buf);
	for(i=0; i<MAX_GUARDIAN; i++)
		_guild_castle_guardian_frombuffer(p.guardian[i],buf);
}
extern inline void guild_castle_frombuffer(struct guild_castle &p, const uchar *buf)
{
	_guild_castle_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct square {
	uint32 val1[5];
	uint32 val2[5];
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#endif	// _MMO_H_
