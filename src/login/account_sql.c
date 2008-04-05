#include "../common/malloc.h"
#include "../common/sql.h"
#include "../common/timer.h"
#include "account.h"

/// imports from outside
extern uint16 login_server_port;
extern char login_server_ip[32];
extern char login_server_id[32];
extern char login_server_pw[32];
extern char login_server_db[32];
extern char default_codepage[32];

/// internal structure
typedef struct AccountDB_SQL
{
	AccountDB vtable;    // public interface
	Sql* accounts;       // SQL accounts storage
	int keepalive_timer; // ping timer id

} AccountDB_SQL;

/// internal functions
static bool account_db_sql_init(AccountDB* self);
static bool account_db_sql_free(AccountDB* self);
static bool account_db_sql_create(AccountDB* self, const struct mmo_account* acc);
static bool account_db_sql_remove(AccountDB* self, const int account_id);
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc);
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const int account_id);
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid);

static int sql_ping_init(AccountDB_SQL* db);
static int login_sql_ping(int tid, unsigned int tick, int id, int data);

/// public constructor
AccountDB* account_db_sql(void)
{
	AccountDB_SQL* db = (AccountDB_SQL*)aCalloc(1, sizeof(AccountDB_SQL));

	db->vtable.init     = &account_db_sql_init;
	db->vtable.free     = &account_db_sql_free;
	db->vtable.save     = &account_db_sql_save;
	db->vtable.create   = &account_db_sql_create;
	db->vtable.remove   = &account_db_sql_remove;
	db->vtable.load_num = &account_db_sql_load_num;
	db->vtable.load_str = &account_db_sql_load_str;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


/// establishes database connection
static bool account_db_sql_init(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle;

//	mmo_auth_sqldb_init();
	db->accounts = Sql_Malloc();
	sql_handle = db->accounts;

	// DB connection start
	if( SQL_ERROR == Sql_Connect(sql_handle, login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(db->accounts);
		db->accounts = NULL;
		return false;
	}

	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
		Sql_ShowDebug(sql_handle);

//	if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s` (`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '0', 'lserver','100','login server started')", loginlog_db) )
//		Sql_ShowDebug(sql_handle);

	sql_ping_init(db);

	return true;
}

static bool account_db_sql_free(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

//	mmo_db_close();

	return true;
}

static bool account_db_sql_create(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
/*
	stmt = SqlStmt_Malloc(sql_handle);
	SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`, `expiration_time`) VALUES (?, ?, '%c', 'a@a.com', '%d')", login_db, login_db_userid, login_db_user_pass, account->sex, expiration_time);
	SqlStmt_BindParam(stmt, 0, SQLDT_STRING, account->userid, strnlen(account->userid, NAME_LENGTH));
	if( login_config.use_md5_passwds )
	{
		MD5_String(account->pass, md5buf);
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, md5buf, 32);
	}
	else
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, account->pass, strnlen(account->pass, NAME_LENGTH));
	SqlStmt_Execute(stmt);

*/
	return true;
}

static bool account_db_sql_remove(AccountDB* self, const int account_id)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	return true;
}

static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

/*
				//Delete all global account variables....
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`='1' AND `account_id`='%d';", reg_db, account_id) )
					Sql_ShowDebug(sql_handle);

				//Proceed to insert them....
				stmt = SqlStmt_Malloc(sql_handle);
				if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , ? , ?);",  reg_db, account_id) )
					SqlStmt_ShowDebug(stmt);
				for( i = 0, off = 13; i < ACCOUNT_REG2_NUM && off < RFIFOW(fd,2); ++i )
				{
					char* p;
					size_t len;

					// str
					p = (char*)RFIFOP(fd,off);
					len = strlen(p);
					SqlStmt_BindParam(stmt, 0, SQLDT_STRING, p, len);
					off += len + 1;

					// value
					p = (char*)RFIFOP(fd,off);
					len = strlen(p);
					SqlStmt_BindParam(stmt, 1, SQLDT_STRING, p, len);
					off += len + 1;

					if( SQL_ERROR == SqlStmt_Execute(stmt) )
						SqlStmt_ShowDebug(stmt);
				}
				SqlStmt_Free(stmt);
*/
	return true;
}

static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const int account_id)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	return true;
}

static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
/*
	// retrieve login entry for the specified username
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `%s`,`%s`,`lastlogin`,`sex`,`expiration_time`,`unban_time`,`state`,`%s` FROM `%s` WHERE `%s`= %s '%s'",
		login_db_account_id, login_db_user_pass, login_db_level,
		login_db, login_db_userid, (login_config.case_sensitive ? "BINARY" : ""), esc_userid) )
		Sql_ShowDebug(sql_handle);

	if( Sql_NumRows(sql_handle) == 0 ) // no such entry
	{
		ShowNotice("auth failed: no such account '%s'\n", esc_userid);
		Sql_FreeResult(sql_handle);
		return 0;
	}


	Sql_NextRow(sql_handle); //TODO: error checking?

	Sql_GetData(sql_handle, 0, &data, NULL); sd->account_id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, &len); safestrncpy(password, data, sizeof(password));
	Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(sd->lastlogin, data, sizeof(sd->lastlogin));
	Sql_GetData(sql_handle, 3, &data, NULL); sd->sex = *data;
	Sql_GetData(sql_handle, 4, &data, NULL); expiration_time = atol(data);
	Sql_GetData(sql_handle, 5, &data, NULL); unban_time = atol(data);
	Sql_GetData(sql_handle, 6, &data, NULL); state = atoi(data);
	Sql_GetData(sql_handle, 7, &data, NULL); sd->level = atoi(data);
	if( len > sizeof(password) - 1 )
		ShowDebug("mmo_auth: password buffer is too small (len=%u,buflen=%u)\n", len, sizeof(password));
	if( sd->level > 99 )
		sd->level = 99;

	Sql_FreeResult(sql_handle);



			while( SQL_SUCCESS == Sql_NextRow(sql_handle) && off < 9000 )
			{
				char* data;
				
				// str
				Sql_GetData(sql_handle, 0, &data, NULL);
				if( *data != '\0' )
				{
					off += sprintf((char*)WFIFOP(fd,off), "%s", data)+1; //We add 1 to consider the '\0' in place.
					
					// value
					Sql_GetData(sql_handle, 1, &data, NULL);
					off += sprintf((char*)WFIFOP(fd,off), "%s", data)+1;
				}
			}
			Sql_FreeResult(sql_handle);

*/

	return true;
}



static int sql_ping_init(AccountDB_SQL* db)
{
	uint32 connection_timeout, connection_ping_interval;
	Sql* sql_handle = db->accounts;

	// set a default value first
	connection_timeout = 28800; // 8 hours

	// ask the mysql server for the timeout value
	if( SQL_SUCCESS == Sql_GetTimeout(sql_handle, &connection_timeout) && connection_timeout < 60 )
		connection_timeout = 60;

	// establish keepalive
	connection_ping_interval = connection_timeout - 30; // 30-second reserve
	add_timer_func_list(login_sql_ping, "login_sql_ping");
	db->keepalive_timer = add_timer_interval(gettick() + connection_ping_interval*1000, login_sql_ping, 0, (int)sql_handle, connection_ping_interval*1000);

	return 0;
}

static int login_sql_ping(int tid, unsigned int tick, int id, int data)
{
	Sql* sql_handle = (Sql*)data;

	Sql_Ping(sql_handle);
	return 0;
}
