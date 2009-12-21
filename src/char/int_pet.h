// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PET_H_
#define _INT_PET_H_

#include "petdb.h"

void inter_pet_init(PetDB* db);
void inter_pet_final(void);
bool inter_pet_delete(int pet_id);
int inter_pet_parse_frommap(int fd);

#endif // _INT_PET_H_
