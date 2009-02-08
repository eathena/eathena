// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"

#include <string.h>


typedef struct RankDB_TXT RankDB_TXT;



/// private
struct RankDB_TXT
{
	RankDB vtable;

	CharServerDB_TXT* owner;
};



static bool rankid_includes_class(int rank_id, int class_)
{
	switch( rank_id )
	{
	case RANK_BLACKSMITH:
		return (class_ == JOB_BLACKSMITH || class_ == JOB_WHITESMITH || class_ == JOB_BABY_BLACKSMITH);
	case RANK_ALCHEMIST:
		return (class_ == JOB_ALCHEMIST || class_ == JOB_CREATOR || class_ == JOB_BABY_ALCHEMIST);
	case RANK_TAEKWON:
		return class_ == JOB_TAEKWON;
	default:
		return false;
	}
}



/// Gets the top rankers in rank rank_id.
static int rank_db_txt_get_top_rankers(RankDB* self, int rank_id, struct fame_list* list, int count)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	CharDB* chardb = db->owner->chardb;
	CharDBIterator* iter;
	struct mmo_charstatus cd;
	int i;
	int n = 0;
	int lowest = INT_MAX;

	if( list == NULL || count <= 0 )
		return 0;// nothing to do
	memset(list, 0, count*sizeof(list[0]));
	if( rank_id != RANK_BLACKSMITH && rank_id != RANK_ALCHEMIST && rank_id != RANK_TAEKWON )
	{
		ShowError("rank_db_txt_get_top_rankers: unsupported rank_id %d.\n", rank_id);
		return 0;
	}

	// XXX expensive
	iter = chardb->iterator(chardb);
	while( iter->next(iter, &cd) )
	{
		if( !rankid_includes_class(rank_id, cd.class_) )
			continue;// wrong job
		if( n == count && cd.fame <= lowest )
			continue;// not enough fame to enter the list

		if( lowest > cd.fame )
			lowest = cd.fame;

		ARR_FIND( 0, n, i, cd.fame > list[i].fame );
		if( n < count )
		{
			memmove(list+i+1, list+i, (n-i)*sizeof(list[0]));// new
			++n;
		}
		else
			memmove(list+i+1, list+i, (n-i-1)*sizeof(list[0]));// replace in list
		list[i].id = cd.char_id;
		list[i].fame = cd.fame;
		safestrncpy(list[i].name, cd.name, sizeof(list[i].name));
	}
	iter->destroy(iter);
	return n;
}



/// Returns the number of points character char_id has in rank rank_id.
/// Returns 0 if not found.
static int rank_db_txt_get_points(RankDB* self, int rank_id, int char_id)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	CharDB* chardb = db->owner->chardb;
	struct mmo_charstatus cd;

	if( chardb->load_num(chardb, &cd, char_id) && rankid_includes_class(rank_id, cd.class_) )
		return cd.fame;
	return 0;
}



/// Sets the number of points character char_id has in rank rank_id.
static void rank_db_txt_set_points(RankDB* self, int rank_id, int char_id, int points)
{
	RankDB_TXT* db = (RankDB_TXT*)self;
	CharDB* chardb = db->owner->chardb;
	struct mmo_charstatus cd;

	if( chardb->load_num(chardb, &cd, char_id) && rankid_includes_class(rank_id, cd.class_) )
	{
		cd.fame = points;
		chardb->save(chardb, &cd);
	}
}



/// Constructs a new RankDB interface.
/// @protected
RankDB* rank_db_txt(CharServerDB_TXT* owner)
{
	RankDB_TXT* db;

	CREATE(db, RankDB_TXT, 1);
	db->vtable.get_top_rankers = rank_db_txt_get_top_rankers;
	db->vtable.get_points      = rank_db_txt_get_points;
	db->vtable.set_points      = rank_db_txt_set_points;

	db->owner = owner;
	return &db->vtable;
}



/// Initializes this RankDB interface.
/// @protected
bool rank_db_txt_init(RankDB* self)
{
	RankDB_TXT* db = (RankDB_TXT*)self;

	return true;
}



/// Destroys this RankDB interface.
/// @protected
void rank_db_txt_destroy(RankDB* self)
{
	RankDB_TXT* db = (RankDB_TXT*)self;

	db->owner = NULL;
	aFree(db);
}
