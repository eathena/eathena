// $Id: battle.c,v 1.10 2004/09/29 21:08:17 Akitasha Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "battle.h"

#include "timer.h"
#include "nullpo.h"
#include "malloc.h"

#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "itemdb.h"
#include "clif.h"
#include "pet.h"
#include "guild.h"

#define	is_boss(bl)	status_get_mexp(bl)	// Can refine later [Aru]

int attr_fix_table[4][10][10];

struct Battle_Config battle_config;

/*==========================================
 * 二点間の距離を返す
 * 戻りは整数で0以上
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
 * 自分をロックしているMOBの?を?える(foreachclient)
 *------------------------------------------
 */
static int battle_counttargeted_sub(struct block_list *bl, va_list ap)
{
	int id, *c, target_lv;
	struct block_list *src;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	id = va_arg(ap,int);
	nullpo_retr(0, c = va_arg(ap, int *));
	src = va_arg(ap,struct block_list *);
	target_lv = va_arg(ap,int);

	if (id == bl->id || (src && id == src->id))
		return 0;
	if (bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if (sd && sd->attacktarget == id && sd->attacktimer != -1 && sd->attacktarget_lv >= target_lv)
			(*c)++;
	}
	else if (bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if (md && md->target_id == id && md->timer != -1 && md->state.state == MS_ATTACK && md->target_lv >= target_lv)		
			(*c)++;
		//printf("md->target_lv:%d, target_lv:%d\n", md->target_lv, target_lv);
	}
	else if (bl->type == BL_PET) {
		struct pet_data *pd = (struct pet_data *)bl;
		if (pd && pd->target_id == id && pd->timer != -1 && pd->state.state == MS_ATTACK && pd->target_lv >= target_lv)
			(*c)++;
	}

	return 0;
}
/*==========================================
 * 自分をロックしている対象の数を返す(汎用)
 * 戻りは整数で0以上
 *------------------------------------------
 */
int battle_counttargeted(struct block_list *bl,struct block_list *src,int target_lv)
{
	int c = 0;
	nullpo_retr(0, bl);

	map_foreachinarea(battle_counttargeted_sub, bl->m,
		bl->x-AREA_SIZE, bl->y-AREA_SIZE,
		bl->x+AREA_SIZE, bl->y+AREA_SIZE, 0,
		bl->id, &c, src, target_lv);

	return c;
}

/*==========================================
 * Get random targetting enemy
 *------------------------------------------
 */
static int battle_gettargeted_sub(struct block_list *bl, va_list ap)
{
	struct block_list **bl_list;
	struct block_list *target;
	int *c;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	bl_list = va_arg(ap, struct block_list **);
	nullpo_retr(0, c = va_arg(ap, int *));
	nullpo_retr(0, target = va_arg(ap, struct block_list *));

	if (bl->id == target->id)
		return 0;
	if (*c >= 24)
		return 0;

	if (bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if (!sd || sd->attacktarget != target->id || sd->attacktimer == -1)
			return 0;
	} else if (bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if (!md || md->target_id != target->id || md->timer == -1 || md->state.state != MS_ATTACK)
			return 0;
	} else if (bl->type == BL_PET) {
		struct pet_data *pd = (struct pet_data *)bl;
		if (!pd || pd->target_id != target->id || pd->timer == -1 || pd->state.state != MS_ATTACK)
			return 0;
	}

	bl_list[(*c)++] = bl;
	return 0;
}
struct block_list* battle_gettargeted(struct block_list *target)
{
	struct block_list *bl_list[24];
	int c = 0;
	nullpo_retr(NULL, target);

	memset(bl_list, 0, sizeof(bl_list));
	map_foreachinarea(battle_gettargeted_sub, target->m,
		target->x-AREA_SIZE, target->y-AREA_SIZE,
		target->x+AREA_SIZE, target->y+AREA_SIZE, 0,
		bl_list, &c, target);
	if (c == 0 || c > 24)
		return NULL;
	return bl_list[rand()%c];
}

// ダメージの遅延
struct delay_damage {
	struct block_list *src;
	int target;
	int damage;
	int flag;
};
int battle_delay_damage_sub (int tid, unsigned int tick, int id, int data)
{
	struct delay_damage *dat = (struct delay_damage *)data;
	struct block_list *target = map_id2bl(dat->target);
	if (target && dat && map_id2bl(id) == dat->src && target->prev != NULL)
		battle_damage(dat->src, target, dat->damage, dat->flag);
	aFree(dat);
	return 0;
}
int battle_delay_damage (unsigned int tick, struct block_list *src, struct block_list *target, int damage, int flag)
{
	struct delay_damage *dat;
	nullpo_retr(0, src);
	nullpo_retr(0, target);

	if (!battle_config.delay_battle_damage) {
		battle_damage(src, target, damage, flag);
		return 0;
	}
	dat = (struct delay_damage *)aCalloc(1, sizeof(struct delay_damage));
	dat->src = src;
	dat->target = target->id;
	dat->damage = damage;
	dat->flag = flag;
	add_timer(tick, battle_delay_damage_sub, src->id, (int)dat);

	return 0;
}

// 実際にHPを操作
int battle_damage(struct block_list *bl,struct block_list *target,int damage,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change *sc_data;
	short *sc_count;
	int i;

	nullpo_retr(0, target); //blはNULLで呼ばれることがあるので他でチェック
	
	sc_data = status_get_sc_data(target);
	sc_count = status_get_sc_count(target);

	if (damage == 0 ||
		target->prev == NULL ||
		target->type == BL_PET)
		return 0;

	if (bl) {
		if (bl->prev == NULL)
			return 0;
		if (bl->type == BL_PC) {
			nullpo_retr(0, sd = (struct map_session_data *)bl);
		}
	}

	if (damage < 0)
		return battle_heal(bl,target,-damage,0,flag);

	if (!flag && sc_count && *sc_count > 0) {
		// 凍結、石化、睡眠を消去
		if (sc_data[SC_FREEZE].timer != -1)
			status_change_end(target,SC_FREEZE,-1);
		if (sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2 == 0)
			status_change_end(target,SC_STONE,-1);
		if (sc_data[SC_SLEEP].timer != -1)
			status_change_end(target,SC_SLEEP,-1);
	}

	if (target->type == BL_MOB) {	// MOB
		struct mob_data *md = (struct mob_data *)target;
		if (md && md->skilltimer != -1 && md->state.skillcastcancel)	// 詠唱妨害
			skill_castcancel(target,0);
		return mob_damage(bl,md,damage,0);
	} else if (target->type == BL_PC) {	// PC
		struct map_session_data *tsd = (struct map_session_data *)target;
		if (!tsd)
			return 0;
		if (sc_data[SC_DEVOTION].val1) {	// ディボーションをかけられている
			struct map_session_data *sd2 = map_id2sd(tsd->sc_data[SC_DEVOTION].val1);
			if (sd2 && skill_devotion3(&sd2->bl, target->id)) {
				skill_devotion(sd2, target->id);
			} else if (sd2 && bl) {
				for (i = 0; i < 5; i++)
					if (sd2->dev.val1[i] == target->id) {
					//	clif_damage(bl, &sd2->bl, gettick(), 0, 0, damage, 0 , 0, 0);
					//	pc_damage(&sd2->bl, sd2, damage);
					//	return 0;
						bl = &sd2->bl;
						tsd = sd2;
					}
			}
		}

		if (tsd->skilltimer != -1) {	// 詠唱妨害
			// フェンカードや妨害されないスキルかの検査
			if ((!tsd->special_state.no_castcancel || map[bl->m].flag.gvg) && tsd->state.skillcastcancel &&
				!tsd->special_state.no_castcancel2)
				skill_castcancel(target,0);
		}
		return pc_damage(bl,tsd,damage);
	} else if (target->type == BL_SKILL)
		return skill_unit_ondamaged((struct skill_unit *)target, bl, damage, gettick());
	return 0;
}
int battle_heal(struct block_list *bl,struct block_list *target,int hp,int sp,int flag)
{
	nullpo_retr(0, target); //blはNULLで呼ばれることがあるので他でチェック

	if (target->type == BL_PET)
		return 0;
	if (target->type == BL_PC && pc_isdead((struct map_session_data *)target) )
		return 0;
	if (hp == 0 && sp == 0)
		return 0;

	if (hp < 0)
		return battle_damage(bl,target,-hp,flag);

	if (target->type == BL_MOB)
		return mob_heal((struct mob_data *)target,hp);
	else if (target->type == BL_PC)
		return pc_heal((struct map_session_data *)target,hp,sp);
	return 0;
}

// 攻撃停止
int battle_stopattack(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if (bl->type == BL_MOB)
		return mob_stopattack((struct mob_data*)bl);
	else if (bl->type == BL_PC)
		return pc_stopattack((struct map_session_data*)bl);
	else if (bl->type == BL_PET)
		return pet_stopattack((struct pet_data*)bl);
	return 0;
}
// 移動停止
int battle_stopwalking(struct block_list *bl,int type)
{
	nullpo_retr(0, bl);
	if (bl->type == BL_MOB)
		return mob_stop_walking((struct mob_data*)bl,type);
	else if (bl->type == BL_PC)
		return pc_stop_walking((struct map_session_data*)bl,type);
	else if (bl->type == BL_PET)
		return pet_stop_walking((struct pet_data*)bl,type);
	return 0;
}


/*==========================================
 * ダメージの属性修正
 *------------------------------------------
 */
int battle_attr_fix(int damage,int atk_elem,int def_elem)
{
	int def_type = def_elem % 10, def_lv = def_elem / 10 / 2;

	if (atk_elem < 0 || atk_elem > 9 ||
		def_type < 0 || def_type > 9 ||
		def_lv < 1 || def_lv > 4) {	// 属 性値がおかしいのでとりあえずそのまま返す
		if (battle_config.error_log)
			printf("battle_attr_fix: unknown attr type: atk=%d def_type=%d def_lv=%d\n",atk_elem,def_type,def_lv);
		return damage;
	}

	return damage*attr_fix_table[def_lv-1][atk_elem][def_type]/100;
}


/*==========================================
 * ダメージ最終計算
 *------------------------------------------
 */
int battle_calc_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,int skill_lv,int flag)
{
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	struct status_change *sc_data, *sc;
	short *sc_count;
	int class_;

	nullpo_retr(0, bl);

	class_ = status_get_class(bl);

	if (bl->type == BL_MOB) {
		nullpo_retr (0, md=(struct mob_data *)bl);
	} else if (bl->type == BL_PC) {
		nullpo_retr (0, sd=(struct map_session_data *)bl);
	}

	sc_data = status_get_sc_data(bl);
	sc_count = status_get_sc_count(bl);

	if (sc_count && *sc_count > 0) {
		if (sc_data[SC_SAFETYWALL].timer!=-1 && damage>0 && flag&BF_WEAPON &&
			flag&BF_SHORT && skill_num != NPC_GUIDEDATTACK) {
			// セーフティウォール
			struct skill_unit *unit;
			unit = (struct skill_unit *)sc_data[SC_SAFETYWALL].val2;
			if (unit) {
				if (unit->group && (--unit->group->val2)<=0)
					skill_delunit(unit);
				damage=0;
			} else {
				status_change_end(bl,SC_SAFETYWALL,-1);
			}
		}
		if(sc_data[SC_PNEUMA].timer!=-1 && damage>0 &&
			((flag&BF_WEAPON && flag&BF_LONG && skill_num != NPC_GUIDEDATTACK) ||
			(flag&BF_MISC && flag&BF_LONG) ||
			(flag&BF_MAGIC && skill_num == ASC_BREAKER))){ // [DracoRPG]
			// ニューマ
			damage=0;
		}

		if(sc_data[SC_ROKISWEIL].timer!=-1 && damage>0 &&
			flag&BF_MAGIC ){
			// ニューマ
			damage=0;
		}

		if(sc_data[SC_AETERNA].timer!=-1 && damage>0){	// レックスエーテルナ
			damage<<=1;
			status_change_end( bl,SC_AETERNA,-1 );
		}

		//属性場のダメージ増加
		if(sc_data[SC_VOLCANO].timer!=-1){	// ボルケーノ
			if(flag&BF_SKILL && skill_get_pl(skill_num)==3)
				//damage += damage*sc_data[SC_VOLCANO].val4/100;
				damage += damage * enchant_eff[sc_data[SC_VOLCANO].val1-1] /100;
			else if(!flag&BF_SKILL && status_get_attack_element(bl)==3)
				//damage += damage*sc_data[SC_VOLCANO].val4/100;
				damage += damage * enchant_eff[sc_data[SC_VOLCANO].val1-1] /100;
		}

		if(sc_data[SC_VIOLENTGALE].timer!=-1){	// バイオレントゲイル
			if(flag&BF_SKILL && skill_get_pl(skill_num)==4)
				//damage += damage*sc_data[SC_VIOLENTGALE].val4/100;
				damage += damage * enchant_eff[sc_data[SC_VIOLENTGALE].val1-1] /100;
			else if(!flag&BF_SKILL && status_get_attack_element(bl)==4)
				//damage += damage*sc_data[SC_VIOLENTGALE].val4/100;
				damage += damage * enchant_eff[sc_data[SC_VIOLENTGALE].val1-1] /100;
		}

		if(sc_data[SC_DELUGE].timer!=-1){	// デリュージ
			if(flag&BF_SKILL && skill_get_pl(skill_num)==1)
				//damage += damage*sc_data[SC_DELUGE].val4/100;
				damage += damage * enchant_eff[sc_data[SC_DELUGE].val1-1] /100;
			else if(!flag&BF_SKILL && status_get_attack_element(bl)==1)
				//damage += damage*sc_data[SC_DELUGE].val4/100;
				damage += damage * enchant_eff[sc_data[SC_DELUGE].val1-1] /100;
		}

		if(sc_data[SC_ENERGYCOAT].timer!=-1 && damage>0  && flag&BF_WEAPON){	// エナジーコート
			if(sd){
				if(sd->status.sp>0){
					int per = sd->status.sp * 5 / (sd->status.max_sp + 1);
					sd->status.sp -= sd->status.sp * (per * 5 + 10) / 1000;
					if( sd->status.sp < 0 ) sd->status.sp = 0;
					damage -= damage * ((per+1) * 6) / 100;
					clif_updatestatus(sd,SP_SP);
				}
				if(sd->status.sp<=0)
					status_change_end( bl,SC_ENERGYCOAT,-1 );
			}
			else
				damage -= damage * (sc_data[SC_ENERGYCOAT].val1 * 6) / 100;
		}

		if(sc_data[SC_KYRIE].timer!=-1 && damage > 0){	// キリエエレイソン
			sc=&sc_data[SC_KYRIE];
			sc->val2-=damage;
			if(flag&BF_WEAPON){
				if(sc->val2>=0)	damage=0;
				else damage=-sc->val2;
			}
			if((--sc->val3)<=0 || (sc->val2<=0) || skill_num == AL_HOLYLIGHT)
				status_change_end(bl, SC_KYRIE, -1);
		}

		if(sc_data[SC_BASILICA].timer!=-1 && damage > 0){
			// ニューマ
			damage=0;
		}
		if(sc_data[SC_LANDPROTECTOR].timer!=-1 && damage>0 && flag&BF_MAGIC){
			// ニューマ
			damage=0;
		}

		if(sc_data[SC_AUTOGUARD].timer != -1 && damage > 0 && flag&BF_WEAPON) {
			if(rand()%100 < sc_data[SC_AUTOGUARD].val2) {
				int delay;

				damage = 0;
				clif_skill_nodamage(bl,bl,CR_AUTOGUARD,sc_data[SC_AUTOGUARD].val1,1);
				// different delay depending on skill level [celest]
				if (sc_data[SC_AUTOGUARD].val1 <= 5)
					delay = 300;
				else if (sc_data[SC_AUTOGUARD].val1 > 5 && sc_data[SC_AUTOGUARD].val1 <= 9)
					delay = 200;
				else
					delay = 100;
				if(sd)
					sd->canmove_tick = gettick() + delay;
				else if(md)
					md->canmove_tick = gettick() + delay;
			}
		}
// -- moonsoul (chance to block attacks with new Lord Knight skill parrying)
//
		if(sc_data[SC_PARRYING].timer != -1 && damage > 0 && flag&BF_WEAPON) {
			if(rand()%100 < sc_data[SC_PARRYING].val2) {
				damage = 0;
				clif_skill_nodamage(bl,bl,LK_PARRYING,sc_data[SC_PARRYING].val1,1);
			}
		}
		// リジェクトソード
		if(sc_data[SC_REJECTSWORD].timer!=-1 && damage > 0 && flag&BF_WEAPON &&
			// Fixed the condition check [Aalye]
			(src->type==BL_MOB || (src->type==BL_PC && (((struct map_session_data *)src)->status.weapon == 1 ||
			((struct map_session_data *)src)->status.weapon == 2 ||
			((struct map_session_data *)src)->status.weapon == 3)))){
			if(rand()%100 < (15*sc_data[SC_REJECTSWORD].val1)){ //反射確率は15*Lv
				damage = damage*50/100;
				clif_damage(bl,src,gettick(),0,0,damage,0,0,0);
				battle_damage(bl,src,damage,0);
				//ダメージを与えたのは良いんだが、ここからどうして表示するんだかわかんねぇ
				//エフェクトもこれでいいのかわかんねぇ
				clif_skill_nodamage(bl,bl,ST_REJECTSWORD,sc_data[SC_REJECTSWORD].val1,1);
				if((--sc_data[SC_REJECTSWORD].val2)<=0)
					status_change_end(bl, SC_REJECTSWORD, -1);
			}
		}
		if(sc_data[SC_SPIDERWEB].timer!=-1 && damage > 0)	// [Celest]
			if ((flag&BF_SKILL && skill_get_pl(skill_num)==3) ||
				(!flag&BF_SKILL && status_get_attack_element(src)==3)) {
				damage<<=1;
				status_change_end(bl, SC_SPIDERWEB, -1);
			}

		if(sc_data[SC_FOGWALL].timer != -1 && flag&BF_MAGIC)
			if(rand()%100 < 75)
				damage = 0;
	}

	if(class_ == 1288 || class_ == 1287 || class_ == 1286 || class_ == 1285) {
		if(class_ == 1288 && (flag&BF_SKILL || skill_num == ASC_BREAKER || skill_num == PA_SACRIFICE))
			damage=0;
		if(src->type == BL_PC) {
			struct guild *g=guild_search(((struct map_session_data *)src)->status.guild_id);
			struct guild_castle *gc=guild_mapname2gc(map[bl->m].name);
			if(!((struct map_session_data *)src)->status.guild_id)
				damage=0;
			if(gc && agit_flag==0 && class_ != 1288)	// guardians cannot be damaged during non-woe [Valaris]
				damage=0;  // end woe check [Valaris]
			if(g == NULL)
				damage=0;//ギルド未加入ならダメージ無し
			else if((gc != NULL) && guild_isallied(g, gc))
				damage=0;//自占領ギルドのエンペならダメージ無し
			else if(g && guild_checkskill(g,GD_APPROVAL) <= 0)
				damage=0;//正規ギルド承認がないとダメージ無し
			else if (battle_config.guild_max_castles != 0 && guild_checkcastles(g)>=battle_config.guild_max_castles)
				damage = 0; // [MouseJstr]
			else if (g && gc && guild_check_alliance(gc->guild_id, g->guild_id, 0) == 1)
				return 0;
		}
		else damage = 0;
	}

	if (damage > 0) { // damage reductions
		if (map[bl->m].flag.gvg && skill_num != PA_PRESSURE) { //GvG
			if (bl->type == BL_MOB){	//defenseがあればダメージが減るらしい？
				struct guild_castle *gc = guild_mapname2gc(map[bl->m].name);
				if (gc) damage -= damage * (gc->defense / 100) * (battle_config.castle_defense_rate/100);
			}
#ifdef OLD_GVG_Damage
			if (flag & BF_WEAPON) {
				if (flag & BF_SHORT)
					damage = damage * battle_config.gvg_short_damage_rate/100;
				if (flag & BF_LONG)
					damage = damage * battle_config.gvg_long_damage_rate/100;
			}
			if (flag&BF_MAGIC)
				damage = damage * battle_config.gvg_magic_damage_rate/100;
			if (flag&BF_MISC && skill_num != CR_ACIDDEMONSTRATION)
				damage = damage * battle_config.gvg_misc_damage_rate/100;
#else
			if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
				if (flag&BF_WEAPON)
					damage = (damage * 6)/10;
				if (flag&BF_MAGIC)
					damage = damage/2;
				if (flag&BF_MISC)
					damage = (damage * 6)/10;
			} else { //Normal attacks get reductions based on range.
//				if (flag & BF_SHORT)
//					damage = damage * battle_config.gvg_short_damage_rate/100;
				if (flag & BF_LONG)
					damage = (damage * 3)/4;
			}

#endif
		} else if (map[bl->m].flag.pkmode && bl->type == BL_PC) {
			if (flag & BF_WEAPON) {
				if (flag & BF_SHORT)
					damage = damage * 80/100;
				if (flag & BF_LONG)
					damage = damage * 70/100;
			}
			if (flag & BF_MAGIC)
				damage = damage * 60/100;
			if(flag & BF_MISC)
				damage = damage * 60/100;
		}
		if(damage < 1) damage  = 1;
	}

	if(battle_config.skill_min_damage || flag&BF_MISC) {
		if(div_ < 255) {
			if(damage > 0 && damage < div_)
				damage = div_;
		}
		else if(damage > 0 && damage < 3)
			damage = 3;
	}

	if( md!=NULL && md->hp>0 && damage > 0 )	// 反撃などのMOBスキル判定
		mobskill_event(md,flag);

	return damage;
}

/*==========================================
 * HP/SP吸収の計算
 *------------------------------------------
 */
int battle_calc_drain(int damage, int rate, int per, int val)
{
	int diff = 0;

	if (damage <= 0 || rate <= 0)
		return 0;

	if (per && rand()%100 < rate) {
		diff = (damage * per) / 100;
		if (diff == 0) {
			if (per > 0)
				diff = 1;
			else
				diff = -1;
		}
	}

	if (val && rand()%100 < rate) {
		diff += val;
	}
	return diff;
}

/*==========================================
 * 修練ダメージ
 *------------------------------------------
 */
int battle_addmastery(struct map_session_data *sd,struct block_list *target,int dmg,int type)
{
	int damage,skill;
	int race=status_get_race(target);
	int weapon;
	damage = 0;

	nullpo_retr(0, sd);

	// デーモンベイン(+3 ～ +30) vs 不死 or 悪魔 (死人は含めない？)
	if((skill = pc_checkskill(sd,AL_DEMONBANE)) > 0 && (battle_check_undead(race,status_get_elem_type(target)) || race==6) )
		damage += (skill*(int)(3+(sd->status.base_level+1)*0.05));	// submitted by orn
		//damage += (skill * 3);

	// ビーストベイン(+4 ～ +40) vs 動物 or 昆虫
	if((skill = pc_checkskill(sd,HT_BEASTBANE)) > 0 && (race==2 || race==4) )
		damage += (skill * 4);

	if(type == 0)
		weapon = sd->weapontype1;
	else
		weapon = sd->weapontype2;
	switch(weapon)
	{
		case 0x01:	// 短剣 Knife
		case 0x02:	// 1HS
		{
			// 剣修練(+4 ～ +40) 片手剣 短剣含む
			if((skill = pc_checkskill(sd,SM_SWORD)) > 0) {
				damage += (skill * 4);
			}
			break;
		}
		case 0x03:	// 2HS
		{
			// 両手剣修練(+4 ～ +40) 両手剣
			if((skill = pc_checkskill(sd,SM_TWOHAND)) > 0) {
				damage += (skill * 4);
			}
			break;
		}
		case 0x04:	// 1HL
		case 0x05:	// 2HL
		{
			// 槍修練(+4 ～ +40,+5 ～ +50) 槍
			if((skill = pc_checkskill(sd,KN_SPEARMASTERY)) > 0) {
				if(!pc_isriding(sd))
					damage += (skill * 4);	// ペコに乗ってない
				else
					damage += (skill * 5);	// ペコに乗ってる
			}
			break;
		}
		case 0x06: // 片手斧
		case 0x07: // Axe by Tato
		{
			if((skill = pc_checkskill(sd,AM_AXEMASTERY)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x08:	// メイス
		{
			// メイス修練(+3 ～ +30) メイス
			if((skill = pc_checkskill(sd,PR_MACEMASTERY)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x09:	// なし?
			break;
		case 0x0a:	// 杖
			break;
		case 0x0b:	// 弓
			break;
		case 0x00:	// 素手 Bare Hands
		case 0x0c:	// Knuckles
		{
			// 鉄拳(+3 ～ +30) 素手,ナックル
			if((skill = pc_checkskill(sd,MO_IRONHAND)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x0d:	// Musical Instrument
		{
			// 楽器の練習(+3 ～ +30) 楽器
			if((skill = pc_checkskill(sd,BA_MUSICALLESSON)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x0e:	// Dance Mastery
		{
			// Dance Lesson Skill Effect(+3 damage for every lvl = +30) 鞭
			if((skill = pc_checkskill(sd,DC_DANCINGLESSON)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x0f:	// Book
		{
			// Advance Book Skill Effect(+3 damage for every lvl = +30) {
			if((skill = pc_checkskill(sd,SA_ADVANCEDBOOK)) > 0) {
				damage += (skill * 3);
			}
			break;
		}
		case 0x10:	// Katars
		{
			// カタール修練(+3 ～ +30) カタール
			if((skill = pc_checkskill(sd,AS_KATAR)) > 0) {
				//ソニックブロー時は別処理（1撃に付き1/8適応)
				damage += (skill * 3);
			}
			break;
		}
	}
	damage = dmg + damage;
	return (damage);
}

static struct Damage battle_calc_pet_weapon_attack(
	struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	struct pet_data *pd = (struct pet_data *)src;
	struct mob_data *tmd=NULL;
	int hitrate,flee,cri = 0,atkmin,atkmax;
	int luk,target_count = 1;
	int def1 = status_get_def(target);
	int def2 = status_get_def2(target);
	int t_vit = status_get_vit(target);
	struct Damage wd;
	int damage,damage2=0,type,div_,blewcount=skill_get_blewcount(skill_num,skill_lv);
	int flag,dmg_lv=0;
	int t_mode=0,t_race=0,t_size=1,s_race=0,s_ele=0;
	struct status_change *t_sc_data;
	int ignore_def_flag = 0;
	int div_flag=0;	// 0: total damage is to be divided by div_
					// 1: damage is distributed,and has to be multiplied by div_ [celest]

	//return前の処理があるので情報出力部のみ変更
	if( target == NULL || pd == NULL ){ //srcは内容に直接触れていないのでスルーしてみる
		nullpo_info(NLP_MARK);
		memset(&wd,0,sizeof(wd));
		return wd;
	}

	s_race=status_get_race(src);
	s_ele=status_get_attack_element(src);

	// ターゲット
	if(target->type == BL_MOB)
		tmd=(struct mob_data *)target;
	else {
		memset(&wd,0,sizeof(wd));
		return wd;
	}
	t_race=status_get_race( target );
	t_size=status_get_size( target );
	t_mode=status_get_mode( target );
	t_sc_data=status_get_sc_data( target );

	flag=BF_SHORT|BF_WEAPON|BF_NORMAL;	// 攻撃の種類の設定

	// 回避率計算、回避判定は後で
	flee = status_get_flee(target);
	if(battle_config.agi_penalty_type > 0 || battle_config.vit_penalty_type > 0)
		target_count += battle_counttargeted(target,src,battle_config.agi_penalty_count_lv);
	if(battle_config.agi_penalty_type > 0) {
		if(target_count >= battle_config.agi_penalty_count) {
			if(battle_config.agi_penalty_type == 1)
				flee = (flee * (100 - (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
			else if(battle_config.agi_penalty_type == 2)
				flee -= (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
			if(flee < 1) flee = 1;
		}
	}
	hitrate=(status_get_hit(src) + skill_gethitbonus(skill_num,skill_lv))- flee + 80;

	type=0;	// normal
	if (skill_num > 0) {
		div_ = skill_get_num(skill_num,skill_lv);
		if (div_ < 1) div_ = 1;	//Avoid the rare case where the db says div_ is 0 and below
	}
	else div_ = 1; // single attack

	luk=status_get_luk(src);

	if(battle_config.pet_str)
		damage = status_get_baseatk(src);
	else
		damage = 0;

	if(skill_num==HW_MAGICCRASHER){			/* マジッククラッシャーはMATKで殴る */
		atkmin = status_get_matk1(src);
		atkmax = status_get_matk2(src);
	}else{
	atkmin = status_get_atk(src);
	atkmax = status_get_atk2(src);
	}
	if(mob_db[pd->class_].range>3 )
		flag=(flag&~BF_RANGEMASK)|BF_LONG;

	if(atkmin > atkmax) atkmin = atkmax;

	cri = status_get_critical(src);
	cri -= status_get_luk(target) * 2; // luk/5*10 => target_luk*2 not target_luk*3
	if(battle_config.enemy_critical_rate != 100) {
		cri = cri*battle_config.enemy_critical_rate/100;
		if(cri < 1)
			cri = 1;
	}
	if(t_sc_data) {
		if (t_sc_data[SC_SLEEP].timer!=-1)
			cri <<=1;
		if(t_sc_data[SC_JOINTBEAT].timer != -1 &&
			t_sc_data[SC_JOINTBEAT].val2 == 5) // Always take crits with Neck broken by Joint Beat [DracoRPG]
			cri = 1000;
	}

	if(skill_num == 0 && battle_config.enemy_critical && (rand() % 1000) < cri)
	{
		damage += atkmax;
		type = 0x0a;
	}
	else {
		int vitbonusmax;

		if(atkmax > atkmin)
			damage += atkmin + rand() % (atkmax-atkmin + 1);
		else
			damage += atkmin ;
		// スキル修正１（攻撃力倍化系）
		// オーバートラスト(+5% ～ +25%),他攻撃系スキルの場合ここで補正
		// バッシュ,マグナムブレイク,
		// ボーリングバッシュ,スピアブーメラン,ブランディッシュスピア,スピアスタッブ,
		// メマーナイト,カートレボリューション
		// ダブルストレイフィング,アローシャワー,チャージアロー,
		// ソニックブロー
		if(skill_num>0){
			int i;
			if( (i=skill_get_pl(skill_num))>0 )
				s_ele=i;

			flag=(flag&~BF_SKILLMASK)|BF_SKILL;
			switch( skill_num ){
			case SM_BASH:		// バッシュ
				damage = damage*(100+ 30*skill_lv)/100;
				//hitrate = (hitrate*(100+5*skill_lv))/100;
				break;
			case SM_MAGNUM:		// マグナムブレイク
				damage = damage*(wflag > 1 ? 5*skill_lv+115 : 30*skill_lv+100)/100;
				//hitrate = (hitrate*(100+10*skill_lv))/100;
				break;
			case MC_MAMMONITE:	// メマーナイト
				damage = damage*(100+ 50*skill_lv)/100;
				break;
			case AC_DOUBLE:	// ダブルストレイフィング
				damage = damage*(180+ 20*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case AC_SHOWER:	// アローシャワー
				damage = damage*(75 + 5*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case AC_CHARGEARROW:	// チャージアロー
				damage = damage*150/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_AUTOCOUNTER:
				ignore_def_flag = 1;
				break;
			case KN_PIERCE:	// ピアース
				damage = damage*(100+ 10*skill_lv)/100;
				//hitrate = hitrate*(100+5*skill_lv)/100;
				div_=t_size+1;
				div_flag = 1;
				break;
			case KN_SPEARSTAB:	// スピアスタブ
				damage = damage*(100+ 15*skill_lv)/100;
				break;
			case KN_SPEARBOOMERANG:	// スピアブーメラン
				damage = damage*(100+ 50*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_BRANDISHSPEAR: // ブランディッシュスピア
				damage = damage*(100+ 20*skill_lv)/100;
				if(skill_lv>3 && wflag==1) damage2+=damage/2;
				if(skill_lv>6 && wflag==1) damage2+=damage/4;
				if(skill_lv>9 && wflag==1) damage2+=damage/8;
				if(skill_lv>6 && wflag==2) damage2+=damage/2;
				if(skill_lv>9 && wflag==2) damage2+=damage/4;
				if(skill_lv>9 && wflag==3) damage2+=damage/2;
				damage +=damage2;
				break;
			case KN_BOWLINGBASH:	// ボウリングバッシュ
				damage = damage*(100+ 50*skill_lv)/100;
				blewcount=0;
				break;
			case AS_GRIMTOOTH:
				damage = damage*(100+ 20*skill_lv)/100;
				break;
			case AS_POISONREACT: // celest
				s_ele = 0;
				damage = damage*(100+ 30*skill_lv)/100;
				break;
			case AS_SONICBLOW:	// ソニックブロウ
				damage = damage*(300+ 50*skill_lv)/100;
				break;
			case TF_SPRINKLESAND:	// 砂まき
				damage = damage*125/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case ITM_TOMAHAWK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case MC_CARTREVOLUTION:	// カートレボリューション
				damage = (damage*150)/100;
				break;
			// 以下MOB
			case NPC_COMBOATTACK:	// 多段攻撃
				div_flag = 1;
				break;
			case NPC_RANDOMATTACK:	// ランダムATK攻撃
				damage = damage*(50+rand()%150)/100;
				break;
			// 属性攻撃（適当）
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_UNDEADATTACK:
			case NPC_TELEKINESISATTACK:
//				div_= pd->skillduration; // [Valaris]
				break;
			case NPC_GUIDEDATTACK:
				hitrate = 1000000;
				break;
			case NPC_RANGEATTACK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case NPC_PIERCINGATT:
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				break;
			case NPC_CRITICALSLASH:
				ignore_def_flag = 1;
				break;
			case RG_BACKSTAP:	// バックスタブ
				damage = damage*(300+ 40*skill_lv)/100;
				hitrate = 1000000;
				break;
			case RG_RAID:	// サプライズアタック
				damage = damage*(100+ 40*skill_lv)/100;
				break;
			case RG_INTIMIDATE:	// インティミデイト
				damage = damage*(100+ 30*skill_lv)/100;
				break;
			case CR_SHIELDCHARGE:	// シールドチャージ
				damage = damage*(100+ 20*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				s_ele = 0;
				break;
			case CR_SHIELDBOOMERANG:	// シールドブーメラン
				damage = damage*(100+ 30*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				s_ele = 0;
				break;
			case CR_HOLYCROSS:	// ホーリークロス
				damage = damage*(100+ 35*skill_lv)/100;
				break;
			case CR_GRANDCROSS:
				hitrate= 1000000;
				break;
			case AM_DEMONSTRATION:	// デモンストレーション
				hitrate= 1000000;
				damage = damage*(100+ 20*skill_lv)/100;
				damage2 = damage2*(100+ 20*skill_lv)/100;
				break;
			case AM_ACIDTERROR:	// アシッドテラー
				hitrate = 1000000;
				ignore_def_flag = 1;
				damage = damage*(100+ 40*skill_lv)/100;
				damage2 = damage2*(100+ 40*skill_lv)/100;
				break;
			case MO_FINGEROFFENSIVE:	//指弾
				damage = damage * (100 + 50 * skill_lv) / 100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;   //orn
				break;
			case MO_INVESTIGATE:	// 発 勁
				if(def1 < 1000000)
					damage = damage*(100+ 75*skill_lv)/100 * (def1 + def2)/50;
				hitrate = 1000000;
				ignore_def_flag = 1;
				s_ele = 0;
				break;
			case MO_EXTREMITYFIST:	// 阿修羅覇鳳拳
				damage = damage * 8 + 250 + (skill_lv * 150);
				hitrate = 1000000;
				ignore_def_flag = 1;
				s_ele = 0;
				break;
			case MO_CHAINCOMBO:	// 連打掌
				damage = damage*(150+ 50*skill_lv)/100;
				break;
			case MO_COMBOFINISH:	// 猛龍拳
				damage = damage*(240+ 60*skill_lv)/100;
				break;
			case DC_THROWARROW:	// 矢撃ち
			case BA_MUSICALSTRIKE:
				damage = damage*(60+ 40 * skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case CH_TIGERFIST:	// 伏虎拳
				damage = damage*(40+ 100*skill_lv)/100;
				break;
			case CH_CHAINCRUSH:	// 連柱崩撃
				damage = damage*(400+ 100*skill_lv)/100;
				break;
			case CH_PALMSTRIKE:	// 猛虎硬派山
				damage = damage*(200+ 100*skill_lv)/100;
				break;
			case LK_SPIRALPIERCE:			/* スパイラルピアース */
				damage = damage*(100+ 50*skill_lv)/100; //増加量が分からないので適当に
				ignore_def_flag = 1;
//				if(map[target->m].flag.gvg) damage = damage * battle_config.gvg_long_damage_rate/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				if(target->type == BL_PC)
					((struct map_session_data *)target)->canmove_tick = gettick() + 1000;
				else if(target->type == BL_MOB)
					((struct mob_data *)target)->canmove_tick = gettick() + 1000;
				break;
			case LK_HEADCRUSH:				/* ヘッドクラッシュ */
				damage = damage*(100+ 40*skill_lv)/100;
				break;
			case LK_JOINTBEAT:				/* ジョイントビート */
				damage = damage*(50+ 10*skill_lv)/100;
				break;
			case ASC_METEORASSAULT:			/* メテオアサルト */
				damage = damage*(40+ 40*skill_lv)/100;
				break;
			case SN_SHARPSHOOTING:			/* シャープシューティング */
				damage += damage*(30*skill_lv)/100;
				break;
			case CG_ARROWVULCAN:			/* アローバルカン */
				damage = damage*(200+100*skill_lv)/100;
				break;
			case AS_SPLASHER:		/* ベナムスプラッシャー */
				damage = damage*(500+50*skill_lv)/100;
				hitrate = 1000000;
				break;
			case PA_SHIELDCHAIN:	// Shield Chain
				damage = damage*(30*skill_lv)/100;
				flag = (flag&~BF_RANGEMASK)|BF_LONG;
				div_flag = 1;
				s_ele = 0;				
				break;
			case WS_CARTTERMINATION:
				damage = damage * (80000 / (10 * (16 - skill_lv)) )/100;
				break;
			}
			if (div_flag && div_ > 1) {	// [Skotlex]
				damage *= div_;
				damage2 *= div_;
			}
		}

		// 対 象の防御力によるダメージの減少
		// ディバインプロテクション（ここでいいのかな？）
		if (!ignore_def_flag && def1 < 1000000) {	//DEF, VIT無視
			int t_def;
			target_count = 1 + battle_counttargeted(target,src,battle_config.vit_penalty_count_lv);
			if(battle_config.vit_penalty_type > 0) {
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						t_vit = (t_vit * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					}
					else if(battle_config.vit_penalty_type == 2) {
						def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						t_vit -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
					if(def1 < 0) def1 = 0;
						if(def2 < 1) def2 = 1;
					if(t_vit < 1) t_vit = 1;
				}
			}
			t_def = def2*8/10;
			vitbonusmax = (t_vit/20)*(t_vit/20)-1;
			if(battle_config.pet_defense_type) {
				damage = damage - (def1 * battle_config.pet_defense_type) - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
			}
			else{
				damage = damage * (100 - def1) /100 - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
			}
			
		}
	}

	// 0未満だった場合1に補正
	if(damage<1) damage=1;

	// 回避修正
	if(	hitrate < 1000000 && t_sc_data ) {			// 必中攻撃
		if(t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			hitrate -= 75;
		if (t_sc_data[SC_SLEEP].timer!=-1 ||	// 睡眠は必中
			t_sc_data[SC_STAN].timer!=-1 ||		// スタンは必中
			t_sc_data[SC_FREEZE].timer!=-1 ||
			(t_sc_data[SC_STONE].timer!=-1 && t_sc_data[SC_STONE].val2==0))	// 凍結は必中
			hitrate = 1000000;
	}
	if(hitrate < 1000000)
		hitrate = ( (hitrate>95)?95: ((hitrate<5)?5:hitrate) );
	if(type == 0 && rand()%100 >= hitrate) {
		damage = damage2 = 0;
		dmg_lv = ATK_FLEE;
	} else {
		dmg_lv = ATK_DEF;
	}


	if(t_sc_data) {
		int cardfix=100;
		if(t_sc_data[SC_DEFENDER].timer != -1 && flag&BF_LONG)
		{
			if(map[target->m].flag.gvg)
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2/2)/100;				
			else
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2)/100;
		}
		if(t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			cardfix=cardfix*50/100;
		if(cardfix != 100)
			damage=damage*cardfix/100;
	}
	if(damage < 0) damage = 0;

	// 属 性の適用
	if(skill_num != 0 || s_ele != 0 || !battle_config.pet_attack_attr_none)
	damage=battle_attr_fix(damage, s_ele, status_get_element(target) );

	if(skill_num==PA_PRESSURE) /* プレッシャー 必中? */
		damage = 500+300*skill_lv;

	// インベナム修正
	if(skill_num==TF_POISON){
		damage = battle_attr_fix(damage + 15*skill_lv, s_ele, status_get_element(target) );
	}
	if(skill_num==MC_CARTREVOLUTION){
		damage = battle_attr_fix(damage, 0, status_get_element(target) );
	}

	// 完全回避の判定
	if(battle_config.enemy_perfect_flee) {
		if(skill_num == 0 && tmd!=NULL && rand()%1000 < status_get_flee2(target) ){
			damage=0;
			type=0x0b;
			dmg_lv = ATK_LUCKY;
		}
	}

//	if(def1 >= 1000000 && damage > 0)
	if(t_mode&0x40 && damage > 0)
		damage = 1;

	if(is_boss(target))
		blewcount = 0;

	if(skill_num != CR_GRANDCROSS)
		damage=battle_calc_damage(src,target,damage,div_,skill_num,skill_lv,flag);

	wd.damage=damage;
	wd.damage2=0;
	wd.type=type;
	wd.div_=div_;
	wd.amotion=status_get_amotion(src);
	if(skill_num == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=status_get_dmotion(target);
	wd.blewcount=blewcount;
	wd.flag=flag;
	wd.dmg_lv=dmg_lv;

	return wd;
}

static struct Damage battle_calc_mob_weapon_attack(
	struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	struct map_session_data *tsd=NULL;
	struct mob_data* md=(struct mob_data *)src,*tmd=NULL;
	int hitrate,flee,cri = 0,atkmin,atkmax;
	int luk,target_count = 1;
	int def1 = status_get_def(target);
	int def2 = status_get_def2(target);
	int t_vit = status_get_vit(target);
	struct Damage wd;
	int damage,damage2=0,type,div_,blewcount=skill_get_blewcount(skill_num,skill_lv);
	int flag,skill,ac_flag = 0,dmg_lv = 0;
	int t_mode=0,t_race=0,t_size=1,s_race=0,s_ele=0,s_size=0,s_race2=0;
	struct status_change *sc_data,*t_sc_data;
	short *sc_count;
	short *option, *opt1, *opt2;
	int ignore_def_flag = 0;
	int div_flag=0;	// 0: total damage is to be divided by div_
					// 1: damage is distributed,and has to be multiplied by div_ [celest]

	//return前の処理があるので情報出力部のみ変更
	if( src == NULL || target == NULL || md == NULL ){
		nullpo_info(NLP_MARK);
		memset(&wd,0,sizeof(wd));
		return wd;
	}

	s_race = status_get_race(src);
	s_ele = status_get_attack_element(src);
	s_size = status_get_size(src);
	sc_data = status_get_sc_data(src);
	sc_count = status_get_sc_count(src);
	option = status_get_option(src);
	opt1 = status_get_opt1(src);
	opt2 = status_get_opt2(src);
	s_race2 = status_get_race2(src);

	// ターゲット
	if(target->type == BL_PC)
		tsd = (struct map_session_data *)target;
	else if(target->type == BL_MOB)
		tmd = (struct mob_data *)target;
	t_race = status_get_race( target );
	t_size = status_get_size( target );
	t_mode = status_get_mode( target );
	t_sc_data = status_get_sc_data( target );

	if(skill_num == 0 || (target->type == BL_PC && battle_config.pc_auto_counter_type&2) ||
		(target->type == BL_MOB && battle_config.monster_auto_counter_type&2)) {
		if(skill_num != CR_GRANDCROSS && t_sc_data && t_sc_data[SC_AUTOCOUNTER].timer != -1) {
			int dir = map_calc_dir(src,target->x,target->y),t_dir = status_get_dir(target);
			int dist = distance(src->x,src->y,target->x,target->y);
			if(dist <= 0 || map_check_dir(dir,t_dir) ) {
				memset(&wd,0,sizeof(wd));
				t_sc_data[SC_AUTOCOUNTER].val3 = 0;
				t_sc_data[SC_AUTOCOUNTER].val4 = 1;
				if(sc_data && sc_data[SC_AUTOCOUNTER].timer == -1) {
					int range = status_get_range(target);
					if((target->type == BL_PC && ((struct map_session_data *)target)->status.weapon != 11 && dist <= range+1) ||
						(target->type == BL_MOB && range <= 3 && dist <= range+1) )
						t_sc_data[SC_AUTOCOUNTER].val3 = src->id;
				}
				return wd;
			}
			else ac_flag = 1;
		} else if(skill_num != CR_GRANDCROSS && t_sc_data && t_sc_data[SC_POISONREACT].timer != -1) {   // poison react [Celest]
			t_sc_data[SC_POISONREACT].val3 = 0;
			t_sc_data[SC_POISONREACT].val4 = 1;
			t_sc_data[SC_POISONREACT].val3 = src->id;
		}
	}
	flag=BF_SHORT|BF_WEAPON|BF_NORMAL;	// 攻撃の種類の設定

	// 回避率計算、回避判定は後で
	flee = status_get_flee(target);
	if(battle_config.agi_penalty_type > 0 || battle_config.vit_penalty_type > 0)
		target_count += battle_counttargeted(target,src,battle_config.agi_penalty_count_lv);
	if(battle_config.agi_penalty_type > 0) {
		if(target_count >= battle_config.agi_penalty_count) {
			if(battle_config.agi_penalty_type == 1)
				flee = (flee * (100 - (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
			else if(battle_config.agi_penalty_type == 2)
				flee -= (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
			if(flee < 1) flee = 1;
		}
	}
	hitrate=(status_get_hit(src) + skill_gethitbonus(skill_num,skill_lv)) - flee + 80;

	type=0;	// normal
	if (skill_num > 0) {
		div_ = skill_get_num(skill_num,skill_lv);
		if (div_ < 1) div_ = 1;	//Avoid the rare case where the db says div_ is 0 and below
	} else div_ = 1; // single attack

	luk=status_get_luk(src);

	if(battle_config.enemy_str)
		damage = status_get_baseatk(src);
	else
		damage = 0;
	if(skill_num==HW_MAGICCRASHER){			/* マジッククラッシャーはMATKで殴る */
		atkmin = status_get_matk1(src);
		atkmax = status_get_matk2(src);
	}else{
	atkmin = status_get_atk(src);
	atkmax = status_get_atk2(src);
	}
	if(mob_db[md->class_].range>3 )
		flag=(flag&~BF_RANGEMASK)|BF_LONG;

	if(atkmin > atkmax) atkmin = atkmax;

	if(sc_data != NULL && sc_data[SC_MAXIMIZEPOWER].timer!=-1 ){	// マキシマイズパワー
		atkmin=atkmax;
	}

	cri = status_get_critical(src);
	cri -= status_get_luk(target) * 3;
	if(battle_config.enemy_critical_rate != 100) {
		cri = cri*battle_config.enemy_critical_rate/100;
		if(cri < 1)
			cri = 1;
	}
	if(t_sc_data) {
		if (t_sc_data[SC_SLEEP].timer!=-1 )	// 睡眠中はクリティカルが倍に
			cri <<=1;
		if(t_sc_data[SC_JOINTBEAT].timer != -1 &&
			t_sc_data[SC_JOINTBEAT].val2 == 5) // Always take crits with Neck broken by Joint Beat [DracoRPG]
			cri = 1000;
	}

	if(ac_flag) cri = 1000;

	if(skill_num == KN_AUTOCOUNTER) {
		if(!(battle_config.monster_auto_counter_type&1))
			cri = 1000;
		else
			cri <<= 1;
	}

	if(tsd && tsd->critical_def)
		cri = cri * (100 - tsd->critical_def) / 100;

	if((skill_num == 0 || skill_num == KN_AUTOCOUNTER) && skill_lv >= 0 && battle_config.enemy_critical && (rand() % 1000) < cri)	// 判定（スキルの場合は無視）
			// 敵の判定
	{
		damage += atkmax;
		type = 0x0a;
	}
	else {
		int vitbonusmax;

		if(atkmax > atkmin)
			damage += atkmin + rand() % (atkmax-atkmin + 1);
		else
			damage += atkmin ;
		// スキル修正１（攻撃力倍化系）
		// オーバートラスト(+5% ～ +25%),他攻撃系スキルの場合ここで補正
		// バッシュ,マグナムブレイク,
		// ボーリングバッシュ,スピアブーメラン,ブランディッシュスピア,スピアスタッブ,
		// メマーナイト,カートレボリューション
		// ダブルストレイフィング,アローシャワー,チャージアロー,
		// ソニックブロー
		if(sc_data){ //状態異常中のダメージ追加
			if(sc_data[SC_OVERTHRUST].timer!=-1)	// オーバートラスト
				damage += damage*(5*sc_data[SC_OVERTHRUST].val1)/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// トゥルーサイト
				damage += damage*(2*sc_data[SC_TRUESIGHT].val1)/100;
#if 0	// moved (aru)
			if(sc_data[SC_BERSERK].timer!=-1)	// バーサーク
				damage += damage;
#endif
			if(sc_data && sc_data[SC_AURABLADE].timer!=-1)	//[DracoRPG]
				damage += sc_data[SC_AURABLADE].val1 * 20;
			if(sc_data[SC_MAXOVERTHRUST].timer!=-1)
				damage += damage*(20*sc_data[SC_MAXOVERTHRUST].val1)/100;
		}

		if(skill_num>0){
			int i;
			if( (i=skill_get_pl(skill_num))>0 )
				s_ele=i;

			flag=(flag&~BF_SKILLMASK)|BF_SKILL;
			switch( skill_num ){
			case SM_BASH:		// バッシュ
				damage = damage*(100+ 30*skill_lv)/100;
				//hitrate = (hitrate*(100+5*skill_lv))/100;
				break;
			case SM_MAGNUM:		// マグナムブレイク
				damage = damage*(wflag > 1 ? 5*skill_lv+115 : 30*skill_lv+100)/100;
				//hitrate = (hitrate*(100+10*skill_lv))/100;
				break;
			case MC_MAMMONITE:	// メマーナイト
				damage = damage*(100+ 50*skill_lv)/100;
				break;
			case AC_DOUBLE:	// ダブルストレイフィング
				damage = damage*(180+ 20*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case AC_SHOWER:	// アローシャワー
				damage = damage*(75 + 5*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case AC_CHARGEARROW:	// チャージアロー
				damage = damage*150/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_PIERCE:	// ピアース
				damage = damage*(100+ 10*skill_lv)/100;
				//hitrate = hitrate*(100+5*skill_lv)/100;
				div_ = t_size+1;
				div_flag = 1;
				break;
			case KN_SPEARSTAB:	// スピアスタブ
				damage = damage*(100+ 15*skill_lv)/100;
				break;
			case KN_SPEARBOOMERANG:	// スピアブーメラン
				damage = damage*(100+ 50*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_BRANDISHSPEAR: // ブランディッシュスピア
				damage = damage*(100+ 20*skill_lv)/100;
				if(skill_lv>3 && wflag==1) damage2+=damage/2;
				if(skill_lv>6 && wflag==1) damage2+=damage/4;
				if(skill_lv>9 && wflag==1) damage2+=damage/8;
				if(skill_lv>6 && wflag==2) damage2+=damage/2;
				if(skill_lv>9 && wflag==2) damage2+=damage/4;
				if(skill_lv>9 && wflag==3) damage2+=damage/2;
				damage +=damage2;
				break;
			case KN_BOWLINGBASH:	// ボウリングバッシュ
				damage = damage*(100+ 50*skill_lv)/100;
				blewcount=0;
				break;
			case KN_AUTOCOUNTER:
				if(battle_config.monster_auto_counter_type&1)
					hitrate += 20;
				else
					hitrate = 1000000;
				ignore_def_flag = 1;
				flag=(flag&~BF_SKILLMASK)|BF_NORMAL;
				break;
			case AS_GRIMTOOTH:
				damage = damage*(100+ 20*skill_lv)/100;
				break;
			case AS_POISONREACT: // celest
				s_ele = 0;
				damage = damage*(100+ 30*skill_lv)/100;
				break;
			case AS_SONICBLOW:	// ソニックブロウ
				damage = damage*(300+ 50*skill_lv)/100;
				break;
			case TF_SPRINKLESAND:	// 砂まき
				damage = damage*125/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case ITM_TOMAHAWK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case MC_CARTREVOLUTION:	// カートレボリューション
				damage = (damage*150)/100;
				break;
			// 以下MOB
			case NPC_COMBOATTACK:	// 多段攻撃
				div_flag = 1;
				break;
			case NPC_RANDOMATTACK:	// ランダムATK攻撃
				damage = damage*(50+rand()%150)/100;
				break;
			// 属性攻撃（適当）
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_UNDEADATTACK:
			case NPC_TELEKINESISATTACK:
				damage = damage*(100+25*(skill_lv-1))/100;
				break;
			case NPC_GUIDEDATTACK:
				hitrate = 1000000;
				break;
			case NPC_RANGEATTACK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case NPC_PIERCINGATT:
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				break;
			case NPC_CRITICALSLASH:
				ignore_def_flag = 1;
				break;
			case RG_BACKSTAP:	// バックスタブ
				damage = damage*(300+ 40*skill_lv)/100;
				hitrate = 1000000;
				break;
			case RG_RAID:	// サプライズアタック
				damage = damage*(100+ 40*skill_lv)/100;
				break;
			case RG_INTIMIDATE:	// インティミデイト
				damage = damage*(100+ 30*skill_lv)/100;
				break;
			case CR_SHIELDCHARGE:	// シールドチャージ
				damage = damage*(100+ 20*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				s_ele = 0;
				break;
			case CR_SHIELDBOOMERANG:	// シールドブーメラン
				damage = damage*(100+ 30*skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				s_ele = 0;
				break;
			case CR_HOLYCROSS:	// ホーリークロス
				damage = damage*(100+ 35*skill_lv)/100;
				break;
			case CR_GRANDCROSS:
				hitrate= 1000000;
				break;
			case AM_DEMONSTRATION:	// デモンストレーション
				hitrate = 1000000;
				damage = damage*(100+ 20*skill_lv)/100;
				damage2 = damage2*(100+ 20*skill_lv)/100;
				break;
			case AM_ACIDTERROR:	// アシッドテラー
				hitrate = 1000000;
				ignore_def_flag = 1;
				damage = damage*(100+ 40*skill_lv)/100;
				damage2 = damage2*(100+ 40*skill_lv)/100;
				break;
			case MO_FINGEROFFENSIVE:	//指弾
				damage = damage * (100 + 50 * skill_lv) / 100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;   //orn
				break;
			case MO_INVESTIGATE:	// 発 勁
				if(def1 < 1000000)
					damage = damage*(100+ 75*skill_lv)/100 * (def1 + def2)/50;
				hitrate = 1000000;
				ignore_def_flag = 1;
				s_ele = 0;
				break;
			case MO_EXTREMITYFIST:	// 阿修羅覇鳳拳
				damage = damage * 8 + 250 + (skill_lv * 150);
				hitrate = 1000000;
				ignore_def_flag = 1;
				s_ele = 0;
				break;
			case MO_CHAINCOMBO:	// 連打掌
				damage = damage*(150+ 50*skill_lv)/100;
				break;
			case BA_MUSICALSTRIKE:	// ミュージカルストライク
			case DC_THROWARROW:	// 矢撃ち
				damage = damage*(60+ 40 * skill_lv)/100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case MO_COMBOFINISH:	// 猛龍拳
				damage = damage*(240+ 60*skill_lv)/100;
				break;
			case CH_TIGERFIST:	// 伏虎拳
				damage = damage*(40+ 100*skill_lv)/100;
				break;
			case CH_CHAINCRUSH:	// 連柱崩撃
				damage = damage*(400+ 100*skill_lv)/100;
				break;
			case CH_PALMSTRIKE:	// 猛虎硬派山
				damage = damage*(200+ 100*skill_lv)/100;
				break;
			case LK_SPIRALPIERCE:			/* スパイラルピアース */
				damage = damage*(100+ 50*skill_lv)/100; //増加量が分からないので適当に
				ignore_def_flag = 1;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
//				if(map[target->m].flag.gvg) damage = damage * battle_config.gvg_long_damage_rate/100;
				if(tsd)
					tsd->canmove_tick = gettick() + 1000;
				else if(tmd)
					tmd->canmove_tick = gettick() + 1000;
				break;
			case LK_HEADCRUSH:				/* ヘッドクラッシュ */
				damage = damage*(100+ 40*skill_lv)/100;
				break;
			case LK_JOINTBEAT:				/* ジョイントビート */
				damage = damage*(50+ 10*skill_lv)/100;
				break;
			case ASC_METEORASSAULT:			/* メテオアサルト */
				damage = damage*(40+ 40*skill_lv)/100;
				break;
			case SN_SHARPSHOOTING:			/* シャープシューティング */
				damage += damage*(30*skill_lv)/100;
				break;
			case CG_ARROWVULCAN:			/* アローバルカン */
				damage = damage*(200+100*skill_lv)/100;
				break;
			case AS_SPLASHER:		/* ベナムスプラッシャー */
				damage = damage*(500+50*skill_lv)/100;
				hitrate = 1000000;
				break;
			case PA_SHIELDCHAIN:	// Shield Chain
				damage = damage*(30*skill_lv)/100;
				flag = (flag&~BF_RANGEMASK)|BF_LONG;
				//hitrate += 20;
				div_flag = 1;
				s_ele = 0;				
				break;
			case WS_CARTTERMINATION:
				damage = damage * (80000 / (10 * (16 - skill_lv)) )/100;
				break;
			}
			if (div_flag && div_ > 1) {	// [Skotlex]
				damage *= div_;
				damage2 *= div_;
			}
		}

		// 対 象の防御力によるダメージの減少
		// ディバインプロテクション（ここでいいのかな？）
		if (!ignore_def_flag && def1 < 1000000) {	//DEF, VIT無視
			int t_def;
			target_count = 1 + battle_counttargeted(target,src,battle_config.vit_penalty_count_lv);
			if(battle_config.vit_penalty_type > 0) {
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						t_vit = (t_vit * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					}
					else if(battle_config.vit_penalty_type == 2) {
						def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						t_vit -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
					if(def1 < 0) def1 = 0;
					if(def2 < 1) def2 = 1;
					if(t_vit < 1) t_vit = 1;
				}
			}
			t_def = def2*8/10;
			if(battle_check_undead(s_race,status_get_elem_type(src)) || s_race==6)
				if(tsd && (skill=pc_checkskill(tsd,AL_DP)) > 0 )
					t_def += skill* (int) (3 + (tsd->status.base_level+1)*0.04);	// submitted by orn
					//t_def += skill*3;

			vitbonusmax = (t_vit/20)*(t_vit/20)-1;
			if(battle_config.monster_defense_type) {
				damage = damage - (def1 * battle_config.monster_defense_type) - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
			}
			else{
				damage = damage * (100 - def1) /100 - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
			}
		}
	}

	// 0未満だった場合1に補正
	if(damage<1) damage=1;

	// 回避修正
	if(	hitrate < 1000000 && t_sc_data ) {			// 必中攻撃
		if(t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			hitrate -= 75;
		if (t_sc_data[SC_SLEEP].timer!=-1 ||	// 睡眠は必中
			t_sc_data[SC_STAN].timer!=-1 ||		// スタンは必中
			t_sc_data[SC_FREEZE].timer!=-1 ||
			(t_sc_data[SC_STONE].timer!=-1 && t_sc_data[SC_STONE].val2==0))	// 凍結は必中
			hitrate = 1000000;
	}
	if(hitrate < 1000000)
		hitrate = ( (hitrate>95)?95: ((hitrate<5)?5:hitrate) );
	if(type == 0 && rand()%100 >= hitrate) {
		damage = damage2 = 0;
		dmg_lv = ATK_FLEE;
	} else {
		dmg_lv = ATK_DEF;
	}

	if(tsd){
		int cardfix=100,i;
		cardfix=cardfix*(100-tsd->subele[s_ele])/100;	// 属 性によるダメージ耐性
		cardfix=cardfix*(100-tsd->subrace[s_race])/100;	// 種族によるダメージ耐性
		cardfix=cardfix*(100-tsd->subsize[s_size])/100;
		cardfix=cardfix*(100-tsd->subrace2[s_race2])/100;	// 種族によるダメージ耐性
		if(is_boss(src))
			cardfix=cardfix*(100-tsd->subrace[10])/100;
		else
			cardfix=cardfix*(100-tsd->subrace[11])/100;
		for(i=0;i<tsd->add_def_class_count;i++) {
			if(tsd->add_def_classid[i] == md->class_) {
				cardfix=cardfix*(100-tsd->add_def_classrate[i])/100;
				break;
			}
		}
		for(i=0;i<tsd->add_damage_class_count2;i++) {
			if(tsd->add_damage_classid2[i] == md->class_) {
				cardfix=cardfix*(100+tsd->add_damage_classrate2[i])/100;
				break;
			}
		}
		if(flag&BF_LONG)
			cardfix=cardfix*(100-tsd->long_attack_def_rate)/100;
		if(flag&BF_SHORT)
			cardfix=cardfix*(100-tsd->near_attack_def_rate)/100;
		damage=damage*cardfix/100;
	}
	if(t_sc_data) {
		int cardfix=100;
		if(t_sc_data[SC_DEFENDER].timer != -1 && flag&BF_LONG)
		{
			if(map[target->m].flag.gvg)
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2/2)/100;				
			else
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2)/100;
		}
		if(t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			cardfix=cardfix*50/100;
		if(cardfix != 100)
			damage=damage*cardfix/100;
	}
	if(t_sc_data && t_sc_data[SC_ASSUMPTIO].timer != -1 &&
		!map[target->m].flag.gvg){ //アシャンプティオ
		if(!map[target->m].flag.pvp)
			damage=damage/3;
		else
			damage=damage/2;
	}

	if(damage < 0) damage = 0;

	// 属 性の適用
	if (!((battle_config.mob_ghostring_fix == 1) &&
		(status_get_elem_type(target) == 8) &&
		(target->type==BL_PC))) // [MouseJstr]
		if(skill_num != 0 || s_ele != 0 || !battle_config.mob_attack_attr_none)
			damage=battle_attr_fix(damage, s_ele, status_get_element(target) );

	//if(sc_data && sc_data[SC_AURABLADE].timer!=-1)	/* オーラブレード 必中 */
	//	damage += sc_data[SC_AURABLADE].val1 * 10;
	if(skill_num==PA_PRESSURE) /* プレッシャー 必中? */
		damage = 500+300*skill_lv;

	// インベナム修正
	if(skill_num==TF_POISON){
		damage = battle_attr_fix(damage + 15*skill_lv, s_ele, status_get_element(target) );
	}
	if(skill_num==MC_CARTREVOLUTION){
		damage = battle_attr_fix(damage, 0, status_get_element(target) );
	}

	// 完全回避の判定
	if(skill_num == 0 && tsd!=NULL && rand()%1000 < status_get_flee2(target) ){
		damage=0;
		type=0x0b;
		dmg_lv = ATK_LUCKY;
	}

	if(battle_config.enemy_perfect_flee) {
		if(skill_num == 0 && tmd!=NULL && rand()%1000 < status_get_flee2(target) ){
			damage=0;
			type=0x0b;
			dmg_lv = ATK_LUCKY;
		}
	}

//	if(def1 >= 1000000 && damage > 0)
	if(t_mode&0x40 && damage > 0)
		damage = 1;

	if(is_boss(target))
		blewcount = 0;

	if( tsd && tsd->special_state.no_weapon_damage)
		damage = 0;

	if(skill_num != CR_GRANDCROSS)
		damage=battle_calc_damage(src,target,damage,div_,skill_num,skill_lv,flag);

	wd.damage=damage;
	wd.damage2=0;
	wd.type=type;
	wd.div_=div_;
	wd.amotion=status_get_amotion(src);
	if(skill_num == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=status_get_dmotion(target);
	wd.blewcount=blewcount;
	wd.flag=flag;
	wd.dmg_lv=dmg_lv;
	return wd;
}
/*
 * =========================================================================
 * PCの武器による攻撃
 *-------------------------------------------------------------------------
 */
static struct Damage battle_calc_pc_weapon_attack(
	struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	struct map_session_data *sd=(struct map_session_data *)src,*tsd=NULL;
	struct mob_data *tmd=NULL;
	int hitrate,flee,cri = 0,atkmin,atkmax;
	int dex,luk,target_count = 1;
	int no_cardfix=0;
	int def1 = status_get_def(target);
	int def2 = status_get_def2(target);
	int t_vit = status_get_vit(target);
	struct Damage wd;
	int damage,damage2,damage_rate=100,type,div_,blewcount=skill_get_blewcount(skill_num,skill_lv);
	int flag,skill,dmg_lv = 0;
	int t_mode=0,t_race=0,t_size=1,s_race=7,s_ele=0,s_size=1;
	int t_race2=0;
	struct status_change *sc_data,*t_sc_data;
	short *sc_count;
	short *option, *opt1, *opt2;
	int atkmax_=0, atkmin_=0, s_ele_;	//二刀流用
	int watk,watk_,cardfix,t_ele;
	int da=0,i,t_class,ac_flag = 0;
	int ignore_def_flag = 0;
	int idef_flag=0,idef_flag_=0;
	int div_flag=0;	// 0: total damage is to be divided by div_
	int is_emperium=0;
	int mastery_flag=1;
					// 1: damage is distributed,and has to be multiplied by div_ [celest]

	//return前の処理があるので情報出力部のみ変更
	if( src == NULL || target == NULL || sd == NULL ){
		nullpo_info(NLP_MARK);
		memset(&wd,0,sizeof(wd));
		return wd;
	}


	// アタッカー
	s_race=status_get_race(src); //種族
	s_ele=status_get_attack_element(src); //属性
	s_ele_=status_get_attack_element2(src); //左手属性
	s_size=status_get_size(src);
	sc_data=status_get_sc_data(src); //ステータス異常
	sc_count=status_get_sc_count(src); //ステータス異常の数
	option=status_get_option(src); //鷹とかペコとかカートとか
	opt1=status_get_opt1(src); //石化、凍結、スタン、睡眠、暗闇
	opt2=status_get_opt2(src); //毒、呪い、沈黙、暗闇？
	t_race2=status_get_race2(target);

	if(skill_num != CR_GRANDCROSS) //グランドクロスでないなら
		sd->state.attack_type = BF_WEAPON; //攻撃タイプは武器攻撃

	// ターゲット
	if(target->type==BL_PC) //対象がPCなら
		tsd=(struct map_session_data *)target; //tsdに代入(tmdはNULL)
	else if(target->type==BL_MOB) //対象がMobなら
		tmd=(struct mob_data *)target; //tmdに代入(tsdはNULL)
	t_race=status_get_race( target ); //対象の種族
	t_ele=status_get_elem_type(target); //対象の属性
	t_size=status_get_size( target ); //対象のサイズ
	t_mode=status_get_mode( target ); //対象のMode
	t_sc_data=status_get_sc_data( target ); //対象のステータス異常

	if(status_get_class(target)==1288) // Emperium
		is_emperium = 1;

//オートカウンター処理ここから
	if(skill_num == 0 || (target->type == BL_PC && battle_config.pc_auto_counter_type&2) ||
		(target->type == BL_MOB && battle_config.monster_auto_counter_type&2)) {
		if(skill_num != CR_GRANDCROSS && t_sc_data && t_sc_data[SC_AUTOCOUNTER].timer != -1) { //グランドクロスでなく、対象がオートカウンター状態の場合
			int dir = map_calc_dir(src,target->x,target->y),t_dir = status_get_dir(target);
			int dist = distance(src->x,src->y,target->x,target->y);
			if(dist <= 0 || map_check_dir(dir,t_dir) ) { //対象との距離が0以下、または対象の正面？
				memset(&wd,0,sizeof(wd));
				t_sc_data[SC_AUTOCOUNTER].val3 = 0;
				t_sc_data[SC_AUTOCOUNTER].val4 = 1;
				if(sc_data && sc_data[SC_AUTOCOUNTER].timer == -1) { //自分がオートカウンター状態
					int range = status_get_range(target);
					if((target->type == BL_PC && ((struct map_session_data *)target)->status.weapon != 11 && dist <= range+1) || //対象がPCで武器が弓矢でなく射程内
						(target->type == BL_MOB && range <= 3 && dist <= range+1) ) //または対象がMobで射程が3以下で射程内
						t_sc_data[SC_AUTOCOUNTER].val3 = src->id;
				}
				return wd; //ダメージ構造体を返して終了
			}
			else ac_flag = 1;
		} else if(skill_num != CR_GRANDCROSS && t_sc_data && t_sc_data[SC_POISONREACT].timer != -1) {   // poison react [Celest]
			t_sc_data[SC_POISONREACT].val3 = 0;
			t_sc_data[SC_POISONREACT].val4 = 1;
			t_sc_data[SC_POISONREACT].val3 = src->id;
		}
	}
//オートカウンター処理ここまで

	flag = BF_SHORT|BF_WEAPON|BF_NORMAL;	// 攻撃の種類の設定

	// 回避率計算、回避判定は後で
	flee = status_get_flee(target);
	if(battle_config.agi_penalty_type > 0 || battle_config.vit_penalty_type > 0) //AGI、VITペナルティ設定が有効
		target_count += battle_counttargeted(target,src,battle_config.agi_penalty_count_lv);	//対象の数を算出
	if(battle_config.agi_penalty_type > 0) {
		if(target_count >= battle_config.agi_penalty_count) { //ペナルティ設定より対象が多い
			if(battle_config.agi_penalty_type == 1) //回避率がagi_penalty_num%ずつ減少
				flee = (flee * (100 - (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
			else if(battle_config.agi_penalty_type == 2) //回避率がagi_penalty_num分減少
				flee -= (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
			if(flee < 1) flee = 1; //回避率は最低でも1
		}
	}
	hitrate = (status_get_hit(src) + skill_gethitbonus(skill_num,skill_lv))- flee + 80; //命中率計算

	type = 0;	// normal
	if (skill_num > 0) {
		div_=skill_get_num(skill_num,skill_lv);
		if (div_ < 1) div_ = 1;	//Avoid the rare case where the db says div_ is 0 and below
	} else div_ = 1; // single attack

	dex = status_get_dex(src); //DEX
	luk = status_get_luk(src); //LUK
	watk = status_get_atk(src); //ATK
	watk_ = status_get_atk_(src); //ATK左手

	if(skill_num == HW_MAGICCRASHER)	/* マジッククラッシャーはMATKで殴る */
		damage = damage2 = status_get_matk1(src); //damega,damega2初登場、base_atkの取得
	else
		damage = damage2 = status_get_baseatk(&sd->bl); //damega,damega2初登場、base_atkの取得

	atkmin = atkmin_ = dex; //最低ATKはDEXで初期化？
	sd->state.arrow_atk = 0; //arrow_atk初期化
	if(sd->equip_index[9] >= 0 && sd->inventory_data[sd->equip_index[9]])
		atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[9]]->wlv*20)/100;
	if(sd->equip_index[8] >= 0 && sd->inventory_data[sd->equip_index[8]])
		atkmin_ = atkmin_*(80 + sd->inventory_data[sd->equip_index[8]]->wlv*20)/100;
	if(sd->status.weapon == 11) { //武器が弓矢の場合
		atkmin = watk * ((atkmin<watk)? atkmin:watk)/100; //弓用最低ATK計算
		flag=(flag&~BF_RANGEMASK)|BF_LONG; //遠距離攻撃フラグを有効
		if(sd->arrow_ele > 0) //属性矢なら属性を矢の属性に変更
			s_ele = sd->arrow_ele;
		sd->state.arrow_atk = 1; //arrow_atk有効化
	}

	// サイズ修正
	// ペコ騎乗していて、槍で攻撃した場合は中型のサイズ修正を100にする
	// ウェポンパーフェクション,ドレイクC
	if (sd->special_state.no_sizefix ||
		skill_num == MO_EXTREMITYFIST ||
		(sc_data && sc_data[SC_WEAPONPERFECTION].timer!=-1) ||
		(pc_isriding(sd) && (sd->status.weapon == 4 || sd->status.weapon == 5) && t_size == 1))
	{
		atkmax = watk;
		atkmax_ = watk_;
	} else {
		atkmax = (watk * sd->right_weapon.atkmods[ t_size ]) / 100;
		atkmin = (atkmin * sd->right_weapon.atkmods[ t_size ]) / 100;
		atkmax_ = (watk_ * sd->left_weapon.atkmods[ t_size ]) / 100;
		atkmin_ = (atkmin_ * sd->left_weapon.atkmods[ t_size ]) / 100;
	}

	if (sc_data && sc_data[SC_MAXIMIZEPOWER].timer!=-1) {	// マキシマイズパワー
		atkmin = atkmax;
		atkmin_ = atkmax_;
	}
	
	if (atkmin > atkmax && !(sd->state.arrow_atk)) atkmin = atkmax;	//弓は最低が上回る場合あり
	if (atkmin_ > atkmax_) atkmin_ = atkmax_;	

	if (skill_num == 0) {
		//ダブルアタック判定
		int da_rate = sd->double_rate; 
		int da_rate2 = pc_checkskill(sd,TF_DOUBLE) * 5;
		if(sd->weapontype1 == 0x01 || da_rate>0) 
		{
			if(da_rate < da_rate2)
				da_rate = da_rate2;
		}
			
		//三段掌	 // triple blow works with bows ^^ [celest]
		if (sd->status.weapon <= 16 && (skill = pc_checkskill(sd,MO_TRIPLEATTACK)) > 0)
			da = (rand()%100 < (30 - skill)) ? 2 : 0;
		if (da == 0 && da_rate > 0)
			// success rate from Double Attack is counted in
			da = (rand()%100 < da_rate) ? 1 : 0;
	}

	// 過剰精錬ボーナス
	if (sd->right_weapon.overrefine > 0)
		damage += (rand() % sd->right_weapon.overrefine) + 1;
	if (sd->left_weapon.overrefine > 0)
		damage2 += (rand() % sd->left_weapon.overrefine) + 1;

	if (da == 0) { //ダブルアタックが発動していない
		// クリティカル計算
		cri = status_get_critical(src) + sd->critaddrace[t_race];

		if (sd->state.arrow_atk)
			cri += sd->arrow_cri;
		if(sd->status.weapon == 16)	// カタールの場合、クリティカルを倍に
			cri <<= 1;
		cri -= status_get_luk(target) * 3;

		if (t_sc_data) {
			if (t_sc_data[SC_SLEEP].timer != -1)	// 睡眠中はクリティカルが倍に
				cri <<=1;
			if (t_sc_data[SC_JOINTBEAT].timer != -1 &&
				t_sc_data[SC_JOINTBEAT].val2 == 5) // Always take crits with Neck broken by Joint Beat [DracoRPG]
				cri = 1000;
		}
		if (ac_flag) cri = 1000;

		if (skill_num == KN_AUTOCOUNTER) {
			if (!(battle_config.pc_auto_counter_type&1))
				cri = 1000;
			else
				cri <<= 1;
		}
		else if (skill_num == SN_SHARPSHOOTING)
			cri += 200;
	}

	if(tsd && tsd->critical_def)
		cri = cri * (100-tsd->critical_def) / 100;

	if (da == 0 && (skill_num == 0 || skill_num == KN_AUTOCOUNTER || skill_num == SN_SHARPSHOOTING) && //ダブルアタックが発動していない
		(rand() % 1000) < cri)	// 判定（スキルの場合は無視）
	{
		damage += atkmax;
		damage2 += atkmax_;
		if (sd->atk_rate != 100 || sd->weapon_atk_rate != 0) {
			if (sd->status.weapon < 16) {
				damage = (damage * (sd->atk_rate + sd->weapon_atk_rate[sd->status.weapon]))/100;
				damage2 = (damage2 * (sd->atk_rate + sd->weapon_atk_rate[sd->status.weapon]))/100;
			}
		}
		if (sd->state.arrow_atk)
			damage += sd->arrow_atk;
		damage += damage * sd->crit_atk_rate / 100;
		type = 0x0a;
	} else {
		int vitbonusmax;

		if(atkmax > atkmin)
			damage += atkmin + rand() % (atkmax-atkmin + 1);
		else
			damage += atkmin ;
		if(atkmax_ > atkmin_)
			damage2 += atkmin_ + rand() % (atkmax_-atkmin_ + 1);
		else
			damage2 += atkmin_ ;
		if(sd->atk_rate != 100 || sd->weapon_atk_rate != 0) {
			if (sd->status.weapon < 16) {
				damage = (damage * (sd->atk_rate + sd->weapon_atk_rate[sd->status.weapon]))/100;
				damage2 = (damage2 * (sd->atk_rate + sd->weapon_atk_rate[sd->status.weapon]))/100;
			}
		}

		if(sd->state.arrow_atk) {
			if(sd->arrow_atk > 0)
				damage += rand()%(sd->arrow_atk+1);
			hitrate += sd->arrow_hit;
		}

		// スキル修正１（攻撃力倍化系）
		// オーバートラスト(+5% ～ +25%),他攻撃系スキルの場合ここで補正
		// バッシュ,マグナムブレイク,
		// ボーリングバッシュ,スピアブーメラン,ブランディッシュスピア,スピアスタッブ,
		// メマーナイト,カートレボリューション
		// ダブルストレイフィング,アローシャワー,チャージアロー,
		// ソニックブロー
		if(sc_data){ //状態異常中のダメージ追加
			if(sc_data[SC_OVERTHRUST].timer!=-1)	// オーバートラスト
				damage_rate += 5*sc_data[SC_OVERTHRUST].val1;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// トゥルーサイト
				damage_rate += 2*sc_data[SC_TRUESIGHT].val1;
#if 0	// moved (aru
			if(sc_data[SC_BERSERK].timer!=-1)	// バーサーク
				damage_rate += 200;
#endif
			if(sc_data[SC_MAXOVERTHRUST].timer!=-1)
				damage_rate += 20*sc_data[SC_MAXOVERTHRUST].val1;
		}

		if(skill_num>0){
			int i;
			if( (i=skill_get_pl(skill_num))>0 )
				s_ele=s_ele_=i;

			flag=(flag&~BF_SKILLMASK)|BF_SKILL;
			switch( skill_num ){
			case SM_BASH:		// バッシュ
				damage_rate += 30*skill_lv;
				//hitrate = (hitrate*(100+5*skill_lv))/100;
				break;
			case SM_MAGNUM:		// マグナムブレイク
				// 20*skill level+100? i think this will do for now [based on jRO info]
				damage_rate += (wflag > 1 ? 5*skill_lv+15 : 30*skill_lv);
				//hitrate = (hitrate*(100+10*skill_lv))/100;
				break;
			case MC_MAMMONITE:	// メマーナイト
				damage_rate += 50*skill_lv;
				break;
			case AC_DOUBLE:	// ダブルストレイフィング
				if(!sd->state.arrow_atk && sd->arrow_atk > 0) {
					int arr = rand()%(sd->arrow_atk+1);
					damage += arr;
					damage2 += arr;
				}
				damage_rate += 80+ 20*skill_lv;
				if(sd->arrow_ele > 0) {
					s_ele = sd->arrow_ele;
					s_ele_ = sd->arrow_ele;
				}
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				sd->state.arrow_atk = 1;
				break;
			case AC_SHOWER:	// アローシャワー
				if(!sd->state.arrow_atk && sd->arrow_atk > 0) {
					int arr = rand()%(sd->arrow_atk+1);
					damage += arr;
					damage2 += arr;
				}
				damage_rate += 5*skill_lv-25;
				if(sd->arrow_ele > 0) {
					s_ele = sd->arrow_ele;
					s_ele_ = sd->arrow_ele;
				}
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				sd->state.arrow_atk = 1;
				break;
			case AC_CHARGEARROW:	// チャージアロー
				if(!sd->state.arrow_atk && sd->arrow_atk > 0) {
					int arr = rand()%(sd->arrow_atk+1);
					damage += arr;
					damage2 += arr;
				}
				damage_rate += 50;
				if(sd->arrow_ele > 0) {
					s_ele = sd->arrow_ele;
					s_ele_ = sd->arrow_ele;
				}
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				sd->state.arrow_atk = 1;
				break;
			case KN_PIERCE:	// ピアース
				damage_rate += 10*skill_lv;
				//hitrate=hitrate*(100+5*skill_lv)/100;
				div_=t_size+1;
				div_flag=1;
				break;
			case KN_SPEARSTAB:	// スピアスタブ
				damage_rate += 15*skill_lv;
				blewcount=0;
				break;
			case KN_SPEARBOOMERANG:	// スピアブーメラン
				damage_rate += 50*skill_lv;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_BRANDISHSPEAR: // ブランディッシュスピア
				damage_rate += 20*skill_lv;
				if(skill_lv>3 && wflag==1) damage_rate+=50;
				if(skill_lv>6 && wflag==1) damage_rate+=25;
				if(skill_lv>9 && wflag==1) damage_rate+=12.5;
				if(skill_lv>6 && wflag==2) damage_rate+=50;
				if(skill_lv>9 && wflag==2) damage_rate+=25;
				if(skill_lv>9 && wflag==3) damage_rate+=50;
				break;
			case KN_BOWLINGBASH:	// ボウリングバッシュ
				damage_rate = 50*skill_lv;
				blewcount=0;
				break;
			case KN_AUTOCOUNTER:
				if(battle_config.pc_auto_counter_type&1)
					hitrate += 20;
				else
					hitrate = 1000000;
				ignore_def_flag = 1;
				flag=(flag&~BF_SKILLMASK)|BF_NORMAL;
				break;
			case AS_GRIMTOOTH:
				damage_rate += 20*skill_lv;
				break;
			case AS_POISONREACT: // celest
				s_ele = 0;
				damage_rate += 30*skill_lv;
				break;
			case AS_SONICBLOW:	// ソニックブロウ
				//hitrate+=30; // hitrate +30, thanks to midas
				damage_rate += 200+ 50*skill_lv;
				break;
			case TF_SPRINKLESAND:	// 砂まき
				damage_rate += 25;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case ITM_TOMAHAWK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case MC_CARTREVOLUTION:	// カートレボリューション
				no_cardfix = 1;
				if(sd->cart_max_weight > 0 && sd->cart_weight > 0) {
					damage_rate += 50 + sd->cart_weight/800;	//fixed CARTREV damage [Lupus] // should be 800, not 80... weight is *10 ^_- [celest]
				}
				else {
					damage_rate += 50;
				}
				break;
			// 以下MOB
			case NPC_COMBOATTACK:	// 多段攻撃
				div_flag=1;
				break;
			case NPC_RANDOMATTACK:	// ランダムATK攻撃
				damage_rate += rand()%150-50;
				break;
			// 属性攻撃（適当）
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_UNDEADATTACK:
			case NPC_TELEKINESISATTACK:
				damage_rate += 25*skill_lv;
				break;
			case NPC_GUIDEDATTACK:
				hitrate = 1000000;
				break;
			case NPC_RANGEATTACK:
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case NPC_PIERCINGATT:
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				break;
			case NPC_CRITICALSLASH:
				ignore_def_flag = 1;
				break;
			case RG_BACKSTAP:	// バックスタブ
				if(battle_config.backstab_bow_penalty == 1 && sd->status.weapon == 11){
					damage_rate += 50+ 20*skill_lv; // Back Stab with a bow does twice less damage
				}else{
					damage_rate += 200+ 40*skill_lv;
				}
				hitrate = 1000000;
				break;
			case RG_RAID:	// サプライズアタック
				damage_rate += 40*skill_lv;
				break;
			case RG_INTIMIDATE:	// インティミデイト
				damage_rate += 30*skill_lv;
				break;
			case CR_SHIELDCHARGE:	// シールドチャージ
				damage_rate += 20*skill_lv;
				flag=(flag&~BF_RANGEMASK)|BF_SHORT;
				s_ele = 0;
				break;
			case CR_SHIELDBOOMERANG:	// シールドブーメラン
				damage_rate += 30*skill_lv;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				s_ele = 0;
				break;
			case CR_HOLYCROSS:	// ホーリークロス
				damage_rate += 35*skill_lv;
				break;
			case CR_GRANDCROSS:
				hitrate= 1000000;
				if(!battle_config.gx_cardfix)
					no_cardfix = 1;
				break;
			case AM_DEMONSTRATION:	// デモンストレーション
				damage_rate += 20*skill_lv;
				no_cardfix = 1;
				break;
			case AM_ACIDTERROR:	// アシッドテラー
				hitrate = 1000000;
				damage_rate += 40*skill_lv;
				s_ele = 0;
				s_ele_ = 0;
				ignore_def_flag = 1;
				no_cardfix = 1;
				break;
			case MO_FINGEROFFENSIVE:	//指弾
				damage_rate += 50 * skill_lv;
				if(battle_config.finger_offensive_type == 0) {
					div_ = sd->spiritball_old;
					div_flag = 1;
				}
				else div_ = 1;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;   //orn
				break;
			case MO_INVESTIGATE:	// 発 勁
				if(def1 < 1000000) {
					damage = damage*(def1 + def2)/50;
					damage2 = damage2*(def1 + def2)/50;
					damage_rate += 75*skill_lv;
				}
				hitrate = 1000000;
				ignore_def_flag = 1;
				mastery_flag = 0;
				s_ele = 0;
				s_ele_ = 0;
				break;
			case MO_EXTREMITYFIST:	// 阿修羅覇鳳拳
				damage = damage * (8 + (sd->status.sp/10)) + 250 + (skill_lv * 150);
				sd->status.sp = 0;
				clif_updatestatus(sd,SP_SP);
				hitrate = 1000000;
				ignore_def_flag = 1;
				mastery_flag = 0;
				s_ele = 0;
				s_ele_ = 0;
				break;
			case MO_CHAINCOMBO:	// 連打掌
				damage_rate += 50+ 50*skill_lv;
				break;
			case MO_COMBOFINISH:	// 猛龍拳
				damage_rate += 140+ 60*skill_lv;;
				break;
			case BA_MUSICALSTRIKE:	// ミュージカルストライク
			case DC_THROWARROW:	// 矢撃ち
				if(!sd->state.arrow_atk && sd->arrow_atk > 0) {
					int arr = rand()%(sd->arrow_atk+1);
					damage += arr;
					damage2 += arr;
				}
				damage_rate += 50 * skill_lv;
				if(sd->arrow_ele > 0) {
					s_ele = sd->arrow_ele;
					s_ele_ = sd->arrow_ele;
				}
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				sd->state.arrow_atk = 1;
				break;
			case CH_TIGERFIST:	// 伏虎拳
				damage_rate +=  100*skill_lv-60;
				break;
			case CH_CHAINCRUSH:	// 連柱崩撃
				damage_rate += 300+ 100*skill_lv;
				break;
			case CH_PALMSTRIKE:	// 猛虎硬派山
				damage_rate += 100+ 100*skill_lv;
				break;
			case LK_SPIRALPIERCE:			/* スパイラルピアース */
				damage_rate += 50*skill_lv; //増加量が分からないので適当に
				ignore_def_flag = 1;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
//				if(map[target->m].flag.gvg) damage = damage * battle_config.gvg_long_damage_rate/100;
				if(tsd)
					tsd->canmove_tick = gettick() + 1000;
				else if(tmd)
					tmd->canmove_tick = gettick() + 1000;
				break;
			case LK_HEADCRUSH:				/* ヘッドクラッシュ */
				damage_rate += 40*skill_lv;
				break;
			case LK_JOINTBEAT:				/* ジョイントビート */
				damage_rate += 10*skill_lv-50;
				break;
			case ASC_METEORASSAULT:			/* メテオアサルト */
				damage_rate += 40*skill_lv-60;
				no_cardfix = 1;
				break;
			case SN_SHARPSHOOTING:			/* シャープシューティング */
				damage_rate += 30*skill_lv;
				break;
			case CG_ARROWVULCAN:			/* アローバルカン */
				damage_rate += 100+100*skill_lv;
				if(sd->arrow_ele > 0) {
					s_ele = sd->arrow_ele;
					s_ele_ = sd->arrow_ele;
				}
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case AS_SPLASHER:		/* ベナムスプラッシャー */
				damage_rate += 400+50*skill_lv;
				no_cardfix = 1;
				hitrate = 1000000;
				break;
			case ASC_BREAKER:
				// calculate physical part of damage
				damage_rate += 100*skill_lv-100;
				flag=(flag&~BF_RANGEMASK)|BF_LONG;
				no_cardfix = 1;
				break;
			case PA_SHIELDCHAIN:
				damage_rate += 30*skill_lv-100;
				flag = (flag&~BF_RANGEMASK)|BF_LONG;
				//hitrate += 20;
				div_flag = 1;
				s_ele = 0;
				break;
			case WS_CARTTERMINATION:
				no_cardfix = 1;
				if(sd->cart_max_weight > 0 && sd->cart_weight > 0) {
					damage_rate += sd->cart_weight/(10*(16-skill_lv))-100;
				}
				break;
			case PA_SACRIFICE:
				damage = (status_get_max_hp(src)*9) / 100;
				damage += (damage/10)*(skill_lv-1);
				hitrate = 1000000;
				ignore_def_flag = 1;
				mastery_flag = 0;
				break;
			}

			//what about normal attacks? [celest]
			//damage *= damage_rate/100;
			//damage2 *= damage_rate/100;

			if (div_flag && div_ > 1) {	// [Skotlex]
				damage *= div_;
				damage2 *= div_;
			}
			if (sd)
				damage += damage*sd->skill_damage_bonus[skill_num]/100;
		}

		damage = damage * damage_rate / 100;
		damage2 = damage2 * damage_rate / 100;

		if(da == 2) { //三段掌が発動しているか
			type = 0x08;
			div_ = 255;	//三段掌用に…
			damage = damage * (100 + 20 * pc_checkskill(sd, MO_TRIPLEATTACK)) / 100;
		}

		if(skill_num != MO_INVESTIGATE && def1 < 1000000) {
			if(sd->right_weapon.def_ratio_atk_ele & (1<<t_ele) || sd->right_weapon.def_ratio_atk_race & (1<<t_race)) {
				damage = (damage * (def1 + def2))/100;
				idef_flag = 1;
			}
			if(sd->left_weapon.def_ratio_atk_ele & (1<<t_ele) || sd->left_weapon.def_ratio_atk_race & (1<<t_race)) {
				damage2 = (damage2 * (def1 + def2))/100;
				idef_flag_ = 1;
			}
			if(is_boss(target)) {
				if(!idef_flag && sd->right_weapon.def_ratio_atk_race & (1<<10)) {
					damage = (damage * (def1 + def2))/100;
					idef_flag = 1;
				}
				if(!idef_flag_ && sd->left_weapon.def_ratio_atk_race & (1<<10)) {
					damage2 = (damage2 * (def1 + def2))/100;
					idef_flag_ = 1;
				}
			}
			else {
				if(!idef_flag && sd->right_weapon.def_ratio_atk_race & (1<<11)) {
					damage = (damage * (def1 + def2))/100;
					idef_flag = 1;
				}
				if(!idef_flag_ && sd->left_weapon.def_ratio_atk_race & (1<<11)) {
					damage2 = (damage2 * (def1 + def2))/100;
					idef_flag_ = 1;
				}
			}
		}


		// 対 象の防御力によるダメージの減少
		// ディバインプロテクション（ここでいいのかな？）
		if (!ignore_def_flag && def1 < 1000000) {	//DEF, VIT無視
			int t_def;
			target_count = 1 + battle_counttargeted(target,src,battle_config.vit_penalty_count_lv);
			if(battle_config.vit_penalty_type > 0) {
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						t_vit = (t_vit * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					}
					else if(battle_config.vit_penalty_type == 2) {
						def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						t_vit -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
					if(def1 < 0) def1 = 0;
					if(def2 < 1) def2 = 1;
					if(t_vit < 1) t_vit = 1;
				}
			}
			t_def = def2*8/10;
			vitbonusmax = (t_vit/20)*(t_vit/20)-1;
			if (tmd) {
				if(is_boss(target)) {
					if(sd->right_weapon.ignore_def_mob & 2)
						idef_flag = 1;
					if(sd->left_weapon.ignore_def_mob & 2)
						idef_flag_ = 1;
				} else {
					if(sd->right_weapon.ignore_def_mob & 1)
						idef_flag = 1;
					if(sd->left_weapon.ignore_def_mob & 1)
						idef_flag_ = 1;
				}
			}
			if(sd->right_weapon.ignore_def_ele & (1<<t_ele) || sd->right_weapon.ignore_def_race & (1<<t_race))
				idef_flag = 1;
			if(sd->left_weapon.ignore_def_ele & (1<<t_ele) || sd->left_weapon.ignore_def_race & (1<<t_race))
				idef_flag_ = 1;
			if(is_boss(target)) {
				if(sd->right_weapon.ignore_def_race & (1<<10))
					idef_flag = 1;
				if(sd->left_weapon.ignore_def_race & (1<<10))
					idef_flag_ = 1;
			}
			else {
				if(sd->right_weapon.ignore_def_race & (1<<11))
					idef_flag = 1;
				if(sd->left_weapon.ignore_def_race & (1<<11))
					idef_flag_ = 1;
			}

			if(!idef_flag){
				if(battle_config.player_defense_type) {
					damage = damage - (def1 * battle_config.player_defense_type) - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
				}
				else{
					damage = damage * (100 - def1) /100 - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
				}
			}
			if(!idef_flag_){
				if(battle_config.player_defense_type) {
					damage2 = damage2 - (def1 * battle_config.player_defense_type) - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
				}
				else{
					damage2 = damage2 * (100 - def1) /100 - t_def - ((vitbonusmax < 1)?0: rand()%(vitbonusmax+1) );
				}
			}
		}

	}

	// 状態異常中のダメージ追加でクリティカルにも有効なスキル
	if (sc_data) {
		// エンチャントデッドリーポイズン
		if(!no_cardfix && sc_data[SC_EDP].timer != -1 &&
			skill_num != ASC_BREAKER && skill_num != ASC_METEORASSAULT)
		{
			if(is_emperium)
				damage += damage * (150 + sc_data[SC_EDP].val1 * 50) / 200;
			else
				damage += damage * (150 + sc_data[SC_EDP].val1 * 50) / 100;
			no_cardfix = 1;
		}
	}

	// 精錬ダメージの追加
	if(mastery_flag) {			//DEF, VIT無視
		damage += status_get_atk2(src);
		damage2 += status_get_atk_2(src);
	}
	if(skill_num == CR_SHIELDBOOMERANG || skill_num == PA_SHIELDCHAIN) {
		if(sd->equip_index[8] >= 0) {
			int index = sd->equip_index[8];
			if(sd->inventory_data[index] && sd->inventory_data[index]->type == 5) {
				damage += sd->inventory_data[index]->weight/10;
				damage += sd->status.inventory[index].refine * status_getrefinebonus(0,1);
			}
		}
	}
	else if(skill_num == LK_SPIRALPIERCE) {			/* スパイラルピアース */
		if(sd->equip_index[9] >= 0) {	//重量で追加ダメージらしいのでシールドブーメランを参考に追加
			int index = sd->equip_index[9];
			if(sd->inventory_data[index] && sd->inventory_data[index]->type == 4) {
				damage += (int)(double)(sd->inventory_data[index]->weight*(0.8*skill_lv*4/10));
				damage += sd->status.inventory[index].refine * status_getrefinebonus(0,1);
			}
		}
	}

	// 0未満だった場合1に補正
	if(damage<1) damage=1;
	if(damage2<1) damage2=1;

	// スキル修正２（修練系）
	// 修練ダメージ(右手のみ) ソニックブロー時は別処理（1撃に付き1/8適応)
	if(mastery_flag && skill_num != CR_GRANDCROSS) {			//修練ダメージ無視
		//Advanced Katar Research by zanetheinsane
		if((sd->weapontype1 == 0x10 || sd->weapontype2 == 0x10) && (skill = pc_checkskill(sd,ASC_KATAR)) > 0)
				damage += damage*(10+(skill * 2))/100;
		damage = battle_addmastery(sd,target,damage,0);
		damage2 = battle_addmastery(sd,target,damage2,1);
	}

	// Aura Blade adds dmg [DracoRPG]
	if(sc_data && sc_data[SC_AURABLADE].timer!=-1) {
		damage += sc_data[SC_AURABLADE].val1 * 20;
		damage2 += sc_data[SC_AURABLADE].val1 * 20;
	}

	// A weapon forged by a Blacksmith in the Fame Top 10 gets +10 dmg [DracoRPG]
	if(sd->right_weapon.fameflag)
		damage += 10;
	if(sd->left_weapon.fameflag)
	    damage2 += 10;

	if (sd->perfect_hit > 0 && rand()%100 < sd->perfect_hit)
		hitrate = 1000000;

	// 回避修正
	if (hitrate < 1000000 && t_sc_data) {			// 必中攻撃
		if (t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			hitrate -= 75;
		if (t_sc_data[SC_SLEEP].timer!=-1 ||	// 睡眠は必中
			t_sc_data[SC_STAN].timer!=-1 ||		// スタンは必中
			t_sc_data[SC_FREEZE].timer!=-1 ||
			(t_sc_data[SC_STONE].timer!=-1 && t_sc_data[SC_STONE].val2==0))	// 凍結は必中
			hitrate = 1000000;
	}
	// weapon research hidden bonus
	if ((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0) {
		hitrate = hitrate * (100+2*skill) / 100;
	}
	if(hitrate < 5)
		hitrate = 5;
	if (type == 0 && rand()%100 >= hitrate) {
		damage = damage2 = 0;
		dmg_lv = ATK_FLEE;
	} else {
		dmg_lv = ATK_DEF;
	}

	// スキル修正３（武器研究）
	if ((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0) {
		damage += skill*2;
		damage2 += skill*2;
	}

//スキルによるダメージ補正ここまで

//カードによるダメージ追加処理ここから
	cardfix=100;
	if(!sd->state.arrow_atk) { //弓矢以外
		if(!battle_config.left_cardfix_to_right) { //左手カード補正設定無し
			cardfix=cardfix*(100+sd->right_weapon.addrace[t_race])/100;	// 種族によるダメージ修正
			cardfix=cardfix*(100+sd->right_weapon.addele[t_ele])/100;	// 属性によるダメージ修正
			cardfix=cardfix*(100+sd->right_weapon.addsize[t_size])/100;	// サイズによるダメージ修正
			cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
		}
		else {
			cardfix=cardfix*(100+sd->right_weapon.addrace[t_race]+sd->left_weapon.addrace[t_race])/100;	// 種族によるダメージ修正(左手による追加あり)
			cardfix=cardfix*(100+sd->right_weapon.addele[t_ele]+sd->left_weapon.addele[t_ele])/100;	// 属性によるダメージ修正(左手による追加あり)
			cardfix=cardfix*(100+sd->right_weapon.addsize[t_size]+sd->left_weapon.addsize[t_size])/100;	// サイズによるダメージ修正(左手による追加あり)
			cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2]+sd->left_weapon.addrace2[t_race2])/100;
		}
	}
	else { //弓矢
		cardfix=cardfix*(100+sd->right_weapon.addrace[t_race]+sd->arrow_addrace[t_race])/100;	// 種族によるダメージ修正(弓矢による追加あり)
		cardfix=cardfix*(100+sd->right_weapon.addele[t_ele]+sd->arrow_addele[t_ele])/100;	// 属性によるダメージ修正(弓矢による追加あり)
		cardfix=cardfix*(100+sd->right_weapon.addsize[t_size]+sd->arrow_addsize[t_size])/100;	// サイズによるダメージ修正(弓矢による追加あり)
		cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
	}
	if(is_boss(target)) { //ボス
		if(!sd->state.arrow_atk) { //弓矢攻撃以外なら
			if(!battle_config.left_cardfix_to_right) //左手カード補正設定無し
				cardfix=cardfix*(100+sd->right_weapon.addrace[10])/100; //ボスモンスターに追加ダメージ
			else //左手カード補正設定あり
				cardfix=cardfix*(100+sd->right_weapon.addrace[10]+sd->left_weapon.addrace[10])/100; //ボスモンスターに追加ダメージ(左手による追加あり)
		}
		else //弓矢攻撃
			cardfix=cardfix*(100+sd->right_weapon.addrace[10]+sd->arrow_addrace[10])/100; //ボスモンスターに追加ダメージ(弓矢による追加あり)
	}
	else { //ボスじゃない
		if(!sd->state.arrow_atk) { //弓矢攻撃以外
			if(!battle_config.left_cardfix_to_right) //左手カード補正設定無し
				cardfix=cardfix*(100+sd->right_weapon.addrace[11])/100; //ボス以外モンスターに追加ダメージ
			else //左手カード補正設定あり
				cardfix=cardfix*(100+sd->right_weapon.addrace[11]+sd->left_weapon.addrace[11])/100; //ボス以外モンスターに追加ダメージ(左手による追加あり)
		}
		else
			cardfix=cardfix*(100+sd->right_weapon.addrace[11]+sd->arrow_addrace[11])/100; //ボス以外モンスターに追加ダメージ(弓矢による追加あり)
	}
	//特定Class用補正処理(少女の日記→ボンゴン用？)
	t_class = status_get_class(target);
	for(i=0;i<sd->right_weapon.add_damage_class_count;i++) {
		if(sd->right_weapon.add_damage_classid[i] == t_class) {
			cardfix=cardfix*(100+sd->right_weapon.add_damage_classrate[i])/100;
			break;
		}
	}
	if(!no_cardfix)
		damage=damage*cardfix/100; //カード補正によるダメージ増加
//カードによるダメージ増加処理ここまで

//カードによるダメージ追加処理(左手)ここから
	cardfix=100;
	if(!battle_config.left_cardfix_to_right) {  //左手カード補正設定無し
		cardfix=cardfix*(100+sd->left_weapon.addrace[t_race])/100;	// 種族によるダメージ修正左手
		cardfix=cardfix*(100+sd->left_weapon.addele[t_ele])/100;	// 属 性によるダメージ修正左手
		cardfix=cardfix*(100+sd->left_weapon.addsize[t_size])/100;	// サイズによるダメージ修正左手
		cardfix=cardfix*(100+sd->left_weapon.addrace2[t_race2])/100;
		if(is_boss(target)) //ボス
			cardfix=cardfix*(100+sd->left_weapon.addrace[10])/100; //ボスモンスターに追加ダメージ左手
		else
			cardfix=cardfix*(100+sd->left_weapon.addrace[11])/100; //ボス以外モンスターに追加ダメージ左手
	}
	//特定Class用補正処理左手(少女の日記→ボンゴン用？)
	for(i=0;i<sd->left_weapon.add_damage_class_count;i++) {
		if(sd->left_weapon.add_damage_classid[i] == t_class) {
			cardfix=cardfix*(100+sd->left_weapon.add_damage_classrate[i])/100;
			break;
		}
	}
	if(!no_cardfix)
		damage2=damage2*cardfix/100;

//カード補正による左手ダメージ増加
//カードによるダメージ増加処理(左手)ここまで

//カードによるダメージ減衰処理ここから
	if(tsd){ //対象がPCの場合
		cardfix=100;
		cardfix=cardfix*(100-tsd->subrace[s_race])/100;	// 種族によるダメージ耐性
		cardfix=cardfix*(100-tsd->subele[s_ele])/100;	// 属性によるダメージ耐性
		cardfix=cardfix*(100-tsd->subsize[s_size])/100;
		if(is_boss(src))
			cardfix=cardfix*(100-tsd->subrace[10])/100; //ボスからの攻撃はダメージ減少
		else
			cardfix=cardfix*(100-tsd->subrace[11])/100; //ボス以外からの攻撃はダメージ減少
		//特定Class用補正処理左手(少女の日記→ボンゴン用？)
		for(i=0;i<tsd->add_def_class_count;i++) {
			if(tsd->add_def_classid[i] == sd->status.class_) {
				cardfix=cardfix*(100-tsd->add_def_classrate[i])/100;
				break;
			}
		}
		if(flag&BF_LONG)
			cardfix=cardfix*(100-tsd->long_attack_def_rate)/100; //遠距離攻撃はダメージ減少(ホルンCとか)
		if(flag&BF_SHORT)
			cardfix=cardfix*(100-tsd->near_attack_def_rate)/100; //近距離攻撃はダメージ減少(該当無し？)
		damage=damage*cardfix/100; //カード補正によるダメージ減少
		damage2=damage2*cardfix/100; //カード補正による左手ダメージ減少
	}
//カードによるダメージ減衰処理ここまで

//対象にステータス異常がある場合のダメージ減算処理ここから
	if(t_sc_data) {
		cardfix=100;
		if(t_sc_data[SC_DEFENDER].timer != -1 && flag&BF_LONG) //ディフェンダー状態で遠距離攻撃
		{
			if(map[target->m].flag.gvg)
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2/2)/100;				
			else
				cardfix=cardfix*(100-t_sc_data[SC_DEFENDER].val2)/100;
		}
		if(t_sc_data[SC_FOGWALL].timer != -1 && flag&BF_LONG)
			cardfix=cardfix*50/100;
		if(cardfix != 100) {
			damage=damage*cardfix/100; //ディフェンダー補正によるダメージ減少
			damage2=damage2*cardfix/100; //ディフェンダー補正による左手ダメージ減少
		}
		if(t_sc_data[SC_ASSUMPTIO].timer != -1 && !map[target->m].flag.gvg){ //アスムプティオ
			if(!map[target->m].flag.pvp){
				damage=damage/3;
				damage2=damage2/3;
			}else{
				damage=damage/2;
				damage2=damage2/2;
			}
		}
	}
//対象にステータス異常がある場合のダメージ減算処理ここまで

	if(damage < 0) damage = 0;
	if(damage2 < 0) damage2 = 0;

	// 属 性の適用
	damage=battle_attr_fix(damage,s_ele, status_get_element(target) );
	damage2=battle_attr_fix(damage2,s_ele_, status_get_element(target) );
#if 0
	if(sc_data[SC_MAGNUM].timer!=0)
	{
		damage += battle_attr_fix(damage,3,status_get_element(target))/5;
		damage2 += battle_attr_fix(damage2,3,status_get_element(target))/5;
	}

#endif

	if(skill_num != PA_SACRIFICE)
	{
		// 星のかけら、気球の適用
		damage += sd->right_weapon.star;
		damage2 += sd->left_weapon.star;
		damage += sd->spiritball*3;
		damage2 += sd->spiritball*3;
	}

	if(skill_num==PA_PRESSURE){ /* プレッシャー 必中? */
		damage = 500+300*skill_lv;
		damage2 = 500+300*skill_lv;
	}

	// >二刀流の左右ダメージ計算誰かやってくれぇぇぇぇえええ！
	// >map_session_data に左手ダメージ(atk,atk2)追加して
	// >status_calc_pc()でやるべきかな？
	// map_session_data に左手武器(atk,atk2,ele,star,atkmods)追加して
	// status_calc_pc()でデータを入力しています

	//左手のみ武器装備
	if(sd->weapontype1 == 0 && sd->weapontype2 > 0) {
		damage = damage2;
		damage2 = 0;
	}

	// 右手、左手修練の適用
	if(sd->status.weapon > 16) {// 二刀流か?
		int dmg = damage, dmg2 = damage2;
		// 右手修練(60% ～ 100%) 右手全般
		skill = pc_checkskill(sd,AS_RIGHT);
		damage = damage * (50 + (skill * 10))/100;
		if(dmg > 0 && damage < 1) damage = 1;
		// 左手修練(40% ～ 80%) 左手全般
		skill = pc_checkskill(sd,AS_LEFT);
		damage2 = damage2 * (30 + (skill * 10))/100;
		if(dmg2 > 0 && damage2 < 1) damage2 = 1;
	}
	else //二刀流でなければ左手ダメージは0
		damage2 = 0;

	// 右手,短剣のみ
	if(da == 1) { //ダブルアタックが発動しているか
		div_ = 2;
		damage += damage;
		type = 0x08;
	}

	if(sd->status.weapon == 16) {
		// カタール追撃ダメージ
		skill = pc_checkskill(sd,TF_DOUBLE);
		damage2 = damage * (1 + (skill * 2))/100;
		if(damage > 0 && damage2 < 1) damage2 = 1;
	}

	// インベナム修正
	if(skill_num==TF_POISON){
		damage = battle_attr_fix(damage + 15*skill_lv, s_ele, status_get_element(target) );
	}
	if(skill_num==MC_CARTREVOLUTION){
		damage = battle_attr_fix(damage, 0, status_get_element(target) );
	}

	// 完全回避の判定
	if(skill_num == 0 && tsd!=NULL && div_ < 255 && rand()%1000 < status_get_flee2(target) ){
		damage=damage2=0;
		type=0x0b;
		dmg_lv = ATK_LUCKY;
	}

	// 対象が完全回避をする設定がONなら
	if(battle_config.enemy_perfect_flee) {
		if(skill_num == 0 && tmd!=NULL && div_ < 255 && rand()%1000 < status_get_flee2(target) ) {
			damage=damage2=0;
			type=0x0b;
			dmg_lv = ATK_LUCKY;
		}
	}

	//MobのModeに頑強フラグが立っているときの処理
	if(t_mode&0x40){
		if(damage > 0)
			damage = 1;
		if(damage2 > 0)
			damage2 = 1;
	}

	if(is_boss(target))
		blewcount = 0;

	//bNoWeaponDamage(設定アイテム無し？)でグランドクロスじゃない場合はダメージが0
	if( tsd && tsd->special_state.no_weapon_damage && skill_num != CR_GRANDCROSS)
		damage = damage2 = 0;

	if(skill_num != CR_GRANDCROSS && (damage > 0 || damage2 > 0) ) {
		if(damage2<1)		// ダメージ最終修正
			damage=battle_calc_damage(src,target,damage,div_,skill_num,skill_lv,flag);
		else if(damage<1)	// 右手がミス？
			damage2=battle_calc_damage(src,target,damage2,div_,skill_num,skill_lv,flag);
		else {	// 両 手/カタールの場合はちょっと計算ややこしい
			int d1=damage+damage2,d2=damage2;
			damage=battle_calc_damage(src,target,damage+damage2,div_,skill_num,skill_lv,flag);
			damage2=(d2*100/d1)*damage/100;
			if(damage > 1 && damage2 < 1) damage2=1;
			damage-=damage2;
		}
	}

	/*				For executioner card [Valaris]				*/
		if(src->type == BL_PC && sd->random_attack_increase_add > 0 && sd->random_attack_increase_per > 0 && skill_num == 0 ){
			if(rand()%100 < sd->random_attack_increase_per){
				if(damage >0) damage*=sd->random_attack_increase_add/100;
				if(damage2 >0) damage2*=sd->random_attack_increase_add/100;
				}
		}
	/*					End addition					*/

		// for azoth weapon [Valaris]
		if(src->type == BL_PC && target->type == BL_MOB && sd->classchange) {
			 if(rand()%10000 < sd->classchange) {
 			 	int changeclass[]={
				1001,1002,1004,1005,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1018,1019,1020,
				1021,1023,1024,1025,1026,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1040,1041,
				1042,1044,1045,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1060,1061,
				1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,1076,1077,1078,1079,1080,1081,1083,
				1084,1085,1094,1095,1097,1099,1100,1101,1102,1103,1104,1105,1106,1107,1108,1109,1110,
				1111,1113,1114,1116,1117,1118,1119,1121,1122,1123,1124,1125,1126,1127,1128,1129,1130,
				1131,1132,1133,1134,1135,1138,1139,1140,1141,1142,1143,1144,1145,1146,1148,1149,1151,
				1152,1153,1154,1155,1156,1158,1160,1161,1163,1164,1165,1166,1167,1169,1170,1174,1175,
				1176,1177,1178,1179,1180,1182,1183,1184,1185,1188,1189,1191,1192,1193,1194,1195,1196,
				1197,1199,1200,1201,1202,1204,1205,1206,1207,1208,1209,1211,1212,1213,1214,1215,1216,
				1219,1242,1243,1245,1246,1247,1248,1249,1250,1253,1254,1255,1256,1257,1258,1260,1261,
				1263,1264,1265,1266,1267,1269,1270,1271,1273,1274,1275,1276,1277,1278,1280,1281,1282,
				1291,1292,1293,1294,1295,1297,1298,1300,1301,1302,1304,1305,1306,1308,1309,1310,1311,
				1313,1314,1315,1316,1317,1318,1319,1320,1321,1322,1323,1364,1365,1366,1367,1368,1369,
				1370,1371,1372,1374,1375,1376,1377,1378,1379,1380,1381,1382,1383,1384,1385,1386,1387,
				1390,1391,1392,1400,1401,1402,1403,1404,1405,1406,1408,1409,1410,1412,1413,1415,1416,
				1417,1493,1494,1495,1497,1498,1499,1500,1502,1503,1504,1505,1506,1507,1508,1509,1510,
				1511,1512,1513,1514,1515,1516,1517,1519,1520,1582,1584,1585,1586,1587 };
				mob_class_change(((struct mob_data *)target),changeclass);
			}
		}

	wd.damage=damage;
	wd.damage2=damage2;
	wd.type=type;
	wd.div_=div_;
	wd.amotion=status_get_amotion(src);
	if(skill_num == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=status_get_dmotion(target);
	wd.blewcount=blewcount;
	wd.flag=flag;
	wd.dmg_lv=dmg_lv;

	return wd;
}

/*==========================================
 * battle_calc_weapon_attack_sub (by Skotlex)
 *------------------------------------------
 */
static struct Damage battle_calc_weapon_attack_sub(
	struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	struct map_session_data *sd=NULL, *tsd=NULL;
	struct mob_data *md=NULL, *tmd=NULL;
	struct pet_data *pd=NULL;//, *tpd=NULL; (Noone can target pets)
	struct Damage wd;
	short skill=0;
	unsigned short skillratio = 100;	//Skill dmg modifiers

	short i;
	short t_mode = status_get_mode(target), t_size = status_get_size(target);
	short t_race=0, t_ele=0, s_race=0;	//Set to 0 because the compiler does not notices they are NOT gonna be used uninitialized
	short s_ele, s_ele_;
	short def1, def2;
	struct status_change *sc_data = status_get_sc_data(src);
	struct status_change *t_sc_data = status_get_sc_data(target);
	struct bcwasf	{	//bcwasf stands for battle_calc_weapon_attack_sub flags
		unsigned hit : 1; //the attack Hit? (not a miss)
		unsigned cri : 1;		//Critical hit
		unsigned idef : 1;	//Ignore defense
		unsigned idef2 : 1;	//Ignore defense (left weapon)
		unsigned infdef : 1;	//Infinite defense (plants?)
		unsigned arrow : 1;	//Attack is arrow-based
		unsigned rh : 1;		//Attack considers right hand (wd.damage)
		unsigned lh : 1;		//Attack considers left hand (wd.damage2)
		unsigned cardfix : 1;
	}	flag;	

	memset(&wd,0,sizeof(wd));
	memset(&flag,0,sizeof(struct bcwasf));

	if(src==NULL || target==NULL)
	{
		nullpo_info(NLP_MARK);
		return wd;
	}
	//Initial flag
	flag.rh=1;
	flag.cardfix=1;

	//Initial Values
	wd.type=0; //Normal attack
	wd.div_=skill_get_num(skill_num,skill_lv);
	wd.amotion=status_get_amotion(src);
	if(skill_num == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=status_get_dmotion(target);
	wd.blewcount==skill_get_blewcount(skill_num,skill_lv);
	wd.flag=BF_SHORT|BF_WEAPON|BF_NORMAL; //Initial Flag
	wd.dmg_lv=ATK_DEF;	//This assumption simplifies the assignation later

	switch (src->type)
	{
		case BL_PC:
			sd=(struct map_session_data *)src;
			break;
		case BL_MOB:
			md=(struct mob_data *)src;
			break;
		case BL_PET:
			pd=(struct pet_data *)src;
			break;
	}
	switch (target->type)
	{
		case BL_PC:	
			tsd=(struct map_session_data *)target;
			if (pd) { //Pets can't target players
				memset(&wd,0,sizeof(wd));	
				return wd;
			}
			break;
		case BL_MOB:
			tmd=(struct mob_data *)target;
			break;
		case BL_PET://Cannot target pets
			memset(&wd,0,sizeof(wd));	
			return wd;
	}

	if(sd && skill_num != CR_GRANDCROSS)
		sd->state.attack_type = BF_WEAPON;

	//Set miscelanous data that needs be filled regardless of hit/miss
	if(sd)
	{
		if (sd->status.weapon == 11)
		{
			wd.flag=(wd.flag&~BF_RANGEMASK)|BF_LONG;
			flag.arrow = 1;
		}
	} else if (status_get_range(src) > 3)
		wd.flag=(wd.flag&~BF_RANGEMASK)|BF_LONG;

	if(skill_num){
		wd.flag=(wd.flag&~BF_SKILLMASK)|BF_SKILL;
		switch(skill_num)
		{		
			case AC_DOUBLE:
			case AC_SHOWER:
			case AC_CHARGEARROW:
			case BA_MUSICALSTRIKE:
			case DC_THROWARROW:
			case CG_ARROWVULCAN:
				wd.flag=(wd.flag&~BF_RANGEMASK)|BF_LONG;
				flag.arrow = 1;
				break;

			case MO_FINGEROFFENSIVE:
				if(sd && battle_config.finger_offensive_type == 0)
					wd.div_ = sd->spiritball_old;
			case KN_SPEARBOOMERANG:
			case NPC_RANGEATTACK:
			case CR_SHIELDBOOMERANG:
			case LK_SPIRALPIERCE:
			case ASC_BREAKER:
			case PA_SHIELDCHAIN:
//			case CR_GRANDCROSS:	//GrandCross really shouldn't count as short-range, aight?
				wd.flag=(wd.flag&~BF_RANGEMASK)|BF_LONG;
				break;
			case KN_PIERCE:
				wd.div_= t_size+1;
				break;

			case KN_SPEARSTAB:
			case KN_BOWLINGBASH:
				wd.blewcount=0;
				break;

			case NPC_PIERCINGATT:
			case CR_SHIELDCHARGE:
				wd.flag=(wd.flag&~BF_RANGEMASK)|BF_SHORT;
				break;

			case KN_AUTOCOUNTER:
				wd.flag=(wd.flag&~BF_SKILLMASK)|BF_NORMAL;
				break;
		}
	}

	if(is_boss(target)) //Bosses can't be knocked-back
		wd.blewcount = 0;

	if (sd)
	{	//Arrow consumption
		sd->state.arrow_atk = flag.arrow;
	}

	//Check for counter 
	if(skill_num != CR_GRANDCROSS &&
 		(!skill_num ||
		(tsd && battle_config.pc_auto_counter_type&2) ||
		(tmd && battle_config.monster_auto_counter_type&2)))
	{
		if(t_sc_data && t_sc_data[SC_AUTOCOUNTER].timer != -1)
		{
			int dir = map_calc_dir(src,target->x,target->y),t_dir = status_get_dir(target);
			int dist = distance(src->x,src->y,target->x,target->y);
			if(dist <= 0 || map_check_dir(dir,t_dir) )
			{
				memset(&wd,0,sizeof(wd));
				t_sc_data[SC_AUTOCOUNTER].val3 = 0;
				t_sc_data[SC_AUTOCOUNTER].val4 = 1;
				if(sc_data && sc_data[SC_AUTOCOUNTER].timer == -1)
				{ //How can the attacking char have Auto-counter active?
					int range = status_get_range(target);
					if((tsd && tsd->status.weapon != 11 && dist <= range+1) ||
						(tmd && range <= 3 && dist <= range+1))
						t_sc_data[SC_AUTOCOUNTER].val3 = src->id;
				}
				return wd;
			} else
				flag.cri = 1;
		}
		else if(t_sc_data && t_sc_data[SC_POISONREACT].timer != -1)
		{	// poison react [Celest]
			t_sc_data[SC_POISONREACT].val3 = 0;
			t_sc_data[SC_POISONREACT].val4 = 1;
			t_sc_data[SC_POISONREACT].val3 = src->id;
		}
	}	//End counter-check

	if(sd && !skill_num && !flag.cri)
	{	//Check for conditions that convert an attack to a skill
		char da=0;
		skill = 0;
		if((sd->weapontype1 == 0x01 && (skill = pc_checkskill(sd,TF_DOUBLE)) > 0) ||
			sd->double_rate > 0) //success rate from Double Attack is counted in
			da = (rand()%100 <  sd->double_rate + 5*skill) ? 1:0;
		if((skill = pc_checkskill(sd,MO_TRIPLEATTACK)) > 0 && sd->status.weapon <= 16) // triple blow works with bows ^^ [celest]
			da = (rand()%100 < (30 - skill)) ? 2:da;
		
		if (da == 1)
		{
			skill_num = TF_DOUBLE;
			wd.div_ = 2;
		} else if (da == 2) {
			skill_num = MO_TRIPLEATTACK;
			skill_lv = skill;
			wd.div_ = 255;
		}
		
		if (da)
			wd.type = 0x08;
	}

	if (!skill_num && (tsd || battle_config.enemy_perfect_flee))
	{	//Check for Lucky Dodge
		short flee2 = status_get_flee2(target);
		if (rand()%1000 < flee2)
		{
			wd.type=0x0b;
			wd.dmg_lv=ATK_LUCKY;
			return wd;
		}
	}

	//Initialize variables that will be used afterwards
	if (sd)
	{
		t_race = status_get_race(target);
		t_ele = status_get_elem_type(target);
	}
	if (tsd)
	{
		s_race = status_get_race(src);
	}
	s_ele=status_get_attack_element(src);
	s_ele_=status_get_attack_element2(src);

	if (flag.arrow && sd && sd->arrow_ele)
		s_ele = sd->arrow_ele;

	if (sd)
	{	//Set whether damage1 or damage2 (or both) will be used
		if(sd->weapontype1 == 0 && sd->weapontype2 > 0)
			{
				flag.rh=0;
				flag.lh=1;
			}
		if(sd->status.weapon > 16)
			flag.rh = flag.lh = 1;
	}

	//Check for critical
	if(!flag.cri &&
		(sd || battle_config.enemy_critical) &&
		(!skill_num || skill_num == KN_AUTOCOUNTER || skill_num == SN_SHARPSHOOTING))
	{
		short cri = status_get_critical(target);
		cri -= status_get_luk(target) * 2; //RoDatazone claims that it should be 2
		if (sd)
		{
			cri+= sd->critaddrace[t_race];
			if(flag.arrow)
				cri += sd->arrow_cri;
			if(sd->status.weapon == 16)
				cri <<=1;
		}
		else if(battle_config.enemy_critical_rate != 100)
		{ //Mob/Pets
			cri = cri*battle_config.enemy_critical_rate/100;
			if (cri<1) cri = 1;
		}
		
		if(t_sc_data)
		{
			if (t_sc_data[SC_SLEEP].timer!=-1 )
				cri <<=1;
			if(t_sc_data[SC_JOINTBEAT].timer != -1 &&
				t_sc_data[SC_JOINTBEAT].val2 == 6) // Always take crits with Neck broken by Joint Beat [DracoRPG]
				flag.cri=1;
		}
		switch (skill_num)
		{
			case KN_AUTOCOUNTER:
				if(!(battle_config.pc_auto_counter_type&1))
					flag.cri = 1;
				else
					cri <<= 1;
				break;
			case SN_SHARPSHOOTING:
				cri += 200;
				break;
		}
		if(tsd && tsd->critical_def)
			cri = cri*(100-tsd->critical_def)/100;
		if (flag.cri || rand() % 1000 < cri)
			flag.cri= 1;
	}
	if (flag.cri)
	{
		wd.type = 0x0a;
		flag.idef = flag.idef2 = flag.hit = 1;
	} else {	//Check for Perfect Hit
		if(sd && sd->perfect_hit > 0 && rand()%100 < sd->perfect_hit)
			flag.hit = 1;
		if (skill_num && !flag.hit)
			switch(skill_num)
		{
			case NPC_GUIDEDATTACK:
			case RG_BACKSTAP:
			case CR_GRANDCROSS:
			case AM_ACIDTERROR:
			case MO_INVESTIGATE:
			case MO_EXTREMITYFIST:
				flag.hit = 1;
				break;
		}
		if ((t_sc_data && !flag.hit) &&
			(t_sc_data[SC_SLEEP].timer!=-1 ||
			t_sc_data[SC_STAN].timer!=-1 ||
			t_sc_data[SC_FREEZE].timer!=-1 ||
			(t_sc_data[SC_STONE].timer!=-1 && t_sc_data[SC_STONE].val2==0))
			)
			flag.hit = 1;
	}

	if (!flag.hit)
	{	//Hit/Flee calculation
		short
			flee = status_get_flee(target),
			hitrate=80; //Default hitrate
		if(battle_config.agi_penalty_type)
		{	
			unsigned char target_count; //256 max targets should be a sane max
			target_count = 1+battle_counttargeted(target,src,battle_config.agi_penalty_count_lv);
			if(target_count >= battle_config.agi_penalty_count)
			{
				if (battle_config.agi_penalty_type == 1)
					flee = (flee * (100 - (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
				else //asume type 2: absolute reduction
					flee -= (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
				if(flee < 1) flee = 1;
			}
		}

		hitrate+= (status_get_hit(src) + skill_gethitbonus(skill_num,skill_lv))- flee;
		
		if(sd)
		{	
			if (flag.arrow)
				hitrate += sd->arrow_hit;
			// weapon research hidden bonus
			if ((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
				hitrate += hitrate * (2*skill)/100;
		}
		if(skill_num)
			switch(skill_num)
		{	//Hit skill modifiers
			case SM_BASH:
//				hitrate += (skill_lv>5?20:10);
//				hitrate += skill_lv*5;
				break;
			case SM_MAGNUM:
				//hitrate += hitrate*(10*skill_lv)/100;
				break;
			case KN_AUTOCOUNTER:
				hitrate += 20;
				break;
			case KN_PIERCE:
				//hitrate += hitrate*(5*skill_lv)/100;
				break;
			case PA_SHIELDCHAIN:
				//hitrate += 20;
				break;
		}

		hitrate = ((hitrate>95)?95: ((hitrate<5)?5:hitrate));

		if(rand()%100 >= hitrate)
			wd.dmg_lv = ATK_FLEE;
		else
			flag.hit =1;
	}	//End hit/miss calculation

	if(tsd && tsd->special_state.no_weapon_damage && skill_num != CR_GRANDCROSS)	
		return wd;

	if (flag.hit && !(t_mode&0x40)) //No need to do the math for plants
	{	//Hitting attack

//Assuming that 99% of the cases we will not need to check for the flag.rh... we don't.
//ATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define ATK_RATE( a ) { wd.damage= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(a)/100; }
#define ATK_RATE2( a , b ) { wd.damage= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(b)/100; }
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define ATK_ADDRATE( a ) { wd.damage+= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(a)/100; }
#define ATK_ADDRATE2( a , b ) { wd.damage+= wd.damage*(a)/100 ; if(flag.lh) wd.damage2+= wd.damage2*(b)/100; }
//Adds an absolute value to damage. 100 = +100 damage
#define ATK_ADD( a ) { wd.damage+= a; if (flag.lh) wd.damage2+= a; }
#define ATK_ADD2( a , b ) { wd.damage+= a; if (flag.lh) wd.damage2+= b; }

		if (status_get_def(target) >= 1000000)
			flag.infdef =1;
		def1 = status_get_def(target);
		def2 = status_get_def2(target);
		
		switch (skill_num)
		{	//Calc base damage according to skill
			case PA_PRESSURE: //Since PRESSURE ignores everything, finish here
				wd.damage=battle_calc_damage(src,target,500+300*skill_lv,wd.div_,skill_num,skill_lv,wd.flag);
				wd.damage2=0;
				return wd;	
			default:
			{
				unsigned short baseatk=0, baseatk_=0, atkmin=0, atkmax=0, atkmin_=0, atkmax_=0;
				if (!sd)
				{	//Mobs/Pets
					if ((md && battle_config.enemy_str) ||
						(pd && battle_config.pet_str))
						baseatk = status_get_baseatk(src);

					if(skill_num==HW_MAGICCRASHER)
					{		  
						if (!flag.cri)
							atkmin = status_get_matk1(src);
						atkmax = status_get_matk2(src);
					} else {
						if (!flag.cri)
							atkmin = status_get_atk(src);
						atkmax = status_get_atk2(src);
					}
					if (atkmin > atkmax)
						atkmin = atkmax;
				} else {	//PCs
					if(skill_num==HW_MAGICCRASHER)
					{
						baseatk = status_get_matk1(src);
						if (flag.lh) baseatk_ = baseatk;
					} else { 
						baseatk = status_get_baseatk(src);
						if (flag.lh) baseatk_ = baseatk;
					}
					//rodatazone says that Overrefine bonuses are part of baseatk
					if(sd->right_weapon.overrefine>0)
						baseatk+= rand()%sd->right_weapon.overrefine+1;
					if (flag.lh && sd->left_weapon.overrefine>0)
						baseatk_+= rand()%sd->left_weapon.overrefine+1;
					
					atkmax = status_get_atk(src);
					if (flag.lh)
						atkmax_ = status_get_atk(src);

					if (!flag.cri)
					{	//Normal attacks
						atkmin = atkmin_ = status_get_dex(src);
						
						if (sd->equip_index[9] >= 0 && sd->inventory_data[sd->equip_index[9]])
							atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[9]]->wlv*20)/100;
						
						if (atkmin > atkmax)
							atkmin = atkmax;
						
						if(flag.lh)
						{
							if (sd->equip_index[8] >= 0 && sd->inventory_data[sd->equip_index[8]])
								atkmin_ = atkmin_*(80 + sd->inventory_data[sd->equip_index[8]]->wlv*20)/100;
						
							if (atkmin_ > atkmax_)
								atkmin_ = atkmax_;
						}
						
						if(sd->status.weapon == 11)
						{	//Bows
							atkmin = atkmin*atkmax/100;
							if (atkmin > atkmax)
								atkmax = atkmin;
						}
					}
				}
				
				if (sc_data && sc_data[SC_MAXIMIZEPOWER].timer!=-1)
				{
					atkmin = atkmax;
					atkmin_ = atkmax_;
				}
				//Weapon Damage calculation
				//Store watk in wd.damage to use the above defines for easy handling, and then add baseatk
				if (!flag.cri)
				{
					ATK_ADD2((atkmax>atkmin? rand()%(atkmax-atkmin) :0) +atkmin,
						(atkmax_>atkmin_? rand()%(atkmax_-atkmin_) :0) +atkmin_);
				} else 
					ATK_ADD2(atkmax, atkmax_);
				
				if (sd)
				{
					//rodatazone says the range is 0~arrow_atk-1 for non crit
					if (flag.arrow && sd->arrow_atk)
						ATK_ADD(flag.cri?sd->arrow_atk:rand()%sd->arrow_atk);
					
					if(sd->status.weapon < 16 && (sd->atk_rate != 100 || sd->weapon_atk_rate != 0))
						ATK_RATE(sd->atk_rate + sd->weapon_atk_rate[sd->status.weapon]);

					if(flag.cri && sd->crit_atk_rate)
						ATK_ADDRATE(sd->crit_atk_rate);
				
					//SizeFix only for players
					if (!(
						/*!tsd || //rodatazone claims that target human players don't have a size! -- I really don't believe it... removed until we find some evidence*/
						sd->special_state.no_sizefix ||
						(sc_data && sc_data[SC_WEAPONPERFECTION].timer!=-1) ||
						(pc_isriding(sd) && (sd->status.weapon==4 || sd->status.weapon==5) && t_size==1) ||
						(skill_num == MO_EXTREMITYFIST)
						))
						ATK_RATE2(sd->right_weapon.atkmods[t_size], sd->left_weapon.atkmods[t_size]);	}
				
				//Finally, add baseatk
				ATK_ADD2(baseatk, baseatk_);
				break;
			}	//End default case
		} //End switch(skill_num)

		//Skill damage modifiers
		if(sc_data && skill_num != PA_SACRIFICE)
		{
			if(sc_data[SC_OVERTHRUST].timer!=-1)
				skillratio += 5*sc_data[SC_OVERTHRUST].val1;
			if(sc_data[SC_TRUESIGHT].timer!=-1)
				skillratio += 2*sc_data[SC_TRUESIGHT].val1;
#if 0 //moved (aru)
			if(sc_data[SC_BERSERK].timer!=-1)
				skillratio += 100; // Although RagnaInfo says +200%, it's *200% so +100%
#endif
			if(sc_data[SC_MAXOVERTHRUST].timer!=-1)
				skillratio += 20*sc_data[SC_MAXOVERTHRUST].val1;
			if(sc_data[SC_EDP].timer != -1 &&
				!is_boss(target) && //Enchant Deadly Poison does not work on bosses and with those 3 skills
				skill_num != AS_SPLASHER &&
				skill_num != ASC_BREAKER &&
				skill_num != ASC_METEORASSAULT)
			{	
				skillratio += 150 + sc_data[SC_EDP].val1 * 50;
				flag.cardfix = 0;
			}
		}
		if (!skill_num)
		{
			//Executioner card addition - Consider it as part of skill-based-damage
			if(sd &&
				sd->random_attack_increase_add > 0 &&
				sd->random_attack_increase_per &&
				rand()%100 < sd->random_attack_increase_per
				)
				skillratio += sd->random_attack_increase_add;
		
			ATK_RATE(skillratio);
		} else {	//Skills
			char ele_flag=0;	//For skills that force the neutral element.

			switch( skill_num )
			{
				case SM_BASH:
					skillratio+= 30*skill_lv;
					break;
				case SM_MAGNUM:
					// 20*skill level+100? I think this will do for now [based on jRO info]
					skillratio+= (wflag > 1 ? 5*skill_lv+15 : 30*skill_lv);
					break;
				case MC_MAMMONITE:
					skillratio+= 50*skill_lv;
					break;
				case AC_DOUBLE:
					skillratio+= 80+ 20*skill_lv;
					break;
				case AC_SHOWER:
					skillratio+= 5*skill_lv -25;
					break;
				case AC_CHARGEARROW:
					skillratio+= 50;
					break;
				case KN_PIERCE:
					skillratio+= wd.div_*(100+10*skill_lv) -100;
					//div_flag=1;
					break;
				case KN_SPEARSTAB:
					skillratio+= 15*skill_lv;
					break;
				case KN_SPEARBOOMERANG:
					skillratio+= 50*skill_lv;
					break;
				case KN_BRANDISHSPEAR:
					skillratio+=20*skill_lv;
					if(skill_lv>3 && wflag==1) skillratio+= 50;
					if(skill_lv>6 && wflag==1) skillratio+= 25;
					if(skill_lv>9 && wflag==1) skillratio+= 12; //1/8th = 12.5%, rounded to 12?
					if(skill_lv>6 && wflag==2) skillratio+= 50;
					if(skill_lv>9 && wflag==2) skillratio+= 25;
					if(skill_lv>9 && wflag==3) skillratio+= 50;
				case KN_BOWLINGBASH:
					skillratio+= 50*skill_lv;
					break;
				case KN_AUTOCOUNTER:
					flag.idef= flag.idef2= 1;
					break;
				case TF_DOUBLE:
					skillratio += 100;
					break;
				case AS_GRIMTOOTH:
					skillratio+= 20*skill_lv;
					break;
				case AS_POISONREACT: // celest
					skillratio+= 30*skill_lv;
					ele_flag=1;
					break;
				case AS_SONICBLOW:
					skillratio+= 200+ 50*skill_lv;
					break;
				case TF_SPRINKLESAND:
					skillratio+= 25;
					break;
				case MC_CARTREVOLUTION:
					if(sd && sd->cart_max_weight > 0 && sd->cart_weight > 0)
						skillratio+= 50+sd->cart_weight/800; // +1% every 80 weight units
					else
						skillratio+= 50;
					break;
				case NPC_COMBOATTACK:
						skillratio += 100*wd.div_ -100;
						//div_flag=1;
					break;
				case NPC_RANDOMATTACK:
					skillratio+= rand()%150-50;
					break;
				case NPC_WATERATTACK:
				case NPC_GROUNDATTACK:
				case NPC_FIREATTACK:
				case NPC_WINDATTACK:
				case NPC_POISONATTACK:
				case NPC_HOLYATTACK:
				case NPC_DARKNESSATTACK:
				case NPC_UNDEADATTACK:
				case NPC_TELEKINESISATTACK:
					skillratio+= 25*skill_lv;
					break;
				case NPC_GUIDEDATTACK:
				case NPC_RANGEATTACK:
				case NPC_PIERCINGATT:
					break;
				case NPC_CRITICALSLASH:
					flag.idef= flag.idef2= 1;
					break;
				case RG_BACKSTAP:
					if(sd && sd->status.weapon == 11 && battle_config.backstab_bow_penalty)
						skillratio+= (200+ 40*skill_lv)/2;
					else
						skillratio+= 200+ 40*skill_lv;
					break;
				case RG_RAID:
					skillratio+= 40*skill_lv;
					break;
				case RG_INTIMIDATE:
					skillratio+= 30*skill_lv;
					break;
				case CR_SHIELDCHARGE:
					skillratio+= 20*skill_lv;
					ele_flag=1;
					break;
				case CR_SHIELDBOOMERANG:
					skillratio+= 30*skill_lv;
					ele_flag=1;
					break;
				case CR_HOLYCROSS:
					skillratio+= 35*skill_lv;
					break;
				case CR_GRANDCROSS:
					if(!battle_config.gx_cardfix)
						flag.cardfix = 0;
					break;
				case AM_DEMONSTRATION:
					skillratio+= 20*skill_lv;
					flag.cardfix = 0;
					break;
				case AM_ACIDTERROR:
					skillratio+= 40*skill_lv;
					ele_flag=1;
					flag.idef = flag.idef2= 1;
					flag.cardfix = 0;
					break;
				case MO_FINGEROFFENSIVE:
					if(battle_config.finger_offensive_type == 0)
						//div_flag = 1;
						skillratio+= wd.div_ * (100 + 50*skill_lv) -100;
					else
						skillratio+= 50 * skill_lv;
					break;
				case MO_INVESTIGATE:
					if (!flag.infdef)
					{
						skillratio+=75*skill_lv;
						ATK_RATE(2*(def1 + def2));
					}
					flag.idef= flag.idef2= 1;
					ele_flag=1;
					break;
				case MO_EXTREMITYFIST:
					if (sd)
					{
						skillratio+= 100*(8 + ((sd->status.sp)/10));
						sd->status.sp = 0;
						clif_updatestatus(sd,SP_SP);
					}
					flag.idef= flag.idef2= 1;
					ele_flag=1;
					break;
				case MO_TRIPLEATTACK:
					skillratio+= 20*skill_lv;
					break;
				case MO_CHAINCOMBO:
					skillratio+= 50+ 50*skill_lv;
					break;
				case MO_COMBOFINISH:
					skillratio+= 140+ 60*skill_lv;
					break;
				case BA_MUSICALSTRIKE:
#if 0
					skillratio+= 40*skill_lv -40;
					break;
#endif
				case DC_THROWARROW:
					skillratio+= 50*skill_lv;
					break;
				case CH_TIGERFIST:
					skillratio+= 100*skill_lv-60;
					break;
				case CH_CHAINCRUSH:
					skillratio+= 300+ 100*skill_lv;
					break;
				case CH_PALMSTRIKE:
					skillratio+= 100+ 100*skill_lv;
					break;
				case LK_SPIRALPIERCE:
					skillratio+=50*skill_lv;
					flag.idef= flag.idef2= 1;
					if(tsd)
						tsd->canmove_tick = gettick() + 1000;
					else if(tmd)
						tmd->canmove_tick = gettick() + 1000;
					break;
				case LK_HEADCRUSH:
					skillratio+=40*skill_lv;
					break;
				case LK_JOINTBEAT:
					skillratio+= 10*skill_lv-50;
					break;
				case ASC_METEORASSAULT:
					skillratio+= 40*skill_lv-60;
					break;
				case SN_SHARPSHOOTING:
					skillratio+= 30*skill_lv;
					break;
				case CG_ARROWVULCAN:
					skillratio+= 100+100*skill_lv;
					break;
				case AS_SPLASHER:
					skillratio+= 400+50*skill_lv;
					flag.cardfix = 0;
					break;
				case ASC_BREAKER:
					skillratio+= 100*skill_lv -100;
					flag.cardfix = 0;
					break;
				case PA_SACRIFICE:
					skillratio+= 10*skill_lv -10;
					flag.idef = flag.idef2 = 1;
					ele_flag=1;
					break;
				case PA_SHIELDCHAIN:
					skillratio+= wd.div_*(100+30*skill_lv)-100;
					//div_flag=1;
					ele_flag=1;
					break;
				case WS_CARTTERMINATION:
					if(sd && sd->cart_max_weight && sd->cart_weight > 0) //Why check for cart_max_weight? It is not used!
						skillratio += sd->cart_weight / (10 * (16 - skill_lv)) - 100;
					else if (!sd)
						skillratio += 80000 / (10 * (16 - skill_lv));
					break;

			}
			if (ele_flag)
				s_ele=s_ele_=0;
			else if((skill=skill_get_pl(skill_num))>0) //Checking for the skill's element
				s_ele=s_ele_=skill;

			if (sd)
				//If we apply skillatk[] as ATK_RATE, it will also affect other skills,
				//unfortunately this way ignores a skill's constant modifiers...
				skillratio += sd->skill_damage_bonus[skill_num];

			//Double attack does not applies to left hand
			ATK_RATE2(skillratio, skillratio - (skill_num==TF_DOUBLE?100:0));

			//Constant/misc additions from skills
			if (skill_num == MO_EXTREMITYFIST)
				ATK_ADD(250 + 150*skill_lv);

			if (sd)
			{
				short index= 0;
				switch (skill_num)
				{
					case CR_SHIELDBOOMERANG:
					case PA_SHIELDCHAIN:
						if ((index = sd->equip_index[8]) >= 0 &&
							sd->inventory_data[index] &&
							sd->inventory_data[index]->type == 5)
						{
							ATK_ADD(sd->inventory_data[index]->weight/10);
							ATK_ADD(sd->status.inventory[index].refine * status_getrefinebonus(0,1));
						}
						break;
					case LK_SPIRALPIERCE:
						if ((index = sd->equip_index[9]) >= 0 &&
							sd->inventory_data[index] &&
							sd->inventory_data[index]->type == 4)
						{
							ATK_ADD((int)(double)(sd->inventory_data[index]->weight*(0.8*skill_lv*4/10)));
							ATK_ADD(sd->status.inventory[index].refine * status_getrefinebonus(0,1));
						}
						break;
				}	//switch
			}	//if (sd)
		}

		if(sd)
		{
			if (skill_num != PA_SACRIFICE && skill_num != MO_INVESTIGATE && !flag.cri && !flag.infdef)
			{	//Elemental/Racial adjustments
				char raceele_flag=0, raceele_flag_=0;
				if(sd->right_weapon.def_ratio_atk_ele & (1<<t_ele) ||
					sd->right_weapon.def_ratio_atk_race & (1<<t_race) ||
					sd->right_weapon.def_ratio_atk_race & (is_boss(target)?1<<10:1<<11)
					)
					raceele_flag = flag.idef = 1;
				
				if(sd->left_weapon.def_ratio_atk_ele & (1<<t_ele) ||
					sd->left_weapon.def_ratio_atk_race & (1<<t_race) ||
					sd->left_weapon.def_ratio_atk_race & (is_boss(target)?1<<10:1<<11)
					)
					raceele_flag_ = flag.idef2 = 1;
				
				if (raceele_flag || raceele_flag_)
					ATK_RATE2(raceele_flag?(def1 + def2):100, raceele_flag_?(def1 + def2):100);
			}

			//Ignore Defense?
			if (!flag.idef && (
				(tmd && sd->right_weapon.ignore_def_mob & (is_boss(target)?2:1)) ||
				sd->right_weapon.ignore_def_ele & (1<<t_ele) ||
				sd->right_weapon.ignore_def_race & (1<<t_race) ||
				sd->right_weapon.ignore_def_race & (is_boss(target)?1<<10:1<<11)
				))
				flag.idef = 1;

			if (!flag.idef2 && (
				(tmd && sd->left_weapon.ignore_def_mob & (is_boss(target)?2:1)) ||
				sd->left_weapon.ignore_def_ele & (1<<t_ele) ||
				sd->left_weapon.ignore_def_race & (1<<t_race) ||
				sd->left_weapon.ignore_def_race & (is_boss(target)?1<<10:1<<11)
				))
				flag.idef2 = 1;
		}

		if (!flag.infdef && (!flag.idef || !flag.idef2))
		{	//Defense reduction
			short t_vit = status_get_vit(target);
			short t_def, vitbonusmax;
			short defense;

			if(battle_config.vit_penalty_type)
			{
				unsigned char target_count; //256 max targets should be a sane max
				target_count = 1 + battle_counttargeted(target,src,battle_config.vit_penalty_count_lv);
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						t_vit = (t_vit * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					} else { //Assume type 2
						def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						t_vit -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
				}
				if(def1 < 0) def1 = 0;
				if(def2 < 1) def2 = 1;
				if(t_vit < 1) t_vit = 1;
			}
			t_def = def2*8/10;
			if(tsd &&
				(battle_check_undead(s_race,status_get_elem_type(src)) || s_race==6) &&
				(skill=pc_checkskill(tsd,AL_DP)) >0)
				t_def += skill*(int)(3 +(tsd->status.base_level+1)*0.04);   // submitted by orn
			vitbonusmax = (t_vit/20)*(t_vit/20)-1;

			if(battle_config.player_defense_type) {
				defense = -(def1*battle_config.player_defense_type) -t_def -(vitbonusmax<1)?0:rand()%(vitbonusmax+1);
			} else {
				defense = -t_def -(vitbonusmax<1)?0:rand()%(vitbonusmax+1);
				ATK_RATE2(flag.idef?100:100-def1, flag.idef2?100:100-def1);
			}
			ATK_ADD2(flag.idef?0:defense, flag.idef2?0:defense);
		}

		//Post skill/vit reduction damage increases
		if (sc_data)
		{	//SC skill damages
			if(sc_data[SC_AURABLADE].timer!=-1) 
				ATK_ADD(20*sc_data[SC_AURABLADE].val1);
		}

		if (sd && skill_num != MO_INVESTIGATE && skill_num != MO_EXTREMITYFIST)
		{	//refine bonus
			ATK_ADD2(status_get_atk2(src), status_get_atk_2(src));
			//Items forged from the Top 10 most famous BS's get 10 dmg bonus
			ATK_ADD2(sd->right_weapon.fameflag*10, sd->left_weapon.fameflag*10);
		}

		//Set to min of 1
		if (flag.rh && wd.damage < 1) wd.damage = 1;
		if (flag.lh && wd.damage2 < 1) wd.damage2 = 1;

		if (sd && skill_num != MO_INVESTIGATE && skill_num != MO_EXTREMITYFIST && skill_num != CR_GRANDCROSS)
		{	//Add mastery damage
			if((sd->weapontype1 == 0x10 || sd->weapontype2 == 0x10) && (skill = pc_checkskill(sd,ASC_KATAR)) > 0)
				ATK_ADDRATE(10+(skill *2));	//Advanced Katar Research by zanetheinsane
			wd.damage = battle_addmastery(sd,target,wd.damage,0);
			if (flag.lh) wd.damage2 = battle_addmastery(sd,target,wd.damage2,1);
		}
	} //Here ends flag.hit section, the rest of the function applies to both hitting and missing attacks

	if(sd && (skill=pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)
		ATK_ADD(skill*2);

	if(skill_num==TF_POISON)
		ATK_ADD(15*skill_lv);

	if (sd ||
		(md && !skill_num && !battle_config.mob_attack_attr_none) ||
		(pd && !skill_num && !battle_config.pet_attack_attr_none))
	{	//Elemental attribute fix
		if	(!(!sd && tsd && !battle_config.mob_ghostring_fix && t_ele==8))
		{
			short t_element = status_get_element(target);
			if (wd.damage > 0)
			{
				wd.damage=battle_attr_fix(wd.damage,s_ele,t_element);
				if(skill_num==MC_CARTREVOLUTION) //Cart Revolution applies the element fix once more with neutral element
					wd.damage=battle_attr_fix(wd.damage,0,t_element);
			}
			if (flag.lh && wd.damage2 > 0) wd.damage2=battle_attr_fix(wd.damage2,s_ele_,t_element);
		}
	}

	if ((!flag.rh || wd.damage == 0) && (!flag.lh || wd.damage2 == 0))
		flag.cardfix = 0;	//When the attack does no damage, avoid doing %bonuses

	if (sd)
	{
		ATK_ADD2(sd->right_weapon.star, sd->left_weapon.star);
		ATK_ADD(sd->spiritball*3);

		//Card Fix, sd side
		if (flag.cardfix)
		{
			short cardfix = 1000, cardfix_ = 1000;
			short t_class = status_get_class(target);
			short t_race2 = status_get_race2(target);	
			if(sd->state.arrow_atk)
			{
				cardfix=cardfix*(100+sd->right_weapon.addrace[t_race]+sd->arrow_addrace[t_race])/100;
				cardfix=cardfix*(100+sd->right_weapon.addele[t_ele]+sd->arrow_addele[t_ele])/100;
				cardfix=cardfix*(100+sd->right_weapon.addsize[t_size]+sd->arrow_addsize[t_size])/100;
				cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
				cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?10:11]+sd->arrow_addrace[t_mode & 0x20?10:11])/100;
			} else {	//Melee attack
				if(!battle_config.left_cardfix_to_right)
				{
					cardfix=cardfix*(100+sd->right_weapon.addrace[t_race])/100;
					cardfix=cardfix*(100+sd->right_weapon.addele[t_ele])/100;
					cardfix=cardfix*(100+sd->right_weapon.addsize[t_size])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?10:11])/100;

					if (flag.lh)
					{
						cardfix_=cardfix_*(100+sd->left_weapon.addrace[t_race])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addele[t_ele])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addsize[t_size])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addrace2[t_race2])/100;
						cardfix_=cardfix_*(100+sd->left_weapon.addrace[is_boss(target)?10:11])/100;
					}
				} else {
					cardfix=cardfix*(100+sd->right_weapon.addrace[t_race]+sd->left_weapon.addrace[t_race])/100;
					cardfix=cardfix*(100+sd->right_weapon.addele[t_ele]+sd->left_weapon.addele[t_ele])/100;
					cardfix=cardfix*(100+sd->right_weapon.addsize[t_size]+sd->left_weapon.addsize[t_size])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace2[t_race2]+sd->left_weapon.addrace2[t_race2])/100;
					cardfix=cardfix*(100+sd->right_weapon.addrace[is_boss(target)?10:11]+sd->left_weapon.addrace[is_boss(target)?10:11])/100;
				}
			}

			for(i=0;i<sd->right_weapon.add_damage_class_count;i++) {
				if(sd->right_weapon.add_damage_classid[i] == t_class) {
					cardfix=cardfix*(100+sd->right_weapon.add_damage_classrate[i])/100;
					break;
				}
			}

			if (flag.lh)
			{
				for(i=0;i<sd->left_weapon.add_damage_class_count;i++) {
					if(sd->left_weapon.add_damage_classid[i] == t_class) {
						cardfix_=cardfix_*(100+sd->left_weapon.add_damage_classrate[i])/100;
						break;
					}
				}
			}

			if (cardfix != 1000 || cardfix_ != 1000)
				ATK_RATE2(cardfix/10, cardfix_/10);	//What happens if you use right-to-left and there's no right weapon, only left?
		}
	} //if (sd)

	//Card Fix, tsd side
	if (tsd && flag.cardfix) {
		short s_size,s_race2,s_mode,s_class;
		short cardfix=1000;
		
		s_size = status_get_size(src);
		s_race2 = status_get_race2(src);
		s_mode = status_get_mode(src);
		s_class = status_get_class(src);
		
		cardfix=cardfix*(100-tsd->subrace[s_race])/100;
		cardfix=cardfix*(100-tsd->subele[s_ele])/100;
		cardfix=cardfix*(100-tsd->subsize[s_size])/100;
 		cardfix=cardfix*(100-tsd->subrace2[s_race2])/100;
		cardfix=cardfix*(100-tsd->subrace[is_boss(src)?10:11])/100;
		
		for(i=0;i<tsd->add_damage_class_count2;i++) {
				if(tsd->add_damage_classid2[i] == s_class) {
					cardfix=cardfix*(100+tsd->add_damage_classrate2[i])/100;
					break;
				}
			}
	
		if(wd.flag&BF_SHORT)
			cardfix=cardfix*(100-tsd->near_attack_def_rate)/100;
		else	// BF_LONG (there's no other choice)
			cardfix=cardfix*(100-tsd->long_attack_def_rate)/100;

		if (cardfix != 1000)
			ATK_RATE(cardfix/10);
	}

	//SC_data fixes
	if (t_sc_data)
	{
		short scfix=1000;

		if(t_sc_data[SC_DEFENDER].timer != -1 && wd.flag&BF_LONG)
		{
			if(map[target->m].flag.gvg)
				scfix=scfix*(100-t_sc_data[SC_DEFENDER].val2/2)/100;
			else
				scfix=scfix*(100-t_sc_data[SC_DEFENDER].val2)/100;
		}
		if(t_sc_data[SC_FOGWALL].timer != -1 && wd.flag&BF_LONG)
			scfix=scfix*50/100;
		
		if(t_sc_data[SC_ASSUMPTIO].timer != -1 && !map[target->m].flag.gvg){
			if(!map[target->m].flag.pvp)
				scfix=scfix/3;
			else
				scfix=scfix*50/100;
		}
	
		if(scfix != 1000)
			ATK_RATE(scfix/10);
   }

	if(t_mode&0x40)
	{ //Plants receive 1 damage when hit
		if (flag.rh && (flag.hit || wd.damage>0))
			wd.damage = 1;
		if (flag.lh && (flag.hit || wd.damage2>0))
			wd.damage2 = 1;
		return wd;
	}
	
	if(!flag.rh || wd.damage<1)
		wd.damage=0;
	
	if(!flag.lh || wd.damage2<1)
		wd.damage2=0;
	
	if (sd)
	{
		if (!flag.rh && flag.lh) 
		{	//Move lh damage to the rh
			wd.damage = wd.damage2;
			wd.damage2 = 0;
			flag.rh=1;
			flag.lh=0;
		}

		if(sd->status.weapon > 16)
		{	//Dual-wield
			if (wd.damage > 0)
			{
				skill = pc_checkskill(sd,AS_RIGHT);
				wd.damage = wd.damage * (50 + (skill * 10))/100;
				if(wd.damage < 1) wd.damage = 1;
			}
			if (wd.damage2 > 0)
			{
				skill = pc_checkskill(sd,AS_LEFT);
				wd.damage2 = wd.damage2 * (30 + (skill * 10))/100;
				if(wd.damage2 < 1) wd.damage2 = 1;
			}
		} else if(sd->status.weapon == 16)
		{ //Katars
			skill = pc_checkskill(sd,TF_DOUBLE);
			wd.damage2 = wd.damage * (1 + (skill * 2))/100;

			if(wd.damage > 0 && wd.damage2 < 1) wd.damage2 = 1;
			flag.lh = 1;
		}
	}

	if(skill_num != CR_GRANDCROSS && (wd.damage > 0 || wd.damage2 > 0) )
	{
		if(wd.damage2<1)
			wd.damage=battle_calc_damage(src,target,wd.damage,wd.div_,skill_num,skill_lv,wd.flag);
		else if(wd.damage<1)
			wd.damage2=battle_calc_damage(src,target,wd.damage2,wd.div_,skill_num,skill_lv,wd.flag);
		else
		{
			int d1=wd.damage+wd.damage2,d2=wd.damage2;
			wd.damage=battle_calc_damage(src,target,d1,wd.div_,skill_num,skill_lv,wd.flag);
			wd.damage2=(d2*100/d1)*wd.damage/100;
			if(wd.damage > 1 && wd.damage2 < 1) wd.damage2=1;
			wd.damage-=wd.damage2;
		}
	}

	if(sd && sd->classchange && tmd && (rand()%10000 < sd->classchange))
	{	//Classchange:
		int changeclass[]={
			1001,1002,1004,1005,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1018,1019,1020,
			1021,1023,1024,1025,1026,1028,1029,1030,1031,1032,1033,1034,1035,1036,1037,1040,1041,
			1042,1044,1045,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1060,1061,
			1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,1076,1077,1078,1079,1080,1081,1083,
			1084,1085,1094,1095,1097,1099,1100,1101,1102,1103,1104,1105,1106,1107,1108,1109,1110,
			1111,1113,1114,1116,1117,1118,1119,1121,1122,1123,1124,1125,1126,1127,1128,1129,1130,
			1131,1132,1133,1134,1135,1138,1139,1140,1141,1142,1143,1144,1145,1146,1148,1149,1151,
			1152,1153,1154,1155,1156,1158,1160,1161,1163,1164,1165,1166,1167,1169,1170,1174,1175,
			1176,1177,1178,1179,1180,1182,1183,1184,1185,1188,1189,1191,1192,1193,1194,1195,1196,
			1197,1199,1200,1201,1202,1204,1205,1206,1207,1208,1209,1211,1212,1213,1214,1215,1216,
			1219,1242,1243,1245,1246,1247,1248,1249,1250,1253,1254,1255,1256,1257,1258,1260,1261,
			1263,1264,1265,1266,1267,1269,1270,1271,1273,1274,1275,1276,1277,1278,1280,1281,1282,
			1291,1292,1293,1294,1295,1297,1298,1300,1301,1302,1304,1305,1306,1308,1309,1310,1311,
			1313,1314,1315,1316,1317,1318,1319,1320,1321,1322,1323,1364,1365,1366,1367,1368,1369,
			1370,1371,1372,1374,1375,1376,1377,1378,1379,1380,1381,1382,1383,1384,1385,1386,1387,
			1390,1391,1392,1400,1401,1402,1403,1404,1405,1406,1408,1409,1410,1412,1413,1415,1416,
			1417,1493,1494,1495,1497,1498,1499,1500,1502,1503,1504,1505,1506,1507,1508,1509,1510,
			1511,1512,1513,1514,1515,1516,1517,1519,1520,1582,1584,1585,1586,1587 };
		mob_class_change(tmd, changeclass);
	}
	return wd;
}
/*==========================================
 * 武器ダメージ計算
 *------------------------------------------
 */
struct Damage battle_calc_weapon_attack(
	struct block_list *src,struct block_list *target,int skill_num,int skill_lv,int wflag)
{
	struct Damage wd;

	//return前の処理があるので情報出力部のみ変更
	if (src == NULL || target == NULL) {
		nullpo_info(NLP_MARK);
		memset(&wd,0,sizeof(wd));
		return wd;
	}

	//Until the function becomes official [Skotlex]
	if (battle_config.new_attack_function)
		wd = battle_calc_weapon_attack_sub(src,target,skill_num,skill_lv,wflag);
	else
	{
		if(target->type == BL_PET)
		memset(&wd,0,sizeof(wd));

	else if(src->type == BL_PC)
		wd = battle_calc_pc_weapon_attack(src,target,skill_num,skill_lv,wflag);
	else if(src->type == BL_MOB)
		wd = battle_calc_mob_weapon_attack(src,target,skill_num,skill_lv,wflag);
	else if(src->type == BL_PET)
		wd = battle_calc_pet_weapon_attack(src,target,skill_num,skill_lv,wflag);
	else
		memset(&wd,0,sizeof(wd));
	}
	if(battle_config.equipment_breaking && src->type==BL_PC && (wd.damage > 0 || wd.damage2 > 0)) {
		struct map_session_data *sd = (struct map_session_data *)src;
		// weapon = 0, armor = 1
		int breakrate = 0;	//0.01% default self weapon breaking chance [DracoRPG]
		int breakrate_[2] = {0,0};	//enemy breaking chance [celest]
		int breaktime = 5000;

		breakrate_[0] += sd->break_weapon_rate;
		breakrate_[1] += sd->break_armor_rate;

		if (sd->sc_count) {
			if (sd->sc_data[SC_MELTDOWN].timer!=-1 && (wd.flag & BF_SHORT)) {
				breakrate_[0] += 100*sd->sc_data[SC_MELTDOWN].val1;
				breakrate_[1] = 70*sd->sc_data[SC_MELTDOWN].val1;
				breaktime = skill_get_time2(WS_MELTDOWN,1);
			}
			if(sd->status.weapon < 6 && sd->status.weapon > 8)
			{
				if(sd->sc_data[SC_OVERTHRUST].timer!=-1)
					breakrate += 10;
				if(sd->sc_data[SC_MAXOVERTHRUST].timer!=-1)
					breakrate += 10;
			}
		}

		if(sd->status.weapon) {
			if(rand() % 10000 < breakrate * battle_config.equipment_break_rate / 100 || breakrate >= 10000)
				if (pc_breakweapon(sd) == 1)
				{
					if (battle_config.new_attack_function)
						wd = battle_calc_weapon_attack_sub(src,target,skill_num,skill_lv,wflag);
					else
						wd = battle_calc_pc_weapon_attack(src,target,skill_num,skill_lv,wflag);
				}
		}
		if(rand() % 10000 < breakrate_[0] * battle_config.equipment_break_rate / 100 || breakrate_[0] >= 10000) {
			if (target->type == BL_PC) {
				struct map_session_data *tsd = (struct map_session_data *)target;
				pc_breakweapon(tsd);
			} else
				status_change_start(target,SC_STRIPWEAPON,1,75,0,0,breaktime,0);
		}
		if(rand() % 10000 < breakrate_[1] * battle_config.equipment_break_rate/100 || breakrate_[1] >= 10000) {
			if (target->type == BL_PC) {
				struct map_session_data *tsd = (struct map_session_data *)target;
				pc_breakarmor(tsd);
			} else
				status_change_start(target,SC_STRIPSHIELD,1,75,0,0,breaktime,0);
		}
	}

	return wd;
}

/*==========================================
 * 魔法ダメージ計算
 *------------------------------------------
 */
struct Damage battle_calc_magic_attack(
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag)
	{
	int mdef1, mdef2, matk1, matk2, damage = 0, div_ = 1, blewcount, rdamage = 0;
	int ele=0, race=7, size=1, race2=7, t_ele=0, t_race=7, t_mode = 0, cardfix, t_class, i;
	struct map_session_data *sd = NULL, *tsd = NULL;
	struct mob_data *tmd = NULL;
	struct Damage md;
	int aflag;	
	int normalmagic_flag = 1;
	int matk_flag = 1;
	int no_cardfix = 0;
	int no_elefix = 0;
	int imdef_flag=0;

	//return前の処理があるので情報出力部のみ変更
	if( bl == NULL || target == NULL ){
		nullpo_info(NLP_MARK);
		memset(&md,0,sizeof(md));
		return md;
	}

	if(target->type == BL_PET) {
		memset(&md,0,sizeof(md));
		return md;
	}

	blewcount = skill_get_blewcount(skill_num,skill_lv);
	matk1=status_get_matk1(bl);
	matk2=status_get_matk2(bl);
	ele = skill_get_pl(skill_num);
	race = status_get_race(bl);
	size = status_get_size(bl);
	race2 = status_get_race2(bl);
	mdef1 = status_get_mdef(target);
	mdef2 = status_get_mdef2(target);
	t_ele = status_get_elem_type(target);
	t_race = status_get_race(target);
	t_mode = status_get_mode(target);

#define MATK_FIX( a,b ) { matk1=matk1*(a)/(b); matk2=matk2*(a)/(b); }

	if( bl->type==BL_PC && (sd=(struct map_session_data *)bl) ){
		sd->state.attack_type = BF_MAGIC;
		if(sd->matk_rate != 100)
			MATK_FIX(sd->matk_rate,100);
		sd->state.arrow_atk = 0;
	}
	if( target->type==BL_PC )
		tsd=(struct map_session_data *)target;
	else if( target->type==BL_MOB )
		tmd=(struct mob_data *)target;

	aflag=BF_MAGIC|BF_LONG|BF_SKILL;

	if(skill_num > 0){
		switch(skill_num){	// 基本ダメージ計算(スキルごとに処理)
					// ヒールor聖体
		case AL_HEAL:
		case PR_BENEDICTIO:
			damage = skill_calc_heal(bl,skill_lv)/2;
			normalmagic_flag=0;
			break;
		case PR_ASPERSIO:		/* アスペルシオ */
			damage = 40; //固定ダメージ
			normalmagic_flag=0;
			break;
		case PR_SANCTUARY:	// サンクチュアリ
			damage = (skill_lv>6)?388:skill_lv*50;
			normalmagic_flag=0;
			blewcount|=0x10000;
			break;
		case ALL_RESURRECTION:
		case PR_TURNUNDEAD:	// 攻撃リザレクションとターンアンデッド
			if(target->type != BL_PC && battle_check_undead(t_race,t_ele)){
				int hp, mhp, thres;
				hp = status_get_hp(target);
				mhp = status_get_max_hp(target);
				thres = (skill_lv * 20) + status_get_luk(bl)+
						status_get_int(bl) + status_get_lv(bl)+
						((200 - hp * 200 / mhp));
				if(thres > 700) thres = 700;
//				if(battle_config.battle_log)
//					printf("ターンアンデッド！ 確率%d ‰(千分率)\n", thres);
				if(rand()%1000 < thres && !(t_mode&0x20))	// 成功
				{
					damage = hp;
					if(tmd) tmd->exp_penalty = 1;
				}
				else					// 失敗
					damage = status_get_lv(bl) + status_get_int(bl) + skill_lv * 10;
			}
			normalmagic_flag=0;
			break;

		case MG_NAPALMBEAT:	// ナパームビート（分散計算込み）
			MATK_FIX(70+ skill_lv*10,100);
			if(flag>0){
				MATK_FIX(1,flag);
			}else {
				if(battle_config.error_log)
					printf("battle_calc_magic_attack(): napam enemy count=0 !\n");
			}
			break;

		case MG_SOULSTRIKE:			/* ソウルストライク （対アンデッドダメージ補正）*/
			if (battle_check_undead(t_race,t_ele)) {
				matk1 = matk1*(20+skill_lv)/20;//MATKに補正じゃ駄目ですかね？
				matk2 = matk2*(20+skill_lv)/20;
			}
			break;

		case MG_FIREBALL:	// ファイヤーボール
			{
				const int drate[]={100,90,70};
				if(flag>2)
					matk1=matk2=0;
				else
					MATK_FIX( (95+skill_lv*5)*drate[flag] ,10000 );
			}
			break;
		case MG_FIREWALL:	// ファイヤーウォール
/*
			if( (t_ele!=3 && !battle_check_undead(t_race,t_ele)) || target->type==BL_PC ) //PCは火属性でも飛ぶ？そもそもダメージ受ける？
				blewcount |= 0x10000;
			else
				blewcount = 0;
*/
			if((t_ele==3 || battle_check_undead(t_race,t_ele)) && target->type!=BL_PC)
				blewcount = 0;
			else
				blewcount |= 0x10000;
			MATK_FIX( 1,2 );
			break;
		case MG_THUNDERSTORM:	// サンダーストーム
			MATK_FIX( 80,100 );
			break;
		case MG_FROSTDIVER:	// フロストダイバ
			MATK_FIX( 100+skill_lv*10, 100);
			break;
		case WZ_FROSTNOVA:	// フロストダイバ
			MATK_FIX((100+skill_lv*10)*2/3, 100);
			break;
		case WZ_FIREPILLAR:	// ファイヤーピラー
			if(mdef1 < 1000000)
				mdef1=mdef2=0;	// MDEF無視
			MATK_FIX( 1,5 );
			matk1+=50;
			matk2+=50;
			break;
		case WZ_SIGHTRASHER:
			MATK_FIX( 100+skill_lv*20, 100);
			break;
		case WZ_METEOR:
		case WZ_JUPITEL:	// ユピテルサンダー
			break;
		case WZ_VERMILION:	// ロードオブバーミリオン
			MATK_FIX( skill_lv*20+80, 100 );
			break;
		case WZ_WATERBALL:	// ウォーターボール
			MATK_FIX( 100+skill_lv*30, 100 );
			break;
		case WZ_STORMGUST:	// ストームガスト
			MATK_FIX( skill_lv*40+100 ,100 );
//			blewcount|=0x10000;
			break;
		case AL_HOLYLIGHT:	// ホーリーライト
			MATK_FIX( 125,100 );
			break;
		case AL_RUWACH:
			MATK_FIX( 145,100 );
			break;
		case HW_NAPALMVULCAN:	// ナパームビート（分散計算込み）
			MATK_FIX(70+ skill_lv*10,100);
			if(flag>0){
				MATK_FIX(1,flag);
			}else {
				if(battle_config.error_log)
					printf("battle_calc_magic_attack(): napalmvulcan enemy count=0 !\n");
			}
			break;
		case PF_SOULBURN: // Celest
			if (target->type != BL_PC || skill_lv < 5) {
				memset(&md,0,sizeof(md));
				return md;
			} else if (target->type == BL_PC) {
				damage = ((struct map_session_data *)target)->status.sp * 2;
				matk_flag = 0; // don't consider matk and matk2
			}
			break;
		case ASC_BREAKER:
			damage = rand()%500 + 500 + skill_lv * status_get_int(bl) * 5;
			normalmagic_flag = 0;
			no_elefix = 1;
			break;
		case HW_GRAVITATION:
			// Now gives more hits per second with high skill level (Aru)
			damage = 200; // + skill_lv * 200;
			normalmagic_flag = 0;
			no_cardfix = 1;
			no_elefix = 1;
			break;
		}
	}

	if(normalmagic_flag){	// 一般魔法ダメージ計算
		if (matk_flag) {
			if(matk1>matk2)
				damage= matk2+rand()%(matk1-matk2+1);
			else
				damage= matk2;
		}
		if(sd) {
			if(sd->ignore_mdef_ele & (1<<t_ele) || sd->ignore_mdef_race & (1<<t_race))
				imdef_flag = 1;
			if(is_boss(target)) {
				if(sd->ignore_mdef_race & (1<<10))
					imdef_flag = 1;
			}
			else {
				if(sd->ignore_mdef_race & (1<<11))
					imdef_flag = 1;
			}
		}
		if(!imdef_flag){
			if(battle_config.magic_defense_type) {
				damage = damage - (mdef1 * battle_config.magic_defense_type) - mdef2;
			}
			else{
			damage = (damage*(100-mdef1))/100 - mdef2;
			}
		}

		if(damage<1)
			damage=1;
	}

	if (sd && !no_cardfix) {
		cardfix=100;
		cardfix=cardfix*(100+sd->magic_addrace[t_race])/100;
		cardfix=cardfix*(100+sd->magic_addele[ele])/100;
		if(is_boss(target))
			cardfix=cardfix*(100+sd->magic_addrace[10])/100;
		else
			cardfix=cardfix*(100+sd->magic_addrace[11])/100;
		t_class = status_get_class(target);
		for(i=0;i<sd->add_magic_damage_class_count;i++) {
			if(sd->add_magic_damage_classid[i] == t_class) {
				cardfix=cardfix*(100+sd->add_magic_damage_classrate[i])/100;
				break;
			}
		}
		damage=damage*cardfix/100;
//		if (skill_num > 0 && sd->skillatk[0] == skill_num)
		damage += damage*sd->skill_damage_bonus[skill_num]/100;
	}

	if (tsd && !no_cardfix) {
		int s_class = status_get_class(bl);
		cardfix=100;
		cardfix=cardfix*(100-tsd->subele[ele])/100;	// 属 性によるダメージ耐性
		cardfix=cardfix*(100-tsd->subrace[race])/100;	// 種族によるダメージ耐性
		cardfix=cardfix*(100-tsd->subsize[size])/100;
		cardfix=cardfix*(100-tsd->magic_subrace[race])/100;
		cardfix=cardfix*(100-tsd->subrace2[race2])/100;	// 種族によるダメージ耐性
		if(is_boss(target))
			cardfix=cardfix*(100-tsd->magic_subrace[10])/100;
		else
			cardfix=cardfix*(100-tsd->magic_subrace[11])/100;
		for(i=0;i<tsd->add_mdef_class_count;i++) {
			if(tsd->add_mdef_classid[i] == s_class) {
				cardfix=cardfix*(100-tsd->add_mdef_classrate[i])/100;
				break;
			}
		}
		cardfix=cardfix*(100-tsd->magic_def_rate)/100;
		damage=damage*cardfix/100;
	}
	if(damage < 0) damage = 0;

	if (!no_elefix)
		damage=battle_attr_fix(damage, ele, status_get_element(target) );		// 属 性修正

	if(skill_num == CR_GRANDCROSS) {	// グランドクロス
		struct Damage wd;
		wd=battle_calc_weapon_attack(bl,target,skill_num,skill_lv,flag);
		damage = (damage + wd.damage) * (100 + 40*skill_lv)/100;
		if(battle_config.gx_dupele) damage=battle_attr_fix(damage, ele, status_get_element(target) );	//属性2回かかる
		if(bl==target) damage=damage/2;	//反動は半分
	}

	div_=skill_get_num( skill_num,skill_lv );

	if(div_>1 && skill_num != WZ_VERMILION)
		damage*=div_;

//	if(mdef1 >= 1000000 && damage > 0)
	if(t_mode&0x40 && damage > 0)
		damage = 1;

	if(is_boss(target))
		blewcount = 0;

	if (tsd && status_isimmune(target)) {
		if (sd && battle_config.gtb_pvp_only != 0)  { // [MouseJstr]
			damage = (damage * (100 - battle_config.gtb_pvp_only)) / 100;
		} else damage = 0;	// 黄 金蟲カード（魔法ダメージ０）
	}

	damage=battle_calc_damage(bl,target,damage,div_,skill_num,skill_lv,aflag);	// 最終修正

	/* magic_damage_return by [AppleGirl] and [Valaris]		*/
	if( target->type==BL_PC && tsd && (rand()%100 < tsd->magic_damage_return)){
			rdamage += damage;
			if(rdamage < 1) rdamage = 1;
			clif_damage(target,bl,gettick(),0,0,rdamage,0,0,0);
			battle_damage(target,bl,rdamage,0);
	}
	/*			end magic_damage_return			*/

	md.damage=damage;
	md.div_=div_;
	md.amotion=status_get_amotion(bl);
	md.dmotion=status_get_dmotion(target);
	md.damage2=0;
	md.type=0;
	md.blewcount=blewcount;
	md.flag=aflag;

	return md;
}

/*==========================================
 * その他ダメージ計算
 *------------------------------------------
 */
struct Damage  battle_calc_misc_attack(
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag)
{
	int int_=status_get_int(bl);
//	int luk=status_get_luk(bl);
	int dex=status_get_dex(bl);
	int skill,ele,race,size,cardfix,race2,t_mode;
	struct map_session_data *sd=NULL,*tsd=NULL;
	int damage=0,div_=1,blewcount=skill_get_blewcount(skill_num,skill_lv);
	struct Damage md;
	int damagefix=1;
	int self_damage=0;	// for Sacrifice (Aru)

	int aflag=BF_MISC|BF_SHORT|BF_SKILL;

	//return前の処理があるので情報出力部のみ変更
	if( bl == NULL || target == NULL ){
		nullpo_info(NLP_MARK);
		memset(&md,0,sizeof(md));
		return md;
	}

	if(target->type == BL_PET) {
		memset(&md,0,sizeof(md));
		return md;
	}

	if( bl->type == BL_PC && (sd=(struct map_session_data *)bl) ) {
		sd->state.attack_type = BF_MISC;
		sd->state.arrow_atk = 0;
	}

	if( target->type==BL_PC )
		tsd=(struct map_session_data *)target;

	t_mode = status_get_mode(target);
	ele = skill_get_pl(skill_num);
	race = status_get_race(bl);
	size = status_get_size(bl);
	race2 = status_get_race(bl);

	switch(skill_num){

	case HT_LANDMINE:	// ランドマイン
		damage=skill_lv*(dex+75)*(100+int_)/100;
		break;

	case HT_BLASTMINE:	// ブラストマイン
		damage=skill_lv*(dex/2+50)*(100+int_)/100;
		break;

	case HT_CLAYMORETRAP:	// クレイモアートラップ
		damage=skill_lv*(dex/2+75)*(100+int_)/100;
		break;

	case HT_BLITZBEAT:	// ブリッツビート
		if( sd==NULL || (skill = pc_checkskill(sd,HT_STEELCROW)) <= 0)
			skill=0;
		damage=(dex/10+int_/2+skill*3+40)*2;
		if(flag > 1)
			damage /= flag;
		aflag |= (flag&~BF_RANGEMASK)|BF_LONG;
		break;

	case TF_THROWSTONE:	// 石投げ
		damage=50;
		damagefix=0;
		aflag |= (flag&~BF_RANGEMASK)|BF_LONG;
		break;

	case BA_DISSONANCE:	// 不協和音
		damage=(skill_lv)*20+pc_checkskill(sd,BA_MUSICALLESSON)*3;
		break;

	case NPC_SELFDESTRUCTION:	// 自爆
		damage = status_get_hp(bl);
		damagefix = 0;
		break;

	case NPC_SMOKING:	// タバコを吸う
		damage=3;
		damagefix=0;
		break;

	case NPC_DARKBREATH:
		{
			struct status_change *sc_data = status_get_sc_data(target);
			int hitrate=status_get_hit(bl) - status_get_flee(target) + 80;
			hitrate = ( (hitrate>95)?95: ((hitrate<5)?5:hitrate) );
			if(sc_data && (sc_data[SC_SLEEP].timer!=-1 || sc_data[SC_STAN].timer!=-1 ||
				sc_data[SC_FREEZE].timer!=-1 || (sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0) ) )
				hitrate = 1000000;
			if(rand()%100 < hitrate) {
				damage = 500 + (skill_lv-1)*1000 + rand()%1000;
				if(damage > 9999) damage = 9999;
			}
		}
		break;
	case SN_FALCONASSAULT:			/* ファルコンアサルト */
#ifdef TWILIGHT
		if( sd==NULL || (skill = pc_checkskill(sd,HT_BLITZBEAT)) <= 0)
			skill=0;
 		damage=(100+50*skill_lv+(dex/10+int_/2+skill*3+40)*2) * 2;
#else
		if( sd==NULL || (skill = pc_checkskill(sd,HT_STEELCROW)) <= 0)
			skill=0;
		damage=((150+50*skill_lv)*(dex/10+int_/2+skill*3+40)*2)/100; // [Celest]
#endif
		if(flag > 1)
			damage /= flag;
		aflag |= (flag&~BF_RANGEMASK)|BF_LONG;
		break;
	case PA_PRESSURE:
		damage = 500+300*skill_lv;
		damagefix = 0;
		break;
	case PA_SACRIFICE:
		self_damage = status_get_max_hp(bl)/10;
		self_damage -= self_damage/10;
		if(status_get_mexp(target))
			self_damage = 1;
		damage = self_damage + (self_damage/10)*(skill_lv-1);
		// Add attacker's weapon cards.
		cardfix = 100;
		cardfix=cardfix*(100+sd->right_weapon.addrace[status_get_race(target)])/100;	// 種族によるダメージ修正
		cardfix=cardfix*(100+sd->right_weapon.addele[status_get_element(target)])/100;	// 属性によるダメージ修正
		cardfix=cardfix*(100+sd->right_weapon.addsize[status_get_size(target)])/100;	// サイズによるダメージ修正
		cardfix=cardfix*(100+sd->right_weapon.addrace2[status_get_race2(target)])/100;
		damage=damage*cardfix/100;
		break;
	case CR_ACIDDEMONSTRATION:
		{
			float fdamage;
			fdamage = status_get_int(bl)*0.28;
			fdamage *= (1+status_get_vit(target))*1.4;
			damage = (int)fdamage;
			if(target->type == BL_PC) damage/=2;
		}
		aflag |= (flag&~BF_RANGEMASK)|BF_LONG;
		break;
	}

	if(damagefix){
		if(damage<1 && skill_num != NPC_DARKBREATH)
			damage=1;

		if( tsd ){
			cardfix=100;
			cardfix=cardfix*(100-tsd->subele[ele])/100;	// 属性によるダメージ耐性
			cardfix=cardfix*(100-tsd->subrace[race])/100;	// 種族によるダメージ耐性
			cardfix=cardfix*(100-tsd->subsize[size])/100;
			cardfix=cardfix*(100-tsd->misc_def_rate)/100;
			cardfix=cardfix*(100-tsd->subrace2[race2])/100;
			damage=damage*cardfix/100;
		}
		if (sd)
			damage += damage*sd->skill_damage_bonus[skill_num]/100;

		if(damage < 0) damage = 0;
		damage=battle_attr_fix(damage, ele, status_get_element(target) );		// 属性修正
	}

	div_=skill_get_num( skill_num,skill_lv );
	if(div_>1)
		damage*=div_;

	if(damage > 0 && (damage < div_ || (status_get_def(target) >= 1000000 && status_get_mdef(target) >= 1000000) ) ) {
		damage = div_;
	}

	if(t_mode&0x40 && damage>0)
		damage = 1;

	if(is_boss(target))
		blewcount = 0;

	damage=battle_calc_damage(bl,target,damage,div_,skill_num,skill_lv,aflag);	// 最終修正

	if(self_damage)
	{
		pc_damage(bl,sd,self_damage);
		clif_damage(bl,bl, gettick(), 0, 0, self_damage, 0 , 0, 0);
//		clif_skill_damage(bl, bl, gettick(), 0, 0, self_damage, PA_SACRIFICE, 1, 1, 9);
	}

	md.damage=damage;
	md.div_=div_;
	md.amotion=status_get_amotion(bl);
	md.dmotion=status_get_dmotion(target);
	md.damage2=0;
	md.type=0;
	md.blewcount=blewcount;
	md.flag=aflag;
	return md;

}
/*==========================================
 * ダメージ計算一括処理用
 *------------------------------------------
 */
struct Damage battle_calc_attack(	int attack_type,
	struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag)
{
	struct Damage d;

	switch(attack_type){
	case BF_WEAPON:
		return battle_calc_weapon_attack(bl,target,skill_num,skill_lv,flag);
	case BF_MAGIC:
		return battle_calc_magic_attack(bl,target,skill_num,skill_lv,flag);
	case BF_MISC:
		return battle_calc_misc_attack(bl,target,skill_num,skill_lv,flag);
	default:
		if(battle_config.error_log)
			printf("battle_calc_attack: unknwon attack type ! %d\n",attack_type);
		memset(&d,0,sizeof(d));
		break;
	}
	return d;
}
/*==========================================
 * 通常攻撃処理まとめ
 *------------------------------------------
 */
int battle_weapon_attack( struct block_list *src,struct block_list *target,
	 unsigned int tick,int flag)
{
	struct map_session_data *sd = NULL, *tsd = NULL;
	struct status_change *sc_data, *tsc_data;
	int race, ele, damage, rdamage = 0;
	struct Damage wd;
	short *opt1;

	nullpo_retr(0, src);
	nullpo_retr(0, target);

	if (src->prev == NULL || target->prev == NULL)
		return 0;
	if(src->type == BL_PC)
		sd = (struct map_session_data *)src;
	if (sd && pc_isdead(sd))
		return 0;

	if (target->type == BL_PC)
		tsd = (struct map_session_data *)target;
	if (tsd && pc_isdead(tsd))
		return 0;

	opt1 = status_get_opt1(src);
	if (opt1 && *opt1 > 0) {
		battle_stopattack(src);
		return 0;
	}

	sc_data = status_get_sc_data(src);
	tsc_data = status_get_sc_data(target);

	if (sc_data && sc_data[SC_BLADESTOP].timer != -1) {
		battle_stopattack(src);
		return 0;
	}

	if (battle_check_target(src,target,BCT_ENEMY) <= 0 && !battle_check_range(src,target,0))
		return 0;	// 攻撃対象外

	race = status_get_race(target);
	ele = status_get_elem_type(target);
	if (battle_check_target(src,target,BCT_ENEMY) > 0 && battle_check_range(src,target,0)) {
		// 攻撃対象となりうるので攻撃
		if(sd && sd->status.weapon == 11) {
			if(sd->equip_index[10] >= 0) {
				if(battle_config.arrow_decrement)
					pc_delitem(sd,sd->equip_index[10],1,0);
			}
			else {
				clif_arrow_fail(sd,0);
				return 0;
			}
		}
		if(flag&0x8000) {
			if(sd && battle_config.pc_attack_direction_change)
				sd->dir = sd->head_dir = map_calc_dir(src, target->x,target->y );
			else if (src->type == BL_MOB && battle_config.monster_attack_direction_change) {
				struct mob_data *md = (struct mob_data *)src;
				if (md) md->dir = map_calc_dir(src, target->x, target->y);
			}
			wd = battle_calc_weapon_attack(src, target, KN_AUTOCOUNTER, flag&0xff, 0);
		}
		else if (flag & AS_POISONREACT && sc_data && sc_data[SC_POISONREACT].timer != -1)
			wd = battle_calc_weapon_attack(src, target, AS_POISONREACT, sc_data[SC_POISONREACT].val1, 0);
		else
			wd = battle_calc_weapon_attack(src,target,0,0,0);

		if (sc_data && sc_data[SC_SACRIFICE].timer != -1)
		{
			skill_castend_damage_id(src, target, PA_SACRIFICE, sc_data[SC_SACRIFICE].val1, tick, flag);
			sc_data[SC_SACRIFICE].val2--;			
			if(sc_data[SC_SACRIFICE].val2 < 1)
				status_change_end(src, SC_SACRIFICE, -1);
		}			

		if(sc_data && sc_data[SC_BERSERK].timer!=-1)
		{
			wd.damage *= 2;
			wd.damage2 *= 2;
		}

		if ((damage = wd.damage + wd.damage2) > 0 && src != target) {
			if(battle_config.pet_attack_support && sd && sd->status.pet_id > 0 && sd->pd && sd->petDB)
				pet_target_check(sd,target,0);

			if (wd.flag & BF_SHORT) {
				if (tsd && (rand()%100 < tsd->short_weapon_damage_return)) {
					rdamage += damage;
					if (rdamage < 1) rdamage = 1;
				}
				if (tsc_data && tsc_data[SC_REFLECTSHIELD].timer != -1) {
					rdamage += damage * tsc_data[SC_REFLECTSHIELD].val2 / 100;
					if (rdamage < 1) rdamage = 1;
				}
			} else if (wd.flag & BF_LONG) {
				if (tsd && (rand()%100 < tsd->long_weapon_damage_return)) {
					rdamage += damage;
					if(rdamage < 1) rdamage = 1;
				}
			}
			if (rdamage > 0)
				clif_damage(src, src, tick, wd.amotion, wd.dmotion, rdamage, 1, 4, 0);
		}

		if (wd.div_ == 255 && sd)	{ //三段掌
			int delay = 1000 - 4 * status_get_agi(src) - 2 *  status_get_dex(src);
			if (wd.damage + wd.damage2 < status_get_hp(target)) {
				int skilllv = pc_checkskill(sd, MO_CHAINCOMBO);
				if (skilllv > 0)
					delay += 300 * battle_config.combo_delay_rate / 100; //追加ディレイをconfにより調整
				status_change_start(src, SC_COMBO, MO_TRIPLEATTACK, skilllv, 0, 0, delay, 0);
			}
			sd->attackabletime = sd->canmove_tick = tick + delay;
			clif_combo_delay(src, delay);
			clif_skill_damage(src, target, tick, wd.amotion, wd.dmotion, wd.damage, 3,
				MO_TRIPLEATTACK, pc_checkskill(sd,MO_TRIPLEATTACK), -1);
			if(tsd)
				skill_check_plag(tsd,MO_TRIPLEATTACK, pc_checkskill(sd,MO_TRIPLEATTACK));
		} else {
			clif_damage(src, target, tick, wd.amotion, wd.dmotion, wd.damage, wd.div_ , wd.type, wd.damage2);
			//二刀流左手とカタール追撃のミス表示(無理やり～)
			if(sd && sd->status.weapon >= 16 && wd.damage2 == 0)
				clif_damage(src, target, tick+10, wd.amotion, wd.dmotion,0, 1, 0, 0);
		}
		if (sd && sd->splash_range > 0 && (wd.damage > 0 || wd.damage2 > 0))
			skill_castend_damage_id(src, target, 0, -1, tick, 0);

		map_freeblock_lock();

		battle_delay_damage(tick+wd.amotion, src, target, (wd.damage+wd.damage2), 0);

		if(wd.type == 4 && tsd!=NULL)
			tsd->endure_tick = gettick();

		if (wd.damage > 0 || wd.damage2 > 0) //Added counter effect [Skotlex]
			skill_counter_additional_effect(src, target, 0, 1, BF_WEAPON, tick);

		if (target->prev)
			skill_card_effect(src,target,tick);

		if (target->prev != NULL && (wd.damage > 0 || wd.damage2 > 0)) {
			skill_additional_effect(src, target, 0, 0, BF_WEAPON, tick);
			if (sd) {
				int hp = status_get_max_hp(target);
				if (sd->weapon_coma_ele[ele] > 0 && rand()%10000 < sd->weapon_coma_ele[ele])
					battle_damage(src, target, hp, 1);
				if (sd->weapon_coma_race[race] > 0 && rand()%10000 < sd->weapon_coma_race[race])
					battle_damage(src, target, hp, 1);
				if (is_boss(target)) {
					if(sd->weapon_coma_race[10] > 0 && rand()%10000 < sd->weapon_coma_race[10])
						battle_damage(src, target, hp, 1);
				} else {
					if (sd->weapon_coma_race[11] > 0 && rand()%10000 < sd->weapon_coma_race[11])
						battle_damage(src, target, hp, 1);
				}
			}
		}

		if (sc_data && sc_data[SC_AUTOSPELL].timer != -1 && rand()%100 < sc_data[SC_AUTOSPELL].val4) {
			int sp = 0, f = 0;
			int skillid = sc_data[SC_AUTOSPELL].val2;
			int skilllv = sc_data[SC_AUTOSPELL].val3;

			int i = rand()%100;
			if (i >= 66) skilllv -= 2;
			else if (i >= 33) skilllv--;
			if (skilllv < 1) skilllv = 1;

			if (sd) sp = skill_get_sp(skillid,skilllv) * 2 / 3;

			sd->state.autocast = 1;
			if ((sd && sd->status.sp >= sp) || !sd) {
				if ((i = skill_get_inf(skillid) == 2) || i == 32)
					f = skill_castend_pos2(src, target->x, target->y, skillid, skilllv, tick, flag);
				else {
					switch(skill_get_nk(skillid)) {
						case 0:	case 2:
							f = skill_castend_damage_id(src, target, skillid, skilllv, tick, flag);
							break;
						case 1:/* 支援系 */
							if((skillid == AL_HEAL || (skillid == ALL_RESURRECTION && !tsd)) && battle_check_undead(race,ele))
								f = skill_castend_damage_id(src, target, skillid, skilllv, tick, flag);
							else
								f = skill_castend_nodamage_id(src, target, skillid, skilllv, tick, flag);
							break;
					}
				}
				if (sd && !f) { pc_heal(sd, 0, -sp); }
			}
			sd->state.autocast = 0;
		}
		if (sd) {
			int i;
			if (wd.flag & BF_WEAPON && src != target && (wd.damage > 0 || wd.damage2 > 0)) {
				int hp = 0, sp = 0;
				if (!battle_config.left_cardfix_to_right) { // 二刀流左手カードの吸収系効果を右手に追加しない場合
					hp += battle_calc_drain(wd.damage, sd->right_weapon.hp_drain_rate, sd->right_weapon.hp_drain_per, sd->right_weapon.hp_drain_value);
					hp += battle_calc_drain(wd.damage2, sd->left_weapon.hp_drain_rate, sd->left_weapon.hp_drain_per, sd->left_weapon.hp_drain_value);
					sp += battle_calc_drain(wd.damage, sd->right_weapon.sp_drain_rate, sd->right_weapon.sp_drain_per, sd->right_weapon.sp_drain_value);
					sp += battle_calc_drain(wd.damage2, sd->left_weapon.sp_drain_rate, sd->left_weapon.sp_drain_per, sd->left_weapon.sp_drain_value);
				} else { // 二刀流左手カードの吸収系効果を右手に追加する場合
					int hp_drain_rate = sd->right_weapon.hp_drain_rate + sd->left_weapon.hp_drain_rate;
					int hp_drain_per = sd->right_weapon.hp_drain_per + sd->left_weapon.hp_drain_per;
					int hp_drain_value = sd->right_weapon.hp_drain_value + sd->left_weapon.hp_drain_value;
					int sp_drain_rate = sd->right_weapon.sp_drain_rate + sd->left_weapon.sp_drain_rate;
					int sp_drain_per = sd->right_weapon.sp_drain_per + sd->left_weapon.sp_drain_per;
					int sp_drain_value = sd->right_weapon.sp_drain_value + sd->left_weapon.sp_drain_value;
					hp += battle_calc_drain(wd.damage, hp_drain_rate, hp_drain_per, hp_drain_value);
					sp += battle_calc_drain(wd.damage, sp_drain_rate, sp_drain_per, sp_drain_value);
				}

				if (hp || sp) pc_heal(sd, hp, sp);
				if (tsd && sd->sp_drain_type)
					pc_heal(tsd, 0, -sp);
			}
		}
		if (rdamage > 0)
			battle_delay_damage(tick+wd.amotion, target, src, rdamage, 0);

		if (tsc_data) {
			if (tsc_data[SC_AUTOCOUNTER].timer != -1 && tsc_data[SC_AUTOCOUNTER].val4 > 0) {
				if (tsc_data[SC_AUTOCOUNTER].val3 == src->id)
					battle_weapon_attack(target, src, tick, 0x8000|tsc_data[SC_AUTOCOUNTER].val1);
				status_change_end(target,SC_AUTOCOUNTER,-1);
			}
			if (tsc_data[SC_POISONREACT].timer != -1 && tsc_data[SC_POISONREACT].val4 > 0 && tsc_data[SC_POISONREACT].val3 == src->id) {   // poison react [Celest]
				if (status_get_elem_type(src) == 5) {
					tsc_data[SC_POISONREACT].val2 = 0;
					battle_weapon_attack(target, src, tick, flag|AS_POISONREACT);
				} else {
					skill_castend_damage_id(target, src, TF_POISON, 5, tick, flag);
					--tsc_data[SC_POISONREACT].val2;
				}
				if (tsc_data[SC_POISONREACT].val2 <= 0)
					status_change_end(target,SC_POISONREACT,-1);
			}
			if (tsc_data[SC_BLADESTOP_WAIT].timer != -1 && !is_boss(target)) { // ボスには無効
				int skilllv = tsc_data[SC_BLADESTOP_WAIT].val1;
				status_change_end(target, SC_BLADESTOP_WAIT, -1);
				status_change_start(src, SC_BLADESTOP, skilllv, 1, (int)src, (int)target, skill_get_time2(MO_BLADESTOP,skilllv), 0);
				status_change_start(target, SC_BLADESTOP, skilllv, 2, (int)target, (int)src, skill_get_time2(MO_BLADESTOP,skilllv), 0);
			}
		}

		map_freeblock_unlock();
	}
	return wd.dmg_lv;
}

int battle_check_undead(int race,int element)
{
	if(battle_config.undead_detect_type == 0) {
		if(element == 9)
			return 1;
	}
	else if(battle_config.undead_detect_type == 1) {
		if(race == 1)
			return 1;
	}
	else {
		if(element == 9 || race == 1)
			return 1;
	}
	return 0;
}

/*==========================================
 * 敵味方判定(1=肯定,0=否定,-1=エラー)
 * flag&0xf0000 = 0x00000:敵じゃないか判定（ret:1＝敵ではない）
 *				= 0x10000:パーティー判定（ret:1=パーティーメンバ)
 *				= 0x20000:全て(ret:1=敵味方両方)
 *				= 0x40000:敵か判定(ret:1=敵)
 *				= 0x50000:パーティーじゃないか判定(ret:1=パーティでない)
 *------------------------------------------
 */
int battle_check_target( struct block_list *src, struct block_list *target,int flag)
{
	int s_p,s_g,t_p,t_g;
	struct block_list *ss=src;
	struct status_change *sc_data;
	struct status_change *tsc_data;
	struct map_session_data *srcsd = NULL;
	struct map_session_data *tsd = NULL;

	nullpo_retr(0, src);
	nullpo_retr(0, target);

	if (flag & BCT_ENEMY){	// 反転フラグ
		int ret = battle_check_target(src,target,flag&0x30000);
		if (ret != -1)
			return !ret;
		return -1;
	}

	if (flag & BCT_ALL){
		if (target->type == BL_MOB || target->type == BL_PC)
			return 1;
		else
			return -1;
	}

	if (src->type == BL_SKILL && target->type == BL_SKILL)	// 対象がスキルユニットなら無条件肯定
		return -1;

	if (target->type == BL_PET)
		return -1;

	if (src->type == BL_PC) {
		nullpo_retr(-1, srcsd = (struct map_session_data *)src);
	}
	if (target->type == BL_PC) {
		nullpo_retr(-1, tsd = (struct map_session_data *)target);
	}
	
	if(tsd && (tsd->invincible_timer != -1 || pc_isinvisible(tsd)))
		return -1;

	// Celest
	sc_data = status_get_sc_data(src);
	tsc_data = status_get_sc_data(target);
	if ((sc_data && sc_data[SC_BASILICA].timer != -1) ||
		(tsc_data && tsc_data[SC_BASILICA].timer != -1))
		return -1;

	if (target->type == BL_SKILL) {
		struct skill_unit *tsu = (struct skill_unit *)target;
		if (tsu && tsu->group) {
			switch (tsu->group->unit_id) {
			case 0x8d:
			case 0x8f:
			case 0x98:
				return 0;
				break;
			}
		}
	}

	// スキルユニットの場合、親を求める
	if (src->type == BL_SKILL) {
		struct skill_unit *su = (struct skill_unit *)src;
		if (su && su->group) {
			int skillid, inf2;		
			skillid = su->group->skill_id;
			inf2 = skill_get_inf2(skillid);
			if ((ss = map_id2bl(su->group->src_id)) == NULL)
				return -1;
			if (ss->prev == NULL)
				return -1;
			if (inf2&0x80 &&
				(map[src->m].flag.pvp ||
				(skillid >= 115 && skillid <= 125 && map[src->m].flag.gvg)) &&
				!(target->type == BL_PC && pc_isinvisible(tsd)))
					return 0;
			if (ss == target) {
				if (inf2&0x100)
					return 0;
				if (inf2&0x200)
					return -1;
			}
		}
	}
	
	if (src->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)src;
		nullpo_retr (-1, md);

		if (tsd) {
			if(md->class_ >= 1285 && md->class_ <= 1287){
				struct guild_castle *gc = guild_mapname2gc (map[target->m].name);
				if(gc && agit_flag==0)	// Guardians will not attack during non-woe time [Valaris]
					return 1;  // end addition [Valaris]
				if(gc && tsd->status.guild_id > 0) {
					struct guild *g=guild_search(tsd->status.guild_id);	// don't attack guild members [Valaris]
					if(g && g->guild_id == gc->guild_id)
						return 1;
					if(g && guild_isallied(g,gc))
						return 1;
				}
			}
			// option to have monsters ignore GMs [Valaris]
			if (battle_config.monsters_ignore_gm > 0 && pc_isGM(tsd) >= battle_config.monsters_ignore_gm)
				return 1;
		}
		// Mobでmaster_idがあってspecial_mob_aiなら、召喚主を求める
		if (md->master_id > 0) {
			if (md->master_id == target->id)	// 主なら肯定
				return 1;
			if (md->state.special_mob_ai){
				if (target->type == BL_MOB){	//special_mob_aiで対象がMob
					struct mob_data *tmd = (struct mob_data *)target;
					if (tmd){
						if(tmd->master_id != md->master_id)	//召喚主が一緒でなければ否定
							return 0;
						else{	//召喚主が一緒なので肯定したいけど自爆は否定
							if(md->state.special_mob_ai>2)
								return 0;
							else
								return 1;
						}
					}
				}
			}
			if((ss = map_id2bl(md->master_id)) == NULL)
				return -1;
		}
	}

	if (src == target || ss == target)	// 同じなら肯定
		return 1;

	if (src->prev == NULL ||	// 死んでるならエラー
		(srcsd && pc_isdead(srcsd)))
		return -1;

	if ((ss->type == BL_PC && target->type == BL_MOB) ||
		(ss->type == BL_MOB && target->type == BL_PC) )
		return 0;	// PCvsMOBなら否定

	if (ss->type == BL_PET && target->type == BL_MOB)
		return 0;

	s_p = status_get_party_id(ss);
	s_g = status_get_guild_id(ss);

	t_p = status_get_party_id(target);
	t_g = status_get_guild_id(target);

	if (flag & 0x10000) {
		if (s_p && t_p && s_p == t_p)	// 同じパーティなら肯定（味方）
			return 1;
		else		// パーティ検索なら同じパーティじゃない時点で否定
			return 0;
	}

	if (ss->type == BL_MOB && s_g > 0 && t_g > 0 && s_g == t_g )	// 同じギルド/mobクラスなら肯定（味方）
		return 1;

//printf("ss:%d src:%d target:%d flag:0x%x %d %d ",ss->id,src->id,target->id,flag,src->type,target->type);
//printf("p:%d %d g:%d %d\n",s_p,t_p,s_g,t_g);

	if (ss->type == BL_PC && target->type == BL_PC) { // 両方PVPモードなら否定（敵）
		struct map_session_data *ssd = (struct map_session_data *)ss;		
		struct skill_unit *su = NULL;
		if (src->type == BL_SKILL)
			su = (struct skill_unit *)src;
		if (map[ss->m].flag.pvp || pc_iskiller(ssd, tsd)) { // [MouseJstr]
			if(su && su->group->target_flag == BCT_NOENEMY)
				return 1;
			else if (map[src->m].flag.pkmode &&
				(ssd->status.class_ == 0 || tsd->status.class_ == 0 ||
				ssd->status.base_level < battle_config.pk_min_level ||
				tsd->status.base_level < battle_config.pk_min_level))
				return 1; // prevent novice engagement in pk_mode [Valaris]
			else if (map[ss->m].flag.pvp_noparty && s_p > 0 && t_p > 0 && s_p == t_p)
				return 1;
			else if (map[ss->m].flag.pvp_noguild && s_g > 0 && t_g > 0 && s_g == t_g)
				return 1;
			return 0;
		}
		if (map[src->m].flag.gvg || map[src->m].flag.gvg_dungeon) {
			struct guild *g;
			if (su && su->group->target_flag == BCT_NOENEMY)
				return 1;
			if (s_g > 0 && s_g == t_g)
				return 1;
			if (map[src->m].flag.gvg_noparty && s_p > 0 && t_p > 0 && s_p == t_p)
				return 1;
			if ((g = guild_search(s_g))) {
				int i;
				for (i = 0; i < MAX_GUILDALLIANCE; i++) {
					if (g->alliance[i].guild_id > 0 && g->alliance[i].guild_id == t_g) {
						if (g->alliance[i].opposition)
							return 0;//敵対ギルドなら無条件に敵
						else
							return 1;//同盟ギルドなら無条件に味方
					}
				}
			}
			return 0;
		}
	}

	return 1;	// 該当しないので無関係人物（まあ敵じゃないので味方）
}
/*==========================================
 * 射程判定
 *------------------------------------------
 */
int battle_check_range(struct block_list *src,struct block_list *bl,int range)
{

	int dx,dy;
	int arange;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);

	dx=abs(bl->x-src->x);
	dy=abs(bl->y-src->y);
	arange=((dx>dy)?dx:dy);

	if(src->m != bl->m)	// 違うマップ
		return 0;

	if( range>0 && range < arange )	// 遠すぎる
		return 0;

	if( arange<2 )	// 同じマスか隣接
		return 1;

//	if(bl->type == BL_SKILL && ((struct skill_unit *)bl)->group->unit_id == 0x8d)
//		return 1;

	// 障害物判定
	return path_search_long(NULL,src->m,src->x,src->y,bl->x,bl->y);
}

/*==========================================
 * Return numerical value of a switch configuration (modified by [Yor])
 * on/off, english, fran軋is, deutsch, espal
 *------------------------------------------
 */
int battle_config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0)
		return 0;
	return atoi(str);
}

static const struct battle_data_short {
	const char *str;
	short *val;
} battle_data_short[] = {	//List here battle_athena options which are type short!
	{ "warp_point_debug",                  &battle_config.warp_point_debug			},
	{ "enemy_critical",                    &battle_config.enemy_critical			},
	{ "enemy_critical_rate",               &battle_config.enemy_critical_rate		},
	{ "enemy_str",                         &battle_config.enemy_str				},
	{ "enemy_perfect_flee",                &battle_config.enemy_perfect_flee		},
	{ "casting_rate",                      &battle_config.cast_rate				},
	{ "delay_rate",                        &battle_config.delay_rate				},
	{ "delay_dependon_dex",                &battle_config.delay_dependon_dex		},
	{ "skill_delay_attack_enable",         &battle_config.sdelay_attack_enable		},
	{ "left_cardfix_to_right",             &battle_config.left_cardfix_to_right	},
	{ "player_skill_add_range",            &battle_config.pc_skill_add_range		},
	{ "skill_out_range_consume",           &battle_config.skill_out_range_consume	},
	{ "monster_skill_add_range",           &battle_config.mob_skill_add_range		},
	{ "player_damage_delay",               &battle_config.pc_damage_delay			},
	{ "player_damage_delay_rate",          &battle_config.pc_damage_delay_rate		},
	{ "defunit_not_enemy",                 &battle_config.defnotenemy				},
	{ "random_monster_checklv",            &battle_config.random_monster_checklv	},
	{ "attribute_recover",                 &battle_config.attr_recover				},
	{ "flooritem_lifetime",                &battle_config.flooritem_lifetime		},
	{ "item_auto_get",                     &battle_config.item_auto_get			},
//	{ "item_rate",	                        &battle_config.item_rate				}, //unused [Skotlex]	
	{ "drop_rate0item",                    &battle_config.drop_rate0item			},
	{ "pvp_exp",                           &battle_config.pvp_exp		},
	{ "gtb_pvp_only",                      &battle_config.gtb_pvp_only		},
	{ "guild_max_castles",                 &battle_config.guild_max_castles		},
	{ "death_penalty_type",                &battle_config.death_penalty_type		},
	{ "death_penalty_base",                &battle_config.death_penalty_base		},
	{ "death_penalty_job",                 &battle_config.death_penalty_job		},
	{ "restart_hp_rate",                   &battle_config.restart_hp_rate			},
	{ "restart_sp_rate",                   &battle_config.restart_sp_rate			},
	{ "mvp_hp_rate",                       &battle_config.mvp_hp_rate				},
	{ "monster_hp_rate",                   &battle_config.monster_hp_rate			},
	{ "monster_max_aspd",                  &battle_config.monster_max_aspd			},
	{ "atcommand_gm_only",                 &battle_config.atc_gmonly				},
	{ "atcommand_spawn_quantity_limit",    &battle_config.atc_spawn_quantity_limit	},
	{ "gm_all_skill",                      &battle_config.gm_allskill				},
	{ "gm_all_skill_add_abra",	            &battle_config.gm_allskill_addabra		},
	{ "gm_all_equipment",                  &battle_config.gm_allequip				},
	{ "gm_skill_unconditional",            &battle_config.gm_skilluncond			},
	{ "player_skillfree",                  &battle_config.skillfree				},
	{ "player_skillup_limit",              &battle_config.skillup_limit			},
	{ "weapon_produce_rate",               &battle_config.wp_rate					},
	{ "potion_produce_rate",               &battle_config.pp_rate					},
	{ "monster_active_enable",             &battle_config.monster_active_enable	},
	{ "monster_damage_delay_rate",         &battle_config.monster_damage_delay_rate},
	{ "monster_loot_type",                 &battle_config.monster_loot_type		},
//	{ "mob_skill_use",                     &battle_config.mob_skill_use			},	//Deprecated
	{ "mob_skill_rate",                    &battle_config.mob_skill_rate			},
	{ "mob_skill_delay",                   &battle_config.mob_skill_delay			},
	{ "mob_count_rate",                    &battle_config.mob_count_rate			},
	{ "mob_spawn_delay",                   &battle_config.mob_spawn_delay			},
	{ "plant_spawn_delay",                 &battle_config.plant_spawn_delay			},
	{ "boss_spawn_delay",                  &battle_config.boss_spawn_delay			},
	{ "quest_skill_learn",                 &battle_config.quest_skill_learn		},
	{ "quest_skill_reset",                 &battle_config.quest_skill_reset		},
	{ "basic_skill_check",                 &battle_config.basic_skill_check		},
	{ "guild_emperium_check",              &battle_config.guild_emperium_check		},
	{ "guild_exp_limit",                   &battle_config.guild_exp_limit			},
	{ "player_invincible_time",            &battle_config.pc_invincible_time		},
	{ "pet_catch_rate",                    &battle_config.pet_catch_rate			},
	{ "pet_rename",                        &battle_config.pet_rename				},
	{ "pet_friendly_rate",                 &battle_config.pet_friendly_rate		},
	{ "pet_hungry_delay_rate",             &battle_config.pet_hungry_delay_rate	},
	{ "pet_hungry_friendly_decrease",      &battle_config.pet_hungry_friendly_decrease},
	{ "pet_str",                           &battle_config.pet_str					},
	{ "pet_status_support",                &battle_config.pet_status_support		},
	{ "pet_attack_support",                &battle_config.pet_attack_support		},
	{ "pet_damage_support",                &battle_config.pet_damage_support		},
	{ "pet_support_rate",                  &battle_config.pet_support_rate			},
	{ "pet_attack_exp_to_master",          &battle_config.pet_attack_exp_to_master	},
	{ "pet_attack_exp_rate",               &battle_config.pet_attack_exp_rate	 },
	{ "pet_lv_rate",                       &battle_config.pet_lv_rate				},	//Skotlex
	{ "pet_max_stats",                     &battle_config.pet_max_stats				},	//Skotlex
	{ "pet_max_atk1",                      &battle_config.pet_max_atk1				},	//Skotlex
	{ "pet_max_atk2",                      &battle_config.pet_max_atk2				},	//Skotlex
	{ "skill_min_damage",                  &battle_config.skill_min_damage			},
	{ "finger_offensive_type",             &battle_config.finger_offensive_type	},
	{ "heal_exp",                          &battle_config.heal_exp					},
	{ "resurrection_exp",                  &battle_config.resurrection_exp			},
	{ "shop_exp",                          &battle_config.shop_exp					},
	{ "combo_delay_rate",                  &battle_config.combo_delay_rate			},
	{ "item_check",                        &battle_config.item_check				},
	{ "wedding_modifydisplay",             &battle_config.wedding_modifydisplay	},
	{ "wedding_ignorepalette",             &battle_config.wedding_ignorepalette	},	//[Skotlex]
	{ "natural_heal_weight_rate",          &battle_config.natural_heal_weight_rate	},
	{ "item_name_override_grffile",        &battle_config.item_name_override_grffile},
	{ "item_equip_override_grffile",       &battle_config.item_equip_override_grffile},	// [Celest]
	{ "item_slots_override_grffile",       &battle_config.item_slots_override_grffile},	// [Celest]
	{ "indoors_override_grffile",          &battle_config.indoors_override_grffile},	// [Celest]
	{ "skill_sp_override_grffile",         &battle_config.skill_sp_override_grffile},	// [Celest]
	{ "cardillust_read_grffile",           &battle_config.cardillust_read_grffile},	// [Celest]
	{ "arrow_decrement",                   &battle_config.arrow_decrement			},
	{ "max_aspd",                          &battle_config.max_aspd					},
	{ "max_lv",                            &battle_config.max_lv					},
	{ "max_parameter",                     &battle_config.max_parameter			},
	{ "player_skill_log",                  &battle_config.pc_skill_log				},
	{ "monster_skill_log",                 &battle_config.mob_skill_log			},
	{ "battle_log",                        &battle_config.battle_log				},
	{ "save_log",                          &battle_config.save_log					},
	{ "error_log",                         &battle_config.error_log				},
	{ "etc_log",                           &battle_config.etc_log					},
	{ "save_clothcolor",                   &battle_config.save_clothcolor			},
	{ "undead_detect_type",                &battle_config.undead_detect_type		},
	{ "player_auto_counter_type",          &battle_config.pc_auto_counter_type		},
	{ "monster_auto_counter_type",         &battle_config.monster_auto_counter_type},
	{ "agi_penalty_type",                  &battle_config.agi_penalty_type			},
	{ "agi_penalty_count",                 &battle_config.agi_penalty_count			},
	{ "agi_penalty_num",                   &battle_config.agi_penalty_num			},
	{ "agi_penalty_count_lv",              &battle_config.agi_penalty_count_lv		},
	{ "vit_penalty_type",                  &battle_config.vit_penalty_type			},
	{ "vit_penalty_count",                 &battle_config.vit_penalty_count			},
	{ "vit_penalty_num",                   &battle_config.vit_penalty_num			},
	{ "vit_penalty_count_lv",              &battle_config.vit_penalty_count_lv		},
	{ "player_defense_type",               &battle_config.player_defense_type		},
	{ "monster_defense_type",              &battle_config.monster_defense_type		},
	{ "pet_defense_type",                  &battle_config.pet_defense_type			},
	{ "magic_defense_type",                &battle_config.magic_defense_type		},
	{ "player_skill_reiteration",          &battle_config.pc_skill_reiteration		},
	{ "monster_skill_reiteration",         &battle_config.monster_skill_reiteration},
	{ "player_skill_nofootset",            &battle_config.pc_skill_nofootset		},
	{ "monster_skill_nofootset",           &battle_config.monster_skill_nofootset	},
	{ "player_cloak_check_type",           &battle_config.pc_cloak_check_type		},
	{ "monster_cloak_check_type",          &battle_config.monster_cloak_check_type	},
	{ "gvg_short_attack_damage_rate",      &battle_config.gvg_short_damage_rate	},
	{ "gvg_long_attack_damage_rate",       &battle_config.gvg_long_damage_rate		},
	{ "gvg_magic_attack_damage_rate",      &battle_config.gvg_magic_damage_rate	},
	{ "gvg_misc_attack_damage_rate",       &battle_config.gvg_misc_damage_rate		},
	{ "mob_changetarget_byskill",          &battle_config.mob_changetarget_byskill},
	{ "player_attack_direction_change",    &battle_config.pc_attack_direction_change },
	{ "monster_attack_direction_change",   &battle_config.monster_attack_direction_change },
	{ "player_land_skill_limit",           &battle_config.pc_land_skill_limit		},
	{ "monster_land_skill_limit",          &battle_config.monster_land_skill_limit},
	{ "party_skill_penalty",               &battle_config.party_skill_penalty		},
	{ "monster_class_change_full_recover", &battle_config.monster_class_change_full_recover },
	{ "produce_item_name_input",           &battle_config.produce_item_name_input	},
	{ "produce_potion_name_input",         &battle_config.produce_potion_name_input},
	{ "making_arrow_name_input",           &battle_config.making_arrow_name_input	},
	{ "holywater_name_input",              &battle_config.holywater_name_input		},
	{ "display_delay_skill_fail",          &battle_config.display_delay_skill_fail	},
	{ "display_snatcher_skill_fail",       &battle_config.display_snatcher_skill_fail	},
	{ "chat_warpportal",                   &battle_config.chat_warpportal			},
	{ "mob_warpportal",                    &battle_config.mob_warpportal			},
	{ "dead_branch_active",                &battle_config.dead_branch_active			},
	{ "show_steal_in_same_party",          &battle_config.show_steal_in_same_party		},
	{ "pet_attack_attr_none",              &battle_config.pet_attack_attr_none		},
	{ "mob_attack_attr_none",              &battle_config.mob_attack_attr_none		},
	{ "mob_ghostring_fix",                 &battle_config.mob_ghostring_fix		},
	{ "pc_attack_attr_none",               &battle_config.pc_attack_attr_none		},
	{ "gx_allhit",                         &battle_config.gx_allhit				},
	{ "gx_cardfix",                        &battle_config.gx_cardfix				},
	{ "gx_dupele",                         &battle_config.gx_dupele				},
	{ "gx_disptype",                       &battle_config.gx_disptype				},
	{ "devotion_level_difference",         &battle_config.devotion_level_difference	},
	{ "player_skill_partner_check",        &battle_config.player_skill_partner_check},
	{ "hide_GM_session",                   &battle_config.hide_GM_session			},
	{ "unit_movement_type",                &battle_config.unit_movement_type		},
	{ "invite_request_check",              &battle_config.invite_request_check		},
	{ "skill_removetrap_type",             &battle_config.skill_removetrap_type	},
	{ "disp_experience",                   &battle_config.disp_experience			},
	{ "castle_defense_rate",               &battle_config.castle_defense_rate		},
	{ "hp_rate",                           &battle_config.hp_rate					},
	{ "sp_rate",                           &battle_config.sp_rate					},
	{ "gm_can_drop_lv",                    &battle_config.gm_can_drop_lv			},
	{ "disp_hpmeter",                      &battle_config.disp_hpmeter				},
	{ "bone_drop",		                     &battle_config.bone_drop				},
	{ "monster_damage_delay",              &battle_config.monster_damage_delay		},

// eAthena additions
	{ "item_drop_common_min",              &battle_config.item_drop_common_min	},	// Added by TyrNemesis^
	{ "item_drop_common_max",              &battle_config.item_drop_common_max	},
	{ "item_drop_equip_min",               &battle_config.item_drop_equip_min	},
	{ "item_drop_equip_max",               &battle_config.item_drop_equip_max	},
	{ "item_drop_card_min",                &battle_config.item_drop_card_min	},
	{ "item_drop_card_max",                &battle_config.item_drop_card_max	},
	{ "item_drop_mvp_min",                 &battle_config.item_drop_mvp_min	},
	{ "item_drop_mvp_max",                 &battle_config.item_drop_mvp_max	},	// End Addition
	{ "prevent_logout",                    &battle_config.prevent_logout		},	// Added by RoVeRT
	{ "alchemist_summon_reward",           &battle_config.alchemist_summon_reward	},	// [Valaris]
	{ "maximum_level",                     &battle_config.maximum_level	},	// [Valaris]
	{ "drops_by_luk",                      &battle_config.drops_by_luk	},	// [Valaris]
	{ "monsters_ignore_gm",                &battle_config.monsters_ignore_gm	},	// [Valaris]
	{ "equipment_breaking",                &battle_config.equipment_breaking	},	// [Valaris]
	{ "equipment_break_rate",              &battle_config.equipment_break_rate	}, 	// [Valaris]
	{ "pk_mode",                           &battle_config.pk_mode			},  	// [Valaris]
	{ "pet_equip_required",                &battle_config.pet_equip_required	},	// [Valaris]
	{ "multi_level_up",                    &battle_config.multi_level_up		}, // [Valaris]
	{ "backstab_bow_penalty",              &battle_config.backstab_bow_penalty	},
	{ "night_at_start",                    &battle_config.night_at_start	}, // added by [Yor]
	{ "show_mob_hp",                       &battle_config.show_mob_hp	}, // [Valaris]
	{ "ban_spoof_namer",                   &battle_config.ban_spoof_namer	}, // added by [Yor]
	{ "hack_info_GM_level",                &battle_config.hack_info_GM_level	}, // added by [Yor]
	{ "any_warp_GM_min_level",             &battle_config.any_warp_GM_min_level	}, // added by [Yor]
	{ "packet_ver_flag",                   &battle_config.packet_ver_flag	}, // added by [Yor]
	{ "min_hair_style",                    &battle_config.min_hair_style	}, // added by [MouseJstr]
	{ "max_hair_style",                    &battle_config.max_hair_style	}, // added by [MouseJstr]
	{ "min_hair_color",                    &battle_config.min_hair_color	}, // added by [MouseJstr]
	{ "max_hair_color",                    &battle_config.max_hair_color	}, // added by [MouseJstr]
	{ "min_cloth_color",                   &battle_config.min_cloth_color	}, // added by [MouseJstr]
	{ "max_cloth_color",                   &battle_config.max_cloth_color	}, // added by [MouseJstr]
	{ "castrate_dex_scale",                &battle_config.castrate_dex_scale	}, // added by [MouseJstr]
	{ "delayrate_dex_scale",		&battle_config.delayrate_dex_scale	}, // added by [Aru]
	{ "area_size",                         &battle_config.area_size	}, // added by [MouseJstr]
	{ "muting_players",                    &battle_config.muting_players}, // added by [Apple]
	{ "zeny_from_mobs",                    &battle_config.zeny_from_mobs}, // [Valaris]
	{ "mobs_level_up",                     &battle_config.mobs_level_up}, // [Valaris]
	{ "pk_min_level",                      &battle_config.pk_min_level}, // [celest]
	{ "skill_steal_type",                  &battle_config.skill_steal_type}, // [celest]
	{ "skill_steal_rate",                  &battle_config.skill_steal_rate}, // [celest]
	{ "night_darkness_level",              &battle_config.night_darkness_level}, // [celest]
	{ "motd_type",                         &battle_config.motd_type}, // [celest]
	{ "allow_atcommand_when_mute",         &battle_config.allow_atcommand_when_mute}, // [celest]
	{ "finding_ore_rate",                  &battle_config.finding_ore_rate}, // [celest]
	{ "exp_calc_type",                     &battle_config.exp_calc_type}, // [celest]
	{ "min_skill_delay_limit",             &battle_config.min_skill_delay_limit}, // [celest]
	{ "require_glory_guild",               &battle_config.require_glory_guild}, // [celest]
	{ "idle_no_share",                     &battle_config.idle_no_share}, // [celest], for a feature by [MouseJstr]
	{ "delay_battle_damage",               &battle_config.delay_battle_damage}, // [celest]
	{ "display_version",	                  &battle_config.display_version}, // [Ancyker], for a feature by...?
	{ "who_display_aid",	                  &battle_config.who_display_aid}, // [Ancyker], for a feature by...?
	{ "display_hallucination",             &battle_config.display_hallucination}, // [Skotlex]
	{ "use_statpoint_table",               &battle_config.use_statpoint_table}, // [Skotlex]
	{ "new_attack_function",               &battle_config.new_attack_function}, // [Skotlex]
	{ "ignore_items_gender",               &battle_config.ignore_items_gender}, // [Lupus]
	{ "copyskill_restrict",			&battle_config.copyskill_restrict}, // [Aru]
	{ "berserk_candels_buffs",		&battle_config.berserk_cancels_buffs}, // [Aru]

//SQL-only options start
#ifndef TXT_ONLY
	{ "mail_system",                       &battle_config.mail_system	}, // added by [Valaris]
//SQL-only options end
#endif
};

static const struct battle_data_int {
	const char *str;
	int *val;
} battle_data_int[] = {	//List here battle_athena options which are type int!
	{ "item_first_get_time",               &battle_config.item_first_get_time		},
	{ "item_second_get_time",              &battle_config.item_second_get_time		},
	{ "item_third_get_time",               &battle_config.item_third_get_time		},
	{ "mvp_item_first_get_time",           &battle_config.mvp_item_first_get_time	},
	{ "mvp_item_second_get_time",          &battle_config.mvp_item_second_get_time	},
	{ "mvp_item_third_get_time",           &battle_config.mvp_item_third_get_time	},
	{ "base_exp_rate",                     &battle_config.base_exp_rate			},
	{ "job_exp_rate",                      &battle_config.job_exp_rate				},
	// Slave Exp Rate, Slave Drop Rate, and Tu (Turn undead) exp rate by aru)
	{ "slave_exp_rate",			&battle_config.slave_exp_rate			},
	{ "slave_drop_rate",			&battle_config.slave_drop_rate			},
	{ "tu_exp_rate",			&battle_config.tu_exp_rate			},
	{ "zeny_penalty",                      &battle_config.zeny_penalty				},
	{ "mvp_item_rate",                     &battle_config.mvp_item_rate			},
	{ "mvp_exp_rate",                      &battle_config.mvp_exp_rate				},
	{ "natural_healhp_interval",           &battle_config.natural_healhp_interval	},
	{ "natural_healsp_interval",           &battle_config.natural_healsp_interval	},
	{ "natural_heal_skill_interval",       &battle_config.natural_heal_skill_interval},
	{ "max_hp",                            &battle_config.max_hp					},
	{ "max_sp",                            &battle_config.max_sp					},
	{ "max_cart_weight",                   &battle_config.max_cart_weight			},
	{ "gvg_eliminate_time",                &battle_config.gvg_eliminate_time		},
	{ "vending_max_value",                 &battle_config.vending_max_value		},
// eAthena additions
	{ "item_rate_common",                  &battle_config.item_rate_common	},	// Added by RoVeRT
	{ "item_rate_equip",                   &battle_config.item_rate_equip	},
	{ "item_rate_card",                    &battle_config.item_rate_card	},	// End Addition
	{ "item_rate_heal",                    &battle_config.item_rate_heal	},	// Added by Valaris
	{ "item_rate_use",                     &battle_config.item_rate_use	},	// End
	{ "item_rate_chest",			&battle_config.item_rate_chest	},	// Added by Aru
	{ "day_duration",                      &battle_config.day_duration	}, // added by [Yor]
	{ "night_duration",                    &battle_config.night_duration	}, // added by [Yor]
};

int battle_set_value(char *w1, char *w2) {
	int i;
	for(i = 0; i < sizeof(battle_data_short) / (sizeof(battle_data_short[0])); i++)
		if (strcmpi(w1, battle_data_short[i].str) == 0) {
			* battle_data_short[i].val = battle_config_switch(w2);
			return 1;
		}
	for(i = 0; i < sizeof(battle_data_int) / (sizeof(battle_data_int[0])); i++)
		if (strcmpi(w1, battle_data_int[i].str) == 0) {
			*battle_data_int[i].val = battle_config_switch(w2);
			return 1;
		}
/*			
                  int val =  battle_config_switch(w2);
                  switch(battle_data[i].size) {
                  case 1:
                    *((unsigned char *) battle_data[i].val) = val;
                    break;
                  case 2:
                    *((unsigned short *) battle_data[i].val) = val;
                    break;
                  case 4:
                    *((unsigned int *) battle_data[i].val) = val;
                    break;
                  }
                  return 1;
		}
*/
	return 0;
}

void battle_set_defaults() {
	battle_config.warp_point_debug=0;
	battle_config.enemy_critical=0;
	battle_config.enemy_critical_rate=100;
	battle_config.enemy_str=1;
	battle_config.enemy_perfect_flee=0;
	battle_config.cast_rate=100;
	battle_config.delay_rate=100;
	battle_config.delay_dependon_dex=0;
	battle_config.sdelay_attack_enable=0;
	battle_config.left_cardfix_to_right=0;
	battle_config.pc_skill_add_range=0;
	battle_config.skill_out_range_consume=1;
	battle_config.mob_skill_add_range=0;
	battle_config.pc_damage_delay=1;
	battle_config.pc_damage_delay_rate=100;
	battle_config.defnotenemy=1;
	battle_config.random_monster_checklv=1;
	battle_config.attr_recover=1;
	battle_config.flooritem_lifetime=LIFETIME_FLOORITEM*1000;
	battle_config.item_auto_get=0;
	battle_config.item_first_get_time=3000;
	battle_config.item_second_get_time=1000;
	battle_config.item_third_get_time=1000;
	battle_config.mvp_item_first_get_time=10000;
	battle_config.mvp_item_second_get_time=10000;
	battle_config.mvp_item_third_get_time=2000;

	battle_config.drop_rate0item=0;
	battle_config.base_exp_rate=100;
	battle_config.job_exp_rate=100;
	battle_config.pvp_exp=1;
	battle_config.gtb_pvp_only=0;
	battle_config.death_penalty_type=0;
	battle_config.death_penalty_base=0;
	battle_config.death_penalty_job=0;
	battle_config.zeny_penalty=0;
	battle_config.restart_hp_rate=0;
	battle_config.restart_sp_rate=0;
	battle_config.mvp_item_rate=100;
	battle_config.mvp_exp_rate=100;
	battle_config.mvp_hp_rate=100;
	battle_config.monster_hp_rate=100;
	battle_config.monster_max_aspd=199;
	battle_config.atc_gmonly=0;
	battle_config.gm_allskill=0;
	battle_config.gm_allequip=0;
	battle_config.gm_skilluncond=0;
	battle_config.guild_max_castles=0;
	battle_config.skillfree = 0;
	battle_config.skillup_limit = 0;
	battle_config.wp_rate=100;
	battle_config.pp_rate=100;
	battle_config.monster_active_enable=1;
	battle_config.monster_damage_delay_rate=100;
	battle_config.monster_loot_type=0;
//	battle_config.mob_skill_use=1;
	battle_config.mob_skill_rate=100;
	battle_config.mob_skill_delay=100;
	battle_config.mob_count_rate=100;
	battle_config.mob_spawn_delay=100;
	battle_config.plant_spawn_delay=100;
	battle_config.boss_spawn_delay=100;
	battle_config.quest_skill_learn=0;
	battle_config.quest_skill_reset=1;
	battle_config.basic_skill_check=1;
	battle_config.guild_emperium_check=1;
	battle_config.guild_exp_limit=50;
	battle_config.pc_invincible_time = 5000;
	battle_config.pet_catch_rate=100;
	battle_config.pet_rename=0;
	battle_config.pet_friendly_rate=100;
	battle_config.pet_hungry_delay_rate=100;
	battle_config.pet_hungry_friendly_decrease=5;
	battle_config.pet_str=1;
	battle_config.pet_status_support=0;
	battle_config.pet_attack_support=0;
	battle_config.pet_damage_support=0;
	battle_config.pet_support_rate=100;
	battle_config.pet_attack_exp_to_master=0;
	battle_config.pet_attack_exp_rate=100;
	battle_config.pet_lv_rate=0;	//Skotlex
	battle_config.pet_max_stats=99;	//Skotlex
	battle_config.pet_max_atk1=750;	//Skotlex
	battle_config.pet_max_atk2=1000;	//Skotlex
	battle_config.skill_min_damage=0;
	battle_config.finger_offensive_type=0;
	battle_config.heal_exp=0;
	battle_config.resurrection_exp=0;
	battle_config.shop_exp=0;
	battle_config.combo_delay_rate=100;
	battle_config.item_check=1;
	battle_config.wedding_modifydisplay=0;
	battle_config.wedding_ignorepalette=0;
	battle_config.natural_healhp_interval=6000;
	battle_config.natural_healsp_interval=8000;
	battle_config.natural_heal_skill_interval=10000;
	battle_config.natural_heal_weight_rate=50;
	battle_config.item_name_override_grffile=1;
	battle_config.item_equip_override_grffile=0;	// [Celest]
	battle_config.item_slots_override_grffile=0;	// [Celest]
	battle_config.indoors_override_grffile=0;	// [Celest]
	battle_config.skill_sp_override_grffile=0;	// [Celest]
	battle_config.cardillust_read_grffile=0;	// [Celest]
	battle_config.arrow_decrement=1;
	battle_config.max_aspd = 199;
	battle_config.max_hp = 32500;
	battle_config.max_sp = 32500;
	battle_config.max_lv = 99; // [MouseJstr]
	battle_config.max_parameter = 99;
	battle_config.max_cart_weight = 8000;
	battle_config.pc_skill_log = 0;
	battle_config.mob_skill_log = 0;
	battle_config.battle_log = 0;
	battle_config.save_log = 0;
	battle_config.error_log = 1;
	battle_config.etc_log = 1;
	battle_config.save_clothcolor = 0;
	battle_config.undead_detect_type = 0;
	battle_config.pc_auto_counter_type = 1;
	battle_config.monster_auto_counter_type = 1;
	battle_config.agi_penalty_type = 1;
	battle_config.agi_penalty_count = 3;
	battle_config.agi_penalty_num = 10;
	battle_config.agi_penalty_count_lv = ATK_FLEE;
	battle_config.vit_penalty_type = 1;
	battle_config.vit_penalty_count = 3;
	battle_config.vit_penalty_num = 5;
	battle_config.vit_penalty_count_lv = ATK_DEF;
	battle_config.player_defense_type = 0;
	battle_config.monster_defense_type = 0;
	battle_config.pet_defense_type = 0;
	battle_config.magic_defense_type = 0;
	battle_config.pc_skill_reiteration = 0;
	battle_config.monster_skill_reiteration = 0;
	battle_config.pc_skill_nofootset = 0;
	battle_config.monster_skill_nofootset = 0;
	battle_config.pc_cloak_check_type = 0;
	battle_config.monster_cloak_check_type = 0;
	battle_config.gvg_short_damage_rate = 100;
	battle_config.gvg_long_damage_rate = 60;
	battle_config.gvg_magic_damage_rate = 50;
	battle_config.gvg_misc_damage_rate = 60;
	battle_config.gvg_eliminate_time = 7000;
	battle_config.mob_changetarget_byskill = 0;
	battle_config.pc_attack_direction_change = 1;
	battle_config.monster_attack_direction_change = 1;
	battle_config.pc_land_skill_limit = 1;
	battle_config.monster_land_skill_limit = 1;
	battle_config.party_skill_penalty = 1;
	battle_config.monster_class_change_full_recover = 0;
	battle_config.produce_item_name_input = 1;
	battle_config.produce_potion_name_input = 1;
	battle_config.making_arrow_name_input = 1;
	battle_config.holywater_name_input = 1;
	battle_config.display_delay_skill_fail = 1;
	battle_config.display_snatcher_skill_fail = 1;
	battle_config.chat_warpportal = 0;
	battle_config.mob_warpportal = 0;
	battle_config.dead_branch_active = 0;
	battle_config.vending_max_value = 10000000;
	battle_config.show_steal_in_same_party = 0;
	battle_config.pet_attack_attr_none = 0;
	battle_config.pc_attack_attr_none = 0;
	battle_config.mob_attack_attr_none = 1;
	battle_config.mob_ghostring_fix = 0;
	battle_config.gx_allhit = 0;
	battle_config.gx_cardfix = 0;
	battle_config.gx_dupele = 1;
	battle_config.gx_disptype = 1;
	battle_config.devotion_level_difference = 10;
	battle_config.player_skill_partner_check = 1;
	battle_config.hide_GM_session = 0;
	battle_config.unit_movement_type = 0;
	battle_config.invite_request_check = 1;
	battle_config.skill_removetrap_type = 0;
	battle_config.disp_experience = 0;
	battle_config.castle_defense_rate = 100;
	battle_config.hp_rate = 100;
	battle_config.sp_rate = 100;
	battle_config.gm_can_drop_lv = 0;
	battle_config.disp_hpmeter = 0;
	battle_config.bone_drop = 0;
	battle_config.monster_damage_delay = 1;

// eAthena additions
	battle_config.item_rate_common = 100;
	battle_config.item_rate_equip = 100;
	battle_config.item_rate_card = 100;
	battle_config.item_rate_heal = 100;		// Added by Valaris
	battle_config.item_rate_use = 100;		// End
	battle_config.item_drop_common_min=1;	// Added by TyrNemesis^
	battle_config.item_drop_common_max=10000;
	battle_config.item_drop_equip_min=1;
	battle_config.item_drop_equip_max=10000;
	battle_config.item_drop_card_min=1;
	battle_config.item_drop_card_max=10000;
	battle_config.item_drop_mvp_min=1;
	battle_config.item_drop_mvp_max=10000;	// End Addition
	battle_config.item_drop_heal_min=1;		// Added by Valaris
	battle_config.item_drop_heal_max=10000;
	battle_config.item_drop_use_min=1;
	battle_config.item_drop_use_max=10000;	// End
	battle_config.prevent_logout = 1;	// Added by RoVeRT
	battle_config.maximum_level = 255;	// Added by Valaris
	battle_config.drops_by_luk = 0;	// [Valaris]
	battle_config.equipment_breaking = 0; // [Valaris]
	battle_config.equipment_break_rate = 100; // [Valaris]
	battle_config.pk_mode = 0; // [Valaris]
	battle_config.pet_equip_required = 0; // [Valaris]
	battle_config.multi_level_up = 0; // [Valaris]
	battle_config.backstab_bow_penalty = 0; // Akaru
	battle_config.night_at_start = 0; // added by [Yor]
	battle_config.day_duration = 2*60*60*1000; // added by [Yor] (2 hours)
	battle_config.night_duration = 30*60*1000; // added by [Yor] (30 minutes)
	battle_config.show_mob_hp = 0; // [Valaris]
	battle_config.ban_spoof_namer = 5; // added by [Yor] (default: 5 minutes)
	battle_config.hack_info_GM_level = 60; // added by [Yor] (default: 60, GM level)
	battle_config.any_warp_GM_min_level = 20; // added by [Yor]
	battle_config.packet_ver_flag = 255; // added by [Yor]
	battle_config.min_hair_style = 0;
	battle_config.max_hair_style = 20;
	battle_config.min_hair_color = 0;
	battle_config.max_hair_color = 9;
	battle_config.min_cloth_color = 0;
	battle_config.max_cloth_color = 4;
	battle_config.zeny_from_mobs = 0;
	battle_config.mobs_level_up = 0;
	battle_config.pk_min_level = 55;
	battle_config.skill_steal_type = 1;
	battle_config.skill_steal_rate = 100;
	battle_config.night_darkness_level = 9;
	battle_config.motd_type = 0;
	battle_config.allow_atcommand_when_mute = 0;
	battle_config.finding_ore_rate = 100;
	battle_config.castrate_dex_scale = 150;
	battle_config.area_size = 14;
	battle_config.exp_calc_type = 1;
	battle_config.min_skill_delay_limit = 100;
	battle_config.require_glory_guild = 0;
	battle_config.idle_no_share = 0;
	battle_config.delay_battle_damage = 1;
	battle_config.display_version = 1;
	battle_config.who_display_aid = 0;
	battle_config.display_hallucination = 1;
	battle_config.ignore_items_gender = 1;

	battle_config.use_statpoint_table = 1;
	battle_config.new_attack_function = 0; //This is for test/debug purposes [Skotlex]

//SQL-only options start
#ifndef TXT_ONLY
	battle_config.mail_system = 0;
//SQL-only options end
#endif
}

void battle_validate_conf() {
	if(battle_config.flooritem_lifetime < 1000)
		battle_config.flooritem_lifetime = LIFETIME_FLOORITEM*1000;
/*	if(battle_config.restart_hp_rate < 0)
		battle_config.restart_hp_rate = 0;
	else*/ if(battle_config.restart_hp_rate > 100)
		battle_config.restart_hp_rate = 100;
/*	if(battle_config.restart_sp_rate < 0)
		battle_config.restart_sp_rate = 0;
	else*/ if(battle_config.restart_sp_rate > 100)
		battle_config.restart_sp_rate = 100;
	if(battle_config.natural_healhp_interval < NATURAL_HEAL_INTERVAL)
		battle_config.natural_healhp_interval=NATURAL_HEAL_INTERVAL;
	if(battle_config.natural_healsp_interval < NATURAL_HEAL_INTERVAL)
		battle_config.natural_healsp_interval=NATURAL_HEAL_INTERVAL;
	if(battle_config.natural_heal_skill_interval < NATURAL_HEAL_INTERVAL)
		battle_config.natural_heal_skill_interval=NATURAL_HEAL_INTERVAL;
	if(battle_config.natural_heal_weight_rate < 50)
		battle_config.natural_heal_weight_rate = 50;
	if(battle_config.natural_heal_weight_rate > 101)
		battle_config.natural_heal_weight_rate = 101;
	battle_config.monster_max_aspd = 2000 - battle_config.monster_max_aspd*10;
	if(battle_config.monster_max_aspd < 10)
		battle_config.monster_max_aspd = 10;
	if(battle_config.monster_max_aspd > 1000)
		battle_config.monster_max_aspd = 1000;
	battle_config.max_aspd = 2000 - battle_config.max_aspd*10;
	if(battle_config.max_aspd < 10)
		battle_config.max_aspd = 10;
	if(battle_config.max_aspd > 1000)
		battle_config.max_aspd = 1000;
	if(battle_config.hp_rate < 1)
		battle_config.hp_rate = 1;
	if(battle_config.sp_rate < 1)
		battle_config.sp_rate = 1;
	if(battle_config.max_hp > 1000000)
		battle_config.max_hp = 1000000;
	if(battle_config.max_hp < 100)
		battle_config.max_hp = 100;
	if(battle_config.max_sp > 1000000)
		battle_config.max_sp = 1000000;
	if(battle_config.max_sp < 100)
		battle_config.max_sp = 100;
	if(battle_config.max_parameter < 10)
		battle_config.max_parameter = 10;
	if(battle_config.max_parameter > 10000)
		battle_config.max_parameter = 10000;
	if(battle_config.max_cart_weight > 1000000)
		battle_config.max_cart_weight = 1000000;
	if(battle_config.max_cart_weight < 100)
		battle_config.max_cart_weight = 100;
	battle_config.max_cart_weight *= 10;

	if(battle_config.agi_penalty_count < 2)
		battle_config.agi_penalty_count = 2;
	if(battle_config.vit_penalty_count < 2)
		battle_config.vit_penalty_count = 2;

	if(battle_config.guild_exp_limit > 99)
		battle_config.guild_exp_limit = 99;
/*	if(battle_config.guild_exp_limit < 0)
		battle_config.guild_exp_limit = 0;*/

	if(battle_config.pet_max_atk1 > battle_config.pet_max_atk2)	//Skotlex
		battle_config.pet_max_atk1 = battle_config.pet_max_atk2;
	
//	if(battle_config.castle_defense_rate < 0)
//		battle_config.castle_defense_rate = 0;
	if(battle_config.castle_defense_rate > 100)
		battle_config.castle_defense_rate = 100;
	if(battle_config.item_drop_common_min < 1)		// Added by TyrNemesis^
		battle_config.item_drop_common_min = 1;
	if(battle_config.item_drop_common_max > 10000)
		battle_config.item_drop_common_max = 10000;
	if(battle_config.item_drop_equip_min < 1)
		battle_config.item_drop_equip_min = 1;
	if(battle_config.item_drop_equip_max > 10000)
		battle_config.item_drop_equip_max = 10000;
	if(battle_config.item_drop_card_min < 1)
		battle_config.item_drop_card_min = 1;
	if(battle_config.item_drop_card_max > 10000)
		battle_config.item_drop_card_max = 10000;
	if(battle_config.item_drop_mvp_min < 1)
		battle_config.item_drop_mvp_min = 1;
	if(battle_config.item_drop_mvp_max > 10000)
		battle_config.item_drop_mvp_max = 10000;	// End Addition

/*	if (battle_config.night_at_start < 0) // added by [Yor]
		battle_config.night_at_start = 0;
	else if (battle_config.night_at_start > 1) // added by [Yor]
		battle_config.night_at_start = 1; */
	if (battle_config.day_duration != 0 && battle_config.day_duration < 60000) // added by [Yor]
		battle_config.day_duration = 60000;
	if (battle_config.night_duration != 0 && battle_config.night_duration < 60000) // added by [Yor]
		battle_config.night_duration = 60000;
	
/*	if (battle_config.ban_spoof_namer < 0) // added by [Yor]
		battle_config.ban_spoof_namer = 0;
	else*/ if (battle_config.ban_spoof_namer > 32767)
		battle_config.ban_spoof_namer = 32767;

/*	if (battle_config.hack_info_GM_level < 0) // added by [Yor]
		battle_config.hack_info_GM_level = 0;
	else*/ if (battle_config.hack_info_GM_level > 100)
		battle_config.hack_info_GM_level = 100;

/*	if (battle_config.any_warp_GM_min_level < 0) // added by [Yor]
		battle_config.any_warp_GM_min_level = 0;
	else*/ if (battle_config.any_warp_GM_min_level > 100)
		battle_config.any_warp_GM_min_level = 100;

	// at least 1 client must be accepted
	if ((battle_config.packet_ver_flag & 255) == 0) // added by [Yor]
		battle_config.packet_ver_flag = 255; // accept all clients

	if (battle_config.night_darkness_level > 10) // Celest
		battle_config.night_darkness_level = 10;

/*	if (battle_config.motd_type < 0)
		battle_config.motd_type = 0;
	else if (battle_config.motd_type > 1)
		battle_config.motd_type = 1;
*/
//	if (battle_config.finding_ore_rate < 0)
//		battle_config.finding_ore_rate = 0;

	if (battle_config.vending_max_value<=0) // Lupus & Kobra_k88
		battle_config.vending_max_value = 10000000;

	if (battle_config.min_skill_delay_limit < 10)
		battle_config.min_skill_delay_limit = 10;	// minimum delay of 10ms

	//Spawn delays [Skotlex]
/*	if (battle_config.mob_spawn_delay < 0)
		battle_config.mob_spawn_delay = 0;
	if (battle_config.boss_spawn_delay < 0)
		battle_config.boss_spawn_delay = 0;
	if (battle_config.plant_spawn_delay < 0)
		battle_config.plant_spawn_delay = 0;
*/	
}

/*==========================================
 * 設定ファイルを読み込む
 *------------------------------------------
 */
int battle_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int count = 0;

	if ((count++) == 0)
		battle_set_defaults();

	fp = fopen(cfgName,"r");
	if (fp == NULL) {
		printf("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]:%s", w1, w2) != 2)
			continue;
		battle_set_value(w1, w2);
		if (strcmpi(w1, "import") == 0)
			battle_config_read(w2);
	}
	fclose(fp);

	if (--count == 0) {
		battle_validate_conf();
		add_timer_func_list(battle_delay_damage_sub, "battle_delay_damage_sub");
	}

	return 0;
}
