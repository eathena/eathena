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
#include "char.h"
#include <stdlib.h>
#include <string.h>


/// Saves an array of 'item' entries into the specified table.
bool memitemdata_to_sql(Sql* sql_handle, const struct item items[], int max, int id, const char* tablename, const char* selectoption)
{
	StringBuf buf;
	SqlStmt* stmt;
	int i;
	int j;
	struct item item; // temp storage variable
	bool* flag; // bit array for inventory matching
	bool found;

	// The following code compares inventory with current database values
	// and performs modification/deletion/insertion only on relevant rows.
	// This approach is more complicated than a trivial delete&insert, but
	// it significantly reduces cpu load on the database server.

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `%s`='%d'", tablename, selectoption, id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return 1;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &item.id,          0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &item.nameid,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &item.amount,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &item.equip,       0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,   &item.identify,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,   &item.refine,      0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,   &item.attribute,   0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,   &item.expire_time, 0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SqlStmt_BindColumn(stmt, 8+j, SQLDT_SHORT, &item.card[j], 0, NULL, NULL);

	// bit array indicating which inventory items have already been matched
	flag = (bool*) aCallocA(max, sizeof(bool));

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		found = false;
		// search for the presence of the item in the char's inventory
		for( i = 0; i < max; ++i )
		{
			// skip empty and already matched entries
			if( items[i].nameid == 0 || flag[i] )
				continue;

			if( items[i].nameid == item.nameid
			&&  items[i].card[0] == item.card[0]
			&&  items[i].card[2] == item.card[2]
			&&  items[i].card[3] == item.card[3]
			) {	//They are the same item.
				ARR_FIND( 0, MAX_SLOTS, j, items[i].card[j] != item.card[j] );
				if( j == MAX_SLOTS &&
				    items[i].amount == item.amount &&
				    items[i].equip == item.equip &&
				    items[i].identify == item.identify &&
				    items[i].refine == item.refine &&
				    items[i].attribute == item.attribute &&
				    items[i].expire_time == item.expire_time )
				;	//Do nothing.
				else
				{
					// update all fields.
					StringBuf_Clear(&buf);
					StringBuf_Printf(&buf, "UPDATE `%s` SET `amount`='%d', `equip`='%d', `identify`='%d', `refine`='%d',`attribute`='%d', `expire_time`='%u'",
						tablename, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time);
					for( j = 0; j < MAX_SLOTS; ++j )
						StringBuf_Printf(&buf, ", `card%d`=%d", j, items[i].card[j]);
					StringBuf_Printf(&buf, " WHERE `id`='%d' LIMIT 1", item.id);
					
					if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
						Sql_ShowDebug(sql_handle);
				}

				found = flag[i] = true; //Item dealt with,
				break; //skip to next item in the db.
			}
		}
		if( !found )
		{// Item not present in inventory, remove it.
			if( SQL_ERROR == Sql_Query(sql_handle, "DELETE from `%s` where `id`='%d'", tablename, item.id) )
				Sql_ShowDebug(sql_handle);
		}
	}
	SqlStmt_Free(stmt);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`", tablename, selectoption);
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_AppendStr(&buf, ") VALUES ");

	found = false;
	// insert non-matched items into the db as new items
	for( i = 0; i < max; ++i )
	{
		// skip empty and already matched entries
		if( items[i].nameid == 0 || flag[i] )
			continue;

		if( found )
			StringBuf_AppendStr(&buf, ",");
		else
			found = true;

		StringBuf_Printf(&buf, "('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u'",
			id, items[i].nameid, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time);
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ", '%d'", items[i].card[j]);
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);
	aFree(flag);

	return true;
}
