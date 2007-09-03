// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "../common/strlib.h"
#include "sql.h"

#ifdef WIN32
#include <winsock2.h>
#endif
#include <mysql.h>
#include <string.h>// strlen/strnlen/memcpy/memset
#include <stdlib.h>// strtoul



/// Sql handle
struct Sql
{
	StringBuf buf;
	MYSQL handle;
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long* lengths;
};

//##TODO only define when necessary
#define SQL_COMPAT_OUTLEN

#if defined(SQL_COMPAT_OUTLEN)
// compatibility between uint32 and unsigned long in out_length of columns
struct compat_outlen_s
{
	uint32* out_length;
	unsigned long buf;
};
typedef struct compat_outlen_s compat_outlen_s;
#endif


/// Sql statement
struct SqlStmt
{
	StringBuf buf;
	MYSQL_STMT* stmt;
	MYSQL_BIND* params;
	MYSQL_BIND* columns;
#if defined(SQL_COMPAT_OUTLEN)
	compat_outlen_s* column_lengths;
#endif
	size_t max_params;
	size_t max_columns;
	bool bind_params;
	bool bind_columns;
};



///////////////////////////////////////////////////////////////////////////////
// Sql Handle
///////////////////////////////////////////////////////////////////////////////



/// Allocates and initializes a new Sql handle.
Sql* Sql_Malloc(void)
{
	Sql* self;
	CREATE(self, Sql, 1);
	mysql_init(&self->handle);
	StringBuf_Init(&self->buf);
	return self;
}



/// Establishes a connection.
int Sql_Connect(Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db)
{
	if( self == NULL )
		return SQL_ERROR;

	StringBuf_Clear(&self->buf);
	if( !mysql_real_connect(&self->handle, host, user, passwd, db, (unsigned int)port, NULL/*unix_socket*/, 0/*clientflag*/) )
	{
		ShowSQL("%s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Retrieves the timeout of the connection.
int Sql_GetTimeout(Sql* self, uint32* out_timeout)
{
	if( self && out_timeout && SQL_SUCCESS == Sql_Query(self, "SHOW VARIABLES LIKE 'wait_timeout'") )
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



/// Retrieves the name of the columns of a table into out_buf, with the separator after each name.
int Sql_GetColumnNames(Sql* self, const char* table, char* out_buf, size_t buf_len, char sep)
{
	char* data;
	size_t len;
	size_t off = 0;

	if( self == NULL || SQL_ERROR == Sql_Query(self, "EXPLAIN `%s`", table) )
		return SQL_ERROR;

	out_buf[off] = '\0';
	while( SQL_SUCCESS == Sql_NextRow(self) && SQL_SUCCESS == Sql_GetData(self, 0, &data, &len) )
	{
		len = strnlen(data, len);
		if( off + len + 2 > buf_len )
		{
			ShowDebug("Sql_GetColumns: output buffer is too small\n");
			*out_buf = '\0';
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



/// Changes the encoding of the connection.
int Sql_SetEncoding(Sql* self, const char* encoding)
{
	return Sql_Query(self, "SET NAMES %s", encoding);
}



/// Pings the connection.
int Sql_Ping(Sql* self)
{
	if( self && mysql_ping(&self->handle) == 0 )
		return SQL_SUCCESS;
	return  SQL_ERROR;
}



/// Escapes a string.
size_t Sql_EscapeString(Sql* self, char *out_to, const char *from)
{
	if( self )
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)strlen(from));
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)strlen(from));
}



/// Escapes a string.
size_t Sql_EscapeStringLen(Sql* self, char *out_to, const char *from, size_t from_len)
{
	if( self )
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)from_len);
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)from_len);
}



/// Executes a query.
int Sql_Query(Sql* self, const char* query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = Sql_QueryV(self, query, args);
	va_end(args);

	return res;
}



/// Executes a query.
int Sql_QueryV(Sql* self, const char* query, va_list args)
{
	if( self == NULL )
		return SQL_ERROR;

	Sql_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_Vprintf(&self->buf, query, args);
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



/// Executes a query.
int Sql_QueryStr(Sql* self, const char* query)
{
	if( self == NULL )
		return SQL_ERROR;

	Sql_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_AppendStr(&self->buf, query);
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
uint64 Sql_LastInsertId(Sql* self)
{
	if( self )
		return (uint64)mysql_insert_id(&self->handle);
	else
		return 0;
}



/// Returns the number of columns in each row of the result.
uint32 Sql_NumColumns(Sql* self)
{
	if( self && self->result )
		return (uint32)mysql_num_fields(self->result);
	return 0;
}



/// Returns the number of rows in the result.
uint64 Sql_NumRows(Sql* self)
{
	if( self && self->result )
		return (uint64)mysql_num_rows(self->result);
	return 0;
}



/// Fetches the next row.
int Sql_NextRow(Sql* self)
{
	if( self && self->result )
	{
		self->row = mysql_fetch_row(self->result);
		if( self->row )
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
int Sql_GetData(Sql* self, size_t col, char** out_buf, size_t* out_len)
{
	if( self && self->row )
	{
		if( col < Sql_NumColumns(self) )
		{
			if( out_buf ) *out_buf = self->row[col];
			if( out_len ) *out_len = (size_t)self->lengths[col];
		}
		else
		{// out of range - ignore
			if( out_buf ) *out_buf = NULL;
			if( out_len ) *out_len = 0;
		}
		return SQL_SUCCESS;
	}
	return SQL_ERROR;
}



/// Frees the result of the query.
void Sql_FreeResult(Sql* self)
{
	if( self && self->result )
	{
		mysql_free_result(self->result);
		self->result = NULL;
		self->row = NULL;
		self->lengths = NULL;
	}
}



/// Shows debug information (last query).
void Sql_ShowDebug_(Sql* self, const char* debug_file, const unsigned long debug_line)
{
	if( self == NULL )
		ShowDebug("at %s:%lu - self is NULL\n", debug_file, debug_line);
	else if( StringBuf_Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}



/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(Sql* self) 
{
	if( self )
	{
		Sql_FreeResult(self);
		StringBuf_Destroy(&self->buf);
		aFree(self);
	}
}



///////////////////////////////////////////////////////////////////////////////
// Prepared Statements
///////////////////////////////////////////////////////////////////////////////



/// Returns the mysql integer type for the target size.
///
/// @private
static enum enum_field_types SizeToMysqlIntType(int sz)
{
	switch( sz )
	{
	case 1: return MYSQL_TYPE_TINY;
	case 2: return MYSQL_TYPE_SHORT;
	case 4: return MYSQL_TYPE_LONG;
	case 8: return MYSQL_TYPE_LONGLONG;
	default:
		ShowDebug("SizeToMysqlIntType: unsupported size (%d)\n", sz);
		return MYSQL_TYPE_NULL;
	}
}



/// Binds a parameter/result.
///
/// @private
static int BindSqlDataType(MYSQL_BIND* bind, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, unsigned long* out_length, int8* out_is_null)
{
	memset(bind, 0, sizeof(MYSQL_BIND));
	switch( buffer_type )
	{
	case SQLDT_NULL: bind->buffer_type = MYSQL_TYPE_NULL;
		break;
	// fixed size
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
	// platform dependent size
	case SQLDT_UCHAR: bind->is_unsigned = 1;
	case SQLDT_CHAR: bind->buffer_type = SizeToMysqlIntType(sizeof(char));
		break;
	case SQLDT_USHORT: bind->is_unsigned = 1;
	case SQLDT_SHORT: bind->buffer_type = SizeToMysqlIntType(sizeof(short));
		break;
	case SQLDT_UINT: bind->is_unsigned = 1;
	case SQLDT_INT: bind->buffer_type = SizeToMysqlIntType(sizeof(int));
		break;
	case SQLDT_ULONG: bind->is_unsigned = 1;
	case SQLDT_LONG: bind->buffer_type = SizeToMysqlIntType(sizeof(long));
		break;
	case SQLDT_ULONGLONG: bind->is_unsigned = 1;
	case SQLDT_LONGLONG: bind->buffer_type = SizeToMysqlIntType(sizeof(long long));
		break;
	// floating point
	case SQLDT_FLOAT: bind->buffer_type = MYSQL_TYPE_FLOAT;
		break;
	case SQLDT_DOUBLE: bind->buffer_type = MYSQL_TYPE_DOUBLE;
		break;
	// other
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
	bind->length = out_length;
	bind->is_null = (my_bool*)out_is_null;
	return SQL_SUCCESS;
}



/// Allocates and initializes a new SqlStmt handle.
SqlStmt* SqlStmt_Malloc(Sql* sql)
{
	SqlStmt* self;
	MYSQL_STMT* stmt;

	if( sql == NULL )
		return NULL;

	stmt = mysql_stmt_init(&sql->handle);
	if( stmt == NULL )
	{
		ShowSQL("DB error - %s\n", mysql_error(&sql->handle));
		return NULL;
	}
	CREATE(self, SqlStmt, 1);
	StringBuf_Init(&self->buf);
	self->stmt = stmt;
	self->params = NULL;
	self->columns = NULL;
#if defined(SQL_COMPAT_OUTLEN)
	if( sizeof(unsigned long) != 4 )//##FIXME
		self->column_lengths = NULL;
#endif
	self->max_params = 0;
	self->max_columns = 0;
	self->bind_params = false;
	self->bind_columns = false;

	return self;
}



/// Prepares the statement.
int SqlStmt_Prepare(SqlStmt* self, const char* query, ...)
{
	int res;
	va_list args;

	va_start(args, query);
	res = SqlStmt_PrepareV(self, query, args);
	va_end(args);

	return res;
}



/// Prepares the statement.
int SqlStmt_PrepareV(SqlStmt* self, const char* query, va_list args)
{
	if( self == NULL )
		return SQL_ERROR;

	SqlStmt_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_Vprintf(&self->buf, query, args);
	if( mysql_stmt_prepare(self->stmt, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	self->bind_params = false;

	return SQL_SUCCESS;
}



/// Prepares the statement.
int SqlStmt_PrepareStr(SqlStmt* self, const char* query)
{
	if( self == NULL )
		return SQL_ERROR;

	SqlStmt_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_AppendStr(&self->buf, query);
	if( mysql_stmt_prepare(self->stmt, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	self->bind_params = false;

	return SQL_SUCCESS;
}



/// Returns the number of parameters in the prepared statement.
size_t SqlStmt_NumParams(SqlStmt* self)
{
	if( self )
		return (size_t)mysql_stmt_param_count(self->stmt);
	else
		return 0;
}



/// Binds a parameter to a buffer.
int SqlStmt_BindParam(SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len)
{
	if( self == NULL )
		return SQL_ERROR;

	if( !self->bind_params )
	{// initialize the bindings
		size_t i;
		size_t count;

		count = SqlStmt_NumParams(self);
		if( self->max_params < count )
		{
			self->max_params = count;
			RECREATE(self->params, MYSQL_BIND, count);
		}
		memset(self->params, 0, count*sizeof(MYSQL_BIND));
		for( i = 0; i < count; ++i )
			self->params[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_params = true;
	}
	if( idx < self->max_params )
		return BindSqlDataType(self->params+idx, buffer_type, buffer, buffer_len, NULL, NULL);
	else
		return SQL_SUCCESS;// out of range - ignore
}



/// Executes the prepared statement.
int SqlStmt_Execute(SqlStmt* self)
{
	if( self == NULL )
		return SQL_ERROR;

	SqlStmt_FreeResult(self);
	if( (self->bind_params && mysql_stmt_bind_param(self->stmt, self->params)) ||
		mysql_stmt_execute(self->stmt) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}
	self->bind_columns = false;
	if( mysql_stmt_store_result(self->stmt) )// store all the data
	{//FIXME is this enough or do i need to bind columns first? [FlavioJS]
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
uint64 SqlStmt_LastInsertId(SqlStmt* self)
{
	if( self )
		return (uint64)mysql_stmt_insert_id(self->stmt);
	else
		return 0;
}



/// Returns the number of columns in each row of the result.
size_t SqlStmt_NumColumns(SqlStmt* self)
{
	if( self )
		return (size_t)mysql_stmt_field_count(self->stmt);
	else
		return 0;
}



/// Binds the result of a column to a buffer.
int SqlStmt_BindColumn(SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null)
{
	unsigned long* outlen;

	if( self == NULL )
		return SQL_ERROR;

	if( !self->bind_columns )
	{// initialize the bindings
		size_t i;
		size_t count;

		count = SqlStmt_NumColumns(self);
		if( self->max_columns < count )
		{
			self->max_columns = count;
			RECREATE(self->columns, MYSQL_BIND, count);
#if defined(SQL_COMPAT_OUTLEN)
			if( sizeof(unsigned long) != 4 )//##FIXME
				RECREATE(self->column_lengths, compat_outlen_s, count);
#endif
		}
		memset(self->columns, 0, count*sizeof(MYSQL_BIND));
#if defined(SQL_COMPAT_OUTLEN)
		if( sizeof(unsigned long) != 4 )//##FIXME
			memset(self->column_lengths, 0, count*sizeof(compat_outlen_s));
#endif
		for( i = 0; i < count; ++i )
			self->columns[i].buffer_type = MYSQL_TYPE_NULL;
		self->bind_columns = true;
	}
	if( idx < self->max_columns )
	{
#if defined(SQL_COMPAT_OUTLEN)
		if( sizeof(unsigned long) != 4 )//##FIXME
		{
			self->column_lengths[idx].out_length = out_length;
			outlen = &(self->column_lengths[idx].buf);
		}
		else
#endif
		{
			outlen = (unsigned long*)out_length;
		}
		return BindSqlDataType(self->columns+idx, buffer_type, buffer, buffer_len, outlen, out_is_null);
	}
	else
	{
		return SQL_SUCCESS;// out of range - ignore
	}
}



/// Returns the number of rows in the result.
uint64 SqlStmt_NumRows(SqlStmt* self)
{
	if( self )
		return (uint64)mysql_stmt_num_rows(self->stmt);
	else
		return 0;
}



/// Fetches the next row.
int SqlStmt_NextRow(SqlStmt* self)
{
	int err;

	if( self == NULL )
		return SQL_ERROR;

	// bind columns
	if( self->bind_columns && mysql_stmt_bind_result(self->stmt, self->columns) )
		err = 1;// error binding columns
	else
		err = (size_t)mysql_stmt_fetch(self->stmt);// fetch row

	if( err == MYSQL_NO_DATA )
		return SQL_NO_DATA;
	if( err )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(self->stmt));
		return SQL_ERROR;
	}

#if defined(SQL_COMPAT_OUTLEN)
	// columns bindings - length
	if( self->bind_columns && sizeof(unsigned long) != 4 )//##FIXME
	{
		size_t i;
		size_t count = SqlStmt_NumColumns(self);
		compat_outlen_s* outlen;

		for( i = 0; i < count; ++i )
		{
			outlen = &self->column_lengths[i];
			if( outlen->out_length )
				*outlen->out_length = (uint32)outlen->buf;
		}
	}
#endif

	return SQL_SUCCESS;
}



/// Frees the result of the statement execution.
void SqlStmt_FreeResult(SqlStmt* self)
{
	if( self )
		mysql_stmt_free_result(self->stmt);
}



/// Shows debug information (with statement).
void SqlStmt_ShowDebug_(SqlStmt* self, const char* debug_file, const unsigned long debug_line)
{
	if( self == NULL )
		ShowDebug("at %s:%lu -  self is NULL\n", debug_file, debug_line);
	if( StringBuf_Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}



/// Frees a SqlStmt returned by SqlStmt_Malloc.
void SqlStmt_Free(SqlStmt* self)
{
	if( self )
	{
		SqlStmt_FreeResult(self);
		StringBuf_Destroy(&self->buf);
		mysql_stmt_close(self->stmt);
		if( self->params )
			aFree(self->params);
		if( self->columns )
		{
			aFree(self->columns);
#if defined(SQL_COMPAT_OUTLEN)
			if( sizeof(unsigned long) != 4 )//##FIXME
				aFree(self->column_lengths);
#endif
		}
		aFree(self);
	}
}
