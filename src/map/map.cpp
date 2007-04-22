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
#include "mapobj.h"
#include "mapcache.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "flooritem.h"
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

#if defined(WITH_FSMAI)
#include "npcai.h"
#endif

#define USE_AFM
#define USE_AF2



const char *LOG_CONF_NAME="conf/log_athena.conf";
const char *MAP_CONF_NAME = "conf/map_athena.conf";
const char *BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
const char *COMMAND_CONF_FILENAME = "conf/command_athena.conf";
const char *SCRIPT_CONF_NAME = "conf/script_athena.conf";
const char *MSG_CONF_NAME = "conf/msg_athena.conf";
const char *GRF_PATH_FILENAME = "conf/grf-files.txt";

char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";
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
CWaterlist waterlist;


///////////////////////////////////////////////////////////////////////////////
static size_t users=0;

///////////////////////////////////////////////////////////////////////////////
/// 全map鯖?計での接??設定
/// (char鯖から送られてくる)
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

///////////////////////////////////////////////////////////////////////////////
/// 全map鯖?計での接??取得 (/wへの?答用)
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
	ICL_EMPTY_COPYCONSTRUCTOR(CNameStorageBase)
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
			md->remove_slaves();
			// check the mob into the cache
			++md->cache->num;
			// and unload it
			md->freeblock();	
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
	{	// exists and has a locally loaded map
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

			// warp players from this map to the new one
			map_session_data::iterator iter(map_session_data::nickdb());
			for(; iter; ++iter)
			{
				map_session_data*sd = iter.data();
				if( sd && sd->block_list::m == md->m )
					pc_setpos(*sd, sd->mapname, sd->block_list::x, sd->block_list::y, 3);
			}
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
		maps[i].wh=waterlist[maps[i].mapname];
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
	if (fp == NULL)
	{
		ShowError("Map configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
	}
	else
	{
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
		ShowStatus("Done reading Map configuration '"CL_WHITE"%s"CL_RESET"'\n", cfgName);
	}
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
			static_cast<map_session_data &>(bl).map_quit();
		else if( bl==BL_NPC || bl==BL_MOB || bl==BL_PET || bl==BL_ITEM )
			bl.freeblock();
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
	block_list::freeblock_lock();
	for(m=0;m<map_num;++m)
	{
		for(i=0;i<maps[m].npc_num && i<MAX_NPC_PER_MAP;++i)
		{
			if(maps[m].npc[i]!=NULL)
			{
				clif_clearchar_area(*maps[m].npc[i],2);
				maps[m].npc[i]->freeblock();
				maps[m].npc[i] = NULL;
				++n;
			}
		}
		if(n>0)
		{
			ShowWarning("Found '"CL_WHITE"%d"CL_RESET"' stray NPCs on map [%s].\n", n, maps[m].mapname);
			n=0;
		}
	}
	block_list::freeblock_unlock();
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
/////////
// possibly unnecessary
	// additional removing
	for (i = 0; i < map_num; ++i)
	{
		if (maps[i].m >= 0)
		{
			const int ll=block_list::foreachinarea( CMapCleanup(), i, 0, 0, maps[i].xs-1, maps[i].ys-1, BL_ALL);
			if(ll)
				ShowWarning("map cleanup '"CL_WHITE"%d"CL_RESET"' objects.\n", ll);
		}
	}
	// and a check
	map_checknpcsleft();
/////////

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
#if defined(WITH_FSMAI)
	npcai_test();
#endif

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
		else if (strcmp(argv[i],"--command_config") == 0 || strcmp(argv[i],"--command-config") == 0)
			COMMAND_CONF_FILENAME = argv[++i];
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

	config.read(BATTLE_CONF_FILENAME);
	msg_txt.read(MSG_CONF_NAME);
	CommandInfo::config_read(COMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);

	log_init(LOG_CONF_NAME);

	block_list::id_db = numdb_init();
	map_db = strdb_init(24);
	map_session_data::initialize();

	grfio_init(GRF_PATH_FILENAME);

	battle_init();
	map_readallmap();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(flooritem_data::clear_timer, "flooritem_data::clear_timer");
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

	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
		CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_data::event("OnInit"));

	if ( console )
	{
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	if (config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	return 0;
}

