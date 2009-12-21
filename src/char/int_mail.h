// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_MAIL_H_
#define _INT_MAIL_H_

#include "maildb.h"
#include "chardb.h"

void inter_mail_init(MailDB* mdb, CharDB* cdb);
void inter_mail_final(void);
int inter_mail_parse_frommap(int fd);

void inter_mail_send(int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item* item);

#endif // _INT_MAIL_H_
