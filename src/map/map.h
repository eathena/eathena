// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_H_
#define _MAP_H_

#include "script.h"
#include "config.h"
#include "mapobj.h"



///////////////////////////////////////////////////////////////////////////////
#define MAX_PC_CLASS 4050
#define PC_CLASS_BASE 0
#define PC_CLASS_BASE2 (PC_CLASS_BASE + 4001)
#define PC_CLASS_BASE3 (PC_CLASS_BASE2 + 22)
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE ((int)config.area_size)
#define LOCAL_REG_NUM 16
#define LIFETIME_FLOORITEM 60
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_SKILL_LEVEL 100
#define MAX_STATUSCHANGE 250
#define MAX_SKILLUNITGROUP 32
#define MAX_MOBSKILLUNITGROUP 8
#define MAX_SKILLUNITGROUPTICKSET 32
#define MAX_SKILLTIMERSKILL 32
#define MAX_MOBSKILLTIMERSKILL 10
#define MAX_MOBSKILL 32
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MAX_LEVEL 255
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 80
#define MAX_VENDING 12
#define MAX_TRADING 10

///////////////////////////////////////////////////////////////////////////////
//Definitions for Jobs, this should help code be more readable. [Skotlex]
#define JOB_NOVICE 0
#define JOB_SWORDMAN 1
#define JOB_MAGE 2
#define JOB_ARCHER 3
#define JOB_ACOLYTE 4
#define JOB_MERCHANT 5
#define JOB_THIEF 6
#define JOB_KNIGHT 7
#define JOB_PRIEST 8
#define JOB_WIZARD 9
#define JOB_BLACKSMITH 10
#define JOB_HUNTER 11
#define JOB_ASSASSIN 12
#define JOB_KNIGHT2 13
#define JOB_CRUSADER 14
#define JOB_MONK 15
#define JOB_SAGE 16
#define JOB_ROGUE 17
#define JOB_ALCHEMIST 18
#define JOB_BARD 19
#define JOB_DANCER 20
#define JOB_CRUSADER2 21
#define JOB_WEDDING 22
#define JOB_SUPER_NOVICE 23

#define JOB_NOVICE_HIGH 4001
#define JOB_SWORDMAN_HIGH 4002
#define JOB_MAGE_HIGH 4003
#define JOB_ARCHER_HIGH 4004
#define JOB_ACOLYTE_HIGH 4005
#define JOB_MERCHANT_HIGH 4006
#define JOB_THIEF_HIGH 4007
#define JOB_LORD_KNIGHT 4008
#define JOB_HIGH_PRIEST 4009
#define JOB_HIGH_WIZARD 4010
#define JOB_WHITESMITH 4011
#define JOB_SNIPER 4012
#define JOB_ASSASSIN_CROSS 4013
#define JOB_LORD_KNIGHT2 4014
#define JOB_PALADIN 4015
#define JOB_CHAMPION 4016
#define JOB_PROFESSOR 4017
#define JOB_STALKER 4018
#define JOB_CREATOR 4019
#define JOB_CLOWN 4020
#define JOB_GYPSY 4021
#define JOB_PALADIN2 4022

#define JOB_BABY 4023
#define JOB_BABY_SWORDMAN 4024
#define JOB_BABY_MAGE 4025
#define JOB_BABY_ARCHER 4026
#define JOB_BABY_ACOLYTE 4027
#define JOB_BABY_MERCHANT 4028
#define JOB_BABY_THIEF 4029
#define JOB_BABY_KNIGHT 4030
#define JOB_BABY_PRIEST 4031
#define JOB_BABY_WIZARD 4032
#define JOB_BABY_BLACKSMITH 4033
#define JOB_BABY_HUNTER  4034
#define JOB_BABY_ASSASSIN 4035
#define JOB_BABY_KNIGHT2 4036
#define JOB_BABY_CRUSADER 4037
#define JOB_BABY_MONK 4038
#define JOB_BABY_SAGE 4039
#define JOB_BABY_ROGUE 4040
#define JOB_BABY_ALCHEMIST 4041
#define JOB_BABY_BARD 4042
#define JOB_BABY_DANCER 4043
#define JOB_BABY_CRUSADER2 4044
#define JOB_SUPER_BABY 4045
#define JOB_TAEKWON 4046
#define JOB_STAR_GLADIATOR 4047
#define JOB_STAR_GLADIATOR2 4048
#define JOB_SOUL_LINKER 4049


///////////////////////////////////////////////////////////////////////////////
#define OPTION_MASK		0xd7b8
#define CART_MASK		0x788
#define STATE_BLIND		0x10
#define MAX_SKILL_TREE	53
#define MOBID_EMPERIUM	1288

///////////////////////////////////////////////////////////////////////////////
#define EFFECT_FOG		515
#define EFFECT_SNOW		162
#define EFFECT_LEAVES	333
#define EFFECT_CLOUDS	233
#define EFFECT_CLOUDS2	516
#define EFFECT_SAKURA	163
#define EFFECT_RAIN		161
#define EFFECT_FIRE1	297
#define EFFECT_FIRE2	299
#define EFFECT_FIRE3	301

#define EFFECT_BIG		423
#define EFFECT_TINY		421



///////////////////////////////////////////////////////////////////////////////
#define FLAG_DISGUISE	0x80000000 // set the msb of the acount_id to signal a disguise


///////////////////////////////////////////////////////////////////////////////
#define DEFAULT_AUTOSAVE_INTERVAL 60*1000

///////////////////////////////////////////////////////////////////////////////
#define OPTION_HIDE 0x40




enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// 囲まれペナルティ計算用

// 装備コード
enum {
	EQP_WEAPON		= 1,	// 右手
	EQP_ARMOR		= 2,	// 体
	EQP_SHIELD		= 4,	// 左手
	EQP_HELM		= 8		// 頭上段
};







///////////////////////////////////////////////////////////////////////////////
/// skill types. (equivalent to INF_*)
/// TODO is this a bit field or not
enum skilltype_t
{
	ST_PASSIVE = 0x00,// not target, can't be casted
	ST_ENEMY   = 0x01,// the skill targets an enemy
	ST_GROUND  = 0x02,// the skill targets the ground
	ST_SELF    = 0x04,// the skill targets self
	ST_FRIEND  = 0x10,// the skill targets a friend
	ST_TRAP    = 0x20 // the skill lays a trap
};



enum {
	SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,	// 0-7
	SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,	// 8-15
	SP_INT,SP_DEX,SP_LUK,SP_CLASS,SP_ZENY,SP_SEX,SP_NEXTBASEEXP,SP_NEXTJOBEXP,	// 16-23
	SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,	// 24-31
	SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,	// 32-39
	SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,	// 40-47
	SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,	// 48-55
	SP_UPPER,SP_PARTNER,SP_CART,SP_FAME,SP_UNBREAKABLE,	//56-60
	SP_CHAOS, // 61
	SP_CARTINFO=99,	// 99

	SP_BASEJOB=119,	// 100+19 - celest

	// original 1000-
	SP_ATTACKRANGE=1000,	SP_ATKELE,SP_DEFELE,	// 1000-1002
	SP_CASTRATE, SP_MAXHPRATE, SP_MAXSPRATE, SP_SPRATE, // 1003-1006
	SP_ADDELE, SP_ADDRACE, SP_ADDSIZE, SP_SUBELE, SP_SUBRACE, // 1007-1011
	SP_ADDEFF, SP_RESEFF,	// 1012-1013
	SP_BASE_ATK,SP_ASPD_RATE,SP_HP_RECOV_RATE,SP_SP_RECOV_RATE,SP_SPEED_RATE, // 1014-1018
	SP_CRITICAL_DEF,SP_NEAR_ATK_DEF,SP_LONG_ATK_DEF, // 1019-1021
	SP_DOUBLE_RATE, SP_DOUBLE_ADD_RATE, SP_MATK, SP_MATK_RATE, // 1022-1025
	SP_IGNORE_DEF_ELE,SP_IGNORE_DEF_RACE, // 1026-1027
	SP_ATK_RATE,SP_SPEED_ADDRATE,SP_ASPD_ADDRATE, // 1028-1030
	SP_MAGIC_ATK_DEF,SP_MISC_ATK_DEF, // 1031-1032
	SP_IGNORE_MDEF_ELE,SP_IGNORE_MDEF_RACE, // 1033-1034
	SP_MAGIC_ADDELE,SP_MAGIC_ADDRACE,SP_MAGIC_SUBRACE, // 1035-1037
	SP_PERFECT_HIT_RATE,SP_PERFECT_HIT_ADD_RATE,SP_CRITICAL_RATE,SP_GET_ZENY_NUM,SP_ADD_GET_ZENY_NUM, // 1038-1042
	SP_ADD_DAMAGE_CLASS,SP_ADD_MAGIC_DAMAGE_CLASS,SP_ADD_DEF_CLASS,SP_ADD_MDEF_CLASS, // 1043-1046
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_ADD_SPEED, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_RANDOM_ATTACK_INCREASE,SP_ALL_STATS,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_DISGUISE,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_ATK_RATE, // 1081-1082
	SP_DELAYRATE,	// 1083
	
	SP_RESTART_FULL_RECOVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_INFINITE_ENDURE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_ADD_DAMAGE_BY_CLASS, // 2018-2020
	SP_SP_GAIN_VALUE, SP_IGNORE_DEF_MOB, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_DAMAGE_WHEN_UNEQUIP, SP_ADD_ITEM_HEAL_RATE, SP_LOSESP_WHEN_UNEQUIP, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_ADDEFF_WHENHIT_SHORT,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD,  // 2034-2037
	SP_INTRAVISION, SP_ADD_MONSTER_DROP_ITEMGROUP, SP_SP_LOSS_RATE // 2038-2040
};

enum
{
	LOOK_BASE,LOOK_HAIR,LOOK_WEAPON,LOOK_HEAD_BOTTOM,LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HAIR_COLOR,LOOK_CLOTHES_COLOR,LOOK_SHIELD,LOOK_SHOES
};




















///////////////////////////////////////////////////////////////////////////////
/// cell on a map.
/// holds the values from .gat & dynamic things
struct mapcell_t
{
	enum cell_t
	{
		GAT_NONE		= 0,	// normal ground
		GAT_WALL		= 1,	// not passable and blocking
		GAT_UNUSED1		= 2,
		GAT_WATER		= 3,	// water
		GAT_UNUSED2		= 4,
		GAT_GROUND		= 5,	// not passable but can shoot/cast over it
		GAT_HOLE		= 6,	// holes in morroc desert
		GAT_UNUSED3		= 7,
		CELL_MASK=0x07	// 3 bit for cell mask
	};

	unsigned char type : 3;		// 3bit used for land,water,wall,(hole) (values 0,1,3,5,6 used)
								// providing 4 bit space and interleave two cells in x dimension
								// would not waste memory too much; will implement it later on a new map model
	unsigned char npc : 4;		// 4bit counter for npc touchups, can hold 15 touchups;
	unsigned char basilica : 1;	// 1bit for basilica (is on/off for basilica enough, what about two casting priests?)
	unsigned char moonlit : 1;	// 1bit for moonlit
	unsigned char regen : 1;	// 1bit for regen
	unsigned char icewall : 1;	// 1bits for icewall, leave type intact
	unsigned char _unused : 5;	// 5 bits left
	// will alloc a short

	mapcell_t(int t=0)
		: type(t&0x07)
		, npc(0)
		, basilica(0)
		, moonlit(0)
		, regen(0)
		, _unused(0)
	{}
	void set_type(int t)
	{
		this->type=t&0x07;
	}
	int get_type() const
	{
		return this->type;
	}

	bool is_passable() const
	{
		return (this->type != GAT_WALL && this->type != GAT_GROUND && !this->icewall );
	}
	bool is_wall() const
	{
		return (this->type == GAT_WALL);
	}
	bool is_ground() const
	{
		return (this->type == GAT_GROUND) && !this->icewall;
	}
	bool is_water() const
	{
		return (this->type == GAT_WATER);
	}
	bool is_quicksand() const
	{
		return (this->type == GAT_HOLE);
	}

	bool is_npc() const
	{
		return this->npc;
	}
	void set_npc()
	{
		if(this->npc<0xF)
			++this->npc;
	}
	void clr_npc()
	{
		if(this->npc>0)
			--this->npc;
	}

	bool is_basilica() const
	{
		return this->basilica;
	}
	void set_basilica()
	{
		this->basilica=1;
	}
	void clr_basilica()
	{
		this->basilica=0;
	}
	
	bool is_moonlit() const
	{
		return this->moonlit;
	}
	void set_moonlit()
	{
		this->moonlit=1;
	}
	void clr_moonlit()
	{
		this->moonlit=0;
	}

	bool is_regen() const
	{
		return this->regen;
	}
	void set_regen()
	{
		this->regen=1;
	}
	void clr_regen()
	{
		this->regen=0;
	}

	bool is_icewall() const
	{
		return this->icewall;
	}
	void set_icewall()
	{
		this->icewall=1;
	}
	void clr_icewall()
	{
		this->icewall=0;
	}
};




///////////////////////////////////////////////////////////////////////////////
/// predecl
struct map_intern;
struct map_extern;

///////////////////////////////////////////////////////////////////////////////
/// base type for maps
struct map_base
{
	virtual map_intern*			get_local()			{ return NULL; }
	virtual const map_intern*	get_local() const	{ return NULL; }
	virtual map_extern*			get_extern()		{ return NULL; }
	virtual const map_extern*	get_extern() const	{ return NULL; }
	char mapname[24];
	map_base()
	{
		mapname[0]=0;
	}
	map_base(const char* name)
	{
		buffer2mapname(mapname, sizeof(mapname), name);
	}
	virtual ~map_base()
	{}
};



///////////////////////////////////////////////////////////////////////////////
/// internal maps with terain info.
struct map_intern : public map_base
{
	struct _objects
	{
		block_list*	root_blk;
		block_list*	root_mob;
		uint		cnt_blk;
		uint		cnt_mob;
		_objects() :
			root_blk(NULL),
			root_mob(NULL),
			cnt_blk(0),
			cnt_mob(0)
		{}
	};

	virtual map_intern*			get_local()			{ return this; }
	virtual const map_intern*	get_local() const	{ return this; }

	mapcell_t	*gat;
	_objects *objects;
	unsigned short xs;
	unsigned short ys;
	unsigned short bxs;
	unsigned short bys;
	size_t npc_num;
	size_t users;
	struct mapflags
	{
		unsigned nomemo : 1;					//  0
		unsigned noteleport : 1;				//  1
		unsigned noreturn : 1;					//  2
		unsigned monster_noteleport : 1;		//  3
		unsigned nosave : 1;					//  4
		unsigned nobranch : 1;					//  5
		unsigned nopenalty : 1;					//  6
		unsigned pvp : 1;						//  7 (byte 1)
		unsigned pvp_noparty : 1;				//  8
		unsigned pvp_noguild : 1;				//  9
		unsigned pvp_nightmaredrop :1;			// 10
		unsigned pvp_nocalcrank : 1;			// 11
		unsigned gvg : 1;						// 12
		unsigned gvg_noparty : 1;				// 13
		unsigned gvg_dungeon : 1;				// 14
		unsigned nozenypenalty : 1;				// 15 (byte 2)
		unsigned notrade : 1;					// 16
		unsigned noskill : 1;					// 17
		unsigned nowarp : 1;					// 18
		unsigned nowarpto : 1;					// 19
		unsigned nopvp : 1;						// 20
		unsigned noicewall : 1;					// 21
		unsigned snow : 1;						// 22
		unsigned rain : 1;						// 23 (byte 3)
		unsigned sakura : 1;					// 24
		unsigned leaves : 1;					// 25
		unsigned clouds : 1;					// 26
		unsigned clouds2 : 1;					// 27
		unsigned fog : 1;						// 28
		unsigned fireworks : 1;					// 29
		unsigned indoors : 1;					// 30
		unsigned nogo : 1;						// 31 (byte 4)
		unsigned nobaseexp	: 1;				// 32
		unsigned nojobexp	: 1;				// 33
		unsigned nomobloot	: 1;				// 34
		unsigned nomvploot	: 1;				// 35
		unsigned _unused : 4;					// 36-39 (byte 5)

		mapflags()
			: nomemo(0)
			, noteleport(0)
			, noreturn(0)
			, monster_noteleport(0)
			, nosave(0)
			, nobranch(0)
			, nopenalty(0)
			, pvp(0)
			, pvp_noparty(0)
			, pvp_noguild(0)
			, pvp_nightmaredrop(0)
			, pvp_nocalcrank(0)
			, gvg(0)
			, gvg_noparty(0)
			, gvg_dungeon(0)
			, nozenypenalty(0)
			, notrade(0)
			, noskill(0)
			, nowarp(0)
			, nowarpto(0)
			, nopvp(0)
			, noicewall(0)
			, snow(0)
			, rain(0)
			, sakura(0)
			, leaves(0)
			, clouds(0)
			, clouds2(0)
			, fog(0)
			, fireworks(0)
			, indoors(0)
			, nogo(0)
			, nobaseexp(0)
			, nojobexp(0)
			, nomobloot(0)
			, nomvploot(0)
			, _unused(0)
		{}
	} flag;
	struct point nosave;
	npc_data *npc[MAX_NPC_PER_MAP];
	struct
	{
		int drop_id;
		int drop_type;
		int drop_per;
	}
	drop_list[MAX_DROP_PER_MAP];

	///////////////////////////////////////////////////////////////////////////
	struct mob_list *moblist[MAX_MOB_LIST_PER_MAP];
	int mob_delete_timer;

	///////////////////////////////////////////////////////////////////////////
	typedef mapcell_t cell_t;


	///////////////////////////////////////////////////////////////////////////
	map_intern();
	map_intern(const char* name);
	virtual ~map_intern()
	{}

	///////////////////////////////////////////////////////////////////////////
	// use default copy/assignment
	// which is inherently dangerous as this struct contains dynamic data
	// will change on the new design

	///////////////////////////////////////////////////////////////////////////
	bool initialize();
	void clear();

	///////////////////////////////////////////////////////////////////////////
	const mapcell_t& operator()(unsigned short x, unsigned short y) const;
	mapcell_t& operator()(unsigned short x, unsigned short y);

	///////////////////////////////////////////////////////////////////////////
	int get_type(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).get_type();
	}
	void set_type(unsigned short x, unsigned short y, int t)
	{
		this->operator()(x,y).set_type(t);
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_passable(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_passable();
	}
	bool is_wall(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_wall();
	}
	bool is_ground(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_ground();
	}
	bool is_water(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_water();
	}
	bool is_quicksand(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_quicksand();
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_npc(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_npc();
	}
	void set_npc(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).set_npc();
	}
	void clr_npc(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).clr_npc();
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_basilica(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_basilica();
	}
	void set_basilica(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).set_basilica();
	}
	void clr_basilica(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).clr_basilica();
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_moonlit(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_moonlit();
	}
	void set_moonlit(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).set_moonlit();
	}
	void clr_moonlit(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).clr_moonlit();
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_regen(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_regen();
	}
	void set_regen(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).set_regen();
	}
	void clr_regen(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).clr_regen();
	}
	///////////////////////////////////////////////////////////////////////////
	bool is_icewall(unsigned short x, unsigned short y) const
	{
		return this->operator()(x,y).is_icewall();
	}
	void set_icewall(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).set_icewall();
	}
	void clr_icewall(unsigned short x, unsigned short y)
	{
		this->operator()(x,y).clr_icewall();
	}

	///////////////////////////////////////////////////////////////////////////
	int addnpc(npc_data *nd);


	int searchrandfreecell(unsigned short x, unsigned short y, unsigned short range);

	int countoncell(int x, int y, object_t type);
	int foreachinmovearea(const CMapProcessor& elem, int x0, int y0, int x1, int y1, int dx, int dy, object_t type);
	int foreach(const CMapProcessor& elem, object_t type);
	int foreachinarea(const CMapProcessor& elem, int x0,int y0,int x1,int y1,object_t type);
	int foreachinarea(const CMapProcessor& elem, int x,int y, int range, object_t type)
	{
		return this->foreachinarea(elem, x-range, y-range, x+range, y+range, type);
	}
	int foreachinpath(const CMapProcessor& elem, int x0, int y0, int x1, int y1, int range, object_t type);
	int foreachincell(const CMapProcessor& elem, int x, int y, object_t type);


	///////////////////////////////////////////////////////////////////////////
private:
	static int moblist_timer(int tid, unsigned long tick, int id, basics::numptr data);
public:
	struct mob_list* moblist_create();
	void moblist_clear();
	void moblist_spawn();
	bool moblist_release();

private:
	static mapcell_t dummy_wall;

public:
	friend void swap(map_intern& a, map_intern&b);
};
void swap(map_intern& a, map_intern&b);


///////////////////////////////////////////////////////////////////////////////
/// external map with ip of the hosting map server
struct map_extern : public map_base
{
	virtual map_extern*			get_extern()		{ return this; }
	virtual const map_extern*	get_extern() const	{ return this; }

	basics::ipset mapset;
	map_intern* map;

	map_extern(const char* name, const basics::ipset& ip, map_intern* m)
		: map_base(name)
		, mapset(ip)
		, map(m)
	{}
	virtual ~map_extern()
	{}
};




///////////////////////////////////////////////////////////////////////////////
/// map array wrapper
struct _map
{
private:
	dbt* map_db;
	struct map_intern map_array[MAX_MAP_PER_SERVER];
	size_t map_num;
public:
	typedef map_intern			value_type;
	typedef map_intern*			pointer_type;
	typedef map_intern&			reference_type;
	typedef map_intern*			iterator;
	typedef const map_intern*	const_iterator;

	_map() : map_db(NULL), map_num(0)
	{}
	~_map()
	{
		this->clear();
	}

	map_intern& operator[](size_t i);
	const map_intern& operator[](size_t i) const;

	size_t size() const;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	size_t capacity();

	void clear();
private:
	iterator erase(iterator pos);
	void push_back(const map_intern& x);
public:
	iterator search(const char *name);
	const_iterator search(const char *name) const
	{
		return const_cast<_map*>(this)->search(name);
	}
	int index_of(const char *name) const;
	int index_of(const map_intern& md) const;

	bool mapname2ipport(const char *name, basics::ipset &mapset);
	int setipport(const char *name, basics::ipset &mapset);
	int eraseipport(const char *name, basics::ipset &mapset);
	int eraseallipport(void);


	void initialize();
	void finalize();

	int addmap(const char *mapname);
	int delmap(const char *mapname);

	void loadallmaps();
};

extern _map maps;







extern int autosave_interval;
extern int agit_flag;


extern int map_read_flag; // 0: grfｫﾕｫ｡ｫ､ｫ・1: ｫｭｫ罩ﾃｫｷｫ・2: ｫｭｫ罩ﾃｫｷｫ・?)
enum {
	READ_FROM_GAT, 
	READ_FROM_AFM,
	READ_FROM_BITMAP, 
	READ_FROM_BITMAP_COMPRESSED
};

extern char motd_txt[];
extern char help_txt[];

extern char talkie_mes[];

extern char wisp_server_name[];






///////////////////////////////////////////////////////////////////////////////

// 鯖全体情報
void map_setusers(int fd);
int map_getusers(void);
void map_helpscreen();




extern const char *LOG_CONF_NAME;
extern const char *MAP_CONF_NAME;
extern const char *BATTLE_CONF_FILENAME;
extern const char *COMMAND_CONF_FILENAME;
extern const char *SCRIPT_CONF_NAME;
extern const char *MSG_CONF_NAME;
extern const char *GRF_PATH_FILENAME;







///////////////////////////////////////////////////////////////////////////////
// day-night cycle
extern int daynight_flag;		// 0=day, 1=night
extern int daynight_timer_tid;	// timer for night.day
int map_daynight_timer(int tid, unsigned long tick, int id, basics::numptr data);
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// permanent name storage
const char*	map_src_namedb(uint32 charid);
bool		map_req_namedb(const map_session_data &sd, uint32 charid);
void		map_add_namedb(uint32 charid, const char *name);
///////////////////////////////////////////////////////////////////////////////



#endif// _MAP_H_

