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

	// default constructor
	status_change() :
		timer(-1),
		val1(),
		val2(),
		val3(),
		val4()
	{ }
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
	virtual bool has_status(uint32 status_id) const
	{
		// old status layout
		return (status_id<MAX_STATUSCHANGE) && (this->sc_data[status_id].timer!=-1);
	//	return this->statusmap.exists(status_id);
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
