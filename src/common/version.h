// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _VERSION_H_
#define _VERSION_H_

#define ATHENA_MAJOR_VERSION	1	// Major Version
#define ATHENA_MINOR_VERSION	0	// Minor Version
#define ATHENA_REVISION			0	// Revision

#define ATHENA_RELEASE_FLAG		1	// 1=Develop,0=Stable
#define ATHENA_OFFICIAL_FLAG	1	// 1=Mod,0=Official


// ATHENA_MOD_VERSIONはパッチ番号です。
// これは無理に変えなくても気が向いたら変える程度の扱いで。
// （毎回アップロードの度に変更するのも面倒と思われるし、そもそも
// 　この項目を参照する人がいるかどうかで疑問だから。）
// その程度の扱いなので、サーバーに問い合わせる側も、あくまで目安程度の扱いで
// あんまり信用しないこと。
// 鯖snapshotの時や、大きな変更があった場合は設定してほしいです。
// C言語の仕様上、最初に0を付けると8進数になるので間違えないで下さい。
#define ATHENA_MOD_VERSION	1249	// mod version (patch No.)

#define ATHENA_SVN_VERSION	2001	// svn version

typedef enum 
{
	ATHENA_SERVER_NONE	=	0x00,	// not defined
	ATHENA_SERVER_LOGIN	=	0x01,	// login server
	ATHENA_SERVER_CHAR	=	0x02,	// char server
	ATHENA_SERVER_INTER	=	0x04,	// inter server
	ATHENA_SERVER_MAP	=	0x08,	// map server
	ATHENA_SERVER_CORE	=	0x10,	// core component
	ATHENA_SERVER_xxx1	=	0x20,
	ATHENA_SERVER_xxx2	=	0x40,
	ATHENA_SERVER_xxx3	=	0x80,
	ATHENA_SERVER_ALL	=	0xFF
} ServerType;



#endif
