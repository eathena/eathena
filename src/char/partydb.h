// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PARTYDB_H_
#define _PARTYDB_H_


#include "../common/mmo.h" // struct party
#include "csdbiterator.h"


typedef struct PartyDB PartyDB;


//Party Flags on what to save/delete.
enum party_save_flags
{
	PS_CREATE    = 0x01, //Create a new party entry (index holds leader's info)
	PS_BASIC     = 0x02, //Update basic party info.
	PS_LEADER    = 0x04, //Update party's leader
	PS_ADDMEMBER = 0x08, //Add new party member (index specifies position)
	PS_DELMEMBER = 0x10, //Remove party member (index specifies position)
};


struct PartyDB
{
	/// For use by CharServerDB.
	/// @protected
	struct
	{
		bool (*init)(PartyDB* self);
		void (*destroy)(PartyDB* self);
		bool (*sync)(PartyDB* self, bool force);
	} p;

	/// Creates a party.
	/// Set p->party_id to -1 to auto-assign an id; it will be updated to the chosen id.
	/// Returns false if the id or name are being used.
	/// Returns true if successful.
	///
	/// @param self Database
	/// @param p Party data
	/// @return true if successful
	bool (*create)(PartyDB* self, struct party* p);

	bool (*remove)(PartyDB* self, const int party_id);

	bool (*save)(PartyDB* self, const struct party* p, enum party_save_flags flag, int index);

	// retrieve data using party id
	// does not calculate temporary/derived values
	bool (*load)(PartyDB* self, struct party* p, int party_id);

	// look up party name using party id
	bool (*id2name)(PartyDB* self, int party_id, char* name, size_t size);

	// look up party id using party name
	bool (*name2id)(PartyDB* self, const char* name, int* party_id);

	/// Returns an iterator over all parties.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(PartyDB* self);
};


#endif /* _PARTYDB_H_ */
