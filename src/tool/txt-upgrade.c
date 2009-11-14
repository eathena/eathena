// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/strlib.h" // sv_parse(), sv_escape_c(), safestnrcpy()
#include <stdio.h>
#include <stdlib.h> // strtol(), strtoul()
#include <string.h> // strlen(), memcpy()
#ifdef WIN32
#include <direct.h> // chdir()
#else
#include <unistd.h> // chdir()
#endif


#define CHARDB_TXT_DB_VERSION 20090825
#define START_CHAR_NUM 1


const char* file_chars = "athena.txt";
const char* file_ranks = "ranks.txt";
const char* file_memos = "memo.txt";
const char* file_inventories = "inventory.txt";
const char* file_carts = "cart.txt";
const char* file_skills = "skill.txt";
const char* file_charregs = "charreg.txt";


//////////////////////////////////////////
/// version 00000000 start
//////////////////////////////////////////
struct chardata_00000000
{
	int char_id;
	int account_id;
	unsigned char slot;
	char name[NAME_LENGTH];
	short class_;
	unsigned int base_level;
	unsigned int job_level;
	unsigned int base_exp;
	unsigned int job_exp;
	int zeny;
	int hp;
	int max_hp;
	int sp;
	int max_sp;
	short str;
	short agi;
	short vit;
	short int_;
	short dex;
	short luk;
	unsigned int status_point;
	unsigned int skill_point;
	unsigned int option;
	unsigned char karma;
	short manner;
	int party_id;
	int guild_id;
	int pet_id;
	int hom_id;
	short hair;
	short hair_color;
	short clothes_color;
	short weapon;
	short shield;
	short head_top;
	short head_mid;
	short head_bottom;
	struct point last_point;
	struct point save_point;
	int partner_id;
	int father;
	int mother;
	int child;
	int fame;
	struct point memo_point[MAX_MEMOPOINTS];
	struct item inventory[MAX_INVENTORY];
	struct item cart[MAX_CART];
	struct s_skill skill[MAX_SKILL];
	struct regs reg;
};

bool load_from_00000000(char* str, struct chardata_00000000* cd)
{
	int fields[32][2];
	int count;
	const char *key, *base, *memo, *inventory, *cart, *skills, *regs;
	const char* p;
	int n;
	int tmp_int[2];
	char tmp_str[2][128];
	int i;

	memset(cd, 0, sizeof(*cd));

	// extract tab-separated column blocks from str
	// block layout: <char id> <base data> <memo points> <inventory> <cart> <skills> <char regs>
	count = sv_parse(str, strlen(str), 0, '\t', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));
	if( count != 20 )
		return false;

	key = &str[fields[1][0]];
	base = &str[fields[2][0]];
	memo = &str[fields[15][0]];
	inventory = &str[fields[16][0]];
	cart = &str[fields[17][0]];
	skills = &str[fields[18][0]];
	regs = &str[fields[19][0]];
	p = key;

	// key (char id)
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	p += n + 1;
	if( p != base )
		return false;

	// base data
	if( sscanf(p, "%d,%d\t%23[^\t]\t%hd,%u,%u\t%u,%u,%d\t%d,%d,%d,%d\t%hd,%hd,%hd,%hd,%hd,%hd\t%u,%u\t%u,%d,%hd%n",
		&cd->account_id, &tmp_int[0], cd->name,
		&cd->class_, &cd->base_level, &cd->job_level,
		&cd->base_exp, &cd->job_exp, &cd->zeny,
		&cd->hp, &cd->max_hp, &cd->sp, &cd->max_sp,
		&cd->str, &cd->agi, &cd->vit, &cd->int_, &cd->dex, &cd->luk,
		&cd->status_point, &cd->skill_point,
		&cd->option, &tmp_int[1], &cd->manner,
		&n) != 24 || p[n] != '\t' )
		return false;

	cd->slot = tmp_int[0];
	cd->karma = tmp_int[1];

	p += n + 1;
	if( sscanf(p, "%d,%d,%d,%d%n", &cd->party_id, &cd->guild_id, &cd->pet_id, &cd->hom_id, &n) != 4 || p[n] != '\t' )
	{
		cd->hom_id = 0;
		if( sscanf(p, "%d,%d,%d%n", &cd->party_id, &cd->guild_id, &cd->pet_id, &n) != 3 || p[n] != '\t' )
		{
			cd->pet_id = 0;
			if( sscanf(p, "%d,%d%n", &cd->party_id, &cd->guild_id, &n) != 2 || p[n] != '\t' )
			{
				return false;
			}
		}
	}

	p += n + 1;
	if( sscanf(p, "%hd,%hd,%hd\t%hd,%hd,%hd,%hd,%hd%n",
		&cd->hair, &cd->hair_color, &cd->clothes_color,
		&cd->weapon, &cd->shield, &cd->head_top, &cd->head_mid, &cd->head_bottom,
		&n) != 8 || p[n] != '\t' )
		return false;

	p += n + 1;
	if( sscanf(p, "%hu,%hd,%hd\t%hu,%hd,%hd%n",
		&cd->last_point.map, &cd->last_point.x, &cd->last_point.y,
		&cd->save_point.map, &cd->save_point.x, &cd->save_point.y,
		&n) == 6 && p[n] == ',' )
		;
	else
	if( sscanf(p, "%127[^,],%hd,%hd\t%127[^,],%hd,%hd%n",
		tmp_str[0], &cd->last_point.x, &cd->last_point.y,
		tmp_str[1], &cd->save_point.x, &cd->save_point.y,
		&n) == 6 && (p[n] == ',' || p[n] == '\t') )
	{// Convert saved map from string to in-memory index
		cd->last_point.map = mapindex_name2id(tmp_str[0]);
		cd->save_point.map = mapindex_name2id(tmp_str[1]);
	}
	else
		return false;

	p += n;
	if( sscanf(p, ",%d,%d,%d,%d,%d%n", &cd->partner_id, &cd->father, &cd->mother, &cd->child, &cd->fame, &n) != 5 || p[n] != '\t' )
	{
		cd->fame = 0;
		if( sscanf(p, ",%d,%d,%d,%d%n", &cd->partner_id, &cd->father, &cd->mother, &cd->child, &n) != 4 || p[n] != '\t' )
		{
			cd->father = 0;
			cd->mother = 0;
			cd->child = 0;
			if( sscanf(p, ",%d%n", &cd->partner_id, &n) != 1 || p[n] != '\t' )
			{
				cd->partner_id = 0;
				if( sscanf(p, "%n", &n) != 0 || p[n] != '\t' )
				{
					return false;
				}
			}
		}
	}

	p += n + 1;
	if( p != memo )
		return false;

	// memo data
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[3];
		char tmp_str[256];

		if( sscanf(p, "%d,%d,%d%n", &tmp_int[0], &tmp_int[1], &tmp_int[2], &n) == 3 )
			;
		else
		if( sscanf(p, "%[^,],%d,%d%n", tmp_str, &tmp_int[1], &tmp_int[2], &n) == 3 )
			tmp_int[0] = mapindex_name2id(tmp_str);
		else
			return false;

		if( p[n] != ' ' )
			return false;

		p += n + 1;

		if( i == MAX_MEMOPOINTS )
			continue; // TODO: warning?

		cd->memo_point[i].map = tmp_int[0];
		cd->memo_point[i].x = tmp_int[1];
		cd->memo_point[i].y = tmp_int[2];
	}

	p += 1;
	if( p != inventory )
		return false;

	// inventory items
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7+MAX_SLOTS+1];
		int len;
		int j, k;

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&len) != 7 )
			return false;

		p += len;

		j = 0;
		while( *p != ' ' && *p != '\0' )
		{
			if( sscanf(p, ",%d%n", &tmp_int[7+j], &len) != 1 )
				return false;

			p += len;

			if( j == MAX_SLOTS )
				continue; // discard card slots over max

			j++;
		}

		if( *p == ' ' )
			p++;

		if( i == MAX_INVENTORY )
			continue; // discard items over max

		cd->inventory[i].id = tmp_int[0];
		cd->inventory[i].nameid = tmp_int[1];
		cd->inventory[i].amount = tmp_int[2];
		cd->inventory[i].equip = tmp_int[3];
		cd->inventory[i].identify = tmp_int[4];
		cd->inventory[i].refine = tmp_int[5];
		cd->inventory[i].attribute = tmp_int[6];
		for( k = 0; k < j; ++k )
			cd->inventory[i].card[k] = tmp_int[7+k];
	}
	
	p += 1;
	if( p != cart )
		return false;

	// cart items
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7+MAX_SLOTS+1];
		int len;
		int j, k;

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&len) != 7 )
			return false;

		p += len;

		j = 0;
		while( *p != ' ' && *p != '\0' )
		{
			if( sscanf(p, ",%d%n", &tmp_int[7+j], &len) != 1 )
				return false;

			p += len;

			if( j == MAX_SLOTS )
				continue; // discard card slots over max

			j++;
		}

		if( *p == ' ' )
			p++;

		if( i == MAX_CART )
			continue; // discard items over max

		cd->cart[i].id = tmp_int[0];
		cd->cart[i].nameid = tmp_int[1];
		cd->cart[i].amount = tmp_int[2];
		cd->cart[i].equip = tmp_int[3];
		cd->cart[i].identify = tmp_int[4];
		cd->cart[i].refine = tmp_int[5];
		cd->cart[i].attribute = tmp_int[6];
		for( k = 0; k < j; ++k )
			cd->cart[i].card[k] = tmp_int[7+k];
	}

	p += 1;
	if( p != skills )
		return false;

	// skills
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[2];

		if( sscanf(p, "%d,%d%n", &tmp_int[0], &tmp_int[1], &n) != 2 )
			return false;

		if( p[n] != ' ' )
			return false;

		p += n + 1;

		if( tmp_int[0] >= MAX_SKILL )
			continue; // TODO: warning?

		cd->skill[tmp_int[0]].id = tmp_int[0];
		cd->skill[tmp_int[0]].lv = tmp_int[1];
	}

	p += 1;
	if( p != regs )
		return false;

	// character regs
	for( i = 0; i < GLOBAL_REG_NUM && *p != '\0' && *p != '\t' && *p != '\n' && *p != '\r'; ++i )
	{
		int len;
		if( sscanf(p, "%[^,],%[^ ] %n", cd->reg.reg[i].str, cd->reg.reg[i].value, &len) != 2 )
		{ 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if( *p == ',' && sscanf(p, ",%[^ ] %n", cd->reg.reg[i].value, &len) == 1 )
				i--;
			else
				return false;
		}

		p += len;
		if ( *p == ' ' )
			p++;
	}
	cd->reg.reg_num = i;

	return true;
}
//////////////////////////////////////////
/// version 00000000 end
//////////////////////////////////////////

//////////////////////////////////////////
/// version 20090810 start
//////////////////////////////////////////
struct chardata_20090810
{
	int char_id;
	int account_id;
	unsigned char slot;
	char name[NAME_LENGTH];
	short class_;
	unsigned int base_level;
	unsigned int job_level;
	unsigned int base_exp;
	unsigned int job_exp;
	int zeny;
	int hp;
	int max_hp;
	int sp;
	int max_sp;
	short str;
	short agi;
	short vit;
	short int_;
	short dex;
	short luk;
	unsigned int status_point;
	unsigned int skill_point;
	unsigned int option;
	unsigned char karma;
	short manner;
	int party_id;
	int guild_id;
	int pet_id;
	int hom_id;
	int mer_id;
	short hair;
	short hair_color;
	short clothes_color;
	short weapon;
	short shield;
	short head_top;
	short head_mid;
	short head_bottom;
	struct point last_point;
	struct point save_point;
	int partner_id;
	int father;
	int mother;
	int child;
	int fame;
	int arch_calls;
	int arch_faith;
	int spear_calls;
	int spear_faith;
	int sword_calls;
	int sword_faith;
	struct point memo_point[MAX_MEMOPOINTS];
	struct item inventory[MAX_INVENTORY];
	struct item cart[MAX_CART];
	struct s_skill skill[MAX_SKILL];
	struct regs reg;
};

bool load_from_20090810(char* str, struct chardata_20090810* cd)
{
	int fields[32][2];
	int count;
	const char *key, *base, *memo, *inventory, *cart, *skills, *regs;
	const char* p;
	int n;
	int i;
	char tmp_map[MAP_NAME_LENGTH];
	int col[54+1][2];
	int cols;

	memset(cd, 0, sizeof(*cd));

	// extract tab-separated column blocks from str
	// block layout: <char id> <base data> <memo points> <inventory> <cart> <skills> <char regs>
	count = sv_parse(str, strlen(str), 0, '\t', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));
	if( count != 7 )
		return false;

	key = &str[fields[1][0]];
	base = &str[fields[2][0]];
	memo = &str[fields[3][0]];
	inventory = &str[fields[4][0]];
	cart = &str[fields[5][0]];
	skills = &str[fields[6][0]];
	regs = &str[fields[7][0]];
	p = key;

	// key (char id)
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	p += n + 1;
	if( p != base )
		return false;

	// base data
	cols = sv_parse(base, (ptrdiff_t)(memo - base), 0, ',', (int*)col, 2*ARRAYLENGTH(col), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF));
	if( cols != 54 )
		return false;

	cd->account_id     = (int)          strtol (&p[col[ 1][0]], NULL, 10);
	cd->slot           = (unsigned char)strtoul(&p[col[ 2][0]], NULL, 10);
	sv_unescape_c(cd->name, &p[col[3][0]], col[3][1]-col[3][0]);
	cd->class_         = (short)        strtol (&p[col[ 4][0]], NULL, 10);
	cd->base_level     = (unsigned int) strtoul(&p[col[ 5][0]], NULL, 10);
	cd->job_level      = (unsigned int) strtoul(&p[col[ 6][0]], NULL, 10);
	cd->base_exp       = (unsigned int) strtoul(&p[col[ 7][0]], NULL, 10);
	cd->job_exp        = (unsigned int) strtoul(&p[col[ 8][0]], NULL, 10);
	cd->zeny           = (int)          strtoul(&p[col[ 9][0]], NULL, 10);
	cd->hp             = (int)          strtoul(&p[col[10][0]], NULL, 10);
	cd->max_hp         = (int)          strtoul(&p[col[11][0]], NULL, 10);
	cd->sp             = (int)          strtoul(&p[col[12][0]], NULL, 10);
	cd->max_sp         = (int)          strtoul(&p[col[13][0]], NULL, 10);
	cd->str            = (short)        strtol (&p[col[14][0]], NULL, 10);
	cd->agi            = (short)        strtol (&p[col[15][0]], NULL, 10);
	cd->vit            = (short)        strtol (&p[col[16][0]], NULL, 10);
	cd->int_           = (short)        strtol (&p[col[17][0]], NULL, 10);
	cd->dex            = (short)        strtol (&p[col[18][0]], NULL, 10);
	cd->luk            = (short)        strtol (&p[col[19][0]], NULL, 10);
	cd->status_point   = (unsigned int) strtoul(&p[col[20][0]], NULL, 10);
	cd->skill_point    = (unsigned int) strtoul(&p[col[21][0]], NULL, 10);
	cd->option         = (unsigned int) strtoul(&p[col[22][0]], NULL, 10);
	cd->karma          = (unsigned char)strtoul(&p[col[23][0]], NULL, 10);
	cd->manner         = (short)        strtol (&p[col[24][0]], NULL, 10);
	cd->party_id       = (int)          strtol (&p[col[25][0]], NULL, 10);
	cd->guild_id       = (int)          strtol (&p[col[26][0]], NULL, 10);
	cd->pet_id         = (int)          strtol (&p[col[27][0]], NULL, 10);
	cd->hom_id         = (int)          strtol (&p[col[28][0]], NULL, 10);
	cd->mer_id         = (int)          strtol (&p[col[29][0]], NULL, 10);
	cd->hair           = (short)        strtol (&p[col[30][0]], NULL, 10);
	cd->hair_color     = (short)        strtol (&p[col[31][0]], NULL, 10);
	cd->clothes_color  = (short)        strtol (&p[col[32][0]], NULL, 10);
	cd->weapon         = (short)        strtol (&p[col[33][0]], NULL, 10);
	cd->shield         = (short)        strtol (&p[col[34][0]], NULL, 10);
	cd->head_top       = (short)        strtol (&p[col[35][0]], NULL, 10);
	cd->head_mid       = (short)        strtol (&p[col[36][0]], NULL, 10);
	cd->head_bottom    = (short)        strtol (&p[col[37][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[38][0]], col[38][1]-col[38][0]+1); cd->last_point.map = mapindex_name2id(tmp_map);
	cd->last_point.x   = (short)        strtol (&p[col[39][0]], NULL, 10);
	cd->last_point.y   = (short)        strtol (&p[col[40][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[41][0]], col[41][1]-col[41][0]+1); cd->save_point.map = mapindex_name2id(tmp_map);
	cd->save_point.x   = (short)        strtol (&p[col[42][0]], NULL, 10);
	cd->save_point.y   = (short)        strtol (&p[col[43][0]], NULL, 10);
	cd->partner_id     = (int)          strtol (&p[col[44][0]], NULL, 10);
	cd->father         = (int)          strtol (&p[col[45][0]], NULL, 10);
	cd->mother         = (int)          strtol (&p[col[46][0]], NULL, 10);
	cd->child          = (int)          strtol (&p[col[47][0]], NULL, 10);
	cd->fame           = (int)          strtol (&p[col[48][0]], NULL, 10);
	cd->arch_calls     = (int)          strtol (&p[col[49][0]], NULL, 10);
	cd->arch_faith     = (int)          strtol (&p[col[50][0]], NULL, 10);
	cd->spear_calls    = (int)          strtol (&p[col[51][0]], NULL, 10);
	cd->spear_faith    = (int)          strtol (&p[col[52][0]], NULL, 10);
	cd->sword_calls    = (int)          strtol (&p[col[53][0]], NULL, 10);
	cd->sword_faith    = (int)          strtol (&p[col[54][0]], NULL, 10);

	p += (ptrdiff_t)(memo - base);
	if( p != memo )
		return false;

	// memo data
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[3];
		char tmp_str[256];

		if( sscanf(p, "%d,%d,%d%n", &tmp_int[0], &tmp_int[1], &tmp_int[2], &n) == 3 )
			;
		else
		if( sscanf(p, "%[^,],%d,%d%n", tmp_str, &tmp_int[1], &tmp_int[2], &n) == 3 )
			tmp_int[0] = mapindex_name2id(tmp_str);
		else
			return false;

		if( p[n] != ' ' )
			return false;

		p += n + 1;

		if( i == MAX_MEMOPOINTS )
			continue; // TODO: warning?

		cd->memo_point[i].map = tmp_int[0];
		cd->memo_point[i].x = tmp_int[1];
		cd->memo_point[i].y = tmp_int[2];
	}

	p += 1;
	if( p != inventory )
		return false;

	// inventory items
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7+MAX_SLOTS+1];
		int len;
		int j, k;

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&len) != 7 )
			return false;

		p += len;

		j = 0;
		while( *p != ' ' && *p != '\0' )
		{
			if( sscanf(p, ",%d%n", &tmp_int[7+j], &len) != 1 )
				return false;

			p += len;

			if( j == MAX_SLOTS )
				continue; // discard card slots over max

			j++;
		}

		if( *p == ' ' )
			p++;

		if( i == MAX_INVENTORY )
			continue; // discard items over max

		cd->inventory[i].id = tmp_int[0];
		cd->inventory[i].nameid = tmp_int[1];
		cd->inventory[i].amount = tmp_int[2];
		cd->inventory[i].equip = tmp_int[3];
		cd->inventory[i].identify = tmp_int[4];
		cd->inventory[i].refine = tmp_int[5];
		cd->inventory[i].attribute = tmp_int[6];
		for( k = 0; k < j; ++k )
			cd->inventory[i].card[k] = tmp_int[7+k];
	}
	
	p += 1;
	if( p != cart )
		return false;

	// cart items
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7+MAX_SLOTS+1];
		int len;
		int j, k;

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
			&len) != 7 )
			return false;

		p += len;

		j = 0;
		while( *p != ' ' && *p != '\0' )
		{
			if( sscanf(p, ",%d%n", &tmp_int[7+j], &len) != 1 )
				return false;

			p += len;

			if( j == MAX_SLOTS )
				continue; // discard card slots over max

			j++;
		}

		if( *p == ' ' )
			p++;

		if( i == MAX_CART )
			continue; // discard items over max

		cd->cart[i].id = tmp_int[0];
		cd->cart[i].nameid = tmp_int[1];
		cd->cart[i].amount = tmp_int[2];
		cd->cart[i].equip = tmp_int[3];
		cd->cart[i].identify = tmp_int[4];
		cd->cart[i].refine = tmp_int[5];
		cd->cart[i].attribute = tmp_int[6];
		for( k = 0; k < j; ++k )
			cd->cart[i].card[k] = tmp_int[7+k];
	}

	p += 1;
	if( p != skills )
		return false;

	// skills
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[2];

		if( sscanf(p, "%d,%d%n", &tmp_int[0], &tmp_int[1], &n) != 2 )
			return false;

		if( p[n] != ' ' )
			return false;

		p += n + 1;

		if( i == MAX_SKILL )
			continue; // TODO: warning?

		cd->skill[tmp_int[0]].id = tmp_int[0];
		cd->skill[tmp_int[0]].lv = tmp_int[1];
	}

	p += 1;
	if( p != regs )
		return false;

	// character regs
	for( i = 0; i < GLOBAL_REG_NUM && *p != '\0' && *p != '\t' && *p != '\n' && *p != '\r'; ++i )
	{
		int len;
		if( sscanf(p, "%[^,],%[^ ] %n", cd->reg.reg[i].str, cd->reg.reg[i].value, &len) != 2 )
		{ 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if( *p == ',' && sscanf(p, ",%[^ ] %n", cd->reg.reg[i].value, &len) == 1 )
				i--;
			else
				return false;
		}

		p += len;
		if ( *p == ' ' )
			p++;
	}
	cd->reg.reg_num = i;

	return true;
}

bool upgrade_00000000_to_20090810(char* str, struct chardata_00000000* d, struct chardata_20090810* cd)
{
	cd->char_id = d->char_id;
	cd->account_id = d->account_id;
	cd->slot = d->slot;
	safestrncpy(cd->name, d->name, NAME_LENGTH);
	cd->class_ = d->class_;
	cd->base_level = d->base_level;
	cd->job_level = d->job_level;
	cd->base_exp = d->base_exp;
	cd->job_exp = d->job_exp;
	cd->zeny = d->zeny;
	cd->hp = d->hp;
	cd->max_hp = d->max_hp;
	cd->sp = d->sp;
	cd->max_sp = d->max_sp;
	cd->str = d->str;
	cd->agi = d->agi;
	cd->vit = d->vit;
	cd->int_ = d->int_;
	cd->dex = d->dex;
	cd->luk = d->luk;
	cd->status_point = d->status_point;
	cd->skill_point = d->skill_point;
	cd->option = d->option;
	cd->karma = d->karma;
	cd->manner = d->manner;
	cd->party_id = d->party_id;
	cd->guild_id = d->guild_id;
	cd->pet_id = d->pet_id;
	cd->hom_id = d->hom_id;
	cd->mer_id = 0;
	cd->hair = d->hair;
	cd->hair_color = d->hair_color;
	cd->clothes_color = d->clothes_color;
	cd->weapon = d->weapon;
	cd->shield = d->shield;
	cd->head_top = d->head_top;
	cd->head_mid = d->head_mid;
	cd->head_bottom = d->head_bottom;
	cd->last_point = d->last_point;
	cd->save_point = d->save_point;
	cd->partner_id = d->partner_id;
	cd->father = d->father;
	cd->mother = d->mother;
	cd->child = d->child;
	cd->fame = d->fame;
	cd->arch_calls = 0;
	cd->arch_faith = 0;
	cd->spear_calls = 0;
	cd->spear_faith = 0;
	cd->sword_calls = 0;
	cd->sword_faith = 0;
	memcpy(cd->memo_point, d->memo_point, sizeof(cd->memo_point));
	memcpy(cd->inventory, d->inventory, sizeof(cd->inventory));
	memcpy(cd->cart, d->cart, sizeof(cd->cart));
	memcpy(cd->skill, d->skill, sizeof(cd->skill));
	memcpy(&cd->reg, &d->reg, sizeof(cd->reg));

	return true;
}
//////////////////////////////////////////
/// version 20090810 end
//////////////////////////////////////////

//////////////////////////////////////////
/// version 20090825 start
//////////////////////////////////////////
struct chardata_20090825
{
	int char_id;
	int account_id;
	unsigned char slot;
	char name[NAME_LENGTH];
	short class_;
	unsigned int base_level;
	unsigned int job_level;
	unsigned int base_exp;
	unsigned int job_exp;
	int zeny;
	int hp;
	int max_hp;
	int sp;
	int max_sp;
	short str;
	short agi;
	short vit;
	short int_;
	short dex;
	short luk;
	unsigned int status_point;
	unsigned int skill_point;
	unsigned int option;
	unsigned char karma;
	short manner;
	int party_id;
	int guild_id;
	int pet_id;
	int hom_id;
	int mer_id;
	short hair;
	short hair_color;
	short clothes_color;
	short weapon;
	short shield;
	short head_top;
	short head_mid;
	short head_bottom;
	struct point last_point;
	struct point save_point;
	int partner_id;
	int father;
	int mother;
	int child;
	int arch_calls;
	int arch_faith;
	int spear_calls;
	int spear_faith;
	int sword_calls;
	int sword_faith;
};

bool write_latest(char* str, struct chardata_20090825* p)
{
	char esc_name[4*NAME_LENGTH+1];
	char *str_p = str;

	sv_escape_c(esc_name, p->name, strlen(p->name), ",");

	// key (char id)
	str_p += sprintf(str_p, "%d", p->char_id);
	*(str_p++) = '\t';

	// base character data
	                  str_p += sprintf(str_p, "%d", p->account_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->slot);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", esc_name);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->class_);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->base_level);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->job_level);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->base_exp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->job_exp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->zeny);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->max_hp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->max_sp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->str);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->agi);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->vit);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->int_);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->dex);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->luk);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->status_point);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->skill_point);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->option);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->karma);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->manner);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->party_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->guild_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->pet_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hom_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->mer_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hair);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hair_color);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->clothes_color);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->weapon);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->shield);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_top);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_mid);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_bottom);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", mapindex_id2name(p->last_point.map));
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->last_point.x);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->last_point.y);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", mapindex_id2name(p->save_point.map));
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->save_point.x);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->save_point.y);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->partner_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->father);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->mother);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->child);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_faith);

	*str_p = '\0';

	return true;
}

bool load_from_20090825(char* str, struct chardata_20090825* cd)
{
	const char* p = str;
	int col[53+1][2];
	int n;
	char tmp_map[MAP_NAME_LENGTH];

	memset(cd, 0, sizeof(*cd));

	// key (char id)
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	p += n + 1;

	// base data
	if( sv_parse(p, strlen(p), 0, ',', (int*)col, 2*ARRAYLENGTH(col), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF)) != 53 )
		return false;

	cd->account_id     = (int)          strtol (&p[col[ 1][0]], NULL, 10);
	cd->slot           = (unsigned char)strtoul(&p[col[ 2][0]], NULL, 10);
	sv_unescape_c(cd->name, &p[col[3][0]], col[3][1]-col[3][0]);
	cd->class_         = (short)        strtol (&p[col[ 4][0]], NULL, 10);
	cd->base_level     = (unsigned int) strtoul(&p[col[ 5][0]], NULL, 10);
	cd->job_level      = (unsigned int) strtoul(&p[col[ 6][0]], NULL, 10);
	cd->base_exp       = (unsigned int) strtoul(&p[col[ 7][0]], NULL, 10);
	cd->job_exp        = (unsigned int) strtoul(&p[col[ 8][0]], NULL, 10);
	cd->zeny           = (int)          strtoul(&p[col[ 9][0]], NULL, 10);
	cd->hp             = (int)          strtoul(&p[col[10][0]], NULL, 10);
	cd->max_hp         = (int)          strtoul(&p[col[11][0]], NULL, 10);
	cd->sp             = (int)          strtoul(&p[col[12][0]], NULL, 10);
	cd->max_sp         = (int)          strtoul(&p[col[13][0]], NULL, 10);
	cd->str            = (short)        strtol (&p[col[14][0]], NULL, 10);
	cd->agi            = (short)        strtol (&p[col[15][0]], NULL, 10);
	cd->vit            = (short)        strtol (&p[col[16][0]], NULL, 10);
	cd->int_           = (short)        strtol (&p[col[17][0]], NULL, 10);
	cd->dex            = (short)        strtol (&p[col[18][0]], NULL, 10);
	cd->luk            = (short)        strtol (&p[col[19][0]], NULL, 10);
	cd->status_point   = (unsigned int) strtoul(&p[col[20][0]], NULL, 10);
	cd->skill_point    = (unsigned int) strtoul(&p[col[21][0]], NULL, 10);
	cd->option         = (unsigned int) strtoul(&p[col[22][0]], NULL, 10);
	cd->karma          = (unsigned char)strtoul(&p[col[23][0]], NULL, 10);
	cd->manner         = (short)        strtol (&p[col[24][0]], NULL, 10);
	cd->party_id       = (int)          strtol (&p[col[25][0]], NULL, 10);
	cd->guild_id       = (int)          strtol (&p[col[26][0]], NULL, 10);
	cd->pet_id         = (int)          strtol (&p[col[27][0]], NULL, 10);
	cd->hom_id         = (int)          strtol (&p[col[28][0]], NULL, 10);
	cd->mer_id         = (int)          strtol (&p[col[29][0]], NULL, 10);
	cd->hair           = (short)        strtol (&p[col[30][0]], NULL, 10);
	cd->hair_color     = (short)        strtol (&p[col[31][0]], NULL, 10);
	cd->clothes_color  = (short)        strtol (&p[col[32][0]], NULL, 10);
	cd->weapon         = (short)        strtol (&p[col[33][0]], NULL, 10);
	cd->shield         = (short)        strtol (&p[col[34][0]], NULL, 10);
	cd->head_top       = (short)        strtol (&p[col[35][0]], NULL, 10);
	cd->head_mid       = (short)        strtol (&p[col[36][0]], NULL, 10);
	cd->head_bottom    = (short)        strtol (&p[col[37][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[38][0]], col[38][1]-col[38][0]+1); cd->last_point.map = mapindex_name2id(tmp_map);
	cd->last_point.x   = (short)        strtol (&p[col[39][0]], NULL, 10);
	cd->last_point.y   = (short)        strtol (&p[col[40][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[41][0]], col[41][1]-col[41][0]+1); cd->save_point.map = mapindex_name2id(tmp_map);
	cd->save_point.x   = (short)        strtol (&p[col[42][0]], NULL, 10);
	cd->save_point.y   = (short)        strtol (&p[col[43][0]], NULL, 10);
	cd->partner_id     = (int)          strtol (&p[col[44][0]], NULL, 10);
	cd->father         = (int)          strtol (&p[col[45][0]], NULL, 10);
	cd->mother         = (int)          strtol (&p[col[46][0]], NULL, 10);
	cd->child          = (int)          strtol (&p[col[47][0]], NULL, 10);
	cd->arch_calls     = (int)          strtol (&p[col[48][0]], NULL, 10);
	cd->arch_faith     = (int)          strtol (&p[col[49][0]], NULL, 10);
	cd->spear_calls    = (int)          strtol (&p[col[50][0]], NULL, 10);
	cd->spear_faith    = (int)          strtol (&p[col[51][0]], NULL, 10);
	cd->sword_calls    = (int)          strtol (&p[col[52][0]], NULL, 10);
	cd->sword_faith    = (int)          strtol (&p[col[53][0]], NULL, 10);

	return true;
}

bool upgrade_20090810_to_20090825(char* str, struct chardata_20090810* d, struct chardata_20090825* cd)
{
	cd->char_id = d->char_id;
	cd->account_id = d->account_id;
	cd->slot = d->slot;
	safestrncpy(cd->name, d->name, NAME_LENGTH);
	cd->class_ = d->class_;
	cd->base_level = d->base_level;
	cd->job_level = d->job_level;
	cd->base_exp = d->base_exp;
	cd->job_exp = d->job_exp;
	cd->zeny = d->zeny;
	cd->hp = d->hp;
	cd->max_hp = d->max_hp;
	cd->sp = d->sp;
	cd->max_sp = d->max_sp;
	cd->str = d->str;
	cd->agi = d->agi;
	cd->vit = d->vit;
	cd->int_ = d->int_;
	cd->dex = d->dex;
	cd->luk = d->luk;
	cd->status_point = d->status_point;
	cd->skill_point = d->skill_point;
	cd->option = d->option;
	cd->karma = d->karma;
	cd->manner = d->manner;
	cd->party_id = d->party_id;
	cd->guild_id = d->guild_id;
	cd->pet_id = d->pet_id;
	cd->hom_id = d->hom_id;
	cd->mer_id = 0;
	cd->hair = d->hair;
	cd->hair_color = d->hair_color;
	cd->clothes_color = d->clothes_color;
	cd->weapon = d->weapon;
	cd->shield = d->shield;
	cd->head_top = d->head_top;
	cd->head_mid = d->head_mid;
	cd->head_bottom = d->head_bottom;
	cd->last_point = d->last_point;
	cd->save_point = d->save_point;
	cd->partner_id = d->partner_id;
	cd->father = d->father;
	cd->mother = d->mother;
	cd->child = d->child;
	cd->arch_calls = 0;
	cd->arch_faith = 0;
	cd->spear_calls = 0;
	cd->spear_faith = 0;
	cd->sword_calls = 0;
	cd->sword_faith = 0;

	// split off fame to ranks.txt
	if( d->fame > 0 )
	{
		int rank_id;

		FILE* fw = fopen(file_ranks, "a");
		if( fw == NULL )
			return false;

		if( cd->class_ == JOB_BLACKSMITH || cd->class_ == JOB_WHITESMITH || cd->class_ == JOB_BABY_BLACKSMITH )
			rank_id = 1;
		else
		if( cd->class_ == JOB_ALCHEMIST || cd->class_ == JOB_CREATOR || cd->class_ == JOB_BABY_ALCHEMIST )
			rank_id = 2;
		else
		if( cd->class_ == JOB_TAEKWON )
			rank_id = 3;
		else
			return false;

		fprintf(fw, "%d\n", 20090210); // version
		fprintf(fw, "%d\t%d\t%d\n", rank_id, cd->char_id, d->fame);

		fclose(fw);
	}

	// split off memo points to memo.txt
	{
		char buf[1024];
		char* p = buf;
		bool first = true;
		int i;

		for( i = 0; i < MAX_MEMOPOINTS; ++i )
		{
			if( d->memo_point[i].map == 0 )
				continue;

			if( first )
				first = false;
			else
				p += sprintf(p, " ");

			p += sprintf(p, "%s,%d,%d", mapindex_id2name(d->memo_point[i].map), d->memo_point[i].x, d->memo_point[i].y);
		}

		if( p != buf )
		{
			FILE* fw = fopen(file_memos, "a");
			if( fw == NULL )
				return false;

			fprintf(fw, "%d\n", 20090825); // version
			fprintf(fw, "%d\t%s\n", cd->char_id, buf);

			fclose(fw);
		}
	}

	// split off inventory to inventory.txt
	{
		char buf[65536];
		char* p = buf;
		bool first = true;
		int i, j;

		for( i = 0; i < MAX_INVENTORY; ++i )
		{
			if( d->inventory[i].nameid == 0 || d->inventory[i].amount == 0 )
				continue;

			if( first )
				first = false;
			else
				p += sprintf(p, " ");

			p += sprintf(p, "%d,%d,%d,%d,%d,%d,%d",
				d->inventory[i].id, d->inventory[i].nameid, d->inventory[i].amount, d->inventory[i].equip,
				d->inventory[i].identify, d->inventory[i].refine, d->inventory[i].attribute);

			for( j = 0; j < MAX_SLOTS; ++j )
				p += sprintf(p, ",%d", d->inventory[i].card[j]);
		}

		if( p != buf )
		{
			FILE* fw = fopen(file_inventories, "a");
			if( fw == NULL )
				return false;

			fprintf(fw, "%d\n", 20090825); // version
			fprintf(fw, "%d\t%s\n", cd->char_id, buf);

			fclose(fw);
		}
	}

	// split off cart inventory to cart.txt
	{
		char buf[65536];
		char* p = buf;
		bool first = true;
		int i, j;

		for( i = 0; i < MAX_CART; ++i )
		{
			if( d->cart[i].nameid == 0 || d->cart[i].amount == 0 )
				continue;

			if( first )
				first = false;
			else
				p += sprintf(p, " ");

			p += sprintf(p, "%d,%d,%d,%d,%d,%d,%d",
				d->cart[i].id, d->cart[i].nameid, d->cart[i].amount, d->cart[i].equip,
				d->cart[i].identify, d->cart[i].refine, d->cart[i].attribute);

			for( j = 0; j < MAX_SLOTS; ++j )
				p += sprintf(p, ",%d", d->cart[i].card[j]);
		}

		if( p != buf )
		{
			FILE* fw = fopen(file_carts, "a");
			if( fw == NULL )
				return false;

			fprintf(fw, "%d\n", 20090825); // version
			fprintf(fw, "%d\t%s\n", cd->char_id, buf);

			fclose(fw);
		}
	}

	// split off skill list to skill.txt
	{
		char buf[65536];
		char* p = buf;
		bool first = true;
		int i;

		for( i = 0; i < MAX_SKILL; ++i )
		{
			if( d->skill[i].id == 0 || d->skill[i].flag == 1 )
				continue;

			if( first )
				first = false;
			else
				p += sprintf(p, " ");

			p += sprintf(p, "%d,%d", d->skill[i].id, (d->skill[i].flag == 0) ? d->skill[i].lv : d->skill[i].flag-2);
		}

		if( p != buf )
		{
			FILE* fw = fopen(file_skills, "a");
			if( fw == NULL )
				return false;

			fprintf(fw, "%d\n", 20090825); // version
			fprintf(fw, "%d\t%s\n", cd->char_id, buf);

			fclose(fw);
		}
	}

	// split off char regs to charreg.txt
	{
		char buf[65536];
		char* p = buf;
		bool first = true;
		int i;

		for( i = 0; i < d->reg.reg_num; ++i )
		{
			char esc_str[4*32+1];
			char esc_value[4*256+1];

			if( d->reg.reg[i].str[0] == '\0' )
				continue;

			if( first )
				first = false;
			else
				p += sprintf(p, " ");

			sv_escape_c(esc_str, d->reg.reg[i].str, strlen(d->reg.reg[i].str), " ,");
			sv_escape_c(esc_value, d->reg.reg[i].value, strlen(d->reg.reg[i].value), " ");
			p += sprintf(p, "%s,%s", esc_str, esc_value);
		}

		if( p != buf )
		{
			FILE* fw = fopen(file_charregs, "a");
			if( fw == NULL )
				return false;

			fprintf(fw, "%d\n", 20090825); // version
			fprintf(fw, "%d\t%s\n", cd->char_id, buf);

			fclose(fw);
		}
	}

	return true;
}
//////////////////////////////////////////
/// version 20090825 end
//////////////////////////////////////////


/// Performs an in-place upgrade of the data in 'line'.
bool upgrade(char* line, unsigned int version)
{
	struct chardata_00000000 cd_00000000;
	struct chardata_20090810 cd_20090810;
	struct chardata_20090825 cd_20090825;

	switch( version )
	{
	default:
		return false;
	case 00000000: if( !load_from_00000000(line, &cd_00000000) ) return false; break;
	case 20090810: if( !load_from_20090810(line, &cd_20090810) ) return false; break;
	case 20090825: if( !load_from_20090825(line, &cd_20090825) ) return false; break;
	}

	switch( version )
	{
	default:
		return false;
	case 00000000: if( !upgrade_00000000_to_20090810(line, &cd_00000000, &cd_20090810) ) return false;
	case 20090810: if( !upgrade_20090810_to_20090825(line, &cd_20090810, &cd_20090825) ) return false;
	case 20090825: if( !write_latest(line, &cd_20090825) ) return false;
	}

	return true;
}


int main(int argc, char** args)
{
	FILE* fr; // input
	FILE* fw; // output
	char line[65536];
	unsigned int version = 0;
	int next_char_id = START_CHAR_NUM;

	mapindex_init();

	if( argc < 3 )
	{
		fprintf(stderr, "eAthena txt character savefile upgrade tool\n");
		fprintf(stderr, "run this from the eAthena root directory\n");
		fprintf(stderr, "usage: txt-upgrade <character savefile path> <output directory>\n");
		return 1;
	}

	fr = fopen(args[1], "r");
	if( fr == NULL )
	{
		fprintf(stderr, "Failed to open file '%s'!\n", args[1]);
		return 1;
	}

	if( chdir(args[2]) != 0 )
	{
		fprintf(stderr, "Failed change directory to '%s'!\n", args[2]);
		return 1;
	}

	fw = fopen(file_chars, "w");
	if( fw == NULL )
	{
		fprintf(stderr, "Failed to create '%s'!\n", file_chars);
		return 1;
	}

	fprintf(fw, "%d\n", CHARDB_TXT_DB_VERSION);

	while( fgets(line, sizeof(line), fr) )
	{
		int char_id;
		int n;
		unsigned int v;

		if( line[0] == '/' && line[1] == '/' )
			continue;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &char_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( next_char_id < char_id )
				next_char_id = char_id;
			continue;
		}

		// in-place upgrade
		if( !upgrade(line, version) )
		{
			fprintf(stderr, "Upgrade failed!\n");
			return 1;
		}

		fprintf(fw, "%s\n", line);
	}

	fprintf(fw, "%d\t%%newid%%\n", next_char_id);

	fclose(fr);
	fclose(fw);

	mapindex_final();

	return 0;
}
