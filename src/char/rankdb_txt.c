// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>


typedef struct RankDB_TXT RankDB_TXT;


#define RANKDB_TXT_VERSION 20090210



/// private
struct RankDB_TXT
{
	RankDB vtable;
	CharServerDB_TXT* owner;
	const char* file_ranks;
	DBMap* rank_blacksmith;// int char_id -> int points
	DBMap* rank_alchemist;// int char_id -> int points
	DBMap* rank_taekwon;// int char_id -> int points
	bool dirty;
};



/// Returns the DBMap of the target rank_id or NULL if not supported.
/// @private
static DBMap* get_ranking(RankDB_TXT* db, enum rank_type rank_id)
{
	switch( rank_id )
	{
	case RANK_BLACKSMITH: return db->rank_blacksmith;
	case RANK_ALCHEMIST: return db->rank_alchemist;
	case RANK_TAEKWON: return db->rank_taekwon;
	default: return NULL;
	}
}



/// Gets the top rankers in rank rank_id.
static int rank_db_txt_get_top_rankers(RankDB* self, enum rank_type rank_id, struct fame_list* list, int count)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	CharDB* chardb = db->owner->chardb;
	DBMap* ranking = get_ranking(db, rank_id);
	DBIterator* iter;
	int i;
	int n = 0;
	DBKey k;
	int points;

	if( list == NULL || count <= 0 )
		return 0;// nothing to do
	memset(list, 0, count*sizeof(list[0]));
	if( ranking == NULL )
	{
		ShowError("rank_db_txt_get_top_rankers: Unsupported rank_id %d.\n", rank_id);
		return 0;
	}

	// get top rankers
	iter = db_iterator(ranking);
	for( points = (int)(intptr)iter->first(iter, &k); iter->exists(iter); points = (int)(intptr)iter->first(iter, &k) )
	{
		int char_id = k.i;

		if( points <= 0 || (n == count && points <= list[n-1].fame) )
			continue;// not enough points to enter the list

		ARR_FIND( 0, n, i, points > list[i].fame );
		if( n < count )
		{
			memmove(list+i+1, list+i, (n-i)*sizeof(list[0]));// new
			++n;
		}
		else
			memmove(list+i+1, list+i, (n-i-1)*sizeof(list[0]));// replace in list
		list[i].id = char_id;
		list[i].fame = points;
	}
	iter->destroy(iter);

	// resolve names
	for( i = 0; i < n; ++i )
		if( !chardb->id2name(chardb, list[i].id, list[i].name) )
			memset(list[i].name, 0, sizeof(list[i].name));

	return n;
}



/// Returns the number of points character char_id has in rank rank_id.
/// Returns 0 if not found.
static int rank_db_txt_get_points(RankDB* self, enum rank_type rank_id, int char_id)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	DBMap* ranking = get_ranking(db, rank_id);

	if( ranking )
		return (int)(intptr)idb_get(ranking, char_id);
	return 0;
}



/// Sets the number of points character char_id has in rank rank_id.
static void rank_db_txt_set_points(RankDB* self, enum rank_type rank_id, int char_id, int points)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	DBMap* ranking = get_ranking(db, rank_id);

	if( ranking )
	{
		idb_put(ranking, char_id, (void*)(intptr)points);
		db->dirty = true;
		db->owner->p.request_sync(db->owner);
	}
	else
		ShowError("rank_db_txt_set_points: Unsupported rank_id. (rank_id=%d char_id=%d points=%d)\n", rank_id, char_id, points);
}



/// Initializes this RankDB interface.
/// @protected
static bool rank_db_txt_init(RankDB* self)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	FILE* fp;
	char line[2048];
	int linenum = 1;
	int n = 0;
	int version = 0;

	if( db->rank_blacksmith == NULL )
		db->rank_blacksmith = idb_alloc(DB_OPT_ALLOW_NULL_DATA);
	if( db->rank_alchemist == NULL )
		db->rank_alchemist = idb_alloc(DB_OPT_ALLOW_NULL_DATA);
	if( db->rank_taekwon == NULL )
		db->rank_taekwon = idb_alloc(DB_OPT_ALLOW_NULL_DATA);
	db_clear(db->rank_blacksmith);
	db_clear(db->rank_alchemist);
	db_clear(db->rank_taekwon);

	// read ranks file
	fp = fopen(db->file_ranks, "r");
	if( fp == NULL )
	{
		ShowError("rank_db_txt_init: Can't open ranks file '%s' (%s).\n", db->file_ranks, strerror(errno));
		return false;
	}
	// read version (first line)
	if( fgets(line, sizeof(line), fp) == NULL )
	{// ok, empty
		fclose(fp);
		return true;
	}
	if( !(sscanf(line, "%d%n", &version, &n) == 1 && (line[n] == '\n' || line[n] == '\r')) )
	{// error
		ShowError("rank_db_txt_init: Missing version definition in file '%s'.\n", db->file_ranks);
		fclose(fp);
		return false;
	}
	if( version != RANKDB_TXT_VERSION )
	{// error
		ShowError("rank_db_txt_init: Unsupported version %d in file '%s'.\n", version, db->file_ranks);
		fclose(fp);
		return false;
	}
	// read ranks
	while( fgets(line, sizeof(line), fp) != NULL )
	{
		int rank_id, char_id, points;

		++linenum;
		trim(line);
		if( line[0] == '\0' || (line[0] == '/' && line[1] == '/') )
			continue;// empty or line comment

		if( sscanf(line, "%d %d %d", &rank_id, &char_id, &points) == 3 )
		{
			DBMap* ranking = get_ranking(db, (enum rank_type)rank_id);
			if( ranking == NULL )
			{
				ShowWarning("rank_db_txt_init: Unsupported rank_id in line %d of file '%s', discarding rank entry. (rank_id=%d char_id=%d points=%d)", linenum, db->file_ranks, rank_id, char_id, points);
				continue;
			}
			idb_put(ranking, char_id, (void*)(intptr)points);
			continue;
		}
		// error
		ShowError("rank_db_txt_init: Invalid rank format in line %d of file '%s'.", linenum, db->file_ranks);
		fclose(fp);
		return false;
	}
	// ok
	fclose(fp);
	return true;
}



/// Destroys this RankDB interface.
/// @protected
static void rank_db_txt_destroy(RankDB* self)
{
	RankDB_TXT* db = (RankDB_TXT*)self;

	if( db->rank_blacksmith != NULL )
	{
		db_destroy(db->rank_blacksmith);
		db->rank_blacksmith = NULL;
	}
	if( db->rank_alchemist != NULL )
	{
		db_destroy(db->rank_alchemist);
		db->rank_alchemist = NULL;
	}
	if( db->rank_taekwon != NULL )
	{
		db_destroy(db->rank_taekwon);
		db->rank_taekwon = NULL;
	}
	db->owner = NULL;
	aFree(db);
}



/// Saves any pending data.
/// @protected
static bool rank_db_txt_save(RankDB* self)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	FILE* fp;
	int lock;
	enum rank_type ranks[] = {RANK_BLACKSMITH, RANK_ALCHEMIST, RANK_TAEKWON};
	int i;

	if( !db->dirty )
		return true;// nothing to do

	fp = lock_fopen(db->file_ranks, &lock);
	if( fp == NULL )
	{// error
		ShowError("rank_db_txt_save: Failed to create locked file '%s'.\n", db->file_ranks);
		return false;
	}

	fprintf(fp, "%d\n", RANKDB_TXT_VERSION);// format version

	fprintf(fp, "// Ranks file: contains all the ranking information.\n");
	fprintf(fp, "// Structure: rank_id, char_id, points\n");
	fprintf(fp, "//   rank_id : %d=blacksmith, %d=alchemist, %d=taekwon\n", RANK_BLACKSMITH, RANK_ALCHEMIST, RANK_TAEKWON);
	fprintf(fp, "//   char_id : character id\n");
	fprintf(fp, "//   points  : ranking points (also called fame in old code)\n");

	for( i = 0; i < ARRAYLENGTH(ranks); ++i )
	{
		DBKey k;
		int points;
		enum rank_type rank_id = ranks[i];
		DBIterator* iter = db_iterator(get_ranking(db, rank_id));
		for( points = (int)(intptr)iter->first(iter, &k); dbi_exists(iter); points = (int)(intptr)iter->next(iter, &k) )
			fprintf(fp, "%d\t%d\t%d\n", rank_id, k.i, points);
		dbi_destroy(iter);
	}

	if( lock_fclose(fp, db->file_ranks, &lock) )
		return false;// error
	db->dirty = false;
	return true;
}



/// Returns an iterator over all rankings of the specified type.
///
/// @param self Database
/// @param rank_id Rank list id
/// @return Iterator
static CSDBIterator* rank_db_txt_iterator(RankDB* self, enum rank_type rank_id)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(get_ranking(db,rank_id)));
}



/// Constructs a new RankDB interface.
/// @protected
RankDB* rank_db_txt(CharServerDB_TXT* owner)
{
	RankDB_TXT* db;

	CREATE(db, RankDB_TXT, 1);
	db->vtable.init            = rank_db_txt_init;
	db->vtable.destroy         = rank_db_txt_destroy;
	db->vtable.sync            = rank_db_txt_save;
	db->vtable.get_top_rankers = rank_db_txt_get_top_rankers;
	db->vtable.get_points      = rank_db_txt_get_points;
	db->vtable.set_points      = rank_db_txt_set_points;
	db->vtable.iterator        = rank_db_txt_iterator;

	db->owner = owner;
	db->file_ranks = owner->file_ranks;
	db->rank_blacksmith = NULL;
	db->rank_alchemist = NULL;
	db->rank_taekwon = NULL;
	db->dirty = false;
	return &db->vtable;
}
