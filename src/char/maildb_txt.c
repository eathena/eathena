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
#include "maildb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define MAILDB_TXT_DB_VERSION 20090707
#define START_MAIL_NUM 1


/// Internal structure.
/// @private
typedef struct MailDB_TXT
{
	// public interface
	MailDB vtable;

	// data provider
	CSDB_TXT* db;

} MailDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool mail_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct mail_message* msg = (struct mail_message*)data;

	*out_size = sizeof(*msg);

	if( size < sizeof(*msg) )
		return true;

	if( version == 20090707 )
	{
		Txt* txt;
		int i;

		// zero out the destination first
		memset(msg, 0x00, sizeof(*msg));

		txt = Txt_Malloc();
		Txt_Init(txt, (char*)str, strlen(str), 16 + MAX_SLOTS, '\t', '\0', "\t");
		Txt_Bind(txt,  0, TXTDT_INT, &msg->id, sizeof(msg->id));
		Txt_Bind(txt,  1, TXTDT_INT, &msg->send_id, sizeof(msg->send_id));
		Txt_Bind(txt,  2, TXTDT_STRING, &msg->send_name, sizeof(msg->send_name));
		Txt_Bind(txt,  3, TXTDT_INT, &msg->dest_id, sizeof(msg->dest_id));
		Txt_Bind(txt,  4, TXTDT_STRING, &msg->dest_name, sizeof(msg->dest_name));
		Txt_Bind(txt,  5, TXTDT_CSTRING, &msg->title, sizeof(msg->title));
		Txt_Bind(txt,  6, TXTDT_CSTRING, &msg->body, sizeof(msg->body));
		Txt_Bind(txt,  7, TXTDT_ENUM, &msg->status, sizeof(msg->status));
		Txt_Bind(txt,  8, TXTDT_TIME, &msg->timestamp, sizeof(msg->timestamp));
		Txt_Bind(txt,  9, TXTDT_INT, &msg->zeny, sizeof(msg->zeny));
		Txt_Bind(txt, 10, TXTDT_SHORT, &msg->item.nameid, sizeof(msg->item.nameid));
		Txt_Bind(txt, 11, TXTDT_SHORT, &msg->item.amount, sizeof(msg->item.amount));
		Txt_Bind(txt, 12, TXTDT_USHORT, &msg->item.equip, sizeof(msg->item.equip));
		Txt_Bind(txt, 13, TXTDT_CHAR, &msg->item.identify, sizeof(msg->item.identify));
		Txt_Bind(txt, 14, TXTDT_CHAR, &msg->item.refine, sizeof(msg->item.refine));
		Txt_Bind(txt, 15, TXTDT_CHAR, &msg->item.attribute, sizeof(msg->item.attribute));
		for( i = 0; i < MAX_SLOTS; ++i )
			Txt_Bind(txt, 16+i, TXTDT_SHORT, &msg->item.card[i], sizeof(msg->item.card[i]));

		if( Txt_Write(txt) != TXT_SUCCESS || Txt_NumFields(txt) < 16 )
		{
			Txt_Free(txt);
			return false;
		}
		Txt_Free(txt);

		*key = msg->id;
	}
	else
	{// unmatched row
		return false;
	}

	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool mail_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	struct mail_message* msg = (struct mail_message*)data;
	bool result;
	int i;

	Txt* txt = Txt_Malloc();
	Txt_Init(txt, str, SIZE_MAX, 16+MAX_SLOTS, '\t', '\0', "\t");
	Txt_Bind(txt,  0, TXTDT_INT, &msg->id, sizeof(msg->id));
	Txt_Bind(txt,  1, TXTDT_INT, &msg->send_id, sizeof(msg->send_id));
	Txt_Bind(txt,  2, TXTDT_STRING, &msg->send_name, sizeof(msg->send_name));
	Txt_Bind(txt,  3, TXTDT_INT, &msg->dest_id, sizeof(msg->dest_id));
	Txt_Bind(txt,  4, TXTDT_STRING, &msg->dest_name, sizeof(msg->dest_name));
	Txt_Bind(txt,  5, TXTDT_CSTRING, &msg->title, sizeof(msg->title));
	Txt_Bind(txt,  6, TXTDT_CSTRING, &msg->body, sizeof(msg->body));
	Txt_Bind(txt,  7, TXTDT_ENUM, &msg->status, sizeof(msg->status));
	Txt_Bind(txt,  8, TXTDT_TIME, &msg->timestamp, sizeof(msg->timestamp));
	Txt_Bind(txt,  9, TXTDT_INT, &msg->zeny, sizeof(msg->zeny));
	Txt_Bind(txt, 10, TXTDT_SHORT, &msg->item.nameid, sizeof(msg->item.nameid));
	Txt_Bind(txt, 11, TXTDT_SHORT, &msg->item.amount, sizeof(msg->item.amount));
	Txt_Bind(txt, 12, TXTDT_USHORT, &msg->item.equip, sizeof(msg->item.equip));
	Txt_Bind(txt, 13, TXTDT_CHAR, &msg->item.identify, sizeof(msg->item.identify));
	Txt_Bind(txt, 14, TXTDT_CHAR, &msg->item.refine, sizeof(msg->item.refine));
	Txt_Bind(txt, 15, TXTDT_CHAR, &msg->item.attribute, sizeof(msg->item.attribute));
	for( i = 0; i < MAX_SLOTS; ++i )
		Txt_Bind(txt, 16+i, TXTDT_SHORT, &msg->item.card[i], sizeof(msg->item.card[i]));

	result = ( Txt_Write(txt) == TXT_SUCCESS && Txt_NumFields(txt) == 16+MAX_SLOTS );
	Txt_Free(txt);

	return result;
}


/// @protected
static bool mail_db_txt_init(MailDB* self)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->init(db);
}


/// @protected
static void mail_db_txt_destroy(MailDB* self)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	db->destroy(db);
	aFree(self);
}


/// @protected
static bool mail_db_txt_sync(MailDB* self, bool force)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool mail_db_txt_create(MailDB* self, struct mail_message* msg)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;

	if( msg->id == -1 )
		msg->id = db->next_key(db);

	return db->insert(db, msg->id, msg, sizeof(*msg));
}


/// @protected
static bool mail_db_txt_remove(MailDB* self, const int mail_id)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->remove(db, mail_id);
}


/// @protected
static bool mail_db_txt_save(MailDB* self, const struct mail_message* msg)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->replace(db, msg->id, msg, sizeof(*msg));
}


/// @protected
static bool mail_db_txt_load(MailDB* self, struct mail_message* msg, const int mail_id)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->load(db, mail_id, msg, sizeof(*msg), NULL);
}


/// @protected
static bool mail_db_txt_loadall(MailDB* self, struct mail_data* md, const int char_id)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	struct mail_message msg;
	CSDBIterator* iter;
	int mail_id;
	int total = 0;
	int i;

	memset(md, 0, sizeof(*md));

	iter = db->iterator(db);
	while( iter->next(iter, &mail_id) )
	{
		if( !db->load(db, mail_id, &msg, sizeof(msg), NULL) )
			continue;

		if( msg.dest_id != char_id )
			continue;

		if( md->amount < MAIL_MAX_INBOX )
		{
			memcpy(&md->msg[md->amount], &msg, sizeof(msg));
			md->amount++;
		}

		++total;
	}
	iter->destroy(iter);

	// process retrieved data
	md->full = ( total > MAIL_MAX_INBOX );
	md->unchecked = 0;
	md->unread = 0;
	for( i = 0; i < md->amount; ++i )
	{
		struct mail_message* msg = &md->msg[i];

		if( msg->status == MAIL_NEW )
		{// change to 'unread'
			msg->status = MAIL_UNREAD;
			md->unchecked++;
			db->update(db, msg->id, msg, sizeof(*msg)); //FIXME: messy
		}
		else
		if( msg->status == MAIL_UNREAD )
			md->unread++;
	}

	return true;
}


/// Returns an iterator over all mails.
/// @protected
static CSDBIterator* mail_db_txt_iterator(MailDB* self)
{
	CSDB_TXT* db = ((MailDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new MailDB interface.
/// @protected
MailDB* mail_db_txt(CharServerDB_TXT* owner)
{
	MailDB_TXT* db = (MailDB_TXT*)aCalloc(1, sizeof(MailDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_mails, MAILDB_TXT_DB_VERSION, START_MAIL_NUM);
	db->db->p.fromstr = &mail_db_txt_fromstr;
	db->db->p.tostr   = &mail_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &mail_db_txt_init;
	db->vtable.p.destroy = &mail_db_txt_destroy;
	db->vtable.p.sync    = &mail_db_txt_sync;
	db->vtable.create    = &mail_db_txt_create;
	db->vtable.remove    = &mail_db_txt_remove;
	db->vtable.save      = &mail_db_txt_save;
	db->vtable.load      = &mail_db_txt_load;
	db->vtable.loadall   = &mail_db_txt_loadall;
	db->vtable.iterator  = &mail_db_txt_iterator;

	return &db->vtable;
}
