//add include for DBMS(mysql)
#include "dbaccess.h"

long sql_cnt;
long sql_scnt;
long sql_icnt;
long sql_dcnt;
long sql_ucnt;
long sql_rcnt;

void sql_close_debug(
	const char *file_name, const char *function_name,int line_number)
{
	mysql_close(&mysql_handle);
}

void sql_query_debug(
	const char *file_name, const char *function_name,int line_number, const char *format, ...)
{
	static char prev_query[65535];
	va_list ap;
	va_start(ap, format);
	vsprintf(tmp_sql,format,ap);
	va_end(ap);

	if(mysql_query(&mysql_handle, tmp_sql)){
		printf("---------- SQL error report ----------\n\n");
		printf("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		printf("Query: %s\n", tmp_sql);
		printf("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
//		printf("\n\nPrevious query: %s\n", prev_query);
		printf("-------- End SQL Error Report --------\n");

	}
	strcpy(prev_query,tmp_sql);
	if (tmp_sql[0]=='S') sql_scnt++;
	else if (tmp_sql[0]=='I') sql_icnt++;
	else if (tmp_sql[0]=='D') sql_dcnt++;
	else if (tmp_sql[0]=='U') sql_ucnt++;
	else if (tmp_sql[0]=='R') sql_rcnt++;
	else sql_cnt++;
}

void sql_connect_debug(
	const char *file_name, const char *function_name,int line_number,
	const char *server_ip, const char *server_id, const char *server_pw, const char *server_db, int server_port)
{
	if (!mysql_init(&mysql_handle)){
		printf("------- SQL INIT error report -------\n\n");
		printf("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		printf("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
		printf("-------- End SQL Error Report --------\n");
	}

	if(!mysql_real_connect(&mysql_handle, server_ip, server_id, server_pw, server_db, server_port, (char *)NULL, 0)) {
		printf("---------- SQL error report ----------\n\n");
		printf("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		printf("Trying to connect to server: %s\n", server_ip);
		printf("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
		printf("-------- End SQL Error Report --------\n");
	}
}
