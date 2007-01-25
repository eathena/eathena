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
	/// function called to test is skill is valid.
	virtual bool is_valid(skillfail_t&) const
	{
		return true;
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
				if(sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].integer2()==0)
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

//////////////////////////////////////////
/// Skillname: SM_MAGNUM
/// skill ID: 7
class skill_sm_magnum : public targetskill, public CMapProcessor
{
	ulong savedtick;
public:
	skill_sm_magnum(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_sm_magnum()
	{}

	// callback from foreach calls
	virtual int process(block_list& bl) const
	{
	// get surrounding enemies
		if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
			return 0;
		if( battle_check_target(&this->caster,&bl,BCT_ENEMY) > 0)
		{
			const int dist = bl.get_distance(this->caster);
			skill_attack(BF_WEAPON,&this->caster,&this->caster, &bl, SKILLID, this->skill_lvl, this->savedtick, 0x0500|dist);
			return 1;
		}
		return 0;
	}

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
			this->savedtick  = tick;	// to have it readable from the mapprocessor callback
			// weapon attack
			block_list::foreachinarea(  *this,
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
			status_change_start(bl,SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );

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
			status_change_start(bl,SkillStatusChangeTable[SKILLID],this->skill_lvl,0,0,0,skill_get_time(SKILLID,this->skill_lvl),0 );
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
/// Skillname: MG_NAPALMBEAT
/// skill ID: 11
class skill_mg_napalmbeat : public targetskill, public CMapProcessor
{
	mutable basics::vector<block_list*>	blocks;
public:
	skill_mg_napalmbeat(fightable& caster, ushort lvl, uint32 id)
		: targetskill(caster, lvl, id)
	{}
	virtual ~skill_mg_napalmbeat()
	{}

	// callback from foreach calls
	virtual int process(block_list& bl) const
	{	// get surrounding enemies, excluding the target
		if(bl!=BL_PC && bl!=BL_MOB && bl!=BL_SKILL)
			return 0;
		if( bl.id!=this->target_id && battle_check_target(&this->caster,&bl,BCT_ENEMY) > 0)
		{
			blocks.push_back(&bl);
			return 1;
		}
		return 0;
	}

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
			block_list::foreachinarea(  *this,
				bl->m,((int)bl->x)-1,((int)bl->y)-1,((int)bl->x)+1,((int)bl->y)+1,BL_ALL);

			// attack the target
			skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,blocks.size());

			// splash the surround
			basics::vector<block_list*>::iterator iter(this->blocks);
			for(; iter; ++iter)
			{
				skill_attack(BF_MAGIC,&this->caster,&this->caster,bl,SKILLID,this->skill_lvl,tick,blocks.size()|0x0500);
			}
			// additional effect
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

			// additional effect
// Need to add support for double cast. (currently done inside skill_attack)
//			struct status_change *sc_data = status_get_sc_data(&this->caster);
//			if( sc_data && sc_data[SC_DOUBLECAST].timer != -1 &&
//				rand() % 100 < 40+10*sc_data[SC_DOUBLECAST].val1.num )
//			{
//				skill_castend_delay(*src, *bl, skillid, skilllv, tick + dmg.div_*dmg.amotion, flag|1);
//			}
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
			struct status_change *sc_data = status_get_sc_data(bl);
			const int sc_def_mdef = status_get_sc_def_mdef(bl);
			int rate = (this->skill_lvl*3+35)*sc_def_mdef/100-(bl->get_int()+bl->get_luk())/15;
			if (rate <= 5)
				rate = 5;
			if(sc_data && sc_data[SC_FREEZE].timer == -1 && rand()%100 < rate)
				status_change_start(bl,SC_FREEZE,this->skill_lvl,0,0,0,skill_get_time2(SKILLID,this->skill_lvl)*(1-sc_def_mdef/100),0);
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
			struct status_change *sc_data = status_get_sc_data(bl);
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
			else if( !status_isimmune(bl) && sc_data && sc_data[SC_STONE].timer != -1)
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
				pc_delitem(*sd, i, skill_db[SKILLID].amount[0], 0);
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

			// additional effect
// Need to add support for double cast. (currently done inside skill_attack)
//			struct status_change *sc_data = status_get_sc_data(&this->caster);
//			if( sc_data && sc_data[SC_DOUBLECAST].timer != -1 &&
//				rand() % 100 < 40+10*sc_data[SC_DOUBLECAST].val1.num)
//			{
//				skill_castend_delay(*src, *bl, skillid, skilllv, tick + dmg.div_*dmg.amotion, flag|1);
//			}
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
// Need to add support for double cast. (currently done inside skill_attack)
//			struct status_change *sc_data = status_get_sc_data(&this->caster);
//			if( sc_data && sc_data[SC_DOUBLECAST].timer != -1 &&
//				rand() % 100 < 40+10*sc_data[SC_DOUBLECAST].val1.num)
//			{
//				skill_castend_delay(*src, *bl, skillid, skilllv, tick + dmg.div_*dmg.amotion, flag|1);
//			}
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
/// create a target skill.
skillbase* skillbase::create(fightable& caster, ushort skillid, ushort skilllv, uint32 targetid)
{	// id2object for target skills
	skillbase* skill = NULL;
	switch(skillid)
	{
	case dummyskill::SKILLID:	skill = new dummyskill(caster, "targetskill"); break;

	case skill_sm_bash::SKILLID:    skill = new skill_sm_bash(caster, skilllv, targetid); break;
	case skill_sm_provoke::SKILLID: skill = new skill_sm_provoke(caster, skilllv, targetid); break;
	case skill_sm_magnum::SKILLID: skill = new skill_sm_magnum(caster, skilllv, targetid); break;
	case skill_sm_endure::SKILLID: skill = new skill_sm_endure(caster, skilllv, targetid); break;
	case skill_mg_sight::SKILLID: skill = new skill_mg_sight(caster, skilllv, targetid); break;
	case skill_mg_napalmbeat::SKILLID: skill = new skill_mg_napalmbeat(caster, skilllv, targetid); break;
	case skill_mg_soulstrike::SKILLID: skill = new skill_mg_soulstrike(caster, skilllv, targetid); break;
	case skill_mg_coldbolt::SKILLID: skill = new skill_mg_coldbolt(caster, skilllv, targetid); break;
	case skill_mg_frostdiver::SKILLID: skill = new skill_mg_frostdiver(caster, skilllv, targetid); break;
	case skill_mg_stonecurse::SKILLID: skill = new skill_mg_stonecurse(caster, skilllv, targetid); break;
	case skill_mg_fireball::SKILLID: skill = new skill_mg_fireball(caster, skilllv, targetid); break;
	case skill_mg_firebolt::SKILLID: skill = new skill_mg_firebolt(caster, skilllv, targetid); break;
	case skill_mg_lightningbolt::SKILLID: skill = new skill_mg_lightningbolt(caster, skilllv, targetid); break;

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
					/// initialize timer.
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
		{
			skill->caster.skill_failed(skillid, errcode);
		}
		// delete the skill, either failed or already executed
		delete skill;
		skill=NULL;
	}
	//called create with a invalid skillid
}
