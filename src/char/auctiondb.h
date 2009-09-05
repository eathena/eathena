// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _AUCTIONDB_H_
#define _AUCTIONDB_H_


#include "../common/mmo.h" // struct auction_data
#include "csdbiterator.h"


typedef struct AuctionDB AuctionDB;


struct AuctionDB
{
	bool (*init)(AuctionDB* self);
	void (*destroy)(AuctionDB* self);

	bool (*sync)(AuctionDB* self);

	bool (*create)(AuctionDB* self, struct auction_data* ad);
	bool (*remove)(AuctionDB* self, const int auction_id);

	bool (*save)(AuctionDB* self, const struct auction_data* ad);
	bool (*load)(AuctionDB* self, struct auction_data* ad, const int auction_id);

	/// List the auctions for the specified search results page.
	bool (*search)(AuctionDB* self, struct auction_data ad[5], int* pages, int* results, int char_id, int page, int type, int price, const char* searchtext);

	/// Count the number of auctions started by this character.
	int (*count)(AuctionDB* self, const int char_id);

	/// Find the auction with the earliest expiration time.
	bool (*first)(AuctionDB* self, struct auction_data* ad);

	/// Returns an iterator over all auctions.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(AuctionDB* self);
};


#endif /* _AUCTIONDB_H_ */
