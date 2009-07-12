// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __CHARLOGDB_H_INCLUDED__
#define __CHARLOGDB_H_INCLUDED__

#include "../common/cbasetypes.h"
#include <stdarg.h>

typedef struct CharLogDB CharLogDB;


struct CharLogDB
{
	bool (*init)(CharLogDB* self);
	void (*destroy)(CharLogDB* self);
	bool (*get_property)(CharLogDB* self, const char* key, char* buf, size_t buflen);
	bool (*set_property)(CharLogDB* self, const char* key, const char* value);
	bool (*log)(CharLogDB* self, int char_id, int account_id, int slot, const char* name, const char* msg, va_list ap);
};


// standard engines
#ifdef WITH_TXT
CharLogDB* charlog_db_txt(void);
#endif
#ifdef WITH_SQL
CharLogDB* charlog_db_sql(void);
#endif


#endif // __CHARLOGDB_H_INCLUDED__
