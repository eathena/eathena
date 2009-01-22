// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _AUCTIONDB_H_
#define _AUCTIONDB_H_

#include "../common/mmo.h" // struct auction_data

typedef struct AuctionDB AuctionDB;


struct AuctionDB
{
	bool (*init)(AuctionDB* self);
	void (*destroy)(AuctionDB* self);

	bool (*sync)(AuctionDB* self);
};


#endif /* _AUCTIONDB_H_ */
