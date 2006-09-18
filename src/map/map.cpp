// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "core.h"
#include "timer.h"
#include "socket.h"
#include "showmsg.h"
#include "utils.h"
#include "nullpo.h"
#include "db.h"
#include "grfio.h"
#include "malloc.h"
#include "version.h"

#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "chat.h"
#include "itemdb.h"
#include "storage.h"
#include "skill.h"
#include "trade.h"
#include "party.h"
#include "battle.h"
#include "script.h"
#include "guild.h"
#include "pet.h"
#include "homun.h"
#include "atcommand.h"
#include "log.h"


#define USE_AFM
#define USE_AF2



const char *LOG_CONF_NAME="conf/log_athena.conf";
const char *MAP_CONF_NAME = "conf/map_athena.conf";
const char *BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
const char *ATCOMMAND_CONF_FILENAME = "conf/atcommand_athena.conf";
const char *CHARCOMMAND_CONF_FILENAME = "conf/charcommand_athena.conf";
const char *SCRIPT_CONF_NAME = "conf/script_athena.conf";
const char *MSG_CONF_NAME = "conf/msg_athena.conf";
const char *GRF_PATH_FILENAME = "conf/grf-files.txt";

char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";
char afm_dir[1024] = "";
char wisp_server_name[24] = "Server"; // can be modified in char-server configuration file


int CHECK_INTERVAL = 3600000;

int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int agit_flag = 0;
bool console = false;



// 極力 staticでロ?カルに?める
static dbt* map_db=NULL;
struct map_data maps[MAX_MAP_PER_SERVER];
size_t map_num = 0;

int  map_read_flag = READ_FROM_GAT;
char map_cache_file[256]="db/map.info";


static size_t users=0;
/*==========================================
 * 全map鯖?計での接??設定
 * (char鯖から送られてくる)
 *------------------------------------------
 */
void map_setusers(int fd) 
{
	if( session_isActive(fd) )
	{
		users = RFIFOL(fd,2);
		// send some anser
		WFIFOW(fd,0) = 0x2718;
		WFIFOSET(fd,2);
	}
}

/*==========================================
 * 全map鯖?計での接??取得 (/wへの?答用)
 *------------------------------------------
 */
int map_getusers(void)
{
	return users;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// permanent char name storage
///////////////////////////////////////////////////////////////////////////////

// predeclarations
class CNameRequest;
class CNameStorage;

/// name storage baseclass.
/// only offers virtual destruction and upcast
class CNameStorageBase : public basics::noncopyable
{
public:
	uint32			char_id;	// char id 
protected:
	CNameStorageBase(uint32 cid) : char_id(cid)
	{}
public:
	virtual ~CNameStorageBase()
	{}

	virtual bool			is_resolved()	{ return false; }
	virtual CNameRequest*	get_request()	{ return NULL; }
	virtual CNameStorage*	get_storage()	{ return NULL; }
};

/// request element.
/// single-linked list of requests to the same char_id
class CNameRequest : public CNameStorageBase
{
public:

	uint32			req_id;	// requesting char
	CNameRequest*	next;	// pointer to the next requesting

	CNameRequest(uint32 cid, uint32 rid, CNameRequest*n=NULL)
		: CNameStorageBase(cid), req_id(rid), next(n)
	{}
	virtual ~CNameRequest()
	{
		if(next)
			delete next;
	}

	virtual CNameRequest*	get_request()	{ return this; }
};

/// name storage element.
class CNameStorage : public CNameStorageBase
{
public:
	char			name[24];	// char name

	CNameStorage(uint32 cid, const char*n) : CNameStorageBase(cid)
	{
		safestrcpy(name,sizeof(name), n);
	}
	virtual ~CNameStorage()
	{}

	virtual bool			is_resolved()	{ return true; }
	virtual CNameStorage*	get_storage()	{ return this; }
};


/// temporary wrapper around dbt.
/// remove when db rewrite finished
static struct _namereq_db
{
	dbt* db;

	_namereq_db() : db(numdb_init())
	{}
	~_namereq_db()
	{
		this->clean();
	}

	static void final(void *k,void *d)
	{
		CNameStorageBase *p = (CNameStorageBase *) d;
		if (p) delete p;
	}

	void clean()
	{
		if(this->db)
		{
			numdb_final(this->db, this->final);
			this->db=NULL;
		}
	}


	dbt* get_db()	{ return this->db; }

	CNameStorageBase* search(uint32 cid)
	{
		return (CNameStorageBase*)numdb_search(this->db,cid);
	}
	void insert(uint32 cid, CNameStorageBase* bas)
	{
		// just to be sure a possibly existing entry is removed
		CNameStorageBase* old = (CNameStorageBase*)numdb_erase(this->db, cid);
		if(old) delete old;
		numdb_insert(this->db, cid, bas);
	}
	CNameStorageBase* erase(uint32 cid)
	{
		return (CNameStorageBase*)numdb_erase(this->db, cid);
	}
} namereq_db;


///////////////////////////////////////////////////////////////////////////////
/// search a name.
/// without requesting it, (function unused)
const char * map_src_namedb(uint32 charid)
{
	CNameStorageBase *p = namereq_db.search(charid);
	CNameStorage *ps = (p)?p->get_storage():NULL;
	return ( ps )?ps->name:NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// request a name.
/// does request/solving automatically
bool map_req_namedb(const map_session_data &sd, uint32 charid) 
{
	CNameStorageBase *p = namereq_db.search(charid);
	if( p == NULL )
	{	// not in database -> request new

		// explicitly using this way since of an ugly VC7 bug
		// that errors on statements like "new typename(var.baseclass::baseelement)"
		// if baseclass is deeper than one inheritance
		// specifying the full inheritance again does not work on VC6
		const size_t rid =sd.block_list::id;
		namereq_db.insert(charid, new CNameRequest(charid, rid));

		// and request the name from char server
		chrif_searchcharid(charid);
	}
	else if( !p->is_resolved() )
	{	// has been requested but not yet resolved
		CNameRequest* pp = p->get_request();
		if(pp)
		{
			const size_t rid =sd.status.char_id;
			// queue in after the first entry
			pp->next = new CNameRequest(charid, rid, pp->next);
		}
	}
	else
	{	// resolved entry exists
		CNameStorage* ps = p->get_storage();
		if(ps)
		{
			clif_solved_charname(sd, charid, ps->name);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// resolve a name.
/// request answer from char server or 
/// called from player entering map server
void map_add_namedb(uint32 charid, const char *name)
{
	CNameStorageBase *p = namereq_db.search(charid);
	if( p == NULL)
	{	// not in database -> add a name entry
		namereq_db.insert(charid, new CNameStorage(charid, name));
	}
	else if( !p->is_resolved() )
	{	// there are pending requests, so process them

		CNameRequest*	pp = p->get_request();
		while(pp)
		{
			map_session_data *sd = map_session_data::charid2sd(pp->req_id);
			if(sd)
				clif_solved_charname(*sd, charid, name);

			pp = pp->next;
		}
		// delete the request entry from the storage
		p = namereq_db.erase(charid);
		if(p) delete p;

		// add a name entry
		namereq_db.insert(charid, new CNameStorage(charid, name));
	}
	else
	{	// entry is already in db
		// copy the name again, maybe it has changed
		CNameStorage *ps = p->get_storage();
		if(ps)
			safestrcpy(ps->name, sizeof(ps->name), name);
	}
}


///////////////////////////////////////////////////////////////////////////////
// permanent char name storage
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// day night cycle
///////////////////////////////////////////////////////////////////////////////


// timer for night.day implementation
int daynight_flag = 0; // 0=day, 1=night
int daynight_timer_tid = -1;

///////////////////////////////////////////////////////////////////////////////
/// timer entry for day/night cycle
///
int map_daynight_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	// the new flag
	// toggle, when normal cycle, stay otherwise
	int new_flag = !daynight_flag &&  config.night_duration || daynight_flag && !config.day_duration;

	if(tid == -1)
	{	// called externally, so we take over the given value
		new_flag = (data.num!=0);
		// clean existing timer
		if( daynight_timer_tid != -1 )
			delete_timer(daynight_timer_tid, map_daynight_timer);
	}
	else if(tid != daynight_timer_tid)
	{
		ShowWarning("timerid %i!=%i", tid, daynight_timer_tid);
		return 0;
	}

	// timer for the next change, use defaut timing when none is given
	daynight_timer_tid = add_timer(gettick()+(new_flag?(config.night_duration?config.night_duration:60000):(config.day_duration?config.day_duration:60000)), map_daynight_timer, 0, 0);

	if( new_flag != daynight_flag )
	{
		daynight_flag = new_flag;

		// set clients and send messages
		const char *str = msg_txt((daynight_flag)?MSG_NIGHT_HAS_FALLEN:MSG_DAY_HAS_ARRIVED);
		size_t sz = 1+strlen(str);
		map_session_data *psd = NULL;

		map_session_data::iterator iter(map_session_data::nickdb());
		for(; iter; ++iter)
		{
			psd = iter.data();
			if(psd)
			{
				clif_wis_message(psd->fd, wisp_server_name, str, sz);

				// initiate night effect
				if(daynight_flag)
				{	
					clif_seteffect(*psd, psd->block_list::id, 474 + config.night_darkness_level);
				}
				// remove night effect
				else if (config.night_darkness_level > 0)
				{	
					clif_refresh (*psd);
				}
				else
				{
					psd->opt2 &= ~STATE_BLIND;
					clif_changeoption(*psd);
				}
			}
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// day night cycle
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// block_list members
//
///////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////
/// block化?理
/// maps[]のblock_listから?がっている場合に
/// bl->prevにbl_headのアドレスを入れておく
static block_list bl_head;

/////////////////////////////////////////////////////////////////////
dbt* block_list::id_db=NULL;

static block_list *objects[MAX_FLOORITEM];
static int first_free_object_id=0,last_object_id=0;

/// !! rewrite, not threadsafe
#define block_free_max 1048576
static block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

/// !! remove, highly insecure/not threadsafe
#define BL_LIST_MAX 1048576
static block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;


///////////////////////////////////////////////////////////////////////////////
/// returns blocklist from given id
block_list * block_list::from_blid(uint32 id)
{
	block_list *bl=NULL;
	if((size_t)id<sizeof(objects)/sizeof(objects[0]))
		bl = objects[id];
	else
		bl = (block_list*)numdb_search(block_list::id_db,id);
	return bl;
}



///////////////////////////////////////////////////////////////////////////////
/// process blocks standing on a path.
int block_list::foreachinpath(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int range,object_t type)
{
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
/*
//////////////////////////////////////////////////////////////
//
// sharp shooting 1
//
//////////////////////////////////////////////////////////////
// problem: 
// finding targets standing on and within some range of the line
// (t1,t2 t3 and t4 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
// solution 1 (straight forward, but a bit calculation expensive)
// calculating perpendiculars from quesionable mobs to the straight line
// if the mob is hit then depends on the distance to the line
// 
// solution 2 (complex, need to handle many cases, but maybe faster)
// make a formula to deside if a given (x,y) is within a shooting area
// the shape can be ie. rectangular or triangular
// if the mob is hit then depends on if the mob is inside or outside the area
// I'm not going to implement this, but if somebody is interested
// in vector algebra, it might be some fun 

//////////////////////////////////////////////////////////////
// possible shooting ranges (I prefer the second one)
//////////////////////////////////////////////////////////////
//
//  ----------------                     ------
//  ----------------               ------------
// Sxxxxxxxxxxxxxxxxtarget    Sxxxxxxxxxxxxxxxxtarget
//  ----------------               ------------
//  ----------------                      -----
//
// the original code implemented the left structure
// might be not that realistic, so I changed to the other one
// I take "range" as max distance from the line
//////////////////////////////////////////////////////////////

	int i, blockcount = bl_list_count;
	block_list *bl;
	int c1,c2;

///////////
	double deltax,deltay;
	double k,kfact,knorm;
	double v1,v2,distance;
	double xm,ym,rd;
	int bx,by,bx0,bx1,by0,by1;
//////////////
	// no map
	if(m >=MAX_MAP_PER_SERVER ) return;

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;

	///////////////////////////////
	// stuff for a linear equation in xy coord to calculate 
	// the perpendicular from a block xy to the straight line
	deltax = (x1-x0);
	deltay = (y1-y0);
	kfact = (deltax*deltax+deltay*deltay);	// the sqare length of the line
	knorm = -deltax*x0-deltay*y0;			// the offset vector param

//ShowMessage("(%i,%i)(%i,%i) range: %i\n",x0,y0,x1,y1,range);

	if(kfact==0) return 0; // shooting at the standing position should not happen
	kfact = 1/kfact; // divide here and multiply in the loop

	range *= range; // compare with range^2 so we can skip a sqrt and signs

	///////////////////////////////
	// prepare shooting area check
	xm = (x1+x0)/2.0;
	ym = (y1+y0)/2.0;// middle point on the shooting line
	// the sqared radius of a circle around the shooting range
	// plus the sqared radius of a block
	rd = (x0-xm)*(x0-xm) + (y0-ym)*(y0-ym) + (range*range)
					+BLOCK_SIZE*BLOCK_SIZE/2;
	// so whenever a block midpoint is within this circle
	// some of the block area is possibly within the shooting range

	///////////////////////////////
	// what blocks we need to test
	// blocks covered by the xy position of begin and end of the line
	bx0 = x0/BLOCK_SIZE;
	bx1 = x1/BLOCK_SIZE;
	by0 = y0/BLOCK_SIZE;
	by1 = y1/BLOCK_SIZE;
	// swap'em for a smallest-to-biggest run
	if(bx0>bx1)	swap(bx0,bx1);
	if(by0>by1)	swap(by0,by1);

	// enlarge the block area by a range value and 1
	// so we can be sure to process all blocks that might touch the shooting area
	// in this case here with BLOCK_SIZE=8 and range=2 it will be only enlarged by 1
	// but I implement it anyway just in case that ranges will be larger 
	// or BLOCK_SIZE smaller in future
	i = (range/BLOCK_SIZE+1);//temp value
	if(bx0>i)				bx0 -=i; else bx0=0;
	if(by0>i)				by0 -=i; else by0=0;
	if(bx1+i<maps[m].bxs)	bx1 +=i; else bx1=maps[m].bxs-1;
	if(by1+i<maps[m].bys)	by1 +=i; else by1=maps[m].bys-1;


//ShowMessage("run for (%i,%i)(%i,%i)\n",bx0,by0,bx1,by1);
	for(bx=bx0; bx<=bx1; ++bx)
	for(by=by0; by<=by1; ++by)
	{	// block xy
		c1  = maps[m].block_count[bx+by*maps[m].bxs];		// number of elements in the block
		c2  = maps[m].block_mob_count[bx+by*maps[m].bxs];	// number of mobs in the mob block
		if( (c1==0) && (c2==0) ) continue;				// skip if nothing in the blocks

//ShowMessage("block(%i,%i) %i %i\n",bx,by,c1,c2);fflush(stdout);
		// test if the mid-point of the block is too far away
		// so we could skip the whole block in this case 
		v1 = (bx*BLOCK_SIZE+BLOCK_SIZE/2-xm)*(bx*BLOCK_SIZE+BLOCK_SIZE/2-xm)
			+(by*BLOCK_SIZE+BLOCK_SIZE/2-ym)*(by*BLOCK_SIZE+BLOCK_SIZE/2-ym);
//ShowMessage("block(%i,%i) v1=%f rd=%f\n",bx,by,v1,rd);fflush(stdout);		
		// check for the worst case scenario
		if(v1 > rd)	continue;

		// it seems that the block is at least partially covered by the shooting range
		// so we go into it
		if(type==0 || type!=BL_MOB) {
  			bl = maps[m].block[bx+by*maps[m].bxs];		// a block with the elements
			for(i=0;i<c1 && bl;i++,bl=bl->next){		// go through all elements
				if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
				{
					// calculate the perpendicular from block xy to the straight line
					k = kfact*(deltax*bl->x + deltay*bl->y + knorm);
					// check if the perpendicular is within start and end of our line
					if(k>=0 && k<=1)
					{	// calculate the distance
						v1 = deltax*k+x0 - bl->x;
						v2 = deltay*k+y0 - bl->y;
						distance = v1*v1+v2*v2;
						// triangular shooting range
						if( distance <= range*k*k )
							bl_list[bl_list_count++]=bl;
					}
				}
			}//end for elements
		}

		if(type==0 || type==BL_MOB) {
			bl = maps[m].block_mob[bx+by*maps[m].bxs];	// and the mob block
			for(i=0;i<c2 && bl;i++,bl=bl->next){
				if(bl && bl_list_count<BL_LIST_MAX) {
					// calculate the perpendicular from block xy to the straight line
					k = kfact*(deltax*bl->x + deltay*bl->y + knorm);
//ShowMessage("mob: (%i,%i) k=%f ",bl->x,bl->y, k);
					// check if the perpendicular is within start and end of our line
					if(k>=0 && k<=1)
					{
						 v1 = deltax*k+x0 - bl->x;
						 v2 = deltay*k+y0 - bl->y;
						 distance = v1*v1+v2*v2;
//ShowMessage("dist: %f",distance);
						 // triangular shooting range
						 if( distance <= range*k*k )
						 {
//ShowMessage("  hit");
							bl_list[bl_list_count++]=bl;
						 }
					}
//ShowMessage("\n");
				}
			}//end for mobs
		}
	}//end for(bx,by)


	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowWarning("map_foreachinpath: *WARNING* block count too many!\n");
	}

	
	block_list::map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// 有?かどうかチェック
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;

*/
/*
//////////////////////////////////////////////////////////////
//
// sharp shooting 2
//
//////////////////////////////////////////////////////////////
// problem: 
// finding targets standing exactly on a line
// (only t1 and t2 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
	int i, blockcount = bl_list_count;
	block_list *bl;
	int c1,c2;

	//////////////////////////////////////////////////////////////
	// linear parametric equation
	// x=(x1-x0)*t+x0; y=(y1-y0)*t+y0; t=[0,1]
	//////////////////////////////////////////////////////////////
	// linear equation for finding a single line between (x0,y0)->(x1,y1)
	// independent of the given xy-values
	double dx = 0.0;
	double dy = 0.0;
	int bx=-1;	// initialize block coords to some impossible value
	int by=-1;
	int t, tmax;

	// no map
	if(m >= MAX_MAP_PER_SERVER) return;

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;

	///////////////////////////////
	// find maximum runindex
	tmax = abs(y1-y0);
	if(tmax  < abs(x1-x0))	
		tmax = abs(x1-x0);
	// pre-calculate delta values for x and y destination
	// should speed up cause you don't need to divide in the loop
	if(tmax>0)
	{
		dx = ((double)(x1-x0)) / ((double)tmax);
		dy = ((double)(y1-y0)) / ((double)tmax);
	}
	// go along the index
	for(t=0; t<=tmax; ++t)
	{	// xy-values of the line including start and end point
		int x = (int)floor(dx * (double)t +0.5)+x0;
		int y = (int)floor(dy * (double)t +0.5)+y0;

		// check the block index of the calculated xy
		if( (bx!=x/BLOCK_SIZE) || (by!=y/BLOCK_SIZE) )
		{	// we have reached a new block
			// so we store the current block coordinates
			bx = x/BLOCK_SIZE;
			by = y/BLOCK_SIZE;

			// and process the data
			c1  = maps[m].block_count[bx+by*maps[m].bxs];		// number of elements in the block
			c2  = maps[m].block_mob_count[bx+by*maps[m].bxs];	// number of mobs in the mob block
			if( (c1==0) && (c2==0) ) continue;				// skip if nothing in the block

			if(type==0 || type!=BL_MOB) {
				bl = maps[m].block[bx+by*maps[m].bxs];		// a block with the elements
				for(i=0;i<c1 && bl;i++,bl=bl->next){		// go through all elements
					if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
					{	
						// check if block xy is on the line
						if( abs((bl->x-x0)*(y1-y0) - (bl->y-y0)*(x1-x0)) <= tmax/2 )

						// and if it is within start and end point
						if( (((x0<=x1)&&(x0<=bl->x)&&(bl->x<=x1)) || ((x0>=x1)&&(x0>=bl->x)&&(bl->x>=x1))) &&
							(((y0<=y1)&&(y0<=bl->y)&&(bl->y<=y1)) || ((y0>=y1)&&(y0>=bl->y)&&(bl->y>=y1))) )
							bl_list[bl_list_count++]=bl;
					}
				}//end for elements
			}

			if(type==0 || type==BL_MOB) {
				bl = maps[m].block_mob[bx+by*maps[m].bxs];	// and the mob block
				for(i=0;i<c2 && bl;i++,bl=bl->next){
					if(bl && bl_list_count<BL_LIST_MAX) {
						// check if mob xy is on the line
						if( abs((bl->x-x0)*(y1-y0) - (bl->y-y0)*(x1-x0)) <= tmax/2 )

						// and if it is within start and end point
						if( (((x0<=x1)&&(x0<=bl->x)&&(bl->x<=x1)) || ((x0>=x1)&&(x0>=bl->x)&&(bl->x>=x1))) &&
							(((y0<=y1)&&(y0<=bl->y)&&(bl->y<=y1)) || ((y0>=y1)&&(y0>=bl->y)&&(bl->y>=y1))) )
							bl_list[bl_list_count++]=bl;
					}
				}//end for mobs
			}	
		}
	}//end for index

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowWarning("map_foreachinpath: *WARNING* block count too many!\n");
	}

	block_list::map_freeblock_lock();	// メモリからの解放を禁止する

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// 有?かどうかチェック
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;
*/

//////////////////////////////////////////////////////////////
//
// sharp shooting 2 version 2
// mix between line calculation and point storage
//////////////////////////////////////////////////////////////
// problem: 
// finding targets standing exactly on a line
// (only t1 and t2 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
	int i,k, blockcount = bl_list_count;
	block_list *bl;
	int c1,c2;

	//////////////////////////////////////////////////////////////
	// linear parametric equation
	// x=(x1-x0)*t+x0; y=(y1-y0)*t+y0; t=[0,1]
	//////////////////////////////////////////////////////////////
	// linear equation for finding a single line between (x0,y0)->(x1,y1)
	// independent of the given xy-values
	double dx = 0.0;
	double dy = 0.0;
	int bx=-1;	// initialize block coords to some impossible value
	int by=-1;
	int t, tmax, x,y;

	int save_x[BLOCK_SIZE],save_y[BLOCK_SIZE],save_cnt=0;

	// no map
	if(m >= MAX_MAP_PER_SERVER) return 0;
	block_list::freeblock_lock();	// メモリからの解放を禁止する

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x0 >= maps[m].xs) x0 = maps[m].xs-1;
	if (y0 >= maps[m].ys) y0 = maps[m].ys-1;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;

	///////////////////////////////
	// find maximum runindex, 
	if( abs(y1-y0) > abs(x1-x0) )
		tmax = abs(y1-y0);
	else
		tmax = abs(x1-x0);
	// pre-calculate delta values for x and y destination
	// should speed up cause you don't need to divide in the loop
	if(tmax>0)
	{
		dx = ((double)(x1-x0)) / ((double)tmax);
		dy = ((double)(y1-y0)) / ((double)tmax);
	}
	// go along the index t from 0 to tmax
	t=0;
	do {	
		x = (int)floor(dx * (double)t +0.5)+x0;
		y = (int)floor(dy * (double)t +0.5)+y0;


		// check the block index of the calculated xy, or the last block
		if( (bx!=x/BLOCK_SIZE) || (by!=y/BLOCK_SIZE) || t>tmax)
		{	// we have reached a new block

			// and process the data of the formerly stored block, if any
			if( save_cnt!=0 )
			{
				c1  = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;	// number of elements in the block
				c2  = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;	// number of mobs in the mob block
				if( (c1!=0) || (c2!=0) )							// skip if nothing in the block
				{
					if(type==0 || type!=BL_MOB)
					{
						bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;	// a block with the elements
						for(i=0;i<c1 && bl;++i,bl=bl->next)
						{	// go through all elements
							if( bl && bl->is_type(type) && bl_list_count<BL_LIST_MAX )
							{	// check if block xy is on the line
								for(k=0; k<save_cnt; ++k)
								{
									if( (save_x[k]==bl->x)&&(save_y[k]==bl->y) )
									{
										bl_list[bl_list_count++]=bl;
										break;
									}
								}
							}
						}//end for elements
					}

					if(type==0 || type==BL_MOB)
					{
						bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;	// and the mob block
						for(i=0;i<c2 && bl;i++,bl=bl->next)
						{
							if(bl && bl_list_count<BL_LIST_MAX)
							{	// check if mob xy is on the line
								for(k=0; k<save_cnt; ++k)
								{
									if( (save_x[k]==bl->x)&&(save_y[k]==bl->y) )
									{
										bl_list[bl_list_count++]=bl;
										break;
									}
								}
							}
						}//end for mobs
					}
				}
				// reset the point storage
				save_cnt=0;
			}

			// store the current block coordinates
			bx = x/BLOCK_SIZE;
			by = y/BLOCK_SIZE;
		}
		// store the new point of the line
		save_x[save_cnt]=x;
		save_y[save_cnt]=y;
		save_cnt++;
	}while( t++ <= tmax );

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowWarning("map_foreachinpath: *WARNING* block count too many!\n");
	}

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)	// 有?かどうかチェック
		{
			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;

	return returnCount;

}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing on a exact position.
int block_list::foreachincell(const CMapProcessor& elem, unsigned short m,int x,int y,object_t type)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	block_list::freeblock_lock();	// メモリからの解放を禁止する

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type==0 || type!=BL_MOB)
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if( bl && !bl->is_type(type) )
				continue;
			if( bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX )
				bl_list[bl_list_count++]=bl;
		}
	}

	if(type==0 || type==BL_MOB)
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowMessage("map_foreachincell: *WARNING* block count too many!\n");
	}

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += elem.process(*bl_list[i]);
	}
	block_list::freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;
	return returnCount;
}

///////////////////////////////////////////////////////////////////////////////
/// count blocks standing on a exact position.
int block_list::countoncell(unsigned short m, int x, int y, object_t type)
{
	int bx,by;
	block_list *bl=NULL;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= maps[m].xs) || (y >= maps[m].ys))
		return 0;
	block_list::freeblock_lock();

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if( type == BL_ALL || type != BL_MOB )
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if( bl->x == x && bl->y == y && *bl == BL_PC )
				++count;
		}
	}
	if( type == BL_ALL || type == BL_MOB )
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if( bl->x == x && bl->y == y )
				++count;
		}
	}
	block_list::freeblock_unlock();
	return count;
}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing inside a move area.
/// (move area is the difference cut between the source and target area
int block_list::foreachinmovearea(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,object_t type)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	block_list::freeblock_lock();	// メモリからの解放を禁止する

	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);
	if(dx==0 || dy==0)
	{
		// 矩形領域の場合
		if(dx==0){
			if(dy<0){
				y0=y1+dy+1;
			} else {
				y1=y0+dy-1;
			}
		} else if(dy==0){
			if(dx<0){
				x0=x1+dx+1;
			} else {
				x1=x0+dx-1;
			}
		}
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=maps[m].xs) x1=maps[m].xs-1;
		if(y1>=maps[m].ys) y1=maps[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by)
		{
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx)
			{
				bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
				c  = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
				for(i=0;i<c && bl;i++,bl=bl->next)
				{
					if(bl && !bl->is_type(type) )
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
				c  = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
				for(i=0;i<c && bl;i++,bl=bl->next)
				{
					if(bl && !bl->is_type(type) )
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}
	else
	{	// L字領域の場合

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=maps[m].xs) x1=maps[m].xs-1;
		if(y1>=maps[m].ys) y1=maps[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by)
		for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
			c  = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if( bl && !bl->is_type(type) )
					continue;
				if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
					continue;
				if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
					(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
					bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
			}
			bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
			c  = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if( bl && !bl->is_type(type) )
					continue;
				if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
					continue;
				if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
					(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
					bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
			}
		}
	}

	if(bl_list_count>=BL_LIST_MAX)
	{
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)
		{	// 有?かどうかチェック

			map_session_data *sd = bl_list[i]->get_sd();
			if( sd && !session_isActive(sd->fd) )
				continue;

			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;
	return returnCount;
}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing inside an area.
int block_list::foreachinarea(const CMapProcessor& elem, unsigned short m, int x0,int y0,int x1,int y1,object_t type)
{
	int bx,by;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if(m >= map_num )
		return 0;
	block_list::freeblock_lock();	// メモリからの解放を禁止する
	
	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;
	if (type == 0 || type != BL_MOB)
	{
		for(by = y0/BLOCK_SIZE; by <= y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx <= x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
			c  = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && !bl->is_type(type) )
					continue;
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}
	if(type==0 || type==BL_MOB)
	{
		for(by = y0/BLOCK_SIZE; by <= y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx <= x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
			c  = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}

	if(bl_list_count>=BL_LIST_MAX)
	{
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}


	for(i=blockcount;i<bl_list_count;++i)
		if(bl_list[i] && bl_list[i]->prev)	// 有?かどうかチェック
			returnCount += elem.process(*bl_list[i]);

	block_list::freeblock_unlock();	// 解放を許可する
	bl_list_count = blockcount;

	return returnCount;	//[Skotlex]
}
///////////////////////////////////////////////////////////////////////////////
/// process party members on same map.
int block_list::foreachpartymemberonmap(const CMapProcessor& elem, map_session_data &sd, bool area)
{
	int returncount=0;
	struct party *p;
	int x0,y0,x1,y1;
	block_list *list[MAX_PARTY];
	size_t i, blockcount=0;
	
	if((p=party_search(sd.status.party_id))==NULL)
		return 0;
	block_list::freeblock_lock();	// メモリからの解放を禁止する

	x0=sd.block_list::x-AREA_SIZE;
	y0=sd.block_list::y-AREA_SIZE;
	x1=sd.block_list::x+AREA_SIZE;
	y1=sd.block_list::y+AREA_SIZE;

	for(i=0;i<MAX_PARTY;++i)
	{
		struct party_member *m=&p->member[i];
		if(m->sd!=NULL)
		{
			if(sd.block_list::m != m->sd->block_list::m)
				continue;
			if(area &&
				(m->sd->block_list::x<x0 || m->sd->block_list::y<y0 ||
				 m->sd->block_list::x>x1 || m->sd->block_list::y>y1 ) )
				continue;
			list[blockcount++] = m->sd; 
		}
	}

	for(i=0;i<blockcount;++i)
	{
		if(list[i] && list[i]->prev)	// 有効かどうかチェック
		{
			returncount += elem.process(*list[i]);
		}
	}
	block_list::freeblock_unlock();	// 解放を許可する
	return returncount;
}
///////////////////////////////////////////////////////////////////////////////
/// process objects.
int block_list::foreachobject(const CMapProcessor& elem, object_t type)
{
	int returncount=0;
	int i;
	int blockcount=bl_list_count;
	block_list::freeblock_lock();

	for(i=2;i<=last_object_id;++i)
	{
		if(objects[i])
		{
			if( !objects[i]->is_type(type) )
				continue;
			if(bl_list_count>=BL_LIST_MAX)
			{
				if(config.error_log)
					ShowMessage("map_foreachobject: too many block !\n");
				break;
			}
			else
				bl_list[bl_list_count++]=objects[i];
		}
	}


	for(i=blockcount;i<bl_list_count;++i)
	{
		if( bl_list[i]->prev || bl_list[i]->next )
		{
			returncount += elem.process(*bl_list[i]);
		}
	}
	block_list::freeblock_unlock();
	bl_list_count = blockcount;
	return returncount;
}


///////////////////////////////////////////////////////////////////////////////
/// get a skillunit on a specific cell.
skill_unit *block_list::skillunit_oncell(block_list &target, int x, int y, ushort skill_id, skill_unit *out_unit)
{
	int m,bx,by;
	block_list *bl;
	int i,c;
	struct skill_unit *unit, *ret=NULL;
	m = target.m;

	if (x < 0 || y < 0 || (x >= maps[m].xs) || (y >= maps[m].ys))
		return NULL;
	block_list::freeblock_lock();

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
	c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
	for(i=0;i<c && bl;i++,bl=bl->next)
	{
		if (bl->x != x || bl->y != y || *bl != BL_SKILL)
			continue;
		unit = (struct skill_unit *) bl;
		if (unit==out_unit || !unit->alive ||
				!unit->group || unit->group->skill_id!=skill_id)
			continue;
		if (battle_check_target(unit,&target,unit->group->target_flag)>0)
		{
			ret = unit;
			break;
		}
	}
	block_list::freeblock_unlock();
	return ret;
}


/// check if withing AREA range.
bool block_list::is_near(const block_list& bl) const
{
	return ( this->is_on_map() && bl.is_on_map() &&
			 this->m == bl.m &&
			 this->x+AREA_SIZE > bl.x &&
			 this->x           < bl.x+AREA_SIZE &&
			 this->y+AREA_SIZE > bl.y && 
			 this->y           < bl.y+AREA_SIZE );
}










///////////////////////////////////////////////////////////////////////////////
/// add id to id_db
/// * id_dbへblを追加
void block_list::addiddb() 
{
	numdb_insert(block_list::id_db, this->id, this);
}

///////////////////////////////////////////////////////////////////////////////
/// remove id from id_db
/// id_dbからblを削除
void block_list::deliddb() 
{
	numdb_erase(block_list::id_db,this->id);
}



///////////////////////////////////////////////////////////////////////////////
///
/// maps[]のblock_listに追加
/// mobは?が多いので別リスト
///
/// ?にlink?みかの確認が無い。危?かも
bool block_list::addblock()
{
	if(this->prev != NULL)
	{
		if(config.error_log)
			ShowMessage("map_addblock error : bl->prev!=NULL (id=%lu)\n",(ulong)this->id);
		this->delblock();
	}

	if( this->m<map_num && this->x<maps[this->m].xs && this->y<maps[this->m].ys )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_data::_objects &obj = maps[this->m].objects[pos];

		if( this->is_type(BL_MOB) )
		{
			this->next = obj.root_mob;
			this->prev = &bl_head;
			if(this->next) this->next->prev = this;
			obj.root_mob = this;
			++obj.cnt_mob;
		}
		else
		{
			this->next = obj.root_blk;
			this->prev = &bl_head;
			if( this->next ) this->next->prev = this;
			obj.root_blk = this;
			++obj.cnt_blk;

			if( this->is_type(BL_PC) )
			{
				map_session_data& sd = *this->get_sd();
				if( agit_flag && config.pet_no_gvg && maps[this->m].flag.gvg && sd.pd)
				{	//Return the pet to egg. [Skotlex]
					clif_displaymessage(sd.fd, "Pets are not allowed in Guild Wars.");
					pet_menu(sd, 3); // Option 3 is return to egg.
				}
				if(maps[this->m].users++ == 0 && config.dynamic_mobs)	// Skotlex
					map_spawnmobs(this->m);
			}
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// maps[]のblock_listから外す
/// prevがNULLの場合listに?がってない
bool block_list::delblock()
{
	// ?にblocklistから?けている
	if(this->prev==NULL)
	{
		if(this->next!=NULL)
		{	// prevがNULLでnextがNULLでないのは有ってはならない
			if(config.error_log)
				ShowError("map_delblock error : bl->next!=NULL\n");
		}
	}
	else if( this->m < map_num )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_data::_objects &obj = maps[this->m].objects[pos];

		if( this->is_type(BL_PC) )
		{
			if( maps[this->m].users>0 && --maps[this->m].users == 0 && config.dynamic_mobs)
				map_removemobs(this->m);
		}
		
		if( this->is_type(BL_MOB) )
		{
			if( obj.cnt_mob > 0 )
				--obj.cnt_mob;
		}
		else
		{
			if( obj.cnt_blk > 0 )
				--obj.cnt_blk;
		}

		if( this->next )
			this->next->prev = this->prev;

		if( this->prev == &bl_head )
		{	// head entry
			if( this->is_type(BL_MOB) )
				obj.root_mob = this->next;
			else
				obj.root_blk = this->next;
		}
		else
		{
			this->prev->next = this->next;
		}

		this->next = NULL;
		this->prev = NULL;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// 
/// block削除の安全性確保?理
int block_list::freeblock()
{
	if (block_free_lock == 0)
	{
		delete this;
	}
	else
	{
		if (block_free_count >= block_free_max)
		{
			if (config.error_log)
				ShowWarning("map_freeblock: too many free block! %d %d\n",
					block_free_count, block_free_lock);
		}
		else block_free[block_free_count++] = this;
	}
	return block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// blockのfreeを一市Iに禁止する
int block_list::freeblock_lock (void)
{
	return ++block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// blockのaFreeのロックを解除する
/// このとき、ロックが完全になくなると
/// バッファにたまっていたblockを全部削除
int block_list::freeblock_unlock (void)
{
	if((--block_free_lock) == 0)
	{
		int i;
		for (i = 0; i < block_free_count; ++i)
		{
			if(block_free[i])
			{
				delete block_free[i];
				block_free[i] = NULL;
			}
		}
		block_free_count = 0;
	}
	else if (block_free_lock < 0)
	{
		if (config.error_log)
			ShowError("map_freeblock_unlock: lock count < 0 !\n");
		block_free_lock = 0; // 次回以降のロックに支障が出てくるのでリセット
	}
	return block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// map_freeblock_lock() を呼んで map_freeblock_unlock() を呼ばない
/// 関数があったので、定期的にblock_free_lockをリセットするようにする。
/// この関数は、do_timer() のトップレベルから呼ばれるので、
/// block_free_lock を直接いじっても支障無いはず。
int map_freeblock_timer (int tid, unsigned long tick, int id, basics::numptr data)
{
	if(block_free_lock > 0)
	{
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		block_list::freeblock_unlock();
	}
	return 0;
}










/*==========================================
 * 床アイテムやエフェクト用の一三bj割り?て
 * object[]への保存とid_db登?まで
 *
 * bl->idもこの中で設定して問題無い?
 *------------------------------------------
 */
int map_addobject(block_list &bl)
{
	int i;
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM;++i)
		if(objects[i]==NULL)
			break;
	if(i>=MAX_FLOORITEM)
	{
		if(config.error_log)
			ShowMessage("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	objects[i]=&bl;
	numdb_insert(block_list::id_db, i, &bl);
	return i;
}

/*==========================================
 * 一三bjectの解放
 *	map_delobjectのaFreeしないバ?ジョン
 *------------------------------------------
 */
int map_delobjectnofree(int id)
{
	if(objects[id]==NULL)
		return 0;

	objects[id]->delblock();
	objects[id]->deliddb();
	objects[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && objects[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * 一三bjectの解放
 * block_listからの削除、id_dbからの削除
 * object dataのaFree、object[]へのNULL代入
 *
 * addとの??性が無いのが?になる
 *------------------------------------------
 */
int map_delobject(int id)
{
	block_list *obj = objects[id];

	if(obj==NULL)
		return 0;

	map_delobjectnofree(id);
	obj->freeblock();

	return 0;
}



/*==========================================
 * 床アイテムを消す
 *
 * data==0の暫ﾍtimerで消えた殊 * data!=0の暫ﾍ拾う等で消えた暫ﾆして動作
 *
 * 後者は、map_clearflooritem(id)へ
 * map.h?で#defineしてある
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	flooritem_data *fitem = (objects[id]) ? objects[id]->get_fd() : NULL;
	if( fitem==NULL || (!data.num && fitem->cleartimer != tid))
	{
		if(config.error_log)
			ShowMessage("map_clearflooritem_timer : error\n");
		return 1;
	}
	if(data.num)
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	else if(fitem->item_data.card[0] == 0xff00)
		intif_delete_petdata( basics::MakeDWord(fitem->item_data.card[1],fitem->item_data.card[2]) );
	clif_clearflooritem(*fitem);
	map_delobject(fitem->block_list::id);

	return 0;
}


/*==========================================
 * (m,x,y)の周?rangeマス?の空き(=侵入可能)cellの
 * ?から適?なマス目の座標をx+(y<<16)で返す
 *
 * 現?range=1でアイテムドロップ用途のみ
 *------------------------------------------
 */
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range)
{
	int free_cell,i,j;

	if (range > 15 || m>=map_num)
		return -1;
	
	CREATE_BUFFER(free_cells, uint32, (2*range+1)*(2*range+1));

	for(free_cell=0,i=-range;i<=range;++i)
	{
		if(i+y<0 || i+y>=maps[m].ys)
			continue;
		for(j=-range;j<=range; ++j)
		{
			if(j+x<0 || j+x>=maps[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(block_list::countoncell(m,j+x,i+y, BL_ITEM) > 1)
				continue;
			free_cells[free_cell++] = j+x+((i+y)<<16);
		}
	}
	free_cell = (free_cell==0)?-1:(int)free_cells[rand()%free_cell];
	DELETE_BUFFER(free_cells);






	return free_cell;
}

/*==========================================
 * (m,x,y)を中心に3x3以?に床アイテム設置
 *
 * item_dataはamount以外をcopyする
 *------------------------------------------
 */
int map_addflooritem(const struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y,
					 map_session_data *first_sd,map_session_data *second_sd,map_session_data *third_sd,int type)
{
	int xy,r;
	unsigned long tick;
	flooritem_data *fitem=NULL;

	if((xy=map_searchrandfreecell(m,x,y,2))<0)
		return 0;
	r=rand();

	fitem = new flooritem_data;
	fitem->block_list::m=m;
	fitem->block_list::x= xy&0xffff;
	fitem->block_list::y=(xy>>16)&0xffff;
	fitem->first_get_id = 0;
	fitem->first_get_tick = 0;
	fitem->second_get_id = 0;
	fitem->second_get_tick = 0;
	fitem->third_get_id = 0;
	fitem->third_get_tick = 0;

	fitem->block_list::id = map_addobject(*fitem);
	if(fitem->block_list::id==0){
		delete fitem;
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->block_list::id;
		if(type)
			fitem->first_get_tick = tick + config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->block_list::id;
		if(type)
			fitem->second_get_tick = tick + config.mvp_item_first_get_time + config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + config.item_first_get_time + config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->block_list::id;
		if(type)
			fitem->third_get_tick = tick + config.mvp_item_first_get_time + config.mvp_item_second_get_time + config.mvp_item_third_get_time;
		else
			fitem->third_get_tick = tick + config.item_first_get_time + config.item_second_get_time + config.item_third_get_time;
	}

	fitem->item_data = item_data;
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=add_timer(gettick()+config.flooritem_lifetime,map_clearflooritem_timer,fitem->block_list::id,0);

	fitem->addblock();
	clif_dropflooritem(*fitem);

	return fitem->block_list::id;
}






/*==========================================
 * PCのquit?理 map.c?分
 *
 * quit?理の主?が違うような?もしてきた
 *------------------------------------------
 */
int map_quit(map_session_data &sd) 
{
	if( sd.state.event_disconnect )
	{
		if( script_config.event_script_type == 0 )
		{
			npcscript_data *sc = npcscript_data::from_name(script_config.logout_event_name);
			if( sc && sc->ref && (sc->block_list::m==0xFFFF || sc->block_list::m==sd.block_list::m) )
			{
				CScriptEngine::run(sc->ref->script,0,sd.block_list::id,sc->block_list::id); // PCLogoutNPC
				ShowStatus ("Event '"CL_WHITE"%s"CL_RESET"' executed.\n", script_config.logout_event_name);
			}
		}
		else
		{
			int evt = npc_event_doall("OnPCLogoutEvent", sd.block_list::id, sd.block_list::m);
			if(evt) ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n", evt, "OnPCLogoutEvent");
		}
	}

	sd.leavechat();

	if(sd.trade_partner)	// 取引を中?する	
		trade_tradecancel(sd);
	if(sd.party_invite>0)	// パ?ティ?誘を拒否する
		party_reply_invite(sd,sd.party_invite_account,0);
	if(sd.guild_invite>0)	// ギルド?誘を拒否する
		guild_reply_invite(sd,sd.guild_invite,0);
	if(sd.guild_alliance>0)	// ギルド同盟?誘を拒否する
		guild_reply_reqalliance(sd,sd.guild_alliance_account,0);

	party_send_logout(sd);	// パ?ティのログアウトメッセ?ジ送信
	party_send_dot_remove(sd);
	guild_send_memberinfoshort(sd,0);	// ギルドのログアウトメッセ?ジ送信

	chrif_save_sc(sd);

	pc_cleareventtimer(sd);	// イベントタイマを破棄する
	if(sd.state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);	// 倉庫を開いてるなら保存する

	// check if we've been authenticated [celest]
	if (sd.state.auth)
		skill_castcancel(&sd,0);	// 詠唱を中?する

	skill_stop_dancing(&sd,1);// ダンス/演奏中?
	if(sd.sc_data && sd.sc_data[SC_BERSERK].timer!=-1) //バ?サ?ク中の終了はHPを100に
	{
		sd.status.hp = 100;
		status_change_end(&sd,SC_BERSERK,-1);
	}

	status_change_clear(&sd,1);	// ステ?タス異常を解除する
	skill_clear_unitgroup(&sd);	// スキルユニットグル?プの削除
	skill_cleartimerskill(&sd);

	sd.stop_walking(0);
	sd.stop_attack();
	pc_delinvincibletimer(sd);

	pc_delspiritball(sd,sd.spiritball,1);
	skill_gangsterparadise(&sd,0);
	skill_unit_move(sd,gettick(),0);
	
	if( sd.state.auth )
		status_calc_pc(sd,4);

	if( !(sd.status.option & OPTION_HIDE) )
		clif_clearchar_area(sd,2);

	if( sd.status.pet_id && sd.pd )
	{
		pet_lootitem_drop(*(sd.pd),&sd);
			pet_remove_map(sd);
		if(sd.pd->pet.intimate <= 0)
		{
			intif_delete_petdata(sd.status.pet_id);
			sd.status.pet_id = 0;
			sd.pd = NULL;
		}
		else
			intif_save_petdata(sd.status.account_id,sd.pd->pet);
	}
	if( sd.is_dead() )
		pc_setrestartvalue(sd,2);

	homun_data::clear_homunculus(sd);
	pc_clean_skilltree(sd);
	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_dirty(sd);
	storage_storage_save(sd);
	sd.delblock();

	
	sd.ScriptEngine.clear();

	chrif_char_offline(sd);
		
	if(sd.reg)
	{
		delete[] sd.reg;
		sd.reg=NULL;
	}
		
	if(sd.regstr)
	{
		delete[] sd.regstr;
		sd.regstr=NULL;
	}
	return 0;
}





/*==========================================
 * map.npcへ追加 (warp等の領域持ちのみ)
 *------------------------------------------
 */
int map_addnpc(unsigned short m, npc_data *nd)
{
	size_t i;
	if(m>=map_num)
		return -1;
	for(i=0;i<maps[m].npc_num && i<MAX_NPC_PER_MAP;++i)
		if(maps[m].npc[i]==NULL)
			break;
	if(i==MAX_NPC_PER_MAP){
		if(config.error_log)
			ShowMessage("too many NPCs in one map %s\n",maps[m].mapname);
		return -1;
	}
	if(i==maps[m].npc_num){
		maps[m].npc_num++;
	}

	nullpo_retr(0, nd);

	maps[m].npc[i]=nd;
	nd->n = i;
	numdb_insert(block_list::id_db, nd->block_list::id,nd);

	return i;
}


/*=========================================
 * Dynamic Mobs [Wizputer]
 *-----------------------------------------
 */

struct mob_list* map_addmobtolist(unsigned short m)
{
	size_t i;
    for(i=0; i<MAX_MOB_LIST_PER_MAP; ++i)
	{
		if(maps[m].moblist[i]==NULL)
		{
			maps[m].moblist[i] = new struct mob_list();
			return maps[m].moblist[i];
		}
	}
	return NULL;
}

void clear_moblist(unsigned short m)
{
	size_t i;
	if(m<MAX_MAP_PER_SERVER)
	for (i = 0; i < MAX_MOB_LIST_PER_MAP; ++i)
	{
		if(maps[m].moblist[i]!=NULL)
		{
			delete maps[m].moblist[i];
			maps[m].moblist[i] = NULL;
		}
	}
}

void map_spawnmobs(unsigned short m)
{
	size_t i, k=0;

	if(m>=map_num)
		return;

	if (maps[m].mob_delete_timer != -1)
	{	//Mobs have not been removed yet [Skotlex]
		delete_timer(maps[m].mob_delete_timer, map_removemobs_timer);
		maps[m].mob_delete_timer = -1;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; ++i)	
	{
		if(maps[m].moblist[i]!=NULL)
		{
			k+=maps[m].moblist[i]->num;
			npc_parse_mob2(*maps[m].moblist[i]);
		}
	}
	if (config.etc_log && k > 0)
		ShowStatus("Map %s: Spawned '"CL_WHITE"%d"CL_RESET"' mobs.\n",maps[m].mapname, k);
}

class CMapMobCacheCleanup : public CMapProcessor
{
public:
	CMapMobCacheCleanup()	{}
	~CMapMobCacheCleanup()	{}
	virtual int process(block_list& bl) const
	{
		// When not to remove:
		//  1: Mob is not from a cache
		//  2: Mob is damaged 

		struct mob_data *md = bl.get_md();
		if( md &&
			// cached, not delayed and not already on delete schedule
			(md->cache && !md->cache->delay1 && !md->cache->delay2 && -1==md->deletetimer) &&
			// unhurt enemies	
			(config.mob_remove_damaged || (md->hp == md->max_hp)) )
		{
			// cleaning a master will also clean its slaves
			if( md->state.is_master )
				mob_deleteslave(*md);
			// check the mob into the cache
			md->cache->num++;
			// and unload it
			mob_unload(*md);	
			return 1;
		}
		return 0;
	}
};
int map_removemobs_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	int k;
	unsigned short m = id;
	if(m >= map_num)
	{	//Incorrect map id!
		if (config.error_log)
			ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, m);
		return 0;
	}
	if (maps[m].mob_delete_timer != tid)
	{	//Incorrect timer call!
		if (config.error_log)
			ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",maps[m].mob_delete_timer, tid, maps[m].mapname);
		return 0;
	}
	maps[m].mob_delete_timer = -1;
	if (maps[m].users > 0) //Map not empty!
		return 1;

	k = block_list::foreachinarea( CMapMobCacheCleanup(),
		m, 0, 0, maps[m].xs-1, maps[m].ys-1, BL_MOB);		
//	k = map_foreachinarea(mob_cache_cleanup_sub, 
//		m, 0, 0, maps[m].xs-1, maps[m].ys-1, BL_MOB);

	if (config.etc_log && k > 0)
		ShowStatus("Map %s: Removed '"CL_WHITE"%d"CL_RESET"' mobs.\n",maps[m].mapname, k);
	return 1;
}

void map_removemobs(unsigned short m)
{
	if (maps[m].mob_delete_timer != -1)
		return; //Mobs are already scheduled for removal

	maps[m].mob_delete_timer = add_timer(gettick()+config.mob_remove_delay, map_removemobs_timer, m, 0);
}

/*==========================================
 * map名からmap番?へ?換
 *------------------------------------------
 */
int map_mapname2mapid(const char *name)
{
	char finename[32], *wpp=finename;
	const char *rpp=name, *epp=finename+sizeof(finename)-1;

	// make a copy in order to cut off the extension
	if(rpp)
	while(*rpp && *rpp!='.' && wpp<epp) *wpp++ = *rpp++;
	*wpp=0;

	// lookup
	struct map_data *md = (struct map_data*)strdb_search(map_db, finename);
	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * 他鯖map名からip,port?換
 *------------------------------------------
 */
bool map_mapname2ipport(const char *name, basics::ipset &mapset)
{
	char finename[32], *wpp=finename;
	const char *rpp=name, *epp=finename+sizeof(finename)-1;

	// make a copy in order to cut off the extension
	if(rpp)
	while(*rpp && *rpp!='.' && wpp<epp) *wpp++ = *rpp++;
	*wpp=0;

	struct map_data_other_server *mdos = (struct map_data_other_server*)strdb_search(map_db, finename);
	if(mdos==NULL || mdos->gat)
		return false;
	mapset = mdos->mapset;
	return true;
}




// gat系
/*==========================================
 * (m,x,y)の状態を調べる
 *------------------------------------------
 */
/////////////////////////////////////////////////////////////////////
// as far as I have seen the gat.type values can contain
// GAT_NONE		= 0,
// GAT_WALL		= 1,
// GAT_UNUSED1	= 2,
// GAT_WATER	= 3,
// GAT_UNUSED2	= 4,
// GAT_GROUND	= 5,
// GAT_HOLE		= 6,	// holes in morroc desert
// GAT_UNUSED3	= 7,
// change the gat to a bitfield with three bits 
// instead of using an unsigned char have it merged with other usages
/////////////////////////////////////////////////////////////////////

int map_getcell(unsigned short m, unsigned short x, unsigned short y, cell_t cellchk)
{
	return (m>=map_num) ? (cellchk==CELL_CHKNOPASS) : map_getcellp(maps[m],x,y,cellchk);
}

int map_getcellp(struct map_data& m, unsigned short x, unsigned short y, cell_t cellchk)
{
	struct mapgat *mg;

	if(x>=m.xs || y>=m.ys)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	mg = m.gat + x + y*m.xs;

	switch(cellchk)
	{
	case CELL_CHKPASS:
		return (mg->type != GAT_WALL && mg->type != GAT_GROUND);
	case CELL_CHKNOPASS:
		return (mg->type == GAT_WALL || mg->type == GAT_GROUND);
	case CELL_CHKNOPASS_NPC:
		return (mg->type == GAT_WALL || mg->type == GAT_GROUND || mg->npc);
	case CELL_CHKWALL:
		return (mg->type == GAT_WALL);
	case CELL_CHKWATER:
		return (mg->type == GAT_WATER);
	case CELL_CHKGROUND:
		return (mg->type == GAT_GROUND);
	case CELL_CHKHOLE:
		return (mg->type == GAT_HOLE);
	case CELL_GETTYPE:
		return mg->type;
	case CELL_CHKNPC:
		return mg->npc;
	case CELL_CHKBASILICA:
		return mg->basilica;
	case CELL_CHKMOONLIT:
		return mg->moonlit;
	case CELL_CHKREGEN:
		return mg->regen;
	default:
		return 0;
	}
}
/*==========================================
 * (m,x,y)の状態を設定する
 *------------------------------------------
 */
void map_setcell(unsigned short m,unsigned short x, unsigned short y, int cellck)
{
	struct mapgat *mg;
	if(m >= MAX_MAP_PER_SERVER || x>=maps[m].xs || y>=maps[m].ys)
		return;

	mg = maps[m].gat+x+y*maps[m].xs;

	switch(cellck)
	{
	case CELL_SETNPC:
		if(mg->npc < 15) // max for a 4bit counter
			mg->npc++;
		else
			ShowWarning("usage of more then 15 stacked npc touchup areas\n");
			break;
	case CELL_CLRNPC:
		if(mg->npc > 0) // 4bit counter
			mg->npc--;
		//else no warning, has been warned at setting up the touchups already
		break;
		case CELL_SETBASILICA:
		mg->basilica = 1;
			break;
		case CELL_CLRBASILICA:
		mg->basilica = 0;
			break;
	case CELL_SETMOONLIT:
		mg->moonlit = 1;
		break;
	case CELL_CLRMOONLIT:
		mg->moonlit = 0;
		break;
		case CELL_SETREGEN:
		mg->regen = 1;
			break;
	case CELL_CLRREGEN:
		mg->regen = 0;
		break;
	default:
		// check the numbers from the gat and warn on an unknown type
		if( (cellck != GAT_NONE) && (cellck != GAT_WALL) && (cellck != GAT_WATER) && 
			(cellck != GAT_GROUND) && (cellck != GAT_HOLE) )
			ShowWarning("Setting mapcell with improper value %i on %s (%i,%i)\n", cellck,maps[m].mapname,x,y);
		else
			mg->type = cellck & CELL_MASK;
			break;
	};
}

/*==========================================
 * 他鯖管理のマップをdbに追加
 *------------------------------------------
 */
int map_setipport(const char *name, basics::ipset &mapset)
{
	struct map_data *md=NULL;
	struct map_data_other_server *mdos=NULL;

	md = (struct map_data*)strdb_search(map_db,name);
	if(md==NULL)
	{	// does not exist -> add new data
		mdos = new struct map_data_other_server;
		memcpy(mdos->name,name,24);
		mdos->gat  = NULL;
		mdos->mapset=mapset;
		mdos->map  = NULL;
		strdb_insert(map_db,mdos->name,mdos);
	}
	else if(md->gat)
	{	// exists and has is a locally loaded map
		if( mapset != getmapaddress() )
		{	// a differnt server then the local one is owning the map
			// 読み甲ﾅいたけど、担当外になったマップ
			mdos= new struct map_data_other_server;
			memcpy(mdos->name,name,24);
			mdos->gat  = NULL;
			mdos->mapset=mapset;
			mdos->map  = md;	// safe the locally loaded map data
			strdb_insert(map_db,mdos->name,mdos);
			// ShowMessage("from char server : %s -> %08lx:%d\n",name,ip,port);
		}
		else
		{	// nothing to do, just keep the map as it is
			// 読み甲ﾅいて、担当になったマップ（何もしない）
			;
		}
	}
	else
	{	// exists and is an other_server_map
		mdos=(struct map_data_other_server *)md;
		if( mapset == getmapaddress() )
		{	// the current server is claiming the map
			// 自分の担当になったマップ

			// reclaim the real mapdata
			// 読み甲ﾅいるので置き換える
			md = mdos->map;
			delete mdos;
			
			if(md)
				strdb_insert(map_db,md->mapname,md);
			else
				ShowError("map_setipport error: data for local map %s does not exist.\n",name);
		}
		else
		{	// the ip of the hostig mapserver has changed
			// 他の鯖の担当マップなので置き換えるだけ
			mdos->mapset=mapset;
		}
	}
	return 0;
}

/*==========================================
 * 他鯖管理のマップを全て削除
 *------------------------------------------
 */
int map_eraseallipport(void)
{
	db_iterator<const char*, map_data_other_server*> iter(map_db);
	for(;iter; ++iter)
	{
		map_data_other_server *mdos = iter.data();
		// decision between map_data and map_data_other_server 
		// is currently done with having member ->gat == NULL
		if(mdos && mdos->gat == NULL)
		{	
			map_data *md = mdos->map;
			
			strdb_erase(map_db, iter.key());
			delete mdos;

			if( md )
			{	// real mapdata exists
				strdb_insert(map_db, md->mapname, md);
			}
		}
	}
	return 1;
}

/*==========================================
 * 他鯖管理のマップをdbから削除
 *------------------------------------------
 */
int map_eraseipport(const char *name, basics::ipset &mapset)
{
	struct map_data *md;
	struct map_data_other_server *mdos;

	md=(struct map_data *) strdb_search(map_db,name);
	if(md && NULL==md->gat)
	{	// check if map exists and if it is a other_server_map
		mdos=(struct map_data_other_server *)md;
		if(mdos->mapset==mapset)
		{
			md = mdos->map;
			strdb_erase(map_db,name);
			delete mdos;
			if( md )
			{	// saved real mapdata exists, insert it again
				strdb_insert(map_db,md->mapname, md);
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// 初期化周り


/*==========================================
 * 水場高さ設定
 *------------------------------------------
 */
#define NO_WATER 1000000

class CWaterlist
{
	struct s_waterlist
	{
		char mapname[24];
		int waterheight;

		s_waterlist() : waterheight(0)
		{
			mapname[0]=0;
		}
	};
	s_waterlist *cWaterlist;
public:
	CWaterlist() : cWaterlist(NULL)	{}
	CWaterlist(const char *watertxt) : cWaterlist(NULL)
	{
		open(watertxt);
	}
	~CWaterlist()
	{
		close();
	}
	bool open(const char *watertxt)
	{
		char line[1024],w1[1024];
		FILE *fp=NULL;
		int n=0;

		fp=basics::safefopen(watertxt,"r");
		if(fp==NULL){
			ShowError("waterheight file not found: %s\n",watertxt);
			return false;
		}
		if(!cWaterlist)
		{
			cWaterlist=new struct s_waterlist[MAX_MAP_PER_SERVER];
		}
		while( n < MAX_MAP_PER_SERVER && fgets(line,sizeof(line),fp) )
		{
			int wh,count;
			if( !is_valid_line(line) )
				continue;
			if((count=sscanf(line,"%1024s %d",w1,&wh)) < 1)
			{
				continue;
			}
			basics::itrim(w1);
			if(!*w1) continue;

			buffer2mapname(cWaterlist[n].mapname, sizeof(cWaterlist[n].mapname), w1);
			cWaterlist[n].mapname[sizeof(cWaterlist[n].mapname)-1]=0;

			if(count >= 2)
				cWaterlist[n].waterheight = wh;
			else
				cWaterlist[n].waterheight = 3;
			n++;
		}
		fclose(fp);
		return true;
	}
	void close()
	{
		if(cWaterlist)
		{
			delete[] cWaterlist;
			cWaterlist = NULL;
		}
	}

	int map_waterheight(const char *mapname)
	{
		if(cWaterlist)
		{
			int i;
			for(i=0;cWaterlist[i].mapname[0] && i < MAX_MAP_PER_SERVER;++i)
			{
				if(strcmp(cWaterlist[i].mapname,mapname)==0)
					return cWaterlist[i].waterheight;
			}
		}
		return NO_WATER;
	}

	// map_readwaterheight
	// Reads from the .rsw for each map
	// Returns water height (or NO_WATER if file doesn't exist)or other error is encountered.
	// This receives a map-name, and changes the extension to rsw if it isn't set already.
	// assumed path for file is data/mapname.rsw
	// Credits to LittleWolf
	int map_waterheight_from_grf(const char *mapname)
	{
		if(mapname)
		{
			char buf[512];
			char* ip;

			// have windows backslash as path seperator here
			snprintf(buf, sizeof(buf), "data\\%s", mapname);
			ip = strrchr(buf,'.');
			if(ip) *ip=0;
			// append ".rsw" for reading in rsw's
			strcat(buf, ".rsw");

			//Load water height from file
			uchar *dat = grfio_read(buf);
			if(dat)
			{	
				const uchar *p = dat+166;
				float whtemp;
				_F_frombuffer(whtemp, p);
				delete[] dat;
				return (int)whtemp;
			}
		}
		return NO_WATER;
	}

};

CWaterlist waterlist;


/*==========================================
* マップキャッシュに追加する
*===========================================*/
///////////////////////////////////////////////////////////////////////////////
// マップキャッシュの最大値
#define MAX_MAP_CACHE 768

//各マップごとの最小限情報を入れるもの、READ_FROM_BITMAP用
struct map_cache_info
{
	char fn[24];			//ファイル名
	unsigned short xs;
	unsigned short ys;		//幅と高さ
	int water_height;
	size_t pos;				// データが入れてある場所
	int compressed;     // zilb通せるようにする為の予約
	size_t datalen;
}; // 40 byte

struct map_cache_head {
	size_t sizeof_header;
	size_t sizeof_map;
	// 上の２つ改変不可
	size_t nmaps;			// マップの個数
	long filesize;
};

struct map_cache {
	struct map_cache_head head;
	struct map_cache_info *map;
	FILE *fp;
	int dirty;
};

struct map_cache map_cache;

bool map_cache_open(const char *fn);
bool map_cache_close(void);
bool map_cache_read(struct map_data &m);
bool map_cache_write(struct map_data &m);

bool map_cache_open(const char *fn)
{
	if(map_cache.fp)
		map_cache_close();

	map_cache.fp = basics::safefopen(fn,"r+b");
	if(map_cache.fp)
	{
		fread(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fseek(map_cache.fp,0,SEEK_END);
		if(
			map_cache.head.sizeof_header == sizeof(struct map_cache_head) &&
			map_cache.head.sizeof_map    == sizeof(struct map_cache_info) &&
			map_cache.head.filesize      == ftell(map_cache.fp) )
		{
			// キャッシュ読み甲ﾝ成功
			map_cache.map = new struct map_cache_info[map_cache.head.nmaps];
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map, sizeof(struct map_cache_info), map_cache.head.nmaps, map_cache.fp);
			return true;
		}
		fclose(map_cache.fp);
	}
	// 読み甲ﾝに失敗したので新規に作成する
	map_cache.fp = basics::safefopen(fn,"wb");
	if(map_cache.fp)
	{
		memset(&map_cache.head,0,sizeof(struct map_cache_head));
		map_cache.map   = new struct map_cache_info[MAX_MAP_CACHE];
		memset(map_cache.map,0, MAX_MAP_CACHE*sizeof(struct map_cache_info));
		map_cache.head.nmaps         = MAX_MAP_CACHE;
		map_cache.head.sizeof_header = sizeof(struct map_cache_head);
		map_cache.head.sizeof_map    = sizeof(struct map_cache_info);

		map_cache.head.filesize  = sizeof(struct map_cache_head);
		map_cache.head.filesize += sizeof(struct map_cache_info) * map_cache.head.nmaps;

		map_cache.dirty = 1;
		return true;
	}
	return false;
}

bool map_cache_close(void)
{
	if(!map_cache.fp)
		return false;
	if(map_cache.dirty)
	{
		fseek(map_cache.fp,0,SEEK_SET);
		fwrite(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fwrite(map_cache.map,map_cache.head.nmaps,sizeof(struct map_cache_info),map_cache.fp);
	}
	fclose(map_cache.fp);
	map_cache.fp = NULL;
	delete[] map_cache.map;
	map_cache.map=NULL;
	return true;
}

bool map_cache_read(struct map_data &block_list)
{
	size_t i;
	if(!map_cache.fp) { return 0; }
	for(i=0; i<map_cache.head.nmaps; ++i)
	{
		if( map_cache.map[i].water_height == block_list.wh &&	// 水場の高さが違うので読み直し
			0==strcmp(block_list.mapname,map_cache.map[i].fn) )
			break;
	}
	if( i < map_cache.head.nmaps )	
	{
		if(map_cache.map[i].compressed == 0)
		{
				// 非圧縮ファイル
			const size_t dest_sz = map_cache.map[i].xs*map_cache.map[i].ys;
			block_list.xs = map_cache.map[i].xs;
			block_list.ys = map_cache.map[i].ys;
			block_list.gat = new struct mapgat[dest_sz];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if( map_cache.map[i].datalen > dest_sz*sizeof(struct mapgat) )
				map_cache.map[i].datalen = dest_sz*sizeof(struct mapgat);
			if( map_cache.map[i].datalen != fread(block_list.gat, 1, map_cache.map[i].datalen, map_cache.fp) )
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] block_list.gat;
				block_list.gat = NULL;
				block_list.xs = 0;
				block_list.ys = 0;
				return false;
			}
		}
		else if(map_cache.map[i].compressed == 1)
		{	//zlib compressed
				// 圧縮フラグ=1 : zlib
			unsigned char *buf;
			unsigned long size_compress = map_cache.map[i].datalen;
			unsigned long dest_len = map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat);
			block_list.xs = map_cache.map[i].xs;
			block_list.ys = map_cache.map[i].ys;
			buf = new unsigned char[size_compress];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if(fread(buf,1,size_compress,map_cache.fp) != size_compress)
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] buf;
				buf = NULL;
				block_list.gat = NULL;
				block_list.xs = 0; 
				block_list.ys = 0; 
				return false;
			}
			block_list.gat = new struct mapgat[map_cache.map[i].xs * map_cache.map[i].ys];
			decode_zip((unsigned char*)block_list.gat, dest_len, buf, size_compress);
			if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat))
			{	// 正常に解凍が出来てない
				delete[] buf;
				buf=NULL;
				delete[] block_list.gat;
				block_list.gat = NULL;
				block_list.xs = 0; 
				block_list.ys = 0; 
				return false;
			}
			if(buf)
			{	// might be ok without this check
				delete[] buf;
				buf=NULL;
			}
		}
		// might add other compressions here

		// clear dynamic field entries
		const size_t sz = block_list.xs*block_list.ys;
		bool warn=true;
		for(i=0; i<sz; ++i)
		{
			/////////////////////////////////////////////////////////
			//## remove at the latest by 12/2006
			if( warn && config.etc_log &&
				(block_list.gat[i].npc || block_list.gat[i].basilica || block_list.gat[i].moonlit || block_list.gat[i].regen) )
			{
				ShowWarning("Map Cache corrupted, please rebuild it\n");
				warn = false;
			}
			/////////////////////////////////////////////////////////

			block_list.gat[i].npc = 0;
			block_list.gat[i].basilica = 0;
			block_list.gat[i].moonlit = 0;
			block_list.gat[i].regen = 0;
		}
		return true;
	}
	return false;
}

bool map_cache_write(struct map_data &block_list)
{
	size_t i;
	unsigned long len_new, len_old;
	unsigned char *write_buf;

	if(!map_cache.fp)
		return false;

	for(i = 0;i < map_cache.head.nmaps ; ++i) 
	{
		if( (0==strcmp(block_list.mapname,map_cache.map[i].fn)) || (map_cache.map[i].fn[0] == 0) )
			break;
	}
	if(i<map_cache.head.nmaps) 
	{	// should always be valid but better check it
		int compress = 0;
		if(map_read_flag == READ_FROM_BITMAP_COMPRESSED)
			compress = 1;	// zlib compress
							// might add other compressions here

		// prepare write_buf and len_new
		if(compress == 1)
		{	// zlib compress
			// 圧縮保存
			// さすがに２倍に膨れる事はないという事で
			len_new = 2 * block_list.xs * block_list.ys * sizeof(struct mapgat);
			write_buf = new unsigned char[len_new];
			encode_zip(write_buf,len_new,(unsigned char *)block_list.gat, block_list.xs*block_list.ys*sizeof(struct mapgat));
		}
		else
		{	// no compress
			len_new = block_list.xs * block_list.ys *sizeof(struct mapgat);
			write_buf = (unsigned char*)block_list.gat;
		}
		
		// now insert it
		if( (map_cache.map[i].fn[0] == 0) )
		{	// new map is inserted
			// write at the end of the mapcache file
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

			// prepare the data header
			memcpy(map_cache.map[i].fn, block_list.mapname, sizeof(map_cache.map[i].fn));
			map_cache.map[i].fn[sizeof(map_cache.map[i].fn)-1]=0;			

			// update file header
			map_cache.map[i].pos = map_cache.head.filesize;
			map_cache.head.filesize += len_new;
		}
		else
		{	// update an existing map
			len_old = map_cache.map[i].datalen;

			if(len_new <= len_old) // size is ok
			{	// サイズが同じか小さくなったので場所は変わらない
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
	
			}
			else 
			{	// new len is larger then the old space -> write at file end
				// 新しい場所に登録
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

					// update file header
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
		}
		// just make sure that everything gets updated
		map_cache.map[i].xs  = block_list.xs;
		map_cache.map[i].ys  = block_list.ys;
		map_cache.map[i].water_height = block_list.wh;
		map_cache.map[i].compressed   = compress;
		map_cache.map[i].datalen      = len_new;
		map_cache.dirty = 1;

		if(compress == 1)
		{	// zlib compress has alloced an additional buffer
			delete[] write_buf;
			write_buf = NULL;
		}
		return true;
	}
	// 書き甲ﾟなかった
	return false;
}

bool map_readafm(struct map_data& block_list, const char *fn=NULL)
{
	/*
	Advanced Fusion Maps Support
	(c) 2003-2004, The Fusion Project
	- AlexKreuz

	The following code has been provided by me for eAthena
	under the GNU GPL.  It provides Advanced Fusion
	Map, the map format desgined by me for Fusion, support
	for the eAthena emulator.

	I understand that because it is under the GPL
	that other emulators may very well use this code in their
	GNU project as well.

	The AFM map format was not originally a part of the GNU
	GPL. It originated from scratch by my own hand.  I understand
	that distributing this code to read the AFM maps with eAthena
	causes the GPL to apply to this code.  But the actual AFM
	maps are STILL copyrighted to the Fusion Project.  By choosing

	In exchange for that 'act of faith' I ask for the following.

	A) Give credit where it is due.  If you use this code, do not
	   place your name on the changelog.  Credit should be given
	   to AlexKreuz.
	B) As an act of courtesy, ask me and let me know that you are putting
	   AFM support in your project.  You will have my blessings if you do.
	C) Use the code in its entirety INCLUDING the copyright message.
	   Although the code provided may now be GPL, the AFM maps are not
	   and so I ask you to display the copyright message on the STARTUP
	   SCREEN as I have done here. (refer to core.c)
	   "Advanced Fusion Maps (c) 2003-2004 The Fusion Project"

	Without this copyright, you are NOT entitled to bundle or distribute
	the AFM maps at all.  On top of that, your "support" for AFM maps
	becomes just as shady as your "support" for Gravity GRF files.

	The bottom line is this.  I know that there are those of you who
	would like to use this code but aren't going to want to provide the
	proper credit.  I know this because I speak frome experience.  If
	you are one of those people who is going to try to get around my
	requests, then save your breath because I don't want to hear it.

	I have zero faith in GPL and I know and accept that if you choose to
	not display the copyright for the AFMs then there is absolutely nothing
	I can do about it.  I am not about to start a legal battle over something
	this silly.

	Provide the proper credit because you believe in the GPL.  If you choose
	not to and would rather argue about it, consider the GPL failed.

	October 18th, 2004
	- AlexKreuz
	- The Fusion Project
	*/

	int x,y,xs,ys;
	size_t size;

	char afm_line[65535];
	int afm_size[2];
	FILE *afm_file;
	char *str;
	char buf[512];

	if(!fn)
	{
		char *ip;
		if(afm_dir && *afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, block_list.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", block_list.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".afm");
	}
	else
		safestrcpy(buf, sizeof(buf), fn);

	afm_file = basics::safefopen(buf, "r");
	if (afm_file != NULL)
	{
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		xs = block_list.xs = afm_size[0];
		ys = block_list.ys = afm_size[1];

		block_list.npc_num=0;
		block_list.users=0;
		memset(&block_list.flag,0,sizeof(block_list.flag));

		if(config.pk_mode) block_list.flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		block_list.gat = new struct mapgat[block_list.xs*block_list.ys];
		for (y = 0; y < ys; ++y)
		{
			str=fgets(afm_line, sizeof(afm_line), afm_file);
			for (x = 0; x < xs; ++x)
			{
				map_setcell(block_list.m,x,y, str[x] & CELL_MASK );
			}
		}

		block_list.bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
		block_list.bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
		size = block_list.bxs * block_list.bys;

		block_list.objects = new struct map_data::_objects[size];

		strdb_insert(map_db,block_list.mapname,&block_list);

		fclose(afm_file);
		return true;
	}
	return false;
}

bool map_readaf2(struct map_data& block_list, const char*fn=NULL)
{
	FILE *af2_file, *dest;
	char buf[256];
	bool ret=false;

	if(!fn)
	{
		char *ip;
		if(afm_dir && *afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, block_list.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", block_list.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".af2");
	}
	else
		safestrcpy(buf, sizeof(buf), fn);

	af2_file = basics::safefopen(buf, "r");
	if( af2_file != NULL )
	{
		memcpy(buf+strlen(buf)-4, ".out", 5);
		dest = basics::safefopen(buf, "w");
		if (dest == NULL)
		{
			ShowMessage("can't open\n");
			fclose(af2_file);
		}
		else
		{
			ret = 0!=decode_file(af2_file, dest);
			fclose(af2_file);
			fclose(dest);
			if(ret) ret = map_readafm(block_list, buf);
			remove(buf);
		}
	}
	return ret;
}


/*==========================================
 * マップ1枚読み甲ﾝ
 * ===================================================*/
bool map_readgrf(struct map_data& block_list, const char *fn=NULL)
{
	// read from grf
	int x,y;
	struct gat_1cell {float high[4]; uint32 type;};
	unsigned char *gat;
	char buf[512];
	
	if(!fn)
	{	char* ip;
		// have windows backslash as path seperator here
		snprintf(buf, sizeof(buf), "data\\%s", block_list.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		// append ".gat" for reading in grf's
		strcat(buf, ".gat");
	}
	else
	{	
		safestrcpy(buf, sizeof(buf), fn);
		// append ".gat" if not already exist
		if( NULL == strstr(buf,".gat") )
			strcat(buf, ".gat");
	}


	gat = grfio_read(buf);
	if( gat )
	{
		const unsigned char *p = gat+14;
		struct gat_1cell pp;	// make a real structure in memory

		block_list.xs= RBUFL(gat, 6);
		block_list.ys= RBUFL(gat,10);
		block_list.gat = new struct mapgat[block_list.xs*block_list.ys];

//float min=3.40282e+38, max=-3.40282e+38;
		
		for(y=0;y<block_list.ys;++y)
		for(x=0;x<block_list.xs;++x)
		{	// faster and typesafe
			_F_frombuffer(pp.high[0], p);
			_F_frombuffer(pp.high[1], p);
			_F_frombuffer(pp.high[2], p);
			_F_frombuffer(pp.high[3], p);
			_L_frombuffer(pp.type, p);
			// buffer increment is done automatically 
/*
if(pp.type!=1 && pp.type!=5)
{
	if(pp.high[0]<min) min = pp.high[0]; else if(pp.high[0]>max) max = pp.high[0];
	if(pp.high[1]<min) min = pp.high[1]; else if(pp.high[1]>max) max = pp.high[1];
	if(pp.high[2]<min) min = pp.high[2]; else if(pp.high[2]>max) max = pp.high[2];
	if(pp.high[3]<min) min = pp.high[3]; else if(pp.high[3]>max) max = pp.high[3];
}
*/
			if(block_list.wh!=NO_WATER && pp.type==0)
				map_setcell(block_list.m,x,y,(pp.high[0]>block_list.wh || pp.high[1]>block_list.wh || pp.high[2]>block_list.wh || pp.high[3]>block_list.wh) ? 3 : 0);
			else
				map_setcell(block_list.m,x,y,pp.type);
		}
//printf("\n%f, %f\n", min, max);
		delete[] gat;
		return true;
	}
	return false;
}

/*==========================================
 * 全てのmapデ?タを?み?む
 *------------------------------------------
 */




int map_readallmap(void)
{
	size_t i, maps_removed=0;
	bool ch;

	// マップキャッシュを開く
	if(map_read_flag >= READ_FROM_BITMAP)
	{
		map_cache_open(map_cache_file);
	}

	ShowStatus("Loading Maps%s...\n",
		(map_read_flag == READ_FROM_BITMAP_COMPRESSED ? " (w/ Compressed Map Cache)" :
		map_read_flag >= READ_FROM_BITMAP ? " (w/ Map Cache)" :
		map_read_flag == READ_FROM_AFM ? " (w/ AFM)" : ""));

	// 先に全部のャbプの存在を確認
	for(i=0;i<map_num;++i)
	{
		maps[i].wh=waterlist.map_waterheight(maps[i].mapname);
		maps[i].m=i;

		/////////////////////////////////////////////////////////////////
		if( (ch=map_cache_read(maps[i])) ||
			map_readafm(maps[i]) ||
			map_readaf2(maps[i]) ||
			map_readgrf(maps[i]) )
		{	
			ShowMessage("Loading Maps [%d/%d]: %s, size (%d %d)(%i)"CL_CLL"\r", i,map_num, maps[i].mapname, maps[i].xs,maps[i].ys, maps[i].wh);
	
			// initialize
			memset(maps[i].moblist, 0, sizeof(maps[i].moblist));	
			maps[i].mob_delete_timer = -1;	//Initialize timer [Skotlex]

			memset(&maps[i].flag,0,sizeof(maps[i].flag));
			if(config.pk_mode)
				maps[i].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

			maps[i].npc_num=0;
			maps[i].users=0;
			maps[i].bxs= ((maps[i].xs+BLOCK_SIZE-1)/BLOCK_SIZE);
			maps[i].bys= ((maps[i].ys+BLOCK_SIZE-1)/BLOCK_SIZE);
			
			maps[i].objects = new struct map_data::_objects[maps[i].bxs*maps[i].bys];

			strdb_insert(map_db,maps[i].mapname,&maps[i]);

			// cache it
			if(!ch) map_cache_write(maps[i]);
		}
		else
		{
			ShowMessage("Removing Map [%d/%d]: %s"CL_CLL"\r", i,map_num, maps[i].mapname);
			map_delmap(maps[i].mapname);
			maps_removed++;
			i--;
		}
	}

	ShowMessage("\r");
	ShowInfo("Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps.%30s\n",map_num,"");

	map_cache_close();

	if (maps_removed) {
		ShowNotice("Maps Removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);
	}
	return 0;
}

/*==========================================
 * ?み?むmapを追加する
 *------------------------------------------
 */
int map_addmap(const char *mapname)
{
	if (strcasecmp(mapname,"clear")==0) {
		map_num=0;
		return 0;
	}

	if (map_num >= MAX_MAP_PER_SERVER - 1) {
		ShowError("Could not add map '"CL_WHITE"%s"CL_RESET"', the limit of maps has been reached.\n",mapname);
		return 1;
	}
	safestrcpy(maps[map_num].mapname, sizeof(maps[map_num].mapname), mapname);
	char *ip = strchr(maps[map_num].mapname, '.');
	if(ip) *ip=0;

	map_num++;
	return 0;
}

/*==========================================
 * ?み?むmapを削除する
 *------------------------------------------
 */
int map_delmap(const char *mapname)
{
	if (strcasecmp(mapname, "all") == 0)
	{
		map_num = 0;
	}
	else
	{
		size_t i;
		char buffer[32], *ip;
		strcpy(buffer, mapname);
		ip = strchr(buffer, '.');
		if(ip) *ip=0;
		
		for(i=0; i<map_num; ++i)
		{
			if (strcmp(maps[i].mapname, buffer) == 0) {
				ShowMessage("Removing map [ %s ] from maplist"CL_CLL"\n", buffer);
				memmove(maps+i, maps+i+1, sizeof(maps[0])*(map_num-i-1));
				map_num--;
			}
		}
	}
	return 0;
}

/*==========================================
 * Console Command Parser [Wizputer]
 *------------------------------------------
 */
int parse_console(const char *buf)
{
	char type[64], command[64],map[64], buf2[128];
	int x = 0, y = 0;
	int m, n;
	static map_session_data sd(0,0,0,0,0,0,0,0);
	sd.fd = 0;

	safestrcpy(sd.status.name, sizeof(sd.status.name), "console");

	if ( ( n = sscanf(buf, "%64[^:]:%64[^:]:%64s %d %d[^\n]", type , command , map , &x , &y )) < 5 )
		if ( ( n = sscanf(buf, "%64[^:]:%64[^\n]", type , command )) < 2 )
			n = sscanf(buf,"%64[^\n]",type);

	if ( n == 5 )
	{
		if (x <= 0) {
			x = rand() % 399 + 1;
			sd.block_list::x = x;
		} else {
			sd.block_list::x = x;
		}

		if (y <= 0) {
			y = rand() % 399 + 1;
			sd.block_list::y = y;
		} else {
			sd.block_list::y = y;
		}

		m = map_mapname2mapid(map);
		if ( m >= 0 )
			sd.block_list::m = m;
		else {
			ShowConsole(CL_BOLD"Unknown map\n"CL_NORM);
			goto end;
		}
	}

    ShowMessage("Type of command: %s || Command: %s || Map: %s Coords: %d %d\n",type,command,map,x,y);

    if ( strcasecmp("admin",type) == 0 && n == 5 )
	{
		sprintf(buf2,"console: %s",command);
		if( !CommandInfo::is_command(sd.fd, sd, buf2, 99) )
			ShowConsole(CL_BOLD"no valid command\n"CL_NORM);
	}
	else if ( strcasecmp("server",type) == 0 && n == 2 )
	{
		if ( strcasecmp("shutdown", command) == 0 || strcasecmp("exit",command) == 0 || strcasecmp("quit",command) == 0 )
		{
			core_stoprunning();
		}
	}
	else if ( strcasecmp("help",type) == 0 )
	{
		ShowMessage("To use GM commands:\n");
        ShowMessage("admin:<gm command>:<map of \"gm\"> <x> <y>\n");
        ShowMessage("You can use any GM command that doesn't require the GM.\n");
        ShowMessage("No using @item or @warp however you can use @charwarp\n");
        ShowMessage("The <map of \"gm\"> <x> <y> is for commands that need coords of the GM\n");
        ShowMessage("IE: @spawn\n");
        ShowMessage("To shutdown the server:\n");
        ShowMessage("server:shutdown\n");
	}

	end:
	return 0;
}

/*==========================================
 * 設定ファイルを?み?む
 *------------------------------------------
 */
int map_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
//	struct hostent *h = NULL;

	fp = basics::safefopen(cfgName,"r");
	if (fp == NULL) {
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return 0;
	}
	
	while(fgets(line, sizeof(line), fp))
	{
		if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
		{
			basics::itrim(w1);
			if(!*w1) continue;
			basics::itrim(w2);

			if (strcasecmp(w1, "userid")==0)
			{
				chrif_setuserid(w2);
			}
			else if (strcasecmp(w1, "passwd") == 0)
			{
				chrif_setpasswd(w2);
			}
			else if (strcasecmp(w1, "char_ip") == 0)
			{
				getcharaddress() = w2;
				ShowInfo("Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", w2, getcharaddress().tostring(NULL));
			} 
			else if (strcasecmp(w1, "char_port") == 0)
			{
				getcharaddress().port() = atoi(w2);
			}
			else if (strcasecmp(w1, "map_ip") == 0)
			{
				getmapaddress() = w2;
				ShowInfo("Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", w2, getmapaddress().tostring(NULL));
			}
			else if (strcasecmp(w1, "map_port") == 0)
			{
				getmapaddress().LANPort() = atoi(w2);
			}
			else if (strcasecmp(w1, "water_height") == 0)
			{
				waterlist.open(w2);
			}
			else if (strcasecmp(w1, "map") == 0)
			{
				map_addmap(w2);
			}
			else if (strcasecmp(w1, "delmap") == 0)
			{
				map_delmap(w2);
			}
			else if (strcasecmp(w1, "npc") == 0)
			{
				npc_addsrcfile(w2);
			}
			else if (strcasecmp(w1, "path") == 0)
			{
				////////////////////////////////////////
				// add all .txt files recursive from ./npc folder to npc source tree
				basics::findFiles(w2, "*.txt", npc_addsrcfile );
				////////////////////////////////////////
			}
			else if (strcasecmp(w1, "delnpc") == 0)
			{
				npc_delsrcfile(w2);
			}
			else if (strcasecmp(w1, "autosave_time") == 0)
			{
				autosave_interval = atoi(w2) * 1000;
				if (autosave_interval <= 0)
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			}
			else if (strcasecmp(w1, "motd_txt") == 0)
			{
				safestrcpy(motd_txt, sizeof(motd_txt), w2);
			}
			else if (strcasecmp(w1, "help_txt") == 0)
			{
				safestrcpy(help_txt, sizeof(help_txt), w2);
			}
			else if (strcasecmp(w1, "mapreg_txt") == 0)
			{
				safestrcpy(mapreg_txt, sizeof(mapreg_txt), w2);
			}
			else if(strcasecmp(w1,"read_map_from_cache")==0)
			{
				if (atoi(w2) == 2)
					map_read_flag = READ_FROM_BITMAP_COMPRESSED;
				else if (atoi(w2) == 1)
					map_read_flag = READ_FROM_BITMAP;
				else
					map_read_flag = READ_FROM_GAT;
			}
			else if(strcasecmp(w1,"map_cache_file")==0)
			{
				safestrcpy(map_cache_file,sizeof(map_cache_file),w2);
			}
			else if(strcasecmp(w1,"afm_dir") == 0)
			{
				safestrcpy(afm_dir,sizeof(afm_dir), w2);
			}
			else if (strcasecmp(w1, "import") == 0)
			{
				map_config_read(w2);
			}
			else if (strcasecmp(w1, "console") == 0)
			{
				console = basics::config_switch<bool>(w2);
			}
		}
	}
	fclose(fp);
	return 0;
}


int online_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	chrif_char_online_check();
	return 0;
}




class CMapCleanup : public CMapProcessor
{
public:
	CMapCleanup()	{}
	~CMapCleanup()	{}
	virtual int process(block_list& bl) const
	{
		if( bl==BL_PC )
			map_quit( (map_session_data &)bl );
		else if( bl==BL_NPC )
			npc_unload( (npc_data *)&bl );
		else if( bl==BL_MOB )
			mob_unload( (mob_data &)bl);
		else if( bl==BL_PET )
			pet_remove_map( (map_session_data &)bl );
		else if( bl==BL_ITEM )
			map_clearflooritem(bl.id);
		else if( bl==BL_SKILL )
			skill_delunit( (skill_unit *)&bl);
		else
			ShowError("cleanup_sub unhandled block type\n");
		return 0;
	}
};
void map_checknpcsleft(void)
{
	size_t i, m,n=0;

	for(m=0;m<map_num;++m) {
		for(i=0;i<maps[m].npc_num && i<MAX_NPC_PER_MAP;++i) {
			if(maps[m].npc[i]!=NULL) {
				clif_clearchar_area(*maps[m].npc[i],2);
				maps[m].npc[i]->delblock();
				maps[m].npc[i]->deliddb();
				// just unlink npc from maps
				// npc will be deleted with do_final_npc
				maps[m].npc[i] = NULL;
				n++;
			}
		}
	}
	if(n>0)
		ShowWarning("Found '"CL_WHITE"%d"CL_RESET"' stray NPCs on maps.\n", n);
}



/*==========================================
 * map鯖終了・理
 *------------------------------------------
 */
void do_final(void)
{
    size_t i;
	ShowStatus("Terminating...\n");
	///////////////////////////////////////////////////////////////////////////

	grfio_final();
	map_eraseallipport();

	chrif_char_reset_offline();
	chrif_flush_fifo();

	// regular removing
	do_final_npc();
	// additional removing
	for (i = 0; i < map_num; ++i)
	{
		if (maps[i].m >= 0)
			block_list::foreachinarea( CMapCleanup(), i, 0, 0, maps[i].xs-1, maps[i].ys-1, BL_ALL);
	}
	// and a check
	map_checknpcsleft();

	do_final_pc();
	do_final_homun();
	do_final_pet();
	do_final_guild();
	do_final_party();
	do_final_script();
	do_final_itemdb();
	do_final_storage();
	do_final_chrif(); // この内部でキャラを全て切断する


	for (i=0; i<map_num; ++i)
	{
		clear_moblist(i);
		if(maps[i].gat)		{ delete[] maps[i].gat; maps[i].gat=NULL; }
		if(maps[i].objects)	{ delete[] (maps[i].objects); maps[i].objects=NULL; }
	}
	if(block_list::id_db)
	{
		numdb_final(block_list::id_db);
		block_list::id_db=NULL;
	}
	if(map_db)
	{
		strdb_final(map_db);
		map_db=NULL;
	}

	map_session_data::finalize();
	log_final();

	// need this cleaning here since current db is not cleaned on destruction but before
	namereq_db.clean();

	///////////////////////////////////////////////////////////////////////////
	// delete sessions
	for(i = 0; i < fd_max; ++i)
		if(session[i] != NULL) 
			session_Delete(i);
	// clear externaly stored fd's

	///////////////////////////////////////////////////////////////////////////
}


	
/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_helpscreen(int flag)
{	// by MC Cameri
	ShowMessage("Usage: map-server [options]");
	ShowMessage("Options:");
	ShowMessage(CL_WHITE"  Commands\t\t\tDescription"CL_RESET);
	ShowMessage("-----------------------------------------------------------------------------");
	ShowMessage("  --help, --h, --?, /?		Displays this help screen");
	ShowMessage("  --map-config <file>		Load map-server configuration from <file>");
	ShowMessage("  --battle-config <file>	Load battle configuration from <file>");
	ShowMessage("  --atcommand-config <file>	Load atcommand configuration from <file>");
	ShowMessage("  --charcommand-config <file>	Load charcommand configuration from <file>");
	ShowMessage("  --script-config <file>	Load script configuration from <file>");
	ShowMessage("  --msg-config <file>		Load message configuration from <file>");
	ShowMessage("  --grf-path-file <file>	Load grf path file configuration from <file>");
	ShowMessage("  --sql-config <file>		Load inter-server configuration from <file>");
	ShowMessage("				(SQL Only)");
	ShowMessage("  --log-config <file>		Load logging configuration from <file>");
	ShowMessage("				(SQL Only)");
	ShowMessage("  --version, --v, -v, /v	Displays the server's version");
	ShowMessage("\n");
	if (flag) exit(1);
}



/*======================================================
 * Map-Server Init and Command-line Arguments [Valaris]
 *------------------------------------------------------
 */
unsigned char getServerType()
{
	return ATHENA_SERVER_MAP | ATHENA_SERVER_CORE;
}

int do_init(int argc, char *argv[])
{
	int i;
	// just clear all maps
	memset(maps, 0, MAX_MAP_PER_SERVER*sizeof(struct map_data));

	for (i = 1; i < argc ; ++i)
	{
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--?") == 0 || strcmp(argv[i], "/?") == 0)
			map_helpscreen(1);
		else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "/v") == 0)
			display_version(true);
		else if (strcmp(argv[i], "--map_config") == 0 || strcmp(argv[i], "--map-config") == 0)
			MAP_CONF_NAME=argv[++i];
		else if (strcmp(argv[i],"--config") == 0 || strcmp(argv[i],"--battle-config") == 0)
			BATTLE_CONF_FILENAME = argv[++i];
		else if (strcmp(argv[i],"--atcommand_config") == 0 || strcmp(argv[i],"--atcommand-config") == 0)
			ATCOMMAND_CONF_FILENAME = argv[++i];
		else if (strcmp(argv[i],"--charcommand_config") == 0 || strcmp(argv[i],"--charcommand-config") == 0)
			CHARCOMMAND_CONF_FILENAME = argv[++i];
		else if (strcmp(argv[i],"--script_config") == 0 || strcmp(argv[i],"--script-config") == 0)
			SCRIPT_CONF_NAME = argv[++i];
		else if (strcmp(argv[i],"--msg_config") == 0 || strcmp(argv[i],"--msg-config") == 0)
			MSG_CONF_NAME = argv[++i];
		else if (strcmp(argv[i],"--grf_path_file") == 0 || strcmp(argv[i],"--grf-path-file") == 0)
			GRF_PATH_FILENAME = argv[++i];
		else if (strcmp(argv[i],"--log_config") == 0 || strcmp(argv[i],"--log-config") == 0)
			LOG_CONF_NAME = argv[++i];
		else if (strcmp(argv[i],"--run_once") == 0)	// close the map-server as soon as its done.. for testing [Celest]
			core_stoprunning();
	}

	map_config_read(MAP_CONF_NAME);
	basics::CParamBase::loadFile(MAP_CONF_NAME);

	if (SHOW_DEBUG_MSG)
		ShowNotice("Server running in '"CL_WHITE"Debug Mode"CL_RESET"'.\n");
	config.read(BATTLE_CONF_FILENAME);
	msg_txt.read(MSG_CONF_NAME);
	CommandInfo::config_read(ATCOMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);

	log_init(LOG_CONF_NAME);

	block_list::id_db = numdb_init();
	map_db = strdb_init(24);
	map_session_data::initialize();

	grfio_init(GRF_PATH_FILENAME);

	battle_init();
	map_readallmap();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	add_timer_func_list(map_removemobs_timer, "map_removemobs_timer");
	add_timer_interval(gettick()+1000, 60*1000, map_freeblock_timer, 0, 0);

	add_timer_func_list(online_timer, "online_timer");
	add_timer_interval(gettick()+10, CHECK_INTERVAL, online_timer, 0, 0);	

	add_timer_func_list(map_daynight_timer, "map_daynight_timer");

	map_daynight_timer(-1,0,0,config.night_at_start);

	

	do_init_script();
	do_init_itemdb();
	do_init_mob();	// npcの初期化・でmob_spawnして、mob_dbを?照するのでinit_npcより先
	do_init_pc();
	do_init_status();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_skill();
	do_init_pet();
	do_init_homun();
	do_init_npc();
	do_init_clif();
	do_init_chrif();

	npc_event_do_oninit();	// npcのOnInitイベント?行

	if ( console )
	{
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	if (config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	return 0;
}

