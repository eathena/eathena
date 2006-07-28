// $Id: itemdb.c,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#include "db.h"
#include "nullpo.h"
#include "malloc.h"
#include "showmsg.h"
#include "utils.h"
#include "grfio.h"

#include "map.h"
#include "battle.h"
#include "itemdb.h"
#include "script.h"
#include "pc.h"

#include "datasq.h"


#define MAX_RANDITEM	2000
#define MAX_ITEMGROUP	20
// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct random_item_data blue_box[MAX_RANDITEM];
static int blue_box_count=0;
static int blue_box_default=0;

static struct random_item_data violet_box[MAX_RANDITEM];
static int violet_box_count=0;
static int violet_box_default=0;

static struct random_item_data card_album[MAX_RANDITEM];
static int card_album_count=0;
static int card_album_default=0;

static struct random_item_data gift_box[MAX_RANDITEM];
static int gift_box_count=0;
static int gift_box_default=0;

static struct random_item_data scroll[MAX_RANDITEM];
static int scroll_count=0;
static int scroll_default=0;

static struct random_item_data finding_ore[MAX_RANDITEM];
static int finding_ore_count = 0;
static int finding_ore_default = 0;

static struct item_group itemgroup_db[MAX_ITEMGROUP];

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
// name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
class CDBItemSearchname : public CDBProcessor
{
	const char* str;
	item_data*& dst;
public:
	CDBItemSearchname(const char* s, item_data*& d) : str(s), dst(d)	{}
	virtual ~CDBItemSearchname()	{}
	virtual bool process(void *key, void *data) const
	{
		struct item_data *item=(struct item_data *)data;
		if( strcasecmp(item->name,str)==0 )
		{
			dst = item;
			return false;
		}
		return true;
	}
};
/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
class CDBItemSearchjname : public CDBProcessor
{
	const char* str;
	item_data*& dst;
public:
	CDBItemSearchjname(const char* s, item_data*& d) : str(s), dst(d)	{}
	virtual ~CDBItemSearchjname()	{}
	virtual bool process(void *key, void *data) const
	{
		struct item_data *item=(struct item_data *)data;
		if( strcasecmp(item->jname,str)==0 )
		{
			dst = item;
			return false;
		}
		return true;
	}
};

/*==========================================
 * 名前で検索
 *------------------------------------------
 */
struct item_data* itemdb_searchname(const char *str)
{
	struct item_data *item=NULL;
	numdb_foreach(item_db, CDBItemSearchname(str,item) );
	return item;
}

/*==========================================
 * 箱系アイテム検索
 *------------------------------------------
 */
int itemdb_searchrandomid(int flags)
{
	unsigned short nameid=0;
	size_t i,index,count;
	struct random_item_data *list=NULL;

	struct
	{
		unsigned short nameid;
		size_t count;
		struct random_item_data *list;
	}
	data[7] =
	{
		{ 0,                   0,                 NULL        },
		{ blue_box_default,    blue_box_count,    blue_box    },
		{ violet_box_default,  violet_box_count,  violet_box  },
		{ card_album_default,  card_album_count,  card_album  },
		{ gift_box_default,    gift_box_count,    gift_box    },
		{ scroll_default,      scroll_count,      scroll      },
		{ finding_ore_default, finding_ore_count, finding_ore }
	};

	if(flags>=1 && flags<=6){
		nameid=data[flags].nameid;
		count=data[flags].count;
		list=data[flags].list;

		if(count > 0) {
			for(i=0;i<1000;++i) {
				index = rand()%count;
				if(	rand()%1000000 < list[index].per) {
					nameid = list[index].nameid;
					break;
				}
			}
		}
	}
	return nameid;
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_group (unsigned short nameid)
{
	int i, j;
	for (i=0; i < MAX_ITEMGROUP; ++i) {
		for (j=0; j < 20 && itemgroup_db[i].nameid[j]; ++j) {
			if (itemgroup_db[i].nameid[j] == nameid)
				return i;
		}
	}
	return -1;
}
int itemdb_searchrandomgroup(unsigned short groupid)
{
	unsigned short nameid;
	size_t i = 0;

	if( groupid >= MAX_ITEMGROUP ||
		itemgroup_db[groupid].nameid[0] == 0)
		return 0;
	do {
		if ((nameid = itemgroup_db[groupid].nameid[ rand()%20 ]) > 0)
			return nameid;		
	} while ((i++) < 20);
	return 0;
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(unsigned short nameid)
{
	return (struct item_data *) numdb_search(item_db,nameid);
}
/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(unsigned short nameid)
{
	struct item_data *id;

	id=(struct item_data *) numdb_search(item_db,nameid);
	if(id) return id;

	id = new struct item_data();
	numdb_insert(item_db,nameid,id);

	id->nameid=nameid;
	id->value_buy=10;
	id->value_sell=id->value_buy/2;
	id->weight=10;
	id->flag.sex=2;
	id->class_array=0xffffffff;

	if(nameid>500 && nameid<600)
		id->type=0;   //heal item
	else if(nameid>600 && nameid<700)
		id->type=2;   //use item
	else if((nameid>700 && nameid<1100) ||
			(nameid>7000 && nameid<8000))
		id->type=3;   //correction
	else if(nameid>=1750 && nameid<1771)
		id->type=10;  //arrow
	else if(nameid>1100 && nameid<2000)
		id->type=4;   //weapon
	else if((nameid>2100 && nameid<3000) ||
			(nameid>5000 && nameid<6000))
		id->type=5;   //armor
	else if(nameid>4000 && nameid<5000)
		id->type=6;   //card
	else if(nameid>9000 && nameid<10000)
		id->type=7;   //egg
	else if(nameid>10000)
		id->type=8;   //petequip

	return id;
}

/*==========================================
 *
 *------------------------------------------
 */
bool itemdb_isSingleStorage(unsigned short nameid)
{
	int type=itemdb_type(nameid);
	return (type==4 || type==5 || type==7 || type==8);
			//Weapon	Armor	Pet Egg		 Pet Equipment
}
/*==========================================
 *
 *------------------------------------------
 */
bool itemdb_isSingleStorage(struct item_data &data)
{
	int type=data.type;
	return (type==4 || type==5 || type==7 || type==8);
			//Weapon	Armor	Pet Egg		 Pet Equipment
}

/*==========================================
 *
 *------------------------------------------
 */
bool itemdb_isEquipment(unsigned short nameid)
{
	int type=itemdb_type(nameid);
	return (type==4 || type==5 || type==8);
			//weapon	//armor	//petequip
}

/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------
 */
bool itemdb_isdropable(unsigned short nameid, unsigned char gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&1)==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_cantrade(unsigned short nameid, unsigned char gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&2)==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_canpartnertrade(unsigned short nameid, unsigned char gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&(2|4))==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_cansell(unsigned short nameid, unsigned char gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&8)==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_cancartstore(unsigned short nameid, unsigned char gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&16)==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_canstore(unsigned short nameid, unsigned char gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&32)==0) || gmlv >= item->gm_lv_trade_override;
}

bool itemdb_canguildstore(unsigned short nameid, unsigned char gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (item->flag.trade_restriction&64)==0) || gmlv >= item->gm_lv_trade_override;
}


/*==========================================
 * ランダムアイテム出現データの読み込み
 *------------------------------------------
 */
int itemdb_read_randomitem()
{
	FILE *fp;
	char line[1024];
	int ln=0;
	unsigned short nameid;
	size_t i,j;
	char *str[10],*p;

	const struct {
		char filename[64];
		struct random_item_data *pdata;
		int *pcount;
		int *pdefault;
	} data[] = {
		{"db/item_bluebox.txt",		blue_box,	&blue_box_count, &blue_box_default		},
		{"db/item_violetbox.txt",	violet_box,	&violet_box_count, &violet_box_default	},
		{"db/item_cardalbum.txt",	card_album,	&card_album_count, &card_album_default	},
		{"db/item_giftbox.txt",		gift_box,	&gift_box_count, &gift_box_default	},
		{"db/item_scroll.txt",		scroll,		&scroll_count, &scroll_default	},
		{"db/item_findingore.txt",	finding_ore,&finding_ore_count, &finding_ore_default	},
	};

	for(i=0;i<sizeof(data)/sizeof(data[0]);++i)
	{
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=(char *) data[i].filename;
		*pdefault = 0;
		if( (fp=basics::safefopen(fn,"r"))==NULL )
		{
			ShowError("can't read %s\n",fn);
			continue;
		}

		while(fgets(line,sizeof(line),fp)){
			if( !get_prepared_line(line) )
				continue;
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<3 && p; ++j){
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}

			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid>=MAX_ITEMS)
				continue;
			if(nameid == 0) {
				if(str[2])
					*pdefault = atoi(str[2]);
				continue;
			}

			if(str[2]){
				pd[ *pc   ].nameid = nameid;
				pd[(*pc)++].per = atoi(str[2]);
			}

			if(ln >= MAX_RANDITEM)
				break;
			ln++;
		}
		fclose(fp);
		if (*pc > 0) {
			ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",*pc,fn);
		}
	}

	return 0;
}

/*==========================================
 * アイテム使用可能フラグのオーバーライド
 *------------------------------------------
 */
int itemdb_read_itemavail(void)
{
	FILE *fp;
	char line[1024];
	int ln = 0;
	unsigned short nameid;
	size_t j,k;
	char *str[10],*p;
	struct item_data *id;

	if( (fp=basics::safefopen("db/item_avail.txt","r"))==NULL ){
		ShowError("can't read %s\n","db/item_avail.txt");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if( !get_prepared_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p; ++j){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}

		if (j < 2 || str[0] == NULL ||
			(nameid = atoi(str[0])) >= 20000 || !(id = itemdb_exists(nameid)))
			continue;

		k=atoi(str[1]);
		if(k > 0) {
			id->flag.available = 1;
			id->view_id = k;
		} else
			id->flag.available = 0;
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/item_avail.txt");

	return 0;
}

/*==========================================
 * read item group data
 *------------------------------------------
 */
int itemdb_read_itemgroup(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	unsigned short groupid,nameid;
	size_t j;
	char *str[31],*p;

	if( (fp=basics::safefopen("db/item_group_db.txt","r"))==NULL ){
		ShowError("can't read db/item_group_db.txt\n");
		return -1;
	}

	while(fgets(line,sizeof(line),fp)){
		if( !get_prepared_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<31 && p; ++j){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		groupid = atoi(str[0]);
		if(groupid >= MAX_ITEMGROUP)
			continue;

		for (j=1; j<=30; ++j) {
			if (!str[j])
				break;
			nameid=atoi(str[j]);
			if(nameid >= MAX_ITEMS || !itemdb_exists(nameid))
				continue;
			//ShowMessage ("%d[%d] = %d\n", groupid, j-1, k);
			itemgroup_db[groupid].nameid[j-1] = nameid;
		}		
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/item_group_db.txt");
	return 0;
}

/*==========================================
 * アイテムの名前テーブルを読み込む
 *------------------------------------------
 */
int itemdb_read_itemnametable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_reads("data\\idnum2itemdisplaynametable.txt", s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){

#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
			if( itemdb_exists(nameid) &&
				strncmp(itemdb_search(nameid)->jname,buf2,24)!=0 ){
				ShowMessage("[override] %d %s => %s\n",nameid
					,itemdb_search(nameid)->jname,buf2);
			}
#endif

			memcpy(itemdb_search(nameid)->jname,buf2,24);
		}

		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	delete[] buf;
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\idnum2itemdisplaynametable.txt");

	return 0;
}

/*==========================================
 * カードイラストのリソース名前テーブルを読み込む
 *------------------------------------------
 */
int itemdb_read_cardillustnametable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_reads("data\\num2cardillustnametable.txt", s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){
			strcat(buf2,".bmp");
			memcpy(itemdb_search(nameid)->cardillustname,buf2,64);
//			ShowMessage("%d %s\n",nameid,itemdb_search(nameid)->cardillustname);
		}
		
		p=strchr(p,10);
		if(!p) break;
		p++;
	}
	delete[] buf;
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\num2cardillustnametable.txt");

	return 0;
}

//
// 初期化
//
/*==========================================
 *
 *------------------------------------------
 */
int itemdb_read_itemslottable(void)
{
	char *buf, *p;
	int s;

	buf = (char *)grfio_reads("data\\itemslottable.txt", s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;
	for (p = buf; p - buf < s; ) {
		int nameid, equip;
		struct item_data* item;
		sscanf(p, "%d#%d#", &nameid, &equip);
		item = itemdb_search(nameid);
		if (item && itemdb_isSingleStorage(*item))			
			item->equip = equip;
		p = strchr(p, 10);
		if(!p) break;
		p++;
		p=strchr(p, 10);
		if(!p) break;
		p++;
	}
	delete[] buf;
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\itemslottable.txt");
	return 0;
}

int itemdb_read_itemslotcounttable(void)
{
	char *buf, *p;
	int s;

	buf = (char *)grfio_reads("data\\itemslotcounttable.txt", s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;
	for (p = buf; p - buf < s;){
		int nameid, slot;
		sscanf(p, "%d#%d#", &nameid, &slot);
		itemdb_search(nameid)->flag.slot = slot;
		p = strchr(p,10);
		if(!p) break;
		p++;
		p = strchr(p,10);
		if(!p) break;
		p++;
	}
	delete[] buf;
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", "data\\itemslotcounttable.txt");
	return 0;
}

/*==========================================
 * 装備制限ファイル読み出し
 *------------------------------------------
 */
int itemdb_read_noequip(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p;
	struct item_data *id;

	if( (fp=basics::safefopen("db/item_noequip.txt","r"))==NULL ){
		ShowError("can't read %s\n", "db/item_noequip.txt");
		return -1;
	}
	while(fgets(line,sizeof(line),fp)){
		if( !get_prepared_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p; ++j){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL || str[1]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid>=MAX_ITEMS || !(id=itemdb_exists(nameid)))
			continue;

		id->flag.no_equip=atoi(str[1]);

		ln++;
	}
	fclose(fp);
	if (ln > 0) {
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/item_noequip.txt");
	}	
	return 0;
}

/*==========================================
 * Reads item trade restrictions [Skotlex]
 *------------------------------------------
 */
int itemdb_read_itemtrade(void)
{
	FILE *fp;
	int nameid, j, flag, gmlv, ln = 0;
	char line[1024], *str[10], *p;
	struct item_data *id;

	if ((fp = fopen("db/item_trade.txt","r")) == NULL) {
		ShowError("can't read db/item_trade.txt\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if( !get_prepared_line(line) )
			continue;
		memset(str, 0, sizeof(str));
		for (j=0, p=line; j < 3 && p; ++j) {
			str[j] = p;
			p = strchr(p, ',');
			if(p) *p++ = 0;
	}

		if (j < 3 || str[0] == NULL ||
			(nameid = atoi(str[0])) < 0 || nameid >= 20000 || !(id = itemdb_exists(nameid)))
			continue;

		flag = atoi(str[1]);
		gmlv = atoi(str[2]);
		
		if (flag > 0 && flag < 128 && gmlv > 0)
		{	//Check range
			id->flag.trade_restriction = flag;
			id->gm_lv_trade_override = gmlv;
			ln++;
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, "db/item_trade.txt");

	return 0;
}

#if defined(WITH_MYSQL)

/*======================================
* SQL
*===================================
*/
int itemdb_read_sqldb(void)
{
	unsigned short nameid;
	struct item_data *id;
	char script[65535 + 2 + 1]; // Maximum length of MySQL TEXT type (65535) + 2 bytes for curly brackets + 1 byte for terminator
	char *item_db_name[] = { item_db_db, item_db2_db };
	long unsigned int ln = 0;
	int i;	

	// ----------
	char tmp_sql[16384];

	for (i = 0; i < 2; ++i)
	{
		snprintf(tmp_sql, sizeof(tmp_sql), "SELECT * FROM `%s`", item_db_name[i]);

		// Execute the query; if the query execution succeeded...
		if (mysql_SendQuery(&mmysql_handle, tmp_sql) == 0)
		{
			sql_res = mysql_store_result(&mmysql_handle);
			// If the storage of the query result succeeded...
			if (sql_res)
			{
				// Parse each row in the query result into sql_row
				while ((sql_row = mysql_fetch_row(sql_res)))
				{
					/* +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
					   |  0 |            1 |             2 |    3 |         4 |          5 |      6 |      7 |       8 |     9 |    10 |         11 |            12 |              13 |           14 |          15 |   16       |   17 |         18 |           19 |
					   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+
					   | id | name_english | name_japanese | type | price_buy | price_sell | weight | attack | defence | range | slots | equip_jobs | equip_genders | equip_locations | weapon_level | equip_level | refineable | view | script_use | script_equip |
					   +----+--------------+---------------+------+-----------+------------+--------+--------+---------+-------+-------+------------+---------------+-----------------+--------------+-------------+------+------------+--------------+ */

					nameid = atoi(sql_row[0]);

					// If the identifier is not within the valid range, process the next row
					if(nameid == 0 || nameid >= MAX_ITEMS)
						continue;

					ln++;

					// ----------
					id = itemdb_search(nameid);
					
					memcpy(id->name, sql_row[1], 24);
					memcpy(id->jname, sql_row[2], 24);

					id->type = atoi(sql_row[3]);
					if (id->type == 11)
					{	//Items that are consumed upon target confirmation
						//(yggdrasil leaf, spells & pet lures) [Skotlex]
						id->type = 2;
						id->flag.delay_consume=1;
					}

					if ((sql_row[4] != NULL) && (sql_row[5] != NULL))
					{	// If price_buy is not NULL and price_sell is not NULL...
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[5]);
					}
					
					else if ((sql_row[4] != NULL) && (sql_row[5] == NULL))
					{	// If price_buy is not NULL and price_sell is NULL...
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[4]) / 2;
					}
					else if ((sql_row[4] == NULL) && (sql_row[5] != NULL))
					{	// If price_buy is NULL and price_sell is not NULL...
						id->value_buy = atoi(sql_row[5]) * 2;
						id->value_sell = atoi(sql_row[5]);
					}
					else// if ((sql_row[4] == NULL) && (sql_row[5] == NULL))
					{	// If price_buy is NULL and price_sell is NULL...
						id->value_buy = 0;
						id->value_sell = 0;
					}

					id->weight	= atoi(sql_row[6]);
					id->atk		= (sql_row[7] != NULL) ? atoi(sql_row[7]) : 0;
					id->def		= (sql_row[8] != NULL) ? atoi(sql_row[8]) : 0;
					id->range	= (sql_row[9] != NULL) ? atoi(sql_row[9]) : 0;
					id->flag.slot= (sql_row[10] != NULL)	? atoi(sql_row[10])	: 0;
					id->class_array	= (sql_row[11] != NULL) ? atoi(sql_row[11]) : 0;
					id->flag.sex= (config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 :
									( (sql_row[12] != NULL) ? atoi(sql_row[12]) : 0);
					id->equip	= (sql_row[13] != NULL) ? atoi(sql_row[13]) : 0;
					id->wlv		= (sql_row[14] != NULL) ? atoi(sql_row[14]) : 0;
					id->elv		= (sql_row[15] != NULL)	? atoi(sql_row[15]) : 0;
					id->flag.no_refine = (sql_row[16] == NULL || atoi(sql_row[16]) == 1)?0:1;
					id->look	= (sql_row[17] != NULL) ? atoi(sql_row[17]) : 0;
					id->view_id	= 0;
					// ----------

					if (sql_row[18] != NULL) {
						if (sql_row[18][0] == '{')
							id->use_script = parse_script((unsigned char *) sql_row[18], 0);
						else
						{
							snprintf(script, sizeof(script), "{%s}", sql_row[18]);
							id->use_script = parse_script((unsigned char *) script, 0);
						}
					}
					else
						id->use_script = NULL;

					if (sql_row[19] != NULL) {
						if (sql_row[19][0] == '{')
							id->equip_script = parse_script((unsigned char *) sql_row[19], 0);
						else
						{
							snprintf(script, sizeof(script), "{%s}", sql_row[19]);
							id->equip_script = parse_script((unsigned char *) script, 0);
						}
					}
					else
						id->equip_script = NULL;
					// ----------

					id->flag.available		= 1;
					id->flag.value_notdc	= 0;
					id->flag.value_notoc	= 0;
				}

				// If the retrieval failed, output an error
				if (mysql_errno(&mmysql_handle))
				{
					ShowError("Database server error (retrieving rows from %s): %s\n", item_db_name[i], mysql_error(&mmysql_handle));
				}
				else
				{
					ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", (unsigned long)ln, item_db_name[i]);
				}
				ln = 0;
			}
			else
			{
				ShowError("MySQL error (storing query result for %s): %s\n", item_db_name[i], mysql_error(&mmysql_handle));
			}
			// Free the query result
			mysql_free_result(sql_res);
	}
		else
			ShowError("Database server error (executing query for %s): %s\n", item_db_name[i], mysql_error(&mmysql_handle));
	}
	return 0;
}
#endif//WITH_MYSQL

/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0,lines=0;
	unsigned short nameid;
	size_t j;
	char *str[32],*p,*np;
	struct item_data *id;
	int i=0;
	char *filename[]={ "db/item_db.txt","db/item_db2.txt" };

	for(i=0;i<2;++i)
	{
		fp=basics::safefopen(filename[i],"r");
		if(fp==NULL)
		{
			ShowError("can't read %s\n",filename[i]);
			continue;
		}

		lines=0;
		while(fgets(line,sizeof(line),fp)){
			lines++;
			if( !get_prepared_line(line) )
				continue;
			memset(str,0,sizeof(str));
			for(j=0,np=p=line;j<18 && p; ++j){
				str[j]=p;
				p=strchr(p,',');
				if(p){ *p++=0; np=p; }
			}
			if( j<18 )
				continue;

			nameid=atoi(str[0]);
			if(nameid>=MAX_ITEMS)
				continue;
			ln++;

			//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,refineable,View
			id=itemdb_search(nameid);
			if(!id)
				continue;

			memcpy(id->name,str[1],24);
			memcpy(id->jname,str[2],24);
			id->type=atoi(str[3]);
			if (id->type == 11)
			{	//Items that are consumed upon target confirmation
				//(yggdrasil leaf, spells & pet lures) [Skotlex]
				id->type = 2;
				id->flag.delay_consume=1;
			}

			{
				int buy = abs(atoi(str[4])), sell = abs(atoi(str[5]));
				// if buying price > selling price * 2 consider it valid and don't change it [celest]
				if (buy && sell && buy > sell*2){
					id->value_buy = buy;
					id->value_sell = sell;
				} else {
					// buy??sell*2 ?I item_value_db.txt ?A?w’e?μ?A?-???3?￠?B
					if (sell) {		// sell’l?d?D?a?A?・?e
						id->value_buy = sell*2;
						id->value_sell = sell;
					} else {
						id->value_buy = buy;
						id->value_sell = buy/2;
					}
				}
				// check for bad prices that can possibly cause exploits
				if (id->value_buy*75/100 < id->value_sell*124/100) {
					ShowWarning ("Item %s [%d] buying:%d < selling:%d\n",
						id->name, id->nameid, id->value_buy*75/100, id->value_sell*124/100);
				}
			}
			id->weight=atoi(str[6]);
			id->atk=atoi(str[7]);
			id->def=atoi(str[8]);
			id->range=atoi(str[9]);
			id->flag.slot=atoi(str[10]);
			id->class_array=atoi(str[11]);
			id->flag.sex=atoi(str[12]);
			id->flag.sex= (config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 : atoi(str[12]);
			if(id->equip != atoi(str[13])){
				id->equip=atoi(str[13]);
			}
			id->wlv=atoi(str[14]);
			id->elv=atoi(str[15]);
			id->flag.no_refine = atoi(str[16])?0:1;	//If the refine column is 1, no_refine is 0
			id->look=atoi(str[17]);
			id->flag.available=1;
			id->flag.value_notdc=0;
			id->flag.value_notoc=0;
			id->view_id=0;

			if((p=strchr(np,'{'))!=NULL)
				id->use_script = parse_script((unsigned char *) p,lines);
			else
				id->use_script=NULL;

			if((p=strchr(p+1,'{'))!=NULL)
				id->equip_script = parse_script((unsigned char *) p,lines);
			else
				id->equip_script=NULL;
		}
		fclose(fp);
		if (ln > 0) {
			ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,filename[i]);
		}
		ln=0;	// reset to 0
	}
	return 0;
}

/*====================================
 * Removed item_value_db, don't re-add
 *------------------------------------
 */
void itemdb_read(void)
{
#if defined(WITH_MYSQL)
	if (db_use_sqldbs)
	{
		itemdb_read_sqldb();
	}
	else
#else
		itemdb_readdb();
#endif

	itemdb_read_itemgroup();
	itemdb_read_randomitem();
	itemdb_read_itemavail();
	itemdb_read_noequip();
	itemdb_read_itemtrade();
	if (config.cardillust_read_grffile)
		itemdb_read_cardillustnametable();
	if (config.item_equip_override_grffile)
		itemdb_read_itemslottable();
	if (config.item_slots_override_grffile)
		itemdb_read_itemslotcounttable();
	if (config.item_name_override_grffile)
		itemdb_read_itemnametable();
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------
 */
class CDBItemFinal : public CDBProcessor
{
	int flag;
public:
	CDBItemFinal(int f) : flag(f)	{}
	virtual ~CDBItemFinal()	{}
	virtual bool process(void *key, void *data) const
	{
		struct item_data *id = (struct item_data *)data;
		if( id )
		{
			if(id->use_script)
			{
				id->use_script->release();
				id->use_script = NULL;
			}
			if (id->equip_script)
			{
				id->equip_script->release();
				id->equip_script = NULL;
			}
			// Whether to clear the item data
			if (flag)
				delete id;
		}
		return true;
	}
};

void itemdb_reload(void)
{
	// free up all item scripts first
	numdb_foreach(item_db, CDBItemFinal(0) );
	itemdb_read();
}














#if defined(WITH_MYSQL)

MYSQL mmysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;

unsigned short map_server_port = 3306;
char map_server_ip[16] = "127.0.0.1";
char map_server_id[32] = "ragnarok";
char map_server_pw[32] = "ragnarok";
char map_server_db[32] = "ragnarok";
int db_use_sqldbs = 0;


char item_db_db[32] = "item_db";
char item_db2_db[32] = "item_db2";
char mob_db_db[32] = "mob_db";
char mob_db2_db[32] = "mob_db2";

char *INTER_CONF_NAME="conf/inter_athena.conf";

int inter_config_read(const char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=basics::safefopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: '%s'.\n",cfgName);
		return 1;
	}
	while(fgets(line,sizeof(line),fp)){
		if( !get_prepared_line(line) )
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcasecmp(w1,"import")==0){
		//support the import command, just like any other config
			inter_config_read(w2);
		}
		  else if(strcasecmp(w1,"item_db_db")==0){
			strcpy(item_db_db,w2);
		} else if(strcasecmp(w1,"mob_db_db")==0){
			strcpy(mob_db_db,w2);
		} else if(strcasecmp(w1,"item_db2_db")==0){
			strcpy(item_db2_db,w2);
		} else if(strcasecmp(w1,"mob_db2_db")==0){
			strcpy(mob_db2_db,w2);
		//Map Server SQL DB
		} else if(strcasecmp(w1,"map_server_ip")==0){
			strcpy(map_server_ip, w2);
		} else if(strcasecmp(w1,"map_server_port")==0){
			map_server_port=atoi(w2);
		} else if(strcasecmp(w1,"map_server_id")==0){
			strcpy(map_server_id, w2);
		} else if(strcasecmp(w1,"map_server_pw")==0){
			strcpy(map_server_pw, w2);
		} else if(strcasecmp(w1,"map_server_db")==0){
			strcpy(map_server_db, w2);
		} else if(strcasecmp(w1,"use_sql_db")==0){
			db_use_sqldbs = config_switch(w2);
			ShowMessage ("Using SQL dbs: %s\n",w2);
		}
	}
	fclose(fp);

	return 0;
}
#endif//defined(WITH_MYSQL)


void do_final_itemdb(void)
{
	if (item_db)
	{
		numdb_final(item_db, CDBItemFinal(1) );
		item_db = NULL;
	}

#if defined(WITH_MYSQL)
	if(db_use_sqldbs)
	{
		mysql_close(&mmysql_handle);
		ShowMessage("Close Map DB Connection....\n");
	}
#endif//WITH_MYSQL
}

int do_init_itemdb(void)
{

#if defined(WITH_MYSQL)
	inter_config_read(INTER_CONF_NAME);

	if(db_use_sqldbs)
	{
		mysql_init(&mmysql_handle);
		//DB connection start
		ShowMessage("Connect Database Server on %s:%u....(Map Server)\n", map_server_ip, map_server_port);
		if( !mysql_real_connect(&mmysql_handle, map_server_ip, map_server_id, map_server_pw, map_server_db ,map_server_port, (char *)NULL, 0))
		{
				//pointer check
				ShowMessage("%s\n",mysql_error(&mmysql_handle));
				exit(1);
		}
		else
		{
			ShowMessage ("connect success! (Map Server)\n");
		}
	}
#endif//WITH_MYSQL


	if(item_db)
	{
		numdb_final(item_db, CDBItemFinal(1) );
		item_db=NULL;
	}
	item_db = numdb_init();
	itemdb_read();

	return 0;
}
