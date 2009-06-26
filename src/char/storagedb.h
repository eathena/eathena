// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGEDB_H_
#define _STORAGEDB_H_

#include "../common/mmo.h" // struct storage_data

typedef struct StorageDB StorageDB;


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
