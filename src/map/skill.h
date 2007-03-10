#ifndef _SKILL_H_
#define _SKILL_H_

#include "oldskill.h"



// questions:
// do we need seperated attackttimer and skilltimer?
// since skilltimer needs separation between area and target skills anyway, 
// separating also timed attacks would be no problem
// {my answer: no, thus can compound attack and skill execution interface [Hinoko]}
// {not sure, but I don't see a reason of having casting in parallel to attacking exept for the auto spells maybe [Shinomori]}
// {autoskill behaviour: triggered from physical attack, does instant cast when all cast conditions are met and adds the cooldowntime [Shinomori]}
// {"no" as final answer [Shinomori]}
//
// do we need seperated target markers for attack and skill usage?
// {my answer: no [Hinoko]}
// {"no" as final answer [Shinomori]}
//
// can multiple skills be casted in parallel aka do we need arrays for skill_timerskill
// {my answer: no [Hinoko]}
// {"no" as final answer [Shinomori]}
//
// is a separated set of skill casting parameters necessary at the object itself
// {my answer: no [Hinoko]}
// {"no" as final answer [Shinomori]}
//
// do we need arrays of unitgroups (or would a linked list be sufficient)
// or even better, have no unitgroup saving at the char but completely outside
// {my answer: set up everything externally, no extra data at the casting object at all [Hinoko]}
// {suggest to have the root node of a doubled linked list at the object [Shinomori]}
//
// do we assume nothing has been checked before the is_valid function is called?
// where do we put checks that affect all skills or a particular class of skills
// {my answer: assume nothing is checked and put global checks in the skillbase::create functions [FlavioJS]}
// {gonna assume nothing has been checked, global checks are going to skill::is_valid in targetskill/groundskill/mapskill that calls a protected function skill::is_valid_sub that will be used in the skills [FlavioJS]}
//
// what will happen to the inf value? currently that value classifies the type 
// of skill and there are restriction based on the type of skill,
// like SC_HERMODE blocking supportive skills
// {my answer: keep the inf value and do a global check [FlavioJS]}
// {for the moment just adding functions in skillbase or higher up like skill::is_support_skill [FlavioJS]}
// 
/*
	proposed new flow for skills:
	(possibly also integrate physical attacks here)

	skill/attack execution is requested
	->
	check if execution is possible
	->
	previous skill/attack is stopped when necessary (by removing the object)
	->
	a suitable skill object is constructed, 
	containing all necessary data for the action
	possible skills are: target skills, area skills, map skills
	
	  former calls:
	  skill_use_pos(<caster>, <x>, <y>, <skillid>, <skilllv>, <additional parameter (talkiemsg) which is currently done via a global variable, empty on default>)
	  skill_use_id(<caster>, <target_id>, <skillid>, <skilllv>)
	  skill_castend_map(<caster>,<skillid>, <mapname>)

	->
	after construction the execution is triggered
	which either does the action immediately or starts timers or 
	do the initial action and start timers or whatever
	->
	on finishing the object removes itself
	and sets the specified cooldown time

	necessary external access:
	* do an action (ie by skillid)
	* stop the action (considering a possible cooldown time also on stopped skills)



	proposed new flow for status changes:

	status changes is requested
	->
	check if status changes is possible
	->
	a suitable status object is constructed, 
	which is starting the status change
	or
	existing previous status updated
	->
	on finishing the object removes itself
	and removes the status change from the applied object

	necessary external access:
	* start (ie by statusid)
	* end
	* load/save (actually some status_tobuffer/status_frombuffer equivalents)
*/


///////////////////////////////////////////////////////////////////////////////
// predeclarations
struct fightable;


///////////////////////////////////////////////////////////////////////////////
/// virtual skill base class.
/// declares a common interface for skills.
class skillbase : public basics::global, public basics::noncopyable
{
	ICL_EMPTY_COPYCONSTRUCTOR(skillbase)
protected:
	template<typename T>
	struct map_callback : public CMapProcessor
	{
		T& parent;
		bool (T::*func)(block_list& bl);


		map_callback(T& p, bool (T::*f)(block_list& bl))
			: parent(p), func(f)
		{}
		virtual int process(block_list& bl) const
		{
			return (parent.*func)(bl);
		}
	};
public:
	int			timerid;	///< will replace skilltimer
	fightable	&caster;	///< casting object
protected:
	/// protected constructor.
	/// only derived can create
	skillbase(fightable &c) : timerid(-1), caster(c)	{}
public:
	/// destructor.
	virtual ~skillbase();

public:
	/// identifier.
	/// overload with specific value
	enum {SKILLID = -1};

	/// function called for initialisation.
	/// overload with specific needs
	/// timeoffset receives the tick for delayed execution,
	/// returns true when execution is ok
	virtual bool init(unsigned long& timeoffset)=0;
	/// function called for skill execution.
	/// overload with specific needs
	virtual void action(unsigned long tick)=0;
	/// function called for cast cancel.
	/// overload with specific needs
	virtual void stop()=0;
	/// function called to test if skill is valid.
	/// overload with specific needs
	virtual bool is_valid(skillfail_t& errcode) const=0;
	/// return object skill id
	virtual ushort get_skillid() const=0;
	/// return object skill level
	virtual ushort get_skilllv() const=0;
	/// check for doublecast.
	virtual bool doublecast(unsigned long& timeoffset) const
	{
		return false;
	}
	// different static constructors
	static skillbase* create(fightable& caster, ushort skillid, ushort skilllv, uint32 targetid);
	static skillbase* create(fightable& caster, ushort skillid, ushort skilllv, ushort x, ushort y, const char*extra=NULL);
	static skillbase* create(fightable& caster, ushort skillid, const char*mapname);
private:
	/// check for timed or immediate execution
	static void initialize(ushort skillid, skillbase*& skill);
	static int timer_entry(int tid, unsigned long tick, int id, basics::numptr data);
	static int timer_entry_double(int tid, unsigned long tick, int id, basics::numptr data);
};



#endif//_SKILL_H_
