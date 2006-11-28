#include "basetypes.h"
#include "baseexceptions.h"
#include "basesync.h"
#include "basethreads.h"

NAMESPACE_BEGIN(basics)


#ifndef SINGLETHREAD

//////////////////////////////////////////////////////////////////////////
// basic thread class
// derived from PTypes (C++ Portable Types Library)
//////////////////////////////////////////////////////////////////////////
thread::thread(bool iautofree)
	:
#ifdef WIN32
	id(0),
#endif
	handle(0), autofree(iautofree),
	running(0), signaled(0), finished(0), freed(0),
	reserved(0), relaxsem(0)
{}

thread::~thread()
{
#ifdef WIN32
	if (autofree)
		// MSDN states this is not necessary, however, without closing
		// the handle debuggers show an obvious handle leak here
		CloseHandle(handle);
#else
	// though we require non-autofree threads to always call waitfor(),
	// the statement below is provided to cleanup thread resources even
	// if waitfor() was not called.
	if (!autofree && atomicexchange(&freed, 1) == 0)
		pthread_detach(handle);
#endif
}

void thread::signal(int value)// default value=1
{
	if (atomicexchange(&signaled, value) == 0)
		relaxsem.post();
}

void thread::waitfor()
{
	if (atomicexchange(&freed, 1) != 0)
		return;
	if (is_thread(get_id()))
		throw exception("Can not waitfor() on myself");
	if (autofree)
		throw exception("Can not waitfor() on an autofree thread");
#ifdef WIN32
	WaitForSingleObject(handle, INFINITE);
	CloseHandle(handle);
#else
	pthread_join(handle, NULL);
	// detaching after 'join' is not required (or even do harm on some systems)
	// except for HPUX. we don't support HPUX yet.
	// pthread_detach(handle);
#endif
	handle = 0;
}

void thread::start()
{
	if (atomicexchange(&running, 1) == 0)
	{
#ifdef WIN32
		handle = (HANDLE)_beginthreadex(NULL, 0, thread::threadentry, this, 0, &id);
		if (handle == 0)
			throw exception("CreateThread() failed");
#else
		pthread_t temp_handle;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, 
			autofree ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
		if (pthread_create(autofree ? &temp_handle : &handle,
				&attr, thread::threadentry, this) != 0)
			throw exception("pthread_create() failed");
		pthread_attr_destroy(&attr);
#endif
	}
}
//////////////////////////////////////////////////////////////////////////
// default startup & cleanup
//////////////////////////////////////////////////////////////////////////
void thread::startup()
{}
void thread::cleanup()
{}

//////////////////////////////////////////////////////////////////////////
// thread entry function
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
unsigned _stdcall thread::threadentry(void* arg)
{
#else
void* thread::threadentry(void* arg) 
{
#endif
	thread* thr = (thread*)arg;
#ifndef WIN32
	if (thr->autofree)
		// start() does not assign the handle for autofree threads
		thr->handle = pthread_self();
#endif
	try 
	{
		thr->startup();
		thr->execute();
	}
	catch(exception*)
	{
		thread::threadepilog(thr);
		throw;
	}
	thread::threadepilog(thr);
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// thread cleanup entry function
//////////////////////////////////////////////////////////////////////////
void thread::threadepilog(thread* thr)
{
	try
	{
		thr->cleanup();
	}
	catch(exception e)
	{	// failed the cleanup
	}
	atomicexchange(&thr->finished, 1);
	if (thr->autofree)
		delete thr;
}



#endif// !SINGLETHREAD

NAMESPACE_END(basics)
