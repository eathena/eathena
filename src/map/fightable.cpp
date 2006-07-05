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
	attackable_tick(0)
{
	add_timer_func_list(this->attacktimer_entry, "attacktimer entry function");
	add_timer_func_list(this->skilltimer_entry, "skilltimer entry function");
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
		mv->attacktimer_func_old(tid, tick, id, data);
	}

	return 0;
}

// old timer entry function
int fightable::skilltimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	// pets are using timer data to transfer the castend_delay object 
	// so we need to send the tid and the data down to the objects
	// to allow cleaning


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
		mv->skilltimer_func_old(tid, tick, id, data);
	}

	return 0;
}

