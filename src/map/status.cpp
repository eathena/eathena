#include "status.h"
#include "clif.h"


///////////////////////////////////////////////////////////////////////////////
/// status change object.
/// offers the basic functionality for creating/removing status changes
class status_change_if : public basics::global, public basics::noncopyable
{
protected:
	affectable& object;	///< the affected object

	/// constructor.
	/// only derived classes can be created
	status_change_if(affectable& o) : object(o)
	{}
public:
	// destructor.
	virtual ~status_change_if()
	{}

	/// return the id of the current object
	/// mandatory on derived classes
	virtual uint32 status_id() const=0;
	/// true when can be saved.
	virtual bool savable() const	{ return false; }
	/// return remaining time.
	virtual ulong remaining() const	{ return 0; }
	/// activate a status.
	void activate()
	{	// tell the client to activate
		clif_status_change(this->object,this->status_id(),1);
		// call user dependend code
		this->start();
	}
	/// activate a status.
	void deactivate()
	{	// tell the client to deactivate
		clif_status_change(this->object,this->status_id(),0);
		// call user dependend code
		this->stop();
	}
	/// executed when starting the status change.
	virtual void start()	{}
	/// executed when another status has started/stopped.
	/// does recalculation when status changes affect each other
	virtual void restart()	{}
	/// executed when stopping the status change.
	virtual void stop()		{}
};

///////////////////////////////////////////////////////////////////////////////
/// specialized status change object.
/// adds functionality for timed removal of status changes
class status_change_timed : public virtual status_change_if
{
	int timerid;
protected:
	/// constructor.
	status_change_timed(affectable& object, ulong tick) : status_change_if(object), timerid(-1)
	{
		this->timerid=add_timer(tick, status_change_timed::timer_entry, object.block_list::id, basics::numptr(this));
	}
public:
	/// destructor.
	virtual ~status_change_timed()
	{
		if(this->timerid!=-1)
			delete_timer(this->timerid, status_change_timed::timer_entry);
	}
	/// return remaining time.
	virtual ulong remaining() const
	{
		TimerData* td;
		return ( timerid!=-1 && (td=get_timer(timerid)) ) ? td->tick-GetTickCount() : 0;
	}
private:
	/// timer entry.
	/// removes the status changes from the affected object
	static int timer_entry(int tid, unsigned long tick, int id, basics::numptr data)
	{
		//affectable* mv = affectable::from_blid(id);
		status_change_timed* ptr = (status_change_timed*)data.ptr;
		if(ptr)
		{
			if( tid!=ptr->timerid )
			{	
				if(config.error_log)
					printf("statustimerentry %d != %d\n",ptr->timerid,tid);
				return 0;
			}
			// erase the status change
			ptr->object.remove_status(ptr);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// implementation of the different status objects


///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 50% icon
class sc_weight50 : public status_change_if
{
public:
	enum { ID=SC_WEIGHT50 };

	sc_weight50(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_weight50()				{}
	virtual uint32 status_id() const	{ return ID; }
};

///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 90% icon
class sc_weight90 : public status_change_if
{
public:
	enum { ID=SC_WEIGHT90 };

	sc_weight90(affectable&	o) : status_change_if(o)	{}
	virtual ~sc_weight90()				{}
	virtual uint32 status_id() const	{ return ID; }
	virtual void stop()
	{	// check for starting the 50% status
		if( this->object.is_50overweight() )
			this->object.create_status(SC_WEIGHT50);
	}
};








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// affectable implementation

///////////////////////////////////////////////////////////////////////////////
/// create a new status. or replace an existing
bool affectable::create_status(uint32 status_id)
{
	status_change_if* status=NULL;
	switch(status_id)
	{
	case sc_weight50::ID:	status = new sc_weight50(*this); break;
	case sc_weight90::ID:	status = new sc_weight90(*this); break;

	}
	if(status)
	{
		status_change_if*& ps = this->statusmap[status->status_id()];
		if( ps )
		{	// remove the previous status
			ps->deactivate();
			delete ps;
		}
		ps = status;
		status->activate();

		// restart all other status changes
		basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
		for(; iter; ++iter)
		{
			if( iter->key != status_id)
				iter->data->restart();
		}
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status
bool affectable::remove_status(uint32 status_id)
{
	status_change_if** ps = this->statusmap.search(status_id);
	if( ps )
		remove_status(*ps);
	return true;
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status
bool affectable::remove_status(status_change_if* status)
{
	if(status)
	{
		status->deactivate();
		this->statusmap.erase(status->status_id());
		delete status;
		// restart all changes
		basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
		for(; iter; ++iter)
		{
			iter->data->restart();
		}
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// remove all status changes
bool affectable::remove_status()
{	// remove any remaining status change
	basics::smap<uint32, status_change_if*>::iterator iter(this->statusmap);
	for(; iter; ++iter)
	{
		iter->data->deactivate();
		delete iter->data;
	}
	statusmap.clear();
	return true;
}
