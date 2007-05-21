// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "db.h"
#include "timer.h"
#include "nullpo.h"
#include "malloc.h"
#include "map.h"
#include "npc.h"
#include "chat.h"
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


struct npc_src_list
{
	struct npc_src_list * next;
	char name[4];
};

// single linked list of pointers to npc structures
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
struct dbt *npc_data::npcname_db=NULL;

/// name to object
npc_data* npc_data::from_name(const char *name)
{
	return (npc_data *)strdb_search(npc_data::npcname_db,name);
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
				maps[this->block_list::m].is_passable(i,k) )
			{	// remove npc ontouch area from map_cells
				maps[this->block_list::m].clr_npc(i,k);
			}
		}

		// going through new rechtangle
		for(i=xnl; i<=xnr; ++i)
		for(k=ynl; k<=ynr; ++k)
		{
			if( i>=0 && i<maps[this->block_list::m].xs &&	// inside map
				k>=0 && k<maps[this->block_list::m].ys &&
				(i<xol || i>xo_ || k<yol || k>yor) &&		// not inside of old rect
				maps[this->block_list::m].is_passable(i,k) )
			{	// add npc ontouch area to map_cells
				maps[this->block_list::m].set_npc(i,k);
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
/// static function called from map_session_data::do_walkend.
void npc_data::touch(map_session_data &sd, unsigned short m, int x,int y)
{
	if( !sd.ScriptEngine.isRunning() && m<maps.size() && maps[m].is_npc(x,y) )
	{
		size_t i;
		int xs,ys;
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
	}
	else
		sd.areanpc_id = 0;
}










/// what to do when clicked.
/// run a script
void npcscript_data::OnClick(map_session_data &sd)
{
	if(this->ref)
	{
		// run the script starting from OnClick label or from the beginning if not exists
		const uint pos = this->ref->get_labelpos("OnClick");
		CScriptEngine::run(this->ref->script, pos, sd.block_list::id, this->block_list::id);
	}
}

/// try and execute touchup 
/// test for ontouch lable or start script from beginning if none
void npcscript_data::OnTouch(block_list& bl)
{
	map_session_data *sd=bl.get_sd();
	if( sd && sd->areanpc_id != this->block_list::id &&
		this->ref )
	{
		sd->areanpc_id = this->block_list::id;

		// run the script starting from OnTouch label or from the beginning if not exists
		const uint pos = this->ref->get_labelpos("OnTouch");
		CScriptEngine::run(this->ref->script, pos, (sd)?sd->block_list::id:0, this->block_list::id);
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





///////////////////////////////////////////////////////////////////////////////
// npc eventtimer system



/// constructor.
npcscript_data::npc_timerobj::npc_timerobj(npcscript_data &n, uint32 r) :
	next(NULL),
	nd(n),
	rid(r),
	pos(0),
	tid(-1)
{	// queue at front
	this->next = this->nd.eventobj;
	this->nd.eventobj = this;
}

/// destructor.
/// dequeues and removes timers.
npcscript_data::npc_timerobj::~npc_timerobj()
{
	// remove the timer
	if(this->tid!=-1)
		delete_timer(this->tid, npcscript_data::eventtimer_entry);

	// dequeue
	if( this == this->nd.eventobj )
	{	// beeing root
		this->nd.eventobj = this->next;
	}
	else
	{	// somewhere inside the list
		npc_timerobj *a;
		for(a=this->nd.eventobj; a && a->next!=this; a=a->next);
		if(a)
		{
			a->next = a->next->next;
		}
	}
}

/// aquire a timer object. 
/// create a new object if not exists
npcscript_data::npc_timerobj* npcscript_data::eventtimer_aquire(uint32 rid)
{	// check if npc_timerobj already exists
	npc_timerobj *a;
	for(a=this->eventobj; a && a->rid!=rid; a=a->next);
	if(a)
	{	// in all cases, clear an existing timer
		if(a->tid!=-1)
		{
			delete_timer(a->tid, npcscript_data::eventtimer_entry);
			a->tid = -1;
		}
	}
	else
	{	// does not exist, so create it new
		a = new npc_timerobj(*this, rid);
	}
	return a;
}

/// get the position inside the eventtimer list.
/// return 0 when no timer has been started
ushort npcscript_data::eventtimer_getpos(uint32 rid)
{	
	npc_timerobj *a;
	for(a=this->eventobj; a && a->rid!=rid; a=a->next);
	return (a)?a->pos:0;
}

/// set the position inside the eventtimer list.
/// return the previous count. does nothing when no timer has been started
ushort npcscript_data::eventtimer_setpos(uint32 rid, ushort pos)
{	
	ushort ret = 0;
	npc_timerobj *a;
	for(a=this->eventobj; a && a->rid!=rid; a=a->next);
	if( a )
	{	
		ret = a->pos;
		a->pos = pos;
	}
	return ret;
}

/// set the rid for the next call.
/// return the previous rid. does nothing when no timer has been started
uint32 npcscript_data::eventtimer_attach(uint32 oldrid, uint32 newrid)
{	
	uint32 ret = 0;
	npc_timerobj *n, *o;
	// get the oldrid
	for(o=this->eventobj; o && o->rid!=oldrid; o=o->next);
	if( o ) 
	{
		// test if newrid already exists and delete it
		for(n=this->eventobj; n && n->rid!=newrid; n=n->next);
		if( n ) delete n;

		// attach the newrid
		ret = o->rid;
		o->rid = newrid;
	}
	return ret;
}

/// initialize a timer object and start the timer.
void npcscript_data::eventtimer_init(uint32 rid, ushort pos)
{
	npc_timerobj *a = npcscript_data::eventtimer_aquire(rid);
	// initialize
	a->pos = pos;
	// start the timer
	ulong tick = (this->ontimer_cnt>a->pos&&this->ontimer_list)?((a->pos)?(this->ontimer_list[pos].tick-this->ontimer_list[pos-1].tick):(this->ontimer_list[0].tick)):0;
	a->tid = add_timer(gettick()+tick, npcscript_data::eventtimer_entry, this->block_list::id, basics::numptr(a));
}

/// start the timer.
void npcscript_data::eventtimer_start(uint32 rid)
{
	npc_timerobj *a = npcscript_data::eventtimer_aquire(rid);
	// start the timer
	ulong tick = (this->ontimer_cnt&&this->ontimer_list)?this->ontimer_list[0].tick:0;
	a->tid = add_timer(gettick()+tick, npcscript_data::eventtimer_entry, this->block_list::id, basics::numptr(a));
}

/// stop the timer.
void npcscript_data::eventtimer_stop(uint32 rid)
{
	npcscript_data::eventtimer_aquire(rid);
	// timer removed on aquire call
}

/// clear the timers if not in use.
void npcscript_data::eventtimer_clear(uint32 rid)
{
	npc_timerobj *a;
	for(a=this->eventobj; a && a->rid!=rid; a=a->next);
	if( a && a->tid==-1 )
		delete a;
}

/// timer entry funtion.
int npcscript_data::eventtimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	npc_timerobj *eo = (npc_timerobj *)data.ptr;
	if( !eo )
	{
		ShowError("eventtimer_entry: no event object\n");
	}
	else if( tid != eo->tid )
	{
		ShowError("eventtimer_entry: invalid tid\n");
	}
	else
	{
		npcscript_data *nd=NULL;
		uint pos = 0;
		uint32 rid = 0;

		bool remove = true;
		if( eo->pos >= eo->nd.ontimer_cnt )
		{
			ShowWarning("eventtimer_entry: timer position out of bound (%i>=%i)\n", eo->pos, eo->nd.ontimer_cnt);
		}
		else
		{
			// get the current event
			struct npc_timerevent &te = eo->nd.ontimer_list[eo->pos];
			ulong difftick = te.tick;


			nd = &eo->nd;	// calling this npc
			rid = eo->rid;	// with the given rid
			pos = te.pos;	// at the script position from the current event step

			// next event
			++(eo->pos);
			
			if( eo->pos >= eo->nd.ontimer_cnt )
			{	// last event has been executed, so there is nothing to do
				eo->tid = -1;
			}
			else
			{	// start with timer for the next event
				difftick = eo->nd.ontimer_list[eo->pos].tick - difftick;
				eo->tid = add_timer(tick+difftick, npcscript_data::eventtimer_entry, id, data);
				// do not remove the object
				remove = false;
			}
		}
		// everything executed, so delete the object
		if(remove) delete eo;

		// execute the script with the local data
		if(nd && nd->ref)
			CScriptEngine::run(nd->ref->script, pos, rid, nd->block_list::id);
	}
	return 0;
}

void npcscript_data::do_ontimer(map_session_data &sd, bool start)
{
	// look for the event inside the label list of the npc
	if( this->ref )
	{
		script_object::script_label *ptr = this->ref->label_list;
		const script_object::script_label *end = this->ref->label_list+this->ref->label_list_num;
		for(; ptr && ptr<end; ++ptr)
		{
			char event[64];
			if( ptr->name && 
				0==strncasecmp("OnTimer", ptr->name, 7) )
			{	// found it
				snprintf(event, sizeof(event), "%s::%s", this->exname, ptr->name);// name::event
				if( start )
					pc_addeventtimer(sd, atoi(ptr->name+7), event);	// with tick
				else
					pc_deleventtimer(sd, event);
			}
		}
	}
}
/// search and execute a script starting from a OnCommand-Label.
/// command parameter is only the string directly following the "::OnCommand", 
/// eg. for the label "OnCommandDie", the command parameter is "Die"
bool npcscript_data::command(const char *command, const map_session_data *sd)
{
	if( !this->invalid && this->ref )
	{	
		// look for the event inside the label list of the npc
		script_object::script_label *ptr = this->ref->label_list;
		const script_object::script_label *end = this->ref->label_list+this->ref->label_list_num;
		for(; ptr && ptr<end; ++ptr)
		{
			if( ptr->name && 
				0==strncasecmp("OnCommand", ptr->name, 9) &&
				0==strcasecmp(command, ptr->name+9) )
			{	// found it
				CScriptEngine::run(this->ref->script, ptr->pos, (sd)?sd->block_list::id:0, this->block_list::id);
				// only execute one single script, so quit here
				return true;
			}
		}
	}
	return false;
}



/// reverse eventname2npc lookup.
/// replace this by a map or multimap,
/// possibly by basics::smap< basics::string<>, basics::smap<uint32,size_t> >
struct event_data
{
	/// structure containing the npc/script position values associated to the event name
	struct event_entry
	{
		npcscript_data *nd;
		size_t pos;
		event_entry(npcscript_data *n=NULL, size_t p=0)
			: nd(n), pos(p)
		{}
		bool operator==(const event_entry& e) const { return this->nd==e.nd; }
		bool operator< (const event_entry& e) const { return this->nd< e.nd; }
	};

	basics::string<>			event_name;
	basics::slist<event_entry>	event_list;

	typedef basics::slist<event_entry>::iterator iterator;

	bool insert(npcscript_data &nd, size_t pos)
	{	// not likly but check if entry already exists
		// only need to test the npcscript_data
		// since there can be only one entry per event
		size_t p;
		event_entry tmp(&nd, pos);
		if( !this->event_list.find(tmp,0,p) )
		{
			this->event_list.append(tmp);
			return true;
		}
		return false;
	}
	bool erase(npcscript_data &nd)
	{
		size_t p;
		event_entry tmp(&nd, 0);
		if( this->event_list.find(tmp,0,p) )
		{
			this->event_list.removeindex(p);
			return true;
		}
		return false;
	}
	bool query(npcscript_data &nd, size_t &pos)
	{
		size_t p;
		event_entry tmp(&nd, 0);
		if( this->event_list.find(tmp,0,p) )
		{
			pos = this->event_list[pos].pos;
			return true;
		}
		return false;
	}

	size_t size() const
	{
		return this->event_list.size();
	}

	event_data(const char* name) : event_name(name)	{}
	~event_data()	{}
};


static struct dbt *ev_db=NULL;


/// search and execute a script starting from a On-Label.
/// eventname parameter is the whole "<npcname>::<eventname>" string
/// can be "mis"used for doing a allevent call by having the eventname with leading "::"
int npc_data::_event(const char *eventname, npcscript_data *nd, map_session_data *sd)
{
	int c=0;
	if(!eventname)
	{
		ShowError("npc_event: empty event\n");
		return 0;
	}
/////////////////////////////////////////////////
// temporary include until all event calls are rewritten

	// possible cases for eventname: 
	// <eventname>	 -> pure eventname, execute event label with all implementing npcs
	// ::<eventname> -> strip '::', then execute event label with all implementing npcs
	// <npcname>::<eventname> -> seperate, then execute single npc at event label
	// copy the npcname
	const char *evname=eventname;
	char namebuffer[64], *ip;
	for(ip=namebuffer; *evname && *evname!=':'; ++evname)
	{	// ignore name when longer than the buffer
		if( ip<namebuffer+sizeof(namebuffer)-1 )
			*ip++ = *evname;
	}
	*ip=0;

	// !*namebuffer && *evname==':'	-> ::<eventname>
	// *namebuffer && *evname=='\n' -> <eventname>
	// *namebuffer && *evname==':'  -> <npcname>::<eventname>

	if( *evname==':' )
	{
		while(*evname==':') ++evname;
		if( !*namebuffer )
		{	// ::<eventname>
			eventname = evname;
		}
		else
		{	// <npcname>::<eventname>
			nd = npcscript_data::from_name(namebuffer);
			if(!nd)
			{
				ShowError("npc_event: npc not found [%s]\n", namebuffer);
				return 0;
			}
			eventname = evname;
		}
	}
/////////////////////////////////////////////////

	if(nd)
	{	// execute eventname only inside the given npc

		// since an npc contains it's own label list, 
		// look up there if it has the queried label
		// (though there is some overhead due to the non-event labels there)
		// instead we also could look up the event label and then
		// check if the npc is in the list of the accociated entries

		// check for maps, 
		// execute the script only if the pc is on the same map than the nd		
		if( !nd->invalid && nd->ref &&
			(!sd || nd->block_list::m==0xFFFF || nd->block_list::m==sd->block_list::m) )
		{	
			// look for the event inside the label list of the npc
			script_object::script_label *ptr = nd->ref->label_list;
			const script_object::script_label *end = nd->ref->label_list+nd->ref->label_list_num;
			for(; ptr && ptr<end; ++ptr)
			{
				if( ptr->name && 0==strcasecmp(ptr->name, eventname) )
				{	// found it
					CScriptEngine::run(nd->ref->script, ptr->pos, (sd)?sd->block_list::id:0, nd->block_list::id);
					// only execute one single script, so quit here
					return 1;
				}
			}
			// display event not found but on non-existing touchup
			if( 0!=strcasecmp("OnTouch", eventname) && config.error_log)
				ShowError("npc_event: event not found [%s]\n", eventname);
		}
	}
	else
	{	// execute eventname on all npc's that implement it
		event_data *ev=(event_data *) strdb_search(ev_db,eventname);
		if(ev && ev->size()>0 )
		{
			event_data::iterator iter(ev->event_list);
			for(; iter; ++iter)
			{	// check for maps, 
				// execute the script only if the pc is on the same map than the nd
				if( iter->nd && !iter->nd->invalid &&
					(!sd || iter->nd->block_list::m==0xFFFF || iter->nd->block_list::m==sd->block_list::m) )
				{
					if(iter->nd->ref)
						CScriptEngine::run(iter->nd->ref->script, iter->pos, (sd)?sd->block_list::id:0, iter->nd->block_list::id);
					++c;
				}
			}
		}
	}
	return c;
}
/// executes On-events via npc and/or label
int npc_data::event(const char*onevent, const char*npcevent, map_session_data& sd)
{
	int evt = 0;
	//if( script_config.event_script_type == 0 )
	// condition removed, can do both

	// event via labels
	{
		evt = npc_data::event(onevent, sd);
		if(evt) ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n", evt, onevent);
		// ============================================ 
	}
	// event via npc name
	{
		npcscript_data *sc = npcscript_data::from_name(npcevent);
		if(sc && sc->ref && (sc->block_list::m==0xFFFF || sc->block_list::m==sd.block_list::m) )
		{
			CScriptEngine::run(sc->ref->script, 0, sd.block_list::id, sc->block_list::id);
			ShowStatus("Event '"CL_WHITE"%s"CL_RESET"' executed.\n", npcevent);
			++evt;
		}
	}
	return evt;
}


// ============================================ 
/*==========================================
 * NPCの無効化/有効化
 * npc_enable
 * npc_enable_sub 有効時にOnTouchイベントを実行
 *------------------------------------------
 */
class CNpcEnable : public CMapProcessor
{
	ICL_EMPTY_COPYCONSTRUCTOR(CNpcEnable)
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
			npc_data::event(name, *sd);
		}
		return 0;
	}
};

bool npc_data::enable(const char *name, int flag)
{
	npc_data *nd= npc_data::from_name(name);
	return (nd)?nd->enable(flag):false;
}

bool npc_data::enable(int flag)
{
	if (flag&1)
	{	// 有効化
		this->invalid=0;
		clif_spawnnpc(*this);
	}
	else if (flag&2)
	{
		this->invalid=0;
		this->option = 0x0000;
		clif_changeoption(*this);
	}
	else if (flag&4)
	{
		this->invalid=1;
		this->option = 0x0002;
		clif_changeoption(*this);
	}
	else
	{		// 無効化
		this->invalid=1;
		clif_clearchar(*this,0);
	}
	if( flag&3 && (this->xs > 0 || this->ys >0) )
	{
		maps[this->block_list::m].foreachinarea( CNpcEnable(*this),
			this->block_list::x-this->xs, this->block_list::y-this->ys, this->block_list::x+this->xs, this->block_list::y+this->ys, BL_PC);
	}
	return true;
}

bool npc_data::changename(const char *newname, unsigned short look)
{
	if(newname)
	{
		this->enable(0);
		strdb_erase(npc_data::npcname_db, this->exname);

		safestrcpy(this->name, sizeof(this->name), newname);
		if(look) this->class_ = look;
		
		strdb_insert(npc_data::npcname_db, this->exname, this);
		this->enable(1);
		return true;
	}
	return false;
}



void npc_data::remove_from_map()
{
    if( this->is_on_map() )
	{
		//Remove corresponding NPC CELLs
		if( this->xs || this->ys )
		{
			int i,j;
			const ushort m=this->block_list::m;
			const int x = this->block_list::x;
			const int y = this->block_list::y;
			const int xs = this->xs;
			const int ys = this->ys;

			for (i = 0; i < ys; ++i)
			for (j = 0; j < xs; ++j)
			{
				if( maps[m].is_npc(x-xs/2+j, y-ys/2+i) )
					maps[m].clr_npc(x-xs/2+j, y-ys/2+i);
			}
		}

		clif_clearchar_area(*this,2);
		this->delblock();
		this->unregister_id();
	}
}


/// destructor.
npc_data::~npc_data()
{
	this->remove_from_map();

	if(npc_data::npcname_db)
		strdb_erase(npc_data::npcname_db, this->exname);
	
	// unlink from map, if exist there
	map_intern &map = maps[this->block_list::m];
	if( this->block_list::m<maps.size() && this->n >=0 && this->n< MAX_NPC_PER_MAP &&
		map.npc[this->n]==this )
	{
		--map.npc_num;
		if(map.npc_num>0)
		{
			map.npc[this->n] = map.npc[map.npc_num];
			if( map.npc[this->n] )
				map.npc[this->n]->n = this->n;
		}
		else
			map.npc[this->n]=NULL;
	}
}

/// destructor.
npcscript_data::~npcscript_data()
{
	// delete chat objects
	npcchat_data::erase(*this);

	// delete pattern matching object
	if(this->listendb)
	{
		delete this->listendb;
		this->listendb = NULL;
	}
	// npc_timerobj destructor is cleaning itself from the root node
	// so delete until the last one is gone
	while( this->eventobj ) delete eventobj;

	// delete ontimers
	if (this->ontimer_list)
	{
		delete[] this->ontimer_list;
		this->ontimer_list=NULL;
		this->ontimer_cnt=0;
	}
	//Clean up all related events. (optimizable)
	db_iterator<const char*, event_data *> iter(ev_db);
	for(; iter; ++iter)
	{
		event_data *ev = iter.data();
		if( ev && ev->erase(*this) && ev->size()==0 )
		{
			strdb_erase(ev_db, iter.key());
			delete ev;
		}
	}
	// release the script
	if(this->ref)
	{
		this->ref->release();
		this->ref=NULL;
	}
}


/// display shop buy window
void npcshop_data::buywindow(map_session_data &sd)
{
	if( !this->invalid && this->is_near(sd) )
	{
		sd.npc_shopid=id;
		clif_buylist(sd,*this);
	}
}
/// display shop sell window
void npcshop_data::sellwindow(map_session_data &sd)
{
	if( !this->invalid && this->is_near(sd) )
	{
		sd.npc_shopid=id;
		clif_selllist(sd);
	}
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
		map_session_data *sd = map_session_data::from_blid(id);
		if(sd)
		{	
			size_t i;
			for(i=0; i<MAX_EVENTTIMER; ++i)
			{
				if( sd->eventtimer[i]==tid )
				{
					sd->eventtimer[i]=-1;
					npc_data::event(eventname, *sd);
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
		c+=npc_data::event(buf);
		snprintf(buf,sizeof(buf),"OnClock%02d%02d",t->tm_hour,t->tm_min);
		c+=npc_data::event(buf);
		snprintf(buf,sizeof(buf),"On%s%02d%02d",day,t->tm_hour,t->tm_min);
		c+=npc_data::event(buf);
	}
	if (t->tm_hour!= ev_tm.tm_hour)
	{
		snprintf(buf,sizeof(buf),"OnHour%02d",t->tm_hour);
		c+=npc_data::event(buf);
	}
	if (t->tm_mday!= ev_tm.tm_mday)
	{
		snprintf(buf,sizeof(buf),"OnDay%02d%02d",t->tm_mon+1,t->tm_mday);
		c+=npc_data::event(buf);
	}
	ev_tm = *t;
	return c;
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
		z = z * sd.skill_check(MC_DISCOUNT) / ((1 + 300 / itemamount) * 4000) * config.shop_exp;
		pc_gainexp(sd,0,z);
	}*/
	if (config.shop_exp > 0 && z > 0 && (skill = sd.skill_check(MC_DISCOUNT)) > 0) {
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
	if (config.shop_exp > 0 && z > 0 && (skill = sd.skill_check(MC_OVERCHARGE)) > 0) {
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

	m = maps.index_of(mapname);

	npcwarp_data *nd = new npcwarp_data(); 
	nd->register_id( npc_get_new_npc_id() );
	nd->n = maps[m].addnpc(nd);
	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;
	nd->invalid = 0;
	safestrcpy(nd->name,  sizeof(nd->name),  w3);
	safestrcpy(nd->exname,sizeof(nd->exname),w3);

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

	for (i = 0; i < ys; ++i)
	for (j = 0; j < xs; ++j)
	{
		if( maps[m].is_passable(x-xs/2+j, y-ys/2+i) )
			maps[m].set_npc(x-xs/2+j, y-ys/2+i);
	}


//	ShowMessage("warp npc %s %d read done\n",mapname,nd->block_list::id);
	++npc_warp;
	nd->addblock();
	clif_spawnnpc(*nd);

	strdb_insert(npc_data::npcname_db, nd->exname, nd);

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
		m = maps.index_of(mapname);
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
	nd->register_id( npc_get_new_npc_id() );
	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;

	nd->init_dir(dir_t(dir&0x07),dir_t(dir&0x07));
	nd->invalid = 0;
	safestrcpy(nd->name,  sizeof(nd->name),  w3);
	safestrcpy(nd->exname,sizeof(nd->exname),w3);
	nd->class_ = (m==-1)?0:atoi(w4);
	nd->speed = 200;
	nd->option = 0;
	nd->opt1 = 0;
	nd->opt2 = 0;
	nd->opt3 = 0;

	//ShowMessage("shop npc %s %d read done\n",mapname,nd->block_list::id);
	++npc_shop;

	nd->n = maps[m].addnpc(nd);
	if (m >= 0)
	{
		nd->addblock();
		clif_spawnnpc(*nd);
	}
	strdb_insert(npc_data::npcname_db, nd->exname, nd);

	return 0;
}


/*==========================================
 * script行解析
 *------------------------------------------
 */
int npc_parse_script(const char *w1,const char *w2,const char *w3,const char *w4,const char *first_line,FILE *fp,int *lines, npc_data **dummy_npc)
{
	int x, y, dir = 0, m, xs = 0, ys = 0, class_ = 0;	//  thanks to fov
	char mapname[32], *ip;
	unsigned char *srcbuf=NULL;
	size_t srcsize=65536;
	int startline = 0;
	unsigned char line[1024];

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
		m = maps.index_of(mapname);
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
			ssize_t i;
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
				if( maps[m].is_passable(x - xs/2 + j, y - ys/2 + i) )
					maps[m].set_npc(x - xs/2 + j, y - ys/2 + i);
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

	// split <name>::<exname>
	const char *rp;
	char *wp;
	for(rp=w3, wp=nd->name; *rp && (rp[0]!=':' || rp[1]!=':'); ++rp)
	{
		if( wp<nd->name+sizeof(nd->name)-1)
		{
			*wp = *rp;
			++wp;
		}
	}
	*wp=0;
	safestrcpy(nd->exname, sizeof(nd->exname), (*rp)?rp+2:nd->name);


	nd->block_list::m = m;
	nd->block_list::x = x;
	nd->block_list::y = y;
	nd->init_dir(dir_t(dir&0x07),dir_t(dir&0x07));
	nd->class_ = class_;
	nd->speed = 200;

	nd->ref=ref;

	//ShowMessage("script npc %s %d %d read done\n",mapname,nd->block_list::id,nd->class_);
	if(!dummy_npc)
	{
		++npc_script;
		nd->register_id( npc_get_new_npc_id() );
	}
	if(m>=0 && !dummy_npc)
	{
		nd->n = maps[m].addnpc(nd);
		nd->addblock();
		if( class_<0 )
		{	// イベント型NPC
			event_data *ev = (event_data *)strdb_search(ev_db,nd->exname);
			if( NULL==ev )
			{	// create new if not exists
				ev = new struct event_data(nd->exname);
				strdb_insert(ev_db,(const char*)ev->event_name, ev);
			}
			ev->insert(*nd, 0);
			
		}
		else
			clif_spawnnpc(*nd);
	}
	
	strdb_insert(npc_data::npcname_db, nd->exname, nd);

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
	{
		size_t i;
		for(i=0;i<nd->ref->label_count();++i)
		{
			const script_object::script_label& label = nd->ref->get_label(i);

			// export event labels (all labels starting with "On")
			if( basics::locase(label.name[0])=='o' && basics::locase(label.name[1])=='n' ) 
			{
				struct event_data *ev = (struct event_data *)strdb_search(ev_db, label.name);
				if( ev )
				{
					if( !ev->insert(*nd, label.pos) )
						ShowError("npc_parse_script : duplicate event %s::%s\n", nd->exname, label.name);
				}
				else
				{
					ev = new struct event_data(label.name);
					ev->insert(*nd,label.pos);
					strdb_insert(ev_db, (const char*)ev->event_name, ev);
				}
			}

			// ラベルデータからタイマーイベント取り込み
			int n = 0;
			ulong t = 0;
			if(sscanf(label.name,"OnTimer%lu%n",&t,&n)==1 && label.name[n]=='\0')
			{
				// タイマーイベント
				struct npc_timerevent *te = nd->ontimer_list;
				int j, k = nd->ontimer_cnt;

				new_realloc(te,k,1);
				for(j=0;j<k;++j)
				{
					if(te[j].tick>t)
					{
						memmove(te+j+1, te+j, sizeof(struct npc_timerevent)*(k-j));
						break;
					}
				}
				te[j].tick = t;
				te[j].pos = label.pos;
				nd->ontimer_list = te;
				nd->ontimer_cnt = k+1;
			}
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

	m = maps.index_of(mapname);
	if(m >= MAX_MAP_PER_SERVER)
		return 1;
	if(v5 <= 1000 || v5 > MAX_MOB_DB) // class check
		return 1;
		
	struct mob_list *dynmob = maps[m].moblist_create();
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
	m = maps.index_of(mapname);
	if (m < 0)
		return 1;

	//マップフラグ
	if ( strcasecmp(w3,"nosave")==0)
	{
		char savemap[32];
		int savex, savey;
		if (strcmp(w4, "SavePoint") == 0)
		{
			safestrcpy(maps[m].nosave.mapname, sizeof(maps[m].nosave.mapname), "SavePoint");
			maps[m].nosave.x = -1;
			maps[m].nosave.y = -1;
		}
		else if (sscanf(w4, "%32[^,],%d,%d", savemap, &savex, &savey) == 3)
		{
			ip = strchr(savemap, '.');
			if(ip) *ip=0;
			safestrcpy(maps[m].nosave.mapname, sizeof(maps[m].nosave.mapname), savemap);			
			maps[m].nosave.x = savex;
			maps[m].nosave.y = savey;
		}
		maps[m].flag.nosave = 1;
	}
	else if (strcasecmp(w3,"nomemo")==0)
	{
		maps[m].flag.nomemo=1;
	}
	else if (strcasecmp(w3,"noteleport")==0)
	{
		maps[m].flag.noteleport=1;
	}
	else if (strcasecmp(w3,"nowarp")==0)
	{
		maps[m].flag.nowarp=1;
	}
	else if (strcasecmp(w3,"nowarpto")==0)
	{
		maps[m].flag.nowarpto=1;
	}
	else if (strcasecmp(w3,"noreturn")==0)
	{
		maps[m].flag.noreturn=1;
	}
	else if (strcasecmp(w3,"monster_noteleport")==0)
	{
		maps[m].flag.monster_noteleport=1;
	}
	else if (strcasecmp(w3,"nobranch")==0)
	{
		maps[m].flag.nobranch=1;
	}
	else if (strcasecmp(w3,"nopenalty")==0)
	{
		maps[m].flag.nopenalty=1;
	}
	else if (strcasecmp(w3,"pvp")==0)
	{
		maps[m].flag.pvp=1;
	}
	else if (strcasecmp(w3,"pvp_noparty")==0)
	{
		maps[m].flag.pvp_noparty=1;
	}
	else if (strcasecmp(w3,"pvp_noguild")==0)
	{
		maps[m].flag.pvp_noguild=1;
	}
	else if (strcasecmp(w3,"pvp_nightmaredrop")==0)
	{
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
	else if (strcasecmp(w3,"pvp_nocalcrank")==0)
	{
		maps[m].flag.pvp_nocalcrank=1;
	}
	else if (strcasecmp(w3,"gvg")==0)
	{
		maps[m].flag.gvg=1;
	}
	else if (strcasecmp(w3,"gvg_noparty")==0)
	{
		maps[m].flag.gvg_noparty=1;
	}
	else if (strcasecmp(w3,"gvg_dungeon")==0)
	{
		maps[m].flag.gvg_dungeon=1;
	}
	else if (strcasecmp(w3,"nozenypenalty")==0)
	{
		maps[m].flag.nozenypenalty=1;
	}
	else if (strcasecmp(w3,"notrade")==0)
	{
		maps[m].flag.notrade=1;
	}
	else if (strcasecmp(w3,"noskill")==0)
	{
		maps[m].flag.noskill=1;
	}
	else if (config.pk_mode && strcasecmp(w3,"nopvp")==0)
	{
		maps[m].flag.nopvp=1;
		maps[m].flag.pvp=0;
	}
	else if (strcasecmp(w3,"noicewall")==0)
	{
		maps[m].flag.noicewall=1;
	}
	else if (strcasecmp(w3,"snow")==0)
	{
		maps[m].flag.snow=1;
	}
	else if (strcasecmp(w3,"clouds")==0)
	{
		maps[m].flag.clouds=1;
	}
	else if (strcasecmp(w3,"clouds2")==0)
	{
		maps[m].flag.clouds2=1;
	}
	else if (strcasecmp(w3,"fog")==0)
	{
		maps[m].flag.fog=1;
	}
	else if (strcasecmp(w3,"fireworks")==0)
	{
		maps[m].flag.fireworks=1;
	}
	else if (strcasecmp(w3,"sakura")==0)
	{
		maps[m].flag.sakura=1;
	}
	else if (strcasecmp(w3,"leaves")==0)
	{
		maps[m].flag.leaves=1;
	}
	else if (strcasecmp(w3,"rain")==0)
	{
		maps[m].flag.rain=1;
	}
	else if (strcasecmp(w3,"indoors")==0)
	{
		maps[m].flag.indoors=1;
	}
	else if (strcasecmp(w3,"nogo")==0)
	{
		maps[m].flag.nogo=1;
	}
	else if (strcasecmp(w3,"noexp")==0)
	{
		maps[m].flag.nobaseexp=1;
		maps[m].flag.nojobexp=1;
	}
	else if (strcasecmp(w3,"nobaseexp")==0)
	{
		maps[m].flag.nobaseexp=1;
	}
	else if (strcasecmp(w3,"nojobexp")==0)
	{
		maps[m].flag.nojobexp=1;
	}
	else if (strcasecmp(w3,"noloot")==0)
	{
		maps[m].flag.nomobloot=1;
		maps[m].flag.nomvploot=1;
	}
	else if (strcasecmp(w3,"nomobloot")==0)
	{
		maps[m].flag.nomobloot=1;
	}
	else if (strcasecmp(w3,"nomvploot")==0)
	{
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
	m = maps.index_of(mapname);
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
		maps[m].set_type(x, y, cell);
		//ShowMessage("setcell 0x%x %d %d %d\n", cell, m, x, y);
	}
	return 0;
}

void npc_parsesinglefile(const char *filename, struct npc_mark*& npcmarkerbase)
{
	int m, lines = 0;
	char line[1024];
	char w1[1024], w2[1024], w3[1024], w4[1024], mapname[1024], *ip;
	int i, j;
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
				m = maps.index_of(mapname);
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
			strdb_erase(npc_data::npcname_db,marker->nd->exname);
			ShowMessage("\rDeleting unused NPC: %s", marker->nd->exname);
			marker->nd->freeblock();
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
			strdb_erase(npc_data::npcname_db,marker->nd->exname);
			ShowMessage("\rDeleting unused NPC: %s", marker->nd->exname);
			marker->nd->freeblock();
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
				if ((m = maps.index_of(mapname)) >= 0)
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
}

void npcname_db_final (void *key,void *data)
{
	npc_data *nd = (npc_data *) data;
	nd->freeblock();
}

/*==========================================
 * 
 *------------------------------------------
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
			nd->freeblock();
		else if( (md=bl.get_md()) )
			md->freeblock();
		return 0;
	}
};
int npc_reload (void)
{
	size_t m;

	for (m = 0; m < maps.size(); ++m)
	{
		maps[m].foreach( CNpcCleanup(), BL_ALL);
		maps[m].moblist_clear();
		maps[m].npc_num = 0;
	}
	if(ev_db)
	{
		strdb_final(ev_db,ev_db_final);
		ev_db = NULL;
	}
	block_list::freeblock_lock();
	if(npc_data::npcname_db)
	{
		strdb_final(npc_data::npcname_db,npcname_db_final);
		npc_data::npcname_db=NULL;
	}
	block_list::freeblock_unlock();

	// anything else we should cleanup?
	// Reloading npc's now
	ev_db = strdb_init(51);
	npc_data::npcname_db = strdb_init(24);
	ev_db->release = ev_release;
	npc_warp = npc_shop = npc_script = 0;
	npc_mob = npc_cache_mob = npc_delay_mob = 0;
	
	npc_parsesrcfiles( );

	//Execute the OnInit event for freshly loaded npcs. [Skotlex]
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
		CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_data::event("OnInit"));

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
	mob_data *md;
	pet_data *pd;

	// clear event data first, there are pointers to the npc names stored inside
	// so we have to call this before deleting the npcs
	if(ev_db)
	{
		strdb_final(ev_db, ev_db_final);
		ev_db = NULL;
	}

	const uint32 last = get_npc_id();
	for (i = START_NPC_NUM; i < last; ++i)
	{
		bl = block_list::from_blid(i);
		if( bl )
		{
			if( (nd = bl->get_nd()) )
			{
				nd->freeblock();
				nd = NULL;
				n++;
			}
			else if( (md = bl->get_md()) )
			{
				md->freeblock();
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
	if(npc_data::npcname_db)
	{
		strdb_final(npc_data::npcname_db, npcname_db_final);
		npc_data::npcname_db = NULL;
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
	npc_data::npcname_db = strdb_init(24);
	ev_db->release = ev_release;

	npc_parsesrcfiles( );

	add_timer_func_list(npcscript_data::eventtimer_entry,"npcscript_data::eventtimer_entry");
	add_timer_func_list(npc_event_timer,"npc_event_timer");
	add_timer_func_list(npc_event_do_clock,"npc_event_do_clock");

	add_timer_interval(gettick()+1000, 1000, npc_event_do_clock, 0, 0);

	return 0;
}

