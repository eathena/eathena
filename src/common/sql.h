// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _COMMON_SQL_H_
#define _COMMON_SQL_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif
#include <stdarg.h>// va_list



// Return codes
#define SQL_ERROR -1
#define SQL_SUCCESS 0
#define SQL_NO_DATA 100



enum SqlDataType
{
	SQLDT_NULL,
	SQLDT_INT8,
	SQLDT_INT16,
	SQLDT_INT32,
	SQLDT_INT64,
	SQLDT_UINT8,
	SQLDT_UINT16,
	SQLDT_UINT32,
	SQLDT_UINT64,
	SQLDT_FLOAT,
	SQLDT_DOUBLE,
	SQLDT_STRING,
	SQLDT_BLOB,
	SQLDT_LASTID
};

struct Sql;// Sql handle (private access)
struct SqlStmt;// Sql statement (private access)



/// Allocates and initializes a new Sql handle.
struct Sql* Sql_Malloc(void);

/// Establishes a connection.
int Sql_Connect(struct Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db);

/// Gets the connection timeout.
int Sql_GetTimeout(struct Sql* self, uint32* out_timeout);

/// Puts the column names of a table in out_buf, with the target separator after each name.
int Sql_GetColumnNames(struct Sql* self, const char* table, char* out_buf, size_t buf_len, char sep);

/// Changes the encoding.
int Sql_SetEncoding(struct Sql* self, const char* encoding);

/// Pings the sql connection.
int Sql_Ping(struct Sql* self);

/// Escapes a string. out_to must be strlen(from)*2+1 in size
size_t Sql_EscapeString(struct Sql*, char *out_to, const char *from);

/// Escapes a string. out_to must be from_len*2+1 in size
size_t Sql_EscapeStringLen(struct Sql*, char *out_to, const char *from, size_t from_len);

/// Executes a query.
int Sql_Query(struct Sql* self, const char* query, ...);

/// Executes a query.
int Sql_QueryV(struct Sql* self, const char* query, va_list args);

/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
uint64 Sql_LastInsertId(struct Sql* self);

/// Returns the number of columns.
uint32 Sql_NumColumns(struct Sql* self);

/// Returns the number of rows.
uint64 Sql_NumRows(struct Sql* self);

/// Fetches the next row.
int Sql_NextRow(struct Sql* self);

/// Gets the data of a column.
int Sql_GetData(struct Sql* self, size_t col, char** out_buf, size_t* out_len);

/// Frees the result of the query.
void Sql_FreeResult(struct Sql* self);

/// Shows debug information (last query).
#define Sql_ShowDebug(self) Sql_ShowDebug_(self, __FILE__, __LINE__)
void Sql_ShowDebug_(struct Sql* self, const char* debug_file, const unsigned long debug_line);

/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(struct Sql* self);



/////////////////////////////////////////////////////////////////////
// Prepared Statements
/////////////////////////////////////////////////////////////////////



/// Allocates a SqlStmt.
struct SqlStmt* SqlStmt_Malloc(struct Sql* sql);

/// Prepares the statement.
int SqlStmt_Prepare(struct SqlStmt* self, const char* query, ...);

/// Prepares the statement.
int SqlStmt_PrepareV(struct SqlStmt* self, const char* query, va_list args);

/// Returns the number of parameters in the statement.
size_t SqlStmt_NumParams(struct SqlStmt* self);

/// Binds a parameter to a buffer.
int SqlStmt_BindParam(struct SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len);

/// Executes the prepared statement.
int SqlStmt_Execute(struct SqlStmt* self);

/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
uint64 SqlStmt_LastInsertId(struct SqlStmt* self);

/// Returns the number of columns in the result.
size_t SqlStmt_NumColumns(struct SqlStmt* self);

/// Binds a parameter to a buffer.
int SqlStmt_BindColumn(struct SqlStmt* self, size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null);

/// Returns the number of rows.
uint64 SqlStmt_NumRows(struct SqlStmt* self);

/// Fetches the next row.
int SqlStmt_NextRow(struct SqlStmt* self);

/// Frees the result of the last execution.
void SqlStmt_FreeResult(struct SqlStmt* self);

/// Shows debug information (with statement).
#define SqlStmt_ShowDebug(self) SqlStmt_ShowDebug_(self, __FILE__, __LINE__)
void SqlStmt_ShowDebug_(struct SqlStmt* self, const char* debug_file, const unsigned long debug_line);

/// Frees a SqlStmt returned by SqlStmt_Malloc.
void SqlStmt_Free(struct SqlStmt* self);



#endif /* _COMMON_SQL_H_ */
