// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_TIMER_H_
#define	_TIMER_H_

#include "base.h"


#define BASE_TICK 5
#define DIFF_TICK(a,b) (((ssize_t)(a)-(ssize_t)(b)))


extern time_t start_time;

// 64bit and user save pointer/number sharing structure
typedef struct _intptr
{
	bool	isptr;
	union{
		void *	ptr;
		ssize_t	num;
	};
	_intptr():isptr(false),ptr(NULL)				{}	// clear as default
	explicit _intptr(void*a):isptr(a!=NULL),ptr(a)	{}	// take over the void pointer, block const pointers here to signal the user
	_intptr(ssize_t a):isptr(false),num(a)			{}	// initialisation from numbers !!DONT CAST POINTERS TO INTS!!
} intptr;


// Struct declaration
struct TimerData
{
	unsigned long	tick;
	unsigned long	interval;

	struct{
		bool		pt : 1;
	}				type;

	int (*func)(int,unsigned long,int,intptr);
	int				id;
	intptr			data;
};
// Function prototype declaration

//unsigned long gettick_nocache(void);
//unsigned long gettick(void);
// just trying without tick_cache
static inline unsigned long gettick_nocache(void)	{ return GetTickCount(); }
static inline unsigned long gettick(void)			{ return GetTickCount(); }



int add_timer(unsigned long tick, int (*func)(int,unsigned long,int,intptr),int id, intptr data, bool ownptr=false);
int add_timer_interval(unsigned long tick, unsigned long interval, int (*func)(int,unsigned long,int,intptr), int id, intptr data, bool ownptr=false);
int add_timer(unsigned long tick, int (*func)(int,unsigned long,int,intptr),int id, int data);
int add_timer_interval(unsigned long tick, unsigned long interval, int (*func)(int,unsigned long,int,intptr), int id, int data);



int delete_timer(size_t tid, int (*func)(int,unsigned long,int,intptr));

int addtick_timer(size_t tid,unsigned long tick);
struct TimerData *get_timer(size_t tid);

int do_timer(unsigned long tick);



int add_timer_func_list(int (*)(int,unsigned long,int,intptr),char*);
char* search_timer_func_list(int (*)(int,unsigned long,int,intptr));


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
