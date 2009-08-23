// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STATUS_H_
#define _INT_STATUS_H_

#include "statusdb.h"

void inter_status_init(StatusDB* db);
void inter_status_final(void);
int inter_status_parse_frommap(int fd);

#endif /* _INT_STATUS_H_ */
