// $Id: itemdb.c,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/db.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "map.h"
#include "grfio.h"
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
	if( strcmpi(item->name,str)==0 ) //by lupus
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
	if( strcmpi(item->jname,str)==0 )
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
	int nameid=0,i,index,count;
	struct random_item_data *list=NULL;

	struct {
		int nameid,count;
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
int itemdb_group (int nameid)
{
	int i, j;
	for (i=0; i < MAX_ITEMGROUP; i++) {
		for (j=0; j < itemgroup_db[i].qty && itemgroup_db[i].id[j]; j++) {
			if (itemgroup_db[i].id[j] == nameid)
				return i;
		}
	}
	return -1;
}
int itemdb_searchrandomgroup (int groupid)
{
	if (groupid < 0 || groupid >= MAX_ITEMGROUP ||
		itemgroup_db[groupid].qty == 0 || itemgroup_db[groupid].id[0] == 0)
		return 0;
	
	return itemgroup_db[groupid].id[ rand()%itemgroup_db[groupid].qty ];
}

/*==========================================
 * DBの存在確認
 *------------------------------------------
 */
struct item_data* itemdb_exists(int nameid)
{
	return (struct item_data *) numdb_search(item_db,nameid);
}
/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
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
	id->sex=2;
//	id->elv=0;
	id->class_=0xffffffff;
/*	
	id->flag.available=0;
	id->flag.value_notdc=0;  //一応・・・
	id->flag.value_notoc=0;
	id->flag.no_equip=0;
	id->flag.no_refine=0;
	id->flag.delay_consume=0;
	id->view_id=0;
*/
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
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==0 || type==2 || type==3 || type==6 || type==10)
		return 0;
	return 1;
}
/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip2(struct item_data *data)
{
	if(data) {
		int type=data->type;
		if(type==0 || type==2 || type==3 || type==6 || type==10)
			return 0;
		else
			return 1;
	}
	return 0;
}

/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------
 */
int itemdb_isdropable(int nameid, int gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&1) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cantrade(int nameid, int gmlv, int gmlv2)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&2) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_canpartnertrade(int nameid, int gmlv, int gmlv2)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&(2|4)) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_cansell(int nameid, int gmlv)
{
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&8) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cancartstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&16) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&32) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canguildstore(int nameid, int gmlv)
{	
	struct item_data* item = itemdb_exists(nameid);
	return (item && (!(item->flag.trade_restriction&64) || gmlv >= item->gm_lv_trade_override));
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip3(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==4 || type==5 || type == 8)
		return 1;
	return 0;
}

/*==========================================
 * ランダムアイテム出現データの読み込み
 *------------------------------------------
 */
static int itemdb_read_randomitem()
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,i,j;
	char *str[10],*p;

	const struct {
		char filename[64];
		struct random_item_data *pdata;
		int *pcount,*pdefault;
	} data[] = {
		{"db/item_bluebox.txt",		blue_box,	&blue_box_count, &blue_box_default		},
		{"db/item_violetbox.txt",	violet_box,	&violet_box_count, &violet_box_default	},
		{"db/item_cardalbum.txt",	card_album,	&card_album_count, &card_album_default	},
		{"db/item_giftbox.txt",		gift_box,	&gift_box_count, &gift_box_default	},
		{"db/item_scroll.txt",		scroll,		&scroll_count, &scroll_default	},
		{"db/item_findingore.txt",	finding_ore,&finding_ore_count, &finding_ore_default	},
	};

	for(i=0;i<sizeof(data)/sizeof(data[0]);i++){
		struct random_item_data *pd=data[i].pdata;
		int *pc=data[i].pcount;
		int *pdefault=data[i].pdefault;
		char *fn=(char *) data[i].filename;

		*pdefault = 0;
		if( (fp=fopen(fn,"r"))==NULL ){
			ShowError("can't read %s\n",fn);
			continue;
		}

		while(fgets(line,1020,fp)){
			if(line[0]=='/' && line[1]=='/')
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
			if(nameid<0 || nameid>=20000)
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
static int itemdb_read_itemavail (void)
{
	FILE *fp;
	int nameid, j, k, ln = 0;
	char line[1024], *str[10], *p;
	struct item_data *id;

	if ((fp = fopen("db/item_avail.txt","r")) == NULL) {
		ShowError("can't read db/item_avail.txt\n");
		return -1;
	}

	while (fgets(line, sizeof(line) - 1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		memset(str, 0, sizeof(str));
		for (j = 0, p = line; j < 2 && p; j++) {
			str[j] = p;
			p = strchr(p, ',');
			if(p) *p++ = 0;
		}

		if (j < 2 || str[0] == NULL ||
			(nameid = atoi(str[0])) < 0 || nameid >= 20000 || !(id = itemdb_exists(nameid)))
			continue;

		k = atoi(str[1]);
		if (k > 0) {
			id->flag.available = 1;
			id->view_id = k;
		} else
			id->flag.available = 0;
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, "db/item_avail.txt");

	return 0;
}

/*==========================================
 * read item group data
 *------------------------------------------
 */
static int itemdb_read_itemgroup(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int groupid,j,k;
	char *str[31],*p;

	if( (fp=fopen("db/item_group_db.txt","r"))==NULL ){
		ShowError("can't read db/item_group_db.txt\n");
		return -1;
	}

	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
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
		if (groupid < 0 || groupid >= MAX_ITEMGROUP)
			continue;

		for (j=1; j<=30; j++) {
			if (!str[j])
				break;
			k=atoi(str[j]);
			if (k < 0 || k >= 20000 || !itemdb_exists(k))
				continue;
			//printf ("%d[%d] = %d\n", groupid, j-1, k);
			itemgroup_db[groupid].id[j-1] = k;
			itemgroup_db[groupid].qty=j;
		}
		for (j=1; j<30; j++) { //Cleanup the contents. [Skotlex]
			if (itemgroup_db[groupid].id[j-1] == 0 &&
				itemgroup_db[groupid].id[j] != 0) 
			{
				itemgroup_db[groupid].id[j-1] = itemgroup_db[groupid].id[j];
				itemgroup_db[groupid].id[j] = 0;
				itemgroup_db[groupid].qty = j;
			}
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
static int itemdb_read_itemnametable(void)
{
	char *buf,*p;
	int s;

	buf=(char *) grfio_reads("data\\idnum2itemdisplaynametable.txt",&s);

	if(buf==NULL)
		return -1;

	buf[s]=0;
	for(p=buf;p-buf<s;){
		int nameid;
		char buf2[64]; //Why 64? What's this for, other than holding an item's name? [Skotlex]

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 ){

#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
			if( itemdb_exists(nameid) &&
				strncmp(itemdb_search(nameid)->jname,buf2,ITEM_NAME_LENGTH)!=0 ){
				ShowNotice("[override] %d %s => %s\n",nameid
					,itemdb_search(nameid)->jname,buf2);
			}
#endif

			memcpy(itemdb_search(nameid)->jname,buf2,ITEM_NAME_LENGTH-1);
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
static int itemdb_read_cardillustnametable(void)
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
static int itemdb_read_itemslottable(void)
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
		if (item && itemdb_isequip2(item))			
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

/*==========================================
 *
 *------------------------------------------
 */
static int itemdb_read_itemslotcounttable(void)
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
		if (slot > MAX_SLOTS)
		{
			ShowWarning("itemdb_read_itemslotcounttable: Item %d specifies %d slots, but the server only supports up to %d\n", nameid, slot, MAX_SLOTS);
			slot = MAX_SLOTS;
		}
		itemdb_slot(nameid) = slot;
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
static int itemdb_read_noequip(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p;
	struct item_data *id;

	if( (fp=fopen("db/item_noequip.txt","r"))==NULL ){
		ShowError("can't read db/item_noequip.txt\n");
		return -1;
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<2 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000 || !(id=itemdb_exists(nameid)))
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
static int itemdb_read_itemtrade(void)
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
		if (line[0] == '/' && line[1] == '/')
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
		
		if (flag > 0 && flag < 128 && gmlv > 0) { //Check range
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
static int itemdb_read_sqldb(void)
{
	unsigned short nameid;
	struct item_data *id;
	char script[65535 + 2 + 1]; // Maximum length of MySQL TEXT type (65535) + 2 bytes for curly brackets + 1 byte for terminator
	char *item_db_name[] = { item_db_db, item_db2_db };
	long unsigned int ln = 0;
	int i;	

	// ----------

	for (i = 0; i < 2; i++) {
		sprintf(tmp_sql, "SELECT * FROM `%s`", item_db_name[i]);

		// Execute the query; if the query execution succeeded...
		if (mysql_query(&mmysql_handle, tmp_sql) == 0) {
			sql_res = mysql_store_result(&mmysql_handle);

			// If the storage of the query result succeeded...
			if (sql_res) {
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
					if (nameid == 0 || nameid >= 20000)
						continue;

					ln++;

					// ----------
					id = itemdb_search(nameid);
					
					memcpy(id->name, sql_row[1], ITEM_NAME_LENGTH-1);
					memcpy(id->jname, sql_row[2], ITEM_NAME_LENGTH-1);

					id->type = atoi(sql_row[3]);
					if (id->type == 11)
					{	//Items that are consumed upon target confirmation
						//(yggdrasil leaf, spells & pet lures) [Skotlex]
						id->type = 2;
						id->flag.delay_consume=1;
					}

					// If price_buy is not NULL and price_sell is not NULL...
					if ((sql_row[4] != NULL) && (sql_row[5] != NULL)) {
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[5]);
					}
					// If price_buy is not NULL and price_sell is NULL...
					else if ((sql_row[4] != NULL) && (sql_row[5] == NULL)) {
						id->value_buy = atoi(sql_row[4]);
						id->value_sell = atoi(sql_row[4]) / 2;
					}
					// If price_buy is NULL and price_sell is not NULL...
					else if ((sql_row[4] == NULL) && (sql_row[5] != NULL)) {
						id->value_buy = atoi(sql_row[5]) * 2;
						id->value_sell = atoi(sql_row[5]);
					}
					// If price_buy is NULL and price_sell is NULL...
					if ((sql_row[4] == NULL) && (sql_row[5] == NULL)) {
						id->value_buy = 0;
						id->value_sell = 0;
					}

					id->weight	= atoi(sql_row[6]);
					id->atk		= (sql_row[7] != NULL) ? atoi(sql_row[7]) : 0;
					id->def		= (sql_row[8] != NULL) ? atoi(sql_row[8]) : 0;
					id->range	= (sql_row[9] != NULL) ? atoi(sql_row[9]) : 0;
					id->slot	= (sql_row[10] != NULL) ? atoi(sql_row[10]) : 0;
					if (id->slot > MAX_SLOTS)
					{
						ShowWarning("itemdb_read_sqldb: Item %d (%s) specifies %d slots, but the server only supports up to %d\n", nameid, id->jname, id->slot, MAX_SLOTS);
						id->slot = MAX_SLOTS;
					}
					id->class_	= (sql_row[11] != NULL) ? atoi(sql_row[11]) : 0;
					id->sex	= (battle_config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 :
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
						else {
							sprintf(script, "{%s}", sql_row[18]);
							id->use_script = parse_script((unsigned char *) script, 0);
						}
					} else id->use_script = NULL;

					if (sql_row[19] != NULL) {
						if (sql_row[19][0] == '{')
							id->equip_script = parse_script((unsigned char *) sql_row[19], 0);
						else {
							sprintf(script, "{%s}", sql_row[19]);
							id->equip_script = parse_script((unsigned char *) script, 0);
						}
					} else id->equip_script = NULL;

					// ----------

					id->flag.available		= 1;
					id->flag.value_notdc	= 0;
					id->flag.value_notoc	= 0;
				}

				ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, item_db_name[i]);
				ln = 0;
			} else {
				ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mmysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}

			// Free the query result
			mysql_free_result(sql_res);
		} else {
			ShowSQL("DB error (%s) - %s\n",item_db_name[i], mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}

	return 0;
}
#endif /* not TXT_ONLY */

/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0,lines=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;
	int i=0;
	char *filename[]={ "db/item_db.txt","db/item_db2.txt" };

	for(i=0;i<2;i++){

		fp=fopen(filename[i],"r");
		if(fp==NULL){
			if(i>0)
				continue;
			ShowFatalError("can't read %s\n",filename[i]);
			exit(1);
		}

		lines=0;
		while(fgets(line,1020,fp)){
			lines++;
			if(line[0]=='/' && line[1]=='/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,np=p=line;j<18 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p){ *p++=0; np=p; }
			}
			if(str[0]==NULL)
				continue;

			nameid=atoi(str[0]);
			if(nameid<=0 || nameid>=20000)
				continue;
			if (j < 18)
			{	//Crash-fix on broken item lines. [Skotlex]
				ShowWarning("Reading %s: Insufficient fields for item with id %d, skipping.\n", filename[i], nameid);
				continue;
			}
			ln++;

			//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,refineable,View
			id=itemdb_search(nameid);
			memcpy(id->name, str[1], ITEM_NAME_LENGTH-1);
			memcpy(id->jname, str[2], ITEM_NAME_LENGTH-1);
			id->type=atoi(str[3]);
			if (id->type == 11)
			{	//Items that are consumed upon target confirmation
				//(yggdrasil leaf, spells & pet lures) [Skotlex]
				id->type = 2;
				id->flag.delay_consume=1;
			}

			{
				int buy = atoi(str[4]), sell = atoi(str[5]);
				// if buying price > selling price * 2 consider it valid and don't change it [celest]
				if (buy && sell && buy > sell*2){
					id->value_buy = buy;
					id->value_sell = sell;
				} else {
					// buy≠sell*2 は item_value_db.txt で指定してください。
					if (sell) {		// sell値を優先とする
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
			id->slot=atoi(str[10]);
			if (id->slot > MAX_SLOTS)
			{
				ShowWarning("itemdb_readdb: Item %d (%s) specifies %d slots, but the server only supports up to %d\n", nameid, id->jname, id->slot, MAX_SLOTS);
				id->slot = MAX_SLOTS;
			}
			id->class_=atoi(str[11]);
			id->sex=atoi(str[12]);
			id->sex	= (battle_config.ignore_items_gender && nameid!=2634 && nameid!=2635) ? 2 : atoi(str[12]);
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

			id->use_script=NULL;
			id->equip_script=NULL;

			if((p=strchr(np,'{'))==NULL)
				continue;
			id->use_script = parse_script((unsigned char *) p,lines);
			if((p=strchr(p+1,'{'))==NULL)
				continue;
			id->equip_script = parse_script((unsigned char *) p,lines);
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
static void itemdb_read(void)
{
#ifndef TXT_ONLY
	if (db_use_sqldbs)
		itemdb_read_sqldb();
	else
#endif
		itemdb_readdb();

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
static int itemdb_final_sub (void *key,void *data,va_list ap)
{
	int flag;
	struct item_data *id = (struct item_data *)data;

	if (id == NULL)
		return 0;
	flag = va_arg(ap, int);
	if (id->use_script)
		aFree(id->use_script);
	if (id->equip_script)
		aFree(id->equip_script);
	// Whether to clear the item data
	if (flag)
		aFree(id);

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
		numdb_final(item_db, itemdb_final_sub, 1);
}

int do_init_itemdb(void)
{
	item_db = numdb_init();
	itemdb_read();

	return 0;
}
