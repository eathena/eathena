// $Id: pc.h,v 1.4 2004/09/25 05:32:18 MouseJstr Exp $

#ifndef _PC_H_
#define _PC_H_

#include "map.h"

#define OPTION_MASK 0xd7b8
#define CART_MASK 0x788
#define STATE_BLIND 0x10

#define MAX_SKILL_TREE 51

extern inline void pc_setdead(struct map_session_data &sd)	{ sd.state.dead_sit = 1; }
extern inline bool pc_isdead(struct map_session_data &sd)	{ return sd.state.dead_sit == 1; }
extern inline void pc_setsit(struct map_session_data &sd)	{ sd.state.dead_sit = 2; }
extern inline bool pc_issit(struct map_session_data &sd)	{ return sd.state.dead_sit == 2; }
extern inline void pc_setdir(struct map_session_data &sd, unsigned char b, unsigned char h) { sd.dir = b; sd.head_dir = h; }
extern inline void pc_setchatid(struct map_session_data &sd, unsigned int chatid)	{ sd.chatID = chatid; }
extern inline bool pc_ishiding(struct map_session_data &sd) { return 0 != (sd.status.option&0x4006); }
extern inline bool pc_iscarton(struct map_session_data &sd) { return 0 != (sd.status.option&CART_MASK); }
extern inline bool pc_isfalcon(struct map_session_data &sd) { return 0 != (sd.status.option&0x0010); }
extern inline bool pc_isriding(struct map_session_data &sd) { return 0 != (sd.status.option&0x0020); }
extern inline bool pc_isinvisible(struct map_session_data &sd) { return 0 != (sd.status.option&0x0040); }
extern inline bool pc_is50overweight(struct map_session_data &sd) { return sd.weight*2 >= sd.max_weight; }
extern inline bool pc_is90overweight(struct map_session_data &sd) { return sd.weight*10 >= sd.max_weight*9; }

unsigned char pc_isGM(struct map_session_data &sd);
bool pc_iskiller(struct map_session_data &src, struct map_session_data &target); // [MouseJstr]
int pc_getrefinebonus(int lv,int type);

int pc_setrestartvalue(struct map_session_data &sd,int type);
int pc_makesavestatus(struct map_session_data &sd);
int pc_setnewpc(int fd, struct map_session_data &sd, unsigned long account_id, unsigned long char_id, unsigned long login_id1, unsigned long client_tick, unsigned char sex);
int pc_authok(unsigned long id, unsigned long login_id2, time_t connect_until_time, unsigned char *buf);
int pc_authfail(int fd);

bool pc_isequip(struct map_session_data &sd,int n);
int pc_equippoint(struct map_session_data &sd,int n);

bool pc_break_equip(struct map_session_data &sd, unsigned short idx);
extern inline bool pc_breakweapon(struct map_session_data &sd)	{ return pc_break_equip(sd, EQP_WEAPON); }
extern inline bool pc_breakarmor(struct map_session_data &sd)	{ return pc_break_equip(sd, EQP_ARMOR); }
extern inline bool pc_breakshield(struct map_session_data &sd)	{ return pc_break_equip(sd, EQP_SHIELD); }
extern inline bool pc_breakhelm(struct map_session_data &sd)	{ return pc_break_equip(sd, EQP_HELM); }

int pc_checkskill(struct map_session_data &sd,unsigned short skill_id);
int pc_checkallowskill(struct map_session_data &sd);
int pc_checkequip(struct map_session_data &sd,int pos);

int pc_calc_skilltree(struct map_session_data &sd);
int pc_calc_skilltree_normalize_job(struct map_session_data &sd, int c);
int pc_clean_skilltree(struct map_session_data &sd);

int pc_checkoverhp(struct map_session_data &sd);
int pc_checkoversp(struct map_session_data &sd);

bool pc_can_reach(struct map_session_data &sd, unsigned short x,unsigned short y);
int pc_walktoxy(struct map_session_data &sd, unsigned short x,unsigned short y);
int pc_stop_walking(struct map_session_data &sd, int type);
int pc_movepos(struct map_session_data &sd, unsigned short x,unsigned short y);
bool pc_setpos(struct map_session_data &sd,const char *mapname_org,unsigned short x,unsigned short y,int clrtype);
int pc_setsavepoint(struct map_session_data &sd,const char *mapname,unsigned short x,unsigned short y);
int pc_randomwarp(struct map_session_data &sd,int type);
int pc_memo(struct map_session_data &sd,int i);
int pc_randomwalk(struct map_session_data &sd,unsigned long tick);

int pc_checkadditem(struct map_session_data &sd,unsigned short nameid,unsigned short amount);
size_t pc_inventoryblank(struct map_session_data &sd);
int pc_search_inventory(struct map_session_data &sd,int item_id);
bool pc_payzeny(struct map_session_data &sd,unsigned long zeny);
bool pc_getzeny(struct map_session_data &sd,unsigned long zeny);
int pc_additem(struct map_session_data &sd,struct item &item_data,size_t amount);
int pc_delitem(struct map_session_data &sd, unsigned short inx, size_t amount, int type);
int pc_checkitem(struct map_session_data &sd);

int pc_cart_additem(struct map_session_data &sd, struct item &item_data, size_t amount);
int pc_cart_delitem(struct map_session_data &sd,unsigned short inx, size_t amount,int type);
int pc_putitemtocart(struct map_session_data &sd,unsigned short idx, size_t amount);
int pc_getitemfromcart(struct map_session_data &sd,unsigned short idx, size_t amount);
int pc_cartitem_amount(struct map_session_data &sd,unsigned short idx, size_t amount);

int pc_takeitem(struct map_session_data &sd,struct flooritem_data &fitem);
int pc_dropitem(struct map_session_data &sd,unsigned short inx, size_t amount);

int pc_checkweighticon(struct map_session_data &sd);

int pc_bonus (struct map_session_data &sd,int type,int val);
int pc_bonus2(struct map_session_data &sd,int type,int type2,int val);
int pc_bonus3(struct map_session_data &sd,int type,int type2,int type3,int val);
int pc_bonus4(struct map_session_data &sd,int type,int type2,int type3,int type4,int val);

int pc_skill(struct map_session_data &sd,unsigned short skillid,unsigned short skilllvl,int flag);
void pc_blockskill_start (struct map_session_data &sd, unsigned short skillid, unsigned long tick);	// [celest]

int pc_insert_card(struct map_session_data &sd, unsigned short idx_card, unsigned short idx_equip);

int pc_item_identify(struct map_session_data &sd, unsigned short idx);
int pc_item_repair(struct map_session_data &sd, unsigned short idx); // [Celest]
int pc_item_refine(struct map_session_data &sd, unsigned short idx); // [Celest]
int pc_steal_item(struct map_session_data &sd,struct block_list *bl);
int pc_steal_coin(struct map_session_data &sd,struct block_list *bl);

int pc_modifybuyvalue(struct map_session_data &sd,size_t orig_value);
int pc_modifysellvalue(struct map_session_data &sd,size_t orig_value);

int pc_attack(struct map_session_data &sd,unsigned long target_id,int type);
int pc_stopattack(struct map_session_data &sd);

int pc_follow(struct map_session_data &sd, unsigned long target_id); // [MouseJstr]

int pc_checkbaselevelup(struct map_session_data &sd);
int pc_checkjoblevelup(struct map_session_data &sd);
int pc_gainexp(struct map_session_data &sd, unsigned long base_exp, unsigned long job_exp);
unsigned long pc_nextbaseexp(struct map_session_data &sd);
unsigned long pc_nextbaseafter(struct map_session_data &sd); // [Valaris]
unsigned long pc_nextjobexp(struct map_session_data &sd);
unsigned long pc_nextjobafter(struct map_session_data &sd); // [Valaris]
unsigned char pc_need_status_point(struct map_session_data &sd,int type);
int pc_statusup(struct map_session_data &sd,int type);
int pc_statusup2(struct map_session_data &sd,int type,int val);
int pc_skillup(struct map_session_data &sd, unsigned short skillid);
int pc_allskillup(struct map_session_data &sd);
int pc_resetlvl(struct map_session_data &sd,int type);
int pc_resetstate(struct map_session_data &sd);
int pc_resetskill(struct map_session_data &sd);

int pc_equipitem(struct map_session_data &sd,unsigned short inx, unsigned short pos);
int pc_unequipitem(struct map_session_data &sd,unsigned short inx, int flag);
int pc_useitem(struct map_session_data &sd,unsigned short inx);

int pc_damage(struct map_session_data &sd, long damage, struct block_list *src);
int pc_heal(struct map_session_data &sd,long hp,long sp);
int pc_itemheal(struct map_session_data &sd,long hp,long sp);
int pc_percentheal(struct map_session_data &sd,long hp,long sp);
int pc_jobchange(struct map_session_data &sd,int job, int upper);
int pc_setoption(struct map_session_data &sd,int type);
int pc_setcart(struct map_session_data &sd,int type);
int pc_setfalcon(struct map_session_data &sd);
int pc_setriding(struct map_session_data &sd);
int pc_changelook(struct map_session_data &sd,int type,unsigned short val);
int pc_equiplookall(struct map_session_data &sd);

int pc_readparam(struct map_session_data &sd,int type);
int pc_setparam(struct map_session_data &sd,int type,int val);
int pc_readreg(struct map_session_data &sd,int reg);
int pc_setreg(struct map_session_data &sd,int reg,int val);
const char *pc_readregstr(struct map_session_data &sd,int reg);
int pc_setregstr(struct map_session_data &sd,int reg,const char *str);
int pc_readglobalreg(struct map_session_data &sd,const char *reg);
int pc_setglobalreg(struct map_session_data &sd,const char *reg,int val);
int pc_readaccountreg(struct map_session_data &sd,const char *reg);
int pc_setaccountreg(struct map_session_data &sd,const char *reg,int val);
int pc_readaccountreg2(struct map_session_data &sd,const char *reg);
int pc_setaccountreg2(struct map_session_data &sd,const char *reg,int val);

int pc_addeventtimer(struct map_session_data &sd,unsigned long tick,const char *name);
int pc_deleventtimer(struct map_session_data &sd,const char *name);
int pc_cleareventtimer(struct map_session_data &sd);
int pc_addeventtimercount(struct map_session_data &sd,const char *name,unsigned long tick);

int pc_calc_pvprank(struct map_session_data &sd);
int pc_calc_pvprank_timer(int tid,unsigned long tick,int id,int data);

unsigned long pc_ismarried(struct map_session_data &sd);
bool pc_marriage(struct map_session_data &sd1,struct map_session_data &sd2);
bool pc_divorce(struct map_session_data &sd);
bool pc_adoption(struct map_session_data &sd1,struct map_session_data &sd2, struct map_session_data &sd3);
struct map_session_data *pc_get_partner(struct map_session_data &sd);
struct map_session_data *pc_get_father(struct map_session_data &sd);
struct map_session_data *pc_get_mother(struct map_session_data &sd);
struct map_session_data *pc_get_child(struct map_session_data &sd);

int pc_set_gm_level(unsigned long account_id, unsigned long level);
void pc_setstand(struct map_session_data &sd);
bool pc_break_equip(struct map_session_data &sd, unsigned short where);
bool pc_candrop(struct map_session_data &sd,unsigned short item_id);

struct pc_base_job{
	int job; //E‹ÆA‚½‚¾‚µ“]¶E‚â—{qE‚Ìê‡‚ÍŒ³‚ÌE‹Æ‚ğ•Ô‚·(”pƒvƒŠ¨ƒvƒŠ)
	int type; //ƒmƒr 0, ˆêŸE 1, “ñŸE 2, ƒXƒpƒmƒr 3
	int upper; //’Êí 0, “]¶ 1, —{q 2
};

struct pc_base_job pc_calc_base_job(int b_class);//“]¶‚â—{qE‚ÌŒ³‚ÌE‹Æ‚ğ•Ô‚·
int pc_calc_base_job2(int b_class);	// Celest
int pc_calc_upper(int b_class);

struct skill_tree_entry {
	short id;
	unsigned char max;
	unsigned char joblv;
	struct {
		short id;
		unsigned char lv;
	} need[5];
}; // Celest
	
extern struct skill_tree_entry skill_tree[3][25][MAX_SKILL_TREE];

int pc_read_gm_account(int fd);
int pc_setinvincibletimer(struct map_session_data &sd,int);
int pc_delinvincibletimer(struct map_session_data &sd);
int pc_addspiritball(struct map_session_data &sd,int,int);
int pc_delspiritball(struct map_session_data &sd,int,int);
int pc_eventtimer(int tid,unsigned long tick,int id,int data);

struct fame_list {
	unsigned long id;
	unsigned long fame;
	char name[24];
};
extern struct fame_list smith_fame_list[MAX_FAMELIST];
extern struct fame_list chemist_fame_list[MAX_FAMELIST];
int pc_addfame(struct map_session_data &sd, unsigned long count,int type);
bool pc_istop10fame(unsigned long char_id,int type);

int pc_readdb(void);
int do_init_pc(void);
void do_final_pc(void);

enum {ADDITEM_EXIST,ADDITEM_NEW,ADDITEM_OVERAMOUNT};

// timer for night.day
extern int day_timer_tid;
extern int night_timer_tid;
int map_day_timer(int tid,unsigned long tick,int id,int data); // by [yor]
int map_night_timer(int tid,unsigned long tick,int id,int data); // by [yor]

#endif

