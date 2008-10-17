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
#ifdef TXT_ONLY 
#include "../common/lock.h"
#else
#include "../common/sql.h"
#endif
#include "char.h"
#include "chardb.h"
#include "charlog.h"
#include "inter.h"
#include "int_fame.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_pet.h"
#include "int_party.h"
#include "int_storage.h"
#include "int_status.h"

// chars database
CharDB* chars = NULL;


// temporary imports
extern int parse_fromlogin(int fd);
extern int parse_char(int fd);
#include "map.h"
extern char friends_txt[1024];
extern char hotkeys_txt[1024];
char char_txt[1024];
extern DBMap* char_db_;
extern int mmo_chars_tobuf(int account_id, uint8* buf);


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
int log_char = 1;	// loggin char or not [devil]
int log_inter = 1;	// loggin inter or not [devil]
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
#define AUTH_TIMEOUT 30000

struct auth_node {
	int account_id;
	int char_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	int sex;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int gmlevel;
};

DBMap* auth_db; // int account_id -> struct auth_node*

//-----------------------------------------------------
// Online User Database
//-----------------------------------------------------

struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server; // -2: unknown server, -1: not connected, 0+: id of server
};

DBMap* online_char_db; // int account_id -> struct online_char_data*
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
	struct mmo_charstatus *cp;
#endif
	
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

#ifndef TXT_ONLY
	//Set char online in guild cache. If char is in memory, use the guild id on it, otherwise seek it.
	cp = (struct mmo_charstatus*)idb_get(char_db_,char_id);
	inter_guild_CharOnline(char_id, cp?cp->guild_id:-1);
#endif

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
		struct mmo_charstatus* cp = (struct mmo_charstatus*)idb_get(char_db_,char_id);
		inter_guild_CharOffline(char_id, cp?cp->guild_id:-1);
		if (cp)
			idb_remove(char_db_,char_id);

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
	int j;

#ifndef TXT_ONLY
	if (save_log)
		ShowInfo("Loading Char Data ("CL_BOLD"%d"CL_RESET")\n",sd->account_id);
#endif

	j = 24; // offset
	WFIFOHEAD(fd, 4 + 20 + MAX_CHARS*108); // or 106(!)
	WFIFOW(fd,0) = 0x6b;
	memset(WFIFOP(fd,4), 0, 20); // unknown bytes
	j += mmo_chars_tobuf(sd->account_id, WFIFOP(fd,j));
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

	//login_log("Console command :%s\n", command);

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
int make_new_char(struct char_session_data* sd, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	struct mmo_charstatus cd;
	char name[NAME_LENGTH];
	int char_id;
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

#ifndef TXT_ONLY
	// check the number of already existing chars in this account
	if( char_per_account != 0 ) {
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` WHERE `account_id` = '%d'", char_db, sd->account_id) )
			Sql_ShowDebug(sql_handle);
		if( Sql_NumRows(sql_handle) >= char_per_account )
			return -2; // character account limit exceeded
	}
#endif

	// check char slot
	if( chars->slot2id(chars, sd->account_id, slot, &i) )
		return -2; // slot already in use

	// insert new char to database
	memset(&cd, 0, sizeof(cd));

	cd.char_id = -1;
	cd.account_id = sd->account_id;
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
	char_id = cd.char_id;

	// validation success, log result
#ifdef TXT_ONLY
	char_log("make new char: account: %d, slot %d, name: %s, stats: %d/%d/%d/%d/%d/%d, hair: %d, hair color: %d.\n",
	         sd->account_id, slot, name, str, agi, vit, int_, dex, luk, hair_style, hair_color);
#else
	if (log_char) {
		char esc_name[NAME_LENGTH*2+1];
		Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			charlog_db, "make new char", sd->account_id, slot, esc_name, str, agi, vit, int_, dex, luk, hair_style, hair_color) )
			Sql_ShowDebug(sql_handle);
	}
#endif

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", sd->account_id, char_id, slot, name);
	return char_id;
}


void char_divorce(int partner_id1, int partner_id2)
{
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


int request_accreg2(int account_id, int char_id)
{
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,10);
		WFIFOW(login_fd,0) = 0x272e;
		WFIFOL(login_fd,2) = account_id;
		WFIFOL(login_fd,6) = char_id;
		WFIFOSET(login_fd,10);
		return 1;
	}
	return 0;
}

//Send packet forward to login-server for account saving
int save_accreg2(unsigned char* buf, int len)
{
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,len+4);
		memcpy(WFIFOP(login_fd,4), buf, len);
		WFIFOW(login_fd,0) = 0x2728;
		WFIFOW(login_fd,2) = len+4;
		WFIFOSET(login_fd,len+4);
		return 1;
	}
	return 0;
}


// Send the fame ranking lists to map-server(s)
// S 2b1b <len>.w <bs len>.w <alch len>.w <tk len>.w <bs data>.?b<alch data>.?b <tk data>.?b
int char_send_fame_list(int fd)
{
	unsigned char buf[3 * MAX_FAME_LIST * sizeof(struct fame_list)];
	int len = 10;
	
	WBUFW(buf,0) = 0x2b1b;
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


void inter_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	ShowInfo("Reading file %s...\n", cfgName);

	if ((fp = fopen(cfgName, "r")) == NULL) {
		ShowError("file not found: %s\n", cfgName);
		return;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

#ifndef TXT_ONLY
		if(!strcmpi(w1,"char_db"))
			strcpy(char_db,w2);
		else if(!strcmpi(w1,"scdata_db"))
			strcpy(scdata_db,w2);
		else if(!strcmpi(w1,"cart_db"))
			strcpy(cart_db,w2);
		else if(!strcmpi(w1,"inventory_db"))
			strcpy(inventory_db, w2);
		else if(!strcmpi(w1,"storage_db"))
			strcpy(storage_db,w2);
		else if(!strcmpi(w1,"reg_db"))
			strcpy(reg_db,w2);
		else if(!strcmpi(w1,"skill_db"))
			strcpy(skill_db,w2);
		else if(!strcmpi(w1,"interlog_db"))
			strcpy(interlog_db,w2);
		else if(!strcmpi(w1,"memo_db"))
			strcpy(memo_db,w2);
		else if(!strcmpi(w1,"guild_db"))
			strcpy(guild_db,w2);
		else if(!strcmpi(w1,"guild_alliance_db"))
			strcpy(guild_alliance_db,w2);
		else if(!strcmpi(w1,"guild_castle_db"))
			strcpy(guild_castle_db,w2);
		else if(!strcmpi(w1,"guild_expulsion_db"))
			strcpy(guild_expulsion_db,w2);
		else if(!strcmpi(w1,"guild_member_db"))
			strcpy(guild_member_db,w2);
		else if(!strcmpi(w1,"guild_skill_db"))
			strcpy(guild_skill_db,w2);
		else if(!strcmpi(w1,"guild_position_db"))
			strcpy(guild_position_db,w2);
		else if(!strcmpi(w1,"guild_storage_db"))
			strcpy(guild_storage_db,w2);
		else if(!strcmpi(w1,"party_db"))
			strcpy(party_db,w2);
		else if(!strcmpi(w1,"pet_db"))
			strcpy(pet_db,w2);
		else if(!strcmpi(w1,"mail_db"))
			strcpy(mail_db,w2);
		else if(!strcmpi(w1,"auction_db"))
			strcpy(auction_db,w2);
		else if(!strcmpi(w1,"friend_db"))
			strcpy(friend_db,w2);
		else if(!strcmpi(w1,"hotkey_db"))
			strcpy(hotkey_db,w2);
		else if(!strcmpi(w1,"quest_db"))
			strcpy(quest_db,w2);
		else if(!strcmpi(w1,"quest_obj_db"))
			strcpy(quest_obj_db, w2);
#endif
		else if( charlog_config_read(w1,w2) )
			continue;
		//support the import command, just like any other config
		else if(!strcmpi(w1,"import"))
			inter_config_read(w2);
	}
	fclose(fp);
	ShowInfo("Done reading %s.\n", cfgName);
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
//		} else if (strcmpi(w1, "scdata_txt") == 0) { //By Skotlex
//			strcpy(scdata_txt, w2);
		} else if (strcmpi(w1, "char_txt") == 0) {
			strcpy(char_txt, w2);
		} else if (strcmpi(w1, "friends_txt") == 0) { //By davidsiaw
			strcpy(friends_txt, w2);
		} else if (strcmpi(w1, "hotkeys_txt") == 0) { //By davidsiaw
			strcpy(hotkeys_txt, w2);
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
		} else if(strcmpi(w1,"log_char")==0) {		//log char or not [devil]
			log_char = atoi(w2);
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
		else if( fame_config_read(w1,w2) )
			continue;
		else if( charlog_config_read(w1,w2) )
			continue;
		else if (strcmpi(w1, "import") == 0)
			char_config_read(w2);
	}
	fclose(fp);
	
	ShowInfo("Done reading %s.\n", cfgName);
	return 0;
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
	
	chars->destroy(chars);

	if (login_fd > 0)
		do_close(login_fd);
	if (char_fd > 0)
		do_close(char_fd);

	inter_final();
	mapindex_final();

	char_log("----End of char-server (normal end with closing of all files).\n");
	charlog_final();

#else

	//check SQL save progress.
	//wait until save char complete

	set_all_offline(-1);
	set_all_offline_sql();

	inter_final();

	flush_fifos();

	mapindex_final();

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `ragsrvinfo`") )
		Sql_ShowDebug(sql_handle);

	if (login_fd > 0)
		do_close(login_fd);
	if (char_fd > 0)
		do_close(char_fd);
	char_db_->destroy(char_db_, NULL);
	online_char_db->destroy(online_char_db, NULL);
	auth_db->destroy(auth_db, NULL);

	Sql_Free(sql_handle);

	charlog_final();
#endif

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

	// intialize engines (to accept config settings)
#ifdef TXT_ONLY
	chars = char_db_txt();
#else
	chars = char_db_sql();
#endif

	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_lan_config_read((argc > 3) ? argv[3] : LAN_CONF_NAME);

	if (strcmp(userid, "s1")==0 && strcmp(passwd, "p1")==0) {
		ShowError("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your save/account.txt file to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("And then change the user/password to use in conf/char_athena.conf (or conf/import/char_conf.txt)\n");
	}

	ShowInfo("Finished reading the char-server configuration.\n");

	inter_config_read(INTER_CONF_NAME);
	inter_init();
	ShowInfo("Finished reading the inter-server configuration.\n");

	// chars database init
	chars->init(chars);

	charlog_init();
	// a newline in the log...
	char_log("");
	// moved behind char_config_read in case we changed the filename [celest]
	char_log("The char-server starting...\n");

	ShowInfo("Initializing char server.\n");
	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);
	online_char_db = idb_alloc(DB_OPT_RELEASE_DATA);

	char_read_fame_list(); //Read fame lists.
	ShowInfo("char server initialized.\n");

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

	ShowInfo("End of char server initilization function.\n");

	char_fd = make_listen_bind(bind_ip, char_port);
#ifdef TXT_ONLY
	char_log("The char-server is ready (Server is listening on the port %d).\n", char_port);
#endif
	ShowStatus("The char-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", char_port);

	return 0;
}
