//add include for DBMS(mysql)
#include "dbaccess.h"
#include "showmsg.h"

long sql_cnt;
long sql_scnt;
long sql_icnt;
long sql_dcnt;
long sql_ucnt;
long sql_rcnt;
char tmp_sql[65535];

MYSQL mysql_handle;
MYSQL_RES* 	sql_res;
MYSQL_ROW	sql_row;


static char *server_options[] = { "mysql_test", "--defaults-file=my.cnf" };
int num_elements = sizeof(server_options)/ sizeof(char *);
static char *server_groups[] = { "libmysqld_server", "libmysqld_client" };

void sql_query_debug(
	const char *file_name, const char *function_name,int line_number, const char *format, ...)
{
	#ifdef SQL_DEBUG
	static char prev_query[65535];
	#endif
	va_list ap;
	va_start(ap, format);
	vsprintf(tmp_sql,format,ap);
	va_end(ap);

	if( mysql_real_query(&mysql_handle, tmp_sql, strlen(tmp_sql)) ){
		ShowWarning("---------- SQL error report ----------\n\n");
		ShowWarning("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		ShowWarning("Query: %s\n", tmp_sql);
		ShowWarning("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
		#ifdef SQL_DEBUG
		ShowWarning("\n\nPrevious query: %s\n", prev_query);
		#endif
		ShowWarning("-------- End SQL Error Report --------\n");
		exit(1);
	}
	#ifdef SQL_DEBUG
	strcpy(prev_query,tmp_sql);
	#endif
	if (tmp_sql[0]=='S' || tmp_sql[0]=='E'){
	#ifdef SQL_DEBUG
		sql_scnt++;
	#endif
		sql_res = mysql_store_result(&mysql_handle);
	}
	#ifdef SQL_DEBUG
	else if (tmp_sql[0]=='I') sql_icnt++;
	else if (tmp_sql[0]=='D') sql_dcnt++;
	else if (tmp_sql[0]=='U') sql_ucnt++;
	else if (tmp_sql[0]=='R') sql_rcnt++;
	else sql_cnt++;
	#endif
}

int sql_fetch_row(void){
	if ((sql_row = mysql_fetch_row(sql_res))) return 1;
	return 0;
}

int sql_num_rows(void){
	return mysql_num_rows(sql_res);
}

void sql_free(void){
	mysql_free_result(sql_res);
}

void sql_connect_debug(
	const char *file_name, const char *function_name,int line_number,
	const char *server_ip, const char *server_id, const char *server_pw, const char *server_db, int server_port)
{
	//Will be used when testing of the embedded server begins.

	//mysql_server_init(num_elements, server_options, server_groups);

	if (!mysql_init(&mysql_handle)){
		ShowWarning("------- SQL INIT error report -------\n\n");
		ShowWarning("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		ShowWarning("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
		ShowWarning("-------- End SQL Error Report --------\n");
		exit(1);
	}

	if(!mysql_real_connect(&mysql_handle, server_ip, server_id, server_pw, server_db, server_port, (char *)NULL, 0)) {
		ShowWarning("---------- SQL error report ----------\n\n");
		ShowWarning("MySQL Server Error: %s\n", mysql_error(&mysql_handle));
		ShowWarning("Trying to connect to server: %s\n", server_ip);
		ShowWarning("Filename: %s Function: %s Line number:%d\n",file_name, function_name,line_number);
		ShowWarning("-------- End SQL Error Report --------\n");
		exit(1);
	}
}

void sql_close_debug(
	const char *file_name, const char *function_name,int line_number)
{
	mysql_close(&mysql_handle);
	// Will be used when testing of the embedded server begins.
	//mysql_server_end();

}
