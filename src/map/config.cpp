#include "config.h"
#include "showmsg.h"
#include "utils.h"
#include "map.h"


///////////////////////////////////////////////////////////////////////////////
/// instanciation of the configration
CConfig config;
CMessageTable msg_txt;


///////////////////////////////////////////////////////////////////////////////
/// internal name->value map
CConfig::_config_map CConfig::config_map[] =
{
	{ "agi_penalty_count",					&CConfig::agi_penalty_count						},
	{ "agi_penalty_count_lv",				&CConfig::agi_penalty_count_lv					},
	{ "agi_penalty_num",					&CConfig::agi_penalty_num						},
	{ "agi_penalty_type",					&CConfig::agi_penalty_type						},
	{ "alchemist_summon_reward",			&CConfig::alchemist_summon_reward				},
	{ "allow_atcommand_when_mute",			&CConfig::allow_atcommand_when_mute				},
	{ "allow_homun_status_change",			&CConfig::allow_homun_status_change				},
	{ "any_warp_GM_min_level",				&CConfig::any_warp_GM_min_level					},
	{ "area_size",							&CConfig::area_size								},
	{ "arrow_decrement",					&CConfig::arrow_decrement						},
	{ "atcommand_gm_only",					&CConfig::atc_gmonly							},
	{ "atcommand_spawn_quantity_limit",		&CConfig::atc_spawn_quantity_limit				},
	{ "attribute_recover",					&CConfig::attr_recover							},
	{ "backstab_bow_penalty",				&CConfig::backstab_bow_penalty					},
	{ "ban_bot",							&CConfig::ban_bot								},
	{ "ban_spoof_namer",					&CConfig::ban_spoof_namer						},
	{ "base_exp_rate",						&CConfig::base_exp_rate							},
	{ "basic_skill_check",					&CConfig::basic_skill_check						},
	{ "battle_log",							&CConfig::battle_log							},
	{ "berserk_canc	els_buffs",				&CConfig::berserk_cancels_buffs					},
	{ "bone_drop",							&CConfig::bone_drop								},
	{ "boss_spawn_delay",					&CConfig::boss_spawn_delay						},
	{ "buyer_name",							&CConfig::buyer_name							},
	{ "cardillust_read_grffile",			&CConfig::cardillust_read_grffile				},
	{ "casting_rate",						&CConfig::cast_rate								},
	{ "castle_defense_rate",				&CConfig::castle_defense_rate					},
	{ "castrate_dex_scale",					&CConfig::castrate_dex_scale					},
	{ "character_size",						&CConfig::character_size						},
	{ "chat_warpportal",					&CConfig::chat_warpportal						},
	{ "combo_delay_rate",					&CConfig::combo_delay_rate						},
	{ "copyskill_restrict",					&CConfig::copyskill_restrict					},
	{ "day_duration",						&CConfig::day_duration							},
	{ "dead_branch_active",					&CConfig::dead_branch_active					},
	{ "death_penalty_base",					&CConfig::death_penalty_base					},
	{ "death_penalty_job",					&CConfig::death_penalty_job						},
	{ "death_penalty_type",					&CConfig::death_penalty_type					},
	{ "defunit_not_enemy",					&CConfig::defnotenemy							},
	{ "delay_battle_damage",				&CConfig::delay_battle_damage					},
	{ "delay_dependon_dex",					&CConfig::delay_dependon_dex					},
	{ "delay_rate",							&CConfig::delay_rate							},
	{ "devotion_level_difference",			&CConfig::devotion_level_difference				},
	{ "disp_experience",					&CConfig::disp_experience						},
	{ "disp_hpmeter",						&CConfig::disp_hpmeter							},
	{ "display_delay_skill_fail",			&CConfig::display_delay_skill_fail				},
	{ "display_hallucination",				&CConfig::display_hallucination					},
	{ "display_snatcher_skill_fail",		&CConfig::display_snatcher_skill_fail			},
	{ "display_version",					&CConfig::display_version						},
	{ "drop_rate0item",						&CConfig::drop_rate0item						},
	{ "drop_rare_announce",					&CConfig::drop_rare_announce					},
	{ "drops_by_luk",						&CConfig::drops_by_luk							},
	{ "dynamic_mobs",						&CConfig::dynamic_mobs							},
	{ "enemy_critical",						&CConfig::enemy_critical						},
	{ "enemy_critical_rate",				&CConfig::enemy_critical_rate					},
	{ "enemy_perfect_flee",					&CConfig::enemy_perfect_flee					},
	{ "enemy_str",							&CConfig::enemy_str								},
	{ "equip_natural_break_rate",			&CConfig::equip_natural_break_rate				},
	{ "equip_self_break_rate",				&CConfig::equip_self_break_rate					},
	{ "equip_skill_break_rate",				&CConfig::equip_skill_break_rate				},
	{ "error_log",							&CConfig::error_log								},
	{ "etc_log",							&CConfig::etc_log								},
	{ "exp_calc_type",						&CConfig::exp_calc_type							},
	{ "finding_ore_rate",					&CConfig::finding_ore_rate						},
	{ "finger_offensive_type",				&CConfig::finger_offensive_type					},
	{ "flooritem_lifetime",					&CConfig::flooritem_lifetime					},
	{ "gm_all_equipment",					&CConfig::gm_allequip							},
	{ "gm_all_skill",						&CConfig::gm_allskill							},
	{ "gm_all_skill_add_abra",				&CConfig::gm_allskill_addabra					},
	{ "gm_can_drop_lv",						&CConfig::gm_can_drop_lv						},
	{ "gm_join_chat",						&CConfig::gm_join_chat							},
	{ "gm_kick_chat",						&CConfig::gm_kick_chat							},
	{ "gm_skill_unconditional",				&CConfig::gm_skilluncond						},
	{ "gtb_pvp_only",						&CConfig::gtb_pvp_only							},
	{ "guild_emperium_check",				&CConfig::guild_emperium_check					},
	{ "guild_exp_limit",					&CConfig::guild_exp_limit						},
	{ "guild_max_castles",					&CConfig::guild_max_castles						},
	{ "gvg_eliminate_time",					&CConfig::gvg_eliminate_time					},
	{ "gvg_long_attack_damage_rate",		&CConfig::gvg_long_damage_rate					},
	{ "gvg_magic_attack_damage_rate",		&CConfig::gvg_magic_damage_rate					},
	{ "gvg_misc_attack_damage_rate",		&CConfig::gvg_misc_damage_rate					},
	{ "gvg_short_attack_damage_rate",		&CConfig::gvg_short_damage_rate					},
	{ "gvg_weapon_damage_rate",				&CConfig::gvg_weapon_damage_rate				},
	{ "gx_allhit",							&CConfig::gx_allhit								},
	{ "gx_cardfix",							&CConfig::gx_cardfix							},
	{ "gx_disptype",						&CConfig::gx_disptype							},
	{ "gx_dupele",							&CConfig::gx_dupele								},
	{ "hack_info_GM_level",					&CConfig::hack_info_GM_level					},
	{ "headset_block_music",				&CConfig::headset_block_music					},
	{ "heal_exp",							&CConfig::heal_exp								},
	{ "hide_GM_session",					&CConfig::hide_GM_session						},
	{ "holywater_name_input",				&CConfig::holywater_name_input					},
	{ "homun_creation_rate",				&CConfig::homun_creation_rate					},
	{ "homun_intimate_rate",				&CConfig::homun_intimate_rate					},
	{ "homun_temporal_intimate_resilience",	&CConfig::homun_temporal_intimate_resilience	},
	{ "hp_rate",							&CConfig::hp_rate								},
	{ "idle_no_share",						&CConfig::idle_no_share							},
	{ "ignore_items_gender",				&CConfig::ignore_items_gender					},
	{ "indoors_override_grffile",			&CConfig::indoors_override_grffile				},
	{ "invite_request_check",				&CConfig::invite_request_check					},
	{ "item_auto_get",						&CConfig::item_auto_get							},
	{ "item_check",							&CConfig::item_check							},
	{ "item_drop_card_max",					&CConfig::item_drop_card_max					},
	{ "item_drop_card_min",					&CConfig::item_drop_card_min					},
	{ "item_drop_common_max",				&CConfig::item_drop_common_max					},
	{ "item_drop_common_min",				&CConfig::item_drop_common_min					},
	{ "item_drop_equip_max",				&CConfig::item_drop_equip_max					},
	{ "item_drop_equip_min",				&CConfig::item_drop_equip_min					},
	{ "item_drop_heal_max",					&CConfig::item_drop_heal_max					},
	{ "item_drop_heal_min",					&CConfig::item_drop_heal_min					},
	{ "item_drop_mvp_max",					&CConfig::item_drop_mvp_max						},
	{ "item_drop_mvp_min",					&CConfig::item_drop_mvp_min						},
	{ "item_drop_use_max",					&CConfig::item_drop_use_max						},
	{ "item_drop_use_min",					&CConfig::item_drop_use_min						},
	{ "item_equip_override_grffile",		&CConfig::item_equip_override_grffile			},
	{ "item_first_get_time",				&CConfig::item_first_get_time					},
	{ "item_name_override_grffile",			&CConfig::item_name_override_grffile			},
	{ "item_rate_card",						&CConfig::item_rate_card						},
	{ "item_rate_common",					&CConfig::item_rate_common						},
	{ "item_rate_equip",					&CConfig::item_rate_equip						},
	{ "item_rate_heal",						&CConfig::item_rate_heal						},
	{ "item_rate_use",						&CConfig::item_rate_use							},
	{ "item_second_get_time",				&CConfig::item_second_get_time					},
	{ "item_slots_override_grffile",		&CConfig::item_slots_override_grffile			},
	{ "item_third_get_time",				&CConfig::item_third_get_time					},
	{ "item_use_interval",					&CConfig::item_use_interval						},
	{ "job_exp_rate",						&CConfig::job_exp_rate							},
	{ "left_cardfix_to_right",				&CConfig::left_cardfix_to_right					},
	{ "magic_defense_type",					&CConfig::magic_defense_type					},
	{ "mailsystem",							&CConfig::mailsystem							},
	{ "making_arrow_name_input",			&CConfig::making_arrow_name_input				},
	{ "master_get_homun_base_exp",			&CConfig::master_get_homun_base_exp				},
	{ "master_get_homun_job_exp",			&CConfig::master_get_homun_job_exp				},
	{ "max_adv_level",						&CConfig::max_adv_level							},
	{ "max_aspd",							&CConfig::max_aspd								},
	{ "max_base_level",						&CConfig::max_base_level						},
	{ "max_cart_weight",					&CConfig::max_cart_weight						},
	{ "max_cloth_color",					&CConfig::max_cloth_color						},
	{ "max_hair_color",						&CConfig::max_hair_color						},
	{ "max_hair_style",						&CConfig::max_hair_style						},
	{ "max_hitrate",						&CConfig::max_hitrate							},
	{ "max_hp",								&CConfig::max_hp								},
	{ "max_job_level",						&CConfig::max_job_level							},
	{ "max_parameter",						&CConfig::max_parameter							},
	{ "max_sn_level",						&CConfig::max_sn_level							},
	{ "max_sp",								&CConfig::max_sp								},
	{ "max_walk_speed",						&CConfig::max_walk_speed						},
	{ "maximum_level",						&CConfig::maximum_level							},
	{ "min_cloth_color",					&CConfig::min_cloth_color						},
	{ "min_hair_color",						&CConfig::min_hair_color						},
	{ "min_hair_style",						&CConfig::min_hair_style						},
	{ "min_hitrate",						&CConfig::min_hitrate							},
	{ "min_skill_delay_limit",				&CConfig::min_skill_delay_limit					},
	{ "mob_attack_attr_none",				&CConfig::mob_attack_attr_none					},
	{ "mob_changetarget_byskill",			&CConfig::mob_changetarget_byskill				},
	{ "mob_clear_delay",					&CConfig::mob_clear_delay						},
	{ "mob_count_rate",						&CConfig::mob_count_rate						},
	{ "mob_ghostring_fix",					&CConfig::mob_ghostring_fix						},
	{ "mob_remove_damaged",					&CConfig::mob_remove_damaged					},
	{ "mob_remove_delay",					&CConfig::mob_remove_delay						},
	{ "mob_skill_delay",					&CConfig::mob_skill_delay						},
	{ "mob_skill_rate",						&CConfig::mob_skill_rate						},
	{ "mob_slaves_inherit_speed",			&CConfig::mob_slaves_inherit_speed				},
	{ "mob_spawn_delay",					&CConfig::mob_spawn_delay						},
	{ "mob_warpportal",						&CConfig::mob_warpportal						},
	{ "mobs_level_up",						&CConfig::mobs_level_up							},
	{ "monster_active_enable",				&CConfig::monster_active_enable					},
	{ "monster_attack_direction_change",	&CConfig::monster_attack_direction_change		},
	{ "monster_auto_counter_type",			&CConfig::monster_auto_counter_type				},
	{ "monster_class_change_full_recover",	&CConfig::monster_class_change_full_recover 	},
	{ "monster_cloak_check_type",			&CConfig::monster_cloak_check_type				},
	{ "monster_damage_delay",				&CConfig::monster_damage_delay					},
	{ "monster_damage_delay_rate",			&CConfig::monster_damage_delay_rate				},
	{ "monster_defense_type",				&CConfig::monster_defense_type					},
	{ "monster_hp_rate",					&CConfig::monster_hp_rate						},
	{ "monster_land_skill_limit",			&CConfig::monster_land_skill_limit				},
	{ "monster_loot_type",					&CConfig::monster_loot_type						},
	{ "monster_max_aspd",					&CConfig::monster_max_aspd						},
	{ "monster_skill_add_range",			&CConfig::mob_skill_add_range					},
	{ "monster_skill_log",					&CConfig::mob_skill_log							},
	{ "monster_skill_nofootset",			&CConfig::monster_skill_nofootset				},
	{ "monster_skill_reiteration",			&CConfig::monster_skill_reiteration				},
	{ "monsters_ignore_gm",					&CConfig::monsters_ignore_gm					},
	{ "motd_type",							&CConfig::motd_type								},
	{ "multi_level_up",						&CConfig::multi_level_up						},
	{ "muting_players",						&CConfig::muting_players						},
	{ "mvp_exp_rate",						&CConfig::mvp_exp_rate							},
	{ "mvp_hp_rate",						&CConfig::mvp_hp_rate							},
	{ "mvp_item_first_get_time",			&CConfig::mvp_item_first_get_time				},
	{ "mvp_item_rate",						&CConfig::mvp_item_rate							},
	{ "mvp_item_second_get_time",			&CConfig::mvp_item_second_get_time				},
	{ "mvp_item_third_get_time",			&CConfig::mvp_item_third_get_time				},
	{ "natural_heal_skill_interval",		&CConfig::natural_heal_skill_interval			},
	{ "natural_heal_weight_rate",			&CConfig::natural_heal_weight_rate				},
	{ "natural_healhp_interval",			&CConfig::natural_healhp_interval				},
	{ "natural_healsp_interval",			&CConfig::natural_healsp_interval				},
	{ "night_at_start",						&CConfig::night_at_start						},
	{ "night_darkness_level",				&CConfig::night_darkness_level					},
	{ "night_duration",						&CConfig::night_duration						},
	{ "packet_ver_flag",					&CConfig::packet_ver_flag						},
	{ "party_bonus",						&CConfig::party_bonus							},
	{ "party_share_mode",					&CConfig::party_share_mode						},
	{ "party_skill_penalty",				&CConfig::party_skill_penalty					},
	{ "pc_attack_attr_none",				&CConfig::pc_attack_attr_none					},
	{ "pet_attack_attr_none",				&CConfig::pet_attack_attr_none					},
	{ "pet_attack_exp_rate",				&CConfig::pet_attack_exp_rate					},
	{ "pet_attack_exp_to_master",			&CConfig::pet_attack_exp_to_master				},
	{ "pet_attack_support",					&CConfig::pet_attack_support					},
	{ "pet_catch_rate",						&CConfig::pet_catch_rate						},
	{ "pet_damage_support",					&CConfig::pet_damage_support					},
	{ "pet_defense_type",					&CConfig::pet_defense_type						},
	{ "pet_equip_required",					&CConfig::pet_equip_required					},
	{ "pet_friendly_rate",					&CConfig::pet_friendly_rate						},
	{ "pet_hair_style",						&CConfig::pet_hair_style						},
	{ "pet_hungry_delay_rate",				&CConfig::pet_hungry_delay_rate					},
	{ "pet_hungry_friendly_decrease",		&CConfig::pet_hungry_friendly_decrease			},
	{ "pet_lv_rate",						&CConfig::pet_lv_rate							},
	{ "pet_max_atk1",						&CConfig::pet_max_atk1							},
	{ "pet_max_atk2",						&CConfig::pet_max_atk2							},
	{ "pet_max_stats",						&CConfig::pet_max_stats							},
	{ "pet_no_gvg",							&CConfig::pet_no_gvg							},
	{ "pet_random_move",					&CConfig::pet_random_move						},
	{ "pet_rename",							&CConfig::pet_rename							},
	{ "pet_status_support",					&CConfig::pet_status_support					},
	{ "pet_str",							&CConfig::pet_str								},
	{ "pet_support_min_friendly",			&CConfig::pet_support_min_friendly				},
	{ "pet_support_rate",					&CConfig::pet_support_rate						},
	{ "pk_min_level",						&CConfig::pk_min_level							},
	{ "pk_mode",							&CConfig::pk_mode								},
	{ "plant_spawn_delay",					&CConfig::plant_spawn_delay						},
	{ "player_attack_direction_change",		&CConfig::pc_attack_direction_change			},
	{ "player_auto_counter_type",			&CConfig::pc_auto_counter_type					},
	{ "player_cloak_check_type",			&CConfig::pc_cloak_check_type					},
	{ "player_damage_delay",				&CConfig::pc_damage_delay						},
	{ "player_damage_delay_rate",			&CConfig::pc_damage_delay_rate					},
	{ "player_defense_type",				&CConfig::player_defense_type					},
	{ "player_invincible_time",				&CConfig::pc_invincible_time					},
	{ "player_land_skill_limit",			&CConfig::pc_land_skill_limit					},
	{ "player_skill_add_range",				&CConfig::pc_skill_add_range					},
	{ "player_skill_log",					&CConfig::pc_skill_log							},
	{ "player_skill_nofootset",				&CConfig::pc_skill_nofootset					},
	{ "player_skill_partner_check",			&CConfig::player_skill_partner_check			},
	{ "player_skill_reiteration",			&CConfig::pc_skill_reiteration					},
	{ "player_skillfree",					&CConfig::skillfree								},
	{ "player_skillup_limit",				&CConfig::skillup_limit							},
	{ "potion_produce_rate",				&CConfig::pp_rate								},
	{ "prevent_logout",						&CConfig::prevent_logout						},
	{ "produce_item_name_input",			&CConfig::produce_item_name_input				},
	{ "produce_potion_name_input",			&CConfig::produce_potion_name_input				},
	{ "pvp_exp",							&CConfig::pvp_exp								},
	{ "quest_skill_learn",					&CConfig::quest_skill_learn						},
	{ "quest_skill_reset",					&CConfig::quest_skill_reset						},
	{ "rainy_waterball",					&CConfig::rainy_waterball						},
	{ "random_monster_checklv",				&CConfig::random_monster_checklv				},
	{ "require_glory_guild",				&CConfig::require_glory_guild					},
	{ "restart_hp_rate",					&CConfig::restart_hp_rate						},
	{ "restart_sp_rate",					&CConfig::restart_sp_rate						},
	{ "resurrection_exp",					&CConfig::resurrection_exp						},
	{ "save_clothcolor",					&CConfig::save_clothcolor						},
	{ "save_log",							&CConfig::save_log								},
	{ "serverside_friendlist",				&CConfig::serverside_friendlist					},
	{ "shop_exp",							&CConfig::shop_exp								},
	{ "show_hp_sp_drain",					&CConfig::show_hp_sp_drain						},
	{ "show_hp_sp_gain",					&CConfig::show_hp_sp_gain						},
	{ "show_mob_hp",						&CConfig::show_mob_hp							},
	{ "show_steal_in_same_party",			&CConfig::show_steal_in_same_party				},
	{ "skill_delay_attack_enable",			&CConfig::skill_delay_attack_enable				},
	{ "skill_min_damage",					&CConfig::skill_min_damage						},
	{ "skill_out_range_consume",			&CConfig::skill_out_range_consume				},
	{ "skill_removetrap_type",				&CConfig::skill_removetrap_type					},
	{ "skill_sp_override_grffile",			&CConfig::skill_sp_override_grffile				},
	{ "skill_steal_rate",					&CConfig::skill_steal_rate						},
	{ "skill_steal_type",					&CConfig::skill_steal_type						},
	{ "sp_rate",							&CConfig::sp_rate								},
	{ "undead_detect_type",					&CConfig::undead_detect_type					},
	{ "unit_movement_type",					&CConfig::unit_movement_type					},
	{ "use_statpoint_table",				&CConfig::use_statpoint_table					},
	{ "vending_max_value",					&CConfig::vending_max_value						},
	{ "vit_penalty_count",					&CConfig::vit_penalty_count						},
	{ "vit_penalty_count_lv",				&CConfig::vit_penalty_count_lv					},
	{ "vit_penalty_num",					&CConfig::vit_penalty_num						},
	{ "vit_penalty_type",					&CConfig::vit_penalty_type						},
	{ "warp_point_debug",					&CConfig::warp_point_debug						},
	{ "weapon_produce_rate",				&CConfig::wp_rate								},
	{ "wedding_ignorepalette",				&CConfig::wedding_ignorepalette					},
	{ "wedding_modifydisplay",				&CConfig::wedding_modifydisplay					},
	{ "who_display_aid",					&CConfig::who_display_aid						},
	{ "zeny_from_mobs",						&CConfig::zeny_from_mobs						},
	{ "zeny_penalty",						&CConfig::zeny_penalty							},
};


///////////////////////////////////////////////////////////////////////////////
/// constructor
CConfig::CConfig()
{
}

///////////////////////////////////////////////////////////////////////////////
/// destructor.
CConfig::~CConfig()
{
}

///////////////////////////////////////////////////////////////////////////////
/// set a config value by name.
/// since the namelist is sorted we actually could do a binary search...
bool CConfig::set_value(const char *name, const char *value)
{
	size_t i;
	for(i=0; i< sizeof(config_map)/(sizeof(config_map[0])); ++i)
	{
		if(config_map[i].val && config_map[i].str && 0==strcasecmp(name, config_map[i].str) )
		{
			this->*(config_map[i].val) = config_switch(value);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// map-like access. returns a dummy value when name does not exist
/// since the namelist is sorted we actually could do a binary search...
uint32& CConfig::operator[](const char *name)
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
void CConfig::defaults()
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
void CConfig::validate()
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
bool CConfig::read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int count = 0;

	if ((count++) == 0)
		this->defaults();

	fp = basics::safefopen(cfgName,"r");
	if (fp == NULL)
	{
		ShowError("Configuration file not found: %s\n", cfgName);
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
						ShowWarning("(Config) %s: no such option.\n", w1);
				}
			}
		}
		fclose(fp);
		if( --count == 0 )
		{
			this->validate();
		}
		ShowStatus("Configuration file '%s' read.\n", cfgName);
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
const char *CMessageTable::operator()(size_t msg_number)
{
	if( msg_number < MAX_MSG && this->msg_table[msg_number] && *this->msg_table[msg_number] )
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
		ShowError("Messages file not found: %s\n", cfgName);
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
		ShowStatus("Messages configuration file '%s' read.\n", cfgName);
		return true;
	}
	return false;
}
