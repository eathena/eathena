#ifndef _SQL_H_
#define _SQL_H_

#include "base.h"
#include "baseio.h"

#ifndef TXT_ONLY

#include <mysql.h>

class CMySQL : public global, public noncopyable
{
public:
	CMySQL();
	virtual ~CMySQL();

	bool mysql_SendQuery(MYSQL_RES*& sql_res, const char* q, size_t sz=0);	// Queries that are obtaining data
	bool mysql_SendQuery(const char* q, size_t sz=0);						// Queries that are sending data

	const char *escape_string(char *target, const char* source, size_t len);// Add excape strings to make it safer

protected:
	MYSQL mysqldb_handle;			// Connection ID
	
	char mysqldb_ip[32];			// Server IP
	unsigned short mysqldb_port;	// Server Port
	char mysqldb_id[32];			// Username
	char mysqldb_pw[32];			// Password
};


class CAccountDB_sql : public CMySQL, private CConfig, public CAccountDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDB_sql(const char* configfile);
	~CAccountDB_sql() {	close(); }

	///////////////////////////////////////////////////////////////////////////
	// functions for db interface

	//!! todo, add some functionality if needed
	virtual size_t size()	{ return 0; }
	virtual CLoginAccount& operator[](size_t i) { static CLoginAccount dummy; return dummy; }

	virtual bool existAccount(const char* userid);
	virtual bool searchAccount(const char* userid, CLoginAccount&account);
	virtual bool searchAccount(uint32 accid, CLoginAccount&account);
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account);
	virtual bool removeAccount(uint32 accid);
	virtual bool saveAccount(const CLoginAccount& account);

protected:
	///////////////////////////////////////////////////////////////////////////
	// data

	// table names
	char login_db[128];
	char log_db[128];

	// field names
	char login_db_userid[128];
	char login_db_account_id[128];
	char login_db_user_pass[128];
	char login_db_level[128];

	// options
	bool case_sensitive;
	bool log_login;

	///////////////////////////////////////////////////////////////////////////
	// functions internal access
private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close();

};

#endif//!TXT_ONLY
#endif //_SQL_H_
