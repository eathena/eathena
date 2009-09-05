// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARDB_H_
#define _CHARDB_H_


#include "../common/mmo.h" // struct mmo_charstatus, NAME_LENGTH
#include "csdbiterator.h"


typedef struct CharDB CharDB;


struct CharDB
{
	bool (*init)(CharDB* self);
	void (*destroy)(CharDB* self);

	bool (*sync)(CharDB* self);

	bool (*create)(CharDB* self, struct mmo_charstatus* status);

	bool (*remove)(CharDB* self, const int char_id);

	bool (*save)(CharDB* self, const struct mmo_charstatus* status);

	// retrieve data using charid
	bool (*load_num)(CharDB* self, struct mmo_charstatus* status, int char_id);

	// retrieve data using charname
	bool (*load_str)(CharDB* self, struct mmo_charstatus* status, const char* name);

	// retrieve data using accid + slot
	bool (*load_slot)(CharDB* self, struct mmo_charstatus* status, int account_id, int slot);

	// look up name using charid
	bool (*id2name)(CharDB* self, int char_id, char* name, size_t size);

	// look up charid/accid using name
	bool (*name2id)(CharDB* self, const char* name, int* char_id, int* account_id);

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