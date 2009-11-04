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
	questlog tmp_questlog;
	int num_quests, num_complete;
	int complete[MAX_QUEST_DB]; // indexes of completed quests
	int i;

	memset(tmp_questlog, 0, sizeof(tmp_questlog));
	memset(complete, 0, sizeof(complete));

	if( !quests->load(quests, &tmp_questlog, char_id, &num_quests) )
		return;

	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	//Active and inactive quests
	num_complete = 0;
	for( i = 0; i < num_quests; i++ )
	{
		if( tmp_questlog[i].state == Q_COMPLETE )
		{
			complete[num_complete++] = i;
			continue;
		}
		memcpy(WFIFOP(fd,(i-num_complete)*sizeof(struct quest)+8), &tmp_questlog[i], sizeof(struct quest));
	}

	// Completed quests
	for( i = num_quests - num_complete; i < num_quests; i++ )
		memcpy(WFIFOP(fd,i*sizeof(struct quest)+8), &tmp_questlog[complete[i-num_quests+num_complete]], sizeof(struct quest));

	WFIFOSET(fd,WFIFOW(fd,2));
}


//Save quests
static void mapif_parse_quests_save(int fd)
{
	int char_id = RFIFOL(fd,4);
	int size = RFIFOW(fd,2)-8;
	const struct quest* data = (const struct quest*)RFIFOP(fd,8);
	int count = size / sizeof(struct quest);
	questlog tmp_questlog;
	bool success;

	memcpy(&tmp_questlog[0], data, size);
	memset(&tmp_questlog[count], 0, sizeof(tmp_questlog) - size);

	success = quests->save(quests, &tmp_questlog, char_id);
	
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
