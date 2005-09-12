//
// original code from athena
// SQL conversion by Jioh L. Jung
//
#include "base.h"
#include "char.h"
#include "strlib.h"
#include "showmsg.h"


#include "inter.h"
#include "int_pet.h"


static int pet_newid = 100;


//---------------------------------------------------------
int inter_pet_tosql(int pet_id, struct s_pet *p)
{	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	char t_name[64];

	ShowMessage("request save pet: %d.......\n",pet_id);

	jstrescapecpy(t_name, p->name);

	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;
	sprintf(tmp_sql,"SELECT * FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0)
		//row reside -> updating
		sprintf(tmp_sql, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%ld',`char_id`='%ld',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%ld'",
			pet_db, p->class_, t_name, (unsigned long)p->account_id, (unsigned long)p->char_id, p->level, p->egg_id,
			p->equip_id, p->intimate, p->hungry, p->rename_flag, p->incuvate, (unsigned long)p->pet_id);
	else //no row -> insert
		sprintf(tmp_sql,"INSERT INTO `%s` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`) VALUES ('%d', '%d', '%s', '%ld', '%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			pet_db, (unsigned long)pet_id, p->class_, t_name, (unsigned long)p->account_id, (unsigned long)p->char_id, p->level, p->egg_id,
			p->equip_id, p->intimate, p->hungry, p->rename_flag, p->incuvate);
	mysql_free_result(sql_res) ; //resource free
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (inset/update `pet`)- %s\n", mysql_error(&mysql_handle) );
	}

	ShowMessage("pet save success.......\n");
	return 0;
}

bool inter_pet_fromsql(int pet_id, struct s_pet *p)
{
	bool ret = false;

	ShowMessage("request load pet: %d.......\n",pet_id);

	memset(p, 0, sizeof(struct s_pet));

	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)

	sprintf(tmp_sql,"SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate` FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (select `pet`)- %s\n", mysql_error(&mysql_handle) );
		return false;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);

		p->pet_id = pet_id;
		p->class_ = atoi(sql_row[1]);
		memcpy(p->name, sql_row[2],24);
		p->account_id = atoi(sql_row[3]);
		p->char_id = atoi(sql_row[4]);
		p->level = atoi(sql_row[5]);
		p->egg_id = atoi(sql_row[6]);
		p->equip_id = atoi(sql_row[7]);
		p->intimate = atoi(sql_row[8]);
		p->hungry = atoi(sql_row[9]);
		p->rename_flag = atoi(sql_row[10]);
		p->incuvate = atoi(sql_row[11]);

		if(p->hungry < 0)
			p->hungry = 0;
		else if(p->hungry > 100)
			p->hungry = 100;
		if(p->intimate < 0)
			p->intimate = 0;
		else if(p->intimate > 1000)
			p->intimate = 1000;
		ShowMessage("pet load success.......\n");
		ret=true;
	}
	mysql_free_result(sql_res);
	return ret;
}
//----------------------------------------------

int inter_pet_sql_init(){
	int i;

	//memory alloc
	ShowMessage("interserver pet memory initialize.... (%d byte)\n",sizeof(struct s_pet));

	sprintf (tmp_sql , "SELECT count(*) FROM `%s`", pet_db);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		exit(0);
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);
	ShowMessage("total pet data -> '%s'.......\n",sql_row[0]);
	i = atoi (sql_row[0]);
	mysql_free_result(sql_res);

	if (i > 0) {
		//set pet_newid
		sprintf (tmp_sql , "SELECT max(`pet_id`) FROM `%s`",pet_db );
		if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}

		sql_res = mysql_store_result(&mysql_handle) ;

		sql_row = mysql_fetch_row(sql_res);
		pet_newid = atoi (sql_row[0])+1; //should SET MAX existing PET ID + 1 [Lupus]
		mysql_free_result(sql_res);
	}

	ShowMessage("set pet_newid: %d.......\n",pet_newid);

	return 0;
}
void inter_pet_sql_final(){
	return;
}
//----------------------------------
int inter_pet_delete(int pet_id){
	ShowMessage("request delete pet: %d.......\n",pet_id);

	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	return 0;
}
//------------------------------------------------------
int mapif_pet_created(int fd, uint32 account_id, struct s_pet *p)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) =0x3880;
	WFIFOL(fd, 2) =account_id;
	if(p!=NULL){
		WFIFOB(fd, 6)=0;
		WFIFOL(fd, 7) =p->pet_id;
		ShowMessage("int_pet: created! %d %s\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd, 6)=1;
		WFIFOL(fd, 7)=0;
	}
	WFIFOSET(fd, 11);

	return 0;
}

int mapif_pet_info(int fd, uint32 account_id, struct s_pet *p)
{
	if( !session_isActive(fd) )
		return 0;

	if(p)
	{
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=0;
		//memcpy(WFIFOP(fd, 9), p, sizeof(struct s_pet));
		s_pet_tobuffer(*p, WFIFOP(fd, 9));
	WFIFOSET(fd, WFIFOW(fd, 2));
	}
	return 0;
}

int mapif_pet_noinfo(int fd, uint32 account_id)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=1;
	memset(WFIFOP(fd, 9), 0, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int mapif_save_pet_ack(int fd, uint32 account_id, int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) =0x3882;
	WFIFOL(fd, 2) =account_id;
	WFIFOB(fd, 6) =flag;
	WFIFOSET(fd, 7);

	return 0;
}

int mapif_delete_pet_ack(int fd, int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd, 0) =0x3883;
	WFIFOB(fd, 2) =flag;
	WFIFOSET(fd, 3);

	return 0;
}

int mapif_create_pet(int fd, uint32 account_id, uint32 char_id, short pet_class, short pet_lv, short pet_egg_id,
	short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, char *pet_name)
{
	struct s_pet pet;
	memset(&pet, 0, sizeof(struct s_pet));

	pet.pet_id = pet_newid++;
	memcpy(pet.name, pet_name, 24);
	if(incuvate == 1)
		pet.account_id = pet.char_id = 0;
	else {
		pet.account_id = account_id;
		pet.char_id = char_id;
	}
	pet.class_ = pet_class;
	pet.level = pet_lv;
	pet.egg_id = pet_egg_id;
	pet.equip_id = pet_equip;
	pet.intimate = intimate;
	pet.hungry = hungry;
	pet.rename_flag = rename_flag;
	pet.incuvate = incuvate;

	if(pet.hungry < 0)
		pet.hungry = 0;
	else if(pet.hungry > 100)
		pet.hungry = 100;
	if(pet.intimate < 0)
		pet.intimate = 0;
	else if(pet.intimate > 1000)
		pet.intimate = 1000;

	inter_pet_tosql(pet.pet_id, &pet);

	mapif_pet_created(fd, account_id, &pet);

	return 0;
}

int mapif_load_pet(int fd, uint32 account_id, uint32 char_id, uint32 pet_id)
{
	struct s_pet pet;
	memset(&pet, 0, sizeof(struct s_pet));

	if( inter_pet_fromsql(pet_id, &pet) )
	{
		if(pet.incuvate == 1) {
			pet.account_id = pet.char_id = 0;
			mapif_pet_info(fd, account_id, &pet);
		}
		else if(account_id == pet.account_id && char_id == pet.char_id)
			mapif_pet_info(fd, account_id, &pet);
		else
			mapif_pet_noinfo(fd, account_id);
	}
	else
		mapif_pet_noinfo(fd, account_id);

	return 0;
}

int mapif_save_pet(int fd, uint32 account_id, unsigned char* buf) {
	//here process pet save request.
	struct s_pet pet;
	if( !session_isActive(fd) )
		return 0;

	int len=RFIFOW(fd, 2);
	if(sizeof(struct s_pet)!=len-8) {
		ShowMessage("inter pet: data size error %d %d\n", sizeof(struct s_pet), len-8);
	}

	else{
		s_pet_frombuffer(pet, buf);
		if(pet.hungry < 0)
			pet.hungry = 0;
		else if(pet.hungry > 100)
			pet.hungry = 100;
		if(pet.intimate < 0)
			pet.intimate = 0;
		else if(pet.intimate > 1000)
			pet.intimate = 1000;
		inter_pet_tosql(pet.pet_id, &pet);
		mapif_save_pet_ack(fd, account_id, 0);
	}

	return 0;
}

int mapif_delete_pet(int fd, int pet_id){
	mapif_delete_pet_ack(fd, inter_pet_delete(pet_id));

	return 0;
}

int mapif_parse_CreatePet(int fd)
{
	if( !session_isActive(fd) )
	return 0;

	mapif_create_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOW(fd, 10), RFIFOW(fd, 12), RFIFOW(fd, 14), 
						RFIFOW(fd, 16), RFIFOW(fd, 18), RFIFOW(fd, 20), RFIFOB(fd, 22), RFIFOB(fd, 23), (char*)RFIFOP(fd, 24));
	return 0;
}

int mapif_parse_LoadPet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_load_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
	return 0;
}

int mapif_parse_SavePet(int fd)
{
	if( !session_isActive(fd) )
	return 0;

	mapif_save_pet(fd, RFIFOL(fd, 4), RFIFOP(fd, 8));
	return 0;
}

int mapif_parse_DeletePet(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_delete_pet(fd, RFIFOL(fd, 2));
	return 0;
}

int inter_pet_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOW(fd, 0)){
	case 0x3080: mapif_parse_CreatePet(fd); break;
	case 0x3081: mapif_parse_LoadPet(fd); break;
	case 0x3082: mapif_parse_SavePet(fd); break;
	case 0x3083: mapif_parse_DeletePet(fd); break;
	default:
		return 0;
	}
	return 1;
}

