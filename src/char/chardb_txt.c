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
static bool mmo_char_fromstr(CharDB* chars, const char* str, struct mmo_charstatus* cd, struct regs* reg)
{
	char tmp_str[3][128]; //To avoid deleting chars with too long names.
	int tmp_int[256];
	unsigned int tmp_uint[2]; //To read exp....
	char tmp_name[NAME_LENGTH];
	int tmp_charid;
	int next, len, i, j;

	// initilialise character
	memset(cd, '\0', sizeof(struct mmo_charstatus));
	
// Char structure of version r13990 (mercenary owner data)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d,%d,%d,%d,%d"
		"\t%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[44],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		&tmp_int[45], &tmp_int[35], &tmp_int[36],
		&tmp_int[46], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43],
		&tmp_int[47], &tmp_int[48], &tmp_int[49], &tmp_int[50], &tmp_int[51], &tmp_int[52], &tmp_int[53], &next) != 55 || str[next] != '\t')
	{
	tmp_int[47] = 0; // mer_id
	tmp_int[48] = 0; // arch_calls
	tmp_int[49] = 0; // arch_faith
	tmp_int[50] = 0; // spear_calls
	tmp_int[51] = 0; // spear_faith
	tmp_int[52] = 0; // sword_calls
	tmp_int[53] = 0; // sword_faith
// Char structure of version 1500 (homun + mapindex maps)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[44],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		&tmp_int[45], &tmp_int[35], &tmp_int[36],
		&tmp_int[46], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) != 48)
	{
	tmp_int[44] = 0; //Hom ID.
// Char structure of version 1488 (fame field addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36],
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) != 47)
	{
	tmp_int[43] = 0; //Fame
// Char structure of version 1363 (family data addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &next) != 46)
	{
	tmp_int[40] = 0; // father
	tmp_int[41] = 0; // mother
	tmp_int[42] = 0; // child
// Char structure version 1008 (marriage partner addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], &next) != 43)
	{
	tmp_int[39] = 0; // partner id
// Char structure version 384 (pet addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &next) != 42)
	{
	tmp_int[26] = 0; // pet id
// Char structure of a version 1 (original data structure)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], //
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &next) != 41)
	{
		ShowError("Char-loading: Unrecognized character data version, info lost!\n");
		ShowDebug("Character info: %s\n", str);
		return false;
	}
	}	// Char structure version 384 (pet addition)
	}	// Char structure version 1008 (marriage partner addition)
	}	// Char structure of version 1363 (family data addition)
	}	// Char structure of version 1488 (fame field addition)
	//Convert save data from string to integer for older formats
		tmp_int[45] = mapindex_name2id(tmp_str[1]);
		tmp_int[46] = mapindex_name2id(tmp_str[2]);
	}	// Char structure of version 1500 (homun + mapindex maps)
	}	// Char structure of version r13990 (mercenary owner data)

	safestrncpy(cd->name, tmp_str[0], sizeof(cd->name));
	cd->char_id = tmp_int[0];
	cd->account_id = tmp_int[1];
	cd->slot = tmp_int[2];
	cd->class_ = tmp_int[3];
	cd->base_level = tmp_int[4];
	cd->job_level = tmp_int[5];
	cd->base_exp = tmp_uint[0];
	cd->job_exp = tmp_uint[1];
	cd->zeny = tmp_int[8];
	cd->hp = tmp_int[9];
	cd->max_hp = tmp_int[10];
	cd->sp = tmp_int[11];
	cd->max_sp = tmp_int[12];
	cd->str = tmp_int[13];
	cd->agi = tmp_int[14];
	cd->vit = tmp_int[15];
	cd->int_ = tmp_int[16];
	cd->dex = tmp_int[17];
	cd->luk = tmp_int[18];
	cd->status_point = tmp_int[19];
	cd->skill_point = tmp_int[20];
	cd->option = tmp_int[21];
	cd->karma = tmp_int[22];
	cd->manner = tmp_int[23];
	cd->party_id = tmp_int[24];
	cd->guild_id = tmp_int[25];
	cd->pet_id = tmp_int[26];
	cd->hair = tmp_int[27];
	cd->hair_color = tmp_int[28];
	cd->clothes_color = tmp_int[29];
	cd->weapon = tmp_int[30];
	cd->shield = tmp_int[31];
	cd->head_top = tmp_int[32];
	cd->head_mid = tmp_int[33];
	cd->head_bottom = tmp_int[34];
	cd->last_point.x = tmp_int[35];
	cd->last_point.y = tmp_int[36];
	cd->save_point.x = tmp_int[37];
	cd->save_point.y = tmp_int[38];
	cd->partner_id = tmp_int[39];
	cd->father = tmp_int[40];
	cd->mother = tmp_int[41];
	cd->child = tmp_int[42];
	cd->fame = tmp_int[43];
	cd->hom_id = tmp_int[44];
	cd->last_point.map = tmp_int[45];
	cd->save_point.map = tmp_int[46];
	cd->mer_id = tmp_int[47];
	cd->arch_calls = tmp_int[48];
	cd->arch_faith = tmp_int[49];
	cd->spear_calls = tmp_int[50];
	cd->spear_faith = tmp_int[51];
	cd->sword_calls = tmp_int[52];
	cd->sword_faith = tmp_int[53];

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

	if (str[next] == '\n' || str[next] == '\r')
		return false;	// 新規データ

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		//mapindex memo format
		if (sscanf(str+next, "%d,%d,%d%n", &tmp_int[2], &tmp_int[0], &tmp_int[1], &len) != 3)
		{	//Old string-based memo format.
			if (sscanf(str+next, "%[^,],%d,%d%n", tmp_str[0], &tmp_int[0], &tmp_int[1], &len) != 3)
				return false;
			tmp_int[2] = mapindex_name2id(tmp_str[0]);
		}
		if (i < MAX_MEMOPOINTS)
	  	{	//Avoid overflowing (but we must also read through all saved memos)
			cd->memo_point[i].x = tmp_int[0];
			cd->memo_point[i].y = tmp_int[1];
			cd->memo_point[i].map = tmp_int[2];
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str[0], &len) == 8)
		{
			cd->inventory[i].id = tmp_int[0];
			cd->inventory[i].nameid = tmp_int[1];
			cd->inventory[i].amount = tmp_int[2];
			cd->inventory[i].equip = tmp_int[3];
			cd->inventory[i].identify = tmp_int[4];
			cd->inventory[i].refine = tmp_int[5];
			cd->inventory[i].attribute = tmp_int[6];

			for(j = 0; j < MAX_SLOTS && tmp_str[0] && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				cd->inventory[i].card[j] = tmp_int[0];

			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return false;
	}
	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str[0], &len) == 8)
		{
			cd->cart[i].id = tmp_int[0];
			cd->cart[i].nameid = tmp_int[1];
			cd->cart[i].amount = tmp_int[2];
			cd->cart[i].equip = tmp_int[3];
			cd->cart[i].identify = tmp_int[4];
			cd->cart[i].refine = tmp_int[5];
			cd->cart[i].attribute = tmp_int[6];
			
			for(j = 0; j < MAX_SLOTS && tmp_str && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				cd->cart[i].card[j] = tmp_int[0];
			
			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return false;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return false;
		cd->skill[tmp_int[0]].id = tmp_int[0];
		cd->skill[tmp_int[0]].lv = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	// parse character regs
	if( !mmo_charreg_fromstr(reg, str + next) )
		return false;

	return true;
}


//-------------------------------------------------
// Function to create the character line (for save)
//-------------------------------------------------
static int mmo_char_tostr(char *str, struct mmo_charstatus *p, const struct regs* reg)
{
	int i,j;
	char *str_p = str;

	// base character data
	str_p += sprintf(str_p,
		"%d\t%d,%d\t%s\t%d,%d,%d\t%u,%u,%d" //Up to Zeny field
		"\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%u,%u" //Up to Skill Point
		"\t%d,%d,%d\t%d,%d,%d,%d" //Up to hom id
		"\t%d,%d,%d\t%d,%d,%d,%d,%d" //Up to head bottom
		"\t%d,%d,%d\t%d,%d,%d" //last point + save point
		",%d,%d,%d,%d,%d" //Family info
		"\t%d,%d,%d,%d,%d,%d,%d", // mercenary owner data
		p->char_id, p->account_id, p->slot, p->name, //
		p->class_, p->base_level, p->job_level,
		p->base_exp, p->job_exp, p->zeny,
		p->hp, p->max_hp, p->sp, p->max_sp,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
		p->status_point, p->skill_point,
		p->option, p->karma, p->manner,	//
		p->party_id, p->guild_id, p->pet_id, p->hom_id,
		p->hair, p->hair_color, p->clothes_color,
		p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
		p->last_point.map, p->last_point.x, p->last_point.y, //
		p->save_point.map, p->save_point.x, p->save_point.y,
		p->partner_id,p->father,p->mother,p->child,p->fame,
		p->mer_id, p->arch_calls, p->arch_faith, p->spear_calls, p->spear_faith, p->sword_calls, p->sword_faith);
	*(str_p++) = '\t';

	// memo points
	for( i = 0; i < MAX_MEMOPOINTS; i++ )
	{
		if( p->memo_point[i].map == 0 )
			continue;

		str_p += sprintf(str_p, "%d,%d,%d ", p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
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
	*(str_p++) = '\t';

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
		struct mmo_charstatus* ch;
		struct regs reg;
		line_count++;

		if( line[0] == '/' && line[1] == '/' )
			continue;

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
		if( !mmo_char_fromstr(self, line, ch, &reg) )
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
