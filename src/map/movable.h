// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __MOVABLE_H__
#define __MOVABLE_H__

#include "coordinate.h"
#include "path.h"


///////////////////////////////////////////////////////////////////////////////
/// moving object on a map
struct movable : public block_list
{
public:
	/////////////////////////////////////////////////////////////////
	static movable* from_blid(uint32 id)
	{
		block_list* bl = block_list::from_blid(id);
		return bl?bl->get_movable():NULL;
	}

	/////////////////////////////////////////////////////////////////



public:
	walkpath_data walkpath;
	coordinate walktarget;

	unsigned long canmove_tick;
	int walktimer;
private:
	unsigned char bodydir : 3;	
	unsigned char headdir : 3;
	unsigned char _dummy  : 2;	
public:
	unsigned short speed;

	///////////////////////////////////////////////////////////////////////////
	/// constructor
	movable();
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	virtual ~movable()	{}



	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual movable*				get_movable()			{ return this; }
	virtual const movable*			get_movable() const		{ return this; }


	///////////////////////////////////////////////////////////////////////////
	/// return head direction.
	virtual dir_t get_headdir() const		{ return (dir_t)this->headdir; }
	/// return body direction.
	virtual dir_t get_bodydir() const		{ return (dir_t)this->bodydir; }
	/// alias to body direction.
	virtual dir_t get_dir() const			{ return (dir_t)this->bodydir; }

	///////////////////////////////////////////////////////////////////////////
	/// set directions seperately.
	virtual void set_dir(dir_t b, dir_t h);
	/// set both directions equally.
	virtual void set_dir(dir_t d);
	/// set directions to look at target
	virtual void set_dir(const coordinate& to);
	/// set body direction only.
	virtual void set_bodydir(dir_t d);
	/// set head direction only.
	virtual void set_headdir(dir_t d);


	///////////////////////////////////////////////////////////////////////////
	/// get randomized move coordinates with same distance
	bool random_position(unsigned short &x, unsigned short &y) const; // [Skotlex]
	/// get randomized move coordinates with same distance
	bool random_position(coordinate &pos) const
	{
		return random_position(pos.x, pos.y);
	}

	///////////////////////////////////////////////////////////////////////////
	/// check for reachability, but don't build a path
	bool can_reach(unsigned short x, unsigned short y) const;
	/// check for reachability, but don't build a path
	bool can_reach(const coordinate& c) const
	{
		return this->can_reach(c.x, c.y);
	}
	/// check for reachability with limiting range, but don't build a path
	bool can_reach(const block_list &bl, size_t range=0) const;

	///////////////////////////////////////////////////////////////////////////
	/// calculate a position around the target coordiantes
	bool calc_pos(const block_list &target_bl);

	///////////////////////////////////////////////////////////////////////////
	/// get the speed for the next walkstep
	int calc_next_walk_step();
	/// retrive the actual speed of the object
	int get_speed() const { return this->speed; }
	/// (re)-calculate the actual speed of the object.
	/// overload this for the specific objects 
	/// or even better do specific recalcuations only when changes happen
	/// (status_start/end, equip/unequip, walking on specific cells, etc)
	virtual int calc_speed();


	///////////////////////////////////////////////////////////////////////////
	// compound of the old interface with modified timer callback
	// clean up when new interface is debugged

	/// change object state
	int changestate(int state,int type);
	/// walk to a random target
	/// is ignored by default
	virtual bool randomwalk(unsigned long tick)	{ return false; }

	/// sets the object to idle state
	virtual bool set_idle();
	

	///////////////////////////////////////////////////////////////////////////
	// walking functions

	/// checks if object is movable at all.
	virtual bool is_movable()	{ return true; }

	/// checks if walking on the quested tile is possible
	/// -> move to map class
	virtual bool can_walk(unsigned short m, unsigned short x, unsigned short y);

	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking()	{}
	/// do object depending stuff for the walk step.
	virtual bool do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy)	{ return true; }
	/// do object depending stuff at the end of the walk step.
	virtual void do_walkend()	{}
	/// do object depending stuff for the walkto
	virtual void do_walkto()	{}
	/// do object depending stuff for changestate
	// -> possibly remove this at all
	virtual void do_changestate(int state,int type)	{}



	/// walk to position
	virtual bool walktoxy(const coordinate& pos,bool easy=false);
	/// walk to a coordinate
	virtual bool walktoxy(unsigned short x,unsigned short y,bool easy=false)
	{
		return this->walktoxy( coordinate(x,y), easy );
	}
	/// stop walking
	virtual bool stop_walking(int type=1);
	/// instant position change
	virtual bool movepos(const coordinate &target);
	/// instant position change
	virtual bool movepos(unsigned short x,unsigned short y)
	{
		return this->movepos( coordinate(x,y) );
	}
	/// warps to a given map/position. 
	virtual bool warp(unsigned short m, unsigned short x, unsigned short y, int type);


	/// walktimer entry point.
	/// call back function for the timer
	static int walktimer_entry(int tid, unsigned long tick, int id, basics::numptr data);

	/// main walking function
	/// called from walktimer_entry
	virtual bool walktimer_func(unsigned long tick);


	/// initialize walkpath. uses current target position as walk target
	bool init_walkpath();
	/// activates the walktimer
	bool set_walktimer(unsigned long tick);


};



#endif//__MOVABLE_H__

