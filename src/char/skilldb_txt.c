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
#include "../common/utils.h"
#include "charserverdb_txt.h"
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

	// state
	CharServerDB_TXT* owner;
	DBMap* skills;
	bool dirty;

	// settings
	const char* skill_db;

} SkillDB_TXT;


/// @private
static void* create_skilllist(DBKey key, va_list args)
{
	return (skilllist*)aMalloc(sizeof(skilllist));
}


/// @private
static bool mmo_skilllist_fromstr(skilllist* list, char* str)
{
	const char* p = str;
	bool first = true;
	int n = 0;
	int i;

	memset(list, 0, sizeof(*list));

	for( i = 0; *p != '\0' && *p != '\n' && *p != '\r'; ++i )
	{
		int tmp_int[2];

		if( first )
			first = false;
		else
		if( *p == ' ' )
			p++;
		else
			return false;

		if( sscanf(p, "%d,%d%n", &tmp_int[0], &tmp_int[1], &n) != 2 )
			return false;

		p += n;

		if( i == MAX_SKILL )
			continue; // TODO: warning?

		(*list)[i].id = tmp_int[0];
		(*list)[i].lv = tmp_int[1];
	}

	return true;
}


/// @private
static bool mmo_skilllist_tostr(const skilllist* list, char* str)
{
	char* p = str;
	bool first = true;
	int i;

	for( i = 0; i < MAX_SKILL; ++i )
	{
		if( (*list)[i].id == 0 || (*list)[i].flag == 1 )
			continue;

		if( first )
			first = false;
		else
			p += sprintf(p, " ");

		p += sprintf(p, "%d,%d", (*list)[i].id, ((*list)[i].flag == 0) ? (*list)[i].lv : (*list)[i].flag-2);
	}

	*p = '\0';

	return true;
}


/// @protected
static bool skill_db_txt_init(SkillDB* self)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBMap* skills;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create skill database
	if( db->skills == NULL )
		db->skills = idb_alloc(DB_OPT_RELEASE_DATA);
	skills = db->skills;
	db_clear(skills);

	// open data file
	fp = fopen(db->skill_db, "r");
	if( fp == NULL )
	{
		ShowError("Skill file not found: %s.\n", db->skill_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
		int char_id, n;
		unsigned int v;
		skilllist* list;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		list = (skilllist*)aCalloc(1, sizeof(skilllist));
		if( list == NULL )
		{
			ShowFatalError("skill_db_txt_init: out of skillry!\n");
			exit(EXIT_FAILURE);
		}

		// load char id
		n = 0;
		if( sscanf(line, "%d%n\t", &char_id, &n) != 1 || line[n] != '\t' )
		{
			aFree(list);
			continue;
		}

		// load skills for this char
		if( !mmo_skilllist_fromstr(list, line + n + 1) )
		{
			ShowError("skill_db_txt_init: skipping invalid data: %s", line);
			aFree(list);
			continue;
		}
	
		// record entry in db
		idb_put(skills, char_id, list);
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void skill_db_txt_destroy(SkillDB* self)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBMap* skills = db->skills;

	// delete skill database
	if( skills != NULL )
	{
		db_destroy(skills);
		db->skills = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool skill_db_txt_sync(SkillDB* self, bool force)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->skill_db, &lock);
	if( fp == NULL )
	{
		ShowError("skill_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->skill_db);
		return false;
	}

	fprintf(fp, "%d\n", SKILLDB_TXT_DB_VERSION);

	iter = db->skills->iterator(db->skills);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		int char_id = key.i;
		skilllist* list = (skilllist*) data;
		char line[8192];

		mmo_skilllist_tostr(list, line);
		fprintf(fp, "%d\t%s\n", char_id, line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->skill_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool skill_db_txt_remove(SkillDB* self, const int char_id)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBMap* skills = db->skills;

	idb_remove(skills, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool skill_db_txt_save(SkillDB* self, const skilllist* list, const int char_id)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBMap* skills = db->skills;

	// retrieve previous data / allocate new data
	skilllist* tmp = idb_ensure(skills, char_id, create_skilllist);
	if( tmp == NULL )
	{// error condition - allocation problem?
		return false;
	}

	// overwrite with new data
	memcpy(tmp, list, sizeof(skilllist));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool skill_db_txt_load(SkillDB* self, skilllist* list, const int char_id)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	DBMap* skills = db->skills;

	skilllist* tmp = idb_get(skills, char_id);

	if( tmp != NULL )
		memcpy(list, tmp, sizeof(skilllist));
	else
		memset(list, 0, sizeof(skilllist));

	return true;
}


/// Returns an iterator over all skill lists.
/// @protected
static CSDBIterator* skill_db_txt_iterator(SkillDB* self)
{
	SkillDB_TXT* db = (SkillDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->skills));
}


/// Constructs a new SkillDB interface.
/// @protected
SkillDB* skill_db_txt(CharServerDB_TXT* owner)
{
	SkillDB_TXT* db = (SkillDB_TXT*)aCalloc(1, sizeof(SkillDB_TXT));

	// set up the vtable
	db->vtable.p.init      = &skill_db_txt_init;
	db->vtable.p.destroy   = &skill_db_txt_destroy;
	db->vtable.p.sync      = &skill_db_txt_sync;
	db->vtable.remove    = &skill_db_txt_remove;
	db->vtable.save      = &skill_db_txt_save;
	db->vtable.load      = &skill_db_txt_load;
	db->vtable.iterator  = &skill_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->skills = NULL;
	db->dirty = true;

	// other settings
	db->skill_db = db->owner->file_skills;

	return &db->vtable;
}
