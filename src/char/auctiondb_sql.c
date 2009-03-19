// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "auctiondb.h"
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct AuctionDB_SQL
{
	AuctionDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* auctions;       // SQL auction storage

	// other settings
	const char* auction_db;

} AuctionDB_SQL;



static bool mmo_auction_tosql(AuctionDB_SQL* db, const struct auction_data* ad, bool is_new)
{
	Sql* sql_handle = db->auctions;
	int j;
	StringBuf buf;
	SqlStmt* stmt;
	bool result = false;

	StringBuf_Init(&buf);

	if( is_new )
	{
		StringBuf_Printf(&buf, "INSERT INTO `%s` (`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`", db->auction_db);
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ",`card%d`", j);
		StringBuf_Printf(&buf, ",`auction_id`) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?");
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ",'%d'", ad->item.card[j]);
		StringBuf_AppendStr(&buf, ",?)");
	}
	else
	{
		StringBuf_Printf(&buf, "UPDATE `%s` SET `seller_id`=?, `seller_name`=?, `buyer_id`=?, `buyer_name`=?, `price`=?, `buynow`=?, `hours`=?, `timestamp`=?, `nameid`=?, `item_name`=?, `type`=?, `refine`=?, `attribute`=?", db->auction_db);
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ", `card%d` = '%d'", j, ad->item.card[j]);
		StringBuf_Printf(&buf, " WHERE `auction_id`=?", ad->auction_id);
	}

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_INT, (void*)&ad->seller_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)ad->seller_name, strnlen(ad->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_INT, (void*)&ad->buyer_id, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_STRING, (void*)ad->buyer_name, strnlen(ad->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_INT, (void*)&ad->price, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 5, SQLDT_INT, (void*)&ad->buynow, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 6, SQLDT_USHORT, (void*)&ad->hours, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 7, SQLDT_ULONG, (void*)&ad->timestamp, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 8, SQLDT_SHORT, (void*)&ad->item.nameid, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 9, SQLDT_STRING, (void*)ad->item_name, strnlen(ad->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_SHORT, (void*)&ad->type, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_CHAR, (void*)&ad->item.refine, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, SQLDT_CHAR, (void*)&ad->item.attribute, 0)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, (ad->auction_id != -1)?SQLDT_INT:SQLDT_NULL, (void*)&ad->auction_id, 0)
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	// success
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


static bool mmo_auction_fromsql(AuctionDB_SQL* db, struct auction_data* ad, int auction_id)
{
	Sql* sql_handle = db->auctions;
	StringBuf buf;
	SqlStmt* stmt;
	int i;
	bool result = false;

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf,
		"SELECT `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`");
	for( i = 0; i < MAX_SLOTS; i++ )
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `auction_id` = %d", db->auction_db, auction_id);

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT, (void*)&ad->auction_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_INT, (void*)&ad->seller_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, (void*)ad->seller_name, sizeof(ad->seller_name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_INT, (void*)&ad->buyer_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_STRING, (void*)ad->buyer_name, sizeof(ad->buyer_name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_INT, (void*)&ad->price, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_INT, (void*)&ad->buynow, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_USHORT, (void*)&ad->hours, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_UINT, (void*)&ad->timestamp, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_SHORT, (void*)&ad->item.nameid, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_STRING, (void*)ad->item_name, sizeof(ad->item_name), NULL, NULL);
	SqlStmt_BindColumn(stmt, 11, SQLDT_SHORT, (void*)&ad->type, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 12, SQLDT_CHAR, (void*)&ad->item.refine, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 13, SQLDT_CHAR, (void*)&ad->item.attribute, 0, NULL, NULL);
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 14+i, SQLDT_SHORT, (void*)&ad->item.card[i], 0, NULL, NULL);

	if( SQL_SUCCESS != SqlStmt_NextRow(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	ad->item.amount = 1;
	ad->item.identify = 1;

	// success
	result = true;

	}
	while(0);

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


static bool auction_db_sql_init(AuctionDB* self)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	db->auctions = db->owner->sql_handle;
	return true;
}

static void auction_db_sql_destroy(AuctionDB* self)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	db->auctions = NULL;
	aFree(db);
}

static bool auction_db_sql_sync(AuctionDB* self)
{
	return true;
}

static bool auction_db_sql_create(AuctionDB* self, struct auction_data* ad)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	Sql* sql_handle = db->auctions;
	return mmo_auction_tosql(db, ad, true);
}

static bool auction_db_sql_remove(AuctionDB* self, const int auction_id)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	Sql* sql_handle = db->auctions;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `auction_id` = %d", db->auction_db, auction_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static bool auction_db_sql_save(AuctionDB* self, const struct auction_data* ad)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	return mmo_auction_tosql(db, ad, false);
}

static bool auction_db_sql_load(AuctionDB* self, struct auction_data* ad, const int auction_id)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	return mmo_auction_fromsql(db, ad, auction_id);
}

/// List the auctions for the specified search results page.
static bool auction_db_sql_search(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	Sql* sql_handle = db->auctions;
	int auction_id;
	char* data;
	int i = 0, j = 0, p = 1;
	StringBuf buf;
	char esc_searchtext[24*2+1];

	Sql_EscapeStringLen(sql_handle, esc_searchtext, searchtext, strnlen(searchtext, 24));

	// fetch a list of matching auction ids
	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "SELECT `auction_id` FROM `%s` WHERE ", db->auction_db);

	switch( type )
	{
	case 0: StringBuf_Printf(&buf, "`type` = 5 OR `type` = 8"); break; // IT_ARMOR or IT_PETARMOR
	case 1: StringBuf_Printf(&buf, "`type` = 4"); break; // IT_WEAPON
	case 2: StringBuf_Printf(&buf, "`type` = 6"); break; // IT_CARD
	case 3: StringBuf_Printf(&buf, "`type` = 3"); break; // IT_ETC
	case 4: StringBuf_Printf(&buf, "`item_name` LIKE '%%%s%%'", esc_searchtext); break;
	case 5: StringBuf_Printf(&buf, "`price` <= %d", price); break;
	case 6: StringBuf_Printf(&buf, "`seller_id` = %d", char_id); break;
	case 7: StringBuf_Printf(&buf, "`buyer_id` = %d", char_id); break;
	default:StringBuf_Printf(&buf, "1"); break;
	}

	StringBuf_Printf(&buf, " ORDER BY `auction_id`");

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	StringBuf_Destroy(&buf);

	// scan through the results
	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		Sql_GetData(sql_handle, 0, &data, NULL);
		auction_id = atoi(data);

		i++;
		if( i > 5 )
		{// counting pages of total results (5 results per page)
			i = 1;
			p++;
		}

		if( p != page )
			continue; // this is not the requested page

		//NOTE: nested sql query, must use SqlStmt inside
		if( !mmo_auction_fromsql(db, &ad[j], auction_id) )
			return false;

		j++; // found results
	}

	*pages = p;
	*results = j;

	return true;
}

/// Count the number of auctions started by this character.
static int auction_db_sql_count(AuctionDB* self, const int char_id)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	Sql* sql_handle = db->auctions;
	char* data;
	int count;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT COUNT(`auction_id`) FROM `%s` WHERE `seller_id` = %d", db->auction_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL);
	count = atoi(data);
	Sql_FreeResult(sql_handle);

	return count;
}

/// Find the auction with the earliest expiration time.
static bool auction_db_sql_first(AuctionDB* self, struct auction_data* ad)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)self;
	Sql* sql_handle = db->auctions;
	char* data;
	int auction_id;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT `auction_id` FROM `%s` ORDER BY `timestamp` ASC LIMIT 1", db->auction_db) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{// no data
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL);
	auction_id = atoi(data);
	Sql_FreeResult(sql_handle);

	return mmo_auction_fromsql(db, ad, auction_id);
}


/// public constructor
AuctionDB* auction_db_sql(CharServerDB_SQL* owner)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)aCalloc(1, sizeof(AuctionDB_SQL));

	// set up the vtable
	db->vtable.init      = &auction_db_sql_init;
	db->vtable.destroy   = &auction_db_sql_destroy;
	db->vtable.sync      = &auction_db_sql_sync;
	db->vtable.create    = &auction_db_sql_create;
	db->vtable.remove    = &auction_db_sql_remove;
	db->vtable.save      = &auction_db_sql_save;
	db->vtable.load      = &auction_db_sql_load;
	db->vtable.search    = &auction_db_sql_search;
	db->vtable.count     = &auction_db_sql_count;
	db->vtable.first     = &auction_db_sql_first;

	// initialize to default values
	db->owner = owner;
	db->auctions = NULL;

	// other settings
	db->auction_db = db->owner->table_auctions;

	return &db->vtable;
}
