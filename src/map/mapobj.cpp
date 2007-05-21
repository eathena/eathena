#include "basesync.h"
#include "showmsg.h"

#include "battle.h"
#include "clif.h"
#include "intif.h"
#include "config.h"
#include "flooritem.h"
#include "mapobj.h"
#include "mob.h"
#include "party.h"
#include "pc.h"
#include "pet.h"
#include "status.h"

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
block_list block_list::bl_head;

/////////////////////////////////////////////////////////////////////
dbt* block_list::id_db=NULL;

static block_list *quickobj_list[MAX_QUICKOBJ];
static uint32 first_free_object_id=0;
static uint32 last_object_id=0;

/// !! rewrite, not threadsafe
#define block_free_max 1048576
static block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

/// !! remove, highly insecure/not threadsafe
#define BL_LIST_MAX 1048576
static block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;


///////////////////////////////////////////////////////////////////////////////
/// add id to id_db
/// * id_dbへblを追加
uint32 block_list::register_id(uint32 iid) 
{
	this->block_list::id = iid;
	numdb_insert(block_list::id_db, iid, this);
	return iid;
}

///////////////////////////////////////////////////////////////////////////////
/// 
uint32 block_list::register_id()
{
	uint32 i;
	if(first_free_object_id<2 || first_free_object_id>=MAX_QUICKOBJ)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_QUICKOBJ;++i)
	{
		if(quickobj_list[i]==NULL)
			break;
	}
	if(i>=MAX_QUICKOBJ)
	{
		if(config.error_log)
			ShowMessage("no free quickobject id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	quickobj_list[i]=this;
	numdb_insert(block_list::id_db, i, this);
	this->block_list::id = i;
	return i;
}

///////////////////////////////////////////////////////////////////////////////
/// remove id from id_db
/// id_dbからblを削除
uint32 block_list::unregister_id() 
{
	if( this->id < MAX_QUICKOBJ )
	{	// clean quicklist
		block_list *obj = quickobj_list[this->id];
		if( obj==this )
		{
			quickobj_list[this->id]=NULL;
			if(first_free_object_id>this->id)
				first_free_object_id=this->id;
			while(last_object_id>2 && quickobj_list[last_object_id]==NULL)
				--last_object_id;
		}
	}
	numdb_erase(block_list::id_db,this->id);
	return this->id;
}

///////////////////////////////////////////////////////////////////////////////
/// returns blocklist from given id
block_list * block_list::from_blid(uint32 id)
{
	block_list *bl=NULL;
	if((size_t)id<sizeof(quickobj_list)/sizeof(quickobj_list[0]))
		bl = quickobj_list[id];
	else
		bl = (block_list*)numdb_search(block_list::id_db,id);
	return bl;
}



/// detect undead state.
/// is only dependend on race/element so not overloaded
bool block_list::is_undead() const
{
	if(config.undead_detect_type == 0)
	{
		return ( 9==status_get_elem_type(this) );
	}
	else if(config.undead_detect_type == 1)
	{
		return ( 1==this->get_race() );
	}
	else
	{
		return ( 9==status_get_elem_type(this) || 1==this->get_race() );
	}
}


///////////////////////////////////////////////////////////////////////////////
/// process blocks standing on a path.
int map_intern::foreachinpath(const CMapProcessor& elem, int x0,int y0,int x1,int y1,int range,object_t type)
{
	int returnCount =0;
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
	if (x1 >= this->xs) x1 = this->xs-1;
	if (y1 >= this->ys) y1 = this->ys-1;

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
	if(bx1+i<this->bxs)	bx1 +=i; else bx1=this->bxs-1;
	if(by1+i<this->bys)	by1 +=i; else by1=this->bys-1;


//ShowMessage("run for (%i,%i)(%i,%i)\n",bx0,by0,bx1,by1);
	for(bx=bx0; bx<=bx1; ++bx)
	for(by=by0; by<=by1; ++by)
	{	// block xy
		c1  = this->block_count[bx+by*this->bxs];		// number of elements in the block
		c2  = this->block_mob_count[bx+by*this->bxs];	// number of mobs in the mob block
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
  			bl = this->block[bx+by*this->bxs];		// a block with the elements
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
			bl = this->block_mob[bx+by*this->bxs];	// and the mob block
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

	
	block_list::map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();
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
	if (x1 >= this->xs) x1 = this->xs-1;
	if (y1 >= this->ys) y1 = this->ys-1;

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
			c1  = this->block_count[bx+by*this->bxs];		// number of elements in the block
			c2  = this->block_mob_count[bx+by*this->bxs];	// number of mobs in the mob block
			if( (c1==0) && (c2==0) ) continue;				// skip if nothing in the block

			if(type==0 || type!=BL_MOB) {
				bl = this->block[bx+by*this->bxs];		// a block with the elements
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
				bl = this->block_mob[bx+by*this->bxs];	// and the mob block
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

	block_list::map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;++i)
	{
		if(bl_list[i]->prev)
		{
			returnCount += elem.process(bl_list[i]);
		}
	}
	block_list::map_freeblock_unlock();
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
	block_list::freeblock_lock();

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x0 >= this->xs) x0 = this->xs-1;
	if (y0 >= this->ys) y0 = this->ys-1;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x1 >= this->xs) x1 = this->xs-1;
	if (y1 >= this->ys) y1 = this->ys-1;

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
				c1  = this->objects[bx+by*this->bxs].cnt_blk;	// number of elements in the block
				c2  = this->objects[bx+by*this->bxs].cnt_mob;	// number of mobs in the mob block
				if( (c1!=0) || (c2!=0) )						// skip if nothing in the block
				{
					if(type==0 || type!=BL_MOB)
					{
						bl = this->objects[bx+by*this->bxs].root_blk;	// a block with the elements
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
						bl = this->objects[bx+by*this->bxs].root_mob;	// and the mob block
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
		if(bl_list[i] && bl_list[i]->prev)
		{
			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::freeblock_unlock();
	bl_list_count = blockcount;

	return returnCount;

}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing on a exact position.
int map_intern::foreachincell(const CMapProcessor& elem, int x,int y, object_t type)
{
	int returnCount =0;
	if(x > 0 && y > 0 && x < this->xs &&  y < this->ys )
	{
		int bx,by;
		block_list *bl=NULL;
		int blockcount=bl_list_count,i,c;
		block_list::freeblock_lock();

		by=y/BLOCK_SIZE;
		bx=x/BLOCK_SIZE;

		if(type==0 || type!=BL_MOB)
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c = this->objects[bx+by*this->bxs].cnt_blk;
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
			bl = this->objects[bx+by*this->bxs].root_mob;
			c = this->objects[bx+by*this->bxs].cnt_mob;
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
			if(bl_list[i] && bl_list[i]->prev)
				returnCount += elem.process(*bl_list[i]);
		}
		block_list::freeblock_unlock();
		bl_list_count = blockcount;
	}
	return returnCount;
}

///////////////////////////////////////////////////////////////////////////////
/// count blocks standing on a exact position.
int map_intern::countoncell(int x, int y, object_t type)
{
	int count = 0;
	if(x > 0 && y > 0 && x < this->xs &&  y < this->ys )
	{
		int bx = x/BLOCK_SIZE;
		int by = y/BLOCK_SIZE;
		block_list *bl=NULL;
		int i,c;

		block_list::freeblock_lock();
		if( type == BL_ALL || type != BL_MOB )
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c = this->objects[bx+by*this->bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if( bl->x == x && bl->y == y && *bl == BL_PC )
					++count;
			}
		}
		if( type == BL_ALL || type == BL_MOB )
		{
			bl = this->objects[bx+by*this->bxs].root_mob;
			c = this->objects[bx+by*this->bxs].cnt_mob;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if( bl->x == x && bl->y == y )
					++count;
			}
		}
		block_list::freeblock_unlock();
	}
	return count;
}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing inside a move area.
/// (move area is the difference cut between the source and target area
int map_intern::foreachinmovearea(const CMapProcessor& elem, int x0,int y0,int x1,int y1,int dx,int dy,object_t type)
{
	int bx,by;
	int returnCount =0;
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;
	block_list::freeblock_lock();

	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);
	if(dx==0 || dy==0)
	{
		if(dx==0)
		{
			if(dy<0)
			{
				y0=y1+dy+1;
			}
			else
			{
				y1=y0+dy-1;
			}
		}
		else if(dy==0)
		{
			if(dx<0)
			{
				x0=x1+dx+1;
			}
			else
			{
				x1=x0+dx-1;
			}
		}
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=this->xs) x1=this->xs-1;
		if(y1>=this->ys) y1=this->ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by)
		for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c  = this->objects[bx+by*this->bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && !bl->is_type(type) )
					continue;
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
			bl = this->objects[bx+by*this->bxs].root_mob;
			c  = this->objects[bx+by*this->bxs].cnt_mob;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && !bl->is_type(type) )
					continue;
				if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}
	else
	{
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=this->xs) x1=this->xs-1;
		if(y1>=this->ys) y1=this->ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE; ++by)
		for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c  = this->objects[bx+by*this->bxs].cnt_blk;
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
			bl = this->objects[bx+by*this->bxs].root_mob;
			c  = this->objects[bx+by*this->bxs].cnt_mob;
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
		{
			map_session_data *sd = bl_list[i]->get_sd();
			if( sd && !session_isActive(sd->fd) )
				continue;

			returnCount += elem.process(*bl_list[i]);
		}
	}
	block_list::freeblock_unlock();
	bl_list_count = blockcount;
	return returnCount;
}
///////////////////////////////////////////////////////////////////////////////
/// process blocks standing inside an area.
int map_intern::foreachinarea(const CMapProcessor& elem, int x0,int y0,int x1,int y1,object_t type)
{
	int bx,by;
	int returnCount =0;
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	block_list::freeblock_lock();
	
	if(x0>x1) basics::swap(x0,x1);
	if(y0>y1) basics::swap(y0,y1);

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= this->xs) x1 = this->xs-1;
	if (y1 >= this->ys) y1 = this->ys-1;
	if (type == 0 || type != BL_MOB)
	{
		for(by = y0/BLOCK_SIZE; by<=y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx<=x1/BLOCK_SIZE; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c  = this->objects[bx+by*this->bxs].cnt_blk;
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
		for(by = y0/BLOCK_SIZE; by<=y1/BLOCK_SIZE; ++by)
		for(bx = x0/BLOCK_SIZE; bx<=x1/BLOCK_SIZE; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_mob;
			c  = this->objects[bx+by*this->bxs].cnt_mob;
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
		if(bl_list[i] && bl_list[i]->prev)
			returnCount += elem.process(*bl_list[i]);

	block_list::freeblock_unlock();
	bl_list_count = blockcount;

	return returnCount;
}

///////////////////////////////////////////////////////////////////////////////
/// process each block
int map_intern::foreach(const CMapProcessor& elem, object_t type)
{
	int bx,by;
	int returnCount =0;
	block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	block_list::freeblock_lock();
	
	if (type == 0 || type != BL_MOB)
	{
		for(by = 0; by < this->bys; ++by)
		for(bx = 0; bx < this->bxs; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_blk;
			c  = this->objects[bx+by*this->bxs].cnt_blk;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && !bl->is_type(type) )
					continue;
				if(bl && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}
	if(type==0 || type==BL_MOB)
	{
		for(by = 0; by < this->bys; ++by)
		for(bx = 0; bx < this->bxs; ++bx)
		{
			bl = this->objects[bx+by*this->bxs].root_mob;
			c  = this->objects[bx+by*this->bxs].cnt_mob;
			for(i=0;i<c && bl;i++,bl=bl->next)
			{
				if(bl && bl_list_count<BL_LIST_MAX)
					bl_list[bl_list_count++]=bl;
			}
		}
	}

	if(bl_list_count>=BL_LIST_MAX)
	{
		if(config.error_log)
			ShowMessage("map_foreach: *WARNING* block count too many!\n");
	}


	for(i=blockcount;i<bl_list_count;++i)
		if(bl_list[i] && bl_list[i]->prev)
			returnCount += elem.process(*bl_list[i]);

	block_list::freeblock_unlock();
	bl_list_count = blockcount;

	return returnCount;
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

	for(i=2;i<=(int)last_object_id;++i)
	{
		if(quickobj_list[i])
		{
			if( !quickobj_list[i]->is_type(type) )
				continue;
			if(bl_list_count>=BL_LIST_MAX)
			{
				if(config.error_log)
					ShowMessage("map_foreachobject: too many block !\n");
				break;
			}
			else
				bl_list[bl_list_count++]=quickobj_list[i];
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

	if( this->m<maps.size() && this->x<maps[this->m].xs && this->y<maps[this->m].ys )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_intern::_objects &obj = maps[this->m].objects[pos];

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
					sd.pd->menu(3); // Option 3 is return to egg.
				}
				if(maps[this->m].users++ == 0 && config.dynamic_mobs)	// Skotlex
					maps[this->m].moblist_spawn();
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
	else if( this->m < maps.size() )
	{
		const size_t pos = this->x/BLOCK_SIZE+(this->y/BLOCK_SIZE)*maps[this->m].bxs;
		map_intern::_objects &obj = maps[this->m].objects[pos];

		if( this->is_type(BL_PC) )
		{
			if( maps[this->m].users>0 && --maps[this->m].users == 0 && config.dynamic_mobs)
				maps[this->m].moblist_release();
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
/// sync object for threadsave freeblock
static basics::Mutex mtx_blockfree;

///////////////////////////////////////////////////////////////////////////////
/// block削除の安全性確保?理
int block_list::freeblock()
{
	basics::ScopeLock sl(mtx_blockfree);
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
	basics::ScopeLock sl(mtx_blockfree);
	return ++block_free_lock;
}

///////////////////////////////////////////////////////////////////////////////
/// blockのaFreeのロックを解除する
/// このとき、ロックが完全になくなると
/// バッファにたまっていたblockを全部削除
int block_list::freeblock_unlock (void)
{
	basics::ScopeLock sl(mtx_blockfree);
	if((--block_free_lock) == 0)
	{
		while(block_free_count)
		{
			--block_free_count;
			if( block_free[block_free_count] )
				delete block_free[block_free_count];
		}
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
	basics::ScopeLock sl(mtx_blockfree);
	if(block_free_lock > 0)
	{
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		block_list::freeblock_unlock();
	}
	return 0;
}
