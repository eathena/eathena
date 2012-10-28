// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "guild_castle.h"
#include "clif.h" // clif_guild_emblem_area()
#include "guild.h" // guild_search(), guild_request_info(), guild_npc_request_info()
#include "intif.h" // intif_guild_castle_dataload(), intif_guild_castle_datasave()
#include "map.h" // EVENT_NAME_LENGTH, BL_MOB, map_id2md()
#include "mob.h" // struct mob_data, mob_guardian_guildchange()
#include "npc.h" // npc_event_do()
#include "status.h" // status_calc_mob()
#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h" // struct guild_castle, MAX_GUILDCASTLE, MAX_GUARDIANS
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include <stdlib.h> // atoi()


static DBMap* castle_db; // int castle_id -> struct guild_castle*
static DBMap* guild_castleinfoevent_db; // int castle_id_index -> struct eventlist*


struct eventlist
{
	char name[EVENT_NAME_LENGTH];
	struct eventlist* next;
};


static bool guild_read_castledb(char* str[], int columns, int current)
{// <castle id>,<map name>,<castle name>,<castle event>[,<reserved/unused switch flag>]
	struct guild_castle* gc;

	CREATE(gc, struct guild_castle, 1);
	gc->castle_id = atoi(str[0]);
	gc->mapindex = mapindex_name2id(str[1]);
	safestrncpy(gc->castle_name, str[2], sizeof(gc->castle_name));
	safestrncpy(gc->castle_event, str[3], sizeof(gc->castle_event));

	idb_put(castle_db, gc->castle_id, gc);

	//intif_guild_castle_info(gc->castle_id);

	return true;
}


static int guild_castle_db_final(DBKey key, void* data, va_list ap)
{
	struct guild_castle* gc = (struct guild_castle*)data;
	if( gc->temp_guardians )
		aFree(gc->temp_guardians);
	aFree(data);
	return 0;
}


int guild_addcastleinfoevent(int castle_id, int index, const char* name)
{
	struct eventlist *ev;
	int code = castle_id|(index<<16);

	if( name==NULL || *name==0 )
		return 0;

	ev = (struct eventlist *)aMalloc(sizeof(struct eventlist));
	safestrncpy(ev->name,name,ARRAYLENGTH(ev->name));
	//The next event becomes whatever was currently stored.
	ev->next = (struct eventlist *)idb_put(guild_castleinfoevent_db,code,ev);
	return 0;
}


static int guild_castleinfoevent_db_final(DBKey key, void* data, va_list ap)
{
	aFree(data);
	return 0;
}


/// lookup: castle id -> castle*
struct guild_castle* guild_castle_search(int castle_id)
{
	return (struct guild_castle*)idb_get(castle_db, castle_id);
}


/// lookup: map index -> castle*
struct guild_castle* guild_mapindex2gc(short mapindex)
{
	struct guild_castle* gc;

	DBIterator* iter = castle_db->iterator(castle_db);
	for( gc = (struct guild_castle*)iter->first(iter,NULL); iter->exists(iter); gc = (struct guild_castle*)iter->next(iter,NULL) )
		if( gc->mapindex == mapindex )
			break;
	iter->destroy(iter);

	return gc;
}


/// lookup: map name -> castle*
struct guild_castle* guild_mapname2gc(const char* mapname)
{
	return guild_mapindex2gc(mapindex_name2id(mapname));
}


int guild_castledataload(int castle_id, int index)
{
	return intif_guild_castle_dataload(castle_id, index);
}


int guild_castledataloadack(int castle_id, int index, int value)
{
	int code = castle_id | (index<<16);
	struct eventlist* ev;

	struct guild_castle* gc = guild_castle_search(castle_id);
	if( gc == NULL )
		return 0;
	
	switch(index){
	case 1:
		gc->guild_id = value;
		if (gc->guild_id && guild_search(gc->guild_id)==NULL) //Request guild data which will be required for spawned guardians. [Skotlex]
			guild_request_info(gc->guild_id);
		break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		gc->guardian[index-10].visible = value; break;
	default:
		ShowError("guild_castledataloadack ERROR!! (Not found castle_id=%d index=%d)\n", castle_id, index);
		return 0;
	}

	ev = (struct eventlist*)idb_remove(guild_castleinfoevent_db, code);
	while( ev != NULL )
	{
		struct eventlist* next = ev->next;
		npc_event_do(ev->name);
		aFree(ev);
		ev = next;
	}

	return 1;
}


int guild_castledatasave(int castle_id, int index, int value)
{
	if( index == 1 )
	{	//The castle's owner has changed? Update Guardian ownership, too. [Skotlex]
		struct guild_castle *gc = guild_castle_search(castle_id);
		int m = -1;
		if (gc) m = map_mapindex2mapid(gc->mapindex);
		if (m != -1)
			map_foreachinmap(mob_guardian_guildchange, m, BL_MOB); //FIXME: why not iterate over gc->guardian[i].id ?
	}
	else
	if( index == 3 )
	{	// defense invest change -> recalculate guardian hp
		struct guild_castle* gc = guild_castle_search(castle_id);
		if( gc )
		{
			int i;
			struct mob_data* gd;
			for( i = 0; i < MAX_GUARDIANS; i++ )
				if( gc->guardian[i].visible && (gd = map_id2md(gc->guardian[i].id)) != NULL )
						status_calc_mob(gd,0);
		}
	}

	return intif_guild_castle_datasave(castle_id,index,value);
}


int guild_castledatasaveack(int castle_id,int index,int value)
{
	struct guild_castle *gc=guild_castle_search(castle_id);
	if(gc==NULL){
		return 0;
	}
	switch(index){
	case 1: gc->guild_id = value; break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		gc->guardian[index-10].visible = value; break;
	default:
		ShowError("guild_castledatasaveack ERROR!! (Not found index=%d)\n", index);
		return 0;
	}
	return 1;
}


/// Receive data of all castles at once (initialization).
int guild_castlealldataload(int len,struct guild_castle *gc)
{
	int i;
	int n = (len-4) / sizeof(struct guild_castle);
	int ev;

	nullpo_ret(gc);

	//Last owned castle in the list invokes ::OnAgitinit
	for( i = n-1; i >= 0 && !(gc[i].guild_id); --i );
	ev = i; // offset of castle or -1

	if( ev < 0 ) { //No castles owned, invoke OnAgitInit as it is.
		npc_event_doall("OnAgitInit");
		npc_event_doall("OnAgitInit2");
	}
	else // load received castles into memory, one by one
	for( i = 0; i < n; i++, gc++ )
	{
		struct guild_castle *c = guild_castle_search(gc->castle_id);
		if (!c) {
			ShowError("guild_castlealldataload Castle id=%d not found.\n", gc->castle_id);
			continue;
		}

		// update mapserver castle data with new info
		memcpy(&c->guild_id, &gc->guild_id, sizeof(struct guild_castle) - ((uintptr_t)&c->guild_id - (uintptr_t)c));

		if( c->guild_id )
		{
			if( i != ev )
				guild_request_info(c->guild_id);
			else { // last owned one
				guild_npc_request_info(c->guild_id, "::OnAgitInit");
				guild_npc_request_info(c->guild_id, "::OnAgitInit2");
			}
		}
	}

	return 0;
}


/// Counts the number of castles belonging to this guild.
int guild_castle_count(int guild_id)
{
	DBIterator* iter = db_iterator(castle_db);
	int count = 0;

	struct guild_castle* gc;
	for( gc = (struct guild_castle*)dbi_first(iter) ; dbi_exists(iter); gc = (struct guild_castle*)dbi_next(iter) )
	{
		if( gc->guild_id != guild_id )
			continue;

		++count;
	}

	dbi_destroy(iter);
	return count;
}


/// Clears the ownership of all castles belonging to this guild.
void guild_castle_onguildbreak(int guild_id)
{
	DBIterator* iter = db_iterator(castle_db);

	struct guild_castle* gc;
	for( gc = (struct guild_castle*)dbi_first(iter) ; dbi_exists(iter); gc = (struct guild_castle*)dbi_next(iter) )
	{
		if( gc->guild_id != guild_id )
			continue;

		//Save the new 'owner', this should invoke guardian clean up and other such things.
		gc->guild_id = 0;
		guild_castledatasave(gc->castle_id, 1, 0);
	}

	dbi_destroy(iter);
}


/// Updates the emblem of guardians in all castles belonging to this guild.
void guild_castle_guardian_updateemblem(int guild_id, int emblem_id)
{
	DBIterator* iter = db_iterator(castle_db);

	struct guild_castle* gc;
	for( gc = (struct guild_castle*)dbi_first(iter) ; dbi_exists(iter); gc = (struct guild_castle*)dbi_next(iter) )
	{
		TBL_MOB* md;
		int i;

		if( gc->guild_id != guild_id )
			continue;

		// update permanent guardians
		for( i = 0; i < ARRAYLENGTH(gc->guardian); ++i )
		{
			if( gc->guardian[i].id == 0 )
				continue;

			md = map_id2md(gc->guardian[i].id);
			if( md == NULL || md->guardian_data == NULL )
				continue;

			md->guardian_data->emblem_id = emblem_id;
			clif_guild_emblem_area(&md->bl);
		}

		// update temporary guardians
		for( i = 0; i < gc->temp_guardians_max; ++i )
		{
			if( gc->temp_guardians[i] == 0 )
				continue;

			md = map_id2md(gc->temp_guardians[i]);
			if( md == NULL || md->guardian_data == NULL )
				continue;

			md->guardian_data->emblem_id = emblem_id;
			clif_guild_emblem_area(&md->bl);
		}
	}

	dbi_destroy(iter);
}


void do_init_guild_castle(void)
{
	castle_db = idb_alloc(DB_OPT_BASE);
	guild_castleinfoevent_db = idb_alloc(DB_OPT_BASE);
	sv_readdb(db_path, "castle_db.txt", ',', 4, 5, -1, &guild_read_castledb);
}


void do_final_guild_castle(void)
{
	castle_db->destroy(castle_db, guild_castle_db_final);
	guild_castleinfoevent_db->destroy(guild_castleinfoevent_db, guild_castleinfoevent_db_final);
}
