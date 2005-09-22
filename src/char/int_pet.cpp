// $Id: int_pet.c,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#include "base.h"
#include "inter.h"
#include "int_pet.h"
#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "db.h"
#include "lock.h"
#include "malloc.h"
#include "showmsg.h"
#include "utils.h"


char pet_txt[1024]="save/pet.txt";

static struct dbt *pet_db;
static uint32 pet_newid = 100;

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

	len=sprintf(str,"%ld,%d,%s\t%ld,%ld,%d,%d,%d,%d,%d,%d,%d",
		(unsigned long)p->pet_id,p->class_,p->name,p->account_id,p->char_id,p->level,p->egg_id,
		p->equip_id,p->intimate,p->hungry,p->rename_flag,p->incuvate);

	return 0;
}

int inter_pet_fromstr(char *str,struct s_pet *p)
{
	int s;
	int tmp_int[16];
	char tmp_str[256];
	
	memset(p,0,sizeof(struct s_pet));
	
//	ShowMessage("sscanf pet main info\n");
	s=sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
		&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

	if(s!=12)
		return 1;
	
	p->pet_id = tmp_int[0];
	p->class_ = tmp_int[1];
	memcpy(p->name,tmp_str,24);
	p->account_id = tmp_int[2];
	p->char_id = tmp_int[3];
	p->level = tmp_int[4];
	p->egg_id = tmp_int[5];
	p->equip_id = tmp_int[6];
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

	pet_db=numdb_init();

	if( (fp=safefopen(pet_txt,"r"))==NULL )
		return 1;
	while(fgets(line,sizeof(line),fp)){
		p = (struct s_pet*)aCalloc(1,sizeof(struct s_pet));
		if(inter_pet_fromstr(line,p)==0 && p->pet_id>0){
			if( p->pet_id >= pet_newid)
				pet_newid=p->pet_id+1;
			numdb_insert(pet_db,p->pet_id,p);
		}else{
			ShowMessage("int_pet: broken data [%s] line %d\n",pet_txt,c);
			aFree(p);
		}
		c++;
	}
	fclose(fp);
//	ShowMessage("int_pet: %s read done (%d pets)\n",pet_txt,c);
	return 0;
}

int pet_db_final (void *k, void *data, va_list &ap) {
	struct s_pet *p = (struct s_pet *)data;
	if (p) aFree(p);
	return 0;
}
void inter_pet_final()
{
	if(pet_db)
	{
		numdb_final(pet_db, pet_db_final);
		pet_db=NULL;
	}
	return;
}

int inter_pet_save_sub(void *key,void *data,va_list &ap)
{
	char line[8192];
	FILE *fp;
	inter_pet_tostr(line,(struct s_pet *)data);
	fp=va_arg(ap,FILE*);
	fprintf(fp,"%s" RETCODE,line);
	return 0;
}

int inter_pet_save()
{
	FILE *fp;
	int lock;
	if( (fp=lock_fopen(pet_txt,&lock))==NULL ){
		ShowMessage("int_pet: cant write [%s] !!! data is lost !!!\n",pet_txt);
		return 1;
	}
	numdb_foreach(pet_db,inter_pet_save_sub,fp);
	lock_fclose(fp,pet_txt,&lock);
//	ShowMessage("int_pet: %s saved.\n",pet_txt);
	return 0;
}

int inter_pet_delete(int pet_id)
{
	struct s_pet *p;
	p = (struct s_pet *)numdb_search(pet_db,pet_id);
	if( p == NULL)
		return 1;
	else {
		numdb_erase(pet_db,pet_id);
		ShowMessage("pet_id: %d deleted\n",pet_id);
	}
	return 0;
}

int mapif_pet_created(int fd,uint32 account_id,struct s_pet *p)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3880;
	WFIFOL(fd,2)=account_id;
	if(p!=NULL){
		WFIFOB(fd,6)=0;
		WFIFOL(fd,7)=p->pet_id;
		ShowMessage("int_pet: created! %d %s\n",p->pet_id,p->name);
	}else{
		WFIFOB(fd,6)=1;
		WFIFOL(fd,7)=0;
	}
	WFIFOSET(fd,11);

	return 0;
}

int mapif_pet_info(int fd,uint32 account_id,struct s_pet *pet)
{
	if( !session_isActive(fd) )
		return 0;

	if(pet)
	{
		WFIFOW(fd,0)=0x3881;
		WFIFOW(fd,2)=sizeof(struct s_pet) + 9;
		WFIFOL(fd,4)=account_id;
		WFIFOB(fd,8)=0;
		s_pet_tobuffer(*pet, WFIFOP(fd,9));

		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return 0;
}

int mapif_pet_noinfo(int fd,uint32 account_id)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3881;
	WFIFOW(fd,2)=sizeof(struct s_pet) + 9;
	WFIFOL(fd,4)=account_id;
	WFIFOB(fd,8)=1;
	memset(WFIFOP(fd,9),0,sizeof(struct s_pet));
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int mapif_save_pet_ack(int fd,uint32 account_id,int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3882;
	WFIFOL(fd,2)=account_id;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,7);

	return 0;
}

int mapif_delete_pet_ack(int fd,int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3883;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,3);

	return 0;
}

int mapif_create_pet(int fd,uint32 account_id,uint32 char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incuvate,char *pet_name)
{
	struct s_pet *p;
	p = (struct s_pet *)aCalloc(1, sizeof(struct s_pet));
	p->pet_id = pet_newid++;
	memcpy(p->name,pet_name,24);
	if(incuvate == 1)
		p->account_id = p->char_id = 0;
	else {
		p->account_id = account_id;
		p->char_id = char_id;
	}
	p->class_ = pet_class;
	p->level = pet_lv;
	p->egg_id = pet_egg_id;
	p->equip_id = pet_equip;
	p->intimate = intimate;
	p->hungry = hungry;
	p->rename_flag = rename_flag;
	p->incuvate = incuvate;

	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;
	
	numdb_insert(pet_db,p->pet_id,p);
	
	mapif_pet_created(fd,account_id,p);
	
	return 0;
}

int mapif_load_pet(int fd,uint32 account_id,uint32 char_id,uint32 pet_id)
{
	struct s_pet *p;
	p = (struct s_pet *)numdb_search(pet_db,pet_id);
	if(p!=NULL) {
		if(p->incuvate == 1) {
			p->account_id = p->char_id = 0;
			mapif_pet_info(fd,account_id,p);
		}
		else if(account_id == p->account_id && char_id == p->char_id)
			mapif_pet_info(fd,account_id,p);
		else
			mapif_pet_noinfo(fd,account_id);
	}
	else
		mapif_pet_noinfo(fd,account_id);

	return 0;
}

int mapif_save_pet(int fd,uint32 account_id, unsigned char* buf)
{
	if( !session_isActive(fd) )
		return 0;

	struct s_pet *ppet, pet;
	int len=RFIFOW(fd,2);
	if(sizeof(struct s_pet)!=len-8) {
		ShowMessage("inter pet: data size error %d %d\n",sizeof(struct s_pet),len-8);
	}
	else{
		s_pet_frombuffer(pet, buf);
		ppet = (struct s_pet *)numdb_search(pet_db,pet.pet_id);
		if(ppet == NULL) {
			ppet = (struct s_pet *)aCalloc(1, sizeof(struct s_pet));
			ppet->pet_id = pet.pet_id;
			if(ppet->pet_id == 0)
				pet.pet_id = ppet->pet_id = pet_newid++;
			numdb_insert(pet_db, ppet->pet_id, ppet);		
		}
		if(pet.hungry < 0)
			pet.hungry = 0;
		else if(pet.hungry > 100)
			pet.hungry = 100;
		if(pet.intimate < 0)
			pet.intimate = 0;
		else if(pet.intimate > 1000)
			pet.intimate = 1000;
		memcpy(ppet,&pet,sizeof(struct s_pet));

		if(ppet->incuvate == 1)
			ppet->account_id = ppet->char_id = 0;

		mapif_save_pet_ack(fd,account_id,0);
	}

	return 0;
}

int mapif_delete_pet(int fd,int pet_id)
{
	mapif_delete_pet_ack(fd,inter_pet_delete(pet_id));

	return 0;
}

int mapif_parse_CreatePet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_create_pet(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOW(fd,14),RFIFOW(fd,16),
		RFIFOW(fd,18),RFIFOW(fd,20),RFIFOB(fd,22),RFIFOB(fd,23),(char*)RFIFOP(fd,24));
	return 0;
}

int mapif_parse_LoadPet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_load_pet(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 0;
}

int mapif_parse_SavePet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_save_pet(fd,RFIFOL(fd,4),RFIFOP(fd,8));
	return 0;
}

int mapif_parse_DeletePet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_delete_pet(fd,RFIFOL(fd,2));
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
	default:
		return 0;
	}
	return 1;
}

