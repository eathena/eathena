// $Id: int_party.c,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#include "base.h"
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

char party_txt[1024] = "save/party.txt";

static struct dbt *party_db;
static unsigned long party_newid = 100;

int mapif_party_broken(unsigned long party_id, int flag);
bool party_isempty(struct party *p);
int mapif_parse_PartyLeave(int fd, unsigned long party_id, unsigned long account_id);

// パーティデータの文字列への変換
int inter_party_tostr(char *str, struct party *p) {
	int i, len;

	len = sprintf(str, "%ld\t%s\t%d,%d\t", p->party_id, p->name, p->expshare, p->itemshare);
	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		len += sprintf(str + len, "%ld,%ld\t%s\t", m->account_id, m->leader, ((m->account_id > 0) ? m->name : "NoMember"));
	}

	return 0;
}

// パーティデータの文字列からの変換
int inter_party_fromstr(char *str, struct party *p) {
	int i, j;
	int tmp_int[16];
	char tmp_str[256];

	memset(p, 0, sizeof(struct party));

//	ShowMessage("sscanf party main info\n");
	if (sscanf(str, "%d\t%255[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
		return 1;

	p->party_id = tmp_int[0];
	memcpy(p->name, tmp_str, 24);
	p->expshare = tmp_int[1];
	p->itemshare = tmp_int[2];
//	ShowMessage("%d [%s] %d %d\n", tmp_int[0], tmp_str[0], tmp_int[1], tmp_int[2]);

	for(j = 0; j < 3 && str != NULL; j++)
		str = strchr(str + 1, '\t');

	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		if (str == NULL)
			return 1;
//		ShowMessage("sscanf party member info %d\n", i);

		if (sscanf(str + 1, "%d,%d\t%255[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str) != 3)
			return 1;

		m->account_id = tmp_int[0];
		m->leader = tmp_int[1];
		memcpy(m->name, tmp_str, 24);
//		ShowMessage(" %d %d [%s]\n", tmp_int[0], tmp_int[1], tmp_str);

		for(j = 0; j < 2 && str != NULL; j++)
			str = strchr(str + 1, '\t');
	}

	return 0;
}

// パーティデータのロード
int inter_party_init() {
	char line[8192];
	struct party *p;
	FILE *fp;
	int c = 0;
	size_t i, j;

	party_db = numdb_init();

	if ((fp = savefopen(party_txt, "r")) == NULL)
		return 1;

	while(fgets(line, sizeof(line) - 1, fp)) {
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && party_newid <= i) {
			party_newid = i;
			continue;
		}

		p = (struct party*)aCalloc(1,sizeof(struct party));
		if (inter_party_fromstr(line, p) == 0 && p->party_id > 0) {
			if (p->party_id >= party_newid)
				party_newid = p->party_id + 1;
			numdb_insert(party_db, p->party_id, p);
			party_isempty(p);
		} else {
			ShowMessage("int_party: broken data [%s] line %d\n", party_txt, c + 1);
			aFree(p);
		}
		c++;
	}
	fclose(fp);
//	ShowMessage("int_party: %s read done (%d parties)\n", party_txt, c);

	return 0;
}

int party_db_final (void *k, void *data, va_list ap) {
	struct party *p = (struct party *) data;
	if (p) aFree(p);
	return 0;
}
void inter_party_final()
{
	numdb_final(party_db, party_db_final);
	return;
}

// パ?ティ?デ?タのセ?ブ用
int inter_party_save_sub(void *key, void *data, va_list ap) {
	char line[8192];
	FILE *fp;

	inter_party_tostr(line, (struct party *)data);
	fp = va_arg(ap, FILE *);
	fprintf(fp, "%s" RETCODE, line);

	return 0;
}

// パーティーデータのセーブ
int inter_party_save() {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(party_txt, &lock)) == NULL) {
		ShowMessage("int_party: cant write [%s] !!! data is lost !!!\n", party_txt);
		return 1;
	}
	numdb_foreach(party_db, inter_party_save_sub, fp);
//	fprintf(fp, "%d\t%%newid%%\n", party_newid);
	lock_fclose(fp,party_txt, &lock);
//	ShowMessage("int_party: %s saved.\n", party_txt);

	return 0;
}

// パーティ名検索用
int search_partyname_sub(void *key,void *data,va_list ap) {
	struct party *p = (struct party *)data,**dst;
	char *str;

	str = va_arg(ap, char *);
	dst = va_arg(ap, struct party **);
	if(dst && p && p->name && str && strcasecmp(p->name, str) == 0)
		*dst = p;

	return 0;
}

// パーティ名検索
struct party* search_partyname(char *str) {
	struct party *p = NULL;
	numdb_foreach(party_db, search_partyname_sub, str, &p);

	return p;
}

// EXP公平分配できるかチェック
bool party_check_exp_share(struct party *p)
{
	size_t i, cnt_lo=0, cnt_hi=0;
	size_t pl1=0,pl2=0,pl3=0;
	unsigned short maxlv = 0, minlv = 0xFFFF;

	for(i = 0; i < MAX_PARTY; i++)
	{
		unsigned short lv = p->member[i].lv;
		if(lv && p->member[i].account_id && p->member[i].online)
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
		(0==strcmp(p->member[0].map,p->member[1].map)) &&
		(0==strcmp(p->member[1].map,p->member[2].map)) )
	{
		pl1=search_character_index(p->member[0].name);
		pl2=search_character_index(p->member[1].name);
		pl3=search_character_index(p->member[2].name);
		//ShowMessage("PARTY: group of 3 Id1 %d lv %d name %s Id2 %d lv %d name %s Id3 %d lv %d name %s\n",pl1,p->member[0].lv,p->member[0].name,pl2,p->member[1].lv,p->member[1].name,pl3,p->member[2].lv,p->member[2].name);
		if( (char_married(pl1,pl2) && char_child(pl1,pl3)) ||
			(char_married(pl1,pl3) && char_child(pl1,pl2)) ||
			(char_married(pl2,pl3) && char_child(pl2,pl1)) )
			return true;
	}
	return (maxlv==0) || (maxlv<=(minlv+party_share_level));
}

// パーティが空かどうかチェック
bool party_isempty(struct party *p) 
{
	if(p)
	{
		int i;
		//ShowMessage("party check empty %08X\n", (int)p);
		for(i = 0; i < MAX_PARTY; i++) {
			//ShowMessage("%d acc=%d\n", i, p->member[i].account_id);
			if (p->member[i].account_id > 0) {
				return false;
			}
		}
			// 誰もいないので解散
		mapif_party_broken(p->party_id, 0);
		numdb_erase(party_db, p->party_id);
		aFree(p);
	}
	return true;
}

// キャラの競合がないかチェック用
int party_check_conflict_sub(void *key, void *data, va_list ap)
{
	struct party *p = (struct party *)data;
	unsigned long party_id, account_id;
	size_t i;
	char *nick;

	party_id=va_arg(ap, int);
	account_id=va_arg(ap, int);
	nick=va_arg(ap, char *);

	if (p->party_id == party_id)	// 本来の所属なので問題なし
		return 0;

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == account_id && strcmp(p->member[i].name, nick) == 0) {
			// 別のパーティに偽の所属データがあるので脱退
			ShowMessage("int_party: party conflict! %d %d %d\n", account_id, party_id, p->party_id);
			mapif_parse_PartyLeave(-1, p->party_id, account_id);
		}
	}

	return 0;
}

// キャラの競合がないかチェック
int party_check_conflict(unsigned long party_id, unsigned long account_id, char *nick) {
	numdb_foreach(party_db, party_check_conflict_sub, party_id, account_id, nick);

	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// パーティ作成可否
int mapif_party_created(int fd,unsigned long account_id, struct party *p)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x3820;
	WFIFOL(fd,2) = account_id;
	if (p != NULL) {
		WFIFOB(fd,6) = 0;
		WFIFOL(fd,7) = p->party_id;
		memcpy(WFIFOP(fd,11), p->name, 24);
		ShowMessage("int_party: created! %d %s\n", p->party_id, p->name);
	} else {
		WFIFOB(fd,6) = 1;
		WFIFOL(fd,7) = 0;
		memcpy(WFIFOP(fd,11), "error", 24);
	}
	WFIFOSET(fd,35);

	return 0;
}

// パーティ情報見つからず
int mapif_party_noinfo(int fd, int party_id)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x3821;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = party_id;
	WFIFOSET(fd,8);
	ShowMessage("int_party: info not found %d\n", party_id);

	return 0;
}

// パーティ情報まとめ送り
int mapif_party_info(int fd, struct party *pparty) {
	unsigned char buf[2048];

	if(pparty)
	{
		WBUFW(buf,0) = 0x3821;
		WBUFW(buf,2) = 4 + sizeof(struct party);
		party_tobuffer(*pparty, buf+4);

		if( !session_isActive(fd) )
			mapif_sendall(buf, WBUFW(buf,2));
		else
			mapif_send(fd, buf, WBUFW(buf,2));
		//ShowMessage("int_party: info %d %s\n", p->party_id, p->name);
	}

	return 0;
}

// パーティメンバ追加可否
int mapif_party_memberadded(int fd, unsigned long party_id, unsigned long account_id, int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0) = 0x3822;
	WFIFOL(fd,2) = party_id;
	WFIFOL(fd,6) = account_id;
	WFIFOB(fd,10) = flag;
	WFIFOSET(fd,11);

	return 0;
}

// パーティ設定変更通知
int mapif_party_optionchanged(int fd,struct party *p, unsigned long account_id, unsigned char flag)
{
	unsigned char buf[15];

	WBUFW(buf,0) = 0x3823;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = account_id;
	WBUFW(buf,10) = p->expshare;
	WBUFW(buf,12) = p->itemshare;
	WBUFB(buf,14) = flag;
	if (flag == 0)
		mapif_sendall(buf, 15);
	else
		mapif_send(fd, buf, 15);
	ShowMessage("int_party: option changed %d %d %d %d %d\n", p->party_id, account_id, p->expshare, p->itemshare, flag);

	return 0;
}

// パーティ脱退通知
int mapif_party_leaved(unsigned long party_id,unsigned long account_id, char *name) {
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
int mapif_party_membermoved(struct party *p, int idx) {
	unsigned char buf[29];

	WBUFW(buf,0) = 0x3825;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = p->member[idx].account_id;
	memcpy(WBUFP(buf,10), p->member[idx].map, 16);
	WBUFB(buf,26) = p->member[idx].online;
	WBUFW(buf,27) = p->member[idx].lv;
	mapif_sendall(buf, 29);

	return 0;
}

// パーティ解散通知
int mapif_party_broken(unsigned long party_id, int flag) {
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3826;
	WBUFL(buf,2) = party_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	ShowMessage("int_party: broken %d\n", party_id);

	return 0;
}

// パーティ内発言
int mapif_party_message(unsigned long party_id, unsigned long account_id, char *mes, size_t len, int sfd) {
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
int mapif_parse_CreateParty(int fd, unsigned long account_id, char *name, char *nick, char *map, int lv) {
	struct party *p;
	char *ip;

	if(NULL==name)
		return 0;
	for(ip=name; *ip; ip++) {
		if ( *((unsigned char*)ip)==0xe0 || *ip== 0x7f) {
			ShowMessage("int_party: illegal party name [%s]\n", name);
			mapif_party_created(fd, account_id, NULL);
			return 0;
		}
	}

	if ((p = search_partyname(name)) != NULL) {
		ShowMessage("int_party: same name party exists [%s]\n", name);
		mapif_party_created(fd, account_id, NULL);
		return 0;
	}
	p = (struct party*)aCalloc(1,sizeof(struct party));
	p->party_id = party_newid++;
	memcpy(p->name, name, 24);
	p->expshare = 0;
	p->itemshare = 0;
	p->member[0].account_id = account_id;
	memcpy(p->member[0].name, nick, 24);
	memcpy(p->member[0].map, map, 24);
	p->member[0].leader = 1;
	p->member[0].online = 1;
	p->member[0].lv = lv;

	numdb_insert(party_db, p->party_id, p);

	mapif_party_created(fd, account_id, p);
	mapif_party_info(fd, p);

	return 0;
}

// パーティ情報要求
int mapif_parse_PartyInfo(int fd, int party_id) {
	struct party *p;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p != NULL)
		mapif_party_info(fd, p);
	else
		mapif_party_noinfo(fd, party_id);

	return 0;
}

// パーティ追加要求
int mapif_parse_PartyAddMember(int fd, unsigned long party_id, unsigned long account_id, char *nick, char *map, int lv)
{
	struct party *p;
	int i;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p == NULL) {
		mapif_party_memberadded(fd, party_id, account_id, 1);
		return 0;
	}

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == 0) {
			p->member[i].account_id = account_id;
			memcpy(p->member[i].name, nick, 24);
			memcpy(p->member[i].map, map, 24);
			p->member[i].leader = 0;
			p->member[i].online = 1;
			p->member[i].lv = lv;
			mapif_party_memberadded(fd, party_id, account_id, 0);
			mapif_party_info(-1, p);

			if( (p->itemshare||p->expshare) && !party_check_exp_share(p) ) {
				// disable the menu
				p->itemshare = p->expshare = 0;
				mapif_party_optionchanged(fd, p, 0, 0);
			}
			return 0;
		}
	}
	mapif_party_memberadded(fd, party_id, account_id, 1);

	return 0;
}

// パーティー設定変更要求
int mapif_parse_PartyChangeOption(int fd, unsigned long party_id, unsigned long account_id, unsigned short expshare, unsigned short itemshare)
{
	struct party *p;
	int flag = 0;
	unsigned short esold, isold;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	esold = p->expshare;
	isold = p->itemshare;

	if( party_check_exp_share(p) )
	{
		p->itemshare = itemshare;
		p->expshare = expshare;
	}
	else
	{	// disable
		p->itemshare = p->expshare =  0;
		flag = 1;
	}
	mapif_party_optionchanged(fd, p, account_id, flag );
	return 0;
}

// パーティ脱退要求
int mapif_parse_PartyLeave(int fd, unsigned long party_id, unsigned long account_id)
{
	struct party *p;
	size_t i,j,k;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p != NULL)
	{
		for(i = 0; i < MAX_PARTY; i++)
		{
			if(p->member[i].account_id == account_id)
			{	
				bool leader_leave = (p->member[i].leader==1);

				mapif_party_leaved(party_id, account_id, p->member[i].name);

				memset(&p->member[i], 0, sizeof(struct party_member));

				if( !party_isempty(p) )
				{	// まだ人がいるのでデータ送信
					// reorganize
					for(k=0; k<MAX_PARTY; k++)
					{
						if(p->member[k].account_id == 0)
						{
							for(j=k+1; j<MAX_PARTY; j++)
							{
								if(p->member[j].account_id != 0)
								{
									memmove(p->member+k,p->member+j, (MAX_PARTY-j)*sizeof(struct party_member));
									memset(p->member+k+MAX_PARTY-j, 0,       (j-k)*sizeof(struct party_member));
									break;
								}
							}
							if(j>=MAX_PARTY)
								break;
						}
					}
					if(leader_leave)
					{	// find a new leader
						p->member[0].leader=1;
					}
					mapif_party_info(-1, p);
				}
				return 0;
			}
		}
	}
	return 0;
}

// パーティマップ更新要求
int mapif_parse_PartyChangeMap(int fd, unsigned long party_id, unsigned long account_id, char *map, int online, int lv) {
	struct party *p;
	int i;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == account_id) {
			int flag = 0;

			memcpy(p->member[i].map, map, 24);
			p->member[i].online = online;
			p->member[i].lv = lv;
			mapif_party_membermoved(p, i);

			if( (p->itemshare||p->expshare) && !party_check_exp_share(p) ) {
				// disable the menu
				p->itemshare = p->expshare = 0;
				mapif_party_optionchanged(fd, p, 0, 0);
			}
			break;
		}
	}

	return 0;
}

// パーティ解散要求
int mapif_parse_BreakParty(int fd, int party_id) {
	struct party *p;

	p = (struct party *) numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	numdb_erase(party_db, party_id);
	mapif_party_broken(fd, party_id);

	return 0;
}

// パーティメッセージ送信
int mapif_parse_PartyMessage(int fd, unsigned long party_id, unsigned long account_id, char *mes, size_t len) {
	return mapif_party_message(party_id, account_id, mes, len, fd);
}
// パーティチェック要求
int mapif_parse_PartyCheck(int fd, unsigned long party_id, unsigned long account_id, char *nick) {
	return party_check_conflict(party_id, account_id, nick);
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

// サーバーから脱退要求（キャラ削除用）
int inter_party_leave(unsigned long party_id, unsigned long account_id) {
	return mapif_parse_PartyLeave(-1, party_id, account_id);
}

