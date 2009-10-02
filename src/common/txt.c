// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/strlib.h"
#include "txt.h"
#include <stdio.h>
#include <stdlib.h>


struct Txt;
struct s_bind;


/// Txt handle
struct Txt
{
	char* str;           // input/output buffer
	size_t length;       // size of buffer (zero byte included)
	char delim;          // delimiter character to use when reading/writing
	char escapes[8+1];   // extra characters to escape when writing
	size_t nfields;      // maximum number of fields to read/write
	struct s_bind* bind; // array of variables bound to fields
};


struct s_bind
{
	void* buffer;
	size_t buffer_len;
	TxtDataType buffer_type;
};


/// Allocates and initializes a new Txt handle.
///
/// @return new Txt handle or NULL
Txt* Txt_Malloc(void)
{
	Txt* self = (Txt*)aMalloc(sizeof(Txt));
	self->str = NULL;
	self->length = 0;
	self->delim = '\0';
	self->escapes[0] = '\0';
	self->nfields = 0;
	self->bind = NULL;

	return self;
}


/// Records parameters which will be used during subsequent processing.
///
/// @param self Txt handle
/// @param str String to be parsed/written
/// @param length Size (for parsing) or capacity (for writing) of the string
/// @param nfields Maximum number of fields to read/write
/// @param delim Delimiter between fields
/// @param escapes Characters to escape when writing fields (the delimiter should be present here)
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Init(Txt* self, char* str, size_t length, size_t nfields, char delim, const char* escapes)
{
	if( self == NULL )
		return TXT_ERROR;

	self->str = str;
	self->length = length;
	self->delim = delim;
	safestrncpy(self->escapes, escapes, sizeof(self->escapes));
	self->nfields = nfields;
	if( self->bind )
		aFree(self->bind);
	self->bind = (struct s_bind*)aCalloc(nfields, sizeof(*self->bind));

	return TXT_SUCCESS;
}


/// Binds a variable to the specified field.
///
/// @param self Txt handle
/// @param idx Field's ordinal (starting from 0)
/// @param buffer_type Type of variable
/// @param buffer Address of variable
/// @param buffer_len Size of variable
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Bind(Txt* self, size_t idx, TxtDataType buffer_type, void* buffer, size_t buffer_len)
{
	if( self == NULL )
		return TXT_ERROR;

	if( idx >= self->nfields )
		return TXT_ERROR;

	// verify that buffer_len matches the buffer_type
	switch( buffer_type )
	{
	case TXTDT_NULL     : if( buffer_len != 0 ) return TXT_ERROR; break;
	// platform dependent size
	case TXTDT_CHAR     : if( buffer_len != sizeof(char) ) return TXT_ERROR; break;
	case TXTDT_SHORT    : if( buffer_len != sizeof(short) ) return TXT_ERROR; break;
	case TXTDT_INT      : if( buffer_len != sizeof(int) ) return TXT_ERROR; break;
	case TXTDT_LONG     : if( buffer_len != sizeof(long) ) return TXT_ERROR; break;
	case TXTDT_UCHAR    : if( buffer_len != sizeof(unsigned char) ) return TXT_ERROR; break;
	case TXTDT_USHORT   : if( buffer_len != sizeof(unsigned short) ) return TXT_ERROR; break;
	case TXTDT_UINT     : if( buffer_len != sizeof(unsigned int) ) return TXT_ERROR; break;
	case TXTDT_ULONG    : if( buffer_len != sizeof(unsigned long) ) return TXT_ERROR; break;
	// other
	case TXTDT_STRING   : break;
	case TXTDT_CSTRING  : break;
	
	default:
		return TXT_ERROR;
	}

	self->bind[idx].buffer = buffer;
	self->bind[idx].buffer_len = buffer_len;
	self->bind[idx].buffer_type = buffer_type;

	return TXT_SUCCESS;
}


/// Parses the string and writes values to bound variables.
///
/// @param self Txt handle
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Parse(Txt* self)
{
	int fields[1+1][2];
	int nfields;
	e_svopt mode;
	const char* str;
	size_t i = 0;

	if( self == NULL )
		return TXT_ERROR;

	str = self->str;

	//FIXME: O(n^2), inefficient
	do
	{
		mode = ( self->bind && self->bind[i].buffer_type != TXTDT_STRING ) ? SV_ESCAPE_C : SV_NOESCAPE_NOTERMINATE;
		nfields = sv_parse(str, self->length, 0, self->delim, (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(mode|SV_TERMINATE_LF|SV_TERMINATE_CRLF));

		if( self->bind )
		{
			switch( self->bind[i].buffer_type )
			{
			//TODO: detect integer overflow
			//TODO: strntol()
			case TXTDT_NULL     : break;
			case TXTDT_CHAR     : *(char*)self->bind[i].buffer = (char)strtol(str, NULL, 0); break;
			case TXTDT_SHORT    : *(short*)self->bind[i].buffer = (short)strtol(str, NULL, 0); break;
			case TXTDT_INT      : *(int*)self->bind[i].buffer = (int)strtol(str, NULL, 0); break;
			case TXTDT_LONG     : *(long*)self->bind[i].buffer = (long)strtol(str, NULL, 0); break;
			case TXTDT_UCHAR    : *(unsigned char*)self->bind[i].buffer = (unsigned char)strtoul(str, NULL, 0); break;
			case TXTDT_USHORT   : *(unsigned short*)self->bind[i].buffer = (unsigned short)strtoul(str, NULL, 0); break;
			case TXTDT_UINT     : *(unsigned int*)self->bind[i].buffer = (unsigned int)strtoul(str, NULL, 0); break;
			case TXTDT_ULONG    : *(unsigned long*)self->bind[i].buffer = (unsigned long)strtoul(str, NULL, 0); break;
			case TXTDT_STRING   : safestrncpy((char*)self->bind[i].buffer, str, min(self->bind[i].buffer_len, (size_t)(fields[1][1] - fields[1][0] + 1))); break;
			case TXTDT_CSTRING  : sv_unescape_c((char*)self->bind[i].buffer, str, min(self->bind[i].buffer_len - 1, (size_t)(fields[1][1] - fields[1][0]))); break;
			default:
				return TXT_ERROR;
			}
		}

		str += fields[1][1] + 1;
		i++;
	}
	while( nfields >= 2 );

	return TXT_SUCCESS;
}


/// Constructs the string using values from bound variables.
///
/// @param self Txt handle
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Write(Txt* self)
{
	char* str;
	size_t i;

	if( self == NULL )
		return TXT_ERROR;

	str = self->str;

	//TODO: output string capacity checking

	for( i = 0; i < self->nfields; ++i )
	{
		if( i != 0 )
			str += sprintf(str, "%c", self->delim);

		switch( self->bind[i].buffer_type )
		{
		case TXTDT_NULL   : break;
		case TXTDT_CHAR   : str += sprintf(str, "%d", (int)*(char*)self->bind[i].buffer); break;
		case TXTDT_SHORT  : str += sprintf(str, "%d", (int)*(short*)self->bind[i].buffer); break;
		case TXTDT_INT    : str += sprintf(str, "%d", (int)*(int*)self->bind[i].buffer); break;
		case TXTDT_LONG   : str += sprintf(str, "%ld", (long)*(long*)self->bind[i].buffer); break;
		case TXTDT_UCHAR  : str += sprintf(str, "%u", (unsigned int)*(unsigned char*)self->bind[i].buffer); break;
		case TXTDT_USHORT : str += sprintf(str, "%u", (unsigned int)*(unsigned short*)self->bind[i].buffer); break;
		case TXTDT_UINT   : str += sprintf(str, "%u", (unsigned int)*(unsigned int*)self->bind[i].buffer); break;
		case TXTDT_ULONG  : str += sprintf(str, "%lu", (unsigned long)*(unsigned long*)self->bind[i].buffer); break;
		case TXTDT_STRING : str += sprintf(str, "%s", (char*)self->bind[i].buffer); break;
		case TXTDT_CSTRING: str += sv_escape_c(str, (char*)self->bind[i].buffer, self->bind[i].buffer_len, self->escapes); break;
		default:
			return TXT_ERROR;
		}
	}

	return TXT_SUCCESS;
}


/// Frees a Txt handle returned by Txt_Malloc().
///
/// @param self Txt handle
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Free(Txt* self)
{
	if( self && self->bind )
		aFree(self->bind);

	return TXT_SUCCESS;
}
