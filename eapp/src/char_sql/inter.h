#ifndef _INTER_H_
#define _INTER_H_

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



///////////////////////////////////////////////////////////////////////////////






int inter_init(const char *file);
void inter_final();
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_send_gmaccounts();

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#define inter_cfgName "conf/inter_athena.conf"

extern size_t party_share_level;
extern char inter_log_filename[1024];

extern MYSQL lmysql_handle;
extern char tmp_lsql[65535];
extern MYSQL_RES* 	lsql_res ;
extern MYSQL_ROW	lsql_row ;

extern unsigned short char_server_port;
extern char char_server_ip[32];
extern char char_server_id[32];
extern char char_server_pw[32];
extern char char_server_db[32];

extern unsigned short login_db_server_port;
extern char login_db_server_ip[32];
extern char login_db_server_id[32];
extern char login_db_server_pw[32];
extern char login_db_server_db[32];

extern int log_inter;

#endif
