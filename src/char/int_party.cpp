// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "baseio.h"
#include "utils.h"
#include "malloc.h"
#include "socket.h"
#include "db.h"
#include "showmsg.h"
#include "lock.h"
#include "mmo.h"

#include "inter.h"
#include "int_party.h"
#include "char.h"


int mapif_party_broken(uint32 party_id, int flag);
int mapif_parse_PartyLeave(int fd, uint32 party_id, uint32 account_id);



CPartyDB	cPartyDB;
basics::CParam<size_t> party_share_level("party_share_level", 10);


// パーティデータのロード
int inter_party_init()
{
	cPartyDB.init(CHAR_CONF_NAME);
	return 0;
}

void inter_party_final()
{
	return;
}


// EXP公平分配できるかチェック
bool party_check_exp_share(CParty &p)
{
	size_t i, cnt_lo=0, cnt_hi=0;
	unsigned short maxlv = 0, minlv = 0xFFFF;

	for(i = 0; i < MAX_PARTY; ++i)
	{
		unsigned short lv = p.member[i].lv;
		if(lv && p.member[i].account_id && p.member[i].online)
		{
			if(minlv == 0xFFFF)
				minlv = maxlv = lv;
			else if (lv < minlv)
				minlv = lv;
			else if (maxlv < lv)
				maxlv = lv;
			cnt_lo++;
			if( lv >= 70 )
				cnt_hi++;
		}
	}
	// check for party with parents with child
	if( (cnt_hi >= 2) && (cnt_lo == 3) &&
		(0==strcmp(p.member[0].mapname,p.member[1].mapname)) &&
		(0==strcmp(p.member[1].mapname,p.member[2].mapname)) )
	{
		//ShowMessage("PARTY: group of 3 Id1 %d lv %d name %s Id2 %d lv %d name %s Id3 %d lv %d name %s\n",pl1,p->member[0].lv,p->member[0].name,pl2,p->member[1].lv,p->member[1].name,pl3,p->member[2].lv,p->member[2].name);
		if( (char_married(p.member[0].name,p.member[1].name) && char_child(p.member[0].name,p.member[2].name)) ||
			(char_married(p.member[0].name,p.member[2].name) && char_child(p.member[0].name,p.member[1].name)) ||
			(char_married(p.member[1].name,p.member[2].name) && char_child(p.member[1].name,p.member[0].name)) )
			return true;
	}
	return (maxlv==0) || (maxlv<=(minlv+party_share_level()));
}

// キャラの競合がないかチェック
int party_check_conflict(uint32 party_id, uint32 account_id, const char *nick)
{
	size_t i,k;
	for(i=0; i<cPartyDB.size(); ++i)
	{
		// 本来の所属なので問題なし
		if( cPartyDB[i].party_id != party_id )
		{
			for(k=0; k<MAX_PARTY; ++k)
			{
				if( cPartyDB[i].member[k].account_id == account_id && 0==strcmp(cPartyDB[i].member[k].name, nick) )
				{	// 別のパーティに偽の所属データがあるので脱退
					ShowMessage("int_party: party conflict! %d %d->%d\n", account_id, party_id, cPartyDB[i].party_id);
					mapif_parse_PartyLeave(-1, cPartyDB[i].party_id, account_id);
					break;
				}
			}
		}	
	}
	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// パーティ作成可否
int mapif_party_created(int fd, uint32 account_id, CParty &p)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3820;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = 0;
		WFIFOL(fd,7) = p.party_id;
		memcpy(WFIFOP(fd,11), p.name, 24);
		WFIFOSET(fd,35);
		ShowMessage("int_party: created! %d %s\n", p.party_id, p.name);
	}
	return 0;
}
int mapif_party_create_failed(int fd, uint32 account_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3820;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = 1;
		WFIFOL(fd,7) = 0;
		memcpy(WFIFOP(fd,11), "error", 24);
		WFIFOSET(fd,35);
	}
	return 0;
}
// パーティ情報見つからず
int mapif_party_noinfo(int fd, int party_id)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3821;
		WFIFOW(fd,2) = 8;
		WFIFOL(fd,4) = party_id;
		WFIFOSET(fd,8);
		ShowError("int_party: info not found %d\n", party_id);
	}
	return 0;
}

// パーティ情報まとめ送り
int mapif_party_info(int fd, CParty& pparty)
{
	unsigned char buf[2048];
	WBUFW(buf,0) = 0x3821;
	WBUFW(buf,2) = 4 + sizeof(struct party);
	party_tobuffer(pparty, buf+4);
	
	if( session_isActive(fd) )
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
	//ShowMessage("int_party: info %d %s\n", p->party_id, p->name);
	return 0;
}

// パーティメンバ追加可否
int mapif_party_memberadded(int fd, uint32 party_id, uint32 account_id, int flag)
{
	if( session_isActive(fd) )
	{
		WFIFOW(fd,0) = 0x3822;
		WFIFOL(fd,2) = party_id;
		WFIFOL(fd,6) = account_id;
		WFIFOB(fd,10) = flag;
		WFIFOSET(fd,11);
	}
	return 0;
}

// パーティ設定変更通知
int mapif_party_optionchanged(int fd, const CParty &p, uint32 account_id, unsigned char flag)
{
	unsigned char buf[15];

	WBUFW(buf,0) = 0x3823;
	WBUFL(buf,2) = p.party_id;
	WBUFL(buf,6) = account_id;
	WBUFW(buf,10) = p.expshare;
	WBUFW(buf,12) = p.itemshare;
	WBUFB(buf,14) = flag;
	if (flag == 0)
		mapif_sendall(buf, 15);
	else
		mapif_send(fd, buf, 15);
	ShowMessage("int_party: option changed %d %d %d %d %d\n", p.party_id, account_id, p.expshare, p.itemshare, flag);
	return 0;
}

// パーティ脱退通知
int mapif_party_leaved(uint32 party_id,uint32 account_id, char *name)
{
	unsigned char buf[34];

	WBUFW(buf,0) = 0x3824;
	WBUFL(buf,2) = party_id;
	WBUFL(buf,6) = account_id;
	memcpy(WBUFP(buf,10), name, 24);
	mapif_sendall(buf, 34);
	ShowMessage("int_party: party leaved %d %d %s\n", party_id, account_id, name);

	return 0;
}

// パーティマップ更新通知
int mapif_party_membermoved(CParty &p, int idx)
{
	unsigned char buf[29];

	WBUFW(buf,0) = 0x3825;
	WBUFL(buf,2) = p.party_id;
	WBUFL(buf,6) = p.member[idx].account_id;
	memcpy(WBUFP(buf,10), p.member[idx].mapname, 16);
	WBUFB(buf,26) = p.member[idx].online;
	WBUFW(buf,27) = p.member[idx].lv;
	mapif_sendall(buf, 29);

	return 0;
}

// パーティ解散通知
int mapif_party_broken(uint32 party_id, int flag)
{
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3826;
	WBUFL(buf,2) = party_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	ShowMessage("int_party: broken %d\n", party_id);
	return 0;
}

// パーティ内発言
int mapif_party_message(uint32 party_id, uint32 account_id, const char *mes, size_t len, int sfd)
{
	CREATE_BUFFER(buf,unsigned char,len+12);
	WBUFW(buf,0) = 0x3827;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = party_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf,len + 12);
	DELETE_BUFFER(buf);
	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信


// パーティ
int mapif_parse_CreateParty(int fd, uint32 account_id, const char *name, const char *nick, const char *mapname, unsigned short lv)
{
	CParty p;
	const char *ip;

	if(NULL==name)
		return 0;

	for(ip=name; *ip; ++ip)
	{
		if ( *((unsigned char*)ip)==0xe0 || *ip== 0x7f)
		{
			ShowMessage("int_party: illegal party name [%s]\n", name);
			mapif_party_create_failed(fd, account_id);
			return 0;
		}
	}

	if( cPartyDB.searchParty(name, p)  )
	{
		ShowMessage("int_party: same name party exists [%s]\n", name);
		mapif_party_create_failed(fd, account_id);
		return 0;
	}

	if( cPartyDB.insertParty(account_id, nick, mapname, lv, name, p) )
	{
		mapif_party_created(fd, account_id, p);
		mapif_party_info(fd, p);
	}
	return 0;
}

// パーティ情報要求
int mapif_parse_PartyInfo(int fd, uint32 party_id)
{
	CParty p;
	if( cPartyDB.searchParty(party_id, p)  )
		mapif_party_info(fd, p);
	else
		mapif_party_noinfo(fd, party_id);
	return 0;
}

// パーティー設定変更要求
int mapif_parse_PartyChangeOption(int fd, uint32 party_id, uint32 account_id, unsigned short expshare, unsigned short itemshare)
{
	CParty p;
	int flag = 0;

	if( cPartyDB.searchParty(party_id, p) )
	{
		if( party_check_exp_share(p) )
		{
			p.itemshare = itemshare;
			p.expshare = expshare;
		}
		else
		{	// disable
			p.itemshare = p.expshare =  0;
			flag = expshare||itemshare;
		}
		mapif_party_optionchanged(fd, p, account_id, flag );
	}
	else
		mapif_party_broken(party_id, 0);
	return 0;
}

// パーティ追加要求
int mapif_parse_PartyAddMember(int fd, uint32 party_id, uint32 account_id, const char *nick, const char *mapname, ushort lv)
{
	CParty p;
	size_t i;
	bool ok=false;

	if( cPartyDB.searchParty(party_id, p)  )
	{
		for(i = 0; i < MAX_PARTY; ++i)
		{
			if( p.member[i].account_id == 0)
			{
				p.member[i].account_id = account_id;
				safestrcpy(p.member[i].name, sizeof(p.member[i].name), nick);
				safestrcpy(p.member[i].mapname, sizeof(p.member[i].mapname), mapname);
				char* ip=strchr(p.member[i].mapname,'.');
				if(ip) *ip=0;

				p.member[i].leader = 0;
				p.member[i].online = 1;
				p.member[i].lv = lv;

				ok=true;
				break;
			}
		}
		if(ok)
		{
			mapif_party_memberadded(fd, party_id, account_id, 0);
			mapif_party_info(-1, p);
			if( (p.itemshare||p.expshare) && !party_check_exp_share(p) )
			{	// disable the menu
				p.itemshare = p.expshare = 0;
				mapif_party_optionchanged(fd, p, 0, 0);
			}
			cPartyDB.saveParty(p);
		}
		else
		{
			mapif_party_memberadded(fd, party_id, account_id, 1);
		}
	}
	else
		mapif_party_broken(party_id,0);
	return 0;
}

// パーティ脱退要求
int mapif_parse_PartyLeave(int fd, uint32 party_id, uint32 account_id)
{
	CParty p;
	size_t i,j,k;
	bool broken=false;

	if( cPartyDB.searchParty(party_id, p) )
	{
		for(i=0; i<MAX_PARTY; ++i)
		{
			if(p.member[i].account_id == account_id)
			{	
				bool leader_leave = (p.member[i].leader==1);

				mapif_party_leaved(party_id, account_id, p.member[i].name);
				memset(&p.member[i], 0, sizeof(struct party_member));

				if( !p.isEmpty() )
				{	// まだ人がいるのでデータ送信
					// reorganize
					for(k=0; k<MAX_PARTY; ++k)
					{
						if(p.member[k].account_id == 0)
						{
							for(j=k+1; j<MAX_PARTY; ++j)
							{
								if(p.member[j].account_id != 0)
								{
									memmove(p.member+k,p.member+j, (MAX_PARTY-j)*sizeof(struct party_member));
									memset(p.member+k+MAX_PARTY-j, 0,       (j-k)*sizeof(struct party_member));
									break;
								}
							}
							if(j>=MAX_PARTY)
								break;
						}
					}
					if(leader_leave)
					{	// find a new leader
						p.member[0].leader=1;
					}
					mapif_party_info(-1, p);

					if( (p.itemshare||p.expshare) && !party_check_exp_share(p) )
					{	// disable the menu
						p.itemshare = p.expshare = 0;
						mapif_party_optionchanged(fd, p, 0, 0);
					}
					cPartyDB.saveParty(p);
				}
				else
				{
					cPartyDB.removeParty(p.party_id);
					broken=true;
				}

				break;
			}
		}
	}
	if(broken) mapif_party_broken(party_id,0);
	return 0;
}

// パーティマップ更新要求
int mapif_parse_PartyChangeMap(int fd, uint32 party_id, uint32 account_id, const char *mapname, int online, int lv)
{
	CParty p;
	size_t i;
	if( cPartyDB.searchParty(party_id, p) )
	{
		for(i=0; i<MAX_PARTY; ++i)
		{
			if (p.member[i].account_id == account_id)
			{
				safestrcpy(p.member[i].mapname, sizeof(p.member[i].mapname), mapname);
				char* ip=strchr(p.member[i].mapname,'.');
				if(ip) *ip=0;

				p.member[i].online = online;
				p.member[i].lv = lv;
				mapif_party_membermoved(p, i);

				if( (p.itemshare||p.expshare) && !party_check_exp_share(p) )
				{	// disable the menu
					p.itemshare = p.expshare = 0;
					mapif_party_optionchanged(fd, p, 0, 0);
				}
				cPartyDB.saveParty(p);
				break;
			}
		}
	}
	else
		mapif_party_broken(party_id,0);
	return 0;
}

// パーティ解散要求
int mapif_parse_BreakParty(int fd, uint32 party_id)
{
	cPartyDB.removeParty(party_id);
	mapif_party_broken(party_id,0);
	return 0;
}

// パーティメッセージ送信
int mapif_parse_PartyMessage(int fd, uint32 party_id, uint32 account_id, const char *mes, size_t len)
{
	return mapif_party_message(party_id, account_id, mes, len, fd);
}
// パーティチェック要求
int mapif_parse_PartyCheck(int fd, uint32 party_id, uint32 account_id, const char *nick)
{
	return party_check_conflict(party_id, account_id, nick);
}
// サーバーから脱退要求（キャラ削除用）
int inter_party_leave(uint32 party_id, uint32 account_id)
{
	return mapif_parse_PartyLeave(-1, party_id, account_id);
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_party_parse_frommap(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	switch(RFIFOW(fd,0)) {
	case 0x3020: mapif_parse_CreateParty(fd, RFIFOL(fd,2), (char*)RFIFOP(fd,6), (char*)RFIFOP(fd,30), (char*)RFIFOP(fd,54), RFIFOW(fd,70)); break;
	case 0x3021: mapif_parse_PartyInfo(fd, RFIFOL(fd,2)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd, RFIFOL(fd,2), RFIFOL(fd,6), (char*)RFIFOP(fd,10), (char*)RFIFOP(fd,34), RFIFOW(fd,50)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd, RFIFOL(fd,2), RFIFOL(fd,6), (char*)RFIFOP(fd,10), RFIFOB(fd,26), RFIFOW(fd,27)); break;
	case 0x3026: mapif_parse_BreakParty(fd, RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3028: mapif_parse_PartyCheck(fd, RFIFOL(fd,2), RFIFOL(fd,6), (char*)RFIFOP(fd,10)); break;
	default:
		return 0;
	}
	return 1;
}
