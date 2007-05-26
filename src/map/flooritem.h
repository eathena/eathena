// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _FLOORITEM_H_
#define _FLOORITEM_H_

#include "mapobj.h"
#include "pc.h"

///////////////////////////////////////////////////////////////////////////////
class flooritem_data : public block_list
{
public:

	/////////////////////////////////////////////////////////////////
	static flooritem_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_fd():NULL;
	}
	/////////////////////////////////////////////////////////////////


	unsigned char subx;
	unsigned char suby;
	int clear_tid;
	uint32 first_get_id;
	uint32 second_get_id;
	uint32 third_get_id;
	uint32 first_get_tick;
	uint32 second_get_tick;
	uint32 third_get_tick;
	struct item item_data;

	flooritem_data() :
		subx(0), suby(0), clear_tid(-1),
		first_get_id(0), second_get_id(0), third_get_id(0),
		first_get_tick(0), second_get_tick(0), third_get_tick(0)
	{ }
	virtual ~flooritem_data();

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_ITEM);
	}
	virtual flooritem_data*			get_fd()				{ return this; }
	virtual const flooritem_data*	get_fd() const			{ return this; }

	static int create(const struct item &item_data, unsigned short amount, unsigned short m, unsigned short x, unsigned short y, const block_list* first_sd, const block_list* second_sd, const block_list* third_sd, int type);


	static int clear_timer(int tid, unsigned long tick, int id, basics::numptr data);

private:
	flooritem_data(const flooritem_data&);					// forbidden
	const flooritem_data& operator=(const flooritem_data&);	// forbidden
};

#endif//_FLOORITEM_H_

