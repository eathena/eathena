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

static struct party *party_pt;
static int party_newid = 100;

int mapif_party_broken(unsigned long party_id,int flag);
int party_check_empty(struct party *p);
int mapif_parse_PartyLeave(int fd,unsigned long party_id,unsigned long account_id);

/*
// Save party to mysql
int inter_party_tosql(unsigned long party_id,struct party *p)
{
	// 'party' ('party_id','name','exp','item','leader')
	char t_name[100], t_member[24];
	int party_member = 0;
	int party_online_member = 0;
	int party_exist = 0;
	unsigned long leader_id = 0;
	int i;

	ShowMessage("("CL_BT_BLUE"%d"CL_NORM")    Request save party - ",party_id);
	jstrescapecpy(t_name, p->name);

	if (p == NULL || party_id == 0 || p->party_id == 0 || party_id != p->party_id) {
		ShowMessage("- Party pointer or party_id error \n");
		return 0;
	}

	// Check if party exists
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id`='%d'", party_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		sql_row = mysql_fetch_row(sql_res);
		party_exist =  atoi (sql_row[0]);
		//ShowMessage("- Check if party %d exists : %s\n",party_id,party_exist==0?"No":"Yes");
	}
	mysql_free_result(sql_res) ; //resource free

	if (party_exist > 0) {
		// Check members in party
		sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id`='%d'", char_db, party_id); // TBR
		if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			return 0;
		}
		sql_res = mysql_store_result(&mysql_handle) ;
		if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
			sql_row = mysql_fetch_row(sql_res);
			party_member =  atoi (sql_row[0]);
		//	ShowMessage("- Check members in party %d : %d \n",party_id,party_member);
		}
		mysql_free_result(sql_res) ; //resource free

		for (i = 0; i < MAX_PARTY; i++)
			if (p->member[i].account_id > 0)
				party_online_member++;

		//if (party_online_member == 0)
		//	ShowMessage("- No member online \n"); 
		//else 
		//	ShowMessage("- Some member %d online \n", party_online_member);

		if (party_member <= 0 && party_online_member == 0) {
			// Delete the party, if has no member.
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `party_id`='%d'", party_db, party_id);
			if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
				ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			}
		//	ShowMessage("No member in party %d, break it \n",party_id);
			memset(p, 0, sizeof(struct party));
			return 0;
		} else {
			char *tmp = tmp_sql;
			tmp_sql[0] = '\0';
			// Update party information, if exists
			for (i = 0; i < MAX_PARTY; i++) {
				if (p->member[i].account_id > 0){
					if (tmp_sql[0] == '\0')
						tmp += sprintf(tmp_sql, "UPDATE `%s` SET `party_id`='%d' WHERE (`account_id` = '%d' AND `name` = '%s')",
							char_db, party_id, p->member[i].account_id, jstrescapecpy(t_member, p->member[i].name));
					else
						tmp += sprintf(tmp, " OR (`account_id` = '%d' AND `name` = '%s')",
							p->member[i].account_id, jstrescapecpy(t_member, p->member[i].name));
				}
			}
			//ShowError("%s",tmp_sql);
			if (tmp_sql[0] != '\0' && mysql_SendQuery(&mysql_handle, tmp_sql)) {
						ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
			}

			sprintf(tmp_sql, "UPDATE `%s` SET `name`='%s', `exp`='%d', `item`='%d' WHERE `party_id`='%d'",
				party_db, t_name, p->exp, p->item, party_id);
			if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
				ShowMessage("DB server Error (inset/update `party`)- %s\n", mysql_error(&mysql_handle) );
			}


		//	ShowMessage("- Update party %d information \n",party_id);
		}
	} else {
		// Add new party, if not exist
		for (i = 0; i < MAX_PARTY && ((p->member[i].account_id > 0 && p->member[i].leader == 0) || (p->member[i].account_id < 0)); i++)
			;
		if (i < MAX_PARTY)
			leader_id = p->member[i].account_id;
		sprintf(tmp_sql, "INSERT INTO `%s`  (`party_id`, `name`, `exp`, `item`, `leader_id`) VALUES ('%d', '%s', '%d', '%d', '%d')",
			party_db, party_id, t_name, p->exp, p->item, leader_id);
		if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
			ShowMessage("DB server Error (inset/update `party`)- %s\n", mysql_error(&mysql_handle) );
			return 0;
		}

		sprintf(tmp_sql,"UPDATE `%s` SET `party_id`='%d'  WHERE `account_id`='%d' AND `name`='%s'",
			char_db, party_id,leader_id, jstrescapecpy(t_member, p->member[i].name));
		if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
			ShowMessage("DB server Error (inset/update `party`)- %s\n", mysql_error(&mysql_handle) );
		}
		//ShowMessage("- Insert new party %d  \n",party_id);
	}

	ShowMessage("Party save success\n");
	return 0;
}
*/
int inter_party_tosql(unsigned long party_id,struct party *p)
{	// 'party' ('party_id','name','exp','item','leader')
	char t_name[100], t_member[24];
	int party_member = 0;
	int party_exist = 0;
	size_t i = 0;
	int party_new = 0;
//	int party_online_member = 0;
	unsigned long leader_id = 0;

	ShowMessage("("CL_BT_BLUE"%d"CL_NORM")    Request save party - ",party_id);
	
	jstrescapecpy(t_name, p->name);
	if (p == NULL || party_id == 0 || p->party_id == 0 || party_id != p->party_id) {
		ShowMessage("- Party pointer or party_id error \n");
		return 0;
	}
	
	// Check if party exists
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id`='%ld'", party_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	
	sql_res = mysql_store_result(&mysql_handle);
	if( sql_res ) {
		sql_row = mysql_fetch_row(sql_res);
		party_exist = atoi( sql_row[0] );
		if( !party_exist )
		{	// Add new party, if not exist
			for (i = 0; i < MAX_PARTY && ((p->member[i].account_id > 0 && p->member[i].leader == 0) || (p->member[i].account_id < 0)); i++);
			if (i < MAX_PARTY)
				leader_id = p->member[i].account_id;
			
			sprintf(tmp_sql,"INSERT INTO `%s`  (`party_id`, `name`, `exp`, `item`, `leader_id`) VALUES ('%ld', '%s', '%d', '%d', '%ld')",
				party_db, party_id, t_name, p->exp, p->item, leader_id);
			if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
				ShowMessage("DB server Error (inset/update `party`)- %s\n", mysql_error(&mysql_handle) );
				return 0;
			}
			
			sprintf(tmp_sql,"UPDATE `%s` SET `party_id`='%ld'  WHERE `account_id`='%ld' AND `name`='%s'",
				char_db, party_id, leader_id, jstrescapecpy(t_member,p->member[i].name));
			
			if(mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error (inset/update `party`)- %s\n", mysql_error(&mysql_handle) );
			ShowMessage("- Insert new party %d  \n",party_id);
			party_new = 1;
		}
	}
	mysql_free_result(sql_res);
	
	// Check members in party
	if( !party_new ) {
		sprintf(tmp_sql,"SELECT count(*) FROM `%s` WHERE `party_id`='%ld'",char_db, party_id); // TBR
		if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			return 0;
		}
		sql_res = mysql_store_result(&mysql_handle);
	}
	
	if( sql_res != NULL && !party_new ) {// Delete the party, if has no member.
		sql_row = mysql_fetch_row(sql_res);
		party_member = atoi( sql_row[0] );
		if( !party_member ) {
			mysql_free_result(sql_res);
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `party_id`='%ld'",party_db, party_id);
			if(mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
			ShowMessage("No member in party %d, break it \n",party_id);
			memset(p, 0, sizeof(struct party));
			return 0;
		}
		else
		{
			mysql_free_result(sql_res);
			char *tmp = tmp_sql;
			tmp_sql[0] = '\0';
			// Update party information, if exists
			
			for (i=0;i<MAX_PARTY;i++){
				if (p->member[i].account_id >0)
				{	//Sasuke- Updates Char db for correct party_id
					if (tmp_sql[0] == '\0')
					{
						tmp += sprintf(tmp_sql, "UPDATE `%s` SET `party_id`='%ld' WHERE (`account_id` = '%ld' AND `name` = '%s')",
									char_db, party_id, p->member[i].account_id, jstrescapecpy(t_member, p->member[i].name));
					}
					else
					{
						tmp += sprintf(tmp, " OR (`account_id` = '%ld' AND `name` = '%s')",
							p->member[i].account_id, jstrescapecpy(t_member, p->member[i].name));
					}
				}
			}
			
			if (tmp_sql[0] != '\0' && mysql_SendQuery(&mysql_handle, tmp_sql))
				ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
			
			//Sasuke- Updates Party db correct info
			
			sprintf(tmp_sql,"UPDATE `%s` SET `name`='%s', `exp`='%d', `item`='%d', `leader_id`=`leader_id` WHERE `party_id`='%ld'",
				party_db, t_name,p->exp,p->item,party_id);
			if(mysql_SendQuery(&mysql_handle, tmp_sql) )
				ShowMessage("DB server Error (insert/update `party`)- %s\n", mysql_error(&mysql_handle) );
			ShowMessage("- Update party %d information \n",party_id);
		}
	}
	ShowMessage("Party save success\n");
	return 0;
}

// Read party from mysql
int inter_party_fromsql(int party_id, struct party *p)
{
	unsigned long leader_id=0;
	ShowMessage("("CL_BT_BLUE"%d"CL_NORM")    Request load party - ",party_id);

	memset(p, 0, sizeof(struct party));

	sprintf(tmp_sql,"SELECT `party_id`, `name`,`exp`,`item`, `leader_id` FROM `%s` WHERE `party_id`='%d'",party_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}

	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		sql_row = mysql_fetch_row(sql_res);
	//	ShowMessage("- Read party %d from MySQL\n",party_id);
		p->party_id = party_id;
		strcpy(p->name, sql_row[1]);
		p->exp = atoi(sql_row[2]);
		p->item = atoi(sql_row[3]);
		leader_id = atoi(sql_row[4]);
	} else {
		mysql_free_result(sql_res);
	//	ShowMessage("- Cannot find party %d \n",party_id);
		return 0;
	}
	mysql_free_result(sql_res);

	// Load members
	sprintf(tmp_sql,"SELECT `account_id`, `name`,`base_level`,`last_map`,`online` FROM `%s` WHERE `party_id`='%d'",
		char_db, party_id); // TBR
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		int i;
		for (i = 0; (sql_row = mysql_fetch_row(sql_res)); i++) {
			struct party_member *m = &p->member[i];
			m->account_id = atoi(sql_row[0]);
			if (m->account_id == leader_id)
				m->leader = 1;
			else
				m->leader = 0;
			strncpy(m->name, sql_row[1], sizeof(m->name));
			m->lv = atoi(sql_row[2]);
			strncpy(m->map, sql_row[3], sizeof(m->map));
			m->online = atoi(sql_row[4]);
		}
	//	ShowMessage("- %d members found in party %d \n",i,party_id);
	}
	mysql_free_result(sql_res);

	ShowMessage("Party load success\n");
	return 0;
}

int inter_party_sql_init(){
	int i;

	//memory alloc
	ShowMessage("interserver party memory initialize.... (%d byte)\n",sizeof(struct party));
	party_pt = (struct party*)aCalloc(sizeof(struct party), 1);

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
	if (party_pt) aFree(party_pt);
	return;
}

// Search for the party according to its name
struct party* search_partyname(char *str)
{
	struct party *p=NULL;
	unsigned long leader_id = 0;
	char t_name[24];

	sprintf(tmp_sql,"SELECT `party_id`, `name`,`exp`,`item`,`leader_id` FROM `%s` WHERE `name`='%s'",party_db, jstrescapecpy(t_name,str));
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res==NULL || mysql_num_rows(sql_res)<=0) { mysql_free_result(sql_res); return p; }
	sql_row = mysql_fetch_row(sql_res);
	p = party_pt;
	p->party_id = atoi(sql_row[0]);
	strcpy(p->name, sql_row[1]);
	p->exp = atoi(sql_row[2]);
	p->item = atoi(sql_row[3]);
	leader_id = atoi(sql_row[4]);
	mysql_free_result(sql_res);

	// Load members
	sprintf(tmp_sql,"SELECT `account_id`, `name`,`base_level`,`last_map`,`online` FROM `%s` WHERE `party_id`='%ld'",char_db, p->party_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (select `party`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			struct party_member *m = &p->member[i];
			m->account_id = atoi(sql_row[0]);
			if (m->account_id == leader_id) m->leader = 1; else m->leader = 0;
			strncpy(m->name,sql_row[1],sizeof(m->name));
			m->lv = atoi(sql_row[2]);
			strncpy(m->map,sql_row[3],sizeof(m->map));
			m->online = atoi(sql_row[4]);
		}
		ShowMessage("- %d members found in party %d \n",i,p->party_id);
	}
	mysql_free_result(sql_res);

	return p;
}

// EXP公平分配できるかチェック
bool party_check_exp_share(struct party *p)
{
	size_t i, cnt_lo=0, cnt_hi=0;
	size_t pl1=0,pl2=0,pl3=0;
	unsigned short maxlv = 0, minlv = 0xFFFF;
	
	for(i=0;i<MAX_PARTY;i++)
	{
		unsigned short lv = p->member[i].lv;
		if (p->member[i].online)
		{
			if( lv < minlv ) minlv=lv;
			else if (maxlv < lv)	maxlv = lv;
			cnt_lo++;
			if( lv >= 70 )			cnt_hi++;
		}
	}
	// check for party with parents with child
	if( (cnt_hi >= 2) && (cnt_lo == 3) &&
		(!strcmp(p->member[0].map,p->member[1].map)) &&
		(!strcmp(p->member[1].map,p->member[2].map)) )
	{
		pl1=char_nick2id(p->member[0].name);
		pl2=char_nick2id(p->member[1].name);
		pl3=char_nick2id(p->member[2].name);
		ShowMessage("PARTY: group of 3 Id1 %d lv %d name %s Id2 %d lv %d name %s Id3 %d lv %d name %s\n",pl1,p->member[0].lv,p->member[0].name,pl2,p->member[1].lv,p->member[1].name,pl3,p->member[2].lv,p->member[2].name);
		if (char_married(pl1,pl2) && char_child(pl1,pl3))
			return true;
		if (char_married(pl1,pl3) && char_child(pl1,pl2))
			return true;
		if (char_married(pl2,pl3) && char_child(pl2,pl1))
			return true;
	}
	return (maxlv==0 || maxlv<=minlv+party_share_level);
}

// Is there any member in the party?
int party_check_empty(struct party *p)
{
	int i;
	if (p==NULL||p->party_id==0) return 1;
//	ShowMessage("party check empty %08X\n",(int)p);
	for(i=0;i<MAX_PARTY;i++){
//		ShowMessage("%d acc=%d\n",i,p->member[i].account_id);
		if(p->member[i].account_id>0){
			return 0;
		}
	}
	// If there is no member, then break the party
	mapif_party_broken(p->party_id,0);
	inter_party_tosql(p->party_id,p);
	return 1;
}


// Check if a member is in two party, not necessary :)
int party_check_conflict(int party_id,unsigned long account_id,char *nick)
{
	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// パーティ作成可否
int mapif_party_created(int fd,unsigned long account_id,struct party *p)
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
		memcpy(WFIFOP(fd,11),"error",24);
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
	ShowMessage("int_party: info not found %d\n",party_id);
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
		//ShowMessage("int_party: info %d %s\n",p->party_id,p->name);
	}
	return 0;
}
// パーティメンバ追加可否
int mapif_party_memberadded(int fd,unsigned long party_id,unsigned long account_id,int flag)
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
int mapif_party_optionchanged(int fd,struct party *p,unsigned long account_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3823;
	WBUFL(buf,2)=p->party_id;
	WBUFL(buf,6)=account_id;
	WBUFW(buf,10)=p->exp;
	WBUFW(buf,12)=p->item;
	WBUFB(buf,14)=flag;
	if(flag==0)
		mapif_sendall(buf,15);
	else
		mapif_send(fd,buf,15);
	//ShowMessage("int_party: option changed %d %d %d %d %d\n",p->party_id,account_id,p->exp,p->item,flag);
	return 0;
}
// パーティ脱退通知
int mapif_party_leaved(unsigned long party_id,unsigned long account_id,char *name)
{
	unsigned char buf[64];
	WBUFW(buf,0)=0x3824;
	WBUFL(buf,2)=party_id;
	WBUFL(buf,6)=account_id;
	memcpy(WBUFP(buf,10),name,24);
	mapif_sendall(buf,34);
	//ShowMessage("int_party: party leaved %d %d %s\n",party_id,account_id,name);
	return 0;
}
// パーティマップ更新通知
int mapif_party_membermoved(struct party *p,int idx)
{
	unsigned char buf[32];
	WBUFW(buf,0)=0x3825;
	WBUFL(buf,2)=p->party_id;
	WBUFL(buf,6)=p->member[idx].account_id;
	memcpy(WBUFP(buf,10),p->member[idx].map,16);
	WBUFB(buf,26)=p->member[idx].online;
	WBUFW(buf,27)=p->member[idx].lv;
	mapif_sendall(buf,29);
	return 0;
}
// パーティ解散通知
int mapif_party_broken(unsigned long party_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3826;
	WBUFL(buf,2)=party_id;
	WBUFB(buf,6)=flag;
	mapif_sendall(buf,7);
	//ShowMessage("int_party: broken %d\n",party_id);
	return 0;
}
// パーティ内発言
int mapif_party_message(unsigned long party_id,unsigned long account_id,char *mes,int len, int sfd)
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
int mapif_parse_CreateParty(int fd,unsigned long account_id,char *name,char *nick,char *map,int lv, int item, int item2)
{
	struct party *p;
	if( (p=search_partyname(name))!=NULL){
//		ShowMessage("int_party: same name party exists [%s]\n",name);
		mapif_party_created(fd,account_id,NULL);
		return 0;
	}
	p=party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		mapif_party_created(fd,account_id,NULL);
		return 0;
	}
	memset(p,0,sizeof(struct party));
	p->party_id=party_newid++;
	memcpy(p->name,name,24);
	p->exp=0;
	p->item=item;
	//<item1>アイテム?集方法。0で個人別、1でパ?ティ公有
	//<item2>アイテム分配方法。0で個人別、1でパ?ティに均等分配
	//difference between "collection" and "distribution" is...? ^^;
	p->itemc = 0;

	p->member[0].account_id=account_id;
	memcpy(p->member[0].name,nick,24);
	memcpy(p->member[0].map,map,16);
	p->member[0].leader=1;
	p->member[0].online=1;
	p->member[0].lv=lv;

	inter_party_tosql(p->party_id,p);

	mapif_party_created(fd,account_id,p);
	mapif_party_info(fd,p);

	return 0;
}
// パーティ情報要求
int mapif_parse_PartyInfo(int fd,int party_id)
{
	struct party *p = party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}
	inter_party_fromsql(party_id, p);

	if(p->party_id >= 0)
		mapif_party_info(fd,p);
	else
		mapif_party_noinfo(fd,party_id);
	return 0;
}
// パーティ追加要求
int mapif_parse_PartyAddMember(int fd,unsigned long party_id,unsigned long account_id,char *nick,char *map,int lv)
{
	struct party *p;
	int i;

	p = party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}
	inter_party_fromsql(party_id, p);

	if(p->party_id <= 0){
		mapif_party_memberadded(fd,party_id,account_id,1);
		return 0;
	}

	for(i=0;i<MAX_PARTY;i++){
		if(p->member[i].account_id==0){
			int flag=0;

			p->member[i].account_id=account_id;
			memcpy(p->member[i].name,nick,24);
			memcpy(p->member[i].map,map,16);
			p->member[i].leader=0;
			p->member[i].online=1;
			p->member[i].lv=lv;
			mapif_party_memberadded(fd,party_id,account_id,0);
			mapif_party_info(-1,p);

			if( p->exp>0 && !party_check_exp_share(p) ){
				p->exp=0;
				flag=0x01;
			}
			if(flag)
				mapif_party_optionchanged(fd,p,0,0);

			inter_party_tosql(party_id, p);
			return 0;
		}
	}
	mapif_party_memberadded(fd,party_id,account_id,1);
	//inter_party_tosql(party_id, p);
	return 0;
}
// パーティー設定変更要求
int mapif_parse_PartyChangeOption(int fd,unsigned long party_id,unsigned long account_id,int exp,int item)
{
	struct party *p;
	int flag=0;

	p = party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}

	inter_party_fromsql(party_id, p);

	if(p->party_id <= 0){
		return 0;
	}

	p->exp=exp;
	if( exp>0 && !party_check_exp_share(p) ){
		flag|=0x01;
		p->exp=0;
	}

	p->item=item;

	mapif_party_optionchanged(fd,p,account_id,flag);
	inter_party_tosql(party_id, p);
	return 0;
}
// パーティ脱退要求
int mapif_parse_PartyLeave(int fd,unsigned long party_id,unsigned long account_id)
{
	char t_member[24];
	struct party *p = party_pt;
	if (p == NULL) {
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}

	inter_party_fromsql(party_id, p);
	if (p->party_id >= 0) {
		int i;
		for (i = 0; i < MAX_PARTY; i++) {
			if (p->member[i].account_id == account_id) {
				//ShowMessage("p->member[i].account_id = %d , account_id = %d \n",p->member[i].account_id,account_id);
				mapif_party_leaved(party_id, account_id, p->member[i].name);

				// Update char information, does the name need encoding?
				sprintf (tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld' AND `name`='%s'",
					char_db, party_id, jstrescapecpy(t_member,p->member[i].name));
				if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
					ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
				}
//				ShowMessage("Delete member %s from MySQL \n", p->member[i].name);

				if (p->member[i].leader == 1){
					int j;
					//char *tmp = tmp_sql;
					//tmp_sql[0] = '\0';
					for (j = 0; j < MAX_PARTY; j++) {
						//ShowMessage("j = %d , p->member[j].account_id = %d , p->member[j].account_id = %d \n",j,p->member[j].account_id,p->member[j].account_id);
						if (p->member[j].account_id > 0 && j != i) {
							mapif_party_leaved(party_id, p->member[j].account_id, p->member[j].name);
							// Update char information, does the name need encoding?
							/*if (tmp_sql[0] == '\0')
								tmp += sprintf (tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%d', `name`='%s'",
									char_db, party_id, jstrescapecpy(t_member, p->member[i].name));
							else
								tmp += sprintf (tmp, " OR `name`='%s'", jstrescapecpy(t_member, p->member[i].name));*/
//							ShowMessage("Delete member %s from MySQL \n", p->member[j].name);
						}
					}
					// we'll skip name-checking and just reset everyone with the same party id [celest]
					// -- if anything goes wrong just uncomment the section above ^^;
					sprintf (tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld'", char_db, party_id);
					if (/*tmp_sql != '\0' &&*/ mysql_SendQuery(&mysql_handle, tmp_sql)) {
						ShowError("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
					}
					// Delete the party, if has no member.
					sprintf(tmp_sql, "DELETE FROM `%s` WHERE `party_id`='%ld'", party_db, party_id);
					if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
						ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
					}
//					ShowMessage("Leader breaks party %d \n",party_id);
					memset(p, 0, sizeof(struct party));
				} else
					memset(&p->member[i], 0, sizeof(struct party_member));
				break;
			}
		}

		if (party_check_empty(p) == 0)
			mapif_party_info(-1,p);// まだ人がいるのでデータ送信
		//else
		//	inter_party_tosql(party_id,p);	// Break the party if no member
	} else {
		sprintf(tmp_sql, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%ld' AND `account_id`='%ld' AND `online`='1'",
			char_db, party_id, account_id);
		if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
			ShowMessage("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
		}
	}
	return 0;
}
// When member goes to other map
int mapif_parse_PartyChangeMap(int fd,unsigned long party_id,unsigned long account_id,char *map,int online,int lv)
{
	struct party *p;
	int i;

	p = party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}
	inter_party_fromsql(party_id, p);

	if(p->party_id <= 0){
		return 0;
	}
	for(i=0;i<MAX_PARTY;i++){
		if(p->member[i].account_id==account_id){
			int flag=0;

			memcpy(p->member[i].map,map,16);
			p->member[i].online=online;
			p->member[i].lv=lv;
			mapif_party_membermoved(p,i);

			if( p->exp>0 && !party_check_exp_share(p) ){
				p->exp=0;
				flag=1;
			}
			if(flag)
				mapif_party_optionchanged(fd,p,0,0);
			break;
		}
	}
	inter_party_tosql(party_id, p);
	return 0;
}
// パーティ解散要求
int mapif_parse_BreakParty(int fd,int party_id)
{
	struct party *p;

	p = party_pt;
	if(p==NULL){
		ShowMessage("int_party: out of memory !\n");
		return 0;
	}

	inter_party_fromsql(party_id, p);

	if(p->party_id <= 0){
		return 0;
	}
	inter_party_tosql(party_id,p);

	mapif_party_broken(fd,party_id);
	return 0;
}
// パーティメッセージ送信
int mapif_parse_PartyMessage(int fd,unsigned long party_id,unsigned long account_id,char *mes,size_t len)
{
	return mapif_party_message(party_id,account_id,mes,len, fd);
}
// パーティチェック要求
int mapif_parse_PartyCheck(int fd,unsigned long party_id,unsigned long account_id,char *nick)
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
int inter_party_leave(unsigned long party_id,unsigned long account_id)
{
	return mapif_parse_PartyLeave(-1,party_id,account_id);
}
