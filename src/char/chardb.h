// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARDB_H_
#define _CHARDB_H_


#include "../common/mmo.h" // struct mmo_charstatus, NAME_LENGTH
#include "csdbiterator.h"


typedef struct CharDB CharDB;


struct CharDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(CharDB* self);
		void (*destroy)(CharDB* self);
		bool (*sync)(CharDB* self, bool force);
	} p;

	bool (*create)(CharDB* self, struct mmo_charstatus* status);

	bool (*remove)(CharDB* self, const int char_id);

	bool (*save)(CharDB* self, const struct mmo_charstatus* status);

	// retrieve data using charid
	bool (*load_num)(CharDB* self, struct mmo_charstatus* status, int char_id);

	/// Retrieve character data by character name.
	/// Returns true if the name match is exact (case-sensitive) or unique (single case-insensitive).
	///
	/// @param self Database
	/// @param status Variable for the character data
	/// @param name Target character name
	/// @param case_sensitive If the search is case-sensitive
	/// @return true if exact or unique
	bool (*load_str)(CharDB* self, struct mmo_charstatus* status, const char* name, bool case_sensitive);

	// retrieve data using accid + slot
	bool (*load_slot)(CharDB* self, struct mmo_charstatus* status, int account_id, int slot);

	// look up name using charid
	bool (*id2name)(CharDB* self, int char_id, char* name, size_t size);

	/// Looks up a character name.
	/// Returns true if found.
	/// Optionally provides char_id, account_id and/or match count.
	///
	/// @param self Database
	/// @param case_sensitive If the name lookup is case sensitive
	/// @param char_id Optional variable for the char_id
	/// @param account_id Optional variable for the account_id
	/// @param count Optional variable for the number of matches (always filled in)
	/// @return true if found
	bool (*name2id)(CharDB* self, const char* name, bool case_sensitive, int* char_id, int* account_id, unsigned int* count);

	// look up charid using accid + slot
	bool (*slot2id)(CharDB* self, int account_id, int slot, int* char_id);

	/// Returns an iterator over all the characters.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(CharDB* self);

	/// Returns an iterator over all the characters of the account.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*characters)(CharDB* self, int account_id);
};


#endif /* _CHARDB_H_ */