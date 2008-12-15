// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _REGDB_H_
#define _REGDB_H_

#include "../common/mmo.h" // struct regs

typedef struct AccRegDB AccRegDB;
typedef struct CharRegDB CharRegDB;


struct AccRegDB
{
	bool (*init)(AccRegDB* self);
	void (*destroy)(AccRegDB* self);

	bool (*sync)(AccRegDB* self);

	bool (*remove)(AccRegDB* self, const int account_id);

	bool (*save)(AccRegDB* self, const struct regs* reg, int account_id);
	bool (*load)(AccRegDB* self, struct regs* reg, int account_id);
};


struct CharRegDB
{
	bool (*init)(CharRegDB* self);
	void (*destroy)(CharRegDB* self);

	bool (*sync)(CharRegDB* self);

	bool (*remove)(CharRegDB* self, const int char_id);

	bool (*save)(CharRegDB* self, const struct regs* reg, int char_id);
	bool (*load)(CharRegDB* self, struct regs* reg, int char_id);
};


#endif /* _REGDB_H_ */
