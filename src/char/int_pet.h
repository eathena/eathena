// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PET_H_
#define _INT_PET_H_

int inter_pet_init();
int inter_pet_final();

int inter_pet_delete(uint32 pet_id);
int inter_pet_parse_frommap(int fd);



#endif
