// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARDB_H_
#define _CHARDB_H_

#include "../common/mmo.h"

struct character_data {
	struct mmo_charstatus status;
	int global_num;
	struct global_reg global[GLOBAL_REG_NUM];
};

extern int char_num, char_max;


int mmo_char_fromstr(char *str, struct mmo_charstatus *p, struct global_reg *reg, int *reg_num);
struct mmo_charstatus* search_character(int aid, int cid);
struct mmo_charstatus* search_character_byname(char* character_name);
int search_character_index(char* character_name);
char* search_character_name(int index);
int char_loadName(int char_id, char* name);


void char_clearparty(int party_id);
int char_married(int pl1,int pl2);
int char_child(int parent_id, int child_id);
int char_family(int cid1, int cid2, int cid3);


#endif /* _CHARDB_H_ */