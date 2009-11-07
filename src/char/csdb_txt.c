// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "charserverdb_txt.h"
#include "csdbiterator.h"
#include "csdb_txt.h"
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // memcpy()


// TODO:
// * update db->max_key inside insert()/replace()


/// Internal structure.
/// @private
typedef struct CSDBimpl_TXT
{
	// public interface
	CSDB_TXT vtable;

	// state
	CharServerDB_TXT* owner;
	DBMap* data;
	int max_key; // highest key value seen so far
	bool dirty;

	// settings
	const char* savefile;
	unsigned int version;
	bool output_newid; // whether to write the %newid% line into the savefile
}
CSDBimpl_TXT;


/// Helper structure to support variable-length data.
/// @private
typedef struct CSDBdata
{
	size_t size;
	unsigned char bytes[1];
}
CSDBdata;


/// Initializes database.
/// Loads data from savefile into memory.
/// @protected
static bool csdb_txt_init(CSDB_TXT* self)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	FILE* fp;
	char line[65536];
	unsigned int lines = 0;
	unsigned int version = 00000000;

	// create dbmap
	if( db->data == NULL )
		db->data = idb_alloc(DB_OPT_RELEASE_DATA);
	db_clear(db->data);

	// open savefile
	fp = fopen(db->savefile, "r");
	if( fp == NULL )
	{
		ShowError("Savefile not found: %s.\n", db->savefile);
		return false;
	}

	// parse savefile
	while( fgets(line, sizeof(line), fp) )
	{
		int key;
		CSDBdata* data;
		size_t size;
		unsigned int v;
		int n;
		bool done = false;

		lines++;

		n = 0;
		if( sscanf(line, "%d%n", &v, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// format version definition
			version = v;
			continue;
		}

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &key, &n) == 1 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( db->max_key < key )
				db->max_key = key - 1;
			continue;
		}

		// parse data
		data = NULL;
		size = 0;
		while( !done )
		{
			void* ptr = ( data != NULL ) ? &data->bytes : NULL;
			size_t out_size;

			key = 0xCCCCCCCC; // debug

			if( !db->vtable.p.fromstr(line, &key, ptr, size, &out_size, version) )
			{// parsing failed
				ShowFatalError("csdb_txt_init: There was a problem processing data in file '%s', line #%d.\nPlease fix manually. Shutting down to prevent data loss.\n", db->savefile, lines);
				if( data )
					aFree(data);
				exit(EXIT_FAILURE);
				return false;
			}

			if( size >= out_size )
				done = true; // parsing completed

			if( size != out_size )
			{// readjust the allocated size (expand if too little, shrink if too much)
				data = (CSDBdata*)aRealloc(data, sizeof(size_t) + out_size);
				if( data == NULL )
				{
					ShowFatalError("csdb_txt_init: Out of memory while processing data in file '%s', line #%d!\n", db->savefile, lines);
					exit(EXIT_FAILURE);
					return false;
				}
			}

			data->size = out_size;
			size = out_size;
		}

		if( key == 0xCCCCCCCC ) // debug
		{
			ShowFatalError("csdb_txt_init: fromstr() forgot to initialize key!\n");
			exit(EXIT_FAILURE);
			return false;
		}
		if( size == 0 ) // suspicious
		{
			size = size;
		}
		
		// record entry in dbmap
		if( idb_put(db->data, key, data) != NULL )
		{
			ShowFatalError("csdb_txt_init: Duplicate entry for key '%d' found in file '%s', line #%d.\nPlease fix manually. Shutting down to prevent data loss.\n", key, db->savefile, lines);
			aFree(data);
			exit(EXIT_FAILURE);
			return false;
		}

		if( db->max_key < key )
			db->max_key = key;
	}

	// close savefile
	fclose(fp);

	db->dirty = false;
	return true;
}


/// Finalizes database.
/// @protected
static bool csdb_txt_destroy(CSDB_TXT* self)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;

	// delete DBMap
	if( db->data != NULL )
	{
		db_destroy(db->data);
		db->data = NULL;
	}

	// delete entire structure
	aFree(db);

	return true;
}


/// Ensures that all data is flushed to secondary storage.
/// @protected
static bool csdb_txt_sync(CSDB_TXT* self, bool force)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	DBIterator* iter;
	DBKey key;
	void* data;
	FILE *fp;
	int lock;

	if( !force && !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->savefile, &lock);
	if( fp == NULL )
	{
		ShowError("csdb_txt_sync: can't write [%s] !!! data is lost !!!\n", db->savefile);
		return false;
	}

	if( db->version != 00000000 )
		fprintf(fp, "%d\n", db->version);

	iter = db->data->iterator(db->data);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
	{
		char line[65536];
		CSDBdata* ptr = (CSDBdata*)data;

		db->vtable.p.tostr(line, key.i, ptr->bytes, ptr->size);
		if( line[0] == '\0' )
			continue;

		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	if( db->output_newid )
		fprintf(fp, "%d\t%%newid%%\n", db->max_key + 1);

	lock_fclose(fp, db->savefile, &lock);

	db->dirty = false;
	return true;
}


/// Checks whether an entry exists in the database.
/// @protected
static bool csdb_txt_exists(CSDB_TXT* self, int key)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	return idb_exists(db->data, key);
}


/// Loads data from db.
/// Fails if no entry with the specified key exists; 'out_size' will be 0.
/// Fails if 'size' is less than required; 'out_size' will hold the required size.
/// @protected
static bool csdb_txt_load(CSDB_TXT* self, int key, void* data, size_t size, size_t* out_size)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	CSDBdata* ptr = idb_get(db->data, key);

	if( out_size != NULL )
		*out_size = 0;

	if( ptr == NULL )
		return false; // does not exist

	if( out_size != NULL )
		*out_size = ptr->size;

	if( size < ptr->size )
		return false;

	memcpy(data, ptr->bytes, ptr->size);

	return true;
}


/// Inserts a new entry.
/// Fails if an entry with the specified key already exists.
/// @protected
static bool csdb_txt_insert(CSDB_TXT* self, int key, const void* data, size_t size)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	CSDBdata* ptr = idb_get(db->data, key);

	if( ptr != NULL )
		return false; // already exists

	ptr = (CSDBdata*)aMalloc(sizeof(size_t) + size);
	if( ptr == NULL )
	{
		ShowFatalError("csdb_txt_insert: out of memory!\n");
		exit(EXIT_FAILURE);
		return false;
	}

	memcpy(ptr->bytes, data, size);
	ptr->size = size;

	idb_put(db->data, key, ptr);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// Updates an existing entry.
/// Fails if no entry with the specified key exists.
/// @protected
static bool csdb_txt_update(CSDB_TXT* self, int key, const void* data, size_t size)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	CSDBdata* ptr = idb_get(db->data, key);

	if( ptr == NULL )
		return false; // does not exist

	if( ptr->size == size )
	{
		memcpy(ptr->bytes, data, size);
	}
	else
	{
		idb_remove(db->data, key);

		ptr = (CSDBdata*)aMalloc(sizeof(size_t) + size);
		if( ptr == NULL )
		{
			ShowFatalError("csdb_txt_save: out of memory!\n");
			exit(EXIT_FAILURE);
			return false;
		}

		memcpy(ptr->bytes, data, size);
		ptr->size = size;

		idb_put(db->data, key, ptr);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// Writes a new entry, replacing any existing one.
/// @protected
static bool csdb_txt_replace(CSDB_TXT* self, int key, const void* data, size_t size)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	CSDBdata* ptr = idb_get(db->data, key);

	if( ptr != NULL && ptr->size == size )
	{
		memcpy(ptr->bytes, data, size);
	}
	else
	{
		idb_remove(db->data, key);

		ptr = (CSDBdata*)aMalloc(sizeof(size_t) + size);
		if( ptr == NULL )
		{
			ShowFatalError("csdb_txt_save: out of memory!\n");
			exit(EXIT_FAILURE);
			return false;
		}

		memcpy(ptr->bytes, data, size);
		ptr->size = size;

		idb_put(db->data, key, ptr);
	}

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// Ensures that no entry with the specified key is present in the db.
/// @protected
static bool csdb_txt_remove(CSDB_TXT* self, int key)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;

	idb_remove(db->data, key);

	db->dirty = true;
	db->owner->p.request_sync(db->owner);
	return true;
}


/// Returns an iterator over all entries in the database.
/// @protected
static DBIterator* csdb_txt_iterator_(CSDB_TXT* self)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	return db_iterator(db->data);
}


/// Returns the next unused key value.
/// @protected
static int csdb_txt_next_key(CSDB_TXT* self)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)self;
	return db->max_key + 1;
}


/// Constructs a new CSDB_TXT interface.
/// @param version Savefile version to use when writing. Using '0' turns off version output.
/// @param start_key Initial key value for new entries. Using '0' turns off %newid% output.
/// @protected
CSDB_TXT* csdb_txt(CharServerDB_TXT* owner, const char* savefile, unsigned int version, const int start_key)
{
	CSDBimpl_TXT* db = (CSDBimpl_TXT*)aCalloc(1, sizeof(CSDBimpl_TXT));

	db->vtable.init      = &csdb_txt_init;
	db->vtable.destroy   = &csdb_txt_destroy;
	db->vtable.sync      = &csdb_txt_sync;
	db->vtable.exists    = &csdb_txt_exists;
	db->vtable.load      = &csdb_txt_load;
	db->vtable.insert    = &csdb_txt_insert;
	db->vtable.update    = &csdb_txt_update;
	db->vtable.replace   = &csdb_txt_replace;
	db->vtable.remove    = &csdb_txt_remove;
	db->vtable.iterator  = &csdb_txt_iterator_;
	db->vtable.next_key  = &csdb_txt_next_key;
	db->vtable.p.fromstr = NULL;
	db->vtable.p.tostr   = NULL;

	db->owner = owner;
	db->savefile = savefile;
	db->version = version;
	db->output_newid = ( start_key != 0 );
	db->data = NULL;
	db->max_key = start_key - 1;
	db->dirty = false;

	return &db->vtable;
}
