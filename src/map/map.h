// $Id: map.h,v 1.8 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _MAP_H_
#define _MAP_H_

#include "mmo.h"
#include "socket.h"
#include "script.h"

///////////////////////////////////////////////////////////////////////////////
#define MAX_PC_CLASS 4050
#define PC_CLASS_BASE 0
#define PC_CLASS_BASE2 (PC_CLASS_BASE + 4001)
#define PC_CLASS_BASE3 (PC_CLASS_BASE2 + 22)
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE ((int)battle_config.area_size)
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
#define MAX_WALKPATH 32
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

///////////////////////////////////////////////////////////////////////////////
enum { BL_NUL, BL_PC, BL_NPC, BL_MOB, BL_ITEM, BL_CHAT, BL_SKILL, BL_PET };	// 0..7 -> 3bit
enum { WARP, SHOP, SCRIPT, MONS };											// 0..3 -> 2bit




///////////////////////////////////////////////////////////////////////////////
// predeclarations
struct skill_unit_group;
struct npc_data;
struct pet_db;
struct item_data;
struct square;




///////////////////////////////////////////////////////////////////////////////
struct block_list
{
	struct block_list *next;
	struct block_list *prev;
	uint32 id;
	unsigned short m;
	unsigned short x;
	unsigned short y;
	unsigned char type;
	unsigned char subtype;
};

///////////////////////////////////////////////////////////////////////////////
struct walkpath_data
{
	unsigned char path_len;
	unsigned char path_pos;
	unsigned char path_half;
	unsigned char path[MAX_WALKPATH];
};
struct shootpath_data
{
	unsigned short rx;
	unsigned short ry;
	unsigned short len;
	unsigned short x[MAX_WALKPATH];
	unsigned short y[MAX_WALKPATH];
};

///////////////////////////////////////////////////////////////////////////////
struct script_reg
{
	int index;
	int data;
};
struct script_regstr
{
	int index;
	char data[256];
};


///////////////////////////////////////////////////////////////////////////////
struct status_change
{
	int timer;
	intptr val1;
	intptr val2;
	intptr val3;
	intptr val4;
};

///////////////////////////////////////////////////////////////////////////////
struct vending
{
	unsigned short index;
	unsigned short amount;
	uint32 value;
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
};

///////////////////////////////////////////////////////////////////////////////
struct skill_unit
{
	struct block_list bl;
	struct skill_unit_group *group;

	long limit;
	long val1;
	long val2;
	short alive;
	short range;
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
	char *valstr;
	unsigned char unit_id;
	long group_id;
	long unit_count;
	long alive_count;
	struct skill_unit *unit;
};
struct skill_unit_group_tickset
{
	unsigned short skill_id;
	unsigned long tick;
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
};


///////////////////////////////////////////////////////////////////////////////
struct map_session_data
{
	struct block_list bl;
	struct {
		unsigned auth : 1;							// 0
		unsigned change_walk_target : 1;			// 1
		unsigned attack_continue : 1;				// 2
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
	unsigned short to_x;
	unsigned short to_y;
	unsigned short speed;
	short opt1;
	short opt2;
	short opt3;
	unsigned char dir;
	unsigned char head_dir;
	uint32 client_tick;
	struct walkpath_data walkpath;
	int walktimer;
	uint32 next_walktime;

	uint32 areanpc_id;
	uint32 npc_shopid;

	CScriptEngine ScriptEngine;
	
	uint32 chatID;
	time_t idletime;

	struct{
		char name[24];
	} ignore[MAX_IGNORE_LIST];

	unsigned short attacktarget_lv;
	unsigned long attackabletime;

	uint32 attacktarget;	
	uint32 followtarget;
	int attacktimer;
	int followtimer; // [MouseJstr]
	int skilltimer;

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
	unsigned long canmove_tick;
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
	long spirit_timer[MAX_SKILL_LEVEL];
	long magic_damage_return; // AppleGirl Was Here
	long random_attack_increase_add;
	long random_attack_increase_per; // [Valaris]

	long classchange; // [Valaris]

	long die_counter;
	short doridori_counter;
	char potion_success_counter;

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
	short party_x;
	short party_y;

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
	struct s_pet pet;
	struct petdb *petDB;
	struct pet_data *pd;
	int pet_hungry_timer;

	uint32 pvp_won;
	uint32 pvp_lost;
	uint32 pvp_point;
	uint32 pvp_rank;
	uint32 pvp_lastusers;
	int pvp_timer;

	int eventtimer[MAX_EVENTTIMER];
	unsigned short eventcount; // [celest]

//	char eventqueue[MAX_EVENTQUEUE][50];

	unsigned short change_level;	// [celest]
	uint32 canuseitem_tick;
	char fakename[24];
	uint32 mail_counter;	// mail counter for mail system [Valaris]
};


///////////////////////////////////////////////////////////////////////////////
struct npc_timerevent_list {
	int timer;
	size_t pos;
};
struct npc_item_list {
	unsigned short nameid;
	long value;
};
struct npc_label_list {
	char labelname[24];
	size_t pos;
};
struct npc_reference{
	char *script;
	struct npc_label_list *label_list;
	size_t label_list_num;	
	size_t refcnt;			//reference counter
};
struct npc_data {
	struct block_list bl;
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
	int walktimer; // [Valaris]
	short to_x;
	short to_y; // [Valaris]
	struct walkpath_data walkpath;
	unsigned long next_walktime;
	unsigned long canmove_tick;

	struct { // [Valaris]
		unsigned npcstate : 8;
		unsigned change_walk_target : 1;
		unsigned walk_easy : 1;
	} state;

	union {
		struct {
			struct npc_reference *ref; // char pointer with reference counter
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
	// ‚±‚±‚Éƒƒ“ƒo‚ð’Ç‰Á‚µ‚Ä‚Í‚È‚ç‚È‚¢(shop_item‚ª‰Â•Ï’·‚Ìˆ×)

	short arenaflag;
	void *chatdb;

	npc_data() : chatdb(NULL)	{} 
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
};

struct mob_data
{
	struct block_list bl;
	unsigned short base_class;
	unsigned short class_;
	unsigned short mode;
	unsigned short speed;
	unsigned char dir;
	char name[24];

	// mobs are divided into cached and noncached (script/spawned)
	mob_list* cache;

	struct {
		unsigned state : 8;						//b1
		unsigned skillstate : 8;				//b2
		unsigned targettype : 1;
		unsigned steal_flag : 1;
		unsigned steal_coin_flag : 1;
		unsigned skillcastcancel : 1;
		unsigned master_check : 1;
		unsigned change_walk_target : 1;
		unsigned walk_easy : 1;
		unsigned soul_change_flag : 1;			//b3
		unsigned special_mob_ai : 2;			//	takes values 0,1,2,3
		unsigned is_master : 1;					//	set if mob is a master with spawns
		unsigned alchemist: 1;
		unsigned size : 2;
		unsigned recall_flag :1;
		unsigned _unused : 1;
	} state;

	int timer;
	long hp;
	long max_hp;

	unsigned short to_x;
	unsigned short to_y;

	unsigned short level;
	unsigned short attacked_count;
	unsigned short target_dir;
	unsigned short target_lv;

	uint32 provoke_id; // Celest
	uint32 target_id;
	uint32 attacked_id;
	struct walkpath_data walkpath;

	unsigned long next_walktime;
	unsigned long attackabletime;
	unsigned long last_deadtime;
	unsigned long last_spawntime;
	unsigned long last_thinktime;
	unsigned long canmove_tick;
	
	struct mob_damage
	{
		uint32 fromid;
		long dmg;
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

	int skilltimer;
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
	uint32 exclusion_src;
	uint32 exclusion_party;
	uint32 exclusion_guild;
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL];
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	char npc_event[50];

	unsigned short recallmob_count;
	unsigned short recallcount;
};

///////////////////////////////////////////////////////////////////////////////
struct pet_data
{
	struct block_list bl;

	//char name[24];
	char *namep;
	struct {
		unsigned state : 8 ;
		unsigned skillstate : 8 ;
		unsigned change_walk_target : 1 ;
		
		unsigned casting_flag : 1; //Skotlex: Used to identify when we are casting. I want a state.state value for that....

		signed skillbonus : 2;
	} state;

	int timer;

	unsigned short class_;
	unsigned short dir;
	unsigned short speed;
	unsigned short equip_id;

	uint32 target_id;
	unsigned short target_lv;
	unsigned short to_x;
	unsigned short to_y;
	short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]
	struct walkpath_data walkpath;
	uint32 move_fail_count;
	unsigned long attackabletime;
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
	} *status;  //[Skotlex]

	struct pet_recovery { //Stat recovery
		unsigned short type;	//Status Change id
		unsigned short delay; //How long before curing (secs).
		int timer;
	} *recovery; //[Valaris] / Reimplemented by [Skotlex]
	
	struct pet_bonus {
		unsigned short type; //bStr, bVit?
		unsigned short val;	//Qty
		unsigned short duration; //in secs
		unsigned short delay;	//Time before recasting (secs)
		int timer;
	} *bonus; //[Valaris] / Reimplemented by [Skotlex]
	
	struct pet_skill_attack { //Attack Skill
		unsigned short id;
		unsigned short lv;
		unsigned short div_; //0 = Normal skill. >0 = Fixed damage (lv), fixed div_.
		unsigned short rate; //Base chance of skill ocurrance (10 = 10% of attacks)
		unsigned short bonusrate; //How being 100% loyal affects cast rate (10 = At 1000 intimacy->rate+10%
	} *a_skill;	//[Skotlex]
	
	struct pet_skill_support { //Support Skill
		unsigned short id;
		unsigned short lv;
		unsigned short hp; //Max HP% for skill to trigger (50 -> 50% for Magnificat)
		unsigned short sp; //Max SP% for skill to trigger (100 = no check)
		unsigned short delay; //Time (secs) between being able to recast.
		int timer;
	} *s_skill;	//[Skotlex]

	struct pet_loot {
		struct item *item;
		uint32 weight;
		unsigned short count;
		unsigned short max;
		unsigned long loottick;
	} *loot; //[Valaris] / Rewritten by [Skotlex]
	
	struct skill_timerskill skilltimerskill[MAX_MOBSKILLTIMERSKILL]; // [Valaris]
	struct skill_unit_group skillunit[MAX_MOBSKILLUNITGROUP]; // [Valaris]
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET]; // [Valaris]
	struct map_session_data *msd;
};

enum { MS_IDLE,MS_WALK,MS_ATTACK,MS_DEAD,MS_DELAY };

enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// ˆÍ‚Ü‚êƒyƒiƒ‹ƒeƒBŒvŽZ—p

// ‘•”õƒR[ƒh
enum {
	EQP_WEAPON		= 1,		// ‰EŽè
	EQP_ARMOR		= 2,		// ‘Ì
	EQP_SHIELD		= 4,		// ¶Žè
	EQP_HELM		= 8,		// “ªã’i
};












///////////////////////////////////////////////////////////////////////////////

// map_getcell()/map_setcell()‚ÅŽg—p‚³‚ê‚éƒtƒ‰ƒO
typedef enum { 
	CELL_CHKWALL=0,		// •Ç(ƒZƒ‹ƒ^ƒCƒv1)
	CELL_CHKWATER,		// …ê(ƒZƒ‹ƒ^ƒCƒv3)
	CELL_CHKGROUND,		// ’n–ÊáŠQ•¨(ƒZƒ‹ƒ^ƒCƒv5)
	CELL_CHKPASS,		// ’Ê‰ß‰Â”\(ƒZƒ‹ƒ^ƒCƒv1,5ˆÈŠO)
	CELL_CHKNOPASS,		// ’Ê‰ß•s‰Â(ƒZƒ‹ƒ^ƒCƒv1,5)
	CELL_CHKHOLE,		// a hole in morroc desert
	CELL_GETTYPE,		// ƒZƒ‹ƒ^ƒCƒv‚ð•Ô‚·
	CELL_CHKNOPASS_NPC,

	CELL_CHKNPC=0x10,	// ƒ^ƒbƒ`ƒ^ƒCƒv‚ÌNPC(ƒZƒ‹ƒ^ƒCƒv0x80ƒtƒ‰ƒO)
	CELL_SETNPC,		// ƒ^ƒbƒ`ƒ^ƒCƒv‚ÌNPC‚ðƒZƒbƒg
	CELL_CLRNPC,		// ƒ^ƒbƒ`ƒ^ƒCƒv‚ÌNPC‚ðclear suru
	CELL_CHKBASILICA,	// ƒoƒWƒŠƒJ(ƒZƒ‹ƒ^ƒCƒv0x40ƒtƒ‰ƒO)
	CELL_SETBASILICA,	// ƒoƒWƒŠƒJ‚ðƒZƒbƒg
	CELL_CLRBASILICA,	// ƒoƒWƒŠƒJ‚ðƒNƒŠƒA
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
	struct mapgat	*gat;	// NULL‚È‚ç‰º‚Ìmap_data_other_server‚Æ‚µ‚Äˆµ‚¤
	///////////////////////////////////////////////////////////////////////////
	char *alias; // [MouseJstr]
	struct block_list **block;
	struct block_list **block_mob;
	int *block_count;
	int *block_mob_count;
	int m;
	unsigned short xs;
	unsigned short ys;
	unsigned short bxs;
	unsigned short bys;
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
		unsigned rain : 1;						// 28
		unsigned sakura : 1;					// 26
		unsigned leaves : 1;					// 27
		unsigned clouds : 1;					// 23 (byte 3)
		unsigned fog : 1;						// 24
		unsigned fireworks : 1;					// 25
		unsigned indoors : 1;					// 29
		unsigned nogo : 1;						// 30
		unsigned nobaseexp	: 1;				// 31 (byte 4) // [Lorky] added by Lupus
		unsigned nojobexp	: 1;				// 32 // [Lorky]
		unsigned nomobloot	: 1;				// 33 // [Lorky]				
		unsigned nomvploot	: 1;				// 34 // [Lorky]		
		unsigned _unused : 5;					// 35-39 (byte 5)
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
	struct mapgat *gat;	// NULLŒÅ’è‚É‚µ‚Ä”»’f
	///////////////////////////////////////////////////////////////////////////
	ipset mapset;
	struct map_data* map;
};


///////////////////////////////////////////////////////////////////////////////
struct flooritem_data
{
	struct block_list bl;
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



///////////////////////////////////////////////////////////////////////////////
struct chat_data
{
	struct block_list bl;	// block of the chatwindow

	char pass[9];			// password (max 8)
	char title[61];			// room title (max 60)
	unsigned short limit;	// join limit
	unsigned short users;	// current users
	unsigned char trigger;
	unsigned char pub;		// room attribute
	struct map_session_data *usersd[20];
	struct block_list *owner;
	char npc_event[50];
};

extern struct map_data map[];
extern size_t map_num;
extern int autosave_interval;
extern int agit_flag;
extern int night_flag; // 0=day, 1=night [Yor]

extern int map_read_flag; // 0: grf«Õ«¡«¤«ë 1: «­«ã«Ã«·«å 2: «­«ã«Ã«·«å(?õê)
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
// gat?Ö§
int map_getcell(unsigned short m,unsigned short x, unsigned short y,cell_t cellchk);
int map_getcellp(struct map_data& m,unsigned short x, unsigned short y,cell_t cellchk);
void map_setcell(unsigned short m,unsigned short x, unsigned short y,int cellck);





///////////////////////////////////////////////////////////////////////////////

// ŽI‘S‘Ìî•ñ
void map_setusers(int fd);
int map_getusers(void);
// blockíœŠÖ˜A
int map_freeblock(void *bl);
int map_freeblock_lock(void);
int map_freeblock_unlock(void);
// blockŠÖ˜A
int map_addblock(struct block_list &bl);
int map_delblock(struct block_list &bl);



///////////////////////////////////////////////////////////////////////////////
//
// new foreach... implementation without needing varargs
//!! integrate to mapdata and spare giving the map argument
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// virtual base class which is called for each block
///////////////////////////////////////////////////////////////////////////////
class CMapProcessor : public noncopyable
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
//blockŠÖ˜A‚É’Ç‰Á
int map_count_oncell(unsigned short m,int x,int y, int type);
struct skill_unit *map_find_skill_unit_oncell(struct block_list &target,int x,int y,unsigned short skill_id,struct skill_unit *out_unit);
// ˆêŽž“IobjectŠÖ˜A
int map_addobject(struct block_list &bl);
int map_delobject(int);
int map_delobjectnofree(int id);
//
int map_quit(struct map_session_data &sd);
// npc
int map_addnpc(unsigned short m, struct npc_data *nd);

// °ƒAƒCƒeƒ€ŠÖ˜A
int map_clearflooritem_timer(int tid, unsigned long tick, int id, intptr data);
int map_removemobs_timer(int tid, unsigned long tick, int id, intptr data);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y,struct map_session_data *first_sd,struct map_session_data *second_sd,struct map_session_data *third_sd,int type);
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range);

// ƒLƒƒƒ‰id„ƒLƒƒƒ‰–¼ •ÏŠ·ŠÖ˜A
void map_addchariddb(uint32 charid,const char *name);
void map_delchariddb(uint32 charid);
int map_reqchariddb(struct map_session_data &sd,uint32 charid);
char * map_charid2nick(uint32 id);
struct map_session_data * map_charid2sd(uint32 id);
struct map_session_data * map_id2sd(uint32 id);
struct block_list * map_id2bl(uint32 id);

int map_mapname2mapid(const char *name);
bool map_mapname2ipport(const char *name, ipset &mapset);
int map_setipport(const char *name, ipset &mapset);
int map_eraseipport(const char *name, ipset &mapset);
int map_eraseallipport(void);
void map_addiddb(struct block_list &bl);
void map_deliddb(struct block_list &bl);
dbt* get_iddb();
void map_addnickdb(struct map_session_data &sd);
struct map_session_data * map_nick2sd(const char *nick);
int compare_item(struct item *a, struct item *b);

// ‚»‚Ì‘¼
int map_check_dir(int s_dir,int t_dir);
int map_calc_dir( struct block_list &src,int x,int y);

// path.c‚æ‚è
int path_search(struct walkpath_data &wpd,unsigned short m,int x0,int y0,int x1,int y1,int flag);
bool path_search_long(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1);
int path_blownpos(unsigned short m,int x0,int y0,int dx,int dy,int count);

int map_who(int fd);

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


/*==========================================
 * “ñ“_ŠÔ‚Ì‹——£‚ð•Ô‚·
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
static inline int distance(int x0,int y0,int x1,int y1)
{
	int dx,dy;

	dx=abs(x0-x1);
	dy=abs(y0-y1);
	return dx>dy ? dx : dy;
}


#ifndef TXT_ONLY

// MySQL
#include <mysql.h>

///////////////////////////////////////////////////////////////////////////////
//
// mysql access function
//
///////////////////////////////////////////////////////////////////////////////
static inline int mysql_SendQuery(MYSQL *mysql, const char* q)
{
#ifdef TWILIGHT
	ShowSQL("%s:%d# %s\n", __FILE__, __LINE__, q);
#endif
	return mysql_real_query(mysql, q, strlen(q));
}



extern MYSQL mmysql_handle;
extern char tmp_sql[65535];
extern MYSQL_RES* sql_res ;
extern MYSQL_ROW	sql_row ;

extern MYSQL lmysql_handle;
extern char tmp_lsql[65535];
extern MYSQL_RES* lsql_res ;
extern MYSQL_ROW	lsql_row ;

extern MYSQL logmysql_handle;
extern MYSQL_RES* logsql_res ;
extern MYSQL_ROW logsql_row ;

extern MYSQL mail_handle;
extern MYSQL_RES* 	mail_res ;
extern MYSQL_ROW	mail_row ;
extern char tmp_msql[65535];

extern int db_use_sqldbs;

extern char item_db_db[32];
extern char item_db2_db[32];
extern char mob_db_db[32];
extern char mob_db2_db[32];
extern char login_db[32];

extern char login_db_level[32];
extern char login_db_account_id[32];

extern char gm_db[32];
extern char gm_db_level[32];
extern char gm_db_account_id[32];

extern int lowest_gm_level;
extern int read_gm_interval;

extern char char_db[32];
#endif /* not TXT_ONLY */

#endif
