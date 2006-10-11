// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef _PC_H_
#define _PC_H_

#include "fightable.h"




struct map_session_data : public fightable, public session_data
{

	/////////////////////////////////////////////////////////////////
private:
	static dbt* nick_db;		///< sessions ordered by char name
	static dbt* accid_db;		///< sessions ordered by account_id
	static dbt* charid_db;		///< sessions ordered by char_id
public:

	/// initialize all static elements.
	static void initialize();
	/// finalize all static elements.
	static void finalize();

	/// search session data by block_list::id.
	static map_session_data* from_blid(uint32 id);

	/// search session data by name.
	static map_session_data* from_name(const char *name)
	{
		return nick2sd(name);
	}

	/// search session data by char_id.
	static map_session_data* charid2sd(uint32 id);
	/// search session data by account_id.
	//## remove for modified account authentification
	static map_session_data* accid2sd(uint32 id);
	/// search session data by name.
	static map_session_data* nick2sd(const char *nick);

	/// return number of valid sessions
	static size_t count_users(void);


public:
	// iterator access
	typedef db_iterator<const char*,map_session_data*>	iterator;
	// nick_db contains only authentified sessions
	// so can use this directly
	static dbt* nickdb()	{ return map_session_data::nick_db; }
	



	/////////////////////////////////////////////////////////////////
	struct
	{
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
		
	mmo_charstatus status;

	int packet_ver;  // 5: old, 6: 7july04, 7: 13july04, 8: 26july04, 9: 9aug04/16aug04/17aug04, 10: 6sept04, 11: 21sept04, 12: 18oct04, 13: 25oct04 (by [Yor])
	uint32 login_id1;
	uint32 login_id2;

	item_data *inventory_data[MAX_INVENTORY];
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
	chat_data*	chat;

	time_t idletime;

	struct{
		char name[24];
	} ignore[MAX_IGNORE_LIST];


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
	skill_unit_group skillunit[MAX_SKILLUNITGROUP];
	skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	skill_timerskill skilltimerskill[MAX_SKILLTIMERSKILL];
	char blockskill[MAX_SKILL];	// [celest]	
	unsigned short cloneskill_id;
	unsigned short cloneskill_lv;
	uint32 potion_hp;
	uint32 potion_sp;
	uint32 potion_per_hp;
	uint32 potion_per_sp;

	int invincible_timer;
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

	weapon_data right_weapon;
	weapon_data left_weapon;

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
	long magic_damage_return;
	long random_attack_increase_add;
	long random_attack_increase_per;

	long classchange;

	long die_counter;
	short doridori_counter;
	char potion_success_counter;
	unsigned short mission_target;

	unsigned short reg_num;
	script_reg *reg;
	unsigned short regstr_num;
	script_regstr *regstr;

	status_change sc_data[MAX_STATUSCHANGE];
	square dev;

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
	struct vending_element vend_list[MAX_VENDING];

	long catch_target_class;

	pet_data		*pd;
	homun_data	*hd;

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

	map_session_data(int fdi, int packver, uint32 account_id, uint32 char_id, uint32 login_id1, uint32 login_id2, uint32 client_tick, unsigned char sex);
	virtual ~map_session_data();

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_PC);
	}
	virtual map_session_data*		get_sd()				{ return this; }
	virtual const map_session_data* get_sd() const			{ return this; }


	///////////////////////////////////////////////////////////////////////////
	// databases/maintainance
	
	/// insert to nick_db.
	//## move this to the assignment of the status data
	void add_nickdb();

	/// clean all entries of this object in dbs
	//## could be placed only at the destructor
	void clean_db();
	
	
	///////////////////////////////////////////////////////////////////////////
	// normal object access

	
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


	///////////////////////////////////////////////////////////////////////////
	// status functions

	/// checks for dead state
	virtual bool is_dead() const		{ return (this->state.dead_sit == 1); }
	/// sets the object to dead state
	virtual bool set_dead()				{ this->state.dead_sit = 1; return true; }

	/// checks for sitting state
	virtual bool is_sitting() const		{ return (this->state.dead_sit == 2); }
	/// sets the object to sitting state
	virtual bool set_sit()				{ if(!this->state.dead_sit) this->state.dead_sit=2; return this->is_sitting(); }
	/// sets the object to standing state
	virtual bool set_stand();


	virtual bool is_hiding() const			{ return 0 != (this->status.option&0x4006); }
	virtual bool is_cloaking() const		{ return 0==(this->status.option&0x4000) && 0!=(this->status.option&0x0004); }
	virtual bool is_chasewalk() const		{ return 0 != (this->status.option&0x4000); }
	virtual bool is_carton() const			{ return 0 != (this->status.option&CART_MASK); }
	virtual bool is_falcon() const			{ return 0 != (this->status.option&0x0010); }
	virtual bool is_riding() const			{ return 0 != (this->status.option&0x0020); }
	virtual bool is_invisible() const		{ return 0 != (this->status.option&0x0040); }
	virtual bool is_50overweight() const	{ return this->weight*2 >= this->max_weight; }
	virtual bool is_90overweight() const	{ return this->weight*10 >= this->max_weight*9; }



	virtual int get_race() const			{ return 7; }
	virtual int get_size() const;



	///////////////////////////////////////////////////////////////////////////
	// attack functions

	/// do object depending stuff for attacking
	virtual void do_attack();

	/// send skill failed message.
	virtual int clif_skill_failed(ushort skill_id, uchar type=0, ushort btype=0);


	virtual int heal(int hp, int sp=0);


	///////////////////////////////////////////////////////////////////////////
	// chat functions


	/// create a chat.
	bool createchat(unsigned short limit, unsigned char pub, const char* pass, const char* title);

	/// leave a chat.
	bool leavechat();


private:
	// no copy/assign since of the bl reference
	map_session_data(const map_session_data& m);
	const map_session_data& operator=(const map_session_data& m);

	void* operator new[](size_t);	// forbidden
	void  operator delete[](void*);	// forbidden
};














bool pc_iskiller(map_session_data &src, map_session_data &target); // [MouseJstr]
int pc_getrefinebonus(int lv,int type);

int pc_setrestartvalue(map_session_data &sd,int type);
int pc_makesavestatus(map_session_data &sd);
int pc_authok(uint32 charid, uint32 login_id2, time_t connect_until_time, unsigned char *buf);
int pc_authfail(int fd);

bool pc_isequipable(map_session_data &sd, unsigned short inx);
unsigned short pc_equippoint(map_session_data &sd, unsigned short inx);

bool pc_break_equip(map_session_data &sd, unsigned short idx);
static inline bool pc_breakweapon(map_session_data &sd)	{ return pc_break_equip(sd, EQP_WEAPON); }
static inline bool pc_breakarmor(map_session_data &sd)	{ return pc_break_equip(sd, EQP_ARMOR); }
static inline bool pc_breakshield(map_session_data &sd)	{ return pc_break_equip(sd, EQP_SHIELD); }
static inline bool pc_breakhelm(map_session_data &sd)	{ return pc_break_equip(sd, EQP_HELM); }

int pc_checkskill(map_session_data &sd,unsigned short skill_id);
bool pc_checkallowskill(map_session_data &sd);
unsigned short pc_checkequip(map_session_data &sd, unsigned short pos);

int pc_calc_skilltree(map_session_data &sd);
int pc_calc_skilltree_normalize_job(map_session_data &sd);
int pc_clean_skilltree(map_session_data &sd);

int pc_checkoverhp(map_session_data &sd);
int pc_checkoversp(map_session_data &sd);

bool pc_setpos(map_session_data &sd,const char *mapname_org,unsigned short x,unsigned short y,int clrtype);
int pc_setsavepoint(map_session_data &sd,const char *mapname,unsigned short x,unsigned short y);
int pc_randomwarp(map_session_data &sd,int type);
int pc_memo(map_session_data &sd,int i);

int pc_checkadditem(map_session_data &sd,unsigned short nameid,unsigned short amount);
size_t pc_inventoryblank(map_session_data &sd);
int pc_search_inventory(map_session_data &sd,int item_id);
bool pc_payzeny(map_session_data &sd,uint32 zeny);
bool pc_getzeny(map_session_data &sd,uint32 zeny);
int pc_additem(map_session_data &sd, item &item_data,size_t amount);
int pc_delitem(map_session_data &sd, unsigned short inx, size_t amount, int type);
int pc_checkitem(map_session_data &sd);

int pc_cart_additem(map_session_data &sd, item &item_data, size_t amount);
int pc_cart_delitem(map_session_data &sd,unsigned short inx, size_t amount,int type);
int pc_putitemtocart(map_session_data &sd,unsigned short idx, size_t amount);
int pc_getitemfromcart(map_session_data &sd,unsigned short idx, size_t amount);
int pc_cartitem_amount(map_session_data &sd,unsigned short idx, size_t amount);

int pc_takeitem(map_session_data &sd,flooritem_data &fitem);
int pc_dropitem(map_session_data &sd,unsigned short inx, size_t amount);

int pc_checkweighticon(map_session_data &sd);

int pc_bonus (map_session_data &sd,int type,int val);
int pc_bonus2(map_session_data &sd,int type,int type2,int val);
int pc_bonus3(map_session_data &sd,int type,int type2,int type3,int val);
int pc_bonus4(map_session_data &sd,int type,int type2,int type3,int type4,int val);

int pc_skill(map_session_data &sd,unsigned short skillid,unsigned short skilllvl,int flag);
void pc_blockskill_start (map_session_data &sd, unsigned short skillid, unsigned long tick);	// [celest]

int pc_insert_card(map_session_data &sd, unsigned short idx_card, unsigned short idx_equip);

int pc_item_identify(map_session_data &sd, unsigned short idx);
int pc_item_repair(map_session_data &sd, unsigned short idx); // [Celest]
int pc_item_refine(map_session_data &sd, unsigned short idx); // [Celest]
int pc_steal_item(map_session_data &sd, block_list *bl);
int pc_steal_coin(map_session_data &sd, block_list *bl);

int pc_modifybuyvalue(map_session_data &sd,size_t orig_value);
int pc_modifysellvalue(map_session_data &sd,size_t orig_value);

int pc_attack(map_session_data &sd,uint32 target_id,int type);

int pc_follow(map_session_data &sd, uint32 target_id); // [MouseJstr]
int pc_stop_following(map_session_data &sd);

int pc_checkbaselevelup(map_session_data &sd);
int pc_checkjoblevelup(map_session_data &sd);
int pc_gainexp(map_session_data &sd, uint32 base_exp, uint32 job_exp);
uint32 pc_nextbaseexp(map_session_data &sd);
uint32 pc_nextbaseafter(map_session_data &sd); // [Valaris]
uint32 pc_nextjobexp(map_session_data &sd);
uint32 pc_nextjobafter(map_session_data &sd); // [Valaris]
unsigned char pc_need_status_point(map_session_data &sd,int type);
int pc_statusup(map_session_data &sd,int type);
int pc_statusup2(map_session_data &sd,int type,int val);
int pc_skillup(map_session_data &sd, unsigned short skillid);
int pc_allskillup(map_session_data &sd);
int pc_resetlvl(map_session_data &sd,int type);
int pc_resetstate(map_session_data &sd);
int pc_resetskill(map_session_data &sd);

int pc_equipitem(map_session_data &sd,unsigned short inx, unsigned short pos);
int pc_unequipitem(map_session_data &sd,unsigned short inx, int flag);
int pc_useitem(map_session_data &sd,unsigned short inx);

int pc_damage(map_session_data &sd, long damage, block_list *src);

int pc_itemheal(map_session_data &sd,long hp,long sp);
int pc_percentheal(map_session_data &sd,long hp,long sp);
int pc_jobchange(map_session_data &sd,int job, int upper);
int pc_setoption(map_session_data &sd,int type);
int pc_setcart(map_session_data &sd,int type);
int pc_setfalcon(map_session_data &sd);
int pc_setriding(map_session_data &sd);
int pc_changelook(map_session_data &sd,int type,unsigned short val);
int pc_equiplookall(map_session_data &sd);

int pc_readparam(map_session_data &sd,int type);
int pc_setparam(map_session_data &sd,int type,int val);
int pc_readreg(map_session_data &sd,int reg);
int pc_setreg(map_session_data &sd,int reg,int val);
const char *pc_readregstr(map_session_data &sd,int reg);
int pc_setregstr(map_session_data &sd,int reg,const char *str);
int pc_readglobalreg(map_session_data &sd,const char *reg);
int pc_setglobalreg(map_session_data &sd,const char *reg,int val);
int pc_readaccountreg(map_session_data &sd,const char *reg);
int pc_setaccountreg(map_session_data &sd,const char *reg,int val);
int pc_readaccountreg2(map_session_data &sd,const char *reg);
int pc_setaccountreg2(map_session_data &sd,const char *reg,int val);

int pc_addeventtimer(map_session_data &sd,unsigned long tick,const char *name);
int pc_deleventtimer(map_session_data &sd,const char *name);
int pc_cleareventtimer(map_session_data &sd);
int pc_addeventtimercount(map_session_data &sd,const char *name,unsigned long tick);

int pc_calc_pvprank(map_session_data &sd);
int pc_calc_pvprank_timer(int tid, unsigned long tick, int id, basics::numptr data);

uint32 pc_ismarried(map_session_data &sd);
bool pc_marriage(map_session_data &sd1,map_session_data &sd2);
bool pc_divorce(map_session_data &sd);
bool pc_adoption(map_session_data &sd1,map_session_data &sd2, map_session_data &sd3);
map_session_data *pc_get_partner(map_session_data &sd);
map_session_data *pc_get_father(map_session_data &sd);
map_session_data *pc_get_mother(map_session_data &sd);
map_session_data *pc_get_child(map_session_data &sd);


bool pc_break_equip(map_session_data &sd, unsigned short where);

struct pc_base_job{
	int job; //êEã∆ÅAÇΩÇæÇµì]ê∂êEÇ‚ó{éqêEÇÃèÍçáÇÕå≥ÇÃêEã∆Çï‘Ç∑(îpÉvÉäÅ®ÉvÉä)
	int type; //ÉmÉr 0, àÍéüêE 1, ìÒéüêE 2, ÉXÉpÉmÉr 3
	int upper; //í èÌ 0, ì]ê∂ 1, ó{éq 2
};

pc_base_job pc_calc_base_job(int b_class);//ì]ê∂Ç‚ó{éqêEÇÃå≥ÇÃêEã∆Çï‘Ç∑
int pc_calc_base_job2(int b_class);	// Celest
int pc_calc_upper(int b_class);

struct skill_tree_entry {
	unsigned short id;
	unsigned char max;
	unsigned char joblv;
	struct {
		unsigned short id;
		unsigned char lv;
	} need[5];
}; // Celest
extern skill_tree_entry skill_tree[MAX_PC_CLASS][MAX_SKILL_TREE];

int	skill_tree_get_max( int id, int b_class );	// Celest


int pc_setinvincibletimer(map_session_data &sd,int);
int pc_delinvincibletimer(map_session_data &sd);
int pc_addspiritball(map_session_data &sd,int,int);
int pc_delspiritball(map_session_data &sd,int,int);
int pc_eventtimer(int tid, unsigned long tick, int id, basics::numptr data);

int pc_readdb(void);
int do_init_pc(void);
void do_final_pc(void);

enum {ADDITEM_EXIST,ADDITEM_NEW,ADDITEM_OVERAMOUNT};


#endif

