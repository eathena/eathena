// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ACCREGDB_H_
#define _ACCREGDB_H_


#include "../common/mmo.h" // struct regs
#include "csdbiterator.h"


typedef struct AccRegDB AccRegDB;


struct AccRegDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(AccRegDB* self);
		void (*destroy)(AccRegDB* self);
		bool (*sync)(AccRegDB* self, bool force);
	} p;

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
	CSDBIterator* (*iterator)(AccRegDB* self);
};


#endif /* _ACCREGDB_H_ */
