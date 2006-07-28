// $Id: map.h,v 1.8 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _MAP_H_
#define _MAP_H_

#include "mmo.h"
#include "socket.h"
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
#define OPTION_MASK 0xd7b8
#define CART_MASK 0x788
#define STATE_BLIND 0x10
#define MAX_SKILL_TREE 53
#define MOBID_EMPERIUM 1288

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




enum { MS_IDLE,MS_WALK,MS_ATTACK,MS_DEAD,MS_DELAY };

enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// àÕÇ‹ÇÍÉyÉiÉãÉeÉBåvéZóp

// ëïîıÉRÅ[Éh
enum {
	EQP_WEAPON		= 1,		// âEéË
	EQP_ARMOR		= 2,		// ëÃ
	EQP_SHIELD		= 4,		// ç∂éË
	EQP_HELM		= 8,		// ì™è„íi
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
struct skill_unit;
class homun_data;

///////////////////////////////////////////////////////////////////////////////
/// object on a map
struct block_list : public coordinate
{
	unsigned short m;
	unsigned char type;
	unsigned char subtype;
	uint32 id;
	block_list *next;
	block_list *prev;
	/// default constructor.
	block_list(uint32 i=0, unsigned char t=0, unsigned char s=0) : 
		m(0),
		type(t),
		subtype(s),
		id(i),
		next(NULL),
		prev(NULL)
	{}
	virtual ~block_list()	
	{
		// not yet removed from map
		if(this->prev)
			this->map_delblock();
		// and just to be sure that it's removed from iddb
		this->map_deliddb();
	}


	// missing:
	// all interactions with map 
	// which should be directly wrapped into a static interface
	// any comments? [Hinoko]

	// we'll leave it global right now and wrap it later
	// just move the basic functions here for the moment
	// and take care for the necessary refinements [Shinomori]

	// block identifier database
	void map_addiddb();
	void map_deliddb();

	// block on a map
	bool map_addblock();
	bool map_delblock();

	// collision free deleting of blocks
	int map_freeblock();
	static int map_freeblock_lock(void);
	static int map_freeblock_unlock(void);



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
	virtual bool is_type(int t)
	{
		return (t==this->type);
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
	virtual bool set_idle()				{ return false; }

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

	///////////////////////////////////////////////////////////////////////////
	// targeting functions

	/// unlock from current target
	virtual void unlock_target()							{ }

};





// include it here until the classes got seperated
#include "movable.h"

#include "fightable.h"



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
struct vending
{
	unsigned short index;
	unsigned short amount;
	uint32 value;

	// default constructor
	vending() : 
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


struct map_session_data : public fightable, public session_data
{
	struct {
		unsigned auth : 1;							// 0
		unsigned monster_ignore :1;					// 3
		unsigned dead_sit : 2;						// 4, 5
		unsigned skillcastcancel : 1;				// 6
		unsigned waitingdisconnect : 1;				// 7 - byte 1
		unsigned lr_flag : 2;						// 8,9 
		unsigned connect_new : 1;					// 10
		unsigned arrow_atk : 1;						// 11
		unsigned attack_type : 3;					// 12,13,14
		unsigned skill_flag : 1;					// 15 - byte 2
		unsigned gangsterparadise : 1;				// 16 
		unsigned produce_flag : 1;					// 17
		unsigned autoloot : 1; //by Upa-Kun			// 18
		unsigned storage_flag : 1;					// 19
		unsigned snovice_flag : 4;					// 20,21,22,23 - byte 3

		// originally by Qamera, adapted by celest
		unsigned event_death : 1;					// 24
		unsigned event_kill : 1;					// 25
		unsigned event_disconnect : 1;				// 26
		unsigned event_onconnect : 1;				// 27

		unsigned killer : 1;						// 28
		unsigned killable : 1;						// 29
		unsigned restart_full_recover : 1;			// 30
		unsigned no_castcancel : 1;					// 31 - byte 4
		unsigned no_castcancel2 : 1;				// 32
		unsigned no_sizefix : 1;					// 33
		unsigned no_magic_damage : 1;				// 34
		unsigned no_weapon_damage : 1;				// 35
		unsigned no_gemstone : 1;					// 36
		unsigned infinite_endure : 1;				// 37
		unsigned intravision : 1;					// 38
		unsigned ignoreAll : 1;						// 39  - byte 5
		unsigned nodelay :1;						// 40
		unsigned noexp :1;							// 41
		unsigned potion_flag : 2;					// 42,43
		unsigned viewsize : 2;						// 44,45
		unsigned abra_flag : 1;						// 46
		unsigned perfect_hiding : 1;				// 47  - byte 6
		unsigned rest : 1;							// 48
													// 7 bits left
	} state;
		
	struct mmo_charstatus status;

	int packet_ver;  // 5: old, 6: 7july04, 7: 13july04, 8: 26july04, 9: 9aug04/16aug04/17aug04, 10: 6sept04, 11: 21sept04, 12: 18oct04, 13: 25oct04 (by [Yor])
	uint32 login_id1;
	uint32 login_id2;

	struct item_data *inventory_data[MAX_INVENTORY];
	unsigned short itemindex;

	unsigned short equip_index[MAX_EQUIP];
	unsigned short unbreakable_equip;
	unsigned short unbreakable;	// chance to prevent equipment breaking [celest]

	uint32 weight;
	uint32 max_weight;
	uint32 cart_weight;
	uint32 cart_max_weight;
	unsigned short cart_num;
	unsigned short cart_max_num;
	char mapname[24];
	int fd;
	unsigned short speed;
	short opt1;
	short opt2;
	short opt3;
	unsigned char dir;
	unsigned char head_dir;
	uint32 client_tick;

	uint32 areanpc_id;
	uint32 npc_shopid;

	CScriptEngine ScriptEngine;
	
	uint32 chatID;
	time_t idletime;

	struct{
		char name[24];
	} ignore[MAX_IGNORE_LIST];

	unsigned short attacktarget_lv;


	uint32 followtarget;
	int followtimer; // [MouseJstr]


	time_t emotionlasttime; // to limit flood with emotion packets

	uint32 skilltarget;
	unsigned short skillx;
	unsigned short skilly;
	unsigned short skillid;
	unsigned short skilllv;
	unsigned short skillitem;
	unsigned short skillitemlv;
	unsigned short skillid_old;
	unsigned short skilllv_old;
	unsigned short skillid_dance;
	unsigned short skilllv_dance;
	struct skill_unit_group skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	struct skill_timerskill skilltimerskill[MAX_SKILLTIMERSKILL];
	char blockskill[MAX_SKILL];	// [celest]	
	unsigned short cloneskill_id;
	unsigned short cloneskill_lv;
	uint32 potion_hp;
	uint32 potion_sp;
	uint32 potion_per_hp;
	uint32 potion_per_sp;

	int invincible_timer;
	unsigned long canact_tick;
	unsigned long canlog_tick;
	unsigned long canregen_tick;
	uint32 hp_sub;
	uint32 sp_sub;
	unsigned long inchealhptick;
	unsigned long inchealsptick;
	unsigned long inchealspirithptick;
	unsigned long inchealspiritsptick;

	unsigned short view_class;
	unsigned short disguise_id; // [Valaris]

	unsigned short weapontype1;
	unsigned short weapontype2;
	unsigned short attackrange;
	unsigned short attackrange_;

	struct weapon_data right_weapon;
	struct weapon_data left_weapon;

	long paramb[6];
	long paramc[6];
	long parame[6];
	long paramcard[6];
	unsigned short hit;
	unsigned short flee;
	unsigned short flee2;
	unsigned short aspd;
	uint32 amotion;
	uint32 dmotion;

	unsigned short  def;
	unsigned short  def2;
	unsigned short  mdef;
	unsigned short  mdef2;
	long			critical;
	unsigned short  matk1;
	unsigned short  matk2;

	long def_ele;
	long castrate;
	long delayrate;
	long hprate;
	long sprate;
	long dsprate;

	long subele[10];
	long subrace[12];
	long addeff[10];
	long addeff2[10];
	long reseff[10];
	unsigned short base_atk;
	long atk_rate;
	long weapon_atk[16];
	long weapon_atk_rate[16];
	long arrow_atk;
	long arrow_ele;
	long arrow_cri;
	long arrow_hit;
	unsigned short arrow_range;
	long arrow_addele[10];
	long arrow_addrace[12];
	long arrow_addsize[3];
	long arrow_addeff[10];
	long arrow_addeff2[10];
	long nhealhp;
	long nhealsp;
	long nshealhp;
	long nshealsp;
	long nsshealhp;
	long nsshealsp;
	long aspd_rate;
	long speed_rate;
	long hprecov_rate;
	long sprecov_rate;
	long critical_def;
	long double_rate;
	long near_attack_def_rate;
	long long_attack_def_rate;
	long magic_def_rate;
	long misc_def_rate;
	long matk_rate;
	long ignore_mdef_ele;
	long ignore_mdef_race;
	long magic_addele[10];
	long magic_addrace[12];
	long magic_subrace[12];
	long perfect_hit;
	long get_zeny_num;
	long critical_rate;
	long hit_rate;
	long flee_rate;
	long flee2_rate;
	long def_rate;
	long def2_rate;
	long mdef_rate;
	long mdef2_rate;
	long add_magic_damage_class_count;
	short add_magic_damage_classid[10];
	long add_magic_damage_classrate[10];
	short add_def_class_count;
	short add_mdef_class_count;
	short add_def_classid[10];
	short add_mdef_classid[10];
	long add_def_classrate[10];
	short add_mdef_classrate[10];
	short monster_drop_item_count;
	short monster_drop_itemid[10];
	long monster_drop_race[10];
	long monster_drop_itemrate[10];
	long double_add_rate;
	long speed_add_rate;
	long aspd_add_rate;
	long perfect_hit_add;
	long get_zeny_add_num;
	short splash_range;
	short splash_add_range;
	short autospell_id[10];
	short autospell_lv[10];
	short autospell_rate[10];
	long short_weapon_damage_return;
	long long_weapon_damage_return;
	long weapon_coma_ele[10];
	long weapon_coma_race[12];
	short break_weapon_rate;
	short break_armor_rate;
	short add_steal_rate;
	//--- 02/15's new card effects [celest]
	long crit_atk_rate;
	long critaddrace[12];
	short no_regen;
	long addeff3[10];
	short addeff3_type[10];
	short autospell2_id[10];
	short autospell2_lv[10];
	short autospell2_rate[10];
	long skillatk[2];
	unsigned short unstripable_equip;
	short add_damage_classid2[10];
	short add_damage_class_count2;
	long add_damage_classrate2[10];
	short sp_gain_value;
	short hp_gain_value;
	short sp_drain_type;

	uint32 hp_loss_tick;
	uint32 hp_loss_rate;
	short hp_loss_value;
	short hp_loss_type;

	uint32 sp_loss_tick;
	uint32 sp_loss_rate;
	short sp_loss_value;

	long addrace2[12],addrace2_[12];
	long subsize[3];
	short unequip_losehp[11];
	short unequip_losesp[11];
	unsigned short itemid;
	long itemhealrate[7];
	//--- 03/15's new card effects
	long expaddrace[12];
	long subrace2[12];
	short sp_gain_race[12];
	short monster_drop_itemgroup[10];

	int setitem_hash;
	//--- end effects
	short spiritball;
	short spiritball_old;
	int spirit_timer[MAX_SKILL_LEVEL];
	long magic_damage_return; // AppleGirl Was Here
	long random_attack_increase_add;
	long random_attack_increase_per; // [Valaris]

	long classchange; // [Valaris]

	long die_counter;
	short doridori_counter;
	char potion_success_counter;
	unsigned short mission_target;

	unsigned short reg_num;
	struct script_reg *reg;
	unsigned short regstr_num;
	struct script_regstr *regstr;

	struct status_change sc_data[MAX_STATUSCHANGE];
	struct square dev;

	uint32 trade_partner;
	unsigned short deal_item_index[MAX_TRADING];
	unsigned short deal_item_amount[MAX_TRADING];
	uint32 deal_zeny;
	unsigned short deal_locked;

	uint32 party_sended;
	uint32 party_invite;
	uint32 party_invite_account;
	sint32 party_hp;
	unsigned short party_x;
	unsigned short party_y;

	uint32 guild_sended;
	uint32 guild_invite;
	uint32 guild_invite_account;
	uint32 guild_emblem_id;
	uint32 guild_alliance;
	uint32 guild_alliance_account;
	uint32 guildspy; // [Syrus22]
	uint32 partyspy; // [Syrus22]

	uint32 vender_id;
	unsigned short vend_num;
	char message[80];
	struct vending vending[MAX_VENDING];

	long catch_target_class;
	struct petstatus pet;
	struct pet_data *pd;

	uint32 pvp_won;
	uint32 pvp_lost;
	uint32 pvp_point;
	uint32 pvp_rank;
	uint32 pvp_lastusers;
	int pvp_timer;

	int eventtimer[MAX_EVENTTIMER];
	unsigned short eventcount; // [celest]

	unsigned short change_level;	// [celest]
	uint32 canuseitem_tick;
	char fakename[24];
	unsigned long mail_tick;	// mail counter for mail system [Valaris]




	// operator new that ensures that memory is cleared before construction
	// I'm just too lazy to initialize all those stupid menbers that get thrown out anyway
	void* operator new(size_t sz)
	{
		void* ret = malloc(sz);
		memset(ret,0,sz);
		return ret;
	}
	void  operator delete(void* p)
	{
		if(p) free(p);
	}

	map_session_data(int fdi=0, int packver=0, uint32 account_id=0, uint32 char_id=0, uint32 login_id1=0, uint32 client_tick=0, unsigned char sex=0)
	{
		this->init(fdi, packver, account_id, char_id, login_id1, client_tick, sex);
	}
	virtual ~map_session_data()
	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual map_session_data*		get_sd()				{ return this; }
	virtual const map_session_data* get_sd() const			{ return this; }


	/// returns GM level
	unsigned char isGM() const;

	/// timer callback
	virtual int attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data);
	virtual int skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data);


	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking()	{}
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy);
	/// do object depending stuff at the end of the walk step.
	virtual void do_walkend();
	/// do object depending stuff for the walkto
	virtual void do_walkto();
	/// do object depending stuff for changestate
	virtual void do_changestate(int state,int type);


	///////////////////////////////////////////////////////////////////////////
	// status functions

	/// checks for dead state
	virtual bool is_dead() const		{ return (this->state.dead_sit == 1); }



	/// do object depending stuff for attacking
	virtual void do_attack();

private:
	// no copy/assign since of the bl reference
	map_session_data(const map_session_data& m);
	const map_session_data& operator=(const map_session_data& m);

protected:
	void init(int fdi=0, int packver=0, uint32 account_id=0, uint32 char_id=0, uint32 login_id1=0, uint32 client_tick=0, unsigned char sex=0)
	{
		size_t i;
		unsigned long tick = gettick();

		this->fd				= fdi;
		this->packet_ver		= packver;

		this->block_list::id	= account_id;
		this->block_list::type	= BL_PC;

		this->status.char_id	= char_id;
		this->status.sex		= sex;

		this->login_id1			= login_id1;
		this->login_id2			= 0;
		this->state.auth		= 0;
		this->speed = DEFAULT_WALK_SPEED;
		this->client_tick		= client_tick;
		
		this->skillitem = 0xFFFF;
		this->skillitemlv = 0xFFFF;
		
		this->party_x = 0xFFFF;
		this->party_y = 0xFFFF;
		this->party_hp = -1;

		// ticks
		this->canact_tick		= tick;
		this->canmove_tick		= tick;
		this->canlog_tick		= tick;
		this->canregen_tick = tick;
		this->attackable_tick = tick;

		// timers
		this->followtimer = -1;
		this->invincible_timer = -1;
		this->pvp_timer = -1;
		
		for(i = 0; i < MAX_EVENTTIMER; ++i)
			this->eventtimer[i] = -1;
		for(i = 0; i < MAX_SKILL_LEVEL; ++i)
			this->spirit_timer[i] = -1;
	}

private:
	void* operator new[](size_t);	// forbidden
	void  operator delete[](void*);	// forbidden

};


///////////////////////////////////////////////////////////////////////////////
struct npc_timerevent_list
{
	int timer;
	size_t pos;

	npc_timerevent_list() : 
		timer(-1),
		pos(0)
	{}
};
struct npc_item_list
{
	unsigned short nameid;
	long value;
// part of a union and cannot be default constructed
// need to wait until new hierarchy is fully implemented
//	npc_item_list() : 
//		nameid(0),
//		value(0)
//	{}
};

struct npc_data : public movable
{
	short n;
	short class_;
	short dir;
	short speed;
	char name[24];
	char exname[24];
	int chat_id;
	short opt1;
	short opt2;
	short opt3;
	short option;
	short flag;

	struct { // [Valaris]
		unsigned state : 8;
	} state;


	short arenaflag;
	void *chatdb;

	union {
		struct {
			struct script_object *ref; // pointer with reference counter
			short xs;
			short ys;
			int guild_id;
			int timer;
			int timerid;
			int timeramount;
			int nexttimer;
			uint32 rid;
			unsigned long timertick;
			struct npc_timerevent_list *timer_event;
		} scr;
		struct npc_item_list shop_item[1];
		struct {
			short xs;
			short ys;
			short x;
			short y;
			char name[16];
		} warp;
	} u;
	// Ç±Ç±Ç…ÉÅÉìÉoÇí«â¡ÇµÇƒÇÕÇ»ÇÁÇ»Ç¢(shop_itemÇ™â¬ïœí∑ÇÃà◊)



	// can have an empty constructor here since it is cleared at allocation
	npc_data()
	{}
	virtual ~npc_data()
	{}


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual npc_data*				get_nd()				{ return this; }
	virtual const npc_data*			get_nd() const			{ return this; }


	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking();
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy);
	/// do object depending stuff for the walkto
	virtual void do_walkto() {}
	/// do object depending stuff for changestate
	virtual void do_changestate(int state,int type);

	/// checks for walking state
	virtual bool is_walking() const		{ return this->movable::is_walking()||(state.state==MS_WALK); }


private:
	npc_data(const npc_data&);					// forbidden
	const npc_data& operator=(const npc_data&);	// forbidden

public:
	// provide special allocation to enable variable space for shop items
	//
	// change to an own shop class later that can be called from an npc
	// to enable the old shop npc and also callable script controlled (dynamic) shops

	void* operator new(size_t sz)
	{
		void *ret = malloc(sz);
		memset(ret,0,sz);
		return ret;
	}
	void operator delete(void *p)
	{
		if(p) free(p);
	}
	void* operator new(size_t sz, size_t shopitems)
	{
		void* ret = malloc(sz + shopitems*sizeof(struct npc_item_list));
		memset(ret,0,sz + shopitems*sizeof(struct npc_item_list));
		return ret;
	}
	void operator delete(void *p, size_t shopitems)
	{
		if(p) free(p);
	}

private:
	void* operator new[](size_t sz);			// forbidden
	void operator delete[](void *p, size_t sz);	// forbidden
};




/*==========================================
 * The structure object for item drop with delay
 * Since it is only two being able to pass [ int ] a timer function
 * Data is put in and passed to this structure object.
 *------------------------------------------
 */
struct delay_item_drop
{
	unsigned short m;
	unsigned short x;
	unsigned short y;
	unsigned short nameid;
	unsigned short amount;
	struct map_session_data *first_sd;
	struct map_session_data *second_sd;
	struct map_session_data *third_sd;

	delay_item_drop(
		unsigned short mi,
		unsigned short xi,
		unsigned short yi,
		unsigned short idi,
		struct map_session_data *sd1=NULL,
		struct map_session_data *sd2=NULL,
		struct map_session_data *sd3=NULL
		) :
		m(mi),
		x(xi),
		y(yi),
		nameid(idi),
		amount(1),
		first_sd(sd1),
		second_sd(sd2),
		third_sd(sd3)
	{}
};

struct delay_item_drop2
{
	unsigned short m;
	unsigned short x;
	unsigned short y;
	struct item item_data;
	struct map_session_data *first_sd;
	struct map_session_data *second_sd;
	struct map_session_data *third_sd;

	delay_item_drop2(
		unsigned short mi,
		unsigned short xi,
		unsigned short yi,
		const struct item& itemi,
		struct map_session_data *sd1=NULL,
		struct map_session_data *sd2=NULL,
		struct map_session_data *sd3=NULL
		) :
		m(mi),
		x(xi),
		y(yi),
		item_data(itemi),
		first_sd(sd1),
		second_sd(sd2),
		third_sd(sd3)
	{}
};



///////////////////////////////////////////////////////////////////////////////
// Mob List Held in memory for Dynamic Mobs [Wizputer]
struct mob_list
{
    unsigned short m;

	unsigned short x0;			/////////
	unsigned short y0;			// will replace the data in the mobs
	unsigned short xs;
	unsigned short ys;
	unsigned long delay1;
	unsigned long delay2;		////////

	unsigned short class_;
	unsigned short level;

    char mobname[24];
	char eventname[24];

	unsigned short num;

	mob_list() : 
		m(0),
		x0(0),
		y0(0),
		xs(0),
		ys(0),
		delay1(0),
		delay2(0),
		class_(0),
		level(0),
		num(0)
	{
		mobname[0]=0;
		eventname[0]=0;
	}

};

struct mob_data : public fightable
{
	unsigned short base_class;
	unsigned short class_;
	unsigned short mode;
	unsigned short speed;
	unsigned char dir;
	char name[24];

	// mobs are divided into cached and noncached (script/spawned)
	mob_list* cache;

	struct _state {
		unsigned state : 8;						//b1
		unsigned skillstate : 8;				//b2
		unsigned targettype : 1;
		unsigned steal_flag : 1;
		unsigned steal_coin_flag : 1;
		unsigned skillcastcancel : 1;
		unsigned master_check : 1;
		unsigned soul_change_flag : 1;			//b3
		unsigned special_mob_ai : 2;			//	takes values 0,1,2,3
		unsigned is_master : 1;					//	set if mob is a master with spawns
		unsigned alchemist: 1;
		unsigned size : 2;
		unsigned recall_flag :1;
		unsigned _unused : 1;

		_state() :
			state(0),
			skillstate(0),
			targettype(0),
			steal_flag(0),
			steal_coin_flag(0),
			skillcastcancel(0),
			master_check(0),
			soul_change_flag(0),
			special_mob_ai(0),
			is_master(0),
			alchemist(0),
			size(0),
			recall_flag(0)
		{}
	} state;

	sint32 hp;
	sint32 max_hp;

	unsigned short level;
	unsigned short attacked_count;
	unsigned short target_dir;
	unsigned short target_lv;

	uint32 provoke_id; // Celest
	uint32 attacked_id;

	unsigned long next_walktime;
	unsigned long last_deadtime;
	unsigned long last_spawntime;
	unsigned long last_thinktime;
	
	struct mob_damage
	{
		uint32 fromid;
		long dmg;
		mob_damage() : 
			fromid(0),
			dmg(0)
		{}
	} dmglog[DAMAGELOG_SIZE];
	struct item *lootitem;
	short move_fail_count;
	short lootitem_count;

	struct status_change sc_data[MAX_STATUSCHANGE];
	short opt1;
	short opt2;
	short opt3;
	short option;
	short min_chase;
	int deletetimer;

	uint32 guild_id; // for guardians

	uint32 skilltarget;
	unsigned short skillx;
	unsigned short skilly;
	unsigned short skillid;
	unsigned short skilllv;
	unsigned short skillidx;
	unsigned long skilldelay[MAX_MOBSKILL];
	int def_ele;
	uint32 master_id;
	int master_dist;
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL];
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	char npc_event[50];

	unsigned short recallmob_count;
	unsigned short recallcount;


	/// constructor.
	/// prepares the minimum data set for MOB spawning
	mob_data(const char *mobname, int class_);
	virtual ~mob_data()	{}


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual mob_data*				get_md()				{ return this; }
	virtual const mob_data*			get_md() const			{ return this; }



	/// walk to a random target
	virtual bool randomwalk(unsigned long tick);
	/// timer callback
	virtual int attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data);
	virtual int skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data);


	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking();
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy);
	/// do object depending stuff for the walkto
	virtual void do_walkto() {}
	/// do object depending stuff for changestate
	virtual void do_changestate(int state,int type);

	/// checks for walking state
	virtual bool is_walking() const		{ return this->movable::is_walking()||(state.state==MS_WALK); }

	/// special target unlocking with standby time
	void unlock_target(unsigned long tick);


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	virtual bool stop_attack();


	///////////////////////////////////////////////////////////////////////////
	// Appearance functions.
	// might fit in status module
	virtual int get_viewclass() const;
	virtual int get_sex() const;
	virtual ushort get_hair() const;
	virtual ushort get_hair_color() const;
	virtual ushort get_weapon() const;
	virtual ushort get_shield() const;
	virtual ushort get_head_top() const;
	virtual ushort get_head_mid() const;
	virtual ushort get_head_buttom() const;
	virtual ushort get_clothes_color() const;
	virtual int get_equip() const;




	///////////////////////////////////////////////////////////////////////////
	// status functions

	/// checks for dead state
	virtual bool is_dead() const		{ return (this->hp<=0); }



private:
	mob_data(const mob_data&);					// forbidden
	const mob_data& operator=(const mob_data&);	// forbidden
};


///////////////////////////////////////////////////////////////////////////////
struct pet_data : public fightable
{
	const char *namep;

	const struct petdb &petDB;


	struct _state
	{
		unsigned state : 8 ;
		unsigned skillstate : 8 ;
		
		unsigned casting_flag : 1; //Skotlex: Used to identify when we are casting. I want a state.state value for that....

		signed skillbonus : 2;
		_state() : 
			state(0),
			skillstate(0),
			casting_flag(0),
			skillbonus(0)
		{}
	} state;

	unsigned short class_;
	unsigned short dir;
	unsigned short speed;
	unsigned short equip_id;

	unsigned short target_lv;
	short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]
	uint32 move_fail_count;
	unsigned long next_walktime;
	unsigned long last_thinktime;

	
	struct pet_status { //Pet Status data
		unsigned short level;
		unsigned short atk1;
		unsigned short atk2;
		unsigned short str;
		unsigned short agi;
		unsigned short vit;
		unsigned short int_;
		unsigned short dex;
		unsigned short luk;
		pet_status() : 
			level(0),
			atk1(0),
			atk2(0),
			str(0),
			agi(0),
			vit(0),
			int_(0),
			dex(0),
			luk(0)
		{}
	} *status;  //[Skotlex]

	struct pet_recovery { //Stat recovery
		unsigned short type;	//Status Change id
		unsigned short delay; //How long before curing (secs).
		int timer;
		pet_recovery() :
			type(0),
			delay(0),
			timer(-1)
		{}
	} *recovery; //[Valaris] / Reimplemented by [Skotlex]
	
	struct pet_bonus {
		unsigned short type; //bStr, bVit?
		unsigned short val;	//Qty
		unsigned short duration; //in secs
		unsigned short delay;	//Time before recasting (secs)
		int timer;
		pet_bonus() :
			type(0),
			val(0),
			duration(0),
			delay(0),
			timer(-1)
		{}
	} *bonus; //[Valaris] / Reimplemented by [Skotlex]
	
	struct pet_skill_attack { //Attack Skill
		unsigned short id;
		unsigned short lv;
		unsigned short div_; //0 = Normal skill. >0 = Fixed damage (lv), fixed div_.
		unsigned short rate; //Base chance of skill ocurrance (10 = 10% of attacks)
		unsigned short bonusrate; //How being 100% loyal affects cast rate (10 = At 1000 intimacy->rate+10%

		pet_skill_attack() : 
			id(0),
			lv(0),
			div_(0),
			rate(0),
			bonusrate(0)
		{}
	} *a_skill;	//[Skotlex]
	
	struct pet_skill_support { //Support Skill
		unsigned short id;
		unsigned short lv;
		unsigned short hp; //Max HP% for skill to trigger (50 -> 50% for Magnificat)
		unsigned short sp; //Max SP% for skill to trigger (100 = no check)
		unsigned short delay; //Time (secs) between being able to recast.
		int timer;

		pet_skill_support() : 
			id(0),
			lv(0),
			hp(0),
			sp(0),
			delay(0),
			timer(-1)
		{}
	} *s_skill;	//[Skotlex]

	struct pet_loot {
		struct item *item;
		uint32 weight;
		unsigned short count;
		unsigned short max;
		unsigned long loottick;

		pet_loot() : 
			item(NULL),
			weight(0),
			count(0),
			max(0),
			loottick(0)
		{}
	} *loot; //[Valaris] / Rewritten by [Skotlex]
	
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL]; // [Valaris]
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP]; // [Valaris]
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET]; // [Valaris]
	struct map_session_data *msd;
	int hungry_timer;

	pet_data(const char *n, const struct petdb &pdb) :
		namep(n),
		petDB(pdb),
		class_(0),
		dir(0),
		speed(0),
		equip_id(0),
		target_lv(0),
		rate_fix(0),
		move_fail_count(0),
		next_walktime(0),
		last_thinktime(0),
		status(NULL),
		recovery(NULL),
		bonus(NULL),
		a_skill(NULL),
		s_skill(NULL),
		loot(NULL),
		msd(NULL),
		hungry_timer(-1)
	{}
	virtual ~pet_data()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual pet_data*				get_pd()				{ return this; }
	virtual const pet_data*			get_pd() const			{ return this; }


	/// walk to a random target
	virtual bool randomwalk(unsigned long tick);
	/// timer callback
	virtual int attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data);
	virtual int skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data);


	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking();
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy);
	/// do object depending stuff for the walkto
	virtual void do_walkto() {}
	/// do object depending stuff for changestate
	virtual void do_changestate(int state,int type);

	/// checks for walking state
	virtual bool is_walking() const		{ return this->movable::is_walking()||(state.state==MS_WALK); }


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	virtual bool stop_attack();

	///////////////////////////////////////////////////////////////////////////
	// Appearance functions.
	// might fit in status module
	virtual int get_viewclass() const;
	virtual int get_sex() const;
	virtual ushort get_hair() const;
	virtual ushort get_hair_color() const;
	virtual ushort get_weapon() const;
	virtual ushort get_shield() const;
	virtual ushort get_head_top() const;
	virtual ushort get_head_mid() const;
	virtual ushort get_head_buttom() const;
	virtual ushort get_clothes_color() const;
	virtual int get_equip() const;

private:
	pet_data(const pet_data&);					// forbidden
	const pet_data& operator=(const pet_data&);	// forbidden

};








///////////////////////////////////////////////////////////////////////////////
class flooritem_data : public block_list
{
public:
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
	virtual flooritem_data*			get_fd()				{ return this; }
	virtual const flooritem_data*	get_fd() const			{ return this; }


private:
	flooritem_data(const flooritem_data&);					// forbidden
	const flooritem_data& operator=(const flooritem_data&);	// forbidden
};












///////////////////////////////////////////////////////////////////////////////

// map_getcell()/map_setcell()Ç≈égópÇ≥ÇÍÇÈÉtÉâÉO
typedef enum { 
	CELL_CHKWALL=0,		// ï«(ÉZÉãÉ^ÉCÉv1)
	CELL_CHKWATER,		// êÖèÍ(ÉZÉãÉ^ÉCÉv3)
	CELL_CHKGROUND,		// ínñ è·äQï®(ÉZÉãÉ^ÉCÉv5)
	CELL_CHKPASS,		// í âﬂâ¬î\(ÉZÉãÉ^ÉCÉv1,5à»äO)
	CELL_CHKNOPASS,		// í âﬂïsâ¬(ÉZÉãÉ^ÉCÉv1,5)
	CELL_CHKHOLE,		// a hole in morroc desert
	CELL_GETTYPE,		// ÉZÉãÉ^ÉCÉvÇï‘Ç∑
	CELL_CHKNOPASS_NPC,

	CELL_CHKNPC=0x10,	// É^ÉbÉ`É^ÉCÉvÇÃNPC(ÉZÉãÉ^ÉCÉv0x80ÉtÉâÉO)
	CELL_SETNPC,		// É^ÉbÉ`É^ÉCÉvÇÃNPCÇÉZÉbÉg
	CELL_CLRNPC,		// É^ÉbÉ`É^ÉCÉvÇÃNPCÇclear suru
	CELL_CHKBASILICA,	// ÉoÉWÉäÉJ(ÉZÉãÉ^ÉCÉv0x40ÉtÉâÉO)
	CELL_SETBASILICA,	// ÉoÉWÉäÉJÇÉZÉbÉg
	CELL_CLRBASILICA,	// ÉoÉWÉäÉJÇÉNÉäÉA
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
};
// will alloc a short now


struct map_data
{
	///////////////////////////////////////////////////////////////////////////
	char mapname[24];
	struct mapgat	*gat;	// NULLÇ»ÇÁâ∫ÇÃmap_data_other_serverÇ∆ÇµÇƒàµÇ§
	///////////////////////////////////////////////////////////////////////////

	struct _objects
	{
		struct block_list*	root_blk;
		struct block_list*	root_mob;
		uint				cnt_blk;
		uint				cnt_mob;
		_objects() :
			root_blk(NULL),
			root_mob(NULL),
			cnt_blk(0),
			cnt_mob(0)
		{}
	} *objects;
//	struct block_list **block;
//	struct block_list **block_mob;
//	int *block_count;
//	int *block_mob_count;
	int m;
	unsigned short xs;
	unsigned short ys;
	unsigned short bxs;
	unsigned short bys;
	int wh;
	size_t npc_num;
	size_t users;
	struct
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
		unsigned gvg_dungeon : 1;				// 14 // celest
		unsigned nozenypenalty : 1;				// 15 (byte 2)
		unsigned notrade : 1;					// 16
		unsigned noskill : 1;					// 17
		unsigned nowarp : 1;					// 18
		unsigned nowarpto : 1;					// 19
		unsigned nopvp : 1;						// 20
		unsigned noicewall : 1;					// 21
		unsigned snow : 1;						// 22
		unsigned rain : 1;						// 23
		unsigned sakura : 1;					// 24
		unsigned leaves : 1;					// 25
		unsigned clouds : 1;					// 26 (byte 3)
		unsigned clouds2 : 1;					// 27
		unsigned fog : 1;						// 28
		unsigned fireworks : 1;					// 29
		unsigned indoors : 1;					// 30
		unsigned nogo : 1;						// 31
		unsigned nobaseexp	: 1;				// 32 (byte 4) // [Lorky] added by Lupus
		unsigned nojobexp	: 1;				// 33 // [Lorky]
		unsigned nomobloot	: 1;				// 34 // [Lorky]				
		unsigned nomvploot	: 1;				// 35 // [Lorky]		
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
	struct mapgat *gat;	// NULLå≈íËÇ…ÇµÇƒîªíf
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
extern int night_flag; // 0=day, 1=night [Yor]

extern int map_read_flag; // 0: grf´’´°´§´Î 1: ´≠´„´√´∑´Â 2: ´≠´„´√´∑´Â(?ıÍ)
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
// gat?÷ß
int map_getcell(unsigned short m,unsigned short x, unsigned short y,cell_t cellchk);
int map_getcellp(struct map_data& m,unsigned short x, unsigned short y,cell_t cellchk);
void map_setcell(unsigned short m,unsigned short x, unsigned short y,int cellck);





///////////////////////////////////////////////////////////////////////////////

// éIëSëÃèÓïÒ
void map_setusers(int fd);
int map_getusers(void);



///////////////////////////////////////////////////////////////////////////////
//
// new foreach... implementation without needing varargs
//!! integrate to mapdata and spare giving the map argument
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// virtual base class which is called for each block
///////////////////////////////////////////////////////////////////////////////
class CMapProcessor : public basics::noncopyable
{
public:
	CMapProcessor()			{}
	virtual ~CMapProcessor()	{}
	virtual int process(struct block_list& bl) const = 0;
};

class CMap
{
public:
	CMap()	{}
	~CMap()	{}
static int foreachinarea(const CMapProcessor& elem, unsigned short m, int x0,int y0,int x1,int y1,int type);
static int foreachincell(const CMapProcessor& elem, unsigned short m,int x,int y,int type);
static int foreachinmovearea(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,int type);
static int foreachinpath(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int range,int type);
static int foreachpartymemberonmap(const CMapProcessor& elem, struct map_session_data &sd, int type);
static int foreachobject(const CMapProcessor& elem,int type);
};




///////////////////////////////////////////////////////////////////////////////

//int map_foreachinarea(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int type,...);
// -- moonsoul (added map_foreachincell)
//int map_foreachincell(int (*func)(struct block_list&,va_list &),unsigned short m,int x,int y,int type,...);
//int map_foreachinmovearea(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...);
//int map_foreachinpath(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int range,int type,...); // Celest
//void party_foreachsamemap(int (*func)(struct block_list&,va_list &),struct map_session_data &sd,int type,...);
//void map_foreachobject(int (*)(struct block_list*,va_list &),int,...);

int map_countnearpc(unsigned short m, int x, int y);
//blockä÷òAÇ…í«â¡
int map_count_oncell(unsigned short m,int x,int y, int type);
struct skill_unit *map_find_skill_unit_oncell(struct block_list &target,int x,int y,unsigned short skill_id,struct skill_unit *out_unit);
// àÍéûìIobjectä÷òA
int map_addobject(struct block_list &bl);
int map_delobject(int);
int map_delobjectnofree(int id);
//
int map_quit(struct map_session_data &sd);
// npc
int map_addnpc(unsigned short m, struct npc_data *nd);

// è∞ÉAÉCÉeÉÄä÷òA
int map_clearflooritem_timer(int tid, unsigned long tick, int id, basics::numptr data);
int map_removemobs_timer(int tid, unsigned long tick, int id, basics::numptr data);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y,struct map_session_data *first_sd,struct map_session_data *second_sd,struct map_session_data *third_sd,int type);
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range);

// ÉLÉÉÉâidÅÅÅÑÉLÉÉÉâñº ïœä∑ä÷òA
void map_addchariddb(uint32 charid,const char *name);
void map_delchariddb(uint32 charid);
int map_reqchariddb(struct map_session_data &sd,uint32 charid);
char * map_charid2nick(uint32 id);
struct map_session_data * map_charid2sd(uint32 id);
struct map_session_data * map_id2sd(uint32 id);
struct block_list * map_id2bl(uint32 id);

int map_mapname2mapid(const char *name);
bool map_mapname2ipport(const char *name, basics::ipset &mapset);
int map_setipport(const char *name, basics::ipset &mapset);
int map_eraseipport(const char *name, basics::ipset &mapset);
int map_eraseallipport(void);
dbt* get_iddb();
void map_addnickdb(struct map_session_data &sd);
struct map_session_data * map_nick2sd(const char *nick);





void map_helpscreen(); // [Valaris]
int map_delmap(const char *mapname);

struct mob_list* map_addmobtolist(unsigned short m);	// [Wizputer]
void clear_moblist(unsigned short m);
void map_spawnmobs(unsigned short m);		// [Wizputer]
void map_removemobs(unsigned short m);		// [Wizputer]

extern char *INTER_CONF_NAME;
extern char *LOG_CONF_NAME;
extern char *MAP_CONF_NAME;
extern char *BATTLE_CONF_FILENAME;
extern char *ATCOMMAND_CONF_FILENAME;
extern char *CHARCOMMAND_CONF_FILENAME;
extern char *SCRIPT_CONF_NAME;
extern char *MSG_CONF_NAME;
extern char *GRF_PATH_FILENAME;


void char_online_check(void); // [Valaris]
void char_offline(struct map_session_data *sd);





#endif

