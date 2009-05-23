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


/// internal structure
typedef struct CharDB_SQL
{
	CharDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* chars;       // SQL character storage

	// other settings
	bool case_sensitive;
	const char* char_db;
	const char* memo_db;
	const char* inventory_db;
	const char* cart_db;
	const char* skill_db;

} CharDB_SQL;

/// internal structure
typedef struct CharDBIterator_SQL
{
	CharDBIterator vtable;    // public interface

	CharDB_SQL* db;
	int* ids_arr;
	int ids_num;
	int pos;
	bool has_more;
} CharDBIterator_SQL;



static bool mmo_char_fromsql(CharDB_SQL* db, struct mmo_charstatus* p, int char_id, bool load_everything)
{
	Sql* sql_handle = db->chars;
	RankDB* rankdb = db->owner->rankdb;

	int i,j;
	StringBuf buf;
	SqlStmt* stmt;
	char last_map[MAP_NAME_LENGTH_EXT];
	char save_map[MAP_NAME_LENGTH_EXT];
	char point_map[MAP_NAME_LENGTH_EXT];
	struct point tmp_point;
	struct item tmp_item;
	struct s_skill tmp_skill;

	memset(p, 0, sizeof(struct mmo_charstatus));
	
	if (save_log) ShowInfo("Char load request (%d)\n", char_id);

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
		"`save_map`,`save_x`,`save_y`,`partner_id`,`father`,`mother`,`child`,`fame`"
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
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 20, SQLDT_USHORT, &p->status_point, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 21, SQLDT_USHORT, &p->skill_point, 0, NULL, NULL)
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
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 47, SQLDT_INT,    &p->fame, 0, NULL, NULL) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return false;
	}
	if( SQL_ERROR == SqlStmt_NextRow(stmt) )
	{
		ShowError("Requested non-existant character id: %d!\n", char_id);
		SqlStmt_Free(stmt);
		return false;	
	}
	p->last_point.map = mapindex_name2id(last_map);
	p->save_point.map = mapindex_name2id(save_map);
	switch( p->class_ )
	{// TODO make the map-server responsible for this? (handle class2rankid logic) [FlavioJS]
	case JOB_BLACKSMITH:
	case JOB_WHITESMITH:
	case JOB_BABY_BLACKSMITH:
		p->fame = rankdb->get_points(rankdb, RANK_BLACKSMITH, p->char_id);
		break;
	case JOB_ALCHEMIST:
	case JOB_CREATOR:
	case JOB_BABY_ALCHEMIST:
		p->fame = rankdb->get_points(rankdb, RANK_ALCHEMIST, p->char_id);
		break;
	case JOB_TAEKWON:
		p->fame = rankdb->get_points(rankdb, RANK_TAEKWON, p->char_id);
		break;
	}

	if (!load_everything) // For quick selection of data when displaying the char menu
	{
		SqlStmt_Free(stmt);
		return true;
	}

	//read memo data
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`=? ORDER by `memo_id` LIMIT %d", db->memo_db, MAX_MEMOPOINTS)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &point_map, sizeof(point_map), NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_point.x, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_point.y, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_MEMOPOINTS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		tmp_point.map = mapindex_name2id(point_map);
		memcpy(&p->memo_point[i], &tmp_point, sizeof(tmp_point));
	}
	//read inventory
	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
	for( i = 0; i < MAX_SLOTS; ++i )
		StringBuf_Printf(&buf, ", `card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id`=? LIMIT %d", db->inventory_db, MAX_INVENTORY);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_item.id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_item.nameid, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_item.amount, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &tmp_item.equip, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,   &tmp_item.identify, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,   &tmp_item.refine, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,   &tmp_item.attribute, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < MAX_SLOTS; ++i )
		if( SQL_ERROR == SqlStmt_BindColumn(stmt, 7+i, SQLDT_SHORT, &tmp_item.card[i], 0, NULL, NULL) )
			SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_INVENTORY && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->inventory[i], &tmp_item, sizeof(tmp_item));

	//read cart
	//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	StringBuf_Clear(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id`=? LIMIT %d", db->cart_db, MAX_CART);

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_item.id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_item.nameid, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_item.amount, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &tmp_item.equip, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,   &tmp_item.identify, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,   &tmp_item.refine, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,   &tmp_item.attribute, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < MAX_SLOTS; ++i )
		if( SQL_ERROR == SqlStmt_BindColumn(stmt, 7+i, SQLDT_SHORT, &tmp_item.card[i], 0, NULL, NULL) )
			SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_CART && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->cart[i], &tmp_item, sizeof(tmp_item));

	//read skill
	//`skill` (`char_id`, `id`, `lv`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `id`, `lv` FROM `%s` WHERE `char_id`=? LIMIT %d", db->skill_db, MAX_SKILL)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_USHORT, &tmp_skill.id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_USHORT, &tmp_skill.lv, 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);
	tmp_skill.flag = 0;

	for( i = 0; i < MAX_SKILL && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		if( tmp_skill.id < ARRAYLENGTH(p->skill) )
			memcpy(&p->skill[tmp_skill.id], &tmp_skill, sizeof(tmp_skill));
		else
			ShowWarning("mmo_char_fromsql: ignoring invalid skill (id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", tmp_skill.id, tmp_skill.lv, p->name, p->account_id, p->char_id);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	if( save_log )
		ShowInfo("Loaded char (%d - %s)\n", char_id, p->name);	//ok. all data load successfuly!

	return true;
}


static bool mmo_char_tosql(CharDB_SQL* db, struct mmo_charstatus* p, bool is_new)
{
	Sql* sql_handle = db->chars;

	int i = 0;
	int count = 0;
	int diff = 0;
	struct mmo_charstatus tmp;
	struct mmo_charstatus* cp = &tmp;
	StringBuf buf;
	SqlStmt* stmt = NULL;
	bool result = false;

	//TODO: add cache
	//TODO: if "is_new", don't consider doing this step and just 'memset' it or something
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
			"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%d',`skill_point`='%d',"
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
 		(p->karma != cp->karma) || (p->manner != cp->manner) ||
		(p->fame != cp->fame)
	)
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',"
			"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',"
			"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d',"
			"`karma`='%d',`manner`='%d', `fame`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			db->char_db, p->class_,
			p->hair, p->hair_color, p->clothes_color,
			p->partner_id, p->father, p->mother, p->child,
			p->karma, p->manner, p->fame,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
		}
	}

	//inventory data
	if( memcmp(p->inventory, cp->inventory, sizeof(p->inventory)) )
		memitemdata_to_sql(db->chars, p->inventory, MAX_INVENTORY, p->char_id, db->inventory_db, "char_id");

	//cart data
	if( memcmp(p->cart, cp->cart, sizeof(p->cart)) )
		memitemdata_to_sql(db->chars, p->cart, MAX_CART, p->char_id, db->cart_db, "char_id");

	//memo points
	if( memcmp(p->memo_point, cp->memo_point, sizeof(p->memo_point)) )
	{
		char esc_mapname[NAME_LENGTH*2+1];

		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->memo_db, p->char_id) )
			Sql_ShowDebug(sql_handle);

		//insert here.
		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ", db->memo_db);
		for( i = 0, count = 0; i < MAX_MEMOPOINTS; ++i )
		{
			if( p->memo_point[i].map )
			{
				if( count )
					StringBuf_AppendStr(&buf, ",");
				Sql_EscapeString(sql_handle, esc_mapname, mapindex_id2name(p->memo_point[i].map));
				StringBuf_Printf(&buf, "('%d', '%s', '%d', '%d')", p->char_id, esc_mapname, p->memo_point[i].x, p->memo_point[i].y);
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
				Sql_ShowDebug(sql_handle);
		}
	}

/*
	//FIXME: is this neccessary? [ultramage]
	for(i=0;i<MAX_SKILL;i++)
		if ((p->skill[i].lv != 0) && (p->skill[i].id == 0))
			p->skill[i].id = i; // Fix skill tree
*/

	//skills
	if( memcmp(p->skill, cp->skill, sizeof(p->skill)) )
	{
		//`skill` (`char_id`, `id`, `lv`)
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->skill_db, p->char_id) )
			Sql_ShowDebug(sql_handle);

		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`id`,`lv`) VALUES ", db->skill_db);
		//insert here.
		for( i = 0, count = 0; i < MAX_SKILL; ++i )
		{
			if(p->skill[i].id && p->skill[i].flag!=1)
			{
				if( count )
					StringBuf_AppendStr(&buf, ",");
				StringBuf_Printf(&buf, "('%d','%d','%d')", p->char_id, p->skill[i].id, (p->skill[i].flag == 0 ? p->skill[i].lv : p->skill[i].flag - 2));
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
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


static bool char_db_sql_init(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	db->chars = db->owner->sql_handle;
	return true;
}

static void char_db_sql_destroy(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	db->chars = NULL;
	aFree(db);
}

static bool char_db_sql_sync(CharDB* self)
{
	return true;
}

static bool char_db_sql_create(CharDB* self, struct mmo_charstatus* cd)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	return mmo_char_tosql(db, cd, true);
}

static bool char_db_sql_remove(CharDB* self, const int char_id)
{
	/*

	// delete memo areas
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->memo_db, char_id) )
		Sql_ShowDebug(sql_handle);

	// delete inventory
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->inventory_db, char_id) )
		Sql_ShowDebug(sql_handle);

	// delete cart inventory
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->cart_db, char_id) )
		Sql_ShowDebug(sql_handle);

	// delete skills
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", db->skill_db, char_id) )
		Sql_ShowDebug(sql_handle);

	// delete character
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);

	*/

	// not implemented yet
	return false;
}

static bool char_db_sql_save(CharDB* self, const struct mmo_charstatus* ch)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	return mmo_char_tosql(db, (struct mmo_charstatus*)ch, false);
}

static bool char_db_sql_load_num(CharDB* self, struct mmo_charstatus* ch, int char_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	return mmo_char_fromsql(db, ch, char_id, true);
}

static bool char_db_sql_load_str(CharDB* self, struct mmo_charstatus* ch, const char* name)
{
//	CharDB_SQL* db = (CharDB_SQL*)self;
	int char_id;

	// find char id
	if( !self->name2id(self, name, &char_id, NULL) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}

static bool char_db_sql_load_slot(CharDB* self, struct mmo_charstatus* ch, int account_id, int slot)
{
//	CharDB_SQL* db = (CharDB_SQL*)self;
	int char_id;

	// find char id
	if( !self->slot2id(self, account_id, slot, &char_id) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}

static bool char_db_sql_id2name(CharDB* self, int char_id, char name[NAME_LENGTH])
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
		safestrncpy(name, data, sizeof(name));
	Sql_FreeResult(sql_handle);

	return true;
}

static bool char_db_sql_name2id(CharDB* self, const char* name, int* char_id, int* account_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	char esc_name[2*NAME_LENGTH+1];
	char* data;

	Sql_EscapeString(sql_handle, esc_name, name);

	// get the list of char IDs for this char name
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`account_id` FROM `%s` WHERE `name`= %s '%s'",
		db->char_db, (db->case_sensitive ? "BINARY" : ""), esc_name) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( Sql_NumRows(sql_handle) > 1 )
	{// serious problem - duplicit char name
		ShowError("char_db_sql_name2id: multiple chars found when looking up char '%s'!\n", name);
		Sql_FreeResult(sql_handle);
		return false;
	}

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

static bool char_db_sql_slot2id(CharDB* self, int account_id, int slot, int* char_id)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	Sql* sql_handle = db->chars;
	char* data;

	// get the list of char IDs for this acc/slot
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d'",
		db->char_db, account_id, slot) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( Sql_NumRows(sql_handle) > 1 )
	{// serious problem - multiple chars on same slot
		ShowError("char_db_sql_slot2id: multiple chars found when looking up acc/slot '%d'/'%d'!\n", account_id, slot);
		Sql_FreeResult(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no such entry
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL);
	if( char_id != NULL )
		*char_id = atoi(data);
	Sql_FreeResult(sql_handle);

	return true;
}

/// Private. Fills the cache of the iterator with ids.
static void char_db_sql_iter_P_fillcache(CharDBIterator_SQL* iter)
{
	CharDB_SQL* db = iter->db;
	Sql* sql_handle = db->chars;
	int res;
	int last_id = 0;
	bool has_last_id = false;

	if( iter->ids_num > 0 )
	{
		last_id = iter->ids_arr[iter->ids_num-1];
		has_last_id = true;
	}

	if( has_last_id )
		res = Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` WHERE `char_id`>%d ORDER BY `char_id` ASC LIMIT %d", db->char_db, last_id, CHARDBITERATOR_MAXCACHE+1);
	else
		res = Sql_Query(sql_handle, "SELECT `char_id` FROM `%s` ORDER BY `char_id` ASC LIMIT %d", db->char_db, CHARDBITERATOR_MAXCACHE+1);
	if( res == SQL_ERROR )
	{
		Sql_ShowDebug(sql_handle);
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else if( Sql_NumRows(sql_handle) == 0 )
	{
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else
	{
		int i;

		if( Sql_NumRows(sql_handle) > CHARDBITERATOR_MAXCACHE )
		{
			iter->has_more = true;
			iter->ids_num = CHARDBITERATOR_MAXCACHE;
		}
		else
		{
			iter->has_more = false;
			iter->ids_num = (int)Sql_NumRows(sql_handle);
		}
		if( has_last_id )
		{
			++iter->ids_num;
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->ids_arr[0] = last_id;
			iter->pos = 0;
			i = 1;
		}
		else
		{
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->pos = -1;
			i = 0;
		}

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
}

/// Destroys this iterator, releasing all allocated memory (including itself).
static void char_db_sql_iter_destroy(CharDBIterator* self)
{
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}

/// Fetches the next character.
static bool char_db_sql_iter_next(CharDBIterator* self, struct mmo_charstatus* ch)
{
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)self;
	CharDB_SQL* db = (CharDB_SQL*)iter->db;
	Sql* sql_handle = db->chars;

	while( iter->pos+1 >= iter->ids_num )
	{
		if( !iter->has_more )
			return false;
		char_db_sql_iter_P_fillcache(iter);
	}

	++iter->pos;
	return mmo_char_fromsql(db, ch, iter->ids_arr[iter->pos], true);
}

/// Returns an iterator over all the characters.
static CharDBIterator* char_db_sql_iterator(CharDB* self)
{
	CharDB_SQL* db = (CharDB_SQL*)self;
	CharDBIterator_SQL* iter = (CharDBIterator_SQL*)aCalloc(1, sizeof(CharDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &char_db_sql_iter_destroy;
	iter->vtable.next    = &char_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;
	iter->has_more = true;// auto load on next

	return &iter->vtable;
}

/// Returns an iterator over all the characters of the account.
static CharDBIterator* char_db_sql_characters(CharDB* self, int account_id)
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
	iter->has_more = false;

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


/// public constructor
CharDB* char_db_sql(CharServerDB_SQL* owner)
{
	CharDB_SQL* db = (CharDB_SQL*)aCalloc(1, sizeof(CharDB_SQL));

	// set up the vtable
	db->vtable.init      = &char_db_sql_init;
	db->vtable.destroy   = &char_db_sql_destroy;
	db->vtable.create    = &char_db_sql_create;
	db->vtable.remove    = &char_db_sql_remove;
	db->vtable.sync      = &char_db_sql_sync;
	db->vtable.save      = &char_db_sql_save;
	db->vtable.load_num  = &char_db_sql_load_num;
	db->vtable.load_str  = &char_db_sql_load_str;
	db->vtable.load_slot = &char_db_sql_load_slot;
	db->vtable.id2name   = &char_db_sql_id2name;
	db->vtable.name2id   = &char_db_sql_name2id;
	db->vtable.slot2id   = &char_db_sql_slot2id;
	db->vtable.iterator  = &char_db_sql_iterator;
	db->vtable.characters = &char_db_sql_characters;

	// initialize to default values
	db->owner = owner;
	db->chars = NULL;

	// other settings
	db->case_sensitive = false;
	db->char_db = db->owner->table_chars;
	db->memo_db = db->owner->table_memos;
	db->inventory_db = db->owner->table_inventories;
	db->cart_db = db->owner->table_carts;
	db->skill_db = db->owner->table_skills;

	return &db->vtable;
}
