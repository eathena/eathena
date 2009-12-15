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
#include "if_client.h"
#include "if_login.h"
#include "if_map.h"
#include "inter.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_mercenary.h"
#include "int_party.h"
#include "int_pet.h"
#include "int_rank.h"
#include "int_registry.h"
#include "int_status.h"
#include "int_storage.h"
#include "charserverdb.h"
#include "online.h"
#include <string.h>

// CharServerDB engines available
static struct{
	CharServerDB* (*constructor)(void);
	CharServerDB* engine;
} charserver_engines[] = {
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
// standard engines
#ifdef WITH_SQL
	{charserver_db_sql, NULL},
#endif
#ifdef WITH_TXT
	{charserver_db_txt, NULL},
#endif
};

// charserver engine
CharServerDB* charserver = NULL;

// charserver configuration
struct Char_Config char_config;

// charserver logging
static bool log_char_enabled = false;


int char_fd=-1;
uint32 login_ip = 0;
uint32 char_ip = 0;
uint32 bind_ip = INADDR_ANY;
char charserver_engine[256] = "auto";// name of the engine to use (defaults to auto, for the first valid engine)


// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];
int subnet_count = 0;

// storage-specific options
//TXT
int email_creation = 0; // disabled by default


//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
DBMap* auth_db = NULL; // int account_id -> struct auth_node*


/// Returns the number of online players in all map-servers.
int count_users(void)
{
	int i, users;

	users = 0;
	for( i = 0; i < MAX_MAP_SERVERS; i++ )
		if( server[i].fd > 0 )
			users += server[i].users;

	return users;
}


void char_auth_ok(int fd, struct char_session_data *sd)
{
	struct online_char_data* character;
	character = onlinedb_get(sd->account_id);
	if( character != NULL )
	{	// check if character is not online already. [Skotlex]
		if (character->server > -1)
		{	//Character already online. KICK KICK KICK
			mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
			set_char_waitdisconnect(sd->account_id, 20000);
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

	loginif_request_account_data(sd->account_id);

	// mark session as 'authed'
	sd->auth = true;

	// set char online on charserver
	set_char_charselect(sd->account_id);

	// continues when account data is received...
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

	// send number of players to login server
	loginif_user_count(users);

	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf,6);

	onlinedb_sync(); // refresh online files (txt and html)

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


/// Validates a character name and checks if it's available.
int check_char_name(const char* name)
{
	CharDB* chars = charserver->chardb(charserver);
	int i;

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// check for reserved names
	if( strcmpi(name, main_chat_nick) == 0 || strcmpi(name, char_config.wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// check content of character name
	for( i = 0; i < NAME_LENGTH && name[i]; i++ )
		if( ISCNTRL(name[i]) )
			return -2; // control chars in name

	// Check Authorised letters/symbols in the name of the character
	if( char_config.char_name_option == 1 ) { // only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_config.char_name_letters, name[i]) == NULL )
				return -2;
	}
	else
	if( char_config.char_name_option == 2 ) { // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_config.char_name_letters, name[i]) != NULL )
				return -2;
	} // else, all letters/symbols are authorised (except control char removed before)

	// check name (already in use?)
	if( chars->name2id(chars, name, char_config.character_name_case_sensitive, NULL, NULL, NULL) )
		return -1; // name already exists

	return 0;
}


//-----------------------------------
// Function to create a new character
//-----------------------------------
int char_create(int account_id, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, int* out_char_id)
{
	CharDB* chars = charserver->chardb(charserver);
	StorageDB* storages = charserver->storagedb(charserver);
	struct mmo_charstatus cd;
	char name[NAME_LENGTH];
	int result;

	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name, "\032\t\x0A\x0D "); //The following characters are always substituted and trimmed because they cause confusion and problems on the servers. [Skotlex]

	result = check_char_name(name);
	if( result < 0 )
		return result;

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
	cd.zeny = char_config.start_zeny;
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
	memcpy(&cd.last_point, &char_config.start_point, sizeof(char_config.start_point));
	memcpy(&cd.save_point, &char_config.start_point, sizeof(char_config.start_point));

	if( char_config.start_weapon > 0 )
	{// add start weapon (Knife?)
		cd.inventory[0].nameid = char_config.start_weapon;
		cd.inventory[0].amount = 1;
		cd.inventory[0].identify = 1;
		cd.inventory[0].equip = 0x0002; // EQP_HAND_R
	}
	if( char_config.start_armor > 0 )
	{// add start armor (Cotton Shirt?)
		cd.inventory[1].nameid = char_config.start_armor;
		cd.inventory[1].amount = 1;
		cd.inventory[1].identify = 1;
		cd.inventory[1].equip = 0x0010; // EQP_ARMOR
	}

	if( !chars->create(chars, &cd) )
		return -2; // abort

	if( !storages->save(storages, cd.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd.char_id) )
	{
		chars->remove(chars, cd.char_id); // uhh... problem.
		return -2; // abort
	}

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
	StorageDB* storages = charserver->storagedb(charserver);
	struct mmo_charstatus cd1, cd2;
	unsigned char buf[10];
	int i;

	if( !chars->load_num(chars, &cd1, partner_id1) || !chars->load_num(chars, &cd2, partner_id2) )
		return; // char not found

	if( cd1.partner_id != cd2.char_id || cd2.partner_id != cd1.char_id )
		return; // not married to each other

	if( !storages->load(storages, cd1.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd1.char_id)
	||  !storages->load(storages, cd2.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd2.char_id) )
		return;

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
	storages->save(storages, cd1.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd1.char_id);
	storages->save(storages, cd2.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd2.char_id);

	// notify all mapservers
	WBUFW(buf,0) = 0x2b12;
	WBUFL(buf,2) = partner_id1;
	WBUFL(buf,6) = partner_id2;
	mapif_sendall(buf,10);
}


int char_delete(int char_id)
{
	CharDB* chars = charserver->chardb(charserver);
	CharRegDB* charregs = charserver->charregdb(charserver);
	FriendDB* friends = charserver->frienddb(charserver);
	HotkeyDB* hotkeys = charserver->hotkeydb(charserver);
	HomunDB* homuns = charserver->homundb(charserver);
	MemoDB* memos = charserver->memodb(charserver);
	MercDB* mercs = charserver->mercdb(charserver);
	QuestDB* quests = charserver->questdb(charserver);
	SkillDB* skills = charserver->skilldb(charserver);
	StatusDB* statuses = charserver->statusdb(charserver);
	StorageDB* storages = charserver->storagedb(charserver);
	struct mmo_charstatus cd;
	int i;

	if( !chars->load_num(chars, &cd, char_id) )
	{
		ShowError("char_delete: Unable to fetch data for char %d, deletion aborted.\n", char_id);
		return 1;
	}

	//check for config char del condition [Lupus]
	if( ( char_config.char_del_level > 0 && cd.base_level >= (unsigned int)(char_config.char_del_level) )
	 || ( char_config.char_del_level < 0 && cd.base_level <= (unsigned int)(-char_config.char_del_level) )
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
		homuns->remove(homuns, cd.hom_id);

	// remove mercenary data
	if( cd.mer_id )
		mercs->remove(mercs, cd.mer_id);

	// leave party
	if( cd.party_id )
		inter_party_leave(cd.party_id, cd.account_id, cd.char_id);

	// leave/break guild
	if( cd.guild_id )
		inter_guild_leave(cd.guild_id, cd.account_id, cd.char_id);

	// divorce
	if( cd.partner_id )
		char_divorce(cd.char_id, cd.partner_id);

	// De-adopt
	if( cd.father || cd.mother )
	{// Char is Baby
		unsigned char buf[64];
		struct mmo_charstatus fd, md;

		if( cd.father && chars->load_num(chars, &fd, cd.father) && skills->load(skills, &fd.skill, cd.father) )
		{
			fd.child = 0;
			fd.skill[410].id = 0;
			fd.skill[410].lv = 0;
			fd.skill[410].flag = 0;
			chars->save(chars, &fd);
			skills->save(skills, &fd.skill, cd.father);
		}
		if( cd.mother && chars->load_num(chars, &md, cd.mother) && skills->load(skills, &md.skill, cd.mother) )
		{
			md.child = 0;
			md.skill[410].id = 0;
			md.skill[410].lv = 0;
			md.skill[410].flag = 0;
			chars->save(chars, &md);
			skills->save(skills, &md.skill, cd.mother);
		}

		WBUFW(buf,0) = 0x2b25;
		WBUFL(buf,2) = cd.father;
		WBUFL(buf,6) = cd.mother;
		WBUFL(buf,10) = char_id; // Baby
		mapif_sendall(buf,14);
	}

	// delete inventory
	storages->remove(storages, STORAGE_INVENTORY, cd.char_id);

	// delete cart
	storages->remove(storages, STORAGE_CART, cd.char_id);

	// delete skill list
	skills->remove(skills, cd.char_id);

	// delete memo points
	memos->remove(memos, cd.char_id);

	// clear status changes
	statuses->remove(statuses, cd.char_id);

	// delete character registry
	charregs->remove(charregs, cd.char_id);

	// delete friends list
	friends->remove(friends, cd.char_id);

	// delete hotkeys list
	hotkeys->remove(hotkeys, cd.char_id);

	// delete quest list
	quests->remove(quests, cd.char_id);

	// delete the character and all associated data
	chars->remove(chars, cd.char_id);

	charlog_log(cd.char_id, cd.account_id, cd.slot, cd.name, "Deleted char");

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


/// Records an event in the charserver log
void log_char(const char* fmt, ...)
{
	char timestamp[24+1];
	time_t now;
	FILE* log_fp;
	va_list ap;

	if( !log_char_enabled )
		return;

	// open log file
	log_fp = fopen(char_config.char_log_filename, "a");
	if( log_fp == NULL )
		return;

	// write timestamp to log file
	time(&now);
	strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&now));
	fprintf(log_fp, "%s\t", timestamp);

	// write formatted message to log file
	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);

	// close log file
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


void char_set_defaults(void)
{
	safestrncpy(char_config.login_ip, "", sizeof(char_config.login_ip));
	safestrncpy(char_config.char_ip, "", sizeof(char_config.char_ip));
	safestrncpy(char_config.bind_ip, "", sizeof(char_config.bind_ip));
	char_config.login_port = 6900;
	char_config.char_port = 6121;
	safestrncpy(char_config.userid, "s1", sizeof(char_config.userid));
	safestrncpy(char_config.passwd, "p1", sizeof(char_config.passwd));
	safestrncpy(char_config.db_path, "db", sizeof(char_config.db_path));
	safestrncpy(char_config.server_name, "", sizeof(char_config.server_name));
	char_config.char_maintenance = 0;
	char_config.char_new_display = 0;
	char_config.char_new = true;
	char_config.char_rename = true;
	char_config.char_name_option = 0;
	safestrncpy(char_config.char_name_letters, "", sizeof(char_config.char_name_letters));
	safestrncpy(char_config.wisp_server_name, "Server", sizeof(char_config.wisp_server_name));
	safestrncpy(char_config.unknown_char_name, "Unknown", sizeof(char_config.unknown_char_name));
	char_config.max_connect_user = 0;
	char_config.gm_allow_level = 99;
	char_config.console = false;
	safestrncpy(char_config.char_log_filename, "log/char.log", sizeof(char_config.char_log_filename));
	char_config.log_char_enabled = true;
	char_config.chars_per_account = 0;
	char_config.char_del_level = 0;
	char_config.character_name_case_sensitive = true;
	char_config.start_zeny = 500;
	char_config.start_weapon = 1201;
	char_config.start_armor = 2301;
	char_config.start_point.map = mapindex_name2id(MAP_NOVICE);
	char_config.start_point.x = 53;
	char_config.start_point.y = 111;
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

		if( strcmpi(w1,"timestamp_format") == 0 )
			strncpy(timestamp_format, w2, 20);
		else
		if( strcmpi(w1,"console_silent") == 0 )
		{
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
		}
		else
		if( strcmpi(w1,"stdout_with_ansisequence") == 0 )
			stdout_with_ansisequence = config_switch(w2);
		else
		if( strcmpi(w1, "userid") == 0 )
			safestrncpy(char_config.userid, w2, sizeof(char_config.userid));
		else
		if( strcmpi(w1, "passwd") == 0 )
			safestrncpy(char_config.passwd, w2, sizeof(char_config.passwd));
		else
		if( strcmpi(w1, "server_name") == 0 )
		{
			safestrncpy(char_config.server_name, w2, sizeof(char_config.server_name));
			ShowStatus("%s server has been initialized\n", w2);
		}
		else
		if( strcmpi(w1, "wisp_server_name") == 0 )
			safestrncpy(char_config.wisp_server_name, w2, sizeof(char_config.wisp_server_name));
		else
		if( strcmpi(w1, "login_ip") == 0 )
		{
			char ip_str[16];
			login_ip = host2ip(w2);
			if (login_ip) {
				safestrncpy(char_config.login_ip, w2, sizeof(char_config.login_ip));
				ShowStatus("Login server IP address : %s -> %s\n", w2, ip2str(login_ip, ip_str));
			}
		}
		else
		if( strcmpi(w1, "login_port") == 0 )
			char_config.login_port = (uint16)atoi(w2);
		else
		if( strcmpi(w1, "char_ip") == 0 )
		{
			char ip_str[16];
			char_ip = host2ip(w2);
			if (char_ip){
				safestrncpy(char_config.char_ip, w2, sizeof(char_config.char_ip));
				ShowStatus("Character server IP address : %s -> %s\n", w2, ip2str(char_ip, ip_str));
			}
		}
		else
		if( strcmpi(w1, "bind_ip") == 0 )
		{
			char ip_str[16];
			bind_ip = host2ip(w2);
			if (bind_ip) {
				safestrncpy(char_config.bind_ip, w2, sizeof(char_config.bind_ip));
				ShowStatus("Character server binding IP address : %s -> %s\n", w2, ip2str(bind_ip, ip_str));
			}
		}
		else
		if( strcmpi(w1, "char_port") == 0 )
			char_config.char_port = (uint16)atoi(w2);
		else
		if( strcmpi(w1, "char_maintenance") == 0 )
			char_config.char_maintenance = atoi(w2);
		else
		if( strcmpi(w1, "char_new") == 0 )
			char_config.char_new = (bool)atoi(w2);
		else
		if( strcmpi(w1, "char_new_display") == 0 )
			char_config.char_new_display = atoi(w2);
#ifdef TXT_ONLY
		else
		if( strcmpi(w1, "email_creation") == 0 )
			email_creation = config_switch(w2);
#endif
		else
		if( strcmpi(w1, "max_connect_user") == 0 )
		{
			char_config.max_connect_user = atoi(w2);
			if (char_config.max_connect_user < 0)
				char_config.max_connect_user = 0; // unlimited online players
		}
		else
		if( strcmpi(w1, "gm_allow_level") == 0 )
		{
			char_config.gm_allow_level = atoi(w2);
			if(char_config.gm_allow_level < 0)
				char_config.gm_allow_level = 99;
		}
		else
		if( strcmpi(w1, "start_point") == 0 )
		{
			char map[MAP_NAME_LENGTH_EXT];
			int x, y;
			if (sscanf(w2, "%15[^,],%d,%d", map, &x, &y) < 3)
				continue;
			char_config.start_point.map = mapindex_name2id(map);
			char_config.start_point.x = x;
			char_config.start_point.y = y;
		}
		else
		if( strcmpi(w1, "start_zeny") == 0 )
			char_config.start_zeny = atoi(w2);
		else
		if( strcmpi(w1, "start_weapon") == 0 )
			char_config.start_weapon = atoi(w2);
		else
		if( strcmpi(w1, "start_armor") == 0 )
			char_config.start_armor = atoi(w2);
		else
		if( strcmpi(w1,"log_char") == 0 )
			log_char_enabled = config_switch(w2);
		else
		if( strcmpi(w1, "char_log_filename") == 0 )
			safestrncpy(char_config.char_log_filename, w2, sizeof(char_config.char_log_filename));
		else
		if( strcmpi(w1, "unknown_char_name") == 0 )
			safestrncpy(char_config.unknown_char_name, w2, sizeof(char_config.unknown_char_name));
		else
		if( strcmpi(w1, "char_name_option") == 0 )
			char_config.char_name_option = atoi(w2);
		else
		if( strcmpi(w1, "char_name_letters") == 0 )
			safestrncpy(char_config.char_name_letters, w2, sizeof(char_config.char_name_letters));
		else
		if( strcmpi(w1, "char_rename") == 0 )
			char_config.char_rename = (bool)config_switch(w2);
		else
		if( strcmpi(w1, "db_path") == 0 )
			safestrncpy(char_config.db_path, w2, sizeof(char_config.db_path));
		else
		if( strcmpi(w1, "chars_per_account") == 0 )
			char_config.chars_per_account = atoi(w2);
		else
		if( strcmpi(w1, "char_del_level") == 0 )
			char_config.char_del_level = atoi(w2);
		else
		if( strcmpi(w1, "console") == 0 )
			char_config.console = config_switch(w2);
		else
		if( strcmpi(w1, "character.name.case_sensitive") == 0 )
			char_config.character_name_case_sensitive = (bool)atoi(w2);
		else
		if( rank_config_read(w1,w2) )
			continue;
		else
		if( strcmpi(w1, "import") == 0 )
			char_config_read(w2);
		else
		if( strcmpi(w1, "charserver.engine") == 0 )
			safestrncpy(charserver_engine, w2, sizeof(charserver_engine));
		else
		{// try the charserver engines
			int i;
			for( i = 0; i < ARRAYLENGTH(charserver_engines); ++i )
			{
				CharServerDB* engine = charserver_engines[i].engine;
				if( engine )
					engine->set_property(engine, w1, w2);
			}
			// try others
			onlinedb_config_read(w1,w2);
			charlog_config_read(w1,w2);
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

	for( i = 0; i < ARRAYLENGTH(charserver_engines); ++i )
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

	ShowStatus("Terminating...\n");

	set_all_offline();

	inter_final();

	onlinedb_final();
	auth_db->destroy(auth_db, NULL);

	flush_fifos();

	do_final_mapif();
	do_final_loginif();

	if( char_fd != -1 )
	{
		do_close(char_fd);
		char_fd = -1;
	}

	mapindex_final();

	log_char("----End of char-server (normal shutdown).\n");

	for( i = 0; i < ARRAYLENGTH(charserver_engines); ++i )
	{// destroy all charserver engines
		CharServerDB* engine = charserver_engines[i].engine;
		if( engine )
		{
			engine->destroy(engine);
			charserver_engines[i].engine = NULL;
		}
	}
	charserver = NULL; // destroyed in charserver_engines

	charlog_final();

	ShowStatus("Finished.\n");
}


/// Called when a terminate signal is received.
void do_shutdown(void)
{
	if( runflag != CHARSERVER_ST_SHUTDOWN )
	{
		int id;
		runflag = CHARSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		// TODO proper shutdown procedure; wait for acks?, kick all characters, ... [FlavoJS]
		for( id = 0; id < ARRAYLENGTH(server); ++id )
			mapif_server_reset(id);
		loginif_check_shutdown();
		flush_fifos();
		runflag = CORE_ST_STOP;
	}
}


int do_init(int argc, char **argv)
{
	int i;

	//Read map indexes
	mapindex_init();

	// create engines (to accept config settings)
	for( i = 0; i < ARRAYLENGTH(charserver_engines); ++i )
		if( charserver_engines[i].constructor )
			charserver_engines[i].engine = charserver_engines[i].constructor();

	onlinedb_create();
	charlog_create();

	char_set_defaults();
	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_lan_config_read((argc > 3) ? argv[3] : LAN_CONF_NAME);

	if( strcmp(char_config.userid, "s1")==0 && strcmp(char_config.passwd, "p1") == 0 )
	{
		ShowWarning("Using the default user/password s1/p1 is a SECURITY RISK.\n");
		ShowNotice("Please change the username and password of your server account\n");
		ShowNotice("and set the userid/passwd settings in char_athena.conf accordingly.\n");
	}

	ShowInfo("Initializing char server.\n");
	log_char("The char-server is starting...\n");

	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if( !init_charserver_engine() )
		exit(EXIT_FAILURE);

	inter_init(charserver);
	onlinedb_init();
	charlog_init();

	if ((naddr_ != 0) && (!login_ip || !char_ip))
	{
		uint32 ip = addr_[0];
		char ip_str[16];
		ip2str(ip, ip_str);

		if (naddr_ > 1)
			ShowStatus("Multiple interfaces detected..  using %s as our IP address\n", ip_str);
		else
			ShowStatus("Defaulting to %s as our IP address\n", ip_str);

		if (!login_ip) {
			safestrncpy(char_config.login_ip, ip_str, sizeof(char_config.login_ip));
			login_ip = ip;
		}
		if (!char_ip) {
			safestrncpy(char_config.char_ip, ip_str, sizeof(char_config.char_ip));
			char_ip = ip;
		}
	}

	do_init_loginif();
	do_init_mapif();

	// periodically update the overall user count on all mapservers + login server
	add_timer_func_list(broadcast_user_count, "broadcast_user_count");
	add_timer_interval(gettick() + 1000, broadcast_user_count, 0, 0, 5 * 1000);

	if( char_config.console )
	{
		//##TODO invoke a CONSOLE_START plugin event
	}

	set_defaultparse(parse_client);
	char_fd = make_listen_bind(bind_ip, char_config.char_port);

	log_char("The char-server is ready (Server is listening on the port %d).\n", char_config.char_port);
	ShowStatus("The char-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", char_config.char_port);

	if( runflag != CORE_ST_STOP )
	{
		shutdown_callback = do_shutdown;
		runflag = CHARSERVER_ST_RUNNING;
	}

	return 0;
}
