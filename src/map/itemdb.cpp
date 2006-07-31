#include "baseparam.h"
#include "basemysql.h"

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
	for(p=buf;p-buf<s;)
	{
		int nameid;
		char buf2[64];

		if(	sscanf(p,"%d#%[^#]#",&nameid,buf2)==2 )
		{
#ifdef ITEMDB_OVERRIDE_NAME_VERBOSE
			item_data* item = itemdb_exists(nameid);
			if( item && strncmp(item->jname,buf2,24)!=0 )
			{
				ShowMessage("[override] %d %s => %s\n",nameid, item->jname, buf2);
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



/////////////////
/// update the item entries in sql database, 
/// since this data is not used for any other purpose 
/// than having fancy diplays on some websites, 
/// a bit more data preprocessing might be usefull beside the pure dump
void itemdb_sqlupdate()
{
#if defined(WITH_MYSQL)
	basics::CParam< basics::string<> > update_sqldbs("update_sqldbs", true);
	if( update_sqldbs() )
	{
		///////////////////////////////////////////////////////////////////////
		// sql access paraemter
		basics::CParam< basics::string<> > mysqldb_id("sql_username", "ragnarok");
		basics::CParam< basics::string<> > mysqldb_pw("sql_password", "ragnarok");
		basics::CParam< basics::string<> > mysqldb_db("sql_database", "ragnarok");
		basics::CParam< basics::string<> > mysqldb_ip("sql_ip",       "127.0.0.1");
		basics::CParam< ushort   >         mysqldb_port("sql_port",   3306);

		// sql control parameter
		basics::CParam< basics::string<> > sql_engine("sql_engine", "InnoDB");
		//basics::CParam< basics::string<> > sql_engine("sql_engine", "MyISAM");

		// sql table names
		basics::CParam< basics::string<> > tbl_item_db("tbl_item_db", "item_db");

		// sql access object
		basics::CMySQL sqlbase(mysqldb_id, mysqldb_pw,mysqldb_db,mysqldb_ip,mysqldb_port);

		// query handler
		basics::CMySQLConnection dbcon1(sqlbase);
		basics::string<> query;

		///////////////////////////////////////////////////////////////////////
		// disable foreign keys
		query << "SET FOREIGN_KEY_CHECKS=0";
		dbcon1.PureQuery(query);
		query.clear();

		///////////////////////////////////////////////////////////////////////////
		// drop tables
		query << "DROP TABLE IF EXISTS `" << dbcon1.escaped(tbl_item_db) << "`";
		dbcon1.PureQuery(query);
		query.clear();

		///////////////////////////////////////////////////////////////////////////
		query << "CREATE TABLE IF NOT EXISTS `" << dbcon1.escaped(tbl_item_db) << "` "
				 "("
				 "`id`			SMALLINT UNSIGNED NOT NULL default '0',"
				 "`name`		VARCHAR(24) NOT NULL default '',"
				 "`jname`		VARCHAR(24) NOT NULL default '',"
				 "`type`		TINYINT UNSIGNED NOT NULL default '0',"
				 "`value_buy`	INTEGER UNSIGNED NOT NULL default '0',"
				 "`value_sell`	INTEGER UNSIGNED NOT NULL default '0',"
				 "`weight`		SMALLINT UNSIGNED NOT NULL default '0',"
				 "`attack`		SMALLINT UNSIGNED NOT NULL default '0',"
				 "`defence`		SMALLINT UNSIGNED NOT NULL default '0',"
				 "`slots`		TINYINT UNSIGNED NOT NULL default '0',"
				 "`class_array`	INTEGER UNSIGNED NOT NULL default '0',"
				 "`equip`		SMALLINT UNSIGNED NOT NULL default '0',"
				 "`range`		TINYINT UNSIGNED NOT NULL default '0',"
				 "`wlv`			TINYINT UNSIGNED NOT NULL default '0',"
				 "`elv`			TINYINT UNSIGNED NOT NULL default '0',"
				 "`refineable`	TINYINT UNSIGNED NOT NULL default '0',"
				 "`view_id`		SMALLINT UNSIGNED NOT NULL default '0',"

				 "PRIMARY KEY  (`id`)"
				 ") "
				 "ENGINE = " << dbcon1.escaped(sql_engine);

		dbcon1.PureQuery(query);
		query.clear();

		///////////////////////////////////////////////////////////////////////
		// enable foreign keys
		query << "SET FOREIGN_KEY_CHECKS=1";
		dbcon1.PureQuery(query);
		query.clear();


		///////////////////////////////////////////////////////////////////////
		// insert entries
		db_iterator iter(item_db);
		for(; iter; ++iter)
		{
			size_t id = (size_t)iter.key();
			const item_data *item=(const item_data *)iter.data();
			//printf("%i %s\n", id, item->name);

			// don't dump items from overwrites
			if(!item || item->name[0]==0)
				continue;

			query << "REPLACE INTO `" << dbcon1.escaped(tbl_item_db) << "` "
					 "("
					 "`id`,"
					 "`name`,"
					 "`jname`,"
					 "`type`,"
					 "`value_buy`,"
					 "`value_sell`,"
					 "`weight`,"
					 "`attack`,"
					 "`defence`,"
					 "`slots`,"
					 "`class_array`,"
					 "`equip`,"
					 "`range`,"
					 "`wlv`,"
					 "`elv`,"
					 "`refineable`,"
					 "`view_id`"
					 ") "
					 "VALUES "
					 "("
					 "'" << id << "',"	// id
					 "'" << dbcon1.escaped(item->name) << "',"	// name
					 "'" << dbcon1.escaped(item->jname) << "',"	// jname
					 "'" << item->type << "',"					// type
					 "'" << item->value_buy << "',"				// value_buy
					 "'" << item->value_sell << "',"			// value_sell
					 "'" << item->weight << "',"				// weight
					 "'" << item->atk << "',"					// attack
					 "'" << item->def << "',"					// defence
					 "'" << item->flag.slot << "',"				// slots
					 "'" << item->class_array << "',"			// class_array
					 "'" << item->equip << "',"					// equip
					 "'" << item->range << "',"					// range
					 "'" << item->wlv << "',"					// wlv
					 "'" << item->elv << "',"					// elv
					 "'" << !item->flag.no_refine << "',"		// refineable
					 "'" << item->view_id << "'"				// view_id
					 ")";
			dbcon1.PureQuery(query);
			query.clear();
		}
		///////////////////////////////////////////////////////////////////////
	}
#endif
}



/*====================================
 * Removed item_value_db, don't re-add
 *------------------------------------
 */
void itemdb_read(void)
{
	itemdb_readdb();
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

	itemdb_sqlupdate();
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

void do_final_itemdb(void)
{
	if (item_db)
	{
		numdb_final(item_db, CDBItemFinal(1) );
		item_db = NULL;
	}
}

int do_init_itemdb(void)
{
	if(item_db)
	{
		numdb_final(item_db, CDBItemFinal(1) );
		item_db=NULL;
	}
	item_db = numdb_init();
	itemdb_read();
	itemdb_sqlupdate();

	return 0;
}





