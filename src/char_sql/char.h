

#ifndef _CHAR_H_
#define _CHAR_H_

#include "mmo.h"



#define MAX_MAP_SERVERS 30

#define LAN_CONF_NAME	"conf/lan_support.conf"

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000

struct itemtmp {
	int flag;//checked = 1 else 0
	int id;
	short nameid;
	short amount;
	unsigned short equip;
	char identify;
	char refine;
	char attribute;
	short card[4];
};
enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};
struct itemtemp{
	struct itemtmp equip[MAX_GUILD_STORAGE],notequip[MAX_GUILD_STORAGE];
};
int memitemdata_to_sql(struct itemtmp mapitem[], int count, int char_id,int tableswitch);
int mapif_sendall(unsigned char *buf,unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf,unsigned int len);
int mapif_send(int fd,unsigned char *buf,unsigned int len);

int char_nick2id (char *name);
int char_married(int pl1,int pl2);
int char_child(int parent_id, int child_id);

extern int autosave_interval;
extern char db_path[];
extern char char_db[256];
extern char cart_db[256];
extern char inventory_db[256];
extern char charlog_db[256];
extern char storage_db[256];
extern char interlog_db[256];
extern char reg_db[256];
extern char skill_db[256];
extern char memo_db[256];
extern char guild_db[256];
extern char guild_alliance_db[256];
extern char guild_castle_db[256];
extern char guild_expulsion_db[256];
extern char guild_member_db[256];
extern char guild_position_db[256];
extern char guild_skill_db[256];
extern char guild_storage_db[256];
extern char party_db[256];
extern char pet_db[256];

extern int db_use_sqldbs; // added for sql item_db read for char server [Valaris]
extern char login_db_level[32];
extern char login_db_account_id[32];

extern int lowest_gm_level;
extern size_t GM_num;
extern struct gm_account *gm_account;


extern int party_modus;


#endif//_CHAR_H_

