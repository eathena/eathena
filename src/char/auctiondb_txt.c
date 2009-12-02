// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/txt.h"
#include "../common/utils.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "auctiondb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define AUCTIONDB_TXT_DB_VERSION 20090319
#define START_AUCTION_NUM 1


/// Internal structure.
/// @private
typedef struct AuctionDB_TXT
{
	// public interface
	AuctionDB vtable;

	// data provider
	CSDB_TXT* db;

} AuctionDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool auction_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct auction_data* ad = (struct auction_data*)data;

	*out_size = sizeof(*ad);

	if( size < sizeof(*ad) )
		return true;

	if( version == 20090319 )
	{
		Txt* txt;
		int i;

		// zero out the destination first
		memset(ad, 0x00, sizeof(*ad));

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 14+MAX_SLOTS, '\t', '\0', "");
		Txt_Bind(txt,  0, TXTDT_INT, &ad->auction_id, sizeof(ad->auction_id));
		Txt_Bind(txt,  1, TXTDT_INT, &ad->seller_id, sizeof(ad->seller_id));
		Txt_Bind(txt,  2, TXTDT_STRING, &ad->seller_name, sizeof(ad->seller_name));
		Txt_Bind(txt,  3, TXTDT_INT, &ad->buyer_id, sizeof(ad->buyer_id));
		Txt_Bind(txt,  4, TXTDT_STRING, &ad->buyer_name, sizeof(ad->buyer_name));
		Txt_Bind(txt,  5, TXTDT_INT, &ad->price, sizeof(ad->price));
		Txt_Bind(txt,  6, TXTDT_INT, &ad->buynow, sizeof(ad->buynow));
		Txt_Bind(txt,  7, TXTDT_USHORT, &ad->hours, sizeof(ad->hours));
		Txt_Bind(txt,  8, TXTDT_ULONG, &ad->timestamp, sizeof(ad->timestamp));
		Txt_Bind(txt,  9, TXTDT_SHORT, &ad->item.nameid, sizeof(ad->item.nameid));
		Txt_Bind(txt, 10, TXTDT_STRING, &ad->item_name, sizeof(ad->item_name));
		Txt_Bind(txt, 11, TXTDT_SHORT, &ad->type, sizeof(ad->type));
		Txt_Bind(txt, 12, TXTDT_CHAR, &ad->item.refine, sizeof(ad->item.refine));
		Txt_Bind(txt, 13, TXTDT_CHAR, &ad->item.attribute, sizeof(ad->item.attribute));
		for( i = 0; i < MAX_SLOTS; ++i )
			Txt_Bind(txt, 14+i, TXTDT_SHORT, &ad->item.card[i], sizeof(ad->item.card[i]));

		if( Txt_Parse(txt) != TXT_SUCCESS || Txt_NumFields(txt) < 14 )
		{
			Txt_Free(txt);
			return false;
		}
		Txt_Free(txt);

		*key = ad->auction_id;
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool auction_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	struct auction_data* ad = (struct auction_data*)data;
	bool result;
	int i;

	Txt* txt = Txt_Malloc();
	Txt_Init(txt, str, SIZE_MAX, 14+MAX_SLOTS, '\t', '\0', "\t");
	Txt_Bind(txt,  0, TXTDT_INT, &ad->auction_id, sizeof(ad->auction_id));
	Txt_Bind(txt,  1, TXTDT_INT, &ad->seller_id, sizeof(ad->seller_id));
	Txt_Bind(txt,  2, TXTDT_STRING, &ad->seller_name, sizeof(ad->seller_name));
	Txt_Bind(txt,  3, TXTDT_INT, &ad->buyer_id, sizeof(ad->buyer_id));
	Txt_Bind(txt,  4, TXTDT_STRING, &ad->buyer_name, sizeof(ad->buyer_name));
	Txt_Bind(txt,  5, TXTDT_INT, &ad->price, sizeof(ad->price));
	Txt_Bind(txt,  6, TXTDT_INT, &ad->buynow, sizeof(ad->buynow));
	Txt_Bind(txt,  7, TXTDT_USHORT, &ad->hours, sizeof(ad->hours));
	Txt_Bind(txt,  8, TXTDT_ULONG, &ad->timestamp, sizeof(ad->timestamp));
	Txt_Bind(txt,  9, TXTDT_SHORT, &ad->item.nameid, sizeof(ad->item.nameid));
	Txt_Bind(txt, 10, TXTDT_STRING, &ad->item_name, sizeof(ad->item_name));
	Txt_Bind(txt, 11, TXTDT_SHORT, &ad->type, sizeof(ad->type));
	Txt_Bind(txt, 12, TXTDT_CHAR, &ad->item.refine, sizeof(ad->item.refine));
	Txt_Bind(txt, 13, TXTDT_CHAR, &ad->item.attribute, sizeof(ad->item.attribute));
	for( i = 0; i < MAX_SLOTS; ++i )
		Txt_Bind(txt, 14+i, TXTDT_SHORT, &ad->item.card[i], sizeof(ad->item.card[i]));

	result = ( Txt_Write(txt) == TXT_SUCCESS && Txt_NumFields(txt) == 14+MAX_SLOTS );
	Txt_Free(txt);

	return result;
}


/// @protected
static bool auction_db_txt_init(AuctionDB* self)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void auction_db_txt_destroy(AuctionDB* self)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool auction_db_txt_sync(AuctionDB* self, bool force)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool auction_db_txt_create(AuctionDB* self, struct auction_data* ad)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;

	if( ad->auction_id == -1 )
		ad->auction_id = db->next_key(db);

	return db->insert(db, ad->auction_id, ad, sizeof(*ad));
}


/// @protected
static bool auction_db_txt_remove(AuctionDB* self, const int auction_id)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->remove(db, auction_id);
}


/// @protected
static bool auction_db_txt_save(AuctionDB* self, const struct auction_data* ad)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->update(db, ad->auction_id, ad, sizeof(*ad));
}


/// @protected
static bool auction_db_txt_load(AuctionDB* self, struct auction_data* ad, const int auction_id)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->load(db, auction_id, ad, sizeof(*ad), NULL);
}


/// List the auctions for the specified search results page.
/// @protected
static bool auction_db_txt_search(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	CSDBIterator* iter = db->iterator(db);
	int auction_id;
	struct auction_data auction;
	int i = 0, j = 0, p = 1;
	
	while( iter->next(iter, &auction_id) )
	{
		if( !db->load(db, auction_id, &auction, sizeof(auction), NULL) )
			continue;

		if( (type == 0 && auction.type != IT_ARMOR && auction.type != IT_PETARMOR) || 
			(type == 1 && auction.type != IT_WEAPON) ||
			(type == 2 && auction.type != IT_CARD) ||
			(type == 3 && auction.type != IT_ETC) ||
			(type == 4 && !stristr(auction.item_name, searchtext)) ||
			(type == 5 && auction.price > price) ||
			(type == 6 && auction.seller_id != char_id) ||
			(type == 7 && auction.buyer_id != char_id) )
			continue;

		i++;
		if( i > 5 )
		{// counting pages of total results (5 results per page)
			i = 1;
			p++;
		}

		if( p != page )
			continue; // this is not the requested page

		memcpy(&ad[j], &auction, sizeof(auction));

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
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	CSDBIterator* iter = db->iterator(db);
	int auction_id;
	struct auction_data auction;
	int result = 0;

	while( iter->next(iter, &auction_id) )
	{
		if( !db->load(db, auction_id, &auction, sizeof(auction), NULL) )
			continue;

		if( auction.seller_id == char_id )
			++result;
	}

	iter->destroy(iter);

	return result;
}


/// @protected
static bool auction_db_txt_first(AuctionDB* self, struct auction_data* ad)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	CSDBIterator* iter = db->iterator(db);
	int auction_id;
	struct auction_data auction;
	struct auction_data result;
	bool found = false;

	// find the auction with the lowest timestamp
	while( iter->next(iter, &auction_id) )
	{
		if( !db->load(db, auction_id, &auction, sizeof(auction), NULL) )
			continue;

		if( !found || auction.timestamp < result.timestamp )
		{
			memcpy(&result, &auction, sizeof(result));
			found = true;
		}
	}

	iter->destroy(iter);

	if( !found )
		return false;

	memcpy(ad, &result, sizeof(*ad));

	return true;
}


/// Returns an iterator over all auctions.
/// @protected
static CSDBIterator* auction_db_txt_iterator(AuctionDB* self)
{
	CSDB_TXT* db = ((AuctionDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new AuctionDB interface.
/// @protected
AuctionDB* auction_db_txt(CharServerDB_TXT* owner)
{
	AuctionDB_TXT* db = (AuctionDB_TXT*)aCalloc(1, sizeof(AuctionDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_auctions, AUCTIONDB_TXT_DB_VERSION, START_AUCTION_NUM);
	db->db->p.fromstr = &auction_db_txt_fromstr;
	db->db->p.tostr   = &auction_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &auction_db_txt_init;
	db->vtable.p.destroy = &auction_db_txt_destroy;
	db->vtable.p.sync    = &auction_db_txt_sync;
	db->vtable.create    = &auction_db_txt_create;
	db->vtable.remove    = &auction_db_txt_remove;
	db->vtable.save      = &auction_db_txt_save;
	db->vtable.load      = &auction_db_txt_load;
	db->vtable.search    = &auction_db_txt_search;
	db->vtable.count     = &auction_db_txt_count;
	db->vtable.first     = &auction_db_txt_first;
	db->vtable.iterator  = &auction_db_txt_iterator;

	return &db->vtable;
}
