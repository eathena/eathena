// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "sql.h"

#ifdef WIN32
#include <winsock2.h>
#endif
//add include for DBMS(mysql)
#include <mysql.h>
#include <stdlib.h>



struct Sql
{
	struct StringBuf buf;
	MYSQL handle;
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long* lengths;
};


struct SqlStmt
{
	struct StringBuf buf;
	MYSQL_STMT* stmt;
	MYSQL_BIND* params;
	MYSQL_BIND* columns;
	size_t max_params;
	size_t max_columns;
	bool bind_params;
	bool bind_columns;
};



/////////////////////////////////////////////////////////////////////
// Sql Handle
/////////////////////////////////////////////////////////////////////



/// Allocates and initializes a new Sql handle.
struct Sql* Sql_Malloc(void)
{
	struct Sql* self;
	CREATE(self, struct Sql, 1);
	mysql_init(&self->handle);
	StringBuf_Init(&self->buf);
	return self;
}



/// Establishes a connection.
int Sql_Connect(struct Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db)
{
	StringBuf_Clear(&self->buf);
	if( !mysql_real_connect(&self->handle, host, user, passwd, db, (unsigned int)port, NULL/*unix_socket*/, 0/*clientflag*/) )
	{
		ShowSQL("%s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Gets the connection timeout.
int Sql_GetTimeout(struct Sql* self, uint32* out_timeout)
{
	if( out_timeout != NULL && SQL_SUCCESS == Sql_Query(self, "SHOW VARIABLES LIKE 'wait_timeout'") )
	{
		char* data;
		size_t len;
		if( SQL_SUCCESS == Sql_NextRow(self) &&
			SQL_SUCCESS == Sql_GetData(self, 1, &data, &len) )
		{
			*out_timeout = (uint32)strtoul(data, NULL, 10);
			Sql_FreeResult(self);
			return SQL_SUCCESS;
		}
		Sql_FreeResult(self);
	}
	return SQL_ERROR;
}



/// Puts the column names of a table in out_buf, with the target separator after each name.
int Sql_GetColumnNames(struct Sql* self, const char* table, char* out_buf, size_t buf_len, char sep)
{
	char* data;
	size_t len;
	size_t off = 0;

	if( Sql_Query(self, "EXPLAIN `%s`", table) == SQL_ERROR )
		return SQL_ERROR;

	out_buf[off] = '\0';
	while( SQL_SUCCESS == Sql_NextRow(self) && SQL_SUCCESS == Sql_GetData(self, 0, &data, &len) )
	{
		len = strnlen(data, len);
		if( off + len + 2 > buf_len )
		{
			ShowDebug("Sql_GetColumns: output buffer is too small\n");
			return SQL_ERROR;
		}
		memcpy(out_buf+off, data, len);
		off += len;
		out_buf[off++] = sep;
	}
	out_buf[off] = '\0';
	Sql_FreeResult(self);
	return SQL_SUCCESS;
}



/// Changes the encoding.
int Sql_SetEncoding(struct Sql* self, const char* encoding)
{
	//## TODO escape encoding?
	return Sql_Query(self, "SET NAMES %s", encoding);
}



/// Pings the sql connection.
int Sql_Ping(struct Sql* self)
{
	if( mysql_ping(&self->handle) == 0 )
		return SQL_SUCCESS;
	return  SQL_ERROR;
}



/// Escapes a string. out_to must be strlen(from)*2+1 in size
size_t Sql_EscapeString(struct Sql* self, char *out_to, const char *from)
{
	return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)strlen(from));
}



/// Escapes a string. out_to must be from_len*2+1 in size
size_t Sql_EscapeStringLen(struct Sql* self, char *out_to, const char *from, size_t from_len)
{
	return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)from_len);
}



/// Executes a query.
int Sql_Query(struct Sql* self, const char* query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = Sql_QueryV(self, query, args);
	va_end(args);

	return res;
}



/// Executes a query.
int Sql_QueryV(struct Sql* self, const char* fmt, va_list args)
{
	Sql_FreeResult(self);
	StringBuf_Vprintf(&self->buf, fmt, args);
	if( mysql_real_query(&self->handle, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}
	self->result = mysql_store_result(&self->handle);
	if( mysql_errno(&self->handle) != 0 )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
uint64 Sql_LastInsertId(struct Sql* self)
{
	return (uint64)mysql_insert_id(&self->handle);
}



/// Returns the number of columns.
uint32 Sql_NumColumns(struct Sql* self)
{
	if( self->result != NULL )
		return (uint32)mysql_num_fields(self->result);
	return 0;
}



/// Returns the number of rows.
uint64 Sql_NumRows(struct Sql* self)
{
	if( self->result != NULL )
		return (uint64)mysql_num_rows(self->result);
	return 0;
}



/// Fetches the next row.
int Sql_NextRow(struct Sql* self)
{
	if( self->result != NULL )
	{
		self->row = mysql_fetch_row(self->result);
		if( self->row != NULL )
		{
			self->lengths = mysql_fetch_lengths(self->result);
			return SQL_SUCCESS;
		}
		self->lengths = NULL;
		if( mysql_errno(&self->handle) == 0 )
			return SQL_NO_DATA;
	}
	return SQL_ERROR;
}



/// Gets the data of a column.
int Sql_GetData(struct Sql* self, size_t col, char** out_buf, size_t* out_len)
{
	if( self->row != NULL )
	{
		if( out_buf ) *out_buf = self->row[col];
		if( out_len ) *out_len = (size_t)self->lengths[col];
		return SQL_SUCCESS;
	}
	return SQL_ERROR;
}



/// Frees the result of the query.
void Sql_FreeResult(struct Sql* self)
{
	if( self->result != NULL )
	{
		mysql_free_result(self->result);
		self->result = NULL;
		self->row = NULL;
		self->lengths = NULL;
	}
}



/// Shows debug information (last query).
void Sql_ShowDebug_(struct Sql* self, const char* debug_file, const unsigned long debug_line)
{
	if( StringBuf_Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}



/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(struct Sql *self) 
{
	Sql_FreeResult(self);
	StringBuf_Destroy(&self->buf);
	aFree(self);
}



/////////////////////////////////////////////////////////////////////
// Prepared Statements
/////////////////////////////////////////////////////////////////////



/// Binds a parameter/result.
///
/// @private
static int BindSqlDataType(MYSQL_BIND* bind, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null)
{
	memset(bind, 0, sizeof(MYSQL_BIND));
	switch( buffer_type )
	{
	case SQLDT_NULL: bind->buffer_type = MYSQL_TYPE_NULL;
		break;
	case SQLDT_UINT8: bind->is_unsigned = 1;
	case SQLDT_INT8: bind->buffer_type = MYSQL_TYPE_TINY;
		break;
	case SQLDT_UINT16: bind->is_unsigned = 1;
	case SQLDT_INT16: bind->buffer_type = MYSQL_TYPE_SHORT;
		break;
	case SQLDT_UINT32: bind->is_unsigned = 1;
	case SQLDT_INT32: bind->buffer_type = MYSQL_TYPE_LONG;
		break;
	case SQLDT_UINT64: bind->is_unsigned = 1;
	case SQLDT_INT64: bind->buffer_type = MYSQL_TYPE_LONGLONG;
		break;
	case SQLDT_FLOAT: bind->buffer_type = MYSQL_TYPE_FLOAT;
		break;
	case SQLDT_DOUBLE: bind->buffer_type = MYSQL_TYPE_DOUBLE;
		break;
	case SQLDT_STRING: bind->buffer_type = MYSQL_TYPE_STRING;
		break;
	case SQLDT_BLOB: bind->buffer_type = MYSQL_TYPE_BLOB;
		break;
	default:
		ShowDebug("BindSqlDataType: unsupported buffer type (%d)\n", buffer_type);
		return SQL_ERROR;
	}
	bind->buffer = buffer;
	bind->buffer_length = (unsigned long)buffer_len;
	bind->length = (unsigned long*)out_length;
	bind->is_null = (my_bool*)out_is_null;
	return SQL_SUCCESS;
}



/// Allocates a SqlStmt.
struct SqlStmt* SqlStmt_Malloc(struct Sql* sql)
{
	struct SqlStmt* self;
	MYSQL_STMT* stmt;

	if( sql == NULL )
		return NULL;
	stmt = mysql_stmt_init(&sql->handle);
	if( stmt == NULL )
	{
		ShowSQL("DB error - %s\n", mysql_error(&sql->handle));
		return NULL;
	}
	CREATE(self, struct SqlStmt, 1);
	StringBuf_Init(&self->buf);
	self->stmt = stmt;

	return self;
}



/// Prepares the statement.
int SqlStmt_Prepare(struct SqlStmt* self, const char* query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = SqlStmt_PrepareV(self, query, args);
	va_end(args);

	return res;
}



/// Prepares the statement.
int SqlStmt_PrepareV(struct SqlStmt* self, const char* query, va_list args)
{
	SqlStmt_FreeResult(self);
	StringBuf_Vprintf(&self->buf, query, args);
	if( mysql_stmt_prepare(self->stmt, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	self->bind_params = false;

	return SQL_SUCCESS;
}



/// Returns the number of parameters in the statement.
size_t SqlStmt_NumParams(struct SqlStmt* self)
{
	return (size_t)mysql_stmt_param_count(self->stmt);
}



/// Binds a parameter to a buffer.
int SqlStmt_BindParam(struct SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len)
{
	if( !self->bind_params )
	{// create binds for params
		size_t i;
		size_t count;
		
		count = SqlStmt_NumParams(self);
		if( self->max_params < count )
		{
			self->max_params = count;
			RECREATE(self->params, MYSQL_BIND, self->max_params);
		}
		memset(self->params, 0, count*sizeof(MYSQL_BIND));
		for( i = 0; i < count; ++i )
			self->params[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_params = true;
	}
	return BindSqlDataType(self->params+idx, buffer_type, buffer, buffer_len, NULL, NULL);
}



/// Executes the prepared statement.
int SqlStmt_Execute(struct SqlStmt* self)
{
	SqlStmt_FreeResult(self);
	if( (self->bind_params && mysql_stmt_bind_param(self->stmt, self->params)) ||
		mysql_stmt_execute(self->stmt) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	self->bind_columns = false;

	return SQL_SUCCESS;
}



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
uint64 SqlStmt_LastInsertId(struct SqlStmt* self)
{
	return (uint64)mysql_stmt_insert_id(self->stmt);
}



/// Returns the number of columns in the result.
size_t SqlStmt_NumColumns(struct SqlStmt* self)
{
	return (size_t)mysql_stmt_field_count(self->stmt);
}



/// Binds a parameter to a buffer.
int SqlStmt_BindColumn(struct SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null)
{
	if( !self->bind_columns )
	{// create binds for columns
		size_t i;
		size_t count;
		
		count = SqlStmt_NumColumns(self);
		if( self->max_columns < count )
		{
			self->max_columns = count;
			RECREATE(self->columns, MYSQL_BIND, self->max_columns);
		}
		memset(self->columns, 0, count*sizeof(MYSQL_BIND));
		for( i = 0; i < count; ++i )
			self->columns[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_columns = true;
	}
	return BindSqlDataType(self->params+idx, buffer_type, buffer, buffer_len, out_length, out_is_null);
}



/// Returns the number of rows.
uint64 SqlStmt_NumRows(struct SqlStmt* self)
{
	return (uint64)mysql_stmt_num_rows(self->stmt);
}



/// Fetches the next row.
int SqlStmt_NextRow(struct SqlStmt* self)
{
	int i;
	if( self->bind_columns && mysql_stmt_bind_result(self->stmt, self->columns) )
		i = 1;// error
	else
		i = mysql_stmt_fetch(self->stmt);
	if( i != 0 )
	{
		if( i == MYSQL_NO_DATA )
			return SQL_NO_DATA;
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Frees the result of the last execution.
void SqlStmt_FreeResult(struct SqlStmt* self)
{
	mysql_stmt_free_result(self->stmt);
}



/// Shows debug information (with statement).
void SqlStmt_ShowDebug_(struct SqlStmt* self, const char* debug_file, const unsigned long debug_line)
{
	if( StringBuf_Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}



/// Frees a SqlStmt returned by SqlStmt_Malloc.
void SqlStmt_Free(struct SqlStmt* self)
{
	SqlStmt_FreeResult(self);
	mysql_stmt_close(self->stmt);
	if( self->params )
		aFree(self->params);
	if( self->columns )
		aFree(self->columns);
	aFree(self);
}
