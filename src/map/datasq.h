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
extern MYSQL_RES* sql_res ;
extern MYSQL_ROW	sql_row ;


extern int db_use_sqldbs;

extern char map_server_ip[16];
extern unsigned short map_server_port;
extern char map_server_id[32];
extern char map_server_pw[32];
extern char map_server_db[32];

extern char item_db_db[32];
extern char item_db2_db[32];
extern char mob_db_db[32];
extern char mob_db2_db[32];




#endif//WITH_MYSQL
