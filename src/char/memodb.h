// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MEMODB_H_
#define _MEMODB_H_

#include "../common/mmo.h" // struct point, MAX_MEMOPOINTS
#include "csdbiterator.h"

typedef struct MemoDB MemoDB;
typedef struct point memolist[MAX_MEMOPOINTS];


struct MemoDB
{
	bool (*init)(MemoDB* self);
	void (*destroy)(MemoDB* self);

	bool (*sync)(MemoDB* self);

	bool (*remove)(MemoDB* self, int char_id);

	bool (*save)(MemoDB* self, const memolist* list, const int char_id);
	bool (*load)(MemoDB* self, memolist* list, const int char_id);

	/// Returns an iterator over all memo lists.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(MemoDB* self);
};


#endif /* _MEMODB_H_ */
