// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ACCREGDB_H_
#define _ACCREGDB_H_

#include "../common/mmo.h" // struct regs

typedef struct AccRegDB AccRegDB;
typedef struct AccRegDBIterator AccRegDBIterator;


struct AccRegDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(AccRegDBIterator* self);

	/// Fetches the next account reg data and stores it in 'data'.
	/// @param self Iterator
	/// @param data an account's registry data
	/// @param key an account's account_id
	/// @return true if successful
	bool (*next)(AccRegDBIterator* self, struct regs* data, int* key);
};


struct AccRegDB
{
	bool (*init)(AccRegDB* self);
	void (*destroy)(AccRegDB* self);

	bool (*sync)(AccRegDB* self);

	/// Erases all regs associated with the specified account_id.
	bool (*remove)(AccRegDB* self, const int account_id);

	/// Saves the provided regs into persistent storage, erasing previous data.
	bool (*save)(AccRegDB* self, const struct regs* reg, int account_id);

	/// Loads account regs from persistent storage.
	/// Unused fields in the output array are zeroed.
	bool (*load)(AccRegDB* self, struct regs* reg, int account_id);

	/// Returns an iterator over all account regs.
	///
	/// @param self Database
	/// @return Iterator
	AccRegDBIterator* (*iterator)(AccRegDB* self);
};


#endif /* _ACCREGDB_H_ */
