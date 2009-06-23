// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _FRIENDDB_H_
#define _FRIENDDB_H_

#include "../common/mmo.h" // struct s_friend, MAX_FRIENDS

typedef struct FriendDB FriendDB;
typedef struct FriendDBIterator FriendDBIterator;
typedef struct s_friend friendlist[MAX_FRIENDS];


struct FriendDBIterator
{
	/// Destroys this iterator, releasing all allocated memory (including itself).
	///
	/// @param self Iterator
	void (*destroy)(FriendDBIterator* self);

	/// Fetches the next friend data and stores it in 'data'.
	/// @param self Iterator
	/// @param data a characters's friend list
	/// @param key a characters's char_id
	/// @return true if successful
	bool (*next)(FriendDBIterator* self, friendlist* data, int* key);
};


struct FriendDB
{
	bool (*init)(FriendDB* self);
	void (*destroy)(FriendDB* self);

	bool (*sync)(FriendDB* self);

	bool (*remove)(FriendDB* self, const int char_id);

	bool (*save)(FriendDB* self, const friendlist* list, const int char_id);
	bool (*load)(FriendDB* self, friendlist* list, const int char_id);

	/// Returns an iterator over all friend lists.
	///
	/// @param self Database
	/// @return Iterator
	FriendDBIterator* (*iterator)(FriendDB* self);
};


#endif /* _FRIENDDB_H_ */
