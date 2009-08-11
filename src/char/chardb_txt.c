// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "charregdb.h"
#include "charserverdb_txt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// global defines
#define CHARDB_TXT_DB_VERSION 20090810
#define START_CHAR_NUM 1

// temporary stuff
extern bool mmo_charreg_tostr(const struct regs* reg, char* str);
extern bool mmo_charreg_fromstr(struct regs* reg, const char* str);


/// internal structure
typedef struct CharDB_TXT
{
	CharDB vtable;       // public interface

	CharServerDB_TXT* owner;
	DBMap* chars;        // in-memory character storage
	int next_char_id;    // auto_increment
	bool dirty;

	const char* char_db; // character data storage file
	bool case_sensitive; // how to look up usernames

} CharDB_TXT;


//-------------------------------------------------------------------------
// Function to set the character from the line (at read of characters file)
//-------------------------------------------------------------------------
static bool mmo_char_fromstr(CharDB* chars, const char* str, struct mmo_charstatus* cd, struct regs* reg, unsigned int version)
{
	int fields[32][2];
	int count;
	const char *key, *base, *memo, *inventory, *cart, *skills, *regs;
	const char* p;
	int n;

	char tmp_str[3][128]; //To avoid deleting chars with too long names.
	int tmp_int[256];
	char tmp_name[NAME_LENGTH];
	int tmp_charid;
	int i, j;

	// initilialise character
	memset(cd, '\0', sizeof(*cd));

	// extract tab-separated column blocks from str
	// block layout: <char id> <base data> <memo points> <inventory> <cart> <skills> <char regs>
	count = sv_parse(str, strlen(str), 0, '\t', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));

	// establish structure
	if( version == 20090810 && count == 7 )
	{
		key = &str[fields[1][0]];
		base = &str[fields[2][0]];
		memo = &str[fields[3][0]];
		inventory = &str[fields[4][0]];
		cart = &str[fields[5][0]];
		skills = &str[fields[6][0]];
		regs = &str[fields[7][0]];
	}
	else
	if( version == 0 && count == 20 )
	{
		key = &str[fields[1][0]];
		base = &str[fields[2][0]];
		memo = &str[fields[15][0]];
		inventory = &str[fields[16][0]];
		cart = &str[fields[17][0]];
		skills = &str[fields[18][0]];
		regs = &str[fields[19][0]];
	}
	else
	{// unmatched row
		return false;
	}

	// key (char id)
	p = key;
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	// base data
	p = base;
	if( version == 20090810 )
	{// added mercenary owner block; using map names instead of indexes; using only commas as delimiters
		char tmp_map[MAP_NAME_LENGTH];
		int col[54+1][2];
		int cols = sv_parse(base, (ptrdiff_t)(memo - base), 0, ',', (int*)col, 2*ARRAYLENGTH(col), (e_svopt)(SV_ESCAPE_C));
		
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
	}
	else
	if( version == 0 )
	{
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

		p += n;
		if( sscanf(p, "\t%d,%d,%d,%d%n",
			&cd->party_id, &cd->guild_id, &cd->pet_id, &cd->hom_id,
			&n) != 4 || p[n] != '\t' )
		{
			cd->hom_id = 0;
			if( sscanf(p, "\t%d,%d,%d%n",
				&cd->party_id, &cd->guild_id, &cd->pet_id,
				&n) != 3 || p[n] != '\t' )
			{
				cd->pet_id = 0;
				if( sscanf(p, "\t%d,%d%n",
					&cd->party_id, &cd->guild_id,
					&n) != 2 || p[n] != '\t' )
				{
					return false;
				}
			}
		}
	
		p += n;
		if( sscanf(p, "\t%hd,%hd,%hd\t%hd,%hd,%hd,%hd,%hd%n",
			&cd->hair, &cd->hair_color, &cd->clothes_color,
			&cd->weapon, &cd->shield, &cd->head_top, &cd->head_mid, &cd->head_bottom,
			&n) != 8 || p[n] != '\t' )
 			return false;

		p += n;
		if( sscanf(p, "\t%hu,%hd,%hd\t%hu,%hd,%hd%n",
			&cd->last_point.map, &cd->last_point.x, &cd->last_point.y,
			&cd->save_point.map, &cd->save_point.x, &cd->save_point.y,
			&n) == 6 && p[n] == ',' )
			;
		else
		if( sscanf(p, "\t%127[^,],%hd,%hd\t%127[^,],%hd,%hd%n",
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
		if( sscanf(p, ",%d,%d,%d,%d,%d%n",
			&cd->partner_id, &cd->father, &cd->mother, &cd->child, &cd->fame,
			&n) != 5 || p[n] != '\t' )
		{
			cd->fame = 0;
			if( sscanf(p, ",%d,%d,%d,%d%n",
				&cd->partner_id, &cd->father, &cd->mother, &cd->child,
				&n) != 4 || p[n] != '\t' )
			{
				cd->father = 0;
				cd->mother = 0;
				cd->child = 0;
				if( sscanf(p, ",%d%n",
					&cd->partner_id,
					&n) != 1 || p[n] != '\t' )
				{
					cd->partner_id = 0;
					if( sscanf(p, "%n", &n) != 0 || p[n] != '\t' )
					{
						return false;
					}
				}
			}
		}
	}
	else
	{// unmatched row
		return false;
	}

	// memo data
	p = memo;
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

	// inventory items
	p = inventory;
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7];
		char tmp_str[256];

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &n) != 8 )
			return false;

		if( p[n] != ' ' )
 			return false;

		p += n + 1;

		if( i == MAX_INVENTORY )
			continue; // TODO: warning?

		cd->inventory[i].id = tmp_int[0];
		cd->inventory[i].nameid = tmp_int[1];
		cd->inventory[i].amount = tmp_int[2];
		cd->inventory[i].equip = tmp_int[3];
		cd->inventory[i].identify = tmp_int[4];
		cd->inventory[i].refine = tmp_int[5];
		cd->inventory[i].attribute = tmp_int[6];

		//FIXME: scanning from a buffer into the same buffer has undefined behavior
		for( j = 0; j < MAX_SLOTS && tmp_str[0] != '\0' && sscanf(tmp_str, ",%d%[0-9,-]", &tmp_int[0], tmp_str) > 0; j++ )
			cd->inventory[i].card[j] = tmp_int[0];
	}

	// cart items
	p = cart;
	for( i = 0; *p != '\0' && *p != '\t'; ++i )
	{
		int tmp_int[7];
		char tmp_str[256];

		if( sscanf(p, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &n) != 8 )
			return false;

		if( p[n] != ' ' )
			return false;

		p += n + 1;

		if( i == MAX_CART )
			continue; // TODO: warning?

		cd->cart[i].id = tmp_int[0];
		cd->cart[i].nameid = tmp_int[1];
		cd->cart[i].amount = tmp_int[2];
		cd->cart[i].equip = tmp_int[3];
		cd->cart[i].identify = tmp_int[4];
		cd->cart[i].refine = tmp_int[5];
		cd->cart[i].attribute = tmp_int[6];
		
		//FIXME: scanning from a buffer into the same buffer has undefined behavior
		for( j = 0; j < MAX_SLOTS && tmp_str[0] != '\0' && sscanf(tmp_str, ",%d%[0-9,-]", &tmp_int[0], tmp_str) > 0; j++ )
			cd->cart[i].card[j] = tmp_int[0];
	}

	// skills
	p = skills;
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

	// character regs
	p = regs;
	if( !mmo_charreg_fromstr(reg, p) )
		return false;

	// uniqueness checks
	if( chars->id2name(chars, cd->char_id, tmp_name) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on id %d between character '%s' and existing character '%s'!\n", cd->char_id, cd->name, tmp_name);
		return false;
	}
	if( chars->name2id(chars, cd->name, &tmp_charid, NULL) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on name '%s' between character %d and existing character %d!\n", cd->name, cd->char_id, tmp_charid);
		return false;
	}

	return true;
}


//-------------------------------------------------
// Function to create the character line (for save)
//-------------------------------------------------
static int mmo_char_tostr(char *str, struct mmo_charstatus *p, const struct regs* reg)
{
	int i,j;
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
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->fame);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_faith);

	*(str_p++) = '\t';

	// memo points
	for( i = 0; i < MAX_MEMOPOINTS; i++ )
	{
		if( p->memo_point[i].map == 0 )
			continue;

		str_p += sprintf(str_p, "%s,%d,%d ", mapindex_id2name(p->memo_point[i].map), p->memo_point[i].x, p->memo_point[i].y);
	}
	*(str_p++) = '\t';

	// inventory
	for( i = 0; i < MAX_INVENTORY; i++ )
	{
		if( p->inventory[i].nameid == 0 )
			continue;

		str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
			p->inventory[i].id,p->inventory[i].nameid,p->inventory[i].amount,p->inventory[i].equip,
			p->inventory[i].identify,p->inventory[i].refine,p->inventory[i].attribute);

		for( j = 0; j < MAX_SLOTS; j++ )
			str_p += sprintf(str_p,",%d",p->inventory[i].card[j]);

		str_p += sprintf(str_p," ");
	}
	*(str_p++) = '\t';

	// cart
	for( i = 0; i < MAX_CART; i++ )
	{
		if( p->cart[i].nameid == 0 )
			continue;

		str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
			p->cart[i].id,p->cart[i].nameid,p->cart[i].amount,p->cart[i].equip,
			p->cart[i].identify,p->cart[i].refine,p->cart[i].attribute);

		for( j = 0; j < MAX_SLOTS; j++ )
			str_p += sprintf(str_p,",%d",p->cart[i].card[j]);

		str_p += sprintf(str_p," ");
	}
	*(str_p++) = '\t';

	// skills
	for( i = 0; i < MAX_SKILL; i++ )
	{
		if( p->skill[i].id && p->skill[i].flag != 1 )
			str_p += sprintf(str_p, "%d,%d ", p->skill[i].id, (p->skill[i].flag == 0) ? p->skill[i].lv : p->skill[i].flag-2);
	}
	*(str_p++) = '\t';

	// registry
	if( reg != NULL )
	{
		mmo_charreg_tostr(reg, str_p);
		str_p += strlen(str_p);
	}

	*str_p = '\0';
	return 0;
}


/// Dumps the entire char db (+ associated data) to disk
static bool mmo_char_sync(CharDB_TXT* db)
{
	CharRegDB* charregs = db->owner->charregdb;
	int lock;
	FILE *fp;
	void* data;
	struct DBIterator* iter;

	// Data save
	fp = lock_fopen(db->char_db, &lock);
	if( fp == NULL )
	{
		ShowWarning("Server cannot save characters.\n");
		return false;
	}

	fprintf(fp, "%d\n", CHARDB_TXT_DB_VERSION); // savefile version

	iter = db->chars->iterator(db->chars);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct mmo_charstatus* ch = (struct mmo_charstatus*) data;
		char line[65536]; // ought to be big enough
		struct regs reg;

		charregs->load(charregs, &reg, ch->char_id);
		mmo_char_tostr(line, ch, &reg);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_char_id);
	iter->destroy(iter);

	lock_fclose(fp, db->char_db, &lock);

	db->dirty = false;
	return true;
}


static bool char_db_txt_init(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	FriendDB* friends = db->owner->frienddb;
	HotkeyDB* hotkeys = db->owner->hotkeydb;
	CharRegDB* charregs = db->owner->charregdb;
	DBMap* chars;

	char line[65536];
	int line_count = 0;
	FILE* fp;
	unsigned int version = 0;

	// create chars database
	if( db->chars == NULL )
		db->chars = idb_alloc(DB_OPT_RELEASE_DATA);
	chars = db->chars;
	db_clear(chars);

	// open data file
	fp = fopen(db->char_db, "r");
	if( fp == NULL )
	{
		ShowError("Characters file not found: %s.\n", db->char_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int char_id, n;
		unsigned int v;
		struct mmo_charstatus* ch;
		struct regs reg;
		line_count++;

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
			if( char_id > db->next_char_id )
				db->next_char_id = char_id;
			continue;
		}

		// allocate memory for the char entry
		ch = (struct mmo_charstatus*)aMalloc(sizeof(struct mmo_charstatus));

		// parse char data
		if( !mmo_char_fromstr(self, line, ch, &reg, version) )
 		{
			ShowFatalError("char_db_txt_init: There was a problem processing data in file '%s', line #%d. Please fix manually. Shutting down to avoid data loss.\n", db->char_db, line_count);
			aFree(ch);
			exit(EXIT_FAILURE);
		}

		charregs->save(charregs, &reg, ch->char_id); // Initialize char regs

		// record entry in db
		idb_put(chars, ch->char_id, ch);

		if( ch->char_id >= db->next_char_id )
			db->next_char_id = ch->char_id + 1;
	}

	// close data file
	fclose(fp);

	ShowStatus("mmo_char_init: %d characters read in %s.\n", chars->size(chars), db->char_db);

	db->dirty = false;
	return true;
}

static void char_db_txt_destroy(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// delete chars database
	if( chars != NULL )
	{
		db_destroy(chars);
		db->chars = NULL;
	}

	// delete entire structure
	aFree(db);
}

static bool char_db_txt_sync(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	return mmo_char_sync(db);
}

static bool char_db_txt_create(CharDB* self, struct mmo_charstatus* cd)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	struct mmo_charstatus* tmp;

	// decide on the char id to assign
	int char_id = ( cd->char_id != -1 ) ? cd->char_id : db->next_char_id;

	// check if the char_id is free
	tmp = idb_get(chars, char_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("char_db_txt_create: cannot create character %d:'%s', this id is already occupied by %d:'%s'!\n", char_id, cd->name, char_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	tmp = (struct mmo_charstatus*)aMalloc(sizeof(struct mmo_charstatus));
	memcpy(tmp, cd, sizeof(struct mmo_charstatus));
	tmp->char_id = char_id;
	idb_put(chars, char_id, tmp);

	// increment the auto_increment value
	if( char_id >= db->next_char_id )
		db->next_char_id = char_id + 1;

	// write output
	cd->char_id = char_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool char_db_txt_remove(CharDB* self, const int char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	idb_remove(chars, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool char_db_txt_save(CharDB* self, const struct mmo_charstatus* ch)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	int char_id = ch->char_id;

	// retrieve previous data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, ch, sizeof(struct mmo_charstatus));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}

static bool char_db_txt_load_num(CharDB* self, struct mmo_charstatus* ch, int char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	RankDB* rankdb = db->owner->rankdb;
	DBMap* chars = db->chars;

	// retrieve data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(ch, tmp, sizeof(struct mmo_charstatus));
	switch( ch->class_ )
	{// TODO make the map-server responsible for this? (handle class2rankid logic) [FlavioJS]
	case JOB_BLACKSMITH:
	case JOB_WHITESMITH:
	case JOB_BABY_BLACKSMITH:
		ch->fame = rankdb->get_points(rankdb, RANK_BLACKSMITH, ch->char_id);
		break;
	case JOB_ALCHEMIST:
	case JOB_CREATOR:
	case JOB_BABY_ALCHEMIST:
		ch->fame = rankdb->get_points(rankdb, RANK_ALCHEMIST, ch->char_id);
		break;
	case JOB_TAEKWON:
		ch->fame = rankdb->get_points(rankdb, RANK_TAEKWON, ch->char_id);
		break;
	}

	return true;
}

static bool char_db_txt_load_str(CharDB* self, struct mmo_charstatus* ch, const char* name)
{
//	CharDB_TXT* db = (CharDB_TXT*)self;
	int char_id;

	// find char id
	if( !self->name2id(self, name, &char_id, NULL) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}

static bool char_db_txt_load_slot(CharDB* self, struct mmo_charstatus* ch, int account_id, int slot)
{
//	CharDB_TXT* db = (CharDB_TXT*)self;
	int char_id;

	// find char id
	if( !self->slot2id(self, account_id, slot, &char_id) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}

static bool char_db_txt_id2name(CharDB* self, int char_id, char name[NAME_LENGTH])
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( name != NULL )
		safestrncpy(name, tmp->name, sizeof(name));
	
	return true;
}

static bool char_db_txt_name2id(CharDB* self, const char* name, int* char_id, int* account_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct DBIterator* iter = chars->iterator(chars);
	struct mmo_charstatus* tmp;
	int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

	for( tmp = (struct mmo_charstatus*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_charstatus*)iter->next(iter,NULL) )
		if( compare(name, tmp->name) == 0 )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( char_id != NULL )
		*char_id = tmp->char_id;
	if( account_id != NULL )
		*account_id = tmp->account_id;

	return true;
}

static bool char_db_txt_slot2id(CharDB* self, int account_id, int slot, int* char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct DBIterator* iter = chars->iterator(chars);
	struct mmo_charstatus* tmp;

	for( tmp = (struct mmo_charstatus*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_charstatus*)iter->next(iter,NULL) )
		if( account_id == tmp->account_id && slot == tmp->slot )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( char_id != NULL )
		*char_id = tmp->char_id;

	return true;
}


/// Returns an iterator over all the characters.
static CSDBIterator* char_db_txt_iterator(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->chars));
}


/// internal structure
typedef struct CharDBIterator_TXT
{
	CSDBIterator vtable;      // public interface

	DBIterator* iter;
	int account_id;

} CharDBIterator_TXT;


/// Destroys this iterator, releasing all allocated memory (including itself).
static void char_db_txt_iter_destroy(CSDBIterator* self)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	dbi_destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next character.
static bool char_db_txt_iter_next(CSDBIterator* self, int* key)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	struct mmo_charstatus* tmp;

	while( true )
	{
		tmp = (struct mmo_charstatus*)dbi_next(iter->iter);
		if( tmp == NULL )
			return false;// not found

		if( iter->account_id != tmp->account_id )
			continue;// wrong account, try next

		if( key )
			*key = tmp->char_id;
		return true;
	}
}


/// Returns an iterator over all the characters of the account.
static CSDBIterator* char_db_txt_characters(CharDB* self, int account_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)aCalloc(1, sizeof(CharDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &char_db_txt_iter_destroy;
	iter->vtable.next    = &char_db_txt_iter_next;

	// fill data
	iter->iter = db_iterator(chars);
	iter->account_id = account_id;

	return &iter->vtable;
}


/// public constructor
CharDB* char_db_txt(CharServerDB_TXT* owner)
{
	CharDB_TXT* db = (CharDB_TXT*)aCalloc(1, sizeof(CharDB_TXT));

	// set up the vtable
	db->vtable.init      = &char_db_txt_init;
	db->vtable.destroy   = &char_db_txt_destroy;
	db->vtable.create    = &char_db_txt_create;
	db->vtable.remove    = &char_db_txt_remove;
	db->vtable.sync      = &char_db_txt_sync;
	db->vtable.save      = &char_db_txt_save;
	db->vtable.load_num  = &char_db_txt_load_num;
	db->vtable.load_str  = &char_db_txt_load_str;
	db->vtable.load_slot = &char_db_txt_load_slot;
	db->vtable.id2name   = &char_db_txt_id2name;
	db->vtable.name2id   = &char_db_txt_name2id;
	db->vtable.slot2id   = &char_db_txt_slot2id;
	db->vtable.iterator  = &char_db_txt_iterator;
	db->vtable.characters = &char_db_txt_characters;

	// initialize to default values
	db->owner = owner;
	db->chars = NULL;
	db->next_char_id = START_CHAR_NUM;
	db->dirty = false;

	// other settings
	db->char_db = db->owner->file_chars;
	db->case_sensitive = false;

	return &db->vtable;
}
