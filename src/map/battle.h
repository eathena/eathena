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
	BCT_NOONE	=0x000000,
	BCT_SELF	=0x010000,
	BCT_ENEMY	=0x020000,
	BCT_NOENEMY	=0x1d0000,//(~BCT_ENEMY&BCT_ALL)
	BCT_PARTY	=0x050000,//Party includes self (0x04|0x01)
	BCT_NOPARTY	=0x1b0000,//(~BCT_PARTY&BCT_ALL)
	BCT_GUILD	=0x090000,//Guild includes self (0x08|0x01)
	BCT_NOGUILD	=0x170000,//(~BCT_GUILD&BCT_ALL)
	BCT_ALL		=0x1f0000,
	BCT_NEUTRAL =0x100000
};


bool battle_check_undead(int race,int element);
int battle_check_target(struct block_list *src, struct block_list *target,int flag);
bool battle_check_range(struct block_list *src,struct block_list *bl,unsigned int range);


// 設定
struct Battle_Config
{
	ulong agi_penalty_count;
	ulong agi_penalty_count_lv;
	ulong agi_penalty_num;
	ulong agi_penalty_type;
	ulong alchemist_summon_reward;
	ulong allow_atcommand_when_mute; // [celest]
	ulong any_warp_GM_min_level; // added by [Yor]
	ulong area_size; // added by [MouseJstr]
	ulong arrow_decrement;
	ulong atc_gmonly;
	ulong atc_spawn_quantity_limit;
	ulong attr_recover;
	ulong backstab_bow_penalty;
	ulong ban_bot;
	ulong ban_hack_trade; // added by [Yor]
	ulong ban_spoof_namer; // added by [Yor]
	ulong base_exp_rate;
	ulong basic_skill_check;
	ulong battle_log;
	ulong berserk_cancels_buffs;
	ulong bone_drop;
	ulong boss_spawn_delay;
	ulong buyer_name;
	ulong cardillust_read_grffile;
	ulong cast_rate;
	ulong castle_defense_rate;
	ulong castrate_dex_scale; // added by [MouseJstr]
	ulong character_size; // if riders have size=2, and baby class riders size=1 [Lupus]
	ulong chat_warpportal;
	ulong combo_delay_rate;
	ulong copyskill_restrict;
	ulong day_duration; // added by [Yor]
	ulong dead_branch_active;
	ulong death_penalty_base;
	ulong death_penalty_job;
	ulong death_penalty_type;
	ulong defnotenemy;
	ulong delay_battle_damage;
	ulong delay_dependon_dex;
	ulong delay_rate;
	ulong devotion_level_difference;
	ulong disp_experience;
	ulong disp_hpmeter;
	ulong display_delay_skill_fail;
	ulong display_hallucination;
	ulong display_snatcher_skill_fail;
	ulong display_version;
	ulong drop_rate0item;
	ulong drop_rare_announce;
	ulong drops_by_luk;
	ulong dynamic_mobs;
	ulong enemy_critical;
	ulong enemy_critical_rate;
	ulong enemy_perfect_flee;
	ulong enemy_str;
	ulong equip_natural_break_rate;
	ulong equip_self_break_rate;
	ulong equip_skill_break_rate;
	ulong error_log;
	ulong etc_log;
	ulong exp_calc_type;
	ulong finding_ore_rate; // orn
	ulong finger_offensive_type;
	ulong flooritem_lifetime;
	ulong gm_allequip;
	ulong gm_allskill;
	ulong gm_allskill_addabra;
	ulong gm_can_drop_lv;
	ulong gm_join_chat;
	ulong gm_kick_chat;
	ulong gm_skilluncond;
	ulong gtb_pvp_only;  // [MouseJstr]
	ulong guild_emperium_check;
	ulong guild_exp_limit;
	ulong guild_max_castles;
	ulong gvg_eliminate_time;
	ulong gvg_long_damage_rate;
	ulong gvg_magic_damage_rate;
	ulong gvg_misc_damage_rate;
	ulong gvg_short_damage_rate;
	ulong gvg_weapon_damage_rate;
	ulong gx_allhit;
	ulong gx_cardfix;
	ulong gx_disptype;
	ulong gx_dupele;
	ulong hack_info_GM_level; // added by [Yor]
	ulong headset_block_music; // do headsets block Frost Joke, etc [Lupus]
	ulong heal_exp;
	ulong hide_GM_session;
	ulong holywater_name_input;
	ulong hp_rate;
	ulong idle_no_share;
	ulong ignore_items_gender; //[Lupus]
	ulong indoors_override_grffile;
	ulong invite_request_check;
	ulong item_auto_get;
	ulong item_check;
	ulong item_drop_card_max;
	ulong item_drop_card_min;
	ulong item_drop_common_max;
	ulong item_drop_common_min;
	ulong item_drop_equip_max;
	ulong item_drop_equip_min;
	ulong item_drop_heal_max;
	ulong item_drop_heal_min;
	ulong item_drop_mvp_max;
	ulong item_drop_mvp_min;
	ulong item_drop_use_max;
	ulong item_drop_use_min;
	ulong item_equip_override_grffile;
	ulong item_first_get_time;
	ulong item_name_override_grffile;
	ulong item_rate_card;
	ulong item_rate_common;
	ulong item_rate_equip;
	ulong item_rate_heal;
	ulong item_rate_use;
	ulong item_second_get_time;
	ulong item_slots_override_grffile;
	ulong item_third_get_time;
	ulong item_use_interval;
	ulong job_exp_rate;
	ulong left_cardfix_to_right;
	ulong magic_defense_type;
	ulong mail_system; // [Valaris]
	ulong making_arrow_name_input;
	ulong max_adv_level;
	ulong max_aspd;
	ulong max_aspd_interval; // not writable
	ulong max_base_level;
	ulong max_cart_weight;
	ulong max_cloth_color; // added by [MouseJstr]
	ulong max_hair_color; // added by [MouseJstr]
	ulong max_hair_style; // added by [MouseJstr]
	ulong max_hitrate;
	ulong max_hp;
	ulong max_job_level;
	ulong max_parameter;
	ulong max_sn_level;
	ulong max_sp;
	ulong max_walk_speed;
	ulong maximum_level;
	ulong min_cloth_color; // added by [MouseJstr]
	ulong min_hair_color; // added by [MouseJstr]
	ulong min_hair_style; // added by [MouseJstr]
	ulong min_hitrate;
	ulong min_skill_delay_limit;
	ulong mob_attack_attr_none;
	ulong mob_changetarget_byskill;
	ulong mob_clear_delay;
	ulong mob_count_rate;
	ulong mob_ghostring_fix;
	ulong mob_remove_damaged;
	ulong mob_remove_delay;
	ulong mob_skill_add_range;
	ulong mob_skill_delay;
	ulong mob_skill_log;
	ulong mob_skill_rate;
	ulong mob_slaves_inherit_speed;
	ulong mob_spawn_delay;
	ulong mob_warpportal;
	ulong mobs_level_up; // [Valaris]
	ulong monster_active_enable;
	ulong monster_attack_direction_change;
	ulong monster_auto_counter_type;
	ulong monster_class_change_full_recover;
	ulong monster_cloak_check_type;
	ulong monster_damage_delay;
	ulong monster_damage_delay_rate;
	ulong monster_defense_type;
	ulong monster_hp_rate;
	ulong monster_land_skill_limit;
	ulong monster_loot_type;
	ulong monster_max_aspd;
	ulong monster_max_aspd_interval;// not writable, 
	ulong monster_skill_nofootset;
	ulong monster_skill_reiteration;
	ulong monsters_ignore_gm;
	ulong motd_type; // [celest]
	ulong multi_level_up;
	ulong muting_players; // added by [PoW]
	ulong mvp_exp_rate;
	ulong mvp_hp_rate;
	ulong mvp_item_first_get_time;
	ulong mvp_item_rate;
	ulong mvp_item_second_get_time;
	ulong mvp_item_third_get_time;
	ulong natural_heal_skill_interval;
	ulong natural_heal_weight_rate;
	ulong natural_healhp_interval;
	ulong natural_healsp_interval;
	ulong new_attack_function; //For testing purposes [Skotlex]
	ulong night_at_start; // added by [Yor]
	ulong night_darkness_level; // [celest]
	ulong night_duration; // added by [Yor]
	ulong packet_ver_flag; // added by [Yor]
	ulong party_bonus;
	ulong party_share_mode;
	ulong party_skill_penalty;
	ulong pc_attack_attr_none;
	ulong pc_attack_direction_change;
	ulong pc_auto_counter_type;
	ulong pc_cloak_check_type;
	ulong pc_damage_delay;
	ulong pc_damage_delay_rate;
	ulong pc_invincible_time;
	ulong pc_land_skill_limit;
	ulong pc_skill_add_range;
	ulong pc_skill_log;
	ulong pc_skill_nofootset;
	ulong pc_skill_reiteration;
	ulong pet_attack_attr_none;
	ulong pet_attack_exp_rate;
	ulong pet_attack_exp_to_master;
	ulong pet_attack_support;
	ulong pet_catch_rate;
	ulong pet_damage_support;
	ulong pet_defense_type;
	ulong pet_equip_required;
	ulong pet_friendly_rate;
	ulong pet_hair_style; // added by [Skotlex]
	ulong pet_hungry_delay_rate;
	ulong pet_hungry_friendly_decrease;
	ulong pet_lv_rate; //[Skotlex]
	ulong pet_max_atk1; //[Skotlex]
	ulong pet_max_atk2; //[Skotlex]
	ulong pet_max_stats; //[Skotlex]
	ulong pet_no_gvg; //Disables pets in gvg. [Skotlex]
	ulong pet_random_move;
	ulong pet_rename;
	ulong pet_status_support;
	ulong pet_str;
	ulong pet_support_min_friendly;
	ulong pet_support_rate;
	ulong pk_min_level; // [celest]
	ulong pk_mode;
	ulong plant_spawn_delay;
	ulong player_defense_type;
	ulong player_skill_partner_check;
	ulong pp_rate;
	ulong prevent_logout;
	ulong produce_item_name_input;
	ulong produce_potion_name_input;
	ulong pvp_exp;  // [MouseJstr]
	ulong quest_skill_learn;
	ulong quest_skill_reset;
	ulong rainy_waterball;
	ulong random_monster_checklv;
	ulong require_glory_guild;
	ulong restart_hp_rate;
	ulong restart_sp_rate;
	ulong resurrection_exp;
	ulong save_clothcolor;
	ulong save_log;
	ulong sdelay_attack_enable;
	ulong shop_exp;
	ulong show_hp_sp_drain;
	ulong show_hp_sp_gain;
	ulong show_mob_hp;  // end additions [Valaris]
	ulong show_steal_in_same_party;
	ulong skill_min_damage;
	ulong skill_out_range_consume;
	ulong skill_removetrap_type;
	ulong skill_sp_override_grffile;
	ulong skill_steal_rate; // [celest]
	ulong skill_steal_type; // [celest]
	ulong skillfree;
	ulong skillup_limit;
	ulong sp_rate;
	ulong undead_detect_type;
	ulong unit_movement_type;
	ulong use_statpoint_table;
	ulong vending_max_value;
	ulong vit_penalty_count;
	ulong vit_penalty_count_lv;
	ulong vit_penalty_num;
	ulong vit_penalty_type;
	ulong warp_point_debug;
	ulong wedding_ignorepalette;
	ulong wedding_modifydisplay;
	ulong who_display_aid;
	ulong wp_rate;
	ulong zeny_from_mobs; // [Valaris]
	ulong zeny_penalty;
};

extern struct Battle_Config battle_config;

int battle_config_read(const char *cfgName);
void battle_validate_conf();
void battle_set_defaults();
int battle_set_value(const char *w1, const char *w2);

#endif
