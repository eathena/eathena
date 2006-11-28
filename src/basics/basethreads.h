#ifndef __THREAD_H
#define __THREAD_H
//////////////////////////////////////////////////////////////////////////
/// thread objects
//////////////////////////////////////////////////////////////////////////

#include "basesync.h"
#include "baseobjects.h"

NAMESPACE_BEGIN(basics)


#ifndef SINGLETHREAD

//////////////////////////////////////////////////////////////////////////
/// basic thread object.
/// derived from PTypes (C++ Portable Types Library)
//////////////////////////////////////////////////////////////////////////
class thread: public global
{
protected:
#ifdef WIN32
    unsigned id;
#endif
    pthread_t  handle;
    int  autofree;
    int  running;
    int  signaled;
    int  finished;
    int  freed;
    int  reserved;   // for priorities
    SemaphoreTimed relaxsem;

	//////////////////////////////////////////////////////////////////////
	/// user functions.
	/// only execute is mandatory in derived classes, the others are optional
	//////////////////////////////////////////////////////////////////////
    virtual void startup();
	virtual void execute() = 0;
    virtual void cleanup();
	//////////////////////////////////////////////////////////////////////
	// global thread entry function
	#ifdef WIN32
    static unsigned __stdcall threadentry(void* arg);
	#else
    static void* threadentry(void* arg);
	#endif
	// global thread cleanup entry function
	static void threadepilog(thread* thr);
	//////////////////////////////////////////////////////////////////////

	bool relax(int msecs) { return relaxsem.wait(msecs); }
public:
    thread(bool iautofree);
    virtual ~thread();

	#ifdef WIN32
    pthread_id_t get_id()   { return int(id); }
	#else
    pthread_id_t get_id()   { return handle; }
	#endif

    bool get_running()	{ return running != 0; }
    bool get_finished()	{ return finished != 0; }
    bool get_signaled()	{ return signaled != 0; }
	int	 get_signal()	{ return signaled; }
	
    void start();
    void signal(int value=1);
    void waitfor();
};
//////////////////////////////////////////////////////////////////////////



#endif// !SINGLETHREAD



//////////////////////////////////////////////////////////////////////////
/// unit.
//////////////////////////////////////////////////////////////////////////
class unit : public Mutex
{
#ifndef SINGLETHREAD
	//////////////////////////////////////////////////////////////////////
	// internal thread class for running units asynchronously
	// example for thread usage
	//////////////////////////////////////////////////////////////////////
	class unit_thread: public thread
	{
		virtual void execute()
		{
			if(target) target->do_main();
		}
	protected:
		unit* target;
	public:
		unit_thread(unit* itarget) : thread(false), target(itarget)
		{	// autostarting
			this->start();
		}
		virtual ~unit_thread()
		{	// wait for finish
			this->waitfor();
		}
	};
#endif
protected:
	friend class unit_thread;
	
	int				running;
    unit*			pipe_next;		// next unit in the pipe chain, assigned by connect()
#ifndef SINGLETHREAD
    unit_thread*	main_thread;	// async execution thread, started by run() if necessary
#endif
    void do_main()
	{
		try
		{
			main();
		}
		catch(exception e)
		{
			printf("Error: %s\n", e.what());
		}

		try
		{
			cleanup();
		}
		catch(exception e)
		{
			printf("Error: %s\n", e.what());
		}
	}
public:
    unit()
		: running(0)
		, pipe_next(NULL)
#ifndef SINGLETHREAD
		, main_thread(NULL)
#endif
	{}
	virtual ~unit()
	{
#ifndef SINGLETHREAD
		unit_thread* tmp = atomicexchange<unit_thread>(&this->main_thread, NULL);
		if(tmp) delete tmp;
#endif
	}

    // things that may be overridden in descendant classes
    virtual void main()		// main code, called from run()
	{}
    virtual void cleanup()	// main code cleanup, called from run()
	{}

    // service methods
    void connect(unit* next)
	{
		waitfor();
		//## TODO: add module connection code
	}
    void run(bool async = false)
	{
		if( atomicexchange(&this->running, 1) != 0 )
			return;
#ifndef SINGLETHREAD		
		if( main_thread == NULL )
#endif
		{
			if( this->pipe_next != NULL )
				this->pipe_next->run(true);
#ifndef SINGLETHREAD		
			if( async )
				this->main_thread = new unit_thread(this);
			else
#endif
			{
				this->do_main();
				this->waitfor();
			}
		}
	}
    void waitfor()
	{
		if (this->running == 0)
			return;
#ifndef SINGLETHREAD
		unit_thread* tmp = atomicexchange<unit_thread>(&this->main_thread, NULL);
		if(tmp) delete tmp;
#endif
		unit* next = atomicexchange<unit>(&this->pipe_next, NULL);
		if(next)
		{
			next->waitfor();
		}
		atomicexchange(&this->running, 0);
	}

};

NAMESPACE_END(basics)


#endif//__THREAD_H
