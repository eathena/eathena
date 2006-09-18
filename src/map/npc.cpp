// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
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
#include "utils.h"
#include "socket.h"


struct npc_src_list {
	struct npc_src_list * next;
	char name[4];
};

// single liked list of pointers to npc structures
struct npc_mark
{
	npc_data *nd;
	struct npc_mark *next;
	npc_mark(npc_data*n, struct npc_mark *nx) :
		nd(n), next(nx)
	{}
};
typedef struct npc_mark* npc_mark_p;


static struct npc_src_list *npc_src_first=NULL;
static struct npc_src_list *npc_src_last=NULL;
static size_t npc_warp=0;
static size_t npc_shop=0;
static size_t npc_script=0;
static size_t npc_mob=0;
static size_t npc_delay_mob=0;
static size_t npc_cache_mob=0;


// currently managed externally
static struct dbt *npcname_db=NULL;


/// name to object
npc_data* npc_data::from_name(const char *name)
{
	return (npc_data *) strdb_search(npcname_db,name);
}


/// do object depending stuff for ending the walk.
void npc_data::do_stop_walking()
{
}
/// do object depending stuff for the walk step.
/// does ontouch fix for moving npcs with ontouch area.
bool npc_data::do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy)
{
	// ontouch fix for moving npcs with ontouch area
	if(this->class_>=0 && this->xs>0 && this->ys>0)
	{
		// "xor" is an operator in C++ (Alternative tokens)
		// together with all other logic operator names and some more digraphs
		// whatever shit those guys had in mind; I don't argue with it 
		// on gcc it could be disables with -fno-operator-names, MS does not have it at all
		// but better make sure to not have those in the code at all

		// coord of old rectangle
		const int xol = this->block_list::x-this->xs/2;
		const int xo_ = this->block_list::x+this->xs/2;
		const int yol = this->block_list::y-this->ys/2;
		const int yor = this->block_list::y+this->ys/2;
		// coord of new rectangle
		const int xnl = target.x-this->xs/2;
		const int xnr = target.x+this->xs/2;
		const int ynl = target.y-this->ys/2;
		const int ynr = target.y+this->ys/2;

		int i,k;
		// going through old rechtangle
		for(i=xol; i<=xo_; ++i)
		for(k=yol; k<=yor; ++k)
		{
			if( i>=0 && i<maps[this->block_list::m].xs &&	// inside map
				k>=0 && k<maps[this->block_list::m].ys &&
				(i<xnl || i>xnr || k<ynl || k>ynr) &&		// not inside of new rect
				!map_getcell(this->block_list::m,i,k,CELL_CHKNOPASS) )
			{	// remove npc ontouch area from map_cells
				map_setcell(this->block_list::m,i,k,CELL_CLRNPC);
			}
		}

		// going through new rechtangle
		for(i=xnl; i<=xnr; ++i)
		for(k=ynl; k<=ynr; ++k)
		{
			if( i>=0 && i<maps[this->block_list::m].xs &&	// inside map
				k>=0 && k<maps[this->block_list::m].ys &&
				(i<xol || i>xo_ || k<yol || k>yor) &&		// not inside of old rect
				!map_getcell(this->block_list::m,i,k,CELL_CHKNOPASS) )
			{	// add npc ontouch area to map_cells
				map_setcell(this->block_list::m,i,k,CELL_SETNPC);
			}
		}
	}
	return true;
}

/// static function called from clif_parse_NpcClicked.
void npc_data::click(map_session_data &sd, uint32 npcid)
{
	if( sd.ScriptEngine.isRunning() )
	{
		if (config.error_log)
			ShowWarning("npc_click: char %s (id=%lu) is already running a script\n", sd.status.name, (ulong)sd.status.char_id);
	}
	else
	{
		npc_data *nd= npc_data::from_blid(npcid);
		if( nd && !(nd->invalid) && nd->is_near(sd) )
		{	// execute the OnClick method of the current object
			nd->OnClick(sd);
		}
	}
}



/// what to do when clicked.
/// run a script
void npcscript_data::OnClick(map_session_data &sd)
{
	if(this->ref)
		CScriptEngine::run(this->ref->script, 0, sd.block_list::id, this->block_list::id);
}

/// try and execute touchup 
/// test for ontouch lable or start script from beginning if none
void npcscript_data::OnTouch(block_list& bl)
{
	map_session_data *sd=bl.get_sd();
	if( sd && sd->areanpc_id != this->block_list::id )
	{
		sd->areanpc_id = this->block_list::id;

		char eventname[64]; // npc->name is 24 max + 9 for a possible ::OnTouch attachment
		snprintf(eventname,sizeof(eventname),"%s::OnTouch", this->name);
		
		// has ontouch label or does click
		if( npc_event(*sd, eventname,0)!=0 )
			this->OnClick(*sd);
	}
}

/// try and execute touchup 
/// execute warp
void npcwarp_data::OnTouch(block_list& bl)
{
	map_session_data *sd=bl.get_sd();
	if( sd &&
		!(sd->status.option&6) )	// hidden chars cannot use warps
	{	
		skill_stop_dancing(sd,0);
		pc_setpos(*sd, this->mapname, this->xt, this->yt, 0);
	}
}

/// what to do when clicked.
/// bring up associated shop
void npcshop_data::OnClick(map_session_data &sd)
{
	clif_npcbuysell(sd, this->block_list::id);
}




























uint32 &get_npc_id()
{
	// simple singleton
	// !! change id generation to support freeing/reloading of npc's
	static uint32 npc_id=START_NPC_NUM;
	return npc_id;
}

uint32 npc_get_new_npc_id(void)
{	// simple singleton
	// !! change id generation to support freeing/reloading of npc's
	return (get_npc_id()++); 
}

static struct dbt *ev_db=NULL;


struct event_data
{
	npcscript_data *sc;
	size_t pos;
	event_data(npcscript_data *s, size_t p) :
		sc(s), pos(p)
	{}
};





// ============================================
// ADDITION Qamera death/disconnect/connect event mod
int npc_event_doall_attached(const char *name, map_session_data &sd)
{
	int c=0;
	if(name)
	{	
		struct npc_att_data nad;
		size_t len = strlen(name)+1;
		if( len+2 > sizeof(nad.buf) ) len = sizeof(nad.buf)-2;
		memset(&nad, 0, sizeof(struct npc_att_data));
		memcpy(nad.buf,"::",2);
		memcpy(nad.buf+2,name,len);
		nad.buf[sizeof(nad.buf)-1]=0;//force EOS
		nad.sd=&sd;

		db_iterator<const char*,event_data *> iter(ev_db);
		for(; iter; ++iter)
		{
			const char *p =iter.key();
			const event_data *ev=iter.data();
			if( ev && ev->sc && ev->sc->ref &&
				p && (p=strchr(p,':')) && 0==strcasecmp(nad.buf, p) )
			{
				CScriptEngine::run(ev->sc->ref->script, ev->pos, nad. sd->block_list::id, ev->sc->block_list::id);
				++c;
			}
		}
	}
	return c;   
}
// END ADDITION
// ============================================ 
/*==========================================
 * NPCの無効化/有効化
 * npc_enable
 * npc_enable_sub 有効時にOnTouchイベントを実行
 *------------------------------------------
 */
class CNpcEnable : public CMapProcessor
{
	npc_data &nd;
public:
	CNpcEnable(npc_data &n) : nd(n)	{}
	~CNpcEnable()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd=bl.get_sd();
		if(sd)
		{
			char name[64];

			if (nd.invalid)	// 無効化されている
				return 1;
			if(sd->areanpc_id==nd.block_list::id)
				return 1;
			sd->areanpc_id=nd.block_list::id;

			snprintf(name,sizeof(name),"%s::OnTouch", nd.name);
			npc_event(*sd,name,0);
		}
		return 0;
	}
};
bool npc_enable(const char *name, int flag)
{
	npc_data *nd= npc_data::from_name(name);
	if(nd)
	{
		if (flag&1)
		{	// 有効化
			nd->invalid=0;
			clif_spawnnpc(*nd);
		}
		else if (flag&2)
		{
			nd->invalid=0;
			nd->option = 0x0000;
			clif_changeoption(*nd);
		}
		else if (flag&4)
		{
			nd->invalid=1;
			nd->option = 0x0002;
			clif_changeoption(*nd);
		}
		else
		{		// 無効化
			nd->invalid=1;
			clif_clearchar(*nd,0);
		}
		if( flag&3 && (nd->xs > 0 || nd->ys >0) )
		{
			block_list::foreachinarea( CNpcEnable(*nd),
				nd->block_list::m, ((int)nd->block_list::x)-nd->xs,((int)nd->block_list::y)-nd->ys, ((int)nd->block_list::x)+nd->xs,((int)nd->block_list::y)+nd->ys,BL_PC);
		}
		return true;
	}
	return false;
}



void ev_release(struct dbn *db, int which)
{
    if (which & 0x1)
	{
        delete[] ((char*)db->key);
		db->key = NULL;
	}
    if (which & 0x2)
	{
        delete ((struct event_data*)db->data);
		db->data=NULL;
	}
}

/*==========================================
 * イベントの遅延実行
 *------------------------------------------
 */
int npc_event_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	char *eventname = (char *)data.ptr;
	if(eventname)
	{
		const event_data *ev = (struct event_data *)strdb_search(ev_db,eventname);
		map_session_data *sd=map_session_data::from_blid(id);
		size_t i;

		if( ev==NULL || ev->sc==NULL )
		{
			if(config.error_log)
				ShowWarning("npc_event_timer: event not found [%s]\n",eventname);
		}	
		else if(sd)
		{
			for(i=0;i<MAX_EVENTTIMER;++i)
			{
				if( sd->eventtimer[i]==tid )
				{
					sd->eventtimer[i]=-1;
					npc_event(*sd,eventname,0);
					break;
				}
			}
			if(i==MAX_EVENTTIMER && config.error_log)
				ShowWarning("npc_event_timer: event timer not found [%s]!\n",eventname);
		}
		delete[] eventname;
		get_timer(tid)->data = 0;
	}
	return 0;
}

int npc_timer_event(const char *eventname)	// Added by RoVeRT
{
	const event_data *ev=(const event_data *) strdb_search(ev_db,eventname);
	if(ev && ev->sc)
	{
		if(ev->sc->ref)
			CScriptEngine::run(ev->sc->ref->script, ev->pos, 0, ev->sc->block_list::id);
	}
	else
	{
		ShowError("npc_timer_event: event not found [%s]\n",eventname);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// イベント用ラベルのエクスポート
/// defaults: rid=0, map=-1
int npc_event_doall(const char *name, int rid, int map)
{
	int c=0;
	char buf[128]="::";
	safestrcpy(buf+2, sizeof(buf)-2, name);

	db_iterator<const char*,event_data*> iter(ev_db);
	for(; iter; ++iter)
	{
		const char *p= iter.key();
		event_data *ev=iter.data();
		if( ev && ev->sc && 
			(p=strchr(p,':')) && strcasecmp(buf,p)==0 && 
			(map<0 || ev->sc->block_list::m>=map_num || map==ev->sc->block_list::m) )
		{
			if(ev->sc->ref)
				CScriptEngine::run(ev->sc->ref->script,ev->pos, rid, ev->sc->block_list::id);
			++c;
		}
	}
	return c;
}


int npc_event_do(const char *name)
{
	if (*name==':' && name[1]==':')
	{
		return npc_event_doall(name+2);
	}
	else
	{
		int c=0;
		db_iterator<const char*, event_data*> iter(ev_db);
		for(; iter; ++iter)
		{
			const char *p=iter.key();
			event_data *ev=iter.data();
			if(ev && p && 0==strcasecmp(name,p) && ev->sc && ev->sc->ref)
			{
				CScriptEngine::run(ev->sc->ref->script,ev->pos,0,ev->sc->block_list::id);
				++c;
			}
		}
		return c;
	}
}

/*==========================================
 * 時計イベント実行
 *------------------------------------------
 */
int npc_event_do_clock(int tid, unsigned long tick, int id, basics::numptr data)
{
	static const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat", ""};
	static struct tm ev_tm    = {-1,-1,-1,-1,-1,-1,-1,-1,-1};	// 時計イベント用

	char buf[64];
	int c=0;
	time_t timer	= time(NULL);
	struct tm *t	= localtime(&timer);
	const char *day	= days[t->tm_wday&0x07];

	if (t->tm_min != ev_tm.tm_min )
	{
		snprintf(buf,sizeof(buf),"OnMinute%02d",t->tm_min);
		c+=npc_event_doall(buf);
		snprintf(buf,sizeof(buf),"OnClock%02d%02d",t->tm_hour,t->tm_min);
		c+=npc_event_doall(buf);
		snprintf(buf,sizeof(buf),"On%s%02d%02d",day,t->tm_hour,t->tm_min);
		c+=npc_event_doall(buf);
	}
	if (t->tm_hour!= ev_tm.tm_hour)
	{
		snprintf(buf,sizeof(buf),"OnHour%02d",t->tm_hour);
		c+=npc_event_doall(buf);
	}
	if (t->tm_mday!= ev_tm.tm_mday)
	{
		snprintf(buf,sizeof(buf),"OnDay%02d%02d",t->tm_mon+1,t->tm_mday);
		c+=npc_event_doall(buf);
	}
	ev_tm = *t;
	return c;
}
/*==========================================
 * OnInitイベント実行(&時計イベント開始)
 *------------------------------------------
 */
int npc_event_do_oninit(void)
{
//	int c = npc_event_doall("OnInit");
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
	CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_event_doall("OnInit"));

	add_timer_interval(gettick()+100, 1000, npc_event_do_clock, 0, 0);

	return 0;
}

int npc_do_ontimer(uint32 npc_id, map_session_data &sd, int option)
{
	db_iterator<const char*, event_data*> iter(ev_db);
	for(; iter; ++iter)
	{
		const char *p = iter.key();
		event_data *ev = iter.data();
		unsigned long tick=0;
		char event[50];

		if( ev && ev->sc && ev->sc->block_list::id==npc_id && 
			(p=strchr(p,':')) && strncasecmp("::OnTimer",p,8)==0 )
		{
			tick = atoi(p+9);
			snprintf(event, sizeof(event), "%s%s", ev->sc->name, p);	// name::event
			if (option!=0)
				pc_addeventtimer(sd,tick,event);
			else
				pc_deleventtimer(sd,event);
		}
	}

	return 0;
}



/*==========================================
 * タイマーイベント実行
 *------------------------------------------
 */
int npc_timerevent(int tid, unsigned long tick, int id, basics::numptr data)
{
	npcscript_data* sc= npcscript_data::from_blid(id);

	if( sc==NULL || sc->nexttimer<0 )
	{
		ShowMessage("npc_timerevent: ??\n");
	}
	else
	{

		int t;
		npc_timerevent_list &te=sc->timer_event[sc->nexttimer];
		sc->timertick=tick;
		sc->timerid = -1;
		t = sc->timer += data.num;
		++sc->nexttimer;

		if( sc->timeramount>sc->nexttimer )
		{
			int next= sc->timer_event[ sc->nexttimer ].timer - t;
			sc->timerid = add_timer(tick+next, npc_timerevent, id, next);
		}
		if(sc->ref)
			CScriptEngine::run(sc->ref->script, te.pos, sc->rid, sc->block_list::id);
	}
	return 0;
}
/*==========================================
 * タイマーイベント開始
 *------------------------------------------
 */
int npc_timerevent_start(npcscript_data &sc, uint32 rid)
{
	int j,n, next;

	n = sc.timeramount;
	if( sc.nexttimer>=0 || n==0 )
		return 0;

	for(j=0;j<n;++j)
	{
		if( sc.timer_event[j].timer > sc.timer )
			break;
	}
	if(j>=n)
		return 0;

	sc.nexttimer=j;
	sc.timertick=gettick();
	sc.rid=rid;	// changed to: attaching to given rid by default [Shinomori]

	next = sc.timer_event[j].timer - sc.timer;
	sc.timerid = add_timer(sc.timertick+next, npc_timerevent, sc.block_list::id, next);

	return 0;
}
/*==========================================
 * タイマーイベント終了
 *------------------------------------------
 */
int npc_timerevent_stop(npcscript_data &sc)
{
	if( sc.nexttimer>=0 )
	{
		sc.nexttimer = -1;
		sc.timer += (int)(gettick() - sc.timertick);
		if(sc.timerid!=-1)
			delete_timer(sc.timerid, npc_timerevent);
		sc.timerid = -1;
		sc.rid = 0;
	}
	return 0;
}
/*==========================================
 * タイマー値の所得
 *------------------------------------------
 */
int npc_gettimerevent_tick(npcscript_data &sc)
{
	unsigned long tick=sc.timer;
	if( sc.nexttimer>=0 )
		tick += gettick() - sc.timertick;
	return tick;
}
/*==========================================
 * タイマー値の設定
 *------------------------------------------
 */
int npc_settimerevent_tick(npcscript_data &sc, int newtimer)
{
	int flag	= sc.nexttimer;
	uint32 rid	= sc.timerid;

	npc_timerevent_stop(sc);
	sc.timer=newtimer;
	if(flag>=0)
		npc_timerevent_start(sc, rid);

	return 0;
}

/*==========================================
 * イベント型のNPC処理
 *------------------------------------------
 */
int npc_event(map_session_data &sd,const char *eventname,int mob_kill)
{
	const event_data *ev=(const event_data *) strdb_search(ev_db,eventname);
	char mobevent[128];

	if(ev==NULL && eventname && strcmp(((eventname)+strlen(eventname)-9),"::OnTouch") == 0)
		return 1;

	if(ev==NULL || ev->sc==NULL)
	{
		if (mob_kill)
		{
			snprintf(mobevent, sizeof(mobevent), "%s%s", eventname, "::OnMyMobDead");
			ev = (event_data *) strdb_search(ev_db, mobevent);
			if(ev==NULL || ev->sc== NULL)
			{
				if (strncasecmp(eventname,"GM_MONSTER",10)!=0)
					ShowError("npc_event: event not found [%s]\n", mobevent);
				return 0;
			}
		}
		else
		{
			if (config.error_log)
				ShowError("npc_event: event not found [%s]\n", eventname);
			return 0;
		}
	}
	if( ev && ev->sc && !ev->sc->invalid && ev->sc->ref )
		CScriptEngine::run(ev->sc->ref->script, ev->pos, sd.block_list::id, ev->sc->block_list::id);
	return 0;
}

int npc_command(map_session_data &sd, const char *npcname, const char *command)
{
	db_iterator<const char*, const event_data*> iter(ev_db);
	for(; iter; ++iter)
	{
		const char *p=iter.key();
		const event_data *ev=iter.data();
		if( ev && ev->sc &&
			strcmp(ev->sc->name,npcname)==0 && (p=strchr(p,':')) && 0==strncasecmp("::OnCommand",p,10) )
		{
			if( 0==strcasecmp(command,p+11) && ev->sc->ref)
				CScriptEngine::run(ev->sc->ref->script,ev->pos, 0, ev->sc->block_list::id);
		}
	}
	return 0;
}
/*==========================================
 * 接触型のNPC処理
 *------------------------------------------
 */
int npc_touch_areanpc(map_session_data &sd, unsigned short m, int x,int y)
{
	size_t i;
	int xs,ys;

	if( sd.ScriptEngine.isRunning() || m>=map_num )
		return 1;

	for(i=0;i<maps[m].npc_num;++i)
	{
		if (maps[m].npc[i]->invalid)
			continue;
		xs = maps[m].npc[i]->xs;
		ys = maps[m].npc[i]->ys;

		if ( (x >= (maps[m].npc[i]->block_list::x-xs/2)) && (x < (maps[m].npc[i]->block_list::x-xs/2+xs)) &&
			 (y >= (maps[m].npc[i]->block_list::y-ys/2)) && (y < (maps[m].npc[i]->block_list::y-ys/2+ys)) )
		{
			maps[m].npc[i]->OnTouch(sd);
		}
	}
	return 0;
}


/*==========================================
 * NPCのオープンチャット発言
 *------------------------------------------
 */
int npc_globalmessage(const char *name, const char *mes)
{
	npc_data *nd=(npc_data *)strdb_search(npcname_db,name);
	char temp[128];
	char ntemp[64];
	char *ltemp;

	if(nd==NULL) return 0;
	if(name==NULL) return 0;

	safestrcpy(ntemp,sizeof(ntemp),name);	// copy the name
	ltemp=strchr(ntemp,'#');				// check for a # numerator
	if(ltemp) *ltemp=0;						// and remove it

	size_t sz = snprintf(temp, sizeof(temp),"%s: %s",ntemp, mes);
	clif_GlobalMessage(*nd, temp, sz);

	return 0;
}



/*==========================================
 *
 *------------------------------------------
 */
int npc_scriptcont(map_session_data &sd, uint32 id)
{
	sd.ScriptEngine.restart(id);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buysellsel(map_session_data &sd,uint32 id,int type)
{
	npcshop_data *sh= npcshop_data::from_blid(id);
	if( !sh || !sh->is_near(sd) || sh->invalid)
		return 1;

	sd.npc_shopid=id;
	if( type==0 )
		clif_buylist(sd,*sh);
	else
		clif_selllist(sd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buylist(map_session_data &sd,unsigned short n,unsigned char *buffer)
{

	uint32 z;
	size_t i,j,w,skill,itemamount=0,new_=0;
	unsigned short amount, itemid;
	nullpo_retr(3, buffer);

	npcshop_data *sh= npcshop_data::from_blid(sd.npc_shopid);
	if( !sh || !sh->is_near(sd) )
		return 3;

	for(i=0,w=0,z=0;i<n;++i)
	{
		// amount fix
		if( (uint32)RBUFW(buffer,2*(i*2)) > config.vending_max_value )
			RBUFW(buffer,2*(i*2)) = 0;	// clear it directly in the buffer

		amount = RBUFW(buffer,2*(i*2));
		itemid = RBUFW(buffer,2*(i*2+1));

		for(j=0; j<sh->shop_item_cnt; ++j)
		{
			if( sh->shop_item_lst[j].nameid==itemid )
				break;
		}
		if( j>=sh->shop_item_cnt )
			return 3;

		if( itemdb_isSingleStorage(sh->shop_item_lst[j].nameid) && amount > 1 )
		{	//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
				sd.status.name, sd.status.account_id, sd.status.char_id, amount, sh->shop_item_lst[j].nameid);
			amount = 1;
		}

		if (itemdb_value_notdc(sh->shop_item_lst[j].nameid))
			z+=(sh->shop_item_lst[j].price * amount);
		else
			z+=(pc_modifybuyvalue(sd,sh->shop_item_lst[j].price) * amount);
		itemamount+=amount;

		switch(pc_checkadditem(sd,itemid,amount)) {
		case ADDITEM_EXIST:
			break;
		case ADDITEM_NEW:
			new_++;
			break;
		case ADDITEM_OVERAMOUNT:
			return 2;
		}

		w+=itemdb_weight(itemid) * amount;
	}
	if (z > sd.status.zeny)
		return 1;	// zeny不足
	if (w+sd.weight > sd.max_weight)
		return 2;	// 重量超過
	if (pc_inventoryblank(sd)<new_)
		return 3;	// 種類数超過

	pc_payzeny(sd, z);
	for(i=0;i<n;++i) {
		struct item item_tmp;
		amount = RBUFW(buffer,2*(i*2));
		itemid = RBUFW(buffer,2*(i*2+1));

		memset(&item_tmp,0,sizeof(struct item));
		item_tmp.nameid = itemid;
		item_tmp.identify = 1;	// npc販売アイテムは鑑定済み

		pc_additem(sd,item_tmp,amount);
	}

	//商人経験値
/*	if ((sd->status.class_ == 5) || (sd->status.class_ == 10) || (sd->status.class_ == 18)) {
		z = z * pc_checkskill(sd,MC_DISCOUNT) / ((1 + 300 / itemamount) * 4000) * config.shop_exp;
		pc_gainexp(sd,0,z);
	}*/
	if (config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_DISCOUNT)) > 0) {
		if (sd.status.skill[MC_DISCOUNT].flag != 0)
			skill = sd.status.skill[MC_DISCOUNT].flag - 2;
		if (skill > 0) {
			z = z * skill * config.shop_exp/10000;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,0,z);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_selllist(map_session_data &sd,unsigned short n,unsigned char *buffer)
{
	uint32 z;
	size_t i,skill,itemamount=0;
	unsigned short amount,itemid,nameid;

	nullpo_retr(1, buffer);

	npcshop_data *nd=npcshop_data::from_blid(sd.npc_shopid);
	if( !nd || !nd->is_near(sd) )
		return 1;
	
	for(i=0,z=0;i<n;++i)
	{
		// amount fix
		if( (uint32)RBUFW(buffer,2*(i*2+1)) > config.vending_max_value )
			RBUFW(buffer,2*(i*2+1)) = 0;	// clear the amount
		
		amount = RBUFW(buffer,2*(i*2+1));
		itemid = RBUFW(buffer,2*(i*2))-2;
				
		if(itemid >=MAX_INVENTORY)
			return 1;
		nameid=sd.status.inventory[itemid].nameid;
		if (nameid == 0 ||
		   sd.status.inventory[itemid].amount < amount)
			return 1;
		if (itemdb_value_notoc(nameid))
			z+=itemdb_value_sell(nameid) * amount;
		else
			z+=pc_modifysellvalue(sd,itemdb_value_sell(nameid)) * amount;
		itemamount+=amount;
	}

	if (z > MAX_ZENY) z = MAX_ZENY;
	pc_getzeny(sd,z);
	for(i=0;i<n;++i) {
		amount = RBUFW(buffer,2*(i*2+1));
		itemid = RBUFW(buffer,2*(i*2))-2;
		if(	sd.status.inventory[itemid].nameid>0 && sd.inventory_data[itemid] != NULL &&
			sd.inventory_data[itemid]->type==7 && sd.status.inventory[itemid].amount>0 &&
			sd.status.inventory[itemid].card[0] == 0xff00)
				if(search_petDB_index(sd.status.inventory[itemid].nameid, PET_EGG) >= 0)
					intif_delete_petdata(basics::MakeDWord(sd.status.inventory[itemid].card[1],sd.status.inventory[itemid].card[2]));

		pc_delitem(sd,itemid,amount,0);
	}

	//商人経験値
	if (config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_OVERCHARGE)) > 0) {
		if (sd.status.skill[MC_OVERCHARGE].flag != 0)
			skill = sd.status.skill[MC_OVERCHARGE].flag - 2;
		if (skill > 0) {
			z = z * skill * config.shop_exp/10000;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,0,z);
		}
	}
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
	struct npc_src_list *p=npc_src_first, *pnext;
	while (p)
	{
		pnext=p->next;
		free(p);
		p=pnext;
	}
	npc_src_first = NULL;
	npc_src_last = NULL;
}

void npc_printsrcfile()
{
	struct npc_src_list *p=npc_src_first;
	while( p )
	{
		ShowMessage("%s\n", p->name);
		p=p->next;
	}
}
/*==========================================
 * 読み込むnpcファイルの追加
 *------------------------------------------
 */
void npc_addsrcfile(const char *name)
{
	struct npc_src_list *nsl;
	size_t len;

	if ( strcasecmp(name,"clear")==0 )
		npc_clearsrcfile();
	else
	{
		// prevent multiple insert of source files
		nsl = npc_src_first;
		while (nsl)
		{   // found the file, no need to insert it again
			if (0 == strcmp(name, nsl->name))
				return;
			nsl = nsl->next;
		}
		
		len = sizeof(struct npc_src_list) + strlen(name); // space for eos is already in npc_src_list
		nsl = (struct npc_src_list *)malloc(len * sizeof(char));
		nsl->next = NULL;
		memcpy(nsl->name, name, strlen(name)+1);//EOF included
		if (npc_src_first == NULL)
			npc_src_first = nsl;
		if (npc_src_last)
			npc_src_last->next = nsl;
		npc_src_last = nsl;
	}
}
/*==========================================
 * 読み込むnpcファイルの削除
 *------------------------------------------
 */
void npc_delsrcfile(const char *name)
{
	if(name)
	{
		struct npc_src_list *p=npc_src_first;
		struct npc_src_list *pp=NULL;

		if ( strcasecmp(name,"all")==0 )
		{
			npc_clearsrcfile();
			return;
		}
		while( p )
		{
			if ( strcmp(p->name,name)==0 )
			{
				if(pp)
					pp->next=p->next;
				if(p==npc_src_first)
					npc_src_first = p->next;
				if(p==npc_src_last)
				npc_src_last = pp;
				free(p);
				break;
			}
			pp = p;
			p = p->next;
		}
	}
}

/*==========================================
 * warp行解析
 *------------------------------------------
 */
bool npc_parse_warp(const char *w1,const char *w2,const char *w3,const char *w4)
{
	int x, y, xs, ys, xt, yt, m;
	int i, j;
	char mapname[32], to_mapname[32], *ip;
	

	// 引数の個数チェック
	if( sscanf(w1, "%32[^,],%d,%d", mapname, &x, &y) != 3 ||
		sscanf(w4, "%d,%d,%32[^,],%d,%d", &xs, &ys, to_mapname, &xt, &yt) != 5)
	{
		ShowError("bad warp line : %s\n", w3);
		return false;
	}
	ip = strchr(mapname, '.');
	if(ip) *ip=0;
	basics::itrim(mapname);
	ip = strchr(to_mapname, '.');
	if(ip) *ip=0;
	basics::itrim(to_mapname);

	m = map_mapname2mapid(mapname);

	npcwarp_data *nd = new npcwarp_data(); 

	nd->block_list::id = npc_get_new_npc_id();
	nd->n = map_addnpc(m, nd);
	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;
	nd->invalid = 0;
	memcpy(nd->name, w3, 24);
	memcpy(nd->exname, w3, 24);

	if (!config.warp_point_debug)
		nd->class_ = WARP_CLASS;
	else
		nd->class_ = WARP_DEBUG_CLASS;
	nd->speed = 200;
	nd->option = 0;
	nd->opt1 = 0;
	nd->opt2 = 0;
	nd->opt3 = 0;
	safestrcpy(nd->mapname, sizeof(nd->mapname), to_mapname);
	xs += 2;
	ys += 2;
	nd->xt = xt;
	nd->yt = yt;
	nd->xs = xs;
	nd->ys = ys;

	for (i = 0; i < ys; ++i) {
		for (j = 0; j < xs; ++j) {
			if (map_getcell(m, x-xs/2+j, y-ys/2+i, CELL_CHKNOPASS))
				continue;
			map_setcell(m, x-xs/2+j, y-ys/2+i, CELL_SETNPC);
		}
	}

//	ShowMessage("warp npc %s %d read done\n",mapname,nd->block_list::id);
	++npc_warp;
	nd->addblock();
	clif_spawnnpc(*nd);
	strdb_insert(npcname_db, nd->name, nd);

	return true;
}

/*==========================================
 * shop行解析
 *------------------------------------------
 */
int npc_parse_shop(const char *w1,const char *w2,const char *w3,const char *w4)
{
	const char *p;
	int x, y, dir, m, pos = 0;
	char mapname[32], *ip;
	npcshop_data *nd;
	npc_item shopitems[MAX_SHOPITEM];

	if (strcmp(w1, "-") == 0)
	{
		x = 0; y = 0; dir = 0; m = -1;
	}
	else
	{	// 引数の個数チェック
		if (sscanf(w1, "%32[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
			strchr(w4, ',') == NULL) {
			ShowError("bad shop line : %s\n", w3);
			return 1;
		}
		ip = strchr(mapname, '.');
		if(ip) *ip=0;
		basics::itrim(mapname);
		m = map_mapname2mapid(mapname);
	}

	p = strchr(w4, ',');
	while (p && pos < MAX_SHOPITEM)
	{
		int nameid, price;
		struct item_data *id;
		p++;
		if (sscanf(p, "%d:%d", &nameid, &price) != 2)
			break;
		shopitems[pos].nameid = nameid;
		id = itemdb_search(nameid);
		if (price < 0)			
			price = id->value_buy;
		shopitems[pos].price = price;
		// check for bad prices that can possibly cause exploits
		if (price*75/100 < id->value_sell*124/100)
		{
			ShowWarning ("Item %s [%d] buying price (%d) is less than selling price (%d)\n",
				id->name, id->nameid, price*75/100, id->value_sell*124/100);

		}
		pos++;
		p = strchr(p, ',');
	}
	if (pos == 0)
	{
		return 1;
	}

	nd = new npcshop_data(shopitems, pos); 

	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;
	nd->block_list::id = npc_get_new_npc_id();
	nd->init_dir(dir_t(dir&0x07),dir_t(dir&0x07));
	nd->invalid = 0;
	memcpy(nd->name, w3, 24);
	nd->class_ = (m==-1)?0:atoi(w4);
	nd->speed = 200;
	nd->option = 0;
	nd->opt1 = 0;
	nd->opt2 = 0;
	nd->opt3 = 0;

	//ShowMessage("shop npc %s %d read done\n",mapname,nd->block_list::id);
	++npc_shop;

	nd->n = map_addnpc(m,nd);
	if (m >= 0)
	{
		nd->addblock();
		clif_spawnnpc(*nd);
	}
	else
		nd->addiddb();
	strdb_insert(npcname_db, nd->name,nd);

	return 0;
}


/*==========================================
 * script行解析
 *------------------------------------------
 */
int npc_parse_script(const char *w1,const char *w2,const char *w3,const char *w4,const char *first_line,FILE *fp,int *lines, npc_data **dummy_npc)
{
	int x, y, dir = 0, m, xs = 0, ys = 0, class_ = 0;	// [Valaris] thanks to fov
	char mapname[32], *ip;
	unsigned char *srcbuf=NULL;
	size_t srcsize=65536;
	int startline = 0;
	unsigned char line[1024];
	size_t i;

	int evflag = 0;
	const char *p;
	struct script_object *ref=NULL;

	if(strcmp(w1,"-")==0)
	{
		x=0;
		y=0;
		m=-1;
	}
	else
	{
		// 引数の個数チェック
		if (sscanf(w1, "%32[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
		   ( strcmp(w2,"script")==0 && strchr(w4,',')==NULL) ) 
		{
			ShowMessage("bad script line : %s\n",w3);
			return 1;
		}
		ip = strchr(mapname, '.');
		if(ip) *ip=0;
		basics::itrim(mapname);
		m = map_mapname2mapid(mapname);
	}

	if(strcmp(w2,"script")==0)
	{

		// スクリプトの解析
		srcbuf = new unsigned char[srcsize];
		if (strchr(first_line,'{')) 
		{
			strcpy((char*)srcbuf,strchr((char*)first_line,'{'));
			startline = *lines;
		}
		else
			srcbuf[0] = 0;

		while(1) 
		{
			for(i=strlen((char*)srcbuf)-1; i>=0 && isspace((int)((unsigned char)srcbuf[i])); i--);
			
			if (i >= 0 && srcbuf[i] == '}')
				break;
			fgets ((char *)line, sizeof(line), fp);
			(*lines)++;
			if (feof(fp))
				break;

			if (strlen((char *) srcbuf)+strlen((char *) line)+1>=srcsize) 
			{
				srcsize = new_realloc(srcbuf,srcsize,65536);
			}
			if (srcbuf[0]!='{') 
			{
				if (strchr((char *) line,'{')) 
				{
					strcpy((char*)srcbuf,strchr((char*)line,'{'));
					startline = *lines;
				}
			}
			else
				strcat((char*)srcbuf,(char*)line);
		}//end while

		ref = parse_script(srcbuf,startline);
		if(!ref)
		{	// script parse error?
			delete[] srcbuf;
			return 1;
		}
	}
	else
	{
		// duplicateする
		char srcname[128];
		npcscript_data *sc2;

		// duplication not on this map server, can skip
		if(m<0)	return 1;

		if( sscanf(w2,"duplicate(%128[^)])",srcname)!=1 )
		{
			ShowError("bad duplicate name! : %s",w2);
			return 1;
		}
		if( (sc2=npcscript_data::from_name(srcname))==NULL )
		{
			ShowError("bad duplicate name! (not exist) : %s\n",srcname);
			return 1;
		}
		if( sc2->ref==NULL )
		{
			ShowError("nothing to duplicate! : %s\n",srcname);
			return 1;
		}
		ref = sc2->ref->clone();
	}// end of スクリプト解析

	npcscript_data *nd = new npcscript_data();

	if(dummy_npc) *dummy_npc=nd;

	if(m<0 || dummy_npc)
	{	// スクリプトコピー用のダミーNPC
	}
	else if( sscanf(w4,"%d,%d,%d",&class_,&xs,&ys)==3) 
	{	// 接触型NPC
		int i, j;

		if (xs >= 0) xs = xs * 2 + 1;
		if (ys >= 0) ys = ys * 2 + 1;

		if (class_>=0)
		{
			for(i=0;i<ys;++i)
			for(j=0;j<xs;++j) 
			{
				if (map_getcell(m, x - xs/2 + j, y - ys/2 + i, CELL_CHKNOPASS))
					continue;
				map_setcell(m, x - xs/2 + j, y - ys/2 + i, CELL_SETNPC);
			}
		}
		nd->xs = xs;
		nd->ys = ys;
	} 
	else
	{	// クリック型NPC
		class_ = atoi(w4);
		nd->xs = 0;
		nd->ys = 0;
	}
	if (class_<0 && m>=0 && !dummy_npc)
	{	// イベント型NPC
		evflag = 1;
	}

	while((p=strchr(w3,':'))) 
	{
		if (p[1] == ':') break;
	}
	if (p) 
	{	
		size_t len = p-w3;
		memcpy(nd->name, w3, p-w3);
		nd->name[len]=0;
		memcpy(nd->exname, p+2, 1+strlen(p+2));
	}
	else
	{
		memcpy(nd->name, w3, 24);
		memcpy(nd->exname, w3, 24);
	}

	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;
	nd->block_list::id = (!dummy_npc) ? npc_get_new_npc_id() : 0;
	nd->init_dir(dir_t(dir&0x07),dir_t(dir&0x07));
	nd->invalid = 0;
	nd->class_ = class_;
	nd->speed = 200;

	nd->ref=ref;

	nd->timer_event=NULL;
	nd->chat = NULL;
	nd->option = 0;
	nd->opt1 = 0;
	nd->opt2 = 0;
	nd->opt3 = 0;
	nd->nexttimer=-1;
	nd->timerid=-1;

	//ShowMessage("script npc %s %d %d read done\n",mapname,nd->block_list::id,nd->class_);
	if(!dummy_npc) ++npc_script;

	if(m>=0 && !dummy_npc)
	{
		nd->n = map_addnpc(m, nd);
		nd->addblock();

		if (evflag) 
		{	// イベント型
			event_data *ev = new struct event_data(nd,0);
			strdb_insert(ev_db, nd->exname, ev);
		}
		else
			clif_spawnnpc(*nd);
	}
	
	strdb_insert(npcname_db, nd->exname, nd);

	//-----------------------------------------
	// ラベルデータの準備
	if(srcbuf)
	{
		// もう使わないのでバッファ解放
		delete[] srcbuf;
	}


	//-----------------------------------------
	// イベント用ラベルデータのエクスポート
	if(!dummy_npc)
	for(i=0;i<nd->ref->label_count();++i)
	{
		const script_object::script_label& label = nd->ref->get_label(i);

		// export event labels (all labels starting with "On")
		if( basics::upcase(label.name[0])=='O' && basics::upcase(label.name[1])=='N' ) 
		{
			// エクスポートされる
			char *buf=new char[(3+strlen(nd->exname)+strlen(label.name))];
			sprintf(buf,"%s::%s",nd->exname,label.name); // buffer size is exact, so snprintf is unnecesary
			struct event_data *ev = (struct event_data *)strdb_search(ev_db,buf);
			if(ev != NULL)
			{
				delete[] buf;
				ShowError("npc_parse_script : duplicate event %s\n",buf);
			}
			else
			{
				ev = new struct event_data(nd,label.pos);
				strdb_insert(ev_db,buf,ev);
			}
		}

		// ラベルデータからタイマーイベント取り込み
		int t = 0, k = 0;
		if(sscanf(label.name,"OnTimer%d%n",&t,&k)==1 && label.name[k]=='\0')
		{
			// タイマーイベント
			struct npc_timerevent_list *te = nd->timer_event;
			int j, k = nd->timeramount;

			new_realloc(te,k,1);
			for(j=0;j<k;++j)
			{
				if(te[j].timer>t)
				{
					memmove(te+j+1, te+j, sizeof(npc_timerevent_list)*(k-j));
					break;
				}
			}
			te[j].timer = t;
			te[j].pos = label.pos;
			nd->timer_event = te;
			nd->timeramount = k+1;
		}

	}
	return 0;
}

/*==========================================
 * function行解析
 *------------------------------------------
 */
bool npc_parse_function(const char *w1,const char *w2,const char *w3,const char *w4,const char *first_line,FILE *fp,int *lines)
{
	bool ret = false;
	int i, startline = 0;
	char line[1024];
	size_t srcsize=65536;
	char *srcbuf= new char[srcsize];
	
	if (strchr(first_line,'{'))
	{
		strcpy(srcbuf, strchr(first_line,'{'));
		startline = *lines;
	}
	else
		srcbuf[0] = 0;

	while (1)
	{
		for(i=strlen(srcbuf)-1;i>=0 && isspace((int)((unsigned char)srcbuf[i]));i--);
		if (i >= 0 && srcbuf[i] == '}')
			break;
		fgets(line, sizeof(line), fp);
		(*lines)++;
		if (feof(fp))
			break;
		if (strlen(srcbuf)+strlen(line)+1 >= srcsize)
		{
			srcsize = new_realloc(srcbuf, srcsize, 65536);
		}
		if (srcbuf[0]!='{') {
			if (strchr(line,'{')) {
				strcpy(srcbuf, strchr(line,'{'));
				startline = *lines;
			}
		} else
			strcat(srcbuf,line);
	}
	if( strdb_search(script_get_userfunc_db(), w3) )
	{
		ShowMessage("\n");
		ShowWarning("Duplicate function [%s] (line %d)\n", w3, (lines)?*lines:0);
	}
	else
	{
		
		script_object *script=parse_script((unsigned char*)(srcbuf),startline);
		if(script)
		{
			char *p= new char[1+strlen(w3)];
			memcpy(p,w3,1+strlen(w3));
			strdb_insert(script_get_userfunc_db(), p, script);
			ret = true;
		}
	}

	// もう使わないのでバッファ解放
	delete[] srcbuf;
	return ret;
}


/*==========================================
 * Parse Mob 1 - Parse mob list into each map
 * Parse Mob 2 - Actually Spawns Mob
 * [Wizputer]
 * If cached =1, it is a dynamic cached mob
 *------------------------------------------
 */
int npc_parse_mob2(struct mob_list &mob)
{
	struct mob_data *md;
    register size_t i;

	for(i = 0; i < mob.num; ++i)
	{
		md = new mob_data(mob.mobname, mob.class_);

		md->block_list::m = mob.m;
		md->block_list::x = mob.x0;
		md->block_list::y = mob.y0;
		md->addiddb();

		md->level = mob.level;

		md->cache = &mob;

		if( strlen(mob.eventname) >= 4 )
		{
			memcpy(md->npc_event, mob.eventname, 24);
		}
		else
		{
			memset(md->npc_event, 0, 24);
			if (strlen(mob.eventname) == 1)
			{	//Portable monster big/small implementation. [Skotlex]
				int size = atoi(mob.eventname);
				if (size & 2)
					md->state.size=1;
				else if (size & 4)
					md->state.size=2;
			}
		}
		
		mob_spawn(md->block_list::id);
	}
	// all mobs from cache are spawned now
	mob.num = 0;

	return 0;
}

int npc_parse_mob(const char *w1, const char *w2, const char *w3, const char *w4)
{
	int level;
	char *ip;
	char mapname[64]="", mobname[64]="", eventname[64]="";
	int v1,v2,v3,v4,v5,v6,v7,v8;
	unsigned short m;

	
	// 引数の個数チェック
	if (sscanf(w1, "%64[^,],%d,%d,%d,%d", mapname, &v1, &v2, &v3, &v4) < 3 ||
		sscanf(w4, "%d,%d,%d,%d,%64s", &v5, &v6, &v7, &v8, eventname) < 2 ) {
		ShowError("bad monster line : %s\n", w3);
		return 1;
	}
	ip = strchr(mapname, '.');
	if(ip) *ip=0;
	basics::itrim(mapname);
	basics::itrim(eventname);

	m = map_mapname2mapid(mapname);
	if(m >= MAX_MAP_PER_SERVER)
		return 1;
	if(v5 <= 1000 || v5 > MAX_MOB_DB) // class check
		return 1;
		
	struct mob_list *dynmob = map_addmobtolist(m);
	if( !dynmob )
	{
		ShowError("no place for mob cache on map: %s\n", maps[m].mapname);
		return 1;
	}
	
	dynmob->m		= m;
	dynmob->x0		= v1;
	dynmob->y0		= v2;
	dynmob->xs		= v3;
	dynmob->ys		= v4;
	dynmob->class_	= v5;
	dynmob->num		= v6;
	dynmob->delay1	= v7;
	dynmob->delay2	= v8;

	if( dynmob->num > 1 && config.mob_count_rate != 100)
	{
		if((dynmob->num = dynmob->num * config.mob_count_rate / 100) < 1)
			dynmob->num = 1;
	}
	
	if (sscanf(w3, "%64[^,],%d", mobname, &level) > 1)
		dynmob->level = level;
	if (strcmp(mobname, "--en--") == 0)
		memcpy(dynmob->mobname, mob_db[dynmob->class_].name, 24);
	else if (strcmp(mobname, "--ja--") == 0)
		memcpy(dynmob->mobname, mob_db[dynmob->class_].jname, 24);
	else 
		memcpy(dynmob->mobname, mobname, 24);
	memcpy(dynmob->eventname, eventname, 24);

	if( dynmob->delay1 || dynmob->delay2 || !config.dynamic_mobs )
	{	// delayed mobs are always created
	    npc_parse_mob2(*dynmob);
		npc_delay_mob += v6;
		}
	else
	{	// others can be cached
		npc_cache_mob += v6;
	}
	npc_mob+=v6;
	return 0;
}

/*==========================================
 * マップフラグ行の解析
 *------------------------------------------
 */
int npc_parse_mapflag(const char *w1,const char *w2,const char *w3,const char *w4)
{
	int m;
	char mapname[32], *ip;

	// 引数の個数チェック
	if (sscanf(w1, "%32[^,]",mapname) != 1)
		return 1;
	ip = strchr(mapname,'.');
	if(ip) *ip=0;
	basics::itrim(mapname);
	m = map_mapname2mapid(mapname);
	if (m < 0)
		return 1;

	//マップフラグ
	if ( strcasecmp(w3,"nosave")==0) {
		char savemap[32];
		int savex, savey;
		if (strcmp(w4, "SavePoint") == 0) {
			safestrcpy(maps[m].save.mapname, sizeof(maps[m].save.mapname), "SavePoint");
			maps[m].save.x = -1;
			maps[m].save.y = -1;
		} else if (sscanf(w4, "%32[^,],%d,%d", savemap, &savex, &savey) == 3) {
			char*ip = strchr(savemap, '.');
			if(ip) *ip=0;
			safestrcpy(maps[m].save.mapname, sizeof(maps[m].save.mapname), savemap);			
			maps[m].save.x = savex;
			maps[m].save.y = savey;
		}
		maps[m].flag.nosave = 1;
	}
	else if (strcasecmp(w3,"nomemo")==0) {
		maps[m].flag.nomemo=1;
	}
	else if (strcasecmp(w3,"noteleport")==0) {
		maps[m].flag.noteleport=1;
	}
	else if (strcasecmp(w3,"nowarp")==0) {
		maps[m].flag.nowarp=1;
	}
	else if (strcasecmp(w3,"nowarpto")==0) {
		maps[m].flag.nowarpto=1;
	}
	else if (strcasecmp(w3,"noreturn")==0) {
		maps[m].flag.noreturn=1;
	}
	else if (strcasecmp(w3,"monster_noteleport")==0) {
		maps[m].flag.monster_noteleport=1;
	}
	else if (strcasecmp(w3,"nobranch")==0) {
		maps[m].flag.nobranch=1;
	}
	else if (strcasecmp(w3,"nopenalty")==0) {
		maps[m].flag.nopenalty=1;
	}
	else if (strcasecmp(w3,"pvp")==0) {
		maps[m].flag.pvp=1;
	}
	else if (strcasecmp(w3,"pvp_noparty")==0) {
		maps[m].flag.pvp_noparty=1;
	}
	else if (strcasecmp(w3,"pvp_noguild")==0) {
		maps[m].flag.pvp_noguild=1;
	}
	else if (strcasecmp(w3,"pvp_nightmaredrop")==0) {
		char drop_arg1[16], drop_arg2[16];
		int drop_id = 0, drop_type = 0, drop_per = 0;
		if( sscanf(w4, "%16[^,],%16[^,],%d", drop_arg1, drop_arg2, &drop_per) == 3 )
		{
			int i;
			basics::itrim(drop_arg1);
			basics::itrim(drop_arg2);
			
			if (strcmp(drop_arg1, "random") == 0)
				drop_id = -1;
			else if (itemdb_exists((drop_id = atoi(drop_arg1))) == NULL)
				drop_id = 0;
			if (strcmp(drop_arg2, "inventory") == 0)
				drop_type = 1;
			else if (strcmp(drop_arg2,"equip") == 0)
				drop_type = 2;
			else if (strcmp(drop_arg2,"all") == 0)
				drop_type = 3;

			if (drop_id != 0){
				for (i = 0; i < MAX_DROP_PER_MAP; ++i) {
					if (maps[m].drop_list[i].drop_id == 0){
						maps[m].drop_list[i].drop_id = drop_id;
						maps[m].drop_list[i].drop_type = drop_type;
						maps[m].drop_list[i].drop_per = drop_per;
						break;
					}
				}
				maps[m].flag.pvp_nightmaredrop = 1;
			}
		}
	}
	else if (strcasecmp(w3,"pvp_nocalcrank")==0) {
		maps[m].flag.pvp_nocalcrank=1;
	}
	else if (strcasecmp(w3,"gvg")==0) {
		maps[m].flag.gvg=1;
	}
	else if (strcasecmp(w3,"gvg_noparty")==0) {
		maps[m].flag.gvg_noparty=1;
	}
	else if (strcasecmp(w3,"gvg_dungeon")==0) {
		maps[m].flag.gvg_dungeon=1;
	}
	else if (strcasecmp(w3,"nozenypenalty")==0) {
		maps[m].flag.nozenypenalty=1;
	}
	else if (strcasecmp(w3,"notrade")==0) {
		maps[m].flag.notrade=1;
	}
	else if (strcasecmp(w3,"noskill")==0) {
		maps[m].flag.noskill=1;
	}
	else if (config.pk_mode && strcasecmp(w3,"nopvp")==0) { // nopvp for pk mode [Valaris]
		maps[m].flag.nopvp=1;
		maps[m].flag.pvp=0;
	}
	else if (strcasecmp(w3,"noicewall")==0) { // noicewall [Valaris]
		maps[m].flag.noicewall=1;
	}
	else if (strcasecmp(w3,"snow")==0) { // snow [Valaris]
		maps[m].flag.snow=1;
	}
	else if (strcasecmp(w3,"clouds")==0) {
		maps[m].flag.clouds=1;
	}
	else if (strcasecmp(w3,"clouds2")==0) {
		maps[m].flag.clouds2=1;
	}
	else if (strcasecmp(w3,"fog")==0) { // fog [Valaris]
		maps[m].flag.fog=1;
	}
	else if (strcasecmp(w3,"fireworks")==0) {
		maps[m].flag.fireworks=1;
	}
	else if (strcasecmp(w3,"sakura")==0) { // sakura [Valaris]
		maps[m].flag.sakura=1;
	}
	else if (strcasecmp(w3,"leaves")==0) { // leaves [Valaris]
		maps[m].flag.leaves=1;
	}
	else if (strcasecmp(w3,"rain")==0) { // rain [Valaris]
		maps[m].flag.rain=1;
	}
	else if (strcasecmp(w3,"indoors")==0) { // celest
		maps[m].flag.indoors=1;
	}
	else if (strcasecmp(w3,"nogo")==0) { // celest
		maps[m].flag.nogo=1;
	}
	else if (strcasecmp(w3,"noexp")==0) { // Lorky
		maps[m].flag.nobaseexp=1;
		maps[m].flag.nojobexp=1;
	}
	else if (strcasecmp(w3,"nobaseexp")==0) { // Lorky
		maps[m].flag.nobaseexp=1;
	}
	else if (strcasecmp(w3,"nojobexp")==0) { // Lorky
		maps[m].flag.nojobexp=1;
	}
	else if (strcasecmp(w3,"noloot")==0) { // Lorky
		maps[m].flag.nomobloot=1;
		maps[m].flag.nomvploot=1;
	}
	else if (strcasecmp(w3,"nomobloot")==0) { // Lorky
		maps[m].flag.nomobloot=1;
	}
	else if (strcasecmp(w3,"nomvploot")==0) { // Lorky
		maps[m].flag.nomvploot=1;
	}

	return 0;
}

/*==========================================
 * Setting up map cells
 *------------------------------------------
 */
int npc_parse_mapcell(const char *w1,const char *w2,const char *w3,const char *w4)
{
	int m, cell, x, y, x0, y0, x1, y1;
	char type[32], mapname[32], *ip;

	if (sscanf(w1, "%32[^,]", mapname) != 1)
		return 1;

	ip = strchr(mapname,'.');
	if(ip) *ip=0;
	basics::itrim(mapname);
	m = map_mapname2mapid(mapname);
	if (m < 0)
		return 1;

	if (sscanf(w3, "%32[^,],%d,%d,%d,%d", type, &x0, &y0, &x1, &y1) < 4) {
		ShowError("Bad setcell line : %s\n",w3);
		return 1;
	}
	cell = strtol(type, (char **)NULL, 0);
	//ShowMessage("0x%x %d %d %d %d\n", cell, x0, y0, x1, y1);

	if (x0 > x1) basics::swap(x0,x1);
	if (y0 > y1) basics::swap(y0,y1);

	for (x = x0; x <= x1; ++x)
	for (y = y0; y <= y1; ++y)
	{
			map_setcell(m, x, y, cell);
		//ShowMessage("setcell 0x%x %d %d %d\n", cell, m, x, y);
		}

	return 0;
}

void npc_parsesinglefile(const char *filename, struct npc_mark*& npcmarkerbase)
{
	int m, lines = 0;
	char line[1024];
	char w1[1024], w2[1024], w3[1024], w4[1024], mapname[1024], *ip;
	unsigned int i, j;
	int count, w4pos;
	FILE *fp;

	fp = basics::safefopen(filename,"r");
	if (fp==NULL)
	{
		ShowError("File not found : %s\n",filename);
	}
	else
	{
		ShowMessage("\rLoading NPCs [%d]: %s"CL_CLL,get_npc_id()-START_NPC_NUM,filename);
		while( fgets(line, sizeof(line), fp) )
		{
			lines++;
			if( !is_valid_line(line) )
				continue;

			// 不要なスペースやタブの連続は詰める
			for (i = j = 0; line[i]; ++i)
			{
				if (line[i]==' ')
				{
					if(!((line[i+1] && (isspace((int)((unsigned char)line[i+1])) || line[i+1]==',')) ||
					 (j && line[j-1]==',')))
					line[j++]=' ';
				}
				else if (line[i]=='\t')
				{
				if (!(j && line[j-1]=='\t'))
					line[j++]='\t';
				}
				else
				line[j++]=line[i];
			}
			line[j]=0;
			// 最初はタブ区切りでチェックしてみて、ダメならスペース区切りで確認
			if( (count = sscanf(line,"%1024[^\t]\t%1024[^\t]\t%1024[^\t\r\n]\t%n%1024[^\t\r\n]", w1, w2, w3, &w4pos, w4)) < 3 &&
				(count = sscanf(line,"%1024s%1024s%1024s%n%1024s", w1, w2, w3, &w4pos, w4)) < 3)
			{
				continue;
			}
			basics::itrim(w1);
			basics::itrim(w2);
			basics::itrim(w3);
			basics::itrim(w4);

			// マップの存在確認
			if( strcmp(w1,"-")!=0 && strcasecmp(w1,"function")!=0 )
			{
				sscanf(w1,"%1024[^,]",mapname);
				ip = strchr(mapname,'.');
				if(ip) *ip=0;
				basics::itrim(mapname);
				m = map_mapname2mapid(mapname);
				if( strlen(mapname)>16 || m<0 )
				{	// "mapname" is not assigned to this server
					m = -1;
				}
			}
			else
				m=0;

			if (strcasecmp(w2,"warp")==0 && count > 3 && m>=0)
			{
				npc_parse_warp(w1,w2,w3,w4);
			}
			else if (strcasecmp(w2,"shop")==0 && count > 3 && m>=0)
			{
				npc_parse_shop(w1,w2,w3,w4);
			}
			else if (strcasecmp(w2,"script")==0 && count > 3)
			{
				if( strcasecmp(w1,"function")==0 )
				{
					npc_parse_function(w1,w2,w3,w4,line+w4pos,fp,&lines);
				}
				else
				{
					if( m>=0 )
						npc_parse_script(w1,w2,w3,w4,line+w4pos,fp,&lines, NULL);
					else
					{	// pre-load the npc, delete it if not used
						npc_data *nd=NULL;
						int ret = npc_parse_script(w1,w2,w3,w4,line+w4pos,fp,&lines, &nd);
						// mark loaded scripts that not reside on this map server
						if(0==ret && nd)
						{
							npcmarkerbase = new struct npc_mark(nd, npcmarkerbase);
						}
					}
				}
			}
			else if ( (i=0,sscanf(w2,"duplicate%n",&i), (i>0 && w2[i]=='(')) && count > 3 && m>=0)
			{
				npc_parse_script(w1,w2,w3,w4,line+w4pos,fp,&lines,NULL);
			}
			else if (strcasecmp(w2,"monster")==0 && count > 3 && m>=0)
			{
				npc_parse_mob(w1,w2,w3,w4);
			}
			else if (strcasecmp(w2,"mapflag")==0 && count >= 3 && m>=0)
			{
				npc_parse_mapflag(w1,w2,w3,w4);
			}
			else if (strcasecmp(w2,"setcell") == 0 && count >= 3)
			{
				npc_parse_mapcell(w1,w2,w3,w4);
			}
		}
		fclose(fp);
	}
}


void npc_parsesrcfiles()
{
	struct npc_mark *npcmarkerbase=NULL;
	struct npc_mark *marker;
	
	struct npc_src_list *nsl;

	for (nsl = npc_src_first; nsl; nsl = nsl->next) 
	{
		npc_parsesinglefile(nsl->name, npcmarkerbase);

	}//end for(nsl)

	// clear and delete marked npcs, 
	// the npc script will stay if it was referenced by duplication at least once
	while(npcmarkerbase)
	{
		marker = npcmarkerbase;
		npcmarkerbase = npcmarkerbase->next;
		if(marker->nd)
		{
			strdb_erase(npcname_db,marker->nd->exname);
			ShowMessage("\rDeleting unused NPC: %s", marker->nd->exname);
			npc_unload(marker->nd);
			// marker->nd is freed now
			delete marker;
		}
	}
	ShowMessage("\r"CL_CLL);
	ShowStatus("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:%30s\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Warps\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Shops\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Scripts\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs on Map\n\t\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs cached\n",
		get_npc_id()-START_NPC_NUM,"",npc_warp,npc_shop,npc_script,npc_mob, npc_delay_mob, npc_cache_mob);

	return;
}
void npc_parsesrcfile(const char*filename)
{	
	struct npc_mark *npcmarkerbase=NULL;
	struct npc_mark *marker;

	npc_parsesinglefile(filename, npcmarkerbase);

	// clear and delete marked npcs, 
	// the npc script will stay if it was referenced by duplication at least once
	while(npcmarkerbase)
	{
		marker = npcmarkerbase;
		npcmarkerbase = npcmarkerbase->next;
		if(marker->nd)
		{
			strdb_erase(npcname_db,marker->nd->exname);
			ShowMessage("\rDeleting unused NPC: %s", marker->nd->exname);
			npc_unload(marker->nd);
			// marker->nd is freed now
			delete marker;
		}
	}
	ShowMessage("\r"CL_CLL);

	return;
}

int npc_read_indoors (void)
{
	int s, m;
	char *p, *buf;

	buf = (char *)grfio_read("data\\indoorrswtable.txt", s);
	if(buf)
	{
		buf[s] = 0;
		p = buf;
		while( p && *p && (p<buf+s) )
		{
		char mapname[64];
		if (sscanf(p, "%64[^#]#", mapname) == 1)
		{
			char* ip = strchr(mapname, '.');
			if(ip) *ip=0;
			if ((m = map_mapname2mapid(mapname)) >= 0)
				maps[m].flag.indoors = 1;
		}
			p=strchr(p, '\n');
		if (!p) break;
		p++;
		delete[] buf;
	}
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\indoorrswtable.txt");
	return 0;
}
	return -1;
}

void ev_db_final (void *key,void *data)
{
	if(data)
	{
		delete ((struct event_data*)data);
	}
	if(key && strstr((char*)key,"::")!=NULL)
	{
		delete[] ((char*)key);
	}
}


void npcname_db_final (void *key,void *data)
{
	npc_data *nd = (npc_data *) data;
	npc_unload(nd, false);// we are inside the db function and cannot call erase from here
}

/*==========================================
 * 
 *------------------------------------------
 */
/*
int npc_cleanup_sub (block_list &bl, va_list &ap)
{
	switch(bl.type) {
	case BL_NPC:
		npc_unload((npc_data *)&bl);
		break;
	case BL_MOB:
		mob_unload((struct mob_data &)bl);
		break;
	}
	return 0;
}
*/
class CNpcCleanup : public CMapProcessor
{
public:
	CNpcCleanup()	{}
	~CNpcCleanup()	{}
	virtual int process(block_list& bl) const
	{
		npc_data *nd;
		mob_data *md;
		if( (nd=bl.get_nd()) )
			npc_unload(nd);
		else if( (md=bl.get_md()) )
			mob_unload(*md);
		return 0;
	}
};
int npc_reload (void)
{
	size_t m;

	for (m = 0; m < map_num; ++m)
	{
		block_list::foreachinarea( CNpcCleanup(), m, 0, 0, maps[m].xs-1, maps[m].ys-1, BL_ALL);
		clear_moblist(m);
		maps[m].npc_num = 0;
	}
	if(ev_db)
		strdb_final(ev_db,ev_db_final);
	if(npcname_db)
		strdb_final(npcname_db,npcname_db_final);

	// anything else we should cleanup?
	// Reloading npc's now
	ev_db = strdb_init(51);
	npcname_db = strdb_init(24);
	ev_db->release = ev_release;
	npc_warp = npc_shop = npc_script = 0;
	npc_mob = npc_cache_mob = npc_delay_mob = 0;
	
	npc_parsesrcfiles( );

	//Execute the OnInit event for freshly loaded npcs. [Skotlex]
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
	CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_event_doall("OnInit"));

	return 0;
				}

/*==========================================
 * 終了
 *------------------------------------------
 */
int npc_remove_map (npc_data *nd)
{
    if(!nd || !nd->is_on_map() )
		return 1;

	//Remove corresponding NPC CELLs
	if( nd->xs || nd->ys )
	{
		int i,j, xs, ys, x, y;
		ushort m=nd->block_list::m;
		x = nd->block_list::x;
		y = nd->block_list::y;
		xs = nd->xs;
		ys = nd->ys;

		for (i = 0; i < ys; ++i)
		for (j = 0; j < xs; ++j)
		{
			if (map_getcell(m, x-xs/2+j, y-ys/2+i, CELL_CHKNPC))
				map_setcell(m, x-xs/2+j, y-ys/2+i, CELL_CLRNPC);
		}
	}

    clif_clearchar_area(*nd,2);
    nd->delblock();
	nd->deliddb();

    return 0;
}


int npc_unload(npc_data *nd, bool erase_strdb)//erase_strdb is default true 
{
	if(!nd) return 0;

	npc_remove_map(nd);

	npcscript_data *sc = nd->get_script();
	if(sc)
	{
		npcchat_data::erase(*sc);
		npclisten_finalize(sc);

		if (sc->timer_event)
		{
			delete (sc->timer_event);
			sc->timer_event=NULL;
		}
		if(sc->ref)
		{
			sc->ref->release();
			sc->ref=NULL;
		}
		//Clean up all related events.
		db_iterator<char*, event_data *> iter(ev_db);
		for(; iter; ++iter)
		{
			char * key     = iter.key();
			event_data *ev = iter.data();
			if( key && ev && ev->sc == sc )
			{
				strdb_erase(ev_db, key);
				delete ev;
				if( strstr(key,"::") != NULL )
					delete[] (key);
			}
		}

		// quite inconsistent, but:
		// script npc's have 'exname' in the db
		if(erase_strdb)
			strdb_erase(npcname_db, sc->exname);
	}
	else
	{	// shop/warp npc's have 'name' in the db
		if(erase_strdb)
			strdb_erase(npcname_db, nd->name);
	}

	// unlink from map, if exist there
	if( nd->block_list::m<map_num && nd->n >=0 && nd->n< MAX_NPC_PER_MAP &&
		maps[nd->block_list::m].npc[nd->n]==nd )
	{
		--maps[nd->block_list::m].npc_num;
		if(maps[nd->block_list::m].npc_num>0)
		{
			maps[nd->block_list::m].npc[nd->n] = maps[nd->block_list::m].npc[maps[nd->block_list::m].npc_num];
			if( maps[nd->block_list::m].npc[nd->n] )
				maps[nd->block_list::m].npc[nd->n]->n = nd->n;
		}
		else
			maps[nd->block_list::m].npc[nd->n]=NULL;
	}
	nd->freeblock(); 
	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------
 */
int do_final_npc(void)
{
	size_t i, n=0, m=0;
	block_list *bl;
	npc_data *nd;
	struct mob_data *md;
	struct pet_data *pd;

	// clear event data first, there are pointers to the npc names stored inside
	// so we have to call this before deleting the npcs
	if(ev_db)
	{
		strdb_final(ev_db, ev_db_final);
		ev_db = NULL;
	}

	for (i = START_NPC_NUM; i < get_npc_id(); ++i)
	{
		bl = block_list::from_blid(i);
		if( bl )
		{
			if( (nd = bl->get_nd()) )
			{
				npc_unload(nd);
				nd = NULL;
				n++;
			}
			else if( (md = bl->get_md()) )
			{
				mob_unload(*md);
				m++;
			}
			else if( (pd = bl->get_pd()) )
			{	// hmm, should never happen
				pd->freeblock();
			}
		}
	}
	ShowStatus("Successfully removed '"CL_WHITE"%d"CL_RESET"' NPCs and '"CL_WHITE"%d"CL_RESET"' Mobs.\n", n, m);

	// unload all functional npcs from db
	if(npcname_db)
	{
		strdb_final(npcname_db, npcname_db_final);
		npcname_db = NULL;
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

	// indoorrswtable.txt and etcinfo.txt [Celest]
	if (config.indoors_override_grffile)
		npc_read_indoors();

	ev_db=strdb_init(50);
	npcname_db = strdb_init(24);
	ev_db->release = ev_release;

	npc_parsesrcfiles( );

	add_timer_func_list(npc_event_timer,"npc_event_timer");
	add_timer_func_list(npc_event_do_clock,"npc_event_do_clock");
	add_timer_func_list(npc_timerevent,"npc_timerevent");

	return 0;
}


int npc_changename(const char *name, const char *newname, unsigned short look)
{
	npc_data *nd= (npc_data *)strdb_search(npcname_db,name);
	if(nd==NULL)
		return 0;

	safestrcpy(nd->name, sizeof(nd->name), newname);
	nd->class_ = look;

	strdb_erase(npcname_db,name);
	strdb_insert(npcname_db,newname, nd);
	
	npc_enable(name,0);
	npc_enable(name,1);

	return 0;
}
