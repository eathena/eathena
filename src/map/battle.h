// $Id: battle.h,v 1.6 2004/09/29 21:08:17 Akitasha Exp $
#ifndef _BATTLE_H_
#define _BATTLE_H_

// ダメージ
struct Damage {
	int damage;
	int damage2;
	int type;
	int div_;
	int amotion;
	int dmotion;
	int blewcount;
	int flag;
	int dmg_lv;	//囲まれ減算計算用　0:スキル攻撃 ATK_LUCKY,ATK_FLEE,ATK_DEF
};

// 属性表（読み込みはpc.c、battle_attr_fixで使用）
extern int attr_fix_table[4][10][10];

struct map_session_data;
struct mob_data;
struct block_list;

// ダメージ計算

struct Damage battle_calc_attack(	int attack_type,
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag);
struct Damage battle_calc_weapon_attack(
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag);
struct Damage battle_calc_magic_attack(
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag);
struct Damage  battle_calc_misc_attack(
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag);

// 属性修正計算
int battle_attr_fix(int damage,int atk_elem,int def_elem);

// ダメージ最終計算
int battle_calc_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,short skill_lv,int flag);
enum {	// 最終計算のフラグ
	BF_WEAPON	= 0x0001,
	BF_MAGIC	= 0x0002,
	BF_MISC		= 0x0004,
	BF_SHORT	= 0x0010,
	BF_LONG		= 0x0040,
	BF_SKILL	= 0x0100,
	BF_NORMAL	= 0x0200,
	BF_WEAPONMASK=0x000f,
	BF_RANGEMASK= 0x00f0,
	BF_SKILLMASK= 0x0f00,
};

// 実際にHPを増減
int battle_delay_damage(unsigned long tick, struct block_list &src, struct block_list &target, int damage, int flag);
int battle_damage(struct block_list *bl,struct block_list *target,int damage,int flag);
int battle_heal(struct block_list *bl,struct block_list *target,int hp,int sp,int flag);

// 攻撃や移動を止める
int battle_stopattack(struct block_list *bl);
int battle_stopwalking(struct block_list *bl,int type);

// 通常攻撃処理まとめ
int battle_weapon_attack( struct block_list *bl,struct block_list *target,unsigned long tick,int flag);

// 各種パラメータを得る
unsigned int battle_counttargeted(struct block_list &bl,struct block_list *src, unsigned short target_lv);
struct block_list* battle_gettargeted(struct block_list &target);

enum {
	BCT_NOENEMY	=0x00000,
	BCT_PARTY	=0x10000,
	BCT_ENEMY	=0x40000,
	BCT_NOPARTY	=0x50000,
	BCT_ALL		=0x20000,
	BCT_NOONE	=0x60000,
	BCT_SELF	=0x60000,
};

bool battle_check_undead(int race,int element);
int battle_check_target( struct block_list *src, struct block_list *target,int flag);
bool battle_check_range(struct block_list *src,struct block_list *bl,unsigned int range);


// 設定
struct Battle_Config
{
	ulong warp_point_debug;
	ulong enemy_critical;
	ulong enemy_critical_rate;
	ulong enemy_str;
	ulong enemy_perfect_flee;
	ulong cast_rate;
	ulong delay_rate;
	ulong delay_dependon_dex;
	ulong sdelay_attack_enable;
	ulong left_cardfix_to_right;
	ulong pc_skill_add_range;
	ulong skill_out_range_consume;
	ulong mob_skill_add_range;
	ulong pc_damage_delay;
	ulong pc_damage_delay_rate;
	ulong defnotenemy;
	ulong random_monster_checklv;
	ulong attr_recover;
	ulong flooritem_lifetime;
	ulong item_auto_get;
	ulong item_first_get_time;
	ulong item_second_get_time;
	ulong item_third_get_time;
	ulong mvp_item_first_get_time;
	ulong mvp_item_second_get_time;
	ulong mvp_item_third_get_time;
	ulong base_exp_rate;
	ulong job_exp_rate;
	ulong drop_rate0item;
	ulong death_penalty_type;
	ulong death_penalty_base;
	ulong death_penalty_job;
	ulong pvp_exp;  // [MouseJstr]
	ulong gtb_pvp_only;  // [MouseJstr]
	ulong zeny_penalty;
	ulong restart_hp_rate;
	ulong restart_sp_rate;
	ulong mvp_item_rate;
	ulong mvp_exp_rate;
	ulong mvp_hp_rate;
	ulong monster_hp_rate;
	ulong monster_max_aspd;
	ulong atc_gmonly;
	ulong atc_spawn_quantity_limit;
	ulong gm_allskill;
	ulong gm_allskill_addabra;
	ulong gm_allequip;
	ulong gm_skilluncond;
	ulong gm_join_chat;
	ulong gm_kick_chat;
	ulong skillfree;
	ulong skillup_limit;
	ulong wp_rate;
	ulong pp_rate;
	ulong monster_active_enable;
	ulong monster_damage_delay_rate;
	ulong monster_loot_type;
	ulong mob_skill_rate;
	ulong mob_skill_delay;
	ulong mob_count_rate;
	ulong mob_spawn_delay;
	ulong plant_spawn_delay;
	ulong boss_spawn_delay;	// [Skotlex]
	ulong quest_skill_learn;
	ulong quest_skill_reset;
	ulong basic_skill_check;
	ulong guild_emperium_check;
	ulong guild_exp_limit;
	ulong guild_max_castles;
	ulong pc_invincible_time;
	ulong pet_catch_rate;
	ulong pet_rename;
	ulong pet_friendly_rate;
	ulong pet_hungry_delay_rate;
	ulong pet_hungry_friendly_decrease;
	ulong pet_str;
	ulong pet_status_support;
	ulong pet_attack_support;
	ulong pet_damage_support;
	ulong pet_support_rate;
	ulong pet_attack_exp_to_master;
	ulong pet_attack_exp_rate;
	ulong pet_lv_rate; //[Skotlex]
	ulong pet_max_stats; //[Skotlex]
	ulong pet_max_atk1; //[Skotlex]
	ulong pet_max_atk2; //[Skotlex]
	ulong skill_min_damage;
	ulong finger_offensive_type;
	ulong heal_exp;
	ulong resurrection_exp;
	ulong shop_exp;
	ulong combo_delay_rate;
	ulong item_check;
	ulong wedding_modifydisplay;
	ulong wedding_ignorepalette;	//[Skotlex]
	ulong natural_healhp_interval;
	ulong natural_healsp_interval;
	ulong natural_heal_skill_interval;
	ulong natural_heal_weight_rate;
	ulong item_name_override_grffile;
	ulong indoors_override_grffile;	// [Celest]
	ulong skill_sp_override_grffile;	// [Celest]
	ulong cardillust_read_grffile;
	ulong item_equip_override_grffile;
	ulong item_slots_override_grffile;
	ulong arrow_decrement;
	ulong max_aspd;
	ulong max_hp;
	ulong max_sp;
	ulong max_lv;
	ulong max_parameter;
	ulong max_cart_weight;
	ulong pc_skill_log;
	ulong mob_skill_log;
	ulong battle_log;
	ulong save_log;
	ulong error_log;
	ulong etc_log;
	ulong save_clothcolor;
	ulong undead_detect_type;
	ulong pc_auto_counter_type;
	ulong monster_auto_counter_type;
	ulong agi_penalty_type;
	ulong agi_penalty_count;
	ulong agi_penalty_num;
	ulong vit_penalty_type;
	ulong vit_penalty_count;
	ulong vit_penalty_num;
	ulong player_defense_type;
	ulong monster_defense_type;
	ulong pet_defense_type;
	ulong magic_defense_type;
	ulong pc_skill_reiteration;
	ulong monster_skill_reiteration;
	ulong pc_skill_nofootset;
	ulong monster_skill_nofootset;
	ulong pc_cloak_check_type;
	ulong monster_cloak_check_type;
	ulong gvg_short_damage_rate;
	ulong gvg_long_damage_rate;
	ulong gvg_magic_damage_rate;
	ulong gvg_misc_damage_rate;
	ulong gvg_eliminate_time;
	ulong mob_changetarget_byskill;
	ulong pc_attack_direction_change;
	ulong monster_attack_direction_change;
	ulong pc_land_skill_limit;
	ulong monster_land_skill_limit;
	ulong party_skill_penalty;
	ulong monster_class_change_full_recover;
	ulong produce_item_name_input;
	ulong produce_potion_name_input;
	ulong making_arrow_name_input;
	ulong holywater_name_input;
	ulong display_delay_skill_fail;
	ulong display_snatcher_skill_fail;
	ulong chat_warpportal;
	ulong mob_warpportal;
	ulong dead_branch_active;
	ulong vending_max_value;
	ulong show_steal_in_same_party;
	ulong pet_attack_attr_none;
	ulong mob_attack_attr_none;
	ulong mob_ghostring_fix;
	ulong pc_attack_attr_none;
	ulong item_rate_common;
	ulong item_rate_card;
	ulong item_rate_equip;
	ulong item_rate_heal;
	ulong item_rate_use;	// Added by RoVeRT, Additional Heal and Usable item rate by Val
	ulong item_drop_common_min;
	ulong item_drop_common_max;	// Added by TyrNemesis^
	ulong item_drop_card_min;
	ulong item_drop_card_max;
	ulong item_drop_equip_min;
	ulong item_drop_equip_max;
	ulong item_drop_mvp_min;
	ulong item_drop_mvp_max;	// End Addition
	ulong item_drop_heal_min;
	ulong item_drop_heal_max;	// Added by Valatris
	ulong item_drop_use_min;
	ulong item_drop_use_max;	//End
	ulong prevent_logout;	// Added by RoVeRT
	ulong alchemist_summon_reward;	// [Valaris]
	ulong maximum_level;
	ulong drops_by_luk;
	ulong monsters_ignore_gm;
	ulong equipment_breaking;
	ulong equipment_break_rate;
	ulong pet_equip_required;
	ulong multi_level_up;
	ulong pk_mode;
	ulong show_mob_hp;  // end additions [Valaris]
	ulong agi_penalty_count_lv;
	ulong vit_penalty_count_lv;
	ulong gx_allhit;
	ulong gx_cardfix;
	ulong gx_dupele;
	ulong gx_disptype;
	ulong devotion_level_difference;
	ulong player_skill_partner_check;
	ulong hide_GM_session;
	ulong unit_movement_type;
	ulong invite_request_check;
	ulong skill_removetrap_type;
	ulong disp_experience;
	ulong castle_defense_rate;
	ulong backstab_bow_penalty;
	ulong hp_rate;
	ulong sp_rate;
	ulong gm_can_drop_lv;
	ulong disp_hpmeter;
	ulong bone_drop;
	ulong monster_damage_delay;
// eAthena additions
	ulong night_at_start; // added by [Yor]
	ulong day_duration; // added by [Yor]
	ulong night_duration; // added by [Yor]
	ulong ban_spoof_namer; // added by [Yor]
	ulong ban_hack_trade; // added by [Yor]
	ulong hack_info_GM_level; // added by [Yor]
	ulong any_warp_GM_min_level; // added by [Yor]
	ulong packet_ver_flag; // added by [Yor]
	ulong muting_players; // added by [PoW]
	ulong min_hair_style; // added by [MouseJstr]
	ulong max_hair_style; // added by [MouseJstr]
	ulong min_hair_color; // added by [MouseJstr]
	ulong max_hair_color; // added by [MouseJstr]
	ulong min_cloth_color; // added by [MouseJstr]
	ulong max_cloth_color; // added by [MouseJstr]
	ulong castrate_dex_scale; // added by [MouseJstr]
	ulong area_size; // added by [MouseJstr]
	ulong zeny_from_mobs; // [Valaris]
	ulong mobs_level_up; // [Valaris]
	ulong pk_min_level; // [celest]
	ulong skill_steal_type; // [celest]
	ulong skill_steal_rate; // [celest]
	ulong night_darkness_level; // [celest]
	ulong motd_type; // [celest]
	ulong allow_atcommand_when_mute; // [celest]
	ulong finding_ore_rate; // orn
	ulong exp_calc_type;
	ulong min_skill_delay_limit;
	ulong require_glory_guild;
	ulong idle_no_share;
	ulong delay_battle_damage;
	ulong display_version;
	ulong who_display_aid;
	ulong rainy_waterball;
	ulong display_hallucination;	// [Skotlex]
	ulong use_statpoint_table;	// [Skotlex]
	ulong mail_system; // [Valaris]
	ulong new_attack_function; //For testing purposes [Skotlex]
	ulong ignore_items_gender; //[Lupus]
	ulong dynamic_mobs;
	ulong mob_remove_damaged;
	ulong mob_remove_delay;
	
	ulong max_hitrate;
	ulong min_hitrate;

	ulong show_hp_sp_drain;
	ulong show_hp_sp_gain;


	ulong party_bonus;
};

extern struct Battle_Config battle_config;

int battle_config_read(const char *cfgName);
void battle_validate_conf();
void battle_set_defaults();
int battle_set_value(const char *w1, const char *w2);

#endif
