// $Id: itemdb.c,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#include "base.h"
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


#define MAX_RANDITEM	2000
#define MAX_ITEMGROUP	20
// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

static struct random_item_data blue_box[MAX_RANDITEM], violet_box[MAX_RANDITEM], card_album[MAX_RANDITEM], gift_box[MAX_RANDITEM], scroll[MAX_RANDITEM], finding_ore[MAX_RANDITEM];
static int blue_box_count=0, violet_box_count=0, card_album_count=0, gift_box_count=0, scroll_count=0, finding_ore_count = 0;
static int blue_box_default=0, violet_box_default=0, card_album_default=0, gift_box_default=0, scroll_default=0, finding_ore_default = 0;

static struct item_group itemgroup_db[MAX_ITEMGROUP];

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
// name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
int itemdb_searchname_sub(void *key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	if( strcasecmp(item->name,str)==0 ) //by lupus
		*dst=item;
	return 0;
}

/*==========================================
 * 名前で検索用
 *------------------------------------------
 */
int itemdb_searchjname_sub(void *key,void *data,va_list ap)
{
	struct item_data *item=(struct item_data *)data,**dst;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	if( strcasecmp(item->jname,str)==0 )
		*dst=item;
	return 0;
}
/*==========================================
 * 名前で検索
 *------------------------------------------
 */
struct item_data* itemdb_searchname(const char *str)
{
	struct item_data *item=NULL;
	numdb_foreach(item_db,itemdb_searchname_sub,str,&item);
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
	} data[7];

	// for BCC32 compile error
	data[0].nameid = 0;						data[0].count = 0; 					data[0].list = NULL;
	data[1].nameid = blue_box_default;		data[1].count = blue_box_count;		data[1].list = blue_box;
	data[2].nameid = violet_box_default;	data[2].count = violet_box_count;	data[2].list = violet_box;
	data[3].nameid = card_album_default;	data[3].count = card_album_count;	data[3].list = card_album;
	data[4].nameid = gift_box_default;		data[4].count = gift_box_count;		data[4].list = gift_box;
	data[5].nameid = scroll_default;		data[5].count = scroll_count;		data[5].list = scroll;
	data[6].nameid = finding_ore_default;	data[6].count = finding_ore_count;	data[6].list = finding_ore;

	if(flags>=1 && flags<=6){
		nameid=data[flags].nameid;
		count=data[flags].count;
		list=data[flags].list;

		if(count > 0) {
			for(i=0;i<1000;i++) {
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
	for (i=0; i < MAX_ITEMGROUP; i++) {
		for (j=0; j < 20 && itemgroup_db[i].nameid[j]; j++) {
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

	id=(struct item_data *)aCalloc(1,sizeof(struct item_data));
	numdb_insert(item_db,nameid,id);

	id->nameid=nameid;
	id->value_buy=10;
	id->value_sell=id->value_buy/2;
	id->weight=10;
	id->flag.sex=2;
	id->class_=0xffff;

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
bool itemdb_isequip(unsigned short nameid)
{
	int type=itemdb_type(nameid);
	return (type!=0 && type!=2 && type!=3 && type!=6 && type!=10);
}
/*==========================================
 *
 *------------------------------------------
 */
bool itemdb_isequip2(struct item_data &data)
{
	int type=data.type;
	return (type!=0 && type!=2 && type!=3 && type!=6 && type!=10);
	}
/*==========================================
 *
 *------------------------------------------
 */
bool itemdb_isequip3(unsigned short nameid)
{
	int type=itemdb_type(nameid);
	return (type!=4 && type!=5 && type!=8);
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

	for(i=0;i<sizeof(data)/sizeof(data[0]);i++)
	{
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=(char *) data[i].filename;
		*pdefault = 0;
		if( (fp=savefopen(fn,"r"))==NULL )
		{
			ShowMessage("can't read %s\n",fn);
			continue;
		}

		while(fgets(line,1020,fp)){
			if( !skip_empty_line(line) )
				continue;
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<3 && p;j++){
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

	if( (fp=savefopen("db/item_avail.txt","r"))==NULL ){
		ShowMessage("can't read %s\n","db/item_avail.txt");
		return -1;
	}

	while (fgets(line, sizeof(line) - 1, fp)) {
		if( !skip_empty_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
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

	if( (fp=savefopen("db/item_group_db.txt","r"))==NULL ){
		ShowError("can't read db/item_group_db.txt\n");
		return -1;
	}

	while(fgets(line,1020,fp)){
		if( !skip_empty_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<31 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		groupid = atoi(str[0]);
		if(groupid >= MAX_ITEMGROUP)
			continue;

		for (j=1; j<=30; j++) {
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

	buf=(char *) grfio_reads("data\\idnum2itemdisplaynametable.txt",&s);

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
	aFree(buf);
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

	buf=(char *) grfio_reads("data\\num2cardillustnametable.txt",&s);

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
	aFree(buf);
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

	buf = (char *)grfio_reads("data\\itemslottable.txt", &s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;
	for (p = buf; p - buf < s; ) {
		int nameid, equip;
		struct item_data* item;
		sscanf(p, "%d#%d#", &nameid, &equip);
		item = itemdb_search(nameid);
		if (item && itemdb_isequip2(*item))			
			item->equip = equip;
		p = strchr(p, 10);
		if(!p) break;
		p++;
		p=strchr(p, 10);
		if(!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\itemslottable.txt");
	return 0;
}

int itemdb_read_itemslotcounttable(void)
{
	char *buf, *p;
	int s;

	buf = (char *)grfio_reads("data\\itemslotcounttable.txt", &s);
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
	aFree(buf);
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

	if( (fp=savefopen("db/item_noequip.txt","r"))==NULL ){
		ShowMessage("can't read %s\n", "db/item_noequip.txt");
		return -1;
	}
	while(fgets(line,1020,fp)){
		if( !skip_empty_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
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

	while (fgets(line, sizeof(line) - 1, fp)) {
		if( !skip_empty_line(line) )
			continue;
		memset(str, 0, sizeof(str));
		for (j = 0, p = line; j < 3 && p; j++) {
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

#ifndef TXT_ONLY

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

	for (i = 0; i < 2; i++)
	{
		sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_name[i]);

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
					id->class_	= (sql_row[11] != NULL) ? atoi(sql_row[11]) : 0;
					id->flag.sex= (battle_config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 :
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
							sprintf(script, "{%s}", sql_row[18]);
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
							sprintf(script, "{%s}", sql_row[19]);
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
					ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, item_db_name[i]);
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
#endif /* not TXT_ONLY */

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

	for(i=0;i<2;i++){

		fp=savefopen(filename[i],"r");
		if(fp==NULL){
			if(i>0)
				continue;
			ShowMessage("can't read %s\n",filename[i]);
			exit(1);
		}

		lines=0;
		while(fgets(line,1020,fp)){
			lines++;
			if( !skip_empty_line(line) )
				continue;
			memset(str,0,sizeof(str));
			for(j=0,np=p=line;j<18 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p){ *p++=0; np=p; }
			}
			if( j<17 )
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
			id->class_=atoi(str[11]);
			id->flag.sex=atoi(str[12]);
			id->flag.sex= (battle_config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 : atoi(str[12]);
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
#ifndef TXT_ONLY
	if (db_use_sqldbs)
	{
		itemdb_read_sqldb();
	}
	else
	{
		itemdb_readdb();	
	}
#else	// not TXT_ONLY
	itemdb_readdb();
#endif	// TXT_ONLY

	itemdb_read_itemgroup();
	itemdb_read_randomitem();
	itemdb_read_itemavail();
	itemdb_read_noequip();
	itemdb_read_itemtrade();
	if (battle_config.cardillust_read_grffile)
		itemdb_read_cardillustnametable();
	if (battle_config.item_equip_override_grffile)
		itemdb_read_itemslottable();
	if (battle_config.item_slots_override_grffile)
		itemdb_read_itemslotcounttable();
	if (battle_config.item_name_override_grffile)
		itemdb_read_itemnametable();
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------
 */
int itemdb_final_sub (void *key,void *data,va_list ap)
{
	struct item_data *id = (struct item_data *)data;

	if( id )
	{
		int flag = va_arg(ap, int);
		if (id->use_script)
			aFree(id->use_script);
		if (id->equip_script)
			aFree(id->equip_script);
		// Whether to clear the item data
		if (flag)
			aFree(id);
	}
	return 0;
}

void itemdb_reload(void)
{
	// free up all item scripts first
	numdb_foreach(item_db, itemdb_final_sub, 0);
	itemdb_read();
}

void do_final_itemdb(void)
{
	if (item_db)
	{
		numdb_final(item_db, itemdb_final_sub, 1);
		item_db = NULL;
	}
}

int do_init_itemdb(void)
{
	if(item_db)
	{
		numdb_final(item_db, itemdb_final_sub, 1);
	}
	item_db = numdb_init();
	itemdb_read();

	return 0;
}
