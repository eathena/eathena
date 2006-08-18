#ifndef __BASEMYSQL_H__
#define __BASEMYSQL_H__


#include "basetypes.h"
#include "baseobjects.h"
#include "basestring.h"
#include "basestrformat.h"
#include "basestrsearch.h"
#include "basepool.h"

#ifdef WITH_MYSQL


#include <mysql.h>


NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// testfunction.
void test_mysql();



///////////////////////////////////////////////////////////////////////////////
/// mysql database interface.
class CMySQL : public global, public noncopyable
{
public:
	///////////////////////////////////////////////////////////////////////////
	/// database connection object.
	/// enables threadsafe/multithreaded connections to the sql database
	class DBConnection
	{
		///////////////////////////////////////////////////////////////////////
		/// class data
		bool			cInit;		///< necessary to enable delayed initialisation/safe destruction
		CMySQL&			cMySQL;		///< reference to the base
		MYSQL			cHandle;	///< handle to the mysql database
		MYSQL_RES*		cRes;		///< result pointer
		MYSQL_ROW		cRow;		///< the result row

		///////////////////////////////////////////////////////////////////////
		/// startup function. is automatically called on the first query
		bool startup(void);
		///////////////////////////////////////////////////////////////////////
		/// assignment disabled.
		const DBConnection& operator=(const DBConnection& DBConnection);
	public:
		///////////////////////////////////////////////////////////////////////
		/// constructor.
		DBConnection(CMySQL& mysql)
			: cInit(false), cMySQL(mysql), cRes(NULL), cRow(0)
		{ }
		///////////////////////////////////////////////////////////////////////
		/// destructor.
		~DBConnection()
		{
			this->close();
		}
		///////////////////////////////////////////////////////////////////////
		/// copy constructor. (necessary for the pool)
		DBConnection(const DBConnection& db)
			: cInit(false), cMySQL(db.cMySQL), cRes(NULL), cRow(0)
		{ }
		

		///////////////////////////////////////////////////////////////////////
		/// Queries with no returns.
		bool PureQuery(const string<>& q);
		///////////////////////////////////////////////////////////////////////
		/// Queries with returns. automatically fetch first result
		bool ResultQuery(const string<>& q);
		///////////////////////////////////////////////////////////////////////
		/// number of results
		size_t countResults() const
		{
			return (this->cRes!=NULL) ? mysql_num_rows(this->cRes) : 0;
		}
		///////////////////////////////////////////////////////////////////////
		/// returns the last generated id. 
		/// if the previous operaion was not an insert with autoincrement
		/// the return value is 0
		size_t getLastID() const
		{
			return mysql_insert_id(const_cast<struct st_mysql *>(&this->cHandle));
		}
		///////////////////////////////////////////////////////////////////////
		/// results iterator is valid.
		bool isvalid() const
		{
			return (this->cRes!=NULL && this->cRow!=NULL);
		}
		///////////////////////////////////////////////////////////////////////
		/// results iterator is valid.
		operator bool() const
		{
			return (this->cRes!=NULL && this->cRow!=NULL);
		}
		///////////////////////////////////////////////////////////////////////
		/// number of results.
		size_t size() const;
		///////////////////////////////////////////////////////////////////////
		/// next result.
		bool operator++(int)	{ return this->next(); }
		bool operator++()		{ return this->next(); }
		bool next();
		///////////////////////////////////////////////////////////////////////
		/// access the row. also prevent returning NULL pointers
		const char*operator[](int inx);
		///////////////////////////////////////////////////////////////////////
		/// free result memory.
		void clear();
		///////////////////////////////////////////////////////////////////////
		/// close the database connection.
		void close();
		///////////////////////////////////////////////////////////////////////
		/// Make string MySQL safe.
		string<> escaped(const string<>& source);
	};

	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class DBConnection;	// allow the database connection to read base internals
	friend class TPoolObj<DBConnection>;

protected:
	///////////////////////////////////////////////////////////////////////////
	// the pool of database handles
	// will automatically create a many as necessary
	TPool<DBConnection>	cDBPool;

	string<> mysqldb_id;	///< username
	string<> mysqldb_pw;	///< password
	string<> mysqldb_db;	///< database
	string<> mysqldb_ip;	///< server ip
	string<> mysqldb_cp;	///< code page
	ushort   mysqldb_port;	///< server port

public:
	/// constructor.
	CMySQL();
	/// constructor with initialisation.
	CMySQL(const string<>& id, const string<>& pw, const string<>& db, const string<>& ip, const ushort port, const string<>& cp=nullstring)
		: cDBPool(*this)	// initialize the first database object
	{
		this->init(id, pw, db, ip, port, cp);
	}
	/// destructor.
	virtual ~CMySQL();
	
	/// initialisation.
	bool init(const string<>& id, const string<>& pw, const string<>& db, const string<>& ip, const ushort port, const string<>& cp=nullstring);

	/// aquire a database connection.
	DBConnection& aquire()				{ return this->cDBPool.aquire(); }
	/// aquire a connection object.
	void release(DBConnection& elem)	{ this->cDBPool.release(elem); }

	/// access to the pool.
	operator TPool<DBConnection>&()		{ return this->cDBPool; }
};


/*
/// overload string streaming. to escape every character
{	
	bool escaped=false;
	if(cstr)
	while(*cstr)
	{
		if( escaped )
		{	// switch off
			escaped = false;
		}
		else if(*cstr == '\\')
		{	// this char is escaped already
			escaped = true;

		}
		else
		{	// we add an escape 
			str.append('\\');
		}
		str.append(*cstr++);
	}
	return str;
}
*/

///////////////////////////////////////////////////////////////////////////////
/// wrapper shortcut to the pool object.
/// does just simplifies writing, 
/// will be optimized away since beeing completely inlined
class CMySQLConnection
{
	TPoolObj<CMySQL::DBConnection> cObj;

public:

	CMySQLConnection(CMySQL& base) : cObj(base)
	{}
	~CMySQLConnection()
	{}

	///////////////////////////////////////////////////////////////////////////
	/// Queries with no returns.
	bool PureQuery(const string<>& q)	{ return cObj->PureQuery(q); }
	///////////////////////////////////////////////////////////////////////////
	/// Queries with returns. automatically fetch first result
	bool ResultQuery(const string<>& q)	{ return cObj->ResultQuery(q); }
	///////////////////////////////////////////////////////////////////////////
	/// number of results
	size_t countResults() const			{ return cObj->countResults(); }
	///////////////////////////////////////////////////////////////////////////
	/// returns the last generated id. 
	size_t getLastID() const			{ return cObj->getLastID(); }
	///////////////////////////////////////////////////////////////////////////
	/// results iterator is valid.
	bool isvalid() const				{ return cObj->isvalid(); }
	///////////////////////////////////////////////////////////////////////////
	/// results iterator is valid.
	operator bool() const				{ return cObj->operator bool(); }
	///////////////////////////////////////////////////////////////////////////
	/// number of results.
	size_t size() const					{ return cObj->size(); }
	///////////////////////////////////////////////////////////////////////////
	/// next result.
	bool operator++(int)				{ return cObj->operator++(1); }
	bool operator++()					{ return cObj->operator++(); }
	bool next()							{ return cObj->next(); }
	///////////////////////////////////////////////////////////////////////////
	/// access the row. also prevent returning NULL pointers
	const char*operator[](int inx)		{ return cObj->operator[](inx); }
	///////////////////////////////////////////////////////////////////////////
	/// free result memory.
	void clear()						{ cObj->clear(); }
	///////////////////////////////////////////////////////////////////////////
	/// close the database connection.
	void close()						{ cObj->close(); }
	///////////////////////////////////////////////////////////////////////////
	/// Make string MySQL safe.
	string<> escaped(const string<>& source)	{ return cObj->escaped(source); }
};

NAMESPACE_END(basics)

#endif//WITH_MYSQL

#endif//__BASEMYSQL_H__

