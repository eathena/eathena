// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "charlogdb.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct CharLogDB_TXT CharLogDB_TXT;


/// internal structure
struct CharLogDB_TXT
{
	// public interface
	CharLogDB vtable;

	// state
	bool initialized;

	// settings
	char file_charlog[256];
};


static bool charlog_db_txt_init(CharLogDB* self)
{
	CharLogDB_TXT* db = (CharLogDB_TXT*)self;
	db->initialized = true;
	return true;
}


static void charlog_db_txt_destroy(CharLogDB* self)
{
	CharLogDB_TXT* db = (CharLogDB_TXT*)self;
	aFree(db);
}


bool charlog_db_txt_get_property(CharLogDB* self, const char* key, char* buf, size_t buflen)
{
	CharLogDB_TXT* db = (CharLogDB_TXT*)self;

	if( strcmpi(key, "engine.name") == 0 )
		safesnprintf(buf, buflen, "txt");
	else
	if( strcmp(key, "charlog.txt.logfile") == 0 )
		safesnprintf(buf, buflen, "%s", db->file_charlog);
	else
		return false;

	return true;
}


bool charlog_db_txt_set_property(CharLogDB* self, const char* key, const char* value)
{
	CharLogDB_TXT* db = (CharLogDB_TXT*)self;
	
	if( strcmpi(key, "charlog.txt.logfile") == 0 )
		safestrncpy(db->file_charlog, value, sizeof(db->file_charlog));
	else
		return false;

	return true;
}


bool charlog_db_txt_log(CharLogDB* self, int char_id, int account_id, int slot, const char* name, const char* msg, va_list ap)
{
	CharLogDB_TXT* db = (CharLogDB_TXT*)self;

	char message[255+1];
	char timestamp[24+1];
	time_t now;
	FILE* log_fp;

	if( !db->initialized )
		return false;

	// open log file
	log_fp = fopen(db->file_charlog, "a");
	if( log_fp == NULL )
	{// failed
		return false;
	}

	// prepare timestamp
	time(&now);
	strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&now));

	// prepare formatted message
	vsnprintf(message, sizeof(message)-1, msg, ap);

	// write log entry
	fprintf(log_fp, "%s\t%d\t%d\t%d\t%s\t%s\n", timestamp, char_id, account_id, slot, name, message);

	// close log file
	fclose(log_fp);

	return true;
}


CharLogDB* charlog_db_txt(void)
{
	CharLogDB_TXT* db;

	CREATE(db, CharLogDB_TXT, 1);
	db->vtable.init         = charlog_db_txt_init;
	db->vtable.destroy      = charlog_db_txt_destroy;
	db->vtable.get_property = charlog_db_txt_get_property;
	db->vtable.set_property = charlog_db_txt_set_property;
	db->vtable.log          = charlog_db_txt_log;

	// initial state
	db->initialized = false;

	// default settings
	safestrncpy(db->file_charlog, "log/charlog.log", sizeof(db->file_charlog));

	return &db->vtable;
}
