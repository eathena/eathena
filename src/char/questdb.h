// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUESTDB_H_
#define _QUESTDB_H_

#include "../common/mmo.h" // struct quest, MAX_QUEST_DB
#include "csdbiterator.h"

typedef struct QuestDB QuestDB;
typedef struct quest questlog[MAX_QUEST_DB];


struct QuestDB
{
	bool (*init)(QuestDB* self);
	void (*destroy)(QuestDB* self);

	bool (*sync)(QuestDB* self);

	/// Deletes a character's entire quest log.
	bool (*remove)(QuestDB* self, const int char_id);

	bool (*add)(QuestDB* self, const struct quest* qd, const int char_id);
	bool (*update)(QuestDB* self, const struct quest* qd, const int char_id);
	bool (*del)(QuestDB* self, const int char_id, const int quest_id);
	bool (*load)(QuestDB* self, questlog* log, int char_id, int* const count);
	bool (*save)(QuestDB* self, questlog* log, int char_id);

	/// Returns an iterator over all quest entries.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(QuestDB* self);
};


#endif /* _QUESTDB_H_ */
