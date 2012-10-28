// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILD_CASTLE_H_
#define _GUILD_CASTLE_H_

#include "../common/mmo.h"

//For quick linking to a guardian's info.
struct guardian_data
{
	int number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.
	int guild_id;
	int emblem_id;
	int guardup_lv; //Level of GD_GUARDUP skill.
	char guild_name[NAME_LENGTH];
	struct guild_castle* castle;
};

struct guild_castle* guild_castle_search(int castle_id);
struct guild_castle* guild_mapname2gc(const char* mapname);
struct guild_castle* guild_mapindex2gc(short mapindex);

int guild_addcastleinfoevent(int castle_id, int index, const char* name);
int guild_castledataload(int castle_id, int index);
int guild_castledataloadack(int castle_id, int index, int value);
int guild_castledatasave(int castle_id, int index, int value);
int guild_castledatasaveack(int castle_id, int index, int value);
int guild_castlealldataload(int len, struct guild_castle* gc);

int guild_castle_count(int guild_id);
void guild_castle_onguildbreak(int guild_id);
void guild_castle_guardian_updateemblem(int guild_id, int emblem_id);

void do_init_guild_castle(void);
void do_final_guild_castle(void);


#endif // _GUILD_CASTLE_H_
