// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _FRIENDDB_H_
#define _FRIENDDB_H_


#include "../common/mmo.h" // struct s_friend, MAX_FRIENDS
#include "csdbiterator.h"


typedef struct FriendDB FriendDB;
typedef struct s_friend friendlist[MAX_FRIENDS];


struct FriendDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(FriendDB* self);
		void (*destroy)(FriendDB* self);
		bool (*sync)(FriendDB* self, bool force);
	} p;

	bool (*remove)(FriendDB* self, const int char_id);

	bool (*save)(FriendDB* self, const friendlist* list, const int char_id);
	bool (*load)(FriendDB* self, friendlist* list, const int char_id);

	/// Returns an iterator over all friend lists.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(FriendDB* self);
};


#endif // _FRIENDDB_H_
