// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CSDB_H_
#define _CSDB_H_


#include "../common/cbasetypes.h" // bool
#include "../common/db.h" // DBIterator
#include "../common/strlib.h" // StringBuf
#include "charserverdb_txt.h"
#include "csdbiterator.h"


typedef struct CSDB_TXT CSDB_TXT;


/// CharServer TXT database interface
struct CSDB_TXT
{
	/// Initializes database.
	bool (*init)(CSDB_TXT* self);

	/// Finalizes database.
	bool (*destroy)(CSDB_TXT* self);

	/// Ensures that all data is flushed to secondary storage.
	bool (*sync)(CSDB_TXT* self, bool force);

	/// Checks whether an entry exists in the database.
	bool (*exists)(CSDB_TXT* self, int key);

	/// Loads data from db.
	/// Fails if no entry with the specified key exists; 'out_size' will be 0.
	/// Fails if 'size' is less than required; 'out_size' will hold the required size.
	bool (*load)(CSDB_TXT* self, int key, void* data, size_t size, size_t* out_size);

	/// Inserts a new entry.
	/// Fails if an entry with the specified key already exists.
	bool (*insert)(CSDB_TXT* self, int key, const void* data, size_t size);

	/// Updates an existing entry.
	/// Fails if no entry with the specified key exists.
	bool (*update)(CSDB_TXT* self, int key, const void* data, size_t size);

	/// Writes a new entry, replacing any existing one.
	bool (*replace)(CSDB_TXT* self, int key, const void* data, size_t size);

	/// Ensures that no entry with the specified key is present in the db.
	bool (*remove)(CSDB_TXT* self, int key);

	/// Returns an iterator over all entries in the database.
	DBIterator* (*iterator)(CSDB_TXT* self);

	/// Returns the next unused key value.
	int (*next_key)(CSDB_TXT* self);

	/// Callbacks.
	struct
	{
		/// Parses string containing serialized data into the provided data structure.
		/// @param str Input string [NOT NULL]
		/// @param key Will receive the key if parsing succeeds [NOT NULL]
		/// @param data Will receive the data if parsing succeeds
		/// @param size Capacity of the 'data' buffer
		/// @param out_size Will receive the capacity required to store all of the data [NOT NULL]
		/// @param version Version of the input string
		/// @return false if the string fails to parse, and true otherwise (even if not all data could be stored)
		bool (*fromstr)(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version);

		/// Serializes the provided data structure into a string.
		bool (*tostr)(char* str, int key, const void* data, size_t size);
	} p;
};


/// Constructs a new CSDB_TXT interface.
/// @param version Savefile version to use when writing. Using '0' turns off version output.
/// @param start_key Initial key value for new entries. Using '0' turns off %newid% output.
/// @protected
extern CSDB_TXT* csdb_txt(CharServerDB_TXT* owner, const char* savefile, unsigned int version, const int start_key);


/// Constructs a new CSDBIterator interface.
/// @protected
extern CSDBIterator* csdb_txt_iterator(DBIterator* db_iterator);


#endif // _CSDB_H_
