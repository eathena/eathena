// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ACCREGDB_H_
#define _ACCREGDB_H_

#include "../common/mmo.h" // struct regs

typedef struct AccRegDB AccRegDB;


struct AccRegDB
{
	bool (*init)(AccRegDB* self);
	void (*destroy)(AccRegDB* self);

	bool (*sync)(AccRegDB* self);

	bool (*remove)(AccRegDB* self, const int account_id);

	bool (*save)(AccRegDB* self, const struct regs* reg, int account_id);
	bool (*load)(AccRegDB* self, struct regs* reg, int account_id);
};


#endif /* _ACCREGDB_H_ */
