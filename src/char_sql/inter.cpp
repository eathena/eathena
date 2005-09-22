//
// original code from athena
// SQL conversion by Jioh L. Jung
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
#include "lock.h"

#include "char.h"
#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_storage.h"
#include "int_pet.h"


#define WISDATA_TTL		(60*1000)	// Wisデータの生存時間(60秒)
#define WISDELLIST_MAX	256			// Wisデータ削除リストの要素数


struct accreg {
	uint32 account_id;
	size_t reg_num;
	struct global_reg reg[ACCOUNT_REG_NUM];
};

static struct accreg *accreg_pt;


size_t party_share_level = 10;
MYSQL mysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
size_t sql_fields;
size_t sql_cnt;
char tmp_sql[65535];

MYSQL lmysql_handle;
char tmp_lsql[65535];
MYSQL_RES* 	lsql_res ;
MYSQL_ROW	lsql_row ;

unsigned short char_server_port = 3306;
char char_server_ip[32] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[32] = "ragnarok";
char char_server_db[32] = "ragnarok";

unsigned short login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";

// sending packet list
int inter_send_packet_length[]={
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0,
	35,-1,11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,
	 9, 9,-1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};
// recv. packet list
int inter_recv_packet_length[]={
	-1,-1, 7, -1, -1, 6, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,
	74, 6,52,14, 10,29, 6,-1, 34, 0, 0, 0,  0, 0,  0, 0,
	-1, 6,-1, 0, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};

struct WisData {
	int id,fd,count,len;
	unsigned long tick;
	char src[24];
	char dst[24];
	char msg[512];
};
static struct dbt * wis_db = NULL;
static int wis_dellist[WISDELLIST_MAX], wis_delnum;

int inter_sql_test (void);

//--------------------------------------------------------
// Save account_reg to sql (type=2)
int inter_accreg_tosql(uint32 account_id,struct accreg *reg){

	size_t j;
	char temp_str[64]; //Needs be twice the source to ensure it fits [Skotlex]
	if (account_id<=0) return 0;
	reg->account_id=account_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%ld'",reg_db, (unsigned long)account_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (delete `global_reg_value`)- %s\n", mysql_error(&mysql_handle) );
	}

	if (reg->reg_num<=0) return 0;

	for(j=0;j<reg->reg_num;j++){
		if(reg->reg[j].str != NULL){
			sprintf(tmp_sql,"INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES (2,'%ld', '%s','%ld')",
				reg_db, reg->account_id, jstrescapecpy(temp_str,reg->reg[j].str), (long)reg->reg[j].value);
			if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
				ShowMessage("DB server Error (insert `global_reg_value`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
	}
	return 0;
}

// Load account_reg from sql (type=2)
int inter_accreg_fromsql(uint32 account_id,struct accreg *reg)
{
	int j=0;
	if (reg==NULL) return 0;
	memset(reg, 0, sizeof(struct accreg));
	reg->account_id=account_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	sprintf (tmp_sql, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%ld'",reg_db, (unsigned long)reg->account_id);
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (select `global_reg_value`)- %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle);

	if (sql_res) {
		for(j=0;(sql_row = mysql_fetch_row(sql_res));j++){
			memcpy(reg->reg[j].str, sql_row[0],32);
			reg->reg[j].value = atoi(sql_row[1]);
		}
		mysql_free_result(sql_res);
	}
	reg->reg_num=j;
	return 0;
}

// Initialize
int inter_accreg_sql_init()
{
	CREATE(accreg_pt, struct accreg, 1);
	return 0;

}

/*==========================================
 * read config file
 *------------------------------------------
 */
int inter_config_read(const char *cfgName) {
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowMessage ("Reading Interserver Configuration: %s\n",cfgName);

	fp=safefopen(cfgName,"r");
	if(fp==NULL){
		ShowError("inter config file not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp)){
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcasecmp(w1,"char_server_ip")==0){
			strcpy(char_server_ip, w2);
			ShowMessage ("set char_server_ip : %s\n",w2);
		}
		else if(strcasecmp(w1,"char_server_port")==0){
			char_server_port=atoi(w2);
			ShowMessage ("set char_server_port : %s\n",w2);
		}
		else if(strcasecmp(w1,"char_server_id")==0){
			strcpy(char_server_id, w2);
			ShowMessage ("set char_server_id : %s\n",w2);
		}
		else if(strcasecmp(w1,"char_server_pw")==0){
			strcpy(char_server_pw, w2);
			ShowMessage ("set char_server_pw : %s\n",w2);
		}
		else if(strcasecmp(w1,"char_server_db")==0){
			strcpy(char_server_db, w2);
			ShowMessage ("set char_server_db : %s\n",w2);
		}
		//Logins information to be read from the inter_athena.conf
		//for character deletion (checks email in the loginDB)

		else if(strcasecmp(w1,"login_server_ip")==0){
			strcpy(login_server_ip, w2);
			ShowMessage ("set login_server_ip : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_port")==0){
			login_server_port=atoi(w2);
			ShowMessage ("set login_server_port : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_id")==0){
			strcpy(login_server_id, w2);
			ShowMessage ("set login_server_id : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_pw")==0){
			strcpy(login_server_pw, w2);
			ShowMessage ("set login_server_pw : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
			ShowMessage ("set login_server_db : %s\n",w2);
		}
		else if(strcasecmp(w1,"party_share_level")==0){
			party_share_level=atoi(w2);
			if(party_share_level < 0) party_share_level = 0;
		}else if(strcasecmp(w1,"import")==0){
			inter_config_read(w2);
		}
		else if(strcasecmp(w1,"log_inter")==0){
			log_inter = atoi(w2);
		}
		else if(strcasecmp(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
			ShowMessage ("set login_server_db : %s\n",w2);
		}
	}
	fclose(fp);

	ShowMessage ("success reading interserver configuration\n");

	return 0;
}

// Save interlog into sql
int inter_log(char *fmt,...)
{
	char str[255];
	char temp_str[510]; //Needs be twice as long as str[] //Skotlex
	va_list ap;
	va_start(ap,fmt);
	vsprintf(str,fmt,ap);
	va_end(ap);

	sprintf(tmp_sql,"INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')",interlog_db, jstrescapecpy(temp_str,str));
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error (insert `interlog`)- %s\n", mysql_error(&mysql_handle) );
	}

	
	return 0;
}


// initialize
int inter_init(const char *file)
{
	//int i;

	ShowMessage ("interserver initialize...\n");
	inter_config_read(file);

	//DB connection initialized
	mysql_init(&mysql_handle);
	ShowMessage("Connect Database server on %s:%u....(Character Server)\n", char_server_ip,char_server_port);
	if(!mysql_real_connect(&mysql_handle, char_server_ip, char_server_id, char_server_pw,
		char_server_db ,char_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowMessage("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
	else if (inter_sql_test()) {
		ShowMessage ("Connect Success! (Character Server)\n");
	}

	mysql_init(&lmysql_handle);
	ShowMessage("Connect Database server on %s:%u (Login Server)....\n", login_server_ip,login_server_port);
	if(!mysql_real_connect(&lmysql_handle, login_server_ip, login_server_id, login_server_pw,
		login_server_db ,login_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowMessage("%s\n",mysql_error(&lmysql_handle));
			exit(1);
	}else {
		ShowMessage ("Connect Success! (Login Server)");
	}
	wis_db = numdb_init();
	inter_guild_sql_init();
	inter_storage_sql_init();
	inter_party_sql_init();

	inter_pet_sql_init();
	inter_accreg_sql_init();

	return 0;
}

int inter_sql_test (void)
{
	const char fields[][24] = {
		"father",	// version 1363
		"fame",		// version 1491
	};	
	char buf[1024] = "";
	size_t i;

	sprintf(tmp_sql, "EXPLAIN `%s`",char_db);
	if (mysql_SendQuery(&mysql_handle, tmp_sql)) {
		ShowSQL ("DB server Error (explain)- %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	// store DB fields
	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res))) {
			strcat (buf, sql_row[0]);
			strcat (buf, " ");
		}
	}

	// check DB strings
	for (i = 0; i < (sizeof(fields) / sizeof(fields[0])); i++) {
		if(!strstr(buf, fields[i])) {
			ShowSQL ("Field `%s` not be found in `%s`. Consider updating your database!\n", fields[i], char_db);
			exit(1);
		}
	}

	mysql_free_result(sql_res);

	return 1;
}

// finalize
int wis_db_final (void *k, void *data, va_list &ap) {
	struct WisData *p = (struct WisData *)data;
	if (p) aFree(p);
	return 0;
}
void inter_final()
{

	if(wis_db)
	{
		numdb_final(wis_db, wis_db_final);
		wis_db=NULL;
	}
	inter_guild_sql_final();
	inter_storage_sql_final();
	inter_party_sql_final();
	inter_pet_sql_final();
	
	if (accreg_pt) aFree(accreg_pt);
	return;
}

int inter_mapif_init(int fd) {
	inter_guild_mapif_init(fd);

	return 0;
}


//--------------------------------------------------------

// GM message sending
int mapif_GMmessage(unsigned char *mes, int len, int sfd)
{
	CREATE_BUFFER(buf,unsigned char,len);

	WBUFW(buf, 0) = 0x3800;
	WBUFW(buf, 2) = len;
	memcpy(WBUFP(buf, 4), mes, len-4);
	mapif_sendallwos(sfd, buf, len);
	ShowMessage(CL_BT_BLUE" inter server: GM[len:%d] - '%s' \n"CL_NORM, len, mes);

	DELETE_BUFFER(buf);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisData *wd)
{
	CREATE_BUFFER(buf,unsigned char,56 + wd->len);

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 +wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, 24);
	memcpy(WBUFP(buf,32), wd->dst, 24);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif_sendall(buf,WBUFW(buf,2));

	DELETE_BUFFER(buf);
	return 0;
}
// Wis sending result
int mapif_wis_end(struct WisData *wd,int flag)
{
	unsigned char buf[27];

	WBUFW(buf, 0)=0x3802;
	memcpy(WBUFP(buf, 2),wd->src,24);
	WBUFB(buf,26)=flag;
	mapif_send(wd->fd,buf,27);
//	ShowMessage("inter server wis_end %d\n",flag);
	return 0;
}

int mapif_account_reg(int fd,unsigned char *src)
{
	CREATE_BUFFER(buf,unsigned char, (unsigned short)WBUFW(src,2));

	memcpy(WBUFP(buf,0),src,WBUFW(src,2));
	WBUFW(buf, 0)=0x3804;
	mapif_sendallwos(fd,buf,WBUFW(buf,2));

	DELETE_BUFFER(buf);
	return 0;
}

// Send the requested account_reg
int mapif_account_reg_reply(int fd,uint32 account_id)
{
	struct accreg *reg=accreg_pt;
	inter_accreg_fromsql(account_id,reg);
	if( !session_isActive(fd) )
		return 0;

	WFIFOW(fd,0)=0x3804;
	WFIFOL(fd,4)=account_id;
	if(reg->reg_num==0){
		WFIFOW(fd,2)=8;
	}else{
		size_t j;
		unsigned short p;
		for(j=0,p=8;j<reg->reg_num;j++,p+=36){
			memcpy(WFIFOP(fd,p),reg->reg[j].str,32);
			WFIFOL(fd,p+32)=reg->reg[j].value;
		}
		WFIFOW(fd,2)=p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

int mapif_send_gmaccounts()
{
	size_t i;
	unsigned short len = 4;
	unsigned char buf[32000];

	// forward the gm accounts to the map server
	len = 4;
	WBUFW(buf,0) = 0x2b15;
				
	for(i = 0; i < GM_num; i++) {
		WBUFL(buf, len) = gm_account[i].account_id;
		WBUFB(buf, len+4) = (unsigned char)gm_account[i].level;
		len += 5;
	}
	WBUFW(buf, 2) = len;
	mapif_sendall(buf, len);

	return 0;
}


//--------------------------------------------------------

// Existence check of WISP data
int check_ttl_wisdata_sub(void *key, void *data, va_list &ap) {
	unsigned long tick;
	struct WisData *wd = (struct WisData *)data;
	tick = (uint32)va_arg(ap, int);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata() {
	unsigned long tick = gettick();
	int i;

	do {
		wis_delnum = 0;
		numdb_foreach(wis_db, check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)numdb_search(wis_db, wis_dellist[i]);
			ShowMessage("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			//mapif_wis_end(wd, 1); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			numdb_erase(wis_db, wd->id);
			aFree(wd);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------

// GM message sending
int mapif_parse_GMmessage(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	mapif_GMmessage(RFIFOP(fd, 4), RFIFOW(fd, 2), fd);
	return 0;
}


// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	static int wisid = 0;
	char t_name[64];

	if( !session_isActive(fd) )
		return 0;

	if((size_t)RFIFOW(fd,2) >= 52+sizeof(wd->msg) ) {
		ShowMessage("inter: Wis message size too long.\n");
		return 0;
	} else if( RFIFOW(fd,2) <= 52 ) { // normaly, impossible, but who knows...
		ShowMessage("inter: Wis message doesn't exist.\n");
		return 0;
	}
	sprintf (tmp_sql, "SELECT `name` FROM `%s` WHERE `name`='%s'",
		char_db, jstrescapecpy(t_name, (char *)RFIFOP(fd,28)));
	if(mysql_SendQuery(&mysql_handle, tmp_sql) ) {
		ShowMessage("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle);

	// search if character exists before to ask all map-servers
	if (!(sql_row = mysql_fetch_row(sql_res))) {
		unsigned char buf[27];
		WBUFW(buf, 0) = 0x3802;
		memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
		WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		mapif_send(fd, buf, 27);
	// Character exists. So, ask all map-servers
	} else {
		// to be sure of the correct name, rewrite it
		memset(RFIFOP(fd,28), 0, 24);
		safestrcpy((char*)RFIFOP(fd,28), sql_row[0], 24);
		// if source is destination, don't ask other servers.
		if (strcmp((char*)RFIFOP(fd,4),(char*)RFIFOP(fd,28)) == 0) {
			unsigned char buf[27];
			WBUFW(buf, 0) = 0x3802;
			memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
			WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			mapif_send(fd, buf, 27);
		} else {
			
			CREATE(wd, struct WisData, 1);

			// Whether the failure of previous wisp/page transmission (timeout)
			check_ttl_wisdata();

			wd->id = ++wisid;
			wd->fd = fd;
			wd->len= RFIFOW(fd,2)-52;
			memcpy(wd->src, RFIFOP(fd, 4), 24);
			memcpy(wd->dst, RFIFOP(fd,28), 24);
			memcpy(wd->msg, RFIFOP(fd,52), wd->len);
			wd->tick = gettick();
			numdb_insert(wis_db, wd->id, wd);
			mapif_wis_message(wd);
		}
	}
	
	//Freeing ... O.o 
	if(sql_res){
		mysql_free_result(sql_res);
	}
	
	return 0;
}


// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	int id = RFIFOL(fd,2), flag = RFIFOB(fd,6);
	struct WisData *wd = (struct WisData*)numdb_search(wis_db, id);

	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		numdb_erase(wis_db, id);
		aFree(wd);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[65536];
	if( !session_isActive(fd) )
		return 0;

	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// Save account_reg into sql (type=2)
int mapif_parse_AccReg(int fd)
{
	if( !session_isActive(fd) )
		return 0;

	int j,p;
	struct accreg *reg=accreg_pt;
	uint32 account_id = RFIFOL(fd,4);
	memset(accreg_pt,0,sizeof(struct accreg));

	for(j=0,p=8;j<ACCOUNT_REG_NUM && p<RFIFOW(fd,2);j++,p+=36){
		memcpy(reg->reg[j].str,RFIFOP(fd,p),32);
		reg->reg[j].value=RFIFOL(fd,p+32);
	}
	reg->reg_num=j;

	inter_accreg_tosql(account_id,reg);
	mapif_account_reg(fd,RFIFOP(fd,0));	// Send confirm message to map
	return 0;
}

// Request the value of account_reg
int mapif_parse_AccRegRequest(int fd)
{
	if( !session_isActive(fd) )
		return 0;

//	ShowMessage("mapif: accreg request\n");
	return mapif_account_reg_reply(fd,RFIFOL(fd,2));
}



//--------------------------------------------------------
int inter_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);
	int len=0;

	// inter鯖管轄かを調べる
	if(cmd<0x3000 || cmd>=0x3000+( sizeof(inter_recv_packet_length)/sizeof(inter_recv_packet_length[0]) ) )
		return 0;

	// パケット長を調べる #1
	if(	(len=inter_check_length(fd,inter_recv_packet_length[cmd-0x3000]))==0 )
		return 0;
	// パケット長を調べる #2
	if(len>0 && RFIFOREST(fd) < len)
		return 2;

	switch(cmd){
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_AccReg(fd); break;
	case 0x3005: mapif_parse_AccRegRequest(fd); break;
	default:
		if( inter_party_parse_frommap(fd) )
			break;
		if( inter_guild_parse_frommap(fd) )
			break;
		if( inter_storage_parse_frommap(fd) )
			break;
		if( inter_pet_parse_frommap(fd) )
			break;
		return 0;
	}
	RFIFOSKIP(fd, len );
	return 1;
}

// RFIFO check
int inter_check_length(int fd, int length)
{
	if(length<0){	// v-len packet
		if(RFIFOREST(fd)<4)	// packet not yet
			return 0;
		length = RFIFOW(fd, 2);
	}

	if(RFIFOREST(fd)<length)	// packet not yet
		return 0;

	return length;
}
