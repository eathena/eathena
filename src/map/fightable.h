// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __FIGHTABLE_H__
#define __FIGHTABLE_H__

#include "movable.h"
#include "skill.h"


///////////////////////////////////////////////////////////////////////////////
/// fightable object.
/// derives from movable (thus can walk) and implements
/// the interface to do physical attacks and skills
struct fightable : public movable
{
public:
	/////////////////////////////////////////////////////////////////
	static fightable* from_blid(uint32 id)
	{
		block_list* bl = block_list::from_blid(id);
		return bl?bl->get_fightable():NULL;
	}

	/////////////////////////////////////////////////////////////////



	int attacktimer;
	int skilltimer;
	unsigned long attackable_tick;
	unsigned long canact_tick;
	uint32 target_id;
	unsigned short target_lv;
	unsigned char attack_continue : 1 ;	// could be part of the attack object


	skillbase*	cSkillObj;

/*
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	struct skill_unit_group *skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	short attacktarget_lv;

	short skillx,skilly;
	short skillid,skilllv;
	int   skilltarget;

	
	unsigned char running : 1;

	unsigned short skillx;
	unsigned short skilly;
	unsigned short skillid;
	unsigned short skilllv;
	uint32   skilltarget;

	struct linkdb_node *skilltimerskill;
	struct linkdb_node *skillunit;
	struct linkdb_node *skilltickset;

	short attacktarget_lv;
	
	unsigned long attackable_tick;
	unsigned long canact_tick;

	unsigned char skillcastcancel : 1 ;	// results from the executed skill object
*/


	///////////////////////////////////////////////////////////////////////////
	/// constructor
	fightable();
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	virtual ~fightable()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual fightable*				get_fightable()			{ return this; }
	virtual const fightable*		get_fightable() const	{ return this; }

	///////////////////////////////////////////////////////////////////////////
	// status functions

	virtual bool is_movable();
	/// checks if this is attackable
	virtual bool is_attackable() const	{ return true; }
	/// checks for attack state
	virtual bool is_attacking() const	{ return (attacktimer!=-1); }
	/// checks for skill state
	virtual bool is_casting() const		{ return (skilltimer!=-1); }

	/// checks for idle state (alive+not sitting+not blocked by skill)
	virtual bool is_idle() const		{ return !is_attacking() && !is_casting() && this->movable::is_idle(); }

	bool can_act() const	{ return DIFF_TICK(gettick(), this->canact_tick)>0; }

	/// sets the object to idle state
	virtual bool set_idle();
	/// sets the object delay
	virtual void set_delay(ulong delaytick);


	/// sets the object to dead state.
	/// force the objects to have an implementation right now, combine it later
	virtual bool set_dead()=0;

	///////////////////////////////////////////////////////////////////////////
	// targeting functions

	/// unlock from current target
	virtual void unlock_target()
	{
		this->target_id=0;
	}
	virtual void unlock_target(unsigned long tick)
	{
		this->target_id=0;
	}

	///////////////////////////////////////////////////////////////////////////
	// attack functions

	/// attacktimer entry point.
	static int attacktimer_entry(int tid, unsigned long tick, int id, basics::numptr data);
	/// call back function for the attacktimer
	virtual int attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data);

	/// starts attack
	virtual bool start_attack(uint32 target_id, bool cont);
	/// starts attack
	virtual bool start_attack(const block_list& target_bl, bool cont);
	/// start attack without changing target
	virtual bool start_attack(int type=0);

	/// stops attack
	virtual bool stop_attack();


	/// object depending check if attack is possible
	virtual bool can_attack(const fightable& target)	{ return false; }
	/// do object depending stuff for attacking
	virtual void do_attack()							{}
	/// get the current attack range
	virtual ushort get_attackrange()					{ return 0; }

	///////////////////////////////////////////////////////////////////////////
	// skill functions

	/// skilltimer entry point.
	static int skilltimer_entry(int tid, unsigned long tick, int id, basics::numptr data);
	static int skilltimer_entry_new(int tid, unsigned long tick, int id, basics::numptr data);

	/// call back function for the skilltimer
	virtual int skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data)=0;


	/// start a skill
	bool start_skill(ushort skillid, ushort skilllv, uint32 targetid);
	bool start_skill(ushort skillid, ushort skilllv, ushort x, ushort y, const char*extra=NULL);
	bool start_skill(ushort skillid, const char*mapname);

	/// stops skill
	virtual bool stop_skill();
};





#endif//__FIGHTABLE_H__
