// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CASTLEDB_H_
#define _CASTLEDB_H_

#include "../common/mmo.h" // struct guild_castle

typedef struct CastleDB CastleDB;
typedef struct CastleDBIterator CastleDBIterator;


struct CastleDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(CastleDBIterator* self);

	/// Fetches the next castle in the database.
	/// Fills gc with the castle data.
	/// @param self Iterator
	/// @param gc Castle data
	/// @return true if successful
	bool (*next)(CastleDBIterator* self, struct guild_castle* gc);
};


struct CastleDB
{
	bool (*init)(CastleDB* self);

	void (*destroy)(CastleDB* self);

	bool (*sync)(CastleDB* self);

	bool (*create)(CastleDB* self, struct guild_castle* gc);

	bool (*remove)(CastleDB* self, const int castle_id);

	bool (*remove_gid)(CastleDB* self, const int guild_id);

	bool (*save)(CastleDB* self, const struct guild_castle* gc);

	bool (*load)(CastleDB* self, struct guild_castle* gc, int castle_id);

	/// Returns a new forward iterator.
	///
	/// @param self Database
	/// @return Iterator
	CastleDBIterator* (*iterator)(CastleDB* self);
};


#endif /* _CASTLEDB_H_ */
