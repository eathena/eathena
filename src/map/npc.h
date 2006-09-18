// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NPC_H_
#define _NPC_H_

#include "map.h"
#include "movable.h"
#include "npclisten.h"


#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define INVISIBLE_CLASS 32767


///////////////////////////////////////////////////////////////////////////////
struct npc_timerevent_list
{
	int timer;
	size_t pos;

	npc_timerevent_list() : 
		timer(-1),
		pos(0)
	{}
};
struct npc_item
{
	ushort	nameid;
	uint32	price;
	npc_item() : 
		nameid(0),
		price(0)
	{}
};


/// virtual npc structure.
/// contains what all different npc objects commonly use
struct npc_data : public movable
{
	/////////////////////////////////////////////////////////////////
	static npc_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_nd():NULL;
	}
	static npc_data* from_name(const char *name);

	/////////////////////////////////////////////////////////////////

	short n;
	short class_;
//	short dir;
	short speed;
	char name[24];
	char exname[24];
	short opt1;
	short opt2;
	short opt3;
	short option;
	short xs;	// touchup area
	short ys;
	unsigned char	invalid	  : 1;
	unsigned char	arenaflag : 1;

protected:
	// can have an empty constructor here since it is cleared at allocation
	npc_data()
	{}
	virtual ~npc_data()
	{}
public:

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_NPC);
	}
	virtual npc_data*				get_nd()				{ return this; }
	virtual const npc_data*			get_nd() const			{ return this; }


	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking();
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy);
	/// do object depending stuff for the walkto
	virtual void do_walkto() {}


private:
	npc_data(const npc_data&);					// forbidden
	const npc_data& operator=(const npc_data&);	// forbidden

public:

	/// check if withing AREA reange.
	/// also true when this is a map-less npc
	virtual bool is_near(const block_list& bl) const
	{
		return  ( this->class_<0 ) || this->block_list::is_near(bl);
	}
	
	/// what to do when clicked.
	/// overloadable
	virtual void OnClick(map_session_data &sd)	{}
	/// try and execute touchup 
	/// overloadable
	virtual void OnTouch(block_list& bl)	{}

	/// called from clif_parse_NpcClicked.
	static void click(map_session_data &sd, uint32 npcid);
	
	
	// operator new overload for cleaning the memory
	void* operator new(size_t sz)
	{
		void *ret = malloc(sz);
		memset(ret,0,sz);
		return ret;
	}
	void operator delete(void *p)
	{
		if(p) free(p);
	}
private:
	void* operator new[](size_t sz);			// forbidden
	void operator delete[](void *p, size_t sz);	// forbidden
};


/// script npc.
/// contains script object, timer stuff should be revised
struct npcscript_data : public npc_data
{
	/////////////////////////////////////////////////////////////////
	static npcscript_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_script():NULL;
	}
	static npcscript_data* from_name(const char *name)
	{
		npc_data *nd = npc_data::from_name(name);
		return nd?nd->get_script():NULL;
	}

	/////////////////////////////////////////////////////////////////

	script_object *ref;		// pointer with reference counter
	npc_parse *listendb;
	npcchat_data* chat;
	int guild_id;
	int timer;
	int timerid;
	int timeramount;
	int nexttimer;
	uint32 rid;
	unsigned long timertick;
	struct npc_timerevent_list *timer_event;

	npcscript_data()
	{}
	~npcscript_data()
	{}


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual npcscript_data*			get_script()		{ return this; }
	virtual const npcscript_data*	get_script() const	{ return this; }


	/// what to do when clicked.
	/// run a script
	virtual void OnClick(map_session_data &sd);
	/// try and execute touchup 
	/// test for ontouch lable or start script from beginning if none
	virtual void OnTouch(block_list& bl);
};

/// warp npc.
/// only contains target coordinates
struct npcwarp_data : public npc_data
{
public:
	/////////////////////////////////////////////////////////////////
	static npcwarp_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_warp():NULL;
	}
	static npcwarp_data* from_name(const char *name)
	{
		npc_data *nd = npc_data::from_name(name);
		return nd?nd->get_warp():NULL;
	}
	/////////////////////////////////////////////////////////////////
	char mapname[16];
	ushort xt;			// target coordinates
	ushort yt;
public:
	npcwarp_data()
	{}
	~npcwarp_data()
	{}


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual npcwarp_data*			get_warp()			{ return this; }
	virtual const npcwarp_data*		get_warp() const	{ return this; }


	/// what to do when clicked.
	/// ignore it
	virtual void OnClick(map_session_data &sd)	{}
	/// try and execute touchup 
	/// execute warp
	virtual void OnTouch(block_list& bl);
};

/// shop npc.
/// only contains shop items
struct npcshop_data : public npc_data
{
	/////////////////////////////////////////////////////////////////
	static npcshop_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_shop():NULL;
	}
	static npcshop_data* from_name(const char *name)
	{
		npc_data *nd = npc_data::from_name(name);
		return nd?nd->get_shop():NULL;
	}
	/////////////////////////////////////////////////////////////////

	npc_item*	shop_item_lst;
	ushort		shop_item_cnt;


	npcshop_data(const npc_item* lst, size_t cnt) : shop_item_lst(NULL), shop_item_cnt(cnt)
	{
		if(lst && cnt)
		{
			this->shop_item_lst = new npc_item[cnt];
			size_t i;
			for(i=0; i<cnt; ++i)
				this->shop_item_lst[i] = lst[i];
		}
	}
	~npcshop_data()
	{
		if(shop_item_lst) delete[] shop_item_lst;
	}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual npcshop_data*			get_shop()			{ return this; }
	virtual const npcshop_data*		get_shop() const	{ return this; }


	/// what to do when clicked.
	/// bring up associated shop
	virtual void OnClick(map_session_data &sd);
	/// try and execute touchup 
	/// ignored, shops do not have  touchup
	virtual void OnTouch(block_list& bl, ushort x, ushort y)	{}
};





//int npc_event_dequeue(map_session_data &sd);
//int npc_event_enqueue(map_session_data &sd, const char *eventname);
int npc_event_timer(int tid, unsigned long tick, int id, basics::numptr data);
int npc_event(map_session_data &sd,const char *npcname,int);
int npc_timer_event(const char *eventname);				// Added by RoVeRT
int npc_command(map_session_data &sd,const char *npcname, const char *command);
int npc_touch_areanpc(map_session_data &sd,unsigned short m,int x,int y);
int npc_scriptcont(map_session_data &sd,uint32 id);
int npc_buysellsel(map_session_data &sd,uint32 id,int type);
int npc_buylist(map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_selllist(map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_parse_mob(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_parse_mob2(mob_list &mob);
bool npc_parse_warp(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_globalmessage(const char *name,const char *mes);

bool npc_enable(const char *name,int flag);
int npc_changename(const char *name, const char *newname, unsigned short look);


uint32 npc_get_new_npc_id(void);

void npc_addsrcfile(const char *);
void npc_delsrcfile(const char *);
void npc_printsrcfile();
void npc_parsesrcfile(const char *);
int do_final_npc(void);
int do_init_npc(void);
int npc_event_do_oninit(void);
int npc_do_ontimer(uint32 npc_id, map_session_data &sd, int option);


int npc_event_do(const char *name);
int npc_event_doall(const char *name, int rid=0, int map=-1);

int npc_timerevent_start(npcscript_data &sc, uint32 rid);
int npc_timerevent_stop(npcscript_data &nd);
int npc_gettimerevent_tick(npcscript_data &sc);
int npc_settimerevent_tick(npcscript_data &sc,int newtimer);
int npc_remove_map(npc_data *nd);
int npc_unload(npc_data *nd, bool erase_strdb=true);
int npc_reload(void);

// ============================================
// ADDITION Qamera death/disconnect/connect event mod
int npc_event_doall_attached(const char *name, map_session_data &sd);
struct npc_att_data {
	map_session_data * sd;
	char buf[64];
} ;
// END ADDITION
// ============================================ 

#endif

