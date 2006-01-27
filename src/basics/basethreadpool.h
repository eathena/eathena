#ifndef __BASETHREADPOOL_H__
#define __BASETHREADPOOL_H__

#include "basetypes.h"

#ifndef SINGLETHREAD

#include "basearray.h"
#include "basethreads.h"
#include "basetime.h"



///////////////////////////////////////////////////////////////////////////////
// test function
void test_threadpool(void);



///////////////////////////////////////////////////////////////////////////////
//!! TODO: re-implement the heap/dl-list task objects with the new environment
//!! TODO: find some passthrough for making it runnable in singlethread


///////////////////////////////////////////////////////////////////////////////
// just a fix combination of existing stuff 


///////////////////////////////////////////////////////////////////////////////
// predeclaration
class task;
class task_link;
class intervaltask;
class pooltask;
class threadpool;


///////////////////////////////////////////////////////////////////////////////
// the actual object that links tasks and its execution together
// creation/destruction is automatically via smartpointer
class task_link : public global, public noncopyable
{
private:
	friend class TPtrAutoCount<task_link>;
public:
	// only the smartpointer can create
	task_link():stamp(0)	{}

	~task_link()			{}

	// the link contains a pointer to the refering task object
	// a pointer is safe since the task object will keep track of it
	task* pTask;
	ulong	stamp;

	// time to shedule
	long schedule() const
	{
		return ( (this->stamp) ? ( this->stamp-GetTickCount() ) : (0) ); 
	}
};

///////////////////////////////////////////////////////////////////////////////
// basic task object
class task : public global, public noncopyable
{
	friend class task_link;
	friend class intervaltask;
	friend class pooltask;
	friend class threadpool;
	friend class testtask;

	// the smartpointer to the link object
	TPtrAutoCount<task_link>	pLink;
protected:
	task()	
	{	// link this object with the tasklink
		pLink->pTask=this;
	}
public:
	virtual ~task()	
	{	// unlink this object from the tasklink
		unlink(); 
	}

	// remove from tasklist
	void unlink()
	{	// don't need to unlink if we alone own the object
		if( !pLink.isunique() )
		{	// otherwise remove our reference from the link
			if( pLink->pTask==this )
				pLink->pTask=NULL;
			// and unlink from it
			pLink.clear();
		}
		// create a new and link this object with the tasklink
		pLink->pTask=this;
	}

	// user function
	virtual void function()	=0;

	// user function for reentering the task into timed list
	// is called after the user function and does nothing on default
	virtual ulong reenter()	{ return 0; }

};


///////////////////////////////////////////////////////////////////////////////
class intervaltask : public task
{
	ulong	interval;
public:
	intervaltask(ulong i) : interval(i) {}
	virtual ~intervaltask()			 {}

	// re-insert this task into the execution list
	virtual ulong reenter()	{ return interval; }
};


///////////////////////////////////////////////////////////////////////////////
// class for holding calling tasks inside the pool
class pooltask : public global
{
	friend class threadpool;
	TPtrAutoCount<task_link>	pLink;

	pooltask(task&t)
	{	// link to the object
		t.pLink->stamp = 0;
		this->pLink = t.pLink;
	}
	pooltask(task&t, ulong time)
	{	
		// set time
		t.pLink->stamp = GetTickCount()+time;
		// link to the object
		this->pLink = t.pLink;
	}
public:
	pooltask()
	{ }
	~pooltask()			
	{	// do not destroy the child object
		// just cut the connection to the link
		this->pLink.clear();
	}
	pooltask(const pooltask& p)
	{	// copy the link
		this->pLink = p.pLink;
	}
	pooltask(pooltask& p, ulong time)
	{	// copy the link and set new time
		p.pLink->stamp = GetTickCount()+time;
		this->pLink = p.pLink;
	}

	const pooltask& operator=(const pooltask& p)
	{	// assign the link
		this->pLink = p.pLink;
		return *this;
	}
	void function()	
	{
		if( this->pLink.exists() && this->pLink->pTask )
			this->pLink->pTask->function();
	}
	ulong reenter()
	{
		if( this->pLink.exists() && this->pLink->pTask )
			return this->pLink->pTask->reenter();
		return 0;
	}
	long schedule()	const
	{	
		if( this->pLink.exists() )
			return this->pLink->schedule();
		return 0;
	}

	bool operator ==(const pooltask& p) const	{ return this->pLink == p.pLink; }
	bool operator !=(const pooltask& p) const	{ return this->pLink != p.pLink; }
	bool operator >=(const pooltask& p) const	{ return this->schedule() >= p.schedule(); }
	bool operator <=(const pooltask& p) const	{ return this->schedule() <= p.schedule(); }
	bool operator > (const pooltask& p) const	{ return this->schedule() >  p.schedule(); }
	bool operator < (const pooltask& p) const	{ return this->schedule() <  p.schedule(); }
};



class threadpool: public global, public noncopyable
{

	/////////////////////////////////////////////////////////////
	// predefine
	/////////////////////////////////////////////////////////////
	class worker;
	/////////////////////////////////////////////////////////////
	// handle for shared data between the worker threads
	/////////////////////////////////////////////////////////////
	class WorkerHandle : public global
	{
		/////////////////////////////////////////////////////////
		// counter of used worker threads, for debug purpose only
		unsigned int		cMaxThreadCount;
		unsigned int		cThreadCount;
		unsigned int		cWorkCount;
		unsigned int		cSignal;
		Mutex				mSystem;
		Mutex				mTask;
		TfifoDCT<pooltask>	fTask;
		Mutex				mTimeTask;
		TslistDCT<pooltask>	fTimeTask;
		SemaphoreTimed		fSema;
		Gate				cRunGate;
	public:
		WorkerHandle():cMaxThreadCount(0),cThreadCount(0),cWorkCount(0),cSignal(0)
		{}

		friend class worker;
		friend class threadpool;
	};

	class worker: public thread
	{
		virtual void execute()
		{
			pooltask		localtask;
			long			waittime;
			WorkerHandle&	whandle = *pWorkerHandle;

			
			// we have to insert ourself to the worker count
			atomicincrement(&whandle.cThreadCount);
			

			// check if thread is signaled to finish
			while( !whandle.cSignal )
			{
				// default wait 500ms before rescheduling
				waittime = 500;

				// get the time of the next timed event if exist
				whandle.mTimeTask.lock();
				if( whandle.fTimeTask.top( localtask ) )
				{
					waittime = localtask.schedule();

					// normalize waittime
					if( waittime<50 )			// task is scheduled within the next 50ms or is already overtimed
					{							// process it immediately
						whandle.fTimeTask.pop();// grab it from the list
						waittime=0;
					}
					else if( waittime>500 )		// otherwise we set a maximum of 500ms 
						waittime=500;			// before the next maintainance loop
				}
				whandle.mTimeTask.unlock();

				if( waittime )
				{	// check the untimed tasks
					if( whandle.fSema.wait(waittime) )
					{	// there is some untimed task to do
						whandle.mTask.lock();
						whandle.fTask.pop(localtask);		// pop it from the fifo
						whandle.mTask.unlock();
						waittime = 0;
					}
				}
				if(!waittime)
				{
					atomicincrement(&whandle.cWorkCount);// we start working
					try
					{	
						// call the task function
						localtask.function();
						// call the epilog function
						ulong interval = localtask.reenter();

						if(interval)
						{	// reenter the task
							whandle.mTimeTask.lock();
							whandle.fTimeTask.insert( pooltask(localtask,interval) );
							whandle.mTimeTask.unlock();
						}

					}
					catch(...)
					{	// ignore the exceptions for now
						//!! TODO: add some usefull stuff here
					}
					atomicdecrement(&whandle.cWorkCount);// and finished working
				}
				// check for start/stop rungate
				whandle.cRunGate.wait();

				// maintainance section
				

				whandle.mSystem.lock();

//printf("thread %p of %lu (max %lu), working %lu (task %lu)(timed %lu)\r", this, 
//	   (ulong)whandle.cThreadCount, (ulong)whandle.cMaxThreadCount, (ulong)whandle.cWorkCount,
//	   (ulong)whandle.fTask.size(), (ulong)whandle.fTimeTask.size());


				// if not all allowed threads are running
				if( whandle.cThreadCount < whandle.cMaxThreadCount )
				{	// check if the existing threads are running at almost full capacity
					if( whandle.cThreadCount <= whandle.cMaxThreadCount/2 ||
						whandle.cThreadCount <= (whandle.cWorkCount+1) *4/3 )
					{	// so we create a new thread to help them
						new worker(pWorkerHandle);
					}
				}
				else if( whandle.cThreadCount > whandle.cMaxThreadCount )
				{	// too many threads are running
					// so we finish but not if we are the last one
					// always keep one thread here
					if( whandle.cThreadCount > 1 )
					{
						whandle.mSystem.unlock();
						break;
					}
				}
				whandle.mSystem.unlock();
			}//end while			
			// we have to remove ourself from the worker count
			atomicdecrement(&whandle.cThreadCount);
			
			// go out of scope and die silently
		}
		TPtrAutoCount<WorkerHandle> pWorkerHandle;
	public:
		worker(TPtrAutoCount<WorkerHandle> &wh):thread(true),pWorkerHandle(wh)
		{	// autostart the thread
			start();
		}
		virtual ~worker()	{}
	};
	TPtrAutoCount<WorkerHandle> pWorkerHandle;

public:
	threadpool(size_t threadcount=1, bool autostart = true)
	{
		pWorkerHandle->cMaxThreadCount = (threadcount)?threadcount:1;
		if(autostart) start();
	}
	~threadpool()
	{	// signal the threads to finish immediatly
		// another method than in SocketBase::receiver
		//atomicexchange(&pWorkerHandle->cSignal,1);

		atomicincrement(&pWorkerHandle->cSignal);

		// wait for threads to close
		// should only take 500ms max
		while(1)
		{
			pWorkerHandle->mSystem.lock();
			if( pWorkerHandle->cThreadCount <= 1 )
				break;
			pWorkerHandle->mSystem.unlock();
			sleep(10);
		}
		pWorkerHandle->mSystem.unlock();
	}
	void start()
	{
		if( pWorkerHandle->cThreadCount < 1 )
		{	new worker(pWorkerHandle);
		}
		pWorkerHandle->cRunGate.open();
	}
	void stop()
	{
		pWorkerHandle->cRunGate.close();
	}

	unsigned int&	MaxThreads()	{ return pWorkerHandle->cMaxThreadCount; }

	bool insert(task &t)
	{
		pWorkerHandle->mTask.lock();
		// untimed
		if( pWorkerHandle->fTask.push( pooltask(t) ) )
			pWorkerHandle->fSema.post();
		pWorkerHandle->mTask.unlock();
		return true;
	}
	bool insert(task &t, ulong time)
	{
		pWorkerHandle->mTimeTask.lock();
		// timed
		pWorkerHandle->fTimeTask.insert( pooltask(t,time) );
		pWorkerHandle->mTimeTask.unlock();
		return true;
	}

};

#endif//!SINGLETHREAD


#endif//__BASETHREADPOOL_H__
