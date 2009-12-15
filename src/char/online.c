// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "if_login.h" // loginif_*()
#include "if_map.h" // server[]
#include "inter.h" // mapif_disconnectplayer()
#include "online.h"
#include "onlinedb.h"
#include <string.h>


// OnlineDB storage engines available
static struct {
	OnlineDB* (*constructor)(void);
	OnlineDB* engine;
} onlinedb_engines[] = {
// standard engines
#ifdef WITH_SQL
	{online_db_sql, NULL},
#endif
#ifdef WITH_TXT
	{online_db_txt, NULL},
#endif
};


DBMap* online_char_db = NULL; // int account_id -> struct online_char_data*
OnlineDB* onlinedb = NULL;
char onlinedb_engine[256] = "auto";


// forward declarations
static void* create_online_char_data(DBKey key, va_list args);
static int char_db_kickoffline(DBKey key, void* data, va_list ap);
static int online_data_cleanup(int tid, unsigned int tick, int id, intptr data_);
static int chardb_waiting_disconnect(int tid, unsigned int tick, int id, intptr data);
static bool onlinedb_engine_init(void);


void onlinedb_create(void)
{
	int i;

	// create onlinedb engines (to accept config settings)
	for( i = 0; i < ARRAYLENGTH(onlinedb_engines); ++i )
		if( onlinedb_engines[i].constructor )
			onlinedb_engines[i].engine = onlinedb_engines[i].constructor();
}


void onlinedb_config_read(const char* w1, const char* w2)
{
	if( strcmpi(w1, "online.engine") == 0 )
		safestrncpy(onlinedb_engine, w2, sizeof(onlinedb_engine));
	else
	{// try onlinedb engines
		int i;
		for( i = 0; i < ARRAYLENGTH(onlinedb_engines); ++i )
		{
			OnlineDB* engine = onlinedb_engines[i].engine;
			if( engine )
				engine->set_property(engine, w1, w2);
		}
	}
}


void onlinedb_init(void)
{
	online_char_db = idb_alloc(DB_OPT_RELEASE_DATA);
	onlinedb_engine_init();

	onlinedb_sync(); // write online players files with no player

	// ???
	add_timer_func_list(chardb_waiting_disconnect, "chardb_waiting_disconnect");

	// ???
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 1000, online_data_cleanup, 0, 0, 600 * 1000);
}


void onlinedb_final(void)
{
	int i;

	onlinedb_sync(); // write online players files with no player

	online_char_db->destroy(online_char_db, NULL); //dispose the db...
	online_char_db = NULL;

	for( i = 0; i < ARRAYLENGTH(onlinedb_engines); ++i )
	{// destroy all onlinedb engines
		OnlineDB* engine = onlinedb_engines[i].engine;
		if( engine )
		{
			engine->destroy(engine);
			onlinedb_engines[i].engine = NULL;
		}
	}
	onlinedb = NULL;
}


void onlinedb_sync(void)
{
	onlinedb->sync(onlinedb);
}


struct online_char_data* onlinedb_get(int account_id)
{
	return (struct online_char_data*)idb_get(online_char_db, account_id);
}


struct online_char_data* onlinedb_ensure(int account_id)
{
	return (struct online_char_data*)idb_ensure(online_char_db, account_id, create_online_char_data);
}


unsigned int onlinedb_size(void)
{
	return online_char_db->size(online_char_db);
}


DBIterator* onlinedb_iterator(void)
{
	return online_char_db->iterator(online_char_db);
}


void set_char_charselect(int account_id)
{
	struct online_char_data* character = onlinedb_ensure(account_id);

	if( character->server > -1 )
		if( server[character->server].users > 0 ) // Prevent this value from going negative.
			server[character->server].users--;

	character->char_id = -1;
	character->server = -1;

	if(character->waiting_disconnect != -1) {
		delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
		character->waiting_disconnect = -1;
	}

	loginif_char_online(account_id);
}


void set_char_online(int map_id, int char_id, int account_id)
{
	struct online_char_data* character = onlinedb_ensure(account_id);
	
	//Check to see for online conflicts
	if( character->char_id != -1 && character->server > -1 && character->server != map_id )
	{
		ShowNotice("set_char_online: Character %d:%d marked in map server %d, but map server %d claims to have (%d:%d) online!\n",
			character->account_id, character->char_id, character->server, map_id, account_id, char_id);
		mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
	}

	//Update state data
	character->char_id = char_id;
	character->server = map_id;

	if( character->server > -1 )
		server[character->server].users++;

	//Get rid of disconnect timer
	if(character->waiting_disconnect != -1) {
		delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
		character->waiting_disconnect = -1;
	}

	onlinedb->set_online(onlinedb, account_id, char_id);
	loginif_char_online(account_id);
}


void set_char_offline(int char_id, int account_id)
{
	struct online_char_data* character = onlinedb_get(account_id);

	if( character != NULL )
	{	//We don't free yet to avoid aCalloc/aFree spamming during char change. [Skotlex]
		if( character->server > -1 )
			if( server[character->server].users > 0 ) // Prevent this value from going negative.
				server[character->server].users--;
		
		if(character->waiting_disconnect != -1){
			delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
			character->waiting_disconnect = -1;
		}

		if(character->char_id == char_id)
		{
			character->char_id = -1;
			character->server = -1;
		}
	}

	onlinedb->set_offline(onlinedb, account_id, char_id);

	//Remove char if 1- Set all offline, or 2- character is no longer connected to char-server.
	if( char_id == -1 || character == NULL || character->fd == -1 )
		loginif_char_offline(account_id);
}


void set_char_waitdisconnect(int account_id, unsigned int time)
{
	struct online_char_data* character = onlinedb_get(account_id);
	if( character != NULL && character->waiting_disconnect == -1 )
		character->waiting_disconnect = add_timer(gettick()+time, chardb_waiting_disconnect, character->account_id, 0);
}


void set_all_offline(void)
{
	ShowNotice("Sending all users offline.\n");
	online_char_db->foreach(online_char_db,char_db_kickoffline,-1);
	onlinedb->set_offline(onlinedb, -1, -1);

	//Tell login-server to also mark all our characters as offline.
	loginif_all_offline();
}


void onlinedb_mapserver_offline(int map_id)
{
	ShowNotice("Sending users of map-server %d offline.\n",map_id);
	online_char_db->foreach(online_char_db,char_db_kickoffline,map_id);
}


void onlinedb_mapserver_unknown(int map_id)
{
	DBIterator* iter = online_char_db->iterator(online_char_db);
	DBKey key;
	void* data;

	while( (data = iter->next(iter, &key)) != NULL )
	{
		int account_id = key.i;
		struct online_char_data* character = (struct online_char_data*)data;

		if( character->server == map_id )
			character->server = -2; //In some map server that we aren't connected to.
	}
	iter->destroy(iter);
}


static void* create_online_char_data(DBKey key, va_list args)
{
	struct online_char_data* character;

	CREATE(character, struct online_char_data, 1);
	character->account_id = key.i;
	character->char_id = -1;
  	character->server = -1;
	character->fd = -1;
	character->waiting_disconnect = -1;

	return character;
}


static int char_db_kickoffline(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int server_id = va_arg(ap, int);

	if (server_id > -1 && character->server != server_id)
		return 0;

	//Kick out any connected characters, and set them offline as appropiate.
	if (character->server > -1)
		mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 1);
	else
	if (character->waiting_disconnect == -1)
		set_char_offline(character->char_id, character->account_id);
	else
		return 0; // fail

	return 1;
}


//------------------------------------------------
//Invoked 15 seconds after mapif_disconnectplayer in case the map server doesn't
//replies/disconnect the player we tried to kick. [Skotlex]
//------------------------------------------------
static int chardb_waiting_disconnect(int tid, unsigned int tick, int id, intptr data)
{
	struct online_char_data* character;
	character = (struct online_char_data*)idb_get(online_char_db, id);
	if( character != NULL && character->waiting_disconnect == tid )
	{	//Mark it offline due to timeout.
		character->waiting_disconnect = -1;
		set_char_offline(character->char_id, character->account_id);
	}
	return 0;
}


static int online_data_cleanup(int tid, unsigned int tick, int id, intptr data_)
{
	DBIterator* iter = online_char_db->iterator(online_char_db);
	DBKey key;
	void* data;

	while( (data = iter->next(iter, &key)) != NULL )
	{
		int account_id = key.i;
		struct online_char_data* character = (struct online_char_data*)data;

		if( character->fd != -1 )
			continue; //Still connected

		if( character->server == -2 ) //Unknown server.. set them offline
			set_char_offline(character->char_id, character->account_id);

		if( character->server < 0 ) //Free data from players that have not been online for a while.
			db_remove(online_char_db, key);
	}
	iter->destroy(iter);
	return 0;
}


static bool onlinedb_engine_init(void)
{
	int i;
	bool try_all = (strcmp(onlinedb_engine,"auto") == 0);

	for( i = 0; i < ARRAYLENGTH(onlinedb_engines); ++i )
	{
		char name[sizeof(onlinedb_engine)];
		OnlineDB* engine = onlinedb_engines[i].engine;
		if( engine && engine->get_property(engine, "engine.name", name, sizeof(name)) &&
			(try_all || strcmp(name, onlinedb_engine) == 0) )
		{
			if( !engine->init(engine) )
			{
				ShowError("onlinedb_engine_init: failed to initialize engine '%s'.\n", name);
				continue;// try next
			}
			if( try_all )
				safestrncpy(onlinedb_engine, name, sizeof(onlinedb_engine));
			ShowInfo("Using onlinedb engine '%s'.\n", onlinedb_engine);
			onlinedb = engine;
			return true;
		}
	}
	ShowError("onlinedb_engine_init: onlinedb engine '%s' not found.\n", onlinedb_engine);
	return false;
}