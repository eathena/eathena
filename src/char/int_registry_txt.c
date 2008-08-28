// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include <stdio.h>


char accreg_txt[1024] = "save/accreg.txt";
static DBMap* accreg_db = NULL; // int account_id -> struct regs*
static DBMap* charreg_db = NULL; // int char_id -> struct regs*

static void* create_regs(DBKey key, va_list args)
{
	return (struct regs*)aCalloc(sizeof(struct regs), 1);
}


/////////////////////////////
/// Account reg manipulation

int inter_accreg_tostr(char* str, const struct regs* reg)
{
	char* p = str;
	int j;

	for( j = 0; j < reg->reg_num; ++j )
		p += sprintf(p, "%s,%s ", reg->reg[j].str, reg->reg[j].value);

	return 0;
}

int inter_accreg_fromstr(const char* str, struct regs* reg)
{
	int j, n;
	const char* p = str;

	for( j = 0; j < ACCOUNT_REG_NUM; j++, p += n )
	{
		if (sscanf(p, "%[^,],%[^ ] %n", reg->reg[j].str, reg->reg[j].value, &n) != 2) 
			break;
	}
	reg->reg_num = j;

	return 0;
}

bool inter_accreg_load(int account_id, struct regs* reg)
{
	struct regs* dbreg = (struct regs*)idb_get(accreg_db,account_id);
	if( dbreg == NULL )
	{
		reg->reg_num = 0;
		return false;
	}

	memcpy(reg, dbreg, sizeof(struct regs));
	return true;
}

bool inter_accreg_save(int account_id, struct regs* reg)
{
	struct regs* dbreg = (struct regs*)idb_ensure(accreg_db, account_id, create_regs);
	memcpy(dbreg, reg, sizeof(struct regs));
	return true;
}

int inter_accreg_sync(void)
{
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(accreg_txt,&lock);
	if( fp == NULL )
	{
		ShowError("int_accreg: can't write [%s] !!! data is lost !!!\n", accreg_txt);
		return 1;
	}

	iter = accreg_db->iterator(accreg_db);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int account_id = key.i;
		struct regs* reg = (struct regs*) data;
		char line[8192];

		if( reg->reg_num == 0 )
			continue;

		inter_accreg_tostr(line,reg);
		fprintf(fp, "%d\t%s\n", account_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, accreg_txt, &lock);

	return 0;
}

/// Sets up accreg_db and loads data into it.
int inter_accreg_init(void)
{
	char line[8192];
	FILE *fp;
	int c = 0;
	int n;
	int account_id;
	struct regs* reg;

	accreg_db = idb_alloc(DB_OPT_RELEASE_DATA);

	fp = fopen(accreg_txt, "r");
	if( fp == NULL )
		return 1;

	while( fgets(line, sizeof(line), fp) )
	{
		reg = (struct regs*)aCalloc(sizeof(struct regs), 1);
		if( reg == NULL )
		{
			ShowFatalError("inter: accreg: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		// load account id
		if( sscanf(line, "%d\t%n", &account_id, &n) != 1 || account_id <= 0 )
			continue;

		// load regs for this account
		if( inter_accreg_fromstr(line + n, reg) != 0 )
		{
			ShowError("inter: accreg: broken data [%s] line %d\n", accreg_txt, c);
			aFree(reg);
			continue;
		}

		idb_put(accreg_db, account_id, reg);
		c++;
	}

	fclose(fp);
	return 0;
}

int inter_accreg_final(void)
{
	accreg_db->destroy(accreg_db, NULL);
	return 0;
}


///////////////////////////////
/// Character reg manipulation

int inter_charreg_tostr(char* str, const struct regs* reg)
{
	int c = 0;
	int i;

	for( i = 0; i < reg->reg_num; ++i )
		if( reg->reg[i].str[0] != '\0' )
			c += sprintf(str+c, "%s,%s ", reg->reg[i].str, reg->reg[i].value);

	return c;
}

bool inter_charreg_fromstr(const char* str, struct regs* reg)
{
	int i;
	int len;

	for( i = 0; *str && *str != '\t' && *str != '\n' && *str != '\r'; ++i )
	{// global_reg実装以前のathena.txt互換のため一応'\n'チェック
		if( sscanf(str, "%[^,],%[^ ] %n", reg->reg[i].str, reg->reg[i].value, &len) != 2 )
		{ 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if( *str == ',' && sscanf(str, ",%[^ ] %n", reg->reg[i].value, &len) == 1 )
				i--;
			else
				return false;
		}

		str += len;
		if ( *str == ' ' )
			str++;
	}

	reg->reg_num = i;

	return true;
}

bool inter_charreg_load(int char_id, struct regs* reg)
{
	struct regs* dbreg = (struct regs*)idb_get(charreg_db, char_id);
	if( dbreg == NULL )
	{
		reg->reg_num = 0;
		return false;
	}

	memcpy(reg, dbreg, sizeof(struct regs));
	return true;
}

bool inter_charreg_save(int char_id, struct regs* reg)
{
	struct regs* dbreg = (struct regs*)idb_ensure(charreg_db, char_id, create_regs);
	memcpy(dbreg, reg, sizeof(struct regs));
	return true;
}

int inter_charreg_init(void)
{
	charreg_db = idb_alloc(DB_OPT_RELEASE_DATA);
	return 0;
}

int inter_charreg_final(void)
{
	charreg_db->destroy(charreg_db, NULL);
	return 0;
}


int inter_registry_init(void)
{
	inter_accreg_init();
	inter_charreg_init();
	return 0;
}

int inter_registry_final(void)
{
	inter_accreg_final();
	inter_charreg_final();
	return 0;
}
