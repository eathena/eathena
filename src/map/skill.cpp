#include "showmsg.h"

#include "battle.h"
#include "clif.h"
#include "fightable.h"
#include "mob.h"
#include "pet.h"
#include "homun.h"
#include "pc.h"
#include "skill.h"
#include "status.h"

///////////////////////////////////////////////////////////////////////////////
/// basic target skill.
/// could be used as their base class,
/// add more functionality here if it can be shared by all derived objects
class targetskill : public skillbase
{
protected:
	ushort skill_lvl;
	uint32 target_id;
public:

	targetskill(fightable& caster, ushort lvl, uint32 id)
		: skillbase(caster), skill_lvl(lvl), target_id(id)
	{}
	virtual ~targetskill()	{}

	/// return object skill level
	virtual ushort get_skilllv() const
	{
		return this->skill_lvl;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// basic ground skill.
/// could be used as their base class
/// add more functionality here if it can be shared by all derived objects
class groundskill : public skillbase
{
protected:
	ushort skill_lvl;
	ushort xpos,ypos;
	basics::string<> extra;
public:

	groundskill(fightable& caster, ushort lvl, ushort xs, ushort ys, const char*e)
		: skillbase(caster), skill_lvl(lvl), xpos(xs), ypos(ys), extra(e)
	{}
	virtual ~groundskill()	{}

	/// return object skill level
	virtual ushort get_skilllv() const
	{
		return this->skill_lvl;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// basic map skill.
/// could be used as their base class
/// add more functionality here if it can be shared by all derived objects
class mapskill : public skillbase
{
protected:
	basics::string<> mapname;
public:

	mapskill(fightable& caster, const char* map)
		: skillbase(caster), mapname(map)
	{}
	virtual ~mapskill()	{}

	/// return object skill level
	virtual ushort get_skilllv() const
	{
		return 1;
	}
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
/// some test class
class dummyskill : public skillbase
{
	const char* msg;
public:

	dummyskill(fightable& caster, const char*m) : skillbase(caster), msg(m)
	{
		printf("%s created\n", msg?msg:"dummy skill");
	}
	virtual ~dummyskill()	{}

	/// identifier.
	enum {SKILLID = MG_FIREBOLT};

	/// function called for initialisation.
	virtual bool init(ulong& timeoffset)
	{
		timeoffset = rand()%5000;
		return true;
	}
	/// function called for execution.
	virtual void action(unsigned long tick)
	{
		printf("%s executed\n", msg?msg:"dummy skill");
	}
	/// function called when stopped
	virtual void stop()
	{
		printf("%s stopped\n", msg?msg:"dummy skill");
	}
	/// function called to test is skill is valid.
	virtual bool is_valid(skillfail_t&) const
	{
		return true;
	}
	/// return object skill id
	virtual ushort get_skillid() const
	{
		return SKILLID;
	}
	/// return object skill level
	virtual ushort get_skilllv() const
	{
		return 1;
	}
};

//////////////////////////////////////////
/// Skillname: SM_BASH
/// skill ID: 5
class skill_sm_bash : public targetskill
{
public:
	skill_sm_bash(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_sm_bash()
	{}

	/// identifier.
	enum {SKILLID = SM_BASH};

	/// function called for initialisation.
	virtual bool init(ulong& timeoffset)
	{
		timeoffset = skill_castfix(&this->caster, skill_get_cast(SKILLID, this->skill_lvl));
		return true;
	}
	/// function called for execution.
	virtual void action(unsigned long tick)
	{
		block_list* bl=block_list::from_blid(this->target_id);
		if(bl)
		{
			// weapon attack
			skill_attack(BF_WEAPON,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);

			// additional effect
			int blowlvl;
			if( this->skill_lvl>5 && (blowlvl=this->caster.skill_check(SM_FATALBLOW))>0 )
			{
				int sc_def_vit = status_get_sc_def_vit(bl);
				if( rand()%100 < (6*(this->skill_lvl-5)+this->caster.get_lv()/10)*sc_def_vit/100 )
				{	//TODO: How much % per base level it actually is?
					status_change_start(bl,SC_STAN,this->skill_lvl,0,0,0,skill_get_time2(SM_FATALBLOW,blowlvl),0);
				}
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	virtual bool is_valid(skillfail_t&) const
	{
		block_list* bl;
		return	(
			(this->caster.skill_check(SKILLID)>0) &&
			(bl=block_list::from_blid(this->target_id))!=NULL &&
			bl->is_on_map() &&
			!bl->is_dead() &&
			!bl->is_hiding() );
	}
	/// return object skill id
	virtual ushort get_skillid() const
	{
		return SKILLID;
	}
};

//////////////////////////////////////////
/// Skillname: SM_PROVOKE
/// skill ID: 6
class skill_sm_provoke : public targetskill
{
public:
	skill_sm_provoke(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_sm_provoke()
	{}

	/// identifier.
	enum {SKILLID = SM_PROVOKE};

	/// function called for initialisation.
	virtual bool init(ulong& timeoffset)
	{
		timeoffset = skill_castfix(&this->caster, skill_get_cast(SKILLID, this->skill_lvl));
		return true;
	}
	/// function called for execution.
	virtual void action(unsigned long tick)
	{
		block_list* bl=block_list::from_blid(this->target_id);
		if(bl)
		{
			mob_data* md = bl->get_md();
			struct status_change *sc_data = status_get_sc_data(bl);
			// check if the monster an MVP or undead
			if(( md && md->get_mode()&0x20) || bl->is_undead() )
				return;

			clif_skill_nodamage(this->caster,*bl,SKILLID, this->skill_lvl,1);
			if(rand()%100 > (50 + 3*this->skill_lvl + this->caster.get_lv() - bl->get_lv()))
			{	//TODO: How much does base level effect? Dummy value of 1% per level difference used. [Skotlex]
				this->caster.skill_failed(SKILLID, SF_FAILED);
			}
			status_change_start(bl,SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );

			if( bl->is_casting() && bl->skill_can_cancel() )
				skill_castcancel(bl,0);

			if(sc_data)
			{
				if(sc_data[SC_FREEZE].timer!=-1)
					status_change_end(bl,SC_FREEZE,-1);
				if(sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2.num==0)
					status_change_end(bl,SC_STONE,-1);
				if(sc_data[SC_SLEEP].timer!=-1)
					status_change_end(bl,SC_SLEEP,-1);
			}

			if(md)
			{
				int range = skill_get_range(SKILLID,this->skill_lvl);
				md->provoke_id = this->caster.block_list::id;
				mob_target(*md,&this->caster,range);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	virtual bool is_valid(skillfail_t&) const
	{
		block_list* bl;
		return	(
			(this->caster.skill_check(SKILLID)>0) &&
			(bl=block_list::from_blid(this->target_id))!=NULL &&
			bl->is_on_map() &&
			!bl->is_dead() &&
			!bl->is_hiding() );
	}
	/// return object skill id
	virtual ushort get_skillid() const
	{
		return SKILLID;
	}
};








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/// timer entry point
int skillbase::timer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	fightable* mv = fightable::from_blid(id);
	if( mv && mv->cSkillObj )
	{
		if(mv->cSkillObj->timerid != tid)
		{
			if(config.error_log)
				ShowError("skilltimer_entry %d != %d\n",mv->cSkillObj->timerid,tid);
			return 0;
		}
		mv->cSkillObj->timerid=-1;

		////////////////
		// execute the skill
		// 1. strip the skill from the the object
		skillbase* sk = mv->cSkillObj;
		mv->cSkillObj=NULL;
		// 2. call the action function
		skillfail_t errcode=SF_FAILED;
		if( sk->is_valid(errcode) )
			sk->action(tick);
		else
			mv->skill_failed(sk->get_skillid(), errcode);
		// 3. delete the skill
		delete sk;
		////////////////
	}
	return 0;
}

/// check for timed or immediate execution
void skillbase::initialize(ushort skillid, skillbase*& skill)
{
	if( skill )
	{
		ulong timeoffset=0;
		skillfail_t errcode=SF_FAILED;
		if( skill->is_valid(errcode) && skill->init(timeoffset) )
		{	// skill is ok
			if( timeoffset>100 )
			{	/// action is to be called more than 100ms from now
				/// initialize timer.
				skill->timerid = add_timer(gettick()+timeoffset, skillbase::timer_entry, skill->caster.block_list::id, basics::numptr(skill));
				return;
			}
			else
			{	// execute it immediately
				skill->action(gettick());
			}
		}
		else
		{
			skill->caster.skill_failed(skillid, errcode);
		}
		// delete the skill, either failed or already executed
		delete skill;
		skill=NULL;
	}
	//called create with a invalid skillid
}
/// create a target skill.
skillbase* skillbase::create(fightable& caster, ushort skillid, ushort skilllv, uint32 targetid)
{	// id2object for target skills
	skillbase* skill = NULL;
	switch(skillid)
	{
	case dummyskill::SKILLID:	skill = new dummyskill(caster, "targetskill"); break;

	case skill_sm_bash::SKILLID:    skill = new skill_sm_bash(caster, skilllv, targetid); break;
	case skill_sm_provoke::SKILLID: skill = new skill_sm_provoke(caster, skilllv, targetid); break;

	// add new target skills here
	}
	// check for timed or immediate execution
	skillbase::initialize(skillid, skill);
	return skill;
}
/// create an area skill.
skillbase* skillbase::create(fightable& caster, ushort skillid, ushort skilllv, ushort x, ushort y, const char*extra)
{	// id2object for area skills
	skillbase* skill = NULL;
	switch(skillid)
	{
	case dummyskill::SKILLID:	skill = new dummyskill(caster,"areaskill"); break;
	// add new area skills here
	}
	// check for timed or immediate execution
	skillbase::initialize(skillid, skill);
	return skill;
}
/// create a map skill.
skillbase* skillbase::create(fightable& caster, ushort skillid, const char*mapname)
{	// id2object for map skills
	skillbase* skill = NULL;
	switch(skillid)
	{
	case dummyskill::SKILLID:	skill = new dummyskill(caster,"mapskill"); break;
	// add new map skills here
	}
	// check for timed or immediate execution
	skillbase::initialize(skillid, skill);
	return skill;
}

/// destructor.
/// removes the timer if any
skillbase::~skillbase()
{
	if(this->timerid!=-1)
	{	// clear the timer, if not yet removed
		delete_timer(this->timerid, fightable::skilltimer_entry);
	}

	if( this->caster.cSkillObj == this )
	{	// clear the caster
		this->caster.cSkillObj=NULL;
		// cannot call stop from here 
		// since the derived class is already destroyed
	}
}
