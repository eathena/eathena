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
#include "../common/utils.h"
#include "char.h"
#include "charlog.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_status.h"
#include "inter.h"
#include "map.h"
#include <stdlib.h>
#include <string.h>


// private declarations
char char_db[256] = "char";
char scdata_db[256] = "sc_data";
char cart_db[256] = "cart_inventory";
char inventory_db[256] = "inventory";
char storage_db[256] = "storage";
char reg_db[256] = "global_reg_value";
char skill_db[256] = "skill";
char memo_db[256] = "memo";
char guild_db[256] = "guild";
char guild_alliance_db[256] = "guild_alliance";
char guild_castle_db[256] = "guild_castle";
char guild_expulsion_db[256] = "guild_expulsion";
char guild_member_db[256] = "guild_member";
char guild_position_db[256] = "guild_position";
char guild_skill_db[256] = "guild_skill";
char guild_storage_db[256] = "guild_storage";
char party_db[256] = "party";
char pet_db[256] = "pet";
char mail_db[256] = "mail"; // MAIL SYSTEM
char auction_db[256] = "auction"; // Auctions System
char friend_db[256] = "friends";
char hotkey_db[256] = "hotkey";
char quest_db[256] = "quest";
char quest_obj_db[256] = "quest_objective";





/// Saves an array of 'item' entries into the specified table.
int memitemdata_to_sql(const struct item items[], int max, int id, int tableswitch)
{
	StringBuf buf;
	SqlStmt* stmt;
	int i;
	int j;
	const char* tablename;
	const char* selectoption;
	struct item item; // temp storage variable
	bool* flag; // bit array for inventory matching
	bool found;

	switch (tableswitch) {
	case TABLE_INVENTORY:     tablename = inventory_db;     selectoption = "char_id";    break;
	case TABLE_CART:          tablename = cart_db;          selectoption = "char_id";    break;
	case TABLE_STORAGE:       tablename = storage_db;       selectoption = "account_id"; break;
	case TABLE_GUILD_STORAGE: tablename = guild_storage_db; selectoption = "guild_id";   break;
	default:
		ShowError("Invalid table name!\n");
		return 1;
	}


	// The following code compares inventory with current database values
	// and performs modification/deletion/insertion only on relevant rows.
	// This approach is more complicated than a trivial delete&insert, but
	// it significantly reduces cpu load on the database server.

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
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

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &item.id,        0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &item.nameid,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &item.amount,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &item.equip,     0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,   &item.identify,  0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,   &item.refine,    0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,   &item.attribute, 0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SqlStmt_BindColumn(stmt, 7+j, SQLDT_SHORT, &item.card[j], 0, NULL, NULL);

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
				    items[i].attribute == item.attribute )
				;	//Do nothing.
				else
				{
					// update all fields.
					StringBuf_Clear(&buf);
					StringBuf_Printf(&buf, "UPDATE `%s` SET `amount`='%d', `equip`='%d', `identify`='%d', `refine`='%d',`attribute`='%d'",
						tablename, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute);
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
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`", tablename, selectoption);
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

		StringBuf_Printf(&buf, "('%d', '%d', '%d', '%d', '%d', '%d', '%d'",
			id, items[i].nameid, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute);
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ", '%d'", items[i].card[j]);
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);
	aFree(flag);

	return 0;
}


/*----------------------------------------------------------------------------------------------------------*/
/* Delete char - davidsiaw */
/*----------------------------------------------------------------------------------------------------------*/
/* Returns 0 if successful
 * Returns < 0 for error
 */
int char_delete(int char_id)
{
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1]; //Name needs be escaped.
	int account_id, party_id, guild_id, hom_id, base_level, partner_id, father_id, mother_id;
	char* data;
	size_t len;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name`,`account_id`,`party_id`,`guild_id`,`base_level`,`homun_id`,`partner_id`,`father`,`mother` FROM `%s` WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		ShowError("delete_char_sql: Unable to fetch character data, deletion aborted.\n");
		Sql_FreeResult(sql_handle);
		return -1;
	}

	Sql_GetData(sql_handle, 0, &data, &len); safestrncpy(name, data, NAME_LENGTH);
	Sql_GetData(sql_handle, 1, &data, NULL); account_id = atoi(data);
	Sql_GetData(sql_handle, 2, &data, NULL); party_id = atoi(data);
	Sql_GetData(sql_handle, 3, &data, NULL); guild_id = atoi(data);
	Sql_GetData(sql_handle, 4, &data, NULL); base_level = atoi(data);
	Sql_GetData(sql_handle, 5, &data, NULL); hom_id = atoi(data);
	Sql_GetData(sql_handle, 6, &data, NULL); partner_id = atoi(data);
	Sql_GetData(sql_handle, 7, &data, NULL); father_id = atoi(data);
	Sql_GetData(sql_handle, 8, &data, NULL); mother_id = atoi(data);

	Sql_EscapeStringLen(sql_handle, esc_name, name, min(len, NAME_LENGTH));
	Sql_FreeResult(sql_handle);

	//check for config char del condition [Lupus]
	if( ( char_del_level > 0 && base_level >= char_del_level )
	 || ( char_del_level < 0 && base_level <= -char_del_level )
	) {
			ShowInfo("Char deletion aborted: %s, BaseLevel: %i\n", name, base_level);
			return -1;
	}

	/* Divorce [Wizputer] */
	if( partner_id )
		char_divorce(char_id, partner_id);

	/* De-addopt [Zephyrus] */
	if( father_id || mother_id )
	{ // Char is Baby
		unsigned char buf[64];

		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `child`='0' WHERE `char_id`='%d' OR `char_id`='%d'", char_db, father_id, mother_id) )
			Sql_ShowDebug(sql_handle);
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '410'AND (`char_id`='%d' OR `char_id`='%d')", skill_db, father_id, mother_id) )
			Sql_ShowDebug(sql_handle);

		WBUFW(buf,0) = 0x2b25;
		WBUFL(buf,2) = father_id;
		WBUFL(buf,6) = mother_id;
		WBUFL(buf,10) = char_id; // Baby
		mapif_sendall(buf,14);
	}

	//Make the character leave the party [Skotlex]
	if (party_id)
		inter_party_leave(party_id, account_id, char_id);

	/* delete char's pet */
	//Delete the hatched pet if you have one...
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d' AND `incuvate` = '0'", pet_db, char_id) )
		Sql_ShowDebug(sql_handle);

	//Delete all pets that are stored in eggs (inventory + cart)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE pet_id IN "
		"(SELECT card1|card2<<2 FROM `%s` WHERE char_id = '%d' AND card0 = -256"
		" UNION"
		" SELECT card1|card2<<2 FROM `%s` WHERE char_id = '%d' AND card0 = -256)",
		pet_db, inventory_db, char_id, cart_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* remove homunculus */ 
	if (hom_id)
		inter_homun_delete(hom_id);

	/* delete char's friends list */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", friend_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete char from other's friend list */
	//NOTE: Won't this cause problems for people who are already online? [Skotlex]
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `friend_id` = '%d'", friend_db, char_id) )
		Sql_ShowDebug(sql_handle);

#ifdef HOTKEY_SAVING
	/* delete hotkeys */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", hotkey_db, char_id) )
		Sql_ShowDebug(sql_handle);
#endif

	/* delete inventory */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", inventory_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete cart inventory */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", cart_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete memo areas */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", memo_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete character registry */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
		Sql_ShowDebug(sql_handle);
	
	/* delete skills */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", skill_db, char_id) )
		Sql_ShowDebug(sql_handle);
	
#ifdef ENABLE_SC_SAVING
	/* status changes */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", scdata_db, account_id, char_id) )
		Sql_ShowDebug(sql_handle);
#endif

	charlog_log(char_id, account_id, -1, name, "Deleted char");

	/* delete character */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* No need as we used inter_guild_leave [Skotlex]
	// Also delete info from guildtables.
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", guild_member_db, char_id) )
		Sql_ShowDebug(sql_handle);
	*/

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `master` = '%s'", guild_db, esc_name) )
		Sql_ShowDebug(sql_handle);
	else if( Sql_NumRows(sql_handle) > 0 )
		mapif_parse_BreakGuild(0,guild_id);
	else if( guild_id )
		inter_guild_leave(guild_id, account_id, char_id);// Leave your guild.
	return 0;
}
