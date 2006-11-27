// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "baseparam.h"
#include "config.h"
#include "showmsg.h"
#include "utils.h"
#include "map.h"


///////////////////////////////////////////////////////////////////////////////
/// instanciation of the configration
CBattleConfig config;
CMessageTable msg_txt;


///////////////////////////////////////////////////////////////////////////////
/// internal name->value map
CBattleConfig::_config_map CBattleConfig::config_map[] =
{
	{ "_temp_",								&CBattleConfig::_temp_								},
	{ "agi_penalty_count",					&CBattleConfig::agi_penalty_count					},
	{ "agi_penalty_count_lv",				&CBattleConfig::agi_penalty_count_lv				},
	{ "agi_penalty_num",					&CBattleConfig::agi_penalty_num						},
	{ "agi_penalty_type",					&CBattleConfig::agi_penalty_type					},
	{ "alchemist_summon_reward",			&CBattleConfig::alchemist_summon_reward				},
	{ "allow_atcommand_when_mute",			&CBattleConfig::allow_atcommand_when_mute			},
	{ "allow_homun_status_change",			&CBattleConfig::allow_homun_status_change			},
	{ "any_warp_GM_min_level",				&CBattleConfig::any_warp_GM_min_level				},
	{ "area_size",							&CBattleConfig::area_size							},
	{ "arrow_decrement",					&CBattleConfig::arrow_decrement						},
	{ "atcommand_gm_only",					&CBattleConfig::atc_gmonly							},
	{ "atcommand_spawn_quantity_limit",		&CBattleConfig::atc_spawn_quantity_limit			},
	{ "attribute_recover",					&CBattleConfig::attr_recover						},
	{ "backstab_bow_penalty",				&CBattleConfig::backstab_bow_penalty				},
	{ "ban_bot",							&CBattleConfig::ban_bot								},
	{ "ban_spoof_namer",					&CBattleConfig::ban_spoof_namer						},
	{ "base_exp_rate",						&CBattleConfig::base_exp_rate						},
	{ "basic_skill_check",					&CBattleConfig::basic_skill_check					},
	{ "battle_log",							&CBattleConfig::battle_log							},
	{ "berserk_canc	els_buffs",				&CBattleConfig::berserk_cancels_buffs				},
	{ "bone_drop",							&CBattleConfig::bone_drop							},
	{ "boss_spawn_delay",					&CBattleConfig::boss_spawn_delay					},
	{ "buyer_name",							&CBattleConfig::buyer_name							},
	{ "cardillust_read_grffile",			&CBattleConfig::cardillust_read_grffile				},
	{ "casting_rate",						&CBattleConfig::cast_rate							},
	{ "castle_defense_rate",				&CBattleConfig::castle_defense_rate					},
	{ "castrate_dex_scale",					&CBattleConfig::castrate_dex_scale					},
	{ "character_size",						&CBattleConfig::character_size						},
	{ "chat_warpportal",					&CBattleConfig::chat_warpportal						},
	{ "combo_delay_rate",					&CBattleConfig::combo_delay_rate					},
	{ "copyskill_restrict",					&CBattleConfig::copyskill_restrict					},
	{ "day_duration",						&CBattleConfig::day_duration						},
	{ "dead_branch_active",					&CBattleConfig::dead_branch_active					},
	{ "death_penalty_base",					&CBattleConfig::death_penalty_base					},
	{ "death_penalty_job",					&CBattleConfig::death_penalty_job					},
	{ "death_penalty_type",					&CBattleConfig::death_penalty_type					},
	{ "defunit_not_enemy",					&CBattleConfig::defnotenemy							},
	{ "delay_battle_damage",				&CBattleConfig::delay_battle_damage					},
	{ "delay_dependon_dex",					&CBattleConfig::delay_dependon_dex					},
	{ "delay_rate",							&CBattleConfig::delay_rate							},
	{ "devotion_level_difference",			&CBattleConfig::devotion_level_difference			},
	{ "disp_experience",					&CBattleConfig::disp_experience						},
	{ "disp_hpmeter",						&CBattleConfig::disp_hpmeter						},
	{ "display_delay_skill_fail",			&CBattleConfig::display_delay_skill_fail			},
	{ "display_hallucination",				&CBattleConfig::display_hallucination				},
	{ "display_snatcher_skill_fail",		&CBattleConfig::display_snatcher_skill_fail			},
	{ "display_version",					&CBattleConfig::display_version						},
	{ "drop_rate0item",						&CBattleConfig::drop_rate0item						},
	{ "drop_rare_announce",					&CBattleConfig::drop_rare_announce					},
	{ "drops_by_luk",						&CBattleConfig::drops_by_luk						},
	{ "dynamic_mobs",						&CBattleConfig::dynamic_mobs						},
	{ "enemy_critical",						&CBattleConfig::enemy_critical						},
	{ "enemy_critical_rate",				&CBattleConfig::enemy_critical_rate					},
	{ "enemy_perfect_flee",					&CBattleConfig::enemy_perfect_flee					},
	{ "enemy_str",							&CBattleConfig::enemy_str							},
	{ "equip_natural_break_rate",			&CBattleConfig::equip_natural_break_rate			},
	{ "equip_self_break_rate",				&CBattleConfig::equip_self_break_rate				},
	{ "equip_skill_break_rate",				&CBattleConfig::equip_skill_break_rate				},
	{ "error_log",							&CBattleConfig::error_log							},
	{ "etc_log",							&CBattleConfig::etc_log								},
	{ "exp_calc_type",						&CBattleConfig::exp_calc_type						},
	{ "finding_ore_rate",					&CBattleConfig::finding_ore_rate					},
	{ "finger_offensive_type",				&CBattleConfig::finger_offensive_type				},
	{ "flooritem_lifetime",					&CBattleConfig::flooritem_lifetime					},
	{ "gm_all_equipment",					&CBattleConfig::gm_allequip							},
	{ "gm_all_skill",						&CBattleConfig::gm_allskill							},
	{ "gm_all_skill_add_abra",				&CBattleConfig::gm_allskill_addabra					},
	{ "gm_can_drop_lv",						&CBattleConfig::gm_can_drop_lv						},
	{ "gm_join_chat",						&CBattleConfig::gm_join_chat						},
	{ "gm_kick_chat",						&CBattleConfig::gm_kick_chat						},
	{ "gm_skill_unconditional",				&CBattleConfig::gm_skilluncond						},
	{ "gtb_pvp_only",						&CBattleConfig::gtb_pvp_only						},
	{ "guild_emperium_check",				&CBattleConfig::guild_emperium_check				},
	{ "guild_exp_limit",					&CBattleConfig::guild_exp_limit						},
	{ "guild_max_castles",					&CBattleConfig::guild_max_castles					},
	{ "gvg_eliminate_time",					&CBattleConfig::gvg_eliminate_time					},
	{ "gvg_long_attack_damage_rate",		&CBattleConfig::gvg_long_damage_rate				},
	{ "gvg_magic_attack_damage_rate",		&CBattleConfig::gvg_magic_damage_rate				},
	{ "gvg_misc_attack_damage_rate",		&CBattleConfig::gvg_misc_damage_rate				},
	{ "gvg_short_attack_damage_rate",		&CBattleConfig::gvg_short_damage_rate				},
	{ "gvg_weapon_damage_rate",				&CBattleConfig::gvg_weapon_damage_rate				},
	{ "gx_allhit",							&CBattleConfig::gx_allhit							},
	{ "gx_cardfix",							&CBattleConfig::gx_cardfix							},
	{ "gx_disptype",						&CBattleConfig::gx_disptype							},
	{ "gx_dupele",							&CBattleConfig::gx_dupele							},
	{ "hack_info_GM_level",					&CBattleConfig::hack_info_GM_level					},
	{ "headset_block_music",				&CBattleConfig::headset_block_music					},
	{ "heal_exp",							&CBattleConfig::heal_exp							},
	{ "hide_GM_session",					&CBattleConfig::hide_GM_session						},
	{ "holywater_name_input",				&CBattleConfig::holywater_name_input				},
	{ "homun_creation_rate",				&CBattleConfig::homun_creation_rate					},
	{ "homun_intimate_rate",				&CBattleConfig::homun_intimate_rate					},
	{ "homun_temporal_intimate_resilience",	&CBattleConfig::homun_temporal_intimate_resilience	},
	{ "hp_rate",							&CBattleConfig::hp_rate								},
	{ "idle_no_share",						&CBattleConfig::idle_no_share						},
	{ "ignore_items_gender",				&CBattleConfig::ignore_items_gender					},
	{ "indoors_override_grffile",			&CBattleConfig::indoors_override_grffile			},
	{ "invite_request_check",				&CBattleConfig::invite_request_check				},
	{ "item_auto_get",						&CBattleConfig::item_auto_get						},
	{ "item_check",							&CBattleConfig::item_check							},
	{ "item_drop_card_max",					&CBattleConfig::item_drop_card_max					},
	{ "item_drop_card_min",					&CBattleConfig::item_drop_card_min					},
	{ "item_drop_common_max",				&CBattleConfig::item_drop_common_max				},
	{ "item_drop_common_min",				&CBattleConfig::item_drop_common_min				},
	{ "item_drop_equip_max",				&CBattleConfig::item_drop_equip_max					},
	{ "item_drop_equip_min",				&CBattleConfig::item_drop_equip_min					},
	{ "item_drop_heal_max",					&CBattleConfig::item_drop_heal_max					},
	{ "item_drop_heal_min",					&CBattleConfig::item_drop_heal_min					},
	{ "item_drop_mvp_max",					&CBattleConfig::item_drop_mvp_max					},
	{ "item_drop_mvp_min",					&CBattleConfig::item_drop_mvp_min					},
	{ "item_drop_use_max",					&CBattleConfig::item_drop_use_max					},
	{ "item_drop_use_min",					&CBattleConfig::item_drop_use_min					},
	{ "item_equip_override_grffile",		&CBattleConfig::item_equip_override_grffile			},
	{ "item_first_get_time",				&CBattleConfig::item_first_get_time					},
	{ "item_name_override_grffile",			&CBattleConfig::item_name_override_grffile			},
	{ "item_rate_card",						&CBattleConfig::item_rate_card						},
	{ "item_rate_common",					&CBattleConfig::item_rate_common					},
	{ "item_rate_equip",					&CBattleConfig::item_rate_equip						},
	{ "item_rate_heal",						&CBattleConfig::item_rate_heal						},
	{ "item_rate_use",						&CBattleConfig::item_rate_use						},
	{ "item_second_get_time",				&CBattleConfig::item_second_get_time				},
	{ "item_slots_override_grffile",		&CBattleConfig::item_slots_override_grffile			},
	{ "item_third_get_time",				&CBattleConfig::item_third_get_time					},
	{ "item_use_interval",					&CBattleConfig::item_use_interval					},
	{ "job_exp_rate",						&CBattleConfig::job_exp_rate						},
	{ "left_cardfix_to_right",				&CBattleConfig::left_cardfix_to_right				},
	{ "magic_defense_type",					&CBattleConfig::magic_defense_type					},
	{ "mailsystem",							&CBattleConfig::mailsystem							},
	{ "making_arrow_name_input",			&CBattleConfig::making_arrow_name_input				},
	{ "master_get_homun_base_exp",			&CBattleConfig::master_get_homun_base_exp			},
	{ "master_get_homun_job_exp",			&CBattleConfig::master_get_homun_job_exp			},
	{ "max_adv_level",						&CBattleConfig::max_adv_level						},
	{ "max_aspd",							&CBattleConfig::max_aspd							},
	{ "max_base_level",						&CBattleConfig::max_base_level						},
	{ "max_cart_weight",					&CBattleConfig::max_cart_weight						},
	{ "max_cloth_color",					&CBattleConfig::max_cloth_color						},
	{ "max_hair_color",						&CBattleConfig::max_hair_color						},
	{ "max_hair_style",						&CBattleConfig::max_hair_style						},
	{ "max_hitrate",						&CBattleConfig::max_hitrate							},
	{ "max_hp",								&CBattleConfig::max_hp								},
	{ "max_job_level",						&CBattleConfig::max_job_level						},
	{ "max_parameter",						&CBattleConfig::max_parameter						},
	{ "max_sn_level",						&CBattleConfig::max_sn_level						},
	{ "max_sp",								&CBattleConfig::max_sp								},
	{ "max_walk_speed",						&CBattleConfig::max_walk_speed						},
	{ "maximum_level",						&CBattleConfig::maximum_level						},
	{ "min_cloth_color",					&CBattleConfig::min_cloth_color						},
	{ "min_hair_color",						&CBattleConfig::min_hair_color						},
	{ "min_hair_style",						&CBattleConfig::min_hair_style						},
	{ "min_hitrate",						&CBattleConfig::min_hitrate							},
	{ "min_skill_delay_limit",				&CBattleConfig::min_skill_delay_limit				},
	{ "mob_attack_attr_none",				&CBattleConfig::mob_attack_attr_none				},
	{ "mob_changetarget_byskill",			&CBattleConfig::mob_changetarget_byskill			},
	{ "mob_clear_delay",					&CBattleConfig::mob_clear_delay						},
	{ "mob_count_rate",						&CBattleConfig::mob_count_rate						},
	{ "mob_ghostring_fix",					&CBattleConfig::mob_ghostring_fix					},
	{ "mob_remove_damaged",					&CBattleConfig::mob_remove_damaged					},
	{ "mob_remove_delay",					&CBattleConfig::mob_remove_delay					},
	{ "mob_skill_delay",					&CBattleConfig::mob_skill_delay						},
	{ "mob_skill_rate",						&CBattleConfig::mob_skill_rate						},
	{ "mob_slaves_inherit_speed",			&CBattleConfig::mob_slaves_inherit_speed			},
	{ "mob_spawn_delay",					&CBattleConfig::mob_spawn_delay						},
	{ "mob_warpportal",						&CBattleConfig::mob_warpportal						},
	{ "mobs_level_up",						&CBattleConfig::mobs_level_up						},
	{ "monster_active_enable",				&CBattleConfig::monster_active_enable				},
	{ "monster_attack_direction_change",	&CBattleConfig::monster_attack_direction_change		},
	{ "monster_auto_counter_type",			&CBattleConfig::monster_auto_counter_type			},
	{ "monster_class_change_full_recover",	&CBattleConfig::monster_class_change_full_recover 	},
	{ "monster_cloak_check_type",			&CBattleConfig::monster_cloak_check_type			},
	{ "monster_damage_delay",				&CBattleConfig::monster_damage_delay				},
	{ "monster_damage_delay_rate",			&CBattleConfig::monster_damage_delay_rate			},
	{ "monster_defense_type",				&CBattleConfig::monster_defense_type				},
	{ "monster_hp_rate",					&CBattleConfig::monster_hp_rate						},
	{ "monster_land_skill_limit",			&CBattleConfig::monster_land_skill_limit			},
	{ "monster_loot_type",					&CBattleConfig::monster_loot_type					},
	{ "monster_max_aspd",					&CBattleConfig::monster_max_aspd					},
	{ "monster_skill_add_range",			&CBattleConfig::mob_skill_add_range					},
	{ "monster_skill_log",					&CBattleConfig::mob_skill_log						},
	{ "monster_skill_nofootset",			&CBattleConfig::monster_skill_nofootset				},
	{ "monster_skill_reiteration",			&CBattleConfig::monster_skill_reiteration			},
	{ "monsters_ignore_gm",					&CBattleConfig::monsters_ignore_gm					},
	{ "motd_type",							&CBattleConfig::motd_type							},
	{ "multi_level_up",						&CBattleConfig::multi_level_up						},
	{ "muting_players",						&CBattleConfig::muting_players						},
	{ "mvp_exp_rate",						&CBattleConfig::mvp_exp_rate						},
	{ "mvp_hp_rate",						&CBattleConfig::mvp_hp_rate							},
	{ "mvp_item_first_get_time",			&CBattleConfig::mvp_item_first_get_time				},
	{ "mvp_item_rate",						&CBattleConfig::mvp_item_rate						},
	{ "mvp_item_second_get_time",			&CBattleConfig::mvp_item_second_get_time			},
	{ "mvp_item_third_get_time",			&CBattleConfig::mvp_item_third_get_time				},
	{ "natural_heal_skill_interval",		&CBattleConfig::natural_heal_skill_interval			},
	{ "natural_heal_weight_rate",			&CBattleConfig::natural_heal_weight_rate			},
	{ "natural_healhp_interval",			&CBattleConfig::natural_healhp_interval				},
	{ "natural_healsp_interval",			&CBattleConfig::natural_healsp_interval				},
	{ "night_at_start",						&CBattleConfig::night_at_start						},
	{ "night_darkness_level",				&CBattleConfig::night_darkness_level				},
	{ "night_duration",						&CBattleConfig::night_duration						},
	{ "packet_ver_flag",					&CBattleConfig::packet_ver_flag						},
	{ "party_bonus",						&CBattleConfig::party_bonus							},
	{ "party_share_mode",					&CBattleConfig::party_share_mode					},
	{ "party_skill_penalty",				&CBattleConfig::party_skill_penalty					},
	{ "pc_attack_attr_none",				&CBattleConfig::pc_attack_attr_none					},
	{ "pet_attack_attr_none",				&CBattleConfig::pet_attack_attr_none				},
	{ "pet_attack_exp_rate",				&CBattleConfig::pet_attack_exp_rate					},
	{ "pet_attack_exp_to_master",			&CBattleConfig::pet_attack_exp_to_master			},
	{ "pet_attack_support",					&CBattleConfig::pet_attack_support					},
	{ "pet_catch_rate",						&CBattleConfig::pet_catch_rate						},
	{ "pet_damage_support",					&CBattleConfig::pet_damage_support					},
	{ "pet_defense_type",					&CBattleConfig::pet_defense_type					},
	{ "pet_equip_required",					&CBattleConfig::pet_equip_required					},
	{ "pet_friendly_rate",					&CBattleConfig::pet_friendly_rate					},
	{ "pet_hair_style",						&CBattleConfig::pet_hair_style						},
	{ "pet_hungry_delay_rate",				&CBattleConfig::pet_hungry_delay_rate				},
	{ "pet_hungry_friendly_decrease",		&CBattleConfig::pet_hungry_friendly_decrease		},
	{ "pet_lv_rate",						&CBattleConfig::pet_lv_rate							},
	{ "pet_max_atk1",						&CBattleConfig::pet_max_atk1						},
	{ "pet_max_atk2",						&CBattleConfig::pet_max_atk2						},
	{ "pet_max_stats",						&CBattleConfig::pet_max_stats						},
	{ "pet_no_gvg",							&CBattleConfig::pet_no_gvg							},
	{ "pet_random_move",					&CBattleConfig::pet_random_move						},
	{ "pet_rename",							&CBattleConfig::pet_rename							},
	{ "pet_status_support",					&CBattleConfig::pet_status_support					},
	{ "pet_str",							&CBattleConfig::pet_str								},
	{ "pet_support_min_friendly",			&CBattleConfig::pet_support_min_friendly			},
	{ "pet_support_rate",					&CBattleConfig::pet_support_rate					},
	{ "pk_min_level",						&CBattleConfig::pk_min_level						},
	{ "pk_mode",							&CBattleConfig::pk_mode								},
	{ "plant_spawn_delay",					&CBattleConfig::plant_spawn_delay					},
	{ "player_attack_direction_change",		&CBattleConfig::pc_attack_direction_change			},
	{ "player_auto_counter_type",			&CBattleConfig::pc_auto_counter_type				},
	{ "player_cloak_check_type",			&CBattleConfig::pc_cloak_check_type					},
	{ "player_damage_delay",				&CBattleConfig::pc_damage_delay						},
	{ "player_damage_delay_rate",			&CBattleConfig::pc_damage_delay_rate				},
	{ "player_defense_type",				&CBattleConfig::player_defense_type					},
	{ "player_invincible_time",				&CBattleConfig::pc_invincible_time					},
	{ "player_land_skill_limit",			&CBattleConfig::pc_land_skill_limit					},
	{ "player_skill_add_range",				&CBattleConfig::pc_skill_add_range					},
	{ "player_skill_log",					&CBattleConfig::pc_skill_log						},
	{ "player_skill_nofootset",				&CBattleConfig::pc_skill_nofootset					},
	{ "player_skill_partner_check",			&CBattleConfig::player_skill_partner_check			},
	{ "player_skill_reiteration",			&CBattleConfig::pc_skill_reiteration				},
	{ "player_skillfree",					&CBattleConfig::skillfree							},
	{ "player_skillup_limit",				&CBattleConfig::skillup_limit						},
	{ "potion_produce_rate",				&CBattleConfig::pp_rate								},
	{ "prevent_logout",						&CBattleConfig::prevent_logout						},
	{ "produce_item_name_input",			&CBattleConfig::produce_item_name_input				},
	{ "produce_potion_name_input",			&CBattleConfig::produce_potion_name_input			},
	{ "pvp_exp",							&CBattleConfig::pvp_exp								},
	{ "quest_skill_learn",					&CBattleConfig::quest_skill_learn					},
	{ "quest_skill_reset",					&CBattleConfig::quest_skill_reset					},
	{ "rainy_waterball",					&CBattleConfig::rainy_waterball						},
	{ "random_monster_checklv",				&CBattleConfig::random_monster_checklv				},
	{ "require_glory_guild",				&CBattleConfig::require_glory_guild					},
	{ "restart_hp_rate",					&CBattleConfig::restart_hp_rate						},
	{ "restart_sp_rate",					&CBattleConfig::restart_sp_rate						},
	{ "resurrection_exp",					&CBattleConfig::resurrection_exp					},
	{ "save_clothcolor",					&CBattleConfig::save_clothcolor						},
	{ "save_log",							&CBattleConfig::save_log							},
	{ "serverside_friendlist",				&CBattleConfig::serverside_friendlist				},
	{ "shop_exp",							&CBattleConfig::shop_exp							},
	{ "show_hp_sp_drain",					&CBattleConfig::show_hp_sp_drain					},
	{ "show_hp_sp_gain",					&CBattleConfig::show_hp_sp_gain						},
	{ "show_mob_hp",						&CBattleConfig::show_mob_hp							},
	{ "show_steal_in_same_party",			&CBattleConfig::show_steal_in_same_party			},
	{ "skill_delay_attack_enable",			&CBattleConfig::skill_delay_attack_enable			},
	{ "skill_min_damage",					&CBattleConfig::skill_min_damage					},
	{ "skill_out_range_consume",			&CBattleConfig::skill_out_range_consume				},
	{ "skill_removetrap_type",				&CBattleConfig::skill_removetrap_type				},
	{ "skill_sp_override_grffile",			&CBattleConfig::skill_sp_override_grffile			},
	{ "skill_steal_rate",					&CBattleConfig::skill_steal_rate					},
	{ "skill_steal_type",					&CBattleConfig::skill_steal_type					},
	{ "sp_rate",							&CBattleConfig::sp_rate								},
	{ "undead_detect_type",					&CBattleConfig::undead_detect_type					},
	{ "unit_movement_type",					&CBattleConfig::unit_movement_type					},
	{ "use_statpoint_table",				&CBattleConfig::use_statpoint_table					},
	{ "vending_max_value",					&CBattleConfig::vending_max_value					},
	{ "vit_penalty_count",					&CBattleConfig::vit_penalty_count					},
	{ "vit_penalty_count_lv",				&CBattleConfig::vit_penalty_count_lv				},
	{ "vit_penalty_num",					&CBattleConfig::vit_penalty_num						},
	{ "vit_penalty_type",					&CBattleConfig::vit_penalty_type					},
	{ "warp_point_debug",					&CBattleConfig::warp_point_debug					},
	{ "weapon_produce_rate",				&CBattleConfig::wp_rate								},
	{ "wedding_ignorepalette",				&CBattleConfig::wedding_ignorepalette				},
	{ "wedding_modifydisplay",				&CBattleConfig::wedding_modifydisplay				},
	{ "who_display_aid",					&CBattleConfig::who_display_aid						},
	{ "zeny_from_mobs",						&CBattleConfig::zeny_from_mobs						},
	{ "zeny_penalty",						&CBattleConfig::zeny_penalty						},
};


///////////////////////////////////////////////////////////////////////////////
/// constructor
CBattleConfig::CBattleConfig()
{
}

///////////////////////////////////////////////////////////////////////////////
/// destructor.
CBattleConfig::~CBattleConfig()
{
}

///////////////////////////////////////////////////////////////////////////////
/// set a config value by name.
/// since the namelist is sorted we actually could do a binary search...
bool CBattleConfig::set_value(const char *name, const char *value)
{
	size_t i;
	for(i=0; i< sizeof(config_map)/(sizeof(config_map[0])); ++i)
	{
		if(config_map[i].val && config_map[i].str && 0==strcasecmp(name, config_map[i].str) )
		{
			this->*(config_map[i].val) = basics::config_switch<long>(value);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// map-like access. returns a dummy value when name does not exist
/// since the namelist is sorted we actually could do a binary search...
uint32& CBattleConfig::operator[](const char *name)
{
	size_t i;
	for(i=0; i< sizeof(config_map)/(sizeof(config_map[0])); ++i)
	{
		if(config_map[i].val && config_map[i].str && 0==strcasecmp(name, config_map[i].str) )
		{
			return this->*(config_map[i].val);
		}
	}
	// name not found, return a dummy
	static uint32 dummy;
	dummy=0;
	return dummy;
}

///////////////////////////////////////////////////////////////////////////////
/// set all values to defaults
void CBattleConfig::defaults()
{
	this->agi_penalty_count = 3;
	this->agi_penalty_count_lv = ATK_FLEE;
	this->agi_penalty_num = 10;
	this->agi_penalty_type = 1;
	this->alchemist_summon_reward = 0;
	this->allow_atcommand_when_mute = 0;
	this->allow_homun_status_change = 0;
	this->any_warp_GM_min_level = 60;
	this->area_size = 14;
	this->arrow_decrement=1;
	this->atc_gmonly=0;
	this->atc_spawn_quantity_limit=0;
	this->attr_recover=1;
	this->backstab_bow_penalty = 0;
	this->ban_bot=1;
	this->ban_hack_trade=1;
	this->ban_spoof_namer = 5;
	this->base_exp_rate=100;
	this->basic_skill_check=1;
	this->battle_log = 0;
	this->berserk_cancels_buffs = 0;
	this->bone_drop = 0;
	this->boss_spawn_delay=100;	
	this->buyer_name = 1;
	this->character_size = 3; //3: Peco riders Size=2, Baby Class Riders Size=1
	this->cardillust_read_grffile=0;
	this->cast_rate=100;
	this->castle_defense_rate = 100;
	this->castrate_dex_scale = 150;
	this->chat_warpportal = 0;
	this->combo_delay_rate=100;
	this->copyskill_restrict=0;
	this->day_duration = 2*60*60*1000;
	this->dead_branch_active = 0;
	this->death_penalty_base=0;
	this->death_penalty_job=0;
	this->death_penalty_type=0;
	this->defnotenemy=0;
	this->delay_battle_damage = 1;
	this->delay_dependon_dex=0;
	this->delay_rate=100;
	this->devotion_level_difference = 10;
	this->disp_experience = 0;
	this->disp_hpmeter = 60;
	this->display_delay_skill_fail = 1;
	this->display_hallucination = 1;
	this->display_snatcher_skill_fail = 1;
	this->display_version = 1;
	this->drop_rate0item=0;
	this->drop_rare_announce=10;//show global announces for rare items drops (<= 0.1% chance)
	this->drops_by_luk = 0;
	this->dynamic_mobs = 1;
	this->enemy_critical_rate=100;
	this->enemy_critical=0;
	this->enemy_perfect_flee=0;
	this->enemy_str=1;
	this->equip_natural_break_rate = 1;
	this->equip_self_break_rate = 100;
	this->equip_skill_break_rate = 100;
	this->error_log = 1;
	this->etc_log = 1;
	this->exp_calc_type = 1;
	this->finding_ore_rate = 100;
	this->finger_offensive_type=0;
	this->flooritem_lifetime=LIFETIME_FLOORITEM*1000;
	this->gm_allequip=0;
	this->gm_allskill=0;
	this->gm_allskill_addabra=0;
	this->gm_can_drop_lv = 0;
	this->gm_join_chat=0;
	this->gm_kick_chat=0;
	this->gm_skilluncond=0;
	this->gtb_pvp_only=0;
	this->guild_emperium_check=1;
	this->guild_exp_limit=50;
	this->guild_max_castles=0;
	this->gvg_eliminate_time = 7000;
	this->gvg_long_damage_rate = 80;
	this->gvg_magic_damage_rate = 60;
	this->gvg_misc_damage_rate = 60;
	this->gvg_short_damage_rate = 100;
	this->gvg_weapon_damage_rate = 60;
	this->gx_allhit = 1;
	this->gx_cardfix = 0;
	this->gx_disptype = 1;
	this->gx_dupele = 1;
	this->hack_info_GM_level = 60;
	this->headset_block_music = 0; //Do headsets block some sound skills like Frost Joke
	this->heal_exp=0;
	this->hide_GM_session = 0;
	this->holywater_name_input = 1;
	this->homun_creation_rate = 100;
	this->homun_intimate_rate = 100;
	this->homun_temporal_intimate_resilience = 50;
	this->hp_rate = 100;
	this->idle_no_share = 0;
	this->ignore_items_gender = 1;
	this->indoors_override_grffile=0;
	this->invite_request_check = 1;
	this->item_auto_get=0;
	this->item_check=1;
	this->item_drop_card_max=10000;
	this->item_drop_card_min=1;
	this->item_drop_common_max=10000;
	this->item_drop_common_min=1;
	this->item_drop_equip_max=10000;
	this->item_drop_equip_min=1;
	this->item_drop_heal_max=10000;
	this->item_drop_heal_min=1;
	this->item_drop_mvp_max=10000;
	this->item_drop_mvp_min=1;
	this->item_drop_use_max=10000;
	this->item_drop_use_min=1;
	this->item_equip_override_grffile=0;
	this->item_first_get_time=3000;
	this->item_name_override_grffile=1;
	this->item_rate_card = 100;
	this->item_rate_common = 100;
	this->item_rate_equip = 100;
	this->item_rate_heal = 100;
	this->item_rate_use = 100;
	this->item_second_get_time=1000;
	this->item_slots_override_grffile=0;
	this->item_third_get_time=1000;
	this->item_use_interval=500;
	this->job_exp_rate=100;
	this->left_cardfix_to_right=0;
	this->magic_defense_type = 0;
	this->mailsystem=1;
	this->making_arrow_name_input = 1;
	this->master_get_homun_base_exp =0;
	this->master_get_homun_job_exp =0;
	this->max_adv_level=70;
	this->max_aspd = 199;
	this->max_aspd_interval=10;
	this->max_base_level = 99;
	this->max_cart_weight = 8000;
	this->max_cloth_color = 4;
	this->max_hair_color = 9;
	this->max_hair_style = 23;
	this->max_hitrate = 95;
	this->max_hp = 32500;
	this->max_job_level = 50;
	this->max_parameter = 99;
	this->max_sn_level = 70;
	this->max_sp = 32500;
	this->max_walk_speed = 300;
	this->maximum_level = 255;
	this->min_cloth_color = 0;
	this->min_hair_color = 0;
	this->min_hair_style = 0;
	this->min_hitrate = 5;
	this->min_skill_delay_limit = 100;
	this->mob_attack_attr_none = 1;
	this->mob_changetarget_byskill = 0;
	this->mob_clear_delay=0;
	this->mob_count_rate=100;
	this->mob_ghostring_fix = 0;
	this->mob_remove_damaged = 0;
	this->mob_remove_delay = 60000;
	this->mob_skill_add_range=0;
	this->mob_skill_delay=100;
	this->mob_skill_log = 0;
	this->mob_skill_rate=100;
	this->mob_slaves_inherit_speed=1;
	this->mob_spawn_delay=100;
	this->mob_warpportal = 0;
	this->mobs_level_up = 0;
	this->monster_active_enable=1;
	this->monster_attack_direction_change = 1;
	this->monster_auto_counter_type = 1;
	this->monster_class_change_full_recover = 0;
	this->monster_cloak_check_type = 0;
	this->monster_damage_delay = 1;
	this->monster_damage_delay_rate=100;
	this->monster_defense_type = 0;
	this->monster_hp_rate=100;
	this->monster_land_skill_limit = 1;
	this->monster_loot_type=0;
	this->monster_max_aspd=199;
	this->monster_skill_nofootset = 0;
	this->monster_skill_reiteration = 0;
	this->monsters_ignore_gm=0;
	this->motd_type = 0;
	this->multi_level_up = 0;
	this->muting_players=0;
	this->mvp_exp_rate=100;
	this->mvp_hp_rate=100;
	this->mvp_item_first_get_time=10000;
	this->mvp_item_rate=100;
	this->mvp_item_second_get_time=10000;
	this->mvp_item_third_get_time=2000;
	this->natural_heal_skill_interval=10000;
	this->natural_heal_weight_rate=50;
	this->natural_healhp_interval=6000;
	this->natural_healsp_interval=8000;
	this->night_at_start = 0;
	this->night_darkness_level = 9;
	this->night_duration = 30*60*1000; 
	this->packet_ver_flag = 0; 
	this->party_bonus = 0;
	this->party_share_mode = 2; // 0 exclude none, 1 exclude idle, 2 exclude idle+chatting
	this->party_skill_penalty = 1;
	this->pc_attack_attr_none = 0;
	this->pc_attack_direction_change = 1;
	this->pc_auto_counter_type = 1;
	this->pc_cloak_check_type = 0;
	this->pc_damage_delay_rate=100;
	this->pc_damage_delay=1;
	this->pc_invincible_time = 5000;
	this->pc_land_skill_limit = 1;
	this->pc_skill_add_range=0;
	this->pc_skill_log = 1;
	this->pc_skill_nofootset = 0;
	this->pc_skill_reiteration = 0;
	this->pet_attack_attr_none = 0;
	this->pet_attack_exp_rate=100;
	this->pet_attack_exp_to_master=0;
	this->pet_attack_support=0;
	this->pet_catch_rate=100;
	this->pet_damage_support=0;
	this->pet_defense_type = 0;
	this->pet_equip_required = 0;
	this->pet_friendly_rate=100;
	this->pet_hair_style = 100;
	this->pet_hungry_delay_rate=100;
	this->pet_hungry_friendly_decrease=5;
	this->pet_lv_rate=0;
	this->pet_max_atk1=750;
	this->pet_max_atk2=1000;
	this->pet_max_stats=99;
	this->pet_no_gvg = 0;
	this->pet_random_move=1;
	this->pet_rename=0;
	this->pet_status_support=0;
	this->pet_str=1;
	this->pet_support_min_friendly=900;
	this->pet_support_rate=100;
	this->pk_min_level = 55;
	this->pk_mode = 0;
	this->plant_spawn_delay=100;
	this->player_defense_type = 0;
	this->player_skill_partner_check = 1;
	this->pp_rate=100;
	this->prevent_logout = 1;
	this->produce_item_name_input = 1;
	this->produce_potion_name_input = 1;
	this->pvp_exp=1;
	this->quest_skill_learn=0;
	this->quest_skill_reset=1;
	this->rainy_waterball = 1;
	this->random_monster_checklv=1;
	this->require_glory_guild = 0;
	this->restart_hp_rate=0;
	this->restart_sp_rate=0;
	this->resurrection_exp=0;
	this->save_clothcolor = 0;
	this->save_log = 0;
	this->serverside_friendlist=1;
	this->shop_exp=100;
	this->show_hp_sp_drain = 0;
	this->show_hp_sp_gain = 1;
	this->show_mob_hp = 0;
	this->show_steal_in_same_party = 0;
	this->skill_delay_attack_enable=0;
	this->skill_min_damage=0;
	this->skill_out_range_consume=1;
	this->skill_removetrap_type = 0;
	this->skill_sp_override_grffile=0;
	this->skill_steal_rate = 100;
	this->skill_steal_type = 1;
	this->skillfree = 0;
	this->skillup_limit = 0;
	this->sp_rate = 100;
	this->undead_detect_type = 0;
	this->unit_movement_type = 0;
	this->use_statpoint_table = 1;
	this->vending_max_value = 10000000;
	this->vit_penalty_count = 3;
	this->vit_penalty_count_lv = ATK_DEF;
	this->vit_penalty_num = 5;
	this->vit_penalty_type = 1;
	this->warp_point_debug=0;
	this->wedding_ignorepalette=0;
	this->wedding_modifydisplay=0;
	this->who_display_aid = 0;
	this->wp_rate=100;
	this->zeny_from_mobs = 0;
	this->zeny_penalty=0;
}

///////////////////////////////////////////////////////////////////////////////
/// validate all values
void CBattleConfig::validate()
{
	if(this->flooritem_lifetime < 1000)
		this->flooritem_lifetime = LIFETIME_FLOORITEM*1000;
	if(this->restart_hp_rate > 100)
		this->restart_hp_rate = 100;
	if(this->restart_sp_rate > 100)
		this->restart_sp_rate = 100;
	if(this->natural_healhp_interval < NATURAL_HEAL_INTERVAL)
		this->natural_healhp_interval=NATURAL_HEAL_INTERVAL;
	if(this->natural_healsp_interval < NATURAL_HEAL_INTERVAL)
		this->natural_healsp_interval=NATURAL_HEAL_INTERVAL;
	if(this->natural_heal_skill_interval < NATURAL_HEAL_INTERVAL)
		this->natural_heal_skill_interval=NATURAL_HEAL_INTERVAL;
	if(this->natural_heal_weight_rate < 50)
		this->natural_heal_weight_rate = 50;
	if(this->natural_heal_weight_rate > 100)
		this->natural_heal_weight_rate = 100;
	
	////////////////////////////////////////////////
	if( this->monster_max_aspd< 200 )
		this->monster_max_aspd_interval = 2000 - this->monster_max_aspd*10;
	else
		this->monster_max_aspd_interval= 10;
	if(this->monster_max_aspd_interval > 1000)
		this->monster_max_aspd_interval = 1000;
	////////////////////////////////////////////////
	if(this->max_aspd>199)
		this->max_aspd_interval = 10;
	else if(this->max_aspd<100)
		this->max_aspd_interval = 1000;
	else
		this->max_aspd_interval = 2000 - this->max_aspd*10;
	////////////////////////////////////////////////
	if(this->max_walk_speed > MAX_WALK_SPEED)
		this->max_walk_speed = MAX_WALK_SPEED;


	if(this->hp_rate < 1)
		this->hp_rate = 1;
	if(this->sp_rate < 1)
		this->sp_rate = 1;
	if(this->max_hp > 1000000)
		this->max_hp = 1000000;
	if(this->max_hp < 100)
		this->max_hp = 100;
	if(this->max_sp > 1000000)
		this->max_sp = 1000000;
	if(this->max_sp < 100)
		this->max_sp = 100;
	if(this->max_parameter < 10)
		this->max_parameter = 10;
	if(this->max_parameter > 10000)
		this->max_parameter = 10000;
	if(this->max_cart_weight > 1000000)
		this->max_cart_weight = 1000000;
	if(this->max_cart_weight < 100)
		this->max_cart_weight = 100;
	this->max_cart_weight *= 10;

	if(this->min_hitrate > this->max_hitrate)
		this->min_hitrate = this->max_hitrate;
		
	if(this->agi_penalty_count < 2)
		this->agi_penalty_count = 2;
	if(this->vit_penalty_count < 2)
		this->vit_penalty_count = 2;

	if(this->guild_exp_limit > 99)
		this->guild_exp_limit = 99;

	if(this->pet_support_min_friendly > 950)
		this->pet_support_min_friendly = 950;
	
	if(this->pet_max_atk1 > this->pet_max_atk2)
		this->pet_max_atk1 = this->pet_max_atk2;
	
	if(this->castle_defense_rate > 100)
		this->castle_defense_rate = 100;
	if(this->item_drop_common_min < 1)	
		this->item_drop_common_min = 1;
	if(this->item_drop_common_max > 10000)
		this->item_drop_common_max = 10000;
	if(this->item_drop_equip_min < 1)
		this->item_drop_equip_min = 1;
	if(this->item_drop_equip_max > 10000)
		this->item_drop_equip_max = 10000;
	if(this->item_drop_card_min < 1)
		this->item_drop_card_min = 1;
	if(this->item_drop_card_max > 10000)
		this->item_drop_card_max = 10000;
	if(this->item_drop_mvp_min < 1)
		this->item_drop_mvp_min = 1;
	if(this->item_drop_mvp_max > 10000)
		this->item_drop_mvp_max = 10000;


	if (this->night_at_start > 1)
		this->night_at_start = 1;
	if (this->day_duration != 0 && this->day_duration < 60000)
		this->day_duration = 60000;
	if (this->night_duration != 0 && this->night_duration < 60000)
		this->night_duration = 60000;
	

	if (this->ban_spoof_namer > 32767)
		this->ban_spoof_namer = 32767;


	if (this->hack_info_GM_level > 100)
		this->hack_info_GM_level = 100;


	if (this->any_warp_GM_min_level > 100)
		this->any_warp_GM_min_level = 100;


	if (this->night_darkness_level > 10)
		this->night_darkness_level = 10;

	if (this->motd_type > 1)
		this->motd_type = 1;

	if (this->vending_max_value > MAX_ZENY || this->vending_max_value<=0)
		this->vending_max_value = MAX_ZENY;

	if (this->min_skill_delay_limit < 10)
		this->min_skill_delay_limit = 10;	// minimum delay of 10ms

	if (this->mob_remove_delay < 15000)	//Min 15 sec
		this->mob_remove_delay = 15000;
	if (this->dynamic_mobs > 1)
		this->dynamic_mobs = 1;	//The flag will be used in assignations	
}

///////////////////////////////////////////////////////////////////////////////
/// read in a configuration file
bool CBattleConfig::read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int count = 0;

	if( count == 0)
		this->defaults();
	++count;
	fp = basics::safefopen(cfgName,"r");
	if (fp == NULL)
	{
		ShowError("Configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
	}
	else
	{
		while(fgets(line,sizeof(line),fp))
		{
			if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
			{
				basics::itrim(w1);
				if(!*w1) continue;
				basics::itrim(w2);
				
				if(strcasecmp(w1, "import") == 0)
					this->read(w2);
				else
				{
					if( !this->set_value(w1, w2) )
						ShowWarning("'"CL_WHITE"%s"CL_RESET"': no option '%s'.\n", cfgName, w1);
				}
			}
		}
		fclose(fp);
		ShowStatus("Done reading Configuration '"CL_WHITE"%s"CL_RESET"'.\n", cfgName);
		if( --count == 0 )
		{
			this->validate();
		}
		return true;
	}
	return false;
}







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// server messages





///////////////////////////////////////////////////////////////////////////////
/// constructor.
CMessageTable::CMessageTable()
{
	memset(msg_table, 0, sizeof(msg_table));
}

///////////////////////////////////////////////////////////////////////////////
/// destructor.
CMessageTable::~CMessageTable()
{
	size_t i;
	for (i=0; i<MAX_MSG; ++i)
	{
		if(msg_table[i])
		{
			delete[] msg_table[i];
			msg_table[i] = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// return the message string of the specified number
const char *CMessageTable::operator()(msg_t msg_number)
{
	if( (int)msg_number < MAX_MSG && this->msg_table[msg_number] && *this->msg_table[msg_number] )
		return msg_table[msg_number];
	return "??";
}


///////////////////////////////////////////////////////////////////////////////
/// read config file
bool CMessageTable::read(const char *cfgName)
{
	size_t msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if((fp = basics::safefopen(cfgName, "r")) == NULL)
	{
		ShowError("Messages Configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
	}
	else
	{
		while(fgets(line, sizeof(line), fp))
		{
			if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
			{
				basics::itrim(w1);
				if(!*w1) continue;
				basics::itrim(w2);

				if(strcasecmp(w1, "import") == 0)
				{
					this->read(w2);
				}
				else
				{
					msg_number = atoi(w1);
					if(msg_number < MAX_MSG)
					{
						if (msg_table[msg_number] != NULL)
							delete[] msg_table[msg_number];
						msg_table[msg_number] = new char[(1+strlen(w2))];
						memcpy(msg_table[msg_number],w2,1+strlen(w2));
					}
				}
			}
		}
		fclose(fp);
		ShowStatus("Done reading Message Configuration '"CL_WHITE"%s"CL_RESET"'.\n", cfgName);
		return true;
	}
	return false;
}
