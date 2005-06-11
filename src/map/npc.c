// $Id: npc.c,v 1.5 2004/09/25 05:32:18 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "db.h"
#include "timer.h"
#include "nullpo.h"
#include "malloc.h"
#include "map.h"
#include "npc.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "status.h"
#include "itemdb.h"
#include "script.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "skill.h"
#include "grfio.h"
#include "showmsg.h"

#ifdef _WIN32
#undef isspace
#define isspace(x)  (x == ' ' || x == '\t')
#endif

struct npc_src_list {
	struct npc_src_list * next;
	char name[4];
};

static struct dbt *npcname_db;
static struct dbt *warpname_db;
static struct dbt *areascriptname_db;
static struct npc_src_list *npc_src_first=NULL;
static struct npc_src_list *npc_src_last=NULL;
static int npc_id=START_NPC_NUM;
static int npc_num=0;
static int areascript_num=0;
static int warp_num=0;
static int mob_num=0;
static int npc_delay_mob=0;
static int npc_cache_mob=0;
char *current_file = NULL;
int npc_get_new_npc_id(void){ return npc_id++; }

int npc_enable(const char *name,int flag)
{
	struct npc_data *nd= (struct npc_data *) strdb_search(npcname_db,name);
	if (nd==NULL)
		return 0;

	if (flag&1) {	// 有効化
		nd->flag&=~1;
		clif_spawnnpc(nd);
	}else if (flag&2){
		nd->flag&=~1;
		nd->option = 0x0000;
		clif_changeoption(&nd->bl);
	}else if (flag&4){
		nd->flag|=1;
		nd->option = 0x0002;
		clif_changeoption(&nd->bl);
	}else{		// 無効化
		nd->flag|=1;
		clif_clearchar(&nd->bl,0);
	}
	return 0;
}

/*==========================================
 * NPCを名前で探す
 *------------------------------------------
 */
struct npc_data* npc_name2id(const char *name)
{
	return (struct npc_data *) strdb_search(npcname_db,name);
}

/*==========================================
 * Activate area scripts
 *------------------------------------------
 */
int npc_touch_areascript(struct map_session_data *sd,int m,int x,int y)
{
	int i;

	nullpo_retr(1, sd);

	if(sd->npc_id)
		return 1;

	for(i=0;i<map[m].areascript_num;i++) { // Scan every areascript on the map
		if (!map[m].areascript[i]->flag&1 && // Ignore hidden/disabled scripts
			x >= map[m].areascript[i]->x1 && x <= map[m].areascript[i]->x2 &&
			y >= map[m].areascript[i]->y1 && x <= map[m].areascript[i]->y2) {
			script_run_function(map[m].areascript[i]->function,"i",sd->char_id);
		}
	}
	return 1;
}

/*==========================================
 * Use warps
 *------------------------------------------
 */
int npc_touch_warp(struct map_session_data *sd,int m,int x,int y)
{
	int i;

	nullpo_retr(1, sd);

	if(sd->npc_id)
		return 1;

	if (sd->status.option&6) // Hidden chars cannot use warps
		return 1;

 	for(i=0;i<map[m].warp_num;i++) { // Scan every warp on the map
		if (!map[m].warp[i]->flag&1 && // Ignore hidden/disabled warps
			x >= map[m].warp[i]->bl.x-map[m].warp[i]->xs/2 && x <= map[m].warp[i]->bl.x+map[m].warp[i]->xs/2 &&
			y >= map[m].warp[i]->bl.y-map[m].warp[i]->ys/2 && y <= map[m].warp[i]->bl.y+map[m].warp[i]->ys/2) {
			skill_stop_dancing(&sd->bl,0);
			pc_setpos(sd,map[m].warp[i]->destmap,map[m].warp[i]->destx,map[m].warp[i]->desty,0);
			return 1;
		}
	}
	return 1;
}

/*==========================================
 * 近くかどうかの判定
 *------------------------------------------
 */
int npc_checknear(struct map_session_data *sd,int id)
{
	struct npc_data *nd;

	nullpo_retr(0, sd);

	nd=(struct npc_data *)map_id2bl(id);
	if (nd==NULL || nd->bl.type!=BL_NPC) {
		if (battle_config.error_log)
			printf("no such npc : %d\n",id);
		return 1;
	}

	if (nd->class_<0)	// イベント系は常にOK
		return 0;

	// エリア判定
	if (nd->bl.m!=sd->bl.m ||
	   nd->bl.x<sd->bl.x-AREA_SIZE-1 || nd->bl.x>sd->bl.x+AREA_SIZE+1 ||
	   nd->bl.y<sd->bl.y-AREA_SIZE-1 || nd->bl.y>sd->bl.y+AREA_SIZE+1)
		return 1;

	return 0;
}

/*==========================================
 * NPCのオープンチャット発言
 *------------------------------------------
 */
int npc_globalmessage(const char *name,char *mes)
{
	struct npc_data *nd=(struct npc_data *) strdb_search(npcname_db,name);
	char temp[100];
	char ntemp[50];
	char *ltemp;

	if(nd==NULL) return 0;
	if(name==NULL) return 0;

	ltemp=strchr(name,'#');
	if(ltemp!=NULL) {
		strncpy(ntemp,name,ltemp - name);	// 123#456 の # から後ろを削除する
		ntemp[ltemp - name]=0x00;	// strncpy のバグ？使い方間違ってる？
	}

	snprintf(temp, sizeof temp ,"%s : %s",ntemp,mes);
	clif_GlobalMessage(&nd->bl,temp);

	return 0;
}

/*==========================================
 * クリック時のNPC処理
 *------------------------------------------
 */
int npc_click(struct map_session_data *sd,int id)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if (sd->npc_id != 0) {
		if (battle_config.error_log)
			printf("npc_click: npc_id != 0\n");
		return 1;
	}

	if (npc_checknear(sd,id))
		return 1;

	nd=(struct npc_data *)map_id2bl(id);

	if (nd->flag&1)	// 無効化されている
		return 1;

	sd->npc_id=id;
	script_run_function(nd->function,"i",sd->char_id);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
/*int npc_scriptcont(struct map_session_data *sd,int id)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if (id!=sd->npc_id)
		return 1;
	if (npc_checknear(sd,id))
		return 1;

	nd=(struct npc_data *)map_id2bl(id);

	sd->npc_pos=run_script(nd->u.scr.script,sd->npc_pos,sd->bl.id,id);

	return 0;
}*/

/*==========================================
 *
 *------------------------------------------
 */
int npc_scriptend(struct map_session_data *sd,int id)
{

	nullpo_retr(1, sd);
	
	if(sd->npc_id > 0)
		sd->npc_id = 0;
		
	return 0;
	
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buysellsel(struct map_session_data *sd,int id,int type)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if (npc_checknear(sd,id))
		return 1;

	nd=(struct npc_data *)map_id2bl(id);
	if (nd->flag&1)	// 無効化されている
		return 1;

	sd->npc_shopid=id;
	if (type==0) {
		clif_buylist(sd,nd);
	} else {
		clif_selllist(sd);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
/*int npc_buylist(struct map_session_data *sd,int n,unsigned short *item_list)
{
	struct npc_data *nd;
	double z;
	int i,j,w,skill,itemamount=0,new_=0;

	nullpo_retr(3, sd);
	nullpo_retr(3, item_list);

	if (npc_checknear(sd,sd->npc_shopid))
		return 3;

	nd=(struct npc_data*)map_id2bl(sd->npc_shopid);
	if (nd->bl.subtype!=SHOP)
		return 3;

	for(i=0,w=0,z=0;i<n;i++) {
		for(j=0;nd->u.shop_item[j].nameid;j++) {
			if (nd->u.shop_item[j].nameid==item_list[i*2+1])
				break;
		}
		if (nd->u.shop_item[j].nameid==0)
			return 3;

		if (itemdb_value_notdc(nd->u.shop_item[j].nameid))
			z+=(double)nd->u.shop_item[j].value * item_list[i*2];
		else
			z+=(double)pc_modifybuyvalue(sd,nd->u.shop_item[j].value) * item_list[i*2];
		itemamount+=item_list[i*2];

		switch(pc_checkadditem(sd,item_list[i*2+1],item_list[i*2])) {
		case ADDITEM_EXIST:
			break;
		case ADDITEM_NEW:
			new_++;
			break;
		case ADDITEM_OVERAMOUNT:
			return 2;
		}

		w+=itemdb_weight(item_list[i*2+1]) * item_list[i*2];
	}
	if (z > (double)sd->status.zeny)
		return 1;	// zeny不足
	if (w+sd->weight > sd->max_weight)
		return 2;	// 重量超過
	if (pc_inventoryblank(sd)<new_)
		return 3;	// 種類数超過

	pc_payzeny(sd,(int)z);
	for(i=0;i<n;i++) {
		struct item item_tmp;

		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid = item_list[i*2+1];
		item_tmp.identify = 1;	// npc販売アイテムは鑑定済み

		pc_additem(sd,&item_tmp,item_list[i*2]);
	}

	if (battle_config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_DISCOUNT)) > 0) {
		if (sd->status.skill[MC_DISCOUNT].flag != 0)
			skill = sd->status.skill[MC_DISCOUNT].flag - 2;
		if (skill > 0) {
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,0,(int)z);
		}
	}

	return 0;
}*/

/*==========================================
 *
 *------------------------------------------
 */
int npc_selllist(struct map_session_data *sd,int n,unsigned short *item_list)
{
	double z;
	int i,skill,itemamount=0;

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	if (npc_checknear(sd,sd->npc_shopid))
		return 1;
	for(i=0,z=0;i<n;i++) {
		int nameid;
		if (item_list[i*2]-2 <0 || item_list[i*2]-2 >=MAX_INVENTORY)
			return 1;
		nameid=sd->status.inventory[item_list[i*2]-2].nameid;
		if (nameid == 0 ||
		   sd->status.inventory[item_list[i*2]-2].amount < item_list[i*2+1])
			return 1;
		if (itemdb_value_notoc(nameid))
			z+=(double)itemdb_value_sell(nameid) * item_list[i*2+1];
		else
			z+=(double)pc_modifysellvalue(sd,itemdb_value_sell(nameid)) * item_list[i*2+1];
		itemamount+=item_list[i*2+1];
	}

	if (z > MAX_ZENY) z = MAX_ZENY;
	pc_getzeny(sd,(int)z);
	for(i=0;i<n;i++) {
		int item_id=item_list[i*2]-2;
		if(	sd->status.inventory[item_id].nameid>0 && sd->inventory_data[item_id] != NULL &&
			sd->inventory_data[item_id]->type==7 && sd->status.inventory[item_id].amount>0 &&
			sd->status.inventory[item_id].card[0] == (short)0xff00)
				if(search_petDB_index(sd->status.inventory[item_id].nameid, PET_EGG) >= 0)
					intif_delete_petdata(MakeDWord(sd->status.inventory[item_id].card[1],sd->status.inventory[item_id].card[2]));
		pc_delitem(sd,item_id,item_list[i*2+1],0);
	}

	//商人経験値
	if (battle_config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_OVERCHARGE)) > 0) {
		if (sd->status.skill[MC_OVERCHARGE].flag != 0)
			skill = sd->status.skill[MC_OVERCHARGE].flag - 2;
		if (skill > 0) {
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,0,(int)z);
		}
	}

	return 0;

}

// [Valaris] NPC Walking

/*==========================================
 * Time calculation concerning one step next to npc
 *------------------------------------------
 */
/*static int calc_next_walk_step(struct npc_data *nd)
{
	nullpo_retr(0, nd);

	if(nd->walkpath.path_pos>=nd->walkpath.path_len)
		return -1;
	if(nd->walkpath.path[nd->walkpath.path_pos]&1)
		return status_get_speed(&nd->bl)*14/10;
	return status_get_speed(&nd->bl);
}*/


/*==========================================
 * npc Walk processing
 *------------------------------------------
 */
/*static int npc_walk(struct npc_data *nd,unsigned int tick,int data)
{
	int moveblock;
	int i;
	static int dirx[8]={0,-1,-1,-1,0,1,1,1};
	static int diry[8]={1,1,0,-1,-1,-1,0,1};
	int x,y,dx,dy;

	nullpo_retr(0, nd);

	nd->state.state=MS_IDLE;
	if(nd->walkpath.path_pos>=nd->walkpath.path_len || nd->walkpath.path_pos!=data)
		return 0;

	nd->walkpath.path_half ^= 1;
	if(nd->walkpath.path_half==0){
		nd->walkpath.path_pos++;
		if(nd->state.change_walk_target){
			npc_walktoxy_sub(nd);
			return 0;
		}
	}
	else {
		if(nd->walkpath.path[nd->walkpath.path_pos]>=8)
			return 1;

		x = nd->bl.x;
		y = nd->bl.y;
		if(map_getcell(nd->bl.m,x,y,CELL_CHKNOPASS)) {
			npc_stop_walking(nd,1);
			return 0;
		}
		nd->dir=nd->walkpath.path[nd->walkpath.path_pos];
		dx = dirx[nd->dir];
		dy = diry[nd->dir];

		if(map_getcell(nd->bl.m,x+dx,y+dy,CELL_CHKNOPASS)) {
			npc_walktoxy_sub(nd);
			return 0;
		}

		moveblock = ( x/BLOCK_SIZE != (x+dx)/BLOCK_SIZE || y/BLOCK_SIZE != (y+dy)/BLOCK_SIZE);

		nd->state.state=MS_WALK;
		map_foreachinmovearea(clif_npcoutsight,nd->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,dx,dy,BL_PC,nd);

		x += dx;
		y += dy;

		if(moveblock) map_delblock(&nd->bl);
		nd->bl.x = x;
		nd->bl.y = y;
		if(moveblock) map_addblock(&nd->bl);

		map_foreachinmovearea(clif_npcinsight,nd->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,-dx,-dy,BL_PC,nd);
		nd->state.state=MS_IDLE;
	}
	if((i=calc_next_walk_step(nd))>0){
		i = i>>1;
		if(i < 1 && nd->walkpath.path_half == 0)
			i = 1;
		nd->walktimer=add_timer(tick+i,npc_walktimer,nd->bl.id,nd->walkpath.path_pos);
		nd->state.state=MS_WALK;

		if(nd->walkpath.path_pos>=nd->walkpath.path_len)
			clif_fixnpcpos(nd);	// When npc stops, retransmission current of a position.

	}
	return 0;
}

int npc_changestate(struct npc_data *nd,int state,int type)
{
	int i;

	nullpo_retr(0, nd);

	if(nd->walktimer != -1)
		delete_timer(nd->walktimer,npc_walktimer);
	nd->walktimer=-1;
	nd->state.state=state;

	switch(state){
	case MS_WALK:
		if((i=calc_next_walk_step(nd))>0){
			i = i>>2;
			nd->walktimer=add_timer(gettick()+i,npc_walktimer,nd->bl.id,0);
		}
		else
			nd->state.state=MS_IDLE;
		break;
	case MS_DELAY:
		nd->walktimer=add_timer(gettick()+type,npc_walktimer,nd->bl.id,0);
		break;

	}

	return 0;
}

static int npc_walktimer(int tid,unsigned int tick,int id,int data)
{
	struct npc_data *nd;

	nd=(struct npc_data*)map_id2bl(id);
	if(nd == NULL || nd->bl.type != BL_NPC)
		return 1;

	if(nd->walktimer != tid){
		return 0;
	}

	nd->walktimer=-1;

	if(nd->bl.prev == NULL)
		return 1;

	switch(nd->state.state){
		case MS_WALK:
			npc_walk(nd,tick,data);
			break;
		case MS_DELAY:
			npc_changestate(nd,MS_IDLE,0);
			break;
		default:
			break;
	}
	return 0;
}


static int npc_walktoxy_sub(struct npc_data *nd)
{
	struct walkpath_data wpd;

	nullpo_retr(0, nd);

	if(path_search(&wpd,nd->bl.m,nd->bl.x,nd->bl.y,nd->to_x,nd->to_y,nd->state.walk_easy))
		return 1;
	memcpy(&nd->walkpath,&wpd,sizeof(wpd));

	nd->state.change_walk_target=0;
	npc_changestate(nd,MS_WALK,0);

	clif_movenpc(nd);

	return 0;
}

int npc_walktoxy(struct npc_data *nd,int x,int y,int easy)
{
	struct walkpath_data wpd;

	nullpo_retr(0, nd);

	if(nd->state.state == MS_WALK && path_search(&wpd,nd->bl.m,nd->bl.x,nd->bl.y,x,y,0) )
		return 1;

	nd->state.walk_easy = easy;
	nd->to_x=x;
	nd->to_y=y;
	if(nd->state.state == MS_WALK) {
		nd->state.change_walk_target=1;
	} else {
		return npc_walktoxy_sub(nd);
	}

	return 0;
}

int npc_stop_walking(struct npc_data *nd,int type)
{
	nullpo_retr(0, nd);

	if(nd->state.state == MS_WALK || nd->state.state == MS_IDLE) {
		int dx=0,dy=0;

		nd->walkpath.path_len=0;
		if(type&4){
			dx=nd->to_x-nd->bl.x;
			if(dx<0)
				dx=-1;
			else if(dx>0)
				dx=1;
			dy=nd->to_y-nd->bl.y;
			if(dy<0)
				dy=-1;
			else if(dy>0)
				dy=1;
		}
		nd->to_x=nd->bl.x+dx;
		nd->to_y=nd->bl.y+dy;
		if(dx!=0 || dy!=0){
			npc_walktoxy_sub(nd);
			return 0;
		}
		npc_changestate(nd,MS_IDLE,0);
	}
	if(type&0x01)
		clif_fixnpcpos(nd);
	if(type&0x02) {
		int delay=status_get_dmotion(&nd->bl);
		unsigned int tick = gettick();
		if(nd->canmove_tick < tick)
			nd->canmove_tick = tick + delay;
	}

	return 0;
}*/

int npc_add (char *name,char *exname,short m,short x,short y,short dir,short class_,char *function)
{
    struct npc_data *nd=(struct npc_data *)aCalloc(1, sizeof(struct npc_data));

	nd->bl.prev=nd->bl.next=NULL;
	nd->bl.m=m;
	nd->bl.x=x;
	nd->bl.y=y;
	nd->bl.id=npc_get_new_npc_id();
	nd->bl.type=BL_NPC;
	strcpy(nd->name,name);
	strcpy(nd->exname,exname);
	strcpy(nd->function,function);
	nd->dir=dir;
	nd->class_=class_;
	nd->flag=0;
	nd->option=0;
	nd->opt1=0;
	nd->opt2=0;
	nd->opt3=0;
	nd->guild_id=0;
	nd->chat_id=0;

	nd->n=map_addnpc(m,nd);
	map_addblock(&nd->bl);
	clif_spawnnpc(nd);
	strdb_insert(npcname_db,nd->exname,nd);
	npc_num++;

	return nd->bl.id;
}

int areascript_add (char *name,short m,short x1,short y1,short x2,short y2,char *function)
{
	int i,j;
	
	struct areascript_data *ad=(struct areascript_data *)aCalloc(1, sizeof(struct areascript_data));

	ad->bl.prev=ad->bl.next=NULL;
	ad->bl.m=m;
	ad->bl.x=x1; // } Those x,y values are dummy anyway
	ad->bl.y=y1; // }
	ad->bl.id=npc_get_new_npc_id();
	ad->bl.type=BL_AREASCRIPT;
	strcpy(ad->name,name);
	strcpy(ad->function,function);
	ad->flag=0;
	ad->x1=x1;
	ad->y1=y1;
	ad->x2=x2;
	ad->y2=y2;

	ad->n=map_addareascript(m,ad);
	map_addblock(&ad->bl);
	for (i=y1;i<=y2;i++) {
		for (j=x1;j<=x2;j++) {
			if (map_getcell(m,j,i,CELL_CHKPASS))
				map_setcell(m,j,i,CELL_SETSCRIPT);
		}
	}
	strdb_insert(areascriptname_db,ad->name,ad);
	areascript_num++;

	return ad->bl.id;
}

int warp_add (char *name,short m,short x,short y,char *destmap,short destx,short desty,short xs,short ys)
{
	int i,j;
	struct warp_data *wd=(struct warp_data *)aCalloc(1, sizeof(struct warp_data));

	wd->bl.prev=wd->bl.next=NULL;
	wd->bl.m=m;
	wd->bl.x=x;
	wd->bl.y=y;
	wd->bl.id=npc_get_new_npc_id();
	wd->bl.type=BL_WARP;
	strcpy(wd->name,name);
	wd->flag=0;
	strcpy(wd->destmap,destmap);
	wd->destx=destx;
	wd->desty=desty;
	wd->xs=xs;
	wd->ys=ys;

	wd->n=map_addwarp(m,wd);
	map_addblock(&wd->bl);
	for (i=-ys;i<=ys;i++) {
		for (j=-xs;j<=xs;j++) {
			if (map_getcell(m,x+j,y+i,CELL_CHKPASS))
				map_setcell(m,x+j,y+i,CELL_SETWARP);
		}
	}
	clif_spawnwarp(wd);
	strdb_insert(warpname_db,wd->name,wd);
	warp_num++;

	return wd->bl.id;
}

int npc_unload (struct npc_data *nd)
{
	nullpo_retr(1, nd);

	if(nd->bl.prev == NULL)
		return 1;

#ifdef PCRE_SUPPORT
	npc_chat_finalize(nd);
#endif
	clif_clearchar_area(&nd->bl,2);
	strdb_erase(npcname_db, nd->exname);
	map_delblock(&nd->bl);
	map_deliddb(&nd->bl);

	if (nd->chat_id) {
		struct chat_data *cd = (struct chat_data*)map_id2bl(nd->chat_id);
		if (cd) aFree (cd);
		cd = NULL;
	}
	aFree(nd);

	return 0;
}

int areascript_unload (struct areascript_data *ad)
{
	nullpo_retr(1, ad);

	if(ad->bl.prev == NULL)
		return 1;

	strdb_erase(areascriptname_db, ad->name);
	map_delblock(&ad->bl);
	map_deliddb(&ad->bl);
	aFree(ad);

	return 0;
}

int warp_unload (struct warp_data *wd)
{
	nullpo_retr(1, wd);

	if(wd->bl.prev == NULL)
		return 1;

	clif_clearchar_area(&wd->bl,2);
	strdb_erase(warpname_db, wd->name);
	map_delblock(&wd->bl);
	map_deliddb(&wd->bl);
	aFree(wd);

	return 0;
}

//
// 初期化関係
//

/*==========================================
 * 読み込むnpcファイルのクリア
 *------------------------------------------
 */
void npc_clearsrcfile ()
{
	struct npc_src_list *p = npc_src_first, *p2;

	while (p) {
		p2 = p;
		p = p->next;
		aFree(p2);
	}
	npc_src_first = NULL;
	npc_src_last = NULL;
}
/*==========================================
 * 読み込むnpcファイルの追加
 *------------------------------------------
 */
void npc_addsrcfile (char *name)
{
	struct npc_src_list *nsl;

	if (strcmpi(name, "clear") == 0) {
		npc_clearsrcfile();
		return;
	}

	// prevent multiple insert of source files
	nsl = npc_src_first;
	while (nsl)
	{   // found the file, no need to insert it again
		if (0 == strcmp(name, nsl->name))
			return;
		nsl = nsl->next;
	}

	nsl = (struct npc_src_list *) aCalloc (1, sizeof(*nsl) + strlen(name));
	nsl->next = NULL;
	strncpy(nsl->name, name, strlen(name) + 1);
	if (npc_src_first == NULL)
		npc_src_first = nsl;
	if (npc_src_last)
		npc_src_last->next = nsl;
	npc_src_last = nsl;
}
/*==========================================
 * 読み込むnpcファイルの削除
 *------------------------------------------
 */
void npc_delsrcfile (char *name)
{
	struct npc_src_list *p = npc_src_first, *pp = NULL, **lp = &npc_src_first;

	if (strcmpi(name, "all") == 0) {
		npc_clearsrcfile();
		return;
	}

	while (p) {
		if (strcmp(p->name, name) == 0) {
			*lp = p->next;
			if (npc_src_last == p)
				npc_src_last = pp;
			aFree(p);
			break;
		}
		lp = &p->next;
		pp = p;
		p = p->next;
	}
}

int npc_parse_mob2 (struct mob_list *mob, int cached)
{
	int i;
	struct mob_data *md;

	for (i = 0; i < mob->num; i++) {
		md = (struct mob_data *) aCalloc (1, sizeof(struct mob_data));
		memset(md, 0, sizeof(struct mob_data));	//Why not 0 up the structure?	[Skotlex]

		if (mob->class_ > 4000) { // large/tiny mobs [Valaris]
			md->size = 2;
			mob->class_ -= 4000;
		} else if (mob->class_ > 2000) {
			md->size = 1;
			mob->class_ -= 2000;
		}

		md->bl.prev = NULL;
		md->bl.next = NULL;
		md->bl.m = mob->m;
		md->bl.x = mob->x;
		md->bl.y = mob->y;
		md->level = mob->level;
		memcpy(md->name, mob->mobname, 24);
		md->n = i;
		md->base_class = md->class_ = mob->class_;
		md->bl.id = npc_get_new_npc_id();
		md->m = mob->m;
		md->x0 = mob->x;
		md->y0 = mob->y;
		md->xs = mob->xs;
		md->ys = mob->ys;
		md->spawndelay1 = mob->delay1;
		md->spawndelay2 = mob->delay2;

		md->cached = cached;	//If cached, mob is dynamically removed
		md->timer = -1;
		md->speed = mob_db[mob->class_].speed;

		if (mob_db[mob->class_].mode & 0x02)
			md->lootitem = (struct item *)aCalloc(LOOTITEM_SIZE, sizeof(struct item));
		else
			md->lootitem = NULL;

		if (strlen(mob->eventname) >= 4) {
			memcpy(md->npc_event, mob->eventname, 24);
		} else
			memset(md->npc_event, 0, 24);

		md->bl.type = BL_MOB;
		map_addiddb(&md->bl);
		mob_spawn(md->bl.id);
	}

	return 0;
}

int npc_parse_mob (char *w1, char *w2, char *w3, char *w4)
{
	int level;
	char mapname[24];
	char mobname[24];
	struct mob_list mob;

	memset(&mob, 0, sizeof(struct mob_list));
	
	// 引数の個数チェック
	if (sscanf(w1, "%[^,],%d,%d,%d,%d", mapname, &mob.x, &mob.y, &mob.xs, &mob.ys) < 3 ||
		sscanf(w4, "%d,%d,%d,%d,%s", &mob.class_, &mob.num, &mob.delay1, &mob.delay2, mob.eventname) < 2 ) {
		ShowError("bad monster line : %s\n", w3);
		return 1;
	}

	mob.m = map_mapname2mapid(mapname);
	if (mob.m < 0)
		return 1;
		
	if (mob.num > 1 && battle_config.mob_count_rate != 100) {
		if ((mob.num = mob.num * battle_config.mob_count_rate / 100) < 1)
			mob.num = 1;
	}
	
	if (sscanf(w3, "%[^,],%d", mobname, &level) > 1)
		mob.level = level;
	if (strcmp(mobname, "--en--") == 0)
		memcpy(mob.mobname, mob_db[mob.class_].name, 24);
	else if (strcmp(mobname, "--ja--") == 0)
		memcpy(mob.mobname, mob_db[mob.class_].jname, 24);
	else memcpy(mob.mobname, mobname, 24);

	if( !battle_config.dynamic_mobs || mob.delay1 || mob.delay2 ) {
		npc_parse_mob2(&mob,0);
		npc_delay_mob += mob.num;
	} else {
		struct mob_list *dynmob = map_addmobtolist(mob.m);
		if( dynmob ) {
			memcpy(dynmob, &mob, sizeof(struct mob_list));
			// check if target map has players
			// (usually shouldn't occur when map server is just starting,
			// but not the case when we do @reloadscript
			if (map[mob.m].users > 0)
				npc_parse_mob2(&mob,1);
			npc_cache_mob += mob.num;
		} else {
			// mobcache is full
			// create them as delayed with one second
			mob.delay1 = 1000;
			npc_parse_mob2(&mob,0);
			npc_delay_mob += mob.num;
		}
	}

	mob_num++;

	return 0;
}

void npc_parsesrcfile (char *name)
{
	FILE *fp = fopen (name,"r");
	if (fp == NULL) {
		ShowError("File not found : %s\n",name);
		exit(1);
	}
	current_file = name;

	if (luaL_loadfile(L,name))
		ShowError("Cannot load script file %s : %s",name,lua_tostring(L,-1));
	if (lua_pcall(L,0,0,0))
		ShowError("Cannot run script file %s : %s",name,lua_tostring(L,-1));

	fclose(fp);

	return;
}

static int npc_read_indoors (void)
{
	char *buf, *p;
	int s, m;

	buf = (char *)grfio_reads("data\\indoorrswtable.txt",&s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;

	for (p = buf; p - buf < s; ) {
		char map_name[64];
		if (sscanf(p, "%[^#]#", map_name) == 1) {
			size_t pos = strlen(map_name) - 4;	// replace '.xxx' extension
			memcpy(map_name+pos,".gat",4);		// with '.gat'
			if ((m = map_mapname2mapid(map_name)) >= 0)
				map[m].flag.indoors = 1;
		}

		p = strchr(p, 10);
		if (!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\indoorrswtable.txt");

	return 0;
}

/*==========================================
 * 
 *------------------------------------------
 */
int npc_cleanup_sub (struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	switch(bl->type) {
	case BL_NPC:
		npc_unload((struct npc_data *)bl);
		break;
 	case BL_WARP:
		warp_unload((struct warp_data *)bl);
		break;
 	case BL_AREASCRIPT:
		areascript_unload((struct areascript_data *)bl);
		break;
	case BL_MOB:
		mob_unload((struct mob_data *)bl);
		break;
	}

	return 0;
}


int npc_reload (void)
{
	struct npc_src_list *nsl;
	int m, i;
	time_t last_time = time(0);
	int busy = 0;
	char c = '-';

	for (m = 0; m < map_num; m++) {
		map_foreachinarea(npc_cleanup_sub, m, 0, 0, map[m].xs, map[m].ys, 0);
		if(battle_config.dynamic_mobs) {	//dynamic check by [random]
			for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++)
				if (map[m].moblist[i]) aFree(map[m].moblist[i]);
			memset (map[m].moblist, 0, sizeof(map[m].moblist));
		}
		map[m].npc_num = 0;
	}

	// anything else we should cleanup?
	// Reloading npc's now
	npcname_db = strdb_init(24);
	
	for (nsl = npc_src_first; nsl; nsl = nsl->next) {
		npc_parsesrcfile(nsl->name);
		printf("\r");
		if (script_config.verbose_mode)
			ShowStatus("Loading NPCs... %-53s", nsl->name);
		else {
			ShowStatus("Loading NPCs... Working: ");
			if (last_time != time(0)) {
				last_time = time(0);
				switch(busy) {
					case 0: c='\\'; busy++; break;
					case 1: c='|'; busy++; break;
					case 2: c='/'; busy++; break;
					case 3: c='-'; busy=0;
				}
			}
			printf("[%c]",c);
		}
		fflush(stdout);
	}
	printf("\r");
/*	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:%30s\n\t-'"
		CL_WHITE"%d"CL_RESET"' Warps\n\t-'"
		CL_WHITE"%d"CL_RESET"' Shops\n\t-'"
		CL_WHITE"%d"CL_RESET"' Scripts\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Cached\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc_id - START_NPC_NUM, "", npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);*/

	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------
 */
int do_final_npc(void)
{
	int i;
	struct block_list *bl;
	struct npc_data *nd;
	struct warp_data *wd;
	struct areascript_data *ad;
	struct mob_data *md;
	struct pet_data *pd;

	for (i = START_NPC_NUM; i < npc_id; i++){
		if ((bl = map_id2bl(i))){
			if (bl->type == BL_NPC && (nd = (struct npc_data *)bl)){
				npc_unload(nd);
			} else if (bl->type == BL_WARP && (wd = (struct warp_data *)bl)){
				warp_unload(wd);
			} else if (bl->type == BL_AREASCRIPT && (ad = (struct areascript_data *)bl)){
				areascript_unload(ad);
			} else if (bl->type == BL_MOB && (md = (struct mob_data *)bl)){
				if (md->lootitem)
					aFree(md->lootitem);
				aFree(md);
			} else if (bl->type == BL_PET && (pd = (struct pet_data *)bl)){
				aFree(pd);
			}
		}
	}

	npc_clearsrcfile();

	return 0;
}

/*==========================================
 * npc初期化
 *------------------------------------------
 */
int do_init_npc(void)
{
	struct npc_src_list *nsl;
	time_t last_time = time(0);
	int busy = 0;
	char c = '-';

	// indoorrswtable.txt and etcinfo.txt [Celest]
	if (battle_config.indoors_override_grffile)
		npc_read_indoors();

	npcname_db = strdb_init(24);
	areascriptname_db = strdb_init(24);
	warpname_db = strdb_init(24);

	for (nsl = npc_src_first; nsl; nsl = nsl->next) {
		npc_parsesrcfile(nsl->name);
		current_file = NULL;
		printf("\r");
		if (script_config.verbose_mode)
			ShowStatus ("Loading NPCs... %-53s", nsl->name);
		else {
			ShowStatus("Loading NPCs... Working: ");
			if (last_time != time(0)) {
				last_time = time(0);
				switch(busy) {
					case 0: c='\\'; busy++; break;
					case 1: c='|'; busy++; break;
					case 2: c='/'; busy++; break;
					case 3: c='-'; busy=0;
				}
			}
			printf("[%c]",c);
		}
		fflush(stdout);
	}
	printf("\r");
/*	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:%30s\n\t-'"
		CL_WHITE"%d"CL_RESET"' Warps\n\t-'"
		CL_WHITE"%d"CL_RESET"' Shops\n\t-'"
		CL_WHITE"%d"CL_RESET"' Scripts\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Cached\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc_id - START_NPC_NUM, "", npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);*/

	return 0;
}
