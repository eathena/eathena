#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "timer.h"
#include "socket.h"
#include "db.h"
#include "nullpo.h"
#include "malloc.h"
#include "map.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "guild.h"
#include "itemdb.h"
#include "skill.h"
#include "battle.h"
#include "party.h"
#include "npc.h"
#include "log.h"
#include "showmsg.h"
#include "script.h"
#include "atcommand.h"

#define MIN_MOBTHINKTIME 100
#define MIN_MOBLINKTIME 1000

#define MOB_LAZYMOVEPERC 50	// Move probability in the negligent mode MOB (rate of 1000 minute)
#define MOB_LAZYWARPPERC 20	// Warp probability in the negligent mode MOB (rate of 1000 minute)

//Dynamic mob database, allows saving of memory when there's big gaps in the mob_db [Skotlex]
struct mob_db *mob_db_data[MAX_MOB_DB+1];
struct mob_db *mob_dummy = NULL;	//Dummy mob to be returned when a non-existant one is requested.

struct mob_db *mob_db(int index) { if (index < 0 || index > MAX_MOB_DB || mob_db_data[index] == NULL) return mob_dummy; return mob_db_data[index]; }

#define CLASSCHANGE_BOSS_NUM 21

/*==========================================
 * Local prototype declaration   (only required thing)
 *------------------------------------------
 */
static int distance(int,int,int,int);
static int mob_makedummymobdb(int);
static int mob_timer(int,unsigned int,int,int);
static int mob_spawn_guardian_sub(int,unsigned int,int,int);
int mobskill_use(struct mob_data *md,unsigned int tick,int event);
int mobskill_deltimer(struct mob_data *md );
int mob_skillid2skillidx(int class_,int skillid);
int mobskill_use_id(struct mob_data *md,struct block_list *target,int skill_idx);

/*==========================================
 * Mob is searched with a name.
 *------------------------------------------
 */
int mobdb_searchname(const char *str)
{
	int i;
	struct mob_db* mob;
	for(i=0;i<=MAX_MOB_DB;i++){
		mob = mob_db(i);
		if(mob == mob_dummy) //Skip dummy mobs.
			continue;
		if(strcmpi(mob->name,str)==0 || strcmp(mob->jname,str)==0 ||
			memcmp(mob->name,str,NAME_LENGTH)==0 || memcmp(mob->jname,str,NAME_LENGTH)==0)
			return i;
	}

	return 0;
}

/*==========================================
 * Id Mob is checked.
 *------------------------------------------
 */
int mobdb_checkid(const int id)
{
	if (mob_db(id) == mob_dummy)
		return 0;

	return id;
}

/*==========================================
 * The minimum data set for MOB spawning
 *------------------------------------------
 */
int mob_spawn_dataset(struct mob_data *md,const char *mobname,int class_)
{
	nullpo_retr(0, md);

	md->bl.prev=NULL;
	md->bl.next=NULL;
	
	md->base_class = md->class_ = class_;
	md->db = mob_db(class_);

	if(strcmp(mobname,"--en--")==0)
		strncpy(md->name,md->db->name,NAME_LENGTH-1);
	else if(strcmp(mobname,"--ja--")==0)
		strncpy(md->name,md->db->jname,NAME_LENGTH-1);
	else
		strncpy(md->name,mobname,NAME_LENGTH-1);

	md->n = 0;
	md->bl.id= npc_get_new_npc_id();
	
	memset(&md->state,0,sizeof(md->state));
	md->timer = -1;
	md->target_id=0;
	md->attacked_id=0;
	md->attacked_count=0;
	md->speed=md->db->speed;

	return 0;
}


/*==========================================
 * The MOB appearance for one time (for scripts)
 *------------------------------------------
 */
int mob_once_spawn (struct map_session_data *sd, char *mapname,
	int x, int y, const char *mobname, int class_, int amount, const char *event)
{
	struct mob_data *md = NULL;
	int m, count, lv = 255;
	int i, j;
	
	if(sd) lv = sd->status.base_level;

	if(sd && strcmp(mapname,"this")==0)
		m = sd->bl.m;
	else
		m = map_mapname2mapid(mapname);

	if (m < 0 || amount <= 0 || (class_ >= 0 && class_ <= 1000) || class_ > MAX_MOB_DB + 2*MAX_MOB_DB)	// 値が異常なら召喚を止める
		return 0;

	if (class_ < 0) {	// ランダムに召喚
		struct mob_db *mob;
		int k;
		i = 0;
		j = -class_-1;
		if(j >= 0 && j < MAX_RANDOMMONSTER) {
			do {
				class_ = rand() % 1000 + 1001;
				k = rand() % 1000000;
				mob = mob_db(class_);
			} while ((mob == mob_dummy || mob->summonper[j] <= k ||
				 (battle_config.random_monster_checklv && lv < mob->lv)) && (i++) < 2000);
			if(i >= 2000)
				class_ = mob_db_data[0]->summonper[j];
		} else 
			return 0;
//		if(battle_config.etc_log)
//			printf("mobclass=%d try=%d\n",class_,i);
	}
	if (sd) { //even if the coords were wrong, spawn mob anyways (but look for most suitable coords first) Got from Freya [Lupus]
		if (x <= 0 || y <= 0) {
			if (x <= 0) x = sd->bl.x + rand() % 3 - 1;
			if (y <= 0) y = sd->bl.y + rand() % 3 - 1;
			if (map_getcell(m, x, y, CELL_CHKNOPASS)) {
				x = sd->bl.x;
				y = sd->bl.y;
			}
		}
	} else if (x <= 0 || y <= 0) {
		i = j = 0;
		do {
			x = rand() % (map[m].xs - 2) + 1;
			y = rand() % (map[m].ys - 2) + 1;
		} while ((i = map_getcell(m, x, y, CELL_CHKNOPASS)) && j++ < 64);
		if (i) { // not solved?
			x = 0;
			y = 0;
		}
	}

	for (count = 0; count < amount; count++) {
		md = (struct mob_data *)aCalloc(1,sizeof(struct mob_data));

		if (class_ > 2*MAX_MOB_DB) { // large/tiny mobs [Valaris]
			md->size = 2;
			class_ -= 2*MAX_MOB_DB;
		} else if (class_ > MAX_MOB_DB) {
			md->size = 1;
			class_ -= MAX_MOB_DB;
		}

		if(mob_db(class_)->mode & MD_LOOTER)
			md->lootitem = (struct item *)aCalloc(LOOTITEM_SIZE,sizeof(struct item));

		mob_spawn_dataset (md, mobname, class_);
		md->bl.m = m;
		md->bl.x = x;
		md->bl.y = y;
		if (class_ < 0 && battle_config.dead_branch_active)
			md->mode = md->db->mode|MD_AGGRESSIVE|MD_CANATTACK;
		md->m = m;
		md->x0 = x;
		md->y0 = y;
		//md->xs = 0;
		//md->ys = 0;
		md->spawndelay1 = -1;	// 一度のみフラグ
		md->spawndelay2 = -1;	// 一度のみフラグ

		//better safe than sorry, current md->npc_event has a size of 50
		if (strlen(event) < 50)
			memcpy(md->npc_event, event, strlen(event));

		md->bl.type = BL_MOB;
		map_addiddb (&md->bl);
		mob_spawn (md->bl.id);

		if(class_ == MOBID_EMPERIUM) {	// emperium hp based on defense level [Valaris]
			struct guild_castle *gc = guild_mapname2gc(map[md->bl.m].name);
			struct guild *g = gc?guild_search(gc->guild_id):NULL;
			if(gc) {
				md->max_hp += 2000 * gc->defense;
				md->hp = md->max_hp;
				md->guardian_data = aCalloc(1, sizeof(struct guardian_data));
				md->guardian_data->castle = gc;
				md->guardian_data->number = MAX_GUARDIANS;
				md->guardian_data->guild_id = gc->guild_id;
				if (g)
				{
					md->guardian_data->emblem_id = g->emblem_id;
					memcpy(md->guardian_data->guild_name, g->name, NAME_LENGTH);
				}
				else if (gc->guild_id) //Guild not yet available, retry in 5.
					add_timer(gettick()+5000,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
			}
		}	// end addition [Valaris]
	}
	return (amount > 0) ? md->bl.id : 0;
}
/*==========================================
 * The MOB appearance for one time (& area specification for scripts)
 *------------------------------------------
 */
int mob_once_spawn_area(struct map_session_data *sd,char *mapname,
	int x0,int y0,int x1,int y1,
	const char *mobname,int class_,int amount,const char *event)
{
	int x,y,i,max,lx=-1,ly=-1,id=0;
	int m;

	if(strcmp(mapname,"this")==0)
		m=sd->bl.m;
	else
		m=map_mapname2mapid(mapname);

	max=(y1-y0+1)*(x1-x0+1)*3;
	if(max>1000)max=1000;

	if(m<0 || amount<=0 || mob_db(class_) == mob_dummy)	// A summon is stopped if a value is unusual
		return 0;

	for(i=0;i<amount;i++){
		int j=0;
		do{
			x=rand()%(x1-x0+1)+x0;
			y=rand()%(y1-y0+1)+y0;
		} while (map_getcell(m,x,y,CELL_CHKNOPASS) && (++j)<max);
		// freya }while( ( (c=map_getcell(m,x,y))==1 || c==5)&& (++j)<max );
		if(j>=max){
			if(lx>=0){	// Since reference went wrong, the place which boiled before is used.
				x=lx;
				y=ly;
			}else
				return 0;	// Since reference of the place which boils first went wrong, it stops.
		}
		if(x==0||y==0) ShowWarning("mob_once_spawn_area: xory=0, x=%d,y=%d,x0=%d,y0=%d\n",x,y,x0,y0);
		id=mob_once_spawn(sd,mapname,x,y,mobname,class_,1,event);
		lx=x;
		ly=y;
	}
	return id;
}
/*==========================================
 * Set a Guardian's guild data [Skotlex]
 *------------------------------------------
 */
static int mob_spawn_guardian_sub(int tid,unsigned int tick,int id,int data)
{	//Needed because the guild_data may not be available at guardian spawn time.
	struct block_list* bl = map_id2bl(id);
	struct mob_data* md; 
	struct guild* g;

	if (bl == NULL) //It is possible mob was already removed from map when the castle has no owner. [Skotlex]
		return 0;
	
	if (bl->type != BL_MOB || (md = (struct mob_data*)bl) == NULL)
	{
		ShowError("mob_spawn_guardian_sub: Block error!\n");
		return 0;
	}
	
	nullpo_retr(0, md->guardian_data);
	g = guild_search(data);

	if (g == NULL)
	{	//Liberate castle, if the guild is not found this is an error! [Skotlex]
		ShowError("mob_spawn_guardian_sub: Couldn't load guild %d!\n",data);
		if (md->class_ == MOBID_EMPERIUM)
		{	//Not sure this is the best way, but otherwise we'd be invoking this for ALL guardians spawned later on.
			md->guardian_data->guild_id = 0;
			if (md->guardian_data->castle->guild_id) //Free castle up.
			{
				ShowNotice("Clearing ownership of castle %d (%s)\n", md->guardian_data->castle->castle_id, md->guardian_data->castle->castle_name);
				guild_castledatasave(md->guardian_data->castle->castle_id, 1, 0);
			}
		} else {
			if (md->guardian_data->castle->guardian[md->guardian_data->number].visible)
			{	//Safe removal of guardian.
				md->guardian_data->castle->guardian[md->guardian_data->number].visible = 0;
				guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
				guild_castledatasave(md->guardian_data->castle->castle_id, 18+md->guardian_data->number,0);
			}
			mob_delete(md); //Remove guardian.
		}
		return 0;
	}
	md->guardian_data->emblem_id = g->emblem_id;
	memcpy (md->guardian_data->guild_name, g->name, NAME_LENGTH);
	md->guardian_data->guardup_lv = guild_checkskill(g,GD_GUARDUP);
	return 0;
}

/*==========================================
 * Summoning Guardians [Valaris]
 *------------------------------------------
 */
int mob_spawn_guardian(struct map_session_data *sd,char *mapname,
	int x,int y,const char *mobname,int class_,int amount,const char *event,int guardian)
{
	struct mob_data *md=NULL;
	struct guild *g=NULL;
	struct guild_castle *gc;
	
	int m,count=1;

	if( sd && strcmp(mapname,"this")==0)
		m=sd->bl.m;
	else
		m=map_mapname2mapid(mapname);

	if(m<0 || amount<=0 || (class_>=0 && class_<=1000) || class_>MAX_MOB_DB)	// Invalid monster classes
		return 0;

	if(class_<0)
		return 0;

	if(guardian < 0 || guardian >= MAX_GUARDIANS)
	{
		ShowError("mob_spawn_guardian: Invalid guardian index %d for guardian %d (castle map %s)\n", guardian, class_, map[m].name);
		return 0;
	}
	if (amount > 1)
		ShowWarning("mob_spawn_guardian: Spawning %d guardians in position %d (castle map %s)\n", amount, map[m].name);
	
	if(sd){
		if(x<=0) x=sd->bl.x;
		if(y<=0) y=sd->bl.y;
	}
	else if(x<=0 || y<=0)
		ShowWarning("mob_spawn_guardian: Invalid coordinates (%d,%d)\n",x,y);

	gc=guild_mapname2gc(map[m].name);
	if (gc == NULL)
	{
		ShowError("mob_spawn_guardian: No castle set at map %s\n", map[m].name);
		return 0;
	}
	if (!gc->guild_id)
		ShowWarning("mob_spawn_guardian: Spawning guardian %d on a castle with no guild (castle map %s)\n", class_, map[m].name);
	else
		g = guild_search(gc->guild_id);

	if (gc->guardian[guardian].id)
		ShowWarning("mob_spawn_guardian: Spawning guardian in position %d which already has a guardian (castle map %s)\n", guardian, map[m].name);
	
	for(count=0;count<amount;count++){
		md=(struct mob_data *) aCalloc(1, sizeof(struct mob_data));
		mob_spawn_dataset(md,mobname,class_);
		md->bl.m=m;
		md->bl.x=x;
		md->bl.y=y;
		md->m =m;
		md->x0=x;
		md->y0=y;
		md->xs=0;
		md->ys=0;
		md->spawndelay1=-1;	// Only once is a flag.
		md->spawndelay2=-1;	// Only once is a flag.

		//better safe than sorry, current md->npc_event has a size of 50 [Skotlex]
		if (strlen(event) < 50)
			memcpy(md->npc_event, event, strlen(event));

		md->bl.type=BL_MOB;
		map_addiddb(&md->bl);
		mob_spawn(md->bl.id);

		md->max_hp += 2000 * gc->defense;
		md->guardian_data = aCalloc(1, sizeof(struct guardian_data));
		md->guardian_data->number = guardian;
		md->guardian_data->guild_id = gc->guild_id;
		md->guardian_data->castle = gc;
		md->hp = gc->guardian[guardian].hp;
		gc->guardian[guardian].id = md->bl.id;
		if (g)
		{
			md->guardian_data->emblem_id = g->emblem_id;
			memcpy (md->guardian_data->guild_name, g->name, NAME_LENGTH);
			md->guardian_data->guardup_lv = guild_checkskill(g,GD_GUARDUP);
		} else if (md->guardian_data->guild_id)
			add_timer(gettick()+5000,mob_spawn_guardian_sub,md->bl.id,md->guardian_data->guild_id);
	}

	return (amount>0)?md->bl.id:0;
}

/*==========================================
 * Is MOB in the state in which the present movement is possible or not?
 *------------------------------------------
 */
int mob_can_move(struct mob_data *md)
{
	nullpo_retr(0, md);

	if(md->canmove_tick > gettick() || (md->opt1 > 0 && md->opt1 != 6) || md->option&2)
		return 0;
	// アンクル中で動けないとか
	if( md->sc_data[SC_ANKLE].timer != -1 || //アンクルスネア
		md->sc_data[SC_AUTOCOUNTER].timer != -1 || //オートカウンター
		md->sc_data[SC_BLADESTOP].timer != -1 || //白刃取り
		md->sc_data[SC_SPIDERWEB].timer != -1 || //スパイダーウェッブ
		md->sc_data[SC_STOP].timer != -1
		)
		return 0;

	return 1;
}

/*==========================================
 * Time calculation concerning one step next to mob
 *------------------------------------------
 */
static int calc_next_walk_step(struct mob_data *md)
{
	nullpo_retr(0, md);

	if(md->walkpath.path_pos>=md->walkpath.path_len)
		return -1;
	if(md->walkpath.path[md->walkpath.path_pos]&1)
		return status_get_speed(&md->bl)*14/10;
	return status_get_speed(&md->bl);
}

static int mob_walktoxy_sub(struct mob_data *md);

/*==========================================
 * Mob Walk processing
 *------------------------------------------
 */
static int mob_walk(struct mob_data *md,unsigned int tick,int data)
{
	int moveblock;
	int i;
	static int dirx[8]={0,-1,-1,-1,0,1,1,1};
	static int diry[8]={1,1,0,-1,-1,-1,0,1};
	int x,y,dx,dy;

	nullpo_retr(0, md);

	md->state.state=MS_IDLE;
	if(md->walkpath.path_pos>=md->walkpath.path_len || md->walkpath.path_pos!=data)
		return 0;

	md->walkpath.path_half ^= 1;
	if(md->walkpath.path_half==0){
		md->walkpath.path_pos++;
		if(md->state.change_walk_target){
			mob_walktoxy_sub(md);
			return 0;
		}
	}
	else {
		if(md->walkpath.path[md->walkpath.path_pos]>=8)
			return 1;

		x = md->bl.x;
		y = md->bl.y;
		if(map_getcell(md->bl.m,x,y,CELL_CHKNOPASS)) {
			mob_stop_walking(md,1);
			return 0;
		}
		md->dir=md->walkpath.path[md->walkpath.path_pos];
		dx = dirx[md->dir];
		dy = diry[md->dir];

		if (map_getcell(md->bl.m,x+dx,y+dy,CELL_CHKBASILICA) && !(status_get_mode(&md->bl)&MD_BOSS)) {
			mob_stop_walking(md,1);
			return 0;
		}

		if (map_getcell(md->bl.m,x+dx,y+dy,CELL_CHKNOPASS)) {
			mob_walktoxy_sub(md);
			return 0;
		}

		if (skill_check_moonlit (&md->bl,x+dx,y+dy)) {
			mob_walktoxy_sub(md);
			return 0;
		}
		moveblock = ( x/BLOCK_SIZE != (x+dx)/BLOCK_SIZE || y/BLOCK_SIZE != (y+dy)/BLOCK_SIZE);

		md->state.state=MS_WALK;
		map_foreachinmovearea(clif_moboutsight,md->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,dx,dy,BL_PC,md);

		x += dx;
		y += dy;
	
		if ( md->min_chase > md->db->range2)
			md->min_chase--;
		
		skill_unit_move(&md->bl,tick,2);
		if(moveblock) map_delblock(&md->bl);
		md->bl.x = x;
		md->bl.y = y;
		if(moveblock) map_addblock(&md->bl);
		skill_unit_move(&md->bl,tick,3);

		map_foreachinmovearea(clif_mobinsight,md->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,-dx,-dy,BL_PC,md);
		md->state.state=MS_IDLE;

		if(md->option&4)
			skill_check_cloaking(&md->bl);
	}
	if((i=calc_next_walk_step(md))>0){
		i = i>>1;
		if(i < 1 && md->walkpath.path_half == 0)
			i = 1;

		if(md->walkpath.path_pos>=md->walkpath.path_len)
			clif_fixmobpos(md);	// とまったときに位置の再送信
		else {
			md->timer=add_timer(tick+i,mob_timer,md->bl.id,md->walkpath.path_pos);
			md->state.state=MS_WALK;
		}
	}
	return 0;
}

/*==========================================
 * Attack processing of mob
 *------------------------------------------
 */
static int mob_attack(struct mob_data *md,unsigned int tick,int data)
{
	struct block_list *tbl=NULL;

	int range;

	nullpo_retr(0, md);

	md->min_chase=md->db->range3;
	md->state.state=MS_IDLE;
	md->state.skillstate=MSS_IDLE;

	if( md->skilltimer!=-1 )	// スキル使用中
		return 0;

	if((tbl=map_id2bl(md->target_id))==NULL || !status_check_skilluse(&md->bl, tbl, 0, 0)){
		md->target_id=0;
		md->state.targettype = NONE_ATTACKABLE;
		return 0;
	}

	if (distance(md->bl.x,md->bl.y,tbl->x,tbl->y)>=md->db->range3){
		mob_stopattack(md);
		return 0;
	}

	range = md->db->range;
	
	if(status_get_mode(&md->bl)&MD_CANMOVE && md->state.state == MS_WALK)
		range++;
	if(distance(md->bl.x,md->bl.y,tbl->x,tbl->y) > range)
		return 0;
	if(battle_config.monster_attack_direction_change)
		md->dir=map_calc_dir(&md->bl, tbl->x,tbl->y );	// 向き設定

	md->state.skillstate=MSS_ATTACK;
	if( mobskill_use(md,tick,-2) )	// スキル使用
		return 0;

	md->target_lv = battle_weapon_attack(&md->bl,tbl,tick,0);

	if(!(battle_config.monster_cloak_check_type&2) && md->sc_data[SC_CLOAKING].timer != -1)
		status_change_end(&md->bl,SC_CLOAKING,-1);

	//Mobs can't move if they can't attack neither. But since the attack delay is so hideously long for some mobs,
	//the can't move is 1/4th of the can't attack tick. [Skotlex]
	md->attackabletime = status_get_adelay(&md->bl);
	md->canmove_tick = tick + md->attackabletime/4;
	md->attackabletime += tick;

	md->timer=add_timer(md->attackabletime,mob_timer,md->bl.id,0);
	md->state.state=MS_ATTACK;

	return 0;
}


/*==========================================
 * The attack of PC which is attacking id is stopped.
 * The callback function of clif_foreachclient
 *------------------------------------------
 */
int mob_stopattacked(struct map_session_data *sd,va_list ap)
{
	int id;

	nullpo_retr(0, sd);
	nullpo_retr(0, ap);

	id=va_arg(ap,int);
	if(sd->attacktarget==id)
		pc_stopattack(sd);
	return 0;
}
/*==========================================
 * The timer in which the mob's states changes
 *------------------------------------------
 */
int mob_changestate(struct mob_data *md,int state,int type)
{
	unsigned int tick;
	int i;

	nullpo_retr(0, md);

	if(md->timer != -1)
		delete_timer(md->timer,mob_timer);
	md->timer=-1;
	md->state.state=state;

	switch(state){
	case MS_WALK:
		if((i=calc_next_walk_step(md))>0){
			i = i>>2;
			md->timer=add_timer(gettick()+i,mob_timer,md->bl.id,0);
		}
		else
			md->state.state=MS_IDLE;
		break;
	case MS_ATTACK:
		tick = gettick();
		i=DIFF_TICK(md->attackabletime,tick);
		if(i>0 && i<2000)
			md->timer=add_timer(md->attackabletime,mob_timer,md->bl.id,0);
		else if(type) {
			md->attackabletime = tick + status_get_amotion(&md->bl);
			md->timer=add_timer(md->attackabletime,mob_timer,md->bl.id,0);
		}
		else {
			md->attackabletime = tick + 1;
			md->timer=add_timer(md->attackabletime,mob_timer,md->bl.id,0);
		}
		break;
	case MS_DELAY:
		md->timer=add_timer(gettick()+type,mob_timer,md->bl.id,0);
		break;
	case MS_DEAD:
		skill_castcancel(&md->bl,0);
//		mobskill_deltimer(md);
		md->state.skillstate=MSS_DEAD;
		md->last_deadtime=gettick();
		// Since it died, all aggressors' attack to this mob is stopped.
		clif_foreachclient(mob_stopattacked,md->bl.id);
		skill_unit_move(&md->bl,gettick(),4);
		status_change_clear(&md->bl,2);	// ステータス異常を解除する
		skill_clear_unitgroup(&md->bl);	// 全てのスキルユニットグループを削除する
		skill_cleartimerskill(&md->bl);
		if(md->deletetimer!=-1)
			delete_timer(md->deletetimer,mob_timer_delete);
		md->deletetimer=-1;
		md->hp = md->target_id = md->attacked_id = md->attacked_count = 0;
		md->state.targettype = NONE_ATTACKABLE;
		break;
	}

	return 0;
}

/*==========================================
 * timer processing of mob (timer function)
 * It branches to a walk and an attack.
 *------------------------------------------
 */
static int mob_timer(int tid,unsigned int tick,int id,int data)
{
	struct mob_data *md;
	struct block_list *bl;

	if( (bl=map_id2bl(id)) == NULL ){ //攻撃してきた敵がもういないのは正常のようだ
		return 1;
	}

	if(!bl || !bl->type || bl->type!=BL_MOB)
		return 1;

	nullpo_retr(1, md=(struct mob_data*)bl);

	if(md->timer != tid){
		if(battle_config.error_log)
			ShowError("mob_timer %d != %d\n",md->timer,tid);
		return 0;
	}
	md->timer=-1;
	if(md->bl.prev == NULL || md->state.state == MS_DEAD)
		return 1;

	map_freeblock_lock();
	switch(md->state.state){
	case MS_WALK:
		mob_walk(md,tick,data);
		break;
	case MS_ATTACK:
		mob_attack(md,tick,data);
		break;
	case MS_DELAY:
		mob_changestate(md,MS_IDLE,0);
		break;
	default:
		if(battle_config.error_log)
			ShowError("mob_timer : %d ?\n",md->state.state);
		break;
	}

	if (md->timer == -1)
		mob_changestate(md,MS_WALK,0);

	map_freeblock_unlock();
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int mob_walktoxy_sub(struct mob_data *md)
{
	struct walkpath_data wpd;
	int x,y;
	static int dirx[8]={0,-1,-1,-1,0,1,1,1};
	static int diry[8]={1,1,0,-1,-1,-1,0,1};

	nullpo_retr(0, md);

	memset(&wpd, 0, sizeof(wpd));

	if(path_search(&wpd,md->bl.m,md->bl.x,md->bl.y,md->to_x,md->to_y,md->state.walk_easy))
		return 1;
	if (wpd.path[0] >= 8)
		return 1;	
	x = md->bl.x+dirx[wpd.path[0]];
	y = md->bl.y+diry[wpd.path[0]];
	if (map_getcell(md->bl.m,x,y,CELL_CHKBASILICA) && !(status_get_mode(&md->bl)&MD_BOSS)) {
		md->state.change_walk_target=0;
		return 1;
	}

	memcpy(&md->walkpath,&wpd,sizeof(wpd));

	md->state.change_walk_target=0;
	mob_changestate(md,MS_WALK,0);
	clif_movemob(md);

	return 0;
}

/*==========================================
 * mob move start
 *------------------------------------------
 */
int mob_walktoxy(struct mob_data *md,int x,int y,int easy)
{
	struct walkpath_data wpd;

	nullpo_retr(0, md);

	if(md->bl.prev == NULL || md->state.state == MS_DEAD) //Just-in-case check to prevent dead mobs from moving. [Skotlex]
		return 1;
	
	if(md->state.state == MS_WALK && path_search(&wpd,md->bl.m,md->bl.x,md->bl.y,x,y,easy) )
		return 1;

	md->state.walk_easy = easy;
	md->to_x=x;
	md->to_y=y;
	if(md->state.state == MS_WALK)
		md->state.change_walk_target=1;
	else
		return mob_walktoxy_sub(md);

	return 0;
}

/*==========================================
 * mob spawn with delay (timer function)
 *------------------------------------------
 */
static int mob_delayspawn(int tid, unsigned int tick, int m, int n)
{
	mob_spawn(m);
	return 0;
}

/*==========================================
 * spawn timing calculation
 *------------------------------------------
 */
int mob_setdelayspawn(int id)
{
	unsigned int spawntime, spawntime1, spawntime2, spawntime3;
	struct mob_data *md;
	struct block_list *bl;

	if ((bl = map_id2bl(id)) == NULL || bl->type != BL_MOB)
		return -1;
	nullpo_retr(-1, md = (struct mob_data*)bl);

	// Processing of MOB which is not revitalized
	if (md->spawndelay1 == -1 && md->spawndelay2 == -1 && md->n == 0) {
		if (md->lootitem) {
			aFree(md->lootitem);
			md->lootitem = NULL;
		}
		if (md->guardian_data)
		{	
			if (md->guardian_data->number < MAX_GUARDIANS)
				md->guardian_data->castle->guardian[md->guardian_data->number].id = 0;
			aFree (md->guardian_data);
			md->guardian_data = NULL;
		}
		map_deliddb(&md->bl);
		map_delblock(bl); //In case it wasn't done before invoking the function.
		map_freeblock(bl);
		return 0;
	}

	spawntime1 = md->last_spawntime + md->spawndelay1;
	spawntime2 = md->last_deadtime + md->spawndelay2;
	spawntime3 = gettick() + 5000 + rand()%5000; //Lupus
	// spawntime = max(spawntime1,spawntime2,spawntime3);
	if (DIFF_TICK(spawntime1, spawntime2) > 0)
		spawntime = spawntime1;
	else
		spawntime = spawntime2;
	if (DIFF_TICK(spawntime3, spawntime) > 0)
		spawntime = spawntime3;

	add_timer(spawntime, mob_delayspawn, id, 0);
	return 0;
}

/*==========================================
 * Mob spawning. Initialization is also variously here.
 *------------------------------------------
 */
int mob_spawn (int id)
{
	int x, y, i = 0;
	unsigned int c, tick = gettick();
	struct mob_data *md;
	struct block_list *bl;

	if ((bl = map_id2bl(id)) == NULL || bl->type != BL_MOB)
		return -1;
	nullpo_retr(-1, md = (struct mob_data*)bl);

	md->last_spawntime = tick;
	if (md->bl.prev != NULL)
		map_delblock(&md->bl);
	else {
		if(md->class_ != md->base_class){	// クラスチェンジしたMob
			md->class_ = md->base_class;
			md->db = mob_db(md->base_class);
			memcpy(md->name,md->db->jname,NAME_LENGTH);
			md->speed=md->db->speed;
		}
	}
	md->bl.m = md->m;
	do {
		if (md->x0 == 0 && md->y0 == 0) {
			x = rand()%(map[md->bl.m].xs-2)+1;
			y = rand()%(map[md->bl.m].ys-2)+1;
		} else {
			x = md->x0+rand()%(md->xs+1)-md->xs/2;
			y = md->y0+rand()%(md->ys+1)-md->ys/2;
		}
		i++;
	} while(map_getcell(md->bl.m,x,y,CELL_CHKNOPASS) && i < 50);

	if (i >= 50) {
		if (md->spawndelay1 != -1 || md->spawndelay2 == -1)
			// retry again later
			add_timer(tick+5000,mob_delayspawn,id,0);
		return 1;
	}

	md->to_x = md->bl.x = x;
	md->to_y = md->bl.y = y;
	md->dir = 0;
	md->target_dir = 0;

	memset(&md->state, 0, sizeof(md->state));
	md->attacked_id = 0;
	md->attacked_count = 0;
	md->target_id = 0;
	md->move_fail_count = 0;

	if (!md->speed)
		md->speed = md->db->speed;
	md->def_ele = md->db->element;

	if (!md->level) // [Valaris]
		md->level=md->db->lv;

	md->master_id = 0;
	md->master_dist = 0;

	md->state.state = MS_IDLE;
	md->state.skillstate = MSS_IDLE;
	md->timer = -1;
	md->last_thinktime = tick;
	md->next_walktime = tick+rand()%50+5000;
	md->attackabletime = tick;
	md->canmove_tick = tick;
	md->last_linktime = tick;

	/* Guardians should be spawned using mob_spawn_guardian! [Skotlex]
	 * and the Emperium is spawned using mob_once_spawn.
	md->guild_id = 0;
	if (md->class_ >= 1285 && md->class_ <= 1288) {
		struct guild_castle *gc=guild_mapname2gc(map[md->bl.m].name);
		if(gc)
			md->guild_id = gc->guild_id;
	}
	*/

	md->deletetimer = -1;

	md->skilltimer = -1;
	for (i = 0, c = tick-1000*3600*10; i < MAX_MOBSKILL; i++)
		md->skilldelay[i] = c;
	md->skillid = 0;
	md->skilllv = 0;

	memset(md->dmglog, 0, sizeof(md->dmglog));
	if (md->lootitem)
		memset(md->lootitem, 0, sizeof(md->lootitem));
	md->lootitem_count = 0;

	for (i = 0; i < MAX_MOBSKILLTIMERSKILL; i++)
		md->skilltimerskill[i].timer = -1;

	for (i = 0; i < MAX_STATUSCHANGE; i++) {
		md->sc_data[i].timer = -1;
		md->sc_data[i].val1 = md->sc_data[i].val2 = md->sc_data[i].val3 = md->sc_data[i].val4 = 0;
	}
	md->sc_count = 0;
	md->opt1 = md->opt2 = md->opt3 = md->option = 0;

	memset(md->skillunit, 0, sizeof(md->skillunit));
	memset(md->skillunittick, 0, sizeof(md->skillunittick));

	md->max_hp = md->db->max_hp;
	if(md->size==1) // change for sized monsters [Valaris]
		md->max_hp/=2;
	else if(md->size==2)
		md->max_hp*=2;
	md->hp = md->max_hp;

	map_addblock(&md->bl);
	skill_unit_move(&md->bl,tick,1);

	clif_spawnmob(md);

	return 0;
}

/*==========================================
 * Distance calculation between two points
 *------------------------------------------
 */
static int distance(int x0,int y0,int x1,int y1)
{
	int dx,dy;

	dx=abs(x0-x1);
	dy=abs(y0-y1);
	return dx>dy ? dx : dy;
}

/*==========================================
 * The stop of MOB's attack
 *------------------------------------------
 */
int mob_stopattack(struct mob_data *md)
{
	md->target_id = 0;
	md->state.targettype = NONE_ATTACKABLE;
	md->attacked_id = 0;
	md->attacked_count = 0;
	return 0;
}
/*==========================================
 * The stop of MOB's walking
 *------------------------------------------
 */
int mob_stop_walking(struct mob_data *md,int type)
{
	nullpo_retr(0, md);

	if(md->state.state == MS_WALK || md->state.state == MS_IDLE) {
		int dx=0,dy=0;

		md->walkpath.path_len=0;
		if(type&4){
			dx=md->to_x-md->bl.x;
			if(dx<0)
				dx=-1;
			else if(dx>0)
				dx=1;
			dy=md->to_y-md->bl.y;
			if(dy<0)
				dy=-1;
			else if(dy>0)
				dy=1;
		}
		md->to_x=md->bl.x+dx;
		md->to_y=md->bl.y+dy;
		if(dx!=0 || dy!=0){
			mob_walktoxy_sub(md);
			return 0;
		}
		mob_changestate(md,MS_IDLE,0);
	}
	if(type&0x01)
		clif_fixmobpos(md);

	return 0;
}

/*==========================================
 * Reachability to a Specification ID existence place
 *------------------------------------------
 */
int mob_can_reach(struct mob_data *md,struct block_list *bl,int range)
{
	int dx,dy;
	struct walkpath_data wpd;
	int i, easy = (battle_config.mob_ai&1?0:1);

	nullpo_retr(0, md);
	nullpo_retr(0, bl);

	dx=abs(bl->x - md->bl.x);
	dy=abs(bl->y - md->bl.y);

	if( md->bl.m != bl->m)	// 違うャbプ
		return 0;

	if( range>0 && range < ((dx>dy)?dx:dy) )	// 遠すぎる
		return 0;

	if( md->bl.x==bl->x && md->bl.y==bl->y )	// 同じマス
		return 1;

	// Obstacle judging
	wpd.path_len=0;
	wpd.path_pos=0;
	wpd.path_half=0;
	if(path_search(&wpd,md->bl.m,md->bl.x,md->bl.y,bl->x,bl->y,easy)!=-1)
		return 1;

	if(bl->type!=BL_PC && bl->type!=BL_MOB)
		return 0;

	// It judges whether it can adjoin or not.
	dx=(dx>0)?1:((dx<0)?-1:0);
	dy=(dy>0)?1:((dy<0)?-1:0);
	if(path_search(&wpd,md->bl.m,md->bl.x,md->bl.y,bl->x-dx,bl->y-dy,easy)!=-1)
		return 1;
	for(i=0;i<9;i++){
		if(path_search(&wpd,md->bl.m,md->bl.x,md->bl.y,bl->x-1+i/3,bl->y-1+i%3,easy)!=-1)
			return 1;
	}
	return 0;
}

/*==========================================
 * Determination for an attack of a monster
 *------------------------------------------
 */
int mob_target(struct mob_data *md,struct block_list *bl,int dist)
{
	int mode;

	nullpo_retr(0, md);
	nullpo_retr(0, bl);

	if(!md->mode)
		mode=md->db->mode;
	else
		mode=md->mode;

	// Nothing will be carried out if there is no mind of changing TAGE by TAGE ending.
	if( (md->target_id > 0 && md->state.targettype == ATTACKABLE) && (!(mode&MD_CHANGETARGET) || rand()%100>25) &&
		// if the monster was provoked ignore the above rule [celest]
		!(md->state.provoke_flag && md->state.provoke_flag == bl->id))
		return 0;

	if(!status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	md->target_id = bl->id;	// Since there was no disturbance, it locks on to target.
	if(bl->type == BL_PC || bl->type == BL_MOB)
		md->state.targettype = ATTACKABLE;
	else
		md->state.targettype = NONE_ATTACKABLE;
	if (md->state.provoke_flag)
		md->state.provoke_flag = 0;
	md->min_chase=dist+md->db->range2;
	if(md->min_chase>26)
		md->min_chase=26;
	return 0;
}

/*==========================================
 * The ?? routine of an active monster
 *------------------------------------------
 */
static int mob_ai_sub_hard_activesearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int dist,*pcc;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=va_arg(ap,struct mob_data *));
	nullpo_retr(0, pcc=va_arg(ap,int *));
	nullpo_retr(0, tick=va_arg(ap,unsigned int));

	//If can't seek yet, not an enemy, or you can't attack it, skip.
	if (battle_check_target(&md->bl,bl,BCT_ENEMY)<=0 || !status_check_skilluse(&md->bl, bl, 0, 0))
		return 0;

	switch (bl->type)
	{
	case BL_PC:
	case BL_MOB:
		if((dist=distance(md->bl.x,md->bl.y,bl->x,bl->y)) < md->db->range2
			&& (md->db->range > 6 || mob_can_reach(md,bl,dist)) && rand()%1000<1000/(++(*pcc)))
		{
			md->target_id=bl->id;
			md->state.targettype = ATTACKABLE;
			md->min_chase= md->db->range3;
			return 1;
		}
		break;
	}
	return 0;
}

/*==========================================
 * loot monster item search
 *------------------------------------------
 */
static int mob_ai_sub_hard_lootsearch(struct block_list *bl,va_list ap)
{
	struct mob_data* md;
	int dist,*itc;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=va_arg(ap,struct mob_data *));
	nullpo_retr(0, itc=va_arg(ap,int *));

	if(!md->lootitem || (battle_config.monster_loot_type == 1 && md->lootitem_count >= LOOTITEM_SIZE))
		return 0;
	
	if((dist=distance(md->bl.x,md->bl.y,bl->x,bl->y))<md->db->range2 &&
		mob_can_reach(md,bl,dist) && rand()%1000<1000/(++(*itc)))
	{	// It is made a probability, such as within the limits PC.
		md->target_id=bl->id;
		md->state.targettype = NONE_ATTACKABLE;
		md->min_chase=md->db->range3;
	}
	return 0;
}

/*==========================================
 * Links nearby mobs (supportive mobs)
 *------------------------------------------
 */
static int mob_ai_sub_hard_linksearch(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int class_;
	struct block_list *target;
	unsigned int tick;
	
	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	md=(struct mob_data *)bl;
	class_ = va_arg(ap, int);
	target = va_arg(ap, struct block_list *);
	tick=va_arg(ap, unsigned int);

	if (md->class_ == class_ && DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME
		&& (!md->target_id || md->state.targettype == NONE_ATTACKABLE))
	{
		md->last_linktime = tick;
		if( mob_can_reach(md,target,md->db->range2) ){	// Reachability judging
			md->target_id = target->id;
			md->attacked_count = 0;
			md->state.targettype = ATTACKABLE;
			md->min_chase=md->db->range3;
			return 1;
		}
	}

	return 0;
}
/*==========================================
 * Processing of slave monsters
 *------------------------------------------
 */
static int mob_ai_sub_hard_slavemob(struct mob_data *md,unsigned int tick)
{
	struct mob_data *mmd=NULL;
	struct block_list *bl;
	int old_dist;

	nullpo_retr(0, md);

	if((bl=map_id2bl(md->master_id)) != NULL && bl->type == BL_MOB)
		mmd=(struct mob_data *)bl;

	if (!bl || status_isdead(bl)) {	//主が死亡しているか見つからない
		if(md->state.special_mob_ai>0)
			mob_timer_delete(0, 0, md->bl.id, 0);
		else
			mob_damage(NULL,md,md->hp,0,0);
		return 0;
	}

	if(status_get_mode(&md->bl)&MD_CANMOVE)
	{	//If the mob can move, follow around. [Check by Skotlex]
		
		if(bl->m != md->bl.m || md->master_dist > 30)
		{	// Since it is not in the same map (or is way to far), just warp it
			mob_warp(md,bl->m,bl->x,bl->y,3);
			return 0;
		}

		// Distance with between slave and master is measured.
		old_dist=md->master_dist;
		md->master_dist=distance(md->bl.x,md->bl.y,bl->x,bl->y);

		// Since the master was in near immediately before, teleport is carried out and it pursues.
		if(old_dist<10 && md->master_dist>18){
			mob_warp(md,-1,bl->x,bl->y,3);
			return 0;
		}

		// Although there is the master, since it is somewhat far, it approaches.
		if((!md->target_id || md->state.targettype == NONE_ATTACKABLE) && mob_can_move(md) &&
			md->master_dist<md->db->range3 && (md->walkpath.path_pos>=md->walkpath.path_len || md->walkpath.path_len==0)){
			int i=0,dx,dy,ret;
			if(md->master_dist>AREA_SIZE/2 && DIFF_TICK(md->next_walktime,tick)<0) {
				do {
					if(i<=5){
						dx=bl->x - md->bl.x;
						dy=bl->y - md->bl.y;
						if(dx<0) dx+=(rand()%-dx);
						else if(dx>0) dx-=(rand()%dx);
						if(dy<0) dy+=(rand()%-dy);
						else if(dy>0) dy-=(rand()%dy);
					}else{
						dx=bl->x - md->bl.x + rand()%11- 5;
						dy=bl->y - md->bl.y + rand()%11- 5;
					}

					ret=mob_walktoxy(md,md->bl.x+dx,md->bl.y+dy,0);
					i++;
				} while(ret && i<10);
				md->next_walktime=tick+500;
			}
			/* Else do nothing. Let mob_random_walk take care of this. [Skotlex]
			else {
				do {
					dx = rand()%(AREA_SIZE/2 +1) - AREA_SIZE/2;
					dy = rand()%(AREA_SIZE/2 +1) - AREA_SIZE/2;
					if( dx == 0 && dy == 0) {
						dx = (rand()%1)? 1:-1;
						dy = (rand()%1)? 1:-1;
					}

					ret=mob_walktoxy(md,bl->x+dx,bl->y+dy,(battle_config.mob_ai&1?1:0));
					i++;
				} while(ret && i<10);
			}
			*/
		}
	}
	
	// There is the master, the master locks a target and he does not lock.
	if( mmd && (mmd->target_id>0 && mmd->state.targettype == ATTACKABLE) && (!md->target_id || md->state.targettype == NONE_ATTACKABLE) ){
		struct map_session_data *sd=map_id2sd(mmd->target_id);
		if(sd && status_check_skilluse(&md->bl, &sd->bl, 0, 0)) {
			md->target_id=sd->bl.id;
			md->state.targettype = ATTACKABLE;
			md->min_chase=md->db->range2+distance(md->bl.x,md->bl.y,sd->bl.x,sd->bl.y);
		}
	}

	return 0;
}

/*==========================================
 * A lock of target is stopped and mob moves to a standby state.
 *------------------------------------------
 */
int mob_unlocktarget(struct mob_data *md,int tick)
{
	nullpo_retr(0, md);

	md->target_id=0;
	md->state.targettype = NONE_ATTACKABLE;
	md->state.skillstate=MSS_IDLE;
	md->next_walktime=tick+rand()%3000;
	return 0;
}
/*==========================================
 * Random walk
 *------------------------------------------
 */
int mob_randomwalk(struct mob_data *md,int tick)
{
	const int retrycount=20;
	int speed;

	nullpo_retr(0, md);

	speed=status_get_speed(&md->bl);
	if(DIFF_TICK(md->next_walktime,tick)<0){
		int i,x,y,c,d=12-md->move_fail_count;
		int mask[8][2] = {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};
		if(d<5) d=5;
		for(i=0;i<retrycount;i++){	// Search of a movable place
			int r=rand();
			x=r%(d*2+1)-d;
			y=r/(d*2+1)%(d*2+1)-d;
			if (md->target_dir){
				if (x<0) x=0-x;
				if (y<0) y=0-y;
				x *= mask[md->target_dir-1][0];
				y *= mask[md->target_dir-1][1];
			}
			x+=md->bl.x;
			y+=md->bl.y;

			if((map_getcell(md->bl.m,x,y,CELL_CHKPASS)) && mob_walktoxy(md,x,y,1)==0){
				md->move_fail_count=0;
				break;
			}
			if(i+1>=retrycount){
				md->move_fail_count++;
				md->target_dir = 0;
				if(md->move_fail_count>1000){
					if(battle_config.error_log)
						ShowWarning("MOB cant move. random spawn %d, class = %d\n",md->bl.id,md->class_);
					md->move_fail_count=0;
					mob_spawn(md->bl.id);
				}
			}
		}
		for(i=c=0;i<md->walkpath.path_len;i++){	// The next walk start time is calculated.
			if(md->walkpath.path[i]&1)
				c+=speed*14/10;
			else
				c+=speed;
		}
		md->next_walktime = tick+rand()%3000+3000+c;
		md->state.skillstate=MSS_WALK;
		return 1;
	}
	return 0;
}

/*==========================================
 * AI of MOB whose is near a Player
 *------------------------------------------
 */
static int mob_ai_sub_hard(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	struct block_list *tbl = NULL, *abl = NULL;
	unsigned int tick;
	int i, dx, dy, dist;
	int attack_type = 0;
	int mode;
	int search_size = AREA_SIZE*2;
	int blind_flag = 0;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	md = (struct mob_data*)bl;
	tick = va_arg(ap, unsigned int);

	if(md->bl.prev == NULL || md->state.state == MS_DEAD)
		return 1;
		
	if (DIFF_TICK(tick, md->last_thinktime) < MIN_MOBTHINKTIME)
		return 0;
	md->last_thinktime = tick;

	if (md->skilltimer != -1){	// Casting skill, or has died
		if (DIFF_TICK (tick, md->next_walktime) > MIN_MOBTHINKTIME)
			md->next_walktime = tick;
		return 0;
	}

	// Abnormalities
	if((md->opt1 > 0 && md->opt1 != 6) || md->state.state == MS_DELAY || md->sc_data[SC_BLADESTOP].timer != -1)
		return 0;

	if (md->sc_data && md->sc_data[SC_BLIND].timer != -1)
		blind_flag = 1;

	if (!md->mode)
		mode = md->db->mode;
	else
		mode = md->mode;

	if (md->attacked_id && mode&MD_ASSIST && DIFF_TICK(md->last_linktime, gettick()) < MIN_MOBLINKTIME)
	{	// Link monster/ if target is not dead [Skotlex]
		abl = map_id2bl(md->attacked_id);
		unsigned int tick = gettick();
		md->last_linktime = tick;
		if (abl && !status_isdead(abl))
			map_foreachinarea(mob_ai_sub_hard_linksearch, md->bl.m,
				md->bl.x-md->db->range2, md->bl.y-md->db->range2,
				md->bl.x+md->db->range2, md->bl.y+md->db->range2,
				BL_MOB, md->class_, abl, tick);
		else
		{
			abl = NULL;
			md->attacked_id = 0;
		}
	}

	if (md->target_id)
	{	//Check validity of current target. [Skotlex]
		tbl = map_id2bl(md->target_id);
		if (!tbl || tbl->m != md->bl.m || !status_check_skilluse(&md->bl, tbl, 0, 0))
		{	//Unlock current target.
			if (md->state.skillstate == MSS_CHASE)
			{	//Confused!
				mob_stop_walking(md, 0);
				clif_emotion(&md->bl, 1);
			}
			mob_unlocktarget(md, tick);
		}
	}
			
	// It checks to see it was attacked first (if active, it is target change at 25% of probability).
	if (md->attacked_id && mode&MD_CANATTACK && md->attacked_id != md->target_id &&
		(!md->target_id || md->state.targettype == NONE_ATTACKABLE || (mode&MD_CHANGETARGET && rand()%100 < 25)))
	{
		if (!abl) //Avoid seeking it if we had it from before (friend scan).
			abl = map_id2bl(md->attacked_id);
		if (!abl) //Target gone.
			md->attacked_id = 0;
		else {
			if (md->bl.m != abl->m || abl->prev == NULL ||
				(dist = distance(md->bl.x, md->bl.y, abl->x, abl->y)) >= 32 ||
				battle_check_target(bl, abl, BCT_ENEMY) <= 0 ||
				(battle_config.mob_ai&2 && !status_check_skilluse(bl, abl, 0, 0)) ||
				!mob_can_reach(md, abl, dist+2)) //Some more cells of grace...
			{	//Can't attack back
				if (md->attacked_count++ > 3) {
					if (mobskill_use(md, tick, MSC_RUDEATTACKED) == 0 &&
						mode&MD_CANMOVE && mob_can_move(md))
					{
						int dist = rand() % 10 + 1;//後退する距離
						int dir = map_calc_dir(abl, bl->x, bl->y);
						int mask[8][2] = {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};
						mob_walktoxy(md, md->bl.x + dist * mask[dir][0], md->bl.y + dist * mask[dir][1], 0);
						md->next_walktime = tick + 500;
					}
					md->attacked_id = 0;
				}
			} else if (!(battle_config.mob_ai&2) && !status_check_skilluse(bl, abl, 0, 0)) {
				//Can't attack back, but didn't invoke a rude attacked skill...
				//This is "dumb" ai, so do nothing.... (that's how players want it) [Skotlex]
				/*
				int dist = rand() % 10 + 1;
				int dir = map_calc_dir(abl, bl->x, bl->y);
				int mask[8][2] = {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};
				mob_walktoxy(md, md->bl.x + dist * mask[dir][0], md->bl.y + dist * mask[dir][1], 0);
				md->next_walktime = tick + 500;
				md->attacked_id = 0;
				*/
			} else if (blind_flag && dist > 2 && DIFF_TICK(tick,md->next_walktime) < 0) { //Blinded, but can reach 
				if (!md->target_id)
				{	//Attempt to follow new target
					md->attacked_id = 0;
					if (mode&MD_CANMOVE && mob_can_move(md)) {	// why is it moving to the target when the mob can't see the player? o.o
						dx = abl->x - md->bl.x;
						dy = abl->y - md->bl.y;
						md->next_walktime = tick + 1000;
						mob_walktoxy(md, md->bl.x+dx, md->bl.y+dy, 0);
					}
				}
			} else { //Attackable
				if (!tbl || dist < md->db->range || distance(md->bl.x, md->bl.y, tbl->x, tbl->y) > dist
					|| battle_gettarget(tbl) != md->bl.id)
				{	//Change if the new target is closer than the actual one
					//or if the previous target is not attacking the mob. [Skotlex]
					md->target_id = md->attacked_id; // set target
					md->state.targettype = ATTACKABLE;
					attack_type = 1;
					md->attacked_id = md->attacked_count = 0;
					md->min_chase = dist + md->db->range2;
					if (md->min_chase > 26)
						md->min_chase = 26;
					tbl = abl; //Set the new target
				}
			}
		}
	}

	// Processing of slave monster
	if (md->master_id > 0)
		mob_ai_sub_hard_slavemob(md, tick);

	// Scan area for targets (aggressive mob)
	if ((!md->target_id || md->state.targettype == NONE_ATTACKABLE) && mode&MD_AGGRESSIVE &&
		battle_config.monster_active_enable) {
		i = 0;
		search_size = (blind_flag) ? 3 : md->db->range2;
		if (md->state.special_mob_ai)
			map_foreachinarea (mob_ai_sub_hard_activesearch, md->bl.m,
					md->bl.x-search_size, md->bl.y-search_size,
					md->bl.x+search_size, md->bl.y+search_size,
					0, md, &i, tick);
		else map_foreachinarea (mob_ai_sub_hard_activesearch, md->bl.m,
					md->bl.x-search_size,md->bl.y-search_size,
					md->bl.x+search_size,md->bl.y+search_size,
					BL_PC, md, &i, tick);
	}

	// Scan area for items to loot, avoid trying to loot of the mob is full and can't consume the items.
	if (!md->target_id && mode&MD_LOOTER && md->lootitem && 
		(md->lootitem_count < LOOTITEM_SIZE || battle_config.monster_loot_type != 1))
	{
		i = 0;
		search_size = (blind_flag) ? 3 : md->db->range2;
		map_foreachinarea (mob_ai_sub_hard_lootsearch, md->bl.m,
					md->bl.x-search_size, md->bl.y-search_size,
					md->bl.x+search_size, md->bl.y+search_size,
					BL_ITEM, md, &i);
	}

	if (tbl)
	{	//Target exists, attack or loot as applicable.
		if (tbl->type != BL_ITEM)
		{	//Attempt to attack.
			//At this point we know the target is attackable, we just gotta check if the range matches.
			if (blind_flag && DIFF_TICK(tick,md->next_walktime) < 0 && distance(md->bl.x, md->bl.y, tbl->x, tbl->y) > 2)
			{	//Run towards the enemy when out of range?
				md->target_id = 0;
				md->attacked_id = 0;
				md->state.targettype = NONE_ATTACKABLE;
				if (!(mode & MD_CANMOVE) || !mob_can_move(md))
					return 0;
				dx = tbl->x - md->bl.x;
				dy = tbl->y - md->bl.y;
				md->next_walktime = tick + 1000;
				mob_walktoxy(md, md->bl.x+dx, md->bl.y+dy, 0);
				return 0;
			}
			if (!battle_check_range (&md->bl, tbl, md->db->range))
			{	//Out of range...
				if (!(mode & MD_CANMOVE))
				{	//Can't chase.
					mob_unlocktarget(md,tick);
					return 0;
				}
				if (!mob_can_move(md)) //Wait until you can move?
					return 0;
				//Follow up
				md->state.skillstate = MSS_CHASE;
				mobskill_use (md, tick, -1);
				if (md->timer != -1 && md->state.state != MS_ATTACK &&
					(DIFF_TICK (md->next_walktime, tick) < 0 ||
					distance(md->to_x, md->to_y, tbl->x, tbl->y) < 2)) {
					return 0; //No need to follow, already doing it?
				}
				search_size = (blind_flag) ? 3 : ((md->min_chase > md->db->range2) ? md->min_chase : md->db->range2);
				if (!mob_can_reach(md, tbl, search_size))
				{	//Can't reach
					mob_unlocktarget(md,tick);
					return 0;
				}
				//Target reachable. Locate suitable spot to move to.
				i = 0;
				dx = tbl->x - md->bl.x;
				dy = tbl->y - md->bl.y;
				if (dx < 0) dx++;
				else if (dx > 0) dx--;
				if (dy < 0) dy++;
				else if (dy > 0) dy--;
				while (i < 5 && mob_walktoxy(md, md->bl.x + dx, md->bl.y + dy, 0))
				{	//Attempt to chase to nearby blocks
					dx = tbl->x - md->bl.x + rand()%3 - 1;
					dy = tbl->y - md->bl.y + rand()%3 - 1;
					i++;
				}
				if (i==5)
				{	//Failed? Try going away from the target before retrying.
					if (dx < 0) dx = 2;
					else if (dx > 0) dx = -2;
					if (dy < 0) dy = 2;
					else if (dy > 0) dy = -2;
				}
				md->next_walktime = tick + 500;
				mob_walktoxy (md, md->bl.x+dx, md->bl.y+dy, 0);
				return 0;
			}
			//Target within range, engage
			md->state.skillstate = MSS_ATTACK;
			if (md->state.state == MS_WALK)
				mob_stop_walking (md, 1);
			if (md->state.state == MS_ATTACK)
				return 0; //Ah, we are already attacking.
			mob_changestate(md, MS_ATTACK, attack_type);
			return 0;
		} else {	//Target is BL_ITEM, attempt loot.
			struct flooritem_data *fitem;
			
			if ((dist = distance(md->bl.x, md->bl.y, tbl->x, tbl->y)) >= md->min_chase || (blind_flag && dist >= 4))
			{	//Can't loot...
				mob_unlocktarget (md, tick);
				if (md->state.state == MS_WALK)
					mob_stop_walking(md,0);
				return 0;
			}
			if (dist)
			{	//Still not within loot range.
				if (!(mode & MD_CANMOVE))
				{	//A looter that can't move? Real smart.
					mob_unlocktarget(md,tick);
					return 0;
				}
				if (!mob_can_move(md))	// 動けない状態にある
					return 0;
				md->state.skillstate = MSS_LOOT;	// ルート時スキル使用
				mobskill_use(md, tick, -1);
				if (md->timer != -1 && md->state.state != MS_ATTACK &&
					(DIFF_TICK(md->next_walktime,tick) < 0 ||
					distance(md->to_x, md->to_y, tbl->x, tbl->y) <= 0))
				{	//Already on the way to looting.
					return 0;
				}
				md->next_walktime = tick + 500;
				dx = tbl->x - md->bl.x;
				dy = tbl->y - md->bl.y;
				if (mob_walktoxy(md, md->bl.x+dx, md->bl.y+dy, 0))
					mob_unlocktarget(md, tick); //Can't loot...
				return 0;
			}
			//Within looting range.
			if (md->state.state == MS_ATTACK)
				return 0; //Busy attacking?
			if (md->state.state == MS_WALK)
				mob_stop_walking(md,0);

			fitem = (struct flooritem_data *)tbl;
			if (md->lootitem_count < LOOTITEM_SIZE) {
				memcpy (&md->lootitem[md->lootitem_count++], &fitem->item_data, sizeof(md->lootitem[0]));
				if(log_config.pick > 0)	//Logs items, taken by (L)ooter Mobs [Lupus]
					log_pick((struct map_session_data*)md, "L", md->class_, md->lootitem[md->lootitem_count-1].nameid, md->lootitem[md->lootitem_count-1].amount, &md->lootitem[md->lootitem_count-1]);
			} else if (battle_config.monster_loot_type == 1) { //Can't loot, stuffed!
				mob_unlocktarget(md,tick);
				return 0;
			} else {	//Destroy first looted item...
				if (md->lootitem[0].card[0] == (short)0xff00)
					intif_delete_petdata( MakeDWord(md->lootitem[0].card[1],md->lootitem[0].card[2]) );
				for (i = 0; i < LOOTITEM_SIZE - 1; i++)
					memcpy (&md->lootitem[i], &md->lootitem[i+1], sizeof(md->lootitem[0]));
				memcpy (&md->lootitem[LOOTITEM_SIZE-1], &fitem->item_data, sizeof(md->lootitem[0]));
			}
			//Clear item.
			map_clearflooritem (tbl->id);
			mob_unlocktarget (md,tick);
			return 0;
		}
	}

	// When there's no target, it is idling.
	if (mobskill_use(md, tick, -1))
		return 0;

	// Nothing else to do... except random walking.
	if (mode&MD_CANMOVE && mob_can_move(md))
	{
		if (DIFF_TICK(md->next_walktime, tick) > 7000 &&
			(md->walkpath.path_len == 0 || md->walkpath.path_pos >= md->walkpath.path_len))
			md->next_walktime = tick + 3000 * rand() % 2000;
		// Random movement
		if (mob_randomwalk(md,tick))
			return 0;
	}

	// Since he has finished walking, it stands by.
	if (md->walkpath.path_len == 0 || md->walkpath.path_pos >= md->walkpath.path_len)
		md->state.skillstate = MSS_IDLE;
	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view (foreachclient)
 *------------------------------------------
 */
static int mob_ai_sub_foreachclient(struct map_session_data *sd,va_list ap)
{
	unsigned int tick;
	nullpo_retr(0, sd);
	nullpo_retr(0, ap);

	tick=va_arg(ap,unsigned int);
	map_foreachinarea(mob_ai_sub_hard,sd->bl.m,
					  sd->bl.x-AREA_SIZE*2,sd->bl.y-AREA_SIZE*2,
					  sd->bl.x+AREA_SIZE*2,sd->bl.y+AREA_SIZE*2,
					  BL_MOB,tick);

	return 0;
}

/*==========================================
 * Serious processing for mob in PC field of view   (interval timer function)
 *------------------------------------------
 */
static int mob_ai_hard(int tid,unsigned int tick,int id,int data)
{
	clif_foreachclient(mob_ai_sub_foreachclient,tick);

	return 0;
}

/*==========================================
 * Negligent mode MOB AI (PC is not in near)
 *------------------------------------------
 */
static int mob_ai_sub_lazy(void * key,void * data,va_list app)
{
	struct mob_data *md = (struct mob_data *)data;
	va_list ap;
	unsigned int tick;
	int mode;

	nullpo_retr(0, md);
	nullpo_retr(0, app);

	if(md->bl.type!=BL_MOB)
		return 0;

	ap = va_arg(app, va_list);
	tick=va_arg(ap,unsigned int);

	if(DIFF_TICK(tick,md->last_thinktime)<MIN_MOBTHINKTIME*10)
		return 0;
	md->last_thinktime=tick;

	if (md->bl.prev==NULL || md->state.state == MS_DEAD)
		return 1;

	if(md->skilltimer!=-1){
		if(DIFF_TICK(tick,md->next_walktime)>MIN_MOBTHINKTIME*10)
			md->next_walktime=tick;
		return 0;
	}

	// 取り巻きモンスターの処理（呼び戻しされた時）
	if (md->master_id > 0) {
		mob_ai_sub_hard_slavemob (md,tick);
		return 0;
	}

	mode = status_get_mode(&md->bl);
	if(DIFF_TICK(md->next_walktime,tick)<0 &&
		(mode&MD_CANMOVE) && mob_can_move(md) ){

		if( map[md->bl.m].users>0 ){
			// Since PC is in the same map, somewhat better negligent processing is carried out.

			// It sometimes moves.
			if(rand()%1000<MOB_LAZYMOVEPERC)
				mob_randomwalk(md,tick);

			// MOB which is not not the summons MOB but BOSS, either sometimes reboils.
			else if( rand()%1000<MOB_LAZYWARPPERC && md->x0<=0 && md->master_id!=0 &&
				!(mode&MD_BOSS))
				mob_spawn(md->bl.id);
		}else{
			// Since PC is not even in the same map, suitable processing is carried out even if it takes.

			// MOB which is not BOSS which is not Summons MOB, either -- a case -- sometimes -- leaping
			if( rand()%1000<MOB_LAZYWARPPERC && md->x0<=0 && md->master_id!=0 &&
				!(mode&MD_BOSS))
				mob_warp(md,-1,-1,-1,-1);
		}

		md->next_walktime = tick+rand()%10000+5000;
	}
	return 0;
}

/*==========================================
 * Negligent processing for mob outside PC field of view   (interval timer function)
 *------------------------------------------
 */
static int mob_ai_lazy(int tid,unsigned int tick,int id,int data)
{
	map_foreachiddb(mob_ai_sub_lazy,tick);

	return 0;
}

/*==========================================
 * The structure object for item drop with delay
 * Since it is only two being able to pass [ int ] a timer function
 * Data is put in and passed to this structure object.
 *------------------------------------------
 */
struct delay_item_drop {
	int m,x,y;
	struct item item_data;
	struct map_session_data *first_sd,*second_sd,*third_sd;
};

/*==========================================
 * Initializes the delay drop structure for mob-dropped items.
 *------------------------------------------
 */
static struct delay_item_drop* mob_setdropitem(int nameid, int qty, int m, int x, int y, 
	struct map_session_data* first_sd, struct map_session_data* second_sd, struct map_session_data* third_sd)
{
	struct delay_item_drop *drop = aCalloc(1, sizeof (struct delay_item_drop));
	drop->item_data.nameid = nameid;
	drop->item_data.amount = qty;
	drop->item_data.identify = !itemdb_isequip3(nameid);
	drop->m = m;
	drop->x = x;
	drop->y = y;
	drop->first_sd = first_sd;
	drop->second_sd = second_sd;
	drop->third_sd = third_sd;
	return drop;
};

/*==========================================
 * Initializes the delay drop structure for mob-looted items.
 *------------------------------------------
 */
static struct delay_item_drop* mob_setlootitem(struct item* item, int m, int x, int y,
	struct map_session_data* first_sd, struct map_session_data* second_sd, struct map_session_data* third_sd)
{
	struct delay_item_drop *drop = aCalloc(1, sizeof (struct delay_item_drop));
	memcpy(&drop->item_data, item, sizeof(struct item));
	drop->m = m;
	drop->x = x;
	drop->y = y;
	drop->first_sd = first_sd;
	drop->second_sd = second_sd;
	drop->third_sd = third_sd;
	return drop;
};

/*==========================================
 * item drop with delay (timer function)
 *------------------------------------------
 */
static int mob_delay_item_drop(int tid,unsigned int tick,int id,int data)
{
	struct delay_item_drop *ditem;
	ditem=(struct delay_item_drop *)id;

#if 0
	if (ditem->first_sd){
		if (ditem->first_sd->status.party_id > 0){
			struct party *p;
			if((p=party_search(ditem->first_sd->status.party_id)) && p->item){
				struct map_session_data *sd = NULL;
				int i;
				for (i = p->itemc + 1; i!=p->itemc; i++) {	// initialise counter and loop through the party
					if (i >= MAX_PARTY)
						i = 0;	// reset counter to 1st person in party so it'll stop when it reaches "itemc"
					if ((sd=p->member[i].sd)!=NULL && sd->bl.m == ditem->first_sd->bl.m)
						break;
				}
				if (sd){	// if an appropiate party member was found
					drop_flag = 0;
					if ((p->itemc++) >= MAX_PARTY)
						p->itemc = 0;
					if ((flag = pc_additem(ditem->first_sd,&temp_item,ditem->amount))) {
						clif_additem(ditem->first_sd,0,0,flag);
						drop_flag = 1;
					}
				}
			}
		}
	}
#endif
	map_addflooritem(&ditem->item_data,1,ditem->m,ditem->x,ditem->y,ditem->first_sd,ditem->second_sd,ditem->third_sd,0);
	aFree(ditem);
	return 0;
}

/*==========================================
 * Sets a timer to drop an item on the ground
 * Also performs logging and autoloot if enabled.
 *------------------------------------------
 * by [Skotlex]
 */
static void mob_item_drop(struct mob_data *md, unsigned int tick, struct delay_item_drop * ditem, int loot)
{
	if(log_config.pick > 0)
	{	//Logs items, dropped by mobs [Lupus]
		if (loot)
			log_pick((struct map_session_data*)md, "L", md->class_, ditem->item_data.nameid, -ditem->item_data.amount, &ditem->item_data);
		else
			log_pick((struct map_session_data*)md, "M", md->class_, ditem->item_data.nameid, -ditem->item_data.amount, NULL);
	}

	if (ditem->first_sd && ditem->first_sd->state.autoloot
		&& pc_additem(ditem->first_sd,&ditem->item_data,ditem->item_data.amount) == 0)
	{	//Autolooted.
		if(log_config.pick > 0)
			log_pick(ditem->first_sd, "P", 0, ditem->item_data.nameid, ditem->item_data.amount, &ditem->item_data);
		aFree(ditem);
	} else
		add_timer(tick, mob_delay_item_drop, (int)ditem, 0);
}

/*==========================================
 * mob data is erased.
 *------------------------------------------
 */
void mob_unload(struct mob_data *md)
{
	nullpo_retv(md);
	mob_remove_map(md, 0);
	map_deliddb(&md->bl);
	map_freeblock((struct block_list*)md);
}

int mob_remove_map(struct mob_data *md, int type)
{
	nullpo_retr(1, md);

	if(md->bl.prev == NULL)
		return 1;
	mob_changestate(md,MS_DEAD,0);
	clif_clearchar_area(&md->bl,type);
	map_delblock(&md->bl);
	if (md->lootitem){
		aFree(md->lootitem);
		md->lootitem = NULL;
	}
	if (md->guardian_data)
	{
		aFree(md->guardian_data);
		md->guardian_data = NULL;
	}
	return 0;
}
int mob_delete(struct mob_data *md)
{
	nullpo_retr(1, md);

	mob_remove_map(md, 1);
	if (mob_get_viewclass(md->class_) <= 1000)
		clif_clearchar_delay(gettick()+3000,&md->bl,0);
	mob_deleteslave(md);
	mob_setdelayspawn(md->bl.id);
	return 0;
}
int mob_timer_delete(int tid, unsigned int tick, int id, int data)
{
	struct mob_data *md=(struct mob_data *)map_id2bl(id);
	nullpo_retr(0, md);

//for Alchemist CANNIBALIZE [Lupus]
	mob_remove_map(md, 3);
	mob_setdelayspawn(md->bl.id);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int mob_deleteslave_sub(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	int id;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md = (struct mob_data *)bl);

	id=va_arg(ap,int);
	if(md->master_id > 0 && md->master_id == id )
		mob_damage(NULL,md,md->hp,0,1);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int mob_deleteslave(struct mob_data *md)
{
	nullpo_retr(0, md);

	map_foreachinarea(mob_deleteslave_sub, md->bl.m,
		0,0,map[md->bl.m].xs,map[md->bl.m].ys,
		BL_MOB,md->bl.id);
	return 0;
}

/*==========================================
 * It is the damage of sd to damage to md.
 *------------------------------------------
 */
int mob_damage(struct block_list *src,struct mob_data *md,int damage,int delay,int type)
{
	int i,count,minpos,mindmg;
	struct map_session_data *sd = NULL,*tmpsd[DAMAGELOG_SIZE];
	struct {
		struct party *p;
		int id,base_exp,job_exp,zeny;
	} pt[DAMAGELOG_SIZE];
	int pnum=0;
	int mvp_damage,max_hp;
	unsigned int tick = gettick();
	struct map_session_data *mvp_sd = NULL, *second_sd = NULL,*third_sd = NULL;
	struct block_list *master = NULL;
	double tdmg,temp;
	struct item item;
	int ret, mode;
	int drop_rate;
	int race;
	
	nullpo_retr(0, md); //srcはNULLで呼ばれる場合もあるので、他でチェック

	max_hp = status_get_max_hp(&md->bl);
	race = status_get_race(&md->bl);

	if(src && src->type == BL_PC) {
		sd = (struct map_session_data *)src;
		mvp_sd = sd;
	}

//	if(battle_config.battle_log)
//		printf("mob_damage %d %d %d\n",md->hp,max_hp,damage);
	if(md->bl.prev==NULL){
		if(battle_config.error_log==1)
			ShowError("mob_damage : BlockError!!\n");
		return 0;
	}

	if(md->state.state==MS_DEAD || md->hp<=0) {
		if(md->bl.prev != NULL) {
			mob_changestate(md,MS_DEAD,0);
			mobskill_use(md,tick,-1);	// It is skill at the time of death.
			clif_clearchar_area(&md->bl,1);
			map_delblock(&md->bl);
			mob_setdelayspawn(md->bl.id);
		}
		return 0;
	}

	if(delay)
	{
		mob_stop_walking(md,3);
		if (md->canmove_tick < tick)
			md->canmove_tick = tick + delay;
	}

	if(damage > max_hp>>2)
		skill_stop_dancing(&md->bl);

	if(md->hp > max_hp)
		md->hp = max_hp;

	// The amount of overkill rounds to hp.
	if(damage>md->hp)
		damage=md->hp;

	if(!(type&2)) {
		if(sd!=NULL){
			for(i=0,minpos=0,mindmg=0x7fffffff;i<DAMAGELOG_SIZE;i++){
				//if(md->dmglog[i].id==sd->bl.id)
				if(md->dmglog[i].id==sd->status.char_id)
					break;
				if(md->dmglog[i].id==0){
					minpos=i;
					mindmg=0;
				}
				else if(md->dmglog[i].dmg<mindmg){
					minpos=i;
					mindmg=md->dmglog[i].dmg;
				}
			}
			if(i<DAMAGELOG_SIZE)
				md->dmglog[i].dmg+=damage;
			else {
				//md->dmglog[minpos].id=sd->bl.id;
				md->dmglog[minpos].id=sd->status.char_id;
				md->dmglog[minpos].dmg=damage;
			}

			if(md->attacked_id <= 0 && md->state.special_mob_ai==0)
				md->attacked_id = sd->bl.id;
		}
		if(src && src->type == BL_PET && battle_config.pet_attack_exp_to_master==1) {
			struct pet_data *pd = (struct pet_data *)src;
			nullpo_retr(0, pd);
			for(i=0,minpos=0,mindmg=0x7fffffff;i<DAMAGELOG_SIZE;i++){
				//if(md->dmglog[i].id==pd->msd->bl.id)
				if(md->dmglog[i].id==pd->msd->status.char_id)
					break;
				if(md->dmglog[i].id==0){
					minpos=i;
					mindmg=0;
				}
				else if(md->dmglog[i].dmg<mindmg){
					minpos=i;
					mindmg=md->dmglog[i].dmg;
				}
			}
			if(i<DAMAGELOG_SIZE)
				md->dmglog[i].dmg+=(damage*battle_config.pet_attack_exp_rate)/100;
			else {
				//md->dmglog[minpos].id=pd->msd->bl.id;
				md->dmglog[minpos].id=pd->msd->status.char_id;
				md->dmglog[minpos].dmg=(damage*battle_config.pet_attack_exp_rate)/100;
			}
			//Let mobs retaliate against the pet's master [Skotlex]
			if(md->attacked_id <= 0 && md->state.special_mob_ai==0)
				md->attacked_id = pd->msd->bl.id;
		}
		if(src && src->type == BL_MOB)
		{
			struct mob_data *md2 = (struct mob_data *)src;
			struct map_session_data *msd = NULL;
			if (md2->state.special_mob_ai && md2->master_id)
				msd = map_id2sd(md2->master_id);
			if (msd)	
			{	//If master is not logged on, we just make his share of exp be lost. [Skotlex]
				for(i=0,minpos=0,mindmg=0x7fffffff;i<DAMAGELOG_SIZE;i++){
					if(md->dmglog[i].id==msd->status.char_id)
						break;
					if(md->dmglog[i].id==0){
						minpos=i;
						mindmg=0;
					}
					else if(md->dmglog[i].dmg<mindmg){
						minpos=i;
						mindmg=md->dmglog[i].dmg;
					}
				}
				if(i<DAMAGELOG_SIZE)
					md->dmglog[i].dmg+=damage;
				else {
					md->dmglog[minpos].id=msd->status.char_id;
					md->dmglog[minpos].dmg=damage;
				}
			}
			if(md->attacked_id <= 0)
			{	//Let players decide whether to retaliate versus the master or the mob. [Skotlex]
				if (md2->master_id && battle_config.retaliate_to_master)
					md->attacked_id = md2->master_id;
				else
					md->attacked_id = md2->bl.id;
			}
		}
	}

	md->hp-=damage;

	if(md->guardian_data && md->guardian_data->number < MAX_GUARDIANS) { // guardian hp update [Valaris] (updated by [Skotlex])
		if ((md->guardian_data->castle->guardian[md->guardian_data->number].hp = md->hp) <= 0)
		{
			guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
			guild_castledatasave(md->guardian_data->castle->castle_id, 18+md->guardian_data->number,0);
		}
	}	// end addition

	if(md->option&2 )
		status_change_end(&md->bl, SC_HIDING, -1);
	if(md->option&4 )
		status_change_end(&md->bl, SC_CLOAKING, -1);

	if(md->state.special_mob_ai == 2 &&	//スフィアーマイン
		src && md->master_id == src->id)
	{
		md->state.alchemist = 1;
		md->target_dir = map_calc_dir(src,md->bl.x,md->bl.y)+1;
		mobskill_use(md, tick, MSC_ALCHEMIST);
	}

/* Uncomment this to enable supportive mobs calling for support when attacked as well as during their AI. [Skotlex]
	mode = status_get_mode(&md->bl);
	if (src && status_get_mode(&md->bl) & MD_ASSIST && DIFF_TICK(md->last_linktime, gettick()) < MIN_MOBLINKTIME)
	{	// Link monster/ if target is not dead [Skotlex]
		unsigned int tick = gettick();
		md->last_linktime = tick;
		map_foreachinarea(mob_ai_sub_hard_linksearch, md->bl.m,
			md->bl.x-13, md->bl.y-13, md->bl.x+13, md->bl.y+13,
			BL_MOB, md->class_, src, tick);
	}
*/
	if (battle_config.show_mob_hp)
		clif_charnameack (0, &md->bl);
		
	if(md->hp > 0)
		return damage;

	// ----- ここから死亡処理 -----

	mode = status_get_mode(&md->bl); //Mode will be used for various checks regarding exp/drops.

	//changestate will clear all status effects, so we need to know if RICHMANKIM is in effect before then. [Skotlex]
	//I just recycled ret because it isn't used until much later and I didn't want to add a new variable for it.
	ret = (md->sc_data[SC_RICHMANKIM].timer != -1)?(25 + 11*sd->sc_data[SC_RICHMANKIM].val1):0;
	
	map_freeblock_lock();
	mob_changestate(md,MS_DEAD,0);
	mobskill_use(md,tick,-1);	// 死亡時スキル

	memset(tmpsd,0,sizeof(tmpsd));
	memset(pt,0,sizeof(pt));

	max_hp = status_get_max_hp(&md->bl);

	if(src && src->type == BL_MOB)
		mob_unlocktarget((struct mob_data *)src,tick);

	
	if(sd) {
		int sp = 0, hp = 0;
		if (sd->state.attack_type == BF_MAGIC && sd->skilltarget == md->bl.id && (i=pc_checkskill(sd,HW_SOULDRAIN))>0)
		{	//Soul Drain should only work on targetted spells [Skotlex]
			if (pc_issit(sd)) pc_setstand(sd); //Character stuck in attacking animation while 'sitting' fix. [Skotlex]
			clif_skill_nodamage(src,&md->bl,HW_SOULDRAIN,i,1);
			sp += (status_get_lv(&md->bl))*(65+15*i)/100;
		}
		sp += sd->sp_gain_value;
		sp += sd->sp_gain_race[race];
		hp += sd->hp_gain_value;
		if (sp > 0) {
			if(sd->status.sp + sp > sd->status.max_sp)
				sp = sd->status.max_sp - sd->status.sp;
			sd->status.sp += sp;
			if (sp > 0 && battle_config.show_hp_sp_gain)
				clif_heal(sd->fd,SP_SP,sp);
		}
		if (hp > 0) {
			if(sd->status.hp + hp > sd->status.max_hp)
				hp = sd->status.max_hp - sd->status.hp;
			sd->status.hp += hp;
			if (hp > 0 && battle_config.show_hp_sp_gain)
				clif_heal(sd->fd,SP_HP,hp);
		}
	}

	// map外に消えた人は計算から除くので
	// overkill分は無いけどsumはmax_hpとは違う

	tdmg = 0;
	for(i=0,count=0,mvp_damage=0;i<DAMAGELOG_SIZE;i++){
		if(md->dmglog[i].id==0)
			continue;
		tmpsd[i] = map_charid2sd(md->dmglog[i].id);
		if(tmpsd[i] == NULL)
			continue;
		count++;
		if(tmpsd[i]->bl.m != md->bl.m || pc_isdead(tmpsd[i]))
			continue;

		tdmg += (double)md->dmglog[i].dmg;
		if(mvp_damage<md->dmglog[i].dmg){
			third_sd = second_sd;
			second_sd = mvp_sd;
			mvp_sd=tmpsd[i];
			mvp_damage=md->dmglog[i].dmg;
		}
	}

	// [MouseJstr]
	if((map[md->bl.m].flag.pvp == 0) || (battle_config.pvp_exp == 1)) {

	// 経験値の分配
	for(i=0;i<DAMAGELOG_SIZE;i++){
		int pid,flag=1,zeny=0;
		unsigned long base_exp,job_exp;
		double per;
		struct party *p;
		if(tmpsd[i]==NULL || tmpsd[i]->bl.m != md->bl.m || pc_isdead(tmpsd[i]))
			continue;

		if (battle_config.exp_calc_type)	// eAthena's exp formula based on max hp.
			per = (double)md->dmglog[i].dmg*(9.+(double)((count > 6)? 6:count))/10./(double)max_hp;
		else //jAthena's exp formula based on total damage.
			per = (double)md->dmglog[i].dmg*(9.+(double)((count > 6)? 6:count))/10./tdmg;

		base_exp = (unsigned long)md->db->base_exp;
		job_exp = (unsigned long)md->db->job_exp;

		per += ret/100.; //SC_RICHMANKIM bonus. [Skotlex]

		if(sd) {
			if (sd->expaddrace[race])
				per += sd->expaddrace[race]/100.;	
			if (battle_config.pk_mode && (md->db->lv - sd->status.base_level >= 20))
				per *= 1.15;	// pk_mode additional exp if monster >20 levels [Valaris]		
		}
		if(md->size==1)	// change experience for different sized monsters [Valaris]
			per /=2.;
		else if(md->size==2)
			per *=2.;
		if(md->master_id) {
			if(((master = map_id2bl(md->master_id)) && status_get_mode(master)&MD_BOSS) ||	// check if its master is a boss (MVP's and minibosses)
				md->state.special_mob_ai) { // for summoned creatures [Valaris]
				per = 0;
			}
		} else {
			if(battle_config.zeny_from_mobs) {
				if(md->level > 0) zeny=(int) ((md->level+rand()%md->level)*per); // zeny calculation moblv + random moblv [Valaris]
				if(md->db->mexp > 0)
					zeny*=rand()%250;
				if(md->size==1 && zeny >=2) // change zeny for different sized monsters [Valaris]
					zeny/=2;
				else if(md->size==2 && zeny >1)
					zeny*=2;
			}
			if(battle_config.mobs_level_up && md->level > md->db->lv) { // [Valaris]
				base_exp+=(unsigned long) ((md->level-md->db->lv)*(md->db->base_exp)*.03);
				job_exp+=(unsigned long) ((md->level-md->db->lv)*(md->db->job_exp)*.03);
			}
		}

		if (per > 3) per = 3; //Limit gained exp to triple the mob's exp.
		base_exp = base_exp*per;
		job_exp = job_exp*per;
	
		if (base_exp > 0x7fffffff) base_exp = 0x7fffffff;
		else if (base_exp < 1) base_exp = 1;
		
		if (job_exp > 0x7fffffff) job_exp = 0x7fffffff;
		else if (job_exp < 1) job_exp = 1;
	
		//mapflags: noexp check [Lorky]
		if (map[md->bl.m].flag.nobaseexp == 1)	base_exp=0; 
		if (map[md->bl.m].flag.nojobexp == 1)	job_exp=0; 
		//end added Lorky 
		if((pid=tmpsd[i]->status.party_id)>0){	// パーティに入っている
			int j;
			for(j=0;j<pnum;j++)	// 公平パーティリストにいるかどうか
				if(pt[j].id==pid)
					break;
			if(j==pnum){	// いないときは公平かどうか確認
				if((p=party_search(pid))!=NULL && p->exp!=0){
					pt[pnum].id=pid;
					pt[pnum].p=p;
					pt[pnum].base_exp=base_exp;
					pt[pnum].job_exp=job_exp;
					if(battle_config.zeny_from_mobs)
						pt[pnum].zeny=zeny; // zeny share [Valaris]
					pnum++;
					flag=0;
				}
			}else{	// いるときは公平
				if (pt[j].base_exp +base_exp < 0x7fffffff)
					pt[j].base_exp+=base_exp;
				else
					pt[j].base_exp = 0x7fffffff;
				if (pt[j].job_exp +job_exp < 0x7fffffff)
					pt[j].job_exp+=job_exp;
				else
					pt[j].job_exp = 0x7fffffff;
				if(battle_config.zeny_from_mobs)
					pt[j].zeny+=zeny;  // zeny share [Valaris]
				flag=0;
			}
		}
		if(flag) {	// added zeny from mobs [Valaris]
			if(base_exp > 0 || job_exp > 0)
				pc_gainexp(tmpsd[i],base_exp,job_exp);
			if (battle_config.zeny_from_mobs && zeny > 0) {
				pc_getzeny(tmpsd[i],zeny); // zeny from mobs [Valaris]
			}
		}

	}
	// 公平分配
	for(i=0;i<pnum;i++)
		party_exp_share(pt[i].p,md->bl.m,pt[i].base_exp,pt[i].job_exp,pt[i].zeny);

	// item drop
	if (!(type&1)) {
		int drop_ore = -1, drop_items = 0; //slot N for DROP LOG, number of dropped items
		int log_item[10]; //8 -> 10 Lupus
		memset(&log_item,0,sizeof(log_item));
		for (i = 0; i < 10; i++) { // 8 -> 10 Lupus
			struct delay_item_drop *ditem;

			if ((master && status_get_mode(master) & MD_BOSS) ||	// check if its master is a boss (MVP's and minibosses)
				(md->state.special_mob_ai &&
					(battle_config.alchemist_summon_reward == 0 || //Noone gives items
					(md->class_ != 1142 && battle_config.alchemist_summon_reward == 1) //Non Marine spheres don't drop items
				)))	// Added [Valaris]
				break;	// End
			//mapflag: noloot check [Lorky]
			if (map[md->bl.m].flag.nomobloot) break;; 
			//end added [Lorky]

			if (md->db->dropitem[i].nameid <= 0)
				continue;
			drop_rate = md->db->dropitem[i].p;
			if (drop_rate <= 0 && !battle_config.drop_rate0item)
				drop_rate = 1;
			// change drops depending on monsters size [Valaris]
			if(md->size==1 && drop_rate >= 2)
				drop_rate/=2;
			else if(md->size==2 && drop_rate > 0)
				drop_rate*=2;
			//Drops affected by luk as a fixed increase [Valaris]
			if (src && battle_config.drops_by_luk > 0)
				drop_rate += status_get_luk(src)*battle_config.drops_by_luk/100;
			//Drops affected by luk as a % increase [Skotlex] 
			if (src && battle_config.drops_by_luk2 > 0)
				drop_rate += (int)(0.5+drop_rate*status_get_luk(src)*battle_config.drops_by_luk2/10000.0);
			if (sd && battle_config.pk_mode == 1 && (md->db->lv - sd->status.base_level >= 20))
				drop_rate = (int)(drop_rate*1.25); // pk_mode increase drops if 20 level difference [Valaris]

			if (drop_rate < rand() % 10000 + 1) { //fixed 0.01% impossible drops bug [Lupus]
				drop_ore = i; //we remember an empty slot to put there ORE DISCOVERY drop later.
				continue;
			}
			drop_items++; //we count if there were any drops

			ditem = mob_setdropitem(md->db->dropitem[i].nameid, 1, md->bl.m, md->bl.x, md->bl.y, mvp_sd, second_sd, third_sd);
			log_item[i] = ditem->item_data.nameid;
			mob_item_drop(md, tick+500+i, ditem, 0);

			//A Rare Drop Global Announce by Lupus
			if(drop_rate<=battle_config.rare_drop_announce) {
				struct item_data *i_data;
				char message[128];
				i_data = itemdb_exists(ditem->item_data.nameid);
				sprintf (message, msg_txt(541), (sd!=NULL && md!=NULL && sd->status.name != NULL)?sd->status.name :"GM", md->db->jname, i_data->jname, (float)drop_rate/100);
				//MSG: "'%s' won %s's %s (chance: %%%0.02f)"
				intif_GMmessage(message,strlen(message)+1,0);
			}
		}

		// Ore Discovery [Celest]
		if (sd == mvp_sd && map[md->bl.m].flag.nomobloot==0 && pc_checkskill(sd,BS_FINDINGORE)>0 && battle_config.finding_ore_rate/100 >= rand()%1000) {
			struct delay_item_drop *ditem;
			ditem = mob_setdropitem(itemdb_searchrandomid(6), 1, md->bl.m, md->bl.x, md->bl.y, mvp_sd, second_sd, third_sd);
			if (drop_ore<0) i=8; //we have only 10 slots in LOG, there's a check to not overflow (9th item usually a card, so we use 8th slot)
			log_item[i] = ditem->item_data.nameid; //it's for logging only
			drop_items++; //we count if there were any drops
			mob_item_drop(md, tick+500+i, ditem, 0);
		}

		//this drop log contains ALL dropped items + ORE (if there was ORE Recovery) [Lupus]
		if(sd && log_config.drop > 0 && drop_items) //we check were there any drops.. and if not - don't write the log
			log_drop(sd, md->class_, log_item); //mvp_sd

		if(sd/* && sd->state.attack_type == BF_WEAPON*/) { //Player reports indicate this SHOULD work with all skills. [Skotlex]
			int itemid = 0;
			for (i = 0; i < sd->monster_drop_item_count; i++) {
				struct delay_item_drop *ditem;
				if (sd->monster_drop_itemid[i] < 0)
					continue;
				if (sd->monster_drop_race[i] & (1<<race) ||
					sd->monster_drop_race[i] & 1<<(mode&MD_BOSS?10:11))
				{
					if (sd->monster_drop_itemrate[i] <= rand()%10000+1)
						continue;
					itemid = (sd->monster_drop_itemid[i] > 0) ? sd->monster_drop_itemid[i] :
						itemdb_searchrandomgroup(sd->monster_drop_itemgroup[i]);

					ditem = mob_setdropitem(itemid, 1, md->bl.m, md->bl.x, md->bl.y, mvp_sd, second_sd, third_sd);
					mob_item_drop(md, tick+520+i, ditem, 0);
				}
			}
			if(sd->get_zeny_num && rand()%100 < sd->get_zeny_rate) //Gets get_zeny_num per level +/-10% [Skotlex]
				pc_getzeny(sd,md->db->lv*sd->get_zeny_num*(90+rand()%21)/100);
		}
		if(md->lootitem) {
			for(i=0;i<md->lootitem_count;i++) {
				struct delay_item_drop *ditem;

				ditem = mob_setlootitem(&md->lootitem[i], md->bl.m, md->bl.x, md->bl.y, mvp_sd, second_sd, third_sd);
				mob_item_drop(md, tick+540+i, ditem, 1);
			}
		}
	}

	// mvp処理
	if(mvp_sd && md->db->mexp > 0 && !md->state.special_mob_ai){
		int log_mvp[2] = {0};
		int j;
		int mexp;
		temp = ((double)md->db->mexp * (9.+(double)count)/10.);	//[Gengar]
		mexp = (temp > 2147483647.)? 0x7fffffff:(int)temp;

		//mapflag: noexp check [Lorky]
		if (map[md->bl.m].flag.nobaseexp == 1 || map[md->bl.m].flag.nojobexp == 1)	mexp=1; 
		//end added [Lorky] 

		if(mexp < 1) mexp = 1;
		clif_mvp_effect(mvp_sd);					// エフェクト
		clif_mvp_exp(mvp_sd,mexp);
		pc_gainexp(mvp_sd,mexp,0);
		log_mvp[1] = mexp;
		for(j=0;j<3;j++){
			i = rand() % 3;
			//mapflag: noloot check [Lorky]
			if (map[md->bl.m].flag.nomvploot == 1) break;
			//end added Lorky 			

			if(md->db->mvpitem[i].nameid <= 0)
				continue;
			drop_rate = md->db->mvpitem[i].p;
			if(drop_rate <= 0 && !battle_config.drop_rate0item)
				drop_rate = 1;
			if(drop_rate <= rand()%10000+1) //if ==0, then it doesn't drop
				continue;
			memset(&item,0,sizeof(item));
			item.nameid=md->db->mvpitem[i].nameid;
			item.identify=!itemdb_isequip3(item.nameid);
			clif_mvp_item(mvp_sd,item.nameid);
			log_mvp[0] = item.nameid;
			if(mvp_sd->weight*2 > mvp_sd->max_weight)
				map_addflooritem(&item,1,mvp_sd->bl.m,mvp_sd->bl.x,mvp_sd->bl.y,mvp_sd,second_sd,third_sd,1);
			else if((ret = pc_additem(mvp_sd,&item,1))) {
				clif_additem(sd,0,0,ret);
				map_addflooritem(&item,1,mvp_sd->bl.m,mvp_sd->bl.x,mvp_sd->bl.y,mvp_sd,second_sd,third_sd,1);
			}
			
			//A Rare MVP Drop Global Announce by Lupus
			if(drop_rate<=battle_config.rare_drop_announce) {
				struct item_data *i_data;
				char message[128];
				i_data = itemdb_exists(item.nameid);
				sprintf (message, msg_txt(541), (mvp_sd!=NULL && md!=NULL && mvp_sd->status.name != NULL)?mvp_sd->status.name :"GM", md->db->jname, i_data->jname, (float)drop_rate/100);
				//MSG: "'%s' won %s's %s (chance: %%%0.02f)"
				intif_GMmessage(message,strlen(message)+1,0);
			}

			if(log_config.mvpdrop > 0)
				log_mvpdrop(mvp_sd, md->class_, log_mvp);
				
				break;
		}

	}

	} // [MouseJstr]

	// <Agit> NPC Event [OnAgitBreak]
	if(md->npc_event[0] && strcmp(((md->npc_event)+strlen(md->npc_event)-13),"::OnAgitBreak") == 0) {
		ShowNotice("MOB.C: Run NPC_Event[OnAgitBreak].\n");
		if (agit_flag == 1) //Call to Run NPC_Event[OnAgitBreak]
			guild_agit_break(md);
	}

		// SCRIPT実行
	if(md->npc_event[0]){
//		if(battle_config.battle_log)
//			printf("mob_damage : run event : %s\n",md->npc_event);
		if(src && src->type == BL_PET)
			sd = ((struct pet_data *)src)->msd;
		if(sd == NULL) {
			if(mvp_sd != NULL)
				sd = mvp_sd;
			else {
				struct map_session_data *tmpsd;
				int i;
				for(i=0;i<fd_max;i++){
					if(session[i] && (tmpsd= (struct map_session_data *) session[i]->session_data) && tmpsd->state.auth) {
						if(md->bl.m == tmpsd->bl.m) {
							sd = tmpsd;
							break;
						}
					}
				}
			}
		}
		if(mvp_sd)
			npc_event(mvp_sd,md->npc_event,0);

	} else if (mvp_sd) {
//lordalfa
		pc_setglobalreg(mvp_sd,"killedrid",(md->class_));
	if (script_config.event_script_type == 0) {
		struct npc_data *npc;
	if ((npc = npc_name2id("NPCKillEvent"))) {
	run_script(npc->u.scr.script,0,mvp_sd->bl.id,npc->bl.id); // NPCKillNPC
	 ShowStatus("Event '"CL_WHITE"NPCKillEvent"CL_RESET"' executed.\n");
	}
		} else {
	ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n",	
		npc_event_doall_id("NPCKillEvent", mvp_sd->bl.id), "NPCKillEvent");
	}
}
//[lordalfa]
	(battle_config.mob_clear_delay) ? clif_clearchar_delay(tick+battle_config.mob_clear_delay,&md->bl,1) : clif_clearchar_area(&md->bl,1);
	clif_clearchar_area(&md->bl,1);
	
	if(md->level) md->level=0;
	map_delblock(&md->bl);
	if(mob_get_viewclass(md->class_) <= JOB_SUPER_NOVICE || (mob_get_viewclass(md->class_) >= JOB_NOVICE_HIGH && mob_get_viewclass(md->class_) <= JOB_SUPER_BABY))
		clif_clearchar_delay(tick+3000,&md->bl,0);
	mob_deleteslave(md);
	mob_setdelayspawn(md->bl.id);
	map_freeblock_unlock();

	return damage;
}

int mob_guardian_guildchange(struct block_list *bl,va_list ap)
{
	struct mob_data *md;
	struct guild* g;

	nullpo_retr(0, bl);
	nullpo_retr(0, md = (struct mob_data *)bl);

	if (!md->guardian_data)
		return 0;

	if (md->guardian_data->castle->guild_id == 0)
	{	//Castle with no owner? Delete the guardians.
		if (md->class_ == MOBID_EMPERIUM)
		{	//But don't delete the emperium, just clear it's guild-data
			md->guardian_data->guild_id = 0;
			md->guardian_data->emblem_id = 0;
			md->guardian_data->guild_name[0] = '\0';
		} else {
			if (md->guardian_data->castle->guardian[md->guardian_data->number].visible)
			{	//Safe removal of guardian.
				md->guardian_data->castle->guardian[md->guardian_data->number].visible = 0;
				guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
				guild_castledatasave(md->guardian_data->castle->castle_id, 18+md->guardian_data->number,0);
			}
			mob_delete(md); //Remove guardian.
		}
		return 0;
	}
	
	g = guild_search(md->guardian_data->castle->guild_id);
	if (g == NULL)
	{	//Properly remove guardian info from Castle data.
		ShowError("mob_guardian_guildchange: New Guild (id %d) does not exists!\n", md->guardian_data->guild_id);
		md->guardian_data->castle->guardian[md->guardian_data->number].visible = 0;
		guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
		guild_castledatasave(md->guardian_data->castle->castle_id, 18+md->guardian_data->number,0);
		mob_delete(md);
		return 0;
	}

	md->guardian_data->guild_id = md->guardian_data->castle->guild_id;
	md->guardian_data->emblem_id = g->emblem_id;
	md->guardian_data->guardup_lv = guild_checkskill(g,GD_GUARDUP);
	memcpy(md->guardian_data->guild_name, g->name, NAME_LENGTH);

	return 1;	
}
	
/*==========================================
 * Pick a random class for the mob
 *------------------------------------------
 */
int mob_random_class (int *value, size_t count)
{
	nullpo_retr(0, value);

	// no count specified, look into the array manually, but take only max 5 elements
	if (count < 1) {
		count = 0;
		while(count < 5 && mobdb_checkid(value[count])) count++;
		if(count < 1)	// nothing found
			return 0;
	} else {
		// check if at least the first value is valid
		if(mobdb_checkid(value[0]) == 0)
			return 0;
	}
	//Pick a random value, hoping it exists. [Skotlex]
	return mobdb_checkid(value[rand()%count]);
}

/*==========================================
 * Change mob base class
 *------------------------------------------
 */
int mob_class_change (struct mob_data *md, int class_)
{
	unsigned int tick = gettick();
	int i, c, hp_rate;

	nullpo_retr(0, md);

	if (md->bl.prev == NULL)
		return 0;

	hp_rate = md->hp*100/status_get_max_hp(&md->bl);
	clif_mob_class_change(md,class_);
	md->class_ = class_;
	md->db = mob_db(class_);
	md->max_hp = md->db->max_hp; //Update the mob's max HP
	if (battle_config.monster_class_change_full_recover) {
		md->hp = md->max_hp;
		memset(md->dmglog, 0, sizeof(md->dmglog));
	} else
		md->hp = md->max_hp*hp_rate/100;
	if(md->hp > md->max_hp) md->hp = md->max_hp;
	else if(md->hp < 1) md->hp = 1;

	memcpy(md->name,md->db->jname,NAME_LENGTH-1);
	memset(&md->state,0,sizeof(md->state));
	md->attacked_id = 0;
	md->target_id = 0;
	md->move_fail_count = 0;

	md->speed = md->db->speed;
	md->def_ele = md->db->element;

	mob_changestate(md,MS_IDLE,0);
	skill_castcancel(&md->bl,0);
	md->state.skillstate = MSS_IDLE;
	md->last_thinktime = tick;
	md->next_walktime = tick+rand()%50+5000;
	md->attackabletime = tick;
	md->canmove_tick = tick;
	md->last_linktime = tick;

	for(i=0,c=tick-1000*3600*10;i<MAX_MOBSKILL;i++)
		md->skilldelay[i] = c;
	md->skillid=0;
	md->skilllv=0;

	if(md->lootitem == NULL && md->db->mode&MD_LOOTER)
		md->lootitem=(struct item *)aCalloc(LOOTITEM_SIZE,sizeof(struct item));

	skill_clear_unitgroup(&md->bl);
	skill_cleartimerskill(&md->bl);

	clif_clearchar_area(&md->bl,0);
	clif_spawnmob(md);

	if (battle_config.show_mob_hp)
		clif_charnameack(0, &md->bl);

	return 0;
}

/*==========================================
 * mob回復
 *------------------------------------------
 */
int mob_heal(struct mob_data *md,int heal)
{
	int max_hp;

	nullpo_retr(0, md);
	max_hp = status_get_max_hp(&md->bl);

	md->hp += heal;
	if( max_hp < md->hp )
		md->hp = max_hp;

	if(md->guardian_data && md->guardian_data->number < MAX_GUARDIANS) { // guardian hp update [Valaris] (updated by [Skotlex])
		if ((md->guardian_data->castle->guardian[md->guardian_data->number].hp = md->hp) <= 0)
		{
			guild_castledatasave(md->guardian_data->castle->castle_id, 10+md->guardian_data->number,0);
			guild_castledatasave(md->guardian_data->castle->castle_id, 18+md->guardian_data->number,0);
		}
	}	// end addition

	if (battle_config.show_mob_hp)
		clif_charnameack(0, &md->bl);

	return 0;
}


/*==========================================
 * Added by RoVeRT
 *------------------------------------------
 */
int mob_warpslave_sub(struct block_list *bl,va_list ap)
{
	struct mob_data *md=(struct mob_data *)bl;
	struct block_list *master;
	int x,y,range,i=0;
	master = va_arg(ap, struct block_list*);
	range = va_arg(ap, int);
	
	if(md->master_id!=master->id)
		return 0;

	do {
		x = master->x - range/2 + rand()%range;
		y = master->y - range/2 + rand()%range;
	} while (map_getcell(master->m,x,y,CELL_CHKNOPASS) && i<25);
	
	if (i == 100)
		mob_warp(md, master->m, master->x, master->y,2);
	else
		mob_warp(md, master->m, x, y,2);

	return 1;
}

/*==========================================
 * Added by RoVeRT
 * Warps slaves. Range is the area around the master that they can
 * appear in randomly.
 *------------------------------------------
 */
int mob_warpslave(struct block_list *bl, int range)
{
	if (range < 1)
		range = 1; //Min range needed to avoid crashes and stuff. [Skotlex]
	
	return map_foreachinarea(mob_warpslave_sub, bl->m, 0, 0, map[bl->m].xs,map[bl->m].ys, BL_MOB, bl, range);
}

/*==========================================
 * mobワープ
 *------------------------------------------
 */
int mob_warp(struct mob_data *md,int m,int x,int y,int type)
{
	int i=0,xs=0,ys=0,bx=x,by=y;
	int tick = gettick();

	nullpo_retr(0, md);

	if( md->bl.prev==NULL )
		return 0;

	if( m<0 ) m=md->bl.m;

	if(type >= 0) {
		if(map[md->bl.m].flag.monster_noteleport)
			return 0;
		clif_clearchar_area(&md->bl,type);
	}
	skill_unit_move(&md->bl,tick,4);
	map_delblock(&md->bl);

	if(bx>0 && by>0){	// 位置指定の場合周囲９セルを探索
		xs=ys=9;
	}

	while( ( x<0 || y<0 || map_getcell(m,x,y,CELL_CHKNOPASS)) && (i++)<1000 ){
		if( xs>0 && ys>0 && i<250 ){	// 指定位置付近の探索
			x=bx+rand()%xs-xs/2;
			y=by+rand()%ys-ys/2;
		}else{			// 完全ランダム探索
			x=rand()%(map[m].xs-2)+1;
			y=rand()%(map[m].ys-2)+1;
		}
	}
	md->dir=0;
	if(i<1000){
		md->bl.x=md->to_x=x;
		md->bl.y=md->to_y=y;
		md->bl.m=m;
	}else {
		m=md->bl.m;
		if(battle_config.error_log==1)
			ShowWarning("MOB %d warp failed, class = %d\n",md->bl.id,md->class_);
	}

	md->target_id=0;	// タゲを解除する
	md->state.targettype=NONE_ATTACKABLE;
	md->attacked_id=0;
	if (md->master_id)
		md->master_dist = 0; //Assume mob warped to leader. [Skotlex]
	md->state.skillstate=MSS_IDLE;
	mob_changestate(md,MS_IDLE,0);

	if(type>0 && i==1000) {
		if(battle_config.battle_log)
			ShowInfo("MOB %d warp to (%d,%d), class = %d\n",md->bl.id,x,y,md->class_);
	}

	map_addblock(&md->bl);
	skill_unit_move(&md->bl,tick,1);
	if(type>0)
	{
		clif_spawnmob(md);
		mob_warpslave(&md->bl,AREA_SIZE);
	}

	return 0;
}

/*==========================================
 * 画面内の取り巻きの数計算用(foreachinarea)
 *------------------------------------------
 */
int mob_countslave_sub(struct block_list *bl,va_list ap)
{
	int id,*c;
	struct mob_data *md;

	id=va_arg(ap,int);

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, c=va_arg(ap,int *));
	nullpo_retr(0, md = (struct mob_data *)bl);

	if( md->master_id==id )
		(*c)++;
	return 0;
}
/*==========================================
 * 画面内の取り巻きの数計算
 *------------------------------------------
 */
int mob_countslave(struct mob_data *md)
{
	int c=0;

	nullpo_retr(0, md);

	map_foreachinarea(mob_countslave_sub, md->bl.m,
		0,0,map[md->bl.m].xs-1,map[md->bl.m].ys-1,
		BL_MOB,md->bl.id,&c);
	return c;
}
/*==========================================
 * Summons amount slaves contained in the value[5] array using round-robin. [adapted by Skotlex]
 *------------------------------------------
 */
int mob_summonslave(struct mob_data *md2,int *value,int amount,int skill_id)
{
	struct mob_data *md;
	int bx,by,m,count = 0,class_,k;

	nullpo_retr(0, md2);
	nullpo_retr(0, value);

	bx=md2->bl.x;
	by=md2->bl.y;
	m=md2->bl.m;

	if(mobdb_checkid(value[0]) == 0)
		return 0;

	while(count < 5 && mobdb_checkid(value[count])) count++;
	if(count < 1) return 0;

	for(k=0;k<amount;k++) {
		int x=0,y=0,i=0;
		class_ = value[k%count]; //Summon slaves in round-robin fashion. [Skotlex]

		if (mobdb_checkid(class_) == 0)
			continue;

		md=(struct mob_data *)aCalloc(1,sizeof(struct mob_data));
		if(mob_db(class_)->mode&MD_LOOTER)
			md->lootitem=(struct item *)aCalloc(LOOTITEM_SIZE,sizeof(struct item));

		while((x<=0 || y<=0 || map_getcell(m,x,y,CELL_CHKNOPASS)) && (i++)<100){
			x=rand()%9-4+bx;
			y=rand()%9-4+by;
		}
		if(i>=100){
			x=bx;
			y=by;
		}

		mob_spawn_dataset(md,"--ja--",class_);
		md->bl.m=m;
		md->bl.x=x;
		md->bl.y=y;

		md->m =m;
		md->x0=x;
		md->y0=y;
		md->xs=0;
		md->ys=0;
		if (battle_config.slaves_inherit_speed && (skill_id != NPC_METAMORPHOSIS && skill_id != NPC_TRANSFORMATION))
			md->speed=md2->speed;
		md->cached= battle_config.dynamic_mobs;	//[Skotlex]
		md->spawndelay1=-1;	// 一度のみフラグ
		md->spawndelay2=-1;	// 一度のみフラグ

		if (!battle_config.monster_class_change_full_recover &&
			(skill_id == NPC_TRANSFORMATION || skill_id == NPC_METAMORPHOSIS))
		{	//Scale HP
			md->hp = (md->max_hp*md2->hp)/md2->max_hp;
		}
		
		memset(md->npc_event,0,sizeof(md->npc_event));
		md->bl.type=BL_MOB;
		map_addiddb(&md->bl);
		mob_spawn(md->bl.id);
		clif_skill_nodamage(&md->bl,&md->bl,skill_id,amount,1);

		if(skill_id == NPC_SUMMONSLAVE)
			md->master_id=md2->bl.id;
	}
	return 0;
}

/*==========================================
 *MOBskillから該当skillidのskillidxを返す
 *------------------------------------------
 */
int mob_skillid2skillidx(int class_,int skillid)
{
	int i, max = mob_db(class_)->maxskill;
	struct mob_skill *ms=mob_db(class_)->skill;

	if(ms==NULL)
		return -1;

	for(i=0;i<max;i++){
		if(ms[i].skill_id == skillid)
			return i;
	}
	return -1;

}

//
// MOBスキル
//

/*==========================================
 * スキル使用（詠唱完了、ID指定）
 *------------------------------------------
 */
int mobskill_castend_id( int tid, unsigned int tick, int id,int data )
{
	struct mob_data* md=NULL;
	struct block_list *bl;
	struct block_list *mbl;
	int range;

	if((mbl = map_id2bl(id)) == NULL ) //詠唱したMobがもういないというのは良くある正常処理
		return 0;
	if((md=(struct mob_data *)mbl) == NULL ){
		ShowError("mobskill_castend_id nullpo mbl->id:%d\n",mbl->id);
		return 0;
	}

	if( md->bl.type!=BL_MOB || md->bl.prev==NULL )
		return 0;

	if( md->skilltimer != tid )	// タイマIDの確認
		return 0;

	md->skilltimer=-1;

	if((bl = map_id2bl(md->skilltarget)) == NULL || bl->prev==NULL){ //スキルターゲットが存在しない
		//printf("mobskill_castend_id nullpo\n");//ターゲットがいないときはnullpoじゃなくて普通に終了
		return 0;
	}
	if(md->bl.m != bl->m)
		return 0;

	if(md->skillid != NPC_EMOTION)
		md->last_thinktime=tick + status_get_adelay(&md->bl);
		
	if(md->skillid == RG_BACKSTAP) {
		int dir = map_calc_dir(&md->bl,bl->x,bl->y),t_dir = status_get_dir(bl);
		int dist = distance(md->bl.x,md->bl.y,bl->x,bl->y);
		if(bl->type != BL_SKILL && (dist == 0 || map_check_dir(dir,t_dir)))
			return 0;
	}
	if( ( skill_get_inf(md->skillid) & INF_ATTACK_SKILL || md->skillid == MO_EXTREMITYFIST ) &&
		battle_check_target(&md->bl,bl, BCT_ENEMY)<=0 )
		return 0;

	if(tid != -1)
	{
		if (!status_check_skilluse(&md->bl, bl, md->skillid, 1))
			return 0;
		range = skill_get_range(md->skillid,md->skilllv);
		if(range < 0)
			range = status_get_range(&md->bl) - (range + 1);
		if(range + battle_config.mob_skill_add_range < distance(md->bl.x,md->bl.y,bl->x,bl->y))
			return 0;
	}
	md->skilldelay[md->skillidx]=tick;

	if(battle_config.mob_skill_log)
		ShowInfo("MOB skill castend skill=%d, class = %d\n",md->skillid,md->class_);
//	mob_stop_wShowInfo(md,0);

	switch( skill_get_nk(md->skillid) )
	{
	case NK_NO_DAMAGE:// 支援系
		if(!md->db->skill[md->skillidx].val[0] &&
			(md->skillid==AL_HEAL || (md->skillid==ALL_RESURRECTION && bl->type != BL_PC)) && battle_check_undead(status_get_race(bl),status_get_elem_type(bl)) )
			clif_emotion(&md->bl, 4); //Prevent mobs from casting offensive heal. [Skotlex]
			//skill_castend_damage_id(&md->bl,bl,md->skillid,md->skilllv,tick,0);
		else
			skill_castend_nodamage_id(&md->bl,bl,md->skillid,md->skilllv,tick,0);
		break;
	// 攻撃系/吹き飛ばし系
	case NK_SPLASH_DAMAGE:
	default:
		skill_castend_damage_id(&md->bl,bl,md->skillid,md->skilllv,tick,0);
		break;
	}

	if (md->sc_count && md->sc_data[SC_MAGICPOWER].timer != -1 && md->skillid != HW_MAGICPOWER)
		status_change_end(&md->bl, SC_MAGICPOWER, -1);
		
	return 0;
}

/*==========================================
 * スキル使用（詠唱完了、場所指定）
 *------------------------------------------
 */
int mobskill_castend_pos( int tid, unsigned int tick, int id,int data )
{
	struct mob_data* md=NULL;
	int range,maxcount;

	nullpo_retr(0, md=(struct mob_data *)map_id2bl(id));

	if( md->bl.type!=BL_MOB || md->bl.prev==NULL )
		return 0;

	if( md->skilltimer != tid )	// タイマIDの確認
		return 0;

	md->skilltimer=-1;

	if (tid != -1)
	{	//Avoid unnecessary checks for instant-cast skills. [Skotlex]
		if (!status_check_skilluse(&md->bl, NULL, md->skillid, 1))
			return 0;
		range = skill_get_range(md->skillid,md->skilllv);
		if(range < 0)
			range = status_get_range(&md->bl) - (range + 1);
		if(range + battle_config.mob_skill_add_range < distance(md->bl.x,md->bl.y,md->skillx,md->skilly))
			return 0;
	}

	if (!battle_config.monster_skill_reiteration &&
			skill_get_unit_flag (md->skillid) & UF_NOREITERATION &&
			skill_check_unit_range (md->bl.m, md->skillx, md->skilly, md->skillid, md->skilllv))
		return 0;

	if(battle_config.monster_skill_nofootset &&
			skill_get_unit_flag (md->skillid) & UF_NOFOOTSET &&
			skill_check_unit_range2(&md->bl, md->bl.m, md->skillx, md->skilly, md->skillid, md->skilllv))
		return 0;

	if(battle_config.monster_land_skill_limit) {
		maxcount = skill_get_maxcount(md->skillid);
		if(maxcount > 0) {
			int i,c;
			for(i=c=0;i<MAX_MOBSKILLUNITGROUP;i++) {
				if(md->skillunit[i].alive_count > 0 && md->skillunit[i].skill_id == md->skillid)
					c++;
			}
			if(c >= maxcount)
				return 0;
		}
	}

	md->skilldelay[md->skillidx]=tick;

	if(battle_config.mob_skill_log)
		ShowInfo("MOB skill castend skill=%d, class = %d\n",md->skillid,md->class_);
//	mob_stop_walking(md,0);

	skill_castend_pos2(&md->bl,md->skillx,md->skilly,md->skillid,md->skilllv,tick,0);

	return 0;
}

/*==========================================
 * Skill use (an aria start, ID specification)
 *------------------------------------------
 */
int mobskill_use_id(struct mob_data *md,struct block_list *target,int skill_idx)
{
	int casttime,range;
	struct mob_skill *ms;
	int skill_id, skill_lv, forcecast = 0;
	int selfdestruct_flag = 0;

	nullpo_retr(0, md);
	nullpo_retr(0, ms=&md->db->skill[skill_idx]);

	if( target==NULL && (target=map_id2bl(md->target_id))==NULL )
		return 0;

	if( target->prev==NULL || md->bl.prev==NULL )
		return 0;

	skill_id=ms->skill_id;
	skill_lv=ms->skill_lv;

	if(map[md->bl.m].flag.gvg && skill_db[skill_id].nocast & 4)
		return 0;
	if(skill_get_inf2(skill_id)&INF2_NO_TARGET_SELF && md->bl.id == target->id)
		return 0;

	if(!status_check_skilluse(&md->bl, target, skill_id, 0))
		return 0;

	// 射程と障害物チェック
	range = skill_get_range(skill_id,skill_lv);
	if(range < 0)
		range = status_get_range(&md->bl) - (range + 1);
	if(!battle_check_range(&md->bl,target,range))
		return 0;

//	delay=skill_delayfix(&md->bl, skill_get_delay( skill_id,skill_lv) );

	casttime=skill_castfix(&md->bl,ms->casttime);
	md->state.skillcastcancel=ms->cancel;
	md->skilldelay[skill_idx]=gettick();

	switch(skill_id){	/* 何か特殊な処理が必要 */
	case ALL_RESURRECTION:	/* リザレクション */
		if(target->type != BL_PC && battle_check_undead(status_get_race(target),status_get_elem_type(target))){	/* 敵がアンデッドなら */
			forcecast=1;	/* ターンアンデットと同じ詠唱時間 */
			casttime=skill_castfix(&md->bl, skill_get_cast(PR_TURNUNDEAD,skill_lv) );
		}
		break;
	case MO_EXTREMITYFIST:	/*阿修羅覇鳳拳*/
	case SA_MAGICROD:
	case SA_SPELLBREAKER:
		forcecast=1;
		break;
	case NPC_SUMMONSLAVE:
	case NPC_SUMMONMONSTER:
		if(md->master_id!=0)
			return 0;
		break;
	case NPC_SELFDESTRUCTION:
		if (casttime == 0 && md->state.special_mob_ai == 2) {
			casttime = skill_get_time(skill_id,skill_lv);
			selfdestruct_flag =  1;
		}
		break;
	}

	if(battle_config.mob_skill_log)
		ShowInfo("MOB skill use target_id=%d skill=%d lv=%d cast=%d, class = %d\n",target->id,skill_id,skill_lv,casttime,md->class_);

	if (casttime || forcecast) { 	// 詠唱が必要
		if (!selfdestruct_flag)
			mob_stop_walking(md,0);		// 歩行停止
		clif_skillcasting(&md->bl, md->bl.id, target->id, 0,0, skill_id, casttime);
	}

	if (casttime <= 0)	// 詠唱の無いものはキャンセルされない
		md->state.skillcastcancel = 0;

	md->skilltarget	= target->id;
	md->skillx		= 0;
	md->skilly		= 0;
	md->skillid		= skill_id;
	md->skilllv		= skill_lv;
	md->skillidx	= skill_idx;

	if(!(battle_config.monster_cloak_check_type&2) && md->sc_data[SC_CLOAKING].timer != -1 && md->skillid != AS_CLOAKING)
		status_change_end(&md->bl,SC_CLOAKING,-1);

	if( casttime>0 ){
		md->skilltimer =
			add_timer( gettick()+casttime, mobskill_castend_id, md->bl.id, 0 );
	}else{
		md->skilltimer = -1;
		mobskill_castend_id(md->skilltimer,gettick(),md->bl.id, 0);
	}

	return 1;
}
/*==========================================
 * スキル使用（場所指定）
 *------------------------------------------
 */
int mobskill_use_pos( struct mob_data *md,
	int skill_x, int skill_y, int skill_idx)
{
	int casttime=0,range;
	struct mob_skill *ms;
	struct block_list bl;
	int skill_id, skill_lv;

	nullpo_retr(0, md);
	nullpo_retr(0, ms=&md->db->skill[skill_idx]);

	if( md->bl.prev==NULL )
		return 0;

	skill_id=ms->skill_id;
	skill_lv=ms->skill_lv;

	if(!status_check_skilluse(&md->bl, NULL, skill_id, 0))
		return 0;

	if(map[md->bl.m].flag.gvg && (skill_id == SM_ENDURE || skill_id == AL_TELEPORT || skill_id == AL_WARP ||
		skill_id == WZ_ICEWALL || skill_id == TF_BACKSLIDING))
		return 0;

	// 射程と障害物チェック
	bl.type = BL_NUL;
	bl.m = md->bl.m;
	bl.x = skill_x;
	bl.y = skill_y;
	range = skill_get_range(skill_id,skill_lv);
	if(range < 0)
		range = status_get_range(&md->bl) - (range + 1);
	if(!battle_check_range(&md->bl,&bl,range))
		return 0;

//	delay=skill_delayfix(&sd->bl, skill_get_delay( skill_id,skill_lv) );
	casttime=skill_castfix(&md->bl,ms->casttime);
	md->skilldelay[skill_idx]=gettick();
	md->state.skillcastcancel=ms->cancel;

	if(battle_config.mob_skill_log)
		ShowInfo("MOB skill use target_pos=(%d,%d) skill=%d lv=%d cast=%d, class = %d\n",
			skill_x,skill_y,skill_id,skill_lv,casttime,md->class_);

	if( casttime>0 ) {	// A cast time is required.
		mob_stop_walking(md,0);		// 歩行停止
		clif_skillcasting( &md->bl,
			md->bl.id, 0, skill_x,skill_y, skill_id,casttime);
	}

	if( casttime<=0 )	// A skill without a cast time wont be cancelled.
		md->state.skillcastcancel=0;


	md->skillx		= skill_x;
	md->skilly		= skill_y;
	md->skilltarget	= 0;
	md->skillid		= skill_id;
	md->skilllv		= skill_lv;
	md->skillidx	= skill_idx;
	if(!(battle_config.monster_cloak_check_type&2) && md->sc_data[SC_CLOAKING].timer != -1)
		status_change_end(&md->bl,SC_CLOAKING,-1);
	if( casttime>0 ){
		md->skilltimer =
			add_timer( gettick()+casttime, mobskill_castend_pos, md->bl.id, 0 );
	}else{
		md->skilltimer = -1;
		mobskill_castend_pos(md->skilltimer,gettick(),md->bl.id, 0);
	}

	return 1;
}


/*==========================================
 * Friendly Mob whose HP is decreasing by a nearby MOB is looked for.
 *------------------------------------------
 */
int mob_getfriendhpltmaxrate_sub(struct block_list *bl,va_list ap)
{
	int rate;
	struct block_list **fr;
	struct mob_data *md;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=va_arg(ap,struct mob_data *));
	rate=va_arg(ap,int);
	fr=va_arg(ap,struct block_list **);

	if( md->bl.id == bl->id )
		return 0;

	if ((*fr) != NULL) //A friend was already found.
		return 0;
	
	if (battle_check_target(&md->bl,bl,BCT_ENEMY)>0)
		return 0;
	
	if (status_get_hp(bl) < status_get_max_hp(bl) * rate / 100)
		(*fr) = bl;
	return 0;
}
struct block_list *mob_getfriendhpltmaxrate(struct mob_data *md,int rate)
{
	struct block_list *fr=NULL;
	const int r=8;
	int type = BL_MOB;
	
	nullpo_retr(NULL, md);

	if (md->state.special_mob_ai == 1) //Summoned creatures. [Skotlex]
		type = BL_PC;
	
	map_foreachinarea(mob_getfriendhpltmaxrate_sub, md->bl.m,
		md->bl.x-r ,md->bl.y-r, md->bl.x+r, md->bl.y+r,
		type,md,rate,&fr);
	return fr;
}
/*==========================================
 * Check hp rate of its master
 *------------------------------------------
 */
struct block_list *mob_getmasterhpltmaxrate(struct mob_data *md,int rate)
{
	if (md && md->master_id > 0) {
		struct block_list *bl = map_id2bl(md->master_id);
		if (status_get_hp(bl) < status_get_max_hp(bl) * rate / 100)
			return bl;
	}

	return NULL;
}
/*==========================================
 * What a status state suits by nearby MOB is looked for.
 *------------------------------------------
 */
int mob_getfriendstatus_sub(struct block_list *bl,va_list ap)
{
	int cond1,cond2;
	struct mob_data **fr, *md, *mmd;
	int flag=0;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=(struct mob_data *)bl);
	nullpo_retr(0, mmd=va_arg(ap,struct mob_data *));

	if( mmd->bl.id == bl->id )
		return 0;
	if (battle_check_target(&mmd->bl,bl,BCT_ENEMY)>0)
		return 0;
	cond1=va_arg(ap,int);
	cond2=va_arg(ap,int);
	fr=va_arg(ap,struct mob_data **);
	if( cond2==-1 ){
		int j;
		for(j=SC_STONE;j<=SC_BLIND && !flag;j++){
			flag=(md->sc_data[j].timer!=-1 );
		}
	}else
		flag=( md->sc_data[cond2].timer!=-1 );
	if( flag^( cond1==MSC_FRIENDSTATUSOFF ) )
		(*fr)=md;

	return 0;
}
struct mob_data *mob_getfriendstatus(struct mob_data *md,int cond1,int cond2)
{
	struct mob_data *fr=NULL;
	const int r=8;

	nullpo_retr(0, md);

	map_foreachinarea(mob_getfriendstatus_sub, md->bl.m,
		md->bl.x-r ,md->bl.y-r, md->bl.x+r, md->bl.y+r,
		BL_MOB,md,cond1,cond2,&fr);
	return fr;
}

/*==========================================
 * Skill use judging
 *------------------------------------------
 */
int mobskill_use(struct mob_data *md, unsigned int tick, int event)
{
	struct mob_skill *ms;
	struct block_list *fbl = NULL; //Friend bl, which can either be a BL_PC or BL_MOB depending on the situation. [Skotlex]
	struct mob_data *fmd = NULL;
	int i;

	nullpo_retr (0, md);
	nullpo_retr (0, ms = md->db->skill);

	if (battle_config.mob_skill_rate == 0 || md->skilltimer != -1)
		return 0;

	for (i = 0; i < md->db->maxskill; i++) {
		int c2 = ms[i].cond2, flag = 0;		

		// ディレイ中
		if (DIFF_TICK(tick, md->skilldelay[i]) < ms[i].delay)
			continue;

		// 状態判定
		if (ms[i].state >= 0 && ms[i].state != md->state.skillstate)
			continue;

		// 条件判定
		flag = (event == ms[i].cond1);
		if (!flag){
			switch (ms[i].cond1)
			{
				case MSC_ALWAYS:
					flag = 1; break;
				case MSC_MYHPLTMAXRATE:		// HP< maxhp%
					{
						int max_hp = status_get_max_hp(&md->bl);
						flag = (md->hp < max_hp * c2 / 100); break;
					}
				case MSC_MYSTATUSON:		// status[num] on
				case MSC_MYSTATUSOFF:		// status[num] off
					if (!md->sc_data) {
						flag = 0;
					} else if (ms[i].cond2 == -1) {
						int j;
						for (j = SC_STONE; j <= SC_BLIND; j++)
							if ((flag = (md->sc_data[j].timer != -1)) != 0)
								break;
					} else {
						flag = (md->sc_data[ms[i].cond2].timer != -1);
					}
					flag ^= (ms[i].cond1 == MSC_MYSTATUSOFF); break;
				case MSC_FRIENDHPLTMAXRATE:	// friend HP < maxhp%
					flag = ((fbl = mob_getfriendhpltmaxrate(md, ms[i].cond2)) != NULL); break;
				case MSC_FRIENDSTATUSON:	// friend status[num] on
				case MSC_FRIENDSTATUSOFF:	// friend status[num] off
					flag = ((fmd = mob_getfriendstatus(md, ms[i].cond1, ms[i].cond2)) != NULL); break;					
				case MSC_SLAVELT:		// slave < num
					flag = (mob_countslave(md) < c2 ); break;
				case MSC_ATTACKPCGT:	// attack pc > num
					flag = (battle_counttargeted(&md->bl, NULL, 0) > c2); break;
				case MSC_SLAVELE:		// slave <= num
					flag = (mob_countslave(md) <= c2 ); break;
				case MSC_ATTACKPCGE:	// attack pc >= num
					flag = (battle_counttargeted(&md->bl, NULL, 0) >= c2); break;
				case MSC_AFTERSKILL:
					flag = (md->skillid == c2); break;
				case MSC_SKILLUSED:		// specificated skill used
					flag = ((event & 0xffff) == MSC_SKILLUSED && ((event >> 16) == c2 || c2 == 0)); break;
				case MSC_RUDEATTACKED:
					flag = (!md->attacked_id && md->attacked_count > 0);
					if (flag) md->attacked_count = 0;	//Rude attacked count should be reset after the skill condition is met. Thanks to Komurka [Skotlex]
					break;
				case MSC_MASTERHPLTMAXRATE:
					flag = ((fbl = mob_getmasterhpltmaxrate(md, ms[i].cond2)) != NULL); break;
				case MSC_MASTERATTACKED:
					flag = (md->master_id > 0 && battle_counttargeted(map_id2bl(md->master_id), NULL, 0) > 0); break;
				case MSC_ALCHEMIST:
					flag = (md->state.alchemist); break;
			}
		}

		// 確率判定
		if (flag && rand() % 10000 < ms[i].permillage) //Lupus (max value = 10000)
		{
			if (skill_get_inf(ms[i].skill_id) & INF_GROUND_SKILL) {
				// 場所指定
				struct block_list *bl = NULL;
				int x = 0, y = 0;
				if (ms[i].target <= MST_AROUND) {
					switch (ms[i].target) {
						case MST_TARGET:
						case MST_AROUND5:
							bl = map_id2bl(md->target_id);
							break;
						case MST_FRIEND:
							if (fbl)
							{
								bl = fbl;
								break;
							} else if (fmd) {
								bl= &fmd->bl;
								break;
							} // else fall through
						default:
							bl = &md->bl;
							break;
					}
					if (bl != NULL) {
						x = bl->x; y=bl->y;
					}
				}
				if (x <= 0 || y <= 0)
					continue;
				// 自分の周囲
				if (ms[i].target >= MST_AROUND1) {
					int bx = x, by = y, i = 0, m = bl->m, r = ms[i].target-MST_AROUND1;
					do {
						bx = x + rand() % (r*2+3) - r;
						by = y + rand() % (r*2+3) - r;
					} while ((
						//bx <= 0 || by <= 0 || bx >= map[m].xs || by >= map[m].ys ||	// checked in getcell
						map_getcell(m, bx, by, CELL_CHKNOPASS)) && (i++) < 1000);
					if (i < 1000){
						x = bx; y = by;
					}
				}
				// 相手の周囲
				if (ms[i].target >= MST_AROUND5) {
					int bx = x, by = y, i = 0, m = bl->m, r = (ms[i].target-MST_AROUND5) + 1;
					do {
						bx = x + rand() % (r*2+1) - r;
						by = y + rand() % (r*2+1) - r;
					} while ((
						//bx <= 0 || by <= 0 || bx >= map[m].xs || by >= map[m].ys ||	// checked in getcell
						map_getcell(m, bx, by, CELL_CHKNOPASS)) && (i++) < 1000);
					if (i < 1000){
						x = bx; y = by;
					}
				}
				if (!mobskill_use_pos(md, x, y, i))
					return 0;
			} else {
				// ID指定
				if (ms[i].target <= MST_FRIEND) {
					struct block_list *bl;
					switch (ms[i].target) {
						case MST_TARGET:
							bl = map_id2bl(md->target_id);
							break;
						case MST_FRIEND:
							if (fbl) {
								bl = fbl;
								break;
							} else if (fmd) {
								bl = &fmd->bl;
								break;
							} // else fall through
						default:
							bl = &md->bl;
							break;
					}
					if (bl && !mobskill_use_id(md, bl, i))
						return 0;
				}
			}
			if (ms[i].emotion >= 0)
				clif_emotion(&md->bl, ms[i].emotion);
			return 1;
		}
	}

	return 0;
}
/*==========================================
 * Skill use event processing
 *------------------------------------------
 */
int mobskill_event(struct mob_data *md, int flag)
{
	int tick = gettick();
	nullpo_retr(0, md);

	if (flag == -1 && mobskill_use(md, tick, MSC_CASTTARGETED))
		return 1;
	if ((flag & BF_SHORT) && mobskill_use(md, tick, MSC_CLOSEDATTACKED))
		return 1;
	if ((flag & BF_LONG) && mobskill_use(md, tick, MSC_LONGRANGEATTACKED))
		return 1;
	return 0;
}

/*==========================================
 * スキル用タイマー削除
 *------------------------------------------
 */
int mobskill_deltimer(struct mob_data *md )
{
	nullpo_retr(0, md);

	if( md->skilltimer!=-1 ){
		if( skill_get_inf( md->skillid )& INF_GROUND_SKILL )
			delete_timer( md->skilltimer, mobskill_castend_pos );
		else
			delete_timer( md->skilltimer, mobskill_castend_id );
		md->skilltimer=-1;
	}
	return 0;
}
//
// 初期化
//
/*==========================================
 * Since un-setting [ mob ] up was used, it is an initial provisional value setup.
 *------------------------------------------
 */
static int mob_makedummymobdb(int class_)
{
	if (mob_dummy != NULL)
	{
		if (mob_db(class_) == mob_dummy)
			return 1; //Using the mob_dummy data already. [Skotlex]
		if (class_ > 0 && class_ <= MAX_MOB_DB)
		{	//Remove the mob data so that it uses the dummy data instead.
			aFree(mob_db_data[class_]);
			mob_db_data[class_] = NULL;
		}
		return 0;
	}
	//Initialize dummy data.	
	mob_dummy = (struct mob_db*)aCalloc(1, sizeof(struct mob_db)); //Initializing the dummy mob.
	sprintf(mob_dummy->name,"DUMMY");
	sprintf(mob_dummy->jname,"Dummy");
	mob_dummy->lv=1;
	mob_dummy->max_hp=1000;
	mob_dummy->max_sp=1;
	mob_dummy->base_exp=2;
	mob_dummy->job_exp=1;
	mob_dummy->range=1;
	mob_dummy->atk1=7;
	mob_dummy->atk2=10;
	mob_dummy->def=0;
	mob_dummy->mdef=0;
	mob_dummy->str=1;
	mob_dummy->agi=1;
	mob_dummy->vit=1;
	mob_dummy->int_=1;
	mob_dummy->dex=6;
	mob_dummy->luk=2;
	mob_dummy->range2=10;
	mob_dummy->range3=10;
	mob_dummy->size=0;
	mob_dummy->race=0;
	mob_dummy->element=0;
	mob_dummy->mode=0;
	mob_dummy->speed=300;
	mob_dummy->adelay=1000;
	mob_dummy->amotion=500;
	mob_dummy->dmotion=500;
	return 0;
}

//Adjusts the drop rate of item according to the criteria given. [Skotlex]
static int mob_drop_adjust(int rate, int rate_adjust, int rate_min, int rate_max)
{
	if (battle_config.logarithmic_drops && rate_adjust > 0) //Logarithmic drops equation by Ishizu-Chan
		//Equation: Droprate(x,y) = x * (5 - log(x)) ^ (ln(y) / ln(5))
		//x is the normal Droprate, y is the Modificator.
		rate = (int)(rate * pow((5.0 - log10(rate)), (log(rate_adjust/100.) / log(5.0))) + 0.5);
	else	//Classical linear rate adjustment.
		rate = rate*rate_adjust/100;
	return (rate>rate_max)?rate_max:((rate<rate_min)?rate_min:rate);
}
/*==========================================
 * db/mob_db.txt reading
 *------------------------------------------
 */
static int mob_readdb(void)
{
	FILE *fp;
	char line[1024];
	char *filename[]={ "db/mob_db.txt","db/mob_db2.txt" };
	int class_, i, fi;

	for(fi=0;fi<2;fi++){

		fp=fopen(filename[fi],"r");
		if(fp==NULL){
			if(fi>0)
				continue;
			return -1;
		}
		while(fgets(line,1020,fp)){
			double exp, maxhp;
			char *str[60], *p, *np; // 55->60 Lupus

			if(line[0] == '/' && line[1] == '/')
				continue;

			for(i=0,p=line;i<60;i++){
				if((np=strchr(p,','))!=NULL){
					str[i]=p;
					*np=0;
					p=np+1;
				} else
					str[i]=p;
			}

			class_ = atoi(str[0]);
			if (class_ == 0)
				continue; //Leave blank lines alone... [Skotlex]

			if (class_ <= 1000 || class_ > MAX_MOB_DB)
			{
				ShowWarning("Mob with ID: %d not loaded. ID must be in range [%d-%d]\n", class_, 1000, MAX_MOB_DB);
				continue;
			} else if (class_ >= JOB_NOVICE_HIGH && class_ < MAX_PC_CLASS)
			{
				ShowWarning("Mob with ID: %d not loaded. That ID is reserved for player classes.\n");
				continue;
			}
			if (mob_db_data[class_] == NULL)
				mob_db_data[class_] = aCalloc(1, sizeof (struct mob_data));

			mob_db_data[class_]->view_class = class_;
			memcpy(mob_db_data[class_]->name, str[1], NAME_LENGTH-1);
			memcpy(mob_db_data[class_]->jname, str[2], NAME_LENGTH-1);
			mob_db_data[class_]->lv = atoi(str[3]);
			mob_db_data[class_]->max_hp = atoi(str[4]);
			mob_db_data[class_]->max_sp = atoi(str[5]);

			exp = (double)atoi(str[6]) * (double)battle_config.base_exp_rate / 100.;
			if (exp < 0) exp = 0;
			else if (exp > 0x7fffffff) exp = 0x7fffffff;
			mob_db_data[class_]->base_exp = (int)exp;

			exp = (double)atoi(str[7]) * (double)battle_config.job_exp_rate / 100.;
			if (exp < 0) exp = 0;
			else if (exp > 0x7fffffff) exp = 0x7fffffff;
			mob_db_data[class_]->job_exp = (int)exp;
			
			mob_db_data[class_]->range=atoi(str[8]);
			mob_db_data[class_]->atk1=atoi(str[9]);
			mob_db_data[class_]->atk2=atoi(str[10]);
			mob_db_data[class_]->def=atoi(str[11]);
			mob_db_data[class_]->mdef=atoi(str[12]);
			mob_db_data[class_]->str=atoi(str[13]);
			mob_db_data[class_]->agi=atoi(str[14]);
			mob_db_data[class_]->vit=atoi(str[15]);
			mob_db_data[class_]->int_=atoi(str[16]);
			mob_db_data[class_]->dex=atoi(str[17]);
			mob_db_data[class_]->luk=atoi(str[18]);
			mob_db_data[class_]->range2=atoi(str[19]);
			mob_db_data[class_]->range3=atoi(str[20]);
			mob_db_data[class_]->size=atoi(str[21]);
			mob_db_data[class_]->race=atoi(str[22]);
			mob_db_data[class_]->element=atoi(str[23]);
			mob_db_data[class_]->mode=atoi(str[24]);
			mob_db_data[class_]->speed=atoi(str[25]);
			mob_db_data[class_]->adelay=atoi(str[26]);
			mob_db_data[class_]->amotion=atoi(str[27]);
			mob_db_data[class_]->dmotion=atoi(str[28]);

			for(i=0;i<10;i++){ // 8 -> 10 Lupus
				int rate = 0,rate_adjust,type,ratemin,ratemax;
				struct item_data *id;
				mob_db_data[class_]->dropitem[i].nameid=atoi(str[29+i*2]);
				type = itemdb_type(mob_db_data[class_]->dropitem[i].nameid);
				rate = atoi(str[30+i*2]);
				if (class_ >= 1324 && class_ <= 1363)
				{	//Treasure box drop rates [Skotlex]
					rate_adjust = battle_config.item_rate_treasure;
					ratemin = battle_config.item_drop_treasure_min;
					ratemax = battle_config.item_drop_treasure_max;
				}
				else switch (type)
				{
				case 0:
					rate_adjust = battle_config.item_rate_heal; 
					ratemin = battle_config.item_drop_heal_min;
					ratemax = battle_config.item_drop_heal_max;
					break;
				case 2:
					rate_adjust = battle_config.item_rate_use;
					ratemin = battle_config.item_drop_use_min;
					ratemax = battle_config.item_drop_use_max;
					break;
				case 4:
				case 5:
				case 8:		// Changed to include Pet Equip
					rate_adjust = battle_config.item_rate_equip;
					ratemin = battle_config.item_drop_equip_min;
					ratemax = battle_config.item_drop_equip_max;
					break;
				case 6:
					rate_adjust = battle_config.item_rate_card;
					ratemin = battle_config.item_drop_card_min;
					ratemax = battle_config.item_drop_card_max;
					break;
				default:
					rate_adjust = battle_config.item_rate_common;
					ratemin = battle_config.item_drop_common_min;
					ratemax = battle_config.item_drop_common_max;
					break;
				}
				mob_db_data[class_]->dropitem[i].p = mob_drop_adjust(rate, rate_adjust, ratemin, ratemax);

				//calculate and store Max available drop chance of the item
				id = itemdb_search(mob_db_data[class_]->dropitem[i].nameid);
				if (mob_db_data[class_]->dropitem[i].p) {
					if (id->maxchance==10000 || (id->maxchance < mob_db_data[class_]->dropitem[i].p) ) {
					//item has bigger drop chance or sold in shops
						id->maxchance = mob_db_data[class_]->dropitem[i].p;
					}			
				}
			}
			// MVP EXP Bonus, Chance: MEXP,ExpPer
			mob_db_data[class_]->mexp=atoi(str[49])*battle_config.mvp_exp_rate/100;
			mob_db_data[class_]->mexpper=atoi(str[50]);
			//Now that we know if it is an mvp or not,
			//apply battle_config modifiers [Skotlex]
			maxhp = (double)mob_db_data[class_]->max_hp;
			if (mob_db_data[class_]->mexp > 0)
			{	//Mvp
				if (battle_config.mvp_hp_rate != 100) 
					maxhp = maxhp * (double)battle_config.mvp_hp_rate /100.;
			} else if (battle_config.monster_hp_rate != 100) //Normal mob
				maxhp = maxhp * (double)battle_config.monster_hp_rate /100.;
			if (maxhp < 1) maxhp = 1;
			else if (maxhp > 0x7fffffff) maxhp = 0x7fffffff;
			mob_db_data[class_]->max_hp = (int)maxhp;

			// MVP Drops: MVP1id,MVP1per,MVP2id,MVP2per,MVP3id,MVP3per
			for(i=0;i<3;i++){
				struct item_data *id;
				int rate=atoi(str[52+i*2]);
				mob_db_data[class_]->mvpitem[i].nameid=atoi(str[51+i*2]);
				mob_db_data[class_]->mvpitem[i].p= mob_drop_adjust(rate, battle_config.item_rate_mvp,
					battle_config.item_drop_mvp_min, battle_config.item_drop_mvp_max);

				//calculate and store Max available drop chance of the MVP item
				id = itemdb_search(mob_db_data[class_]->mvpitem[i].nameid);
				if (mob_db_data[class_]->mvpitem[i].p) {
					if (id->maxchance==10000 || (id->maxchance < mob_db_data[class_]->mvpitem[i].p/10+1) ) {
					//item has bigger drop chance or sold in shops
						id->maxchance = mob_db_data[class_]->mvpitem[i].p/10+1; //reduce MVP drop info to not spoil common drop rate
					}			
				}
			}

			if (mob_db_data[class_]->max_hp <= 0) {
				ShowWarning ("Mob %d (%s) has no HP, using poring data for it\n", class_, mob_db_data[class_]->jname);
				mob_makedummymobdb(class_);
			}
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",filename[fi]);
	}
	return 0;
}

/*==========================================
 * MOB display graphic change data reading
 *------------------------------------------
 */
static int mob_readdb_mobavail(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int class_,j,k;
	char *str[20],*p,*np;

	if( (fp=fopen("db/mob_avail.txt","r"))==NULL ){
		ShowError("can't read db/mob_avail.txt\n");
		return -1;
	}

	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));

		for(j=0,p=line;j<12;j++){
			if((np=strchr(p,','))!=NULL){
				str[j]=p;
				*np=0;
				p=np+1;
			} else
				str[j]=p;
		}

		if(str[0]==NULL)
			continue;

		class_=atoi(str[0]);
		if (class_ == 0)
			continue; //Leave blank lines alone... [Skotlex]
		
		if(mob_db(class_) == mob_dummy)	// 値が異常なら処理しない。
			continue;

		k=atoi(str[1]);
		if(k < 0)
			continue;
		if (j > 3 && k > 23 && k < 69)
			k += 3977;	// advanced job/baby class
		mob_db_data[class_]->view_class=k;

		if((k < 24) || (k > 4000)) {
			mob_db_data[class_]->sex=atoi(str[2]);
			mob_db_data[class_]->hair=atoi(str[3]);
			mob_db_data[class_]->hair_color=atoi(str[4]);
			mob_db_data[class_]->weapon=atoi(str[5]);
			mob_db_data[class_]->shield=atoi(str[6]);
			mob_db_data[class_]->head_top=atoi(str[7]);
			mob_db_data[class_]->head_mid=atoi(str[8]);
			mob_db_data[class_]->head_buttom=atoi(str[9]);
			mob_db_data[class_]->option=atoi(str[10])&~0x46;
			mob_db_data[class_]->clothes_color=atoi(str[11]); // Monster player dye option - Valaris
		}
		else if(atoi(str[2]) > 0) mob_db_data[class_]->equip=atoi(str[2]); // mob equipment [Valaris]

		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/mob_avail.txt");
	return 0;
}

/*==========================================
 * Reading of random monster data
 *------------------------------------------
 */
static int mob_read_randommonster(void)
{
	FILE *fp;
	char line[1024];
	char *str[10],*p;
	int i,j;

	const char* mobfile[] = {
		"db/mob_branch.txt",
		"db/mob_poring.txt",
		"db/mob_boss.txt" };

	for(i=0;i<MAX_RANDOMMONSTER;i++){
		mob_db_data[0]->summonper[i] = 1002;	// 設定し忘れた場合はポリンが出るようにしておく
		fp=fopen(mobfile[i],"r");
		if(fp==NULL){
			ShowError("can't read %s\n",mobfile[i]);
			return -1;
		}
		while(fgets(line,1020,fp)){
			int class_,per;
			if(line[0] == '/' && line[1] == '/')
				continue;
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<3 && p;j++){
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}

			if(str[0]==NULL || str[2]==NULL)
				continue;

			class_ = atoi(str[0]);
			per=atoi(str[2]);
			if(mob_db(class_) != mob_dummy)
				mob_db_data[class_]->summonper[i]=per;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",mobfile[i]);
	}
	return 0;
}

/*==========================================
 * db/mob_skill_db.txt reading
 *------------------------------------------
 */
static int mob_readskilldb(void)
{
	FILE *fp;
	char line[1024];
	int i;

	const struct {
		char str[32];
		int id;
	} cond1[] = {
		{	"always",			MSC_ALWAYS				},
		{	"myhpltmaxrate",	MSC_MYHPLTMAXRATE		},
		{	"friendhpltmaxrate",MSC_FRIENDHPLTMAXRATE	},
		{	"mystatuson",		MSC_MYSTATUSON			},
		{	"mystatusoff",		MSC_MYSTATUSOFF			},
		{	"friendstatuson",	MSC_FRIENDSTATUSON		},
		{	"friendstatusoff",	MSC_FRIENDSTATUSOFF		},
		{	"attackpcgt",		MSC_ATTACKPCGT			},
		{	"attackpcge",		MSC_ATTACKPCGE			},
		{	"slavelt",			MSC_SLAVELT				},
		{	"slavele",			MSC_SLAVELE				},
		{	"closedattacked",	MSC_CLOSEDATTACKED		},
		{	"longrangeattacked",MSC_LONGRANGEATTACKED	},
		{	"skillused",		MSC_SKILLUSED			},
		{	"afterskill",		MSC_AFTERSKILL			},
		{	"casttargeted",		MSC_CASTTARGETED		},
		{	"rudeattacked",		MSC_RUDEATTACKED		},
		{	"masterhpltmaxrate",MSC_MASTERHPLTMAXRATE	},
		{	"masterattacked",	MSC_MASTERATTACKED		},
		{	"alchemist",		MSC_ALCHEMIST			},
	}, cond2[] ={
		{	"anybad",		-1				},
		{	"stone",		SC_STONE		},
		{	"freeze",		SC_FREEZE		},
		{	"stan",			SC_STAN			},
		{	"sleep",		SC_SLEEP		},
		{	"poison",		SC_POISON		},
		{	"curse",		SC_CURSE		},
		{	"silence",		SC_SILENCE		},
		{	"confusion",	SC_CONFUSION	},
		{	"blind",		SC_BLIND		},
		{	"hiding",		SC_HIDING		},
		{	"sight",		SC_SIGHT		},
	}, state[] = {
		{	"any",		-1			},
		{	"idle",		MSS_IDLE	},
		{	"walk",		MSS_WALK	},
		{	"attack",	MSS_ATTACK	},
		{	"dead",		MSS_DEAD	},
		{	"loot",		MSS_LOOT	},
		{	"chase",	MSS_CHASE	},
	}, target[] = {
		{	"target",	MST_TARGET	},
		{	"self",		MST_SELF	},
		{	"friend",	MST_FRIEND	},
		{	"around5",	MST_AROUND5	},
		{	"around6",	MST_AROUND6	},
		{	"around7",	MST_AROUND7	},
		{	"around8",	MST_AROUND8	},
		{	"around1",	MST_AROUND1	},
		{	"around2",	MST_AROUND2	},
		{	"around3",	MST_AROUND3	},
		{	"around4",	MST_AROUND4	},
		{	"around",	MST_AROUND	},
	};

	int x;
	char *filename[]={ "db/mob_skill_db.txt","db/mob_skill_db2.txt" };

	for(x=0;x<2;x++){

		fp=fopen(filename[x],"r");
		if(fp==NULL){
			if(x==0)
				ShowError("can't read %s\n",filename[x]);
			continue;
		}
		while(fgets(line,1020,fp)){
			char *sp[20],*p;
			int mob_id;
			struct mob_skill *ms, gms;
			int j=0;

			if(line[0] == '/' && line[1] == '/')
				continue;

			memset(sp,0,sizeof(sp));
			for(i=0,p=line;i<18 && p;i++){
				sp[i]=p;
				if((p=strchr(p,','))!=NULL)
					*p++=0;
			}
			if( (mob_id=atoi(sp[0]))== 0 || (mob_id > 0 && mob_db(mob_id) == mob_dummy))
				continue;

			if( strcmp(sp[1],"clear")==0 ){
				if (mob_id < 0)
					continue;
				memset(mob_db_data[mob_id]->skill,0,sizeof(struct mob_skill));
					mob_db_data[mob_id]->maxskill=0;
				continue;
			}

			if (mob_id < 0)
			{	//Prepare global skill. [Skotlex]
				memset(&gms, 0, sizeof (struct mob_skill));
				ms = &gms;
			} else {			
				for(i=0;i<MAX_MOBSKILL;i++)
					if( (ms=&mob_db_data[mob_id]->skill[i])->skill_id == 0)
						break;
				if(i==MAX_MOBSKILL){
					ShowWarning("mob_skill: readdb: too many skill ! [%s] in %d[%s]\n",
						sp[1],mob_id,mob_db_data[mob_id]->jname);
					continue;
				}
			}

			ms->state=atoi(sp[2]);
			for(j=0;j<sizeof(state)/sizeof(state[0]);j++){
				if( strcmp(sp[2],state[j].str)==0)
					ms->state=state[j].id;
			}

			//Skill ID
			j=atoi(sp[3]);
			if (j<=0 || j>MAX_SKILL_DB) //fixed Lupus
			{
				if (mob_id < 0)
					ShowWarning("Invalid Skill ID (%d) for all mobs\n", j);
				else
					ShowWarning("Invalid Skill ID (%d) for mob %d (%s)\n", j, mob_id, mob_db_data[mob_id]->jname);
				continue;
			}
			ms->skill_id=j;
			//Skill lvl
			j= atoi(sp[4])<=0 ? 1 : atoi(sp[4]);
			ms->skill_lv= j>battle_config.mob_max_skilllvl ? battle_config.mob_max_skilllvl : j; //we strip max skill level

			//Apply battle_config modifiers to rate (permillage) and delay [Skotlex]
			ms->permillage=atoi(sp[5]);
			if (battle_config.mob_skill_rate != 100)
				ms->permillage = ms->permillage*battle_config.mob_skill_rate/100;
			ms->casttime=atoi(sp[6]);
			ms->delay=atoi(sp[7]);
			if (battle_config.mob_skill_delay != 100)
				ms->delay = ms->delay*battle_config.mob_skill_delay/100;
			ms->cancel=atoi(sp[8]);
			if( strcmp(sp[8],"yes")==0 ) ms->cancel=1;
			ms->target=atoi(sp[9]);
			for(j=0;j<sizeof(target)/sizeof(target[0]);j++){
				if( strcmp(sp[9],target[j].str)==0)
					ms->target=target[j].id;
			}
			ms->cond1=-1;
			for(j=0;j<sizeof(cond1)/sizeof(cond1[0]);j++){
				if( strcmp(sp[10],cond1[j].str)==0)
					ms->cond1=cond1[j].id;
			}
			ms->cond2=atoi(sp[11]);
			for(j=0;j<sizeof(cond2)/sizeof(cond2[0]);j++){
				if( strcmp(sp[11],cond2[j].str)==0)
					ms->cond2=cond2[j].id;
			}
			ms->val[0]=atoi(sp[12]);
			ms->val[1]=atoi(sp[13]);
			ms->val[2]=atoi(sp[14]);
			ms->val[3]=atoi(sp[15]);
			ms->val[4]=atoi(sp[16]);
			if(sp[17] != NULL && strlen(sp[17])>2)
				ms->emotion=atoi(sp[17]);
			else
				ms->emotion=-1;
			if (mob_id < 0)
			{	//Set this skill to ALL mobs. [Skotlex]
				mob_id *= -1;
				for (i = 1; i < MAX_MOB_DB; i++)
				{
					if (mob_db_data[i] == NULL)
						continue;
					if (mob_db_data[i]->mode&MD_BOSS)
					{
						if (!(mob_id&2)) //Skill not for bosses
							continue;
					} else
						if (!(mob_id&1)) //Skill not for normal enemies.
							continue;
					
					for(j=0;j<MAX_MOBSKILL;j++)
						if( mob_db_data[i]->skill[j].skill_id == 0)
							break;
					if(j==MAX_MOBSKILL)
						continue;

					memcpy (&mob_db_data[i]->skill[j], ms, sizeof(struct mob_skill));
					mob_db_data[i]->maxskill=j+1;
				}
			} else //Skill set on a single mob.
				mob_db_data[mob_id]->maxskill=i+1;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n",filename[x]);
	}
	return 0;
}
/*==========================================
 * db/mob_race_db.txt reading
 *------------------------------------------
 */
static int mob_readdb_race(void)
{
	FILE *fp;
	char line[1024];
	int race,j,k;
	char *str[20],*p,*np;

	if( (fp=fopen("db/mob_race2_db.txt","r"))==NULL ){
		ShowError("can't read db/mob_race2_db.txt\n");
		return -1;
	}
	
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));

		for(j=0,p=line;j<12;j++){
			if((np=strchr(p,','))!=NULL){
				str[j]=p;
				*np=0;
				p=np+1;
			} else
				str[j]=p;
		}
		if(str[0]==NULL)
			continue;

		race=atoi(str[0]);
		if (race < 0 || race >= MAX_MOB_RACE_DB)
			continue;

		for (j=1; j<20; j++) {
			if (!str[j])
				break;
			k=atoi(str[j]);
			if (mob_db(k) == mob_dummy)
				continue;
			mob_db_data[k]->race2 = race;
		}
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/mob_race2_db.txt");
	return 0;
}

#ifndef TXT_ONLY
/*==========================================
 * SQL reading
 *------------------------------------------
 */
static int mob_read_sqldb(void)
{
	const char unknown_str[NAME_LENGTH] ="unknown";
	int i, fi, class_;
	double exp, maxhp;
	long unsigned int ln = 0;
	char *mob_db_name[] = { mob_db_db, mob_db2_db };

	//For easier handling of converting. [Skotlex]
#define TO_INT(a) (sql_row[a]==NULL?0:atoi(sql_row[a]))
#define TO_STR(a) (sql_row[a]==NULL?unknown_str:sql_row[a])
	
    for (fi = 0; fi < 2; fi++) {
		sprintf (tmp_sql, "SELECT * FROM `%s`", mob_db_name[fi]);
		if (mysql_query(&mmysql_handle, tmp_sql)) {
			ShowSQL("DB error (%s) - %s\n", mob_db_name[fi], mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			continue;
		}
		sql_res = mysql_store_result(&mmysql_handle);
		if (sql_res) {
			while((sql_row = mysql_fetch_row(sql_res))){
				class_ = TO_INT(0);
				if (class_ <= 1000 || class_ > MAX_MOB_DB)
				{
					ShowWarning("Mob with ID: %d not loaded. ID must be in range [%d-%d]\n", class_, 1000, MAX_MOB_DB);
					continue;
				} else if (class_ >= JOB_NOVICE_HIGH && class_ <= JOB_SUPER_BABY)
				{
					ShowWarning("Mob with ID: %d not loaded. That ID is reserved for Upper Classes.\n");
					continue;
				}
				if (mob_db_data[class_] == NULL)
					mob_db_data[class_] = aCalloc(1, sizeof (struct mob_data));
				
				ln++;

				mob_db_data[class_]->view_class = class_;
				memcpy(mob_db_data[class_]->name, TO_STR(1), NAME_LENGTH-1);
				memcpy(mob_db_data[class_]->jname, TO_STR(2), NAME_LENGTH-1);
				mob_db_data[class_]->lv = TO_INT(3);
				mob_db_data[class_]->max_hp = TO_INT(4);
				mob_db_data[class_]->max_sp = TO_INT(5);

				exp = (double)TO_INT(6) * (double)battle_config.base_exp_rate / 100.;
				if (exp < 0) exp = 0;
				else if (exp > 0x7fffffff) exp = 0x7fffffff;
				mob_db_data[class_]->base_exp = (int)exp;

				exp = (double)TO_INT(7) * (double)battle_config.job_exp_rate / 100.;
				if (exp < 0) exp = 0;
				else if (exp > 0x7fffffff) exp = 0x7fffffff;
				mob_db_data[class_]->job_exp = (int)exp;
				
				mob_db_data[class_]->range = TO_INT(8);
				mob_db_data[class_]->atk1 = TO_INT(9);
				mob_db_data[class_]->atk2 = TO_INT(10);
				mob_db_data[class_]->def = TO_INT(11);
				mob_db_data[class_]->mdef = TO_INT(12);
				mob_db_data[class_]->str = TO_INT(13);
				mob_db_data[class_]->agi = TO_INT(14);
				mob_db_data[class_]->vit = TO_INT(15);
				mob_db_data[class_]->int_ = TO_INT(16);
				mob_db_data[class_]->dex = TO_INT(17);
				mob_db_data[class_]->luk = TO_INT(18);
				mob_db_data[class_]->range2 = TO_INT(19);
				mob_db_data[class_]->range3 = TO_INT(20);
				mob_db_data[class_]->size = TO_INT(21);
				mob_db_data[class_]->race = TO_INT(22);
				mob_db_data[class_]->element = TO_INT(23);
				mob_db_data[class_]->mode = TO_INT(24);
				mob_db_data[class_]->speed = TO_INT(25);
				mob_db_data[class_]->adelay = TO_INT(26);
				mob_db_data[class_]->amotion = TO_INT(27);
				mob_db_data[class_]->dmotion = TO_INT(28);

				for (i = 0; i < 10; i++){ // 8 -> 10 Lupus
					int rate = 0, rate_adjust, type, ratemin, ratemax;
					struct item_data *id;
					mob_db_data[class_]->dropitem[i].nameid=TO_INT(29+i*2);
					type = itemdb_type(mob_db_data[class_]->dropitem[i].nameid);
					rate = TO_INT(30+i*2);
					if (class_ >= 1324 && class_ <= 1363)
					{	//Treasure box drop rates [Skotlex]
						rate_adjust = battle_config.item_rate_treasure;
						ratemin = battle_config.item_drop_treasure_min;
						ratemax = battle_config.item_drop_treasure_max;
					}
					else switch(type)
					{
					case 0:							// Added by Valaris
						rate_adjust = battle_config.item_rate_heal;
						ratemin = battle_config.item_drop_heal_min;
						ratemax = battle_config.item_drop_heal_max;
						break;
					case 2:
						rate_adjust = battle_config.item_rate_use;
						ratemin = battle_config.item_drop_use_min;
						ratemax = battle_config.item_drop_use_max;	// End
						break;
					case 4:
					case 5:
					case 8:	// Changed to include Pet Equip
						rate_adjust = battle_config.item_rate_equip;
						ratemin = battle_config.item_drop_equip_min;
						ratemax = battle_config.item_drop_equip_max;
						break;
					case 6:
						rate_adjust = battle_config.item_rate_card;
						ratemin = battle_config.item_drop_card_min;
						ratemax = battle_config.item_drop_card_max;
						break;
					default:
						rate_adjust = battle_config.item_rate_common;
						ratemin = battle_config.item_drop_common_min;
						ratemax = battle_config.item_drop_common_max;
						break;
					}
					mob_db_data[class_]->dropitem[i].p = mob_drop_adjust(rate, rate_adjust, ratemin, ratemax);

					//calculate and store Max available drop chance of the item
					id = itemdb_search(mob_db_data[class_]->dropitem[i].nameid);
					if (mob_db_data[class_]->dropitem[i].p) {
						if (id->maxchance==10000 || (id->maxchance < mob_db_data[class_]->dropitem[i].p) ) {
						//item has bigger drop chance or sold in shops
							id->maxchance = mob_db_data[class_]->dropitem[i].p;
						}			
					}
				}
				// MVP EXP Bonus, Chance: MEXP,ExpPer
				mob_db_data[class_]->mexp = TO_INT(49) * battle_config.mvp_exp_rate / 100;
				mob_db_data[class_]->mexpper = TO_INT(50);
				//Now that we know if it is an mvp or not,
				//apply battle_config modifiers [Skotlex]
				maxhp = (double)mob_db_data[class_]->max_hp;
				if (mob_db_data[class_]->mexp > 0)
				{	//Mvp
					if (battle_config.mvp_hp_rate != 100) 
						maxhp = maxhp * (double)battle_config.mvp_hp_rate /100.;
				} else if (battle_config.monster_hp_rate != 100) //Normal mob
					maxhp = maxhp * (double)battle_config.monster_hp_rate /100.;
				if (maxhp < 0) maxhp = 1;
				else if (maxhp > 0x7fffffff) maxhp = 0x7fffffff;
				mob_db_data[class_]->max_hp = (int)maxhp;

				// MVP Drops: MVP1id,MVP1per,MVP2id,MVP2per,MVP3id,MVP3per
				for (i=0; i<3; i++) {
					struct item_data *id;
					mob_db_data[class_]->mvpitem[i].nameid = TO_INT(51+i*2);
					mob_db_data[class_]->mvpitem[i].p = mob_drop_adjust(TO_INT(52+i*2),
						battle_config.item_rate_mvp, battle_config.item_drop_mvp_min, battle_config.item_drop_mvp_max);

					//calculate and store Max available drop chance of the MVP item
					id = itemdb_search(mob_db_data[class_]->mvpitem[i].nameid);
					if (mob_db_data[class_]->mvpitem[i].p) {
						if (id->maxchance==10000 || (id->maxchance < mob_db_data[class_]->mvpitem[i].p/10+1) ) {
						//item has bigger drop chance or sold in shops
							id->maxchance = mob_db_data[class_]->mvpitem[i].p/10+1; //reduce MVP drop info to not spoil common drop rate
						}			
					}
				}
				if (mob_db_data[class_]->max_hp <= 0) {
					ShowWarning ("Mob %d (%s) has no HP, using poring data for it\n", class_, mob_db_data[class_]->jname);
					mob_makedummymobdb(class_);
				}
			}

			mysql_free_result(sql_res);
			ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", ln, mob_db_name[fi]);
			ln = 0;
		}
	}
	return 0;
}
#endif /* not TXT_ONLY */

void mob_reload(void)
{
	int i;
#ifndef TXT_ONLY
    if(db_use_sqldbs)
        mob_read_sqldb();
    else
#endif /* TXT_ONLY */
	mob_readdb();

	mob_readdb_mobavail();
	mob_read_randommonster();

	//Mob skills need to be cleared before re-reading them. [Skotlex]
	for (i = 0; i < MAX_MOB_DB; i++)
		if (mob_db_data[i])
		{
			memset(&mob_db_data[i]->skill,0,sizeof(mob_db_data[i]->skill));
			mob_db_data[i]->maxskill=0;
		}
	mob_readskilldb();
	mob_readdb_race();
}

/*==========================================
 * Circumference initialization of mob
 *------------------------------------------
 */
int do_init_mob(void)
{	//Initialize the mob database
	memset(mob_db_data,0,sizeof(mob_db_data)); //Clear the array
	mob_db_data[0] = aCalloc(1, sizeof (struct mob_data));	//This mob is used for random spawns
	mob_makedummymobdb(0); //The first time this is invoked, it creates the dummy mob

#ifndef TXT_ONLY
    if(db_use_sqldbs)
        mob_read_sqldb();
    else
#endif /* TXT_ONLY */
        mob_readdb();

	mob_readdb_mobavail();
	mob_read_randommonster();
	mob_readskilldb();
	mob_readdb_race();

	add_timer_func_list(mob_timer,"mob_timer");
	add_timer_func_list(mob_delayspawn,"mob_delayspawn");
	add_timer_func_list(mob_delay_item_drop,"mob_delay_item_drop");
	add_timer_func_list(mob_ai_hard,"mob_ai_hard");
	add_timer_func_list(mob_ai_lazy,"mob_ai_lazy");
	add_timer_func_list(mobskill_castend_id,"mobskill_castend_id");
	add_timer_func_list(mobskill_castend_pos,"mobskill_castend_pos");
	add_timer_func_list(mob_timer_delete,"mob_timer_delete");
	add_timer_func_list(mob_spawn_guardian_sub,"mob_spawn_guardian_sub");
	add_timer_interval(gettick()+MIN_MOBTHINKTIME,mob_ai_hard,0,0,MIN_MOBTHINKTIME);
	add_timer_interval(gettick()+MIN_MOBTHINKTIME*10,mob_ai_lazy,0,0,MIN_MOBTHINKTIME*10);

	return 0;
}

/*==========================================
 * Clean memory usage.
 *------------------------------------------
 */
int do_final_mob(void)
{
	int i;
	if (mob_dummy)
	{
		aFree(mob_dummy);
		mob_dummy = NULL;
	}
	for (i = 0; i <= MAX_MOB_DB; i++)
	{
		if (mob_db_data[i] != NULL)
		{
			aFree(mob_db_data[i]);
			mob_db_data[i] = NULL;
		}
	}

	return 0;
}
