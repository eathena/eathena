// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/version.h"
#include "../common/utils.h"
#include "char.h"
#include "chardb.h"
#include "charlog.h"
#include "inter.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_rank.h"
#include "int_registry.h"
#include "int_status.h"
#include "int_storage.h"
#include "charserverdb.h"

#include <string.h>

// CharServerDB engines available
static struct{
	CharServerDB* (*constructor)(void);
	CharServerDB* engine;
} charserver_engines[] = {
// standard engines
#ifdef WITH_TXT
	{charserver_db_txt, NULL},
#endif
#ifdef WITH_SQL
	{charserver_db_sql, NULL},
#endif
// extra engines
#ifdef CHARSERVERDB_ENGINE_0
	{CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_0), NULL},
#endif
#ifdef CHARSERVERDB_ENGINE_1
	{CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_1), NULL},
#endif
#ifdef CHARSERVERDB_ENGINE_2
	{CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_2), NULL},
#endif
#ifdef CHARSERVERDB_ENGINE_3
	{CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_3), NULL},
#endif
#ifdef CHARSERVERDB_ENGINE_4
	{CHARSERVERDB_CONSTRUCTOR(CHARSERVERDB_ENGINE_4), NULL},
#endif
	// end of structure
	{NULL, NULL}
};
// charserver engine
CharServerDB* charserver = NULL;


// temporary imports
extern int parse_fromlogin(int fd);
extern int parse_char(int fd);
#include "map.h"

//FIXME
#ifndef TXT_ONLY
#include "../common/sql.h"
static Sql* sql_handle = NULL;
static char char_db[] = "baadf00d";
static char guild_db[] = "baadf00d";
static char guild_member_db[] = "baadf00d";
#endif


int login_fd=-1, char_fd=-1;
char userid[24];
char passwd[24];
char login_ip_str[128];
uint32 login_ip = 0;
uint16 login_port = 6900;
char char_ip_str[128];
uint32 char_ip = 0;
char bind_ip_str[128];
uint32 bind_ip = INADDR_ANY;
uint16 char_port = 6121;
int char_maintenance = 0;
bool char_new = true;
int char_new_display = 0;
char charserver_engine[256] = "auto";// name of the engine to use (defaults to auto, for the first valid engine)

int online_check = 1; //If one, it won't let players connect when their account is already registered online and will send the relevant map server a kick user request. [Skotlex]


// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];
int subnet_count = 0;

int console = 0;

char db_path[1024] = "db";

int max_connect_user = 0;
int gm_allow_level = 99;

// char config
static char char_log_filename[1024] = "log/char.log";
static bool log_char_enabled = false; // charserver logging
int char_name_option = 0; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
char char_name_letters[1024] = ""; // list of letters/symbols authorised (or not) in a character name. by [Yor]
int guild_exp_rate = 100;
char server_name[20];
char wisp_server_name[NAME_LENGTH] = "Server";
char unknown_char_name[NAME_LENGTH] = "Unknown"; // Name to use when the requested name cannot be determined
int start_zeny = 500;
int start_weapon = 1201;
int start_armor = 2301;
struct point start_point = { 0, 53, 111 }; // Initial position (it's possible to set it in conf file)
#define DEFAULT_AUTOSAVE_INTERVAL 300*1000
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;


// storage-specific options
//TXT
int email_creation = 0; // disabled by default
//SQL
bool db_use_sqldbs;
int char_per_account = 0; //Maximum charas per account (default unlimited) [Sirius]
int char_del_level = 0; //From which level u can delete character [Lupus]


bool name_ignoring_case = false; // Allow or not identical name for characters but with a different case by [Yor]
#define TRIM_CHARS "\032\t\x0A\x0D " //The following characters are trimmed regardless because they cause confusion and problems on the servers. [Skotlex]
bool char_rename = true;

#ifdef TXT_SQL_CONVERT
int save_log = 0; //Have the logs be off by default when converting
#else
int save_log = 1; // show loading/saving messages
#endif


//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
DBMap* auth_db = NULL; // int account_id -> struct auth_node*

//-----------------------------------------------------
// Online User Database
//-----------------------------------------------------
DBMap* online_char_db = NULL; // int account_id -> struct online_char_data*


int chardb_waiting_disconnect(int tid, unsigned int tick, int id, intptr data);

void* create_online_char_data(DBKey key, va_list args)
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

// Searches if the given character is online, and returns the fd of the
// map-server it is connected to.
int search_character_online(int aid, int cid)
{
	//Look for online character.
	struct online_char_data* character;
	character = idb_get(online_char_db, aid);
	if(character &&
		character->char_id == cid &&
		character->server > -1) 
		return server[character->server].fd;
	return -1;
}

void set_char_charselect(int account_id)
{
	struct online_char_data* character;

	character = (struct online_char_data*)idb_ensure(online_char_db, account_id, create_online_char_data);

	if( character->server > -1 )
		server[character->server].users--;

	character->char_id = -1;
	character->server = -1;

	if(character->waiting_disconnect != -1) {
		delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
		character->waiting_disconnect = -1;
	}

	if (login_fd > 0 && !session[login_fd]->flag.eof)
	{
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272b;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}

}

void set_char_online(int map_id, int char_id, int account_id)
{
	struct online_char_data* character;
	
#ifndef TXT_ONLY
	//Update DB
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='1' WHERE `char_id`='%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);
#endif

	//Check to see for online conflicts
	character = (struct online_char_data*)idb_ensure(online_char_db, account_id, create_online_char_data);
	if (online_check && character->char_id != -1 && character->server > -1 && character->server != map_id)
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

	//Notify login server
	if (login_fd > 0 && !session[login_fd]->flag.eof)
	{	
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272b;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}
}

void set_char_offline(int char_id, int account_id)
{
	struct online_char_data* character;

#ifndef TXT_ONLY
	if ( char_id == -1 )
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='0' WHERE `account_id`='%d'", char_db, account_id) )
			Sql_ShowDebug(sql_handle);
	}
	else
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='0' WHERE `char_id`='%d'", char_db, char_id) )
			Sql_ShowDebug(sql_handle);
	}
#endif

	if ((character = (struct online_char_data*)idb_get(online_char_db, account_id)) != NULL)
	{	//We don't free yet to avoid aCalloc/aFree spamming during char change. [Skotlex]
		if( character->server > -1 )
			server[character->server].users--;
		
		if(character->waiting_disconnect != -1){
			delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
			character->waiting_disconnect = -1;
		}

		//If user is NOT at char screen, delete entry [Kevin]
		if(character->char_id != -1)
		{
			idb_remove(online_char_db, account_id);
		}
	}
	
	if (login_fd > 0 && !session[login_fd]->flag.eof && (char_id == -1 || character == NULL || character->char_id != -1))
	{
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272c;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}
}

int char_db_setoffline(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int server = va_arg(ap, int);
	if (server == -1) {
		character->char_id = -1;
		character->server = -1;
		if(character->waiting_disconnect != -1){
			delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
			character->waiting_disconnect = -1;
		}
	} else if (character->server == server)
		character->server = -2; //In some map server that we aren't connected to.
	return 0;
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
	else if (character->waiting_disconnect == -1)
		set_char_offline(character->char_id, character->account_id);
	else
		return 0; // fail

	return 1;
}

void set_all_offline(int id)
{
	if (id < 0)
		ShowNotice("Sending all users offline.\n");
	else
		ShowNotice("Sending users of map-server %d offline.\n",id);
	online_char_db->foreach(online_char_db,char_db_kickoffline,id);

	if (id >= 0 || login_fd <= 0 || session[login_fd]->flag.eof)
		return;
	//Tell login-server to also mark all our characters as offline.
	WFIFOHEAD(login_fd,2);
	WFIFOW(login_fd,0) = 0x2737;
	WFIFOSET(login_fd,2);
}

#ifndef TXT_ONLY
void set_all_offline_sql(void)
{
	//Set all players to 'OFFLINE'
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online` = '0'", char_db) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online` = '0'", guild_member_db) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `connect_member` = '0'", guild_db) )
		Sql_ShowDebug(sql_handle);
}
#endif


//---------------------------------------------------------------------
// This function return the number of online players in all map-servers
//---------------------------------------------------------------------
int count_users(void)
{
	int i, users;

	if( login_fd <= 0 || session[login_fd] == NULL )
		return 0;

	users = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		if (server[i].fd > 0) {
			users += server[i].users;
		}
	}
	return users;
}


/// Writes char data to the buffer in the format used by the client.
/// Used in packets 0x6b (chars info) and 0x6d (new char info)
/// Returns the size (106 or 108)
int mmo_char_tobuf(uint8* buf, struct mmo_charstatus* p)
{
	if( buf == NULL || p == NULL )
		return 0;

	WBUFL(buf,0) = p->char_id;
	WBUFL(buf,4) = min(p->base_exp, LONG_MAX);
	WBUFL(buf,8) = p->zeny;
	WBUFL(buf,12) = min(p->job_exp, LONG_MAX);
	WBUFL(buf,16) = p->job_level;
	WBUFL(buf,20) = 0; // probably opt1
	WBUFL(buf,24) = 0; // probably opt2
	WBUFL(buf,28) = p->option;
	WBUFL(buf,32) = p->karma;
	WBUFL(buf,36) = p->manner;
	WBUFW(buf,40) = min(p->status_point, SHRT_MAX);
	WBUFW(buf,42) = min(p->hp, SHRT_MAX);
	WBUFW(buf,44) = min(p->max_hp, SHRT_MAX);
	WBUFW(buf,46) = min(p->sp, SHRT_MAX);
	WBUFW(buf,48) = min(p->max_sp, SHRT_MAX);
	WBUFW(buf,50) = DEFAULT_WALK_SPEED; // p->speed;
	WBUFW(buf,52) = p->class_;
	WBUFW(buf,54) = p->hair;
	WBUFW(buf,56) = p->option&0x20 ? 0 : p->weapon; //When the weapon is sent and your option is riding, the client crashes on login!?
	WBUFW(buf,58) = p->base_level;
	WBUFW(buf,60) = min(p->skill_point, SHRT_MAX);
	WBUFW(buf,62) = p->head_bottom;
	WBUFW(buf,64) = p->shield;
	WBUFW(buf,66) = p->head_top;
	WBUFW(buf,68) = p->head_mid;
	WBUFW(buf,70) = p->hair_color;
	WBUFW(buf,72) = p->clothes_color;
	memcpy(WBUFP(buf,74), p->name, NAME_LENGTH);
	WBUFB(buf,98) = min(p->str, UCHAR_MAX);
	WBUFB(buf,99) = min(p->agi, UCHAR_MAX);
	WBUFB(buf,100) = min(p->vit, UCHAR_MAX);
	WBUFB(buf,101) = min(p->int_, UCHAR_MAX);
	WBUFB(buf,102) = min(p->dex, UCHAR_MAX);
	WBUFB(buf,103) = min(p->luk, UCHAR_MAX);
	WBUFW(buf,104) = p->slot;
	if (char_rename) {
		WBUFW(buf,106) = 1;// Rename bit (0=rename,1=no rename)
		return 108;
	} else {
		return 106;
	}
}

//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int mmo_char_send006b(int fd, struct char_session_data* sd)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd_arr[MAX_CHARS];
	struct mmo_charstatus cd;
	CharDBIterator* it;
	int i,j;

#ifndef TXT_ONLY
	if (save_log)
		ShowInfo("Loading Char Data ("CL_BOLD"%d"CL_RESET")\n",sd->account_id);
#endif

	// load characters
	memset(cd_arr, 0, sizeof(cd_arr));
	sd->chars_num = 0;
	it = chars->characters(chars, sd->account_id);
	while( it->next(it, &cd) )
	{
		++sd->chars_num;
		if( cd.slot < MAX_CHARS )
		{// use slot position
			if( cd_arr[cd.slot].account_id == sd->account_id )
			{// move occupant to first free slot
				ARR_FIND(0, MAX_CHARS, i, cd_arr[i].account_id != sd->account_id);
				if( i < MAX_CHARS )
					memcpy(&cd_arr[i], &cd_arr[cd.slot], sizeof(struct mmo_charstatus));
			}
			memcpy(&cd_arr[cd.slot], &cd, sizeof(struct mmo_charstatus));
		}
		else
		{// use first free slot
			ARR_FIND(0, MAX_CHARS, i, cd_arr[i].account_id != sd->account_id);
			if( i < MAX_CHARS )
				memcpy(&cd_arr[i], &cd, sizeof(struct mmo_charstatus));
		}
	}
	it->destroy(it);

	// update client view
	for( i = 0; i < MAX_CHARS; ++i )
	{
		cd_arr[i].slot = i; // XXX if different, update slot in the database?
		sd->slots[i] = cd_arr[i].char_id;
	}

	j = 24; // offset
	WFIFOHEAD(fd, 4 + 20 + MAX_CHARS*108); // or 106(!)
	WFIFOW(fd,0) = 0x6b;
	memset(WFIFOP(fd,4), 0, 20); // unknown bytes
	for( i = 0; i < MAX_CHARS; ++i )
		if( cd_arr[i].account_id == sd->account_id )
			j += mmo_char_tobuf(WFIFOP(fd,j), &cd_arr[i]);
	WFIFOW(fd,2) = j; // packet len
	WFIFOSET(fd,j);

	return 0;
}


void char_auth_ok(int fd, struct char_session_data *sd)
{
	struct online_char_data* character;
	if (max_connect_user && count_users() >= max_connect_user && sd->gmlevel < gm_allow_level)
	{
		// refuse connection (over populated)
		WFIFOW(fd,0) = 0x6c;
		WFIFOW(fd,2) = 0;
		WFIFOSET(fd,3);
		return;
	}

	if( online_check && (character = (struct online_char_data*)idb_get(online_char_db, sd->account_id)) != NULL )
	{	// check if character is not online already. [Skotlex]
		if (character->server > -1)
		{	//Character already online. KICK KICK KICK
			mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
			if (character->waiting_disconnect == -1)
				character->waiting_disconnect = add_timer(gettick()+20000, chardb_waiting_disconnect, character->account_id, 0);
			WFIFOW(fd,0) = 0x81;
			WFIFOB(fd,2) = 8;
			WFIFOSET(fd,3);
			return;
		}
		if (character->fd >= 0 && character->fd != fd)
		{	//There's already a connection from this account that hasn't picked a char yet.
			WFIFOW(fd,0) = 0x81;
			WFIFOB(fd,2) = 8;
			WFIFOSET(fd,3);
			return;
		}
		character->fd = fd;
	}

	if (login_fd > 0) {
		// request to login-server to obtain e-mail/time limit
		//FIXME: isn't this part of the auth_ok packet? [ultramage]
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2716;
		WFIFOL(login_fd,2) = sd->account_id;
		WFIFOSET(login_fd,6);
	}

	// mark session as 'authed'
	sd->auth = true;

	// set char online on charserver
	set_char_charselect(sd->account_id);

	// send characters to player
	mmo_char_send006b(fd, sd);
}


//----------------------------------------------------------------------
// Force disconnection of an online player (with account value) by [Yor]
//----------------------------------------------------------------------
int disconnect_player(int account_id)
{
	int i;
	struct char_session_data *sd;

	// disconnect player if online on char-server
	ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == account_id );
	if( i < fd_max )
		set_eof(i);

	return 0;
}

int broadcast_user_count(int tid, unsigned int tick, int id, intptr data)
{
	uint8 buf[6];
	int users = count_users();

	// only send an update when needed
	static int prev_users = 0;
	if( prev_users == users )
		return 0;
	prev_users = users;

	if( login_fd > 0 && session[login_fd] )
	{
		// send number of user to login server
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}

	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf,6);

	/* TODO: move to plugin
#ifdef TXT_ONLY
	// refresh online files (txt and html)
	create_online_files();
#endif
	*/

	return 0;
}


// Console Command Parser [Wizputer]
int parse_console(char* buf)
{
	char command[256];

	memset(command, 0, sizeof(command));

	sscanf(buf, "%[^\n]", command);

	log_char("Console command :%s\n", command);

	if( strcmpi("shutdown", command) == 0 ||
	    strcmpi("exit", command) == 0 ||
	    strcmpi("quit", command) == 0 ||
	    strcmpi("end", command) == 0 )
		runflag = 0;
	else if( strcmpi("alive", command) == 0 ||
	         strcmpi("status", command) == 0 )
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else if( strcmpi("help", command) == 0 ){
		ShowInfo(CL_BOLD"Help of commands:"CL_RESET"\n");
		ShowInfo("  To shutdown the server:\n");
		ShowInfo("  'shutdown|exit|qui|end'\n");
		ShowInfo("  To know if server is alive:\n");
		ShowInfo("  'alive|status'\n");
	}

	return 0;
}


// sends data to all mapservers
int mapif_sendall(unsigned char *buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

// sends data to all mapservers other than the one specified
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

// send data to a single mapserver
int mapif_send(int fd, unsigned char *buf, unsigned int len)
{
	int i;

	if (fd >= 0) {
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (fd == server[i].fd) {
				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}


//-----------------------------------
// Function to create a new character
//-----------------------------------
int char_create(int account_id, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, int* out_char_id)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd;
	char name[NAME_LENGTH];
	int i;

	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name,TRIM_CHARS);

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// check content of character name
	if( remove_control_chars(name) )
		return -2; // control chars in name

	// check for reserved names
	if( strcmpi(name, main_chat_nick) == 0 || strcmpi(name, wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// Check Authorised letters/symbols in the name of the character
	if( char_name_option == 1 ) { // only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) == NULL )
				return -2;
	} else
	if( char_name_option == 2 ) { // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) != NULL )
				return -2;
	} // else, all letters/symbols are authorised (except control char removed before)

	// check name (already in use?)
	if( chars->name2id(chars, name, NULL, NULL) )
		return -1; // name already exists

	//check other inputs
	if((slot >= MAX_CHARS) // slots
	|| (hair_style >= 24) // hair style
	|| (hair_color >= 9) // hair color
	|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
	|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
	|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
		return -2; // invalid input

	// check char slot
	if( chars->slot2id(chars, account_id, slot, &i) )
		return -2; // slot already in use

	// insert new char to database
	memset(&cd, 0, sizeof(cd));

	cd.char_id = -1;
	cd.account_id = account_id;
	cd.slot = slot;
	safestrncpy(cd.name, name, NAME_LENGTH);
	cd.class_ = 0;
	cd.base_level = 1;
	cd.job_level = 1;
	cd.base_exp = 0;
	cd.job_exp = 0;
	cd.zeny = start_zeny;
	cd.str = str;
	cd.agi = agi;
	cd.vit = vit;
	cd.int_ = int_;
	cd.dex = dex;
	cd.luk = luk;
	cd.max_hp = 40 * (100 + cd.vit) / 100;
	cd.max_sp = 11 * (100 + cd.int_) / 100;
	cd.hp = cd.max_hp;
	cd.sp = cd.max_sp;
	cd.status_point = 0;
	cd.skill_point = 0;
	cd.option = 0;
	cd.karma = 0;
	cd.manner = 0;
	cd.party_id = 0;
	cd.guild_id = 0;
	cd.hair = hair_style;
	cd.hair_color = hair_color;
	cd.clothes_color = 0;
	cd.weapon = 0; // W_FIST
	cd.shield = 0;
	cd.head_top = 0;
	cd.head_mid = 0;
	cd.head_bottom = 0;
	memcpy(&cd.last_point, &start_point, sizeof(start_point));
	memcpy(&cd.save_point, &start_point, sizeof(start_point));

	if( start_weapon > 0 )
	{// add Start Weapon (Knife?)
		cd.inventory[0].nameid = start_weapon; // Knife
		cd.inventory[0].amount = 1;
		cd.inventory[0].identify = 1;
	}
	if( start_armor > 0 )
	{// Add default armor (cotton shirt?)
		cd.inventory[1].nameid = start_armor; // Cotton Shirt
		cd.inventory[1].amount = 1;
		cd.inventory[1].identify = 1;
	}

	if( !chars->create(chars, &cd) )
		return -2; // abort

	//Retrieve the newly auto-generated char id
	if( out_char_id )
		*out_char_id = cd.char_id;

	// validation success, log result
	charlog_log(cd.char_id, cd.account_id, cd.slot, cd.name, "make new char (stats:%d/%d/%d/%d/%d/%d, hair-style:%d, hair-color:%d)", str, agi, vit, int_, dex, luk, hair_style, hair_color);

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", account_id, cd.char_id, slot, name);
	return 0;
}


void char_divorce(int partner_id1, int partner_id2)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd1, cd2;
	unsigned char buf[10];
	int i;

	if( !chars->load_num(chars, &cd1, partner_id1) || !chars->load_num(chars, &cd2, partner_id2) )
		return; // char not found

	if( cd1.partner_id != cd2.char_id || cd2.partner_id != cd1.char_id )
		return; // not married to each other

	// end marriage
	cd1.partner_id = 0;
	cd2.partner_id = 0;

	// remove rings from both partners' inventory
	for( i = 0; i < MAX_INVENTORY; ++i )
	{
		if( cd1.inventory[i].nameid == WEDDING_RING_M || cd1.inventory[i].nameid == WEDDING_RING_F )
			memset(&cd1.inventory[i], 0, sizeof(struct item));
		if( cd2.inventory[i].nameid == WEDDING_RING_M || cd2.inventory[i].nameid == WEDDING_RING_F )
			memset(&cd2.inventory[i], 0, sizeof(struct item));
	}

	// update data
	//FIXME: unsafe
	chars->save(chars, &cd1);
	chars->save(chars, &cd2);

	// notify all mapservers
	WBUFW(buf,0) = 0x2b12;
	WBUFL(buf,2) = partner_id1;
	WBUFL(buf,6) = partner_id2;
	mapif_sendall(buf,10);
}


int char_delete(int char_id)
{
	CharDB* chars = charserver->chardb(charserver);
	FriendDB* friends = charserver->frienddb(charserver);
	HotkeyDB* hotkeys = charserver->hotkeydb(charserver);
	struct mmo_charstatus cd;
	int i;

	if( !chars->load_num(chars, &cd, char_id) )
	{
		ShowError("char_delete: Unable to fetch data for char %d, deletion aborted.\n", char_id);
		return 1;
	}

	//check for config char del condition [Lupus]
	if( ( char_del_level > 0 && cd.base_level >= (unsigned int)(char_del_level) )
	 || ( char_del_level < 0 && cd.base_level <= (unsigned int)(-char_del_level) )
	) {
		ShowInfo("char_delete: Deletion of char %d:'%s' aborted due to min/max level restrictions (has %d).\n", cd.char_id, cd.name, cd.base_level);
		return -1;
	}

	// delete the hatched pet if you have one...
	if( cd.pet_id )
		inter_pet_delete(cd.pet_id);

	// delete all pets that are stored in eggs (inventory + cart)
	for( i = 0; i < MAX_INVENTORY; i++ )
		if( cd.inventory[i].card[0] == (short)0xff00 ) // CARD0_PET
			inter_pet_delete(MakeDWord(cd.inventory[i].card[1],cd.inventory[i].card[2]));
	for( i = 0; i < MAX_CART; i++ )
		if( cd.cart[i].card[0] == (short)0xff00 ) // CARD0_PET
			inter_pet_delete(MakeDWord(cd.cart[i].card[1],cd.cart[i].card[2]));

	// delete homunculus
	if( cd.hom_id )
		inter_homun_delete(cd.hom_id);

	// leave party
	if( cd.party_id )
		inter_party_leave(cd.party_id, cd.account_id, cd.char_id);

	// leave/break guild
	if( cd.guild_id )
		inter_guild_leave(cd.guild_id, cd.account_id, cd.char_id);

	// divorce
	if( cd.partner_id )
		char_divorce(cd.char_id, cd.partner_id);

	// De-addopt
	if( cd.father || cd.mother )
	{// Char is Baby
		unsigned char buf[64];
		struct mmo_charstatus fd, md;

		if( cd.father && chars->load_num(chars, &fd, cd.father) )
		{
			fd.child = 0;
			fd.skill[410].id = 0;
			fd.skill[410].lv = 0;
			fd.skill[410].flag = 0;
			chars->save(chars, &fd);
		}
		if( cd.mother && chars->load_num(chars, &md, cd.mother) )
		{
			md.child = 0;
			md.skill[410].id = 0;
			md.skill[410].lv = 0;
			md.skill[410].flag = 0;
			chars->save(chars, &md);
		}

		WBUFW(buf,0) = 0x2b25;
		WBUFL(buf,2) = cd.father;
		WBUFL(buf,6) = cd.mother;
		WBUFL(buf,10) = char_id; // Baby
		mapif_sendall(buf,14);
	}

	// clear status changes
	inter_status_delete(cd.char_id);

	// delete character registry
	inter_charreg_delete(cd.char_id);

	// delete friends list
	friends->remove(friends, cd.char_id);

	// delete hotkeys list
	hotkeys->remove(hotkeys, cd.char_id);

	// delete the character and all associated data
	chars->remove(chars, cd.char_id);

	charlog_log(cd.char_id, cd.account_id, -1, cd.name, "Deleted char");

	return 0;
}

int char_married(int pl1, int pl2)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd1, cd2;
	if( !chars->load_num(chars, &cd1, pl1) || !chars->load_num(chars, &cd2, pl2) )
		return 0; //Some character not found??

	return( cd1.char_id == cd2.partner_id && cd2.char_id == cd1.partner_id );
}

int char_child(int parent_id, int child_id)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus parent, child;
	if( !chars->load_num(chars, &parent, parent_id) || !chars->load_num(chars, &child, child_id) )
		return 0; //Some character not found??

	return( parent.child == child.char_id && (parent.char_id == child.father || parent.char_id == child.mother) );
}

int char_family(int cid1, int cid2, int cid3)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd1, cd2, cd3;
	if( !chars->load_num(chars, &cd1, cid1) || !chars->load_num(chars, &cd2, cid2) || !chars->load_num(chars, &cd3, cid3) )
		return 0; //Some character not found??

	//Unless the dbs are corrupted, these 3 checks should suffice, even though 
	//we could do a lot more checks and force cross-reference integrity.
	if( cd1.partner_id == cid2 && cd1.child == cid3 )
		return cid3; //cid1/cid2 parents. cid3 child.

	if( cd1.partner_id == cid3 && cd1.child == cid2 )
		return cid2; //cid1/cid3 parents. cid2 child.

	if( cd2.partner_id == cid3 && cd2.child == cid1 )
		return cid1; //cid2/cid3 parents. cid1 child.

	return 0;
}


//------------------------------------------------
//Invoked 15 seconds after mapif_disconnectplayer in case the map server doesn't
//replies/disconnect the player we tried to kick. [Skotlex]
//------------------------------------------------
int chardb_waiting_disconnect(int tid, unsigned int tick, int id, intptr data)
{
	struct online_char_data* character;
	if ((character = (struct online_char_data*)idb_get(online_char_db, id)) != NULL && character->waiting_disconnect == tid)
	{	//Mark it offline due to timeout.
		character->waiting_disconnect = -1;
		set_char_offline(character->char_id, character->account_id);
	}
	return 0;
}

static int online_data_cleanup_sub(DBKey key, void *data, va_list ap)
{
	struct online_char_data *character= (struct online_char_data*)data;
	if (character->fd != -1)
		return 0; //Still connected
	if (character->server == -2) //Unknown server.. set them offline
		set_char_offline(character->char_id, character->account_id);
	if (character->server < 0)
		//Free data from players that have not been online for a while.
		db_remove(online_char_db, key);
	return 0;
}

static int online_data_cleanup(int tid, unsigned int tick, int id, intptr data)
{
	online_char_db->foreach(online_char_db, online_data_cleanup_sub);
	return 0;
}


/// load this char's account id into the 'online accounts' packet
static int send_accounts_tologin_sub(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int* i = va_arg(ap, int*);

	if(character->server > -1)
	{
		WFIFOL(login_fd,8+(*i)*4) = character->account_id;
		(*i)++;
		return 1;
	}
	return 0;
}

int send_accounts_tologin(int tid, unsigned int tick, int id, intptr data)
{
	if (login_fd > 0 && session[login_fd])
	{
		// send account list to login server
		int users = online_char_db->size(online_char_db);
		int i = 0;

		WFIFOHEAD(login_fd,8+users*4);
		WFIFOW(login_fd,0) = 0x272d;
		online_char_db->foreach(online_char_db, send_accounts_tologin_sub, &i, users);
		WFIFOW(login_fd,2) = 8+ i*4;
		WFIFOL(login_fd,4) = i;
		WFIFOSET(login_fd,WFIFOW(login_fd,2));
	}
	return 0;
}

int check_connect_login_server(int tid, unsigned int tick, int id, intptr data)
{
	if (login_fd > 0 && session[login_fd] != NULL)
		return 0;

	ShowInfo("Attempt to connect to login-server...\n");
	login_fd = make_connection(login_ip, login_port);
	if (login_fd == -1)
	{	//Try again later. [Skotlex]
		login_fd = 0;
		return 0;
	}
	session[login_fd]->func_parse = parse_fromlogin;
	session[login_fd]->flag.server = 1;
	realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
	
	WFIFOHEAD(login_fd,86);
	WFIFOW(login_fd,0) = 0x2710;
	memcpy(WFIFOP(login_fd,2), userid, 24);
	memcpy(WFIFOP(login_fd,26), passwd, 24);
	WFIFOL(login_fd,50) = 0;
	WFIFOL(login_fd,54) = htonl(char_ip);
	WFIFOL(login_fd,58) = htons(char_port);
	memcpy(WFIFOP(login_fd,60), server_name, 20);
	WFIFOW(login_fd,80) = 0;
	WFIFOW(login_fd,82) = char_maintenance;
	WFIFOW(login_fd,84) = char_new_display; //only display (New) if they want to [Kevin]
	WFIFOSET(login_fd,86);
	
	return 1;
}

// sends a ping packet to login server (will receive pong 0x2718)
int ping_login_server(int tid, unsigned int tick, int id, intptr data)
{
	if (login_fd > 0 && session[login_fd] != NULL)
	{
		WFIFOHEAD(login_fd,2);
		WFIFOW(login_fd,0) = 0x2719;
		WFIFOSET(login_fd,2);
	}
	return 0;
}


/// server event logging
void log_char(const char* fmt, ...)
{
	FILE* log_fp;
	va_list ap;

	if( !log_char_enabled )
		return;

	log_fp = fopen(char_log_filename, "a");
	if( log_fp == NULL )
		return;

	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);

	fclose(log_fp);
}

//----------------------------------
// Reading Lan Support configuration
// Rewrote: Anvanced subnet check [LuzZza]
//----------------------------------
int char_lan_config_read(const char *lancfgName)
{
	FILE *fp;
	int line_num = 0;
	char line[1024], w1[64], w2[64], w3[64], w4[64];
	
	if((fp = fopen(lancfgName, "r")) == NULL) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	ShowInfo("Reading the configuration file %s...\n", lancfgName);

	while(fgets(line, sizeof(line), fp))
	{
		line_num++;		
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4) {
	
			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);	
			continue;
		}

		remove_control_chars(w1);
		remove_control_chars(w2);
		remove_control_chars(w3);
		remove_control_chars(w4);

		if( strcmpi(w1, "subnet") == 0 )
		{
			subnet[subnet_count].mask = str2ip(w2);
			subnet[subnet_count].char_ip = str2ip(w3);
			subnet[subnet_count].map_ip = str2ip(w4);

			if( (subnet[subnet_count].char_ip & subnet[subnet_count].mask) != (subnet[subnet_count].map_ip & subnet[subnet_count].mask) )
			{
				ShowError("%s: Configuration Error: The char server (%s) and map server (%s) belong to different subnetworks!\n", lancfgName, w3, w4);
				continue;
			}
				
			subnet_count++;
		}
	}

	ShowStatus("Read information about %d subnetworks.\n", subnet_count);

	fclose(fp);
	return 0;
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_subnetcheck(uint32 ip)
{
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	if( i < subnet_count ) {
		ShowInfo("Subnet check [%u.%u.%u.%u]: Matches "CL_CYAN"%u.%u.%u.%u/%u.%u.%u.%u"CL_RESET"\n", CONVIP(ip), CONVIP(subnet[i].char_ip & subnet[i].mask), CONVIP(subnet[i].mask));
		return subnet[i].char_ip;
	} else {
		ShowInfo("Subnet check [%u.%u.%u.%u]: "CL_CYAN"WAN"CL_RESET"\n", CONVIP(ip));
		return 0;
	}
}


int char_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");

	if (fp == NULL) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return 1;
	}

	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);
		if(strcmpi(w1,"timestamp_format") == 0) {
			strncpy(timestamp_format, w2, 20);
		} else if(strcmpi(w1,"console_silent")==0){
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
		} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
			stdout_with_ansisequence = config_switch(w2);
		} else if (strcmpi(w1, "userid") == 0) {
			strncpy(userid, w2, 24);
		} else if (strcmpi(w1, "passwd") == 0) {
			strncpy(passwd, w2, 24);
		} else if (strcmpi(w1, "server_name") == 0) {
			strncpy(server_name, w2, 20);
			server_name[sizeof(server_name) - 1] = '\0';
			ShowStatus("%s server has been initialized\n", w2);
		} else if (strcmpi(w1, "wisp_server_name") == 0) {
			if (strlen(w2) >= 4) {
				memcpy(wisp_server_name, w2, sizeof(wisp_server_name));
				wisp_server_name[sizeof(wisp_server_name) - 1] = '\0';
			}
		} else if (strcmpi(w1, "login_ip") == 0) {
			char ip_str[16];
			login_ip = host2ip(w2);
			if (login_ip) {
				strncpy(login_ip_str, w2, sizeof(login_ip_str));
				ShowStatus("Login server IP address : %s -> %s\n", w2, ip2str(login_ip, ip_str));
			}
		} else if (strcmpi(w1, "login_port") == 0) {
			login_port = atoi(w2);
		} else if (strcmpi(w1, "char_ip") == 0) {
			char ip_str[16];
			char_ip = host2ip(w2);
			if (char_ip){
				strncpy(char_ip_str, w2, sizeof(char_ip_str));
				ShowStatus("Character server IP address : %s -> %s\n", w2, ip2str(char_ip, ip_str));
			}
		} else if (strcmpi(w1, "bind_ip") == 0) {
			char ip_str[16];
			bind_ip = host2ip(w2);
			if (bind_ip) {
				strncpy(bind_ip_str, w2, sizeof(bind_ip_str));
				ShowStatus("Character server binding IP address : %s -> %s\n", w2, ip2str(bind_ip, ip_str));
			}
		} else if (strcmpi(w1, "char_port") == 0) {
			char_port = atoi(w2);
		} else if (strcmpi(w1, "char_maintenance") == 0) {
			char_maintenance = atoi(w2);
		} else if (strcmpi(w1, "char_new") == 0) {
			char_new = (bool)atoi(w2);
		} else if (strcmpi(w1, "char_new_display") == 0) {
			char_new_display = atoi(w2);
#ifdef TXT_ONLY
		} else if (strcmpi(w1, "email_creation") == 0) {
			email_creation = config_switch(w2);
#endif
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			max_connect_user = atoi(w2);
			if (max_connect_user < 0)
				max_connect_user = 0; // unlimited online players
		} else if(strcmpi(w1, "gm_allow_level") == 0) {
			gm_allow_level = atoi(w2);
			if(gm_allow_level < 0)
				gm_allow_level = 99;
		} else if (strcmpi(w1, "online_check") == 0) {
			online_check = config_switch(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2)*1000;
		} else if (strcmpi(w1, "save_log") == 0) {
			save_log = config_switch(w2);
		} else if (strcmpi(w1, "start_point") == 0) {
			char map[MAP_NAME_LENGTH_EXT];
			int x, y;
			if (sscanf(w2, "%15[^,],%d,%d", map, &x, &y) < 3)
				continue;
			start_point.map = mapindex_name2id(map);
			if (!start_point.map)
				ShowError("Specified start_point %s not found in map-index cache.\n", map);
			start_point.x = x;
			start_point.y = y;
		} else if (strcmpi(w1, "start_zeny") == 0) {
			start_zeny = atoi(w2);
			if (start_zeny < 0)
				start_zeny = 0;
		} else if (strcmpi(w1, "start_weapon") == 0) {
			start_weapon = atoi(w2);
			if (start_weapon < 0)
				start_weapon = 0;
		} else if (strcmpi(w1, "start_armor") == 0) {
			start_armor = atoi(w2);
			if (start_armor < 0)
				start_armor = 0;
		} else if(strcmpi(w1,"log_char")==0) {
			log_char_enabled = atoi(w2);
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			strcpy(unknown_char_name, w2);
			unknown_char_name[NAME_LENGTH-1] = '\0';
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			name_ignoring_case = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			strcpy(char_name_letters, w2);
		} else if (strcmpi(w1, "char_rename") == 0) {
			char_rename = config_switch(w2);
		} else if(strcmpi(w1,"db_path")==0) {
			strcpy(db_path,w2);
#ifndef TXT_ONLY
		} else if (strcmpi(w1, "chars_per_account") == 0) { //maxchars per account [Sirius]
			char_per_account = atoi(w2);
		} else if (strcmpi(w1, "char_del_level") == 0) { //disable/enable char deletion by its level condition [Lupus]
			char_del_level = atoi(w2);
#endif
		} else if (strcmpi(w1, "console") == 0) {
			console = config_switch(w2);
		} else if (strcmpi(w1, "guild_exp_rate") == 0) {
			guild_exp_rate = atoi(w2);
		}
		else if( rank_config_read(w1,w2) )
			continue;
		else if( charlog_config_read(w1,w2) ) // FIXME: some shared settings are in here
			continue;
		else if (strcmpi(w1, "import") == 0)
			char_config_read(w2);
		else if(!strcmpi(w1, "charserver.engine"))
			safestrncpy(charserver_engine, w2, sizeof(charserver_engine));
		else
		{// try the charserver engines
			int i;
			for( i = 0; charserver_engines[i].constructor; ++i )
			{
				CharServerDB* engine = charserver_engines[i].engine;
				if( engine )
					engine->set_property(engine, w1, w2);
			}
			// try others
		}
	}
	fclose(fp);
	
	ShowInfo("Done reading %s.\n", cfgName);
	return 0;
}

/// Select the engine based on the config settings.
/// Updates the config setting with the selected engine if using 'auto'.
///
/// @return true if a valid engine is found and initialized
static bool init_charserver_engine(void)
{
	int i;
	bool try_all = (strcmp(charserver_engine,"auto") == 0);

	for( i = 0; charserver_engines[i].constructor; ++i )
	{
		char name[sizeof(charserver_engine)];
		CharServerDB* engine = charserver_engines[i].engine;
		if( engine && engine->get_property(engine, "engine.name", name, sizeof(name)) &&
			(try_all || strcmp(name, charserver_engine) == 0) )
		{
			if( !engine->init(engine) )
			{
				ShowError("init_charserver_engine: failed to initialize engine '%s'.\n", name);
				continue;// try next
			}
			if( try_all )
				safestrncpy(charserver_engine, name, sizeof(charserver_engine));
			ShowInfo("Using charserver engine '%s'.\n", charserver_engine);
			charserver = engine;
			return true;
		}
	}
	ShowError("init_charserver_engine: charserver engine '%s' not found.\n", charserver_engine);
	return false;
}


//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_CHAR;
}

void do_final(void)
{
	int i;

	//TODO: pick one
	ShowStatus("Terminating server.\n");
	ShowInfo("Doing final stage...\n");

#ifdef TXT_ONLY

	inter_save();
	set_all_offline(-1);
	flush_fifos();
	// write online players files with no player
	online_char_db->clear(online_char_db, NULL); //clean the db...
	/* TODO: move to plugin
	create_online_files();
	*/
	online_char_db->destroy(online_char_db, NULL); //dispose the db...
	auth_db->destroy(auth_db, NULL);

	if (login_fd > 0)
		do_close(login_fd);
	if (char_fd > 0)
		do_close(char_fd);

	inter_final();
	mapindex_final();
#else

	//check SQL save progress.
	//wait until save char complete

	set_all_offline(-1);
	set_all_offline_sql();

	inter_final();

	flush_fifos();

	mapindex_final();

	if (login_fd > 0)
		do_close(login_fd);
	if (char_fd > 0)
		do_close(char_fd);
	online_char_db->destroy(online_char_db, NULL);
	auth_db->destroy(auth_db, NULL);
#endif

	log_char("----End of char-server (normal shutdown).\n");
	charlog_final();

	for( i = 0; charserver_engines[i].constructor; ++i )
	{// destroy all charserver engines
		CharServerDB* engine = charserver_engines[i].engine;
		if( engine )
		{
			engine->destroy(engine);
			charserver_engines[i].engine = NULL;
		}
	}
	charserver = NULL; // destroyed in charserver_engines

	ShowInfo("ok! all done...\n");
}

int do_init(int argc, char **argv)
{
	int i;

	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		memset(&server[i], 0, sizeof(struct mmo_map_server));
		server[i].fd = -1;
	}

	//Read map indexes
	mapindex_init();
	start_point.map = mapindex_name2id("new_zone01");

	// create engines (to accept config settings)
	for( i = 0; charserver_engines[i].constructor; ++i )
		charserver_engines[i].engine = charserver_engines[i].constructor();

	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_lan_config_read((argc > 3) ? argv[3] : LAN_CONF_NAME);

	if (strcmp(userid, "s1")==0 && strcmp(passwd, "p1")==0) {
		ShowError("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your save/account.txt file to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("And then change the user/password to use in conf/char_athena.conf (or conf/import/char_conf.txt)\n");
	}

	ShowInfo("Initializing char server.\n");
	log_char("The char-server is starting...\n");
	charlog_init();

	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);
	online_char_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if( !init_charserver_engine() )
		;// TODO stop server

	inter_init(charserver);

	set_defaultparse(parse_char);

	if ((naddr_ != 0) && (!login_ip || !char_ip))
	{
		char ip_str[16];
		ip2str(addr_[0], ip_str);

		if (naddr_ > 1)
			ShowStatus("Multiple interfaces detected..  using %s as our IP address\n", ip_str);
		else
			ShowStatus("Defaulting to %s as our IP address\n", ip_str);
		if (!login_ip) {
			strcpy(login_ip_str, ip_str);
			login_ip = str2ip(login_ip_str);
		}
		if (!char_ip) {
			strcpy(char_ip_str, ip_str);
			char_ip = str2ip(char_ip_str);
		}
	}

	// establish char-login connection if not present
	add_timer_func_list(check_connect_login_server, "check_connect_login_server");
	add_timer_interval(gettick() + 1000, check_connect_login_server, 0, 0, 10 * 1000);

	// keep the char-login connection alive
	add_timer_func_list(ping_login_server, "ping_login_server");
	add_timer_interval(gettick() + 1000, ping_login_server, 0, 0, ((int)stall_time-2) * 1000);

	// periodically update the overall user count on all mapservers + login server
	add_timer_func_list(broadcast_user_count, "broadcast_user_count");
	add_timer_interval(gettick() + 1000, broadcast_user_count, 0, 0, 5 * 1000);

	// send a list of all online account IDs to login server
	add_timer_func_list(send_accounts_tologin, "send_accounts_tologin");
	add_timer_interval(gettick() + 1000, send_accounts_tologin, 0, 0, 3600 * 1000); //Sync online accounts every hour

	// ???
	add_timer_func_list(chardb_waiting_disconnect, "chardb_waiting_disconnect");

	// ???
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 1000, online_data_cleanup, 0, 0, 600 * 1000);

	if( console )
	{
		//##TODO invoke a CONSOLE_START plugin event
	}

	char_fd = make_listen_bind(bind_ip, char_port);

	log_char("The char-server is ready (Server is listening on the port %d).\n", char_port);
	ShowStatus("The char-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", char_port);

	return 0;
}
