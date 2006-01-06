#include "basesq.h"
#include <wctype.h>

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


CMySQL::CMySQL(MYSQL new_handle)
{
	original_mysql_handle = mysqldb_handle;
	mysqldb_handle = new_handle;  // wonder if this works.. will ask shino later.

	safestrcpy(mysqldb_ip, "127.0.0.1", sizeof(mysqldb_ip));
	mysqldb_port=3306;
	safestrcpy(mysqldb_id, "ragnarok", sizeof(mysqldb_id));
	safestrcpy(mysqldb_pw, "ragnarok", sizeof(mysqldb_pw));
	safestrcpy(mysqldb_db, "ragnarok", sizeof(mysqldb_db));

}

// CMySQL Class Destructor
// Code is executed a CMySQL var is destroyed
inline CMySQL::~CMySQL() {
	this->Free();

	// if it's different, change it back =O
	mysqldb_handle = original_mysql_handle;
}


bool CMySQL::Query(const MiniString q) {

	#ifdef DEBUG_SQL
		ShowSQL("%s\n", q);
	#endif

	if( 0==mysql_real_query(&mysqldb_handle, q, q.length()) ) {

		this->Free();
		result = mysql_store_result(&mysqldb_handle);

		if(result)
			return true;
		else if(mysql_field_count(&mysqldb_handle) == 0)
            // query does not return data
            // (it was not a SELECT)
            return true;
		else
			ShowError("DB result error\nQuery:    %s\n", (const char *)q);
	}
	else
		ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&mysqldb_handle), (const char *)q);

	return false;
}

bool CMySQL::Fetch()
{
	if (!result) // was there even a result?
		if ((row = mysql_fetch_row(result))) // is there a row to pull out?
			return true;
	return false;
}

long int CMySQL::CountRes()
{
	return mysql_num_rows(result);
}

void CMySQL::Free()
{
	if (result)
	{
		mysql_free_result(result);
	}
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
bool CMySQL::mysql_SendQuery(const char* q, size_t sz)
{

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
	: cSqlRes(NULL)
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
		MiniString query;

		escape_string(uid, userid, strlen(userid));
		query << "SELECT `userid` FROM `" << login_auth_db << "` WHERE " << (case_sensitive?"BINARY":"") << "`userid` = '" <<  uid  <<"'";
		this->Query(query);
		if( this->Fetch())
		{
			ret = this->CountRes();
		}
	}
	return ret;
}

bool CAccountDB_sql::searchAccount(const char* userid, CLoginAccount& account)
{	// get account by user/pass
	bool ret = false;
	if(userid)
	{
		MiniString query;
		char uid[64];

		escape_string(uid, userid, strlen(userid));
		query << "SELECT"
			<<"`account_id`,"		//  0
			<<"`userid`,"			//  1
			<<"`user_pass`,"		//  2
			<<"`sex`,"			//  3
			<<"`gm_level`,"		//  4
			<<"`online`,"			//  5
			<<"`email`,"			//  6
			<<"`login_id1`,"		//  7
			<<"`login_id2`,"		//  8
			<<"`client_ip`,"		//  9
			<<"`last_login`,"		// 10
			<<"`login_count`,"	// 11
			<<"`ban_until`"		// 12
			<<"`valid_until`"		// 13
			<<" FROM `" << login_auth_db << "` WHERE " << (case_sensitive ? "BINARY" : "") << " `userid`='" << uid << "'";

		if( this->Query(query) )
		{
			//row fetching
			if(this->Fetch())
			{
				account.account_id	= this->row[0]?atol(this->row[0]):0;
				safestrcpy(account.userid, this->row[1]?this->row[1]:"", sizeof(account.userid));
				safestrcpy(account.passwd, this->row[2]?this->row[2]:"", sizeof(account.passwd));
				account.sex			= this->row[3][0] == 'S' ? 2 : this->row[3][0]=='M';
				account.gm_level	= this->row[4]?atol(this->row[4]):0;
				account.online		= this->row[5]?atol(this->row[5]):0;
				safestrcpy(account.email, this->row[6]?this->row[6]:"" , sizeof(account.email));
				account.login_id1	= this->row[7]?atol(this->row[7]):0;
				account.login_id2	= this->row[8]?atol(this->row[8]):0;
				account.client_ip	= ipaddress(this->row[9]);
				safestrcpy(account.last_login, this->row[10]?this->row[10]:"" , sizeof(account.last_login));
				account.login_count	= this->row[11]?atol(this->row[11]):0;
				account.valid_until	= (time_t)(this->row[12]?atol(this->row[12]):0);
				account.ban_until	= (time_t)(this->row[13]?atol(this->row[13]):0);

				// clear unused fields until they got removed from all implementations
				account.state = 0;
				account.error_message[0]=0;
				account.memo[0]=0;
				account.last_ip[0]=0;

				this->Free();

				query.clear();
				query << "SELECT `str`,`value` FROM `" << login_reg_db << "` WHERE `account_id`='" << (unsigned long)account.account_id << "'";
				if( this->Query(query) )
				{
					size_t i=0;
					while( i<ACCOUNT_REG2_NUM && this->Fetch() )
					{
						safestrcpy(account.account_reg2[i].str, this->row[0], sizeof(account.account_reg2[0].str));
						account.account_reg2[i].value = (this->row[1]) ? atoi(this->row[1]):0;
						i++;
					}
					account.account_reg2_num = i;

					this->Free();
				}
				ret = true;
			}
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

	sz=snprintf(query,sizeof(query), "SELECT "
			"`account_id`,"		//  0
			"`userid`,"			//  1
			"`user_pass`,"		//  2
			"`sex`,"			//  3
			"`gm_level`,"		//  4
			"`online`,"			//  5
			"`email`,"			//  6
			"`login_id1`,"		//  7
			"`login_id2`,"		//  8
			"`client_ip`,"		//  9
			"`last_login`,"		// 10
			"`login_count`,"	// 11
			"`ban_until`"		// 12
			"`valid_until`"		// 13
			" FROM `%s` WHERE `account_id`='%s'", login_auth_db, accid);

	if( this->mysql_SendQuery(sql_res1, query, sz) )
	{
		MYSQL_ROW sql_row = mysql_fetch_row(sql_res1);	//row fetching
		if(sql_row)
		{
			account.account_id	= sql_row[0]?atol(sql_row[0]):0;
			safestrcpy(account.userid, sql_row[1]?sql_row[1]:"", sizeof(account.userid));
			safestrcpy(account.passwd, sql_row[2]?sql_row[2]:"", sizeof(account.passwd));
			account.sex			= sql_row[3][0] == 'S' ? 2 : sql_row[3][0]=='M';
			account.gm_level	= sql_row[4]?atol(sql_row[4]):0;
			account.online		= sql_row[5]?atol(sql_row[5]):0;
			safestrcpy(account.email, sql_row[6]?sql_row[6]:"" , sizeof(account.email));
			account.login_id1	= sql_row[7]?atol(sql_row[7]):0;
			account.login_id2	= sql_row[8]?atol(sql_row[8]):0;
			account.client_ip	= ipaddress(sql_row[9]);
			safestrcpy(account.last_login, sql_row[10]?sql_row[10]:"" , sizeof(account.last_login));
			account.login_count	= sql_row[11]?atol(sql_row[11]):0;
			account.valid_until	= (time_t)(sql_row[12]?atol(sql_row[12]):0);
			account.ban_until	= (time_t)(sql_row[13]?atol(sql_row[13]):0);

			// clear unused fields until they got removed from all implementations
			account.state = 0;
			account.error_message[0]=0;
			account.memo[0]=0;
			account.last_ip[0]=0;

			sz = snprintf(query,sizeof(query), "SELECT `str`,`value` FROM `%s` WHERE `account_id`='%ld'", login_reg_db, (unsigned long)account.account_id);
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
	sz = snprintf(query,sizeof(query), "INSERT INTO `%s` (`userid`, `user_pass`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')", login_auth_db, uid, pwd, sex, email);
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

	sz=snprintf(query,sizeof(query),"DELETE FROM `%s` WHERE `account_id`='%d';",login_auth_db, accid);
	ret = this->mysql_SendQuery(query, sz);

	sz=snprintf(query,sizeof(query),"DELETE FROM `%s` WHERE `account_id`='%d';",login_reg_db, accid); // must update with the variable login_reg_db
	ret &=this->mysql_SendQuery(query, sz);
	return ret;
}

bool CAccountDB_sql::init(const char* configfile)
{	// init db
	size_t sz;		 // Used for size of queries
	bool wipe=false; // i dont know how a bool is set..
	char query[1024]; // used for the queries themselves

	CConfig::LoadConfig(configfile);

	mysql_init(&(this->mysqldb_handle));

	// All tables used for login: login_auth, login_reg, login_log, login_status

	// DB connection start
	ShowMessage("Connect Database Server on %s%u....\n", mysqldb_ip, mysqldb_port);
	if( mysql_real_connect(&(this->mysqldb_handle), mysqldb_ip, mysqldb_id, mysqldb_pw, mysqldb_db, mysqldb_port, (char *)NULL, 0) )
	{
		ShowMessage("connect success!\n");
		if (log_login)
		{
			sz = snprintf(query, sizeof(query), "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'Login', '100','login server started')", login_log_db);
			//query
			this->mysql_SendQuery(query, sz);
		}
	}
	else
	{	// pointer check
		ShowMessage("%s\n", mysql_error(&(this->mysqldb_handle)));
		return false;
	}

	if (wipe)
	{
		sz = snprintf(query, sizeof(query), "DROP TABLE IF EXISTS %s", login_auth_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = snprintf(query, sizeof(query),
		"CREATE TABLE IF NOT EXISTS `%s` ("
		"`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		"`user_id` VARCHAR(24) NOT NULL,"
		"`user_pass` VARCHAR(34) NOT NULL,"
		"`sex` ENUM('M','F','S') default 'M',"
		"`gm_level` INT(3) UNSIGNED NOT NULL,"
		"`online` BOOL default 'false',"
		"`email` VARCHAR(40) NOT NULL,"
		"`login_id1` INTEGER UNSIGNED NOT NULL,"
		"`login_id2` INTEGER UNSIGNED NOT NULL,"
		"`client_ip` VARCHAR(16) NOT NULL,"
		"`last_login` INTEGER UNSIGNED NOT NULL,"
		"`login_count` INTEGER UNSIGNED NOT NULL,"
		"`ban_until` INTEGER UNSIGNED NOT NULL,"
		"`valid_until` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`account_id`)"
		")", login_auth_db);
	this->mysql_SendQuery(query, sz);

	if (wipe)
	{
		sz = snprintf(query, sizeof(query), "DROP TABLE IF EXISTS %s", login_reg_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = snprintf(query,sizeof(query),
		"CREATE TABLE IF NOT EXISTS `%s` ("
		"`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		"`str` VARCHAR(34) NOT NULL,"  // Not sure on the length needed. (struct global_reg::str[32]) but better read the size from the struct later
		"`value` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`account_id`,`str`)"
		")", login_reg_db
		);
	this->mysql_SendQuery(query, sz);

	if (wipe)
	{
		sz = snprintf(query, sizeof(query), "DROP TABLE IF EXISTS %s", login_log_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = snprintf(query, sizeof(query),
		"CREATE TABLE IF NOT EXISTS `%s` ("
		"`time` INTEGER UNSIGNED,"
		"`ip` VARCHAR(16) NOT NULL,"
		"`user` VARCHAR(24) NOT NULL,"
		"`rcode` INTEGER(3) UNSIGNED NOT NULL,"
		"`log` VARCHAR(100) NOT NULL"
		")", login_log_db
		);
	this->mysql_SendQuery(query, sz);

	if (wipe)
	{
		sz = snprintf(query, sizeof(query), "DROP TABLE IF EXISTS %s", login_status_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = snprintf(query, sizeof(query),
		"CREATE TABLE IF NOT EXISTS `%s` ("
		"`index` INTEGER UNSIGNED NOT NULL,"
		"`name` VARCHAR(24) NOT NULL,"
		"`user` INTEGER UNSIGNED NOT NULL,"
		"PRIMARY KEY(`index`)"
		")", login_status_db
		);
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
		sz = snprintf(query, sizeof(query),"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", login_log_db);
		this->mysql_SendQuery(query, sz);
	}

	//delete all server status
	sz = snprintf(query, sizeof(query),"DELETE FROM `%s`", login_status_db);
	this->mysql_SendQuery(query, sz);

	mysql_close(&(this->mysqldb_handle));

	return true;
}

bool CAccountDB_sql::saveAccount(const CLoginAccount& account)
{
	bool ret = false;
	size_t sz, i;
	char query[2048], tempstr[64];

	sz = snprintf(query, sizeof(query), "UPDATE `%s` SET "
		"`user_id` = '%s', "
		"`user_pass` = '%s', "
		"`sex` = '%c', "
		"`gm_level` = '%d', "
		"`online` = '%ld', "
		"`email` = '%s', "
		"`login_id1` = '%ld', "
		"`login_id2` = '%ld', "
		"`client_ip` = '%s', "
		"`last_login` = '%s', "
		"`login_count` = '%ld', "
		"`valid_until` = '%ld', "
		"`ban_until` = '%ld', "
		"WHERE `account_id` = '%ld'",
		login_auth_db,
		account.userid,
		account.passwd,
		(account.sex==1)? 'M':'F',
		account.gm_level,
		account.online,
		account.email,
		(unsigned long)account.login_id1,
		(unsigned long)account.login_id2,
		((ipaddress)account.client_ip).getstring(tempstr),
		account.last_login,
		(unsigned long)account.login_count,
		(unsigned long)account.valid_until,
		(unsigned long)account.ban_until,
		(unsigned long)account.account_id);

	ret = this->mysql_SendQuery(query, sz);

	sz=snprintf(query, sizeof(query),"DELETE FROM `%s` WHERE `account_id`='%d';",login_reg_db, account.account_id);
	this->mysql_SendQuery(query, sz);

	sz = snprintf(query, sizeof(query),"INSERT INTO `%s` (`account_id`, `str`, `value`) VALUES ",  login_reg_db);
	for(i=0; i<account.account_reg2_num; i++)
	{
		this->escape_string(tempstr, account.account_reg2[i].str, strlen(account.account_reg2[i].str));
		sz += snprintf(query+sz, sizeof(query)-sz, "%s %s( '%d' , '%s' , '%d')",
			i?",":"", account.account_id, tempstr, account.account_reg2[i].value);
	}

	ret &= this->mysql_SendQuery(query, sz);

	return ret;
}


// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /
/// / CHAR SERVER IMPLEMENTATION BY CLOWNISIUS  / / / / / / / / / / / / / / / / / /
// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /


/* data for sql tables:

DROP TABLE IF EXISTS `char_characters`

CREATE TABLE IF NOT EXISTS `char_characters` (
  `char_id` 		INTEGER UNSIGNED NOT NULL auto_increment,
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `slot`			TINYINT UNSIGNED NOT NULL default '0',
  `name`			VARCHAR(24) NOT NULL default '',
  `class`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `base_level`		TINYINT UNSIGNED NOT NULL default '1',
  `job_level`		TINYINT UNSIGNED NOT NULL default '1',
  `base_exp`		BIGINT UNSIGNED NOT NULL default '0',
  `job_exp`			BIGINT UNSIGNED NOT NULL default '0',
  `zeny` 			BIGINT UNSIGNED NOT NULL default '500',  // set the default zeny later on...
  `str` 			TINYINT UNSIGNED NOT NULL default '0',
  `agi` 			TINYINT UNSIGNED NOT NULL default '0',
  `vit` 			TINYINT UNSIGNED NOT NULL default '0',
  `int`				TINYINT UNSIGNED NOT NULL default '0',
  `dex` 			TINYINT UNSIGNED NOT NULL default '0',
  `luk`				TINYINT UNSIGNED NOT NULL default '0',
  `max_hp` 			MEDIUMINT UNSIGNED NOT NULL default '0',  // possible to set a CREATE VIEW() to do the math of vit...
  `hp`				MEDIUMINT UNSIGNED NOT NULL default '0',
  `max_sp`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `sp` 				MEDIUMINT UNSIGNED NOT NULL default '0',
  `status_point`	MEDIUMINT UNSIGNED NOT NULL default '0',
  `skill_point`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `option` 			INTEGER NOT NULL default '0',
  `karma`			INTEGER NOT NULL default '0',
  `manner`			INTEGER NOT NULL default '0',
  `party_id`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `guild_id`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `pet_id`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `hair`			TINYINT UNSIGNED NOT NULL default '0',
  `hair_color` 		TINYINT UNSIGNED NOT NULL default '0',
  `clothes_color`	TINYINT UNSIGNED NOT NULL default '0',
  `weapon`			MEDIUMINT UNSIGNED NOT NULL default '1',
  `shield`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_top`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_mid`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_bottom`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `last_map`		VARCHAR(20) NOT NULL default 'new_5-1.gat',
  `last_x`			MEDIUMINT(3) UNSIGNED NOT NULL default '53',
  `last_y`			MEDIUMINT(3) UNSIGNED NOT NULL default '111',
  `save_map`		VARCHAR(20) NOT NULL default 'new_5-1.gat',
  `save_x`			MEDIUMINT(3) UNSIGNED NOT NULL default '53',
  `save_y`			MEDIUMINT(3) UNSIGNED NOT NULL default '111',
  `partner_id`		INTEGER UNSIGNED NOT NULL default '0',
  `father_id`		INTEGER UNSIGNED NOT NULL default '0',
  `mother_id`		INTEGER UNSIGNED NOT NULL default '0',
  `child_id`		INTEGER UNSIGNED NOT NULL default '0',
  `fame_points`		INTEGER UNSIGNED NOT NULL default '0',
  `online`			BOOL NOT NULL default 'false',
  PRIMARY KEY  (`char_id`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`)
)

DROP TABLE IF EXISTS `char_character_reg`

CREATE TABLE IF NOT EXISTS `char_character_reg` (
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `str`				VARCHAR(255) NOT NULL default '',
  `value`			VARCHAR(255) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`str`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `account_reg`

CREATE TABLE IF NOT EXISTS `account_reg` (
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `str`				VARCHAR(255) NOT NULL default '',
  `value`			VARCHAR(255) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`str`),
  KEY `account_id` (`account_id`)
)

DROP TABLE IF EXISTS `char_friends`

CREATE TABLE IF NOT EXISTS `char_friends` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `friend_id`		INTEGER UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`friend_id`),
  KEY `char_id` (`char_id`)
)


DROP TABLE IF EXISTS `char_inventory`

CREATE TABLE IF NOT EXISTS `char_inventory` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `char_cart`

CREATE TABLE IF NOT EXISTS `char_cart` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `account_storage`

CREATE TABLE IF NOT EXISTS `account_storage` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
)


DROP TABLE IF EXISTS `char_memo`

CREATE TABLE IF NOT EXISTS `char_memo` (
  `memo_id`			TINYINT UNSIGNED NOT NULL default '0',
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `map` 			VARCHAR(20) NOT NULL default '',
  `x` 				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  `y`				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`memo_id`),
  KEY `char_id` (`char_id`)
)


DROP TABLE IF EXISTS `char_skill`

CREATE TABLE IF NOT EXISTS `char_skill` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `id`				MEDIUMINT UNSIGNED NOT NULL default '0',
  `lv` 				TINYINT UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  KEY `char_id` (`char_id`)
)

	will continue when i have more rest.




		p.char_id = tmp_int[0];
		p.account_id = tmp_int[1];
		p.slot = tmp_int[2];
		p.class_ = tmp_int[3];
		p.base_level = tmp_int[4];
		p.job_level = tmp_int[5];
		p.base_exp = tmp_int[6];
		p.job_exp = tmp_int[7];
		p.zeny = tmp_int[8];
		p.hp = tmp_int[9];
		p.max_hp = tmp_int[10];
		p.sp = tmp_int[11];
		p.max_sp = tmp_int[12];
		p.str = tmp_int[13];
		p.agi = tmp_int[14];
		p.vit = tmp_int[15];
		p.int_ = tmp_int[16];
		p.dex = tmp_int[17];
		p.luk = tmp_int[18];
		p.status_point = tmp_int[19];
		p.skill_point = tmp_int[20];
		p.option = tmp_int[21];
		p.karma = GetByte(tmp_int[22],0);
		p.chaos = GetByte(tmp_int[22],1);
		p.manner = tmp_int[23];
		p.party_id = tmp_int[24];
		p.guild_id = tmp_int[25];
		p.pet_id = tmp_int[26];
		p.hair = tmp_int[27];
		p.hair_color = tmp_int[28];
		p.clothes_color = tmp_int[29];
		p.weapon = tmp_int[30];
		p.shield = tmp_int[31];
		p.head_top = tmp_int[32];
		p.head_mid = tmp_int[33];
		p.head_bottom = tmp_int[34];
		p.last_point.x = tmp_int[35];
		p.last_point.y = tmp_int[36];
		p.save_point.x = tmp_int[37];
		p.save_point.y = tmp_int[38];
		p.partner_id = tmp_int[39];
		p.father_id = tmp_int[40];
		p.mother_id = tmp_int[41];
		p.child_id = tmp_int[42];
		p.fame_points = tmp_int[43];

		size_t pos;

		if( cCharList.find(p, pos, 0) )
		{
			ShowError(CL_BT_RED"Character has an identical id to another.\n"CL_NORM);
			ShowMessage("           Character id #%ld -> new character not read.\n", (unsigned long)p.char_id);
			ShowMessage("           Character saved in log file.\n");
			return false;
		}
		else if( cCharList.find(p, pos, 1) )
		{
			ShowError(CL_BT_RED"Character name already exists.\n"CL_NORM);
			ShowMessage("           Character name '%s' -> new character not read.\n", p.name);
			ShowMessage("           Character saved in log file.\n");
			return false;
		}


		if( cAccountList.find( CCharCharAccount(p.account_id),0,pos) )
		{
			if( cAccountList[pos].charlist[p.slot] != 0 )
			{
				ShowError(CL_BT_RED"Character Slot already exists.\n"CL_NORM);
				ShowMessage("           Character name '%s' -> new character not read.\n", p.name);
				ShowMessage("           Character saved in log file.\n");
				return false;
			}
			else
			{
				cAccountList[pos].charlist[p.slot]=p.char_id;
			}
		}
		else
		{
			CCharCharAccount account(p.account_id);
			memset(account.charlist,0,sizeof(account.charlist));
			account.charlist[p.slot]=p.char_id;
			cAccountList.insert(account);
		}



		// 新規データ
		if (str[next] == '\n' || str[next] == '\r')
			return true;


		///////////////////////////////////////////////////////////////////////
		// more chaotic from here; code might look a bit weired
		bool ret = true;

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'&&i<MAX_MEMO; i++)
			{
				if (sscanf(str+next, "%[^,],%d,%d%n", p.memo_point[i].map, &tmp_int[0], &tmp_int[1], &len) != 3)
				{
					ShowError(CL_BT_RED"Character Memo points invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				p.memo_point[i].x = tmp_int[0];
				p.memo_point[i].y = tmp_int[1];
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}
		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++)
			{
				if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
					&tmp_int[4], &tmp_int[5], &tmp_int[6],
					&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
				{
					// do nothing, it's ok
				}
				else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
						  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
						  &tmp_int[4], &tmp_int[5], &tmp_int[6],
						  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
				{
				}
				else // invalid structure
				{
					ShowError(CL_BT_RED"Character Inventory invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

				p.inventory[i].id = tmp_int[0];
				p.inventory[i].nameid = tmp_int[1];
				p.inventory[i].amount = tmp_int[2];
				p.inventory[i].equip = tmp_int[3];
				p.inventory[i].identify = tmp_int[4];
				p.inventory[i].refine = tmp_int[5];
				p.inventory[i].attribute = tmp_int[6];
				p.inventory[i].card[0] = tmp_int[7];
				p.inventory[i].card[1] = tmp_int[8];
				p.inventory[i].card[2] = tmp_int[9];
				p.inventory[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++)
			{
				if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
					&tmp_int[4], &tmp_int[5], &tmp_int[6],
					&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
				{
					// do nothing, it's ok
				}
				else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
						   &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
						   &tmp_int[4], &tmp_int[5], &tmp_int[6],
						   &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
				{
				}
				else // invalid structure
				{
					ShowError(CL_BT_RED"Character Cart Items invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

				p.cart[i].id = tmp_int[0];
				p.cart[i].nameid = tmp_int[1];
				p.cart[i].amount = tmp_int[2];
				p.cart[i].equip = tmp_int[3];
				p.cart[i].identify = tmp_int[4];
				p.cart[i].refine = tmp_int[5];
				p.cart[i].attribute = tmp_int[6];
				p.cart[i].card[0] = tmp_int[7];
				p.cart[i].card[1] = tmp_int[8];
				p.cart[i].card[2] = tmp_int[9];
				p.cart[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++) {
				if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
				{
					ShowError(CL_BT_RED"Character skills invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				p.skill[tmp_int[0]].id = tmp_int[0];
				p.skill[tmp_int[0]].lv = tmp_int[1];
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			unsigned long val;
			next++;
			for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++)
			{	// global_reg実装以前のathena.txt互換のため一応'\n'チェック
				if(sscanf(str + next, "%[^,],%ld%n", p.global_reg[i].str, &val, &len) != 2)
				{	// because some scripts are not correct, the str can be "". So, we must check that.
					// If it's, we must not refuse the character, but just this REG value.
					// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
					if(str[next] == ',' && sscanf(str + next, ",%ld%n", &val, &len) == 1)
						i--;
					else
					{
						ShowError(CL_BT_RED"Character Char Variable invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
						ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
						ret = false;
						break;
					}

				}
				p.global_reg[i].value = val;
				next += len;
				if (str[next] == ' ')
					next++;
			}
			p.global_reg_num = i;
		}

		// insert the character to the list
		cCharList.insert(p);

		// check the next_char_id
		if(p.char_id >= next_char_id)
			next_char_id = p.char_id + 1;

		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// Function to read characters file
	bool read_chars(void)
	{
		char line[65536];
		int line_count;
		FILE *fp;


		fp = safefopen(char_txt, "r");
		if (fp == NULL)
		{
			ShowError("Characters file not found: %s.\n", char_txt);
	//		char_log("Characters file not found: %s." RETCODE, char_txt);
	//		char_log("Id for the next created character: %d." RETCODE, char_id_count);
			return false;
		}

		line_count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			unsigned long i;
			int j;
			line_count++;

			if( !get_prepared_line(line) )
				continue;
			line[sizeof(line)-1] = '\0';

			j = 0;
			if(sscanf(line, "%ld\t%%newid%%%n", &i, &j) == 1 && j > 0)
			{
				if(next_char_id < i)
					next_char_id = i;
				continue;
			}

			if( !char_from_str(line) )
			{
				// some error, message printed within
				//!! log line
				//char_log("%s", line);
			}

		}
		fclose(fp);

		if( cCharList.size() == 0 )
		{
			ShowError("No character found in %s.\n", char_txt);
			//char_log("No character found in %s." RETCODE, char_txt);
		}
		else if( cCharList.size() == 1 )
		{
			ShowStatus("1 character read in %s.\n", char_txt);
			//char_log("1 character read in %s." RETCODE, char_txt);
		}
		else
		{
			ShowStatus("mmo_char_init: %d characters read in %s.\n", cCharList.size(), char_txt);
			//char_log("mmo_char_init: %d characters read in %s." RETCODE, cList.size(), char_txt);
		}
		//char_log("Id for the next created character: %d." RETCODE, char_id_count);
		return true;
	}
*/


class CCharDB_sql : public CMySQL, private CConfig, public CCharDBInterface
{
	///////////////////////////////////////////////////////////////////////////
	// config stuff
	// uint32 next_char_id;

	char char_db[256];
	char mail_db[256];
	char friend_db[256];
	char memo_db[256];
	char cart_db[256];
	char inventory_db[256];
	char skill_db[256];
	char guild_skill_db[256];
	char char_reg_db[256];



	bool name_ignoring_case;
	int char_name_option;
	char char_name_letters[256];
	uint32 start_zeny;
	unsigned short start_weapon;
	unsigned short start_armor;
	struct point start_point;

	size_t savecount;

	///////////////////////////////////////////////////////////////////////////
	// data
//	TMultiListP<CCharCharacter, 2>	cCharList;
//	TslistDCT<CCharCharAccount>		cAccountList;

public:
	CCharDB_sql(const char *dbcfgfile)
	{
		safestrcpy(char_db,			"char",				sizeof(char_db));
		safestrcpy(mail_db,			"mail",				sizeof(mail_db));
		safestrcpy(friend_db,		"friends",			sizeof(friend_db));
		safestrcpy(memo_db,			"Memo",				sizeof(memo_db));
		safestrcpy(cart_db,			"cart_inventory",	sizeof(cart_db));
		safestrcpy(inventory_db,	"inventory",		sizeof(inventory_db));
		safestrcpy(skill_db,		"skill",			sizeof(skill_db));
		safestrcpy(guild_skill_db,	"guild_skill",		sizeof(guild_skill_db));
		safestrcpy(char_reg_db,		"char_reg",			sizeof(char_reg_db));


		char_name_option=0;
		memset(char_name_letters,0,sizeof(char_name_letters));

		name_ignoring_case=0;

		start_zeny		= 500;
		start_weapon	= 1201;
		start_armor		= 2301;

		safestrcpy(start_point.mapname,		"new_1-1.gat",		sizeof(start_point.mapname));
		start_point.x=53;
		start_point.y=111;

		init(dbcfgfile);
	}
	~CCharDB_sql()	{}


private:

	bool compare_item(const struct item &a, const struct item &b)
	{
		return ( (a.id == b.id) &&
				 (a.nameid == b.nameid) &&
				 (a.amount == b.amount) &&
				 (a.equip == b.equip) &&
				 (a.identify == b.identify) &&
				 (a.refine == b.refine) &&
				 (a.attribute == b.attribute) &&
				 (a.card[0] == b.card[0]) &&
				 (a.card[1] == b.card[1]) &&
				 (a.card[2] == b.card[2]) &&
				 (a.card[3] == b.card[3]) );
	}




	bool char_to_sql(const CCharCharacter& p)
	{
		MiniString query;

		bool diff,l,status, loaded=false;

		size_t sz,i;


		CCharCharacter cp;
/*
		size_t pos;
		if( cCharList.find( CCharCharacter(p.char_id), pos, 0) )
		{
			CCharCharacter &cp = cCharList(pos,0);
			loaded = true;
		}
		else // in case there is nothing loaded. we set one up with defaults 0. and compare against this new set.
		{
			memset(&cp, 0, sizeof(CCharCharacter));
			loaded = false;
		}
*/

// Build the update for the character
		query.clear();
		query <<
			"UPDATE `" << char_db << "` SET " <<

				"`class` = '"		<< p.class_ 		<< "'," <<
				"`base_level`='"	<< p.base_level		<< "'," <<
				"`job_level`='"		<< p.job_level		<< "'," <<
				"`base_exp`='"		<< p.base_exp		<< "'," <<
				"`job_exp`='"		<< p.job_exp		<< "'," <<
				"`zeny`='"			<< p.zeny			<< "'," <<

				"`hp`='"			<< p.hp				<< "'," <<
				"`max_hp`='"		<< p.max_hp			<< "'," <<

				"`sp`='"			<< p.sp				<< "'," <<
				"`max_sp`='"		<< p.max_sp			<< "'," <<

				"`str`='"			<< p.str			<< "'," <<
				"`agi`='"			<< p.agi			<< "'," <<
				"`vit`='"			<< p.vit			<< "'," <<
				"`int`='"			<< p.int_			<< "'," <<
				"`dex`='"			<< p.dex			<< "'," <<
				"`luk`='"			<< p.luk			<< "'," <<

				"`status_point`='"	<< p.status_point	<< "'," <<
				"`skill_point`='"	<< p.skill_point	<< "'," <<

				"`option`='"		<< p.option			<< "'," <<
				"`karma`='"			<< p.karma			<< "'," <<
				"`manner`='"		<< p.manner			<< "'," <<
				"`party_id`='"		<< p.party_id		<< "'," <<
				"`guild_id`='"		<< p.guild_id		<< "'," <<
				"`pet_id`='"		<< p.pet_id			<< "'," <<

				"`hair`='"			<< p.hair			<< "'," <<
				"`hair_color`='"	<< p.hair_color		<< "'," <<
				"`clothes_color`='"	<< p.clothes_color	<< "'," <<
				"`weapon`='"		<< p.weapon			<< "'," <<
				"`shield`='"		<< p.shield			<< "'," <<
				"`head_top`='"		<< p.head_top		<< "'," <<
				"`head_mid`='"		<< p.head_mid		<< "'," <<
				"`head_bottom`='"	<< p.head_bottom	<< "'," <<

				"`last_map`='"		<< p.last_point.mapname		<< "'," <<
				"`last_x`='"		<< p.last_point.x			<< "'," <<
				"`last_y`='"		<< p.last_point.y			<< "'," <<
				"`save_map`='"		<< p.save_point.mapname		<< "'," <<
				"`save_x`='"		<< p.save_point.x			<< "'," <<
				"`save_y`='"		<< p.save_point.y			<< "'," <<

				"`partner_id`='"	<< p.partner_id		<< "'," <<
				"`father`='"		<< p.father_id		<< "'," <<
				"`mother`='"		<< p.mother_id		<< "'," <<
				"`child`='"			<< p.child_id		<< "'," <<

				"`fame`='"			<< p.fame_points	<< "'" << // dont forget to remove commas at ends

			"WHERE  " <<
				"`account_id`='" << (unsigned long)p.account_id << "' " <<

			"AND " <<
				"`char_id` = '" << (unsigned long)p.char_id <<"' " <<

			"AND " <<
				"`slot` = '" << p.slot << "'";  // dont forget to finish the line


		this->Query(query);

		// This is set for overall.. if anything changed.... it will copy over old data... =o
		status = false;
		///////////////////////////////////////////////////////////////////////
		// Memo Insert

		diff = false;
		l = false;

		for(i=0;i<MAX_MEMO;i++)
		{
			if(
				(strcmp(p.memo_point[i].mapname, cp.memo_point[i].mapname) == 0) &&
				(p.memo_point[i].x == cp.memo_point[i].x) &&
				(p.memo_point[i].y == cp.memo_point[i].y)
				)continue;
			diff = true;
			status = true;
			break;
		}

		if (diff)
		{	query.clear();
			query << (const char*)
				"DELETE FROM `" << (const char*)memo_db << "` WHERE `char_id`='" << p.char_id << "'";

			this->Query(query);


			//insert here.
			query.clear();
			query << (const char*)
				"INSERT INTO `" << (const char*)memo_db << "`(`char_id`,`memo_id`,`map`,`x`,`y`) VALUES ";
			l=false;
			for(i=0;i<MAX_MEMO;i++)
			{
				if(p.memo_point[i].mapname[0])
				{
					query << (l?",":"") << "(" <<

					"'" << 	(unsigned long)p.char_id 	<< "'," <<
					"'" <<	i							<< "'," <<
					"'" <<	p.memo_point[i].mapname		<< "'," <<
					"'" <<	p.memo_point[i].x			<< "'," <<
					"'" <<	p.memo_point[i].y			<< "," <<  // Dont forget to end commas
					")";
					l= true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}


		///////////////////////////////////////////////////////////////////////
		// Inventory Insert

		diff = false;
		l = false;

		for(i = 0; i<MAX_INVENTORY; i++)
		{
			if (!compare_item(p.inventory[i], cp.inventory[i]))
			{
				diff = true;
				status = true;
				break;
			}
		}

		if (diff)
		{
			query.clear();
			query <<
				"DELETE FROM `" << cart_db << "` WHERE `char_id`='" << (unsigned long)p.char_id << "'";
			this->Query(query);

			//insert here.
			query.clear();
			query <<
				"INSERT INTO `" << inventory_db << "`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";
			l=false;
			for(i=0;i<MAX_INVENTORY;i++)
			{
				if(p.inventory[i].nameid>0)
				{
					query << (l?",":"") << "(" <<
					"'" <<	(unsigned long)p.char_id	<< "'," <<
					"'" <<	p.inventory[i].nameid		<< "'," <<
					"'" <<	p.inventory[i].amount		<< "'," <<
					"'" <<	p.inventory[i].equip		<< "'," <<
					"'" <<	p.inventory[i].identify		<< "'," <<
					"'" <<	p.inventory[i].refine		<< "'," <<
					"'" <<	p.inventory[i].attribute	<< "'," <<
					"'" <<	p.inventory[i].card[0]		<< "'," <<
					"'" <<	p.inventory[i].card[1]		<< "'," <<
					"'" <<	p.inventory[i].card[2]		<< "'," <<
					"'" <<	p.inventory[i].card[3]		<< "'" <<
					")";
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}


		///////////////////////////////////////////////////////////////////////
		// Cart Insert

		diff = false;
		l = false;

		for(i = 0; i<MAX_CART; i++)
		{
			if (!compare_item(p.cart[i], cp.cart[i]))
			{
				diff = true;
				status = true;
				break;
			}
		}

		if (diff)
		{
			query.clear();
			query << "DELETE FROM `" << cart_db << "` WHERE `char_id`='" << (unsigned long) p.char_id << "'";
			this->Query(query);

			//insert here.
			query.clear();
			query << "INSERT INTO `" << cart_db << "`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";
			l=false;
			for(i=0;i<MAX_CART;i++)
			{
				if(p.cart[i].nameid>0)
				{
					query << (l?",":"") << "(" <<
					"'" <<	(unsigned long)p.char_id	<< "'," <<
					"'" <<	p.cart[i].nameid		<< "'," <<
					"'" <<	p.cart[i].amount		<< "'," <<
					"'" <<	p.cart[i].equip		<< "'," <<
					"'" <<	p.cart[i].identify		<< "'," <<
					"'" <<	p.cart[i].refine		<< "'," <<
					"'" <<	p.cart[i].attribute	<< "'," <<
					"'" <<	p.cart[i].card[0]		<< "'," <<
					"'" <<	p.cart[i].card[1]		<< "'," <<
					"'" <<	p.cart[i].card[2]		<< "'," <<
					"'" <<	p.cart[i].card[3]		<< "'" <<
					")";

					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}

		///////////////////////////////////////////////////////////////////////
		// Skill Insert

		diff = false;
		l = false;

		if (diff)
		{
			query.clear();
			query << "DELETE FROM `" << skill_db << "` WHERE `char_id`='" << (unsigned long) p.char_id << "'";
			this->Query(query);

			//insert here.
			query.clear();
			query << "INSERT INTO `" << skill_db << "`(`char_id`,`id`,`lvl`) VALUES ";
			l=false;
			for(i=0;i<MAX_MEMO;i++)
			{
				if(p.skill[i].id>0 && p.skill[i].flag != 1)
				{
					query << (l?",":"") << "(" <<

					"'" << (unsigned long)p.char_id << "'," <<
					"'" << p.skill[i].id 			<< "'," <<
					"'" << p.skill[i].lv 			<< "'" <<
					")";
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}

		///////////////////////////////////////////////////////////////////////
		// Character Reg Insert

		diff = false;
		l = false;

		for(i=0;i<p.global_reg_num;i++)
		{
			if(
				((p.global_reg[i].str == NULL) == (cp.global_reg[i].str == NULL)) ||
				(p.global_reg[i].value == cp.global_reg[i].value) ||
				strcmp(p.global_reg[i].str, cp.global_reg[i].str) == 0
				)continue;
			diff = true;
			status = true;
			break;
		}


		if (diff)
		{
			query.clear();
			query << "DELETE FROM `" << char_reg_db << "` WHERE `char_id`='" << (unsigned long) p.char_id << "'";
			this->Query(query);

			//insert here.
			query.clear();
			query << "INSERT INTO `" << char_reg_db << "`(`char_id`,`str`,`value`) VALUES ";
			for(i=0;i<p.global_reg_num;i++)
			{
				if(p.global_reg[i].str && p.global_reg[i].value !=0)
				{
					query <<
					(l?",":"") << "(" <<

					"'" << (unsigned long)p.char_id << "'," <<
					"'" << p.global_reg[i].str		<< "'," <<
					"'" << p.global_reg[i].value	<< "'" <<   // end commas at the end
					")";

					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}


		///////////////////////////////////////////////////////////////////////
		// Friends Insert

		diff = false;
		l = false;

		for (i=0; i<MAX_FRIENDLIST; i++)
		{
			if( p.friendlist[i].friend_id != cp.friendlist[i].friend_id )
			{
				diff = true;
				status = true;
				break;
			}
		}

		if(diff)
		{
			query.clear();
			query << "DELETE FROM `" << friend_db << "` WHERE `char_id` = '" << (unsigned long) p.char_id << "'";
			this->Query(query);

			//insert here.
			query.clear();
			query << "INSERT INTO `" << friend_db << "`(`char_id`, `friend_id`) VALUES ";

			for(i=0;i<MAX_FRIENDLIST;i++)
			{
				if(p.friendlist[i].friend_id!=0)
				{
					query <<
					(l?",":"") << "(" <<
					"'" << (unsigned long) p.char_id << "'," <<
					"'" << (unsigned long) p.friendlist[i].friend_id << "'" <<
					")";

					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->Query(query);
		}

		if (status) // If anything has changed in this process, set to true in any of the differences check.
		{
			if(loaded)
				printf("need to think on how to load the character");
//				cCharList.removeindex(pos, 0); // remove old char_id where ever the cp index is pointing to
//			cCharList.insert(p); // now lets add our CharID to the index list.
		}

		return true;
	}

	bool char_from_sql(const char* name, CCharCharacter &p)
	{
		MiniString query;
		uint32 charid = 0;

		query.clear();
		query << "SELECT `char_id` FROM `" << char_db << " WHERE `name`='" << name << "'";

		if (this->Query(query))
		{
			this->Fetch();
			charid = atoi(this->row[0]);
			this->Free();

			return this->char_from_sql(charid,p);
		}
		return false;
	}

	bool char_from_sql(uint32 char_id, CCharCharacter &p, bool online = false)
	{

		int tmp_int[256];
		int next, len;
		size_t i,n;

		//CCharCharacter p;
		//CCharCharacter cp;

		MiniString query;

		// initilialise character
		memset(&p, 0, sizeof(CCharCharacter));
		//memset(&cp, 0, sizeof(CCharCharacter));
		memset(tmp_int, 0, sizeof(tmp_int));

		// if the data exists in the char_server database already,
		// wipe before loading new sql data before proceding..
		//cp = (struct mmo_charstatus*)numdb_search(char_db_,char_id);
		//if (cp != NULL)
		//  aFree(cp);

		#ifdef CHAR_DEBUG_INFO
			printf("Loaded: ");
		#endif

		p.char_id = char_id;

		//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
		//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
		//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
		//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
		//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
		//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
		//splite 2 parts. cause veeeery long SQL syntax

		query.clear();
		query <<
			"SELECT " <<
				"`char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`," <<
				"`str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point` FROM `" << (const char*)char_db << "` " <<
			"WHERE `char_id` = '" << (unsigned long)char_id << "'";

		if (this-Query(query))
		{
			this->Fetch();

			p.char_id = char_id;
			p.account_id = atoi(this->row[1]);
			p.slot = atoi(this->row[2]);
			strcpy(p.name, this->row[3]);
			p.class_ = atoi(this->row[4]);
			p.base_level = atoi(this->row[5]);
			p.job_level = atoi(this->row[6]);
			p.base_exp = atoi(this->row[7]);
			p.job_exp = atoi(this->row[8]);
			p.zeny = atoi(this->row[9]);
			p.str = atoi(this->row[10]);
			p.agi = atoi(this->row[11]);
			p.vit = atoi(this->row[12]);
			p.int_ = atoi(this->row[13]);
			p.dex = atoi(this->row[14]);
			p.luk = atoi(this->row[15]);
			p.max_hp = atoi(this->row[16]);
			p.hp = atoi(this->row[17]);
			p.max_sp = atoi(this->row[18]);
			p.sp = atoi(this->row[19]);
			p.status_point = atoi(this->row[20]);
			p.skill_point = atoi(this->row[21]);

			//free mysql result.
			this->Free();
		} else
			printf("char1 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?

		#ifdef CHAR_DEBUG_INFO
			printf("(\033[1;32m%d\033[0m)\033[1;32m%s\033[0m\t[",p.char_id,p.name);
			printf("char1 ");
		#endif

		query.clear();
		query <<
			"SELECT "<<
			"`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`," <<
			"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`," <<
			"`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`, `partner_id`, `father`, `mother`, `child`, `fame`" <<
			"FROM `" << char_db << "` WHERE `char_id` = '" << (unsigned int)char_id << "'";

		if (this->Query(query))
		{
			this->Fetch();

			p.option = atoi(this->row[0]);
			p.karma = atoi(this->row[1]);
			p.manner = atoi(this->row[2]);
			p.party_id = atoi(this->row[3]);
			p.guild_id = atoi(this->row[4]);
			p.pet_id = atoi(this->row[5]);

			p.hair = atoi(this->row[6]);
			p.hair_color = atoi(this->row[7]);
			p.clothes_color = atoi(this->row[8]);
			p.weapon = atoi(this->row[9]);
			p.shield = atoi(this->row[10]);
			p.head_top = atoi(this->row[11]);
			p.head_mid = atoi(this->row[12]);
			p.head_bottom = atoi(this->row[13]);
			strcpy(p.last_point.mapname,this->row[14]);
			p.last_point.x = atoi(this->row[15]);
			p.last_point.y = atoi(this->row[16]);
			strcpy(p.save_point.mapname,this->row[17]);
			p.save_point.x = atoi(this->row[18]);
			p.save_point.y = atoi(this->row[19]);
			p.partner_id = atoi(this->row[20]);
			p.father_id = atoi(this->row[21]);
			p.mother_id = atoi(this->row[22]);
			p.child_id = atoi(this->row[23]);
			p.fame_points = atoi(this->row[24]);

			//free mysql result.
			this->Free();
		} else
			printf("char2 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?




		if (p.last_point.x == 0 || p.last_point.y == 0 || p.last_point.mapname[0] == '\0')
		{
			char errbuf[64];
			sprintf(errbuf,"%s has no last point?\n",p.name);
			memcpy(&p.last_point, &start_point, sizeof(start_point));
		}

		if (p.save_point.x == 0 || p.save_point.y == 0 || p.save_point.mapname[0] == '\0')
		{
			char errbuf[64];
			sprintf(errbuf,"%s has no save point?\n",p.name);
			memcpy(&p.save_point, &start_point, sizeof(start_point));
		}
		#ifdef CHAR_DEBUG_INFO
			printf("char2 ");
		#endif

		//read memo data
		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		query.clear();
		query << "SELECT `map`,`x`,`y` FROM `" << memo_db << "` WHERE `char_id`='" << (unsigned int)char_id << "'";

		if (this->Query(query))
		{
			for(i=0; this->Fetch() && i < 3;i++)
			{
				strcpy (p.memo_point[i].mapname,this->row[0]);
				p.memo_point[i].x=atoi(this->row[1]);
				p.memo_point[i].y=atoi(this->row[2]);
			}
			this->Free();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("memo ");
		#endif

		//read inventory
		//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `gm_made`)
		query.clear();
		query << "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`" <<
			"FROM `"<< inventory_db << "` WHERE `char_id`='" << (unsigned long) char_id << "'";

		if (this->Query(query))
		{
			for(i=0;this->Fetch();i++)
			{
				p.inventory[i].id = atoi(this->row[0]);
				p.inventory[i].nameid = atoi(this->row[1]);
				p.inventory[i].amount = atoi(this->row[2]);
				p.inventory[i].equip = atoi(this->row[3]);
				p.inventory[i].identify = atoi(this->row[4]);
				p.inventory[i].refine = atoi(this->row[5]);
				p.inventory[i].attribute = atoi(this->row[6]);
				p.inventory[i].card[0] = atoi(this->row[7]);
				p.inventory[i].card[1] = atoi(this->row[8]);
				p.inventory[i].card[2] = atoi(this->row[9]);
				p.inventory[i].card[3] = atoi(this->row[10]);
			}
			this->Free();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("inventory ");
		#endif

		//read cart.
		//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
		query.clear();
		query << "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`" <<
			"FROM `" << cart_db << "` WHERE `char_id`='" << char_id = "'";

		if (this->Query(query))
		{
			for(i=0;this->Fetch();i++)
			{
				p.cart[i].id = atoi(this->row[0]);
				p.cart[i].nameid = atoi(this->row[1]);
				p.cart[i].amount = atoi(this->row[2]);
				p.cart[i].equip = atoi(this->row[3]);
				p.cart[i].identify = atoi(this->row[4]);
				p.cart[i].refine = atoi(this->row[5]);
				p.cart[i].attribute = atoi(this->row[6]);
				p.cart[i].card[0] = atoi(this->row[7]);
				p.cart[i].card[1] = atoi(this->row[8]);
				p.cart[i].card[2] = atoi(this->row[9]);
				p.cart[i].card[3] = atoi(this->row[10]);
			}
			this->Free();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("cart ");
		#endif
		//read skill
		//`skill` (`char_id`, `id`, `lv`)
		query.clear();
		query << "SELECT `id`, `lv` FROM `" << skill_db << "` WHERE `char_id`='" << (unsigned long) char_id << "'";

		if (this->Query(query))
		{
			for(i=0;this->Fetch();i++)
			{
				n = atoi(this->row[0]);
				p.skill[n].id = n; //memory!? shit!.
				p.skill[n].lv = atoi(this->row[1]);
			}
			this->Free();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("skill ");
		#endif

		//global_reg
		//`global_reg_value` (`char_id`, `str`, `value`)
		query.clear();
		query << "SELECT `str`, `value` FROM ` " << (const char*)char_reg_db << "` WHERE `char_id`='" << (unsigned long) char_id << "'";

		if (this->Query(query))
		{
			for(i=0;this->Fetch();i++)
			{
				strcpy (p.global_reg[i].str, this->row[0]);
				p.global_reg[i].value = atoi (this->row[1]);
			}
			this->Free();
		}
		p.global_reg_num=i;

		//Friends List Load

		for(i=0;i<20;i++)
		{
			p.friendlist[i].friend_id = 0;
			p.friendlist[i].friend_name[0] = '\0';
		}

		query.clear();
		query << "SELECT f.`friend_id`, c.`name` FROM `" << friend_db << "` f JOIN `" << (const char*)char_db << "` c ON f.`friend_id`=c.`char_id` WHERE f.`char_id` = '" << (unsigned long) char_id << "'";



		if (this->Query(query))
		{
			for(i=0;this->Fetch();i++)
			{
				p.friendlist[i].friend_id = atoi(this->row[0]);
				sprintf(p.friendlist[i].friend_name, "%s", this->row[1]);
			}
			this->Free();
		}

		#ifdef CHAR_DEBUG_INFO
			printf("friends ");
		#endif
		//-- end friends list load --

		if (online)
		{
			//set_char_online(char_id,p.account_id); // not setup yet... just leave as is
		}
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////
	// Function to create a new character

	bool make_new_char(CCharAccount& account,
						const char *n, // Name
						unsigned char str,
						unsigned char agi,
						unsigned char vit,
						unsigned char int_,
						unsigned char dex,
						unsigned char luk,
						unsigned char slot,
						unsigned char hair_style,
						unsigned char hair_color,
						CCharCharacter& p)
	{
		CCharCharacter tempchar(n);
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		char query [1024];
		char t_name[128];
		size_t i, sz;

		this->escape_string(t_name, tempchar.name, strlen(tempchar.name));

		//check stat error
		if (
			(str + agi + vit + int_ + dex + luk !=6*5 ) || // stats

			// Check slots
			(slot >= 9) || // slots must not over 9

			// Check hair
			(hair_style <= 0) || (hair_style >= 24) || // hair style
			(hair_color >= 9) ||					   // Hair color?

			// Check stats pairs and make sure they are balanced

			((str + int_) > 10) || // str + int pairs check
			((agi + luk ) > 10) || // agi + luk pairs check
			((vit + dex ) > 10) || // vit + dex pairs check

			// Check individual stats
			(str < 1 || str > 9) ||
			(agi < 1 || agi > 9) ||
			(vit < 1 || vit > 9) ||
			(int_< 1 || int_> 9) ||
			(dex < 1 || dex > 9) ||
			(luk < 1 || luk > 9) ||

			// Check size of the name, too short?
			(strlen(tempchar.name) < 4)

			)
		{
			printf("fail (aid: %d), stats error(bot cheat?!)\n", account.account_id);
			return false;
		} // now when we have passed all stat checks

		if( remove_control_chars(tempchar.name) )
			return false;

		// Check Authorised letters/symbols in the name of the character

		if (char_name_option == 1)// only letters/symbols in char_name_letters are authorised
			for (i = 0; tempchar.name[i]; i++)
				if( strchr(char_name_letters, tempchar.name[i]) == NULL )
					return false;

		else if (char_name_option == 2) // letters/symbols in char_name_letters are forbidden
			for (i = 0; tempchar.name[i]; i++)
				if (strchr(char_name_letters, tempchar.name[i]) != NULL)
					return false;

		// else, all letters/symbols are authorised (except control char removed before)



		//Check Name (already in use?)
		sz = snprintf(query, sizeof(query), "SELECT count(*) FROM `%s` WHERE `name` = '%s'",char_db, t_name);
		if ( this->mysql_SendQuery(sql_res, query, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if (atol(sql_row[0]))
				printf("fail, charname \"%s\" already in use\n", sql_row[0]);

			mysql_free_result(sql_res);
			return false;
		}


		// check char slot.
		sz = snprintf(query, sizeof(query), "SELECT count(*) FROM `%s` WHERE `account_id` = '%d' AND `char_num` = '%d'",char_db, account.account_id, slot);
		if ( this->mysql_SendQuery(sql_res, query, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if(atol(sql_row[0]))
				printf("fail (aid: %d, slot: %d), slot already in use\n", account.account_id, slot);

			mysql_free_result(sql_res);
			return false;
		}

		// It has passed both the name and slot check, let's insert the info since it doesnt conflict =D
		// make new char.
		sz = snprintf(query, sizeof(query),
				"INSERT INTO `%s` "
					"(`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`) "
					"VALUES "
					"('%ld'        ,'%d'      ,'%s'  ,'%d' ,'%d' ,'%d' ,'%d' ,'%d' ,'%d' ,'%d'  ,'%d'        )",
				 char_db,
				 (unsigned long)account.account_id , slot, t_name,  str, agi, vit, int_, dex, luk, hair_style, hair_color);

		this->mysql_SendQuery(query, sz);


		//Now we need the charid from sql!
		sz = snprintf(query, sizeof(query), "SELECT `char_id` FROM `%s` WHERE `account_id` = '%ld' AND `char_num` = '%d' AND `name` = '%s'", char_db, (unsigned long)account.account_id, slot, t_name);

		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if (sql_row)
				tempchar.char_id = atol(sql_row[0]); //char id :)
			mysql_free_result(sql_res);
		}

		//Give the char the default items
		//knife & cotton shirts, add on as needed ifmore items are to be included.
		sz = snprintf(query, sizeof(query),
					"INSERT INTO `%s` "
						"(`char_id`,`nameid`, `amount`, `equip`, `identify`) "
						"VALUES "
						"('%ld', '%d', '%d', '%d', '%d'),"
						"('%ld', '%d', '%d', '%d', '%d')",
						inventory_db,
						(unsigned long)tempchar.char_id, start_weapon,1,0x02,1, //add Knife
						(unsigned long)tempchar.char_id, start_armor, 1,0x10,1  //add cotton shirts
						);

		this->mysql_SendQuery(query, sz);


		// Update the map they are starting on and where they respawn at.
		sz = snprintf(query, sizeof(query),
					"UPDATE `%s` "
					"SET `last_map` = '%s', `last_x` = '%d', `last_y` = '%d', "
					    "`save_map` = '%s', `save_x` = '%d', `save_y` = '%d'  "
					"WHERE `char_id` = '%ld'",
					char_db,
					start_point.mapname, start_point.x, start_point.y,
					start_point.mapname, start_point.x, start_point.y,
					(unsigned long)tempchar.char_id);




		// All good, init the character data to return i think


		tempchar.account_id = account.account_id;
		tempchar.slot = slot;
		tempchar.class_ = 0;
		tempchar.base_level = 1;
		tempchar.job_level = 1;
		tempchar.base_exp = 0;
		tempchar.job_exp = 0;
		tempchar.zeny = start_zeny;
		tempchar.str = str;
		tempchar.agi = agi;
		tempchar.vit = vit;
		tempchar.int_ = int_;
		tempchar.dex = dex;
		tempchar.luk = luk;
		tempchar.max_hp = 40 * (100 + vit) / 100;
		tempchar.max_sp = 11 * (100 + int_) / 100;
		tempchar.hp = tempchar.max_hp;
		tempchar.sp = tempchar.max_sp;
		tempchar.status_point = 0;
		tempchar.skill_point = 0;
		tempchar.option = 0;
		tempchar.karma = 0;
		tempchar.manner = 0;
		tempchar.party_id = 0;
		tempchar.guild_id = 0;
		tempchar.hair = hair_style;
		tempchar.hair_color = hair_color;
		tempchar.clothes_color = 0;
		tempchar.inventory[0].nameid = start_weapon; // Knife
		tempchar.inventory[0].amount = 1;
		tempchar.inventory[0].equip = 0x02;
		tempchar.inventory[0].identify = 1;
		tempchar.inventory[1].nameid = start_armor; // Cotton Shirt
		tempchar.inventory[1].amount = 1;
		tempchar.inventory[1].equip = 0x10;
		tempchar.inventory[1].identify = 1;
		tempchar.weapon = 1;
		tempchar.shield = 0;
		tempchar.head_top = 0;
		tempchar.head_mid = 0;
		tempchar.head_bottom = 0;
		tempchar.last_point = start_point;
		tempchar.save_point = start_point;


		// unknown thingy i found =O
//		account.charlist[slot] = tempchar.char_id;

		p = tempchar;
//		cCharList.insert(tempchar);

		return true;
	}

	bool read_friends()
	{
		return true;
		// SELECT f.`char_id`,f.`friend_id`,c.`name` FROM `friends_db` f JOIN `char_db` c ON f.`friend_id` = c.`char_id`
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return 0;/*cCharList.size();*/ }
	virtual CCharCharacter& operator[](size_t i)	{ static CCharCharacter tmp; return tmp; /*cCharList[i];*/ }

	virtual bool existChar(const char* name);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(uint32 charid, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(uint32 charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(uint32 accid);



	///////////////////////////////////////////////////////////////////////////
	// mail access interface

/*
	//!! need rework
	current fields in table 'mail'
	------------------------------
	message_id			// used as is
	to_account_id		// used for target charid
	to_char_name		// used as is, not necessary though
	from_account_id		// used for sender charid
	from_char_name		// used as is, not necessary though
	message				// used as is
	read_flag			// used as is
	priority			// not used
	check_flag			// not used

	head				// missing
*/
	virtual size_t getUnreadCount(uint32 cid)
	{
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		char query[1024];
		size_t count = 0;
		size_t sz = snprintf(query, sizeof(query),
			"SELECT count(*) "
			"FROM `%s` WHERE `to_account_id` = \"%ld\" AND `read_flag` = \"0\"",
			mail_db, (unsigned long)cid);
		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			count = atol(sql_row[0]);

			mysql_free_result(sql_res);
		}
		return count;
	}
	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer)
	{
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		char query[1024];
		size_t sz = snprintf(query, sizeof(query),
			"SELECT `message_id`,`read_flag`,`from_char_name` "
			"FROM `%s` WHERE `to_account_id` = \"%ld\" ",
			mail_db, (unsigned long)cid);
		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			size_t count=0;
			unsigned char *buf = buffer;
			while( (sql_row = mysql_fetch_row(sql_res)) )
			{
				CMailHead mailhead( atol(sql_row[0]), atol(sql_row[1]), sql_row[2], "" );
				mailhead._tobuffer(buf); // automatic buffer increment
				count++;
			}
			mysql_free_result(sql_res);
			return count;
		}
		return 0;
	}
	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail)
	{
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		char query[1024];
		bool ret = false;
		size_t sz = snprintf(query, sizeof(query),
			"SELECT `read_flag`,`from_char_name`,`message` "
			"FROM `%s` WHERE `to_account_id` = \"%ld\" AND `message_id` = \"%ld\" ",
			mail_db, (unsigned long)cid, (unsigned long)mid);

		// default clearing
		mail.read    = 0;
		mail.name[0] = 0;
		mail.head[0] = 0;
		mail.body[0] = 0;

		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			if( (sql_row = mysql_fetch_row(sql_res)) )
			{
				ret = true;
				mail = CMail(mid, atol(sql_row[0]), sql_row[1], "", sql_row[2] );
				if( 0==mail.read )
				{
					sz = snprintf(query, sizeof(query),
						"UPDATE `%s` SET `read_flag`='1' WHERE `message_id`= \"%ld\"",
						mail_db, (unsigned long)mid);
					this->mysql_SendQuery(query, sz);
				}
			}
			mysql_free_result(sql_res);
		}
		return ret;
	}
	virtual bool deleteMail(uint32 cid, uint32 mid)
	{
		char query[1024];
		size_t sz = snprintf(query, sizeof(query),
			"DELETE "
			"FROM `%s` WHERE `to_account_id` = \"%ld\" AND `message_id` = \"%ld\" ",
			mail_db, (unsigned long)cid, (unsigned long)mid);
		return this->mysql_SendQuery(query, sz);
	}
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid)
	{
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		bool ret = false;
		size_t sz;
		char query[1024];
		char _head[128];
		char _body[128];
		char _targetname[32];
		escape_string(_targetname, targetname, strlen(targetname));
		escape_string(_head, head, strlen(head));
		escape_string(_body, body, strlen(body));

		if( 0==strcmp(targetname,"*") )
		{
			sz = snprintf(query, sizeof(query),
				"SELECT DISTINCT `char_id`,`name` "
				"FROM `%s` WHERE `char_id` <> '%ld' ORDER BY `char_id`",
				char_db, (unsigned long)senderid);
		}
		else
		{
			sz = snprintf(query, sizeof(query),
				"SELECT `char_id`,`name` "
				"FROM `%s` WHERE `name` = \"%s\"",
				char_db, _targetname);
		}
		if( this->mysql_SendQuery(sql_res, query, sz) )
		{
			ret = true;
			while( (sql_row = mysql_fetch_row(sql_res)) )
			{
				sz = snprintf(query, sizeof(query),
					"INSERT DELAYED INTO `%s` "
					"(`to_account_id`,`to_char_name`,"
					"`from_account_id`,`from_char_name`,"
					"`message`,`read_flag`)"
					" VALUES ('%ld', '%s', '%ld', '%s', '%s', '%d')",
					mail_db,
					atol(sql_row[0]), sql_row[1],
					(unsigned long)senderid, sendername,
					_body, 0);

				ret &= this->mysql_SendQuery(query, sz);
			}
			mysql_free_result(sql_res);
		}
		return ret;
	}


private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		//return read_chars() && read_friends();
		return true;
	}
	bool close()
	{
		//return save_chars() && save_friends();
		return true;
	}
};


bool CCharDB_sql::ProcessConfig(const char*w1, const char*w2)
{

	if(strcasecmp(w1, "start_point") == 0)
	{
		char mapname[32];
		int x, y;
		if(sscanf(w2, "%[^,],%d,%d", mapname, &x, &y) == 3 )
		{
			char *ip=strchr(mapname, '.');
			if( ip != NULL ) *ip=0;
			safestrcpy(start_point.mapname, mapname, sizeof(start_point.mapname));
			start_point.x = x;
			start_point.y = y;
		}
	}
	else if(strcasecmp(w1, "start_zeny") == 0)
	{
		start_zeny = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "start_weapon") == 0)
	{
		start_weapon = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "start_armor") == 0)
	{
		start_armor = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "name_ignoring_case") == 0)
	{
		name_ignoring_case = Switch(w2);
	}
	else if(strcasecmp(w1, "char_name_option") == 0)
	{
		char_name_option = atoi(w2);
	}
	else if(strcasecmp(w1, "char_name_letters") == 0)
	{
		safestrcpy(char_name_letters, w2, sizeof(char_name_letters));
	}
	return true;
}

bool CCharDB_sql::existChar(const char* name)
{
	// select char id where name = *name
//	size_t pos;
	return false;//cCharList.find( CCharCharacter(name), pos, 1);
}
bool CCharDB_sql::searchChar(const char* name, CCharCharacter&data)
{
	if (char_from_sql(name,data) )
		return true;
	return false;
}
bool CCharDB_sql::searchChar(uint32 charid, CCharCharacter&data)
{
	if (char_from_sql(charid,data) )
		return true;
	return false;
}

bool CCharDB_sql::insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)
{
	if ( make_new_char(account,name,str,agi,vit,int_,dex,luk,slot,hair_style,hair_color,data) )
		return true;
	return false;
}
bool CCharDB_sql::removeChar(uint32 charid)
{
	MiniString query;
	query << "DELETE FROM `" << (const char*)char_db << "` WHERE `char_id`='" << (unsigned char)charid << "'";
	this-Query(query);
	return true;
}

bool CCharDB_sql::saveChar(const CCharCharacter& data)
{
	// INSERT all character data into this function =o
	return char_to_sql(data);
}
bool CCharDB_sql::searchAccount(uint32 accid, CCharCharAccount& account)
{
	// SELECT char_id where account_id = *accid
/*	size_t pos;
	if( cAccountList.find(CCharCharAccount(accid),0,pos) )
	{
		account = cAccountList[pos];
		return true;
	}
*/	return false;
}
bool CCharDB_sql::saveAccount(CCharAccount& account)
{
	// Unknown function, can't find relative info for cAccountList.insert()

	// this is function is used in txt server to synchronise login and char server
	// to given account data,
	// function could be empty if login and char are using the same database
	// otherwise char needs to store the values to allow account authentification
	// will change when uniting login and char data interface

/*	size_t pos;
	if( cAccountList.find(account,0,pos) )lol
	{	// exist -> update list entry
		cAccountList[pos].CCharAccount::operator=(account);
		return true;
	}
	else
	{	// create new
		return cAccountList.insert(account);
	}
*/
	return false;
}
bool CCharDB_sql::removeAccount(uint32 accid)
{
	MiniString query;

	query << "DELETE FROM `" << char_db << "` WHERE `account_id` = '" << (unsigned long) accid << "'";

	this->Query(query);
	return true;
}





class CGuildDB_sql : public CMySQL, private CConfig, public CGuildDBInterface
{

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB_sql(const char *configfile)
	{
		init(configfile);
	}
	virtual ~CGuildDB_sql()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// data


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return false;
	}
	bool close()
	{
		return false;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return 0;/*cGuilds.size();*/ }
	virtual CGuild& operator[](size_t i)	{ static CGuild tmp; return tmp; /*return cGuilds[i];*/ }

	virtual size_t castlesize()				{ return 0;/*cGuilds.size();*/ }
	virtual CCastle &castle(size_t i)		{ static CCastle tmp; return tmp; }


	virtual bool searchGuild(const char* name, CGuild& guild)
	{
		//select guild_data where name = name
		// return true and guild data
		return false;
	}
	virtual bool searchGuild(uint32 guildid, CGuild& guild)
	{
		// select guild_data where guild_id = guildid
		// return true and guild data
		return false;
	}
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild)
	{
		// Insert into guild (columns) VALUES (*guild)
		// return true if succesfull

/*			//tmp.name[24];
			tmp.guild_id = next_guild_id++;

			tmp.member[0] = member;
			safestrcpy(tmp.master, member.name, sizeof(tmp.master));
			tmp.position[0].mode=0x11;
			safestrcpy(tmp.position[0].name,"GuildMaster", sizeof(tmp.position[0].name));
			safestrcpy(tmp.position[MAX_GUILDPOSITION-1].name,"Newbie", sizeof(tmp.position[0].name));
			for(i=1; i<MAX_GUILDPOSITION-1; i++)
				snprintf(tmp.position[i].name,sizeof(tmp.position[0].name),"Position %d",i+1);

			tmp.max_member=16;
			tmp.average_lv=tmp.member[0].lv;
			tmp.castle_id=0xFFFF;
			for(i=0;i<MAX_GUILDSKILL;i++)
				tmp.skill[i].id = i+GD_SKILLBASE;

			guild = tmp;
			return cGuilds.insert(tmp);
		}
		*/
		return false;
	}
	virtual bool removeGuild(uint32 guildid)
	{
		// Delete from guild where guild_id = *guildid
		// clear alliances
		// clear castles
		// else
		return false;
	}
	virtual bool saveGuild(const CGuild& guild)
	{
		// update guild set new_data where guild_id = *guild.id
		// else
		return false;
	}

	virtual bool searchCastle(ushort cid, CCastle& castle)
	{
		// select data from castle_db where castle_id = cid
		//else
		return false;
	}
	virtual bool saveCastle(CCastle& castle)
	{
		// update castle_db set new_data where castle_id = *castle.id
		// else
		return false;
	}
	virtual bool removeCastle(ushort cid)
	{
		// Delete from castle_db where castle_id = *cid
		// else
		return false;
	}
};



///////////////////////////////////////////////////////////////////////////////
// Party Database
///////////////////////////////////////////////////////////////////////////////
class CPartyDB_sql : public CMySQL, private CConfig, public CPartyDBInterface
{
/*		// Shouldnt be needed for SQL versions
		// WIll keep for reference to data structures and usage
		// after done, will remove, and test compile =D

	ssize_t party_to_string(char *str, size_t maxlen, const CParty &p)
	{
		ssize_t i, len;
		len = sprintf(str, "%ld\t%s\t%d,%d\t", (unsigned long)p.party_id, p.name, p.expshare, p.itemshare);
		for(i = 0; i < MAX_PARTY; i++)
		{
			const struct party_member &m = p.member[i];
			len += sprintf(str+len, "%ld,%ld\t%s\t", (unsigned long)m.account_id, (unsigned long)m.leader, ((m.account_id > 0) ? m.name : "NoMember"));
		}
		sprintf(str+len, RETCODE);
		return len;
	}
	bool party_from_string(const char *str, CParty &p)
	{
		int i, j;
		int tmp_int[16];
		char tmp_str[256];

		if (sscanf(str, "%d\t%255[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
			return false;

		p.party_id = tmp_int[0];
		safestrcpy(p.name, tmp_str, sizeof(p.name));
		p.expshare = tmp_int[1];
		p.itemshare = tmp_int[2];
		for(j=0; j<3 && str != NULL; j++)
			str = strchr(str+1, '\t');

		for(i=0; i<MAX_PARTY; i++)
		{
			struct party_member &m = p.member[i];
			if(str == NULL)
				return false;
			if(sscanf(str + 1, "%d,%d\t%255[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str) != 3)
				return false;

			m.account_id = tmp_int[0];
			m.leader = tmp_int[1];
			safestrcpy(m.name, tmp_str, sizeof(m.name));
			for(j=0; j<2 && str != NULL; j++)
				str = strchr(str + 1, '\t');
		}
		return true;
	}
	bool readParties()
	{
		char line[8192];
		int c = 0;
		int i, j;
		FILE *fp = safefopen(party_filename, "r");

		if( fp == NULL )
		{
			ShowError("Party: cannot open %s\n", party_filename);
			return false;
		}
		while(fgets(line, sizeof(line), fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			j = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && next_party_id <= (uint32)i)
			{
				next_party_id = i;
			}
			else
			{
				CParty p;
				if( party_from_string(line, p) && p.party_id > 0)
				{
					if( !p.isempty() )
					{
						if(p.party_id >= next_party_id)
							next_party_id = p.party_id + 1;

						cParties.insert(p);
					}
				}
				else
				{
					ShowError("int_party: broken data [%s] line %d\n", party_filename, c);
				}
			}
		}
		fclose(fp);
		ShowStatus("Party: %s read done (%d parties)\n", party_filename, c);
		return true;
	}
	bool saveParties()
	{
		char line[65536];
		FILE *fp;
		int lock;
		size_t i;
		ssize_t sz;

		if ((fp = lock_fopen(party_filename, &lock)) == NULL) {
			ShowError("Party: cannot open [%s]\n", party_filename);
			return false;
		}
		for(i=0; i<cParties.size(); i++)
		{
			sz = party_to_string(line, sizeof(line), cParties[i]);
			if(sz>0) fwrite(line, sz,1, fp);
		}
		fprintf(fp, "%d\t%%newid%%\n", next_party_id);
		lock_fclose(fp,party_filename, &lock);
		return 0;
	}
*/


public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDB_sql(const char *configfile)
	{
		init(configfile);
	}
	virtual ~CPartyDB_sql()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CParty, 2>	cParties;
	uint32					next_party_id;
	uint					savecount;
	char					party_filename[1024];

	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return true;
	}
	bool close()
	{
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return 0; /*cParties.size();*/ }
	virtual CParty& operator[](size_t i)	{ static CParty tmp; return tmp; /*return cParties[i];*/ }

	virtual bool searchParty(const char* name, CParty& party)
	{
		// select data from party where name = *name
		// return *party true
		return false;
	}
	virtual bool searchParty(uint32 pid, CParty& party)
	{
		// select data from party where party_id = *pid
		// return party data true
		return false;
	}
	virtual bool insertParty(uint32 accid, const char *nick, const char *map, ushort lv, const char *name, CParty &party)
	{
		// insert into party values (*party)
/*
		size_t pos;
		CParty temp(name);
		if( !cParties.find( temp, pos, 0) )
		{
			//temp.name[24];
			temp.party_id = next_party_id++;
			temp.expshare = 0;
			temp.itemshare = 0;
			temp.member[0].account_id = accid;
			safestrcpy(temp.member[0].name, nick, sizeof(temp.member[0].name));
			safestrcpy(temp.member[0].mapname, map, sizeof(temp.member[0].mapname));
			temp.member[0].leader = 1;
			temp.member[0].online = 1;
			temp.member[0].lv = lv;

			cParties.insert(temp);
			party = temp;
			return true;
		}

*/
		return false;
	}
	virtual bool removeParty(uint32 pid)
	{
		// delete from party where party_id = *pid
		// return true
		return false;
	}
	virtual bool saveParty(const CParty& party)
	{
		// update party set values=*party
		// return true
		return false;
	}
};



///////////////////////////////////////////////////////////////////////////////
// Storage Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPCStorageDB_sql : public CMySQL, private CConfig, public CPCStorageDBInterface
{
/*		// Shouldnt be needed for SQL versions
		// WIll keep for reference to data structures and usage
		// after done, will remove, and test compile =D

	ssize_t storage_to_string(char *str, size_t maxlen, const CPCStorage &stor)
	{
		int i,f=0;
		char *str_p = str;
		str_p += sprintf(str_p,"%ld,%d\t",(unsigned long)stor.account_id, stor.storage_amount);
		for(i=0;i<MAX_STORAGE;i++)
		{
			if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
			{
				str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					stor.storage[i].id,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
					stor.storage[i].identify,stor.storage[i].refine,stor.storage[i].attribute,
					stor.storage[i].card[0],stor.storage[i].card[1],stor.storage[i].card[2],stor.storage[i].card[3]);
				f++;
			}
		}
		*(str_p++)='\t';
		*str_p='\0';
		if(!f)
		{
			str[0]=0;
			str_p=str;
		}
		return (str_p-str);
	}
	int storage_from_string(const char *str, CPCStorage &stor)
	{
		int tmp_int[256];
		int set,next,len,i;

		set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
		stor.storage_amount=tmp_int[1];

		if(set!=2)
			return false;
		if(str[next]=='\n' || str[next]=='\r')
			return false;
		next++;
		for(i=0;str[next] && str[next]!='\t';i++)
		{
			if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
			else if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
			else
				return false;
		}
		return true;
	}
	bool readPCStorage()
	{
		char line[65536];
		int c=0;
		unsigned long tmp;
		CPCStorage s;
		FILE *fp=safefopen(pcstorage_filename,"r");

		if(fp==NULL)
		{
			ShowError("Storage: cannot open %s\n", pcstorage_filename);
			return false;
		}
		while(fgets(line,sizeof(line),fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			sscanf(line,"%ld",&tmp);
			s.account_id=tmp;
			if(s.account_id > 0 && storage_from_string(line,s) )
			{
				cPCStorList.insert(s);
			}
			else
			{
				ShowError("Storage: broken data [%s] line %d\n",pcstorage_filename,c);
			}
		}
		fclose(fp);
		return true;
	}
	bool savePCStorage()
	{
		char line[65536];
		int lock;
		size_t i, sz;
		FILE *fp=lock_fopen(pcstorage_filename,&lock);

		if( fp==NULL )
		{
			ShowError("Storage: cannot open [%s]\n",pcstorage_filename);
			return false;
		}
		for(i=0; i<cPCStorList.size(); i++)
		{
			sz = storage_to_string(line, sizeof(line), cPCStorList[i]);
			if(sz>0) fprintf(fp,"%s"RETCODE,line);
		}
		lock_fclose(fp, pcstorage_filename, &lock);
		return true;
	}

*/

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDB_sql(const char *dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CPCStorageDB_sql()
	{
		close();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// data


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return true;
	}
	bool close()
	{
		return true;
	}



	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return 0; /*cPCStorList.size();*/ }
	virtual CPCStorage& operator[](size_t i)	{ static CPCStorage tmp; return tmp; /*cPCStorList[i];*/ }

	virtual bool searchStorage(uint32 accid, CPCStorage& stor)
	{
		//select data from storage where account_id = *accid
		// return true and stor
		return false;
	}
	virtual bool removeStorage(uint32 accid)
	{
		// delete from storage where account_id = *accid
		// else
		return false;
	}
	virtual bool saveStorage(const CPCStorage& stor)
	{
		// insert into storage values (*stor)
		// return false, and possible debugging on why it couldnt insert
	}
};



class CGuildStorageDB_sql : public CMySQL, private CConfig, public CGuildStorageDBInterface
{
/*		// Shouldnt be needed for SQL versions
		// WIll keep for reference to data structures and usage
		// after done, will remove, and test compile =D

	ssize_t guild_storage_to_string(char *str, size_t maxlen, const CGuildStorage &stor)
	{
		int i,f=0;
		char *str_p = str;
		str_p+=sprintf(str,"%ld,%d\t",(unsigned long)stor.guild_id, stor.storage_amount);

		for(i=0;i<MAX_GUILD_STORAGE;i++)
		{
			if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
			{
				str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					stor.storage[i].id,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
					stor.storage[i].identify,stor.storage[i].refine,stor.storage[i].attribute,
					stor.storage[i].card[0],stor.storage[i].card[1],stor.storage[i].card[2],stor.storage[i].card[3]);
				f++;
			}
			*(str_p++)='\t';
			*str_p='\0';
		}
		if(!f)
		{
			str[0]=0;
			str_p=str;
		}
		return (str_p-str);
	}
	bool guild_storage_from_string(const char *str, CGuildStorage &stor)
	{
		int tmp_int[256];
		int set,next,len,i;

		set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
		if(set!=2)
			return false;
		if(str[next]=='\n' || str[next]=='\r')
			return false;
		next++;

		stor.storage_amount = (((ushort)tmp_int[1])<MAX_GUILD_STORAGE) ? tmp_int[1] : MAX_GUILD_STORAGE;
		for(i=0; str[next] && str[next]!='\t'; i++)
		{
			if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				while(str[next] == ' ') next++;
			}
			else if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				  &tmp_int[4], &tmp_int[5], &tmp_int[6],
				  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				while(str[next] == ' ')	next++;
			}
			else
				return false;
		}
		return true;
	}

	bool readGuildStorage()
	{
		char line[65536];
		int c=0;
		unsigned long tmp;
		CGuildStorage gs;
		FILE *fp=safefopen(guildstorage_filename,"r");
		if(fp==NULL){
			ShowMessage("cant't read : %s\n",guildstorage_filename);
			return 1;
		}
		while(fgets(line,sizeof(line),fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			sscanf(line,"%ld",&tmp);
			gs.guild_id=tmp;
			if(gs.guild_id > 0 && guild_storage_from_string(line,gs) )
			{
				cGuildStorList.insert(gs);
			}
			else
			{
				ShowError("Storage: broken data [%s] line %d\n", guildstorage_filename, c);
			}
		}
		fclose(fp);
		return true;
	}
	bool saveGuildStorage()
	{
		char line[65536];
		int lock;
		size_t i, sz;
		FILE *fp=lock_fopen(guildstorage_filename,&lock);

		if( fp==NULL )
		{
			ShowError("Storage: cannot open [%s]\n",guildstorage_filename);
			return false;
		}
		for(i=0; i<cGuildStorList.size(); i++)
		{
			sz = guild_storage_to_string(line, sizeof(line), cGuildStorList[i]);
			if(sz>0) fprintf(fp,"%s"RETCODE,line);
		}
		lock_fclose(fp, guildstorage_filename, &lock);
		return true;
	}
	*/

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDB_sql(const char *dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CGuildStorageDB_sql()
	{
		close();
	}


private:
	///////////////////////////////////////////////////////////////////////////
	// data


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return false;
	}
	bool close()
	{
		return false;
	}



	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return 0; /*cGuildStorList.size();*/ }
	virtual CGuildStorage& operator[](size_t i)	{ static CGuildStorage tmp; return tmp; /*cGuildStorList[i];*/ }

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor)
	{
		// select data from guild_storage where guild_id = *gid
		// return stor and true
		return false;
	}
	virtual bool removeStorage(uint32 gid)
	{
		// delete from guild_storage where guild_id = *gid
		return false;
	}
	virtual bool saveStorage(const CGuildStorage& stor)
	{
		//insert into storage values (*sto)
	}
};


#endif// SQL

