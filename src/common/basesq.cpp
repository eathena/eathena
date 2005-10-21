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

		escape_string(uid, userid, strlen(userid));
		sz=sprintf(query, "SELECT"
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
			" FROM `%s` WHERE %s `userid`='%s'", login_auth_db, case_sensitive ? "BINARY" : "", uid);

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

				sz = sprintf(query, "SELECT `str`,`value` FROM `%s` WHERE `account_id`='%ld'", login_reg_db, (unsigned long)account.account_id);
				if( this->mysql_SendQuery(sql_res2, query, sz) )
				{
					size_t i=0;
					while( i<ACCOUNT_REG2_NUM && (sql_row = mysql_fetch_row(sql_res2)) )
					{
						safestrcpy(account.account_reg2[i].str, sql_row[0], sizeof(account.account_reg2[0].str));
						account.account_reg2[i].value = (sql_row[1]) ? atoi(sql_row[1]):0;
						i++;
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

	sz=sprintf(query, "SELECT "
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
	size_t sz;		 // Used for size of queries
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
			sz = sprintf(query, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'Login', '100','login server started')", login_log_db);
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
		sz = sprintf(query, "DROP TABLE IF EXISTS %s", login_auth_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
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
		sz = sprintf(query, "DROP TABLE IF EXISTS %s", login_reg_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
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
		sz = sprintf(query, "DROP TABLE IF EXISTS %s", login_log_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
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
		sz = sprintf(query, "DROP TABLE IF EXISTS %s", login_status_db);
		this->mysql_SendQuery(query, sz);
	}

	sz = sprintf(query,
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
	size_t sz, i;
	char query[2048], tempstr[64];

	sz = sprintf(query, "UPDATE `%s` SET "
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

	sz=sprintf(query,"DELETE FROM `%s` WHERE `account_id`='%d';",login_reg_db, account.account_id);
	this->mysql_SendQuery(query, sz)


	sz = sprintf(query,"INSERT INTO `%s` (`account_id`, `str`, `value`) VALUES ",  login_reg_db);

	for(i=0; i<account.account_reg2_num; i++)
	{
		mysql_real_escape_string(&mysql_handle, tempstr, account.account_reg2[i].str, strlen(account.account_reg2[i].str));
		sz += sprintf(query, "%s %s( '%d' , '%s' , '%d')",query,i?",":"", account.account_id, tempstr, account.account_reg2[i].value);
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

			if( !skip_empty_line(line) )
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


class CCharDB_sql : public CTimerBase, private CConfig, public CCharDBInterface
{
	///////////////////////////////////////////////////////////////////////////
	// config stuff
	// uint32 next_char_id;

	char char_db[256];
//	char backup_txt[1024]; // DOesnt exist
	char friends_db[256];
	bool backup_txt_flag;

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
	CCharDB_sql(const char *dbcfgfile) :
		CTimerBase(300*1000)		// 300sec save interval
	{
		next_char_id = 150000;
		safestrcpy(char_txt, "save/athena.txt", sizeof(char_txt));
		safestrcpy(backup_txt, "save/backup.txt", sizeof(backup_txt));
		safestrcpy(friends_txt, "save/friends.txt", sizeof(friends_txt));

		backup_txt_flag=0;

		char_name_option=0;
		memset(char_name_letters,0,sizeof(char_name_letters));

		name_ignoring_case=0;

		start_zeny = 500;
		start_weapon = 1201;
		start_armor = 2301;
		safestrcpy(start_point.map, "new_1-1.gat", sizeof(start_point.map));
		start_point.x=53;
		start_point.y=111;

		init(dbcfgfile);
	}
	~CCharDB_sql()	{}


private:

	bool compare_item(struct item *a, struct item *b) {
		return (
		  (a->id == b->id) &&
		  (a->nameid == b->nameid) &&
		  (a->amount == b->amount) &&
		  (a->equip == b->equip) &&
		  (a->identify == b->identify) &&
		  (a->refine == b->refine) &&
		  (a->attribute == b->attribute) &&
		  (a->card[0] == b->card[0]) &&
		  (a->card[1] == b->card[1]) &&
		  (a->card[2] == b->card[2]) &&
		  (a->card[3] == b->card[3]));
	}




	bool char_to_sql(const CCharCharacter& p)
	{

		bool diff,l,status, loaded;
		size_t sz,i,pos;

		loaded = false
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



		sz = sprintf(tmp_sql ,

			"UPDATE `%s` "
			"SET " // Sorted things out so it was easy to read and compare
			"`class`='%d', `base_level`='%d', `job_level`='%d',"
			"`base_exp`='%d', `job_exp`='%d', `zeny`='%d',"

			"`hp`='%d',`max_hp`='%d',"
			"`sp`='%d',`max_sp`='%d',"
			"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
			"`status_point`='%d',`skill_point`='%d',"

			"`option`='%d',`karma`='%d',`manner`='%d',"
			"`party_id`='%d',`guild_id`='%d',"
			"`pet_id`='%d',"

			"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',"
			"`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"

			"`last_map`='%s',`last_x`='%d',`last_y`='%d',"
			"`save_map`='%s',`save_x`='%d',`save_y`='%d',"
			"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d', "

			"`fame`='%d'"

			"WHERE  `account_id`='%ld' AND `char_id` = '%ld' And `slot` = '%d'",

			p.class_, p.base_level, p.job_level,
			p.base_exp,p.job_exp,p.zeny,

			p.hp,p.max_hp,
			p.sp,p.max_sp,
			p.str,p.agi,p.vit,p.int_,p.dex,p.luk,
			p.status_point,p.skill_point

			p.option,p.karma,p.chaos,p.manner
			p.party_id,p.guild_id
			p.pet_id

			p.hair,p.hair_color,p.clothes_color
			p.weapon,p.shield,p.head_top,p.head_mid,p.head_bottom

			p.last_point.map,p.last_point.x,p.last_point.y
			p.save_point.map,p.save_point.x,p.save_point.y
			p.partner_id,p.father_id,p.mother_id,p.child_id

			p.fame_points

			,(unsigned long)p.account_id, (unsigned long)p.char_id p.slot,
			);
		this->mysql_SendQuery(tmp_sql, sz);

		// This is set for overall.. if anything changed.... it will copy over old data... =o
		status = false;
		///////////////////////////////////////////////////////////////////////
		// Memo Insert

		diff = false;
		l = false;

		for(i=0;i<MAX_MEMO;i++)
		{
			if(
				(strcmp(p.memo_point[i].map,cp.memo_point[i].map) == 0) &&
				(p.memo_point[i].x == cp.memo_point[i].x) &&
				(p.memo_point[i].y == cp.memo_point[i].y)
				)continue;
			diff = true;
			status = true;
			break;
		}

		if (diff)
		{
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",memo_db, p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`,`memo_id`,`map`,`x`,`y`) VALUES ", memo_db);
			for(i=0;i<MAX_MEMO;i++)
			{
				if(p->memo_point[i].map[0])
				{
					sz +=sprintf(tmp_sql,"%s%s('%ld', '%d', '%s', '%d', '%d')",
						tmp_sql,l?",":"",
						(unsigned long)char_id, i, p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
					l= true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}


		///////////////////////////////////////////////////////////////////////
		// Inventory Insert

		diff = false;
		l = false;

		for(i = 0; i<MAX_INVENTORY; i++)
		{
			if (!compare_item(&p.inventory[i], &cp.inventory[i]))
			{
				diff = true;
				status = true;
				break;
			}
		}

		if (diff)
		{
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",cart_db, p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ", inventory_db);
			for(i=0;i<MAX_INVENTORY;i++)
			{
				if(p->inventory[i].nameid>0)
				{
					sz +=sprintf(tmp_sql,
						"%s%s('%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
						tmp_sql,l?",":"",
						(unsigned long)char_id,
						p.inventory[i].nameid,
						p.inventory[i].amount,
						p.inventory[i].equip,
						p.inventory[i].identify,
						p.inventory[i].refine,
						p.inventory[i].attribute,
						p.inventory[i].card[0],
						p.inventory[i].card[1],
						p.inventory[i].card[2],
						p.inventory[i].card[3]);
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}


		///////////////////////////////////////////////////////////////////////
		// Cart Insert

		diff = false;
		l = false;

		for(i = 0; i<MAX_CART; i++)
		{
			if (!compare_item(&p.cart[i], &cp.cart[i]))
			{
				diff = true;
				status = true;
				break;
			}
		}

		if (diff)
		{
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",cart_db, p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ", inventory_db);			for(i=0;i<MAX_CART;i++)
			{
				if(p->cart[i].nameid>0)
				{
					sz +=sprintf(tmp_sql,
						"%s%s('%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
						tmp_sql,l?",":"",
						(unsigned long)p.char_id,
						p.cart[i].nameid,
						p.cart[i].amount,
						p.cart[i].equip,
						p.cart[i].identify,
						p.cart[i].refine,
						p.cart[i].attribute,
						p.cart[i].card[0],
						p.cart[i].card[1],
						p.cart[i].card[2],
						p.cart[i].card[3]);
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}

		///////////////////////////////////////////////////////////////////////
		// Skill Insert

		diff = false;
		l = false;

		for(i=0;i<MAX_SKILL;i++)
		{
			if(
				(p.skill[i].lv != 0) &&
				(p.skill[i].id == 0)
				)p.skill[i].id = i; // Fix skill tree

			if(
				(p.skill[i].id == cp.skill[i].id) ||
				(p.skill[i].lv == cp.skill[i].lv) ||
				(p.skill[i].flag == cp.skill[i].flag)
				) continue;

			diff = true;
			status = true;
			break;
		}


		if (diff)
		{
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%ld'",skill_db, (unsigned long)p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`,`id`,`lvl`) VALUES ", skill_db);
			for(i=0;i<MAX_MEMO;i++)
			{
				if(p.skill[i].id>0 && p.skill[i].flag != 1)
				{
					sz +=sprintf(tmp_sql,"%s%s('%ld', '%d', '%d')",
						tmp_sql,l?",":"",
						(unsigned long)p.char_id, i, p.skill[i].id, p.skill[i].lv);
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}

		///////////////////////////////////////////////////////////////////////
		// Character Reg Insert

		diff = false;
		l = false;

		for(i=0;i<p.global_reg_num;i++)
		{
			if(
				((p->global_reg[i].str == NULL) == (cp->global_reg[i].str == NULL)) ||
				(p->global_reg[i].value == cp->global_reg[i].value) ||
				strcmp(p->global_reg[i].str, cp->global_reg[i].str) == 0
				)continue;
			diff = true;
			status = true;
			break;
		}


		if (diff)
		{
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%ld'",char_reg_db, (unsigned long)p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`,`str`,`value`) VALUES ", char_reg_db);
			for(i=0;i<p.global_reg_num;i++)
			{
				if(p.global_reg[i].str && p.global_reg[i].value !=0)
				{
					sz +=sprintf(tmp_sql,"%s%s('%ld', '%d', '%d')",
						tmp_sql,l?",":"",
						(unsigned long)p.char_id, i, p.global_reg[i].str, p.global_reg[i].value);
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}


		///////////////////////////////////////////////////////////////////////
		// Friends Insert

		diff = false;
		l = false;

		for (i=0; i<20; i++)
		{
			if(
				p.friend_id == cp.friend_id
				)continue;
			diff = true;
			status = true;
			break;
		}

		if(diff)
			sz = sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id` = '%ld'", friend_db, (unsigned long)p.char_id);
			this->mysql_SendQuery(tmp_sql, sz);

			//insert here.
			sz = sprintf(tmp_sql,"INSERT INTO `%s` (`char_id`, `friend_id`) VALUES ", friend_db);
			for(i=0;i<20;i++)
			{
				if(p.friendlist[i].id!=0)
				{
					sz +=sprintf(tmp_sql,"%s%s('%ld', '%ld')",
						tmp_sql,l?",":"",
						(unsigned long)p.char_id, (unsigned long)p.friendlist[i].friend_id);
					l = true;
				}
			}
			// if at least one entry spotted.
			if(l)this->mysql_SendQuery(tmp_sql, sz);
		}

		if (status) // If anything has changed in this process, set to true in any of the differences check.
		{
			if(loaded)cCharList.removeindex(pos, 0); // remove old char_id where ever the &cp index is pointing to
			cCharList.insert(p); // now lets add our CharID to the index list.
		}

		return true;
	}

/*	bool char_from_sql(const char *str)
	{

		int tmp_int[256];
		int next, len;
		size_t i,n;
		CCharCharacter p;

		// initilialise character
		memset(&p, 0, sizeof(CCharCharacter));
		memset(tmp_int, 0, sizeof(tmp_int));



		int i, n;
		struct mmo_charstatus *cp;

		cp = (struct mmo_charstatus*)numdb_search(char_db_,char_id);
		if (cp != NULL)
		  aFree(cp);

		memset(p, 0, sizeof(struct mmo_charstatus));

		p->char_id = char_id;
	#ifdef CHAR_DEBUG_INFO
		printf("Loaded: ");
	#endif
		//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
		//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
		//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
		//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
		//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
		//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
		//splite 2 parts. cause veeeery long SQL syntax

		sprintf(tmp_sql, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
			"`str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point` FROM `%s` WHERE `char_id` = '%d'",char_db, char_id); // TBR

		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
		}

		sql_res = mysql_store_result(&mysql_handle);

		if (sql_res) {
			sql_row = mysql_fetch_row(sql_res);

			p->char_id = char_id;
			p->account_id = atoi(sql_row[1]);
			p->char_num = atoi(sql_row[2]);
			strcpy(p->name, sql_row[3]);
			p->class_ = atoi(sql_row[4]);
			p->base_level = atoi(sql_row[5]);
			p->job_level = atoi(sql_row[6]);
			p->base_exp = atoi(sql_row[7]);
			p->job_exp = atoi(sql_row[8]);
			p->zeny = atoi(sql_row[9]);
			p->str = atoi(sql_row[10]);
			p->agi = atoi(sql_row[11]);
			p->vit = atoi(sql_row[12]);
			p->int_ = atoi(sql_row[13]);
			p->dex = atoi(sql_row[14]);
			p->luk = atoi(sql_row[15]);
			p->max_hp = atoi(sql_row[16]);
			p->hp = atoi(sql_row[17]);
			p->max_sp = atoi(sql_row[18]);
			p->sp = atoi(sql_row[19]);
			p->status_point = atoi(sql_row[20]);
			p->skill_point = atoi(sql_row[21]);
			//free mysql result.
			mysql_free_result(sql_res);
		} else
			printf("char1 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?
	#ifdef CHAR_DEBUG_INFO
		printf("(\033[1;32m%d\033[0m)\033[1;32m%s\033[0m\t[",p->char_id,p->name);
		printf("char1 ");
	#endif

		sprintf(tmp_sql, "SELECT `option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`,"
			"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,"
			"`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`, `partner_id`, `father`, `mother`, `child`, `fame`"
			"FROM `%s` WHERE `char_id` = '%d'",char_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `char2`)- %s\n", mysql_error(&mysql_handle));
		}

		sql_res = mysql_store_result(&mysql_handle);
		if (sql_res) {
			sql_row = mysql_fetch_row(sql_res);


			p->option = atoi(sql_row[0]);	p->karma = atoi(sql_row[1]);	p->manner = atoi(sql_row[2]);
				p->party_id = atoi(sql_row[3]);	p->guild_id = atoi(sql_row[4]);	p->pet_id = atoi(sql_row[5]);

			p->hair = atoi(sql_row[6]);	p->hair_color = atoi(sql_row[7]);	p->clothes_color = atoi(sql_row[8]);
			p->weapon = atoi(sql_row[9]);	p->shield = atoi(sql_row[10]);
			p->head_top = atoi(sql_row[11]);	p->head_mid = atoi(sql_row[12]);	p->head_bottom = atoi(sql_row[13]);
			strcpy(p->last_point.map,sql_row[14]); p->last_point.x = atoi(sql_row[15]);	p->last_point.y = atoi(sql_row[16]);
			strcpy(p->save_point.map,sql_row[17]); p->save_point.x = atoi(sql_row[18]);	p->save_point.y = atoi(sql_row[19]);
			p->partner_id = atoi(sql_row[20]); p->father = atoi(sql_row[21]); p->mother = atoi(sql_row[22]); p->child = atoi(sql_row[23]);
			p->fame = atoi(sql_row[24]);

			//free mysql result.
			mysql_free_result(sql_res);
		} else
			printf("char2 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?

		if (p->last_point.x == 0 || p->last_point.y == 0 || p->last_point.map[0] == '\0')
		{
			char errbuf[64];
			sprintf(errbuf,"%s has no last point?\n",p->name);
			memcpy(&p->last_point, &start_point, sizeof(start_point));
		}

		if (p->save_point.x == 0 || p->save_point.y == 0 || p->save_point.map[0] == '\0')
		{
			char errbuf[64];
			sprintf(errbuf,"%s has no save point?\n",p->name);
			memcpy(&p->save_point, &start_point, sizeof(start_point));
		}
	#ifdef CHAR_DEBUG_INFO
		printf("char2 ");
	#endif
		//read memo data
		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		sprintf(tmp_sql, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`='%d'",memo_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `memo`)- %s\n", mysql_error(&mysql_handle));
		}
		sql_res = mysql_store_result(&mysql_handle);

		if (sql_res) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res)) && i < 3;i++){
				strcpy (p->memo_point[i].map,sql_row[0]);
				p->memo_point[i].x=atoi(sql_row[1]);
				p->memo_point[i].y=atoi(sql_row[2]);
				//i ++;
			}
			mysql_free_result(sql_res);
		}
	#ifdef CHAR_DEBUG_INFO
		printf("memo ");
	#endif

		//read inventory
		//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `gm_made`)
		sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `gm_made`"
			"FROM `%s` WHERE `char_id`='%d'",inventory_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `inventory`)- %s\n", mysql_error(&mysql_handle));
		}
		sql_res = mysql_store_result(&mysql_handle);
		if (sql_res) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
				p->inventory[i].id = atoi(sql_row[0]);
				p->inventory[i].nameid = atoi(sql_row[1]);
				p->inventory[i].amount = atoi(sql_row[2]);
				p->inventory[i].equip = atoi(sql_row[3]);
				p->inventory[i].identify = atoi(sql_row[4]);
				p->inventory[i].refine = atoi(sql_row[5]);
				p->inventory[i].attribute = atoi(sql_row[6]);
				p->inventory[i].card[0] = atoi(sql_row[7]);
				p->inventory[i].card[1] = atoi(sql_row[8]);
				p->inventory[i].card[2] = atoi(sql_row[9]);
				p->inventory[i].card[3] = atoi(sql_row[10]);
				p->inventory[i].gm_made = atoi(sql_row[11]);
			}
			mysql_free_result(sql_res);
		}
	#ifdef CHAR_DEBUG_INFO
		printf("inventory ");
	#endif

		//read cart.
		//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
		sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `gm_made`"
			"FROM `%s` WHERE `char_id`='%d'",cart_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `cart_inventory`)- %s\n", mysql_error(&mysql_handle));
		}
		sql_res = mysql_store_result(&mysql_handle);
		if (sql_res) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
				p->cart[i].id = atoi(sql_row[0]);
				p->cart[i].nameid = atoi(sql_row[1]);
				p->cart[i].amount = atoi(sql_row[2]);
				p->cart[i].equip = atoi(sql_row[3]);
				p->cart[i].identify = atoi(sql_row[4]);
				p->cart[i].refine = atoi(sql_row[5]);
				p->cart[i].attribute = atoi(sql_row[6]);
				p->cart[i].card[0] = atoi(sql_row[7]);
				p->cart[i].card[1] = atoi(sql_row[8]);
				p->cart[i].card[2] = atoi(sql_row[9]);
				p->cart[i].card[3] = atoi(sql_row[10]);
				p->cart[i].gm_made = atoi(sql_row[11]);
			}
			mysql_free_result(sql_res);
		}
	#ifdef CHAR_DEBUG_INFO
		printf("cart ");
	#endif
		//read skill
		//`skill` (`char_id`, `id`, `lv`)
		sprintf(tmp_sql, "SELECT `id`, `lv` FROM `%s` WHERE `char_id`='%d'",skill_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `skill`)- %s\n", mysql_error(&mysql_handle));
		}
		sql_res = mysql_store_result(&mysql_handle);
		if (sql_res) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
				n = atoi(sql_row[0]);
				p->skill[n].id = n; //memory!? shit!.
				p->skill[n].lv = atoi(sql_row[1]);
			}
			mysql_free_result(sql_res);
		}
	#ifdef CHAR_DEBUG_INFO
		printf("skill ");
	#endif

		//global_reg
		//`global_reg_value` (`char_id`, `str`, `value`)
		sprintf(tmp_sql, "SELECT `str`, `value` FROM `%s` WHERE `char_id`='%d'",char_reg_db, char_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (select `global_reg_value`)- %s\n", mysql_error(&mysql_handle));
		}
		i = 0;
		sql_res = mysql_store_result(&mysql_handle);
		if (sql_res) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
				strcpy (p->global_reg[i].str, sql_row[0]);
				p->global_reg[i].value = atoi (sql_row[1]);
			}
			mysql_free_result(sql_res);
		}
		p->global_reg_num=i;

		//Friends List Load

		for(i=0;i<20;i++) {
			p->friend_id[i] = 0;
			p->friend_name[i][0] = '\0';
		}

		sprintf(tmp_sql,"SELECT f.`friend_id`, c.`name` FROM `%s` f JOIN `%s` c ON f.`friend_id`=c.`char_id` WHERE f.`char_id` = '%d'",friend_db, char_db,char_id);

		if (mysql_query(&mysql_handle, tmp_sql))
		{
			printf("DB server Error (select `friends list`)- %s\n", mysql_error(&mysql_handle));
		}

		sql_res = mysql_store_result(&mysql_handle);

		if (sql_res)
		{
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++)
			{
				p->friend_id[i] = atoi(sql_row[0]);
				sprintf(p->friend_name[i], "%s", sql_row[1]);
			}
			mysql_free_result(sql_res);
		}

	#ifdef CHAR_DEBUG_INFO
		printf("friends ");
	#endif
		//-- end friends list load --

		if (online) {
			set_char_online(char_id,p->account_id);
		}
		return 1;
	}
	}
*/
	///////////////////////////////////////////////////////////////////////////
	// Function to create a new character
	bool make_new_char(CCharCharAccount& account,
						const char *n, // Name
						unsigned char str,
						unsigned char agi,
						unsigned char vit,
						unsigned char int_,
						unsigned char dex,
						unsigned char luk,
						unsigned char slot,
						unsigned char hair_style,
						unsigned char hair_color)
	{
		CCharCharacter tempchar(n);
		MYSQL_RES *sql_res=NULL;
		MYSQL_ROW sql_row;
		char tmp_sql [1024];
		char t_name[100];

		mysql_real_escape_string(&mysql_handle, t_name, tempchar.name, strlen(tempchar.name));

		//check stat error
		if (
			(str + agi + vit + int_ + dex + luk !=6*5 ) || // stats

			// Check slots
			(slot >= 9) || (slot < 0 ) || // slots can not be negative or over 9

			// Check hair
			(hair_style <= 0) || (hair_style >= 24) || // hair style
			(hair_color >= 9) || (hair_color < 0) ||   // Hair color?

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
			printf("fail (aid: %d), stats error(bot cheat?!)\n", sd->account_id);
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
		sz = sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `name` = '%s'",char_db, t_name);
		if ( this->mysql_SendQuery(sql_res, tmp_sql, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if (atol(sql_row[0]))
				printf("fail, charname \"%s\" already in use\n", sql_row[0]);

			mysql_free_result(sql_res);
			return false;
		}


		// check char slot.
		sz = sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `account_id` = '%d' AND `char_num` = '%d'",char_db, account.account_id, slot);
		if ( this->mysql_SendQuery(sql_res, tmp_sql, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if(atol(sql_row[0]))
				printf("fail (aid: %d, slot: %d), slot already in use\n", sd->account_id, dat[30]);

			mysql_free_result(sql_res);
			return false;
		}

		// It has passed both the name and slot check, let's insert the info since it doesnt conflict =D
		// make new char.
		sz = sprintf(tmp_sql,
				"INSERT INTO `%s` "
					"(`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`) "
					"VALUES "
					"('%ld'        ,'%d'      ,'%s'  ,'%d' ,'%d' ,'%d' ,'%d' ,'%d' ,'%d' ,'%d'  ,'%d'        )",
				 char_db,
				 (unsigned long)account.account_id , slot, t_name,  str, agi, vit, int_, dex, luk, hair_style, hair_color);

		this->mysql_SendQuery(tmp_sql, sz)


		//Now we need the charid from sql!
		sz = sprintf(tmp_sql, "SELECT `char_id` FROM `%s` WHERE `account_id` = '%ld' AND `char_num` = '%d' AND `name` = '%s'", char_db, (unsigned long)account.account_id, slot, t_name);

		if( this->mysql_SendQuery(sql_res, tmp_sql, sz) )
		{
			sql_row = mysql_fetch_row(sql_res);
			if (sql_row)
				tempchar.char_id = atol(sql_row[0]); //char id :)
			mysql_free_result(sql_res);
		}

		//Give the char the default items
		//knife & cotton shirts, add on as needed ifmore items are to be included.
		sz = sprintf(tmp_sql,
					"INSERT INTO `%s` "
						"(`char_id`,`nameid`, `amount`, `equip`, `identify`) "
						"VALUES "
						"('%ld', '%d', '%d', '%d', '%d'),"
						"('%ld', '%d', '%d', '%d', '%d')",
						inventory_db,
						(unsigned long)tempchar.char_id, start_weapon,1,0x02,1, //add Knife
						(unsigned long)tempchar.char_id, start_armor, 1,0x10,1  //add cotton shirts
						);

		this->mysql_SendQuery(tmp_sql, sz);


		// Update the map they are starting on and where they respawn at.
		sz = sprintf(tmp_sql,
					"UPDATE `%s` "
					"SET `last_map` = '%s', `last_x` = '%d', `last_y` = '%d', "
					"    `save_map` = '%s', `save_x` = '%d', `save_y` = '%d'  "
					"WHERE `char_id` = '%ld'",
					char_db,
					start_point.map, start_point.x, start_point.y,
					start_point.map, start_point.x, start_point.y,
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
		memcpy(&tempchar.last_point, &start_point, sizeof(start_point));
		memcpy(&tempchar.save_point, &start_point, sizeof(start_point));


		account.charlist[slot] = tempchar.char_id;

		cCharList.insert(tempchar);

		return true;
	}

	bool read_friends()
	{
		MYSQL_RES *sql_res=NULL, *sql_res2=NULL;
		MYSQL_ROW sql_row, sql_row2;
		char tmp_sql [1024];
		unsigned long cid=0;
		unsigned long fid=0;
		struct friends friendlist[20];
		size_t pos, i;

		// SELECT f.`char_id`,f.`friend_id`,c.`name` FROM `lege_friend` f JOIN `lege_char` c ON f.`friend_id`=c.`char_id` ORDER BY f.char_id DESC, f.friend_id DESC
		sz = sprintf(tmp_sql,"SELECT `char_id` FROM `%s` GROUP BY `char_id` ORDER BY `char_id` ASC", friend_db);

		if( this->mysql_SendQuery(sql_res, tmp_sql, sz) )
		{
			while( (sql_row = mysql_fetch_row(sql_res)) )
			{

				if( cCharList.find( CCharCharacter(atol(sql_row[0])), pos, 0) )
				{

					CCharCharacter &temp = cCharList(pos,0);
					sz = sprintf(tmp_sql,"SELECT f.`friend_id`, c.`name` FROM `%s` f JOIN `%s` c ON f.`friend_id` = c.`char_id` WHERE f.`char_id` = '%ld'",friend_db, char_db,atol(sql_row[0]) );

					if( this->mysql_SendQuery(sql_res2, tmp_sql, sz) )
					{
						i=0;

						for (i<MAX_FRIENDLIST && (sql_row2 = mysql_fetch_row(sql_res2)))
						{
							temp.friendlist[i].friend_id   = atol(sql_row2[0]);
							safestrcpy(temp.friendlist[i].friend_name,sql_row2[1],sizeof(temp.friendlist[0].friend_name) );
							i++
						}
						mysql_free_result(sql_res2)
					}
				}
			}

			mysql_free_result(sql_res);
		}

		return true;

		// This is the query when searching to load friends list via char_id... which is what this will be replaced with.
		// sz = sprintf(tmp_sql,"SELECT f.`char_id`,f.`friend_id`,c.`name` FROM `%s` f JOIN `%s` c ON f.`friend_id`=c.`char_id`", friend_db, char_db);
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cCharList.size(); }
	virtual CCharCharacter& operator[](size_t i)	{ return cCharList[i]; }

	virtual bool existChar(const char* name);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(uint32 charid, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(uint32 charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(uint32 accid);

private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		CConfig::LoadConfig(configfile);
		return read_chars() && read_friends();
	}
	bool close()
	{
		return save_chars() && save_friends();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 100 )
		{
			savecount=0;
//			save_chars();
			save_friends();
		}

		//!! todo check changes in files and reload them if necessary

		return true;
	}
};


bool CCharDB_sql::ProcessConfig(const char*w1, const char*w2)
{
	if(strcasecmp(w1, "char_txt") == 0)
	{
		safestrcpy(char_txt, w2, sizeof(char_txt));
	}
	else if(strcasecmp(w1, "backup_txt") == 0)
	{
		safestrcpy(backup_txt, w2, sizeof(backup_txt));
	}
	else if(strcasecmp(w1, "friends_txt") == 0)
	{
		safestrcpy(friends_txt, w2, sizeof(friends_txt));
	}
	else if(strcasecmp(w1, "backup_txt_flag") == 0)
	{
		backup_txt_flag = Switch(w2);
	}
	else if(strcasecmp(w1, "autosave_time") == 0)
	{
		// less then 10 seconds and more than an hour is not realistic
		CTimerBase::init( SwitchValue(w2, 10, 3600)*1000 );
	}
	else if(strcasecmp(w1, "start_point") == 0)
	{
		char map[32];
		int x, y;
		if(sscanf(w2, "%[^,],%d,%d", map, &x, &y) == 3 &&  NULL!=strstr(map, ".gat") )
		{	// Verify at least if '.gat' is in the map name
			safestrcpy(start_point.map, map, sizeof(start_point.map));
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
	size_t pos;
	return cCharList.find( CCharCharacter(name), pos, 1);
}
bool CCharDB_sql::searchChar(const char* name, CCharCharacter&data)
{
	// select char_data where name = *name
	size_t pos;
	if( cCharList.find( CCharCharacter(name), pos, 1) )
	{
		//if found char_id
		//select char_data where char_id =.....  why not just put that in the first query, if not found, return false
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_sql::searchChar(uint32 charid, CCharCharacter&data)
{
	// select char_data where char_id = *char_id
	size_t pos;
	if( cCharList.find( CCharCharacter(charid), pos, 0) )
	{
		//if found char_id do like i said for search char
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_sql::insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)
{

	// insert into database all the data needed to finish creating a character, return false, if unfinished. return character data when complete
	size_t pos;
	if( cAccountList.find(account,0,pos) )
	{
		make_new_char(cAccountList[pos], name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color)
		account = cAccountList[pos];
		return searchChar(name, data);
	}
	return false;
}
bool CCharDB_sql::removeChar(uint32 charid)
{
	size_t sz=sprintf(query, "DELETE FROM `%s` WHERE `char_id`='%d'", char_character_db, charid);
	this->mysql_SendQuery(query, sz);

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
	size_t pos;
	if( cAccountList.find(CCharCharAccount(accid),0,pos) )
	{
		account = cAccountList[pos];
		return true;
	}
	return false;
}
bool CCharDB_sql::saveAccount(CCharAccount& account)
{
	// Unknown function, can't find relative info for cAccountList.insert()
	size_t pos;
	if( cAccountList.find(account,0,pos) )lol
	{	// exist -> update list entry
		cAccountList[pos].CCharAccount::operator=(account);
		return true;
	}
	else
	{	// create new
		return cAccountList.insert(account);
	}
}
bool CCharDB_sql::removeAccount(uint32 accid)
{

	size_t sz = sprintf(tmp_sql, "DELETE FROM `%s` WHERE `account_id` = '%ld'", (unsigned long) accid);
	this->mysql_SendQuery(tmp_sql, sz);

	size_t pos;
	if( cAccountList.find(CCharAccount(accid),0,pos) )
	{	// exist -> update list entry
		cAccountList.removeindex(pos);
	}
	return true;
}

CCharDBInterface* CCharDB::getDB(const char *dbcfgfile)
{
	return new CCharDB_sql(dbcfgfile);
}



*/



class CGuildDB_sql : public CTimerBase, private CConfig, public CGuildDBInterface
{
	bool string2guild(const char *str, CGuild &g)
	{
		int i, j, c;
		int tmp_int[16];
		char tmp_str[4][256];
		char tmp_str2[4096];
		char *pstr;

		// 基本データ
		if( 8 > sscanf(str,
			"%d\t%[^\t]\t%[^\t]\t%d,%d,%d,%d,%d\t%[^\t]\t%[^\t]\t",
			&tmp_int[0],
			tmp_str[0], tmp_str[1],
			&tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5],
			tmp_str[2], tmp_str[3]) )
		{
			return false;
		}

		g.guild_id = tmp_int[0];
		g.guild_lv = tmp_int[1];
		g.max_member = tmp_int[2];
		g.exp = tmp_int[3];
		g.skill_point = tmp_int[4];
		g.castle_id = tmp_int[5];
		safestrcpy(g.name, tmp_str[0], sizeof(g.name));
		safestrcpy(g.master, tmp_str[1], sizeof(g.master));
		safestrcpy(g.mes1, tmp_str[2], sizeof(g.mes1));
		safestrcpy(g.mes2, tmp_str[3], sizeof(g.mes2));

		for(j=0; j<6 && str!=NULL; j++)	// 位置スキップ
			str = strchr(str + 1, '\t');

		// メンバー
		if(g.max_member>MAX_GUILD)
			g.max_member=0;
		for(i=0; i<g.max_member; i++)
		{
			struct guild_member &m = g.member[i];

			if( sscanf(str+1, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\t%[^\t]\t",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4],
				&tmp_int[5], &tmp_int[6], &tmp_int[7], &tmp_int[8], &tmp_int[9],
				tmp_str[0]) < 11)
				return false;

			m.account_id = tmp_int[0];
			m.char_id = tmp_int[1];
			m.hair = tmp_int[2];
			m.hair_color = tmp_int[3];
			m.gender = tmp_int[4];
			m.class_ = tmp_int[5];
			m.lv = tmp_int[6];
			m.exp = tmp_int[7];
			m.exp_payper = tmp_int[8];
			m.position = tmp_int[9];
			safestrcpy(m.name, tmp_str[0], sizeof(m.name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
		}

		// 役職
		i = 0;
		while( sscanf(str+1, "%d,%d%n", &tmp_int[0], &tmp_int[1], &j) == 2 &&
			str[1+j] == '\t')
		{
			struct guild_position &p = g.position[i];
			if( sscanf(str+1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
				return false;
			p.mode = tmp_int[0];
			p.exp_mode = tmp_int[1];
			safestrcpy(p.name, tmp_str[0], sizeof(p.name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
			i++;
		}

		// エンブレム
		tmp_int[1] = 0;
		if( sscanf(str + 1, "%d,%d,%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str2)< 3 &&
			sscanf(str + 1, "%d,%[^\t]\t", &tmp_int[0], tmp_str2) < 2 )
			return false;
		g.emblem_len = tmp_int[0];
		g.emblem_id = tmp_int[1];
		for(i=0, pstr=tmp_str2; i<g.emblem_len; i++, pstr+=2)
		{
			int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
			if (c1 >= '0' && c1 <= '9') x1 = c1 - '0';
			if (c1 >= 'a' && c1 <= 'f') x1 = c1 - 'a' + 10;
			if (c1 >= 'A' && c1 <= 'F') x1 = c1 - 'A' + 10;
			if (c2 >= '0' && c2 <= '9') x2 = c2 - '0';
			if (c2 >= 'a' && c2 <= 'f') x2 = c2 - 'a' + 10;
			if (c2 >= 'A' && c2 <= 'F') x2 = c2 - 'A' + 10;
			g.emblem_data[i] = (x1<<4) | x2;
		}

		str=strchr(str+1, '\t');	// 位置スキップ

		// 同盟リスト
		if (sscanf(str+1, "%d\t", &c) < 1)
			return false;

		str = strchr(str + 1, '\t');	// 位置スキップ
		for(i = 0; i < c; i++)
		{
			struct guild_alliance &a = g.alliance[i];
			if( sscanf(str + 1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
				return false;
			a.guild_id = tmp_int[0];
			a.opposition = tmp_int[1];
			safestrcpy(a.name, tmp_str[0], sizeof(a.name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str + 1, '\t');
		}

		// 追放リスト
		if (sscanf(str+1, "%d\t", &c) < 1)
			return false;

		str = strchr(str + 1, '\t');	// 位置スキップ
		for(i=0; i<c; i++)
		{
			struct guild_explusion &e = g.explusion[i];
			if( sscanf(str + 1, "%d,%d,%d,%d\t%[^\t]\t%[^\t]\t%[^\t]\t",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				tmp_str[0], tmp_str[1], tmp_str[2]) < 6)
				return false;
			e.account_id = tmp_int[0];
			e.rsv1 = tmp_int[1];
			e.rsv2 = tmp_int[2];
			e.rsv3 = tmp_int[3];
			safestrcpy(e.name, tmp_str[0], sizeof(e.name));
			safestrcpy(e.acc, tmp_str[1], sizeof(e.acc));
			safestrcpy(e.mes, tmp_str[2], sizeof(e.mes));

			for(j=0; j<4 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
		}

		// ギルドスキル
		for(i=0; i<MAX_GUILDSKILL; i++)
		{
			if (sscanf(str+1,"%d,%d ", &tmp_int[0], &tmp_int[1]) < 2)
				break;
			g.skill[i].id = tmp_int[0];
			g.skill[i].lv = tmp_int[1];
			str = strchr(str+1, ' ');
		}
		str = strchr(str + 1, '\t');

		return true;
	}

	ssize_t guild2string(char *str, size_t maxlen, const CGuild &g)
	{
		ssize_t i, c, len;

		// 基本データ
		len = sprintf(str, "%ld\t%s\t%s\t%d,%d,%ld,%d,%d\t%s#\t%s#\t",
					  (unsigned long)g.guild_id, g.name, g.master,
					  g.guild_lv, g.max_member, (unsigned long)g.exp, g.skill_point, g.castle_id,
					  g.mes1, g.mes2);
		// メンバー
		for(i = 0; i < g.max_member; i++) {
			const struct guild_member &m = g.member[i];
			len += sprintf(str + len, "%ld,%ld,%d,%d,%d,%d,%d,%ld,%ld,%d\t%s\t",
						   (unsigned long)m.account_id, (unsigned long)m.char_id,
						   m.hair, m.hair_color, m.gender,
						   m.class_, m.lv, (unsigned long)m.exp, (unsigned long)m.exp_payper, m.position,
						   ((m.account_id > 0) ? m.name : "-"));
		}
		// 役職
		for(i = 0; i < MAX_GUILDPOSITION; i++) {
			const struct guild_position &p = g.position[i];
			len += sprintf(str + len, "%ld,%ld\t%s#\t", (unsigned long)p.mode, (unsigned long)p.exp_mode, p.name);
		}
		// エンブレム
		len += sprintf(str + len, "%d,%ld,", g.emblem_len, (unsigned long)g.emblem_id);
		for(i = 0; i < g.emblem_len; i++) {
			len += sprintf(str + len, "%02x", (unsigned char)(g.emblem_data[i]));
		}
		len += sprintf(str + len, "$\t");
		// 同盟リスト
		c = 0;
		for(i = 0; i < MAX_GUILDALLIANCE; i++)
			if (g.alliance[i].guild_id > 0)
				c++;
		len += sprintf(str + len, "%d\t", c);
		for(i = 0; i < MAX_GUILDALLIANCE; i++) {
			const struct guild_alliance &a = g.alliance[i];
			if (a.guild_id > 0)
				len += sprintf(str + len, "%ld,%ld\t%s\t", (unsigned long)a.guild_id, (unsigned long)a.opposition, a.name);
		}
		// 追放リスト
		c = 0;
		for(i = 0; i < MAX_GUILDEXPLUSION; i++)
			if (g.explusion[i].account_id > 0)
				c++;
		len += sprintf(str + len, "%d\t", c);
		for(i = 0; i < MAX_GUILDEXPLUSION; i++) {
			const struct guild_explusion &e = g.explusion[i];
			if (e.account_id > 0)
				len += sprintf(str + len, "%ld,%ld,%ld,%ld\t%s\t%s\t%s#\t",
							   (unsigned long)e.account_id, (unsigned long)e.rsv1, (unsigned long)e.rsv2, (unsigned long)e.rsv3,
							   e.name, e.acc, e.mes );
		}
		// ギルドスキル
		for(i = 0; i < MAX_GUILDSKILL; i++) {
			len += sprintf(str + len, "%d,%d ", g.skill[i].id, g.skill[i].lv);
		}
		len += sprintf(str+len, "\t"RETCODE);
		return len;
	}

	ssize_t castle2string(char *str, size_t maxlen, const CCastle &gc)
	{
		ssize_t len;
		len = snprintf(str, maxlen, "%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld"RETCODE,	// added Guardian HP [Valaris]
					  gc.castle_id, (unsigned long)gc.guild_id, (unsigned long)gc.economy, (unsigned long)gc.defense, (unsigned long)gc.triggerE,
					  (unsigned long)gc.triggerD, (unsigned long)gc.nextTime, (unsigned long)gc.payTime, (unsigned long)gc.createTime, (unsigned long)gc.visibleC,
					  (unsigned long)gc.guardian[0].visible, (unsigned long)gc.guardian[1].visible, (unsigned long)gc.guardian[2].visible, (unsigned long)gc.guardian[3].visible,
					  (unsigned long)gc.guardian[4].visible, (unsigned long)gc.guardian[5].visible, (unsigned long)gc.guardian[6].visible, (unsigned long)gc.guardian[7].visible,
					  (unsigned long)gc.guardian[0].guardian_hp, (unsigned long)gc.guardian[1].guardian_hp, (unsigned long)gc.guardian[2].guardian_hp, (unsigned long)gc.guardian[3].guardian_hp,
					  (unsigned long)gc.guardian[4].guardian_hp, (unsigned long)gc.guardian[5].guardian_hp, (unsigned long)gc.guardian[6].guardian_hp, (unsigned long)gc.guardian[7].guardian_hp);
		return len;
	}

	// ギルド城データの文字列からの変換
	bool string2castle(char *str, CCastle &gc)
	{
		int tmp_int[26];
		memset(tmp_int, 0, sizeof(tmp_int));
		if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				   &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
				   &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
				   &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18], &tmp_int[19], &tmp_int[20],
				   &tmp_int[21], &tmp_int[22], &tmp_int[23], &tmp_int[24], &tmp_int[25]) == 26) {
			gc.castle_id = tmp_int[0];
			gc.guild_id = tmp_int[1];
			gc.economy = tmp_int[2];
			gc.defense = tmp_int[3];
			gc.triggerE = tmp_int[4];
			gc.triggerD = tmp_int[5];
			gc.nextTime = tmp_int[6];
			gc.payTime = tmp_int[7];
			gc.createTime = tmp_int[8];
			gc.visibleC = tmp_int[9];
			gc.guardian[0].visible = tmp_int[10];
			gc.guardian[1].visible = tmp_int[11];
			gc.guardian[2].visible = tmp_int[12];
			gc.guardian[3].visible = tmp_int[13];
			gc.guardian[4].visible = tmp_int[14];
			gc.guardian[5].visible = tmp_int[15];
			gc.guardian[6].visible = tmp_int[16];
			gc.guardian[7].visible = tmp_int[17];
			gc.guardian[0].guardian_hp = tmp_int[18];
			gc.guardian[1].guardian_hp = tmp_int[19];
			gc.guardian[2].guardian_hp = tmp_int[20];
			gc.guardian[3].guardian_hp = tmp_int[21];
			gc.guardian[4].guardian_hp = tmp_int[22];
			gc.guardian[5].guardian_hp = tmp_int[23];
			gc.guardian[6].guardian_hp = tmp_int[24];
			gc.guardian[7].guardian_hp = tmp_int[25];	// end additions [Valaris]
		// old structure of guild castle
		} else if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
						  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
						  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
						  &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17]) == 18) {
			gc.castle_id = tmp_int[0];
			gc.guild_id = tmp_int[1];
			gc.economy = tmp_int[2];
			gc.defense = tmp_int[3];
			gc.triggerE = tmp_int[4];
			gc.triggerD = tmp_int[5];
			gc.nextTime = tmp_int[6];
			gc.payTime = tmp_int[7];
			gc.createTime = tmp_int[8];
			gc.visibleC = tmp_int[9];
			gc.guardian[0].visible = tmp_int[10];
			gc.guardian[1].visible = tmp_int[11];
			gc.guardian[2].visible = tmp_int[12];
			gc.guardian[3].visible = tmp_int[13];
			gc.guardian[4].visible = tmp_int[14];
			gc.guardian[5].visible = tmp_int[15];
			gc.guardian[6].visible = tmp_int[16];
			gc.guardian[7].visible = tmp_int[17];
			gc.guardian[0].guardian_hp = (gc.guardian[0].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[1].guardian_hp = (gc.guardian[1].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[2].guardian_hp = (gc.guardian[2].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[3].guardian_hp = (gc.guardian[3].visible) ? 30214 + 2000 * gc.defense : 0;
			gc.guardian[4].guardian_hp = (gc.guardian[4].visible) ? 30214 + 2000 * gc.defense : 0;
			gc.guardian[5].guardian_hp = (gc.guardian[5].visible) ? 28634 + 2000 * gc.defense : 0;
			gc.guardian[6].guardian_hp = (gc.guardian[6].visible) ? 28634 + 2000 * gc.defense : 0;
			gc.guardian[7].guardian_hp = (gc.guardian[7].visible) ? 28634 + 2000 * gc.defense : 0;
		}
		else
		{
			return false;
		}
		return true;
	}
	bool readGuildsCastles()
	{
		char line[16384];
		FILE *fp;
		int i, j, c;

		///////////////////////////////////////////////////////////////////////
		fp = safefopen(guild_filename,"r");
		if(fp == NULL)
		{
			ShowError("can't read %s\n", guild_filename);
			return false;
		}

		c=0;
		while(fgets(line, sizeof(line), fp))
		{
			c++;
			if( !skip_empty_line(line) )
				continue;

			CGuild g;
			j = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && next_guild_id <= (uint32)i) {
				next_guild_id = i;
				continue;
			}
			if( string2guild(line, g) && g.guild_id > 0 )
			{
				g.calcInfo();
				if(g.max_member>0 && g.max_member < MAX_GUILD)
				{
					for(i=0;i<g.max_member;i++)
					{
						if(g.member[i].account_id>0)
							break;
					}
					if(i<g.max_member)
					{


						if( g.guild_id >= next_guild_id )
							next_guild_id = g.guild_id + 1;

						cGuilds.insert(g);
					}
				}
			}
			else
			{
				ShowError("Guild: broken data [%s] line %d\n", guild_filename, c);
			}
		}
		fclose(fp);
		ShowStatus("Guild: %s read done (%d guilds)\n", guild_filename, cGuilds.size());
		///////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////
		fp = safefopen(castle_filename, "r");
		if( fp==NULL )
		{
			ShowError("GuildCastle: cannot open %s\n", castle_filename);
			return false;
		}
		c = 0;
		while(fgets(line, sizeof(line), fp))
		{
			c++;

			if( !skip_empty_line(line) )
				continue;

			size_t pos;
			CCastle gc;
			if( string2castle(line, gc) )
			{	// clear guildcastles with removed guilds
				if( gc.guild_id && !cGuilds.find(CGuild(gc.guild_id), pos, 0) )
					gc.guild_id = 0;

				cCastles.insert(gc);
			}
			else
				ShowError("GuildCastle: broken data [%s] line %d\n", castle_filename, c);

		}
		fclose(fp);
		ShowStatus("GuildCastle: %s read done (%d castles)\n", castle_filename, cCastles.size());

		for(i = 0; i < MAX_GUILDCASTLE; i++)
		{	// check if castle exists
			size_t pos;
			if( !cCastles.find( CCastle(i), 0, pos) )
			{	// construct a new one if not
				cCastles.insert( CCastle(i) ); // constructor takes care of all settings
			}
		}
		///////////////////////////////////////////////////////////////////////
		return true;
	}
	bool saveGuildsCastles()
	{
		bool ret=true;
		char line[65536];
		FILE *fp;
		int lock;
		size_t i, sz;
		///////////////////////////////////////////////////////////////////////
		fp = lock_fopen(guild_filename,&lock);
		if( fp == NULL) {
			ShowError("Guild: cannot open [%s]\n", guild_filename);
			ret = false;
		}
		else
		{
			for(i=0; i<cGuilds.size(); i++)
			{
				sz=guild2string(line, sizeof(line), cGuilds[i]);
				//fprintf(fp, "%s" RETCODE, line); // retcode integrated to line generation
				if(sz>0)
					fwrite(line, sz,1,fp);
			}
			lock_fclose(fp, guild_filename, &lock);
		}


		///////////////////////////////////////////////////////////////////////
		fp = lock_fopen(castle_filename,&lock);
		if( fp == NULL) {
			ShowError("Guild: cannot open [%s]\n", castle_filename);
			ret = false;
		}
		else
		{
			for(i=0; i<cCastles.size(); i++)
			{
				sz=castle2string(line, sizeof(line), cCastles[i]);
				//fprintf(fp, "%s" RETCODE, line);	// retcode integrated to line generation
				if(sz>0) fwrite(line, sz,1,fp);
			}
			lock_fclose(fp, castle_filename, &lock);
		}
		///////////////////////////////////////////////////////////////////////
		return ret;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB_sql(const char *configfile) :
		CTimerBase(60000),		// 60sec save interval
		next_guild_id(10000),
		savecount(0)
	{
		safestrcpy(guild_filename,"save/guild.txt",sizeof(guild_filename));
		safestrcpy(castle_filename,"save/castle.txt",sizeof(castle_filename));
		safestrcpy(guildexp_filename,"db/exp_guild.txt",sizeof(guildexp_filename));
		init(configfile);
	}
	virtual ~CGuildDB_sql()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CGuild, 2>	cGuilds;
	TslistDST<CCastle>		cCastles;
	uint32					next_guild_id;
	uint					savecount;
	char					guild_filename[1024];
	char					castle_filename[1024];
	char					guildexp_filename[1024];


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
		CConfig::LoadConfig(configfile);
		CGuild::cGuildExp.init(guildexp_filename);
		return readGuildsCastles();
	}
	bool close()
	{
		return saveGuildsCastles();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveGuildsCastles();
		}
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return cGuilds.size(); }
	virtual CGuild& operator[](size_t i)	{ return cGuilds[i]; }

	virtual bool searchGuild(const char* name, CGuild& guild)
	{
		//select guild_data where name = *name
		// return true and guild data
		return false;
	}
	virtual bool searchGuild(uint32 guildid, CGuild& guild)
	{
		// select guild_data where guild_id = *guildid
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




CGuildDBInterface* CGuildDB::getDB(const char *dbcfgfile)
{
	return new CGuildDB_sql(dbcfgfile);
}


///////////////////////////////////////////////////////////////////////////////
// Guild Experience
void CGuildExp::init(const char* filename)
{
	FILE* fp=safefopen(filename,"r");
	memset(exp,0,sizeof(exp));
	if(fp==NULL)
	{
		ShowError("can't read %s\n", filename);
	}
	else
	{
		char line[1024];
		int c=0;
		while(fgets(line,sizeof(line),fp) && c<100)
		{
			if( !skip_empty_line(line) )
				continue;
			exp[c]=atoi(line);
			c++;
		}
		fclose(fp);
	}
}
// static member of CGuild
CGuildExp CGuild::cGuildExp;



///////////////////////////////////////////////////////////////////////////////
// Party Database
///////////////////////////////////////////////////////////////////////////////
class CPartyDB_sql : public CTimerBase, private CConfig, public CPartyDBInterface
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
			if( !skip_empty_line(line) )
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
	CPartyDB_sql(const char *configfile) :
		CTimerBase(60000),		// 60sec save interval
		next_party_id(1000),
		savecount(0)
	{
		safestrcpy(party_filename,"save/party.txt",sizeof(party_filename));
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
		CConfig::LoadConfig(configfile);
		return readParties();
	}
	bool close()
	{
		return saveParties();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveParties();
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return cParties.size(); }
	virtual CParty& operator[](size_t i)	{ return cParties[i]; }

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


CPartyDBInterface* CPartyDB::getDB(const char *dbcfgfile)
{
	return new CPartyDB_sql(dbcfgfile);
}

///////////////////////////////////////////////////////////////////////////////
// Storage Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPCStorageDB_sql : public CTimerBase, private CConfig, public CPCStorageDBInterface
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
			if( !skip_empty_line(line) )
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
	CPCStorageDB_sql(const char *dbcfgfile) :
		CTimerBase(60000),		// 60sec save interval
		savecount(0)
	{
		safestrcpy(pcstorage_filename, "save/storage.txt", sizeof(pcstorage_filename));
		init(dbcfgfile);
	}
	virtual ~CPCStorageDB_sql()
	{
		close();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TslistDST<CPCStorage>	cPCStorList;
	uint savecount;
	char pcstorage_filename[1024];


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
		CConfig::LoadConfig(configfile);
		return readPCStorage();
	}
	bool close()
	{
		return savePCStorage();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			savePCStorage();
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cPCStorList.size(); }
	virtual CPCStorage& operator[](size_t i)	{ return cPCStorList[i]; }

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


CPCStorageDBInterface* CPCStorageDB::getDB(const char *dbcfgfile)
{
	return new CPCStorageDB_sql(dbcfgfile);
}

class CGuildStorageDB_sql : public CTimerBase, private CConfig, public CGuildStorageDBInterface
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
			if( !skip_empty_line(line) )
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
	CGuildStorageDB_sql(const char *dbcfgfile) :
		CTimerBase(60000),		// 60sec save interval
		savecount(0)
	{
		safestrcpy(guildstorage_filename, "save/g_storage.txt", sizeof(guildstorage_filename));
		init(dbcfgfile);
	}
	virtual ~CGuildStorageDB_sql()
	{
		close();
	}


private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TslistDST<CGuildStorage> cGuildStorList;
	uint savecount;
	char guildstorage_filename[1024];


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
		CConfig::LoadConfig(configfile);
		return readGuildStorage();
	}
	bool close()
	{
		return saveGuildStorage();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveGuildStorage();
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cGuildStorList.size(); }
	virtual CGuildStorage& operator[](size_t i)	{ return cGuildStorList[i]; }

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


CGuildStorageDBInterface* CGuildStorageDB::getDB(const char *dbcfgfile)
{
	return new CGuildStorageDB_sql(dbcfgfile);
}

#endif// SQL
