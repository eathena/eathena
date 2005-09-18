#ifndef _DBACCESS_H_
#define _DBACCESS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mysql.h"

MYSQL mysql_handle;
char tmp_sql[65535];
MYSQL_RES* 	sql_res;
MYSQL_ROW	sql_row;

extern long sql_cnt;
extern long sql_icnt;
extern long sql_scnt;
extern long sql_dcnt;
extern long sql_ucnt;
extern long sql_rcnt;

#define sql_close(void) sql_close_debug(__FILE__,__func__,__LINE__)
#define sql_query(_x,...) sql_query_debug(__FILE__,__func__,__LINE__,_x,__VA_ARGS__)
#define sql_connect(_v,_w,_x,_y,_z) sql_connect_debug(__FILE__,__func__,__LINE__,_v,_w,_x,_y,_z)

void sql_close_debug(const char *file_name, const char *function_name,int line_number);
void sql_query_debug(const char *file_name, const char *function_name,int line_number, const char *format, ...);
void sql_connect_debug(const char *file_name, const char *function_name,int line_number,const char *server_ip, const char *server_id, const char *server_pw, const char *server_db, int server_port);

#endif
