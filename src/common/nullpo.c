// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nullpo.h"
#include "showmsg.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/// Checks for and reports NULL pointers.
/// Used by nullpo_ret* macros.
int nullpo_chk(const char* file, int line, const char* func, const void* target, const char* targetname)
{
	if( target )
	{
		return 0;
	}

	file =
		( file == NULL ) ? "??" : file;

	func =
		( func == NULL ) ? "unknown" :
		( func[0] == 0 ) ? "unknown" : func;

	ShowDebug("--- nullpo info --------------------------------------------\n");
	ShowDebug("%s:%d: target '%s' in func '%s'\n", file, line, targetname, func);
	ShowDebug("--- end nullpo info ----------------------------------------\n");
	return 1;
}
