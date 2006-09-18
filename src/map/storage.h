// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "mmo.h"

int storage_storageopen(map_session_data &sd);
int storage_storageadd(map_session_data &sd, size_t index,size_t amount);
int storage_storageget(map_session_data &sd, size_t index,size_t amount);
int storage_storageaddfromcart(map_session_data &sd,size_t index,size_t amount);
int storage_storagegettocart(map_session_data &sd,size_t index,size_t amount);
int storage_storageclose(map_session_data &sd);
int do_init_storage(void);
void do_final_storage(void);
struct pc_storage *account2storage(uint32 account_id);
struct pc_storage *account2storage2(uint32 account_id);
int storage_delete(uint32 account_id);
int storage_storage_quit(map_session_data &sd);
int storage_storage_save(map_session_data &sd);
void storage_storage_dirty(map_session_data &sd);

struct guild_storage *guild2storage(uint32 guild_id);
int guild_storage_delete(uint32 guild_id);
int storage_guild_storageopen(map_session_data &sd);
int guild_storage_additem(map_session_data &sd,struct guild_storage &stor,struct item &item_data,size_t amount);
int guild_storage_delitem(map_session_data &sd,struct guild_storage &stor,size_t n,size_t amount);
int storage_guild_storageadd(map_session_data &sd,size_t index,size_t amount);
int storage_guild_storageget(map_session_data &sd,size_t index,size_t amount);
int storage_guild_storageaddfromcart(map_session_data &sd,size_t index,size_t amount);
int storage_guild_storagegettocart(map_session_data &sd,size_t index,size_t amount);
int storage_guild_storageclose(map_session_data &sd);
int storage_guild_storage_quit(map_session_data &sd,int flag);
int storage_guild_storagesave(map_session_data &sd);

int storage_comp_item(const void *_i1, const void *_i2);
//int storage_comp_item(const struct item* i1, const struct item* i2);
void sortage_sortitem(struct pc_storage& stor);
void sortage_gsortitem(struct guild_storage& gstor);

#endif
