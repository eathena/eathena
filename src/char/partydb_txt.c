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
	DBMap* parties;
	int next_party_id;
	bool dirty;

	// settings
	bool case_sensitive;
	const char* party_db;

} PartyDB_TXT;


/// @private
static bool mmo_party_fromstr(struct party* p, char* str, unsigned int version)
{
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
	parties = db->parties;
	db_clear(parties);

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
		struct party_data p;
		struct party_data* tmp;
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

		if( !mmo_party_fromstr(&p.party, line, version) )
		{
			ShowError("party_db_txt_init: skipping invalid data: %s", line);
			continue;
		}

		// record entry in db
		tmp = (struct party_data*)aMalloc(sizeof(struct party_data));
		memcpy(tmp, &p, sizeof(struct party_data));
		idb_put(parties, p.party.party_id, tmp);

		if( p.party.party_id >= db->next_party_id )
			db->next_party_id = p.party.party_id + 1;
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
	if( parties != NULL )
	{
		db_destroy(parties);
		db->parties = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// @protected
static bool party_db_txt_sync(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	FILE *fp;
	int lock;
	struct DBIterator* iter;
	struct party_data* p;

	fp = lock_fopen(db->party_db, &lock);
	if( fp == NULL )
	{
		ShowError("party_db_txt_sync: can't write [%s] !!! data is lost !!!\n", db->party_db);
		return false;
	}

	fprintf(fp, "%d\n", PARTYDB_TXT_DB_VERSION);

	iter = db->parties->iterator(db->parties);
	for( p = (struct party_data*)iter->first(iter,NULL); iter->exists(iter); p = (struct party_data*)iter->next(iter,NULL) )
	{
		char buf[8192]; // ought to be big enough ^^
		mmo_party_tostr(&p->party, buf);
		fprintf(fp, "%s\n", buf);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_party_id);
	iter->destroy(iter);

	lock_fclose(fp, db->party_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool party_db_txt_create(PartyDB* self, struct party_data* p)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;
	struct party_data* tmp;

	// decide on the party id to assign
	int party_id = ( p->party.party_id != -1 ) ? p->party.party_id : db->next_party_id;

	// check if the party_id is free
	tmp = idb_get(parties, party_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("party_db_txt_create: cannot create party %d:'%s', this id is already occupied by %d:'%s'!\n", party_id, p->party.name, party_id, tmp->party.name);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct party_data, 1);
	memcpy(tmp, p, sizeof(struct party_data));
	tmp->party.party_id = party_id;
	idb_put(parties, party_id, tmp);

	// increment the auto_increment value
	if( party_id >= db->next_party_id )
		db->next_party_id = party_id + 1;

	// write output
	p->party.party_id = party_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_remove(PartyDB* self, const int party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	idb_remove(parties, party_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_save(PartyDB* self, const struct party_data* p, enum party_save_flags flag, int index)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;
	int party_id = p->party.party_id;

	// retrieve previous data
	struct party_data* tmp = idb_get(parties, party_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, p, sizeof(struct party_data));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool party_db_txt_load(PartyDB* self, struct party_data* p, int party_id)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// retrieve data
	struct party_data* tmp = idb_get(parties, party_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(p, tmp, sizeof(struct party_data));

	return true;
}


/// @protected
static bool party_db_txt_name2id(PartyDB* self, int* party_id, const char* name)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	DBMap* parties = db->parties;

	// retrieve data
	struct DBIterator* iter;
	struct party_data* tmp;
	int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

	iter = parties->iterator(parties);
	for( tmp = (struct party_data*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct party_data*)iter->next(iter,NULL) )
		if( compare(name, tmp->party.name) == 0 )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	if( party_id != NULL )
		*party_id = tmp->party.party_id;

	return true;
}


/// Returns an iterator over all parties.
/// @protected
static CSDBIterator* party_db_txt_iterator(PartyDB* self)
{
	PartyDB_TXT* db = (PartyDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->parties));
}


/// Constructs a new PartyDB interface.
/// @protected
PartyDB* party_db_txt(CharServerDB_TXT* owner)
{
	PartyDB_TXT* db = (PartyDB_TXT*)aCalloc(1, sizeof(PartyDB_TXT));

	// set up the vtable
	db->vtable.init      = &party_db_txt_init;
	db->vtable.destroy   = &party_db_txt_destroy;
	db->vtable.sync      = &party_db_txt_sync;
	db->vtable.create    = &party_db_txt_create;
	db->vtable.remove    = &party_db_txt_remove;
	db->vtable.save      = &party_db_txt_save;
	db->vtable.load      = &party_db_txt_load;
	db->vtable.name2id   = &party_db_txt_name2id;
	db->vtable.iterator  = &party_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->parties = NULL;
	db->next_party_id = START_PARTY_NUM;
	db->dirty = false;

	// other settings
	db->case_sensitive = false;
	db->party_db = db->owner->file_parties;

	return &db->vtable;
}
