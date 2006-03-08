#ifndef _SQL_H_
#define _SQL_H_

#include "base.h"
#include "baseio.h"

#ifndef TXT_ONLY

#include <mysql.h>


class CMySQL : public global, public noncopyable
{
public:
	///////////////////////////////////////////////////////////////////////////
	// database connection object
	// enables threadsafe/multithreaded connections to the sql database
	class DBConnection
	{
		///////////////////////////////////////////////////////////////////////
		// class data
		bool			cInit;		// necessary to enable delayed initialisation/safe destruction
		CMySQL&			cMySQL;		// reference to the base
		MYSQL			cHandle;	// handle to the mysql database
		MYSQL_RES*		cRes;		// result pointer
		MYSQL_ROW		cRow;		// the result row

		///////////////////////////////////////////////////////////////////////
		// startup function is automatically called on the first query
		bool startup(void)
		{	
			if(!this->cInit)
			{	// init new database handle
				mysql_init(&(this->cHandle));

				// DB connection start
				ShowMessage("Connect Database Server on %s%u....\n", cMySQL.mysqldb_ip, cMySQL.mysqldb_port);
				if( mysql_real_connect(&(this->cHandle), cMySQL.mysqldb_ip, cMySQL.mysqldb_id, cMySQL.mysqldb_pw, cMySQL.mysqldb_db, cMySQL.mysqldb_port, (char *)NULL, 0) )
				{
					ShowMessage("connect success!\n");
					this->cInit = true;
				}
				else
				{	// pointer check
					ShowError("%s\n", mysql_error(&(this->cHandle)));
				}
			}
			return this->cInit;
		}
	public:
		///////////////////////////////////////////////////////////////////////
		// constructor/destructor
		DBConnection(CMySQL& mysql)
			: cInit(false), cMySQL(mysql), cRes(NULL), cRow(0)
		{ }
		~DBConnection()
		{
			this->close();
		}
		///////////////////////////////////////////////////////////////////////
		// copy/assignment (assignment disabled)
		DBConnection(const DBConnection& db)
			: cInit(false), cMySQL(db.cMySQL), cRes(NULL), cRow(0)
		{ }
		const DBConnection& operator=(const DBConnection& DBConnection);
		
		///////////////////////////////////////////////////////////////////////
		// Queries with no returns
		bool PureQuery(const string<>& q)
		{
			if( this->startup() )
			{
				this->clear();
				if( 0==mysql_real_query(&cHandle, (const char*)q, q.length()) )
					return true;
				else
				{
					ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&this->cHandle), (const char*)q);
				}
			}
			return false;
		}
		///////////////////////////////////////////////////////////////////////
		// Queries with returns, automatically fetch first result
		bool ResultQuery(const string<>& q)
		{
			if( this->startup() )
			{
				this->clear();
				if( 0==mysql_real_query(&cHandle, (const char*)q, q.length()) )
				{
					cRes = mysql_store_result(&this->cHandle);
					if(cRes)
					{
						this->cRow = mysql_fetch_row(this->cRes);
						return true;
					}
					else
						ShowError("DB result error\nQuery:    %s\n", (const char*)q);
				}
				else
					ShowError("Database Error %s\nQuery:    %s\n", mysql_error(&this->cHandle), (const char*)q);
			}
			return false;
		}
		///////////////////////////////////////////////////////////////////////
		// number of results
		size_t size() const
		{
			return (this->cRes) ? mysql_num_rows(this->cRes) : 0;
		}
		///////////////////////////////////////////////////////////////////////
		// next result
		bool operator++(int)
		{
			return ( this->cRes && (cRow = mysql_fetch_row(this->cRes) ) );
		}
		///////////////////////////////////////////////////////////////////////
		// access the row, also prevent returning NULL pointers
		const char*operator[](int inx)
		{
			return (this->cRes && this->cRow[inx])?(this->cRow[inx]):("");
		}
		///////////////////////////////////////////////////////////////////////
		// free result memory
		void clear()
		{
			if (this->cRes)
			{
				mysql_free_result(this->cRes);
				this->cRes=NULL;
			}
		}
		///////////////////////////////////////////////////////////////////////
		// close the database connection
		void close()
		{
			this->clear();
			if( this->cInit )
			{
				ShowMessage("Closing Database Server %s%u\n", cMySQL.mysqldb_ip, cMySQL.mysqldb_port);
				mysql_close(&(this->cHandle));
				this->cInit=false;
			}
		}
	};
	///////////////////////////////////////////////////////////////////////////
protected:
	// the pool of database handles
	// will automatically create a many as necessary
	TPool<DBConnection>	cDBPool;
	// allow the database connection to read base internals
	friend class DBConnection;

	void example()
	{	
		// usage of the database pool:
	
		// get a database object out of the pool:
		CMySQL::DBConnection& db = this->cDBPool.aquire();

		//do a query:
		if( !db.ResultQuery("select * from `somewhere`") )
			printf("some error");

		printf( "has found %lu results\n", (ulong)db.size());
		
		// the first result is automatically fetched on success
		// so read it's cols
		int      resi = atoi( db[0] );

		// select the next row:
		db++;

		// put the database object back to the pool after queries beeing finished:
		this->cDBPool.release(db);
		
		// !! dont work on released objects, when using manual aquire/release !!
		// !! dont forget to to release an object when finished with it !!


		// therefore:
		// automatic aquire/release by using the TPoolObj:
		// aquires on instantiation/releases on destruction
		// TPoolObj behaves like a pointer to the database object
		TPoolObj<CMySQL::DBConnection> dbobj(this->cDBPool);

		//do a query:
		if( !dbobj->ResultQuery("select * from `somewhere`") )
			printf("some error");

		if( dbobj->PureQuery("DROP TABLE IF EXISTS `something`") )
			printf("ok");

		printf( "has found %lu results\n", (ulong)dbobj->size());

		// the first result is automatically fetched on success
		// so read it's cols
		string<> ress = (*dbobj)[1];
		
		// select the next row:
		(*dbobj)++;


		// going out of scope
		// dbobj is destroyed and puts back the DBConnection to the pool


		resi++;	// just to have annoying C4189 disabled
	}


public:
	CMySQL();
	virtual ~CMySQL();

	//Some public var?
	MYSQL_ROW row;

	// string<> versions -> my versions =D
	bool mysql_SendQuery(const string<> q); // Queries with no returns
	bool mysql_SendQuery(MYSQL_RES*& sql_res, const string<> q); // queries with returns

	// New style functions
	// Will be easy to use for SQL type functions SQLite, ODBC, etc... all support the same
	// results.

	bool SendQuery(const string<> q);	// Do query
	bool FetchResults();				// Fetch the rows
	long CountResults();				// Show how many results came in
	void FreeResults();					// Free results



	// Old versions to removed at some point
	bool mysql_SendQuery(MYSQL_RES*& sql_res, const char* q, size_t sz=0);	// Queries that are obtaining data
	bool mysql_SendQuery(const char* q, size_t sz=0);						// Queries that are sending data


	// Please be aware that I was drunk when i wrote what i need here, so if spelling errors are
	// present, please yell at me and beat me with a stick == CLOWNISIUS

	//Queries that wil be here will be for string<> or variants =D

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
	string<> login_auth_db;
	string<> login_reg_db;
	string<> login_log_db;
	string<> login_status_db;

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
	virtual ~CAccountDB_sql() {	close(); }

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
		string<> query;

		query << "SELECT `*` FROM `" << login_auth_db << "`";

		if(cSqlRes)
		{
			mysql_free_result(cSqlRes);
			cSqlRes=NULL;
		}
		if( this->mysql_SendQuery(cSqlRes, query.c_str(), query.length() ) )
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

class CCharDB_sql : public CMySQL, private CConfig, public CCharDBInterface
{
	///////////////////////////////////////////////////////////////////////////
	// config stuff
	// uint32 next_char_id;

	string<> char_db;
	string<> friend_db;
	string<> memo_db;
	string<> cart_db;
	string<> inventory_db;
	string<> skill_db;
	string<> char_reg_db;

	string<> mail_db;

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
	CCharDB_sql(const char *dbcfgfile);
	virtual ~CCharDB_sql();

private:

	bool compare_item(const struct item &a, const struct item &b);
	bool read_friends(){return true;}


public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return 0;/*cCharList.size();*/ }
	virtual CCharCharacter& operator[](size_t i)	{ static CCharCharacter tmp; return tmp; /*cCharList[i];*/ }

	virtual bool existChar(const char* name);
	virtual bool existChar(uint32 char_id);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(uint32 char_id, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(uint32 charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(uint32 accid);


	///////////////////////////////////////////////////////////////////////////
	// Mail should be in its own class at some point, shouldnt be mixed in
	// saving and loading of character info... no relation
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

	virtual size_t getUnreadCount(uint32 cid);
	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer);
	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail);
	virtual bool deleteMail(uint32 cid, uint32 mid);
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid);




	virtual size_t getMailCount(uint32, uint32&, uint32&){return 0;}
	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire()
	{
		return this->first();
	}
	virtual bool release()
	{
		return true;
	}
	virtual bool first()
	{
		return this->operator bool();
	}
	virtual operator bool()
	{
		return true;
	}
	virtual bool operator++(int)
	{
		return this->operator bool();
	}
	virtual bool save()
	{
		return true;
	}

	virtual bool find(const char* name)
	{
		// search in index 1
		return existChar(name);

	}
	virtual bool find(uint32 char_id)
	{
		return existChar(char_id);
	}
	virtual CCharCharacter& operator()()
	{
		static CCharCharacter dummy;
		return dummy;
	}



private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		char_db = "char";
		friend_db = "friends";
		memo_db = "memo";
		cart_db = "cart";
		inventory_db = "inventory";
		skill_db = "skill";
		char_reg_db = "char_reg";

		mail_db = "mail";

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




class CGuildDB_sql : public CMySQL, private CConfig, public CGuildDBInterface
{


	string<> guild_db;
	string<> guild_member_db;
	string<> guild_skill_db;
	string<> guild_position_db;
	string<> guild_alliance_db;
	string<> guild_expulsion_db;

	string<> char_db;

	char castle_db[256];

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB_sql(const char *configfile)
	{
		guild_db = "guild";
		guild_member_db = "guild_member";
		guild_skill_db = "guild_skill_db";
		guild_position_db = "guild_position";
		guild_alliance_db = "guild_alliance";
		guild_expulsion_db = "guild_expulsion";
		char_db = "char_db";

		init(configfile);
	}
	virtual ~CGuildDB_sql()
	{
		close();
	}

private:
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
		//saveAllInMemory();
		return false;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return 0;/*cGuilds.size();*/ }
	virtual CGuild& operator[](size_t i)	{ static CGuild tmp; return tmp; /*return cGuilds[i];*/ }

	virtual size_t castlesize()				{ return 0;/*cGuilds.size();*/ }
	virtual CCastle &castle(size_t i)		{ static CCastle tmp; return tmp; }


	virtual bool searchGuild(const char* name, CGuild& guild);
	virtual bool searchGuild(uint32 guildid, CGuild& guild);
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &g);
	virtual bool removeGuild(uint32 guild_id);
	virtual bool saveGuild(const CGuild& g);

	virtual bool searchCastle(ushort castle_id, CCastle& castle);
	virtual bool saveCastle(CCastle& castle);
	virtual bool removeCastle(ushort castle_id);
};


#endif//!TXT_ONLY
#endif //_SQL_H_
