// $Id: login.c,v 1.6 2004/09/19 21:12:07 Valaris Exp $
// original : login2.c 2003/01/28 02:29:17 Rev.1.1.1.1
// txt version 1.100

// Includes of the Common folder
#include "../common/base.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/utils.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/dbaccess.h"

// Source Includes
#include "login.h"
#include "md5.hh"

//-----------------------------------------------------
// global variable
//-----------------------------------------------------
unsigned long account_id_count = START_ACCOUNT_NUM;
size_t server_num;
size_t new_account_flag = 0; //Set from config too XD [Sirius]

netaddress loginaddress((ulong)INADDR_ANY, 6900);

struct mmo_char_server server[MAX_SERVERS];

int login_fd=-1;

unsigned int new_reg_tick=0;
int allowed_regs=1;
int num_regs=0;
int time_allowed=10; //Init this to 10 secs, not 10K secs [Skotlex]

char date_format[32] = "%Y-%m-%d %H:%M:%S";
int auth_num = 0, auth_max = 0;

int min_level_to_connect = 0; // minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int check_ip_flag = 1; // It's to check IP of a player between login-server and char-server (part of anti-hacking system)
int check_client_version = 0; //Client version check ON/OFF .. (sirius)
int client_version_to_connect = 20; //Client version needed to connect ..(sirius)
int register_users_online = 1;


int ipban = 1;
int dynamic_account_ban = 1;
int dynamic_account_ban_class = 0;
int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 60;



unsigned short login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";
int use_md5_passwds = 0;


char login_db[256] = "login";
int log_login=1; //Whether to log the logins or not. [Skotlex]
char loginlog_db[256] = "loginlog";

// added to help out custom login tables, without having to recompile
// source so options are kept in the login_athena.conf or the inter_athena.conf
char login_db_account_id[256] = "account_id";
char login_db_userid[256] = "userid";
char login_db_user_pass[256] = "user_pass";
char login_db_level[256] = "level";

int console = 0;

int case_sensitive = 1;

//-----------------------------------------------------

#define AUTH_FIFO_SIZE 256
struct {
	unsigned long account_id;
	unsigned long login_id1;
	unsigned long login_id2;
	unsigned long ip;
	unsigned char sex;
	int delflag;
} auth_fifo[AUTH_FIFO_SIZE];
size_t auth_fifo_pos = 0;


//-----------------------------------------------------

struct dbt *online_db = NULL;

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

void add_online_user(unsigned long account_id) {
	int *p;
	if (register_users_online <= 0)
		return;
	p = (int*)numdb_search(online_db, account_id);
	if (p == NULL)
	{
		p = (int*)aMalloc(sizeof(int));
		*p = account_id;
		numdb_insert(online_db, account_id, p);
	}
}

int is_user_online(unsigned long account_id)
{
	int *p;
	if(register_users_online <= 0)
		return 0;

	p = (int*)numdb_search(online_db, account_id);

	if(p == NULL)
		return 0;

	ShowMessage("Acccount %d\n",*p);
	return 1;
}
void remove_online_user(unsigned long account_id)
{
	int *p;
	if(register_users_online <= 0)
		return;
	p = (int*)numdb_erase(online_db,account_id);
	if (p) aFree(p);
}

//-----------------------------------------------------
// check user level
//-----------------------------------------------------
int isGM(unsigned long account_id) {
	int level;

	level = 0;
	sql_query("SELECT `%s` FROM `%s` WHERE `%s`='%ld'", login_db_level, login_db, login_db_account_id, account_id);
	if (sql_res) {
		sql_fetch_row();
		level = atoi(sql_row[0]);
		if (level > 99)
			level = 99;
	}
	if (level == 0) {
		return 0;
		//not GM
	}
	sql_free();
	return level;
}



//-----------------------------------------------------
// Read Account database - sql db
//-----------------------------------------------------
int mmo_auth_sqldb_init(void) {

	ShowMessage("Login server init....\n");

	// memory initialize
	ShowMessage("memory initialize....\n");

	// DB connection start
	ShowMessage("Connect Login Database Server....\n");
	sql_connect(login_server_ip, login_server_id, login_server_pw,login_server_db, login_server_port);
	ShowMessage("connect success!\n");

	if (log_login)
	{
		sql_query("INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver', '100','login server started')", loginlog_db);
	}

	return 0;
}

//-----------------------------------------------------
// DB server connect check
//-----------------------------------------------------
void mmo_auth_sqldb_sync(void) {
	// db connect check? or close?
	// ping pong DB server -if losted? then connect try. else crash.
}

//-----------------------------------------------------
// close DB
//-----------------------------------------------------
void mmo_db_close(void) {
	int i;

	//set log.
	if (log_login){
		sql_query("INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", loginlog_db);
	}

	//delete all server status
	sql_query("DELETE FROM `sstatus`",NULL);
	//query

	sql_close();
	ShowMessage("close DB connect....\n");

	for (i = 0; i < MAX_SERVERS; i++) {
		if( server[i].fd > 0 )
		{
			session_Remove( server[i].fd );
			server[i].fd = -1;
		}
	}
	session_Remove(login_fd);
	login_fd = -1;
}

//-----------------------------------------------------
// Auth
//-----------------------------------------------------
#ifdef CLOWNPHOBIA
struct temp_account {
	char pass_hash[64], pass_salt[64], userid[24];
	int id, connect_until, state, sex, mgroup;
	time_t ban_until;
};
#endif // CLOWNPHOBIA

int mmo_auth( struct mmo_account* account , int fd)
{
	#ifdef CLOWNPHOBIA
	struct temp_account ta;
	#endif // CLOWNPHOBIA
	struct timeval tv;
	time_t unixtime;
	time_t ban_until_time;
	char tmpstr[256];
	char t_uid[256], t_pass[256];
	char user_password[256];
	char hash_password[64], hashed_password[64], in_password[64], hash_salt[64];

	//added for account creation _M _F
	int len;

	char md5str[64], md5bin[32];

	char ip_str[16];

	unsigned long client_ip = session[fd]->client_ip;
	sprintf(ip_str, "%ld.%ld.%ld.%ld", (client_ip>>24)&0xFF, (client_ip>>16)&0xFF, (client_ip>>8)&0xFF, (client_ip)&0xFF);


	ShowMessage ("auth start...\n");

#ifndef CLOWNPHOBIA
	//accountreg with _M/_F .. [Sirius]
	len = strlen(account->userid) -2;
	if( (account->passwdenc == 0 && account->userid[len] == '_') &&
		(account->userid[len+1] == 'F' || account->userid[len+1] == 'M') && new_account_flag == 1 &&
		(account_id_count <= END_ACCOUNT_NUM) && (len >= 4) && (strlen(account->passwd) >= 4) )
	{
		if (new_account_flag == 1)
			account->userid[len] = '\0';

		sql_query("SELECT `%s` FROM `%s` WHERE `userid` = '%s'", login_db_userid, login_db, account->userid);
		if(sql_res){
			//only continue if amount in this time limit is allowed
			if(gettick() <= new_reg_tick && num_regs >= allowed_regs){
				ShowMessage("Notice: Account registration in disallowed time from: %s!", ip_str);
				return 3;
			}else{
				num_regs=0;
			}
			if(sql_num_rows() == 0){	//ok no existing acc,
				ShowMessage("Adding a new account user: %s with passwd: %s sex: %c (ip: %s)\n", account->userid, account->passwd, account->userid[len+1], ip_str);
				mysql_real_escape_string(&mysql_handle, account->userid, account->userid, strlen(account->userid));
				mysql_real_escape_string(&mysql_handle, account->passwd, account->passwd, strlen(account->passwd));
				sql_query("INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')", login_db, login_db_userid, login_db_user_pass, account->userid, account->passwd, account->userid[len+1], "a@a.com");
				if(num_regs==0){
					new_reg_tick=gettick()+time_allowed*1000;
				}
				num_regs++;
			}//rownum check (0!)
			sql_free();
		}// if there was a result
	}//all values for NEWaccount ok ?

	// auth start : time seed
	gettimeofday(&tv, NULL);
	unixtime = tv.tv_sec;
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&unixtime));
	sprintf(tmpstr+19, ".%03ld", tv.tv_usec/1000);

	jstrescapecpy(t_uid,account->userid);
	jstrescapecpy(t_pass, account->passwd);

	// make query
	sql_query("SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`%s`"
	                " FROM `%s` WHERE %s `%s`='%s'", login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, case_sensitive ? "BINARY" : "", login_db_userid, t_uid);
	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state}


	if (sql_res) {
		sql_fetch_row();
		if (!sql_row) {
			//there's no id.
			ShowMessage ("auth failed no account %s %s %s\n", tmpstr, account->userid, account->passwd);
			sql_free();
			return 0;
		}
	} else {
		ShowMessage("mmo_auth DB result error ! \n");
		return 0;
	}
	//Client Version check[Sirius]
	if(check_client_version == 1 && account->version != 0)
	{
		if(account->version != client_version_to_connect)
		{
			sql_free();
			return 6;
		}
	}

	// Documented by CLOWNISIUS || LLRO || Gunstar lead this one with me
	// IF changed to diferent returns~ you get diferent responses from your msgstringtable.txt
	//Ireturn 2  == line 9
	//Ireturn 5  == line 311
	//Ireturn 6  == line 450
	//Ireturn 7  == line 440
	//Ireturn 8  == line 682
	//Ireturn 9  == line 704
	//Ireturn 10 == line 705
	//Ireturn 11 == line 706
	//Ireturn 12 == line 707
	//Ireturn 13 == line 708
	//Ireturn 14 == line 709
	//Ireturn 15 == line 710
	//Ireturn -1 == line 010
	// Check status

	{
		int encpasswdok = 0;

		if (atoi(sql_row[9]) == -3) {
			//id is banned
			sql_free();
			return -3;
		} else if (atoi(sql_row[9]) == -2) { //dynamic ban
			//id is banned
			sql_free();
			//add IP list.
			return -2;
		}

		if (use_md5_passwds) {
			MD5_String(account->passwd,user_password);
		} else {
			jstrescapecpy(user_password, account->passwd);
		}
		ShowMessage("account id ok encval:%d\n",account->passwdenc);
		if ((strcmp(user_password, sql_row[2]) && !encpasswdok)) {
			if (account->passwdenc == 0) {
				ShowMessage ("auth failed pass error %s %s %s" RETCODE, tmpstr, account->userid, user_password);
			}
			return 1;
		}
		ShowMessage("auth ok %s %s" RETCODE, tmpstr, account->userid);
	}

	ban_until_time = atol(sql_row[8]);

	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state}
	if (ban_until_time != 0) { // if account is banned
		strftime(tmpstr, 20, date_format, localtime(&ban_until_time));
		tmpstr[19] = '\0';
		if (ban_until_time > time(NULL)) { // always banned
			return 6; // 6 = Your are Prohibited to log in until %s
		} else { // ban is finished
			// reset the ban time
			if (atoi(sql_row[9])==7) {//it was a temp ban - so we set STATE to 0
				sql_query("UPDATE `%s` SET `ban_until`='0', `state`='0' WHERE %s `%s`='%s'", login_db, case_sensitive ? "BINARY" : "", login_db_userid, t_uid);
				strcpy(sql_row[9],"0"); //we clear STATE
			} else //it was a permanent ban + temp ban. So we leave STATE = 5, but clear the temp ban
				sql_query("UPDATE `%s` SET `ban_until`='0' WHERE %s `%s`='%s'", login_db, case_sensitive ? "BINARY" : "", login_db_userid, t_uid);

		}
	}

	if (atoi(sql_row[9])) {
		switch(atoi(sql_row[9])) { // packet 0x006a value + 1
		case 1:   // 0 = Unregistered ID
		case 2:   // 1 = Incorrect Password
		case 3:   // 2 = This ID is expired
		case 4:   // 3 = Rejected from Server
		case 5:   // 4 = You have been blocked by the GM Team
		case 6:   // 5 = Your Game's EXE file is not the latest version
		case 7:   // 6 = Your are Prohibited to log in until %s
		case 8:   // 7 = Server is jammed due to over populated
		case 9:   // 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
		case 100: // 99 = This ID has been totally erased
			ShowMessage("Auth Error #%d\n", atoi(sql_row[9]));
			return atoi(sql_row[9]) - 1;
			break;
		default:
			return 99; // 99 = ID has been totally erased
			break;
		}
	}

	if (atol(sql_row[6]) != 0 && atol(sql_row[6]) < time(NULL)) {
		return 2; // 2 = This ID is expired
	}

	if (register_users_online > 0 && is_user_online(atol(sql_row[0]))) {
	        ShowMessage("User [%s] is already online - Rejected.\n",sql_row[1]);
#ifndef TWILIGHT
		return 3; // Rejected
#endif // TWILIGHT
	}

	account->account_id = atoi(sql_row[0]);
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(tmpstr, sql_row[3], 19);
	memcpy(account->lastlogin, tmpstr, 24);
	account->sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';

	sql_query("UPDATE `%s` SET `lastlogin` = NOW(), `logincount`=`logincount` +1, `last_ip`='%s'  WHERE %s  `%s` = '%s'",
	        login_db, ip_str, case_sensitive ? "BINARY" : "", login_db_userid, sql_row[1]);
	sql_free() ; //resource free

#else // CLOWNPHOBIA

	memset(hash_salt,0,sizeof(hash_salt));
	memset(in_password,0,sizeof(in_password));
	memset(hash_password,0,sizeof(hash_password));
	memset(hashed_password,0,sizeof(hashed_password));

	jstrescapecpy(t_uid,account->userid);
	jstrescapecpy(t_pass,account->passwd);

	sql_query("SELECT m.`id`, m.`name`, c.`converge_pass_hash`, c.`converge_pass_salt`, m.`mgroup`, e.`field_1` FROM ibf_members m "
			 "LEFT JOIN ibf_members_converge c ON (m.id = c.converge_id) LEFT JOIN ibf_pfields_content e ON (m.id = e.member_id) WHERE m.`name` = '%s';",t_uid);

	if (sql_res && sql_fetch_row()){
		ta.id = atoi(sql_row[0]);
		strcpy(ta.userid,sql_row[1]);
		strcpy(ta.pass_hash,sql_row[2]);  //hash to compare salt and password
		strcpy(ta.pass_salt,sql_row[3]); //salt to mix in with password for hash
		ta.mgroup = atoi(sql_row[4]);
		ta.sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';
		sql_free();
	} else return 0;



	MD5_String(account->passwd,in_password);
	MD5_String(ta.pass_salt,hash_salt);

	sprintf(hash_password, "%s%s", hash_salt, in_password); // merge the hash of salt and password

	MD5_String(hash_password,hashed_password); // Hash it all together.

	if (strcmp(hashed_password, ta.pass_hash))return 1; // Check what is in the database already

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

	sql_query("UPDATE `ibf_members` SET `last_activity` = UNIX_TIMESTAMP(), `ip_address`='%s' WHERE  `id` = '%d'", ip_str,  ta.id);
#endif // CLOWNPHOBIA

	ShowMessage ("auth Complete...\n");
	return -1;




}

// Send to char
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;

	c = 0;
	for(i = 0; i < MAX_SERVERS; i++)
	{
		fd = server[i].fd;
		if( fd > 0 && fd != sfd )
		{
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}
	return c;
}



///////////////////////////////////////////////////////
// char-server packet parse
//-----------------------------------------------------
///////////////////////////////////////////////////////

//-----------------------------------------------------
// Char Server Packet Functions
//-----------------------------------------------------
void Parse_Char_Connect(int id,int fd){
	int i;
	unsigned long account_id;

	account_id = RFIFOL(fd,2);

	for(i=0;i<AUTH_FIFO_SIZE;i++)
	{
		if (auth_fifo[i].account_id == account_id &&
			auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
			auth_fifo[i].login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18
#endif
			auth_fifo[i].sex == RFIFOB(fd,14) &&
#if CMP_AUTHFIFO_IP != 0
			auth_fifo[i].ip == RFIFOLIP(fd,15) &&
#endif
			!auth_fifo[i].delflag)
		{
			auth_fifo[i].delflag = 1;
			ShowMessage("auth -> %d\n", i);
			break;
		}
	}
	if (i != AUTH_FIFO_SIZE)
	{	// send account_reg
		int p;
		time_t connect_until_time = 0;
		char email[40] = "";
		account_id=RFIFOL(fd,2);
		#ifdef CLOWNPHOBIA
		sql_query("SELECT `email` FROM `%s` WHERE `%s`='%ld'", login_db, login_db_account_id, account_id);
		#else // CLOWNPHOBIA
		sql_query("SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%ld'", login_db, login_db_account_id, account_id);
		#endif // CLOWNPHOBIA
		if (sql_res) {
			sql_fetch_row();
			#ifdef CLOWNPHOBIA
			connect_until_time = 0;
			#else // CLOWNPHOBIA
			connect_until_time = atol(sql_row[1]);
			#endif // CLOWNPHOBIA
			strcpy(email, sql_row[0]);
		}
		sql_free();
		if (account_id > 0)
		{
			sql_query("SELECT `str`,`value` FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%ld'",account_id);
			if (sql_res)
			{
				WFIFOW(fd,0) = 0x2729;
				WFIFOL(fd,4) = account_id;
				for(p = 8; sql_fetch_row(); p+=36)
				{
					memcpy(WFIFOP(fd,p), sql_row[0], 32);
					WFIFOL(fd,p+32) = atoi(sql_row[1]);
				}
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,p);
				//ShowMessage("account_reg2 send : login->char (auth fifo)\n");
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), email, 40);
				WFIFOL(fd,47) = (unsigned long) connect_until_time;
				WFIFOSET(fd,51);
			}
			sql_free();
		}
	}
	else
	{
		WFIFOW(fd,0) = 0x2713;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = 1;
		WFIFOSET(fd,51);
	}
	RFIFOSKIP(fd,19);
}

void Parse_Char_UserCount(int id, int fd){
	// how many users on world? (update)
	if (server[id].users != RFIFOL(fd,2))
	{
		ShowMessage("set users %s : %ld\n", server[id].name, (unsigned long)RFIFOL(fd,2));

		server[id].users = RFIFOL(fd,2);

		sql_query("UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id);
		// query
	}

	// send some answer
	WFIFOW(fd,0) = 0x2718;
	WFIFOSET(fd,2);

	RFIFOSKIP(fd,6);
}

void Parse_Char_PlayerReturn(int id, int fd){
	unsigned long account_id;
	time_t connect_until_time = 0;
	char email[40] = "";

	account_id=RFIFOL(fd,2);
	#ifdef CLOWNPHOBIA
	sql_query("SELECT `email` FROM `%s` WHERE `%s`='%ld'",login_db, login_db_account_id, account_id);
	#else
	sql_query("SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%ld'",login_db, login_db_account_id, account_id);
	#endif
	if (sql_res)
	{
		sql_fetch_row();
		#ifdef CLOWNPHOBIA
		connect_until_time = 0;
		#else
		connect_until_time = atol(sql_row[1]);
		#endif
		strcpy(email, sql_row[0]);
	}
	sql_free();
	//ShowMessage("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %ld)\n", server[id].name, (unsigned long)RFIFOL(fd,2));
	WFIFOW(fd,0) = 0x2717;
	WFIFOL(fd,2) = RFIFOL(fd,2);
	memcpy(WFIFOP(fd, 6), email, 40);
	WFIFOL(fd,46) = (unsigned long) connect_until_time;
	WFIFOSET(fd,50);

	RFIFOSKIP(fd,6);
}

void Parse_Char_Alive(int id, int fd){
	// do whatever it's supposed to do here?

	RFIFOSKIP(fd,2);
}
void Parse_Char_GM(int id, int fd){

	//oldacc = RFIFOL(fd,4);
	ShowMessage("change GM isn't support in this login server version.\n");
	ShowMessage("change GM error 0 %s\n", RFIFOP(fd, 8));

	RFIFOSKIP(fd, RFIFOW(fd, 2));

	WFIFOW(fd, 0) = 0x2721;
	WFIFOL(fd, 2) = RFIFOL(fd,4); // oldacc;
	WFIFOL(fd, 6) = 0; // newacc;
	WFIFOSET(fd, 10);
}

void Parse_Char_ChangeEmail(int id, int fd){
	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
	char ip_str[16];
	int acc;
	char actual_email[40], new_email[40];

	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this.
	RFIFOSKIP(fd, 86);
	return;
	#endif // CLOWNPHOBIA

	sprintf(ip_str, "%ld.%ld.%ld.%ld",
			(session[fd]->client_ip>>24)&0xFF,
			(session[fd]->client_ip>>16)&0xFF,
			(session[fd]->client_ip>>8)&0xFF,
			(session[fd]->client_ip)&0xFF);

	acc = RFIFOL(fd,2);
	memcpy(actual_email, RFIFOP(fd,6), 40);
	memcpy(new_email, RFIFOP(fd,46), 40);
	if (e_mail_check(actual_email) == 0)
		ShowMessage("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
			server[id].name, acc, ip_str);
	else if (e_mail_check(new_email) == 0)
		ShowMessage("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
			server[id].name, acc, ip_str);
	else if (strcasecmp(new_email, "a@a.com") == 0)
		ShowMessage("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
			server[id].name, acc, ip_str);
	else
	{
		sql_query("SELECT `%s`,`email` FROM `%s` WHERE `%s` = '%d'", login_db_userid, login_db, login_db_account_id, acc);
		if (sql_res)
		{
			sql_fetch_row();	//row fetching
			if (strcasecmp(sql_row[1], actual_email) == 0)
			{
				sql_query("UPDATE `%s` SET `email` = '%s' WHERE `%s` = '%d'", login_db, new_email, login_db_account_id, acc);
				// query
				ShowMessage("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
					server[id].name, acc, sql_row[0], actual_email, ip_str);
			}
		}
		sql_free();
	}
	RFIFOSKIP(fd, 86);
}


void Parse_Char_Ban(int id, int fd){
	int acc;
	struct tm *tmtime;
	time_t timestamp, tmptime;

	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this
	RFIFOSKIP(fd,18);
	return;
	#endif // CLOWNPHOBIA


	acc = RFIFOL(fd,2);
	sql_query("SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
	if (sql_res) {
		sql_fetch_row(); // row fetching
	}
	tmptime = atol(sql_row[0]);
	if (tmptime == 0 || tmptime < time(NULL))
		timestamp = time(NULL);
	else
		timestamp = tmptime;
	tmtime = localtime(&timestamp);
	tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
	tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
	tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
	tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
	tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
	tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);
	timestamp = mktime(tmtime);
	if (timestamp != -1) {
		if (timestamp <= time(NULL))
			timestamp = 0;
		if (tmptime != timestamp) {
			if (timestamp != 0) {
				unsigned char buf[16];
				WBUFW(buf,0) = 0x2731;
				WBUFL(buf,2) = acc;
				WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
				WBUFL(buf,7) = timestamp; // status or final date of a banishment
				charif_sendallwos(-1, buf, 11);
			}
			ShowMessage("Account: %d Banned until: %ld\n", acc, timestamp);
			sql_query("UPDATE `%s` SET `ban_until` = '%ld', `state`='7' WHERE `%s` = '%d'", login_db, (unsigned long)timestamp, login_db_account_id, acc);
		}
	}
	sql_free();
	RFIFOSKIP(fd,18);
}



void Parse_Char_StatusChange(int id, int fd){
	int acc, statut;

	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this.
	RFIFOSKIP(fd,10);
	return;
	#endif // CLOWNPHOBIA

	acc = RFIFOL(fd,2);
	statut = RFIFOL(fd,6);
	sql_query("SELECT `state` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, acc);
	if (sql_res) {
		sql_fetch_row(); // row fetching
	}
	if (atoi(sql_row[0]) != statut && statut != 0) {
		unsigned char buf[16];
		WBUFW(buf,0) = 0x2731;
		WBUFL(buf,2) = acc;
		WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
		WBUFL(buf,7) = statut; // status or final date of a banishment
		charif_sendallwos(-1, buf, 11);
	}
	sql_query("UPDATE `%s` SET `state` = '%d' WHERE `%s` = '%d'", login_db, statut,login_db_account_id,acc);
	//query
	sql_free();
	RFIFOSKIP(fd,10);
}

void Parse_Char_SexChange(int id, int fd){
	int acc,sex;
	unsigned char buf[16];

	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this
	RFIFOSKIP(fd,6);
	return;
	#endif //CLOWNPHOBIA

	acc=RFIFOL(fd,2);
	sql_query("SELECT `sex` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
	if (sql_res){
		if (sql_num_rows() != 0){
			sql_fetch_row();	//row fetching
			if (strcasecmp(sql_row[0], "M") == 0)
				sex = 1;
			else
				sex = 0;
			sql_query("UPDATE `%s` SET `sex` = '%c' WHERE `%s` = '%d'",
				login_db, (sex==0?'M':'F'), login_db_account_id, acc);
			WBUFW(buf,0) = 0x2723;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			charif_sendallwos(-1, buf, 7);
		}
	}
	sql_free();
	RFIFOSKIP(fd,6);
}

void Parse_Char_SaveAccount(int id, int fd){
	int acc,p,j;
	char str[32];
	char temp_str[32];
	int value;

	#ifdef CLOWNPHOBIA
	RFIFOSKIP(fd,RFIFOW(fd,2));
	return;
	#endif //CLOWNPHOBIA


	acc=RFIFOL(fd,4);
	if (acc>0){
		CREATE_BUFFER(buf,unsigned char, (unsigned short)RFIFOW(fd,2)+1);
		for(p=8,j=0;p<RFIFOW(fd,2) && j<ACCOUNT_REG2_NUM;p+=36,j++){
			memcpy(str,RFIFOP(fd,p),32);
			value=RFIFOL(fd,p+32);
			sql_query("DELETE FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%d' AND `str`='%s';",acc,jstrescapecpy(temp_str,str));
			sql_query("INSERT INTO `global_reg_value` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , '%s' , '%d');",  acc, jstrescapecpy(temp_str,str), value);
		}
		// Send to char
		memcpy(WBUFP(buf,0),RFIFOP(fd,0),RFIFOW(fd,2));
		WBUFW(buf,0)=0x2729;
		charif_sendallwos(fd,buf,WBUFW(buf,2));
		DELETE_BUFFER(buf);
	}
	//ShowMessage("login: save account_reg (from char)\n");
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void Parse_Char_Unban(int id, int fd){
	int acc;
	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this
	RFIFOSKIP(fd,6);
	return;
	#endif //CLOWNPHOBIA

	acc = RFIFOL(fd,2);
	sql_query("SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
	if (sql_res) {
		sql_fetch_row();	//row fetching
		if (atol(sql_row[0]) != 0)
			sql_query("UPDATE `%s` SET `ban_until` = '0', `state`='0' WHERE `%s` = '%d'", login_db,login_db_account_id,acc);
	}
	sql_free();

	RFIFOSKIP(fd,6);
}

void Parse_Char_SetOffline(int id, int fd){
	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this.
	RFIFOSKIP(fd,6);
	return;
	#endif // CLOWNPHOBIA

	remove_online_user(RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
}

void Parse_Char_SetOnline(int id, int fd){
	#ifdef CLOWNPHOBIA // Clownphobia doesnt support this.
	RFIFOSKIP(fd,6);
	return;
	#endif // CLOWNPHOBIA

	add_online_user(RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
}

//-----------------------------------------------------
// Char Server Packet Parse
//-----------------------------------------------------
int parse_fromchar(int fd){
	size_t i, id;
	unsigned long client_ip = session[fd]->client_ip;
	char ip_str[16];
	sprintf(ip_str, "%ld.%ld.%ld.%ld",
			(session[fd]->client_ip>>24)&0xFF,
			(session[fd]->client_ip>>16)&0xFF,
			(session[fd]->client_ip>>8)&0xFF,
			(session[fd]->client_ip)&0xFF);

	for(id = 0; id < MAX_SERVERS; id++)
		if( server[id].fd == fd )
			break;
	if (id == MAX_SERVERS)
		session_Remove(fd);
	//else fd is server[id].fd

	if( !session_isActive(fd) ){
		ShowMessage("Char-server '%s' has disconnected.\n", server[id].name);
		memset(&server[id], 0, sizeof(struct mmo_char_server));
		// server delete
		sql_query("DELETE FROM `sstatus` WHERE `index`='%d'", id);
		// query
		server[id].fd = -1;
		session_Remove(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
		ShowMessage("char_parse: %d %d packet case=%x\n", fd, RFIFOREST(fd), (unsigned short)RFIFOW(fd, 0));

		switch (RFIFOW(fd,0)) {
		case 0x2712:
			if (RFIFOREST(fd) < 19) return 0;
			Parse_Char_Connect(id,fd);
			break;

		case 0x2714: // Set user count
			if (RFIFOREST(fd) < 6) return 0;
			Parse_Char_UserCount(id,fd);
			break;

		// We receive an e-mail/limited time request, because a player comes back from a map-server to the char-server
		case 0x2716:
			if (RFIFOREST(fd) < 6) return 0;

			Parse_Char_PlayerReturn(id,fd);
			break;

		// login-server alive packet reply
		case 0x2718:
			if (RFIFOREST(fd) < 2)  return 0;
			Parse_Char_Alive(id,fd);
			break;

		case 0x2720:	// GM
			if (RFIFOREST(fd) < 4) return 0;
			if (RFIFOREST(fd) < RFIFOW(fd,2)) return 0;
			Parse_Char_GM(id,fd);
			break;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86) return 0;
			Parse_Char_ChangeEmail(id,fd);
			break;

		case 0x2724:	// Receiving of map-server via char-server a status change resquest (by Yor)
			if (RFIFOREST(fd) < 10) return 0;
			Parse_Char_StatusChange(id,fd);
			break;

		case 0x2725: // Receiving of map-server via char-server a ban resquest (by Yor)
			if (RFIFOREST(fd) < 18) return 0;
			Parse_Char_Ban(id,fd);
			break;

		case 0x2727:
			if (RFIFOREST(fd) < 6) return 0;
			Parse_Char_SexChange(id,fd);
			break;

		case 0x2728:	// save account_reg
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2)) return 0;
			Parse_Char_SaveAccount(id,fd);
			break;

		case 0x272a:	// Receiving of map-server via char-server a unban resquest (by Yor)
			if (RFIFOREST(fd) < 6) return 0;
			Parse_Char_Unban(id,fd);
			break;

		case 0x272b:    // Set account_id to online [Wizputer]
			if (RFIFOREST(fd) < 6) return 0;
			Parse_Char_SetOnline(id,fd);
			break;
		case 0x272c:   // Set account_id to offline [Wizputer]
			if (RFIFOREST(fd) < 6) return 0;
			Parse_Char_SetOffline(id,fd);
			break;

		default:
			ShowWarning("login: unknown packet %x! (from Char ip: %s name: %s) ."RETCODE, (unsigned short)RFIFOW(fd,0),ip_str,server[id].name);
			session_Remove(fd);
			return 0;
		}
	}
	return 0;
}


//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int IPBanCheck(unsigned char * p){
	if (ipban > 0)
	{	//ip ban
		//p[0], p[1], p[2], p[3]
		//request DB connection
		sql_query("SELECT count(*) FROM `ipbanlist` WHERE `list` = '%d.*.*.*' OR `list` = '%d.%d.*.*' OR `list` = '%d.%d.%d.*' OR `list` = '%d.%d.%d.%d'",
		  p[0], p[0], p[1], p[0], p[1], p[2], p[0], p[1], p[2], p[3]);

		sql_fetch_row();	//row fetching
		if (atoi(sql_row[0]) >0) {	// ip ban ok.
			ShowMessage ("packet from banned ip : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);

			if (log_login){
				sql_query("INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', 'unknown','-3', 'ip banned')", loginlog_db, p[0], p[1], p[2], p[3]);
			}
			ShowMessage ("close session connection...\n");
			// close connection
			sql_free();
			return 1;

		} else {

			ShowMessage ("packet from ip (ban check ok) : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);
		}

		sql_free();
	}
	return 0;
}

void ClientConnectIsGM(int gm_level,char * userid){
	if (gm_level)
		ShowMessage("Connection of the GM (level:%d) account '%s' accepted.\n", gm_level, userid);
	else
		ShowMessage("Connection of the account '%s' accepted.\n", userid);
}

void Parse_Login_ClientConnect(int fd){
	struct mmo_account account;
	int result, i;
	char t_uid[100];
	unsigned long client_ip = session[fd]->client_ip;
	unsigned char p[] = {(unsigned char)(client_ip>>24)&0xFF,(unsigned char)(client_ip>>16)&0xFF,(unsigned char)(client_ip>>8)&0xFF,(unsigned char)(client_ip)&0xFF};

	account.version = RFIFOL(fd, 2);
	account.userid = (char*)RFIFOP(fd, 6);
	account.passwd = (char*)RFIFOP(fd, 30);
	account.passwdenc=0;

	result=mmo_auth(&account, fd);

	jstrescapecpy(t_uid,(char*)RFIFOP(fd, 6));
	if(result==-1)
	{
		int gm_level = isGM(account.account_id);
		if (min_level_to_connect > gm_level)
		{
			WFIFOW(fd,0) = 0x81;
			WFIFOL(fd,2) = 1; // 01 = Server closed
			WFIFOSET(fd,3);
		} else {
			ClientConnectIsGM(gm_level,account.userid);

			server_num=0;
			for(i = 0; i < MAX_SERVERS; i++)
			{
				if( server[i].fd >= 0){
// This seriously needs fixing, Wan is a blank marker, using LANIP for the meantime.
//							if( server[i].address.isLAN(client_ip) ){
						WFIFOLIP(fd,47+server_num*32) = server[i].address.LANIP();
						WFIFOW(fd,47+server_num*32+4) = server[i].address.LANPort();
//							}else{
//								WFIFOLIP(fd,47+server_num*32) = server[i].address.WANIP();
//								WFIFOW(fd,47+server_num*32+4) = server[i].address.WANPort();
//							}
					ShowMessage("Name: %s ",server[i].name);
					memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,47+server_num*32+26) = server[i].users;
					WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,47+server_num*32+30) = server[i].new_;
					server_num++;
				}
			}
			// if at least 1 char-server
			if (server_num > 0) {
				WFIFOW(fd,0)=0x69;
				WFIFOW(fd,2)=47+32*server_num;
				WFIFOL(fd,4)=account.login_id1;
				WFIFOL(fd,8)=account.account_id;
				WFIFOL(fd,12)=account.login_id2;
				WFIFOL(fd,16)=0;
				memcpy(WFIFOP(fd,20),account.lastlogin,24);
				WFIFOB(fd,46)=account.sex;
				WFIFOSET(fd,47+32*server_num);
				if(auth_fifo_pos>=AUTH_FIFO_SIZE)
					auth_fifo_pos=0;
				auth_fifo[auth_fifo_pos].account_id=account.account_id;
				auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
				auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
				auth_fifo[auth_fifo_pos].sex=account.sex;
				auth_fifo[auth_fifo_pos].delflag=0;
				auth_fifo[auth_fifo_pos].ip = client_ip;
				auth_fifo_pos++;
			} else {
				WFIFOW(fd,0) = 0x81;
				WFIFOL(fd,2) = 1; // 01 = Server closed
				WFIFOSET(fd,3);
			}
		}
	}
	else
	{
		const char *error;
		switch((result + 1)) {
		case -2:  //-3 = Account Banned
			error = "Account banned.";
			break;
		case -1:  //-2 = Dynamic Ban
			error="dynamic ban (ip and account).";
			break;
		case 1:   // 0 = Unregistered ID
			error="Unregisterd ID.";
			break;
		case 2:   // 1 = Incorrect Password
			error="Incorrect Password.";
			break;
		case 3:   // 2 = This ID is expired
			error="Account Expired.";
			break;
		case 4:   // 3 = Rejected from Server
			error="Rejected from server.";
			break;
		case 5:   // 4 = You have been blocked by the GM Team
			error="Blocked by GM.";
			break;
		case 6:   // 5 = Your Game's EXE file is not the latest version
			error="Not latest game EXE.";
			break;
		case 7:   // 6 = Your are Prohibited to log in until %s
			error="Banned.";
			break;
		case 8:   // 7 = Server is jammed due to over populated
			error="Server Over-population.";
			break;
		case 9:   // 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
			error=" ";
			break;
		case 100: // 99 = This ID has been totally erased
			error="Account gone.";
			break;
		default:
			error="Unknown Error.";
			break;
		}
		if(log_login)
		{
			sql_query("INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s', '%d','login failed : %s')", loginlog_db, p[0], p[1], p[2], p[3], t_uid, result, error);
		}
		if((result == 1) && (dynamic_pass_failure_ban != 0))
		{	// failed password
			sql_query("SELECT count(*) FROM `%s` WHERE `ip` = '%d.%d.%d.%d' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
				loginlog_db, p[0], p[1], p[2], p[3], dynamic_pass_failure_ban_time);	//how many times filed account? in one ip.
			//check query result
			sql_fetch_row();	//row fetching

			if (atoi(sql_row[0]) >= dynamic_pass_failure_ban_how_many )
			{
				sql_query("INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[0], p[1], p[2], dynamic_pass_failure_ban_how_long, t_uid);
			}
			sql_free();
		}
		else if (result == -2)
		{	//dynamic banned - add ip to ban list.
			sql_query("INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL 1 MONTH ,'Dynamic banned user id : %s')", p[0], p[1], p[2], t_uid);
			result = -3;
		}
		else if(result == 6)
		{	//not lastet version ..
			//result = 5;
		}

		sql_query("SELECT `ban_until` FROM `%s` WHERE %s `%s` = '%s'",login_db, case_sensitive ? "BINARY" : "",login_db_userid, t_uid);
		if (sql_res)
		{
			sql_fetch_row();	//row fetching
		}
		//cannot connect login failed
		memset(WFIFOP(fd,0),'\0',23);
		WFIFOW(fd,0)=0x6a;
		WFIFOB(fd,2)=result;

		if(result == 6)
		{	// 6 = Your are Prohibited to log in until %s
			if (atol(sql_row[0]) != 0)
			{	// if account is banned, we send ban timestamp
				char tmpstr[256];
				time_t ban_until_time;
				ban_until_time = atol(sql_row[0]);
				strftime(tmpstr, 20, date_format, localtime(&ban_until_time));
				tmpstr[19] = '\0';
				memcpy(WFIFOP(fd,3), tmpstr, 20);
			}
			else
			{	// we send error message
				memcpy(WFIFOP(fd,3), error, 20);
			}
		}
		WFIFOSET(fd,23);
		sql_free();
	}
	RFIFOSKIP(fd,55);
}

void Parse_Login_CharServerConnect(int fd){
	int result;
	unsigned char* server_name;
	struct mmo_account account;
	char t_uid[100];
	unsigned long client_ip = session[fd]->client_ip;
	unsigned char p[] = {(unsigned char)(client_ip>>24)&0xFF,(unsigned char)(client_ip>>16)&0xFF,(unsigned char)(client_ip>>8)&0xFF,(unsigned char)(client_ip)&0xFF};

	if(log_login){
		sql_query("INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s@%s','100', 'charserver - %s@%d.%d.%d.%d:%d')", loginlog_db, p[0], p[1], p[2], p[3], RFIFOP(fd, 2),RFIFOP(fd, 60),RFIFOP(fd, 60), (unsigned char)RFIFOB(fd, 54), (unsigned char)RFIFOB(fd, 55), (unsigned char)RFIFOB(fd, 56), (unsigned char)RFIFOB(fd, 57), (unsigned short)RFIFOW(fd, 58));
	}

	ShowMessage("server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
		RFIFOP(fd, 60), (unsigned char)RFIFOB(fd, 74), (unsigned char)RFIFOB(fd, 75), (unsigned char)RFIFOB(fd, 76), (unsigned char)RFIFOB(fd, 77), (unsigned short)RFIFOW(fd, 82),	p[0], p[1], p[2], p[3]);

	account.userid = (char*)RFIFOP(fd, 2);
	account.passwd = (char*)RFIFOP(fd, 26);
	account.passwdenc = 0;
	server_name = RFIFOP(fd,50);
	result = mmo_auth(&account, fd);
	//ShowMessage("Result: %d - Sex: %d - Account ID: %d\n",result,account.sex,(int) account.account_id);

	if( result == -1 && account.sex==2 && account.account_id<MAX_SERVERS && server[account.account_id].fd==-1)
	{
		ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
		memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));

		memcpy(server[account.account_id].name,RFIFOP(fd,50),20);
		server[account.account_id].maintenance=RFIFOB(fd,70);
		server[account.account_id].new_=RFIFOB(fd,71);
		// wanip,wanport,lanip,lanmask,lanport
		server[account.account_id].address = ipset(	RFIFOLIP(fd,74), RFIFOLIP(fd,78), RFIFOW(fd,82), RFIFOLIP(fd,84), RFIFOW(fd,88) );

		server[account.account_id].users=0;
		server[account.account_id].fd = fd;


		sql_query("DELETE FROM `sstatus` WHERE `index`='%ld'", account.account_id);

		jstrescapecpy(t_uid,server[account.account_id].name);

		sql_query("INSERT INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%ld', '%s', '%d')",account.account_id, server[account.account_id].name,0);

		WFIFOW(fd,0)=0x2711;
		WFIFOB(fd,2)=0;
		WFIFOSET(fd,3);
		session[fd]->func_parse=parse_fromchar;
		realloc_fifo(fd,FIFOSIZE_SERVERLINK,FIFOSIZE_SERVERLINK);
	}
	else
	{
		WFIFOW(fd, 0) =0x2711;
		WFIFOB(fd, 2)=3;
		WFIFOSET(fd, 3);
	}
	RFIFOSKIP(fd, 90);
}

void SendAthenaVersion(int fd){
	WFIFOW(fd,0)=0x7531;
	WFIFOB(fd,2)=ATHENA_MAJOR_VERSION;
	WFIFOB(fd,3)=ATHENA_MINOR_VERSION;
	WFIFOB(fd,4)=ATHENA_REVISION;
	WFIFOB(fd,5)=ATHENA_RELEASE_FLAG;
	WFIFOB(fd,6)=ATHENA_OFFICIAL_FLAG;
	WFIFOB(fd,7)=ATHENA_SERVER_LOGIN;
	WFIFOW(fd,8)=ATHENA_MOD_VERSION;
	WFIFOSET(fd,10);
	RFIFOSKIP(fd,2);
	ShowMessage ("Athena version check...\n");
}

int parse_login(int fd){
	int i;
	char ip_str[16];


	struct mmo_account account;
	char t_uid[100];
	unsigned long client_ip = session[fd]->client_ip;
	unsigned char p[] = {(unsigned char)(client_ip>>24)&0xFF,(unsigned char)(client_ip>>16)&0xFF,(unsigned char)(client_ip>>8)&0xFF,(unsigned char)(client_ip)&0xFF};
	sprintf(ip_str, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);


	if (IPBanCheck(p)) session_Remove(fd);

	if ( !session_isActive(fd) )
		session_Remove(fd);// have it removed by do_sendrecv

	if( session_isRemoved(fd) )
	{
		for(i = 0; i < MAX_SERVERS; i++)
			if(server[i].fd == fd)
				server[i].fd = -1;
		return 0;
	}

	while(RFIFOREST(fd)>=2){
		ShowMessage("parse_login : %d %d packet case=%x\n", fd, RFIFOREST(fd), (unsigned short)RFIFOW(fd,0));

		switch(RFIFOW(fd,0)){
		case 0x200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26) return 0;
			RFIFOSKIP(fd,26);
			break;

		case 0x204:		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
			if (RFIFOREST(fd) < 18) return 0;
			RFIFOSKIP(fd,18);
			break;

		case 0x64:		// request client login
			if(RFIFOREST(fd) < 55) return 0;
			Parse_Login_ClientConnect(fd);
			break;

		case 0x2710:	// request Char-server connection
			if(RFIFOREST(fd) < 90) return 0;
			Parse_Login_CharServerConnect(fd);
		break;

		case 0x7530:	// request Athena information
			SendAthenaVersion(fd);
			break;

		case 0x7532:
		default:
			ShowMessage ("End of connection (ip: %s)" RETCODE, ip_str);
			session_Remove(fd);
			return 0;
		}//end case
	}//end while
	return 0;
}

// Console Command Parser [Wizputer]
int parse_console(char *buf) {
    char type[64];
	char command[64];

    ShowMessage("Console: %s\n",buf);

    if ( sscanf(buf, "%[^:]:%[^\n]", type , command ) < 2 )
        sscanf(buf,"%[^\n]",type);

    ShowMessage("Type of command: %s || Command: %s \n",type,command);

    if(buf) aFree(buf);
    return 0;
}

//-----------------------------------------------------
//BANNED IP CHECK.
//-----------------------------------------------------
int ip_ban_check(int tid, unsigned long tick, int id, int data){

	//query
	sql_query("DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()",NULL);
	return 0;
}

//-----------------------------------------------------
// reading configuration
//-----------------------------------------------------
int login_config_read(const char *cfgName){
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=safefopen(cfgName,"r");

	if(fp==NULL){
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	ShowStatus("Reading Login Configuration %s\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if( !skip_empty_line(line) )
			continue;

		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		else if (strcasecmp(w1, "login_ip") == 0) {
			loginaddress = w2;
			ShowMessage("Login server IP address: %s -> %s\n", w2, loginaddress.getstring());
		}
		else if(strcasecmp(w1,"login_port")==0){
			loginaddress.port()=atoi(w2);
		}

		//account ban -> ip ban
		else if(strcasecmp(w1,"dynamic_account_ban")==0){
			dynamic_account_ban=atoi(w2);
			ShowMessage ("set dynamic_account_ban : %d\n",dynamic_account_ban);
		}
		else if(strcasecmp(w1,"dynamic_account_ban_class")==0){
			dynamic_account_ban_class=atoi(w2);
			ShowMessage ("set dynamic_account_ban_class : %d\n",dynamic_account_ban_class);
		}
		//dynamic password error ban
		else if(strcasecmp(w1,"dynamic_pass_failure_ban")==0){
			dynamic_pass_failure_ban=atoi(w2);
			ShowMessage ("set dynamic_pass_failure_ban : %d\n",dynamic_pass_failure_ban);
		}
		else if(strcasecmp(w1,"dynamic_pass_failure_ban_time")==0){
			dynamic_pass_failure_ban_time=atoi(w2);
			ShowMessage ("set dynamic_pass_failure_ban_time : %d\n",dynamic_pass_failure_ban_time);
		}
		else if(strcasecmp(w1,"dynamic_pass_failure_ban_how_many")==0){
			dynamic_pass_failure_ban_how_many=atoi(w2);
			ShowMessage ("set dynamic_pass_failure_ban_how_many : %d\n",dynamic_pass_failure_ban_how_many);
		}
		else if(strcasecmp(w1,"dynamic_pass_failure_ban_how_long")==0){
			dynamic_pass_failure_ban_how_long=atoi(w2);
			ShowMessage ("set dynamic_pass_failure_ban_how_long : %d\n",dynamic_pass_failure_ban_how_long);
		}
		else if (strcasecmp(w1, "import") == 0) {
			login_config_read(w2);
		} else if(strcasecmp(w1, "new_account") == 0){ 	//Added by Sirius for new account _M/_F
			new_account_flag = atoi(w2);		//Added by Sirius for new account _M/_F
		} else if(strcasecmp(w1, "check_client_version") == 0) {
			check_client_version = config_switch(w2);
		} else if(strcasecmp(w1, "client_version_to_connect") == 0){	//Added by Sirius for client version check
			client_version_to_connect = atoi(w2);			//Added by SIrius for client version check
		} else if(strcasecmp(w1,"use_MD5_passwords")==0){
			use_md5_passwds=config_switch(w2);
			ShowMessage ("Using MD5 Passwords: %s \n",w2);
			}
        else if (strcasecmp(w1, "date_format") == 0) { // note: never have more than 19 char for the date!
				switch (atoi(w2)) {
				case 0:
					strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
				}
		}
        else if (strcasecmp(w1, "min_level_to_connect") == 0) {
				min_level_to_connect = atoi(w2);
		}
        else if (strcasecmp(w1, "check_ip_flag") == 0) {
                check_ip_flag = config_switch(w2);
    	}
    	else if (strcasecmp(w1, "console") == 0) {
			console = config_switch(w2);
        }
    	else if (strcasecmp(w1, "case_sensitive") == 0) {
			case_sensitive = config_switch(w2);
        }
		else if(strcasecmp(w1, "register_users_online") == 0) {
			register_users_online = config_switch(w2);
		} else if (strcasecmp(w1, "allowed_regs") == 0) {
			allowed_regs = atoi(w2);
		} else if (strcasecmp(w1, "time_allowed") == 0) {
			time_allowed = atoi(w2);
		}
 	}
	fclose(fp);
	return 0;
}

void sql_config_read(const char *cfgName){ /* Kalaspuff, to get login_db */
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp=safefopen(cfgName,"r");
	if(fp==NULL){
		ShowError("sql config file not found: %s\n",cfgName);
		return;
	}
	ShowMessage("reading configure: %s\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if( !skip_empty_line(line) )
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if (strcasecmp(w1, "login_db") == 0) {
			safestrcpy(login_db, w2,sizeof(login_db));
			ShowMessage ("set login_db : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_db_account_id")==0){
			safestrcpy(login_db_account_id, w2,sizeof(login_db_account_id));
			ShowMessage ("set login_db_account_id : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_db_userid")==0){
			safestrcpy(login_db_userid, w2,sizeof(login_db_userid));
			ShowMessage ("set login_db_userid : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_db_user_pass")==0){
			safestrcpy(login_db_user_pass, w2,sizeof(login_db_user_pass));
			ShowMessage ("set login_db_user_pass : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_db_level")==0){
			safestrcpy(login_db_level, w2, sizeof(login_db_level));
			ShowMessage ("set login_db_level : %s\n",w2);
		}


		//add for DB connection
		else if(strcasecmp(w1,"login_server_ip")==0){
			safestrcpy(login_server_ip, w2,sizeof(login_server_ip));
			ShowMessage ("set login_server_ip : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_port")==0){
			login_server_port=atoi(w2);
			ShowMessage ("set login_server_port : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_id")==0){
			safestrcpy(login_server_id, w2,sizeof(login_server_id));
			ShowMessage ("set login_server_id : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_pw")==0){
			safestrcpy(login_server_pw, w2,sizeof(login_server_pw));
			ShowMessage ("set login_server_pw : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_db")==0){
			safestrcpy(login_server_db, w2,sizeof(login_server_db));
			ShowMessage ("set login_server_db : %s\n",w2);
		} //added for custom column names for custom login table
		//end of custom table config
		else if (strcasecmp(w1, "loginlog_db") == 0) {
			safestrcpy(loginlog_db, w2,sizeof(loginlog_db));
		}
		else if (strcasecmp(w1, "log_login") == 0) {
			log_login = config_switch(w2);
		}
		//support the import command, just like any other config
		else if(strcasecmp(w1,"import")==0){
			sql_config_read(w2);
		}
	}
	fclose(fp);
	ShowMessage("reading configure done.....\n");
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
int online_db_final(void *key,void *data,va_list ap)
{
	int *p = (int *) data;
	if (p) aFree(p);
	return 0;
}
void do_final(void) {
	//sync account when terminating.
	//but no need when you using DBMS (sql)
	mmo_db_close();
	numdb_final(online_db, online_db_final);
}

unsigned char getServerType()
{
	return ATHENA_SERVER_LOGIN | ATHENA_SERVER_CORE;
}
int do_init(int argc,char **argv){
	//initialize login server
	int i;

	//read login configue
	login_config_read( (argc>1)?argv[1]:LOGIN_CONF_NAME );
	sql_config_read(SQL_CONF_NAME);


	ShowMessage ("set FIFO Size\n");
	for(i=0;i<AUTH_FIFO_SIZE;i++)
		auth_fifo[i].delflag=1;
	ShowMessage ("set FIFO Size complete\n");

	ShowMessage ("set max servers...");
	for(i=0;i<MAX_SERVERS;i++)
		server[i].fd=-1;
	ShowMessage ("complete\n");

	//server port open & binding
	login_fd=make_listen(loginaddress.addr(),loginaddress.port());

	//Auth start
	ShowMessage ("Running mmo_auth_sqldb_init()\n");
	mmo_auth_sqldb_init();
	ShowMessage ("finished mmo_auth_sqldb_init()\n");

	//set default parser as parse_login function
	set_defaultparse(parse_login);

	// ban deleter timer - 1 minute term
	ShowMessage("add interval tic (ip_ban_check)....\n");
	add_timer_func_list(ip_ban_check,"ip_ban_check");
	i=add_timer_interval(gettick()+10, 60*1000, ip_ban_check,0,0);

	if (console) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	// Online user database init
    if(online_db) aFree(online_db);
	online_db = numdb_init();

	ShowStatus("The login-server is "CL_BT_GREEN"ready"CL_NORM" (Server is listening on port %d).\n", loginaddress.port());

	return 0;
}


