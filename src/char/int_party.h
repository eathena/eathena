// $Id: int_party.h,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

int inter_party_init();
void inter_party_final();


int inter_party_parse_frommap(int fd);
int inter_party_leave(uint32 party_id, uint32 account_id);


#endif
