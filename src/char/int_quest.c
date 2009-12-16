// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "questdb.h"
#include "int_quest.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// quest database
static QuestDB* quests = NULL;


// Send questlog to map server
static void mapif_parse_quests_load(int fd)
{
	int char_id = RFIFOL(fd,2);
	int count = quests->count(quests, char_id);

	WFIFOHEAD(fd,count*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = count*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;
	quests->load(quests, (struct quest*)WFIFOP(fd,8), count, char_id);
	WFIFOSET(fd,WFIFOW(fd,2));
}


//Save quests
static void mapif_parse_quests_save(int fd)
{
	int size = RFIFOW(fd,2)-8;
	int char_id = RFIFOL(fd,4);
	const struct quest* data = (const struct quest*)RFIFOP(fd,8);
	int count = size / sizeof(struct quest);
	bool success;

	success = quests->save(quests, data, count, char_id);
	
	// send back result
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = success ? 1 : 0;
	WFIFOSET(fd,7);
}


int inter_quest_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3060: mapif_parse_quests_load(fd); break;
		case 0x3061: mapif_parse_quests_save(fd); break;
		default:
			return 0;
	}
	return 1;
}

void inter_quest_init(QuestDB* db)
{
	quests = db;
}

void inter_quest_final(void)
{
	quests = NULL;
}
