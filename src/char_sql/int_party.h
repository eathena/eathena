// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PARTY_SQL_H_
#define _INT_PARTY_SQL_H_

struct party;

int inter_party_parse_frommap(int fd);
int inter_party_sql_init(void);
void inter_party_sql_final(void);
int inter_party_leave(int party_id,int account_id, int char_id);
int inter_party_CharOnline(int char_id, int party_id);
int inter_party_CharOffline(int char_id, int party_id);
//Required for the TXT->SQL converter
int inter_party_tosql(struct party *p, int flag, int index);

#endif /* _INT_PARTY_SQL_H_ */
