// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

#include "questdb.h"

void inter_quest_init(QuestDB* db);
void inter_quest_final(void);
int inter_quest_parse_frommap(int fd);

#endif

