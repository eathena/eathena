// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_H_
#define _CHAR_H_

enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};

struct char_session_data {
	bool auth; // whether the session is authed or not
	int account_id, login_id1, login_id2, sex;
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int gmlevel;
};

#define CHAR_CONF_NAME  "conf/char_athena.conf"
#define LAN_CONF_NAME   "conf/subnet_athena.conf"
#define INTER_CONF_NAME "conf/inter_athena.conf"


int mapif_send(int fd,unsigned char *buf, unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf, unsigned int len);
int mapif_sendall(unsigned char *buf, unsigned int len);

int search_character_online(int aid, int cid);
void char_divorce(int partner_id1, int partner_id2);
int char_delete(int char_id);

// char config
extern char db_path[];
extern int char_name_option;
extern char char_name_letters[];
extern int log_char;
extern int log_inter;
extern int guild_exp_rate;
extern int save_log;
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

#endif /* _CHAR_H_ */
