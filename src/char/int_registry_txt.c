// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include <stdio.h>


char accreg_txt[1024] = "save/accreg.txt";
static DBMap* accreg_db = NULL; // int account_id -> struct accreg*

/*

// アカウント変数を文字列へ変換
int inter_accreg_tostr(char *str, struct accreg *reg)
{
	int j;
	char *p = str;

	p += sprintf(p, "%d\t", reg->account_id);
	for(j = 0; j < reg->reg_num; j++) {
		p += sprintf(p,"%s,%s ", reg->reg[j].str, reg->reg[j].value);
	}

	return 0;
}

// アカウント変数を文字列から変換
int inter_accreg_fromstr(const char *str, struct accreg *reg)
{
	int j, n;
	const char *p = str;

	if (sscanf(p, "%d\t%n", &reg->account_id, &n ) != 1 || reg->account_id <= 0)
		return 1;

	for(j = 0, p += n; j < ACCOUNT_REG_NUM; j++, p += n) {
		if (sscanf(p, "%[^,],%[^ ] %n", reg->reg[j].str, reg->reg[j].value, &n) != 2) 
			break;
	}
	reg->reg_num = j;

	return 0;
}

/// Serializes account regs of the specified acccount into the provided buffer.
/// Returns the number of bytes written.
int inter_accreg_tobuf(uint8* buf, int account_id)
{
	int c = 0;
	int i;

	struct accreg* reg = (struct accreg*)idb_get(accreg_db,account_id);
	if( reg == NULL )
		return 0;

	for( i = 0; i < reg->reg_num; i++)
	{
		c += sprintf((char*)WBUFP(buf,c), "%s", reg->reg[i].str)+1; //We add 1 to consider the '\0' in place.
		c += sprintf((char*)WBUFP(buf,c), "%s", reg->reg[i].value)+1;
	}

	return c;
}

int inter_accreg_frombuf(uint8* buf, struct accreg* reg)
{

}



// アカウント変数の読み込み
int inter_accreg_init(void)
{
	char line[8192];
	FILE *fp;
	int c = 0;
	struct accreg *reg;

	accreg_db = idb_alloc(DB_OPT_RELEASE_DATA);

	fp = fopen(accreg_txt, "r");
	if( fp == NULL )
		return 1;

	while(fgets(line, sizeof(line), fp))
	{
		reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1);
		if (reg == NULL) {
			ShowFatalError("inter: accreg: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		if (inter_accreg_fromstr(line, reg) == 0 && reg->account_id > 0) {
			idb_put(accreg_db, reg->account_id, reg);
		} else {
			ShowError("inter: accreg: broken data [%s] line %d\n", accreg_txt, c);
			aFree(reg);
		}
		c++;
	}

	fclose(fp);
	return 0;
}

int inter_accreg_final(void)
{
	accreg_db->destroy(accreg_db, NULL);
}

// アカウント変数のセーブ用
int inter_accreg_save_sub(DBKey key, void *data, va_list ap)
{
	char line[8192];
	FILE *fp;
	struct accreg *reg = (struct accreg *)data;

	if( reg->reg_num > 0 )
	{
		inter_accreg_tostr(line,reg);
		fp = va_arg(ap, FILE *);
		fprintf(fp, "%s\n", line);
	}

	return 0;
}

// アカウント変数のセーブ
int inter_accreg_save(void)
{
	FILE *fp;
	int lock;

	fp = lock_fopen(accreg_txt,&lock);
	if( fp == NULL )
	{
		ShowError("int_accreg: can't write [%s] !!! data is lost !!!\n", accreg_txt);
		return 1;
	}
	accreg_db->foreach(accreg_db, inter_accreg_save_sub,fp);
	lock_fclose(fp, accreg_txt, &lock);

	return 0;
}

static void* create_accreg(DBKey key, va_list args)
{
	struct accreg *reg;
	reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1);
	reg->account_id = key.i;
	return reg;
}







bool mmo_globalreg_tobuf(uint8* buf, struct mmo_charstatus* status)
{

}

//Reply to map server with acc reg values.
int mapif_account_reg_reply(int fd,int account_id,int char_id)
{
	int i,j,p;
	WFIFOHEAD(fd, GLOBAL_REG_NUM*288 + 13);
	WFIFOW(fd,0)=0x3804;
	WFIFOL(fd,4)=account_id;
	WFIFOL(fd,8)=char_id;
	WFIFOB(fd,12)=3; //Type 3: char acc reg.

	ARR_FIND( 0, char_num, i, char_dat[i].status.account_id == account_id && char_dat[i].status.char_id == char_id );
	if( i == char_num ) //Character not found? Send empty packet.
		WFIFOW(fd,2)=13;
	else
	{
		for (p=13,j = 0; j < char_dat[i].global_num; j++) {
			if (char_dat[i].global[j].str[0]) {
				p+= sprintf((char*)WFIFOP(fd,p), "%s", char_dat[i].global[j].str)+1; //We add 1 to consider the '\0' in place.
				p+= sprintf((char*)WFIFOP(fd,p), "%s", char_dat[i].global[j].value)+1;
			}
		}
		WFIFOW(fd,2) = p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
*/