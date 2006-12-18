#include "basetypes.h"
#include "baseobjects.h"
#include "basememory.h"
#include "basealgo.h"
#include "basetime.h"
#include "basestring.h"
#include "basesync.h"
#include "baseexceptions.h"

NAMESPACE_BEGIN(basics)

#ifndef SINGLETHREAD

//////////////////////////////////////////////////////////////////////////
// event
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//////////////////////////////////////////////////////////////////////////
// WIN32 event is completely inlined
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
// POSIX event
//////////////////////////////////////////////////////////////////////////
event::event(bool iautoreset, bool istate)
    : state(int(istate)), autoreset(iautoreset)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if(0 != pthread_mutex_init(&mtx, &attr))	throw exception("Trigger failed");
    if(0 != pthread_cond_init(&cond, 0))		throw exception("Trigger failed");
	pthread_mutexattr_destroy(&attr);
}

event::~event()
{
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mtx);
}

void event::wait()
{
    pthread_mutex_lock(&mtx);
    while (state == 0)
        pthread_cond_wait(&cond, &mtx);
    if (autoreset)
	state = 0;
    pthread_mutex_unlock(&mtx);
} 

void event::pulse()
{
    pthread_mutex_lock(&mtx);
    state = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
}

void event::post()
{
    pthread_mutex_lock(&mtx);
    state = 1;
    if (autoreset)
        pthread_cond_signal(&cond);
    else
        pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mtx);
}

void event::reset()
{
    state = 0;
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
#if defined(__MUTEX_RWLOCK__)
#ifdef WIN32
//////////////////////////////////////////////////////////////////////////
// this implementation of the read/write lock is derived
// from Apache Portable Runtime (APR) source, which 
// in turn was originally based on an erroneous (or just
// incomplete?) example in one of the MSDN technical articles.
//////////////////////////////////////////////////////////////////////////
rwlock::rwlock()
    : Mutex(), readcnt(-1), writecnt(0)
{
    reading = CreateEvent(0, true, false, 0);
    finished = CreateEvent(0, false, true, 0);
    if (reading == 0 || finished == 0)
        throw exception("rwlock failed");
}

rwlock::~rwlock()
{
    CloseHandle(reading);
    CloseHandle(finished);
}

void rwlock::rdlock()
{
    if (atomicincrement(&readcnt) == 0) 
    {
        WaitForSingleObject(finished, INFINITE);
        SetEvent(reading);
    }
    WaitForSingleObject(reading, INFINITE);
}

void rwlock::wrlock()
{
    Mutex::enter();
    WaitForSingleObject(finished, INFINITE);
    writecnt++;
}

void rwlock::unlock()
{
    if (writecnt != 0) 
    {
        writecnt--;
        SetEvent(finished);
        Mutex::leave();
    } 
    else if (atomicdecrement(&readcnt) < 0) 
    {
        ResetEvent(reading);
        SetEvent(finished);
    } 
}
//////////////////////////////////////////////////////////////////////////
#  else	  // !defined(WIN32)
//////////////////////////////////////////////////////////////////////////
// for other platforms that lack POSIX rwlock we implement
// the rwlock object using POSIX condvar. the code below
// is based on Sean Burke's algorithm posted in comp.programming.threads.
//////////////////////////////////////////////////////////////////////////

rwlock::rwlock()
    : locks(0), writers(0), readers(0)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (0 != pthread_mutex_init(&mtx, &attr))	throw exception("rwlock failed");
    if (0 != pthread_cond_init(&readcond, 0))	throw exception("rwlock failed");
    if (0 != pthread_cond_init(&writecond, 0))	throw exception("rwlock failed");
	pthread_mutexattr_destroy(&attr);
}

rwlock::~rwlock()
{
    pthread_cond_destroy(&writecond);
    pthread_cond_destroy(&readcond);
    pthread_mutex_destroy(&mtx);
}

void rwlock::rdlock()
{
    pthread_mutex_lock(&mtx);
    readers++;
    while (locks < 0)
		pthread_cond_wait(&readcond, &mtx);
    readers--;
    locks++;
    pthread_mutex_unlock(&mtx);
}

void rwlock::wrlock()
{
	pthread_mutex_lock(&mtx);
    writers++;
    while (locks != 0)
		pthread_cond_wait(&writecond, &mtx);
    locks = -1;
    writers--;
    pthread_mutex_unlock(&mtx);
}

void rwlock::unlock()
{
    pthread_mutex_lock(&mtx);
    if (locks > 0)
    {
		locks--;
		if (locks == 0)
			pthread_cond_signal(&writecond);
    }
    else
    {
		locks = 0;
        if (readers != 0)
		    pthread_cond_broadcast(&readcond);
        else
			pthread_cond_signal(&writecond);
    }
    pthread_mutex_unlock(&mtx);
}
//////////////////////////////////////////////////////////////////////////
#  endif
//////////////////////////////////////////////////////////////////////////
#elif defined(__POSIX_RWLOCK__)
//////////////////////////////////////////////////////////////////////////
// for other systems we declare a fully-inlined rwlock
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////
// Semaphore
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) || defined(__DARWIN__) || defined(__bsdi__)

int _semaphore_dummy_symbol;  // avoid ranlib's warning message

//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////

Semaphore::Semaphore(ulong initvalue) 
{
    if (sem_init(&handle, 0, initvalue) != 0)
        throw exception("Semaphore failed");
}

Semaphore::~Semaphore() 
{
    sem_destroy(&handle);
}

void Semaphore::wait() 
{
    if (sem_wait(&handle) != 0)
        throw exception("Semaphore failed");
}

void Semaphore::post(size_t amount) 
{
    while (amount--)
		if (sem_post(&handle) != 0)
			throw exception("Semaphore failed");
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
// timed Semaphore
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//////////////////////////////////////////////////////////////////////////
// WIN32 Semaphore is timed already
//////////////////////////////////////////////////////////////////////////
SemaphoreTimed::SemaphoreTimed(ulong initvalue)
{
    handle = CreateSemaphore(NULL, initvalue, 65535, NULL);
    if (handle == 0)
        throw exception("Timed Semaphore failed");
}

SemaphoreTimed::~SemaphoreTimed() 
{
    CloseHandle(handle);
}

bool SemaphoreTimed::wait(ulong timeout)
{
    uint r = WaitForSingleObject(handle, timeout);
    if (r == WAIT_FAILED)
        throw exception("Timed Semaphore failed");
    return r != WAIT_TIMEOUT;
}

void SemaphoreTimed::post(size_t amount)
{
    if (ReleaseSemaphore(handle, amount, NULL) == 0)
        throw exception("Timed Semaphore failed");
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
// POSIX timed Semaphore
//////////////////////////////////////////////////////////////////////////
SemaphoreTimed::SemaphoreTimed(ulong initvalue)
    : global(), count(initvalue)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (0 != pthread_mutex_init(&mtx, &attr))	throw exception("Timed Semaphore failed");
    if (0 != pthread_cond_init(&cond, 0))		throw exception("Timed Semaphore failed");
	pthread_mutexattr_destroy(&attr);
}

SemaphoreTimed::~SemaphoreTimed()
{
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mtx);
}

bool SemaphoreTimed::wait(ulong timeout)
{
    pthread_mutex_lock(&mtx);
    while (count <= 0) { 
        if (timeout != ulong(-1))
        {
            timespec abs_ts; 
            timeval cur_tv;
            gettimeofday(&cur_tv, NULL);
            abs_ts.tv_sec = cur_tv.tv_sec + timeout / 1000; 
            abs_ts.tv_nsec = cur_tv.tv_usec * 1000
                + (timeout % 1000) * 1000000;
            int rc = pthread_cond_timedwait(&cond, &mtx, &abs_ts);
            if (rc == ETIMEDOUT) { 
                pthread_mutex_unlock(&mtx);
                return false;
            }
        }
        else
            pthread_cond_wait(&cond, &mtx);
    } 
    count--;
    pthread_mutex_unlock(&mtx);
    return true;
} 

void SemaphoreTimed::post(size_t amount)
{
    pthread_mutex_lock(&mtx);
    count += amount; 
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////



#endif// !SINGLETHREAD


NAMESPACE_END(basics)
