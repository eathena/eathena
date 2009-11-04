// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/strlib.h"
#include "txt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Txt;
struct s_bind;


/// Txt handle
struct Txt
{
	char* str;           // input/output buffer
	size_t length;       // size of buffer (zero byte included)
	char field_delim;    // field delimiter to use when parsing/writing
	char block_delim;    // block delimiter to use when parsing/writing
	char escapes[8+1];   // extra characters to escape when writing
	size_t max_fields;   // maximum number of fields to parse/write
	struct s_bind* bind; // array of variables bound to fields
	size_t nfields;      // number of fields in string (determined during processing)
	size_t cursor;       // points to the spot after the last successfully processed byte
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
	self->field_delim = '\0';
	self->block_delim = '\0';
	self->escapes[0] = '\0';
	self->max_fields = 0;
	self->bind = NULL;
	self->nfields = 0;
	self->cursor = 0;

	return self;
}


/// Records parameters which will be used during subsequent processing.
///
/// @param self Txt handle
/// @param str String to be parsed/written
/// @param length Size (for parsing) or capacity (for writing) of the string
/// @param max_fields Maximum number of fields to parse/write
/// @param field_delim Delimiter between fields
/// @param block_delim Delimiter between blocks of fields
/// @param escapes Characters to escape when writing fields (all delimiters should be present here)
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Init(Txt* self, char* str, size_t length, size_t max_fields, char field_delim, char block_delim, const char* escapes)
{
	if( self == NULL )
		return TXT_ERROR;

	if( field_delim == block_delim )
		return TXT_ERROR; // not supported

	self->str = str;
	self->length = length;
	self->field_delim = field_delim;
	self->block_delim = block_delim;
	safestrncpy(self->escapes, escapes, sizeof(self->escapes));
	self->max_fields = max_fields;
	if( self->bind )
		aFree(self->bind);
	self->bind = (struct s_bind*)aCalloc(max_fields, sizeof(*self->bind));
	self->nfields = 0;
	self->cursor = 0;

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

	if( idx >= self->max_fields )
		return TXT_ERROR;

	// verify that buffer_len matches the buffer_type
	switch( buffer_type )
	{
	case TXTDT_NULL     : if( buffer != NULL || buffer_len != 0 ) return TXT_ERROR; break;
	// platform dependent size
	case TXTDT_BOOL     : if( buffer == NULL || buffer_len != sizeof(bool) ) return TXT_ERROR; break;
	case TXTDT_CHAR     : if( buffer == NULL || buffer_len != sizeof(char) ) return TXT_ERROR; break;
	case TXTDT_SHORT    : if( buffer == NULL || buffer_len != sizeof(short) ) return TXT_ERROR; break;
	case TXTDT_INT      : if( buffer == NULL || buffer_len != sizeof(int) ) return TXT_ERROR; break;
	case TXTDT_LONG     : if( buffer == NULL || buffer_len != sizeof(long) ) return TXT_ERROR; break;
	case TXTDT_UCHAR    : if( buffer == NULL || buffer_len != sizeof(unsigned char) ) return TXT_ERROR; break;
	case TXTDT_USHORT   : if( buffer == NULL || buffer_len != sizeof(unsigned short) ) return TXT_ERROR; break;
	case TXTDT_UINT     : if( buffer == NULL || buffer_len != sizeof(unsigned int) ) return TXT_ERROR; break;
	case TXTDT_ULONG    : if( buffer == NULL || buffer_len != sizeof(unsigned long) ) return TXT_ERROR; break;
	case TXTDT_TIME     : if( buffer == NULL || buffer_len != sizeof(time_t) ) return TXT_ERROR; break;
	// other
	case TXTDT_STRING   : if( buffer == NULL ) return TXT_ERROR; break; // TODO: verify that there are no delimiters inside this string
	case TXTDT_CSTRING  : if( buffer == NULL ) return TXT_ERROR; break;
	case TXTDT_ENUM     : if( buffer == NULL || buffer_len > sizeof(long) ) return TXT_ERROR; break; // only up to sizeof(long) for now
	case TXTDT_UENUM    : if( buffer == NULL || buffer_len > sizeof(unsigned long) ) return TXT_ERROR; break; // only up to sizeof(unsigned long) for now
	
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
	struct s_svstate sv;
	size_t i;

	if( self == NULL )
		return TXT_ERROR;

	//TODO: handle the case when self->block_delim is '\0'

	// set up block-level parsing
	sv.str = self->str;
	sv.len = self->length;
	sv.off = self->cursor;
	sv.opt = SV_ESCAPE_C | SV_TERMINATE_LF | SV_TERMINATE_CRLF;
	sv.delim = self->block_delim;
	sv.done = false;

	if( sv.off > 0 )
	{
		if( sv.str[sv.off] == '\0' || sv.str[sv.off] == '\r' || sv.str[sv.off] == '\n' )
		{// already processed all blocks
			self->nfields = 0;
			return TXT_SUCCESS;
		}
		else
		if( sv.str[sv.off] == sv.delim )
			sv.off++; // step to next block
	}

	// locate the start and end of this block
	if( sv_parse_next(&sv) <= 0 )
		return TXT_ERROR;

	// set up field-level parsing
	sv.str = self->str;
	sv.len = sv.end;
	sv.off = sv.start;
	sv.opt = SV_NOESCAPE_NOTERMINATE;
	sv.delim = self->field_delim;
	sv.done = false;

	// parse fields in this block
	i = 0;
	while( !sv.done )
	{
		sv.opt = ( self->bind && self->bind[i].buffer_type != TXTDT_STRING ) ? SV_ESCAPE_C : SV_NOESCAPE_NOTERMINATE;

		if( sv_parse_next(&sv) <= 0 )
			return TXT_ERROR;

		if( self->bind && i < self->max_fields )
		{
			const char* str = &self->str[sv.start];
			size_t size = sv.end - sv.start + 1;

			switch( self->bind[i].buffer_type )
			{
			//TODO: detect integer overflow/truncation
			//TODO: strntol()
			case TXTDT_NULL     : break;
			case TXTDT_BOOL     : *(bool*)self->bind[i].buffer = (bool)strtol(str, NULL, 0); break;
			case TXTDT_CHAR     : *(char*)self->bind[i].buffer = (char)strtol(str, NULL, 0); break;
			case TXTDT_SHORT    : *(short*)self->bind[i].buffer = (short)strtol(str, NULL, 0); break;
			case TXTDT_INT      : *(int*)self->bind[i].buffer = (int)strtol(str, NULL, 0); break;
			case TXTDT_LONG     : *(long*)self->bind[i].buffer = (long)strtol(str, NULL, 0); break;
			case TXTDT_UCHAR    : *(unsigned char*)self->bind[i].buffer = (unsigned char)strtoul(str, NULL, 0); break;
			case TXTDT_USHORT   : *(unsigned short*)self->bind[i].buffer = (unsigned short)strtoul(str, NULL, 0); break;
			case TXTDT_UINT     : *(unsigned int*)self->bind[i].buffer = (unsigned int)strtoul(str, NULL, 0); break;
			case TXTDT_ULONG    : *(unsigned long*)self->bind[i].buffer = (unsigned long)strtoul(str, NULL, 0); break;
			case TXTDT_TIME     : *(time_t*)self->bind[i].buffer = (time_t)strtoul(str, NULL, 0); break; // only unsigned long for now
			case TXTDT_STRING   : safestrncpy((char*)self->bind[i].buffer, str, min(self->bind[i].buffer_len, size)); break;
			case TXTDT_CSTRING  : sv_unescape_c((char*)self->bind[i].buffer, str, min(self->bind[i].buffer_len - 1, size - 1)); break;
			case TXTDT_ENUM     : { signed long   tmp = strtol (str, NULL, 0); memcpy(self->bind[i].buffer, &tmp, self->bind[i].buffer_len); }; break; // only up to sizeof(long) for now
			case TXTDT_UENUM    : { unsigned long tmp = strtoul(str, NULL, 0); memcpy(self->bind[i].buffer, &tmp, self->bind[i].buffer_len); }; break; // only up to sizeof(unsigned long) for now
			default:
				return TXT_ERROR;
			}
		}

		i++;
	}

	self->nfields = i;
	self->cursor = sv.end;

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

	str = self->str + self->cursor;

	if( self->cursor > 0 )
		str += sprintf(str, "%c", self->block_delim);

	//TODO: handle the case when self->block_delim is '\0'
	//TODO: output string capacity checking

	self->nfields = 0;

	for( i = 0; i < self->max_fields; ++i )
	{
		if( i != 0 )
			str += sprintf(str, "%c", self->field_delim);

		switch( self->bind[i].buffer_type )
		{
		case TXTDT_NULL   : break;
		case TXTDT_BOOL   : str += sprintf(str, "%d", (int)*(bool*)self->bind[i].buffer); break;
		case TXTDT_CHAR   : str += sprintf(str, "%d", (int)*(char*)self->bind[i].buffer); break;
		case TXTDT_SHORT  : str += sprintf(str, "%d", (int)*(short*)self->bind[i].buffer); break;
		case TXTDT_INT    : str += sprintf(str, "%d", (int)*(int*)self->bind[i].buffer); break;
		case TXTDT_LONG   : str += sprintf(str, "%ld", (long)*(long*)self->bind[i].buffer); break;
		case TXTDT_UCHAR  : str += sprintf(str, "%u", (unsigned int)*(unsigned char*)self->bind[i].buffer); break;
		case TXTDT_USHORT : str += sprintf(str, "%u", (unsigned int)*(unsigned short*)self->bind[i].buffer); break;
		case TXTDT_UINT   : str += sprintf(str, "%u", (unsigned int)*(unsigned int*)self->bind[i].buffer); break;
		case TXTDT_ULONG  : str += sprintf(str, "%lu", (unsigned long)*(unsigned long*)self->bind[i].buffer); break;
		case TXTDT_TIME   : str += sprintf(str, "%lu", (unsigned long)*(time_t*)self->bind[i].buffer); break; // unsigned long for now
		case TXTDT_STRING : str += sprintf(str, "%s", (char*)self->bind[i].buffer); break; //TODO: buffer length check
		case TXTDT_CSTRING: str += sv_escape_c(str, (char*)self->bind[i].buffer, strnlen((char*)self->bind[i].buffer, self->bind[i].buffer_len - 1), self->escapes); break;
		default:
			self->str[self->cursor] = '\0'; // restore original string
			return TXT_ERROR;
		}

		self->nfields++;
	}

	self->cursor = (size_t)(str - self->str);

	return TXT_SUCCESS;
}


/// Number of fields in the string.
/// Determined during Parse()/Write().
///
/// @param self Txt handle
/// @return Number of fields, or 0 if no procsesing has been done yet
size_t Txt_NumFields(Txt* self)
{
	return self->nfields;
}


/// Frees a Txt handle returned by Txt_Malloc().
///
/// @param self Txt handle
/// @return TXT_SUCCESS or TXT_ERROR
int Txt_Free(Txt* self)
{
	if( self )
	{
		if( self->bind )
		aFree(self->bind);

		aFree(self);
	}

	return TXT_SUCCESS;
}
