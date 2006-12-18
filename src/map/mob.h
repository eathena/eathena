// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_H_
#define _MOB_H_

#include "fightable.h"


#define MAX_RANDOMMONSTER 3
#define MAX_MOB_RACE_DB 6
#define MAX_MOB_DB 2000		/* Change this to increase the table size in your mob_db to accomodate
				numbers more than 2000 for mobs if you want to (and know what you're doing).
				Be sure to note that 4001 to 4047 are for advanced classes. */

struct mob_skill {
	short state;
	short skill_id,skill_lv;
	short permillage;
	int casttime;
	int delay;
	short cancel;
	short cond1,cond2;
	short target;
	int val[5];
	short emotion;
};

struct mob_db
{
	char name[24];
	char jname[24];
	short lv;
	int max_hp;
	int max_sp;
	int base_exp;
	int job_exp;
	int atk1;
	int atk2;
	int def;
	int mdef;
	int str;
	int agi;
	int vit;
	int int_;
	int dex;
	int luk;
	int range;
	int range2;
	int range3;
	int size;
	int race;
	int element;
	int mode;
	short race2;	// celest
	int speed;
	int adelay;
	int amotion;
	int dmotion;
	int mexp;
	int mexpper;
	struct 
	{
		unsigned short nameid;
		unsigned short p;
	} dropitem[10]; //8 -> 10 Lupus
	struct 
	{
		unsigned short nameid;
		unsigned short p;
	} mvpitem[3];
	int view_class;
	int sex;
	unsigned short hair;
	unsigned short hair_color;
	unsigned short weapon;
	unsigned short shield;
	unsigned short head_top;
	unsigned short head_mid;
	unsigned short head_buttom;
	unsigned short option;
	unsigned short clothes_color;
	int equip;
	int summonper[MAX_RANDOMMONSTER];
	int maxskill;
	mob_skill skill[MAX_MOBSKILL];
};
extern mob_db mob_db[];

enum {
	MST_TARGET			=	0,
	MST_SELF			=	1,
	MST_FRIEND			=	2,
	MST_AROUND5			=	3,
	MST_AROUND6			=	4,
	MST_AROUND7			=	5,
	MST_AROUND8			=	6,
	MST_AROUND1			=	7,
	MST_AROUND2			=	8,
	MST_AROUND3			=	9,
	MST_AROUND4			=	10,
	MST_AROUND			=	MST_AROUND4,

	MSC_ALWAYS			=	0x0000,
	MSC_MYHPLTMAXRATE	=	0x0001,
	MSC_FRIENDHPLTMAXRATE=	0x0010,
	MSC_MYSTATUSON		=	0x0020,
	MSC_MYSTATUSOFF		=	0x0021,
	MSC_FRIENDSTATUSON	=	0x0030,
	MSC_FRIENDSTATUSOFF	=	0x0031,

	MSC_ATTACKPCGT		=	0x0100,
	MSC_ATTACKPCGE		=	0x0101,
	MSC_SLAVELT			=	0x0110,
	MSC_SLAVELE			=	0x0111,
	MSC_CLOSEDATTACKED	=	0x1000,
	MSC_LONGRANGEATTACKED=	0x1001,
	MSC_SKILLUSED		=	0x1010,
	MSC_CASTTARGETED	=	0x1011,
	MSC_RUDEATTACKED	=	0x1100,
	MSC_MASTERHPLTMAXRATE=	0x1101,
	MSC_MASTERATTACKED	=	0x1110,
	MSC_ALCHEMIST		=	0x1111
};

enum {
	MSS_IDLE,
	MSS_WALK,
	MSS_ATTACK,
	MSS_DEAD,
	MSS_LOOT,
	MSS_CHASE,
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
	map_session_data *first_sd;
	map_session_data *second_sd;
	map_session_data *third_sd;

	delay_item_drop(
		const block_list& bl,
		unsigned short idi,
		map_session_data *sd1=NULL,
		map_session_data *sd2=NULL,
		map_session_data *sd3=NULL
		) :
		m(bl.m),
		x(bl.x),
		y(bl.y),
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
	item item_data;
	map_session_data *first_sd;
	map_session_data *second_sd;
	map_session_data *third_sd;

	delay_item_drop2(
		const block_list& bl,
		const item& itemi,
		map_session_data *sd1=NULL,
		map_session_data *sd2=NULL,
		map_session_data *sd3=NULL
		) :
		m(bl.m),
		x(bl.x),
		y(bl.y),
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
	/////////////////////////////////////////////////////////////////
	static mob_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_md():NULL;
	}
	/////////////////////////////////////////////////////////////////



	unsigned short base_class;
	unsigned short class_;
	unsigned short mode;
	unsigned short speed;
	unsigned char dir;
	char name[24];

	// mobs are divided into cached and noncached (script/spawned)
	mob_list* cache;

	struct _state {
//		unsigned state : 8;						//b1
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
//			state(0),
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
	item *lootitem;
	short move_fail_count;
	short lootitem_count;

	status_change sc_data[MAX_STATUSCHANGE];
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
	skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL];
	skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP];
	skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	char npc_event[50];

	unsigned short recallmob_count;
	unsigned short recallcount;


	/// constructor.
	/// prepares the minimum data set for MOB spawning
	mob_data(const char *mobname, int cl);
	virtual ~mob_data();


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_MOB);
	}
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


	/// special target unlocking with standby time
	void unlock_target(unsigned long tick);


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	virtual bool stop_attack();

	virtual int heal(int hp, int sp=0);


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
	virtual bool set_dead();

	virtual int get_class() const		{ return this->class_; }
	virtual int get_lv() const			{ return this->level; }
	virtual int get_range() const		{ return mob_db[this->class_].range; }
	virtual int get_hp() const			{ return this->hp; }

	virtual int get_race() const		{ return mob_db[this->class_].race; }
	virtual int get_race2() const		{ return mob_db[this->class_].race2; }
	virtual int get_mode() const		{ return mob_db[this->class_].mode; }
	virtual int get_mexp() const		{ return mob_db[this->class_].mexp; }
	virtual int get_size() const		{ return mob_db[this->class_].size; }

	virtual uint32 get_party_id() const;
	virtual uint32 get_guild_id() const;


	void remove_map(int type);
	void set_spawndelay();
	void remove_slaves() const;
	uint count_slaves() const;
	uint summon_slaves(unsigned short skillid, unsigned short skilllv);

//	virtual bool warp(unsigned short m, unsigned short x, unsigned short y, int type);


	///////////////////////////////////////////////////////////////////////////
	// skill functions

	/// returns skill_lvl for given skillid of the current object
	virtual int  skill_check(ushort id)			
	{
		//##TODO add access to mobdb
		return 0; 
	}
	virtual bool skill_can_cancel() const
	{
		return this->state.skillcastcancel;
	}

private:
	mob_data(const mob_data&);					// forbidden
	const mob_data& operator=(const mob_data&);	// forbidden
};








int mobdb_searchname(const char *str);
int mobdb_checkid(const uint32 id);
int mob_once_spawn(map_session_data *sd,const char *mapname,
	int x,int y,const char *mobname,int class_,int amount,const char *event);
int mob_once_spawn_area(map_session_data *sd,const char *mapname,
	int x0,int y0,int x1,int y1,
	const char *mobname,int class_,int amount,const char *event);

int mob_spawn_guardian(map_session_data *sd,const char *mapname,	// Spawning Guardians [Valaris]
	int x,int y,const char *mobname,int class_,int amount,const char *event,int guardian);	// Spawning Guardians [Valaris]


int mob_target(mob_data &md, block_list *bl,int dist);
int mob_spawn(uint32 id);
int mob_damage(mob_data &md,int damage,int type, block_list *src);
int mob_warp(mob_data &md,int m,int x,int y,int type);

int do_init_mob(void);

int mob_timer_delete(int tid, unsigned long tick, int id, basics::numptr data);

int mob_class_change(mob_data &md,int value[], size_t count);


int mobskill_use(mob_data &md,unsigned long tick,int event);
int mobskill_event(mob_data &md,int flag);
int mobskill_castend_id(int tid, unsigned long tick, int id, basics::numptr data);
int mobskill_castend_pos(int tid, unsigned long tick, int id, basics::numptr data);


bool mob_gvmobcheck(map_session_data &sd, block_list &bl);
void mob_reload(void);

#endif
