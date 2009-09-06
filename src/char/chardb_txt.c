// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "charserverdb_txt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define CHARDB_TXT_DB_VERSION 20090825
#define START_CHAR_NUM 1


/// Internal structure.
/// @private
typedef struct CharDB_TXT
{
	// public interface
	CharDB vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* chars;
	int next_char_id;
	bool dirty;

	// settings
	const char* char_db;
	bool case_sensitive;

} CharDB_TXT;


/// Function to set the character from the line (at read of characters file).
/// @private
static bool mmo_char_fromstr(CharDB* chars, const char* str, struct mmo_charstatus* cd, unsigned int version)
{
	const char* p = str;
	int col[53+1][2];
	int n;
	int tmp_charid;
	char tmp_name[NAME_LENGTH];
	char tmp_map[MAP_NAME_LENGTH];

	memset(cd, 0, sizeof(*cd));

	// key (char id)
	if( sscanf(p, "%d%n", &cd->char_id, &n) != 1 || p[n] != '\t' )
		return false;

	p += n + 1;

	// base data
	if( sv_parse(p, strlen(p), 0, ',', (int*)col, 2*ARRAYLENGTH(col), (e_svopt)(SV_ESCAPE_C|SV_TERMINATE_LF|SV_TERMINATE_CRLF)) != 53 )
		return false;

	cd->account_id     = (int)          strtol (&p[col[ 1][0]], NULL, 10);
	cd->slot           = (unsigned char)strtoul(&p[col[ 2][0]], NULL, 10);
	sv_unescape_c(cd->name, &p[col[3][0]], col[3][1]-col[3][0]);
	cd->class_         = (short)        strtol (&p[col[ 4][0]], NULL, 10);
	cd->base_level     = (unsigned int) strtoul(&p[col[ 5][0]], NULL, 10);
	cd->job_level      = (unsigned int) strtoul(&p[col[ 6][0]], NULL, 10);
	cd->base_exp       = (unsigned int) strtoul(&p[col[ 7][0]], NULL, 10);
	cd->job_exp        = (unsigned int) strtoul(&p[col[ 8][0]], NULL, 10);
	cd->zeny           = (int)          strtoul(&p[col[ 9][0]], NULL, 10);
	cd->hp             = (int)          strtoul(&p[col[10][0]], NULL, 10);
	cd->max_hp         = (int)          strtoul(&p[col[11][0]], NULL, 10);
	cd->sp             = (int)          strtoul(&p[col[12][0]], NULL, 10);
	cd->max_sp         = (int)          strtoul(&p[col[13][0]], NULL, 10);
	cd->str            = (short)        strtol (&p[col[14][0]], NULL, 10);
	cd->agi            = (short)        strtol (&p[col[15][0]], NULL, 10);
	cd->vit            = (short)        strtol (&p[col[16][0]], NULL, 10);
	cd->int_           = (short)        strtol (&p[col[17][0]], NULL, 10);
	cd->dex            = (short)        strtol (&p[col[18][0]], NULL, 10);
	cd->luk            = (short)        strtol (&p[col[19][0]], NULL, 10);
	cd->status_point   = (unsigned int) strtoul(&p[col[20][0]], NULL, 10);
	cd->skill_point    = (unsigned int) strtoul(&p[col[21][0]], NULL, 10);
	cd->option         = (unsigned int) strtoul(&p[col[22][0]], NULL, 10);
	cd->karma          = (unsigned char)strtoul(&p[col[23][0]], NULL, 10);
	cd->manner         = (short)        strtol (&p[col[24][0]], NULL, 10);
	cd->party_id       = (int)          strtol (&p[col[25][0]], NULL, 10);
	cd->guild_id       = (int)          strtol (&p[col[26][0]], NULL, 10);
	cd->pet_id         = (int)          strtol (&p[col[27][0]], NULL, 10);
	cd->hom_id         = (int)          strtol (&p[col[28][0]], NULL, 10);
	cd->mer_id         = (int)          strtol (&p[col[29][0]], NULL, 10);
	cd->hair           = (short)        strtol (&p[col[30][0]], NULL, 10);
	cd->hair_color     = (short)        strtol (&p[col[31][0]], NULL, 10);
	cd->clothes_color  = (short)        strtol (&p[col[32][0]], NULL, 10);
	cd->weapon         = (short)        strtol (&p[col[33][0]], NULL, 10);
	cd->shield         = (short)        strtol (&p[col[34][0]], NULL, 10);
	cd->head_top       = (short)        strtol (&p[col[35][0]], NULL, 10);
	cd->head_mid       = (short)        strtol (&p[col[36][0]], NULL, 10);
	cd->head_bottom    = (short)        strtol (&p[col[37][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[38][0]], col[38][1]-col[38][0]+1); cd->last_point.map = mapindex_name2id(tmp_map);
	cd->last_point.x   = (short)        strtol (&p[col[39][0]], NULL, 10);
	cd->last_point.y   = (short)        strtol (&p[col[40][0]], NULL, 10);
	safestrncpy(tmp_map, &p[col[41][0]], col[41][1]-col[41][0]+1); cd->save_point.map = mapindex_name2id(tmp_map);
	cd->save_point.x   = (short)        strtol (&p[col[42][0]], NULL, 10);
	cd->save_point.y   = (short)        strtol (&p[col[43][0]], NULL, 10);
	cd->partner_id     = (int)          strtol (&p[col[44][0]], NULL, 10);
	cd->father         = (int)          strtol (&p[col[45][0]], NULL, 10);
	cd->mother         = (int)          strtol (&p[col[46][0]], NULL, 10);
	cd->child          = (int)          strtol (&p[col[47][0]], NULL, 10);
	cd->arch_calls     = (int)          strtol (&p[col[48][0]], NULL, 10);
	cd->arch_faith     = (int)          strtol (&p[col[49][0]], NULL, 10);
	cd->spear_calls    = (int)          strtol (&p[col[50][0]], NULL, 10);
	cd->spear_faith    = (int)          strtol (&p[col[51][0]], NULL, 10);
	cd->sword_calls    = (int)          strtol (&p[col[52][0]], NULL, 10);
	cd->sword_faith    = (int)          strtol (&p[col[53][0]], NULL, 10);

	// uniqueness checks
	if( chars->id2name(chars, cd->char_id, tmp_name, sizeof(tmp_name)) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on id %d between character '%s' and existing character '%s'!\n", cd->char_id, cd->name, tmp_name);
		return false;
	}
	if( chars->name2id(chars, cd->name, &tmp_charid, NULL) )
	{
		ShowError(CL_RED"mmo_char_fromstr: Collision on name '%s' between character %d and existing character %d!\n", cd->name, cd->char_id, tmp_charid);
		return false;
	}

	return true;
}


/// Function to create the character line (for save)
/// @private
static int mmo_char_tostr(char *str, struct mmo_charstatus *p)
{
	char esc_name[4*NAME_LENGTH+1];
	char *str_p = str;

	sv_escape_c(esc_name, p->name, strlen(p->name), ",");

	// key (char id)
	str_p += sprintf(str_p, "%d", p->char_id);
	*(str_p++) = '\t';

	// base character data
	                  str_p += sprintf(str_p, "%d", p->account_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->slot);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", esc_name);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->class_);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->base_level);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->job_level);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->base_exp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->job_exp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->zeny);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->max_hp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->max_sp);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->str);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->agi);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->vit);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->int_);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->dex);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->luk);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->status_point);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->skill_point);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->option);
	*(str_p++) = ','; str_p += sprintf(str_p, "%u", p->karma);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->manner);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->party_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->guild_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->pet_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hom_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->mer_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hair);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->hair_color);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->clothes_color);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->weapon);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->shield);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_top);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_mid);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->head_bottom);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", mapindex_id2name(p->last_point.map));
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->last_point.x);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->last_point.y);
	*(str_p++) = ','; str_p += sprintf(str_p, "%s", mapindex_id2name(p->save_point.map));
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->save_point.x);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->save_point.y);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->partner_id);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->father);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->mother);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->child);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->arch_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->spear_faith);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_calls);
	*(str_p++) = ','; str_p += sprintf(str_p, "%d", p->sword_faith);

	*(str_p++) = '\0';

	return 0;
}


/// @protected
static bool char_db_txt_init(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars;

	char line[65536];
	int line_count = 0;
	FILE* fp;
	unsigned int version = 0;

	// create chars database
	if( db->chars == NULL )
		db->chars = idb_alloc(DB_OPT_RELEASE_DATA);
	chars = db->chars;
	db_clear(chars);

	// open data file
	fp = fopen(db->char_db, "r");
	if( fp == NULL )
	{
		ShowError("Characters file not found: %s.\n", db->char_db);
		return false;
	}

	// load data file
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int char_id, n;
		unsigned int v;
		struct mmo_charstatus* ch;

		line_count++;

		if( line[0] == '/' && line[1] == '/' )
			continue;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &char_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( char_id > db->next_char_id )
				db->next_char_id = char_id;
			continue;
		}

		// allocate memory for the char entry
		ch = (struct mmo_charstatus*)aMalloc(sizeof(struct mmo_charstatus));

		// parse char data
		if( !mmo_char_fromstr(self, line, ch, version) )
 		{
			ShowFatalError("char_db_txt_init: There was a problem processing data in file '%s', line #%d. Please fix manually. Shutting down to avoid data loss.\n", db->char_db, line_count);
			aFree(ch);
			exit(EXIT_FAILURE);
		}

		// record entry in db
		idb_put(chars, ch->char_id, ch);

		if( ch->char_id >= db->next_char_id )
			db->next_char_id = ch->char_id + 1;
	}

	// close data file
	fclose(fp);

	ShowStatus("mmo_char_init: %d characters read in %s.\n", chars->size(chars), db->char_db);

	db->dirty = false;
	return true;
}


/// @protected
static void char_db_txt_destroy(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// delete chars database
	if( chars != NULL )
	{
		db_destroy(chars);
		db->chars = NULL;
	}

	// delete entire structure
	aFree(db);
}


/// Dumps the entire char db (+ associated data) to disk
/// @protected
static bool char_db_txt_sync(CharDB* self, bool force)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	int lock;
	FILE *fp;
	void* data;
	struct DBIterator* iter;

	if( !force && !db->dirty )
		return true;// nothing to do

	// Data save
	fp = lock_fopen(db->char_db, &lock);
	if( fp == NULL )
	{
		ShowWarning("Server cannot save characters.\n");
		return false;
	}

	fprintf(fp, "%d\n", CHARDB_TXT_DB_VERSION); // savefile version

	iter = db->chars->iterator(db->chars);
	for( data = iter->first(iter,NULL); iter->exists(iter); data = iter->next(iter,NULL) )
	{
		struct mmo_charstatus* ch = (struct mmo_charstatus*) data;
		char line[65536]; // ought to be big enough

		mmo_char_tostr(line, ch);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", db->next_char_id);
	iter->destroy(iter);

	lock_fclose(fp, db->char_db, &lock);

	db->dirty = false;
	return true;
}


/// @protected
static bool char_db_txt_create(CharDB* self, struct mmo_charstatus* cd)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	struct mmo_charstatus* tmp;

	// decide on the char id to assign
	int char_id = ( cd->char_id != -1 ) ? cd->char_id : db->next_char_id;

	// check if the char_id is free
	tmp = idb_get(chars, char_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("char_db_txt_create: cannot create character %d:'%s', this id is already occupied by %d:'%s'!\n", char_id, cd->name, char_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	tmp = (struct mmo_charstatus*)aMalloc(sizeof(struct mmo_charstatus));
	memcpy(tmp, cd, sizeof(struct mmo_charstatus));
	tmp->char_id = char_id;
	idb_put(chars, char_id, tmp);

	// increment the auto_increment value
	if( char_id >= db->next_char_id )
		db->next_char_id = char_id + 1;

	// write output
	cd->char_id = char_id;

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool char_db_txt_remove(CharDB* self, const int char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	idb_remove(chars, char_id);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool char_db_txt_save(CharDB* self, const struct mmo_charstatus* ch)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	int char_id = ch->char_id;

	// retrieve previous data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, ch, sizeof(struct mmo_charstatus));

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// @protected
static bool char_db_txt_load_num(CharDB* self, struct mmo_charstatus* ch, int char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	RankDB* rankdb = db->owner->rankdb;
	DBMap* chars = db->chars;

	// retrieve data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(ch, tmp, sizeof(struct mmo_charstatus));

	return true;
}


/// @protected
static bool char_db_txt_load_str(CharDB* self, struct mmo_charstatus* ch, const char* name)
{
//	CharDB_TXT* db = (CharDB_TXT*)self;
	int char_id;

	// find char id
	if( !self->name2id(self, name, &char_id, NULL) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}


/// @protected
static bool char_db_txt_load_slot(CharDB* self, struct mmo_charstatus* ch, int account_id, int slot)
{
//	CharDB_TXT* db = (CharDB_TXT*)self;
	int char_id;

	// find char id
	if( !self->slot2id(self, account_id, slot, &char_id) )
	{// entry not found
		return false;
	}

	// retrieve data
	return self->load_num(self, ch, char_id);
}


/// @protected
static bool char_db_txt_id2name(CharDB* self, int char_id, char* name, size_t size)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct mmo_charstatus* tmp = idb_get(chars, char_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( name != NULL )
		safestrncpy(name, tmp->name, size);
	
	return true;
}


/// @protected
static bool char_db_txt_name2id(CharDB* self, const char* name, int* char_id, int* account_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct DBIterator* iter = chars->iterator(chars);
	struct mmo_charstatus* tmp;
	int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

	for( tmp = (struct mmo_charstatus*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_charstatus*)iter->next(iter,NULL) )
		if( compare(name, tmp->name) == 0 )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( char_id != NULL )
		*char_id = tmp->char_id;
	if( account_id != NULL )
		*account_id = tmp->account_id;

	return true;
}


/// @protected
static bool char_db_txt_slot2id(CharDB* self, int account_id, int slot, int* char_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;

	// retrieve data
	struct DBIterator* iter = chars->iterator(chars);
	struct mmo_charstatus* tmp;

	for( tmp = (struct mmo_charstatus*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct mmo_charstatus*)iter->next(iter,NULL) )
		if( account_id == tmp->account_id && slot == tmp->slot )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	if( char_id != NULL )
		*char_id = tmp->char_id;

	return true;
}


/// Returns an iterator over all the characters.
/// @protected
static CSDBIterator* char_db_txt_iterator(CharDB* self)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->chars));
}


/// internal structure
/// @private
typedef struct CharDBIterator_TXT
{
	CSDBIterator vtable;      // public interface

	DBIterator* iter;
	int account_id;

} CharDBIterator_TXT;


/// Destroys this iterator, releasing all allocated memory (including itself).
/// @protected
static void char_db_txt_iter_destroy(CSDBIterator* self)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	dbi_destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next character.
/// @protected
static bool char_db_txt_iter_next(CSDBIterator* self, int* key)
{
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)self;
	struct mmo_charstatus* tmp;

	while( true )
	{
		tmp = (struct mmo_charstatus*)dbi_next(iter->iter);
		if( tmp == NULL )
			return false;// not found

		if( iter->account_id != tmp->account_id )
			continue;// wrong account, try next

		if( key )
			*key = tmp->char_id;
		return true;
	}
}


/// Returns an iterator over all the characters of the account.
/// @protected
static CSDBIterator* char_db_txt_characters(CharDB* self, int account_id)
{
	CharDB_TXT* db = (CharDB_TXT*)self;
	DBMap* chars = db->chars;
	CharDBIterator_TXT* iter = (CharDBIterator_TXT*)aCalloc(1, sizeof(CharDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &char_db_txt_iter_destroy;
	iter->vtable.next    = &char_db_txt_iter_next;

	// fill data
	iter->iter = db_iterator(chars);
	iter->account_id = account_id;

	return &iter->vtable;
}


/// Constructs a new CharDB interface.
/// @protected
CharDB* char_db_txt(CharServerDB_TXT* owner)
{
	CharDB_TXT* db = (CharDB_TXT*)aCalloc(1, sizeof(CharDB_TXT));

	// set up the vtable
	db->vtable.p.init      = &char_db_txt_init;
	db->vtable.p.destroy   = &char_db_txt_destroy;
	db->vtable.p.sync      = &char_db_txt_sync;
	db->vtable.create    = &char_db_txt_create;
	db->vtable.remove    = &char_db_txt_remove;
	db->vtable.save      = &char_db_txt_save;
	db->vtable.load_num  = &char_db_txt_load_num;
	db->vtable.load_str  = &char_db_txt_load_str;
	db->vtable.load_slot = &char_db_txt_load_slot;
	db->vtable.id2name   = &char_db_txt_id2name;
	db->vtable.name2id   = &char_db_txt_name2id;
	db->vtable.slot2id   = &char_db_txt_slot2id;
	db->vtable.iterator  = &char_db_txt_iterator;
	db->vtable.characters = &char_db_txt_characters;

	// initialize to default values
	db->owner = owner;
	db->chars = NULL;
	db->next_char_id = START_CHAR_NUM;
	db->dirty = false;

	// other settings
	db->char_db = db->owner->file_chars;
	db->case_sensitive = false;

	return &db->vtable;
}
