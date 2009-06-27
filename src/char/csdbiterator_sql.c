// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/sql.h"
#include "csdbiterator.h"
#include <stdlib.h>


/// Maximum number of keys cached in the iterator.
#define CSDBITERATOR_MAXCACHE 16000


/// generic sql db iterator
typedef struct CSDBIterator_SQL
{
	CSDBIterator vtable;

	Sql* sql_handle;
	const char* sql_table;
	const char* sql_column;
	int* ids_arr;
	int ids_num;
	int pos;
	bool has_more;
}
CSDBIterator_SQL;


/// Private. Fills the cache of the iterator with keys.
static void csdb_sql_iter_P_fillcache(CSDBIterator_SQL* iter)
{
	int res;
	int last_id = 0;
	bool has_last_id = false;

	if( iter->ids_num > 0 )
	{
		last_id = iter->ids_arr[iter->ids_num-1];
		has_last_id = true;
	}

	if( has_last_id )
		res = Sql_Query(iter->sql_handle, "SELECT DISTINCT `%s` FROM `%s` WHERE `%s`>%d ORDER BY `%s` ASC LIMIT %d", iter->sql_column, iter->sql_table, iter->sql_column, last_id, iter->sql_column, CSDBITERATOR_MAXCACHE+1);
	else
		res = Sql_Query(iter->sql_handle, "SELECT DISTINCT `%s` FROM `%s` ORDER BY `%s` ASC LIMIT %d", iter->sql_column, iter->sql_table, iter->sql_column, CSDBITERATOR_MAXCACHE+1);
	if( res == SQL_ERROR )
	{
		Sql_ShowDebug(iter->sql_handle);
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else if( Sql_NumRows(iter->sql_handle) == 0 )
	{
		iter->ids_num = 0;
		iter->pos = -1;
		iter->has_more = false;
	}
	else
	{
		int i;

		if( Sql_NumRows(iter->sql_handle) > CSDBITERATOR_MAXCACHE )
		{
			iter->has_more = true;
			iter->ids_num = CSDBITERATOR_MAXCACHE;
		}
		else
		{
			iter->has_more = false;
			iter->ids_num = (int)Sql_NumRows(iter->sql_handle);
		}
		if( has_last_id )
		{
			++iter->ids_num;
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->ids_arr[0] = last_id;
			iter->pos = 0;
			i = 1;
		}
		else
		{
			RECREATE(iter->ids_arr, int, iter->ids_num);
			iter->pos = -1;
			i = 0;
		}

		while( i < iter->ids_num )
		{
			char* data;
			int res = Sql_NextRow(iter->sql_handle);
			if( res == SQL_SUCCESS )
				res = Sql_GetData(iter->sql_handle, 0, &data, NULL);
			if( res == SQL_ERROR )
				Sql_ShowDebug(iter->sql_handle);
			if( res != SQL_SUCCESS )
				break;

			if( data == NULL )
				continue;

			iter->ids_arr[i] = atoi(data);
			++i;
		}
		iter->ids_num = i;
	}
	Sql_FreeResult(iter->sql_handle);
}


/// Destroys this iterator, releasing all allocated memory (including itself).
static void csdb_sql_iter_destroy(CSDBIterator* self)
{
	CSDBIterator_SQL* iter = (CSDBIterator_SQL*)self;
	if( iter->ids_arr )
		aFree(iter->ids_arr);
	aFree(iter);
}


/// Fetches the next entry's key.
static bool csdb_sql_iter_next(CSDBIterator* self, int* key)
{
	CSDBIterator_SQL* iter = (CSDBIterator_SQL*)self;
	Sql* sql_handle = iter->sql_handle;

	while( iter->pos+1 >= iter->ids_num )
	{
		if( !iter->has_more )
			return false;
		csdb_sql_iter_P_fillcache(iter);
	}

	++iter->pos;
	if( key )
		*key = iter->ids_arr[iter->pos];

	return true;
}


/// generic sql db iterator constructor
CSDBIterator* csdb_sql_iterator(Sql* sql_handle, const char* sql_table, const char* sql_column)
{
	struct CSDBIterator_SQL* iter = (CSDBIterator_SQL*)aCalloc(1, sizeof(CSDBIterator_SQL));

	// set up the vtable
	iter->vtable.destroy = &csdb_sql_iter_destroy;
	iter->vtable.next    = &csdb_sql_iter_next;
	
	iter->sql_handle = sql_handle;
	iter->sql_table = sql_table;
	iter->sql_column = sql_column;
	iter->ids_arr = NULL;
	iter->ids_num = 0;
	iter->pos = -1;
	iter->has_more = true;// auto load on next

	return &iter->vtable;
}