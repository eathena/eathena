// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "char.h"
#include "inter.h"
#include "int_pet.h"
#include "petdb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// pet database
static PetDB* pets = NULL;


int mapif_pet_created(int fd, int account_id, struct s_pet *p)
{
	WFIFOHEAD(fd, 11);
	WFIFOW(fd,0) = 0x3880;
	WFIFOL(fd,2) = account_id;
	if(p!=NULL){
		WFIFOB(fd,6) = 0;
		WFIFOL(fd,7) = p->pet_id;
		ShowInfo("Created pet (%d - %s)\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd,6) = 1;
		WFIFOL(fd,7) = 0;
	}
	WFIFOSET(fd,11);

	return 0;
}

int mapif_pet_info(int fd, int account_id, struct s_pet *p)
{
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd,0) = 0x3881;
	WFIFOW(fd,2) = sizeof(struct s_pet) + 9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = 0;
	memcpy(WFIFOP(fd,9), p, sizeof(struct s_pet));
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int mapif_pet_noinfo(int fd, int account_id)
{
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd,0) = 0x3881;
	WFIFOW(fd,2) = sizeof(struct s_pet) + 9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = 1;
	memset(WFIFOP(fd,9), 0, sizeof(struct s_pet));
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int mapif_save_pet_ack(int fd, int account_id, int flag)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3882;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag;
	WFIFOSET(fd,7);

	return 0;
}

int mapif_delete_pet_ack(int fd, int flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3883;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);

	return 0;
}

int mapif_create_pet(int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id,
	short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, char *pet_name)
{
	struct s_pet pd;
	memset(&pd, 0, sizeof(pd));

	safestrncpy(pd.name, pet_name, NAME_LENGTH);
	if(incuvate == 1)
	{
		pd.account_id = 0;
		pd.char_id = 0;
	}
	else
	{
		pd.account_id = account_id;
		pd.char_id = char_id;
	}
	pd.class_ = pet_class;
	pd.level = pet_lv;
	pd.egg_id = pet_egg_id;
	pd.equip = pet_equip;
	pd.intimate = intimate;
	pd.hungry = hungry;
	pd.rename_flag = rename_flag;
	pd.incuvate = incuvate;

	pd.hungry = cap_value(pd.hungry, 0, 100);
	pd.intimate = cap_value(pd.intimate, 0, 1000);

	//FIXME: why 'account_id' ???
	pd.pet_id = -1; //Signal NEW pet.
	if( pets->create(pets, &pd) )
		mapif_pet_created(fd, account_id, &pd);
	else	//Failed...
		mapif_pet_created(fd, account_id, NULL);

	return 0;
}

int mapif_load_pet(int fd, int account_id, int char_id, int pet_id)
{
	struct s_pet pd;

	if( !pets->load(pets, &pd, pet_id) )
	{
		mapif_pet_noinfo(fd, account_id);
		return 0;
	}

	if( pd.incuvate == 1 )
	{
		pd.account_id = pd.char_id = 0;
		mapif_pet_info(fd, account_id, &pd);
	}
	else
	if( account_id == pd.account_id && char_id == pd.char_id )
		mapif_pet_info(fd, account_id, &pd);
	else
		mapif_pet_noinfo(fd, account_id);

	return 0;
}

int mapif_save_pet(int fd, int account_id, struct s_pet *data)
{
	int len = RFIFOW(fd,2);
	if( sizeof(struct s_pet) != len - 8 )
	{
		ShowError("inter pet: data size error %d %d\n", sizeof(struct s_pet), len - 8);
		return 0;
	}

	data->hungry = cap_value(data->hungry, 0, 100);
	data->intimate = cap_value(data->intimate, 0, 1000);

	//FIXME: suspicious code
#ifdef TXT_ONLY
	if( data->incuvate == 1 )
		data->account_id = data->char_id = 0;
#endif

	pets->save(pets, data);

	mapif_save_pet_ack(fd, account_id, 0);

	return 0;
}

int mapif_delete_pet(int fd, int pet_id)
{
	if( pets->remove(pets, pet_id) )
		mapif_delete_pet_ack(fd, 0);
	else
		mapif_delete_pet_ack(fd, 1);

	return 0;
}

int mapif_parse_CreatePet(int fd)
{
	mapif_create_pet(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12), RFIFOW(fd,14), RFIFOW(fd,16), RFIFOW(fd,18),
		RFIFOW(fd,20), RFIFOB(fd,22), RFIFOB(fd,23), (char*)RFIFOP(fd,24));
	return 0;
}

int mapif_parse_LoadPet(int fd)
{
	mapif_load_pet(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
	return 0;
}

int mapif_parse_SavePet(int fd)
{
	mapif_save_pet(fd, RFIFOL(fd,4), (struct s_pet *) RFIFOP(fd,8));
	return 0;
}

int mapif_parse_DeletePet(int fd)
{
	mapif_delete_pet(fd, RFIFOL(fd,2));
	return 0;
}

// communication with map-server
int inter_pet_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x3080: mapif_parse_CreatePet(fd); break;
	case 0x3081: mapif_parse_LoadPet(fd); break;
	case 0x3082: mapif_parse_SavePet(fd); break;
	case 0x3083: mapif_parse_DeletePet(fd); break;
	default:
		return 0;
	}
	return 1;
}

bool inter_pet_delete(int pet_id)
{
	return pets->remove(pets, pet_id);
}

void inter_pet_init(PetDB* db)
{
	pets = db;
}

void inter_pet_final(void)
{
	pets = NULL;
}
