// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_H_
#define _CHAR_H_

#include "../common/mmo.h" // MAX_CHARS

struct char_session_data {
	bool auth; // whether the session is authed or not
	int account_id, login_id1, login_id2, sex;
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int gmlevel;
	uint32 version;
	uint8 clienttype;
	int chars_num;// total number of characters in the account
	int slots[MAX_CHARS];// client view of the characters/slots (array of char_id's, 0=free)
};

#define CHAR_CONF_NAME  "conf/char_athena.conf"
#define LAN_CONF_NAME   "conf/subnet_athena.conf"
#define INTER_CONF_NAME "conf/inter_athena.conf"

void log_char(const char* fmt, ...);

int mapif_send(int fd,unsigned char *buf, unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf, unsigned int len);
int mapif_sendall(unsigned char *buf, unsigned int len);

void char_divorce(int partner_id1, int partner_id2);
int char_create(int account_id, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, int* out_char_id);
int char_delete(int char_id);

int char_married(int pl1,int pl2);
int char_child(int parent_id, int child_id);
int char_family(int cid1, int cid2, int cid3);

// char config
extern int guild_exp_rate;
extern int start_zeny;
extern int start_weapon;
extern int start_armor;
extern struct point start_point;

// auth system
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

struct Char_Config {

	char login_ip[128];   // login-server's address/hostname
	char char_ip[128];    // char-server's external address/hostname
	char bind_ip[128];    // char-server's physical address/hostname
	uint16 login_port;    // login-server's listen port
	uint16 char_port;     // char-server's listen port
	char userid[24];      // server-to-server userid
	char passwd[24];      // server-to-server password

	char db_path[1024];   // path to /db directory
	char server_name[20]; // server's display name
	int char_maintenance; // defines appearance in server select: 0 =  (%d)#, 1 =  (On the maintenance)#, 2 =  (%d People) - over the age 18#, 3 =  (%d players) - Pay to Play#, 4 =  (%d players) - Free Play Server#
	int char_new_display; // shows 'new' in server select if set
	bool char_new;        // new character creation enabled/disabled
	bool char_rename;     // when enabled, uses extended char data packet w/ char rename bit (kRO sakexe compatibility setting)
	int char_name_option; // letters/symbols authorized in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters)
	char char_name_letters[1024]; // list of letters/symbols authorised (or not) in a character name
	char wisp_server_name[NAME_LENGTH]; // name reserved for sending whispers from server to players
	char unknown_char_name[NAME_LENGTH]; // name to use when the requested name cannot be determined
	int max_connect_user; // maximum number of online users (0: unlimited)
	int gm_allow_level;   // minimum GM level that is allowed to bypass the server's max user limit
	bool console;         // console command parser enabled/disabled
	char char_log_filename[1024]; // path to charserver logfile
	bool log_char_enabled; // whether to log the charserver or not
	int chars_per_account; // maximum number of characters on a single account (0: unlimited)
	int char_del_level;   // char deletion prevention using base level (-n: can't delete <= n, +n: can't delete >= n)
};

extern struct Char_Config char_config;

#endif /* _CHAR_H_ */
