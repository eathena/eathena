// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "skilldb.h"
#include <stdlib.h>
#include <string.h>


/// Internal structure.
/// @private
typedef struct SkillDB_SQL
{
	// public interface
	SkillDB vtable;

	// state
	CharServerDB_SQL* owner;
	Sql* skills;

	// settings
	const char* skill_db;

} SkillDB_SQL;


/// @private
static bool mmo_skilllist_fromsql(SkillDB_SQL* db, skilllist* list, int char_id)
{
	Sql* sql_handle = db->skills;
	SqlStmt* stmt = NULL;
	struct s_skill tmp_skill;
	bool result = false;
	int i;

	memset(list, 0, sizeof(*list));

	do
	{

	//`skill` (`char_id`, `id`, `lv`)
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `id`, `lv` FROM `%s` WHERE `char_id`=? LIMIT %d", db->skill_db, MAX_SKILL)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_USHORT, &tmp_skill.id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT, &tmp_skill.lv, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);
	tmp_skill.flag = 0;

	for( i = 0; i < MAX_SKILL && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		if( tmp_skill.id < MAX_SKILL )
			memcpy(&(*list)[tmp_skill.id], &tmp_skill, sizeof(tmp_skill));
		else
			ShowWarning("mmo_char_fromsql: ignoring invalid skill (id=%u,lv=%u) of character with CID=%d\n", tmp_skill.id, tmp_skill.lv, char_id);
	}

	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	return result;
}


/// @private
static bool mmo_skilllist_tosql(SkillDB_SQL* db, const skilllist* list, int char_id)
{
	Sql* sql_handle = db->skills;
	StringBuf buf;
	bool result = false;
	int i, count;

	StringBuf_Init(&buf);

	do
	{

	//`skill` (`char_id`, `id`, `lv`)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->skill_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`id`,`lv`) VALUES ", db->skill_db);
	for( i = 0, count = 0; i < MAX_SKILL; ++i )
	{
		int id = (*list)[i].id;
		int lv = (*list)[i].lv;
		int flag = (*list)[i].flag;

		//FIXME: is this neccessary? [ultramage]
		if( id == 0 && lv != 0 )
			id = i; // Fix skill tree

		if( id == 0 || flag == 1 )
			continue;

		if( flag != 0 )
			lv = flag - 2;

		if( count != 0)
			StringBuf_AppendStr(&buf, ",");

		StringBuf_Printf(&buf, "('%d','%d','%d')", char_id, id, lv);
		++count;
	}

	if( count > 0 )
	{
		if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		{
			Sql_ShowDebug(sql_handle);
			break;
		}
	}

	result = true;

	}
	while(0);

	StringBuf_Destroy(&buf);
	return result;
}


/// @protected
static bool skill_db_sql_init(SkillDB* self)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	db->skills = db->owner->sql_handle;
	return true;
}


/// @protected
static void skill_db_sql_destroy(SkillDB* self)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	db->skills = NULL;
	aFree(db);
}


/// @protected
static bool skill_db_sql_sync(SkillDB* self, bool force)
{
	return true;
}


/// @protected
static bool skill_db_sql_remove(SkillDB* self, const int char_id)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	Sql* sql_handle = db->skills;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->skill_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}


/// @protected
static bool skill_db_sql_save(SkillDB* self, const skilllist* list, const int char_id)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	return mmo_skilllist_tosql(db, list, char_id);
}


/// @protected
static bool skill_db_sql_load(SkillDB* self, skilllist* list, const int char_id)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	return mmo_skilllist_fromsql(db, list, char_id);
}


/// Returns an iterator over all skill lists.
/// @protected
static CSDBIterator* skill_db_sql_iterator(SkillDB* self)
{
	SkillDB_SQL* db = (SkillDB_SQL*)self;
	return csdb_sql_iterator(db->skills, db->skill_db, "char_id");
}


/// Constructs a new SkillDB interface.
/// @protected
SkillDB* skill_db_sql(CharServerDB_SQL* owner)
{
	SkillDB_SQL* db = (SkillDB_SQL*)aCalloc(1, sizeof(SkillDB_SQL));

	// set up the vtable
	db->vtable.p.init    = &skill_db_sql_init;
	db->vtable.p.destroy = &skill_db_sql_destroy;
	db->vtable.p.sync    = &skill_db_sql_sync;
	db->vtable.remove    = &skill_db_sql_remove;
	db->vtable.save      = &skill_db_sql_save;
	db->vtable.load      = &skill_db_sql_load;
	db->vtable.iterator  = &skill_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->skills = NULL;

	// other settings
	db->skill_db = db->owner->table_skills;

	return &db->vtable;
}
