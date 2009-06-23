// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGEDB_H_
#define _STORAGEDB_H_

#include "../common/mmo.h" // struct storage_data

typedef struct StorageDB StorageDB;
typedef struct StorageDBIterator StorageDBIterator;


struct StorageDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(StorageDBIterator* self);

	/// Fetches the next storage data and stores it in 'data'.
	/// @param self Iterator
	/// @param data an account's storage data
	/// @param key an account's account_id
	/// @return true if successful
	bool (*next)(StorageDBIterator* self, struct storage_data* data, int* key);
};


struct StorageDB
{
	bool (*init)(StorageDB* self);
	void (*destroy)(StorageDB* self);

	bool (*sync)(StorageDB* self);

	bool (*remove)(StorageDB* self, const int account_id);

	bool (*save)(StorageDB* self, const struct storage_data* s, int account_id);
	bool (*load)(StorageDB* self, struct storage_data* s, int account_id);

	/// Returns an iterator over all storages.
	///
	/// @param self Database
	/// @return Iterator
	StorageDBIterator* (*iterator)(StorageDB* self);
};


#endif /* _STORAGEDB_H_ */
