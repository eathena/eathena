// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PETDB_H_
#define _PETDB_H_

#include "../common/mmo.h" // struct s_pet, NAME_LENGTH

typedef struct PetDB PetDB;
typedef struct PetDBIterator PetDBIterator;


struct PetDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(PetDBIterator* self);

	/// Fetches the next pet data and stores it in 'data'.
	/// @param self Iterator
	/// @param data a pet's data
	/// @return true if successful
	bool (*next)(PetDBIterator* self, struct s_pet* data);
};


struct PetDB
{
	bool (*init)(PetDB* self);
	void (*destroy)(PetDB* self);

	bool (*sync)(PetDB* self);

	bool (*create)(PetDB* self, struct s_pet* pd);

	bool (*remove)(PetDB* self, const int pet_id);

	bool (*save)(PetDB* self, const struct s_pet* pd);

	// retrieve data using pet id
	bool (*load)(PetDB* self, struct s_pet* pd, int pet_id);

	/// Returns an iterator over all pets.
	///
	/// @param self Database
	/// @return Iterator
	PetDBIterator* (*iterator)(PetDB* self);
};


#endif /* _PETDB_H_ */
