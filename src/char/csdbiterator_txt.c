// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/malloc.h"
#include "csdbiterator.h"


/// generic txt db iterator
typedef struct CSDBIterator_TXT
{
	CSDBIterator vtable;

	DBIterator* iter;
}
CSDBIterator_TXT;


/// Destroys this iterator, releasing all allocated memory (including itself).
static void csdb_txt_iter_destroy(CSDBIterator* self)
{
	CSDBIterator_TXT* iter = (CSDBIterator_TXT*)self;
	dbi_destroy(iter->iter);
	aFree(iter);
}


/// Fetches the next entry's key.
static bool csdb_txt_iter_next(CSDBIterator* self, int* key)
{
	CSDBIterator_TXT* iter = (CSDBIterator_TXT*)self;
	void* data;
	DBKey k;

	data = iter->iter->next(iter->iter, &k);
	if( data == NULL )
		return false;// not found

	if( key )
		*key = k.i;

	return true;
}


/// generic txt db iterator constructor
CSDBIterator* csdb_txt_iterator(DBIterator* db_iterator)
{
	struct CSDBIterator_TXT* iter = (CSDBIterator_TXT*)aCalloc(1, sizeof(CSDBIterator_TXT));

	// set up the vtable
	iter->vtable.destroy = &csdb_txt_iter_destroy;
	iter->vtable.next    = &csdb_txt_iter_next;
	
	iter->iter = db_iterator;

	return &iter->vtable;
}
