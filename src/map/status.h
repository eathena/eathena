#ifndef _STATUS_H_
#define _STATUS_H_

#include "oldstatus.h"
#include "fightable.h"



///////////////////////////////////////////////////////////////////////////////
// old status change element
struct status_change
{
	int timer;
	basics::numptr val1;
	basics::numptr val2;
	basics::numptr val3;
	basics::numptr val4;
public:
	// default constructor
	status_change() :
		timer(-1),
		val1(),
		val2(),
		val3(),
		val4()
	{ }

	bool is_active() const { return timer==-1; }

	const basics::numptr& value1() const	{ return val1; }
	const basics::numptr& value2() const	{ return val2; }
	const basics::numptr& value3() const	{ return val3; }
	const basics::numptr& value4() const	{ return val4; }

	basics::numptr& value1()	{ return val1; }
	basics::numptr& value2()	{ return val2; }
	basics::numptr& value3()	{ return val3; }
	basics::numptr& value4()	{ return val4; }

	ssize_t integer1() const	{ return val1.integer(); }
	ssize_t integer2() const	{ return val2.integer(); }
	ssize_t integer3() const	{ return val3.integer(); }
	ssize_t integer4() const	{ return val4.integer(); }

	ssize_t& integer1()	{ return val1.integer(); }
	ssize_t& integer2()	{ return val2.integer(); }
	ssize_t& integer3()	{ return val3.integer(); }
	ssize_t& integer4()	{ return val4.integer(); }

	const void* pointer1() const	{ return val1.pointer(); }
	const void* pointer2() const	{ return val2.pointer(); }
	const void* pointer3() const	{ return val3.pointer(); }
	const void* pointer4() const	{ return val4.pointer(); }

	void*& pointer1()	{ return val1.pointer(); }
	void*& pointer2()	{ return val2.pointer(); }
	void*& pointer3()	{ return val3.pointer(); }
	void*& pointer4()	{ return val4.pointer(); }


	// temporary constructs to allow stepwise code trandformations
	const basics::numptr& value(size_t i) const
	{
		switch(i&0x03)
		{
		case 0:		return val1;
		case 1:		return val2;
		case 2:		return val3;
		default:	return val4;
		}
	}
	basics::numptr& value(size_t i)
	{
		switch(i&0x03)
		{
		case 0:		return val1;
		case 1:		return val2;
		case 2:		return val3;
		default:	return val4;
		}
	}
	
	ssize_t integer(size_t i) const		{ return this->value(i).integer(); }
	ssize_t& integer(size_t i)			{ return this->value(i).integer(); }
	const void* pointer(size_t i) const	{ return this->value(i).pointer(); }
	void*& pointer(size_t i)			{ return this->value(i).pointer(); }
};


///////////////////////////////////////////////////////////////////////////////
/// predeclaration
class status_change_if;


///////////////////////////////////////////////////////////////////////////////
/// object that can receive status changes
struct affectable : public fightable
{
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
	basics::smap<uint32, status_change_if*> statusmap;
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


	/// create a new status. or replace an existing
	virtual bool create_status(uint32 status_id);


	/// check if status exists
	virtual bool has_status(status_t status_id) const
	{	// old status layout
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].is_active() );
	//	return this->statusmap.exists(status_id);
	}
	status_change& get_status(status_t status_id)
	{
		return this->sc_data[status_id];
		//	return this->statusmap[status_id];
	}
	const status_change& get_status(status_t status_id) const
	{
		return this->sc_data[status_id];
		//	return this->statusmap[status_id];
	}
	/// remove a status
	virtual bool remove_status(uint32 status_id);
	/// remove a status
	virtual bool remove_status(status_change_if* status);
	/// remove all status changes
	virtual bool remove_status();




	/////////////////////
	//old interface. still in use
	status_change sc_data[MAX_STATUSCHANGE];
};




#endif//_STATUS_H_
