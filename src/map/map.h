// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_H_
#define _MAP_H_

#include "mmo.h"
#include "socket.h"
#include "db.h"
#include "script.h"
#include "coordinate.h"
#include "path.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////
#define MAX_PC_CLASS 4050
#define PC_CLASS_BASE 0
#define PC_CLASS_BASE2 (PC_CLASS_BASE + 4001)
#define PC_CLASS_BASE3 (PC_CLASS_BASE2 + 22)
#define MAX_NPC_PER_MAP 512
#define BLOCK_SIZE 8
#define AREA_SIZE ((int)config.area_size)
#define LOCAL_REG_NUM 16
#define LIFETIME_FLOORITEM 60
#define DAMAGELOG_SIZE 30
#define LOOTITEM_SIZE 10
#define MAX_SKILL_LEVEL 100
#define MAX_STATUSCHANGE 250
#define MAX_SKILLUNITGROUP 32
#define MAX_MOBSKILLUNITGROUP 8
#define MAX_SKILLUNITGROUPTICKSET 32
#define MAX_SKILLTIMERSKILL 32
#define MAX_MOBSKILLTIMERSKILL 10
#define MAX_MOBSKILL 32
#define MAX_MOB_LIST_PER_MAP 128
#define MAX_EVENTTIMER 32
#define NATURAL_HEAL_INTERVAL 500
#define MAX_FLOORITEM 500000
#define MAX_LEVEL 255
#define MAX_DROP_PER_MAP 48
#define MAX_IGNORE_LIST 80
#define MAX_VENDING 12
#define MAX_TRADING 10

///////////////////////////////////////////////////////////////////////////////
//Definitions for Jobs, this should help code be more readable. [Skotlex]
#define JOB_NOVICE 0
#define JOB_SWORDMAN 1
#define JOB_MAGE 2
#define JOB_ARCHER 3
#define JOB_ACOLYTE 4
#define JOB_MERCHANT 5
#define JOB_THIEF 6
#define JOB_KNIGHT 7
#define JOB_PRIEST 8
#define JOB_WIZARD 9
#define JOB_BLACKSMITH 10
#define JOB_HUNTER 11
#define JOB_ASSASSIN 12
#define JOB_KNIGHT2 13
#define JOB_CRUSADER 14
#define JOB_MONK 15
#define JOB_SAGE 16
#define JOB_ROGUE 17
#define JOB_ALCHEMIST 18
#define JOB_BARD 19
#define JOB_DANCER 20
#define JOB_CRUSADER2 21
#define JOB_WEDDING 22
#define JOB_SUPER_NOVICE 23

#define JOB_NOVICE_HIGH 4001
#define JOB_SWORDMAN_HIGH 4002
#define JOB_MAGE_HIGH 4003
#define JOB_ARCHER_HIGH 4004
#define JOB_ACOLYTE_HIGH 4005
#define JOB_MERCHANT_HIGH 4006
#define JOB_THIEF_HIGH 4007
#define JOB_LORD_KNIGHT 4008
#define JOB_HIGH_PRIEST 4009
#define JOB_HIGH_WIZARD 4010
#define JOB_WHITESMITH 4011
#define JOB_SNIPER 4012
#define JOB_ASSASSIN_CROSS 4013
#define JOB_LORD_KNIGHT2 4014
#define JOB_PALADIN 4015
#define JOB_CHAMPION 4016
#define JOB_PROFESSOR 4017
#define JOB_STALKER 4018
#define JOB_CREATOR 4019
#define JOB_CLOWN 4020
#define JOB_GYPSY 4021
#define JOB_PALADIN2 4022

#define JOB_BABY 4023
#define JOB_BABY_SWORDMAN 4024
#define JOB_BABY_MAGE 4025
#define JOB_BABY_ARCHER 4026
#define JOB_BABY_ACOLYTE 4027
#define JOB_BABY_MERCHANT 4028
#define JOB_BABY_THIEF 4029
#define JOB_BABY_KNIGHT 4030
#define JOB_BABY_PRIEST 4031
#define JOB_BABY_WIZARD 4032
#define JOB_BABY_BLACKSMITH 4033
#define JOB_BABY_HUNTER  4034
#define JOB_BABY_ASSASSIN 4035
#define JOB_BABY_KNIGHT2 4036
#define JOB_BABY_CRUSADER 4037
#define JOB_BABY_MONK 4038
#define JOB_BABY_SAGE 4039
#define JOB_BABY_ROGUE 4040
#define JOB_BABY_ALCHEMIST 4041
#define JOB_BABY_BARD 4042
#define JOB_BABY_DANCER 4043
#define JOB_BABY_CRUSADER2 4044
#define JOB_SUPER_BABY 4045
#define JOB_TAEKWON 4046
#define JOB_STAR_GLADIATOR 4047
#define JOB_STAR_GLADIATOR2 4048
#define JOB_SOUL_LINKER 4049


///////////////////////////////////////////////////////////////////////////////
#define OPTION_MASK		0xd7b8
#define CART_MASK		0x788
#define STATE_BLIND		0x10
#define MAX_SKILL_TREE	53
#define MOBID_EMPERIUM	1288

///////////////////////////////////////////////////////////////////////////////
#define EFFECT_FOG		515
#define EFFECT_SNOW		162
#define EFFECT_LEAVES	333
#define EFFECT_CLOUDS	233
#define EFFECT_CLOUDS2	516
#define EFFECT_SAKURA	163
#define EFFECT_RAIN		161
#define EFFECT_FIRE1	297
#define EFFECT_FIRE2	299
#define EFFECT_FIRE3	301

#define EFFECT_BIG		423
#define EFFECT_TINY		421



///////////////////////////////////////////////////////////////////////////////
#define FLAG_DISGUISE	0x80000000 // set the msb of the acount_id to signal a disguise


///////////////////////////////////////////////////////////////////////////////
#define DEFAULT_AUTOSAVE_INTERVAL 60*1000

///////////////////////////////////////////////////////////////////////////////
#define OPTION_HIDE 0x40




enum { NONE_ATTACKABLE,ATTACKABLE };

enum { ATK_LUCKY=1,ATK_FLEE,ATK_DEF};	// 囲まれペナルティ計算用

// 装備コード
enum {
	EQP_WEAPON		= 1,	// 右手
	EQP_ARMOR		= 2,	// 体
	EQP_SHIELD		= 4,	// 左手
	EQP_HELM		= 8		// 頭上段
};



///////////////////////////////////////////////////////////////////////////////
enum object_t { BL_NUL=0, BL_ALL=0, BL_PC, BL_NPC, BL_MOB, BL_ITEM, BL_CHAT, BL_SKILL, BL_PET, BL_HOM };	//xxx 0..7 -> 3bit
enum object_sub_t { WARP, SHOP, SCRIPT, MONS };											// 0..3 -> 2bit

///////////////////////////////////////////////////////////////////////////////
/// skill fail types.
enum skillfail_t
{
	SF_FAILED	= 0x0000,	//	btype==0 "skill failed"
	SF_NOEMOTION= 0x0100,	//	btype==1 "no emotions"
	SF_NOSIT	= 0x0200,	//	btype==2 "no sit"
	SF_NOCHAT	= 0x0300,	//	btype==3 "no chat"
	SF_NOPARTY	= 0x0400,	//	btype==4 "no party"
	SF_NOSHOUT	= 0x0500,	//	btype==5 "no shout"
	SF_NOPKING	= 0x0600,	//	btype==6 "no PKing"
	SF_NOALLIGN	= 0x0700,	//	btype==7 "no alligning"
	SF_SP		= 0x0001,	//	type==1 "insufficient SP"
	SF_HP		= 0x0002,	//	type==2 "insufficient HP"
	SF_MATERIAL	= 0x0003,	//	type==3 "insufficient materials"
	SF_DELAY	= 0x0004,	//	type==4 "there is a delay after using a skill"
	SF_ZENY		= 0x0005,	//	type==5 "insufficient zeny"
	SF_WEAPON	= 0x0006,	//	type==6 "wrong weapon"
	SF_REDGEM	= 0x0007,	//	type==7 "red jemstone needed"
	SF_BLUEGEM	= 0x0008,	//	type==8 "blue jemstone needed"
	SF_WEIGHT	= 0x0009,	//	type==9 "overweight"
	SF_FAILED2	= 0x0010,	//	type==10 "skill failed"
	SF_IGNORED	= 0x0011	//  type==11 ignored
};



///////////////////////////////////////////////////////////////////////////////
/// skill types. (equivalent to INF_*)
/// TODO is this a bit field or not
enum skilltype_t
{
	ST_PASSIVE = 0x00,// not target, can't be casted
	ST_ENEMY   = 0x01,// the skill targets an enemy
	ST_GROUND  = 0x02,// the skill targets the ground
	ST_SELF    = 0x04,// the skill targets self
	ST_FRIEND  = 0x10,// the skill targets a friend
	ST_TRAP    = 0x20 // the skill lays a trap
};



enum status_t {	// map_session_data の status_changeの番?テ?ブル
// MAX_STATUSCHANGE未?はクライアントへの通知あり。
// 2-2次職の値はなんかめちゃくちゃっぽいので暫定。たぶん?更されます。

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
	SC_SPEEDUP0			= 41, // for skill speedup
	SC_SPEEDUP1			= 42, // for skill speedup
	SC_ATKPOT			= 43,	// [Valaris]
	SC_MATKPOT			= 44,	// [Valaris]
	SC_WEDDING			= 45,	//結婚用(結婚衣裳になって?くのが?いとか)
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
	SC_SPLASHER			= 60,	// ベナムスプラッシャ? 
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
	SC_STUN				= 76,
	SC_SLEEP			= 77,
	SC_POISON			= 78,
	SC_CURSE			= 79,
	SC_SILENCE			= 80,
	SC_CONFUSION		= 81,
	SC_BLIND			= 82,
	SC_DIVINA			= SC_SILENCE,
	SC_BLEEDING			= 83,
	SC_DPOISON			= 84,		// 猛毒 

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
	SC_WATK_ELEMENT		= 97,
	SC_LANDPROTECTOR	= 98,
// 99
	SC_NOCHAT			= 100,	//赤エモ?態
	SC_BABY				= 101,
// <-- 102 = gloria
	SC_AURABLADE		= 103, // オ?ラブレ?ド 
	SC_PARRYING			= 104, // パリイング 
	SC_CONCENTRATION	= 105, // コンセントレ?ション 
	SC_TENSIONRELAX		= 106, // テンションリラックス 
	SC_BERSERK			= 107, // バ?サ?ク 
	SC_FURY				= 108,
	SC_GOSPEL			= 109,
	SC_ASSUMPTIO		= 110, // アシャンプティオ 
	SC_BASILICA			= 111,
	SC_GUILDAURA        = 112,
	SC_MAGICPOWER		= 113, // 魔法力?幅 
	SC_EDP				= 114, // エフェクトが判明したら移動 
	SC_TRUESIGHT		= 115, // トゥル?サイト 
	SC_WINDWALK			= 116, // ウインドウォ?ク 
	SC_MELTDOWN			= 117, // メルトダウン 
	SC_CARTBOOST		= 118, // カ?トブ?スト 
	SC_CHASEWALK		= 119,
	SC_REJECTSWORD		= 120, // リジェクトソ?ド 
	SC_MARIONETTE		= 121, // マリオネットコントロ?ル 
	SC_MARIONETTE2		= 122, // Marionette target
	SC_MOONLIT			= 123,
	SC_HEADCRUSH		= 124, // ヘッドクラッシュ 
	SC_JOINTBEAT		= 125, // ジョイントビ?ト 
	SC_MINDBREAKER		= 126,
	SC_MEMORIZE			= 127,		// メモライズ 
	SC_FOGWALL			= 128,
	SC_SPIDERWEB		= 129,		// スパイダ?ウェッブ 
	SC_DEVOTION			= 130,
	SC_SACRIFICE		= 131,	// サクリファイス 
	SC_STEELBODY		= 132,
// 133
// 134 = wobbles the character's sprite when SC starts or ends
	SC_READYSTORM		= 135,
	SC_STORMKICK		= 136,
	SC_READYDOWN		= 137,
	SC_DOWNKICK			= 138,
	SC_READYCOUNTER		= 139,
    SC_COUNTER			= 140,
	SC_READYTURN		= 141,
	SC_TURNKICK			= 142,
	SC_DODGE			= 143,
// 144
	SC_RUN				= 145,
// 146 = korean letter
	SC_ADRENALINE2      = 147,
// 148 = another korean letter
	SC_DANCING			= 149,
	SC_LULLABY			= 150,	//0x96
	SC_RICHMANKIM		= 151,
	SC_ETERNALCHAOS		= 152,
	SC_DRUMBATTLE		= 153,
	SC_NIBELUNGEN		= 154,
	SC_ROKISWEIL		= 155,
	SC_INTOABYSS		= 156,
	SC_SIEGFRIED		= 157,
	SC_DISSONANCE		= 158,	//0x9E
	SC_WHISTLE			= 159,
	SC_ASSNCROS			= 160,
	SC_POEMBRAGI		= 161,
	SC_APPLEIDUN		= 162,
	SC_UGLYDANCE		= 163,
	SC_HUMMING			= 164,
	SC_DONTFORGETME		= 165,
	SC_FORTUNE			= 166,	//0xA6
	SC_SERVICE4U		= 167,
// 168
// 169 = sun
// 170 = moon
// 171 = stars
	SC_INCALLSTATUS		= 172,		// 全てのステータスを上昇(今のところゴスペル用) 
	SC_INCHIT			= 173,		// HIT上昇(今のところゴスペル用) 
	SC_INCFLEE			= 174,		// FLEE上昇(今のところゴスペル用) 
	SC_INCMHP2			= 175,		// MHPを%上昇(今のところゴスペル用) 
	SC_INCMSP2			= 176,		// MSPを%上昇(今のところゴスペル用) 
	SC_INCATK2			= 177,		// ATKを%上昇(今のところゴスペル用) 
	SC_INCMATK2			= 178,		// ATKを%上昇(今のところゴスペル用) 
	SC_INCHIT2			= 179,		// HITを%上昇(今のところゴスペル用) 
	SC_INCFLEE2			= 180,		// FLEEを%上昇(今のところゴスペル用) 
	SC_PRESERVE         = 181,
	SC_BATTLEORDERS		= 182,	// unsure
	SC_REGENERATION		= 184,
// 184
// 185
	SC_DOUBLECAST		= 186,
	SC_GRAVITATION		= 187,
	SC_MAXOVERTHRUST	= 188,
	SC_LONGING			= 189,
	SC_HERMODE			= 190,
	SC_TAROT			= 191,	// unsure
//<-- 192 = gloria
//<-- 193 = gloria
	SC_INCDEF2			= 194,
	SC_INCSTR			= 195,
	SC_INCAGI			= 196,
	SC_INCVIT			= 197,
	SC_INCINT			= 198,
	SC_INCDEX			= 199,
	SC_INCLUK			= 200,
// <-- 201 = two hand quicken
/*
	SC_PROVOKE				= 0,	// プロボック 
	SC_ENDURE				= 1,	// インデュア 
	SC_TWOHANDQUICKEN		= 2,	// ツーハンドクイッケン 
	SC_CONCENTRATE			= 3,	// 集中力向上 
	SC_HIDING				= 4,	// ハイディング 
	SC_CLOAKING				= 5,	// クローキング 
	SC_ENCPOISON			= 6,	// エンチャントポイズン 
	SC_POISONREACT			= 7,	// ポイズンリアクト 
	SC_QUAGMIRE				= 8,	// クァグマイア 
	SC_ANGELUS				= 9,	// エンジェラス 
	SC_BLESSING				=10,	// ブレッシング 
	SC_SIGNUMCRUCIS			=11,	// シグナムクルシス？ 
	SC_INCREASEAGI			=12,	//  
	SC_DECREASEAGI			=13,	//  
	SC_SLOWPOISON			=14,	// スローポイズン 
	SC_IMPOSITIO			=15,	// イムポシティオマヌス 
	SC_SUFFRAGIUM			=16,	// サフラギウム 
	SC_ASPERSIO				=17,	// アスペルシオ 
	SC_BENEDICTIO			=18,	// 聖体降臨 
	SC_KYRIE				=19,	// キリエエレイソン 
	SC_MAGNIFICAT			=20,	// マグニフィカート 
	SC_GLORIA				=21,	// グロリア 
	SC_AETERNA				=22,	//  
	SC_ADRENALINE			=23,	// アドレナリンラッシュ 
	SC_WEAPONPERFECTION		=24,	// ウェポンパーフェクション 
	SC_OVERTHRUST			=25,	// オーバートラスト 
	SC_MAXIMIZEPOWER		=26,	// マキシマイズパワー 
	SC_RIDING				=27,	// ライディング 
	SC_FALCON				=28,	// ファルコンマスタリー 
	SC_TRICKDEAD			=29,	// 死んだふり 
	SC_LOUD					=30,	// ラウドボイス 
	SC_ENERGYCOAT			=31,	// エナジーコート 
	SC_PK_PENALTY			=32,	//PKのペナルティ
	SC_REVERSEORCISH		=33,    //リバースオーキッシュ
	SC_HALLUCINATION		=34,	// ハルネーションウォーク？ 
	SC_WEIGHT50				=35,	// 重量50％オーバー 
	SC_WEIGHT90				=36,	// 重量90％オーバー 
	SC_SPEEDPOTION0			=37,	// 速度ポーション？ 
	SC_SPEEDPOTION1			=38,	// スピードアップポーション？ 
	SC_SPEEDPOTION2			=39,	// ハイスピードポーション？ 
	SC_SPEEDPOTION3			=40,	// バーサークポーション 
	SC_ITEM_DELAY			=41,
	//
	//
	//
	//
	//
	//
	//
	//
	SC_STRIPWEAPON			=50,	// ストリップウェポン 
	SC_STRIPSHIELD			=51,	// ストリップシールド 
	SC_STRIPARMOR			=52,	// ストリップアーマー 
	SC_STRIPHELM			=53,	// ストリップヘルム 
	SC_CP_WEAPON			=54,	// ケミカルウェポンチャージ 
	SC_CP_SHIELD			=55,	// ケミカルシールドチャージ 
	SC_CP_ARMOR				=56,	// ケミカルアーマーチャージ 
	SC_CP_HELM				=57,	// ケミカルヘルムチャージ 
	SC_AUTOGUARD			=58,	// オートガード 
	SC_REFLECTSHIELD		=59,	// リフレクトシールド 
	SC_DEVOTION				=60,	// ディボーション 
	SC_PROVIDENCE			=61,	// プロヴィデンス 
	SC_DEFENDER				=62,	// ディフェンダー 
	SC_SANTA				=63,	//サンタ
	//
	SC_AUTOSPELL			=65,	// オートスペル 
	//
	//
	SC_SPEARSQUICKEN		=68,	// スピアクイッケン 
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	SC_EXPLOSIONSPIRITS		=86,	// 爆裂波動 
	SC_STEELBODY			=87,	// 金剛 
	//
	SC_COMBO				=89,
	SC_FLAMELAUNCHER		=90,	// フレイムランチャー 
	SC_FROSTWEAPON			=91,	// フロストウェポン 
	SC_LIGHTNINGLOADER		=92,	// ライトニングローダー 
	SC_SEISMICWEAPON		=93,	// サイズミックウェポン 
	//
	//
	//
	//
	//
	//
	//
	//
	//
	//
	SC_AURABLADE			=103,	// オーラブレード 
	SC_PARRYING				=104,	// パリイング 
	SC_CONCENTRATION		=105,	// コンセントレーション 
	SC_TENSIONRELAX			=106,	// テンションリラックス 
	SC_BERSERK				=107,	// バーサーク 
	//
	//
	//
	SC_ASSUMPTIO			=110,	// アスムプティオ 
	//
	//
	SC_MAGICPOWER			=113,	// 魔法力増幅 
	SC_EDP					=114,	// エフェクトが判明したら移動 
	SC_TRUESIGHT			=115,	// トゥルーサイト 
	SC_WINDWALK				=116,	// ウインドウォーク 
	SC_MELTDOWN				=117,	// メルトダウン 
	SC_CARTBOOST			=118,	// カートブースト 
	SC_CHASEWALK			=119,	// チェイスウォーク 
	SC_REJECTSWORD			=120,	// リジェクトソード 
	SC_MARIONETTE			=121,	// マリオネットコントロール  //自分用
	SC_MARIONETTE2			=122,	// マリオネットコントロール  //ターゲット用
	//
	SC_HEADCRUSH			=124,	// ヘッドクラッシュ 
	SC_JOINTBEAT			=125,	// ジョイントビート 
	//
	//
	SC_STONE				=128,	// 状態異常：石化 
	SC_FREEZE				=129,	// 状態異常：氷結 
	SC_STUN					=130,	// 状態異常：スタン 
	SC_SLEEP				=131,	// 状態異常：睡眠 
	SC_POISON				=132,	// 状態異常：毒 
	SC_CURSE				=133,	// 状態異常：呪い 
	SC_SILENCE				=134,	// 状態異常：沈黙 
	SC_CONFUSION			=135,	// 状態異常：混乱 
	SC_BLIND				=136,	// 状態異常：暗闇 
	SC_BLEED				=137,	// 状態異常：出血 
	SC_DIVINA				= SC_SILENCE,	// 状態異常：沈黙 
	//138
	//139
	SC_SAFETYWALL			=140,	// セーフティーウォール 
	SC_PNEUMA				=141,	// ニューマ 
	//
	SC_ANKLE				=143,	// アンクルスネア 
	SC_DANCING				=144,	//  
	SC_KEEPING				=145,	//  
	SC_BARRIER				=146,	//  
	//
	//
	SC_MAGICROD				=149,	//  
	SC_SIGHT				=150,	//  
	SC_RUWACH				=151,	//  
	SC_AUTOCOUNTER			=152,	//  
	SC_VOLCANO				=153,	//  
	SC_DELUGE				=154,	//  
	SC_VIOLENTGALE			=155,	//  
	SC_BLADESTOP_WAIT		=156,	//  
	SC_BLADESTOP			=157,	//  
	SC_EXTREMITYFIST		=158,	//  
	SC_GRAFFITI				=159,	//  
	SC_LULLABY				=160,	//  
	SC_RICHMANKIM			=161,	//  
	SC_ETERNALCHAOS			=162,	//  
	SC_DRUMBATTLE			=163,	//  
	SC_NIBELUNGEN			=164,	//  
	SC_ROKISWEIL			=165,	//  
	SC_INTOABYSS			=166,	//  
	SC_SIEGFRIED			=167,	//  
	SC_DISSONANCE			=168,	//  
	SC_WHISTLE				=169,	//  
	SC_ASSNCROS				=170,	//  
	SC_POEMBRAGI			=171,	//  
	SC_APPLEIDUN			=172,	//  
	SC_UGLYDANCE			=173,	//  
	SC_HUMMING				=174,	//  
	SC_DONTFORGETME			=175,	//  
	SC_FORTUNE				=176,	//  
	SC_SERVICE4U			=177,	//  
	SC_BASILICA				=178,	//  
	SC_MINDBREAKER			=179,	//  
	SC_SPIDERWEB			=180,	// スパイダーウェッブ 
	SC_MEMORIZE				=181,	// メモライズ 
	SC_DPOISON				=182,	// 猛毒 
	//
	SC_SACRIFICE			=184,	// サクリファイス 
	SC_INCATK				=185,	//item 682用
	SC_INCMATK				=186,	//item 683用
	SC_WEDDING				=187,	//結婚用(結婚衣裳になって歩くのが遅いとか)
	SC_NOCHAT				=188,	//赤エモ状態
	SC_SPLASHER				=189,	// ベナムスプラッシャー 
	SC_SELFDESTRUCTION		=190,	// 自爆 
	SC_MAGNUM				=191,	// マグナムブレイク 
	SC_GOSPEL				=192,	// ゴスペル 
	SC_INCALLSTATUS			=193,	// 全てのステータスを上昇(今のところゴスペル用) 
	SC_INCHIT				=194,	// HIT上昇(今のところゴスペル用) 
	SC_INCFLEE				=195,	// FLEE上昇(今のところゴスペル用) 
	SC_INCMHP2				=196,	// MHPを%上昇(今のところゴスペル用) 
	SC_INCMSP2				=197,	// MSPを%上昇(今のところゴスペル用) 
	SC_INCATK2				=198,	// ATKを%上昇(今のところゴスペル用) 
	SC_INCHIT2				=199,	// HITを%上昇(今のところゴスペル用) 
	SC_INCFLEE2				=200,	// FLEEを%上昇(今のところゴスペル用) 
	SC_PRESERVE				=201,	// プリザーブ 
	SC_OVERTHRUSTMAX		=202,	// オーバートラストマックス 
	SC_CHASEWALK_STR		=203,	//STR上昇（チェイスウォーク用）
	SC_WHISTLE_				=204,
	SC_ASSNCROS_			=205,
	SC_POEMBRAGI_			=206,
	SC_APPLEIDUN_			=207,
	SC_HUMMING_				=209,
	SC_DONTFORGETME_		=210,
	SC_FORTUNE_				=211,
	SC_SERVICE4U_			=212,
	SC_BATTLEORDER			=213,	//ギルドスキル
	SC_REGENERATION			=214,
	SC_BATTLEORDER_DELAY	=215,
	SC_REGENERATION_DELAY	=216,
	SC_RESTORE_DELAY		=217,
	SC_EMERGENCYCALL_DELAY	=218,
	SC_POISONPOTION			=219,
	SC_THE_MAGICIAN			=220,
	SC_STRENGTH				=221,
	SC_THE_DEVIL			=222,
	SC_THE_SUN				=223,
	SC_MEAL_INCSTR			=224,	//食事用
	SC_MEAL_INCAGI			=225,
	SC_MEAL_INCVIT			=226,
	SC_MEAL_INCINT			=227,
	SC_MEAL_INCDEX			=228,
	SC_MEAL_INCLUK			=229,
	SC_RUN 					= 230,
	SC_SPURT 				= 231,
	SC_TKCOMBO 				= 232,	//テコンのコンボ用
	SC_DODGE				= 233,
	SC_HERMODE				= 234,
	SC_TRIPLEATTACK_RATE_UP	= 235,	//三段発動率アップ
	SC_COUNTER_RATE_UP		= 236,	//カウンターキック発動率アップ
	SC_SUN_WARM				= 237,
	SC_MOON_WARM			= 238,
	SC_STAR_WARM			= 239,
	SC_SUN_COMFORT			= 240,
	SC_MOON_COMFORT			= 241,
	SC_STAR_COMFORT			= 242,
	SC_FUSION				= 243,
	SC_ALCHEMIST			= 244,	//魂
	SC_MONK					= 245,
	SC_STAR					= 246,
	SC_SAGE					= 247,
	SC_CRUSADER				= 248,
	SC_SUPERNOVICE			= 249,
	SC_KNIGHT				= 250,
	SC_WIZARD				= 251,
	SC_PRIEST				= 252,
	SC_BARDDANCER			= 253,
	SC_ROGUE				= 254,
	SC_ASSASIN				= 255,
	SC_BLACKSMITH			= 256,
	SC_HUNTER				= 257,
	SC_SOULLINKER			= 258,
	SC_HIGH					= 259,
	//魂の追加があったらいけないので予約
	//なければ適当にあとから埋める
	//= 260,	//忍者の魂の予約？
	//= 261,	//ガンスリンガーの魂の予約？
	//= 262,	//？？？の魂の予約？
	SC_ADRENALINE2			= 263,
	SC_KAIZEL				= 264,
	SC_KAAHI				= 265,
	SC_KAUPE				= 266,
	SC_KAITE				= 267,
	SC_SMA					= 268,	//エスマ詠唱可能時間用
	SC_SWOO					= 269,
	SC_SKE					= 270,
	SC_SKA					= 271,
	SC_ONEHAND				= 272,
	SC_READYSTORM			= 273,
	SC_READYDOWN			= 274,
	SC_READYTURN			= 275,
	SC_READYCOUNTER			= 276,
	SC_DODGE_DELAY			= 277,
	SC_AUTOBERSERK			= 278,
	SC_DEVIL				= 279,
	SC_DOUBLECASTING 		= 280,	//ダブルキャスティング
	SC_ELEMENTFIELD			= 281,	//属性場
	SC_DARKELEMENT			= 282,	//闇
	SC_ATTENELEMENT			= 283,	//念
	SC_MIRACLE				= 284,	//太陽と月と星の奇跡
	SC_ANGEL				= 285,	//太陽と月と星の天使
	SC_HIGHJUMP				= 286,	//ハイジャンプ
	SC_DOUBLE				= 287,	//ダブルストレイフィング状態
	SC_ACTION_DELAY			= 288,	//
	SC_BABY					= 289,	//パパ、ママ、大好き
	SC_LONGINGFREEDOM		= 290,
	SC_SHRINK				= 291,	//#シュリンク#
	SC_CLOSECONFINE			= 292,	//#クローズコンファイン#
	SC_SIGHTBLASTER			= 293,	//#サイトブラスター#
	SC_ELEMENTWATER			= 294,	//#エルレメンタルチェンジ水#
	//食事用2
	SC_MEAL_INCHIT			= 295,
	SC_MEAL_INCFLEE			= 296,
	SC_MEAL_INCFLEE2		= 297,
	SC_MEAL_INCCRITICAL		= 298,
	SC_MEAL_INCDEF			= 299,
	SC_MEAL_INCMDEF			= 300,
	SC_MEAL_INCATK			= 301,
	SC_MEAL_INCMATK			= 302,
	SC_MEAL_INCEXP			= 303,
	SC_MEAL_INCJOB			= 304,
	//
	SC_ELEMENTGROUND		= 305,	//土(鎧)
	SC_ELEMENTFIRE			= 306,	//火(鎧)
	SC_ELEMENTWIND			= 307,	//風(鎧)
	SC_WINKCHARM			= 308,
	SC_ELEMENTPOISON		= 309,	//毒(鎧)
	SC_ELEMENTDARK			= 310,	//闇(鎧)
	SC_ELEMENTELEKINESIS	= 311,	//念(鎧)
	SC_ELEMENTUNDEAD		= 312,	//不死(鎧)
	SC_UNDEADELEMENT		= 313,	//不死(武)
	SC_ELEMENTHOLY			= 314,	//聖(鎧)
	SC_NPC_DEFENDER			= 315,
	SC_RESISTWATER			= 316,	//耐性
	SC_RESISTGROUND			= 317,	//耐性
	SC_RESISTFIRE			= 318,	//耐性
	SC_RESISTWIND			= 319,	//耐性
	SC_RESISTPOISON			= 320,	//耐性
	SC_RESISTHOLY			= 321,	//耐性
	SC_RESISTDARK			= 322,	//耐性
	SC_RESISTTELEKINESIS	= 323,	//耐性
	SC_RESISTUNDEAD			= 324,	//耐性
	SC_RESISTALL			= 325,	//耐性
	//種族変更？
	SC_RACEUNKNOWN			= 326,	//無形
	SC_RACEUNDEAD			= 327,	//不死種族
	SC_RACEBEAST			= 328,
	SC_RACEPLANT			= 329,
	SC_RACEINSECT			= 330,
	SC_RACEFISH				= 332,
	SC_RACEDEVIL			= 333,
	SC_RACEHUMAN			= 334,
	SC_RACEANGEL			= 335,
	SC_RACEDRAGON			= 336,
	SC_TIGEREYE				= 337,
	SC_GRAVITATION_USER		= 338,
	SC_GRAVITATION			= 339,
	SC_FOGWALL				= 340,
	SC_FOGWALLPENALTY		= 341,
	SC_REDEMPTIO			= 342,
	SC_TAROTCARD			= 343,
	SC_HOLDWEB				= 344,
	SC_INVISIBLE			= 345,
	SC_DETECTING			= 346,
	//
	SC_FLING				= 347,
	SC_MADNESSCANCEL		= 348,
	SC_ADJUSTMENT			= 349,
	SC_INCREASING			= 350,
	SC_DISARM				= 351,
	SC_GATLINGFEVER			= 352,
	SC_FULLBUSTER			= 353,
	//ニンジャスキル
	SC_TATAMIGAESHI			= 354,
	SC_UTSUSEMI				= 355,	//#NJ_UTSUSEMI#
	SC_BUNSINJYUTSU			= 356,	//#NJ_BUNSINJYUTSU#
	SC_SUITON				= 357,	//#NJ_SUITON#
	SC_NEN					= 358,	//#NJ_NEN#
	SC_AVOID				= 359,	//#緊急回避#
	SC_CHANGE				= 360,	//#メンタルチェンジ#
	SC_DEFENCE				= 361,	//#ディフェンス#
	SC_BLOODLUST			= 362,	//#ブラッドラスト#
	SC_FLEET				= 363,	//#フリートムーブ#
	SC_SPEED				= 364,	//#オーバードスピード#
	
	//startでは使えないresistをアイテム側で全てクリアするための物
	SC_RESISTCLEAR			= 1001,
	SC_RACECLEAR			= 1002,
	SC_SOUL					= 1003,
	SC_SOULCLEAR			= 1004,
*/
	SC_SUITON				= 201,	//#NJ_SUITON#
	SC_AVOID				= 202,	//#緊急回避#
	SC_CHANGE				= 203,	//#メンタルチェンジ#
	SC_DEFENCE				= 204,	//#ディフェンス#
	SC_BLOODLUST			= 205,	//#ブラッドラスト#
	SC_FLEET				= 206,	//#フリートムーブ#
	SC_SPEED				= 207,	//#オーバードスピード#

	SC_MAX //Automatically updated max, used in for's and at startup to check we are within bounds. [Skotlex]
};


/*
external base classes
doubled linked list	-> 2 pointers -> 8[16] byte

mapbase			a virtual interface for maps
				derives:
				data:
				members:
				............
				just the base class for map and foreign_map (which are "maps on other host")
				............

map				the actual map
				derives:	mapbase
				data:		mapdata, blockroots ...
				members:	pathsearch, mapiterators (replacement for the foreach_..)
				............


mapobject		something that can be added to a map, a simple immobile object with id
				derives:	doubled linked list
				data:		ui id, us map, us posx, us posy, (ch bdir,ch hdir)	-> +10 (+12) byte
				members:	whatever is done with mapobjects
				............
				the directions might fit in here since they fill the data structure to a 4byte boundary
				(which is done by the compiler anyway) and (almost) any derived element is using it
				maybe also combine it with a status bitfield since dir's only use 3 bit
				............

movable			move interface, allows moving objects
				derives:	block
				data:		
				members:	
				............

battleobj		battle interface, hp/sp, skill
				derives:	movable
				data:		
				members:	
				............

script			script interface
				derives:	
				data:		
				members:	
				............

npc/warp/shop	script interface, move interface (script controlled) [warp,shop as special derivated, no unions]
				derives:	movable, script
				data:		
				members:	
				............

pet/mob/merc	script interface, move/battle interface (script/ai controlled)
				derives:	battleobj, (script)
				data:		
				members:	
				............

player/homun	client contolled  move/battle interface, socket processor
				derives:	battleobj
				data:		
				members:	
				............

*/


///////////////////////////////////////////////////////////////////////////////
// predeclarations
struct block_list;
struct skill_unit_group;
struct item_data;
struct pet_db;

struct movable;
struct affectable;
struct fightable;
struct map_session_data;
struct npc_data;
struct npcscript_data;
struct npcwarp_data;
struct npcshop_data;
struct mob_data;
struct pet_data;
class flooritem_data;
class chat_data;
class npcchat_data;
struct skill_unit;
class homun_data;









///////////////////////////////////////////////////////////////////////////////
/// processing base class.
/// process member is called for each block
///////////////////////////////////////////////////////////////////////////////
class CMapProcessor : public basics::noncopyable
{
public:
	ICL_EMPTY_COPYCONSTRUCTOR(CMapProcessor)
protected:
	CMapProcessor()				{}
public:
	virtual ~CMapProcessor()	{}
	virtual int process(block_list& bl) const = 0;
};



///////////////////////////////////////////////////////////////////////////////
/// object on a map
struct block_list : public coordinate
{
//private:
	/////////////////////////////////////////////////////////////////
	static block_list bl_head;
	static dbt* id_db;
public:
	/// returns blocklist from given id
	static block_list* from_blid(uint32 id);
	/// returns blocklist from name.
	/// only implemented  for individual block types
	static block_list* from_name(const char *name)	{ return NULL; }

	// functions that work on block_lists
	static int foreachinarea(const CMapProcessor& elem, unsigned short m, int x0,int y0,int x1,int y1,object_t type);
	static int foreachincell(const CMapProcessor& elem, unsigned short m,int x,int y,object_t type);
	static int countoncell(unsigned short m, int x, int y, object_t type);
	static int foreachinmovearea(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,object_t type);
	static int foreachinpath(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int range,object_t type);
	static int foreachpartymemberonmap(const CMapProcessor& elem, map_session_data &sd, bool area);
	static int foreachobject(const CMapProcessor& elem,object_t type);
	static skill_unit *skillunit_oncell(block_list &target, int x, int y, ushort skill_id, skill_unit *out_unit);


	/////////////////////////////////////////////////////////////////
	unsigned short m;	// redo coordinate to also hold maps
	uint32 id;
private:
	// data for internally used double-link list
	block_list *next;
	block_list *prev;
public:

	/////////////////////////////////////////////////////////////////
	/// default constructor.
	block_list(uint32 i=0) : 
		m(0),
		id(i),
		next(NULL),
		prev(NULL)
	{}
	/////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~block_list()	
	{	// not yet removed from map
		if(this->prev)
			this->delblock();
		// and just to be sure that it's removed from iddb
		this->deliddb();
	}
	// missing:
	// all interactions with map 
	// which should be directly wrapped into a static interface
	// any comments? [Hinoko]

	// we'll leave it global right now and wrap it later
	// just move the basic functions here for the moment
	// and take care for the necessary refinements [Shinomori]

	/// add block to identifier database
	void addiddb();
	/// remove block from identifier database
	void deliddb();

	/// add block to map
	bool addblock();
	/// remove block from map
	bool delblock();

	/// check if a block in on a map
	bool is_on_map() const
	{
		return this->prev!=NULL;
	}

	// collision free deleting of blocks
	int freeblock();
	static int freeblock_lock(void);
	static int freeblock_unlock(void);



	// New member functions by Aru to cleanup code and 
	// automate sanity checking when converting block_list
	// to map_session_data, mob_data or pet_data

	// change all access to virtual overloads
	// so these might be not necessary when finished, 
	// also the type and subtype members will be obsolete
	// since objects can identify themselfs [Shinomori]

	// ok some more explanation on this; 
	// simple example in virtual overloads:
	/*
	// predeclaration
	class derive1;
	class derive2;

	// base class
	class base		
	{
	public:
		// virtual conversion operators to derived class
		// initialized with returning NULL since base class 
		// is neither of type derived1 nor of derived2
		virtual operator derive1*()	{return NULL;}
		virtual operator derive2*()	{return NULL;}
	};


	class derive1 : public base
	{
	public:
		// virtual conversion operator
		// only the one that gets overloaded in this class
		// actually only returns a pointer to the class itself
		virtual operator derive1*()	{return this;}
	};

	class derive2 : public base
	{
	public:
		// virtual conversion operator
		// only the one that gets overloaded in this class
		// actually only returns a pointer to the class itself
		virtual operator derive2*()	{return this;}
	};

	{
		// instanciate the two objects
		derive1 d1;
		derive2 d2;

		// get the pointers
		base *b1 = &d1;
		base *b2 = &d2;

		// from here on we can work with pointers to base objects
		// but the object can identify itself, in this case here
		// by using the conversion operator, 
		// any other function would be also possible
		
		derive1 *pd11 = *b1;	// since b1 points to a d1 object, it used the d1 conversion
		derive1 *pd12 = *b2;	// this conersion was not overloaded and will return NULL
								// meaning that b2 does not point to a derived1 object

		derive2 *pd21 = *b1;	// vise versa
		derive2 *pd22 = *b2;
	}

	*/


	// might later replace the type compare
	// this has to be overloaded and could also used with masks instead pure enums
	virtual bool is_type(object_t t) const
	{
		return (t==BL_NUL); // non-type
	}
	bool operator==(object_t t) const
	{
		return this->is_type(t);
	}
	bool operator!=(object_t t) const
	{
		return !this->is_type(t);
	}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual map_session_data*		get_sd()				{ return NULL; }
	virtual const map_session_data* get_sd() const			{ return NULL; }

	virtual pet_data*				get_pd()				{ return NULL; }
	virtual const pet_data*			get_pd() const			{ return NULL; }

	virtual mob_data*				get_md()				{ return NULL; }
	virtual const mob_data*			get_md() const			{ return NULL; }

	virtual npc_data*				get_nd()				{ return NULL; }
	virtual const npc_data*			get_nd() const			{ return NULL; }

	virtual npcshop_data*			get_shop()				{ return NULL; }
	virtual const npcshop_data*		get_shop() const		{ return NULL; }

	virtual npcscript_data*			get_script()			{ return NULL; }
	virtual const npcscript_data*	get_script() const		{ return NULL; }

	virtual npcwarp_data*			get_warp()				{ return NULL; }
	virtual const npcwarp_data*		get_warp() const		{ return NULL; }

	virtual homun_data*				get_hd()				{ return NULL; }
	virtual const homun_data*		get_hd() const			{ return NULL; }

	virtual chat_data*				get_cd()				{ return NULL; }
	virtual const chat_data*		get_cd() const			{ return NULL; }

	virtual skill_unit*				get_sk()				{ return NULL; }
	virtual const skill_unit*		get_sk() const			{ return NULL; }

	virtual flooritem_data*			get_fd()				{ return NULL; }
	virtual const flooritem_data*	get_fd() const			{ return NULL; }

	virtual movable*				get_movable()			{ return NULL; }
	virtual const movable*			get_movable() const		{ return NULL; }

	virtual affectable*				get_affectable()		{ return NULL; }
	virtual const affectable*		get_affectable() const	{ return NULL; }

	virtual fightable*				get_fightable()			{ return NULL; }
	virtual const fightable*		get_fightable() const	{ return NULL; }



	///////////////////////////////////////////////////////////////////////////
	/// return head direction.
	/// default function, needs to be overloaded at specific implementations
	virtual dir_t get_headdir() const		{ return DIR_S; }
	/// return body direction.
	/// default function, needs to be overloaded at specific implementations
	virtual dir_t get_bodydir() const		{ return DIR_S; }
	/// alias to body direction.
	virtual dir_t get_dir() const			{ return DIR_S; }

	///////////////////////////////////////////////////////////////////////////
	/// set directions seperately.
	virtual void set_dir(dir_t b, dir_t h)	{}
	/// set both directions equally.
	virtual void set_dir(dir_t d)			{}
	/// set directions to look at target
	virtual void set_dir(const coordinate& to)	{}
	/// set body direction only.
	virtual void set_bodydir(dir_t d)		{}
	/// set head direction only.
	virtual void set_headdir(dir_t d)		{}

	/// check if withing AREA reange.
	virtual bool is_near(const block_list& bl) const;

	///////////////////////////////////////////////////////////////////////////
	// status functions
	// overloaded at derived classes

	/// checks for walking state
	virtual bool is_walking() const			{ return false; }
	/// checks for attack state
	virtual bool is_attacking() const		{ return false; }
	/// checks for skill state
	virtual bool is_casting() const		{ return false; }
	/// checks for dead state
	virtual bool is_dead() const			{ return false; }
	/// checks for sitting state
	virtual bool is_sitting() const			{ return false; }
	/// checks for idle state (alive+not sitting+not blocked by skill)
	virtual bool is_idle() const			{ return true; }

	/// sets the object to idle state
	virtual bool set_idle()					{ return false; }
	/// sets the object delay
	virtual void set_delay(ulong delaytick)	{ this->set_idle(); }
	/// sets the object to idle state
	virtual bool set_dead()					{ return false; }
	/// sets the object to sitting state
	virtual bool set_sit()					{ return false; }
	/// sets the object to standing state
	virtual bool set_stand()				{ return false; }



	/// checks for flying state
	virtual bool is_flying() const			{ return false; }
	/// checks for confusion state
	virtual bool is_confuse() const			{ return false; }
	/// checks if this is attackable
	virtual bool is_attackable() const		{ return false; }


	virtual bool is_hiding() const			{ return false; }
	virtual bool is_cloaking() const		{ return false; }
	virtual bool is_chasewalk() const		{ return false; }
	virtual bool is_carton() const			{ return false; }
	virtual bool is_falcon() const			{ return false; }
	virtual bool is_riding() const			{ return false; }
	/// checks for invisible state
	virtual bool is_invisible() const		{ return false; }
	virtual bool is_50overweight() const	{ return false; }
	virtual bool is_90overweight() const	{ return false; }

	virtual int get_class() const			{ return 0; }
	virtual int get_lv() const				{ return 0; }
	virtual int get_range() const			{ return 0; }
	virtual int get_hp() const				{ return 1; }

	virtual int get_race() const			{ return 0; }
	virtual int get_race2() const			{ return 0; }
	virtual int get_mode() const			{ return 1; }// とりあえず動くということで1
	virtual int get_mexp() const			{ return 0; }
	virtual int get_size() const			{ return 1; }

	virtual uint32 get_party_id() const		{ return 0; }
	virtual uint32 get_guild_id() const		{ return 0; }


	////////
	//## TODO
	virtual int get_max_hp() const;
	virtual int get_str() const;
	virtual int get_agi() const;
	virtual int get_vit() const;
	virtual int get_int() const;
	virtual int get_dex() const;
	virtual int get_luk() const;
	////////


	bool is_boss() const					{ return 0!=this->get_mexp(); }
	bool is_undead() const;


	///////////////////////////////////////////////////////////////////////////
	// walking functions
	// overloaded at derived classes

	/// walk to position
	virtual bool walktoxy(const coordinate& pos,bool easy=false)						{ return false; }
	/// walk to a coordinate
	virtual bool walktoxy(unsigned short x,unsigned short y,bool easy=false)			{ return false; }
	/// stop walking
	virtual bool stop_walking(int type=1)												{ return false; }
	/// instant position change
	virtual bool movepos(const coordinate &target)										{ return false; }
	/// instant position change
	virtual bool movepos(unsigned short x,unsigned short y)								{ return false; }
	/// warps to a given map/position. 
	virtual bool warp(unsigned short m, unsigned short x, unsigned short y, int type)	{ return false; }


	///////////////////////////////////////////////////////////////////////////
	// attack functions

	/// starts attack
	virtual bool start_attack(uint32 target_id, bool cont)		{ return false; }
	/// starts attack
	virtual bool start_attack(const block_list& target_bl, bool cont)	{ return false; }
	/// stops attack
	virtual bool stop_attack()							{ return false; }

	virtual int heal(int hp, int sp=0)					{ return 0; }

	///////////////////////////////////////////////////////////////////////////
	// targeting functions

	/// unlock from current target
	virtual void unlock_target()						{ unlock_target(0); }
	virtual void unlock_target(unsigned long tick)		{ }


	///////////////////////////////////////////////////////////////////////////
	// skill functions
	/// stops skill
	virtual bool stop_skill()							{ return false; }
	// skill failed message.
	virtual int  skill_check(ushort id)					{ return 0; }
	/// check if current skill can be canceled
	virtual bool skill_can_cancel() const				{ return false; }
	/// called when a skill has failed.
	/// does nothing on default
	virtual void skill_stopped(ushort skillid)			{}
	/// does nothing on default
	virtual void skill_failed(ushort skill_id, skillfail_t type=SF_FAILED)	{}


	///////////////////////////////////////////////////////////////////////////
	// status functions, new layout
	/// check if status exists
	virtual bool has_status(status_t status_id) const		{ return false; }
	virtual basics::numptr& get_statusvalue1(status_t status_id)	{ static basics::numptr dummy; return dummy; }
	virtual basics::numptr& get_statusvalue2(status_t status_id)	{ static basics::numptr dummy; return dummy; }
	virtual basics::numptr& get_statusvalue3(status_t status_id)	{ static basics::numptr dummy; return dummy; }
	virtual basics::numptr& get_statusvalue4(status_t status_id)	{ static basics::numptr dummy; return dummy; }
	virtual basics::numptr get_statusvalue1(status_t status_id)	const	{ return basics::numptr(); }
	virtual basics::numptr get_statusvalue2(status_t status_id) const	{ return basics::numptr(); }
	virtual basics::numptr get_statusvalue3(status_t status_id) const	{ return basics::numptr(); }
	virtual basics::numptr get_statusvalue4(status_t status_id) const	{ return basics::numptr(); }



	/// remove a status
	virtual bool remove_status(status_t status_id)		{ return false; }
	/// remove all status changes
	virtual bool remove_status()						{ return false; }
	/// create a new status. or replace an existing
	virtual bool create_status(status_t status_id, const basics::numptr& v1=basics::numptr(), const basics::numptr& v2=basics::numptr(), const basics::numptr& v3=basics::numptr(), const basics::numptr& v4=basics::numptr())
	{
		return false;
	}
};



///////////////////////////////////////////////////////////////////////////////
struct vending_element
{
	unsigned short index;
	unsigned short amount;
	uint32 value;

	// default constructor
	vending_element() : 
		index(0),
		amount(0),
		value(0)
	{}
};

///////////////////////////////////////////////////////////////////////////////
struct weapon_data
{
	long watk;
	long watk2;

	long overrefine;
	long star;
	long atk_ele;
	long atkmods[3];
	long addsize[3];
	long addele[10];
	long addrace[12];
	long addrace2[12];
	long ignore_def_ele;
	long ignore_def_race;
	short ignore_def_mob;
	long def_ratio_atk_ele;
	long def_ratio_atk_race;
	long add_damage_class_count;
	short add_damage_classid[10];
	long add_damage_classrate[10];
	short hp_drain_rate;
	short hp_drain_per;
	short hp_drain_value;
	short sp_drain_rate;
	short sp_drain_per;
	short sp_drain_value;

	unsigned fameflag : 1;

	// default constructor
	weapon_data() : 
		watk(0),
		watk2(0),
		overrefine(0),
		star(0),
		atk_ele(0),
		ignore_def_ele(0),
		ignore_def_race(0),
		ignore_def_mob(0),
		def_ratio_atk_ele(0),
		def_ratio_atk_race(0),
		add_damage_class_count(0),
		hp_drain_rate(0),
		hp_drain_per(0),
		hp_drain_value(0),
		sp_drain_rate(0),
		sp_drain_per(0),
		sp_drain_value(0),
		fameflag(0)
	{
		memset(atkmods,0,sizeof(atkmods));
		memset(addsize,0,sizeof(addsize));
		memset(addele,0,sizeof(addele));
		memset(addrace,0,sizeof(addrace));
		memset(addrace2,0,sizeof(addrace2));
		memset(add_damage_classid,0,sizeof(add_damage_classid));
		memset(add_damage_classrate,0,sizeof(add_damage_classrate));
	}
};






















///////////////////////////////////////////////////////////////////////////////
class flooritem_data : public block_list
{
public:

	/////////////////////////////////////////////////////////////////
	static flooritem_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_fd():NULL;
	}
	/////////////////////////////////////////////////////////////////


	unsigned char subx;
	unsigned char suby;
	int cleartimer;
	uint32 first_get_id;
	uint32 second_get_id;
	uint32 third_get_id;
	uint32 first_get_tick;
	uint32 second_get_tick;
	uint32 third_get_tick;
	struct item item_data;

	flooritem_data() :
		subx(0), suby(0), cleartimer(-1),
		first_get_id(0), second_get_id(0), third_get_id(0),
		first_get_tick(0), second_get_tick(0), third_get_tick(0)
	{ }
	virtual ~flooritem_data()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_ITEM);
	}
	virtual flooritem_data*			get_fd()				{ return this; }
	virtual const flooritem_data*	get_fd() const			{ return this; }


private:
	flooritem_data(const flooritem_data&);					// forbidden
	const flooritem_data& operator=(const flooritem_data&);	// forbidden
};












///////////////////////////////////////////////////////////////////////////////

// map_getcell()/map_setcell()で使用されるフラグ
typedef enum { 
	CELL_CHKWALL=0,		// 壁(セルタイプ1)
	CELL_CHKWATER,		// 水場(セルタイプ3)
	CELL_CHKGROUND,		// 地面障害物(セルタイプ5)
	CELL_CHKPASS,		// 通過可能(セルタイプ1,5以外)
	CELL_CHKNOPASS,		// 通過不可(セルタイプ1,5)
	CELL_CHKHOLE,		// a hole in morroc desert
	CELL_GETTYPE,		// セルタイプを返す
	CELL_CHKNOPASS_NPC,

	CELL_CHKNPC=0x10,	// タッチタイプのNPC(セルタイプ0x80フラグ)
	CELL_SETNPC,		// タッチタイプのNPCをセット
	CELL_CLRNPC,		// タッチタイプのNPCをclear suru
	CELL_CHKBASILICA,	// バジリカ(セルタイプ0x40フラグ)
	CELL_SETBASILICA,	// バジリカをセット
	CELL_CLRBASILICA,	// バジリカをクリア
	CELL_CHKMOONLIT,
	CELL_SETMOONLIT,
	CELL_CLRMOONLIT,
	CELL_CHKREGEN,
	CELL_SETREGEN,
	CELL_CLRREGEN
} cell_t;

// CELL
#define CELL_MASK		0x07	// 3 bit for cell mask

// celests new stuff
//#define CELL_MOONLIT	0x100
//#define CELL_REGEN		0x200

enum {
	GAT_NONE		= 0,	// normal ground
	GAT_WALL		= 1,	// not passable and blocking
	GAT_UNUSED1		= 2,
	GAT_WATER		= 3,	// water
	GAT_UNUSED2		= 4,
	GAT_GROUND		= 5,	// not passable but can shoot/cast over it
	GAT_HOLE		= 6,	// holes in morroc desert
	GAT_UNUSED3		= 7
};

struct mapgat // values from .gat & 
{
	unsigned char type : 3;		// 3bit used for land,water,wall,(hole) (values 0,1,3,5,6 used)
								// providing 4 bit space and interleave two cells in x dimension
								// would not waste memory too much; will implement it later on a new map model
	unsigned char npc : 4;		// 4bit counter for npc touchups, can hold 15 touchups;
	unsigned char basilica : 1;	// 1bit for basilica (is on/off for basilica enough, what about two casting priests?)
	unsigned char moonlit : 1;	// 1bit for moonlit
	unsigned char regen : 1;	// 1bit for regen
	unsigned char _unused : 6;	// 6 bits left

	mapgat() :
		type(0),npc(0),basilica(0),moonlit(0),regen(0),_unused(0)
	{}
};
// will alloc a short now


struct map_data
{
	///////////////////////////////////////////////////////////////////////////
	char mapname[24];
	struct mapgat	*gat;	// NULLなら下のmap_data_other_serverとして扱う
	///////////////////////////////////////////////////////////////////////////

	struct _objects
	{
		block_list*	root_blk;
		block_list*	root_mob;
		uint				cnt_blk;
		uint				cnt_mob;
		_objects() :
			root_blk(NULL),
			root_mob(NULL),
			cnt_blk(0),
			cnt_mob(0)
		{}
	} *objects;
	int m;
	unsigned short xs;
	unsigned short ys;
	unsigned short bxs;
	unsigned short bys;
	int wh;
	size_t npc_num;
	size_t users;
	struct mapflags
	{
		unsigned nomemo : 1;					//  0
		unsigned noteleport : 1;				//  1
		unsigned noreturn : 1;					//  2
		unsigned monster_noteleport : 1;		//  3
		unsigned nosave : 1;					//  4
		unsigned nobranch : 1;					//  5
		unsigned nopenalty : 1;					//  6
		unsigned pvp : 1;						//  7 (byte 1)
		unsigned pvp_noparty : 1;				//  8
		unsigned pvp_noguild : 1;				//  9
		unsigned pvp_nightmaredrop :1;			// 10
		unsigned pvp_nocalcrank : 1;			// 11
		unsigned gvg : 1;						// 12
		unsigned gvg_noparty : 1;				// 13
		unsigned gvg_dungeon : 1;				// 14
		unsigned nozenypenalty : 1;				// 15 (byte 2)
		unsigned notrade : 1;					// 16
		unsigned noskill : 1;					// 17
		unsigned nowarp : 1;					// 18
		unsigned nowarpto : 1;					// 19
		unsigned nopvp : 1;						// 20
		unsigned noicewall : 1;					// 21
		unsigned snow : 1;						// 22
		unsigned rain : 1;						// 23 (byte 3)
		unsigned sakura : 1;					// 24
		unsigned leaves : 1;					// 25
		unsigned clouds : 1;					// 26
		unsigned clouds2 : 1;					// 27
		unsigned fog : 1;						// 28
		unsigned fireworks : 1;					// 29
		unsigned indoors : 1;					// 30
		unsigned nogo : 1;						// 31 (byte 4)
		unsigned nobaseexp	: 1;				// 32
		unsigned nojobexp	: 1;				// 33
		unsigned nomobloot	: 1;				// 34
		unsigned nomvploot	: 1;				// 35
		unsigned _unused : 4;					// 36-39 (byte 5)
	} flag;
	struct point nosave;
	npc_data *npc[MAX_NPC_PER_MAP];
	struct
	{
		int drop_id;
		int drop_type;
		int drop_per;
	}
	drop_list[MAX_DROP_PER_MAP];

	struct mob_list *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int mob_delete_timer;	// [Skotlex]
};

struct map_data_other_server
{
	///////////////////////////////////////////////////////////////////////////
	char name[24];
	struct mapgat *gat;	// NULL固定にして判断
	///////////////////////////////////////////////////////////////////////////
	basics::ipset mapset;
	struct map_data* map;

	map_data_other_server() : 
		gat(NULL),
		map(NULL)
	{
		name[0]=0;
	}
};


enum {
	SP_SPEED,SP_BASEEXP,SP_JOBEXP,SP_KARMA,SP_MANNER,SP_HP,SP_MAXHP,SP_SP,	// 0-7
	SP_MAXSP,SP_STATUSPOINT,SP_0a,SP_BASELEVEL,SP_SKILLPOINT,SP_STR,SP_AGI,SP_VIT,	// 8-15
	SP_INT,SP_DEX,SP_LUK,SP_CLASS,SP_ZENY,SP_SEX,SP_NEXTBASEEXP,SP_NEXTJOBEXP,	// 16-23
	SP_WEIGHT,SP_MAXWEIGHT,SP_1a,SP_1b,SP_1c,SP_1d,SP_1e,SP_1f,	// 24-31
	SP_USTR,SP_UAGI,SP_UVIT,SP_UINT,SP_UDEX,SP_ULUK,SP_26,SP_27,	// 32-39
	SP_28,SP_ATK1,SP_ATK2,SP_MATK1,SP_MATK2,SP_DEF1,SP_DEF2,SP_MDEF1,	// 40-47
	SP_MDEF2,SP_HIT,SP_FLEE1,SP_FLEE2,SP_CRITICAL,SP_ASPD,SP_36,SP_JOBLEVEL,	// 48-55
	SP_UPPER,SP_PARTNER,SP_CART,SP_FAME,SP_UNBREAKABLE,	//56-60
	SP_CHAOS, // 61
	SP_CARTINFO=99,	// 99

	SP_BASEJOB=119,	// 100+19 - celest

	// original 1000-
	SP_ATTACKRANGE=1000,	SP_ATKELE,SP_DEFELE,	// 1000-1002
	SP_CASTRATE, SP_MAXHPRATE, SP_MAXSPRATE, SP_SPRATE, // 1003-1006
	SP_ADDELE, SP_ADDRACE, SP_ADDSIZE, SP_SUBELE, SP_SUBRACE, // 1007-1011
	SP_ADDEFF, SP_RESEFF,	// 1012-1013
	SP_BASE_ATK,SP_ASPD_RATE,SP_HP_RECOV_RATE,SP_SP_RECOV_RATE,SP_SPEED_RATE, // 1014-1018
	SP_CRITICAL_DEF,SP_NEAR_ATK_DEF,SP_LONG_ATK_DEF, // 1019-1021
	SP_DOUBLE_RATE, SP_DOUBLE_ADD_RATE, SP_MATK, SP_MATK_RATE, // 1022-1025
	SP_IGNORE_DEF_ELE,SP_IGNORE_DEF_RACE, // 1026-1027
	SP_ATK_RATE,SP_SPEED_ADDRATE,SP_ASPD_ADDRATE, // 1028-1030
	SP_MAGIC_ATK_DEF,SP_MISC_ATK_DEF, // 1031-1032
	SP_IGNORE_MDEF_ELE,SP_IGNORE_MDEF_RACE, // 1033-1034
	SP_MAGIC_ADDELE,SP_MAGIC_ADDRACE,SP_MAGIC_SUBRACE, // 1035-1037
	SP_PERFECT_HIT_RATE,SP_PERFECT_HIT_ADD_RATE,SP_CRITICAL_RATE,SP_GET_ZENY_NUM,SP_ADD_GET_ZENY_NUM, // 1038-1042
	SP_ADD_DAMAGE_CLASS,SP_ADD_MAGIC_DAMAGE_CLASS,SP_ADD_DEF_CLASS,SP_ADD_MDEF_CLASS, // 1043-1046
	SP_ADD_MONSTER_DROP_ITEM,SP_DEF_RATIO_ATK_ELE,SP_DEF_RATIO_ATK_RACE,SP_ADD_SPEED, // 1047-1050
	SP_HIT_RATE,SP_FLEE_RATE,SP_FLEE2_RATE,SP_DEF_RATE,SP_DEF2_RATE,SP_MDEF_RATE,SP_MDEF2_RATE, // 1051-1057
	SP_SPLASH_RANGE,SP_SPLASH_ADD_RANGE,SP_AUTOSPELL,SP_HP_DRAIN_RATE,SP_SP_DRAIN_RATE, // 1058-1062
	SP_SHORT_WEAPON_DAMAGE_RETURN,SP_LONG_WEAPON_DAMAGE_RETURN,SP_WEAPON_COMA_ELE,SP_WEAPON_COMA_RACE, // 1063-1066
	SP_ADDEFF2,SP_BREAK_WEAPON_RATE,SP_BREAK_ARMOR_RATE,SP_ADD_STEAL_RATE, // 1067-1070
	SP_MAGIC_DAMAGE_RETURN,SP_RANDOM_ATTACK_INCREASE,SP_ALL_STATS,SP_AGI_VIT,SP_AGI_DEX_STR,SP_PERFECT_HIDE, // 1071-1076
	SP_DISGUISE,SP_CLASSCHANGE, // 1077-1078
	SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1079-1080
	SP_WEAPON_ATK,SP_WEAPON_ATK_RATE, // 1081-1082
	SP_DELAYRATE,	// 1083
	
	SP_RESTART_FULL_RECOVER=2000,SP_NO_CASTCANCEL,SP_NO_SIZEFIX,SP_NO_MAGIC_DAMAGE,SP_NO_WEAPON_DAMAGE,SP_NO_GEMSTONE, // 2000-2005
	SP_NO_CASTCANCEL2,SP_INFINITE_ENDURE,SP_UNBREAKABLE_WEAPON,SP_UNBREAKABLE_ARMOR, SP_UNBREAKABLE_HELM, // 2006-2010
	SP_UNBREAKABLE_SHIELD, SP_LONG_ATK_RATE, // 2011-2012

	SP_CRIT_ATK_RATE, SP_CRITICAL_ADDRACE, SP_NO_REGEN, SP_ADDEFF_WHENHIT, SP_AUTOSPELL_WHENHIT, // 2013-2017
	SP_SKILL_ATK, SP_UNSTRIPABLE, SP_ADD_DAMAGE_BY_CLASS, // 2018-2020
	SP_SP_GAIN_VALUE, SP_IGNORE_DEF_MOB, SP_HP_LOSS_RATE, SP_ADDRACE2, SP_HP_GAIN_VALUE, // 2021-2025
	SP_SUBSIZE, SP_DAMAGE_WHEN_UNEQUIP, SP_ADD_ITEM_HEAL_RATE, SP_LOSESP_WHEN_UNEQUIP, SP_EXP_ADDRACE,	// 2026-2030
	SP_SP_GAIN_RACE, SP_SUBRACE2, SP_ADDEFF_WHENHIT_SHORT,	// 2031-2033
	SP_UNSTRIPABLE_WEAPON,SP_UNSTRIPABLE_ARMOR,SP_UNSTRIPABLE_HELM,SP_UNSTRIPABLE_SHIELD,  // 2034-2037
	SP_INTRAVISION, SP_ADD_MONSTER_DROP_ITEMGROUP, SP_SP_LOSS_RATE // 2038-2040
};

enum {
	LOOK_BASE,LOOK_HAIR,LOOK_WEAPON,LOOK_HEAD_BOTTOM,LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HAIR_COLOR,LOOK_CLOTHES_COLOR,LOOK_SHIELD,LOOK_SHOES
};



extern struct map_data maps[];
extern size_t map_num;
extern int autosave_interval;
extern int agit_flag;


extern int map_read_flag; // 0: grfｫﾕｫ｡ｫ､ｫ・1: ｫｭｫ罩ﾃｫｷｫ・2: ｫｭｫ罩ﾃｫｷｫ・?)
enum {
	READ_FROM_GAT, 
	READ_FROM_BITMAP, 
	READ_FROM_BITMAP_COMPRESSED
};

extern char motd_txt[];
extern char help_txt[];

extern char talkie_mes[];

extern char wisp_server_name[];


///////////////////////////////////////////////////////////////////////////////
// gat?ﾖｧ
int map_getcell(unsigned short m,unsigned short x, unsigned short y,cell_t cellchk);
int map_getcellp(struct map_data& m,unsigned short x, unsigned short y,cell_t cellchk);
void map_setcell(unsigned short m,unsigned short x, unsigned short y,int cellck);





///////////////////////////////////////////////////////////////////////////////

// 鯖全体情報
void map_setusers(int fd);
int map_getusers(void);








// 一時的object関連
int map_addobject(block_list &bl);
int map_delobject(int);
int map_delobjectnofree(int id);

//
int map_quit(map_session_data &sd);
// npc
int map_addnpc(unsigned short m, npc_data *nd);

// 床アイテム関連
int map_clearflooritem_timer(int tid, unsigned long tick, int id, basics::numptr data);
int map_removemobs_timer(int tid, unsigned long tick, int id, basics::numptr data);
#define map_clearflooritem(id) map_clearflooritem_timer(0,0,id,1)
int map_addflooritem(const struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y, map_session_data *first_sd, map_session_data *second_sd, map_session_data *third_sd,int type);
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range);


int map_mapname2mapid(const char *name);
bool map_mapname2ipport(const char *name, basics::ipset &mapset);
int map_setipport(const char *name, basics::ipset &mapset);
int map_eraseipport(const char *name, basics::ipset &mapset);
int map_eraseallipport(void);






void map_helpscreen(); // [Valaris]
int map_delmap(const char *mapname);

struct mob_list* map_addmobtolist(unsigned short m);	// [Wizputer]
void clear_moblist(unsigned short m);
void map_spawnmobs(unsigned short m);		// [Wizputer]
void map_removemobs(unsigned short m);		// [Wizputer]

extern const char *LOG_CONF_NAME;
extern const char *MAP_CONF_NAME;
extern const char *BATTLE_CONF_FILENAME;
extern const char *COMMAND_CONF_FILENAME;
extern const char *SCRIPT_CONF_NAME;
extern const char *MSG_CONF_NAME;
extern const char *GRF_PATH_FILENAME;







///////////////////////////////////////////////////////////////////////////////
// day-night cycle
extern int daynight_flag;		// 0=day, 1=night
extern int daynight_timer_tid;	// timer for night.day
int map_daynight_timer(int tid, unsigned long tick, int id, basics::numptr data);



///////////////////////////////////////////////////////////////////////////////
// permanent name storage
const char*	map_src_namedb(uint32 charid);
bool		map_req_namedb(const map_session_data &sd, uint32 charid);
void		map_add_namedb(uint32 charid, const char *name);

#endif

