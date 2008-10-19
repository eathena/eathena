// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "char.h"
#include "chardb.h"
#include "int_fame.h"
#include <stdlib.h>
#include <string.h>

// temporary imports
extern CharDB* chars;
extern char unknown_char_name[NAME_LENGTH];
void char_read_fame_list(void);


//Custom limits for the fame lists. [Skotlex]
int fame_list_size_chemist = MAX_FAME_LIST;
int fame_list_size_smith = MAX_FAME_LIST;
int fame_list_size_taekwon = MAX_FAME_LIST;

// Char-server-side stored fame lists [DracoRPG]
struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

/// Provides access to the selected fame list.
/// @param type the type of fame list
/// @param list address of the chosen list
/// @param size the number of entries in the chosen list
///TODO: perhaps make it read-only?
bool get_fame_list(enum fame_type type, struct fame_list** list, int* size)
{
	switch( type )
	{
		case FAME_SMITH  : *size = fame_list_size_smith;   *list = smith_fame_list;   break;
		case FAME_CHEMIST: *size = fame_list_size_chemist; *list = chemist_fame_list; break;
		case FAME_TAEKWON: *size = fame_list_size_taekwon; *list = taekwon_fame_list; break;
		default          : *size = 0;                      *list = NULL;              break;
	}

	return( list != NULL );
}

/// Alters fame of a single character and updates ranking list.
/// @return true if anything changed, false otherwise
bool fame_list_update(enum fame_type type, int charid, int fame)
{
	int player_pos, fame_pos;
	struct fame_list* list;
	int size;

	get_fame_list(type, &list, &size);

	//TODO: move code to int_fame

	ARR_FIND(0, size, player_pos, list[player_pos].id == charid);// position of the player
	ARR_FIND(0, size, fame_pos, list[fame_pos].fame <= fame);// where the player should be

	if( player_pos == size && fame_pos == size )
		return false; // not on list and not enough fame to get on it

	if( fame_pos == player_pos )
	{// same position
		list[player_pos].fame = fame;
	}
	else
	if( player_pos == size )
	{// new ranker - not in the list
		ARR_MOVE(size - 1, fame_pos, list, struct fame_list);
		list[fame_pos].id = charid;
		list[fame_pos].fame = fame;
		if( !chars->id2name(chars, charid, list[fame_pos].name) )
			safestrncpy(list[fame_pos].name, unknown_char_name, NAME_LENGTH);
	}
	else
	{// already in the list
		if( fame_pos == size )
			--fame_pos;// move to the end of the list
		ARR_MOVE(player_pos, fame_pos, list, struct fame_list);
		list[fame_pos].fame = fame;
	}

	return true;
}

/// Places the selected fame list into the provided buffer.
/// @return number of bytes written
int fame_list_tobuf(uint8* buf, enum fame_type type)
{
	struct fame_list* list;
	int size;
	int len = 0;
	int i;

	get_fame_list(type, &list, &size);

	for( i = 0; i < size && list[i].id != 0; i++ )
	{
		memcpy(buf + len, &list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}

	return len;
}

bool fame_config_read(const char* key, const char* value)
{
	if( strcmpi(key, "fame_list_alchemist") == 0 )
	{
		fame_list_size_chemist = atoi(value);
		if (fame_list_size_chemist > MAX_FAME_LIST) {
			ShowWarning("Max fame list size is %d (fame_list_alchemist)\n", MAX_FAME_LIST);
			fame_list_size_chemist = MAX_FAME_LIST;
		}
	}
	else
	if( strcmpi(key, "fame_list_blacksmith") == 0 )
	{
		fame_list_size_smith = atoi(value);
		if (fame_list_size_smith > MAX_FAME_LIST) {
			ShowWarning("Max fame list size is %d (fame_list_blacksmith)\n", MAX_FAME_LIST);
			fame_list_size_smith = MAX_FAME_LIST;
		}
	}
	else
	if( strcmpi(key, "fame_list_taekwon") == 0 )
	{
		fame_list_size_taekwon = atoi(value);
		if (fame_list_size_taekwon > MAX_FAME_LIST) {
			ShowWarning("Max fame list size is %d (fame_list_taekwon)\n", MAX_FAME_LIST);
			fame_list_size_taekwon = MAX_FAME_LIST;
		}
	}
	else
		return false;

	return true;
}


// Send the fame ranking lists to map-server(s)
// S 3805 <len>.w <bs len>.w <alch len>.w <tk len>.w <bs data>.?b<alch data>.?b <tk data>.?b
int char_send_fame_list(int fd)
{
	unsigned char buf[3 * MAX_FAME_LIST * sizeof(struct fame_list)];
	int len = 10;
	
	WBUFW(buf,0) = 0x3805;
	len += WBUFW(buf,4) = fame_list_tobuf(WBUFP(buf,len), FAME_SMITH);
	len += WBUFW(buf,6) = fame_list_tobuf(WBUFP(buf,len), FAME_CHEMIST);
	len += WBUFW(buf,8) = fame_list_tobuf(WBUFP(buf,len), FAME_TAEKWON);
	WBUFW(buf,2) = len;

	if( fd != -1 )
		mapif_send(fd, buf, len);
	else
		mapif_sendall(buf, len);

	return 0;
}


// update a char's fame and send back an updated fame list
void mapif_parse_FameChange(int fd)
{
	int cid = RFIFOL(fd,2);
	int fame = RFIFOL(fd,6);
	enum fame_type type = (enum fame_type)RFIFOB(fd,10);

	if( fame_list_update(type, cid, fame) )
		char_send_fame_list(-1);
}

// Build and send fame ranking lists
void mapif_parse_FameListRequest(int fd)
{
	char_read_fame_list(); // required since there's no real sync with the CharDB
	char_send_fame_list(fd);
}


int inter_fame_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
	case 0x30B0: mapif_parse_FameChange(fd); break;
	case 0x30B1: mapif_parse_FameListRequest(fd); break;
	default:
		return 0;
	}
	return 1;
}

void inter_fame_init(void)
{
	char_read_fame_list();
}

void inter_fame_final(void)
{

}
