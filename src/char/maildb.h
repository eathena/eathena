// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAILDB_H_
#define _MAILDB_H_


#include "../common/mmo.h"  // struct mail_data, struct mail_message
#include "csdbiterator.h"


typedef struct MailDB MailDB;


struct MailDB
{
	bool (*init)(MailDB* self);
	void (*destroy)(MailDB* self);

	bool (*sync)(MailDB* self);

	bool (*create)(MailDB* self, struct mail_message* msg);
	bool (*remove)(MailDB* self, const int mail_id);

	bool (*save)(MailDB* self, const struct mail_message* msg);
	bool (*load)(MailDB* self, struct mail_message* msg, const int mail_id);
	bool (*loadall)(MailDB* self, struct mail_data* md, const int char_id);

	/// Returns an iterator over all mails.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(MailDB* self);
};


#endif  // _MAILDB_H_
