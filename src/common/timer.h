// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_TIMER_H_
#define	_TIMER_H_

#include "basetypes.h"
#include "baseobjects.h"
#include "basetime.h"

#define BASE_TICK 5
#define DIFF_TICK(a,b) (((long)(a)-(long)(b)))


extern time_t start_time;

// timer function
typedef int (*timerfunction)(int,unsigned long,int,basics::numptr);

// Struct declaration
struct TimerData
{
	unsigned long	tick;
	unsigned long	interval;

	struct _type
	{
		bool		pt : 1;
		_type() : pt(false)	{}
	}				type;

	timerfunction	func;
	int				id;
	basics::numptr	data;

	TimerData() : 	
		tick(0),
		interval(0),
		func(NULL),
		id(0)
	{}
};
// Function prototype declaration

//unsigned long gettick_nocache(void);
//unsigned long gettick(void);
// just trying without tick_cache
static inline unsigned long gettick_nocache(void)	{ return GetTickCount(); }
static inline unsigned long gettick(void)			{ return GetTickCount(); }



int add_timer(unsigned long tick, timerfunction func,int id, const basics::numptr& data, bool ownptr=false);
int add_timer_interval(unsigned long tick, unsigned long interval, timerfunction func, int id, const basics::numptr& data, bool ownptr=false);
int add_timer(unsigned long tick, timerfunction func,int id, int data);
int add_timer_interval(unsigned long tick, unsigned long interval, timerfunction func, int id, int data);



int delete_timer(size_t tid, timerfunction func);

int addtick_timer(size_t tid,unsigned long tick);
struct TimerData *get_timer(size_t tid);

int do_timer(unsigned long tick);



int add_timer_func_list(timerfunction func, const char* name);
const char* search_timer_func_list(timerfunction func);


void timer_init(void);
void timer_final(void);






class CTimer
{
	class CTimerData
	{
		unsigned long	tick;
		unsigned long	interval;
	};

	class TTimerData0 : public CTimerData
	{
		int (*func)(int, unsigned long);
	};

	template <class T> class TTimerData1 : public CTimerData
	{
		int (*func)(int, unsigned long, T&);
		T	data1;
	};

	template <class T1, class T2> class TTimerData2 : public CTimerData
	{
		int (*func)(int, unsigned long, T1&, T1&);
		T1	data1;
		T2	data2;
	};




};






#endif	// _TIMER_H_
