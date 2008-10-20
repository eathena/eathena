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
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	struct guild *g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
#else
	g = inter_guild_fromsql(guild_id);
#endif
	if( g == NULL )
		return 0;

	memcpy(g->mes1, mes1, 60);
	memcpy(g->mes2, mes2, 120);

#ifndef TXT_ONLY
	g->save_flag |= GS_MES;	//Change mes of guild
#endif

	return mapif_guild_notice(g);
}

// ギルドエンブレム変更要求
int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	struct guild* g;

#ifdef TXT_ONLY
	g = (struct guild*)idb_get(guild_db, guild_id);
	if( g == NULL )
		return 0;
#else
	g = inter_guild_fromsql(guild_id);
	if( g == NULL )
		return 0;
#endif

	if (len > sizeof(g->emblem_data))
		len = sizeof(g->emblem_data);

	memcpy(g->emblem_data, data, len);
	g->emblem_len = len;
	g->emblem_id++;

#ifndef TXT_ONLY
	g->save_flag |= GS_EMBLEM;	//Change guild
#endif

	return mapif_guild_emblem(g);
}

int mapif_parse_GuildCastleDataLoad(int fd, int castle_id, int index)
{
#ifdef TXT_ONLY
	struct guild_castle *gc;
	
	gc = (struct guild_castle*)idb_get(castle_db, castle_id);
	if (gc == NULL)
		return mapif_guild_castle_dataload(castle_id, 0, 0);
#else
	struct guild_castle tmp, *gc;
	gc = &tmp;

	if (!inter_guildcastle_fromsql(castle_id, &gc))
		return mapif_guild_castle_dataload(castle_id,0,0);
#endif

	switch(index) {
	case 1: return mapif_guild_castle_dataload(gc->castle_id, index, gc->guild_id); break;
	case 2: return mapif_guild_castle_dataload(gc->castle_id, index, gc->economy); break;
	case 3: return mapif_guild_castle_dataload(gc->castle_id, index, gc->defense); break;
	case 4: return mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerE); break;
	case 5: return mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerD); break;
	case 6: return mapif_guild_castle_dataload(gc->castle_id, index, gc->nextTime); break;
	case 7: return mapif_guild_castle_dataload(gc->castle_id, index, gc->payTime); break;
	case 8: return mapif_guild_castle_dataload(gc->castle_id, index, gc->createTime); break;
	case 9: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleC); break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		return mapif_guild_castle_dataload(gc->castle_id, index, gc->guardian[index-10].visible); break;
	default:
		ShowError("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
		return 0;
	}

	return 0;
}

int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
#ifdef TXT_ONLY
	struct guild_castle *gc;

	gc = (struct guild_castle*)idb_get(castle_db, castle_id);
	if (gc == NULL)
		return mapif_guild_castle_datasave(castle_id, index, value);
#else
	struct guild_castle tmp, *gc;
	gc = &tmp;

	if(!inter_guildcastle_fromsql(castle_id, gc))
		return mapif_guild_castle_datasave(castle_id, index, value);
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
		return 0;
	}

#ifndef TXT_ONLY
	inter_guildcastle_tosql(&gc);
#endif

	return mapif_guild_castle_datasave(gc->castle_id, index, value);
}

int mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len)
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
		return 0;
	
	// Find member (name)
	ARR_FIND( 0, g->max_member, pos, strncmp(g->member[pos].name, name, len) == 0 );
	if (pos == g->max_member)
		return 0; //Character not found??
	
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
	return mapif_guild_master_changed(g, g->member[0].account_id, g->member[0].char_id);
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
int inter_guild_mapif_init(int fd)
{
	return mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(int guild_id, int account_id, int char_id)
{
	return mapif_parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "** Character Deleted **");
}
