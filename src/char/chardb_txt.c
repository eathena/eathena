// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/txt.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "char.h"
#include "inter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define CHARDB_TXT_DB_VERSION 20090825
#define START_CHAR_NUM 1


/// Internal structure.
/// @private
typedef struct CharDB_TXT
{
	// public interface
	CharDB vtable;

	// data provider
	CSDB_TXT* db;

	// indexes
	DBMap* idx_name;// char* name -> int char_id (case-sensitive)

} CharDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool char_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	const char* p = str;
	struct mmo_charstatus* cd = (struct mmo_charstatus*)data;
	char last_map[MAP_NAME_LENGTH], save_map[MAP_NAME_LENGTH];
	int n;
	Txt* txt;

	*out_size = sizeof(*cd);

	if( size < sizeof(*cd) )
		return true;

	memset(cd, 0, sizeof(*cd));

	if( version != CHARDB_TXT_DB_VERSION )
		return false;

	// key (char id)
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	p += n + 1;

	// base data
	txt = Txt_Malloc();
	Txt_Init(txt, (char*)p, strlen(p), 53, ',', '\0', "");
	Txt_Bind(txt,  0, TXTDT_INT, &cd->account_id, sizeof(cd->account_id));
	Txt_Bind(txt,  1, TXTDT_UCHAR, &cd->slot, sizeof(cd->slot));
	Txt_Bind(txt,  2, TXTDT_CSTRING, &cd->name, sizeof(cd->name));
	Txt_Bind(txt,  3, TXTDT_SHORT, &cd->class_, sizeof(cd->class_));
	Txt_Bind(txt,  4, TXTDT_UINT, &cd->base_level, sizeof(cd->base_level));
	Txt_Bind(txt,  5, TXTDT_UINT, &cd->job_level, sizeof(cd->job_level));
	Txt_Bind(txt,  6, TXTDT_UINT, &cd->base_exp, sizeof(cd->base_exp));
	Txt_Bind(txt,  7, TXTDT_UINT, &cd->job_exp, sizeof(cd->job_exp));
	Txt_Bind(txt,  8, TXTDT_INT, &cd->zeny, sizeof(cd->zeny));
	Txt_Bind(txt,  9, TXTDT_INT, &cd->hp, sizeof(cd->hp));
	Txt_Bind(txt, 10, TXTDT_INT, &cd->max_hp, sizeof(cd->max_hp));
	Txt_Bind(txt, 11, TXTDT_INT, &cd->sp, sizeof(cd->sp));
	Txt_Bind(txt, 12, TXTDT_INT, &cd->max_sp, sizeof(cd->max_sp));
	Txt_Bind(txt, 13, TXTDT_SHORT, &cd->str, sizeof(cd->str));
	Txt_Bind(txt, 14, TXTDT_SHORT, &cd->agi, sizeof(cd->agi));
	Txt_Bind(txt, 15, TXTDT_SHORT, &cd->vit, sizeof(cd->vit));
	Txt_Bind(txt, 16, TXTDT_SHORT, &cd->int_, sizeof(cd->int_));
	Txt_Bind(txt, 17, TXTDT_SHORT, &cd->dex, sizeof(cd->dex));
	Txt_Bind(txt, 18, TXTDT_SHORT, &cd->luk, sizeof(cd->luk));
	Txt_Bind(txt, 19, TXTDT_UINT, &cd->status_point, sizeof(cd->status_point));
	Txt_Bind(txt, 20, TXTDT_UINT, &cd->skill_point, sizeof(cd->skill_point));
	Txt_Bind(txt, 21, TXTDT_UINT, &cd->option, sizeof(cd->option));
	Txt_Bind(txt, 22, TXTDT_UCHAR, &cd->karma, sizeof(cd->karma));
	Txt_Bind(txt, 23, TXTDT_SHORT, &cd->manner, sizeof(cd->manner));
	Txt_Bind(txt, 24, TXTDT_INT, &cd->party_id, sizeof(cd->party_id));
	Txt_Bind(txt, 25, TXTDT_INT, &cd->guild_id, sizeof(cd->guild_id));
	Txt_Bind(txt, 26, TXTDT_INT, &cd->pet_id, sizeof(cd->pet_id));
	Txt_Bind(txt, 27, TXTDT_INT, &cd->hom_id, sizeof(cd->hom_id));
	Txt_Bind(txt, 28, TXTDT_INT, &cd->mer_id, sizeof(cd->mer_id));
	Txt_Bind(txt, 29, TXTDT_SHORT, &cd->hair, sizeof(cd->hair));
	Txt_Bind(txt, 30, TXTDT_SHORT, &cd->hair_color, sizeof(cd->hair_color));
	Txt_Bind(txt, 31, TXTDT_SHORT, &cd->clothes_color, sizeof(cd->clothes_color));
	Txt_Bind(txt, 32, TXTDT_SHORT, &cd->weapon, sizeof(cd->weapon));
	Txt_Bind(txt, 33, TXTDT_SHORT, &cd->shield, sizeof(cd->shield));
	Txt_Bind(txt, 34, TXTDT_SHORT, &cd->head_top, sizeof(cd->head_top));
	Txt_Bind(txt, 35, TXTDT_SHORT, &cd->head_mid, sizeof(cd->head_mid));
	Txt_Bind(txt, 36, TXTDT_SHORT, &cd->head_bottom, sizeof(cd->head_bottom));
	Txt_Bind(txt, 37, TXTDT_STRING, last_map, sizeof(last_map));
	Txt_Bind(txt, 38, TXTDT_SHORT, &cd->last_point.x, sizeof(cd->last_point.x));
	Txt_Bind(txt, 39, TXTDT_SHORT, &cd->last_point.y, sizeof(cd->last_point.y));
	Txt_Bind(txt, 40, TXTDT_STRING, save_map, sizeof(save_map));
	Txt_Bind(txt, 41, TXTDT_SHORT, &cd->save_point.x, sizeof(cd->save_point.x));
	Txt_Bind(txt, 42, TXTDT_SHORT, &cd->save_point.y, sizeof(cd->save_point.y));
	Txt_Bind(txt, 43, TXTDT_INT, &cd->partner_id, sizeof(cd->partner_id));
	Txt_Bind(txt, 44, TXTDT_INT, &cd->father, sizeof(cd->father));
	Txt_Bind(txt, 45, TXTDT_INT, &cd->mother, sizeof(cd->mother));
	Txt_Bind(txt, 46, TXTDT_INT, &cd->child, sizeof(cd->child));
	Txt_Bind(txt, 47, TXTDT_INT, &cd->arch_calls, sizeof(cd->arch_calls));
	Txt_Bind(txt, 48, TXTDT_INT, &cd->arch_faith, sizeof(cd->arch_faith));
	Txt_Bind(txt, 49, TXTDT_INT, &cd->spear_calls, sizeof(cd->spear_calls));
	Txt_Bind(txt, 50, TXTDT_INT, &cd->spear_faith, sizeof(cd->spear_faith));
	Txt_Bind(txt, 51, TXTDT_INT, &cd->sword_calls, sizeof(cd->sword_calls));
	Txt_Bind(txt, 52, TXTDT_INT, &cd->sword_faith, sizeof(cd->sword_faith));
	if( Txt_Parse(txt) != TXT_SUCCESS || Txt_NumFields(txt) != 53 )
	{
		Txt_Free(txt);
		return false;
	}
	Txt_Free(txt);

	cd->last_point.map = mapindex_name2id(last_map);
	cd->save_point.map = mapindex_name2id(save_map);

	*key = cd->char_id;

/*
	int tmp_charid;
	char tmp_name[NAME_LENGTH];
	char tmp_map[MAP_NAME_LENGTH];

	// uniqueness checks
	if( chars->id2name(chars, cd->char_id, tmp_name, sizeof(tmp_name)) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on id %d between character '%s' and existing character '%s'!\n", cd->char_id, cd->name, tmp_name);
		return false;
	}
	if( chars->name2id(chars, cd->name, true, &tmp_charid, NULL, NULL) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on name '%s' between character %d and existing character %d!\n", cd->name, cd->char_id, tmp_charid);
		return false;
	}
*/

	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool char_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	char* p = str;
	struct mmo_charstatus* cd = (struct mmo_charstatus*)data;
	char last_map[MAP_NAME_LENGTH], save_map[MAP_NAME_LENGTH];
	bool result;
	Txt* txt;

	safestrncpy(last_map, mapindex_id2name(cd->last_point.map), sizeof(last_map));
	safestrncpy(save_map, mapindex_id2name(cd->save_point.map), sizeof(save_map));

	// key (char id)
	p += sprintf(p, "%d\t", cd->char_id);

	// base character data
	txt = Txt_Malloc();
	Txt_Init(txt, p, SIZE_MAX, 53, ',', '\0', ",\t");
	Txt_Bind(txt,  0, TXTDT_INT, &cd->account_id, sizeof(cd->account_id));
	Txt_Bind(txt,  1, TXTDT_UCHAR, &cd->slot, sizeof(cd->slot));
	Txt_Bind(txt,  2, TXTDT_CSTRING, &cd->name, sizeof(cd->name));
	Txt_Bind(txt,  3, TXTDT_SHORT, &cd->class_, sizeof(cd->class_));
	Txt_Bind(txt,  4, TXTDT_UINT, &cd->base_level, sizeof(cd->base_level));
	Txt_Bind(txt,  5, TXTDT_UINT, &cd->job_level, sizeof(cd->job_level));
	Txt_Bind(txt,  6, TXTDT_UINT, &cd->base_exp, sizeof(cd->base_exp));
	Txt_Bind(txt,  7, TXTDT_UINT, &cd->job_exp, sizeof(cd->job_exp));
	Txt_Bind(txt,  8, TXTDT_INT, &cd->zeny, sizeof(cd->zeny));
	Txt_Bind(txt,  9, TXTDT_INT, &cd->hp, sizeof(cd->hp));
	Txt_Bind(txt, 10, TXTDT_INT, &cd->max_hp, sizeof(cd->max_hp));
	Txt_Bind(txt, 11, TXTDT_INT, &cd->sp, sizeof(cd->sp));
	Txt_Bind(txt, 12, TXTDT_INT, &cd->max_sp, sizeof(cd->max_sp));
	Txt_Bind(txt, 13, TXTDT_SHORT, &cd->str, sizeof(cd->str));
	Txt_Bind(txt, 14, TXTDT_SHORT, &cd->agi, sizeof(cd->agi));
	Txt_Bind(txt, 15, TXTDT_SHORT, &cd->vit, sizeof(cd->vit));
	Txt_Bind(txt, 16, TXTDT_SHORT, &cd->int_, sizeof(cd->int_));
	Txt_Bind(txt, 17, TXTDT_SHORT, &cd->dex, sizeof(cd->dex));
	Txt_Bind(txt, 18, TXTDT_SHORT, &cd->luk, sizeof(cd->luk));
	Txt_Bind(txt, 19, TXTDT_UINT, &cd->status_point, sizeof(cd->status_point));
	Txt_Bind(txt, 20, TXTDT_UINT, &cd->skill_point, sizeof(cd->skill_point));
	Txt_Bind(txt, 21, TXTDT_UINT, &cd->option, sizeof(cd->option));
	Txt_Bind(txt, 22, TXTDT_UCHAR, &cd->karma, sizeof(cd->karma));
	Txt_Bind(txt, 23, TXTDT_SHORT, &cd->manner, sizeof(cd->manner));
	Txt_Bind(txt, 24, TXTDT_INT, &cd->party_id, sizeof(cd->party_id));
	Txt_Bind(txt, 25, TXTDT_INT, &cd->guild_id, sizeof(cd->guild_id));
	Txt_Bind(txt, 26, TXTDT_INT, &cd->pet_id, sizeof(cd->pet_id));
	Txt_Bind(txt, 27, TXTDT_INT, &cd->hom_id, sizeof(cd->hom_id));
	Txt_Bind(txt, 28, TXTDT_INT, &cd->mer_id, sizeof(cd->mer_id));
	Txt_Bind(txt, 29, TXTDT_SHORT, &cd->hair, sizeof(cd->hair));
	Txt_Bind(txt, 30, TXTDT_SHORT, &cd->hair_color, sizeof(cd->hair_color));
	Txt_Bind(txt, 31, TXTDT_SHORT, &cd->clothes_color, sizeof(cd->clothes_color));
	Txt_Bind(txt, 32, TXTDT_SHORT, &cd->weapon, sizeof(cd->weapon));
	Txt_Bind(txt, 33, TXTDT_SHORT, &cd->shield, sizeof(cd->shield));
	Txt_Bind(txt, 34, TXTDT_SHORT, &cd->head_top, sizeof(cd->head_top));
	Txt_Bind(txt, 35, TXTDT_SHORT, &cd->head_mid, sizeof(cd->head_mid));
	Txt_Bind(txt, 36, TXTDT_SHORT, &cd->head_bottom, sizeof(cd->head_bottom));
	Txt_Bind(txt, 37, TXTDT_STRING, last_map, sizeof(last_map));
	Txt_Bind(txt, 38, TXTDT_SHORT, &cd->last_point.x, sizeof(cd->last_point.x));
	Txt_Bind(txt, 39, TXTDT_SHORT, &cd->last_point.y, sizeof(cd->last_point.y));
	Txt_Bind(txt, 40, TXTDT_STRING, save_map, sizeof(save_map));
	Txt_Bind(txt, 41, TXTDT_SHORT, &cd->save_point.x, sizeof(cd->save_point.x));
	Txt_Bind(txt, 42, TXTDT_SHORT, &cd->save_point.y, sizeof(cd->save_point.y));
	Txt_Bind(txt, 43, TXTDT_INT, &cd->partner_id, sizeof(cd->partner_id));
	Txt_Bind(txt, 44, TXTDT_INT, &cd->father, sizeof(cd->father));
	Txt_Bind(txt, 45, TXTDT_INT, &cd->mother, sizeof(cd->mother));
	Txt_Bind(txt, 46, TXTDT_INT, &cd->child, sizeof(cd->child));
	Txt_Bind(txt, 47, TXTDT_INT, &cd->arch_calls, sizeof(cd->arch_calls));
	Txt_Bind(txt, 48, TXTDT_INT, &cd->arch_faith, sizeof(cd->arch_faith));
	Txt_Bind(txt, 49, TXTDT_INT, &cd->spear_calls, sizeof(cd->spear_calls));
	Txt_Bind(txt, 50, TXTDT_INT, &cd->spear_faith, sizeof(cd->spear_faith));
	Txt_Bind(txt, 51, TXTDT_INT, &cd->sword_calls, sizeof(cd->sword_calls));
	Txt_Bind(txt, 52, TXTDT_INT, &cd->sword_faith, sizeof(cd->sword_faith));

	result = ( Txt_Write(txt) == TXT_SUCCESS && Txt_NumFields(txt) == 53 );
	Txt_Free(txt);

	return result;
}


/// @protected
static bool char_db_txt_init(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	CSDBIterator* iter;
	int char_id;

	if( !db->db->init(db->db) )
		return false;

	// create index
	if( db->idx_name == NULL )
		db->idx_name = strdb_alloc(DB_OPT_DUP_KEY, 0);
	db_clear(db->idx_name);
	iter = db->db->iterator(db->db);
	while( iter->next(iter, &char_id) )
	{
		struct mmo_charstatus cd;
		if( !db->db->load(db->db, char_id, &cd, sizeof(cd), NULL) )
			continue;
		strdb_put(db->idx_name, cd.name, (void*)char_id);
	}
	iter->destroy(iter);

	return true;
}


/// @protected
static void char_db_txt_destroy(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;

	// delete chars database
	db->db->destroy(db->db);

	// delete indexes
	if( db->idx_name != NULL )
		db_destroy(db->idx_name);

	// delete entire structure
	aFree(db);
}


/// @protected
static bool char_db_txt_sync(CharDB* self, bool force)
{
	CSDB_TXT* db = ((CharDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool char_db_txt_create(CharDB* self, struct mmo_charstatus* cd)
{
	CharDB_TXT* db = (CharDB_TXT*)self;

	if( cd->char_id == -1 )
		cd->char_id = db->db->next_key(db->db);

	// data restrictions
	if( self->id2name(self, cd->char_id, NULL, 0) )
		return false;// id is being used
	if( self->name2id(self, cd->name, true, NULL, NULL, NULL) )
		return false;// name is being used

	// store data
	db->db->insert(db->db, cd->char_id, cd, sizeof(*cd));
	strdb_put(db->idx_name, cd->name, (void*)cd->char_id);

	return true;
}


/// @protected
static bool char_db_txt_remove(CharDB* self, const int char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	struct mmo_charstatus cd;

	if( !db->db->load(db->db, char_id, &cd, sizeof(cd), NULL) )
		return true; // nothing to delete

	// delete from database and index
	db->db->remove(db->db, char_id);
	strdb_remove(db->idx_name, cd.name);

	return true;
}


/// @protected
static bool char_db_txt_save(CharDB* self, const struct mmo_charstatus* cd)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	struct mmo_charstatus tmp;
	bool name_changed = false;

	// retrieve previous data
	if( !db->db->load(db->db, cd->char_id, &tmp, sizeof(tmp), NULL) )
		return false; // entry not found

	// check integrity constraints
	if( strcmp(cd->name, tmp.name) != 0 )
	{
		name_changed = true;
		if( strdb_exists(db->idx_name, cd->name) )
			return false; // name already taken
	}

	// write new data
	if( !db->db->update(db->db, cd->char_id, cd, sizeof(*cd)) )
		return false;

	// update index
	if( name_changed )
	{
		strdb_remove(db->idx_name, tmp.name);
		strdb_put(db->idx_name, cd->name, (void*)cd->char_id);
	}

	return true;
}


/// @protected
static bool char_db_txt_load_num(CharDB* self, struct mmo_charstatus* cd, int char_id)
{
	CSDB_TXT* db = ((CharDB_TXT*)self)->db;
	return db->load(db, char_id, cd, sizeof(*cd), NULL);
}


/// @protected
static bool char_db_txt_load_str(CharDB* self, struct mmo_charstatus* cd, const char* name, bool case_sensitive)
{
	int char_id;
	unsigned int n;

	// find char id
	if( !self->name2id(self, name, true, &char_id, NULL, &n) &&// not exact
		!(!case_sensitive && self->name2id(self, name, false, &char_id, NULL, &n) && n == 1) )// not unique
	{// name not exact and not unique
		return false;
	}

	// retrieve data
	return self->load_num(self, cd, char_id);
}


/// @protected
static bool char_db_txt_id2name(CharDB* self, int char_id, char* name, size_t size)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	struct mmo_charstatus cd;

	if( !db->db->load(db->db, char_id, &cd, sizeof(cd), NULL) )
		return false;

	if( name != NULL )
		safestrncpy(name, cd.name, size);

	return true;
}


/// @protected
static bool char_db_txt_name2id(CharDB* self, const char* name, bool case_sensitive, int* char_id, int* account_id, unsigned int* count)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	struct mmo_charstatus tmp;
	bool found = false;

	if( count != NULL )
		*count = 1;

	// retrieve data
	if( case_sensitive )
	{
		int tmp_id;

		if( !strdb_exists(db->idx_name, name) )
			return false;

		tmp_id = (int)strdb_get(db->idx_name, name);

		if( !db->db->load(db->db, tmp_id, &tmp, sizeof(tmp), NULL) )
			return false;

		found = true;
	}
	else
	{
		int tmp_id;
		CSDBIterator* iter = db->db->iterator(db->db);
		while( iter->next(iter, &tmp_id) )
		{
			if( !db->db->load(db->db, tmp_id, &tmp, sizeof(tmp), NULL) )
				continue;

			if( stricmp(name, tmp.name) == 0 )
			{
				found = true;
				break;
			}
		}

		if( count != NULL )
		{// count other matches
			struct mmo_charstatus tmp2;
			while( iter->next(iter, &tmp_id) )
			{
				if( !db->db->load(db->db, tmp_id, &tmp2, sizeof(tmp2), NULL) )
					continue;

				if( stricmp(name, tmp2.name) == 0 )
					*count = *count + 1;
			}
		}
		iter->destroy(iter);
	}

	if( !found )
	{// entry not found
		if( count != NULL )
			*count = 0;
		return false;
	}

	if( char_id != NULL )
		*char_id = tmp.char_id;
	if( account_id != NULL )
		*account_id = tmp.account_id;

	return true;
}


/// Returns an iterator over all characters.
/// @protected
static CSDBIterator* char_db_txt_iterator(CharDB* self)
{
	CSDB_TXT* db = ((CharDB_TXT*)self)->db;
	return db->iterator(db);
}


/// internal structure
/// @private
typedef struct CharDBIterator_TXT
{
	CSDBIterator vtable;      // public interface

	CharDB_TXT* owner;
	CSDBIterator* iter;
	int account_id;

} CharDBIterator_TXT;


/// Destroys this iterator, releasing all allocated memory (including itself).
/// @protected
static void char_db_txt_iter_destroy(CSDBIterator* self)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	iter->iter->destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next character.
/// @protected
static bool char_db_txt_iter_next(CSDBIterator* self, int* key)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	struct mmo_charstatus tmp;
	int char_id;

	while( true )
	{
		if( !iter->iter->next(iter->iter, &char_id) )
			return false;// not found

		//FIXME: very expensive
		if( !iter->owner->db->load(iter->owner->db, char_id, &tmp, sizeof(tmp), NULL) )
			return false;// failed to load

		if( iter->account_id != tmp.account_id )
			continue;// wrong account, try next

		if( key )
			*key = tmp.char_id;
		return true;
	}
}


/// Returns an iterator over all the characters of the account.
/// @protected
static CSDBIterator* char_db_txt_characters(CharDB* self, int account_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)aCalloc(1, sizeof(CharDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &char_db_txt_iter_destroy;
	iter->vtable.next    = &char_db_txt_iter_next;

	// fill data
	iter->owner = db;
	iter->iter = db->db->iterator(db->db);
	iter->account_id = account_id;

	return &iter->vtable;
}


/// Constructs a new CharDB interface.
/// @protected
CharDB* char_db_txt(CharServerDB_TXT* owner)
{
	CharDB_TXT* db = (CharDB_TXT*)aCalloc(1, sizeof(CharDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_chars, CHARDB_TXT_DB_VERSION, START_CHAR_NUM);
	db->db->p.fromstr = &char_db_txt_fromstr;
	db->db->p.tostr   = &char_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &char_db_txt_init;
	db->vtable.p.destroy = &char_db_txt_destroy;
	db->vtable.p.sync    = &char_db_txt_sync;
	db->vtable.create    = &char_db_txt_create;
	db->vtable.remove    = &char_db_txt_remove;
	db->vtable.save      = &char_db_txt_save;
	db->vtable.load_num  = &char_db_txt_load_num;
	db->vtable.load_str  = &char_db_txt_load_str;
	db->vtable.id2name   = &char_db_txt_id2name;
	db->vtable.name2id   = &char_db_txt_name2id;
	db->vtable.iterator  = &char_db_txt_iterator;
	db->vtable.characters = &char_db_txt_characters;

	// initialize to default values
	db->idx_name = NULL;

	return &db->vtable;
}
