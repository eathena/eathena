// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_H_
#define _MAP_H_

#include "mmo.h"
#include "socket.h"
#include "db.h"
#include "script.h"
#include "coordinate.h"
#include "path.h"
#include "config.h"

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
#define MAX_EVENTQUEUE 2
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MAX_FLOORITEM 500000
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



/*
enum { 
	MS_IDLE,
	MS_WALK,
	MS_ATTACK,
	MS_DEAD,
	MS_DELAY };
*/

enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// 囲まれペナルティ計算用

// 装備コード
enum {
	EQP_WEAPON		= 1,		// 右手
	EQP_ARMOR		= 2,		// 体
	EQP_SHIELD		= 4,		// 左手
	EQP_HELM		= 8,		// 頭上段
};



///////////////////////////////////////////////////////////////////////////////
enum object_t { BL_NUL, BL_PC, BL_NPC, BL_MOB, BL_ITEM, BL_CHAT, BL_SKILL, BL_PET, BL_HOM };	//xxx 0..7 -> 3bit
enum object_sub_t { WARP, SHOP, SCRIPT, MONS };											// 0..3 -> 2bit




/*
external base classes
doubled linked list	-> 2 pointers -> 8[16] byte

mapbase			a virtual interface for maps
				derives:
				data:
				members:
				............
				just the base class for map and foreign_map (which are "maps on other host")
				............

map				the actual map
				derives:	mapbase
				data:		mapdata, blockroots ...
				members:	pathsearch, mapiterators (replacement for the foreach_..)
				............


mapobject		something that can be added to a map, a simple immobile object with id
				derives:	doubled linked list
				data:		ui id, us map, us posx, us posy, (ch bdir,ch hdir)	-> +10 (+12) byte
				members:	whatever is done with mapobjects
				............
				the directions might fit in here since they fill the data structure to a 4byte boundary
				(which is done by the compiler anyway) and (almost) any derived element is using it
				maybe also combine it with a status bitfield since dir's only use 3 bit
				............

movable			move interface, allows moving objects
				derives:	block
				data:		
				members:	
				............

battleobj		battle interface, hp/sp, skill
				derives:	movable
				data:		
				members:	
				............

script			script interface
				derives:	
				data:		
				members:	
				............

npc/warp/shop	script interface, move interface (script controlled) [warp,shop as special derivated, no unions]
				derives:	movable, script
				data:		
				members:	
				............

pet/mob/merc	script interface, move/battle interface (script/ai controlled)
				derives:	battleobj, (script)
				data:		
				members:	
				............

player/homun	client contolled  move/battle interface, socket processor
				derives:	battleobj
				data:		
				members:	
				............

*/


///////////////////////////////////////////////////////////////////////////////
// predeclarations
struct block_list;
struct skill_unit_group;
struct item_data;
struct pet_db;

struct movable;
struct fightable;
struct map_session_data;
struct npc_data;
struct mob_data;
struct pet_data;
class flooritem_data;
class chat_data;
class npcchat_data;
struct skill_unit;
class homun_data;









///////////////////////////////////////////////////////////////////////////////
/// processing base class.
/// process member is called for each block
///////////////////////////////////////////////////////////////////////////////
class CMapProcessor : public basics::noncopyable
{
protected:
	CMapProcessor()				{}
public:
	virtual ~CMapProcessor()	{}
	virtual int process(block_list& bl) const = 0;
};



///////////////////////////////////////////////////////////////////////////////
/// object on a map
struct block_list : public coordinate
{
	/////////////////////////////////////////////////////////////////
	static dbt* id_db;

	/// returns blocklist from given id
	static block_list* from_blid(uint32 id);

	// functions that work on block_lists
	static int foreachinarea(const CMapProcessor& elem, unsigned short m, int x0,int y0,int x1,int y1,int type);
	static int foreachincell(const CMapProcessor& elem, unsigned short m,int x,int y,int type);
	static int foreachinmovearea(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,int type);
	static int foreachinpath(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int range,int type);
	static int foreachpartymemberonmap(const CMapProcessor& elem, struct map_session_data &sd, int type);
	static int foreachobject(const CMapProcessor& elem,int type);


	/////////////////////////////////////////////////////////////////
	unsigned short m;
//private:
	unsigned char type;
public:
	unsigned char subtype;
	uint32 id;
//private:
	block_list *next;
	block_list *prev;
public:

	/////////////////////////////////////////////////////////////////
	/// default constructor.
	block_list(uint32 i=0, unsigned char t=0, unsigned char s=0) : 
		m(0),
		type(t),
		subtype(s),
		id(i),
		next(NULL),
		prev(NULL)
	{}
	/////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~block_list()	
	{
		// not yet removed from map
		if(this->prev)
			this->delblock();
		// and just to be sure that it's removed from iddb
		this->deliddb();
	}

	// missing:
	// all interactions with map 
	// which should be directly wrapped into a static interface
	// any comments? [Hinoko]

	// we'll leave it global right now and wrap it later
	// just move the basic functions here for the moment
	// and take care for the necessary refinements [Shinomori]

	/// add block to identifier database
	void addiddb();
	/// remove block from identifier database
	void deliddb();

	/// add block to map
	bool addblock();
	/// remove block from map
	bool delblock();

	/// check if a block in on a map
	bool is_on_map() const
	{
		return this->prev!=NULL;
	}

	// collision free deleting of blocks
	int freeblock();
	static int freeblock_lock(void);
	static int freeblock_unlock(void);



	// New member functions by Aru to cleanup code and 
	// automate sanity checking when converting block_list
	// to map_session_data, mob_data or pet_data

	// change all access to virtual overloads
	// so these might be not necessary when finished, 
	// also the type and subtype members will be obsolete
	// since objects can identify themselfs [Shinomori]

	// ok some more explanation on this; 
	// simple example in virtual overloads:
	/*
	// predeclaration
	class derive1;
	class derive2;

	// base class
	class base		
	{
	public:
		// virtual conversion operators to derived class
		// initialized with returning NULL since base class 
		// is neither of type derived1 nor of derived2
		virtual operator derive1*()	{return NULL;}
		virtual operator derive2*()	{return NULL;}
	};


	class derive1 : public base
	{
	public:
		// virtual conversion operator
		// only the one that gets overloaded in this class
		// actually only returns a pointer to the class itself
		virtual operator derive1*()	{return this;}
	};

	class derive2 : public base
	{
	public:
		// virtual conversion operator
		// only the one that gets overloaded in this class
		// actually only returns a pointer to the class itself
		virtual operator derive2*()	{return this;}
	};

	{
		// instanciate the two objects
		derive1 d1;
		derive2 d2;

		// get the pointers
		base *b1 = &d1;
		base *b2 = &d2;

		// from here on we can work with pointers to base objects
		// but the object can identify itself, in this case here
		// by using the conversion operator, 
		// any other function would be also possible
		
		derive1 *pd11 = *b1;	// since b1 points to a d1 object, it used the d1 conversion
		derive1 *pd12 = *b2;	// this conersion was not overloaded and will return NULL
								// meaning that b2 does not point to a derived1 object

		derive2 *pd21 = *b1;	// vise versa
		derive2 *pd22 = *b2;
	}

	*/


	// might later replace the type compare
	// this here can be overloaded and could also used with masks instead pure enums
	virtual bool is_type(object_t t)
	{
		return (t==this->type);
	}
	bool operator==(object_t t)
	{
		return this->is_type(t);
	}
	bool operator!=(object_t t)
	{
		return !this->is_type(t);
	}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual map_session_data*		get_sd()				{ return NULL; }
	virtual const map_session_data* get_sd() const			{ return NULL; }

	virtual pet_data*				get_pd()				{ return NULL; }
	virtual const pet_data*			get_pd() const			{ return NULL; }

	virtual mob_data*				get_md()				{ return NULL; }
	virtual const mob_data*			get_md() const			{ return NULL; }

	virtual npc_data*				get_nd()				{ return NULL; }
	virtual const npc_data*			get_nd() const			{ return NULL; }

	virtual homun_data*				get_hd()				{ return NULL; }
	virtual const homun_data*		get_hd() const			{ return NULL; }

	virtual chat_data*				get_cd()				{ return NULL; }
	virtual const chat_data*		get_cd() const			{ return NULL; }

	virtual skill_unit*				get_sk()				{ return NULL; }
	virtual const skill_unit*		get_sk() const			{ return NULL; }

	virtual flooritem_data*			get_fd()				{ return NULL; }
	virtual const flooritem_data*	get_fd() const			{ return NULL; }

	virtual movable*				get_movable()			{ return NULL; }
	virtual const movable*			get_movable() const		{ return NULL; }

	virtual fightable*				get_fightable()			{ return NULL; }
	virtual const fightable*		get_fightable() const	{ return NULL; }



	///////////////////////////////////////////////////////////////////////////
	/// return head direction.
	/// default function, needs to be overloaded at specific implementations
	virtual dir_t get_headdir() const		{ return DIR_S; }
	/// return body direction.
	/// default function, needs to be overloaded at specific implementations
	virtual dir_t get_bodydir() const		{ return DIR_S; }
	/// alias to body direction.
	virtual dir_t get_dir() const			{ return DIR_S; }

	///////////////////////////////////////////////////////////////////////////
	/// set directions seperately.
	virtual void set_dir(dir_t b, dir_t h)	{}
	/// set both directions equally.
	virtual void set_dir(dir_t d)			{}
	/// set directions to look at target
	virtual void set_dir(const coordinate& to)	{}
	/// set body direction only.
	virtual void set_bodydir(dir_t d)		{}
	/// set head direction only.
	virtual void set_headdir(dir_t d)		{}



	///////////////////////////////////////////////////////////////////////////
	// status functions
	// overloaded at derived classes

	/// checks for walking state
	virtual bool is_walking() const		{ return false; }
	/// checks for attack state
	virtual bool is_attacking() const	{ return false; }
	/// checks for skill state
	virtual bool is_skilling() const	{ return false; }
	/// checks for dead state
	virtual bool is_dead() const		{ return false; }
	/// checks for sitting state
	virtual bool is_sitting() const		{ return false; }
	/// checks for idle state (alive+not sitting+not blocked by skill)
	virtual bool is_idle() const		{ return true; }
	/// checks for flying state
	virtual bool is_flying() const		{ return false; }
	/// checks for invisible state
	virtual bool is_invisible() const	{ return false; }
	/// checks for confusion state
	virtual bool is_confuse() const		{ return false; }
	/// checks if this is attackable
	virtual bool is_attackable() const	{ return false; }

	/// sets the object to idle state
	virtual bool set_idle(ulong delaytick=0){ return false; }
	/// sets the object to idle state
	virtual bool set_dead()				{ return false; }
	/// sets the object to sitting state
	virtual bool set_sit()				{ return false; }
	/// sets the object to standing state
	virtual bool set_stand()			{ return false; }




	///////////////////////////////////////////////////////////////////////////
	// walking functions
	// overloaded at derived classes

	/// walk to position
	virtual bool walktoxy(const coordinate& pos,bool easy=false)						{ return false; }
	/// walk to a coordinate
	virtual bool walktoxy(unsigned short x,unsigned short y,bool easy=false)			{ return false; }
	/// stop walking
	virtual bool stop_walking(int type=1)												{ return false; }
	/// instant position change
	virtual bool movepos(const coordinate &target)										{ return false; }
	/// instant position change
	virtual bool movepos(unsigned short x,unsigned short y)								{ return false; }
	/// warps to a given map/position. 
	virtual bool warp(unsigned short m, unsigned short x, unsigned short y, int type)	{ return false; }


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	/// starts attack
	virtual bool start_attack(uint32 target_id, bool cont)		{ return false; }
	/// starts attack
	virtual bool start_attack(const block_list& target_bl, bool cont)	{ return false; }
	/// stops attack
	virtual bool stop_attack()									{ return false; }
	/// stops skill
	virtual bool stop_skill()									{ return false; }
	// skill failed message.
	virtual int clif_skill_failed(ushort skill_id, uchar type=0, ushort btype=0)	{ return 0; }


	virtual int heal(int hp, int sp=0)	{ return 0; }

	///////////////////////////////////////////////////////////////////////////
	// targeting functions

	/// unlock from current target
	virtual void unlock_target()							{ }

};


///////////////////////////////////////////////////////////////////////////////
struct status_change
{
	int timer;
	basics::numptr val1;
	basics::numptr val2;
	basics::numptr val3;
	basics::numptr val4;

	// default constructor
	status_change() :
		timer(-1),
		val1(),
		val2(),
		val3(),
		val4()
	{ }
};

///////////////////////////////////////////////////////////////////////////////
struct vending_element
{
	unsigned short index;
	unsigned short amount;
	uint32 value;

	// default constructor
	vending_element() : 
		index(0),
		amount(0),
		value(0)
	{}
};

///////////////////////////////////////////////////////////////////////////////
struct weapon_data
{
	long watk;
	long watk2;

	long overrefine;
	long star;
	long atk_ele;
	long atkmods[3];
	long addsize[3];
	long addele[10];
	long addrace[12];
	long addrace2[12];
	long ignore_def_ele;
	long ignore_def_race;
	short ignore_def_mob;
	long def_ratio_atk_ele;
	long def_ratio_atk_race;
	long add_damage_class_count;
	short add_damage_classid[10];
	long add_damage_classrate[10];
	short hp_drain_rate;
	short hp_drain_per;
	short hp_drain_value;
	short sp_drain_rate;
	short sp_drain_per;
	short sp_drain_value;

	unsigned fameflag : 1;

	// default constructor
	weapon_data() : 
		watk(0),
		watk2(0),
		overrefine(0),
		star(0),
		atk_ele(0),
		ignore_def_ele(0),
		ignore_def_race(0),
		ignore_def_mob(0),
		def_ratio_atk_ele(0),
		def_ratio_atk_race(0),
		add_damage_class_count(0),
		hp_drain_rate(0),
		hp_drain_per(0),
		hp_drain_value(0),
		sp_drain_rate(0),
		sp_drain_per(0),
		sp_drain_value(0),
		fameflag(0)
	{
		memset(atkmods,0,sizeof(atkmods));
		memset(addsize,0,sizeof(addsize));
		memset(addele,0,sizeof(addele));
		memset(addrace,0,sizeof(addrace));
		memset(addrace2,0,sizeof(addrace2));
		memset(add_damage_classid,0,sizeof(add_damage_classid));
		memset(add_damage_classrate,0,sizeof(add_damage_classrate));
	}
};

///////////////////////////////////////////////////////////////////////////////
struct skill_unit : public block_list
{
	struct skill_unit_group *group;

	long limit;
	long val1;
	long val2;
	short alive;
	short range;

	skill_unit() :
		group(NULL),
		limit(0),
		val1(0),
		val2(0),
		alive(0),
		range(0)
	{}
	virtual ~skill_unit()
	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t)
	{
		return t==BL_SKILL;
	}
	virtual skill_unit*				get_sk()				{ return this; }
	virtual const skill_unit*		get_sk() const			{ return this; }


private:
	skill_unit(const skill_unit&);					// forbidden
	const skill_unit& operator=(const skill_unit&);	// forbidden
};
struct skill_unit_group
{
	uint32 src_id;
	uint32 party_id;
	uint32 guild_id;
	unsigned short map;
	long target_flag;
	unsigned long tick;
	long limit;
	long interval;

	unsigned short skill_id;
	unsigned short skill_lv;
	long val1;
	long val2;
	long val3;
	basics::string<> valstring;
	unsigned char unit_id;
	long group_id;
	long unit_count;
	long alive_count;
	struct skill_unit *unit;

	skill_unit_group() :
		src_id(0),
		party_id(0),
		guild_id(0),
		map(0),
		target_flag(0),
		tick(0),
		limit(0),
		interval(0),
		skill_id(0),
		skill_lv(0),
		val1(0),
		val2(0),
		val3(0),
		unit_id(0),
		group_id(0),
		unit_count(0),
		alive_count(0),
		unit(NULL)
	{}
};

struct skill_unit_group_tickset
{
	unsigned short skill_id;
	unsigned long tick;

	skill_unit_group_tickset() :
		skill_id(0),
		tick(0)
	{}
};

struct skill_timerskill
{
	uint32 src_id;
	uint32 target_id;
	unsigned short map;
	unsigned short x;
	unsigned short y;
	unsigned short skill_id;
	unsigned short skill_lv;
	int timer;
	long type;
	long flag;

	skill_timerskill() : 
		src_id(0),
		target_id(0),
		map(0),
		x(0),
		y(0),
		skill_id(0),
		skill_lv(0),
		timer(-1),
		type(0),
		flag(0)
	{}
};





















///////////////////////////////////////////////////////////////////////////////
class flooritem_data : public block_list
{
public:

	/////////////////////////////////////////////////////////////////
	static flooritem_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_fd():NULL;
	}
	/////////////////////////////////////////////////////////////////


	unsigned char subx;
	unsigned char suby;
	int cleartimer;
	uint32 first_get_id;
	uint32 second_get_id;
	uint32 third_get_id;
	uint32 first_get_tick;
	uint32 second_get_tick;
	uint32 third_get_tick;
	struct item item_data;

	flooritem_data() :
		subx(0), suby(0), cleartimer(-1),
		first_get_id(0), second_get_id(0), third_get_id(0),
		first_get_tick(0), second_get_tick(0), third_get_tick(0)
	{ }
	virtual ~flooritem_data()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t)
	{
		return t==BL_ITEM;
	}
	virtual flooritem_data*			get_fd()				{ return this; }
	virtual const flooritem_data*	get_fd() const			{ return this; }


private:
	flooritem_data(const flooritem_data&);					// forbidden
	const flooritem_data& operator=(const flooritem_data&);	// forbidden
};












///////////////////////////////////////////////////////////////////////////////

// map_getcell()/map_setcell()で使用されるフラグ
typedef enum { 
	CELL_CHKWALL=0,		// 壁(セルタイプ1)
	CELL_CHKWATER,		// 水場(セルタイプ3)
	CELL_CHKGROUND,		// 地面障害物(セルタイプ5)
	CELL_CHKPASS,		// 通過可能(セルタイプ1,5以外)
	CELL_CHKNOPASS,		// 通過不可(セルタイプ1,5)
	CELL_CHKHOLE,		// a hole in morroc desert
	CELL_GETTYPE,		// セルタイプを返す
	CELL_CHKNOPASS_NPC,

	CELL_CHKNPC=0x10,	// タッチタイプのNPC(セルタイプ0x80フラグ)
	CELL_SETNPC,		// タッチタイプのNPCをセット
	CELL_CLRNPC,		// タッチタイプのNPCをclear suru
	CELL_CHKBASILICA,	// バジリカ(セルタイプ0x40フラグ)
	CELL_SETBASILICA,	// バジリカをセット
	CELL_CLRBASILICA,	// バジリカをクリア
	CELL_CHKMOONLIT,
	CELL_SETMOONLIT,
	CELL_CLRMOONLIT,
	CELL_CHKREGEN,
	CELL_SETREGEN,
	CELL_CLRREGEN
} cell_t;

// CELL
#define CELL_MASK		0x07	// 3 bit for cell mask

// celests new stuff
//#define CELL_MOONLIT	0x100
//#define CELL_REGEN		0x200

enum {
	GAT_NONE		= 0,	// normal ground
	GAT_WALL		= 1,	// not passable and blocking
	GAT_UNUSED1		= 2,
	GAT_WATER		= 3,	// water
	GAT_UNUSED2		= 4,
	GAT_GROUND		= 5,	// not passable but can shoot/cast over it
	GAT_HOLE		= 6,	// holes in morroc desert
	GAT_UNUSED3		= 7,
};

struct mapgat // values from .gat & 
{
	unsigned char type : 3;		// 3bit used for land,water,wall,(hole) (values 0,1,3,5,6 used)
								// providing 4 bit space and interleave two cells in x dimension
								// would not waste memory too much; will implement it later on a new map model
	unsigned char npc : 4;		// 4bit counter for npc touchups, can hold 15 touchups;
	unsigned char basilica : 1;	// 1bit for basilica (is on/off for basilica enough, what about two casting priests?)
	unsigned char moonlit : 1;	// 1bit for moonlit
	unsigned char regen : 1;	// 1bit for regen
	unsigned char _unused : 6;	// 6 bits left

	mapgat() :
		type(0),npc(0),basilica(0),moonlit(0),regen(0),_unused(0)
	{}
};
// will alloc a short now


struct map_data
{
	///////////////////////////////////////////////////////////////////////////
	char mapname[24];
	struct mapgat	*gat;	// NULLなら下のmap_data_other_serverとして扱う
	///////////////////////////////////////////////////////////////////////////

	struct _objects
	{
		block_list*	root_blk;
		block_list*	root_mob;
		uint				cnt_blk;
		uint				cnt_mob;
		_objects() :
			root_blk(NULL),
			root_mob(NULL),
			cnt_blk(0),
			cnt_mob(0)
		{}
	} *objects;
	int m;
	unsigned short xs;
	unsigned short ys;
	unsigned short bxs;
	unsigned short bys;
	int wh;
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
	} flag;
	struct point save;
	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct {
		int drop_id;
		int drop_type;
		int drop_per;
	} drop_list[MAX_DROP_PER_MAP];
	struct mob_list *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int mob_delete_timer;	// [Skotlex]
};

struct map_data_other_server
{
	///////////////////////////////////////////////////////////////////////////
	char name[24];
	struct mapgat *gat;	// NULL固定にして判断
	///////////////////////////////////////////////////////////////////////////
	basics::ipset mapset;
	struct map_data* map;

	map_data_other_server() : 
		gat(NULL),
		map(NULL)
	{
		name[0]=0;
	}
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

enum {
	LOOK_BASE,LOOK_HAIR,LOOK_WEAPON,LOOK_HEAD_BOTTOM,LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HAIR_COLOR,LOOK_CLOTHES_COLOR,LOOK_SHIELD,LOOK_SHOES
};



extern struct map_data maps[];
extern size_t map_num;
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
// gat?ﾖｧ
int map_getcell(unsigned short m,unsigned short x, unsigned short y,cell_t cellchk);
int map_getcellp(struct map_data& m,unsigned short x, unsigned short y,cell_t cellchk);
void map_setcell(unsigned short m,unsigned short x, unsigned short y,int cellck);





///////////////////////////////////////////////////////////////////////////////

// 鯖全体情報
void map_setusers(int fd);
int map_getusers(void);







//block関連に追加
int map_count_oncell(unsigned short m,int x,int y, int type);
struct skill_unit *map_find_skill_unit_oncell(block_list &target,int x,int y,unsigned short skill_id, skill_unit *out_unit);
// 一時的object関連
int map_addobject(block_list &bl);
int map_delobject(int);
int map_delobjectnofree(int id);
//
int map_quit(map_session_data &sd);
// npc
int map_addnpc(unsigned short m, npc_data *nd);

// 床アイテム関連
int map_clearflooritem_timer(int tid, unsigned long tick, int id, basics::numptr data);
int map_removemobs_timer(int tid, unsigned long tick, int id, basics::numptr data);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y,struct map_session_data *first_sd,struct map_session_data *second_sd,struct map_session_data *third_sd,int type);
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range);


int map_mapname2mapid(const char *name);
bool map_mapname2ipport(const char *name, basics::ipset &mapset);
int map_setipport(const char *name, basics::ipset &mapset);
int map_eraseipport(const char *name, basics::ipset &mapset);
int map_eraseallipport(void);






void map_helpscreen(); // [Valaris]
int map_delmap(const char *mapname);

struct mob_list* map_addmobtolist(unsigned short m);	// [Wizputer]
void clear_moblist(unsigned short m);
void map_spawnmobs(unsigned short m);		// [Wizputer]
void map_removemobs(unsigned short m);		// [Wizputer]

extern const char *LOG_CONF_NAME;
extern const char *MAP_CONF_NAME;
extern const char *BATTLE_CONF_FILENAME;
extern const char *ATCOMMAND_CONF_FILENAME;
extern const char *CHARCOMMAND_CONF_FILENAME;
extern const char *SCRIPT_CONF_NAME;
extern const char *MSG_CONF_NAME;
extern const char *GRF_PATH_FILENAME;


void char_online_check(void);
void char_offline(struct map_session_data *sd);




///////////////////////////////////////////////////////////////////////////////
// day-night cycle
extern int daynight_flag;		// 0=day, 1=night
extern int daynight_timer_tid;	// timer for night.day
int map_daynight_timer(int tid, unsigned long tick, int id, basics::numptr data);



///////////////////////////////////////////////////////////////////////////////
// permanent name storage
const char*	map_src_namedb(uint32 charid);
bool		map_req_namedb(const map_session_data &sd, uint32 charid);
void		map_add_namedb(uint32 charid, const char *name);

#endif

