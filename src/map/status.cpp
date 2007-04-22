#include "status.h"
#include "pc.h"
#include "clif.h"
#include "itemdb.h"



///////////////////////////////////////////////////////////////////////////////
/// status change object.
/// offers the basic functionality for creating/removing status changes
class status_change_if : public basics::global, public basics::noncopyable
{
	ICL_EMPTY_COPYCONSTRUCTOR(status_change_if)
protected:
	/// constructor.
	/// only derived classes can be created
	status_change_if()
	{}
public:
	// destructor.
	virtual ~status_change_if()
	{}

	/// activate a status.
	bool activate(affectable& object)
	{	// call user dependend code
		this->start(object);
		// tell the client to activate
		clif_status_change(object,(ushort)this->status_id(),1);
		// start the timer (if any)
		const ulong tick = this->duration();
		return tick==0 || this->start_timer(object, tick);
	}
	/// deactivate a status.
	void deactivate(affectable& object)
	{	// stop the timer (if any)
		this->stop_timer();
		// tell the client to deactivate
		clif_status_change(object,(ushort)this->status_id(),0);
		// call user dependend code
		this->stop(object);
	}

	
	/// return the id of the current object.
	virtual status_t status_id() const =0;
	/// true when can be saved.
	virtual bool savable() const =0;
	/// true when an active status needs to be removed.
	virtual bool is_invalid(affectable& object) const =0;
	/// return remaining time.
	virtual ulong remaining() const =0;
	/// return status duration.
	virtual ulong duration() const =0;
	/// start the timer for a timed status.
	virtual bool start_timer(affectable& object, ulong tick) =0;
	/// stop the timer for a timed status.
	virtual void stop_timer() =0;

	
	/// executed when starting the status change.
	virtual void start(affectable& object)		{}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)		{}
	/// external action call.
	/// does nothing by default
	virtual basics::numptr action(affectable& object, const basics::numptr& value)		{ return value; }
	/// returns a stored value.
	/// does nothing by default
	virtual basics::numptr value()				{ return basics::numptr(); }

	/// executed when another status has started/stopped.
	/// does recalculation when status changes affect each other
	virtual void restart(affectable& object) =0;
	/// return the tick when autoresume.
	virtual ulong resume(affectable& object) =0;
};

///////////////////////////////////////////////////////////////////////////////
template<status_t IDENTIFIER>
struct status_identifier : public virtual status_change_if
{
	enum { ID = IDENTIFIER };
	virtual status_t status_id() const { return IDENTIFIER; }
};

///////////////////////////////////////////////////////////////////////////////
template<bool Cond>
struct status_savable : public virtual status_change_if
{
	virtual bool savable() const	{ return Cond; }
};

///////////////////////////////////////////////////////////////////////////////
template<bool Cond>
struct status_needrestart : public virtual status_change_if
{
	virtual void restart(affectable& o)	{ this->stop(o); this->start(o); }
};
template<>
struct status_needrestart<false> : public virtual status_change_if
{
	virtual void restart(affectable& o)	{}
};

///////////////////////////////////////////////////////////////////////////////
template<bool Cond>
struct status_timed : public virtual status_change_if
{
	int timerid;

	status_timed() : timerid(-1)
	{}
	virtual ~status_timed()
	{
		this->stop_timer();
	}

	/// return remaining time.
	virtual ulong remaining() const
	{
		TimerData* td;
		return ( this->timerid!=-1 && (td=get_timer(this->timerid)) ) ? td->tick-gettick() : 0;
	}
	/// start the timer.
	virtual bool start_timer(affectable& object, ulong tick)
	{
		return this->timerid != -1 || (tick>100 && -1 != (this->timerid=add_timer(gettick()+tick, status_timed::timer_entry, object.block_list::id, basics::numptr(this))));
	}
	/// stop the timer.
	virtual void stop_timer()
	{
		if(this->timerid!=-1)
			delete_timer(this->timerid, status_timed::timer_entry);
	}
private:
	/// timer entry.
	/// removes the status changes from the affected object
	static int timer_entry(int tid, unsigned long tick, int id, basics::numptr data)
	{
		affectable* mv = affectable::from_blid(id);
		status_timed* ptr = (status_timed*)data.ptr;
		if(ptr)
		{
			if( !mv )
			{
				ptr->timerid=-1;
				delete ptr;
			}
			else if( tid!=ptr->timerid )
			{	
				if(config.error_log)
					printf("statustimerentry %d != %d\n",ptr->timerid,tid);
			}
			else
			{	// test if status is resumable
				ptr->timerid=-1;
				tick = ptr->resume(*mv);
				if( tick )
				{	// status is resumed
					ptr->start_timer(*mv, tick);
				}
				else
				{	// erase the status change
					mv->remove_status(ptr);
				}
			}
		}
		return 0;
	}
};
template<>
struct status_timed<false> : public virtual status_change_if
{
	/// return status duration.
	virtual ulong duration() const			{ return 0; }
	/// return remaining time.
	virtual ulong remaining() const			{ return 0; }
	/// start the timer.
	virtual bool start_timer(affectable& object, ulong tick)	{ return true; }
	/// stop the timer.
	virtual void stop_timer()				{ }
};

///////////////////////////////////////////////////////////////////////////////
template<bool Cond>
struct status_resuming : public virtual status_change_if
{
	// has to be defined manually
};
template<>
struct status_resuming<false> : public virtual status_change_if
{
	/// return status duration between resumes.
	virtual ulong resume(affectable& o)		{ return 0; }
};

///////////////////////////////////////////////////////////////////////////////
template<bool Cond>
struct status_alwaysvalid : public virtual status_change_if
{
};
template<>
struct status_alwaysvalid<true> : public virtual status_change_if
{
	virtual bool is_invalid(affectable& object) const
	{
		return false;
	}
	static bool is_applyable(affectable& object, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{
		return true;
	}
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// implementation of the different status objects



///////////////////////////////////////////////////////////////////////////////
/// sc_provoke.
class sc_provoke
	: public status_identifier<SC_PROVOKE>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_provoke(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_provoke()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return 1000; // 1 sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object._def2.factor() -= (5+5*this->lvl);
		object._batk.factor() += (2+3*this->lvl);
		object._ratk.factor() += (2+3*this->lvl);
		object._latk.factor() += (2+3*this->lvl);
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object._def2.factor() += (5+5*this->lvl);
		object._batk.factor() -= (2+3*this->lvl);
		object._ratk.factor() -= (2+3*this->lvl);
		object._latk.factor() -= (2+3*this->lvl);
	}
};


///////////////////////////////////////////////////////////////////////////////
/// 
class sc_endure
	: public status_identifier<SC_ENDURE>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_endure(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_endure()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (7+this->lvl*3)*1000; // 7+3*lvl sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object._mdef1.addition() += this->lvl;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object._mdef1.addition() -= this->lvl;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_twohandquicken
	: public status_identifier<SC_TWOHANDQUICKEN>
	, public status_savable<true>
	, public status_needrestart<true>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<false>
{
	int lvl;
public:
	sc_twohandquicken(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_twohandquicken()
	{}
	
	/// true when needs to be terminated.
	virtual bool is_invalid(affectable& object) const
	{
		return !this->is_applyable(object,basics::numptr(),basics::numptr(),basics::numptr(),basics::numptr());
	}
	/// true when possible to apply.
	static bool is_applyable(affectable& object, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{
		return	!object.has_status(SC_QUAGMIRE) &&
				!object.has_status(SC_DONTFORGETME) &&
				!object.has_status(SC_DECREASEAGI);
				//&& is_wearing_twohanded_weapon
	}

	/// return status duration.
	virtual ulong duration() const
	{
		return (this->lvl*30)*1000; // 30*lvl sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object._aspd.factor() -= 30; // reduce since 'speed' here is acually 'delay'
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object._aspd.factor() += 30;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_concentrate
	: public status_identifier<SC_CONCENTRATE>
	, public status_savable<true>
	, public status_needrestart<true>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<false>
{
	int lvl;
public:
	sc_concentrate(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_concentrate()
	{}

	/// true when needs to be terminated.
	virtual bool is_invalid(affectable& object) const
	{
		return object.has_status(SC_QUAGMIRE);
	}
	/// true when possible to apply.
	static bool is_applyable(affectable& object, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{
		return !object.has_status(SC_QUAGMIRE);
	}
	/// return status duration.
	virtual ulong duration() const
	{
		return (40+20*this->lvl)*1000; // 40+lvl*20 sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.attributes.agi.factor() += 2+this->lvl;
		object.attributes.dex.factor() += 2+this->lvl;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object.attributes.agi.factor() -= 2+this->lvl;
		object.attributes.dex.factor() -= 2+this->lvl;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_hiding
	: public status_identifier<SC_HIDING>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<true>
	, public status_alwaysvalid<true>
{
	int lvl;
	int counter;
public:
	sc_hiding(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
		, counter(0)
	{}
	virtual ~sc_hiding()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (4+this->lvl)*1000; // 4+lvl sec, then resumed
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.stop_attack();
		// set hiding in object
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		// remove hiding in object
	}
	/// return durations when autoresume.
	virtual ulong resume(affectable& object)
	{
		map_session_data *psd = object.get_sd();
		if( psd )
		{	
			// runs for 30*lvl sec
			// drains sp every 4+lvl sec
			this->counter += this->duration();

			if( this->counter>=this->lvl*30*1000 || psd->status.sp<=0 )
			{	// timeout or run out of sp
				return 0;
			}
			
			--psd->status.sp;
			clif_updatestatus(*psd,SP_SP);
		}
		return (this->counter+(int)this->duration() > this->lvl*30*1000) ? this->lvl*30*1000-this->counter : this->duration();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_cloaking
	: public status_identifier<SC_CLOAKING>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<true>
	, public status_alwaysvalid<true>
{
	int lvl;
	
public:
	sc_cloaking(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())

	{}
	virtual ~sc_cloaking()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (3+this->lvl)*1000; // 3+lvl sec, then resumed
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.stop_attack();

		// set cloak in object

		object._crit.addition() += 100;
		object._speed.factor() += 30 - 3*this->lvl;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		// remove cloak in object

		object._crit.addition() -= 100;
		object._speed.factor() -= 30 - 3*this->lvl;
	}
	/// return time>0 when autoresume.
	virtual ulong resume(affectable& object)
	{
		map_session_data *psd = object.get_sd();
		if( psd )
		{
			if( psd->status.sp > 0 )
			{
				--psd->status.sp;
				clif_updatestatus(*psd,SP_SP);
			}
			else
			{	// run out of sp
				return 0;
			}
		}
		return this->duration();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_encpoison
	: public status_identifier<SC_ENCPOISON>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_encpoison(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_encpoison()
	{}

	/// return status duration.
	virtual ulong duration() const
	{	
		return (15+this->lvl*15)*1000; // 15+ 15*lvl sec
	}
	/// returns a stored value
	virtual basics::numptr value()
	{	// poisening percentage
		return basics::numptr( (int)(3+(this->lvl-1)/2) ); // 3/3/4/4/5/5/6/6/7/7
	}
};


///////////////////////////////////////////////////////////////////////////////
/// 
class sc_poisonreact
	: public status_identifier<SC_POISONREACT>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_poisonreact(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_poisonreact()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return 30*1000; // 30 sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object._batk.factor() += 30*this->lvl;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object._batk.factor() -= 30*this->lvl;
	}
};

///////////////////////////////////////////////////////////////////////////////
///  possibly not implement it as status but as groundeffect
class sc_quagmire
	: public status_identifier<SC_QUAGMIRE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_quagmire(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4) 
		: lvl(v1.integer())
	{}
	virtual ~sc_quagmire()				{}

	/// return status duration.
	virtual ulong duration() const
	{
		return this->lvl*5*1000; // 5*lvl sec
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_angelus
	: public status_identifier<SC_ANGELUS>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_angelus(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)	
		: lvl(v1.integer())
	{}
	virtual ~sc_angelus()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return this->lvl*30*1000; // lvl*30sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.attributes.vit.factor() += 5*this->lvl;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object.attributes.vit.factor() -= 5*this->lvl;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_blessing
	: public status_identifier<SC_BLESSING>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_blessing(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_blessing()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (40+this->lvl*20)*1000; // 40+lvl*20sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.attributes.str.addition() += this->lvl;
		object.attributes.wis.addition() += this->lvl;
		object.attributes.dex.addition() += this->lvl;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object.attributes.str.addition() -= this->lvl;
		object.attributes.wis.addition() -= this->lvl;
		object.attributes.dex.addition() -= this->lvl;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_signumcrucis
	: public status_identifier<SC_SIGNUMCRUCIS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_signumcrucis(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_signumcrucis()
	{}

	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object._def1.factor() -= (10+2*lvl);
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object._def1.factor() += (10+2*lvl);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_increaseagi
	: public status_identifier<SC_INCREASEAGI>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_increaseagi(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_increaseagi()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (40+this->lvl*20)*1000; // 40+lvl*20sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.attributes.agi.addition() += 2+this->lvl;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object.attributes.agi.addition() -= 2+this->lvl;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_decreaseagi
	: public status_identifier<SC_DECREASEAGI>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_decreaseagi(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_decreaseagi()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (30+this->lvl*10)*1000; // 30+lvl*10sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
		object.attributes.agi.addition() -= 2+this->lvl;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
		object.attributes.agi.addition() += 2+this->lvl;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_slowpoison
	: public status_identifier<SC_SLOWPOISON>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_slowpoison(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_slowpoison()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (10*+this->lvl)*1000; // 10*lvl sec
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_impositio
	: public status_identifier<SC_IMPOSITIO>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_impositio(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)	
		: lvl(v1.integer())
	{}
	virtual ~sc_impositio()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return 60*1000; // 60 sec
	}
	/// returns a stored value
	virtual basics::numptr value()
	{	//damage bonus = 5*lvl
		return basics::numptr( 5*this->lvl );
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_suffragium
	: public status_identifier<SC_SUFFRAGIUM>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_suffragium(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_suffragium()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (3-this->lvl)*10*1000; // 10*(3-lvl) sec
	}
	/// returns a stored value
	virtual basics::numptr value()
	{	// casttime reduction by 5*lvl
		return basics::numptr( (lvl>0 && lvl<=6)?100-15*lvl:100 );
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_aspersio
	: public status_identifier<SC_ASPERSIO>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_aspersio(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_aspersio()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (30+30*this->lvl)*1000; // (30+30*lvl) sec
	}
};

///////////////////////////////////////////////////////////////////////////////
///  possibly not implement it as status but as groundeffect
class sc_benedictio
	: public status_identifier<SC_BENEDICTIO>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_benedictio(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_benedictio()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (40*this->lvl)*1000; // 40*lvl sec
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_kyrie
	: public status_identifier<SC_KYRIE>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	char lvl;
	char counter;
public:
	sc_kyrie(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl((char)v1.integer())
		, counter((char)(5+v1.integer()/2))
	{}
	virtual ~sc_kyrie()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return 120*1000; // 120 sec
	}

	/// external action call.
	/// called with the damage of the current hit,
	/// returns the remaining damage
	virtual basics::numptr action(affectable& object, const basics::numptr& value)
	{
		if( --this->counter== 0 )
		{	// max hit count reached
			object.remove_status(this);
		}
		if( (size_t)value.integer()*100 > object._maxhp * (10+2*lvl) )
		{	// damage limit exeeded

			// barrier broken
			object.remove_status(this);
			// return the remaining damage
			return basics::numptr( value.integer()-object._maxhp*(10+2*lvl)/100 );
		}
		// return 0
		return basics::numptr();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_magnificat
	: public status_identifier<SC_MAGNIFICAT>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_magnificat(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_magnificat()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (20*this->lvl)*1000; // 20*lvl sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{
		object._hprecover.factor() += 100;
		object._sprecover.factor() += 100;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{
		object._hprecover.factor() -= 100;
		object._sprecover.factor() -= 100;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_gloria
	: public status_identifier<SC_GLORIA>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_gloria(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_gloria()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return (5+5*this->lvl)*1000; // 5+5*lvl sec
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{
		object.attributes.luk.addition() += 30;
		object.calculate_attributes();
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{
		object.attributes.luk.addition() -= 30;
		object.calculate_attributes();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_aeterna
	: public status_identifier<SC_AETERNA>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
public:
	sc_aeterna(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{}
	virtual ~sc_aeterna()
	{}
	/// return status duration.
	virtual ulong duration() const
	{
		return 600*1000; // 10min
	}
	
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_adrenaline
	: public status_identifier<SC_ADRENALINE>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<false>
{
	int lvl;
public:
	sc_adrenaline(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_adrenaline()
	{}

	/// true when needs to be terminated.
	virtual bool is_invalid(affectable& object) const
	{
		return false;//object is not wearing axe or mace
	}
	/// true when possible to apply.
	static bool is_applyable(affectable& object, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{
		return true;//object is wearing axe or mace
	}
	/// return status duration.
	virtual ulong duration() const
	{
		return lvl*30*1000; // 30sec per level
		//##TODO:
		// add 10% from Hilt Biding
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{
		object._aspd.factor() += 100;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{
		object._aspd.factor() -= 100;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_weaponperfection
	: public status_identifier<SC_WEAPONPERFECTION>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_weaponperfection(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_weaponperfection()
	{}
	/// return status duration.
	virtual ulong duration() const
	{
		return lvl*10*1000; // 10sec per level
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_overthrust
	: public status_identifier<SC_OVERTHRUST>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_overthrust(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_overthrust()
	{}

	/// return status duration.
	virtual ulong duration() const
	{
		return lvl*20*1000; // 20sec per level
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{
		object._batk.factor() += 5*lvl;
		//object.weaponbreak.factor() += 2/1000*lvl;
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{
		object._batk.factor() -= 5*lvl;
		//object.weaponbreak.factor() -= 2/1000*lvl;
	}

};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_maximizepower
	: public status_identifier<SC_MAXIMIZEPOWER>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<true>
	, public status_resuming<true>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_maximizepower(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_maximizepower()
	{}
	/// return status duration.
	virtual ulong duration() const
	{
		return this->lvl*1000; // lvl sec, then resumed
	}
	/// executed when starting the status change.
	virtual void start(affectable& object)
	{	// apply effects
	}
	/// executed when stopping the status change.
	virtual void stop(affectable& object)
	{	// remove effects
	}
	/// return time>0 when autoresume.
	virtual ulong resume(affectable& object)
	{
		map_session_data *psd = object.get_sd();
		if( psd )
		{
			if( psd->status.sp > 0 )
			{
				--psd->status.sp;
				clif_updatestatus(*psd,SP_SP);
			}
			else
			{	// run out of sp
				return 0;
			}
		}
		return this->duration();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_riding
	: public status_identifier<SC_RIDING>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_riding(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_riding()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_falcon
	: public status_identifier<SC_FALCON>
	, public status_savable<true>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_falcon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_falcon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_trickdead
	: public status_identifier<SC_TRICKDEAD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_trickdead(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_trickdead()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_loud
	: public status_identifier<SC_LOUD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_loud(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_loud()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_energycoat
	: public status_identifier<SC_ENERGYCOAT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_energycoat(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_energycoat()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_brokenarmor
	: public status_identifier<SC_BROKNARMOR>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_brokenarmor(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_brokenarmor()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_brokenweapon
	: public status_identifier<SC_BROKNWEAPON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_brokenweapon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_brokenweapon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_hallucination
	: public status_identifier<SC_HALLUCINATION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_hallucination(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_hallucination()
	{}
};


///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 50% icon
class sc_weight50
	: public status_identifier<SC_WEIGHT50>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
public:
	sc_weight50(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{}
	virtual ~sc_weight50()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// SC_WEIGHT50.
/// displays a 90% icon
class sc_weight90
	: public status_identifier<SC_WEIGHT90>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
public:
	sc_weight90(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
	{}
	virtual ~sc_weight90()
	{}

	virtual void stop(affectable& object)
	{	// check for starting the 50% status, 
		// could also be done outside with the weight calc that has to be done anyway
		if( object.is_50overweight() )
			object.create_status(SC_WEIGHT50);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion0
	: public status_identifier<SC_SPEEDPOTION0>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedpotion0(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedpotion0()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion1
	: public status_identifier<SC_SPEEDPOTION1>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedpotion1(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedpotion1()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion2
	: public status_identifier<SC_SPEEDPOTION2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedpotion2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedpotion2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedpotion3
	: public status_identifier<SC_SPEEDPOTION3>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedpotion3(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedpotion3()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedup0
	: public status_identifier<SC_SPEEDUP0>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedup0(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedup0()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_speedup1
	: public status_identifier<SC_SPEEDUP1>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speedup1(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speedup1()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_atkpot
	: public status_identifier<SC_ATKPOT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_atkpot(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_atkpot()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_matkpot
	: public status_identifier<SC_MATKPOT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_matkpot(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_matkpot()
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// 
class sc_wedding
	: public status_identifier<SC_WEDDING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_wedding(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_wedding()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_slowdown
	: public status_identifier<SC_SLOWDOWN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_slowdown(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_slowdown()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_ankle
	: public status_identifier<SC_ANKLE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_ankle(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_ankle()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_keeping
	: public status_identifier<SC_KEEPING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_keeping(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_keeping()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_barrier
	: public status_identifier<SC_BARRIER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_barrier(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_barrier()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stripweapon
	: public status_identifier<SC_STRIPWEAPON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_stripweapon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_stripweapon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stripshield
	: public status_identifier<SC_STRIPSHIELD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_stripshield(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_stripshield()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_striparmor
	: public status_identifier<SC_STRIPARMOR>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_striparmor(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_striparmor()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_striphelm
	: public status_identifier<SC_STRIPHELM>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_striphelm(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_striphelm()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_weapon
	: public status_identifier<SC_CP_WEAPON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_cp_weapon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_cp_weapon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_shield
	: public status_identifier<SC_CP_SHIELD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_cp_shield(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_cp_shield()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_armor
	: public status_identifier<SC_CP_ARMOR>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_cp_armor(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_cp_armor()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cp_helm
	: public status_identifier<SC_CP_HELM>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_cp_helm(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_cp_helm()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autoguard
	: public status_identifier<SC_AUTOGUARD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_autoguard(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_autoguard()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_reflectshield
	: public status_identifier<SC_REFLECTSHIELD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_reflectshield(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_reflectshield()
	{}
};


///////////////////////////////////////////////////////////////////////////////
///
class sc_splasher
	: public status_identifier<SC_SPLASHER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_splasher(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_splasher()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_providence
	: public status_identifier<SC_PROVIDENCE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_providence(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_providence()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_defender
	: public status_identifier<SC_DEFENDER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_defender(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_defender()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_magicrod
	: public status_identifier<SC_MAGICROD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_magicrod(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_magicrod()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spellbreaker
	: public status_identifier<SC_SPELLBREAKER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_spellbreaker(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_spellbreaker()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autospell
	: public status_identifier<SC_AUTOSPELL>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_autospell(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_autospell()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sighttrasher
	: public status_identifier<SC_SIGHTTRASHER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_sighttrasher(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_sighttrasher()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autoberserk
	: public status_identifier<SC_AUTOBERSERK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_autoberserk(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_autoberserk()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spearsquicken
	: public status_identifier<SC_SPEARSQUICKEN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_spearsquicken(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_spearsquicken()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_autocounter
	: public status_identifier<SC_AUTOCOUNTER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_autocounter(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_autocounter()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sight
	: public status_identifier<SC_SIGHT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_sight(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_sight()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_safetywall
	: public status_identifier<SC_SAFETYWALL>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_safetywall(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_safetywall()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_ruwach
	: public status_identifier<SC_RUWACH>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_ruwach(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_ruwach()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_pneuma
	: public status_identifier<SC_PNEUMA>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_pneuma(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_pneuma()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stone
	: public status_identifier<SC_STONE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_stone(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_stone()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_freeze
	: public status_identifier<SC_FREEZE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_freeze(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_freeze()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stun
	: public status_identifier<SC_STUN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_stun(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_stun()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sleep
	: public status_identifier<SC_SLEEP>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_sleep(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_sleep()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_poison
	: public status_identifier<SC_POISON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_poison(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_poison()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_curse
	: public status_identifier<SC_CURSE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_curse(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_curse()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_silence
	: public status_identifier<SC_SILENCE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_silence(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_silence()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_confusion
	: public status_identifier<SC_CONFUSION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_confusion(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_confusion()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_blind
	: public status_identifier<SC_BLIND>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_blind(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_blind()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_divina
	: public status_identifier<SC_DIVINA>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_divina(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_divina()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bleeding
	: public status_identifier<SC_BLEEDING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_bleeding(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_bleeding()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dpoison
	: public status_identifier<SC_DPOISON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_dpoison(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_dpoison()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_extremityfist
	: public status_identifier<SC_EXTREMITYFIST>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_extremityfist(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_extremityfist()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_explosionspirits
	: public status_identifier<SC_EXPLOSIONSPIRITS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_explosionspirits(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_explosionspirits()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_combo
	: public status_identifier<SC_COMBO>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_combo(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_combo()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bladestop_wait
	: public status_identifier<SC_BLADESTOP_WAIT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_bladestop_wait(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_bladestop_wait()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bladestop
	: public status_identifier<SC_BLADESTOP>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_bladestop(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_bladestop()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_flamelauncher
	: public status_identifier<SC_FLAMELAUNCHER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_flamelauncher(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_flamelauncher()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_frostweapon
	: public status_identifier<SC_FROSTWEAPON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_frostweapon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_frostweapon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_lightningloader
	: public status_identifier<SC_LIGHTNINGLOADER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_lightningloader(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_lightningloader()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_seismicweapon
	: public status_identifier<SC_SEISMICWEAPON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_seismicweapon(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_seismicweapon()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_volcano
	: public status_identifier<SC_VOLCANO>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_volcano(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_volcano()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_deluge
	: public status_identifier<SC_DELUGE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_deluge(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_deluge()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_violentgale
	: public status_identifier<SC_VIOLENTGALE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_violentgale(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_violentgale()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_watk_element
	: public status_identifier<SC_WATK_ELEMENT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_watk_element(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_watk_element()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_landprotector
	: public status_identifier<SC_LANDPROTECTOR>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_landprotector(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_landprotector()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_nochat
	: public status_identifier<SC_NOCHAT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_nochat(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_nochat()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_baby
	: public status_identifier<SC_BABY>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_baby(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_baby()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_aurablade
	: public status_identifier<SC_AURABLADE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_aurablade(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_aurablade()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_parrying
	: public status_identifier<SC_PARRYING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_parrying(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_parrying()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_concentration
	: public status_identifier<SC_CONCENTRATION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_concentration(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_concentration()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_tensionrelax
	: public status_identifier<SC_TENSIONRELAX>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_tensionrelax(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_tensionrelax()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_berserk
	: public status_identifier<SC_BERSERK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_berserk(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_berserk()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fury
	: public status_identifier<SC_FURY>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_fury(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_fury()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_gospel
	: public status_identifier<SC_GOSPEL>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_gospel(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_gospel()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_assumptio
	: public status_identifier<SC_ASSUMPTIO>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_assumptio(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_assumptio()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_basilica
	: public status_identifier<SC_BASILICA>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_basilica(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_basilica()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_guildaura
	: public status_identifier<SC_GUILDAURA>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_guildaura(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_guildaura()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_magicpower
	: public status_identifier<SC_MAGICPOWER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_magicpower(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_magicpower()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_edp
	: public status_identifier<SC_EDP>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_edp(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_edp()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_truesight
	: public status_identifier<SC_TRUESIGHT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_truesight(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_truesight()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_windwalk
	: public status_identifier<SC_WINDWALK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_windwalk(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_windwalk()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_meltdown
	: public status_identifier<SC_MELTDOWN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_meltdown(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_meltdown()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_cartboost
	: public status_identifier<SC_CARTBOOST>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_cartboost(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_cartboost()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_chasewalk
	: public status_identifier<SC_CHASEWALK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_chasewalk(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_chasewalk()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_rejectsword
	: public status_identifier<SC_REJECTSWORD>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_rejectsword(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_rejectsword()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_marionette
	: public status_identifier<SC_MARIONETTE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_marionette(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_marionette()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_marionette2
	: public status_identifier<SC_MARIONETTE2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_marionette2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_marionette2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_moonlit
	: public status_identifier<SC_MOONLIT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_moonlit(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_moonlit()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_headcrush
	: public status_identifier<SC_HEADCRUSH>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_headcrush(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_headcrush()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_jointbeat
	: public status_identifier<SC_JOINTBEAT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_jointbeat(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_jointbeat()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_mindbreaker
	: public status_identifier<SC_MINDBREAKER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_mindbreaker(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_mindbreaker()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_memorize
	: public status_identifier<SC_MEMORIZE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_memorize(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_memorize()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fogwall
	: public status_identifier<SC_FOGWALL>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_fogwall(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_fogwall()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_spiderweb
	: public status_identifier<SC_SPIDERWEB>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_spiderweb(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_spiderweb()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_devotion
	: public status_identifier<SC_DEVOTION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_devotion(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_devotion()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_sacrifice
	: public status_identifier<SC_SACRIFICE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_sacrifice(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_sacrifice()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_steelbody
	: public status_identifier<SC_STEELBODY>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_steelbody(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_steelbody()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readystorm
	: public status_identifier<SC_READYSTORM>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_readystorm(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_readystorm()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_stormkick
	: public status_identifier<SC_STORMKICK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_stormkick(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_stormkick()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readydown
	: public status_identifier<SC_READYDOWN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_readydown(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_readydown()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_downkick
	: public status_identifier<SC_DOWNKICK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_downkick(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_downkick()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readycounter
	: public status_identifier<SC_READYCOUNTER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_readycounter(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_readycounter()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_counter
	: public status_identifier<SC_COUNTER>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_counter(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_counter()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_readyturn
	: public status_identifier<SC_READYTURN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_readyturn(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_readyturn()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_turnkick
	: public status_identifier<SC_TURNKICK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_turnkick(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_turnkick()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dodge
	: public status_identifier<SC_DODGE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_dodge(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_dodge()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_run
	: public status_identifier<SC_RUN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_run(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_run()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_adrenaline2
	: public status_identifier<SC_ADRENALINE2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_adrenaline2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_adrenaline2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dancing
	: public status_identifier<SC_DANCING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_dancing(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_dancing()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_lullaby
	: public status_identifier<SC_LULLABY>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_lullaby(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_lullaby()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_richmankim
	: public status_identifier<SC_RICHMANKIM>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_richmankim(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_richmankim()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_eternalchaos
	: public status_identifier<SC_ETERNALCHAOS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_eternalchaos(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_eternalchaos()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_drumbattle
	: public status_identifier<SC_DRUMBATTLE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_drumbattle(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_drumbattle()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_nibelungen
	: public status_identifier<SC_NIBELUNGEN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_nibelungen(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_nibelungen()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_rokisweil
	: public status_identifier<SC_ROKISWEIL>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_rokisweil(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_rokisweil()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_intoabyss
	: public status_identifier<SC_INTOABYSS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_intoabyss(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_intoabyss()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_siegfried
	: public status_identifier<SC_SIEGFRIED>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_siegfried(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_siegfried()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dissonance
	: public status_identifier<SC_DISSONANCE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_dissonance(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_dissonance()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_whistle
	: public status_identifier<SC_WHISTLE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_whistle(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_whistle()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_assncros
	: public status_identifier<SC_ASSNCROS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_assncros(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_assncros()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_poembragi
	: public status_identifier<SC_POEMBRAGI>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_poembragi(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_poembragi()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_appleidun
	: public status_identifier<SC_APPLEIDUN>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_appleidun(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_appleidun()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_uglydance
	: public status_identifier<SC_UGLYDANCE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_uglydance(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_uglydance()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_humming
	: public status_identifier<SC_HUMMING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_humming(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_humming()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_dontforgetme
	: public status_identifier<SC_DONTFORGETME>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_dontforgetme(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_dontforgetme()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fortune
	: public status_identifier<SC_FORTUNE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_fortune(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_fortune()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_service4u
	: public status_identifier<SC_SERVICE4U>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_service4u(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_service4u()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incallstatus
	: public status_identifier<SC_INCALLSTATUS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incallstatus(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incallstatus()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_inchit
	: public status_identifier<SC_INCHIT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_inchit(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_inchit()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incflee
	: public status_identifier<SC_INCFLEE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incflee(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incflee()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmhp2
	: public status_identifier<SC_INCMHP2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incmhp2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incmhp2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmsp2
	: public status_identifier<SC_INCMSP2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incmsp2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incmsp2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incatk2
	: public status_identifier<SC_INCATK2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incatk2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incatk2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incmatk2
	: public status_identifier<SC_INCMATK2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incmatk2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incmatk2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_inchit2
	: public status_identifier<SC_INCHIT2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_inchit2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_inchit2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incflee2
	: public status_identifier<SC_INCFLEE2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incflee2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incflee2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_preserve
	: public status_identifier<SC_PRESERVE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_preserve(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_preserve()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_battleorders
	: public status_identifier<SC_BATTLEORDERS>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_battleorders(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_battleorders()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_regeneration
	: public status_identifier<SC_REGENERATION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_regeneration(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_regeneration()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_doublecast
	: public status_identifier<SC_DOUBLECAST>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_doublecast(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_doublecast()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_gravitation
	: public status_identifier<SC_GRAVITATION>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_gravitation(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_gravitation()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_maxoverthrust
	: public status_identifier<SC_MAXOVERTHRUST>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_maxoverthrust(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_maxoverthrust()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_longing
	: public status_identifier<SC_LONGING>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_longing(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_longing()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_hermode
	: public status_identifier<SC_HERMODE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_hermode(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_hermode()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_tarot
	: public status_identifier<SC_TAROT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_tarot(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_tarot()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incdef2
	: public status_identifier<SC_INCDEF2>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incdef2(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incdef2()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incstr
	: public status_identifier<SC_INCSTR>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incstr(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incstr()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incagi
	: public status_identifier<SC_INCAGI>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incagi(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incagi()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incvit
	: public status_identifier<SC_INCVIT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incvit(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incvit()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incint
	: public status_identifier<SC_INCINT>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incint(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incint()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incdex
	: public status_identifier<SC_INCDEX>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incdex(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incdex()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_incluk
	: public status_identifier<SC_INCLUK>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_incluk(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_incluk()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_suiton
	: public status_identifier<SC_SUITON>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_suiton(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_suiton()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_avoid
	: public status_identifier<SC_AVOID>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_avoid(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_avoid()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_change
	: public status_identifier<SC_CHANGE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_change(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_change()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_defence
	: public status_identifier<SC_DEFENCE>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_defence(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_defence()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_bloodlust
	: public status_identifier<SC_BLOODLUST>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_bloodlust(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_bloodlust()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_fleet
	: public status_identifier<SC_FLEET>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_fleet(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_fleet()
	{}
};

///////////////////////////////////////////////////////////////////////////////
///
class sc_speed
	: public status_identifier<SC_SPEED>
	, public status_savable<false>
	, public status_needrestart<false>
	, public status_timed<false>
	, public status_resuming<false>
	, public status_alwaysvalid<true>
{
	int lvl;
public:
	sc_speed(affectable& o, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
		: lvl(v1.integer())
	{}
	virtual ~sc_speed()
	{}
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// affectable implementation

///////////////////////////////////////////////////////////////////////////////
/// create a new status. or replace an existing
bool affectable::create_status(status_t status_id, const basics::numptr& v1, const basics::numptr& v2, const basics::numptr& v3, const basics::numptr& v4)
{
	status_change_if* status=NULL;

	if( this->has_status(status_id) )
	{	// delete the previous status
		status = this->statusmap[status_id];
		status->deactivate(*this);
		delete status;
		status = NULL;
	}
	// create a new status
	switch(status_id)
	{
	case sc_provoke::ID:			if( sc_provoke::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_provoke(*this,v1,v2,v3,v4); break;
	case sc_endure::ID:				if( sc_endure::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_endure(*this,v1,v2,v3,v4); break;
	case sc_twohandquicken::ID:		if( sc_twohandquicken::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_twohandquicken(*this,v1,v2,v3,v4); break;
	case sc_concentrate::ID:		if( sc_concentrate::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_concentrate(*this,v1,v2,v3,v4); break;
	case sc_hiding::ID:				if( sc_hiding::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_hiding(*this,v1,v2,v3,v4); break;
	case sc_cloaking::ID:			if( sc_cloaking::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cloaking(*this,v1,v2,v3,v4); break;
	case sc_encpoison::ID:			if( sc_encpoison::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_encpoison(*this,v1,v2,v3,v4); break;
	case sc_poisonreact::ID:		if( sc_poisonreact::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_poisonreact(*this,v1,v2,v3,v4); break;
	case sc_quagmire::ID:			if( sc_quagmire::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_quagmire(*this,v1,v2,v3,v4); break;
	case sc_angelus::ID:			if( sc_angelus::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_angelus(*this,v1,v2,v3,v4); break;
	case sc_blessing::ID:			if( sc_blessing::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_blessing(*this,v1,v2,v3,v4); break;
	case sc_signumcrucis::ID:		if( sc_signumcrucis::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_signumcrucis(*this,v1,v2,v3,v4); break;
	case sc_increaseagi::ID:		if( sc_increaseagi::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_increaseagi(*this,v1,v2,v3,v4); break;
	case sc_decreaseagi::ID:		if( sc_decreaseagi::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_decreaseagi(*this,v1,v2,v3,v4); break;
	case sc_slowpoison::ID:			if( sc_slowpoison::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_slowpoison(*this,v1,v2,v3,v4); break;
	case sc_impositio::ID:			if( sc_impositio::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_impositio(*this,v1,v2,v3,v4); break;
	case sc_suffragium::ID:			if( sc_suffragium::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_suffragium(*this,v1,v2,v3,v4); break;
	case sc_aspersio::ID:			if( sc_aspersio::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_aspersio(*this,v1,v2,v3,v4); break;
	case sc_benedictio::ID:			if( sc_benedictio::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_benedictio(*this,v1,v2,v3,v4); break;
	case sc_kyrie::ID:				if( sc_kyrie::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_kyrie(*this,v1,v2,v3,v4); break;
	case sc_magnificat::ID:			if( sc_magnificat::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_magnificat(*this,v1,v2,v3,v4); break;
	case sc_gloria::ID:				if( sc_gloria::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_gloria(*this,v1,v2,v3,v4); break;
	case sc_aeterna::ID:			if( sc_aeterna::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_aeterna(*this,v1,v2,v3,v4); break;
	case sc_adrenaline::ID:			if( sc_adrenaline::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_adrenaline(*this,v1,v2,v3,v4); break;
	case sc_weaponperfection::ID:	if( sc_weaponperfection::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_weaponperfection(*this,v1,v2,v3,v4); break;
	case sc_overthrust::ID:			if( sc_overthrust::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_overthrust(*this,v1,v2,v3,v4); break;
	case sc_maximizepower::ID:		if( sc_maximizepower::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_maximizepower(*this,v1,v2,v3,v4); break;
	case sc_riding::ID:				if( sc_riding::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_riding(*this,v1,v2,v3,v4); break;
	case sc_falcon::ID:				if( sc_falcon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_falcon(*this,v1,v2,v3,v4); break;
	case sc_trickdead::ID:			if( sc_trickdead::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_trickdead(*this,v1,v2,v3,v4); break;
	case sc_loud::ID:				if( sc_loud::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_loud(*this,v1,v2,v3,v4); break;
	case sc_energycoat::ID:			if( sc_energycoat::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_energycoat(*this,v1,v2,v3,v4); break;
	case sc_brokenarmor::ID:		if( sc_brokenarmor::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_brokenarmor(*this,v1,v2,v3,v4); break;
	case sc_brokenweapon::ID:		if( sc_brokenweapon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_brokenweapon(*this,v1,v2,v3,v4); break;
	case sc_hallucination::ID:		if( sc_hallucination::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_hallucination(*this,v1,v2,v3,v4); break;
	case sc_weight50::ID:			if( sc_weight50::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_weight50(*this,v1,v2,v3,v4); break;
	case sc_weight90::ID:			if( sc_weight90::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_weight90(*this,v1,v2,v3,v4); break;
	case sc_speedpotion0::ID:		if( sc_speedpotion0::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedpotion0(*this,v1,v2,v3,v4); break;
	case sc_speedpotion1::ID:		if( sc_speedpotion1::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedpotion1(*this,v1,v2,v3,v4); break;
	case sc_speedpotion2::ID:		if( sc_speedpotion2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedpotion2(*this,v1,v2,v3,v4); break;
	case sc_speedpotion3::ID:		if( sc_speedpotion3::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedpotion3(*this,v1,v2,v3,v4); break;
	case sc_speedup0::ID:			if( sc_speedup0::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedup0(*this,v1,v2,v3,v4); break;
	case sc_speedup1::ID:			if( sc_speedup1::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speedup1(*this,v1,v2,v3,v4); break;
	case sc_atkpot::ID:				if( sc_atkpot::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_atkpot(*this,v1,v2,v3,v4); break;
	case sc_matkpot::ID:			if( sc_matkpot::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_matkpot(*this,v1,v2,v3,v4); break;
	case sc_wedding::ID:			if( sc_wedding::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_wedding(*this,v1,v2,v3,v4); break;
	case sc_slowdown::ID:			if( sc_slowdown::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_slowdown(*this,v1,v2,v3,v4); break;
	case sc_ankle::ID:				if( sc_ankle::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_ankle(*this,v1,v2,v3,v4); break;
	case sc_keeping::ID:			if( sc_keeping::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_keeping(*this,v1,v2,v3,v4); break;
	case sc_barrier::ID:			if( sc_barrier::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_barrier(*this,v1,v2,v3,v4); break;
	case sc_stripweapon::ID:		if( sc_stripweapon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_stripweapon(*this,v1,v2,v3,v4); break;
	case sc_stripshield::ID:		if( sc_stripshield::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_stripshield(*this,v1,v2,v3,v4); break;
	case sc_striparmor::ID:			if( sc_striparmor::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_striparmor(*this,v1,v2,v3,v4); break;
	case sc_striphelm::ID:			if( sc_striphelm::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_striphelm(*this,v1,v2,v3,v4); break;
	case sc_cp_weapon::ID:			if( sc_cp_weapon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cp_weapon(*this,v1,v2,v3,v4); break;
	case sc_cp_shield::ID:			if( sc_cp_shield::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cp_shield(*this,v1,v2,v3,v4); break;
	case sc_cp_armor::ID:			if( sc_cp_armor::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cp_armor(*this,v1,v2,v3,v4); break;
	case sc_cp_helm::ID:			if( sc_cp_helm::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cp_helm(*this,v1,v2,v3,v4); break;
	case sc_autoguard::ID:			if( sc_autoguard::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_autoguard(*this,v1,v2,v3,v4); break;
	case sc_reflectshield::ID:		if( sc_reflectshield::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_reflectshield(*this,v1,v2,v3,v4); break;
	case sc_splasher::ID:			if( sc_splasher::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_splasher(*this,v1,v2,v3,v4); break;
	case sc_providence::ID:			if( sc_providence::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_providence(*this,v1,v2,v3,v4); break;
	case sc_defender::ID:			if( sc_defender::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_defender(*this,v1,v2,v3,v4); break;
	case sc_magicrod::ID:			if( sc_magicrod::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_magicrod(*this,v1,v2,v3,v4); break;
	case sc_spellbreaker::ID:		if( sc_spellbreaker::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_spellbreaker(*this,v1,v2,v3,v4); break;
	case sc_autospell::ID:			if( sc_autospell::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_autospell(*this,v1,v2,v3,v4); break;
	case sc_sighttrasher::ID:		if( sc_sighttrasher::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_sighttrasher(*this,v1,v2,v3,v4); break;
	case sc_autoberserk::ID:		if( sc_autoberserk::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_autoberserk(*this,v1,v2,v3,v4); break;
	case sc_spearsquicken::ID:		if( sc_spearsquicken::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_spearsquicken(*this,v1,v2,v3,v4); break;
	case sc_autocounter::ID:		if( sc_autocounter::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_autocounter(*this,v1,v2,v3,v4); break;
	case sc_sight::ID:				if( sc_sight::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_sight(*this,v1,v2,v3,v4); break;
	case sc_safetywall::ID:			if( sc_safetywall::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_safetywall(*this,v1,v2,v3,v4); break;
	case sc_ruwach::ID:				if( sc_ruwach::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_ruwach(*this,v1,v2,v3,v4); break;
	case sc_pneuma::ID:				if( sc_pneuma::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_pneuma(*this,v1,v2,v3,v4); break;
	case sc_stone::ID:				if( sc_stone::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_stone(*this,v1,v2,v3,v4); break;
	case sc_freeze::ID:				if( sc_freeze::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_freeze(*this,v1,v2,v3,v4); break;
	case sc_stun::ID:				if( sc_stun::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_stun(*this,v1,v2,v3,v4); break;
	case sc_sleep::ID:				if( sc_sleep::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_sleep(*this,v1,v2,v3,v4); break;
	case sc_poison::ID:				if( sc_poison::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_poison(*this,v1,v2,v3,v4); break;
	case sc_curse::ID:				if( sc_curse::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_curse(*this,v1,v2,v3,v4); break;
	case sc_silence::ID:			if( sc_silence::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_silence(*this,v1,v2,v3,v4); break;
	case sc_confusion::ID:			if( sc_confusion::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_confusion(*this,v1,v2,v3,v4); break;
	case sc_blind::ID:				if( sc_blind::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_blind(*this,v1,v2,v3,v4); break;
//	case sc_divina::ID:				if( sc_divina::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_divina(*this,v1,v2,v3,v4); break;
	case sc_bleeding::ID:			if( sc_bleeding::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_bleeding(*this,v1,v2,v3,v4); break;
	case sc_dpoison::ID:			if( sc_dpoison::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_dpoison(*this,v1,v2,v3,v4); break;
	case sc_extremityfist::ID:		if( sc_extremityfist::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_extremityfist(*this,v1,v2,v3,v4); break;
	case sc_explosionspirits::ID:	if( sc_explosionspirits::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_explosionspirits(*this,v1,v2,v3,v4); break;
	case sc_combo::ID:				if( sc_combo::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_combo(*this,v1,v2,v3,v4); break;
	case sc_bladestop_wait::ID:		if( sc_bladestop_wait::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_bladestop_wait(*this,v1,v2,v3,v4); break;
	case sc_bladestop::ID:			if( sc_bladestop::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_bladestop(*this,v1,v2,v3,v4); break;
	case sc_flamelauncher::ID:		if( sc_flamelauncher::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_flamelauncher(*this,v1,v2,v3,v4); break;
	case sc_frostweapon::ID:		if( sc_frostweapon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_frostweapon(*this,v1,v2,v3,v4); break;
	case sc_lightningloader::ID:	if( sc_lightningloader::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_lightningloader(*this,v1,v2,v3,v4); break;
	case sc_seismicweapon::ID:		if( sc_seismicweapon::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_seismicweapon(*this,v1,v2,v3,v4); break;
	case sc_volcano::ID:			if( sc_volcano::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_volcano(*this,v1,v2,v3,v4); break;
	case sc_deluge::ID:				if( sc_deluge::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_deluge(*this,v1,v2,v3,v4); break;
	case sc_violentgale::ID:		if( sc_violentgale::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_violentgale(*this,v1,v2,v3,v4); break;
	case sc_watk_element::ID:		if( sc_watk_element::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_watk_element(*this,v1,v2,v3,v4); break;
	case sc_landprotector::ID:		if( sc_landprotector::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_landprotector(*this,v1,v2,v3,v4); break;
	case sc_nochat::ID:				if( sc_nochat::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_nochat(*this,v1,v2,v3,v4); break;
	case sc_baby::ID:				if( sc_baby::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_baby(*this,v1,v2,v3,v4); break;
	case sc_aurablade::ID:			if( sc_aurablade::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_aurablade(*this,v1,v2,v3,v4); break;
	case sc_parrying::ID:			if( sc_parrying::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_parrying(*this,v1,v2,v3,v4); break;
	case sc_concentration::ID:		if( sc_concentration::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_concentration(*this,v1,v2,v3,v4); break;
	case sc_tensionrelax::ID:		if( sc_tensionrelax::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_tensionrelax(*this,v1,v2,v3,v4); break;
	case sc_berserk::ID:			if( sc_berserk::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_berserk(*this,v1,v2,v3,v4); break;
	case sc_fury::ID:				if( sc_fury::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_fury(*this,v1,v2,v3,v4); break;
	case sc_gospel::ID:				if( sc_gospel::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_gospel(*this,v1,v2,v3,v4); break;
	case sc_assumptio::ID:			if( sc_assumptio::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_assumptio(*this,v1,v2,v3,v4); break;
	case sc_basilica::ID:			if( sc_basilica::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_basilica(*this,v1,v2,v3,v4); break;
	case sc_guildaura::ID:			if( sc_guildaura::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_guildaura(*this,v1,v2,v3,v4); break;
	case sc_magicpower::ID:			if( sc_magicpower::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_magicpower(*this,v1,v2,v3,v4); break;
	case sc_edp::ID:				if( sc_edp::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_edp(*this,v1,v2,v3,v4); break;
	case sc_truesight::ID:			if( sc_truesight::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_truesight(*this,v1,v2,v3,v4); break;
	case sc_windwalk::ID:			if( sc_windwalk::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_windwalk(*this,v1,v2,v3,v4); break;
	case sc_meltdown::ID:			if( sc_meltdown::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_meltdown(*this,v1,v2,v3,v4); break;
	case sc_cartboost::ID:			if( sc_cartboost::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_cartboost(*this,v1,v2,v3,v4); break;
	case sc_chasewalk::ID:			if( sc_chasewalk::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_chasewalk(*this,v1,v2,v3,v4); break;
	case sc_rejectsword::ID:		if( sc_rejectsword::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_rejectsword(*this,v1,v2,v3,v4); break;
	case sc_marionette::ID:			if( sc_marionette::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_marionette(*this,v1,v2,v3,v4); break;
	case sc_marionette2::ID:		if( sc_marionette2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_marionette2(*this,v1,v2,v3,v4); break;
	case sc_moonlit::ID:			if( sc_moonlit::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_moonlit(*this,v1,v2,v3,v4); break;
	case sc_headcrush::ID:			if( sc_headcrush::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_headcrush(*this,v1,v2,v3,v4); break;
	case sc_jointbeat::ID:			if( sc_jointbeat::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_jointbeat(*this,v1,v2,v3,v4); break;
	case sc_mindbreaker::ID:		if( sc_mindbreaker::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_mindbreaker(*this,v1,v2,v3,v4); break;
	case sc_memorize::ID:			if( sc_memorize::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_memorize(*this,v1,v2,v3,v4); break;
	case sc_fogwall::ID:			if( sc_fogwall::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_fogwall(*this,v1,v2,v3,v4); break;
	case sc_spiderweb::ID:			if( sc_spiderweb::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_spiderweb(*this,v1,v2,v3,v4); break;
	case sc_devotion::ID:			if( sc_devotion::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_devotion(*this,v1,v2,v3,v4); break;
	case sc_sacrifice::ID:			if( sc_sacrifice::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_sacrifice(*this,v1,v2,v3,v4); break;
	case sc_steelbody::ID:			if( sc_steelbody::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_steelbody(*this,v1,v2,v3,v4); break;
	case sc_readystorm::ID:			if( sc_readystorm::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_readystorm(*this,v1,v2,v3,v4); break;
	case sc_stormkick::ID:			if( sc_stormkick::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_stormkick(*this,v1,v2,v3,v4); break;
	case sc_readydown::ID:			if( sc_readydown::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_readydown(*this,v1,v2,v3,v4); break;
	case sc_downkick::ID:			if( sc_downkick::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_downkick(*this,v1,v2,v3,v4); break;
	case sc_readycounter::ID:		if( sc_readycounter::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_readycounter(*this,v1,v2,v3,v4); break;
	case sc_counter::ID:			if( sc_counter::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_counter(*this,v1,v2,v3,v4); break;
	case sc_readyturn::ID:			if( sc_readyturn::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_readyturn(*this,v1,v2,v3,v4); break;
	case sc_turnkick::ID:			if( sc_turnkick::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_turnkick(*this,v1,v2,v3,v4); break;
	case sc_dodge::ID:				if( sc_dodge::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_dodge(*this,v1,v2,v3,v4); break;
	case sc_run::ID:				if( sc_run::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_run(*this,v1,v2,v3,v4); break;
	case sc_adrenaline2::ID:		if( sc_adrenaline2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_adrenaline2(*this,v1,v2,v3,v4); break;
	case sc_dancing::ID:			if( sc_dancing::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_dancing(*this,v1,v2,v3,v4); break;
	case sc_lullaby::ID:			if( sc_lullaby::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_lullaby(*this,v1,v2,v3,v4); break;
	case sc_richmankim::ID:			if( sc_richmankim::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_richmankim(*this,v1,v2,v3,v4); break;
	case sc_eternalchaos::ID:		if( sc_eternalchaos::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_eternalchaos(*this,v1,v2,v3,v4); break;
	case sc_drumbattle::ID:			if( sc_drumbattle::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_drumbattle(*this,v1,v2,v3,v4); break;
	case sc_nibelungen::ID:			if( sc_nibelungen::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_nibelungen(*this,v1,v2,v3,v4); break;
	case sc_rokisweil::ID:			if( sc_rokisweil::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_rokisweil(*this,v1,v2,v3,v4); break;
	case sc_intoabyss::ID:			if( sc_intoabyss::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_intoabyss(*this,v1,v2,v3,v4); break;
	case sc_siegfried::ID:			if( sc_siegfried::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_siegfried(*this,v1,v2,v3,v4); break;
	case sc_dissonance::ID:			if( sc_dissonance::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_dissonance(*this,v1,v2,v3,v4); break;
	case sc_whistle::ID:			if( sc_whistle::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_whistle(*this,v1,v2,v3,v4); break;
	case sc_assncros::ID:			if( sc_assncros::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_assncros(*this,v1,v2,v3,v4); break;
	case sc_poembragi::ID:			if( sc_poembragi::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_poembragi(*this,v1,v2,v3,v4); break;
	case sc_appleidun::ID:			if( sc_appleidun::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_appleidun(*this,v1,v2,v3,v4); break;
	case sc_uglydance::ID:			if( sc_uglydance::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_uglydance(*this,v1,v2,v3,v4); break;
	case sc_humming::ID:			if( sc_humming::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_humming(*this,v1,v2,v3,v4); break;
	case sc_dontforgetme::ID:		if( sc_dontforgetme::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_dontforgetme(*this,v1,v2,v3,v4); break;
	case sc_fortune::ID:			if( sc_fortune::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_fortune(*this,v1,v2,v3,v4); break;
	case sc_service4u::ID:			if( sc_service4u::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_service4u(*this,v1,v2,v3,v4); break;
	case sc_incallstatus::ID:		if( sc_incallstatus::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incallstatus(*this,v1,v2,v3,v4); break;
	case sc_inchit::ID:				if( sc_inchit::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_inchit(*this,v1,v2,v3,v4); break;
	case sc_incflee::ID:			if( sc_incflee::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incflee(*this,v1,v2,v3,v4); break;
	case sc_incmhp2::ID:			if( sc_incmhp2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incmhp2(*this,v1,v2,v3,v4); break;
	case sc_incmsp2::ID:			if( sc_incmsp2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incmsp2(*this,v1,v2,v3,v4); break;
	case sc_incatk2::ID:			if( sc_incatk2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incatk2(*this,v1,v2,v3,v4); break;
	case sc_incmatk2::ID:			if( sc_incmatk2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incmatk2(*this,v1,v2,v3,v4); break;
	case sc_inchit2::ID:			if( sc_inchit2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_inchit2(*this,v1,v2,v3,v4); break;
	case sc_incflee2::ID:			if( sc_incflee2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incflee2(*this,v1,v2,v3,v4); break;
	case sc_preserve::ID:			if( sc_preserve::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_preserve(*this,v1,v2,v3,v4); break;
	case sc_battleorders::ID:		if( sc_battleorders::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_battleorders(*this,v1,v2,v3,v4); break;
	case sc_regeneration::ID:		if( sc_regeneration::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_regeneration(*this,v1,v2,v3,v4); break;
	case sc_doublecast::ID:			if( sc_doublecast::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_doublecast(*this,v1,v2,v3,v4); break;
	case sc_gravitation::ID:		if( sc_gravitation::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_gravitation(*this,v1,v2,v3,v4); break;
	case sc_maxoverthrust::ID:		if( sc_maxoverthrust::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_maxoverthrust(*this,v1,v2,v3,v4); break;
	case sc_longing::ID:			if( sc_longing::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_longing(*this,v1,v2,v3,v4); break;
	case sc_hermode::ID:			if( sc_hermode::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_hermode(*this,v1,v2,v3,v4); break;
	case sc_tarot::ID:				if( sc_tarot::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_tarot(*this,v1,v2,v3,v4); break;
	case sc_incdef2::ID:			if( sc_incdef2::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incdef2(*this,v1,v2,v3,v4); break;
	case sc_incstr::ID:				if( sc_incstr::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incstr(*this,v1,v2,v3,v4); break;
	case sc_incagi::ID:				if( sc_incagi::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incagi(*this,v1,v2,v3,v4); break;
	case sc_incvit::ID:				if( sc_incvit::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incvit(*this,v1,v2,v3,v4); break;
	case sc_incint::ID:				if( sc_incint::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incint(*this,v1,v2,v3,v4); break;
	case sc_incdex::ID:				if( sc_incdex::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incdex(*this,v1,v2,v3,v4); break;
	case sc_incluk::ID:				if( sc_incluk::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_incluk(*this,v1,v2,v3,v4); break;
	case sc_suiton::ID:				if( sc_suiton::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_suiton(*this,v1,v2,v3,v4); break;
	case sc_avoid::ID:				if( sc_avoid::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_avoid(*this,v1,v2,v3,v4); break;
	case sc_change::ID:				if( sc_change::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_change(*this,v1,v2,v3,v4); break;
	case sc_defence::ID:			if( sc_defence::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_defence(*this,v1,v2,v3,v4); break;
	case sc_bloodlust::ID:			if( sc_bloodlust::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_bloodlust(*this,v1,v2,v3,v4); break;
	case sc_fleet::ID:				if( sc_fleet::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_fleet(*this,v1,v2,v3,v4); break;
	case sc_speed::ID:				if( sc_speed::is_applyable(*this,v1,v2,v3,v4) ) status = new sc_speed(*this,v1,v2,v3,v4); break;
	default: break;
	}
	if(status)
	{	// status has been created, so activate it
		if( status->activate(*this) )
		{	// store the status
			this->statusmap[status_id] = status;
			// restart the others
			this->restart_status(status_id);
			return true;
		}
		else
		{	// activation failed
			delete status;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status
bool affectable::remove_status(status_t status_id)
{
	status_change_if** ps = this->statusmap.search(status_id);
	if( ps )
	{	// found an entry
		if(*ps)
			this->remove_status(*ps);
		else // paranoia
			this->statusmap.erase(status_id);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// remove all status changes
bool affectable::remove_status()
{	// remove any remaining status change
	statusmap_t::iterator iter(this->statusmap);
	for(; iter; ++iter)
	{
		iter->data->deactivate(*this);
		delete iter->data;
	}
	statusmap.clear();
	return true;
}
///////////////////////////////////////////////////////////////////////////////
/// does interact with the status.
basics::numptr affectable::status_action(status_t status_id, const basics::numptr& value)
{
	status_change_if** ps = this->statusmap.search(status_id);
	if( ps && *ps)
	{	// let the status modify the value
		return (*ps)->action(*this, value);
	}
	// return given value unchanged
	return value;
}
///////////////////////////////////////////////////////////////////////////////
/// returns a stored value
basics::numptr affectable::status_value(status_t status_id)
{
	status_change_if** ps = this->statusmap.search(status_id);
	return (ps && *ps) ? (*ps)->value() : basics::numptr();
}
///////////////////////////////////////////////////////////////////////////////
/// maintainance.
void affectable::restart_status(status_t exept)
{
	// check for invalids
	statusmap_t::iterator iter(this->statusmap);
	for(; iter; ++iter)
	{
		if( iter->key != exept && iter->data->is_invalid(*this) )
		{
			iter->data->deactivate(*this);
			delete iter->data;
			this->statusmap.erase(iter->key);
			// iterator invalidated, reinit
			iter = this->statusmap;
		}	
	}
	// restart all remaining changes
	iter = this->statusmap;
	for(; iter; ++iter)
	{
		if( iter->key != exept )
			iter->data->restart(*this);
	}
}
///////////////////////////////////////////////////////////////////////////////
/// remove a status.
void affectable::remove_status(status_change_if* status)
{
	if(status)
	{
		const status_t id = status->status_id();
		// deactivate
		status->deactivate(*this);
		delete status;
		// remove from storage
		this->statusmap.erase(id);
		// restart the rest
		this->restart_status(id);
	}
}
