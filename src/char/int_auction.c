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
#include "int_mail.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// Databases
static AuctionDB* auctions = NULL;
static MailDB* mails = NULL;

/// Auction end timer.
/// Always targets the auction with the closest expiration time.
/// Code must ensure that this invariant is preserved when adding/deleting auctions.
int auction_end_timer = INVALID_TIMER;

static int auction_end(int tid, unsigned int tick, int id, intptr data);
static void mapif_Auction_message(int char_id, unsigned char result);


/// drops the current target and finds a new one
static void auction_timer_reset(void)
{
	struct auction_data auction;
	unsigned int new_tick;

	// drop current target
	if( auction_end_timer != INVALID_TIMER )
	{
		delete_timer(auction_end_timer, auction_end);
		auction_end_timer = INVALID_TIMER;
	}

	// find the new target
	if( !auctions->first(auctions, &auction) )
		return;

	// start timer
	new_tick = gettick() + (auction.timestamp - time(NULL)) * 1000;
	auction_end_timer = add_timer(new_tick, auction_end, auction.auction_id, 0);
}

static void auction_create(struct auction_data* auction)
{
	const struct TimerData* t = NULL;
	unsigned int new_tick;

	auction->auction_id = -1; // auto-generate
	auction->timestamp = time(NULL) + auction->hours * 3600;
	auction->item.amount = 1;
	auction->item.identify = 1;

	if( !auctions->create(auctions, auction) )
		return; // failed

	// if new end time < old end time, switch the auction_end_timer
	t = get_timer(auction_end_timer);
	new_tick = gettick() + auction->hours * 3600 * 1000;
	if( t == NULL || DIFF_TICK(new_tick, t->tick) < 0 )
		auction_timer_reset();
}

void auction_delete(struct auction_data* auction)
{
	const struct TimerData* t = NULL;
	unsigned int auction_id = auction->auction_id;

	if( !auctions->remove(auctions, auction_id) )
		return; // failed

	// if this auction's id == auction_end_timer's auction id, find a new target
	t = get_timer(auction_end_timer);
	if( t == NULL || t->id == auction_id )
		auction_timer_reset();
}

static int auction_end(int tid, unsigned int tick, int id, intptr data)
{
	struct auction_data auction;
	if( !auctions->load(auctions, &auction, id) )
		return 0;

	if( auction.buyer_id > 0 )
	{
		mapif_Auction_message(auction.buyer_id, 6); // You have won the auction
		inter_mail_send(0, "Auction Manager", auction.buyer_id, auction.buyer_name, "Auction", "Thanks, you won the auction!.", 0, &auction.item);
		inter_mail_send(0, "Auction Manager", auction.seller_id, auction.seller_name, "Auction", "Payment for your auction!.", auction.price, NULL);
	}
	else
		inter_mail_send(0, "Auction Manager", auction.seller_id, auction.seller_name, "Auction", "No buyers have been found for your auction.", 0, &auction.item);

	auction_delete(&auction);

	return 0;
}


/// 6 - You have won the auction
/// 7 - You have failed to win the auction
static void mapif_Auction_message(int char_id, unsigned char result)
{
	unsigned char buf[7];
	
	WBUFW(buf,0) = 0x3854;
	WBUFL(buf,2) = char_id;
	WBUFB(buf,6) = result;
	mapif_sendall(buf,7);
}


static void mapif_Auction_sendlist(int fd, int char_id, short count, short pages, const struct auction_data* data)
{
	int len = (sizeof(struct auction_data) * count) + 12;

	WFIFOHEAD(fd, len);
	WFIFOW(fd,0) = 0x3850;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = char_id;
	WFIFOW(fd,8) = count;
	WFIFOW(fd,10) = pages;
	memcpy(WFIFOP(fd,12), data, count * sizeof(*data));
	WFIFOSET(fd,len);
}

static void mapif_parse_Auction_requestlist(int fd)
{
	int char_id = RFIFOL(fd,4);
	short type = RFIFOW(fd,8);
	int price = RFIFOL(fd,10);
	short page = max(1,RFIFOW(fd,14));
	const char* searchtext = (char*)RFIFOP(fd,16);

	struct auction_data data[5];
	int results;
	int pages;

	if( !auctions->search(auctions, data, &pages, &results, char_id, page, type, price, searchtext) )
		return;

	mapif_Auction_sendlist(fd, char_id, results, pages, data);
}


static void mapif_Auction_register(int fd, struct auction_data* auction)
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
	int size = RFIFOW(fd,2);
	struct auction_data* auction = (struct auction_data*)RFIFOP(fd,4);

	if( size != sizeof(struct auction_data) + 4 )
		return;

	// check if seller_id may create more auctions
	if( auctions->count(auctions, auction->seller_id) < 5 )
		auction_create(auction);

	// send back reply (result is derived from auction_id)
	mapif_Auction_register(fd, auction);
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

	struct auction_data auction;
	if( !auctions->load(auctions, &auction, auction_id) )
	{
		mapif_Auction_cancel(fd, char_id, 1); // Bid Number is Incorrect
		return;
	}

	if( auction.seller_id != char_id )
	{
		mapif_Auction_cancel(fd, char_id, 2); // You cannot end the auction
		return;
	}

	if( auction.buyer_id > 0 )
	{
		mapif_Auction_cancel(fd, char_id, 3); // An auction with at least one bidder cannot be canceled
		return;
	}

	mapif_Auction_cancel(fd, char_id, 0); // The auction has been canceled

	inter_mail_send(0, "Auction Manager", auction.seller_id, auction.seller_name, "Auction", "Auction canceled.", 0, &auction.item);

	auction_delete(&auction);
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

	struct auction_data auction;
	if( !auctions->load(auctions, &auction, auction_id) )
	{
		mapif_Auction_close(fd, char_id, 2); // Bid Number is Incorrect
		return;
	}

	if( auction.seller_id != char_id )
	{
		mapif_Auction_close(fd, char_id, 1); // You cannot end the auction
		return;
	}

	if( auction.buyer_id == 0 )
	{
		mapif_Auction_close(fd, char_id, 1); // You cannot end the auction
		return;
	}

	mapif_Auction_message(auction.buyer_id, 6); // You have won the auction
	mapif_Auction_close(fd, char_id, 0); // You have ended the auction

	// Send Money to Seller
	inter_mail_send(0, "Auction Manager", auction.seller_id, auction.seller_name, "Auction", "Auction closed.", auction.price, NULL);
	// Send Item to Buyer
	inter_mail_send(0, "Auction Manager", auction.buyer_id, auction.buyer_name, "Auction", "Auction winner.", 0, &auction.item);

	auction_delete(&auction);
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
	const char* name = (char*)RFIFOP(fd,16);

	struct auction_data auction;
	if( !auctions->load(auctions, &auction, auction_id) || auction.price >= bid || auction.seller_id == char_id )
	{
		mapif_Auction_bid(fd, char_id, bid, 0); // You have failed to bid in the auction
		return;
	}

/*	would need another count() operation, won't implement
	if( auction_count(char_id, true) >= 5 && bid < auction.buynow && auction.buyer_id != char_id )
	{
		mapif_Auction_bid(fd, char_id, bid, 9); // You cannot place more than 5 bids at a time
		return;
	}
*/

	if( auction.buyer_id > 0 )
	{// Send Money back to the previous Buyer
		if( auction.buyer_id != char_id )
		{
			inter_mail_send(0, "Auction Manager", auction.buyer_id, auction.buyer_name, "Auction", "Someone has placed a higher bid.", auction.price, NULL);
			mapif_Auction_message(auction.buyer_id, 7); // You have failed to win the auction
		}
		else
			inter_mail_send(0, "Auction Manager", auction.buyer_id, auction.buyer_name, "Auction", "You have placed a higher bid.", auction.price, NULL);
	}

	auction.buyer_id = char_id;
	safestrncpy(auction.buyer_name, name, NAME_LENGTH);
	auction.price = bid;

	if( bid >= auction.buynow )
	{// automatically wins the auction
		//TODO: move the excessive money send from this packet to the mail with the item attached
		mapif_Auction_bid(fd, char_id, bid - auction.buynow, 1); // You have successfully bid in the auction
		mapif_Auction_message(char_id, 6); // You have won the auction
		inter_mail_send(0, "Auction Manager", auction.buyer_id, auction.buyer_name, "Auction", "You have won the auction.", 0, &auction.item);
		inter_mail_send(0, "Auction Manager", auction.seller_id, auction.seller_name, "Auction", "Payment for your auction!.", auction.buynow, NULL);

		auction_delete(&auction);
	}
	else
	{// auction continues
		mapif_Auction_bid(fd, char_id, 0, 1); // You have successfully bid in the auction
		auctions->save(auctions, &auction);
	}
}


/// Packets From Map Server.
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

	auction_timer_reset();
}

void inter_auction_final(void)
{
	auctions = NULL;
	mails = NULL;
}
