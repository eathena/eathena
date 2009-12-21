// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_AUCTION_H_
#define _INT_AUCTION_H_

#include "auctiondb.h"
#include "maildb.h"

void inter_auction_init(AuctionDB* adb, MailDB* mdb);
void inter_auction_final(void);
int inter_auction_parse_frommap(int fd);

#endif // _INT_AUCTION_H_
