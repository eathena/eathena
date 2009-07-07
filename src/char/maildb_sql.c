// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/malloc.h"
#include "charserverdb_sql.h"
#include "maildb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct MailDB_SQL
{
	MailDB vtable;

	CharServerDB_SQL* owner;
	Sql* mails;  // sql handler

	// other settings
	const char* mail_db;

} MailDB_SQL;



static bool mmo_mail_tosql(MailDB_SQL* db, struct mail_message* msg, bool is_new)
{
	Sql* sql_handle = db->mails;

	StringBuf buf;
	SqlStmt* stmt;
	int insert_id;
	bool result = false;
	int j;

	StringBuf_Init(&buf);

	// try
	do
	{

	if( is_new )
	{// insert new mail entry
		StringBuf_Printf(&buf, "INSERT INTO `%s` (`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,`zeny`,`nameid`,`amount`,`refine`,`attribute`,`identify`", db->mail_db);
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ",`card%d`", j);
		StringBuf_Printf(&buf, ",`id`) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?");
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ",'%d'", msg->item.card[j]); //TODO: bind as params
		StringBuf_AppendStr(&buf, ",?)");
	}
	else
	{// update existing mail entry
		StringBuf_Printf(&buf, "UPDATE `%s` SET `send_name`=?, `send_id`=?, `dest_name`=?, `dest_id`=?, `title`=?, `message`=?, `time`=?, `status`=?, `zeny`=?, `nameid`=?, `amount`=?, `refine`=?, `attribute`=?, `identify`=?", db->mail_db);
		for( j = 0; j < MAX_SLOTS; j++ )
			StringBuf_Printf(&buf, ", `card%d` = '%d'", j, msg->item.card[j]);
		StringBuf_Printf(&buf, " WHERE `id`=?", msg->id);
	}

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  0, SQLDT_STRING, (void*)&msg->send_name, strnlen(msg->send_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  1, SQLDT_INT,    (void*)&msg->send_id, sizeof(msg->send_id))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  2, SQLDT_STRING, (void*)&msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  3, SQLDT_INT,    (void*)&msg->dest_id, sizeof(msg->dest_id))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  4, SQLDT_STRING, (void*)&msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  5, SQLDT_STRING, (void*)&msg->body, strnlen(msg->body, MAIL_BODY_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  6, SQLDT_UINT,   (void*)&msg->timestamp, sizeof(msg->timestamp))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  7, SQLDT_INT,    (void*)&msg->status, sizeof(msg->status)) //FIXME: type-size mismatch
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  8, SQLDT_INT,    (void*)&msg->zeny, sizeof(msg->zeny))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt,  9, SQLDT_SHORT,  (void*)&msg->item.nameid, sizeof(msg->item.nameid))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_SHORT,  (void*)&msg->item.amount, sizeof(msg->item.amount))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_CHAR,   (void*)&msg->item.refine, sizeof(msg->item.refine))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 12, SQLDT_CHAR,   (void*)&msg->item.attribute, sizeof(msg->item.attribute))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 13, SQLDT_CHAR,   (void*)&msg->item.identify, sizeof(msg->item.identify))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 14, (msg->id != -1)?SQLDT_INT:SQLDT_NULL, (void*)&msg->id, sizeof(msg->id)) //FIXME: column is actually uBIGINT
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		break;
	}

	if( is_new )
	{
		insert_id = (int)SqlStmt_LastInsertId(stmt);
		if( msg->id == -1 )
			msg->id = insert_id; // fill in output value
		else
		if( msg->id != insert_id )
			break; // error, unexpected value
	}

	// if we got this far, everything was successful
	result = true;

	} while(0);
	// finally

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return result;
}


static bool mmo_mail_fromsql(MailDB_SQL* db, struct mail_message* msg, int mail_id)
{
	Sql* sql_handle = db->mails;

	StringBuf buf;
	char* data;
	int j;

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `id` = '%d'", db->mail_db, mail_id);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) || SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	Sql_GetData(sql_handle, 0, &data, NULL); msg->id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
	Sql_GetData(sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
	Sql_GetData(sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
	Sql_GetData(sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
	Sql_GetData(sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
	Sql_GetData(sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
	Sql_GetData(sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
	Sql_GetData(sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
	Sql_GetData(sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
	Sql_GetData(sql_handle,10, &data, NULL); msg->item.amount = (short)atoi(data);
	Sql_GetData(sql_handle,11, &data, NULL); msg->item.nameid = atoi(data);
	Sql_GetData(sql_handle,12, &data, NULL); msg->item.refine = atoi(data);
	Sql_GetData(sql_handle,13, &data, NULL); msg->item.attribute = atoi(data);
	Sql_GetData(sql_handle,14, &data, NULL); msg->item.identify = atoi(data);

	for( j = 0; j < MAX_SLOTS; ++j )
	{
		Sql_GetData(sql_handle, 15 + j, &data, NULL); msg->item.card[j] = atoi(data);
	}

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	return true;
}

static bool mmo_mails_fromsql(MailDB_SQL* db, struct mail_data* md, const int char_id)
{
	Sql* sql_handle = db->mails;

	struct mail_message* msg;
	char *data;
	StringBuf buf;
	int i, j;

	memset(md, 0, sizeof(struct mail_data));
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`");
	for( i = 0; i < MAX_SLOTS; ++i )
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `dest_id` = '%d'  ORDER BY `id`", db->mail_db, char_id);

	if( SQL_SUCCESS != Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	for( i = 0; i < MAIL_MAX_INBOX && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		msg = &md->msg[i];
		Sql_GetData(sql_handle,  0, &data, NULL); msg->id = atoi(data);
		Sql_GetData(sql_handle,  1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle,  2, &data, NULL); msg->send_id = atoi(data);
		Sql_GetData(sql_handle,  3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle,  4, &data, NULL); msg->dest_id = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		Sql_GetData(sql_handle,  6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		Sql_GetData(sql_handle,  7, &data, NULL); msg->timestamp = atoi(data);
		Sql_GetData(sql_handle,  8, &data, NULL); msg->status = (mail_status)atoi(data);
		Sql_GetData(sql_handle,  9, &data, NULL); msg->zeny = atoi(data);
		Sql_GetData(sql_handle, 10, &data, NULL); msg->item.amount = (short)atoi(data);
		Sql_GetData(sql_handle, 11, &data, NULL); msg->item.nameid = atoi(data);
		Sql_GetData(sql_handle, 12, &data, NULL); msg->item.refine = atoi(data);
		Sql_GetData(sql_handle, 13, &data, NULL); msg->item.attribute = atoi(data);
		Sql_GetData(sql_handle, 14, &data, NULL); msg->item.identify = atoi(data);

		for( j = 0; j < MAX_SLOTS; ++j )
		{
			Sql_GetData(sql_handle, 15 + j, &data, NULL); msg->item.card[j] = atoi(data);
		}
	}

	md->full = ( Sql_NumRows(sql_handle) > MAIL_MAX_INBOX );
	md->amount = i;

	StringBuf_Destroy(&buf);
	Sql_FreeResult(sql_handle);

	// process retrieved data
	md->unchecked = 0;
	md->unread = 0;
	for( i = 0; i < md->amount; ++i )
	{
		msg = &md->msg[i];

		if( msg->status == MAIL_NEW )
		{// change to 'unread'
			msg->status = MAIL_UNREAD;
			if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%u'", db->mail_db, msg->status, msg->id) )
				Sql_ShowDebug(sql_handle);

			md->unchecked++;
		}
		else
		if( msg->status == MAIL_UNREAD )
			md->unread++;
	}

	return true;
}


static bool mail_db_sql_init(MailDB* self)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	db->mails = db->owner->sql_handle;
	return true;
}

static void mail_db_sql_destroy(MailDB* self)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	db->mails = NULL;
	aFree(db);
}

static bool mail_db_sql_sync(MailDB* self)
{
	return true;
}

static bool mail_db_sql_create(MailDB* self, struct mail_message* msg)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	Sql* sql_handle = db->mails;

	// decide on the mail id to assign
	int mail_id;
	if( msg->id != -1 )
	{// caller specifies it manually
		mail_id = msg->id;
	}
	else
	{// ask the database
		char* data;
		size_t len;

		if( SQL_SUCCESS != Sql_Query(sql_handle, "SELECT MAX(`id`)+1 FROM `%s`", db->mail_db) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
		{
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			return false;
		}

		Sql_GetData(sql_handle, 0, &data, &len);
		mail_id = ( data != NULL ) ? atoi(data) : 0;
		Sql_FreeResult(sql_handle);
	}

	// zero value is prohibited
	if( mail_id == 0 )
		return false;

	// insert the data into the database
	msg->id = mail_id;
	return mmo_mail_tosql(db, msg, true);
}

static bool mail_db_sql_remove(MailDB* self, const int mail_id)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	Sql* sql_handle = db->mails;

	if( SQL_SUCCESS != Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id`='%d'", db->mail_db, mail_id) )
	{
		Sql_ShowDebug(db->mails);
		return false;
	}

	return true;
}

static bool mail_db_sql_save(MailDB* self, const struct mail_message* msg)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	return mmo_mail_tosql(db, (struct mail_message*)msg, false);
}

static bool mail_db_sql_load(MailDB* self, struct mail_message* msg, const int mail_id)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	return mmo_mail_fromsql(db, msg, mail_id);
}

static bool mail_db_sql_loadall(MailDB* self, struct mail_data* md, const int char_id)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	return mmo_mails_fromsql(db, md, char_id);
}


/// Returns an iterator over all mails.
static CSDBIterator* mail_db_sql_iterator(MailDB* self)
{
	MailDB_SQL* db = (MailDB_SQL*)self;
	return csdb_sql_iterator(db->mails, db->mail_db, "id");
}


/// public constructor
MailDB* mail_db_sql(CharServerDB_SQL* owner)
{
	MailDB_SQL* db = (MailDB_SQL*)aCalloc(1, sizeof(MailDB_SQL));

	// set up the vtable
	db->vtable.init    = &mail_db_sql_init;
	db->vtable.destroy = &mail_db_sql_destroy;
	db->vtable.sync    = &mail_db_sql_sync;
	db->vtable.create  = &mail_db_sql_create;
	db->vtable.remove  = &mail_db_sql_remove;
	db->vtable.save    = &mail_db_sql_save;
	db->vtable.load    = &mail_db_sql_load;
	db->vtable.loadall = &mail_db_sql_loadall;
	db->vtable.iterator = &mail_db_sql_iterator;

	// initialize to default values
	db->owner = owner;
	db->mails = NULL;

	// other settings
	db->mail_db = db->owner->table_mails;

	return &db->vtable;
}
