// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STORAGE_H_
#define _INT_STORAGE_H_

#include "storagedb.h"

void inter_storage_init(StorageDB* db);
void inter_storage_final(void);
int inter_storage_parse_frommap(int fd);

bool inter_storage_delete(int account_id);
int inter_guild_storage_delete(int guild_id);

#endif /* _INT_STORAGE_H_ */
