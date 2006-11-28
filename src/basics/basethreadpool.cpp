#include "basethreadpool.h"


NAMESPACE_BEGIN(basics)


#if defined(DEBUG)
//#ifndef SINGLETHREAD
class testtask : public task
{
public:
	bool timed; 

	testtask() : timed(false)	{}
	~testtask()	{}

	virtual void function()
	{
		if( this->pLink->stamp )
			printf("task %p; processed %ld, scheduled %ld (diff %ld)\n", 
				this, GetTickCount(), this->pLink->stamp, this->pLink->stamp-GetTickCount() ); 
		else
			printf("task %p, processed %ld\n", 
				this, GetTickCount() ); 

		fflush(stdout);
		// block task for some time
		sleep(100);
	}
	virtual ulong reenter()	{ return (rand()%2)?0:10000; }
};
//#endif//SINGLETHREAD
#endif//DEBUG



void test_threadpool(void)
{
#if defined(DEBUG)


	{
		testtask a,b,c;
		testtask *d = new testtask;

		taskqueue tq;

		tq.insert(a);
		tq.insert(b);
		tq.insert(c);

		tq.insert(*d);
		sleep(100);

		delete d;
	}

	{
		//## TODO copy testcases from caldon
		threadpool	tp(2,false);

		testtask tt[500];
		testtask ts[500];

		for(size_t ii=0; ii<500; ++ii)
		{
			tt[ii].timed=true;
			tp.insert( tt[ii], (800+rand()%8192) );
			tp.insert( ts[ii] );
		}

		printf("start 2 threads %ld\n", GetTickCount() );
		fflush(stdout);
		tp.start();

		//sleep(1000);
		ulong tick=GetTickCount()+1000;
		while( GetTickCount() < tick )
			tp.process( tick-GetTickCount() );

		printf("switch to 10 threads %ld\n", GetTickCount() );
		fflush(stdout);
		tp.MaxThreads() = 10;

		//sleep(20000);
		tick=GetTickCount()+20000;
		while( GetTickCount() < tick )
			tp.process( tick-GetTickCount() );

		

		printf("switch to 1 thread %ld\n", GetTickCount() );
		fflush(stdout);
		tp.MaxThreads() = 2;

		//sleep(20000);
		tick=GetTickCount()+20000;
		while( GetTickCount() < tick )
			tp.process( tick-GetTickCount() );
	}
#endif//DEBUG
}


NAMESPACE_END(basics)
