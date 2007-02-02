// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _OLDSTATUS_H_
#define _OLDSTATUS_H_

#include "map.h" // for basic type definitions, move to a seperated file
#include "fightable.h"


extern int SkillStatusChangeTable[];

extern int current_equip_item_index;


int status_recalc_speed(block_list *bl);

// パラメータ所得系 battle.c より移動
int status_get_hit(block_list *bl);
int status_get_flee(block_list *bl);
int status_get_def(block_list *bl);
int status_get_mdef(block_list *bl);
int status_get_flee2(block_list *bl);
int status_get_def2(const block_list *bl);
int status_get_mdef2(const block_list *bl);
int status_get_baseatk(block_list *bl);
int status_get_atk(block_list *bl);
int status_get_atk2(block_list *bl);
int status_get_adelay(block_list *bl);
int status_get_amotion(const block_list *bl);
int status_get_dmotion(block_list *bl);
int status_get_element(const block_list *bl);
int status_get_attack_element(block_list *bl);
int status_get_attack_element2(block_list *bl);  //左手武器属性取得
#define status_get_elem_type(bl)	(status_get_element(bl)%10)
#define status_get_elem_level(bl)	(status_get_element(bl)/10/2)



short *status_get_opt1(block_list *bl);
short *status_get_opt2(block_list *bl);
short *status_get_opt3(block_list *bl);
short *status_get_option(block_list *bl);

int status_get_matk1(block_list *bl);
int status_get_matk2(block_list *bl);
int status_get_critical(block_list *bl);
int status_get_atk_(block_list *bl);
int status_get_atk_2(block_list *bl);
int status_get_atk2(block_list *bl);

int status_isimmune(block_list *bl);

int status_get_sc_def(block_list *bl, int type);
#define status_get_sc_def_mdef(bl)	(status_get_sc_def(bl, SP_MDEF1))
#define status_get_sc_def_vit(bl)	(status_get_sc_def(bl, SP_DEF2))
#define status_get_sc_def_int(bl)	(status_get_sc_def(bl, SP_MDEF2))
#define status_get_sc_def_luk(bl)	(status_get_sc_def(bl, SP_LUK))

// 状態異常関連 skill.c より移動
int status_change_start(block_list *bl, status_t type,basics::numptr val1,basics::numptr val2,basics::numptr val3,basics::numptr val4,unsigned long tick,int flag);
int status_change_end(block_list* bl, status_t type,int tid );
int status_change_timer(int tid, unsigned long tick, int id, basics::numptr data);

class CStatusChangetimer : public CMapProcessor
{
	block_list &src;
	int type;
	unsigned long tick;
public:
	CStatusChangetimer(block_list &s, int ty, unsigned long t)
		: src(s), type(ty), tick(t)
	{}
	~CStatusChangetimer()	{}
	virtual int process(block_list& bl) const;
};

int status_change_clear(block_list *bl,int type);
int status_change_clear_buffs(block_list *bl);
int status_change_clear_debuffs(block_list *bl);

int status_calc_pet(map_session_data &sd, bool first); // [Skotlex]
// ステータス計算 pc.c から分離
// pc_calcstatus

int status_calc_pc(map_session_data &sd,int first);
int status_calc_speed(map_session_data &sd, unsigned short skill_num, unsigned short skill_lv, int start);
int status_calc_speed_old(map_session_data &sd); // [Celest]
// int status_calc_skilltree(map_session_data *sd);
int status_getrefinebonus(int lv,int type);
int status_percentrefinery(map_session_data &sd,struct item &item);
//Use this to refer the max refinery level [Skotlex]

extern int percentrefinery[MAX_REFINE_BONUS][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

int status_readdb(void);
int do_init_status(void);





#endif
