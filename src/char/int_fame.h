// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_FAME_H_
#define _INT_FAME_H_

#include "../common/cbasetypes.h" // uint8
#include "../common/mmo.h" // struct fame_list

enum fame_type { FAME_SMITH = 1, FAME_CHEMIST = 2, FAME_TAEKWON = 3 };

void inter_fame_init(void);
void inter_fame_final(void);
bool fame_config_read(const char* key, const char* value);
bool get_fame_list(enum fame_type type, struct fame_list** list, int* size);
int inter_fame_parse_frommap(int fd);

#endif // _INT_FAME_H_
