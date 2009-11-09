// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/txt.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "skilldb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define SKILLDB_TXT_DB_VERSION 20090825


/// Internal structure.
/// @private
typedef struct SkillDB_TXT
{
	// public interface
	SkillDB vtable;

	// data provider
	CSDB_TXT* db;

} SkillDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool skill_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	skilllist* list = (skilllist*)data;

	*out_size = sizeof(*list);

	if( size < sizeof(*list) )
		return true;

	if( version == 20090825 )
	{
		Txt* txt;
		int char_id, n;
		int skill_id, skill_lv;
		bool done = false;

		memset(list, 0, sizeof(*list));

		// load char id
		if( sscanf(str, "%d%n", &char_id, &n) != 1 || str[n] != '\t' )
			return false;

		str += n + 1;

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 2, ',', ' ', "");
		Txt_Bind(txt, 0, TXTDT_INT, &skill_id, sizeof(skill_id));
		Txt_Bind(txt, 1, TXTDT_INT, &skill_lv, sizeof(skill_lv));

		while( Txt_Parse(txt) == TXT_SUCCESS )
		{
			if( Txt_NumFields(txt) == 0 )
			{// no more data
				done = true;
				break;
			}

			if( Txt_NumFields(txt) != 2 )
				break; // parsing failure

			if( skill_id >= MAX_SKILL )
				continue; // TODO: warning?

			(*list)[skill_id].id = skill_id;
			(*list)[skill_id].lv = skill_lv;
		}

		Txt_Free(txt);

		if( !done )
			return false;

		*key = char_id;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @private
static bool skill_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	char* p = str;
	int char_id = key;
	skilllist* list = (skilllist*)data;
	int i;
	int count = 0;
	bool first = true;

	if( size != sizeof(*list) )
		return false;

	// write char id
	p += sprintf(p, "%d\t", char_id);

	// write friend list for this char
	for( i = 0; i < MAX_SKILL; i++ )
	{
		if( (*list)[i].id == 0 || (*list)[i].flag == 1 )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, " ");

		p += sprintf(p, "%d,%d", (*list)[i].id, ((*list)[i].flag == 0) ? (*list)[i].lv : (*list)[i].flag-2);

		count++;
	}

	if( count == 0 )
		str[0] = '\0';

	return true;
}


/// @protected
static bool skill_db_txt_init(SkillDB* self)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void skill_db_txt_destroy(SkillDB* self)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool skill_db_txt_sync(SkillDB* self, bool force)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool skill_db_txt_remove(SkillDB* self, const int char_id)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	return db->remove(db, char_id);
}


/// @protected
static bool skill_db_txt_save(SkillDB* self, const skilllist* list, const int char_id)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	return db->replace(db, char_id, list, sizeof(*list));
}


/// @protected
static bool skill_db_txt_load(SkillDB* self, skilllist* list, const int char_id)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;

	if( !db->load(db, char_id, list, sizeof(*list), NULL) )
		memset(list, 0, sizeof(*list));

	return true;
}


/// Returns an iterator over all skill lists.
/// @protected
static CSDBIterator* skill_db_txt_iterator(SkillDB* self)
{
	CSDB_TXT* db = ((SkillDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new SkillDB interface.
/// @protected
SkillDB* skill_db_txt(CharServerDB_TXT* owner)
{
	SkillDB_TXT* db = (SkillDB_TXT*)aCalloc(1, sizeof(SkillDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_skills, SKILLDB_TXT_DB_VERSION, 0);
	db->db->p.fromstr = &skill_db_txt_fromstr;
	db->db->p.tostr   = &skill_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &skill_db_txt_init;
	db->vtable.p.destroy = &skill_db_txt_destroy;
	db->vtable.p.sync    = &skill_db_txt_sync;
	db->vtable.remove    = &skill_db_txt_remove;
	db->vtable.save      = &skill_db_txt_save;
	db->vtable.load      = &skill_db_txt_load;
	db->vtable.iterator  = &skill_db_txt_iterator;

	return &db->vtable;
}
