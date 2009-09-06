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
#include <stdlib.h>
#include <string.h>


/// global defines
#define AUCTION_TXT_DB_VERSION 20090319
#define START_AUCTION_NUM 1


/// Internal structure.
/// @private
typedef struct AuctionDB_TXT
{
	// public interface
	AuctionDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* auctions;
	int next_auction_id;
	bool dirty;

	// settings
	const char* auction_db;

} AuctionDB_TXT;


/// @private
static bool mmo_auction_fromstr(struct auction_data* ad, char* str, unsigned int version)
{
	char* fields[32];
	int count;
	int i;

	// zero out the destination first
	memset(ad, 0x00, sizeof(*ad));

	// extract tab-separated columns from str
	count = sv_split(str, strlen(str), 0, '\t', fields, ARRAYLENGTH(fields), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF)) - 1;

	if( version == 20090319 && count >= 14 )
	{
		ad->auction_id = strtol(fields[1], NULL, 10);
		ad->seller_id = strtol(fields[2], NULL, 10);
		safestrncpy(ad->seller_name, fields[3], sizeof(ad->seller_name));
		ad->buyer_id = strtol(fields[4], NULL, 10);
		safestrncpy(ad->buyer_name, fields[5], sizeof(ad->buyer_name));
		ad->price = strtol(fields[6], NULL, 10);
		ad->buynow = strtol(fields[7], NULL, 10);
		ad->hours = (unsigned short)strtoul(fields[8], NULL, 10);
		ad->timestamp = strtoul(fields[9], NULL, 10);
		ad->item.nameid = (short)strtol(fields[10], NULL, 10);
		safestrncpy(ad->item_name, fields[11], sizeof(ad->item_name));
		ad->type = (short)strtol(fields[12], NULL, 10);
		ad->item.refine = (char)strtol(fields[13], NULL, 10);
		ad->item.attribute = (char)strtol(fields[14], NULL, 10);

		for( i = 0; i < count - 14 && i < MAX_SLOTS; ++i )
			ad->item.card[i] = (short)strtol(fields[15+i], NULL, 10);
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// @private
static bool mmo_auction_tostr(const struct auction_data* ad, char* str)
{
	char* p = str;
	int i;

	p += sprintf(p, "%d\t%d\t%s\t%d\t%s\t%d\t%d\t%d\t%lu\t%d\t%s\t%d\t%d\t%d\t",
		ad->auction_id, ad->seller_id, ad->seller_name, ad->buyer_id, ad->buyer_name,
		ad->price, ad->buynow, ad->hours, (unsigned long)ad->timestamp,
		ad->item.nameid, ad->item_name, ad->type, ad->item.refine, ad->item.attribute);

	for( i = 0; i < MAX_SLOTS; i++ )
		p += sprintf(p, "%d\t", ad->item.card[i]);

	return true;
}


/// @protected
static bool auction_db_txt_init(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create auction database
	if( db->auctions == NULL )
		db->auctions = idb_alloc(DB_OPT_RELEASE_DATA);
	auctions = db->auctions;
	db_clear(auctions);

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
		int auction_id, n;
		unsigned int v;
		struct auction_data p;
		struct auction_data* tmp;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &auction_id, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( auction_id > db->next_auction_id )
				db->next_auction_id = auction_id;
			continue;
		}

		if( !mmo_auction_fromstr(&p, line, version) )
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
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void auction_db_txt_destroy(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;

	// delete auction database
	if( auctions != NULL )
	{
		db_destroy(auctions);
		db->auctions = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool auction_db_txt_sync(AuctionDB* self, bool force)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBIterator* iter;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->auction_db, &lock);
	if( fp == NULL )
	{
		ShowError("auction_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->auction_db);
		return false;
	}

	fprintf(fp, "%d\n", AUCTION_TXT_DB_VERSION); // savefile version

	iter = db->auctions->iterator(db->auctions);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct auction_data* ad = (struct auction_data*) data;
		char line[8192];

		mmo_auction_tostr(ad, line);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_auction_id);
	iter->destroy(iter);

	lock_fclose(fp, db->auction_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool auction_db_txt_create(AuctionDB* self, struct auction_data* ad)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;
	struct auction_data* tmp;

	// decide on the pet id to assign
	int auction_id = ( ad->auction_id != -1 ) ? ad->auction_id : db->next_auction_id;

	// check if the auction id is free
	tmp = idb_get(auctions, auction_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("auction_db_txt_create: cannot create auction %d, this id is already occupied!\n", auction_id);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct auction_data, 1);
	memcpy(tmp, ad, sizeof(struct auction_data));
	tmp->auction_id = auction_id;
	idb_put(auctions, auction_id, tmp);

	// increment the auto_increment value
	if( auction_id >= db->next_auction_id )
		db->next_auction_id = auction_id + 1;

	// write output
	ad->auction_id = auction_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool auction_db_txt_remove(AuctionDB* self, const int auction_id)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;

	idb_remove(auctions, auction_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool auction_db_txt_save(AuctionDB* self, const struct auction_data* ad)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;
	int auction_id = ad->auction_id;

	// retrieve previous data
	struct auction_data* tmp = idb_get(auctions, auction_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, ad, sizeof(*ad));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool auction_db_txt_load(AuctionDB* self, struct auction_data* ad, const int auction_id)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;

	// retrieve data
	struct auction_data* tmp = idb_get(auctions, auction_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(ad, tmp, sizeof(*tmp));

	return true;
}


/// List the auctions for the specified search results page.
/// @protected
static bool auction_db_txt_search(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;
	DBIterator* iter;
	void* data;
	int i = 0, j = 0, p = 1;

	iter = auctions->iterator(auctions);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct auction_data* auction = (struct auction_data*)data;

		if( (type == 0 && auction->type != IT_ARMOR && auction->type != IT_PETARMOR) || 
			(type == 1 && auction->type != IT_WEAPON) ||
			(type == 2 && auction->type != IT_CARD) ||
			(type == 3 && auction->type != IT_ETC) ||
			(type == 4 && !stristr(auction->item_name, searchtext)) ||
			(type == 5 && auction->price > price) ||
			(type == 6 && auction->seller_id != char_id) ||
			(type == 7 && auction->buyer_id != char_id) )
			continue;

		i++;
		if( i > 5 )
		{// counting pages of total results (5 results per page)
			i = 1;
			p++;
		}

		if( p != page )
			continue; // this is not the requested page

		memcpy(&ad[j], auction, sizeof(*auction));

		j++; // found results
	}
	iter->destroy(iter);

	*pages = p;
	*results = j;

	return true;
}


/// @protected
static int auction_db_txt_count(AuctionDB* self, const int char_id)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;
	DBIterator* iter;
	void* data;
	int result = 0;

	iter = auctions->iterator(auctions);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		const struct auction_data* auction = (struct auction_data*)data;
		if( auction->seller_id == char_id )
			++result;
	}
	iter->destroy(iter);

	return result;
}


/// @protected
static bool auction_db_txt_first(AuctionDB* self, struct auction_data* ad)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	DBMap* auctions = db->auctions;
	DBIterator* iter;
	void* data;
	const struct auction_data* result = NULL;

	iter = auctions->iterator(auctions);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		const struct auction_data* auction = (struct auction_data*)data;
		if( result == NULL || auction->timestamp < result->timestamp )
			result = auction;
	}
	iter->destroy(iter);

	if( result == NULL )
		return false;

	memcpy(ad, result, sizeof(*result));
	return true;
}


/// Returns an iterator over all auctions.
/// @protected
static CSDBIterator* auction_db_txt_iterator(AuctionDB* self)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->auctions));
}


/// Constructs a new AuctionDB interface.
/// @protected
AuctionDB* auction_db_txt(CharServerDB_TXT* owner)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)aCalloc(1, sizeof(AuctionDB_TXT));

	// set up the vtable
	db->vtable.p.init      = &auction_db_txt_init;
	db->vtable.p.destroy   = &auction_db_txt_destroy;
	db->vtable.p.sync      = &auction_db_txt_sync;
	db->vtable.create    = &auction_db_txt_create;
	db->vtable.remove    = &auction_db_txt_remove;
	db->vtable.save      = &auction_db_txt_save;
	db->vtable.load      = &auction_db_txt_load;
	db->vtable.search    = &auction_db_txt_search;
	db->vtable.count     = &auction_db_txt_count;
	db->vtable.first     = &auction_db_txt_first;
	db->vtable.iterator  = &auction_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->auctions = NULL;
	db->next_auction_id = START_AUCTION_NUM;
	db->dirty = false;

	// other settings
	db->auction_db = db->owner->file_auctions;

	return &db->vtable;
}
