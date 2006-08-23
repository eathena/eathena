#include "basemysql.h"


#ifdef WITH_MYSQL

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// CMySQL Class Constructor
CMySQL::CMySQL()
	: cDBPool(*this)			// initialize the first database object
	, mysqldb_id("ragnarok")
	, mysqldb_pw("ragnarok")
	, mysqldb_db("ragnarok")
	, mysqldb_ip("127.0.0.1")
	, mysqldb_port(3306)
{}

///////////////////////////////////////////////////////////////////////////////
/// CMySQL Class Destructor
CMySQL::~CMySQL()
{	// close all existing database objects in the pool
	this->cDBPool.call( &DBConnection::close );
}

///////////////////////////////////////////////////////////////////////////////
/// initialisation.
bool CMySQL::init(const string<>& id, const string<>& pw, const string<>& db, const string<>& ip, const ushort port, const string<>& cp)
{
	// set the new connection info
	this->mysqldb_id	= id;
	this->mysqldb_pw	= pw;
	this->mysqldb_db	= db;
	this->mysqldb_ip	= ip;
	this->mysqldb_cp	= cp;
	this->mysqldb_port	= port;

	// close all existing database objects in the pool
	// so they reopen on the next query with the new db
	this->cDBPool.call( &DBConnection::close );	
	return true;
}


///////////////////////////////////////////////////////////////////////////////
/// startup function.
/// is automatically called on the first query
bool CMySQL::DBConnection::startup(void)
{
	if(!this->cInit)
	{	// init new database handle
		mysql_init(&(this->cHandle));

		// DB connection start
		printf("Connect Database Server on %s%u...", cMySQL.mysqldb_ip.c_str(), cMySQL.mysqldb_port);
		if( mysql_real_connect(&(this->cHandle), cMySQL.mysqldb_ip, cMySQL.mysqldb_id, cMySQL.mysqldb_pw, cMySQL.mysqldb_db, cMySQL.mysqldb_port, (char *)NULL, 0) )
		{
			this->cInit = true;
			const bool cpset = ( cMySQL.mysqldb_cp.length() && this->PureQuery( "SET NAMES " + cMySQL.mysqldb_cp ) );
			printf( ((cpset) ? "success! (cp is '%s')\n" : "success!\n"), (const char*)cMySQL.mysqldb_cp);
		}
		else
		{	// pointer check
			printf("failed\n%s\n", mysql_error(&(this->cHandle)));
		}
	}
	return this->cInit;
}
///////////////////////////////////////////////////////////////////////////////
/// Queries with no returns.
bool CMySQL::DBConnection::PureQuery(const string<>& q)
{
	int i=2;
	while(i--)
	{
		if( this->startup() )
		{
			this->clear();
			if( 0==mysql_real_query(&this->cHandle, (const char*)q, q.length()) )
			{
				return true;
			}
			if(i)
			{
				// CR_SERVER_GONE_ERROR
				// mysql does not include its errormessages 
				//if( 2006 == mysql_errno(&this->cHandle) )

				// just reconnect and try again
				this->close();
			}
			else
			{
				printf("Database Error %s\nQuery:    %s\n", mysql_error(&this->cHandle), (const char*)q);
			}
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// Queries with returns, automatically fetch first result
bool CMySQL::DBConnection::ResultQuery(const string<>& q)
{
	if( this->PureQuery(q) )
	{
		cRes = mysql_store_result(&this->cHandle);
		if(cRes)
		{
			this->cRow = mysql_fetch_row(this->cRes);
			return (this->cRow);
		}
		else
			printf("DB result error\nQuery:    %s\n", (const char*)q);
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// number of results.
size_t CMySQL::DBConnection::size() const
{
	return (this->cRes) ? mysql_num_rows(this->cRes) : 0;
}
///////////////////////////////////////////////////////////////////////////////
/// select next result.
bool CMySQL::DBConnection::next()
{
	return ( this->cRes && (cRow = mysql_fetch_row(this->cRes) ) );
}
///////////////////////////////////////////////////////////////////////////////
/// access the row. also prevent returning NULL pointers
const char* CMySQL::DBConnection::operator[](int inx)
{
	return (this->cRes && this->cRow && this->cRow[inx])?(this->cRow[inx]):("");
}
///////////////////////////////////////////////////////////////////////////////
// free result memory.
void CMySQL::DBConnection::clear()
{
	if (this->cRes)
	{
		mysql_free_result(this->cRes);
		this->cRes=NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////
/// close the database connection.
void CMySQL::DBConnection::close()
{
	this->clear();
	if( this->cInit )
	{
		printf("Closing Database Server %s%u\n", cMySQL.mysqldb_ip.c_str(), cMySQL.mysqldb_port);
		mysql_close(&(this->cHandle));
		this->cInit=false;
	}
}
///////////////////////////////////////////////////////////////////////////////
/// Make string MySQL safe.
/// disadvantage is the additional allocation of an intermediate buffer
/// to be able to call the external mysql_real_escape_string
string<> CMySQL::DBConnection::escaped(const string<>& source)
{
	basestring<> str;	// use it just as expandable container
	if( source.length()>0 && this->startup() && str.checksize(2*source.size()) )
	{	// keep enough space to hold all chars with escape
		char* ptr = str.begin();
		// call the mysql escaper
		mysql_real_escape_string(&this->cHandle, ptr, source.begin(), source.size());
		// build the return string
		return string<>(ptr);
	}
	return string<>();
}














void test_mysql()
{
#ifdef DEBUG
	// usage of the database pool:
	CMySQL database;


	// get a database object out of the pool:
	CMySQL::DBConnection& conn = database.aquire();

	//do a query:
	if( !conn.ResultQuery("select * from `charlog`") )
		printf("some error");

	printf( "has found %lu results\n", (ulong)conn.size());

	// the first result is automatically fetched on success
	// so read it's cols
	int i = atoi( conn[0] );

	i=0;
	while( conn )
	{
		printf("%i: %s\n", i, conn[0]);
		// select the next row:
		conn++;
		i++;
	}

	// put the database object back to the pool after queries beeing finished:
	database.release(conn);

	// !! dont work on released objects, when using manual aquire/release !!
	// !! dont forget to to release an object when finished with it !!


	// therefore:
	// automatic aquire/release by using the TPoolObj:
	// aquires on instantiation/releases on destruction
	// TPoolObj behaves like a pointer to the database object
	TPoolObj<CMySQL::DBConnection> dbobj(database);

	// do a query:
	if( !dbobj->ResultQuery("select * from `loginlog`") )
		printf("some error");


	printf( "has found %lu results\n", (ulong)dbobj->size());

	// the first result is automatically fetched on success
	// so read it's cols
	string<> ress = (*dbobj)[1];

	i=0;
	while( *dbobj )
	{
		printf("%i: %s\n", i, (*dbobj)[1]);
		// select the next row:
		(*dbobj)++;
		i++;
	}

	// access on invalid result/row returns an empty string by default
	ress = (*dbobj)[1];

	// going out of scope
	// dbobj is destroyed and puts back the DBConnection to the pool


	string<> escaped = dbobj->escaped("abs'cde´asas`asas");




	// same functionality a bit more wrapped
	CMySQLConnection dbcon(database);
	// do a query:
	if( !dbcon.ResultQuery("select * from `loginlog`") )
		printf("some error");


	printf( "has found %lu results\n", (ulong)dbcon.size());

	i=0;
	while( dbcon )
	{
		printf("%i: %s\n", i, dbcon[1]);
		// select the next row:
		dbcon++;
		i++;
	}
#endif//DEBUG
}

NAMESPACE_END(basics)

#endif//WITH_MYSQL


