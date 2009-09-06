// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CASTLEDB_H_
#define _CASTLEDB_H_


#include "../common/mmo.h" // struct guild_castle
#include "csdbiterator.h"


typedef struct CastleDB CastleDB;


struct CastleDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(CastleDB* self);
		void (*destroy)(CastleDB* self);
		bool (*sync)(CastleDB* self, bool force);
	} p;

	bool (*create)(CastleDB* self, struct guild_castle* gc);

	bool (*remove)(CastleDB* self, const int castle_id);

	bool (*remove_gid)(CastleDB* self, const int guild_id);

	bool (*save)(CastleDB* self, const struct guild_castle* gc);

	bool (*load)(CastleDB* self, struct guild_castle* gc, int castle_id);

	/// Returns an iterator over all castles.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(CastleDB* self);
};


#endif /* _CASTLEDB_H_ */
