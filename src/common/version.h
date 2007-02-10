// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _VERSION_H_
#define _VERSION_H_

//##TODO: find some way to platformindependendly generate these defines


#define ATHENA_MAJOR_VERSION	1	// Major Version
#define ATHENA_MINOR_VERSION	0	// Minor Version
#define ATHENA_REVISION			0	// Revision

#define ATHENA_RELEASE_FLAG		1	// 1=Develop,0=Stable
#define ATHENA_OFFICIAL_FLAG	1	// 1=Mod,0=Official

#define ATHENA_MOD_VERSION	1249	// mod version (patch No.)

// does come with configure
#ifndef SVNREVISION
#define ATHENA_MOD_STRING	"Shinomori's Modified Version (2007-02-07)"
#else
#define STRINGIFY_TWOLEVEL(s) STRINGIFY(s)
#define STRINGIFY(s) #s
#define ATHENA_MOD_STRING "Shinomori's Modified Version svn-rev: " STRINGIFY_TWOLEVEL(SVNREVISION)
#endif


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
} server_t;



#endif
