// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "db.h"
#include "timer.h"
#include "socket.h"
#include "nullpo.h"
#include "malloc.h"
#include "pc.h"
#include "status.h"
#include "map.h"
#include "intif.h"
#include "clif.h"
#include "chrif.h"
#include "pet.h"
#include "itemdb.h"
#include "battle.h"
#include "mob.h"
#include "npc.h"
#include "script.h"
#include "skill.h"
#include "showmsg.h"
#include "utils.h"
#include "pc.h"

int petskill_castend(struct pet_data &pd,unsigned long tick, struct castend_delay *dat);
int petskill_castend2(struct pet_data &pd, block_list &target, unsigned short skill_id, unsigned short skill_lv, unsigned short skill_x, unsigned short skill_y, unsigned long tick);
int pet_attackskill(struct pet_data &pd, unsigned long tick, int data);


#define MIN_PETTHINKTIME 100

struct petdb pet_db[MAX_PET_DB];

static const char dirx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
static const char diry[8] = { 1, 1, 0,-1,-1,-1, 0, 1};




pet_data::~pet_data()
{
	skill_cleartimerskill(this);
	if(this->status)
	{
		delete this->status;
		this->status = NULL;
	}
	if(this->a_skill)
	{
		delete this->a_skill;
		this->a_skill = NULL;
	}
	if(this->s_skill)
	{
		if(this->s_skill->timer != -1)
		{
			if(this->s_skill->id == 0)
				delete_timer(this->s_skill->timer, pet_heal_timer);
			else
				delete_timer(this->s_skill->timer, pet_skill_support_timer);
		}
		delete this->s_skill;
		this->s_skill = NULL;
	}
	if(this->recovery)
	{
		if(this->recovery->timer != -1)
			delete_timer(this->recovery->timer, pet_recovery_timer);
		delete this->recovery;
		this->recovery = NULL;
	}
	if(this->bonus)
	{
		if (this->bonus->timer != -1)
			delete_timer(this->bonus->timer, pet_skill_bonus_timer);
		delete this->bonus;
		this->bonus = NULL;
	}
	if(this->loot)
	{
		if(this->loot->itemlist)
			delete this->loot->itemlist;
		delete this->loot;
		this->loot = NULL;
	}
	this->state.skillbonus=-1;

	if(this->hungry_timer != -1)
	{
		delete_timer(this->hungry_timer,pet_hungry_timer);
		this->hungry_timer = -1;
	}

	this->set_idle();

	this->msd.state.perfect_hiding=0;
	this->msd.pd = NULL;

	clif_clearchar_area(*this,0);
	this->delblock();
	this->deliddb();
}

int pet_data::attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data)
{
	pet_data &pd = *this;

	if( pd.msd.is_dead() )
	{	//Stop attacking when master died.
		pd.stop_attack();
		return 0;
	}
	if (pd.state.casting_flag) 
	{	//There is a skill being cast.
		petskill_castend(pd, tick, data.isptr?((struct castend_delay *)data.ptr):NULL );
		struct TimerData *td = get_timer(tid);
		if(td && td->data.isptr)
		{
			td->data = 0;
		}
		return 0;
	}
	if (config.pet_status_support &&
		pd.a_skill &&
		(!config.pet_equip_required || pd.pet.equip_id > 0) &&
		(rand()%100 < (pd.a_skill->rate +pd.pet.intimate*pd.a_skill->bonusrate/1000))
		)
	{	//Skotlex: Use pet's skill 
		pet_attackskill(pd,tick,data.num);
		return 0;
	}
	
	mob_data *md = mob_data::from_blid(pd.target_id);
	short range;

	if(md == NULL || pd.block_list::m != md->block_list::m || !md->is_on_map() ||
		distance(pd, *md) > 13)
	{
		pd.unlock_target();
		return 0;
	}

	range = mob_db[pd.pet.class_].range + 1;
	if(distance(pd, *md) > range)
		return 0;
	if(config.monster_attack_direction_change)
		pd.dir=pd.get_direction(*md);

	clif_fixobject(pd);

	pd.target_lv = battle_weapon_attack(&pd,md,tick,0);

	pd.attackable_tick = tick + status_get_adelay(&pd);

	if( pd.is_walking() || pd.is_attacking() )
		pd.set_idle();

	pd.attacktimer=add_timer(pd.attackable_tick,fightable::attacktimer_entry,pd.block_list::id,0);

	return 0;
}

int pet_data::skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data)
{
	return 0;
}

/// do object depending stuff for ending the walk.
void pet_data::do_stop_walking()
{
}
/// do object depending stuff for the walk step.
bool pet_data::do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy)
{
	return true;
}

// Random walk
bool pet_data::randomwalk(unsigned long tick)
{
	pet_data& pd = *this;
	const int retrycount=20;
	int speed = pd.get_speed();

	if(DIFF_TICK(pd.next_walktime,tick) < 0)
	{
		int i,x,y,d=12-pd.move_fail_count;
		if(d<5) d=5;
		for(i=0;i<retrycount;++i)
		{
			int r=rand();
			if(distance(pd, pd.msd)<d )
			{	// use master coordinates as base for random walk
				x=pd.msd.block_list::x+r%(d*2+1)-d;
				y=pd.msd.block_list::y+r/(d*2+1)%(d*2+1)-d;
			}
			else
			{
				x=pd.block_list::x+r%(d*2+1)-d;
				y=pd.block_list::y+r/(d*2+1)%(d*2+1)-d;
			}

			if( map_getcell(pd.block_list::m,x,y,CELL_CHKPASS) && pd.walktoxy(x,y) )
			{
				pd.move_fail_count=0;
				break;
			}
		}
		if( i>=retrycount && (++pd.move_fail_count)>1000 )
		{
			if(config.error_log)
				ShowMessage("PET cant move. hold position %d, class_ = %d\n",pd.block_list::id,pd.pet.class_);
			pd.move_fail_count=0;
			pd.set_delay(tick+60000);
		}
		else
		{
			pd.next_walktime = tick+rand()%3000+3000+speed*pd.walkpath.get_path_time()/10;
			return true;
		}
	}
	return false;
}





// Appearance income of mob
// actually a copy from mob but until pet and mob don't have a real heritage
// the best is to have both classes completely seperated
int pet_data::get_viewclass() const
{
	return mob_db[this->pet.class_].view_class;
}
int pet_data::get_sex() const
{
	return mob_db[this->pet.class_].sex;
}
ushort pet_data::get_hair() const
{
	return mob_db[this->pet.class_].hair;
}
ushort pet_data::get_hair_color() const
{
	return mob_db[this->pet.class_].hair_color;
}
ushort pet_data::get_weapon() const
{
	return mob_db[this->pet.class_].weapon;
}
ushort pet_data::get_shield() const
{
	return mob_db[this->pet.class_].shield;
}
ushort pet_data::get_head_top() const
{
	return mob_db[this->pet.class_].head_top;
}
ushort pet_data::get_head_mid() const
{
	return mob_db[this->pet.class_].head_mid;
}
ushort pet_data::get_head_buttom() const
{
	return mob_db[this->pet.class_].head_buttom;
}
ushort pet_data::get_clothes_color() const
{
	return mob_db[this->pet.class_].clothes_color;
}
int pet_data::get_equip() const
{
	return mob_db[this->pet.class_].equip;
}
int pet_data::get_range() const
{
	return mob_db[this->pet.class_].range;
}
int pet_data::get_race() const
{
	return mob_db[this->pet.class_].race;
}
int pet_data::get_race2() const
{
	return mob_db[this->pet.class_].race2;
}
int pet_data::get_mode() const
{
	return mob_db[this->pet.class_].mode;
}
int pet_data::get_mexp() const
{
	return mob_db[this->pet.class_].mexp;
}
int pet_data::get_size() const
{
	return mob_db[this->pet.class_].size;
}




bool pet_data::stop_attack()
{
//	if(this->state.state == MS_ATTACK)
//		this->set_idle();
	return this->fightable::stop_attack();
}



void pet_data::menu(int menunum)
{
	switch(menunum)
	{
		case 0:
			clif_send_petstatus(this->msd);
			break;
		case 1:
			this->food();
			break;
		case 2:
			this->performance();
			break;
		case 3:
			this->return_to_egg();
			break;
		case 4:
			pet_unequipitem(this->msd);
			break;
	}
}

void pet_data::food()
{
	map_session_data &sd = this->msd;
	int i,k,t;
	i=pc_search_inventory(sd,sd.pd->petDB.FoodID);
	if(i < 0)
	{
		clif_pet_food(sd,sd.pd->petDB.FoodID,0);
	}
	else
	{
		pc_delitem(sd,i,1,0);
		t = sd.pd->pet.intimate;
		if(sd.pd->pet.hungry > 90)
			sd.pd->pet.intimate -= sd.pd->petDB.r_full;
		else if(sd.pd->pet.hungry > 75)
		{
			if(config.pet_friendly_rate != 100)
				k = (sd.pd->petDB.r_hungry * config.pet_friendly_rate)/100;
			else
				k = sd.pd->petDB.r_hungry;
			k = k >> 1;
			if(k <= 0)
				k = 1;
			sd.pd->pet.intimate += k;
		}
		else
		{
			if(config.pet_friendly_rate != 100)
				k = (sd.pd->petDB.r_hungry * config.pet_friendly_rate)/100;
			else
				k = sd.pd->petDB.r_hungry;
			sd.pd->pet.intimate += k;
		}
		if(sd.pd->pet.intimate <= 0)
		{
			sd.pd->pet.intimate = 0;
			if(config.pet_status_support && t > 0)
			{
				if( sd.is_on_map() )
					status_calc_pc(sd,0);
				else
					status_calc_pc(sd,2);
			}
		}
		else if(sd.pd->pet.intimate > 1000)
			sd.pd->pet.intimate = 1000;
		status_calc_pet(sd, 0);
		sd.pd->pet.hungry += sd.pd->petDB.fullness;
		if(sd.pd->pet.hungry > 100)
			sd.pd->pet.hungry = 100;

		clif_send_petdata(sd,2,sd.pd->pet.hungry);
		clif_send_petdata(sd,1,sd.pd->pet.intimate);
		clif_pet_food(sd,sd.pd->petDB.FoodID,1);
	}
}

void pet_data::droploot(bool drop)
{
	size_t i,flag=0;
	if( this->loot )
	{	// is a looter
		for(i=0;i<this->loot->count;++i)
		{	// 落とさないで直接PCのItem欄へ
			if(drop)
			{	// create a delay drop structure
				struct delay_item_drop2 *ditem;
				ditem = new delay_item_drop2(*this, this->loot->itemlist[i]);
				add_timer(gettick()+540+i,pet_delay_item_drop2,0, basics::numptr(ditem), false);
			}
			else if((flag = pc_additem(this->msd, this->loot->itemlist[i], this->loot->itemlist[i].amount)))
			{	// drop items on floor
				clif_additem(this->msd,0,0,flag);
				map_addflooritem(this->loot->itemlist[i], this->loot->itemlist[i].amount, this->block_list::m, this->block_list::x, this->block_list::y, NULL, NULL, NULL, 0);
			}
		}
		this->loot->count = 0;
		this->loot->weight = 0;
		this->loot->loottick = gettick()+10000;	//	10*1000msの間拾わない
	}
}

int pet_performance_val(map_session_data &sd)
{
	if( sd.pd )
	{

	}
	return 0;
}

void pet_data::performance()
{
	this->stop_walking(2000<<8);
	int value =  ( this->pet.intimate > 900 ) ? ((this->petDB.s_perfor > 0) ? 4:3) : (( this->pet.intimate > 750) ? 2 : 1);
	clif_pet_performance(*this,1+rand()%value);
	// ルートしたItemを落とさせる
	this->droploot(true);
}

void pet_data::return_to_egg()
{
	struct item tmp_item;
	int flag;

	// ルートしたItemを落とさせる
	this->droploot();
	this->pet.incuvate = 1;

	tmp_item = (struct item)( this->petDB.EggID );
	tmp_item.identify = 1;
	tmp_item.card[0] = 0xff00;
	tmp_item.card[1] = basics::GetWord(this->pet.pet_id,0);
	tmp_item.card[2] = basics::GetWord(this->pet.pet_id,1);
	tmp_item.card[3] = this->pet.rename_flag;
	flag = pc_additem(this->msd, tmp_item, 1);
	if( flag )
	{
		clif_additem(this->msd,0,0,flag);
		map_addflooritem(tmp_item,1,this->msd.block_list::m,this->msd.block_list::x,this->msd.block_list::y,NULL,NULL,NULL,0);
	}

	this->msd.status.pet_id = 0;
	this->msd.pd = NULL;

	if(config.pet_status_support && this->pet.intimate > 0)
	{
		if( this->msd.is_on_map() )
			status_calc_pc(this->msd,0);
		else
			status_calc_pc(this->msd,2);
	}

	intif_save_petdata(this->msd.status.account_id, this->pet);
	pc_makesavestatus(this->msd);
	chrif_save(this->msd);
	storage_storage_save(this->msd);

	this->freeblock();
}








int pet_hungry_val(map_session_data &sd)
{
	if( sd.pd )
	{
		if(sd.pd->pet.hungry > 90)
			return 4;
		else if(sd.pd->pet.hungry > 75)
			return 3;
		else if(sd.pd->pet.hungry > 25)
			return 2;
		else if(sd.pd->pet.hungry > 10)
			return 1;
	}
	return 0;
}


/*==========================================
 * Pet Attack Skill [Skotlex]
 *------------------------------------------
 */
int pet_attackskill(struct pet_data &pd, unsigned long tick, int data)
{

	block_list *bl = block_list::from_blid(pd.target_id);
	if(bl == NULL || pd.block_list::m != bl->m || !bl->is_on_map() ||
		NULL==pd.a_skill ||
		distance(pd,*bl) > 13)
	{
		pd.unlock_target();
		return 0;
	}
	pet_skill_use(pd, *bl, pd.a_skill->id, pd.a_skill->lv, tick);
	return 0;
}

/*==========================================
 * Pet Skill Use [Skotlex]
 *------------------------------------------
 */
int pet_skill_use(struct pet_data &pd, block_list &target, short skill_id, short skill_lv, unsigned int tick)
{
	int casttime;

	if(pd.state.casting_flag)
		return 1;	//Will not interrupt an already casting skill.

	if( pd.is_walking() )	//Cancel whatever else the pet is doing.
		pd.set_idle();

	
	if(config.monster_attack_direction_change)
		pd.dir=pd.get_direction(target);
	clif_fixobject(pd);

	//Casting time
	casttime=skill_castfix(&pd, skill_get_cast(skill_id, skill_lv));
		
	pd.stop_walking(1);
	pd.attackable_tick = tick;

	if (casttime > 0)
	{
		pd.attackable_tick += casttime;
		pd.state.casting_flag = 1;

		if (skill_get_inf(skill_id) & INF_GROUND_SKILL)
			clif_skillcasting(pd, pd.block_list::id, 0, target.x, target.y, skill_id, casttime);
		else
			clif_skillcasting(pd, pd.block_list::id, target.id, 0, 0, skill_id,casttime);
		
		struct castend_delay *dat = new struct castend_delay(pd, target.id, skill_id, skill_lv, 0);
		pd.attacktimer = add_timer(pd.attackable_tick,fightable::attacktimer_entry,pd.block_list::id, basics::numptr(dat), false);
	}
	else
	{
		petskill_castend2(pd, target, skill_id, skill_lv, target.x, target.y, tick);
	}	
	return 0;
}

/*==========================================
 * Pet Attack Cast End [Skotlex]
 *------------------------------------------
 */
int petskill_castend(struct pet_data &pd,unsigned long tick, struct castend_delay *dat)
{
	if(dat)
	{
		block_list *target = block_list::from_blid(dat->target_id);
		pd.state.casting_flag = 0;
		if (target && dat->src.id == pd.block_list::id && target->is_on_map() )
			petskill_castend2(pd, *target, dat->skill_id, dat->skill_lv, target->x, target->y, tick);
		delete dat;
	}
	return 0;
}

/*==========================================
 * Pet Attack Cast End2 [Skotlex]
 *------------------------------------------
 */
int petskill_castend2(struct pet_data &pd, block_list &target, unsigned short skill_id, unsigned short skill_lv, unsigned short skill_x, unsigned short skill_y, unsigned long tick)
{	//Invoked after the casting time has passed.
	short delaytime =0, range;

	if (skill_get_inf(skill_id) & INF_GROUND_SKILL)
	{	//Area skill
		skill_castend_pos2(&pd, skill_x, skill_y, skill_id, skill_lv, tick,0);
	} else { //Targeted Skill
		//Skills with inf = 4 (cast on self) have view range (assumed party skills)
		range = (skill_get_inf(skill_id) & INF_SELF_SKILL?config.area_size:skill_get_range(skill_id, skill_lv));
		if(range < 0)
			range = pd.get_range() - (range + 1);
		if(distance(pd, target) > range)
			return 0; 
		switch( skill_get_nk(skill_id) )
		{
			case NK_NO_DAMAGE:
				if(
				(skill_id==AL_HEAL || skill_id==ALL_RESURRECTION) && target.is_undead() )
					skill_castend_damage_id(&pd, &target, skill_id, skill_lv, tick, 0);
				else
				{
				  skill_castend_nodamage_id(&pd,&target, skill_id, skill_lv,tick, 0);
				}
				break;
			case NK_SPLASH_DAMAGE:
			default:
				skill_castend_damage_id(&pd,&target,skill_id,skill_lv,tick,0);
				break;
		}
	}

	if (pd.walktimer != -1) //The above skill casting could had changed the state (Abracadabra?)
		return 0;

	delaytime = skill_delayfix(&pd,skill_get_delay(skill_id, skill_lv));
	if (delaytime < MIN_PETTHINKTIME)
		delaytime = status_get_adelay(&pd);
	pd.attackable_tick = tick + delaytime; 
	pd.attacktimer=add_timer(pd.attackable_tick,fightable::attacktimer_entry,pd.block_list::id,0);

	return 0;
}



int pet_target_check(map_session_data &sd,block_list *bl,int type)
{
	struct pet_data *pd= sd.pd;
	struct mob_data *md= (bl)?bl->get_md():NULL;
	if( md && pd && 
		pd->pet.intimate >= (short)config.pet_support_min_friendly &&
		pd->pet.hungry >= 1 &&
		pd->pet.class_ != bl->get_class() &&
		pd->can_act() )
	{
		int rate;
		int mode = mob_db[pd->pet.class_].mode;
		int race = mob_db[pd->pet.class_].race;

		if(pd->block_list::m != md->block_list::m ||
			distance(*pd, *md) > 13 || 
			(md->class_ >= 1285 && md->class_ <= 1288)) // Cannot attack Guardians/Emperium
			return 0;

		if(mob_db[pd->pet.class_].mexp <= 0 && !(mode&0x20) && (md->option & 0x06 && race!=4 && race!=6) )
			return 0;
		
		if(!type)
		{
			rate = pd->petDB.attack_rate;
			rate = rate * pd->rate_fix/1000;
			if(pd->petDB.attack_rate > 0 && rate <= 0)
				rate = 1;
		} else {
			rate = pd->petDB.defence_attack_rate;
			rate = rate * pd->rate_fix/1000;
			if(pd->petDB.defence_attack_rate > 0 && rate <= 0)
				rate = 1;
		}
		if(rand()%10000 < rate)
		{
			if(pd->target_id == 0 || rand()%10000 < pd->petDB.change_target_rate)
				pd->target_id = bl->id;
		}
	}
	return 0;
}
/*==========================================
 * Pet SC Check [Skotlex]
 *------------------------------------------
 */
int pet_sc_check(map_session_data &sd, int type)
{	
	struct pet_data *pd;

	pd = sd.pd;
	if (pd == NULL ||
		(config.pet_equip_required && pd->pet.equip_id == 0) ||
		pd->recovery == NULL ||
		pd->recovery->timer != -1 ||
		pd->recovery->type != type)
		return 1;

	pd->recovery->timer = add_timer(gettick()+pd->recovery->delay*1000,pet_recovery_timer,sd.block_list::id,0);
	
	return 0;
}




int pet_hungry_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	int interval, t;

	pet_data *pd = pet_data::from_blid(id);
	if( pd==NULL )
		return 1;

	if(pd->hungry_timer != tid)
	{
		if(config.error_log)
			ShowMessage("pet_hungry_timer %d != %d\n", pd->hungry_timer, tid);
		return 0;
	}
	pd->hungry_timer = -1;

	if(!pd->msd.status.pet_id || !pd->msd.pd)
		return 1;

	pd->pet.hungry--;
	
	// remind owner to feed the pet
	// "*~ Original code by Kitty, adapted by flaviojs ~*"
	if( pd->pet.hungry <= 35 && (pd->pet.hungry <= 25 || pd->pet.hungry%2) )
	{
		static const unsigned char petemicon_list[] = 
		{
			 7,	// Smoke cloud (angry) /ag
			16,	// Wah (crying) /wah
			43,	// Obsessed 2 (eyes popping) /e8 or /slur
			37,	// Obsessed 1 (drooling) /e2 or /rice
			20	// Hmm /hmm			 
		};
		unsigned char i = (pd->pet.hungry) ? ((pd->pet.hungry+4)/10+1) : 0;
		clif_pet_emotion(*pd, petemicon_list[i%(sizeof(petemicon_list)/sizeof(petemicon_list[0]))]);
	}


	t = pd->pet.intimate;
	if(pd->pet.hungry < 0)
	{
		if(pd->target_id > 0)
			pd->stop_attack();
		pd->pet.hungry = 0;
		pd->pet.intimate -= config.pet_hungry_friendly_decrease;
		if(pd->pet.intimate <= 0)
		{
			pd->pet.intimate = 0;
			if(config.pet_status_support && t > 0)
			{
				if( pd->msd.is_on_map() )
					status_calc_pc(pd->msd,0);
				else
					status_calc_pc(pd->msd,2);
			}
		}
		status_calc_pet(pd->msd, 0);
		clif_send_petdata(pd->msd,1, pd->pet.intimate);
	}
	clif_send_petdata(pd->msd,2, pd->pet.hungry);

	if(config.pet_hungry_delay_rate != 100)
		interval = (pd->petDB.hungry_delay*config.pet_hungry_delay_rate)/100;
	else
		interval = pd->petDB.hungry_delay;
	if(interval <= 0)
		interval = 1;
	pd->hungry_timer = add_timer(tick+interval,pet_hungry_timer,pd->block_list::id,0);

	return 0;
}

int search_petDB_index(int key,int type)
{
	int i;

	for(i=0;i<MAX_PET_DB;++i) {
		if(pet_db[i].class_ <= 0)
			continue;
		switch(type) {
			case PET_CLASS:
				if(pet_db[i].class_ == key)
					return i;
				break;
			case PET_CATCH:
				if(pet_db[i].itemID == key)
					return i;
				break;
			case PET_EGG:
				if(pet_db[i].EggID == key)
					return i;
				break;
			case PET_EQUIP:
				if(pet_db[i].AcceID == key)
					return i;
				break;
			case PET_FOOD:
				if(pet_db[i].FoodID == key)
					return i;
				break;
			default:
				return -1;
		}
	}
	return -1;
}



int pet_data_init(map_session_data &sd, const petstatus &p)
{
	struct pet_data *pd;
	int i=0,interval=0;

	if(sd.status.account_id != p.account_id || sd.status.char_id != p.char_id ||
		sd.status.pet_id != p.pet_id)
	{
		sd.status.pet_id = 0;
		return 1;
	}

	i = search_petDB_index(sd.pd->pet.class_,PET_CLASS);
	if(i < 0)
	{
		sd.status.pet_id = 0;
		return 1;
	}
	sd.pd = pd = new pet_data(sd, p, pet_db[i]);

	pd->block_list::m = sd.block_list::m;
	pd->block_list::x = pd->walktarget.x = sd.block_list::x;
	pd->block_list::y = pd->walktarget.y = sd.block_list::y;
	pd->random_walktarget(sd);
	pd->block_list::x = pd->walktarget.x;
	pd->block_list::y = pd->walktarget.y;
	pd->block_list::id = npc_get_new_npc_id();
	pd->dir = sd.dir;
	pd->speed = pd->petDB.speed;
	pd->next_walktime = pd->attackable_tick = pd->last_thinktime = gettick();

	
	for(i=0;i<MAX_MOBSKILLTIMERSKILL;++i)
		pd->skilltimerskill[i].timer = -1;

	pd->addiddb();
	pd->addblock();
	// initialise
	if (config.pet_lv_rate)	//[Skotlex]
	{
		pd->status = new pet_data::pet_status;
	}
	status_calc_pet(sd,1);

	pd->state.skillbonus = -1;
	if (config.pet_status_support && pet_db[i].script && pet_db[i].script->script) //Skotlex
		CScriptEngine::run(pet_db[i].script->script,0,sd.block_list::id,0);

	if(config.pet_hungry_delay_rate != 100)
		interval = (pd->petDB.hungry_delay*config.pet_hungry_delay_rate)/100;
	else
		interval = pd->petDB.hungry_delay;
	if(interval <= 0)
		interval = 1;

	if(pd->hungry_timer != -1) {
		delete_timer(pd->hungry_timer,pet_hungry_timer);
		pd->hungry_timer = -1;
	}
	pd->hungry_timer = add_timer(gettick()+interval,pet_hungry_timer,pd->block_list::id,0);

	return 0;
}



int pet_recv_petdata(uint32 account_id, struct petstatus &p,int flag)
{
	map_session_data *sd;

	sd = map_session_data::from_blid(account_id);
	if(sd == NULL)
		return 1;

	if(flag == 1)
	{
		if(sd->pd)
		{
			sd->pd->freeblock();
			sd->pd = NULL;
		}
		sd->status.pet_id = 0;
		return 1;
	}

	pet_data_init(*sd, p);

	if(sd->pd && sd->is_on_map() )
	{
		clif_spawnpet(*sd->pd);
		clif_send_petdata(*sd,0,0);
		clif_send_petdata(*sd,5,config.pet_hair_style);
		clif_pet_equip(*sd->pd);
		clif_send_petstatus(*sd);

		if(config.pet_status_support && sd->pd->pet.intimate > 0)
		{
			if( sd->is_on_map() )
				status_calc_pc(*sd,0);
			else
				status_calc_pc(*sd,2);
		}
	}
	return 0;
}

int pet_select_egg(map_session_data &sd,short egg_index)
{
	if(sd.status.inventory[egg_index].card[0] == 0xff00)
	{
		intif_request_petdata(sd.status.account_id, sd.status.char_id, basics::MakeDWord(sd.status.inventory[egg_index].card[1], sd.status.inventory[egg_index].card[2]) );
		pc_delitem(sd,egg_index,1,0);
	}
	else {
		if(config.error_log)
			ShowMessage("wrong egg item inventory %d\n",egg_index);
	}
	return 0;
}

int pet_catch_process1(map_session_data &sd,int target_class)
{
	sd.catch_target_class = target_class;
	clif_catch_process(sd);

	return 0;
}

int pet_catch_process2(map_session_data &sd,uint32 target_id)
{
	struct mob_data *md;
	int pet_catch_rate=0;

	if( sd.itemindex >=MAX_INVENTORY ||
		sd.status.inventory[sd.itemindex].nameid != sd.itemid ||
		!sd.inventory_data[sd.itemindex]->flag.delay_consume ||
		sd.status.inventory[sd.itemindex].amount < 1 )
	{	//Something went wrong, items moved or they tried an exploit.
		clif_pet_rulet(sd,0);
		sd.catch_target_class = -1;
		return 1;
	}
	//Consume the pet lure [Skotlex]
	pc_delitem(sd,sd.itemindex,1,0);
	sd.itemid = sd.itemindex = 0xFFFF;
	
	md= mob_data::from_blid(target_id);
	if(!md || !md->is_on_map() )
	{
		clif_pet_rulet(sd,0);
		sd.catch_target_class = -1;
		return 1;
	}

	int i = search_petDB_index(md->class_,PET_CLASS);
	//catch_target_class == 0 is used for universal lures. [Skotlex]
	//for now universal lures do not include bosses.
	if (sd.catch_target_class == 0 && !(md->mode&0x20))
		sd.catch_target_class = md->class_;
	if(i < 0 || i>=MAX_PET_DB || sd.catch_target_class != md->class_) {
		clif_emotion(*md, 7);	//mob will do /ag if wrong lure is used on them.
		clif_pet_rulet(sd,0);
		sd.catch_target_class = -1;
		return 1;
	}

	//target_idによる敵→卵判定
//	if(config.etc_log)
//		ShowMessage("mob_id = %d, mob_class = %d\n",md->block_list::id,md->class_);
		//成功の場合
	pet_catch_rate = (pet_db[i].capture + (sd.status.base_level - mob_db[md->class_].lv)*30 + sd.paramc[5]*20)*(200 - md->hp*100/mob_db[md->class_].max_hp)/100;
	if(pet_catch_rate < 1) pet_catch_rate = 1;
	if(config.pet_catch_rate != 100)
		pet_catch_rate = (pet_catch_rate*config.pet_catch_rate)/100;

	if(rand()%10000 < pet_catch_rate)
	{
		md->remove_map(0);
		clif_pet_rulet(sd,1);
//		if(config.etc_log)
//			ShowMessage("rulet success %d\n",target_id);
		intif_create_pet(
			sd.status.account_id,sd.status.char_id,
			pet_db[i].class_,mob_db[pet_db[i].class_].lv,
			pet_db[i].EggID,0,pet_db[i].intimate,100,
			0,1,
			pet_db[i].jname);
	}
	else
	{
		sd.catch_target_class = -1;
		clif_pet_rulet(sd,0);
	}

	return 0;
}

int pet_get_egg(uint32 account_id, uint32 pet_id, int flag)
{
	map_session_data *sd;
	struct item tmp_item;
	int i=0,ret=0;

	if(flag==0)
	{
		sd = map_session_data::from_blid(account_id);
		if(sd == NULL)
			return 1;

		i = search_petDB_index(sd->catch_target_class,PET_CLASS);
		sd->catch_target_class = -1;
		
		if(i >= 0)
		{
			tmp_item = (struct item)( pet_db[i].EggID );
			tmp_item.identify = 1;
			tmp_item.card[0] = 0xff00;
			tmp_item.card[1] = basics::GetWord(pet_id,0);
			tmp_item.card[2] = basics::GetWord(pet_id,1);
			tmp_item.card[3] = 0;
			if((ret = pc_additem(*sd,tmp_item,1)))
			{
				clif_additem(*sd,0,0,ret);
				map_addflooritem(tmp_item,1,sd->block_list::m,sd->block_list::x,sd->block_list::y,NULL,NULL,NULL,0);
			}
		}
		else
			intif_delete_petdata(pet_id);
	}

	return 0;
}



bool pet_data::change_name(const char *name)
{
	if( this->pet.rename_flag == 0 || config.pet_rename == 1 )
	{
		int i;
		for(i=0;i<24 && name[i];++i)
		{
			if( !(name[i]&0xe0) || name[i]==0x7f)
				return false;
		}
		this->stop_walking(1);
		safestrcpy(this->pet.name, sizeof(this->pet.name), name);
		
		clif_clearchar_area(*this, 0);
		clif_spawnpet(*this);
		clif_send_petdata(this->msd,0,0);
		clif_send_petdata(this->msd, 5, config.pet_hair_style);
		this->pet.rename_flag = 1;
		clif_pet_equip(*this);
		clif_send_petstatus(this->msd);
		return true;
	}
	return false;
}

int pet_equipitem(map_session_data &sd,int index)
{
	unsigned short nameid;

	nameid = sd.status.inventory[index].nameid;
	if(sd.pd==NULL)
		return 1;
	if(sd.pd->petDB.AcceID == 0 || nameid != sd.pd->petDB.AcceID || sd.pd->pet.equip_id != 0)
	{
		clif_equipitemack(sd,0,0,0);
		return 1;
	}
	else
	{
		pc_delitem(sd,index,1,0);
		sd.pd->pet.equip_id = nameid;
		status_calc_pc(sd,0);
		clif_pet_equip(*sd.pd);
		if (config.pet_equip_required)
		{ 	//Skotlex: start support timers if needd
			if (sd.pd->s_skill && sd.pd->s_skill->timer == -1)
			{
				if (sd.pd->s_skill->id)
					sd.pd->s_skill->timer=add_timer(gettick()+sd.pd->s_skill->delay*1000, pet_skill_support_timer, sd.block_list::id, 0);
				else
					sd.pd->s_skill->timer=add_timer(gettick()+sd.pd->s_skill->delay*1000, pet_heal_timer, sd.block_list::id, 0);
			}
			if (sd.pd->bonus && sd.pd->bonus->timer == -1)
				sd.pd->bonus->timer=add_timer(gettick()+sd.pd->bonus->delay*1000, pet_skill_bonus_timer, sd.block_list::id, 0);
		}
	}
	return 0;
}

int pet_unequipitem(map_session_data &sd)
{
	struct item tmp_item;
	unsigned short nameid;
	int flag;

	if(sd.pd==NULL)
		return 1;
	if(sd.pd->pet.equip_id == 0)
		return 1;

	nameid = sd.pd->pet.equip_id;
	sd.pd->pet.equip_id = 0;
	status_calc_pc(sd,0);
	clif_pet_equip(*sd.pd);

	tmp_item = (struct item)( nameid );
	tmp_item.identify = 1;
	if((flag = pc_additem(sd,tmp_item,1))) {
		clif_additem(sd,0,0,flag);
		map_addflooritem(tmp_item,1,sd.block_list::m,sd.block_list::x,sd.block_list::y,NULL,NULL,NULL,0);
	}
	if (config.pet_equip_required)
	{ 	//Skotlex: halt support timers if needed
		if (sd.pd->s_skill && sd.pd->s_skill->timer != -1)
		{
			if (sd.pd->s_skill->id == 0)
				delete_timer(sd.pd->s_skill->timer, pet_heal_timer);
			else
				delete_timer(sd.pd->s_skill->timer, pet_skill_support_timer);
			sd.pd->s_skill->timer=-1;
		}
		if(sd.pd->bonus && sd.pd->bonus->timer != -1)
		{
			delete_timer(sd.pd->bonus->timer, pet_skill_bonus_timer);
			sd.pd->bonus->timer = -1;
		}
	}

	return 0;
}



class CPetAiHardLootsearch : public CMapProcessor
{
	ICL_EMPTY_COPYCONSTRUCTOR(CPetAiHardLootsearch)
	struct pet_data& pd;
public:
	mutable int itc;
	CPetAiHardLootsearch(struct pet_data& p) : pd(p), itc(0)	{}
	~CPetAiHardLootsearch()	{}
	virtual int process(block_list& bl) const
	{
		if( !pd.target_id )
		{
			flooritem_data *fitem = (flooritem_data *)&bl;
			map_session_data *sd = NULL;
			// ルート権無し
			if(fitem && fitem->first_get_id>0)
				sd = map_session_data::from_blid(fitem->first_get_id);

			if( pd.loot == NULL || pd.loot->itemlist == NULL || (pd.loot->count >= pd.loot->max) || 
				(sd && sd->pd && sd->pd->block_list::id != pd.block_list::id) )
				return 0;
			if(bl.m == pd.block_list::m && distance(pd.block_list::x,pd.block_list::y,bl.x,bl.y)<5 )
			{
				if( pd.can_reach(bl.x,bl.y) &&		// 到達可能性判定
					rand()%1000<1000/(++itc) )			// 範囲内PCで等確率にする
				{	
					pd.target_id=bl.id;
				}
			}
		}
		return 0;
	}
};
int pet_ai_sub_hard(struct pet_data &pd, unsigned long tick)
{
	map_session_data *sd = &pd.msd;
	struct mob_data *md = NULL;
	int dist,i=0,dx=-1,dy=-1;

	if( !pd.is_on_map() || sd == NULL || !sd->is_on_map() )
		return 0;

	if( sd->status.pet_id == 0 || sd->pd == NULL)
		return 0;

	if(DIFF_TICK(tick,pd.last_thinktime) < MIN_PETTHINKTIME)
		return 0;

	pd.last_thinktime=tick;

	if( !pd.can_act() || pd.block_list::m != sd->block_list::m)
		return 0;

	// ペットによるルート
	if(!pd.target_id && pd.loot && pd.loot->count < pd.loot->max && DIFF_TICK(gettick(),pd.loot->loottick)>0)
	{
		CPetAiHardLootsearch pal(pd);

		block_list::foreachinarea( pal,
			pd.block_list::m, ((int)pd.block_list::x)-AREA_SIZE/2,((int)pd.block_list::y)-AREA_SIZE/2, ((int)pd.block_list::x)+AREA_SIZE/2,((int)pd.block_list::y)+AREA_SIZE/2,BL_ITEM);
		i=pal.itc;
	}

	if(sd->pd->pet.intimate > 0)
	{
		dist = distance(*sd,pd);
		if(dist > 12)
		{
			if(pd.target_id > 0)
				pd.unlock_target();
			if( pd.is_walking() && distance(*sd, pd.walktarget) < 3)
				return 0;
			pd.speed = sd->speed *3/4; // be faster than master
			if( pd.speed <= 10 )
				pd.speed = 10;
			pd.random_walktarget(*sd);
			if( !pd.walktoxy(pd.walktarget.x,pd.walktarget.y) )
				pd.randomwalk(tick);
		}
		else if(pd.target_id > MAX_FLOORITEM)
		{	//Mob targeted
			md= mob_data::from_blid(pd.target_id);
			if( md == NULL || 
				//md->block_list::type != BL_MOB || 
				pd.block_list::m != md->block_list::m || !md->is_on_map() ||
				distance(pd, *md) > 13)
			{
				pd.unlock_target();
			}
//			else if(mob_db[pd->class_].mexp <= 0 && !(mode&0x20) && (md->option & 0x06 && race!=4 && race!=6) )
//				pd.unlock_target();
			else if(!battle_check_range(&pd,md,mob_db[pd.pet.class_].range && !pd.state.casting_flag))
			{	//Skotlex Don't interrupt a casting spell when targed moved
				if( pd.is_walking() && distance(*md, pd.walktarget) < 2)
					return 0;
				if( !pd.can_reach(md->block_list::x,md->block_list::y) )
					pd.unlock_target();
				else
				{
					i=0;
					pd.calc_speed();
					for(i=0; i<5; ++i)
					{
						if(i==0)
						{	// 最初はAEGISと同じ方法で検索
							dx=md->block_list::x - pd.block_list::x;
							dy=md->block_list::y - pd.block_list::y;
							if(dx<0) dx++;
							else if(dx>0) dx--;
							if(dy<0) dy++;
							else if(dy>0) dy--;
						}
						else
						{	// だめならAthena式(ランダム)
							dx=md->block_list::x - pd.block_list::x + rand()%3 - 1;
							dy=md->block_list::y - pd.block_list::y + rand()%3 - 1;
						}
						if( pd.walktoxy(pd.block_list::x+dx,pd.block_list::y+dy) )
							break;
					}
					if(i>=5)
					{	// 移動不可能な所からの攻撃なら2歩下る
						if(dx<0) dx=2;
						else if(dx>0) dx=-2;
						if(dy<0) dy=2;
						else if(dy>0) dy=-2;
						pd.walktoxy(pd.block_list::x+dx,pd.block_list::y+dy);
					}
				}
			}
			else
			{
				pd.stop_walking(1);
				if( pd.is_attacking() )
					return 0;
				pd.start_attack();
			}
		}
		else if(pd.target_id > 0 && pd.loot)
		{	//Item Targeted, attempt loot
			flooritem_data *fitem = flooritem_data::from_blid(pd.target_id);
			if( fitem == NULL || fitem->m != pd.block_list::m || (dist=distance(pd,*fitem))>=5 )
			{	// 遠すぎるかアイテムがなくなった
 				pd.unlock_target();
			}
			else if(dist)
			{
				if( pd.walktimer != -1 && !pd.is_attacking() && (DIFF_TICK(pd.next_walktime,tick)<0 || distance(*fitem, pd.walktarget) == 0) )
					return 0; // 既に移動中

				pd.next_walktime=tick+500;
				pd.walktoxy( fitem->x, fitem->y);
			}
			else
			{	// アイテムまでたどり着いた
				if( pd.is_attacking() )
					return 0; // 攻撃中

				pd.stop_walking(1);

				if(pd.loot && pd.loot->count < pd.loot->max)
				{
					pd.loot->itemlist[pd.loot->count++] = fitem->item_data;
					pd.loot->weight += itemdb_search(fitem->item_data.nameid)->weight*fitem->item_data.amount;
					map_clearflooritem(fitem->id);
				}
				pd.unlock_target();		
			}
		}
		else
		{
			if( pd.is_walking() && distance(*sd, pd.walktarget) < 3 )
				return 0;
			if(dist<=3)
			{
				if(config.pet_random_move && !sd->is_sitting() )
					pd.randomwalk(tick);
				return 0;
			}
			pd.calc_speed();
			pd.random_walktarget(*sd);
			if( !pd.walktoxy(pd.walktarget.x,pd.walktarget.y) )
				pd.randomwalk(tick);
		}
	}
	else
	{
		pd.calc_speed();
		pd.stop_attack();
		pd.randomwalk(tick);
	}
	return 0;
}


int pet_ai_hard(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if(sd)
		{
			if( sd->status.pet_id && sd->pd)
				pet_ai_sub_hard(*sd->pd, tick);
		}
	}
	return 0;
}



int pet_delay_item_drop2(int tid, unsigned long tick, int id, basics::numptr data)
{
	delay_item_drop2 *ditem=(delay_item_drop2 *)data.ptr;
	if(ditem)
	{
		map_addflooritem(ditem->item_data,ditem->item_data.amount,ditem->m,ditem->x,ditem->y,ditem->first_sd,ditem->second_sd,ditem->third_sd,0);
		delete ditem;
		get_timer(tid)->data=0;
	}
	return 0;
}

/*==========================================
 * pet bonus giving skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------
 */ 
int pet_skill_bonus_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd=map_session_data::from_blid(id);
	struct pet_data *pd;
	int timer = 0;

	if(sd == NULL || sd->pd==NULL || sd->pd->bonus == NULL)
		return 1;
	
	pd=sd->pd;
	if(pd->bonus->timer != tid) {
		if(config.error_log)
			ShowMessage("pet_skill_bonus_timer %d != %d\n",pd->bonus->timer,tid);
		return 0;
	}
	
	// determine the time for the next timer
	if (pd->state.skillbonus == 0) {
		// pet bonuses are not active at the moment, so,
		pd->state.skillbonus = 1;
		timer = pd->bonus->duration*1000;	// the duration for pet bonuses to be in effect
	} else if (pd->state.skillbonus == 1) {
		// pet bonuses are already active, so,
		pd->state.skillbonus = 0;
		timer = (pd->bonus->delay - pd->bonus->duration)*1000;	// the duration until pet bonuses will be reactivated again
		if (timer < 0) //Always active bonus
			timer = MIN_PETTHINKTIME; 
	}

	// add/remove our bonuses
	status_calc_pc(*sd, 0);

	// wait for the next timer
	pd->bonus->timer=add_timer(tick+timer,pet_skill_bonus_timer,sd->block_list::id,0);
	
	return 0;
}

int pet_recovery_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd= map_session_data::from_blid(id);
	struct pet_data *pd;
	
	if(sd==NULL || sd->pd == NULL || sd->pd->recovery == NULL)
		return 1;
	
	pd=sd->pd;

	if(pd->recovery == NULL || pd->recovery->timer != tid) {
		if(config.error_log)
		{
			if (pd->recovery)
				ShowMessage("pet_recovery_timer %d != %d\n",pd->recovery->timer,tid);
			else
				ShowMessage("pet_recovery_timer called with no recovery skill defined (tid=%d)\n",tid);
		}
		return 0;
	}

	if( sd->has_status((status_t)pd->recovery->type) )
	{	//Display a heal
		clif_skill_nodamage(*pd,*sd,TF_DETOXIFY,1,1);
		status_change_end(sd,(status_t)pd->recovery->type,-1);
		clif_emotion(*pd, 33);
	}

	pd->recovery->timer = -1;
	
	return 0;
}

int pet_heal_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd= map_session_data::from_blid(id);
	struct pet_data *pd;
	short rate = 100;
	
	if(sd==NULL || sd->pd == NULL)
		return 1;
	
	pd=sd->pd;

	if(pd->s_skill == NULL || pd->s_skill->timer != tid) {
		if(config.error_log)
		{
			if (pd->s_skill)
				ShowMessage("pet_heal_timer %d != %d\n",pd->s_skill->timer,tid);
			else
				ShowMessage("pet_heal_timer called with no support skill defined (tid=%d)\n",tid);
		}
		return 0;
	}
	
	if( sd->is_dead() ||
		(rate = sd->status.sp*100/sd->status.max_sp) > pd->s_skill->sp ||
		(rate = sd->status.hp*100/sd->status.max_hp) > pd->s_skill->hp ||
		(rate = pd->state.casting_flag) || //Another skill is in effect
		(rate = pd->is_walking())) //Better wait until the pet stops moving (MS_WALK is 2)
	{  //Wait (how long? 1 sec for every 10% of remaining)
		pd->s_skill->timer=add_timer(gettick()+(rate>10?rate:10)*100,pet_heal_timer,sd->block_list::id,0);
		return 0;
	}

	pd->stop_attack();
	clif_skill_nodamage(*pd,*sd,AL_HEAL,pd->s_skill->lv,1);
	sd->heal(pd->s_skill->lv,0);
	
	pd->s_skill->timer=add_timer(tick+pd->s_skill->delay*1000,pet_heal_timer,sd->block_list::id,0);
	
	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------
 */ 
int pet_skill_support_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd = map_session_data::from_blid(id);
	struct pet_data *pd;
	short rate = 100;	
	if(sd==NULL || sd->pd == NULL)
		return 1;
	
	pd=sd->pd;
	
	if(pd->s_skill == NULL || pd->s_skill->timer != tid) {
		if(config.error_log)
		{
			if (pd->s_skill)
				ShowMessage("pet_skill_support_timer %d != %d\n",pd->s_skill->timer,tid);
			else
				ShowMessage("pet_skill_support_timer called with no support skill defined (tid=%d)\n",tid);
		}
		return 0;
	}
	
	if( sd->is_dead() ||
		(rate = sd->status.sp*100/sd->status.max_sp) > pd->s_skill->sp ||
		(rate = sd->status.hp*100/sd->status.max_hp) > pd->s_skill->hp ||
		(rate = pd->state.casting_flag) || //Another skill is in effect
		(rate = pd->is_walking()) ) //Better wait until the pet stops moving (MS_WALK is 2)
	{  //Wait (how long? 1 sec for every 10% of remaining)
		pd->s_skill->timer=add_timer(gettick()+(rate>10?rate:10)*100,pet_skill_support_timer,sd->block_list::id,0);
		return 0;
	}
	
	pd->stop_attack();
	pet_skill_use(*pd, *sd, pd->s_skill->id, pd->s_skill->lv, tick);

	pd->s_skill->timer=add_timer(tick+pd->s_skill->delay*1000,pet_skill_support_timer,sd->block_list::id,0);
	
	return 0;
}

/*==========================================
 *ペットデータ読み込み
 *------------------------------------------
 */ 
int read_petdb()
{
	FILE *fp;
	char line[1024];
	unsigned short nameid;
	size_t i,k,j=0;
	int lines;
	char *filename[]={"db/pet_db.txt","db/pet_db2.txt"};
	char *str[32],*p,*np;
	
	memset(pet_db,0,sizeof(pet_db));
	for(i=0;i<2;++i){
		fp=basics::safefopen(filename[i],"r");
		if(fp==NULL){
			if(i>0)
				continue;
			ShowError("can't read %s\n",filename[i]);
			return -1;
		}
		lines = 0;
		while(fgets(line,sizeof(line),fp)){
			
			lines++;

			if( !is_valid_line(line) )
				continue;

			for(k=0,p=line;k<20;++k){
				if((np=strchr(p,','))!=NULL){
					str[k]=p;
					*np=0;
					p=np+1;
				} else {
					str[k]=p;
					p+=strlen(p);
				}
			}

			nameid=atoi(str[0]);
			if(nameid<=0 || nameid>2000)
				continue;
		
			//MobID,Name,JName,ItemID,EggID,AcceID,FoodID,"Fullness (1回の餌での満腹度増加率%)","HungryDeray (/min)","R_Hungry (空腹時餌やり親密度増加率%)","R_Full (とても満腹時餌やり親密度減少率%)","Intimate (捕獲時親密度%)","Die (死亡時親密度減少率%)","Capture (捕獲率%)",(Name)
			pet_db[j].class_ = nameid;
			memcpy(pet_db[j].name,str[1],24);
			memcpy(pet_db[j].jname,str[2],24);
			pet_db[j].itemID=atoi(str[3]);
			pet_db[j].EggID=atoi(str[4]);
			pet_db[j].AcceID=atoi(str[5]);
			pet_db[j].FoodID=atoi(str[6]);
			pet_db[j].fullness=atoi(str[7]);
			pet_db[j].hungry_delay=atoi(str[8])*1000;
			pet_db[j].r_hungry=atoi(str[9]);
			if(pet_db[j].r_hungry <= 0)
				pet_db[j].r_hungry=1;
			pet_db[j].r_full=atoi(str[10]);
			pet_db[j].intimate=atoi(str[11]);
			pet_db[j].die=atoi(str[12]);
			pet_db[j].capture=atoi(str[13]);
			pet_db[j].speed=atoi(str[14]);
			pet_db[j].s_perfor=(char)atoi(str[15]);
			pet_db[j].talk_convert_class=atoi(str[16]);
			pet_db[j].attack_rate=atoi(str[17]);
			pet_db[j].defence_attack_rate=atoi(str[18]);
			pet_db[j].change_target_rate=atoi(str[19]);
			pet_db[j].script = NULL;
			if((np=strchr(p,'{'))==NULL)
				continue;
			pet_db[j].script = parse_script((unsigned char *) np,lines);
			j++;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' pets in '"CL_WHITE"%s"CL_RESET"'.\n",j,filename[i]);
	}
	return 0;
}

/*==========================================
 * スキル関係初期化処理
 *------------------------------------------
 */
int do_init_pet(void)
{
	read_petdb();

	add_timer_func_list(pet_hungry_timer,"pet_hungry");
	add_timer_func_list(pet_ai_hard,"pet_ai_hard");
	add_timer_func_list(pet_skill_bonus_timer,"pet_skill_bonus_timer"); // [Valaris]
	add_timer_func_list(pet_delay_item_drop2,"pet_delay_item_drop2");	
	add_timer_func_list(pet_skill_support_timer, "pet_skill_support_timer"); // [Skotlex]
	add_timer_func_list(pet_recovery_timer,"pet_recovery_timer"); // [Valaris]
	add_timer_func_list(pet_heal_timer,"pet_heal_timer"); // [Valaris]
	add_timer_interval(gettick()+MIN_PETTHINKTIME,MIN_PETTHINKTIME,pet_ai_hard,0,0);

	return 0;
}

int do_final_pet(void)
{
	int i;
	for(i = 0;i < MAX_PET_DB; ++i)
	{
		if(pet_db[i].script)
		{
			pet_db[i].script->release();
			pet_db[i].script=NULL;
		}
	}
	return 0;
}
