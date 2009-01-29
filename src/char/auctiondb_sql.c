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


/// internal structure
typedef struct AuctionDB_SQL
{
	AuctionDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* auctions;       // SQL auction storage

	// other settings
	const char* auction_db;

} AuctionDB_SQL;

/// internal functions
static bool auction_db_sql_init(AuctionDB* self);
static void auction_db_sql_destroy(AuctionDB* self);
static bool auction_db_sql_sync(AuctionDB* self);

//static bool mmo_auction_fromsql(AuctionDB_SQL* db, struct auction_data* ad, int auction_id);
//static bool mmo_auction_tosql(AuctionDB_SQL* db, const struct auction_data* ad, bool is_new);

/// public constructor
AuctionDB* auction_db_sql(CharServerDB_SQL* owner)
{
	AuctionDB_SQL* db = (AuctionDB_SQL*)aCalloc(1, sizeof(AuctionDB_SQL));

	// set up the vtable
	db->vtable.init      = &auction_db_sql_init;
	db->vtable.destroy   = &auction_db_sql_destroy;
	db->vtable.sync      = &auction_db_sql_sync;

	// initialize to default values
	db->owner = owner;
	db->auctions = NULL;

	// other settings
	db->auction_db = db->owner->table_auctions;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


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
