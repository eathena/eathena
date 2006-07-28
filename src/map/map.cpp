// $Id: map.c,v 1.6 2004/09/25 17:37:01 MouseJstr Exp $

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
#include "charcommand.h"
#include "log.h"


char *LOG_CONF_NAME="conf/log_athena.conf";
char *MAP_CONF_NAME = "conf/map_athena.conf";
char *BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
char *ATCOMMAND_CONF_FILENAME = "conf/atcommand_athena.conf";
char *CHARCOMMAND_CONF_FILENAME = "conf/charcommand_athena.conf";
char *SCRIPT_CONF_NAME = "conf/script_athena.conf";
char *MSG_CONF_NAME = "conf/msg_athena.conf";
char *GRF_PATH_FILENAME = "conf/grf-files.txt";

#define USE_AFM
#define USE_AF2

// ‹É—Í static‚Åƒ?ƒJƒ‹‚É?‚ß‚é
static struct dbt * id_db=NULL;
static struct dbt * map_db=NULL;
static struct dbt * nick_db=NULL;
static struct dbt * charid_db=NULL;

static size_t users=0;
static struct block_list *objects[MAX_FLOORITEM];
static int first_free_object_id=0,last_object_id=0;

#define block_free_max 1048576
static struct block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

static char afm_dir[1024] = ""; // [Valaris]

struct map_data maps[MAX_MAP_PER_SERVER];
size_t map_num = 0;

int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int agit_flag = 0;
int night_flag = 0; // 0=day, 1=night [Yor]

//Added for Mugendai's I'm Alive mod
int imalive_on=0;
int imalive_time=60;
//Added by Mugendai for GUI
int flush_on=1;
int flush_time=100;

struct charid2nick
{
	char nick[24];
	uint32 req_id;

	charid2nick(uint32 id) : 
		req_id(id)
	{
		nick[0]=0;
	}
};

// «Ş«Ã«×«­«ã«Ã«·«å××éÄ«Õ«é«°(map_athana.conf?ªÎread_map_from_cacheªÇò¦E)
// 0:××éÄª·ªÊª¤ 1:Şª?õêÜÁğí 2:?õêÜÁğí
int  map_read_flag = READ_FROM_GAT;
char map_cache_file[256]="db/map.info"; // «Ş«Ã«×«­«ã«Ã«·«å«Õ«¡«¤«E£

char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";

char wisp_server_name[24] = "Server"; // can be modified in char-server configuration file

int console = 0;

int CHECK_INTERVAL = 3600000; // [Valaris]

/*==========================================
 * ‘SmapI?Œv‚Å‚ÌÚ??İ’è
 * (charI‚©‚ç‘—‚ç‚ê‚Ä‚­‚é)
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
 * ‘SmapI?Œv‚Å‚ÌÚ??æ“¾ (/w‚Ö‚Ì?“š—p)
 *------------------------------------------
 */
int map_getusers(void)
{
	return users;
}

///////////////////////////////////////////////////////////////////////////////
/// add id to id_db
/// * id_db‚Öbl‚ğ’Ç‰Á
void block_list::map_addiddb() 
{
	numdb_insert(id_db, this->id, this);
}

///////////////////////////////////////////////////////////////////////////////
/// remove id from id_db
/// id_db‚©‚çbl‚ğíœ
void block_list::map_deliddb() 
{
	numdb_erase(id_db,this->id);
}


///////////////////////////////////////////////////////////////////////////////
///
/// block‰»?—
/// maps[]‚Ìblock_list‚©‚ç?‚ª‚Á‚Ä‚¢‚éê‡‚É
/// bl->prev‚Ébl_head‚ÌƒAƒhƒŒƒX‚ğ“ü‚ê‚Ä‚¨‚­
static struct block_list bl_head;

///////////////////////////////////////////////////////////////////////////////
///
/// maps[]‚Ìblock_list‚É’Ç‰Á
/// mob‚Í?‚ª‘½‚¢‚Ì‚Å•ÊƒŠƒXƒg
///
/// ?‚Élink?‚İ‚©‚ÌŠm”F‚ª–³‚¢BŠë?‚©‚à
bool block_list::map_addblock()
{
	if(this->prev != NULL)
	{
		if(config.error_log)
			ShowMessage("map_addblock error : bl->prev!=NULL (id=%lu)\n",(ulong)this->id);
	}
	else if( this->m<map_num && this->x<maps[this->m].xs && this->y<maps[this->m].ys )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_data::_objects &obj = maps[this->m].objects[pos];

		if( this->type == BL_MOB )
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

			if( this->type == BL_PC )
			{
				struct map_session_data& sd = (struct map_session_data&)(*this);
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
/// maps[]‚Ìblock_list‚©‚çŠO‚·
/// prev‚ªNULL‚Ìê‡list‚É?‚ª‚Á‚Ä‚È‚¢
bool block_list::map_delblock()
{
	// ?‚Éblocklist‚©‚ç?‚¯‚Ä‚¢‚é
	if(this->prev==NULL)
	{
		if(this->next!=NULL)
		{	// prev‚ªNULL‚Ånext‚ªNULL‚Å‚È‚¢‚Ì‚Í—L‚Á‚Ä‚Í‚È‚ç‚È‚¢
			if(config.error_log)
				ShowError("map_delblock error : bl->next!=NULL, (type=%i)\n", this->type);
		}
	}
	else if( this->m < map_num )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_data::_objects &obj = maps[this->m].objects[pos];

		if( this->type == BL_PC )
		{
			if( maps[this->m].users>0 && --maps[this->m].users == 0 && config.dynamic_mobs)
				map_removemobs(this->m);
		}

		if(this->type==BL_MOB)
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
			if(this->type==BL_MOB)
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
/// blockíœ‚ÌˆÀ‘S«Šm•Û?—
int block_list::map_freeblock()
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
/// block‚Ìfree‚ğˆêsI‚É‹Ö~‚·‚é
int block_list::map_freeblock_lock (void)
{
	return ++block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// block‚ÌaFree‚ÌƒƒbƒN‚ğ‰ğœ‚·‚é
/// ‚±‚Ì‚Æ‚«AƒƒbƒN‚ªŠ®‘S‚É‚È‚­‚È‚é‚Æ
/// ƒoƒbƒtƒ@‚É‚½‚Ü‚Á‚Ä‚¢‚½block‚ğ‘S•”íœ
int block_list::map_freeblock_unlock (void)
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
		block_free_lock = 0; // Ÿ‰ñˆÈ~‚ÌƒƒbƒN‚Éxá‚ªo‚Ä‚­‚é‚Ì‚ÅƒŠƒZƒbƒg
	}
	return block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// map_freeblock_lock() ‚ğŒÄ‚ñ‚Å map_freeblock_unlock() ‚ğŒÄ‚Î‚È‚¢
/// ŠÖ”‚ª‚ ‚Á‚½‚Ì‚ÅA’èŠú“I‚Éblock_free_lock‚ğƒŠƒZƒbƒg‚·‚é‚æ‚¤‚É‚·‚éB
/// ‚±‚ÌŠÖ”‚ÍAdo_timer() ‚ÌƒgƒbƒvƒŒƒxƒ‹‚©‚çŒÄ‚Î‚ê‚é‚Ì‚ÅA
/// block_free_lock ‚ğ’¼Ú‚¢‚¶‚Á‚Ä‚àxá–³‚¢‚Í‚¸B
int map_freeblock_timer (int tid, unsigned long tick, int id, basics::numptr data)
{
	if (block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		block_list::map_freeblock_unlock();
	}
	return 0;
}





/*==========================================
 * ü?‚ÌPCl?‚ğ?‚¦‚é (unused)
 *------------------------------------------
 */
int map_countnearpc (unsigned short m, int x, int y)
{
	int bx, by, c = 0;
	struct block_list *bl=NULL;

	if( m>MAX_MAP_PER_SERVER || maps[m].users==0 )
		return 0;

	block_list::map_freeblock_lock();

	for(by=y/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;by<=y/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; ++by)
	{
		if (by < 0 || by >= maps[m].bys)
			continue;
		for(bx=x/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1;bx<=x/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; ++bx)
		{
			if (bx < 0 || bx >= maps[m].bxs)
				continue;
			bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
			while(bl)
			{
				if (bl->type == BL_PC)
					c++;
				bl = bl->next;
			}
		}
	}
	block_list::map_freeblock_unlock();
	return c;
}

/*==========================================
 * ƒZƒ‹ã‚ÌPC‚ÆMOB‚Ì?‚ğ?‚¦‚é (ƒOƒ‰ƒ“ƒhƒNƒƒX—p)
 *------------------------------------------
 */
int map_count_oncell(unsigned short m, int x, int y, int type)
{
	int bx,by;
	struct block_list *bl=NULL;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= maps[m].xs) || (y >= maps[m].ys))
		return 0;
	block_list::map_freeblock_lock();

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type == 0 || type != BL_MOB)
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if( bl->x == x && bl->y == y && bl->type == BL_PC )
				count++;
		}
	}
	if (type == 0 || type == BL_MOB)
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if( bl->x == x && bl->y == y )
				count++;
		}
	}
	block_list::map_freeblock_unlock();
	return count;
}
/*
 * «»«E¾ªÎõÌôøªËÌ¸ªÄª±ª¿«¹«­«Eæ«Ë«Ã«ÈªòÚ÷ª¹
 */
struct skill_unit *map_find_skill_unit_oncell(struct block_list &target,int x,int y,unsigned short skill_id,struct skill_unit *out_unit)
{
	int m,bx,by;
	struct block_list *bl;
	int i,c;
	struct skill_unit *unit, *ret=NULL;
	m = target.m;

	if (x < 0 || y < 0 || (x >= maps[m].xs) || (y >= maps[m].ys))
		return NULL;
	block_list::map_freeblock_lock();

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
	c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
	for(i=0;i<c && bl;i++,bl=bl->next)
	{
		if (bl->x != x || bl->y != y || bl->type != BL_SKILL)
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
	block_list::map_freeblock_unlock();
	return ret;
}


//////////////////////////////////////////////////////////////////////////


int CMap::foreachinpath(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int range,int type)
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
	struct block_list *bl;
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

	
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
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
	struct block_list *bl;
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

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
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
	struct block_list *bl;
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
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

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
							if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
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
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;

	return returnCount;

}

int CMap::foreachincell(const CMapProcessor& elem, unsigned short m,int x,int y,int type)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type==0 || type!=BL_MOB)
	{
		bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
		c = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(type && bl && bl->type!=type)
				continue;
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
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
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += elem.process(*bl_list[i]);
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
	return returnCount;
}

int CMap::foreachinmovearea(const CMapProcessor& elem, unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,int type)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);
	if(dx==0 || dy==0){
		// ‹éŒ`—Ìˆæ‚Ìê‡
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
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = maps[m].objects[bx+by*maps[m].bxs].root_mob;
				c  = maps[m].objects[bx+by*maps[m].bxs].cnt_mob;
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}else{
		// Lš—Ìˆæ‚Ìê‡

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
				if(bl && type && bl->type!=type)
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
			for(i=0;i<c && bl;i++,bl=bl->next){
				if(bl && type && bl->type!=type)
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

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)
		{	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			if( bl_list[i]->type == BL_PC && session[((struct map_session_data *) bl_list[i])->fd] == NULL )
				continue;

			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
	return returnCount;
}

int CMap::foreachinarea(const CMapProcessor& elem, unsigned short m, int x0,int y0,int x1,int y1,int type)
{
	int bx,by;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if(m >= map_num )
		return 0;
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é
	
	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;
	if (type == 0 || type != BL_MOB)
		for(by = y0/BLOCK_SIZE; by <= y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx <= x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].objects[bx+by*maps[m].bxs].root_blk;
			c  = maps[m].objects[bx+by*maps[m].bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && type && bl->type!=type)
					continue;
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	if(type==0 || type==BL_MOB)
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

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}


	for(i=blockcount;i<bl_list_count;++i)
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += elem.process(*bl_list[i]);

	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;

	return returnCount;	//[Skotlex]
}
int CMap::foreachpartymemberonmap(const CMapProcessor& elem, struct map_session_data &sd, int type)
{
	int returncount=0;
	struct party *p;
	int x0,y0,x1,y1;
	struct block_list *list[MAX_PARTY];
	size_t i, blockcount=0;
	
	if((p=party_search(sd.status.party_id))==NULL)
		return 0;
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

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
			if(type!=0 &&
				(m->sd->block_list::x<x0 || m->sd->block_list::y<y0 ||
				 m->sd->block_list::x>x1 || m->sd->block_list::y>y1 ) )
				continue;
			list[blockcount++] = m->sd; 
		}
	}

	for(i=0;i<blockcount;++i)
	{
		if(list[i] && list[i]->prev)	// —LŒø‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			returncount += elem.process(*list[i]);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	return returncount;
}
int CMap::foreachobject(const CMapProcessor& elem,int type)
{
	int returncount=0;
	int i;
	int blockcount=bl_list_count;
	block_list::map_freeblock_lock();

	for(i=2;i<=last_object_id;++i)
	{
		if(objects[i])
		{
			if(type && objects[i]->type!=type)
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
	block_list::map_freeblock_unlock();
	bl_list_count = blockcount;
	return returncount;
}



// “¯‚¶ƒ}ƒbƒv‚Ìƒp[ƒeƒBƒƒ“ƒo[‘S‘Ì‚Éˆ—‚ğ‚©‚¯‚é
// type==0 “¯‚¶ƒ}ƒbƒv
//     !=0 ‰æ–Ê“à
/*
void party_foreachsamemap(int (*func)(struct block_list&,va_list &), struct map_session_data &sd, int type,...)
{
	struct party *p;
	int x0,y0,x1,y1;
	struct block_list *list[MAX_PARTY];
	size_t i, blockcount=0;
	
	if((p=party_search(sd.status.party_id))==NULL)
		return;

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
			if(type!=0 &&
				(m->sd->block_list::x<x0 || m->sd->block_list::y<y0 ||
				 m->sd->block_list::x>x1 || m->sd->block_list::y>y1 ) )
				continue;
			list[blockcount++]=&m->sd->bl; 
		}
	}

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é
	for(i=0;i<blockcount;++i)
	{
		if(list[i] && list[i]->prev)	// —LŒø‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			func(*list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

}
*/
/*==========================================
 * map m (x0,y0)-(x1,y1)?‚Ì‘Sobj‚É?‚µ‚Ä
 * func‚ğŒÄ‚Ô
 * type!=0 ‚È‚ç‚»‚Ìí—Ş‚Ì‚İ
 *------------------------------------------
 */
/*
int map_foreachinarea(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int type,...)
{
	
	int bx,by;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if(m >= map_num)
		return 0;
	
	if(x0>x1) swap(x0,x1);
	if(y0>y1) swap(y0,y1);

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= maps[m].xs) x1 = maps[m].xs-1;
	if (y1 >= maps[m].ys) y1 = maps[m].ys-1;
	if (type == 0 || type != BL_MOB)
		for(by = y0/BLOCK_SIZE; by <= y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx <= x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].block[bx+by*maps[m].bxs];
			c = maps[m].block_count[bx+by*maps[m].bxs];
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && type && bl->type!=type)
					continue;
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	if(type==0 || type==BL_MOB)
		for(by = y0/BLOCK_SIZE; by <= y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx <= x1/BLOCK_SIZE; ++bx)
		{
			bl = maps[m].block_mob[bx+by*maps[m].bxs];
			c = maps[m].block_mob_count[bx+by*maps[m].bxs];
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			returnCount += func(*bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}
*/
/*==========================================
 * ‹éŒ`(x0,y0)-(x1,y1)‚ª(dx,dy)ˆÚ“®‚µ‚½bÌ
 * —ÌˆæŠO‚É‚È‚é—Ìˆæ(‹éŒ`‚©LšŒ`)?‚Ìobj‚É
 * ?‚µ‚Äfunc‚ğŒÄ‚Ô
 *
 * dx,dy‚Í-1,0,1‚Ì‚İ‚Æ‚·‚éi‚Ç‚ñ‚È’l‚Å‚à‚¢‚¢‚Á‚Û‚¢Hj
 *------------------------------------------
 */
/*
int map_foreachinmovearea(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if(dx==0 || dy==0){
		// ‹éŒ`—Ìˆæ‚Ìê‡
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
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx){
				bl = maps[m].block[bx+by*maps[m].bxs];
				c = maps[m].block_count[bx+by*maps[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = maps[m].block_mob[bx+by*maps[m].bxs];
				c = maps[m].block_mob_count[bx+by*maps[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}else{
		// Lš—Ìˆæ‚Ìê‡

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=maps[m].xs) x1=maps[m].xs-1;
		if(y1>=maps[m].ys) y1=maps[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx){
				bl = maps[m].block[bx+by*maps[m].bxs];
				c = maps[m].block_count[bx+by*maps[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
				bl = maps[m].block_mob[bx+by*maps[m].bxs];
				c = maps[m].block_mob_count[bx+by*maps[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
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
	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(config.error_log)
			ShowMessage("map_foreachinarea: *WARNING* block count too many!\n");
	}

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)
		{	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			if( bl_list[i]->type == BL_PC && session[((struct map_session_data *) bl_list[i])->fd] == NULL )
				continue;

			va_list ap;
			va_start(ap,type);
			returnCount += func(*bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
	return returnCount;
}
*/
// -- moonsoul	(added map_foreachincell which is a rework of map_foreachinarea but
//			 which only checks the exact single x/y passed to it rather than an
//			 area radius - may be more useful in some instances)
//
/*
int map_foreachincell(int (*func)(struct block_list&,va_list &),unsigned short m,int x,int y,int type,...)
{
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type==0 || type!=BL_MOB)
	{
		bl = maps[m].block[bx+by*maps[m].bxs];
		c = maps[m].block_count[bx+by*maps[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(type && bl && bl->type!=type)
				continue;
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(type==0 || type==BL_MOB)
	{
		bl = maps[m].block_mob[bx+by*maps[m].bxs];
		c = maps[m].block_mob_count[bx+by*maps[m].bxs];
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

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			returnCount += func(*bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
	return returnCount;
}
*/
/*============================================================
* For checking a path between two points (x0, y0) and (x1, y1)
*------------------------------------------------------------
 */
/*
int map_foreachinpath(int (*func)(struct block_list&,va_list &),unsigned short m,int x0,int y0,int x1,int y1,int range,int type,...)
{
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
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
	struct block_list *bl;
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

	
	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			returnCount += func(bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
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
	struct block_list *bl;
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

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			returnCount += func(bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;
*/
/*
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
	struct block_list *bl;
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

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
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
				c1  = maps[m].block_count[bx+by*maps[m].bxs];		// number of elements in the block
				c2  = maps[m].block_mob_count[bx+by*maps[m].bxs];	// number of mobs in the mob block
				if( (c1!=0) || (c2!=0) )						// skip if nothing in the block
				{

					if(type==0 || type!=BL_MOB) {
						bl = maps[m].block[bx+by*maps[m].bxs];		// a block with the elements
						for(i=0;i<c1 && bl;i++,bl=bl->next){		// go through all elements
							if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
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

					if(type==0 || type==BL_MOB) {
						bl = maps[m].block_mob[bx+by*maps[m].bxs];	// and the mob block
						for(i=0;i<c2 && bl;i++,bl=bl->next){
							if(bl && bl_list_count<BL_LIST_MAX) {
								// check if mob xy is on the line
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

	block_list::map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i] && bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
		{
			va_list ap;
			va_start(ap,type);
			returnCount += func(*bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	bl_list_count = blockcount;

	return returnCount;
}
*/
/*==========================================
 * ‘SˆêObj‘Šè‚Éfunc‚ğŒÄ‚Ô
 *
 *------------------------------------------
 */
/*
void map_foreachobject(int (*func)(struct block_list*,va_list &),int type,...)
{
	int i;
	int blockcount=bl_list_count;

	for(i=2;i<=last_object_id;++i)
	{
		if(objects[i])
		{
			if(type && objects[i]->type!=type)
				continue;
			if(bl_list_count>=BL_LIST_MAX)
			{
				if(config.error_log)
					ShowMessage("map_foreachobject: too many block !\n");
			}
			else
				bl_list[bl_list_count++]=objects[i];
		}
	}

	block_list::map_freeblock_lock();
	for(i=blockcount;i<bl_list_count;++i)
	{
		if( bl_list[i]->prev || bl_list[i]->next )
		{
			va_list ap;
			va_start(ap,type);
			func(bl_list[i],ap);
			va_end(ap);
		}
	}
	block_list::map_freeblock_unlock();
	bl_list_count = blockcount;
}
*/
/*==========================================
 * °ƒAƒCƒeƒ€‚âƒGƒtƒFƒNƒg—p‚ÌˆêObjŠ„‚è?‚Ä
 * object[]‚Ö‚Ì•Û‘¶‚Æid_db“o?‚Ü‚Å
 *
 * bl->id‚à‚±‚Ì’†‚Åİ’è‚µ‚Ä–â‘è–³‚¢?
 *------------------------------------------
 */
int map_addobject(struct block_list &bl)
{
	int i;
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM;++i)
		if(objects[i]==NULL)
			break;
	if(i>=MAX_FLOORITEM){
		if(config.error_log)
			ShowMessage("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	objects[i]=&bl;
	numdb_insert(id_db,i,&bl);
	return i;
}

/*==========================================
 * ˆêObject‚Ì‰ğ•ú
 *	map_delobject‚ÌaFree‚µ‚È‚¢ƒo?ƒWƒ‡ƒ“
 *------------------------------------------
 */
int map_delobjectnofree(int id)
{
	if(objects[id]==NULL)
		return 0;

	objects[id]->map_delblock();
	numdb_erase(id_db,id);
	objects[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && objects[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * ˆêObject‚Ì‰ğ•ú
 * block_list‚©‚ç‚ÌíœAid_db‚©‚ç‚Ìíœ
 * object data‚ÌaFreeAobject[]‚Ö‚ÌNULL‘ã“ü
 *
 * add‚Æ‚Ì??«‚ª–³‚¢‚Ì‚ª?‚É‚È‚é
 *------------------------------------------
 */
int map_delobject(int id)
{
	struct block_list *obj = objects[id];

	if(obj==NULL)
		return 0;

	map_delobjectnofree(id);
	obj->map_freeblock();

	return 0;
}

/*==========================================
 * °ƒAƒCƒeƒ€‚ğÁ‚·
 *
 * data==0‚ÌbÍtimer‚ÅÁ‚¦‚½ê * data!=0‚ÌbÍE‚¤“™‚ÅÁ‚¦‚½bÆ‚µ‚Ä“®ì
 *
 * ŒãÒ‚ÍAmap_clearflooritem(id)‚Ö
 * map.h?‚Å#define‚µ‚Ä‚ ‚é
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	flooritem_data *fitem = (flooritem_data *)objects[id];
	if(fitem==NULL || fitem->block_list::type!=BL_ITEM || (!data.num && fitem->cleartimer != tid)){
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
 * (m,x,y)‚Ìü?rangeƒ}ƒX?‚Ì‹ó‚«(=N“ü‰Â”\)cell‚Ì
 * ?‚©‚ç“K?‚Èƒ}ƒX–Ú‚ÌÀ•W‚ğx+(y<<16)‚Å•Ô‚·
 *
 * Œ»?range=1‚ÅƒAƒCƒeƒ€ƒhƒƒbƒv—p“r‚Ì‚İ
 *------------------------------------------
 */
int map_searchrandfreecell(unsigned short m, unsigned short x, unsigned short y, unsigned short range)
{
	int free_cell,i,j;

	if (range > 15)
		return -1;
	
	CREATE_BUFFER(free_cells, int, (2*range+1)*(2*range+1));

	for(free_cell=0,i=-range;i<=range;++i){
		if(i+y<0 || i+y>=maps[m].ys)
			continue;
		for(j=-range;j<=range; ++j){
			if(j+x<0 || j+x>=maps[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(map_count_oncell(m,j+x,i+y, BL_ITEM) > 1) //Avoid item stacking to prevent against exploits. [Skotlex]
				continue;
			free_cells[free_cell++] = j+x+((i+y)<<16);
		}
	}
	free_cell = (free_cell==0)?-1:free_cells[rand()%free_cell];
	DELETE_BUFFER(free_cells);
	return free_cell;
/*
	int free_cell,i,j;

	if(m<map_num)
	{
		for(free_cell=0,i=-range;i<=range;++i){
			if(i+y<0 || i+y>=maps[m].ys)
				continue;
			for(j=-range;j<=range; ++j){
				if(j+x<0 || j+x>=maps[m].xs)
					continue;
				if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
					continue;
				free_cell++;
			}
		}
		if(free_cell==0)
			return -1;
		free_cell=rand()%free_cell;
		for(i=-range;i<=range;++i){
			if(i+y<0 || i+y>=maps[m].ys)
				continue;
			for(j=-range;j<=range; ++j){
				if(j+x<0 || j+x>=maps[m].xs)
					continue;
				if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
					continue;
				if(free_cell==0){
					x+=j;
					y+=i;
					i=range+1;
					break;
				}
				free_cell--;
			}
		}
	}
	return x+(y<<16);
*/
}

/*==========================================
 * (m,x,y)‚ğ’†S‚É3x3ˆÈ?‚É°ƒAƒCƒeƒ€İ’u
 *
 * item_data‚ÍamountˆÈŠO‚ğcopy‚·‚é
 *------------------------------------------
 */
int map_addflooritem(struct item &item_data,unsigned short amount,unsigned short m,unsigned short x,unsigned short y,
					 struct map_session_data *first_sd,struct map_session_data *second_sd,struct map_session_data *third_sd,int type)
{
	int xy,r;
	unsigned long tick;
	flooritem_data *fitem=NULL;

	if((xy=map_searchrandfreecell(m,x,y,2))<0)
		return 0;
	r=rand();

	fitem = new flooritem_data;
	fitem->block_list::type=BL_ITEM;
	fitem->block_list::prev = fitem->block_list::next = NULL;
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

	memcpy(&fitem->item_data,&item_data,sizeof(struct item));
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=add_timer(gettick()+config.flooritem_lifetime,map_clearflooritem_timer,fitem->block_list::id,0);

	fitem->map_addblock();
	clif_dropflooritem(*fitem);

	return fitem->block_list::id;
}

/*==========================================
 * charid_db‚Ö’Ç‰Á(•ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM)
 *------------------------------------------
 */
void map_addchariddb(uint32 charid, const char *name)
{
	struct charid2nick *p;
	int req = 0;

	p = (struct charid2nick*)numdb_search(charid_db,charid);
	if (p == NULL)
	{	// not in database -> create new
		p = new struct charid2nick(0);
	}
	else
	{	// in database, remove to be reinserted afterwards
		//## propably not necessary, 
		// either way, the whole system does not work for multiple requests on the same charid
		numdb_erase(charid_db, charid);
		req = p->req_id;
	}
	p->req_id = 0;
	memcpy(p->nick, name, 24);
	p->nick[23]=0;
	numdb_insert(charid_db, charid, p);

	if (req)
	{	// •ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM
		struct map_session_data *sd = map_id2sd(req);
		if (sd != NULL)
			clif_solved_charname(*sd,charid);
	}
}

/*==========================================
 * charid_db‚Ö’Ç‰Ái•ÔM—v‹‚Ì‚İj
 *------------------------------------------
 */
int map_reqchariddb(struct map_session_data &sd, uint32 charid) 
{
	struct charid2nick *p= (struct charid2nick*)numdb_search(charid_db,charid);
	if(p==NULL)
	{	// not in database -> create new
		p = new struct charid2nick(sd.block_list::id);
		numdb_insert(charid_db,charid,p);
	}
	return 0;
}



/*==========================================
 * nick_db‚Ösd‚ğ’Ç‰Á
 *------------------------------------------
 */
void map_addnickdb(struct map_session_data &sd) 
{
	strdb_insert(nick_db,sd.status.name,&sd);
}

/*==========================================
 * PC‚Ìquit?— map.c?•ª
 *
 * quit?—‚Ìå?‚ªˆá‚¤‚æ‚¤‚È?‚à‚µ‚Ä‚«‚½
 *------------------------------------------
 */
int map_quit(struct map_session_data &sd) 
{
	if( sd.state.event_disconnect )
	{
		if( script_config.event_script_type == 0 )
		{
			struct npc_data *npc = npc_name2id(script_config.logout_event_name);
			if( npc && npc->u.scr.ref && (npc->block_list::m==0xFFFF || npc->block_list::m==sd.block_list::m) )
			{
				CScriptEngine::run(npc->u.scr.ref->script,0,sd.block_list::id,npc->block_list::id); // PCLogoutNPC
				ShowStatus ("Event '"CL_WHITE"%s"CL_RESET"' executed.\n", script_config.logout_event_name);
			}
		}
		else
		{
			int evt = npc_event_doall_id("OnPCLogoutEvent", sd.block_list::id, sd.block_list::m);
			if(evt) ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n", evt, "OnPCLogoutEvent");
		}
	}

	if(sd.chatID)	// ƒ`ƒƒƒbƒg‚©‚ço‚é
		chat_leavechat(sd);
	if(sd.trade_partner)	// æˆø‚ğ’†?‚·‚é	
		trade_tradecancel(sd);
	if(sd.party_invite>0)	// ƒp?ƒeƒB?—U‚ğ‹‘”Û‚·‚é
		party_reply_invite(sd,sd.party_invite_account,0);
	if(sd.guild_invite>0)	// ƒMƒ‹ƒh?—U‚ğ‹‘”Û‚·‚é
		guild_reply_invite(sd,sd.guild_invite,0);
	if(sd.guild_alliance>0)	// ƒMƒ‹ƒh“¯–¿?—U‚ğ‹‘”Û‚·‚é
		guild_reply_reqalliance(sd,sd.guild_alliance_account,0);

	party_send_logout(sd);	// ƒp?ƒeƒB‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M
	party_send_dot_remove(sd);
	guild_send_memberinfoshort(sd,0);	// ƒMƒ‹ƒh‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M

	chrif_save_sc(sd);

	pc_cleareventtimer(sd);	// ƒCƒxƒ“ƒgƒ^ƒCƒ}‚ğ”jŠü‚·‚é
	if(sd.state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);	// ‘qŒÉ‚ğŠJ‚¢‚Ä‚é‚È‚ç•Û‘¶‚·‚é

	// check if we've been authenticated [celest]
	if (sd.state.auth)
		skill_castcancel(&sd,0);	// ‰r¥‚ğ’†?‚·‚é

	skill_stop_dancing(&sd,1);// ƒ_ƒ“ƒX/‰‰‘t’†?
	if(sd.sc_data && sd.sc_data[SC_BERSERK].timer!=-1) //ƒo?ƒT?ƒN’†‚ÌI—¹‚ÍHP‚ğ100‚É
	{
		sd.status.hp = 100;
		status_change_end(&sd,SC_BERSERK,-1);
	}

	status_change_clear(&sd,1);	// ƒXƒe?ƒ^ƒXˆÙí‚ğ‰ğœ‚·‚é
	skill_clear_unitgroup(&sd);	// ƒXƒLƒ‹ƒ†ƒjƒbƒgƒOƒ‹?ƒv‚Ìíœ
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
		if(sd.pet.intimate <= 0)
		{
			intif_delete_petdata(sd.status.pet_id);
			sd.status.pet_id = 0;
			sd.pd = NULL;
		}
		else
			intif_save_petdata(sd.status.account_id,sd.pet);
	}
	if(pc_isdead(sd))
		pc_setrestartvalue(sd,2);

	homun_data::clear_homunculus(sd);
	pc_clean_skilltree(sd);
	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_dirty(sd);
	storage_storage_save(sd);
	sd.map_delblock();

	
	sd.ScriptEngine.temporaty_close(); //!! calling the destructor here directly since still c-style allocation

	chrif_char_offline(sd);


	struct charid2nick *p = (struct charid2nick *)numdb_search(charid_db,sd.status.char_id);
	if(p)
	{
		numdb_erase(charid_db,sd.status.char_id);
		delete p;
	}

	strdb_erase(nick_db,sd.status.name);
	numdb_erase(charid_db,sd.status.char_id);
	numdb_erase(id_db,sd.block_list::id);
		
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
 * id”Ô?‚ÌPC‚ğ’T‚·B‹‚È‚¯‚ê‚ÎNULL
 *------------------------------------------
 */
struct map_session_data * map_id2sd(uint32 id)
{
// remove search from db, because:
// 1 - all players, npc, items and mob are in this db (to search, it's not speed, and search in session is more sure)
// 2 - DB seems not always correct. Sometimes, when a player disconnects, its id (account value) is not removed and structure
//     point to a memory area that is not more a session_data and value are incorrect (or out of available memory) -> crash
// replaced by searching in all session.
// by searching in session, we are sure that fd, session, and account exist.

// this explanation is not consistent at all
// db search is O(lg(N)), session search is O(M) 
// with N == object count and M == player count
// so, for ~50000 map objects it takes fd_max >= 16 (usually less than 8 players)
// to have the for-loop performing slower than the db search
//
// for the crash argument:
// if the id's are not cleared correctly it will crash in any case,
// when not in this search than in some other
//
// anyway to enable the mapsearch, all db-insert/erase operations need to be restructured
// so I leave this loop as it is until this part is done
/*
	if(id) // skip if zero id's are searched
	{
		struct block_list *bl = (struct block_list *)numdb_search(id_db,id);
		struct map_session_data *sd;
		if(bl && (sd=bl->get_sd()) && session_isActive(sd->fd) )
			return sd;
	}
	return NULL;
*/

	size_t i;
	struct map_session_data *sd;

	if(id) // skip if zero id's are searched
	for(i = 0; i < fd_max; ++i)
		if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) && sd->block_list::id == id)
			return sd;
	return NULL;

}

/*==========================================
 * char_id”Ô?‚Ì–¼‘O‚ğ’T‚·
 *------------------------------------------
 */
char * map_charid2nick(uint32 id)
{
	struct charid2nick *p = (struct charid2nick*)numdb_search(charid_db,id);

	if(p==NULL)
		return NULL;
	if(p->req_id!=0)
		return NULL;
	return p->nick;
}

struct map_session_data * map_charid2sd(uint32 id)
{
	size_t i;
	struct map_session_data *sd;

	if (id <= 0) return 0;

	for(i = 0; i < fd_max; ++i)
		if (session[i] && (sd = (struct map_session_data*)session[i]->user_session) && sd->status.char_id == id)
			return sd;

	return NULL;
}

/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or NULL
 *------------------------------------------
 */
struct map_session_data * map_nick2sd(const char *nick) {
	size_t i;
	int quantity=0, nicklen;
	struct map_session_data *sd = NULL;
	struct map_session_data *pl_sd = NULL;

	if (nick == NULL)
		return NULL;

    nicklen = strlen(nick);

	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data*)session[i]->user_session) && pl_sd->state.auth)
			// Without case sensitive check (increase the number of similar character names found)
			if (strncasecmp(pl_sd->status.name, nick, nicklen) == 0) {
				// Strict comparison (if found, we finish the function immediatly with correct value)
				if (strcmp(pl_sd->status.name, nick) == 0)
					return pl_sd;
				quantity++;
				sd = pl_sd;
			}
	}
	// Here, the exact character name is not found
	// We return the found index of a similar account ONLY if there is 1 similar character
	if (quantity == 1)
		return sd;

	// Exact character name is not found and 0 or more than 1 similar characters have been found ==> we say not found
	return NULL;
}

/*==========================================
 * id”Ô?‚Ì•¨‚ğ’T‚·
 * ˆêObject‚Ìê‡‚Í”z—ñ‚ğˆø‚­‚Ì‚İ
 *------------------------------------------
 */
struct block_list * map_id2bl(uint32 id)
{
	struct block_list *bl=NULL;
	if((size_t)id<sizeof(objects)/sizeof(objects[0]))
		bl = objects[id];
	else
		bl = (struct block_list*)numdb_search(id_db,id);

	return bl;
}

/*==========================================
 * id_db?‚Ì‘S‚Ä‚Éfunc‚ğ?s
 *------------------------------------------
 */
dbt* get_iddb()	{ return id_db; }

/*==========================================
 * map.npc‚Ö’Ç‰Á (warp“™‚Ì—Ìˆæ‚¿‚Ì‚İ)
 *------------------------------------------
 */
int map_addnpc(unsigned short m, struct npc_data *nd)
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
	numdb_insert(id_db,nd->block_list::id,nd);

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
/*
int mob_cache_cleanup_sub(struct block_list &bl, va_list &ap)
{
	struct mob_data &md = (struct mob_data &)bl;
	
	if( bl.type!= BL_MOB )
		return 0;

	//When not to remove:
	//1: Mob is not from a cache
	//2: Mob is damaged 

	// not cached, delayed or already on delete schedule
	if( !md.cache || md.cache->delay1 || md.cache->delay2 || md.deletetimer != -1)
		return 0;
	
	// hurt enemies	
	if ( (md.hp != md.max_hp) && !config.mob_remove_damaged )
		return 0;

	// cleaning a master will also clean its slaves
	if( md.state.is_master )
		mob_deleteslave(md);

	// check the mob into the cache
	md.cache->num++;
	// and unload it
	mob_unload(md);	
	return 1;
}
*/
class CMapMobCacheCleanup : public CMapProcessor
{
public:
	CMapMobCacheCleanup()	{}
	~CMapMobCacheCleanup()	{}
	virtual int process(struct block_list& bl) const
	{
		struct mob_data &md = (struct mob_data &)bl;
		//When not to remove:
		//1: Mob is not from a cache
		//2: Mob is damaged 

		if( bl.type==BL_MOB &&
			// cached, not delayed and not already on delete schedule
			(md.cache && !md.cache->delay1 && !md.cache->delay2 && -1==md.deletetimer) &&
			// unhurt enemies	
			(config.mob_remove_damaged || (md.hp == md.max_hp)) )
		{
			// cleaning a master will also clean its slaves
			if( md.state.is_master )
				mob_deleteslave(md);
			// check the mob into the cache
			md.cache->num++;
			// and unload it
			mob_unload(md);	
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

	k = CMap::foreachinarea( CMapMobCacheCleanup(),
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
 * map–¼‚©‚çmap”Ô?‚Ö?Š·
 *------------------------------------------
 */
int map_mapname2mapid(const char *name)
{
	struct map_data *md=NULL;
	md = (struct map_data*)strdb_search(map_db,name);
	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * ‘¼Imap–¼‚©‚çip,port?Š·
 *------------------------------------------
 */
bool map_mapname2ipport(const char *name, basics::ipset &mapset)
{
	struct map_data_other_server *mdos=NULL;

	mdos = (struct map_data_other_server*)strdb_search(map_db, name);
	if(mdos==NULL || mdos->gat)
		return false;
	mapset = mdos->mapset;
	return true;
}




// gatŒn
/*==========================================
 * (m,x,y)‚Ìó‘Ô‚ğ’²‚×‚é
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
 * (m,x,y)‚Ìó‘Ô‚ğİ’è‚·‚é
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
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğdb‚É’Ç‰Á
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
			// “Ç‚İbñ‚Å‚¢‚½‚¯‚ÇA’S“–ŠO‚É‚È‚Á‚½ƒ}ƒbƒv
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
			// “Ç‚İbñ‚Å‚¢‚ÄA’S“–‚É‚È‚Á‚½ƒ}ƒbƒvi‰½‚à‚µ‚È‚¢j
			;
		}
	}
	else
	{	// exists and is an other_server_map
		mdos=(struct map_data_other_server *)md;
		if( mapset == getmapaddress() )
		{	// the current server is claiming the map
			// ©•ª‚Ì’S“–‚É‚È‚Á‚½ƒ}ƒbƒv

			// reclaim the real mapdata
			// “Ç‚İbñ‚Å‚¢‚é‚Ì‚Å’u‚«Š·‚¦‚é
			md = mdos->map;
			delete mdos;
			
			if(md)
				strdb_insert(map_db,md->mapname,md);
			else
				ShowError("map_setipport error: data for local map %s does not exist.\n",name);
		}
		else
		{	// the ip of the hostig mapserver has changed
			// ‘¼‚ÌI‚Ì’S“–ƒ}ƒbƒv‚È‚Ì‚Å’u‚«Š·‚¦‚é‚¾‚¯
			mdos->mapset=mapset;
		}
	}
	return 0;
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğ‘S‚Äíœ
 *------------------------------------------
 */
/*
int map_eraseallipport_sub(void *key,void *data,va_list &va)
{
	struct map_data_other_server *mdos = (struct map_data_other_server*)data;
	if(mdos->gat == NULL)
	{
		struct map_data *md = mdos->map;
		strdb_erase(map_db,key);
		aFree(mdos);
		if( md )
		{	// real mapdata exists
			strdb_insert(map_db,md->mapname, md);
		}
	}
	return 0;
}
*/
class CDBMapEraseallipport : public CDBProcessor
{
public:
	CDBMapEraseallipport()	{}
	virtual ~CDBMapEraseallipport()	{}
	virtual bool process(void *key, void *data) const
	{
		struct map_data_other_server *mdos = (struct map_data_other_server*)data;
		if(mdos && mdos->gat == NULL)
		{
			struct map_data *md = mdos->map;
			strdb_erase(map_db,key);
			delete mdos;
			if( md )
			{	// real mapdata exists
				strdb_insert(map_db,md->mapname, md);
			}
		}
		return true;
	}
};

int map_eraseallipport(void)
{
	strdb_foreach(map_db, CDBMapEraseallipport() );
//	strdb_foreach(map_db,map_eraseallipport_sub);
	return 1;
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğdb‚©‚çíœ
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
// ‰Šú‰»ü‚è


/*==========================================
 * …ê‚‚³İ’è
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
			if( !get_prepared_line(line) )
				continue;
			if((count=sscanf(line,"%s%d",w1,&wh)) < 1){
				continue;
			}
			char*ip=strchr(w1,'.');
			if(ip) *ip=0;
			memcpy(cWaterlist[n].mapname, w1, sizeof(cWaterlist[n].mapname));
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
			uchar *dat = (uchar *)grfio_read(buf);
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
* ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚É’Ç‰Á‚·‚é
*===========================================*/
///////////////////////////////////////////////////////////////////////////////
// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ÌÅ‘å’l
#define MAX_MAP_CACHE 768

//Šeƒ}ƒbƒv‚²‚Æ‚ÌÅ¬ŒÀî•ñ‚ğ“ü‚ê‚é‚à‚ÌAREAD_FROM_BITMAP—p
struct map_cache_info
{
	char fn[24];			//ƒtƒ@ƒCƒ‹–¼
	unsigned short xs;
	unsigned short ys;		//•‚Æ‚‚³
	int water_height;
	size_t pos;				// ƒf[ƒ^‚ª“ü‚ê‚Ä‚ ‚éêŠ
	int compressed;     // zilb’Ê‚¹‚é‚æ‚¤‚É‚·‚éˆ×‚Ì—\–ñ
	size_t datalen;
}; // 40 byte

struct map_cache_head {
	size_t sizeof_header;
	size_t sizeof_map;
	// ã‚Ì‚Q‚Â‰ü•Ï•s‰Â
	size_t nmaps;			// ƒ}ƒbƒv‚ÌŒÂ”
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
			// ƒLƒƒƒbƒVƒ…“Ç‚İbİ¬Œ÷
			map_cache.map = new struct map_cache_info[map_cache.head.nmaps];
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map, sizeof(struct map_cache_info), map_cache.head.nmaps, map_cache.fp);
			return true;
		}
		fclose(map_cache.fp);
	}
	// “Ç‚İbİ‚É¸”s‚µ‚½‚Ì‚ÅV‹K‚Éì¬‚·‚é
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

bool map_cache_read(struct map_data &cmap)
{
	size_t i;
	if(!map_cache.fp) { return 0; }
	for(i=0; i<map_cache.head.nmaps; ++i)
	{
		if( map_cache.map[i].water_height == cmap.wh &&	// …ê‚Ì‚‚³‚ªˆá‚¤‚Ì‚Å“Ç‚İ’¼‚µ
			0==strcmp(cmap.mapname,map_cache.map[i].fn) )
			break;
	}
	if( i < map_cache.head.nmaps )	
	{
		if(map_cache.map[i].compressed == 0)
		{
				// ”ñˆ³kƒtƒ@ƒCƒ‹
			const size_t dest_sz = map_cache.map[i].xs*map_cache.map[i].ys;
			cmap.xs = map_cache.map[i].xs;
			cmap.ys = map_cache.map[i].ys;
			cmap.gat = new struct mapgat[dest_sz];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if( map_cache.map[i].datalen > dest_sz*sizeof(struct mapgat) )
				map_cache.map[i].datalen = dest_sz*sizeof(struct mapgat);
			if( map_cache.map[i].datalen != fread(cmap.gat, 1, map_cache.map[i].datalen, map_cache.fp) )
			{	// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
				delete[] cmap.gat;
				cmap.gat = NULL;
				cmap.xs = 0;
				cmap.ys = 0;
				return false;
			}
			else
			{	// ¬Œ÷
				return true;
			}
		}
		else if(map_cache.map[i].compressed == 1)
		{	//zlib compressed
				// ˆ³kƒtƒ‰ƒO=1 : zlib
			unsigned char *buf;
			unsigned long size_compress = map_cache.map[i].datalen;
			unsigned long dest_len = map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat);
			cmap.xs = map_cache.map[i].xs;
			cmap.ys = map_cache.map[i].ys;
			buf = new unsigned char[size_compress];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if(fread(buf,1,size_compress,map_cache.fp) != size_compress)
			{	// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
				delete[] buf;
				buf = NULL;
				cmap.gat = NULL;
				cmap.xs = 0; 
				cmap.ys = 0; 
				return false;
			}
			cmap.gat = new struct mapgat[map_cache.map[i].xs * map_cache.map[i].ys];
			decode_zip((unsigned char*)cmap.gat, dest_len, buf, size_compress);
			if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat))
			{	// ³í‚É‰ğ“€‚ªo—ˆ‚Ä‚È‚¢
				delete[] buf;
				buf=NULL;
				delete[] cmap.gat;
				cmap.gat = NULL;
				cmap.xs = 0; 
				cmap.ys = 0; 
				return false;
			}
			if(buf)
			{	// might be ok without this check
				delete[] buf;
				buf=NULL;
			}
			return true;
		}
		// might add other compressions here
	}
	return false;
}

bool map_cache_write(struct map_data &cmap)
{
	size_t i;
	unsigned long len_new, len_old;
	unsigned char *write_buf;

	if(!map_cache.fp)
		return false;

	for(i = 0;i < map_cache.head.nmaps ; ++i) 
	{
		if( (0==strcmp(cmap.mapname,map_cache.map[i].fn)) || (map_cache.map[i].fn[0] == 0) )
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
			// ˆ³k•Û‘¶
			// ‚³‚·‚ª‚É‚Q”{‚É–c‚ê‚é–‚Í‚È‚¢‚Æ‚¢‚¤–‚Å
			len_new = 2 * cmap.xs * cmap.ys * sizeof(struct mapgat);
			write_buf = new unsigned char[len_new];
			encode_zip(write_buf,len_new,(unsigned char *)cmap.gat, cmap.xs*cmap.ys*sizeof(struct mapgat));
		}
		else
		{	// no compress
			len_new = cmap.xs * cmap.ys *sizeof(struct mapgat);
			write_buf = (unsigned char*)cmap.gat;
		}
		
		// now insert it
		if( (map_cache.map[i].fn[0] == 0) )
		{	// new map is inserted
			// write at the end of the mapcache file
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

			// prepare the data header
			memcpy(map_cache.map[i].fn, cmap.mapname, sizeof(map_cache.map[i].fn));
			map_cache.map[i].fn[sizeof(map_cache.map[i].fn)-1]=0;			

			// update file header
			map_cache.map[i].pos = map_cache.head.filesize;
			map_cache.head.filesize += len_new;
		}
		else
		{	// update an existing map
			len_old = map_cache.map[i].datalen;

			if(len_new <= len_old) // size is ok
			{	// ƒTƒCƒY‚ª“¯‚¶‚©¬‚³‚­‚È‚Á‚½‚Ì‚ÅêŠ‚Í•Ï‚í‚ç‚È‚¢
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
	
			}
			else 
			{	// new len is larger then the old space -> write at file end
				// V‚µ‚¢êŠ‚É“o˜^
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

					// update file header
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
		}
		// just make sure that everything gets updated
		map_cache.map[i].xs  = cmap.xs;
		map_cache.map[i].ys  = cmap.ys;
		map_cache.map[i].water_height = cmap.wh;
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
	// ‘‚«bß‚È‚©‚Á‚½
	return false;
}

bool map_readafm(struct map_data& cmap, const char *fn=NULL)
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
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, cmap.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", cmap.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".afm");
	}
	else
		safestrcpy(buf, fn, sizeof(buf));

	afm_file = basics::safefopen(buf, "r");
	if (afm_file != NULL)
	{
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		xs = cmap.xs = afm_size[0];
		ys = cmap.ys = afm_size[1];

		cmap.npc_num=0;
		cmap.users=0;
		memset(&cmap.flag,0,sizeof(cmap.flag));

		if(config.pk_mode) cmap.flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		cmap.gat = new struct mapgat[cmap.xs*cmap.ys];
		for (y = 0; y < ys; ++y)
		{
			str=fgets(afm_line, sizeof(afm_line), afm_file);
			for (x = 0; x < xs; ++x)
			{
				map_setcell(cmap.m,x,y, str[x] & CELL_MASK );
			}
		}

		cmap.bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
		cmap.bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
		size = cmap.bxs * cmap.bys;

		cmap.objects = new struct map_data::_objects[size];

		strdb_insert(map_db,cmap.mapname,&cmap);

		fclose(afm_file);
		return true;
	}
	return false;
}

bool map_readaf2(struct map_data& cmap, const char*fn=NULL)
{
	FILE *af2_file, *dest;
	char buf[256];
	bool ret=false;

	if(!fn)
	{
		char *ip;
		if(afm_dir && *afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, cmap.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", cmap.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".af2");
	}
	else
		safestrcpy(buf, fn, sizeof(buf));

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
			if(ret) ret = map_readafm(cmap, buf);
			remove(buf);
		}
	}
	return ret;
}


/*==========================================
 * ƒ}ƒbƒv1–‡“Ç‚İbİ
 * ===================================================*/
bool map_readgrf(struct map_data& cmap, const char *fn=NULL)
{
	// read from grf
	int x,y;
	struct gat_1cell {float high[4]; uint32 type;};
	unsigned char *gat;
	char buf[512];
	
	if(!fn)
	{	char* ip;
		// have windows backslash as path seperator here
		snprintf(buf, sizeof(buf), "data\\%s", cmap.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		// append ".gat" for reading in grf's
		strcat(buf, ".gat");
	}
	else
	{	
		safestrcpy(buf, fn, sizeof(buf));
		// append ".gat" if not already exist
		if( NULL == strstr(buf,".gat") )
			strcat(buf, ".gat");
	}


	gat = (unsigned char *)grfio_read(buf);
	if( gat )
	{
		const unsigned char *p = gat+14;
		struct gat_1cell pp;	// make a real structure in memory

		cmap.xs= RBUFL(gat, 6);
		cmap.ys= RBUFL(gat,10);
		cmap.gat = new struct mapgat[cmap.xs*cmap.ys];

//float min=3.40282e+38, max=-3.40282e+38;
		
		for(y=0;y<cmap.ys;++y)
		for(x=0;x<cmap.xs;++x)
		{
			// faster and typesafe
			_F_frombuffer(pp.high[0], p);
			_F_frombuffer(pp.high[1], p);
			_F_frombuffer(pp.high[2], p);
			_F_frombuffer(pp.high[3], p);
			_L_frombuffer(pp.type, p);
			// buffer increment is done automatically 
/*
			struct gat_1cell px;
			memcpy(&px, p, sizeof(struct gat_1cell));	// copy all stuff
			p += sizeof(struct gat_1cell);				// set pointer to next section

			if(MSB_FIRST==CheckByteOrder()) // little/big endian
			{	// need to correct the whole struct since we have no suitable buffer assigns
				// gat_1cell contains 4 floats and one int (4byte each) so swapping these is enough
				// the structure is memory alligned so it is safe to use just the pointers
				SwapFourBytes(((char*)(&px)) + sizeof(float)*0);
				SwapFourBytes(((char*)(&px)) + sizeof(float)*1);
				SwapFourBytes(((char*)(&px)) + sizeof(float)*2);
				SwapFourBytes(((char*)(&px)) + sizeof(float)*3);
				SwapFourBytes(((char*)(&px)) + sizeof(int)*4);
			}
		// debugcompare
			if( px.high[0]!=pp.high[0] || 
				px.high[1]!=pp.high[1] || 
				px.high[2]!=pp.high[2] || 
				px.high[3]!=pp.high[3] || 
				px.type   !=pp.type )
			{
				printf("\nMaploaderror\n");
			}
*/
/*
if(pp.type!=1 && pp.type!=5)
{
	if(pp.high[0]<min) min = pp.high[0]; else if(pp.high[0]>max) max = pp.high[0];
	if(pp.high[1]<min) min = pp.high[1]; else if(pp.high[1]>max) max = pp.high[1];
	if(pp.high[2]<min) min = pp.high[2]; else if(pp.high[2]>max) max = pp.high[2];
	if(pp.high[3]<min) min = pp.high[3]; else if(pp.high[3]>max) max = pp.high[3];
}
*/
			if(cmap.wh!=NO_WATER && pp.type==0)
				map_setcell(cmap.m,x,y,(pp.high[0]>cmap.wh || pp.high[1]>cmap.wh || pp.high[2]>cmap.wh || pp.high[3]>cmap.wh) ? 3 : 0);
			else
				map_setcell(cmap.m,x,y,pp.type);
		}
//printf("\n%f, %f\n", min, max);
		delete[] gat;
		return true;
	}
	return false;
}

/*==========================================
 * ‘S‚Ä‚Ìmapƒf?ƒ^‚ğ?‚İ?‚Ş
 *------------------------------------------
 */




int map_readallmap(void)
{
	size_t i, maps_removed=0;
	bool ch;

	// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ğŠJ‚­
	if(map_read_flag >= READ_FROM_BITMAP)
	{
		map_cache_open(map_cache_file);
	}

	ShowStatus("Loading Maps%s...\n",
		(map_read_flag == READ_FROM_BITMAP_COMPRESSED ? " (w/ Compressed Map Cache)" :
		map_read_flag >= READ_FROM_BITMAP ? " (w/ Map Cache)" :
		map_read_flag == READ_FROM_AFM ? " (w/ AFM)" : ""));

	// æ‚É‘S•”‚Ìƒƒbƒv‚Ì‘¶İ‚ğŠm”F
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
			ShowMessage("\rLoading Maps [%d/%d]: %s, size (%d %d)(%i)"CL_CLL, i,map_num, maps[i].mapname, maps[i].xs,maps[i].ys, maps[i].wh);
	
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
			ShowMessage("\rRemoving Map [%d/%d]: %s"CL_CLL, i,map_num, maps[i].mapname);
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
 * ?‚İ?‚Şmap‚ğ’Ç‰Á‚·‚é
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
	safestrcpy(maps[map_num].mapname, mapname, sizeof(maps[map_num].mapname));
	char *ip = strchr(maps[map_num].mapname, '.');
	if(ip) *ip=0;

	map_num++;
	return 0;
}

/*==========================================
 * ?‚İ?‚Şmap‚ğíœ‚·‚é
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
				ShowMessage("Removing map [ %s ] from maplist\n", buffer);
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
	struct map_session_data sd;

	sd.fd = 0;
	strcpy( sd.status.name , "console");

	if ( ( n = sscanf(buf, "%[^:]:%[^:]:%99s %d %d[^\n]", type , command , map , &x , &y )) < 5 )
		if ( ( n = sscanf(buf, "%[^:]:%[^\n]", type , command )) < 2 )
			n = sscanf(buf,"%[^\n]",type);

	if ( n == 5 ) {
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

    if ( strcasecmp("admin",type) == 0 && n == 5 ) {
		sprintf(buf2,"console: %s",command);
        if( is_atcommand(sd.fd, sd, buf2, 99) == AtCommand_None )
			ShowConsole(CL_BOLD"no valid atcommand\n"CL_NORM);
    } else if ( strcasecmp("server",type) == 0 && n == 2 ) {
        if ( strcasecmp("shutdown", command) == 0 || strcasecmp("exit",command) == 0 || strcasecmp("quit",command) == 0 ) {
            core_stoprunning();
		}
    } else if ( strcasecmp("help",type) == 0 ) {
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
 * İ’èƒtƒ@ƒCƒ‹‚ğ?‚İ?‚Ş
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
	
	while(fgets(line, sizeof(line), fp)) {
		if( !get_prepared_line(line) )
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			if (strcasecmp(w1, "userid")==0){
				chrif_setuserid(w2);
			} else if (strcasecmp(w1, "passwd") == 0) {
				chrif_setpasswd(w2);
			}
			else if (strcasecmp(w1, "char_ip") == 0) {
				getcharaddress() = w2;
				ShowInfo("Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", w2, getcharaddress().tostring(NULL));
			} 
else if (strcasecmp(w1, "char_port") == 0) {
	getcharaddress().port() = atoi(w2);
}
			else if (strcasecmp(w1, "map_ip") == 0) {
				getmapaddress() = w2;
				ShowInfo("Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", w2, getmapaddress().tostring(NULL));
			}
else if (strcasecmp(w1, "map_port") == 0) {
	getmapaddress().LANPort() = atoi(w2);
}

			else if (strcasecmp(w1, "water_height") == 0) {
				waterlist.open(w2);
			} else if (strcasecmp(w1, "map") == 0) {
				map_addmap(w2);
			} else if (strcasecmp(w1, "delmap") == 0) {
				map_delmap(w2);
			} else if (strcasecmp(w1, "npc") == 0) {
				npc_addsrcfile(w2);
			} else if (strcasecmp(w1, "path") == 0) {
				////////////////////////////////////////
				// add all .txt files recursive from ./npc folder to npc source tree
				basics::findFiles(w2, "*.txt", npc_addsrcfile );
				////////////////////////////////////////
			} else if (strcasecmp(w1, "delnpc") == 0) {
				npc_delsrcfile(w2);
			} else if (strcasecmp(w1, "autosave_time") == 0) {
				autosave_interval = atoi(w2) * 1000;
				if (autosave_interval <= 0)
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			} else if (strcasecmp(w1, "motd_txt") == 0) {
				safestrcpy(motd_txt, w2, sizeof(motd_txt));
			} else if (strcasecmp(w1, "help_txt") == 0) {
				safestrcpy(help_txt, w2, sizeof(help_txt));
			} else if (strcasecmp(w1, "mapreg_txt") == 0) {
				safestrcpy(mapreg_txt, w2, sizeof(mapreg_txt));
			}else if(strcasecmp(w1,"read_map_from_cache")==0){
				if (atoi(w2) == 2)
					map_read_flag = READ_FROM_BITMAP_COMPRESSED;
				else if (atoi(w2) == 1)
					map_read_flag = READ_FROM_BITMAP;
				else
					map_read_flag = READ_FROM_GAT;
			}else if(strcasecmp(w1,"map_cache_file")==0){
				safestrcpy(map_cache_file,w2,sizeof(map_cache_file));
			} else if(strcasecmp(w1,"afm_dir") == 0) {
				safestrcpy(afm_dir, w2,sizeof(afm_dir));
			} else if (strcasecmp(w1, "import") == 0) {
				map_config_read(w2);
			} else if (strcasecmp(w1, "console") == 0) {
				console = config_switch(w2);
				}
            }
		}
	fclose(fp);


	return 0;
}

void char_online_check(void)
{
	size_t i;
	struct map_session_data *sd;

	chrif_char_reset_offline();

	for(i=0;i<fd_max;++i)
	{
		if( session[i] &&
			(sd = (struct map_session_data*)session[i]->user_session) && 
			sd->state.auth &&
			!(config.hide_GM_session && sd->isGM()) &&
			sd->status.char_id)
		{
			chrif_char_online(*sd);
		}
	}
}

int online_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	char_online_check();
	return 0;
}

int id_db_final(void *k,void *d) 
{
	return 0; 
}

int map_db_final(void *k,void *d) 
{
	return 0; 
}
int nick_db_final(void *k,void *d)
{
	char *p = (char *) d;
	if (p) delete[] p;
	return 0;
}
int charid_db_final(void *k,void *d)
{
	struct charid2nick *p = (struct charid2nick *) d;
	if (p) delete p;
	return 0;
}


class CMapCleanup : public CMapProcessor
{
public:
	CMapCleanup()	{}
	~CMapCleanup()	{}
	virtual int process(struct block_list& bl) const
	{
		switch(bl.type)
		{
			case BL_PC:
				map_quit( (struct map_session_data &)bl );
				break;
			case BL_NPC:
				npc_unload( (struct npc_data *)&bl );
				break;
			case BL_MOB:
				mob_unload( (struct mob_data &)bl);
				break;
			case BL_PET:
				pet_remove_map( (struct map_session_data &)bl );
				break;
			case BL_ITEM:
				map_clearflooritem(bl.id);
				break;
			case BL_SKILL:
				skill_delunit( (struct skill_unit *)&bl);
				break;
			default:
				ShowError("cleanup_sub unhandled block, type%i\n", bl.type);
		}
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
				maps[m].npc[i]->map_delblock();
				numdb_erase(id_db,maps[m].npc[i]->block_list::id);
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
 * mapII—¹—
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
			CMap::foreachinarea( CMapCleanup(), i, 0, 0, maps[i].xs-1, maps[i].ys-1, 0);
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
	do_final_msg();
	do_final_chrif(); // ‚±‚Ì“à•”‚ÅƒLƒƒƒ‰‚ğ‘S‚ÄØ’f‚·‚é


	for (i=0; i<map_num; ++i)
	{
		clear_moblist(i);
		if(maps[i].gat)		{ delete[] maps[i].gat; maps[i].gat=NULL; }
		if(maps[i].objects)	{ delete[] (maps[i].objects); maps[i].objects=NULL; }
	}
	if(id_db)
	{
		numdb_final(id_db, id_db_final);
		id_db=NULL;
	}
	if(map_db)
	{
		strdb_final(map_db, map_db_final);
		map_db=NULL;
	}
	if(nick_db)
	{
		strdb_final(nick_db, nick_db_final);
		nick_db=NULL;
	}
	if(charid_db)
	{
		numdb_final(charid_db, charid_db_final);
		charid_db=NULL;
	}


	log_final();


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
void map_helpscreen(int flag) { // by MC Cameri
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
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_versionscreen(int flag) {
	ShowMessage(CL_WHITE"eAthena version %d.%02d.%02d, Athena Mod version %d"CL_RESET"\n",
		ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION, ATHENA_MOD_VERSION);
	ShowMessage(CL_BT_GREEN "Website/Forum:" CL_RESET "\thttp://eathena.deltaanime.net/");
	ShowMessage(CL_BT_GREEN "Download URL:" CL_RESET "\thttp://eathena.systeminplace.net/");
	ShowMessage(CL_BT_GREEN "IRC Channel:" CL_RESET "\tirc://irc.deltaanime.net/#athena");
	ShowMessage("\nOpen " CL_WHITE "readme.html" CL_RESET " for more information.");
	if (ATHENA_RELEASE_FLAG) ShowNotice("This version is not for release.\n");
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


#ifdef GCOLLECT
	GC_enable_incremental();
#endif


	// just clear all maps
	memset(maps, 0, MAX_MAP_PER_SERVER*sizeof(struct map_data));

	for (i = 1; i < argc ; ++i) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--?") == 0 || strcmp(argv[i], "/?") == 0)
			map_helpscreen(1);
		else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "/v") == 0)
			map_versionscreen(1);
		else if (strcmp(argv[i], "--map_config") == 0 || strcmp(argv[i], "--map-config") == 0)
			MAP_CONF_NAME=argv[i+1];
		else if (strcmp(argv[i],"--config") == 0 || strcmp(argv[i],"--battle-config") == 0)
			BATTLE_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--atcommand_config") == 0 || strcmp(argv[i],"--atcommand-config") == 0)
			ATCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--charcommand_config") == 0 || strcmp(argv[i],"--charcommand-config") == 0)
			CHARCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--script_config") == 0 || strcmp(argv[i],"--script-config") == 0)
			SCRIPT_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--msg_config") == 0 || strcmp(argv[i],"--msg-config") == 0)
			MSG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--grf_path_file") == 0 || strcmp(argv[i],"--grf-path-file") == 0)
			GRF_PATH_FILENAME = argv[i+1];
#if defined(WITH_MYSQL)
		else if (strcmp(argv[i],"--inter_config") == 0 || strcmp(argv[i],"--inter-config") == 0)
			INTER_CONF_NAME = argv[i+1];
#endif
		else if (strcmp(argv[i],"--log_config") == 0 || strcmp(argv[i],"--log-config") == 0)
			LOG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--run_once") == 0)	// close the map-server as soon as its done.. for testing [Celest]
			core_stoprunning();
	}

	map_config_read(MAP_CONF_NAME);
	basics::CParamBase::loadFile(MAP_CONF_NAME);

	if (SHOW_DEBUG_MSG)
		ShowNotice("Server running in '"CL_WHITE"Debug Mode"CL_RESET"'.\n");
	config_read(BATTLE_CONF_FILENAME);
	msg_config_read(MSG_CONF_NAME);
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	charcommand_config_read(CHARCOMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);


	log_init(LOG_CONF_NAME);

	id_db = numdb_init();
	map_db = strdb_init(24);
	nick_db = strdb_init(24);
	charid_db = numdb_init();

	grfio_init(GRF_PATH_FILENAME);

	battle_init();
	map_readallmap();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	add_timer_func_list(map_removemobs_timer, "map_removemobs_timer");
	add_timer_interval(gettick()+1000, 60*1000, map_freeblock_timer, 0, 0);

	// online status timer, checks every hour [Valaris]
	add_timer_func_list(online_timer, "online_timer");
	add_timer_interval(gettick()+10, CHECK_INTERVAL, online_timer, 0, 0);	

	do_init_script();
	do_init_itemdb();
	do_init_mob();	// npc‚Ì‰Šú‰»‚Åmob_spawn‚µ‚ÄAmob_db‚ğ?Æ‚·‚é‚Ì‚Åinit_npc‚æ‚èæ
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



	npc_event_do_oninit();	// npc‚ÌOnInitƒCƒxƒ“ƒg?s

	if ( console ) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	if (config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	return 0;
}

