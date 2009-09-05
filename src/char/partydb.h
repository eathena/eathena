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


struct party_data
{
	struct party party;
	unsigned int min_lv, max_lv;
	int family; //Is this party a family? if so, this holds the child id.
	unsigned char size; //Total size of party.
};


struct PartyDB
{
	bool (*init)(PartyDB* self);

	void (*destroy)(PartyDB* self);

	bool (*sync)(PartyDB* self);

	bool (*create)(PartyDB* self, struct party_data* p);

	bool (*remove)(PartyDB* self, const int party_id);

	bool (*save)(PartyDB* self, const struct party_data* p, enum party_save_flags flag, int index);

	// retrieve data using party id
	// does not calculate temporary/derived values
	bool (*load)(PartyDB* self, struct party_data* p, int party_id);

	// look up party id using party name
	bool (*name2id)(PartyDB* self, int* party_id, const char* name);

	/// Returns an iterator over all parties.
	///
	/// @param self Database
	/// @return Iterator
	CSDBIterator* (*iterator)(PartyDB* self);
};


#endif /* _PARTYDB_H_ */
