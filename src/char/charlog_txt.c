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

static bool log_char_enabled = true;
static char char_log_filename[1024] = "log/char.log";


/*=============================================
 * Records an event in the character log
 *---------------------------------------------*/
void charlog_log(int char_id, int account_id, int slot, const char* name, const char* msg, ...)
{
	va_list ap;
	char message[255+1];
	char timestamp[24+1];
	time_t now;
	FILE* log_fp;

	if( !log_char_enabled )
		return;

	// open log file
	log_fp = fopen(char_log_filename, "a");
	if( log_fp == NULL )
	{// failed
		return;
	}

	// prepare timestamp
	time(&now);
	strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&now));

	// prepare formatted message
	va_start(ap, msg);
	vsnprintf(message, sizeof(message)-1, msg, ap);
	va_end(ap);

	// write log entry
	fprintf(log_fp, "%s\t%d\t%d\t%d\t%s\t%s\n", timestamp, char_id, account_id, slot, name, message);

	// close log file
	fclose(log_fp);
}


bool charlog_init(void)
{
	return true;
}


bool charlog_final(void)
{
	return true;
}


bool charlog_config_read(const char* key, const char* value)
{
	if( strcmpi(key, "log_char") == 0 )
		log_char_enabled = atoi(value);
	else
	if( strcmpi(key, "char_log_filename") == 0 )
		safestrncpy(char_log_filename, value, sizeof(char_log_filename));
	else
		return false;

	return true;
}
