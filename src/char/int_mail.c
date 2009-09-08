// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "char.h"
#include "inter.h"
#include "int_mail.h"
#include "maildb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Databases
static MailDB* mails = NULL;
static CharDB* chars = NULL;


/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
static void mapif_Mail_sendinbox(int fd, int char_id, unsigned char flag)
{
	struct mail_data md;

	if( !mails->loadall(mails, &md, char_id) )
		return;
	
	WFIFOHEAD(fd, sizeof(md) + 9);
	WFIFOW(fd,0) = 0x3848;
	WFIFOW(fd,2) = sizeof(md) + 9;
	WFIFOL(fd,4) = char_id;
	WFIFOB(fd,8) = flag;
	memcpy(WFIFOP(fd,9), &md, sizeof(md)); //FIXME: unsafe
	WFIFOSET(fd,WFIFOW(fd,2));
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
static void mapif_Mail_getattach(int fd, int char_id, int mail_id)
{
	struct mail_message msg;
	struct item it;
	int zeny;

	if( !mails->load(mails, &msg, mail_id) )
		return;

	if( msg.dest_id != char_id )
		return;

	if( msg.status != MAIL_READ )
		return;

	if( (msg.item.nameid < 1 || msg.item.amount < 1) && msg.zeny < 1 )
		return; // No Attachment

	// remove attachment
	memcpy(&it, &msg.item, sizeof(it));
	memset(&msg.item, 0, sizeof(msg.item));
	zeny = msg.zeny;
	msg.zeny = 0;

	// save first(!)
	if( !mails->save(mails, &msg) )
		return;

	// send attachment
	WFIFOHEAD(fd, sizeof(struct item) + 12);
	WFIFOW(fd,0) = 0x384a;
	WFIFOW(fd,2) = sizeof(struct item) + 12;
	WFIFOL(fd,4) = char_id;
	WFIFOL(fd,8) = zeny;
	memcpy(WFIFOP(fd,12), &it, sizeof(it));
	WFIFOSET(fd,WFIFOW(fd,2));
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
static void mapif_Mail_delete(int fd, int char_id, int mail_id)
{
	bool failed = ( mails->remove(mails, mail_id) == false );

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = failed;
	WFIFOSET(fd,11);
}

/*==========================================
 * Report New Mail to Map Server
 *------------------------------------------*/
void mapif_Mail_new(struct mail_message* msg)
{
	unsigned char buf[74];	
	WBUFW(buf,0) = 0x3849;
	WBUFL(buf,2) = msg->dest_id;
	WBUFL(buf,6) = msg->id;
	memcpy(WBUFP(buf,10), msg->send_name, NAME_LENGTH);
	memcpy(WBUFP(buf,34), msg->title, MAIL_TITLE_LENGTH);
	mapif_sendall(buf, 74);
}

/*==========================================
 * Return Mail
 *------------------------------------------*/
static void mapif_Mail_return(int fd, int char_id, int mail_id)
{
	struct mail_message msg;
	char temp_[MAIL_TITLE_LENGTH];
	bool result = false;

	// try
	do
	{

	if( !mails->load(mails, &msg, mail_id) )
		break;

	if( msg.dest_id != char_id)
		break;

	if( mails->remove(mails, mail_id) )
		break;

	// swap sender and receiver
	swap(msg.send_id, msg.dest_id);
	safestrncpy(temp_, msg.send_name, NAME_LENGTH);
	safestrncpy(msg.send_name, msg.dest_name, NAME_LENGTH);
	safestrncpy(msg.dest_name, temp_, NAME_LENGTH);

	// set reply message title
	snprintf(temp_, MAIL_TITLE_LENGTH, "RE:%s", msg.title);
	safestrncpy(msg.title, temp_, MAIL_TITLE_LENGTH);

	msg.status = MAIL_NEW;
	msg.timestamp = time(NULL);

	msg.id = -1; // auto-generate
	if( !mails->create(mails, &msg) )
		break;

	// success
	mapif_Mail_new(&msg);
	result = true;

	}
	while(0);
	// finally

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = ( result == false );
	WFIFOSET(fd,11);
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
static void mapif_Mail_send(int fd, struct mail_message* msg)
{
	int len = 4 + sizeof(struct mail_message);
	
	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), msg, sizeof(struct mail_message));
	WFIFOSET(fd,len);
}

static void mapif_parse_Mail_send(int fd, int size, int account_id, const void* data)
{
	struct mail_message msg;
	int charid, accid;
	unsigned int n;

	if( size != 8 + sizeof(struct mail_message) )
		return;

	memcpy(&msg, data, sizeof(struct mail_message));
	msg.status = MAIL_NEW;

	// try
	do
	{

	if( !chars->name2id(chars, msg.dest_name, true, &charid, &accid, &n) &&// not exact
		!(!char_config.character_name_case_sensitive && chars->name2id(chars, msg.dest_name, false, &charid, &accid, &n) && n == 1) )// not unique
		break;

	if( accid == account_id )
		break; // Cannot send mail to char in the same account

	msg.dest_id = charid;

	if( !mails->create(mails, &msg) )
		msg.dest_id = 0; // FIXME: add a proper boolean flag to the reply packet

	}
	while(0);
	// finally

	mapif_Mail_send(fd, &msg);
}

static void mapif_parse_Mail_requestinbox(int fd, int char_id, unsigned char flag)
{
	mapif_Mail_sendinbox(fd, char_id, flag);
}

static void mapif_parse_Mail_read(int fd, int mail_id)
{
	struct mail_message msg;
	if( mails->load(mails, &msg, mail_id) )
	{// Mark mail as 'Read'
		msg.status = MAIL_READ;
		mails->save(mails, &msg);
	}
}

static void mapif_parse_Mail_getattach(int fd, int char_id, int mail_id)
{
	mapif_Mail_getattach(fd, char_id, mail_id);
}

static void mapif_parse_Mail_delete(int fd, int char_id, int mail_id)
{
	mapif_Mail_delete(fd, char_id, mail_id);
}

static void mapif_parse_Mail_return(int fd, int char_id, int mail_id)
{
	mapif_Mail_return(fd, char_id, mail_id);
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_mail_parse_frommap(int fd)
{
	int cmd = RFIFOW(fd,0);

	switch( cmd )
	{
	case 0x3070: mapif_parse_Mail_requestinbox(fd, RFIFOL(fd,2), RFIFOB(fd,6)); break;
	case 0x3071: mapif_parse_Mail_read(fd, RFIFOL(fd,2)); break;
	case 0x3072: mapif_parse_Mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3073: mapif_parse_Mail_delete(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3074: mapif_parse_Mail_return(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3075: mapif_parse_Mail_send(fd, RFIFOW(fd,2), RFIFOL(fd,4), RFIFOP(fd,8)); break;
	default:
		return 0;
	}
	return 1;
}

void inter_mail_init(MailDB* mdb, CharDB* cdb)
{
	mails = mdb;
	chars = cdb;
}

void inter_mail_final(void)
{
	mails = NULL;
	chars = NULL;
}

void inter_mail_send(int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item *item)
{
	struct mail_message msg;
	memset(&msg, 0, sizeof(struct mail_message));

	msg.send_id = send_id;
	safestrncpy(msg.send_name, send_name, NAME_LENGTH);
	msg.dest_id = dest_id;
	safestrncpy(msg.dest_name, dest_name, NAME_LENGTH);
	safestrncpy(msg.title, title, MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, body, MAIL_BODY_LENGTH);
	msg.zeny = zeny;
	if( item != NULL )
		memcpy(&msg.item, item, sizeof(struct item));

	msg.timestamp = time(NULL);

	msg.id = -1; //auto-generate
	mails->create(mails, &msg);

	mapif_Mail_new(&msg);
}
