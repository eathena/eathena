// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_MERCENARY_H_
#define _INT_MERCENARY_H_

#include "mercdb.h"

void inter_mercenary_init(MercDB* db);
void inter_mercenary_final(void);
int inter_mercenary_parse_frommap(int fd);
void inter_mercenary_delete(int mer_id);

#endif /* _INT_MERCENARY_H_ */
