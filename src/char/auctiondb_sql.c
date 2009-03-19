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
		StringBuf_Printf(&buf, ") VALUES ('%d',?,'%d',?,'%d','%d','%d','%lu','%d',?,'%d','%d','%d'",
			ad->seller_id, ad->buyer_id, ad->price, ad->buynow, ad->hours, (unsigned long)ad->timestamp, ad->item.nameid, ad->type, ad->item.refine, ad->item.attribute);
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ",'%d'", ad->item.card[j]);
		StringBuf_AppendStr(&buf, ")");
	}
	else
	{
		StringBuf_Printf(&buf, "UPDATE `%s` SET `seller_id` = '%d', `seller_name` = ?, `buyer_id` = '%d', `buyer_name` = ?, `price` = '%d', `buynow` = '%d', `hours` = '%d', `timestamp` = '%lu', `nameid` = '%d', `item_name` = ?, `type` = '%d', `refine` = '%d', `attribute` = '%d'",
			db->auction_db, ad->seller_id, ad->buyer_id, ad->price, ad->buynow, ad->hours, (unsigned long)ad->timestamp, ad->item.nameid, ad->type, ad->item.refine, ad->item.attribute);
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ", `card%d` = '%d'", j, ad->item.card[j]);
		StringBuf_Printf(&buf, " WHERE `auction_id` = '%d'", ad->auction_id);
	}

	do
	{

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (void*)ad->seller_name, strnlen(ad->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)ad->buyer_name, strnlen(ad->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void*)ad->item_name, strnlen(ad->item_name, ITEM_NAME_LENGTH))
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
/*  FIXME: adapt to single-row retrieval

	int i;
	struct auction_data *auction;
	struct item *item;
	char *data;
	StringBuf buf;
	unsigned int tick = gettick(), endtick;
	time_t now = time(NULL);

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`");
	for( i = 0; i < MAX_SLOTS; i++ )
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` ORDER BY `auction_id` DESC", auction_db);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);

	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		CREATE(auction, struct auction_data, 1);
		Sql_GetData(sql_handle, 0, &data, NULL); auction->auction_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); auction->seller_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(auction->seller_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 3, &data, NULL); auction->buyer_id = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); safestrncpy(auction->buyer_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 5, &data, NULL); auction->price	= atoi(data);
		Sql_GetData(sql_handle, 6, &data, NULL); auction->buynow = atoi(data);
		Sql_GetData(sql_handle, 7, &data, NULL); auction->hours = atoi(data);
		Sql_GetData(sql_handle, 8, &data, NULL); auction->timestamp = atoi(data);

		item = &auction->item;
		Sql_GetData(sql_handle, 9, &data, NULL); item->nameid = atoi(data);
		Sql_GetData(sql_handle,10, &data, NULL); safestrncpy(auction->item_name, data, ITEM_NAME_LENGTH);
		Sql_GetData(sql_handle,11, &data, NULL); auction->type = atoi(data);

		Sql_GetData(sql_handle,12, &data, NULL); item->refine = atoi(data);
		Sql_GetData(sql_handle,13, &data, NULL); item->attribute = atoi(data);

		item->identify = 1;
		item->amount = 1;

		for( i = 0; i < MAX_SLOTS; i++ )
		{
			Sql_GetData(sql_handle, 14 + i, &data, NULL);
			item->card[i] = atoi(data);
		}
	}

	Sql_FreeResult(sql_handle);
*/
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
}

static bool auction_db_sql_remove(AuctionDB* self, const int auction_id)
{
/*
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `auction_id` = '%d'", auction_db, auction_id) )
		Sql_ShowDebug(sql_handle);
*/
}

static bool auction_db_sql_save(AuctionDB* self, const struct auction_data* ad)
{
}

static bool auction_db_sql_load(AuctionDB* self, struct auction_data* ad, const int auction_id)
{
}

static bool auction_db_sql_search(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext)
{
}

static int auction_db_sql_count(AuctionDB* self, const int char_id)
{
}

static bool auction_db_sql_first(AuctionDB* self, struct auction_data* ad)
{
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
