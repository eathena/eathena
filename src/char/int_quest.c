// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h" // ARR_FIND()
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
	struct quest qd1[MAX_QUEST_DB]; // new quest log, to be saved
	struct quest qd2[MAX_QUEST_DB]; // previous quest log
	int num1 = (RFIFOW(fd,2)-8)/sizeof(struct quest);
	int num2;
	int buf[MAX_QUEST_DB];
	int count = 0;
	int i, j;

	memset(qd1, 0, sizeof(qd1));
	memset(qd2, 0, sizeof(qd2));

	memcpy(&qd1, RFIFOP(fd,8), num1 * sizeof(struct quest));
	quests->load(quests, &qd2, char_id, &num2);

	for( i = 0; i < num1; i++ )
	{
		ARR_FIND( 0, num2, j, qd1[i].quest_id == qd2[j].quest_id );
		if( j < num2 ) // Update existed quests
		{	// Only states and counts are changable.
			if( qd1[i].state != qd2[j].state || qd1[i].count[0] != qd2[j].count[0] || qd1[i].count[1] != qd2[j].count[1] || qd1[i].count[2] != qd2[j].count[2] )
				quests->update(quests, &qd1[i], char_id);

			if( j < (--num2) )
			{
				memmove(&qd2[j],&qd2[j+1],sizeof(struct quest)*(num2-j));
				memset(&qd2[num2], 0, sizeof(struct quest));
			}
		}
		else // Add new quests
		{
			quests->add(quests, &qd1[i], char_id);

			WBUFL(buf,count*4) = qd1[i].quest_id;
			count++;
		}
	}

	for( i = 0; i < num2; i++ ) // Quests not in qd1 but in qd2 are to be erased.
		quests->del(quests, char_id, qd2[i].quest_id);

	// send back list of newly added quest ids
	WFIFOHEAD(fd,8+4*count);
	WFIFOW(fd,0) = 0x3861;
	WFIFOW(fd,2) = 8+4*count;
	WFIFOL(fd,4) = char_id;
	memcpy(WFIFOP(fd,8), buf, count*4);
	WFIFOSET(fd,WFIFOW(fd,2));
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
