// $Id: int_pet.h,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#ifndef _INT_PET_H_
#define _INT_PET_H_

int inter_pet_init();
int inter_pet_final();

int inter_pet_delete(uint32 pet_id);
int inter_pet_parse_frommap(int fd);



#endif
