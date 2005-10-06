#include "basesq.h"

#ifndef TXT_ONLY

//////////////////////////////////////////////////////////////////////////////////////
// CMySQL Class
//////////////////////////////////////////////////////////////////////////////////////
// CMySQL Class Constructor
// Code is executed everytime a CMySQL var is initialized
CMySQL::CMySQL() {
	safestrcpy(mysqldb_ip, "127.0.0.1", sizeof(mysqldb_ip));
	mysqldb_port=3306;
	safestrcpy(mysqldb_id, "ragnarok", sizeof(mysqldb_id));
	safestrcpy(mysqldb_pw, "ragnarok", sizeof(mysqldb_pw));
	safestrcpy(mysqldb_db, "ragnarok", sizeof(mysqldb_db));
}

// CMySQL Class Destructor
// Code is executed a CMySQL var is destroyed
inline CMySQL::~CMySQL() {
}

// Send a MySQL query to get data
bool CMySQL::mysql_SendQuery(MYSQL_RES*& sql_res, const char* q, size_t sz) {

	#ifdef DEBUG_SQL
		ShowSQL("%s\n", q);
	#endif

	if( 0==mysql_real_query(&mysqldb_handle, q, (sz)?sz:strlen(q)) ) {
		if(sql_res) mysql_free_result(sql_res);

		sql_res = mysql_store_result(&mysqldb_handle);

		if(sql_res)
			return true;
		else
			ShowError("DB result error\nQuery:    %s\n", q);
	}
	else
		ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&mysqldb_handle), q);

	return false;
}

// Send a MySQL query to send data
bool CMySQL::mysql_SendQuery(const char* q, size_t sz) {

	#ifdef DEBUG_SQL
		ShowSQL("%s\n", q);
	#endif

	if( 0==mysql_real_query(&mysqldb_handle, q, (sz)?sz:strlen(q)) )
		return true;
	else
		ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&mysqldb_handle), q);

	return false;
}

// Make string MySQL safe
inline const char* CMySQL::escape_string(char *target, const char* source, size_t len) {

	mysql_real_escape_string(&mysqldb_handle, target, source, len);

	return target;
}

//////////////////////////////////////////////////////////////////////////////////////
// CAccountDB_sql Class
//////////////////////////////////////////////////////////////////////////////////////
CAccountDB_sql::CAccountDB_sql(const char* configfile)
{
	safestrcpy(login_auth_db,"login_auth",sizeof(login_auth_db));
	safestrcpy(login_reg_db,"login_reg",sizeof(login_reg_db));
	safestrcpy(login_status_db,"login_status",sizeof(login_status_db));
	safestrcpy(login_log_db,"login_log",sizeof(login_log_db));

	case_sensitive=true;
	log_login=false;
	init(configfile);
}

bool CAccountDB_sql::ProcessConfig(const char*w1, const char*w2)
{
	if(w1 && w2)
	{
		// All Tables in Database
		if (strcasecmp(w1, "login_auth_db") == 0) {
			safestrcpy(login_auth_db, w2,sizeof(login_auth_db));
		}
		else if (strcasecmp(w1, "login_reg_db") == 0) {
			safestrcpy(login_reg_db, w2,sizeof(login_reg_db));
		}
		else if (strcasecmp(w1, "login_log_db") == 0) {
			safestrcpy(login_log_db, w2,sizeof(login_log_db));
		}
		else if (strcasecmp(w1, "login_status_db") == 0) {
			safestrcpy(login_status_db, w2,sizeof(login_status_db));
		}

		// Database Connection Settings
		else if(strcasecmp(w1,"login_server_ip")==0){
			safestrcpy(this->mysqldb_ip, w2, sizeof(this->mysqldb_ip));
			ShowMessage ("set login_server_ip : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_port")==0){
			this->mysqldb_port=atoi(w2);
			ShowMessage ("set login_server_port : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_id")==0){
			safestrcpy(this->mysqldb_id, w2,sizeof(this->mysqldb_id));
			ShowMessage ("set login_server_id : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_pw")==0){
			safestrcpy(this->mysqldb_pw, w2,sizeof(this->mysqldb_pw));
			ShowMessage ("set login_server_pw : %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_db")==0){
			safestrcpy(this->mysqldb_db, w2,sizeof(this->mysqldb_db));
			ShowMessage ("set login_server_db : %s\n",w2);
		}

		//end of custom table config
		else if (strcasecmp(w1, "log_login") == 0) {
			log_login = Switch(w2);
		}
	}
	return true;
}

bool CAccountDB_sql::existAccount(const char* userid)
{	// check if account with userid already exist
	bool ret = false;
	if(userid)
	{
		char uid[64];
		char query[1024];
		MYSQL_RES* sql_res=NULL;

		escape_string(uid, userid, sizeof(userid));
		size_t sz=sprintf(query, "SELECT `userid` FROM `%s` WHERE %s `userid` = '%s'", login_auth_db, case_sensitive ? "BINARY" : "", uid);
		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			ret = (mysql_num_rows(sql_res) == 1);
			mysql_free_result(sql_res);
		}
	}
	return ret;
}

bool CAccountDB_sql::searchAccount(const char* userid, CLoginAccount& account)
{	// get account by user/pass
	bool ret = false;
	if(userid)
	{
		size_t sz;
		char query[4096];
		char uid[64];
		MYSQL_RES *sql_res1=NULL, *sql_res2=NULL;
//!! get auth from db
		escape_string(uid, userid, strlen(userid));
		sz=sprintf(query, "SELECT `account_id`,`userid`,`user_pass`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`level`,`email`"
						" FROM `%s` WHERE %s `userid`='%s'", login_auth_db, case_sensitive ? "BINARY" : "", uid);
		//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state/10-gmlevel/11-email}
		if( this->mysql_SendQuery(sql_res1, query, sz) )
		{
			MYSQL_ROW sql_row = mysql_fetch_row(sql_res1);	//row fetching
			if(sql_row)
			{

				account.account_id = sql_row[0]?atol(sql_row[0]):0;
				safestrcpy(account.userid, userid, 24);
				safestrcpy(account.passwd, sql_row[2]?sql_row[2]:"", 32);
				safestrcpy(account.last_login, sql_row[3]?sql_row[3]:"" , 24);
				account.login_count = sql_row[4]?atol( sql_row[4]):0;
				account.sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';
				account.valid_until = (time_t)(sql_row[6]?atol(sql_row[6]):0);
				safestrcpy(account.last_ip, sql_row[7], 16);
				account.ban_until = (time_t)(sql_row[8]?atol(sql_row[8]):0);;
				account.state = sql_row[9]?atol(sql_row[9]):0;;
				account.gm_level = sql_row[10]?atoi( sql_row[10]):0;
				safestrcpy(account.email, sql_row[11]?sql_row[11]:"" , 40);

				sz = sprintf(query, "SELECT `str`,`value` FROM `%s` WHERE `account_id`='%ld'", login_reg_db, (unsigned long)account.account_id);
				if( this->mysql_SendQuery(sql_res2, query, sz) )
				{
					size_t i=0;
					while( i<ACCOUNT_REG2_NUM && (sql_row = mysql_fetch_row(sql_res2)) )
					{
						safestrcpy(account.account_reg2[i].str, sql_row[0], sizeof(account.account_reg2[0].str));
						account.account_reg2[i].value = (sql_row[1]) ? atoi(sql_row[1]):0;
					}
					account.account_reg2_num = i;

					mysql_free_result(sql_res2);
				}
				ret = true;
			}
			mysql_free_result(sql_res1);
		}
	}
	return ret;
}

bool CAccountDB_sql::searchAccount(uint32 accid, CLoginAccount& account)
{	// get account by account_id
	bool ret = false;
	size_t sz;
	char query[4096];
	MYSQL_RES *sql_res1=NULL, *sql_res2=NULL;
//!! get auth from db
	sz=sprintf(query, "SELECT `account_id`,`userid`,`user_pass`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`level`,`email`"
	                " FROM `%s` WHERE `account_id`='%s'", login_auth_db, accid);
	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state/10-gmlevel/11-email}
	if( this->mysql_SendQuery(sql_res1, query, sz) )
	{
		MYSQL_ROW sql_row = mysql_fetch_row(sql_res1);	//row fetching
		if(sql_row)
		{

			account.account_id = sql_row[0]?atol(sql_row[0]):0;
			safestrcpy(account.userid, sql_row[1]?sql_row[1]:"", 24);
			safestrcpy(account.passwd, sql_row[2]?sql_row[2]:"", 32);
			safestrcpy(account.last_login, sql_row[3]?sql_row[3]:"" , 24);
			account.login_count = sql_row[4]?atol( sql_row[4]):0;
			account.sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';
			account.valid_until = (time_t)(sql_row[6]?atol(sql_row[6]):0);
			safestrcpy(account.last_ip, sql_row[7], 16);
			account.ban_until = (time_t)(sql_row[8]?atol(sql_row[8]):0);;
			account.state = sql_row[9]?atol(sql_row[9]):0;;
			account.gm_level = sql_row[10]?atoi( sql_row[10]):0;
			safestrcpy(account.email, sql_row[11]?sql_row[11]:"" , 40);


			sz = sprintf(query, "SELECT `str`,`value` FROM `%s` WHERE `account_id`='%ld'", login_reg_db, (unsigned long)account.account_id);
			if( this->mysql_SendQuery(sql_res2, query, sz) )
			{
				size_t i=0;
				while( i<ACCOUNT_REG2_NUM && (sql_row = mysql_fetch_row(sql_res2)) )
				{
					safestrcpy(account.account_reg2[i].str, sql_row[0], sizeof(account.account_reg2[0].str));
					account.account_reg2[i].value = (sql_row[1]) ? atoi(sql_row[1]):0;
				}
				account.account_reg2_num = i;

				mysql_free_result(sql_res2);
			}
			ret = true;
		}
		mysql_free_result(sql_res1);
	}
	return ret;
}

bool CAccountDB_sql::insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount& account)
{	// insert a new account to db
	size_t sz;
	char uid[64], pwd[64];
	char query[1024];

	escape_string(uid, userid, strlen(userid));
	escape_string(pwd, passwd, strlen(passwd));
	sz = sprintf(query, "INSERT INTO `%s` (`userid`, `user_pass`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')", login_auth_db, uid, pwd, sex, email);
	if( this->mysql_SendQuery(query, sz) )
	{
		// read the complete account back from db
		return searchAccount(userid, account);
	}
	return false;
}

bool CAccountDB_sql::removeAccount(uint32 accid)
{
	bool ret;
	size_t sz;
	char query[1024];

	sz=sprintf(query,"DELETE FROM `%s` WHERE `account_id`='%d';",login_auth_db, accid);
	ret = this->mysql_SendQuery(query, sz);

	sz=sprintf(query,"DELETE FROM `%s` WHERE `account_id`='%d';",login_reg_db, accid); // must update with the variable login_reg_db
	ret &=this->mysql_SendQuery(query, sz);
	return ret;
}

bool CAccountDB_sql::init(const char* configfile)
{	// init db
	size_t sz; // Used for size of queries
	bool wipe=false; // i dont know how a bool is set..
	char query[512]; // used for the queries themselves

	CConfig::LoadConfig(configfile);

	mysql_init(&mysqldb_handle);

	// All tables used for login: login_auth, login_reg, login_log, login_status


	// DB connection start
	ShowMessage("Connect Database Server on %s%u....\n", mysqldb_ip, mysqldb_port);
	if( mysql_real_connect(&mysqldb_handle, mysqldb_ip, mysqldb_id, mysqldb_pw, mysqldb_db, mysqldb_port, (char *)NULL, 0) )
	{
		ShowMessage("connect success!\n");
		if (log_login)
		{
			char query[512];
			size_t sz = sprintf(query, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'Login', '100','login server started')", login_log_db);
			//query
			this->mysql_SendQuery(query, sz);
		}
	}
	else
	{	// pointer check
		ShowMessage("%s\n", mysql_error(&mysqldb_handle));
		return false;
	}

	if (wipe)
	{
		sz = sprintf(query, "DROP TABLE IF EXISTS login_auth");
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
		"CREATE TABLE IF NOT EXISTS `login_auth` ("
		"`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		"`userid` VARCHAR(24) NOT NULL,"
		"`user_pass` VARCHAR(34) NOT NULL,"
		"`sex` ENUM('M','F','S') default 'M',"
		"`gm_level` INT(3) UNSIGNED NOT NULL,"
		"`online` BOOL default 'false',"
		"`email` VARCHAR(40) NOT NULL,"
		"`login_ip` VARCHAR(16) NOT NULL,"
		"`login_id1` INTEGER UNSIGNED NOT NULL,"
		"`login_id2` INTEGER UNSIGNED NOT NULL,"
		"`last_login` INTEGER UNSIGNED NOT NULL,"
		"`ban_until` INTEGER UNSIGNED NOT NULL,"
		"`valid_until` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`account_id`)"
		")" );
	this->mysql_SendQuery(query, sz);





	if (wipe)
	{
		size_t sz = sprintf(query, "DROP TABLE IF EXISTS login_reg");
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
		"CREATE TABLE IF NOT EXISTS `login_reg` ("
		"`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		"`str` VARCHAR(34) NOT NULL,"  // Not sure on the length needed.
		"`value` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`account_id`,`str`)"
		")" );
	this->mysql_SendQuery(query, sz);

	if (wipe)
	{
		size_t sz = sprintf(query, "DROP TABLE IF EXISTS login_log");
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
		"CREATE TABLE IF NOT EXISTS `login_log` ("
		"`time` INTEGER UNSIGNED,"
		"`ip` VARCHAR(16) NOT NULL,"
		"`user` VARCHAR(24) NOT NULL,"
		"`rcode` INTEGER(3) UNSIGNED NOT NULL,"
		"`log` VARCHAR(100) NOT NULL"
		")" );
	this->mysql_SendQuery(query, sz);

	if (wipe)
	{
		sz = sprintf(query, "DROP TABLE IF EXISTS login_sstatus");
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
		"CREATE TABLE IF NOT EXISTS `login_log` ("
		"`index` INTEGER UNSIGNED NOT NULL,"
		"`name` VARCHAR(24) NOT NULL,"
		"`user` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`index`)"
		")" );
	this->mysql_SendQuery(query, sz);

	return true;
}

bool CAccountDB_sql::close()
{
	size_t sz;
	char query[512];
	//set log.
	if (log_login)
	{
		sz = sprintf(query,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", login_log_db);
		this->mysql_SendQuery(query, sz);
	}

	//delete all server status
	sz = sprintf(query,"DELETE FROM `%s`", login_status_db);
	this->mysql_SendQuery(query, sz);

	mysql_close(&mysqldb_handle);

	return true;
}

bool CAccountDB_sql::saveAccount(const CLoginAccount& account)
{
	bool ret = false;
	size_t sz;
	char query[1024], tempstr[64];
//!! store auth in db
	sz = sprintf(query, "UPDATE `%s` SET "
		"`userid` = '%s', "
		"`user_pass` = '%d', "
		"`level` = '%d', "
		"`logincount` = '%ld', "
		"`sex` = '%c', "
		"`last_ip` = '%s', "
		"`connect_until` = '%ld', "
		"`ban_until` = '%ld', "
		"`email` = '%s', "
		"WHERE `account_id` = '%ld'",
		login_auth_db,
		account.userid,
		account.passwd,
		account.gm_level,
		(unsigned long)account.login_count,
		(account.sex==1)? 'M':'F',
		account.last_ip,
		(unsigned long)account.valid_until,
		(unsigned long)account.ban_until,
		account.email,
		(unsigned long)account.account_id);

	ret = this->mysql_SendQuery(query, sz);

	sz=sprintf(query,"DELETE FROM `%s` WHERE `account_id`='%d';",login_reg_db, account.account_id);
	if( this->mysql_SendQuery(query, sz) )
	{
		size_t i;
		for(i=0; i<account.account_reg2_num; i++)
		{
			jstrescapecpy(tempstr,account.account_reg2[i].str);
			sz=sprintf(query,"INSERT INTO `%s` (`account_id`, `str`, `value`) VALUES ( '%d' , '%s' , '%d');",  login_reg_db, account.account_id, tempstr, account.account_reg2[i].value);
			ret &= this->mysql_SendQuery(query, sz);
		}
	}
	return ret;
}



#endif//!TXT_ONLY
