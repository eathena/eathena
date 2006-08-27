// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

int inter_party_init();
void inter_party_final();


int inter_party_parse_frommap(int fd);
int inter_party_leave(uint32 party_id, uint32 account_id);


#endif
