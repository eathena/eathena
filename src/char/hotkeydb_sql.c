// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "hotkeydb.h"
#include <stdlib.h>


/// internal structure
typedef struct HotkeyDB_SQL
{
	HotkeyDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* hotkeys;       // SQL hotkey storage

	// other settings
	char hotkey_db[32];

} HotkeyDB_SQL;

/// internal functions
static bool hotkey_db_sql_init(HotkeyDB* self);
static void hotkey_db_sql_destroy(HotkeyDB* self);
static bool hotkey_db_sql_sync(HotkeyDB* self);
static bool hotkey_db_sql_remove(HotkeyDB* self, const int char_id);
static bool hotkey_db_sql_save(HotkeyDB* self, const hotkeylist* list, const int char_id);
static bool hotkey_db_sql_load(HotkeyDB* self, hotkeylist* list, const int char_id);

static bool mmo_hotkeylist_fromsql(HotkeyDB_SQL* db, hotkeylist* list, int char_id);
static bool mmo_hotkeylist_tosql(HotkeyDB_SQL* db, const hotkeylist* list, int char_id);

/// public constructor
HotkeyDB* hotkey_db_sql(CharServerDB_SQL* owner)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)aCalloc(1, sizeof(HotkeyDB_SQL));

	// set up the vtable
	db->vtable.init      = &hotkey_db_sql_init;
	db->vtable.destroy   = &hotkey_db_sql_destroy;
	db->vtable.sync      = &hotkey_db_sql_sync;
	db->vtable.remove    = &hotkey_db_sql_remove;
	db->vtable.save      = &hotkey_db_sql_save;
	db->vtable.load      = &hotkey_db_sql_load;

	// initialize to default values
	db->owner = owner;
	db->hotkeys = NULL;
	// other settings
	safestrncpy(db->hotkey_db, "hotkey", sizeof(db->hotkey_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


static bool hotkey_db_sql_init(HotkeyDB* self)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)self;
	db->hotkeys = db->owner->sql_handle;
	return true;
}

static void hotkey_db_sql_destroy(HotkeyDB* self)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)self;
	db->hotkeys = NULL;
	aFree(db);
}

static bool hotkey_db_sql_sync(HotkeyDB* self)
{
	return true;
}

static bool hotkey_db_sql_remove(HotkeyDB* self, const int char_id)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)self;
	Sql* sql_handle = db->hotkeys;

	//TODO: doesn't return proper value

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->hotkey_db, char_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}

static bool hotkey_db_sql_save(HotkeyDB* self, const hotkeylist* list, const int char_id)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)self;
	return mmo_hotkeylist_tosql(db, list, char_id);
}

static bool hotkey_db_sql_load(HotkeyDB* self, hotkeylist* list, const int char_id)
{
	HotkeyDB_SQL* db = (HotkeyDB_SQL*)self;
	return mmo_hotkeylist_fromsql(db, list, char_id);
}


static bool mmo_hotkeylist_fromsql(HotkeyDB_SQL* db, hotkeylist* list, int char_id)
{
/*
#ifdef HOTKEY_SAVING
	struct hotkey tmp_hotkey;
	int hotkey_num;
#endif

	//`hotkey` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `hotkey`, `type`, `itemskill_id`, `skill_lvl` FROM `%s` WHERE `char_id`=?", hotkey_db)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &hotkey_num, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UCHAR,  &tmp_hotkey.type, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT,   &tmp_hotkey.id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &tmp_hotkey.lv, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		if( hotkey_num >= 0 && hotkey_num < MAX_HOTKEYS )
			memcpy(&p->hotkeys[hotkey_num], &tmp_hotkey, sizeof(tmp_hotkey));
		else
			ShowWarning("mmo_char_fromsql: ignoring invalid hotkey (hotkey=%d,type=%u,id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", hotkey_num, tmp_hotkey.type, tmp_hotkey.id, tmp_hotkey.lv, p->name, p->account_id, p->char_id);
	}
*/
	return true;
}

static bool mmo_hotkeylist_tosql(HotkeyDB_SQL* db, const hotkeylist* list, int char_id)
{
/*
	StringBuf_Printf(&buf, "REPLACE INTO `%s` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`) VALUES ", hotkey_db);
	diff = 0;
	for(i = 0; i < ARRAYLENGTH(p->hotkeys); i++){
		if(memcmp(&p->hotkeys[i], &cp->hotkeys[i], sizeof(struct hotkey)))
		{
			if( diff )
				StringBuf_AppendStr(&buf, ",");// not the first hotkey
			StringBuf_Printf(&buf, "('%d','%u','%u','%u','%u')", p->char_id, (unsigned int)i, (unsigned int)p->hotkeys[i].type, p->hotkeys[i].id , (unsigned int)p->hotkeys[i].lv);
			diff = 1;
		}
	}
	if(diff) {
		if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
			Sql_ShowDebug(sql_handle);
		else
			strcat(save_status, " hotkeys");
	}

	StringBuf_Destroy(&buf);
*/
	return true;
}
