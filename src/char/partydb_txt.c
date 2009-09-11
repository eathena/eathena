// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"
#include "partydb.h"
#include "charserverdb.h"
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

	// state
	CharServerDB_TXT* owner;
	DBMap* parties;// int party_id -> struct party* g (releases data)
	DBMap* idx_name;// char* name -> struct party* g (case-sensitive, WARNING: uses data of DBMap parties)
	int next_party_id;
	bool dirty;

	// settings
	const char* party_db;

} PartyDB_TXT;


/// @private
static bool mmo_party_fromstr(PartyDB_TXT* db, struct party* p, char* str, unsigned int version)
{
	struct party* tmp;

	memset(p, 0, sizeof(*p));

	if( version == 20090906 )
	{
		int fields[3+1][2];
		int base[3+1][2];
		int members[MAX_PARTY+1][2];
		int member[3+1][2];
		int nmembers;
		int i;

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
	}
	else
	if( version == 0 )
	{
		int party_id;
		char name[256];
		int exp;
		int item;
		int n;
		int i;

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
	}
	else
	{// unmatched row	
		return false;
	}

	// uniqueness checks
	tmp = (struct party*)idb_get(db->parties, p->party_id);
	if( tmp != NULL )
	{
		ShowError(CL_RED"mmo_party_fromstr: Collision on id %d between party '%s' and existing party '%s'!\n", p->party_id, p->name, tmp->name);
		return false;
	}
	tmp = (struct party*)strdb_get(db->idx_name, p->name);
	if( tmp != NULL )
	{
		ShowError(CL_RED"mmo_party_fromstr: Collision on name '%s' between party %d and existing party %d!\n", p->party_id, p->name, tmp->name);
		return false;
	}

	return true;
}


/// @private
static bool mmo_party_tostr(const struct party* p, char* str)
{
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
	DBMap* parties;
	char line[8192];
	FILE *fp;
	unsigned int version = 0;

	// create party database
	if( db->parties == NULL )
		db->parties = idb_alloc(DB_OPT_RELEASE_DATA);
	if( db->idx_name == NULL )
		db->idx_name = strdb_alloc(DB_OPT_DUP_KEY, 0);
	parties = db->parties;
	db_clear(parties);
	db_clear(db->idx_name);

	// open data file
	fp = fopen(db->party_db, "r");
	if( fp == NULL )
	{
		ShowError("Party file not found: %s.\n", db->party_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int party_id, n;
		struct party p;
		struct party* tmp;
		unsigned int v;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &party_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( party_id > db->next_party_id )
				db->next_party_id = party_id;
			continue;
		}

		if( !mmo_party_fromstr(db, &p, line, version) )
		{
			ShowError("party_db_txt_init: skipping invalid data: %s", line);
			continue;
		}

		// record entry in db
		tmp = (struct party*)aMalloc(sizeof(struct party));
		memcpy(tmp, &p, sizeof(struct party));
		idb_put(parties, p.party_id, tmp);
		strdb_put(db->idx_name, p.name, tmp);

		if( p.party_id >= db->next_party_id )
			db->next_party_id = p.party_id + 1;
	}

	// close data file
	fclose(fp);

	db->dirty = false;
	return true;
}


/// @protected
static void party_db_txt_destroy(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// delete party database
	if( db->idx_name != NULL )
	{
		db_destroy(db->idx_name);
		db->idx_name = NULL;
	}
	if( parties != NULL )
	{
		db_destroy(parties);
		db->parties = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool party_db_txt_sync(PartyDB* self, bool force)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	FILE *fp;
	int lock;
	struct DBIterator* iter;
	struct party* p;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->party_db, &lock);
	if( fp == NULL )
	{
		ShowError("party_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->party_db);
		return false;
	}

	fprintf(fp, "%d\n", PARTYDB_TXT_DB_VERSION);

	iter = db->parties->iterator(db->parties);
	for( p = (struct party*)iter->first(iter,NULL); iter->exists(iter); p = (struct party*)iter->next(iter,NULL) )
	{
		char buf[8192]; // ought to be big enough ^^
		mmo_party_tostr(p, buf);
		fprintf(fp, "%s\n", buf);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_party_id);
	iter->destroy(iter);

	lock_fclose(fp, db->party_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool party_db_txt_create(PartyDB* self, struct party* p)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;
	struct party* tmp;

	// decide on the party id to assign
	int party_id = ( p->party_id != -1 ) ? p->party_id : db->next_party_id;

	// data restrictions
	if( p->party_id != -1 && self->id2name(self, p->party_id, NULL, 0) )
		return false;// id is being used
	if( self->name2id(self, p->name, NULL) )
		return false;// name is being used

	// copy the data and store it in the db
	CREATE(tmp, struct party, 1);
	memcpy(tmp, p, sizeof(struct party));
	tmp->party_id = party_id;
	idb_put(parties, party_id, tmp);
	strdb_put(db->idx_name, tmp->name, tmp);

	// increment the auto_increment value
	if( party_id >= db->next_party_id )
		db->next_party_id = party_id + 1;

	// write output
	p->party_id = party_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_remove(PartyDB* self, const int party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;
	struct party* tmp;

	tmp = (struct party*)idb_get(parties, party_id);
	if( tmp == NULL )
		return true;// nothing to do
	strdb_remove(db->idx_name, tmp->name);
	idb_remove(parties, party_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_save(PartyDB* self, const struct party* p, enum party_save_flags flag, int index)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;
	bool name_changed = false;

	// retrieve previous data
	struct party* tmp = idb_get(parties, p->party_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	if( strcmp(p->name, tmp->name) != 0 )
	{// name changed
		name_changed = true;
		if( strdb_get(db->idx_name, p->name) != NULL )
			return false;// name is being used
	}
	
	// overwrite with new data
	if( name_changed )
	{
		strdb_remove(db->idx_name, tmp->name);
		strdb_put(db->idx_name, p->name, tmp);
	}
	memcpy(tmp, p, sizeof(struct party));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_load(PartyDB* self, struct party* p, int party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// retrieve data
	struct party* tmp = idb_get(parties, party_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(p, tmp, sizeof(struct party));

	return true;
}


/// Returns an iterator over all parties.
/// @protected
static CSDBIterator* party_db_txt_iterator(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->parties));
}


/// @protected
static bool party_db_txt_id2name(PartyDB* self, int party_id, char* name, size_t size)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// retrieve data
	struct party* tmp = (struct party*)idb_get(parties, party_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( name != NULL )
		safestrncpy(name, tmp->name, size);
	
	return true;
}


/// @protected
static bool party_db_txt_name2id(PartyDB* self, const char* name, int* party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// retrieve data
	struct party* tmp = (struct party*)strdb_get(db->idx_name, name);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	if( party_id != NULL )
		*party_id = tmp->party_id;

	return true;
}


/// Constructs a new PartyDB interface.
/// @protected
PartyDB* party_db_txt(CharServerDB_TXT* owner)
{
	PartyDB_TXT* db = (PartyDB_TXT*)aCalloc(1, sizeof(PartyDB_TXT));

	// set up the vtable
	db->vtable.p.init      = &party_db_txt_init;
	db->vtable.p.destroy   = &party_db_txt_destroy;
	db->vtable.p.sync      = &party_db_txt_sync;
	db->vtable.create    = &party_db_txt_create;
	db->vtable.remove    = &party_db_txt_remove;
	db->vtable.save      = &party_db_txt_save;
	db->vtable.load      = &party_db_txt_load;
	db->vtable.id2name   = &party_db_txt_id2name;
	db->vtable.name2id   = &party_db_txt_name2id;
	db->vtable.iterator  = &party_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->parties = NULL;
	db->idx_name = NULL;
	db->next_party_id = START_PARTY_NUM;
	db->dirty = false;

	// other settings
	db->party_db = db->owner->file_parties;

	return &db->vtable;
}
