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

static bool auction_db_txt_create(AuctionDB* self, struct auction_data* ad)
{
}

static bool auction_db_txt_remove(AuctionDB* self, const int auction_id)
{
}

static bool auction_db_txt_save(AuctionDB* self, const struct auction_data* ad)
{
}

static bool auction_db_txt_load(AuctionDB* self, struct auction_data* ad, const int auction_id)
{
}

static bool auction_db_txt_search(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext)
{
/*
	unsigned char buf[5 * sizeof(struct auction_data)];
	DBIterator* iter;
	DBKey key;
	void* data;
	short i = 0, j = 0, pages = 1;

	iter = auction_db_->iterator(auction_db_);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		struct auction_data* auction = (struct auction_data*)data;

		if( (type == 0 && auction->type != IT_ARMOR && auction->type != IT_PETARMOR) || 
			(type == 1 && auction->type != IT_WEAPON) ||
			(type == 2 && auction->type != IT_CARD) ||
			(type == 3 && auction->type != IT_ETC) ||
			(type == 4 && !strstr(auction->item_name, searchtext)) ||
			(type == 5 && auction->price > price) ||
			(type == 6 && auction->seller_id != char_id) ||
			(type == 7 && auction->buyer_id != char_id) )
			continue;

		i++;
		if( i > 5 )
		{ // Counting Pages of Total Results (5 Results per Page)
			pages++;
			i = 1; // First Result of This Page
		}

		if( page != pages )
			continue; // This is not the requested Page

		memcpy(WBUFP(buf, j * sizeof(struct auction_data)), auction, len);
		j++; // Found Results
	}
	iter->destroy(iter);
*/
}

static int auction_db_txt_count(AuctionDB* self, const int char_id)
{
/*
	int i = 0;
	struct auction_data *auction;
	DBIterator* iter;
	DBKey key;

	iter = auction_db_->iterator(auction_db_);
	for( auction = (struct auction_data*)iter->first(iter,&key); iter->exists(iter); auction = (struct auction_data*)iter->next(iter,&key) )
	{
		if( (buy && auction->buyer_id == char_id) || (!buy && auction->seller_id == char_id) )
			i++;
	}
	iter->destroy(iter);

	return i;
*/
}

static bool auction_db_txt_first(AuctionDB* self, struct auction_data* ad)
{
}


/// public constructor
AuctionDB* auction_db_txt(CharServerDB_TXT* owner)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)aCalloc(1, sizeof(AuctionDB_TXT));

	// set up the vtable
	db->vtable.init      = &auction_db_txt_init;
	db->vtable.destroy   = &auction_db_txt_destroy;
	db->vtable.sync      = &auction_db_txt_sync;
	db->vtable.create    = &auction_db_txt_create;
	db->vtable.remove    = &auction_db_txt_remove;
	db->vtable.save      = &auction_db_txt_save;
	db->vtable.load      = &auction_db_txt_load;
	db->vtable.search    = &auction_db_txt_search;
	db->vtable.count     = &auction_db_txt_count;
	db->vtable.first     = &auction_db_txt_first;

	// initialize to default values
	db->owner = owner;
	db->auctions = NULL;
	db->next_auction_id = START_AUCTION_NUM;

	// other settings
	db->auction_db = db->owner->file_auctions;

	return &db->vtable;
}
