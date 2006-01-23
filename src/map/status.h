#ifndef _STATUS_H_
#define _STATUS_H_

enum {	// struct map_session_data ÇÃ status_changeÇÃî‘?Ée?ÉuÉã
// SC_SENDMAXñ¢?ÇÕÉNÉâÉCÉAÉìÉgÇ÷ÇÃí ímÇ ÇËÅB
// 2-2éüêEÇÃílÇÕÇ»ÇÒÇ©ÇﬂÇøÇ·Ç≠ÇøÇ·Ç¡Ç€Ç¢ÇÃÇ≈ébíËÅBÇΩÇ‘ÇÒ?çXÇ≥ÇÍÇ‹Ç∑ÅB
	SC_SENDMAX			= 200,
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
	SC_IMPOSITIO		= 15,
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
	SC_BROKNARMOR		= 32,
	SC_BROKNWEAPON		= 33,
	SC_HALLUCINATION	= 34,
	SC_WEIGHT50			= 35,
	SC_WEIGHT90			= 36,
	SC_SPEEDPOTION0		= 37,
	SC_SPEEDPOTION1		= 38,
	SC_SPEEDPOTION2		= 39,
	SC_SPEEDPOTION3		= 40,
	SC_SPEEDUP0			= 41,
	SC_SPEEDUP1			= 42,
	SC_ATKPOT			= 43,	// [Valaris]
	SC_MATKPOT			= 44,	// [Valaris]
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
	SC_SPLASHER			= 60,	/* ÉxÉiÉÄÉXÉvÉâÉbÉVÉÉ? */
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
	SC_BLEEDING2			= 83,
	SC_DPOISON			= 84,
	SC_EXTREMITYFIST	= 85,
	SC_EXPLOSIONSPIRITS	= 86,
	SC_COMBO			= 87,
	SC_BLADESTOP_WAIT	= 88,
	SC_BLADESTOP		= 89,
	SC_FLAMELAUNCHER	= 90,
	SC_FROSTWEAPON		= 91,
	SC_LIGHTNINGLOADER	= 92,
	SC_SEISMICWEAPON	= 93,	
	SC_VOLCANO			= 94,
	SC_DELUGE			= 95,
	SC_VIOLENTGALE		= 96,
// 97
	SC_LANDPROTECTOR	= 98,
// 99
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
	SC_GUILDAURA        = 112,
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
// 133
// 134 = wobbles the character's sprite when SC starts or ends
	SC_READYSTORM		= 135,
// 136
	SC_READYDOWN		= 137,
// 138
	SC_READYCOUNTER		= 139,
// 140
	SC_READYTURN		= 141,
// 142
	SC_DODGE			= 143,
// 144
	SC_RUN				= 145,
// 146 = korean letter
	SC_ADRENALINE2      = 147,
// 148 = another korean letter
	SC_DANCING			= 149,
	SC_LULLABY			= 150,
	SC_RICHMANKIM		= 151,
	SC_ETERNALCHAOS		= 152,
	SC_DRUMBATTLE		= 153,
	SC_NIBELUNGEN		= 154,
	SC_ROKISWEIL		= 155,
	SC_INTOABYSS		= 156,
	SC_SIEGFRIED		= 157,
	SC_DISSONANCE		= 158,
	SC_WHISTLE			= 159,
	SC_ASSNCROS			= 160,
	SC_POEMBRAGI		= 161,
	SC_APPLEIDUN		= 162,
	SC_UGLYDANCE		= 163,
	SC_HUMMING			= 164,
	SC_DONTFORGETME		= 165,
	SC_FORTUNE			= 166,
	SC_SERVICE4U		= 167,
// 168
// 169 = sun
// 170 = moon
// 171 = stars
	SC_INCALLSTATUS		= 172,		/* ëSÇƒÇÃÉXÉeÅ[É^ÉXÇè„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCHIT			= 173,		/* HITè„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCFLEE			= 174,		/* FLEEè„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCMHP2			= 175,		/* MHPÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCMSP2			= 176,		/* MSPÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCATK2			= 177,		/* ATKÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCMATK2			= 178,		/* ATKÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCHIT2			= 179,		/* HITÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_INCFLEE2			= 180,		/* FLEEÇ%è„è∏(ç°ÇÃÇ∆Ç±ÇÎÉSÉXÉyÉãóp) */
	SC_PRESERVE         = 181,
	SC_BATTLEORDERS		= 182,
	SC_REGENERATION		= 183,
// 184 = WTF creates the black shape of 4_m_02 NPC, with NPC talk cursor
// 185
	SC_DOUBLECAST		= 186,
	SC_GRAVITATION		= 187,
	SC_MAXOVERTHRUST	= 188,
	SC_LONGING			= 189,
	SC_HERMODE			= 190,
	SC_TAROT			= 191,
// 192
// 193
	SC_INCDEF2			= 194,
	SC_INCSTR			= 195,
	SC_INCAGI			= 196,
	SC_SELFDESTRUCTION		= 201,
	SC_MAGNUM			= 202
};

extern int SkillStatusChangeTable[];

extern int current_equip_item_index;

#define SC_DIVINA SC_SILENCE
#define SC_BLEEDING SC_HEADCRUSH

// ÉpÉâÉÅÅ[É^èäìæån battle.c ÇÊÇËà⁄ìÆ
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
int status_get_baseatk(struct block_list *bl);
int status_get_atk(struct block_list *bl);
int status_get_atk2(struct block_list *bl);
int status_get_speed(struct block_list *bl);
int status_get_adelay(struct block_list *bl);
int status_get_amotion(struct block_list *bl);
int status_get_dmotion(struct block_list *bl);
int status_get_element(struct block_list *bl);
int status_get_attack_element(struct block_list *bl);
int status_get_attack_element2(struct block_list *bl);  //ç∂éËïêäÌëÆê´éÊìæ
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

// èÛë‘àŸèÌä÷òA skill.c ÇÊÇËà⁄ìÆ
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag);
int status_change_end( struct block_list* bl , int type,int tid );
int status_change_timer(int tid, unsigned int tick, int id, int data);
int status_change_timer_sub(struct block_list *bl, va_list ap );
int status_change_clear(struct block_list *bl,int type);
int status_change_clear_buffs(struct block_list *bl);
int status_change_clear_debuffs(struct block_list *bl);

int status_calc_pet(struct map_session_data* sd, int first); // [Skotlex]
// ÉXÉeÅ[É^ÉXåvéZ pc.c Ç©ÇÁï™ó£
// pc_calcstatus
int status_calc_pc(struct map_session_data* sd,int first);
int status_calc_speed(struct map_session_data*); // [Celest]
// int status_calc_skilltree(struct map_session_data *sd);
int status_getrefinebonus(int lv,int type);
int status_percentrefinery(struct map_session_data *sd,struct item *item);
//Use this to refer the max refinery level [Skotlex]
#define MAX_REFINE 10
extern int percentrefinery[5][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

int status_readdb(void);
int do_init_status(void);

//#define status_need_reset(i) 0;

#endif
