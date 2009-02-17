// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PARTYDB_H_
#define _PARTYDB_H_

#include "../common/mmo.h" // struct party

typedef struct PartyDB PartyDB;


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

	bool (*save)(PartyDB* self, const struct party_data* p);

	// retrieve data using party id
	bool (*load)(PartyDB* self, struct party_data* p, int party_id);

	// look up party id using party name
	bool (*name2id)(PartyDB* self, int* party_id, const char* name);
};


#endif /* _PARTYDB_H_ */
