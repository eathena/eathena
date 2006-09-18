// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SKILL_H_
#define _SKILL_H_

#include "map.h"

#define MAX_SKILL_DB			750
#define MAX_SKILL_PRODUCE_DB	150
#define MAX_PRODUCE_RESOURCE	7
#define MAX_SKILL_ARROW_DB		150
#define MAX_SKILL_ABRA_DB		350

//Constants to identify the skill's inf value:
#define INF_ATTACK_SKILL 1
//For the time being, all trap-targetted skills ARE ground based:
#define INF_GROUND_SKILL (2|32)
// Skills casted on self where target is automatically chosen:
#define INF_SELF_SKILL 4
#define INF_SUPPORT_SKILL 16
#define INF_TARGET_TRAP 32

//Constants to identify a skill's nk value.
//The NK value applies only to non INF_GROUND_SKILL skills.
#define NK_NO_DAMAGE 1
#define NK_SPLASH_DAMAGE 2

//Constants to identify a skill's inf2 value.
#define INF2_QUEST_SKILL 1
//NPC skills are those that players can't have in their skill tree.
#define INF2_NPC_SKILL 2
#define INF2_WEDDING_SKILL 4
#define INF2_GUILD_SKILL 16
#define INF2_SONG_DANCE 32
#define INF2_ENSEMBLE_SKILL 64
#define INF2_TRAP 128
//Refers to ground placed skills that won't hurt the caster (like Grandcross)
#define INF2_TARGET_SELF 256
#define INF2_NO_TARGET_SELF 512
#define INF2_PARTY_ONLY 1024
#define INF2_GUILD_ONLY 2048

// スキルデ?タベ?ス
struct skill_db {
	const char *name;
	const char *desc;
	int range[MAX_SKILL_LEVEL];
	int hit;
	int inf;
	int pl;
	int nk;
	int max;
	int num[MAX_SKILL_LEVEL];
	int cast[MAX_SKILL_LEVEL];
	int delay[MAX_SKILL_LEVEL];
	int upkeep_time[MAX_SKILL_LEVEL];
	int upkeep_time2[MAX_SKILL_LEVEL];
	int castcancel;
	int cast_def_rate;
	int inf2;
	int maxcount;
	int skill_type;
	int blewcount[MAX_SKILL_LEVEL];
	int hp[MAX_SKILL_LEVEL];
	int sp[MAX_SKILL_LEVEL];
	int mhp[MAX_SKILL_LEVEL];
	int hp_rate[MAX_SKILL_LEVEL];
	int sp_rate[MAX_SKILL_LEVEL];
	int zeny[MAX_SKILL_LEVEL];
	int weapon;
	int state;
	int spiritball[MAX_SKILL_LEVEL];
	int itemid[10];
	int amount[10];
	int castnodex[MAX_SKILL_LEVEL];
	int delaynodex[MAX_SKILL_LEVEL];
	int nocast;
	int unit_id[2];
	int unit_layout_type[MAX_SKILL_LEVEL];
	int unit_range;
	int unit_interval;
	int unit_target;
	int unit_flag;
};
extern struct skill_db skill_db[MAX_SKILL_DB];

struct skill_name_db { 
        unsigned short id;     // skill id
        char *name; // search strings
        char *desc; // description that shows up for search's
};
extern const struct skill_name_db skill_names[];

#define MAX_SKILL_UNIT_LAYOUT	50
#define MAX_SQUARE_LAYOUT		5	// 11*11のユニット配置が最大
#define MAX_SKILL_UNIT_COUNT ((MAX_SQUARE_LAYOUT*2+1)*(MAX_SQUARE_LAYOUT*2+1))
struct skill_unit_layout {
	int count;
	int dx[MAX_SKILL_UNIT_COUNT];
	int dy[MAX_SKILL_UNIT_COUNT];
};

enum {
	UF_DEFNOTENEMY		= 0x0001,	// defnotenemy 設定でBCT_NOENEMYに切り替え
	UF_NOREITERATION	= 0x0002,	// 重複置き禁止 
	UF_NOFOOTSET		= 0x0004,	// 足元置き禁止
	UF_NOOVERLAP		= 0x0008,	// ユニット効果が重複しない
	UF_NOPC				= 0x0010,	//May not target players
	UF_NOMOB			= 0x0020,	//May not target mobs
	_UF_UNUSED			= 0x0040,
	UF_DUALMODE			= 0x0080,	//Spells should trigger both ontimer and onplace/onout/onleft effects.
	UF_DANCE			= 0x0100,	// ダンススキル
	UF_ENSEMBLE			= 0x0200,	// 合奏スキル
};

struct castend_delay
{
	block_list &src;
	uint32 target_id;
	unsigned short skill_id;
	unsigned short skill_lv;
	int flag;

	castend_delay(block_list &s, uint32 tid, unsigned short skid, unsigned short  sklv, int f) :
	 	src(s),
		target_id(tid),
		skill_id(skid),
		skill_lv(sklv),
		flag(f)
	{}
};


// アイテム作成デ?タベ?ス
struct skill_produce_db {
	unsigned short nameid;
	int trigger;
	unsigned short req_skill;
	int itemlv;
	int mat_id[MAX_PRODUCE_RESOURCE];
	int mat_amount[MAX_PRODUCE_RESOURCE];
};
extern struct skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];

// 矢作成デ?タベ?ス
struct skill_arrow_db {
	unsigned short nameid;
	int trigger;
	int cre_id[5];
	int cre_amount[5];
};
extern struct skill_arrow_db skill_arrow_db[MAX_SKILL_ARROW_DB];

// アブラカダブラデ?タベ?ス
struct skill_abra_db {
	unsigned short nameid;
	unsigned short req_lv;
	unsigned short per;
};
extern struct skill_abra_db skill_abra_db[MAX_SKILL_ABRA_DB];

extern int enchant_eff[5];
extern int deluge_eff[5];

struct block_list;
struct map_session_data;
struct skill_unit;
struct skill_unit_group;

int do_init_skill(void);

// スキルデ?タベ?スへのアクセサ
int	skill_get_hit( int id );
int	skill_get_inf( int id );
int	skill_get_pl( int id );
int	skill_get_nk( int id );
int	skill_get_max( int id );
int skill_get_range( int id , int lv );
int	skill_get_hp( int id ,int lv );
int	skill_get_mhp( int id ,int lv );
int	skill_get_sp( int id ,int lv );
int	skill_get_zeny( int id ,int lv );
int	skill_get_num( int id ,int lv );
int	skill_get_cast( int id ,int lv );
int	skill_get_delay( int id ,int lv );
int	skill_get_time( int id ,int lv );
int	skill_get_time2( int id ,int lv );
int	skill_get_castdef( int id );
int	skill_get_weapontype( int id );
int skill_get_unit_id(int id,int flag);
int	skill_get_inf2( int id );
int	skill_get_maxcount( int id );
int	skill_get_blewcount( int id ,int lv );
int	skill_get_unit_flag( int id );
const char*	skill_get_name( unsigned short id );


// スキルの使用
int skill_use_id( map_session_data *sd, uint32 target_id,unsigned short skill_num, unsigned short skill_lv);
int skill_use_pos( map_session_data *sd,int skill_x, int skill_y, unsigned short skill_num, unsigned short skill_lv);

int skill_castend_map( map_session_data *sd,int skill_num, const char *manname);

int skill_cleartimerskill(block_list *src);
int skill_addtimerskill(block_list *src,unsigned long tick,int target,int x,int y,unsigned short skill_id,unsigned short skill_lv,int type,int flag);

// 追加?果
int skill_additional_effect( block_list* src, block_list *bl,unsigned short skillid,unsigned short skilllv,int attack_type,unsigned long tick);

// ユニットスキル
struct skill_unit_group *skill_unitsetting( block_list *src, unsigned short skillid,unsigned short skilllv,int x,int y,int flag);
struct skill_unit *skill_initunit(struct skill_unit_group *group,int idx,int x,int y);
int skill_delunit(struct skill_unit *unit);
struct skill_unit_group *skill_initunitgroup(block_list *src,int count,unsigned short skillid,unsigned short skilllv,int unit_id);
int skill_delunitgroup(struct skill_unit_group &group);
int skill_clear_unitgroup(block_list *src);

int skill_unit_ondamaged(struct skill_unit *src,block_list *bl,int damage,unsigned long tick);

int skill_castfix(block_list *bl, long time);
int skill_delayfix(block_list *bl, long time);
int skill_check_unit_range(int m,int x,int y,unsigned short skillid, unsigned short skilllv);
int skill_check_unit_range2(int m,int x,int y,unsigned short skillid, unsigned short skilllv, object_t type);

// -- moonsoul	(added skill_check_unit_cell)
//int skill_check_unit_cell(int skillid,int m,int x,int y,int unit_id);
//int skill_unit_out_all( block_list *bl,unsigned long tick,int range);
int skill_unit_move(block_list &bl,unsigned long tick,int flag);
int skill_unit_move_unit_group(struct skill_unit_group& group, unsigned short m, int dx,int dy);

struct skill_unit_group *skill_check_dancing( block_list *src );
void skill_stop_dancing(block_list *src, int flag);

// Guild skills [celest]
//int skill_guildaura_sub (block_list &bl,va_list &ap);
class CSkillGuildaura : public CMapProcessor
{
	uint32 id;
	uint32 gid;
	int flag;
public:
	CSkillGuildaura(uint32 i, uint32 g, int f) : id(i), gid(g), flag(f)	{}
	~CSkillGuildaura()	{}
	virtual int process(block_list& bl) const;
};

// 詠唱キャンセル
int skill_castcancel(block_list *bl,int type);

int skill_gangsterparadise(map_session_data *sd ,int type);
int skill_rest(map_session_data &sd ,int type);
int skill_check_moonlit (block_list *bl, int dx, int dy);
void skill_brandishspear_first(struct square *tc,int dir,int x,int y);
void skill_brandishspear_dir(struct square *tc,int dir,int are);
int skill_autospell(map_session_data *md,int skillid);
void skill_devotion(map_session_data *md,uint32 target);
void skill_devotion2(block_list *bl,uint32 crusader);
int skill_devotion3(block_list *bl,uint32 target);
void skill_devotion_end(map_session_data *md,map_session_data *sd,uint32 target);

#define skill_calc_heal(bl,skill_lv) (( status_get_lv(bl)+status_get_int(bl) )/8 *(4+ skill_lv*8))

// その他
int skill_check_cloaking(block_list *bl);

// ステ?タス異常
int skill_enchant_elemental_end(block_list *bl, int type);
int skillnotok(int skillid, map_session_data &sd);

// アイテム作成
int skill_can_produce_mix(map_session_data &sd, unsigned short nameid, int trigger );
int skill_produce_mix( map_session_data &sd, unsigned short nameid, unsigned short slot1, unsigned short slot2, unsigned short slot3 );

int skill_arrow_create( map_session_data *sd,unsigned short nameid);

// mobスキルのため
int skill_castend_nodamage_id( block_list *src, block_list *bl,unsigned short skillid,unsigned short skilllv,unsigned long tick,int flag );
int skill_castend_damage_id  ( block_list* src, block_list *bl,unsigned short skillid,unsigned short skilllv,unsigned long tick,int flag );
int skill_castend_pos2       ( block_list *src, int x,int y,unsigned short skillid,unsigned short skilllv,unsigned long tick,int flag);

// スキル攻?一括?理
int skill_attack(int attack_type, block_list* src, block_list *dsrc,block_list *bl,unsigned short skillid,unsigned short skilllv,unsigned long tick,int flag );

void skill_reload(void);

enum {
	ST_NONE,ST_HIDING,ST_CLOAKING,ST_HIDDEN,ST_RIDING,ST_FALCON,ST_CART,ST_SHIELD,ST_SIGHT,ST_EXPLOSIONSPIRITS,
	ST_RECOV_WEIGHT_RATE,ST_MOVE_ENABLE,ST_WATER,
};

enum {
	NV_BASIC = 1,

	SM_SWORD,
	SM_TWOHAND,
	SM_RECOVERY,
	SM_BASH,
	SM_PROVOKE,
	SM_MAGNUM,
	SM_ENDURE,

	MG_SRECOVERY,
	MG_SIGHT,
	MG_NAPALMBEAT,
	MG_SAFETYWALL,
	MG_SOULSTRIKE,
	MG_COLDBOLT,
	MG_FROSTDIVER,
	MG_STONECURSE,
	MG_FIREBALL,
	MG_FIREWALL,
	MG_FIREBOLT,
	MG_LIGHTNINGBOLT,
	MG_THUNDERSTORM,

	AL_DP,
	AL_DEMONBANE,
	AL_RUWACH,
	AL_PNEUMA,
	AL_TELEPORT,
	AL_WARP,
	AL_HEAL,
	AL_INCAGI,
	AL_DECAGI,
	AL_HOLYWATER,
	AL_CRUCIS,
	AL_ANGELUS,
	AL_BLESSING,
	AL_CURE,

	MC_INCCARRY,
	MC_DISCOUNT,
	MC_OVERCHARGE,
	MC_PUSHCART,
	MC_IDENTIFY,
	MC_VENDING,
	MC_MAMMONITE,

	AC_OWL,
	AC_VULTURE,
	AC_CONCENTRATION,
	AC_DOUBLE,
	AC_SHOWER,

	TF_DOUBLE,
	TF_MISS,
	TF_STEAL,
	TF_HIDING,
	TF_POISON,
	TF_DETOXIFY,

	ALL_RESURRECTION,

	KN_SPEARMASTERY,
	KN_PIERCE,
	KN_BRANDISHSPEAR,
	KN_SPEARSTAB,
	KN_SPEARBOOMERANG,
	KN_TWOHANDQUICKEN,
	KN_AUTOCOUNTER,
	KN_BOWLINGBASH,
	KN_RIDING,
	KN_CAVALIERMASTERY,

	PR_MACEMASTERY,
	PR_IMPOSITIO,
	PR_SUFFRAGIUM,
	PR_ASPERSIO,
	PR_BENEDICTIO,
	PR_SANCTUARY,
	PR_SLOWPOISON,
	PR_STRECOVERY,
	PR_KYRIE,
	PR_MAGNIFICAT,
	PR_GLORIA,
	PR_LEXDIVINA,
	PR_TURNUNDEAD,
	PR_LEXAETERNA,
	PR_MAGNUS,

	WZ_FIREPILLAR,
	WZ_SIGHTRASHER,
	WZ_FIREIVY,
	WZ_METEOR,
	WZ_JUPITEL,
	WZ_VERMILION,
	WZ_WATERBALL,
	WZ_ICEWALL,
	WZ_FROSTNOVA,
	WZ_STORMGUST,
	WZ_EARTHSPIKE,
	WZ_HEAVENDRIVE,
	WZ_QUAGMIRE,
	WZ_ESTIMATION,

	BS_IRON,
	BS_STEEL,
	BS_ENCHANTEDSTONE,
	BS_ORIDEOCON,
	BS_DAGGER,
	BS_SWORD,
	BS_TWOHANDSWORD,
	BS_AXE,
	BS_MACE,
	BS_KNUCKLE,
	BS_SPEAR,
	BS_HILTBINDING,
	BS_FINDINGORE,
	BS_WEAPONRESEARCH,
	BS_REPAIRWEAPON,
	BS_SKINTEMPER,
	BS_HAMMERFALL,
	BS_ADRENALINE,
	BS_WEAPONPERFECT,
	BS_OVERTHRUST,
	BS_MAXIMIZE,

	HT_SKIDTRAP,
	HT_LANDMINE,
	HT_ANKLESNARE,
	HT_SHOCKWAVE,
	HT_SANDMAN,
	HT_FLASHER,
	HT_FREEZINGTRAP,
	HT_BLASTMINE,
	HT_CLAYMORETRAP,
	HT_REMOVETRAP,
	HT_TALKIEBOX,
	HT_BEASTBANE,
	HT_FALCON,
	HT_STEELCROW,
	HT_BLITZBEAT,
	HT_DETECTING,
	HT_SPRINGTRAP,

	AS_RIGHT,
	AS_LEFT,
	AS_KATAR,
	AS_CLOAKING,
	AS_SONICBLOW,
	AS_GRIMTOOTH,
	AS_ENCHANTPOISON,
	AS_POISONREACT,
	AS_VENOMDUST,
	AS_SPLASHER,

	NV_FIRSTAID,
	NV_TRICKDEAD,
	SM_MOVINGRECOVERY,
	SM_FATALBLOW,
	SM_AUTOBERSERK,
	AC_MAKINGARROW,
	AC_CHARGEARROW,
	TF_SPRINKLESAND,
	TF_BACKSLIDING,
	TF_PICKSTONE,
	TF_THROWSTONE,
	MC_CARTREVOLUTION,
	MC_CHANGECART,
	MC_LOUD,
	AL_HOLYLIGHT,
	MG_ENERGYCOAT,

	NPC_PIERCINGATT,
	NPC_MENTALBREAKER,
	NPC_RANGEATTACK,
	NPC_ATTRICHANGE,
	NPC_CHANGEWATER,
	NPC_CHANGEGROUND,
	NPC_CHANGEFIRE,
	NPC_CHANGEWIND,
	NPC_CHANGEPOISON,
	NPC_CHANGEHOLY,
	NPC_CHANGEDARKNESS,
	NPC_CHANGETELEKINESIS,
	NPC_CRITICALSLASH,
	NPC_COMBOATTACK,
	NPC_GUIDEDATTACK,
	NPC_SELFDESTRUCTION,
	NPC_SPLASHATTACK,
	NPC_SUICIDE,
	NPC_POISON,
	NPC_BLINDATTACK,
	NPC_SILENCEATTACK,
	NPC_STUNATTACK,
	NPC_PETRIFYATTACK,
	NPC_CURSEATTACK,
	NPC_SLEEPATTACK,
	NPC_RANDOMATTACK,
	NPC_WATERATTACK,
	NPC_GROUNDATTACK,
	NPC_FIREATTACK,
	NPC_WINDATTACK,
	NPC_POISONATTACK,
	NPC_HOLYATTACK,
	NPC_DARKNESSATTACK,
	NPC_TELEKINESISATTACK,
	NPC_MAGICALATTACK,
	NPC_METAMORPHOSIS,
	NPC_PROVOCATION,
	NPC_SMOKING,
	NPC_SUMMONSLAVE,
	NPC_EMOTION,
	NPC_TRANSFORMATION,
	NPC_BLOODDRAIN,
	NPC_ENERGYDRAIN,
	NPC_KEEPING,
	NPC_DARKBREATH,
	NPC_DARKBLESSING,
	NPC_BARRIER,
	NPC_DEFENDER,
	NPC_LICK,
	NPC_HALLUCINATION,
	NPC_REBIRTH,
	NPC_SUMMONMONSTER,

	RG_SNATCHER,
	RG_STEALCOIN,
	RG_BACKSTAP,
	RG_TUNNELDRIVE,
	RG_RAID,
	RG_STRIPWEAPON,
	RG_STRIPSHIELD,
	RG_STRIPARMOR,
	RG_STRIPHELM,
	RG_INTIMIDATE,
	RG_GRAFFITI,
	RG_FLAGGRAFFITI,
	RG_CLEANER,
	RG_GANGSTER,
	RG_COMPULSION,
	RG_PLAGIARISM,

	AM_AXEMASTERY,
	AM_LEARNINGPOTION,
	AM_PHARMACY,
	AM_DEMONSTRATION,
	AM_ACIDTERROR,
	AM_POTIONPITCHER,
	AM_CANNIBALIZE,
	AM_SPHEREMINE,
	AM_CP_WEAPON,
	AM_CP_SHIELD,
	AM_CP_ARMOR,
	AM_CP_HELM,
	AM_BIOETHICS,
	AM_BIOTECHNOLOGY,
	AM_CREATECREATURE,
	AM_CULTIVATION,
	AM_FLAMECONTROL,
	AM_CALLHOMUN,
	AM_REST,
	AM_DRILLMASTER,
	AM_HEALHOMUN,
	AM_RESURRECTHOMUN,

	CR_TRUST,
	CR_AUTOGUARD,
	CR_SHIELDCHARGE,
	CR_SHIELDBOOMERANG,
	CR_REFLECTSHIELD,
	CR_HOLYCROSS,
	CR_GRANDCROSS,
	CR_DEVOTION,
	CR_PROVIDENCE,
	CR_DEFENDER,
	CR_SPEARQUICKEN,

	MO_IRONHAND,
	MO_SPIRITSRECOVERY,
	MO_CALLSPIRITS,
	MO_ABSORBSPIRITS,
	MO_TRIPLEATTACK,
	MO_BODYRELOCATION,
	MO_DODGE,
	MO_INVESTIGATE,
	MO_FINGEROFFENSIVE,
	MO_STEELBODY,
	MO_BLADESTOP,
	MO_EXPLOSIONSPIRITS,
	MO_EXTREMITYFIST,
	MO_CHAINCOMBO,
	MO_COMBOFINISH,

	SA_ADVANCEDBOOK,
	SA_CASTCANCEL,
	SA_MAGICROD,
	SA_SPELLBREAKER,
	SA_FREECAST,
	SA_AUTOSPELL,
	SA_FLAMELAUNCHER,
	SA_FROSTWEAPON,
	SA_LIGHTNINGLOADER,
	SA_SEISMICWEAPON,
	SA_DRAGONOLOGY,
	SA_VOLCANO,
	SA_DELUGE,
	SA_VIOLENTGALE,
	SA_LANDPROTECTOR,
	SA_DISPELL,
	SA_ABRACADABRA,
	SA_MONOCELL,
	SA_CLASSCHANGE,
	SA_SUMMONMONSTER,
	SA_REVERSEORCISH,
	SA_DEATH,
	SA_FORTUNE,
	SA_TAMINGMONSTER,
	SA_QUESTION,
	SA_GRAVITY,
	SA_LEVELUP,
	SA_INSTANTDEATH,
	SA_FULLRECOVERY,
	SA_COMA,

	BD_ADAPTATION,
	BD_ENCORE,
	BD_LULLABY,
	BD_RICHMANKIM,
	BD_ETERNALCHAOS,
	BD_DRUMBATTLEFIELD,
	BD_RINGNIBELUNGEN,
	BD_ROKISWEIL,
	BD_INTOABYSS,
	BD_SIEGFRIED,
	BD_RAGNAROK,

	BA_MUSICALLESSON,
	BA_MUSICALSTRIKE,
	BA_DISSONANCE,
	BA_FROSTJOKE,
	BA_WHISTLE,
	BA_ASSASSINCROSS,
	BA_POEMBRAGI,
	BA_APPLEIDUN,

	DC_DANCINGLESSON,
	DC_THROWARROW,
	DC_UGLYDANCE,
	DC_SCREAM,
	DC_HUMMING,
	DC_DONTFORGETME,
	DC_FORTUNEKISS,
	DC_SERVICEFORYOU,

	NPC_RANDOMMOVE,
	NPC_SPEEDUP,
	NPC_REVENGE,

	WE_MALE,
	WE_FEMALE,
	WE_CALLPARTNER,

	ITM_TOMAHAWK = 337,

	NPC_DARKCROSS = 338,
	NPC_GRANDDARKNESS,
	NPC_DARKSTRIKE,
	NPC_DARKTHUNDER,
	NPC_STOP,
	NPC_BREAKWEAPON,
	NPC_BREAKARMOR,
	NPC_BREAKHELM,
	NPC_BREAKSHIELD,
	NPC_UNDEADATTACK,
	NPC_CHANGEUNDEAD,
	NPC_POWERUP,
	NPC_AGIUP,
	NPC_SIEGEMODE,
	NPC_CALLSLAVE,
	NPC_INVISIBLE,
	NPC_RUN,
	
	LK_AURABLADE = 355,
	LK_PARRYING,
	LK_CONCENTRATION,
	LK_TENSIONRELAX,
	LK_BERSERK,
	LK_FURY,
	HP_ASSUMPTIO,
	HP_BASILICA,
	HP_MEDITATIO,
	HW_SOULDRAIN,
	HW_MAGICCRASHER,
	HW_MAGICPOWER,
	PA_PRESSURE,
	PA_SACRIFICE,
	PA_GOSPEL,
	CH_PALMSTRIKE,
	CH_TIGERFIST,
	CH_CHAINCRUSH,
	PF_HPCONVERSION,
	PF_SOULCHANGE,
	PF_SOULBURN,
	ASC_KATAR,
	ASC_HALLUCINATION,
	ASC_EDP,
	ASC_BREAKER,
	SN_SIGHT,
	SN_FALCONASSAULT,
	SN_SHARPSHOOTING,
	SN_WINDWALK,
	WS_MELTDOWN,
	WS_CREATECOIN,
	WS_CREATENUGGET,
	WS_CARTBOOST,
	WS_SYSTEMCREATE,
	ST_CHASEWALK,
	ST_REJECTSWORD,
	ST_STEALBACKPACK,
	CR_ALCHEMY,
	CR_SYNTHESISPOTION,
	CG_ARROWVULCAN,
	CG_MOONLIT,
	CG_MARIONETTE,
	LK_SPIRALPIERCE,
	LK_HEADCRUSH,
	LK_JOINTBEAT,
	HW_NAPALMVULCAN,
	CH_SOULCOLLECT,
	PF_MINDBREAKER,
	PF_MEMORIZE,
	PF_FOGWALL,
	PF_SPIDERWEB,
	ASC_METEORASSAULT,
	ASC_CDP,
	WE_BABY,
	WE_CALLPARENT,
	WE_CALLBABY,

	TK_RUN = 411,
	TK_READYSTORM,
	TK_STORMKICK,
	TK_READYDOWN,
	TK_DOWNKICK,
	TK_READYTURN,
	TK_TURNKICK,
	TK_READYCOUNTER,
	TK_COUNTER,
	TK_DODGE,
	TK_JUMPKICK,
	TK_HPTIME,
	TK_SPTIME,
	TK_POWER,
	TK_SEVENWIND,
	TK_HIGHJUMP,
	SG_FEEL,
	SG_SUN_WARM,
	SG_MOON_WARM,
	SG_STAR_WARM,
	SG_SUN_COMFORT,
	SG_MOON_COMFORT,
	SG_STAR_COMFORT,
	SG_HATE,
	SG_SUN_ANGER,
	SG_MOON_ANGER,
	SG_STAR_ANGER,
	SG_SUN_BLESS,
	SG_MOON_BLESS,
	SG_STAR_BLESS,
	SG_DEVIL,
	SG_FRIEND,
	SG_KNOWLEDGE,
	SG_FUSION,
	SL_ALCHEMIST,
	AM_BERSERKPITCHER,
	SL_MONK,
	SL_STAR,
	SL_SAGE,
	SL_CRUSADER,
	SL_SUPERNOVICE,
	SL_KNIGHT,
	SL_WIZARD,
	SL_PRIEST,
	SL_BARDDANCER,
	SL_ROGUE,
	SL_ASSASIN,
	SL_BLACKSMITH,
	BS_ADRENALINE2,
	SL_HUNTER,
	SL_SOULLINKER,
	SL_KAIZEL,
	SL_KAAHI,
	SL_KAUPE,
	SL_KAITE,
	SL_KAINA,
	SL_STIN,
	SL_STUN,
	SL_SMA,
	SL_SWOO,
	SL_SKE,
	SL_SKA,

	SM_SELFPROVOKE = 473,
	NPC_EMOTION_ON,	
	ST_PRESERVE,
	ST_FULLSTRIP,
	WS_WEAPONREFINE,
	CR_SLIMPITCHER,
	CR_FULLPROTECTION,
	PA_SHIELDCHAIN,
	HP_MANARECHARGE,
	PF_DOUBLECASTING,
	HW_GANBANTEIN,
	HW_GRAVITATION,
	WS_CARTTERMINATION,
	WS_OVERTHRUSTMAX,
	CG_LONGINGFREEDOM,
	CG_HERMODE,
	CG_TAROTCARD,
	CR_ACIDDEMONSTRATION,
	CR_CULTIVATION,

	// = 492,
	TK_MISSION		= 493,
	SL_HIGH			= 494,
	KN_ONEHAND		= 495,
	AM_TWILIGHT1	= 496,
	AM_TWILIGHT2	= 497,
	AM_TWILIGHT3	= 498,
	HT_POWER 		= 499,

	GS_GLITTERING   = 500,//#GS_GLITTERING#
	GS_FLING,//#GS_FLING#
	GS_TRIPLEACTION,//#GS_TRIPLEACTION#
	GS_BULLSEYE,//#GS_BULLSEYE#
	GS_MADNESSCANCEL,//#GS_MADNESSCANCEL#
	GS_ADJUSTMENT,//#GS_ADJUSTMENT#
	GS_INCREASING,//#GS_INCREASING#
	GS_MAGICALBULLET,//#GS_MAGICALBULLET#
	GS_CRACKER,//#GS_CRACKER#
	GS_SINGLEACTION,//#GS_SINGLEACTION#
	GS_SNAKEEYE,//#GS_SNAKEEYE#	
	GS_CHAINACTION,//#GS_CHAINACTION#
	GS_TRACKING,//#GS_TRACKING#
	GS_DISARM,//#GS_DISARM#
	GS_PIERCINGSHOT,//#GS_PIERCINGSHOT#
	GS_RAPIDSHOWER,//#GS_RAPIDSHOWER#
	GS_DESPERADO,//#GS_DESPERADO#
	GS_GATLINGFEVER,//#GS_GATLINGFEVER#
	GS_DUST,//#GS_DUST#
	GS_FULLBUSTER,//#GS_FULLBUSTER#
	GS_SPREADATTACK,//#GS_SPREADATTACK#
	GS_GROUNDDRIFT,//#GS_GROUNDDRIFT#

	NJ_TOBIDOUGU,//#NJ_TOBIDOUGU#
	NJ_SYURIKEN,//#NJ_SYURIKEN#
	NJ_KUNAI,//#NJ_KUNAI#
	NJ_HUUMA,//#NJ_HUUMA#
	NJ_ZENYNAGE,//#NJ_ZENYNAGE#
	NJ_TATAMIGAESHI,//#NJ_TATAMIGAESHI#
	NJ_KASUMIKIRI,//#NJ_KASUMIKIRI#
	NJ_SHADOWJUMP,//#NJ_SHADOWJUMP#
	NJ_KIRIKAGE,//#NJ_KIRIKAGE#
	NJ_UTSUSEMI,//#NJ_UTSUSEMI#
	NJ_BUNSINJYUTSU,//#NJ_BUNSINJYUTSU#
	NJ_NINPOU,//#NJ_NINPOU#
	NJ_KOUENKA,//#NJ_KOUENKA#
	NJ_KAENSIN,//#NJ_KAENSIN#
	NJ_BAKUENRYU,//#NJ_BAKUENRYU#
	NJ_HYOUSENSOU,//#NJ_HYOUSENSOU#
	NJ_SUITON,//#NJ_SUITON#
	NJ_HYOUSYOURAKU,//#NJ_HYOUSYOURAKU#
	NJ_HUUJIN,//#NJ_HUUJIN#
	NJ_RAIGEKISAI,//#NJ_RAIGEKISAI#
	NJ_KAMAITACHI,//#NJ_KAMAITACHI#
	NJ_NEN,//#NJ_NEN#
	NJ_ISSEN,//#NJ_ISSEN#
	
	KN_CHARGEATK	=	1001,//#チャージアタック#
	CR_SHRINK		=	1002,//#シュリンク#
	AS_SONICACCEL	=	1003,//#ソニックアクセラレーション#
	AS_VENOMKNIFE	=	1004,//#ベナムナイフ#
	RG_CLOSECONFINE	=	1005,//#クローズコンファイン#
	WZ_SIGHTBLASTER	=	1006,//#サイトブラスター#
	SA_CREATECON	=	1007,//#エルレメンタルコンバータ製造#
	SA_ELEMENTWATER	=	1008,//#エルレメンタルチェンジ（水）#
	HT_PHANTASMIC	=	1009,//#ファンタスミックアロー#
	BA_PANGVOICE	=	1010,//#パンボイス#
	DC_WINKCHARM	=	1011,//#魅惑のウィンク#
	BS_UNFAIRLYTRICK=	1012,//#アンフェアリートリック#
	BS_GREED		=	1013,//#貪欲#
	PR_REDEMPTIO	=	1014,//#レデムプティオ#
	MO_KITRANSLATION=	1015,//#珍奇注入(振気注入)#
	MO_BALKYOUNG	=	1016,//#足頃(発勁)#
	SA_ELEMENTGROUND=	1017,
	SA_ELEMENTFIRE	=	1018,
	SA_ELEMENTWIND	=	1019,

	HM_SKILLBASE	=8001,
	HLIF_HEAL		=8001,//#治癒の手助け(ヒール)#
	HLIF_AVOID		=8002,//#緊急回避#
	HLIF_BRAIN		=8003,//=//#脳手術#
	HLIF_CHANGE		=8004,//#メンタルチェンジ#
	HAMI_CASTLE		=8005,//#キャストリング#
	HAMI_DEFENCE	=8006,//#ディフェンス#
	HAMI_SKIN		=8007,//#アダマンティウムスキン#
	HAMI_BLOODLUST	=8008,//#ブラッドラスト#
	HFLI_MOON		=8009,//#ムーンライト#
	HFLI_FLEET		=8010,//#フリートムーブ#
	HFLI_SPEED		=8011,//#オーバードスピード#
	HFLI_SBR44		=8012,//#S.B.R.44#
	HVAN_CAPRICE	=8013,//#カプリス#
	HVAN_CHAOTIC	=8014,//#カオティックベネディクション#
	HVAN_INSTRUCT	=8015,//#チェンジインストラクション#
	HVAN_EXPLOSION	=8016,//#バイオエクスプロージョン#

};

enum {
	UNT_SAFETYWALL = 0x7e,
	UNT_FIREWALL = 0x7f,
	UNT_WARP = 0x80,
	UNT_WARP_2 = 0x81,
	UNT_SANCTUARY = 0x83,
	UNT_MAGNUS = 0x84,
	UNT_PNEUMA = 0x85,
	UNT_MAGIC_SKILLS = 0x86,
	UNT_FIREPILLAR_HIDDEN = 0x87,
	UNT_FIREPILLAR_ACTIVE = 0x88,
	UNT_USEDTRAP = 0x8c,
	UNT_ICEWALL = 0x8d,
	UNT_QUAGMIRE = 0x8e,
	UNT_BLASTMINE = 0x8f,
	UNT_SKIDTRAP = 0x90,
	UNT_ANKLESNARE = 0x91,
	UNT_VENOMDUST = 0x92,
	UNT_LANDMINE = 0x93,
	UNT_SHOCKWAVE = 0x94,
	UNT_SANDMAN = 0x95,
	UNT_FLASHER = 0x96,
	UNT_FREEZINGTRAP = 0x97,
	UNT_CLAYMORETRAP = 0x98,
	UNT_TALKIEBOX = 0x99,
	UNT_VOLCANO = 0x9a,
	UNT_DELUGE = 0x9b,
	UNT_VIOLENTGALE = 0x9c,
	UNT_LANDPROTECTOR = 0x9d,
	UNT_LULLABY = 0x9e,
	UNT_RICHMANKIM = 0x9f,
	UNT_ETERNALCHAOS = 0xa0,
	UNT_DRUMBATTLEFIELD = 0xa1,
	UNT_RINGNIBELUNGEN = 0xa2,
	UNT_ROKISWEIL = 0xa3,
	UNT_INTOABYSS = 0xa4,
	UNT_SIEGFRIED = 0xa5,
	UNT_DISSONANCE = 0xa6,
	UNT_WHISTLE = 0xa7,
	UNT_ASSASSINCROSS = 0xa8,
	UNT_POEMBRAGI = 0xa9,
	UNT_APPLEIDUN = 0xaa,
	UNT_UGLYDANCE = 0xab,
	UNT_HUMMING = 0xac,
	UNT_DONTFORGETME = 0xad,
	UNT_FORTUNEKISS = 0xae,
	UNT_SERVICEFORYOU = 0xaf,
	UNT_GRAFFITI = 0xb0,
	UNT_DEMONSTRATION = 0xb1,
	UNT_CALLPARTNER = 0xb2,
	UNT_GOSPEL = 0xb3,
	UNT_BASILICA = 0xb4,
	UNT_FOGWALL = 0xb6,
	UNT_SPIDERWEB = 0xb7,
	UNT_GRAVITATION = 0xb8,
	UNT_HERMODE = 0xb9,
};

#endif
