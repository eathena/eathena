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

	/// Erases all regs associated with the specified char_id.
	bool (*remove)(CharRegDB* self, const int char_id);

	/// Saves the provided regs into persistent storage, erasing previous data.
	bool (*save)(CharRegDB* self, const struct regs* reg, int char_id);

	/// Loads character regs from persistent storage.
	/// Unused fields in the output array are zeroed.
	bool (*load)(CharRegDB* self, struct regs* reg, int char_id);

	/// Returns an iterator over all character regs.
	///
	/// @param self Database
	/// @return Iterator
	CharRegDBIterator* (*iterator)(CharRegDB* self);
};


#endif /* _CHARREGDB_H_ */
