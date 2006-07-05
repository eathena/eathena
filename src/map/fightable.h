#ifndef __FIGHTABLE_H__
#define __FIGHTABLE_H__

#include "movable.h"



struct fightable : public movable
{
	int attacktimer;
	int skilltimer;
	unsigned long attackable_tick;


	///////////////////////////////////////////////////////////////////////////
	/// constructor
	fightable();
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	virtual ~fightable()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overload.
	virtual fightable*	get_fightable()	{ return this; }


	/// attacktimer entry point.
	static int attacktimer_entry(int tid, unsigned long tick, int id, basics::numptr data);
	/// call back function for the attacktimer
	virtual int attacktimer_func_old(int tid, unsigned long tick, int id, basics::numptr data)=0;

	/// skilltimer entry point.
	static int skilltimer_entry(int tid, unsigned long tick, int id, basics::numptr data);
	/// call back function for the skilltimer
	virtual int skilltimer_func_old(int tid, unsigned long tick, int id, basics::numptr data)=0;



	///////////////////////////////////////////////////////////////////////////
	// status functions

	/// checks for walking state
	virtual bool is_attacking() const	{ return (attacktimer!=-1); }
	/// checks for walking state
	virtual bool is_skilling() const	{ return (skilltimer!=-1); }
};





#endif//__FIGHTABLE_H__
