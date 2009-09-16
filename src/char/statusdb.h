// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STATUSDB_H_
#define _STATUSDB_H_


#include "../common/mmo.h" // struct status_change_data, NAME_LENGTH
#include "csdbiterator.h"


typedef struct StatusDB StatusDB;


struct StatusDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(StatusDB* self);
		void (*destroy)(StatusDB* self);
		bool (*sync)(StatusDB* self, bool force);
	} p;

	bool (*remove)(StatusDB* self, int char_id);

	/// Saves the array of status changes.
	/// @param self Database
	/// @param sc Array of status_change_data structures to save
	/// @param size Number of fields in the array
	/// @param char_id The character's char_id
	/// @return true if operation succeeds, false if it fails
	bool (*save)(StatusDB* self, const struct status_change_data* sc, size_t size, int char_id);

	/// Loads status changes into the specified array.
	/// @param self Database
	/// @param sc Array of status_change_data structures to load into
	/// @param size Number of fields in the array (max. capacity)
	/// @param char_id The character's char_id
	/// @return true if operation succeeds, false if it fails
	bool (*load)(StatusDB* self, struct status_change_data* sc, size_t size, int char_id);

	/// Gives the number of status changes stored for this character.
	/// @param self Database
	/// @param char_id The character's char_id
	/// @return Number of status changes, or 0 on failure.
	size_t (*size)(StatusDB* self, int char_id);

	/// Returns an iterator over all status entries.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(StatusDB* self);
};


#endif /* _STATUSDB_H_ */
