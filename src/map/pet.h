// $Id: pet.h,v 1.2 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _PET_H_
#define _PET_H_

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
extern struct petdb pet_db[MAX_PET_DB];

enum { PET_CLASS,PET_CATCH,PET_EGG,PET_EQUIP,PET_FOOD };

int pet_hungry_val(struct map_session_data &sd);
int pet_target_check(struct map_session_data &sd,struct block_list *bl,int type);
int pet_sc_check(struct map_session_data &sd, int type); //Skotlex
int pet_stopattack(struct pet_data &pd);
int pet_timer(int tid, unsigned long tick, int id, basics::numptr data);

int search_petDB_index(int key,int type);

int pet_remove_map(struct map_session_data &sd);
int pet_data_init(struct map_session_data &sd);
int pet_birth_process(struct map_session_data &sd);
int pet_recv_petdata(uint32 account_id,struct petstatus &p,int flag);
int pet_select_egg(struct map_session_data &sd,short egg_index);
int pet_catch_process1(struct map_session_data &sd,int target_class);
int pet_catch_process2(struct map_session_data &sd,uint32 target_id);
int pet_get_egg(uint32 account_id,uint32 pet_id,int flag);
int pet_menu(struct map_session_data &sd,int menunum);
int pet_change_name(struct map_session_data &sd, const char *name);
int pet_equipitem(struct map_session_data &sd,int index);
int pet_unequipitem(struct map_session_data &sd);
int pet_food(struct map_session_data &sd);
int pet_lootitem_drop(struct pet_data &pd,struct map_session_data *sd);
int pet_delay_item_drop2(int tid, unsigned long tick, int id, basics::numptr data);
int petskill_use(struct pet_data &pd, struct block_list &target, short skill_id, short skill_lv, unsigned int tick);
int pet_skill_support_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Skotlex]
int pet_skill_bonus_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_recovery_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_heal_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skillsupport_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Skotlex]

int pet_skill_bonus_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_recovery_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
//int pet_mag_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_heal_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skillattack_timer(int tid, unsigned long tick, int id, basics::numptr data); // [Valaris]
int pet_skill_support_timer(int tid, unsigned long tick, int id, basics::numptr data);// [Skotlex]

int read_petdb();
int do_init_pet(void);
int do_final_pet(void);

#endif

