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
#include "charlog.h"
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
#include "char.h"
struct character_data {
	struct mmo_charstatus status;
	int global_num;
	struct global_reg global[GLOBAL_REG_NUM];
};
extern struct character_data *char_dat;
extern int login_fd;
extern int char_fd;
extern int char_num, char_max;
extern char wisp_server_name[NAME_LENGTH];
#define TRIM_CHARS "\032\t\x0A\x0D "
//Custom limits for the fame lists. [Skotlex]
extern int fame_list_size_chemist;
extern int fame_list_size_smith;
extern int fame_list_size_taekwon;
// Char-server-side stored fame lists [DracoRPG]
extern struct fame_list smith_fame_list[MAX_FAME_LIST];
extern struct fame_list chemist_fame_list[MAX_FAME_LIST];
extern struct fame_list taekwon_fame_list[MAX_FAME_LIST];
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
extern void set_all_offline(int id);
extern int mmo_hotkeys_tostr(char *str, struct mmo_charstatus *p);
extern void char_read_fame_list(void);



char friends_txt[1024] = "save/friends.txt";
char hotkeys_txt[1024] = "save/hotkeys.txt";



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

	for(i = 0; i < char_num; i++)
	{
		mmo_friends_list_data_str(line, &char_dat[i].status);
		fprintf(fp, "%s\n", line);
	}

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

	for(i = 0; i < char_num; i++)
	{
		mmo_hotkeys_tostr(line, &char_dat[i].status);
		fprintf(fp, "%s\n", line);
	}

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



// 離婚(char削除時に使用)
int char_divorce(struct mmo_charstatus *cs)
{
	int i, j;

	if (cs == NULL)
		return 0;

	if (cs->partner_id <= 0)
		return 0;
	
	ARR_FIND( 0, char_num, i, char_dat[i].status.char_id == cs->partner_id && char_dat[i].status.partner_id == cs->char_id );
	if( i == char_num )
		return 0;

	cs->partner_id = 0;
	char_dat[i].status.partner_id = 0;

	for(j = 0; j < MAX_INVENTORY; j++)
		if (char_dat[i].status.inventory[j].nameid == WEDDING_RING_M || char_dat[i].status.inventory[j].nameid == WEDDING_RING_F)
			memset(&char_dat[i].status.inventory[j], 0, sizeof(char_dat[i].status.inventory[0]));
		if (cs->inventory[j].nameid == WEDDING_RING_M || cs->inventory[j].nameid == WEDDING_RING_F)
			memset(&cs->inventory[j], 0, sizeof(cs->inventory[0]));

	return 0;
}


// キャラ削除に伴うデータ削除
int char_delete(struct mmo_charstatus *cs)
{
	int j;

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
	if (cs->partner_id){
		// 離婚情報をmapに通知
		unsigned char buf[10];
		WBUFW(buf,0) = 0x2b12;
		WBUFL(buf,2) = cs->char_id;
		WBUFL(buf,6) = cs->partner_id;
		mapif_sendall(buf,10);
		// 離婚
		char_divorce(cs);
	}
#ifdef ENABLE_SC_SAVING
	status_delete_scdata(cs->account_id, cs->char_id);
#endif
	return 0;
}


void char_read_fame_list(void)
{
	int i, j, k;
	struct fame_list fame_item;
	CREATE_BUFFER(id, int, char_num);

	for(i = 0; i < char_num; i++) {
		id[i] = i;
		for(j = 0; j < i; j++) {
			if (char_dat[i].status.fame > char_dat[id[j]].status.fame) {
				for(k = i; k > j; k--)
					id[k] = id[k-1];
				id[j] = i; // id[i]
				break;
			}
		}
	}

	// Empty ranking lists
	memset(smith_fame_list, 0, sizeof(smith_fame_list));
	memset(chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset(taekwon_fame_list, 0, sizeof(taekwon_fame_list));
	// Build Blacksmith ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_smith; i++) {
		if (char_dat[id[i]].status.fame && (
			char_dat[id[i]].status.class_ == JOB_BLACKSMITH ||
			char_dat[id[i]].status.class_ == JOB_WHITESMITH ||
			char_dat[id[i]].status.class_ == JOB_BABY_BLACKSMITH))
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&smith_fame_list[j],&fame_item,sizeof(struct fame_list));
			j++;
		}
	}
	// Build Alchemist ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_chemist; i++) {
		if (char_dat[id[i]].status.fame && (
			char_dat[id[i]].status.class_ == JOB_ALCHEMIST ||
			char_dat[id[i]].status.class_ == JOB_CREATOR ||
			char_dat[id[i]].status.class_ == JOB_BABY_ALCHEMIST))
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&chemist_fame_list[j],&fame_item,sizeof(struct fame_list));

			j++;
		}
	}
	// Build Taekwon ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_taekwon; i++) {
		if (char_dat[id[i]].status.fame &&
			char_dat[id[i]].status.class_ == JOB_TAEKWON)
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&taekwon_fame_list[j],&fame_item,sizeof(struct fame_list));

			j++;
		}
	}
	DELETE_BUFFER(id);
}
