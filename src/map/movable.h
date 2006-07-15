#ifndef __MOVABLE_H__
#define __MOVABLE_H__

#include "coordinate.h"
#include "path.h"


///////////////////////////////////////////////////////////////////////////////
/// moving object on a map
struct movable : public block_list
{
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
	/// upcasting overload.
	virtual movable* get_movable()	{ return this; }



	///////////////////////////////////////////////////////////////////////////
	/// return body direction.
	dir_t get_bodydir() const		{ return (dir_t)this->bodydir; }
	/// alias to body direction.
	dir_t get_dir() const			{ return (dir_t)this->bodydir; }
	/// return head direction.
	dir_t get_headdir() const		{ return (dir_t)this->headdir; }

	///////////////////////////////////////////////////////////////////////////
	/// set directions seperately.
	void set_dir(dir_t b, dir_t h);
	/// set both directions equally.
	void set_dir(dir_t d);
	/// set body direction only.
	void set_bodydir(dir_t d);
	/// set head direction only.
	void set_headdir(dir_t d);

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

	/// internal walk subfunction
	int walktoxy_sub();
	/// walks to a coordinate
	int walktoxy(unsigned short x,unsigned short y,bool easy=false);
	/// interrupts walking
	int stop_walking(int type=1);
	/// do a walk step
	int walk(unsigned long tick);
	/// change object state
	int changestate(int state,int type);
	/// walk to a random target
	/// is ignored by default
	virtual bool randomwalk(unsigned long tick)	{ return false; }


	/// internal walk subfunction
	virtual int walktoxy_sub_old()=0;
	/// walks to a coordinate
	virtual int walktoxy_old(unsigned short x,unsigned short y,bool easy=false)=0;
	/// do a walk step
	virtual int walkstep_old(unsigned long tick)=0;
	/// change object state
	virtual int changestate_old(int state,int type)=0;
	/// interrupts walking
	virtual int stop_walking_old(int type=1)=0;

	/// call back function for the walktimer
	virtual int walktimer_func_old(int tid, unsigned long tick, int id, basics::numptr data)=0;


// new interface




	/// sets the object to idle state
	virtual bool set_idle();

	

	///////////////////////////////////////////////////////////////////////////
	// walking functions

	/// checks if object is movable at all. immobile by default
	// currently not overloaded and only used for mobs
	virtual bool is_movable();

	/// checks if walking on the quested tile is possible
	/// -> move to map class
	virtual bool can_walk(unsigned short m, unsigned short x, unsigned short y);

	/// do object depending stuff for ending the walk.
	virtual void do_stop_walking()	{}
	/// do object depending stuff for the walk step.
	virtual void do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy)	{}
	/// do object depending stuff for the walkto
	virtual void do_walkto()	{}
	/// do object depending stuff for changestate
	// -> possibly remove this at all
	virtual void do_changestate(int state,int type)	{}


	/// walktimer entry point.
	/// call back function for the timer
	static int walktimer_entry(int tid, unsigned long tick, int id, basics::numptr data);

	/// timer function.
	/// called from walktimer_entry
	// possibly clean and compound all related functions, the interface might only need 2 or 3 accesses
	virtual bool walktimer_func(unsigned long tick)
	{	// does standard walking by default
		return this->walkstep(tick);
	}

	/// initialize walkpath. uses current target position as walk target
	bool init_walkpath();
	/// activates the walktimer
	bool set_walktimer(unsigned long tick);
	/// main walking function
	bool walkstep(unsigned long tick);
	/// walk to position
	bool walkto(const coordinate& pos);
	/// walk to xy
	bool walkto(const unsigned short x, const unsigned short y)
	{
		return this->walkto( coordinate(x,y) );
	}
	bool stop_walking_new(int type);
};



#endif//__MOVABLE_H__

