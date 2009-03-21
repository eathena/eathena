// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "homundb.h"
#include <string.h>

/// internal structure
typedef struct HomunDB_SQL
{
	HomunDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* homuns;       // SQL homun storage

	// other settings
	const char* homun_db;
	const char* homun_skill_db;

} HomunDB_SQL;



static bool mmo_homun_fromsql(HomunDB_SQL* db, struct s_homunculus* hd, int homun_id)
{
/*
	int i;
	char* data;
	size_t len;

	memset(hd, 0, sizeof(*hd));

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `homun_id`,`char_id`,`class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`, `vaporize` FROM `homunculus` WHERE `homun_id`='%u'", homun_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( !Sql_NumRows(sql_handle) )
	{	//No homunculus found.
		Sql_FreeResult(sql_handle);
		return false;
	}
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return false;
	}

	hd->hom_id = homun_id;
	Sql_GetData(sql_handle,  1, &data, NULL); hd->char_id = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); hd->class_ = atoi(data);
	Sql_GetData(sql_handle,  3, &data, &len); safestrncpy(hd->name, data, sizeof(hd->name));
	Sql_GetData(sql_handle,  4, &data, NULL); hd->level = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); hd->exp = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); hd->intimacy = (unsigned int)strtoul(data, NULL, 10);
	Sql_GetData(sql_handle,  7, &data, NULL); hd->hunger = atoi(data);
	Sql_GetData(sql_handle,  8, &data, NULL); hd->str = atoi(data);
	Sql_GetData(sql_handle,  9, &data, NULL); hd->agi = atoi(data);
	Sql_GetData(sql_handle, 10, &data, NULL); hd->vit = atoi(data);
	Sql_GetData(sql_handle, 11, &data, NULL); hd->int_ = atoi(data);
	Sql_GetData(sql_handle, 12, &data, NULL); hd->dex = atoi(data);
	Sql_GetData(sql_handle, 13, &data, NULL); hd->luk = atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); hd->hp = atoi(data);
	Sql_GetData(sql_handle, 15, &data, NULL); hd->max_hp = atoi(data);
	Sql_GetData(sql_handle, 16, &data, NULL); hd->sp = atoi(data);
	Sql_GetData(sql_handle, 17, &data, NULL); hd->max_sp = atoi(data);
	Sql_GetData(sql_handle, 18, &data, NULL); hd->skillpts = atoi(data);
	Sql_GetData(sql_handle, 19, &data, NULL); hd->rename_flag = atoi(data);
	Sql_GetData(sql_handle, 20, &data, NULL); hd->vaporize = atoi(data);
	Sql_FreeResult(sql_handle);

	hd->intimacy = cap_value(hd->intimacy, 0, 100000);
	hd->hunger = cap_value(hd->hunger, 0, 100);

	// Load Homunculus Skill
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`lv` FROM `skill_homunculus` WHERE `homun_id`=%d", homun_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		// id
		Sql_GetData(sql_handle, 0, &data, NULL);
		i = atoi(data);
		if( i < HM_SKILLBASE || i >= HM_SKILLBASE + MAX_HOMUNSKILL )
			continue;// invalid skill id
		i = i - HM_SKILLBASE;
		hd->hskill[i].id = (unsigned short)atoi(data);

		// lv
		Sql_GetData(sql_handle, 1, &data, NULL);
		hd->hskill[i].lv = (unsigned short)atoi(data);
	}
	Sql_FreeResult(sql_handle);

	if( save_log )
		ShowInfo("Homunculus loaded (%d - %s).\n", hd->hom_id, hd->name);

	return true;
*/
}


static bool mmo_homun_tosql(HomunDB_SQL* db, const struct s_homunculus* hd, bool is_new)
{
/*
	bool flag = true;
	char esc_name[NAME_LENGTH*2+1];

	Sql_EscapeStringLen(sql_handle, esc_name, hd->name, strnlen(hd->name, NAME_LENGTH));

	if( hd->hom_id == 0 )
	{// new homunculus
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `homunculus` "
			"(`char_id`, `class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`, `rename_flag`, `vaporize`) "
			"VALUES ('%d', '%d', '%s', '%d', '%u', '%u', '%d', '%d', %d, '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			hd->char_id, hd->class_, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp, hd->max_hp, hd->sp, hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
		{
			hd->hom_id = (int)Sql_LastInsertId(sql_handle);
		}
	}
	else
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `homunculus` SET `char_id`='%d', `class`='%d',`name`='%s',`level`='%d',`exp`='%u',`intimacy`='%u',`hunger`='%d', `str`='%d', `agi`='%d', `vit`='%d', `int`='%d', `dex`='%d', `luk`='%d', `hp`='%d',`max_hp`='%d',`sp`='%d',`max_sp`='%d',`skill_point`='%d', `rename_flag`='%d', `vaporize`='%d' WHERE `homun_id`='%d'",
			hd->char_id, hd->class_, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp, hd->max_hp, hd->sp, hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize, hd->hom_id) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
		{
			SqlStmt* stmt;
			int i;

			stmt = SqlStmt_Malloc(sql_handle);
			if( SQL_ERROR == SqlStmt_Prepare(stmt, "REPLACE INTO `skill_homunculus` (`homun_id`, `id`, `lv`) VALUES (%d, ?, ?)", hd->hom_id) )
				SqlStmt_ShowDebug(stmt);
			for( i = 0; i < MAX_HOMUNSKILL; ++i )
			{
				if( hd->hskill[i].id > 0 && hd->hskill[i].lv != 0 )
				{
					SqlStmt_BindParam(stmt, 0, SQLDT_USHORT, &hd->hskill[i].id, 0);
					SqlStmt_BindParam(stmt, 1, SQLDT_USHORT, &hd->hskill[i].lv, 0);
					if( SQL_ERROR == SqlStmt_Execute(stmt) )
					{
						SqlStmt_ShowDebug(stmt);
						SqlStmt_Free(stmt);
						flag = false;
						break;
					}
				}
			}
			SqlStmt_Free(stmt);
		}
	}

	return flag;
*/
}


static bool homun_db_sql_init(HomunDB* self)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	db->homuns = db->owner->sql_handle;
	return true;
}

static void homun_db_sql_destroy(HomunDB* self)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	db->homuns = NULL;
	aFree(db);
}

static bool homun_db_sql_sync(HomunDB* self)
{
	return true;
}

static bool homun_db_sql_create(HomunDB* self, struct s_homunculus* hd)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	Sql* sql_handle = db->homuns;

}

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

static bool homun_db_sql_save(HomunDB* self, const struct s_homunculus* hd)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return mmo_homun_tosql(db, hd, false);
}

static bool homun_db_sql_load(HomunDB* self, struct s_homunculus* hd, int homun_id)
{
	HomunDB_SQL* db = (HomunDB_SQL*)self;
	return mmo_homun_fromsql(db, hd, homun_id);
}


/// public constructor
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

	// initialize to default values
	db->owner = owner;
	db->homuns = NULL;

	// other settings
	db->homun_db = db->owner->table_homuns;
	db->homun_skill_db = db->owner->table_homun_skills;

	return &db->vtable;
}
