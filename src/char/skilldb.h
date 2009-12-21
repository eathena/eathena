// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SKILLDB_H_
#define _SKILLDB_H_


#include "../common/mmo.h" // struct s_skill, MAX_SKILL
#include "csdbiterator.h"


typedef struct SkillDB SkillDB;
typedef struct s_skill skilllist[MAX_SKILL];


struct SkillDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(SkillDB* self);
		void (*destroy)(SkillDB* self);
		bool (*sync)(SkillDB* self, bool force);
	} p;

	bool (*remove)(SkillDB* self, const int char_id);

	bool (*save)(SkillDB* self, const skilllist* list, const int char_id);
	bool (*load)(SkillDB* self, skilllist* list, const int char_id);

	/// Returns an iterator over all skill lists.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(SkillDB* self);
};


#endif // _SKILLDB_H_
