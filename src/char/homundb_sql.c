// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "charserverdb_sql.h"
#include "homundb.h"
#include <string.h>


/// Internal structure.
/// @private
typedef struct HomunDB_SQL
{
	// public interface
	HomunDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* homuns;

	// settings
	const char* homun_db;
	const char* homun_skill_db;

} HomunDB_SQL;


/// @private
static bool mmo_homun_fromsql(HomunDB_SQL* db, struct s_homunculus* hd, int homun_id)
{
	Sql* sql_handle = db->homuns;
	SqlStmt* stmt;
	StringBuf buf;
	int id, lv;
	int count;
	int i;
	bool result = false;

	memset(hd, 0, sizeof(*hd));

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "SELECT `homun_id`,`char_id`,`class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`, `vaporize` FROM `%s` WHERE `homun_id`='%u'", db->homun_db, homun_id);

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);

	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( SqlStmt_NumRows(stmt) == 0 )
		break; // no homunculus found

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT, (void*)&hd->hom_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_INT, (void*)&hd->char_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT, (void*)&hd->class_, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_STRING, (void*)hd->name, sizeof(hd->name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_SHORT, (void*)&hd->level, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_UINT, (void*)&hd->exp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_UINT, (void*)&hd->intimacy, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_SHORT, (void*)&hd->hunger, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_INT, (void*)&hd->str, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_INT, (void*)&hd->agi, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_INT, (void*)&hd->vit, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 11, SQLDT_INT, (void*)&hd->int_, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 12, SQLDT_INT, (void*)&hd->dex, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 13, SQLDT_INT, (void*)&hd->luk, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 14, SQLDT_INT, (void*)&hd->hp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 15, SQLDT_INT, (void*)&hd->max_hp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 16, SQLDT_INT, (void*)&hd->sp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 17, SQLDT_INT, (void*)&hd->max_sp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 18, SQLDT_SHORT, (void*)&hd->skillpts, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 19, SQLDT_SHORT, (void*)&hd->rename_flag, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 20, SQLDT_SHORT, (void*)&hd->vaporize, 0, NULL, NULL);

	if( SQL_SUCCESS != SqlStmt_NextRow(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	hd->intimacy = cap_value(hd->intimacy, 0, 100000);
	hd->hunger = cap_value(hd->hunger, 0, 100);

	StringBuf_Clear(&buf);

	// Load Homunculus Skill
	StringBuf_Printf(&buf, "SELECT `id`,`lv` FROM `%s` WHERE `homun_id`=%d", db->homun_skill_db, homun_id);

	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &id, 0, NULL, NULL)
	||	SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &lv, 0, NULL, NULL) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( SqlStmt_NumRows(stmt) == 0 )
	{// no homun skills found
		result = true;
		break;
	}

	for( count = 0; count < MAX_HOMUNSKILL && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++count )
	{
		if( id < HM_SKILLBASE || id >= HM_SKILLBASE + MAX_HOMUNSKILL )
			continue;// invalid skill id

		i = id - HM_SKILLBASE;

		hd->hskill[i].id = id;
		hd->hskill[i].lv = lv;
	}

	// success
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


/// @private
static bool mmo_homun_tosql(HomunDB_SQL* db, struct s_homunculus* hd, bool is_new)
{
	Sql* sql_handle = db->homuns;
	StringBuf buf;
	SqlStmt* stmt = NULL;
	int insert_id;
	int count;
	int i;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	StringBuf_Init(&buf);

	if( is_new )
	{
		StringBuf_Printf(&buf,
			"INSERT INTO `%s` "
			"(`char_id`,`class`,`name`,`level`,`exp`,`intimacy`,`hunger`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`,`vaporize`,`homun_id`) "
			"VALUES "
			"(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
			, db->homun_db);
	}
	else
	{
		StringBuf_Printf(&buf,
			"UPDATE `%s` SET `char_id`=?,`class`=?,`name`=?,`level`=?,`exp`=?,`intimacy`=?,`hunger`=?,`str`=?,`agi`=?,`vit`=?,`int`=?,`dex`=?,`luk`=?, `hp`=?,`max_hp`=?,`sp`=?,`max_sp`=?,`skill_point`=?,`rename_flag`=?,`vaporize`=? WHERE `homun_id`=?"
			, db->homun_db);
	}

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_INT, (void*)&hd->char_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_SHORT, (void*)&hd->class_, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void*)hd->name, strnlen(hd->name, ARRAYLENGTH(hd->name)))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_SHORT, (void*)&hd->level, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_UINT, (void*)&hd->exp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 5, SQLDT_UINT, (void*)&hd->intimacy, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 6, SQLDT_SHORT, (void*)&hd->hunger, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 7, SQLDT_INT, (void*)&hd->str, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 8, SQLDT_INT, (void*)&hd->agi, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 9, SQLDT_INT, (void*)&hd->vit, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_INT, (void*)&hd->int_, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_INT, (void*)&hd->dex, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, SQLDT_INT, (void*)&hd->luk, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_INT, (void*)&hd->hp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, SQLDT_INT, (void*)&hd->max_hp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 15, SQLDT_INT, (void*)&hd->sp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 16, SQLDT_INT, (void*)&hd->max_sp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 17, SQLDT_SHORT, (void*)&hd->skillpts, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 18, SQLDT_SHORT, (void*)&hd->rename_flag, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 19, SQLDT_SHORT, (void*)&hd->vaporize, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 20, (hd->hom_id != -1)?SQLDT_INT:SQLDT_NULL, (void*)&hd->hom_id, 0)
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( is_new )
	{
		insert_id = (int)SqlStmt_LastInsertId(stmt);
		if( hd->hom_id == -1 )
			hd->hom_id = insert_id; // fill in output value
		else
		if( hd->hom_id != insert_id )
			break; // error, unexpected value
	}

	StringBuf_Clear(&buf);

	// skills
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `homun_id`='%d'", db->homun_skill_db, hd->hom_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s` (`homun_id`, `id`, `lv`) VALUES ", db->homun_skill_db);

	count = 0;
	for( i = 0; i < MAX_HOMUNSKILL; ++i )
	{
		if( hd->hskill[i].id <= 0 || hd->hskill[i].lv == 0 )
			continue;

		if( count > 0 )
			StringBuf_AppendStr(&buf, ", ");

		StringBuf_Printf(&buf, "('%d','%hu','%hu')", hd->hom_id, hd->hskill[i].id, hd->hskill[i].lv);

		count++;
	}

	if( count == 0 )
	{// nothing to save
		result = true;
		break;
	}

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	// success
	result = true;

	}
	while(0);

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
static bool homun_db_sql_init(HomunDB* self)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	db->homuns = db->owner->sql_handle;
	return true;
}


/// @protected
static void homun_db_sql_destroy(HomunDB* self)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	db->homuns = NULL;
	aFree(db);
}


/// @protected
static bool homun_db_sql_sync(HomunDB* self)
{
	return true;
}


/// @protected
static bool homun_db_sql_create(HomunDB* self, struct s_homunculus* hd)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return mmo_homun_tosql(db, hd, true);
}


/// @protected
static bool homun_db_sql_remove(HomunDB* self, int homun_id)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	Sql* sql_handle = db->homuns;
	bool result = false;

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		return result;
	}

	// try
	do
	{

	if( SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `homun_id` = '%d'", db->homun_db, homun_id)
	||	SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `homun_id` = '%d'", db->homun_skill_db, homun_id)
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
static bool homun_db_sql_save(HomunDB* self, const struct s_homunculus* hd)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return mmo_homun_tosql(db, (struct s_homunculus*)hd, false);
}


/// @protected
static bool homun_db_sql_load(HomunDB* self, struct s_homunculus* hd, int homun_id)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return mmo_homun_fromsql(db, hd, homun_id);
}


/// Returns an iterator over all homunculi.
/// @protected
static CSDBIterator* homun_db_sql_iterator(HomunDB* self)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return csdb_sql_iterator(db->homuns, db->homun_db, "homun_id");
}


/// Constructs a new HomunDB interface.
/// @protected
HomunDB* homun_db_sql(CharServerDB_SQL* owner)
{
	HomunDB_SQL* db = (HomunDB_SQL*)aCalloc(1, sizeof(HomunDB_SQL));

	// set up the vtable
	db->vtable.init      = &homun_db_sql_init;
	db->vtable.destroy   = &homun_db_sql_destroy;
	db->vtable.sync      = &homun_db_sql_sync;
	db->vtable.create    = &homun_db_sql_create;
	db->vtable.remove    = &homun_db_sql_remove;
	db->vtable.save      = &homun_db_sql_save;
	db->vtable.load      = &homun_db_sql_load;
	db->vtable.iterator  = &homun_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->homuns = NULL;

	// other settings
	db->homun_db = db->owner->table_homuns;
	db->homun_skill_db = db->owner->table_homun_skills;

	return &db->vtable;
}
