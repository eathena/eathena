// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "showmsg.h"
#include "timer.h"

#include "map.h"
#include "battle.h"
#include "clif.h"
#include "movable.h"
#include "pc.h"
#include "skill.h"
#include "status.h"


const static char dirx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
const static char diry[8] = { 1, 1, 0,-1,-1,-1, 0, 1};


//## change to static initializer when timer function management got classified
void _movable_init()
{
	add_timer_func_list(movable::walktimer_entry, "walktimer entry function");
}

///////////////////////////////////////////////////////////////////////////////
/// constructor
movable::movable() : 
	canmove_tick(0),
	walktimer(-1),
	bodydir(DIR_S),
	headdir(DIR_S),
	speed(DEFAULT_WALK_SPEED)
{
	static bool initialiser = true;
	if(initialiser)
	{
		_movable_init();
		initialiser = false;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// init directions seperately. dont do sends;
void movable::init_dir(dir_t b, dir_t h)
{
	this->bodydir = b;
	this->headdir = h;
}

///////////////////////////////////////////////////////////////////////////////
/// set directions seperately.
void movable::set_dir(dir_t b, dir_t h)
{
	const bool ch = this->bodydir == b || this->headdir == h;
	if(ch) this->bodydir = b, this->headdir = h, clif_changed_dir(*this); 
}
///////////////////////////////////////////////////////////////////////////////
/// set both directions equally.
void movable::set_dir(dir_t d)
{
	const bool ch = this->bodydir == d || this->headdir == d;
	if(ch) this->bodydir = this->headdir = d, clif_changed_dir(*this);
}
///////////////////////////////////////////////////////////////////////////////
/// set directions to look at target
void movable::set_dir(const coordinate& to)
{
	const dir_t d = get_direction(to);
	const bool ch = this->bodydir == d || this->headdir == d;
	if(ch) this->bodydir = this->headdir = d, clif_changed_dir(*this);
}
///////////////////////////////////////////////////////////////////////////////
/// set body direction only.
void movable::set_bodydir(dir_t d)
{
	const bool ch = this->bodydir == d;
	if(ch) this->bodydir=d, clif_changed_dir(*this);
}
///////////////////////////////////////////////////////////////////////////////
/// set head direction only.
void movable::set_headdir(dir_t d)
{
	const bool ch = this->headdir == d;
	if(ch) this->headdir=d, clif_changed_dir(*this);
}
///////////////////////////////////////////////////////////////////////////////
/// walktimer entry function.
/// it actually combines the extraction of the object and calling the 
/// object interal handler, which formally was pc_timer, mob_timer, etc.
int movable::walktimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	movable* mv = movable::from_blid(id);
	if( mv )
	{
		if(mv->walktimer != tid)
		{
			if(config.error_log)
				ShowError("walktimer_entry %d != %d\n",mv->walktimer,tid);
			return 0;
		}
		// timer was executed, clear it
		// have it temporarily set for clif functions
		mv->walktimer = -1;
		// and call the actual timer function
		mv->walktimer_func(tick);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// main walking function
bool movable::walktimer_func(unsigned long tick)
{
	if( !this->walkpath.finished() &&	// walking not finished
		this->is_movable() &&			// object is movable and can walk on the source tile
		this->can_walk(this->block_list::m,this->block_list::x,this->block_list::y) )
	{	// do a walk step
		int dx,dy;
		dir_t d;
		int x = this->block_list::x;
		int y = this->block_list::y;

		// test the target tile
		for(;;)
		{
			// get the direction to walk to
			d = this->walkpath.get_current_step();
			// get the delta's
			dx = dirx[d];
			dy = diry[d];

			// check if the object can move to the target tile
			if( this->can_walk(this->block_list::m,x+dx,y+dy) )
				// the selected step is valid, so break the loop
				break;

			// otherwise the next target tile is not walkable
			// try to get a new path to the final target 
			// that is different from the current
			if( this->init_walkpath() && d != this->walkpath.get_current_step() )
			{	// and use it
				continue;
			}
			else
			{	// otherwise fail to walk
				clif_fixobject(*this);
				this->walkpath.clear();
				this->walktarget = *this;
				this->do_stop_walking();
				return false;
			}
		}

		// new coordinate
		x += dx;
		y += dy;

		// call object depending move code
		if( this->do_walkstep(tick, coordinate(x,y), dx,dy) )
		{
			// check if crossing a block border
			const bool moveblock = ( this->block_list::x/BLOCK_SIZE != x/BLOCK_SIZE || this->block_list::y/BLOCK_SIZE != y/BLOCK_SIZE);

			// set the object direction
			this->set_dir( d );

			// signal out-of-sight
			this->walktimer=1;
			block_list::foreachinmovearea( CClifOutsight(*this),
				this->block_list::m,this->block_list::x-AREA_SIZE,this->block_list::y-AREA_SIZE,this->block_list::x+AREA_SIZE,this->block_list::y+AREA_SIZE,dx,dy,this->get_sd()?BL_ALL:BL_PC);
			this->walktimer=-1;


			skill_unit_move(*this,tick,0);

			// remove from blocklist when crossing a block border
			if(moveblock) this->delblock();

			// assign the new coordinate
			this->block_list::x = x;
			this->block_list::y = y;

			// reinsert to blocklist when crossing a block border
			if(moveblock) this->addblock();

			skill_unit_move(*this,tick,1);

			// signal in-sight
			this->walktimer=1;
			block_list::foreachinmovearea( CClifInsight(*this),
				this->block_list::m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,-dx,-dy,this->get_sd()?BL_ALL:BL_PC);
			this->walktimer=-1;

			// do object depending stuff at the end of the walk step.
			this->do_walkend();

			// this could be simplified when allowing pathsearch to reuse the current path
			// the change_target member would be obsolete and 
			// this->init_walkpath would be called from this->walktoxy
			// the extra expence is that multiple target changes within a single walk cycle
			// would also do multiple path searches
			if( this->walkpath.change_target )
			{	// build a new path when target has changed
				this->walkpath.change_target=0;
				this->init_walkpath();
			}
			else
			{	// set the next walk position otherwise
				++this->walkpath;
			}

			// set the timer for the next loop
			if( this->set_walktimer(tick) )
				return true;
		}
	}
	// finished walking
	// the normal walking flow will end here
	// when the target is reached

//	clif_fixobject(*this);	
// it might be not necessary to force the client to sync with the current position
// this might cause small walk irregularities when server and client are slightly out of sync

	this->walkpath.clear();
	this->walktarget = *this;
	this->do_stop_walking();

	return false;
}


///////////////////////////////////////////////////////////////////////////////
/// Randomizes target cell.
/// get a random walkable cell that has the same distance from object 
/// as given coordinates do, but randomly choosen. [Skotlex]
bool movable::random_position(coordinate &pos) const
{
	unsigned short xi, yi;
	unsigned short dist = this->get_distance(pos);
	size_t i;
	if (dist < 1) dist =1;
	
	for(i=0; i<100; ++i)
	{
		xi = this->x + rand()%(2*dist) - dist;
		yi = this->y + rand()%(2*dist) - dist;
		if( map_getcell(this->m, xi, yi, CELL_CHKPASS) )
		{
			pos.x = xi;
			pos.y = yi;
			return true;
		}
	}
	// keep the object position, don't move it
	pos.x = this->block_list::x;
	pos.y = this->block_list::y;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// calculate a random walktarget within 5x5 around the target coordiantes
bool movable::random_walktarget(const block_list &target_bl)
{
	dir_t default_dir = target_bl.get_dir();
	int dx,dy,x,y;
	uint32 vary = rand();
	

	default_dir = (dir_t)((default_dir+vary&0x03-1)&0x07);	// vary the default direction
	dx = -dirx[default_dir]*((vary&0x04)!=0)?2:1;
	dy = -diry[default_dir]*((vary&0x08)!=0)?2:1;

	x = target_bl.x + dx;
	y = target_bl.y + dy;
	if( !this->can_reach(x,y) )
	{	// bunch of unnecessary code
/*
		int i,k;
		if(dx > 0) x--;
		else if(dx < 0) x++;
		if(dy > 0) y--;
		else if(dy < 0) y++;
		if( !this->can_reach(x,y) )
		{
			for(i=0;i<12;++i)
			{
				k = rand()&0x07;
				dx = -dirx[k]*2;
				dy = -diry[k]*2;
				x = target_bl.x + dx;
				y = target_bl.y + dy;
				if( this->can_reach(x,y) )
					break;
				else
				{
					if(dx > 0) x--;
					else if(dx < 0) x++;
					if(dy > 0) y--;
					else if(dy < 0) y++;
					if( this->can_reach(x,y) )
						break;
				}
			}
			if(i>=12)
				return false;
		}
*/
		// scheme using a simple multiply/modulo randomizer
		// which covers the all elements without doing multiple checks
		// area is 5x5, so have 25 tiles to check, so the divider is 25.
		// divider prime is then 5, so the multiplier should not be a multiple of 5
		// so calculate (multiplier*i)%25 with i=1..24 for shuffled indices
		// (zero is not beeing shuffled and can be spared)
		div_t d;
		uint i,k;
		uint32 multiplyer = vary;	//rand();	// reuse random value
		if( 0==multiplyer%5 ) ++multiplyer;	// just change

		for(i=1, k=multiplyer; i<25; ++i, k+=multiplyer)
		{	// modulo shuffle
			k = k%25;

			// split delta x and delta y
			d = div(k,5);
			// also move the origin of the 5x5 area from lower left to center by wrapping over
			x = target_bl.x + (d.quot>2)?d.quot-5:d.quot;
			y = target_bl.y + (d.rem >2)?d.rem -5:d.rem;

			if( this->can_reach(x,y) ) // found something usable
				break;
		}
		// no free position found
		if(i>=25)
			return false;
	}

	this->walktarget.x = x;
	this->walktarget.y = y;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// check for reachability, but don't build a path
bool movable::can_reach(unsigned short x, unsigned short y) const
{
	// true when already at this position
	if( this->block_list::x==x && this->block_list::y==y )
		return true;
	// otherwise try to build a path
	return walkpath_data::is_possible(this->block_list::m,this->block_list::x,this->block_list::y, x, y, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// check for reachability with limiting range, but don't build a path
bool movable::can_reach(const block_list &bl, size_t range) const
{
	if( this->block_list::m == bl.m &&
		(range==0 || distance(*this, bl) <= (int)range) )
	{

		if( this->block_list::x==bl.x && 
			this->block_list::y==bl.y )
			return true;

		// Obstacle judging
		if( walkpath_data::is_possible(this->block_list::m, this->block_list::x, this->block_list::y, bl.x, bl.y, 0) )
			return true;

	//////////////
	// possibly unnecessary since when the block itself is not reachable
	// also the surrounding is not reachable
		
		// check the surrounding
		// get the start direction
		const dir_t d = bl.get_direction(*this);

		int i;
		for(i=0; i<8; ++i)
		{	// check all eight surrounding cells by going clockwise beginning with the start direction
			if( walkpath_data::is_possible(this->block_list::m, this->block_list::x, this->block_list::y, bl.x+dirx[(d+i)&0x7], bl.y+diry[(d+i)&0x7], 0) )
				return true;
		}
	//////////////
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////
/// retrive the actual speed of the object
int movable::calc_speed()
{	// current default is using the status functions 
	// which will be split up later into the objects itself
	// and also merging with the seperated calc_pc
	// possibly combine it into set_walktimer 
	// for only doing map position specific speed modifications,
	// the other speed modification should go into the status code anyway
	this->speed = status_recalc_speed(this);
	return this->speed;
}
///////////////////////////////////////////////////////////////////////////////
/// sets the object to idle state
bool movable::set_idle()
{
	this->stop_walking();
	return this->is_idle();
}
///////////////////////////////////////////////////////////////////////////////
///
void movable::set_delay(ulong delay)
{
	this->canmove_tick = delay;
}
///////////////////////////////////////////////////////////////////////////////
///
bool movable::can_walk(unsigned short m, unsigned short x, unsigned short y)
{	// default only checks for non-passable cells
	return !map_getcell(m,x,y,CELL_CHKNOPASS);
}
///////////////////////////////////////////////////////////////////////////////
/// initialize walkpath. uses current target position as walk target
bool movable::init_walkpath()
{
	if( this->walkpath.path_search(this->block_list::m,this->block_list::x,this->block_list::y,this->walktarget.x,this->walktarget.y,this->walkpath.walk_easy) )
	{
		clif_moveobject(*this);
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// activates the walktimer
bool movable::set_walktimer(unsigned long tick)
{
	if( this->walkpath.finished() )
		return false;

	// update the object speed
	unsigned short old_speed = this->speed;
	if( old_speed != this->calc_speed() )
	{	// retransfer the object (only) when the speed changes
		// so the client can sync to the new object speed

		// still need to find out the best way 
		// to force the client to cancel the previous walk packet
		// the current one is a bit heavy, though it's rarely used (at least now)

		clif_fixpos(*this);
		if( this->get_sd() ) clif_updatestatus(*this->get_sd(),SP_SPEED);
		clif_moveobject(*this);
	}

	if( this->walktimer != -1)
	{
		delete_timer(this->walktimer, walktimer_entry);
		//this->walktimer=-1;
	}
	// walking diagonally takes sqrt(2) longer, otherwise take the speed directly
	const size_t sp = (this->walkpath.get_current_step()&1 ) ? this->speed*14/10 : this->speed;
	this->walktimer = add_timer(tick+sp, this->walktimer_entry, this->block_list::id, basics::numptr(this) );
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
bool movable::walktoxy(const coordinate& pos, bool easy)
{
	if( this->is_on_map() )
	{
		if( this->get_sd() )
			this->get_sd();
		//
		// insert code for target position checking here
		//

		this->walktarget = pos;

		if( this->is_confuse() ) //Randomize target direction.
			this->random_position(this->walktarget);

		this->walkpath.walk_easy = easy;
		if( this->walktimer == -1 )
		{
			if( this->init_walkpath() )
			{
				this->set_walktimer( gettick() );
				return true;
			}
		}
		else
		{
			this->walkpath.change_target=1;
			return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
///
bool movable::stop_walking(int type)
{
	if( this->walktimer != -1 )
	{
		delete_timer(this->walktimer, this->walktimer_entry);
		this->walktimer = -1;

		// always send a fixed position to the client when killing the timer
//		if(type&0x01)
		{
			clif_fixpos(*this);
		}

		// move this out
		if( type&0x02 )
		{
			const int delay=status_get_dmotion(this);
			unsigned long tick;
			if( delay && DIFF_TICK(this->canmove_tick,(tick = gettick()))<0 )
				this->canmove_tick = tick + delay;
		}

		this->walkpath.clear();
		this->walktarget = *this;
		this->do_stop_walking();
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
/// instant position change
bool movable::movepos(const coordinate &target)
{
	if( !walkpath_data::is_possible(this->block_list::m,this->block_list::x,this->block_list::y,target.x,target.y,0) )
		return false;

	const unsigned long tick = gettick();
	int dx = target.x - this->block_list::x;
	int dy = target.y - this->block_list::y;

	this->set_idle();
	this->set_dir( direction(*this, target) );

	// call object depending move code
	if( this->do_walkstep(tick, target, dx,dy) )
	{
		bool moveblock = ( this->block_list::x/BLOCK_SIZE != target.x/BLOCK_SIZE || this->block_list::y/BLOCK_SIZE != target.y/BLOCK_SIZE);

		block_list::foreachinmovearea( CClifOutsight(*this),
			this->block_list::m,((int)this->block_list::x)-AREA_SIZE,((int)this->block_list::y)-AREA_SIZE,((int)this->block_list::x)+AREA_SIZE,((int)this->block_list::y)+AREA_SIZE,dx,dy,BL_ALL);

		skill_unit_move(*this,tick,0);
		if(moveblock) this->delblock();
		this->block_list::x = target.x;
		this->block_list::y = target.y;
		if(moveblock) this->addblock();
		skill_unit_move(*this,tick,1);

		block_list::foreachinmovearea( CClifInsight(*this),
			this->block_list::m,((int)this->block_list::x)-AREA_SIZE,((int)this->block_list::y)-AREA_SIZE,((int)this->block_list::x)+AREA_SIZE,((int)this->block_list::y)+AREA_SIZE,-dx,-dy,BL_ALL);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// warps to a given map/position. 
/// In the case of players, pc_setpos is used.
/// it respects the no warp flags, so it is safe to call this without doing nowarpto/nowarp checks.
bool movable::warp(unsigned short m, unsigned short x, unsigned short y, int type)
{
	if( !this->is_on_map() )
		return false;

	if(type < 0 || type == 1)
		// Type 1 is invalid, since you shouldn't warp a bl with the "death" animation
		return false;
	
	if( m>=map_num )
		m = this->block_list::m;
	
	
	if( this->get_sd() ) //Use pc_setpos
	{
		if( maps[this->block_list::m].flag.noteleport )
			return false;
		return pc_setpos(*this->get_sd(), maps[m].mapname, x, y, type);
	}

	if( this->get_md() )
	{
		if( maps[this->block_list::m].flag.monster_noteleport )
			return false;
	}

	clif_clearchar_area(*this, 0);
	this->delblock();

	this->block_list::x = this->walktarget.x = x;
	this->block_list::y = this->walktarget.y = y;
	this->block_list::m = m;

	this->addblock();
	clif_spawn(*this);

	skill_unit_move(*this,gettick(),1);

//	if( this->get_md() )
//	{
//		mob_data* md = this->get_sd();
//		if(md->nd) // Tell the script engine we've warped
//			mob_script_callback(md, NULL, CALLBACK_WARPACK);
//	}
	return 0;
}

