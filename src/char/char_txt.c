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
#include "../common/utils.h"
#include "chardb.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_status.h"
#include "inter.h"
#include "map.h"
#include <stdlib.h>
#include <string.h>

// temporary imports
extern CharDB* chars;
#include "char.h"
extern int mmo_hotkeys_tostr(char *str, struct mmo_charstatus *p);



char friends_txt[1024] = "save/friends.txt";
char hotkeys_txt[1024] = "save/hotkeys.txt";


// キャラ削除に伴うデータ削除
int char_delete(int char_id)
{
	struct mmo_charstatus cd, *cs;
	int j;

	cs = &cd;
	if( !chars->load_num(chars, &cd, char_id) )
		return 1;

	// ペット削除
	if (cs->pet_id)
		inter_pet_delete(cs->pet_id);
	if (cs->hom_id)
		inter_homun_delete(cs->hom_id);
	for (j = 0; j < MAX_INVENTORY; j++)
		if (cs->inventory[j].card[0] == (short)0xff00)
			inter_pet_delete(MakeDWord(cs->inventory[j].card[1],cs->inventory[j].card[2]));
	for (j = 0; j < MAX_CART; j++)
		if (cs->cart[j].card[0] == (short)0xff00)
			inter_pet_delete( MakeDWord(cs->cart[j].card[1],cs->cart[j].card[2]) );
	// ギルド脱退
	if (cs->guild_id)
		inter_guild_leave(cs->guild_id, cs->account_id, cs->char_id);
	// パーティー脱退
	if (cs->party_id)
		inter_party_leave(cs->party_id, cs->account_id, cs->char_id);
	// 離婚
	if (cs->partner_id)
		char_divorce(cs->char_id, cs->partner_id);

	inter_status_delete(cs->char_id);

	return 0;
}


/*---------------------------------------------------
  Make a data line for friends list
 --------------------------------------------------*/
int mmo_friends_list_data_str(char *str, struct mmo_charstatus *p)
{
	int i;
	char *str_p = str;
	str_p += sprintf(str_p, "%d", p->char_id);

	for (i=0;i<MAX_FRIENDS;i++){
		if (p->friends[i].account_id > 0 && p->friends[i].char_id > 0 && p->friends[i].name[0])
			str_p += sprintf(str_p, ",%d,%d,%s", p->friends[i].account_id, p->friends[i].char_id, p->friends[i].name);
	}

	str_p += '\0';

	return 0;
}

void mmo_friends_sync(void)
{
	char line[1024];
	int lock;
	FILE* fp;
	int i;

	// Friends List data save (davidsiaw)
	fp = lock_fopen(friends_txt, &lock);
/*
	for(i = 0; i < char_num; i++)
	{
		mmo_friends_list_data_str(line, &char_dat[i]);
		fprintf(fp, "%s\n", line);
	}
*/
	lock_fclose(fp, friends_txt, &lock);
}

void mmo_hotkeys_sync(void)
{
	char line[1024];
	int lock;
	FILE* fp;
	int i;

	// Hotkey List data save (Skotlex)
	fp = lock_fopen(hotkeys_txt, &lock);
/*
	for(i = 0; i < char_num; i++)
	{
		mmo_hotkeys_tostr(line, &char_dat[i]);
		fprintf(fp, "%s\n", line);
	}
*/
	lock_fclose(fp, hotkeys_txt, &lock);
}


/*---------------------------------------------------
  Make a data line for hotkeys list
 --------------------------------------------------*/
int mmo_hotkeys_tostr(char *str, struct mmo_charstatus *p)
{
#ifdef HOTKEY_SAVING
	int i;
	char *str_p = str;
	str_p += sprintf(str_p, "%d", p->char_id);
	for (i=0;i<MAX_HOTKEYS;i++)
		str_p += sprintf(str_p, ",%d,%d,%d", p->hotkeys[i].type, p->hotkeys[i].id, p->hotkeys[i].lv);
	str_p += '\0';
#endif

	return 0;
}


//---------------------------------
// Function to read friend list
//---------------------------------
int parse_friend_txt(struct mmo_charstatus *p)
{
	char line[1024], temp[1024];
	int pos = 0, count = 0, next;
	int i,len;
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(friends_txt, "r");

	if(fp == NULL)
		return -1;
	
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%d%n",&i, &pos) < 1 || i != p->char_id)
			continue; //Not this line...
		//Read friends
		len = strlen(line);
		next = pos;
		for (count = 0; next < len && count < MAX_FRIENDS; count++)
		{ //Read friends.
			if (sscanf(line+next, ",%d,%d,%23[^,^\n]%n",&p->friends[count].account_id,&p->friends[count].char_id, p->friends[count].name, &pos) < 3)
			{	//Invalid friend?
				memset(&p->friends[count], 0, sizeof(p->friends[count]));
				break;
			}
			next+=pos;
			//What IF the name contains a comma? while the next field is not a 
			//number, we assume it belongs to the current name. [Skotlex]
			//NOTE: Of course, this will fail if someone sets their name to something like
			//Bob,2005 but... meh, it's the problem of parsing a text file (encasing it in "
			//won't do as quotes are also valid name chars!)
			while(next < len && sscanf(line+next, ",%23[^,^\n]%n", temp, &pos) > 0)
			{
				if (atoi(temp)) //We read the next friend, just continue.
					break;
				//Append the name.
				next+=pos;
				i = strlen(p->friends[count].name);
				if (i + strlen(temp) +1 < NAME_LENGTH)
				{
					p->friends[count].name[i] = ',';
					strcpy(p->friends[count].name+i+1, temp);
				}
			} //End Guess Block
		} //Friend's for.
		break; //Found friends.
	}
	fclose(fp);
	return count;
}

//---------------------------------
// Function to read hotkey list
//---------------------------------
int parse_hotkey_txt(struct mmo_charstatus *p)
{
#ifdef HOTKEY_SAVING
	char line[1024];
	int pos = 0, count = 0, next;
	int i,len;
	int type, id, lv;
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(hotkeys_txt, "r");
	if(fp == NULL)
		return -1;
	
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%d%n",&i, &pos) < 1 || i != p->char_id)
			continue; //Not this line...
		//Read hotkeys 
		len = strlen(line);
		next = pos;
		for (count = 0; next < len && count < MAX_HOTKEYS; count++)
		{
			if (sscanf(line+next, ",%d,%d,%d%n",&type,&id,&lv, &pos) < 3)
				//Invalid entry?
				break;
			p->hotkeys[count].type = type;
			p->hotkeys[count].id = id;
			p->hotkeys[count].lv = lv;
			next+=pos;
		}
		break; //Found hotkeys.
	}
	fclose(fp);
	return count;
#else
	return 0;
#endif
}
