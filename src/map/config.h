// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "basetypes.h"


///////////////////////////////////////////////////////////////////////////////
/// config data.
class CConfig
{
private:
	/////////////////////////////////////////////////////////////////
	/// internal name->value map
	static struct _config_map
	{
		const char *		str;
		uint32 CConfig::*	val;

/*		/////////////////////////////////////////////////////////////
		/// map internal compare
		int  compare(const char* name) const
		{
			return (this->str && name) ? strcasecmp(this->str, name) : (this->str-name);
		}

		bool operator==(const _config_map& cm) const	{ return 0==this->compare(cm.str); }
		bool operator< (const _config_map& cm) const	{ return 0> this->compare(cm.str); }

		bool operator==(const char* name) const			{ return 0==this->compare(name); }
		bool operator< (const char* name) const			{ return 0> this->compare(name); }
*/
	} config_map[];
public:
	/////////////////////////////////////////////////////////////////
	/// class data
	uint32 agi_penalty_count;
	uint32 agi_penalty_count_lv;
	uint32 agi_penalty_num;
	uint32 agi_penalty_type;
	uint32 alchemist_summon_reward;
	uint32 allow_atcommand_when_mute;
	uint32 allow_homun_status_change;
	uint32 any_warp_GM_min_level;
	uint32 area_size;
	uint32 arrow_decrement;
	uint32 atc_gmonly;
	uint32 atc_spawn_quantity_limit;
	uint32 attr_recover;
	uint32 backstab_bow_penalty;
	uint32 ban_bot;
	uint32 ban_hack_trade;
	uint32 ban_spoof_namer;
	uint32 base_exp_rate;
	uint32 basic_skill_check;
	uint32 battle_log;
	uint32 berserk_cancels_buffs;
	uint32 bone_drop;
	uint32 boss_spawn_delay;
	uint32 buyer_name;
	uint32 cardillust_read_grffile;
	uint32 cast_rate;
	uint32 castle_defense_rate;
	uint32 castrate_dex_scale;
	uint32 character_size;
	uint32 chat_warpportal;
	uint32 combo_delay_rate;
	uint32 copyskill_restrict;
	uint32 day_duration;
	uint32 dead_branch_active;
	uint32 death_penalty_base;
	uint32 death_penalty_job;
	uint32 death_penalty_type;
	uint32 defnotenemy;
	uint32 delay_battle_damage;
	uint32 delay_dependon_dex;
	uint32 delay_rate;
	uint32 devotion_level_difference;
	uint32 disp_experience;
	uint32 disp_hpmeter;
	uint32 display_delay_skill_fail;
	uint32 display_hallucination;
	uint32 display_snatcher_skill_fail;
	uint32 display_version;
	uint32 drop_rate0item;
	uint32 drop_rare_announce;
	uint32 drops_by_luk;
	uint32 dynamic_mobs;
	uint32 enemy_critical;
	uint32 enemy_critical_rate;
	uint32 enemy_perfect_flee;
	uint32 enemy_str;
	uint32 equip_natural_break_rate;
	uint32 equip_self_break_rate;
	uint32 equip_skill_break_rate;
	uint32 error_log;
	uint32 etc_log;
	uint32 exp_calc_type;
	uint32 finding_ore_rate;
	uint32 finger_offensive_type;
	uint32 flooritem_lifetime;
	uint32 gm_allequip;
	uint32 gm_allskill;
	uint32 gm_allskill_addabra;
	uint32 gm_can_drop_lv;
	uint32 gm_join_chat;
	uint32 gm_kick_chat;
	uint32 gm_skilluncond;
	uint32 gtb_pvp_only;
	uint32 guild_emperium_check;
	uint32 guild_exp_limit;
	uint32 guild_max_castles;
	uint32 gvg_eliminate_time;
	uint32 gvg_long_damage_rate;
	uint32 gvg_magic_damage_rate;
	uint32 gvg_misc_damage_rate;
	uint32 gvg_short_damage_rate;
	uint32 gvg_weapon_damage_rate;
	uint32 gx_allhit;
	uint32 gx_cardfix;
	uint32 gx_disptype;
	uint32 gx_dupele;
	uint32 hack_info_GM_level;
	uint32 headset_block_music;
	uint32 heal_exp;
	uint32 hide_GM_session;
	uint32 holywater_name_input;
	uint32 homun_creation_rate;
	uint32 homun_intimate_rate;
	uint32 homun_temporal_intimate_resilience;
	uint32 hp_rate;
	uint32 idle_no_share;
	uint32 ignore_items_gender;
	uint32 indoors_override_grffile;
	uint32 invite_request_check;
	uint32 item_auto_get;
	uint32 item_check;
	uint32 item_drop_card_max;
	uint32 item_drop_card_min;
	uint32 item_drop_common_max;
	uint32 item_drop_common_min;
	uint32 item_drop_equip_max;
	uint32 item_drop_equip_min;
	uint32 item_drop_heal_max;
	uint32 item_drop_heal_min;
	uint32 item_drop_mvp_max;
	uint32 item_drop_mvp_min;
	uint32 item_drop_use_max;
	uint32 item_drop_use_min;
	uint32 item_equip_override_grffile;
	uint32 item_first_get_time;
	uint32 item_name_override_grffile;
	uint32 item_rate_card;
	uint32 item_rate_common;
	uint32 item_rate_equip;
	uint32 item_rate_heal;
	uint32 item_rate_use;
	uint32 item_second_get_time;
	uint32 item_slots_override_grffile;
	uint32 item_third_get_time;
	uint32 item_use_interval;
	uint32 job_exp_rate;
	uint32 left_cardfix_to_right;
	uint32 magic_defense_type;
	uint32 mailsystem;
	uint32 making_arrow_name_input;
	uint32 master_get_homun_base_exp;
	uint32 master_get_homun_job_exp;
	uint32 max_adv_level;
	uint32 max_aspd;
	uint32 max_aspd_interval;
	uint32 max_base_level;
	uint32 max_cart_weight;
	uint32 max_cloth_color;
	uint32 max_hair_color;
	uint32 max_hair_style;
	uint32 max_hitrate;
	uint32 max_hp;
	uint32 max_job_level;
	uint32 max_parameter;
	uint32 max_sn_level;
	uint32 max_sp;
	uint32 max_walk_speed;
	uint32 maximum_level;
	uint32 min_cloth_color;
	uint32 min_hair_color;
	uint32 min_hair_style;
	uint32 min_hitrate;
	uint32 min_skill_delay_limit;
	uint32 mob_attack_attr_none;
	uint32 mob_changetarget_byskill;
	uint32 mob_clear_delay;
	uint32 mob_count_rate;
	uint32 mob_ghostring_fix;
	uint32 mob_remove_damaged;
	uint32 mob_remove_delay;
	uint32 mob_skill_add_range;
	uint32 mob_skill_delay;
	uint32 mob_skill_log;
	uint32 mob_skill_rate;
	uint32 mob_slaves_inherit_speed;
	uint32 mob_spawn_delay;
	uint32 mob_warpportal;
	uint32 mobs_level_up;
	uint32 monster_active_enable;
	uint32 monster_attack_direction_change;
	uint32 monster_auto_counter_type;
	uint32 monster_class_change_full_recover;
	uint32 monster_cloak_check_type;
	uint32 monster_damage_delay;
	uint32 monster_damage_delay_rate;
	uint32 monster_defense_type;
	uint32 monster_hp_rate;
	uint32 monster_land_skill_limit;
	uint32 monster_loot_type;
	uint32 monster_max_aspd;
	uint32 monster_max_aspd_interval;
	uint32 monster_skill_nofootset;
	uint32 monster_skill_reiteration;
	uint32 monsters_ignore_gm;
	uint32 motd_type;
	uint32 multi_level_up;
	uint32 muting_players;
	uint32 mvp_exp_rate;
	uint32 mvp_hp_rate;
	uint32 mvp_item_first_get_time;
	uint32 mvp_item_rate;
	uint32 mvp_item_second_get_time;
	uint32 mvp_item_third_get_time;
	uint32 natural_heal_skill_interval;
	uint32 natural_heal_weight_rate;
	uint32 natural_healhp_interval;
	uint32 natural_healsp_interval;
	uint32 night_at_start;
	uint32 night_darkness_level;
	uint32 night_duration;
	uint32 packet_ver_flag;
	uint32 party_bonus;
	uint32 party_share_mode;
	uint32 party_skill_penalty;
	uint32 pc_attack_attr_none;
	uint32 pc_attack_direction_change;
	uint32 pc_auto_counter_type;
	uint32 pc_cloak_check_type;
	uint32 pc_damage_delay;
	uint32 pc_damage_delay_rate;
	uint32 pc_invincible_time;
	uint32 pc_land_skill_limit;
	uint32 pc_skill_add_range;
	uint32 pc_skill_log;
	uint32 pc_skill_nofootset;
	uint32 pc_skill_reiteration;
	uint32 pet_attack_attr_none;
	uint32 pet_attack_exp_rate;
	uint32 pet_attack_exp_to_master;
	uint32 pet_attack_support;
	uint32 pet_catch_rate;
	uint32 pet_damage_support;
	uint32 pet_defense_type;
	uint32 pet_equip_required;
	uint32 pet_friendly_rate;
	uint32 pet_hair_style;
	uint32 pet_hungry_delay_rate;
	uint32 pet_hungry_friendly_decrease;
	uint32 pet_lv_rate;
	uint32 pet_max_atk1;
	uint32 pet_max_atk2;
	uint32 pet_max_stats;
	uint32 pet_no_gvg;
	uint32 pet_random_move;
	uint32 pet_rename;
	uint32 pet_status_support;
	uint32 pet_str;
	uint32 pet_support_min_friendly;
	uint32 pet_support_rate;
	uint32 pk_min_level;
	uint32 pk_mode;
	uint32 plant_spawn_delay;
	uint32 player_defense_type;
	uint32 player_skill_partner_check;
	uint32 pp_rate;
	uint32 prevent_logout;
	uint32 produce_item_name_input;
	uint32 produce_potion_name_input;
	uint32 pvp_exp;
	uint32 quest_skill_learn;
	uint32 quest_skill_reset;
	uint32 rainy_waterball;
	uint32 random_monster_checklv;
	uint32 require_glory_guild;
	uint32 restart_hp_rate;
	uint32 restart_sp_rate;
	uint32 resurrection_exp;
	uint32 save_clothcolor;
	uint32 save_log;
	uint32 serverside_friendlist;
	uint32 skill_delay_attack_enable;
	uint32 shop_exp;
	uint32 show_hp_sp_drain;
	uint32 show_hp_sp_gain;
	uint32 show_mob_hp;
	uint32 show_steal_in_same_party;
	uint32 skill_min_damage;
	uint32 skill_out_range_consume;
	uint32 skill_removetrap_type;
	uint32 skill_sp_override_grffile;
	uint32 skill_steal_rate;
	uint32 skill_steal_type;
	uint32 skillfree;
	uint32 skillup_limit;
	uint32 sp_rate;
	uint32 undead_detect_type;
	uint32 unit_movement_type;
	uint32 use_statpoint_table;
	uint32 vending_max_value;
	uint32 vit_penalty_count;
	uint32 vit_penalty_count_lv;
	uint32 vit_penalty_num;
	uint32 vit_penalty_type;
	uint32 warp_point_debug;
	uint32 wedding_ignorepalette;
	uint32 wedding_modifydisplay;
	uint32 who_display_aid;
	uint32 wp_rate;
	uint32 zeny_from_mobs;
	uint32 zeny_penalty;


	/////////////////////////////////////////////////////////////////
	/// constructor
	CConfig();
	/// destructor.
	~CConfig();

	/////////////////////////////////////////////////////////////////
	/// read in a configuration file
	bool read(const char *cfgName);
	/// set a config value by name
	bool set_value(const char *name, const char *value);
	/// map-like access. returns a dummy value when name does not exist
	uint32& operator[](const char *name);
	/// validate all values
	void validate();
	/// set all values to defaults
	void defaults();
};





/// type-safe message ids
typedef enum 
{
	MSG_WARPED								=  0,
	MSG_MAP_NOT_FOUND						=  1,
	MSG_COORD_OUT_OF_RANGE					=  2,
	MSG_CHAR_NOT_FOUND						=  3,
	MSG_JUMP_TO_S							=  4,
	MSG_JUMP_TO_D_D							=  5,
	MSG_CHAR_RESPAWN_SAVED					=  6,
	MSG_WARPING_TO_RESPAWN					=  7,
	MSG_SPEED_CHANGED						=  8,
	MSG_OPTIONS_CHANGED						=  9,
	MSG_INVISIBLE_OFF						= 10,
	MSG_INVISIBLE_ON						= 11,
	MSG_YOUR_JOB_CHANGED					= 12,
	MSG_A_PITY_YOUVE_DIED					= 13,
	MSG_CHARACTER_KILLED					= 14,
	MSG_PLAYER_WARPED						= 15,
	MSG_YOUVE_BEEN_REVIVED					= 16,
	MSG_HP_SP_RECOVERED						= 17,
	MSG_ITEM_CREATED						= 18,
	MSG_INVALID_ITEM						= 19,
	MSG_ITEMS_REMOVED						= 20,
	MSG_BASE_LV_RAISED						= 21,
	MSG_BASE_LV_LOWERED						= 22,
	MSG_JOB_LV_MAX							= 23,
	MSG_JOB_LV_RAISED						= 24,
	MSG_JOB_LV_LOWERED						= 25,
	MSG_HELP_COMMANDS						= 26,
	MSG_HELPTXT_NOT_FOUND					= 27,
	MSG_NO_PLAYER_FOUND						= 28,
	MSG_ONE_PLAYER_FOUND					= 29,
	MSG_D_PLAYERS_FOUND						= 30,
	MSG_PVP_OFF								= 31,
	MSG_PVP_ON								= 32,
	MSG_GVG_OFF								= 33,
	MSG_GVG_ON								= 34,
	MSG_CMD_INVALID_CLASS					= 35,
	MSG_APPEARENCE_CHANGED					= 36,
	MSG_INVALID_NUMBER						= 37,
	MSG_INVALID_LOCATION					= 38,
	MSG_ALL_MONSTER_SUMMONED				= 39,
	MSG_INVALID_MONSTER						= 40,
	MSG_IMPOSSIBLE_TO_DECREASE				= 41,
	MSG_STAT_CHANGED						= 42,
	MSG_NOT_IN_GUILD						= 43,
	MSG_NOT_GUILDMASTER						= 44,
	MSG_GLV_FAILED							= 45,
	MSG_S_RECALLED							= 46,
	MSG_BASE_LV_MAX							= 47,
	MSG_JOB_CHANGED							= 48,
	MSG_INVALID_JOB_ID						= 49,
	MSG_ALREADY_GM							= 50,
	MSG_CHAR_REVIVED						= 51,
	MSG_NOT_IN_PK							= 52,
	MSG_S_STATS								= 53,
	MSG_NO_PLAYER_IN_MAP_S					= 54,
	MSG_ONE_PLAYER_IN_MAP_S					= 55,
	MSG_D_PLAYERS_IN_MAP_S					= 56,
	MSG_CHAR_RESPAWN_CHANGED				= 57,
	MSG_CHAR_OPTIONS_CHANGED				= 58,
	MSG_NIGHT_HAS_FALLEN					= 59,
	MSG_DAY_HAS_ARRIVED						= 60,
	MSG_HAS_GIVEN_JUDGEMENT					= 61,
	MSG_JUDGEMENT_WAS_MADE					= 62,
	MSG_MERCY_HAS_BEEN_SHOWN				= 63,
	MSG_MERCY_HAS_BEEN_GRANTED				= 64,
	MSG_CHAR_BASE_LV_RAISED					= 65,
	MSG_CHAR_BASE_LV_LOWERED				= 66,
	MSG_CHAR_JOB_LV_MAX						= 67,
	MSG_CHAR_JOB_LV_RAISED					= 68,
	MSG_CHAR_JOB_LV_LOWERED					= 69,
	MSG_YOU_LEARNED_SKILL					= 70,
	MSG_YOU_FORGOT_SKILL					= 71,
	MSG_SIEGE_START							= 72,
	MSG_ALREADY_SIEGE						= 73,
	MSG_SIEGE_END							= 74,
	MSG_NOT_SIEGE							= 75,
	MSG_YOU_RECEIVED_ALL_SKILLS				= 76,
	MSG_REFERENCE_RESULT_S					= 77,
	MSG_S_D									= 78,
	MSG_D_AFFAIR_ABOVE						= 79,
	MSG_NAME_MONSTER_ID						= 80,
	MSG_GM_LV_TOO_LOW						= 81,
	MSG_USE_ONE_OF_THIS						= 82,
	MSG_CANNOT_SPAWN_EMPERIUM				= 83,
	MSG_ALL_STATS_CHANGED					= 84,
	MSG_INVALID_BAN_TIME					= 85,
	MSG_PLY_NAME_MIN_4_CHARS				= 86,
	MSG_PLY_NAME_MAX_23_CHARS				= 87,
	MSG_NAME_SEND_TO_CHAR_SERVER			= 88,
	MSG_ALREADY_NIGHT						= 89,
	MSG_ALREADY_DAY							= 90,
	MSG_CHAR_BASE_LV_MAX					= 91,
	MSG_ALL_CHARS_RECALLED					= 92,
	MSG_ALL_ON_CHARS_S_GUILD_NEAR			= 93,
	MSG_NONE_OF_GUILD_ON					= 94,
	MSG_ALL_ON_CHARS_S_PARTY_NEAR			= 95,
	MSG_NONE_OF_PARTY_ON					= 96,
	MSG_ITEM_DATABASE_RELOADED				= 97,
	MSG_MONSTER_DATABASE_RELOADED			= 98,
	MSG_SKILL_DATABASE_RELOADED				= 99,
	MSG_SCRIPTS_RELOADED					=100,
	MSG_GM_ACCOUNT_RELOAD					=101,
	MSG_MOUNTED_PECO						=102,
	MSG_NO_LONGER_SPYING_ON_THE_S_GUILD		=103,
	MSG_SPYING_ON_THE_S_GUILD				=104,
	MSG_NO_LONGER_SPYING_ON_THE_S_PARTY		=105,
	MSG_SPYING_ON_THE_S_PARTY				=106,
	MSG_ALL_ITEMS_HAVE_BEEN_REPAIRED		=107,
	MSG_NO_ITEM_NEED_TO_BE_REPAIRED			=108,
	MSG_PLAYER_HAS_BEEN_NUKED				=109,
	MSG_NPC_ENABLED							=110,
	MSG_THIS_NPC_DOESNT_EXIST				=111,
	MSG_NPC_DISABLED						=112,
	MSG_D_ITEM_REMOVED_BY_A_GM				=113,
	MSG_D_ITEM_REMOVED_FROM_THE_PLAYER		=114,
	MSG_D_ITEM_REMOVED_HAD_ONLY_D_ON_D		=115,
	MSG_CHARACTER_DOES_NOT_HAVE_THE_ITEM	=116,
	MSG_GM_HAS_SEND_YOU_IN_JAILS			=117,
	MSG_PLAYER_WARPED_IN_JAILS				=118,
	MSG_THIS_PLAYER_IS_NOT_IN_JAILS			=119,
	MSG_GM_HAS_DISCHARGE_YOU				=120,
	MSG_PLAYER_WARPED_TO_PRONTERA			=121,
	MSG_DISGUISE_APPLIED					=122,
	MSG__NAMEID_HASNT_BEEN_FOUND			=123,
	MSG_UNDISGUISE_APPLIED					=124,
	MSG_YOURE_NOT_DISGUISED					=125,
	MSG_ACCEPT_ANY_WISP						=126,
	MSG_ACCEPT_ANY_WISP_EXCEPT_FROM_D		=127,
	MSG_REFUSE_ALL_WISPS					=128,
	MSG_REFUSE_ALL_WISPS_REFUSE_FROM_D		=129,
	MSG_S_ACCEPT_ANY_WISP					=130,
	MSG_S_ACCEPT_ANY_WISP_EXCEPT_FROM_D		=131,
	MSG_S_REFUSE_ALL_WISPS					=132,
	MSG_S_REFUSE_ALL_WISPS_REFUSE_FROM_D	=133,
	MSG_S_ALREADY_ACCEPTS_ALL_WISPERS		=134,
	MSG_S_NOW_ACCEPTS_ALL_WISPERS			=135,
	MSG_AUTHORISED_ALL_WISPERS				=136,
	MSG_S_ALREADY_BLOCKS_ALL_WISPERS		=137,
	MSG_S_BLOCKS_NOW_ALL_WISPERS			=138,
	MSG_BLOCKED_ALL_WISPERS					=139,
	MSG_CHARACTERS_DISGUISE_APPLIED			=140,
	MSG_CHARACTERS_UNDISGUISE_APPLIED		=141,
	MSG_CHARACTER_IS_NOT_DISGUISED			=142,
	MSG_GIVE_A_MONSTER_NAME					=143,
	MSG_INVALID_ACTUAL_EMAIL				=144,
	MSG_INVALID_NEW_EMAIL					=145,
	MSG_EMAIL_MUST_BE_VALID					=146,
	MSG_EMAIL_MUST_BE_DIFFERENT				=147,
	MSG_SENDED_TO_LOGIN_VIA_CHAR			=148,
	MSG_IMPOSSIBLE_TO_INCREASE				=149,
	MSG_NO_GM_FOUND							=150,
	MSG_1_GM_FOUND							=151,
	MSG_D_GMS_FOUND							=152,
	MSG_S_IS_UNKNOWN_COMMAND				=153,
	MSG_S_FAILED							=154,
	MSG_IMPOSSIBLE_TO_CHANGE_YOUR_JOB		=155,
	MSG_HP_SP_MODIFIED						=156,
	MSG_HP_AND_SP_ARE_ALREADY_GOOD			=157,
	MSG_BASE_LEVEL_CANT_GO_ANY_LOWER		=158,
	MSG_JOB_LEVEL_CANT_GO_ANY_LOWER			=159,
	MSG_PVP_IS_ALREADY_OFF					=160,
	MSG_PVP_IS_ALREADY_ON					=161,
	MSG_GVG_IS_ALREADY_OFF					=162,
	MSG_GVG_IS_ALREADY_ON					=163,
	MSG_YOUR_MEMO_POINT_D_DOESNT_EXIST		=164,
	MSG_ALL_MONSTERS_KILLED					=165,
	MSG_NO_ITEM_HAS_BEEN_REFINED			=166,
	MSG_1_ITEM_HAS_BEEN_REFINED				=167,
	MSG_D_ITEMS_HAVE_BEEN_REFINED			=168,
	MSG_THIS_ITEM__IS_NOT_AN_EQUIPMENT		=169,
	MSG_THIS_ITEM_D_S_IS_NOT_AN_EQUIPMENT	=170,
	MSG_D___VOID							=171,
	MSG_REPLACE_MEMO_D_S					=172,
	MSG_DONT_HAVE_WARP_SKILL				=173,
	MSG_STATUS_POINTS_CHANGED				=174,
	MSG_SKILL_POINTS_CHANGED				=175,
	MSG_ZENYS_CHANGED						=176,
	MSG_IMPOSSIBLE_TO_DECREASE_A_STAT		=177,
	MSG_IMPOSSIBLE_TO_INCREASE_A_STAT		=178,
	MSG_GUILD_LEVEL_CHANGED					=179,
	MSG_MONTER_EGG_NAME_DOESNT_EXIST		=180,
	MSG_YOU_ALREADY_HAVE_A_PET				=181,
	MSG_PET_FRIENDLY_VALUE_CHANGED			=182,
	MSG_PET_FRIENDLY_GOOD					=183,
	MSG_SORRY_BUT_YOU_HAVE_NO_PET			=184,
	MSG_PET_HUNGRY_VALUE_CHANGED			=185,
	MSG_PET_HUNGRY_GOOD						=186,
	MSG_YOU_CAN_NOW_RENAME_YOUR_PET			=187,
	MSG_YOU_CAN_ALREADY_RENAME_YOUR_PET		=188,
	MSG_CAN_NOW_RENAME						=189,
	MSG_CAN_ALREADY_RENAME_PET				=190,
	MSG_HAS_NO_PET							=191,
	MSG_IMPOSSIBLE_TO_CHANGE_JOB			=192,
	MSG_CHARACTERS_BASE_LV_MAX				=193,
	MSG_CHARACTERS_JOB_LV_MIN				=194,
	MSG_ALL_PLAYERS_HAVE_BEEN_KICKED		=195,
	MSG_YOU_ALREADY_HAVE_THIS_QUEST_SKILL	=196,
	MSG_SKILL_NOT_EXIST_OR_NO_QUEST_SKILL	=197,
	MSG_SKILL_NOT_EXIST						=198,
	MSG_HAS_LEARNED_THE_SKILL				=199,
	MSG_ALREADY_HAS_THIS_QUEST_SKILL		=200,
	MSG_YOU_DONT_HAVE_THIS_QUEST_SKILL		=201,
	MSG_HAS_FORGOTTEN_THE_SKILL				=202,
	MSG_DOESNT_HAVE_THIS_QUEST_SKILL		=203,
	MSG_TOO_MANY_SPIRITBALLS				=204,
	MSG_ALREADY_HAS_SPIRITBALLS				=205,
	MSG_S_SKILL_POINTS_RESETED				=206,
	MSG_S_STATS_POINTS_RESETED				=207,
	MSG_S_SKILL_AND_STATS_POINTS_RESETED	=208,
	MSG_CHAR_SKILL_POINTS_CHANGED			=209,
	MSG_CHAR_STATUS_POINTS_CHANGED			=210,
	MSG_CHAR_ZENYS_CHANGED					=211,
	MSG_YOU_NOT_MOUNT_WHILE_IN_DISGUISE		=212,
	MSG_YOU_NOT_MOUNT_WITH_JOB				=213,
	MSG_UNMOUNTED_PECO						=214,
	MSG_PLAYER_NOT_MOUNT_WHILE_IN_DISGUISE	=215,
	MSG_PLAYER_MOUNTS_A_PECO				=216,
	MSG_PLAYER_NOT_MOUNT_WITH_JOB			=217,
	MSG_PLAYER_HAS_UNMOUNTED				=218,
	MSG_D_DAY								=219,
	MSG_D_DAYS								=220,
	MSG_S_D_HOUR							=221,
	MSG_S_D_HOURS							=222,
	MSG_S_D_MINUTE							=223,
	MSG_S_D_MINUTES							=224,
	MSG_S_AND_D_SECOND						=225,
	MSG_S_AND_D_SECONDS						=226,
	MSG_CANNOT_DISGUISE_WHILE_RIDING		=227,
	MSG_CHAR_CANNOT_DISGUISE_WHILE_RIDING	=228,
	MSG_YOUR_EFFECT_HAS_CHANGED				=229,
	MSG_SERVER_TIME__A_B_D_Y_X				=230,
	MSG_PERMANENT_DAYLIGHT					=231,
	MSG_PERMANENT_NIGHT						=232,
	MSG_CURRENTLY_IN_NIGHT_FOR_S			=233,
	MSG_AFTER_PERMANENT_DAYLIGHT			=234,
	MSG_CURRENTLY_IN_DAYLIGHT_FOR_S			=235,
	MSG_AFTER_PERMANENT_NIGHT				=236,
	MSG_AFTER_NIGHT_FOR_S					=237,
	MSG_DAY_CYCLE_DURATION_OF_S				=238,
	MSG_AFTER_DAYLIGHT_FOR_S				=239,
	MSG_D_MONSTER_SUMMONED					=240,
	MSG_YOU_BE_A_KILLA						=241,
	MSG_YOU_GONNA_BE_OWN3D					=242,
	MSG_MAP_SKILLS_ARE_OFF					=243,
	MSG_MAP_SKILLS_ARE_ON					=244,
	MSG_SERVER_UPTIME						=245,
	MSG_CURRENTLY_ITS_THE_NIGHT				=500,
	MSG_ACCOUNT_TIME_LIMIT_IS_DMY_HMS		=501,
	MSG_THE_DAY_HAS_ARRIVED					=502,
	MSG_THE_NIGHT_HAS_FALLEN				=503,
	MSG_YOU_HAVE_NO_MESSAGES				=510,
	MSG_D_FROM_S_PRIORITY					=511,
	MSG_D_FROM_S_NEW						=512,
	MSG_D_FROM_S							=513,
	MSG_YOU_HAVE_D_NEW_MESSAGES				=514,
	MSG_YOU_HAVE_D_UNREAD_PRIORITY_MESSAGES	=515,
	MSG_YOU_HAVE_NO_NEW_MESSAGES			=516,
	MSG_MESSAGE_NOT_FOUND					=517,
	MSG_READING_MESSAGE_FROM_S				=518,
	MSG_CANNOT_DELETE_UNREAD_PRIORITY_MAIL	=519,
	MSG_YOU_HAVE_RECIEVED_NEW_MAIL			=520,
	MSG_MESSAGE_DELETED						=521,
	MSG_10_MIN_BEFORE_SENDING				=522,
	MSG_ACCESS_DENIED						=523,
	MSG_CHARACTER_DOES_NOT_EXIST			=524,
	MSG_MAIL_HAS_BEEN_SENT					=525,
	MSG_YOU_HAVE_NEW_MAIL					=526,
	MSG_GUARDIAN_ANGEL						=540,
	MSG_NAME_IS_S_AND_SUPER_NOVICE			=541,
	MSG_PLEASE_HELP_ME						=542
} msg_t;


///////////////////////////////////////////////////////////////////////////////
/// message table
class CMessageTable
{
	///////////////////////////////////////////////////////////////////////////
	/// max number of server messages.
	/// Server messages (0-499 reserved for GM commands, 500-999 reserved for others)
	enum { MAX_MSG = 1000 };

	char* msg_table[MAX_MSG];
public:
	/// constructor.
	CMessageTable();
	/// destructor.
	~CMessageTable();

	/// return the message string by the specified number.
	const char *operator()(size_t msg_number);
	//const char *operator()(msg_t msg_id);

	/// read config file.
	bool read(const char *cfgName);
};







///////////////////////////////////////////////////////////////////////////////
/// direct access on the instanciated config/msgtable, 
/// instanciationf should actually not be placed here
extern CConfig config;
extern CMessageTable msg_txt;





#endif//_CONFIG_H_
