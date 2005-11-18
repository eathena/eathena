#ifndef _STATUS_H_
#define _STATUS_H_

#include "map.h"

// Status changes listing. These code are for use by the server. 
enum {
	//First we enumerate common status ailments which are often used around.
	SC_STONE = 0,
	SC_FREEZE,
	SC_STAN,
	SC_SLEEP,
	SC_POISON,
	SC_CURSE,
	SC_SILENCE,
	SC_CONFUSION,
	SC_BLIND,
	SC_BLEEDING,
	SC_DPOISON, //10
	
	//Next up, we continue on 20, to leave enough room for additional "common" ailments in the future.
	SC_PROVOKE = 20,
	SC_ENDURE,
	SC_TWOHANDQUICKEN,
	SC_CONCENTRATE,
	SC_HIDING,
	SC_CLOAKING,
	SC_ENCPOISON,
	SC_POISONREACT,
	SC_QUAGMIRE,
	SC_ANGELUS,
	SC_BLESSING,
	SC_SIGNUMCRUCIS,
	SC_INCREASEAGI,
	SC_DECREASEAGI,
	SC_SLOWPOISON,
	SC_IMPOSITIO  ,
	SC_SUFFRAGIUM,
	SC_ASPERSIO,
	SC_BENEDICTIO,
	SC_KYRIE,
	SC_MAGNIFICAT,
	SC_GLORIA,
	SC_AETERNA,
	SC_ADRENALINE,
	SC_WEAPONPERFECTION,
	SC_OVERTHRUST,
	SC_MAXIMIZEPOWER,
	SC_RIDING, //DEPRECATED. We don't remove it yet to avoid screwing up with saved sc_data [Skotlex]
	SC_FALCON, //DEPRECATED. We don't remove it yet to avoid screwing up with saved sc_data [Skotlex]
	SC_TRICKDEAD,
	SC_LOUD,
	SC_ENERGYCOAT,
	SC_BROKENARMOR,
	SC_BROKENWEAPON,
	SC_HALLUCINATION,
	SC_WEIGHT50 ,
	SC_WEIGHT90,
	SC_ASPDPOTION0,
	SC_ASPDPOTION1,
	SC_ASPDPOTION2,
	SC_ASPDPOTION3,
	SC_SPEEDUP0,
	SC_SPEEDUP1,
	SC_ATKPOTION,
	SC_MATKPOTION,
	SC_WEDDING,
	SC_SLOWDOWN,
	SC_ANKLE,
	SC_KEEPING,
	SC_BARRIER,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR,
	SC_STRIPHELM,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	SC_AUTOGUARD,
	SC_REFLECTSHIELD,
	SC_SPLASHER,
	SC_PROVIDENCE,
	SC_DEFENDER,
	SC_MAGICROD,
	SC_SPELLBREAKER,
	SC_AUTOSPELL,
	SC_SIGHTTRASHER,
	SC_AUTOBERSERK,
	SC_SPEARSQUICKEN,
	SC_AUTOCOUNTER,
	SC_SIGHT,
	SC_SAFETYWALL,
	SC_RUWACH,
	SC_PNEUMA, //DEPRECATED: Pneuma now uses ground tiles. [Skotlex]
	SC_EXTREMITYFIST,
	SC_EXPLOSIONSPIRITS,
	SC_COMBO,
	SC_BLADESTOP_WAIT,
	SC_BLADESTOP,
	SC_FIREWEAPON,
	SC_WATERWEAPON,
	SC_WINDWEAPON,
	SC_EARTHWEAPON,
	SC_VOLCANO,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_WATK_ELEMENT,
	SC_LANDPROTECTOR,
	SC_ARMOR_ELEMENT,
	SC_NOCHAT,
	SC_BABY,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION,
	SC_TENSIONRELAX,
	SC_BERSERK,
	SC_FURY,
	SC_GOSPEL,
	SC_ASSUMPTIO,
	SC_BASILICA,
	SC_GUILDAURA,
	SC_MAGICPOWER,
	SC_EDP,
	SC_TRUESIGHT,
	SC_WINDWALK,
	SC_MELTDOWN,
	SC_CARTBOOST,
	SC_CHASEWALK,
	SC_REJECTSWORD,
	SC_MARIONETTE,
	SC_MARIONETTE2,
	SC_MOONLIT,
	SC_JOINTBEAT,
	SC_MINDBREAKER,
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	SC_DEVOTION,
	SC_SACRIFICE,
	SC_STEELBODY,
	SC_ORCISH,
	SC_READYSTORM,
	SC_STORMKICK,
	SC_READYDOWN,
	SC_DOWNKICK,
	SC_READYTURN,
	SC_TURNKICK,
	SC_READYCOUNTER,
	SC_COUNTER,
	SC_DODGE,
	SC_JUMPKICK,
	SC_RUN,
	SC_SHADOWWEAPON,
	SC_ADRENALINE2,
	SC_GHOSTWEAPON,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_ONEHAND,
	SC_PRESERVE,
	SC_BATTLEORDERS,
	SC_REGENERATION,
	SC_DOUBLECAST,
	SC_GRAVITATION,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	SC_SHRINK,
	SC_SIGHTBLASTER,
	SC_WINKCHARM,
	SC_CLOSECONFINE,
	SC_CLOSECONFINE2,
	SC_DANCING,
	SC_LULLABY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
	SC_NIBELUNGEN,
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	SC_WHISTLE,
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	SC_UGLYDANCE,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE,
	SC_SERVICE4U,
	SC_INTRAVISION,
	SC_INCALLSTATUS,
	SC_INCSTR,
	SC_INCAGI,
	SC_INCVIT,
	SC_INCINT,
	SC_INCDEX,
	SC_INCLUK,
	SC_INCHIT,
	SC_INCHITRATE,
	SC_INCFLEE,
	SC_INCFLEERATE,
	SC_INCMHPRATE,
	SC_INCMSPRATE,
	SC_INCATKRATE,
	SC_INCMATKRATE,
	SC_INCDEFRATE,
	SC_STRFOOD,
	SC_AGIFOOD,
	SC_VITFOOD,
	SC_INTFOOD,
	SC_DEXFOOD,
	SC_LUKFOOD,
	SC_STOP,	//Prevents inflicted chars from walking. [Skotlex]
	SC_SPIRIT,
	SC_COMA, //Not a real SC_, it makes a char's HP/SP hit 1.

	//
	SC_MAX, //Automatically updated max, used in for's and at startup to check we are within bounds. [Skotlex]
};
extern int SkillStatusChangeTable[];

//Numerates the Number for the status changes (client-dependent), imported from jA
enum {
	SI_BLANK			= -1,
	SI_PROVOKE			= 0,
	SI_ENDURE			= 1,
	SI_TWOHANDQUICKEN		= 2,
	SI_CONCENTRATE		= 3,
	SI_HIDING			= 4,
	SI_CLOAKING			= 5,
	SI_ENCPOISON		= 6,
	SI_POISONREACT		= 7,
	SI_QUAGMIRE			= 8,
	SI_ANGELUS			= 9,
	SI_BLESSING			= 10,
	SI_SIGNUMCRUCIS		= 11,
	SI_INCREASEAGI		= 12,
	SI_DECREASEAGI		= 13,
	SI_SLOWPOISON		= 14,
	SI_IMPOSITIO  		= 15,
	SI_SUFFRAGIUM		= 16,
	SI_ASPERSIO			= 17,
	SI_BENEDICTIO		= 18,
	SI_KYRIE			= 19,
	SI_MAGNIFICAT		= 20,
	SI_GLORIA			= 21,
	SI_AETERNA			= 22,
	SI_ADRENALINE		= 23,
	SI_WEAPONPERFECTION	= 24,
	SI_OVERTHRUST		= 25,
	SI_MAXIMIZEPOWER		= 26,
	SI_RIDING			= 27,
	SI_FALCON			= 28,
	SI_TRICKDEAD		= 29,
	SI_LOUD			= 30,
	SI_ENERGYCOAT		= 31,
	SI_BROKENARMOR		= 32,
	SI_BROKENWEAPON		= 33,
	SI_HALLUCINATION		= 34,
	SI_WEIGHT50 		= 35,
	SI_WEIGHT90			= 36,
	SI_ASPDPOTION		= 37,
	//38: Again Aspd Potion
	//39: Again Aspd Potion
	//40: Again Aspd Potion
	SI_SPEEDPOTION		= 41,
	//42: Again Speed Up
	SI_STRIPWEAPON		= 50,
	SI_STRIPSHIELD		= 51,
	SI_STRIPARMOR		= 52,
	SI_STRIPHELM		= 53,
	SI_CP_WEAPON		= 54,
	SI_CP_SHIELD		= 55,
	SI_CP_ARMOR			= 56,
	SI_CP_HELM			= 57,
	SI_AUTOGUARD		= 58,
	SI_REFLECTSHIELD		= 59,
	SI_PROVIDENCE		= 61,
	SI_DEFENDER			= 62,
	SI_AUTOSPELL		= 65,
	SI_SPEARQUICKEN		= 68,
	SI_EXPLOSIONSPIRITS	= 86,
	SI_FURY			= 87,
	SI_FIREWEAPON		= 90,
	SI_WATERWEAPON		= 91,
	SI_WINDWEAPON		= 92,
	SI_EARTHWEAPON		= 93,
// 102 = again gloria - from what I saw on screenshots, I wonder if it isn't gospel... [DracoRPG]
	SI_AURABLADE		= 103,
	SI_PARRYING			= 104,
	SI_CONCENTRATION		= 105,
	SI_TENSIONRELAX		= 106,
	SI_BERSERK			= 107,
	SI_ASSUMPTIO		= 110,
	SI_GUILDAURA		= 112,
	SI_MAGICPOWER		= 113,
	SI_EDP			= 114,
	SI_TRUESIGHT		= 115,
	SI_WINDWALK			= 116,
	SI_MELTDOWN			= 117,
	SI_CARTBOOST		= 118,
	SI_REJECTSWORD		= 120,
	SI_MARIONETTE		= 121,
// 122 SI_MARIONETTE again
	SI_MOONLIT			= 123,
	SI_BLEEDING			= 124,
	SI_JOINTBEAT		= 125,
	SI_DEVOTION			= 130,
	SI_STEELBODY		= 132,
	SI_WIGGLE			= 134, //It "wiggles" the character sprite on start/end. Used for SC_ORCISH
	SI_READYSTORM		= 135,
	SI_READYDOWN		= 137,
	SI_READYTURN		= 139,
	SI_READYCOUNTER		= 141,
	SI_DODGE			= 143,
	SI_RUN			= 145,
	SI_SHADOWWEAPON		= 146,
	//147 SI_ADRENALINE again
	SI_GHOSTWEAPON		= 148,
	SI_NIGHT			= 149, //slightly colors the screen with blue (night-like), unused by skills, but used for day/night
// 153 = causes character after-image effect.
	SI_KAIZEL			= 156,
	SI_KAAHI			= 157,
	SI_KAUPE			= 158,
// 159 = blue sparks and item-heal sound effect. Looks like item-use effect.
	SI_ONEHAND			= 161,
// 165 = ultra-red character
// 166 = ultra-red character
// 167 = ultra-red character
// 169 = sun
// 170 = moon
// 171 = stars
	SI_PRESERVE			= 181,
	SI_BATTLEORDERS		= 182,
// 184 = WTF?? creates the black shape of 4_m_02 NPC, with NPC talk cursor
	SI_DOUBLECAST		= 186,
	SI_MAXOVERTHRUST		= 188,
	SI_TAROT			= 191, // the icon allows no doubt... but what is it really used for ?? [DracoRPG]
	SI_SHRINK			= 197,
	SI_SIGHTBLASTER		= 198,
	SI_WINKCHARM		= 199,
	SI_CLOSECONFINE		= 200,
	SI_CLOSECONFINE2		= 201,
};
extern int StatusIconChangeTable[];

extern int current_equip_item_index;

//Mode definitions to clear up code reading. [Skotlex]
#define MD_CANMOVE 0x001
#define MD_LOOTER 0x002
#define MD_AGGRESSIVE 0x004
#define MD_ASSIST 0x08
#define MD_CASTSENSOR 0x010
#define MD_BOSS 0x020
#define MD_PLANT 0x040
#define MD_CANATTACK 0x080
#define MD_DETECTOR 0x100
//TODO: The following three modes are not yet in the mob_db!
#define MD_CHANGETARGET (0x004|0x200)
#define MD_CHANGECHASE 0x400
#define MD_BERSERK (0x800|0x004)
#define MD_MASK 0xFFF

// パラメータ所得系 battle.c より移動
int status_get_class(struct block_list *bl);
int status_get_dir(struct block_list *bl);
int status_get_lv(struct block_list *bl);
int status_get_range(struct block_list *bl);
int status_get_hp(struct block_list *bl);
int status_get_max_hp(struct block_list *bl);
int status_get_str(struct block_list *bl);
int status_get_agi(struct block_list *bl);
int status_get_vit(struct block_list *bl);
int status_get_int(struct block_list *bl);
int status_get_dex(struct block_list *bl);
int status_get_luk(struct block_list *bl);
int status_get_hit(struct block_list *bl);
int status_get_flee(struct block_list *bl);
int status_get_def(struct block_list *bl);
int status_get_mdef(struct block_list *bl);
int status_get_flee2(struct block_list *bl);
int status_get_def2(struct block_list *bl);
int status_get_mdef2(struct block_list *bl);
int status_get_batk(struct block_list *bl);
int status_get_atk(struct block_list *bl);
int status_get_atk2(struct block_list *bl);
int status_get_speed(struct block_list *bl);
int status_get_adelay(struct block_list *bl);
int status_get_amotion(struct block_list *bl);
int status_get_dmotion(struct block_list *bl);
int status_get_element(struct block_list *bl);
int status_get_attack_element(struct block_list *bl);
int status_get_attack_element2(struct block_list *bl);  //左手武器属性取得
#define status_get_elem_type(bl)	(status_get_element(bl)%10)
#define status_get_elem_level(bl)	(status_get_element(bl)/10/2)
int status_get_party_id(struct block_list *bl);
int status_get_guild_id(struct block_list *bl);
int status_get_race(struct block_list *bl);
int status_get_size(struct block_list *bl);
int status_get_mode(struct block_list *bl);
int status_get_mexp(struct block_list *bl);
int status_get_race2(struct block_list *bl);

struct status_change *status_get_sc_data(struct block_list *bl);
short *status_get_sc_count(struct block_list *bl);
short *status_get_opt1(struct block_list *bl);
short *status_get_opt2(struct block_list *bl);
short *status_get_opt3(struct block_list *bl);
short *status_get_option(struct block_list *bl);

int status_get_matk1(struct block_list *bl);
int status_get_matk2(struct block_list *bl);
int status_get_critical(struct block_list *bl);
int status_get_atk_(struct block_list *bl);
int status_get_atk_2(struct block_list *bl);
int status_get_atk2(struct block_list *bl);

int status_isdead(struct block_list *bl);
int status_isimmune(struct block_list *bl);

int status_get_sc_def(struct block_list *bl, int type);
#define status_get_sc_def_mdef(bl)	(status_get_sc_def(bl, SP_MDEF1))
#define status_get_sc_def_vit(bl)	(status_get_sc_def(bl, SP_DEF2))
#define status_get_sc_def_int(bl)	(status_get_sc_def(bl, SP_MDEF2))
#define status_get_sc_def_luk(bl)	(status_get_sc_def(bl, SP_LUK))

// 状態異常関連 skill.c より移動
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag);
int status_change_end( struct block_list* bl , int type,int tid );
int status_change_timer(int tid, unsigned int tick, int id, int data);
int status_change_timer_sub(struct block_list *bl, va_list ap );
int status_change_clear(struct block_list *bl,int type);
int status_change_clear_buffs(struct block_list *bl);
int status_change_clear_debuffs(struct block_list *bl);

int status_calc_pet(struct map_session_data* sd, int first); // [Skotlex]
int status_calc_pc(struct map_session_data* sd,int first);
int status_calc_str(struct block_list *,int);
int status_calc_agi(struct block_list *,int);
int status_calc_vit(struct block_list *,int);
int status_calc_int(struct block_list *,int);
int status_calc_dex(struct block_list *,int);
int status_calc_luk(struct block_list *,int);
int status_calc_batk(struct block_list *,int);
int status_calc_watk(struct block_list *,int);
int status_calc_matk(struct block_list *,int);
int status_calc_hit(struct block_list *,int);
int status_calc_critical(struct block_list *,int);
int status_calc_flee(struct block_list *,int);
int status_calc_flee2(struct block_list *,int);
int status_calc_def(struct block_list *,int);
int status_calc_def2(struct block_list *,int);
int status_calc_mdef(struct block_list *,int);
int status_calc_mdef2(struct block_list *,int);
int status_calc_speed(struct block_list *,int);
int status_calc_aspd_rate(struct block_list *,int);
int status_calc_maxhp(struct block_list *,int);
int status_calc_maxsp(struct block_list *,int);
int status_quick_recalc_speed(struct map_session_data*, int, int, char); // [Celest] - modified by [Skotlex]
int status_getrefinebonus(int lv,int type);
int status_check_skilluse(struct block_list *src, struct block_list *target, int skill_num, int flag); // [Skotlex]

//Use this to refer the max refinery level [Skotlex]
#define MAX_REFINE 10
extern int percentrefinery[5][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

int status_readdb(void);
int do_init_status(void);

#endif
