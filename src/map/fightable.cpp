#include "showmsg.h"
#include "timer.h"

#include "map.h"
#include "battle.h"
#include "clif.h"
#include "fightable.h"
#include "mob.h"
#include "npc.h"
#include "pc.h"
#include "pet.h"
#include "skill.h"
#include "status.h"





///////////////////////////////////////////////////////////////////////////////
/// constructor
fightable::fightable() : 
	attacktimer(-1),
	skilltimer(-1),
	attackable_tick(0),
	target_id(0)
{
	add_timer_func_list(this->attacktimer_entry, "attacktimer entry function");
	add_timer_func_list(this->skilltimer_entry, "skilltimer entry function");
}

/// sets the object to idle state
bool fightable::set_idle()
{
	this->stop_attack();
	this->stop_skill();

	return this->movable::set_idle();
}

bool fightable::start_attack(uint32 target_id, bool cont)
{
	struct block_list *bl=map_id2bl(target_id);
	

	if(bl==NULL)
		return false;

	if( bl->type==BL_NPC && this->get_sd() )
	{	// monster npcs [Valaris]
		npc_click(*this->get_sd(),target_id);
		return true;
	}

	if( this->get_sd() && battle_check_target(this->get_sd(), bl, BCT_ENEMY) <= 0)
		return false;

	this->do_attack();

	if( this->attacktimer != -1 )
	{
		delete_timer(this->attacktimer, fightable::attacktimer_entry);
		this->attacktimer = -1;
	}

	this->target_id = target_id;
	this->attack_continue = cont;

	int d=DIFF_TICK(this->attackable_tick,gettick());
	if( d>0 && d<2000 )
	{	// attack with delay
		this->attacktimer=add_timer(this->attackable_tick, fightable::attacktimer_entry, this->block_list::id, 0);
	}
	else
	{	// call it directly
		fightable::attacktimer_entry(-1, gettick(), this->block_list::id, 0);
	}
	return 0;
}

/// stops attack
bool fightable::stop_attack()
{
	if(this->attacktimer!=-1)
	{
		delete_timer(this->attacktimer, fightable::attacktimer_entry);
		this->attacktimer = -1;
	}
	this->target_id=0;
	this->attack_continue=0;
	return true;
}

/// stops skill
bool fightable::stop_skill()
{
	if(this->skilltimer!=-1)
	{
		delete_timer(this->skilltimer, fightable::skilltimer_entry);
		this->skilltimer = -1;
	}
	return true;
}

// old timer entry function
int fightable::attacktimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	block_list* bl = map_id2bl(id);
	fightable* mv;
	if( bl && (mv=bl->get_fightable()) )
	{
		if(mv->attacktimer != tid)
		{
			if(battle_config.error_log)
				ShowError("attacktimer_entry %d != %d\n",mv->attacktimer,tid);
			return 0;
		}
		// timer was executed, clear it
		mv->attacktimer = -1;

		// call the user function
		mv->attacktimer_func(tid, tick, id, data);
	}
	return 0;
}

// old timer entry function
int fightable::skilltimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	// pets are using timer data to transfer the castend_delay object 
	// so we need to send the tid and the data down to the objects
	// to allow cleaning
	// ...at least until the pet mess got cleaned out...

	// also need to seperate between target and ground skills here

	block_list* bl = map_id2bl(id);
	fightable* mv;
	if( bl && (mv=bl->get_fightable()) )
	{
		if(mv->skilltimer != tid)
		{
			if(battle_config.error_log)
				ShowError("skilltimer_entry %d != %d\n",mv->skilltimer,tid);
			return 0;
		}
		// timer was executed, clear it
		mv->skilltimer = -1;

		// call the user function
		mv->skilltimer_func(tid, tick, id, data);
	}

	return 0;
}

