// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUESTDB_H_
#define _QUESTDB_H_


#include "../common/mmo.h" // struct quest
#include "csdbiterator.h"


typedef struct QuestDB QuestDB;


struct QuestDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(QuestDB* self);
		void (*destroy)(QuestDB* self);
		bool (*sync)(QuestDB* self, bool force);
	} p;

	/// Deletes a character's questlog.
	bool (*remove)(QuestDB* self, const int char_id);

	/// Saves the questlog.
	/// @param size Number of fields in the array to process.
	bool (*save)(QuestDB* self, const struct quest* log, size_t size, int char_id);

	/// Loads a character's questlog into the provided array.
	/// @param size Capacity of the array, in fields.
	bool (*load)(QuestDB* self, struct quest* log, size_t size, int char_id);

	/// Gives the number of questlog entries stored for this character.
	/// @param self Database
	/// @param char_id The character's char_id
	/// @return Number of questlog entries, or 0 on failure.
	size_t (*count)(QuestDB* self, int char_id);

	/// Returns an iterator over all quest entries.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(QuestDB* self);
};


#endif // _QUESTDB_H_
