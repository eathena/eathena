// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// Original : mmo.h 2003/03/14 12:07:02 Rev.1.7

#ifndef	_MMO_H_
#define	_MMO_H_

#include "basetypes.h"
#include "socket.h"
#include "utils.h"



///////////////////////////////////////////////////////////////////////////////
// defines
///////////////////////////////////////////////////////////////////////////////
#define FIFOSIZE_SERVERLINK	128*1024

#define CMP_AUTHFIFO_LOGIN2 1

#define MAX_MAP_PER_SERVER 1024
#define MAX_INVENTORY 100
#define MAX_ITEMS 20000
#define MAX_EQUIP 11
#define MAX_AMOUNT 30000
#define MAX_ZENY 1000000000	// 1G zeny
#define MAX_FAME 1000000000 // 1G fame point [DracoRPG]
#define MAX_FAMELIST 10
enum fame_t{ FAME_PK=0, FAME_SMITH, FAME_CHEM, FAME_TEAK };
#define MAX_CART 100
#define MAX_SKILL 650
#define MAX_REFINE 10
#define MAX_REFINE_BONUS 5
#define MAX_MEMO 10
#define MAX_GUARDIAN 8
#define MAX_SHOPITEM 128
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

#define MIN_HAIR_STYLE config.min_hair_style
#define MAX_HAIR_STYLE config.max_hair_style
#define MIN_HAIR_COLOR config.min_hair_color
#define MAX_HAIR_COLOR config.max_hair_color
#define MIN_CLOTH_COLOR config.min_cloth_color
#define MAX_CLOTH_COLOR config.max_cloth_color

// for produce
#define MAX_ATTRIBUTE 4
#define MAX_STAR 3
#define MAX_PORTAL_MEMO 3


#define WEDDING_RING_M 2634
#define WEDDING_RING_F 2635




/////////////////////////////////////////////////////////////////////////////
// guild enums
enum
{
	GBI_EXP			=1,		// ギルドのEXP
	GBI_GUILDLV		=2,		// ギルドのLv
	GBI_SKILLPOINT	=3,		// ギルドのスキルポイント
	GBI_SKILLLV		=4,		// ギルドスキルLv

	GMI_POSITION	=0,		// メンバーの役職変更
	GMI_EXP			=1,		// メンバーのEXP

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
	GD_DEVELOPMENT=10014
};

/////////////////////////////////////////////////////////////////////////////
// humunculus defined
#define MAX_HOMUN_DB 16			// ホムの数
#define HOM_ID 6001				// ID開始値
#define HOM_SKILLID 8001		// ホムスキルIDの開始値
#define MAX_HOM_SKILLID (HOM_SKILLID+MAX_HOMSKILL)	// ホムスキルIDの最大値
#define MAX_HOMSKILL 16



///////////////////////////////////////////////////////////////////////////////
/// class_id to jobname conversion
const char* job_name(unsigned short class_);
/// jobname to class_id conversion
unsigned short job_id(const char *jobname);
/// is valid jobid
bool job_isvalid(unsigned short class_);

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
extern inline void _L_tobuffer(const basics::ipaddress &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0xFF000000)>> 0x18 );
	*buf++ = (uchar)((valin & 0x00FF0000)>> 0x10 );
	*buf++ = (uchar)((valin & 0x0000FF00)>> 0x08 );
	*buf++ = (uchar)((valin & 0x000000FF)        );
}
extern inline void _T_tobuffer(const time_t &valin, uchar *&buf)
{	
	*buf++ = (uchar)((valin & 0xFF000000)>> 0x18 );
	*buf++ = (uchar)((valin & 0x00FF0000)>> 0x10 );
	*buf++ = (uchar)((valin & 0x0000FF00)>> 0x08 );
	*buf++ = (uchar)((valin & 0x000000FF)        );
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

extern inline void _L_frombuffer(basics::ipaddress &valin, const uchar *&buf)
{	
	valin =  ( ((uint32)(buf[3]))        )
			|( ((uint32)(buf[2])) << 0x08)
			|( ((uint32)(buf[1])) << 0x10)
			|( ((uint32)(buf[0])) << 0x18);
	buf += 4;
}
extern inline void _T_frombuffer(time_t &valin, const uchar *&buf)
{	
	valin =  ( ((uint32)(buf[3]))        )
			|( ((uint32)(buf[2])) << 0x08)
			|( ((uint32)(buf[1])) << 0x10)
			|( ((uint32)(buf[0])) << 0x18);
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


extern inline void _F_tobuffer(const float &valin, uchar *&buf)
{	
	// do it the dirty way
	union
	{
		uint32 i;
		float f;
	} cc;
	cc.f = valin;
	_L_tobuffer(cc.i, buf);
}
extern inline void _F_frombuffer(float &valin, const uchar *&buf)
{	
	// do it the dirty way
	union
	{
		uint32 i;
		float f;
	} cc;
	_L_frombuffer(cc.i, buf);
	valin = cc.f;
}







///////////////////////////////////////////////////////////////////////////////
// basic server storage struct
struct mmo_server
{
	int fd;
	basics::ipset address;
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
// transferable status changes
/////////////////////////////////////////////////////////////////////////////
struct sc_data
{
	ushort type; //SC_type
	uint32 val1;
	uint32 val2;
	uint32 val3;
	uint32 val4;
	uint32 tick; //Remaining duration (only 32bits used)

	sc_data() :
		type(0),
		val1(0),
		val2(0),
		val3(0),
		val4(0),
		tick(0)
	{}	
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



///////////////////////////////////////////////////////////////////////////////
// items
///////////////////////////////////////////////////////////////////////////////
struct item
{
	unsigned short	nameid;			// nameid of the corrosponding item_data

	unsigned short	amount;			// number of items in this stash
	unsigned short	equip;			//-> remove, make a better equip system

	unsigned char	identify;		// :1; identified (used as boolean only)
	unsigned char	refine;			// :4; stores number of refines (max 10)
	unsigned char	attribute;		// :1; broken (used as boolean only)

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

	item() : 
		nameid(0),
		amount(1),
		equip(0),
		identify(1),
		refine(0),
		attribute(0)
	{
		card[0]=0;
		card[1]=0;
		card[2]=0;
		card[3]=0;
	}
	explicit item(unsigned short id) : 
		nameid(id),
		amount(1),
		equip(0),
		identify(1),
		refine(0),
		attribute(0)
	{
		card[0]=0;
		card[1]=0;
		card[2]=0;
		card[3]=0;
	}
	friend bool operator==(const struct item& a, const struct item& b);
};

extern inline void _item_tobuffer(const struct item &p, uchar *&buf)
{
	if(NULL==buf )	return;
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

///////////////////////////////////////////////////////////////////////////////
// item compare
bool operator==(const struct item& a, const struct item& b);
inline bool operator!=(const struct item& a, const struct item& b)		{ return !(a==b); }



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct point
{
	char mapname[24];
	signed short x;
	signed short y;

	point() :
		x(0),
		y(0)
	{
		mapname[0]=0;
	}

	// create point from string
	// format is mapname, x, y
	explicit point(const char* str)
	{
		*this = str;
	}

	// create point from string
	// format is mapname, x, y
	const point& operator=(const char* str)
	{
		char nbuf[32]="";
		int nx=0, ny=0;
		if(str && *str && sscanf(str, "%30[^,],%d,%d", nbuf, &nx, &ny) == 3 )
		{	
			char *ip=strchr(nbuf, '.');
			if( ip != NULL ) *ip=0;
			safestrcpy(mapname, sizeof(mapname), nbuf);
			x = nx;
			y = ny;
		}
		return *this;
	}
	// create string from point
	// format is mapname, x, y
	friend basics::string<>& operator<<(basics::string<>& str, const struct point& p)
	{
		str << p.mapname << "," << p.x << "," << p.y;
		return str;
	}
	friend bool operator==(const struct point& a, const struct point& b)
	{
		int ret = strcasecmp(a.mapname,b.mapname);
		if( ret==0 ) ret = a.x-b.x;
		if( ret==0 ) ret = a.y-b.y;
		return ret==0;
	}
	friend bool operator< (const struct point& a, const struct point& b)
	{
		int ret = strcasecmp(a.mapname,b.mapname);
		if( ret==0 ) ret = a.x-b.x;
		if( ret==0 ) ret = a.y-b.y;
		return ret< 0;
	}
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

	global_reg() : value(0)
	{
		str[0]=0;
	}
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
struct petstatus
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
extern inline void _s_pet_tobuffer(const struct petstatus &p, uchar *&buf)
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
extern inline void s_pet_tobuffer(const struct petstatus &p, uchar *buf)
{
	_s_pet_tobuffer(p, buf);
}
extern inline void _s_pet_frombuffer(struct petstatus &p, const uchar *&buf)
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
extern inline void s_pet_frombuffer(struct petstatus &p, const uchar *buf)
{
	_s_pet_frombuffer(p, buf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

struct homunstatus
{
	uint32 homun_id;
	uint32 account_id;
	uint32 char_id;
	uint32 base_exp;
	char name[24];

	uint32 hp;
	uint32 max_hp;
	uint32 sp;
	uint32 max_sp;

	ushort class_;
	ushort status_point;
	ushort skill_point;
	
	ushort str;
	ushort agi;
	ushort vit;
	ushort int_;
	ushort dex;
	ushort luk;

	ushort option;
	ushort equip;

	uint32 intimate;	// ペットと違い、最大100,000で計算
	signed short hungry;
	ushort base_level;
	uchar rename_flag;
	uchar incubate;

	struct skill skill[MAX_HOMSKILL];
};

extern inline void _homun_tobuffer(const struct homunstatus &p, uchar *&buf)
{
	if( NULL==buf )	return;

	_L_tobuffer( (p.homun_id),		buf);
	_L_tobuffer( (p.account_id),	buf);
	_L_tobuffer( (p.char_id),		buf);
	_L_tobuffer( (p.base_exp),		buf);

	_S_tobuffer( (p.name),			buf, 24);

	_L_tobuffer( (p.hp),			buf);
	_L_tobuffer( (p.max_hp),		buf);
	_L_tobuffer( (p.sp),			buf);
	_L_tobuffer( (p.max_sp),		buf);

	_W_tobuffer( (p.class_),		buf);
	_W_tobuffer( (p.status_point),	buf);
	_W_tobuffer( (p.skill_point),	buf);


	_W_tobuffer( (p.str),			buf);
	_W_tobuffer( (p.agi),			buf);
	_W_tobuffer( (p.vit),			buf);
	_W_tobuffer( (p.int_),			buf);
	_W_tobuffer( (p.dex),			buf);
	_W_tobuffer( (p.luk),			buf);

	_W_tobuffer( (p.option),		buf);
	_W_tobuffer( (p.equip),			buf);


	_L_tobuffer( (p.intimate),		buf);
	_W_tobuffer( (p.hungry),		buf);
	_W_tobuffer( (p.base_level),	buf);
	_B_tobuffer( (p.rename_flag),	buf);
	_B_tobuffer( (p.incubate),		buf);

	for(size_t i=0; i<MAX_HOMSKILL; ++i)
		_skill_tobuffer(p.skill[i], buf);
}
extern inline void homun_tobuffer(const struct homunstatus &p, uchar *buf)
{
	_homun_tobuffer(p, buf);
}
extern inline void _homun_frombuffer(struct homunstatus &p, const uchar *&buf)
{
	if( NULL==buf )	return;

	_L_frombuffer( (p.homun_id),		buf);
	_L_frombuffer( (p.account_id),		buf);
	_L_frombuffer( (p.char_id),			buf);
	_L_frombuffer( (p.base_exp),		buf);

	_S_frombuffer( (p.name),			buf, 24);

	_L_frombuffer( (p.hp),				buf);
	_L_frombuffer( (p.max_hp),			buf);
	_L_frombuffer( (p.sp),				buf);
	_L_frombuffer( (p.max_sp),			buf);

	_W_frombuffer( (p.class_),			buf);
	_W_frombuffer( (p.status_point),	buf);
	_W_frombuffer( (p.skill_point),		buf);


	_W_frombuffer( (p.str),				buf);
	_W_frombuffer( (p.agi),				buf);
	_W_frombuffer( (p.vit),				buf);
	_W_frombuffer( (p.int_),			buf);
	_W_frombuffer( (p.dex),				buf);
	_W_frombuffer( (p.luk),				buf);

	_W_frombuffer( (p.option),			buf);
	_W_frombuffer( (p.equip),			buf);


	_L_frombuffer( (p.intimate),		buf);
	_W_frombuffer( (p.hungry),			buf);
	_W_frombuffer( (p.base_level),		buf);
	_B_frombuffer( (p.rename_flag),		buf);
	_B_frombuffer( (p.incubate),		buf);

	for(size_t i=0; i<MAX_HOMSKILL; ++i)
		_skill_frombuffer(p.skill[i],	buf);
}
extern inline void homun_frombuffer(struct homunstatus &p, const uchar *buf)
{
	_homun_frombuffer(p, buf);
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

	signed char karma;		// good<->evil		(-100...+100/saturation)
	signed char chaos;		// chaotic<->lawful	(-100...+100/saturation)
	signed short manner;	// stores negative values of mute
	signed short option;
	unsigned short hair;
	unsigned short hair_color;
	unsigned short clothes_color;

	uint32 party_id;
	uint32 guild_id;
	uint32 pet_id;
	uint32 homun_id;
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
	_L_tobuffer( (p.char_id),		buf);
	_L_tobuffer( (p.account_id),	buf);
	_L_tobuffer( (p.partner_id),	buf);
	_L_tobuffer( (p.father_id),		buf);
	_L_tobuffer( (p.mother_id),		buf);
	_L_tobuffer( (p.child_id),		buf);

	_L_tobuffer( (p.base_exp),		buf);
	_L_tobuffer( (p.job_exp),		buf);
	_L_tobuffer( (p.zeny),			buf);

	_W_tobuffer( (p.class_),		buf);
	_W_tobuffer( (p.status_point),	buf);
	_W_tobuffer( (p.skill_point),	buf);

	_L_tobuffer( (p.hp),			buf);
	_L_tobuffer( (p.max_hp),		buf);
	_L_tobuffer( (p.sp),			buf);
	_L_tobuffer( (p.max_sp),		buf);

	_B_tobuffer( (p.karma),			buf);
	_B_tobuffer( (p.chaos),			buf);
	_W_tobuffer( (p.manner),		buf);
	_W_tobuffer( (p.option),		buf);
	_W_tobuffer( (p.hair),			buf);
	_W_tobuffer( (p.hair_color),	buf);
	_W_tobuffer( (p.clothes_color),	buf);

	_L_tobuffer( (p.party_id),		buf);
	_L_tobuffer( (p.guild_id),		buf);
	_L_tobuffer( (p.pet_id),		buf);
	_L_tobuffer( (p.homun_id),		buf);
	
	_L_tobuffer( (p.fame_points),	buf);

	_W_tobuffer( (p.weapon),		buf);
	_W_tobuffer( (p.shield),		buf);
	_W_tobuffer( (p.head_top),		buf);
	_W_tobuffer( (p.head_mid),		buf);
	_W_tobuffer( (p.head_bottom),	buf);

	_S_tobuffer( p.name,			buf, 24);

	_W_tobuffer( (p.base_level),	buf);
	_W_tobuffer( (p.job_level),		buf);
	_W_tobuffer( (p.str),			buf);
	_W_tobuffer( (p.agi),			buf);
	_W_tobuffer( (p.vit),			buf);
	_W_tobuffer( (p.int_),			buf);
	_W_tobuffer( (p.dex),			buf);
	_W_tobuffer( (p.luk),			buf);

	_B_tobuffer( (p.slot),			buf);
	_B_tobuffer( (p.sex),			buf);
	_B_tobuffer( (p.gm_level),		buf);

	_L_tobuffer( (p.mapip),			buf);
	_W_tobuffer( (p.mapport),		buf);

	_point_tobuffer(             (p.last_point),		buf);
	_point_tobuffer(             (p.save_point),		buf);
	for(i=0; i<MAX_MEMO; ++i)
		_point_tobuffer(           p.memo_point[i],		buf);

	for(i=0; i<MAX_INVENTORY; ++i)
		_item_tobuffer(            p.inventory[i],		buf);

	for(i=0; i<MAX_CART; ++i)
		_item_tobuffer(            p.cart[i],			buf);

	for(i=0; i<MAX_SKILL; ++i)
		_skill_tobuffer(           p.skill[i],			buf);

	_L_tobuffer(  (p.global_reg_num),	buf);
	for(i=0; i<GLOBAL_REG_NUM; ++i)
		_global_reg_tobuffer(      p.global_reg[i],		buf);


	_L_tobuffer(  (p.account_reg_num),	buf);
	for(i=0; i<ACCOUNT_REG_NUM; ++i)
		_global_reg_tobuffer(      p.account_reg[i],	buf);

	_L_tobuffer(  (p.account_reg2_num),buf);
	for(i=0; i<ACCOUNT_REG2_NUM; ++i)
		_global_reg_tobuffer(      p.account_reg2[i],	buf);

	// Friends list vars
	for(i=0; i<MAX_FRIENDLIST; ++i)
		_L_tobuffer( (p.friendlist[i].friend_id),	buf);
	for(i=0; i<MAX_FRIENDLIST; ++i)
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
	_L_frombuffer( (p.homun_id),		buf);

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
	for(i=0; i<MAX_MEMO; ++i)
		_point_frombuffer(           p.memo_point[i],	buf);

	for(i=0; i<MAX_INVENTORY; ++i)
		_item_frombuffer(            p.inventory[i],	buf);

	for(i=0; i<MAX_CART; ++i)
		_item_frombuffer(            p.cart[i],			buf);

	for(i=0; i<MAX_SKILL; ++i)
		_skill_frombuffer(           p.skill[i],		buf);

	_L_frombuffer(  (p.global_reg_num),buf);
	for(i=0; i<GLOBAL_REG_NUM; ++i)
		_global_reg_frombuffer(      p.global_reg[i],	buf);


	_L_frombuffer(  (p.account_reg_num),buf);
	for(i=0; i<ACCOUNT_REG_NUM; ++i)
		_global_reg_frombuffer(      p.account_reg[i],	buf);

	_L_frombuffer(  (p.account_reg2_num),buf);
	for(i=0; i<ACCOUNT_REG2_NUM; ++i)
		_global_reg_frombuffer(      p.account_reg2[i],	buf);

	// Friends list vars
	for(i=0; i<MAX_FRIENDLIST; ++i)
		_L_frombuffer( (p.friendlist[i].friend_id),buf);
	for(i=0; i<MAX_FRIENDLIST; ++i)
		_S_frombuffer(               p.friendlist[i].friend_name,	buf, 24);
}
extern inline void mmo_charstatus_frombuffer(struct mmo_charstatus &p, const uchar *buf)
{
	_mmo_charstatus_frombuffer(p, buf);
}

///////////////////////////////////////////////////////////////////////////////
// Authentification Data used to authentify that clients are going from
// login through char to map.
// 
// uint32 login_id1;	// just a random id given by login
// uint32 login_id2;	// just a random id given by login
// uint32 client_ip;	// the current ip of the client
// 
// a client has to show these three values to get autentified
// (gets it in login process)
// 
///////////////////////////////////////////////////////////////////////////////
// Account Data which holds the necessary data for an account
// 
// uint32 account_id;			// id to identify an account
// char userid[24];				// user name
// char passwd[34];				// user password
// unsigned char sex;			// gender
// unsigned char gm_level;		// gm_level
// unsigned char online;		// true when online (actually only usefull when adding datamining onto the storing data and not onto the server)
// char email[40];				// email address for confiming char deletion
// uint32 login_count;			// number of logins
// char last_login[24];			// timestamp of last login
// char last_ip[16];			// last ip as string
// time_t ban_until;			// set to time(NULL)+delta for temporary ban
// time_t valid_until;			// set to time(NULL)+delta for temporary valid account or time(NULL) for complete disable
// 
// 
///////////////////////////////////////////////////////////////////////////////
// Account Reg for account wide variables:
// 
// unsigned short account_reg2_num;
// struct global_reg account_reg2[ACCOUNT_REG2_NUM];
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Authentification
///////////////////////////////////////////////////////////////////////////////
class CAuth
{
public:
	uint32 account_id;
	uint32 login_id1;
	uint32 login_id2;
	basics::ipaddress client_ip;

	CAuth(uint32 aid=0) : account_id(aid)	{}
	~CAuth()	{}

	///////////////////////////////////////////////////////////////////////////
	// sorting by accountid
	bool operator==(const CAuth& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CAuth& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CAuth& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CAuth& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CAuth& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CAuth& c) const { return this->account_id<=c.account_id; }

	///////////////////////////////////////////////////////////////////////////
	// comparing
	bool isEqual(const CAuth& a) const	{ return account_id==a.account_id && client_ip==a.client_ip && login_id1==a.login_id1 && login_id2==a.login_id2; }

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return 4*sizeof(uint32); }	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account Reg
///////////////////////////////////////////////////////////////////////////////
class CAccountReg
{
public:
	ushort account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

	CAccountReg()	{ account_reg2_num=0; memset(account_reg2,0,sizeof(account_reg2)); }
	~CAccountReg()	{}

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return sizeof(account_reg2_num)+ACCOUNT_REG2_NUM*sizeof(struct global_reg); }

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for map server
///////////////////////////////////////////////////////////////////////////////
class CMapAccount : public CAuth, public CAccountReg
{
public:
	unsigned char sex;
	unsigned char gm_level;
	time_t ban_until;
	time_t valid_until;

	CMapAccount()	{}
	~CMapAccount()	{}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid
	CMapAccount(uint32 aid):CAuth(aid)	{}
	bool operator==(const CMapAccount& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CMapAccount& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CMapAccount& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CMapAccount& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CMapAccount& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CMapAccount& c) const { return this->account_id<=c.account_id; }

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const;	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for char server
///////////////////////////////////////////////////////////////////////////////
class CCharAccount : public CMapAccount
{
public:
	char email[40];

	CCharAccount()	{}
	CCharAccount(uint32 aid):CMapAccount(aid)	{}
	~CCharAccount()	{}
	
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return sizeof(email)+CMapAccount::size();	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for login server
///////////////////////////////////////////////////////////////////////////////
class CLoginAccount : public CCharAccount
{
public:
	char userid[24];
	char passwd[34];
	unsigned char online;
	uint32 login_count;
	char last_login[24];
	char last_ip[16];

	CLoginAccount()	{}
	~CLoginAccount()	{}
	///////////////////////////////////////////////////////////////////////////
	// creation of a new account
	CLoginAccount(uint32 accid, const char* uid, const char* pwd, unsigned char sx, const char* em)
		: CCharAccount(accid)
	{	// init account data
		safestrcpy(this->userid, sizeof(this->userid), uid);
		safestrcpy(this->passwd, sizeof(this->passwd), pwd);
		this->sex = sx;
		if( !email_check(em) )
			safestrcpy(this->email, sizeof(this->email), "a@a.com");
		else
			safestrcpy(this->email, sizeof(this->email), em);
		this->gm_level=0;
		this->login_count=0;
		*this->last_login= 0;
		this->ban_until = 0;
		this->valid_until = 0;
		this->account_reg2_num=0;
	}
	CLoginAccount(const char* uid)		{ safestrcpy(this->userid, sizeof(this->userid), uid);  }
	CLoginAccount(uint32 accid)	{ this->account_id=accid; }

	const CLoginAccount& operator=(const CCharAccount&a)
	{
		this->CCharAccount::operator=(a);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CLoginAccount& c, size_t i=0) const	
	{
		if(i==0)
			return (account_id - c.account_id);
		else
			return strcmp(this->userid, c.userid); 
	}
	// no buffer transfer necessary
};

///////////////////////////////////////////////////////////////////////////////
/// mail structures.
/// the structure itself is fixed to the client order
///////////////////////////////////////////////////////////////////////////////
class CMailHead
{
public:
	uint32 msgid;
	char head[40];
	unsigned char read;
	char name[24];
	uint32 sendtime;

	CMailHead() : msgid(0), read(0), sendtime(0)
	{
		head[0]=0;
		name[0]=0;
	}
	CMailHead(uint32 id, unsigned char r, const char *n, time_t t, const char *h) : msgid(id), read(r), sendtime(t)
	{
		safestrcpy(name, sizeof(name),	n);
		safestrcpy(head, sizeof(head),	h);
	}
	~CMailHead()	{}
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	
	{
		return ( sizeof(msgid)+sizeof(read)+sizeof(name)+sizeof(head)+sizeof(sendtime) );	
	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const		// Put class into given buffer
	{
		_L_tobuffer(msgid,	buf);
		_S_tobuffer(head,	buf, sizeof(head));
		_B_tobuffer(read,	buf);
		_S_tobuffer(name,	buf, sizeof(name));
		_L_tobuffer(sendtime,buf);
	}
	void _frombuffer(const unsigned char* &buf)		// Get class from given buffer
	{
		_L_frombuffer(msgid,buf);
		_S_frombuffer(head,	buf, sizeof(head));
		_B_frombuffer(read,	buf);
		_S_frombuffer(name,	buf, sizeof(name));
		_L_frombuffer(sendtime,buf);
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

class CMail : public CMailHead
{
public:
	uint32 zeny;
	struct item item;
	char body[512];

	CMail() : zeny(0)
	{
		body[0] = 0;
	}
	CMail(uint32 id, unsigned char r, const char *n, const char *h, time_t t, uint32 z, const struct item& i, const char *b)
		: CMailHead(id, r, n, t, h), zeny(z), item(i)
	{
		safestrcpy(body, sizeof(body),	b);
	}
	~CMail()	{}


	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	
	{
		return ( CMailHead::size()+sizeof(body)+sizeof(zeny)+sizeof(item) );	
	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const		// Put class into given buffer
	{
		CMailHead::_tobuffer(buf);
		_L_tobuffer(zeny,	buf);
		_item_tobuffer(item,buf);
		_S_tobuffer(body,	buf, sizeof(body));
	}
	void _frombuffer(const unsigned char* &buf)		// Get class from given buffer
	{
		CMailHead::_frombuffer(buf);
		_L_frombuffer(zeny,	buf);
		_item_frombuffer(item,buf);
		_S_frombuffer(body,	buf, sizeof(body));
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};


///////////////////////////////////////////////////////////////////////////////
// char structures
///////////////////////////////////////////////////////////////////////////////
class CCharCharAccount : public CCharAccount
{
public:
	uint32 charlist[9];

	CCharCharAccount()
	{
		memset(charlist,0,sizeof(charlist));
	}
	~CCharCharAccount()		{}
	CCharCharAccount(const CCharAccount& c) : CCharAccount(c)	
	{
		memset(charlist,0,sizeof(charlist));
	}
	

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid
	CCharCharAccount(uint32 aid):CCharAccount(aid) {}
	bool operator==(const CCharAccount& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CCharAccount& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CCharAccount& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CCharAccount& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CCharAccount& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CCharAccount& c) const { return this->account_id<=c.account_id; }
};



class CCharCharacter : public mmo_charstatus
{
	int	server;
public:
	CCharCharacter():server(-1)		{}
	~CCharCharacter()		{}


	CCharCharacter(const char* n)	{ memset(this, 0, sizeof(CCharCharacter)); server=-1; safestrcpy(this->name, sizeof(this->name), n); }
	CCharCharacter(uint32 cid)	{ memset(this, 0, sizeof(CCharCharacter)); server=-1; this->char_id=cid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by charid

	bool operator==(const CCharCharacter& c) const { return this->char_id==c.char_id; }
	bool operator!=(const CCharCharacter& c) const { return this->char_id!=c.char_id; }
	bool operator> (const CCharCharacter& c) const { return this->char_id> c.char_id; }
	bool operator>=(const CCharCharacter& c) const { return this->char_id>=c.char_id; }
	bool operator< (const CCharCharacter& c) const { return this->char_id< c.char_id; }
	bool operator<=(const CCharCharacter& c) const { return this->char_id<=c.char_id; }


	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CCharCharacter& c, size_t i=0) const	
	{
		if(i==0)
			return (this->char_id - c.char_id);
		else
			return strcmp(this->name, c.name); 
	}

	void _tobuffer(unsigned char* &buf, bool new_charscreen) const;		// Put class into given buffer
	void tobuffer(unsigned char* buf, bool new_charscreen) const	{ _tobuffer(buf, new_charscreen); }		// Put class into given buffer
};

///////////////////////////////////////////////////////////////////////////////
// map structure
///////////////////////////////////////////////////////////////////////////////
class CMapCharacter : public CCharCharacter, public CAuth
{
public:
	CMapCharacter()			{}
	~CMapCharacter()		{}

};




///////////////////////////////////////////////////////////////////////////////
/// famelist.
/// stores topmost chars accoding fame
///////////////////////////////////////////////////////////////////////////////
class CFameList : public basics::noncopyable
{
public:
	typedef struct _fameentry
	{
		uint32	char_id;
		uint32	fame_points;
		char	name[24];

		_fameentry()
		{}
		_fameentry(uint32 c, const char *n, uint32 f) : char_id(c), fame_points(f)
		{
			safestrcpy(name,sizeof(name),n);
		}
	} fameentry;

	uchar		cCount;
	// MAX_FAMELIST limited to 255!!
	fameentry	cEntry[MAX_FAMELIST+1]; 
	// have one additional element to allow both shift up/shift down 
	// without re-reading the whole list

	CFameList() : cCount(0)
	{
		memset(cEntry, 0, sizeof(cEntry));
	}

	///////////////////////////////////////////////////////////////////////////
	/// returns number of elements in list.
	/// only counts MAX_FAMELIST elements, not the extra elment
	size_t count() const
	{
		return (cCount>MAX_FAMELIST)?MAX_FAMELIST:cCount;
	}
	///////////////////////////////////////////////////////////////////////////
	/// check if char_id is listed.
	/// only counts MAX_FAMELIST elements, not the extra elment
	bool exists(uint32 char_id) const
	{
		size_t p;
		for(p=0; p<cCount && p<MAX_FAMELIST; ++p)
		{
			if(cEntry[p].char_id == char_id)
				return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// clears the list.
	void clear()
	{
		if(cCount)
		{
			cCount=0;
			// actually not necessary to memset since explicitely counted
			memset(cEntry,0,sizeof(cEntry));
		}
	}
	///////////////////////////////////////////////////////////////////////////
	/// update.
	/// check if new entry can be inserted, returns true on changing list.
	/// the compares are done as lt (lower than) so new entries get 
	/// inserted only if they are larger in value.
	bool update(uint32	char_id, const char* name, uint32 points)
	{
		size_t p;
		// check if already in list
		for(p=0; p<cCount; ++p)
		{
			if(cEntry[p].char_id == char_id)
			{
				if(points==0)
				{	// removing
					--cCount; // decrement here, skip the -1 at size calculation
					memmove(cEntry+p, cEntry+p+1, (cCount-p)*sizeof(fameentry));
					return true;
				}
				else if(cEntry[p].fame_points != points)
					cEntry[p].fame_points = points;
				else
					return false;
				// check for bubbling up
				for( ; p && cEntry[p-1].fame_points<cEntry[p].fame_points; --p )
					basics::swap( cEntry[p], cEntry[p-1] );
				// check for bubbling down
				for( ; p+1<cCount && cEntry[p].fame_points<cEntry[p+1].fame_points; ++p )
					basics::swap( cEntry[p], cEntry[p+1] );
				return true;
			}
		}
		if(p>=cCount)
		{	// not found, look if it can be inserted
			for(p=cCount; p && cEntry[p-1].fame_points < points; --p);

			if(p<MAX_FAMELIST+1)
			{	// insert at p
				memmove(cEntry+p+1, cEntry+p, (cCount-p-1)*sizeof(fameentry));
				cEntry[p] = fameentry(char_id, name, points);
				if(cCount<MAX_FAMELIST+1) ++cCount;
				return true;
			}
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	/// element access.
	/// returns the element outside the MAX_FAMELIST count on out-of-bound access
	const fameentry& operator[](size_t i) const
	{
		return (i<MAX_FAMELIST)?cEntry[i]:cEntry[MAX_FAMELIST];
	}



	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	
	{
		return sizeof(cCount) + sizeof(cEntry);
	}	// Return size of class (which is maximum here)

	size_t _tobuffer(unsigned char* &buf) const		// Put class into given buffer
	{
		const unsigned char* tmp = buf;
		size_t i;
		_B_tobuffer(cCount, buf);
		for(i=0; i<cCount && i<MAX_FAMELIST; ++i)
		{
			_L_tobuffer(cEntry[i].char_id, buf);
			_L_tobuffer(cEntry[i].fame_points, buf);
			_S_tobuffer(cEntry[i].name, buf, 24);
		}
		return buf-tmp;
	}
	size_t _frombuffer(const unsigned char* &buf)		// Get class from given buffer
	{
		const unsigned char* tmp = buf;
		size_t i;
		_B_frombuffer(cCount, buf);
		for(i=0; i<cCount && i<MAX_FAMELIST; ++i)
		{
			_L_frombuffer(cEntry[i].char_id, buf);
			_L_frombuffer(cEntry[i].fame_points, buf);
			_S_frombuffer(cEntry[i].name, buf, 24);
		}
		cCount = i;
		// clear the remaining
		for( ; i < MAX_FAMELIST; ++i)
			cEntry[i].char_id=cEntry[i].fame_points=cEntry[i].name[0]=0;
		return buf-tmp;
	}
	size_t tobuffer(unsigned char* buf) const	{ return _tobuffer(buf); }		// Put class into given buffer
	size_t frombuffer(const unsigned char* buf) { return _frombuffer(buf); } // Get class from given buffer
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct pc_storage
{
	uint32 dirty;
	uint32 account_id;
	signed short storage_status;
	signed short storage_amount;
	struct item storage[MAX_STORAGE];

	pc_storage() : 
		dirty(0),
		account_id(0),
		storage_status(0),
		storage_amount(0)
	{}
	explicit pc_storage(uint32 id) : 
		dirty(0),
		account_id(id),
		storage_status(0),
		storage_amount(0)
	{}
};
extern inline void _pc_storage_tobuffer(const struct pc_storage &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.dirty),				buf);
	_L_tobuffer( (p.account_id),		buf);
	_W_tobuffer( (p.storage_status),	buf);
	_W_tobuffer( (p.storage_amount),	buf);
	for(i=0; i<MAX_STORAGE; ++i)
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
	for(i=0; i<MAX_STORAGE; ++i)
		_item_frombuffer(p.storage[i],	buf);
}
extern inline void pc_storage_frombuffer(struct pc_storage &p, const uchar *buf)
{
	_pc_storage_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct guild_storage
{
	uint32 guild_id;
	signed short storage_status;
	signed short storage_amount;
	struct item storage[MAX_GUILD_STORAGE];

	guild_storage() :
		guild_id(0),
		storage_status(0),
		storage_amount(0)
	{}
	explicit guild_storage(uint32 id) :
		guild_id(id),
		storage_status(0),
		storage_amount(0)
	{}
};
extern inline void _guild_storage_tobuffer(const struct guild_storage &p, uchar *&buf)
{
	size_t i;
	if( NULL==buf )	return;
	_L_tobuffer( (p.guild_id),			buf);
	_W_tobuffer( (p.storage_status),	buf);
	_W_tobuffer( (p.storage_amount),	buf);
	for(i=0; i<MAX_STORAGE; ++i)
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
	for(i=0; i<MAX_STORAGE; ++i)
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

// predeclaration
struct map_session_data;

struct party_member
{
	uint32 account_id;
	char name[24];
	char mapname[24];
	uchar leader;
	uchar online;
	unsigned short lv;
	map_session_data *sd;

	party_member() :
		account_id(0),
		leader(0),
		online(0),
		lv(0),
		sd(NULL)
	{
		name[0]=0;
		mapname[0]=0;
	}

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
	buf+=sizeof(map_session_data *);
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
	buf+=sizeof(map_session_data *); 
	p.sd = NULL; 
	// skip the map_session_data *
}
extern inline void party_member_frombuffer(struct party_member &p, const uchar *buf)
{
	_party_member_frombuffer(p, buf);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct party
{
	uint32 party_id;
	char name[24];
	unsigned short expshare;
	unsigned short itemshare;
	unsigned short itemc;
	struct party_member member[MAX_PARTY];

	party(uint32 id=0) :
		party_id(id),
		expshare(0),
		itemshare(0),
		itemc(0)
	{
		name[0]=0;
	}
	explicit party(uint32 id, const char* n) :
		party_id(id),
		expshare(0),
		itemshare(0),
		itemc(0)
	{
		safestrcpy(this->name,sizeof(this->name), n);
	}
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
	for(i=0; i< MAX_PARTY; ++i)
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
	for(i=0; i< MAX_PARTY; ++i)
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
	map_session_data *sd;
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
	buf+=sizeof(map_session_data *);
	// skip the map_session_data *
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
	buf+=sizeof(map_session_data *);
	p.sd = NULL; 
	// skip the map_session_data *
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

#define GUILD_SAFE_GUILD	0x01	// safe basic guild
#define GUILD_SAFE_POSITION	0x02	// safe pos
#define GUILD_SAFE_ALLIANCE	0x04	// safe alliance
#define GUILD_SAFE_SKILL	0x08	// safe skill
#define GUILD_SAFE_EXPULSE	0x10	// safe expulsion
#define GUILD_SAFE_MEMBER	0x10	// safe members
// 0x40
// 0x80
#define GUILD_CLEAR_MEMBER	0x80	// clear members before safine
#define GUILD_SAFE_ALL		0xFF	// safe all

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
	uchar save_flags;
	int save_timer;
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
	for(i=0; i< MAX_GUILD; ++i)
		_guild_member_tobuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; ++i)
		_guild_position_tobuffer(p.position[i],	buf);
	_S_tobuffer( (p.mes1),				buf, 60);
	_S_tobuffer( (p.mes2),				buf, 120);
	_L_tobuffer( (p.emblem_id),			buf);
	_W_tobuffer( (p.emblem_len),		buf);
	_X_tobuffer( (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; ++i)
		_guild_alliance_tobuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; ++i)
		_guild_explusion_tobuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; ++i)
		_guild_skill_tobuffer(p.skill[i],		buf);
	_B_tobuffer( (p.save_flags),		buf);

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
	for(i=0; i< MAX_GUILD; ++i)
		_guild_member_frombuffer(p.member[i],		buf);
	for(i=0; i< MAX_GUILDPOSITION; ++i)
		_guild_position_frombuffer(p.position[i],	buf);
	_S_frombuffer( (p.mes1),			buf, 60);
	_S_frombuffer( (p.mes2),			buf, 120);
	_L_frombuffer( (p.emblem_id),		buf);
	_W_frombuffer( (p.emblem_len),		buf);
	_X_frombuffer( (p.emblem_data),		buf, 2048);
	for(i=0; i< MAX_GUILDALLIANCE; ++i)
		_guild_alliance_frombuffer(p.alliance[i],	buf);
	for(i=0; i< MAX_GUILDEXPLUSION; ++i)
		_guild_explusion_frombuffer(p.explusion[i],buf);
	for(i=0; i< MAX_GUILDSKILL; ++i)
		_guild_skill_frombuffer(p.skill[i],		buf);
	_B_frombuffer( (p.save_flags),		buf);
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

	guild_castle(unsigned short id=0) :
		castle_id(id),
		guild_id(0),
		economy(0),
		defense(0),
		triggerE(0),
		triggerD(0),
		nextTime(0),
		payTime(0),
		createTime(0),
		visibleC(0)
	{
		mapname[0] = 0;
		castle_name[0] = 0;
		castle_event[0] = 0;
	}
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

	for(i=0; i<MAX_GUARDIAN; ++i)
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
	for(i=0; i<MAX_GUARDIAN; ++i)
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





///////////////////////////////////////////////////////////////////////////////
// Guild Expirience Definition
///////////////////////////////////////////////////////////////////////////////
class CGuildExp
{
	uint32 exp[100];
public:
	CGuildExp()	{ memset(exp,0,sizeof(exp)); }
	void init(const char* filename);

	uint32 operator[](size_t i)	{ --i; return (i<sizeof(exp)/sizeof(exp[0])) ? exp[i] : 0; }
};
///////////////////////////////////////////////////////////////////////////////
// Guild Class Definition
///////////////////////////////////////////////////////////////////////////////
class CGuild : public guild
{
public:
	static CGuildExp cGuildExp;

	CGuild()					{  }
	CGuild(const char* n)		{ memset(this, 0, sizeof(CGuild)); safestrcpy(this->name, sizeof(this->name), n); }
	CGuild(uint32 gid)			{ memset(this, 0, sizeof(CGuild)); this->guild_id=gid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CGuild& c) const { return this->guild_id==c.guild_id; }
	bool operator!=(const CGuild& c) const { return this->guild_id!=c.guild_id; }
	bool operator> (const CGuild& c) const { return this->guild_id> c.guild_id; }
	bool operator>=(const CGuild& c) const { return this->guild_id>=c.guild_id; }
	bool operator< (const CGuild& c) const { return this->guild_id< c.guild_id; }
	bool operator<=(const CGuild& c) const { return this->guild_id<=c.guild_id; }


	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CGuild& c, size_t i=0) const;

	///////////////////////////////////////////////////////////////////////////
	// class internal functions

	unsigned short checkSkill(unsigned short id);

	bool calcInfo();
	int is_empty();
};
///////////////////////////////////////////////////////////////////////////////
// Guild Castle Class Definition
///////////////////////////////////////////////////////////////////////////////
class CCastle : public guild_castle
{
public:
	CCastle()	{}
	CCastle(ushort cid)				{ memset(this, 0, sizeof(CCastle)); this->castle_id=cid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by id
	bool operator==(const CCastle& c) const { return this->castle_id==c.castle_id; }
	bool operator!=(const CCastle& c) const { return this->castle_id!=c.castle_id; }
	bool operator> (const CCastle& c) const { return this->castle_id> c.castle_id; }
	bool operator>=(const CCastle& c) const { return this->castle_id>=c.castle_id; }
	bool operator< (const CCastle& c) const { return this->castle_id< c.castle_id; }
	bool operator<=(const CCastle& c) const { return this->castle_id<=c.castle_id; }
};






#endif	// _MMO_H_
