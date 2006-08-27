// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STORAGE_H_
#define _INT_STORAGE_H_

int inter_storage_init(void);
void inter_storage_final();

int inter_storage_delete(uint32 account_id);
int inter_guild_storage_delete(uint32 guild_id);

int inter_storage_parse_frommap(int fd);


#endif
