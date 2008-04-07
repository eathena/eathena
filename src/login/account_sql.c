#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "account.h"

#include <stdlib.h>
#include <string.h>

/// imports from outside
extern uint16 login_server_port;
extern char login_server_ip[32];
extern char login_server_id[32];
extern char login_server_pw[32];
extern char login_server_db[32];
extern char default_codepage[32];
extern char login_db[256];
extern char reg_db[256];
extern char login_db_account_id[256];
extern char login_db_userid[256];
extern char login_db_user_pass[256];
extern char login_db_level[256];

/// internal structure
typedef struct AccountDB_SQL
{
	AccountDB vtable;    // public interface
	Sql* accounts;       // SQL accounts storage
	int keepalive_timer; // ping timer id
	bool case_sensitive; // how to look up usernames

} AccountDB_SQL;

/// internal functions
static bool account_db_sql_init(AccountDB* self);
static bool account_db_sql_free(AccountDB* self);
static bool account_db_sql_create(AccountDB* self, const struct mmo_account* acc);
static bool account_db_sql_remove(AccountDB* self, const int account_id);
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc);
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const int account_id);
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid);

static int account_db_ping_init(AccountDB_SQL* db);
static int account_db_ping(int tid, unsigned int tick, int id, int data);

/// public constructor
AccountDB* account_db_sql(bool case_sensitive)
{
	AccountDB_SQL* db = (AccountDB_SQL*)aCalloc(1, sizeof(AccountDB_SQL));

	db->vtable.init     = &account_db_sql_init;
	db->vtable.free     = &account_db_sql_free;
	db->vtable.save     = &account_db_sql_save;
	db->vtable.create   = &account_db_sql_create;
	db->vtable.remove   = &account_db_sql_remove;
	db->vtable.load_num = &account_db_sql_load_num;
	db->vtable.load_str = &account_db_sql_load_str;
	
	db->case_sensitive = case_sensitive;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


/// establishes database connection
static bool account_db_sql_init(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle;

	db->accounts = Sql_Malloc();
	sql_handle = db->accounts;

	if( SQL_ERROR == Sql_Connect(sql_handle, login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(db->accounts);
		db->accounts = NULL;
		return false;
	}

	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
		Sql_ShowDebug(sql_handle);

	account_db_ping_init(db);

	return true;
}	

/// disconnects from database
static bool account_db_sql_free(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	delete_timer(db->keepalive_timer, account_db_ping);
	db->keepalive_timer = INVALID_TIMER;

	Sql_Free(db->accounts);
	db->accounts = NULL;

	return true;
}

/// create a new account entry
static bool account_db_sql_create(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle = db->accounts;
	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`, `expiration_time`) VALUES (?, ?, ?, 'a@a.com', '%d')",
	                      login_db, login_db_userid, login_db_user_pass, acc->expiration_time);

	SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (char*)acc->userid, strlen(acc->userid));
	SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (char*)acc->pass, strlen(acc->pass));
	SqlStmt_BindParam(stmt, 2, SQLDT_ENUM, (char*)&acc->sex, 1);

	SqlStmt_Execute(stmt);

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
	Sql* sql_handle = db->accounts;
	char* data;
	int i = 0;

	// retrieve login entry for the specified account
	if( SQL_ERROR == Sql_Query(sql_handle,
	    "SELECT `%s`,`%s`,`%s`,`lastlogin`,`sex`,`logincount`,`email`,`%s`,`expiration_time`,`last_ip`,`memo`,`unban_time`,`state` FROM `%s` WHERE `%s` = %d",
		login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, login_db_account_id, account_id )
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); acc->account_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); safestrncpy(acc->userid, data, sizeof(acc->userid));
	Sql_GetData(sql_handle,  2, &data, NULL); safestrncpy(acc->pass, data, sizeof(acc->pass));
	Sql_GetData(sql_handle,  3, &data, NULL); safestrncpy(acc->lastlogin, data, sizeof(acc->lastlogin));
	Sql_GetData(sql_handle,  4, &data, NULL); acc->sex = data[0];
	Sql_GetData(sql_handle,  5, &data, NULL); acc->logincount = atol(data);
	Sql_GetData(sql_handle,  6, &data, NULL); safestrncpy(acc->email, data, sizeof(acc->email));
	Sql_GetData(sql_handle,  7, &data, NULL); acc->level = atoi(data);
	Sql_GetData(sql_handle,  8, &data, NULL); acc->expiration_time = atol(data);
	Sql_GetData(sql_handle,  9, &data, NULL); safestrncpy(acc->last_ip, data, sizeof(acc->last_ip));
	Sql_GetData(sql_handle, 10, &data, NULL); safestrncpy(acc->memo, data, sizeof(acc->memo));
	Sql_GetData(sql_handle, 11, &data, NULL); acc->unban_time = atol(data);
	Sql_GetData(sql_handle, 12, &data, NULL); acc->state = atoi(data);

	Sql_FreeResult(sql_handle);


	// retrieve account regs for the specified user
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`,`value` FROM `%s` WHERE `type`='1' AND `account_id`='%d'", reg_db, acc->account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	acc->account_reg2_num = (int)Sql_NumRows(sql_handle);

	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;
		Sql_GetData(sql_handle, 0, &data, NULL); safestrncpy(acc->account_reg2[i].str, data, sizeof(acc->account_reg2[i].str));
		Sql_GetData(sql_handle, 1, &data, NULL); safestrncpy(acc->account_reg2[i].value, data, sizeof(acc->account_reg2[i].value));
		++i;
	}
	Sql_FreeResult(sql_handle);

	if( i != acc->account_reg2_num )
		return false;

	return true;
}

static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	Sql* sql_handle = db->accounts;
	char esc_userid[2*NAME_LENGTH+1];
	int account_id;
	char* data;

	Sql_EscapeString(sql_handle, esc_userid, userid);

	// get the list of account IDs for this user ID
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `%s` FROM `%s` WHERE `%s`= %s '%s'", login_db_account_id, login_db, login_db_userid, (db->case_sensitive ? "BINARY" : ""), esc_userid) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( Sql_NumRows(sql_handle) > 1 )
	{// serious problem - duplicit account
		ShowError("account_db_sql_load_str: multiple accounts found when retrieving data for account '%s'!\n", userid);
		Sql_FreeResult(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL);
	account_id = atoi(data);

	return account_db_sql_load_num(self, acc, account_id);
}



static int account_db_ping_init(AccountDB_SQL* db)
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
	add_timer_func_list(account_db_ping, "account_db_ping");
	db->keepalive_timer = add_timer_interval(gettick() + connection_ping_interval*1000, account_db_ping, 0, (int)sql_handle, connection_ping_interval*1000);

	return 0;
}

static int account_db_ping(int tid, unsigned int tick, int id, int data)
{
	Sql* sql_handle = (Sql*)data;

	Sql_Ping(sql_handle);
	return 0;
}
