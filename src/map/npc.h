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
// part of a union and cannot be default constructed
// need to wait until new hierarchy is fully implemented
//	npc_item_list() : 
//		nameid(0),
//		value(0)
//	{}
};





struct npc_data : public movable
{
	/////////////////////////////////////////////////////////////////
	static npc_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_nd():NULL;
	}

	/////////////////////////////////////////////////////////////////

	short n;
	short class_;
	short dir;
	short speed;
	char name[24];
	char exname[24];
	npcchat_data* chat;
	short opt1;
	short opt2;
	short opt3;
	short option;
	short flag;

	short arenaflag;
	npc_parse *listendb;

	union {
		struct {
			struct script_object *ref; // pointer with reference counter
			short xs;
			short ys;
			int guild_id;
			int timer;
			int timerid;
			int timeramount;
			int nexttimer;
			uint32 rid;
			unsigned long timertick;
			struct npc_timerevent_list *timer_event;
		} scr;
		struct npc_item shop_item[1];
		struct {
			short xs;
			short ys;
			short x;
			short y;
			char name[16];
		} warp;
	} u;
	// Ç±Ç±Ç…ÉÅÉìÉoÇí«â¡ÇµÇƒÇÕÇ»ÇÁÇ»Ç¢(shop_itemÇ™â¬ïœí∑ÇÃà◊)



	// can have an empty constructor here since it is cleared at allocation
	npc_data()
	{}
	virtual ~npc_data()
	{}


	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t)
	{
		return t==BL_NPC;
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
	// provide special allocation to enable variable space for shop items
	//
	// change to an own shop class later that can be called from an npc
	// to enable the old shop npc and also callable script controlled (dynamic) shops

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
	void* operator new(size_t sz, size_t shopitems)
	{
		void* ret = malloc(sz + shopitems*sizeof(npc_item));
		memset(ret,0,sz + shopitems*sizeof(npc_item));
		return ret;
	}
	void operator delete(void *p, size_t shopitems)
	{
		if(p) free(p);
	}

private:
	void* operator new[](size_t sz);			// forbidden
	void operator delete[](void *p, size_t sz);	// forbidden
};










//int npc_event_dequeue(struct map_session_data &sd);
//int npc_event_enqueue(struct map_session_data &sd, const char *eventname);
int npc_event_timer(int tid, unsigned long tick, int id, basics::numptr data);
int npc_event(struct map_session_data &sd,const char *npcname,int);
int npc_timer_event(const char *eventname);				// Added by RoVeRT
int npc_command(struct map_session_data &sd,const char *npcname, const char *command);
int npc_touch_areanpc(struct map_session_data &sd,unsigned short m,int x,int y);
int npc_click(struct map_session_data &sd,uint32 npcid);
int npc_scriptcont(struct map_session_data &sd,uint32 id);
bool npc_isNear(struct map_session_data &sd, struct npc_data &nd);
int npc_buysellsel(struct map_session_data &sd,uint32 id,int type);
int npc_buylist(struct map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_selllist(struct map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_parse_mob(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_parse_mob2(struct mob_list &mob);
bool npc_parse_warp(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_globalmessage(const char *name,const char *mes);

int npc_enable(const char *name,int flag);
int npc_changename(const char *name, const char *newname, unsigned short look);
struct npc_data* npc_name2id(const char *name);


uint32 npc_get_new_npc_id(void);

void npc_addsrcfile(const char *);
void npc_delsrcfile(const char *);
void npc_printsrcfile();
void npc_parsesrcfile(const char *);
int do_final_npc(void);
int do_init_npc(void);
int npc_event_do_oninit(void);
int npc_do_ontimer(uint32 npc_id, struct map_session_data &sd, int option);


int npc_event_do(const char *name);
int npc_event_doall(const char *name, int rid=0, int map=-1);

int npc_timerevent_start(struct npc_data &nd, uint32 rid);
int npc_timerevent_stop(struct npc_data &nd);
int npc_gettimerevent_tick(struct npc_data &nd);
int npc_settimerevent_tick(struct npc_data &nd,int newtimer);
int npc_remove_map(struct npc_data *nd);
int npc_unload(struct npc_data *nd, bool erase_strdb=true);
int npc_reload(void);

// ============================================
// ADDITION Qamera death/disconnect/connect event mod
int npc_event_doall_attached(const char *name, struct map_session_data &sd);
struct npc_att_data {
	struct map_session_data * sd;
	char buf[64];
} ;
// END ADDITION
// ============================================ 

#endif

