// SELECT element FROM table ORDER BY name LIMIT OFFSET,NUMBER



#ifndef TXT_ONLY

#include "basesq.h"

//////////////////////////////////////////////////////////////////////////////////////
// CMySQL Class
//////////////////////////////////////////////////////////////////////////////////////
// CMySQL Class Constructor
// Code is executed everytime a CMySQL var is initialized
CMySQL::CMySQL()
	: cDBPool(*this),	// initialize the first database object
	  result(NULL)		// if insisting on class global data, then initialize it
{
	safestrcpy(mysqldb_ip, "127.0.0.1", sizeof(mysqldb_ip));
	mysqldb_port=3306;
	safestrcpy(mysqldb_id, "ragnarok", sizeof(mysqldb_id));
	safestrcpy(mysqldb_pw, "ragnarok", sizeof(mysqldb_pw));
	safestrcpy(mysqldb_db, "ragnarok", sizeof(mysqldb_db));
}



// CMySQL Class Destructor
// Code is executed a CMySQL var is destroyed
inline CMySQL::~CMySQL()
{
	// close all existing database objects in the pool
	this->cDBPool.call( &DBConnection::close );
	

	this->FreeResults();
}








bool CMySQL::SendQuery(const string<> q) {

//	ShowError("Query = %s\n",q.c_str());

	if( 0==mysql_real_query(&mysqldb_handle, q.c_str(), q.length()) )
	{
		result = mysql_store_result(&mysqldb_handle);
		if(result)
			return true;
		else if(mysql_field_count(&mysqldb_handle) == 0)
            // query does not return data
            // (it was not a SELECT)
            return true;
		else
			ShowError("DB result error\nQuery:    %s\n", q.c_str() );
	}
	else
	{
		ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&mysqldb_handle), q.c_str());
		abort();
	}

	return false;
}

bool CMySQL::FetchResults()
{
	if (result) // was there even a result?
		if ((row = mysql_fetch_row(result))) // is there a row to pull out?
			return true;
	return false;
}

long int CMySQL::CountResults()
{
	return mysql_num_rows(result);
}

void CMySQL::FreeResults()
{
	if (result)
	{
		mysql_free_result(result);
	}
}



// Send a MySQL query to get data
bool CMySQL::mysql_SendQuery(MYSQL_RES*& sql_res, const char* q, size_t sz)
{

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
inline const char* CMySQL::escape_string(char *target, const char* source, size_t len)
{

	mysql_real_escape_string(&mysqldb_handle, target, source, len);

	return target;
}

//////////////////////////////////////////////////////////////////////////////////////
// CAccountDB_sql Class
//////////////////////////////////////////////////////////////////////////////////////
CAccountDB_sql::CAccountDB_sql(const char* configfile)
	: cSqlRes(NULL)
{
	login_auth_db	= "login_auth";
	login_reg_db	= "login_reg";
	login_status_db	= "login_status";
	login_log_db	= "login_log";

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
			login_auth_db = w2;
		}
		else if (strcasecmp(w1, "login_reg_db") == 0) {
			login_reg_db = w2;
		}
		else if (strcasecmp(w1, "login_log_db") == 0) {
			login_log_db = w2;
		}
		else if (strcasecmp(w1, "login_status_db") == 0) {
			login_status_db = w2;
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
		string<> query;

		escape_string(uid, userid, strlen(userid));
		query << "SELECT `user_id` FROM `" << login_auth_db << "` WHERE " << (case_sensitive?"BINARY":"") << "`user_id` = '" <<  uid  <<"'";
		this->SendQuery(query);
		if( this->FetchResults())
		{

			ret = this->CountResults()>0;
		}
	}
	return ret;
}

bool CAccountDB_sql::searchAccount(const char* userid, CLoginAccount& account)
{	// get account by user/pass
	bool ret = false;

	if(userid)
	{

		string<> query;
		char uid[64];
		uint32 accid = 0;

		//-----------
		// Find the account_id and pass that on to the proper function that gets them via account_id
		escape_string(uid, userid, strlen(userid));
		query
			<< "SELECT `account_id`"		//  0
			<<" FROM `" << login_auth_db << "` WHERE `user_id`='" << uid << "'";

		if( this->SendQuery(query) )
		{
			//row fetching
			if(this->FetchResults())
			{
				accid = this->row[0]?atol(this->row[0]):0;
			}

			this->FreeResults();
		}

		ret = this->searchAccount(accid,account);

	}
	return ret;
}

bool CAccountDB_sql::searchAccount(uint32 accid, CLoginAccount& account)
{	// get account by account_id

	bool ret = false;

	if(accid)
	{

		string<> query;
//		char uid[64];

		query
			<< "SELECT"
			<<"`account_id`,"		//  0
			<<"`user_id`,"			//  1
			<<"`user_pass`,"		//  2
			<<"`sex`,"				//  3
			<<"`gm_level`,"			//  4
			<<"`online`,"			//  5
			<<"`email`,"			//  6
			<<"`login_id1`,"		//  7
			<<"`login_id2`,"		//  8
			<<"`client_ip`,"		//  9
			<<"`last_login`,"		// 10
			<<"`login_count`,"		// 11
			<<"`ban_until`,"		// 12
			<<"`valid_until`"		// 13
			<<" FROM `" << login_auth_db << "` WHERE `account_id`='" << accid << "'";

		if( this->SendQuery(query) )
		{
			//row fetching
			if(this->FetchResults())
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

				this->FreeResults();

				ret = true;
			}
		}
		query.clear();

		if(ret)
		{
			query << "SELECT `str`,`value` FROM `" << login_reg_db << "` WHERE `account_id`='" << account.account_id << "'";
			if( this->SendQuery(query) )
			{
				size_t i=0;
				while( i<ACCOUNT_REG2_NUM && this->FetchResults() )
				{
					safestrcpy(account.account_reg2[i].str, this->row[0], sizeof(account.account_reg2[0].str));
					account.account_reg2[i].value = (this->row[1]) ? atoi(this->row[1]):0;
					i++;
				}
				account.account_reg2_num = i;

				this->FreeResults();
			}
			query.clear();
		}
	}

	return ret;
}

bool CAccountDB_sql::insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount& account)
{	// insert a new account to db
//	size_t sz;
	char uid[64], pwd[64];
	string<> query;

	escape_string(uid, userid, strlen(userid));
	escape_string(pwd, passwd, strlen(passwd));

	query << "INSERT INTO `" << login_auth_db << "` (`user_id`, `user_pass`, `sex`, `email`) VALUES ('" << uid << "', '" << pwd << "', '" << sex << "', '" << email << "')";
	if( this->SendQuery(query) )
	{
		// read the complete account back from db
		return searchAccount(userid, account);
	}
	return false;
}

bool CAccountDB_sql::removeAccount(uint32 accid)
{
	bool ret;
	string<> query;

	query << "DELETE FROM `" << login_auth_db << "` WHERE `account_id`='" << accid << "'";
	ret = this->SendQuery(query);
	query.clear();

	query << "DELETE FROM `" << login_reg_db << "` WHERE `account_id`='" << accid << "'"; // must update with the variable login_reg_db
	ret &=this->SendQuery(query);
	query.clear();

	return ret;
}

bool CAccountDB_sql::init(const char* configfile)
{	// init db
	bool wipe=false; // i dont know how a bool is set..
	string<> query; // used for the queries themselves

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
			query << "INSERT DELAYED INTO `" << login_log_db << "`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'Login', '100','login server started')";
			//query
			this->SendQuery(query);
			query.clear();
		}
	}
	else
	{	// pointer check
		ShowMessage("%s\n", mysql_error(&(this->mysqldb_handle)));
		return false;
	}

	if (wipe)
	{
		query << "DROP TABLE IF EXISTS `" << login_auth_db << "`";
		this->SendQuery(query);
		query.clear();
	}


	query
		<< "CREATE TABLE IF NOT EXISTS `" << login_auth_db << "` ("
		<< "`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		<< "`user_id` VARCHAR(24) NOT NULL,"
		<< "`user_pass` VARCHAR(34) NOT NULL,"
		<< "`sex` ENUM('M','F','S') default 'M',"
		<< "`gm_level` INT(3) UNSIGNED NOT NULL,"
		<< "`online` BOOL default 'false',"
		<< "`email` VARCHAR(40) NOT NULL,"
		<< "`login_id1` INTEGER UNSIGNED NOT NULL,"
		<< "`login_id2` INTEGER UNSIGNED NOT NULL,"
		<< "`client_ip` VARCHAR(16) NOT NULL,"
		<< "`last_login` INTEGER UNSIGNED NOT NULL,"
		<< "`login_count` INTEGER UNSIGNED NOT NULL,"
		<< "`ban_until` INTEGER UNSIGNED NOT NULL,"
		<< "`valid_until` INTEGER UNSIGNED NOT NULL,"
		<< "PRIMARY KEY(`account_id`)"
		<< ")";

	this->SendQuery(query);
	query.clear();

	if (wipe)
	{
		query << "DROP TABLE IF EXISTS `" << login_reg_db << "`";
		this->SendQuery(query);
		query.clear();
	}

	query
		<< "CREATE TABLE IF NOT EXISTS `" << login_reg_db << "` ("
		<< "`account_id` INTEGER UNSIGNED AUTO_INCREMENT,"
		<< "`str` VARCHAR(34) NOT NULL,"  // Not sure on the length needed. (struct global_reg::str[32]) but better read the size from the struct later
		<< "`value` INTEGER UNSIGNED NOT NULL,"
		<< "PRIMARY KEY(`account_id`,`str`)"
		<< ")";

	this->SendQuery(query);
	query.clear();


	if (wipe)
	{
		query << "DROP TABLE IF EXISTS `" << login_log_db << "`";
		this->SendQuery(query);
		query.clear();
	}

	query
		<< "CREATE TABLE IF NOT EXISTS `" << login_log_db << "` ("
		<< "`time` INTEGER UNSIGNED,"
		<< "`ip` VARCHAR(16) NOT NULL,"
		<< "`user` VARCHAR(24) NOT NULL,"
		<< "`rcode` INTEGER(3) UNSIGNED NOT NULL,"
		<< "`log` VARCHAR(100) NOT NULL"
		<< ")";

	this->SendQuery(query);
	query.clear();

	if (wipe)
	{
		query << "DROP TABLE IF EXISTS `" << login_status_db << "`";
		this->SendQuery(query);
		query.clear();
	}

	query
		<< "CREATE TABLE IF NOT EXISTS `" << login_status_db << "` ("
		<< "`index` INTEGER UNSIGNED NOT NULL,"
		<< "`name` VARCHAR(24) NOT NULL,"
		<< "`user` INTEGER UNSIGNED NOT NULL,"
		<< "PRIMARY KEY(`index`)"
		<< ")";

	this->SendQuery(query);
	query.clear();

	return true;
}

bool CAccountDB_sql::close()
{
	string<> query;
	//set log.
	if (log_login)
	{
		query << "INSERT DELAYED INTO `" << login_log_db << "`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')";
		this->SendQuery(query);
		query.clear();
	}

	//delete all server status
	query << "DELETE FROM `" << login_status_db << "`";
	this->SendQuery(query);
	query.clear();

	mysql_close(&(this->mysqldb_handle));

	return true;
}

bool CAccountDB_sql::saveAccount(const CLoginAccount& account)
{
	bool ret = false;
//	size_t sz;
	size_t i;
	string<> query;
	char tempstr[64];

	//-----------
	// Update the login_auth_db with new info
	query
		<<"UPDATE `" << login_auth_db << "` SET "

		<< "`user_id` = '" << 		account.userid 								<< "',"
		<< "`user_pass` = '" <<		account.passwd								<< "',"
		<< "`sex` = '" <<			((account.sex==1)?"M":"F")					<< "',"
		<< "`gm_level` = '" <<		account.gm_level							<< "',"
		<< "`online` = '" << 		account.online								<< "',"
		<< "`email` = '" << 		account.email								<< "',"
		<< "`login_id1` = '" << 	account.login_id1							<< "',"
		<< "`login_id2` = '" << 	account.login_id2							<< "',"
		<< "`client_ip` = '" <<		((ipaddress)account.client_ip).tostring()	<< "',"
		<< "`last_login` = '" <<	account.last_login							<< "',"
		<< "`login_count` = '" <<	(ulong)account.login_count			<< "',"
		<< "`valid_until` = '" <<	(ulong)account.valid_until			<< "',"
		<< "`ban_until` = '" <<		(ulong)account.ban_until			<< "'"

		<< "WHERE `account_id` = '" << account.account_id						<< "'";


	ret = this->SendQuery(query);
	query.clear();

	//----------
	// Delete and reinsert data for the registry values

	query << "DELETE FROM `" << login_reg_db << "` WHERE `account_id`='" << account.account_id << "'";
	this->SendQuery(query);
	query.clear();

	//----------
	// Insert here
	query << "INSERT INTO `" << login_reg_db << "` (`account_id`, `str`, `value`) VALUES ";

	for(i=0; i<account.account_reg2_num; i++)
	{
		this->escape_string(tempstr, account.account_reg2[i].str, strlen(account.account_reg2[i].str));

		query << (i?",":"") << "( '" << account.account_id << "','" << tempstr << "','" << account.account_reg2[i].value << "')";
	}
	ret &= this->SendQuery(query);

	return ret;
}


// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /
/// / CHAR SERVER IMPLEMENTATION BY CLOWNISIUS  / / / / / / / / / / / / / / / / / /
// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /


/* data for sql tables:

DROP TABLE IF EXISTS `account_storage`;

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
);

DROP TABLE IF EXISTS `account_reg`;

CREATE TABLE IF NOT EXISTS `account_reg` (
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `str`				VARCHAR(255) NOT NULL default '',
  `value`			VARCHAR(255) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`str`),
  KEY `account_id` (`account_id`)
);

DROP TABLE IF EXISTS `char_characters`;

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
  `zeny` 			BIGINT UNSIGNED NOT NULL default '500',
  `str` 			TINYINT UNSIGNED NOT NULL default '0',
  `agi` 			TINYINT UNSIGNED NOT NULL default '0',
  `vit` 			TINYINT UNSIGNED NOT NULL default '0',
  `int`				TINYINT UNSIGNED NOT NULL default '0',
  `dex` 			TINYINT UNSIGNED NOT NULL default '0',
  `luk`				TINYINT UNSIGNED NOT NULL default '0',
  `max_hp` 			MEDIUMINT UNSIGNED NOT NULL default '0',
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
);

DROP TABLE IF EXISTS `char_character_reg`;

CREATE TABLE IF NOT EXISTS `char_character_reg` (
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `str`				VARCHAR(255) NOT NULL default '',
  `value`			VARCHAR(255) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`str`),
  KEY `char_id` (`char_id`)
);

DROP TABLE IF EXISTS `char_friends`;

CREATE TABLE IF NOT EXISTS `char_friends` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `friend_id`		INTEGER UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`friend_id`),
  KEY `char_id` (`char_id`)
);


DROP TABLE IF EXISTS `char_inventory`;

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
);

DROP TABLE IF EXISTS `char_cart`;

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
);

DROP TABLE IF EXISTS `char_memo`;

CREATE TABLE IF NOT EXISTS `char_memo` (
  `memo_id`			TINYINT UNSIGNED NOT NULL default '0',
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `map` 			VARCHAR(20) NOT NULL default '',
  `x` 				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  `y`				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`memo_id`),
  KEY `char_id` (`char_id`)
);


DROP TABLE IF EXISTS `char_skill`;

CREATE TABLE IF NOT EXISTS `char_skill` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `id`				MEDIUMINT UNSIGNED NOT NULL default '0',
  `lv` 				TINYINT UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  KEY `char_id` (`char_id`)
);

*/

	/**************** NOTES FOR CHAR_FROM_SQL ********************

	Ok, this is a function I'm not too sure how it will end

	But the plan is to unload data from the in house database if
	the online = false, meaning the person has logged off or is
	just looking at a character listing.

	A second query will be done for just searching for the needed
	data for just viewing a list just like on clownphobia branch.

	The function goes like this:

	load local data from in house database

	run each part of the save process, if different, save to
	database, at the end of the char_from sql, if (online = false)
	then dont keep or save a copy in the inhouse database. if
	there is a copy in the inhouse database, remove it, must mean
	that the player logged off, or was kicked off and this was the
	last saving function for him.

	*************************************************************/

CCharDB_sql::CCharDB_sql(const char *dbcfgfile)
{
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
CCharDB_sql::~CCharDB_sql()
{
	// Add cleaning code here
}

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

bool CCharDB_sql::compare_item(const struct item &a, const struct item &b)
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

bool CCharDB_sql::existChar(uint32 char_id)
{
	string<> query;
	bool ret = false;

	query
		<< "SELECT count(*) FROM `" << char_db << "` WHERE char_id = '" << char_id << "'";

	if ( this->SendQuery(query) )
	{
		if ( this->FetchResults() )
		{
			if ( atol(this->row[0]) ) ret = true;
		}
		this->FreeResults();
	}

	return ret;
}

bool CCharDB_sql::existChar(const char* name)
{
	string<> query;
	char _name[32];
	bool ret = false;

	escape_string(_name, name, strlen(name));

	query
		<< "SELECT count(*) FROM `" << char_db << "` WHERE name = '" << _name << "'";

	if ( this->SendQuery(query) )
	{
		if ( this->FetchResults() )
		{
			if ( atol(this->row[0]) ) ret = true;
		}
		this->FreeResults();
	}

	return ret;
}

bool CCharDB_sql::searchChar(const char* name, CCharCharacter &p)
{
	bool ret = false;
	string<> query;
	uint32 charid = 0;


	query << "SELECT `char_id` FROM `" << char_db << " WHERE `name`='" << name << "'";
	if (this->SendQuery(query))
	{
		this->FetchResults();
		charid = atoi(this->row[0]);
		ret = this->searchChar(charid,p);
		this->FreeResults();
	}

	return ret;
}

bool CCharDB_sql::searchChar(uint32 char_id, CCharCharacter &p)
{
		bool ret = false;
		int tmp_int[256];
		size_t i,n;
		string<> query;


		//CCharCharacter p;
		//CCharCharacter cp;


		// initilialise character
		memset(&p, 0, sizeof(CCharCharacter));
		//memset(&cp, 0, sizeof(CCharCharacter));
		memset(tmp_int, 0, sizeof(tmp_int));

		#ifdef CHAR_DEBUG_INFO
			printf("Loaded: ");
		#endif

		p.char_id = char_id;

	/**************** Load all base stats ********************/
		query.clear();
		query
			<<"SELECT "

				<< "`char_id`,"		// 0
				<< "`account_id`,"	// 1
				<< "`char_num`,"	// 2
				<< "`name`,"		// 3
				<< "`class`,"		// 4
				<< "`base_level`,"	// 5
				<< "`job_level`,"	// 6
				<< "`base_exp`,"	// 7
				<< "`job_exp`,"		// 8
				<< "`zeny`,"		// 9
				<< "`str`,`agi`,`vit`,`int`,`dex`,`luk`," // 10 - 15
				<< "`max_hp`,`hp`,"	// 16 - 17
				<< "`max_sp`,`sp`,"	// 18 - 19
				<< "`status_point`,"// 20
				<< "`skill_point` "	// 21

				<< "`option`,"		//22
				<< "`karma`,"		//23
				<< "`manner`,"		//24
				<< "`party_id`,"	//25
				<< "`guild_id`,"	//26
				<< "`pet_id`,"		//27
				<< "`hair`,"		//28
				<< "`hair_color`,"	//29
				<< "`clothes_color`,"//30
				<< "`weapon`,"		//31
				<< "`shield`,"		//32
				<< "`head_top`,"	//33
				<< "`head_mid`,"	//34
				<< "`head_bottom`,"	//35
				<< "`last_map`,"	//36
				<< "`last_x`,"		//37
				<< "`last_y`,"		//38
				<< "`save_map`,"	//39
				<< "`save_x`,"		//40
				<< "`save_y`,"		//41
				<< "`partner_id`,"	//42
				<< "`father`,"		//43
				<< "`mother`,"		//44
				<< "`child`,"		//45
				<< "`fame`"			//46


			<< "FROM `" << char_db << "` "
			<< "WHERE `char_id` = '" << char_id << "'";

		if( this->SendQuery(query) )
		{
			this->FetchResults();

			p.char_id 			= char_id;
			p.account_id 		= atoi(this->row[1]);
			p.slot 				= atoi(this->row[2]);
			strcpy(p.name, 		       this->row[3]);
			p.class_ 			= atoi(this->row[4]);
			p.base_level 		= atoi(this->row[5]);
			p.job_level 		= atoi(this->row[6]);
			p.base_exp 			= atoi(this->row[7]);
			p.job_exp 			= atoi(this->row[8]);
			p.zeny 				= atoi(this->row[9]);
			p.str 				= atoi(this->row[10]);
			p.agi 				= atoi(this->row[11]);
			p.vit 				= atoi(this->row[12]);
			p.int_				= atoi(this->row[13]);
			p.dex 				= atoi(this->row[14]);
			p.luk 				= atoi(this->row[15]);
			p.max_hp 			= atoi(this->row[16]);
			p.hp 				= atoi(this->row[17]);
			p.max_sp 			= atoi(this->row[18]);
			p.sp 				= atoi(this->row[19]);
			p.status_point 		= atoi(this->row[20]);
			p.skill_point 		= atoi(this->row[21]);

			p.option			= atoi(this->row[22]);
			p.karma				= atoi(this->row[23]);
			p.manner			= atoi(this->row[24]);
			p.party_id			= atoi(this->row[25]);
			p.guild_id			= atoi(this->row[26]);
			p.pet_id			= atoi(this->row[27]);
			p.hair				= atoi(this->row[28]);
			p.hair_color		= atoi(this->row[29]);
			p.clothes_color		= atoi(this->row[30]);
			p.weapon			= atoi(this->row[31]);
			p.shield			= atoi(this->row[32]);
			p.head_top			= atoi(this->row[33]);
			p.head_mid			= atoi(this->row[34]);
			p.head_bottom		= atoi(this->row[35]);
			strcpy(p.last_point.mapname,this->row[36]);
			p.last_point.x		= atoi(this->row[37]);
			p.last_point.y		= atoi(this->row[38]);
			strcpy(p.save_point.mapname,this->row[39]);
			p.save_point.x		= atoi(this->row[40]);
			p.save_point.y		= atoi(this->row[41]);
			p.partner_id		= atoi(this->row[42]);
			p.father_id			= atoi(this->row[43]);
			p.mother_id			= atoi(this->row[44]);
			p.child_id			= atoi(this->row[45]);
			p.fame_points		= atoi(this->row[46]);

			//free mysql result.
			this->FreeResults();
		} else return ret;

		#ifdef CHAR_DEBUG_INFO
			printf("(\033[1;32m%d\033[0m)\033[1;32m%s\033[0m\t[",p.char_id,p.name);
			printf("char1 ");
		#endif


		/**************** Check start/save locations ********************/
		if (p.last_point.x == 0 || p.last_point.y == 0 || p.last_point.mapname[0] == '\0')
		{
			char errbuf[64];
			snprintf(errbuf,sizeof(errbuf),"%s has no last point?\n",p.name);
			memcpy(&p.last_point, &start_point, sizeof(start_point));
		}

		if (p.save_point.x == 0 || p.save_point.y == 0 || p.save_point.mapname[0] == '\0')
		{
			char errbuf[64];
			snprintf(errbuf,sizeof(errbuf),"%s has no save point?\n",p.name);
			memcpy(&p.save_point, &start_point, sizeof(start_point));
		}
		#ifdef CHAR_DEBUG_INFO
			printf("char2 ");
		#endif

		/**************** Load Memo ********************/
		query.clear();
		query << "SELECT `map`,`x`,`y` FROM `" << memo_db << "` WHERE `char_id`='" << char_id << "'";

		if (this->SendQuery(query))
		{
			for(i=0; this->FetchResults() && i < 3;i++)
			{
				strcpy (p.memo_point[i].mapname,this->row[0]);
				p.memo_point[i].x=atoi(this->row[1]);
				p.memo_point[i].y=atoi(this->row[2]);
			}
			this->FreeResults();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("memo ");
		#endif

		/**************** Load Inventory ********************/
		query.clear();
		query
			<< "SELECT "
				<< "`id`,"			// 0
				<< "`nameid`,"		// 1
				<< "`amount`,"		// 2
				<< "`equip`,"		// 3
				<< "`identify`,"	// 4
				<< "`refine`,"		// 5
				<< "`attribute`,"	// 6
				<< "`card0`,"		// 7
				<< "`card1`,"		// 8
				<< "`card2`,"		// 9
				<< "`card3`"		//10

			<< "FROM `"<< inventory_db << "` WHERE `char_id`='" << char_id << "'";

		if (this->SendQuery(query))
		{
			for(i=0;this->FetchResults();i++)
			{
				p.inventory[i].id			= atoi(this->row[0]);
				p.inventory[i].nameid		= atoi(this->row[1]);
				p.inventory[i].amount		= atoi(this->row[2]);
				p.inventory[i].equip		= atoi(this->row[3]);
				p.inventory[i].identify		= atoi(this->row[4]);
				p.inventory[i].refine		= atoi(this->row[5]);
				p.inventory[i].attribute	= atoi(this->row[6]);
				p.inventory[i].card[0]		= atoi(this->row[7]);
				p.inventory[i].card[1]		= atoi(this->row[8]);
				p.inventory[i].card[2]		= atoi(this->row[9]);
				p.inventory[i].card[3]		= atoi(this->row[10]);
			}
			this->FreeResults();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("inventory ");
		#endif

		/**************** Load Cart ********************/
		query.clear();
		query
			<< "SELECT "
				<< "`id`,"			// 0
				<< "`nameid`,"		// 1
				<< "`amount`,"		// 2
				<< "`equip`,"		// 3
				<< "`identify`,"	// 4
				<< "`refine`,"		// 5
				<< "`attribute`,"	// 6
				<< "`card0`,"		// 7
				<< "`card1`,"		// 8
				<< "`card2`,"		// 9
				<< "`card3`"		//10

			<< "FROM `"<< cart_db << "` WHERE `char_id`='" << char_id << "'";

		if (this->SendQuery(query))
		{
			for(i=0;this->FetchResults();i++)
			{
				p.cart[i].id				= atoi(this->row[0]);
				p.cart[i].nameid			= atoi(this->row[1]);
				p.cart[i].amount			= atoi(this->row[2]);
				p.cart[i].equip				= atoi(this->row[3]);
				p.cart[i].identify			= atoi(this->row[4]);
				p.cart[i].refine			= atoi(this->row[5]);
				p.cart[i].attribute			= atoi(this->row[6]);
				p.cart[i].card[0]			= atoi(this->row[7]);
				p.cart[i].card[1]			= atoi(this->row[8]);
				p.cart[i].card[2]			= atoi(this->row[9]);
				p.cart[i].card[3]			= atoi(this->row[10]);
			}
			this->FreeResults();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("cart ");
		#endif

		/**************** Load skill ********************/
		query.clear();
		query
			<< "SELECT"
				<< "`id`,"
				<< "`lv`"

			<< "FROM `" << skill_db << "` WHERE `char_id`='" << char_id << "'";

		if (this->SendQuery(query))
		{
			for(i=0;this->FetchResults();i++)
			{
				n = atoi(this->row[0]);
				p.skill[n].id = n; //memory!? shit!.
				p.skill[n].lv = atoi(this->row[1]);
			}
			this->FreeResults();
		}
		#ifdef CHAR_DEBUG_INFO
			printf("skill ");
		#endif

		/**************** Load registry ********************/
		query.clear();
		query
			<< "SELECT"
				<< "`str`,"
				<< "`value`"

			<< "FROM ` " << char_reg_db << "` WHERE `char_id`='" << char_id << "'";

		i=0;
		if (this->SendQuery(query))
		{
			for(i=0;this->FetchResults();i++)
			{
				strcpy (p.global_reg[i].str, this->row[0]);
				p.global_reg[i].value = atoi (this->row[1]);
			}
			this->FreeResults();
		}
		p.global_reg_num=i;



		/**************** Load Friends ********************/

		for(i=0;i<20;i++) // init friends struct
		{
			p.friendlist[i].friend_id = 0;
			p.friendlist[i].friend_name[0] = '\0';
		}

		query.clear();
		query
			<< "SELECT "
				<< "`f`.`friend_id`,"
				<< "`c`.`name`"

			<< "FROM `" << friend_db << "` `f` "

				<< "JOIN `" << char_db << "` `c` ON `f`.`friend_id`=`c`.`char_id`"

			<< "WHERE `f`.`char_id` = '" << char_id << "'";



		if (this->SendQuery(query))
		{
			for(i=0;this->FetchResults();i++)
			{
				p.friendlist[i].friend_id = atoi(this->row[0]);
				snprintf(p.friendlist[i].friend_name, sizeof(p.friendlist[i].friend_name), "%s", this->row[1]);
			}
			this->FreeResults();
		}

		#ifdef CHAR_DEBUG_INFO
			printf("friends ");
		#endif

		/**************** Finish setup ********************/
//		if (online)
//		{
			//set_char_online(char_id,p.account_id); // not setup yet... just leave as is
//		}
		return true;
}

bool CCharDB_sql::insertChar(CCharAccount &account,
					const char *n,
					unsigned char str,
					unsigned char agi,
					unsigned char vit,
					unsigned char int_,
					unsigned char dex,
					unsigned char luk,
					unsigned char slot,
					unsigned char hair_style,
					unsigned char hair_color,
					CCharCharacter &p)
{

	CCharCharacter tempchar(n);

	string<> query;

	char t_name[128];

	size_t i;

	this->escape_string(t_name, tempchar.name, strlen(tempchar.name));

	//check stat error
	if (
		(str + agi + vit + int_ + dex + luk !=6*5 ) || // stats

		// Check slots
		(slot >= 9) || // slots must not be over 9

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
		ShowError("fail (aid: %d), stats error(bot cheat?!)\n", account.account_id);
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
	query << "SELECT count(*) FROM `" << char_db << "` WHERE `name` = '" << t_name << "'";
	if ( this->SendQuery(query) )
	{
		this->FetchResults();
		if (atol(this->row[0]))
		{
			printf("fail, charname '%s' already in use\n", t_name);
			return false;
		}
		this->FreeResults();
	}
	query.clear();


	// check char slot.
	query << "SELECT count(*) FROM `" << char_db << "` WHERE `account_id` = '" << account.account_id << "' AND `char_num` = '" << slot << "'";
	if ( this->SendQuery(query) )
	{
		this->FetchResults();

		if(atol(this->row[0]))
		{
			printf("fail (aid: %d, slot: %d), slot already in use\n", account.account_id, slot);
			return false;
		}
		this->FreeResults();
	}
	query.clear();

	// It has passed both the name and slot check, let's insert the info since it doesnt conflict =D
	// make new char.
	query
		<< "INSERT INTO `" << char_db << "` "
			<< "(`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`) "
			<< "VALUES "
			<< "('" << account.account_id << "',"
			<< "'" << slot << "',"
			<< "'" << t_name << "',"
			<< "'" << str << "','" << agi << "','" << vit << "','" << int_ << "','" << dex << "','" << luk << "',"
			<< "'" << hair_style << "',"
			<< "'" << hair_color << "')";

	this->SendQuery(query);
	query.clear();


	//Now we need the charid from sql!
	query
		<< "SELECT "
			<< "`char_id` "
		<< "FROM `" << char_db << "`"

		<< "WHERE `account_id` = '" << account.account_id << "' AND `char_num` = '" << slot << "' AND `name` = '" << t_name << "'";

	if( this->SendQuery(query) )
	{
		this->FetchResults();
		if (this->row)
			tempchar.char_id = atol(this->row[0]); //char id :)
		this->FreeResults();
	} else
		return false;
	query.clear();

	//Give the char the default items
	//knife & cotton shirts, add on as needed ifmore items are to be included.
	query
		<<"INSERT INTO `" << inventory_db << "` "
			<< "(`char_id`,`nameid`, `amount`, `equip`, `identify`) "
		<< "VALUES "
			<< "('" << tempchar.char_id << "', '" << start_weapon << "', '1', '" << 0x02 << "', '1'),"
			<< "('" << tempchar.char_id << "', '" << start_armor  << "', '1', '" << 0x10 << "', '1')";

	this->mysql_SendQuery(query, query.length());


	// Update the map they are starting on and where they respawn at.
	query
		<< "UPDATE `" << char_db << "` "
		<< "SET "

			<< "`last_map` = `save_map` = '" << start_point.mapname << "',"
			<< "`last_x`   = save_x     = '" << start_point.x << "',"
			<< "`last_y`   = save_y     = '" << start_point.y << "'"

		<< "WHERE `char_id` = '" << tempchar.char_id << "'";
	// All good, init the character data to return i think
	query.clear();


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


bool CCharDB_sql::removeChar(uint32 charid)
{
	string<> query;
	query << "DELETE FROM `" << char_db << "` WHERE `char_id`='" << charid << "'";
	this->SendQuery(query);
	return true;
}

bool CCharDB_sql::saveChar(const CCharCharacter& p)
{
	string<> query;

	bool diff, l , status, loaded=false;

	size_t i;


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
			"`account_id`='" 	<< p.account_id		<< "' " <<

		"AND " <<
			"`char_id` = '"		<< p.char_id		<<"' " <<

		"AND " <<
			"`slot` = '" 		<< p.slot			<< "'";  // dont forget to finish the line


	this->SendQuery(query);
	query.clear();


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
	{
		query << "DELETE FROM `" << memo_db << "` WHERE `char_id`='" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();


		//insert here.
		query << "INSERT INTO `" << memo_db << "`(`char_id`,`memo_id`,`map`,`x`,`y`) VALUES ";
		l=false;
		for(i=0;i<MAX_MEMO;i++)
		{
			if(p.memo_point[i].mapname[0])
			{
				query << (l?",":"") << "(" <<

				"'" << 	p.char_id 					<< "'," <<
				"'" <<	(ulong)i							<< "'," <<
				"'" <<	p.memo_point[i].mapname		<< "'," <<
				"'" <<	p.memo_point[i].x			<< "'," <<
				"'" <<	p.memo_point[i].y			<< "," <<  // Dont forget to end commas
				")";
				l= true;
			}
		}
		// if at least one entry spotted.
		if(l)this->SendQuery(query);
		query.clear();
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
		query <<
			"DELETE FROM `" << cart_db << "` WHERE `char_id`='" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();

		//insert here.
		query <<
			"INSERT INTO `" << inventory_db << "`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";
		l=false;
		for(i=0;i<MAX_INVENTORY;i++)
		{
			if(p.inventory[i].nameid>0)
			{
				query << (l?",":"") << "(" <<
				"'" <<	p.char_id	<< "'," <<
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
		if(l)this->SendQuery(query);
		query.clear();
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
		query << "DELETE FROM `" << cart_db << "` WHERE `char_id`='" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();

		//insert here.
		query << "INSERT INTO `" << cart_db << "`(`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";
		l=false;
		for(i=0;i<MAX_CART;i++)
		{
			if(p.cart[i].nameid>0)
			{
				query << (l?",":"") << "(" <<
				"'" <<	p.char_id	<< "'," <<
				"'" <<	p.cart[i].nameid		<< "'," <<
				"'" <<	p.cart[i].amount		<< "'," <<
				"'" <<	p.cart[i].equip			<< "'," <<
				"'" <<	p.cart[i].identify		<< "'," <<
				"'" <<	p.cart[i].refine		<< "'," <<
				"'" <<	p.cart[i].attribute		<< "'," <<
				"'" <<	p.cart[i].card[0]		<< "'," <<
				"'" <<	p.cart[i].card[1]		<< "'," <<
				"'" <<	p.cart[i].card[2]		<< "'," <<
				"'" <<	p.cart[i].card[3]		<< "'" <<
				")";

				l = true;
			}
		}
		// if at least one entry spotted.
		if(l)this->SendQuery(query);
		query.clear();
	}

	///////////////////////////////////////////////////////////////////////
	// Skill Insert

	diff = false;
	l = false;

	if (diff)
	{
		query << "DELETE FROM `" << skill_db << "` WHERE `char_id`='" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();

		//insert here.
		query << "INSERT INTO `" << skill_db << "`(`char_id`,`id`,`lvl`) VALUES ";
		l=false;
		for(i=0;i<MAX_MEMO;i++)
		{
			if(p.skill[i].id>0 && p.skill[i].flag != 1)
			{
				query << (l?",":"") << "(" <<

				"'" << p.char_id 				<< "'," <<
				"'" << p.skill[i].id 			<< "'," <<
				"'" << p.skill[i].lv 			<< "'" <<
				")";
				l = true;
			}
		}
		// if at least one entry spotted.
		if(l)this->SendQuery(query);
		query.clear();
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
		query << "DELETE FROM `" << char_reg_db << "` WHERE `char_id`='" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();

		//insert here.
		query << "INSERT INTO `" << char_reg_db << "`(`char_id`,`str`,`value`) VALUES ";
		for(i=0;i<p.global_reg_num;i++)
		{
			if(p.global_reg[i].str && p.global_reg[i].value !=0)
			{
				query <<
				(l?",":"") << "(" <<

				"'" << p.char_id << "'," <<
				"'" << p.global_reg[i].str		<< "'," <<
				"'" << p.global_reg[i].value	<< "'" <<   // end commas at the end
				")";

				l = true;
			}
		}
		// if at least one entry spotted.
		if(l)this->SendQuery(query);
		query.clear();
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
		query << "DELETE FROM `" << friend_db << "` WHERE `char_id` = '" << p.char_id << "'";
		this->SendQuery(query);
		query.clear();

		//insert here.
		query << "INSERT INTO `" << friend_db << "`(`char_id`, `friend_id`) VALUES ";

		for(i=0;i<MAX_FRIENDLIST;i++)
		{
			if(p.friendlist[i].friend_id!=0)
			{
				query <<
				(l?",":"") << "(" <<
				"'" << p.char_id << "'," <<
				"'" << p.friendlist[i].friend_id << "'" <<
				")";

				l = true;
			}
		}
		// if at least one entry spotted.
		if(l)this->SendQuery(query);
		query.clear();
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
	string<> query;

	query << "DELETE FROM `" << char_db << "` WHERE `account_id` = '" << accid << "'";

	this->SendQuery(query);
	return true;
}

/*************************************************/
/************************MAIL STUFF***************/
/*************************************************/
size_t CCharDB_sql::getUnreadCount(uint32 cid)
{
	string<> query;
	size_t count = 0;

	query
		<< "SELECT count(*) "
		<< "FROM `" << mail_db << "` WHERE `to_account_id` = '" << cid << "' AND `read_flag` = '0'";

	if( this->SendQuery(query) )
	{
		this->FetchResults();
		count = atol(this->row[0]);

		this->FreeResults();
	}
	return count;
}

size_t CCharDB_sql::listMail(uint32 cid, unsigned char box, unsigned char *buffer)
{
	string<> query;
	query
		<< "SELECT `message_id`,`read_flag`,`from_char_name` "
		<< "FROM `" << mail_db << "` WHERE `to_account_id` = '"<< cid << "'";

	if( this->SendQuery(query) )
	{
		size_t count=0;
		unsigned char *buf = buffer;
		while( this->FetchResults() )
		{
			CMailHead mailhead( atol(this->row[0]), atol(this->row[1]), this->row[2], "" );
			mailhead._tobuffer(buf); // automatic buffer increment
			count++;
		}

		this->FreeResults();
		return count;
	}
	return 0;
}

bool CCharDB_sql::readMail(uint32 cid, uint32 mid, CMail& mail)
{
	string<> query;
	bool ret = false;

	query
		<< "SELECT `read_flag`,`from_char_name`,`message` "
		<< "FROM `" << mail_db << "` WHERE `to_account_id` = '" << cid << "' AND `message_id` = '" << mid << "'";

	// default clearing
	mail.read    = 0;
	mail.name[0] = 0;
	mail.head[0] = 0;
	mail.body[0] = 0;

	if( this->SendQuery(query) )
	{
		if( this->FetchResults() )
		{
			ret = true;
			mail = CMail(mid, atol(this->row[0]), this->row[1], "", this->row[2] );
			if( 0==mail.read )
			{
				query.clear();
				query <<
					"UPDATE `" << mail_db << "` SET `read_flag`='1' WHERE `message_id`= '" << mid << "'";
				this->SendQuery(query);
			}
		}
		this->FreeResults();
	}
	return ret;
}

bool CCharDB_sql::deleteMail(uint32 cid, uint32 mid)
{
	string<> query;

	query
		<< "DELETE "
		<< "FROM `" << mail_db << "` WHERE `to_account_id` = '" << cid << "' AND `message_id` = '" << mid << "'";
	return this->SendQuery(query);
}

bool CCharDB_sql::sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid)
{
	bool ret = false;
	bool l = false; // if query is built
	string<> query;

	char _head[128];
	char _body[128];
	char _targetname[32];

	escape_string(_targetname, targetname, strlen(targetname));
	escape_string(_head, head, strlen(head));
	escape_string(_body, body, strlen(body));

	if( 0==strcmp(targetname,"*") )
	{
		query.clear();
		query
			<< "SELECT DISTINCT `char_id`,`name` "
			<< "FROM `" << char_db << "` WHERE `char_id` <> '" << senderid << "' ORDER BY `char_id`";
	}
	else
	{
		query.clear();
		query
			<< "SELECT `char_id`,`name` "
			<< "FROM `" << char_db << "` WHERE `name` = '" << _targetname << "'";
	}

	if( this->SendQuery(query) )
	{
		query.clear();
		query
			<< "INSERT DELAYED INTO `" << mail_db << "` "
			<< "(`to_account_id`,`to_char_name`,`from_account_id`,`from_char_name`,`message`,`read_flag`)"
			<< " VALUES ";

		while( this->FetchResults() )
		{

			query
				<< (l?",":"") << "('" << atol(this->row[0]) << "', '" << this->row[1] << "', '" << senderid << "', '" << sendername << "', '" << _body << "', 0)";

			l = true;
		}
		this->FreeResults();
	}

	if(l) ret &= this->SendQuery(query);

	return ret;
}

/******************************************************************************
******************************************************************************
*******     *****  ******  ***  ***  ***********       *********     *********
****   ****  ****  ******  ***  ***  ***********  ***      **   ****  ********
****  ***********  ******  ***  ***  ***********  *******  **   **************
****  ***********  ******  ***  ***  ***********  *******  **   **************
****  ****   ****  ******  ***  ***  ***********  *******  ***        ********
****  *****  ****  ******  ***  ***  ***********  *******  **********  *******
*****  ****  *****  ****  ****  ***  ***********  *******  **********  *******
******      ******  ****  ****  ***          ***  ***      ***  *****  *******
*******************      *****  ***          ***       *******        ********
****************************************************************    **********/

bool CGuildDB_sql::searchGuild(const char* name, CGuild& g)
{
	uint32 guild_id = 0;
	string<> query;

	query
		<< "SELECT `guild_id` FROM `" << guild_db << "` WHERE `name` = '" << name <<"'";

	if ( this->SendQuery(query) )
	{
		if( this->FetchResults() )
			guild_id = atol(this->row[0]);
		this->FreeResults();
	}

	if(guild_id)
		return false;

	return this->searchGuild(guild_id,g);
}

bool CGuildDB_sql::searchGuild(uint32 guildid, CGuild& g)
{
	// select guild_data where guild_id = guildid
	// return true and guild data
	return false;
}



bool CGuildDB_sql::insertGuild(const struct guild_member &m, const char *name, CGuild &g)
{
	char t_name[100],t_master[24],t_mes1[60],t_mes2[240];
	string<> query;


	this->escape_string(t_name, g.name, strlen(g.name));
	this->escape_string(t_master, g.member[0].name, strlen(g.member[0].name));
	this->escape_string(t_mes1, g.mes2, strlen(g.mes1));
	this->escape_string(t_mes2, g.mes2, strlen(g.mes2));

	query
		<< "INSERT INTO `" << guild_db << "` "
		"(`guild_id`,`name`,`master`,`char_id`) "
		"VALUES "
		<< "("
			<< "'" << g.guild_id			<< "',"
			<< "'" << t_name				<< "',"
			<< "'" << t_master				<< "',"
			<< "'" << g.member[0].char_id	<< "'"
		<< ")";
	this->SendQuery(query);
	query.clear();

	return this->saveGuild(g); // Save the rest of the guild now that we have the basics inserted

	return false;
}
bool CGuildDB_sql::removeGuild(uint32 guild_id)
{
	string<> query;
	query
		<< "DELETE FROM `" << guild_db << "` WHERE guild_id = '" << guild_id << "'";

	this->SendQuery(query);

	//removeFromMemory(guild_id);

	return true;
}

bool CGuildDB_sql::saveGuild(const CGuild& g)
{

	// 1 `guild` (`guild_id`, `name`,`master`,`guild_lv`,`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`castle_id`,`mes1`,`mes2`,`emblem_len`,`emblem_id`,`emblem_data`)
	// 2 `guild_member` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`rsv1`,`rsv2`,`name`)
	// 4 `guild_position` (`guild_id`,`position`,`name`,`mode`,`exp_mode`)
	// 8 `guild_alliance` (`guild_id`,`opposition`,`alliance_id`,`name`)
	// 16 `guild_expulsion` (`guild_id`,`name`,`mes`,`acc`,`account_id`,`rsv1`,`rsv2`,`rsv3`)
	// 32 `guild_skill` (`guild_id`,`id`,`lv`)

	char t_name[100],t_master[24],t_mes1[60],t_mes2[240],t_member[24],t_position[24],t_alliance[24];  // temporay storage for str convertion;
	char t_ename[24],t_emes[40];
	char emblem_data[4096];
	int i = 0, len = 0;
	int guild_member=0,guild_online_member=0;

	if (g.guild_id<=0) return false;

	this->escape_string(t_name, g.name, strlen(g.name));

	printf("(\033[1;35m%d\033[0m)  Request save guild -(flag 0x%x) ",g.guild_id);

	while (i<g.max_member) {
		if (g.member[i].account_id>0) guild_online_member++;
		i++;
	}


	//printf("- Insert guild %d to guild\n",g->guild_id);
	for(i=0;i<g.emblem_len;i++){
		len+=snprintf(emblem_data+len,(sizeof(emblem_data)-len), "%02x",(unsigned char)(g.emblem_data[i]));
		//printf("%02x",(unsigned char)(g->emblem_data[i]));
	}
	emblem_data[len] = '\0';
	//printf("- emblem_len = %d \n",g->emblem_len);


	{
		string<> query;
		query
			<< "UPDATE `" << guild_db << "` SET"
				<< "`guild_lv`='" <<		g.guild_lv			<< "',"
				<< "`connect_member`='" <<	g.connect_member	<< "',"
				<< "`max_member`='" <<		g.max_member		<< "',"
				<< "`average_lv`='" <<		g.average_lv		<< "',"
				<< "`exp`='" <<				g.exp 				<< "',"
				<< "`next_exp`='" <<		g.next_exp			<< "',"
				<< "`skill_point`='" <<		g.skill_point		<< "',"
				<< "`mes1`='" <<			t_mes1				<< "',"
				<< "`mes2`='" <<			t_mes2				<< "',"
				<< "`emblem_len`='" <<		len					<< "',"
				<< "`emblem_id`='" <<		g.emblem_id			<< "',"
				<< "`emblem_data`='" <<		emblem_data			<< "'"

			<< "WHERE `guild_id`='" <<		g.guild_id 			<< "'";

		this->SendQuery(query);
	}

	{
		string<> query2;
		string<> query;
		int l = 0;

		struct guild_member *m;
		// Re-writing from scratch (Aru)
		query
			<< "DELETE from `" << guild_member_db << "` where `guild_id` = '" << g.guild_id << "'";
		this->SendQuery(query);
		query.clear();

		// Remove guild IDs' from the character information sheets
		query
			<< "UPDATE `" << char_db << "` SET `guild_id` = '0' WHERE `guild_id` = '" << g.guild_id << "'";
		this->SendQuery(query);
		query.clear();


		// Now reinsert all the data
		// Begin building long insert
		query << "INSERT INTO `" << guild_member_db << "` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name`) VALUES ";
		query2 << "UPDATE `" << char_db << "` SET `guild_id` = '" << g.guild_id << "' WHERE `char_id` IN ";

		for(i=0;i<g.max_member;i++)
		{

			if(g.member[i].account_id > 0)
			{
				this->escape_string(t_member, g.member[i].name, strlen(g.member[i].name));

				query
					<< (l?",":"")
					<< "("
						<< "'" << g.guild_id 					<< "',"
						<< "'" << g.member[i].account_id		<< "',"
						<< "'" << g.member[i].char_id			<< "',"
						<< "'" << g.member[i].hair				<< "',"
						<< "'" << g.member[i].hair_color		<< "',"
						<< "'" << g.member[i].gender			<< "',"
						<< "'" << g.member[i].class_			<< "',"
						<< "'" << g.member[i].lv				<< "',"
						<< "'" << g.member[i].exp				<< "',"
						<< "'" << g.member[i].exp_payper		<< "',"
						<< "'" << g.member[i].online			<< "',"
						<< "'" << g.member[i].position			<< "',"
						<< "'" << t_member						<< "'"
					<< ")";


				query2
					<< (l?",":"")
					<< "'" << g.member[i].char_id << "'";

				l++;

			}
		}

		if(l) // if there was a complete query
		{
			this->SendQuery(query);
			this->SendQuery(query2);
		}
	}

	{
		//printf("- Insert guild %d to guild_position\n",g->guild_id);
		string<> query;

		query
			<< "REPLACE INTO `" << guild_position_db << "` (`guild_id`,`position`,`name`,`mode`,`exp_mode`) VALUES ";

		int l = 0;

		for(i=0;i<MAX_GUILDPOSITION;i++)
		{
			this->escape_string(t_position, g.position[i].name, strlen(g.position[i].name));

			query
				<< (l?",":"")
				<< "("
					<< "'" << g.guild_id	<< "',"
					<< "'" << i				<< "',"
					<< "'" << t_position	<< "',"
					<< "'" << g.position[i].mode		<< "',"
					<< "'" << g.position[i].exp_mode	<< "'"

				<< ")";
			l++;
		}

		if(l) // if there was a complete query
		{
			this->SendQuery(query);
		}
	}

	{
		string<> query;
		int l = 0;

//		printf("- Delete guild %d from guild_alliance\n",g->guild_id);
		query << "DELETE FROM `" << guild_alliance_db << "` WHERE '" << g.guild_id << "' IN (`alliance_id`,`guild_id`)";
		this->SendQuery(query);
		query.clear();


//		printf("- Insert guild %d to guild_alliance\n",g->guild_id);
		query << "REPLACE INTO `" << guild_alliance_db << "` (`guild_id`,`opposition`,`alliance_id`,`name`) VALUES ";
		for(i=0;i<MAX_GUILDALLIANCE;i++)
		{
			if(g.alliance[i].guild_id>0)
			{
				this->escape_string(t_alliance, g.alliance[i].name, strlen(g.alliance[i].name));
				query
					<< (l?",":"")
					<< "(" // Guild alliance for the current guild
						<< "'" << g.guild_id	<< "',"
						<< "'" << g.alliance[i].opposition	<< "',"
						<< "'" << g.alliance[i].guild_id	<< "',"
						<< "'" << t_alliance	<< "'"
					<< "),"
					<< "(" // Guild alliance for the other guild
						<< "'" << g.alliance[i].guild_id	<< "',"
						<< "'" << g.alliance[i].opposition	<< "',"
						<< "'" << g.guild_id	<< "',"
						<< "'" << t_name		<< "'"
					<< ")";
				l++;
			}
		}

		this->SendQuery(query);

	}

	{
		string<> query;
		int l = 0;

//		printf("- Insert guild %d to guild_expulsion\n",g->guild_id);

		query << "REPLACE INTO `" << guild_expulsion_db << "` (`guild_id`,`name`,`mes`,`acc`,`account_id`,`rsv1`,`rsv2`,`rsv3`) VALUES";

		for(i=0;i<MAX_GUILDEXPLUSION;i++)
		{

			if(g.explusion[i].account_id>0)
			{
				this->escape_string(t_ename, g.explusion[i].name, strlen(g.explusion[i].name));
				this->escape_string(t_emes, g.explusion[i].mes, strlen(g.explusion[i].mes));

				query
					<< (l?",":"")
					<< "("
						<< "'" << g.guild_id					<< "',"
						<< "'" << t_ename						<< "',"
						<< "'" << t_emes						<< "',"
						<< "'" << g.explusion[i].acc			<< "',"
						<< "'" << g.explusion[i].account_id		<< "',"
						<< "'" << g.explusion[i].rsv1			<< "',"
						<< "'" << g.explusion[i].rsv2			<< "',"
						<< "'" << g.explusion[i].rsv3			<< "'"
					<< ")";
				l++;
			}
		}

		if (l)
		{
			this->SendQuery(query);
		}

	}

	{
		string<> query;
		int l = 0;

//		printf("- Insert guild %d to guild_skill\n",g->guild_id);

		query << "REPLACE INTO `" << guild_skill_db << "` (`guild_id`,`id`,`lv`) VALUES ";

		for(i=0;i<MAX_GUILDSKILL;i++)
		{
			if (g.skill[i].id>0)
			{
				query
					<< (l?",":"")
					<< "("
						<< "'" << g.guild_id		<< "',"
						<< "'" << g.skill[i].id		<< "',"
						<< "'" << g.skill[i].lv		<< "'"
					<< ")";
				l++;
			}
		}
	}

	printf("Save guild %d (%s) done\n",g.guild_id,g.name);
	return true;
}

bool CGuildDB_sql::searchCastle(ushort castle_id, CCastle& castle)
{
	// select data from castle_db where castle_id = cid
	//else
	return false;
}
bool CGuildDB_sql::saveCastle(CCastle& castle)
{
	// update castle_db set new_data where castle_id = *castle.id
	// else
	return false;
}
bool CGuildDB_sql::removeCastle(ushort castle_id)
{
	// Delete from castle_db where castle_id = *cid
	// else
	string<> query;
	query
		<< "DELETE FROM `" << castle_db << "` WHERE castle_id = '" << castle_id << "'";

	//removeFromMemory(castle_id);
	this->SendQuery(query);
	return true;

}





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

	string<> party_db;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDB_sql(const char *configfile)
	{
		party_db = "party";
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
		string<> query;
		query << "DELETE FROM `" << party_db << "` WHERE `party_id` = '" << pid << "'";
		return this->SendQuery(query);
	}
	virtual bool saveParty(const CParty& party)
	{/*
		string<> query;
		query "REPLACE INTO `" << party_db << "` (`contents`) VALUES ";

		// update party set values=*party
		// return true */
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

