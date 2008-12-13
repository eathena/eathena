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

static bool log_inter_enabled = true;
static char inter_log_filename[1024] = "log/inter.log";


/*=============================================
 * Records an event in the interserver log
 *---------------------------------------------*/
void interlog_log(const char* msg, ...)
{
	va_list ap;
	char message[255+1];
	char timestamp[24+1];
	time_t now;
	FILE* log_fp;

	if( !log_inter_enabled )
		return;

	// open log file
	log_fp = fopen(inter_log_filename, "a");
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
	fprintf(log_fp, "%s\t%s\n", timestamp, message);

	// close log file
	fclose(log_fp);
}


bool interlog_init(void)
{
	return true;
}


bool interlog_final(void)
{
	return true;
}


bool interlog_config_read(const char* key, const char* value)
{
	if( strcmpi(key, "log_inter") == 0 )
		log_inter_enabled = atoi(value);
	else
	if( strcmpi(key, "inter_log_filename") == 0 )
		safestrncpy(inter_log_filename, value, sizeof(inter_log_filename));
	else
		return false;

	return true;
}
