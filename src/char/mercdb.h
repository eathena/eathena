// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MERCDB_H_
#define _MERCDB_H_

#include "../common/mmo.h" // struct s_mercenary
#include "csdbiterator.h"

typedef struct MercDB MercDB;


struct MercDB
{
	bool (*init)(MercDB* self);

	void (*destroy)(MercDB* self);

	bool (*sync)(MercDB* self);

	bool (*create)(MercDB* self, struct s_mercenary* md);

	bool (*remove)(MercDB* self, int merc_id);

	bool (*save)(MercDB* self, const struct s_mercenary* md);

	bool (*load)(MercDB* self, struct s_mercenary* md, int merc_id);

	/// Returns an iterator over all mercenaries.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(MercDB* self);
};


#endif /* _MERCDB_H_ */
