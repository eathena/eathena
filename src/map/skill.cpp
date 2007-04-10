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
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		// TODO target skill checks
		return skillbase::is_valid(errcode);
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
	/// function called to test is skill is valid
	virtual bool is_valid(skillfail_t& errcode) const
	{
		// TODO ground skill checks
		return skillbase::is_valid(errcode);
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
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		// TODO map skill checks
		return skillbase::is_valid(errcode);
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
	enum {SKILLID = -1};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		return skillbase::is_valid(errcode);
	}
	/// return object skill id
	virtual ushort get_skillid() const
	{
		return (ushort)SKILLID;
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
	virtual bool init(unsigned long& timeoffset)
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
					status_change_start(bl,SC_STUN,this->skill_lvl,0,0,0,skill_get_time2(SM_FATALBLOW,blowlvl),0);
				}
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
	virtual bool init(unsigned long& timeoffset)
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
			// check if the monster an MVP or undead
			if(( md && md->get_mode()&0x20) || bl->is_undead() )
				return;

			clif_skill_nodamage(this->caster,*bl,SKILLID, this->skill_lvl,1);
			if(rand()%100 > (50 + 3*this->skill_lvl + this->caster.get_lv() - bl->get_lv()))
			{	//TODO: How much does base level effect? Dummy value of 1% per level difference used. [Skotlex]
				this->caster.skill_failed(SKILLID, SF_FAILED);
			}
			status_change_start(bl,(status_t)SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );

			if( bl->is_casting() && bl->skill_can_cancel() )
				skill_castcancel(bl,0);

			if( bl->has_status(SC_FREEZE) )
				status_change_end(bl,SC_FREEZE,-1);
			if( bl->has_status(SC_STONE) && bl->get_statusvalue2(SC_STONE).integer()==0)
				status_change_end(bl,SC_STONE,-1);
			if( bl->has_status(SC_SLEEP) )
				status_change_end(bl,SC_SLEEP,-1);

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
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: SM_MAGNUM
/// skill ID: 7
class skill_sm_magnum : public targetskill
{
	struct spash_damage : public CMapProcessor
	{
		skill_sm_magnum& parent;
		ulong tick;

		spash_damage(skill_sm_magnum& p, ulong t)
			: parent(p), tick(t)
		{}

		virtual int process(block_list& bl) const
		{
			if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
				return 0;
			if( battle_check_target(&parent.caster,&bl,BCT_ENEMY) > 0)
			{
				const int dist = bl.get_distance(parent.caster);
				skill_attack(BF_WEAPON,&parent.caster,&parent.caster, &bl, skill_sm_magnum::SKILLID, parent.skill_lvl, this->tick, 0x0500|dist);
				return 1;
			}
			return 0;
		}
	};
	friend struct spash_damage;
	
public:
	skill_sm_magnum(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_sm_magnum()
	{}

	/// identifier.
	enum {SKILLID = SM_MAGNUM};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			block_list::foreachinarea(  spash_damage(*this, tick),
				this->caster.m,((int)this->caster.x)-2,((int)this->caster.y)-2,((int)this->caster.x)+2,((int)this->caster.y)+2,BL_ALL);
			clif_skill_nodamage(this->caster,this->caster,SKILLID,this->skill_lvl,1);

			status_change_start(&this->caster,SC_WATK_ELEMENT,3,10,0,0,10000,0); //Initiate 10% of your damage becomes fire element.
			
			// additional effect
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: SM_ENDURE
/// skill ID: 8
class skill_sm_endure : public targetskill
{
public:
	skill_sm_endure(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_sm_endure()
	{}

	/// identifier.
	enum {SKILLID = SM_ENDURE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			clif_skill_nodamage(this->caster,*bl,SKILLID,this->skill_lvl,1);
			status_change_start(bl,(status_t)SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );

			map_session_data *sd = bl->get_sd();
			if(sd)
				pc_blockskill_start(*sd, SKILLID, 10000);
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_SIGHT
/// skill ID: 10
class skill_mg_sight : public targetskill
{
public:
	skill_mg_sight(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_sight()
	{}

	/// identifier.
	enum {SKILLID = MG_SIGHT};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			clif_skill_nodamage(this->caster,*bl,SKILLID,this->skill_lvl,1);
			status_change_start(bl,(status_t)SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_NAPALMBEAT
/// skill ID: 11
class skill_mg_napalmbeat : public targetskill
{

	// using the callback to this function from the default template
	bool splash_count(block_list& bl)
	{
		if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
			return false;
		if( bl.id!=this->target_id && battle_check_target(&this->caster,&bl,BCT_ENEMY) > 0)
		{
			return true;
		}
		return false;
	}

	// having own callback objects defined
	struct splash_count : public CMapProcessor
	{
		skill_mg_napalmbeat& parent;

		splash_count(skill_mg_napalmbeat& p)
			: parent(p)
		{}
		virtual int process(block_list& bl) const
		{
			if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
				return 0;
			if( bl.id!=parent.target_id && battle_check_target(&parent.caster,&bl,BCT_ENEMY) > 0)
			{
				return 1;
			}
			return 0;
		}
	};
	friend struct splash_count;
	// having own callback objects defined
	struct splash_damage : public CMapProcessor
	{
		skill_mg_napalmbeat& parent;
		ulong tick;
		uint count;

		splash_damage(skill_mg_napalmbeat& p, ulong t, uint c)
			: parent(p), tick(t), count(c)
		{}

		virtual int process(block_list& bl) const
		{
			if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
				return 0;
			if( bl.id==parent.target_id || battle_check_target(&parent.caster,&bl,BCT_ENEMY) > 0)
			{
				skill_attack(BF_MAGIC,&parent.caster,&parent.caster,&bl,skill_mg_napalmbeat::SKILLID,parent.skill_lvl,tick,count|0x0500);
				return 1;
			}
			return 0;
		}
	};
	friend struct splash_damage;

public:
	skill_mg_napalmbeat(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_napalmbeat()
	{}

	/// identifier.
	enum {SKILLID = MG_NAPALMBEAT};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			// get surrounding enemies
			// calling with own callback objects
	//		const uint count = block_list::foreachinarea(  splash_count(*this),
	//			bl->m,((int)bl->x)-1,((int)bl->y)-1,((int)bl->x)+1,((int)bl->y)+1,BL_ALL);

			// calling with default object template
			const uint count = block_list::foreachinarea(  skillbase::map_callback<skill_mg_napalmbeat>(*this,&skill_mg_napalmbeat::splash_count),
				bl->m,((int)bl->x)-1,((int)bl->y)-1,((int)bl->x)+1,((int)bl->y)+1,BL_ALL);

			// calling with own callback objects
			// attack the surround
			block_list::foreachinarea(  splash_damage(*this, tick, count),
				bl->m,((int)bl->x)-1,((int)bl->y)-1,((int)bl->x)+1,((int)bl->y)+1,BL_ALL);

			// additional effect
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_SOULSTRIKE
/// skill ID: 13
class skill_mg_soulstrike : public targetskill
{
public:
	skill_mg_soulstrike(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_soulstrike()
	{}

	/// identifier.
	enum {SKILLID = MG_SOULSTRIKE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);

			// additional effect
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_COLDBOLT
/// skill ID: 14
class skill_mg_coldbolt : public targetskill
{
public:
	skill_mg_coldbolt(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_coldbolt()
	{}

	/// identifier.
	enum {SKILLID = MG_COLDBOLT};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
	/// check for doublecast.
	virtual bool doublecast(unsigned long& timeoffset) const
	{
		if( this->caster.has_status(SC_DOUBLECAST) &&
			rand()%100 < 40+10*this->caster.get_statusvalue1(SC_DOUBLECAST).integer() )
			{
				timeoffset = 500;//dmg.div_*dmg.amotion;
				return true;
			}

		return false;
	}
};

//////////////////////////////////////////
/// Skillname: MG_FROSTDIVER
/// skill ID: 15
class skill_mg_frostdiver : public targetskill
{
public:
	skill_mg_frostdiver(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_frostdiver()
	{}

	/// identifier.
	enum {SKILLID = MG_FROSTDIVER};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);

			// additional effect
			const int sc_def_mdef = status_get_sc_def_mdef(bl);
			int rate = (this->skill_lvl*3+35)*sc_def_mdef/100-(bl->get_int()+bl->get_luk())/15;
			if (rate <= 5)
				rate = 5;
			if( !bl->has_status(SC_FREEZE) && rand()%100 < rate)
				status_change_start(bl,SC_FREEZE,this->skill_lvl,0,0,0,skill_get_time2(SKILLID,this->skill_lvl)*(1-sc_def_mdef/100),0);
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_STONECURSE
/// skill ID: 16
class skill_mg_stonecurse : public targetskill
{
public:
	skill_mg_stonecurse(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_stonecurse()
	{}

	/// identifier.
	enum {SKILLID = MG_STONECURSE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			int i=0;
			bool fail_flag = true;
			map_session_data *sd = this->caster.get_sd();

			if( sd )
			{
				i=pc_search_inventory(*sd, skill_db[SKILLID].itemid[0]);
				if( i<0 || sd->status.inventory[i].amount<skill_db[SKILLID].amount[0] )
				{
					caster.skill_failed(SKILLID, (skill_db[SKILLID].itemid[0]==716)?SF_REDGEM:(skill_db[SKILLID].itemid[0]==717)?SF_BLUEGEM:SF_FAILED);
					return;
				}
			}

			if( *bl==BL_MOB && bl->get_mode()&0x20 )
			{	// failed
				caster.skill_failed(SKILLID);
			}
			else if( !status_isimmune(bl) && bl->has_status(SC_STONE) )
			{	// un-stoned
				caster.skill_failed(SKILLID);
				status_change_end(bl,SC_STONE,-1);
			}
			else if( rand()%100< this->skill_lvl*4+20 && !bl->is_undead())
			{	// success
				clif_skill_nodamage(caster,*bl,SKILLID,this->skill_lvl,1);
				status_change_start(bl,SC_STONE,this->skill_lvl,0,0,0,skill_get_time2(SKILLID,this->skill_lvl),0);
				fail_flag = false;
			}

			if( *bl==BL_MOB )
				mob_target(*(bl->get_md()),&this->caster,skill_get_range(SKILLID,this->skill_lvl));
			

			if( sd && (!fail_flag || this->skill_lvl <= 5) )
			{	// Level 6-10 doesn't consume a red gem if it fails [celest]
				pc_delitem(*sd, (ushort)i, skill_db[SKILLID].amount[0], 0);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_FIREBALL
/// skill ID: 17
class skill_mg_fireball : public targetskill
{
public:
	skill_mg_fireball(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_fireball()
	{}

	/// identifier.
	enum {SKILLID = MG_FIREBALL};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
// recode splashed damage distribution
//			if (flag & 1)
//			{
//				if (bl->id != skill_area_temp[1])
//				{
//					skill_area_temp[0] = bl->get_distance(skill_area_temp[2], skill_area_temp[3]);
//					skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,skill_area_temp[0]| 0x0500);
//				}
//			}
//			else
			{
//				int ar = 2;
//				skill_area_temp[0]=0;
//				skill_area_temp[1]=bl->id;
//				skill_area_temp[2]=bl->x;
//				skill_area_temp[3]=bl->y;
//				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,skill_area_temp[0]);
//				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,skill_area_temp[0]);
//				block_list::foreachinarea(  CSkillArea(*src,skillid,skilllv,tick, flag|BCT_ENEMY|1, skill_castend_damage_id),
//					bl->m,((int)bl->x)-ar,((int)bl->y)-ar,((int)bl->x)+ar,((int)bl->y)+ar,BL_ALL);
			}

			// additional effect
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MG_FIREBOLT
/// skill ID: 19
class skill_mg_firebolt : public targetskill
{
public:
	skill_mg_firebolt(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_firebolt()
	{}

	/// identifier.
	enum {SKILLID = MG_FIREBOLT};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
	/// check for doublecast.
	virtual bool doublecast(unsigned long& timeoffset) const
	{
		if( this->caster.has_status(SC_DOUBLECAST) &&
			rand()%100 < 40+10*this->caster.get_statusvalue1(SC_DOUBLECAST).integer() )
			{
				timeoffset = 500;//dmg.div_*dmg.amotion;
				return true;
			}

		return false;
	}
};

//////////////////////////////////////////
/// Skillname: MG_LIGHTNINGBOLT
/// skill ID: 20
class skill_mg_lightningbolt : public targetskill
{
public:
	skill_mg_lightningbolt(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_lightningbolt()
	{}

	/// identifier.
	enum {SKILLID = MG_LIGHTNINGBOLT};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);

			// additional effect
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
	/// check for doublecast.
	virtual bool doublecast(unsigned long& timeoffset) const
	{
		if( this->caster.has_status(SC_DOUBLECAST) &&
			rand()%100 < 40+10*this->caster.get_statusvalue1(SC_DOUBLECAST).integer() )
			{
				timeoffset = 500;//dmg.div_*dmg.amotion;
				return true;
			}

		return false;
	}
};

/*
//////////////////////////////////////////
/// Skillname: AL_RUWACH
/// skill ID: 24
class skill_al_ruwach : public targetskill
{
public:
	skill_al_ruwach(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_ruwach()
	{}

	/// identifier.
	enum {SKILLID = AL_RUWACH};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
				// Q: Shouldn't Ruwach damage you if you're hiding? i think it does, but i'm not sure how much. [Redozzen]
				// A: skill_attack is called when SC_RUWACH reveals someone. [FlavioJS]

			// additional effect
			clif_skill_nodamage(this->caster,*bl,SKILLID,this->skill_lvl,1);
			status_change_start(bl,(status_t)SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_PNEUMA
/// skill ID: 25
class skill_al_pneuma : public groundskill
{
public:
	skill_al_pneuma(fightable& caster, ushort lvl, ushort xs, ushort ys, const char* e)
		: groundskill(caster, lvl, xs, ys, e)
	{}
	virtual ~skill_al_pneuma()
	{}

	/// identifier.
	enum {SKILLID = AL_RUWACH};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
	{
		timeoffset = skill_castfix(&this->caster, skill_get_cast(SKILLID, this->skill_lvl));
		return true;
	}
	/// function called for execution.
	virtual void action(unsigned long tick)
	{
		// TODO
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		// TODO
		block_list* bl;
		return	(groundskill::is_valid(errcode) &&
			(this->caster.skill_check(SKILLID)>0) &&
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
/// Skillname: AL_TELEPORT
/// skill ID: 26
/// TODO class skill_al_teleport : public mapskill

//////////////////////////////////////////
/// Skillname: AL_WARP
/// skill ID: 27
/// TODO class skill_al_warp : public mapskill

//////////////////////////////////////////
/// Skillname: AL_HEAL
/// skill ID: 28
class skill_al_heal : public targetskill
{
public:
	skill_al_heal(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_heal()
	{}

	/// identifier.
	enum {SKILLID = AL_HEAL};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			skill_unit unit;
			struct skill_unit_group *sg = unit.group;
			map_session_data *sd = this->caster.get_sd();
			map_session_data *dstsd = this->caster.get_sd();

			if( bl->is_undead() )
			{// damage
				int damage = skill_calc_heal(&this->caster, this->skill_lvl);
			}
			if (this->target_id != sg->src_id)
			{
				int heal = 30 + sg->skill_lv * 5 + ((sg->val1) >> 16) * 5 + ((sg->val2) & 0xfff) / 2;
				clif_skill_nodamage(unit, bl, AL_HEAL, heal, 1);
				battle_heal(NULL, &bl, heal, 0, 0);
				skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,0);
			} else {
			
			// additional effect
				int heal = skill_calc_heal(&this->caster, this->skill_lvl);
				int skill;
				if (this->skill_lvl > 10)
					heal = 9999; //9999ヒール
				if (status_isimmune(bl))
					heal=0;	// ?金蟲カ?ド（ヒ?ル量０）
				if (sd) {
					if (skill = this->caster.skill_check(HP_MEDITATIO)>0)
						heal += heal * skill * 2 / 100;
				if (sd && dstsd && sd->status.partner_id == dstsd->status.char_id &&
					pc_calc_base_job2(sd->status.class_) == 23 && sd->status.sex == 0) //自分も?象もPC、?象が自分のパ?トナ?、自分がスパノビ、自分が♀なら
					heal = heal*2;	//スパノビの嫁が旦那にヒ?ルすると2倍になる
				}

				clif_skill_nodamage (*src, *bl, skillid, heal, 1);
			}
				
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_INCAGI
/// skill ID: 29
class skill_al_incagi : public targetskill
{
public:
	skill_al_incagi(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_incagi()
	{}

	/// identifier.
	enum {SKILLID = AL_INCAGI};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if (status_isimmune(bl))
				clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			else {
				status_change_start(bl,(status_t)SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
				clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_DECAGI
/// skill ID: 30
class skill_al_decagi : public targetskill
{
public:
	skill_al_decagi(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_decagi()
	{}

	/// identifier.
	enum {SKILLID = AL_DECAGI};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if (!status_isimmune(bl))
			{
				if (rand() % 100 < (40 + skilllv * 2 + (src->get_lv() + bl->get_int())/5 +(sc_def_mdef-100)))
				{	//0 defense is sc_def_mdef == 100! [Skotlex]
					int time = skill_get_time(skillid,skilllv);
					if (*bl == BL_PC) time/=2; //Halved duration for Players
					clif_skill_nodamage (*src, *bl, skillid, skilllv, 1);
					status_change_start (bl, (status_t)SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, time, 0);
				}
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_HOLYWATER
/// skill ID: 31
class skill_al_holywater : public targetskill
{
public:
	skill_al_holywater(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_holywater()
	{}

	/// identifier.
	enum {SKILLID = AL_HOLYWATER};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if(sd) {
				int eflag;
				struct item item_tmp(523);
				clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
				if(config.holywater_name_input)
				{
					item_tmp.card[0] = 0xfe;
					item_tmp.card[1] = 0;
					item_tmp.card[2] = basics::GetWord(sd->status.char_id,0);	// キャラID
					item_tmp.card[3] = basics::GetWord(sd->status.char_id,1);
				}
				eflag = pc_additem(*sd,item_tmp,1);
				if(eflag)
				{
					clif_additem(*sd,0,0,eflag);
					map_addflooritem(item_tmp,1,sd->block_list::m,sd->block_list::x,sd->block_list::y,NULL,NULL,NULL,0);
				}
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_CRUCIS
/// skill ID: 32
class skill_al_crucis : public targetskill
{
public:
	skill_al_crucis(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_crucis()
	{}

	/// identifier.
	enum {SKILLID = AL_CRUCIS};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if (flag & 1) {
				int race = bl->get_race();
				if( battle_check_target (src, bl, BCT_ENEMY) && (race == 6 || bl->is_undead()) )
				{
					int slv = src->get_lv(),tlv = bl->get_lv();
					int rate = 23 + skilllv*4 + slv - tlv;
					if (rand()%100 < rate)
						status_change_start(bl,(status_t)SkillStatusChangeTable[skillid],skilllv,0,0,0,0,0);
				}
			} else {
				clif_skill_nodamage(*src, *bl, skillid, skilllv, 1);
				block_list::foreachinarea(  CSkillArea(*src, skillid, skilllv, tick, flag|BCT_ENEMY|1,skill_castend_nodamage_id),
					src->m, ((int)src->x)-15, ((int)src->y)-15, ((int)src->x)+15, ((int)src->y)+15, BL_ALL);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_ANGELUS
/// skill ID: 33
class skill_al_angelus : public targetskill
{
public:
	skill_al_angelus(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_angelus()
	{}

	/// identifier.
	enum {SKILLID = AL_ANGELUS};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if (sd == NULL || sd->status.party_id == 0 || (flag & 1)) {
				// 個別の?理
				clif_skill_nodamage(*bl,*bl,skillid,skilllv,1);
				if(status_isimmune(bl))
					break;
				status_change_start(bl,(status_t)SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0);
			} else if (sd) {
				// パ?ティ全?への?理

				block_list::foreachpartymemberonmap(  CSkillArea(*src,skillid,skilllv,tick, flag|BCT_PARTY|1,skill_castend_nodamage_id), *sd,true);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_BLESSING
/// skill ID: 34
class skill_al_blessing : public targetskill
{
public:
	skill_al_blessing(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_blessing()
	{}

	/// identifier.
	enum {SKILLID = AL_BLESSING};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if (status_isimmune(bl))
				clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			else {
				status_change_start(bl,(status_t)SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
				clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool is_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AL_CURE
/// skill ID: 35
class skill_al_cure : public targetskill
{
public:
	skill_al_cure(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_al_cure()
	{}

	/// identifier.
	enum {SKILLID = AL_CURE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			if(!status_isimmune(bl) && !bl->is_undead())
			{
				status_change_end(bl, SC_SILENCE	, -1 );
				status_change_end(bl, SC_BLIND	, -1 );
				status_change_end(bl, SC_CONFUSION, -1 );
			} else if (!status_isimmune(bl) && bl->is_undead())
			{	//アンデッドなら暗闇?果
				status_change_start(bl, SC_CONFUSION,1,0,0,0,6000,0);
			}
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MC_IDENTIFY
/// skill ID: 40
class skill_mc_identify : public targetskill
{
public:
	skill_mc_identify(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mc_identify()
	{}

	/// identifier.
	enum {SKILLID = MC_IDENTIFY};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if(sd) clif_item_identify_list(*sd);
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MC_VENDING
/// skill ID: 41
class skill_mc_vending : public targetskill
{
public:
	skill_mc_vending(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mc_vending()
	{}

	/// identifier.
	enum {SKILLID = MC_VENDING};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			
			// additional effect
			if(sd) clif_openvendingreq(*sd,2+sd->skilllv);
		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: MC_MAMMONITE
/// skill ID: 42
class skill_mc_mammonite : public targetskill
{
public:
	skill_mc_mammonite(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mc_mammonite()
	{}

	/// identifier.
	enum {SKILLID = MC_MAMMONITE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			long zeny;
			zeny = skill_get_zeny(skill,lv);
			if( zeny>0 && sd->status.zeny < (uint32)zeny)
				sd->skill_failed(skill,SF_ZENY);
			Else
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			
			// additional effect

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AC_CONCENTRATION
/// skill ID: 45
class skill_ac_concentration : public targetskill
{
public:
	skill_ac_concentration(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_ac_concentration()
	{}

	/// identifier.
	enum {SKILLID = AC_CONCENTRATION};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			int range = 1;
			clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			status_change_start(bl,(status_t)SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
			block_list::foreachinarea( CStatusChangetimer(*src,SkillStatusChangeTable[skillid],tick),
				src->m, ((int)src->x)-range, ((int)src->y)-range, ((int)src->x)+range, ((int)src->y)+range,BL_ALL);

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AC_DOUBLE
/// skill ID: 46
class skill_ac_double : public targetskill
{
public:
	skill_ac_double(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_ac_double()
	{}

	/// identifier.
	enum {SKILLID = AC_DOUBLE};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			if(sd->equip_index[10] >= MAX_INVENTORY) {
				clif_arrow_fail(*sd,0);
			} else {
				// add delete arrows
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			}

			// additional effect

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: AC_SHOWER
/// skill ID: 47
class skill_ac_shower : public targetskill
{
public:
	skill_ac_shower(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_ac_shower()
	{}

	/// identifier.
	enum {SKILLID = AC_SHOWER};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			if(sd->equip_index[10] >= MAX_INVENTORY) {
				clif_arrow_fail(*sd,0);
			} else {
				// add delete arrows
				int ar = 2;
				int x = bl->x, y = bl->y;
				skill_area_temp[1]=bl->id;
				skill_area_temp[2]=x;
				skill_area_temp[3]=y;
				// まずタ?ゲットに攻?を加える
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
				// その後タ?ゲット以外の範??の敵全?に?理を行う

				block_list::foreachinarea(  CSkillArea(*src,skillid,skilllv,tick, flag|BCT_ENEMY|1,skill_castend_damage_id),
					bl->m,x-ar,y-ar,x+ar,y+ar,BL_ALL);
			}

			// additional effect

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: TF_STEAL
/// skill ID: 50
class skill_tf_steal : public targetskill
{
public:
	skill_tf_steal(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_tf_steal()
	{}

	/// identifier.
	enum {SKILLID = TF_STEAL};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			if(sd) {
				if(pc_steal_item(*sd,bl))
					clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			else
				sd->skill_failed(skillid,SF_STEAL);

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: TF_HIDING
/// skill ID: 51
class skill_tf_hiding : public targetskill
{
public:
	skill_tf_hiding(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_tf_hiding()
	{}

	/// identifier.
	enum {SKILLID = TF_HIDING};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			status_t sc = (status_t)SkillStatusChangeTable[skillid];
			//clif_skill_nodamage(*src,*bl,skillid,0xFFFF,1);
			if( bl->has_status(sc) )
				status_change_end(bl, sc, -1);
			else
				status_change_start(bl,sc,skilllv,0,0,0,skill_get_time(skillid,skilllv),0)

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: TF_POISON
/// skill ID: 52
class skill_tf_poison : public targetskill
{
public:
	skill_tf_poison(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_tf_poison()
	{}

	/// identifier.
	enum {SKILLID = TF_POISON};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			// This dosnt seem right... i thought TF_POISON was just enchant poison.
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);

			// additional effect
			if(rand()%100< (2*skilllv+10)*sc_def_vit/100 )
				status_change_start(bl,SC_POISON,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
			else
				sd->skill_failed(skillid);

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: TF_DETOXIFY
/// skill ID: 53
class skill_tf_detoxify : public targetskill
{
public:
	skill_tf_detoxify(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_tf_detoxify()
	{}

	/// identifier.
	enum {SKILLID = TF_DETOXIFY};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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

			// additional effect
			clif_skill_nodamage(*src,*bl,skillid,skilllv,1);
			status_change_end(bl, SC_POISON	, -1 );
			status_change_end(bl, SC_DPOISON	, -1 );

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
/// Skillname: ALL_RESURRECTION
/// skill ID: 54
class skill_all_resurrection : public targetskill
{
public:
	skill_all_resurrection(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_all_resurrection()
	{}

	/// identifier.
	enum {SKILLID = ALL_RESURRECTION};

	/// function called for initialisation.
	virtual bool init(unsigned long& timeoffset)
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
			if (*bl != BL_PC && bl->is_undead() )
				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);

			// additional effect
			if(dstsd) {
				int per = 0;
				if (maps[bl->m].flag.pvp && dstsd->pvp_point == 0)
					return;

				if( dstsd->is_dead() )
				{
					clif_skill_nodamage(*src,*bl,skillid,skilllv,1);

					switch(skilllv)
					{
					case 1: per=10; break;
					case 2: per=30; break;
					case 3: per=50; break;
					case 4: per=80; break;
					}

					dstsd->status.hp = dstsd->status.max_hp * per / 100;

					if (dstsd->status.hp <= 0)
						dstsd->status.hp = 1;

					if(dstsd->state.restart_full_recover)
					{
						dstsd->status.hp = dstsd->status.max_hp;
						dstsd->status.sp = dstsd->status.max_sp;
					}

					dstsd->set_stand();

					if(config.pc_invincible_time > 0)
						pc_setinvincibletimer(*dstsd, config.pc_invincible_time);

					clif_updatestatus(*dstsd, SP_HP);
					clif_resurrection(*bl, 1);
				}

		}
	}
	/// function called when stopped
	virtual void stop()
	{	
		this->caster.skill_stopped(SKILLID);
	}
	/// function called to test if skill is valid.
	virtual bool id_valid(skillfail_t& errcode) const
	{
		block_list* bl;
		return	(targetskill::is_valid(errcode) &&
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
*/

///////////////////////////////////////////////////////////////////////////////
/// create a target skill.
skillbase* skillbase::create(fightable& caster, ushort skillid, ushort skilllv, uint32 targetid)
{	// id2object for target skills
	skillbase* skill = NULL;
	switch(skillid)
	{
	case skill_sm_bash::SKILLID:	skill = new skill_sm_bash(caster, skilllv, targetid); break;
	case skill_sm_provoke::SKILLID:	skill = new skill_sm_provoke(caster, skilllv, targetid); break;
	case skill_sm_magnum::SKILLID:	skill = new skill_sm_magnum(caster, skilllv, targetid); break;
	case skill_sm_endure::SKILLID:	skill = new skill_sm_endure(caster, skilllv, targetid); break;
	case skill_mg_sight::SKILLID:	skill = new skill_mg_sight(caster, skilllv, targetid); break;
	case skill_mg_napalmbeat::SKILLID:	skill = new skill_mg_napalmbeat(caster, skilllv, targetid); break;
	case skill_mg_soulstrike::SKILLID:	skill = new skill_mg_soulstrike(caster, skilllv, targetid); break;
	case skill_mg_coldbolt::SKILLID:	skill = new skill_mg_coldbolt(caster, skilllv, targetid); break;
	case skill_mg_frostdiver::SKILLID:	skill = new skill_mg_frostdiver(caster, skilllv, targetid); break;
	case skill_mg_stonecurse::SKILLID:	skill = new skill_mg_stonecurse(caster, skilllv, targetid); break;
	case skill_mg_fireball::SKILLID:	skill = new skill_mg_fireball(caster, skilllv, targetid); break;
	case skill_mg_firebolt::SKILLID:	skill = new skill_mg_firebolt(caster, skilllv, targetid); break;
	case skill_mg_lightningbolt::SKILLID:	skill = new skill_mg_lightningbolt(caster, skilllv, targetid); break;

	// add new target skills here
	}
	// check for timed or immediate execution
	skillbase::initialize(skillid, skill);
	return skill;
}
///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
/// timer entry point. does also check for doublecast
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
		skillbase* skill = mv->cSkillObj;
		mv->cSkillObj=NULL;
		// 2. call the action function
		skillfail_t errcode=SF_FAILED;
		if( skill->is_valid(errcode) )
		{
			skill->action(tick);
			unsigned long timeoffset=0;
			if( skill->doublecast(timeoffset) )
			{	// re-enter the skill
				mv->cSkillObj=skill;
				if( timeoffset>100 )
				{	/// action is to be called more than 100ms from now
					/// initialize timer. use the timer_entry_double for execution
					skill->timerid = add_timer(tick+timeoffset, skillbase::timer_entry_double, skill->caster.block_list::id, basics::numptr(skill));
					return 0;
				}
				else
				{	// execute it immediately
					skill->action(tick+timeoffset);
				}
			}
		}
		else
			mv->skill_failed(skill->get_skillid(), errcode);
		// 3. delete the skill
		delete skill;
		////////////////
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// timer entry point for double casted spells.
int skillbase::timer_entry_double(int tid, unsigned long tick, int id, basics::numptr data)
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
		skillbase* skill = mv->cSkillObj;
		mv->cSkillObj=NULL;
		// 2. call the action function
		skillfail_t errcode=SF_FAILED;
		if( skill->is_valid(errcode) )
			skill->action(tick);
		else
			mv->skill_failed(skill->get_skillid(), errcode);
		// 3. delete the skill
		delete skill;
		////////////////
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
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
		{	// skill is invalid
			skill->caster.skill_failed(skillid, errcode);
		}
		// delete the skill, either failed or already executed
		delete skill;
		skill=NULL;
	}
	//called create with a invalid skillid
}

///////////////////////////////////////////////////////////////////////////////
/// default virtual function stub
/// check for doublecast. (if the skill should be double-casted?)
bool skillbase::doublecast(unsigned long& timeoffset) const
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// default virtual function stub
/// check if the skill is supportive
///
/// list of supportive skills from trunk 10047
/// 28, AL_HEAL
/// 29, AL_INCAGI
/// 34, AL_BLESSING
/// 35, AL_CURE
/// 53, TF_DETOXIFY
/// 54, ALL_RESURRECTION
/// 66, PR_IMPOSITIO
/// 67, PR_SUFFRAGIUM
/// 68, PR_ASPERSIO
/// 71, PR_SLOWPOISON
/// 72, PR_STRECOVERY
/// 73, PR_KYRIE
/// 108, BS_REPAIRWEAPON
/// 138, AS_ENCHANTPOISON
/// 231, AM_POTIONPITCHER
/// 234, AM_CP_WEAPON
/// 235, AM_CP_SHIELD
/// 236, AM_CP_ARMOR
/// 237, AM_CP_HELM
/// 255, CR_DEVOTION
/// 256, CR_PROVIDENCE
/// 262, MO_ABSORBSPIRITS
/// 280, SA_FLAMELAUNCHER
/// 281, SA_FROSTWEAPON
/// 282, SA_LIGHTNINGLOADER
/// 283, SA_SEISMICWEAPON
/// 361, HP_ASSUMPTIO
/// 374, PF_SOULCHANGE
/// 396, CG_MARIONETTE
/// 445, SL_ALCHEMIST
/// 446, AM_BERSERKPITCHER
/// 447, SL_MONK
/// 448, SL_STAR
/// 449, SL_SAGE
/// 450, SL_CRUSADER
/// 451, SL_SUPERNOVICE
/// 452, SL_KNIGHT
/// 453, SL_WIZARD
/// 454, SL_PRIEST
/// 455, SL_BARDDANCER
/// 456, SL_ROGUE
/// 457, SL_ASSASIN
/// 458, SL_BLACKSMITH
/// 460, SL_HUNTER
/// 461, SL_SOULLINKER
/// 462, SL_KAIZEL
/// 463, SL_KAAHI
/// 464, SL_KAUPE
/// 465, SL_KAITE
/// 479, CR_FULLPROTECTION
/// 494, SL_HIGH
/// 1015, MO_KITRANSLATION
bool skillbase::is_supportive() const
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/// check if the skill is valid (only global checks)
bool skillbase::is_valid(skillfail_t& errcode) const
{
	// TODO global checks
	return this->caster.can_castskill(*this,errcode);
}
