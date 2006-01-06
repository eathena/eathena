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
	// Set the new sqldb_handle to replace the current standard
	CMySQL(MYSQL sqldb_handle); // Change the handler in case someone wants to use a diff one

	// Set back the default mysql_handle, and free any unfreed results
	virtual ~CMySQL();


	//Some public var?
	MYSQL_ROW row;

	// MiniString versions -> my versions =D
	bool mysql_SendQuery(const MiniString q); // Queries with no returns
	bool mysql_SendQuery(MYSQL_RES*& sql_res, const MiniString q); // queries with returns

	// New style functions
	// Will be easy to use for SQL type functions SQLite, ODBC, etc... all support the same
	// results.

	bool Query(const MiniString q);		// Do query
	bool Fetch();						// Fetch the rows
	long CountRes();					// Show how many results came in
	void Free();						// Free results



	// Old versions to removed at some point
	bool mysql_SendQuery(MYSQL_RES*& sql_res, const char* q, size_t sz=0);	// Queries that are obtaining data
	bool mysql_SendQuery(const char* q, size_t sz=0);						// Queries that are sending data

	// Please be aware that I was drunk when i wrote what i need here, so if spelling errors are
	// present, please yell at me and beat me with a stick == CLOWNISIUS

	//Queries that wil be here will be for MiniString or variants =D

	// When creating a new instance of `query` must specify what connection
	// ex:  query_sql query(MYSQL); where MYSQL is the connection created...
	//
	// This will create a string type that is the current MySQL conneciton
	// this will be useful posibly in the multi threading of MySQL connection:
	// ex: query_sql query(thread_id);

	// this will limit a thread ID per mysql connection which can be created via:
	// ex: CMySQL::connection(thread thread_id, const char * hostname, const char * user, const char * pass, int port);

	// operators should be for + and << to append more data

	// send();  sends the query, returns a MYSQL_RES for manipulation..
	// count(); returns the number of rows returned
	//
	// fetch_row(); returns an array like below each time this is run, it goes to the next row, returns true if rows exist, false if empty
	// result[n]; // array that send[] creates... returns a reference to it like MYSQL_RES does. similar to a vector
	// size();  for length/size of query/string
	// length(); for length/size of query/string
	// clear(); does a mysql_free_result for send() when the query returns a result...

	const char *escape_string(char *target, const char* source, size_t len);// Add excape strings to make it safer

protected:
	MYSQL original_mysql_handle;
	MYSQL mysqldb_handle;			// Connection ID

	MYSQL_RES *result;

	char mysqldb_ip[32];			// Server IP
	unsigned short mysqldb_port;	// Server Port
	char mysqldb_id[32];			// Username
	char mysqldb_pw[32];			// Password
	char mysqldb_db[32];			// Database to use
};


class CAccountDB_sql : public CMySQL, private CConfig, public CAccountDBInterface
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// data

	// table names
	char login_auth_db[128];
	char login_reg_db[128];
	char login_log_db[128];
	char login_status_db[128];

	// options
	bool case_sensitive;
	bool log_login;

	///////////////////////////////////////////////////////////////////////////
	// data for alternative interface
	CLoginAccount	cTempAccount;
	Mutex			cMx;
	MYSQL_RES*		cSqlRes;
	MYSQL_ROW		cSqlRow;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDB_sql(const char* configfile);
	~CAccountDB_sql() {	close(); }

	///////////////////////////////////////////////////////////////////////////
	// functions for db interface

	//!! todo, add some functionality if needed
//	virtual size_t size()	{ return 0; }
//	virtual CLoginAccount& operator[](size_t i) { static CLoginAccount dummy; return dummy; }

	virtual bool existAccount(const char* userid);
	virtual bool searchAccount(const char* userid, CLoginAccount&account);
	virtual bool searchAccount(uint32 accid, CLoginAccount&account);
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account);
	virtual bool removeAccount(uint32 accid);
	virtual bool saveAccount(const CLoginAccount& account);


	///////////////////////////////////////////////////////////////////////////
	// alternative interface

	///////////////////////////////////////////////////////////////////////////
	// get exclusive access on the database
	virtual bool aquire()
	{
		release();	// just in case
		cMx.lock();
		return this->first();
	}
	///////////////////////////////////////////////////////////////////////////
	// return exclusive access
	virtual bool release()
	{
		if(cSqlRes)
		{
			mysql_free_result(cSqlRes);
			cSqlRes=NULL;
		}
		cMx.unlock();
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// set database reader to first entry
	virtual bool first()
	{
		ScopeLock sl(cMx);
		char query[2048];
		size_t sz=snprintf(query, sizeof(query), "SELECT `*` FROM `%s`", login_auth_db);
		if(cSqlRes)
		{
			mysql_free_result(cSqlRes);
			cSqlRes=NULL;
		}
		if( this->mysql_SendQuery(cSqlRes, query, sz) )
		{
			if( mysql_num_rows(cSqlRes) > 0 )
			{
				return (*this)++;
			}
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// check if current data position is valid
	virtual operator bool()		{ ScopeLock sl(cMx); return (NULL!=cSqlRow); }
	///////////////////////////////////////////////////////////////////////////
	// go to the next data position
	virtual bool operator++(int)
	{
		ScopeLock sl(cMx);
		cSqlRow = mysql_fetch_row(cSqlRes);	//row fetching
		if(cSqlRow)
		{
			cTempAccount.account_id	= cSqlRow[0]?atol(cSqlRow[0]):0;
			safestrcpy(cTempAccount.userid, cSqlRow[1]?cSqlRow[1]:"", sizeof(cTempAccount.userid));
			safestrcpy(cTempAccount.passwd, cSqlRow[2]?cSqlRow[2]:"", sizeof(cTempAccount.passwd));
			cTempAccount.sex			= cSqlRow[3][0] == 'S' ? 2 : cSqlRow[3][0]=='M';
			cTempAccount.gm_level	= cSqlRow[4]?atol(cSqlRow[4]):0;
			cTempAccount.online		= cSqlRow[5]?atol(cSqlRow[5]):0;
			safestrcpy(cTempAccount.email, cSqlRow[6]?cSqlRow[6]:"" , sizeof(cTempAccount.email));
			cTempAccount.login_id1	= cSqlRow[7]?atol(cSqlRow[7]):0;
			cTempAccount.login_id2	= cSqlRow[8]?atol(cSqlRow[8]):0;
			cTempAccount.client_ip	= ipaddress(cSqlRow[9]);
			safestrcpy(cTempAccount.last_login, cSqlRow[10]?cSqlRow[10]:"" , sizeof(cTempAccount.last_login));
			cTempAccount.login_count	= cSqlRow[11]?atol(cSqlRow[11]):0;
			cTempAccount.valid_until	= (time_t)(cSqlRow[12]?atol(cSqlRow[12]):0);
			cTempAccount.ban_until	= (time_t)(cSqlRow[13]?atol(cSqlRow[13]):0);

			// clear unused fields until they got removed from all implementations
			cTempAccount.state = 0;
			cTempAccount.error_message[0]=0;
			cTempAccount.memo[0]=0;
			cTempAccount.last_ip[0]=0;
		}
		return (cSqlRow!=NULL);
	}
	///////////////////////////////////////////////////////////////////////////
	// store the current data
	virtual bool save()
	{
		ScopeLock sl(cMx);
		return saveAccount(cTempAccount);
	}
	///////////////////////////////////////////////////////////////////////////
	// search data (limited functionality)
	virtual bool find(const char* userid)
	{
		ScopeLock sl(cMx);
		return searchAccount(userid, cTempAccount);
	}
	virtual bool find(uint32 accid)
	{
		ScopeLock sl(cMx);
		return searchAccount(accid, cTempAccount);
	}
	///////////////////////////////////////////////////////////////////////////
	// access currently selected data
	virtual CLoginAccount& operator()()
	{
		ScopeLock sl(cMx);
		return cTempAccount;
	}

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
