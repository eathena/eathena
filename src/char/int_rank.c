// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "char.h" // mapif_sendall mapif_send
#include "rankdb.h"
#include "int_rank.h"
#include <stdlib.h>
#include <string.h>



static RankDB* rankdb = NULL;
static bool rankdb_enabled = true;

// Maximum number of top rankers.
static int blacksmith_top_rankers_size = MAX_FAME_LIST;
static int alchemist_top_rankers_size = MAX_FAME_LIST;
static int taekwon_top_rankers_size = MAX_FAME_LIST;

// Cache of top rankers.
static struct fame_list blacksmith_top_rankers[MAX_FAME_LIST];
static struct fame_list alchemist_top_rankers[MAX_FAME_LIST];
static struct fame_list taekwon_top_rankers[MAX_FAME_LIST];



/// Provides access to the selected top rankers list.
/// @param rank_id Rank list id
/// @param list address of the chosen list
/// @param size the number of entries in the chosen list
static bool get_top_rankers_list(int rank_id, struct fame_list** list, int* size)
{
	switch( rank_id )
	{
		case RANK_BLACKSMITH: *size = blacksmith_top_rankers_size; *list = blacksmith_top_rankers; break;
		case RANK_ALCHEMIST:  *size = alchemist_top_rankers_size;  *list = alchemist_top_rankers;  break;
		case RANK_TAEKWON:    *size = taekwon_top_rankers_size;    *list = taekwon_top_rankers;    break;
		default:              *size = 0;                           *list = NULL;                   break;
	}
	return *list != NULL ;
}



/// Places the selected fame list into the provided buffer.
/// @return number of bytes written
static int top_rankers_tobuf(uint8* buf, int rank_id)
{
	struct fame_list* list;
	int count;
	int len = 0;
	int i;

	if( rankdb == NULL || !rankdb_enabled )
		return 0;// disabled

	get_top_rankers_list(rank_id, &list, &count);

	for( i = 0; i < count && list[i].id != 0; i++ )
	{
		memcpy(buf + len, &list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}

	return len;
}



// Sends the top rankers to the map-server(s).
// S 3805 <len>.w <bs len>.w <alch len>.w <tk len>.w <bs data>.?b<alch data>.?b <tk data>.?b
static int mapif_top_rankers(int fd)
{
	unsigned char buf[10 + 3*MAX_FAME_LIST*sizeof(struct fame_list)];
	int len = 10;

	WBUFW(buf,0) = 0x3805;
	len += WBUFW(buf,4) = top_rankers_tobuf(WBUFP(buf,len), RANK_BLACKSMITH);
	len += WBUFW(buf,6) = top_rankers_tobuf(WBUFP(buf,len), RANK_ALCHEMIST);
	len += WBUFW(buf,8) = top_rankers_tobuf(WBUFP(buf,len), RANK_TAEKWON);
	WBUFW(buf,2) = len;

	if( fd == -1 )
		mapif_sendall(buf, len);
	else
		mapif_send(fd, buf, len);
	return 0;
}


/// Updates the selected top ranker cache(s).
/// Broadcasts to all map-servers if something changed.
/// @param rank_id Rank id or -1 for all
static void update_top_rankers(int rank_id)
{
	bool modified = false;

	if( rankdb == NULL || !rankdb_enabled )
		return;// disabled

	if( rank_id == -1 )
	{// update all
		struct fame_list list[MAX_FAME_LIST];
		int count;

		// blacksmith
		memset(&list, 0, sizeof(list));
		count = rankdb->get_top_rankers(rankdb, RANK_BLACKSMITH, list, blacksmith_top_rankers_size);
		if( memcmp(blacksmith_top_rankers, list, sizeof(list)) != 0 )
		{
			modified = true;
			memcpy(blacksmith_top_rankers, list, sizeof(list));
		}
		// alchemist
		memset(&list, 0, sizeof(list));
		count = rankdb->get_top_rankers(rankdb, RANK_ALCHEMIST, list, alchemist_top_rankers_size);
		if( memcmp(alchemist_top_rankers, list, sizeof(list)) != 0 )
		{
			modified = true;
			memcpy(alchemist_top_rankers, list, sizeof(list));
		}
		// taekwon
		memset(&list, 0, sizeof(list));
		count = rankdb->get_top_rankers(rankdb, RANK_TAEKWON, list, taekwon_top_rankers_size);
		if( memcmp(taekwon_top_rankers, list, sizeof(list)) != 0 )
		{
			modified = true;
			memcpy(taekwon_top_rankers, list, sizeof(list));
		}
	}
	else
	{// refresh a specific ranking
		struct fame_list list[MAX_FAME_LIST];
		struct fame_list* dest;
		int count;

		if( !get_top_rankers_list(rank_id, &dest, &count) )
			return;
		memset(&list, 0, sizeof(list));
		count = rankdb->get_top_rankers(rankdb, rank_id, list, taekwon_top_rankers_size);
		if( memcmp(dest, list, sizeof(list)) != 0 )
		{
			modified = true;
			memcpy(dest, list, sizeof(list));
		}
	}

	if( modified )
		mapif_top_rankers(-1);
}


/// Alters the points of character char_id in ranking rank_id.
static void update_rank_points(int rank_id, int char_id, int points)
{
	struct fame_list* list;
	int count;
	int i;

	if( rankdb == NULL || !rankdb_enabled )
		return;// disabled

	rankdb->set_points(rankdb, rank_id, char_id, points);
	if( !get_top_rankers_list(rank_id, &list, &count) )
		return;
	ARR_FIND(0, count, i, char_id == list[i].id || points > list[i].fame);
	if( i < count )
		update_top_rankers(rank_id);
}

// update a char's fame and send back an updated fame list
static void mapif_parse_RankChange(int fd)
{
	int char_id = RFIFOL(fd,2);
	int points = RFIFOL(fd,6);
	int rank_id = RFIFOB(fd,10);

	update_rank_points(rank_id, char_id, points);
}

// Build and send fame ranking lists
static void mapif_parse_RankListRequest(int fd)
{
	mapif_top_rankers(fd);
}



bool rank_config_read(const char* key, const char* value)
{
	const char* signature;

	signature = "rankdb.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "enabled") == 0 )
			rankdb_enabled = config_switch(value);
		else
		if( strcmpi(key, "blacksmith_top_rankers") == 0 )
		{
			blacksmith_top_rankers_size = atoi(value);
			if( blacksmith_top_rankers_size < 0 || blacksmith_top_rankers_size > MAX_FAME_LIST )
			{
				int v = cap_value(blacksmith_top_rankers_size, 0, MAX_FAME_LIST);
				ShowWarning("rank_config_read: capping 'rankdb.blacksmith_top_rankers' from %d to %d.\n", blacksmith_top_rankers_size, v);
				blacksmith_top_rankers_size = v;
			}
		}
		else
		if( strcmpi(key, "alchemist_top_rankers") == 0 )
		{
			alchemist_top_rankers_size = atoi(value);
			if( alchemist_top_rankers_size < 0 || alchemist_top_rankers_size > MAX_FAME_LIST )
			{
				int v = cap_value(alchemist_top_rankers_size, 0, MAX_FAME_LIST);
				ShowWarning("rank_config_read: capping 'rankdb.alchemist_top_rankers' from %d to %d.\n", alchemist_top_rankers_size, v);
				alchemist_top_rankers_size = v;
			}
		}
		else
		if(	strcmpi(key, "taekwon_top_rankers") == 0 )
		{
			taekwon_top_rankers_size = atoi(value);
			if( taekwon_top_rankers_size < 0 || taekwon_top_rankers_size > MAX_FAME_LIST )
			{
				int v = cap_value(taekwon_top_rankers_size, 0, MAX_FAME_LIST);
				ShowWarning("rank_config_read: capping 'rankdb.taekwon_top_rankers' from %d to %d.\n", taekwon_top_rankers_size, v);
				taekwon_top_rankers_size = v;
			}
		}
		else
			return false;// not found
		return true;
	}

	return false;// not found
}



int inter_rank_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x30B0: mapif_parse_RankChange(fd); break;
	case 0x30B1: mapif_parse_RankListRequest(fd); break;
	// TODO deprecate  RankChange <char_id>.L <total_points>.L <rank_id>.B
	// TODO implement(map->inter)  AwardPoints      <rank_id>.L <char_id>.L <points>.L
	// TODO implement(inter->map)  AwardPointsReply <rank_id>.L <char_id>.L <points>.L <total_points>.L
	default:
		return 0;
	}
	return 1;
}



void inter_rank_init(RankDB* db)
{
	rankdb = db;// can be NULL
	memset(blacksmith_top_rankers, 0, sizeof(blacksmith_top_rankers));
	memset(alchemist_top_rankers, 0, sizeof(alchemist_top_rankers));
	memset(taekwon_top_rankers, 0, sizeof(taekwon_top_rankers));
	update_top_rankers(-1);
}



void inter_rank_final(void)
{
	rankdb = NULL;
}



enum rank_type inter_rank_class2rankid(int class_)
{
	enum rank_type result;

	switch( class_ )
	{
	case JOB_BLACKSMITH:
	case JOB_WHITESMITH:
	case JOB_BABY_BLACKSMITH:
		result = RANK_BLACKSMITH;
		break;
	case JOB_ALCHEMIST:
	case JOB_CREATOR:
	case JOB_BABY_ALCHEMIST:
		result = RANK_ALCHEMIST;
		break;
	case JOB_TAEKWON:
		result = RANK_TAEKWON;
		break;
	default:
		result = (enum rank_type)0;
	}

	return result;
}
