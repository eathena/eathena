// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// ƒXƒe[ƒ^ƒXŒvZAó‘ÔˆÙíˆ—
#include "pc.h"
#include "map.h"
#include "pet.h"
#include "homun.h"
#include "mob.h"
#include "npc.h"
#include "clif.h"
#include "guild.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "chrif.h"
#include "status.h"

#include "timer.h"
#include "nullpo.h"
#include "script.h"
#include "showmsg.h"
#include "utils.h"


// ƒXƒLƒ‹”Ô?„ƒXƒe?ƒ^ƒXˆÙí”Ô??Š·ƒe?ƒuƒ‹
int SkillStatusChangeTable[]={	// status.h‚Ìenum‚ÌSC_***‚Æ‚ ‚í‚¹‚é‚±‚Æ
// 0-
	-1,-1,-1,-1,-1,-1,
	SC_PROVOKE,			// ƒvƒƒ{ƒbƒN
	SC_WATK_ELEMENT,		//Converts part of your final attack into an elemental one. [Skotlex]
	SC_ENDURE,
	-1,
// 10-
	SC_SIGHT,			// ƒTƒCƒg
	-1,
	SC_SAFETYWALL,		// ƒZ[ƒtƒeƒB[ƒEƒH[ƒ‹
	-1,-1,-1,
	SC_FREEZE,			// ƒtƒƒXƒgƒ_ƒCƒo?
	SC_STONE,			// ƒXƒg?ƒ“ƒJ?ƒX
	-1,-1,
// 20-
	-1,-1,-1,-1,
	SC_RUWACH,			// ƒ‹ƒAƒt
	SC_PNEUMA,			// ƒjƒ…[ƒ}
	-1,-1,-1,
	SC_INCREASEAGI,		// ‘¬“x?‰Á
// 30-
	SC_DECREASEAGI,		// ‘¬“xŒ¸­
	-1,
	SC_SIGNUMCRUCIS,	// ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX
	SC_ANGELUS,			// ƒGƒ“ƒWƒFƒ‰ƒX
	SC_BLESSING,		// ƒuƒŒƒbƒVƒ“ƒO
	-1,-1,-1,-1,-1,
// 40-
	-1,-1,-1,-1,-1,
	SC_CONCENTRATE,		// W’†—ÍŒüã
	-1,-1,-1,-1,
// 50-
	-1,
	SC_HIDING,			// ƒnƒCƒfƒBƒ“ƒO
	-1,-1,-1,-1,-1,-1,-1,-1,
// 60-
	SC_TWOHANDQUICKEN,	// 2HQ
	SC_AUTOCOUNTER,
	-1,-1,-1,-1,
	SC_IMPOSITIO,		// ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX
	SC_SUFFRAGIUM,		// ƒTƒtƒ‰ƒMƒEƒ€
	SC_ASPERSIO,		// ƒAƒXƒyƒ‹ƒVƒI
	SC_BENEDICTIO,		// ¹?~•Ÿ
// 70-
	-1,
	SC_SLOWPOISON,
	-1,
	SC_KYRIE,			// ƒLƒŠƒGƒGƒŒƒCƒ\ƒ“
	SC_MAGNIFICAT,		// ƒ}ƒOƒjƒtƒBƒJ?ƒg
	SC_GLORIA,			// ƒOƒƒŠƒA
	SC_DIVINA,			// ƒŒƒbƒNƒXƒfƒBƒr?ƒi
	-1,
	SC_AETERNA,			// ƒŒƒbƒNƒXƒG?ƒeƒ‹ƒi
	-1,
// 80-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 90-
	-1,-1,
	SC_QUAGMIRE,		// ƒNƒ@ƒOƒ}ƒCƒA
	-1,-1,-1,-1,-1,-1,-1,
// 100-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 110-
	-1,
	SC_ADRENALINE,		// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
	SC_WEAPONPERFECTION,// ƒEƒFƒ|ƒ“ƒp?ƒtƒFƒNƒVƒ‡ƒ“
	SC_OVERTHRUST,		// ƒI?ƒo?ƒgƒ‰ƒXƒg
	SC_MAXIMIZEPOWER,	// ƒ}ƒLƒVƒ}ƒCƒYƒpƒ?
	-1,-1,-1,-1,-1,
// 120-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 130-
	-1,-1,-1,-1,-1,
	SC_CLOAKING,		// ƒNƒ?ƒLƒ“ƒO
	SC_STAN,			// ƒ\ƒjƒbƒNƒuƒ?
	-1,
	SC_ENCPOISON,		// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
	SC_POISONREACT,		// ƒ|ƒCƒYƒ“ƒŠƒAƒNƒg
// 140-
	SC_POISON,			// ƒxƒmƒ€ƒ_ƒXƒg
	SC_SPLASHER,		// ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ?
	-1,
	SC_TRICKDEAD,		// €‚ñ‚¾‚Ó‚è
	-1,-1,
	SC_AUTOBERSERK,
	-1,-1,-1,
// 150-
	-1,-1,-1,-1,-1,
	SC_LOUD,			// ƒ‰ƒEƒhƒ{ƒCƒX
	-1,
	SC_ENERGYCOAT,		// ƒGƒiƒW?ƒR?ƒg
	-1,-1,
// 160-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,
	SC_KEEPING,
	-1,-1,
	SC_BARRIER,
	-1,-1,
	SC_HALLUCINATION,
	-1,-1,
// 210-
	-1,-1,-1,-1,-1,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR,
	SC_STRIPHELM,
	-1,
// 220-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 230-
	-1,-1,-1,-1,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	-1,-1,
// 240-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
	SC_AUTOGUARD,
// 250-
	-1,-1,
	SC_REFLECTSHIELD,
	-1,-1,
	SC_DEVOTION,
	SC_PROVIDENCE,
	SC_DEFENDER,
	SC_SPEARSQUICKEN,
	-1,
// 260-
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_STEELBODY,
	SC_BLADESTOP_WAIT,
// 270-
	SC_EXPLOSIONSPIRITS,
	SC_EXTREMITYFIST,
	-1,-1,-1,-1,
	SC_MAGICROD,
	-1,-1,-1,
// 280-
	SC_FLAMELAUNCHER,
	SC_FROSTWEAPON,
	SC_LIGHTNINGLOADER,
	SC_SEISMICWEAPON,
	-1,
	SC_VOLCANO,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_LANDPROTECTOR,
	-1,
// 290-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 300-
	-1,-1,-1,-1,-1,-1,
	SC_LULLABY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
// 310-
	SC_NIBELUNGEN,
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	-1,-1,-1,
	SC_DISSONANCE,
	-1,
	SC_WHISTLE,
// 320-
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	-1,-1,
	SC_UGLYDANCE,
	-1,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE,
// 330-
	SC_SERVICE4U,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 340-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 350-
	-1,-1,-1,-1,-1,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION,
	SC_TENSIONRELAX,
	SC_BERSERK,
// 360-
	SC_BERSERK,
	SC_ASSUMPTIO,
	SC_BASILICA,
	-1,-1,-1,
	SC_MAGICPOWER,
	-1,
	SC_SACRIFICE,
	SC_GOSPEL,
// 370-
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_EDP,
	-1,
// 380-
	SC_TRUESIGHT,
	-1,-1,
	SC_WINDWALK,
	SC_MELTDOWN,
	-1,-1,
	SC_CARTBOOST,
	-1,
	SC_CHASEWALK,
// 390-
	SC_REJECTSWORD,
	-1,-1,-1,-1,
	SC_MOONLIT,
	SC_MARIONETTE,
	-1,
	SC_BLEEDING,
	SC_JOINTBEAT,
// 400
	-1,-1,
	SC_MINDBREAKER,
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	-1,-1,
	SC_BABY,
	-1,
// 410-
	-1,
	SC_RUN,
	SC_READYSTORM,
	SC_STORMKICK,
	SC_READYDOWN,
	SC_DOWNKICK,
	SC_READYTURN,
	SC_TURNKICK,
	SC_READYCOUNTER,
	SC_COUNTER,
// 420-
	SC_DODGE,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 430-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 440-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 450-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 460-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
// 470-
	-1,-1,-1,-1,-1,
	SC_PRESERVE,
	-1,-1,-1,-1,
// 480-
	-1,-1,
	SC_DOUBLECAST,
	-1,
	SC_GRAVITATION,
	-1,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	-1,
// 490-
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

static int max_weight_base[MAX_PC_CLASS];
static int hp_coefficient[MAX_PC_CLASS];
static int hp_coefficient2[MAX_PC_CLASS];
static int hp_sigma_val[MAX_PC_CLASS][MAX_LEVEL];
static int sp_coefficient[MAX_PC_CLASS];
static int aspd_base[MAX_PC_CLASS][20];
static int refinebonus[MAX_REFINE_BONUS][3];	// ¸˜Bƒ{[ƒiƒXƒe[ƒuƒ‹(refine_db.txt)
int percentrefinery[5][MAX_REFINE+1];	// ¸˜B¬Œ÷—¦(refine_db.txt)
static int atkmods[3][20];	// •ŠíATKƒTƒCƒYC³(size_fix.txt)
static char job_bonus[3][MAX_PC_CLASS][MAX_LEVEL];

int current_equip_item_index;
//Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

/*==========================================
 * ¸˜Bƒ{[ƒiƒX
 *------------------------------------------
 */
int status_getrefinebonus(int lv,int type)
{
	if (lv >= 0 && lv < MAX_REFINE_BONUS && type >= 0 && type < 3)
		return refinebonus[lv][type];
	return 0;
}

/*==========================================
 * ¸˜B¬Œ÷—¦
 *------------------------------------------
 */
int status_percentrefinery(map_session_data &sd,struct item &item)
{
	int percent;

	percent=percentrefinery[itemdb_wlv(item.nameid)][item.refine];

	percent += pc_checkskill(sd,BS_WEAPONRESEARCH);	// •ŠíŒ¤‹†ƒXƒLƒ‹Š

	// Šm—¦‚Ì—LŒø”ÍˆÍƒ`ƒFƒbƒN
	if( percent > 100 )
		percent = 100;
	if( percent < 0 )
		percent = 0;
	return percent;
}

//Skotlex: Calculates the stats of the given pet.
int status_calc_pet(map_session_data &sd, bool first)
{
	struct pet_data *pd;
	
	if (sd.status.pet_id == 0 || sd.pd == NULL)
		return 1;

	pd = sd.pd;
	
	if (config.pet_lv_rate && pd->status)
	{
		sd.pd->pet.level = sd.status.base_level*config.pet_lv_rate/100;
		if( pd->pet.level <= 0 )
			pd->pet.level = 1;
		if( first || pd->status && pd->status->level != pd->pet.level)
		{
			if (!first) //Lv Up animation
				clif_misceffect(*pd, 0);
			pd->status->level = pd->pet.level;
			pd->status->atk1 = (mob_db[pd->pet.class_].atk1*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->atk2 = (mob_db[pd->pet.class_].atk2*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->str = (mob_db[pd->pet.class_].str*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->agi = (mob_db[pd->pet.class_].agi*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->vit = (mob_db[pd->pet.class_].vit*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->int_ = (mob_db[pd->pet.class_].int_*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->dex = (mob_db[pd->pet.class_].dex*pd->status->level)/mob_db[pd->pet.class_].lv;
			pd->status->luk = (mob_db[pd->pet.class_].luk*pd->status->level)/mob_db[pd->pet.class_].lv;
		
			if (pd->status->atk1 > config.pet_max_atk1) pd->status->atk1 = config.pet_max_atk1;
			if (pd->status->atk2 > config.pet_max_atk2) pd->status->atk2 = config.pet_max_atk2;

			if (pd->status->str > config.pet_max_stats) pd->status->str = config.pet_max_stats;
			else if (pd->status->str < 1) pd->status->str = 1;
			if (pd->status->agi > config.pet_max_stats) pd->status->agi = config.pet_max_stats;
			else if (pd->status->agi < 1) pd->status->agi = 1;
			if (pd->status->vit > config.pet_max_stats) pd->status->vit = config.pet_max_stats;
			else if (pd->status->vit < 1) pd->status->vit = 1;
			if (pd->status->int_ > config.pet_max_stats) pd->status->int_ = config.pet_max_stats;
			else if (pd->status->int_ < 1) pd->status->int_ = 1;
			if (pd->status->dex > config.pet_max_stats) pd->status->dex = config.pet_max_stats;
			else if (pd->status->dex < 1) pd->status->dex = 1;
			if (pd->status->luk > config.pet_max_stats) pd->status->luk = config.pet_max_stats;
			else if (pd->status->luk < 1) pd->status->luk = 1;

			if (!first)	//Not done the first time because the pet is not visible yet
				clif_send_petstatus(sd);
		}
	}
	//Support rate modifier (1000 = 100%)
	pd->rate_fix = 1000*(pd->pet.intimate - config.pet_support_min_friendly)/(1000- config.pet_support_min_friendly) +500;
	if(config.pet_support_rate != 100)
		pd->rate_fix = pd->rate_fix*config.pet_support_rate/100;
	return 0;
}	

/*==========================================
 * ƒpƒ‰ƒ[ƒ^ŒvZ
 * first==0‚ÌAŒvZ‘ÎÛ‚Ìƒpƒ‰ƒ[ƒ^‚ªŒÄ‚Ño‚µ‘O‚©‚ç
 * •Ï ‰»‚µ‚½ê‡©“®‚Åsend‚·‚é‚ªA
 * ”\“®“I‚É•Ï‰»‚³‚¹‚½ƒpƒ‰ƒ[ƒ^‚Í©‘O‚Åsend‚·‚é‚æ‚¤‚É
 *------------------------------------------
 */

int status_calc_pc(map_session_data& sd, int first)
{
	static int calculating = 0; //Check for recursive call preemption. [Skotlex]
	if (calculating < 10) //Too many recursive calls to status_calc_pc!
	{
		calculating++;

		int b_speed,b_max_hp,b_max_sp,b_hp,b_sp,b_weight,b_max_weight,b_hit,b_flee;
		int b_aspd,b_watk,b_def,b_watk2,b_def2,b_flee2,b_critical,b_attackrange,b_matk1,b_matk2,b_mdef,b_mdef2,b_class;
		int b_base_atk;
		struct skill b_skill[MAX_SKILL];
		long b_paramb[6];
		long b_parame[6];
		int i;
		size_t j;
		int bl,index;
		int skill,aspd_rate,wele,wele_,def_ele,refinedef=0;
		int pele=0,pdef_ele=0;
		int str,dstr,dex;
		struct pc_base_job s_class;


		//Calculate Common Class and Baby/High/Common flags
		s_class = pc_calc_base_job(sd.status.class_);

		b_speed = sd.speed;
		b_max_hp = sd.status.max_hp;
		b_max_sp = sd.status.max_sp;
		b_hp = sd.status.hp;
		b_sp = sd.status.sp;
		b_weight = sd.weight;
		b_max_weight = sd.max_weight;
		memcpy(b_paramb,sd.paramb,sizeof(sd.paramb));
		memcpy(b_parame,sd.paramc,sizeof(sd.paramc));
		memcpy(b_skill,sd.status.skill,sizeof(sd.status.skill));
		b_hit = sd.hit;
		b_flee = sd.flee;
		b_aspd = sd.aspd;
		b_watk = sd.right_weapon.watk;
		b_def = sd.def;
		b_watk2 = sd.right_weapon.watk2;
		b_def2 = sd.def2;
		b_flee2 = sd.flee2;
		b_critical = sd.critical;
		b_attackrange = sd.attackrange;
		b_matk1 = sd.matk1;
		b_matk2 = sd.matk2;
		b_mdef = sd.mdef;
		b_mdef2 = sd.mdef2;
		b_class = sd.view_class;
		sd.view_class = sd.status.class_;
		b_base_atk = sd.base_atk;

		pc_calc_skilltree(sd);	// ƒXƒLƒ‹ƒcƒŠ?‚ÌŒvZ

		sd.max_weight = max_weight_base[s_class.job]+sd.status.str*300;

		if(first&1)
		{
			sd.weight=0;
			for(i=0;i<MAX_INVENTORY;++i){
				if(sd.status.inventory[i].nameid==0 || sd.inventory_data[i] == NULL)
					continue;
				sd.weight += sd.inventory_data[i]->weight*sd.status.inventory[i].amount;
			}
			sd.cart_max_weight=config.max_cart_weight;
			sd.cart_weight=0;
			sd.cart_max_num=MAX_CART;
			sd.cart_num=0;
			for(i=0;i<MAX_CART;++i){
				if(sd.status.cart[i].nameid==0)
					continue;
				sd.cart_weight+=itemdb_weight(sd.status.cart[i].nameid)*sd.status.cart[i].amount;
				sd.cart_num++;
			}
		}

		memset(sd.paramb,0,sizeof(sd.paramb));
		memset(sd.parame,0,sizeof(sd.parame));
		sd.hit = 0;
		sd.flee = 0;
		sd.flee2 = 0;
		sd.critical = 0;
		sd.aspd = 0;
		sd.right_weapon.watk = 0;
		sd.def = 0;
		sd.mdef = 0;
		sd.right_weapon.watk2 = 0;
		sd.def2 = 0;
		sd.mdef2 = 0;
		sd.status.max_hp = 0;
		sd.status.max_sp = 0;
		sd.attackrange = 0;
		sd.attackrange_ = 0;
		sd.right_weapon.atk_ele = 0;
		sd.def_ele = 0;
		sd.right_weapon.star = 0;
		sd.right_weapon.overrefine = 0;
		sd.matk1 = 0;
		sd.matk2 = 0;
		sd.speed = DEFAULT_WALK_SPEED ;
		sd.hprate=config.hp_rate;
		sd.sprate=config.sp_rate;
		sd.castrate=100;
		sd.delayrate=100;
		sd.dsprate=100;
		sd.base_atk=0;
		sd.arrow_atk=0;
		sd.arrow_ele=0;
		sd.arrow_hit=0;
		sd.arrow_range=0;
		sd.nhealhp=sd.nhealsp=sd.nshealhp=sd.nshealsp=sd.nsshealhp=sd.nsshealsp=0;
		memset(sd.right_weapon.addele,0,sizeof(sd.right_weapon.addele));
		memset(sd.right_weapon.addrace,0,sizeof(sd.right_weapon.addrace));
		memset(sd.right_weapon.addsize,0,sizeof(sd.right_weapon.addsize));
		memset(sd.left_weapon.addele,0,sizeof(sd.left_weapon.addele));
		memset(sd.left_weapon.addrace,0,sizeof(sd.left_weapon.addrace));
		memset(sd.left_weapon.addsize,0,sizeof(sd.left_weapon.addsize));
		memset(sd.subele,0,sizeof(sd.subele));
		memset(sd.subrace,0,sizeof(sd.subrace));
		memset(sd.addeff,0,sizeof(sd.addeff));
		memset(sd.addeff2,0,sizeof(sd.addeff2));
		memset(sd.reseff,0,sizeof(sd.reseff));
		sd.state.killer = 0;
		sd.state.killable = 0;
		sd.state.restart_full_recover = 0;
		sd.state.no_castcancel = 0;
		sd.state.no_castcancel2 = 0;
		sd.state.no_sizefix = 0;
		sd.state.no_magic_damage = 0;
		sd.state.no_weapon_damage = 0;
		sd.state.no_gemstone = 0;
		sd.state.infinite_endure = 0;
		memset(sd.weapon_coma_ele,0,sizeof(sd.weapon_coma_ele));
		memset(sd.weapon_coma_race,0,sizeof(sd.weapon_coma_race));
		memset(sd.weapon_atk,0,sizeof(sd.weapon_atk));
		memset(sd.weapon_atk_rate,0,sizeof(sd.weapon_atk_rate));

		sd.left_weapon.watk = 0;			//“ñ“—¬—p(?)
		sd.left_weapon.watk2 = 0;
		sd.left_weapon.atk_ele = 0;
		sd.left_weapon.star = 0;
		sd.left_weapon.overrefine = 0;

		sd.aspd_rate = 100;
		sd.speed_rate = 100;
		sd.hprecov_rate = 100;
		sd.sprecov_rate = 100;
		sd.critical_def = 0;
		sd.double_rate = 0;
		sd.near_attack_def_rate = sd.long_attack_def_rate = 0;
		sd.atk_rate = sd.matk_rate = 100;
		sd.right_weapon.ignore_def_ele = sd.right_weapon.ignore_def_race = 0;
		sd.left_weapon.ignore_def_ele = sd.left_weapon.ignore_def_race = 0;
		sd.ignore_mdef_ele = sd.ignore_mdef_race = 0;
		sd.arrow_cri = 0;
		sd.magic_def_rate = sd.misc_def_rate = 0;
		memset(sd.arrow_addele,0,sizeof(sd.arrow_addele));
		memset(sd.arrow_addrace,0,sizeof(sd.arrow_addrace));
		memset(sd.arrow_addsize,0,sizeof(sd.arrow_addsize));
		memset(sd.arrow_addeff,0,sizeof(sd.arrow_addeff));
		memset(sd.arrow_addeff2,0,sizeof(sd.arrow_addeff2));
		memset(sd.magic_addele,0,sizeof(sd.magic_addele));
		memset(sd.magic_addrace,0,sizeof(sd.magic_addrace));
		memset(sd.magic_subrace,0,sizeof(sd.magic_subrace));
		sd.perfect_hit = 0;
		sd.critical_rate = sd.hit_rate = sd.flee_rate = sd.flee2_rate = 100;
		sd.def_rate = sd.def2_rate = sd.mdef_rate = sd.mdef2_rate = 100;
		sd.right_weapon.def_ratio_atk_ele = sd.left_weapon.def_ratio_atk_ele = 0;
		sd.right_weapon.def_ratio_atk_race = sd.left_weapon.def_ratio_atk_race = 0;
		sd.get_zeny_num = 0;
		sd.right_weapon.add_damage_class_count = sd.left_weapon.add_damage_class_count = sd.add_magic_damage_class_count = 0;
		sd.add_def_class_count = sd.add_mdef_class_count = 0;
		sd.monster_drop_item_count = 0;
		memset(sd.right_weapon.add_damage_classrate,0,sizeof(sd.right_weapon.add_damage_classrate));
		memset(sd.left_weapon.add_damage_classrate,0,sizeof(sd.left_weapon.add_damage_classrate));
		memset(sd.add_magic_damage_classrate,0,sizeof(sd.add_magic_damage_classrate));
		memset(sd.add_def_classrate,0,sizeof(sd.add_def_classrate));
		memset(sd.add_mdef_classrate,0,sizeof(sd.add_mdef_classrate));
		memset(sd.monster_drop_race,0,sizeof(sd.monster_drop_race));
		memset(sd.monster_drop_itemrate,0,sizeof(sd.monster_drop_itemrate));
		sd.speed_add_rate = sd.aspd_add_rate = 100;
		sd.double_add_rate = sd.perfect_hit_add = sd.get_zeny_add_num = 0;
		sd.splash_range = sd.splash_add_range = 0;
		memset(sd.autospell_id,0,sizeof(sd.autospell_id));
		memset(sd.autospell_lv,0,sizeof(sd.autospell_lv));
		memset(sd.autospell_rate,0,sizeof(sd.autospell_rate));
		sd.right_weapon.hp_drain_rate = sd.right_weapon.hp_drain_per = sd.right_weapon.sp_drain_rate = sd.right_weapon.sp_drain_per = 0;
		sd.left_weapon.hp_drain_rate = sd.left_weapon.hp_drain_per = sd.left_weapon.sp_drain_rate = sd.left_weapon.sp_drain_per = 0;
		sd.short_weapon_damage_return = sd.long_weapon_damage_return = 0;
		sd.magic_damage_return = 0;
		sd.random_attack_increase_add = sd.random_attack_increase_per = 0;
		sd.right_weapon.hp_drain_value = sd.left_weapon.hp_drain_value = sd.right_weapon.sp_drain_value = sd.left_weapon.sp_drain_value = 0;
		sd.unbreakable_equip = 0;

		sd.break_weapon_rate = sd.break_armor_rate = 0;
		sd.add_steal_rate = 0;
		sd.crit_atk_rate = 0;
		sd.no_regen = 0;
		sd.unstripable_equip = 0;
		memset(sd.autospell2_id,0,sizeof(sd.autospell2_id));
		memset(sd.autospell2_lv,0,sizeof(sd.autospell2_lv));
		memset(sd.autospell2_rate,0,sizeof(sd.autospell2_rate));
		memset(sd.critaddrace,0,sizeof(sd.critaddrace));
		memset(sd.addeff3,0,sizeof(sd.addeff3));
		memset(sd.addeff3_type,0,sizeof(sd.addeff3_type));
		memset(sd.skillatk,0,sizeof(sd.skillatk));
		sd.right_weapon.add_damage_class_count = sd.left_weapon.add_damage_class_count = sd.add_magic_damage_class_count = 0;
		sd.add_def_class_count = sd.add_mdef_class_count = 0;
		sd.add_damage_class_count2 = 0;
		memset(sd.right_weapon.add_damage_classid,0,sizeof(sd.right_weapon.add_damage_classid));
		memset(sd.left_weapon.add_damage_classid,0,sizeof(sd.left_weapon.add_damage_classid));
		memset(sd.add_magic_damage_classid,0,sizeof(sd.add_magic_damage_classid));
		memset(sd.right_weapon.add_damage_classrate,0,sizeof(sd.right_weapon.add_damage_classrate));
		memset(sd.left_weapon.add_damage_classrate,0,sizeof(sd.left_weapon.add_damage_classrate));
		memset(sd.add_magic_damage_classrate,0,sizeof(sd.add_magic_damage_classrate));
		memset(sd.add_def_classid,0,sizeof(sd.add_def_classid));
		memset(sd.add_def_classrate,0,sizeof(sd.add_def_classrate));
		memset(sd.add_mdef_classid,0,sizeof(sd.add_mdef_classid));
		memset(sd.add_mdef_classrate,0,sizeof(sd.add_mdef_classrate));
		memset(sd.add_damage_classid2,0,sizeof(sd.add_damage_classid2));
		memset(sd.add_damage_classrate2,0,sizeof(sd.add_damage_classrate2));
		sd.sp_gain_value = 0;
		sd.right_weapon.ignore_def_mob = sd.left_weapon.ignore_def_mob = 0;
		sd.hp_loss_rate = sd.hp_loss_value = sd.hp_loss_type = 0;
		sd.sp_loss_rate = sd.sp_loss_value = 0;
		memset(sd.right_weapon.addrace2,0,sizeof(sd.right_weapon.addrace2));
		memset(sd.left_weapon.addrace2,0,sizeof(sd.left_weapon.addrace2));
		sd.hp_gain_value = sd.sp_drain_type = 0;
		memset(sd.subsize,0,sizeof(sd.subsize));
		memset(sd.unequip_losehp,0,sizeof(sd.unequip_losehp));
		memset(sd.unequip_losesp,0,sizeof(sd.unequip_losesp));
		memset(sd.subrace2,0,sizeof(sd.subrace2));
		memset(sd.expaddrace,0,sizeof(sd.expaddrace));
		memset(sd.sp_gain_race,0,sizeof(sd.sp_gain_race));
		sd.setitem_hash = 0;

		for(i=0;i<10;++i)
		{	//We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
			current_equip_item_index = index = sd.equip_index[i]; 
			if(index >= MAX_INVENTORY)
				continue;
			if(i == 9 && sd.equip_index[8] == index)
				continue;
			if(i == 5 && sd.equip_index[4] == index)
				continue;
			if(i == 6 && (sd.equip_index[5] == index || sd.equip_index[4] == index))
				continue;

			if(sd.inventory_data[index])
			{
				if(sd.inventory_data[index]->type == 4)
				{	// Weapon cards
					if(sd.status.inventory[index].card[0]!=0x00ff && sd.status.inventory[index].card[0]!=0x00fe && sd.status.inventory[index].card[0]!=0xff00)
					{
						for(j=0;j<sd.inventory_data[index]->flag.slot;++j)
						{	// ƒJ?ƒh
							int c=sd.status.inventory[index].card[j];
							if(c>0){
								if(i==8 && sd.status.inventory[index].equip==0x20)
									sd.state.lr_flag = 1;
								CScriptEngine::run(itemdb_equipscript(c),0,sd.block_list::id,0);
								sd.state.lr_flag = 0;
							}
						}
					}
				}
				else if(sd.inventory_data[index]->type==5)
				{	// Non-weapon equipment cards
					if(sd.status.inventory[index].card[0]!=0x00ff && sd.status.inventory[index].card[0]!=0x00fe && sd.status.inventory[index].card[0]!=0xff00)
					{
						for(j=0;j<sd.inventory_data[index]->flag.slot;++j)
						{	// ƒJ?ƒh
							int c=sd.status.inventory[index].card[j];
							if(c>0)
							{
								CScriptEngine::run(itemdb_equipscript(c),0,sd.block_list::id,0);
							}
						}
					}
				}
			}
		}
		wele = sd.right_weapon.atk_ele;
		wele_ = sd.left_weapon.atk_ele;
		def_ele = sd.def_ele;

		if(sd.status.pet_id > 0) { // Pet
			struct pet_data *pd=sd.pd;
			if((pd && config.pet_status_support) && (!config.pet_equip_required || pd->pet.equip_id > 0))
			{
				if(pd->pet.intimate > 0 && pd->state.skillbonus == 1 && pd->bonus)
				{	//Skotlex: Readjusted for pets
					pc_bonus(sd,pd->bonus->type, pd->bonus->val);
				}
				pele = sd.right_weapon.atk_ele;
				pdef_ele = sd.def_ele;
				sd.right_weapon.atk_ele = sd.def_ele = 0;
			}
		}
		memcpy(sd.paramcard,sd.parame,sizeof(sd.paramcard));

		// ?”õ•i‚É‚æ‚éƒXƒe?ƒ^ƒX?‰»‚Í‚±‚±‚Å?s
		for(i=0;i<10;++i) {
			current_equip_item_index = index = sd.equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
			if(index >=MAX_INVENTORY)
				continue;
			if(i == 9 && sd.equip_index[8] == index)
				continue;
			if(i == 5 && sd.equip_index[4] == index)
				continue;
			if(i == 6 && (sd.equip_index[5] == index || sd.equip_index[4] == index))
				continue;
			if(sd.inventory_data[index]) {
				sd.def += sd.inventory_data[index]->def;
				
				if(sd.inventory_data[index]->type == 4)
				{
					int r,wlv = sd.inventory_data[index]->wlv;
					if (wlv >= MAX_REFINE_BONUS) 
						wlv = MAX_REFINE_BONUS - 1;

					sd.state.lr_flag = (i == 8 && sd.status.inventory[index].equip == 0x20);
					if( sd.state.lr_flag )
					{	// Left-hand weapon
						sd.left_weapon.watk += sd.inventory_data[index]->atk;
						sd.left_weapon.watk2 = (r=sd.status.inventory[index].refine)*	// ¸?U?—Í
							refinebonus[wlv][0];
						if( (r-=refinebonus[wlv][2])>0 )	// ‰ß?¸?ƒ{?ƒiƒX
							sd.left_weapon.overrefine = r*refinebonus[wlv][1];

						if(sd.status.inventory[index].card[0]==0x00ff)
						{	// Forged weapon
							sd.left_weapon.star = (sd.status.inventory[index].card[1]>>8);	// ¯‚Ì‚©‚¯‚ç
							if(sd.left_weapon.star >= 15) sd.left_weapon.star = 50; // 3 Star Crumbs now give +50 dmg
							wele_= (sd.status.inventory[index].card[1]&0x0f);	// ? «
							sd.left_weapon.fameflag = chrif_istop10fame( basics::MakeDWord(sd.status.inventory[index].card[2],sd.status.inventory[index].card[3]), FAME_SMITH);
						}
						sd.attackrange_ += sd.inventory_data[index]->range;
					}
					else
					{	// Right-hand weapon
						sd.right_weapon.watk += sd.inventory_data[index]->atk;
						sd.right_weapon.watk2 += (r=sd.status.inventory[index].refine)*	// ¸?U?—Í
							refinebonus[wlv][0];
						if( (r-=refinebonus[wlv][2])>0 )	// ‰ß?¸?ƒ{?ƒiƒX
							sd.right_weapon.overrefine += r*refinebonus[wlv][1];

						if(sd.status.inventory[index].card[0]==0x00ff)
						{	// Forged weapon
							sd.right_weapon.star += (sd.status.inventory[index].card[1]>>8);	// ¯‚Ì‚©‚¯‚ç
							if(sd.right_weapon.star >= 15) sd.right_weapon.star = 50; // 3 Star Crumbs now give +50 dmg
							wele = (sd.status.inventory[index].card[1]&0x0f);	// ? «
							sd.right_weapon.fameflag = chrif_istop10fame( basics::MakeDWord(sd.status.inventory[index].card[2],sd.status.inventory[index].card[3]), FAME_SMITH);
						}
						sd.attackrange += sd.inventory_data[index]->range;
					}
					if( sd.inventory_data[index]->equip_script && sd.inventory_data[index]->equip_script->script)
						CScriptEngine::run(sd.inventory_data[index]->equip_script->script,0,sd.block_list::id,0);
					sd.state.lr_flag = 0;
				}
				else if(sd.inventory_data[index]->type == 5)
				{
					sd.right_weapon.watk += sd.inventory_data[index]->atk;
					refinedef += sd.status.inventory[index].refine*refinebonus[0][0];
					if( sd.inventory_data[index]->equip_script && sd.inventory_data[index]->equip_script->script)
						CScriptEngine::run(sd.inventory_data[index]->equip_script->script,0,sd.block_list::id,0);
				}
			}
		}

		if(sd.equip_index[10] < MAX_INVENTORY)
		{	// –î
			index = sd.equip_index[10];
			if(sd.inventory_data[index] && sd.inventory_data[index]->equip_script && sd.inventory_data[index]->equip_script->script)
			{	// Arrows
				sd.state.lr_flag = 2;
				CScriptEngine::run(sd.inventory_data[index]->equip_script->script,0,sd.block_list::id,0);
				sd.state.lr_flag = 0;
				sd.arrow_atk += sd.inventory_data[index]->atk;
			}
		}
		sd.def += (refinedef+50)/100;

		if(sd.attackrange < 1) sd.attackrange = 1;
		if(sd.attackrange_ < 1) sd.attackrange_ = 1;
		if(sd.attackrange < sd.attackrange_)
			sd.attackrange = sd.attackrange_;
		if(sd.status.weapon == 11)
			sd.attackrange += sd.arrow_range;
		if(wele > 0)
			sd.right_weapon.atk_ele = wele;
		if(wele_ > 0)
			sd.left_weapon.atk_ele = wele_;
		if(def_ele > 0)
			sd.def_ele = def_ele;
		if(config.pet_status_support) {
			if(pele > 0 && !sd.right_weapon.atk_ele)
				sd.right_weapon.atk_ele = pele;
			if(pdef_ele > 0 && !sd.def_ele)
				sd.def_ele = pdef_ele;
		}
		sd.double_rate += sd.double_add_rate;
		sd.perfect_hit += sd.perfect_hit_add;
		sd.get_zeny_num += sd.get_zeny_add_num;
		sd.splash_range += sd.splash_add_range;
		if(sd.speed_add_rate != 100)	
			sd.speed_rate = sd.speed_rate*sd.speed_add_rate/100;
		if(sd.aspd_add_rate != 100)
			sd.aspd_rate = sd.aspd_rate*sd.aspd_add_rate/100;

		// •ŠíATKƒTƒCƒY•â³ (‰Eè)
		sd.right_weapon.atkmods[0] = atkmods[0][sd.weapontype1];
		sd.right_weapon.atkmods[1] = atkmods[1][sd.weapontype1];
		sd.right_weapon.atkmods[2] = atkmods[2][sd.weapontype1];
		//•ŠíATKƒTƒCƒY•â³ (¶è)
		sd.left_weapon.atkmods[0] = atkmods[0][sd.weapontype2];
		sd.left_weapon.atkmods[1] = atkmods[1][sd.weapontype2];
		sd.left_weapon.atkmods[2] = atkmods[2][sd.weapontype2];

		// jobƒ{?ƒiƒX•ª
		for(i=0;i<sd.status.job_level && i<MAX_LEVEL;++i){
			if(job_bonus[s_class.upper][s_class.job][i])
				sd.paramb[job_bonus[s_class.upper][s_class.job][i]-1]++;
		}

		if( (skill=pc_checkskill(sd,MC_INCCARRY))>0 )	// skill can be used with an item now, thanks to orn [Valaris]
			sd.max_weight += skill*2000;

		if( (skill=pc_checkskill(sd,AC_OWL))>0 )	// ‚Ó‚­‚ë‚¤‚Ì–Ú
			sd.paramb[4] += skill;

		if((skill=pc_checkskill(sd,BS_HILTBINDING))>0) {   // Hilt binding gives +1 str +4 atk
			sd.paramb[0] ++;
			sd.base_atk += 4;
		}
		if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){ // Dragonology increases +1 int every 2 levels
			sd.paramb[3] += (skill+1)/2;
		}
		if((skill=pc_checkskill(sd,HP_MANARECHARGE))>0 ){
			sd.dsprate -= 4 * skill;
		}

		// ƒXƒe?ƒ^ƒX?‰»‚É‚æ‚éŠî–{ƒpƒ‰ƒ?ƒ^•â³

		{
			if(sd.sc_data[SC_INCALLSTATUS].timer!=-1){
				sd.paramb[0]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
				sd.paramb[1]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
				sd.paramb[2]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
				sd.paramb[3]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
				sd.paramb[4]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
				sd.paramb[5]+= sd.sc_data[SC_INCALLSTATUS].val1.num;
			}
			if(sd.sc_data[SC_INCSTR].timer!=-1)
				sd.paramb[0] += sd.sc_data[SC_INCSTR].val1.num;
			if(sd.sc_data[SC_INCAGI].timer!=-1)
				sd.paramb[1] += sd.sc_data[SC_INCAGI].val1.num;
			if(sd.sc_data[SC_INCVIT].timer!=-1)
				sd.paramb[2] += sd.sc_data[SC_INCVIT].val1.num;
			if(sd.sc_data[SC_INCINT].timer!=-1)
				sd.paramb[3] += sd.sc_data[SC_INCINT].val1.num;
			if(sd.sc_data[SC_INCDEX].timer!=-1)
				sd.paramb[4] += sd.sc_data[SC_INCDEX].val1.num;
			if(sd.sc_data[SC_INCLUK].timer!=-1)
				sd.paramb[5] += sd.sc_data[SC_INCLUK].val1.num;
			if(sd.sc_data[SC_CONCENTRATE].timer!=-1 && sd.sc_data[SC_QUAGMIRE].timer == -1){	// W’†—ÍŒüã
				sd.paramb[1]+= (sd.status.agi+sd.paramb[1]+sd.parame[1]-sd.paramcard[1])*(2+sd.sc_data[SC_CONCENTRATE].val1.num)/100;
				sd.paramb[4]+= (sd.status.dex+sd.paramb[4]+sd.parame[4]-sd.paramcard[4])*(2+sd.sc_data[SC_CONCENTRATE].val1.num)/100;
			}
			if(sd.sc_data[SC_INCREASEAGI].timer!=-1){	// ‘¬“x?‰Á
				sd.paramb[1]+= 2 + sd.sc_data[SC_INCREASEAGI].val1.num;
				sd.speed_rate -= 25;
			}
			if(sd.sc_data[SC_DECREASEAGI].timer!=-1) {	// ‘¬“xŒ¸­(agi‚Íbattle.c‚Å)
				sd.paramb[1] -= 2 + sd.sc_data[SC_DECREASEAGI].val1.num;	// reduce agility [celest]
				sd.speed_rate += 25;
			}
			if(sd.sc_data[SC_CLOAKING].timer!=-1) {
				sd.critical_rate += 100; // critical increases
				sd.speed = sd.speed * (sd.sc_data[SC_CLOAKING].val3.num-sd.sc_data[SC_CLOAKING].val1.num*3) /100;
			}
			if(sd.sc_data[SC_CHASEWALK].timer!=-1)
				sd.speed = sd.speed * sd.sc_data[SC_CHASEWALK].val3.num /100; // slow down by chasewalk
			if(sd.sc_data[SC_SLOWDOWN].timer!=-1)
				sd.speed_rate += 50;
			if(sd.sc_data[SC_SPEEDUP1].timer!=-1)
				sd.speed_rate -= 50;
			else if(sd.sc_data[SC_SPEEDUP0].timer!=-1 && sd.sc_data[SC_INCREASEAGI].timer==-1)
				sd.speed_rate -= 25;
			if(sd.sc_data[SC_BLESSING].timer!=-1){	// ƒuƒŒƒbƒVƒ“ƒO
				sd.paramb[0]+= sd.sc_data[SC_BLESSING].val1.num;
				sd.paramb[3]+= sd.sc_data[SC_BLESSING].val1.num;
				sd.paramb[4]+= sd.sc_data[SC_BLESSING].val1.num;
			}
			if(sd.sc_data[SC_GLORIA].timer!=-1)	// ƒOƒƒŠƒA
				sd.paramb[5]+= 30;
			if(sd.sc_data[SC_LOUD].timer!=-1)	// ƒ‰ƒEƒhƒ{ƒCƒX
				sd.paramb[0]+= 4;
			if(sd.sc_data[SC_QUAGMIRE].timer!=-1){	// ƒNƒ@ƒOƒ}ƒCƒA
				sd.paramb[1]-= sd.sc_data[SC_QUAGMIRE].val1.num*5;
				sd.paramb[4]-= sd.sc_data[SC_QUAGMIRE].val1.num*5;
				sd.speed_rate += 50;
			}
			if(sd.sc_data[SC_TRUESIGHT].timer!=-1){	// ƒgƒDƒ‹?ƒTƒCƒg
				sd.paramb[0]+= 5;
				sd.paramb[1]+= 5;
				sd.paramb[2]+= 5;
				sd.paramb[3]+= 5;
				sd.paramb[4]+= 5;
				sd.paramb[5]+= 5;
			}
			if(sd.sc_data[SC_MARIONETTE].timer!=-1){
				// skip partner checking -- should be handled in status_change_timer
				//map_session_data *psd = map_session_data::from_blid(sd.sc_data[SC_MARIONETTE2].val3);
				//if (psd) {	// if partner is found
					sd.paramb[0]-= sd.status.str/2;	// bonuses not included
					sd.paramb[1]-= sd.status.agi/2;
					sd.paramb[2]-= sd.status.vit/2;
					sd.paramb[3]-= sd.status.int_/2;
					sd.paramb[4]-= sd.status.dex/2;
					sd.paramb[5]-= sd.status.luk/2;
				//}
			}
			else if(sd.sc_data[SC_MARIONETTE2].timer!=-1){
				map_session_data *psd = map_session_data::from_blid(sd.sc_data[SC_MARIONETTE2].val3.num);
				if (psd) {	// if partner is found
					sd.paramb[0] += sd.status.str+psd->status.str/2 > 99 ? 99-sd.status.str : psd->status.str/2;
					sd.paramb[1] += sd.status.agi+psd->status.agi/2 > 99 ? 99-sd.status.agi : psd->status.agi/2;
					sd.paramb[2] += sd.status.vit+psd->status.vit/2 > 99 ? 99-sd.status.vit : psd->status.vit/2;
					sd.paramb[3] += sd.status.int_+psd->status.int_/2 > 99 ? 99-sd.status.int_ : psd->status.int_/2;
					sd.paramb[4] += sd.status.dex+psd->status.dex/2 > 99 ? 99-sd.status.dex : psd->status.dex/2;
					sd.paramb[5] += sd.status.luk+psd->status.luk/2 > 99 ? 99-sd.status.luk : psd->status.luk/2;
				}
			}
			if(sd.sc_data[SC_GOSPEL].timer!=-1 && sd.sc_data[SC_GOSPEL].val4.num == BCT_PARTY){
				if (sd.sc_data[SC_GOSPEL].val3.num == 6) {
					sd.paramb[0]+= 2;
					sd.paramb[1]+= 2;
					sd.paramb[2]+= 2;
					sd.paramb[3]+= 2;
					sd.paramb[4]+= 2;
					sd.paramb[5]+= 2;
				}
			}
			// New guild skills - Celest
			if (sd.sc_data[SC_BATTLEORDERS].timer != -1) {
				sd.paramb[0]+= 5;
				sd.paramb[3]+= 5;
				sd.paramb[4]+= 5;
			}
			if (sd.sc_data[SC_GUILDAURA].timer != -1) {
				int guildflag = sd.sc_data[SC_GUILDAURA].val4.num;
				for (i = 16; i >= 0; i -= 4) {
					skill = guildflag >> i;
					switch (i) {
						// guild skills
						case 16: sd.paramb[0] += skill; break;
						case 12: sd.paramb[2] += skill; break;
						case 8: sd.paramb[1] += skill; break;
						case 4: sd.paramb[4] += skill; break;
						case 0:
							// custom stats, since there's no info on how much it actually gives ^^; [Celest]
							if (skill) {
								sd.hit += 10;
								sd.flee += 10;
							}
							break;
						default:
							break;
					}
					guildflag ^= skill << i;
				}
			}
			if(sd.sc_data[SC_ENDURE].timer!=-1)
				sd.mdef += sd.sc_data[SC_ENDURE].val1.num;
		}

		// If Super Novice / Super Baby Never Died till Job70 they get bonus: AllStats +10
		if(s_class.job == 23 && sd.die_counter == 0 && sd.status.job_level >= 70){
			sd.paramb[0]+= 10;
			sd.paramb[1]+= 10;
			sd.paramb[2]+= 10;
			sd.paramb[3]+= 10;
			sd.paramb[4]+= 10;
			sd.paramb[5]+= 10;
		}
		sd.paramc[0]=sd.status.str+sd.paramb[0]+sd.parame[0];
		sd.paramc[1]=sd.status.agi+sd.paramb[1]+sd.parame[1];
		sd.paramc[2]=sd.status.vit+sd.paramb[2]+sd.parame[2];
		sd.paramc[3]=sd.status.int_+sd.paramb[3]+sd.parame[3];
		sd.paramc[4]=sd.status.dex+sd.paramb[4]+sd.parame[4];
		sd.paramc[5]=sd.status.luk+sd.paramb[5]+sd.parame[5];
		for(i=0;i<6;++i)
			if(sd.paramc[i] < 0) sd.paramc[i] = 0;

		if (sd.sc_data[SC_CURSE].timer!=-1)
			sd.paramc[5] = 0;

		if(sd.status.weapon == 11 || sd.status.weapon == 13 || sd.status.weapon == 14) {
			str = sd.paramc[4];
			dex = sd.paramc[0];
		}
		else {
			str = sd.paramc[0];
			dex = sd.paramc[4];
		}
		dstr = str/10;
		sd.base_atk += str + dstr*dstr + dex/5 + sd.paramc[5]/5;
		sd.matk1 += sd.paramc[3]+(sd.paramc[3]/5)*(sd.paramc[3]/5);
		sd.matk2 += sd.paramc[3]+(sd.paramc[3]/7)*(sd.paramc[3]/7);
		if(sd.matk1 < sd.matk2) {
			int temp = sd.matk2;
			sd.matk2 = sd.matk1;
			sd.matk1 = temp;
		}
		sd.hit += sd.paramc[4] + sd.status.base_level;
		sd.flee += sd.paramc[1] + sd.status.base_level;
		if( maps[sd.block_list::m].flag.gvg ) sd.flee = (sd.flee>20)?sd.flee-20:0; //20 flee penalty on GVG grounds [Skotlex]
		sd.def2 += (unsigned short)sd.paramc[2];
		sd.mdef2 += (unsigned short)sd.paramc[3];
		sd.flee2 += sd.paramc[5]+10;
		sd.critical += (sd.paramc[5]*3)+10;

		if(sd.base_atk < 1)
			sd.base_atk = 1;
		if(sd.critical_rate != 100)
			sd.critical = (sd.critical*sd.critical_rate)/100;
		if(sd.critical < 10) sd.critical = 10;
		if(sd.hit_rate != 100)
			sd.hit = (sd.hit*sd.hit_rate)/100;
		if(sd.hit < 1) sd.hit = 1;
		if(sd.flee_rate != 100)
			sd.flee = (sd.flee*sd.flee_rate)/100;
		if(sd.flee < 1) sd.flee = 1;
		if(sd.flee2_rate != 100)
			sd.flee2 = (sd.flee2*sd.flee2_rate)/100;
		if(sd.flee2 < 10) sd.flee2 = 10;
		if(sd.def_rate != 100)
			sd.def = (sd.def*sd.def_rate)/100;
		if(sd.def2_rate != 100)
			sd.def2 = (sd.def2*sd.def2_rate)/100;
		if(sd.def2 < 1) sd.def2 = 1;
		if(sd.mdef_rate != 100)
			sd.mdef = (sd.mdef*sd.mdef_rate)/100;
		if(sd.mdef2_rate != 100)
			sd.mdef2 = (sd.mdef2*sd.mdef2_rate)/100;
		if(sd.mdef2 < 1) sd.mdef2 = 1;

		// “ñ“—¬ ASPD C³
		if (sd.status.weapon <= 16)
		{
			int ofs = (sd.paramc[1]*4+sd.paramc[4])*aspd_base[s_class.job][sd.status.weapon]/1000;
			sd.aspd += aspd_base[s_class.job][sd.status.weapon];
			if( sd.aspd>10+ofs )
				sd.aspd-=ofs;
			else
				sd.aspd = 10; // minimum	
		}
		else
		{
			int ofs = 
				(
				 (-aspd_base[s_class.job][sd.weapontype1]+(sd.paramc[1]*4+sd.paramc[4])*aspd_base[s_class.job][sd.weapontype1]/1000) +
				 (-aspd_base[s_class.job][sd.weapontype2]+(sd.paramc[1]*4+sd.paramc[4])*aspd_base[s_class.job][sd.weapontype2]/1000)
				) * 140 / 200;

			if( sd.aspd>10+ofs )
				sd.aspd-=ofs;
			else
				sd.aspd = 10; // minimum	

		}

		aspd_rate = sd.aspd_rate;

		//U?‘¬“x?‰Á

		if((skill=pc_checkskill(sd,AC_VULTURE))>0){	// ƒƒV‚Ì–Ú
			sd.hit += skill;
			if(sd.status.weapon == 11)
				sd.attackrange += skill;
		}

		if( (skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)	// •Ší?‹†‚Ì–½’†—¦?‰Á
			sd.hit += skill*2;
		if(sd.status.option&2 && (skill = pc_checkskill(sd,RG_TUNNELDRIVE))>0 )	// ƒgƒ“ƒlƒ‹ƒhƒ‰ƒCƒu	// ƒgƒ“ƒlƒ‹ƒhƒ‰ƒCƒu
			sd.speed = sd.speed*100/(20+6*skill);
		if (sd.is_carton() && (skill=pc_checkskill(sd,MC_PUSHCART))>0)	// ƒJ?ƒg‚É‚æ‚é‘¬“x’á‰º
			sd.speed += (10-skill) * DEFAULT_WALK_SPEED/10;
		if (sd.is_riding()) {	// ƒyƒRƒyƒR?‚è‚É‚æ‚é‘¬“x?‰Á
			sd.speed -= DEFAULT_WALK_SPEED/4;
			sd.max_weight += 10000;
		}
		if((skill=pc_checkskill(sd,CR_TRUST))>0) { // ƒtƒFƒCƒX
			sd.status.max_hp += skill*200;
			sd.subele[6] += skill*5;
		}
		if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
			sd.subele[0] += skill;
			sd.subele[3] += skill*4;
		}
		if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0 )
			aspd_rate -= skill/2;

		bl = sd.status.base_level;
		sd.status.max_hp += (3500 + bl*hp_coefficient2[s_class.job] +
			hp_sigma_val[s_class.job][(bl > 0)? bl-1:0])/100 *
			(100 + sd.paramc[2])/100 + (sd.parame[2] - sd.paramcard[2]);

		if (s_class.upper==1) // [MouseJstr]
			sd.status.max_hp = sd.status.max_hp * 130/100;
		else if (s_class.upper==2)
			sd.status.max_hp = sd.status.max_hp * 70/100;

			{
				if(sd.sc_data[SC_INCMHP2].timer!=-1)
					sd.status.max_hp += sd.status.max_hp * sd.sc_data[SC_INCMHP2].val1.num / 100;
				if (sd.sc_data[SC_BERSERK].timer!=-1)	// ƒo?ƒT?ƒN
					sd.status.max_hp = sd.status.max_hp * 3;			
			}
			if(s_class.job == 23 && sd.status.base_level >= 99){
				sd.status.max_hp = sd.status.max_hp + 2000;
		}

		if (sd.hprate <= 0)
			sd.hprate = 1;
		if(sd.hprate!=100)
			sd.status.max_hp = sd.status.max_hp * sd.hprate/100;

		if(sd.status.hp > (long)config.max_hp)
			sd.status.hp = config.max_hp;
		if(sd.status.max_hp > (long)config.max_hp)
			sd.status.max_hp = config.max_hp;
		if(sd.status.max_hp <= 0) sd.status.max_hp = 1; // end

		// Å‘åSPŒvZ
		sd.status.max_sp += ((sp_coefficient[s_class.job] * bl) + 1000)/100 * (100 + sd.paramc[3])/100 + (sd.parame[3] - sd.paramcard[3]);
		if (s_class.upper == 1) // [MouseJstr]
			sd.status.max_sp = sd.status.max_sp * 130/100;
		else if (s_class.upper==2)
			sd.status.max_sp = sd.status.max_sp * 70/100;
		
		if((skill=pc_checkskill(sd,HP_MEDITATIO))>0) // ƒƒfƒBƒeƒCƒeƒBƒI
			sd.status.max_sp += sd.status.max_sp*skill/100;
		if((skill=pc_checkskill(sd,HW_SOULDRAIN))>0) // ƒ\ƒEƒ‹ƒhƒŒƒCƒ“ 
			sd.status.max_sp += sd.status.max_sp*2*skill/100;
		if(sd.sc_data && sd.sc_data[SC_INCMSP2].timer!=-1) {
			sd.status.max_sp += sd.status.max_sp*sd.sc_data[SC_INCMSP2].val1.num/100;
		}

		if (sd.sprate <= 0)
			sd.sprate = 1;
		if(sd.sprate!=100)
			sd.status.max_sp = sd.status.max_sp*sd.sprate/100;
		if(sd.status.max_sp < 0 || sd.status.max_sp > (long)config.max_sp)
			sd.status.max_sp = config.max_sp;

		//©‘R‰ñ•œHP
		sd.nhealhp = 1 + (sd.paramc[2]/5) + (sd.status.max_hp/200);
		if((skill=pc_checkskill(sd,SM_RECOVERY)) > 0) {	// HP‰ñ•œ—ÍŒüã
			sd.nshealhp = skill*5 + (sd.status.max_hp*skill/500);
			if(sd.nshealhp > 0x7fff) sd.nshealhp = 0x7fff;
		}
		//TK_HPTIME regen enable [Dralnu]
		if((skill=pc_checkskill(sd,TK_HPTIME)) > 0 && sd.state.rest == 1) {
			sd.nshealhp = 30*skill;
			if(sd.nshealhp > 0x7fff) sd.nshealhp = 0x7fff;
		}
		//TK_HPTIME regen disable [Dralnu]
		if(pc_checkskill(sd,TK_HPTIME) > 0 && sd.state.rest != 1)
			sd.nshealhp = 0;
		//TK_SPTIME regen [Dralnu]
		if((skill=pc_checkskill(sd,TK_SPTIME)) > 0 && sd.state.rest == 1) {
			sd.nshealsp = skill*3;
			if(sd.nshealsp > 0x7fff) sd.nshealsp = 0x7fff;
		}
		//TK_SPTIME regen disable [Dralnu]
		if(pc_checkskill(sd,TK_SPTIME) > 0 && sd.state.rest != 1)
			sd.nshealsp = 0;
		//©‘R‰ñ•œSP
		sd.nhealsp = 1 + (sd.paramc[3]/6) + (sd.status.max_sp/100);
		if(sd.paramc[3] >= 120)
			sd.nhealsp += ((sd.paramc[3]-120)>>1) + 4;
		if((skill=pc_checkskill(sd,MG_SRECOVERY)) > 0) { // SP‰ñ•œ—ÍŒüã 
			sd.nshealsp = skill*3 + (sd.status.max_sp*skill/500);
			if(sd.nshealsp > 0x7fff) sd.nshealsp = 0x7fff;
		}
		if((skill = pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0) {
			sd.nsshealhp = skill*4 + (sd.status.max_hp*skill/500);
			sd.nsshealsp = skill*2 + (sd.status.max_sp*skill/500);
			if(sd.nsshealhp > 0x7fff) sd.nsshealhp = 0x7fff;
			if(sd.nsshealsp > 0x7fff) sd.nsshealsp = 0x7fff;
		}
		if((skill=pc_checkskill(sd,HP_MEDITATIO)) > 0) {
			// ƒƒfƒBƒeƒCƒeƒBƒI‚ÍSPR‚Å‚Í‚È‚­©‘R‰ñ•œ‚É‚©‚©‚é
			sd.nhealsp += (sd.nhealsp)*3*skill/100;
			if(sd.nhealsp > 0x7fff) sd.nhealsp = 0x7fff;
		}

		if(sd.hprecov_rate != 100) {
			sd.nhealhp = sd.nhealhp*sd.hprecov_rate/100;
			if(sd.nhealhp < 1) sd.nhealhp = 1;
			if(sd.nhealhp > 0x7fff) sd.nhealhp = 0x7fff;
		}
		if(sd.sprecov_rate != 100) {
			sd.nhealsp = sd.nhealsp*sd.sprecov_rate/100;
			if(sd.nhealsp < 1) sd.nhealsp = 1;
			if(sd.nhealsp > 0x7fff) sd.nhealsp = 0x7fff;
		}

		// í‘°‘Ï«i‚±‚ê‚Å‚¢‚¢‚ÌH ƒfƒBƒoƒCƒ“ƒvƒƒeƒNƒVƒ‡ƒ“‚Æ“¯‚¶?—‚ª‚¢‚é‚©‚àj
		if( (skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){ // ƒhƒ‰ƒSƒmƒƒW?
			skill = skill*4;
			sd.right_weapon.addrace[9]+=skill;
			sd.left_weapon.addrace[9]+=skill;
			sd.subrace[9]+=skill;
			sd.magic_addrace[9]+=skill;
			sd.magic_subrace[9]-=skill;
		}

		//Fleeã¸
		if( (skill=pc_checkskill(sd,TF_MISS))>0 ){	// ‰ñ”ğ—¦?‰Á
			sd.flee += skill*((sd.status.class_==12 || sd.status.class_==17 || sd.status.class_==4013 || sd.status.class_==4018) ? 4 : 3);
			if( (sd.status.class_==12 || sd.status.class_==4013) && sd.sc_data[SC_CLOAKING].timer==-1)
				sd.speed -= DEFAULT_WALK_SPEED * skill*3/2/100;
		}
		if( (skill=pc_checkskill(sd,MO_DODGE))>0 )	// Œ©Ø‚è
			sd.flee += (skill*3)>>1;


		if( sd.sc_data[SC_INCFLEE].timer!=-1 )
			sd.flee += (unsigned short)sd.sc_data[SC_INCFLEE].val1.num;
		if( sd.sc_data[SC_INCFLEE2].timer!=-1 )
			sd.flee += (unsigned short)sd.sc_data[SC_INCFLEE2].val1.num;


		// ƒXƒLƒ‹‚âƒXƒe?ƒ^ƒXˆÙí‚É‚æ‚é?‚è‚Ìƒpƒ‰ƒ?ƒ^•â³


		// ATK/DEF?‰»Œ`
		if(sd.sc_data[SC_ANGELUS].timer!=-1)	// ƒGƒ“ƒWƒFƒ‰ƒX
			sd.def2 = sd.def2*(110+5*sd.sc_data[SC_ANGELUS].val1.num)/100;
		if(sd.sc_data[SC_IMPOSITIO].timer!=-1)	{// ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX
			sd.right_weapon.watk += sd.sc_data[SC_IMPOSITIO].val1.num*5;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk += sd.sc_data[SC_IMPOSITIO].val1.num*5;
		}
		if(sd.sc_data[SC_PROVOKE].timer!=-1){	// ƒvƒƒ{ƒbƒN
			sd.def2 -= sd.def2*(5+5*sd.sc_data[SC_PROVOKE].val1.num)/100;
			sd.base_atk += sd.base_atk*(2+3*sd.sc_data[SC_PROVOKE].val1.num)/100;
			sd.right_weapon.watk += sd.right_weapon.watk*(2+3*sd.sc_data[SC_PROVOKE].val1.num)/100;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk += sd.left_weapon.watk*(2+3*sd.sc_data[SC_PROVOKE].val1.num)/100;
		}
		if(sd.sc_data[SC_MINDBREAKER].timer!=-1){	// ƒvƒƒ{ƒbƒN
			sd.mdef2 -= sd.mdef2*(12*sd.sc_data[SC_MINDBREAKER].val1.num)/100;
			sd.matk1 += sd.matk1*(20*sd.sc_data[SC_MINDBREAKER].val1.num)/100;
			sd.matk2 += sd.matk2*(20*sd.sc_data[SC_MINDBREAKER].val1.num)/100;
		}
		if(sd.sc_data[SC_INCMATK2].timer!=-1)	// ƒvƒƒ{ƒbƒN
			sd.matk1 += sd.matk1*sd.sc_data[SC_INCMATK2].val1.num/100;
		
		if(sd.sc_data[SC_POISON].timer!=-1)	// “Å?‘Ô
			sd.def2 = sd.def2*75/100;
		if(sd.sc_data[SC_CURSE].timer!=-1){
			sd.base_atk = sd.base_atk*75/100;
			sd.right_weapon.watk = sd.right_weapon.watk*75/100;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk = sd.left_weapon.watk*75/100;
		}
		if(sd.sc_data[SC_DRUMBATTLE].timer!=-1){	// ?‘¾ŒÛ‚Ì‹¿‚«
			sd.right_weapon.watk += sd.sc_data[SC_DRUMBATTLE].val2.num;
			sd.def  += sd.sc_data[SC_DRUMBATTLE].val3.num;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk += sd.sc_data[SC_DRUMBATTLE].val2.num;
		}
		if(sd.sc_data[SC_NIBELUNGEN].timer!=-1) {	// ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö
			index = sd.equip_index[9];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->wlv == 4)
				sd.right_weapon.watk2 += sd.sc_data[SC_NIBELUNGEN].val2.num;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->wlv == 4)
				sd.left_weapon.watk2 += sd.sc_data[SC_NIBELUNGEN].val2.num;
		}

		if(sd.sc_data[SC_VOLCANO].timer != -1 && sd.def_ele == 3)	// ƒ{ƒ‹ƒP?ƒm
			sd.right_weapon.watk += sd.sc_data[SC_VOLCANO].val3.num;
		if(sd.sc_data[SC_INCATK2].timer != -1)
			sd.right_weapon.watk += sd.right_weapon.watk * sd.sc_data[SC_INCATK2].val1.num / 100;
		if(sd.sc_data[SC_SIGNUMCRUCIS].timer != -1)
			sd.def = sd.def * (100 - sd.sc_data[SC_SIGNUMCRUCIS].val2.num)/100;
		if(sd.sc_data[SC_ETERNALCHAOS].timer != -1)	// ƒGƒ^?ƒiƒ‹ƒJƒIƒX
			sd.def = 0;

		if(sd.sc_data[SC_CONCENTRATION].timer!=-1){ //ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			sd.base_atk = sd.base_atk * (100 + 5*sd.sc_data[SC_CONCENTRATION].val1.num)/100;
			sd.right_weapon.watk = sd.right_weapon.watk * (100 + 5*sd.sc_data[SC_CONCENTRATION].val1.num)/100;
			index = sd.equip_index[8];
			if(index < MAX_INVENTORY && sd.inventory_data[index] && sd.inventory_data[index]->type == 4)
				sd.left_weapon.watk = sd.left_weapon.watk * (100 + 5*sd.sc_data[SC_CONCENTRATION].val1.num)/100;
			sd.def = sd.def * (100 - 5*sd.sc_data[SC_CONCENTRATION].val1.num)/100;
			sd.def2 = sd.def2 * (100 - 5*sd.sc_data[SC_CONCENTRATION].val1.num)/100;
		}

		if(sd.sc_data[SC_MAGICPOWER].timer!=-1){ //–‚–@—Í?•
			sd.matk1 = sd.matk1*(100+5*sd.sc_data[SC_MAGICPOWER].val1.num)/100;
			sd.matk2 = sd.matk2*(100+5*sd.sc_data[SC_MAGICPOWER].val1.num)/100;
		}
		if(sd.sc_data[SC_ATKPOT].timer!=-1)
			sd.right_weapon.watk += sd.sc_data[SC_ATKPOT].val1.num;
		if(sd.sc_data[SC_MATKPOT].timer!=-1){
			sd.matk1 += (unsigned short)sd.sc_data[SC_MATKPOT].val1.num;
			sd.matk2 += (unsigned short)sd.sc_data[SC_MATKPOT].val1.num;
		}

		// ASPD/ˆÚ“®‘¬“x?‰»Œn
		if(sd.sc_data[SC_TWOHANDQUICKEN].timer != -1 && sd.sc_data[SC_QUAGMIRE].timer == -1 && sd.sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
			aspd_rate -= 30;
		if(sd.sc_data[SC_ADRENALINE].timer != -1 && sd.sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
			sd.sc_data[SC_QUAGMIRE].timer == -1 && sd.sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
			if(sd.sc_data[SC_ADRENALINE].val2.num || !config.party_skill_penalty)
				aspd_rate -= 30;
			else
				aspd_rate -= 25;
		}
		if(sd.sc_data[SC_SPEARSQUICKEN].timer != -1 && sd.sc_data[SC_ADRENALINE].timer == -1 &&
			sd.sc_data[SC_TWOHANDQUICKEN].timer == -1 && sd.sc_data[SC_QUAGMIRE].timer == -1 && sd.sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
			aspd_rate -= sd.sc_data[SC_SPEARSQUICKEN].val2.num;
		if(sd.sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
			sd.sc_data[SC_TWOHANDQUICKEN].timer==-1 && sd.sc_data[SC_ADRENALINE].timer==-1 && sd.sc_data[SC_SPEARSQUICKEN].timer==-1 &&
			sd.sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sd.sc_data[SC_ASSNCROS].val1.num+sd.sc_data[SC_ASSNCROS].val2.num+sd.sc_data[SC_ASSNCROS].val3.num;
		if(sd.sc_data[SC_DONTFORGETME].timer!=-1){		// „‚ğ–Y‚ê‚È‚¢‚Å
			aspd_rate += sd.sc_data[SC_DONTFORGETME].val1.num*3 + sd.sc_data[SC_DONTFORGETME].val2.num + (sd.sc_data[SC_DONTFORGETME].val3.num>>16);
			sd.speed_rate += sd.sc_data[SC_DONTFORGETME].val1.num*2 + sd.sc_data[SC_DONTFORGETME].val2.num + (sd.sc_data[SC_DONTFORGETME].val3.num&0xffff);
		}
		if(	sd.sc_data[i=SC_SPEEDPOTION3].timer!=-1 ||
			sd.sc_data[i=SC_SPEEDPOTION2].timer!=-1 ||
			sd.sc_data[i=SC_SPEEDPOTION1].timer!=-1 ||
			sd.sc_data[i=SC_SPEEDPOTION0].timer!=-1)	// ? ‘¬ƒ|?ƒVƒ‡ƒ“
			aspd_rate -= sd.sc_data[i].val2.num;
		if(sd.sc_data[SC_GRAVITATION].timer!=-1)
			aspd_rate += sd.sc_data[SC_GRAVITATION].val2.num;
		if(sd.sc_data[SC_WINDWALK].timer!=-1 && sd.sc_data[SC_INCREASEAGI].timer==-1)	//ƒEƒBƒ“ƒhƒEƒH?ƒNbÍLv*2%Œ¸Z
			sd.speed_rate -= sd.sc_data[SC_WINDWALK].val1.num*4;
		if(sd.sc_data[SC_CARTBOOST].timer!=-1)	// ƒJ?ƒgƒu?ƒXƒg
			sd.speed_rate -= 20;
		if(sd.sc_data[SC_BERSERK].timer!=-1)	//ƒo?ƒT?ƒN’†‚ÍIA‚Æ“¯‚¶‚®‚ç‚¢‘¬‚¢H
			sd.speed_rate -= 25;
		if(sd.sc_data[SC_WEDDING].timer!=-1)	//Œ‹¥’†‚Í?‚­‚Ì‚ª?‚¢
			sd.speed_rate += 100;

		// HIT/FLEE?‰»Œn
		if(sd.sc_data[SC_WHISTLE].timer!=-1){  // Œû“J
			sd.flee += sd.flee * (sd.sc_data[SC_WHISTLE].val1.num
					+sd.sc_data[SC_WHISTLE].val2.num+(sd.sc_data[SC_WHISTLE].val3.num>>16))/100;
			sd.flee2+= (sd.sc_data[SC_WHISTLE].val1.num+sd.sc_data[SC_WHISTLE].val2.num+(sd.sc_data[SC_WHISTLE].val3.num&0xffff)) * 10;
		}
		if(sd.sc_data[SC_HUMMING].timer!=-1)  // ƒnƒ~ƒ“ƒO
			sd.hit += (sd.sc_data[SC_HUMMING].val1.num*2+sd.sc_data[SC_HUMMING].val2.num
					+sd.sc_data[SC_HUMMING].val3.num) * sd.hit/100;
		if(sd.sc_data[SC_VIOLENTGALE].timer != -1 && sd.def_ele == 4)	// ƒoƒCƒIƒŒƒ“ƒgƒQƒCƒ‹
			sd.flee += sd.flee * sd.sc_data[SC_VIOLENTGALE].val3.num / 100;
		if(sd.sc_data[SC_BLIND].timer != -1)
		{	// ˆÃ?
			sd.hit -= sd.hit * 25 / 100;
			sd.flee -= sd.flee * 25 / 100;
		}
		if(sd.sc_data[SC_WINDWALK].timer != -1)
		{	// ƒEƒBƒ“ƒhƒEƒH?ƒN
			sd.flee += sd.flee * sd.sc_data[SC_WINDWALK].val2.num / 100;
		}
		if(sd.sc_data[SC_SPIDERWEB].timer != -1) //ƒXƒpƒCƒ_?ƒEƒFƒu
		{
			sd.flee = sd.flee * 50 / 100;
		}
		if(sd.sc_data[SC_TRUESIGHT].timer != -1) //ƒgƒDƒ‹?ƒTƒCƒg
			sd.hit += 3 * sd.sc_data[SC_TRUESIGHT].val1.num;
		if(sd.sc_data[SC_CONCENTRATION].timer != -1) //ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			sd.hit += sd.hit * 10 * sd.sc_data[SC_CONCENTRATION].val1.num / 100;
		if(sd.sc_data[SC_INCHIT].timer != -1)
			sd.hit += (unsigned short)sd.sc_data[SC_INCHIT].val1.num;
		if(sd.sc_data[SC_INCHIT2].timer != -1)
			sd.hit += sd.hit * sd.sc_data[SC_INCHIT2].val1.num / 100;

		// ‘Ï«
		if(sd.sc_data[SC_SIEGFRIED].timer!=-1){  // •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh
			sd.subele[1] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[2] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[3] += sd.sc_data[SC_SIEGFRIED].val2.num;	// ‰Î
			sd.subele[4] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[5] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[6] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[7] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[8] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
			sd.subele[9] += sd.sc_data[SC_SIEGFRIED].val2.num;	// …
		}
		if(sd.sc_data[SC_PROVIDENCE].timer!=-1){	// ƒvƒƒ”ƒBƒfƒ“ƒX
			sd.subele[6] += sd.sc_data[SC_PROVIDENCE].val2.num;	// ? ¹?«
			sd.subrace[6] += sd.sc_data[SC_PROVIDENCE].val2.num;	// ? ?–‚
		}

		// ‚»‚Ì‘¼
		if(sd.sc_data[SC_APPLEIDUN].timer!=-1){	// ƒCƒhƒDƒ“‚Ì—ÑŒç
			sd.status.max_hp +=
					(5 + sd.sc_data[SC_APPLEIDUN].val1.num * 2 + sd.sc_data[SC_APPLEIDUN].val2.num
					+ sd.sc_data[SC_APPLEIDUN].val3.num / 10) * sd.status.max_hp / 100;
			if(sd.status.max_hp < 0 || sd.status.max_hp > (long)config.max_hp)
				sd.status.max_hp = config.max_hp;
		}
		if(sd.sc_data[SC_DELUGE].timer!=-1 && sd.def_ele==1){	// ƒfƒŠƒ…?ƒW
			sd.status.max_hp += sd.status.max_hp * deluge_eff[sd.sc_data[SC_DELUGE].val1.num-1]/100;
			if(sd.status.max_hp < 0 || sd.status.max_hp > (long)config.max_hp)
				sd.status.max_hp = config.max_hp;
		}
		if(sd.sc_data[SC_SERVICE4U].timer!=-1) {	// ƒT?ƒrƒXƒtƒH?ƒ†?
			sd.status.max_sp += sd.status.max_sp*(10+sd.sc_data[SC_SERVICE4U].val1.num+sd.sc_data[SC_SERVICE4U].val2.num
						+sd.sc_data[SC_SERVICE4U].val3.num)/100;
			if(sd.status.max_sp < 0 || sd.status.max_sp > (long)config.max_sp)
				sd.status.max_sp = config.max_sp;
			sd.dsprate-=(10+sd.sc_data[SC_SERVICE4U].val1.num*3+sd.sc_data[SC_SERVICE4U].val2.num
					+sd.sc_data[SC_SERVICE4U].val3.num);			
		}

		if(sd.sc_data[SC_FORTUNE].timer!=-1)	// K‰^‚ÌƒLƒX
			sd.critical += (10+sd.sc_data[SC_FORTUNE].val1.num+sd.sc_data[SC_FORTUNE].val2.num
						+sd.sc_data[SC_FORTUNE].val3.num)*10;

		if(sd.sc_data[SC_EXPLOSIONSPIRITS].timer!=-1){	// ”š—ô”g“®
			if(s_class.job==23)
				sd.critical += sd.sc_data[SC_EXPLOSIONSPIRITS].val1.num*100;
			else
			sd.critical += sd.sc_data[SC_EXPLOSIONSPIRITS].val2.num;
		}

		if(sd.sc_data[SC_STEELBODY].timer!=-1){	// ‹à„
			sd.def = 90;
			sd.mdef = 90;
			aspd_rate += 25;
			sd.speed_rate += 25;
		}
		if(sd.sc_data[SC_DEFENDER].timer != -1) {
			aspd_rate += 25 - sd.sc_data[SC_DEFENDER].val1.num*5;
			sd.speed += 55 - sd.sc_data[SC_DEFENDER].val1.num*5;
		}
		if(sd.sc_data[SC_ENCPOISON].timer != -1)
			sd.addeff[4] += sd.sc_data[SC_ENCPOISON].val2.num;

		if( sd.sc_data[SC_DANCING].timer!=-1 ){		// ‰‰‘t/ƒ_ƒ“ƒXg—p’†
			int s_rate = 600 - 40 * pc_checkskill(sd,((s_class.job == 19) ? BA_MUSICALLESSON : DC_DANCINGLESSON));
			if (sd.sc_data[SC_LONGING].timer != -1)
				s_rate -= 20 * sd.sc_data[SC_LONGING].val1.num;
			sd.speed = sd.speed * s_rate / 100;
			// is attack speed affected?
			//aspd_rate = 600 - 40 * pc_checkskill(*sd, ((s_class.job == 19) ? BA_MUSICALLESSON : DC_DANCINGLESSON));
			//if (sd.sc_data[SC_LONGING].timer != -1)
			//	aspd_rate -= 20 * sd.sc_data[SC_LONGING].val1.num;
			//sd.speed*=4;
			sd.nhealsp = 0;
			sd.nshealsp = 0;
			sd.nsshealsp = 0;
		}
		if(sd.sc_data[SC_CURSE].timer!=-1)
			sd.speed += 450;

		if(sd.sc_data[SC_TRUESIGHT].timer!=-1) //ƒgƒDƒ‹?ƒTƒCƒg
			sd.critical += sd.sc_data[SC_TRUESIGHT].val1.num; // not +10% CRIT but +CRIT!! [Lupus] u can see it in any RO calc stats

		if(sd.sc_data[SC_BERSERK].timer!=-1) {	//All Def/MDef reduced to 0 while in Berserk [DracoRPG]
			sd.def = sd.def2 = 0;
			sd.mdef = sd.mdef2 = 0;
			sd.flee -= sd.flee*50/100;
			aspd_rate -= 30;
		}
		if(sd.sc_data[SC_INCDEF2].timer!=-1)
			sd.def += sd.def * sd.sc_data[SC_INCDEF2].val1.num/100;
		if(sd.sc_data[SC_KEEPING].timer!=-1)
			sd.def = 100;
		if(sd.sc_data[SC_BARRIER].timer!=-1)
			sd.mdef = 100;

		if(sd.sc_data[SC_JOINTBEAT].timer!=-1)
		{	// Random break [DracoRPG]
			switch(sd.sc_data[SC_JOINTBEAT].val2.num) {
			case 0: //Ankle break
				sd.speed_rate += 50;
				break;
			case 1:	//Wrist	break
				sd.speed_rate += 25;
				break;
			case 2:	//Knee break
				sd.speed_rate += 30;
				sd.aspd_rate += 10;
				break;
			case 3:	//Shoulder break
				sd.def2 -= sd.def2*50/100;
				break;
			case 4:	//Waist	break
				sd.def2 -= sd.def2*50/100;
				sd.base_atk -= sd.base_atk*25/100;
				break;
			}
		}
		if(sd.sc_data[SC_GOSPEL].timer!=-1) {
			if (sd.sc_data[SC_GOSPEL].val4.num == BCT_PARTY){
				switch (sd.sc_data[SC_GOSPEL].val3.num)
				{
				case 4:
					sd.status.max_hp += sd.status.max_hp * 25 / 100;
					if(sd.status.max_hp > (long)config.max_hp)
						sd.status.max_hp = config.max_hp;
					break;
				case 5:
					sd.status.max_sp += sd.status.max_sp * 25 / 100;
					if(sd.status.max_sp > (long)config.max_sp)
						sd.status.max_sp = config.max_sp;
					break;
				case 11:
					sd.def += sd.def * 25 / 100;
					sd.def2 += sd.def2 * 25 / 100;
					break;
				case 12:
					sd.base_atk += sd.base_atk * 8 / 100;
					break;
				case 13:
					sd.flee += sd.flee * 5 / 100;
					break;
				case 14:
					sd.hit += sd.hit * 5 / 100;
					break;
				}
			} else if (sd.sc_data[SC_GOSPEL].val4.num == BCT_ENEMY){
				switch (sd.sc_data[SC_GOSPEL].val3.num)
				{
					case 5:
						sd.def = 0;
						sd.def2 = 0;
						break;
					case 6:
						sd.base_atk = 0;
						sd.right_weapon.watk = 0;
						sd.right_weapon.watk2 = 0;
						break;
					case 7:
						sd.flee = 0;
						break;
					case 8:
						sd.speed_rate += 75;
						aspd_rate += 75;
						break;
				}
			}
		}

		if (sd.speed_rate <= 0)
			sd.speed_rate = 1;

		if(sd.speed_rate != 100)
			sd.speed = sd.speed*sd.speed_rate/100;

		if(sd.skilltimer != -1 && (skill = pc_checkskill(sd,SA_FREECAST)) > 0) {
			sd.speed = sd.speed*(175 - skill*5)/100;
		}

		if(sd.speed < MAX_WALK_SPEED/config.max_walk_speed)
			sd.speed = MAX_WALK_SPEED/config.max_walk_speed;

		if(sd.is_riding())
			sd.aspd += sd.aspd * 10*(5 - pc_checkskill(sd,KN_CAVALIERMASTERY))/100;
		if(aspd_rate != 100)
			sd.aspd = sd.aspd*aspd_rate/100;
		if(sd.is_riding())							// ‹R•ºC—û
			sd.aspd = sd.aspd*(100 + 10*(5 - pc_checkskill(sd,KN_CAVALIERMASTERY)))/ 100;
		if(sd.aspd < config.max_aspd_interval) sd.aspd = config.max_aspd_interval;
		sd.amotion = sd.aspd;
		if( sd.paramc[1] < 100 )
			sd.dmotion = 800-sd.paramc[1]*4;
		else
			sd.dmotion = 400;
		if(sd.dsprate < 0)
			sd.dsprate = 0;
		if(sd.status.hp>sd.status.max_hp)
			sd.status.hp=sd.status.max_hp;
		if(sd.status.sp>sd.status.max_sp)
			sd.status.sp=sd.status.max_sp;

		if(first&4)
			return 0;
		if(first&3) {
			clif_updatestatus(sd,SP_SPEED);
			clif_updatestatus(sd,SP_MAXHP);
			clif_updatestatus(sd,SP_MAXSP);
			if(first&1) {
				clif_updatestatus(sd,SP_HP);
				clif_updatestatus(sd,SP_SP);
			}
			return 0;
		}

		if(b_class != sd.view_class) {
			clif_changelook(sd,LOOK_BASE,sd.view_class);
#if PACKETVER < 4
			clif_changelook(sd,LOOK_WEAPON,sd.status.weapon);
			clif_changelook(sd,LOOK_SHIELD,sd.status.shield);
#else
			clif_changelook(sd,LOOK_WEAPON,0);
#endif
		//Restoring cloth dye color after the view class changes. [Skotlex]
		if(config.save_clothcolor && sd.status.clothes_color > 0 &&
			(sd.view_class != 22 || !config.wedding_ignorepalette))
				clif_changelook(sd,LOOK_CLOTHES_COLOR,sd.status.clothes_color);
		}

		if( 0!=memcmp(b_skill,sd.status.skill,sizeof(sd.status.skill)) || b_attackrange != sd.attackrange)
			clif_skillinfoblock(sd);	// ƒXƒLƒ‹‘—M

		if(b_speed != sd.speed)
			clif_updatestatus(sd,SP_SPEED);
		if(b_weight != (int)sd.weight)
			clif_updatestatus(sd,SP_WEIGHT);
		if(b_max_weight != (int)sd.max_weight) {
			clif_updatestatus(sd,SP_MAXWEIGHT);
			pc_checkweighticon(sd);
		}
		for(i=0;i<6;++i)
			if(b_paramb[i] + b_parame[i] != sd.paramb[i] + sd.parame[i])
				clif_updatestatus(sd,SP_STR+i);
		if(b_hit != sd.hit)
			clif_updatestatus(sd,SP_HIT);
		if(b_flee != sd.flee)
			clif_updatestatus(sd,SP_FLEE1);
		if(b_aspd != (long)sd.aspd)
			clif_updatestatus(sd,SP_ASPD);
		if(b_watk != sd.right_weapon.watk || b_base_atk != sd.base_atk)
			clif_updatestatus(sd,SP_ATK1);
		if(b_def != sd.def)
			clif_updatestatus(sd,SP_DEF1);
		if(b_watk2 != sd.right_weapon.watk2)
			clif_updatestatus(sd,SP_ATK2);
		if(b_def2 != sd.def2)
			clif_updatestatus(sd,SP_DEF2);
		if(b_flee2 != sd.flee2)
			clif_updatestatus(sd,SP_FLEE2);
		if(b_critical != sd.critical)
			clif_updatestatus(sd,SP_CRITICAL);
		if(b_matk1 != sd.matk1)
			clif_updatestatus(sd,SP_MATK1);
		if(b_matk2 != sd.matk2)
			clif_updatestatus(sd,SP_MATK2);
		if(b_mdef != sd.mdef)
			clif_updatestatus(sd,SP_MDEF1);
		if(b_mdef2 != sd.mdef2)
			clif_updatestatus(sd,SP_MDEF2);
		if(b_attackrange != sd.attackrange)
			clif_updatestatus(sd,SP_ATTACKRANGE);
		if(b_max_hp != sd.status.max_hp)
			clif_updatestatus(sd,SP_MAXHP);
		if(b_max_sp != sd.status.max_sp)
			clif_updatestatus(sd,SP_MAXSP);
		if(b_hp != sd.status.hp)
			clif_updatestatus(sd,SP_HP);
		if(b_sp != sd.status.sp)
			clif_updatestatus(sd,SP_SP);

		//if(sd.status.hp<sd.status.max_hp>>2 && pc_checkskill(*sd,SM_AUTOBERSERK)>0 &&
		if(sd.status.hp<sd.status.max_hp>>2 && sd.sc_data[SC_AUTOBERSERK].timer != -1 &&
			(sd.sc_data[SC_PROVOKE].timer==-1 || sd.sc_data[SC_PROVOKE].val2.num==0 ) && !sd.is_dead() )
			// ƒI?ƒgƒo?ƒT?ƒN?“®
			status_change_start(&sd,SC_PROVOKE,10,1,0,0,0,0);

		calculating--;
		return 0;
	}
	return -1;
}

/*==========================================
 * For quick calculating [Celest]
 *------------------------------------------
 */
int status_calc_speed_old (map_session_data &sd)
{
	int b_speed, skill;
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);

	b_speed = sd.speed;
	sd.speed = DEFAULT_WALK_SPEED ;

	if(sd.sc_data[SC_INCREASEAGI].timer!=-1 && sd.sc_data[SC_QUAGMIRE].timer == -1 && sd.sc_data[SC_DONTFORGETME].timer == -1){	// ‘¬“x?‰Á
		sd.speed -= sd.speed *25/100;
	}
	if(sd.sc_data[SC_DECREASEAGI].timer!=-1) {
		sd.speed = sd.speed *125/100;
	}
	if(sd.sc_data[SC_CLOAKING].timer!=-1) {
		sd.speed = sd.speed * (sd.sc_data[SC_CLOAKING].val3.num-sd.sc_data[SC_CLOAKING].val1.num*3) /100;
	}
	if(sd.sc_data[SC_CHASEWALK].timer!=-1) {
		sd.speed = sd.speed * sd.sc_data[SC_CHASEWALK].val3.num /100;
	}
	if(sd.sc_data[SC_QUAGMIRE].timer!=-1){
		sd.speed = sd.speed*3/2;
	}
	if(sd.sc_data[SC_WINDWALK].timer!=-1 && sd.sc_data[SC_INCREASEAGI].timer==-1) {
		sd.speed -= sd.speed *(sd.sc_data[SC_WINDWALK].val1.num*2)/100;
	}
	if(sd.sc_data[SC_CARTBOOST].timer!=-1) {
		sd.speed -= (DEFAULT_WALK_SPEED * 20)/100;
	}
	if(sd.sc_data[SC_BERSERK].timer!=-1) {
		sd.speed -= sd.speed *25/100;
	}
	if(sd.sc_data[SC_WEDDING].timer!=-1) {
		sd.speed = 2*DEFAULT_WALK_SPEED;
	}
	if(sd.sc_data[SC_DONTFORGETME].timer!=-1){
		sd.speed= sd.speed*(100+sd.sc_data[SC_DONTFORGETME].val1.num*2 + sd.sc_data[SC_DONTFORGETME].val2.num + (sd.sc_data[SC_DONTFORGETME].val3.num&0xffff))/100;
	}
	if(sd.sc_data[SC_STEELBODY].timer!=-1){
		sd.speed = (sd.speed * 125) / 100;
	}
	if(sd.sc_data[SC_DEFENDER].timer != -1) {
		sd.speed = (sd.speed * (155 - sd.sc_data[SC_DEFENDER].val1.num*5)) / 100;
	}
	if( sd.sc_data[SC_DANCING].timer!=-1 ){
		int s_rate = 600 - 40 * pc_checkskill(sd, ((s_class.job == 19) ? BA_MUSICALLESSON : DC_DANCINGLESSON));
		if (sd.sc_data[SC_LONGING].timer != -1)
			s_rate -= 20 * sd.sc_data[SC_LONGING].val1.num;
		sd.speed = sd.speed * s_rate / 100;			
	}
	if(sd.sc_data[SC_CURSE].timer!=-1)
		sd.speed += 450;
	if(sd.sc_data[SC_SLOWDOWN].timer!=-1)
		sd.speed = sd.speed*150/100;
	if(sd.sc_data[SC_SPEEDUP1].timer!=-1)
		sd.speed -= sd.speed*50/100;
	else if(sd.sc_data[SC_SPEEDUP0].timer!=-1 && sd.sc_data[SC_INCREASEAGI].timer==-1)
		sd.speed -= sd.speed*25/100;


	if(sd.status.option&2 && (skill = pc_checkskill(sd,RG_TUNNELDRIVE))>0 )
		sd.speed = sd.speed*100/(20+6*skill);
	if (sd.is_carton() && (skill=pc_checkskill(sd,MC_PUSHCART))>0)
		sd.speed += (10-skill) * DEFAULT_WALK_SPEED/10;
	else if (sd.is_riding()) {
		sd.speed -= DEFAULT_WALK_SPEED/4;
	}
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		if(s_class.job==12)
			sd.speed -= sd.speed *(skill*3/2)/100;

	if(sd.speed_rate != 100)
		sd.speed = sd.speed*sd.speed_rate/100;
	if(sd.speed < DEFAULT_WALK_SPEED/4)
		sd.speed = DEFAULT_WALK_SPEED/4;

	if(sd.skilltimer != -1 && (skill = pc_checkskill(sd,SA_FREECAST)) > 0) {
		sd.speed = sd.speed*(175 - skill*5)/100;
	}

	if(b_speed != sd.speed)
		clif_updatestatus(sd,SP_SPEED);

	return 0;
}
/*==========================================
 * For quick calculating [Celest] Adapted by [Skotlex]
 *------------------------------------------
 */
int status_calc_speed(map_session_data &sd, unsigned short skill_num, unsigned short skill_lv, int start)
{
	// [Skotlex]
	// This function individually changes a character's speed upon a skill change and restores it upon it's ending.
	// Should only be used on non-inclusive skills to avoid exploits.
	// Currently used for freedom of cast
	// and when cloaking changes it's val3 (in which case the new val3 value comes in the level.
	
	int b_speed = sd.speed;
	
	switch (skill_num)
	{
	case SA_FREECAST:
	{
		int val = 175 - skill_lv*5;
		if (start) // do round integers properly
			sd.speed = (sd.speed*val + 100/2)/100;
		else//stop
			sd.speed = (sd.speed*100 + val/2)/val;
printf("speed set to %i (%i)\n",sd.speed, val);

		break;
	}
	case AS_CLOAKING:
		if (start && sd.sc_data[SC_CLOAKING].timer != -1)
		{	//There shouldn't be an "stop" case here.
			//If the previous upgrade was 
			//SPEED_ADD_RATE(3*sd->sc_data[SC_CLOAKING].val1.num -sd->sc_data[SC_CLOAKING].val3);
			//Then just changing val3 should be a net difference of....
			if (3*sd.sc_data[SC_CLOAKING].val1.num != sd.sc_data[SC_CLOAKING].val3.num)	//This reverts the previous value.
				sd.speed = sd.speed * 100 /(sd.sc_data[SC_CLOAKING].val3.num-3*sd.sc_data[SC_CLOAKING].val1.num);
			sd.sc_data[SC_CLOAKING].val3 = skill_lv;
				sd.speed = sd.speed * (sd.sc_data[SC_CLOAKING].val3.num-sd.sc_data[SC_CLOAKING].val1.num*3) /100;
		}
		break;
	}

	if(sd.speed < MAX_WALK_SPEED/config.max_walk_speed)
		sd.speed = MAX_WALK_SPEED/config.max_walk_speed;

	if(b_speed != sd.speed)
		clif_updatestatus(sd,SP_SPEED);

	return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌSpeed(ˆÚ“®‘¬“x)‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 * Speed‚Í¬‚³‚¢‚Ù‚¤‚ªˆÚ“®‘¬“x‚ª‘¬‚¢
 *------------------------------------------
 */
int status_recalc_speed(block_list *bl)
{
	int speed;
	nullpo_retr(1000, bl);
	if(*bl==BL_PC)
	{
		speed = bl->get_sd()->speed;
	}
	else
	{
		struct status_change *sc_data=status_get_sc_data(bl);
		speed = 1000;
		if(*bl==BL_MOB)
		{
			mob_data *md=bl->get_md();
			speed = md->speed;
			if(config.mobs_level_up) // increase from mobs leveling up [Valaris]
				speed-=md->level - mob_db[md->class_].lv;
		}
		else if(*bl==BL_PET)
		{	
			pet_data *pd = bl->get_pd();
			speed = pd->petDB.speed;
		}
		else if(*bl==BL_HOM)
		{
			homun_data *hd = bl->get_hd();
			if(hd) 
				speed = hd->msd.speed;
		}
		else if(*bl==BL_NPC)
		{
			npc_data *nd = bl->get_nd();
			speed = nd->speed;
		}

		if(sc_data)
		{
			//‘¬“x‘‰Á‚Í25%Œ¸Z
			if(sc_data[SC_INCREASEAGI].timer!=-1)
				speed -= speed*25/100;
			//ƒEƒBƒ“ƒhƒEƒH[ƒN‚ÍLv*2%Œ¸Z
			else if(sc_data[SC_WINDWALK].timer!=-1)
				speed -= (speed*(sc_data[SC_WINDWALK].val1.num*4))/100;
			//‘¬“xŒ¸­‚Í25%‰ÁZ
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				speed = speed*125/100;
			//ƒNƒ@ƒOƒ}ƒCƒA‚Í50%‰ÁZ
			if(sc_data[SC_QUAGMIRE].timer!=-1)
				speed = speed*3/2;
			//„‚ğ–Y‚ê‚È‚¢‚Åc‚Í‰ÁZ
			if(sc_data[SC_DONTFORGETME].timer!=-1)
				speed = speed*(100+sc_data[SC_DONTFORGETME].val1.num*2 + sc_data[SC_DONTFORGETME].val2.num + (sc_data[SC_DONTFORGETME].val3.num&0xffff))/100;
			//‹à„‚Í25%‰ÁZ
			if(sc_data[SC_STEELBODY].timer!=-1)
				speed = speed*125/100;
			//ƒfƒBƒtƒFƒ“ƒ_[‚Í‰ÁZ
			if(sc_data[SC_DEFENDER].timer!=-1)
				speed = (speed * (155 - sc_data[SC_DEFENDER].val1.num*5)) / 100;
			//—x‚èó‘Ô‚Í4”{’x‚¢
			if(sc_data[SC_DANCING].timer!=-1 )
				speed *= 6;
			//ô‚¢‚Í450‰ÁZ
			if(sc_data[SC_CURSE].timer!=-1)
				speed = speed + 450;
			if(sc_data[SC_SLOWDOWN].timer!=-1)
				speed = speed*150/100;

			if(sc_data[SC_SPEEDUP1].timer!=-1)
				speed -= speed*50/100;
			else if(sc_data[SC_SPEEDUP0].timer!=-1 && sc_data[SC_INCREASEAGI].timer==-1)
				speed -= speed*25/100;

			if(sc_data[SC_GOSPEL].timer!=-1 &&
				sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
				sc_data[SC_GOSPEL].val3.num == 8)
				speed = speed*125/100;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2.num == 0)
					speed = speed*150/100;
				else if (sc_data[SC_JOINTBEAT].val2.num == 2)
					speed = speed*130/100;				
			}
		}
	}

	// map tile dependend reducing of speed

	if( map_getcell(bl->m, bl->x, bl->y, CELL_CHKHOLE) )
	{
		int i,k;
		for(i=-1; i<=1; ++i)
		for(k=-1; k<=1; ++k)
		{
			if( map_getcell(bl->m, bl->x+i, bl->y+k, CELL_CHKHOLE) )
				speed = speed*116/100; // slow down by 16% per surrounding quicksand tile
		}
	}
	else if( map_getcell(bl->m, bl->x, bl->y, CELL_CHKWATER) )
	{
		speed *= 2; // slower in water
	}


	if(speed < 1) speed = 1;
	return speed;
}




/*==========================================
 * ‘ÎÛ‚ÌMHP‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_max_hp() const
{
	const block_list* bl=this;
	// default return 1
	if(*bl==BL_PC)
		return bl->get_sd()->status.max_hp;
	else
	{
		struct status_change *sc_data;		
		int max_hp = 1;
		if(*bl == BL_MOB)
		{
			const mob_data *md = bl->get_md();
			max_hp = md->max_hp;
			if(config.mobs_level_up) // mobs leveling up increase [Valaris]
				max_hp += (md->level - mob_db[md->class_].lv) * bl->get_vit();
		}
		else if(*bl == BL_PET)
		{
			const pet_data *pd = bl->get_pd();
			max_hp = mob_db[pd->pet.class_].max_hp;
		}

		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_APPLEIDUN].timer != -1)
				max_hp += ((5 + sc_data[SC_APPLEIDUN].val1.num * 2 + ((sc_data[SC_APPLEIDUN].val2.num + 1) >> 1)
					+ sc_data[SC_APPLEIDUN].val3.num / 10) * max_hp)/100;
			if(sc_data[SC_GOSPEL].timer != -1 &&
				sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
				sc_data[SC_GOSPEL].val3.num == 4)
				max_hp += max_hp * 25 / 100;
			if(sc_data[SC_INCMHP2].timer!=-1)
				max_hp *= (100+ sc_data[SC_INCMHP2].val1.num)/100;
		}
		if(max_hp < 1) max_hp = 1;
		return max_hp;
	}
}
/*==========================================
 * ‘ÎÛ‚ÌStr‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_str() const
{
	const block_list* bl=this;
	// default return 0
	int str = 0;

	if (*bl == BL_PC)
		return bl->get_sd()->paramc[0];
	else
	{
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);

		if(*bl == BL_MOB)
		{
			const mob_data* md=bl->get_md();
			str = mob_db[md->class_].str;
			if(config.mobs_level_up) // mobs leveling up increase [Valaris]
				str += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				str/=2;
			else if(md->state.size==2)
				str*=2;
		}
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				str = pd->status->str;
			else
				str = mob_db[pd->pet.class_].str;
		}
		if(sc_data) {
			if(sc_data[SC_LOUD].timer != -1)
				str += 4;
			if( sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = bl->get_race();
				if( bl->is_undead() || race == 6)
					str >>= 1;	// ˆ« –‚/•s€
				else str += sc_data[SC_BLESSING].val1.num;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				str += 5;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				str += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCSTR].timer!=-1)
				str += sc_data[SC_INCSTR].val1.num;
		}
	}
	if(str < 0) str = 0;
	return str;
}
/*==========================================
 * ‘ÎÛ‚ÌAgi‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */

int block_list::get_agi() const
{
	const block_list* bl=this;
	// default return 0
	int agi=0;
	if(*bl==BL_PC)
		return bl->get_sd()->paramc[1];
	else
	{
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		if(*bl == BL_MOB)
		{
			const mob_data *md = bl->get_md();
			agi = mob_db[md->class_].agi;
			if(config.mobs_level_up) // increase of mobs leveling up [Valaris]
				agi += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				agi/=2;
			else if(md->state.size==2)
				agi*=2;
		}
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				agi = pd->status->agi;
			else
				agi = mob_db[pd->pet.class_].agi;
		}
		if(sc_data) {
			if(sc_data[SC_INCREASEAGI].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ‘¬“x‘‰Á(PC‚Ípc.c‚Å)
				agi += 2 + sc_data[SC_INCREASEAGI].val1.num;
			if(sc_data[SC_CONCENTRATE].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1)
				agi += agi * (2 + sc_data[SC_CONCENTRATE].val1.num)/100;
			if(sc_data[SC_DECREASEAGI].timer!=-1)	// ‘¬“xŒ¸­
			{
				agi -= 2+sc_data[SC_DECREASEAGI].val1.num;
			}
			if(sc_data[SC_QUAGMIRE].timer!=-1 ) {	// ƒNƒ@ƒOƒ}ƒCƒA
				//agi >>= 1;
				//int agib = agi*(sc_data[SC_QUAGMIRE].val1.num*10)/100;
				//agi -= agib > 50 ? 50 : agib;
				agi -= sc_data[SC_QUAGMIRE].val1.num*10;
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				agi += 5;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				agi += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCAGI].timer!=-1)
				agi += sc_data[SC_INCAGI].val1.num;
		}
	}
	if(agi < 0) agi = 0;
	return agi;
}
/*==========================================
 * ‘ÎÛ‚ÌVit‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_vit() const
{
	const block_list* bl=this;
	// default return 0
	int vit = 0;

	if(*bl == BL_PC && (map_session_data *)bl)
		return bl->get_sd()->paramc[2];
	else {
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		if(*bl == BL_MOB)
		{
			const mob_data *md = bl->get_md();
			vit = mob_db[md->class_].vit;
			if(config.mobs_level_up) // increase from mobs leveling up [Valaris]
				vit += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				vit/=2;
			else if(md->state.size==2)
				vit*=2;
		}	
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				vit = pd->status->vit;
			else
				vit = mob_db[pd->pet.class_].vit;
		}
		if(sc_data) {
			if(sc_data[SC_STRIPARMOR].timer != -1)
				vit = vit*60/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				vit += 5;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				vit += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCVIT].timer!=-1)
				vit += sc_data[SC_INCVIT].val1.num;
		}
	}
	if(vit < 0) vit = 0;
	return vit;
}
/*==========================================
 * ‘ÎÛ‚ÌInt‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_int() const
{
	const block_list* bl=this;
	// default return 0
	int int_=0;

	if(*bl == BL_PC && (map_session_data *)bl)
		return bl->get_sd()->paramc[3];
	else
	{
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);
		if(*bl == BL_MOB)
		{
			const mob_data *md = bl->get_md();
			int_ = mob_db[md->class_].int_;
			if(config.mobs_level_up) // increase from mobs leveling up [Valaris]
				int_ += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				int_/=2;
			else if(md->state.size==2)
				int_*=2;
		}		
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data*pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				int_ = pd->status->int_;
			else
				int_ = mob_db[pd->pet.class_].int_;
		}

		if(sc_data) {
			if(sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = bl->get_race();
				if(bl->is_undead() || race == 6 )
					int_ >>= 1;	// ˆ« –‚/•s€
				else
					int_ += sc_data[SC_BLESSING].val1.num;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_STRIPHELM].timer != -1)
				int_ = int_*60/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				int_ += 5;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				int_ += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCINT].timer!=-1)
				int_ += sc_data[SC_INCINT].val1.num;
		}
	}
	if(int_ < 0) int_ = 0;
	return int_;
}
/*==========================================
 * ‘ÎÛ‚ÌDex‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_dex() const
{
	const block_list* bl=this;
	// default return 0
	int dex = 0;

	if(*bl==BL_PC)
		return bl->get_sd()->paramc[4];
	else
	{
		struct status_change *sc_data = status_get_sc_data(bl);
		if(*bl == BL_MOB)
		{
			const mob_data *md = bl->get_md();
			dex = mob_db[md->class_].dex;
			if(config.mobs_level_up) // increase from mobs leveling up [Valaris]
				dex += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				dex/=2;
			else if(md->state.size==2)
				dex*=2;
		}		
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				dex = pd->status->dex;
			else
				dex = mob_db[pd->pet.class_].dex;
		}

		if(sc_data)
		{
			if(sc_data[SC_CONCENTRATE].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1)
				dex += dex*(2+sc_data[SC_CONCENTRATE].val1.num)/100;
			if(sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = bl->get_race();
				if( bl->is_undead() || race == 6 )
					dex >>= 1;	// ˆ« –‚/•s€
				else dex += sc_data[SC_BLESSING].val1.num;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_QUAGMIRE].timer!=-1)	{ // ƒNƒ@ƒOƒ}ƒCƒA
				// dex >>= 1;
				//int dexb = dex*(sc_data[SC_QUAGMIRE].val1.num*10)/100;
				//dex -= dexb > 50 ? 50 : dexb;
				dex -= sc_data[SC_QUAGMIRE].val1.num*10;
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				dex += 5;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				dex += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCDEX].timer!=-1)
				dex += sc_data[SC_INCDEX].val1.num;
		}
	}
	if(dex < 0) dex = 0;
	return dex;
}
/*==========================================
 * ‘ÎÛ‚ÌLuk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int block_list::get_luk() const
{
	const block_list* bl=this;
	// default return 0
	int luk = 0;

	if(*bl == BL_PC)
		return ((map_session_data *)bl)->paramc[5];
	else
	{
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);
		if( *bl == BL_MOB )
		{
			const mob_data *md = bl->get_md();
			luk = mob_db[md->class_].luk;
			if(config.mobs_level_up) // increase from mobs leveling up [Valaris]
				luk += md->level - mob_db[md->class_].lv;
			if(md->state.size==1) // change for sized monsters [Valaris]
				luk/=2;
			else if(md->state.size==2)
				luk*=2;
		}		
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				luk = pd->status->luk;
			else
				luk = mob_db[pd->pet.class_].luk;
		}
		if(sc_data) {
			if(sc_data[SC_GLORIA].timer!=-1)	// ƒOƒƒŠƒA(PC‚Ípc.c‚Å)
				luk += 30;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				luk += 5;
			if(sc_data[SC_CURSE].timer!=-1)		// ô‚¢
				luk = 0;
			if(sc_data[SC_INCALLSTATUS].timer!=-1)
				luk += sc_data[SC_INCALLSTATUS].val1.num;
			if(sc_data[SC_INCLUK].timer!=-1)
				luk += sc_data[SC_INCLUK].val1.num;
		}
	}
	if(luk < 0) luk = 0;
	return luk;
}

/*==========================================
 * ‘ÎÛ‚ÌFlee‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_flee(block_list *bl)
{
	int flee = 1;
	nullpo_retr(1, bl);

	if(*bl == BL_PC)
		return ((map_session_data *)bl)->flee;
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		flee = bl->get_agi() + bl->get_lv();

		if(sc_data){
			if(sc_data[SC_WHISTLE].timer!=-1)
				flee += flee*(sc_data[SC_WHISTLE].val1.num+sc_data[SC_WHISTLE].val2.num +(sc_data[SC_WHISTLE].val3.num>>16))/100;
			if(sc_data[SC_BLIND].timer!=-1)
				flee -= flee*25/100;
			if(sc_data[SC_WINDWALK].timer!=-1) // ƒEƒBƒ“ƒhƒEƒH[ƒN
				flee += flee*(sc_data[SC_WINDWALK].val2.num)/100;
			if(sc_data[SC_SPIDERWEB].timer!=-1) //ƒXƒpƒCƒ_[ƒEƒFƒu
				flee -= flee*50/100;
			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3.num == 13)
					flee += flee*5/100;
				else if (sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3.num == 7)
					flee = 0;
			}
			if(sc_data[SC_INCFLEE].timer!=-1)
				flee += flee * sc_data[SC_INCFLEE].val1.num / 100;
		}
	}
	if(flee < 1) flee = 1;
	return flee;
}
/*==========================================
 * ‘ÎÛ‚ÌHit‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_hit(block_list *bl)
{
	int hit = 1;
	nullpo_retr(1, bl);
	if (*bl == BL_PC)
		return ((map_session_data *)bl)->hit;
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		hit = bl->get_dex() + bl->get_lv();

		if (sc_data) {
			if (sc_data[SC_HUMMING].timer != -1)
				hit += hit * (sc_data[SC_HUMMING].val1.num * 2 + sc_data[SC_HUMMING].val2.num
					+ sc_data[SC_HUMMING].val3.num) / 100;
			if (sc_data[SC_BLIND].timer != -1)	// ô‚¢
				hit -= hit * 25 / 100;
			if (sc_data[SC_TRUESIGHT].timer != -1)	// ƒgƒDƒ‹[ƒTƒCƒg
				hit += 3 * sc_data[SC_TRUESIGHT].val1.num;
			if (sc_data[SC_CONCENTRATION].timer != -1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				//hit += hit * sc_data[SC_CONCENTRATION].val1.num * 10 / 100;
				hit += 10 * sc_data[SC_CONCENTRATION].val1.num; //+10 hit per level as per updates (?) [Skotlex]
			if (sc_data[SC_GOSPEL].timer != -1 &&
				sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
				sc_data[SC_GOSPEL].val3.num == 14)
				hit += hit * 5 / 100;
			if (sc_data[SC_EXPLOSIONSPIRITS].timer != -1)
				hit += 20 * sc_data[SC_EXPLOSIONSPIRITS].val1.num;
			if (sc_data[SC_INCHIT].timer != -1)
				hit += hit * sc_data[SC_INCHIT].val1.num / 100;
		}
	}
	if(hit < 1) hit = 1;
	return hit;
}
/*==========================================
 * ‘ÎÛ‚ÌŠ®‘S‰ñ”ğ‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_flee2(block_list *bl)
{
	int flee2 = 1;
	nullpo_retr(1, bl);

	if( *bl == BL_PC){
		return ((map_session_data *)bl)->flee2;
	} else {
		struct status_change *sc_data = status_get_sc_data(bl);
		flee2 = bl->get_luk()+1;

		if (sc_data) {
			if (sc_data[SC_WHISTLE].timer!=-1)
				flee2 += (sc_data[SC_WHISTLE].val1.num + sc_data[SC_WHISTLE].val2.num + (sc_data[SC_WHISTLE].val3.num&0xffff)) * 10;
		}
	}
	if (flee2 < 1) flee2 = 1;
	return flee2;
}
/*==========================================
 * ‘ÎÛ‚ÌƒNƒŠƒeƒBƒJƒ‹‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_critical(block_list *bl)
{
	int critical = 1;
	nullpo_retr(1, bl);

	if (*bl == BL_PC) {
		return ((map_session_data *)bl)->critical;
	} else {
		struct status_change *sc_data = status_get_sc_data(bl);
		critical = bl->get_luk()*3 + 1;

		if(sc_data) {
			if (sc_data[SC_FORTUNE].timer != -1)
				critical += 10 + sc_data[SC_FORTUNE].val1.num + sc_data[SC_FORTUNE].val2.num
					+ sc_data[SC_FORTUNE].val3.num * 10;
			if (sc_data[SC_EXPLOSIONSPIRITS].timer != -1)
				critical += sc_data[SC_EXPLOSIONSPIRITS].val2.num;
			if (sc_data[SC_TRUESIGHT].timer != -1) //ƒgƒDƒ‹[ƒTƒCƒg
				critical += sc_data[SC_TRUESIGHT].val1.num;
		}
	}
	if (critical < 1) critical = 1;
	return critical;
}
/*==========================================
 * base_atk‚Ìæ“¾
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_baseatk(block_list *bl)
{
	int batk = 1;
	nullpo_retr(1, bl);
	
	if(*bl==BL_PC)
	{
		map_session_data *sd = bl->get_sd();
		batk = sd->base_atk; //İ’è‚³‚ê‚Ä‚¢‚ébase_atk
		if( sd->status.weapon < 16 )
			batk += sd->weapon_atk[sd->status.weapon];
	}
	else
	{	//‚»‚êˆÈŠO‚È‚ç
		struct status_change *sc_data;
		int str,dstr;
		str = bl->get_str(); //STR
		dstr = str/10;
		batk = dstr*dstr + str; //base_atk‚ğŒvZ‚·‚é
		sc_data = status_get_sc_data(bl);

        if( *bl == BL_MOB )
		{
			mob_data *md = bl->get_md();
			if(md->class_ >=1285 && md->class_ <=1287 && md->guild_id)
			{
				guild *g = guild_search(md->guild_id);
				if(g)
				{
					int skill = guild_checkskill(*g,GD_GUARDUP);
					batk += batk * 10*skill/100; // Strengthen Guardians - custom value +10% ATK / lv
				}
			}
		}


		if(sc_data)
		{	//ó‘ÔˆÙí‚ ‚è
			if(sc_data[SC_PROVOKE].timer!=-1) //PC‚Åƒvƒƒ{ƒbƒN(SM_PROVOKE)ó‘Ô
				batk += batk *(2+3*sc_data[SC_PROVOKE].val1.num)/100; //base_atk‘‰Á
			if(sc_data[SC_CURSE].timer!=-1) //ô‚í‚ê‚Ä‚¢‚½‚ç
				batk -= batk*25/100; //base_atk‚ª25%Œ¸­
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				batk += batk*5*sc_data[SC_CONCENTRATION].val1.num/100;
			if(sc_data[SC_INCATK2].timer!=-1)
				batk += batk*sc_data[SC_INCATK2].val1.num/100;
		}
	}
	if(batk < 1) batk = 1; //base_atk‚ÍÅ’á‚Å‚à1
	return batk;
}
/*==========================================
 * ‘ÎÛ‚ÌAtk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk(block_list *bl)
{
	int atk = 0;
	nullpo_retr(0, bl);

	if(*bl==BL_PC)
	{
		atk = bl->get_sd()->right_weapon.watk;
	}
	else
	{
		struct status_change *sc_data=status_get_sc_data(bl);
		
		if( *bl == BL_MOB )
		{
			mob_data *md = bl->get_md();
			atk = mob_db[md->class_].atk1;
  			if( md->class_ >=1285 && md->class_ <=1287 && md->guild_id)
			{
				struct guild *g = guild_search(md->guild_id);
				if(g)
				{
					int skill = guild_checkskill(*g,GD_GUARDUP);
					atk += atk * 10*skill/100; // Strengthen Guardians - custom value +10% ATK / lv
				}
			}
		}
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			pet_data* pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				atk = pd->status->atk1;
			else
				atk = mob_db[pd->pet.class_].atk1;
		}
		if(sc_data) {
			if(sc_data[SC_PROVOKE].timer!=-1)
				atk += atk * (2+3*sc_data[SC_PROVOKE].val1.num)/100;
			if(sc_data[SC_CURSE].timer!=-1)
				atk -= atk*25/100;
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				atk += atk*(5*sc_data[SC_CONCENTRATION].val1.num)/100;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				atk += (1000*sc_data[SC_EXPLOSIONSPIRITS].val1.num);
			if(sc_data[SC_STRIPWEAPON].timer!=-1)
				atk -= atk*10/100;

			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3.num == 12)
					atk += atk*8/100;
				else if (sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3.num == 6)
					atk = 0;
			}
			if(sc_data[SC_INCATK2].timer!=-1)
				atk += atk * sc_data[SC_INCATK2].val1.num / 100;
		}
	}
	return (atk<0)?0:atk;
}
/*==========================================
 * ‘ÎÛ‚Ì¶èAtk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk_(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_PC){
		int atk=bl->get_sd()->left_weapon.watk;
		return atk;
	}
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk2(block_list *bl)
{
	int atk2=0;
	nullpo_retr(0, bl);
	if(*bl==BL_PC)
	{
		atk2 = bl->get_sd()->right_weapon.watk2;
	}
	else
	{
		struct status_change *sc_data=status_get_sc_data(bl);
		if( *bl==BL_MOB )
		{
			mob_data* md = bl->get_md();
			atk2 = mob_db[md->class_].atk2;
			if(md->class_ >=1285 && md->class_ <=1287 && md->guild_id)
			{
				struct guild *g = guild_search(md->guild_id);
				if(g)
				{
					int skill = guild_checkskill(*g,GD_GUARDUP);
					atk2 += atk2 * 10*skill/100; // Strengthen Guardians - custom value +10% ATK / lv
				}
			}
		}
		else if( *bl==BL_PET )
		{	//<Skotlex> Use pet's stats
			pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				atk2 = pd->status->atk2;
			else
				atk2 = mob_db[pd->pet.class_].atk2;
		}		  
		if(sc_data) {
			if( sc_data[SC_IMPOSITIO].timer!=-1)
				atk2 += sc_data[SC_IMPOSITIO].val1.num*5;
			if( sc_data[SC_PROVOKE].timer!=-1 )
				atk2 += atk2*2*sc_data[SC_PROVOKE].val1.num/100;
			if( sc_data[SC_CURSE].timer!=-1 )
				atk2 -= atk2*25/100;
			if(sc_data[SC_DRUMBATTLE].timer!=-1)
				atk2 += sc_data[SC_DRUMBATTLE].val2.num;
			if(sc_data[SC_NIBELUNGEN].timer!=-1 && (status_get_element(bl)/10) >= 8 )
				atk2 += sc_data[SC_NIBELUNGEN].val2.num;
			if(sc_data[SC_STRIPWEAPON].timer!=-1)
				atk2 = atk2*sc_data[SC_STRIPWEAPON].val2.num/100;
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				atk2 += atk2*(5*sc_data[SC_CONCENTRATION].val1.num)/100;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				atk2 += (1000*sc_data[SC_EXPLOSIONSPIRITS].val1.num);
		}
	}
	return (atk2<0)?0:atk2;
}
/*==========================================
 * ‘ÎÛ‚Ì¶èAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk_2(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_PC)
		return bl->get_sd()->left_weapon.watk2;
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌMAtk1‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_matk1(block_list *bl)
{
	int matk = 0;
	nullpo_retr(0, bl);

	if(*bl == BL_PC && (map_session_data *)bl)
		return ((map_session_data *)bl)->matk1;
	else {
		struct status_change *sc_data;
		int int_ = bl->get_int();
		matk = int_+(int_/5)*(int_/5);

		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				matk += matk*(20*sc_data[SC_MINDBREAKER].val1.num)/100;
			if(sc_data[SC_INCMATK2].timer!=-1)
				matk += matk * sc_data[SC_INCMATK2].val1.num / 100;
		}
	}
	return matk;
}
/*==========================================
 * ‘ÎÛ‚ÌMAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_matk2(block_list *bl)
{
	int matk = 0;
	nullpo_retr(0, bl);

	if(*bl == BL_PC && (map_session_data *)bl)
		return ((map_session_data *)bl)->matk2;
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		int int_ = bl->get_int();
		matk = int_+(int_/7)*(int_/7);

		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				matk += matk*(20*sc_data[SC_MINDBREAKER].val1.num)/100;
			if(sc_data[SC_INCMATK2].timer!=-1)
				matk += matk * sc_data[SC_INCMATK2].val1.num / 100;
		}
	}
	return matk;
}
/*==========================================
 * ‘ÎÛ‚ÌDef‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_def(block_list *bl)
{
	struct status_change *sc_data;
	int def=0,skilltimer=-1,skillid=0;

	nullpo_retr(0, bl);
	sc_data=status_get_sc_data(bl);
	if(*bl==BL_PC)
	{
		map_session_data *sd = bl->get_sd();
		def = sd->def;
		skilltimer = sd->skilltimer;
		skillid = sd->skillid;
	}
	else if(*bl==BL_MOB)
	{
		mob_data *md = bl->get_md();
		def = mob_db[md->class_].def;
		skilltimer = md->skilltimer;
		skillid = md->skillid;
	}
	else if(*bl==BL_PET)
	{
		pet_data *pd = bl->get_pd();
		def = mob_db[pd->pet.class_].def;
	}

	if(sc_data) {
		//“€Œ‹AÎ‰»‚Í‰EƒVƒtƒg
		if(sc_data[SC_FREEZE].timer != -1 || (sc_data[SC_STONE].timer != -1 && sc_data[SC_STONE].val2.num == 0))
			def >>= 1;

		if (*bl != BL_PC) {
			if(sc_data[SC_BERSERK].timer!=-1)
				return 0;
			if(sc_data[SC_ETERNALCHAOS].timer!=-1)
				return 0;

			//ƒL[ƒsƒ“ƒO‚ÍDEF100
			if( sc_data[SC_KEEPING].timer!=-1)
				def = 100;
			//ƒvƒƒ{ƒbƒN‚ÍŒ¸Z
			if(sc_data[SC_PROVOKE].timer!=-1 && *bl != BL_PC) //Provoke doesn't alters player defense.
				def -= def*(5+5*sc_data[SC_PROVOKE].val1.num)/100;
			//í‘¾ŒÛ‚Ì‹¿‚«‚Í‰ÁZ
			if( sc_data[SC_DRUMBATTLE].timer!=-1)
				def += sc_data[SC_DRUMBATTLE].val3.num;
			//“Å‚É‚©‚©‚Á‚Ä‚¢‚é‚ÍŒ¸Z
			if(sc_data[SC_POISON].timer!=-1)
				def = def*75/100;
			//ƒXƒgƒŠƒbƒvƒV[ƒ‹ƒh‚ÍŒ¸Z
			if(sc_data[SC_STRIPSHIELD].timer!=-1)
				def = def*sc_data[SC_STRIPSHIELD].val2.num/100;
			//ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX‚ÍŒ¸Z
			if(sc_data[SC_SIGNUMCRUCIS].timer!=-1)
				def -= def * (10+4*sc_data[SC_SIGNUMCRUCIS].val2.num)/100;
			//‰i‰“‚Ì¬“×‚ÍDEF0‚É‚È‚é
			//ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“‚ÍŒ¸Z
			if( sc_data[SC_CONCENTRATION].timer!=-1)
				def = (def*(100 - 5*sc_data[SC_CONCENTRATION].val1.num))/100;
			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3.num == 11)
					def += def*25/100;
				else if (sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3.num == 5)
					def = 0;
			}
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2.num == 3)
					def -= def*50/100;
				else if (sc_data[SC_JOINTBEAT].val2.num == 4)
					def -= def*25/100;
			}
			if(sc_data[SC_INCDEF2].timer!=-1)
				def += def * sc_data[SC_INCDEF2].val1.num / 100;
		}
	}
	//‰r¥’†‚Í‰r¥Œ¸Z—¦‚ÉŠî‚Ã‚¢‚ÄŒ¸Z
	if(skilltimer != -1) {
		int def_rate = skill_get_castdef(skillid);
		if(def_rate != 0)
			def = (def * (100 - def_rate))/100;
	}
	if(def < 0) def = 0;
	return def;
}
/*==========================================
 * ‘ÎÛ‚ÌMDef‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_mdef(block_list *bl)
{
	struct status_change *sc_data;
	int mdef=0;

	nullpo_retr(0, bl);
	sc_data=status_get_sc_data(bl);
	if(*bl==BL_PC)
		mdef = bl->get_sd()->mdef;
	else if(*bl==BL_MOB)
		mdef = mob_db[bl->get_md()->class_].mdef;
	else if(*bl==BL_PET)
		mdef = mob_db[bl->get_pd()->pet.class_].mdef;

	if(sc_data) {
		if(sc_data[SC_BERSERK].timer!=-1)
			return 0;
		//ƒoƒŠƒA[ó‘Ô‚ÍMDEF100
		if(sc_data[SC_BARRIER].timer != -1)
			mdef = 100;
		//“€Œ‹AÎ‰»‚Í1.25”{
		if(sc_data[SC_FREEZE].timer != -1 || (sc_data[SC_STONE].timer != -1 && sc_data[SC_STONE].val2.num == 0))
			mdef += mdef/4; // == *1.25
		if( sc_data[SC_MINDBREAKER].timer!=-1 && *bl != BL_PC)
			mdef -= mdef*(12*sc_data[SC_MINDBREAKER].val1.num)/100;
	}
	if(mdef < 0) mdef = 0;
	return mdef;
}
/*==========================================
 * ‘ÎÛ‚ÌDef2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_def2(const block_list *bl)
{
	int def2 = 1;
	nullpo_retr(1, bl);
	
	if(*bl==BL_PC)
		return bl->get_sd()->def2;
	else {
		struct status_change *sc_data;

		if(*bl==BL_MOB)
			def2 = mob_db[bl->get_md()->class_].vit;
		else if(*bl==BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd=bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				def2 = pd->status->vit;
			else
				def2 = mob_db[pd->pet.class_].vit;
		}
		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_BERSERK].timer!=-1)
				return 0;
			if(sc_data[SC_ETERNALCHAOS].timer!=-1)
				return 0;

			if(sc_data[SC_ANGELUS].timer != -1)
				def2 += def2*(10+5*sc_data[SC_ANGELUS].val1.num)/100;
			if(sc_data[SC_PROVOKE].timer!=-1)
				def2 -= def2*(5+5*sc_data[SC_PROVOKE].val1.num)/100;
			if(sc_data[SC_POISON].timer!=-1)
				def2 = def2*75/100;
			//ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“‚ÍŒ¸Z
			if( sc_data[SC_CONCENTRATION].timer!=-1)
				def2 -= def2*(5*sc_data[SC_CONCENTRATION].val1.num)/100;
			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4.num == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3.num == 11)
					def2 += def2*25/100;
				else if (sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3.num == 5)
					def2 = 0;
			}
		}
	}
	if(def2 < 0) def2 = 0;
	return def2;
}
/*==========================================
 * ‘ÎÛ‚ÌMDef2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_mdef2(const block_list *bl)
{
	int mdef2 = 0;
	nullpo_retr(0, bl);

	if(*bl == BL_PC)
		return bl->get_sd()->mdef2 + (((map_session_data *)bl)->paramc[2]>>1);
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		if(*bl == BL_MOB)
			mdef2 = mob_db[bl->get_md()->class_].int_ + (mob_db[bl->get_md()->class_].vit>>1);
		else if(*bl == BL_PET)
		{	//<Skotlex> Use pet's stats
			const pet_data *pd = bl->get_pd();
			if (config.pet_lv_rate && pd->status)
				mdef2 = pd->status->int_ +(pd->status->vit>>1);
			else
				mdef2 = mob_db[pd->pet.class_].int_ + (mob_db[pd->pet.class_].vit>>1);
		}
		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				mdef2 -= mdef2*12*sc_data[SC_MINDBREAKER].val1.num/100;
		}
	}
	if(mdef2 < 0) mdef2 = 0;
		return mdef2;
}



/*==========================================
 * ‘ÎÛ‚ÌaDelay(UŒ‚ƒfƒBƒŒƒC)‚ğ•Ô‚·(”Ä—p)
 * aDelay‚Í¬‚³‚¢‚Ù‚¤‚ªUŒ‚‘¬“x‚ª‘¬‚¢
 *------------------------------------------
 */
int status_get_adelay(block_list *bl)
{
	nullpo_retr(4000, bl);
	if(*bl==BL_PC)
		return (bl->get_sd()->aspd<<1);
	else
	{
		struct status_change *sc_data=status_get_sc_data(bl);
		int adelay=4000,aspd_rate = 100,i;
		if(*bl==BL_MOB)
		{
			const mob_data *md = bl->get_md();
			adelay = mob_db[md->class_].adelay;
			if(md->class_ >=1285 && md->class_ <=1287 && md->guild_id)
			{
				const struct guild *g = guild_search(md->guild_id);
				if(g)
				{
					int skill = guild_checkskill(*g,GD_GUARDUP);
					aspd_rate -= 10*skill/100; // Strengthen Guardians - custom value +10% ASPD / lv
				}
			}
		}
		else if(*bl==BL_PET)
			adelay = mob_db[bl->get_pd()->pet.class_].adelay;

		if(sc_data)
		{
			//ƒc[ƒnƒ“ƒhƒNƒCƒbƒPƒ“g—p‚ÅƒNƒ@ƒOƒ}ƒCƒA‚Å‚à„‚ğ–Y‚ê‚È‚¢‚Åc‚Å‚à‚È‚¢‚Í3Š„Œ¸Z
			if(sc_data[SC_TWOHANDQUICKEN].timer != -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
				aspd_rate -= 30;
			//ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…g—p‚Åƒc[ƒnƒ“ƒhƒNƒCƒbƒPƒ“‚Å‚àƒNƒ@ƒOƒ}ƒCƒA‚Å‚à„‚ğ–Y‚ê‚È‚¢‚Åc‚Å‚à‚È‚¢‚Í
			if(sc_data[SC_ADRENALINE].timer != -1 && sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
				sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
				//g—pÒ‚Æƒp[ƒeƒBƒƒ“ƒo[‚ÅŠi·‚ªo‚éİ’è‚Å‚È‚¯‚ê‚Î3Š„Œ¸Z
				if(sc_data[SC_ADRENALINE].val2.num || !config.party_skill_penalty)
					aspd_rate -= 30;
				//‚»‚¤‚Å‚È‚¯‚ê‚Î2.5Š„Œ¸Z
				else
					aspd_rate -= 25;
			}
			//ƒXƒsƒAƒNƒBƒbƒPƒ“‚ÍŒ¸Z
			if(sc_data[SC_SPEARSQUICKEN].timer != -1 && sc_data[SC_ADRENALINE].timer == -1 &&
				sc_data[SC_TWOHANDQUICKEN].timer == -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
				aspd_rate -= sc_data[SC_SPEARSQUICKEN].val2.num;
			//—[“ú‚ÌƒAƒTƒVƒ“ƒNƒƒX‚ÍŒ¸Z
			if(sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
				sc_data[SC_TWOHANDQUICKEN].timer==-1 && sc_data[SC_ADRENALINE].timer==-1 && sc_data[SC_SPEARSQUICKEN].timer==-1 &&
				sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sc_data[SC_ASSNCROS].val1.num+sc_data[SC_ASSNCROS].val2.num+sc_data[SC_ASSNCROS].val3.num;
			//„‚ğ–Y‚ê‚È‚¢‚Åc‚Í‰ÁZ
			if(sc_data[SC_DONTFORGETME].timer!=-1)		// „‚ğ–Y‚ê‚È‚¢‚Å
				aspd_rate += sc_data[SC_DONTFORGETME].val1.num*3 + sc_data[SC_DONTFORGETME].val2.num + (sc_data[SC_DONTFORGETME].val3.num>>16);
			//‹à„25%‰ÁZ
			if(sc_data[SC_STEELBODY].timer!=-1)	// ‹à„
				aspd_rate += 25;
			//‘‘¬ƒ|[ƒVƒ‡ƒ“g—p‚ÍŒ¸Z
			if(	sc_data[i=SC_SPEEDPOTION3].timer!=-1 || sc_data[i=SC_SPEEDPOTION2].timer!=-1 || sc_data[i=SC_SPEEDPOTION1].timer!=-1 || sc_data[i=SC_SPEEDPOTION0].timer!=-1)
				aspd_rate -= sc_data[i].val2.num;
			//ƒfƒBƒtƒFƒ“ƒ_[‚Í‰ÁZ
			if(sc_data[SC_DEFENDER].timer != -1)
				aspd_rate += (25 - sc_data[SC_DEFENDER].val1.num*5);
				//adelay += (1100 - sc_data[SC_DEFENDER].val1.num*100);
			if(sc_data[SC_GOSPEL].timer!=-1 &&
				sc_data[SC_GOSPEL].val4.num == BCT_ENEMY &&
				sc_data[SC_GOSPEL].val3.num == 8)
				aspd_rate += 25;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2.num == 1)
					aspd_rate += 25;
				else if (sc_data[SC_JOINTBEAT].val2.num == 2)
					aspd_rate += 10;
			}
			if(sc_data[SC_GRAVITATION].timer!=-1)
				aspd_rate += sc_data[SC_GRAVITATION].val2.num;
		}
		if(aspd_rate != 100)
			adelay = adelay*aspd_rate/100;
		if(adelay < 2*(int)config.monster_max_aspd_interval) adelay = 2*config.monster_max_aspd_interval;
		return adelay;
	}
}

int status_get_amotion(const block_list *bl)
{
	nullpo_retr(2000, bl);
	if(*bl==BL_PC)
		return bl->get_sd()->amotion;
	else
	{
		struct status_change *sc_data=status_get_sc_data( const_cast<block_list*>(bl) );
		int amotion=2000, aspd_rate = 100, i;
		if(*bl==BL_MOB)
		{
			const mob_data *md = bl->get_md();
			amotion = mob_db[md->class_].amotion;
			if( md->class_ >=1285 && md->class_ <=1287 && md->guild_id)
			{
				struct guild *g = guild_search(md->guild_id);
				if(g)
				{
					int skill = guild_checkskill(*g,GD_GUARDUP);
					aspd_rate -= 10*skill/100; // Strengthen Guardians - custom value +10% ASPD / lv
				}
			}
		}
		else if(*bl==BL_PET)
			amotion = mob_db[bl->get_pd()->pet.class_].amotion;

		if(sc_data) {
			if(sc_data[SC_TWOHANDQUICKEN].timer != -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
				aspd_rate -= 30;
			if(sc_data[SC_ADRENALINE].timer != -1 && sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
				sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
				if(sc_data[SC_ADRENALINE].val2.num || !config.party_skill_penalty)
					aspd_rate -= 30;
				else
					aspd_rate -= 25;
			}
			if(sc_data[SC_SPEARSQUICKEN].timer != -1 && sc_data[SC_ADRENALINE].timer == -1 &&
				sc_data[SC_TWOHANDQUICKEN].timer == -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
				aspd_rate -= sc_data[SC_SPEARSQUICKEN].val2.num;
			if(sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
				sc_data[SC_TWOHANDQUICKEN].timer==-1 && sc_data[SC_ADRENALINE].timer==-1 && sc_data[SC_SPEARSQUICKEN].timer==-1 &&
				sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sc_data[SC_ASSNCROS].val1.num+sc_data[SC_ASSNCROS].val2.num+sc_data[SC_ASSNCROS].val3.num;
			if(sc_data[SC_DONTFORGETME].timer!=-1)		// „‚ğ–Y‚ê‚È‚¢‚Å
				aspd_rate += sc_data[SC_DONTFORGETME].val1.num*3 + sc_data[SC_DONTFORGETME].val2.num + (sc_data[SC_DONTFORGETME].val3.num>>16);
			if(sc_data[SC_STEELBODY].timer!=-1)	// ‹à„
				aspd_rate += 25;
			if(	sc_data[i=SC_SPEEDPOTION3].timer!=-1 || sc_data[i=SC_SPEEDPOTION2].timer!=-1 || sc_data[i=SC_SPEEDPOTION1].timer!=-1 || sc_data[i=SC_SPEEDPOTION0].timer!=-1)
				aspd_rate -= sc_data[i].val2.num;
			if(sc_data[SC_DEFENDER].timer != -1)
				aspd_rate += (25 - sc_data[SC_DEFENDER].val1.num*5);
			if(sc_data[SC_GRAVITATION].timer!=-1)
				aspd_rate += sc_data[SC_GRAVITATION].val2.num;
		}
		if(aspd_rate != 100)
			amotion = amotion*aspd_rate/100;
		if(amotion < (int)config.monster_max_aspd_interval) amotion = config.monster_max_aspd_interval;
		return amotion;
	}
}


int status_get_dmotion(block_list *bl)
{
	int ret=0;
	struct status_change *sc_data;

	nullpo_retr(0, bl);
	sc_data = status_get_sc_data(bl);
	if(*bl==BL_MOB)
	{
		const mob_data *md = bl->get_md();
		ret=mob_db[md->class_].dmotion;
		if(config.monster_damage_delay_rate != 100)
			ret = ret*config.monster_damage_delay_rate/100;
	}
	else if(*bl==BL_PC)
	{
		ret=bl->get_sd()->dmotion;
		if(config.pc_damage_delay_rate != 100)
			ret = ret*config.pc_damage_delay_rate/100;
	}
	else if(*bl==BL_PET)
		ret=mob_db[bl->get_pd()->pet.class_].dmotion;
	else
		return 2000;

	if( !maps[bl->m].flag.gvg && sc_data && 
		(sc_data[SC_ENDURE].timer!=-1 || sc_data[SC_BERSERK].timer!=-1 || sc_data[SC_CONCENTRATION].timer!=-1 ||
		(*bl == BL_PC && ((map_session_data *)bl)->state.infinite_endure)) )
		ret=0;
	else	//Let's apply a random damage modifier to prevent 'stun-lock' abusers. [Skotlex]
		ret = ret*(85+rand()%31)/100;	//Currently: +/- 15%
	return ret;
}

int status_get_element(const block_list *bl)
{
	int ret = 20;
	struct status_change *sc_data;

	nullpo_retr(ret, bl);
	sc_data = status_get_sc_data(bl);
	if(*bl==BL_MOB)	// 10‚ÌˆÊLv*2A‚P‚ÌˆÊ‘®«
		ret=bl->get_md()->def_ele;
	else if(*bl==BL_PC)
		ret=20+bl->get_sd()->def_ele;	// –hŒä‘®«Lv1
	else if(*bl==BL_PET)
		ret = mob_db[bl->get_pd()->pet.class_].element;

	if(sc_data) {
		if( sc_data[SC_BENEDICTIO].timer!=-1 )	// ¹‘Ì~•Ÿ
			ret=26;
		if( sc_data[SC_FREEZE].timer!=-1 )	// “€Œ‹
			ret=21;
		if( sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2.num==0)
			ret=22;
	}
	return ret;
}

int status_get_attack_element(block_list *bl)
{
	int ret = 0;
	struct status_change *sc_data=status_get_sc_data(bl);

	nullpo_retr(0, bl);
	if(*bl==BL_MOB)
		ret=0;
	else if(*bl==BL_PC)
		ret=((map_session_data *)bl)->right_weapon.atk_ele;
	else if(*bl==BL_PET)
		ret=0;

	if(sc_data) {
		if( sc_data[SC_FROSTWEAPON].timer!=-1)	// ƒtƒƒXƒgƒEƒFƒ|ƒ“
			ret=1;
		if( sc_data[SC_SEISMICWEAPON].timer!=-1)	// ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“
			ret=2;
		if( sc_data[SC_FLAMELAUNCHER].timer!=-1)	// ƒtƒŒ[ƒ€ƒ‰ƒ“ƒ`ƒƒ[
			ret=3;
		if( sc_data[SC_LIGHTNINGLOADER].timer!=-1)	// ƒ‰ƒCƒgƒjƒ“ƒOƒ[ƒ_[
			ret=4;
		if( sc_data[SC_ENCPOISON].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
			ret=5;
		if( sc_data[SC_ASPERSIO].timer!=-1)		// ƒAƒXƒyƒ‹ƒVƒI
			ret=6;
	}

	return ret;
}
int status_get_attack_element2(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_PC) {
		int ret = ((map_session_data *)bl)->left_weapon.atk_ele;
		struct status_change *sc_data = ((map_session_data *)bl)->sc_data;

		if(sc_data) {
			if( sc_data[SC_FROSTWEAPON].timer!=-1)	// ƒtƒƒXƒgƒEƒFƒ|ƒ“
				ret=1;
			if( sc_data[SC_SEISMICWEAPON].timer!=-1)	// ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“
				ret=2;
			if( sc_data[SC_FLAMELAUNCHER].timer!=-1)	// ƒtƒŒ[ƒ€ƒ‰ƒ“ƒ`ƒƒ[
				ret=3;
			if( sc_data[SC_LIGHTNINGLOADER].timer!=-1)	// ƒ‰ƒCƒgƒjƒ“ƒOƒ[ƒ_[
				ret=4;
			if( sc_data[SC_ENCPOISON].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
				ret=5;
			if( sc_data[SC_ASPERSIO].timer!=-1)		// ƒAƒXƒyƒ‹ƒVƒI
				ret=6;
		}
		return ret;
	}
	return 0;
}









int status_isimmune(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl == BL_PC)
	{
		map_session_data *sd = bl->get_sd();
		if(sd->state.no_magic_damage)
			return 1;
		else if(sd->sc_data[SC_HERMODE].timer != -1)
			return 1;
	}
	return 0;
}

// StatusChangeŒn‚ÌŠ“¾
struct status_change *status_get_sc_data(const block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(*bl==BL_MOB)
		return const_cast<block_list*>(bl)->get_md()->sc_data;	// bad
	else if(*bl==BL_PC)
		return const_cast<block_list*>(bl)->get_sd()->sc_data;
	return NULL;
}

short *status_get_opt1(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_MOB)
		return &bl->get_md()->opt1;
	else if(*bl==BL_PC)
		return &bl->get_sd()->opt1;
	else if(*bl==BL_NPC)
		return &bl->get_nd()->opt1;
	return 0;
}
short *status_get_opt2(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_MOB)
		return &bl->get_md()->opt2;
	else if(*bl==BL_PC)
		return &bl->get_sd()->opt2;
	else if(*bl==BL_NPC)
		return &bl->get_nd()->opt2;
	return 0;
}
short *status_get_opt3(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_MOB)
		return &bl->get_md()->opt3;
	else if(*bl==BL_PC)
		return &bl->get_sd()->opt3;
	else if(*bl==BL_NPC)
		return &bl->get_nd()->opt3;
	return 0;
}
short *status_get_option(block_list *bl)
{
	nullpo_retr(0, bl);
	if(*bl==BL_MOB)
		return &bl->get_md()->option;
	else if(*bl==BL_PC)
		return &bl->get_sd()->status.option;
	else if(*bl==BL_NPC)
		return &bl->get_nd()->option;
	return 0;
}

int status_get_sc_def(block_list *bl, int type)
{
	int sc_def;
	nullpo_retr(0, bl);
	
	switch (type)
	{
	case SP_MDEF1:	// mdef
		sc_def = 100 - (3 + status_get_mdef(bl) + bl->get_luk()/3);
		break;
	case SP_MDEF2:	// int
		sc_def = 100 - (3 + bl->get_int() + bl->get_luk()/3);
		break;
	case SP_DEF1:	// def
		sc_def = 100 - (3 + status_get_def(bl) + bl->get_luk()/3);
		break;
	case SP_DEF2:	// vit
		sc_def = 100 - (3 + bl->get_vit() + bl->get_luk()/3);
		break;
	case SP_LUK:	// luck
		sc_def = 100 - (3 + bl->get_luk());
		break;

	case SC_STONE:
	case SC_FREEZE:
		sc_def = 100 - (3 + status_get_mdef(bl) + bl->get_luk()/3);
		break;
	case SC_STAN:
	case SC_POISON:
	case SC_SILENCE:
		sc_def = 100 - (3 + bl->get_vit() + bl->get_luk()/3);
		break;	
	case SC_SLEEP:
	case SC_CONFUSION:
		sc_def = 100 - (3 + bl->get_int() + bl->get_luk()/3);
		break;
	case SC_BLIND:
		sc_def = 100 - (3 + bl->get_int() + bl->get_vit()/3);
		break;
	case SC_CURSE:
		sc_def = 100 - (3 + bl->get_luk() + bl->get_vit()/3);
		break;	

	default:
		sc_def = 100;
		break;
	}

	if(*bl == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if (md && md->class_ == MOBID_EMPERIUM)
			return 0;
		if (sc_def < 50)
			sc_def = 50;
	} else if(*bl == BL_PC) {
		struct status_change* sc_data = status_get_sc_data(bl);
		if (sc_data)
		{
			if( sc_data[SC_GOSPEL].timer != -1 && sc_data[SC_GOSPEL].val4.num == BCT_PARTY )
				sc_def = 0; //Status inmunity
			else if (sc_data[SC_SIEGFRIED].timer != -1)
				sc_def -= sc_data[SC_SIEGFRIED].val2.num; //Status resistance.
		}
	}

	return (sc_def < 0) ? 0 : sc_def;
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíŠJn
 *------------------------------------------
 */
int status_change_start(block_list *bl,int type, basics::numptr val1,basics::numptr val2,basics::numptr val3,basics::numptr val4,unsigned long tick,int flag)
{
	map_session_data *sd = NULL;
	struct status_change* sc_data;
	short*option, *opt1, *opt2, *opt3;
	int opt_flag = 0, calc_flag = 0,updateflag = 0, save_flag = 0, race, mode, elem, undead_flag;
	int scdef = 0;

	nullpo_retr(0, bl);
	if(*bl == BL_SKILL)
		return 0;
	if(*bl == BL_MOB)
		if( bl->is_dead() ) return 0;
	if(*bl == BL_PET)	//Pets cannot have status effects
		return 0;

	nullpo_retr(0, sc_data=status_get_sc_data(bl));
	nullpo_retr(0, option=status_get_option(bl));
	nullpo_retr(0, opt1=status_get_opt1(bl));
	nullpo_retr(0, opt2=status_get_opt2(bl));
	nullpo_retr(0, opt3=status_get_opt3(bl));

	race=bl->get_race();
	mode=bl->get_mode();
	elem=status_get_elem_type(bl);
	undead_flag=bl->is_undead();

	if(type == SC_AETERNA && (sc_data[SC_STONE].timer != -1 || sc_data[SC_FREEZE].timer != -1) )
		return 0;

	switch(type)
	{
		case SC_STONE:
		case SC_FREEZE:
			scdef=3+status_get_mdef(bl)+bl->get_luk()/3;
			break;
		case SC_STAN:
		case SC_SILENCE:
		case SC_POISON:
		case SC_DPOISON:
			scdef=3+bl->get_vit()+bl->get_luk()/3;
			break;
		case SC_SLEEP:
		case SC_BLIND:
			scdef=3+bl->get_int()+bl->get_luk()/3;
			break;
		case SC_CURSE:
			scdef=3+bl->get_luk();
			break;
		default:
			scdef=0;
	}
	if(scdef>=100)
		return 0;
	if(*bl==BL_PC){
		sd=(map_session_data *)bl;
		if( sd && type == SC_ADRENALINE && !(skill_get_weapontype(BS_ADRENALINE)&(1<<sd->status.weapon)))
			return 0;

		if(SC_STONE<=type && type<=SC_BLIND){	// ƒJ?ƒh‚É‚æ‚é‘Ï«
			if( sd && sd->reseff[type-SC_STONE] > 0 && rand()%10000<sd->reseff[type-SC_STONE]){
				if(config.battle_log)
					ShowMessage("PC %d skill_sc_start: card‚É‚æ‚éˆÙí‘Ï«?“®\n",sd->block_list::id);
				return 0;
			}
		}
	}
	else if(*bl == BL_MOB) {		
	}
	else {
		if(config.error_log)
			ShowMessage("status_change_start: neither MOB nor PC !\n");
		return 0;
	}

	if((type==SC_FREEZE || type==SC_STONE) && undead_flag && !(flag&1))
	//I've been informed that undead chars are inmune to stone curse too. [Skotlex]
		return 0;
	
	
	if (type==SC_BLESSING && (*bl==BL_PC || (!undead_flag && race!=6))) {
		if (sc_data[SC_CURSE].timer!=-1)
			status_change_end(bl,SC_CURSE,-1);
		if (sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2.num==0)
			status_change_end(bl,SC_STONE,-1);
	}

	if((type == SC_ADRENALINE || type == SC_WEAPONPERFECTION || type == SC_OVERTHRUST) &&
		sc_data[type].timer != -1 && sc_data[type].val2.num && !val2.num)
		return 0;

	if( mode & 0x20  && !(flag&1) &&
		(type==SC_STONE || type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP || type==SC_SILENCE ||
		 type==SC_POISON || type==SC_DPOISON || type==SC_CURSE  || type==SC_ROKISWEIL ||
		 type==SC_QUAGMIRE || type == SC_DECREASEAGI || type == SC_SIGNUMCRUCIS || type == SC_PROVOKE ||
		 (type == SC_BLESSING && (undead_flag || race == 6))) )
	{	// ƒ{ƒX‚É‚Í?‚©‚È‚¢(‚½‚¾‚µƒJ?ƒh‚É‚æ‚é?‰Ê‚Í“K—p‚³‚ê‚é)
		return 0;
	}
	if(type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP || type == SC_CONFUSION)
		bl->stop_walking(1);

	if(sc_data[type].timer != -1)
	{	// ‚·‚Å‚É“¯‚¶ˆÙí‚É‚È‚Á‚Ä‚¢‚éê‡ƒ^ƒCƒ}‰ğœ
		if(sc_data[type].val1.num > val1.num && type != SC_COMBO && type != SC_DANCING && type != SC_DEVOTION &&
			type != SC_SPEEDPOTION0 && type != SC_SPEEDPOTION1 && type != SC_SPEEDPOTION2 && type != SC_SPEEDPOTION3
			&& type != SC_ATKPOT && type != SC_MATKPOT) // added atk and matk potions [Valaris]
			return 0;

		if ((type >=SC_STAN && type <= SC_BLIND) || type == SC_DPOISON)
			return 0;// ?‚¬‘«‚µ‚ª‚Å‚«‚È‚¢?‘ÔˆÙí‚Å‚ ‚é‚Í?‘ÔˆÙí‚ğs‚í‚È‚¢

		delete_timer(sc_data[type].timer, status_change_timer);
		sc_data[type].timer = -1;
	}

	// ƒNƒAƒOƒ}ƒCƒA/„‚ğ–Y‚ê‚È‚¢‚Å’†‚Í–³Œø‚ÈƒXƒLƒ‹
	if ((sc_data[SC_QUAGMIRE].timer!=-1 || sc_data[SC_DONTFORGETME].timer!=-1) &&
		(type==SC_CONCENTRATE || type==SC_INCREASEAGI ||
		type==SC_TWOHANDQUICKEN || type==SC_SPEARSQUICKEN ||
		type==SC_ADRENALINE || type==SC_TRUESIGHT ||
		type==SC_WINDWALK || type==SC_CARTBOOST || type==SC_ASSNCROS))
	return 0;

	switch(type){	// ˆÙí‚Ìí—Ş‚²‚Æ‚Ì?—
		case SC_PROVOKE:			// ƒvƒƒ{ƒbƒN
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	// (ƒI?ƒgƒo?ƒT?ƒN)
			break;
		case SC_ENDURE:				// ƒCƒ“ƒfƒ…ƒA
			calc_flag = 1; // for updating mdef
			if(tick <= 0) tick = 1000 * 60;
			val2 = 7; // [Celest]
			break;
		case SC_AUTOBERSERK:
			{
				tick = 60*1000;
				if (*bl == BL_PC && sd->status.hp<sd->status.max_hp>>2 &&
					(sc_data[SC_PROVOKE].timer==-1 || sc_data[SC_PROVOKE].val2.num==0))
					status_change_start(bl,SC_PROVOKE,10,1,0,0,0,0);
			}
			break;
		
		case SC_INCREASEAGI:		// ‘¬“xã¸
			calc_flag = 1;
			if(sc_data[SC_DECREASEAGI].timer!=-1 )
				status_change_end(bl,SC_DECREASEAGI,-1);
			// the effect will still remain [celest]
//			if(sc_data[SC_WINDWALK].timer!=-1 )	// ƒEƒCƒ“ƒhƒEƒH?ƒN
//				status_change_end(bl,SC_WINDWALK,-1);
			break;
		case SC_DECREASEAGI:		// ‘¬“xŒ¸­
			if (*bl == BL_PC)	// Celest
				tick>>=1;
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			break;
		case SC_SIGNUMCRUCIS:		// ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX
			calc_flag = 1;
			val2 = 10 + val1.num*2;
			tick = 600*1000;
			clif_emotion(*bl,4);
			break;
		case SC_SLOWPOISON:
			if (sc_data[SC_POISON].timer == -1 && sc_data[SC_DPOISON].timer == -1)
				return 0;
			break;
		case SC_TWOHANDQUICKEN:		// 2HQ
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			*opt3 |= 1;
			calc_flag = 1;
			break;
		case SC_ADRENALINE:			// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			if(*bl == BL_PC)
				if(pc_checkskill(*sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			calc_flag = 1;
			break;
		case SC_WEAPONPERFECTION:	// ƒEƒFƒ|ƒ“ƒp?ƒtƒFƒNƒVƒ‡ƒ“
			if(*bl == BL_PC)
				if(pc_checkskill(*sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			break;
		case SC_OVERTHRUST:			// ƒI?ƒo?ƒXƒ‰ƒXƒg
			if(*bl == BL_PC)
				if(pc_checkskill(*sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			*opt3 |= 2;
			break;
		case SC_MAXIMIZEPOWER:		// ƒ}ƒLƒVƒ}ƒCƒYƒpƒ?(SP‚ª1Œ¸‚éŠÔ,val2‚É‚à)
			if(*bl == BL_PC)
				val2 = tick;
			else
				tick = 5000*val1.num;
			break;
		case SC_ENCPOISON:			// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
			calc_flag = 1;
			val2=(((val1.num - 1) / 2) + 3)*100;	// “Å•t?Šm—¦
			skill_enchant_elemental_end(bl,SC_ENCPOISON);
			break;
		case SC_EDP:	// [Celest]
			val2 = val1.num + 2;			// –Ò“Å•t?Šm—¦(%)
			calc_flag = 1;
			break;
		case SC_POISONREACT:	// ƒ|ƒCƒYƒ“ƒŠƒAƒNƒg
			val2=val1.num/2 + val1.num%2; // [Celest]
			break;
		case SC_ASPERSIO:			// ƒAƒXƒyƒ‹ƒVƒI
			skill_enchant_elemental_end(bl,SC_ASPERSIO);
			break;
		case SC_ENERGYCOAT:			// ƒGƒiƒW?ƒR?ƒg
			*opt3 |= 4;
			break;
		case SC_MAGICROD:
			val2 = val1.num*20;
			break;
		case SC_KYRIE:				// ƒLƒŠƒGƒGƒŒƒCƒ\ƒ“
			val2 = bl->get_max_hp() * (val1.num * 2 + 10) / 100;// ‘Ï‹v“x
			val3 = (val1.num / 2 + 5);	// ‰ñ?
// -- moonsoul (added to undo assumptio status if target has it)
			if(sc_data[SC_ASSUMPTIO].timer!=-1 )
				status_change_end(bl,SC_ASSUMPTIO,-1);
			break;
		case SC_MINDBREAKER:
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	// (ƒI?ƒgƒo?ƒT?ƒN)
		case SC_TRICKDEAD:			// €‚ñ‚¾‚Ó‚è
			bl->stop_attack();
			break;
		case SC_QUAGMIRE:			// ƒNƒ@ƒOƒ}ƒCƒA
			calc_flag = 1;
			if(sc_data[SC_CONCENTRATE].timer!=-1 )	// W’†—ÍŒüã‰ğœ
				status_change_end(bl,SC_CONCENTRATE,-1);
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	// ‘¬“xã¸‰ğœ
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	// ƒgƒDƒ‹?ƒTƒCƒg
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	// ƒEƒCƒ“ƒhƒEƒH?ƒN
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	// ƒJ?ƒgƒu?ƒXƒg
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_MAGICPOWER:
			calc_flag = 1;
			val2 = 1;
			break;
		case SC_SACRIFICE:
			val2 = 5;
			break;
		case SC_FLAMELAUNCHER:		// ƒtƒŒ?ƒ€ƒ‰ƒ“ƒ`ƒƒ?
			skill_enchant_elemental_end(bl,SC_FLAMELAUNCHER);
			break;
		case SC_FROSTWEAPON:		// ƒtƒƒXƒgƒEƒFƒ|ƒ“
			skill_enchant_elemental_end(bl,SC_FROSTWEAPON);
			break;
		case SC_LIGHTNINGLOADER:	// ƒ‰ƒCƒgƒjƒ“ƒOƒ?ƒ_?
			skill_enchant_elemental_end(bl,SC_LIGHTNINGLOADER);
			break;
		case SC_SEISMICWEAPON:		// ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“
			skill_enchant_elemental_end(bl,SC_SEISMICWEAPON);
			break;
		case SC_PROVIDENCE:			// ƒvƒƒ”ƒBƒfƒ“ƒX
			calc_flag = 1;
			val2=val1.num*5;
			break;
		case SC_REFLECTSHIELD:
			val2=10+val1.num*3;
			break;
		case SC_STRIPWEAPON:
			if (val2.num==0) val2=90;
			break;
		case SC_STRIPSHIELD:
			if (val2.num==0) val2=85;
			break;

		case SC_AUTOSPELL:			// ƒI?ƒgƒXƒyƒ‹
			val4 = 5 + val1.num*2;
			break;

		case SC_VOLCANO:
			calc_flag = 1;
			val3 = val1.num*10;
			break;
		case SC_DELUGE:
			calc_flag = 1;
			if (sc_data[SC_FOGWALL].timer != -1 && sc_data[SC_BLIND].timer != -1)
				status_change_end(bl,SC_BLIND,-1);
			break;
		case SC_VIOLENTGALE:
			calc_flag = 1;
			val3 = val1.num*3;
			break;

		case SC_SPEARSQUICKEN:		// ƒXƒsƒAƒNƒCƒbƒPƒ“
			calc_flag = 1;
			val2 = 20+val1.num;
			*opt3 |= 1;
			break;

		case SC_BLADESTOP:		// ”’næ‚è
			if(val2.num==2 && val3.isptr && val4.isptr)
				clif_bladestop(*((block_list *)val3.ptr),*((block_list *)val4.ptr),1);
			*opt3 |= 32;
			break;

		case SC_LULLABY:			// qç‰S
			val2 = 11;
			break;
		case SC_RICHMANKIM:
			break;
		case SC_ETERNALCHAOS:		// ƒGƒ^?ƒiƒ‹ƒJƒIƒX
			calc_flag = 1;
			break;
		case SC_DRUMBATTLE:			// ?‘¾ŒÛ‚Ì‹¿‚«
			calc_flag = 1;
			val2 = (val1.num+1)*25;
			val3 = (val1.num+1)*2;
			break;
		case SC_NIBELUNGEN:			// ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö
			calc_flag = 1;
			//val2 = (val1.num+2)*50;
			val3 = (val1.num+2)*25;
			break;
		case SC_SIEGFRIED:			// •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh
			calc_flag = 1;
			val2 = 55 + val1.num*5;
			val3 = val1.num*10;
			break;
		case SC_DISSONANCE:			// •s‹¦˜a‰¹
		case SC_UGLYDANCE:			// ©•ªŸè‚Èƒ_ƒ“ƒX
			val2 = 10;
			break;

		case SC_ROKISWEIL:			// ƒƒL‚Ì‹©‚Ñ
		case SC_INTOABYSS:			// [•£‚Ì’†‚É
		case SC_POEMBRAGI:			// ƒuƒ‰ƒM‚Ì
			break;
		
		case SC_WHISTLE:			// Œû“J
		case SC_ASSNCROS:			// —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
		case SC_APPLEIDUN:			// ƒCƒhƒDƒ“‚Ì—ÑŒç
		case SC_HUMMING:			// ƒnƒ~ƒ“ƒO
		case SC_FORTUNE:			// K‰^‚ÌƒLƒX
		case SC_SERVICE4U:			// ƒT?ƒrƒXƒtƒH?ƒ†?
			calc_flag = 1;
			break;

		case SC_DONTFORGETME:		// „‚ğ–Y‚ê‚È‚¢‚Å
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	// ‘¬“xã¸‰ğœ
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_ASSNCROS].timer!=-1 )
				status_change_end(bl,SC_ASSNCROS,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	// ƒgƒDƒ‹?ƒTƒCƒg
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	// ƒEƒCƒ“ƒhƒEƒH?ƒN
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	// ƒJ?ƒgƒu?ƒXƒg
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_MOONLIT:
			val2 = bl->id;
			break;
		case SC_DANCING:			// ƒ_ƒ“ƒX/‰‰‘t’†
			calc_flag = 1;
			val3= tick / 1000;
			tick = 1000;
			break;

		case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			calc_flag = 1;
			val2 = 75 + 25*val1.num;
			*opt3 |= 8;
			break;
		case SC_STEELBODY:			// ‹à„
			calc_flag = 1;
			*opt3 |= 16;
			break;

		case SC_AUTOCOUNTER:
			val3 = val4 = 0;
			break;

		case SC_SPEEDPOTION0:		// ?‘¬ƒ|?ƒVƒ‡ƒ“
		case SC_SPEEDPOTION1:
		case SC_SPEEDPOTION2:
		case SC_SPEEDPOTION3:
			calc_flag = 1;
			tick = 1000 * tick;
			val2 = 5*(2+type-SC_SPEEDPOTION0);
			// Since people complain so much about the various icons showing up, here we disable the visual of any other potions [Skotlex] 
			if (sc_data[SC_SPEEDPOTION0].timer != -1)
				clif_status_change(*bl,SC_SPEEDPOTION0,0);
			else if (sc_data[SC_SPEEDPOTION1].timer != -1)
				clif_status_change(*bl,SC_SPEEDPOTION1,0);
			else if (sc_data[SC_SPEEDPOTION2].timer != -1)
				clif_status_change(*bl,SC_SPEEDPOTION2,0);
			else if (sc_data[SC_SPEEDPOTION3].timer != -1)
				clif_status_change(*bl,SC_SPEEDPOTION3,0);
			break;

		// atk & matk potions [Valaris]
		case SC_ATKPOT:
		case SC_MATKPOT:
			calc_flag = 1;
			tick = 1000 * tick;
			break;
		case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			{
				time_t timer;

				calc_flag = 1;
				tick = 10000;
				if(!val2.num)
					val2 = time(&timer);
			}
			break;
		case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
			{
				time_t timer;

				if(!config.muting_players)
					break;

				tick = 60000;
				if(!val2.num)
					val2 = time(&timer);
				updateflag = SP_MANNER;
				save_flag = 1; // celest
			}
			break;

		// option1
		case SC_STONE:				// Î‰»
			if(!(flag&2)) {
				int sc_def = status_get_mdef(bl)*200;
				tick = tick - sc_def;
			}
			val3 = tick/1000;
			if(val3.num < 1) val3.num = 1;
			tick = 5000;
			val2 = 1;
			break;
		case SC_SLEEP:				// ‡–°
			if(!(flag&2)) {
				tick = 30000;//‡–°‚ÍƒXƒe?ƒ^ƒX‘Ï«‚É?‚í‚ç‚¸30•b
			}
			break;
		case SC_FREEZE:				// “€Œ‹
			if(!(flag&2)) {
				int sc_def = 100 - status_get_mdef(bl);
				tick = tick * sc_def / 100;
			}
			break;
		case SC_STAN:				// ƒXƒ^ƒ“ival2‚Éƒ~ƒŠ•bƒZƒbƒgj
			if(!(flag&2)) {
				int sc_def = status_get_sc_def_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;

			// option2
		case SC_DPOISON:			// –Ò“Å
		{
			int mhp = bl->get_max_hp();
			int hp = bl->get_hp();
			// MHP?1/4????????
			if (hp > mhp>>2) {
				if(*bl == BL_PC)
				{
					int diff = mhp*10/100;
					if (hp - diff < mhp>>2)
						hp = hp - (mhp>>2);
					bl->heal(-hp, 0);
				}
				else if(*bl == BL_MOB)
				{
					struct mob_data *md = (struct mob_data *)bl;
					hp -= mhp*15/100;
					if (hp > mhp>>2)
						md->hp = hp;
					else
						md->hp = mhp>>2;
				}
			}
		}	// fall through
		case SC_POISON:				// “Å
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = 100 - (bl->get_vit() + bl->get_luk()/5);
				tick = tick * sc_def / 100;
			}
			val3 = tick/1000;
			if(val3.num < 1) val3.num = 1;
			tick = 1000;
			break;
		case SC_SILENCE:			// ’¾?iƒŒƒbƒNƒXƒfƒr?ƒij
			if (sc_data && sc_data[SC_GOSPEL].timer!=-1)
			{
				struct skill_unit_group *ptr = (struct skill_unit_group *)sc_data[SC_GOSPEL].val3.ptr;
				if(ptr) skill_delunitgroup(*ptr);
				status_change_end(bl,SC_GOSPEL,-1);
				break;
			}
			if(!(flag&2)) {
				int sc_def = 100 - bl->get_vit();
				tick = tick * sc_def / 100;
			}
			break;
		case SC_CONFUSION:
			val2 = tick;
			tick = 100;
			clif_emotion(*bl,1);
			if(sd)	sd->stop_walking(0);
			break;
		case SC_BLIND:				// ˆÃ?
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = bl->get_lv()/10 + bl->get_int()/15;
				tick = 30000 - sc_def;
			}
			break;
		case SC_CURSE:
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = 100 - bl->get_vit();
				tick = tick * sc_def / 100;
			}
			break;

		// option
		case SC_HIDING:		// ƒnƒCƒfƒBƒ“ƒO
			calc_flag = 1;
			if(*bl == BL_PC) {
				val2 = tick / 1000;		// ?ŠÔ
				tick = 1000;
			}
			break;
		case SC_CHASEWALK:
		case SC_CLOAKING:		// ƒNƒ?ƒLƒ“ƒO
			if(*bl == BL_PC) {
				calc_flag = 1; // [Celest]
				val2 = tick;
				val3 = type==SC_CLOAKING ? 130-val1.num*3 : 135-val1.num*5;
			}
			else
				tick = 5000*val1.num;
			break;
		case SC_SIGHT:			// ƒTƒCƒg/ƒ‹ƒAƒt
		case SC_RUWACH:
			val2 = tick/250;
			tick = 10;
			break;

		// ƒXƒLƒ‹‚¶‚á‚È‚¢/ŠÔ‚É?ŒW‚µ‚È‚¢
		case SC_RIDING:
			calc_flag = 1;
			tick = 600*1000;
			break;
		case SC_FALCON:
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKNWEAPON:
		case SC_BROKNARMOR:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
			tick=600*1000;
			break;
		case SC_STORMKICK: // Taekwon Kicks SC [Dralnu]
		case SC_DOWNKICK:
		case SC_COUNTER:
		case SC_TURNKICK:
			tick = 1000;
			clif_displaymessage(sd->fd,"Hit now !!"); // Waiting to know how it should work [Dralnu]
			break;
		case SC_AUTOGUARD:
			{
				int i,t;
				for(i=val2.num=0;i<val1.num;++i) {
					t = 5-(i>>1);
					val2.num += (t < 0)? 1:t;
				}
			}
			break;

		case SC_DEFENDER:
			calc_flag = 1;
			val2 = 5 + val1.num*15;
			break;

		case SC_CONCENTRATION:	// ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			*opt3 |= 1;
			calc_flag = 1;
			break;

		case SC_TENSIONRELAX:	// ƒeƒ“ƒVƒ‡ƒ“ƒŠƒ‰ƒbƒNƒX
			if(*bl == BL_PC) {
				tick = 10000;
			} else return 0;
			break;

		case SC_PARRYING:		// ƒpƒŠƒCƒ“ƒO
		    val2 = 20 + val1.num*3;
			break;

		case SC_WINDWALK:		// ƒEƒCƒ“ƒhƒEƒH?ƒN
			calc_flag = 1;
			val2 = (val1.num+1) / 2; //Fleeã¸—¦
			break;
		
		case SC_JOINTBEAT: // Random break [DracoRPG]
			calc_flag = 1;
			val2 = rand()%6;
			if (val2.num == 5)
				status_change_start(bl,SC_BLEEDING,val1.num,0,0,0,skill_get_time2(type,val1.num),0);
			break;

		case SC_BERSERK:		// ƒo?ƒT?ƒN
			if(sd){
				sd->status.hp = sd->status.max_hp * 3;
				sd->status.sp = 0;
				clif_updatestatus(*sd,SP_HP);
				clif_updatestatus(*sd,SP_SP);
				sd->canregen_tick = gettick() + 300000;
			}
			*opt3 |= 128;
			tick = 10000;
			calc_flag = 1;
			break;

		case SC_ASSUMPTIO:		// ƒAƒXƒ€ƒvƒeƒBƒI
			if(sc_data[SC_KYRIE].timer!=-1 )
			{
				status_change_end(bl,SC_KYRIE,-1);
				break;
			}
			*opt3 |= 2048;
			break;

		case SC_GOSPEL:
			if (val4.num == BCT_SELF) {	// self effect
				if (sd) {
					sd->canact_tick += tick;
					sd->canmove_tick += tick;
				}
				val2 = tick;
				tick = 1000;
				status_change_clear_buffs(bl);
			}
			break;

		case SC_MARIONETTE:		// ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹
		case SC_MARIONETTE2:
			val2 = tick;
			if (!val3.num)
				return 0;
			tick = 1000;
			calc_flag = 1;
			*opt3 |= 1024;
			break;

		case SC_REJECTSWORD:	// ƒŠƒWƒFƒNƒgƒ\?ƒh
			val2 = 3; //3‰ñU?‚ğ’µ‚Ë•Ô‚·
			break;

		case SC_MEMORIZE:		// ƒƒ‚ƒ‰ƒCƒY
			val2 = 5; //‰ñ‰r¥‚ğ1/3‚É‚·‚é
			break;

		case SC_GRAVITATION:
			if (sd) {
				if (val3.num == BCT_SELF) {
					sd->canmove_tick += tick;
					sd->canact_tick += tick;
				} else calc_flag = 1;
			}
			break;

		case SC_HERMODE:
			status_change_clear_buffs(bl);
			break;

		case SC_BLEEDING:
			{
				val4 = tick;
				tick = 10000;
			}
			break;

		case SC_REGENERATION:
			val1 = 2;
		case SC_BATTLEORDERS:
			tick = 60000; // 1 minute
			calc_flag = 1;
			break;
		case SC_GUILDAURA:
			calc_flag = 1;
			tick = 1000;
			break;


		case SC_CONCENTRATE:		// W’†—ÍŒüã
		case SC_BLESSING:			// ƒuƒŒƒbƒVƒ“ƒO
		case SC_ANGELUS:			// ƒAƒ“ƒ[ƒ‹ƒX
		case SC_IMPOSITIO:			// ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX
		case SC_GLORIA:				// ƒOƒƒŠƒA
		case SC_LOUD:				// ƒ‰ƒEƒhƒ{ƒCƒX
		case SC_DEVOTION:			// ƒfƒBƒ{?ƒVƒ‡ƒ“
		case SC_KEEPING:
		case SC_BARRIER:
		case SC_MELTDOWN:		// ƒƒ‹ƒgƒ_ƒEƒ“
		case SC_CARTBOOST:		// ƒJ?ƒgƒu?ƒXƒg
		case SC_TRUESIGHT:		// ƒgƒDƒ‹?ƒTƒCƒg
		case SC_SPIDERWEB:		// ƒXƒpƒCƒ_?ƒEƒFƒbƒu
		case SC_SLOWDOWN:
		case SC_SPEEDUP0:
		case SC_SPEEDUP1:
		case SC_INCALLSTATUS:
		case SC_INCHIT:			// HITã¸
		case SC_INCFLEE:		// FLEEã¸
		case SC_INCMHP2:		// MHP%ã¸
		case SC_INCMSP2:		// MSP%ã¸
		case SC_INCATK2:		// ATK%ã¸
		case SC_INCMATK2:
		case SC_INCHIT2:		// HIT%ã¸
		case SC_INCFLEE2:		// FLEE%ã¸
		case SC_INCDEF2:
		case SC_INCSTR:
		case SC_INCAGI:
		case SC_INCVIT:
		case SC_INCINT:
		case SC_INCDEX:
		case SC_INCLUK:
			calc_flag = 1;
			break;

		// ƒZ?ƒtƒeƒBƒEƒH?ƒ‹Aƒjƒ…?ƒ} 
		case SC_SAFETYWALL:
		case SC_PNEUMA:

			if(val2.isptr)
				tick = ((struct skill_unit *)val2.ptr)->group->limit;
			break;

		case SC_SUFFRAGIUM:			// ƒTƒtƒ‰ƒMƒ€
		case SC_BENEDICTIO:			// ¹?
		case SC_MAGNIFICAT:			// ƒ}ƒOƒjƒtƒBƒJ?ƒg
		case SC_AETERNA:			// ƒG?ƒeƒ‹ƒi
  		case SC_STRIPARMOR:
		case SC_STRIPHELM:
		case SC_CP_WEAPON:
		case SC_CP_SHIELD:
		case SC_CP_ARMOR:
		case SC_CP_HELM:
		case SC_EXTREMITYFIST:		// ˆ¢C—…”e™€Œ
		case SC_ANKLE:	// ƒAƒ“ƒNƒ‹
		case SC_COMBO:
		case SC_BLADESTOP_WAIT:		// ”’næ‚è(‘Ò‚¿)
		case SC_HALLUCINATION:
		case SC_BASILICA: // [celest]
		case SC_SPLASHER:		// ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ?
		case SC_FOGWALL:
		case SC_PRESERVE:
		case SC_DOUBLECAST:
		case SC_MAXOVERTHRUST:
        case SC_AURABLADE:		// ƒI?ƒ‰ƒuƒŒ?ƒh
       	case SC_BABY:
		case SC_RUN:
		case SC_WATK_ELEMENT:
			break;

		default:
			if(config.error_log)
				ShowMessage("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	if (*bl == BL_PC && (config.display_hallucination || type != SC_HALLUCINATION))
		clif_status_change(*bl,type,1);	// ƒAƒCƒRƒ“•\¦

	// option‚Ì?X
	switch(type){
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:

            // Cancel cast when get status [LuzZza]
            if (*bl == BL_PC) {
		        map_session_data *sd = (map_session_data *)bl;
		        if (sd->skilltimer != -1 && sd->skillid != PA_PRESSURE) skill_castcancel(bl, 0);
		    } else if (*bl == BL_MOB) {
		        struct mob_data *md = (struct mob_data *)bl;
		        if (md->skilltimer != -1) skill_castcancel(bl, 0);
			}	    		

			bl->stop_attack();	// U?’â~
			skill_stop_dancing(bl,0);	// ‰‰‘t/ƒ_ƒ“ƒX‚Ì’†?
			{	// “¯‚ÉŠ|‚©‚ç‚È‚¢ƒXƒe?ƒ^ƒXˆÙí‚ğ‰ğœ
				int i;
				for(i = SC_STONE; i <= SC_SLEEP; ++i){
					if(sc_data[i].timer != -1){
						delete_timer(sc_data[i].timer, status_change_timer);
						sc_data[i].timer = -1;
					}
				}
			}
			if(type == SC_STONE)
				*opt1 = 6;
			else
				*opt1 = type - SC_STONE + 1;
			opt_flag = 1;
			break;
		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 |= 1<<(type-SC_POISON);
			opt_flag = 1;
			break;
		case SC_DPOISON:	// b’è‚Å“Å‚ÌƒGƒtƒFƒNƒg‚ğg—p
			*opt2 |= 1;
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 |= 0x40;
			opt_flag = 1;
			break;
		case SC_HIDING:
		case SC_CLOAKING:
			bl->stop_attack();	// U?’â~
			*option |= ((type==SC_HIDING)?2:4);
			opt_flag =1 ;
			break;
		case SC_CHASEWALK:
			bl->stop_attack();	// U?’â~
			*option |= 16388;
			opt_flag =1 ;
			break;
		case SC_SIGHT:
			*option |= 1;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option |= 8192;
			opt_flag = 1;
			break;
		case SC_WEDDING:
			*option |= 4096;
			opt_flag = 1;
	}

	if(opt_flag)	// option‚Ì?X
		clif_changeoption(*bl);

	sc_data[type].val1 = val1;
	sc_data[type].val2 = val2;
	sc_data[type].val3 = val3;
	sc_data[type].val4 = val4;
	// ƒ^ƒCƒ}?İ’è
	sc_data[type].timer = add_timer(gettick() + tick, status_change_timer, bl->id, type);

	if(*bl==BL_PC && calc_flag)
		status_calc_pc(*sd,0);	// ƒXƒe?ƒ^ƒXÄŒvZ

	if(*bl==BL_PC && save_flag)
		chrif_save(*sd); // save the player status

	if(*bl==BL_PC && updateflag)
		clif_updatestatus(*sd,updateflag);	// ƒXƒe?ƒ^ƒX‚ğƒNƒ‰ƒCƒAƒ“ƒg‚É‘—‚é

	if (*bl==BL_PC && sd->pd)
		pet_sc_check(*sd, type); //Skotlex: Pet Status Effect Healing
	return 0;
}
/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙí‘S‰ğœ
 *------------------------------------------
 */
int status_change_clear(block_list *bl,int type)
{
	struct status_change* sc_data;
	short *option, *opt1, *opt2, *opt3;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc_data = status_get_sc_data(bl));
	nullpo_retr(0, option = status_get_option(bl));
	nullpo_retr(0, opt1 = status_get_opt1(bl));
	nullpo_retr(0, opt2 = status_get_opt2(bl));
	nullpo_retr(0, opt3 = status_get_opt3(bl));

	for(i = 0; i < MAX_STATUSCHANGE; ++i)
	{
		if(sc_data[i].timer != -1){	// ˆÙí‚ª‚ ‚é‚È‚çƒ^ƒCƒ}?‚ğíœ‚·‚é
			status_change_end(bl, i, -1);
		}
	}
	*opt1 = 0;
	*opt2 = 0;
	*opt3 = 0;
	*option &= OPTION_MASK;

	if(!type || type&2)
		clif_changeoption(*bl);

	return 0;
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíI—¹
 *------------------------------------------
 */
int status_change_end( block_list* bl, int type, int tid )
{
	struct status_change* sc_data;
	int opt_flag=0, calc_flag = 0;
	short *option, *opt1, *opt2, *opt3;

	nullpo_retr(0, bl);
	if(*bl!=BL_PC && *bl!=BL_MOB) {
		if(config.error_log)
			ShowMessage("status_change_end: neither MOB nor PC !\n");
		return 0;
	}
	nullpo_retr(0, sc_data = status_get_sc_data(bl));
	nullpo_retr(0, option = status_get_option(bl));
	nullpo_retr(0, opt1 = status_get_opt1(bl));
	nullpo_retr(0, opt2 = status_get_opt2(bl));
	nullpo_retr(0, opt3 = status_get_opt3(bl));

	if( sc_data[type].timer != -1 && (sc_data[type].timer == tid || tid == -1))
	{
		if (tid == -1)	// ƒ^ƒCƒ}‚©‚çŒÄ‚Î‚ê‚Ä‚¢‚È‚¢‚È‚çƒ^ƒCƒ}íœ‚ğ‚·‚é
		{
			delete_timer(sc_data[type].timer,status_change_timer);
		}
		// ŠY?‚ÌˆÙí‚ğ³í‚É?‚· 
		sc_data[type].timer=-1;

		switch(type){	// ˆÙí‚Ìí—Ş‚²‚Æ‚Ì?—
			case SC_PROVOKE:			// ƒvƒƒ{ƒbƒN
			case SC_ENDURE: // celest
			case SC_CONCENTRATE:		// W’†—ÍŒüã
			case SC_BLESSING:			// ƒuƒŒƒbƒVƒ“ƒO
			case SC_ANGELUS:			// ƒAƒ“ƒ[ƒ‹ƒX
			case SC_INCREASEAGI:		// ‘¬“xã¸
			case SC_DECREASEAGI:		// ‘¬“xŒ¸­
			case SC_SIGNUMCRUCIS:		// ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX
			case SC_HIDING:
			case SC_TWOHANDQUICKEN:		// 2HQ
			case SC_ADRENALINE:			// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
			case SC_ENCPOISON:			// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
			case SC_IMPOSITIO:			// ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX
			case SC_GLORIA:				// ƒOƒƒŠƒA
			case SC_LOUD:				// ƒ‰ƒEƒhƒ{ƒCƒX
			case SC_QUAGMIRE:			// ƒNƒ@ƒOƒ}ƒCƒA
			case SC_PROVIDENCE:			// ƒvƒƒ”ƒBƒfƒ“ƒX
			case SC_SPEARSQUICKEN:		// ƒXƒsƒAƒNƒCƒbƒPƒ“
			case SC_VOLCANO:
			case SC_DELUGE:
			case SC_VIOLENTGALE:
			case SC_ETERNALCHAOS:		// ƒGƒ^?ƒiƒ‹ƒJƒIƒX
			case SC_DRUMBATTLE:			// ?‘¾ŒÛ‚Ì‹¿‚«
			case SC_NIBELUNGEN:			// ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö
			case SC_SIEGFRIED:			// •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh
			case SC_WHISTLE:			// Œû“J
			case SC_ASSNCROS:			// —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
			case SC_HUMMING:			// ƒnƒ~ƒ“ƒO
			case SC_DONTFORGETME:		// „‚ğ–Y‚ê‚È‚¢‚Å
			case SC_FORTUNE:			// K‰^‚ÌƒLƒX
			case SC_SERVICE4U:			// ƒT?ƒrƒXƒtƒH?ƒ†?
			case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			case SC_STEELBODY:			// ‹à„
			case SC_DEFENDER:
			case SC_APPLEIDUN:			// ƒCƒhƒDƒ“‚Ì—ÑŒç
			case SC_RIDING:
			case SC_BLADESTOP_WAIT:
			case SC_CONCENTRATION:		// ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			case SC_ASSUMPTIO:			// ƒAƒVƒƒƒ“ƒvƒeƒBƒI
			case SC_WINDWALK:		// ƒEƒCƒ“ƒhƒEƒH?ƒN
			case SC_TRUESIGHT:		// ƒgƒDƒ‹?ƒTƒCƒg
			case SC_SPIDERWEB:		// ƒXƒpƒCƒ_?ƒEƒFƒbƒu
			case SC_MAGICPOWER:		// –‚–@—Í?•
			case SC_CHASEWALK:
			case SC_ATKPOT:		// attack potion [Valaris]
			case SC_MATKPOT:		// magic attack potion [Valaris]
			case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			case SC_MELTDOWN:		// ƒƒ‹ƒgƒ_ƒEƒ“
			case SC_CARTBOOST:
			case SC_MINDBREAKER:		// ƒ}ƒCƒ“ƒhƒuƒŒ[ƒJ[
			case SC_BERSERK:
			case SC_EDP:
			case SC_SLOWDOWN:
			case SC_SPEEDUP0:
			case SC_SPEEDUP1:
			case SC_INCALLSTATUS:
			case SC_INCHIT:
			case SC_INCFLEE:
			case SC_INCMHP2:
			case SC_INCMSP2:
			case SC_INCATK2:
			case SC_INCMATK2:
			case SC_INCHIT2:
			case SC_INCFLEE2:
			case SC_INCDEF2:
			case SC_INCSTR:
			case SC_INCAGI:
			case SC_INCVIT:
			case SC_INCINT:
			case SC_INCDEX:
			case SC_INCLUK:
			case SC_BATTLEORDERS:
			case SC_REGENERATION:
			case SC_GUILDAURA:
				calc_flag = 1;
				break;
			case SC_SPEEDPOTION0:		// ?‘¬ƒ|?ƒVƒ‡ƒ“
			case SC_SPEEDPOTION1:
			case SC_SPEEDPOTION2:
			case SC_SPEEDPOTION3:
				calc_flag = 1;
				//Restore the icon if another speed potion is still in effect.
				if (sc_data[SC_SPEEDPOTION3].timer != -1)
					clif_status_change(*bl,SC_SPEEDPOTION3,1);
				else if (sc_data[SC_SPEEDPOTION2].timer != -1)
					clif_status_change(*bl,SC_SPEEDPOTION2,1);
				else if (sc_data[SC_SPEEDPOTION1].timer != -1)
					clif_status_change(*bl,SC_SPEEDPOTION1,1);
				else if (sc_data[SC_SPEEDPOTION0].timer != -1)
					clif_status_change(*bl,SC_SPEEDPOTION0,1);
				break;
			case SC_AUTOBERSERK:
				if(sc_data[SC_PROVOKE].timer != -1)
					status_change_end(bl,SC_PROVOKE,-1);
				break;
			case SC_DEVOTION:		// ƒfƒBƒ{?ƒVƒ‡ƒ“
				{
					map_session_data *md = map_session_data::from_blid(sc_data[type].val1.num);
					sc_data[type].val1=sc_data[type].val2=0;
					if(md) skill_devotion(md,bl->id);
					calc_flag = 1;
				}
				break;
			case SC_BLADESTOP:
				{
					struct status_change *t_sc_data = status_get_sc_data((block_list *)sc_data[type].val4.ptr);
					//•Ğ•û‚ªØ‚ê‚½‚Ì‚Å‘Šè‚Ì”’n?‘Ô‚ªØ‚ê‚Ä‚È‚¢‚Ì‚È‚ç‰ğœ
					if(t_sc_data && t_sc_data[SC_BLADESTOP].timer!=-1 && sc_data[type].val4.isptr)
						status_change_end((block_list *)sc_data[type].val4.ptr,SC_BLADESTOP,-1);

					if(sc_data[type].val2.num==2 && sc_data[type].val3.isptr && sc_data[type].val4.isptr)
						clif_bladestop(*((block_list *)sc_data[type].val3.ptr),*((block_list *)sc_data[type].val4.ptr),0);
				}
				break;
			case SC_DANCING:
				{
					map_session_data *dsd;
					struct status_change *d_sc_data;
					if(sc_data[type].val4.num && (dsd=map_session_data::from_blid(sc_data[type].val4.num))){
						d_sc_data = dsd->sc_data;
						//‡‘t‚Å‘Šè‚ª‚¢‚éê‡‘Šè‚Ìval4‚ğ0‚É‚·‚é
						if(d_sc_data && d_sc_data[type].timer!=-1)
							d_sc_data[type].val4=0;
					}
				}
				if (sc_data[SC_LONGING].timer!=-1)
					status_change_end(bl,SC_LONGING,-1);				
				calc_flag = 1;
				break;
			case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
				{
					map_session_data *sd=NULL;
					if(*bl == BL_PC && (sd=(map_session_data *)bl)){
						if (sd->status.manner >= 0) // weeee ^^ [celest]
							sd->status.manner = 0;
						clif_updatestatus(*sd,SP_MANNER);
					}
				}
				break;
			case SC_SPLASHER:		// ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ?
				{
					block_list *src=block_list::from_blid(sc_data[type].val3.num);
					if(src && tid!=-1){
						//©•ª‚Éƒ_ƒ?ƒW•ü?3*3‚Éƒ_ƒ?ƒW
						skill_castend_damage_id(src, bl,(unsigned short)sc_data[type].val2.num,(unsigned short)sc_data[type].val1.num,gettick(),0 );
					}
				}
				break;
			case SC_RUN:
				if (sc_data[type].val1.num >= 7 && !sc_data[type].val2.num && (*bl != BL_PC || ((map_session_data *)bl)->status.weapon == 0))
					status_change_start(bl, SC_INCSTR,10,0,0,0,skill_get_time2(TK_RUN,sc_data[type].val1.num),0);
				break;

		// option1
			case SC_FREEZE:
				sc_data[type].val3 = 0;
				break;

		// option2
			case SC_POISON:				// “Å
			case SC_BLIND:				// ˆÃ?
			case SC_CURSE:
				calc_flag = 1;
				break;

			case SC_MARIONETTE:		// ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹
			case SC_MARIONETTE2:	/// Marionette target
				{
					// check for partner and end their marionette status as well
					int type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
					block_list *pbl = block_list::from_blid(sc_data[type].val3.num);
					if (pbl) {
						struct status_change* sc_data;
						if(//*status_get_sc_count(pbl) > 0 &&
							(sc_data = status_get_sc_data(pbl)) &&
							sc_data[type2].timer != -1)
							status_change_end(pbl, type2, -1);
					}
					if (type == SC_MARIONETTE)
						clif_marionette(*bl, NULL); 
					calc_flag = 1;
				}
				break;

			case SC_GRAVITATION:
				if (*bl == BL_PC) {
					if (sc_data[type].val3.num == BCT_SELF) {
						map_session_data *sd = (map_session_data *)bl;
						if (sd) {
							unsigned long tick = gettick();
							sd->canmove_tick = tick;
							sd->canact_tick = tick;
						}
					} else calc_flag = 1;
				}
				break;

			case SC_BABY:
				break;
			}

		if (*bl == BL_PC && (config.display_hallucination || type != SC_HALLUCINATION))
			clif_status_change(*bl,type,0);	// ƒAƒCƒRƒ“Á‹

		switch(type){	// ³í‚É?‚é‚Æ‚«‚È‚É‚©?—‚ª•K—v
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:
			*opt1 = 0;
			opt_flag = 1;
			break;

		case SC_POISON:
			if (sc_data[SC_DPOISON].timer != -1)	//
				break;						// DPOISON—p‚ÌƒIƒvƒVƒ‡ƒ“
			*opt2 &= ~1;					// ‚ª?—p‚É—pˆÓ‚³‚ê‚½ê‡‚É‚Í
			opt_flag = 1;					// ‚±‚±‚Ííœ‚·‚é
			break;							//
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 &= ~(1<<(type-SC_POISON));
			opt_flag = 1;
			break;
		case SC_DPOISON:
			if (sc_data[SC_POISON].timer != -1)	// DPOISON—p‚ÌƒIƒvƒVƒ‡ƒ“‚ª
				break;							// —pˆÓ‚³‚ê‚½‚çíœ
			*opt2 &= ~1;	// “Å?‘Ô‰ğœ
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 &= ~0x40;
			opt_flag = 1;
			break;

		case SC_HIDING:
		case SC_CLOAKING:
			*option &= ~((type == SC_HIDING) ? 2 : 4);
			calc_flag = 1;	// orn
			opt_flag = 1 ;
			break;

		case SC_CHASEWALK:
			*option &= ~16388;
			opt_flag = 1 ;
			break;

		case SC_SIGHT:
			*option &= ~1;
			opt_flag = 1;
			break;
		case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			*option &= ~4096;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option &= ~8192;
			opt_flag = 1;
			break;

		//opt3
		case SC_TWOHANDQUICKEN:		// 2HQ
		case SC_SPEARSQUICKEN:		// ƒXƒsƒAƒNƒCƒbƒPƒ“
		case SC_CONCENTRATION:		// ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			*opt3 &= ~1;
			break;
		case SC_OVERTHRUST:			// ƒI?ƒo?ƒXƒ‰ƒXƒg
			*opt3 &= ~2;
			break;
		case SC_ENERGYCOAT:			// ƒGƒiƒW?ƒR?ƒg
			*opt3 &= ~4;
			break;
		case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			*opt3 &= ~8;
			break;
		case SC_STEELBODY:			// ‹à„
			*opt3 &= ~16;
			break;
		case SC_BLADESTOP:		// ”’næ‚è
			*opt3 &= ~32;
			break;
		case SC_BERSERK:		// ƒo?ƒT?ƒN
			*opt3 &= ~128;
			break;
		case SC_MARIONETTE:		// ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹
		case SC_MARIONETTE2:
			*opt3 &= ~1024;
			break;
		case SC_ASSUMPTIO:		// ƒAƒXƒ€ƒvƒeƒBƒI
			*opt3 &= ~2048;
			break;
		}

		if(opt_flag)	// option‚Ì?X‚ğ?‚¦‚é
			clif_changeoption(*bl);

		if (*bl == BL_PC && calc_flag)
			status_calc_pc(*((map_session_data *)bl),0);	// ƒXƒe?ƒ^ƒXÄŒvZ
	}

	return 0;
}


/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíI—¹ƒ^ƒCƒ}[
 *------------------------------------------
 */
int status_change_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	int type = data.num;
	
	struct status_change *sc_data;
	map_session_data *sd=NULL;
	struct mob_data *md=NULL;
	block_list *bl=block_list::from_blid(id);

#ifdef nullpo_retr_f
	nullpo_retr_f(0, bl, "id=%d data=%ld",id,(unsigned long)data.num);
#else
	nullpo_retr(0, bl);
#endif

	nullpo_retr(0, sc_data=status_get_sc_data(bl));

	if(sc_data[type].timer != tid) {
		if(config.error_log)
			ShowMessage("status_change_timer %d != %d\n",tid,sc_data[type].timer);
		return 0;
	}
	// security system to prevent forgetting timer removal
	int temp_timerid = sc_data[type].timer;
	sc_data[type].timer = -1;


	if(*bl==BL_PC)
		sd=(map_session_data *)bl;
	else if(*bl==BL_MOB)
		md=(struct mob_data *)bl;

	switch(type){	// “Áê‚È?—‚É‚È‚éê‡
	case SC_MAXIMIZEPOWER:	// ƒ}ƒLƒVƒ}ƒCƒYƒpƒ?
	case SC_CLOAKING:
		if(sd){
			if( sd->status.sp > 0 ){ // SPØ‚ê‚é‚Ü‚Å? 
				sd->status.sp--;
				clif_updatestatus(*sd,SP_SP);
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(sc_data[type].val2.num+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_CHASEWALK:
		if(sd){
			long sp = 10+sc_data[SC_CHASEWALK].val1.num*2;
			if (maps[sd->block_list::m].flag.gvg) sp *= 5;
			if (sd->status.sp > sp){
				sd->status.sp -= sp; // update sp cost [Celest]
				clif_updatestatus(*sd,SP_SP);
				if ((++sc_data[SC_CHASEWALK].val4.num) == 1) {
					status_change_start(bl, SC_INCSTR, 1<<(sc_data[SC_CHASEWALK].val1.num-1), 0, 0, 0, skill_get_time2(ST_CHASEWALK,sc_data[SC_CHASEWALK].val1.num), 0);
					//status_calc_pc (*sd, 0);
				}
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(sc_data[type].val2.num+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
	break;

	case SC_HIDING:		// ƒnƒCƒfƒBƒ“ƒO
		if(sd){		// SP‚ª‚ ‚Á‚ÄAŠÔ§ŒÀ‚ÌŠÔ‚Í?
			if( sd->status.sp > 0 && (--sc_data[type].val2.num)>0 ){
				if(sc_data[type].val2.num % (sc_data[type].val1.num+3) ==0 ){
					sd->status.sp--;
					clif_updatestatus(*sd,SP_SP);
				}
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(1000+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
	break;

	case SC_SIGHT:	// ƒTƒCƒg
	case SC_RUWACH:	// ƒ‹ƒAƒt
		{
			int range = 5;
			if ( type == SC_SIGHT ) range = 7;

			block_list::foreachinarea( CStatusChangetimer(*bl,type,tick),
				bl->m, ((int)bl->x)-range, ((int)bl->y)-range, ((int)bl->x)+range,((int)bl->y)+range,BL_ALL);

			if( (--sc_data[type].val2.num)>0 ){
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(250+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SIGNUMCRUCIS:		// ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX
		{
			int race = bl->get_race();
			if( race == 6 || bl->is_undead() ) {
				sc_data[type].timer=add_timer(1000*600+tick,status_change_timer, bl->id, data );
				return 0;
			}
		}
		break;

	case SC_PROVOKE:	// ƒvƒƒ{ƒbƒN/ƒI?ƒgƒo?ƒT?ƒN
		if(sc_data[type].val2.num!=0){	// ƒI?ƒgƒo?ƒT?ƒNi‚P•b‚²‚Æ‚ÉHPƒ`ƒFƒbƒNj
			if(sd && sd->status.hp>sd->status.max_hp>>2)	// ’â~
				break;
			sc_data[type].timer=add_timer( 1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_ENDURE:	// ƒCƒ“ƒfƒ…ƒA
	case SC_AUTOBERSERK: // Celest
		if(sd && sd->state.infinite_endure) {
			sc_data[type].timer=add_timer( 1000*60+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_STONE:
		if(sc_data[type].val2.num != 0) {
			short *opt1 = status_get_opt1(bl);
			sc_data[type].val2 = 0;
			sc_data[type].val4 = 0;
			bl->stop_walking(1);
			if(opt1) {
				*opt1 = 1;
				clif_changeoption(*bl);
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		else if( (--sc_data[type].val3.num) > 0) {
			int hp = bl->get_max_hp();
			if((++sc_data[type].val4.num)%5 == 0 && bl->get_hp() > hp>>2)
			{
				hp = hp/100;
				if(hp < 1) hp = 1;
				if(sd)
					sd->heal(-hp,0);
				else if(md)
				{
					md->hp -= hp;
				}
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;
	case SC_POISON:
	case SC_DPOISON:
		if (sc_data[SC_SLOWPOISON].timer == -1 && (--sc_data[type].val3.num) > 0) {
			int hp = bl->get_max_hp();
			if( type == SC_POISON && bl->get_hp() < hp>>2)
				break;
			if(sd) {
				hp = (type == SC_DPOISON) ? 3 + hp/50 : 3 + hp*3/200;
				sd->heal(-hp, 0);
			} else if(md) {
				hp = (type == SC_DPOISON) ? 3 + hp/100 : 3 + hp/200;
				md->hp -= hp;
			}
		}
		if( sc_data[type].val3.num > 0 && !bl->is_dead() )
		{
			sc_data[type].timer = add_timer (1000 + tick, status_change_timer, bl->id, data );
			return 0;
		}
		break;
	case SC_TENSIONRELAX:	// ƒeƒ“ƒVƒ‡ƒ“ƒŠƒ‰ƒbƒNƒX
		if(sd){		// SP‚ª‚ ‚Á‚ÄAHP‚ª?ƒ^ƒ“‚Å‚È‚¯‚ê‚Î??
			if( sd->status.sp > 12 && sd->status.max_hp > sd->status.hp ){
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(10000+tick, status_change_timer,bl->id, data);
				return 0;
			}
			if(sd->status.max_hp <= sd->status.hp)
			{
				status_change_end(sd,SC_TENSIONRELAX,-1);
				return 0;
			}
		}
		break;
	case SC_BLEEDING:	// [celest]
		// i hope i haven't interpreted it wrong.. which i might ^^;
		// Source:
		// - 10õ©ª´ªÈªËHPª¬Êõá´
		// - õóúìªÎªŞªŞ«µ?«ĞEÔÑªä«EúÇ°ª·ªÆªEÍıªÏá¼ª¨ªÊª¤
		// To-do: bleeding effect increases damage taken?
		if ((sc_data[type].val4.num -= 10000) > 0) {
			int hp = rand()%300 + 400;
			if(sd)
			{
				sd->heal(-hp,0);
			}
			else if(md)
			{
				md->hp -= hp;
			}
			if( !bl->is_dead() )
			{
				// walking and casting effect is lost
				bl->stop_walking(1);
				skill_castcancel (bl, 0);
				sc_data[type].timer = add_timer(10000 + tick, status_change_timer, bl->id, data );
			}
			return 0;
		}
		break;

	// ŠÔØ‚ê–³‚µHH
	case SC_AETERNA:
	case SC_TRICKDEAD:
	case SC_RIDING:
	case SC_FALCON:
	case SC_WEIGHT50:
	case SC_WEIGHT90:
	case SC_MAGICPOWER:		// –‚–@—Í?•
	case SC_REJECTSWORD:	// ƒŠƒWƒFƒNƒgƒ\?ƒh
	case SC_MEMORIZE:	// ƒƒ‚ƒ‰ƒCƒY
	case SC_BROKNWEAPON:
	case SC_BROKNARMOR:
	case SC_SACRIFICE:
	case SC_READYSTORM:
	case SC_READYDOWN:
	case SC_READYTURN:
	case SC_READYCOUNTER:
	case SC_DODGE:
		sc_data[type].timer=add_timer( tick+1000*600,status_change_timer, bl->id, data );
		return 0;
	case SC_RUN:
		sc_data[type].val2 = 1; // Once the first second is spent, no more STR bonus when stopping
		sc_data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
		return 0;
	case SC_DANCING: //ƒ_ƒ“ƒXƒXƒLƒ‹‚ÌŠÔSPÁ”ï
		{
			int s = 0, sp = 1;
			if(sd && (--sc_data[type].val3.num) > 0) {
				switch(sc_data[type].val1.num){
				case BD_RICHMANKIM:				// ƒjƒˆƒ‹ƒh‚Ì‰ƒ 3•b‚ÉSP1
				case BD_DRUMBATTLEFIELD:		// ?‘¾ŒÛ‚Ì‹¿‚« 3•b‚ÉSP1
				case BD_RINGNIBELUNGEN:			// ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö 3•b‚ÉSP1
				case BD_SIEGFRIED:				// •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh 3•b‚ÉSP1
				case BA_DISSONANCE:				// •s‹¦˜a‰¹ 3•b‚ÅSP1
				case BA_ASSASSINCROSS:			// —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX 3•b‚ÅSP1
				case DC_UGLYDANCE:				// ©•ªŸè‚Èƒ_ƒ“ƒX 3•b‚ÅSP1
					s=3;
					break;
				case BD_LULLABY:				// qç‰Ì 4•b‚ÉSP1
				case BD_ETERNALCHAOS:			// ‰i‰“‚Ì¬“× 4•b‚ÉSP1
				case BD_ROKISWEIL:				// ƒƒL‚Ì‹©‚Ñ 4•b‚ÉSP1
				case DC_FORTUNEKISS:			// K‰^‚ÌƒLƒX 4•b‚ÅSP1
					s=4;
					break;
				case BD_INTOABYSS:				// [•£‚Ì’†‚É 5•b‚ÉSP1
				case BA_WHISTLE:				// Œû“J 5•b‚ÅSP1
				case DC_HUMMING:				// ƒnƒ~ƒ“ƒO 5•b‚ÅSP1
				case BA_POEMBRAGI:				// ƒuƒ‰ƒM‚Ì 5•b‚ÅSP1
				case DC_SERVICEFORYOU:			// ƒT?ƒrƒXƒtƒH?ƒ†? 5•b‚ÅSP1
				case CG_HERMODE:				// Wand of Hermod
					s=5;
					break;
				case BA_APPLEIDUN:				// ƒCƒhƒDƒ“‚Ì—ÑŒç 6•b‚ÅSP1
					s=6;
					break;
				case DC_DONTFORGETME:			// „‚ğ–Y‚ê‚È‚¢‚Åc 10•b‚ÅSP1
				case CG_MOONLIT:				// Œ–¾‚è‚Ìò‚É—‚¿‚é‰Ô‚Ñ‚ç 10•b‚ÅSP1H
					s=10;
					break;
				}
				if (s && ((sc_data[type].val3.num % s) == 0)) {
					if (sc_data[SC_LONGING].timer != -1 ||
						sc_data[type].val1.num == CG_HERMODE) {
						sp = s;						
					}
					if (sp > sd->status.sp)
					{
						///We HAVE to stop dancing, otherwise the status wears off and the skill remains in the ground! :X [Skotlex]
						sc_data[type].timer = temp_timerid; //Timer needs to be restored or stop_dancing won't work!
						skill_stop_dancing(bl,0);
						return 0; //No need to continue as skill_stop_dancing will invoke the status_change_end call.
					}
					else
					{
						sd->status.sp -= sp;
						clif_updatestatus(*sd,SP_SP);
					}
				}
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(1000+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;

	case SC_BERSERK:		// ƒo?ƒT?ƒN
		if(sd){		// HP‚ª100ˆÈã‚È‚ç??
			if( (sd->status.hp - sd->status.max_hp*5/100) > 100 ){	// 5% every 10 seconds [DracoRPG]
				sd->status.hp -= sd->status.max_hp*5/100;	// changed to max hp [celest]
				clif_updatestatus(*sd,SP_HP);
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer = add_timer(10000+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;
	case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
		if(sd){
			time_t timer;
			if(time(&timer) < ((sc_data[type].val2.num) + 3600)){	//1ŠÔ‚½‚Á‚Ä‚¢‚È‚¢‚Ì‚Å??
				// ƒ^ƒCƒ}?Äİ’è
				sc_data[type].timer=add_timer(10000+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;
	case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
		if(sd && config.muting_players){
			time_t timer;
			if((++sd->status.manner) && time(&timer) < ((sc_data[type].val2.num) + 60*(0-sd->status.manner))){	//ŠJn‚©‚çstatus.manner•ª?‚Á‚Ä‚È‚¢‚Ì‚Å??
				clif_updatestatus(*sd,SP_MANNER);
				// ƒ^ƒCƒ}?Äİ’è(60•b)
				sc_data[type].timer=add_timer(60000+tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		if (sc_data[type].val4.num % 1000 == 0) {
			char timer[32];
			snprintf(timer, sizeof(timer), "%ld", (unsigned long)(sc_data[type].val4.num/1000));
			clif_message(*bl, timer);
		}
		if((sc_data[type].val4.num -= 500) > 0) {
			sc_data[type].timer = add_timer(500 + tick, status_change_timer,bl->id, data);
				return 0;
		}
		break;

	case SC_MARIONETTE:		// ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹
	case SC_MARIONETTE2:
		{
			block_list *pbl = block_list::from_blid(sc_data[type].val3.num);
			if(pbl && battle_check_range(bl, pbl, 7) && (sc_data[type].val2.num -= 1000)>0) {
				sc_data[type].timer = add_timer(1000 + tick, status_change_timer,bl->id, data);
					return 0;
			}
		}
		break;

	// Celest
	case SC_CONFUSION:
		{
			int i = 3000;
			if(sd) 
			{
				i=1000 + rand()%1000;
			} 
			else if(md && md->mode&1 && md->is_movable()) 
			{
				if( DIFF_TICK(md->next_walktime,tick) > 7000 && md->walkpath.finished() )
				{
					i = 3000 + rand()%2000;
					md->next_walktime = tick + i;
				}
				md->randomwalk(tick);
			}
			if( sc_data[type].val2.num > 1000 ) 
			{
				sc_data[type].timer = add_timer(tick+i, status_change_timer,bl->id, data);
					return 0;
			}
		}
		break;

	case SC_GOSPEL:
		{
			int calc_flag = 0;
			if (sc_data[type].val3.num > 0) {
				sc_data[type].val3 = 0;
				calc_flag = 1;
			}
			if(sd && sc_data[type].val4.num == BCT_SELF){
				int hp, sp;
				hp = (sc_data[type].val1.num > 5) ? 45 : 30;
				sp = (sc_data[type].val1.num > 5) ? 35 : 20;
				if(sd->status.hp - hp > 0 &&
					sd->status.sp - sp > 0){
					sd->status.hp -= hp;
					sd->status.sp -= sp;
					clif_updatestatus(*sd,SP_HP);
					clif_updatestatus(*sd,SP_SP);
					if((sc_data[type].val2.num -= 10000) > 0) {
						sc_data[type].timer = add_timer(10000+tick, status_change_timer,bl->id, data);
						return 0;
					}
				}
			} else if (sd && sc_data[type].val4.num == BCT_PARTY) {
				int i;
				switch ((i = rand() % 12)) {
				case 1: // heal between 100-1000
					{
						block_list tbl;
						int heal = rand() % 900 + 100;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,AL_HEAL,heal,1);
						battle_heal(NULL,bl,heal,0,0);
					}
					break;
				case 2: // end negative status
					status_change_clear_debuffs (bl);
					break;
				case 3:	// +25% resistance to negative status
				case 4: // +25% max hp
				case 5: // +25% max sp
				case 6: // +2 to all stats
				case 11: // +25% armor and vit def
				case 12: // +8% atk
				case 13: // +5% flee
				case 14: // +5% hit
					sc_data[type].val3 = i;
					if (i == 6 ||
						(i >= 11 && i <= 14))
						calc_flag = 1;
					break;
				case 7: // level 5 bless
					{
						block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,AL_BLESSING,5,1);
						status_change_start(bl,SkillStatusChangeTable[AL_BLESSING],5,0,0,0,10000,0 );
					}
					break;
				case 8: // level 5 increase agility
					{
						block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,AL_INCAGI,5,1);
						status_change_start(bl,SkillStatusChangeTable[AL_INCAGI],5,0,0,0,10000,0 );
					}
					break;
				case 9: // holy element to weapon
					{
						block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,PR_ASPERSIO,1,1);
						status_change_start(bl,SkillStatusChangeTable[PR_ASPERSIO],1,0,0,0,10000,0 );
					}
					break;
				case 10: // holy element to armour
					{
						block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,PR_BENEDICTIO,1,1);
						status_change_start(bl,SkillStatusChangeTable[PR_BENEDICTIO],1,0,0,0,10000,0 );
					}
					break;
				default:
					break;
				}
			} else if (sc_data[type].val4.num == BCT_ENEMY) {
				int i;
				switch ((i = rand() % 8)) {
				case 1: // damage between 300-800
				case 2: // damage between 150-550 (ignore def)
					battle_damage(NULL, bl, rand() % 500,0); // temporary damage
					break;
				case 3: // random status effect
					{
						int effect[3] = {
							SC_CURSE,
							SC_BLIND,
							SC_POISON };
						status_change_start(bl,effect[rand()%3],1,0,0,0,10000,0 );
					}
					break;
				case 4: // level 10 provoke
					{
						block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(tbl,*bl,SM_PROVOKE,1,1);
						status_change_start(bl,SkillStatusChangeTable[SM_PROVOKE],10,0,0,0,10000,0 );
					}
					break;
				case 5: // 0 def
				case 6: // 0 atk
				case 7: // 0 flee
				case 8: // -75% move speed and aspd
					sc_data[type].val3 = i;
					calc_flag = 1;
					break;
				default:
					break;
				}
			}
			if (sd && calc_flag)
				status_calc_pc (*sd, 0);
		}
		break;

	case SC_GUILDAURA:
		{
			block_list *tbl = block_list::from_blid(sc_data[type].val2.num);
			if( tbl && battle_check_range(bl, tbl, 2) )
			{
				sc_data[type].timer = add_timer(1000 + tick, status_change_timer,bl->id, data);
				return 0;
			}
		}
		break;
	}
	
	// default for all non-handled control paths
	// security system to prevent forgetting timer removal
	sc_data[type].timer = temp_timerid; // to have status_change_end handle a valid timer

	return status_change_end( bl,type,tid );
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíƒ^ƒCƒ}[”ÍˆÍˆ—
 *------------------------------------------
 */
 /*
int status_change_timer_sub(block_list &bl, va_list &ap )
{
	block_list *src;
	int type;
	unsigned long tick;

	nullpo_retr(0, ap);
	src=va_arg(ap,block_list*);
	nullpo_retr(0, src);
	type=va_arg(ap,int);
	tick=va_arg(ap,unsigned long);

	if(bl.type!=BL_PC && bl.type!=BL_MOB)
		return 0;

	switch( type ){
	case SC_SIGHT:	// ƒTƒCƒg 
	case SC_CONCENTRATE:
		if( (*status_get_option(&bl))&6 ){
			status_change_end( &bl, SC_HIDING, -1);
			status_change_end( &bl, SC_CLOAKING, -1);
		}
		break;
	case SC_RUWACH:	// ƒ‹ƒAƒt 
		if( (*status_get_option(&bl))&6 ){
			struct status_change *sc_data = status_get_sc_data(&bl);	// check whether the target is hiding/cloaking [celest]
			if (sc_data && (sc_data[SC_HIDING].timer != -1 ||	// if the target is using a special hiding, i.e not using normal hiding/cloaking, don't bother
				sc_data[SC_CLOAKING].timer != -1)) {
				status_change_end( &bl, SC_HIDING, -1);
				status_change_end( &bl, SC_CLOAKING, -1);
				if(battle_check_target( src, &bl, BCT_ENEMY ) > 0)
					skill_attack(BF_MAGIC,src,src,&bl,AL_RUWACH,1,tick,0);
			}
		}
		break;
	}
	return 0;
}
*/
int CStatusChangetimer::process(block_list& bl) const
{
	if(bl==BL_PC || bl==BL_MOB)
	{
		switch( type ){
		case SC_SIGHT:	// ƒTƒCƒg 
		case SC_CONCENTRATE:
			if( (*status_get_option(&bl))&6 ){
				status_change_end( &bl, SC_HIDING, -1);
				status_change_end( &bl, SC_CLOAKING, -1);
			}
			break;
		case SC_RUWACH:	// ƒ‹ƒAƒt 
			if( (*status_get_option(&bl))&6 )
			{	
				struct status_change *sc_data = status_get_sc_data(&bl);	
				// check whether the target is hiding/cloaking [celest]
				// if the target is using a special hiding, i.e not using normal hiding/cloaking, don't bother
				if (sc_data && (sc_data[SC_HIDING].timer != -1 ||	
					sc_data[SC_CLOAKING].timer != -1)) {
					status_change_end( &bl, SC_HIDING, -1);
					status_change_end( &bl, SC_CLOAKING, -1);
					if(battle_check_target( &src, &bl, BCT_ENEMY ) > 0)
						skill_attack(BF_MAGIC,&src,&src,&bl,AL_RUWACH,1,tick,0);
				}
			}
			break;
		}
	}
	return 0;
}

int status_change_clear_buffs (block_list *bl)
{
	int i;
	struct status_change *sc_data = status_get_sc_data(bl);
	if (!sc_data)
		return 0;		
	for (i = 0; i <= 26; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 37; i <= 44; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 46; i <= 73; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 90; i <= 93; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 103; i <= 106; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 109; i <= 132; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	for (i = 172; i <= 188; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}
	return 0;
}
int status_change_clear_debuffs (block_list *bl)
{
	int i;
	struct status_change *sc_data = status_get_sc_data(bl);
	if (!sc_data)
		return 0;
	for (i = SC_STONE; i <= SC_DPOISON; ++i) {
		if(sc_data[i].timer != -1)
			status_change_end(bl,i,-1);
	}

	return 0;
}

int status_calc_sigma(void)
{
	int i,j,k;

	for(i=0;i<MAX_PC_CLASS;++i) {
		memset(hp_sigma_val[i],0,sizeof(hp_sigma_val[i]));
		for(k=0,j=2;j<=MAX_LEVEL;++j) {
			k += hp_coefficient[i]*j + 50;
			k -= k%100;
			hp_sigma_val[i][j-1] = k;
		}
	}
	return 0;
}

int status_readdb(void) {
	int i,j,k;
	FILE *fp;
	char line[1024],*p;

	// JOB•â³?’l‚P
	fp=basics::safefopen("db/job_db1.txt","r");
	if(fp==NULL){
		ShowError("can't read db/job_db1.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp)){
		char *split[50];
		if( !is_valid_line(line) )
			continue;
		for(j=0,p=line;j<21 && p;++j){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j<21)
			continue;
		max_weight_base[i]=atoi(split[0]);
		hp_coefficient[i]=atoi(split[1]);
		hp_coefficient2[i]=atoi(split[2]);
		sp_coefficient[i]=atoi(split[3]);
		for(j=0;j<17;++j)
			aspd_base[i][j]=atoi(split[j+4]);
		++i;
// -- moonsoul (below two lines added to accommodate high numbered new class ids)
		if(i==24)
			i=4001;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db1.txt");

	// JOBƒ{?ƒiƒX
	memset(job_bonus,0,sizeof(job_bonus));
	fp=basics::safefopen("db/job_db2.txt","r");
	if(fp==NULL){
		ShowError("can't read db/job_db2.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp))
	{
		if( !is_valid_line(line) )
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;++j)
		{
			if(sscanf(p,"%d",&k)==0)
				break;
			job_bonus[0][i][j]=k;
			job_bonus[2][i][j]=k; //—{qE‚Ìƒ{?ƒiƒX‚Í•ª‚©‚ç‚È‚¢‚Ì‚Å?
			p=strchr(p,',');
			if(p) p++;
		}
		++i;
// -- moonsoul (below two lines added to accommodate high numbered new class ids)
		if(i==24)
			i=4001;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db2.txt");

	// JOBƒ{?ƒiƒX2 ?¶E—p
	fp=basics::safefopen("db/job_db2-2.txt","r");
	if(fp==NULL){
		ShowError("can't read db/job_db2-2.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp))
	{
		if( !is_valid_line(line) )
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;++j)
		{
			if(sscanf(p,"%d",&k)==0)
				break;
			job_bonus[1][i][j]=k;
			p=strchr(p,',');
			if(p) p++;
		}
		++i;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db2-2.txt");

	// ƒTƒCƒY•â³ƒe?ƒuƒ‹
	for(i=0;i<3;++i)
		for(j=0;j<20;++j)
			atkmods[i][j]=100;
	fp=basics::safefopen("db/size_fix.txt","r");
	if(fp==NULL){
		ShowError("can't read db/size_fix.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp))
	{
		char *split[20];
		if( !is_valid_line(line) )
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<20 && p;++j)
		{
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0;j<20 && split[j];++j)
			atkmods[i][j]=atoi(split[j]);
		++i;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/size_fix.txt");

	// ¸?ƒf?ƒ^ƒe?ƒuƒ‹
	for(i=0;i<MAX_REFINE_BONUS;++i){
		for(j=0;j<MAX_REFINE; ++j)
			percentrefinery[i][j]=100;
		percentrefinery[i][j]=0; //Slot MAX+1 always has 0% success chance [Skotlex]
		refinebonus[i][0]=0;
		refinebonus[i][1]=0;
		refinebonus[i][2]=10;
	}
	fp=basics::safefopen("db/refine_db.txt","r");
	if(fp==NULL){
		ShowError("can't read db/refine_db.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line), fp) && i<MAX_REFINE_BONUS)
	{
		char *split[16];
		if( !is_valid_line(line) )
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<16 && p;++j)
		{
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		refinebonus[i][0]=atoi(split[0]);	// ¸?ƒ{?ƒiƒX
		refinebonus[i][1]=atoi(split[1]);	// ‰ß?¸?ƒ{?ƒiƒX
		refinebonus[i][2]=atoi(split[2]);	// ˆÀ‘S¸?ŒÀŠE
		for(j=0;j<MAX_REFINE && split[j];++j)
			percentrefinery[i][j]=atoi(split[j+3]);
		++i;
	}
	fclose(fp); //Lupus. close this file!!!
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/refine_db.txt");

	return 0;
}

/*==========================================
 * ƒXƒLƒ‹ŠÖŒW‰Šú‰»ˆ—
 *------------------------------------------
 */
int do_init_status(void)
{
	add_timer_func_list(status_change_timer,"status_change_timer");
	status_readdb();
	status_calc_sigma();
	return 0;
}
