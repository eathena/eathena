// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_H_
#define _MOB_H_

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
	struct mob_skill skill[MAX_MOBSKILL];
};
extern struct mob_db mob_db[];

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
int mob_setdelayspawn(uint32 id);
int mob_damage(mob_data &md,int damage,int type, block_list *src);
int mob_heal(mob_data &md,int heal);

int do_init_mob(void);

void mob_unload(mob_data &md);
int mob_remove_map(mob_data &md, int type);
int mob_timer_delete(int tid, unsigned long tick, int id, basics::numptr data);

int mob_deleteslave(mob_data &md);

int mob_class_change(mob_data &md,int value[], size_t count);
int mob_warp(mob_data &md,int m,int x,int y,int type);

int mobskill_use(mob_data &md,unsigned long tick,int event);
int mobskill_event(mob_data &md,int flag);
int mobskill_castend_id(int tid, unsigned long tick, int id, basics::numptr data);
int mobskill_castend_pos(int tid, unsigned long tick, int id, basics::numptr data);
int mob_summonslave(struct mob_data &md2,int *value,size_t amount,unsigned short skillid);
unsigned int mob_countslave(mob_data &md);

bool mob_gvmobcheck(map_session_data &sd, block_list &bl);
void mob_reload(void);

#endif
