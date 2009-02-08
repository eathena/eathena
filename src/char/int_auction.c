// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "int_auction.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Databases
static AuctionDB* auctions = NULL;
static MailDB* mails = NULL;


/// 6 - You have won the auction
/// 7 - You have failed to win the auction
static void mapif_Auction_message(int char_id, unsigned char result)
{
	unsigned char buf[74];
	
	WBUFW(buf,0) = 0x3854;
	WBUFL(buf,2) = char_id;
	WBUFL(buf,6) = result;
	mapif_sendall(buf,7);
}

static void mapif_Auction_sendlist(int fd, int char_id, short count, short pages, unsigned char *buf)
{
	int len = (sizeof(struct auction_data) * count) + 12;

	WFIFOHEAD(fd, len);
	WFIFOW(fd,0) = 0x3850;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = char_id;
	WFIFOW(fd,8) = count;
	WFIFOW(fd,10) = pages;
	memcpy(WFIFOP(fd,12), buf, len - 12);
	WFIFOSET(fd,len);
}

static void mapif_parse_Auction_requestlist(int fd)
{
	int char_id = RFIFOL(fd,4);
	short type = RFIFOW(fd,8);
	int price = RFIFOL(fd,10);
	short page = max(1,RFIFOW(fd,14));
	char searchtext[NAME_LENGTH];

	memcpy(searchtext, RFIFOP(fd,16), NAME_LENGTH);

	//TODO
}

static void mapif_Auction_register(int fd, struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x3851;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(fd,len);
}

static void mapif_parse_Auction_register(int fd)
{
	struct auction_data auction;
	if( RFIFOW(fd,2) != sizeof(struct auction_data) + 4 )
		return;

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));

	//TODO
}

/// 0 - The auction has been canceled
/// 1 - Bid Number is Incorrect
/// 2 - You cannot end the auction
/// 3 - An auction with at least one bidder cannot be canceled
static void mapif_Auction_cancel(int fd, int char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3852;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

static void mapif_parse_Auction_cancel(int fd)
{
	int char_id = RFIFOL(fd,2);
	int auction_id = RFIFOL(fd,6);

	//TODO
}

/// 0 - You have ended the auction
/// 1 - You cannot end the auction
/// 2 - Bid Number is Incorrect
static void mapif_Auction_close(int fd, int char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3853;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

static void mapif_parse_Auction_close(int fd)
{
	int char_id = RFIFOL(fd,2);
	int auction_id = RFIFOL(fd,6);

	//TODO
}

/// 0 - You have failed to bid in the auction
/// 1 - You have successfully bid in the auction
/// 7 - You have failed to win the auction
/// 9 - You cannot place more than 5 bids at a time
static void mapif_Auction_bid(int fd, int char_id, int bid, unsigned char result)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3855;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = bid; // To Return Zeny
	WFIFOB(fd,10) = result;
	WFIFOSET(fd,11);
}

static void mapif_parse_Auction_bid(int fd)
{
	int char_id = RFIFOL(fd,4);
	unsigned int auction_id = RFIFOL(fd,8);
	int bid = RFIFOL(fd,12);

	//TODO
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_auction_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3050: mapif_parse_Auction_requestlist(fd); break;
		case 0x3051: mapif_parse_Auction_register(fd); break;
		case 0x3052: mapif_parse_Auction_cancel(fd); break;
		case 0x3053: mapif_parse_Auction_close(fd); break;
		case 0x3055: mapif_parse_Auction_bid(fd); break;
		default:
			return 0;
	}
	return 1;
}

void inter_auction_init(AuctionDB* adb, MailDB* mdb)
{
	auctions = adb;
	mails = mdb;
}

void inter_auction_final(void)
{
	auctions = NULL;
	mails = NULL;
}
