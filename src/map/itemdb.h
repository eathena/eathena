// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "map.h"

struct item_data
{
	unsigned short nameid;
	char name[24];
	char jname[24];
	char prefix[24];
	char suffix[24];
	char cardillustname[64];
	long value_buy;
	long value_sell;

	uint32 weight;
	uint32 atk;
	uint32 def;

	unsigned char type;
	//0 Healing, 2: Usable, 3: Misc, 4: Weapon, 5: Armor, 6: Card, 7: Pet Egg,
	//8: Pet Equipment, 10: Arrow, 11: Usable with delayed consumption (all items with script "pet" or "itemskill": Lures, Scrolls, Magnifier, Yggdrasil Leaf)

	unsigned char gm_lv_trade_override;

	uint32 class_array;
	unsigned short equip;
	unsigned short range;
	unsigned short look;
	unsigned short elv;
	unsigned short wlv;
	unsigned short view_id;

	struct _flags {
		unsigned available : 1;			// 0
		unsigned value_notdc : 1;		// 1
		unsigned value_notoc : 1;		// 2
		unsigned no_use : 1;			// 3
		unsigned sex : 2;				// 4,5 // male=0, female=1, all=2
		unsigned slot : 3;				// 6,7,8 (0,1,2,3,4 [5..7 unused]
		unsigned no_refine : 1;			// 9 [celest]
		unsigned no_equip : 3;			// 10,11,12
		unsigned delay_consume : 1;		// 13 Signifies items that are not consumed inmediately upon double-click [Skotlex]
		unsigned trade_restriction : 7;	// 14-20 Item restrictions mask [Skotlex]
		unsigned _unused : 3;			// 21-23

		_flags() :
			available(0),
			value_notdc(0),
			value_notoc(0),
			no_use(0),
			sex(0),
			slot(0),
			no_refine(0),
			no_equip(0),
			delay_consume(0),
			trade_restriction(0)
		{}
	} flag;

	script_object* use_script;		// 回復とかも全部この中でやろうかなと
	script_object* equip_script;	// 攻撃,防御の属性設定もこの中で可能かな?


	unsigned char getType()	{ return (type==7) ? 4 : type; } // pet eggs are handled as weapons by the client

	item_data(unsigned short nid=0) : 
		nameid(nid),
		value_buy(0),
		value_sell(0),
		weight(0),
		atk(0),
		def(0),
		type(0),
		gm_lv_trade_override(0),
		class_array(0),
		equip(0),
		range(0),
		look(0),
		elv(0),
		wlv(0),
		view_id(0),
		use_script(NULL),
		equip_script(NULL)
	{
		name[0]=0;
		jname[0]=0;
		prefix[0]=0;
		suffix[0]=0;
		cardillustname[0]=0;
	}
};


struct random_item_data {
	unsigned short nameid;
	unsigned short per;
};

struct item_group {
	unsigned short nameid[30];	// 60 bytes
};

struct item_data* itemdb_searchname(const char *name);
struct item_data* itemdb_search(unsigned short nameid);
struct item_data* itemdb_exists(unsigned short nameid);
#define itemdb_type(n) itemdb_search(n)->type
#define itemdb_atk(n) itemdb_search(n)->atk
#define itemdb_def(n) itemdb_search(n)->def
#define itemdb_look(n) itemdb_search(n)->look
#define itemdb_weight(n) itemdb_search(n)->weight
#define itemdb_equip(n) itemdb_search(n)->equip

inline const char* itemdb_usescript(unsigned short n)
{
	item_data* it = itemdb_exists(n);
	return ( it && it->use_script && it->use_script->script )?it->use_script->script:NULL; 
}
inline const char* itemdb_equipscript(unsigned short n)
{
	item_data* it = itemdb_exists(n);
	return ( it && it->equip_script && it->equip_script->script )?it->equip_script->script:NULL; 
}

#define itemdb_wlv(n) itemdb_search(n)->wlv
#define itemdb_range(n) itemdb_search(n)->range
#define itemdb_slot(n) itemdb_search(n)->slot
#define	itemdb_available(n) (itemdb_exists(n) && itemdb_search(n)->flag.available)
#define	itemdb_viewid(n) (itemdb_search(n)->view_id)
int itemdb_group(unsigned short nameid);

int itemdb_searchrandomid(int flags);
int itemdb_searchrandomgroup(unsigned short groupid);

#define itemdb_value_buy(n) itemdb_search(n)->value_buy
#define itemdb_value_sell(n) itemdb_search(n)->value_sell
#define itemdb_value_notdc(n) itemdb_search(n)->flag.value_notdc
#define itemdb_value_notoc(n) itemdb_search(n)->flag.value_notoc
#define itemdb_canrefine(n) itemdb_search(n)->flag.no_refine
//Item trade restrictions [Skotlex]
bool itemdb_isdropable(unsigned short nameid, unsigned char gmlv);
bool itemdb_cantrade(unsigned short nameid, unsigned char gmlv);
bool itemdb_cansell(unsigned short nameid, unsigned char gmlv);
bool itemdb_canstore(unsigned short nameid, unsigned char gmlv);
bool itemdb_canguildstore(unsigned short nameid, unsigned char gmlv);
bool itemdb_cancartstore(unsigned short nameid, unsigned char gmlv);
bool itemdb_canpartnertrade(unsigned short nameid, unsigned char gmlv);

bool itemdb_isSingleStorage(unsigned short nameid);
bool itemdb_isSingleStorage(struct item_data &data);
bool itemdb_isEquipment(unsigned short nameid);

// itemdb_equipマクロとitemdb_equippointとの違いは
// 前者が鯖側dbで定義された値そのものを返すのに対し
// 後者はsessiondataを考慮した鞍側での装備可能場所
// すべての組み合わせを返す

void itemdb_reload(void);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif
