// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_HOMUN_H_
#define _INT_HOMUN_H_

#include "homundb.h"

void inter_homun_init(HomunDB* db);
void inter_homun_final(void);
int inter_homun_parse_frommap(int fd);

#endif // _INT_HOMUN_H_
