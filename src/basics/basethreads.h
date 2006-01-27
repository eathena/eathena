#ifndef __THREAD_H
#define __THREAD_H
//////////////////////////////////////////////////////////////////////////
// thread objects
//////////////////////////////////////////////////////////////////////////

#include "basesync.h"
#include "baseobjects.h"


#ifndef SINGLETHREAD

//////////////////////////////////////////////////////////////////////////
// basic thread object
// derived from PTypes (C++ Portable Types Library)
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
	// user functions
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


#endif//!SINGLETHREAD

#endif//__THREAD_H
