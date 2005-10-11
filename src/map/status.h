#ifndef _STATUS_H_
#define _STATUS_H_

#include "map.h"

enum {
// Status changes that are sent to the client for icon/effect display
	SC_PROVOKE			= 0,
	SC_ENDURE			= 1,
	SC_TWOHANDQUICKEN	= 2,
	SC_CONCENTRATE		= 3,
	SC_HIDING			= 4,
	SC_CLOAKING			= 5,
	SC_ENCPOISON		= 6,
	SC_POISONREACT		= 7,
	SC_QUAGMIRE			= 8,
	SC_ANGELUS			= 9,
	SC_BLESSING			= 10,
	SC_SIGNUMCRUCIS		= 11,
	SC_INCREASEAGI		= 12,
	SC_DECREASEAGI		= 13,
	SC_SLOWPOISON		= 14,
	SC_IMPOSITIO  		= 15,
	SC_SUFFRAGIUM		= 16,
	SC_ASPERSIO			= 17,
	SC_BENEDICTIO		= 18,
	SC_KYRIE			= 19,
	SC_MAGNIFICAT		= 20,
	SC_GLORIA			= 21,
	SC_AETERNA			= 22,
	SC_ADRENALINE		= 23,
	SC_WEAPONPERFECTION	= 24,
	SC_OVERTHRUST		= 25,
	SC_MAXIMIZEPOWER	= 26,
	SC_RIDING			= 27,
	SC_FALCON			= 28,
	SC_TRICKDEAD		= 29,
	SC_LOUD				= 30,
	SC_ENERGYCOAT		= 31,
	SC_BROKENARMOR		= 32,
	SC_BROKENWEAPON		= 33,
	SC_HALLUCINATION	= 34,
	SC_WEIGHT50 		= 35,
	SC_WEIGHT90			= 36,
	SC_ASPDPOTION0		= 37,
	SC_ASPDPOTION1		= 38,
	SC_ASPDPOTION2		= 39,
	SC_ASPDPOTION3		= 40,
	SC_SPEEDUP0			= 41,
	SC_SPEEDUP1			= 42,
	SC_ATKPOTION		= 43,
	SC_MATKPOTION		= 44,
	SC_WEDDING			= 45,
	SC_SLOWDOWN			= 46,
	SC_ANKLE			= 47,
	SC_KEEPING			= 48,
	SC_BARRIER			= 49,
	SC_STRIPWEAPON		= 50,
	SC_STRIPSHIELD		= 51,
	SC_STRIPARMOR		= 52,
	SC_STRIPHELM		= 53,
	SC_CP_WEAPON		= 54,
	SC_CP_SHIELD		= 55,
	SC_CP_ARMOR			= 56,
	SC_CP_HELM			= 57,
	SC_AUTOGUARD		= 58,
	SC_REFLECTSHIELD	= 59,
	SC_SPLASHER			= 60,
	SC_PROVIDENCE		= 61,
	SC_DEFENDER			= 62,
	SC_MAGICROD			= 63,
	SC_SPELLBREAKER		= 64,
	SC_AUTOSPELL		= 65,
	SC_SIGHTTRASHER		= 66,
	SC_AUTOBERSERK		= 67,
	SC_SPEARSQUICKEN	= 68,
	SC_AUTOCOUNTER		= 69,
	SC_SIGHT			= 70,
	SC_SAFETYWALL		= 71,
	SC_RUWACH			= 72,
	SC_PNEUMA			= 73,
	SC_STONE			= 74,
	SC_FREEZE			= 75,
	SC_STAN				= 76,
	SC_SLEEP			= 77,
	SC_POISON			= 78,
	SC_CURSE			= 79,
	SC_SILENCE			= 80,
	SC_CONFUSION		= 81,
	SC_BLIND			= 82,
	SC_BLEEDING			= 83,
	SC_DPOISON			= 84,
	SC_EXTREMITYFIST	= 85,
	SC_EXPLOSIONSPIRITS	= 86,
	SC_COMBO			= 87,
	SC_BLADESTOP_WAIT	= 88,
	SC_BLADESTOP		= 89,
	SC_FIREWEAPON		= 90,
	SC_WATERWEAPON		= 91,
	SC_WINDWEAPON		= 92,
	SC_EARTHWEAPON		= 93,
	SC_VOLCANO			= 94,
	SC_DELUGE			= 95,
	SC_VIOLENTGALE		= 96,
	SC_WATK_ELEMENT		= 97, //Has no visual
	SC_LANDPROTECTOR	= 98,
	SC_ARMOR_ELEMENT	= 99, //Has no visual
	SC_NOCHAT			= 100,
	SC_BABY				= 101,
// 102 = gloria - from what I saw on screenshots, I wonder if it isn't gospel... [DracoRPG]
	SC_AURABLADE		= 103,
	SC_PARRYING			= 104,
	SC_CONCENTRATION	= 105,
	SC_TENSIONRELAX		= 106,
	SC_BERSERK			= 107,
	SC_FURY				= 108,
	SC_GOSPEL			= 109,
	SC_ASSUMPTIO		= 110,
	SC_BASILICA			= 111,
	SC_GUILDAURA		= 112,
	SC_MAGICPOWER		= 113,
	SC_EDP				= 114,
	SC_TRUESIGHT		= 115,
	SC_WINDWALK			= 116,
	SC_MELTDOWN			= 117,
	SC_CARTBOOST		= 118,
	SC_CHASEWALK		= 119,
	SC_REJECTSWORD		= 120,
	SC_MARIONETTE		= 121,
	SC_MARIONETTE2		= 122,
	SC_MOONLIT			= 123,
	SC_HEADCRUSH		= 124,
	SC_JOINTBEAT		= 125,
	SC_MINDBREAKER		= 126,
	SC_MEMORIZE			= 127,
	SC_FOGWALL			= 128,
	SC_SPIDERWEB		= 129,
	SC_DEVOTION			= 130,
	SC_SACRIFICE		= 131,
	SC_STEELBODY		= 132,
// 133 = empty
// 134 = wobbles the character's sprite when SC starts or ends
	SC_READYSTORM		= 135,
	SC_STORMKICK		= 136,
	SC_READYDOWN		= 137,
	SC_DOWNKICK			= 138,
	SC_READYTURN		= 139,
	SC_TURNKICK			= 140,
	SC_READYCOUNTER		= 141,
	SC_COUNTER			= 142,
	SC_DODGE			= 143,
	SC_JUMPKICK         = 144,
	SC_RUN				= 145,
	SC_SHADOWWEAPON		= 146,
	SC_ADRENALINE2      = 147,
	SC_GHOSTWEAPON		= 148,
// 149 = slightly colors the screen with blue (night-like)
// 150 = empty
// 151 = empty
// 152 = empty
// 153 = causes character after-image effect.
	SC_KAIZEL			= 156,
	SC_KAAHI            = 157,
	SC_KAUPE            = 158,
// 159 = slightly colors the screen with blue (night-like)
// 160 = empty
	SC_ONEHAND          = 161,
// 162 = empty
// 163 = empty
// 164 = empty
// 165 = ultra-red character
// 166 = ultra-red character
// 167 = ultra-red character
// 168 = empty
// 169 = sun
// 170 = moon
// 171 = stars
// 172 = empty
// 173 = empty
// 174 = empty
// 175 = empty
// 176 = empty
// 177 = empty
// 178 = empty
// 179 = empty
// 180 = empty
	SC_PRESERVE         = 181,
	SC_BATTLEORDERS		= 182,
	SC_REGENERATION		= 183,
// 184 = WTF?? creates the black shape of 4_m_02 NPC, with NPC talk cursor
// 185 = empty
	SC_DOUBLECAST		= 186,
	SC_GRAVITATION		= 187,
	SC_MAXOVERTHRUST	= 188,
	SC_LONGING			= 189,
	SC_HERMODE			= 190,
	SC_TAROT			= 191, // the icon allows no doubt... but what is it really used for ?? [DracoRPG]
// 193 = empty
// 194 = empty
// 195 = empty
// 196 = empty
// 197 = empty
// 198 = empty
// 199 = empty

	SC_SENDMAX			= 200, // 200 is enough at the moment to allow display of all client-side effects [DracoRPG]

// Status changes that are handled by the server but not sent to the client
	SC_DANCING			= 201,
	SC_LULLABY			= 202,
	SC_RICHMANKIM		= 203,
	SC_ETERNALCHAOS		= 204,
	SC_DRUMBATTLE		= 205,
	SC_NIBELUNGEN		= 206,
	SC_ROKISWEIL		= 207,
	SC_INTOABYSS		= 208,
	SC_SIEGFRIED		= 209,
//210 = empty
	SC_WHISTLE			= 211,
	SC_ASSNCROS			= 212,
	SC_POEMBRAGI		= 213,
	SC_APPLEIDUN		= 214,
	SC_UGLYDANCE		= 215,
	SC_HUMMING			= 216,
	SC_DONTFORGETME		= 217,
	SC_FORTUNE			= 218,
	SC_SERVICE4U		= 219,
	SC_INCALLSTATUS		= 220,
	SC_INCSTR			= 221,
	SC_INCAGI			= 222,
	SC_INCVIT			= 223,
	SC_INCINT			= 224,
	SC_INCDEX			= 225,
	SC_INCLUK			= 226,
	SC_INCHIT			= 227,
	SC_INCHITRATE		= 228,
	SC_INCFLEE			= 229,
	SC_INCFLEERATE		= 230,
	SC_INCMHPRATE		= 231,
	SC_INCMSPRATE		= 232,
	SC_INCATKRATE		= 233,
	SC_INCMATKRATE		= 234,
	SC_INCDEFRATE		= 235,
	SC_STRFOOD			= 236,
	SC_AGIFOOD			= 237,
	SC_VITFOOD			= 238,
	SC_INTFOOD			= 239,
	SC_DEXFOOD			= 240,
	SC_LUKFOOD			= 241,
	SC_HIGHJUMP			= 242,
	//
	SC_INTRAVISION		= 337

};
extern int SkillStatusChangeTable[];

extern int current_equip_item_index;

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
//Use this to refer the max refinery level [Skotlex]
#define MAX_REFINE 10
extern int percentrefinery[5][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

int status_readdb(void);
int do_init_status(void);

#endif
