// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "chardb.h"
#include "charlog.h"
#include "inter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// private declarations
char char_txt[1024] = "save/athena.txt";

struct mmo_charstatus* char_dat;
int char_num, char_max;
int char_id_count = START_CHAR_NUM;


extern DBMap* auth_db;
struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server; // -2: unknown server, -1: not connected, 0+: id of server
};
extern DBMap* online_char_db;
extern bool name_ignoring_case;
#define TRIM_CHARS "\032\t\x0A\x0D "

extern int mmo_friends_list_data_str(char *str, struct mmo_charstatus *p);
extern int mmo_hotkeys_tostr(char *str, struct mmo_charstatus *p);
extern int parse_friend_txt(struct mmo_charstatus *p);
extern int parse_hotkey_txt(struct mmo_charstatus *p);
extern void mmo_hotkeys_sync(void);
extern void mmo_friends_sync(void);
extern int autosave_interval;



//TODO:
// - search char data by account id (multiple results)
// - search char data by char id
// - search char data by account id and char id

void mmo_char_sync(void);


//-------------------------------------------------
// Function to create the character line (for save)
//-------------------------------------------------
int mmo_char_tostr(char *str, struct mmo_charstatus *p, struct global_reg *reg, int reg_num)
{
	int i,j;
	char *str_p = str;

	// character data
	str_p += sprintf(str_p,
		"%d\t%d,%d\t%s\t%d,%d,%d\t%u,%u,%d" //Up to Zeny field
		"\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d" //Up to Skill Point
		"\t%d,%d,%d\t%d,%d,%d,%d" //Up to hom id
		"\t%d,%d,%d\t%d,%d,%d,%d,%d" //Up to head bottom
		"\t%d,%d,%d\t%d,%d,%d" //last point + save point
		",%d,%d,%d,%d,%d\t",	//Family info
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
		p->partner_id,p->father,p->mother,p->child,p->fame);
	for(i = 0; i < MAX_MEMOPOINTS; i++)
		if (p->memo_point[i].map) {
			str_p += sprintf(str_p, "%d,%d,%d ", p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
		}
	*(str_p++) = '\t';

	// inventory
	for(i = 0; i < MAX_INVENTORY; i++)
		if (p->inventory[i].nameid) {
			str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
				p->inventory[i].id,p->inventory[i].nameid,p->inventory[i].amount,p->inventory[i].equip,
				p->inventory[i].identify,p->inventory[i].refine,p->inventory[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += sprintf(str_p,",%d",p->inventory[i].card[j]);
			str_p += sprintf(str_p," ");
		}
	*(str_p++) = '\t';

	// cart
	for(i = 0; i < MAX_CART; i++)
		if (p->cart[i].nameid) {
			str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
				p->cart[i].id,p->cart[i].nameid,p->cart[i].amount,p->cart[i].equip,
				p->cart[i].identify,p->cart[i].refine,p->cart[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += sprintf(str_p,",%d",p->cart[i].card[j]);
			str_p += sprintf(str_p," ");
		}
	*(str_p++) = '\t';

	// skills
	for(i = 0; i < MAX_SKILL; i++)
		if (p->skill[i].id && p->skill[i].flag != 1) {
			str_p += sprintf(str_p, "%d,%d ", p->skill[i].id, (p->skill[i].flag == 0) ? p->skill[i].lv : p->skill[i].flag-2);
		}
	*(str_p++) = '\t';

	// registry
	for(i = 0; i < reg_num; i++)
		if (reg[i].str[0])
			str_p += sprintf(str_p, "%s,%s ", reg[i].str, reg[i].value);
	*(str_p++) = '\t';

	*str_p = '\0';
	return 0;
}

//-------------------------------------------------------------------------
// Function to set the character from the line (at read of characters file)
//-------------------------------------------------------------------------
int mmo_char_fromstr(char *str, struct mmo_charstatus *p, struct global_reg *reg, int *reg_num)
{
	char tmp_str[3][128]; //To avoid deleting chars with too long names.
	int tmp_int[256];
	unsigned int tmp_uint[2]; //To read exp....
	int next, len, i, j;

	// initilialise character
	memset(p, '\0', sizeof(struct mmo_charstatus));
	
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
		return 0;
	}
	}	// Char structure version 384 (pet addition)
	}	// Char structure version 1008 (marriage partner addition)
	}	// Char structure of version 1363 (family data addition)
	}	// Char structure of version 1488 (fame field addition)
	//Convert save data from string to integer for older formats
		tmp_int[45] = mapindex_name2id(tmp_str[1]);
		tmp_int[46] = mapindex_name2id(tmp_str[2]);
	}	// Char structure of version 1500 (homun + mapindex maps)

	memcpy(p->name, tmp_str[0], NAME_LENGTH); //Overflow protection [Skotlex]
	p->char_id = tmp_int[0];
	p->account_id = tmp_int[1];
	p->slot = tmp_int[2];
	p->class_ = tmp_int[3];
	p->base_level = tmp_int[4];
	p->job_level = tmp_int[5];
	p->base_exp = tmp_uint[0];
	p->job_exp = tmp_uint[1];
	p->zeny = tmp_int[8];
	p->hp = tmp_int[9];
	p->max_hp = tmp_int[10];
	p->sp = tmp_int[11];
	p->max_sp = tmp_int[12];
	p->str = tmp_int[13];
	p->agi = tmp_int[14];
	p->vit = tmp_int[15];
	p->int_ = tmp_int[16];
	p->dex = tmp_int[17];
	p->luk = tmp_int[18];
	p->status_point = min(tmp_int[19], USHRT_MAX);
	p->skill_point = min(tmp_int[20], USHRT_MAX);
	p->option = tmp_int[21];
	p->karma = tmp_int[22];
	p->manner = tmp_int[23];
	p->party_id = tmp_int[24];
	p->guild_id = tmp_int[25];
	p->pet_id = tmp_int[26];
	p->hair = tmp_int[27];
	p->hair_color = tmp_int[28];
	p->clothes_color = tmp_int[29];
	p->weapon = tmp_int[30];
	p->shield = tmp_int[31];
	p->head_top = tmp_int[32];
	p->head_mid = tmp_int[33];
	p->head_bottom = tmp_int[34];
	p->last_point.x = tmp_int[35];
	p->last_point.y = tmp_int[36];
	p->save_point.x = tmp_int[37];
	p->save_point.y = tmp_int[38];
	p->partner_id = tmp_int[39];
	p->father = tmp_int[40];
	p->mother = tmp_int[41];
	p->child = tmp_int[42];
	p->fame = tmp_int[43];
	p->hom_id = tmp_int[44];
	p->last_point.map = tmp_int[45];
	p->save_point.map = tmp_int[46];

#ifndef TXT_SQL_CONVERT
	// Some checks
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].char_id == p->char_id) {
			ShowError(CL_RED"mmmo_auth_init: a character has an identical id to another.\n");
			ShowError("               character id #%d -> new character not readed.\n", p->char_id);
			ShowError("               Character saved in log file."CL_RESET"\n");
			return -1;
		} else if (strcmp(char_dat[i].name, p->name) == 0) {
			ShowError(CL_RED"mmmo_auth_init: a character name already exists.\n");
			ShowError("               character name '%s' -> new character not read.\n", p->name);
			ShowError("               Character saved in log file."CL_RESET"\n");
			return -2;
		}
	}

	if (strcmpi(wisp_server_name, p->name) == 0) {
		ShowWarning("mmo_auth_init: ******WARNING: character name has wisp server name.\n");
		ShowWarning("               Character name '%s' = wisp server name '%s'.\n", p->name, wisp_server_name);
		ShowWarning("               Character readed. Suggestion: change the wisp server name.\n");
		char_log("mmo_auth_init: ******WARNING: character name has wisp server name: Character name '%s' = wisp server name '%s'.\n",
		          p->name, wisp_server_name);
	}
#endif //TXT_SQL_CONVERT
	if (str[next] == '\n' || str[next] == '\r')
		return 1;	// 新規データ

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		//mapindex memo format
		if (sscanf(str+next, "%d,%d,%d%n", &tmp_int[2], &tmp_int[0], &tmp_int[1], &len) != 3)
		{	//Old string-based memo format.
			if (sscanf(str+next, "%[^,],%d,%d%n", tmp_str[0], &tmp_int[0], &tmp_int[1], &len) != 3)
				return -3;
			tmp_int[2] = mapindex_name2id(tmp_str[0]);
		}
		if (i < MAX_MEMOPOINTS)
	  	{	//Avoid overflowing (but we must also read through all saved memos)
			p->memo_point[i].x = tmp_int[0];
			p->memo_point[i].y = tmp_int[1];
			p->memo_point[i].map = tmp_int[2];
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
			p->inventory[i].id = tmp_int[0];
			p->inventory[i].nameid = tmp_int[1];
			p->inventory[i].amount = tmp_int[2];
			p->inventory[i].equip = tmp_int[3];
			p->inventory[i].identify = tmp_int[4];
			p->inventory[i].refine = tmp_int[5];
			p->inventory[i].attribute = tmp_int[6];

			for(j = 0; j < MAX_SLOTS && tmp_str[0] && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				p->inventory[i].card[j] = tmp_int[0];

			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return -4;
	}
	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str[0], &len) == 8)
		{
			p->cart[i].id = tmp_int[0];
			p->cart[i].nameid = tmp_int[1];
			p->cart[i].amount = tmp_int[2];
			p->cart[i].equip = tmp_int[3];
			p->cart[i].identify = tmp_int[4];
			p->cart[i].refine = tmp_int[5];
			p->cart[i].attribute = tmp_int[6];
			
			for(j = 0; j < MAX_SLOTS && tmp_str && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				p->cart[i].card[j] = tmp_int[0];
			
			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return -5;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return -6;
		p->skill[tmp_int[0]].id = tmp_int[0];
		p->skill[tmp_int[0]].lv = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++) { // global_reg実装以前のathena.txt互換のため一応'\n'チェック
		if (sscanf(str + next, "%[^,],%[^ ] %n", reg[i].str, reg[i].value, &len) != 2) { 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if (str[next] == ',' && sscanf(str + next, ",%[^ ] %n", reg[i].value, &len) == 1) 
				i--;
			else
				return -7;
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}
	*reg_num = i;

	return 1;
}


//-----------------------------------
// Function to create a new character
//-----------------------------------
int make_new_char(struct char_session_data* sd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	char name[NAME_LENGTH];
	int i;
	
	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name,TRIM_CHARS);

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// check content of character name
	if( remove_control_chars(name) )
		return -2; // control chars in name

	// check for reserved names
	if( strcmpi(name, main_chat_nick) == 0 || strcmpi(name, wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// Check Authorised letters/symbols in the name of the character
	if( char_name_option == 1 ) { // only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) == NULL )
				return -2;
	} else
	if( char_name_option == 2 ) { // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) != NULL )
				return -2;
	} // else, all letters/symbols are authorised (except control char removed before)

	// check name (already in use?)
	ARR_FIND( 0, char_num, i,
		(name_ignoring_case && strncmp(char_dat[i].name, name, NAME_LENGTH) == 0) ||
		(!name_ignoring_case && strncmpi(char_dat[i].name, name, NAME_LENGTH) == 0) );
	if( i < char_num )
		return -1; // name already exists

	//check other inputs
	if((slot >= MAX_CHARS) // slots
	|| (hair_style >= 24) // hair style
	|| (hair_color >= 9) // hair color
	|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
	|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
	|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
		return -2; // invalid input

	// check char slot
	ARR_FIND( 0, char_num, i, char_dat[i].account_id == sd->account_id && char_dat[i].slot == slot );
	if( i < char_num )
		return -2; // slot already in use

	if (char_num >= char_max) {
		char_max += 256;
		RECREATE(char_dat, struct mmo_charstatus, char_max);
		if (!char_dat) {
			ShowFatalError("Out of memory: make_new_char (realloc of char_dat).\n");
			char_log("Out of memory: make_new_char (realloc of char_dat).\n");
			exit(EXIT_FAILURE);
		}
	}

	// validation success, log result
	char_log("make new char: account: %d, slot %d, name: %s, stats: %d/%d/%d/%d/%d/%d, hair: %d, hair color: %d.\n",
	         sd->account_id, slot, name, str, agi, vit, int_, dex, luk, hair_style, hair_color);

	i = char_num;
	memset(&char_dat[i], 0, sizeof(char_dat[i]));

	char_dat[i].char_id = char_id_count++;
	char_dat[i].account_id = sd->account_id;
	char_dat[i].slot = slot;
	safestrncpy(char_dat[i].name,name,NAME_LENGTH);
	char_dat[i].class_ = 0;
	char_dat[i].base_level = 1;
	char_dat[i].job_level = 1;
	char_dat[i].base_exp = 0;
	char_dat[i].job_exp = 0;
	char_dat[i].zeny = start_zeny;
	char_dat[i].str = str;
	char_dat[i].agi = agi;
	char_dat[i].vit = vit;
	char_dat[i].int_ = int_;
	char_dat[i].dex = dex;
	char_dat[i].luk = luk;
	char_dat[i].max_hp = 40 * (100 + char_dat[i].vit) / 100;
	char_dat[i].max_sp = 11 * (100 + char_dat[i].int_) / 100;
	char_dat[i].hp = char_dat[i].max_hp;
	char_dat[i].sp = char_dat[i].max_sp;
	char_dat[i].status_point = 0;
	char_dat[i].skill_point = 0;
	char_dat[i].option = 0;
	char_dat[i].karma = 0;
	char_dat[i].manner = 0;
	char_dat[i].party_id = 0;
	char_dat[i].guild_id = 0;
	char_dat[i].hair = hair_style;
	char_dat[i].hair_color = hair_color;
	char_dat[i].clothes_color = 0;
	char_dat[i].inventory[0].nameid = start_weapon; // Knife
	char_dat[i].inventory[0].amount = 1;
	char_dat[i].inventory[0].identify = 1;
	char_dat[i].inventory[1].nameid = start_armor; // Cotton Shirt
	char_dat[i].inventory[1].amount = 1;
	char_dat[i].inventory[1].identify = 1;
	char_dat[i].weapon = 0; // W_FIST
	char_dat[i].shield = 0;
	char_dat[i].head_top = 0;
	char_dat[i].head_mid = 0;
	char_dat[i].head_bottom = 0;
	memcpy(&char_dat[i].last_point, &start_point, sizeof(start_point));
	memcpy(&char_dat[i].save_point, &start_point, sizeof(start_point));
	char_num++;

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", sd->account_id, i, slot, name);
	mmo_char_sync();
	return i;
}


//---------------------------------
// Function to read characters file
//---------------------------------
int mmo_char_init(void)
{
	char line[65536];
	int ret, line_count;
	FILE* fp;

	char_num = 0;
	char_max = 0;
	char_dat = NULL;

	fp = fopen(char_txt, "r");

	if (fp == NULL) {
		ShowError("Characters file not found: %s.\n", char_txt);
		char_log("Characters file not found: %s.\n", char_txt);
		char_log("Id for the next created character: %d.\n", char_id_count);
		return 0;
	}

	line_count = 0;
	while(fgets(line, sizeof(line), fp))
	{
		int i, j;
		line_count++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		j = 0;
		if (sscanf(line, "%d\t%%newid%%%n", &i, &j) == 1 && j > 0) {
			if (char_id_count < i)
				char_id_count = i;
			continue;
		}

		if (char_num >= char_max) {
			char_max += 256;
			char_dat = (struct mmo_charstatus*)aRealloc(char_dat, sizeof(struct mmo_charstatus) * char_max);
			if (!char_dat) {
				ShowFatalError("Out of memory: mmo_char_init (realloc of char_dat).\n");
				char_log("Out of memory: mmo_char_init (realloc of char_dat).\n");
				exit(EXIT_FAILURE);
			}
		}

		ret = mmo_char_fromstr(line, &char_dat[char_num], char_dat[char_num].global, &char_dat[char_num].global_num);

		// Initialize friends list
		parse_friend_txt(&char_dat[char_num]);  // Grab friends for the character
		// Initialize hotkey list
		parse_hotkey_txt(&char_dat[char_num]);  // Grab hotkeys for the character
		
		if (ret > 0) { // negative value or zero for errors
			if (char_dat[char_num].char_id >= char_id_count)
				char_id_count = char_dat[char_num].char_id + 1;
			char_num++;
		} else {
			ShowError("mmo_char_init: in characters file, unable to read the line #%d.\n", line_count);
			ShowError("               -> Character saved in log file.\n");
			switch (ret) {
			case -1:
				char_log("Duplicate character id in the next character line (character not readed):\n");
				break;
			case -2:
				char_log("Duplicate character name in the next character line (character not readed):\n");
				break;
			case -3:
				char_log("Invalid memo point structure in the next character line (character not readed):\n");
				break;
			case -4:
				char_log("Invalid inventory item structure in the next character line (character not readed):\n");
				break;
			case -5:
				char_log("Invalid cart item structure in the next character line (character not readed):\n");
				break;
			case -6:
				char_log("Invalid skill structure in the next character line (character not readed):\n");
				break;
			case -7:
				char_log("Invalid register structure in the next character line (character not readed):\n");
				break;
			default: // 0
				char_log("Unabled to get a character in the next line - Basic structure of line (before inventory) is incorrect (character not readed):\n");
				break;
			}
			char_log("%s", line);
		}
	}
	fclose(fp);

	if (char_num == 0) {
		ShowNotice("mmo_char_init: No character found in %s.\n", char_txt);
		char_log("mmo_char_init: No character found in %s.\n", char_txt);
	} else if (char_num == 1) {
		ShowStatus("mmo_char_init: 1 character read in %s.\n", char_txt);
		char_log("mmo_char_init: 1 character read in %s.\n", char_txt);
	} else {
		ShowStatus("mmo_char_init: %d characters read in %s.\n", char_num, char_txt);
		char_log("mmo_char_init: %d characters read in %s.\n", char_num, char_txt);
	}

	char_log("Id for the next created character: %d.\n", char_id_count);

	return 0;
}

//---------------------------------------------------------
// Function to save characters in files (speed up by [Yor])
//---------------------------------------------------------
void mmo_char_sync(void)
{
	char line[65536];
	int lock;
	FILE *fp;
	int i;

	// Data save
	fp = lock_fopen(char_txt, &lock);
	if (fp == NULL) {
		ShowWarning("Server cannot save characters.\n");
		char_log("WARNING: Server cannot save characters.\n");
	} else {
		for(i = 0; i < char_num; i++)
		{
			mmo_char_tostr(line, &char_dat[i], char_dat[i].global, char_dat[i].global_num);
			fprintf(fp, "%s\n", line);
		}
		fprintf(fp, "%d\t%%newid%%\n", char_id_count);
		lock_fclose(fp, char_txt, &lock);
	}

	mmo_friends_sync();

#ifdef HOTKEY_SAVING
	mmo_hotkeys_sync();
#endif
}

//----------------------------------------------------
// Function to save (in a periodic way) datas in files
//----------------------------------------------------
int mmo_char_sync_timer(int tid, unsigned int tick, int id, intptr data)
{
	if (save_log)
		ShowInfo("Saving all files...\n");
	mmo_char_sync();
	inter_save();
	return 0;
}

void mmo_char_sync_init(void)
{
	add_timer_func_list(mmo_char_sync_timer, "mmo_char_sync_timer");
	add_timer_interval(gettick() + 1000, mmo_char_sync_timer, 0, 0, autosave_interval);
}


//Search character data from the aid/cid givem
struct mmo_charstatus* search_character(int aid, int cid)
{
	int i;
	for (i = 0; i < char_num; i++) {
		if (char_dat[i].char_id == cid && char_dat[i].account_id == aid)
			return &char_dat[i];
	}
	return NULL;
}
	
struct mmo_charstatus* search_character_byname(char* character_name)
{
	int i = search_character_index(character_name);
	if (i == -1) return NULL;
	return &char_dat[i];
}

//----------------------------------------------
// Search an character id
//   (return character index or -1 (if not found))
//   If exact character name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 character is found
//   and similar to the searched name.
//----------------------------------------------
int search_character_index(char* character_name)
{
	int i, quantity, index;

	quantity = 0;
	index = -1;
	for(i = 0; i < char_num; i++) {
		// Without case sensitive check (increase the number of similar character names found)
		if (stricmp(char_dat[i].name, character_name) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(char_dat[i].name, character_name) == 0)
				return i;
			quantity++;
			index = i;
		}
	}
	// Here, the exact character name is not found
	// We return the found index of a similar account ONLY if there is 1 similar character
	if (quantity == 1)
		return index;

	// Exact character name is not found and 0 or more than 1 similar characters have been found ==> we say not found
	return -1;
}

//Loads a character's name and stores it in the buffer given (must be NAME_LENGTH in size)
//Returns 1 on found, 0 on not found (buffer is filled with Unknown char name)
int char_loadName(int char_id, char* name)
{
	int j;
	for( j = 0; j < char_num && char_dat[j].char_id != char_id; ++j )
		;// find char
	if( j < char_num )
		strncpy(name, char_dat[j].name, NAME_LENGTH);
	else
		strncpy(name, unknown_char_name, NAME_LENGTH);

	return (j < char_num) ? 1 : 0;
}

//Clears the given party id from all characters.
//Since sometimes the party format changes and parties must be wiped, this 
//method is required to prevent stress during the "party not found!" stages.
void char_clearparty(int party_id)
{
	int i;
	for(i = 0; i < char_num; i++)
  	{
		if (char_dat[i].party_id == party_id)
			char_dat[i].party_id = 0;
	}
}



int char_married(int pl1,int pl2)
{
	return (char_dat[pl1].char_id == char_dat[pl2].partner_id && char_dat[pl2].char_id == char_dat[pl1].partner_id);
}

int char_child(int parent_id, int child_id)
{
	return (char_dat[parent_id].child == char_dat[child_id].char_id && 
		((char_dat[parent_id].char_id == char_dat[child_id].father) || 
		(char_dat[parent_id].char_id == char_dat[child_id].mother)));		
}

int char_family(int cid1, int cid2, int cid3)
{
	int i, idx1 = -1, idx2 =-1;//, idx3 =-1;
	for(i = 0; i < char_num && (idx1 == -1 || idx2 == -1/* || idx3 == 1*/); i++)
  	{
		if (char_dat[i].char_id == cid1)
			idx1 = i;
		if (char_dat[i].char_id == cid2)
			idx2 = i;
//		if (char_dat[i].char_id == cid2)
//			idx3 = i;
	}
	if (idx1 == -1 || idx2 == -1/* || idx3 == -1*/)
  		return 0; //Some character not found??

	//Unless the dbs are corrupted, these 3 checks should suffice, even though 
	//we could do a lot more checks and force cross-reference integrity.
	if(char_dat[idx1].partner_id == cid2 &&
		char_dat[idx1].child == cid3)
		return cid3; //cid1/cid2 parents. cid3 child.

	if(char_dat[idx1].partner_id == cid3 &&
		char_dat[idx1].child == cid2)
		return cid2; //cid1/cid3 parents. cid2 child.

	if(char_dat[idx2].partner_id == cid3 &&
		char_dat[idx2].child == cid1)
		return cid1; //cid2/cid3 parents. cid1 child.
	return 0;
}
