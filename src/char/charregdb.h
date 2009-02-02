// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARREGDB_H_
#define _CHARREGDB_H_

#include "../common/mmo.h" // struct regs

typedef struct CharRegDB CharRegDB;


struct CharRegDB
{
	bool (*init)(CharRegDB* self);
	void (*destroy)(CharRegDB* self);

	bool (*sync)(CharRegDB* self);

	bool (*remove)(CharRegDB* self, const int char_id);

	bool (*save)(CharRegDB* self, const struct regs* reg, int char_id);
	bool (*load)(CharRegDB* self, struct regs* reg, int char_id);
};


#endif /* _CHARREGDB_H_ */
