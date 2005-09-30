// $Id: mob.h,v 1.4 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _MOB_H_
#define _MOB_H_

#define MAX_RANDOMMONSTER 3
#define MAX_MOB_RACE_DB 6
#define MAX_MOB_DB 10000
	/* Change this to increase the table size in your mob_db to accomodate
		a larger mob database. Be sure to note that IDs 4001 to 4048 are reserved for advanced/baby/expanded classes.
	*/

struct mob_skill {
	short state;
	short skill_id,skill_lv;
	short permillage;
	int casttime,delay;
	short cancel;
	short cond1,cond2;
	short target;
	int val[5];
	short emotion;
};

struct mob_db {
	char name[NAME_LENGTH],jname[NAME_LENGTH];
	short lv;
	int max_hp,max_sp;
	int base_exp,job_exp;
	int atk1,atk2;
	int def,mdef;
	int str,agi,vit,int_,dex,luk;
	int range,range2,range3;
	int size,race,element,mode;
	short race2;	// celest
	int speed,adelay,amotion,dmotion;
	int mexp,mexpper;
	struct { int nameid,p; } dropitem[10]; //8 -> 10 Lupus
	struct { int nameid,p; } mvpitem[3];
	int view_class,sex;
	short hair,hair_color,weapon,shield,head_top,head_mid,head_buttom,option,clothes_color; // [Valaris]
	int equip; // [Valaris]
	int summonper[MAX_RANDOMMONSTER];
	int maxskill;
	struct mob_skill skill[MAX_MOBSKILL];
};

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

struct mob_db* mob_db(int class_);
int mobdb_searchname(const char *str);
int mobdb_checkid(const int id);
int mob_once_spawn(struct map_session_data *sd,char *mapname,
	int x,int y,const char *mobname,int class_,int amount,const char *event);
int mob_once_spawn_area(struct map_session_data *sd,char *mapname,
	int x0,int y0,int x1,int y1,
	const char *mobname,int class_,int amount,const char *event);

int mob_spawn_guardian(struct map_session_data *sd,char *mapname,	// Spawning Guardians [Valaris]
	int x,int y,const char *mobname,int class_,int amount,const char *event,int guardian);	// Spawning Guardians [Valaris]
int mob_guardian_guildchange(struct block_list *bl,va_list ap); //Change Guardian's ownership. [Skotlex]

int mob_walktoxy(struct mob_data *md,int x,int y,int easy);
int mob_randomwalk(struct mob_data *md,int tick);
int mob_can_move(struct mob_data *md);

int mob_target(struct mob_data *md,struct block_list *bl,int dist);
int mob_unlocktarget(struct mob_data *md,int tick);
int mob_stop_walking(struct mob_data *md,int type);
int mob_stopattack(struct mob_data *);
int mob_spawn(int);
int mob_setdelayspawn(int);
int mob_damage(struct block_list *,struct mob_data*,int,int,int);
int mob_changestate(struct mob_data *md,int state,int type);
int mob_heal(struct mob_data*,int);

//Defines to speed up search.
#define mob_get_viewclass(class_) mob_db(class_)->view_class
#define mob_get_sex(class_) mob_db(class_)->sex
#define mob_get_hair(class_) mob_db(class_)->hair
#define mob_get_hair_color(class_) mob_db(class_)->hair_color
#define mob_get_weapon(class_) mob_db(class_)->weapon
#define mob_get_shield(class_) mob_db(class_)->shield
#define mob_get_head_top(class_) mob_db(class_)->head_top
#define mob_get_head_mid(class_) mob_db(class_)->head_mid
#define mob_get_head_buttom(class_) mob_db(class_)->head_buttom
#define mob_get_clothes_color(class_) mob_db(class_)->clothes_color
#define mob_get_equip(class_) mob_db(class_)->equip

int do_init_mob(void);
int do_final_mob(void);

void mob_unload(struct mob_data *md);
int mob_remove_map(struct mob_data *md, int type);
int mob_delete(struct mob_data *md);
int mob_timer_delete(int tid, unsigned int tick, int id, int data);

int mob_deleteslave(struct mob_data *md);

int mob_random_class (int *value, size_t count);
int mob_class_change(struct mob_data *md,int class_);
int mob_warp(struct mob_data *md,int m,int x,int y,int type);
int mob_warpslave(struct block_list *bl, int range);

int mobskill_use(struct mob_data *md,unsigned int tick,int event);
int mobskill_event(struct mob_data *md,int flag);
int mobskill_castend_id( int tid, unsigned int tick, int id,int data );
int mobskill_castend_pos( int tid, unsigned int tick, int id,int data );
int mob_summonslave(struct mob_data *md2,int *value,int amount,int skill_id);
int mob_countslave(struct mob_data *md);

void mob_reload(void);

#endif
