#ifndef _STATUS_H_
#define _STATUS_H_

#include "map.h" // for basic type definitions, move to a seperated file


enum {	// struct map_session_data の status_changeの番?テ?ブル
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
	SC_STAN				= 76,
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
	SC_STAN					=130,	// 状態異常：スタン 
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
extern int SkillStatusChangeTable[];

extern int current_equip_item_index;


int status_recalc_speed(block_list *bl);

// パラメータ所得系 battle.c より移動
int status_get_class(block_list *bl);
int status_get_lv(const block_list *bl);
int status_get_range(const block_list *bl);
int status_get_hp(block_list *bl);
int status_get_max_hp(block_list *bl);
int status_get_str(block_list *bl);
int status_get_agi(block_list *bl);
int status_get_vit(block_list *bl);
int status_get_int(block_list *bl);
int status_get_dex(block_list *bl);
int status_get_luk(block_list *bl);
int status_get_hit(block_list *bl);
int status_get_flee(block_list *bl);
int status_get_def(block_list *bl);
int status_get_mdef(block_list *bl);
int status_get_flee2(block_list *bl);
int status_get_def2(block_list *bl);
int status_get_mdef2(block_list *bl);
int status_get_baseatk(block_list *bl);
int status_get_atk(block_list *bl);
int status_get_atk2(block_list *bl);
int status_get_adelay(block_list *bl);
int status_get_amotion(const block_list *bl);
int status_get_dmotion(block_list *bl);
int status_get_element(block_list *bl);
int status_get_attack_element(block_list *bl);
int status_get_attack_element2(block_list *bl);  //左手武器属性取得
#define status_get_elem_type(bl)	(status_get_element(bl)%10)
#define status_get_elem_level(bl)	(status_get_element(bl)/10/2)
uint32 status_get_party_id(const block_list *bl);
uint32 status_get_guild_id(const block_list *bl);
int status_get_race(block_list *bl);
int status_get_size(block_list *bl);
int status_get_mode(block_list *bl);
int status_get_mexp(block_list *bl);
int status_get_race2(block_list *bl);

struct status_change *status_get_sc_data(block_list *bl);
short *status_get_opt1(block_list *bl);
short *status_get_opt2(block_list *bl);
short *status_get_opt3(block_list *bl);
short *status_get_option(block_list *bl);

int status_get_matk1(block_list *bl);
int status_get_matk2(block_list *bl);
int status_get_critical(block_list *bl);
int status_get_atk_(block_list *bl);
int status_get_atk_2(block_list *bl);
int status_get_atk2(block_list *bl);

int status_isimmune(block_list *bl);

int status_get_sc_def(block_list *bl, int type);
#define status_get_sc_def_mdef(bl)	(status_get_sc_def(bl, SP_MDEF1))
#define status_get_sc_def_vit(bl)	(status_get_sc_def(bl, SP_DEF2))
#define status_get_sc_def_int(bl)	(status_get_sc_def(bl, SP_MDEF2))
#define status_get_sc_def_luk(bl)	(status_get_sc_def(bl, SP_LUK))

// 状態異常関連 skill.c より移動
int status_change_start(block_list *bl,int type,basics::numptr val1,basics::numptr val2,basics::numptr val3,basics::numptr val4,unsigned long tick,int flag);
int status_change_end(block_list* bl, int type,int tid );
int status_change_timer(int tid, unsigned long tick, int id, basics::numptr data);

class CStatusChangetimer : public CMapProcessor
{
	block_list &src;
	int type;
	unsigned long tick;
public:
	CStatusChangetimer(block_list &s, int ty, unsigned long t)
		: src(s), type(ty), tick(t)
	{}
	~CStatusChangetimer()	{}
	virtual int process(block_list& bl) const;
};

int status_change_clear(block_list *bl,int type);
int status_change_clear_buffs(block_list *bl);
int status_change_clear_debuffs(block_list *bl);

int status_calc_pet(struct map_session_data &sd, bool first); // [Skotlex]
// ステータス計算 pc.c から分離
// pc_calcstatus

int status_calc_pc(struct map_session_data &sd,int first);
int status_calc_speed(struct map_session_data &sd, unsigned short skill_num, unsigned short skill_lv, int start);
int status_calc_speed_old(struct map_session_data &sd); // [Celest]
// int status_calc_skilltree(struct map_session_data *sd);
int status_getrefinebonus(int lv,int type);
int status_percentrefinery(struct map_session_data &sd,struct item &item);
//Use this to refer the max refinery level [Skotlex]

extern int percentrefinery[MAX_REFINE_BONUS][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

int status_readdb(void);
int do_init_status(void);

#endif
