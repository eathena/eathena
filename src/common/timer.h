// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_TIMER_H_
#define	_TIMER_H_

#include "base.h"


#define BASE_TICK 5
#define DIFF_TICK(a,b) (((int)(a)-(int)(b)))

extern time_t start_time;

// Struct declaration

struct TimerData {
	unsigned long	tick;
	unsigned long	interval;

	struct{
		bool		pt : 1;
	}				type;

	int (*func)(int,unsigned long,int,int);
	int				id;
	int				data;

};

// Function prototype declaration

//unsigned long gettick_nocache(void);
//unsigned long gettick(void);
// just trying without tick_cache
static inline unsigned long gettick_nocache(void)	{return GetTickCount();}
static inline unsigned long gettick(void)			{return GetTickCount();}


int add_timer_p(unsigned long tick, int (*func)(int,unsigned long,int,int),int id, void* pdata);
int add_timer(unsigned long tick, int (*func)(int,unsigned long,int,int),int id, int data);
int add_timer_interval(unsigned long tick,unsigned long interval,int (*func)(int,unsigned long,int,int),int id,int data);
int add_timer_interval_p(unsigned long tick,unsigned long interval, int (*func)(int,unsigned long,int,int),int id,void* pdata);
int delete_timer(size_t tid, int (*func)(int,unsigned long,int,int));

int addtick_timer(size_t tid,unsigned long tick);
struct TimerData *get_timer(size_t tid);

int do_timer(unsigned long tick);



int add_timer_func_list(int (*)(int,unsigned long,int,int),char*);
char* search_timer_func_list(int (*)(int,unsigned long,int,int));


void timer_init(void);
void timer_final(void);

#endif	// _TIMER_H_
