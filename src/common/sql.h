#ifndef _SQL_H_
#define _SQL_H_

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

	char* tmpSql;					// Build query into here

};
#endif //_SQL_H_