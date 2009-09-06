// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STATUSDB_H_
#define _STATUSDB_H_


#include "../common/mmo.h" // struct status_change_data, NAME_LENGTH
#include "csdbiterator.h"


typedef struct StatusDB StatusDB;


struct scdata
{
	int account_id, char_id;
	int count;
	struct status_change_data* data;
};


struct StatusDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(StatusDB* self);
		void (*destroy)(StatusDB* self);
		bool (*sync)(StatusDB* self, bool force);
	} p;

	bool (*remove)(StatusDB* self, int char_id);

	bool (*save)(StatusDB* self, struct scdata* sc);
	bool (*load)(StatusDB* self, struct scdata* sc, int char_id);

	/// Returns an iterator over all status entries.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(StatusDB* self);
};


#endif /* _STATUSDB_H_ */
