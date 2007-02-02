#ifndef __BASETIMERHANDLER_H__
#define __BASETIMERHANDLER_H__

//	/// external calling from external timer implementation
//	static int timercallback(int timer, unsigned long tick, int id, numptr data);


#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"

NAMESPACE_BEGIN(basics)

///////////////////////////////////////////////////////////////////////////////
// predeclaration
class CTimerBase;


///////////////////////////////////////////////////////////////////////////////
/// basic class for a timer handler.
class CTimerHandlerBase : public noncopyable
{
protected:
	static CTimerHandlerBase& get_defaulthandler();
	static CTimerHandlerBase*& get_handlersingleton();
public:
	static void attach_handler(CTimerHandlerBase& handler);
	static void detach_handler(CTimerHandlerBase& handler);
	static CTimerHandlerBase* get_current_handler()
	{
		return CTimerHandlerBase::get_handlersingleton();
	}

	virtual ~CTimerHandlerBase()	{}
	/// change handler. called when a new timerhandler is taking over
	virtual void change_handler(CTimerHandlerBase& new_handler)
	{}
	/// start a timer within this handler.
	virtual bool start_timer(CTimerBase& timer, unsigned long interval) =0;
	/// stop a timer within this handler.
	virtual bool stop_timer(CTimerBase& timer) =0;
};


///////////////////////////////////////////////////////////////////////////////
/// basic class for using timers.
/// derived classes define timeruserfunc, 
/// which gets called back
class CTimerBase : public global, public noncopyable
{
	ICL_EMPTY_COPYCONSTRUCTOR(CTimerBase)
protected:
	CTimerBase(unsigned long interval) : cTimer(-1)
	{
		this->start_timer(interval);
	}
	virtual ~CTimerBase()
	{
		this->stop_timer();
	}
	/// initialisationn
	bool start_timer(unsigned long interval)
	{
		CTimerHandlerBase* hdl = CTimerHandlerBase::get_current_handler();
		return (hdl)?hdl->start_timer(*this,interval):false;
	}
	/// finalisation.
	void stop_timer()
	{
		CTimerHandlerBase* hdl = CTimerHandlerBase::get_current_handler();
		if(hdl) hdl->stop_timer(*this);
	}

public:
	/// user function
	virtual bool timeruserfunc(unsigned long tick) =0;
	/// identifier.
	/// for usage inside the timerhandler 
	int cTimer;	
};


NAMESPACE_END(basics)


#endif //__BASETIMERHANDLER_H__
