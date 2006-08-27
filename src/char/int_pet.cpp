// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "baseio.h"
#include "mmo.h"
#include "socket.h"
#include "db.h"
#include "lock.h"
#include "malloc.h"
#include "showmsg.h"
#include "utils.h"


#include "char.h"
#include "inter.h"
#include "int_pet.h"

CPetDB			cPetDB;
CHomunculusDB	cHomunculusDB;

int inter_pet_init()
{
	cPetDB.init(CHAR_CONF_NAME);
	cHomunculusDB.init(CHAR_CONF_NAME);
	return 0;
}

int inter_pet_final ()
{
	return 0;
}


int inter_pet_delete(uint32 pet_id)
{
	return cPetDB.removePet(pet_id);
}

int mapif_pet_created(int fd,uint32 account_id, CPet& pet)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3880;
		WFIFOL(fd,2)=account_id;
		WFIFOB(fd,6)=0;
		WFIFOL(fd,7)=pet.pet_id;
		WFIFOSET(fd,11);
		ShowMessage("int_pet: created! %d %s\n",pet.pet_id,pet.name);
	}
	return 0;
}

int mapif_pet_info(int fd, uint32 account_id, const CPet &pet)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3881;
		WFIFOW(fd,2)=sizeof(struct petstatus) + 9;
		WFIFOL(fd,4)=account_id;
		WFIFOB(fd,8)=0;
		s_pet_tobuffer(pet, WFIFOP(fd,9));
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

int mapif_pet_noinfo(int fd, uint32 account_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3881;
		WFIFOW(fd,2)=sizeof(struct petstatus) + 9;
		WFIFOL(fd,4)=account_id;
		WFIFOB(fd,8)=1;
		memset(WFIFOP(fd,9),0,sizeof(struct petstatus));
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

int mapif_save_pet_ack(int fd, uint32 account_id, int flag)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3882;
		WFIFOL(fd,2)=account_id;
		WFIFOB(fd,6)=flag;
		WFIFOSET(fd,7);
	}
	return 0;
}

int mapif_delete_pet_ack(int fd,int flag)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0)=0x3883;
		WFIFOB(fd,2)=flag;
		WFIFOSET(fd,3);
	}
	return 0;
}

int mapif_create_pet(int fd,uint32 account_id,uint32 char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incuvate,char *pet_name)
{
	CPet pet;
	cPetDB.insertPet(account_id, char_id, pet_class, pet_lv, pet_egg_id,
		pet_equip, intimate, hungry, rename_flag, incuvate, pet_name, pet);

	mapif_pet_created(fd, account_id, pet);
	return 0;
}

int mapif_load_pet(int fd, uint32 account_id, uint32 char_id, uint32 pet_id)
{
	CPet pet;
	if( cPetDB.searchPet(pet_id, pet) )
	{
		if(pet.incuvate == 1)
		{
			pet.account_id = pet.char_id = 0;
			mapif_pet_info(fd, account_id, pet);
		}
		else if(account_id == pet.account_id && char_id == pet.char_id)
			mapif_pet_info(fd, account_id, pet);
		else
			mapif_pet_noinfo(fd, account_id);
	}
	else
		mapif_pet_noinfo(fd,account_id);

	return 0;
}

int mapif_save_pet(int fd, uint32 account_id, unsigned char* buf)
{
	if( session_isActive(fd) )
	{
		
		int len=RFIFOW(fd,2);
		if(sizeof(struct petstatus)!=len-8)
		{
			ShowMessage("inter pet: data size error %d %d\n",sizeof(struct petstatus),len-8);
		}
		else
		{
			CPet pet;
			s_pet_frombuffer(pet, buf);

			if(pet.hungry < 0)
				pet.hungry = 0;
			else if(pet.hungry > 100)
				pet.hungry = 100;
			if(pet.intimate < 0)
				pet.intimate = 0;
			else if(pet.intimate > 1000)
				pet.intimate = 1000;
			if(pet.incuvate == 1)
				pet.account_id = pet.char_id = 0;
			
			cPetDB.savePet(pet);
			
			mapif_save_pet_ack(fd,account_id,0);
		}
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
	if( session_isActive(fd) )
		mapif_create_pet(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOW(fd,14),RFIFOW(fd,16),
			RFIFOW(fd,18),RFIFOW(fd,20),RFIFOB(fd,22),RFIFOB(fd,23),(char*)RFIFOP(fd,24));
	return 0;
}

int mapif_parse_LoadPet(int fd)
{
	if( session_isActive(fd) )
		mapif_load_pet(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 0;
}

int mapif_parse_SavePet(int fd)
{
	if( session_isActive(fd) )
		mapif_save_pet(fd,RFIFOL(fd,4),RFIFOP(fd,8));
	return 0;
}

int mapif_parse_DeletePet(int fd)
{
	if( session_isActive(fd) )
		mapif_delete_pet(fd,RFIFOL(fd,2));
	return 0;
}



int mapif_parse_CreateHomun(int fd)
{
	if( session_isActive(fd) )
	{
		// insert
		CHomunculus hom;
		homun_frombuffer(hom, RFIFOP(fd,12));

		bool ok = cHomunculusDB.insertHomunculus( hom );

		// send back complete dataset
		const size_t sz=5 + ( (ok)?sizeof(struct homunstatus):0 );
		WFIFOW(fd,0)=0x3889;
		WFIFOW(fd,2)=sz;
		WFIFOB(fd,4)=(ok)?0:1;
		if(ok) homun_tobuffer(hom, WFIFOP(fd,5));
		WFIFOSET(fd,sz);
	}
	return 0;
}
int mapif_parse_LoadHomun(int fd)
{
	if( session_isActive(fd) )
	{
		// search
		CHomunculus hom;
		bool ok = cHomunculusDB.searchHomunculus( RFIFOL(fd,10), hom )
			&& hom.account_id==RFIFOL(fd,2) && hom.char_id==RFIFOL(fd,6);

		// send back complete dataset
		const size_t sz=5 + ( (ok)?sizeof(struct homunstatus):0 );
		WFIFOW(fd,0)=0x3889;
		WFIFOW(fd,2)=sz;
		WFIFOB(fd,4)=(ok)?0:1;
		if(ok) homun_tobuffer(hom, WFIFOP(fd,5));
		WFIFOSET(fd,sz);
	}
	return 0;
}
int mapif_parse_SaveHomun(int fd)
{
	if( session_isActive(fd) )
	{
		// save
		CHomunculus hom;
		homun_frombuffer(hom, RFIFOP(fd,12));
		// update the current owner just to be sure 
		hom.account_id = RFIFOL(fd,4);
		hom.char_id = RFIFOL(fd,8);
		bool ok = cHomunculusDB.saveHomunculus( hom );

		// send back ok/fail
		WFIFOW(fd,0)=0x388a;
		WFIFOB(fd,2)=(ok)?0:1;
		WFIFOSET(fd,3);
	}
	return 0;
}
int mapif_parse_DeleteHomun(int fd)
{
	if( session_isActive(fd) )
	{
		// delete
		bool ok = cHomunculusDB.removeHomunculus( RFIFOL(fd,10) );

		// send back ok/fail
		WFIFOW(fd,0)=0x388b;
		WFIFOB(fd,2)=(ok)?0:1;
		WFIFOSET(fd,3);
	}
	return 0;
}




// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_pet_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOW(fd,0)){
	case 0x3080: mapif_parse_CreatePet(fd); break;
	case 0x3081: mapif_parse_LoadPet(fd); break;
	case 0x3082: mapif_parse_SavePet(fd); break;
	case 0x3083: mapif_parse_DeletePet(fd); break;


	case 0x3088: mapif_parse_CreateHomun(fd); break;
	case 0x3089: mapif_parse_LoadHomun(fd); break;
	case 0x308a: mapif_parse_SaveHomun(fd); break;
	case 0x308b: mapif_parse_DeleteHomun(fd); break;


	default:
		return 0;
	}
	return 1;
}
