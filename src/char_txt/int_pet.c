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

char pet_txt[1024]="save/pet.txt";

static DBMap* pet_db; // int pet_id -> struct s_pet*
static int pet_newid = 100;

int inter_pet_tostr(char *str,struct s_pet *p)
{
	int len;

	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;

	len=sprintf(str,"%d,%d,%s\t%d,%d,%d,%d,%d,%d,%d,%d,%d",
		p->pet_id,p->class_,p->name,p->account_id,p->char_id,p->level,p->egg_id,
		p->equip,p->intimate,p->hungry,p->rename_flag,p->incuvate);

	return 0;
}

int inter_pet_fromstr(char *str,struct s_pet *p)
{
	int s;
	int tmp_int[16];
	char tmp_str[256];

	memset(p,0,sizeof(struct s_pet));

	s=sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
		&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

	if(s!=12)
		return 1;

	p->pet_id = tmp_int[0];
	p->class_ = tmp_int[1];
	memcpy(p->name,tmp_str,NAME_LENGTH);
	p->account_id = tmp_int[2];
	p->char_id = tmp_int[3];
	p->level = tmp_int[4];
	p->egg_id = tmp_int[5];
	p->equip = tmp_int[6];
	p->intimate = tmp_int[7];
	p->hungry = tmp_int[8];
	p->rename_flag = tmp_int[9];
	p->incuvate = tmp_int[10];

	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;

	return 0;
}

int inter_pet_init()
{
	char line[8192];
	struct s_pet *p;
	FILE *fp;
	int c=0;

	pet_db= idb_alloc(DB_OPT_RELEASE_DATA);

	if( (fp=fopen(pet_txt,"r"))==NULL )
		return 1;
	while(fgets(line, sizeof(line), fp))
	{
		p = (struct s_pet*)aCalloc(sizeof(struct s_pet), 1);
		if(p==NULL){
			ShowFatalError("int_pet: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		memset(p,0,sizeof(struct s_pet));
		if(inter_pet_fromstr(line,p)==0 && p->pet_id>0){
			if( p->pet_id >= pet_newid)
				pet_newid=p->pet_id+1;
			idb_put(pet_db,p->pet_id,p);
		}else{
			ShowError("int_pet: broken data [%s] line %d\n",pet_txt,c);
			aFree(p);
		}
		c++;
	}
	fclose(fp);
	return 0;
}

void inter_pet_final()
{
	pet_db->destroy(pet_db, NULL);
	return;
}

int inter_pet_save()
{
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(pet_txt, &lock);
	if( fp == NULL )
	{
		ShowError("int_pet: can't write [%s] !!! data is lost !!!\n", pet_txt);
		return 1;
	}

	iter = pet_db->iterator(pet_db);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct s_pet* pd = (struct s_pet*) data;
		char line[8192];

		inter_pet_tostr(line, pd);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, pet_txt, &lock);

	return 0;
}

int inter_pet_delete(int pet_id)
{
	struct s_pet *p;
	p = (struct s_pet*)idb_get(pet_db,pet_id);
	if( p == NULL)
		return 1;
	else {
		idb_remove(pet_db,pet_id);
		ShowInfo("Deleted pet (pet_id: %d)\n",pet_id);
	}
	return 0;
}

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

int mapif_pet_noinfo(int fd,int account_id)
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
	struct s_pet *p;
	p= (struct s_pet *) aCalloc(sizeof(struct s_pet), 1);
	if(p==NULL){
		ShowFatalError("int_pet: out of memory !\n");
		mapif_pet_created(fd,account_id,NULL);
		return 0;
	}

	memcpy(p->name,pet_name,NAME_LENGTH);
	if(incuvate == 1)
	{
		p->account_id = 0;
		p->char_id = 0;
	}
	else
	{
		p->account_id = account_id;
		p->char_id = char_id;
	}
	p->class_ = pet_class;
	p->level = pet_lv;
	p->egg_id = pet_egg_id;
	p->equip = pet_equip;
	p->intimate = intimate;
	p->hungry = hungry;
	p->rename_flag = rename_flag;
	p->incuvate = incuvate;

	p->hungry = cap_value(p->hungry, 0, 100);
	p->intimate = cap_value(p->intimate, 0, 1000);

	p->pet_id = pet_newid++;
	idb_put(pet_db,p->pet_id,p);

	mapif_pet_created(fd,account_id,p);

	return 0;
}

int mapif_load_pet(int fd, int account_id, int char_id, int pet_id)
{
	struct s_pet *p;
	p = (struct s_pet*)idb_get(pet_db,pet_id);
	if(p ==NULL) {
		mapif_pet_noinfo(fd,account_id);
		return 0;
	}

	if( p->incuvate == 1 )
	{
		p->account_id = p->char_id = 0;
		mapif_pet_info(fd, account_id, p);
	}
	else
	if(account_id == p->account_id && char_id == p->char_id)
		mapif_pet_info(fd, account_id, p);
	else
		mapif_pet_noinfo(fd, account_id);

	return 0;
}

static void* create_pet(DBKey key, va_list args)
{
	struct s_pet *p;
	p=(struct s_pet *)aCalloc(sizeof(struct s_pet),1);
	p->pet_id = key.i;
	return p;
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
	mapif_save_pet(fd, RFIFOL(fd,4), (struct s_pet *)RFIFOP(fd,8));
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
