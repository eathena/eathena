
/*
 for IPB of version 2.x  comment out for 1.x  but be warned
 This requires you know your database, and aren't afraid of editing your PHP files.

 you require to know how to make the registration script in the IPB files already
 no one offers this help, but this makes for perfect usage of the forums
 to manage your login, from banning IPs, to banning Accounts from forums.

 passwdenc IPBNEW if you have an older system, but i hope you dont have something old.
 as this was somewhat designed for the newer systems.

*/
#define IPBNEW

#include "main.h"

int auth_fifo_pos = 0;
int server_freezeflag[MAX_SERVERS]; // Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed
int server_fd[MAX_SERVERS];
int ipban = 1;
int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 60;
char loginlog_db[256] = "loginlog";

//----------------------------------------------------------------------------------------
// Login Server Client Parsing Functions
//----------------------------------------------------------------------------------------
void dump_packet_info(int fd){
	int x;
	FILE *fp;
	char packet_txt[256] = "save/packet.txt";
	time_t now;

	if ((fp = fopen(packet_txt, "a")) == NULL) {
		printf("int_login.c: cant write [%s] !!! data is lost !!!\n", packet_txt);
		return;
	} else {
		time(&now);
		for(x = 0; x < 50; x++)
			fprintf(fp, " id:%d RFIFOLdec: %d   RIFIOBdec: %d   RFIFOBhex: %d   RFIFOWhex: %x   RFIFOWdec: %d   RFIFOPstr:  %s \n",
							x, RFIFOL(fd,x), RFIFOB(fd,x), RFIFOB(fd,x), RFIFOW(fd,x), RFIFOW(fd,x), RFIFOP(fd,x));
		fprintf(fp, "\n\n");
		fclose(fp);
	}
}

//-----------------------------------------------------
// check user level
//-----------------------------------------------------
int isGM(int account_id) {
	int level;

	level = 0;
	sql_query("SELECT `mgroup` FROM `ibf_members` WHERE `id`='%d'", account_id);

	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);
		level = atoi(sql_row[0]);
		if (level > 99)	level = 99;
		if (level < 0) level = 0;
	}

	mysql_free_result(sql_res);
	return level;
}

//-----------------------------------------------------
// Auth
//-----------------------------------------------------
struct temp_account {
	char pass_hash[64], pass_salt[64], userid[24];
	int id, connect_until, state, sex, mgroup;
	time_t ban_until;
};


int mmo_auth( struct mmo_account* account , int fd){

	struct temp_account ta;
	char t_uid[32], t_pass[32];
	char hash_password[64], hashed_password[64], in_password[64], hash_salt[64];
	char ip[16];

	unsigned char *sin_addr = (unsigned char *)&session[fd]->client_addr.sin_addr;


	sprintf(ip, "%d.%d.%d.%d", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);

	jstrescapecpy(t_uid,account->userid);
	jstrescapecpy(t_pass,account->passwd);

	sql_query("SELECT m.`id`, m.`name`, c.`converge_pass_hash`, c.`converge_pass_salt`, m.`mgroup`, e.`field_1` FROM ibf_members m "
			 "LEFT JOIN ibf_members_converge c ON (m.id = c.converge_id) LEFT JOIN ibf_pfields_content e ON (m.id = e.member_id) WHERE m.`name` = '%s';",t_uid);

	sql_res = mysql_store_result(&mysql_handle);

	if ((sql_row = mysql_fetch_row(sql_res))){
		ta.id = atoi(sql_row[0]);
		strcpy(ta.userid,sql_row[1]);
		strcpy(ta.pass_hash,sql_row[2]);  //hash to compare salt and password
		strcpy(ta.pass_salt,sql_row[3]); //salt to mix in with password for hash
		ta.mgroup = atoi(sql_row[4]);
		ta.sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';
		mysql_free_result(sql_res);
	} else return 0;



	MD5_String(account->passwd,in_password);
	MD5_String(ta.pass_salt,hash_salt);
	sprintf(hash_password, "%s%s", hash_salt, in_password);
	MD5_String(hash_password,hashed_password);


	if (strcmp(hashed_password, ta.pass_hash))return 1;

	// use only when blocked by group (exploiters, banned etc.)
	switch(ta.mgroup) {
		case 1: return 4;
		case 5: return 15;
		case 6: return 7;
		case 7: return 9;
		case 8: return 10;
		case 9: return 11;
		default: break;
	}

	account->account_id = ta.id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	account->sex = ta.sex;

	sql_query("UPDATE `ibf_members` SET `last_activity` = UNIX_TIMESTAMP(), `ip_address`='%s' WHERE  `id` = '%d'", ip,  ta.id);
	return -1;
}

//-------------------------------------------
// Alive packets for 2 RO versions
//-------------------------------------------
void parse_login_alive_x200(int fd){
	if (RFIFOREST(fd) < 26) return;
	RFIFOSKIP(fd,26);
}

//-------------------------------------------
// Alive packets for 2 RO versions
//-------------------------------------------
void parse_login_alive_x204(int fd){
	if (RFIFOREST(fd) < 18) return;
	RFIFOSKIP(fd,18);
}

//-------------------------------------------
// Client login packet
//-------------------------------------------
void parse_login_client(int fd){
	if(RFIFOREST(fd)< 55) return;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	struct mmo_account account;
	char t_uid[100];
	int i,server_num;

	account.userid = RFIFOP(fd, 6);
	account.passwd = RFIFOP(fd, 30);
	account.passwdenc=0;
	int result=mmo_auth(&account, fd);

	jstrescapecpy(t_uid,RFIFOP(fd, 6));
	if(result==-1){
		int gm_level = isGM(account.account_id);
		if (min_level_to_connect > gm_level) {
			// Anti hammering packet, will close client down after error reporting,
//			WFIFOW(fd,0)=0x6a;
//			WFIFOB(fd,2)=5;
//			WFIFOSET(fd,23);
			session[fd]->eof = 1;
		} else {

			sql_query("INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s', '100', 'login ok')", loginlog_db,  p[0], p[1], p[2], p[3], t_uid);
			server_num=0;
			for(i = 0; i < MAX_SERVERS; i++) {
				if (server_fd[i] >= 0) {
					WFIFOL(fd,47+server_num*32) = server[i].ip;
					WFIFOW(fd,47+server_num*32+4) = server[i].port;
					memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,47+server_num*32+26) = server[i].users;
					WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,47+server_num*32+30) = server[i].new;
					server_num++;
				}
			}

			// if at least 1 char-server
			if (server_num > 0 || gm_level >= 20) {
				WFIFOW(fd,0)=0x69;
				WFIFOW(fd,2)=47+32*server_num;
				WFIFOL(fd,4)=account.login_id1;
				WFIFOL(fd,8)=account.account_id;
				WFIFOL(fd,12)=account.login_id2;
				WFIFOL(fd,16)=0;
				memcpy(WFIFOP(fd,20),account.lastlogin,24);
				WFIFOB(fd,46)=account.sex;
				WFIFOSET(fd,47+32*server_num);
				if(auth_fifo_pos>=AUTH_FIFO_SIZE) auth_fifo_pos=0;
				auth_fifo[auth_fifo_pos].account_id=account.account_id;
				auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
				auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
				auth_fifo[auth_fifo_pos].sex=account.sex;
				auth_fifo[auth_fifo_pos].delflag=0;
				auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
				auth_fifo_pos++;
			} else {
#if 0
				// Anti hammering packet, will close client down after error reporting,
				WFIFOW(fd,0)=0x6a;
				WFIFOB(fd,2)=5;
				WFIFOSET(fd,23);
#endif
				session[fd]->eof=1;
			}
		}

	} else {
		char error[64];
		switch((result)) {
			case  0: sprintf(error,"Unregisterd ID."); break;
			case  1: sprintf(error,"Wrong password."); break;
			case  4: sprintf(error,"Not validated yet."); break;
			case  7: sprintf(error,"Banned for exploiting."); break;
			case  9: sprintf(error,"Banned for scamming."); break;
			case 10: sprintf(error,"Banned for misc."); break;
			case 11: sprintf(error,"Banned for Rule Breaking."); break;
			case 15: sprintf(error,"Temp banned for rule breaking."); break;
			default: sprintf(error,"Uknown Error."); break;
		}

		sql_query("REPLACE INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s', '%d','login failed : %s')", loginlog_db, p[0], p[1], p[2], p[3], t_uid, result, error);

		if ((result == 1) && (dynamic_pass_failure_ban != 0)){	// failed password
			sql_query("SELECT count(*) FROM `%s` WHERE `ip` = '%d.%d.%d.%d' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE", loginlog_db, p[0], p[1], p[2], p[3], dynamic_pass_failure_ban_time);	//how many times filed account? in one ip.
			sql_res = mysql_store_result(&mysql_handle) ;
			sql_row = mysql_fetch_row(sql_res);	//row fetching
			if (atoi(sql_row[0]) >= dynamic_pass_failure_ban_how_many )
				sql_query("REPLACE INTO `ipbanlist`(`list`,`timeb`,`timer`,`code`,`reason`) VALUES ('%d.%d.%d.%d', UNIX_TIMESTAMP() , UNIX_TIMESTAMP() +  60 * %d,'%d', 'Password error ban: %s')", p[0], p[1], p[2],p[3], dynamic_pass_failure_ban_how_long,result,t_uid);
			mysql_free_result(sql_res);
		}

#if 1
		//cannot connect login failed
		WFIFOW(fd,0)=0x6a;
		WFIFOB(fd,2)=result;
		WFIFOSET(fd,23);
#endif
//		session[fd]->eof = 1;

	}
	RFIFOSKIP(fd,55);
}

//-------------------------------------------
// Char-Server packet parsing
//-------------------------------------------
void parse_login_charserver(int fd){
	if(RFIFOREST(fd)<86) return;
	struct mmo_account account;
	char t_uid[32];

	#ifdef LOGIN_DEBUG
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	printf("server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
		RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58),
		p[0], p[1], p[2], p[3]);
	#endif
	unsigned char* server_name;
	account.userid = RFIFOP(fd, 2);
	account.passwd = RFIFOP(fd, 26);
	account.passwdenc = 0;
	server_name = RFIFOP(fd,60);
	int result = mmo_auth(&account, fd);
	if(result == -1 && account.sex==2 && server_fd[account.account_id]==-1){
		memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
		server[account.account_id].ip=RFIFOL(fd,54);
		server[account.account_id].port=RFIFOW(fd,58);
		memcpy(server[account.account_id].name,RFIFOP(fd,60),20);
		server[account.account_id].users=0;
		server[account.account_id].maintenance=RFIFOW(fd,82);
		server[account.account_id].new=RFIFOW(fd,84);
		server_fd[account.account_id]=fd;
		server_freezeflag[account.account_id] = 5; // Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed
		sql_query("DELETE FROM `sstatus` WHERE `index`='%ld'", account.account_id);
		jstrescapecpy(t_uid,server[account.account_id].name);
		sql_query("INSERT DELAYED INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%ld', '%s', '%d')", account.account_id, t_uid,0);

		WFIFOW(fd,0)=0x2711;
		WFIFOB(fd,2)=0;
		WFIFOSET(fd,3);
		session[fd]->func_parse=parse_fromchar;
		realloc_fifo(fd,FIFOSIZE_SERVERLINK,FIFOSIZE_SERVERLINK);
	} else {
		WFIFOW(fd, 0) =0x2711;
		WFIFOB(fd, 2)=3;
		WFIFOSET(fd, 3);

	}
	RFIFOSKIP(fd, 86);
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd) {
	#ifdef LOGIN_DEBUG
	printf("fd = %d  or %x\n",fd,fd);
	#endif

	int i;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	char ip[16];
	static int last_fd = 0;

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
#if 0
	if(last_fd && (fd == last_fd) && (session[fd]->func_parse!=parse_fromchar))
	{
		printf("Flood on fd %d, ip %s\n",fd,ip);
		return 0;
	}
	
	last_fd = fd;
#endif
	if (ipban > 0) {
		sql_query(
			"SELECT count(*) "
			"FROM `ipbanlist` "
			"WHERE `list` LIKE '%d.%d.%d.%'"
			"OR `list` LIKE '%d.%d.%d.%d'",
			p[0], p[1], p[2],
			p[0], p[1], p[2], p[3]);

		if ((sql_res = mysql_store_result(&mysql_handle))){
			sql_row = mysql_fetch_row(sql_res);
			if (atoi(sql_row[0]) >0) {
				sql_query(
					"REPLACE INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) "
					"VALUES (NOW(), '%d.%d.%d.%d', 'unknown','-3', 'ip banned')",
					loginlog_db, p[0], p[1], p[2], p[3]);
#if 1
				WFIFOW(fd,0)=0x6a;
				WFIFOB(fd,2)=5;
				WFIFOSET(fd,23);
#else
				session[fd]->eof = 1;
#endif
			}
			mysql_free_result(sql_res);
		}
	}

	if (session[fd]->eof) {
		for(i = 0; i < MAX_SERVERS; i++) {
			if (server_fd[i] == fd) {
				server_fd[i] = -1;
			}
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	while(RFIFOREST(fd)>=2){
		#ifdef LOGIN_DEBUG
		printf("parse_login : %d %d packet case=%x \n", fd, RFIFOREST(fd), RFIFOW(fd,0));
		#endif
		switch(RFIFOW(fd,0)){
		case 0x200: parse_login_alive_x200(fd); break;
		case 0x204: parse_login_alive_x204(fd); break;
		case 0x64: parse_login_client(fd); break;
		case 0x01dd: parse_login_client(fd); break;		// request client login with encrypt
		case 0x2710: parse_login_charserver(fd); return 0;	// return 0;
		default: session[fd]->eof = 1; 
		printf("parse_login : %d %d packet case=%x \n", fd, RFIFOREST(fd), RFIFOW(fd,0));
		return 0;
		}
	}

	return 0;
}


