#include "base.h"
#include "baseio.h"
#include "sql.h"

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
	
	tmpSql = new char[65536];
}

// CMySQL Class Destructor
// Code is executed a CMySQL var is destroyed
CMySQL::~CMySQL() {
	delete[] tmpSql;
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
const char* CMySQL::escape_string(char *target, const char* source, size_t len)	{
	
	mysql_real_escape_string(&mysqldb_handle, target, source, len);
	
	return target;
}

//////////////////////////////////////////////////////////////////////////////////////
// CAccountDB_sql Class
//////////////////////////////////////////////////////////////////////////////////////
CAccountDB_sql::CAccountDB_sql(const char* configfile)
{
	safestrcpy(login_db,"login",sizeof(login_db));
	safestrcpy(log_db,"loginlog",sizeof(log_db));
	safestrcpy(login_db_userid,"userid",sizeof(login_db_userid));
	safestrcpy(login_db_account_id,"account_id",sizeof(login_db_account_id));
	safestrcpy(login_db_user_pass,"user_pass",sizeof(login_db_user_pass));
	safestrcpy(login_db_level,"level",sizeof(login_db_level));
	case_sensitive=true;
	log_login=false;

	init(configfile);
}

bool CAccountDB_sql::ProcessConfig(const char*w1, const char*w2)
{
	if(w1 && w2)
	{
		if (strcasecmp(w1, "login_db") == 0) {
			safestrcpy(login_db, w2,sizeof(login_db));
		}
		//add for DB connection
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
			safestrcpy(login_db, w2,sizeof(login_db));
			ShowMessage ("set login_server_db : %s\n",w2);
		}
		//added for custom column names for custom login table
		else if(strcasecmp(w1,"login_db_account_id")==0){
			safestrcpy(login_db_account_id, w2,sizeof(login_db_account_id));
		}
		else if(strcasecmp(w1,"login_db_userid")==0){
			safestrcpy(login_db_userid, w2,sizeof(login_db_userid));
		}
		else if(strcasecmp(w1,"login_db_user_pass")==0){
			safestrcpy(login_db_user_pass, w2,sizeof(login_db_user_pass));
		}
		else if(strcasecmp(w1,"login_db_level")==0){
			safestrcpy(login_db_level, w2, sizeof(login_db_level));
		}
		//end of custom table config
		else if (strcasecmp(w1, "loginlog_db") == 0) {
			safestrcpy(log_db, w2,sizeof(log_db));
		}
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
		size_t sz=sprintf(query, "SELECT `%s` FROM `%s` WHERE %s `%s` = '%s'", login_db_userid, login_db, case_sensitive ? "BINARY" : "", login_db_userid, uid);
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
		sz=sprintf(query, "SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`%s`,`email`"
						" FROM `%s` WHERE %s `%s`='%s'", login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, case_sensitive ? "BINARY" : "", login_db_userid, uid);
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

				sz = sprintf(query, "SELECT `str`,`value` FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%ld'", account.account_id);
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

bool CAccountDB_sql::searchAccount(unsigned long accid, CLoginAccount& account)
{	// get account by account_id
	bool ret = false;
	size_t sz;
	char query[4096];
	MYSQL_RES *sql_res1=NULL, *sql_res2=NULL;
//!! get auth from db
	sz=sprintf(query, "SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`%s`,`email`"
	                " FROM `%s` WHERE `%s`='%s'", login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, login_db_account_id, accid);
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


			sz = sprintf(query, "SELECT `str`,`value` FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%ld'", account.account_id);
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
	sz = sprintf(query, "INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')", login_db, login_db_userid, login_db_user_pass, uid, pwd, sex, email);
	if( this->mysql_SendQuery(query, sz) )
	{
		// read the complete account back from db
		return searchAccount(userid, account);
	}
	return false;
}

bool CAccountDB_sql::removeAccount(unsigned long accid)
{
	bool ret;
	size_t sz;
	char query[1024];

	sz=sprintf(query,"DELETE FROM `%s` WHERE `account_id`='%d';",login_db, accid);
	ret = this->mysql_SendQuery(query, sz);

	sz=sprintf(query,"DELETE FROM `global_reg_value` WHERE `account_id`='%d';",accid);
	ret &=this->mysql_SendQuery(query, sz);
	return ret;
}

bool CAccountDB_sql::init(const char* configfile)
{	// init db
	CConfig::LoadConfig(configfile);

	mysql_init(&mysqldb_handle);
	// DB connection start
	ShowMessage("Connect Login Database Server....\n");
	if( mysql_real_connect(&mysqldb_handle, mysqldb_ip, mysqldb_id, mysqldb_pw, login_db, mysqldb_port, (char *)NULL, 0) )
	{
		ShowMessage("connect success!\n");
		if (log_login)
		{	
			char query[512];
			size_t sz = sprintf(query, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver', '100','login server started')", log_db);
			//query
			this->mysql_SendQuery(query, sz);
		}
		return true;
	}
	else
	{	// pointer check
		ShowMessage("%s\n", mysql_error(&mysqldb_handle));
		return false;
	}
}

bool CAccountDB_sql::close()
{
	size_t sz;
	char query[512];
	//set log.
	if (log_login)
	{
		sz = sprintf(query,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", log_db);
		this->mysql_SendQuery(query, sz);
	}

	//delete all server status
	sz = sprintf(query,"DELETE FROM `sstatus`");
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
		"`%s` = '%s', "
		"`%s` = '%d', "
		"`logincount` = '%ld', "
		"`sex` = '%c', "
		"`last_ip` = '%s', "
		"`connect_until` = '%ld', "
		"`ban_until` = '%ld', "
		"`email` = '%s', "
		"WHERE `%s` = '%ld'", 
		login_db, 
		login_db_user_pass, account.passwd,
		login_db_level, account.gm_level,
		account.login_count,
		(account.sex==1)? 'M':'F',
		account.last_ip,
		(unsigned long)account.valid_until,
		(unsigned long)account.ban_until,
		account.email,
		login_db_account_id, account.account_id);
	ret = this->mysql_SendQuery(query, sz);

	sz=sprintf(query,"DELETE FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%d';",account.account_id);
	if( this->mysql_SendQuery(query, sz) )
	{	
		size_t i;
		for(i=0; i<account.account_reg2_num; i++)
		{
			jstrescapecpy(tempstr,account.account_reg2[i].str);
			sz=sprintf(query,"INSERT INTO `global_reg_value` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , '%s' , '%d');",  account.account_id, tempstr, account.account_reg2[i].value);
			ret &= this->mysql_SendQuery(query, sz);
		}
	}
	return ret;
}