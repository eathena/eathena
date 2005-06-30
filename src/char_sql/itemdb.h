#ifndef _ITEMDB_H_
#define _ITEMDB_H_
#include "mmo.h"

struct item_data {
	unsigned short nameid;
	char name[24],jname[24];
	int value_buy,value_sell,value_notdc,value_notoc;
	int type;
	int class_;
	int sex;
	int equip;
	int weight;
	int atk;
	int def;
	int range;
	int slot;
	int look;
	int elv;
	int wlv;
	char *use_script;	// ‰ñ•œ‚Æ‚©‚à‘S•”‚±‚Ì’†‚Å‚â‚ë‚¤‚©‚È‚Æ
	char *equip_script;	// UŒ‚,–hŒä‚Ì‘®«Ý’è‚à‚±‚Ì’†‚Å‰Â”\‚©‚È?
	char available;
};

struct item_data* itemdb_search(unsigned short nameid);
#define itemdb_type(n) itemdb_search(n)->type

bool itemdb_isSingleStorage(unsigned short nameid);
bool itemdb_isSingleStorage(struct item_data &data);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif
