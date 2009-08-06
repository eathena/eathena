// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

#include "partydb.h"

void inter_party_init(PartyDB* db);
void inter_party_final(void);
int inter_party_parse_frommap(int fd);
void inter_party_leave(int party_id,int account_id, int char_id);
void int_party_calc_state(struct party_data *p);

#endif /* _INT_PARTY_H_ */
