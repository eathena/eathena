// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_FAME_H_
#define _INT_FAME_H_

#include "../common/mmo.h" // struct fame_list
#include "rankdb.h"

void inter_rank_init(RankDB* db);
void inter_rank_final(void);
bool rank_config_read(const char* key, const char* value);
int inter_rank_parse_frommap(int fd);
enum rank_type inter_rank_class2rankid(int class_);

#endif // _INT_FAME_H_
