// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOMUNDB_H_
#define _HOMUNDB_H_


#include "../common/mmo.h" // struct s_homunculus
#include "csdbiterator.h"


typedef struct HomunDB HomunDB;


struct HomunDB
{
	bool (*init)(HomunDB* self);

	void (*destroy)(HomunDB* self);

	bool (*sync)(HomunDB* self);

	bool (*create)(HomunDB* self, struct s_homunculus* p);

	bool (*remove)(HomunDB* self, int homun_id);

	bool (*save)(HomunDB* self, const struct s_homunculus* p);

	bool (*load)(HomunDB* self, struct s_homunculus* p, int homun_id);

	/// Returns an iterator over all homunculi.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(HomunDB* self);
};


#endif /* _HOMUNDB_H_ */
