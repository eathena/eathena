// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "char.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char char_log_filename[1024] = "log/char.log";



/*=============================================
 * Records an event in the char log
 *---------------------------------------------*/
void char_log(char *fmt, ...)
{
	FILE* log_fp;

	if( !log_char )
		return;

	log_fp = fopen(char_log_filename, "a");
	if( log_fp != NULL )
	{
		fclose(log_fp);
	}
}


bool charlog_config_read(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "char_log_filename"))
		safestrncpy(char_log_filename, w2, sizeof(char_log_filename));
	else
		return false;

	return true;
}


bool charlog_init(void)
{
	return true;
}


bool charlog_final(void)
{
	return true;
}
