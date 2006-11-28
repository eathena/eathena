// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOMUN_H_
#define _HOMUN_H_

#include "map.h"
#include "fightable.h"

#define NATURAL_HEAL_HP_INTERVAL 2*1000
#define NATURAL_HEAL_SP_INTERVAL 4*1000

struct homun_db
{
	unsigned short class_;
	char name[24];
	char jname[24];
	unsigned short base_level;
	uint32 hp;
	uint32 sp;
	uint32 hp_kmax;
	uint32 hp_kmin;
	uint32 sp_kmax;
	uint32 sp_kmin;
	unsigned short str;
	unsigned short agi;
	unsigned short vit;
	unsigned short int_;
	unsigned short dex;
	unsigned short luk;
	int base;
	unsigned short str_k;
	unsigned short agi_k;
	unsigned short vit_k;
	unsigned short int_k;
	unsigned short dex_k;
	unsigned short luk_k;
	unsigned short AcceID;
	unsigned short FoodID;
	unsigned short aspd_k;
	unsigned short maxskill;
	unsigned short view_class;
	unsigned short size;
	unsigned short race;
	unsigned short element;
	unsigned short evo_class;
	unsigned short exp_table;
	unsigned short skillpoint;
	script_object* script;
};
extern struct homun_db homun_db[MAX_HOMUN_DB];

struct random_homun_data
{
	unsigned short homunid;
	unsigned short per;
};












class homun_data : public fightable
{
public:
	/////////////////////////////////////////////////////////////////
	static homun_data* from_blid(uint32 id)
	{
		block_list* bl = block_list::from_blid(id);
		return bl?bl->get_hd():NULL;
	}

	/////////////////////////////////////////////////////////////////

public:
	struct homunstatus status;
	struct _state
	{
		unsigned skillstate : 8 ;

		_state() :
			skillstate(0)
		{}
	} state;
	short view_size;
	int invincible_timer;
	uint32 hp_sub;
	uint32 sp_sub;
	uint32 max_hp;
	uint32 max_sp;
	ushort str;
	ushort agi;
	ushort vit;
	ushort int_;
	ushort dex;
	ushort luk;
	ushort atk;
	ushort matk;
	ushort def;
	ushort mdef;
	ushort hit;
	ushort critical;
	ushort flee;
	ushort aspd;
	ushort equip;
	uint32 intimate;
	int homskillstatictimer[MAX_HOMSKILL];
	struct status_change sc_data[MAX_STATUSCHANGE];
	short atackable;
	short limits_to_growth;
	ushort view_class;
	int nhealhp;
	int nhealsp;
	int hprecov_rate;
	int sprecov_rate;
	int natural_heal_hp_timer;
	int natural_heal_sp_timer;
	int hungry_timer;
	int hungry_cry_timer;

	map_session_data &msd;


	/// constructor. needs map_session_data of associated charater for creation
	homun_data(map_session_data &sd);
	/// destructor. does automatic cleanup
	virtual ~homun_data();


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_HOM);
	}
	virtual homun_data*				get_hd()				{ return this; }
	virtual const homun_data*		get_hd() const			{ return this; }


	///////////////////////////////////////////////////////////////////////////
	// status functions

	/// checks for dead state
	virtual bool is_dead() const		{ return (this->status.hp<=0); }

	virtual bool set_dead();

	
	///////////////////////////////////////////////////////////////////////////
	// walking functions

	// no changes to default behaviour


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	/// object depending check if attack is possible
	virtual bool can_attack(const fightable& target)	{ return true; }
	/// do object depending stuff for attacking
	virtual void do_attack()							{}
	/// get the current attack range
	virtual ushort get_attackrange()					{ return 1; }

	///////////////////////////////////////////////////////////////////////////
	// skill functions

	virtual int skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data)
	{
		// no skills for now
		return 0;
	}

	


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
private:
	void* operator new[](size_t sz);			// forbidden
	void operator delete[](void *p, size_t sz);	// forbidden

private:
	homun_data(const homun_data&);					// forbidden
	const homun_data& operator=(const homun_data&);	// forbidden



public:
	


	int checkskill(unsigned short skill_id);
	bool check_baselevelup();

	void gain_exp(uint32 base_exp, uint32 job_exp, block_list &obj);
	int next_baseexp() const;
	
	int damage(block_list &src, uint32 damage);
	virtual int heal(int hp, int sp=0);
	void delete_natural_heal_timer();
	void delete_hungry_timer();
	
	void calc_skilltree();
	void calc_status();
	void recalc_status();
	void save_data();

	void return_to_master();
	bool return_to_embryo();
	void menu(unsigned short menunum);
	void food();
	void delete_data();
	bool change_name(const char *name);
	bool revive(unsigned short skilllv);
	bool skillup(unsigned short skill_num);

//	static homun_data *get_homunculus(uint32 char_id);
//	static homun_data *get_homunculus(const map_session_data &sd);
	static homun_data *create_homunculus(map_session_data &sd, unsigned short homunid);
	static void clear_homunculus(map_session_data &sd);
	static bool call_homunculus(map_session_data &sd);
	static void recv_homunculus(homunstatus &p, int flag);


	virtual int get_class() const		{ return this->status.class_; }
	virtual int get_lv() const			{ return this->status.base_level; }
	virtual int get_hp() const			{ return this->status.hp; }

	virtual uint32 get_party_id() const	{ return this->msd.status.party_id; }
	virtual uint32 get_guild_id() const	{ return this->msd.status.guild_id; }


};



int do_init_homun(void);
int do_final_homun(void);

int read_homundb(void);
void homun_reload(void);

#endif
