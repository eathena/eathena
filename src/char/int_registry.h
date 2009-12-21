// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_REGISTRY_H_
#define _INT_REGISTRY_H_

#include "accregdb.h"
#include "charregdb.h"

void inter_registry_init(AccRegDB* accregdb, CharRegDB* charregdb);
void inter_registry_final(void);
int inter_registry_parse_frommap(int fd);

#endif // _INT_REGISTRY_H_
