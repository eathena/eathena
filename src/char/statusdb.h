// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STATUSDB_H_
#define _STATUSDB_H_

#include "../common/mmo.h" // struct status_change_data, NAME_LENGTH

typedef struct StatusDB StatusDB;


struct scdata
{
	int account_id, char_id;
	int count;
	struct status_change_data* data;
};


struct StatusDB
{
	bool (*init)(StatusDB* self);
	void (*destroy)(StatusDB* self);

	bool (*sync)(StatusDB* self);

	bool (*remove)(StatusDB* self, int char_id);

	bool (*save)(StatusDB* self, struct scdata* sc);
	bool (*load)(StatusDB* self, struct scdata* sc, int account_id, int char_id);

	/// Returns an iterator over all statuses.
	///
	/// @param self Database
	/// @return Iterator
	StatusDBIterator* (*iterator)(StatusDB* self);
};


#endif /* _STATUSDB_H_ */
