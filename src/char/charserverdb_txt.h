// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSERVERDB_TXT_H_
#define _CHARSERVERDB_TXT_H_

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "charserverdb.h"



/// global defines
#define CHARSERVERDB_TXT_VERSION 20081028



typedef struct CharServerDB_TXT CharServerDB_TXT;
typedef struct CharDB_TXT CharDB_TXT;



/// internal structure
struct CharServerDB_TXT
{
	CharServerDB vtable;

	// TODO DB interfaces
	CharDB_TXT* chardb;
};

/// internal structure
struct CharDB_TXT
{
	CharDB vtable;       // public interface

	CharServerDB_TXT* owner;
	DBMap* chars;        // in-memory character storage
	int next_char_id;    // auto_increment
	int save_timer;      // save timer id

	char char_db[1024];  // character data storage file
	bool case_sensitive; // how to look up usernames
};



CharDB_TXT* char_db_txt(CharServerDB_TXT* owner);
bool char_db_txt_init(CharDB_TXT* self);
void char_db_txt_destroy(CharDB_TXT* self);



#endif /* _CHARSERVERDB_TXT_H_ */
