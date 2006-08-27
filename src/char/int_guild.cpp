// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "baseio.h"
#include "inter.h"
#include "int_guild.h"
#include "int_storage.h"
#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "db.h"
#include "lock.h"
#include "showmsg.h"
#include "utils.h"
#include "malloc.h"


int mapif_parse_GuildLeave(int fd, uint32 guild_id, uint32 account_id, uint32 char_id, int flag, const char *mes);
int mapif_guild_broken(uint32 guild_id, int flag);
int guild_check_empty(CGuild &g);
int mapif_guild_basicinfochanged(uint32 guild_id, int type, uint32 data);
int mapif_guild_info(int fd, CGuild &g);


CGuildDB	cGuildDB;




// ギルドデータの読み込み
int inter_guild_init()
{
	cGuildDB.init(CHAR_CONF_NAME);
	return 0;
}

void inter_guild_final()
{
	return;
}


//-------------------------------------------------------------------
// map serverへの通信

// ギルド作成可否
int mapif_guild_created(int fd, uint32 account_id, CGuild &g)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3830;
		WFIFOL(fd,2) = account_id;
		WFIFOL(fd,6) = g.guild_id;
		WFIFOSET(fd,10);
		ShowMessage("int_guild: created! %d %s\n", g.guild_id, g.name);
	}
	return 0;
}
int mapif_guild_create_failed(int fd, uint32 account_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3830;
		WFIFOL(fd,2) = account_id;
		WFIFOL(fd,6) = 0;
		WFIFOSET(fd,10);
	}
	return 0;
}

// ギルド情報見つからず
int mapif_guild_noinfo(int fd, int guild_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3831;
		WFIFOW(fd,2) = 8;
		WFIFOL(fd,4) = guild_id;
		WFIFOSET(fd,8);
		//ShowError("int_guild: info not found %d\n", guild_id);
	}
	return 0;
}

// ギルド情報まとめ送り
int mapif_guild_info(int fd, CGuild &g)
{
	unsigned char buf[16384];
	WBUFW(buf,0) = 0x3831;
	WBUFW(buf,2) = 4 + sizeof(struct guild);
	guild_tobuffer(g, WBUFP(buf,4));
	if( !session_isActive(fd) )
		mapif_sendall(buf, 4 + sizeof(struct guild));
	else
		mapif_send(fd, buf, 4 + sizeof(struct guild));
	//ShowMessage("int_guild: info %d %s\n", g->guild_id, g->name);
	return 0;
}

// メンバ追加可否
int mapif_guild_memberadded(int fd, uint32 guild_id, uint32 account_id, uint32 char_id, int flag)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3832;
		WFIFOL(fd,2) = guild_id;
		WFIFOL(fd,6) = account_id;
		WFIFOL(fd,10) = char_id;
		WFIFOB(fd,14) = flag;
		WFIFOSET(fd, 15);
	}
	return 0;
}

// 脱退/追放通知
int mapif_guild_leaved(uint32 guild_id, uint32 account_id, uint32 char_id, int flag, const char *name, const char *mes)
{
	unsigned char buf[79];

	WBUFW(buf, 0) = 0x3834;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = account_id;
	WBUFL(buf,10) = char_id;
	WBUFB(buf,14) = flag;
	memcpy(WBUFP(buf,15), mes, 40);
	memcpy(WBUFP(buf,55), name, 24);
	mapif_sendall(buf, 79);
	ShowMessage("int_guild: guild leaved %d %d %s %s\n", guild_id, account_id, name, mes);

	return 0;
}

// オンライン状態とLv更新通知
int mapif_guild_memberinfoshort(CGuild &g, size_t idx)
{
	if(idx<MAX_GUILD)
	{
		unsigned char buf[19];
		WBUFW(buf, 0) = 0x3835;
		WBUFL(buf, 2) = g.guild_id;
		WBUFL(buf, 6) = g.member[idx].account_id;
		WBUFL(buf,10) = g.member[idx].char_id;
		WBUFB(buf,14) = (unsigned char)g.member[idx].online;
		WBUFW(buf,15) = g.member[idx].lv;
		WBUFW(buf,17) = g.member[idx].class_;
		mapif_sendall(buf, 19);
	}
	return 0;
}

// 解散通知
int mapif_guild_broken(uint32 guild_id, int flag)
{
	unsigned char buf[7];

	WBUFW(buf,0) = 0x3836;
	WBUFL(buf,2) = guild_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	ShowMessage("int_guild: broken %d\n", guild_id);

	return 0;
}

// ギルド内発言
int mapif_guild_message(uint32 guild_id, uint32 account_id, char *mes, size_t len, int sfd)
{
	CREATE_BUFFER(buf,unsigned char,len+12);

	WBUFW(buf,0) = 0x3837;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = guild_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf, len + 12);

	DELETE_BUFFER(buf);

	return 0;
}

// ギルド基本情報変更通知
int mapif_guild_basicinfochanged(uint32 guild_id, int type, uint32 data)
{
	unsigned char buf[2048];
	WBUFW(buf, 0) = 0x3839;
	WBUFW(buf, 2) = 14;
	WBUFL(buf, 4) = guild_id;
	WBUFW(buf, 8) = type;
	WBUFL(buf,10) = data;
	mapif_sendall(buf,14);
	return 0;
}

// ギルドメンバ情報変更通知
int mapif_guild_memberinfochanged(uint32 guild_id, uint32 account_id, uint32 char_id, int type, uint32 data)
{
	unsigned char buf[22];
	WBUFW(buf, 0) = 0x383a;
	WBUFW(buf, 2) = 22;
	WBUFL(buf, 4) = guild_id;
	WBUFL(buf, 8) = account_id;
	WBUFL(buf,12) = char_id;
	WBUFW(buf,16) = type;
	WBUFL(buf,18) = data;
	mapif_sendall(buf,22);
	return 0;
}

// ギルドスキルアップ通知
int mapif_guild_skillupack(uint32 guild_id, uint32 skill_num, uint32 account_id)
{
	unsigned char buf[14];

	WBUFW(buf, 0) = 0x383c;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = skill_num;
	WBUFL(buf,10) = account_id;
	mapif_sendall(buf, 14);

	return 0;
}

// ギルド同盟/敵対通知
int mapif_guild_alliance(uint32 guild_id1, uint32 guild_id2, uint32 account_id1, uint32 account_id2, int flag, const char *name1, const char *name2)
{
	unsigned char buf[128];

	WBUFW(buf, 0) = 0x383d;
	WBUFL(buf, 2) = guild_id1;
	WBUFL(buf, 6) = guild_id2;
	WBUFL(buf,10) = account_id1;
	WBUFL(buf,14) = account_id2;
	WBUFB(buf,18) = flag;
	memcpy(WBUFP(buf,19), name1, 24);
	memcpy(WBUFP(buf,43), name2, 24);
	mapif_sendall(buf, 67);

	return 0;
}

// ギルド役職変更通知
int mapif_guild_position(CGuild &g, size_t idx)
{
	if(idx < MAX_GUILDPOSITION)
	{
		unsigned char buf[128];
		WBUFW(buf,0) = 0x383b;
		WBUFW(buf,2) = sizeof(struct guild_position) + 12;
		WBUFL(buf,4) = g.guild_id;
		WBUFL(buf,8) = idx;
		guild_position_tobuffer(g.position[idx], WBUFP(buf,12));
		mapif_sendall(buf, sizeof(struct guild_position) + 12);
	}
	return 0;
}

// ギルド告知変更通知
int mapif_guild_notice(CGuild &g)
{
	unsigned char buf[186];

	WBUFW(buf,0) = 0x383e;
	WBUFL(buf,2) = g.guild_id;
	memcpy(WBUFP(buf,6), g.mes1, 60);
	memcpy(WBUFP(buf,66), g.mes2, 120);
	mapif_sendall(buf, 186);

	return 0;
}

// ギルドエンブレム変更通知
int mapif_guild_emblem(CGuild &g)
{
	unsigned char buf[2048+12];

	WBUFW(buf,0) = 0x383f;
	WBUFW(buf,2) = g.emblem_len + 12;
	WBUFL(buf,4) = g.guild_id;
	WBUFL(buf,8) = g.emblem_id;
	memcpy(WBUFP(buf,12), g.emblem_data, g.emblem_len);
	mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

int mapif_guild_castle_dataload(unsigned short castle_id, unsigned char index, int value)
{
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3840;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_datasave(unsigned short castle_id, unsigned char index, int value)
{
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3841;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_alldataload(int fd)
{
	if( session_isActive(fd) )
	{
		size_t i, len = 4;
/*
		// using alternalte interface
		cGuildDB.aquireCastle();
		while( cGuildDB.isCastleOk() )
		{
			guild_castle_tobuffer( cGuildDB.getCastle(), WFIFOP(fd,len));
			len += sizeof(struct guild_castle);

			cGuildDB.nextCastle();
		}
		cGuildDB.releaseCastle();
*/
/*
		// using random access
		for(i=0; i<cGuildDB.castlesize(); ++i)
		{
			guild_castle_tobuffer( cGuildDB.castle(i), WFIFOP(fd,len));
			len += sizeof(struct guild_castle);
		}
*/
		// using specific implementation
		basics::vector<CCastle> cv;
		cGuildDB.getCastles(cv);
		for(i=0; i<cv.size(); ++i)
		{
			guild_castle_tobuffer( cv[i], WFIFOP(fd,len) );
			len += sizeof(struct guild_castle);
		}

		WFIFOW(fd,0) = 0x3842;
		WFIFOW(fd,2) = len;
		WFIFOSET(fd, len);
	}
	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信

// ギルド作成要求
int mapif_parse_CreateGuild(int fd, uint32 account_id, char *name, unsigned char *buf)
{
	CGuild g;
	size_t i;

	for(i = 0; i < 24 && name[i]; ++i) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			ShowMessage("int_guild: illeagal guild name [%s]\n", name);
			mapif_guild_create_failed(fd, account_id);
			return 0;
		}
	}

	if( cGuildDB.searchGuild(name, g) )
	{
		ShowMessage("int_guild: same name guild exists [%s]\n", name);
		mapif_guild_create_failed(fd, account_id);
	}
	else
	{
		struct guild_member master;
		guild_member_frombuffer(master, buf);

		cGuildDB.insertGuild(master, name, g);

		mapif_guild_created(fd, account_id, g);
		mapif_guild_info(fd, g);

		if(log_inter)
			inter_log("guild %s (id=%d) created by master %s (id=%d)" RETCODE,
				name, g.guild_id, g.member[0].name, g.member[0].account_id);
	}
	return 0;
}

// ギルド情報要求
int mapif_parse_GuildInfo(int fd, uint32 guild_id)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		g.calcInfo();
		mapif_guild_info(fd, g);
	}
	else
		mapif_guild_noinfo(fd, guild_id);

	return 0;
}

// ギルドメンバ追加要求
int mapif_parse_GuildAddMember(int fd, int guild_id, unsigned char *buf)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		size_t i;
		for(i=0; i<g.max_member && i<MAX_GUILD; ++i)
		{
			if(g.member[i].account_id == 0)
			{
				guild_member_frombuffer(g.member[i],buf);
				mapif_guild_memberadded(fd, guild_id, g.member[i].account_id, g.member[i].char_id, 0);
				g.calcInfo();
				mapif_guild_info(-1, g);
				g.save_flags = GUILD_SAFE_ALL & !GUILD_CLEAR_MEMBER;
				cGuildDB.saveGuild(g);
				return 0;
			}
		}
	}
	// failed
	struct guild_member member;
	guild_member_frombuffer(member,buf);
	mapif_guild_memberadded(fd, guild_id, member.account_id, member.char_id, 1);
	return 0;
}

// ギルド脱退/追放要求
int mapif_parse_GuildLeave(int fd, uint32 guild_id, uint32 account_id, uint32 char_id, int flag, const char *mes)
{
	CGuild g;
	int i, j;

	if( cGuildDB.searchGuild(guild_id, g) )
	{
		for(i=0; i<MAX_GUILD; ++i)
		{
			if (g.member[i].account_id == account_id && g.member[i].char_id == char_id)
			{
				if (flag)
				{	// 追放の場合追放リストに入れる
					for(j=0; j<MAX_GUILDEXPLUSION; ++j)
					{
						if (g.explusion[j].account_id == 0)
							break;
					}
					if (j == MAX_GUILDEXPLUSION)
					{	// 一杯なので古いのを消す
						for(j = 0; j < MAX_GUILDEXPLUSION - 1; ++j)
							g.explusion[j] = g.explusion[j+1];
						j = MAX_GUILDEXPLUSION - 1;
					}
					g.explusion[j].account_id = account_id;
					g.explusion[j].char_id    = char_id;
					memcpy(g.explusion[j].acc, "dummy", 24);
					memcpy(g.explusion[j].name, g.member[i].name, 24);
					memcpy(g.explusion[j].mes, mes, 40);
					g.explusion[j].mes[39]=0;
				}
				mapif_guild_leaved(guild_id, account_id, char_id, flag, g.member[i].name, mes);
				memset(&g.member[i], 0, sizeof(struct guild_member));

				if( g.is_empty() )
				{
					cGuildDB.removeGuild(g.guild_id);
				}
				else
				{
					mapif_guild_info(-1,g);// まだ人がいるのでデータ送信
					g.save_flags = GUILD_SAFE_ALL;
					cGuildDB.saveGuild(g);
				}
				break;
			}
		}
	}
	else
		mapif_guild_broken(guild_id, 0);
	return 0;
}

// オンライン/Lv更新
int mapif_parse_GuildChangeMemberInfoShort(int fd, uint32 guild_id, uint32 account_id, uint32 char_id, int online, int lv, int class_)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		int i, alv, c;
		g.connect_member = 0;

		alv = 0;
		c = 0;
		for(i=0; i<MAX_GUILD; ++i)
		{
			if (g.member[i].account_id == account_id && g.member[i].char_id == char_id) {
				g.member[i].online = online;
				g.member[i].lv = lv;
				g.member[i].class_ = class_;
				mapif_guild_memberinfoshort(g, i);
			}
			if (g.member[i].account_id > 0) {
				alv += g.member[i].lv;
				c++;
			}
			if (g.member[i].online)
				g.connect_member++;
		}
		
		if (c)
			g.average_lv = alv / c;

		g.save_flags = GUILD_SAFE_GUILD | GUILD_SAFE_MEMBER;
		cGuildDB.saveGuild(g);
	}
	return 0;
}

// ギルド解散要求
int mapif_parse_BreakGuild(int fd, uint32 guild_id)
{
	cGuildDB.removeGuild(guild_id);
	inter_guild_storage_delete(guild_id);
	mapif_guild_broken(guild_id, 0);
	if(log_inter)
		inter_log("guild broken (id=%d)" RETCODE, guild_id);
	return 0;
}

// ギルドメッセージ送信
int mapif_parse_GuildMessage(int fd, uint32 guild_id, uint32 account_id, char *mes, size_t len)
{
	return mapif_guild_message(guild_id, account_id, mes, len, fd);
}

// ギルド基本データ変更要求
int mapif_parse_GuildBasicInfoChange(int fd, uint32 guild_id, int type, uint32 data)
{
	CGuild g;

	if( cGuildDB.searchGuild(guild_id, g) )
	{
		switch(type)
		{
		case GBI_GUILDLV:
			if(g.guild_lv + data <= 50)
			{
				g.guild_lv+=data;
				g.skill_point+=data;
			}
			else if (g.guild_lv + data >= 1)
				g.guild_lv += data;
			mapif_guild_info(-1, g);
			break;
		default:
			ShowMessage("int_guild: GuildBasicInfoChange: Unknown type %d\n", type);
			break;
		}
		mapif_guild_basicinfochanged(guild_id, type, data);
		g.save_flags = GUILD_SAFE_GUILD;
		cGuildDB.saveGuild(g);
	}
	else
		mapif_guild_broken(guild_id, 0);
	return 0;
}

// ギルドメンバデータ変更要求
int mapif_parse_GuildMemberInfoChange(int fd, uint32 guild_id, uint32 account_id, uint32 char_id, unsigned short type, uint32 data)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		size_t i;
		for(i=0; i<g.max_member && i<MAX_GUILD; ++i)
		{
			if (g.member[i].account_id == account_id && g.member[i].char_id == char_id)
				break;
		}
		if( i>=g.max_member ||  i>=MAX_GUILD )
		{
			ShowError("int_guild: GuildMemberChange: Not found %d,%d in %d[%s]\n", account_id, char_id, guild_id, g.name);
			return 0;
		}

		switch(type)
		{
		case GMI_POSITION:	// 役職
			g.member[i].position = data;
			break;
		case GMI_EXP:	// EXP
		{
			int exp, oldexp = g.member[i].exp;
			exp = g.member[i].exp = data;
			g.exp += (exp - oldexp);
			g.calcInfo();	// Lvアップ判断
			mapif_guild_basicinfochanged(guild_id, GBI_EXP, g.exp);
			break;
		}
		default:
			ShowMessage("int_guild: GuildMemberInfoChange: Unknown type %d\n", type);
			break;
		}
		mapif_guild_memberinfochanged(guild_id, account_id, char_id, type, data);
		g.save_flags = GUILD_SAFE_ALL & !GUILD_CLEAR_MEMBER;
		cGuildDB.saveGuild(g);
	}
	else
		mapif_guild_broken(guild_id, 0);
	return 0;
}

// ギルド役職名変更要求
int mapif_parse_GuildPosition(int fd, uint32 guild_id, uint32 idx, unsigned char *buf)
{
	if( idx < MAX_GUILDPOSITION )
	{
		CGuild g;
		if( cGuildDB.searchGuild(guild_id, g) )
		{
			guild_position_frombuffer(g.position[idx],buf);
			mapif_guild_position(g, idx);
			ShowMessage("int_guild: position changed %d\n", idx);
			g.save_flags = GUILD_SAFE_GUILD | GUILD_SAFE_POSITION;
			cGuildDB.saveGuild(g);
		}
		else
			mapif_guild_broken(guild_id, 0);
	}
	return 0;
}

// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd, uint32 guild_id, uint32 skillid, uint32 account_id)
{
	CGuild g;
	int skillidx = skillid - GD_SKILLBASE;

	if( skillidx >= 0 && skillidx < MAX_GUILDSKILL &&
		cGuildDB.searchGuild(guild_id, g) )
	{
		if (g.skill_point > 0 && g.skill[skillidx].id > 0 && g.skill[skillidx].lv < 10)
		{
			g.skill[skillidx].lv++;
			g.skill_point--;
			if( g.calcInfo() )
				mapif_guild_info(-1, g);
			mapif_guild_skillupack(guild_id, skillid, account_id);
			ShowMessage("int_guild: skill %d up\n", skillid);
			g.save_flags = GUILD_SAFE_GUILD | GUILD_SAFE_SKILL;
			cGuildDB.saveGuild(g);
		}
	}
	return 0;
}

// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd, uint32 guild_id1, uint32 guild_id2, uint32 account_id1, uint32 account_id2, int flag)
{
	CGuild g[2];
	int j, i;

	if( cGuildDB.searchGuild(guild_id1, g[0]) && cGuildDB.searchGuild(guild_id2, g[1]) )
	{
		if (!(flag & 0x8))
		{
			for(i=0; i<2 - (flag&1); ++i)
			{
				for(j=0; j<MAX_GUILDALLIANCE; ++j)
				{
					if (g[i].alliance[j].guild_id == 0)
					{
						g[i].alliance[j].guild_id = g[1-i].guild_id;
						memcpy(g[i].alliance[j].name, g[1-i].name, 24);
						g[i].alliance[j].opposition = flag&1;
						break;
					}
				}
			}
		}
		else
		{	// 関係解消
			for(i=0; i<2 - (flag&1); ++i)
			{
				for(j=0; j<MAX_GUILDALLIANCE; ++j)
				{
					if (g[i].alliance[j].guild_id == g[1-i].guild_id && g[i].alliance[j].opposition == (flag & 1))
					{
						g[i].alliance[j].guild_id = 0;
						break;
					}
				}
			}
		}
		mapif_guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, g[0].name, g[1].name);
		g[0].save_flags = GUILD_SAFE_ALLIANCE;
		cGuildDB.saveGuild(g[0]);
		g[1].save_flags = GUILD_SAFE_ALLIANCE;
		cGuildDB.saveGuild(g[1]);
	}
	return 0;
}

// ギルド告知変更要求
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		memcpy(g.mes1, mes1, 60);
		memcpy(g.mes2, mes2, 120);
		g.save_flags = GUILD_SAFE_GUILD;
		cGuildDB.saveGuild(g);
		return mapif_guild_notice(g);
	}
	else
		mapif_guild_broken(guild_id, 0);
	return 0;
}

// ギルドエンブレム変更要求
int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	CGuild g;
	if( cGuildDB.searchGuild(guild_id, g) )
	{
		if(len>2048) len = 2048;// test for buffer limit
		memcpy(g.emblem_data, data, len);
		g.emblem_len = len;
		g.emblem_id++;
		g.save_flags = GUILD_SAFE_GUILD;
		cGuildDB.saveGuild(g);
		return mapif_guild_emblem(g);
	}
	else
		mapif_guild_broken(guild_id, 0);
	return 0;
}

int mapif_parse_GuildCastleDataLoad(int fd, unsigned short castle_id, unsigned short index)
{
	CCastle gc;
	if( cGuildDB.searchCastle(castle_id, gc) )
	{
		switch(index) {
		case 1: return mapif_guild_castle_dataload(gc.castle_id, index, gc.guild_id);
		case 2: return mapif_guild_castle_dataload(gc.castle_id, index, gc.economy);
		case 3: return mapif_guild_castle_dataload(gc.castle_id, index, gc.defense);
		case 4: return mapif_guild_castle_dataload(gc.castle_id, index, gc.triggerE);
		case 5: return mapif_guild_castle_dataload(gc.castle_id, index, gc.triggerD);
		case 6: return mapif_guild_castle_dataload(gc.castle_id, index, gc.nextTime);
		case 7: return mapif_guild_castle_dataload(gc.castle_id, index, gc.payTime);
		case 8: return mapif_guild_castle_dataload(gc.castle_id, index, gc.createTime);
		case 9: return mapif_guild_castle_dataload(gc.castle_id, index, gc.visibleC);
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			mapif_guild_castle_dataload(gc.castle_id, index, gc.guardian[index-10].visible);
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
			return mapif_guild_castle_dataload(gc.castle_id, index, gc.guardian[index-18].guardian_hp);

		default:
			ShowError("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
			return 0;
		}
	}
	else
	{	// this is causing the loadack error
		ShowMessage("called castle %i, index %i; does not exist.\n", castle_id, index);
		return mapif_guild_castle_dataload(castle_id, 0, 0);
	}
}

int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
	CCastle gc;
	if( cGuildDB.searchCastle(castle_id, gc) )
	{
		switch(index) {
		case 1:
			if(gc.guild_id != (uint32)value && log_inter)
			{
				int gid = (value) ? value : gc.guild_id;
				CGuild g;
				bool found = cGuildDB.searchGuild(gid,g);
				inter_log("guild %s (id=%d) %s castle id=%d" RETCODE,
					(found) ? g.name : "??", gid, (value) ? "occupy" : "abandon", castle_id);
			}
			gc.guild_id = value;
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
			gc.guardian[index-10].visible = (0!=value); break;
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
			 gc.guardian[index-18].guardian_hp = value; break;
		default:
			ShowError("mapif_parse_GuildCastleDataSave ERROR!! (Not found index=%d)\n", index);
			return 0;
		}
		cGuildDB.saveCastle(gc);
		return mapif_guild_castle_datasave(gc.castle_id, index, value);
	}
	else
	{
		return mapif_guild_castle_datasave(castle_id, index, value);
	}
}

// ギルドチェック要求
int mapif_parse_GuildCheck(int fd, uint32 guild_id, uint32 account_id, uint32 char_id)
{
/*
	// using alternate interface
	size_t k;
	cGuildDB.aquireGuild();
	while( cGuildDB.isGuildOk() )
	{	CGuild &g = cGuildDB.getGuild();
		if( g.guild_id != guild_id )
		{
			for(k=0; k<MAX_GUILD; ++k)
			{
				if( g.member[k].account_id == account_id && 
					g.member[k].char_id == char_id)
				{	// 別のギルドに偽の所属データがあるので脱退
					ShowMessage("int_guild: guild conflict! %d,%d %d!=%d\n", 
						account_id, char_id, guild_id, g.guild_id);
					mapif_parse_GuildLeave(-1, g.guild_id, account_id, char_id, 0, "**データ競合**");
					
					goto finish; // not nice but fast
				}
			}
		}
		cGuildDB.nextGuild();
	}
finish:
	cGuildDB.releaseGuild();
*/
/*
	// using random access operator
	size_t i,k;
	for(i=0; i<cGuildDB.size(); ++i)
	{
		if( cGuildDB[i].guild_id != guild_id )
		{
			for(k=0; k<MAX_GUILD; ++k)
			{
				if( cGuildDB[i].member[k].account_id == account_id && 
					cGuildDB[i].member[k].char_id == char_id)
				{	// 別のギルドに偽の所属データがあるので脱退
					ShowMessage("int_guild: guild conflict! %d,%d %d!=%d\n", 
						account_id, char_id, guild_id, cGuildDB[i].guild_id);
					mapif_parse_GuildLeave(-1, cGuildDB[i].guild_id, account_id, char_id, 0, "**データ競合**");
					return 0;
				}
			}
		}
	}
*/
	// using special implementation
	const uint32 other_guild = cGuildDB.has_conflict(guild_id, account_id, char_id);
	if( other_guild )
		mapif_parse_GuildLeave(-1, other_guild, account_id, char_id, 0, "");
	return 0;
}

// マップサーバーの接続時処理
int inter_guild_mapif_init(int fd)
{
	return mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(uint32 guild_id, uint32 account_id, uint32 char_id)
{
	return mapif_parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "**サーバー命令**");
}


// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif_parse_CreateGuild(fd, RFIFOL(fd,4), (char*)RFIFOP(fd,8), RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfo(fd, RFIFOL(fd,2)); break;
	case 0x3032: mapif_parse_GuildAddMember(fd, RFIFOL(fd,4), RFIFOP(fd,8)); break;
	case 0x3034: mapif_parse_GuildLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), (char*)RFIFOP(fd,15)); break;
	case 0x3035: mapif_parse_GuildChangeMemberInfoShort(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOW(fd,15), RFIFOW(fd,17)); break;
	case 0x3036: mapif_parse_BreakGuild(fd, RFIFOL(fd,2)); break;
	case 0x3037: mapif_parse_GuildMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3038: mapif_parse_GuildCheck(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x3039: mapif_parse_GuildBasicInfoChange(fd, RFIFOL(fd,4), RFIFOW(fd,8), RFIFOL(fd,10)); break;
	case 0x303A: mapif_parse_GuildMemberInfoChange(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOL(fd,12), RFIFOW(fd,16), RFIFOL(fd,18)); break;
	case 0x303B: mapif_parse_GuildPosition(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12)); break;
	case 0x303C: mapif_parse_GuildSkillUp(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x303D: mapif_parse_GuildAlliance(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOB(fd,18)); break;
	case 0x303E: mapif_parse_GuildNotice(fd, RFIFOL(fd,2), (char*)RFIFOP(fd,6), (char*)RFIFOP(fd,66)); break;
	case 0x303F: mapif_parse_GuildEmblem(fd, RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12)); break;
	case 0x3040: mapif_parse_GuildCastleDataLoad(fd, RFIFOW(fd,2), RFIFOB(fd,4)); break;
	case 0x3041: mapif_parse_GuildCastleDataSave(fd, RFIFOW(fd,2), RFIFOB(fd,4), RFIFOL(fd,5)); break;

	default:
		return 0;
	}

	return 1;
}
