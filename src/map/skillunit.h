#ifndef _SKILLUNIT_H_
#define _SKILLUNIT_H_

#include "map.h"

struct skill_unit_group
{
	uint32 src_id;
	uint32 party_id;
	uint32 guild_id;
	unsigned short map;
	long target_flag;
	unsigned long tick;
	long limit;
	long interval;

	unsigned short skill_id;
	unsigned short skill_lv;
	long val1;
	long val2;
	long val3;
	basics::string<> valstring;
	unsigned char unit_id;
	long group_id;
	long unit_count;
	long alive_count;
	struct skill_unit *unit;

	skill_unit_group() :
		src_id(0),
		party_id(0),
		guild_id(0),
		map(0),
		target_flag(0),
		tick(0),
		limit(0),
		interval(0),
		skill_id(0),
		skill_lv(0),
		val1(0),
		val2(0),
		val3(0),
		unit_id(0),
		group_id(0),
		unit_count(0),
		alive_count(0),
		unit(NULL)
	{}
};

///////////////////////////////////////////////////////////////////////////////
struct skill_unit : public block_list
{
	struct skill_unit_group *group;

	long limit;
	long val1;
	long val2;
	short alive;
	short range;

	skill_unit() :
		group(NULL),
		limit(0),
		val1(0),
		val2(0),
		alive(0),
		range(0)
	{}
	virtual ~skill_unit()
	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_SKILL);
	}
	virtual skill_unit*				get_sk()				{ return this; }
	virtual const skill_unit*		get_sk() const			{ return this; }


private:
	skill_unit(const skill_unit&);					// forbidden
	const skill_unit& operator=(const skill_unit&);	// forbidden

	virtual uint32 get_party_id() const		{ return this->group?this->group->party_id:0; }
	virtual uint32 get_guild_id() const		{ return this->group?this->group->guild_id:0; }
};



struct skill_unit_group_tickset
{
	unsigned short skill_id;
	unsigned long tick;

	skill_unit_group_tickset() :
		skill_id(0),
		tick(0)
	{}
};

struct skill_timerskill
{
	uint32 src_id;
	uint32 target_id;
	unsigned short map;
	unsigned short x;
	unsigned short y;
	unsigned short skill_id;
	unsigned short skill_lv;
	int timer;
	long type;
	long flag;

	skill_timerskill() : 
		src_id(0),
		target_id(0),
		map(0),
		x(0),
		y(0),
		skill_id(0),
		skill_lv(0),
		timer(-1),
		type(0),
		flag(0)
	{}
};



#endif//_SKILLUNIT_H_
