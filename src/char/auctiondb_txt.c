// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "auctiondb.h"
#include <stdio.h>
#include <string.h>

#define START_AUCTION_NUM 1


/// internal structure
typedef struct AuctionDB_TXT
{
	AuctionDB vtable;        // public interface

	CharServerDB_TXT* owner;
	DBMap* auctions;         // in-memory auction storage
	int next_auction_id;     // auto_increment

	const char* auction_db;  // auction data storage file

} AuctionDB_TXT;

/// internal functions
static bool auction_db_txt_init(AuctionDB* self);
static void auction_db_txt_destroy(AuctionDB* self);
static bool auction_db_txt_sync(AuctionDB* self);

static bool mmo_auction_fromstr(struct auction_data* ad, char* str);
static bool mmo_auction_tostr(const struct auction_data* ad, char* str);
static bool mmo_auctiondb_sync(AuctionDB_TXT* db);

/// public constructor
AuctionDB* auction_db_txt(CharServerDB_TXT* owner)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)aCalloc(1, sizeof(AuctionDB_TXT));

	// set up the vtable
	db->vtable.init      = &auction_db_txt_init;
	db->vtable.destroy   = &auction_db_txt_destroy;
	db->vtable.sync      = &auction_db_txt_sync;

	// initialize to default values
	db->owner = owner;
	db->auctions = NULL;
	db->next_auction_id = START_AUCTION_NUM;

	// other settings
	db->auction_db = db->owner->file_auctions;

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


static bool auction_db_txt_init(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions;

	char line[8192];
	FILE *fp;

	// create auction database
	db->auctions = idb_alloc(DB_OPT_RELEASE_DATA);
	auctions = db->auctions;

	// open data file
	fp = fopen(db->auction_db, "r");
	if( fp == NULL )
	{
		ShowError("Auction file not found: %s.\n", db->auction_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{
/*
		int auction_id, n;
		struct auction_data p;
		struct auction_data* tmp;

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &auction_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( auction_id > db->next_auction_id )
				db->next_auction_id = auction_id;
			continue;
		}

		if( !mmo_auction_fromstr(&p, line) )
		{
			ShowError("auction_db_txt_init: skipping invalid data: %s", line);
			continue;
		}
	
		// record entry in db
		tmp = (struct auction_data*)aMalloc(sizeof(struct auction_data));
		memcpy(tmp, &p, sizeof(struct auction_data));
		idb_put(auctions, p.auction_id, tmp);

		if( p.auction_id >= db->next_auction_id )
			db->next_auction_id = p.auction_id + 1;
*/
	}

	// close data file
	fclose(fp);

	return true;
}

static void auction_db_txt_destroy(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;

	// write data
	mmo_auctiondb_sync(db);

	// delete auction database
	auctions->destroy(auctions, NULL);
	db->auctions = NULL;

	// delete entire structure
	aFree(db);
}

static bool auction_db_txt_sync(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	return mmo_auctiondb_sync(db);
}

static bool mmo_auction_fromstr(struct auction_data* ad, char* str)
{
}

static bool mmo_auction_tostr(const struct auction_data* ad, char* str)
{
}

static bool mmo_auctiondb_sync(AuctionDB_TXT* db)
{
/*
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	fp = lock_fopen(db->auction_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_auction_sync: can't write [%s] !!! data is lost !!!\n", db->auction_db);
		return false;
	}

	iter = db->auctions->iterator(db->auctions);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct auction_data* ad = (struct auction_data*) data;
		char line[8192];

		mmo_auction_tostr(ad, line);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->auction_db, &lock);
*/
	return true;
}
