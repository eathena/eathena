// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STORAGEDB_H_
#define _STORAGEDB_H_


#include "../common/mmo.h" // struct item
#include "csdbiterator.h"


typedef struct StorageDB StorageDB;


// different storage types supported
enum storage_type
{
	STORAGE_INVENTORY,
	STORAGE_CART,
	STORAGE_KAFRA,
	STORAGE_GUILD,
};


struct StorageDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(StorageDB* self);
		void (*destroy)(StorageDB* self);
		bool (*sync)(StorageDB* self, bool force);
	} p;

	bool (*remove)(StorageDB* self, enum storage_type type, const int id);

	bool (*save)(StorageDB* self, const struct item* s, size_t size, enum storage_type type, int id);
	bool (*load)(StorageDB* self, struct item* s, size_t size, enum storage_type type, int id);

	/// Returns an iterator over all item arrays.
	///
	/// @param self Database
	/// @param type Type of storage
	/// @return Iterator
	CSDBIterator* (*iterator)(StorageDB* self, enum storage_type type);
};


#endif /* _STORAGEDB_H_ */
