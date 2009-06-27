// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _RANKDB_H_
#define _RANKDB_H_
/// \file
/// This file contains the public database interface that handles rankings.



#include "../common/mmo.h" // struct fame_list
#include "csdbiterator.h"



typedef struct RankDB RankDB;



#define RANK_BLACKSMITH   1// JOB_BLACKSMITH / JOB_WHITESMITH / JOB_BABY_BLACKSMITH
#define RANK_ALCHEMIST    2// JOB_ALCHEMIST / JOB_CREATOR / JOB_BABY_ALCHEMIST
#define RANK_TAEKWON      3// JOB_TAEKWON
//#define RANK_?            ?// JOB_GANGSI
//#define RANK_?            ?// JOB_DEATHKNIGHT
//#define RANK_?            ?// JOB_COLLECTOR



/// Public database interface to handle rankings.
struct RankDB
{
	bool (*init)(RankDB* self);
	void (*destroy)(RankDB* self);
	bool (*sync)(RankDB* self);

	/// Gets the top rankers in rank rank_id.
	/// Entries are sorted by points.
	/// The buffer is zeroed before being filled.
	/// Returns the number of rankers inserted in the list.
	///
	/// @param self Database interface
	/// @param rank_id Rank list id
	/// @param list Buffer that receives the top rankers
	/// @param count Size of the buffer
	/// @return Number of rankers in the list
	int (*get_top_rankers)(RankDB* self, int rank_id, struct fame_list* list, int count);

	/// Returns the number of points character char_id has in rank rank_id.
	/// Returns 0 if not found.
	///
	/// @param self Database interface
	/// @param rank_id Rank list id
	/// @param char_id Character id
	/// @return Number of points or 0 if not found
	int (*get_points)(RankDB* self, int rank_id, int char_id);

	/// Sets the number of points character char_id has in rank rank_id.
	///
	/// @param self Database interface
	/// @param rank_id Rank list id
	/// @param char_id Character id
	/// @param points Number of points
	void (*set_points)(RankDB* self, int rank_id, int char_id, int points);

	/// Returns an iterator over all rankings.
	///
	/// @param self Database
	/// @return Iterator
//	CSDBIterator* (*iterator)(RankDB* self);
};

#endif /* _RANKDB_H_ */
