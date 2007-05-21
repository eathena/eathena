// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "socket.h" // [Valaris]
#include "timer.h"
#include "db.h"
#include "showmsg.h"
#include "utils.h"

#include "malloc.h"
#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "status.h"
#include "npc.h"
#include "flooritem.h"
#include "mob.h"
#include "pet.h"
#include "homun.h"
#include "itemdb.h"
#include "script.h"
#include "battle.h"
#include "skill.h"
#include "party.h"
#include "guild.h"
#include "chat.h"
#include "trade.h"
#include "storage.h"
#include "vending.h"
#include "nullpo.h"
#include "atcommand.h"
#include "log.h"
#include "showmsg.h"
#include "core.h"


#define PVP_CALCRANK_INTERVAL 1000	// PVP順位計算の間隔

static uint32 exp_table[14][MAX_LEVEL];
static unsigned short statp[MAX_LEVEL];





static const char dirx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
static const char diry[8] = { 1, 1, 0,-1,-1,-1, 0, 1};



static unsigned int equip_pos[11]={0x0080,0x0008,0x0040,0x0004,0x0001,0x0200,0x0100,0x0010,0x0020,0x0002,0x8000};



///////////////////////////////////////////////////////////////////////////////
//
// map_session_data members
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// static elements and members
dbt* map_session_data::accid_db=NULL;
dbt* map_session_data::charid_db=NULL;
dbt* map_session_data::nick_db=NULL;



/// initialize all static elements
void map_session_data::initialize()
{
	map_session_data::finalize();

	map_session_data::accid_db = numdb_init();
	map_session_data::charid_db = numdb_init();
	map_session_data::nick_db = strdb_init(24);
	
}


/// finalize all static elements
void map_session_data::finalize()
{
	if(map_session_data::accid_db)
	{
		numdb_final(map_session_data::accid_db, NULL);
		map_session_data::accid_db=NULL;
	}
	if(map_session_data::charid_db)
	{
		numdb_final(map_session_data::charid_db, NULL);
		map_session_data::charid_db=NULL;
	}
	if(map_session_data::nick_db)
	{
		strdb_final(map_session_data::nick_db, NULL);
		map_session_data::nick_db=NULL;
	}
}




/// search session data by block_list::id.
map_session_data* map_session_data::from_blid(uint32 id)
{
	block_list* bl = block_list::from_blid(id);
	return (bl)?bl->get_sd():NULL;
}
/// search session data by char_id.
map_session_data* map_session_data::charid2sd(uint32 id)
{
	return (map_session_data *)numdb_search(map_session_data::charid_db, id);
}
/// search session data by account_id.
map_session_data* map_session_data::accid2sd(uint32 id)
{
	return (map_session_data *)numdb_search(map_session_data::accid_db, id);
}
/// search session data by name.
/// also tests for a given incomplete nick
map_session_data* map_session_data::nick2sd(const char *nick)
{	// try exact match first, it's only O(ld(N))
	map_session_data* psd = (map_session_data *)strdb_search(map_session_data::nick_db, nick);
	if( !psd )
	{	// test if nick is the beginning of a charname,
		// although this is O(N)
		map_session_data::iterator iter(map_session_data::nick_db);
		size_t len = strlen(nick);
		for(; iter; ++iter)
		{	
			if( 0==strncasecmp(iter.key(), nick, len) == 0)
			{	// found a possible match
				if(psd)
					return NULL;		// double match found -> return failure
				else 
					psd = iter.data();	// store found match
			}
		}
	}
	return psd;
}

/// return number of valid sessions
size_t map_session_data::count_users(void)
{	// nick_db contains only authentified sessions
	// so can use this directly

	if( config.hide_GM_session )
	{	// have to go through the list
		map_session_data::iterator iter(map_session_data::nick_db);
		size_t c=0;
		for(; iter; ++iter)
		{
			if( iter.data() && !iter.data()->isGM() )
				++c;
		}
		return c;
	}
	else
	{	// otherwise just return the element count
		return (map_session_data::nick_db)?map_session_data::nick_db->item_count:0;
	}
}





///////////////////////////////////////////////////////////////////////////////
/// constructor.
/// builds on having nulled memory already (overloaded operator new),
/// although this is not true for plain instances
map_session_data::map_session_data(int fdi, int packver, uint32 accid, uint32 chrid, uint32 logid1, uint32 logid2, uint32 client_tick, unsigned char sex)
{
	size_t i;
	unsigned long tick = gettick();

	this->fd				= fdi;
	this->packet_ver		= packver;
	this->client_tick		= client_tick;

	this->block_list::id	= accid;

	this->status.account_id = accid;
	this->status.char_id	= chrid;
	this->status.sex		= sex;
	this->status.name[0]	= 0;

	this->login_id1			= logid1;
	this->login_id2			= logid2;
	this->speed				= DEFAULT_WALK_SPEED;
	
	this->skillitem			= 0xFFFF;
	this->skillitemlv		= 0xFFFF;
	
	this->party_x			= 0xFFFF;
	this->party_y			= 0xFFFF;
	this->party_hp			= -1;

	// ticks
	this->canact_tick		= tick;
	this->canmove_tick		= tick;
	this->canlog_tick		= tick;
	this->canregen_tick		= tick;
	this->attackable_tick	= tick;

	// timers
	this->followtimer		= -1;
	this->invincible_timer	= -1;
	this->pvp_timer			= -1;
	
	for(i = 0; i < MAX_EVENTTIMER; ++i)
		this->eventtimer[i] = -1;
	for(i = 0; i < MAX_SKILL_LEVEL; ++i)
		this->spirit_timer[i] = -1;


	if( this->status.account_id )
		numdb_insert(map_session_data::accid_db, this->status.account_id, this);
	if( this->status.char_id )
		numdb_insert(map_session_data::charid_db, this->status.char_id, this);
}

/// destructor.
map_session_data::~map_session_data()
{
	if(this->pd) delete pd;
	if(this->hd) delete hd;
	this->clean_db();
}


/// remove from map.
void map_session_data::map_quit()
{
	map_session_data& sd = *this;

	if( sd.state.event_disconnect )
	{
		npc_data::event("OnPCLogoutEvent", script_config.logout_event_name, sd);
	}
	sd.leavechat();
	if(sd.trade_partner)	// 取引を中?する	
		trade_tradecancel(sd);
	if(sd.party_invite>0)	// パ?ティ?誘を拒否する
		party_reply_invite(sd,sd.party_invite_account,0);
	if(sd.guild_invite>0)	// ギルド?誘を拒否する
		guild_reply_invite(sd,sd.guild_invite,0);
	if(sd.guild_alliance>0)	// ギルド同盟?誘を拒否する
		guild_reply_reqalliance(sd,sd.guild_alliance_account,0);

	party_send_logout(sd);	// パ?ティのログアウトメッセ?ジ送信
	party_send_dot_remove(sd);
	guild_send_memberinfoshort(sd,0);	// ギルドのログアウトメッセ?ジ送信

	chrif_save_sc(sd);

	pc_cleareventtimer(sd);	// イベントタイマを破棄する
	if(sd.state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);	// 倉庫を開いてるなら保存する

	// check if we've been authenticated [celest]
	if (sd.state.auth)
		skill_castcancel(&sd,0);	// 詠唱を中?する

	skill_stop_dancing(&sd,1);// ダンス/演奏中?
	if( sd.has_status(SC_BERSERK) ) //バ?サ?ク中の終了はHPを100に
	{
		sd.status.hp = 100;
		status_change_end(&sd,SC_BERSERK,-1);
	}

	status_change_clear(&sd,1);	// ステ?タス異常を解除する
	skill_clear_unitgroup(&sd);	// スキルユニットグル?プの削除
	skill_cleartimerskill(&sd);

	sd.stop_walking(0);
	sd.stop_attack();
	pc_delinvincibletimer(sd);

	pc_delspiritball(sd,sd.spiritball,1);
	skill_gangsterparadise(&sd,0);
	skill_unit_move(sd,gettick(),0);
	
	if( sd.state.auth )
		status_calc_pc(sd,4);

	if( !(sd.status.option & OPTION_HIDE) )
		clif_clearchar_area(sd,2);

	if( sd.status.pet_id && sd.pd )
	{
		sd.pd->droploot();
		if(sd.pd->pet.intimate <= 0)
		{
			intif_delete_petdata(sd.status.pet_id);
			sd.status.pet_id = 0;
			sd.pd = NULL;
		}
		else
			intif_save_petdata(sd.status.account_id,sd.pd->pet);
		sd.pd->freeblock();
	}
	if( sd.is_dead() )
		pc_setrestartvalue(sd,2);

	homun_data::clear_homunculus(sd);
	pc_clean_skilltree(sd);
	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_dirty(sd);
	storage_storage_save(sd);
	sd.delblock();

	
	sd.ScriptEngine.clear();

	chrif_char_offline(sd);
		
	if(sd.reg)
	{
		delete[] sd.reg;
		sd.reg=NULL;
	}
		
	if(sd.regstr)
	{
		delete[] sd.regstr;
		sd.regstr=NULL;
	}
}


/// insert to nick_db.
void map_session_data::add_nickdb()
{
	if( *this->status.name )
		strdb_insert(map_session_data::nick_db, this->status.name, this);
}

/// clean all entries of this object in dbs
void map_session_data::clean_db()
{
	if( *this->status.name )
		strdb_erase(map_session_data::nick_db, this->status.name);
	if( this->status.account_id )
		numdb_erase(map_session_data::accid_db, this->status.account_id );
	if( this->status.char_id )
		numdb_erase(map_session_data::charid_db, this->status.char_id );
}




unsigned char map_session_data::isGM() const
{
	//For console [Wizputer]
	if ( this->fd == 0 )
	    return 99;

	return this->status.gm_level;
}


int map_session_data::attacktimer_func(int tid, unsigned long tick, int id, basics::numptr data)
{	
	map_session_data *sd = this;	// to lazy to remove the pointer usage
	short *opt;
	int dist,skill,range;

	sd->idletime = last_tick;
	if( !sd->is_on_map() || sd->is_dead() )
		return 0;

	block_list* bl=block_list::from_blid(sd->target_id);
	if( bl==NULL || !bl->is_on_map() || bl->is_dead() || bl->is_hiding() )
		return 0;


	// 同じmapでないなら攻?しない
	// PCが死んでても攻?しない
	if( sd->block_list::m != bl->m || sd->is_dead() )
		return 0;

	if( sd->opt1>0 || sd->status.option&2 || sd->is_chasewalk())	// 異常などで攻?できない
		return 0;

	if( sd->has_status(SC_AUTOCOUNTER) )
		return 0;
	if(sd->has_status(SC_BLADESTOP) )
		return 0;
	if(sd->has_status(SC_GRAVITATION) )
		return 0;

	if((opt = status_get_option(bl)) != NULL && *opt&0x42)
		return 0;
	if( bl->has_status(SC_TRICKDEAD) || bl->has_status(SC_BASILICA) )
		return 0;

	if(sd->skilltimer != -1 && sd->skill_check(SA_FREECAST) <= 0)
		return 0;

	if(!config.skill_delay_attack_enable && sd->skill_check(SA_FREECAST) <= 0) {
		if(DIFF_TICK(tick, sd->canact_tick) < 0) {
			sd->skill_failed(1,SF_DELAY);
			return 0;
		}
	}

	if(sd->status.weapon == 11 && sd->equip_index[10] >= MAX_INVENTORY) {
		clif_arrow_fail(*sd,0);
		return 0;
	}

	dist = distance(*sd,*bl);
	range = sd->attackrange;
	if(sd->status.weapon != 11) range++;
	if( dist > range ){	// ? かないので移動
		if( sd->can_reach(bl->x,bl->y) )
			clif_movetoattack(*sd,*bl);
		return 0;
	}

	if(dist <= range && !battle_check_range(sd,bl,range) ) {
		if( sd->can_reach(bl->x,bl->y) && DIFF_TICK(sd->canmove_tick,tick)<0 && (!sd->has_status(SC_ANKLE) || !sd->has_status(SC_SPIDERWEB)))
			sd->walktoxy(bl->x,bl->y);
		sd->attackable_tick = tick + (sd->aspd<<1);
	}
	else
	{	// On this point, we have reached our target, and guarantee an attack, so.. uncloak. [Skotlex]
		if( sd->is_cloaking() )
			status_change_end(sd, SC_CLOAKING, -1);

		if(config.pc_attack_direction_change)
			sd->dir=sd->head_dir=sd->get_direction(*bl);	// 向き設定

		sd->stop_walking(1);

		if( !sd->has_status(SC_COMBO) )
		{
			block_list::freeblock_lock();
			sd->target_lv = battle_weapon_attack(sd,bl,tick,0);
			// &2 = ? - Celest
			if(!(config.pc_cloak_check_type&2) && sd->has_status(SC_CLOAKING) )
				status_change_end(sd,SC_CLOAKING,-1);
			if(sd->target_lv >0 && sd->status.pet_id > 0 && sd->pd && config.pet_attack_support)
				pet_target_check(*sd,bl,0);
			block_list::freeblock_unlock();
			if(sd->skilltimer != -1 && (skill = sd->skill_check(SA_FREECAST)) > 0 ) // フリ?キャスト
				sd->attackable_tick = tick + ((sd->aspd<<1)*(150 - skill*5)/100);
			else
				sd->attackable_tick = tick + (sd->aspd<<1);
		}
		else if( DIFF_TICK(sd->attackable_tick,tick) <= 0 )
		{
			if(sd->skilltimer != -1 && (skill = sd->skill_check(SA_FREECAST)) > 0 ) // フリ?キャスト
			{
				sd->attackable_tick = tick + ((sd->aspd<<1)*(150 - skill*5)/100);
			}
			else
				sd->attackable_tick = tick + (sd->aspd<<1);
		}
	}
	if( DIFF_TICK(sd->attackable_tick,tick) < (int)(config.max_aspd_interval<<1) )
		sd->attackable_tick = tick + (config.max_aspd_interval<<1);

	if(sd->fightable::attack_continue)
		sd->attacktimer=add_timer(sd->attackable_tick,fightable::attacktimer_entry,sd->block_list::id,0);

	return 0;
}

int map_session_data::skilltimer_func(int tid, unsigned long tick, int id, basics::numptr data)
{	
	return 0;
}











/// do object depending stuff for the walk step.
bool map_session_data::do_walkstep(unsigned long tick, const coordinate &target, int dx, int dy)
{

	//?いたので息吹のタイマ?を初期化
	this->inchealspirithptick = 0;
	this->inchealspiritsptick = 0;

//->
// move this check into "can_calk"
// then the object will stop automatically
	const int mcnt = maps[this->block_list::m].foreachinarea( CSkillMoonlitCount(this->block_list::id), target.x, target.y, 1, BL_PC);
	if( mcnt>0 )
		return false;

//<-

	if(this->status.party_id > 0)
	{	// パ?ティのＨＰ情報通知?査
		struct party *p = party_search(this->status.party_id);
		if (p != NULL) {
			int p_flag = 0;

			maps[this->block_list::m].foreachinmovearea(CPartySendHP(this->status.party_id, p_flag),
				target.x-AREA_SIZE, target.y-AREA_SIZE, target.x+AREA_SIZE, target.y+AREA_SIZE, -dx, -dy, BL_PC);
			if (p_flag)
				this->party_hp = -1;
		}
	}

	// ディボ?ション?査 
	size_t i;
	for(i=0; i<5; ++i)
	{
		if( this->dev.val1[i] )
		{
			skill_devotion3(this, this->dev.val1[i]);
			break;
		}
	}

	// 被ディボ?ション?査 
	if( this->has_status(SC_DANCING) )
	{
		struct skill_unit_group *ptr = (struct skill_unit_group *)this->get_statusvalue2(SC_DANCING).pointer();
		if(ptr) skill_unit_move_unit_group(*ptr, this->block_list::m, dx, dy);
	}

	if( this->get_statusvalue1(SC_DEVOTION).integer())
		skill_devotion2(this, this->get_statusvalue1(SC_DEVOTION).integer());

	if( this->has_status(SC_BASILICA) )
	{	// Basilica cancels if caster moves [celest]
		struct skill_unit_group *sg = (struct skill_unit_group *)this->get_statusvalue4(SC_BASILICA).pointer();
		if (sg && sg->src_id == this->block_list::id)
			skill_delunitgroup (*sg);
		status_change_end(this,SC_BASILICA,-1);
	}
	return true;
}
/// do object depending stuff at the end of the walk step.
void map_session_data::do_walkend()
{
	if( this->is_cloaking() )	// クロ?キングの消滅?査
		skill_check_cloaking(this);

	// check for a npc_ontouch event
	npc_data::touch(*this,this->block_list::m,block_list::x,block_list::y);

	if (config.disp_hpmeter)
	{
		clif_hpmeter(*this);
	}
}
void map_session_data::do_walkto()
{
	if( this->status.guild_id > 0 )
	{	// check for beeing guildmaster
		struct guild *g = guild_search(this->status.guild_id);
		if (g && strcmp(this->status.name, g->master)==0)
		{
			int skill, guildflag = 0;

			if ((skill = guild_checkskill(*g, GD_LEADERSHIP)) > 0)
				guildflag |= skill<<16;
			if ((skill = guild_checkskill(*g, GD_GLORYWOUNDS)) > 0)
				guildflag |= skill<<12;
			if ((skill = guild_checkskill(*g, GD_SOULCOLD)) > 0)
				guildflag |= skill<<8;
			if ((skill = guild_checkskill(*g, GD_HAWKEYES)) > 0)
				guildflag |= skill<<4;
			if ((skill = guild_checkskill(*g, GD_CHARISMA)) > 0)
				guildflag |= skill;

			if (guildflag)
			{	
				maps[this->block_list::m].foreachinarea( CSkillGuildaura(this->block_list::id, this->status.guild_id, guildflag),
					this->block_list::x, this->block_list::y, 2, BL_PC);
			}
		}
	}
}


/// sets the object to standing state
bool map_session_data::set_stand()
{
	if( this->has_status(SC_TENSIONRELAX) )
		status_change_end(this,SC_TENSIONRELAX,-1);
	this->state.dead_sit = 0;
	return true;
}
int map_session_data::get_size() const
{
	//Baby Class Peco Rider + enabled option -> size = 1, else 0
	if( pc_calc_upper(this->status.class_)==2 )
		return (this->is_riding()!=0 && config.character_size&2);
	//Peco Rider + enabled option -> size = 2, else 1
	return 1+(this->is_riding()!=0 && config.character_size&1);
}




/// do object depending stuff for attacking
void map_session_data::do_attack()
{
	this->idletime = last_tick;
}



/*==========================================
 * HP/SP回復
 *------------------------------------------
 */
int map_session_data::heal(int hp, int sp)
{
	if(config.battle_log)
		ShowMessage("heal %d %d\n",hp,sp);
	map_session_data& sd = *this;

	if( sd.has_status(SC_BERSERK) ) //バ?サ?ク中は回復させないらしい
		return 0;

	if( hp>0 && pc_checkoverhp(sd) )
			hp = 0;
	else if(hp+sd.status.hp > sd.status.max_hp)
		hp = sd.status.max_hp - sd.status.hp;

	if( sp>0 && pc_checkoversp(sd) )
			sp = 0;
	else if(sp+sd.status.sp > sd.status.max_sp)
		sp = sd.status.max_sp - sd.status.sp;

	sd.status.hp += hp;
	if(sd.status.hp <= 0)
	{
		sd.status.hp = 1;
		pc_damage(sd,1,NULL);
		hp = 0;
	}
	sd.status.sp+=sp;
	if(sd.status.sp <= 0)
		sd.status.sp = 0;

	if(hp)
		clif_updatestatus(sd,SP_HP);
	if(sp)
		clif_updatestatus(sd,SP_SP);

	if( sd.status.party_id>0 )
	{	// on-the-fly party hp updates [Valaris]
		struct party *p=party_search(sd.status.party_id);
		if(p!=NULL) clif_party_hp(*p,sd);
	}	// end addition [Valaris]
	return hp + sp;
}


int map_session_data::skill_check(ushort skillid)
{
	if( skillid>=10000 )
	{
		struct guild *g;
		if( this->status.guild_id>0 && (g=guild_search(this->status.guild_id))!=NULL )
			return guild_checkskill(*g,skillid);
	}
	else if( skillid<MAX_SKILL && this->status.skill[skillid].id == skillid)
		return this->status.skill[skillid].lv;
	return 0;
}
bool map_session_data::skill_can_cancel() const
{
	return (this->state.no_castcancel || maps[this->block_list::m].flag.gvg) && 
			this->state.skillcastcancel	&& !this->state.no_castcancel2;
}
void map_session_data::skill_failed(ushort skill_id, skillfail_t type)
{
	clif_skill_failed(*this,skill_id, type);
}


///////////////////////////////////////////////////////////////////////////
// chat functions


/// create a chat.
/// actually calls the chat member to do this
bool map_session_data::createchat(unsigned short limit, unsigned char pub, const char* pass, const char* title)
{
	return chat_data::create(*this, limit, pub, pass, title);
}

/// leave a chat.
/// actually calls the chat member to get removed from there
bool map_session_data::leavechat()
{
	return ( this->chat ) ? this->chat->remove(*this) : true;
}






















/*==========================================
 * 攻?要求
 * typeが1なら??攻?
 *------------------------------------------
 */
int pc_attack(map_session_data &sd, uint32 target_id, int type)
{
	block_list *bl;
	int d;

	bl=block_list::from_blid(target_id);
	if(bl==NULL)
		return 1;

	sd.idletime = last_tick;

	if( *bl==BL_NPC )
	{
		npc_data::click(sd,target_id);
		return 0;
	}

	if(battle_check_target(&sd,bl,BCT_ENEMY) <= 0)
		return 1;
	sd.stop_attack();
	sd.target_id=target_id;
	sd.attack_continue=type;

	d=DIFF_TICK(sd.attackable_tick,gettick());

	if(d>0 && d<2000){	// 攻?delay中
		sd.attacktimer=add_timer(sd.attackable_tick,fightable::attacktimer_entry,sd.block_list::id,0);
	} else {
		// 本?timer??なので引?を合わせる
		fightable::attacktimer_entry(-1,gettick(),sd.block_list::id,0);
	}

	return 0;
}













struct skill_tree_entry skill_tree[MAX_PC_CLASS][MAX_SKILL_TREE];
int skill_tree_get_max(int id, int b_class){
	int i, skillid;
	for(i=0;(skillid=skill_tree[b_class][i].id)>0;++i)
		if (id == skillid) return skill_tree[b_class][i].max;
	return skill_get_max (id);
}





bool pc_iskiller(map_session_data &src, map_session_data &target)
{	
	return (src.state.killer) || (target.state.killable);
}



int pc_invincible_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd=map_session_data::from_blid(id);

	if( sd == NULL )
		return 1;

	if(sd->invincible_timer != tid)
	{
		if(config.error_log)
			ShowMessage("invincible_timer %d != %d\n",sd->invincible_timer,tid);
		return 0;
	}
	sd->invincible_timer=-1;
	skill_unit_move(*sd,tick,1);

	return 0;
}

int pc_setinvincibletimer(map_session_data &sd,int val)
{
	if(sd.invincible_timer != -1)
		delete_timer(sd.invincible_timer,pc_invincible_timer);
	sd.invincible_timer = add_timer(gettick()+val,pc_invincible_timer,sd.block_list::id,0);
	return 0;
}

int pc_delinvincibletimer(map_session_data &sd)
{
	if(sd.invincible_timer != -1) {
		delete_timer(sd.invincible_timer,pc_invincible_timer);
		sd.invincible_timer = -1;
	}
	skill_unit_move(sd,gettick(),1);
	return 0;
}

int pc_spiritball_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd;

	if( (sd=map_session_data::from_blid(id)) == NULL )
		return 1;

	if(sd->spirit_timer[0] != tid){
		if(config.error_log)
			ShowMessage("spirit_timer %d != %d\n",sd->spirit_timer[0],tid);
		return 0;
	}

	if(sd->spiritball <= 0) {
		if(config.error_log)
			ShowMessage("Spiritballs are already 0 when pc_spiritball_timer gets called");
		sd->spiritball = 0;
		return 0;
	}

	sd->spiritball--;
	memmove( sd->spirit_timer+0, sd->spirit_timer+1, (sd->spiritball)*sizeof(sd->spirit_timer[0]) );
	sd->spirit_timer[sd->spiritball]=-1;

	clif_spiritball(*sd);

	return 0;
}

int pc_addspiritball(map_session_data &sd,int interval,int max) 
{
	if(max > MAX_SKILL_LEVEL)
		max = MAX_SKILL_LEVEL;	// could happen
	if(sd.spiritball < 0)
		sd.spiritball = 0;		// should never happen

	if(sd.spiritball >= max) {
		if(sd.spirit_timer[0] != -1)
			delete_timer(sd.spirit_timer[0],pc_spiritball_timer);
		memmove( sd.spirit_timer+0, sd.spirit_timer+1, (sd.spiritball - 1)*sizeof(sd.spirit_timer[0]) );
		//sd->spirit_timer[sd->spiritball-1] = -1; // intentionally, but will be overwritten
	}
	else
		sd.spiritball++;

	sd.spirit_timer[sd.spiritball-1] = add_timer(gettick()+interval,pc_spiritball_timer,sd.block_list::id,0);
	clif_spiritball(sd);

	return 0;
}

int pc_delspiritball(map_session_data &sd,int count,int type)
{
	int i;

	if(sd.spiritball <= 0) {
		sd.spiritball = 0;
		return 0;
	}

	if(count > sd.spiritball)
		count = sd.spiritball;
	sd.spiritball -= count;
	if(count > MAX_SKILL_LEVEL)
		count = MAX_SKILL_LEVEL;

	for(i=0;i<count;++i) {
		if(sd.spirit_timer[i] != -1) {
			delete_timer(sd.spirit_timer[i],pc_spiritball_timer);
			sd.spirit_timer[i] = -1;
		}
	}
	for(i=count;i<MAX_SKILL_LEVEL;++i) {
		sd.spirit_timer[i-count] = sd.spirit_timer[i];
		sd.spirit_timer[i] = -1;
	}

	if(!type)
		clif_spiritball(sd);

	return 0;
}


int pc_setrestartvalue(map_session_data &sd,int type)
{	//?生や養子の場合の元の職業を算出する
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);
	//-----------------------
	// 死亡した
	if( sd.state.restart_full_recover ||	// オシリスカ?ド
		sd.state.snovice_flag == 4)
	{	// [Celest]
		sd.status.hp=sd.status.max_hp;
		sd.status.sp=sd.status.max_sp;
		if (sd.state.snovice_flag == 4)
		{
			sd.state.snovice_flag = 0;
			status_change_start(&sd,(status_t)SkillStatusChangeTable[MO_STEELBODY],1,0,0,0,skill_get_time(MO_STEELBODY,1),0 );
		}
	}
	else
	{
		if(s_class.job == 0 && config.restart_hp_rate < 50)
		{	//ノビは半分回復
			sd.status.hp=(sd.status.max_hp)/2;
		}
		else
		{
			if(config.restart_hp_rate <= 0)
				sd.status.hp = 1;
			else
			{
				sd.status.hp = sd.status.max_hp * config.restart_hp_rate / 100;
				if(sd.status.hp <= 0)
					sd.status.hp = 1;
			}
		}
		if(config.restart_sp_rate > 0)
		{
			int sp = sd.status.max_sp * config.restart_sp_rate /100;
			if(sd.status.sp < sp)
				sd.status.sp = sp;
		}
	}
	if(type&1)
		clif_updatestatus(sd,SP_HP);
	if(type&1)
		clif_updatestatus(sd,SP_SP);

	// removed exp penalty on spawn [Valaris]
	if(type&2 && sd.status.class_ != 0 && config.zeny_penalty > 0 && !maps[sd.block_list::m].flag.nozenypenalty)
	{
		uint32 zeny = sd.status.zeny * config.zeny_penalty / 10000;
		if(zeny < 1) zeny = 1;
		if( sd.status.zeny < zeny )
			sd.status.zeny = 0;
		else
			sd.status.zeny -= zeny;

		clif_updatestatus(sd,SP_ZENY);
	}
	return 0;
}


/*==========================================
 * saveに必要なステ?タス修正を行なう
 *------------------------------------------
 */
int pc_makesavestatus(map_session_data &sd)
{	// 秒ﾌ色は色?弊害が多いので保存?象にはしない
	if(!config.save_clothcolor)
		sd.status.clothes_color=0;

	// 死亡?態だったのでhpを1、位置をセ?ブ場所に?更
	if( sd.is_dead() )
	{
		pc_setrestartvalue(sd,0);
		sd.status.last_point = sd.status.save_point;
	}
	else
	{
		memcpy(sd.status.last_point.mapname, sd.mapname,24);
		sd.status.last_point.x = sd.block_list::x;
		sd.status.last_point.y = sd.block_list::y;
	}

		// セ?ブ禁止マップだったので指定位置に移動
	if(maps[sd.block_list::m].flag.nosave)
	{
		if( strcmp(maps[sd.block_list::m].nosave.mapname,"SavePoint")==0 )
			sd.status.last_point = sd.status.save_point;
		else
			sd.status.last_point = maps[sd.block_list::m].nosave;
	}
	return 0;
}


unsigned short pc_equippoint(map_session_data &sd, unsigned short inx)
{
	int ep = 0;
	//?生や養子の場合の元の職業を算出する
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);

	if( inx<MAX_INVENTORY && sd.inventory_data[inx] )
	{
		ep = sd.inventory_data[inx]->equip;
		if(sd.inventory_data[inx]->look == 1 || sd.inventory_data[inx]->look == 2 || sd.inventory_data[inx]->look == 6)
		{
			if(ep == 2 && (sd.skill_check(AS_LEFT) > 0 || s_class.job == 12))
				return 34;
		}
	}
	return ep;
}

int pc_setinventorydata(map_session_data &sd)
{
	size_t i;
	unsigned short id;
	for(i=0;i<MAX_INVENTORY;++i) {
		id = sd.status.inventory[i].nameid;
		sd.inventory_data[i] = itemdb_search(id);
	}
	return 0;
}

int pc_calcweapontype(map_session_data &sd)
{
	if(sd.weapontype1 != 0 && sd.weapontype2 == 0)
		sd.status.weapon = sd.weapontype1;
	if(sd.weapontype1 == 0 && sd.weapontype2 != 0)// 左手武器 Only
		sd.status.weapon = sd.weapontype2;
	else if(sd.weapontype1 == 1 && sd.weapontype2 == 1)// ?短?
		sd.status.weapon = 0x11;
	else if(sd.weapontype1 == 2 && sd.weapontype2 == 2)// ??手?
		sd.status.weapon = 0x12;
	else if(sd.weapontype1 == 6 && sd.weapontype2 == 6)// ??手斧
		sd.status.weapon = 0x13;
	else if( (sd.weapontype1 == 1 && sd.weapontype2 == 2) ||
		(sd.weapontype1 == 2 && sd.weapontype2 == 1) ) // 短? - ?手?
		sd.status.weapon = 0x14;
	else if( (sd.weapontype1 == 1 && sd.weapontype2 == 6) ||
		(sd.weapontype1 == 6 && sd.weapontype2 == 1) ) // 短? - 斧
		sd.status.weapon = 0x15;
	else if( (sd.weapontype1 == 2 && sd.weapontype2 == 6) ||
		(sd.weapontype1 == 6 && sd.weapontype2 == 2) ) // ?手? - 斧
		sd.status.weapon = 0x16;
	else
		sd.status.weapon = sd.weapontype1;

	return 0;
}

int pc_setequipindex(map_session_data &sd)
{
	size_t i,j;

	for(i=0;i<MAX_EQUIP;++i)
		sd.equip_index[i] = 0xFFFF;

	for(i=0;i<MAX_INVENTORY;++i)
	{
		if(sd.status.inventory[i].nameid <= 0)
			continue;
		if(sd.status.inventory[i].equip)
		{
			for(j=0;j<MAX_EQUIP;++j)
				if(sd.status.inventory[i].equip & equip_pos[j])
					sd.equip_index[j] = i;
			if(sd.status.inventory[i].equip & 0x0002) {
				if(sd.inventory_data[i])
					sd.weapontype1 = sd.inventory_data[i]->look;
				else
					sd.weapontype1 = 0;
			}
			if(sd.status.inventory[i].equip & 0x0020) {
				if(sd.inventory_data[i]) {
					if(sd.inventory_data[i]->type == 4) {
						if(sd.status.inventory[i].equip == 0x0020)
							sd.weapontype2 = sd.inventory_data[i]->look;
						else
							sd.weapontype2 = 0;
					}
					else
						sd.weapontype2 = 0;
				}
				else
					sd.weapontype2 = 0;
			}
		}
	}
	pc_calcweapontype(sd);

	return 0;
}

bool pc_isequipable(map_session_data &sd, unsigned short inx)
{	//?生や養子の場合の元の職業を算出する
	struct item_data *item = sd.inventory_data[inx];
	
	if( config.gm_allequip>0 && sd.isGM()>=config.gm_allequip )
		return true;

	if(item == NULL)
		return false;

	if(item->flag.sex != 2 && sd.status.sex != item->flag.sex)
		return false;

	if(item->elv > 0 && sd.status.base_level < item->elv)
		return false;

	if(maps[sd.block_list::m].flag.pvp && (item->flag.no_equip&1))
		return false;

	if(maps[sd.block_list::m].flag.gvg && (item->flag.no_equip>1))
		return false;

	if((item->equip & 0x0002 || item->equip & 0x0020) && item->type == 4 && sd.has_status(SC_STRIPWEAPON))
		return false;

	if(item->equip & 0x0020 && item->type == 5 && sd.has_status(SC_STRIPSHIELD))
		return false;

	if(item->equip & 0x0010 && sd.has_status(SC_STRIPARMOR) )
		return false;

	if(item->equip & 0x0100 && sd.has_status(SC_STRIPHELM))
		return false;

	// Taekwon Class
	if (sd.status.class_ == JOB_TAEKWON)
	{	// cannot equip ANY weapons. [Lupus]
		// Any equipments that can be equipped by 1) either by everybody, 2) or everybody except Novice, can be equipped 
		if( item->type == 4 || (item->class_array & 126) != 126 ) //126 = all 1st classes but Novice
			return false;
	}
	// Star Gladiator
	else if (sd.status.class_ == JOB_STAR_GLADIATOR || sd.status.class_ == JOB_STAR_GLADIATOR2)
	{	// They can equip anything TK Boy can equip, plus, [Lupus]
		// 2nd-class specific items as well. In addition, they can equip these specific equipments:
		// All book-type items, Manteau, Mink Coat, Mirror Shield, Sharp Headgear, Majestic Goat, Boots.
		if( (item->nameid>=1550 && item->nameid<1599) //a Book type
			|| item->nameid==2505 || item->nameid==2506 //Manteau[0] [1]
			|| item->nameid==2107 || item->nameid==2108 //Mirror Shield [0] [1]
			|| item->nameid==2405 || item->nameid==2406 //Boots [0] [1]
			|| item->nameid==2256 || item->nameid==2258 //Majestic Goat | Spiky Band
//			|| item->nameid==2108 // What is ID of "Sharp Geadgear" ?
			)
			return true; //You can equip these exceptions
		if( item->type == 4 || !(((item->class_array & 126) == 126) || ((item->class_array & 2088832) == 2088832)) )	//2088832 = all 2nd classes but Super Novice
			return false;
	}
	//Soul Linker
	else if( sd.status.class_ == JOB_SOUL_LINKER ) { //[Lupus]
		//They can only equip equipments that Wizards can equip.
		if( (1<< JOB_WIZARD & item->class_array) == 0)
			return false;
	}
	//Other Common Classes
	else if ((1<<(pc_calc_base_job(sd.status.class_).job) & item->class_array) == 0) //[Skotlex]
		return false;


	return true;
}

//装備破壊
bool pc_break_equip(map_session_data &sd, unsigned short where)
{
	size_t i, j;

	if (sd.unbreakable_equip & where)
		return 0;
	if( where == EQP_WEAPON && 
		(sd.status.weapon ==  0 || // Added bare fists to the list [Skotlex]
		 sd.status.weapon ==  6 || sd.status.weapon ==  7 || sd.status.weapon == 8 || // Axes and Maces can't be broken [DracoRPG] 
		 sd.status.weapon == 10 || sd.status.weapon == 15) )	//Rods and Books can't be broken [Skotlex]
		return 0;

	if (sd.unbreakable >= rand()%100)
		return 0;

	switch (where)
	{
		case EQP_WEAPON:
			i = SC_CP_WEAPON;
			break;
		case EQP_ARMOR:
			i = SC_CP_ARMOR;
			break;
		case EQP_SHIELD:
			i = SC_CP_SHIELD;
			break;
		case EQP_HELM:
			i = SC_CP_HELM;
			break;
		default:
			return 0;
	}
	if ( sd.has_status((status_t)i))
		return true;

	for (i=0;i<MAX_EQUIP;++i)
	{
		if( (j = sd.equip_index[i]) <MAX_INVENTORY && sd.status.inventory[j].attribute != 1 &&
			((where == EQP_HELM && i == 6) ||
			(where == EQP_ARMOR && i == 7) ||
			 (where == EQP_WEAPON && (i == 8 || i == 9) && sd.inventory_data[j]->type == 4) ||
			 (where == EQP_SHIELD && i == 9 && sd.inventory_data[j]->type == 5)) )
		{
			char buf[64];
			sd.status.inventory[j].attribute = 1;
			snprintf(buf,sizeof(buf),"%s has broken.", sd.inventory_data[j]->name);
			clif_emotion(sd, 23);
			clif_displaymessage(sd.fd, buf);
			pc_unequipitem(sd, j, 3);
			clif_equiplist(sd);
			return true;
		}
	}
	return false;
}




/*==========================================
 * session idに問題無し
 * char鯖から送られてきたステ?タスを設定
 *------------------------------------------
 */
int pc_authok(uint32 charid, uint32 login_id2, time_t connect_until_time, unsigned char *buf)
{
	struct guild *g;
	size_t i;

	map_session_data *sd = map_session_data::charid2sd(charid);

	nullpo_retr(1, sd);
	sd->login_id2 = login_id2; // actually have this already
	mmo_charstatus_frombuffer(sd->status, buf);

	sd->view_class = sd->status.class_;

	// 基本的な初期化
	sd->state.connect_new = 1;
	sd->state.auth = 1;

	sd->change_level = pc_readglobalreg(*sd,"jobchange_level");

	// アカウント??の送信要求
	intif_request_accountreg(*sd);

	// アイテムチェック
	pc_setinventorydata(*sd);
	pc_checkitem(*sd);

	if ((config.atc_gmonly == 0 || sd->isGM()) &&
	    (sd->isGM() >= CommandInfo::get_level(command_hide)))
		sd->status.option &= (OPTION_MASK | OPTION_HIDE);
	else
		sd->status.option &= OPTION_MASK;

	// 位置の設定
	if( !pc_setpos(*sd,sd->status.last_point.mapname, sd->status.last_point.x, sd->status.last_point.y, 0) ) 
	{
		if(config.error_log) 
			ShowError("Last_point_map %s not found\n", sd->status.last_point.mapname);

		// try warping to a default map instead
		for(i=0; i<maps.size(); ++i)
		{
			if(maps[i].gat) 
				break;
		}
		if( i>=maps.size() || !pc_setpos(*sd, maps[i].mapname, 100, 100, 0) ) {
			// if we fail again
			clif_authfail(*sd, 0);
			return 1;
		}
	}

	// pet
	if (sd->status.pet_id > 0)
		intif_request_petdata(sd->status.account_id, sd->status.char_id, sd->status.pet_id);

	// hom
	if(sd->status.homun_id > 0)
		intif_request_homdata(sd->status.account_id,sd->status.char_id,sd->status.homun_id);


	// パ?ティ、ギルドデ?タの要求
	party_request_info(sd->status.party_id);

	if (sd->status.guild_id > 0)
	{
		if ((g = guild_search(sd->status.guild_id)) == NULL)
			guild_request_info(sd->status.guild_id);
		else if (strcmp(sd->status.name,g->master) == 0)
		{	//Block Guild Skills to prevent logout/login reuse exploiting. [Skotlex]
			int skill_num[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
			for (i = 0; i < 4; ++i)
				if (guild_checkskill(*g, skill_num[i]))
					pc_blockskill_start(*sd, skill_num[i], 300000);
		}
	}

	// Automated script events
	if (script_config.event_requires_trigger)
	{
		sd->state.event_death = pc_readglobalreg(*sd, script_config.die_event_name);
		sd->state.event_kill = pc_readglobalreg(*sd, script_config.kill_event_name);
		sd->state.event_disconnect = pc_readglobalreg(*sd, script_config.logout_event_name);
	}
	else
	{	// if script triggers are not required
		sd->state.event_death = 1;
		sd->state.event_kill = 1;
		sd->state.event_disconnect = 1;
	}

	// set proper termination function for the authentificated login
	session_SetTermFunction(sd->fd, clif_terminate);


	// 通知
	clif_authok(*sd);
	sd->add_nickdb();
	map_add_namedb(sd->status.char_id, sd->status.name);

	// init sd members from script variables
	sd->die_counter = pc_readglobalreg(*sd,"PC_DIE_COUNTER");
	sd->mission_target = pc_readglobalreg(*sd,"PC_MISSION_TARGET");
	if ((i = sd->skill_check(RG_PLAGIARISM)) > 0) {
		sd->cloneskill_id = pc_readglobalreg(*sd,"CLONE_SKILL");
		if (sd->cloneskill_id > 0) {
			sd->status.skill[sd->cloneskill_id].id = sd->cloneskill_id;
			//sd->status.skill[sd->cloneskill_id].lv = skill_get_max(sd->cloneskill_id);
			sd->status.skill[sd->cloneskill_id].lv = pc_readglobalreg(*sd,"CLONE_SKILL_LV");
			if (i < sd->status.skill[sd->cloneskill_id].lv)
				sd->status.skill[sd->cloneskill_id].lv = i;
			sd->status.skill[sd->cloneskill_id].flag = 13;	//cloneskill flag			
			clif_skillinfoblock(*sd);
		}
	}

	if(daynight_flag && !maps[sd->block_list::m].flag.indoors)
	{
		const char *str = msg_txt(MSG_CURRENTLY_ITS_THE_NIGHT); // Currently, it's the night...
		clif_wis_message(sd->fd, wisp_server_name, str, strlen(str)+1);
		clif_seteffect(*sd, sd->block_list::id, 474 + config.night_darkness_level);
	}

	// Send friends list
	clif_friend_send_info(*sd);
	if (config.display_version == 1)
	{
		char strbuf[256];
		snprintf(strbuf, sizeof(strbuf), "Revision: %s", get_revision());
		clif_displaymessage(sd->fd, strbuf);
	}

	// Message of the day
	{
		char strbuf[256];
		size_t sl;
		FILE *fp;
		if((fp = basics::safefopen(motd_txt, "r")) != NULL)
		{
			while( fgets(strbuf, sizeof(strbuf), fp) != NULL )
			{
				sl = prepare_line(strbuf);
				if(sl)
				{
					if(config.motd_type || sd->is_hiding() || sd->is_cloaking() || sd->is_chasewalk() )
						clif_disp_onlyself(*sd, strbuf);
					else
						clif_displaymessage(sd->fd, strbuf);
				}
			}
			fclose(fp);
		}
		else if(config.error_log)
			ShowWarning("In function pc_authok() -> File '"CL_WHITE"%s"CL_RESET"' not found.\n", motd_txt);

		snprintf(strbuf,sizeof(strbuf),"You are using client packet version %i", sd->packet_ver);
		clif_disp_onlyself(*sd,strbuf);
	}

	chrif_mail_check(*sd); // check mail at login [Valaris]

	// message of the limited time of the account
	if( connect_until_time != 0 )
	{	// don't display if it's unlimited
		char tmpstr[1024];
		// strftime counts the trailing EOS
		size_t sz = strftime(tmpstr, sizeof(tmpstr) - 1, msg_txt(MSG_ACCOUNT_TIME_LIMIT_IS_DMY_HMS), localtime(&connect_until_time)); // "Your account time limit is: %d-%m-%Y %H:%M:%S."
		clif_wis_message(sd->fd, wisp_server_name, tmpstr, sz);
	}

	// ステ?タス初期計算など
	status_calc_pc(*sd,1);


	if( sd->isGM() )
		ShowInfo("GM Character '"CL_WHITE"%s"CL_RESET"' logged in.\n"CL_SPACE"(Acc. ID: '"CL_WHITE"%d"CL_RESET"', GM Level '"CL_WHITE"%d"CL_RESET"') [connection %i, ver. %i].\n", sd->status.name, sd->status.account_id, sd->isGM(), sd->fd, sd->packet_ver);
	else
		ShowInfo("Character '"CL_WHITE"%s"CL_RESET"' logged in.\n"CL_SPACE"(Account ID: '"CL_WHITE"%d"CL_RESET"') [connection %i, ver. %i].\n", sd->status.name, sd->status.account_id, sd->fd, sd->packet_ver);

	npc_data::event("OnPCLoginEvent", script_config.login_event_name, *sd);

	return 0;
}

/*==========================================
 * session idに問題ありなので後始末
 *------------------------------------------
 */
int pc_authfail(int id)
{
	map_session_data *sd = map_session_data::from_blid(id);
	if (sd == NULL)
		return 1;

	clif_authfail(*sd, 0);

	return 0;
}

int pc_calc_skillpoint(map_session_data &sd)
{
	int i,inf2,skill,skill_point=0;

	for(i=1;i<MAX_SKILL;++i)
	{
		if( (skill = sd.skill_check(i)) > 0)
		{
			inf2 = skill_get_inf2(i);
			if( ( !(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) &&
				 !(inf2&INF2_WEDDING_SKILL) ) //Do not count wedding skills. [Skotlex]
			{
				if(!sd.status.skill[i].flag)
					skill_point += skill;
				else if(sd.status.skill[i].flag > 2 && sd.status.skill[i].flag != 13)
					skill_point += (sd.status.skill[i].flag - 2);
			}
		}
	}
	return skill_point;
}

/*==========================================
 * ?えられるスキルの計算
 *------------------------------------------
 */
int pc_calc_skilltree(map_session_data &sd)
{
	int i,id=0,flag;

	int c= pc_calc_skilltree_normalize_job(sd);
	
	for(i=0;i<MAX_SKILL;++i)
	{
		if (sd.status.skill[i].flag != 13)
			sd.status.skill[i].id=0;
		if (sd.status.skill[i].flag && sd.status.skill[i].flag != 13)
		{	// cardスキルなら、
			sd.status.skill[i].lv=(sd.status.skill[i].flag==1)?0:sd.status.skill[i].flag-2;	// 本?のlvに
			sd.status.skill[i].flag=0;	// flagは0にしておく
		}
	}

	if (config.gm_allskill > 0 && sd.isGM() >= config.gm_allskill)
	{
		// 全てのスキル
		for(i=1;i<158;++i)
			sd.status.skill[i].id=i;
		for(i=210;i<291;++i)
			sd.status.skill[i].id=i;
		for(i=304;i<331;++i)
			sd.status.skill[i].id=i;
		for(i=334;i<338;++i)			
			sd.status.skill[i].id=i;
		for(i=355;i<473;++i)
			sd.status.skill[i].id=i;
		for(i=475;i<492;++i)
			sd.status.skill[i].id=i;
	}
	else
	{
		do
		{
			flag=0; id=0;
			for(i=0; i<MAX_SKILL && (id=skill_tree[c][i].id)>0; ++i)
			{
				int j,f=1;
				if(!config.skillfree)
				{
					for(j=0;j<5;++j)
					{
						if( skill_tree[c][i].need[j].id &&
							sd.skill_check(skill_tree[c][i].need[j].id) < skill_tree[c][i].need[j].lv )
						{
							f=0;
							break;
						}
					}
					if( sd.status.job_level < skill_tree[c][i].joblv )
					{
						f=0;
					}
					else if (id >= 2 && id <= 53 && sd.skill_check( NV_BASIC) < 9)
					{
						f=0;
					}
				}
				if(f && sd.status.skill[id].id==0 )
				{
					sd.status.skill[id].id=id;
					flag=1;
				}
			}
		} while(flag);
	}
	return 0;
}

// Make sure all the skills are in the correct condition
// before persisting to the backend.. [MouseJstr]
int pc_clean_skilltree(map_session_data &sd)
{
	size_t i;
	for (i = 0; i < MAX_SKILL; ++i)
	{
		if (sd.status.skill[i].flag == 13)
		{
			sd.status.skill[i].id = 0;
			sd.status.skill[i].lv = 0;
			sd.status.skill[i].flag = 0;
		}
	}
	return 0;
}

int pc_calc_skilltree_normalize_job(map_session_data &sd)
{
	struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);
	int c = s_class.job;

	if (!config.skillup_limit || !((c >= JOB_NOVICE && c < JOB_SUPER_NOVICE) || c == JOB_TAEKWON))
		return sd.status.class_;
	else
	{
		int skill_point = pc_calc_skillpoint(sd);
		if(skill_point < 9)
			c = JOB_NOVICE;
		else if (((c > JOB_THIEF && c < JOB_SUPER_NOVICE) || c == JOB_TAEKWON) && sd.status.skill_point >= sd.status.job_level && ((sd.change_level > 0 && skill_point < sd.change_level+8) || skill_point < 58))
		{
			switch(c)
			{
				case JOB_KNIGHT:
				case JOB_KNIGHT2:
				case JOB_CRUSADER:
				case JOB_CRUSADER2:
					c = JOB_SWORDMAN;
					break;
				case JOB_PRIEST:
				case JOB_MONK:
					c = JOB_ACOLYTE;
					break;
				case JOB_WIZARD:
				case JOB_SAGE:
					c = JOB_MAGE;
					break;
				case JOB_BLACKSMITH:
				case JOB_ALCHEMIST:
					c = JOB_MERCHANT;
					break;
				case JOB_HUNTER:
				case JOB_BARD:
				case JOB_DANCER:
					c = JOB_ARCHER;
					break;
				case JOB_ASSASSIN:
				case JOB_ROGUE:
					c = JOB_THIEF;
					break;
				case JOB_STAR_GLADIATOR:
				case JOB_STAR_GLADIATOR2:
				case JOB_SOUL_LINKER:
					c = JOB_TAEKWON;
					break;
			}
		}
		if (s_class.upper == 1)
			c += JOB_NOVICE_HIGH;
		else if (s_class.upper == 2)
			c += JOB_BABY;
		return c;
	}
}
/*==========================================
 * 重量アイコンの確認
 *------------------------------------------
 */
int pc_checkweighticon(map_session_data &sd)
{
	int flag=0;

	if(sd.weight*100 >= sd.max_weight*config.natural_heal_weight_rate)
	//if(sd.weight*2 >= sd.max_weight)
		flag=1;
	if(sd.weight*10 >= sd.max_weight*9)
		flag=2;

	if(flag==1)
	{
		if( !sd.has_status(SC_WEIGHT50) )
			status_change_start(&sd,SC_WEIGHT50,0,0,0,0,0,0);
	}
	else
	{
		status_change_end(&sd,SC_WEIGHT50,-1);
	}
	if(flag==2)
	{
		if( sd.has_status(SC_WEIGHT90) )
			status_change_start(&sd,SC_WEIGHT90,0,0,0,0,0,0);
	}
	else
	{
		status_change_end(&sd,SC_WEIGHT90,-1);
	}
	return 0;
}

/*==========================================
 * ? 備品による能力等のボ?ナス設定
 *------------------------------------------
 */

inline void rangemodify(unsigned short &val, int change, unsigned short max=0xFFFF)
{	//keep the value within range
	if(change >0)
	{
		if( change<=(max-val) )
			val+=change;
		else
			val=max;
	}
	else
	{
		if( -change<val)
			val+=change;
		else
			val=0;
	}
}

int pc_bonus(map_session_data &sd,int type,int val)
{
	int i;
	switch(type){
	case SP_STR:
	case SP_AGI:
	case SP_VIT:
	case SP_INT:
	case SP_DEX:
	case SP_LUK:
		if(sd.state.lr_flag != 2)
			sd.parame[type-SP_STR]+=val;
		break;
	case SP_ATK1:
		if(!sd.state.lr_flag)
			sd.right_weapon.watk+=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.watk+=val;
		break;
	case SP_ATK2:
		if(!sd.state.lr_flag)
			sd.right_weapon.watk2+=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.watk2+=val;
		break;
	case SP_BASE_ATK:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.base_atk,val);
		break;
	case SP_MATK1:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.matk1,val);
		break;
	case SP_MATK2:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.matk2,val);
		break;
	case SP_MATK:
		if(sd.state.lr_flag != 2) {
			rangemodify(sd.matk1,val);
			rangemodify(sd.matk2,val);
		}
		break;
	case SP_DEF1:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.def,val);
		break;
	case SP_MDEF1:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.mdef,val);
		break;
	case SP_MDEF2:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.mdef,val);
		break;
	case SP_HIT:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.hit,val);
		else
			sd.arrow_hit+=val;
		break;
	case SP_FLEE1:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.flee,val);
		break;
	case SP_FLEE2:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.flee2,val*10);
		break;
	case SP_CRITICAL:
		if(sd.state.lr_flag != 2)
			sd.critical+=val*10;
		else
			sd.arrow_cri += val*10;
		break;
	case SP_ATKELE:
		if(!sd.state.lr_flag)
			sd.right_weapon.atk_ele=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.atk_ele=val;
		else if(sd.state.lr_flag == 2)
			sd.arrow_ele=val;
		break;
	case SP_DEFELE:
		if(sd.state.lr_flag != 2)
			sd.def_ele=val;
		break;
	case SP_MAXHP:
		if(sd.state.lr_flag != 2)
			sd.status.max_hp+=val;
		break;
	case SP_MAXSP:
		if(sd.state.lr_flag != 2)
			sd.status.max_sp+=val;
		break;
	case SP_CASTRATE:
		if(sd.state.lr_flag != 2)
			sd.castrate+=val;
		break;
	case SP_MAXHPRATE:
		if(sd.state.lr_flag != 2)
			sd.hprate+=val;
		break;
	case SP_MAXSPRATE:
		if(sd.state.lr_flag != 2)
			sd.sprate+=val;
		break;
	case SP_SPRATE:
		if(sd.state.lr_flag != 2)
			sd.dsprate+=val;
		break;
	case SP_ATTACKRANGE:
		if(!sd.state.lr_flag)
			rangemodify(sd.attackrange,val);
		else if(sd.state.lr_flag == 1)
			rangemodify(sd.attackrange_,val);
		else if(sd.state.lr_flag == 2)
			rangemodify(sd.arrow_range,val);
		break;
	case SP_ADD_SPEED:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.speed,-val);
		break;
	case SP_SPEED_RATE:
		if(sd.state.lr_flag != 2)
			if(sd.speed_rate > val)
				sd.speed_rate -= val;
			else
				sd.speed_rate = 0;
		break;
	case SP_SPEED_ADDRATE:
		if(sd.state.lr_flag != 2)
			sd.speed_add_rate = sd.speed_add_rate * (100-val)/100;
		break;
	case SP_ASPD:
		if(sd.state.lr_flag != 2)
			rangemodify(sd.aspd,-val*10);
		break;
	case SP_ASPD_RATE:
		if(sd.state.lr_flag != 2)
			if(sd.speed_rate > val)
				sd.aspd_rate -= val;
			else
				sd.aspd_rate = 0;
		break;
	case SP_ASPD_ADDRATE:	//Stackable increase - Made it linear as per rodatazone
		if(sd.state.lr_flag != 2)
			sd.aspd_add_rate -= val;
	case SP_HP_RECOV_RATE:
		if(sd.state.lr_flag != 2)
			sd.hprecov_rate += val;
		break;
	case SP_SP_RECOV_RATE:
		if(sd.state.lr_flag != 2)
			sd.sprecov_rate += val;
		break;
	case SP_CRITICAL_DEF:
		if(sd.state.lr_flag != 2)
			sd.critical_def += val;
		break;
	case SP_NEAR_ATK_DEF:
		if(sd.state.lr_flag != 2)
			sd.near_attack_def_rate += val;
		break;
	case SP_LONG_ATK_DEF:
		if(sd.state.lr_flag != 2)
			sd.long_attack_def_rate += val;
		break;
	case SP_DOUBLE_RATE:
		if(sd.state.lr_flag == 0 && sd.double_rate < val)
			sd.double_rate = val;
		break;
	case SP_DOUBLE_ADD_RATE:
		if(sd.state.lr_flag == 0)
			sd.double_add_rate += val;
		break;
	case SP_MATK_RATE:
		if(sd.state.lr_flag != 2)
			sd.matk_rate += val;
		break;
	case SP_IGNORE_DEF_ELE:
		if(!sd.state.lr_flag)
			sd.right_weapon.ignore_def_ele |= 1<<val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.ignore_def_ele |= 1<<val;
		break;
	case SP_IGNORE_DEF_RACE:
		if(!sd.state.lr_flag)
			sd.right_weapon.ignore_def_race |= 1<<val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.ignore_def_race |= 1<<val;
		break;
	case SP_ATK_RATE:
		if(sd.state.lr_flag != 2)
			sd.atk_rate += val;
		break;
	case SP_MAGIC_ATK_DEF:
		if(sd.state.lr_flag != 2)
			sd.magic_def_rate += val;
		break;
	case SP_MISC_ATK_DEF:
		if(sd.state.lr_flag != 2)
			sd.misc_def_rate += val;
		break;
	case SP_IGNORE_MDEF_ELE:
		if(sd.state.lr_flag != 2)
			sd.ignore_mdef_ele |= 1<<val;
		break;
	case SP_IGNORE_MDEF_RACE:
		if(sd.state.lr_flag != 2)
			sd.ignore_mdef_race |= 1<<val;
		break;
	case SP_PERFECT_HIT_RATE:
		if(sd.state.lr_flag != 2 && sd.perfect_hit < val)
			sd.perfect_hit = val;
		break;
	case SP_PERFECT_HIT_ADD_RATE:
		if(sd.state.lr_flag != 2)
			sd.perfect_hit_add += val;
		break;
	case SP_CRITICAL_RATE:
		if(sd.state.lr_flag != 2)
			sd.critical_rate+=val;
		break;
	case SP_GET_ZENY_NUM:
		if(sd.state.lr_flag != 2 && sd.get_zeny_num < val)
			sd.get_zeny_num = val;
		break;
	case SP_ADD_GET_ZENY_NUM:
		if(sd.state.lr_flag != 2)
			sd.get_zeny_add_num += val;
		break;
	case SP_DEF_RATIO_ATK_ELE:
		if(!sd.state.lr_flag)
			sd.right_weapon.def_ratio_atk_ele |= 1<<val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.def_ratio_atk_ele |= 1<<val;
		break;
	case SP_DEF_RATIO_ATK_RACE:
		if(!sd.state.lr_flag)
			sd.right_weapon.def_ratio_atk_race |= 1<<val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.def_ratio_atk_race |= 1<<val;
		break;
	case SP_HIT_RATE:
		if(sd.state.lr_flag != 2)
			sd.hit_rate += val;
		break;
	case SP_FLEE_RATE:
		if(sd.state.lr_flag != 2)
			sd.flee_rate += val;
		break;
	case SP_FLEE2_RATE:
		if(sd.state.lr_flag != 2)
			sd.flee2_rate += val;
		break;
	case SP_DEF_RATE:
		if(sd.state.lr_flag != 2)
			sd.def_rate += val;
		break;
	case SP_DEF2_RATE:
		if(sd.state.lr_flag != 2)
			sd.def2_rate += val;
		break;
	case SP_MDEF_RATE:
		if(sd.state.lr_flag != 2)
			sd.mdef_rate += val;
		break;
	case SP_MDEF2_RATE:
		if(sd.state.lr_flag != 2)
			sd.mdef2_rate += val;
		break;
	case SP_RESTART_FULL_RECOVER:
		if(sd.state.lr_flag != 2)
			sd.state.restart_full_recover = 1;
		break;
	case SP_NO_CASTCANCEL:
		if(sd.state.lr_flag != 2)
			sd.state.no_castcancel = 1;
		break;
	case SP_NO_CASTCANCEL2:
		if(sd.state.lr_flag != 2)
			sd.state.no_castcancel2 = 1;
		break;
	case SP_NO_SIZEFIX:
		if(sd.state.lr_flag != 2)
			sd.state.no_sizefix = 1;
		break;
	case SP_NO_MAGIC_DAMAGE:
		if(sd.state.lr_flag != 2)
			sd.state.no_magic_damage = 1;
		break;
	case SP_NO_WEAPON_DAMAGE:
		if(sd.state.lr_flag != 2)
			sd.state.no_weapon_damage = 1;
		break;
	case SP_NO_GEMSTONE:
		if(sd.state.lr_flag != 2)
			sd.state.no_gemstone = 1;
		break;
	case SP_INFINITE_ENDURE:
		if(sd.state.lr_flag != 2)
			sd.state.infinite_endure = 1;
		break;
	case SP_INTRAVISION: // Maya Purple Card effect allowing to see Hiding/Cloaking people [DracoRPG]
		if(sd.state.lr_flag != 2)
			sd.state.intravision = 1;
		break;
	case SP_SPLASH_RANGE:
		if(sd.state.lr_flag != 2 && sd.splash_range < val)
			sd.splash_range = val;
		break;
	case SP_SPLASH_ADD_RANGE:
		if(sd.state.lr_flag != 2)
			sd.splash_add_range += val;
		break;
	case SP_SHORT_WEAPON_DAMAGE_RETURN:
		if(sd.state.lr_flag != 2)
			sd.short_weapon_damage_return += val;
		break;
	case SP_LONG_WEAPON_DAMAGE_RETURN:
		if(sd.state.lr_flag != 2)
			sd.long_weapon_damage_return += val;
		break;
	case SP_MAGIC_DAMAGE_RETURN:
		if(sd.state.lr_flag != 2)
			sd.magic_damage_return += val;
		break;
	case SP_ALL_STATS:
		if(sd.state.lr_flag!=2) {
			sd.parame[SP_STR-SP_STR]+=val;
			sd.parame[SP_AGI-SP_STR]+=val;
			sd.parame[SP_VIT-SP_STR]+=val;
			sd.parame[SP_INT-SP_STR]+=val;
			sd.parame[SP_DEX-SP_STR]+=val;
			sd.parame[SP_LUK-SP_STR]+=val;
		}
		break;
	case SP_AGI_VIT:	// [Valaris]
		if(sd.state.lr_flag!=2) {
			sd.parame[SP_AGI-SP_STR]+=val;
			sd.parame[SP_VIT-SP_STR]+=val;
		}
		break;
	case SP_AGI_DEX_STR:	// [Valaris]
		if(sd.state.lr_flag!=2) {
			sd.parame[SP_AGI-SP_STR]+=val;
			sd.parame[SP_DEX-SP_STR]+=val;
			sd.parame[SP_STR-SP_STR]+=val;
		}
		break;
	case SP_PERFECT_HIDE: // [Valaris]
		if(sd.state.lr_flag!=2) {
			sd.state.perfect_hiding=1;
		}
		break;
	case SP_DISGUISE: // Disguise script for items [Valaris]
		if( sd.state.lr_flag!=2 && !sd.disguise_id && !sd.is_riding() )
		{
			clif_clearchar(sd, 0);
			sd.disguise_id=val;
			clif_changeoption(sd);
			clif_spawnpc(sd);
		}
		break;
	case SP_UNBREAKABLE:
		if(sd.state.lr_flag!=2) {
			sd.unbreakable += val;
		}
		break;
	case SP_UNBREAKABLE_WEAPON:
		if(sd.state.lr_flag != 2)
			sd.unbreakable_equip |= EQP_WEAPON;
		break;
	case SP_UNBREAKABLE_ARMOR:
		if(sd.state.lr_flag != 2)
			sd.unbreakable_equip |= EQP_ARMOR;
		break;
	case SP_UNBREAKABLE_HELM:
		if(sd.state.lr_flag != 2)
			sd.unbreakable_equip |= EQP_HELM;
		break;
	case SP_UNBREAKABLE_SHIELD:
		if(sd.state.lr_flag != 2)
			sd.unbreakable_equip |= EQP_SHIELD;
		break;
	case SP_CLASSCHANGE: // [Valaris]
		if(sd.state.lr_flag !=2){
			sd.classchange=val;
		}
		break;
	case SP_LONG_ATK_RATE:
		//if(sd.status.weapon == 11 && sd.state.lr_flag != 2)
		//	sd.atk_rate += val;
		if(sd.state.lr_flag != 2) //Rather than adding atk_rate only when the weapon is 11 (bow), make it a weapon_atk_rate. [Skotlex]
			sd.weapon_atk_rate[11]+=val;
		break;
	case SP_BREAK_WEAPON_RATE:
		if(sd.state.lr_flag != 2)
			sd.break_weapon_rate+=val;
		break;
	case SP_BREAK_ARMOR_RATE:
		if(sd.state.lr_flag != 2)
			sd.break_armor_rate+=val;
		break;
	case SP_ADD_STEAL_RATE:
		if(sd.state.lr_flag != 2)
			sd.add_steal_rate+=val;
		break;
	case SP_DELAYRATE:
		if(sd.state.lr_flag != 2)
			sd.delayrate+=val;
		break;
	case SP_CRIT_ATK_RATE:
		if(sd.state.lr_flag != 2)
			sd.crit_atk_rate += val;
		break;
	case SP_NO_REGEN:
		if(sd.state.lr_flag != 2)
			sd.no_regen = val;
		break;
	case SP_UNSTRIPABLE_WEAPON:
		if(sd.state.lr_flag != 2)
			sd.unstripable_equip |= EQP_WEAPON;
		break;
	case SP_UNSTRIPABLE:
	case SP_UNSTRIPABLE_ARMOR:
		if(sd.state.lr_flag != 2)
			sd.unstripable_equip |= EQP_ARMOR;
		break;
	case SP_UNSTRIPABLE_HELM:
		if(sd.state.lr_flag != 2)
			sd.unstripable_equip |= EQP_HELM;
		break;
	case SP_UNSTRIPABLE_SHIELD:
		if(sd.state.lr_flag != 2)
			sd.unstripable_equip |= EQP_SHIELD;
		break;
	case SP_SP_GAIN_VALUE:
		if(!sd.state.lr_flag)
			sd.sp_gain_value += val;
		break;
	case SP_IGNORE_DEF_MOB:	// 0:normal monsters only, 1:affects boss monsters as well
		if(!sd.state.lr_flag)
			sd.right_weapon.ignore_def_mob |= 1<<val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.ignore_def_mob |= 1<<val;
		break;
	case SP_HP_GAIN_VALUE:
		if(!sd.state.lr_flag)
			sd.hp_gain_value += val;
		break;
	case SP_DAMAGE_WHEN_UNEQUIP:
		if(!sd.state.lr_flag) {
			for (i=0; i<MAX_EQUIP; ++i) {
				if(sd.status.inventory[current_equip_item_index].equip & equip_pos[i]) {				
				
					sd.unequip_losehp[i] += val;
					break;
				}
			}
		}
		break;
	case SP_LOSESP_WHEN_UNEQUIP:
		if(!sd.state.lr_flag) {
			for (i=0; i<MAX_EQUIP; ++i) {
				if (sd.status.inventory[current_equip_item_index].equip & equip_pos[i]) {
					sd.unequip_losesp[i] += val;
					break;
				}
			}
		}
		break;
	default:
		if(config.error_log)
			ShowMessage("pc_bonus: unknown type %d %d !\n",type,val);
		break;
	}
	return 0;
}

/*==========================================
 * ? 備品による能力等のボ?ナス設定
 *------------------------------------------
 */
int pc_bonus2(map_session_data &sd,int type,int type2,int val)
{
	int i;

	switch(type){
	case SP_ADDELE:
		if(!sd.state.lr_flag)
			sd.right_weapon.addele[type2]+=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.addele[type2]+=val;
		else if(sd.state.lr_flag == 2)
			sd.arrow_addele[type2]+=val;
		break;
	case SP_ADDRACE:
		if(!sd.state.lr_flag)
			sd.right_weapon.addrace[type2]+=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.addrace[type2]+=val;
		else if(sd.state.lr_flag == 2)
			sd.arrow_addrace[type2]+=val;
		break;
	case SP_ADDSIZE:
		if(!sd.state.lr_flag)
			sd.right_weapon.addsize[type2]+=val;
		else if(sd.state.lr_flag == 1)
			sd.left_weapon.addsize[type2]+=val;
		else if(sd.state.lr_flag == 2)
			sd.arrow_addsize[type2]+=val;
		break;
	case SP_SUBELE:
		if(sd.state.lr_flag != 2)
			sd.subele[type2]+=val;
		break;
	case SP_SUBRACE:
		if(sd.state.lr_flag != 2)
			sd.subrace[type2]+=val;
		break;
	case SP_ADDEFF:
		if(sd.state.lr_flag != 2)
			sd.addeff[type2]+=val;
		else
			sd.arrow_addeff[type2]+=val;
		break;
	case SP_ADDEFF2:
		if(sd.state.lr_flag != 2)
			sd.addeff2[type2]+=val;
		else
			sd.arrow_addeff2[type2]+=val;
		break;
	case SP_RESEFF:
		if(sd.state.lr_flag != 2)
			sd.reseff[type2]+=val;
		break;
	case SP_MAGIC_ADDELE:
		if(sd.state.lr_flag != 2)
			sd.magic_addele[type2]+=val;
		break;
	case SP_MAGIC_ADDRACE:
		if(sd.state.lr_flag != 2)
			sd.magic_addrace[type2]+=val;
		break;
	case SP_MAGIC_SUBRACE:
		if(sd.state.lr_flag != 2)
			sd.magic_subrace[type2]+=val;
		break;
	case SP_ADD_DAMAGE_CLASS:
		if(!sd.state.lr_flag) {
			for(i=0;i<sd.right_weapon.add_damage_class_count;++i) {
				if(sd.right_weapon.add_damage_classid[i] == type2) {
					sd.right_weapon.add_damage_classrate[i] += val;
					break;
				}
			}
			if(i >= sd.right_weapon.add_damage_class_count && sd.right_weapon.add_damage_class_count < 10) {
				sd.right_weapon.add_damage_classid[sd.right_weapon.add_damage_class_count] = type2;
				sd.right_weapon.add_damage_classrate[sd.right_weapon.add_damage_class_count] += val;
				sd.right_weapon.add_damage_class_count++;
			}
		}
		else if(sd.state.lr_flag == 1) {
			for(i=0;i<sd.left_weapon.add_damage_class_count;++i) {
				if(sd.left_weapon.add_damage_classid[i] == type2) {
					sd.left_weapon.add_damage_classrate[i] += val;
					break;
				}
			}
			if(i >= sd.left_weapon.add_damage_class_count && sd.left_weapon.add_damage_class_count < 10) {
				sd.left_weapon.add_damage_classid[sd.left_weapon.add_damage_class_count] = type2;
				sd.left_weapon.add_damage_classrate[sd.left_weapon.add_damage_class_count] += val;
				sd.left_weapon.add_damage_class_count++;
			}
		}
		break;
	case SP_ADD_MAGIC_DAMAGE_CLASS:
		if(sd.state.lr_flag != 2) {
			for(i=0;i<sd.add_magic_damage_class_count;++i) {
				if(sd.add_magic_damage_classid[i] == type2) {
					sd.add_magic_damage_classrate[i] += val;
					break;
				}
			}
			if(i >= sd.add_magic_damage_class_count && sd.add_magic_damage_class_count < 10) {
				sd.add_magic_damage_classid[sd.add_magic_damage_class_count] = type2;
				sd.add_magic_damage_classrate[sd.add_magic_damage_class_count] += val;
				sd.add_magic_damage_class_count++;
			}
		}
		break;
	case SP_ADD_DEF_CLASS:
		if(sd.state.lr_flag != 2) {
			for(i=0;i<sd.add_def_class_count;++i) {
				if(sd.add_def_classid[i] == type2) {
					sd.add_def_classrate[i] += val;
					break;
				}
			}
			if(i >= sd.add_def_class_count && sd.add_def_class_count < 10) {
				sd.add_def_classid[sd.add_def_class_count] = type2;
				sd.add_def_classrate[sd.add_def_class_count] += val;
				sd.add_def_class_count++;
			}
		}
		break;
	case SP_ADD_MDEF_CLASS:
		if(sd.state.lr_flag != 2) {
			for(i=0;i<sd.add_mdef_class_count;++i) {
				if(sd.add_mdef_classid[i] == type2) {
					sd.add_mdef_classrate[i] += val;
					break;
				}
			}
			if(i >= sd.add_mdef_class_count && sd.add_mdef_class_count < 10) {
				sd.add_mdef_classid[sd.add_mdef_class_count] = type2;
				sd.add_mdef_classrate[sd.add_mdef_class_count] += val;
				sd.add_mdef_class_count++;
			}
		}
		break;
	case SP_HP_DRAIN_RATE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.hp_drain_rate += type2;
			sd.right_weapon.hp_drain_per += val;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.hp_drain_rate += type2;
			sd.left_weapon.hp_drain_per += val;
		}
		break;
	case SP_HP_DRAIN_VALUE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.hp_drain_rate += type2;
			sd.right_weapon.hp_drain_value += val;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.hp_drain_rate += type2;
			sd.left_weapon.hp_drain_value += val;
		}
		break;
	case SP_SP_DRAIN_RATE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.sp_drain_rate += type2;
			sd.right_weapon.sp_drain_per += val;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.sp_drain_rate += type2;
			sd.left_weapon.sp_drain_per += val;
		}
		sd.sp_drain_type = 0;
		break;
	case SP_SP_DRAIN_VALUE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.sp_drain_rate += type2;
			sd.right_weapon.sp_drain_value += val;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.sp_drain_rate += type2;
			sd.left_weapon.sp_drain_value += val;
		}
		sd.sp_drain_type = 0;
		break;
	case SP_WEAPON_COMA_ELE:
		if(sd.state.lr_flag != 2)
			sd.weapon_coma_ele[type2] += val;
		break;
	case SP_WEAPON_COMA_RACE:
		if(sd.state.lr_flag != 2)
			sd.weapon_coma_race[type2] += val;
		break;
	case SP_RANDOM_ATTACK_INCREASE:	// [Valaris]
		if(sd.state.lr_flag !=2){
			sd.random_attack_increase_add = type2;
			sd.random_attack_increase_per += val;
		}
		break;
	case SP_WEAPON_ATK:
		if(sd.state.lr_flag != 2)
			sd.weapon_atk[type2]+=val;
		break;
	case SP_WEAPON_ATK_RATE:
		if(sd.state.lr_flag != 2)
			sd.weapon_atk_rate[type2]+=val;
		break;
	case SP_CRITICAL_ADDRACE:
		if(sd.state.lr_flag != 2)
			sd.critaddrace[type2] += val*10;
		break;
	case SP_ADDEFF_WHENHIT:
		if(sd.state.lr_flag != 2)
			sd.addeff3[type2]+=val;
		break;
	case SP_SKILL_ATK:
		if(sd.state.lr_flag != 2) {
			if (sd.skillatk[0] == type2)
				sd.skillatk[1] += val;
			else {
				sd.skillatk[0] = type2;
				sd.skillatk[1] = val;
			}
		}
		break;
	case SP_ADD_DAMAGE_BY_CLASS:
		if(sd.state.lr_flag != 2) {
			for(i=0;i<sd.add_damage_class_count2;++i) {
				if(sd.add_damage_classid2[i] == type2) {
					sd.add_damage_classrate2[i] += val;
					break;
				}
			}
			if(i >= sd.add_damage_class_count2 && sd.add_damage_class_count2 < 10) {
				sd.add_damage_classid2[sd.add_damage_class_count2] = type2;
				sd.add_damage_classrate2[sd.add_damage_class_count2] += val;
				sd.add_damage_class_count2++;				
			}			
		}
		break;
	case SP_HP_LOSS_RATE:
		if(sd.state.lr_flag != 2) {
			sd.hp_loss_value = type2;
			sd.hp_loss_rate = val;
		}
		break;
	case SP_ADDRACE2:
		if (!(type2 > 0 && type2 < MAX_MOB_RACE_DB))
			if(sd.state.lr_flag != 2)
				sd.right_weapon.addrace2[type2] += val;
		else
				sd.left_weapon.addrace2[type2] += val;
		break;
	case SP_SUBSIZE:
		if(sd.state.lr_flag != 2)
			sd.subsize[type2]+=val;
		break;
	case SP_SUBRACE2:
		if(sd.state.lr_flag != 2)
			sd.subrace2[type2]+=val;
		break;
	case SP_ADD_ITEM_HEAL_RATE:
		if(sd.state.lr_flag != 2)
			sd.itemhealrate[type2 - 1] += val;
		break;
	case SP_EXP_ADDRACE:
		if(sd.state.lr_flag != 2)
			sd.expaddrace[type2]+=val;
		break;
	case SP_SP_GAIN_RACE:
		if(sd.state.lr_flag != 2)
			sd.sp_gain_race[type2]+=val;
		break;
	case SP_ADD_MONSTER_DROP_ITEM:
		if (sd.state.lr_flag != 2) {
			for(i = 0; i < sd.monster_drop_item_count; ++i) {
				if(sd.monster_drop_itemid[i] == type2) {
					sd.monster_drop_race[i] |= (1<<10)|(1<<11);
					if(sd.monster_drop_itemrate[i] < val)
						sd.monster_drop_itemrate[i] = val;
					break;
				}
			}
			if(i >= sd.monster_drop_item_count && sd.monster_drop_item_count < 10) {
				sd.monster_drop_itemid[sd.monster_drop_item_count] = type2;
				// all monsters, including boss and non boss monsters
				sd.monster_drop_race[sd.monster_drop_item_count] |= (1<<10)|(1<<11);
				sd.monster_drop_itemrate[sd.monster_drop_item_count] = val;
				sd.monster_drop_item_count++;
			}
		}
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP:
		if (sd.state.lr_flag != 2) {
			for(i = 0; i < sd.monster_drop_item_count; ++i) {
				if(sd.monster_drop_itemgroup[i] == type2) {
					sd.monster_drop_itemid[i] = 0;
					sd.monster_drop_race[i] |= (1<<10)|(1<<11);
					if(sd.monster_drop_itemrate[i] < val)
						sd.monster_drop_itemrate[i] = val;
					break;
				}
			}
			if(i >= sd.monster_drop_item_count && sd.monster_drop_item_count < 10) {
				sd.monster_drop_itemgroup[sd.monster_drop_item_count] = type2;
				sd.monster_drop_itemid[sd.monster_drop_item_count] = 0;
				// all monsters, including boss and non boss monsters
				sd.monster_drop_race[sd.monster_drop_item_count] |= (1<<10)|(1<<11);
				sd.monster_drop_itemrate[sd.monster_drop_item_count] = val;
				sd.monster_drop_item_count++;
			}
		}
		break;
	case SP_SP_LOSS_RATE:
		if(sd.state.lr_flag != 2) {
			sd.sp_loss_value = type2;
			sd.sp_loss_rate = val;
		}
		break;

	default:
		if(config.error_log)
			ShowMessage("pc_bonus2: unknown type %d %d %d!\n",type,type2,val);
		break;
	}
	return 0;
}

int pc_bonus3(map_session_data &sd,int type,int type2,int type3,int val)
{
	int i;

	switch(type){
	case SP_ADD_MONSTER_DROP_ITEM:
		if(sd.state.lr_flag != 2) {
			for(i=0;i<sd.monster_drop_item_count;++i) {
				if(sd.monster_drop_itemid[i] == type2) {
					sd.monster_drop_race[i] |= 1<<type3;
					if(sd.monster_drop_itemrate[i] < val)
						sd.monster_drop_itemrate[i] = val;
					break;
				}
			}
			if(i >= sd.monster_drop_item_count && sd.monster_drop_item_count < 10) {
				sd.monster_drop_itemid[sd.monster_drop_item_count] = type2;
				sd.monster_drop_race[sd.monster_drop_item_count] |= 1<<type3;
				sd.monster_drop_itemrate[sd.monster_drop_item_count] = val;
				sd.monster_drop_item_count++;
			}
		}
		break;
	case SP_AUTOSPELL:
		if(sd.state.lr_flag != 2) {
			for (i = 0; i < 10; ++i) {
				if (sd.autospell_id[i] == 0 ||
					(sd.autospell_id[i] == type2 && sd.autospell_lv[i] < type3) ||
					(sd.autospell_id[i] == type2 && sd.autospell_lv[i] == type3 && sd.autospell_rate[i] < val))
				{
					sd.autospell_id[i] = type2;
					sd.autospell_lv[i] = type3;
					sd.autospell_rate[i] = val;
					break;
				}
			}
		}
		break;
	case SP_AUTOSPELL_WHENHIT:
		if(sd.state.lr_flag != 2) {
			for (i = 0; i < 10; ++i) {
				if (sd.autospell2_id[i] == 0 ||
					(sd.autospell2_id[i] == type2 && sd.autospell2_lv[i] < type3) ||
					(sd.autospell2_id[i] == type2 && sd.autospell2_lv[i] == type3 && sd.autospell2_rate[i] < val))
				{
					sd.autospell2_id[i] = type2;
					sd.autospell2_lv[i] = type3;
					sd.autospell2_rate[i] = val;
					break;
				}
			}
		}
		break;
	case SP_HP_LOSS_RATE:
		if(sd.state.lr_flag != 2) {
			sd.hp_loss_value = type2;
			sd.hp_loss_rate = type3;
			sd.hp_loss_type = val;
		}
		break;
	case SP_SP_DRAIN_RATE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.sp_drain_rate += type2;
			sd.right_weapon.sp_drain_per += type3;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.sp_drain_rate += type2;
			sd.left_weapon.sp_drain_per += type3;
		}
		sd.sp_drain_type = val;
		break;
	case SP_SP_DRAIN_VALUE:
		if(!sd.state.lr_flag) {
			sd.right_weapon.sp_drain_rate += type2;
			sd.right_weapon.sp_drain_value += type3;
		}
		else if(sd.state.lr_flag == 1) {
			sd.left_weapon.sp_drain_rate += type2;
			sd.left_weapon.sp_drain_value += type3;
		}
		sd.sp_drain_type = val;
		break;
	case SP_ADD_MONSTER_DROP_ITEMGROUP:
		if (sd.state.lr_flag != 2) {
			for(i = 0; i < sd.monster_drop_item_count; ++i) {
				if(sd.monster_drop_itemgroup[i] == type2) {
					sd.monster_drop_itemid[i] = 0;
					sd.monster_drop_race[i] |= 1<<type3;
					if(sd.monster_drop_itemrate[i] < val)
						sd.monster_drop_itemrate[i] = val;
					break;
				}
			}
			if(i >= sd.monster_drop_item_count && sd.monster_drop_item_count < 10) {
				sd.monster_drop_itemgroup[sd.monster_drop_item_count] = type2;
				sd.monster_drop_itemid[sd.monster_drop_item_count] = 0;
				// all monsters, including boss and non boss monsters
				sd.monster_drop_race[sd.monster_drop_item_count] |= 1<<type3;
				sd.monster_drop_itemrate[sd.monster_drop_item_count] = val;
				sd.monster_drop_item_count++;
			}
		}
		break;

	default:
		if(config.error_log)
			ShowMessage("pc_bonus3: unknown type %d %d %d %d!\n",type,type2,type3,val);
		break;
	}

	return 0;
}

int pc_bonus4(map_session_data &sd,int type,int type2,int type3,int type4,int val)
{
	int i;

	switch(type){
	case SP_AUTOSPELL:
		if(sd.state.lr_flag != 2) {
			for (i = 0; i < 10; ++i) {
				if (sd.autospell_id[i] == 0 ||
					(sd.autospell_id[i] == type2 && sd.autospell_lv[i] < type3) ||
					(sd.autospell_id[i] == type2 && sd.autospell_lv[i] == type3 && sd.autospell_rate[i] < type4))
				{
					sd.autospell_id[i] = (val) ? type2 : -type2;		// val = 0: self, 1: enemy
					sd.autospell_lv[i] = type3;
					sd.autospell_rate[i] = type4;
					break;
				}
			}
		}			
		break;
	case SP_AUTOSPELL_WHENHIT:
		if(sd.state.lr_flag != 2) {
			for (i = 0; i < 10; ++i) {
				if (sd.autospell2_id[i] == 0 ||
					(sd.autospell2_id[i] == type2 && sd.autospell2_lv[i] < type3) ||
					(sd.autospell2_id[i] == type2 && sd.autospell2_lv[i] == type3 && sd.autospell2_rate[i] < type4))
				{
					sd.autospell2_id[i] = (val) ? type2 : -type2;		// val = 0: self, 1: enemy
					sd.autospell2_lv[i] = type3;
					sd.autospell2_rate[i] = type4;
					break;
				}
			}
		}
		break;
	default:
		if(config.error_log)
			ShowWarning("pc_bonus4: unknown type %d %d %d %d %d!\n",type,type2,type3,type4,val);
		break;
	}

	return 0;
}

/*==========================================
 * スクリプトによるスキル所得
 *------------------------------------------
 */
int pc_skill(map_session_data &sd,unsigned short skillid,unsigned short skilllvl,int flag)
{
	if(skilllvl > MAX_SKILL_LEVEL){
		if(config.error_log)
			ShowMessage("support card skill only!\n");
		return 0;
	}
	if(!flag && (sd.status.skill[skillid].id == skillid || skilllvl == 0)){	// クエスト所得ならここで?件を確認して送信する
		sd.status.skill[skillid].lv=skilllvl;
		status_calc_pc(sd,0);
		clif_skillinfoblock(sd);
	}
	else if(flag==2 && (sd.status.skill[skillid].id == skillid || skilllvl == 0)){	// クエスト所得ならここで?件を確認して送信する
		sd.status.skill[skillid].lv+=skilllvl;
		status_calc_pc(sd,0);
		clif_skillinfoblock(sd);
	}
	else if(sd.status.skill[skillid].lv < skilllvl)
	{	// ?えられるがlvが小さいなら
		if(sd.status.skill[skillid].id==skillid)
			sd.status.skill[skillid].flag=sd.status.skill[skillid].lv+2;	// lvを記憶
		else
		{
			sd.status.skill[skillid].id=skillid;
			sd.status.skill[skillid].flag=1;	// cardスキルとする
		}
		sd.status.skill[skillid].lv=skilllvl;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int pc_blockskill_end(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd = map_session_data::from_blid(id);
	if (sd && data.num > 0 && data.num < MAX_SKILL)
	{
		sd->blockskill[data.num] = 0;
	}
	return 1;
}
void pc_blockskill_start(map_session_data &sd, unsigned short skillid, unsigned long tick)
{
	if (skillid >= 10000 && skillid < 10015)
		skillid -= 9500;
	else if (skillid < 1 || skillid > MAX_SKILL)
		return;

	sd.blockskill[skillid] = 1;
	add_timer(gettick()+tick,pc_blockskill_end,sd.block_list::id,skillid);
}

/*==========================================
 * カ?ド?入
 *------------------------------------------
 */
int pc_insert_card(map_session_data &sd, unsigned short idx_card, unsigned short idx_equip)
{
	if(idx_card < MAX_INVENTORY && idx_equip < MAX_INVENTORY && sd.inventory_data[idx_card])
	{
		size_t i;
		unsigned short nameid=sd.status.inventory[idx_equip].nameid;
		unsigned short cardid=sd.status.inventory[idx_card].nameid;
		unsigned short ep    =sd.inventory_data[idx_card]->equip;

		if( nameid <= 0 || sd.inventory_data[idx_equip] == NULL ||
			( sd.inventory_data[idx_equip]->type!=4 && sd.inventory_data[idx_equip]->type!=5) ||	// ? 備じゃない
			( sd.status.inventory[idx_equip].identify==0 ) ||		// 未鑑定
			( sd.inventory_data[idx_card]->type!=6)|| // Prevent Hack [Ancyker]
			( sd.status.inventory[idx_equip].card[0]==0x00ff) ||		// 製造武器
			( sd.status.inventory[idx_equip].card[0]==0x00fe) ||
			((sd.inventory_data[idx_equip]->equip&ep)==0 ) ||					// ? 備個所違い
			( sd.inventory_data[idx_equip]->type==4 && ep==32) ||			// ? 手武器と盾カ?ド
			( sd.status.inventory[idx_equip].card[0]==0xff00) || sd.status.inventory[idx_equip].equip){

			clif_insert_card(sd,idx_equip,idx_card,1);
			return 0;
		}
		for(i=0;i<sd.inventory_data[idx_equip]->flag.slot;++i)
		{
			if( sd.status.inventory[idx_equip].card[i] == 0)
			{	// 空きスロットがあったので差し?む
				sd.status.inventory[idx_equip].card[i]=cardid;
			// カ?ドは減らす
				clif_insert_card(sd,idx_equip,idx_card,0);
				pc_delitem(sd,idx_card,1,1);
				return 0;
			}
		}
	}
	else
		clif_insert_card(sd,idx_equip,idx_card,1);

	return 0;
}

//
// アイテム物
//

/*==========================================
 * スキルによる買い値修正
 *------------------------------------------
 */
int pc_modifybuyvalue(map_session_data &sd,size_t orig_value)
{
	int skill,rate1 = 0,rate2 = 0;
	size_t val = orig_value;
	
	if((skill=sd.skill_check(MC_DISCOUNT))>0)	// ディスカウント
		rate1 = 5+skill*2-((skill==10)? 1:0);
	if((skill=sd.skill_check(RG_COMPULSION))>0)	// コムパルションディスカウント
		rate2 = 5+skill*4;
	if(rate1 < rate2) rate1 = rate2;
	if(rate1)
		val = orig_value*(100-rate1)/100;

	if(orig_value > 0 && val < 1) val = 1;

	return val;
}

/*==========================================
 * スキルによる?り値修正
 *------------------------------------------
 */
int pc_modifysellvalue(map_session_data &sd,size_t orig_value)
{
	int skill,rate = 0;
	size_t val = orig_value;
	if((skill=sd.skill_check(MC_OVERCHARGE))>0)	// オ?バ?チャ?ジ
		rate = 5+skill*2-((skill==10)? 1:0);
	if(rate)
		val = orig_value*(100+rate)/100;
	if(orig_value > 0 && val < 1) val = 1;

	return val;
}

/*==========================================
 * アイテムを買った暫ﾉ、新しいアイテム欄を使うか、
 * 3万個制限にかかるか確認
 *------------------------------------------
 */
int pc_checkadditem(map_session_data &sd,unsigned short nameid,unsigned short amount)
{
	int i;

	if(itemdb_isSingleStorage(nameid))
		return ADDITEM_NEW;

	if(amount > MAX_AMOUNT)
				return ADDITEM_OVERAMOUNT;

	for(i=0;i<MAX_INVENTORY;++i)
	{
		if(sd.status.inventory[i].nameid==nameid)
		{
			if(sd.status.inventory[i].amount+amount > MAX_AMOUNT)
				return ADDITEM_OVERAMOUNT;
			return ADDITEM_EXIST;
		}
	}
	return ADDITEM_NEW;
}

/*==========================================
 * 空きアイテム欄の個?
 *------------------------------------------
 */
size_t pc_inventoryblank(map_session_data &sd)
{
	size_t i,b;
	for(i=0,b=0;i<MAX_INVENTORY;++i)
	{
		if(sd.status.inventory[i].nameid==0)
			b++;
	}

	return b;
}

/*==========================================
 * お金を?う
 *------------------------------------------
 */
bool pc_payzeny(map_session_data &sd,uint32 zeny)
{
	if( sd.status.zeny < zeny )
		return false;

	sd.status.zeny-=zeny;
	clif_updatestatus(sd,SP_ZENY);
	return true;
}

/*==========================================
 * お金を得る
 *------------------------------------------
 */
bool pc_getzeny(map_session_data &sd,uint32 zeny)
{
	uint32 z = sd.status.zeny+zeny;
	if( z > MAX_ZENY || z<sd.status.zeny) // max or overflow
	{
		sd.status.zeny = MAX_ZENY;
	}
	else
	{
		sd.status.zeny = z;
	}
	clif_updatestatus(sd,SP_ZENY);
	return true;
}

/*==========================================
 * アイテムを探して、インデックスを返す
 *------------------------------------------
 */
int pc_search_inventory(map_session_data &sd,int item_id)
{
	size_t i;
	for(i=0;i<MAX_INVENTORY;++i)
	{
		if( sd.status.inventory[i].nameid == item_id &&
			(sd.status.inventory[i].amount > 0 || item_id == 0) )
			return i;
	}
	return -1;
}

/*==========================================
 * アイテム追加。個?のみitem構造?の?字を無視
 *------------------------------------------
 */
int pc_additem(map_session_data &sd,struct item &item_data,size_t amount)
{
	struct item_data *data;
	size_t i,w;

	if(item_data.nameid <= 0 || amount <= 0)
		return 1;
	data = itemdb_exists(item_data.nameid);
	if(!data || (w = data->weight*amount)+ sd.weight > sd.max_weight)
		return 2;

	i = MAX_INVENTORY;
	if( !itemdb_isSingleStorage(*data) )
	{	// 装 備品ではないので、既所有品なら個数のみ変化させる
		for (i = 0; i < MAX_INVENTORY; ++i)
			{
			if( sd.status.inventory[i].nameid  == item_data.nameid &&
				sd.status.inventory[i].card[0] == item_data.card[0] &&
				sd.status.inventory[i].card[1] == item_data.card[1] &&
				sd.status.inventory[i].card[2] == item_data.card[2] &&
				sd.status.inventory[i].card[3] == item_data.card[3])
			{
				if (sd.status.inventory[i].amount + amount > MAX_AMOUNT)
					return 5;
				sd.status.inventory[i].amount += amount;
				clif_additem(sd,i,amount,0);
				break;
			}
		}
	}
	if (i >= MAX_INVENTORY)
	{	// 装 備品か未所有品だったので空き欄へ追加
		i = pc_search_inventory(sd,0);
		if(i < MAX_INVENTORY)
		{	// clear equips field first, just in case
			if (item_data.equip != 0)
				item_data.equip = 0;
			memcpy(&sd.status.inventory[i], &item_data, sizeof(sd.status.inventory[0]));
			sd.status.inventory[i].amount = amount;
			sd.inventory_data[i] = data;
			clif_additem(sd,i,amount,0);
		}
		else return 4;
	}
	sd.weight += w;
	clif_updatestatus(sd,SP_WEIGHT);

	return 0;
}

/*==========================================
 * アイテムを減らす
 *------------------------------------------
 */
int pc_delitem(map_session_data &sd, unsigned short inx, size_t amount, int type)
{
	if(inx >= MAX_INVENTORY || sd.status.inventory[inx].nameid==0 || 
		amount <= 0 || sd.status.inventory[inx].amount<amount || 
		sd.inventory_data[inx] == NULL)
		return 1;

	sd.status.inventory[inx].amount -= amount;
	sd.weight -= sd.inventory_data[inx]->weight*amount;
	if(sd.status.inventory[inx].amount<=0)
	{
		if(sd.status.inventory[inx].equip)
			pc_unequipitem(sd,inx,3);
		memset(&sd.status.inventory[inx],0,sizeof(sd.status.inventory[0]));
		sd.inventory_data[inx] = NULL;
	}
	if(!(type&1))
		clif_delitem(sd,inx,amount);
	if(!(type&2))
		clif_updatestatus(sd,SP_WEIGHT);

	return 0;
}

/*==========================================
 * アイテムを落す
 *------------------------------------------
 */
int pc_dropitem(map_session_data &sd,unsigned short inx, size_t amount)
{
	if( inx >= MAX_INVENTORY || amount <= 0 )
		return 1;

	if( sd.status.inventory[inx].nameid <= 0 ||
	    sd.status.inventory[inx].amount < amount ||
	    sd.trade_partner != 0 || 
		sd.vender_id != 0 ||
	    sd.status.inventory[inx].amount <= 0 )
		return 1;
	
	if( !itemdb_isdropable(sd.status.inventory[inx].nameid, sd.isGM()) )
	{	//The client does not likes being silently ignored, so we send it a del of 0 qty
		clif_delitem(sd,inx,0);
		clif_displaymessage (sd.fd, msg_txt(MSG_ITEM_CANNOT_BE_DROPPED));
		return 1;
	}

	if( flooritem_data::create(sd.status.inventory[inx], amount, sd.block_list::m, sd.block_list::x, sd.block_list::y, NULL, NULL, NULL, 0) )
		pc_delitem(sd, inx, amount, 0);
	else
		clif_delitem(sd,inx,0);
	return 0;
}

/*==========================================
 * アイテムを拾う
 *------------------------------------------
 */
int pc_takeitem(map_session_data &sd, flooritem_data &fitem)
{
	int flag;
	unsigned long tick = gettick();
	map_session_data *first_sd = NULL,*second_sd = NULL,*third_sd = NULL;

	if( distance(fitem, sd)>2 )
		return 0;	// 距離が遠い

	if(fitem.first_get_id > 0)
	{
		first_sd = map_session_data::from_blid(fitem.first_get_id);
		if(tick < fitem.first_get_tick)
		{
			if(fitem.first_get_id != sd.block_list::id && !(first_sd && first_sd->status.party_id == sd.status.party_id))
			{
				clif_additem(sd,0,0,6);
				return 0;
			}
		}
		else if( fitem.second_get_id > 0 )
		{
			second_sd = map_session_data::from_blid(fitem.second_get_id);
			if(tick < fitem.second_get_tick)
			{
				if( fitem.first_get_id != sd.block_list::id && fitem.second_get_id != sd.block_list::id &&
					!(first_sd && first_sd->status.party_id == sd.status.party_id) && !(second_sd && second_sd->status.party_id == sd.status.party_id))
				{
					clif_additem(sd,0,0,6);
					return 0;
				}
			}
			else if(fitem.third_get_id > 0)
			{
				third_sd = map_session_data::from_blid(fitem.third_get_id);
				if(tick < fitem.third_get_tick)
				{
					if(fitem.first_get_id != sd.block_list::id && fitem.second_get_id != sd.block_list::id && fitem.third_get_id != sd.block_list::id &&
						!(first_sd && first_sd->status.party_id == sd.status.party_id) && !(second_sd && second_sd->status.party_id == sd.status.party_id) &&
						!(third_sd && third_sd->status.party_id == sd.status.party_id)) {
						clif_additem(sd,0,0,6);
						return 0;
					}
				}
			}
		}
	}
	if((flag = pc_additem(sd,fitem.item_data,fitem.item_data.amount)))
		// 重量overで取得失敗
		clif_additem(sd,0,0,flag);
	else
	{	// 取得成功 
		sd.stop_attack();
		clif_takeitem(sd,fitem);
		fitem.item_data = item();
		fitem.freeblock();
	}
	return 0;
}

bool pc_isUseitem(map_session_data &sd, unsigned short inx)
{
	struct item_data *item;
	unsigned short nameid;

	if(inx >= MAX_INVENTORY || (item = sd.inventory_data[inx]) == NULL)
		return 0;
	nameid = sd.status.inventory[inx].nameid;

	if(item->type != 0 && item->type != 2)
		return 0;
	if((nameid == 605) && maps[sd.block_list::m].flag.gvg)
		return 0;
	if(nameid == 601 && (maps[sd.block_list::m].flag.noteleport || maps[sd.block_list::m].flag.gvg)) {
		clif_skill_teleportmessage(sd,0);
		return 0;
	}
	if(nameid == 602 && maps[sd.block_list::m].flag.noreturn)
		return 0;
	if(nameid == 604 && (maps[sd.block_list::m].flag.nobranch || maps[sd.block_list::m].flag.gvg))
		return 0;
	if(item->flag.sex != 2 && sd.status.sex != item->flag.sex)
		return 0;
	if(item->elv > 0 && sd.status.base_level < item->elv)
		return 0;
	if(((sd.status.class_==13 || sd.status.class_==4014) && ((1<<7)&item->class_array) == 0) || // have mounted classes use unmounted items [Valaris]
		((sd.status.class_==21 || sd.status.class_==4022) && ((1<<14)&item->class_array) == 0))
		return 0;
	if(sd.status.class_!=13 && sd.status.class_!=4014 && sd.status.class_!=21 && sd.status.class_!=4022)
	if((sd.status.class_<=4000 && ((1<<sd.status.class_)&item->class_array) == 0) || (sd.status.class_>4000 && sd.status.class_<4023 && ((1<<(sd.status.class_-4001))&item->class_array) == 0) ||
	  (sd.status.class_>=4023 && ((1<<(sd.status.class_-4023))&item->class_array) == 0))
		return 0;

	if((log_config.branch > 0) && (nameid == 604))
		log_branch(sd);

	return 1;
}

/*==========================================
 * アイテムを使う
 *------------------------------------------
 */
int pc_useitem(map_session_data &sd, unsigned short inx)
{
	if(inx < MAX_INVENTORY)
	{
		script_object* script = sd.inventory_data[inx]->use_script;
		unsigned short amount = sd.status.inventory[inx].amount;
		sd.itemid = sd.status.inventory[inx].nameid;
		sd.itemindex = inx;

		if( sd.status.inventory[inx].nameid <= 0 ||
			sd.status.inventory[inx].amount <= 0 ||
			sd.inventory_data[inx]->use_script==NULL || sd.inventory_data[inx]->use_script->script==NULL ||
			gettick() < sd.canuseitem_tick || //Prevent mass item usage. [Skotlex]
			sd.has_status(SC_BERSERK) ||
			sd.has_status(SC_MARIONETTE) ||
			sd.has_status(SC_GRAVITATION) ||
			(sd.is_sitting() && (sd.itemid == 605 || sd.itemid == 606)) ||
			//added item_noequip.txt items check by Maya&[Lupus]
			(maps[sd.block_list::m].flag.pvp && (sd.inventory_data[inx]->flag.no_equip&1) ) || // PVP
			(maps[sd.block_list::m].flag.gvg && (sd.inventory_data[inx]->flag.no_equip>1) ) || // GVG
			!pc_isUseitem(sd,inx) )
		{
			clif_useitemack(sd,inx,0,0);
			return 1;
		}


		//Check if the item is to be consumed inmediately [Skotlex]
		if (sd.inventory_data[inx]->flag.delay_consume)
		{
			clif_useitemack(sd,inx,amount,1);
		}
		else
		{
			clif_useitemack(sd,inx,amount-1,1);
			pc_delitem(sd,inx,1,1);
		}
		if( sd.status.inventory[inx].card[0]==0x00fe && 
			chrif_istop10fame(basics::MakeDWord(sd.status.inventory[inx].card[2],sd.status.inventory[inx].card[3]),FAME_CHEM) )
		    sd.state.potion_flag = 2;

		CScriptEngine::run(script->script,0,sd.block_list::id,0);
		sd.state.potion_flag = 0;

		sd.canuseitem_tick= gettick() + config.item_use_interval; //Update item use time.
	}
	return 0;
}

/*==========================================
 * カ?トアイテム追加。個?のみitem構造?の?字を無視
 *------------------------------------------
 */
int pc_cart_additem(map_session_data &sd, struct item &item_data, size_t amount)
{
	struct item_data *data;
	size_t i,w;

	if(item_data.nameid <= 0 || amount <= 0)
		return 1;

	data = itemdb_exists(item_data.nameid);

	if(!itemdb_cancartstore(item_data.nameid, sd.isGM()))
	{	//Check item trade restrictions	[Skotlex]
		clif_displaymessage (sd.fd, msg_txt(MSG_ITEM_CANNOT_BE_STORED));
		return 1;
	}

	if((w=data->weight*amount) + sd.cart_weight > sd.cart_max_weight)
	{
		clif_displaymessage (sd.fd, "cart too heavy");
		return 1;
	}

	i=MAX_CART;
	if(!itemdb_isSingleStorage(*data))
	{
		// 装 備品ではないので、既所有品なら個数のみ変化させる
		for(i=0;i<MAX_CART;++i)
		{
			if( sd.status.cart[i].nameid  == item_data.nameid &&
				sd.status.cart[i].card[0] == item_data.card[0] && 
				sd.status.cart[i].card[1] == item_data.card[1] &&
				sd.status.cart[i].card[2] == item_data.card[2] && 
				sd.status.cart[i].card[3] == item_data.card[3] )
			{
				if(sd.status.cart[i].amount+amount > MAX_AMOUNT)
					return 1;
				sd.status.cart[i].amount+=amount;
				clif_cart_additem(sd,i,amount,0);
				break;
			}
		}
	}
	if(i >= MAX_CART)
	{
		// 装 備品か未所有品だったので空き欄へ追加
		for(i=0;i<MAX_CART;++i)
		{
			if(sd.status.cart[i].nameid==0){
				memcpy(&sd.status.cart[i],&item_data,sizeof(sd.status.cart[0]));
				if( itemdb_isSingleStorage(*data) )
				{
					sd.status.cart[i].amount=1;
					amount=1;
				}
				else
				{
					sd.status.cart[i].amount=amount;
				}
				sd.cart_num++;
				clif_cart_additem(sd,i,amount,0);
				break;
			}
		}
		if(i >= MAX_CART)
			return 1;
	}
	sd.cart_weight += w;
	clif_updatestatus(sd,SP_CARTINFO);

	return 0;
}


/*==========================================
 * カ?トアイテムを減らす
 *------------------------------------------
 */
int pc_cart_delitem(map_session_data &sd,unsigned short inx, size_t amount,int type)
{
	if( inx >= MAX_CART ||
		sd.status.cart[inx].nameid==0 ||
		sd.status.cart[inx].amount<amount )
		return 1;

	sd.status.cart[inx].amount -= amount;
	sd.cart_weight -= itemdb_weight(sd.status.cart[inx].nameid)*amount;
	if( sd.status.cart[inx].amount <= 0 )
	{
		memset(&sd.status.cart[inx],0,sizeof(sd.status.cart[0]));
		sd.cart_num--;
	}
	if(!type)
	{
		clif_cart_delitem(sd,inx,amount);
		clif_updatestatus(sd,SP_CARTINFO);
	}
	return 0;
}

/*==========================================
 * カ?トへアイテム移動
 *------------------------------------------
 */
int pc_putitemtocart(map_session_data &sd,unsigned short idx, size_t amount)
{
	if( idx >= MAX_INVENTORY ) return 0;

	if( !itemdb_isdropable(sd.status.inventory[idx].nameid, sd.isGM()) )
		return 1;
	if( sd.status.inventory[idx].nameid==0 || sd.status.inventory[idx].amount<amount || sd.vender_id)
		return 1;
	if( pc_cart_additem(sd,sd.status.inventory[idx],amount) == 0 )
		return pc_delitem(sd,idx,amount,0);

	return 1;
}

/*==========================================
 * カ?ト?のアイテム?確認(個?の差分を返す)
 *------------------------------------------
 */
int pc_cartitem_amount(map_session_data &sd,unsigned short idx, size_t amount)
{
	struct item *item_data;
	if( idx >= MAX_CART ) return -1;
	nullpo_retr(-1, item_data=&sd.status.cart[idx]);

	if( item_data->nameid==0 || !item_data->amount)
		return -1;
	return item_data->amount-amount;
}
/*==========================================
 * カ?トからアイテム移動
 *------------------------------------------
 */

int pc_getitemfromcart(map_session_data &sd,unsigned short idx, size_t amount)
{
	struct item *item_data;
	int flag;

	if( idx >= MAX_CART ) return 0;
	nullpo_retr(0, item_data=&sd.status.cart[idx]);

	if( item_data->nameid==0 || item_data->amount<amount || sd.vender_id )
		return 1;
	if((flag = pc_additem(sd,*item_data,amount)) == 0)
		return pc_cart_delitem(sd,idx,amount,0);

	clif_additem(sd,0,0,flag);
	return 1;
}

/*==========================================
 * アイテム鑑定
 *------------------------------------------
 */
int pc_item_identify(map_session_data &sd, unsigned short idx)
{	// Celest
	int flag=1;
	if( sd.skillid == BS_REPAIRWEAPON )
		return pc_item_repair (sd, idx);
	else if( sd.skillid == WS_WEAPONREFINE )
		return pc_item_refine (sd, idx);
	else if(idx < MAX_INVENTORY)
	{
		if(sd.status.inventory[idx].nameid > 0 && sd.status.inventory[idx].identify == 0 )
		{
			sd.status.inventory[idx].identify=1;
			flag=0;
		}
	}
	clif_item_identified(sd,idx,flag);
	return !flag;
}

/*==========================================
 * Weapon Repair [Celest]
 *------------------------------------------
 */
int pc_item_repair(map_session_data &sd, unsigned short idx)
{
	if(sd.state.produce_flag==1 && idx!=0xFFFF)
	{
		static int materials[5] = { 0, 1002, 998, 999, 756 };
		int flag=1, material;
		unsigned short nameid = 0;

		sd.state.produce_flag = 0;
		if(idx < MAX_INVENTORY)
		{
			struct item &item = sd.status.inventory[idx];
			nameid = item.nameid;
			if(item.nameid > 0 && item.attribute == 1 )
			{
				if (itemdb_type(item.nameid)==4)
					material = materials[itemdb_wlv(item.nameid)];
				else
					material = materials[3];

				if(pc_search_inventory(sd, material) < 0 ) { //fixed by Lupus (item pos can be = 0!)
					sd.skill_failed(sd.skillid);
					return 0;
				}
				flag=0;
				item.attribute=0;
				pc_delitem(sd, pc_search_inventory(sd, material), 1, 0);
				clif_equiplist(sd);
				clif_displaymessage(sd.fd,"Item has been repaired.");
			}
		}
		return clif_repaireffect(sd,nameid,flag);
	}
	return 0;
}

/*==========================================
 * Weapon Refining [Celest]
 *------------------------------------------
 */
int pc_item_refine(map_session_data &sd, unsigned short idx)
{
	int flag = 1, i = 0, ep = 0, per;
	int material[5] = { 0, 1010, 1011, 984, 984 };

	if(idx < MAX_INVENTORY && sd.state.produce_flag)
	{
		struct item &item		= sd.status.inventory[idx];
		struct item_data *ditem	= sd.inventory_data[idx];
	
		sd.state.produce_flag = 0;

		if(item.nameid > 0 && ditem && ditem->type == 4)
		{
			if (item.refine >= sd.skilllv ||
				item.refine >= MAX_REFINE ||		// if it's no longer refineable
				ditem->flag.no_refine ||	// if the item isn't refinable
				(i = pc_search_inventory(sd, material [ditem->wlv])) < 0 ) { //fixed by Lupus (item pos can be = 0!)
				sd.skill_failed(sd.skillid);
				return 0;
			}
			per = percentrefinery[ditem->wlv][item.refine];
			//per *= (75 + sd.status.job_level/2)/100;
			per += (sd.status.job_level-50)/2; //Updated per the new kro descriptions. [Skotlex]

			if (per > rand() % 100) {
				flag = 0;
				item.refine++;
				pc_delitem(sd, i, 1, 0);
				if(item.equip)
				{
					ep = item.equip;
					pc_unequipitem(sd,idx,3);
				}
				clif_refine(sd.fd,sd,0,idx,item.refine);
				clif_delitem(sd,idx,1);
				clif_additem(sd,idx,1,0);
				if (ep)
					pc_equipitem(sd,idx,ep);
				clif_misceffect(sd,3);
				if( item.refine == MAX_REFINE && item.card[0] == 0x00ff &&
					basics::MakeDWord(item.card[2],item.card[3]) == sd.status.char_id &&
					ditem->wlv>0 && ditem->wlv<4 )
				{	// Fame point system [DracoRPG]
					static const ushort fame_points[4] = {0,1,25,1000};
					chrif_updatefame(sd, FAME_SMITH, fame_points[ditem->wlv]);
					// Success to refine to +10 a lv1 weapon you forged = +1 fame point
					// Success to refine to +10 a lv2 weapon you forged = +25 fame point
					// Success to refine to +10 a lv3 weapon you forged = +1000 fame point
				}
			}
			else
			{
				pc_delitem(sd, i, 1, 0);
				item.refine = 0;
				if(item.equip)
					pc_unequipitem(sd,idx,3);
				clif_refine(sd.fd,sd,1,idx,item.refine);
				pc_delitem(sd,idx,1,0);
				clif_misceffect(sd,2);
				clif_emotion(sd, 23);
			}
		}
	}

	return !flag;
}

/*==========================================
 * スティル品公開
 *------------------------------------------
 */
/*
int pc_show_steal(block_list &bl,va_list &ap)
{
	map_session_data *sd;
	int itemid;
	int type;

	struct item_data *item=NULL;
	char output[100];

	nullpo_retr(0, ap);
	sd=va_arg(ap,map_session_data*);
	nullpo_retr(0, sd);

	itemid=va_arg(ap,int);
	type=va_arg(ap,int);

	if(!type){
		if((item=itemdb_exists(itemid))==NULL)
			sprintf(output,"%s stole an Unknown_Item.",sd->status.name);
		else
			sprintf(output,"%s stole %s.",sd->status.name,item->jname);
		clif_displaymessage( ((map_session_data *)&bl)->fd, output);
	}else{
		sprintf(output,"%s has not stolen the item because of being  overweight.",sd->status.name);
		clif_displaymessage( ((map_session_data *)&bl)->fd, output);
	}

	return 0;
}
*/
class CPcShowSteal : public CMapProcessor
{
	map_session_data& sd;
	int itemid;
	int type;
public:
	CPcShowSteal(map_session_data& s, int iid, int ty)
		: sd(s), itemid(iid), type(ty)
	{}
	~CPcShowSteal()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data* ssd = bl.get_sd();
		if( ssd && session_isActive(ssd->fd) )
		{
			char output[100];
			if(!type)
			{
				struct item_data *item=itemdb_exists(itemid);
				if(item==NULL)
					snprintf(output, sizeof(output), "%s stole an Unknown_Item.", sd.status.name);
				else
					snprintf(output, sizeof(output), "%s stole %s.", sd.status.name,item->jname);
			}
			else
			{
				snprintf(output, sizeof(output), "%s has not stolen the item because of being overweight.",sd.status.name);
			}
			clif_displaymessage(ssd->fd, output);
		}
		return 0;
	}
};
/*==========================================
 *
 *------------------------------------------
 */
//** pc.c: Small Steal Item fix by fritz
int pc_steal_item(map_session_data &sd,block_list *bl)
{
	int flag=0;
	struct mob_data *md;
	if( bl && (md=bl->get_md()) &&
		!maps[md->block_list::m].flag.nomobloot &&			// check noloot map flag [Lorky]
		!md->state.steal_flag && 
		mob_db[md->class_].mexp <= 0 && 
		!(mob_db[md->class_].mode&0x20) &&
		!md->state.steal_flag &&
		!(md->has_status(SC_STONE) || md->has_status(SC_FREEZE)) &&
		md->cache &&								// prevent stealing from summoned creatures. [Skotlex]
		!(md->class_>=1324 && md->class_<1364) )	// prevent stealing from treasure boxes [Valaris]
	{
		unsigned short itemid;
		int skill;
		size_t i;

		skill = config.skill_steal_type == 1
			? (sd.paramc[4] - mob_db[md->class_].dex)/2 + sd.skill_check(TF_STEAL)*6 + 10
			: sd.paramc[4] - mob_db[md->class_].dex + sd.skill_check(TF_STEAL)*3 + 10;

		if(0 < skill)
		{
			// count the items
			i=0; 
			while( mob_db[md->class_].dropitem[i].nameid && i<sizeof(mob_db[md->class_].dropitem)/sizeof(mob_db[md->class_].dropitem[0]))
				i++;
			// choose a random item
			if( i > 0 &&
				(itemid = mob_db[md->class_].dropitem[rand()%i].nameid) > 0 &&
				(itemdb_type(itemid) != 6 || sd.skill_check(TF_STEAL) > 5) &&
				rand()%10000 < ((mob_db[md->class_].dropitem[i].p * skill)/100 + sd.add_steal_rate) )//fixed rate. From Freya [Lupus]
			{
				struct item tmp_item(itemid);
				tmp_item.amount = 1;
				tmp_item.identify = !itemdb_isEquipment(itemid);
				flag = pc_additem(sd,tmp_item,1);
				if(config.show_steal_in_same_party)
					block_list::foreachpartymemberonmap( CPcShowSteal(sd,tmp_item.nameid,flag), sd, false);
				if(flag)
					clif_additem(sd,0,0,flag);
				md->state.steal_flag = 1;
			}
		}
	}
	return flag;
}

/*==========================================
 *
 *------------------------------------------
 */
int pc_steal_coin(map_session_data &sd,block_list *bl)
{
	struct mob_data *md;
	if( bl && (md=bl->get_md()) &&
		!md->state.steal_coin_flag )
	{
		int rate,skill;
		if( md->has_status(SC_STONE) || md->has_status(SC_FREEZE) )
			return 0;
		skill = sd.skill_check(RG_STEALCOIN)*10;
		rate = skill + (sd.status.base_level - mob_db[md->class_].lv)*3 + sd.paramc[4]*2 + sd.paramc[5]*2;
		if(rand()%1000 < rate) {
			pc_getzeny(sd,mob_db[md->class_].lv*10 + rand()%100);
			md->state.steal_coin_flag = 1;
			return 1;
		}
	}
	return 0;
}
/*==========================================
 * PCの位置設定
 *------------------------------------------
 */
bool pc_setpos(map_session_data &sd, const char *mapname_org, unsigned short x, unsigned short y, int clrtype)
{
	char mapname[24], *ip;
	int m=0;
	size_t i;

	nullpo_retr(false, mapname_org);

	// チャットから出る
	sd.leavechat();

	// 取引を中断する
	if(sd.trade_partner)
		trade_tradecancel(sd);

	// 倉庫を開いてるなら保存する
	if(sd.state.storage_flag)
		storage_guild_storage_quit(sd,0);
	else
		storage_storage_quit(sd);

	// パーティ勧誘を拒否する
	if(sd.party_invite>0)
		party_reply_invite(sd,sd.party_invite_account,0);

	// ギルド勧誘を拒否する
	if(sd.guild_invite>0)
		guild_reply_invite(sd,sd.guild_invite,0);

	// ギルド同盟勧誘を拒否する
	if(sd.guild_alliance>0)
		guild_reply_reqalliance(sd,sd.guild_alliance_account,0);

	skill_castcancel(&sd,0);// 詠唱中断
	sd.stop_walking(0);		// 歩行中断
	sd.stop_attack();			// 攻撃中断

	// Delete timer before the player moved to hise repawn point
	if (sd.pvp_timer != -1 && !config.pk_mode) {
		delete_timer(sd.pvp_timer, pc_calc_pvprank_timer);
		sd.pvp_timer = -1;
	}

	for(i=0; i<5; ++i)
	{
		if(sd.dev.val1[i])
		{
			map_session_data *tsd = map_session_data::from_blid(sd.dev.val1[i]);
			skill_devotion_end(&sd,tsd,i);
		}
	}

	if( sd.is_sitting() )
	{
		sd.set_stand();
		skill_gangsterparadise(&sd,0);
	}

	if( sd.has_status(SC_TRICKDEAD) )
		status_change_end(&sd, SC_TRICKDEAD, -1);
	if( sd.has_status(SC_BLADESTOP) )
		status_change_end(&sd,SC_BLADESTOP,-1);
	if( sd.has_status(SC_DANCING) )
		skill_stop_dancing(&sd,0);
	if( sd.has_status(SC_BASILICA) )
	{
		struct skill_unit_group *sg = (struct skill_unit_group *)sd.get_statusvalue4(SC_BASILICA).pointer();
		if (sg && sg->src_id == sd.block_list::id)
				skill_delunitgroup (*sg);
		status_change_end(&sd,SC_BASILICA,-1);
	}
	if( sd.has_status(SC_DEVOTION) )
		status_change_end(&sd,SC_DEVOTION,-1);

	if(sd.status.option&2)
		status_change_end(&sd, SC_HIDING, -1);
	if(sd.is_cloaking())
		status_change_end(&sd, SC_CLOAKING, -1);
	if(sd.is_chasewalk())
		status_change_end(&sd, SC_CHASEWALK, -1);

	if(sd.status.option&2)
		status_change_end(&sd, SC_HIDING, -1);
	if(sd.status.option&4)
		status_change_end(&sd, SC_CLOAKING, -1);
	if(sd.status.option&16384)
		status_change_end(&sd, SC_CHASEWALK, -1);

	if(sd.status.pet_id > 0 && sd.pd && sd.pd->pet.intimate > 0) {
		sd.pd->stop_attack();
		sd.pd->set_idle();
	}


	safestrcpy(mapname, sizeof(mapname), mapname_org);
	ip = strchr(mapname, '.');
	if(ip) *ip=0;

	m=maps.index_of(mapname);

	if(m<0)
	{
		basics::ipset mapset;
		if( maps.mapname2ipport(mapname,mapset) )
		{	// is foreign map
			if(sd.status.pet_id > 0 && sd.pd)
			{
				if(sd.pd->block_list::m != m && sd.pd->pet.intimate <= 0)
				{
					intif_delete_petdata(sd.status.pet_id);
					sd.pd->freeblock();
					sd.status.pet_id = 0;
					sd.pd = NULL;
					if(config.pet_status_support)
						status_calc_pc(sd,2);
				}
				else if(sd.pd->pet.intimate > 0)
				{
					sd.pd->stop_attack();
					sd.pd->set_idle();
					clif_clearchar_area(*sd.pd,clrtype);
					sd.pd->delblock();
				}
			}

			if(sd.hd)
			{
				sd.hd->stop_attack();
				sd.hd->set_idle();
				clif_clearchar_area(*sd.hd,clrtype);
				sd.hd->delblock();
			}
			skill_unit_move(sd,gettick(),0);
			skill_gangsterparadise(&sd,0);
			
			party_send_logout(sd);					// パーティのログアウトメッセージ送信
			guild_send_memberinfoshort(sd,0);		// ギルドのログアウトメッセージ送信
			status_change_clear(&sd,1);	// ステータス異常を解除する
			skill_stop_dancing(&sd,1);			// ダンス/演奏中断
			pc_cleareventtimer(sd);					// イベントタイマを破棄する
			pc_delspiritball(sd,sd.spiritball,1);	// 気功削除
			skill_unit_move(sd,gettick(),0);	// スキルユニットから離脱
			skill_cleartimerskill(&sd);			// タイマースキルクリア
			skill_clear_unitgroup(&sd);			// スキルユニットグループの削除

			memcpy(sd.mapname,mapname,24);
			sd.block_list::x=x;
			sd.block_list::y=y;

			sd.state.waitingdisconnect=1;
			pc_clean_skilltree(sd);
			pc_makesavestatus(sd);
			if(sd.status.pet_id > 0 && sd.pd)
				intif_save_petdata(sd.status.account_id,sd.pd->pet);
			chrif_save(sd);
			storage_storage_save(sd);
			storage_delete(sd.status.account_id);

			clif_clearchar_area(sd,clrtype);
			sd.delblock();

			chrif_changemapserver(sd, mapname, x, y, mapset);
			return true;
		}
		return false;
	}

	if(x >= maps[m].xs || y >= maps[m].ys)
		x=y=0;
	if((x==0 && y==0) || !maps[m].is_passable(x,y) )
	{
		if(x||y)
		{
			if(config.error_log)
				ShowMessage("stacked (%d,%d)\n",x,y);
		}
		do {
			x=rand()%(maps[m].xs-2)+1;
			y=rand()%(maps[m].ys-2)+1;
		} while( !maps[m].is_passable(x,y) );
	}

	if(m == sd.block_list::m)
	{	// 同じマップなのでダンスユニット引き継ぎ
		sd.walktarget.x = x;
		sd.walktarget.y = y;
		skill_stop_dancing(&sd, 2); //移動先にユニットを移動するかどうかの判断もする
	}
	else
	{	// 違うマップなのでダンスユニット削除
		skill_stop_dancing(&sd, 1);
	}
		
	if( sd.is_on_map() )
	{
		skill_gangsterparadise(&sd,0);

		if(sd.status.pet_id > 0 && sd.pd)
		{
			if(sd.pd->block_list::m != m && sd.pd->pet.intimate <= 0)
			{
				intif_delete_petdata(sd.status.pet_id);
				sd.pd->freeblock();
				sd.status.pet_id = 0;
				sd.pd = NULL;
				if(config.pet_status_support)
					status_calc_pc(sd,2);
				pc_clean_skilltree(sd);
				pc_makesavestatus(sd);
				chrif_save(sd);
				storage_storage_save(sd);
			}
			else if(sd.pd->pet.intimate > 0)
			{
				sd.pd->stop_attack();
				sd.pd->set_idle();
				clif_clearchar_area(*sd.pd,clrtype);
				sd.pd->delblock();
			}
		}
		if(sd.hd)
		{
			sd.hd->stop_attack();
			sd.hd->set_idle();
			clif_clearchar_area(*sd.hd,clrtype);
			sd.hd->delblock();
		}

		clif_clearchar_area(sd,clrtype);
		sd.delblock();

		clif_changemap(sd,maps[m].mapname,x,y); // [MouseJstr]
		clif_changed_dir(sd);
	}
	
	if( strcmp(sd.mapname,mapname)!=0 )
		party_send_dot_remove(sd);

	memcpy(sd.mapname,mapname,24);
	sd.block_list::m = m;
	sd.block_list::x = sd.walktarget.x = x;
	sd.block_list::y = sd.walktarget.y = y;

	if(sd.status.pet_id > 0 && sd.pd && sd.pd->pet.intimate > 0)
	{
		sd.pd->block_list::m = m;
		sd.pd->block_list::x = sd.pd->walktarget.x = x;
		sd.pd->block_list::y = sd.pd->walktarget.y = y;
		sd.pd->random_walktarget(sd);
		sd.pd->block_list::x = sd.pd->walktarget.x;
		sd.pd->block_list::y = sd.pd->walktarget.y;
		sd.pd->dir  = sd.dir;
	}
	if(sd.hd)
	{
		sd.hd->block_list::m = m;
		sd.hd->block_list::x = sd.hd->walktarget.x = x;
		sd.hd->block_list::y = sd.hd->walktarget.y = y;
		sd.hd->random_walktarget(sd);
		sd.hd->block_list::x = sd.hd->walktarget.x;
		sd.hd->block_list::y = sd.hd->walktarget.y;
	}
	// warp is closing a text window opend by script
	sd.ScriptEngine.clearMessage();

//	sd.addblock();	/// ブロック登?とspawnは
//	clif_spawnpc(&sd);

	return true;
}

/*==========================================
 * PCのランダムワ?プ
 *------------------------------------------
 */
int pc_randomwarp(map_session_data &sd, int type)
{
	unsigned short x,y,i=0;
	int m;
	m=sd.block_list::m;

	if (maps[sd.block_list::m].flag.noteleport)	// テレポ?ト禁止
		return 0;

	do{
		x=rand()%(maps[m].xs-2)+1;
		y=rand()%(maps[m].ys-2)+1;
	}while( (!maps[m].is_passable(x,y) || maps[m].is_npc(x,y)) && (i++)<1000 );

	if (i < 1000)
		pc_setpos(sd,maps[m].mapname,x,y,type);
	return 0;
}

/*==========================================
 * 現在位置のメモ
 *------------------------------------------
 */
int pc_memo(map_session_data &sd, int i)
{
	int skill;
	int j;

	if (maps[sd.block_list::m].flag.nomemo || (maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()))
	{
		clif_skill_teleportmessage(sd, 1);
		return 0;
	}
	else if ( (skill = sd.skill_check( AL_WARP)) < 1)
	{
		clif_skill_memo(sd,2);
	}
	else if (skill < 2 || i < -1 || i > 2)
	{
		clif_skill_memo(sd, 1);

	}
	else
	{
		for(j=0 ; j<MAX_PORTAL_MEMO; ++j)
		{
			if(strcmp(sd.status.memo_point[j].mapname, maps[sd.block_list::m].mapname) == 0)
			{
				i = j;
				break;
			}
		}
		if(i == -1)
		{
			for(i=skill-3; i>0;)
			{
				 --i;
				sd.status.memo_point[i+1] = sd.status.memo_point[i];
			}
			i=0;
		}
		if(i>=0 && i<MAX_PORTAL_MEMO)
		{
			memcpy(sd.status.memo_point[i].mapname, maps[sd.block_list::m].mapname, 24);
			sd.status.memo_point[i].x = sd.block_list::x;
			sd.status.memo_point[i].y = sd.block_list::y;
		}
		clif_skill_memo(sd, 0);
	}
	return 1;
}






/*==========================================
 * 武器?更によるスキルの??チェック
 * 引?：
 *   map_session_data *sd	セッションデ?タ
 *   unsigned short nameid			?備品ID
 * 返り値：
 *   true		?更なし
 *   false		スキルを解除
 *------------------------------------------
 */
bool pc_checkallowskill(map_session_data &sd)
{
	bool ret = true;

	if( sd.has_status(SC_TWOHANDQUICKEN) && !(skill_get_weapontype(KN_TWOHANDQUICKEN)&(1<<sd.status.weapon)) ) {	// 2HQ
		status_change_end(&sd,SC_TWOHANDQUICKEN,-1);	// 2HQを解除
		ret=false;
	}
	if( sd.has_status(SC_AURABLADE)      && !(skill_get_weapontype(LK_AURABLADE)&(1<<sd.status.weapon)) ) {	// オ?ラブレ?ド 
		status_change_end(&sd,SC_AURABLADE,-1);	// オ-ラブレ-ドを解除 
		ret=false;
	}
	if( sd.has_status(SC_PARRYING)       && !(skill_get_weapontype(LK_PARRYING)&(1<<sd.status.weapon)) ) {	// パリイング 
		status_change_end(&sd,SC_PARRYING,-1);	// パリイングを解除 
		ret=false;
	}
	if( sd.has_status(SC_CONCENTRATION)  && !(skill_get_weapontype(LK_CONCENTRATION)&(1<<sd.status.weapon)) ) {	// コンセントレ?ション 
		status_change_end(&sd,SC_CONCENTRATION,-1);	// コンセントレ-ションを解除 
		ret=false;
	}
	if( sd.has_status(SC_SPEARSQUICKEN)  && !(skill_get_weapontype(CR_SPEARQUICKEN)&(1<<sd.status.weapon)) ){	// スピアクィッケン
		status_change_end(&sd,SC_SPEARSQUICKEN,-1);	// スピアクイッケンを解除
		ret=false;
	}
	if( sd.has_status(SC_ADRENALINE)     && !(skill_get_weapontype(BS_ADRENALINE)&(1<<sd.status.weapon)) ){	// アドレナリンラッシュ
		status_change_end(&sd,SC_ADRENALINE,-1);	// アドレナリンラッシュを解除
		ret=false;
	}

	if(sd.status.shield <= 0) {
		if(sd.has_status(SC_AUTOGUARD)){	// オ-トガ-ド
			status_change_end(&sd,SC_AUTOGUARD,-1);
			ret=false;
		}
		if(sd.has_status(SC_DEFENDER)){	// ディフェンダ?
			status_change_end(&sd,SC_DEFENDER,-1);
			ret=false;
		}
		if(sd.has_status(SC_REFLECTSHIELD)){ //リフレクトシ-ルド
			status_change_end(&sd,SC_REFLECTSHIELD,-1);
			ret=false;
		}
	}
	return ret;
}

/*==========================================
 * ? 備品のチェック
 *------------------------------------------
 */
unsigned short pc_checkequip(map_session_data &sd, unsigned short pos)
{
	size_t i;
	for(i=0;i<MAX_EQUIP;++i)
	{
		if(pos & equip_pos[i])
			return sd.equip_index[i];
	}
	return 0xFFFF;
}

/*==========================================
 * ?生職や養子職の元の職業を返す
 *------------------------------------------
 */
struct pc_base_job pc_calc_base_job(int b_class)
{
	struct pc_base_job bj;
	if(b_class < JOB_NOVICE_HIGH || (b_class >= JOB_TAEKWON && b_class <= JOB_SOUL_LINKER))
	{
		if (b_class == JOB_KNIGHT2)
			bj.job = JOB_KNIGHT;
		else if (b_class == JOB_CRUSADER2)
			bj.job = JOB_CRUSADER;
		else
			bj.job = b_class;
		bj.upper = 0;
	}
	else if(b_class >= JOB_NOVICE_HIGH && b_class <= JOB_PALADIN2)
	{	//High Jobs
		if (b_class == JOB_LORD_KNIGHT2)
			bj.job = JOB_KNIGHT;
		else if (b_class == JOB_PALADIN2)
			bj.job = JOB_CRUSADER;
		else
			bj.job = b_class - JOB_NOVICE_HIGH;
		bj.upper = 1;
	}
	else
	{	//Baby Classes
		if (b_class == JOB_SUPER_BABY)
			bj.job = JOB_SUPER_NOVICE;
		else if (b_class == JOB_BABY_KNIGHT2)
			bj.job = JOB_KNIGHT;
		else if (b_class == JOB_BABY_CRUSADER2)
			bj.job = JOB_CRUSADER;
		else
			bj.job = b_class - JOB_BABY;
		bj.upper = 2;
	}

	if(bj.job == JOB_NOVICE)
		bj.type = 0;
	else if(bj.job <= JOB_THIEF || bj.job == JOB_TAEKWON)
		bj.type = 1;
	else
		bj.type = 2;

	return bj;
}

/*==========================================
 * For quick calculating [Celest]
 *------------------------------------------
 */
int pc_calc_base_job2 (int b_class)
{
	if(b_class < JOB_NOVICE_HIGH || (b_class >= JOB_TAEKWON && b_class <= JOB_SOUL_LINKER))
	{
		if (b_class == JOB_KNIGHT2)
			return JOB_KNIGHT;
		if (b_class == JOB_CRUSADER2)
			return JOB_CRUSADER;
		return b_class;
	}
	else if(b_class >= JOB_NOVICE_HIGH && b_class < JOB_BABY)
	{
		if (b_class == JOB_LORD_KNIGHT2)
			return JOB_KNIGHT;
		if (b_class == JOB_PALADIN2)
			return JOB_CRUSADER;
		return b_class - JOB_NOVICE_HIGH;
	}
	else
	{	//Baby Classes
		if (b_class == JOB_SUPER_BABY)
			return JOB_SUPER_NOVICE;
		if (b_class == JOB_BABY_KNIGHT2)
			return JOB_KNIGHT;
		if (b_class == JOB_BABY_CRUSADER2)
			return JOB_CRUSADER;
		return b_class - JOB_BABY;
	}
}
int pc_calc_upper(int b_class)
{
	if(b_class < JOB_NOVICE_HIGH || (b_class >= JOB_TAEKWON && b_class <= JOB_SOUL_LINKER))
		return 0;
	else if(b_class >= JOB_NOVICE_HIGH && b_class < JOB_BABY)
		return 1;
	return 2;
}




int pc_follow_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd=map_session_data::from_blid(id);
	if(sd == NULL || sd->followtimer != tid)
		return 0;

	sd->followtimer=-1;

	block_list *bl = block_list::from_blid(sd->followtarget);
	if( bl && bl->is_on_map() && sd->is_on_map() && !sd->is_dead() )
	{

		if( sd->skilltimer == -1 && sd->attacktimer == -1 && !sd->is_walking() && sd->block_list::m == bl->m)
		{
			if( sd->can_reach(bl->x,bl->y) )
			{
				if( distance(*sd, *bl) > 5 )
					sd->walktoxy(bl->x,bl->y);
			}
			else
				pc_setpos(*sd, maps[bl->m].mapname, bl->x, bl->y, 3);
		}
		sd->followtimer=add_timer(tick + sd->aspd,pc_follow_timer,sd->block_list::id,0);
	}
	return 0;
}

int pc_stop_following (map_session_data &sd)
{
	if (sd.followtimer != -1)
	{
		delete_timer(sd.followtimer,pc_follow_timer);
		sd.followtimer = -1;
	}
	sd.followtarget = 0xFFFFFFFF;
	return 0;
}

int pc_follow(map_session_data &sd, uint32 target_id)
{
	block_list *bl=block_list::from_blid(target_id);
	if(bl==NULL)
		return 1;

	sd.followtarget=target_id;
	if(sd.followtimer != -1)
	{
		delete_timer(sd.followtimer,pc_follow_timer);
		sd.followtimer = -1;
	}
	pc_follow_timer(-1,gettick(),sd.block_list::id,0);

	return 0;
}

int pc_checkbaselevelup(map_session_data &sd)
{
	uint32 next = pc_nextbaseexp(sd);

	if(sd.status.base_exp >= next && next > 0){
		struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);

		// base側レベルアップ?理
		sd.status.base_exp -= next;

		sd.status.base_level ++;
		if(config.pet_lv_rate && sd.pd)	//<Skotlex> update pet's level
			status_calc_pet(sd,false);
		sd.status.status_point += (sd.status.base_level+14) / 5 ;
		clif_updatestatus(sd,SP_STATUSPOINT);
		clif_updatestatus(sd,SP_BASELEVEL);
		clif_updatestatus(sd,SP_NEXTBASEEXP);
		status_calc_pc(sd,0);
		sd.heal(sd.status.max_hp,sd.status.max_sp);

		//スパノビはキリエ、イムポ、マニピ、グロ、サフラLv1がかかる
		if(s_class.job == 23){
			status_change_start(&sd,(status_t)SkillStatusChangeTable[PR_KYRIE],1,0,0,0,skill_get_time(PR_KYRIE,1),0 );
			status_change_start(&sd,(status_t)SkillStatusChangeTable[PR_IMPOSITIO],1,0,0,0,skill_get_time(PR_IMPOSITIO,1),0 );
			status_change_start(&sd,(status_t)SkillStatusChangeTable[PR_MAGNIFICAT],1,0,0,0,skill_get_time(PR_MAGNIFICAT,1),0 );
			status_change_start(&sd,(status_t)SkillStatusChangeTable[PR_GLORIA],1,0,0,0,skill_get_time(PR_GLORIA,1),0 );
			status_change_start(&sd,(status_t)SkillStatusChangeTable[PR_SUFFRAGIUM],1,0,0,0,skill_get_time(PR_SUFFRAGIUM,1),0 );
		}

		clif_misceffect(sd,0);
		//レベルアップしたのでパ?ティ?情報を更新する
		//(公平範?チェック)
		party_send_movemap(sd);
		npc_data::event("PCBaseUpEvent", "PCBaseUpEvent", sd);
		return 1;
	}

	return 0;
}

int pc_checkjoblevelup(map_session_data &sd)
{
	uint32 next = pc_nextjobexp(sd);

	if(sd.status.job_exp >= next && next > 0){
		// job側レベルアップ?理
		sd.status.job_exp -= next;
		sd.status.job_level ++;
		clif_updatestatus(sd,SP_JOBLEVEL);
		clif_updatestatus(sd,SP_NEXTJOBEXP);
		sd.status.skill_point ++;
		clif_updatestatus(sd,SP_SKILLPOINT);
		status_calc_pc(sd,0);

		clif_misceffect(sd,1);
		return 1;
	}

	return 0;
}

/*==========================================
 * ??値取得
 *------------------------------------------
 */
int pc_gainexp(map_session_data &sd,uint32 base_exp,uint32 job_exp)
{
	char output[256];
	uint32 nextb=0, nextj=0;

	if( !sd.is_on_map() || sd.is_dead() )
		return 0;

	if((config.pvp_exp == 0) && maps[sd.block_list::m].flag.pvp)  // [MouseJstr]
		return 0; // no exp on pvp maps

	if( sd.has_status(SC_RICHMANKIM) )
	{
		base_exp += base_exp*(25 + sd.get_statusvalue1(SC_RICHMANKIM).integer()*11)/100;
		job_exp += job_exp*(25 + sd.get_statusvalue1(SC_RICHMANKIM).integer()*11)/100;
	}

	if(sd.status.guild_id>0)
	{	// ギルドに上納
		uint32 payexp=guild_payexp(sd,base_exp);
		if(base_exp > payexp)
			base_exp-=payexp;
		else
			base_exp = 0;
	}

	if(!config.multi_level_up && pc_nextbaseafter(sd) && sd.status.base_exp+base_exp >= pc_nextbaseafter(sd))
	{
		if( pc_nextbaseafter(sd) > sd.status.base_exp )
			base_exp = pc_nextbaseafter(sd) - sd.status.base_exp;
		else
			base_exp = 0;
	}
	nextb = pc_nextbaseexp(sd);
	nextj = pc_nextjobexp(sd);

	sd.status.base_exp += base_exp;
	//TODO: overflow check

	while(pc_checkbaselevelup(sd)) ;

	clif_updatestatus(sd,SP_BASEEXP);
	if(!config.multi_level_up && pc_nextjobafter(sd) && sd.status.job_exp+job_exp >= pc_nextjobafter(sd))
	{
		if( pc_nextjobafter(sd) > sd.status.job_exp )
			job_exp = pc_nextjobafter(sd) - sd.status.job_exp;
		else
			job_exp = 0;
	}

	sd.status.job_exp += job_exp;

	while(pc_checkjoblevelup(sd)) {}

	clif_updatestatus(sd,SP_JOBEXP);

	if(config.disp_experience && !sd.state.noexp)
	{
		double nextbp=0, nextjp=0;
		if(nextb > 0)
			nextbp = 100. * base_exp / nextb;
		if(nextj > 0)
			nextjp = 100. * job_exp / nextj;

		snprintf(output,sizeof(output),"Experienced Gained Base: %lu (%.2f%%) Job:%lu (%.2f%%)",(unsigned long)base_exp,nextbp,(unsigned long)job_exp,nextjp);
		clif_disp_onlyself(sd,output);
	}

	return 0;
}

/*==========================================
 * base level側必要??値計算
 *------------------------------------------
 */
uint32 pc_nextbaseexp(map_session_data &sd)
{
	int i;

	if(sd.status.base_level>=MAX_LEVEL || sd.status.base_level<=0)
		return 0;

	if(sd.status.class_==0 || sd.status.class_==4023) i=0; //Novice & Baby Novice [Lupus]
	else if(sd.status.class_<=6 || (sd.status.class_>=4024 && sd.status.class_<=4029)) i=1; //1st Job & Baby 1st Job
	else if(sd.status.class_<=22 || (sd.status.class_>=4030 && sd.status.class_<=4044)) i=2; //2nd Job & Baby 2nd Job
	else if(sd.status.class_==23 || sd.status.class_==4045) i=3; //Super Novice & Super Baby
	else if(sd.status.class_==4001) i=4; //High Novice
	else if(sd.status.class_<=4007) i=5; //High 1st Job
	else i=6; //3rd Job

	return exp_table[i][sd.status.base_level-1];
}

/*==========================================
 * job level側必要??値計算
 *------------------------------------------
 */
uint32 pc_nextjobexp(map_session_data &sd)
{
	size_t i;
	if(sd.status.job_level>=MAX_LEVEL || sd.status.job_level<=0)
		return 0;

	if(sd.status.class_==0 || sd.status.class_==4023) i=7; //Novice & Baby Novice [Lupus]
	else if(sd.status.class_<=6 || (sd.status.class_>=4024 && sd.status.class_<=4029)) i=8; //1st Job & Baby 1st Job
	else if(sd.status.class_<=22 || (sd.status.class_>=4030 && sd.status.class_<=4044)) i=9; //2nd Job & Baby 2nd Job
	else if(sd.status.class_==23 || sd.status.class_==4045) i=10; //Super Novice & Super Baby
	else if(sd.status.class_==4001) i=11; //High Novice
	else if(sd.status.class_<=4007) i=12; //High 1st Job
	else i=13; //3rd Job

	return exp_table[i][sd.status.job_level-1];
}

/*==========================================
 * base level after next [Valaris]
 *------------------------------------------
 */
uint32 pc_nextbaseafter(map_session_data &sd)
{
	int i;

	if(sd.status.base_level>=MAX_LEVEL || sd.status.base_level<=0)
		return 0;

	if(sd.status.class_==0 || sd.status.class_==4023) i=0; //Novice & Baby Novice [Lupus]
	else if(sd.status.class_<=6 || (sd.status.class_>=4024 && sd.status.class_<=4029)) i=1; //1st Job & Baby 1st Job
	else if(sd.status.class_<=22 || (sd.status.class_>=4030 && sd.status.class_<=4044)) i=2; //2nd Job & Baby 2nd Job
	else if(sd.status.class_==23 || sd.status.class_==4045) i=3; //Super Novice & Super Baby
	else if(sd.status.class_==4001) i=4; //High Novice
	else if(sd.status.class_<=4007) i=5; //High 1st Job
	else i=6; //3rd Job

	return exp_table[i][sd.status.base_level];
}

/*==========================================
 * job level after next [Valaris]
 *------------------------------------------
 */
uint32 pc_nextjobafter(map_session_data &sd)
{
	int i;
	if(sd.status.job_level>=MAX_LEVEL || sd.status.job_level<=0)
		return 0;

	if(sd.status.class_==0 || sd.status.class_==4023) i=7; //Novice & Baby Novice [Lupus]
	else if(sd.status.class_<=6 || (sd.status.class_>=4024 && sd.status.class_<=4029)) i=8; //1st Job & Baby 1st Job
	else if(sd.status.class_<=22 || (sd.status.class_>=4030 && sd.status.class_<=4044)) i=9; //2nd Job & Baby 2nd Job
	else if(sd.status.class_==23 || sd.status.class_==4045) i=10; //Super Novice & Super Baby
	else if(sd.status.class_==4001) i=11; //High Novice
	else if(sd.status.class_<=4007) i=12; //High 1st Job
	else i=13; //3rd Job

	return exp_table[i][sd.status.job_level];
}
/*==========================================

 * 必要ステ?タスポイント計算
 *------------------------------------------
 */
unsigned char pc_need_status_point(map_session_data &sd,int type)
{
	size_t val;
	if(type<SP_STR || type>SP_LUK)
		return 0xFF;
	val =
		type==SP_STR ? sd.status.str :
		type==SP_AGI ? sd.status.agi :
		type==SP_VIT ? sd.status.vit :
		type==SP_INT ? sd.status.int_:
		type==SP_DEX ? sd.status.dex : sd.status.luk;

	return (val+9)/10+1;
}

/*==========================================
 * 能力値成長
 *------------------------------------------
 */
int pc_statusup(map_session_data &sd,int type)
{
	int max, need,val = 0;

	max = (pc_calc_upper(sd.status.class_)==2) ? 80 : config.max_parameter;

	need=pc_need_status_point(sd,type);
	if(type<SP_STR || type>SP_LUK || need<0 || need>sd.status.status_point){
		clif_statusupack(sd,type,0,0);
		return 1;
	}
	switch(type){
	case SP_STR:
		if(sd.status.str >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.str;
		break;
	case SP_AGI:
		if(sd.status.agi >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.agi;
		break;
	case SP_VIT:
		if(sd.status.vit >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.vit;
		break;
	case SP_INT:
		if(sd.status.int_ >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.int_;
		break;
	case SP_DEX:
		if(sd.status.dex >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.dex;
		break;
	case SP_LUK:
		if(sd.status.luk >= max) {
			clif_statusupack(sd,type,0,0);
			return 1;
		}
		val= ++sd.status.luk;
		break;
	}
	sd.status.status_point-=need;
	if(need!=pc_need_status_point(sd,type)){
		clif_updatestatus(sd,type-SP_STR+SP_USTR);
	}
	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,type);
	status_calc_pc(sd,0);
	clif_statusupack(sd,type,1,val);

	return 0;
}

/*==========================================
 * 能力値成長
 *------------------------------------------
 */
int pc_statusup2(map_session_data &sd,int type,int val)
{
	if(type<SP_STR || type>SP_LUK)
	{
		clif_statusupack(sd,type,0,0);
		return 1;
	}
	switch(type)
	{
	case SP_STR:
		val += sd.status.str;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.str = val;
		break;
	case SP_AGI:
		val += sd.status.agi;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.agi = val;
		break;
	case SP_VIT:
		val += sd.status.vit;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.vit = val;
		break;
	case SP_INT:
		val += sd.status.int_;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.int_ = val;
		break;
	case SP_DEX:
		val += sd.status.dex;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.dex = val;
		break;
	case SP_LUK:
		val += sd.status.luk;
		if((uint32)val >= config.max_parameter)
			val = config.max_parameter;
		else if(val < 1)
			val = 1;
		sd.status.luk = val;
		break;
	}
	clif_updatestatus(sd,type-SP_STR+SP_USTR);
	clif_updatestatus(sd,type);
	status_calc_pc(sd,0);
	clif_statusupack(sd,type,1,val);

	return 0;
}

/*==========================================
 * スキルポイント割り振り
 *------------------------------------------
 */
int pc_skillup(map_session_data &sd, unsigned short skillid)
{
	if( skillid>=10000 )
	{
		guild_skillup(sd,skillid,0);
		return 0;
	}

	if( sd.status.skill_point>0 &&
		sd.status.skill[skillid].id!=0 &&
		sd.status.skill[skillid].lv < skill_tree_get_max(skillid, sd.status.class_) )
	{
		sd.status.skill[skillid].lv++;
		sd.status.skill_point--;
		status_calc_pc(sd,0);
		clif_skillup(sd,skillid);
		clif_updatestatus(sd,SP_SKILLPOINT);
		clif_skillinfoblock(sd);
	}
	return 0;
}

/*==========================================
 * /allskill
 *------------------------------------------
 */
int pc_allskillup(map_session_data &sd)
{
	size_t i;
	unsigned short id;

	for(i=0;i<MAX_SKILL;++i)
	{
		sd.status.skill[i].id=0;
		if( sd.status.skill[i].flag && sd.status.skill[i].flag != 13)
		{	// cardスキルなら、
			sd.status.skill[i].lv=(sd.status.skill[i].flag==1)?0:sd.status.skill[i].flag-2;	// 本?のlvに
			sd.status.skill[i].flag=0;	// flagは0にしておく
		}
	}

	if( config.gm_allskill > 0 && sd.isGM() >= config.gm_allskill )
	{	// 全てのスキル
		for(i=1;i<158;++i)
			sd.status.skill[i].lv=skill_get_max(i);
		for(i=210;i<291;++i)
			sd.status.skill[i].lv=skill_get_max(i);
		for(i=304;i<331;++i)
			sd.status.skill[i].lv=skill_get_max(i);
		for(i=334;i<338;++i)
			sd.status.skill[i].lv=skill_get_max(i);
		for(i=355;i<473;++i)
			sd.status.skill[i].lv=skill_get_max(i);
		for(i=475;i<492;++i)
			sd.status.skill[i].lv=skill_get_max(i);
	}
	else
	{
		int inf2;
		for(i=0;i<MAX_SKILL&&(id=skill_tree[sd.status.class_][i].id)>0;++i)
		{
			inf2 = skill_get_inf2(id);
			if(sd.status.skill[id].id==0 && (!(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) && !(inf2&INF2_WEDDING_SKILL))
			{
				sd.status.skill[id].id = id;	// celest
				//sd.status.skill[id].lv=skill_get_max(id);
				sd.status.skill[id].lv = skill_tree_get_max(id, sd.status.class_);	// celest
			}
		}
	}
	status_calc_pc(sd,0);

	return 0;
}

/*==========================================
 * /resetlvl
 *------------------------------------------
 */
int pc_resetlvl(map_session_data &sd,int type)
{
	size_t  i;

	for(i=1; i<MAX_SKILL; ++i)
		sd.status.skill[i].lv = 0;

	if(type == 1)
	{
		sd.status.skill_point=0;
		sd.status.base_level=1;
		sd.status.job_level=1;
		sd.status.base_exp = sd.status.base_exp=0;
		sd.status.job_exp = sd.status.job_exp=0;
		if(sd.status.option !=0)
			sd.status.option = 0;

		sd.status.str=1;
		sd.status.agi=1;
		sd.status.vit=1;
		sd.status.int_=1;
		sd.status.dex=1;
		sd.status.luk=1;
		if(sd.status.class_ == 4001)
			sd.status.status_point=100;	// not 88 [celest]
		// give platinum skills upon changing
		pc_skill(sd,142,1,0);
		pc_skill(sd,143,1,0);
	}
	else if(type == 2)
	{
		sd.status.skill_point=0;
		sd.status.base_level=1;
		sd.status.job_level=1;
		sd.status.base_exp=0;
		sd.status.job_exp=0;
	}
	else if(type == 3)
	{
		sd.status.base_level=1;
		sd.status.base_exp=0;
	}
	else if(type == 4)
	{
		sd.status.job_level=1;
		sd.status.job_exp=0;
	}

	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);
	clif_updatestatus(sd,SP_BASELEVEL);
	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_STATUSPOINT);
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	clif_updatestatus(sd,SP_SKILLPOINT);

	clif_updatestatus(sd,SP_USTR);	// Updates needed stat points - Valaris
	clif_updatestatus(sd,SP_UAGI);
	clif_updatestatus(sd,SP_UVIT);
	clif_updatestatus(sd,SP_UINT);
	clif_updatestatus(sd,SP_UDEX);
	clif_updatestatus(sd,SP_ULUK);	// End Addition

	for(i=0;i<MAX_EQUIP;++i)
	{	// unequip items that can't be equipped by base 1 [Valaris]
		if(sd.equip_index[i] < MAX_INVENTORY)
			if(!pc_isequipable(sd,sd.equip_index[i]))
				pc_unequipitem(sd,sd.equip_index[i],2);
	}
	clif_skillinfoblock(sd);
	status_calc_pc(sd,0);
	return 0;
}
/*==========================================
 * /resetstate
 *------------------------------------------
 */
	
//Old bugged equation:
//#define sumsp(a) ((a)*((a-2)/10+2) - 5*((a-2)/10)*((a-2)/10) - 6*((a-2)/10) -2)
//Use new stat-calculating equation [Skotlex]
inline unsigned int sumsp(unsigned int a) { return (((a-1)/10 +2)*(5*((a-1)/10 +1) + (a-1)%10) -10); }

int pc_resetstate(map_session_data &sd)
{
	if (config.use_statpoint_table)
	{	// New statpoint table used here - Dexity
		unsigned short lv;
		// allow it to just read the last entry [celest]
		lv = (sd.status.base_level < MAX_LEVEL) ? sd.status.base_level : MAX_LEVEL - 1;
		
		sd.status.status_point = statp[lv];
		if(sd.status.class_ >= 4001 && sd.status.class_ <= 4024)
			sd.status.status_point+=52;	// extra 52+48=100 stat points
	}
	else
	{
		unsigned int add=0;
		add += sumsp(sd.status.str);
		add += sumsp(sd.status.agi);
		add += sumsp(sd.status.vit);
		add += sumsp(sd.status.int_);
		add += sumsp(sd.status.dex);
		add += sumsp(sd.status.luk);
		sd.status.status_point += add;
	}

	sd.status.str=1;
	sd.status.agi=1;
	sd.status.vit=1;
	sd.status.int_=1;
	sd.status.dex=1;
	sd.status.luk=1;

	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);

	clif_updatestatus(sd,SP_USTR);	// Updates needed stat points - Valaris
	clif_updatestatus(sd,SP_UAGI);
	clif_updatestatus(sd,SP_UVIT);
	clif_updatestatus(sd,SP_UINT);
	clif_updatestatus(sd,SP_UDEX);
	clif_updatestatus(sd,SP_ULUK);	// End Addition
	
	clif_updatestatus(sd,SP_STATUSPOINT);
	status_calc_pc(sd,0);

	return 0;
}

/*==========================================
 * /resetskill
 *------------------------------------------
 */
int pc_resetskill(map_session_data &sd)
{
	size_t i, inf2, skill;
	for (i = 1; i < MAX_SKILL; ++i)
	{
		if ((skill = sd.status.skill[i].lv) > 0)
		{
			inf2 = skill_get_inf2(i);	
			if ((!(inf2&INF2_QUEST_SKILL) || config.quest_skill_learn) &&
				!(inf2&INF2_WEDDING_SKILL) ) //Avoid reseting wedding skills.
			{
				if (!sd.status.skill[i].flag)
					sd.status.skill_point += skill;
				else if (sd.status.skill[i].flag > 2 && sd.status.skill[i].flag != 13)
					sd.status.skill_point += (sd.status.skill[i].flag - 2);
				sd.status.skill[i].lv = 0;
			}
			else if (config.quest_skill_reset && (inf2&INF2_QUEST_SKILL))
			{
				sd.status.skill[i].lv = 0;
				sd.status.skill[i].flag = 0;
			}
		}
		else
		{
			sd.status.skill[i].lv = 0;
		}
	}
	clif_updatestatus(sd,SP_SKILLPOINT);
	clif_skillinfoblock(sd);
	status_calc_pc(sd,0);
	return 0;
}


int pc_respawn(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd = map_session_data::from_blid(id);
	if (sd && sd->is_dead() )
	{	//Auto-respawn [Skotlex]
		sd->set_stand();
		pc_setrestartvalue(*sd,3);
		pc_setpos(*sd,sd->status.save_point.mapname,sd->status.save_point.x,sd->status.save_point.y,0);
	}
	return 0;
}

/*==========================================
 * pcにダメ?ジを?える
 *------------------------------------------
 */
int pc_damage(map_session_data &sd, long damage, block_list *src)
{
	size_t i=0,j=0;
	struct pc_base_job s_class;

	// ?に死んでいたら無?
	if( sd.is_dead() )
		return 0;
	// 座ってたら立ち上がる
	if( sd.is_sitting() )
	{
		sd.set_stand();
		skill_gangsterparadise(&sd,0);
	}
	//?生や養子の場合の元の職業を算出する
	s_class = pc_calc_base_job(sd.status.class_);

	// ? いていたら足を止める
	if( sd.has_status(SC_BERSERK) || sd.state.infinite_endure)
		;	// do nothing
	else if (sd.has_status(SC_ENDURE) && (src != NULL && *src == BL_MOB) && !maps[sd.block_list::m].flag.gvg)
	{
		if( !sd.state.infinite_endure && (--sd.get_statusvalue2(SC_ENDURE).integer()) < 0 )
			status_change_end(&sd, SC_ENDURE, -1);
	}
	else
		sd.stop_walking(3);

	if( sd.has_status(SC_GRAVITATION) && sd.get_statusvalue3(SC_GRAVITATION).integer() == BCT_SELF)
	{
		struct skill_unit_group *sg = (struct skill_unit_group *)sd.get_statusvalue4(SC_GRAVITATION).pointer();
		if (sg)
		{
			skill_delunitgroup(*sg);
			status_change_end(&sd, SC_GRAVITATION, -1);
		}
	}

	// 演奏/ダンスの中?
	if(damage > sd.status.max_hp/4)
		skill_stop_dancing(&sd,0);

	if(sd.status.hp > damage)
		sd.status.hp -= damage;
	else
		sd.status.hp = 0;

	if(sd.status.pet_id > 0 && sd.pd && config.pet_damage_support && src)
		pet_target_check(sd,src,1);

	if( sd.has_status(SC_TRICKDEAD) )
		status_change_end(&sd, SC_TRICKDEAD, -1);
	if(sd.status.option&2)
		status_change_end(&sd, SC_HIDING, -1);
	if(sd.is_cloaking())
		status_change_end(&sd, SC_CLOAKING, -1);
	if(sd.is_chasewalk())
		status_change_end(&sd, SC_CHASEWALK, -1);


	if(sd.status.hp > 0)
	{	// まだ生きているならHP更新
		clif_updatestatus(sd,SP_HP);

		//if(sd.status.hp<sd->status.max_hp/4 && sd.skill_check(SM_AUTOBERSERK)>0 &&
		if( sd.status.hp<sd.status.max_hp/4 && sd.has_status(SC_AUTOBERSERK) &&
			( !sd.has_status(SC_PROVOKE) || sd.get_statusvalue2(SC_PROVOKE).integer()==0 ))
			// オ?トバ?サ?ク?動
			status_change_start(&sd,SC_PROVOKE,10,1,0,0,0,0);

		sd.canlog_tick = gettick();

		if(sd.status.party_id)
		{	// on-the-fly party hp updates [Valaris]
			struct party *p=party_search(sd.status.party_id);
			if(p!=NULL) clif_party_hp(*p,sd);
		}	// end addition [Valaris]
		return damage;
	}
	// else dead

	if(sd.vender_id)
		vending_closevending(sd);

	if(sd.status.pet_id > 0 && sd.pd)
	{
		if(sd.pd->pet.intimate > sd.pd->petDB.die)
			sd.pd->pet.intimate	-= sd.pd->petDB.die;
		else
			sd.pd->pet.intimate = 0;
		clif_send_petdata(sd,1,sd.pd->pet.intimate);
	}

	pc_setglobalreg(sd,"PC_DIE_COUNTER",++sd.die_counter); //死にカウンタ?書き?み

	sd.stop_walking(0);
	skill_castcancel(&sd,0);	// 詠唱の中止
	clif_clearchar_area(sd,1);

	
	if (src && *src == BL_PC)
	{
		map_session_data &ssd = *src->get_sd();
		
		// killerid
		if (sd.state.event_death)
			pc_setglobalreg(sd,"killerrid",(ssd.status.account_id));

		// killevent
		if (ssd.state.event_kill)
			npc_data::event("OnPCKillEvent", script_config.kill_event_name, ssd);

		// pk points
		if( config.pk_mode )
		{	
			// 5 points penalty for beeing killed
			chrif_updatefame(sd, FAME_PK, -5);
			// 1 points reward for beeing winner
			chrif_updatefame(sd, FAME_PK,  1);


			
			// limit karma to +/-100 (is a char anyway)
			// got killed -> bad karma
			if( sd.status.karma >-100 )
				--sd.status.karma;

			if (sd.status.karma > ssd.status.karma)
			{	// If player killed was more good
				if( ssd.status.karma >-100 )
					--ssd.status.karma;
			}
			else if (sd.status.karma < ssd.status.karma)
			{	// If player killed was more evil
				if( ssd.status.karma < 100 )
					++ssd.status.karma;
			}
		}
		// drop a random item when bad Karma
/*		if(sd.status.karma < 0)
		{	
			if(eq_num > 0)
			{
				int n = eq_n[rand()%eq_num];
				if(rand()%10000 < sd->status.karma && sd.skill_check(BS_HILTBINDING) < 1)
				{
					if(sd->status.inventory[n].equip)
						pc_unequipitem(sd,n,0);
					pc_dropitem(sd,n,1);
				}
			}
		}
*/
	}

	if (sd.state.event_death)
		npc_data::event("OnPCDieEvent", script_config.die_event_name, sd);

	if(config.bone_drop==2
		|| (config.bone_drop==1 && maps[sd.block_list::m].flag.pvp)){	// ドクロドロップ
		struct item item_tmp(7420); //PVP Skull item ID
		item_tmp.identify=1;
		item_tmp.card[0]=0x00fe;
		item_tmp.card[1]=0;
		item_tmp.card[2]=basics::GetWord(sd.status.char_id,0);	// キャラID 
		item_tmp.card[3]=basics::GetWord(sd.status.char_id,1);
		flooritem_data::create(item_tmp,1,sd.block_list::m,sd.block_list::x,sd.block_list::y,NULL,NULL,NULL,0);
	}

	// activate Steel body if a super novice dies at 99+% exp [celest]
	if (s_class.job == 23) {
		if ((i=pc_nextbaseexp(sd))<=0)
			i=sd.status.base_exp;
		if((i>0) && (j=sd.status.base_exp*1000/i)>=990 && j<1000)
			sd.state.snovice_flag = 4;
	}

	for(i = 0; i < 5; ++i)
		if (sd.dev.val1[i]){
			map_session_data *devsd = map_session_data::from_blid(sd.dev.val1[i]);
			if (devsd) status_change_end( devsd,SC_DEVOTION,-1);
			sd.dev.val1[i] = sd.dev.val2[i]=0;
		}

	sd.set_dead();
	sd.ScriptEngine.clearAll();

	skill_unit_move(sd,gettick(),0);
	if( sd.has_status(SC_BLADESTOP) )//白刃は事前に解除
		status_change_end(&sd,SC_BLADESTOP,-1);

	status_change_clear(&sd,0);	// ステ?タス異常を解除する
	clif_updatestatus(sd,SP_HP);
	status_calc_pc(sd,0);

	if(config.death_penalty_type>0) { // changed penalty options, added death by player if pk_mode [Valaris]
		if(sd.status.class_ != 0 && !maps[sd.block_list::m].flag.nopenalty && !maps[sd.block_list::m].flag.gvg &&	// only novices will recieve no penalty
			!(sd.has_status(SC_BABY))) {
			if(config.death_penalty_type==1 && config.death_penalty_base > 0)
				sd.status.base_exp -= pc_nextbaseexp(sd)*config.death_penalty_base/10000;
				if(config.pk_mode && src && *src==BL_PC)
				sd.status.base_exp -= pc_nextbaseexp(sd)*config.death_penalty_base/10000;
			else if(config.death_penalty_type==2 && config.death_penalty_base > 0) 
			{
				if( pc_nextbaseexp(sd) > 0 )
					sd.status.base_exp -= (config.death_penalty_base<10000)?sd.status.base_exp*config.death_penalty_base/10000:sd.status.base_exp;
				if(config.pk_mode && src && *src==BL_PC)
					sd.status.base_exp -= (config.death_penalty_base<10000)?sd.status.base_exp*config.death_penalty_base/10000:sd.status.base_exp;
			}
			clif_updatestatus(sd,SP_BASEEXP);

			if(config.death_penalty_type==1 && config.death_penalty_job > 0)
				sd.status.job_exp -= pc_nextjobexp(sd)*config.death_penalty_job/10000;
			if(config.pk_mode && src && *src==BL_PC)
				sd.status.job_exp -= pc_nextjobexp(sd)*config.death_penalty_job/10000;
			else if(config.death_penalty_type==2 && config.death_penalty_job > 0)
			{
				if(pc_nextjobexp(sd) > 0)
					sd.status.job_exp -= (config.death_penalty_job<10000)?sd.status.job_exp*config.death_penalty_job/10000:sd.status.job_exp;
				if(config.pk_mode && src && *src==BL_PC)
					sd.status.job_exp -= (config.death_penalty_job<10000)?sd.status.job_exp*config.death_penalty_job/10000:sd.status.job_exp;
			}
			clif_updatestatus(sd,SP_JOBEXP);
		}
	}
	if(src && *src==BL_MOB)
	{
		struct mob_data *md=(struct mob_data *)src;
		if(md && md->target_id != 0 && md->target_id==sd.block_list::id)
		{	// reset target id when player dies
			md->target_id=0;
			md->randomwalk(200);
		}
		if( config.mobs_level_up && md && !md->is_dead() && 
			md->level < config.max_base_level &&
			(md->class_ < 1285 || md->class_ > 1288) )
		{	// monster level up [Valaris]
			clif_misceffect(*md,0);
			md->level++;
			md->hp += sd.status.max_hp/10;
		}
	}

	//ナイトメアモ?ドアイテムドロップ
	if(maps[sd.block_list::m].flag.pvp_nightmaredrop){ // Moved this outside so it works when PVP isnt enabled and during pk mode [Ancyker]
		for(j=0;j<MAX_DROP_PER_MAP;++j){
			int id = maps[sd.block_list::m].drop_list[j].drop_id;
			int type = maps[sd.block_list::m].drop_list[j].drop_type;
			int per = maps[sd.block_list::m].drop_list[j].drop_per;
			if(id == 0)
				continue;
			if(id == -1){//ランダムドロップ
				int eq_num=0,eq_n[MAX_INVENTORY];
				memset(eq_n,0,sizeof(eq_n));
				//先ず?備しているアイテム?をカウント
				for(i=0;i<MAX_INVENTORY;++i){
					int k;
					if( (type == 1 && !sd.status.inventory[i].equip)
						|| (type == 2 && sd.status.inventory[i].equip)
						||  type == 3){
						//InventoryIndexを格納
						for(k=0;k<MAX_INVENTORY;++k){
							if(eq_n[k] <= 0){
								eq_n[k]=i;
								break;
							}
						}
						eq_num++;
					}
				}
				if(eq_num > 0){
					int n = eq_n[rand()%eq_num];//該?アイテムの中からランダム
					if(rand()%10000 < per){
						if(sd.status.inventory[n].equip)
							pc_unequipitem(sd,n,3);
						pc_dropitem(sd,n,1);
					}
				}
			}
			else if(id > 0){
				for(i=0;i<MAX_INVENTORY;++i){
					if(sd.status.inventory[i].nameid == id//ItemIDが一致していて
						&& rand()%10000 < per//ドロップ率判定もOKで
						&& ((type == 1 && !sd.status.inventory[i].equip)//タイプ判定もOKならドロップ
							|| (type == 2 && sd.status.inventory[i].equip)
							|| type == 3) ){
						if(sd.status.inventory[i].equip)
							pc_unequipitem(sd,i,3);
						pc_dropitem(sd,i,1);
						break;
					}
				}
			}
		}
	}
	// pvp
	if( maps[sd.block_list::m].flag.pvp && !config.pk_mode)
	{	// disable certain pvp functions on pk_mode [Valaris]
		//ランキング計算
		if (!maps[sd.block_list::m].flag.pvp_nocalcrank)
		{
			sd.pvp_point -= (sd.pvp_point>5)?5:sd.pvp_point;
			sd.pvp_lost++;
			if (src && *src== BL_PC)
			{
				map_session_data *ssd = src->get_sd();
				if (ssd)
				{
					ssd->pvp_point++; ssd->pvp_won++;
				}
			}
			sd.set_dead();
		}
		// ?制送還
		if( sd.pvp_point == 0 )
		{
			add_timer(gettick()+1000, pc_respawn,sd.block_list::id,0);
			return damage;
		}
	}
	//GvG
	if(maps[sd.block_list::m].flag.gvg){
		add_timer(gettick()+1000, pc_respawn,sd.block_list::id,0);
	}
	return damage;
}

//
// script? 連
//
/*==========================================
 * script用PCステ?タス?み出し
 *------------------------------------------
 */
int pc_readparam(map_session_data &sd,int type)
{
	int val=0;
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);

	switch(type){
	case SP_SKILLPOINT:
		val= sd.status.skill_point;
		break;
	case SP_STATUSPOINT:
		val= sd.status.status_point;
		break;
	case SP_ZENY:
		val= sd.status.zeny;
		break;
	case SP_BASELEVEL:
		val= sd.status.base_level;
		break;
	case SP_JOBLEVEL:
		val= sd.status.job_level;
		break;
	case SP_CLASS:
		if(val>=24 && val < 45)
			val+=3978;
		else
		val= sd.status.class_;
		break;
	case SP_BASEJOB:
		val= s_class.job;
		break;
	case SP_UPPER:
		val= s_class.upper;
		break;
	case SP_SEX:
		val= sd.status.sex;
		break;
	case SP_WEIGHT:
		val= sd.weight;
		break;
	case SP_MAXWEIGHT:
		val= sd.max_weight;
		break;
	case SP_BASEEXP:
		val= sd.status.base_exp;
		break;
	case SP_JOBEXP:
		val= sd.status.job_exp;
		break;
	case SP_NEXTBASEEXP:
		val= pc_nextbaseexp(sd);
		break;
	case SP_NEXTJOBEXP:
		val= pc_nextjobexp(sd);
		break;
	case SP_HP:
		val= sd.status.hp;
		break;
	case SP_MAXHP:
		val= sd.status.max_hp;
		break;
	case SP_SP:
		val= sd.status.sp;
		break;
	case SP_MAXSP:
		val= sd.status.max_sp;
		break;
	case SP_STR:
		val= sd.status.str;
		break;
	case SP_AGI:
		val= sd.status.agi;
		break;
	case SP_VIT:
		val= sd.status.vit;
		break;
	case SP_INT:
		val= sd.status.int_;
		break;
	case SP_DEX:
		val= sd.status.dex;
		break;
	case SP_LUK:
		val= sd.status.luk;
		break;
	case SP_KARMA:	// celest
		val = sd.status.karma;
		break;
	case SP_CHAOS:
		val = sd.status.chaos;
		break;
	case SP_MANNER:
		val = sd.status.manner;
		break;
	case SP_FAME:
		val= sd.status.fame_points;
		break;
	}

	return val;
}

/*==========================================
 * script用PCステ?タス設定
 *------------------------------------------
 */
int pc_setparam(map_session_data &sd,int type,int val)
{
	int i = 0;
	int up_level = 50;
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);

	switch(type){
	case SP_BASELEVEL:
		if (val > sd.status.base_level) {
			for (i = 1; i <= (val - sd.status.base_level); ++i)
				sd.status.status_point += (sd.status.base_level + i + 14) / 5 ;
		}
		sd.status.base_level = (val>0) ? val : 1;
		sd.status.base_exp = 0;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_updatestatus(sd, SP_BASEEXP);
		status_calc_pc(sd, 0);
		sd.heal(sd.status.max_hp, sd.status.max_sp);
		break;
	case SP_JOBLEVEL:
		if (s_class.job == 0) //Novice & Baby Novice have 10 Job Levels only
			up_level -= 40;
		else if (s_class.job == 23) //Super Novice & Super Baby can go up to 99
			up_level += 49;
		else if (sd.status.class_ >= 4008 && sd.status.class_ <= 4022) //3rd Job has 70 Job Levels
			up_level += 20;
		if (val >= sd.status.job_level) {
			if(val > up_level)
				val = up_level;
			sd.status.skill_point += (val-sd.status.job_level);
			sd.status.job_level = val;
			sd.status.job_exp = 0;
			clif_updatestatus(sd, SP_JOBLEVEL);
			clif_updatestatus(sd, SP_NEXTJOBEXP);
			clif_updatestatus(sd, SP_JOBEXP);
			clif_updatestatus(sd, SP_SKILLPOINT);
			status_calc_pc(sd, 0);
			clif_misceffect(sd, 1);
		} else {
			sd.status.job_level = (val>0) ? val : 1;
			sd.status.job_exp = 0;
			clif_updatestatus(sd, SP_JOBLEVEL);
			clif_updatestatus(sd, SP_NEXTJOBEXP);
			clif_updatestatus(sd, SP_JOBEXP);
			status_calc_pc(sd, 0);
		}
		clif_updatestatus(sd,type);
		break;
	case SP_SKILLPOINT:
		sd.status.skill_point = (val>0) ? val : 0;
		break;
	case SP_STATUSPOINT:
		sd.status.status_point = (val>0) ? val : 0;
		break;
	case SP_ZENY:
		if(val < 0)
			sd.status.zeny = 0;
		else if(val <= MAX_ZENY)
			sd.status.zeny = val;
		else // val > MAX_ZENY
			sd.status.zeny = MAX_ZENY;
		break;
	case SP_BASEEXP:
		if(pc_nextbaseexp(sd) > 0) {
			sd.status.base_exp = (val>0) ? val : 0;
			pc_checkbaselevelup(sd);
		}
		break;
	case SP_JOBEXP:
		if(pc_nextjobexp(sd) > 0) {
			sd.status.job_exp = (val>0) ? val : 0;
			pc_checkjoblevelup(sd);
		}
		break;
	case SP_SEX:
		sd.status.sex = val&0x1; // only use the LSB
		break;
	case SP_WEIGHT:
		sd.weight = (val>0) ? val : 0;
		break;
	case SP_MAXWEIGHT:
		sd.max_weight = (val>0) ? val : 0;
		break;
	case SP_HP:
		sd.status.hp = (val>0) ? val : 0;
		break;
	case SP_MAXHP:
		sd.status.max_hp = (val>0) ? val : 0;
		break;
	case SP_SP:
		sd.status.sp = (val>0) ? val : 0;
		break;
	case SP_MAXSP:
		sd.status.max_sp = (val>0) ? val : 0;
		break;
	case SP_STR:
		sd.status.str = (val>0) ? val : 0;
		break;
	case SP_AGI:
		sd.status.agi = (val>0) ? val : 0;
		break;
	case SP_VIT:
		sd.status.vit = (val>0) ? val : 0;
		break;
	case SP_INT:
		sd.status.int_ = (val>0) ? val : 0;
		break;
	case SP_DEX:
		sd.status.dex = (val>0) ? val : 0;
		break;
	case SP_LUK:
		sd.status.luk = (val>0) ? val : 0;
		break;
	case SP_KARMA:
		sd.status.karma = (val>100)?100 : (val<-100) ? -100 : val;
		break;
	case SP_CHAOS:
		sd.status.chaos = (val>100)?100 : (val<-100) ? -100 : val;
		break;
	case SP_MANNER:
		sd.status.manner= (val>32767)?32767 : (val<-32768) ? -32768 : val;
		break;
	case SP_FAME:
		sd.status.fame_points = (val>0) ? val : 0;
		break;
	}
	clif_updatestatus(sd,type);

	return 0;
}



/*==========================================
 * HP/SP回復
 *------------------------------------------
 */
int pc_itemheal(map_session_data &sd,long hp,long sp)
{
	int bonus, type;
	if(config.battle_log)
		ShowMessage("heal %d %d\n",hp,sp);

	if(	sd.has_status(SC_GOSPEL) ) //バ?サ?ク中は回復させないらしい
		return 0;

	if(sd.state.potion_flag==1)
	{
		sd.potion_hp = hp;
		sd.potion_sp = sp;
		return 0;
	}

	if( hp>0 && pc_checkoverhp(sd) )
			hp = 0;
	if( sp>0 && pc_checkoversp(sd) )
			sp = 0;

	if(hp > 0)
	{
		bonus = (sd.paramc[2]<<1) + 100 + sd.skill_check(SM_RECOVERY)*10
			+ sd.skill_check(AM_LEARNINGPOTION)*5 + (sd.state.potion_flag == 2)*50; // A potion produced by an Alchemist in the Fame Top 10 gets +50% effect [DracoRPG]
		if ((type = itemdb_group(sd.itemid)) > 0 && type <= 7)
			bonus = bonus * (100+sd.itemhealrate[type - 1]) / 100;
		if(bonus != 100)
			hp = hp * bonus / 100;
	}
	if(sp > 0)
	{
		bonus = (sd.paramc[3]<<1) + 100 + sd.skill_check(MG_SRECOVERY)*10
			+ sd.skill_check(AM_LEARNINGPOTION)*5 + (sd.state.potion_flag == 2)*50; // A potion produced by an Alchemist in the Fame Top 10 gets +50% effect [DracoRPG]
		if(bonus != 100)
			sp = sp * bonus / 100;
	}
	if(hp+sd.status.hp>sd.status.max_hp)
		hp=sd.status.max_hp-sd.status.hp;
	if(sp+sd.status.sp>sd.status.max_sp)
		sp=sd.status.max_sp-sd.status.sp;
	sd.status.hp+=hp;
	if(sd.status.hp <= 0) {
		sd.status.hp = 0;
		pc_damage(sd,1,NULL);
		hp = 0;
	}
	sd.status.sp+=sp;
	if(sd.status.sp <= 0)
		sd.status.sp = 0;
	if(hp)
		clif_updatestatus(sd,SP_HP);
	if(sp)
		clif_updatestatus(sd,SP_SP);

	return 0;
}

/*==========================================
 * HP/SP回復
 *------------------------------------------
 */
int pc_percentheal(map_session_data &sd,long hp,long sp)
{
	if(sd.state.potion_flag==1)
	{
		sd.potion_per_hp = hp;
		sd.potion_per_sp = sp;
		return 0;
	}

	if( hp>0 && pc_checkoverhp(sd) )
			hp = 0;
	if( sp>0 && pc_checkoversp(sd) )
			sp = 0;
	
	if(hp)
	{
		if(hp >= 100)
		{
			sd.status.hp = sd.status.max_hp;
	}
		else if(hp <= -100)
		{
			sd.status.hp = 0;
			pc_damage(sd,1,NULL);
		}
		else
		{
			sd.status.hp += sd.status.max_hp*hp/100;
			if(sd.status.hp > sd.status.max_hp)
				sd.status.hp = sd.status.max_hp;
			if(sd.status.hp <= 0)
			{
				sd.status.hp = 0;
				pc_damage(sd,1,NULL);
				hp = 0;
			}
		}
	}
	if(sp)
	{
		if(sp >= 100)
		{
			sd.status.sp = sd.status.max_sp;
		}
		else if(sp <= -100)
		{
			sd.status.sp = 0;
		}
		else
		{
			sd.status.sp += sd.status.max_sp*sp/100;
			if(sd.status.sp > sd.status.max_sp)
				sd.status.sp = sd.status.max_sp;
			if(sd.status.sp < 0)
				sd.status.sp = 0;
		}
	}
	if(hp)
		clif_updatestatus(sd,SP_HP);
	if(sp)
		clif_updatestatus(sd,SP_SP);

	return 0;
}

/*==========================================
 * 職?更
 * 引?	job 職業 0～23
 *		upper 通常 0, ?生 1, 養子 2, そのまま -1
 * Rewrote to make it tidider [Celest]
 *------------------------------------------
 */
int pc_jobchange(map_session_data &sd,int job, int upper)
{
	int i;
	int b_class = 0;
	//?生や養子の場合の元の職業を算出する
	struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);

	if (upper < 0 || upper > 2) //現在?生かどうかを判?する
		upper = s_class.upper;

	b_class = job;	//通常職ならjobそのまんま
	if (job < JOB_SUPER_NOVICE) {
		if (upper == 1)
			b_class += JOB_NOVICE_HIGH;
		else if (upper == 2)	//養子に結婚はないけどどうせ次で蹴られるからいいや
			b_class += JOB_BABY;
	} else if (job == JOB_SUPER_NOVICE) {
		if (upper == 1)	//?生にスパノビは存在しないのでお?り
			return 1;
		else if (upper == 2)
			b_class = JOB_SUPER_BABY;
	} else if (job < JOB_SUPER_BABY-JOB_NOVICE_HIGH+JOB_SUPER_NOVICE+2) {
		b_class += JOB_NOVICE_HIGH - JOB_SUPER_NOVICE -1;
	} else if (job >= JOB_TAEKWON && job <= JOB_SOUL_LINKER) {
		if (upper > 0)
			return 1;
	} else if (job < JOB_NOVICE_HIGH || job > JOB_SOUL_LINKER) //Invalid value
		return 1;

	job = pc_calc_base_job2 (b_class); // check base class [celest]

	if((sd.status.sex == 0 && job == 19) || (sd.status.sex == 1 && job == 20) ||
		// not needed [celest]
		//(sd.status.sex == 0 && job == 4020) || (sd.status.sex == 1 && job == 4021) ||
		job == 22 || sd.status.class_ == b_class) //♀はバ?ドになれない、♂はダンサ?になれない、結婚衣裳もお?り
		return 1;

	// check if we are changing from 1st to 2nd job
	if ((job >= JOB_KNIGHT && job <= JOB_CRUSADER2) || (job >= JOB_STAR_GLADIATOR && job <= JOB_SOUL_LINKER)) {
		if ((s_class.job > JOB_NOVICE && s_class.job < JOB_KNIGHT) || s_class.job == JOB_TAEKWON)
			sd.change_level = sd.status.job_level;
		else
			sd.change_level = 40;
	}
 	else
 		sd.change_level = 0;

	pc_setglobalreg(sd, "jobchange_level", sd.change_level);

	sd.status.class_ = sd.view_class = b_class;

	sd.status.job_level=1;
	sd.status.job_exp=0;
	clif_updatestatus(sd,SP_JOBLEVEL);
	clif_updatestatus(sd,SP_JOBEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);

	for(i=0;i<MAX_EQUIP;++i) {
		if(sd.equip_index[i] < MAX_INVENTORY)
			if(!pc_isequipable(sd,sd.equip_index[i]))
				pc_unequipitem(sd,sd.equip_index[i],2);	// ?備外し
	}

	clif_changelook(sd,LOOK_BASE,sd.view_class); // move sprite update to prevent client crashes with incompatible equipment [Valaris]

	if(config.save_clothcolor && sd.status.clothes_color > 0 &&
		(sd.view_class != 22 || !config.wedding_ignorepalette) )
		clif_changelook(sd,LOOK_CLOTHES_COLOR,sd.status.clothes_color);
	if(config.muting_players && sd.status.manner < 0)
		clif_changestatus(sd,SP_MANNER,sd.status.manner);

	status_calc_pc(sd,0);
	pc_checkallowskill(sd);
	pc_equiplookall(sd);
	clif_equiplist(sd);

	if(sd.is_riding())
	{	// remove peco status if changing into invalid class [Valaris]
		if(!(sd.skill_check(KN_RIDING)))
			pc_setoption(sd,sd.status.option|-0x0000);
		if(sd.skill_check(KN_RIDING)>0)
			 pc_setriding(sd);
	}

	return 0;
}

/*==========================================
 * 見た目?更
 *------------------------------------------
 */
int pc_equiplookall(map_session_data &sd)
{
#if PACKETVER < 4
	clif_changelook(sd,LOOK_WEAPON,sd.status.weapon);
	clif_changelook(sd,LOOK_SHIELD,sd.status.shield);
#else
	clif_changelook(sd,LOOK_WEAPON,0);
	clif_changelook(sd,LOOK_SHOES,0);
#endif
	clif_changelook(sd,LOOK_HEAD_BOTTOM,sd.status.head_bottom);
	clif_changelook(sd,LOOK_HEAD_TOP,sd.status.head_top);
	clif_changelook(sd,LOOK_HEAD_MID,sd.status.head_mid);

	return 0;
}

/*==========================================
 * 見た目?更
 *------------------------------------------
 */
int pc_changelook(map_session_data &sd,int type,unsigned short val)
{
	switch(type){
	case LOOK_HAIR:	//Use the config limits! [Skotlex]
		if (val < config.min_hair_style)
			val = config.min_hair_style;
		else if (val > config.max_hair_style)
			val = config.max_hair_style;
		sd.status.hair=val;
		break;
	case LOOK_WEAPON:
		sd.status.weapon=val;
		break;
	case LOOK_HEAD_BOTTOM:
		sd.status.head_bottom=val;
		break;
	case LOOK_HEAD_TOP:
		sd.status.head_top=val;
		break;
	case LOOK_HEAD_MID:
		sd.status.head_mid=val;
		break;
	case LOOK_HAIR_COLOR:	//Use the config limits! [Skotlex]
		if (val < config.min_hair_color)
			val = config.min_hair_color;
		else if (val > config.max_hair_color)
			val = config.max_hair_color;
		sd.status.hair_color=val;
		break;
	case LOOK_CLOTHES_COLOR:	//Use the config limits! [Skotlex]
		if (val < config.min_cloth_color)
			val = config.min_cloth_color;
		else if (val > config.max_cloth_color)
			val = config.max_cloth_color;
		sd.status.clothes_color=val;
		break;
	case LOOK_SHIELD:
		sd.status.shield=val;
		break;
	case LOOK_SHOES:
		break;
	}
	clif_changelook(sd,type,val);

	return 0;
}

/*==========================================
 * 付?品(鷹,ペコ,カ?ト)設定
 *------------------------------------------
 */
int pc_setoption(map_session_data &sd,int type)
{
	sd.status.option=type;
	clif_changeoption(sd);
	status_calc_pc(sd,0);
	return 0;
}

/*==========================================
 * カ?ト設定
 *------------------------------------------
 */
int pc_setcart(map_session_data &sd, int type)
{
	static int cart[6]={0x0000,0x0008,0x0080,0x0100,0x0200,0x0400};
	size_t option, i;
	if (type < 0 || type > 5)
		return 0; //Never trust the values sent by the client! [Skotlex]

	option = sd.status.option;
	for (i = 0; i < 6; ++i)
	{	//This should preserve the current option, only modifying the cart bit.
		if(i == (size_t)type)
			option |= cart[i];
		else
			option &= ~cart[i];
	}

	if(sd.skill_check(MC_PUSHCART)>0)
	{	// プッシュカ?トスキル所持
		if( !sd.is_carton() )
		{	// カ?トを付けていない
			pc_setoption(sd,option);
			clif_cart_itemlist(sd);
			clif_cart_equiplist(sd);
			clif_updatestatus(sd,SP_CARTINFO);
			clif_status_change(sd,0x0c,0);
		}
		else
		{
			pc_setoption(sd,option);
		}
	}
	return 0;
}

/*==========================================
 * 鷹設定
 *------------------------------------------
 */
int pc_setfalcon(map_session_data &sd)
{
	if(sd.skill_check(HT_FALCON)>0)
	{	// ファルコンマスタリ?スキル所持
		pc_setoption(sd,sd.status.option|0x0010);
	}
	return 0;
}

/*==========================================
 * ペコペコ設定
 *------------------------------------------
 */
int pc_setriding(map_session_data &sd)
{
	if((sd.skill_check(KN_RIDING)>0)){ // ライディングスキル所持
		pc_setoption(sd,sd.status.option|0x0020);

		if(sd.status.class_==7)
			sd.status.class_=sd.view_class=13;

		if(sd.status.class_==14)
			sd.status.class_=sd.view_class=21;

		if(sd.status.class_==4008)
			sd.status.class_=sd.view_class=4014;

		if(sd.status.class_==4015)
			sd.status.class_=sd.view_class=4022;
	}
	return 0;
}


/*==========================================
 * script用??の値を?む
 *------------------------------------------
 */
int pc_readreg(map_session_data &sd,int reg)
{
	int i;
	for(i=0;i<sd.reg_num;++i)
		if(sd.reg[i].index==reg)
			return sd.reg[i].data;

	return 0;
}
/*==========================================
 * script用??の値を設定
 *------------------------------------------
 */
int pc_setreg(map_session_data &sd,int reg,int val)
{
	int i;
	for (i = 0; i < sd.reg_num; ++i)
	{
		if (sd.reg[i].index == reg)
		{
			sd.reg[i].data = val;
			return 0;
		}
	}
	sd.reg_num = new_realloc(sd.reg, sd.reg_num, 1);
	sd.reg[i].index = reg;
	sd.reg[i].data = val;
	return 0;
}

/*==========================================
 * script用文字列??の値を?む
 *------------------------------------------
 */
const char *pc_readregstr(map_session_data &sd,int reg)
{
	int i;
	for(i=0;i<sd.regstr_num;++i)
		if(sd.regstr[i].index==reg)
			return sd.regstr[i].data;
	return NULL;
}
/*==========================================
 * script用文字列??の値を設定
 *------------------------------------------
 */
int pc_setregstr(map_session_data &sd,int reg,const char *str)
{
	if(str)
	{
		int i;
		if(strlen(str)+1 >= sizeof(sd.regstr[0].data)){
			ShowMessage("pc_setregstr: string too long !\n");
			return 0;
		}
		for(i=0;i<sd.regstr_num;++i)
		{
			if(sd.regstr[i].index==reg)
				break;
		}
		if(i >= sd.regstr_num)
		{
			sd.regstr_num = new_realloc(sd.regstr, sd.regstr_num, 1);
		}
		sd.regstr[i].index=reg;
		safestrcpy(sd.regstr[i].data,sizeof(sd.regstr[i].data),str);
	}
	return 0;
}

/*==========================================
 * script用グロ?バル??の値を?む
 *------------------------------------------
 */
int pc_readglobalreg(map_session_data &sd,const char *reg)
{
	size_t i;
	for(i=0;i<sd.status.global_reg_num;++i){
		if(strcmp(sd.status.global_reg[i].str,reg)==0)
			return sd.status.global_reg[i].value;
	}
	return 0;
}

/*==========================================
 * script用グロ?バル??の値を設定
 *------------------------------------------
 */
int pc_setglobalreg(map_session_data &sd,const char *reg,int val)
{
	size_t i;
	//PC_DIE_COUNTERがスクリプトなどで?更された暫ﾌ?理
	if(strcmp(reg,"PC_DIE_COUNTER") == 0 && sd.die_counter != val)
	{
		sd.die_counter = val;
		status_calc_pc(sd,0);
	}
	else if(strcmp(reg,script_config.die_event_name) == 0)
	{
		sd.state.event_death = val!=0;
	}
	else if(strcmp(reg,script_config.kill_event_name) == 0)
	{
		sd.state.event_kill = val!=0;
	}
	else if(strcmp(reg,script_config.logout_event_name) == 0)
	{
		sd.state.event_disconnect = val!=0;
	}

	// find name
	for(i=0;i<sd.status.global_reg_num;++i)
	{
		if( 0==strcmp(sd.status.global_reg[i].str,reg) )
			break;
	}
	if( i<sd.status.global_reg_num )
	{	// name found
		if( val==0 )
		{	// clear variable
			sd.status.global_reg[i] = sd.status.global_reg[sd.status.global_reg_num-1];
			sd.status.global_reg_num--;
		}
		else
		{	// change value
			sd.status.global_reg[i].value=val;
		}
	}
	else if( val==0 )
	{	// just nothing
	}
	else if(sd.status.global_reg_num<GLOBAL_REG_NUM)
	{	// append new value
		safestrcpy(sd.status.global_reg[i].str, sizeof(sd.status.global_reg[i].str), reg);
		sd.status.global_reg[i].value=val;
		sd.status.global_reg_num++;
	}
	else if(config.error_log)
	{	// error
		ShowMessage("pc_setglobalreg : couldn't set %s (GLOBAL_REG_NUM = %d)\n", reg, GLOBAL_REG_NUM);
		return 1;
	}
	return 0;
}

/*==========================================
 * script用アカウント??の値を?む
 *------------------------------------------
 */
int pc_readaccountreg(map_session_data &sd,const char *reg)
{
	size_t i;

	for(i=0;i<sd.status.account_reg_num;++i){
		if(strcmp(sd.status.account_reg[i].str,reg)==0)
			return sd.status.account_reg[i].value;
	}

	return 0;
}
/*==========================================
 * script用アカウント??の値を設定
 *------------------------------------------
 */
int pc_setaccountreg(map_session_data &sd,const char *reg,int val)
{
	size_t i;

	if(sd.status.account_reg_num >ACCOUNT_REG_NUM) {
		if(config.error_log)
			ShowMessage("pc_setaccountreg : refusing to set until vars are received\n");
		return 1;
	}

	if(val==0){
		for(i=0;i<sd.status.account_reg_num;++i){
			if(strcmp(sd.status.account_reg[i].str,reg)==0){
				sd.status.account_reg[i]=sd.status.account_reg[sd.status.account_reg_num-1];
				sd.status.account_reg_num--;
				break;
			}
		}
		intif_saveaccountreg(sd);
		return 0;
	}
	for(i=0;i<sd.status.account_reg_num;++i){
		if(strcmp(sd.status.account_reg[i].str,reg)==0){
			sd.status.account_reg[i].value=val;
			intif_saveaccountreg(sd);
			return 0;
		}
	}
	if(sd.status.account_reg_num<ACCOUNT_REG_NUM){
		safestrcpy(sd.status.account_reg[i].str, sizeof(sd.status.account_reg[i].str), reg);
		sd.status.account_reg[i].value=val;
		sd.status.account_reg_num++;
		intif_saveaccountreg(sd);
		return 0;
	}
	if(config.error_log)
		ShowMessage("pc_setaccountreg : couldn't set %s (ACCOUNT_REG_NUM = %d)\n", reg, ACCOUNT_REG_NUM);

	return 1;
}
/*==========================================
 * script用アカウント??2の値を?む
 *------------------------------------------
 */
int pc_readaccountreg2(map_session_data &sd,const char *reg)
{
	size_t i;

	for(i=0;i<sd.status.account_reg2_num;++i){
		if(strcmp(sd.status.account_reg2[i].str,reg)==0)
			return sd.status.account_reg2[i].value;
	}

	return 0;
}
/*==========================================
 * script用アカウント??2の値を設定
 *------------------------------------------
 */
int pc_setaccountreg2(map_session_data &sd,const char *reg,int val)
{
	size_t i;
	if(val==0){
		for(i=0;i<sd.status.account_reg2_num;++i){
			if(strcmp(sd.status.account_reg2[i].str,reg)==0){
				sd.status.account_reg2[i]=sd.status.account_reg2[sd.status.account_reg2_num-1];
				sd.status.account_reg2_num--;
				break;
			}
		}
		chrif_saveaccountreg2(sd);
		return 0;
	}
	for(i=0;i<sd.status.account_reg2_num;++i){
		if(strcmp(sd.status.account_reg2[i].str,reg)==0){
			sd.status.account_reg2[i].value=val;
			chrif_saveaccountreg2(sd);
			return 0;
		}
	}
	if(sd.status.account_reg2_num<ACCOUNT_REG2_NUM){
		safestrcpy(sd.status.account_reg2[i].str, sizeof(sd.status.account_reg2[i].str), reg);
		sd.status.account_reg2[i].value=val;
		sd.status.account_reg2_num++;
		chrif_saveaccountreg2(sd);
		return 0;
	}
	if(config.error_log)
		ShowMessage("pc_setaccountreg2 : couldn't set %s (ACCOUNT_REG2_NUM = %d)\n", reg, ACCOUNT_REG2_NUM);

	return 1;
}

/*==========================================
 * イベントタイマ??理
 *------------------------------------------
 */
int pc_eventtimer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd=map_session_data::from_blid(id);
	if(sd != NULL)
	{
		int i;
		char *evname = (char *)data.ptr;
		for(i=0;i<MAX_EVENTTIMER;++i)
		{
			if( sd->eventtimer[i]==tid )
			{
				sd->eventtimer[i]=-1;
				npc_data::event(evname, *sd);
				break;
			}
		}
		if(i==MAX_EVENTTIMER && config.error_log)
			ShowWarning("pc_eventtimer: no such event timer\n");
		if(evname) delete[] evname;
	}
	return 0;
}
/*==========================================
 * イベントタイマ?追加
 *------------------------------------------
 */
int pc_addeventtimer(map_session_data &sd, unsigned long tick, const char *name)
{
	size_t i=0;
	if(name)
	{
		for(i=0;i<MAX_EVENTTIMER;++i)
			if( sd.eventtimer[i]==-1 )
				break;
		if(i<MAX_EVENTTIMER)
		{
			char *evname= new char[1+strlen(name)];
			memcpy(evname,name,(strlen(name)+1));
			sd.eventtimer[i]=add_timer(gettick()+tick,pc_eventtimer,sd.block_list::id,basics::numptr(evname), false);
			sd.eventcount++;
		}
		else
		{
			ShowWarning("pc_addeventtimer: eventqueue full");
		}
	}
	return 0;
}

/*==========================================
 * イベントタイマ?削除
 *------------------------------------------
 */
int pc_deleventtimer(map_session_data &sd,const char *name)
{
	size_t i;
	char * evname;

	if (sd.eventcount <= 0)
		return 0;

	for(i=0;i<MAX_EVENTTIMER;++i)
	{
		evname = (char *)(get_timer(sd.eventtimer[i])->data.ptr);
		if( sd.eventtimer[i]!=-1 && strcmp(evname, name)==0 )
		{
			delete[] evname;
			get_timer(sd.eventtimer[i])->data = 0;
			delete_timer(sd.eventtimer[i],pc_eventtimer);
			sd.eventtimer[i]=-1;
			sd.eventcount--;
				break;
			}
		}
	return 0;
}

/*==========================================
 * イベントタイマ?カウント値追加
 *------------------------------------------
 */
int pc_addeventtimercount(map_session_data &sd, const char *name, unsigned long tick)
{
	size_t i;
	for(i=0;i<MAX_EVENTTIMER;++i)
	{
		if( sd.eventtimer[i]!=-1 && strcmp((char*)(get_timer(sd.eventtimer[i])->data.ptr),name)==0 )
		{
			addtick_timer(sd.eventtimer[i],tick);
			break;
		}
	}

	return 0;
}

/*==========================================
 * イベントタイマ?全削除
 *------------------------------------------
 */
int pc_cleareventtimer(map_session_data &sd)
{
	int i;
	char * evname;
	for(i=0;i<MAX_EVENTTIMER;++i)
	{
		if( sd.eventtimer[i]!=-1 )
		{
			evname = (char *)(get_timer(sd.eventtimer[i])->data.ptr);
			delete[] evname;
			delete_timer(sd.eventtimer[i],pc_eventtimer);
			sd.eventtimer[i]=-1;
		}
	}

	return 0;
}

//
// ? 備物
//
/*==========================================
 * アイテムを?備する
 *------------------------------------------
 */
int pc_equipitem(map_session_data &sd,unsigned short inx, unsigned short pos)
{
	size_t i;
	unsigned short nameid, arrowpos;
	struct item_data *id;
	//?生や養子の場合の元の職業を算出する

	if( inx >= MAX_INVENTORY )
	{
		clif_equipitemack(sd,inx,0,0);	// fail
		return 0;
	}

	nameid = sd.status.inventory[inx].nameid;
	id = sd.inventory_data[inx];
	pos = pc_equippoint(sd,inx);
	if(config.battle_log)
		ShowMessage("(char %i) equip %d(%d) %x:%x\n",
			sd.status.char_id, nameid, inx, id->equip, pos);

	if( !pc_isequipable(sd,inx) || !pos || 
		sd.status.inventory[inx].attribute==1 ||
		sd.has_status(SC_BERSERK) )	// -- moonsoul (if player is berserk then cannot equip)
	{	// [Valaris]
		clif_equipitemack(sd,inx,0,0);	// fail
		return 0;
	}
	if(pos==0x88)
	{	// アクセサリ用例外?理
		int epor=0;
		if(sd.equip_index[0] < MAX_INVENTORY)
			epor |= sd.status.inventory[sd.equip_index[0]].equip;
		if(sd.equip_index[1] < MAX_INVENTORY)
			epor |= sd.status.inventory[sd.equip_index[1]].equip;
		epor &= 0x88;
		pos = epor == 0x08 ? 0x80 : 0x08;
	}

	// 二刀流?理
	if( (pos==0x22) &&		// 一?、?備要求箇所が二刀流武器かチェックする
		(id->equip==2) &&	// ? 手武器
		(sd.skill_check( AS_LEFT) > 0 || pc_calc_base_job2(sd.status.class_) == 12) ) // 左手修?有
	{
		int tpos=0;
		if(sd.equip_index[8] < MAX_INVENTORY)
			tpos |= sd.status.inventory[sd.equip_index[8]].equip;
		if(sd.equip_index[9] < MAX_INVENTORY)
			tpos |= sd.status.inventory[sd.equip_index[9]].equip;
		tpos &= 0x02;
		pos = (tpos==0x02) ? 0x20 : 0x02;
	}

	arrowpos=pc_checkequip(sd,9);	// Added by RoVeRT
	for(i=0;i<MAX_EQUIP;++i)
	{
		if(sd.equip_index[i] < MAX_INVENTORY && sd.status.inventory[sd.equip_index[i]].equip&pos)
			pc_unequipitem(sd,sd.equip_index[i],2);
	}
	// 弓矢?備
	if(pos==0x8000)
	{
		clif_arrowequip(sd,inx);
		clif_arrow_fail(sd,3);	// 3=矢が?備できました
	}
	else
		clif_equipitemack(sd,inx,pos,1);

	for(i=0;i<MAX_EQUIP;++i) {
		if(pos & equip_pos[i])
			sd.equip_index[i] = inx;
	}
	sd.status.inventory[inx].equip=pos;

	if(sd.status.inventory[inx].equip & 0x0002) {
		if(sd.inventory_data[inx])
			sd.weapontype1 = sd.inventory_data[inx]->look;
		else
			sd.weapontype1 = 0;
		pc_calcweapontype(sd);
		clif_changelook(sd,LOOK_WEAPON,sd.status.weapon);
	}
	if(sd.status.inventory[inx].equip & 0x0020) {
		if(sd.inventory_data[inx]) {
			if(sd.inventory_data[inx]->type == 4) {
				sd.status.shield = 0;
				if(sd.status.inventory[inx].equip == 0x0020)
					sd.weapontype2 = sd.inventory_data[inx]->look;
				else
					sd.weapontype2 = 0;
			}
			else if(sd.inventory_data[inx]->type == 5) {
				sd.status.shield = sd.inventory_data[inx]->look;
				sd.weapontype2 = 0;
			}
		}
		else
			sd.status.shield = sd.weapontype2 = 0;
		pc_calcweapontype(sd);
		clif_changelook(sd,LOOK_SHIELD,sd.status.shield);
	}
	if(sd.status.inventory[inx].equip & 0x0100) {
		if(sd.inventory_data[inx])
			sd.status.head_top = sd.inventory_data[inx]->look;
		else
			sd.status.head_top = 0;
		clif_changelook(sd,LOOK_HEAD_TOP,sd.status.head_top);
	}
	if(sd.status.inventory[inx].equip & 0x0200) {
		if(sd.inventory_data[inx] && (sd.status.inventory[inx].equip & 0x0100)==0 )
			sd.status.head_mid = sd.inventory_data[inx]->look;
		else
			sd.status.head_mid = 0;
		clif_changelook(sd,LOOK_HEAD_MID,sd.status.head_mid);
	}
	if(sd.status.inventory[inx].equip & 0x0001) {
		if(sd.inventory_data[inx] && (sd.status.inventory[inx].equip & 0x0300)==0 )
			sd.status.head_bottom = sd.inventory_data[inx]->look;
		else
			sd.status.head_bottom = 0;
		clif_changelook(sd,LOOK_HEAD_BOTTOM,sd.status.head_bottom);
	}
	if(sd.status.inventory[inx].equip & 0x0040)
		clif_changelook(sd,LOOK_SHOES,0);

	pc_checkallowskill(sd);	// ?備品でスキルか解除されるかチェック
	if( itemdb_look(sd.status.inventory[inx].nameid) == 11 && (arrowpos < MAX_INVENTORY) )
	{	// Added by RoVeRT
		clif_arrowequip(sd,arrowpos);
		sd.status.inventory[arrowpos].equip=32768;
	}
	status_calc_pc(sd,0);

	if(sd.state.infinite_endure)
	{
		if( !sd.has_status(SC_ENDURE) )
			status_change_start(&sd,SC_ENDURE,10,1,0,0,0,0);
	}
	else
	{
		if(	sd.has_status(SC_ENDURE) && sd.get_statusvalue2(SC_ENDURE).integer())
			status_change_end(&sd,SC_ENDURE,-1);
	}
	if( sd.has_status(SC_SIGNUMCRUCIS) && !sd.is_undead() )
		status_change_end(&sd,SC_SIGNUMCRUCIS,-1);
	if(sd.has_status(SC_DANCING) && (sd.status.weapon != 13 && sd.status.weapon !=14))
		skill_stop_dancing(&sd,0);

	return 0;
}

/*==========================================
 * ? 備した物を外す
 * type:
 * 0 - only unequip
 * 1 - calculate status after unequipping
 * 2 - force unequip
 *------------------------------------------
 */
int pc_unequipitem(map_session_data &sd,unsigned short inx, int flag)
{
	long hp = 0, sp = 0;
// -- moonsoul	(if player is berserk then cannot unequip)
//
	if( !(flag&2) && 
		(sd.has_status(SC_BLADESTOP) || sd.has_status(SC_BERSERK)) )
	{
		clif_unequipitemack(sd,inx,0,0);
		return 0;
	}

	if(config.battle_log)
		ShowMessage("unequip %d %x:%x\n",inx,pc_equippoint(sd,inx),sd.status.inventory[inx].equip);
	if(sd.status.inventory[inx].equip)
	{
		int i;
		for(i=0;i<MAX_EQUIP;++i) {
			if(sd.status.inventory[inx].equip & equip_pos[i])
			{
				sd.equip_index[i] = 0xFFFF;
				if(sd.unequip_losehp[i] > 0) {
					hp += sd.unequip_losehp[i];
					sd.unequip_losehp[i] = 0;
				}
				if(sd.unequip_losesp[i] > 0) {
					sp += sd.unequip_losesp[i];
					sd.unequip_losesp[i] = 0;
				}
			}
		}
		if(sd.status.inventory[inx].equip & 0x0002)
		{
			sd.weapontype1 = 0;
			sd.status.weapon = sd.weapontype2;
			pc_calcweapontype(sd);
			clif_changelook(sd,LOOK_WEAPON,sd.status.weapon);
		}
		if(sd.status.inventory[inx].equip & 0x0020)
		{
			sd.status.shield = sd.weapontype2 = 0;
			pc_calcweapontype(sd);
			clif_changelook(sd,LOOK_SHIELD,sd.status.shield);
		}
		if(sd.status.inventory[inx].equip & 0x0001)
		{
			sd.status.head_bottom = 0;
			clif_changelook(sd,LOOK_HEAD_BOTTOM,sd.status.head_bottom);
		}
		if(sd.status.inventory[inx].equip & 0x0100)
		{
			sd.status.head_top = 0;
			clif_changelook(sd,LOOK_HEAD_TOP,sd.status.head_top);
		}
		if(sd.status.inventory[inx].equip & 0x0200)
		{
			sd.status.head_mid = 0;
			clif_changelook(sd,LOOK_HEAD_MID,sd.status.head_mid);
		}
		if(sd.status.inventory[inx].equip & 0x0040)
			clif_changelook(sd,LOOK_SHOES,0);
		{
			if (sd.has_status(SC_BROKNWEAPON) && 
				sd.status.inventory[inx].equip & 0x0002 &&
				sd.status.inventory[inx].attribute == 1)
				status_change_end(&sd,SC_BROKNWEAPON,-1);
			if(sd.has_status(SC_BROKNARMOR) && 
				sd.status.inventory[inx].equip & 0x0010 &&
				sd.status.inventory[inx].attribute == 1)
				status_change_end(&sd,SC_BROKNARMOR,-1);
		}

		clif_unequipitemack(sd,inx,sd.status.inventory[inx].equip,1);
		sd.status.inventory[inx].equip=0;
		if(flag&1)
			pc_checkallowskill(sd);
		if(sd.weapontype1 == 0 && sd.weapontype2 == 0)
			skill_enchant_elemental_end(&sd,-1);  //武器持ち誓えは無?件で?性付?解除
	} else {
		clif_unequipitemack(sd,inx,0,0);
	}

	if(flag&1) {
		status_calc_pc(sd,0);
		if(	sd.has_status(SC_SIGNUMCRUCIS) && 
			!sd.is_undead() )
			status_change_end(&sd,SC_SIGNUMCRUCIS,-1);
	}

	if (hp > 0 || sp > 0) {
		if(hp > sd.status.hp)
			hp = sd.status.hp;
		if(sp > sd.status.sp)
			sp = sd.status.sp;
		sd.heal(-hp,-sp);
	}
	return 0;
}

/*==========================================
 * アイテムのindex番?を詰めたり
 * ? 備品の?備可能チェックを行なう
 *------------------------------------------
 */
int pc_checkitem(map_session_data &sd)
{
	size_t i,j;
	unsigned short id;
	int calc_flag = 0;
	struct item_data *it=NULL;

	if (sd.vender_id) //Avoid reorganizing items when we are vending, as that leads to exploits (pointed out by End of Exam)
		return 0;
	

	// 所持品空き詰め
	for(i=j=0;i<MAX_INVENTORY;++i)
	{
		if( (id=sd.status.inventory[i].nameid)==0)
			continue;
		if( config.item_check && !itemdb_available(id) )
		{
			if(config.error_log)
				ShowMessage("illeagal item id %d in %d[%s] inventory.\n",id,sd.block_list::id,sd.status.name);
			pc_delitem(sd,i,sd.status.inventory[i].amount,3);
			continue;
		}
		if(i>j)
		{
			memcpy(&sd.status.inventory[j],&sd.status.inventory[i],sizeof(struct item));
			sd.inventory_data[j] = sd.inventory_data[i];
		}
		j++;
	}
	if(j < MAX_INVENTORY)
		memset(&sd.status.inventory[j],0,sizeof(struct item)*(MAX_INVENTORY-j));
	for(i=j;i<MAX_INVENTORY;++i)
		sd.inventory_data[i] = NULL;

	// カ?ト?空き詰め
	for(i=j=0;i<MAX_CART;++i)
	{
		if( (id=sd.status.cart[i].nameid)==0 )
			continue;
		if( config.item_check &&  !itemdb_available(id) ){
			if(config.error_log)
				ShowMessage("illeagal item id %d in %d[%s] cart.\n",id,sd.block_list::id,sd.status.name);
			pc_cart_delitem(sd,i,sd.status.cart[i].amount,1);
			continue;
		}
		if(i>j){
			memcpy(&sd.status.cart[j],&sd.status.cart[i],sizeof(struct item));
		}
		j++;
	}
	if(j < MAX_CART)
		memset(&sd.status.cart[j],0,sizeof(struct item)*(MAX_CART-j));

	// ? 備位置チェック

	for(i=0;i<MAX_INVENTORY;++i)
	{
		it=sd.inventory_data[i];

		if(sd.status.inventory[i].nameid==0)
			continue;
		if(sd.status.inventory[i].equip & ~pc_equippoint(sd,i)) {
			sd.status.inventory[i].equip=0;
			calc_flag = 1;
		}
		//?備制限チェック
		if(sd.status.inventory[i].equip && maps[sd.block_list::m].flag.pvp && (it->flag.no_equip&1)){//PVP check for forbiden items. optimized by [Lupus]
			sd.status.inventory[i].equip=0;
			calc_flag = 1;
		}else if(sd.status.inventory[i].equip && maps[sd.block_list::m].flag.gvg && (it->flag.no_equip>1)){//GvG optimized by [Lupus]
			sd.status.inventory[i].equip=0;
			calc_flag = 1;
		}
	}

	pc_setequipindex(sd);
	if(calc_flag)
		status_calc_pc(sd,2);

	return 0;
}

int pc_checkoverhp(map_session_data &sd)
{
	if(sd.status.hp == sd.status.max_hp)
		return 1;
	if(sd.status.hp > sd.status.max_hp) {
		sd.status.hp = sd.status.max_hp;
		clif_updatestatus(sd,SP_HP);
		return 2;
	}
	return 0;
}

int pc_checkoversp(map_session_data &sd)
{
	if(sd.status.sp == sd.status.max_sp)
		return 1;
	if(sd.status.sp > sd.status.max_sp) {
		sd.status.sp = sd.status.max_sp;
		clif_updatestatus(sd,SP_SP);
		return 2;
	}
	return 0;
}

/*==========================================
 * PVP順位計算用(foreachinarea)
 *------------------------------------------
 */
/*
int pc_calc_pvprank_sub(block_list &bl,va_list &ap)
{
	map_session_data *sd1,*sd2=NULL;

	nullpo_retr(0, sd1=(map_session_data *)&bl);
	nullpo_retr(0, ap);
	sd2=va_arg(ap,map_session_data*);
	nullpo_retr(0, sd2);

	if( sd1->pvp_point > sd2->pvp_point )
		sd2->pvp_rank++;
	return 0;
}
*/
class CPcCalcPvprank : public CMapProcessor
{
	ICL_EMPTY_COPYCONSTRUCTOR(CPcCalcPvprank)
	map_session_data &sd2;
public:
	CPcCalcPvprank(map_session_data &s) : sd2(s)	{}
	~CPcCalcPvprank()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd1=bl.get_sd();
		if( sd1 && sd1->pvp_point > sd2.pvp_point )
			sd2.pvp_rank++;
		return 0;
	}
};
/*==========================================
 * PVP順位計算
 *------------------------------------------
 */
int pc_calc_pvprank(map_session_data &sd)
{
	struct map_intern &m=maps[sd.block_list::m];
	if( (m.flag.pvp) )
	{
		uint32 old=sd.pvp_rank;
		sd.pvp_rank=1;

		maps[sd.block_list::m].foreach(CPcCalcPvprank(sd), BL_PC);

		if(old!=sd.pvp_rank || sd.pvp_lastusers!=m.users)
			clif_pvpset(sd,sd.pvp_rank,sd.pvp_lastusers=m.users,0);
		return sd.pvp_rank;
	}
	return 0;
}
/*==========================================
 * PVP順位計算(timer)
 *------------------------------------------
 */
int pc_calc_pvprank_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	map_session_data *sd=NULL;
	if(config.pk_mode) // disable pvp ranking if pk_mode on [Valaris]
		return 0;

	sd=map_session_data::from_blid(id);
	if(sd==NULL)
		return 0;
	sd->pvp_timer=-1;
	if( pc_calc_pvprank(*sd)>0 )
		sd->pvp_timer=add_timer(
			gettick()+PVP_CALCRANK_INTERVAL,
			pc_calc_pvprank_timer,id,data);
	return 0;
}

/*==========================================
 * sdは結婚しているか(?婚の場合は相方のchar_idを返す)
 *------------------------------------------
 */
uint32 pc_ismarried(map_session_data &sd)
{
	return sd.status.partner_id;
}
/*==========================================
 * sdがdstsdと結婚(dstsd→sdの結婚?理も同暫ﾉ行う)
 *------------------------------------------
 */
bool pc_marriage(map_session_data &sd1,map_session_data &sd2)
{
	if( sd1.status.partner_id > 0 || sd2.status.partner_id > 0 ||
		pc_calc_upper(sd1.status.class_) == 2 || pc_calc_upper(sd2.status.class_) == 2)
		return false;
	sd1.status.partner_id = sd2.status.char_id;
	sd2.status.partner_id = sd1.status.char_id;
	return true;
}

/*==========================================
 * sdが離婚(相手はsd->status.partner_idに依る)(相手も同暫ﾉ離婚?結婚指輪自動?奪)
 *------------------------------------------
 */
bool pc_divorce(map_session_data &sd)
{
	map_session_data *p_sd;
	if( !pc_ismarried(sd) )
		return false;

	if( (p_sd = map_session_data::charid2sd(sd.status.partner_id)) != NULL )
	{
		size_t i;
		if (p_sd->status.partner_id != sd.status.char_id || sd.status.partner_id != p_sd->status.char_id)
		{
			ShowMessage("pc_divorce: Illegal partner_id sd=%d p_sd=%d\n",sd.status.partner_id,p_sd->status.partner_id);
			return false;
		}
		sd.status.partner_id = 0;
		p_sd->status.partner_id = 0;
		for (i = 0; i < MAX_INVENTORY; ++i)
			if (sd.status.inventory[i].nameid == WEDDING_RING_M || sd.status.inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(sd, i, 1, 0);
		for (i = 0; i < MAX_INVENTORY; ++i)
			if (p_sd->status.inventory[i].nameid == WEDDING_RING_M || p_sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(*p_sd, i, 1, 0);
		clif_divorced(sd, p_sd->status.name);
		clif_divorced(*p_sd, sd.status.name);
	}
	else
	{
		ShowMessage("pc_divorce: p_sd nullpo\n");
		return false;
}
	return true;
}

/*==========================================
 * sd - father dstsd - mother jasd - child
 */
bool pc_adoption(map_session_data &sd1,map_session_data &sd2, map_session_data &sd3)
{       
	size_t j;          
	if( sd1.status.partner_id <= 0 || sd2.status.partner_id <= 0 ||
		sd1.status.partner_id != sd2.status.char_id ||
		sd2.status.partner_id != sd1.status.char_id ||
		sd1.status.child_id > 0 || sd2.status.child_id > 0 ||
		sd3.status.father_id > 0 || sd3.status.mother_id > 0 )
		return false;

	sd3.status.father_id = sd1.status.char_id;
	sd3.status.mother_id = sd2.status.char_id;
	sd1.status.child_id  = sd3.status.char_id;
	sd2.status.child_id  = sd3.status.char_id;

	for (j=0; j < MAX_INVENTORY; ++j)
	{
		if( sd3.status.inventory[j].nameid>0 && sd3.status.inventory[j].equip!=0 )
			pc_unequipitem(sd3, j, 3);
	}

	if( pc_jobchange(sd3, 4023, 0) == 0 )
	{	//Success, and give Junior the Baby skills. [Skotlex]
		pc_skill(sd3,WE_BABY,1,0);
		pc_skill(sd3,WE_CALLPARENT,1,0);
		clif_displaymessage(sd3.fd, msg_txt(MSG_YOUR_JOB_CHANGED)); // Your job has been changed.
		//We should also grant the parent skills to the parents [Skotlex]
		pc_skill(sd1,WE_CALLBABY,1,0);
		pc_skill(sd2,WE_CALLBABY,1,0);
	}
	else
	{
		clif_displaymessage(sd3.fd, msg_txt(MSG_IMPOSSIBLE_TO_CHANGE_YOUR_JOB)); // Impossible to change your job.
		return false;
	}

	return true;
}

/*==========================================
 * sdの相方のmap_session_dataを返す
 *------------------------------------------
 */
map_session_data *pc_get_partner(map_session_data &sd)
{
	if( pc_ismarried(sd) )
		// charid2sd returns NULL if not online
		return map_session_data::charid2sd(sd.status.partner_id);
	return NULL;
}

map_session_data *pc_get_father (map_session_data &sd)
{
	if( pc_calc_upper(sd.status.class_) == 2 && sd.status.father_id > 0)
		// charid2sd returns NULL if not online
		return map_session_data::charid2sd(sd.status.father_id);
	return NULL;
}

map_session_data *pc_get_mother (map_session_data &sd)
{
	if( pc_calc_upper(sd.status.class_) == 2 && sd.status.mother_id > 0)
		// charid2sd returns NULL if not online
		return map_session_data::charid2sd(sd.status.mother_id);

	return NULL;
}

map_session_data *pc_get_child (map_session_data &sd)
{
	if( pc_ismarried(sd) && sd.status.child_id > 0)
		// charid2sd returns NULL if not online
		return map_session_data::charid2sd(sd.status.child_id);

	return NULL;
}

//
// 自然回復物
//
/*==========================================
 * SP回復量計算
 *------------------------------------------
 */
static uint32 natural_heal_tick, natural_heal_prev_tick, natural_heal_diff_tick;

int pc_spheal(map_session_data &sd)
{
	int a = natural_heal_diff_tick;
	
	if( sd.is_sitting() )
		a += a;
	{
		if( sd.has_status(SC_MAGNIFICAT) )	// マグニフィカ?ト
			a += a;
		if( sd.has_status(SC_REGENERATION) )
			a *= sd.get_statusvalue1(SC_REGENERATION).integer();
	}
	// Re-added back to status_calc
	//if((skill = sd.skill_check(HP_MEDITATIO)) > 0) //Increase natural SP regen with Meditatio [DracoRPG]
		//a += a*skill*3/100;
	
	if (sd.status.guild_id > 0) {
		struct guild_castle *gc = guild_mapname2gc(sd.mapname);	// Increased guild castle regen [Valaris]
		if(gc)	{
			struct guild *g = guild_search(sd.status.guild_id);
			if(g && g->guild_id == gc->guild_id)
				a += a;
		}	// end addition [Valaris]
	}

	if( maps[sd.block_list::m].is_regen(sd.block_list::x,sd.block_list::y) )
		a += a;

	return a;
}

/*==========================================
 * HP回復量計算
 *------------------------------------------
 */

int pc_hpheal(map_session_data &sd)
{
	int a = natural_heal_diff_tick;

	if( sd.is_sitting() )
		a += a;

	if( sd.has_status(SC_MAGNIFICAT) )
		a += a;
	if( sd.has_status(SC_REGENERATION) )
		a *= sd.get_statusvalue1(SC_REGENERATION).integer();

	if (sd.status.guild_id > 0) {
		struct guild_castle *gc = guild_mapname2gc(sd.mapname);	// Increased guild castle regen [Valaris]
		if(gc)	{
			struct guild *g = guild_search(sd.status.guild_id);
			if(g && g->guild_id == gc->guild_id)
				a += a;
		}	// end addition [Valaris]
	}

	if( maps[sd.block_list::m].is_regen(sd.block_list::x,sd.block_list::y) )
		a += a;

	return a;
}

int pc_natural_heal_hp(map_session_data &sd)
{
	int bhp,inc_num,bonus,hp_flag;

	if( sd.has_status(SC_TRICKDEAD) )
		return 0;

	if (sd.no_regen & 1)
		return 0;

	if(pc_checkoverhp(sd)) {
		sd.hp_sub = sd.inchealhptick = 0;
		return 0;
	}

	bhp=sd.status.hp;
	hp_flag = (sd.skill_check(SM_MOVINGRECOVERY) > 0 && sd.is_walking() );

	if( !sd.is_walking() ) {
		inc_num = pc_hpheal(sd);
		if( sd.has_status(SC_TENSIONRELAX) )
		{
			sd.hp_sub += 2*inc_num;
			sd.inchealhptick += 3*natural_heal_diff_tick;
		}
		else
		{
			sd.hp_sub += inc_num;
			sd.inchealhptick += natural_heal_diff_tick;
		}
	}
	else if(hp_flag) {
		inc_num = pc_hpheal(sd);
		sd.hp_sub += inc_num;
		sd.inchealhptick = 0;
	}
	else {
		sd.hp_sub = sd.inchealhptick = 0;
		return 0;
	}

	if(sd.hp_sub >= config.natural_healhp_interval) {
		bonus = sd.nhealhp;
		if(hp_flag) {
			bonus >>= 2;
			if(bonus <= 0) bonus = 1;
		}
		while(sd.hp_sub >= config.natural_healhp_interval) {
			sd.hp_sub -= config.natural_healhp_interval;
			if(sd.status.hp + bonus <= sd.status.max_hp)
				sd.status.hp += bonus;
			else {
				sd.status.hp = sd.status.max_hp;
				sd.hp_sub = sd.inchealhptick = 0;
			}
		}
	}
	if(bhp!=sd.status.hp)
		clif_updatestatus(sd,SP_HP);

	if(sd.nshealhp > 0) {
		if(sd.inchealhptick >= config.natural_heal_skill_interval && sd.status.hp < sd.status.max_hp)
		{
			bonus = sd.nshealhp;
			if(sd.doridori_counter && sd.skill_check(TK_HPTIME)) //TK_HPTIME doridori provided bonus [Dralnu]
				bonus += 30;
			//sd.doridori_counter = 0; set 0 on sp heal
			while(sd.inchealhptick >= config.natural_heal_skill_interval) {
				sd.inchealhptick -= config.natural_heal_skill_interval;
				if(sd.status.hp + bonus <= sd.status.max_hp)
					sd.status.hp += bonus;
				else {
					bonus = sd.status.max_hp - sd.status.hp;
					sd.status.hp = sd.status.max_hp;
					sd.hp_sub = sd.inchealhptick = 0;
				}
				clif_heal(sd.fd,SP_HP,bonus);
			}
		}
	}
	else sd.inchealhptick = 0;

	return 0;
}

int pc_natural_heal_sp(map_session_data &sd)
{
	int bsp,inc_num,bonus;

	if( sd.has_status(SC_TRICKDEAD) ||
		sd.has_status(SC_BERSERK) ||
		sd.has_status(SC_BLEEDING) )
		return 0;

	if (sd.no_regen & 2)
		return 0;

	if(pc_checkoversp(sd)) {
		sd.sp_sub = sd.inchealsptick = 0;
		return 0;
	}

	bsp=sd.status.sp;

	inc_num = pc_spheal(sd);
	if( !sd.has_status(SC_EXPLOSIONSPIRITS) )
		sd.sp_sub += inc_num;
	if( !sd.is_walking() )
		sd.inchealsptick += natural_heal_diff_tick;
	else sd.inchealsptick = 0;

	if(sd.sp_sub >= config.natural_healsp_interval){
		bonus = sd.nhealsp;;
		while(sd.sp_sub >= config.natural_healsp_interval){
			sd.sp_sub -= config.natural_healsp_interval;
			if(sd.status.sp + bonus <= sd.status.max_sp)
				sd.status.sp += bonus;
			else {
				sd.status.sp = sd.status.max_sp;
				sd.sp_sub = sd.inchealsptick = 0;
			}
		}
	}

	if(bsp != sd.status.sp)
		clif_updatestatus(sd,SP_SP);

	if(sd.nshealsp > 0) {
		if(sd.inchealsptick >= config.natural_heal_skill_interval && sd.status.sp < sd.status.max_sp)
		{
			struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);
			bonus = sd.nshealsp;
			if(sd.doridori_counter && s_class.job == 23)
				bonus *= 2;
			if(sd.doridori_counter && sd.skill_check(TK_SPTIME)) //TK_SPTIME doridori provided bonus [Dralnu]
				bonus += 3;
			sd.doridori_counter = 0;

			while(sd.inchealsptick >= config.natural_heal_skill_interval) {
				sd.inchealsptick -= config.natural_heal_skill_interval;
				if(sd.status.sp + bonus <= sd.status.max_sp)
					sd.status.sp += bonus;
				else {
					bonus = sd.status.max_sp - sd.status.sp;
					sd.status.sp = sd.status.max_sp;
					sd.sp_sub = sd.inchealsptick = 0;
				}
				clif_heal(sd.fd,SP_SP,bonus);
			}
		}
	}
	else sd.inchealsptick = 0;

	return 0;
}

int pc_spirit_heal_hp(map_session_data &sd)
{
	int bonus_hp;
	uint32 interval = config.natural_heal_skill_interval;

	if(pc_checkoverhp(sd)) {
		sd.inchealspirithptick = 0;
		return 0;
	}

	sd.inchealspirithptick += natural_heal_diff_tick;

	if(sd.weight*100 >= sd.max_weight*config.natural_heal_weight_rate)
		interval += interval;

	if(sd.inchealspirithptick >= interval) {
		bonus_hp = sd.nsshealhp;
		while(sd.inchealspirithptick >= interval) {
			if( sd.is_sitting() )
			{
				sd.inchealspirithptick -= interval;
				if(sd.status.hp < sd.status.max_hp) {
					if(sd.status.hp + bonus_hp <= sd.status.max_hp)
						sd.status.hp += bonus_hp;
					else {
						bonus_hp = sd.status.max_hp - sd.status.hp;
						sd.status.hp = sd.status.max_hp;
					}
					clif_heal(sd.fd,SP_HP,bonus_hp);
					sd.inchealspirithptick = 0;
				}
			}else{
				sd.inchealspirithptick -= natural_heal_diff_tick;
				break;
			}
		}
	}

	return 0;
}

int pc_spirit_heal_sp(map_session_data &sd)
{
	int bonus_sp;
	uint32 interval = config.natural_heal_skill_interval;

	if(pc_checkoversp(sd)) {
		sd.inchealspiritsptick = 0;
		return 0;
	}

	sd.inchealspiritsptick += natural_heal_diff_tick;

	if(sd.weight*100 >= sd.max_weight*config.natural_heal_weight_rate)
		interval += interval;

	if(sd.inchealspiritsptick >= interval) {
		bonus_sp = sd.nsshealsp;
		while(sd.inchealspiritsptick >= interval) {
			if( sd.is_sitting() )
			{
				sd.inchealspiritsptick -= interval;
				if(sd.status.sp < sd.status.max_sp) {
					if(sd.status.sp + bonus_sp <= sd.status.max_sp)
						sd.status.sp += bonus_sp;
					else {
						bonus_sp = sd.status.max_sp - sd.status.sp;
						sd.status.sp = sd.status.max_sp;
					}
					clif_heal(sd.fd,SP_SP,bonus_sp);
					sd.inchealspiritsptick = 0;
				}
			}else{
				sd.inchealspiritsptick -= natural_heal_diff_tick;
				break;
			}
		}
	}
	return 0;
}

int pc_bleeding(map_session_data &sd)
{
	long hp=0, sp=0;
	if (sd.hp_loss_value > 0)
	{
		sd.hp_loss_tick += natural_heal_diff_tick;
		if( sd.hp_loss_tick >= sd.hp_loss_rate )
		{
			do
			{
				hp += sd.hp_loss_value;
				sd.hp_loss_tick -= sd.hp_loss_rate;
			} while (sd.hp_loss_tick >= sd.hp_loss_rate);
			sd.hp_loss_tick = 0;
		}
	}
	if (sd.sp_loss_value > 0)
	{
		sd.sp_loss_tick += natural_heal_diff_tick;
		if (sd.sp_loss_tick >= sd.sp_loss_rate)
		{
			do
			{
				sp += sd.sp_loss_value;
				sd.sp_loss_tick -= sd.sp_loss_rate;
			} while (sd.sp_loss_tick >= sd.sp_loss_rate);
			sd.sp_loss_tick = 0;
		}
	}
	if (hp > 0 || sp > 0)
		sd.heal(-hp,-sp);
	return 0;
}


/*==========================================
 * HP/SP自然回復 (interval timer??)
 *------------------------------------------
 */
int pc_natural_heal(int tid, unsigned long tick, int id, basics::numptr data)
{
	natural_heal_tick = tick;
	natural_heal_diff_tick = DIFF_TICK(natural_heal_tick,natural_heal_prev_tick);

	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if(sd)
		{
			int skill;
			// -- moonsoul (if conditions below altered to disallow natural healing if under berserk status)
			if((config.natural_heal_weight_rate > 100 || sd->weight*100 < sd->max_weight * config.natural_heal_weight_rate) &&
				!sd->is_dead() &&
				!sd->is_hiding() &&
			//-- cannot regen for 5 minutes after using Berserk --- [Celest]
				DIFF_TICK (tick, sd->canregen_tick)>=0 &&
				(!(sd->has_status(SC_POISON) && !sd->has_status(SC_SLOWPOISON)) &&
				!sd->has_status(SC_BERSERK) ))
			{
				pc_natural_heal_hp(*sd);
				if( !sd->has_status(SC_EXTREMITYFIST) &&	//阿修羅?態ではSPが回復しない
					!sd->has_status(SC_DANCING) &&			//ダンス?態ではSPが回復しない
					!sd->has_status(SC_BERSERK) )			//バ?サ?ク?態ではSPが回復しない
					pc_natural_heal_sp(*sd);
				sd->canregen_tick = tick;
			}
			else
			{
				sd->hp_sub = sd->inchealhptick = 0;
				sd->sp_sub = sd->inchealsptick = 0;
			}

			skill = sd->skill_check(MO_SPIRITSRECOVERY);
			if( skill>0 && !sd->is_hiding() && !sd->has_status(SC_POISON) && !sd->has_status(SC_BERSERK) )
			{
				pc_spirit_heal_hp(*sd);
				pc_spirit_heal_sp(*sd);
			}
			else
			{
				sd->inchealspirithptick = 0;
				sd->inchealspiritsptick = 0;
			}

			if (sd->hp_loss_value > 0 || sd->sp_loss_value > 0)
				pc_bleeding(*sd);
			else
				sd->hp_loss_tick = sd->sp_loss_tick = 0;
		}
	}


	natural_heal_prev_tick = tick;
	return 0;
}

/*==========================================
 * セ?ブポイントの保存
 *------------------------------------------
 */
int pc_setsavepoint(map_session_data &sd, const char *mapname, unsigned short x, unsigned short y)
{
	safestrcpy(sd.status.save_point.mapname, sizeof(sd.status.save_point.mapname), mapname);
	sd.status.save_point.x = x;
	sd.status.save_point.y = y;

	return 0;
}

/*==========================================
 * 自動セ?ブ (timer??)
 *------------------------------------------
 */
int pc_autosave(int tid, unsigned long tick, int id, basics::numptr data)
{
	uint c=0;
	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data* sd = iter.data();
		if(sd)
		{
			// pet
			if(sd->status.pet_id && sd->pd)
				intif_save_petdata(sd->status.account_id, sd->pd->pet);
			pc_makesavestatus(*sd);
			chrif_save(*sd);
			storage_storage_dirty(*sd);
			storage_storage_save(*sd);
			if(sd->state.storage_flag)
				storage_guild_storagesave(*sd);
			++c;
		}
	}
	if(c && config.save_log)
		ShowStatus("autosave %d chars\n", c);

	add_timer(gettick()+((autosave_interval>1000)?autosave_interval:1000),pc_autosave,0,0);
	return 0;
}


//
// 初期化物
//
/*==========================================
 * 設定ファイル?み?む
 * exp.txt 必要??値
 * job_db1.txt 重量,hp,sp,攻?速度
 * job_db2.txt job能力値ボ?ナス
 * skill_tree.txt 各職?のスキルツリ?
 * attr_fix.txt ?性修正テ?ブル
 * size_fix.txt サイズ補正テ?ブル
 * refine_db.txt 精?デ?タテ?ブル
 *------------------------------------------
 */
int pc_readdb(void)
{
	size_t i,j,k;
	FILE *fp;
	char *p, line[1024];

	// 必要??値?み?み
	memset(exp_table,0,sizeof(exp_table));
	fp=basics::safefopen("db/exp.txt","r");
	if(fp==NULL){
		ShowError("can't read %s\n","db/exp.txt");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp)){
		int bn,b1,b2,b3,b4,b5,b6,jn,j1,j2,j3,j4,j5,j6;
		if( !is_valid_line(line) )
			continue;
		if(sscanf(line,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&bn,&b1,&b2,&b3,&b4,&b5,&b6,&jn,&j1,&j2,&j3,&j4,&j5,&j6)!=14)
			continue;
		exp_table[0][i]=bn;
		exp_table[1][i]=b1;
		exp_table[2][i]=b2;
		exp_table[3][i]=b3;
		exp_table[4][i]=b4;
		exp_table[5][i]=b5;
		exp_table[6][i]=b6;
		exp_table[7][i]=jn;
		exp_table[8][i]=j1;
		exp_table[9][i]=j2;
		exp_table[10][i]=j3;
		exp_table[11][i]=j4;
		exp_table[12][i]=j5;
		exp_table[13][i]=j6;
		i++;
		if(i >= config.maximum_level)
			break;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/exp.txt");

	// スキルツリ?
	memset(skill_tree,0,sizeof(skill_tree));

	fp=basics::safefopen("db/skill_tree.txt","r");
	if(fp==NULL){
		ShowError("can't read %s\n", "db/skill_tree.txt");
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		char *split[50];
		int f=0, m=3, jobid;
		if( !is_valid_line(line) )
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<14 && p;++j)
		{
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j<13)
			continue;
		if (j == 14)
		{	// MinJobLvl has been added
			f=1;
			m++;
		}
		// check for bounds [celest]
		jobid = atoi(split[0]);
		if ( jobid>= MAX_PC_CLASS)
			continue;
		k = atoi(split[1]); //This is to avoid adding two lines for the same skill. [Skotlex]
		for(j = 0; j < MAX_SKILL_TREE && skill_tree[jobid][j].id && skill_tree[jobid][j].id != k; ++j);
		if (j == MAX_SKILL_TREE)
		{
			ShowWarning("Unable to load skill %d into job %d's tree. Maximum number of skills per class has been reached.\n", k, jobid);
			continue;
		}
		skill_tree[jobid][j].id=k;
		skill_tree[jobid][j].max=atoi(split[2]);
		if(f) 
			skill_tree[jobid][j].joblv=atoi(split[3]);

		for(k=0;k<5;++k){
			skill_tree[jobid][j].need[k].id=atoi(split[k*2+m]);
			skill_tree[jobid][j].need[k].lv=atoi(split[k*2+m+1]);
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/skill_tree.txt");

	// ?性修正テ?ブル
	for(i=0;i<4;++i)
		for(j=0;j<10;++j)
			for(k=0;k<10;++k)
				attr_fix_table[i][j][k]=100;
	fp=basics::safefopen("db/attr_fix.txt","r");
	if(fp==NULL){
		ShowError("can't read %s\n","db/attr_fix.txt");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)){
		char *split[10];
		size_t lv,n;
		if( !is_valid_line(line) )
			continue;
		for(j=0,p=line;j<3 && p;++j){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		lv=atoi(split[0]);
		n=atoi(split[1]);
//		ShowMessage("%d %d\n",lv,n);

		for(i=0;i<n;){
			if( !fgets(line, sizeof(line), fp) )
				break;
			if( !is_valid_line(line) )
				continue;

			for(j=0,p=line;j<n && p;++j){
				while(*p==32 && *p>0)
					p++;
				attr_fix_table[lv-1][i][j]=atoi(p);
				if(config.attr_recover == 0 && attr_fix_table[lv-1][i][j] < 0)
					attr_fix_table[lv-1][i][j] = 0;
				p=strchr(p,',');
				if(p) *p++=0;
			}

			i++;
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/attr_fix.txt");

	// スキルツリ?
	memset(statp,0,sizeof(statp));
	i=1;
	j=45;	// base points
	fp=basics::safefopen("db/statpoint.txt","r");
	if(fp == NULL)
	{
		ShowError("Can't read '"CL_WHITE"%s"CL_RESET"'... Generating DB.\n","db/statpoint.txt");
		//return 1;
	}
	else
	{
		while(fgets(line, sizeof(line), fp))
		{
			if( !is_valid_line(line) )
				continue;
			int x = atoi(line);
			if(x<0)
				x=0;
			if(i >= MAX_LEVEL)
				break;
			statp[i]=x;			
			++i;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/statpoint.txt");
	}
	// generate the remaining parts of the db if necessary
	for (; i < MAX_LEVEL; ++i) {
		j += (i+15)/5;
		statp[i] = j;		
	}
	return 0;
}

/*==========================================
 * pc? 係初期化
 *------------------------------------------
 */
void do_final_pc(void)
{
}

int do_init_pc(void)
{
	pc_readdb();

	add_timer_func_list(pc_natural_heal, "pc_natural_heal");
	add_timer_func_list(pc_invincible_timer, "pc_invincible_timer");
	add_timer_func_list(pc_eventtimer, "pc_eventtimer");
	add_timer_func_list(pc_calc_pvprank_timer, "pc_calc_pvprank_timer");
	add_timer_func_list(pc_autosave, "pc_autosave");
	add_timer_func_list(pc_spiritball_timer, "pc_spiritball_timer");
	add_timer_func_list(pc_blockskill_end, "pc_blockskill_end");
	add_timer_func_list(pc_follow_timer, "pc_follow_timer");	

	add_timer_interval((natural_heal_prev_tick = gettick() + NATURAL_HEAL_INTERVAL), NATURAL_HEAL_INTERVAL, pc_natural_heal, 0, 0);
	add_timer(gettick() + autosave_interval, pc_autosave, 0, 0);

	return 0;
}
