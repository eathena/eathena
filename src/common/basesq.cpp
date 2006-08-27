// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// SELECT element FROM table ORDER BY name LIMIT OFFSET,NUMBER


#include "basesq.h"


#if defined(WITH_MYSQL)


///////////////////////////////////////////////////////////////////////////////
// sql base interface.
// wrapper for the sql handle, table control and parameter storage

basics::CMySQL CSQLParameter::sqlbase;

basics::CParam< basics::string<> > CSQLParameter::mysqldb_id("sql_username", "ragnarok",  &ParamCallback_Database_string);
basics::CParam< basics::string<> > CSQLParameter::mysqldb_pw("sql_password", "ragnarok",  &ParamCallback_Database_string);
basics::CParam< basics::string<> > CSQLParameter::mysqldb_db("sql_database", "ragnarok",  &ParamCallback_Database_string);
basics::CParam< basics::string<> > CSQLParameter::mysqldb_ip("sql_ip",       "127.0.0.1", &ParamCallback_Database_string);
basics::CParam< basics::string<> > CSQLParameter::mysqldb_cp("sql_codepage", "DEFAULT", &ParamCallback_Database_string);
basics::CParam< ushort   >         CSQLParameter::mysqldb_port("sql_port",   3306,        &ParamCallback_Database_ushort);



basics::CParam< basics::string<> > CSQLParameter::tbl_login_log("tbl_login_log", "login_log", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_char_log("tbl_char_log", "char_log", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_map_log("tbl_map_log", "map_log", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_login_status("tbl_login_status", "login_status", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_char_status("tbl_char_status", "char_status", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_map_status("tbl_map_status", "map_status", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_account("tbl_account", "account", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_char("tbl_char", "character", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_memo("tbl_memo", "memo", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_inventory("tbl_inventory", "inventory", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_cart("tbl_cart", "cart", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_skill("tbl_skill", "skill", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_friends("tbl_friends", "friends", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_mail("tbl_mail", "mail", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_login_reg("tbl_login_reg", "login_reg", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_login_reg2("tbl_login_reg2", "login_reg2", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_char_reg("tbl_char_reg", "char_reg", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_guild("tbl_guild","guild", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_skill("tbl_guild_skill","guild_skill", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_member("tbl_guild_member","guild_member", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_position("tbl_guild_position","guild_position", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_alliance("tbl_guild_alliance","guild_alliance", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_expulsion("tbl_guild_expulsion","guild_expulsion", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_castle("tbl_castle","castle", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_castle_guardian("tbl_castle_guardian", "castle_guardian", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_party ("tbl_party", "party", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_storage("tbl_storage", "storage", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_guild_storage("tbl_guild_storage", "guild_storage", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_pet("tbl_pet", "pet", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_homunculus("tbl_homunculus", "homunculus", ParamCallback_Tables);
basics::CParam< basics::string<> > CSQLParameter::tbl_homunskill("tbl_homunskill", "homunskill", ParamCallback_Tables);

basics::CParam< basics::string<> > CSQLParameter::tbl_variable("tbl_variable", "variable", ParamCallback_Tables);


basics::CParam<bool> CSQLParameter::wipe_sql("wipe_sql", false);
basics::CParam< basics::string<> > CSQLParameter::sql_engine("sql_engine", "InnoDB"); // or "MyISAM"

basics::CParam<bool> CSQLParameter::log_login("log_login", true);
basics::CParam<bool> CSQLParameter::log_char("log_char", true);
basics::CParam<bool> CSQLParameter::log_map("log_map", true);


bool CSQLParameter::ParamCallback_Database_string(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval)
{
	sqlbase.init(mysqldb_id, mysqldb_pw,mysqldb_db,mysqldb_ip,mysqldb_port, mysqldb_cp);
	return true;
}
bool CSQLParameter::ParamCallback_Database_ushort(const basics::string<>& name, ushort& newval, const ushort& oldval)
{
	sqlbase.init(mysqldb_id, mysqldb_pw,mysqldb_db,mysqldb_ip,mysqldb_port, mysqldb_cp);
	return true;
}
bool CSQLParameter::ParamCallback_Tables(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval)
{
	CSQLParameter::rebuild();
	return true;
}


void CSQLParameter::rebuild()
{
	// from mysql manual: 

	// restrictions on InnoDB:
	// 
	// * `InnoDB' does not support the `AUTO_INCREMENT' table option for
	//   setting the initial sequence value in a `CREATE TABLE' or `ALTER
	//   TABLE' statement.  To set the value with `InnoDB', insert a dummy
	//   row with a value one less and delete that dummy row, or insert the
	//   first row with an explicit value specified.
	//
	// foreign key issues:
	//
	// * If `ON DELETE' is the only referential integrity capability an
	//   application needs, note that as of MySQL Server 4.0, you can use
	//   multiple-table `DELETE' statements to delete rows from many tables
	//   with a single statement. *Note `DELETE': DELETE.
	//   
	// * A workaround for the lack of `ON DELETE' is to add the appropriate
	//   `DELETE' statement to your application when you delete records
	//   from a table that has a foreign key. In practice, this is often as
	//   quick as using foreign keys, and is more portable.
	// * Foreign key support addresses many referential integrity issues,
	//   but it is still necessary to design key relationships carefully to
	//   avoid circular rules or incorrect combinations of cascading
	//   deletes.
	//   
	// * It is not uncommon for a DBA to create a topology of relationships
	//   that makes it difficult to restore individual tables from a backup.
	//   (MySQL alleviates this difficulty by allowing you to temporarily
	//   disable foreign key checks when reloading a table that depends on
	//   other tables.  *Note InnoDB foreign key constraints::.
	//
	//
	//   * `INSERT DELAYED' works only with `MyISAM' and `ISAM' tables.  For
	//	 `MyISAM' tables, if there are no free blocks in the middle of the
	//	 data file, concurrent `SELECT' and `INSERT' statements are
	//	 supported.  Under these circumstances, you very seldom need to use
	//	 `INSERT DELAYED' with `MyISAM'.  *Note `MyISAM' storage engine:
	//	 MyISAM storage engine.


	basics::CMySQLConnection dbcon1(CSQLParameter::sqlbase);
	basics::string<> query;

	///////////////////////////////////////////////////////////////////////
	// disable foreign keys
	query << "SET FOREIGN_KEY_CHECKS=0";
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	// drop all tables, drop child tables first
	if( CSQLParameter::wipe_sql )
	{
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_variable) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_homunskill) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_homunculus) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_pet) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_party) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_castle_guardian) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_castle) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_expulsion) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_alliance) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_position) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_skill) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_member) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_storage) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_storage) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_mail) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_skill) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_memo) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_cart) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_inventory) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_friends) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_char_reg) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_char) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_reg2) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_reg) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_account) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_status) << "`";
		dbcon1.PureQuery(query);
		query.clear();
		///////////////////////////////////////////////////////////////////////
		query << "DROP TABLE IF EXISTS `" 
			<< dbcon1.escaped(CSQLParameter::tbl_login_log) << "`";
		dbcon1.PureQuery(query);
		query.clear();
	}


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_log) << "` "
			 "("
			 "`time`	TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
			 "`ip`		VARCHAR(16) NOT NULL,"
			 "`user`	VARCHAR(24) NOT NULL,"
			 "`rcode`	INTEGER(3) UNSIGNED NOT NULL,"
			 "`log`		VARCHAR(100) NOT NULL"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_status) << "` "
			 "("
			 "`index` INTEGER UNSIGNED NOT NULL,"
			 "`name` VARCHAR(24) NOT NULL,"
			 "`user` INTEGER UNSIGNED NOT NULL,"
			 "PRIMARY KEY(`index`)"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_account_num("start_account_num", 10000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` "
			 "("
			 "`account_id`   INTEGER UNSIGNED AUTO_INCREMENT,"
			 "`user_id`      VARCHAR(24) NOT NULL,"
			 "`user_pass`    VARCHAR(34) NOT NULL,"
			 "`sex`          ENUM('M','F','S') default 'M',"
			 "`gm_level`     INT(3) UNSIGNED NOT NULL default '0',"
			 "`online`       BOOL default '0',"
			 "`email`        VARCHAR(40) NOT NULL default 'a@a.com',"
			 "`login_id1`    INTEGER UNSIGNED NOT NULL default '0',"
			 "`login_id2`    INTEGER UNSIGNED NOT NULL  default '0',"
			 "`client_ip`    VARCHAR(16) NOT NULL default '',"
			 "`last_login`   VARCHAR(24) NOT NULL default '',"
			 "`login_count`  INTEGER UNSIGNED NOT NULL default '0',"
			 "`ban_until`    INTEGER UNSIGNED NOT NULL default '0',"
			 "`valid_until`  INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`account_id`)"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_account_num;
	dbcon1.PureQuery(query);
	query.clear();

	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` "
			 "(`account_id`, `user_id`,`user_pass`) "
			 "VALUES "
			 "('" << start_account_num << "',' ',' ')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` "
			 "WHERE `account_id`=" << start_account_num;
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	// add the default accounts 
	//## change to inserting data from the config file
	if( CSQLParameter::wipe_sql )
	{
		query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` "
				 "(`user_id`,`user_pass`,`sex`) VALUES ('s1','p1','S'),('s2','p2','S'),('s3','p3','S')";
		dbcon1.PureQuery(query);
		query.clear();
	}

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_reg) << "` "
			 "("
			 "`account_id`      INTEGER UNSIGNED NOT NULL,"
			 "`str`             VARCHAR(34) NOT NULL,"
			 "`value`           VARCHAR(255) NOT NULL default '',"
			 "PRIMARY KEY (`account_id`,`str`),"
			 "KEY `account_id` (`account_id`),"
			 "FOREIGN KEY (`account_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` (`account_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_login_reg2) << "` "
			 "("
			 "`account_id`      INTEGER UNSIGNED NOT NULL,"
			 "`str`             VARCHAR(34) NOT NULL,"
			 "`value`           VARCHAR(255) NOT NULL default '',"
			 "PRIMARY KEY (`account_id`,`str`),"
			 "KEY `account_id` (`account_id`),"
			 "FOREIGN KEY (`account_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` (`account_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_char_num("start_char_num", 20000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` "
			 "("
			 "`char_id` 		INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`account_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`slot`			TINYINT UNSIGNED NOT NULL default '0',"
			 "`name`			VARCHAR(24) NOT NULL default '',"
			 "`class`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`base_level`		SMALLINT UNSIGNED NOT NULL default '1',"
			 "`job_level`		SMALLINT UNSIGNED NOT NULL default '1',"
			 "`base_exp`		BIGINT UNSIGNED NOT NULL default '0',"
			 "`job_exp`			BIGINT UNSIGNED NOT NULL default '0',"
			 "`zeny` 			BIGINT UNSIGNED NOT NULL default '0',"
			 "`str` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`agi` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`vit` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`int`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`dex` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`luk`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`max_hp` 			INTEGER UNSIGNED NOT NULL default '0',"
			 "`hp`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`max_sp`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`sp` 				INTEGER UNSIGNED NOT NULL default '0',"
			 "`status_point`	SMALLINT UNSIGNED NOT NULL default '0',"
			 "`skill_point`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`option` 			SMALLINT NOT NULL default '0',"
			 "`karma`			TINYINT NOT NULL default '0',"
			 "`chaos`			TINYINT NOT NULL default '0',"
			 "`manner`			SMALLINT NOT NULL default '0',"
			 "`party_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`pet_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`hair`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`hair_color` 		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`clothes_color`	SMALLINT UNSIGNED NOT NULL default '0',"
			 "`weapon`			SMALLINT UNSIGNED NOT NULL default '1',"
			 "`shield`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`head_top`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`head_mid`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`head_bottom`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`last_map`		VARCHAR(20) NOT NULL default 'new_5-1',"
			 "`last_x`			SMALLINT UNSIGNED NOT NULL default '53',"
			 "`last_y`			SMALLINT UNSIGNED NOT NULL default '111',"
			 "`save_map`		VARCHAR(20) NOT NULL default 'new_5-1',"
			 "`save_x`			SMALLINT UNSIGNED NOT NULL default '53',"
			 "`save_y`			SMALLINT UNSIGNED NOT NULL default '111',"
			 "`partner_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`father_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`mother_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`child_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`fame_points`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`online`			BOOL NOT NULL default '0',"
			 "PRIMARY KEY (`char_id`),"
			 "KEY `account_id` (`account_id`),"
			 "KEY `party_id` (`party_id`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`account_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` (`account_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_char_num;
	dbcon1.PureQuery(query);
	query.clear();

	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` "
			 "(`char_id`) "
			 "VALUES "
			 "('" << start_char_num << "')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` "
			 "WHERE `char_id`=" << start_char_num;
	dbcon1.PureQuery(query);
	query.clear();



	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_char_reg) << "` "
			 "("
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`str`				VARCHAR(32) NOT NULL,"
			 "`value`			VARCHAR(255) NOT NULL default '',"
			 "PRIMARY KEY (`char_id`,`str`),"
			 "KEY `char_id` (`char_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_friends) << "` ("
			 "`char_id` 		INTEGER UNSIGNED NOT NULL default '0',"
			 "`friend_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY  (`char_id`,`friend_id`),"
			 "KEY `char_id` (`char_id`),"
			 "KEY `friend_id` (`friend_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
			 "FOREIGN KEY (`friend_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_inventory) << "` ("
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`nameid`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`amount`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`refine`			TINYINT(2) UNSIGNED NOT NULL default '0',"
			 "`attribute`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`identify`		TINYINT UNSIGNED NOT NULL default '1',"
			 "`card0` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card1` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card2`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card3`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "KEY `char_id` (`char_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_cart) << "` ("
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`nameid`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`amount`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`refine`			TINYINT(2) UNSIGNED NOT NULL default '0',"
			 "`attribute`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`identify`		TINYINT UNSIGNED NOT NULL default '1',"
			 "`card0` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card1` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card2`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card3`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "KEY `char_id` (`char_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_memo) << "` ("
			 "`char_id` 		INTEGER UNSIGNED NOT NULL default '0',"
			 "`map` 			VARCHAR(20) NOT NULL default '',"
			 "`x` 				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`y`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "KEY `char_id` (`char_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_skill) << "` ("
			 "`char_id` 		INTEGER UNSIGNED NOT NULL default '0',"
			 "`id`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`lv` 				SMALLINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`char_id`,`id`),"
			 "KEY `char_id` (`char_id`),"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_mail) << "` ("
			 "`message_id`		INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`to_char_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`to_char_name`	VARCHAR(24) NOT NULL default '',"
			 "`from_char_id`	INTEGER UNSIGNED NOT NULL default '0',"
			 "`from_char_name`	VARCHAR(24) NOT NULL default '',"
			 "`header`			VARCHAR(32) NOT NULL default '',"
			 "`message`			VARCHAR(80) NOT NULL default '',"
			 "`read_flag`		BOOL default '0',"
			 "`sendtime`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`zeny`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`item_nameid`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`item_amount`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`item_equip`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`item_identify`	TINYINT UNSIGNED NOT NULL default '1',"
			 "`item_refine`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`item_attribute`	TINYINT UNSIGNED NOT NULL default '0',"
			 "`item_card0` 		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`item_card1` 		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`item_card2` 		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`item_card3` 		SMALLINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`message_id`),"
			 "KEY `to_char_id` (`to_char_id`),"
			 "FOREIGN KEY (`to_char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=1";
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_storage) << "` "
			 "("
			 "`account_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`nameid`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`amount`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`refine`			TINYINT UNSIGNED NOT NULL default '0',"
			 "`attribute`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`identify`		TINYINT UNSIGNED NOT NULL default '1',"
			 "`card0` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card1` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card2`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card3`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "KEY `account_id` (`account_id`),"
			 "FOREIGN KEY (`account_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_account) << "` (`account_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_guild_num("start_guild_num", 30000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`guild_lv`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`connect_member`	SMALLINT UNSIGNED NOT NULL default '0',"
			 "`max_member`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`average_lv`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`exp`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`next_exp`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`skill_point`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`name`			VARCHAR(24) NOT NULL default '',"
			 "`master_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`mes1`			VARCHAR(64) NOT NULL default '',"
			 "`mes2`			VARCHAR(128) NOT NULL default '',"
			 "`emblem_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`emblem_len`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`emblem_data`		BLOB NOT NULL,"
			 "PRIMARY KEY (`guild_id`),"
			 "FOREIGN KEY (`master_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_guild_num;
	dbcon1.PureQuery(query);
	query.clear();

	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` "
			 "(`guild_id`) "
			 "VALUES "
			 "('" << start_guild_num << "')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` "
			 "WHERE `guild_id`=" << start_guild_num;
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_storage) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`nameid`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`amount`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`refine`			TINYINT UNSIGNED NOT NULL default '0',"
			 "`attribute`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`identify`		TINYINT UNSIGNED NOT NULL default '1',"
			 "`card0` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card1` 			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card2`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`card3`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_member) << "` "
			 "("
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`exp`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`exp_payper`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`position`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`rsv1`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`rsv2`			INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`char_id`, `guild_id`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_skill) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`id`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`lv`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`guild_id`, `id`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_position) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`position`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`name`			VARCHAR(24) NOT NULL default '',"
			 "`mode`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`exp_mode`		INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`guild_id`, `position`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_alliance) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`alliance_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`opposition`		INTEGER NOT NULL default '0',"
			 "PRIMARY KEY (`guild_id`, `alliance_id`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
			 "FOREIGN KEY (`alliance_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_guild_expulsion) << "` "
			 "("
			 "`guild_id`		INTEGER UNSIGNED NOT NULL default '0',"	
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`mes`				VARCHAR(40) NOT NULL default '',"
			 "`acc`				VARCHAR(40) NOT NULL default '',"
			 "`rsv1`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`rsv2`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`rsv3`			INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`guild_id`, `char_id`),"
			 "KEY `guild_id` (`guild_id`),"
			 "FOREIGN KEY (`guild_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_guild) << "` (`guild_id`) ON DELETE CASCADE ON UPDATE CASCADE,"
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_castle) << "` "
			 "("
			 "`castle_id`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`guild_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`economy`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`defense`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`triggerE`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`triggerD`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`nextTime`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`payTime`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`createTime`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`visibleC`			INTEGER UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY `castle_id` (`castle_id`),"
			 "KEY `guild_id` (`guild_id`)"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_castle_guardian) << "` "
			 "("
			 "`castle_id`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`guardian_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`guardian_hp`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`guardian_visible`	BOOL NOT NULL default '0',"
			 "PRIMARY KEY (`castle_id`,`guardian_id`),"
			 "KEY `castle_id` (`castle_id`)"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_party_num("start_party_num", 40000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_party) << "` "
			 "("
			 "`party_id`		INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`name`			VARCHAR(24) NOT NULL default '',"
			 "`leader`			VARCHAR(24) NOT NULL default '',"
			 "`expshare`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`itemshare`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`itemc`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`party_id`),"
			 "UNIQUE `name` (`name`),"
			 "UNIQUE `leader` (`leader`)"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_party_num;
	dbcon1.PureQuery(query);
	query.clear();

	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_party) << "` "
			 "(`party_id`) "
			 "VALUES "
			 "('" << start_party_num << "')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_party) << "` "
			 "WHERE `party_id`=" << start_party_num;
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_pet_num("start_pet_num", 50000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_pet) << "` "
			 "("
			 "`pet_id`			INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`class`			SMALLINT NOT NULL default '0',"
			 "`level`			SMALLINT NOT NULL default '0',"
			 "`egg_id`			SMALLINT NOT NULL default '0',"
			 "`equip_id`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`intimate`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`hungry`			SMALLINT NOT NULL default '0',"
			 "`name`			VARCHAR(24) NOT NULL default '',"
			 "`rename_flag`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`incuvate`		TINYINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY `pet_id` (`pet_id`),"
			 "KEY `char_id` (`char_id`)," 
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_pet_num;
	dbcon1.PureQuery(query);
	query.clear();
	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_pet) << "` "
			 "(`pet_id`) "
			 "VALUES "
			 "('" << start_pet_num << "')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_pet) << "` "
			 "WHERE `pet_id`=" << start_pet_num;
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_homun_num("start_homun_num", 60000000);
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_homunculus) << "` "
			 "("
			 "`homun_id`		INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,"
			 "`account_id`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`char_id`			INTEGER UNSIGNED NOT NULL default '0',"

			 "`base_exp`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`name`			VARCHAR(24) NOT NULL default '',"

			 "`hp`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`max_hp`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`sp`				INTEGER UNSIGNED NOT NULL default '0',"
			 "`max_sp`			INTEGER UNSIGNED NOT NULL default '0',"

			 "`class`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`status_point`	SMALLINT UNSIGNED NOT NULL default '0',"
			 "`skill_point`		SMALLINT UNSIGNED NOT NULL default '0',"

			 "`str`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`agi`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`vit`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`int`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`dex`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`luk`				SMALLINT UNSIGNED NOT NULL default '0',"

			 "`option`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"

			 "`intimate`		INTEGER UNSIGNED NOT NULL default '0',"
			 "`hungry`			SMALLINT NOT NULL default '0',"
			 "`equip`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`base_level`		SMALLINT UNSIGNED NOT NULL default '0',"
			 
			 "`rename_flag`		TINYINT UNSIGNED NOT NULL default '0',"
			 "`incubate`		TINYINT UNSIGNED NOT NULL default '0',"

			 "PRIMARY KEY `homun_id` (`homun_id`),"
			 "KEY `char_id` (`char_id`)," 
			 "FOREIGN KEY (`char_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_char) << "` (`char_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine) << " AUTO_INCREMENT=" << start_homun_num;
	dbcon1.PureQuery(query);
	query.clear();
	query << "INSERT INTO `" << dbcon1.escaped(CSQLParameter::tbl_homunculus) << "` "
			 "(`homun_id`) "
			 "VALUES "
			 "('" << start_homun_num << "')";
	dbcon1.PureQuery(query);
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(CSQLParameter::tbl_homunculus) << "` "
			 "WHERE `homun_id`=" << start_homun_num;
	dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_homunskill) << "` ("
			 "`homun_id` 		INTEGER UNSIGNED NOT NULL default '0',"
			 "`id`				SMALLINT UNSIGNED NOT NULL default '0',"
			 "`lv` 				SMALLINT UNSIGNED NOT NULL default '0',"
			 "PRIMARY KEY (`homun_id`,`id`),"
			 "KEY `homun_id` (`homun_id`),"
			 "FOREIGN KEY (`homun_id`) REFERENCES `" << dbcon1.escaped(CSQLParameter::tbl_homunculus) << "` (`homun_id`) ON DELETE CASCADE ON UPDATE CASCADE"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////////
	query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(CSQLParameter::tbl_variable) << "` ("
			 "`name`			VARCHAR(32) NOT NULL default '',"
			 "`stortype`		SMALLINT UNSIGNED NOT NULL default '0',"
			 "`storid`			INTEGER UNSIGNED NOT NULL default '0',"
			 "`vartype`			SMALLINT UNSIGNED NOT NULL default '0',"
			 "`value`			SMALLINT UNSIGNED NOT NULL default '0',"
		 
			 "PRIMARY KEY (`name`,`stortype`,`storid`),"
			 "KEY `name` (`name`),"
			 "KEY `storid` (`storid`),"
			 ") "
			"ENGINE = " << dbcon1.escaped(CSQLParameter::sql_engine);
	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////
	// enable foreign keys
	query << "SET FOREIGN_KEY_CHECKS=1";
	dbcon1.PureQuery(query);
	query.clear();
}
















//////////////////////////////////////////////////////////////////////////////////////
// CAccountDB_sql Class
//////////////////////////////////////////////////////////////////////////////////////
bool CAccountDB_sql::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return true;
}

bool CAccountDB_sql::close()
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	//set log.
	if( this->log_login )
	{
		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_login_log) << "` "
				 "(`time`,`ip`,`user`,`rcode`,`log`) "
				 "VALUES (NOW(), '', 'lserver','100', 'login server shutdown')";
		dbcon1.PureQuery(query);
		query.clear();
	}

	//delete all server status
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_login_status) << "`";
	dbcon1.PureQuery(query);
	query.clear();

	return true;
}

bool CAccountDB_sql::sql2struct(const basics::string<>& querycondition, CLoginAccount& account)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	query << "SELECT "
			 "`account_id`,"	//  0
			 "`user_id`,"		//  1
			 "`user_pass`,"		//  2
			 "`sex`,"			//  3
			 "`gm_level`,"		//  4
			 "`online`,"		//  5
			 "`email`,"			//  6
			 "`login_id1`,"		//  7
			 "`login_id2`,"		//  8
			 "`client_ip`,"		//  9
			 "`last_login`,"	// 10
			 "`login_count`,"	// 11
			 "`ban_until`,"		// 12
			 "`valid_until` "	// 13
			 "FROM `" << dbcon1.escaped(this->tbl_account) << "` " <<
			 querycondition;

	if( dbcon1.ResultQuery(query) )
	{
		account.account_id	= atol(dbcon1[0]);
		safestrcpy(account.userid, sizeof(account.userid), dbcon1[1]);
		safestrcpy(account.passwd, sizeof(account.passwd), dbcon1[2]);
		account.sex			= ((dbcon1[3][0]=='S') ? (2) : (dbcon1[3][0]=='M'));
		account.gm_level	= atol(dbcon1[4]);
		account.online		= atol(dbcon1[5]);
		safestrcpy(account.email, sizeof(account.email), dbcon1[6]);
		account.login_id1	= atol(dbcon1[7]);
		account.login_id2	= atol(dbcon1[8]);
		account.client_ip	= basics::ipaddress(dbcon1[9]);
		safestrcpy(account.last_ip, sizeof(account.last_ip), dbcon1[9]);
		safestrcpy(account.last_login, sizeof(account.last_login), dbcon1[10]);
		account.login_count	= atol(dbcon1[11]);
		account.ban_until	= (time_t)(atol(dbcon1[12]));
		account.valid_until	= (time_t)(atol(dbcon1[13]));

		query.clear();
		query << "SELECT `str`,`value` "
				 "FROM `" << dbcon1.escaped(this->tbl_login_reg) << "` "
				 "WHERE `account_id`='" << account.account_id << "'";
		dbcon1.ResultQuery(query);
		for(i=0; dbcon1 && i<ACCOUNT_REG2_NUM; ++dbcon1, ++i)
		{
			safestrcpy(account.account_reg2[i].str, sizeof(account.account_reg2[0].str), dbcon1[0]);
			account.account_reg2[i].value = atoi(dbcon1[1]);
		}
		account.account_reg2_num = i;
		for( ; i<ACCOUNT_REG2_NUM; ++i)
		{
			account.account_reg2[i].str[0] = 0;
			account.account_reg2[i].value = 0;
		}
		return true;
	}
	return false;
}

size_t CAccountDB_sql::size() const
{
	return this->get_table_size(this->tbl_account);
}

CLoginAccount& CAccountDB_sql::operator[](size_t i)
{	// playing around with this
	static CLoginAccount account;
	basics::string<> querycondition;
	querycondition <<	"ORDER BY `account_id`"
						"LIMIT "<< i << ",1 ";
	sql2struct(querycondition, account);
	// might check the return value
	return account;
}



bool CAccountDB_sql::existAccount(const char* userid)
{	// check if account with userid already exist
	bool ret = false;
	if(userid)
	{
		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> query;

		query << "SELECT `user_id` "
				 "FROM `" << dbcon1.escaped(this->tbl_account) << "` "
				 "WHERE " << (this->case_sensitive?"BINARY ":"") << "`user_id` = '" <<  dbcon1.escaped(userid) << "'";
		
		if( dbcon1.ResultQuery(query) && dbcon1 )
		{
			ret = dbcon1.countResults()>0;
		}
	}
	return ret;
}

bool CAccountDB_sql::searchAccount(const char* userid, CLoginAccount& account)
{	// get account by user/pass
	if(userid)
	{
		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> querycondition;
		querycondition <<	"WHERE `user_id`='" << dbcon1.escaped(userid) << "'";
		return sql2struct(querycondition, account);
	}
	return false;
}

bool CAccountDB_sql::searchAccount(uint32 accid, CLoginAccount& account)
{	// get account by account_id
	if(accid)
	{
		basics::string<> querycondition;
		querycondition <<	"WHERE `account_id`='" << accid << "'";
		return sql2struct(querycondition, account);
	}
	return false;
}

bool CAccountDB_sql::insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount& account)
{	// insert a new account to db
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_account) << "` "
			 "(`user_id`, `user_pass`, `sex`, `email`) "
			 "VALUES ('" << 
			 dbcon1.escaped(userid) << "', '" << 
			 dbcon1.escaped(passwd) << "', '" << 
			 sex << "', '" << 
			 dbcon1.escaped(email) << "')";

	return	dbcon1.PureQuery(query) &&
			searchAccount(userid, account);
}

bool CAccountDB_sql::removeAccount(uint32 accid)
{
	bool ret;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "DELETE "
			 "FROM `" << this->tbl_account << "` "
			 "WHERE `account_id`='" << accid << "'";
	ret = dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << this->tbl_login_reg << "` "
			 "WHERE `account_id`='" << accid << "'"; // must update with the variable this->tbl_login_reg
	ret &=dbcon1.PureQuery(query);

	return ret;
}

bool CAccountDB_sql::saveAccount(const CLoginAccount& account)
{
	bool ret;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	size_t i, doit;
	basics::string<> query;

	//-----------
	// Update the this->tbl_account with new info
	query << "UPDATE `" << dbcon1.escaped(this->tbl_account) << "` "
			 "SET "
			 "`user_id` = '"			<< dbcon1.escaped(account.userid)<< "', "
			 "`user_pass` = '"			<< dbcon1.escaped(account.passwd)<< "', "
			 "`sex` = '"				<< ((account.sex==1)?"M":"F")	<< "', "
			 "`gm_level` = '"			<< account.gm_level				<< "', "
			 "`online` = '"				<< account.online				<< "', "
			 "`email` = '"				<< dbcon1.escaped(account.email)<< "', "
			 "`login_id1` = '"			<< account.login_id1			<< "', "
			 "`login_id2` = '"			<< account.login_id2			<< "', "
			 "`client_ip` = '"			<< account.client_ip			<< "', "
			 "`last_login` = '"			<< account.last_login			<< "', "
			 "`login_count` = '"		<< account.login_count			<< "', "
			 "`valid_until` = '"		<< (ulong)account.valid_until	<< "', "
			 "`ban_until` = '"			<< (ulong)account.ban_until		<< "' "
			 "WHERE `account_id` = '"	<< account.account_id			<< "'";

	ret = dbcon1.PureQuery(query);
	

	//----------
	// Delete and reinsert data for the registry values
	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_login_reg) << "` "
			 "WHERE `account_id`='" << account.account_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	// Only need to do registry inserts if we have registry stuff to insert.. =o
	if( account.account_reg2_num )
	{
		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_login_reg) << "` "
				 "(`account_id`, `str`, `value`) VALUES ";

		for(i=0, doit=0; i<account.account_reg2_num; ++i)
		{
			if( account.account_reg2[i].str[0] && account.account_reg2[i].value )
			{
				query << (doit?",":"") <<
					"("
					"'" << account.account_id << "',"
					"'" << dbcon1.escaped(account.account_reg2[i].str) << "',"
					"'" << account.account_reg2[i].value << "'"
					") ";
				doit++;
			}
		}
		if(doit) ret &= dbcon1.PureQuery(query);
	}
	return ret;
}








// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /
// / CHAR SERVER IMPLEMENTATION BY CLOWNISIUS  / / / / / / / / / / / / / / / / / /
// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / /



bool CCharDB_sql::init(const char* configfile)
{	// init db

	return true;
}

size_t CCharDB_sql::size() const
{
	return this->get_table_size(this->tbl_char);
}
CCharCharacter& CCharDB_sql::operator[](size_t i)
{
	// not threadsafe
	static CCharCharacter ch;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`char_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "ORDER BY `char_id`"
			 "LIMIT "<< i << ",1 ";

	if( dbcon1.ResultQuery(query) )
	{
		const uint32 char_id = atol(dbcon1[0]);
		if( this->searchChar(char_id, ch) )
			return ch;
	}
	ch.char_id=0;
	return ch;
}

bool CCharDB_sql::existChar(uint32 char_id)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT count(*) "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE char_id = '" << char_id << "'";
	return dbcon1.ResultQuery(query) && dbcon1 && (atol(dbcon1[0])>0);
}

bool CCharDB_sql::existChar(const char* name)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT count(*) "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE name = '" << dbcon1.escaped(name) << "'";
	return dbcon1.ResultQuery(query) && dbcon1 && (atol(dbcon1[0])>0);
}

bool CCharDB_sql::searchChar(const char* name, CCharCharacter &p)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	bool ret = false;
	basics::string<> query;
	query << "SELECT `char_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << " "
			 "WHERE `name`='" << dbcon1.escaped(name) << "'";
	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		const uint32 charid = atoi(dbcon1[0]);
		ret = this->searchChar(charid,p);
	}
	return ret;
}

bool CCharDB_sql::searchChar(uint32 char_id, CCharCharacter &p)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	// Load all base stats
	query.clear();
	query << "SELECT "
			 "`char_id`,"		// 0
			 "`account_id`,"	// 1
			 "`slot`,"			// 2
			 "`name`,"			// 3
			 "`class`,"			// 4
			 "`base_level`,"	// 5
			 "`job_level`,"		// 6
			 "`base_exp`,"		// 7
			 "`job_exp`,"		// 8
			 "`zeny`,"			// 9
			 "`str`,`agi`,`vit`,`int`,`dex`,`luk`," // 10 - 15
			 "`max_hp`,`hp`,"	//16 - 17
			 "`max_sp`,`sp`,"	//18 - 19
			 "`status_point`,"	//20
			 "`skill_point`,"	//21
			 "`option`,"		//22
			 "`karma`,"			//23
			 "`chaos`,"			//24
			 "`manner`,"		//25
			 "`party_id`,"		//26
			 "`guild_id`,"		//27
			 "`pet_id`,"		//28
			 "`hair`,"			//29
			 "`hair_color`,"	//30
			 "`clothes_color`,"	//31
			 "`weapon`,"		//32
			 "`shield`,"		//33
			 "`head_top`,"		//34
			 "`head_mid`,"		//35
			 "`head_bottom`,"	//36
			 "`last_map`,"		//37
			 "`last_x`,"		//38
			 "`last_y`,"		//39
			 "`save_map`,"		//40
			 "`save_x`,"		//41
			 "`save_y`,"		//42
			 "`partner_id`,"	//43
			 "`father_id`,"		//44
			 "`mother_id`,"		//45
			 "`child_id`,"		//46
			 "`fame_points` "	//47
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE `char_id` = '" << char_id << "'";

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		p.char_id 			= atoi(dbcon1[0]);
		p.account_id 		= atoi(dbcon1[1]);
		p.slot 				= atoi(dbcon1[2]);
		safestrcpy(p.name, sizeof(p.name),         dbcon1[3]);
		p.class_ 			= atoi(dbcon1[4]);
		p.base_level 		= atoi(dbcon1[5]);
		p.job_level 		= atoi(dbcon1[6]);
		p.base_exp 			= atoi(dbcon1[7]);
		p.job_exp 			= atoi(dbcon1[8]);
		p.zeny 				= atoi(dbcon1[9]);
		p.str 				= atoi(dbcon1[10]);
		p.agi 				= atoi(dbcon1[11]);
		p.vit 				= atoi(dbcon1[12]);
		p.int_				= atoi(dbcon1[13]);
		p.dex 				= atoi(dbcon1[14]);
		p.luk 				= atoi(dbcon1[15]);
		p.max_hp 			= atoi(dbcon1[16]);
		p.hp 				= atoi(dbcon1[17]);
		p.max_sp 			= atoi(dbcon1[18]);
		p.sp 				= atoi(dbcon1[19]);
		p.status_point 		= atoi(dbcon1[20]);
		p.skill_point 		= atoi(dbcon1[21]);
		p.option			= atoi(dbcon1[22]);
		p.karma				= atoi(dbcon1[23]);
		p.chaos				= atoi(dbcon1[24]);
		p.manner			= atoi(dbcon1[25]);
		p.party_id			= atoi(dbcon1[26]);
		p.guild_id			= atoi(dbcon1[27]);
		p.pet_id			= atoi(dbcon1[28]);
		p.hair				= atoi(dbcon1[29]);
		p.hair_color		= atoi(dbcon1[30]);
		p.clothes_color		= atoi(dbcon1[31]);
		p.weapon			= atoi(dbcon1[32]);
		p.shield			= atoi(dbcon1[33]);
		p.head_top			= atoi(dbcon1[34]);
		p.head_mid			= atoi(dbcon1[35]);
		p.head_bottom		= atoi(dbcon1[36]);
		safestrcpy(p.last_point.mapname, sizeof(p.last_point.mapname),dbcon1[37]);
		p.last_point.x		= atoi(dbcon1[38]);
		p.last_point.y		= atoi(dbcon1[39]);
		safestrcpy(p.save_point.mapname, sizeof(p.save_point.mapname),dbcon1[40]);
		p.save_point.x		= atoi(dbcon1[41]);
		p.save_point.y		= atoi(dbcon1[42]);
		p.partner_id		= atoi(dbcon1[43]);
		p.father_id			= atoi(dbcon1[44]);
		p.mother_id			= atoi(dbcon1[45]);
		p.child_id			= atoi(dbcon1[46]);
		p.fame_points		= atoi(dbcon1[47]);

		///////////////////////////////////////////////////////////////////////
		// Check start/save locations
		if( p.last_point.x == 0 || p.last_point.y == 0 || p.last_point.mapname[0] == '\0')
		{
			ShowWarning("%s (%lu, %lu) has no last point?\n", p.name, (ulong)p.account_id, (ulong)p.char_id);
			p.last_point = this->start_point;
		}

		if (p.save_point.x == 0 || p.save_point.y == 0 || p.save_point.mapname[0] == '\0')
		{
			ShowWarning("%s (%lu, %lu) has no safe point?\n", p.name, (ulong)p.account_id, (ulong)p.char_id);
			p.save_point = this->start_point;
		}

		///////////////////////////////////////////////////////////////////////
		// Load Memo
		query.clear();
		query << "SELECT `map`,`x`,`y` "
				 "FROM `" << dbcon1.escaped(this->tbl_memo) << "` "
				 "WHERE `char_id`='" << char_id << "'";

		i=0;
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1 && i<MAX_MEMO; ++dbcon1, ++i)
			{
				safestrcpy(p.memo_point[i].mapname, sizeof(p.memo_point[i].mapname), dbcon1[0]);
				p.memo_point[i].x=atoi(dbcon1[1]);
				p.memo_point[i].y=atoi(dbcon1[2]);
			}
		}
		for( ; i<MAX_MEMO; ++i)
		{
			p.memo_point[i].mapname[0] = 0;
			p.memo_point[i].x = 0;
			p.memo_point[i].y = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Load Inventory
		query.clear();
		query << "SELECT "
				 "`nameid`,"		// 0
				 "`amount`,"		// 1
				 "`equip`,"			// 2
				 "`identify`,"		// 3
				 "`refine`,"		// 4
				 "`attribute`,"		// 5
				 "`card0`,"			// 6
				 "`card1`,"			// 7
				 "`card2`,"			// 8
				 "`card3`"			// 9
				 "FROM `" << dbcon1.escaped(this->tbl_inventory) << "` "
				 "WHERE `char_id`='" << char_id << "'";
		i=0;
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1 && i<MAX_INVENTORY; ++dbcon1, ++i)
			{
				p.inventory[i].nameid		= atoi(dbcon1[0]);
				p.inventory[i].amount		= atoi(dbcon1[1]);
				p.inventory[i].equip		= atoi(dbcon1[2]);
				p.inventory[i].identify		= atoi(dbcon1[3]);
				p.inventory[i].refine		= atoi(dbcon1[4]);
				p.inventory[i].attribute	= atoi(dbcon1[5]);
				p.inventory[i].card[0]		= atoi(dbcon1[6]);
				p.inventory[i].card[1]		= atoi(dbcon1[7]);
				p.inventory[i].card[2]		= atoi(dbcon1[8]);
				p.inventory[i].card[3]		= atoi(dbcon1[9]);
			}
		}
		for( ; i<MAX_INVENTORY; ++i)
		{
			p.inventory[i].nameid		= 0;
			p.inventory[i].amount		= 0;
			p.inventory[i].equip		= 0;
			p.inventory[i].identify		= 0;
			p.inventory[i].refine		= 0;
			p.inventory[i].attribute	= 0;
			p.inventory[i].card[0]		= 0;
			p.inventory[i].card[1]		= 0;
			p.inventory[i].card[2]		= 0;
			p.inventory[i].card[3]		= 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Load Cart
		query.clear();
		query << "SELECT "
				 "`nameid`,"		// 0
				 "`amount`,"		// 1
				 "`equip`,"			// 2
				 "`identify`,"		// 3
				 "`refine`,"		// 4
				 "`attribute`,"		// 5
				 "`card0`,"			// 6
				 "`card1`,"			// 7
				 "`card2`,"			// 8
				 "`card3`"			// 9
				 "FROM `"<< dbcon1.escaped(this->tbl_cart) << "` "
				 "WHERE `char_id`='" << char_id << "'";

		i=0;
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1 && i<MAX_CART; ++dbcon1, ++i)
			{
				p.cart[i].nameid		= atoi(dbcon1[0]);
				p.cart[i].amount		= atoi(dbcon1[1]);
				p.cart[i].equip			= atoi(dbcon1[2]);
				p.cart[i].identify		= atoi(dbcon1[3]);
				p.cart[i].refine		= atoi(dbcon1[4]);
				p.cart[i].attribute		= atoi(dbcon1[5]);
				p.cart[i].card[0]		= atoi(dbcon1[6]);
				p.cart[i].card[1]		= atoi(dbcon1[7]);
				p.cart[i].card[2]		= atoi(dbcon1[8]);
				p.cart[i].card[3]		= atoi(dbcon1[9]);
			}
		}
		for( ; i<MAX_CART; ++i)
		{
			p.cart[i].nameid		= 0;
			p.cart[i].amount		= 0;
			p.cart[i].equip			= 0;
			p.cart[i].identify		= 0;
			p.cart[i].refine		= 0;
			p.cart[i].attribute		= 0;
			p.cart[i].card[0]		= 0;
			p.cart[i].card[1]		= 0;
			p.cart[i].card[2]		= 0;
			p.cart[i].card[3]		= 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Load skill
		query.clear();
		query << "SELECT `id`, `lv` "
				 "FROM `" << dbcon1.escaped(this->tbl_skill) << "` "
				 "WHERE `char_id`='" << char_id << "'";

		for(i=0; i<MAX_SKILL; ++i)
		{
			p.skill[i].id = 0;
			p.skill[i].lv = 0;
			p.skill[i].flag = 0;
		}
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1; ++dbcon1)
			{
				i = atoi(dbcon1[0]);
				if(i<MAX_SKILL)
				{
					p.skill[i].id = i;
					p.skill[i].lv = atoi(dbcon1[1]);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Load registry
		query.clear();
		query << "SELECT `str`, `value` "
				 "FROM `" << dbcon1.escaped(this->tbl_char_reg) << "` "
				 "WHERE `char_id`='" << char_id << "'";

		i=0;
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1 && i<GLOBAL_REG_NUM; ++dbcon1, ++i)
			{
				safestrcpy(p.global_reg[i].str, sizeof(p.global_reg[i].str), dbcon1[0]);
				p.global_reg[i].value = atoi(dbcon1[1]);
			}
		}
		p.global_reg_num=i;
		for( ; i<GLOBAL_REG_NUM; ++i)
		{
			p.global_reg[i].str[0] = 0;
			p.global_reg[i].value = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Load Friends
		query.clear();
		query << "SELECT `f`.`friend_id`,`c`.`name` "
				 "FROM `" << dbcon1.escaped(this->tbl_friends) << "` `f` "
				 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `f`.`friend_id`=`c`.`char_id` "
				 "WHERE `f`.`char_id` = '" << char_id << "'";
		i=0;
		if( dbcon1.ResultQuery(query) )
		{
			for( ; dbcon1 && i<MAX_FRIENDLIST; ++dbcon1, ++i)
			{
				p.friendlist[i].friend_id = atoi(dbcon1[0]);
				safestrcpy(p.friendlist[i].friend_name, sizeof(p.friendlist[i].friend_name), dbcon1[1]);
			}
		}
		for( ; i<MAX_FRIENDLIST; ++i)
		{
			p.friendlist[i].friend_id = 0;
			p.friendlist[i].friend_name[0] = '\0';
		}
		///////////////////////////////////////////////////////////////////////
		return true;
	}
	return false;
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
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	p = CCharCharacter(n);

	//Check Name (already in use?)
	query << "SELECT count(*) "
			 "FROM `" << this->tbl_char << "` "
			 "WHERE `name` = '" << dbcon1.escaped(p.name) << "'";
	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		if( atol(dbcon1[0]) )
		{
			ShowError("char creation failed, charname '%s' already in use\n", p.name);
			return false;
		}
	}

	// check char slot.
	query.clear();
	query << "SELECT count(*) "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE `account_id` = '" << account.account_id << "' "
			 "AND `slot` = '" << slot << "'";
	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		if( atol(dbcon1[0]) )
		{
			ShowError("char creation failed, (aid: %d, slot: %d), slot already in use\n", account.account_id, slot);
			return false;
		}
	}
	// It has passed both the name and slot check, let's insert the info since it doesnt conflict =D

	
	///////////////////////////////////////////////////////////////////////////

	p.account_id = account.account_id;
	p.slot = slot;
	p.class_ = 0;
	p.base_level = 1;
	p.job_level = 1;
	p.base_exp = 0;
	p.job_exp = 0;
	p.zeny = start_zeny;
	p.str = str;
	p.agi = agi;
	p.vit = vit;
	p.int_ = int_;
	p.dex = dex;
	p.luk = luk;
	p.max_hp = 40 * (100 + vit) / 100;
	p.max_sp = 11 * (100 + int_) / 100;
	p.hp = p.max_hp;
	p.sp = p.max_sp;
	p.status_point = 0;
	p.skill_point = 0;
	p.option = 0;
	p.karma = 0;
	p.chaos = 0;
	p.manner = 0;
	p.party_id = 0;
	p.guild_id = 0;
	p.hair = hair_style;
	p.hair_color = hair_color;
	p.clothes_color = 0;
	p.inventory[0].nameid = start_weapon; // Knife
	p.inventory[0].amount = 1;
	p.inventory[0].equip = 0x02;
	p.inventory[0].identify = 1;
	p.inventory[1].nameid = start_armor; // Cotton Shirt
	p.inventory[1].amount = 1;
	p.inventory[1].equip = 0x10;
	p.inventory[1].identify = 1;
	p.weapon = 1;
	p.shield = 0;
	p.head_top = 0;
	p.head_mid = 0;
	p.head_bottom = 0;
	p.last_point = start_point;
	p.save_point = start_point;	
	
	// make new char.
	query.clear();
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_char) << "` "
			 "("
			 "`account_id`,"
			 "`slot`,"
			 "`name`,"
			 "`str`,`agi`,`vit`,`int`,`dex`,`luk`,"
			 "`hair`,"
			 "`hair_color`,"
			 "`zeny`,"
			 "`hp`,`sp`,`max_hp`,`max_sp`,"
			 "`last_map`,`last_x`,`last_y`,"
			 "`save_map`,`save_x`,`save_y`"
			 ") "
			 "VALUES "
			 "("
			 "'" << p.account_id << "',"
			 "'" << p.slot << "',"
			 "'" << dbcon1.escaped(p.name) << "',"
			 "'" << p.str << "','" << p.agi << "','" << p.vit << "',"
			 "'" << p.int_ << "','" << p.dex << "','" << p.luk << "',"
			 "'" << p.hair << "',"
			 "'" << p.hair_color << "',"
			 "'" << p.zeny << "',"
			 "'" << p.hp << "','" << p.sp << "',"
			 "'" << p.max_hp << "','" << p.max_sp << "',"
			 "'" << dbcon1.escaped(p.last_point.mapname) << "',"
			 "'" << p.last_point.x << "',"
			 "'" << p.last_point.y << "',"
			 "'" << dbcon1.escaped(p.save_point.mapname) << "',"
			 "'" << p.save_point.x << "',"
			 "'" << p.save_point.y << "'"
			 ")";
	dbcon1.PureQuery(query);

	//Now we need the charid from sql!
	p.char_id = dbcon1.getLastID();

	//Give the char the default items
	//knife & cotton shirts, add on as needed ifmore items are to be included.
	query.clear();
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_inventory) << "` "
			 "(`char_id`,`nameid`, `amount`, `equip`, `identify`) "
			 "VALUES "
			 "('" << p.char_id << "', '" << start_weapon << "', '1', '2', '1'),"
			 "('" << p.char_id << "', '" << start_armor  << "', '1', '16', '1')";

	dbcon1.PureQuery(query);

	return true;
}


bool CCharDB_sql::removeChar(uint32 charid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE `char_id`='" << charid << "'";
	dbcon1.PureQuery(query);
	return true;
}

bool CCharDB_sql::saveChar(const CCharCharacter& p)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i, doit;

	// Build the update for the character
	query.clear();
	query << "UPDATE `" << dbcon1.escaped(this->tbl_char) << "` "
			 "SET "
			 "`class` = '"		<< p.class_ 		<< "',"
			 "`base_level`='"	<< p.base_level		<< "',"
			 "`job_level`='"	<< p.job_level		<< "',"
			 "`base_exp`='"		<< p.base_exp		<< "',"
			 "`job_exp`='"		<< p.job_exp		<< "',"
			 "`zeny`='"			<< p.zeny			<< "',"

			 "`hp`='"			<< p.hp				<< "',"
			 "`max_hp`='"		<< p.max_hp			<< "',"

			 "`sp`='"			<< p.sp				<< "',"
			 "`max_sp`='"		<< p.max_sp			<< "',"

			 "`str`='"			<< p.str			<< "',"
			 "`agi`='"			<< p.agi			<< "',"
			 "`vit`='"			<< p.vit			<< "',"
			 "`int`='"			<< p.int_			<< "',"
			 "`dex`='"			<< p.dex			<< "',"
			 "`luk`='"			<< p.luk			<< "',"

			 "`status_point`='"	<< p.status_point	<< "',"
			 "`skill_point`='"	<< p.skill_point	<< "',"

			 "`option`='"		<< p.option			<< "',"
			 "`karma`='"		<< p.karma			<< "',"
			 "`chaos`='"		<< p.chaos			<< "',"
			 "`manner`='"		<< p.manner			<< "',"
			 "`party_id`='"		<< p.party_id		<< "',"
			 "`guild_id`='"		<< p.guild_id		<< "',"
			 "`pet_id`='"		<< p.pet_id			<< "',"

			 "`hair`='"			<< p.hair			<< "',"
			 "`hair_color`='"	<< p.hair_color		<< "',"
			 "`clothes_color`='"<< p.clothes_color	<< "',"
			 "`weapon`='"		<< p.weapon			<< "',"
			 "`shield`='"		<< p.shield			<< "',"
			 "`head_top`='"		<< p.head_top		<< "',"
			 "`head_mid`='"		<< p.head_mid		<< "',"
			 "`head_bottom`='"	<< p.head_bottom	<< "',"

			 "`last_map`='"		<< p.last_point.mapname		<< "',"
			 "`last_x`='"		<< p.last_point.x			<< "',"
			 "`last_y`='"		<< p.last_point.y			<< "',"
			 "`save_map`='"		<< p.save_point.mapname		<< "',"
			 "`save_x`='"		<< p.save_point.x			<< "',"
			 "`save_y`='"		<< p.save_point.y			<< "',"

			 "`partner_id`='"	<< p.partner_id		<< "',"
			 "`father_id`='"	<< p.father_id		<< "',"
			 "`mother_id`='"	<< p.mother_id		<< "',"
			 "`child_id`='"		<< p.child_id		<< "',"

			 "`fame_points`='"	<< p.fame_points	<< "' "	// dont forget to remove commas at ends
			 "WHERE  "
			 "`account_id`='" 	<< p.account_id		<< "' "
			 "AND "
			 "`char_id` = '"	<< p.char_id		<< "' "
			 "AND "
			 "`slot` = '" 		<< p.slot			<< "'";  // dont forget to finish the line


	dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////
	// Memo Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_memo) << "` "
			 "WHERE `char_id`='" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();


	//insert here.
	query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_memo) << "`"
			 "(`char_id`,`memo_id`,`map`,`x`,`y`) VALUES ";
	for(doit=0, i=0; i<MAX_MEMO; ++i)
	{
		if(p.memo_point[i].mapname[0])
		{
			query << (doit?",":"") << "("
				"'" << 	p.char_id 			<< "',"
				"'" <<	(ulong)i					<< "',"
				"'" <<	dbcon1.escaped(p.memo_point[i].mapname)	<< "'," <<
				"'" <<	p.memo_point[i].x	<< "'," <<
				"'" <<	p.memo_point[i].y	<< "'" <<  // Dont forget to end commas
			")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////
	// Inventory Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_inventory) << "` "
			"WHERE `char_id`='" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	//insert here.
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_inventory) << "`"
			 "(`char_id`, `nameid`, `amount`, `equip`, "
			 "`identify`, `refine`, `attribute`, "
			 "`card0`, `card1`, `card2`, `card3`) VALUES ";
	for(doit=0,i=0; i<MAX_INVENTORY; ++i)
	{
		if(p.inventory[i].nameid>0)
		{
			query << (doit?",":"") <<
				"("
				"'" <<	p.char_id					<< "',"
				"'" <<	p.inventory[i].nameid		<< "',"
				"'" <<	p.inventory[i].amount		<< "',"
				"'" <<	p.inventory[i].equip		<< "',"
				"'" <<	p.inventory[i].identify		<< "',"
				"'" <<	p.inventory[i].refine		<< "',"
				"'" <<	p.inventory[i].attribute	<< "',"
				"'" <<	p.inventory[i].card[0]		<< "',"
				"'" <<	p.inventory[i].card[1]		<< "',"
				"'" <<	p.inventory[i].card[2]		<< "',"
				"'" <<	p.inventory[i].card[3]		<< "'"
				")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////
	// Cart Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_cart) << "` "
			 "WHERE `char_id`='" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	//insert here.
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_cart) << "`"
			 "(`char_id`, `nameid`, `amount`, `equip`, "
			 "`identify`, `refine`, `attribute`, "
			 "`card0`, `card1`, `card2`, `card3`) VALUES ";
	for(doit=0,i=0; i<MAX_CART; ++i)
	{
		if(p.cart[i].nameid>0)
		{
			query << (doit?",":"") <<
				"("
				"'" <<	p.char_id				<< "',"
				"'" <<	p.cart[i].nameid		<< "',"
				"'" <<	p.cart[i].amount		<< "',"
				"'" <<	p.cart[i].equip			<< "',"
				"'" <<	p.cart[i].identify		<< "',"
				"'" <<	p.cart[i].refine		<< "',"
				"'" <<	p.cart[i].attribute		<< "',"
				"'" <<	p.cart[i].card[0]		<< "',"
				"'" <<	p.cart[i].card[1]		<< "',"
				"'" <<	p.cart[i].card[2]		<< "',"
				"'" <<	p.cart[i].card[3]		<< "'"
				")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();


	///////////////////////////////////////////////////////////////////////
	// Skill Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_skill) << "` "
			 "WHERE `char_id`='" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	//insert here.
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_skill) << "` "
			 "(`char_id`,`id`,`lv`) VALUES ";
	for(doit=0,i=0; i<MAX_SKILL; ++i)
	{
		if(p.skill[i].id==i && p.skill[i].lv>0 && p.skill[i].flag != 1)
		{
			query << (doit?",":"") <<
				"("
				"'" << p.char_id 		<< "'," <<
				"'" << p.skill[i].id 	<< "'," <<
				"'" << p.skill[i].lv 	<< "'" <<
				")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();

	///////////////////////////////////////////////////////////////////////
	// Character Reg Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_char_reg) << "` "
			 "WHERE `char_id`='" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	//insert here.
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_char_reg) << "`"
			 "(`char_id`,`str`,`value`) VALUES ";
	for(doit=0,i=0; i<p.global_reg_num && i<GLOBAL_REG_NUM; ++i)
	{
		if(p.global_reg[i].str && p.global_reg[i].value !=0)
		{
			query << (doit?",":"") <<
				"("
				"'" << p.char_id				<< "',"
				"'" << p.global_reg[i].str		<< "',"
				"'" << p.global_reg[i].value	<< "'" <<   // end commas at the end
				")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();



	///////////////////////////////////////////////////////////////////////
	// Friends Insert
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_friends) << "` "
			 "WHERE `char_id` = '" << p.char_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	//insert here.
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_friends) << "`"
			 "(`char_id`, `friend_id`) VALUES ";
	for(doit=0,i=0; i<MAX_FRIENDLIST; ++i)
	{
		if(p.friendlist[i].friend_id!=0)
		{
			query << (doit?",":"") <<
				"("
				"'" << p.char_id					<< "',"
				"'" << p.friendlist[i].friend_id	<< "'"
				")";
			++doit;
		}
	}
	// if at least one entry spotted.
	if(doit) dbcon1.PureQuery(query);
	query.clear();

	return true;
}
bool CCharDB_sql::searchAccount(uint32 accid, CCharCharAccount& account)
{	// read account data
	bool ret = false;
	if(accid)
	{
		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> query;
		size_t i;

		query << "SELECT "
				 "`account_id`,"	//  0
				 "`sex`,"			//  1
				 "`gm_level`,"		//  2
				 "`email`,"			//  3
				 "`login_id1`,"		//  4
				 "`login_id2`,"		//  5
				 "`client_ip`,"		//  6
				 "`ban_until`,"		//  7
				 "`valid_until` "	//  8
				 "FROM `" << dbcon1.escaped(this->tbl_account) << "` "
				 "WHERE `account_id`='" << accid << "'";

		if( dbcon1.ResultQuery(query) && dbcon1 )
		{	
			account.account_id	= atol(dbcon1[0]);
			account.sex			= dbcon1[1][0] == 'S' ? 2 : dbcon1[1][0]=='M';
			account.gm_level	= atol(dbcon1[2]);
			safestrcpy(account.email, sizeof(account.email), dbcon1[3]);
			account.login_id1	= atol(dbcon1[4]);
			account.login_id2	= atol(dbcon1[5]);
			account.client_ip	= basics::ipaddress(dbcon1[6]);
			account.ban_until	= (time_t)(atol(dbcon1[7]));
			account.valid_until	= (time_t)(atol(dbcon1[8]));

			// read accociated char_id's
			query.clear();
			query << "SELECT `char_id`,`slot` "
					 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
					 "WHERE `account_id` = '" << accid << "'";

			
			for(i=0; i<9; ++i) account.charlist[i]=0;

			if( dbcon1.ResultQuery(query) )
			{
				for( ; dbcon1; ++dbcon1)
				{
					i = atoi( dbcon1[1] );
					if( i<9 && dbcon1[0] && atoi(dbcon1[0]) )
					{
						if( account.charlist[i] != 0 )
							ShowError("CharDB: doubled used slot %i for account_id %i\n", i, accid );
						account.charlist[ atoi( dbcon1[1] ) ] = atoi( dbcon1[0] );
					}
				}
			}

			query.clear();
			query << "SELECT `str`,`value` "
					 "FROM `" << dbcon1.escaped(this->tbl_login_reg) << "` "
					 "WHERE `account_id`='" << account.account_id << "'";
			i=0;
			if( dbcon1.ResultQuery(query) )
			{
				for( ; dbcon1 && i<ACCOUNT_REG2_NUM; ++dbcon1, ++i)
				{
					safestrcpy(account.account_reg2[i].str, sizeof(account.account_reg2[0].str), dbcon1[0]);
					account.account_reg2[i].value = atoi(dbcon1[1]);
				}
			}
			account.account_reg2_num = i;
			for( ; i<ACCOUNT_REG2_NUM; ++i)
			{
				account.account_reg2[i].str[0] = 0;
				account.account_reg2[i].value = 0;
			}
			ret = true;
		}
	}
	return ret;
}



bool CCharDB_sql::saveAccount(CCharAccount& account)
{
	// save account data from login
	// only necessary if login-server database and char-server database
	// are physically seperated
	// otherwise the login-server already has saved the data into sql

	// asuming that only the simple data storage model will be used
	// this here can be skipped
	return true;
}

bool CCharDB_sql::removeAccount(uint32 accid)
{
	// remove account data
	// only necessary if login-server database and char-server database
	// are physically seperated

	// asuming that only the simple data storage model will be used
	// this here can be skipped
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// MAIL STUFF
size_t CCharDB_sql::getMailCount(uint32 cid, uint32 &all, uint32 &unread)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	all = 0;
	unread = 0;

	query << "SELECT count(*) "
			 "FROM `" << dbcon1.escaped(this->tbl_mail) << "` "
			 "WHERE `to_char_id` = '" << cid << "' "
			 "AND `read_flag` = '0'";

	if( dbcon1.ResultQuery(query) )
	{
		unread = atol(dbcon1[0]);
	}

	query.clear();
	query << "SELECT count(*) "
			 "FROM `" << dbcon1.escaped(this->tbl_mail) << "` "
			 "WHERE `to_char_id` = '" << cid << "' ";
	if( dbcon1.ResultQuery(query) )
	{
		all = atol(dbcon1[0]);
	}
	return all;
}

size_t CCharDB_sql::listMail(uint32 cid, unsigned char box, unsigned char *buffer)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT "
			 "`message_id`,`read_flag`,`from_char_name`,`sendtime`,`header`"
			 "FROM `" << dbcon1.escaped(this->tbl_mail) << "` "
			 "WHERE `to_char_id` = '" << cid << "'";

	if( dbcon1.ResultQuery(query) )
	{
		
		unsigned char *buf = buffer;
		size_t count;
		for(count=0; dbcon1; ++dbcon1, ++count)
		{
			CMailHead mailhead( atol(dbcon1[0]),
								atol(dbcon1[1]),
								dbcon1[2],
								atol(dbcon1[3]),
								dbcon1[4]);
			mailhead._tobuffer(buf); // automatic buffer increment
		}
		return count;
	}
	return 0;
}

bool CCharDB_sql::readMail(uint32 cid, uint32 mid, CMail& mail)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	bool ret = false;

	query << "SELECT "
			 "`read_flag`,`from_char_name`,`header`,`sendtime`,`message` "
			 "`zeny`,"
			 "`item_nameid`,`item_amount`,`item_equip`,`item_identify`,"
			 "`item_refine`,`item_attribute`,"
			 "`item_card0`,`item_card1`,`item_card2`,`item_card3` "
			 "FROM `" << dbcon1.escaped(this->tbl_mail) << "` "
			 "WHERE `to_char_id` = '" << cid << "' "
			 "AND `message_id` = '" << mid << "'";

	// default clearing
	mail.read    = 0;
	mail.name[0] = 0;
	mail.head[0] = 0;
	mail.body[0] = 0;

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		ret = true;

		struct item item;
		item.nameid		= atol(dbcon1[ 6]);
		item.amount		= atol(dbcon1[ 7]);
		item.equip  	= atol(dbcon1[ 8]);
		item.identify	= atol(dbcon1[ 9]);
		item.refine 	= atol(dbcon1[10]);
		item.attribute	= atol(dbcon1[11]);
		item.card[0]	= atol(dbcon1[12]);
		item.card[1]	= atol(dbcon1[13]);
		item.card[2]	= atol(dbcon1[14]);
		item.card[3]	= atol(dbcon1[15]);

		mail = CMail( mid, atol(dbcon1[0]), dbcon1[1], dbcon1[2], atol(dbcon1[3]),
						atol(dbcon1[5]), item, dbcon1[4]);
		if( 0==mail.read )
		{
			query.clear();
			query << "UPDATE `" << dbcon1.escaped(this->tbl_mail) << "` "
					 "SET `read_flag`='1',"
					 "`zeny`='0',"
					 "`item_nameid`='0',`item_amount`='0',`item_equip`='0',`item_identify`='0',"
					 "`item_refine`='0',`item_attribute`='0',"
					 "`item_card0`='0',`item_card1`='0',`item_card2`='0',`item_card3`='0' "
					 "WHERE `message_id`= '" << mid << "'";
			dbcon1.PureQuery(query);
		}
	}
	return ret;
}

bool CCharDB_sql::deleteMail(uint32 cid, uint32 mid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_mail) << "` "
			 "WHERE `to_char_id` = '" << cid << "' "
			 "AND `message_id` = '" << mid << "'";
	return dbcon1.PureQuery(query);
}

bool CCharDB_sql::sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32 zeny, const struct item& item, uint32& msgid, uint32& tid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	if( 0==strcmp(targetname,"*") )
	{	// send to all
		query << "SELECT DISTINCT "
				 "`char_id`,`name` "
				 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
				 "WHERE `char_id` <> '" << senderid << "' "
				 "ORDER BY `char_id`";
	}
	else
	{	// send to specific
		query << "SELECT "
				 "`char_id`,`name` "
				 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
				 "WHERE `name` = '" << dbcon1.escaped(targetname) << "'";
	}

	bool ret = dbcon1.ResultQuery(query);
	if( ret )
	{
		size_t doit = 0;

		basics::string<> _head = dbcon1.escaped(head);
		basics::string<> _body = dbcon1.escaped(body);
		basics::string<> _sendername = dbcon1.escaped(sendername);

		query.clear();
		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_mail) << "` "
				 "("
				 "`to_char_id`,`to_char_name`,"
				 "`from_char_id`,`from_char_name`,"
				 "`header`,`message`,`read_flag`"
				 "`zeny`,"
				 "`item_nameid`,`item_amount`,`item_equip`,`item_identify`,`item_refine`,`item_attribute`,"
				 "`item_card0`,`item_card1`,`item_card2`,`item_card3`"
				 ") "
				 "VALUES ";

		for( ; dbcon1; ++dbcon1)
		{
			query << (doit?",":"") <<
				"("
				"'" << dbcon1[0] << "','" << dbcon1.escaped(dbcon1[1]) << "',"
				"'" << senderid << "','" << _sendername << "',"				
				"'" << _head << "','" << _body << "','0',"
				"'" << zeny << "',"
				"'" << item.nameid << "','" << item.amount << "','" << /*item.equip*/0 << "',"
				"'" << item.identify << "','" << item.refine << "','" << item.attribute << "',"
				"'" << item.card[0] << "','" << item.card[1] << "','" << item.card[2] << "','" << item.card[3] << "'"
				")";
			++doit;
		}
		if(doit) ret = dbcon1.PureQuery(query);
	}
	return ret;
}


void CCharDB_sql::loadfamelist()
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	const static fame_t fametype[] = {FAME_PK, FAME_SMITH, FAME_CHEM, FAME_TEAK};
	const char* queryselect[4] = 
	{	// pk
		"(`s`.`str`='PC_PK_FAME')",
		// blacksmith
		"(`s`.`str`='PC_SMITH_FAME' AND (`c`.`class`='10' OR `c`.`class`='4011' OR `c`.`class`='4033'))",
		// alchemist
		"(`s`.`str`='PC_CHEM_FAME' AND (`c`.`class`='18' OR `c`.`class`='4019' OR `c`.`class`='4041'))",
		// teakwon
		"(`s`.`str`='PC_TEAK_FAME' AND (`c`.`class`='4046'))"
	};

	size_t i,k;
	for(i=0; i<4; ++i)
	{
		CFameList &fl = this->famelists[fametype[i]];
		fl.clear();
		query.clear();
		query << "SELECT "
				 "`s`.`char_id`,`c`.`name`,`s`.`value` "
				 "FROM `" << dbcon1.escaped(this->tbl_char_reg) << "` `s` "
				 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `c`.`char_id` = `s`.`char_id` "
				 "WHERE `s`.`value`>'0' AND " << queryselect[i] <<
				 "ORDER BY `s`.`value` LIMIT 0," << (MAX_FAMELIST+1);

		if( dbcon1.ResultQuery(query) )
		{
			for(k=0; dbcon1 && k<MAX_FAMELIST+1; ++dbcon1, ++k )
			{
				if( atol(dbcon1[0]) && dbcon1[1][0] && atol(dbcon1[2]) )
				{
					fl.cEntry[k] = 
						CFameList::fameentry(atol(dbcon1[0]), dbcon1[1], atol(dbcon1[2]));
				}
			}
			fl.cCount=k;
			// clear the rest
			for( ; k<MAX_FAMELIST+1; ++k )
			{
				fl.cEntry[k] = 
					CFameList::fameentry(0, "", 0);
			}
		}
	}
}



//******************************************************************************
//******************************************************************************
//*******     *****  ******  ***  ***  ***********       *********     *********
//****   ****  ****  ******  ***  ***  ***********  ***      **   ****  ********
//****  ***********  ******  ***  ***  ***********  *******  **   **************
//****  ***********  ******  ***  ***  ***********  *******  **   **************
//****  ****   ****  ******  ***  ***  ***********  *******  ***        ********
//****  *****  ****  ******  ***  ***  ***********  *******  **********  *******
//*****  ****  *****  ****  ****  ***  ***********  *******  **********  *******
//******      ******  ****  ****  ***          ***  ***      ***  *****  *******
//*******************      *****  ***          ***       *******        ********
//****************************************************************    **********
bool CGuildDB_sql::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);


	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	// check if all castles exist
	size_t i;
	for(i = 0; i < MAX_GUILDCASTLE; ++i, query.clear())
	{	
		query <<  "SELECT "
				  "count(*) "
				  "FROM `" << dbcon1.escaped(this->tbl_castle) << "` "
				  "WHERE `castle_id`='" << i << "'";
		if( !dbcon1.ResultQuery(query) || 0==atoi(dbcon1[0]) )
		{
			this->saveCastle( CCastle(i) ); // constructor takes care of all settings
		}
	}
	return true;
}

size_t CGuildDB_sql::size() const
{
	return this->get_table_size(this->tbl_guild);
}
CGuild& CGuildDB_sql::operator[](size_t i)
{	// not threadsafe
	static CGuild g;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`guild_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_guild) << "` "
			 "ORDER BY `guild_id`"
			 "LIMIT "<< i << ",1 ";

	if( dbcon1.ResultQuery(query) )
	{
		const uint32 guild_id = atol(dbcon1[0]);
		if( !this->searchGuild(guild_id,g) )
			g.guild_id=0;
	}
	else
	{
		g.guild_id=0;
	}
	return g;
}

size_t CGuildDB_sql::castlesize() const
{
	return this->get_table_size(this->tbl_castle);
}
CCastle& CGuildDB_sql::castle(size_t i)
{	// not threadsafe
	static CCastle c;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`castle_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_castle) << "` "
			 "ORDER BY `castle_id`"
			 "LIMIT "<< i << ",1 ";

	if( dbcon1.ResultQuery(query) )
	{
		const uint32 castle_id = atol(dbcon1[0]);
		if( !this->searchCastle(castle_id,c) )
			c.castle_id=0;
	}
	else
	{
		c.castle_id=0;
	}
	return c;
}

bool CGuildDB_sql::searchGuild(const char* name, CGuild& g)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`guild_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_guild) << "` "
			 "WHERE `name` = '" << dbcon1.escaped(name) << "'";

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		const uint32 guild_id = atol(dbcon1[0]);
		return this->searchGuild(guild_id,g);
	}
	return false;
}

bool CGuildDB_sql::searchGuild(uint32 guild_id, CGuild& g)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	query << "SELECT "
			 "`g`.`guild_id`, `g`.`guild_lv`, "
			 "`g`.`connect_member`, `g`.`max_member`, `g`.`average_lv`,"
			 "`g`.`exp`, `g`.`next_exp`, `g`.`skill_point`, "
			 "`g`.`name`, `c`.`name`, `g`.`mes1`, `g`.`mes2`,"
			 "`g`.`emblem_id`, `g`.`emblem_len`, `g`.`emblem_data` "
			 "FROM `" << dbcon1.escaped(this->tbl_guild) << "` `g`"
			 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `c`.`char_id` = `g`.`master_id`"
			 "WHERE `g`.`guild_id` = " << guild_id;

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		///////////////////////////////////////////////////////////////////////
		g.guild_id		= atoi(dbcon1[0]);
		g.guild_lv		= atoi(dbcon1[1]);
		g.connect_member= atoi(dbcon1[2]);
		g.max_member	= atoi(dbcon1[3]);
		g.average_lv	= atoi(dbcon1[4]);
		g.exp			= atoi(dbcon1[5]);
		g.next_exp		= atoi(dbcon1[6]);
		g.skill_point	= atoi(dbcon1[7]);
		safestrcpy(g.name, sizeof(g.name), dbcon1[8]);
		safestrcpy(g.master, sizeof(g.name), dbcon1[9]);
		safestrcpy(g.mes1, sizeof(g.mes1), dbcon1[10]);
		safestrcpy(g.mes2, sizeof(g.mes2), dbcon1[11]);
		g.emblem_id		= atoi(dbcon1[12]);
		g.emblem_len	= atoi(dbcon1[13]);
		g.chaos = 0;
		g.honour = 0;
		g.save_flags = 0;
		g.save_timer = 0;

		if(g.max_member>MAX_GUILD) g.max_member = MAX_GUILD;

		///////////////////////////////////////////////////////////////////////
		{	// Decode the emblem from SQL
			const uchar *ptr = (uchar*)dbcon1[14];
			uchar val1, val2;
			if(ptr)
			{
				i=0;
				while( i<g.emblem_len && i<sizeof(g.emblem_data) && ptr[0] && ptr[1] )
				{
					if( ptr[0] >= '0' && ptr[0] <= '9')
						val1 = ptr[0] - '0';
					else if(ptr[0] >= 'a' && ptr[0] <= 'f')
						val1 = ptr[0] - 'a' + 10;
					else if(ptr[0] >= 'A' && ptr[0] <= 'F')
						val1 = ptr[0] - 'A' + 10;
					else
						break;
					if(ptr[1] >= '0' && ptr[1] <= '9')
						val2 = ptr[1] - '0';
					else if(ptr[1] >= 'a' && ptr[1] <= 'f')
						val2 = ptr[1] - 'a' + 10;
					else if(ptr[1] >= 'A' && ptr[1] <= 'F')
						val2 = ptr[1] - 'A' + 10;
					else
						break;
					g.emblem_data[i++] = (val1<<4) | val2;
					ptr+=2;
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Get the guild's members
		query.clear();
		query << "SELECT "
				 "`c`.`account_id`,"
				 "`g`.`char_id`,"
				 "`c`.`hair`,"
				 "`c`.`hair_color`,"
				 "`a`.`sex`,"
				 "`c`.`class`,"
				 "`c`.`base_level`,"
				 "`g`.`exp`,"
				 "`g`.`exp_payper`,"
				 "`a`.`online`,"
				 "`g`.`position`,"
				 "`g`.`rsv1`,"
				 "`g`.`rsv2`,"
				 "`c`.`name`"
				 "FROM `" << dbcon1.escaped(this->tbl_guild_member) << "` `g` "
				 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `g`.`char_id`=`c`.`char_id` "
				 "JOIN `" << dbcon1.escaped(this->tbl_account) << "` `a` ON `c`.`account_id`=`a`.`account_id` "
				 "WHERE `g`.`guild_id` = " << g.guild_id << " "
				 "ORDER BY `g`.`position` ASC, `c`.`class`, `c`.`base_level` DESC, `a`.`sex`";

		dbcon1.ResultQuery(query);
		for(i=0; dbcon1 && i<MAX_GUILD; ++dbcon1, ++i)
		{
			g.member[i].account_id = atoi(dbcon1[0]);
			g.member[i].char_id = atoi(dbcon1[1]);
			g.member[i].hair = atoi(dbcon1[2]);
			g.member[i].hair_color = atoi(dbcon1[3]);
			g.member[i].gender = atoi(dbcon1[4]);
			g.member[i].class_ = atoi(dbcon1[5]);
			g.member[i].lv = atoi(dbcon1[6]);
			g.member[i].exp = atoi(dbcon1[7]);
			g.member[i].exp_payper = atoi(dbcon1[8]);
			g.member[i].online = atoi(dbcon1[9]);
			g.member[i].position = atoi(dbcon1[10]);
			g.member[i].rsv1 = atoi(dbcon1[11]);
			g.member[i].rsv2 = atoi(dbcon1[12]);
			safestrcpy(g.member[i].name, sizeof(g.member[i].name), dbcon1[13]);
		}
		for( ; i<MAX_GUILD; ++i)
		{
			g.member[i].account_id = 0;
			g.member[i].char_id = 0;
			g.member[i].hair = 0;
			g.member[i].hair_color = 0;
			g.member[i].gender = 0;
			g.member[i].class_ = 0;
			g.member[i].lv = 0;
			g.member[i].exp = 0;
			g.member[i].exp_payper = 0;
			g.member[i].online = 0;
			g.member[i].position = 0;
			g.member[i].rsv1 = 0;
			g.member[i].rsv2 = 0;
			g.member[i].name[0] = 0;
		}



		///////////////////////////////////////////////////////////////////////
		// Get the guild's positions
		query.clear();
		query << "SELECT "
				 "`name`, `mode`, `exp_mode` "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_position) << "` "
				 "WHERE `guild_id` = '" << g.guild_id << "' "
				 "ORDER BY `position` ASC";
		dbcon1.ResultQuery(query);

		// Fills in the guild positions array for this guild from sql.
		// SQL may not provide data for all of the positions, so we
		// get as much as we can from the DB and fill that in.
		for(i=0; dbcon1 && i<MAX_GUILDPOSITION; ++dbcon1, ++i)
		{
			safestrcpy(g.position[i].name, sizeof(g.position[i].name), dbcon1[0]);
			g.position[i].mode = atoi(dbcon1[1]);
			g.position[i].exp_mode = atoi(dbcon1[2]);
		}
		// If the guild positions array isn't full, we generate names
		for (; i < MAX_GUILDPOSITION; ++i)
		{
			basics::string<> position_name;
			position_name << "Position " << i+1;
			safestrcpy(g.position[i].name, sizeof(g.position[i].name), position_name);
			g.position[i].mode = 0;
			g.position[i].exp_mode = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Get the guild's aliances
		query.clear();
		query << "SELECT "
				 "`a`.`opposition`, `a`.`alliance_id`, `g`.`name` "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_alliance) << "` `a`"
				 "JOIN `" << dbcon1.escaped(this->tbl_guild) << "` `g` ON `a`.`alliance_id`=`g`.`guild_id` "
				 "WHERE `a`.`guild_id` = " << g.guild_id;
		dbcon1.ResultQuery(query);
		for(i=0; dbcon1 && i<MAX_GUILDALLIANCE; ++dbcon1, ++i)
		{
			g.alliance[i].opposition = atoi(dbcon1[0]);
			g.alliance[i].guild_id = atoi(dbcon1[1]);
			safestrcpy(g.alliance[i].name, sizeof(g.alliance[i].name), dbcon1[2]);
		}
		for( ; i<MAX_GUILDALLIANCE; ++i)
		{
			g.alliance[i].opposition = 0;
			g.alliance[i].guild_id = 0;
			g.alliance[i].name[0] = 0;
		}
		///////////////////////////////////////////////////////////////////////
		// Get the guild's history of expulsions
		query.clear();
		query << "SELECT "
				 "`c`.`name`, `e`.`mes`, `e`.`acc`, `c`.`account_id`, `e`.`rsv1`, `e`.`rsv2`, `e`.`rsv3` "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_expulsion) << "` `e`"
				 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `e`.`char_id`=`c`.`char_id` "
				 "WHERE `e`.`guild_id` = " << g.guild_id;
		dbcon1.ResultQuery(query);
		for(i=0; dbcon1 && i<MAX_GUILDEXPLUSION; ++dbcon1, ++i)
		{
			safestrcpy(g.explusion[i].name, sizeof(g.explusion[i].name), dbcon1[0]);
			safestrcpy(g.explusion[i].mes, sizeof(g.explusion[i].mes), dbcon1[1]);
			safestrcpy(g.explusion[i].acc, sizeof(g.explusion[i].acc), dbcon1[2]);
			g.explusion[i].account_id = atoi(dbcon1[3]);
			g.explusion[i].char_id = atoi(dbcon1[4]);
			g.explusion[i].rsv1 = atoi(dbcon1[5]);
			g.explusion[i].rsv2 = atoi(dbcon1[6]);
			g.explusion[i].rsv3 = atoi(dbcon1[7]);
		}
		for( ; i<MAX_GUILDEXPLUSION; ++i)
		{
			g.explusion[i].name[0] = 0;
			g.explusion[i].mes[0] = 0;
			g.explusion[i].acc[0] = 0;
			g.explusion[i].account_id = 0;
			g.explusion[i].char_id = 0;
			g.explusion[i].rsv1 = 0;
			g.explusion[i].rsv2 = 0;
			g.explusion[i].rsv3 = 0;
		}

		///////////////////////////////////////////////////////////////////////
		// Get the guild's skills
		query.clear();
		query << "SELECT "
				 "`id`, `lv` "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_skill) << "` "
				 "WHERE `guild_id` = " << g.guild_id;
		dbcon1.ResultQuery(query);
		for(i=0; i<MAX_GUILDSKILL; ++i)
		{
			g.skill[i].id = i+GD_SKILLBASE;
			g.skill[i].lv = 0;
		}
		for( ; dbcon1; ++dbcon1)
		{
			i = atol(dbcon1[0])-GD_SKILLBASE;
			if(i<MAX_GUILDSKILL)
				g.skill[i].lv = atol(dbcon1[1]);
		}
		return true;
	}
	return false;
}



bool CGuildDB_sql::insertGuild(const struct guild_member &m, const char *name, CGuild &g)
{
	size_t i;
	// construct initial guild
	g = CGuild(name);
	g.member[0] = m;
	safestrcpy(g.master, sizeof(g.master), m.name);
	g.position[0].mode=0x11;
	safestrcpy(g.position[0].name, sizeof(g.position[0].name),"GuildMaster");
	safestrcpy(g.position[MAX_GUILDPOSITION-1].name, sizeof(g.position[0].name),"Newbie");
	for(i=1; i<MAX_GUILDPOSITION-1; ++i)
		snprintf(g.position[i].name,sizeof(g.position[0].name),"Position %ld",(unsigned long)(i+1));

	g.max_member=(16>MAX_GUILD)?MAX_GUILD:16;
	g.average_lv=g.member[0].lv;
	for(i=0;i<MAX_GUILDSKILL;++i)
	{
		g.skill[i].id = i+GD_SKILLBASE;
		g.skill[i].lv = 0;
	}

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_guild) << "` "
			 "(`name`,`master_id`) "
			 "VALUES "
			 "("
			 "'" << dbcon1.escaped(name)<< "',"
			 "'" << g.member[0].char_id	<< "'"	
			 ")";
	if( dbcon1.PureQuery(query) )
	{
		g.guild_id = dbcon1.getLastID();
		// Save the rest of the guild now that we have the basics inserted
		return this->saveGuild(g) && searchGuild(g.guild_id, g);
	}
	return false;
}

bool CGuildDB_sql::removeGuild(uint32 guild_id)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "UPDATE `" << dbcon1.escaped(this->tbl_char) << "` "
			 "SET `guild_id`='0' "
			 "WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);
	query.clear();

	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_member) << "` "
			 "WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_skill) << "` "
			"WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_storage) << "` "
			"WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_position) << "` "
			"WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_alliance) << "` "
			"WHERE `guild_id` = '" << guild_id << "' "
			"OR `alliance_id` = '" << guild_id << "' ";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_expulsion) << "` "
			"WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild) << "` "
			"WHERE `guild_id` = '" << guild_id << "'";
	dbcon1.PureQuery(query);

	return true;
}

bool CGuildDB_sql::saveGuild(const CGuild& g)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	basics::string<> query2;
	uint doit;
	size_t i;

	if(g.save_flags&GUILD_SAFE_GUILD)
	{
		char emblem_data[4096], *ptr=emblem_data;
		const char *eptr=emblem_data+sizeof(emblem_data)-2;

		for(i=0;i<g.emblem_len && ptr<eptr;++i)
		{
			const uchar v1 = (g.emblem_data[i]     ) & 0x0f;
			const uchar v2 = (g.emblem_data[i] >> 4) & 0x0f;
			*ptr++ = ( v2<10 ) ? char(v2+'0') : char(v2-10+'a');
			*ptr++ = ( v1<10 ) ? char(v1+'0') : char(v1-10+'a');
		}
		*ptr = '\0';

		query << "UPDATE `" << dbcon1.escaped(this->tbl_guild) << "` "
				 "SET"
				 "`guild_lv`='" 		<< g.guild_lv		<< "',"
				 "`connect_member`='"	<< g.connect_member	<< "',"
				 "`max_member`='"		<< g.max_member		<< "',"
				 "`average_lv`='"		<< g.average_lv		<< "',"
				 "`exp`='"				<< g.exp 			<< "',"
				 "`next_exp`='"			<< g.next_exp		<< "',"
				 "`skill_point`='"		<< g.skill_point	<< "',"
				 "`mes1`='"				<< dbcon1.escaped(g.mes1) << "',"
				 "`mes2`='" 			<< dbcon1.escaped(g.mes2) << "',"
				 "`emblem_len`='"		<< g.emblem_len		<< "',"
				 "`emblem_id`='"		<< g.emblem_id		<< "',"
				 "`emblem_data`='"		<< emblem_data			<< "'"
				 "WHERE `guild_id`='"	<< g.guild_id 		<< "'";

		dbcon1.PureQuery(query);
	}

	if(g.save_flags&GUILD_SAFE_MEMBER)
	{
		if(g.save_flags&GUILD_CLEAR_MEMBER)
		{
			query.clear();
			query << "DELETE "
					 "FROM `" << dbcon1.escaped(this->tbl_guild_member) << "` "
					 "WHERE `guild_id` = '" << g.guild_id << "'";
			dbcon1.PureQuery(query);

			// Remove guild IDs' from the character information sheets
			query.clear();
			query << "UPDATE `" << dbcon1.escaped(this->tbl_char) << "` "
					 "SET `guild_id` = '0' "
					 "WHERE `guild_id` = '" << g.guild_id << "'";
			dbcon1.PureQuery(query);

		}

		// Now reinsert all the data
		// Begin building long insert
		query.clear();
		query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_guild_member) << "` "
				 "("
				 "`guild_id`,`char_id`,`exp`,`exp_payper`,`position`,`rsv1`,`rsv2`"
				 ") "
				 "VALUES ";

		query2.clear();
		query2 << "UPDATE `" << dbcon1.escaped(this->tbl_char) << "` "
				  "SET `guild_id` = '" << g.guild_id << "' "
				  "WHERE `char_id` IN ( ";

		for(i=0, doit=0; i<g.max_member && i<MAX_GUILD; ++i)
		{
			if(g.member[i].account_id > 0)
			{
				query << (doit?",":"") <<
					"("
					"'" << g.guild_id 				<< "',"
					"'" << g.member[i].char_id		<< "',"
					"'" << g.member[i].exp			<< "',"
					"'" << g.member[i].exp_payper	<< "',"
					"'" << g.member[i].position		<< "',"
					"'" << g.member[i].rsv1			<< "',"
					"'" << g.member[i].rsv2			<< "'"
					")";

				query2 << (doit?",'":"'") << g.member[i].char_id << "'";
				++doit;
			}
		}
		if(doit)
		{
			dbcon1.PureQuery(query);
			query2 << ")";	// -> WHERE <field> IN ( <list> )
			dbcon1.PureQuery(query2);
		}
	}

	if(g.save_flags&GUILD_SAFE_POSITION)
	{
		query.clear();
		query << "REPLACE "
				 "INTO `" << dbcon1.escaped(this->tbl_guild_position) << "` "
				 "(`guild_id`,`position`,`name`,`mode`,`exp_mode`) VALUES ";
		for(i=0, doit=0; i<MAX_GUILDPOSITION; ++i)
		{
			query << (doit?",":"") <<
				"("
				"'" << g.guild_id				<< "',"
				"'" << i							<< "',"
				"'" << dbcon1.escaped(g.position[i].name)<< "',"
				"'" << g.position[i].mode		<< "',"
				"'" << g.position[i].exp_mode	<< "'"
				")";
			++doit;
		}
		if(doit) dbcon1.PureQuery(query);
	}

	if(g.save_flags&GUILD_SAFE_ALLIANCE)
	{
		query.clear();
		query << "DELETE "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_alliance) << "` "
				 "WHERE '" << g.guild_id << "' IN (`alliance_id`,`guild_id`)";
		dbcon1.PureQuery(query);


		query.clear();
		query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_guild_alliance) << "` "
				 "(`guild_id`,`alliance_id`,`opposition`) VALUES ";
		for(i=0, doit=0;i<MAX_GUILDALLIANCE;++i)
		{
			if(g.alliance[i].guild_id>0)
			{
				query << (doit?",":"") <<
					"(" // Guild alliance for the current guild
					"'" << g.guild_id				<< "',"
					"'" << g.alliance[i].opposition	<< "',"
					"'" << g.alliance[i].guild_id	<< "',"
					")"
					"(" // Guild alliance for the other guild
					"'" << g.alliance[i].guild_id	<< "',"
					"'" << g.alliance[i].opposition	<< "',"
					"'" << g.guild_id				<< "',"
					")";
				++doit;
			}
		}
		if(doit) dbcon1.PureQuery(query);
	}

	if(g.save_flags&GUILD_SAFE_EXPULSE)
	{
		query.clear();
		query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_guild_expulsion) << "` "
				 "(`guild_id`,`char_id`,`mes`,`acc`,`rsv1`,`rsv2`,`rsv3`) "
				 "VALUES ";

		for(i=0, doit=0;i<MAX_GUILDEXPLUSION;++i)
		{
			if(g.explusion[i].account_id>0)
			{
				query << (doit?",":"") <<
					"("
					"'" << g.guild_id					<< "',"
					"'" << g.explusion[i].char_id		<< "',"
					"'" << dbcon1.escaped(g.explusion[i].mes)<< "',"
					"'" << g.explusion[i].acc			<< "',"
					"'" << g.explusion[i].rsv1			<< "',"
					"'" << g.explusion[i].rsv2			<< "',"
					"'" << g.explusion[i].rsv3			<< "'"
					")";
				++doit;
			}
		}
		if(doit) dbcon1.PureQuery(query);

	}

	if(g.save_flags&GUILD_SAFE_SKILL)
	{
		query.clear();
		query << "DELETE "
				 "FROM `" << dbcon1.escaped(this->tbl_guild_skill) << "` "
				 "WHERE `guild_id` = '" << g.guild_id << "'";
		dbcon1.PureQuery(query);

		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_guild_skill) << "` "
				 "(`guild_id`,`id`,`lv`) VALUES ";

		for(i=0, doit=0;i<MAX_GUILDSKILL;++i)
		{
			if(g.skill[i].lv>0)
			{
				query << (doit?",":"") <<
					"("
					"'" << g.guild_id		<< "',"
					"'" << g.skill[i].id	<< "',"
					"'" << g.skill[i].lv	<< "'"
					")";
				++doit;
			}
		}
		if(doit) dbcon1.PureQuery(query);
	}
	const_cast<CGuild&>(g).save_flags = 0;
	return true;
}

//////
// Grabs information about a castle from the sql database, and writes the
// newly-fetched information to 'castle'.
//////
bool CGuildDB_sql::searchCastle(ushort castle_id, CCastle& castle)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;
	query << "SELECT "
			 "`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, "
			 "`nextTime`, `payTime`, `createTime`, `visibleC`"
			 " FROM `" << dbcon1.escaped(this->tbl_castle) << "` "
			 "WHERE `castle_id` = " << castle_id;

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		castle.castle_id = atoi(dbcon1[0]);
		castle.guild_id = atoi(dbcon1[1]);
		castle.economy = atoi(dbcon1[2]);
		castle.defense = atoi(dbcon1[3]);
		castle.triggerE = atoi(dbcon1[4]);
		castle.triggerD = atoi(dbcon1[5]);
		castle.nextTime = atoi(dbcon1[6]);
		castle.payTime = atoi(dbcon1[7]);
		castle.createTime = atoi(dbcon1[8]);
		castle.visibleC = atoi(dbcon1[9]);

		query.clear();
		query << "SELECT "
				 "`guardian_hp`, `guardian_visible` "
				 "FROM `" << dbcon1.escaped(this->tbl_castle_guardian) << "` "
				 "WHERE `castle_id` = " << castle_id;

		dbcon1.ResultQuery(query);
		for(i=0; dbcon1 && i<MAX_GUARDIAN; ++dbcon1, ++i)
		{
			castle.guardian[i].guardian_id = 0;
			castle.guardian[i].guardian_hp = atoi(dbcon1[0]);
			castle.guardian[i].visible = atoi(dbcon1[1]);
		}
		for( ; i<MAX_GUARDIAN; ++i)
		{
			castle.guardian[i].guardian_id = 0;
			castle.guardian[i].guardian_hp = 0;
			castle.guardian[i].visible = 0;
		}
	}
	return true;
}
bool CGuildDB_sql::saveCastle(const CCastle& castle)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	// create/update the castle's information
	query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_castle) << "` "
			 "("
			 "`castle_id`,`guild_id`,`economy`,`defense`,`triggerE`,`triggerD`,"
			 "`nextTime`,`payTime`,`createTime`,`visibleC`"
			 ") "
			 "VALUES "
			 "("
			 "'" << castle.castle_id	<< "',"
			 "'" << castle.guild_id		<< "',"
			 "'" << castle.economy		<< "',"
			 "'" << castle.defense		<< "',"
			 "'" << castle.triggerE		<< "',"
			 "'" << castle.triggerD		<< "',"
			 "'" << castle.nextTime		<< "',"
			 "'" << castle.payTime		<< "',"
			 "'" << castle.createTime	<< "',"
			 "'" << castle.visibleC		<< "'"
			 ") ";

	dbcon1.PureQuery(query);

	// Clear the guardians
	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_castle_guardian) << "` "
			 "WHERE castle_id = " << castle.castle_id;
	dbcon1.PureQuery(query);

	// Update the guardians
	for (i=0; i<MAX_GUARDIAN; ++i)
	{
		if( castle.guardian[i].guardian_id )
		{
			query.clear();
			query << "REPLACE INTO `" << dbcon1.escaped(this->tbl_castle_guardian) << "` "
					 "(castle_id, guardian_id, guardian_hp, guardian_visible) "
					 "VALUES "
					 "(" << 
					 castle.castle_id << ", " << 
					 castle.guardian[i].guardian_id << ", " << 
					 castle.guardian[i].guardian_hp << ", " << 
					 castle.guardian[i].visible << 
					 ")";
			dbcon1.PureQuery(query);
		}
	}
	return true;
}
bool CGuildDB_sql::removeCastle(ushort castle_id)
{	// Delete from this->tbl_castle where castle_id = *cid
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_castle_guardian) << "` "
			 "WHERE castle_id = " << castle_id;
	dbcon1.PureQuery(query);

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_castle) << "` "
			 "WHERE castle_id = '" << castle_id << "'";

	dbcon1.PureQuery(query);
	return true;
}

bool CGuildDB_sql::getCastles(basics::vector<CCastle>& castlevector)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	CCastle tmp;

	query << "SELECT "
			 "castle_id"
			 " FROM `" << dbcon1.escaped(this->tbl_castle) << "` ";
	castlevector.clear();
	for( dbcon1.ResultQuery(query); dbcon1; ++dbcon1)
	{
		if( this->searchCastle( atoi(dbcon1[0]), tmp) )
			castlevector.push(tmp);
	}
	return true;
}
uint32 CGuildDB_sql::has_conflict(uint32 guild_id, uint32 account_id, uint32 char_id)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	// check guild's members
	query << "SELECT `guild_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_member) << "` "
			 "WHERE `char_id` = " << char_id << " "
			 "AND `guild_id` != " << guild_id;

	if( dbcon1.ResultQuery(query) && 0!=atoi(dbcon1[0]) && guild_id!=(uint32)atoi(dbcon1[0]) )
	{	// entry in members
		return atoi(dbcon1[0]);
	}

	// check char
	query.clear();
	query << "SELECT `guild_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
			 "WHERE `account_id` = " << account_id << " "
			 "AND `char_id` = " << char_id;
	if( dbcon1.ResultQuery(query) && 0!=atoi(dbcon1[0]) && guild_id!=(uint32)atoi(dbcon1[0]) )
	{
		return atoi(dbcon1[0]);
	}
	return 0;
}

//////////
// Party
///////
bool CPartyDB_sql::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return true;
}
size_t CPartyDB_sql::size() const
{
	return this->get_table_size(this->tbl_party);
}

CParty& CPartyDB_sql::operator[](size_t i)
{	// not threadsafe
	static CParty p;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	
	query << "SELECT `party_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_party) << "` "
			 "ORDER BY `party_id`"
			 "LIMIT "<< i << ",1 ";

	if( !dbcon1.ResultQuery(query) || !searchParty( atol(dbcon1[0]), p) )
			p.party_id = 0;
	return p;
}
bool CPartyDB_sql::searchParty(const char* name, CParty& p)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	
	query << "SELECT `party_id` "
			 "FROM `" << dbcon1.escaped(this->tbl_party) << "` "
			 "WHERE `name` = '" << dbcon1.escaped(name) << "'";

	if( dbcon1.ResultQuery(query) )
	{
		return searchParty( atol(dbcon1[0]), p);
	}
	return false;
}


bool CPartyDB_sql::searchParty(uint32 pid, CParty& p)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;
	bool found, ret = false;
	query << "SELECT "
			 "`party_id`, `name`, `expshare`, `itemshare`, `itemc`, `leader` "
			 "FROM `" << dbcon1.escaped(this->tbl_party) << "` "
			 "WHERE `party_id` = '" << pid << "'";

	if( dbcon1.ResultQuery(query) )
	{
		p.party_id = atol( dbcon1[0] );
		safestrcpy(p.name, sizeof(p.name), dbcon1[1]);
		p.expshare = atol( dbcon1[2] );
		p.itemshare= atol( dbcon1[3] );
		p.itemc    = atol( dbcon1[4] );
		basics::string<> leader = atol( dbcon1[5] );

		query.clear();
		query << "SELECT "
				 "`account_id`, `name`, `base_level` "
				 "FROM `" << dbcon1.escaped(this->tbl_char) << "` "
				 "WHERE `party_id` = '" << p.party_id << "'";
		dbcon1.ResultQuery(query);


		for(i=0, found=false; dbcon1 && i<MAX_PARTY; ++dbcon1, ++i)
		{
			struct party_member &m = p.member[i];
			m.account_id = atol( dbcon1[0] );
			safestrcpy(m.name, sizeof(m.name), dbcon1[1]);
			m.lv = atol( dbcon1[2] );

			found |= m.leader = (leader==m.name);
		}
		if(i==0)
		{	// party is empty but still there
			this->removeParty(pid);
		}
		else
		{
			if(!found)
			{	// first member takes over the party
				p.member[0].leader=1;
			}
			ret = true;
		}
		// clear the other members
		for( ; i<MAX_PARTY; ++i)
		{
			struct party_member &m = p.member[i];
			m.account_id = 0;
			m.name[0] = 0;
			m.lv = 0;

			found |= m.leader = (leader==m.name);
		}
	}
	return ret;
}

bool CPartyDB_sql::insertParty(uint32 accid, const char* nick, const char* mapname, ushort lv, const char* name, CParty& p)
{	// insert into party values (*party)
	if( !searchParty(name,p) )
	{	// not in the database, better create the entry and insert into the database
		safestrcpy(p.name,sizeof(p.name),name);
		p.expshare = 0;
		p.itemshare= 0;
		p.itemc    = 0;
		p.member[0].account_id = accid;
		safestrcpy( p.member[0].name, sizeof(p.member[0].name), nick );
		safestrcpy( p.member[0].mapname, sizeof(p.member[0].mapname), mapname );
		p.member[0].leader = 1;
		p.member[0].online = 1;
		p.member[0].lv = lv;

		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> query;

		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_party) << "` "
				 "(`name`, `leader`) VALUES "
				 "('" << dbcon1.escaped(name) << "','" << dbcon1.escaped(nick) << "')";
		bool ret = dbcon1.PureQuery(query);
		// now we get the ID from the last inserted INSERT statement 
		//(returns the last ID for this client, other clients wont affect this)
		p.party_id = dbcon1.getLastID();
		return ret;
	}
	return false;
}
bool CPartyDB_sql::removeParty(uint32 pid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "UPDATE `" << dbcon1.escaped(this->tbl_char) << "` "
			 "SET `party_id`='0' "
			 "WHERE `party_id` = '" << pid << "'";
	dbcon1.PureQuery(query);
	
	query.clear();
	query << "DELETE FROM `" << dbcon1.escaped(this->tbl_party) << "` "
			 "WHERE `party_id` = '" << pid << "'";
	return dbcon1.PureQuery(query);
}

bool CPartyDB_sql::saveParty(const CParty& p)
{
	size_t i;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	// check for existing leader, or just set one
	for(i=0; i<MAX_PARTY; ++i)
		if(p.member[i].leader)
			break;
	if(i>=MAX_PARTY)
		i=0;

	query << "UPDATE `" << dbcon1.escaped(this->tbl_party) << "` "
			 "SET "
			 "`leader` ='"     << dbcon1.escaped(p.member[i].name) << "',"
			 "`expshare` = '"  << p.expshare << "',"
			 "`itemshare` = '" << p.itemshare << "',"
			 "`itemc` = '"     << p.itemc << "' "
			 "WHERE `party_id` = '" << p.party_id << "'";
	dbcon1.PureQuery(query);
	return true;
}


////////////
// Account Storage
/////////
bool CPCStorageDB_sql::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return true;
}

size_t CPCStorageDB_sql::size() const
{
	return this->get_table_size(this->tbl_storage);
}

CPCStorage& CPCStorageDB_sql::operator[](size_t i)
{	// not threadsafe
	static CPCStorage stor;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	stor.account_id = 0;

	query << "SELECT "
			 "`account_id`"
			 "FROM `" << dbcon1.escaped(this->tbl_account) << "` "
			 "ORDER BY `account_id`"
			 "LIMIT "<< i << ",1 ";
	if( dbcon1.ResultQuery(query) )
	{
		this->searchStorage(atol(dbcon1[0]), stor);
	}
	return stor;
}


bool CPCStorageDB_sql::searchStorage(uint32 accid, CPCStorage& stor)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	query << "SELECT "
			 "`nameid`, `amount`, `equip`, `identify`, "
			 "`refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
			 "FROM `" << dbcon1.escaped(this->tbl_storage) << "` "
			 "WHERE `account_id` = '" << accid << "'";
	if( dbcon1.ResultQuery(query) )
	{
		stor.account_id = accid;

		for(i=0; dbcon1 && i<MAX_STORAGE; ++dbcon1, ++i)
		{
			stor.storage[i].nameid		= atol( dbcon1[0] );
			stor.storage[i].amount		= atol( dbcon1[1] );
			stor.storage[i].equip		= atol( dbcon1[2] );
			stor.storage[i].identify	= atol( dbcon1[3] );
			stor.storage[i].refine		= atol( dbcon1[4] );
			stor.storage[i].attribute	= atol( dbcon1[5] );
			stor.storage[i].card[0]		= atol( dbcon1[6] );
			stor.storage[i].card[1]		= atol( dbcon1[7] );
			stor.storage[i].card[2]		= atol( dbcon1[8] );
			stor.storage[i].card[3]		= atol( dbcon1[9] );
		}
		stor.storage_amount = i;
		for( ; i<MAX_STORAGE; ++i)
		{
			stor.storage[i].nameid		= 0;
			stor.storage[i].amount		= 0;
			stor.storage[i].equip		= 0;
			stor.storage[i].identify	= 0;
			stor.storage[i].refine		= 0;
			stor.storage[i].attribute	= 0;
			stor.storage[i].card[0]		= 0;
			stor.storage[i].card[1]		= 0;
			stor.storage[i].card[2]		= 0;
			stor.storage[i].card[3]		= 0;
		}
		return true;
	}
	return false;
}

bool CPCStorageDB_sql::removeStorage(uint32 accid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_storage) << "` "
			 "WHERE `account_id`='" << accid << "'";
	return dbcon1.PureQuery( query );
}

bool CPCStorageDB_sql::saveStorage(const CPCStorage& stor)
{
	this->removeStorage(stor.account_id);

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i, doit;

	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_storage) << "`"
			 "(`account_id`, `nameid`, `amount`, `equip`, `identify`, "
			 "`refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";

	for(i=0,doit=0; i<MAX_STORAGE; ++i)
	{
		if(stor.storage[i].nameid>0)
		{
			query << (doit?",":"") <<
				"("
				"'" <<	stor.account_id				<< "',"
				"'" <<	stor.storage[i].nameid		<< "',"
				"'" <<	stor.storage[i].amount		<< "',"
				"'" <<	stor.storage[i].equip		<< "',"
				"'" <<	stor.storage[i].identify	<< "',"
				"'" <<	stor.storage[i].refine		<< "',"
				"'" <<	stor.storage[i].attribute	<< "',"
				"'" <<	stor.storage[i].card[0]		<< "',"
				"'" <<	stor.storage[i].card[1]		<< "',"
				"'" <<	stor.storage[i].card[2]		<< "',"
				"'" <<	stor.storage[i].card[3]		<< "'"
				")";
			++doit;
		}
	}
	if(doit) dbcon1.PureQuery(query);
	return true;
}

///////////
// Guild storage
///////

bool CGuildStorageDB_sql::init(const char* configfile)
{       // init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return true;
}

size_t CGuildStorageDB_sql::size() const
{
	return this->get_table_size(this->tbl_guild_storage);
}

CGuildStorage& CGuildStorageDB_sql::operator[](size_t i)
{	// not threadsafe
	static CGuildStorage stor;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	stor.guild_id = 0;

	query << "SELECT "
			 "`guild_id`"
			 "FROM `" << dbcon1.escaped(this->tbl_guild) << "` "
			 "ORDER BY `guild_id`"
			 "LIMIT "<< i << ",1 ";
	if( dbcon1.ResultQuery(query) )
	{
		searchStorage(atol(dbcon1[0]), stor);
	}
	return stor;
}

bool CGuildStorageDB_sql::searchStorage(uint32 gid, CGuildStorage& stor)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i;

	query << "SELECT "
			 "`nameid`, `amount`, `equip`, `identify`, "
			 "`refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_storage) << "` "
			 "WHERE `guild_id` = '" << gid << "'";

	if( dbcon1.ResultQuery(query) )
	{
		stor.guild_id = gid;

		for(i=0; dbcon1 && i<MAX_GUILD_STORAGE; ++dbcon1, ++i)
		{
			stor.storage[i].nameid		= atol( dbcon1[0] );
			stor.storage[i].amount		= atol( dbcon1[1] );
			stor.storage[i].equip		= atol( dbcon1[2] );
			stor.storage[i].identify	= atol( dbcon1[3] );
			stor.storage[i].refine		= atol( dbcon1[4] );
			stor.storage[i].attribute	= atol( dbcon1[5] );
			stor.storage[i].card[0]		= atol( dbcon1[6] );
			stor.storage[i].card[1]		= atol( dbcon1[7] );
			stor.storage[i].card[2]		= atol( dbcon1[8] );
			stor.storage[i].card[3]		= atol( dbcon1[9] );
		}
		for( ; i<MAX_GUILD_STORAGE; ++i)
		{
			stor.storage[i].nameid		= 0;
			stor.storage[i].amount		= 0;
			stor.storage[i].equip		= 0;
			stor.storage[i].identify	= 0;
			stor.storage[i].refine		= 0;
			stor.storage[i].attribute	= 0;
			stor.storage[i].card[0]		= 0;
			stor.storage[i].card[1]		= 0;
			stor.storage[i].card[2]		= 0;
			stor.storage[i].card[3]		= 0;
		}
		return true;
	}
	return false;
}
bool CGuildStorageDB_sql::removeStorage(uint32 gid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_guild_storage) << "` "
			 "WHERE `guild_id`='" << gid << "'";
	return dbcon1.PureQuery( query );
}
bool CGuildStorageDB_sql::saveStorage(const CGuildStorage& stor)
{
	this->removeStorage(stor.guild_id);

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	size_t i, doit;
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_guild_storage) << "`"
			 "(`guild_id`, `nameid`, `amount`, `equip`, `identify`, "
			 "`refine`, `attribute`, `card0`, `card1`, `card2`, `card3`) VALUES ";

	for(i=0,doit=0; i<MAX_STORAGE; ++i)
	{
		if(stor.storage[i].nameid>0)
		{
			query << (doit?",":"") <<
				"("
				"'" <<	stor.guild_id				<< "',"
				"'" <<	stor.storage[i].nameid		<< "',"
				"'" <<	stor.storage[i].amount		<< "',"
				"'" <<	stor.storage[i].equip		<< "',"
				"'" <<	stor.storage[i].identify	<< "',"
				"'" <<	stor.storage[i].refine		<< "',"
				"'" <<	stor.storage[i].attribute	<< "',"
				"'" <<	stor.storage[i].card[0]		<< "',"
				"'" <<	stor.storage[i].card[1]		<< "',"
				"'" <<	stor.storage[i].card[2]		<< "',"
				"'" <<	stor.storage[i].card[3]		<< "'"
				")";
			++doit;
		}
	}
	if(doit) dbcon1.PureQuery(query);
	return true;
}


////////
// Pets
////
bool CPetDB_sql::init(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
	return true;
}

size_t CPetDB_sql::size() const
{
	return this->get_table_size(this->tbl_pet);
}

CPet& CPetDB_sql::operator[](size_t i)
{	// not threadsafe
	static CPet pet;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT "
			 "`p`.`pet_id`,`c`.`account_id`,`p`.`char_id`,`p`.`class`,`p`.`level`,"
			 "`p`.`egg_id`,`p`.`equip_id`,`p`.`intimate`,`p`.`hungry`,`p`.`name`,"
			 "`p`.`rename_flag`,`p`.`incuvate` "
			 "FROM `" << dbcon1.escaped(this->tbl_pet) << "` `p`"
			 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `p`.`char_id`=`c`.`char_id` "
			 "ORDER BY `pet_id`"
			 "LIMIT "<< i << ",1 ";

	if( dbcon1.ResultQuery(query) )
	{
		pet.pet_id = atoi(dbcon1[0]);
		pet.account_id = atoi(dbcon1[1]);
		pet.char_id = atoi(dbcon1[2]);
		pet.class_ = atoi(dbcon1[3]);
		pet.level = atoi(dbcon1[4]);
		pet.egg_id = atoi(dbcon1[5]);
		pet.equip_id = atoi(dbcon1[6]);
		pet.intimate = atoi(dbcon1[7]);
		pet.hungry = atoi(dbcon1[8]);
		safestrcpy(pet.name, sizeof(pet.name), dbcon1[9]);
		pet.rename_flag = atoi(dbcon1[10]);
		pet.incuvate = atoi(dbcon1[11]);
	}
	else
	{
		pet.pet_id = 0;
	}

	return pet;
}

bool CPetDB_sql::searchPet(uint32 pid, CPet& pet)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT "
			 "`p`.`pet_id`,`c`.`account_id`,`p`.`char_id`,`p`.`class`,`p`.`level`,"
			 "`p`.`egg_id`,`p`.`equip_id`,`p`.`intimate`,`p`.`hungry`,`p`.`name`,"
			 "`p`.`rename_flag`,`p`.`incuvate` "
			 "FROM `" << dbcon1.escaped(this->tbl_pet) << "` `p` "
			 "JOIN `" << dbcon1.escaped(this->tbl_char) << "` `c` ON `p`.`char_id`=`c`.`char_id` "
			 "WHERE `p`.`pet_id` = '" << pid << "'";
	if( dbcon1.ResultQuery(query) )
	{
		pet.pet_id = atoi(dbcon1[0]);
		pet.account_id = atoi(dbcon1[1]);
		pet.char_id = atoi(dbcon1[2]);
		pet.class_ = atoi(dbcon1[3]);
		pet.level = atoi(dbcon1[4]);
		pet.egg_id = atoi(dbcon1[5]);
		pet.equip_id = atoi(dbcon1[6]);
		pet.intimate = atoi(dbcon1[7]);
		pet.hungry = atoi(dbcon1[8]);
		safestrcpy(pet.name, sizeof(pet.name), dbcon1[9]);
		pet.rename_flag = atoi(dbcon1[10]);
		pet.incuvate = atoi(dbcon1[11]);
		return true;
	}
	return false;
}

bool CPetDB_sql::insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pd)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;


	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_pet) << "` "
			 "("
			 "`char_id`,"
			 "`class`,"
			 "`level`,"
			 "`egg_id`,"
			 "`equip_id`,"
			 "`intimate`,"
			 "`hungry`,"
			 "`name`,"
			 "`rename_flag`,"
			 "`incuvate`"
			 ") "
			 "VALUES "
			 "(" 
			 "'" << cid << "',"
			 "'" << pet_class << "',"
			 "'" << pet_lv << "',"
			 "'" << pet_egg_id << "',"
			 "'" << pet_equip << "',"
			 "'" << intimate << "',"
			 "'" << hungry << "',"
			 "'" << dbcon1.escaped(pet_name) << "',"
			 "'" << ((int)renameflag) << "',"
			 "'" << ((int)incuvat) << "'"
			 ")";

	if( dbcon1.PureQuery(query) )
	{
		pd.pet_id = dbcon1.getLastID();
		pd.account_id = accid;
		pd.char_id = cid;
		pd.class_ = pet_class;
		pd.level = pet_lv;
		pd.egg_id = pet_egg_id;
		pd.equip_id = pet_equip;
		pd.intimate = intimate;
		pd.hungry = hungry;
		safestrcpy(pd.name, sizeof(pd.name), pet_name);
		pd.rename_flag = renameflag;
		pd.incuvate = incuvat;
		return true;
	}
	return false;
}

bool CPetDB_sql::removePet(uint32 pid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_pet) << "` "
			 "WHERE `pet_id` = '" << pid <<"'";
	return dbcon1.PureQuery( query );
}

bool CPetDB_sql::savePet(const CPet& pet)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "UPDATE `" << dbcon1.escaped(this->tbl_pet) << "` "
			 "SET "
			 "`char_id` = '" << pet.char_id << "',"
			 "`class` = '" << pet.class_ << "',"
			 "`level` = '" << pet.level << "',"
			 "`egg_id` = '" << pet.egg_id << "',"
			 "`equip_id` = '" << pet.equip_id << "',"
			 "`intimate` = '" << pet.intimate << "',"
			 "`hungry` = '" << pet.hungry << "',"
			 "`name` = '" << dbcon1.escaped(pet.name) << "',"
			 "`rename_flag` = '" << pet.rename_flag << "',"
			 "`incuvate` = '" << pet.incuvate << "'"
			 "WHERE `pet_id` = '" << pet.pet_id << "'";
	
	return dbcon1.PureQuery( query );
}




////////
// Homunculus
////
bool CHomunculusDB_sql::init(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
	return true;
}

size_t CHomunculusDB_sql::size() const
{
	return this->get_table_size(this->tbl_homunculus);
}

CHomunculus& CHomunculusDB_sql::operator[](size_t i)
{	// not threadsafe
	static CHomunculus hom;
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT "
			 "`homun_id`,"
			 "`account_id`,"
			 "`char_id`,"
			 "`base_exp`,"
			 "`name`,"
			 "`hp`,"
			 "`max_hp`,"
			 "`sp`,"
			 "`max_sp`,"
			 "`class`,"
			 "`status_point`,"
			 "`skill_point`,"
			 "`str`,"
			 "`agi`,"
			 "`vit`,"
			 "`int`,"
			 "`dex`,"
			 "`luk`,"
			 "`option`,"
			 "`equip`,"
			 "`intimate`,"
			 "`hungry`,"
			 "`base_level`,"
			 "`rename_flag`,"
			 "`incubate` "
			 "FROM `" << dbcon1.escaped(this->tbl_homunculus) << "` "
			 "ORDER BY `homun_id`"
			 "LIMIT "<< i << ",1 ";
	if( dbcon1.ResultQuery(query) )
	{
		hom.homun_id		= atoi(dbcon1[ 0]);
		hom.account_id		= atoi(dbcon1[ 1]);
		hom.char_id			= atoi(dbcon1[ 2]);
		hom.base_exp		= atoi(dbcon1[ 3]);
		safestrcpy(hom.name, sizeof(hom.name), dbcon1[ 4]);
		hom.hp				= atoi(dbcon1[ 5]);
		hom.max_hp			= atoi(dbcon1[ 6]);
		hom.sp				= atoi(dbcon1[ 7]);
		hom.max_sp			= atoi(dbcon1[ 8]);
		hom.class_			= atoi(dbcon1[ 9]);
		hom.status_point	= atoi(dbcon1[10]);
		hom.skill_point		= atoi(dbcon1[11]);
		hom.str				= atoi(dbcon1[12]);
		hom.agi				= atoi(dbcon1[13]);
		hom.vit				= atoi(dbcon1[14]);
		hom.int_			= atoi(dbcon1[15]);
		hom.dex				= atoi(dbcon1[16]);
		hom.luk				= atoi(dbcon1[17]);
		hom.option			= atoi(dbcon1[18]);
		hom.equip			= atoi(dbcon1[19]);
		hom.intimate		= atoi(dbcon1[20]);
		hom.hungry			= atoi(dbcon1[21]);
		hom.base_level		= atoi(dbcon1[22]);
		hom.rename_flag		= atoi(dbcon1[23]);
		hom.incubate		= atoi(dbcon1[24]);
		

		///////////////////////////////////////////////////////////////////////
		// Get the skills
		query.clear();
		query << "SELECT "
				 "`id`, `lv` "
				 "FROM `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "WHERE `homun_id` = " << hom.homun_id;
		dbcon1.ResultQuery(query);
		size_t i;
		for(i=0; i<MAX_HOMSKILL; ++i)
		{
			hom.skill[i].id = i+HOM_SKILLID;
			hom.skill[i].lv = 0;
		}
		for( ; dbcon1; ++dbcon1)
		{
			i = atol(dbcon1[0])-HOM_SKILLID;
			if(i<MAX_HOMSKILL)
				hom.skill[i].lv = atol(dbcon1[1]);
		}
	}
	return hom;
}

bool CHomunculusDB_sql::searchHomunculus(uint32 hid, CHomunculus& hom)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "SELECT "
			 "`homun_id`,"
			 "`account_id`,"
			 "`char_id`,"
			 "`base_exp`,"
			 "`name`,"
			 "`hp`,"
			 "`max_hp`,"
			 "`sp`,"
			 "`max_sp`,"
			 "`class`,"
			 "`status_point`,"
			 "`skill_point`,"
			 "`str`,"
			 "`agi`,"
			 "`vit`,"
			 "`int`,"
			 "`dex`,"
			 "`luk`,"
			 "`option`,"
			 "`equip`,"
			 "`intimate`,"
			 "`hungry`,"
			 "`base_level`,"
			 "`rename_flag`,"
			 "`incubate` "
			 "FROM `" << dbcon1.escaped(this->tbl_homunculus) << "` "
			 "WHERE `homun_id` = '" << hid << "'";
	if( dbcon1.ResultQuery(query) )
	{
		hom.homun_id		= atoi(dbcon1[ 0]);
		hom.account_id		= atoi(dbcon1[ 1]);
		hom.char_id			= atoi(dbcon1[ 2]);
		hom.base_exp		= atoi(dbcon1[ 3]);
		safestrcpy(hom.name, sizeof(hom.name), dbcon1[ 4]);
		hom.hp				= atoi(dbcon1[ 5]);
		hom.max_hp			= atoi(dbcon1[ 6]);
		hom.sp				= atoi(dbcon1[ 7]);
		hom.max_sp			= atoi(dbcon1[ 8]);
		hom.class_			= atoi(dbcon1[ 9]);
		hom.status_point	= atoi(dbcon1[10]);
		hom.skill_point		= atoi(dbcon1[11]);
		hom.str				= atoi(dbcon1[12]);
		hom.agi				= atoi(dbcon1[13]);
		hom.vit				= atoi(dbcon1[14]);
		hom.int_			= atoi(dbcon1[15]);
		hom.dex				= atoi(dbcon1[16]);
		hom.luk				= atoi(dbcon1[17]);
		hom.option			= atoi(dbcon1[18]);
		hom.equip			= atoi(dbcon1[19]);
		hom.intimate		= atoi(dbcon1[20]);
		hom.hungry			= atoi(dbcon1[21]);
		hom.base_level		= atoi(dbcon1[22]);
		hom.rename_flag		= atoi(dbcon1[23]);
		hom.incubate		= atoi(dbcon1[24]);
		

		///////////////////////////////////////////////////////////////////////
		// Get the skills
		query.clear();
		query << "SELECT "
				 "`id`, `lv` "
				 "FROM `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "WHERE `homun_id` = " << hom.homun_id;
		dbcon1.ResultQuery(query);
		size_t i;
		for(i=0; i<MAX_HOMSKILL; ++i)
		{
			hom.skill[i].id = i+HOM_SKILLID;
			hom.skill[i].lv = 0;
		}
		for( ; dbcon1; ++dbcon1)
		{
			i = atol(dbcon1[0])-HOM_SKILLID;
			if(i<MAX_HOMSKILL)
				hom.skill[i].lv = atol(dbcon1[1]);
		}
		return true;
	}
	return false;
}

bool CHomunculusDB_sql::insertHomunculus(CHomunculus& hom)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_homunculus) << "` "
			 "("
			 "`account_id`,"
			 "`char_id`,"
			 "`base_exp`,"
			 "`name`,"
			 "`hp`,"
			 "`max_hp`,"
			 "`sp`,"
			 "`max_sp`,"
			 "`class`,"
			 "`status_point`,"
			 "`skill_point`,"
			 "`str`,"
			 "`agi`,"
			 "`vit`,"
			 "`int`,"
			 "`dex`,"
			 "`luk`,"
			 "`option`,"
			 "`equip`,"
			 "`intimate`,"
			 "`hungry`,"
			 "`base_level`,"
			 "`rename_flag`,"
			 "`incubate`"
			 ") "
			 "VALUES "
			 "(" 
			 "'" << hom.account_id << "',"
			 "'" << hom.char_id << "',"
			 "'" << hom.base_exp << "',"
			 "'" << dbcon1.escaped(hom.name) << "',"
			 "'" << hom.hp << "',"
			 "'" << hom.max_hp << "',"
			 "'" << hom.sp << "',"
			 "'" << hom.max_sp << "',"
			 "'" << hom.class_ << "',"
			 "'" << hom.status_point << "',"
			 "'" << hom.skill_point << "',"
			 "'" << hom.str << "',"
			 "'" << hom.agi << "',"
			 "'" << hom.vit << "',"
			 "'" << hom.int_ << "',"
			 "'" << hom.dex << "',"
			 "'" << hom.luk << "',"
			 "'" << hom.option << "',"
			 "'" << hom.equip << "',"
			 "'" << hom.intimate << "',"
			 "'" << hom.hungry << "',"
			 "'" << hom.base_level << "',"			 
			 "'" << ((int)hom.rename_flag) << "',"
			 "'" << ((int)hom.incubate) << "'"
			 ")";
	if( dbcon1.PureQuery(query) )
	{
		hom.homun_id = dbcon1.getLastID();

		query.clear();
		query << "DELETE "
				 "FROM `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "WHERE `homun_id` = '" << hom.homun_id <<"'";
		dbcon1.PureQuery(query);


		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "(`homun_id`,`id`,`lv`) VALUES ";

		size_t i, doit;
		for(i=0,doit=0; i<MAX_HOMSKILL; ++i)
		{
			if(hom.skill[i].lv>0)
			{
				query << (doit?",":"") <<
					"("
					"'" << hom.homun_id		<< "',"
					"'" << hom.skill[i].id	<< "',"
					"'" << hom.skill[i].lv	<< "'"
					")";
				++doit;
			}
		}
		if(doit) dbcon1.PureQuery(query);
		return true;
	}
	return false;
}

bool CHomunculusDB_sql::removeHomunculus(uint32 hid)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_homunskill) << "` "
			 "WHERE `homun_id` = '" << hid <<"'";
	const bool ret = dbcon1.PureQuery( query );

	query.clear();
	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_homunculus) << "` "
			 "WHERE `homun_id` = '" << hid <<"'";

	return ret && dbcon1.PureQuery( query );
}

bool CHomunculusDB_sql::saveHomunculus(const CHomunculus& hom)
{
	if( hom.homun_id==0 )
	{	// insert will create an unique id for the object
		return this->insertHomunculus( const_cast<CHomunculus&>(hom) );
	}
	else
	{
		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> query;

		query << "UPDATE `" << dbcon1.escaped(this->tbl_homunculus) << "` "
				 "SET "
				 "`homun_id` = '"		<< hom.homun_id << "',"
				 "`account_id` = '"		<< hom.account_id << "',"
				 "`char_id` = '"		<< hom.char_id << "',"

				 "`base_exp` = '"		<< hom.base_exp << "',"
				 "`name` = '"			<< hom.name << "',"

				 "`hp` = '"				<< hom.hp << "',"
				 "`max_hp` = '"			<< hom.max_hp << "',"
				 "`sp` = '"				<< hom.sp << "',"
				 "`max_sp` = '"			<< hom.max_sp << "',"

				 "`class` = '"			<< hom.class_ << "',"
				 "`status_point` = '"	<< hom.status_point << "',"
				 "`skill_point` = '"	<< hom.skill_point << "',"

				 "`str` = '"			<< hom.str << "',"
				 "`agi` = '"			<< hom.agi << "',"
				 "`vit` = '"			<< hom.vit << "',"
				 "`int` = '"			<< hom.int_ << "',"
				 "`dex` = '"			<< hom.dex << "',"
				 "`luk` = '"			<< hom.luk << "',"

				 "`option` = '"			<< hom.option << "',"
				 "`equip` = '"			<< hom.equip << "',"

				 "`intimate` = '"		<< hom.intimate << "',"
				 "`hungry` = '"			<< hom.hungry << "',"
				 "`equip` = '"			<< hom.equip << "',"
				 "`base_level` = '"		<< hom.base_level << "',"
				 
				 "`rename_flag` = '"	<< hom.rename_flag << "',"
				 "`incubate` = '"		<< hom.incubate << "' "

				 "WHERE `homun_id` = '" << hom.homun_id << "'";
		
		bool ret = dbcon1.PureQuery( query );

		query.clear();
		query << "DELETE "
				 "FROM `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "WHERE `homun_id` = '" << hom.homun_id <<"'";
		dbcon1.PureQuery(query);


		query << "INSERT INTO `" << dbcon1.escaped(this->tbl_homunskill) << "` "
				 "(`homun_id`,`id`,`lv`) VALUES ";

		size_t i, doit;
		for(i=0,doit=0; i<MAX_HOMSKILL; ++i)
		{
			if(hom.skill[i].lv>0)
			{
				query << (doit?",":"") <<
					"("
					"'" << hom.homun_id		<< "',"
					"'" << hom.skill[i].id	<< "',"
					"'" << hom.skill[i].lv	<< "'"
					")";
				++doit;
			}
		}
		if(doit) ret &= dbcon1.PureQuery(query);
		return ret;
	}
}







////////
// Variables
////
bool CVarDB_sql::init(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
	return true;
}

size_t CVarDB_sql::size() const
{
	return this->get_table_size(this->tbl_variable);
}
CVar& CVarDB_sql::operator[](size_t i)
{	// not threadsafe
	static CVar var;

	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`name`,"
			 "`stortype`,"
			 "`storid`,"
			 "`vartype`,"
			 "`value`"
			 "FROM `" << dbcon1.escaped(this->tbl_variable) << "` "
			 "ORDER BY `name`"
			 "LIMIT "<< i << ",1 ";

	if( dbcon1.ResultQuery(query) && dbcon1 )
	{
		var = CVar(
				dbcon1[0],		// name
				//dbcon1[1],	// stortype
				//dbcon1[2],	// storid
				//dbcon1[3],	// vartype
				dbcon1[4] );	// value
	}
	else
		var = CVar("","");
	return var;
}

bool CVarDB_sql::searchVar(const char* name, CVar& var)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "SELECT "
			 "`name`,"
			 "`stortype`,"
			 "`storid`,"
			 "`vartype`,"
			 "`value`"
			 "FROM `" << dbcon1.escaped(this->tbl_variable) << "` "
			 "WHERE `name` = " << dbcon1.escaped(var.name());
	dbcon1.ResultQuery(query);
	if(dbcon1)
	{
		var = CVar(
				dbcon1[0],		// name
				//dbcon1[1],	// stortype
				//dbcon1[2],	// storid
				//dbcon1[3],	// vartype
				dbcon1[4] );	// value
		return true;
	}
	return false;
}
bool CVarDB_sql::insertVar(const char* name, const char* value)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;
	query << "INSERT INTO `" << dbcon1.escaped(this->tbl_variable) << "` "
			 "("
			 "`name`,"
			 "`stortype`,"
			 "`storid`,"
			 "`vartype`,"
			 "`value`"
			 ") "
			 "VALUES "
			 "(" 
			 "'" << dbcon1.escaped(name) << "',"
			 "'" << 0 << "',"
			 "'" << 0 << "',"
			 "'" << 0 << "',"
			 "'" << dbcon1.escaped(value) << "'"
			 ")";
	return dbcon1.PureQuery( query );
}
bool CVarDB_sql::removeVar(const char* name)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "DELETE "
			 "FROM `" << dbcon1.escaped(this->tbl_variable) << "` "
			 "WHERE `name` = '" << dbcon1.escaped(name) <<"'";
	return dbcon1.PureQuery( query );
}
bool CVarDB_sql::saveVar(const CVar& var)
{
	basics::CMySQLConnection dbcon1(this->sqlbase);
	basics::string<> query;

	query << "UPDATE `" << dbcon1.escaped(this->tbl_variable) << "` "
			 "SET "
			 "`name` = '"		<< dbcon1.escaped(var.name()) << "',"
			 "`stortype` = '"	<< 0 << "',"
			 "`storid` = '"		<< 0 << "',"
			 "`vartype` = '"	<< 0 << "',"
			 "`value` = '"		<< dbcon1.escaped(var.value()) << "',"

			 "WHERE `name` = '" << dbcon1.escaped(var.name()) << "'";
	return dbcon1.PureQuery( query );
}


#endif//WITH_MYSQL


