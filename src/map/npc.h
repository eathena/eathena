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
/// virtual npc structure.
/// contains what all different npc objects commonly use
struct npc_data : public movable
{
	static struct dbt *npcname_db;
	/////////////////////////////////////////////////////////////////
	static npc_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_nd():NULL;
	}
	/// returns npcdata from name.
	static npc_data* from_name(const char *name);

	/////////////////////////////////////////////////////////////////

	char name[24];
	char exname[24];
	short n;
	short class_;
	short speed;
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
	virtual ~npc_data();
public:

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_NPC);
	}
	virtual npc_data*				get_nd()				{ return this; }
	virtual const npc_data*			get_nd() const			{ return this; }



	// removes object from map, but leave it in memory
	void npc_data::remove_from_map();

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
	/// called from map_session_data::do_walkend.
	static void touch(map_session_data &sd,unsigned short m,int x,int y);

private:
	/// internal function for executeing events.
	static int _event(const char *eventname, npcscript_data *nd, map_session_data *sd);
public:
	/// search and execute all scripts implementing the given On-Label.
	/// execute without an attached pc.
	static int event(const char *eventname)
	{
		return npc_data::_event(eventname,NULL,NULL);
	}
	/// search and execute all scripts implementing the given On-Label.
	static int event(const char *eventname, map_session_data &sd)
	{
		return npc_data::_event(eventname,NULL,&sd);
	}
	/// search and execute a single script at given On-Label.
	/// execute without an attached pc.
	static int event(const char *eventname, npcscript_data &nd)
	{
		return npc_data::_event(eventname,&nd,NULL);
	}
	/// search and execute a single script at given On-Label.
	static int event(const char *eventname, npcscript_data &nd, map_session_data &sd)
	{
		return npc_data::_event(eventname,&nd,&sd);
	}
	/// executes On-events via npc and/or label
	static int event(const char*onevent, const char*npcevent, map_session_data& sd);

	/// enable/disable a npc via name.
	static bool enable(const char *name, int flag);
	/// enable/disable a npc
	bool enable(int flag);
	/// change name of a npc
	bool changename(const char *newname, unsigned short look=0);


	
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



///////////////////////////////////////////////////////////////////////////////
// timer event structure.
// contains the tick (counted from 0) and the script position which
// is called at the given time, when the script is started
struct npc_timerevent
{
	ulong	tick;	// executing tick 
	size_t	pos;	// position inside the script to be called

	npc_timerevent() : 
		tick(0),
		pos(0)
	{}
};








///////////////////////////////////////////////////////////////////////////////
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


private:
	///////////////////////////////////////////////////////////////////////////
	/// internal npc timer object.
	/// wrapper for a calling structure that is doing npc_timerevent.
	/// contains the necessary data for recalling the script.
	/// check for merging this whole thing with the new script engine pool,
	/// since only this way also variables could be reused savely with threads
	struct npc_timerobj : public basics::global
	{
		npc_timerobj*	next;	// pointer for a single linked list
		npcscript_data	&nd;	// root to the executed npc
		uint32			rid;	// id of the session that has called this
		ushort			pos;	// position inside of npc_timerevent to be executed next
		int				tid;	// the timer id, this object is running from

		/// constructor.
		/// queues this in.
		npc_timerobj(npcscript_data &n, uint32 r);
		/// destructor.
		/// dequeues and removes timers.
		~npc_timerobj();
	};
	friend struct npc_timerobj;

	///////////////////////////////////////////////////////////////////////////

public:
	script_object *ref;				///< pointer with reference counter to script binary
	npc_parse *listendb;			///< pointer to npclisten object
	npcchat_data* chat;				///< pointer to a npcchat object

	uint32 guild_id;				///< set when this npc belongs to a guild (guild kafra)

	npc_timerevent *ontimer_list;	///< list of OnTimer entries
	ushort ontimer_cnt;				///< number of OnTimer entries
	npc_timerobj	*eventobj;		///< eventtimer root


	///////////////////////////////////////////////////////////////////////////
	// eventtimer functions
public: //## current add_timer_func_list needs public access
	/// timer entry funtion.
	static int eventtimer_entry(int tid, unsigned long tick, int id, basics::numptr data);
private:
	/// aquire a timer object. 
	/// create a new object if not exists
	npc_timerobj* eventtimer_aquire(uint32 rid);
public:
	/// get the position inside the eventtimer list.
	/// return 0 when no timer has been started
	ushort npcscript_data::eventtimer_getpos(uint32 rid);
	/// set the position inside the eventtimer list.
	/// return the previous count. does nothing when no timer has been started
	ushort npcscript_data::eventtimer_setpos(uint32 rid, ushort pos);
	/// set the rid for the next call.
	/// return the previous rid. does nothing when no timer has been started
	uint32 npcscript_data::eventtimer_attach(uint32 oldrid, uint32 newrid);
	/// initialize a timer object and start the timer.
	void eventtimer_init(uint32 rid, ushort pos=0);
	/// start the timer.
	void eventtimer_start(uint32 rid);
	/// stop the timer.
	void eventtimer_stop(uint32 rid);
	/// clear the timers if not in use.
	void eventtimer_clear(uint32 rid);
	
	/// start/stop ontimers
	void do_ontimer(map_session_data &sd, bool start);
	/// search and execute a script starting from a OnCommand-Label.
	/// command parameter is only the string directly following the "::OnCommand", 
	/// eg. for the label "OnCommandDie", the command parameter is "Die"
	bool command(const char *command, const map_session_data *sd);

	///////////////////////////////////////////////////////////////////////////
	/// constructor
	npcscript_data() :
		eventobj(NULL)
	{}
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	~npcscript_data();


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



///////////////////////////////////////////////////////////////////////////////
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




///////////////////////////////////////////////////////////////////////////////
/// npc shop item.
struct npc_item
{
	ushort	nameid;
	uint32	price;
	npc_item() : 
		nameid(0),
		price(0)
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// shop npc.
/// only contains a list of shop items
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
	/// ignored, shops do not implement touchup
	virtual void OnTouch(block_list& bl, ushort x, ushort y)	{}

	/// display shop buy window
	void npcshop_data::buywindow(map_session_data &sd);
	/// display shop sell window
	void npcshop_data::sellwindow(map_session_data &sd);

};









int npc_buylist(map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_selllist(map_session_data &sd,unsigned short n,unsigned char *buffer);


int npc_parse_mob(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_parse_mob2(mob_list &mob);
bool npc_parse_warp(const char *w1,const char *w2,const char *w3,const char *w4);


uint32 npc_get_new_npc_id(void);

void npc_addsrcfile(const char *);
void npc_delsrcfile(const char *);
void npc_printsrcfile();
void npc_parsesrcfile(const char *);

int npc_reload(void);


int do_final_npc(void);
int do_init_npc(void);





#endif

