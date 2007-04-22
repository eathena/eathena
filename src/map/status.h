#ifndef _STATUS_H_
#define _STATUS_H_

#include "oldstatus.h"
#include "fightable.h"
#include "baselinequate.h"


///////////////////////////////////////////////////////////////////////////////
// old status change element
struct status_change
{
	friend struct affectable;
private:
	int timerid;
	basics::numptr val1;
	basics::numptr val2;
	basics::numptr val3;
	basics::numptr val4;
public:
	// default constructor
	status_change() :
		timerid(-1),
		val1(),
		val2(),
		val3(),
		val4()
	{ }

	bool is_active() const		{ return timerid!=-1; }
	const int& timer() const	{ return timerid; }
	int& timer()				{ return timerid; }

	const basics::numptr& value1() const	{ return val1; }
	const basics::numptr& value2() const	{ return val2; }
	const basics::numptr& value3() const	{ return val3; }
	const basics::numptr& value4() const	{ return val4; }

	basics::numptr& value1()	{ return val1; }
	basics::numptr& value2()	{ return val2; }
	basics::numptr& value3()	{ return val3; }
	basics::numptr& value4()	{ return val4; }

	void stop_timer()
	{
		if( this->timerid != -1 )
		{
			delete_timer(this->timerid, status_change_timer);
			this->timerid = -1;
		}
	}
	void start_timer(status_t status_id, uint32 blid, ulong tick, basics::numptr data)
	{
		this->stop_timer();
		this->timerid = add_timer(gettick() + tick, status_change_timer, blid, data);
	}
};













///////////////////////////////////////////////////////////////////////////////
/// predeclaration
class status_change_if;




///////////////////////////////////////////////////////////////////////////////
struct stats
{
	basics::addmulvalue<ushort,100>str;
	basics::addmulvalue<ushort,100>agi;
	basics::addmulvalue<ushort,100>vit;
	basics::addmulvalue<ushort,100>wis;
	basics::addmulvalue<ushort,100>dex;
	basics::addmulvalue<ushort,100>luk;
};

///////////////////////////////////////////////////////////////////////////////
/// object that can receive status changes
struct affectable : public fightable
{
/////////////////////////////
	typedef basics::recalculator<affectable, stats> subscript_t;

	uint32 _curhp;
	uint32 _maxhp;

	stats attributes;
	// values which are depending on stats
	basics::addmulvalue<ushort,100> _batk;
	basics::addmulvalue<ushort,100> _ratk;
	basics::addmulvalue<ushort,100> _latk;
	basics::addmulvalue<ushort,100> _matk1;
	basics::addmulvalue<ushort,100> _matk2;
	basics::addmulvalue<ushort,100> _hit;
	basics::addmulvalue<ushort,100> _flee1;
	basics::addmulvalue<ushort,100> _flee2;
	basics::addmulvalue<ushort,100> _def1;
	basics::addmulvalue<ushort,100> _def2;
	basics::addmulvalue<ushort,100> _mdef1;
	basics::addmulvalue<ushort,100> _mdef2;
	basics::addmulvalue<ushort,100> _crit;
	basics::addmulvalue<ushort,100> _aspd;
	basics::addmulvalue<ushort,100> _speed;
	basics::addmulvalue<ushort,100> _hprecover;
	basics::addmulvalue<ushort,100> _sprecover;

	void calculate_attributes()
	{	// whatever
	}
	subscript_t get_attributes()
	{
		return subscript_t(*this,this->attributes,&affectable::calculate_attributes);
	}
/////////////////////////////


public:
	/////////////////////////////////////////////////////////////////
	static affectable* from_blid(uint32 id)
	{
		block_list* bl = block_list::from_blid(id);
		return bl?bl->get_affectable():NULL;
	}

	/////////////////////////////////////////////////////////////////
private:
	friend class status_change_if;
	typedef basics::smap<status_t, status_change_if*> statusmap_t;
	statusmap_t statusmap;
protected:
	affectable()
	{}
public:
	virtual ~affectable()
	{
		this->remove_status();
	}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual affectable*				get_affectable()		{ return this; }
	virtual const affectable*		get_affectable() const	{ return this; }

////////////////
// new status interface
	/// create a new status. or replace an existing
	virtual bool create_status(status_t status_id, const basics::numptr& v1=basics::numptr(), const basics::numptr& v2=basics::numptr(), const basics::numptr& v3=basics::numptr(), const basics::numptr& v4=basics::numptr());
	/// remove a status
	virtual bool remove_status(status_t status_id);
	/// remove all status changes
	virtual bool remove_status();
	/// does interact with the status.
	virtual basics::numptr status_action(status_t status_id, const basics::numptr& value);
	/// returns a stored value
	virtual basics::numptr status_value(status_t status_id);

	/// restart the existing status object.
	/// used for internal maintainance.
	void restart_status(status_t exept);
	/// remove a status.
	/// used for internal maintainance.
	void remove_status(status_change_if* status);

////////////////

////////////////
// shared interface
	/// check if status exists
	virtual bool has_status(status_t status_id) const
	{	// old status layout
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() );
	//	return this->statusmap.exists(status_id);
	}
////////////////


////////////////
// old interface
	virtual basics::numptr& get_statusvalue1(status_t status_id)
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value1():this->block_list::get_statusvalue1(status_id);
	}
	virtual basics::numptr& get_statusvalue2(status_t status_id)
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value2():this->block_list::get_statusvalue2(status_id);
	}
	virtual basics::numptr& get_statusvalue3(status_t status_id)
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value3():this->block_list::get_statusvalue3(status_id);
	}
	virtual basics::numptr& get_statusvalue4(status_t status_id)
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value4():this->block_list::get_statusvalue4(status_id);
	}
	virtual basics::numptr get_statusvalue1(status_t status_id) const
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value1():basics::numptr();
	}
	virtual basics::numptr get_statusvalue2(status_t status_id) const
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value2():basics::numptr();
	}
	virtual basics::numptr get_statusvalue3(status_t status_id) const
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value3():basics::numptr();
	}
	virtual basics::numptr get_statusvalue4(status_t status_id) const
	{	
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() )?this->sc_data[status_id].value4():basics::numptr();
	}
	///////
	unsigned long get_status_remaining(status_t status_id, ulong tick) const
	{
		if( status_id<MAX_STATUSCHANGE && this->sc_data[status_id].is_active() )
		{
			const struct TimerData *td = get_timer(this->sc_data[status_id].timer());
			return td?DIFF_TICK(td->tick, tick):0;
		}
		return 0; 
	}
	void status_stoptimer(status_t status_id)
	{
		if( status_id<MAX_STATUSCHANGE && this->sc_data[status_id].is_active() )
			this->sc_data[status_id].stop_timer();
	}
	void status_starttimer(status_t status_id, ulong tick, basics::numptr data)
	{
		if( status_id<MAX_STATUSCHANGE && this->sc_data[status_id].is_active() )
			this->sc_data[status_id].start_timer(status_id, this->block_list::id, tick, data);
	}
	const int& status_timer(status_t status_id) const
	{	// no checks here
		return this->sc_data[status_id].timer();
	}
	int& status_timer(status_t status_id)
	{	// no checks here
		return this->sc_data[status_id].timer();
	}
	void status_clear()
	{
		size_t i;
		for (i = 0; i < MAX_STATUSCHANGE; ++i)
		{
			this->sc_data[i].timer() = -1;
			this->sc_data[i].value1() = this->sc_data[i].value2() = this->sc_data[i].value3() = this->sc_data[i].value4() = 0;
		}
	}
////////////////


private:
	/////////////////////
	//old interface. still in use
	status_change sc_data[MAX_STATUSCHANGE];
};




#endif//_STATUS_H_
