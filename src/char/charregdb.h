// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARREGDB_H_
#define _CHARREGDB_H_

#include "../common/mmo.h" // struct regs

typedef struct CharRegDB CharRegDB;
typedef struct CharRegDBIterator CharRegDBIterator;


struct CharRegDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(CharRegDBIterator* self);

	/// Fetches the next character reg data and stores it in 'data'.
	/// @param self Iterator
	/// @param data a char's registry data
	/// @param key a char's char_id
	/// @return true if successful
	bool (*next)(CharRegDBIterator* self, struct regs* data, int* key);
};


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
