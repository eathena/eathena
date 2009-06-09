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

int search_character_online(int aid, int cid);
void char_divorce(int partner_id1, int partner_id2);
int char_create(int account_id, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, int* out_char_id);
int char_delete(int char_id);

int char_married(int pl1,int pl2);
int char_child(int parent_id, int child_id);
int char_family(int cid1, int cid2, int cid3);

// char config
extern char db_path[];
extern int char_name_option;
extern char char_name_letters[];
extern int guild_exp_rate;
extern char server_name[20];
extern char wisp_server_name[NAME_LENGTH];
extern char unknown_char_name[NAME_LENGTH];
extern int char_per_account;
extern int char_del_level;
extern int start_zeny;
extern int start_weapon;
extern int start_armor;
extern struct point start_point;
extern int autosave_interval;

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

// online user tracking system
struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server; // -2: unknown server, -1: not connected, 0+: id of server
};

struct Char_Config {

	char login_ip[128];   // login-server's address/hostname
	char char_ip[128];    // char-server's external address/hostname
	char bind_ip[128];    // char-server's physical address/hostname
	uint16 login_port;    // login-server's listen port
	uint16 char_port;     // char-server's listen port
	char userid[24];      // server-to-server userid
	char passwd[24];      // server-to-server password

	int char_maintenance; // defines appearance in server select: 0 =  (%d)#, 1 =  (On the maintenance)#, 2 =  (%d People) - over the age 18#, 3 =  (%d players) - Pay to Play#, 4 =  (%d players) - Free Play Server#
	int char_new_display; // shows 'new' in server select if set
	bool char_new;        // new character creation enabled/disabled
	bool char_rename;     // when enabled, uses extended char data packet w/ char rename bit (kRO sakexe compatibility setting)
	bool online_check;    // when enabled, access to an already online account is rejected and the mapserver is asked to disconnect the corresponding character
};

extern struct Char_Config char_config;

#endif /* _CHAR_H_ */
