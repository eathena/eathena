// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "partydb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define PARTYDB_TXT_DB_VERSION 20090906
#define START_PARTY_NUM 1


/// Internal structure.
/// @private
typedef struct PartyDB_TXT
{
	// public interface
	PartyDB vtable;

	// data provider
	CSDB_TXT* db;

	// indexes
	DBMap* idx_name;// char* name -> int party_id (case-sensitive)

} PartyDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool party_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct party* p = (struct party*)data;

	*out_size = sizeof(*p);

	if( size < sizeof(*p) )
		return true;

	if( version == 20090906 )
	{
		int fields[3+1][2];
		int base[3+1][2];
		int members[MAX_PARTY+1][2];
		int member[3+1][2];
		int nmembers;
		int i;

		memset(p, 0, sizeof(*p));

		// layout := <party id> \t <party base data> \t <party member list>
		if( sv_parse(str, strlen(str), 0, '\t', (int*)fields, 2*ARRAYLENGTH(fields), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF)) != 3 )
			return false;

		// party id := <party id>
		p->party_id = strtol(&str[fields[1][0]], NULL, 10);

		// party base data := <name>,<exp>,<item>
		if( sv_parse(str, fields[2][1], fields[2][0], ',', (int*)base, 2*ARRAYLENGTH(base), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF)) != 3 )
			return false;
		sv_unescape_c(p->name, &str[base[1][0]], base[1][1]-base[1][0]);
		p->exp = strtoul(&str[base[2][0]], NULL, 10) ? 1 : 0;
		p->item = strtoul(&str[base[3][0]], NULL, 10);

		// party member list := {<party member data> }*
		nmembers = sv_parse(str, fields[3][1], fields[3][0], ' ', (int*)members, 2*ARRAYLENGTH(members), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF));
		for( i = 0; i < nmembers; ++i )
		{
			// party member data := <account id>,<char id>,<leader flag>
			if( sv_parse(str, members[i+1][1], members[i+1][0], ',', (int*)member, 2*ARRAYLENGTH(member), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF)) != 3 )
				return false;
			p->member[i].account_id = strtol(&str[member[1][0]], NULL, 10);
			p->member[i].char_id = strtol(&str[member[2][0]], NULL, 10);
			p->member[i].leader = strtoul(&str[member[3][0]], NULL, 10) ? 1 : 0;
		}

		*key = p->party_id;
	}
	else
	if( version == 00000000 )
	{
		int party_id;
		char name[256];
		int exp;
		int item;
		int n;
		int i;

		memset(p, 0, sizeof(*p));

		n = 0;
		if( sscanf(str, "%d\t%255[^\t]\t%d,%d%n", &party_id, name, &exp, &item, &n) != 4 || str[n] != '\t' )
			return false;

		p->party_id = party_id;
		safestrncpy(p->name, name, sizeof(p->name));
		p->exp = exp ? 1:0;
		p->item = item;

		str += n + 1;

		for( i = 0; i < MAX_PARTY; i++ )
		{
			struct party_member* m = &p->member[i];
			int account_id;
			int char_id;
			int leader;

			n = 0;
			if( sscanf(str, "%d,%d,%d%n", &account_id, &char_id, &leader, &n) != 3 || str[n] != '\t' )
				return false;

			m->account_id = account_id;
			m->char_id = char_id; 
			m->leader = leader ? 1:0;

			str += n + 1;
		}

		*key = p->party_id;
	}
	else
	{// unmatched row	
		return false;
	}
/*
	// uniqueness checks
	if( db->db->exists(db->db, p->party_id) )
	{
		char tmp[NAME_LENGTH];
		db->vtable.id2name(&db->vtable, p->party_id, tmp, sizeof(tmp));
		ShowError(CL_RED"mmo_party_fromstr: Collision on id %d between party '%s' and existing party '%s'!\n", p->party_id, p->name, tmp);
		return false;
	}

	if( strdb_exists(db->idx_name, p->name) )
	{
		int tmp;
		tmp = (int)strdb_get(db->idx_name, p->name);
		ShowError(CL_RED"mmo_party_fromstr: Collision on name '%s' between party %d and existing party %d!\n", p->party_id, p->name, tmp);
		return false;
	}
*/
	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool party_db_txt_tostr(char* str, size_t strsize, int key, const void* data, size_t datasize)
{
	const struct party* p = (const struct party*)data;
	char esc_name[sizeof(p->name)*4+1];
	bool first = true;
	int i;

	// write base data
	sv_escape_c(esc_name, p->name, strlen(p->name), ",");
	str += sprintf(str, "%d\t%s,%d,%d\t", p->party_id, p->name, p->exp, p->item);

	// write party member data
	for( i = 0; i < MAX_PARTY; i++ )
	{
		const struct party_member* m = &p->member[i];
		if( m->account_id == 0 && m->char_id == 0 )
			continue; // skip empty entries

		if( first )
			first = false;
		else
			str += sprintf(str, " ");

		str += sprintf(str, "%d,%d,%d", m->account_id, m->char_id, m->leader);
	}

	return true;
}


/// @protected
static bool party_db_txt_init(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	CSDBIterator* iter;
	int party_id;

	if( !db->db->init(db->db) )
		return false;

	// create index
	if( db->idx_name == NULL )
		db->idx_name = strdb_alloc(DB_OPT_DUP_KEY, 0);
	db_clear(db->idx_name);
	iter = db->db->iterator(db->db);
	while( iter->next(iter, &party_id) )
	{
		struct party p;
		if( !db->db->load(db->db, party_id, &p, sizeof(p), NULL) )
			continue;
		strdb_put(db->idx_name, p.name, (void*)party_id);
	}
	iter->destroy(iter);

	return true;
}


/// @protected
static void party_db_txt_destroy(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;

	// delete party database
	db->db->destroy(db->db);

	// delete indexes
	if( db->idx_name != NULL )
		db_destroy(db->idx_name);

	// delete entire structure
	aFree(db);
}


/// @protected
static bool party_db_txt_sync(PartyDB* self, bool force)
{
	CSDB_TXT* db = ((PartyDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool party_db_txt_create(PartyDB* self, struct party* p)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;

	if( p->party_id == -1 )
		p->party_id = db->db->next_key(db->db);

	// data restrictions
	if( self->id2name(self, p->party_id, NULL, 0) )
		return false;// id is being used
	if( self->name2id(self, p->name, NULL) )
		return false;// name is being used

	// store data
	db->db->insert(db->db, p->party_id, p, sizeof(*p));
	strdb_put(db->idx_name, p->name, (void*)p->party_id);

	return true;
}


/// @protected
static bool party_db_txt_remove(PartyDB* self, const int party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	struct party p;

	if( !db->db->load(db->db, party_id, &p, sizeof(p), NULL) )
		return true; // nothing to delete

	// delete from database and index
	db->db->remove(db->db, party_id);
	strdb_remove(db->idx_name, p.name);

	return true;
}


/// @protected
static bool party_db_txt_save(PartyDB* self, const struct party* p, enum party_save_flags flag, int index)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	struct party tmp;
	bool name_changed = false;

	// retrieve previous data
	if( !db->db->load(db->db, p->party_id, &tmp, sizeof(tmp), NULL) )
		return false; // entry not found

	// check integrity constraints
	if( strcmp(p->name, tmp.name) != 0 )
	{
		name_changed = true;
		if( strdb_exists(db->idx_name, p->name) )
			return false; // name already taken
	}

	// write new data
	if( !db->db->update(db->db, p->party_id, p, sizeof(*p)) )
		return false;

	// update index
	if( name_changed )
	{
		strdb_remove(db->idx_name, tmp.name);
		strdb_put(db->idx_name, p->name, (void*)p->party_id);
	}

	return true;
}


/// @protected
static bool party_db_txt_load(PartyDB* self, struct party* p, int party_id)
{
	CSDB_TXT* db = ((PartyDB_TXT*)self)->db;
	return db->load(db, party_id, p, sizeof(*p), NULL);
}


/// @protected
static bool party_db_txt_id2name(PartyDB* self, int party_id, char* name, size_t size)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	struct party p;

	if( !db->db->load(db->db, party_id, &p, sizeof(p), NULL) )
		return false;

	if( name != NULL )
		safestrncpy(name, p.name, size);

	return true;
}


/// @protected
static bool party_db_txt_name2id(PartyDB* self, const char* name, int* party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;

	if( !strdb_exists(db->idx_name, name) )
		return false;
	
	// store it
	if( party_id != NULL )
		*party_id = (int)strdb_get(db->idx_name, name);

	return true;
}


/// Returns an iterator over all parties.
/// @protected
static CSDBIterator* party_db_txt_iterator(PartyDB* self)
{
	CSDB_TXT* db = ((PartyDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new PartyDB interface.
/// @protected
PartyDB* party_db_txt(CharServerDB_TXT* owner)
{
	PartyDB_TXT* db = (PartyDB_TXT*)aCalloc(1, sizeof(PartyDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_parties, PARTYDB_TXT_DB_VERSION, START_PARTY_NUM);
	db->db->p.fromstr = &party_db_txt_fromstr;
	db->db->p.tostr   = &party_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &party_db_txt_init;
	db->vtable.p.destroy = &party_db_txt_destroy;
	db->vtable.p.sync    = &party_db_txt_sync;
	db->vtable.create    = &party_db_txt_create;
	db->vtable.remove    = &party_db_txt_remove;
	db->vtable.save      = &party_db_txt_save;
	db->vtable.load      = &party_db_txt_load;
	db->vtable.id2name   = &party_db_txt_id2name;
	db->vtable.name2id   = &party_db_txt_name2id;
	db->vtable.iterator  = &party_db_txt_iterator;

	return &db->vtable;
}
