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


//Send questlog to map server
static void mapif_send_quests(int fd, int char_id)
{

	questlog tmp_questlog;
	int num_quests, i;

	memset(tmp_questlog, 0, sizeof(tmp_questlog));

	if( !quests->load(quests, &tmp_questlog, char_id, &num_quests) )
		return;

	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	for( i = 0; i < num_quests; i++ )
		memcpy(WFIFOP(fd,i*sizeof(struct quest)+8), &tmp_questlog[i], sizeof(struct quest));

	WFIFOSET(fd,num_quests*sizeof(struct quest)+8);
}


static void mapif_quest_added(int fd, int char_id, int quest_id, bool success)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = quest_id;
	WFIFOB(fd,10) = success ? 1 : 0;
	WFIFOSET(fd,11);
}

static void mapif_quest_deleted(int fd, int char_id, int quest_id, bool success)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3862;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = quest_id;
	WFIFOB(fd,10) = success ? 1 : 0;
	WFIFOSET(fd,11);
}


//Map server requesting a character's quest log
static void mapif_parse_loadquestrequest(int fd)
{
	int char_id = RFIFOL(fd,2);
	mapif_send_quests(fd, char_id);
}

//Add a quest to a questlog
static void mapif_parse_quest_add(int fd)
{
	int len = RFIFOW(fd,2);
	int char_id = RFIFOL(fd,4);
	const struct quest* qd = (struct quest*)RFIFOP(fd,8);
	bool success;

	if( len - 8 != sizeof(struct quest) )
	{
		//TODO: error message
		return;
	}

	success = quests->add(quests, qd, char_id);
	mapif_quest_added(fd, char_id, qd->quest_id, success);
}

//Delete a quest
void mapif_parse_quest_delete(int fd)
{
	int char_id = RFIFOL(fd,2);
	int quest_id = RFIFOL(fd,6);
	bool success;

	success = quests->del(quests, char_id, quest_id);
	mapif_quest_deleted(fd, char_id, quest_id, success);
}


int inter_quest_parse_frommap(int fd)
{

	switch(RFIFOW(fd,0))
	{
		case 0x3060: mapif_parse_loadquestrequest(fd); break;
		case 0x3061: mapif_parse_quest_add(fd); break;
		case 0x3062: mapif_parse_quest_delete(fd); break;
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
