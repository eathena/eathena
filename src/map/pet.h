// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PET_H_
#define _PET_H_

#include "fightable.h"


#define MAX_PET_DB	300
#define MAX_PETLOOT_SIZE	30 // [Valaris] - Changed to MAX_PETLOOT_SIZE [Skotlex]

struct petdb
{
	char	name[24];
	char	jname[24];
	short	class_;
	short itemID;
	short EggID;
	short AcceID;
	short FoodID;
	short intimate;		// counts 0...1000 as per thousand
	int fullness;
	int hungry_delay;
	int r_hungry;
	int r_full;
	int die;
	int capture;
	int speed;
	int talk_convert_class;
	int attack_rate;
	int defence_attack_rate;
	int change_target_rate;
	char s_perfor;
	script_object*script;
};
extern petdb pet_db[MAX_PET_DB];

enum { PET_CLASS,PET_CATCH,PET_EGG,PET_EQUIP,PET_FOOD };









///////////////////////////////////////////////////////////////////////////////
struct pet_data : public fightable
{
	/////////////////////////////////////////////////////////////////
	static pet_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_pd():NULL;
	}
	/////////////////////////////////////////////////////////////////


	petstatus pet;

	const petdb &petDB;


	struct _state
	{
//		unsigned state : 8 ;
		unsigned skillstate : 8 ;
		
		unsigned casting_flag : 1; //Skotlex: Used to identify when we are casting. I want a state.state value for that....

		signed skillbonus : 2;
		_state() : 
//			state(0),
			skillstate(0),
			casting_flag(0),
			skillbonus(0)
		{}
	} state;

	unsigned short dir;
	unsigned short speed;


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
		item *itemlist;
		uint32 weight;
		unsigned short count;
		unsigned short max;
		unsigned long loottick;

		pet_loot() : 
			itemlist(NULL),
			weight(0),
			count(0),
			max(0),
			loottick(0)
		{}
	} *loot; //[Valaris] / Rewritten by [Skotlex]
	
	skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL]; // [Valaris]
	skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP]; // [Valaris]
	skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET]; // [Valaris]
	map_session_data &msd;
	int hungry_timer;

	pet_data(map_session_data& sd, const petstatus &p, const petdb &pdb) :
		pet(p),
		petDB(pdb),
		dir(0),
		speed(0),
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
		msd(sd),
		hungry_timer(-1)
	{}
	virtual ~pet_data();

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_PET);
	}
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

	virtual bool set_dead()				{ return false; }// cannot die
	virtual int get_race() const;
	virtual int get_race2() const;
	virtual int get_mode() const;
	virtual int get_mexp() const;
	virtual int get_size() const;


	void menu(int menunum);
	void food();
	void droploot(bool drop=false);
	void performance();
	void return_to_egg();
	bool change_name(const char *name);


private:
	pet_data(const pet_data&);					// forbidden
	const pet_data& operator=(const pet_data&);	// forbidden

};






int pet_hungry_val(map_session_data &sd);
int pet_target_check(map_session_data &sd, block_list *bl,int type);
int pet_sc_check(map_session_data &sd, int type); //Skotlex
int pet_stopattack(pet_data &pd);
int pet_timer(int tid, unsigned long tick, int id, basics::numptr data);

int search_petDB_index(int key,int type);

int pet_recv_petdata(uint32 account_id, petstatus &p,int flag);
int pet_select_egg(map_session_data &sd,short egg_index);
int pet_catch_process1(map_session_data &sd,int target_class);
int pet_catch_process2(map_session_data &sd,uint32 target_id);
int pet_get_egg(uint32 account_id,uint32 pet_id,int flag);


int pet_equipitem(map_session_data &sd,int index);
int pet_unequipitem(map_session_data &sd);

int pet_delay_item_drop2(int tid, unsigned long tick, int id, basics::numptr data);
int pet_skill_use(pet_data &pd, block_list &target, short skill_id, short skill_lv, unsigned int tick);
int pet_skill_support_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Skotlex]
int pet_skill_bonus_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_recovery_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_heal_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skillsupport_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Skotlex]
int pet_hungry_timer(int tid, unsigned long tick, int id, basics::numptr data);

int pet_skill_bonus_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_recovery_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_heal_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skillattack_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skill_support_timer(int tid, unsigned long tick, int id, basics::numptr data);// [Skotlex]

int read_petdb();
int do_init_pet(void);
int do_final_pet(void);

#endif

