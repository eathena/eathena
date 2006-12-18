#include "basetimerhandler.h"
#include "basearray.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// default timer handler.
/// actually no handler at all, just buffers requests 
/// until a real handler takes over
class CTimerDefaultHandler : public CTimerHandlerBase
{
	smap<CTimerBase*, unsigned long> timermap;
public:
	CTimerDefaultHandler()
	{}
	virtual ~CTimerDefaultHandler()
	{	// not detaching here
		CTimerHandlerBase*& hdl = CTimerHandlerBase::get_handlersingleton();
		if(hdl==this) hdl=NULL;
	}
	/// change handler. called when a new timerhandler is taking over
	virtual void change_handler(CTimerHandlerBase& new_handler)
	{	// put all currently existing timers into the new handler
		smap<CTimerBase*, unsigned long>::iterator iter(timermap);
		for(; iter; ++iter)
		{
			new_handler.start_timer(*iter->key, iter->data);
		}
		timermap.clear();
	}
	/// start a timer within this handler.
	virtual bool start_timer(CTimerBase& timer, unsigned long interval)
	{
		return timermap.insert(&timer,interval);		
	}
	/// stop a timer within this handler.
	virtual bool stop_timer(CTimerBase& timer)
	{
		return timermap.erase(&timer);
	}
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// returns the one and only instanciation of the defaulthandler.
CTimerHandlerBase& CTimerHandlerBase::get_defaulthandler()
{
	static CTimerDefaultHandler timerdefaulthandler;
	return timerdefaulthandler;
}
/// returns the currently active timer handler.
CTimerHandlerBase*& CTimerHandlerBase::get_handlersingleton()
{	
	static CTimerHandlerBase* timerhandler=&CTimerHandlerBase::get_defaulthandler();
	return timerhandler;
}

void CTimerHandlerBase::attach_handler(CTimerHandlerBase& handler)
{
	CTimerHandlerBase*& hdl = CTimerHandlerBase::get_handlersingleton();
	if(hdl)
	{	// there is an active handler
		// call the change_handler function
		hdl->change_handler(handler);
	}
	// take over
	hdl = &handler;
}
void CTimerHandlerBase::detach_handler(CTimerHandlerBase& handler)
{
	CTimerHandlerBase*& hdl = CTimerHandlerBase::get_handlersingleton();
	if( hdl==&handler )
	{	// attach the defaulthandler
		CTimerHandlerBase::attach_handler( CTimerHandlerBase::get_defaulthandler() );
	}
	// wrong handler given
}

///////////////////////////////////////////////////////////////////////////////
/*
///////////////////////////////////////////////////////////////////////////////
class CTimerHandler : public basics::CTimerHandlerBase
{
public:
	CTimerHandler()
	{
		basics::CTimerHandlerBase::attach_handler(*this);
	}
	virtual ~CTimerHandler()
	{
		basics::CTimerHandlerBase::detach_handler(*this);
	}

	/// start a timer within this handler.
	virtual bool start_timer(basics::CTimerBase& timer, unsigned long interval)
	{
		if(interval<1000)
			interval = 1000;
		timer.cTimer = add_timer_interval(gettick()+interval, interval, timercallback, 0, basics::numptr(&timer), false);
		return (timer.cTimer>=0);
	}

	/// stop a timer within this handler.
	virtual bool stop_timer(basics::CTimerBase& timer)
	{
		if(timer.cTimer>0)
		{
			delete_timer(timer.cTimer, timercallback);
			timer.cTimer = -1;
		}
		return true;
	}

	// external calling from external timer implementation
	static int CTimerHandler::timercallback(int timer, unsigned long tick, int id, basics::numptr data)
	{
		if(data.isptr)
		{
			basics::CTimerBase* base = (basics::CTimerBase*)data.ptr;
			if(timer==base->cTimer)
			{
				if( !base->timeruserfunc(tick) )
				{
					delete_timer(base->cTimer, timercallback);
					base->cTimer = -1;
				}
			}
		}
		return 0;
	}
};
*/
///////////////////////////////////////////////////////////////////////////////






NAMESPACE_END(basics)
