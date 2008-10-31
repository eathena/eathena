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



/// internal structure
struct CharServerDB_TXT
{
	CharServerDB vtable;

	// TODO DB interfaces
	CharDB* chardb;
};



CharDB* char_db_txt(CharServerDB_TXT* owner);



#endif /* _CHARSERVERDB_TXT_H_ */
