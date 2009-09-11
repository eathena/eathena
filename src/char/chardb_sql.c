// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "charserverdb_sql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// Maximum number of character ids cached in the iterator.
#define CHARDBITERATOR_MAXCACHE 16000


/// Internal structure.
/// @private
typedef struct CharDB_SQL
{
	// public interface
	CharDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* chars;

	// settings
	const char* char_db;
	const char* mercenary_owner_db;

} CharDB_SQL;


/// @private
static bool mmo_char_fromsql(CharDB_SQL* db, struct mmo_charstatus* p, int char_id, bool load_everything)
{
	Sql* sql_handle = db->chars;
	SqlStmt* stmt;
	char last_map[MAP_NAME_LENGTH_EXT];
	char save_map[MAP_NAME_LENGTH_EXT];

	memset(p, 0, sizeof(struct mmo_charstatus));
	
	stmt = SqlStmt_Malloc(sql_handle);
	if( stmt == NULL )
	{
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	// read char data
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT "
		"`char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
		"`status_point`,`skill_point`,`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`homun_id`,`hair`,"
		"`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`last_x`,`last_y`,"
		"`save_map`,`save_x`,`save_y`,`partner_id`,`father`,`mother`,`child`"
		" FROM `%s` WHERE `char_id`=? LIMIT 1", db->char_db)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0,  SQLDT_INT,    &p->char_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1,  SQLDT_INT,    &p->account_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2,  SQLDT_UCHAR,  &p->slot, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3,  SQLDT_STRING, &p->name, sizeof(p->name), NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4,  SQLDT_SHORT,  &p->class_, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5,  SQLDT_UINT,   &p->base_level, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6,  SQLDT_UINT,   &p->job_level, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 7,  SQLDT_UINT,   &p->base_exp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 8,  SQLDT_UINT,   &p->job_exp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 9,  SQLDT_INT,    &p->zeny, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 10, SQLDT_SHORT,  &p->str, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 11, SQLDT_SHORT,  &p->agi, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 12, SQLDT_SHORT,  &p->vit, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 13, SQLDT_SHORT,  &p->int_, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 14, SQLDT_SHORT,  &p->dex, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 15, SQLDT_SHORT,  &p->luk, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 16, SQLDT_INT,    &p->max_hp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 17, SQLDT_INT,    &p->hp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 18, SQLDT_INT,    &p->max_sp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 19, SQLDT_INT,    &p->sp, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 20, SQLDT_UINT,   &p->status_point, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 21, SQLDT_UINT,   &p->skill_point, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 22, SQLDT_UINT,   &p->option, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 23, SQLDT_UCHAR,  &p->karma, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 24, SQLDT_SHORT,  &p->manner, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 25, SQLDT_INT,    &p->party_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 26, SQLDT_INT,    &p->guild_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 27, SQLDT_INT,    &p->pet_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 28, SQLDT_INT,    &p->hom_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 29, SQLDT_SHORT,  &p->hair, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 30, SQLDT_SHORT,  &p->hair_color, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 31, SQLDT_SHORT,  &p->clothes_color, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 32, SQLDT_SHORT,  &p->weapon, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 33, SQLDT_SHORT,  &p->shield, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 34, SQLDT_SHORT,  &p->head_top, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 35, SQLDT_SHORT,  &p->head_mid, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 36, SQLDT_SHORT,  &p->head_bottom, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 37, SQLDT_STRING, &last_map, sizeof(last_map), NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 38, SQLDT_SHORT,  &p->last_point.x, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 39, SQLDT_SHORT,  &p->last_point.y, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 40, SQLDT_STRING, &save_map, sizeof(save_map), NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 41, SQLDT_SHORT,  &p->save_point.x, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 42, SQLDT_SHORT,  &p->save_point.y, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 43, SQLDT_INT,    &p->partner_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 44, SQLDT_INT,    &p->father, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 45, SQLDT_INT,    &p->mother, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 46, SQLDT_INT,    &p->child, 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return false;
	}

	if( SqlStmt_NumRows(stmt) == 0 )
	{
		ShowError("Requested non-existant character id: %d!\n", char_id);
		SqlStmt_Free(stmt);
		return false;
	}

	if( SQL_SUCCESS != SqlStmt_NextRow(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return false;
	}

	p->last_point.map = mapindex_name2id(last_map);
	p->save_point.map = mapindex_name2id(save_map);

	if (!load_everything) // For quick selection of data when displaying the char menu
	{
		SqlStmt_Free(stmt);
		return true;
	}

	//read mercenary owner data
	//`mercenary_owner` (`char_id`,`merc_id`,`arch_calls`,`arch_faith`,`spear_calls`,`spear_faith`,`sword_calls`,`sword_faith`)
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith` FROM `%s` WHERE `char_id` = '%d'", db->mercenary_owner_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;
		Sql_GetData(sql_handle,  0, &data, NULL); p->mer_id = atoi(data);
		Sql_GetData(sql_handle,  1, &data, NULL); p->arch_calls = atoi(data);
		Sql_GetData(sql_handle,  2, &data, NULL); p->arch_faith = atoi(data);
		Sql_GetData(sql_handle,  3, &data, NULL); p->spear_calls = atoi(data);
		Sql_GetData(sql_handle,  4, &data, NULL); p->spear_faith = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); p->sword_calls = atoi(data);
		Sql_GetData(sql_handle,  6, &data, NULL); p->sword_faith = atoi(data);
	}

	SqlStmt_Free(stmt);

	return true;
}


/// @private
static bool mmo_char_tosql(CharDB_SQL* db, struct mmo_charstatus* p, bool is_new)
{
	Sql* sql_handle = db->chars;

	struct mmo_charstatus tmp;
	struct mmo_charstatus* cp = &tmp;
	StringBuf buf;
	SqlStmt* stmt = NULL;
	bool result = false;

	// get previous data to diff against
	if( is_new )
		memset(cp, 0, sizeof(*cp));
	else
		mmo_char_fromsql(db, cp, p->char_id, true);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	StringBuf_Init(&buf);

	// try
	do
	{

	if( is_new )
	{// Insert the barebones to then update the rest.
		int insert_id;

		SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
		if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "REPLACE INTO `%s` (`char_id`, `account_id`, `char_num`, `name`)  VALUES (?,?,?,?)", db->char_db)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, (p->char_id != -1)?SQLDT_INT:SQLDT_NULL, (void*)&p->char_id, 0)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_INT, (void*)&p->account_id, 0)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_UCHAR, (void*)&p->slot, 0)
		||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_STRING, (void*)p->name, strnlen(p->name, ITEM_NAME_LENGTH))
		||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
		{
			SqlStmt_ShowDebug(stmt);
			break;
		}

		insert_id = (int)SqlStmt_LastInsertId(stmt);
		if( p->char_id == -1 )
			p->char_id = insert_id; // fill in output value
		else
		if( p->char_id != insert_id )
			break; // error, unexpected value
	}

	if (
		(p->base_exp != cp->base_exp) || (p->base_level != cp->base_level) ||
		(p->job_level != cp->job_level) || (p->job_exp != cp->job_exp) ||
		(p->zeny != cp->zeny) ||
		(p->last_point.x != cp->last_point.x) || (p->last_point.y != cp->last_point.y) ||
		(p->max_hp != cp->max_hp) || (p->hp != cp->hp) ||
		(p->max_sp != cp->max_sp) || (p->sp != cp->sp) ||
		(p->status_point != cp->status_point) || (p->skill_point != cp->skill_point) ||
		(p->str != cp->str) || (p->agi != cp->agi) || (p->vit != cp->vit) ||
		(p->int_ != cp->int_) || (p->dex != cp->dex) || (p->luk != cp->luk) ||
		(p->option != cp->option) ||
		(p->party_id != cp->party_id) || (p->guild_id != cp->guild_id) ||
		(p->pet_id != cp->pet_id) || (p->weapon != cp->weapon) || (p->hom_id != cp->hom_id) ||
		(p->shield != cp->shield) || (p->head_top != cp->head_top) ||
		(p->head_mid != cp->head_mid) || (p->head_bottom != cp->head_bottom)
	)
	{	//Save status
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `base_level`='%d', `job_level`='%d',"
			"`base_exp`='%u', `job_exp`='%u', `zeny`='%d',"
			"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%u',`skill_point`='%u',"
			"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
			"`option`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',`homun_id`='%d',"
			"`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
			"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			db->char_db, p->base_level, p->job_level,
			p->base_exp, p->job_exp, p->zeny,
			p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
			p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
			p->option, p->party_id, p->guild_id, p->pet_id, p->hom_id,
			p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
			mapindex_id2name(p->last_point.map), p->last_point.x, p->last_point.y,
			mapindex_id2name(p->save_point.map), p->save_point.x, p->save_point.y,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
		}
	}

	//Values that will seldom change (to speed up saving)
	if (
		(p->hair != cp->hair) || (p->hair_color != cp->hair_color) || (p->clothes_color != cp->clothes_color) ||
		(p->class_ != cp->class_) ||
		(p->partner_id != cp->partner_id) || (p->father != cp->father) ||
		(p->mother != cp->mother) || (p->child != cp->child) ||
 		(p->karma != cp->karma) || (p->manner != cp->manner)
	)
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',"
			"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',"
			"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d',"
			"`karma`='%d',`manner`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			db->char_db, p->class_,
			p->hair, p->hair_color, p->clothes_color,
			p->partner_id, p->father, p->mother, p->child,
			p->karma, p->manner,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
		}
	}

	//mercenary owner data
	if( (p->mer_id != cp->mer_id) ||
		(p->arch_calls != cp->arch_calls) || (p->arch_faith != cp->arch_faith) ||
		(p->spear_calls != cp->spear_calls) || (p->spear_faith != cp->spear_faith) ||
		(p->sword_calls != cp->sword_calls) || (p->sword_faith != cp->sword_faith) )
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`char_id`, `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith`) VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			db->mercenary_owner_db, p->char_id, p->mer_id, p->arch_calls, p->arch_faith, p->spear_calls, p->spear_faith, p->sword_calls, p->sword_faith) )
		{
			Sql_ShowDebug(sql_handle);
		}
	}

	// success
	result = true;

	}
	while(0);
	// finally

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") )
	{
		Sql_ShowDebug(sql_handle);
		result = false;
	}

	return result;
}


/// @protected
static bool char_db_sql_init(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	db->chars = db->owner->sql_handle;
	return true;
}


/// @protected
static void char_db_sql_destroy(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	db->chars = NULL;
	aFree(db);
}


/// @protected
static bool char_db_sql_sync(CharDB* self, bool force)
{
	return true;
}


/// @protected
static bool char_db_sql_create(CharDB* self, struct mmo_charstatus* cd)
{
	CharDB_SQL* db = (CharDB_SQL*)self;

	// data restrictions
	if( cd->char_id != -1 && self->id2name(self, cd->char_id, NULL, 0) )
		return false;// id is being used
	if( self->name2id(self, cd->name, true, NULL, NULL, NULL) )
		return false;// name is being used

	return mmo_char_tosql(db, cd, true);
}


/// @protected
static bool char_db_sql_remove(CharDB* self, const int char_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	// try
	do
	{

	if( SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `mercenary_owner` WHERE `char_id` = '%d'", char_id) // mercenary owner data
	||  SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->char_db, char_id) // character
	) {
		Sql_ShowDebug(sql_handle);
		break;
	}

	// success
	result = true;

	}
	while(0);
	// finally

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") )
	{
		Sql_ShowDebug(sql_handle);
		result = false;
	}

	return result;
}


/// @protected
static bool char_db_sql_save(CharDB* self, const struct mmo_charstatus* ch)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	int tmp_id;

	// data restrictions
	if( self->name2id(self, ch->name, true, &tmp_id, NULL, NULL) && tmp_id != ch->char_id )
		return false;// name is being used

	return mmo_char_tosql(db, (struct mmo_charstatus*)ch, false);
}


/// @protected
static bool char_db_sql_load_num(CharDB* self, struct mmo_charstatus* ch, int char_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	return mmo_char_fromsql(db, ch, char_id, true);
}


/// @protected
static bool char_db_sql_load_str(CharDB* self, struct mmo_charstatus* ch, const char* name, bool case_sensitive)
{
//	CharDB_SQL* db = (CharDB_SQL*)self;
	int char_id;
	unsigned int n;

	// find char id
	if( !self->name2id(self, name, true, &char_id, NULL, &n) &&// not exact
		!(!case_sensitive && self->name2id(self, name, false, &char_id, NULL, &n) && n == 1) )// not unique
	{// name not exact and not unique
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}


/// @protected
static bool char_db_sql_id2name(CharDB* self, int char_id, char* name, size_t size)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	char* data;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `name` FROM `%s` WHERE `char_id`='%d'", db->char_db, char_id)
	||  SQL_SUCCESS != Sql_NextRow(sql_handle)
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL);
	if( name != NULL )
		safestrncpy(name, data, size);
	Sql_FreeResult(sql_handle);

	return true;
}


/// @protected
static bool char_db_sql_name2id(CharDB* self, const char* name, bool case_sensitive, int* char_id, int* account_id, unsigned int* count)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	char esc_name[2*NAME_LENGTH+1];
	char* data;

	if( count != NULL )
		*count = 0;

	Sql_EscapeString(sql_handle, esc_name, name);

	// get the list of char IDs for this char name
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`account_id` FROM `%s` WHERE `name`= %s '%s'",
		db->char_db, (case_sensitive ? "BINARY" : ""), esc_name) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( count != NULL )
		*count = (unsigned int)Sql_NumRows(sql_handle);

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL); if( char_id != NULL ) *char_id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, NULL); if( account_id != NULL ) *account_id = atoi(data);
	Sql_FreeResult(sql_handle);

	return true;
}


/// Returns an iterator over all the characters.
/// @protected
static CSDBIterator* char_db_sql_iterator(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	return csdb_sql_iterator(db->chars, db->char_db, "char_id");
}



/// internal structure
/// @private
typedef struct CharDBIterator_SQL
{
	CSDBIterator vtable;    // public interface

	CharDB_SQL* db;
	int* ids_arr;
	int ids_num;
	int pos;

} CharDBIterator_SQL;


/// Destroys this iterator, releasing all allocated memory (including itself).
/// @protected
static void char_db_sql_iter_destroy(CSDBIterator* self)
{
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}


/// Fetches the next character.
/// @protected
static bool char_db_sql_iter_next(CSDBIterator* self, int* key)
{
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)self;

	if( iter->pos+1 >= iter->ids_num )
		return false;

	++iter->pos;
	if( key )
		*key = iter->ids_arr[iter->pos];
	return true;
}


/// Returns an iterator over all the characters of the account.
/// @protected
static CSDBIterator* char_db_sql_characters(CharDB* self, int account_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)aCalloc(1, sizeof(CharDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &char_db_sql_iter_destroy;
	iter->vtable.next    = &char_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`=%d ORDER BY `char_id` ASC", db->char_db, account_id) )
		Sql_ShowDebug(sql_handle);
	else if( Sql_NumRows(sql_handle) > 0 )
	{
		int i;

		iter->ids_num = (int)Sql_NumRows(sql_handle);
		CREATE(iter->ids_arr, int, iter->ids_num);

		i = 0;
		while( i < iter->ids_num )
		{
			char* data;
			int res = Sql_NextRow(sql_handle);
			if( res == SQL_SUCCESS )
				res = Sql_GetData(sql_handle, 0, &data, NULL);
			if( res == SQL_ERROR )
				Sql_ShowDebug(sql_handle);
			if( res != SQL_SUCCESS )
				break;

			if( data == NULL )
				continue;

			iter->ids_arr[i] = atoi(data);
			++i;
		}
		iter->ids_num = i;
	}
	Sql_FreeResult(sql_handle);

	return &iter->vtable;
}


/// Constructs a new CharDB interface.
/// @protected
CharDB* char_db_sql(CharServerDB_SQL* owner)
{
	CharDB_SQL* db = (CharDB_SQL*)aCalloc(1, sizeof(CharDB_SQL));

	// set up the vtable
	db->vtable.p.init      = &char_db_sql_init;
	db->vtable.p.destroy   = &char_db_sql_destroy;
	db->vtable.p.sync      = &char_db_sql_sync;
	db->vtable.create    = &char_db_sql_create;
	db->vtable.remove    = &char_db_sql_remove;
	db->vtable.save      = &char_db_sql_save;
	db->vtable.load_num  = &char_db_sql_load_num;
	db->vtable.load_str  = &char_db_sql_load_str;
	db->vtable.id2name   = &char_db_sql_id2name;
	db->vtable.name2id   = &char_db_sql_name2id;
	db->vtable.iterator  = &char_db_sql_iterator;
	db->vtable.characters = &char_db_sql_characters;

	// initialize to default values
	db->owner = owner;
	db->chars = NULL;

	// other settings
	db->char_db = db->owner->table_chars;
	db->mercenary_owner_db = db->owner->table_mercenary_owners;

	return &db->vtable;
}
