// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "char.h"
#include "inter.h"
#include "int_storage.h"
#include "int_guild.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef TXT_ONLY
#define GS_BASIC 0x0001
#define GS_MEMBER 0x0002
#define GS_POSITION 0x0004
#define GS_ALLIANCE 0x0008
#define GS_EXPULSION 0x0010
#define GS_SKILL 0x0020
#define GS_EMBLEM 0x0040
#define GS_CONNECT 0x0080
#define GS_LEVEL 0x0100
#define GS_MES 0x0200
#define GS_MASK 0x03FF
#define GS_BASIC_MASK (GS_BASIC | GS_EMBLEM | GS_CONNECT | GS_LEVEL | GS_MES)
#define GS_REMOVE 0x8000

#define GS_MEMBER_UNMODIFIED 0x00
#define GS_MEMBER_MODIFIED 0x01
#define GS_MEMBER_NEW 0x02

#define GS_POSITION_UNMODIFIED 0x00
#define GS_POSITION_MODIFIED 0x01
#endif
// LSB = 0 => Alliance, LSB = 1 => Opposition
#define GUILD_ALLIANCE_TYPE_MASK 0x01
#define GUILD_ALLIANCE_REMOVE 0x08

static unsigned int guild_exp[100];
void mapif_guild_info(int fd, struct guild *g);


// Read exp_guild.txt
int inter_guild_exp_readdb(void)
{
	int i;
	FILE *fp;
	char line[1024];
	char path[1024];

	for( i = 0; i < 100; i++ )
		guild_exp[i] = 0;

	sprintf(path, "%s/%s", db_path, "exp_guild.txt");

	fp = fopen(path, "r");
	if( fp == NULL )
	{
		ShowError("inter_guild_exp_readdb: can't read %s\n", path);
		return 1;
	}

	i = 0;
	while( fgets(line, sizeof(line), fp) && i < 100 )
	{
		if( line[0]=='/' && line[1]=='/' )
			continue;
		guild_exp[i] = (unsigned int)atof(line);
		i++;
	}
	fclose(fp);

	return 0;
}


static bool guild_check_empty(struct guild *g)
{
	int i;
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id > 0 );
	if( i < g->max_member )
		return false; // not empty

#ifdef TXT_ONLY
	guild_db->foreach(guild_db, guild_break_sub, g->guild_id);
	inter_guild_storage_delete(g->guild_id);
	mapif_guild_broken(g->guild_id, 0);
	idb_remove(guild_db, g->guild_id);
#else
	//Let the calling function handle the guild removal in case they need
	//to do something else with it before freeing the data. [Skotlex]
#endif

	return true;
}

unsigned int guild_nextexp(int level)
{
	if (level == 0)
		return 1;
	if (level > 0 && level < 100)
		return guild_exp[level-1];

	return 0;
}

int guild_checkskill(struct guild *g, int id)
{
	int idx = id - GD_SKILLBASE;

	if(idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	return g->skill[idx].lv;
}

int guild_calcinfo(struct guild* g)
{
	int i, c;
	unsigned int nextexp;
	struct guild before = *g; // Save guild current values

	// set up skill ids
	for( i = 0; i < MAX_GUILDSKILL; i++ )
		g->skill[i].id = i + GD_SKILLBASE;

	// guild level
	if (g->guild_lv <= 0)
		g->guild_lv = 1;
	nextexp = guild_nextexp(g->guild_lv);

	// Consume guild exp and increase guild level
	while(g->exp >= nextexp && nextexp > 0)
	{	//fixed guild exp overflow [Kevin]
		g->exp -= nextexp;
		g->guild_lv++;
		g->skill_point++;
		nextexp = guild_nextexp(g->guild_lv);
	}

	// Save next exp step
	g->next_exp = guild_nextexp(g->guild_lv);

	// Set the max number of members
	g->max_member = 16 + guild_checkskill(g, GD_EXTENSION) * 6; //Guild Extention skill - +6 per skill lv.
	if(g->max_member > MAX_GUILD)
	{	
		ShowError("Guild %d:%s has capacity for too many guild members (%d), max supported is %d\n", g->guild_id, g->name, g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}

	// Compute the guild member level average
	g->average_lv = 0;
	g->connect_member = 0;
	c = 0;
	for( i = 0; i < g->max_member; i++ )
	{
		if( g->member[i].account_id > 0 )
		{
			g->average_lv += g->member[i].lv;
			c++;

			if( g->member[i].online )
				g->connect_member++;
		}
	}
	if(c)
		g->average_lv /= c;

	// Check if guild data changed
	if( g->max_member != before.max_member || g->guild_lv != before.guild_lv || g->skill_point != before.skill_point )
	{
#ifndef TXT_ONLY
		g->save_flag |= GS_LEVEL;
#endif
		mapif_guild_info(-1, g);
		return 1;
	}

	return 0;
}


#ifndef TXT_ONLY
static int guild_save_timer(int tid, unsigned int tick, int id, intptr data)
{
/*
	static int last_id = 0; //To know in which guild we were.
	int state = 0; //0: Have not reached last guild. 1: Reached last guild, ready for save. 2: Some guild saved, don't do further saving.
	DBIterator* iter;
	DBKey key;
	struct guild* g;

	if( last_id == 0 ) //Save the first guild in the list.
		state = 1;

	iter = guild_db_->iterator(guild_db_);
	for( g = (struct guild*)iter->first(iter,&key); iter->exists(iter); g = (struct guild*)iter->next(iter,&key) )
	{
		if( state == 0 && g->guild_id == last_id )
			state++; //Save next guild in the list.
		else
		if( state == 1 && g->save_flag&GS_MASK )
		{
			inter_guild_tosql(g, g->save_flag&GS_MASK);
			g->save_flag &= ~GS_MASK;

			//Some guild saved.
			last_id = g->guild_id;
			state++;
		}

		if( g->save_flag == GS_REMOVE )
		{// Nothing to save, guild is ready for removal.
			if (save_log)
				ShowInfo("Guild Unloaded (%d - %s)\n", g->guild_id, g->name);
			db_remove(guild_db_, key);
		}
	}
	iter->destroy(iter);

	if( state != 2 ) //Reached the end of the guild db without saving.
		last_id = 0; //Reset guild saved, return to beginning.

	state = guild_db_->size(guild_db_);
	if( state < 1 ) state = 1; //Calculate the time slot for the next save.
	add_timer(tick  + autosave_interval/state, guild_save_timer, 0, 0);
*/
	return 0;
}

int inter_guild_removemember_tosql(int account_id, int char_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE from `%s` where `account_id` = '%d' and `char_id` = '%d'", guild_member_db, account_id, char_id) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `guild_id` = '0' WHERE `char_id` = '%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);
	return 0;
}

int inter_guild_CharOnline(int char_id, int guild_id)
{
   struct guild *g;
   int i;
   
	if (guild_id == -1) {
		//Get guild_id from the database
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT guild_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}

		if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
		{
			char* data;

			Sql_GetData(sql_handle, 0, &data, NULL);
			guild_id = atoi(data);
		}
		else
		{
			guild_id = 0;
		}
		Sql_FreeResult(sql_handle);
	}
	if (guild_id == 0)
		return 0; //No guild...
	
	g = inter_guild_fromsql(guild_id);
	if(!g) {
		ShowError("Character %d's guild %d not found!\n", char_id, guild_id);
		return 0;
	}

	//Member has logged in before saving, tell saver not to delete
	if(g->save_flag & GS_REMOVE)
		g->save_flag &= ~GS_REMOVE;

	//Set member online
	ARR_FIND( 0, g->max_member, i, g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = 1;
		g->member[i].modified = GS_MEMBER_MODIFIED;
	}

	return 1;
}

int inter_guild_CharOffline(int char_id, int guild_id)
{
	struct guild *g=NULL;
	int online_count, i;

	if (guild_id == -1)
	{
		//Get guild_id from the database
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT guild_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}

		if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
		{
			char* data;

			Sql_GetData(sql_handle, 0, &data, NULL);
			guild_id = atoi(data);
		}
		else
		{
			guild_id = 0;
		}
		Sql_FreeResult(sql_handle);
	}
	if (guild_id == 0)
		return 0; //No guild...
	
	//Character has a guild, set character offline and check if they were the only member online
	g = inter_guild_fromsql(guild_id);
	if (g == NULL) //Guild not found?
		return 0;

	//Set member offline
	ARR_FIND( 0, g->max_member, i, g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = 0;
		g->member[i].modified = GS_MEMBER_MODIFIED;
	}

	online_count = 0;
	for( i = 0; i < g->max_member; i++ )
		if( g->member[i].online )
			online_count++;

	// Remove guild from memory if no players online
	if( online_count == 0 )
		g->save_flag |= GS_REMOVE;

	return 1;
}
#endif


//-------------------------------------------------------------------
// Packets sent to map server


void mapif_guild_created(int fd, int account_id, struct guild *g)
{
	WFIFOHEAD(fd, 10);
	WFIFOW(fd,0) = 0x3830;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = ( g != NULL ) ? g->guild_id : 0;
	WFIFOSET(fd,10);
}

void mapif_guild_noinfo(int fd, int guild_id)
{
	unsigned char buf[8];
	WBUFW(buf,0) = 0x3831;
	WBUFW(buf,2) = 8;
	WBUFL(buf,4) = guild_id;
	if( fd < 0 )
		mapif_sendall(buf, 8);
	else
		mapif_send(fd, buf, 8);
}

void mapif_guild_info(int fd, struct guild *g)
{
	unsigned char buf[8+sizeof(struct guild)];

	WBUFW(buf,0) = 0x3831;
	memcpy(buf + 4, g, sizeof(struct guild));
	WBUFW(buf,2) = 4 + sizeof(struct guild);
	if( fd < 0 )
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
}

void mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag)
{
	WFIFOHEAD(fd,15);
	WFIFOW(fd,0) = 0x3832;
	WFIFOL(fd,2) = guild_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd,15);
}

void mapif_guild_leaved(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes)
{
	unsigned char buf[79];
	WBUFW(buf, 0) = 0x3834;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = account_id;
	WBUFL(buf,10) = char_id;
	WBUFB(buf,14) = flag;
	memcpy(WBUFP(buf,15), mes, 40);
	memcpy(WBUFP(buf,55), name, NAME_LENGTH);
	mapif_sendall(buf, 79);
}

void mapif_guild_memberinfoshort(struct guild *g, int idx)
{
	unsigned char buf[19];
	WBUFW(buf, 0) = 0x3835;
	WBUFL(buf, 2) = g->guild_id;
	WBUFL(buf, 6) = g->member[idx].account_id;
	WBUFL(buf,10) = g->member[idx].char_id;
	WBUFB(buf,14) = (unsigned char)g->member[idx].online;
	WBUFW(buf,15) = g->member[idx].lv;
	WBUFW(buf,17) = g->member[idx].class_;
	mapif_sendall(buf, 19);
}

void mapif_guild_broken(int guild_id, int flag)
{
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3836;
	WBUFL(buf,2) = guild_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf,7);
}

void mapif_guild_message(int guild_id, int account_id, char *mes, int len, int sfd)
{
	unsigned char buf[2048];
	WBUFW(buf,0) = 0x3837;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = guild_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf, len + 12);
}

void mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len)
{
	unsigned char buf[2048];
	WBUFW(buf,0) = 0x3839;
	WBUFW(buf,2) = len+10;
	WBUFL(buf,4) = guild_id;
	WBUFW(buf,8) = type;
	memcpy(WBUFP(buf,10), data, len);
	mapif_sendall(buf,len+10);
}

void mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len)
{
	unsigned char buf[4096];
	WBUFW(buf, 0) = 0x383a;
	WBUFW(buf, 2) = len + 18;
	WBUFL(buf, 4) = guild_id;
	WBUFL(buf, 8) = account_id;
	WBUFL(buf,12) = char_id;
	WBUFW(buf,16) = type;
	memcpy(WBUFP(buf,18), data, len);
	mapif_sendall(buf,len+18);
}

void mapif_guild_skillupack(int guild_id, int skill_num, int account_id)
{
	unsigned char buf[14];
	WBUFW(buf, 0) = 0x383c;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = skill_num;
	WBUFL(buf,10) = account_id;
	mapif_sendall(buf,14);
}

void mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2)
{
	unsigned char buf[67];
	WBUFW(buf, 0) = 0x383d;
	WBUFL(buf, 2) = guild_id1;
	WBUFL(buf, 6) = guild_id2;
	WBUFL(buf,10) = account_id1;
	WBUFL(buf,14) = account_id2;
	WBUFB(buf,18) = flag;
	memcpy(WBUFP(buf,19), name1, NAME_LENGTH);
	memcpy(WBUFP(buf,43), name2, NAME_LENGTH);
	mapif_sendall(buf,67);
}

void mapif_guild_position(struct guild* g, int idx)
{
	unsigned char buf[12 + sizeof(struct guild_position)];
	WBUFW(buf,0) = 0x383b;
	WBUFW(buf,2) = sizeof(struct guild_position) + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = idx;
	memcpy(WBUFP(buf,12), &g->position[idx], sizeof(struct guild_position));
	mapif_sendall(buf,WBUFW(buf,2));
}

void mapif_guild_notice(struct guild* g)
{
	unsigned char buf[186];
	WBUFW(buf,0) = 0x383e;
	WBUFL(buf,2) = g->guild_id;
	memcpy(WBUFP(buf,6), g->mes1, 60);
	memcpy(WBUFP(buf,66), g->mes2, 120);
	mapif_sendall(buf,186);
}

void mapif_guild_emblem(struct guild* g)
{
	unsigned char buf[12 + sizeof(g->emblem_data)];
	WBUFW(buf,0) = 0x383f;
	WBUFW(buf,2) = g->emblem_len + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = g->emblem_id;
	memcpy(WBUFP(buf,12), g->emblem_data, g->emblem_len);
	mapif_sendall(buf, WBUFW(buf,2));
}

void mapif_guild_master_changed(struct guild* g, int aid, int cid)
{
	unsigned char buf[14];
	WBUFW(buf,0) = 0x3843;
	WBUFL(buf,2) = g->guild_id;
	WBUFL(buf,6) = aid;
	WBUFL(buf,10) = cid;
	mapif_sendall(buf,14);
}

void mapif_guild_castle_dataload(int castle_id, int index, int value)
{
	unsigned char buf[9];
	WBUFW(buf,0) = 0x3840;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);
}

void mapif_guild_castle_datasave(int castle_id, int index, int value)
{
	unsigned char buf[9];
	WBUFW(buf,0) = 0x3841;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);
}

/// sends all castle data information
void mapif_guild_castle_alldataload(int fd)
{
#ifdef TXT_ONLY
	DBIterator* iter;
	struct guild_castle* gc;
#else
	struct guild_castle s_gc;
	struct guild_castle* gc = &s_gc;
	int i;
#endif
	int len = 4;

	WFIFOHEAD(fd, 4 + MAX_GUILDCASTLE*sizeof(struct guild_castle));
	WFIFOW(fd,0) = 0x3842;

#ifdef TXT_ONLY
	iter = castle_db->iterator(castle_db);
	for( gc = (struct guild_castle*)iter->first(iter,NULL); iter->exists(iter); gc = (struct guild_castle*)iter->next(iter,NULL) )
	{
		memcpy(WFIFOP(fd,len), gc, sizeof(struct guild_castle));
		len += sizeof(struct guild_castle);
	}
	iter->destroy(iter);
#else
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`,"
		" `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`"
		" FROM `%s` ORDER BY `castle_id`", guild_castle_db) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < MAX_GUILDCASTLE && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		char* data;
		memset(gc, 0, sizeof(struct guild_castle));

		Sql_GetData(sql_handle,  0, &data, NULL); gc->castle_id = atoi(data);
		Sql_GetData(sql_handle,  1, &data, NULL); gc->guild_id = atoi(data);
		Sql_GetData(sql_handle,  2, &data, NULL); gc->economy = atoi(data);
		Sql_GetData(sql_handle,  3, &data, NULL); gc->defense = atoi(data);
		Sql_GetData(sql_handle,  4, &data, NULL); gc->triggerE = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); gc->triggerD = atoi(data);
		Sql_GetData(sql_handle,  6, &data, NULL); gc->nextTime = atoi(data);
		Sql_GetData(sql_handle,  7, &data, NULL); gc->payTime = atoi(data);
		Sql_GetData(sql_handle,  8, &data, NULL); gc->createTime = atoi(data);
		Sql_GetData(sql_handle,  9, &data, NULL); gc->visibleC = atoi(data);
		Sql_GetData(sql_handle, 10, &data, NULL); gc->guardian[0].visible = atoi(data);
		Sql_GetData(sql_handle, 11, &data, NULL); gc->guardian[1].visible = atoi(data);
		Sql_GetData(sql_handle, 12, &data, NULL); gc->guardian[2].visible = atoi(data);
		Sql_GetData(sql_handle, 13, &data, NULL); gc->guardian[3].visible = atoi(data);
		Sql_GetData(sql_handle, 14, &data, NULL); gc->guardian[4].visible = atoi(data);
		Sql_GetData(sql_handle, 15, &data, NULL); gc->guardian[5].visible = atoi(data);
		Sql_GetData(sql_handle, 16, &data, NULL); gc->guardian[6].visible = atoi(data);
		Sql_GetData(sql_handle, 17, &data, NULL); gc->guardian[7].visible = atoi(data);

		memcpy(WFIFOP(fd, len), gc, sizeof(struct guild_castle));
		len += sizeof(struct guild_castle);
	}
	Sql_FreeResult(sql_handle);
#endif

	WFIFOW(fd,2) = len;
	WFIFOSET(fd, len);
}


//-------------------------------------------------------------------
// Packets received from map server


// ギルド作成要求
int mapif_parse_CreateGuild(int fd, int account_id, char *name, struct guild_member *master)
{
	struct guild *g;
	int i;

	if ( search_guildname(name) != NULL )
	{
		mapif_guild_created(fd, account_id, NULL);
		return 0;
	}

	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) == NULL) {
				mapif_guild_created(fd,account_id,NULL);
				return 0;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) != NULL) {
				mapif_guild_created(fd,account_id,NULL);
				return 0;
			}
	}

	g = (struct guild *)aCalloc(sizeof(struct guild), 1);

	memcpy(g->name, name, NAME_LENGTH);
	memcpy(g->master, master->name, NAME_LENGTH);
	memcpy(&g->member[0], master, sizeof(struct guild_member));
#ifndef TXT_ONLY
	g->member[0].modified = GS_MEMBER_MODIFIED;
#endif

	// Set default positions
	g->position[0].mode = 0x11;
	strcpy(g->position[0].name, "GuildMaster");
	strcpy(g->position[MAX_GUILDPOSITION-1].name, "Newbie");
#ifndef TXT_ONLY
	g->position[0].modified = g->position[MAX_GUILDPOSITION-1].modified = GS_POSITION_MODIFIED;
#endif
	for( i = 1; i < MAX_GUILDPOSITION-1; i++ )
	{
		sprintf(g->position[i].name, "Position %d", i + 1);
#ifndef TXT_ONLY
		g->position[i].modified = GS_POSITION_MODIFIED;
#endif
	}

	// Initialize guild property
	g->max_member = 16;
	g->average_lv = master->lv;
	g->connect_member = 1;

	for( i = 0; i < MAX_GUILDSKILL; i++ )
		g->skill[i].id = i + GD_SKILLBASE;

#ifdef TXT_ONLY
	g->guild_id = guild_newid++;
#else
	g->guild_id = -1; //Request to create guild.
#endif

#ifndef TXT_ONLY
	// Create the guild
	if (!inter_guild_tosql(g,GS_BASIC|GS_POSITION|GS_SKILL)) {
		//Failed to Create guild....
		ShowError("Failed to create Guild %s (Guild Master: %s)\n", g->name, g->master);
		mapif_guild_created(fd,account_id,NULL);
		aFree(g);
		return 0;
	}
	ShowInfo("Created Guild %d - %s (Guild Master: %s)\n", g->guild_id, g->name, g->master);
#endif

#ifdef TXT_ONLY
	idb_put(guild_db, g->guild_id, g);
#else
	idb_put(guild_db_, g->guild_id, g);
#endif

	// Report to client
	mapif_guild_created(fd, account_id, g);
	mapif_guild_info(fd, g);

	if(log_inter)
		inter_log("guild %s (id=%d) created by master %s (id=%d)\n",
			name, g->guild_id, master->name, master->account_id);

	return 0;
}

// ギルド情報要求
int mapif_parse_GuildInfo(int fd, int guild_id)
{
	struct guild* g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id); //We use this because on start-up the info of castle-owned guilds is requied. [Skotlex]
#endif
	if( g != NULL )
	{
#ifdef TXT_ONLY
		guild_calcinfo(g);
		mapif_guild_info(fd, g);
#else
		if (!guild_calcinfo(g))
			mapif_guild_info(fd,g);
#endif
	}
	else
		mapif_guild_noinfo(fd, guild_id);

	return 0;
}

// ギルドメンバ追加要求
int mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member *m)
{
	struct guild* g;
	int i;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
	{
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);
		return 0;
	}

	// Find an empty slot
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == 0 );
	if( i == g->max_member )
	{// Failed to add
		mapif_guild_memberadded(fd,guild_id,m->account_id,m->char_id,1);
		return 0;
	}

	memcpy(&g->member[i], m, sizeof(struct guild_member));
#ifndef TXT_ONLY
	g->member[i].modified = (GS_MEMBER_NEW | GS_MEMBER_MODIFIED);
#endif
	mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 0);
	
#ifdef TXT_ONLY
	if (!guild_calcinfo(g)) //Send members if it was not invoked.
		mapif_guild_info(-1,g);
#else
	guild_calcinfo(g);
	mapif_guild_info(-1, g);
#endif

#ifndef TXT_ONLY
	g->save_flag |= GS_MEMBER;
	if (g->save_flag&GS_REMOVE)
		g->save_flag&=~GS_REMOVE;
#endif

	return 0;
}

// Delete member from guild
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes)
{
	int i, j;

#ifdef TXT_ONLY
	struct guild* g = (struct guild*)idb_get(guild_db, guild_id);
#else
	struct guild* g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
	{
#ifdef TXT_ONLY
		//TODO
#else
		// Unknown guild, just update the player
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `guild_id`='0' WHERE `account_id`='%d' AND `char_id`='%d'", char_db, account_id, char_id) )
			Sql_ShowDebug(sql_handle);
		// mapif_guild_leaved(guild_id,account_id,char_id,flag,g->member[i].name,mes);
#endif
		return 0;
	}

	// Find the member
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i == g->max_member )
	{
		//TODO
		return 0;
	}

	if( flag )
	{// Write expulsion reason
		// find an empty slot
		ARR_FIND( 0, MAX_GUILDEXPULSION, j, g->expulsion[j].account_id == 0 );
		if( j == MAX_GUILDEXPULSION )
		{// expulsion list is full, flush the oldest entry
			for( j = 0; j < MAX_GUILDEXPULSION - 1; j++ )
				g->expulsion[j] = g->expulsion[j+1];
			j = MAX_GUILDEXPULSION-1;
		}
		// Save the expulsion entry
		g->expulsion[j].account_id = account_id;
		safestrncpy(g->expulsion[j].name, g->member[i].name, NAME_LENGTH);
		safestrncpy(g->expulsion[j].mes, mes, 40);
	}

	mapif_guild_leaved(guild_id, account_id, char_id, flag, g->member[i].name, mes);

#ifndef TXT_ONLY
	inter_guild_removemember_tosql(g->member[i].account_id,g->member[i].char_id);
#endif

	memset(&g->member[i], 0, sizeof(struct guild_member));

#ifdef TXT_ONLY
	if( guild_check_empty(g) == 0 )
		mapif_guild_info(-1,g);// まだ人がいるのでデータ送信
#else
	if( guild_check_empty(g) )
		mapif_parse_BreakGuild(-1,guild_id); //Break the guild.
	else
	{//Update member info.
		if (!guild_calcinfo(g))
			mapif_guild_info(fd,g);
		g->save_flag |= GS_EXPULSION;
	}
#endif

	return 0;
}

// オンライン/Lv更新
int mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_)
{
	struct guild* g;
	int i, sum, c;
#ifndef TXT_ONLY
	int prev_count, prev_alv;
#endif

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return 0;
	
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
		g->member[i].online = online;
		g->member[i].lv = lv;
		g->member[i].class_ = class_;
#ifndef TXT_ONLY
		g->member[i].modified = GS_MEMBER_MODIFIED;
#endif
		mapif_guild_memberinfoshort(g,i);
	}

#ifndef TXT_ONLY
	prev_count = g->connect_member;
	prev_alv = g->average_lv;
#endif

	g->average_lv = 0;
	g->connect_member = 0;
	c = 0; // member count
	sum = 0; // total sum of base levels

	for( i = 0; i < g->max_member; i++ )
	{
		if( g->member[i].account_id > 0 )
		{
			sum += g->member[i].lv;
			c++;
		}
		if( g->member[i].online )
			g->connect_member++;
	}

	if( c ) // this check should always succeed...
	{
		g->average_lv = sum / c;
#ifndef TXT_ONLY
		if( g->connect_member != prev_count || g->average_lv != prev_alv )
			g->save_flag |= GS_CONNECT;
		if( g->save_flag & GS_REMOVE )
			g->save_flag &= ~GS_REMOVE;
#endif
	}

#ifndef TXT_ONLY
	g->save_flag |= GS_MEMBER; //Update guild member data
#endif

	//FIXME: how about sending a mapif_guild_info() update to the mapserver? [ultramage] 

	return 0;
}

// ギルド解散要求
int mapif_parse_BreakGuild(int fd,int guild_id)
{
	struct guild* g;
#ifdef TXT_ONLY
	struct DBIterator* iter;
	struct guild* tmp;
#endif

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return 0;

#ifdef TXT_ONLY
	iter = guild_db->iterator(iter);
	while( (tmp = iter->next(iter,NULL)) != NULL )
	{// ギルド解散処理用（同盟/敵対を解除）
		for( i = 0; i < MAX_GUILDALLIANCE; i++ )
			if( tmp->alliance[i].guild_id == guild_id )
				tmp->alliance[i].guild_id = 0;
	}
	iter->destroy(iter);

	inter_guild_storage_delete(guild_id);
#else
	// Delete guild from sql
	//printf("- Delete guild %d from guild\n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_member_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_castle_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_storage_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d' OR `alliance_id` = '%d'", guild_alliance_db, guild_id, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_position_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_skill_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_expulsion_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	//printf("- Update guild %d of char\n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `guild_id`='0' WHERE `guild_id`='%d'", char_db, guild_id) )
		Sql_ShowDebug(sql_handle);
#endif

	mapif_guild_broken(guild_id, 0);

	if( log_inter )
		inter_log("guild %s (id=%d) broken\n", g->name, guild_id);

#ifdef TXT_ONLY
	idb_remove(guild_db, guild_id);
#else
	//Remove the guild from memory. [Skotlex]
	idb_remove(guild_db_, guild_id);
#endif

	return 0;
}

// ギルドメッセージ送信
void mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len)
{
	mapif_guild_message(guild_id, account_id, mes, len, fd);
}

// ギルド基本データ変更要求
int mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const char *data, int len)
{
	struct guild *g;
	short dw = *((short *)data);

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return 0;

	switch(type)
	{
	case GBI_GUILDLV:
		if( dw > 0 && g->guild_lv+dw <= 50 )
		{
			g->guild_lv += dw;
			g->skill_point += dw;
		}
		else
		if( dw < 0 && g->guild_lv + dw >= 1 )
			g->guild_lv += dw;

		mapif_guild_info(-1, g);
#ifndef TXT_ONLY
		g->save_flag |= GS_LEVEL;
#endif
		return 0;
	default:
		ShowError("int_guild: GuildBasicInfoChange: Unknown type %d\n", type);
		break;
	}

	mapif_guild_basicinfochanged(guild_id, type, data, len);

	return 0;
}

// ギルドメンバデータ変更要求
int mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len)
{
	int i;
	struct guild* g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return 0;

	// find the member
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i == g->max_member )
	{
		ShowWarning("int_guild: GuildMemberChange: Not found %d,%d in guild (%d - %s)\n", account_id, char_id, guild_id, g->name);
		return 0;
	}

	switch(type)
	{
	case GMI_POSITION:
		g->member[i].position = *((int *)data);
		break;
	case GMI_EXP:
	{
		unsigned int exp, old_exp=g->member[i].exp;
		g->member[i].exp=*((unsigned int *)data);
		if (g->member[i].exp > old_exp)
		{
			exp = g->member[i].exp - old_exp;

			// Compute gained exp
			if (guild_exp_rate != 100)
				exp = exp*guild_exp_rate/100;

			// Update guild exp
			if (exp > UINT_MAX - g->exp)
				g->exp = UINT_MAX;
			else
				g->exp+=exp;

			guild_calcinfo(g);
			mapif_guild_basicinfochanged(guild_id,GBI_EXP,&g->exp,4);
#ifndef TXT_ONLY
			g->save_flag |= GS_LEVEL;
#endif
		}
		break;
	}
	case GMI_HAIR:
		g->member[i].hair=*((int *)data);
		break;
	case GMI_HAIR_COLOR:
		g->member[i].hair_color=*((int *)data);
		break;
	case GMI_GENDER:
		g->member[i].gender=*((int *)data);
		break;
	case GMI_CLASS:
		g->member[i].class_=*((int *)data);
		break;
	case GMI_LEVEL:
		g->member[i].lv=*((int *)data);
		break;

	default:
		ShowError("int_guild: GuildMemberInfoChange: Unknown type %d\n",type);
		return 0;
	}

	mapif_guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);

#ifndef TXT_ONLY
	g->member[i].modified = GS_MEMBER_MODIFIED;
	g->save_flag |= GS_MEMBER; //Save new data.
#endif

	return 0;
}

int inter_guild_sex_changed(int guild_id,int account_id,int char_id, int gender)
{
	return mapif_parse_GuildMemberInfoChange(0, guild_id, account_id, char_id, GMI_GENDER, (const char*)&gender, sizeof(gender));
}

// ギルド役職名変更要求
int mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position *p)
{
	struct guild* g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL || idx < 0 || idx >= MAX_GUILDPOSITION )
		return 0;

	memcpy(&g->position[idx], p, sizeof(struct guild_position));
	mapif_guild_position(g, idx);

#ifndef TXT_ONLY
	g->position[idx].modified = GS_POSITION_MODIFIED;
	g->save_flag |= GS_POSITION; // Change guild_position
#endif

	return 0;
}

// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd, int guild_id, int skill_num, int account_id)
{
	struct guild* g;
	int idx = skill_num - GD_SKILLBASE;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL || idx < 0 || idx >= MAX_GUILDSKILL )
		return 0;

	if( g->skill_point > 0 && g->skill[idx].id > 0 && g->skill[idx].lv < 10 )
	{
		g->skill[idx].lv++;
		g->skill_point--;
		if( guild_calcinfo(g) == 0 )
			mapif_guild_info(-1, g);
		mapif_guild_skillupack(guild_id, skill_num, account_id);

#ifndef TXT_ONLY
		g->save_flag |= (GS_LEVEL|GS_SKILL); // Change guild & guild_skill
#endif
	}

	return 0;
}

//Manual deletion of an alliance when partnering guild does not exists. [Skotlex]
static int mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag)
{
	int i;
	char name[NAME_LENGTH];

	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->alliance[i].guild_id == guild_id );
	if( i == MAX_GUILDALLIANCE )
		return -1;

	strcpy(name, g->alliance[i].name);
	g->alliance[i].guild_id = 0;

	mapif_guild_alliance(g->guild_id,guild_id,account_id1,account_id2,flag,g->name,name);

#ifndef TXT_ONLY
	g->save_flag |= GS_ALLIANCE;
#endif

	return 0;
}

// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag)
{
	struct guild *g[2];
	int j, i;

#ifdef TXT_ONLY
	g[0] = (struct guild*)idb_get(guild_db, guild_id1);
	g[1] = (struct guild*)idb_get(guild_db, guild_id2);
#else
	g[0] = inter_guild_fromsql(guild_id1);
	g[1] = inter_guild_fromsql(guild_id2);
#endif

	if( g[0] != NULL && g[1] == NULL && (flag & GUILD_ALLIANCE_REMOVE) ) //Requested to remove an alliance with a not found guild.
		return mapif_parse_GuildDeleteAlliance(g[0], guild_id2, account_id1, account_id2, flag); //Try to do a manual removal of said guild.

	if( g[0] == NULL || g[1] == NULL )
		return 0;

	if( flag&GUILD_ALLIANCE_REMOVE )
	{// Remove alliance/opposition, in case of alliance, remove on both side
		for( i = 0; i < 2 - (flag & GUILD_ALLIANCE_TYPE_MASK); i++ )
		{
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == g[1-i]->guild_id && g[i]->alliance[j].opposition == (flag&GUILD_ALLIANCE_TYPE_MASK) );
			if( j < MAX_GUILDALLIANCE )
				g[i]->alliance[j].guild_id = 0;
		}
	}
	else
	{// Add alliance, in case of alliance, add on both side
		for( i = 0; i < 2 - (flag & GUILD_ALLIANCE_TYPE_MASK); i++ )
		{
			// Search an empty slot
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == 0 );
			if( j < MAX_GUILDALLIANCE )
			{
				g[i]->alliance[j].guild_id = g[1-i]->guild_id;
				memcpy(g[i]->alliance[j].name, g[1-i]->name, NAME_LENGTH);
				g[i]->alliance[j].opposition = flag & GUILD_ALLIANCE_TYPE_MASK;
			}
		}
	}

	// Send on all map the new alliance/opposition
	mapif_guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, g[0]->name, g[1]->name);

#ifndef TXT_ONLY
	// Mark the two guild to be saved
	g[0]->save_flag |= GS_ALLIANCE;
	g[1]->save_flag |= GS_ALLIANCE;
#endif

	return 0;
}

// ギルド告知変更要求
void mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	struct guild *g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return;

	memcpy(g->mes1, mes1, 60);
	memcpy(g->mes2, mes2, 120);

#ifndef TXT_ONLY
	g->save_flag |= GS_MES;	//Change mes of guild
#endif

	mapif_guild_notice(g);
}

// ギルドエンブレム変更要求
void mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	struct guild* g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
	if( g == NULL )
		return;
#else
	g = inter_guild_fromsql(guild_id);
	if( g == NULL )
		return;
#endif

	if (len > sizeof(g->emblem_data))
		len = sizeof(g->emblem_data);

	memcpy(g->emblem_data, data, len);
	g->emblem_len = len;
	g->emblem_id++;

#ifndef TXT_ONLY
	g->save_flag |= GS_EMBLEM;	//Change guild
#endif

	mapif_guild_emblem(g);
}

void mapif_parse_GuildCastleDataLoad(int fd, int castle_id, int index)
{
#ifdef TXT_ONLY
	struct guild_castle *gc;
	
	gc = (struct guild_castle*)idb_get(castle_db, castle_id);
	if (gc == NULL)
	{
		mapif_guild_castle_dataload(castle_id, 0, 0);
		return;
	}
#else
	struct guild_castle tmp, *gc;
	gc = &tmp;

	if (!inter_guildcastle_fromsql(castle_id, &gc))
	{
		mapif_guild_castle_dataload(castle_id, 0, 0);
		return;
	}
#endif

	switch(index)
	{
	case 1: mapif_guild_castle_dataload(gc->castle_id, index, gc->guild_id); break;
	case 2: mapif_guild_castle_dataload(gc->castle_id, index, gc->economy); break;
	case 3: mapif_guild_castle_dataload(gc->castle_id, index, gc->defense); break;
	case 4: mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerE); break;
	case 5: mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerD); break;
	case 6: mapif_guild_castle_dataload(gc->castle_id, index, gc->nextTime); break;
	case 7: mapif_guild_castle_dataload(gc->castle_id, index, gc->payTime); break;
	case 8: mapif_guild_castle_dataload(gc->castle_id, index, gc->createTime); break;
	case 9: mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleC); break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		mapif_guild_castle_dataload(gc->castle_id, index, gc->guardian[index-10].visible); break;
	default:
		ShowError("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
	}
}

void mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
#ifdef TXT_ONLY
	struct guild_castle *gc;

	gc = (struct guild_castle*)idb_get(castle_db, castle_id);
	if (gc == NULL)
	{
		mapif_guild_castle_datasave(castle_id, index, value);
		return;
	}
#else
	struct guild_castle tmp, *gc;
	gc = &tmp;

	if(!inter_guildcastle_fromsql(castle_id, gc))
	{
		mapif_guild_castle_datasave(castle_id, index, value);
		return;
	}
#endif

	switch(index) {
	case 1:
		if( gc->guild_id != value )
		{
			int gid = (value) ? value : gc->guild_id;
#ifdef TXT_ONLY
			struct guild *g = (struct guild*)idb_get(guild_db, gid);
#else
			struct guild *g = (struct guild*)idb_get(guild_db_, gid);
#endif
			if(log_inter)
				inter_log("guild %s (id=%d) %s castle id=%d\n",
					(g) ? g->name : "??", gid, (value) ? "occupy" : "abandon", castle_id);
		}
		gc->guild_id = value;
		if(gc->guild_id == 0) {
			//Delete guardians.
			memset(&gc->guardian, 0, sizeof(gc->guardian));
		}
		break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		gc->guardian[index-10].visible = value; break;
	default:
		ShowError("mapif_parse_GuildCastleDataSave ERROR!! (Not found index=%d)\n", index);
		return;
	}

#ifndef TXT_ONLY
	inter_guildcastle_tosql(&gc);
#endif

	mapif_guild_castle_datasave(gc->castle_id, index, value);
}

void mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len)
{
	struct guild *g;
	struct guild_member gm;
	int pos;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);	
#endif
	if( g == NULL || len > NAME_LENGTH )
		return;
	
	// Find member (name)
	ARR_FIND( 0, g->max_member, pos, strncmp(g->member[pos].name, name, len) == 0 );
	if (pos == g->max_member)
		return; //Character not found??
	
	// Switch current and old GMs
	memcpy(&gm, &g->member[pos], sizeof (struct guild_member));
	memcpy(&g->member[pos], &g->member[0], sizeof(struct guild_member));
	memcpy(&g->member[0], &gm, sizeof(struct guild_member));

	// Switch positions
	g->member[pos].position = g->member[0].position;
	g->member[0].position = 0; //Position 0: guild Master.
#ifndef TXT_ONLY
	g->member[pos].modified = GS_MEMBER_MODIFIED;
	g->member[0].modified = GS_MEMBER_MODIFIED;
#endif

	safestrncpy(g->master, name, NAME_LENGTH);

#ifndef TXT_ONLY
	g->save_flag |= (GS_BASIC|GS_MEMBER); //Save main data and member data.
#endif

	ShowInfo("int_guild: Guildmaster Changed to %s (Guild %d - %s)\n", g->master, guild_id, g->name);
	mapif_guild_master_changed(g, g->member[0].account_id, g->member[0].char_id);
}


// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif_parse_CreateGuild(fd, RFIFOL(fd,4), (char*)RFIFOP(fd,8), (struct guild_member *)RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfo(fd, RFIFOL(fd,2)); break;
	case 0x3032: mapif_parse_GuildAddMember(fd, RFIFOL(fd,4), (struct guild_member *)RFIFOP(fd,8)); break;
	case 0x3033: mapif_parse_GuildMasterChange(fd,RFIFOL(fd,4),(const char*)RFIFOP(fd,8),RFIFOW(fd,2)-8); break;
	case 0x3034: mapif_parse_GuildLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), (const char*)RFIFOP(fd,15)); break;
	case 0x3035: mapif_parse_GuildChangeMemberInfoShort(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOW(fd,15), RFIFOW(fd,17)); break;
	case 0x3036: mapif_parse_BreakGuild(fd, RFIFOL(fd,2)); break;
	case 0x3037: mapif_parse_GuildMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3039: mapif_parse_GuildBasicInfoChange(fd, RFIFOL(fd,4), RFIFOW(fd,8), (const char*)RFIFOP(fd,10), RFIFOW(fd,2)-10); break;
	case 0x303A: mapif_parse_GuildMemberInfoChange(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOL(fd,12), RFIFOW(fd,16), (const char*)RFIFOP(fd,18), RFIFOW(fd,2)-18); break;
	case 0x303B: mapif_parse_GuildPosition(fd, RFIFOL(fd,4), RFIFOL(fd,8), (struct guild_position *)RFIFOP(fd,12)); break;
	case 0x303C: mapif_parse_GuildSkillUp(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x303D: mapif_parse_GuildAlliance(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOB(fd,18)); break;
	case 0x303E: mapif_parse_GuildNotice(fd, RFIFOL(fd,2), (const char*)RFIFOP(fd,6), (const char*)RFIFOP(fd,66)); break;
	case 0x303F: mapif_parse_GuildEmblem(fd, RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), (const char*)RFIFOP(fd,12)); break;
	case 0x3040: mapif_parse_GuildCastleDataLoad(fd, RFIFOW(fd,2), RFIFOB(fd,4)); break;
	case 0x3041: mapif_parse_GuildCastleDataSave(fd, RFIFOW(fd,2), RFIFOB(fd,4), RFIFOL(fd,5)); break;

	default:
		return 0;
	}

	return 1;
}


// processes a mapserver connection event
void inter_guild_mapif_init(int fd)
{
	mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(int guild_id, int account_id, int char_id)
{
	return mapif_parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "** Character Deleted **");
}

int inter_guild_init(void)
{
	//Read guild exp table
	inter_guild_exp_readdb();
   
	//TODO: init guilddb here
	//TODO: init castledb here

	return 0;
}

void inter_guild_final(void)
{
	//TODO: destroy guilddb here
	//TODO: destroy castledb here
	//TODO: use the correct order of the above calls
}
