#ifdef WITH_MYSQL

// MySQL
#include <mysql.h>

///////////////////////////////////////////////////////////////////////////////
//
// mysql access function
//
///////////////////////////////////////////////////////////////////////////////
static inline int mysql_SendQuery(MYSQL *mysql, const char* q)
{
#ifdef TWILIGHT
	ShowSQL("%s:%d# %s\n", __FILE__, __LINE__, q);
#endif
	return mysql_real_query(mysql, q, strlen(q));
}



extern MYSQL mmysql_handle;
extern char tmp_sql[65535];
extern MYSQL_RES* sql_res ;
extern MYSQL_ROW	sql_row ;

extern MYSQL lmysql_handle;
extern char tmp_lsql[65535];
extern MYSQL_RES* lsql_res ;
extern MYSQL_ROW	lsql_row ;

extern MYSQL logmysql_handle;
extern MYSQL_RES* logsql_res ;
extern MYSQL_ROW logsql_row ;

extern MYSQL mail_handle;
extern MYSQL_RES* 	mail_res ;
extern MYSQL_ROW	mail_row ;
extern char tmp_msql[65535];

extern int db_use_sqldbs;

extern char item_db_db[32];
extern char item_db2_db[32];
extern char mob_db_db[32];
extern char mob_db2_db[32];




#endif//WITH_MYSQL
