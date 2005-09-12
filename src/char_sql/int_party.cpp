//
// original code from athena
// SQL conversion by hack
//

#include "base.h"
#include "socket.h"
#include "utils.h"
#include "strlib.h"
#include "itemdb.h"
#include "inter.h"
#include "db.h"
#include "malloc.h"
#include "showmsg.h"

#include "char.h"

static int party_newid = 100;

int mapif_party_broken(uint32 party_id,int flag);


bool inter_party_tosql(uint32 party_id, struct party &p)
{	// 'party' ('party_id','name','exp','item','leader')
	char t_name[128], t_member[64];
	int party_member = 0;
	int party_exist = 0;
	size_t i = 0;
	int party_new = 0;
	uint32 leader_id = 0;

	ShowMessage("("CL_BT_BLUE"%ld"CL_NORM")    Request save party - ", (unsigned long)party_id);
	
	if(party_id == 0 || p.party_id == 0 || party_id != p.party_id) {
		ShowMessage("- party_id error \n");
		return false;
	}

	// Check if party exists
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id`='%ld'", party_db, (unsigned long)party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return false;
	}

	jstrescapecpy(t_name, p.name);
	sql_res = mysql_store_result(&mysql_handle);
	if( sql_res )
	{
		sql_row = mysql_fetch_row(sql_res);
		party_exist = atoi( sql_row[0] );
		if( !party_exist )
		{	// Add new party, if not exist
			for(i = 0; i < MAX_PARTY && ((p.member[i].account_id > 0 && p.member[i].leader == 0) || (p.member[i].account_id <= 0)); i++);
			if(i < MAX_PARTY)
				leader_id = p.member[i].account_id;
			
			sprintf(tmp_sql,"INSERT INTO `%s`  (`party_id`, `name`, `exp`, `item`, `leader_id`) VALUES ('%ld', '%s', '%d', '%d', '%ld')",
				party_db, (unsigned long)party_id, t_name, p.expshare, p.itemshare, (unsigned long)leader_id);
			if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
				ShowMessage("DB server Error (inset/update `party` 1)- %s\n", mysql_error(&mysql_handle) );
				return false;
			}
			
			sprintf(tmp_sql,"UPDATE `%s` SET `party_id`='%ld'  WHERE `account_id`='%ld' AND `name`='%s'",
				char_db, (unsigned long)party_id, (unsigned long)leader_id, jstrescapecpy(t_member,p.member[i].name));
			if(mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error (inset/update `party` 2)- %s\n", mysql_error(&mysql_handle) );

			ShowMessage("- Insert new party %ld  \n", (unsigned long)party_id);
			party_new = 1;
		}
	}
	mysql_free_result(sql_res);
	
	// Check members in party
	if( !party_new )
	{
		sprintf(tmp_sql,"SELECT count(*) FROM `%s` WHERE `party_id`='%ld'",char_db, (unsigned long)party_id); // TBR
		if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			return false;
		}
		sql_res = mysql_store_result(&mysql_handle);
	}
	
	if( sql_res != NULL && !party_new )
	{	
		sql_row = mysql_fetch_row(sql_res);
		party_member = atoi( sql_row[0] );
		mysql_free_result(sql_res);

		if( !party_member )
		{	// Delete the party, it has no member.
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `party_id`='%ld'",party_db, (unsigned long)party_id);
			if( mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			ShowMessage("No member in party %d, break it \n",party_id);
			memset(&p, 0, sizeof(struct party));
			return false;
		}
		else
		{	uint32 leader_id=0;
			uint32 lastaccount_id=0;
			char *tmp = tmp_sql;
			tmp_sql[0] = '\0';
			// Update party information, if exists
			for (i=0;i<MAX_PARTY;i++)
			{
				if( p.member[i].account_id >0 )
				{	//Sasuke- Updates Char db for correct party_id
					lastaccount_id = p.member[i].account_id;
					if(p.member[i].leader) leader_id=p.member[i].account_id;

					if(tmp_sql[0] == '\0')
					{
						tmp += sprintf(tmp_sql, "UPDATE `%s` SET `party_id`='%ld' WHERE (`account_id` = '%ld' AND `name` = '%s')",
									char_db, (unsigned long)party_id, (unsigned long)p.member[i].account_id, jstrescapecpy(t_member, p.member[i].name));
					}
					else
					{
						tmp += sprintf(tmp, " OR (`account_id` = '%ld' AND `name` = '%s')",
							(unsigned long)p.member[i].account_id, jstrescapecpy(t_member, p.member[i].name));
					}
				}
			}
			// there should be at least one valid account here
			if(leader_id==0) leader_id=lastaccount_id;
			
			if (tmp_sql[0] != '\0' && mysql_SendQuery(&mysql_handle, tmp_sql))
				ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
			
			//Sasuke- Updates Party db correct info
			sprintf(tmp_sql,"UPDATE `%s` SET `name`='%s', `exp`='%d', `item`='%d', `leader_id`='%ld' WHERE `party_id`='%ld'",
				party_db, t_name, p.expshare, p.itemshare, (unsigned long)leader_id, (unsigned long)party_id);
			if( mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error (insert/update `party` 3)- %s\n", mysql_error(&mysql_handle) );
			//ShowMessage("- Update party %d information \n",party_id);
		}
	}
	ShowMessage("Party save success\n");
	return true;
}

// Read party from mysql
bool inter_party_fromsql(int party_id, struct party &p)
{
	uint32 leader_id=0;
	//ShowMessage("("CL_BT_BLUE"%d"CL_NORM")    Request load party - ",party_id);

	memset(&p, 0, sizeof(struct party));

	sprintf(tmp_sql,"SELECT `name`,`exp`,`item`, `leader_id` FROM `%s` WHERE `party_id`='%d'",party_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return false;
	}

	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		sql_row = mysql_fetch_row(sql_res);
		ShowMessage("- Read party %d from MySQL\n",party_id);
		p.party_id = party_id;
		safestrcpy(p.name, sql_row[0], 24);
		p.expshare = atoi(sql_row[1]);
		p.itemshare = atoi(sql_row[2]);
		leader_id = atoi(sql_row[3]);
	} else {
		mysql_free_result(sql_res);
		ShowMessage("- Cannot find party %d \n",party_id);
		return false;
	}
	mysql_free_result(sql_res);

	// Load members
	sprintf(tmp_sql,"SELECT `account_id`, `name`,`base_level`,`last_map`,`online` FROM `%s` WHERE `party_id`='%d'",
		char_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return false;
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		int i;
		for (i = 0; (sql_row = mysql_fetch_row(sql_res)); i++) {
			struct party_member *m = &p.member[i];
			m->account_id = atoi(sql_row[0]);
			if (m->account_id == leader_id)
				m->leader = 1;
			else
				m->leader = 0;
			safestrcpy(m->name, sql_row[1], 24);
			m->lv = atoi(sql_row[2]);
			m->online = atoi(sql_row[4]);
			if(m->online)
				safestrcpy(m->map, sql_row[3], 24);
			else
				m->map[0]=0;
		}
		//ShowMessage("- %d members found in party %d \n",i,party_id);
	}
	mysql_free_result(sql_res);

	//ShowMessage("Party load success\n");
	return true;
}

int inter_party_sql_init()
{
	int i;

	sprintf (tmp_sql , "SELECT count(*) FROM `%s`", party_db);
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);
	ShowMessage("total party data -> '%s'.......\n",sql_row[0]);
	i = atoi (sql_row[0]);
	mysql_free_result(sql_res);

	if (i > 0) {
		//set party_newid
		sprintf (tmp_sql , "SELECT max(`party_id`) FROM `%s`", party_db);
		if(mysql_SendQuery(&mysql_handle, tmp_sql)) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}

		sql_res = mysql_store_result(&mysql_handle) ;

		sql_row = mysql_fetch_row(sql_res);
		party_newid = atoi (sql_row[0])+1;
		mysql_free_result(sql_res);
	}
	ShowMessage("set party_newid: %d.......\n",party_newid);
	return 0;
}

void inter_party_sql_final()
{
	return;
}

// Search for the party according to its name
bool search_partyname(const char *str)
{
	char t_name[64];
	uint32 party_id;

	sprintf(tmp_sql,"SELECT `party_id` FROM `%s` WHERE `name`='%s'", party_db, jstrescapecpy(t_name,str));
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if(sql_res==NULL || mysql_num_rows(sql_res)<=0) {
		mysql_free_result(sql_res); 
		return false;
	}
	sql_row = mysql_fetch_row(sql_res);
	party_id = atoi(sql_row[0]);
	mysql_free_result(sql_res);

	///////////// debug purpose only
	// Load members
	sprintf(tmp_sql,"SELECT `account_id`, `online` FROM `%s` WHERE `party_id`='%ld'",char_db, (unsigned long)party_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return false;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0)
	{
		size_t i, ocnt=0;
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++)
		{
			if( atoi(sql_row[2]) ) // online
				ocnt++;
		}
		ShowMessage("- %d members (%d online) found in party %d\n",i, ocnt, party_id);
	}
	mysql_free_result(sql_res);
	/////////////
	return true;
}

// EXP公平分配できるかチェック
bool party_check_exp_share(struct party &p)
{
	size_t i, cnt_lo=0, cnt_hi=0;
	size_t pl1=0,pl2=0,pl3=0;
	unsigned short maxlv = 0, minlv = 0xFFFF;
	
	for(i = 0; i < MAX_PARTY; i++)
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
		(!strcmp(p.member[0].map,p.member[1].map)) &&
		(!strcmp(p.member[1].map,p.member[2].map)) )
	{
		pl1=char_nick2id(p.member[0].name);
		pl2=char_nick2id(p.member[1].name);
		pl3=char_nick2id(p.member[2].name);
		ShowMessage("PARTY: group of 3 Id1 %d lv %d name %s Id2 %d lv %d name %s Id3 %d lv %d name %s\n",pl1,p.member[0].lv,p.member[0].name,pl2,p.member[1].lv,p.member[1].name,pl3,p.member[2].lv,p.member[2].name);
		if( (char_married(pl1,pl2) && char_child(pl1,pl3)) ||
			(char_married(pl1,pl3) && char_child(pl1,pl2)) ||
			(char_married(pl2,pl3) && char_child(pl2,pl1)) )
			return true;
	}
	return (maxlv==0) || (maxlv<=(minlv+party_share_level));
}

// Is there any member in the party?
bool party_isempty(struct party &p)
{
	int i;
	//ShowMessage("party check empty %ld\n",(unsigned long)p.party_id);
	if (p.party_id==0)
		return true;
	for(i=0;i<MAX_PARTY;i++){
		ShowMessage("%d acc=%d\n",i,p.member[i].account_id);
		if(p.member[i].account_id>0){
			return false;
		}
	}
	// If there is no member, then break the party
	mapif_party_broken(p.party_id, 0);
	inter_party_tosql(p.party_id, p);
	return true;
}


// Check if a member is in two party, not necessary :)
int party_check_conflict(int party_id,uint32 account_id,char *nick)
{
	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// パーティ作成可否
int mapif_party_created(int fd,uint32 account_id,struct party *p)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3820;
	WFIFOL(fd,2)=account_id;
	if(p!=NULL){
		WFIFOB(fd,6)=0;
		WFIFOL(fd,7)=p->party_id;
		memcpy(WFIFOP(fd,11),p->name,24);
		ShowMessage("int_party: created! %d %s\n",p->party_id,p->name);
	}else{
		WFIFOB(fd,6)=1;
		WFIFOL(fd,7)=0;
		memcpy(WFIFOP(fd,11),"error",6);
	}
	WFIFOSET(fd,35);

	return 0;
}

// パーティ情報見つからず
int mapif_party_noinfo(int fd,int party_id)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3821;
	WFIFOW(fd,2)=8;
	WFIFOL(fd,4)=party_id;
	WFIFOSET(fd,8);
	ShowError("int_party: info not found %d\n",party_id);
	return 0;
}
// パーティ情報まとめ送り
int mapif_party_info(int fd,struct party *p)
{
	if(p)
	{
		unsigned char buf[1024];
		WBUFW(buf,0)=0x3821;
		WBUFW(buf,2)=4+sizeof(struct party);
		party_tobuffer(*p,buf+4);
		if(fd<0)
			mapif_sendall(buf,4+sizeof(struct party));
		else
			mapif_send(fd,buf,4+sizeof(struct party));
		ShowMessage("mapif_party_info: info %d %s\n",p->party_id,p->name);
	}
	return 0;
}
// パーティメンバ追加可否
int mapif_party_memberadded(int fd,uint32 party_id,uint32 account_id,int flag)
{
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3822;
	WFIFOL(fd,2)=party_id;
	WFIFOL(fd,6)=account_id;
	WFIFOB(fd,10)=flag;
	WFIFOSET(fd,11);
	return 0;
}
// パーティ設定変更通知
int mapif_party_optionchanged(int fd,struct party *p,uint32 account_id, unsigned char flag)
{
	if(p)
	{
		unsigned char buf[16];
		WBUFW(buf,0)=0x3823;
		WBUFL(buf,2)=p->party_id;
		WBUFL(buf,6)=account_id;
		WBUFW(buf,10)=p->expshare;
		WBUFW(buf,12)=p->itemshare;
		WBUFB(buf,14)=flag;
		if(flag==0)
			mapif_sendall(buf,15);
		else
			mapif_send(fd,buf,15);
		ShowMessage("int_party: option changed %ld %ld %d %d %d\n",(unsigned long)p->party_id,(unsigned long)account_id,p->expshare,p->itemshare,flag);
	}
	return 0;
}
// パーティ脱退通知
int mapif_party_leaved(uint32 party_id,uint32 account_id,const char *name)
{
	unsigned char buf[64];
	WBUFW(buf,0)=0x3824;
	WBUFL(buf,2)=party_id;
	WBUFL(buf,6)=account_id;
	safestrcpy((char*)WBUFP(buf,10),name,24);
	mapif_sendall(buf,34);
	ShowMessage("int_party: party leaved %ld %ld %s\n",(unsigned long)party_id,(unsigned long)account_id,name);
	return 0;
}
// パーティマップ更新通知
int mapif_party_membermoved(struct party *p,int idx)
{
	if(p)
	{
		unsigned char buf[32];
		WBUFW(buf,0) = 0x3825;
		WBUFL(buf,2) = p->party_id;
		WBUFL(buf,6) = p->member[idx].account_id;
		memcpy(WBUFP(buf,10), p->member[idx].map, 16);
		WBUFB(buf,26) = p->member[idx].online;
		WBUFW(buf,27) = p->member[idx].lv;
		mapif_sendall(buf, 29);
	}
	return 0;
}
// パーティ解散通知
int mapif_party_broken(uint32 party_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3826;
	WBUFL(buf,2)=party_id;
	WBUFB(buf,6)=flag;
	mapif_sendall(buf,7);
	ShowMessage("int_party: broken %ld\n", (unsigned long)party_id);
	return 0;
}
// パーティ内発言
int mapif_party_message(uint32 party_id,uint32 account_id,char *mes,int len, int sfd)
{
	unsigned char buf[512];
	WBUFW(buf,0)=0x3827;
	WBUFW(buf,2)=len+12;
	WBUFL(buf,4)=party_id;
	WBUFL(buf,8)=account_id;
	memcpy(WBUFP(buf,12),mes,len);
	mapif_sendallwos(sfd, buf,len+12);
	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信


// Create Party
int mapif_parse_CreateParty(int fd,uint32 account_id,char *name,char *nick,char *map,int lv, int item, int item2)
{
	if( search_partyname(name) )
	{
		ShowMessage("int_party: same name party exists [%s]\n",name);
		mapif_party_created(fd,account_id,NULL);
	}
	else
	{
		struct party p;

		memset(&p,0,sizeof(struct party));

		p.party_id=party_newid++;
		safestrcpy(p.name,name,24);
		p.expshare=0;
		p.itemshare=item;
		//<item1>アイテム?集方法。0で個人別、1でパ?ティ公有
		//<item2>アイテム分配方法。0で個人別、1でパ?ティに均等分配
		p.itemc = 0;
		p.member[0].account_id=account_id;
		safestrcpy(p.member[0].name,nick,24);
		safestrcpy(p.member[0].map,map,24);
		p.member[0].leader=1;
		p.member[0].online=1;
		p.member[0].lv=lv;

		inter_party_tosql(p.party_id,p);

		mapif_party_created(fd,account_id, &p);
		mapif_party_info(fd, &p);
	}

	return 0;
}
// パーティ情報要求
int mapif_parse_PartyInfo(int fd,int party_id)
{
	struct party p;
	inter_party_fromsql(party_id, p);

	if(p.party_id >= 0)
		mapif_party_info(fd, &p);
	else
		mapif_party_noinfo(fd, party_id);
	return 0;
}
// パーティ追加要求
int mapif_parse_PartyAddMember(int fd,uint32 party_id,uint32 account_id,char *nick,char *map,int lv)
{
	struct party p;
	int i;

	inter_party_fromsql(party_id, p);

	if(p.party_id != party_id)
	{	// fail
		mapif_party_memberadded(fd, party_id, account_id, 1);
		return 0;
	}

	for(i=0;i<MAX_PARTY;i++)
	{
		if(p.member[i].account_id==0)
		{
			p.member[i].account_id=account_id;
			memcpy(p.member[i].name,nick,24);
			memcpy(p.member[i].map,map,24);
			p.member[i].leader=0;
			p.member[i].online=1;
			p.member[i].lv=lv;
			mapif_party_memberadded(fd, party_id, account_id, 0);
			mapif_party_info(-1, &p);

			if( (p.itemshare||p.expshare) && !party_check_exp_share(p) ) {
				// disable the menu
				p.itemshare = p.expshare = 0;
				mapif_party_optionchanged(fd, &p, 0, 0);
			}
			inter_party_tosql(party_id, p);
			return 0;
		}
	}
	mapif_party_memberadded(fd,party_id,account_id,1);
	return 0;
}
// パーティー設定変更要求
int mapif_parse_PartyChangeOption(int fd,uint32 party_id,uint32 account_id,unsigned short expshare,unsigned short itemshare)
{
	int flag=0;
	struct party p;
	inter_party_fromsql(party_id, p);

	if(p.party_id != party_id){
		return 0;
	}
	if( party_check_exp_share(p) )
	{
		p.itemshare = itemshare;
		p.expshare = expshare;
	}
	else
	{	// disable the menu
		p.itemshare = p.expshare =  0;
		flag = expshare||itemshare;
	}
	mapif_party_optionchanged(fd, &p, account_id, flag );
	inter_party_tosql(party_id, p);
	return 0;
}
// パーティ脱退要求
int mapif_parse_PartyLeave(int fd, uint32 party_id, uint32 account_id)
{
	char t_member[64];
	struct party p;

	inter_party_fromsql(party_id, p);
	if(p.party_id == party_id)
	{
		size_t i,k,j;
		for (i=0; i<MAX_PARTY; i++)
		{
			if (p.member[i].account_id == account_id)
			{
				bool leader_leave = (p.member[i].leader==1);

				// Update char information
				sprintf (tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld' AND `name`='%s'",
					char_db, (unsigned long)party_id, jstrescapecpy(t_member,p.member[i].name));
				if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
					ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
				}
				ShowMessage("Delete member %s from MySQL \n", p.member[i].name);

				mapif_party_leaved(party_id, account_id, p.member[i].name);

				memset(&p.member[i], 0, sizeof(struct party_member));
				if( !party_isempty(p) )
				{
					// reorganize
					for(k=0; k<MAX_PARTY; k++)
					{
						if(p.member[k].account_id == 0)
						{
							for(j=k+1; j<MAX_PARTY; j++)
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
					if( party_modus==1 )
					{	///////////////
						// leader shifting
						if(leader_leave)
						{	// find a new leader
							p.member[0].leader=1;
						}
						mapif_party_info(-1, &p);// まだ人がいるのでデータ送信
						inter_party_tosql(party_id, p);
						///////////////
					}
					else
					{	///////////////
						// leader breaking
						if(leader_leave)
						{
							for (j=0; j<MAX_PARTY; j++)
							{
								//ShowMessage("j = %d , p.member[j].account_id = %d , p.member[j].account_id = %d \n",j,p.member[j].account_id,p.member[j].account_id);
								if (p.member[j].account_id > 0 && j != i)
								{
									mapif_party_leaved(party_id, p.member[j].account_id, p.member[j].name);
									ShowMessage("Delete member %s from MySQL \n", p.member[j].name);
								}
							}
							// we'll skip name-checking and just reset everyone with the same party id [celest]
							sprintf (tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld'", char_db, (unsigned long)party_id);
							if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
								ShowError("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
							}
							// Delete the party, t has no member.
							sprintf(tmp_sql, "DELETE FROM `%s` WHERE `party_id`='%ld'", party_db, (unsigned long)party_id);
							if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
								ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
							}
							ShowMessage("Leader breaks party %d \n",party_id);

							mapif_party_broken(fd,party_id);
						}
					}
				}
				break;
			}
		}
	}
	else
	{
		sprintf(tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld' AND `account_id`='%ld' AND `online`='1'",
			char_db, (unsigned long)party_id, (unsigned long)account_id);
		if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
		}
	}
	return 0;
}
// When member goes to other map
int mapif_parse_PartyChangeMap(int fd,uint32 party_id,uint32 account_id,char *map,int online,int lv)
{
	struct party p;
	int i;

	inter_party_fromsql(party_id, p);

	if(p.party_id <= 0){
		return 0;
	}
	for(i=0;i<MAX_PARTY;i++){
		if(p.member[i].account_id==account_id)
		{
			safestrcpy(p.member[i].map,map,24);
			p.member[i].online=online;
			p.member[i].lv=lv;
			mapif_party_membermoved(&p,i);

			if( (p.itemshare||p.expshare) && !party_check_exp_share(p) ) {
				// disable the menu
				p.itemshare = p.expshare = 0;
				mapif_party_optionchanged(fd, &p,0,0);

			}
			break;
		}
	}
	inter_party_tosql(party_id, p);
	return 0;
}
// パーティ解散要求
int mapif_parse_BreakParty(int fd,uint32 party_id)
{
	struct party p;

	inter_party_fromsql(party_id, p);

	if(p.party_id == party_id)
	{
		inter_party_tosql(party_id,p);
		mapif_party_broken(fd,party_id);
	}
	return 0;
}
// パーティメッセージ送信
int mapif_parse_PartyMessage(int fd,uint32 party_id,uint32 account_id,char *mes,size_t len)
{
	return mapif_party_message(party_id,account_id,mes,len, fd);
}
// パーティチェック要求
int mapif_parse_PartyCheck(int fd,uint32 party_id,uint32 account_id,char *nick)
{
	return party_check_conflict(party_id,account_id,nick);
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

	switch(RFIFOW(fd,0)){
	case 0x3020: mapif_parse_CreateParty(fd,RFIFOL(fd,2),(char*)RFIFOP(fd,6),(char*)RFIFOP(fd,30),(char*)RFIFOP(fd,54),RFIFOW(fd,70), RFIFOB(fd,72), RFIFOB(fd,73)); break;
	case 0x3021: mapif_parse_PartyInfo(fd,RFIFOL(fd,2)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd,RFIFOL(fd,2),RFIFOL(fd,6),(char*)RFIFOP(fd,10),(char*)RFIFOP(fd,34),RFIFOW(fd,50)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd,RFIFOL(fd,2),RFIFOL(fd,6)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd,RFIFOL(fd,2),RFIFOL(fd,6),(char*)RFIFOP(fd,10),RFIFOB(fd,26),RFIFOW(fd,27)); break;
	case 0x3026: mapif_parse_BreakParty(fd,RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd,RFIFOL(fd,4),RFIFOL(fd,8),(char*)RFIFOP(fd,12),RFIFOW(fd,2)-12); break;
	case 0x3028: mapif_parse_PartyCheck(fd,RFIFOL(fd,2),RFIFOL(fd,6),(char*)RFIFOP(fd,10)); break;
	default:
		return 0;
	}
	return 1;
}

// サーバーから脱退要求（キャラ削除用）
int inter_party_leave(uint32 party_id,uint32 account_id)
{
	return mapif_parse_PartyLeave(-1,party_id,account_id);
}
