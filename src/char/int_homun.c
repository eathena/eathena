// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "char.h"
#include "homundb.h"
#include "inter.h"
#include "int_homun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// homun database
static HomunDB* homuns = NULL;


static void mapif_homunculus_created(int fd, int account_id, struct s_homunculus *sh, unsigned char flag)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3890;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = flag;
	memcpy(WFIFOP(fd,9), sh, sizeof(struct s_homunculus));
	WFIFOSET(fd, WFIFOW(fd,2));
}

static void mapif_homunculus_loaded(int fd, int account_id, struct s_homunculus *hd)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3891;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	if( hd != NULL )
	{
		WFIFOB(fd,8) = 1; // success
		memcpy(WFIFOP(fd,9), hd, sizeof(struct s_homunculus));
	}
	else
	{
		WFIFOB(fd,8) = 0; // not found.
		memset(WFIFOP(fd,9), 0, sizeof(struct s_homunculus));
	}
	WFIFOSET(fd, sizeof(struct s_homunculus)+9);
}

static void mapif_homunculus_saved(int fd, int account_id, bool flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0) = 0x3892;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag; // 1:success, 0:failure
	WFIFOSET(fd, 7);
}

static void mapif_homunculus_deleted(int fd, int flag)
{
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) = 0x3893;
	WFIFOB(fd,2) = flag; //Flag 1 = success
	WFIFOSET(fd, 3);
}

static void mapif_homunculus_renamed(int fd, int account_id, int char_id, unsigned char flag, char* name)
{
	WFIFOHEAD(fd, NAME_LENGTH+12);
	WFIFOW(fd, 0) = 0x3894;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = flag;
	safestrncpy((char*)WFIFOP(fd,11), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+12);
}

// tests if 'name' is an allowed homunculus name
bool mapif_homunculus_rename(char *name)
{
	int i;

	// Check Authorised letters/symbols in the name of the homun
	if( char_config.char_name_option == 1 )
	{// only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_config.char_name_letters, name[i]) == NULL )
				return false;
	} else
	if( char_config.char_name_option == 2 )
	{// letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_config.char_name_letters, name[i]) != NULL )
				return false;
	}

	return true;
}


static void mapif_parse_homunculus_create(int fd, int len, int account_id, struct s_homunculus* hd)
{
	bool result = homuns->create(homuns, hd);
	mapif_homunculus_created(fd, account_id, hd, result);
}

static void mapif_parse_homunculus_load(int fd, int account_id, int homun_id)
{
	struct s_homunculus hd;
	bool result = homuns->load(homuns, &hd, homun_id);
	mapif_homunculus_loaded(fd, account_id, ( result ? &hd : NULL ));
}

static void mapif_parse_homunculus_save(int fd, int len, int account_id, struct s_homunculus* hd)
{
	bool result = homuns->save(homuns, hd);
	mapif_homunculus_saved(fd, account_id, result);
}

static void mapif_parse_homunculus_delete(int fd, int homun_id)
{
	bool result = homuns->remove(homuns, homun_id);
	mapif_homunculus_deleted(fd, result);
}

static void mapif_parse_homunculus_rename(int fd, int account_id, int char_id, char* name)
{
	bool result = mapif_homunculus_rename(name);
	mapif_homunculus_renamed(fd, account_id, char_id, result, name);
}

int inter_homun_parse_frommap(int fd)
{
	switch( RFIFOW(fd,0) )
	{
	case 0x3090: mapif_parse_homunculus_create(fd, (int)RFIFOW(fd,2), (int)RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,8)); break;
	case 0x3091: mapif_parse_homunculus_load  (fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
	case 0x3092: mapif_parse_homunculus_save  (fd, (int)RFIFOW(fd,2), (int)RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,8)); break;
	case 0x3093: mapif_parse_homunculus_delete(fd, (int)RFIFOL(fd,2)); break;
	case 0x3094: mapif_parse_homunculus_rename(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6), (char*)RFIFOP(fd,10)); break;
	default:
		return 0;
	}
	return 1;
}

void inter_homun_init(HomunDB* db)
{
	homuns = db;
}

void inter_homun_final()
{
	homuns = NULL;
}
