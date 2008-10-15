// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/utils.h"
#include "char.h"
#include "inter.h"
#include "int_pet.h"

#ifdef TXT_ONLY
#include "../common/db.h"
#include "../common/lock.h"
#else
#include "../common/sql.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------
int inter_pet_tosql(int pet_id, struct s_pet* p)
{
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	char esc_name[NAME_LENGTH*2+1];// escaped pet name

	Sql_EscapeStringLen(sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
	p->hungry = cap_value(p->hungry, 0, 100);
	p->intimate = cap_value(p->intimate, 0, 1000);

	if( pet_id == -1 )
	{// New pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` "
			"(`class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`) "
			"VALUES ('%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
		p->pet_id = (int)Sql_LastInsertId(sql_handle);
	}
	else
	{// Update pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%d'",
			pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate, p->pet_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
	}

	if (save_log)
		ShowInfo("Pet saved %d - %s.\n", pet_id, p->name);
	return 1;
}

int inter_pet_fromsql(int pet_id, struct s_pet* p)
{
	char* data;
	size_t len;

#ifdef NOISY
	ShowInfo("Loading pet (%d)...\n",pet_id);
#endif
	memset(p, 0, sizeof(struct s_pet));

	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate` FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		p->pet_id = pet_id;
		Sql_GetData(sql_handle,  1, &data, NULL); p->class_ = atoi(data);
		Sql_GetData(sql_handle,  2, &data, &len); memcpy(p->name, data, min(len, NAME_LENGTH));
		Sql_GetData(sql_handle,  3, &data, NULL); p->account_id = atoi(data);
		Sql_GetData(sql_handle,  4, &data, NULL); p->char_id = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); p->level = atoi(data);
		Sql_GetData(sql_handle,  6, &data, NULL); p->egg_id = atoi(data);
		Sql_GetData(sql_handle,  7, &data, NULL); p->equip = atoi(data);
		Sql_GetData(sql_handle,  8, &data, NULL); p->intimate = atoi(data);
		Sql_GetData(sql_handle,  9, &data, NULL); p->hungry = atoi(data);
		Sql_GetData(sql_handle, 10, &data, NULL); p->rename_flag = atoi(data);
		Sql_GetData(sql_handle, 11, &data, NULL); p->incuvate = atoi(data);

		Sql_FreeResult(sql_handle);

		p->hungry = cap_value(p->hungry, 0, 100);
		p->intimate = cap_value(p->intimate, 0, 1000);

		if( save_log )
			ShowInfo("Pet loaded (%d - %s).\n", pet_id, p->name);
	}
	return 0;
}
//----------------------------------------------

int inter_pet_init(void)
{
	return 0;
}
void inter_pet_final(void)
{
	return;
}

//----------------------------------
int inter_pet_delete(int pet_id)
{
	ShowInfo("delete pet request: %d...\n",pet_id);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
		Sql_ShowDebug(sql_handle);
	return 0;
}

//------------------------------------------------------
int mapif_pet_created(int fd, int account_id, struct s_pet *p)
{
	WFIFOHEAD(fd, 11);
	WFIFOW(fd,0) = 0x3880;
	WFIFOL(fd,2) = account_id;
	if(p!=NULL){
		WFIFOB(fd,6) = 0;
		WFIFOL(fd,7) = p->pet_id;
		ShowInfo("int_pet: created pet %d - %s\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd,6) = 1;
		WFIFOL(fd,7) = 0;
	}
	WFIFOSET(fd, 11);

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

	strncpy(pd.name, pet_name, NAME_LENGTH);
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

	pd.pet_id = -1; //Signal NEW pet.
	if( inter_pet_tosql(pd.pet_id, &pd) )
		mapif_pet_created(fd, account_id, &pd);
	else	//Failed...
		mapif_pet_created(fd, account_id, NULL);

	return 0;
}

int mapif_load_pet(int fd, int account_id, int char_id, int pet_id)
{
	struct s_pet pd;
	memset(&pd, 0, sizeof(pd));

	inter_pet_fromsql(pet_id, &pd);

	if( pd.incuvate == 1 )
	{
		pd.account_id = pd.char_id = 0;
		mapif_pet_info(fd, account_id, &pd);
	}
	else
	if(account_id == pd.account_id && char_id == pd.char_id)
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
	else
	{
#ifdef TXT_ONLY
		struct s_pet *p;
		int pet_id;

		pet_id = data->pet_id;
		if (pet_id == 0)
			pet_id = data->pet_id = pet_newid++;
		p = (struct s_pet*)idb_ensure(pet_db,pet_id,create_pet);
#endif

		data->hungry = cap_value(data->hungry, 0, 100);
		data->intimate = cap_value(data->intimate, 0, 1000);

#ifdef TXT_ONLY
		memcpy(p,data,sizeof(struct s_pet));
		if(p->incuvate == 1)
			p->account_id = p->char_id = 0;
#else
		inter_pet_tosql(data->pet_id,data);
#endif

		mapif_save_pet_ack(fd, account_id, 0);
	}

	return 0;
}

int mapif_delete_pet(int fd, int pet_id)
{
	mapif_delete_pet_ack(fd, inter_pet_delete(pet_id));

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
