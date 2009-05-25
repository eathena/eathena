// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "castledb.h"
#include "char.h"
#include "guilddb.h"
#include "inter.h"
#include "interlog.h"
#include "int_storage.h"
#include "int_guild.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// databases
static GuildDB* guilds = NULL;
static CastleDB* castles = NULL;

// LSB = 0 => Alliance, LSB = 1 => Opposition
#define GUILD_ALLIANCE_TYPE_MASK 0x01
#define GUILD_ALLIANCE_REMOVE 0x08

static unsigned int guild_exp[100];
void mapif_guild_info(int fd, struct guild *g);
void mapif_guild_broken(int guild_id, int flag);


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

static bool guild_break(int guild_id)
{
/*
#ifdef TXT_ONLY
	struct DBIterator* iter;
	struct guild* tmp;

	// end all alliances / oppositions
	iter = guilds->iterator(iter);
	while( (tmp = iter->next(iter,NULL)) != NULL )
	{// ギルド解散処理用（同盟/敵対を解除）
		for( i = 0; i < MAX_GUILDALLIANCE; i++ )
			if( tmp->alliance[i].guild_id == guild_id )
				tmp->alliance[i].guild_id = 0;
	}
	iter->destroy(iter);
#else
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d' OR `alliance_id` = '%d'", guild_alliance_db, guild_id, guild_id) )
		Sql_ShowDebug(sql_handle);
#endif
*/

	inter_guild_storage_delete(guild_id);
	guilds->remove(guilds, guild_id);

	mapif_guild_broken(guild_id, 0);
	interlog_log("guild (id=%d) broken\n", guild_id);

	return true;
}

static bool guild_check_empty(struct guild *g)
{
	int i;
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id > 0 );
	if( i < g->max_member )
		return false; // not empty

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

/// Recalculates some values in the provided guild structure.
/// Returns true if anything changes, otherwise returns false.
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

	return ( g->max_member != before.max_member || g->guild_lv != before.guild_lv || g->skill_point != before.skill_point );
}


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
	CastleDBIterator* iter;
	struct guild_castle gc;
	int len = 4;

	WFIFOHEAD(fd, 4 + MAX_GUILDCASTLE*sizeof(struct guild_castle));
	WFIFOW(fd,0) = 0x3842;

	iter = castles->iterator(castles);
	while( iter->next(iter, &gc) )
	{
		memcpy(WFIFOP(fd,len), &gc, sizeof(struct guild_castle));
		len += sizeof(struct guild_castle);
	}
	iter->destroy(iter);

	WFIFOW(fd,2) = len;
	WFIFOSET(fd, len);
}


//-------------------------------------------------------------------
// Packets received from map server


// ギルド作成要求
int mapif_parse_CreateGuild(int fd, int account_id, char *name, struct guild_member *master)
{
	struct guild g;
	int i;

	if ( guilds->name2id(guilds, name, NULL) )
	{// guild name already occupied
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

	memset(&g, 0, sizeof(g));

	memcpy(g.name, name, NAME_LENGTH);
	memcpy(g.master, master->name, NAME_LENGTH);
	memcpy(&g.member[0], master, sizeof(struct guild_member));
	g.member[0].modified = GS_MEMBER_MODIFIED;

	// Set default positions
	g.position[0].mode = 0x11; //FIXME: what is this?
	strcpy(g.position[0].name, "GuildMaster");
	strcpy(g.position[MAX_GUILDPOSITION-1].name, "Newbie");
	g.position[0].modified = GS_POSITION_MODIFIED;
	g.position[MAX_GUILDPOSITION-1].modified = GS_POSITION_MODIFIED;
	for( i = 1; i < MAX_GUILDPOSITION-1; i++ )
	{
		sprintf(g.position[i].name, "Position %d", i + 1);
		g.position[i].modified = GS_POSITION_MODIFIED;
	}

	// Initialize guild property
	g.max_member = 16;
	g.average_lv = master->lv;
	g.connect_member = 1;

	for( i = 0; i < MAX_GUILDSKILL; i++ )
		g.skill[i].id = i + GD_SKILLBASE;

	g.guild_id = -1; // Request to assign new guild id
	if( !guilds->create(guilds, &g) )
	{// Failed to Create guild....
		ShowError("Failed to create Guild %s (Guild Master: %s)\n", g.name, g.master);
		mapif_guild_created(fd, account_id, NULL);
	}

	// Report to client
	mapif_guild_created(fd, account_id, &g);
	mapif_guild_info(fd, &g);

	ShowInfo("Created Guild %d - %s (Guild Master: %s)\n", g.guild_id, g.name, g.master);
	interlog_log("guild %s (id=%d) created by master %s (id=%d)\n", name, g.guild_id, master->name, master->account_id);

	return 0;
}

// ギルド情報要求
void mapif_parse_GuildInfoRequest(int fd, int guild_id)
{
	struct guild g;

	if( !guilds->load(guilds, &g, guild_id) )
	{
		mapif_guild_noinfo(fd, guild_id);
		return;
	}

	if( guild_calcinfo(&g) )
	{
		guilds->save(guilds, &g, GS_LEVEL);
		mapif_guild_info(-1, &g);
	}
	else
		mapif_guild_info(fd, &g);

}

// ギルドメンバ追加要求
void mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member* m)
{
	struct guild g;
	int i;
	enum guild_save_flag save_flag = 0;

	if( !guilds->load(guilds, &g, guild_id) )
	{
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);
		return;
	}

	// Find an empty slot
	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == 0 );
	if( i == g.max_member )
	{// Failed to add
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);
		return;
	}

	memcpy(&g.member[i], m, sizeof(struct guild_member));
	g.member[i].modified = GS_MEMBER_NEW | GS_MEMBER_MODIFIED;
	save_flag |= GS_MEMBER;

	mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 0);

	if( guild_calcinfo(&g) )
		save_flag |= GS_LEVEL;

	mapif_guild_info(-1, &g);

	guilds->save(guilds, &g, save_flag);
}

// Delete member from guild
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes)
{
	struct guild g;
	int i, j;
	enum guild_save_flag save_flag = 0;

	if( !guilds->load(guilds, &g, guild_id) )
	{
		//TODO
		return 0;
	}

	// Find the member
	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == account_id && g.member[i].char_id == char_id );
	if( i == g.max_member )
	{
		//TODO
		return 0;
	}

	if( i == 0 )
	{// guild master
		guild_break(guild_id);
		return 0;
	}

	if( flag )
	{// Write expulsion reason
		// find an empty slot
		ARR_FIND( 0, MAX_GUILDEXPULSION, j, g.expulsion[j].account_id == 0 );
		if( j == MAX_GUILDEXPULSION )
		{// expulsion list is full, flush the oldest entry
			for( j = 0; j < MAX_GUILDEXPULSION - 1; j++ )
				g.expulsion[j] = g.expulsion[j+1];
			j = MAX_GUILDEXPULSION-1;
		}
		// Save the expulsion entry
		g.expulsion[j].account_id = account_id;
		safestrncpy(g.expulsion[j].name, g.member[i].name, NAME_LENGTH);
		safestrncpy(g.expulsion[j].mes, mes, 40);
		save_flag |= GS_EXPULSION;
	}

	mapif_guild_leaved(guild_id, account_id, char_id, flag, g.member[i].name, mes);

	g.member[i].modified = GS_MEMBER_DELETED;
	guilds->save(guilds, &g, GS_MEMBER); // first step with data intact, delete from storage (SQL)

	memset(&g.member[i], 0, sizeof(struct guild_member));

	if( guild_check_empty(&g) )
	{
		guild_break(guild_id);
		return 0;
	}

	//TODO: check if this thing is working correctly
	if( guild_calcinfo(&g) )
	{
		mapif_guild_info(-1, &g);// まだ人がいるのでデータ送信
		save_flag |= GS_LEVEL|GS_CONNECT;
	}

	guilds->save(guilds, &g, flag); // second step with data erased, delete from memory (TXT)

	return 0;
}

// オンライン/Lv更新
void mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_)
{
	struct guild g;
	int i, sum, c;
	int prev_count, prev_alv;
	enum guild_save_flags save_flag = 0;

	if( !guilds->load(guilds, &g, guild_id) )
		return;
	
	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == account_id && g.member[i].char_id == char_id );
	if( i == g.max_member )
		return; // player not in guild? (error)

	// update member data
	g.member[i].online = online;
	g.member[i].lv = lv;
	g.member[i].class_ = class_;
	g.member[i].modified = GS_MEMBER_MODIFIED;
	save_flag |= GS_MEMBER;
	mapif_guild_memberinfoshort(&g, i);

	// store previous values (for comparison purposes)
	prev_count = g.connect_member;
	prev_alv = g.average_lv;

	// recalculate guild information
	g.average_lv = 0;
	g.connect_member = 0;
	c = 0; // member count
	sum = 0; // total sum of base levels

	for( i = 0; i < g.max_member; i++ )
	{
		if( g.member[i].account_id > 0 )
		{
			sum += g.member[i].lv;
			c++;
		}
		if( g.member[i].online )
			g.connect_member++;
	}

	if( c > 0 ) // this check should always succeed...
		g.average_lv = sum / c;

	//FIXME: how about sending a mapif_guild_info() update to the mapserver? [ultramage] 

	if( g.connect_member != prev_count || g.average_lv != prev_alv )
		save_flag |= GS_CONNECT;

	guilds->save(guilds, &g, save_flag);
}

// ギルド解散要求
void mapif_parse_BreakGuild(int fd, int guild_id)
{
	guild_break(guild_id);
}

// ギルドメッセージ送信
void mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len)
{
	mapif_guild_message(guild_id, account_id, mes, len, fd);
}

// ギルド基本データ変更要求
void mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const char *data, int len)
{
	struct guild g;
	short dw = *((short *)data);

	if( !guilds->load(guilds, &g, guild_id) )
		return;

	switch( type )
	{
	case GBI_GUILDLV:
		if( dw > 0 && g.guild_lv + dw <= 50 )
		{
			g.guild_lv += dw;
			g.skill_point += dw;
		}
		else
		if( dw < 0 && g.guild_lv + dw >= 1 )
			g.guild_lv += dw;

		mapif_guild_info(-1, &g);
		guilds->save(guilds, &g, GS_LEVEL);
		return;
	default:
		ShowError("int_guild: GuildBasicInfoChange: Unknown type %d\n", type);
		break;
	}

	mapif_guild_basicinfochanged(guild_id, type, data, len);
}

// ギルドメンバデータ変更要求
void mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len)
{
	int i;
	struct guild g;
	enum guild_save_flag save_flag = 0;

	if( !guilds->load(guilds, &g, guild_id) )
		return;

	// find the member
	ARR_FIND( 0, g.max_member, i, g.member[i].account_id == account_id && g.member[i].char_id == char_id );
	if( i == g.max_member )
	{
		ShowWarning("int_guild: GuildMemberChange: Not found %d,%d in guild (%d - %s)\n", account_id, char_id, guild_id, g.name);
		return;
	}

	switch(type)
	{
	case GMI_POSITION:
		g.member[i].position = *((int *)data);
		break;
	case GMI_EXP:
	{
		unsigned int exp, old_exp = g.member[i].exp;
		g.member[i].exp = *((unsigned int *)data);
		if( g.member[i].exp > old_exp )
		{
			exp = g.member[i].exp - old_exp;

			// Compute gained exp
			if (guild_exp_rate != 100)
				exp = exp*guild_exp_rate/100;

			// Update guild exp
			if (exp > UINT_MAX - g.exp)
				g.exp = UINT_MAX;
			else
				g.exp += exp;

			//TODO: check if this is working correctly
			if( guild_calcinfo(&g) )
				mapif_guild_info(-1, &g);
			else
				mapif_guild_basicinfochanged(guild_id, GBI_EXP, &g.exp, 4);
			save_flag |= GS_LEVEL;
		}
		break;
	}
	case GMI_HAIR:
		g.member[i].hair = *((int *)data);
		break;
	case GMI_HAIR_COLOR:
		g.member[i].hair_color = *((int *)data);
		break;
	case GMI_GENDER:
		g.member[i].gender = *((int *)data);
		break;
	case GMI_CLASS:
		g.member[i].class_ = *((int *)data);
		break;
	case GMI_LEVEL:
		g.member[i].lv = *((int *)data);
		break;

	default:
		ShowError("int_guild: GuildMemberInfoChange: Unknown type %d\n", type);
		return;
	}

	mapif_guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);

	g.member[i].modified = GS_MEMBER_MODIFIED;
	save_flag |= GS_MEMBER;

	guilds->save(guilds, &g, save_flag);
}

int inter_guild_sex_changed(int guild_id,int account_id,int char_id, int gender)
{
	mapif_parse_GuildMemberInfoChange(0, guild_id, account_id, char_id, GMI_GENDER, (const char*)&gender, sizeof(gender));
	return 0;
}

// ギルド役職名変更要求
void mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position* p)
{
	struct guild g;

	if( !guilds->load(guilds, &g, guild_id) )
		return;
	if( idx < 0 || idx >= MAX_GUILDPOSITION )
		return;

	memcpy(&g.position[idx], p, sizeof(struct guild_position));
	g.position[idx].modified = GS_POSITION_MODIFIED;
	mapif_guild_position(&g, idx);

	guilds->save(guilds, &g, GS_POSITION);
}

// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd, int guild_id, int skill_num, int account_id)
{
	struct guild g;
	int idx = skill_num - GD_SKILLBASE;

	if( !guilds->load(guilds, &g, guild_id) )
		return 0;
	if( idx < 0 || idx >= MAX_GUILDSKILL )
		return 0;

	if( g.skill_point > 0 && g.skill[idx].id > 0 && g.skill[idx].lv < 10 ) //FIXME: what's with this '< 10' check?
	{
		g.skill[idx].lv++;
		g.skill_point--;
		guild_calcinfo(&g);
		mapif_guild_info(-1, &g);
		mapif_guild_skillupack(guild_id, skill_num, account_id);

		guilds->save(guilds, &g, GS_LEVEL|GS_SKILL);
	}

	return 0;
}

// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag)
{
	struct guild g[2];
	bool b[2];
	int j, i;

	b[0] = guilds->load(guilds, &g[0], guild_id1);
	b[1] = guilds->load(guilds, &g[1], guild_id2);

	if( b[0] && !b[1] && (flag & GUILD_ALLIANCE_REMOVE) ) // Requested to remove an alliance with a not found guild.
	{// Try to do a manual removal of said guild.
		ARR_FIND( 0, MAX_GUILDALLIANCE, i, g[0].alliance[i].guild_id == guild_id2 );
		if( i == MAX_GUILDALLIANCE )
			return -1;

		mapif_guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, g[0].name, g[0].alliance[i].name);
		g[0].alliance[i].guild_id = 0;
		guilds->save(guilds, &g[0], GS_ALLIANCE);
	}

	if( !b[0] || !b[1] )
		return 0;

	if( flag&GUILD_ALLIANCE_REMOVE )
	{// Remove alliance/opposition, in case of alliance, remove on both side
		for( i = 0; i < 2 - (flag & GUILD_ALLIANCE_TYPE_MASK); i++ )
		{
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i].alliance[j].guild_id == g[1-i].guild_id && g[i].alliance[j].opposition == (flag&GUILD_ALLIANCE_TYPE_MASK) );
			if( j < MAX_GUILDALLIANCE )
				g[i].alliance[j].guild_id = 0;
		}
	}
	else
	{// Add alliance, in case of alliance, add on both side
		for( i = 0; i < 2 - (flag & GUILD_ALLIANCE_TYPE_MASK); i++ )
		{
			// Search an empty slot
			ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i].alliance[j].guild_id == 0 );
			if( j < MAX_GUILDALLIANCE )
			{
				g[i].alliance[j].guild_id = g[1-i].guild_id;
				memcpy(g[i].alliance[j].name, g[1-i].name, NAME_LENGTH);
				g[i].alliance[j].opposition = flag & GUILD_ALLIANCE_TYPE_MASK;
			}
		}
	}

	// Send on all map the new alliance/opposition
	mapif_guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, g[0].name, g[1].name);

	// Save changes
	guilds->save(guilds, &g[0], GS_ALLIANCE);
	guilds->save(guilds, &g[1], GS_ALLIANCE);

	return 0;
}

// ギルド告知変更要求
void mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	struct guild g;

	if( !guilds->load(guilds, &g, guild_id) )
		return;

	safestrncpy(g.mes1, mes1, sizeof(g.mes1));
	safestrncpy(g.mes2, mes2, sizeof(g.mes2));

	if( !guilds->save(guilds, &g, GS_MES) )
		return;

	mapif_guild_notice(&g);
}

// ギルドエンブレム変更要求
void mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	struct guild g;

	if( !guilds->load(guilds, &g, guild_id) )
		return;

	if (len > sizeof(g.emblem_data))
		len = sizeof(g.emblem_data);

	memcpy(g.emblem_data, data, len);
	g.emblem_len = len;
	g.emblem_id++;

	if( !guilds->save(guilds, &g, GS_EMBLEM) )
		return;

	mapif_guild_emblem(&g);
}

void mapif_parse_GuildCastleDataLoad(int fd, int castle_id, int index)
{
	struct guild_castle gc;

	if( !castles->load(castles, &gc, castle_id) )
	{
		mapif_guild_castle_dataload(castle_id, 0, 0);
		return;
	}

	switch(index)
	{
	case 1: mapif_guild_castle_dataload(gc.castle_id, index, gc.guild_id); break;
	case 2: mapif_guild_castle_dataload(gc.castle_id, index, gc.economy); break;
	case 3: mapif_guild_castle_dataload(gc.castle_id, index, gc.defense); break;
	case 4: mapif_guild_castle_dataload(gc.castle_id, index, gc.triggerE); break;
	case 5: mapif_guild_castle_dataload(gc.castle_id, index, gc.triggerD); break;
	case 6: mapif_guild_castle_dataload(gc.castle_id, index, gc.nextTime); break;
	case 7: mapif_guild_castle_dataload(gc.castle_id, index, gc.payTime); break;
	case 8: mapif_guild_castle_dataload(gc.castle_id, index, gc.createTime); break;
	case 9: mapif_guild_castle_dataload(gc.castle_id, index, gc.visibleC); break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		mapif_guild_castle_dataload(gc.castle_id, index, gc.guardian[index-10].visible); break;
	default:
		ShowError("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
	}
}

void mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
	struct guild_castle gc;

	if( !castles->load(castles, &gc, castle_id) )
	{
		//FIXME: why's a positive reply being sent here?
		mapif_guild_castle_datasave(castle_id, index, value);
		return;
	}

	switch( index )
	{
	case 1:
	{// castle owner change
		struct guild g;
		int	gid;

		if( gc.guild_id == value )
			break;

		// value = conquering guild_id, or 0 for abandoning the castle
		gid = ( value != 0 ) ? value : gc.guild_id;

		if( !guilds->load(guilds, &g, gid) )
			safestrncpy(g.name, "??", sizeof(g.name));

		interlog_log("guild %s (id=%d) %s castle id=%d\n", g.name, gid, (value) ? "occupy" : "abandon", castle_id);

		gc.guild_id = value;

		if( value == 0 ) // Delete guardians.
			memset(&gc.guardian, 0, sizeof(gc.guardian));
	}
		break;
	case 2: gc.economy = value; break;
	case 3: gc.defense = value; break;
	case 4: gc.triggerE = value; break;
	case 5: gc.triggerD = value; break;
	case 6: gc.nextTime = value; break;
	case 7: gc.payTime = value; break;
	case 8: gc.createTime = value; break;
	case 9: gc.visibleC = value; break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		gc.guardian[index-10].visible = value; break;
	default:
		ShowError("mapif_parse_GuildCastleDataSave ERROR!! (Not found index=%d)\n", index);
		return;
	}

	castles->save(castles, &gc);

	mapif_guild_castle_datasave(gc.castle_id, index, value);
}

void mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len)
{
	struct guild g;
	struct guild_member m;
	int pos;

	if( !guilds->load(guilds, &g, guild_id) )
		return;
	if( len > NAME_LENGTH )
		return;

	// Find member (name)
	ARR_FIND( 0, g.max_member, pos, strncmp(g.member[pos].name, name, len) == 0 );
	if( pos == g.max_member )
		return; //Character not found??

	// Switch current and old GLs
	memcpy(&m, &g.member[pos], sizeof (struct guild_member));
	memcpy(&g.member[pos], &g.member[0], sizeof(struct guild_member));
	memcpy(&g.member[0], &m, sizeof(struct guild_member));

	// Switch positions
	g.member[pos].position = g.member[0].position;
	g.member[0].position = 0; //Position 0: guild Master.
	g.member[pos].modified = GS_MEMBER_MODIFIED;
	g.member[0].modified = GS_MEMBER_MODIFIED;

	// Write new leader name
	safestrncpy(g.master, name, NAME_LENGTH);

	// update base info
	guilds->save(guilds, &g, GS_BASIC|GS_MEMBER);

	ShowInfo("int_guild: Guildmaster Changed to %s (Guild %d - %s)\n", g.master, guild_id, g.name);
	mapif_guild_master_changed(&g, g.member[0].account_id, g.member[0].char_id);
}


// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif_parse_CreateGuild(fd, RFIFOL(fd,4), (char*)RFIFOP(fd,8), (struct guild_member *)RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfoRequest(fd, RFIFOL(fd,2)); break;
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

void inter_guild_init(GuildDB* gdb, CastleDB* cdb)
{
	guilds = gdb;
	castles = cdb;

	//Read guild exp table
	inter_guild_exp_readdb();
}

void inter_guild_final(void)
{
	guilds = NULL;
	castles = NULL;
}
