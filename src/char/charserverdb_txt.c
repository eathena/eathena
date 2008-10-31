// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"

#include <string.h>
#include <stdlib.h>



/// Initializes this database engine, making it ready for use.
static bool charserver_db_txt_init(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	// TODO DB interfaces
	return char_db_txt_init(db->chardb);
}



/// Destroys this database engine, releasing all allocated memory (including itself).
static void charserver_db_txt_destroy(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	// TODO DB interfaces
	char_db_txt_destroy(db->chardb);
	db->chardb = NULL;
	aFree(db);
}



/// Gets a property from this database engine.
static bool charserver_db_txt_get_property(CharServerDB* self, const char* key, char* buf, size_t buflen)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;
	const char* signature;

	signature = "engine.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "name") == 0 )
			safesnprintf(buf, buflen, "txt");
		else
		if( strcmpi(key, "version") == 0 )
			safesnprintf(buf, buflen, "%d", CHARSERVERDB_TXT_VERSION);
		else
		if( strcmpi(key, "comment") == 0 )
			safesnprintf(buf, buflen, "CharServerDB TXT engine");
		else
			return false;// not found
		return true;
	}

	signature = "txt.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		return false;// not found
	}

	// TODO DB interface properties

	return false;// not found
}



/// Sets a property in this database engine.
static bool charserver_db_txt_set_property(CharServerDB* self, const char* key, const char* value)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;
	const char* signature;


	signature = "txt.";
	if( strncmp(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		return false;// not found
	}

	// TODO DB interface properties

	return false;// not found
}



/// TODO
static CharDB* charserver_db_txt_chardb(CharServerDB* self)
{
	CharServerDB_TXT* db = (CharServerDB_TXT*)self;

	return &db->chardb;
}



/// constructor
CharServerDB* charserver_db_txt(void)
{
	CharServerDB_TXT* db;

	CREATE(db, CharServerDB_TXT, 1);
	db->vtable.init         = charserver_db_txt_init;
	db->vtable.destroy      = charserver_db_txt_destroy;
	db->vtable.get_property = charserver_db_txt_get_property;
	db->vtable.set_property = charserver_db_txt_set_property;
	db->vtable.chardb       = charserver_db_txt_chardb;
	// TODO DB interfaces

	db->chardb = char_db_txt(db);
	// initialize to default values
	// other settings

	return &db->vtable;
}
