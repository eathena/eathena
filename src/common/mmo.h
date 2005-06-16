// $Id: mmo.h,v 1.3 2004/09/25 20:12:25 PoW Exp $
// Original : mmo.h 2003/03/14 12:07:02 Rev.1.7

#ifndef	_MMO_H_
#define	_MMO_H_

#include "base.h"
#include "socket.h"


#define FIFOSIZE_SERVERLINK	128*1024

// set to 0 to not check IP of player between each server.
// set to another value if you want to check (1)
#define CMP_AUTHFIFO_IP 1
#define CMP_AUTHFIFO_LOGIN2 1



#define MAX_MAP_PER_SERVER 512
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
#define GLOBAL_REG_NUM 96
#define ACCOUNT_REG_NUM 16
#define ACCOUNT_REG2_NUM 16
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
#define MAX_FRIENDLIST 20

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


/////////////////////////////////////////////////////////////////////////////
// char server definition for incomming map server connections

// currently only lanip/lanport are used for storing the ip/port 
// which are sent in connect packet
// maybe it is not necessary to store extra lan/wan ip
// since there is already the client ip in session_data

struct mmo_map_server
{
	int fd;
	unsigned long	lanip;
	unsigned short	lanport;
	unsigned long	lanmask;
	unsigned long	wanip;
	unsigned short	wanport;

	unsigned long	users;
	char map[MAX_MAP_PER_SERVER][16];
};

/////////////////////////////////////////////////////////////////////////////
// login server definition for incomming char server connections
struct mmo_char_server {
	int fd;
	unsigned long	lanip;
	unsigned short	lanport;
	unsigned long	lanmask;
	unsigned long	wanip;
	unsigned short	wanport;

	char name[20];
	size_t users;
	int maintenance;
	int new_;
};





/////////////////////////////////////////////////////////////////////////////
// simplified buffer functions with moving buffer pointer 
// to enable secure continous inserting of data to a buffer
// no pointer checking is implemented here, so make sure the calls are correct
/////////////////////////////////////////////////////////////////////////////

extern inline void _L_tobuffer(const unsigned long &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0x000000FF)        );
	*buf++ = ((valin & 0x0000FF00)>> 0x08 );
	*buf++ = ((valin & 0x00FF0000)>> 0x10 );
	*buf++ = ((valin & 0xFF000000)>> 0x18 );
}
extern inline void _L_tobuffer(const long &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0x000000FF)        );
	*buf++ = ((valin & 0x0000FF00)>> 0x08 );
	*buf++ = ((valin & 0x00FF0000)>> 0x10 );
	*buf++ = ((valin & 0xFF000000)>> 0x18 );
}

extern inline void _W_tobuffer(const unsigned short &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0x00FF)        );
	*buf++ = ((valin & 0xFF00)>> 0x08 );
}
extern inline void _W_tobuffer(const short &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0x00FF)        );
	*buf++ = ((valin & 0xFF00)>> 0x08 );
}

extern inline void _B_tobuffer(const unsigned char &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0xFF)        );
}
extern inline void _B_tobuffer(const char &valin, unsigned char *&buf)
{	
	*buf++ = ((valin & 0xFF)        );
}

extern inline void _S_tobuffer(const char *valin, unsigned char *&buf, const size_t sz)
{	
	strncpy((char*)buf, (char*)valin, sz);
	buf += sz;
}
extern inline void S_tobuffer(const char *valin, unsigned char *&buf, const size_t sz)
{	
	strncpy((char*)buf, (char*)valin, sz);
}

extern inline void _L_frombuffer(unsigned long &valin, const unsigned char *&buf)
{	
	valin = ( ((buf[0]))      )
			|( ((buf[1])) << 0x08)
			|( ((buf[2])) << 0x10)
			|( ((buf[3])) << 0x18);
	buf += 4;
}
extern inline void _L_frombuffer(long &valin, const unsigned char *&buf)
{	
	valin = ( ((buf[0]))      )
			|( ((buf[1])) << 0x08)
			|( ((buf[2])) << 0x10)
			|( ((buf[3])) << 0x18);
	buf += 4;
}

extern inline void _W_frombuffer(unsigned short &valin, const unsigned char *&buf)
{	
	valin = ( ((buf[0]))      )
			|( ((buf[1])) << 0x08);
	buf += 2;
}
extern inline void _W_frombuffer(short &valin, const unsigned char *&buf)
{	
	valin = ( ((buf[0]))      )
			|( ((buf[1])) << 0x08);
	buf += 2;
}

extern inline void _B_frombuffer(unsigned char &valin, const unsigned char *&buf)
{	
	valin	= *buf++;
}
extern inline void _B_frombuffer(char &valin, const unsigned char *&buf)
{	
	valin	= (char)(*buf++);
}

extern inline void _S_frombuffer(char *valin, const unsigned char *&buf, const size_t sz)
{	
	strncpy((char*)valin, (char*)buf, sz);
	buf += sz;
}
extern inline void S_frombuffer(char *valin, const unsigned char *buf, const size_t sz)
{	
	strncpy((char*)valin, (char*)buf, sz);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// predeclaration
struct map_session_data;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct item
{
	unsigned short	id;				// 
	unsigned short	nameid;			// nameid of the corrosponding item_data

	unsigned short	amount;			// number of items in this stash
	unsigned short	equip;			//-> remove, make a better equip system

	unsigned char	identify;		// :1; used as boolean only 
	unsigned char	refine;			// :4; stores number of refines (max 10)
	unsigned char	attribute;		// 

// -> introduce, if it should be possible to introduce production of slotted items
//	unsigned long	producer_id;	// the id of the producer or zero


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
			unsigned long nameid[4];
		};
	}
*/
};

extern inline void _item_tobuffer(const struct item &p, unsigned char *&buf)
{
	if(NULL==buf )	return;
	_W_tobuffer(  (p.id),			buf);
	_W_tobuffer( (p.nameid),		buf);
	_W_tobuffer( (p.amount),		buf);
	_W_tobuffer( (p.equip),		buf);
	_B_tobuffer(  (p.identify),	buf);
	_B_tobuffer(  (p.refine),		buf);
	_B_tobuffer(  (p.attribute),	buf);
	_W_tobuffer( (p.card[0]),		buf);
	_W_tobuffer( (p.card[1]),		buf);
	_W_tobuffer( (p.card[2]),		buf);
	_W_tobuffer( (p.card[3]),		buf);
}
extern inline void item_tobuffer(const struct item &p, unsigned char *buf)
{
	_item_tobuffer(p, buf);
}

extern inline void _item_frombuffer(struct item &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer( (p.id),		buf);
	_W_frombuffer( (p.nameid),	buf);
	_W_frombuffer( (p.amount),	buf);
	_W_frombuffer( (p.equip),		buf);
	_B_frombuffer( (p.identify),	buf);
	_B_frombuffer( (p.refine),	buf);
	_B_frombuffer( (p.attribute),buf);
	_W_frombuffer( (p.card[0]),	buf);
	_W_frombuffer( (p.card[1]),	buf);
	_W_frombuffer( (p.card[2]),	buf);
	_W_frombuffer( (p.card[3]),	buf);
}
extern inline void item_frombuffer(struct item &p, const unsigned char *buf)
{
	_item_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct point
{
	char map[24];
	short x;
	short y;
};
extern inline void _point_tobuffer(const struct point &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer(  p.map,		buf, 24);
	_W_tobuffer( (p.x),		buf);
	_W_tobuffer( (p.y),		buf);
}
extern inline void point_tobuffer(const struct point &p, unsigned char *buf)
{
	_point_tobuffer(p, buf);
}

extern inline void _point_frombuffer(struct point &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer(                   p.map,	buf, 24);
	_W_frombuffer( (p.x),		buf);
	_W_frombuffer( (p.y),		buf);
}
extern inline void point_frombuffer(struct point &p, const unsigned char *buf)
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
extern inline void _skill_tobuffer(const struct skill &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_tobuffer( (p.id),		buf);
	_W_tobuffer( (p.lv),		buf);
	_W_tobuffer( (p.flag),	buf);
}
extern inline void skill_tobuffer(const struct skill &p, unsigned char *buf)
{
	_skill_tobuffer(p, buf);
}

extern inline void _skill_frombuffer(struct skill &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer( (p.id),	buf);
	_W_frombuffer( (p.lv),	buf);
	_W_frombuffer( (p.flag),	buf);
}
extern inline void skill_frombuffer(struct skill &p, const unsigned char *buf)
{
	_skill_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct global_reg
{
	char str[32];
	long value;
};
extern inline void _global_reg_tobuffer(const struct global_reg &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer(                   p.str,		buf, 32);
	_L_tobuffer(  (p.value),	buf);
}
extern inline void global_reg_tobuffer(const struct global_reg &p, unsigned char *buf)
{
	_global_reg_tobuffer(p, buf);
}

extern inline void _global_reg_frombuffer(struct global_reg &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer(                   p.str,	buf, 32);
	_L_frombuffer(  (p.value),	buf);
}
extern inline void global_reg_frombuffer(struct global_reg &p, const unsigned char *buf)
{
	_global_reg_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct s_pet
{
	unsigned long account_id;
	unsigned long char_id;
	unsigned long pet_id;
	short class_;
	short level;
	short egg_id;//pet egg id
	unsigned short equip_id;		//pet equip name_id
	short intimate;//pet friendly
	short hungry;//pet hungry
	char name[24];
	char rename_flag;
	char incuvate;
};
extern inline void _s_pet_tobuffer(const struct s_pet &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer(  (p.account_id),	buf);
	_L_tobuffer(  (p.char_id),		buf);
	_L_tobuffer(  (p.pet_id),		buf);
	_W_tobuffer( (p.class_),		buf);
	_W_tobuffer( (p.level),		buf);
	_W_tobuffer( (p.egg_id),		buf);
	_W_tobuffer( (p.equip_id),		buf);
	_W_tobuffer( (p.intimate),	buf);
	_W_tobuffer( (p.hungry),		buf);
	_S_tobuffer(                  (p.name),		buf, 24);
	_B_tobuffer(  (p.rename_flag),	buf);
	_B_tobuffer(  (p.incuvate),	buf);
}
extern inline void s_pet_tobuffer(const struct s_pet &p, unsigned char *buf)
{
	_s_pet_tobuffer(p, buf);
}
extern inline void _s_pet_frombuffer(struct s_pet &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer(  (p.account_id),	buf);
	_L_frombuffer(  (p.char_id),		buf);
	_L_frombuffer(  (p.pet_id),		buf);
	_W_frombuffer( (p.class_),		buf);
	_W_frombuffer( (p.level),			buf);
	_W_frombuffer( (p.egg_id),		buf);
	_W_frombuffer( (p.equip_id),			buf);
	_W_frombuffer( (p.intimate),		buf);
	_W_frombuffer( (p.hungry),		buf);
	_S_frombuffer(                  (p.name),			buf, 24);
	_B_frombuffer(  (p.rename_flag),	buf);
	_B_frombuffer(  (p.incuvate),		buf);
}
extern inline void s_pet_frombuffer(struct s_pet &p, const unsigned char *buf)
{
	_s_pet_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct mmo_charstatus
{
	unsigned long char_id;
	unsigned long account_id;
	unsigned long partner_id;
	unsigned long father_id;
	unsigned long mother_id;
	unsigned long child_id;

	unsigned long base_exp;
	unsigned long job_exp;
	unsigned long zeny;

	unsigned short class_;
	unsigned short status_point;
	unsigned short skill_point;

	long hp;
	long max_hp;
	long sp;
	long max_sp;

	short option;
	char karma;
	short manner;

	unsigned short hair;
	unsigned short hair_color;
	unsigned short clothes_color;

	unsigned long party_id;
	unsigned long guild_id;
	unsigned long pet_id;
	unsigned long fame_points;

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
	unsigned char char_num;
	unsigned char sex;
	unsigned long mapip;
	unsigned short mapport;

	struct point last_point;
	struct point save_point;
	struct point memo_point[10];
	struct item inventory[MAX_INVENTORY];

	struct item equipment[MAX_EQUIP];

	struct item cart[MAX_CART];
	struct skill skill[MAX_SKILL];
	unsigned long global_reg_num;
	struct global_reg global_reg[GLOBAL_REG_NUM];
	unsigned long account_reg_num;
	struct global_reg account_reg[ACCOUNT_REG_NUM];
	unsigned long account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

	// Friends list vars
	unsigned long friend_id[MAX_FRIENDLIST];
	char friend_name[MAX_FRIENDLIST][24];
};
extern inline void _mmo_charstatus_tobuffer(const struct mmo_charstatus &p, unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer(  (p.char_id),			buf);
	_L_tobuffer(  (p.account_id),		buf);
	_L_tobuffer(  (p.partner_id),		buf);
	_L_tobuffer(  (p.father_id),		buf);
	_L_tobuffer(  (p.mother_id),		buf);
	_L_tobuffer(  (p.child_id),		buf);

	_L_tobuffer(  (p.base_exp),		buf);
	_L_tobuffer(  (p.job_exp),			buf);
	_L_tobuffer(  (p.zeny),			buf);

	_W_tobuffer( (p.class_),			buf);
	_W_tobuffer( (p.status_point),	buf);
	_W_tobuffer( (p.skill_point),		buf);

	_L_tobuffer(  (p.hp),				buf);
	_L_tobuffer(  (p.max_hp),			buf);
	_L_tobuffer(  (p.sp),				buf);
	_L_tobuffer(  (p.max_sp),			buf);

	_W_tobuffer( (p.option),			buf);
	_B_tobuffer( (p.karma),			buf);
	_W_tobuffer( (p.manner),			buf);
	_W_tobuffer( (p.hair),			buf);
	_W_tobuffer( (p.hair_color),		buf);
	_W_tobuffer( (p.clothes_color),	buf);

	_L_tobuffer(  (p.party_id),		buf);
	_L_tobuffer(  (p.guild_id),		buf);
	_L_tobuffer(  (p.pet_id),			buf);

	_L_tobuffer(  (p.fame_points),			buf);

	_W_tobuffer( (p.weapon),			buf);
	_W_tobuffer( (p.shield),			buf);
	_W_tobuffer( (p.head_top),		buf);
	_W_tobuffer( (p.head_mid),		buf);
	_W_tobuffer( (p.head_bottom),		buf);

	_S_tobuffer(                   p.name,				buf, 24);


	_W_tobuffer(                 (p.base_level),		buf);
	_W_tobuffer(                 (p.job_level),		buf);
	_W_tobuffer( (p.str),			buf);
	_W_tobuffer( (p.agi),			buf);
	_W_tobuffer( (p.vit),			buf);
	_W_tobuffer( (p.int_),			buf);
	_W_tobuffer( (p.dex),			buf);
	_W_tobuffer( (p.luk),			buf);

	_B_tobuffer(                  (p.char_num),		buf);
	_B_tobuffer(                  (p.sex),			buf);

	_L_tobuffer(                  (p.mapip),			buf);
	_W_tobuffer(                  (p.mapport),		buf);

	_point_tobuffer(             (p.last_point),		buf);
	_point_tobuffer(             (p.save_point),		buf);
	for(i=0; i<10; i++)
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
		_L_tobuffer( (p.friend_id[i]),	buf);
	for(i=0; i<MAX_FRIENDLIST; i++)
		_S_tobuffer(               p.friend_name[i],	buf, 24);
}
extern inline void mmo_charstatus_tobuffer(const struct mmo_charstatus &p, unsigned char *buf)
{
	_mmo_charstatus_tobuffer(p, buf);
}
extern inline void _mmo_charstatus_frombuffer(struct mmo_charstatus &p, const unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer(  (p.char_id),		buf);
	_L_frombuffer(  (p.account_id),	buf);
	_L_frombuffer(  (p.partner_id),	buf);
	_L_frombuffer(  (p.father_id),	buf);
	_L_frombuffer(  (p.mother_id),	buf);
	_L_frombuffer(  (p.child_id),		buf);

	_L_frombuffer(  (p.base_exp),		buf);
	_L_frombuffer(  (p.job_exp),		buf);
	_L_frombuffer(  (p.zeny),			buf);

	_W_frombuffer( (p.class_),		buf);
	_W_frombuffer( (p.status_point),	buf);
	_W_frombuffer( (p.skill_point),	buf);

	_L_frombuffer(  (p.hp),			buf);
	_L_frombuffer(  (p.max_hp),		buf);
	_L_frombuffer(  (p.sp),			buf);
	_L_frombuffer(  (p.max_sp),		buf);

	_W_frombuffer( (p.option),		buf);
	_B_frombuffer( (p.karma),		buf);
	_W_frombuffer( (p.manner),		buf);
	_W_frombuffer( (p.hair),			buf);
	_W_frombuffer( (p.hair_color),	buf);
	_W_frombuffer( (p.clothes_color),buf);

	_L_frombuffer(  (p.party_id),		buf);
	_L_frombuffer(  (p.guild_id),		buf);
	_L_frombuffer(  (p.pet_id),		buf);

	_L_frombuffer(  (p.fame_points),			buf);

	_W_frombuffer( (p.weapon),		buf);
	_W_frombuffer( (p.shield),		buf);
	_W_frombuffer( (p.head_top),		buf);
	_W_frombuffer( (p.head_mid),		buf);
	_W_frombuffer( (p.head_bottom),	buf);

	_S_frombuffer(                   p.name,			buf, 24);

	_W_frombuffer(                 (p.base_level),	buf);
	_W_frombuffer(                 (p.job_level),		buf);
	_W_frombuffer( (p.str),			buf);
	_W_frombuffer( (p.agi),			buf);
	_W_frombuffer( (p.vit),			buf);
	_W_frombuffer( (p.int_),			buf);
	_W_frombuffer( (p.dex),			buf);
	_W_frombuffer( (p.luk),			buf);

	_B_frombuffer(                 (p.char_num),		buf);
	_B_frombuffer(                 (p.sex),			buf);

	_L_frombuffer(                 (p.mapip),			buf);
	_W_frombuffer(                 (p.mapport),		buf);


	_point_frombuffer(             (p.last_point),	buf);
	_point_frombuffer(             (p.save_point),	buf);
	for(i=0; i<10; i++)
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
		_L_frombuffer( (p.friend_id[i]),buf);
	for(i=0; i<MAX_FRIENDLIST; i++)
		_S_frombuffer(               p.friend_name[i],	buf, 24);
}
extern inline void mmo_charstatus_frombuffer(struct mmo_charstatus &p, const unsigned char *buf)
{
	_mmo_charstatus_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct pc_storage {
	unsigned long dirty;
	unsigned long account_id;
	short storage_status;
	short storage_amount;
	struct item storage[MAX_STORAGE];
};
extern inline void _pc_storage_tobuffer(const struct pc_storage &p, unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer(  (p.dirty),			buf);
	_L_tobuffer(  (p.account_id),		buf);
	_W_tobuffer( (p.storage_status),	buf);
	_W_tobuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_tobuffer(p.storage[i],		buf);
}
extern inline void pc_storage_tobuffer(const struct pc_storage &p, unsigned char *buf)
{
	_pc_storage_tobuffer(p, buf);
}
extern inline void _pc_storage_frombuffer(struct pc_storage &p, const unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer(  (p.dirty),			buf);
	_L_frombuffer(  (p.account_id),	buf);
	_W_frombuffer( (p.storage_status),buf);
	_W_frombuffer( (p.storage_amount),buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_frombuffer(p.storage[i],	buf);
}
extern inline void pc_storage_frombuffer(struct pc_storage &p, const unsigned char *buf)
{
	_pc_storage_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_storage {
	unsigned long guild_id;
	short storage_status;
	short storage_amount;
	struct item storage[MAX_GUILD_STORAGE];
};
extern inline void _guild_storage_tobuffer(const struct guild_storage &p, unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer(  (p.guild_id),			buf);
	_W_tobuffer( (p.storage_status),		buf);
	_W_tobuffer( (p.storage_amount),		buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_tobuffer(p.storage[i],		buf);
}
extern inline void guild_storage_tobuffer(const struct guild_storage &p, unsigned char *buf)
{
	_guild_storage_tobuffer(p, buf);
}
extern inline void _guild_storage_frombuffer(struct guild_storage &p, const unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer(  (p.guild_id),			buf);
	_W_frombuffer( (p.storage_status),	buf);
	_W_frombuffer( (p.storage_amount),	buf);
	for(i=0;i<MAX_STORAGE;i++)
		_item_frombuffer(p.storage[i],		buf);
}
extern inline void guild_storage_frombuffer(struct guild_storage &p, const unsigned char *buf)
{
	_guild_storage_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct gm_account {
	unsigned long account_id;
	unsigned char level;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct party_member {
	unsigned long account_id;
	char name[24];
	char map[24];
	unsigned long leader;
	unsigned char online;
	unsigned short lv;
	struct map_session_data *sd;
};
extern inline void _party_member_tobuffer(const struct party_member &p, unsigned char *&buf)
{	
	if( NULL==buf )	return;
	_L_tobuffer(  (p.account_id),		buf);
	_S_tobuffer(                  (p.name),			buf, 24);
	_S_tobuffer(                  (p.map),				buf, 24);
	_L_tobuffer(  (p.leader),			buf);
	_B_tobuffer(  (p.online),			buf);
	_W_tobuffer(                 (p.lv),				buf);
	//_L_tobuffer( &(p.sd),				buf);
	(*buf)+=4;
	// skip the map_session_data *
}
extern inline void party_member_tobuffer(const struct party_member &p, unsigned char *buf)
{
	_party_member_tobuffer(p, buf);
}
extern inline void _party_member_frombuffer(struct party_member &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer(  (p.account_id),	buf);
	_S_frombuffer(                  (p.name),			buf, 24);
	_S_frombuffer(                  (p.map),			buf, 24);
	_L_frombuffer(  (p.leader),		buf);
	_B_frombuffer(  (p.online),		buf);
	_W_frombuffer(                 (p.lv),			buf);
	//_L_frombuffer( (p.sd),			buf);
	buf+=4; p.sd = NULL; 
	// skip the map_session_data *
}
extern inline void party_member_frombuffer(struct party_member &p, const unsigned char *buf)
{
	_party_member_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct party {
	unsigned long party_id;
	char name[24];
	unsigned short exp;
	unsigned short item;
	unsigned short itemc;
	struct party_member member[MAX_PARTY];
};
extern inline void _party_tobuffer(const struct party &p, unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer(  (p.party_id),	buf);
	_S_tobuffer(                  (p.name),		buf, 24);
	_W_tobuffer(                  (p.exp),			buf);
	_W_tobuffer(                  (p.item),		buf);
	_W_tobuffer(                  (p.itemc),		buf);
	for(i=0; i< MAX_PARTY; i++)
		_party_member_tobuffer(p.member[i], buf);
}
extern inline void party_tobuffer(const struct party &p, unsigned char *buf)
{
	_party_tobuffer(p, buf);
}
extern inline void _party_frombuffer(struct party &p, const unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer(  (p.party_id),	buf);
	_S_frombuffer(                  (p.name),		buf, 24);
	_W_frombuffer(                  (p.exp),		buf);
	_W_frombuffer(                  (p.item),		buf);
	_W_frombuffer(                  (p.itemc),	buf);
	for(i=0; i< MAX_PARTY; i++)
		_party_member_frombuffer(p.member[i], buf);
}
extern inline void party_frombuffer(struct party &p, const unsigned char *buf)
{
	_party_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_member {
	unsigned long account_id;
	unsigned long char_id;
	unsigned short hair;
	unsigned short hair_color;
	unsigned char gender;
	unsigned short class_;
	unsigned short lv;
	unsigned long exp;
	unsigned long exp_payper;
	unsigned short online;
	unsigned short position;
	unsigned long rsv1;
	unsigned long rsv2;
	char name[24];
	struct map_session_data *sd;
};
extern inline void _guild_member_tobuffer(const struct guild_member &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer(  (p.account_id),	buf);
	_L_tobuffer(  (p.char_id),		buf);
	_W_tobuffer( (p.hair),		buf);
	_W_tobuffer( (p.hair_color),	buf);
	_B_tobuffer(                 (p.gender),		buf);
	_W_tobuffer( (p.class_),		buf);
	_W_tobuffer(                 (p.lv),			buf);
	_L_tobuffer(  (p.exp),			buf);
	_L_tobuffer(  (p.exp_payper),	buf);
	_W_tobuffer( (p.online),		buf);
	_W_tobuffer( (p.position),	buf);
	_L_tobuffer(  (p.rsv1),		buf);
	_L_tobuffer(  (p.rsv2),		buf);
	_S_tobuffer(                  (p.name),		buf, 24);
	//_L_tobuffer( &(p.sd),			buf);
	(*buf)+=4;
	// skip the struct map_session_data *
}
extern inline void guild_member_tobuffer(const struct guild_member &p, unsigned char *buf)
{
	_guild_member_tobuffer(p, buf);
}
extern inline void _guild_member_frombuffer(struct guild_member &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer(  (p.account_id),	buf);
	_L_frombuffer(  (p.char_id),		buf);
	_W_frombuffer( (p.hair),		buf);
	_W_frombuffer( (p.hair_color),	buf);
	_B_frombuffer(                 (p.gender),		buf);
	_W_frombuffer( (p.class_),		buf);
	_W_frombuffer(                 (p.lv),			buf);
	_L_frombuffer(  (p.exp),			buf);
	_L_frombuffer(  (p.exp_payper),	buf);
	_W_frombuffer( (p.online),		buf);
	_W_frombuffer( (p.position),	buf);
	_L_frombuffer(  (p.rsv1),		buf);
	_L_frombuffer(  (p.rsv2),		buf);
	_S_frombuffer(                  (p.name),		buf, 24);
	//_L_frombuffer( &(p.sd),		buf);
	buf+=4; p.sd = NULL; 
	// skip the struct map_session_data *
}
extern inline void guild_member_frombuffer(struct guild_member &p, const unsigned char *buf)
{
	_guild_member_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_position {
	char name[24];
	unsigned long mode;
	unsigned long exp_mode;
};
extern inline void _guild_position_tobuffer(const struct guild_position &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer(                  (p.name),		buf, 24);
	_L_tobuffer(  (p.mode),		buf);
	_L_tobuffer(  (p.exp_mode),	buf);
}
extern inline void guild_position_tobuffer(const struct guild_position &p, unsigned char *buf)
{
	_guild_position_tobuffer(p, buf);
}
extern inline void _guild_position_frombuffer(struct guild_position &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer(                  (p.name),		buf, 24);
	_L_frombuffer(  (p.mode),		buf);
	_L_frombuffer(  (p.exp_mode),	buf);
}
extern inline void guild_position_frombuffer(struct guild_position &p, const unsigned char *buf)
{
	_guild_position_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_alliance {
	long opposition;
	unsigned long guild_id;
	char name[24];
};
extern inline void _guild_alliance_tobuffer(const struct guild_alliance &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_tobuffer(  (p.opposition),	buf);
	_L_tobuffer(  (p.guild_id),	buf);
	_S_tobuffer(                  (p.name),		buf, 24);
}
extern inline void guild_alliance_tobuffer(const struct guild_alliance &p, unsigned char *buf)
{
	_guild_alliance_tobuffer(p, buf);
}
extern inline void _guild_alliance_frombuffer(struct guild_alliance &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_L_frombuffer(  (p.opposition),buf);
	_L_frombuffer(  (p.guild_id),	buf);
	_S_frombuffer(                  (p.name),		buf, 24);
}
extern inline void guild_alliance_frombuffer(struct guild_alliance &p, const unsigned char *buf)
{
	_guild_alliance_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild_explusion {
	char name[24];
	char mes[40];
	char acc[40];
	unsigned long account_id;
	unsigned long char_id;
	unsigned long rsv1;
	unsigned long rsv2;
	unsigned long rsv3;
};
extern inline void _guild_explusion_tobuffer(const struct guild_explusion &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_tobuffer(                  (p.name),		buf, 24);
	_S_tobuffer(                  (p.mes),			buf, 40);
	_S_tobuffer(                  (p.acc),			buf, 40);
	_L_tobuffer(  (p.account_id),	buf);
	_L_tobuffer(  (p.char_id),	buf);
	_L_tobuffer(  (p.rsv1),		buf);
	_L_tobuffer(  (p.rsv2),		buf);
	_L_tobuffer(  (p.rsv3),		buf);
}
extern inline void guild_explusion_tobuffer(const struct guild_explusion &p, unsigned char *buf)
{
	_guild_explusion_tobuffer(p, buf);
}
extern inline void _guild_explusion_frombuffer(struct guild_explusion &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_S_frombuffer(                  (p.name),		buf, 24);
	_S_frombuffer(                  (p.mes),		buf, 40);
	_S_frombuffer(                  (p.acc),		buf, 40);
	_L_frombuffer(  (p.account_id),buf);
	_L_frombuffer(  (p.char_id),buf);
	_L_frombuffer(  (p.rsv1),		buf);
	_L_frombuffer(  (p.rsv2),		buf);
	_L_frombuffer(  (p.rsv3),		buf);
}
extern inline void guild_explusion_frombuffer(struct guild_explusion &p, const unsigned char *buf)
{
	_guild_explusion_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_skill {
	unsigned short id;
	unsigned short lv;
};
extern inline void _guild_skill_tobuffer(const struct guild_skill &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_tobuffer(  (p.id),	buf);
	_W_tobuffer(  (p.lv),	buf);
}
extern inline void guild_skill_tobuffer(const struct guild_skill &p, unsigned char *buf)
{
	_guild_skill_tobuffer(p, buf);
}
extern inline void _guild_skill_frombuffer(struct guild_skill &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer(  (p.id),buf);
	_W_frombuffer(  (p.lv),buf);
}
extern inline void guild_skill_frombuffer(struct guild_skill &p, const unsigned char *buf)
{
	_guild_skill_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct guild {
	unsigned long guild_id;
	unsigned short guild_lv;
	unsigned short connect_member;
	unsigned short max_member; 
	unsigned short average_lv;
	unsigned long exp;
	unsigned long next_exp;
	unsigned short skill_point;
	unsigned short castle_id;
	char name[24];
	char master[24];
	struct guild_member member[MAX_GUILD];
	struct guild_position position[MAX_GUILDPOSITION];
	char mes1[60];
	char mes2[120];
	unsigned long emblem_id;
	unsigned short emblem_len;
	unsigned char emblem_data[2048];
	struct guild_alliance alliance[MAX_GUILDALLIANCE];
	struct guild_explusion explusion[MAX_GUILDEXPLUSION];
	struct guild_skill skill[MAX_GUILDSKILL];
#ifndef TXT_ONLY
	unsigned char save_flag;
	int save_timer;
#endif
};
extern inline void _guild_tobuffer(const struct guild &p, unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer(  (p.guild_id),		buf);
	_W_tobuffer(                  (p.guild_lv),		buf);
	_W_tobuffer(                  (p.connect_member),	buf);
	_W_tobuffer(                  (p.max_member),		buf);
	_W_tobuffer(                  (p.average_lv),		buf);
	_L_tobuffer(  (p.exp),				buf);
	_L_tobuffer(  (p.next_exp),		buf);
	_W_tobuffer(                  (p.skill_point),		buf);
	_W_tobuffer(  (p.castle_id),		buf);
	_S_tobuffer(                  (p.name),			buf, 24);
	_S_tobuffer(                  (p.master),			buf, 24);
	for(i=0; i< MAX_GUILD; i++)
		_guild_member_tobuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; i++)
		_guild_position_tobuffer(p.position[i],	buf);
	_S_tobuffer(                  (p.mes1),			buf, 60);
	_S_tobuffer(                  (p.mes2),			buf, 120);
	_L_tobuffer(  (p.emblem_id),		buf);
	_W_tobuffer(                  (p.emblem_len),		buf);
	_S_tobuffer( (         char*) (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; i++)
		_guild_alliance_tobuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; i++)
		_guild_explusion_tobuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; i++)
		_guild_skill_tobuffer(p.skill[i],		buf);
}
extern inline void guild_tobuffer(const struct guild &p, unsigned char *buf)
{
	_guild_tobuffer(p, buf);
}
extern inline void _guild_frombuffer(struct guild &p, const unsigned char *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_frombuffer(  (p.guild_id),		buf);
	_W_frombuffer(                  (p.guild_lv),		buf);
	_W_frombuffer(                  (p.connect_member),	buf);
	_W_frombuffer(                  (p.max_member),		buf);
	_W_frombuffer(                  (p.average_lv),		buf);
	_L_frombuffer(  (p.exp),				buf);
	_L_frombuffer(  (p.next_exp),		buf);
	_W_frombuffer(                  (p.skill_point),		buf);
	_W_frombuffer(  (p.castle_id),		buf);
	_S_frombuffer(                  (p.name),			buf, 24);
	_S_frombuffer(                  (p.master),			buf, 24);
	for(i=0; i< MAX_GUILD; i++)
		_guild_member_frombuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; i++)
		_guild_position_frombuffer(p.position[i],	buf);
	_S_frombuffer(                  (p.mes1),			buf, 60);
	_S_frombuffer(                  (p.mes2),			buf, 120);
	_L_frombuffer(  (p.emblem_id),		buf);
	_W_frombuffer(                  (p.emblem_len),		buf);
	_S_frombuffer( (         char*) (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; i++)
		_guild_alliance_frombuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; i++)
		_guild_explusion_frombuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; i++)
		_guild_skill_frombuffer(p.skill[i],		buf);
}
extern inline void guild_frombuffer(struct guild &p, const unsigned char *buf)
{
	_guild_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_castle {
	unsigned short castle_id;
	char map_name[24];
	char castle_name[24];
	char castle_event[24];
	unsigned long guild_id;
	unsigned long economy;
	unsigned long defense;
	unsigned long triggerE;
	unsigned long triggerD;
	unsigned long nextTime;
	unsigned long payTime;
	unsigned long createTime;
	unsigned long visibleC;
	unsigned long visibleG0;
	unsigned long visibleG1;
	unsigned long visibleG2;
	unsigned long visibleG3;
	unsigned long visibleG4;
	unsigned long visibleG5;
	unsigned long visibleG6;
	unsigned long visibleG7;
	unsigned long Ghp0;	// added Guardian HP [Valaris]
	unsigned long Ghp1;
	unsigned long Ghp2;
	unsigned long Ghp3;
	unsigned long Ghp4;
	unsigned long Ghp5;
	unsigned long Ghp6;
	unsigned long Ghp7;	
	unsigned long GID0;	
	unsigned long GID1;
	unsigned long GID2;
	unsigned long GID3;
	unsigned long GID4;
	unsigned long GID5;
	unsigned long GID6;
	unsigned long GID7;	// end addition [Valaris]
};
extern inline void _guild_castle_tobuffer(const struct guild_castle &p, unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_tobuffer( (p.castle_id),	buf);
	_S_tobuffer(                  (p.map_name),	buf, 24);
	_S_tobuffer(                  (p.castle_name),	buf, 24);
	_S_tobuffer(                  (p.castle_event),buf, 24);
	_L_tobuffer(  (p.guild_id),	buf);
	_L_tobuffer(  (p.economy),		buf);
	_L_tobuffer(  (p.defense),		buf);
	_L_tobuffer(  (p.triggerE),	buf);
	_L_tobuffer(  (p.triggerD),	buf);
	_L_tobuffer(  (p.nextTime),	buf);
	_L_tobuffer(  (p.payTime),		buf);
	_L_tobuffer(  (p.createTime),	buf);
	_L_tobuffer(  (p.visibleC),	buf);
	_L_tobuffer(  (p.visibleG0),	buf);
	_L_tobuffer(  (p.visibleG1),	buf);
	_L_tobuffer(  (p.visibleG2),	buf);
	_L_tobuffer(  (p.visibleG3),	buf);
	_L_tobuffer(  (p.visibleG4),	buf);
	_L_tobuffer(  (p.visibleG5),	buf);
	_L_tobuffer(  (p.visibleG6),	buf);
	_L_tobuffer(  (p.visibleG7),	buf);
	_L_tobuffer(  (p.Ghp0),		buf);
	_L_tobuffer(  (p.Ghp1),		buf);
	_L_tobuffer(  (p.Ghp2),		buf);
	_L_tobuffer(  (p.Ghp3),		buf);
	_L_tobuffer(  (p.Ghp4),		buf);
	_L_tobuffer(  (p.Ghp5),		buf);
	_L_tobuffer(  (p.Ghp6),		buf);
	_L_tobuffer(  (p.Ghp7),		buf);
	_L_tobuffer(  (p.GID0),		buf);
	_L_tobuffer(  (p.GID1),		buf);
	_L_tobuffer(  (p.GID2),		buf);
	_L_tobuffer(  (p.GID3),		buf);
	_L_tobuffer(  (p.GID4),		buf);
	_L_tobuffer(  (p.GID5),		buf);
	_L_tobuffer(  (p.GID6),		buf);
	_L_tobuffer(  (p.GID7),		buf);
}
extern inline void guild_castle_tobuffer(const struct guild_castle &p, unsigned char *buf)
{
	_guild_castle_tobuffer(p, buf);
}
extern inline void _guild_castle_frombuffer(struct guild_castle &p, const unsigned char *&buf)
{
	if( NULL==buf )	return;
	_W_frombuffer( (p.castle_id),	buf);
	_S_frombuffer(                  (p.map_name),	buf, 24);
	_S_frombuffer(                  (p.castle_name),	buf, 24);
	_S_frombuffer(                  (p.castle_event),buf, 24);
	_L_frombuffer(  (p.guild_id),	buf);
	_L_frombuffer(  (p.economy),		buf);
	_L_frombuffer(  (p.defense),		buf);
	_L_frombuffer(  (p.triggerE),	buf);
	_L_frombuffer(  (p.triggerD),	buf);
	_L_frombuffer(  (p.nextTime),	buf);
	_L_frombuffer(  (p.payTime),		buf);
	_L_frombuffer(  (p.createTime),	buf);
	_L_frombuffer(  (p.visibleC),	buf);
	_L_frombuffer(  (p.visibleG0),	buf);
	_L_frombuffer(  (p.visibleG1),	buf);
	_L_frombuffer(  (p.visibleG2),	buf);
	_L_frombuffer(  (p.visibleG3),	buf);
	_L_frombuffer(  (p.visibleG4),	buf);
	_L_frombuffer(  (p.visibleG5),	buf);
	_L_frombuffer(  (p.visibleG6),	buf);
	_L_frombuffer(  (p.visibleG7),	buf);
	_L_frombuffer(  (p.Ghp0),		buf);
	_L_frombuffer(  (p.Ghp1),		buf);
	_L_frombuffer(  (p.Ghp2),		buf);
	_L_frombuffer(  (p.Ghp3),		buf);
	_L_frombuffer(  (p.Ghp4),		buf);
	_L_frombuffer(  (p.Ghp5),		buf);
	_L_frombuffer(  (p.Ghp6),		buf);
	_L_frombuffer(  (p.Ghp7),		buf);
	_L_frombuffer(  (p.GID0),		buf);
	_L_frombuffer(  (p.GID1),		buf);
	_L_frombuffer(  (p.GID2),		buf);
	_L_frombuffer(  (p.GID3),		buf);
	_L_frombuffer(  (p.GID4),		buf);
	_L_frombuffer(  (p.GID5),		buf);
	_L_frombuffer(  (p.GID6),		buf);
	_L_frombuffer(  (p.GID7),		buf);
}
extern inline void guild_castle_frombuffer(struct guild_castle &p, const unsigned char *buf)
{
	_guild_castle_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct square {
	unsigned long val1[5];
	unsigned long val2[5];
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#endif	// _MMO_H_
