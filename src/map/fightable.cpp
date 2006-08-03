#include "showmsg.h"
#include "timer.h"

#include "map.h"
#include "battle.h"
#include "clif.h"
#include "fightable.h"
#include "mob.h"
#include "npc.h"
#include "pc.h"
#include "pet.h"
#include "homun.h"
#include "skill.h"
#include "status.h"


//## change to static initializer when timer function management got classified
static void _fightable_init()
{
	add_timer_func_list(fightable::attacktimer_entry, "attacktimer entry function");
	add_timer_func_list(fightable::skilltimer_entry, "skilltimer entry function");
}
///////////////////////////////////////////////////////////////////////////////
/// constructor
fightable::fightable() : 
	attacktimer(-1),
	skilltimer(-1),
	attackable_tick(0),
	target_id(0)
{
	static bool initialiser = (_fightable_init(), true);
}


/// sets the object to idle state
bool fightable::set_idle()
{
	this->stop_attack();
	this->stop_skill();
	return this->movable::set_idle();
}

bool fightable::is_movable()
{
	if( DIFF_TICK(this->canmove_tick, gettick()) > 0 )
		return false;

	map_session_data *sd = this->get_sd();
	mob_data *md = this->get_md();
	pet_data *pd = this->get_pd();
	homun_data *hd = this->get_hd();
	if( sd )
	{
		if( sd->skilltimer != -1 && !pc_checkskill(*sd, SA_FREECAST) ||
			pc_issit(*sd) )
			return false;
	}
	else if( md )
	{
		if( md->skilltimer != -1 )
			return false;
	}
	else if( pd )
	{
		if(pd->state.casting_flag )
			return false;
	}
	else if( hd )
	{
		if( hd->skilltimer != -1 )
			return false;
		if( hd->attacktimer != -1 )
			return false;
	}	
	else
		return false;

	struct status_change *sc_data = status_get_sc_data(this);
	if (sc_data)
	{
		if(	sc_data[SC_ANKLE].timer != -1 ||
			sc_data[SC_AUTOCOUNTER].timer !=-1 ||
			sc_data[SC_TRICKDEAD].timer !=-1 ||
			sc_data[SC_BLADESTOP].timer !=-1 ||
			sc_data[SC_BLADESTOP_WAIT].timer !=-1 ||
			sc_data[SC_SPIDERWEB].timer !=-1 ||
			(sc_data[SC_DANCING].timer !=-1 && (
				(sc_data[SC_DANCING].val4.num && sc_data[SC_LONGING].timer == -1) ||
				sc_data[SC_DANCING].val1.num == CG_HERMODE	//cannot move while Hermod is active.
			)) ||
//			sc_data[SC_STOP].timer != -1 ||
//			sc_data[SC_CLOSECONFINE].timer != -1 ||
//			sc_data[SC_CLOSECONFINE2].timer != -1 ||
			sc_data[SC_MOONLIT].timer != -1 ||
			(sc_data[SC_GOSPEL].timer !=-1 && sc_data[SC_GOSPEL].val4.num == BCT_SELF) // cannot move while gospel is in effect
			)
			return false;
	}
	return true;
}


/// starts attack
bool fightable::start_attack(uint32 target_id, bool cont)
{
	struct block_list *bl=map_id2bl(target_id);
	return (bl)?this->start_attack(*bl,cont):false;
}
/// starts attack
bool fightable::start_attack(const block_list& target_bl, bool cont)
{
	if( target_bl.get_nd() && this->get_sd() )
	{	
		npc_click(*this->get_sd(), target_bl.block_list::id);
	}
	else if( this->get_sd() && battle_check_target(this->get_sd(), &target_bl, BCT_ENEMY) <= 0)
		return false;
	else
	{
		if( this->attacktimer != -1 )
		{
			delete_timer(this->attacktimer, fightable::attacktimer_entry);
			this->attacktimer = -1;
		}

		this->target_id = target_bl.block_list::id;
		this->attack_continue = cont;

		int d=DIFF_TICK(this->attackable_tick,gettick());
		if( d>0 && d<2000 )
		{	// attack with delay
			this->attacktimer=add_timer(this->attackable_tick, fightable::attacktimer_entry, this->block_list::id, 0);
		}
		else
		{	// call it directly
			fightable::attacktimer_entry(-1, gettick(), this->block_list::id, 0);
		}
	}
	return true;
}
/// stops attack
bool fightable::stop_attack()
{
	if(this->attacktimer!=-1)
	{
		delete_timer(this->attacktimer, fightable::attacktimer_entry);
		this->attacktimer = -1;
		this->target_id=0;
		this->attack_continue=0;
		return true;
	}
	return false;
}

/// stops skill
bool fightable::stop_skill()
{
	if(this->skilltimer!=-1)
	{
		delete_timer(this->skilltimer, fightable::skilltimer_entry);
		this->skilltimer = -1;
	}
	return true;
}

// old timer entry function
int fightable::attacktimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	block_list* bl = map_id2bl(id);
	fightable* mv;
	if( bl && (mv=bl->get_fightable()) )
	{
		if(mv->attacktimer != tid)
		{
			if(config.error_log)
				ShowError("attacktimer_entry %d != %d\n",mv->attacktimer,tid);
			return 0;
		}
		// timer was executed, clear it
		mv->attacktimer = -1;

		// call the user function
		mv->attacktimer_func(tid, tick, id, data);
	}
	return 0;
}

/// default function for physical attacks.
// only used for homunculus right now; 
// needs combining with other object functions since those are very similar
int fightable::attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data)
{
	block_list *target_bl = map_id2bl(this->target_id);
	fightable *target_fi;
	short range;
	if( NULL != target_bl &&												// target exists
		target_bl->prev != NULL &&											// and is spawned
		NULL != (target_fi=target_bl->get_fightable()) &&					// and is fightable
		target_fi->block_list::m == this->block_list::m &&					// and is on the same map as we are
		distance(*this, *target_fi) <= (range = this->get_attackrange()) &&	// and is within attack range
		target_fi->is_attackable() &&										// and is attackable
		this->can_attack(*target_fi) )										// and we can attack it
	{
		// stop movements
		this->stop_walking();

		// look at the target
		this->set_dir(*target_fi);

		// do the object code for the attack
		this->do_attack();

		// attack it
		//int target_lv = 
			battle_weapon_attack(this,target_fi,tick,0);

		// set the tick for the next possible attack
		this->attackable_tick = tick + status_get_adelay(this);

		// set the timer for the next attack when attacking continiously
		if(attack_continue)
			this->attacktimer=add_timer(this->attackable_tick,fightable::attacktimer_entry,this->block_list::id,0);
	}
	else
	{	// otherwise failed, so just stop further attacks
		this->target_id=0;
		this->attack_continue=0;
	}
	return 0;
}


// old timer entry function
int fightable::skilltimer_entry(int tid, unsigned long tick, int id, basics::numptr data)
{
	// pets are using timer data to transfer the castend_delay object 
	// so we need to send the tid and the data down to the objects
	// to allow cleaning
	// ...at least until the pet mess got cleaned out...

	// also need to seperate between target and ground skills here

	block_list* bl = map_id2bl(id);
	fightable* mv;
	if( bl && (mv=bl->get_fightable()) )
	{
		if(mv->skilltimer != tid)
		{
			if(config.error_log)
				ShowError("skilltimer_entry %d != %d\n",mv->skilltimer,tid);
			return 0;
		}
		// timer was executed, clear it
		mv->skilltimer = -1;

		// call the user function
		mv->skilltimer_func(tid, tick, id, data);
	}

	return 0;
}



/*


int unit_skilluse_id(struct block_list *src, int target_id, int skill_num, int skill_lv) {

	if(skill_num < 0) return 0;

	return unit_skilluse_id2(
		src, target_id, skill_num, skill_lv,
		skill_castfix(src, skill_num, skill_lv),
		skill_get_castcancel(skill_num)
	);
}

int unit_skilluse_id2(struct block_list *src, int target_id, int skill_num, int skill_lv, int casttime, int castcancel) {
	struct unit_data *ud;
	struct status_data *tstatus;
	struct status_change *sc;
	struct map_session_data *sd = NULL;
	struct block_list * target = NULL;
	unsigned int tick = gettick();
	int temp;

	nullpo_retr(0, src);
	if(src->is_dead())
		return 0; // 死んでいないか

	if( BL_CAST( BL_PC,  src, sd ) ) {
		ud = &sd->ud;
	} else
		ud = unit_bl2ud(src);

	if(ud == NULL) return 0;
	sc = status_get_sc(src);	
	if (sc && !sc->count)
		sc = NULL; //Unneeded
	//temp: used to signal combo-skills right now.
	temp = (target_id == src->id && !(sd && sd->state.skill_flag)
		&& skill_get_inf(skill_num)&INF_SELF_SKILL
		&& skill_get_inf2(skill_num)&INF2_NO_TARGET_SELF);
	if (temp)
		target_id = ud->target; //Auto-select skills. [Skotlex]

	if (sd) {
		//Target_id checking.
		if(skillnotok(skill_num, sd)) // [MouseJstr]
			return 0;
		switch(skill_num)
		{	//Check for skills that auto-select target
		case MO_CHAINCOMBO:
			if (sc && sc->data[SC_BLADESTOP].timer != -1){
				if ((target=(struct block_list *)sc->data[SC_BLADESTOP].val4) == NULL)
					return 0;
			}
			break;
		case TK_JUMPKICK:
		case TK_COUNTER:
		case HT_POWER:
			if (sc && sc->data[SC_COMBO].timer != -1 && sc->data[SC_COMBO].val1 == skill_num)
				target_id = sc->data[SC_COMBO].val2;
			break;
		case WE_MALE:
		case WE_FEMALE:
			if (!sd->status.partner_id)
				return 0;
			target = (struct block_list*)map_charid2sd(sd->status.partner_id);
			if (!target) {
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			break;
		}
		if (target)
			target_id = target->id;
	}
	if(!target && (target=map_id2bl(target_id)) == NULL )
		return 0;
	if(src->m != target->m)
		return 0; // 同じマップかどうか
	if(!src->prev || !target->prev)
		return 0; // map 上に存在するか

	//Normally not needed because clif.c checks for it, but the at/char/script commands don't! [Skotlex]
	if(ud->skilltimer != -1 && skill_num != SA_CASTCANCEL)
		return 0;

	if(skill_get_inf2(skill_num)&INF2_NO_TARGET_SELF && src->id == target_id)
		return 0;

	if(!status_check_skilluse(src, target, skill_num, 0))
		return 0;

	tstatus = status_get_status_data(target);
	//直前のスキル状況の記録
	if(sd) {
		switch(skill_num){
		case SA_CASTCANCEL:
			if(ud->skillid != skill_num){
				sd->skillid_old = ud->skillid;
				sd->skilllv_old = ud->skilllv;
				break;
			}
		case BD_ENCORE:	
			//Prevent using the dance skill if you no longer have the skill in your tree. 
			if(!sd->skillid_dance || pc_checkskill(sd,sd->skillid_dance)<=0){
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			sd->skillid_old = skill_num;
			break;
		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
		case CG_MOONLIT:
			if (config.player_skill_partner_check &&
				(!config.gm_skilluncond || sd.isGM() < config.gm_skilluncond) &&
				(skill_check_pc_partner(sd, skill_num, &skill_lv, 1, 0) < 1)
			) {
				clif_skill_fail(sd,skill_num,0,0);
				return 0;
			}
			break;
		}
		if (!skill_check_condition(sd, skill_num, skill_lv, 0))
			return 0;	
	}
	//TODO: Add type-independant skill_check_condition function.
	if (src->type == BL_MOB) {
		switch (skill_num) {
			case NPC_SUMMONSLAVE:
			case NPC_SUMMONMONSTER:
			case AL_TELEPORT:
				if (((TBL_MOB*)src)->master_id && ((TBL_MOB*)src)->special_state.ai)
					return 0;
		}
	}

	if(src->id != target_id &&
		!battle_check_range(src,target,skill_get_range2(src, skill_num,skill_lv)
		+(skill_num==RG_CLOSECONFINE?0:1))) //Close confine is exploitable thanks to this extra range "feature" of the client. [Skotlex]
		return 0;

	if (!temp) //Stop attack on non-combo skills [Skotlex]
		unit_stop_attack(src);
	else if(ud->attacktimer != -1) //Elsewise, delay current attack sequence
		ud->attackable_tick = tick + status_get_adelay(src);
	
	ud->state.skillcastcancel = castcancel;

	//temp: Used to signal force cast now.
	temp = 0;
	
	switch(skill_num){
	case ALL_RESURRECTION:
		if(battle_check_undead(tstatus->race,tstatus->def_ele)){	
			temp=1;
			casttime = skill_castfix(src, PR_TURNUNDEAD, skill_lv);
		}
		break;
	case MO_FINGEROFFENSIVE:
		if(sd)
			casttime += casttime * ((skill_lv > sd->spiritball)? sd->spiritball:skill_lv);
		break;
	case MO_EXTREMITYFIST:
		if (sc && sc->data[SC_COMBO].timer != -1 &&
			(sc->data[SC_COMBO].val1 == MO_COMBOFINISH ||
			sc->data[SC_COMBO].val1 == CH_TIGERFIST ||
			sc->data[SC_COMBO].val1 == CH_CHAINCRUSH))
			casttime = 0;
		temp = 1;
		break;
	case TK_RUN:
		if (sc && sc->data[SC_RUN].timer != -1)
			casttime = 0;
		break;
	case SA_MAGICROD:
	case SA_SPELLBREAKER:
		temp =1;
		break;
	case KN_CHARGEATK:
		//Taken from jA: Casttime is increased by dist/3*100%
		casttime = casttime * ((distance_bl(src,target)-1)/3+1);
		break;
	}

	if (sc && sc->data[SC_MEMORIZE].timer != -1 && casttime > 0) {
		casttime = casttime/2;
		if ((--sc->data[SC_MEMORIZE].val2) <= 0)
			status_change_end(src, SC_MEMORIZE, -1);
	}

	if( casttime>0 || temp){ 

		clif_skillcasting(src, src->id, target_id, 0,0, skill_num,casttime);

		if (sd && target->type == BL_MOB)
		{
			TBL_MOB *md = (TBL_MOB*)target;
			mobskill_event(md, src, tick, -1); //Cast targetted skill event.
			//temp: used to store mob's mode now.
			if (tstatus->mode&MD_CASTSENSOR &&
				battle_check_target(target, src, BCT_ENEMY) > 0)
			{
				switch (md->state.skillstate) {
				case MSS_ANGRY:
				case MSS_RUSH:
				case MSS_FOLLOW:
					if (!(tstatus->mode&(MD_AGGRESSIVE|MD_ANGRY)))
						break; //Only Aggressive mobs change target while chasing.
				case MSS_IDLE:
				case MSS_WALK:
					md->target_id = src->id;
					md->state.aggressive = (temp&MD_ANGRY)?1:0;
					md->min_chase = md->db->range3;
				}
			}
		}
	}

	if( casttime<=0 )
		ud->state.skillcastcancel=0;

	ud->canact_tick  = tick + casttime + 100;
	ud->skilltarget  = target_id;
	ud->skillx       = 0;
	ud->skilly       = 0;
	ud->skillid      = skill_num;
	ud->skilllv      = skill_lv;

 	if(sc && sc->data[SC_CLOAKING].timer != -1 &&
		!(sc->data[SC_CLOAKING].val4&1) && skill_num != AS_CLOAKING)
		status_change_end(src,SC_CLOAKING,-1);

	if(casttime > 0) {
		ud->skilltimer = add_timer( tick+casttime, skill_castend_id, src->id, 0 );
		if(sd && pc_checkskill(sd,SA_FREECAST))
			status_freecast_switch(sd);
		else
			unit_stop_walking(src,1);
	}
	else
		skill_castend_id(ud->skilltimer,tick,src->id,0);
	return 1;
}

int unit_skilluse_pos(struct block_list *src, int skill_x, int skill_y, int skill_num, int skill_lv) {
	if(skill_num < 0)
		return 0;
	return unit_skilluse_pos2(
		src, skill_x, skill_y, skill_num, skill_lv,
		skill_castfix(src, skill_num, skill_lv),
		skill_get_castcancel(skill_num)
	);
}

int unit_skilluse_pos2( struct block_list *src, int skill_x, int skill_y, int skill_num, int skill_lv, int casttime, int castcancel) {
	struct map_session_data *sd = NULL;
	struct unit_data        *ud = NULL;
	struct status_change *sc;
	struct block_list    bl;
	unsigned int tick = gettick();

	nullpo_retr(0, src);

	if(!src->prev) return 0; // map 上に存在するか
	if(src->is_dead()) return 0;

	if( BL_CAST( BL_PC, src, sd ) ) {
		ud = &sd->ud;
	} else
		ud = unit_bl2ud(src);
	if(ud == NULL) return 0;

	if(ud->skilltimer != -1) //Normally not needed since clif.c checks for it, but at/char/script commands don't! [Skotlex]
		return 0;
	
	sc = status_get_sc(src);
	if (sc && !sc->count)
		sc = NULL;
	
	if(sd) {
		if (skillnotok(skill_num, sd) ||
			!skill_check_condition(sd, skill_num, skill_lv,0))
		return 0;
	} 
	
	if (!status_check_skilluse(src, NULL, skill_num, 0))
		return 0;

	if (map_getcell(src->m, skill_x, skill_y, CELL_CHKNOREACH))
	{	//prevent casting ground targeted spells on non-walkable areas. [Skotlex] 
		if (sd) clif_skill_fail(sd,skill_num,0,0);
		return 0;
	}

	// 射程と障害物チェック 
	bl.type = BL_NUL;
	bl.m = src->m;
	bl.x = skill_x;
	bl.y = skill_y;
	if(skill_num != TK_HIGHJUMP &&
		!battle_check_range(src,&bl,skill_get_range2(src, skill_num,skill_lv)+1))
		return 0;

	unit_stop_attack(src);
	ud->state.skillcastcancel = castcancel;

	if (sc && sc->data[SC_MEMORIZE].timer != -1 && casttime > 0){
		casttime = casttime/3;
		if ((--sc->data[SC_MEMORIZE].val2)<=0)
			status_change_end(src, SC_MEMORIZE, -1);
	}

	if( casttime>0 ) {
		unit_stop_walking( src, 1);
		clif_skillcasting(src, src->id, 0, skill_x,skill_y, skill_num,casttime);
	}

	if( casttime<=0 )
		ud->state.skillcastcancel=0;

	ud->canact_tick  = tick + casttime + 100;
	ud->skillid      = skill_num;
	ud->skilllv      = skill_lv;
	ud->skillx       = skill_x;
	ud->skilly       = skill_y;
	ud->skilltarget  = 0;

	if (sc && sc->data[SC_CLOAKING].timer != -1 &&
		!(sc->data[SC_CLOAKING].val4&1))
		status_change_end(src,SC_CLOAKING,-1);

	if(casttime > 0) {
		ud->skilltimer = add_timer( tick+casttime, skill_castend_pos, src->id, 0 );
		if(sd && pc_checkskill(sd,SA_FREECAST))
			status_freecast_switch(sd);
		else
			unit_stop_walking(src,1);
	}
	else {
		ud->skilltimer = -1;
		skill_castend_pos(ud->skilltimer,tick,src->id,0);
	}
	return 1;
}

static int unit_attack_timer(int tid,unsigned int tick,int id,int data);

int unit_stop_attack(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_retr(0, bl);

	if(!ud || ud->attacktimer == -1)
		return 0;

	delete_timer( ud->attacktimer, unit_attack_timer );
	ud->attacktimer = -1;
	ud->target = 0;
	return 0;
}

//Means current target is unattackable. For now only unlocks mobs.
int unit_unattackable(struct block_list *bl) {
	struct unit_data *ud = unit_bl2ud(bl);
	if (ud) {
		ud->target = 0;
		ud->state.attack_continue = 0;
	}
	
	if(bl->type == BL_MOB)
		((struct mob_data*)bl)->unlock_target(gettick()) ;
	else if(bl->type == BL_PET)
		((struct pet_data*)bl)->unlock_target();
	return 0;
}


int unit_attack(struct block_list *src,int target_id,int type)
{
	struct block_list *target;
	struct unit_data  *ud;

	nullpo_retr(0, ud = unit_bl2ud(src));

	target=map_id2bl(target_id);
	if(target==NULL || target->is_dead()) {
		unit_unattackable(src);
		return 1;
	}

	if(src->type == BL_PC && target->type==BL_NPC) { // monster npcs [Valaris]
		npc_click((TBL_PC*)src,target); // submitted by leinsirk10 [Celest]
		return 0;
	}

	if(battle_check_target(src,target,BCT_ENEMY)<=0 ||
		!status_check_skilluse(src, target, 0, 0)
	) {
		unit_unattackable(src);
		return 1;
	}

	ud->target = target_id;
	ud->state.attack_continue = type;
	if (type) //If you re to attack continously, set to auto-case character
		ud->chaserange = status_get_range(src);
	
	//Just change target/type. [Skotlex]
	if(ud->attacktimer != -1)
		return 0;

	if(DIFF_TICK(ud->attackable_tick, gettick()) > 0)
		//Do attack next time it is possible. [Skotlex]
		ud->attacktimer=add_timer(ud->attackable_tick,unit_attack_timer,src->id,0);
	else //Attack NOW.
		unit_attack_timer(-1,gettick(),src->id,0);

	return 0;
}



static int unit_attack_timer_sub(struct block_list* src, int tid, unsigned int tick)
{
	struct block_list *target;
	struct unit_data *ud;
	struct status_data *sstatus;
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	int range;
	
	if((ud=unit_bl2ud(src))==NULL)
		return 0;
	if(ud->attacktimer != tid){
		if(config.error_log)
			ShowError("unit_attack_timer %d != %d\n",ud->attacktimer,tid);
		return 0;
	}
	BL_CAST( BL_PC , src, sd);
	BL_CAST( BL_MOB, src, md);
	ud->attacktimer=-1;
	target=map_id2bl(ud->target);

	if(src->prev == NULL || target==NULL || target->prev == NULL)
		return 0;

	if(ud->skilltimer != -1 && (!sd || pc_checkskill(sd,SA_FREECAST) <= 0))
		return 0;
	
	if(src->m != target->m || src->is_dead() || target->is_dead() || !status_check_skilluse(src, target, 0, 0))
		return 0;

	sstatus = status_get_status_data(src);

	if(!config.skill_delay_attack_enable &&
		DIFF_TICK(ud->canact_tick,tick) > 0 && 
		(!sd || pc_checkskill(sd,SA_FREECAST) <= 0)
	) {
		if (tid == -1) { //requested attack.
			if(sd) clif_skill_fail(sd,1,4,0);
			return 0;
		}
		//Otherwise, we are in a combo-attack, delay this until your canact time is over. [Skotlex]
		if(ud->state.attack_continue) {
			if (DIFF_TICK(ud->canact_tick, ud->attackable_tick) > 0)
				ud->attackable_tick = ud->canact_tick;
			ud->attacktimer=add_timer(ud->attackable_tick,unit_attack_timer,src->id,0);
		}
		return 1;
	}

	range = sstatus->rhw.range;
	
	if(!sd || sd->status.weapon != W_BOW) range++; //Dunno why everyone but bows gets this extra range...
	if(unit_is_walking(target)) range++; //Extra range when chasing

	if(!check_distance_bl(src,target,range) ) {
		//Chase if required.
		if(ud->state.attack_continue) {
			if(sd)
				clif_movetoattack(sd,target);
			else
				unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		}
		return 1;
	}
	if(!battle_check_range(src,target,range)) {
	  	//Within range, but no direct line of attack
		if(ud->state.attack_continue) {
			if(ud->chaserange > 2) ud->chaserange-=2;
			unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		}
		return 1;
	}

	//Sync packet only for players.
	//Non-players use the sync packet on the walk timer. [Skotlex]
	if (tid == -1 && sd) clif_fixpos(src);

	if(DIFF_TICK(ud->attackable_tick,tick) <= 0) {
		if (config.attack_direction_change &&
			(src->type&config.attack_direction_change)) {
			ud->dir = map_calc_dir(src, target->x,target->y );
			if (sd) sd->head_dir = ud->dir;
		}
		if(ud->walktimer != -1)
			unit_stop_walking(src,1);
		if(md) {
			if (mobskill_use(md,tick,-1))
				return 1;
			if (sstatus->mode&MD_ASSIST && DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME)
			{	// Link monsters nearby [Skotlex]
				md->last_linktime = tick;
				map_foreachinrange(mob_linksearch, src, md->db->range2,
					BL_MOB, md->class_, target, tick);
			}
		}
		if(src->type == BL_PET && pet_attackskill((TBL_PET*)src, target->id))
			return 1;
		
		map_freeblock_lock();
		ud->attacktarget_lv = battle_weapon_attack(src,target,tick,0);

		if(sd && sd->status.pet_id > 0 && sd->pd && config.pet_attack_support)
			pet_target_check(sd,target,0);
		map_freeblock_unlock();

		ud->attackable_tick = tick + sstatus->adelay;
//		You can't move if you can't attack neither.
		unit_set_walkdelay(src, tick, sstatus->amotion, 1);
	}

	if(ud->state.attack_continue)
		ud->attacktimer = add_timer(ud->attackable_tick,unit_attack_timer,src->id,0);

	return 1;
}

static int unit_attack_timer(int tid,unsigned int tick,int id,int data) {
	struct block_list *bl;
	bl = map_id2bl(id);
	if(bl && unit_attack_timer_sub(bl, tid, tick) == 0)
		unit_unattackable(bl);
	return 0;
}

int unit_skillcastcancel(struct block_list *bl,int type)
{
	struct map_session_data *sd = NULL;
	struct unit_data *ud = unit_bl2ud( bl);
	unsigned int tick=gettick();
	int ret=0, skill;
	
	nullpo_retr(0, bl);
	if (!ud || ud->skilltimer==-1)
		return 0; //Nothing to cancel.

	BL_CAST(BL_PC,  bl, sd);

	if (type&2) {
		//See if it can be cancelled.
		if (!ud->state.skillcastcancel)
			return 0;

		if (sd && (sd->special_state.no_castcancel2 ||
			(sd->special_state.no_castcancel && !map_flag_gvg(bl->m)))) //fixed flags being read the wrong way around [blackhole89]
			return 0;
	}
	
	ud->canact_tick=tick;
	if(sd && pc_checkskill(sd,SA_FREECAST))
		status_freecast_switch(sd);
	
	if(type&1 && sd)
		skill = sd->skillid_old;
	else
		skill = ud->skillid;
	
	if (skill_get_inf(skill) & INF_GROUND_SKILL)
		ret=delete_timer( ud->skilltimer, skill_castend_pos );
	else
		ret=delete_timer( ud->skilltimer, skill_castend_id );
	if(ret<0)
		ShowError("delete timer error : skillid : %d\n",ret);
	
	if(bl->type==BL_MOB) ((TBL_MOB*)bl)->skillidx  = -1;

	ud->skilltimer = -1;
	clif_skillcastcancel(bl);
	return 1;
}

// unit_data の初期化処理
void unit_dataset(struct block_list *bl) {
	struct unit_data *ud;
	nullpo_retv(ud = unit_bl2ud(bl));

	memset( ud, 0, sizeof( struct unit_data) );
	ud->bl             = bl;
	ud->walktimer      = -1;
	ud->skilltimer     = -1;
	ud->attacktimer    = -1;
	ud->attackable_tick = 
	ud->canact_tick    = 
	ud->canmove_tick   = gettick();
}

static int unit_counttargeted_sub(struct block_list *bl, va_list ap)
{
	int id, target_lv;
	struct unit_data *ud;
	id = va_arg(ap,int);
	target_lv = va_arg(ap,int);
	if(bl->id == id)
		return 0;

	ud = unit_bl2ud(bl);

	if (ud && ud->target == id && ud->attacktimer != -1 && ud->attacktarget_lv >= target_lv)
		return 1;

	return 0;	
}

int unit_fixdamage(struct block_list *src,struct block_list *target,unsigned int tick,int sdelay,int ddelay,int damage,int div,int type,int damage2)
{
	nullpo_retr(0, target);

	if(damage+damage2 <= 0)
		return 0;
	
	return status_fix_damage(src,target,damage+damage2,clif_damage(target,target,tick,sdelay,ddelay,damage,div,type,damage2));
}

int unit_counttargeted(struct block_list *bl,int target_lv)
{
	nullpo_retr(0, bl);
	return (map_foreachinrange(unit_counttargeted_sub, bl, AREA_SIZE, BL_CHAR,
		bl->id, target_lv));
}


int unit_remove_map(struct block_list *bl, int clrtype)
{
	struct unit_data *ud = unit_bl2ud(bl);
	struct status_change *sc = status_get_sc(bl);
	nullpo_retr(0, ud);

	if(bl->prev == NULL)
		return 0; //Already removed?

	map_freeblock_lock();

	ud->target = 0; //Unlock walk/attack target.
	if (ud->walktimer != -1)
		unit_stop_walking(bl,0);
	if (ud->attacktimer != -1)
		unit_stop_attack(bl);
	if (ud->skilltimer != -1)
		unit_skillcastcancel(bl,0);
	ud->attackable_tick = ud->canmove_tick = ud->canact_tick = gettick();
	clif_clearchar_area(bl,clrtype);
	
	if(sc && sc->count ) { //map-change/warp dispells.
		if(sc->data[SC_BLADESTOP].timer!=-1)
			status_change_end(bl,SC_BLADESTOP,-1);
		if(sc->data[SC_BASILICA].timer!=-1)
			status_change_end(bl,SC_BASILICA,-1);
		if(sc->data[SC_ANKLE].timer != -1)
			status_change_end(bl, SC_ANKLE, -1);
		if (sc->data[SC_TRICKDEAD].timer != -1)
			status_change_end(bl, SC_TRICKDEAD, -1);
		if (sc->data[SC_BLADESTOP].timer!=-1)
			status_change_end(bl,SC_BLADESTOP,-1);
		if (sc->data[SC_RUN].timer!=-1)
			status_change_end(bl,SC_RUN,-1);
		if (sc->data[SC_DANCING].timer!=-1) // clear dance effect when warping [Valaris]
			skill_stop_dancing(bl);
		if (sc->data[SC_DEVOTION].timer!=-1)
			status_change_end(bl,SC_DEVOTION,-1);
		if (sc->data[SC_MARIONETTE].timer!=-1)
			status_change_end(bl,SC_MARIONETTE,-1);
		if (sc->data[SC_MARIONETTE2].timer!=-1)
			status_change_end(bl,SC_MARIONETTE2,-1);
		if (sc->data[SC_CLOSECONFINE].timer!=-1)
			status_change_end(bl,SC_CLOSECONFINE,-1);
		if (sc->data[SC_CLOSECONFINE2].timer!=-1)
			status_change_end(bl,SC_CLOSECONFINE2,-1);
		if (sc->data[SC_HIDING].timer!=-1)
			status_change_end(bl, SC_HIDING, -1);
		if (sc->data[SC_CLOAKING].timer!=-1)
			status_change_end(bl, SC_CLOAKING, -1);
		if (sc->data[SC_CHASEWALK].timer!=-1)
			status_change_end(bl, SC_CHASEWALK, -1);
		if (sc->data[SC_GOSPEL].timer != -1 && sc->data[SC_GOSPEL].val4 == BCT_SELF)
			status_change_end(bl, SC_GOSPEL, -1);
	}

	if (bl->type&BL_CHAR) {
		skill_unit_move(bl,gettick(),4);
		skill_cleartimerskill(bl);			// タイマースキルクリア
	}

	if(bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data*)bl;

		//Leave/reject all invitations.
		if(sd->chatID)
			chat_leavechat(sd);
		if(sd->trade_partner)
			trade_tradecancel(sd);
		if(sd->vender_id)
			vending_closevending(sd);
		if(sd->state.storage_flag == 1)
			storage_storage_quit(sd,0);
		else if (sd->state.storage_flag == 2)
			storage_guild_storage_quit(sd,0);

		if(sd->party_invite>0)
			party_reply_invite(sd,sd->party_invite_account,0);
		if(sd->guild_invite>0)
			guild_reply_invite(sd,sd->guild_invite,0);
		if(sd->guild_alliance>0)
			guild_reply_reqalliance(sd,sd->guild_alliance_account,0);

		pc_stop_following(sd);
		pc_delinvincibletimer(sd);

		if(sd->pvp_timer!=-1) {
			delete_timer(sd->pvp_timer,pc_calc_pvprank_timer);
			sd->pvp_timer = -1;
		}

		if(pc_issit(sd)) {
			pc_setstand(sd);
			skill_gangsterparadise(sd,0);
			skill_rest(sd,0);
		}
		party_send_dot_remove(sd);//minimap dot fix [Kevin]
		guild_send_dot_remove(sd);
	} else if(bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data*)bl;
		md->target_id=0;
		md->attacked_id=0;
		md->state.skillstate= MSS_IDLE;
	} else if (bl->type == BL_PET) {
		struct pet_data *pd = (struct pet_data*)bl;
		struct map_session_data *sd = pd->msd;
		
		if(!sd) {
			map_delblock(bl);
			unit_free(bl);
			map_freeblock_unlock();
			return 0;
		}
		if (sd->pet.intimate <= 0)
		{	//Remove pet.
			intif_delete_petdata(sd->status.pet_id);
			sd->status.pet_id = 0;
			sd->pd = NULL;
			pd->msd = NULL;
			map_delblock(bl);
			unit_free(bl);
			map_freeblock_unlock();
			return 0;
		}
	}
	map_delblock(bl);
	map_freeblock_unlock();
	return 1;
}


int unit_free(struct block_list *bl) {
	struct unit_data *ud = unit_bl2ud( bl );
	nullpo_retr(0, ud);

	map_freeblock_lock();
	if( bl->prev )	//Players are supposed to logout with a "warp" effect.
		unit_remove_map(bl, bl->type==BL_PC?3:0);
	
	if( bl->type == BL_PC ) {
		struct map_session_data *sd = (struct map_session_data*)bl;
		if(bl->is_dead())
			pc_setrestartvalue(sd,2);

		//Status that are not saved...
		if(sd->sc.count) {
			if(sd->sc.data[SC_SPURT].timer!=-1)
				status_change_end(bl,SC_SPURT,-1);
			if(sd->sc.data[SC_BERSERK].timer!=-1)
				status_change_end(bl,SC_BERSERK,-1);
			if(sd->sc.data[SC_TRICKDEAD].timer!=-1)
				status_change_end(bl,SC_TRICKDEAD,-1);
			if (config.debuff_on_logout) {
				if(sd->sc.data[SC_ORCISH].timer!=-1)
					status_change_end(bl,SC_ORCISH,-1);
				if(sd->sc.data[SC_STRIPWEAPON].timer!=-1)
					status_change_end(bl,SC_STRIPWEAPON,-1);
				if(sd->sc.data[SC_STRIPARMOR].timer!=-1)
					status_change_end(bl,SC_STRIPARMOR,-1);
				if(sd->sc.data[SC_STRIPSHIELD].timer!=-1)
					status_change_end(bl,SC_STRIPSHIELD,-1);
				if(sd->sc.data[SC_STRIPHELM].timer!=-1)
					status_change_end(bl,SC_STRIPHELM,-1);
				if(sd->sc.data[SC_EXTREMITYFIST].timer!=-1)
					status_change_end(bl,SC_EXTREMITYFIST,-1);
				if(sd->sc.data[SC_EXPLOSIONSPIRITS].timer!=-1)
					status_change_end(bl,SC_EXPLOSIONSPIRITS,-1);
			}
		}
		// Notify friends that this char logged out. [Skotlex]
		clif_foreachclient(clif_friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 0);
		party_send_logout(sd);
		guild_send_memberinfoshort(sd,0);
		pc_cleareventtimer(sd);		
		pc_delspiritball(sd,sd->spiritball,1);
		chrif_save_scdata(sd); //Save status changes, then clear'em out from memory. [Skotlex]
		pc_makesavestatus(sd);
		sd->state.waitingdisconnect = 1;
		pc_clean_skilltree(sd);
	} else if( bl->type == BL_PET ) {
		struct pet_data *pd = (struct pet_data*)bl;
		struct map_session_data *sd = pd->msd;
		pet_hungry_timer_delete(pd);
		if (pd->a_skill)
		{
			aFree(pd->a_skill);
			pd->a_skill = NULL;
		}
		if (pd->s_skill)
		{
			if (pd->s_skill->timer != -1) {
				if (pd->s_skill->id)
					delete_timer(pd->s_skill->timer, pet_skill_support_timer);
				else
					delete_timer(pd->s_skill->timer, pet_heal_timer);
			}
			aFree(pd->s_skill);
			pd->s_skill = NULL;
		}
		if(pd->recovery)
		{
			if(pd->recovery->timer != -1)
				delete_timer(pd->recovery->timer, pet_recovery_timer);
			aFree(pd->recovery);
			pd->recovery = NULL;
		}
		if(pd->bonus)
		{
			if (pd->bonus->timer != -1)
				delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
			aFree(pd->bonus);
			pd->bonus = NULL;
		}
		if (pd->loot)
		{
			if (pd->loot->item)
				aFree(pd->loot->item);
			aFree (pd->loot);
			pd->loot = NULL;
		}
		if (sd) {
			if(sd->pet.intimate > 0)
				intif_save_petdata(sd->status.account_id,&sd->pet);
			sd->pd = NULL;
		}
	} else if(bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data*)bl;
		if(md->deletetimer!=-1)
			delete_timer(md->deletetimer,mob_timer_delete);
		md->deletetimer=-1;
		if(md->lootitem) {
			aFree(md->lootitem);
			md->lootitem=NULL;
		}
		if (md->guardian_data)
		{
			if (md->guardian_data->number < MAX_GUARDIANS)
				md->guardian_data->castle->guardian[md->guardian_data->number].id = 0;
			aFree(md->guardian_data);
			md->guardian_data = NULL;
		}
		if (md->spawn && md->spawn_n < 0 && --(md->spawn->num) == 0)
		{	//Spawning data is not attached to the map, so free it
			//if this is the last mob who is pointing at it.
			aFree(md->spawn);
			md->spawn = NULL;
		}
		if(md->base_status) {
			aFree(md->base_status);
			md->base_status = NULL;
		}
		if(mob_is_clone(md->class_))
			mob_clone_delete(md->class_);
	}

	skill_clear_unitgroup(bl);
	status_change_clear(bl,1);
	if (bl->type != BL_PC)
  	{	//Players are handled by map_quit
		map_deliddb(bl);
		map_freeblock(bl);
	}
	map_freeblock_unlock();
	return 0;
}


int unit_run(struct block_list *bl)
{
	struct status_change *sc = status_get_sc(bl);
	int i,to_x,to_y,dir_x,dir_y;

	if (!sc || !sc->count || sc->data[SC_RUN].timer == -1)
		return 0;
	
	if (!unit_can_move(bl)) {
		if(sc->data[SC_RUN].timer!=-1)
			status_change_end(bl,SC_RUN,-1);
		return 0;
	}
	
	to_x = bl->x;
	to_y = bl->y;
	dir_x = dirx[sc->data[SC_RUN].val2];
	dir_y = diry[sc->data[SC_RUN].val2];

	for(i=0;i<AREA_SIZE;i++)
	{
		if(!map_getcell(bl->m,to_x+dir_x,to_y+dir_y,CELL_CHKPASS))
			break;
		to_x += dir_x;
		to_y += dir_y;
	}

	if(to_x == bl->x && to_y == bl->y) {
		//If you can't run forward, you must be next to a wall, so bounce back. [Skotlex]
		status_change_end(bl,SC_RUN,-1);
		skill_blown(bl,bl,skill_get_blewcount(TK_RUN,sc->data[SC_RUN].val1)|0x10000);
		return 0;
	}
	unit_walktoxy(bl, to_x, to_y, 1);
	return 1;
}
*/
