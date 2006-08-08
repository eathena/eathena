#include "showmsg.h"
#include "utils.h"
#include "config.h"
#include "map.h"

struct CConfig config;

static struct _config_map
{
	const char *str;
	uint32 *val;
}
config_map[] =
{
	{ "agi_penalty_count",					&config.agi_penalty_count					},
	{ "agi_penalty_count_lv",				&config.agi_penalty_count_lv				},
	{ "agi_penalty_num",					&config.agi_penalty_num						},
	{ "agi_penalty_type",					&config.agi_penalty_type					},
	{ "alchemist_summon_reward",			&config.alchemist_summon_reward				},	
	{ "allow_atcommand_when_mute",			&config.allow_atcommand_when_mute			},
	{ "allow_homun_status_change",			&config.allow_homun_status_change			},
	{ "any_warp_GM_min_level",				&config.any_warp_GM_min_level				},
	{ "area_size",							&config.area_size							},
	{ "arrow_decrement",					&config.arrow_decrement						},
	{ "atcommand_gm_only",					&config.atc_gmonly							},
	{ "atcommand_spawn_quantity_limit",		&config.atc_spawn_quantity_limit			},
	{ "attribute_recover",					&config.attr_recover						},
	{ "backstab_bow_penalty",				&config.backstab_bow_penalty				},
	{ "ban_bot",							&config.ban_bot								},	
	{ "ban_spoof_namer",					&config.ban_spoof_namer						},
	{ "base_exp_rate",						&config.base_exp_rate						},
	{ "basic_skill_check",					&config.basic_skill_check					},
	{ "battle_log",							&config.battle_log							},
	{ "berserk_candels_buffs",				&config.berserk_cancels_buffs				},
	{ "bone_drop",							&config.bone_drop							},
	{ "boss_spawn_delay",					&config.boss_spawn_delay					},
	{ "buyer_name",							&config.buyer_name							},
	{ "cardillust_read_grffile",			&config.cardillust_read_grffile				},	
	{ "casting_rate",						&config.cast_rate							},
	{ "castle_defense_rate",				&config.castle_defense_rate					},
	{ "castrate_dex_scale",					&config.castrate_dex_scale					},
	{ "character_size",						&config.character_size						},
	{ "chat_warpportal",					&config.chat_warpportal						},
	{ "combo_delay_rate",					&config.combo_delay_rate					},
	{ "copyskill_restrict",					&config.copyskill_restrict					},
	{ "day_duration",						&config.day_duration						},
	{ "dead_branch_active",					&config.dead_branch_active					},
	{ "death_penalty_base",					&config.death_penalty_base					},
	{ "death_penalty_job",					&config.death_penalty_job					},
	{ "death_penalty_type",					&config.death_penalty_type					},
	{ "defunit_not_enemy",					&config.defnotenemy							},
	{ "delay_battle_damage",				&config.delay_battle_damage					},
	{ "delay_dependon_dex",					&config.delay_dependon_dex					},
	{ "delay_rate",							&config.delay_rate							},
	{ "devotion_level_difference",			&config.devotion_level_difference			},
	{ "disp_experience",					&config.disp_experience						},
	{ "disp_hpmeter",						&config.disp_hpmeter						},
	{ "display_delay_skill_fail",			&config.display_delay_skill_fail			},
	{ "display_hallucination",				&config.display_hallucination				},
	{ "display_snatcher_skill_fail",		&config.display_snatcher_skill_fail			},
	{ "display_version",					&config.display_version						},
	{ "drop_rate0item",						&config.drop_rate0item						},
	{ "drop_rare_announce",					&config.drop_rare_announce					},
	{ "drops_by_luk",						&config.drops_by_luk						},	
	{ "dynamic_mobs",						&config.dynamic_mobs						},
	{ "enemy_critical",						&config.enemy_critical						},
	{ "enemy_critical_rate",				&config.enemy_critical_rate					},
	{ "enemy_perfect_flee",					&config.enemy_perfect_flee					},
	{ "enemy_str",							&config.enemy_str							},
	{ "equip_natural_break_rate",			&config.equip_natural_break_rate			},
	{ "equip_self_break_rate",				&config.equip_self_break_rate				},
	{ "equip_skill_break_rate",				&config.equip_skill_break_rate				},
	{ "error_log",							&config.error_log							},
	{ "etc_log",							&config.etc_log								},
	{ "exp_calc_type",						&config.exp_calc_type						},
	{ "finding_ore_rate",					&config.finding_ore_rate					},
	{ "finger_offensive_type",				&config.finger_offensive_type				},
	{ "flooritem_lifetime",					&config.flooritem_lifetime					},
	{ "gm_all_equipment",					&config.gm_allequip							},
	{ "gm_all_skill",						&config.gm_allskill							},
	{ "gm_all_skill_add_abra",				&config.gm_allskill_addabra					},
	{ "gm_can_drop_lv",						&config.gm_can_drop_lv						},
	{ "gm_join_chat",						&config.gm_join_chat						},
	{ "gm_kick_chat",						&config.gm_kick_chat						},
	{ "gm_skill_unconditional",				&config.gm_skilluncond						},
	{ "gtb_pvp_only",						&config.gtb_pvp_only						},
	{ "guild_emperium_check",				&config.guild_emperium_check				},
	{ "guild_exp_limit",					&config.guild_exp_limit						},
	{ "guild_max_castles",					&config.guild_max_castles					},
	{ "gvg_eliminate_time",					&config.gvg_eliminate_time					},
	{ "gvg_long_attack_damage_rate",		&config.gvg_long_damage_rate				},
	{ "gvg_magic_attack_damage_rate",		&config.gvg_magic_damage_rate				},
	{ "gvg_misc_attack_damage_rate",		&config.gvg_misc_damage_rate				},
	{ "gvg_short_attack_damage_rate",		&config.gvg_short_damage_rate				},
	{ "gvg_weapon_damage_rate",				&config.gvg_weapon_damage_rate				},
	{ "gx_allhit",							&config.gx_allhit							},
	{ "gx_cardfix",							&config.gx_cardfix							},
	{ "gx_disptype",						&config.gx_disptype							},
	{ "gx_dupele",							&config.gx_dupele							},
	{ "hack_info_GM_level",					&config.hack_info_GM_level					},
	{ "headset_block_music",				&config.headset_block_music					},
	{ "heal_exp",							&config.heal_exp							},
	{ "hide_GM_session",					&config.hide_GM_session						},
	{ "holywater_name_input",				&config.holywater_name_input				},
	{ "homun_creation_rate",				&config.homun_creation_rate					},
	{ "homun_intimate_rate",				&config.homun_intimate_rate					},
	{ "homun_temporal_intimate_resilience",	&config.homun_temporal_intimate_resilience	},
	{ "hp_rate",							&config.hp_rate								},
	{ "idle_no_share",						&config.idle_no_share						},
	{ "ignore_items_gender",				&config.ignore_items_gender					},
	{ "indoors_override_grffile",			&config.indoors_override_grffile			},	
	{ "invite_request_check",				&config.invite_request_check				},
	{ "item_auto_get",						&config.item_auto_get						},
	{ "item_check",							&config.item_check							},
	{ "item_drop_card_max",					&config.item_drop_card_max					},
	{ "item_drop_card_min",					&config.item_drop_card_min					},
	{ "item_drop_common_max",				&config.item_drop_common_max				},
	{ "item_drop_common_min",				&config.item_drop_common_min				},	
	{ "item_drop_equip_max",				&config.item_drop_equip_max					},
	{ "item_drop_equip_min",				&config.item_drop_equip_min					},
	{ "item_drop_heal_max",					&config.item_drop_heal_max					},
	{ "item_drop_heal_min",					&config.item_drop_heal_min					},
	{ "item_drop_mvp_max",					&config.item_drop_mvp_max					},	
	{ "item_drop_mvp_min",					&config.item_drop_mvp_min					},
	{ "item_drop_use_max",					&config.item_drop_use_max					},
	{ "item_drop_use_min",					&config.item_drop_use_min					},
	{ "item_equip_override_grffile",		&config.item_equip_override_grffile			},	
	{ "item_first_get_time",				&config.item_first_get_time					},
	{ "item_name_override_grffile",			&config.item_name_override_grffile			},
	{ "item_rate_card",						&config.item_rate_card						},	
	{ "item_rate_common",					&config.item_rate_common					},	
	{ "item_rate_equip",					&config.item_rate_equip						},
	{ "item_rate_heal",						&config.item_rate_heal						},	
	{ "item_rate_use",						&config.item_rate_use						},	
	{ "item_second_get_time",				&config.item_second_get_time				},
	{ "item_slots_override_grffile",		&config.item_slots_override_grffile			},	
	{ "item_third_get_time",				&config.item_third_get_time					},
	{ "item_use_interval",					&config.item_use_interval					},
	{ "job_exp_rate",						&config.job_exp_rate						},
	{ "left_cardfix_to_right",				&config.left_cardfix_to_right				},
	{ "magic_defense_type",					&config.magic_defense_type					},
	{ "mailsystem",							&config.mailsystem							},
	{ "making_arrow_name_input",			&config.making_arrow_name_input				},
	{ "master_get_homun_base_exp",			&config.master_get_homun_base_exp			},
	{ "master_get_homun_job_exp",			&config.master_get_homun_job_exp			},
	{ "max_adv_level",						&config.max_adv_level						},
	{ "max_aspd",							&config.max_aspd							},
	{ "max_base_level",						&config.max_base_level						},
	{ "max_cart_weight",					&config.max_cart_weight						},
	{ "max_cloth_color",					&config.max_cloth_color						},
	{ "max_hair_color",						&config.max_hair_color						},
	{ "max_hair_style",						&config.max_hair_style						},
	{ "max_hitrate",						&config.max_hitrate							},
	{ "max_hp",								&config.max_hp								},
	{ "max_job_level",						&config.max_job_level						},
	{ "max_parameter",						&config.max_parameter						},
	{ "max_sn_level",						&config.max_sn_level						},
	{ "max_sp",								&config.max_sp								},
	{ "max_walk_speed",						&config.max_walk_speed						},
	{ "maximum_level",						&config.maximum_level						},	
	{ "min_cloth_color",					&config.min_cloth_color						},
	{ "min_hair_color",						&config.min_hair_color						},
	{ "min_hair_style",						&config.min_hair_style						},
	{ "min_hitrate",						&config.min_hitrate							},
	{ "min_skill_delay_limit",				&config.min_skill_delay_limit				},
	{ "mob_attack_attr_none",				&config.mob_attack_attr_none				},
	{ "mob_changetarget_byskill",			&config.mob_changetarget_byskill			},
	{ "mob_clear_delay",					&config.mob_clear_delay						},
	{ "mob_count_rate",						&config.mob_count_rate						},
	{ "mob_ghostring_fix",					&config.mob_ghostring_fix					},
	{ "mob_remove_damaged",					&config.mob_remove_damaged					},
	{ "mob_remove_delay",					&config.mob_remove_delay					},
	{ "mob_skill_delay",					&config.mob_skill_delay						},
	{ "mob_skill_rate",						&config.mob_skill_rate						},
	{ "mob_slaves_inherit_speed",			&config.mob_slaves_inherit_speed			},
	{ "mob_spawn_delay",					&config.mob_spawn_delay						},
	{ "mob_warpportal",						&config.mob_warpportal						},
	{ "mobs_level_up",						&config.mobs_level_up						},
	{ "monster_active_enable",				&config.monster_active_enable				},
	{ "monster_attack_direction_change",	&config.monster_attack_direction_change		},
	{ "monster_auto_counter_type",			&config.monster_auto_counter_type			},
	{ "monster_class_change_full_recover",	&config.monster_class_change_full_recover 	},
	{ "monster_cloak_check_type",			&config.monster_cloak_check_type			},
	{ "monster_damage_delay",				&config.monster_damage_delay				},
	{ "monster_damage_delay_rate",			&config.monster_damage_delay_rate			},
	{ "monster_defense_type",				&config.monster_defense_type				},
	{ "monster_hp_rate",					&config.monster_hp_rate						},
	{ "monster_land_skill_limit",			&config.monster_land_skill_limit			},
	{ "monster_loot_type",					&config.monster_loot_type					},
	{ "monster_max_aspd",					&config.monster_max_aspd					},
	{ "monster_skill_add_range",			&config.mob_skill_add_range					},
	{ "monster_skill_log",					&config.mob_skill_log						},
	{ "monster_skill_nofootset",			&config.monster_skill_nofootset				},
	{ "monster_skill_reiteration",			&config.monster_skill_reiteration			},
	{ "monsters_ignore_gm",					&config.monsters_ignore_gm					},	
	{ "motd_type",							&config.motd_type							},
	{ "multi_level_up",						&config.multi_level_up						},
	{ "muting_players",						&config.muting_players						},
	{ "mvp_exp_rate",						&config.mvp_exp_rate						},
	{ "mvp_hp_rate",						&config.mvp_hp_rate							},
	{ "mvp_item_first_get_time",			&config.mvp_item_first_get_time				},
	{ "mvp_item_rate",						&config.mvp_item_rate						},
	{ "mvp_item_second_get_time",			&config.mvp_item_second_get_time			},
	{ "mvp_item_third_get_time",			&config.mvp_item_third_get_time				},
	{ "natural_heal_skill_interval",		&config.natural_heal_skill_interval			},
	{ "natural_heal_weight_rate",			&config.natural_heal_weight_rate			},
	{ "natural_healhp_interval",			&config.natural_healhp_interval				},
	{ "natural_healsp_interval",			&config.natural_healsp_interval				},
	{ "night_at_start",						&config.night_at_start						},
	{ "night_darkness_level",				&config.night_darkness_level				},
	{ "night_duration",						&config.night_duration						},
	{ "packet_ver_flag",					&config.packet_ver_flag						},
	{ "party_bonus",						&config.party_bonus							},
	{ "party_share_mode",					&config.party_share_mode					},
	{ "party_skill_penalty",				&config.party_skill_penalty					},
	{ "pc_attack_attr_none",				&config.pc_attack_attr_none					},
	{ "pet_attack_attr_none",				&config.pet_attack_attr_none				},
	{ "pet_attack_exp_rate",				&config.pet_attack_exp_rate					},
	{ "pet_attack_exp_to_master",			&config.pet_attack_exp_to_master			},
	{ "pet_attack_support",					&config.pet_attack_support					},
	{ "pet_catch_rate",						&config.pet_catch_rate						},
	{ "pet_damage_support",					&config.pet_damage_support					},
	{ "pet_defense_type",					&config.pet_defense_type					},
	{ "pet_equip_required",					&config.pet_equip_required					},	
	{ "pet_friendly_rate",					&config.pet_friendly_rate					},
	{ "pet_hair_style",						&config.pet_hair_style						},
	{ "pet_hungry_delay_rate",				&config.pet_hungry_delay_rate				},
	{ "pet_hungry_friendly_decrease",		&config.pet_hungry_friendly_decrease		},
	{ "pet_lv_rate",						&config.pet_lv_rate							},	
	{ "pet_max_atk1",						&config.pet_max_atk1						},	
	{ "pet_max_atk2",						&config.pet_max_atk2						},	
	{ "pet_max_stats",						&config.pet_max_stats						},	
	{ "pet_no_gvg",							&config.pet_no_gvg							},	
	{ "pet_random_move",					&config.pet_random_move						},
	{ "pet_rename",							&config.pet_rename							},
	{ "pet_status_support",					&config.pet_status_support					},
	{ "pet_str",							&config.pet_str								},
	{ "pet_support_min_friendly",			&config.pet_support_min_friendly			},
	{ "pet_support_rate",					&config.pet_support_rate					},
	{ "pk_min_level",						&config.pk_min_level						},
	{ "pk_mode",							&config.pk_mode								},	
	{ "plant_spawn_delay",					&config.plant_spawn_delay					},
	{ "player_attack_direction_change",		&config.pc_attack_direction_change			},
	{ "player_auto_counter_type",			&config.pc_auto_counter_type				},
	{ "player_cloak_check_type",			&config.pc_cloak_check_type					},
	{ "player_damage_delay",				&config.pc_damage_delay						},
	{ "player_damage_delay_rate",			&config.pc_damage_delay_rate				},
	{ "player_defense_type",				&config.player_defense_type					},
	{ "player_invincible_time",				&config.pc_invincible_time					},
	{ "player_land_skill_limit",			&config.pc_land_skill_limit					},
	{ "player_skill_add_range",				&config.pc_skill_add_range					},
	{ "player_skill_log",					&config.pc_skill_log						},
	{ "player_skill_nofootset",				&config.pc_skill_nofootset					},
	{ "player_skill_partner_check",			&config.player_skill_partner_check			},
	{ "player_skill_reiteration",			&config.pc_skill_reiteration				},
	{ "player_skillfree",					&config.skillfree							},
	{ "player_skillup_limit",				&config.skillup_limit						},
	{ "potion_produce_rate",				&config.pp_rate								},
	{ "prevent_logout",						&config.prevent_logout						},	
	{ "produce_item_name_input",			&config.produce_item_name_input				},
	{ "produce_potion_name_input",			&config.produce_potion_name_input			},
	{ "pvp_exp",							&config.pvp_exp								},
	{ "quest_skill_learn",					&config.quest_skill_learn					},
	{ "quest_skill_reset",					&config.quest_skill_reset					},
	{ "rainy_waterball",					&config.rainy_waterball						},
	{ "random_monster_checklv",				&config.random_monster_checklv				},
	{ "require_glory_guild",				&config.require_glory_guild					},
	{ "restart_hp_rate",					&config.restart_hp_rate						},
	{ "restart_sp_rate",					&config.restart_sp_rate						},
	{ "resurrection_exp",					&config.resurrection_exp					},
	{ "save_clothcolor",					&config.save_clothcolor						},
	{ "save_log",							&config.save_log							},
	{ "serverside_friendlist",				&config.serverside_friendlist				},
	{ "shop_exp",							&config.shop_exp							},
	{ "show_hp_sp_drain",					&config.show_hp_sp_drain					},
	{ "show_hp_sp_gain",					&config.show_hp_sp_gain						},
	{ "show_mob_hp",						&config.show_mob_hp							},
	{ "show_steal_in_same_party",			&config.show_steal_in_same_party			},
	{ "skill_delay_attack_enable",			&config.skill_delay_attack_enable			},
	{ "skill_min_damage",					&config.skill_min_damage					},
	{ "skill_out_range_consume",			&config.skill_out_range_consume				},
	{ "skill_removetrap_type",				&config.skill_removetrap_type				},
	{ "skill_sp_override_grffile",			&config.skill_sp_override_grffile			},	
	{ "skill_steal_rate",					&config.skill_steal_rate					},
	{ "skill_steal_type",					&config.skill_steal_type					},
	{ "sp_rate",							&config.sp_rate								},
	{ "undead_detect_type",					&config.undead_detect_type					},
	{ "unit_movement_type",					&config.unit_movement_type					},
	{ "use_statpoint_table",				&config.use_statpoint_table					},
	{ "vending_max_value",					&config.vending_max_value					},
	{ "vit_penalty_count",					&config.vit_penalty_count					},
	{ "vit_penalty_count_lv",				&config.vit_penalty_count_lv				},
	{ "vit_penalty_num",					&config.vit_penalty_num						},
	{ "vit_penalty_type",					&config.vit_penalty_type					},
	{ "warp_point_debug",					&config.warp_point_debug					},
	{ "weapon_produce_rate",				&config.wp_rate								},
	{ "wedding_ignorepalette",				&config.wedding_ignorepalette				},
	{ "wedding_modifydisplay",				&config.wedding_modifydisplay				},
	{ "who_display_aid",					&config.who_display_aid						},
	{ "zeny_from_mobs",						&config.zeny_from_mobs						},
	{ "zeny_penalty",						&config.zeny_penalty						},
};




bool config_set_value(const char *w1, const char *w2)
{
	size_t i;
	for(i=0; i< sizeof(config_map)/(sizeof(config_map[0])); ++i)
	{
		if(config_map[i].val && config_map[i].str && 0==strcasecmp(w1, config_map[i].str) )
		{
			*(config_map[i].val) = config_switch(w2);
			return true;
		}
	}
	return false;
}

void config_defaults()
{
	config.agi_penalty_count = 3;
	config.agi_penalty_count_lv = ATK_FLEE;
	config.agi_penalty_num = 10;
	config.agi_penalty_type = 1;
	config.alchemist_summon_reward = 0;
	config.allow_atcommand_when_mute = 0;
	config.allow_homun_status_change = 0;
	config.any_warp_GM_min_level = 60;
	config.area_size = 14;
	config.arrow_decrement=1;
	config.atc_gmonly=0;
	config.atc_spawn_quantity_limit=0;
	config.attr_recover=1;
	config.backstab_bow_penalty = 0;
	config.ban_bot=1;
	config.ban_hack_trade=1;
	config.ban_spoof_namer = 5;
	config.base_exp_rate=100;
	config.basic_skill_check=1;
	config.battle_log = 0;
	config.berserk_cancels_buffs = 0;
	config.bone_drop = 0;
	config.boss_spawn_delay=100;	
	config.buyer_name = 1;
	config.character_size = 3; //3: Peco riders Size=2, Baby Class Riders Size=1
	config.cardillust_read_grffile=0;
	config.cast_rate=100;
	config.castle_defense_rate = 100;
	config.castrate_dex_scale = 150;
	config.chat_warpportal = 0;
	config.combo_delay_rate=100;
	config.copyskill_restrict=0;
	config.day_duration = 2*60*60*1000;
	config.dead_branch_active = 0;
	config.death_penalty_base=0;
	config.death_penalty_job=0;
	config.death_penalty_type=0;
	config.defnotenemy=0;
	config.delay_battle_damage = 1;
	config.delay_dependon_dex=0;
	config.delay_rate=100;
	config.devotion_level_difference = 10;
	config.disp_experience = 0;
	config.disp_hpmeter = 60;
	config.display_delay_skill_fail = 1;
	config.display_hallucination = 1;
	config.display_snatcher_skill_fail = 1;
	config.display_version = 1;
	config.drop_rate0item=0;
	config.drop_rare_announce=10;//show global announces for rare items drops (<= 0.1% chance)
	config.drops_by_luk = 0;
	config.dynamic_mobs = 1;
	config.enemy_critical_rate=100;
	config.enemy_critical=0;
	config.enemy_perfect_flee=0;
	config.enemy_str=1;
	config.equip_natural_break_rate = 1;
	config.equip_self_break_rate = 100;
	config.equip_skill_break_rate = 100;
	config.error_log = 1;
	config.etc_log = 1;
	config.exp_calc_type = 1;
	config.finding_ore_rate = 100;
	config.finger_offensive_type=0;
	config.flooritem_lifetime=LIFETIME_FLOORITEM*1000;
	config.gm_allequip=0;
	config.gm_allskill=0;
	config.gm_allskill_addabra=0;
	config.gm_can_drop_lv = 0;
	config.gm_join_chat=0;
	config.gm_kick_chat=0;
	config.gm_skilluncond=0;
	config.gtb_pvp_only=0;
	config.guild_emperium_check=1;
	config.guild_exp_limit=50;
	config.guild_max_castles=0;
	config.gvg_eliminate_time = 7000;
	config.gvg_long_damage_rate = 80;
	config.gvg_magic_damage_rate = 60;
	config.gvg_misc_damage_rate = 60;
	config.gvg_short_damage_rate = 100;
	config.gvg_weapon_damage_rate = 60;
	config.gx_allhit = 1;
	config.gx_cardfix = 0;
	config.gx_disptype = 1;
	config.gx_dupele = 1;
	config.hack_info_GM_level = 60;
	config.headset_block_music = 0; //Do headsets block some sound skills like Frost Joke
	config.heal_exp=0;
	config.hide_GM_session = 0;
	config.holywater_name_input = 1;
	config.homun_creation_rate = 100;
	config.homun_intimate_rate = 100;
	config.homun_temporal_intimate_resilience = 50;
	config.hp_rate = 100;
	config.idle_no_share = 0;
	config.ignore_items_gender = 1;
	config.indoors_override_grffile=0;
	config.invite_request_check = 1;
	config.item_auto_get=0;
	config.item_check=1;
	config.item_drop_card_max=10000;
	config.item_drop_card_min=1;
	config.item_drop_common_max=10000;
	config.item_drop_common_min=1;
	config.item_drop_equip_max=10000;
	config.item_drop_equip_min=1;
	config.item_drop_heal_max=10000;
	config.item_drop_heal_min=1;
	config.item_drop_mvp_max=10000;
	config.item_drop_mvp_min=1;
	config.item_drop_use_max=10000;
	config.item_drop_use_min=1;
	config.item_equip_override_grffile=0;
	config.item_first_get_time=3000;
	config.item_name_override_grffile=1;
	config.item_rate_card = 100;
	config.item_rate_common = 100;
	config.item_rate_equip = 100;
	config.item_rate_heal = 100;
	config.item_rate_use = 100;
	config.item_second_get_time=1000;
	config.item_slots_override_grffile=0;
	config.item_third_get_time=1000;
	config.item_use_interval=500;
	config.job_exp_rate=100;
	config.left_cardfix_to_right=0;
	config.magic_defense_type = 0;
	config.mailsystem=1;
	config.making_arrow_name_input = 1;
	config.master_get_homun_base_exp =0;
	config.master_get_homun_job_exp =0;
	config.max_adv_level=70;
	config.max_aspd = 199;
	config.max_aspd_interval=10;
	config.max_base_level = 99;
	config.max_cart_weight = 8000;
	config.max_cloth_color = 4;
	config.max_hair_color = 9;
	config.max_hair_style = 23;
	config.max_hitrate = 95;
	config.max_hp = 32500;
	config.max_job_level = 50;
	config.max_parameter = 99;
	config.max_sn_level = 70;
	config.max_sp = 32500;
	config.max_walk_speed = 300;
	config.maximum_level = 255;
	config.min_cloth_color = 0;
	config.min_hair_color = 0;
	config.min_hair_style = 0;
	config.min_hitrate = 5;
	config.min_skill_delay_limit = 100;
	config.mob_attack_attr_none = 1;
	config.mob_changetarget_byskill = 0;
	config.mob_clear_delay=0;
	config.mob_count_rate=100;
	config.mob_ghostring_fix = 0;
	config.mob_remove_damaged = 0;
	config.mob_remove_delay = 60000;
	config.mob_skill_add_range=0;
	config.mob_skill_delay=100;
	config.mob_skill_log = 0;
	config.mob_skill_rate=100;
	config.mob_slaves_inherit_speed=1;
	config.mob_spawn_delay=100;
	config.mob_warpportal = 0;
	config.mobs_level_up = 0;
	config.monster_active_enable=1;
	config.monster_attack_direction_change = 1;
	config.monster_auto_counter_type = 1;
	config.monster_class_change_full_recover = 0;
	config.monster_cloak_check_type = 0;
	config.monster_damage_delay = 1;
	config.monster_damage_delay_rate=100;
	config.monster_defense_type = 0;
	config.monster_hp_rate=100;
	config.monster_land_skill_limit = 1;
	config.monster_loot_type=0;
	config.monster_max_aspd=199;
	config.monster_skill_nofootset = 0;
	config.monster_skill_reiteration = 0;
	config.monsters_ignore_gm=0;
	config.motd_type = 0;
	config.multi_level_up = 0;
	config.muting_players=0;
	config.mvp_exp_rate=100;
	config.mvp_hp_rate=100;
	config.mvp_item_first_get_time=10000;
	config.mvp_item_rate=100;
	config.mvp_item_second_get_time=10000;
	config.mvp_item_third_get_time=2000;
	config.natural_heal_skill_interval=10000;
	config.natural_heal_weight_rate=50;
	config.natural_healhp_interval=6000;
	config.natural_healsp_interval=8000;
	config.night_at_start = 0;
	config.night_darkness_level = 9;
	config.night_duration = 30*60*1000; 
	config.packet_ver_flag = 0; 
	config.party_bonus = 0;
	config.party_share_mode = 2; // 0 exclude none, 1 exclude idle, 2 exclude idle+chatting
	config.party_skill_penalty = 1;
	config.pc_attack_attr_none = 0;
	config.pc_attack_direction_change = 1;
	config.pc_auto_counter_type = 1;
	config.pc_cloak_check_type = 0;
	config.pc_damage_delay_rate=100;
	config.pc_damage_delay=1;
	config.pc_invincible_time = 5000;
	config.pc_land_skill_limit = 1;
	config.pc_skill_add_range=0;
	config.pc_skill_log = 1;
	config.pc_skill_nofootset = 0;
	config.pc_skill_reiteration = 0;
	config.pet_attack_attr_none = 0;
	config.pet_attack_exp_rate=100;
	config.pet_attack_exp_to_master=0;
	config.pet_attack_support=0;
	config.pet_catch_rate=100;
	config.pet_damage_support=0;
	config.pet_defense_type = 0;
	config.pet_equip_required = 0;
	config.pet_friendly_rate=100;
	config.pet_hair_style = 100;
	config.pet_hungry_delay_rate=100;
	config.pet_hungry_friendly_decrease=5;
	config.pet_lv_rate=0;
	config.pet_max_atk1=750;
	config.pet_max_atk2=1000;
	config.pet_max_stats=99;
	config.pet_no_gvg = 0;
	config.pet_random_move=1;
	config.pet_rename=0;
	config.pet_status_support=0;
	config.pet_str=1;
	config.pet_support_min_friendly=900;
	config.pet_support_rate=100;
	config.pk_min_level = 55;
	config.pk_mode = 0;
	config.plant_spawn_delay=100;
	config.player_defense_type = 0;
	config.player_skill_partner_check = 1;
	config.pp_rate=100;
	config.prevent_logout = 1;
	config.produce_item_name_input = 1;
	config.produce_potion_name_input = 1;
	config.pvp_exp=1;
	config.quest_skill_learn=0;
	config.quest_skill_reset=1;
	config.rainy_waterball = 1;
	config.random_monster_checklv=1;
	config.require_glory_guild = 0;
	config.restart_hp_rate=0;
	config.restart_sp_rate=0;
	config.resurrection_exp=0;
	config.save_clothcolor = 0;
	config.save_log = 0;
	config.serverside_friendlist=1;
	config.shop_exp=100;
	config.show_hp_sp_drain = 0;
	config.show_hp_sp_gain = 1;
	config.show_mob_hp = 0;
	config.show_steal_in_same_party = 0;
	config.skill_delay_attack_enable=0;
	config.skill_min_damage=0;
	config.skill_out_range_consume=1;
	config.skill_removetrap_type = 0;
	config.skill_sp_override_grffile=0;
	config.skill_steal_rate = 100;
	config.skill_steal_type = 1;
	config.skillfree = 0;
	config.skillup_limit = 0;
	config.sp_rate = 100;
	config.undead_detect_type = 0;
	config.unit_movement_type = 0;
	config.use_statpoint_table = 1;
	config.vending_max_value = 10000000;
	config.vit_penalty_count = 3;
	config.vit_penalty_count_lv = ATK_DEF;
	config.vit_penalty_num = 5;
	config.vit_penalty_type = 1;
	config.warp_point_debug=0;
	config.wedding_ignorepalette=0;
	config.wedding_modifydisplay=0;
	config.who_display_aid = 0;
	config.wp_rate=100;
	config.zeny_from_mobs = 0;
	config.zeny_penalty=0;
}

void config_validate()
{
	if(config.flooritem_lifetime < 1000)
		config.flooritem_lifetime = LIFETIME_FLOORITEM*1000;
	if(config.restart_hp_rate > 100)
		config.restart_hp_rate = 100;
	if(config.restart_sp_rate > 100)
		config.restart_sp_rate = 100;
	if(config.natural_healhp_interval < NATURAL_HEAL_INTERVAL)
		config.natural_healhp_interval=NATURAL_HEAL_INTERVAL;
	if(config.natural_healsp_interval < NATURAL_HEAL_INTERVAL)
		config.natural_healsp_interval=NATURAL_HEAL_INTERVAL;
	if(config.natural_heal_skill_interval < NATURAL_HEAL_INTERVAL)
		config.natural_heal_skill_interval=NATURAL_HEAL_INTERVAL;
	if(config.natural_heal_weight_rate < 50)
		config.natural_heal_weight_rate = 50;
	if(config.natural_heal_weight_rate > 100)
		config.natural_heal_weight_rate = 100;
	
	////////////////////////////////////////////////
	if( config.monster_max_aspd< 200 )
		config.monster_max_aspd_interval = 2000 - config.monster_max_aspd*10;
	else
		config.monster_max_aspd_interval= 10;
	if(config.monster_max_aspd_interval > 1000)
		config.monster_max_aspd_interval = 1000;
	////////////////////////////////////////////////
	if(config.max_aspd>199)
		config.max_aspd_interval = 10;
	else if(config.max_aspd<100)
		config.max_aspd_interval = 1000;
	else
		config.max_aspd_interval = 2000 - config.max_aspd*10;
	////////////////////////////////////////////////
	if(config.max_walk_speed > MAX_WALK_SPEED)
		config.max_walk_speed = MAX_WALK_SPEED;


	if(config.hp_rate < 1)
		config.hp_rate = 1;
	if(config.sp_rate < 1)
		config.sp_rate = 1;
	if(config.max_hp > 1000000)
		config.max_hp = 1000000;
	if(config.max_hp < 100)
		config.max_hp = 100;
	if(config.max_sp > 1000000)
		config.max_sp = 1000000;
	if(config.max_sp < 100)
		config.max_sp = 100;
	if(config.max_parameter < 10)
		config.max_parameter = 10;
	if(config.max_parameter > 10000)
		config.max_parameter = 10000;
	if(config.max_cart_weight > 1000000)
		config.max_cart_weight = 1000000;
	if(config.max_cart_weight < 100)
		config.max_cart_weight = 100;
	config.max_cart_weight *= 10;

	if(config.min_hitrate > config.max_hitrate)
		config.min_hitrate = config.max_hitrate;
		
	if(config.agi_penalty_count < 2)
		config.agi_penalty_count = 2;
	if(config.vit_penalty_count < 2)
		config.vit_penalty_count = 2;

	if(config.guild_exp_limit > 99)
		config.guild_exp_limit = 99;

	if(config.pet_support_min_friendly > 950)
		config.pet_support_min_friendly = 950;
	
	if(config.pet_max_atk1 > config.pet_max_atk2)
		config.pet_max_atk1 = config.pet_max_atk2;
	
	if(config.castle_defense_rate > 100)
		config.castle_defense_rate = 100;
	if(config.item_drop_common_min < 1)	
		config.item_drop_common_min = 1;
	if(config.item_drop_common_max > 10000)
		config.item_drop_common_max = 10000;
	if(config.item_drop_equip_min < 1)
		config.item_drop_equip_min = 1;
	if(config.item_drop_equip_max > 10000)
		config.item_drop_equip_max = 10000;
	if(config.item_drop_card_min < 1)
		config.item_drop_card_min = 1;
	if(config.item_drop_card_max > 10000)
		config.item_drop_card_max = 10000;
	if(config.item_drop_mvp_min < 1)
		config.item_drop_mvp_min = 1;
	if(config.item_drop_mvp_max > 10000)
		config.item_drop_mvp_max = 10000;


	if (config.night_at_start > 1)
		config.night_at_start = 1;
	if (config.day_duration != 0 && config.day_duration < 60000)
		config.day_duration = 60000;
	if (config.night_duration != 0 && config.night_duration < 60000)
		config.night_duration = 60000;
	

	if (config.ban_spoof_namer > 32767)
		config.ban_spoof_namer = 32767;


	if (config.hack_info_GM_level > 100)
		config.hack_info_GM_level = 100;


	if (config.any_warp_GM_min_level > 100)
		config.any_warp_GM_min_level = 100;


	if (config.night_darkness_level > 10)
		config.night_darkness_level = 10;

	if (config.motd_type > 1)
		config.motd_type = 1;

	if (config.vending_max_value > MAX_ZENY || config.vending_max_value<=0)
		config.vending_max_value = MAX_ZENY;

	if (config.min_skill_delay_limit < 10)
		config.min_skill_delay_limit = 10;	// minimum delay of 10ms

	if (config.mob_remove_delay < 15000)	//Min 15 sec
		config.mob_remove_delay = 15000;
	if (config.dynamic_mobs > 1)
		config.dynamic_mobs = 1;	//The flag will be used in assignations	
}

/*==========================================
 * ê›íËÉtÉ@ÉCÉãÇì«Ç›çûÇﬁ
 *------------------------------------------
 */
bool config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int count = 0;

	if ((count++) == 0)
		config_defaults();

	fp = basics::safefopen(cfgName,"r");
	if (fp == NULL)
	{
		ShowError("file not found: %s\n", cfgName);
	}
	else
	{
		while(fgets(line,sizeof(line),fp))
		{
			if( !prepare_line(line) )
				continue;
			if( sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) != 2 )
				continue;
			basics::itrim(w1);
			basics::itrim(w2);
			if(strcasecmp(w1, "import") == 0)
				config_read(w2);
			else
			{
				if( !config_set_value(w1, w2) )
					ShowWarning("(Config) %s: no such option.\n", w1);
			}
		}
		fclose(fp);
		if( --count == 0 )
		{
			config_validate();
		}
		return true;
	}
	return false;
}
