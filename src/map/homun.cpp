// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/* homun.c
	関数名：homun_*		*hom*
	ホムンクルス ID 6001-6016	※2006/03/14現在
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "timer.h"
#include "socket.h"
#include "nullpo.h"
#include "malloc.h"
#include "pc.h"
#include "map.h"
#include "intif.h"
#include "clif.h"
#include "chrif.h"
#include "homun.h"
#include "itemdb.h"
#include "battle.h"
#include "mob.h"
#include "npc.h"
#include "script.h"
#include "status.h"
#include "skill.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

struct homun_db homun_db[MAX_HOMUN_DB];
struct random_homun_data embryo_data[MAX_HOMUN_DB];
int embryo_count=0;
int embryo_default=6001;

static int homun_exp_table[6][MAX_LEVEL];
int homun_count=0;

const static char dirx[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
const static char diry[8] = { 1, 1, 0,-1,-1,-1, 0, 1};


static struct dbt* homun_id_db=NULL;

static struct
{
	unsigned short id;
	unsigned short max;
	struct
	{
		unsigned short id;
		unsigned short lv;
	} need[6];
	unsigned short base_level;
	unsigned short intimate;
} homun_skill_tree[MAX_HOMUN_DB][100];


static int create_homunculus_id()
{
	int i;
	while(embryo_count)
	{
		i = rand()%homun_count;
		if(rand()%1000000 < embryo_data[i].per)
			return embryo_data[i].homunid;
	}
	return embryo_default;
}



int homun_natural_heal_hp(int tid,unsigned long tick,int id, basics::numptr data);
int homun_natural_heal_sp(int tid,unsigned long tick,int id, basics::numptr data);







/// constructor. needs char_id of associated charater for creation
homun_data::homun_data(map_session_data &sd) :
	invincible_timer(-1),
	natural_heal_hp_timer(-1),
	natural_heal_sp_timer(-1),
	hungry_timer(-1),
	hungry_cry_timer(-1),
	msd(sd)
{
	this->status.char_id = sd.status.char_id;

	// check if local database already contains an element
	// (should actually not happen, but just to be sure)
	struct homun_data *hd = (struct homun_data *)numdb_search(homun_id_db, sd.status.char_id);
	block_list::freeblock_lock();
	
	if(hd)
	{
		intif_delete_homdata(hd->status.account_id, hd->status.char_id, hd->status.homun_id);
		hd->freeblock();
	}
	block_list::freeblock_unlock();

	// insert to local database
	numdb_insert(homun_id_db, sd.status.char_id, this);
	this->msd.hd = this;
}
/// destructor. does automatic cleanup
homun_data::~homun_data()
{
	// delete timers when exist
	this->delete_natural_heal_timer();
	this->delete_hungry_timer();

	// remove from local database
	numdb_erase(homun_id_db, this->status.char_id);
	this->msd.hd = NULL;
}
/*
homun_data *homun_data::get_homunculus(uint32 char_id)
{
	return (struct homun_data *)numdb_search(homun_id_db, char_id);
}

homun_data *homun_data::get_homunculus(const map_session_data &sd)
{
	return homun_data::get_homunculus(sd.status.char_id);
}
*/
void homun_data::clear_homunculus(map_session_data &sd)
{
	if( sd.hd )
	{
		sd.hd->save_data();
		sd.hd->freeblock();
		sd.hd=NULL;
	}
}

void homun_data::recv_homunculus(struct homunstatus &p, int flag)
{
	map_session_data *sd = map_session_data::charid2sd(p.char_id);

	if(sd == NULL || !sd->skill_check( AM_CALLHOMUN ) )
		return;

	if( !sd->hd )
	{
		sd->hd = new homun_data(*sd);
	}
	else if(sd->hd->status.homun_id != p.homun_id)
	{
		intif_delete_homdata(sd->hd->status.account_id, sd->hd->status.char_id, sd->hd->status.homun_id);
	}

	// apply the received data
	sd->hd->status = p;

	if( sd->status.homun_id > 0 )
	{	
		// if there is no bioethics skill, homunculus cannot be hatched
		if(!sd->skill_check( AM_BIOETHICS))
			sd->hd->status.incubate = 0;
		// do the hatching, when incobated and alive
		if(sd->hd->status.incubate && sd->hd->status.hp > 0)
		{
			homun_data::call_homunculus(*sd);
			clif_homskillinfoblock(*sd);
		}
	}
	else
	{	// receive the data first time after creation
		sd->status.homun_id = sd->hd->status.homun_id;
		homun_data::call_homunculus(*sd);
	}
	return;
}

void homun_data::delete_data()
{
	//親密度保存
//	if(config.save_homun_temporal_intimate)
//		pc_setglobalreg(this->msd,"HOM_TEMP_INTIMATE",2000);//初期値に

	this->status.incubate = 0;
	this->save_data();

	intif_delete_homdata(this->msd.status.account_id, this->msd.status.char_id, this->status.homun_id);

	this->msd.hd=NULL;
	this->msd.status.homun_id = 0;
	pc_makesavestatus(this->msd);
	chrif_save(this->msd);
	storage_storage_save(this->msd);

	this->delblock();
	this->deliddb();
	this->freeblock();
}

void homun_data::save_data()
{
//	if(config.save_homun_temporal_intimate)
//		pc_setglobalreg(sd,"HOM_TEMP_INTIMATE",hd->intimate);
	intif_save_homdata(this->status.account_id, this->status.char_id, *this);
}

int homun_hungry_cry(int tid, unsigned long tick, int id, basics::numptr data)
{
	homun_data *hd = homun_data::from_blid(id);
	if( hd )
	{
		if( hd->hungry_cry_timer != tid )
		{
			if(config.error_log)
				ShowError("homun_hungry_cry_timer %d != %d\n", hd->hungry_cry_timer, tid);
			return 0;
		}
		hd->hungry_cry_timer = -1;
		clif_emotion(*hd,28);
		hd->hungry_cry_timer = add_timer(tick+20*1000, homun_hungry_cry, hd->block_list::id, 0);
	}
	return 0;
}

int homun_hungry(int tid, unsigned long tick, int id, basics::numptr data)
{
	block_list *bl = homun_data::from_blid(id);
	homun_data * hd;
	if(bl && (hd = bl->get_hd()) )
	{
		if(hd->hungry_timer != tid)
		{
			if(config.error_log)
				ShowError("homun_hungry_timer %d != %d\n", hd->hungry_timer, tid);
			return 0;
		}
		hd->hungry_timer = -1;
		
		if( hd->status.hungry>1 )
			hd->status.hungry--;
		if( hd->status.hungry <= 10 )
		{	// 10以下で減り始める(泣きエモを20秒に1回出すようになる)
			int f=0;
			if( hd->intimate == hd->status.intimate )
				f=1;
			hd->status.intimate -= 20*config.homun_intimate_rate/100;
			clif_emotion(*hd,28);
			clif_send_homdata(hd->msd, 0x100, hd->intimate/100);
			if( hd->status.intimate <= 0 )
				hd->status.intimate = 0;
			if(f)
				hd->intimate = hd->status.intimate;
			
			if( hd->hungry_cry_timer == -1 )
				hd->hungry_cry_timer = add_timer(tick+20*1000,homun_hungry_cry, hd->block_list::id, 0);
		}
		else if( hd->hungry_cry_timer != -1 )
		{
			delete_timer(hd->hungry_cry_timer,homun_hungry_cry);
			hd->hungry_cry_timer = -1;
		}
		clif_send_homdata(hd->msd, 2, hd->status.hungry);
		// 本鯖ではここでステータスを送らないが、送らないと"ホムが腹ぺこです！"が出ない
		clif_send_homstatus(hd->msd,0);
		
		hd->hungry_timer = add_timer(tick+60*1000, homun_hungry, hd->block_list::id, 0);
	}
	return 0;
}




int homun_upstatus(struct homunstatus &hd)
{
	int total,class_,ret=0;
	class_ = hd.class_-HOM_ID;

	total = homun_db[class_].str_k+homun_db[class_].agi_k+homun_db[class_].vit_k+homun_db[class_].int_k+homun_db[class_].dex_k+homun_db[class_].luk_k;
	if(total <= 0) return 0;
// STポイントが足りなかった場合、
//・次にレベルが上がるまでSTポイントを保存するか	return 0
//・振れるだけ振るか								continue
#define AUT_ST_RET return 0
//#define AUT_ST_RET continue
	while(hd.status_point && ret<64)
	{
		int s = rand()%total;
		int require;
		ret++;
		if((s-=homun_db[class_].str_k) <0){
			require = (hd.str==0)?1:(hd.str-1)/10+2;
			if(require < hd.status_point){
				hd.str++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else if((s-=homun_db[class_].agi_k) <0){
			require = (hd.agi==0)?1:(hd.agi-1)/10+2;
			if(require < hd.status_point){
				hd.agi++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else if((s-=homun_db[class_].vit_k) <0){
			require = (hd.vit==0)?1:(hd.vit-1)/10+2;
			if(require < hd.status_point){
				hd.vit++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else if((s-=homun_db[class_].int_k)<0){
			require = (hd.int_==0)?1:(hd.int_-1)/10+2;
			if(require < hd.status_point){
				hd.int_++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else if((s-=homun_db[class_].dex_k) <0){
			require = (hd.dex==0)?1:(hd.dex-1)/10+2;
			if(require < hd.status_point){
				hd.dex++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else if((s-=homun_db[class_].luk_k) <0){
			require = (hd.luk==0)?1:(hd.luk-1)/10+2;
			if(require < hd.status_point){
				hd.luk++;
				hd.status_point-=require;
			}else{
				AUT_ST_RET;
			}
		}else{
			AUT_ST_RET;
		}
	}
	return 0;
}


int homun_upstatus2(struct homunstatus &hd)
{
	int class_;
	class_ = hd.class_-HOM_ID;
	// 各BasePointに1割くらいの追加でボーナス上昇？
	if(rand()%100 < homun_db[class_].str_k)
		hd.str += (rand()%(homun_db[class_].base*100-90))/100+1;
	if(rand()%100 < homun_db[class_].agi_k)
		hd.agi += (rand()%(homun_db[class_].base*100-90))/100+1;
	if(rand()%100 < homun_db[class_].vit_k)
		hd.vit += (rand()%(homun_db[class_].base*100-90))/100+1;
	if(rand()%100 < homun_db[class_].int_k)
		hd.int_+= (rand()%(homun_db[class_].base*100-90))/100+1;
	if(rand()%100 < homun_db[class_].dex_k)
		hd.dex += (rand()%(homun_db[class_].base*100-90))/100+1;
	if(rand()%100 < homun_db[class_].luk_k)
		hd.luk += (rand()%(homun_db[class_].base*100-90))/100+1;
	return 0;
}


void homun_data::calc_status()
{
	homun_data &hd = *this;
	int dstr,base_level,aspd_k,lv;
	int aspd_rate=100,speed_rate=100,atk_rate=100,matk_rate=100,hp_rate=100,sp_rate=100;
	int flee_rate=100,def_rate=100,mdef_rate=100,critical_rate=100,hit_rate=100;
	hd.atk		= 0;
	hd.matk	= 0;
	hd.hit		= 0;
	hd.flee	= 0;
	hd.def		= 0;
	hd.mdef	= 0;
	hd.critical= 0;
	hd.max_hp = hd.status.max_hp;
	hd.max_sp = hd.status.max_sp;
	hd.str = hd.status.str;
	hd.agi = hd.status.agi;
	hd.vit = hd.status.vit;
	hd.dex = hd.status.dex;
	hd.int_= hd.status.int_;
	hd.luk = hd.status.luk;
	hd.speed = 150;
	hd.nhealhp=0;
	hd.nhealsp=0;
	hd.hprecov_rate=100;
	hd.sprecov_rate=100;
	//チェンジインストラクション
	if((lv = hd.checkskill(HVAN_INSTRUCT))>0)
	{
		static int instruct_str[11]={0,1,1,3,4,4,6,7,7,9,10};//10まで拡張
		hd.str+= instruct_str[lv];
		hd.int_+= lv;
	}
	//脳手術
	if((lv = hd.checkskill(HLIF_BRAIN))>0)
	{
		sp_rate += lv;
		hd.sprecov_rate+=lv*3;
	}
	//アダマンティウムスキン
	if((lv = hd.checkskill(HAMI_SKIN))>0)
	{
		hp_rate += lv*2;
		hd.def += lv*4;
		hd.hprecov_rate+=lv*5;
	}
	// ステータス変化による基本パラメータ補正ホムスキル
	if(hd.sc_data)
	{
		//緊急回避
		if( hd.has_status(SC_AVOID) )
			speed_rate -= hd.sc_data[SC_AVOID].integer1()*10;
		//
		if( hd.has_status(SC_CHANGE) )
			hd.int_ += 60;
		//ブラッドラスト
		if( hd.has_status(SC_BLOODLUST) )
			atk_rate += hd.sc_data[SC_BLOODLUST].integer1()*10+20;
		//フリットムーブ
		if( hd.has_status(SC_FLEET) )
		{
			aspd_rate -= hd.sc_data[SC_FLEET].integer1()*3;
			atk_rate+=5+hd.sc_data[SC_FLEET].integer1()*5;
		}
	}
	// ステータス変化による基本パラメータ補正
	if(config.allow_homun_status_change && hd.sc_data)
	{
		//ゴスペルALL+20
		if( hd.has_status(SC_INCALLSTATUS) )
		{
			hd.str+= hd.sc_data[SC_INCALLSTATUS].integer1();
			hd.agi+= hd.sc_data[SC_INCALLSTATUS].integer1();
			hd.vit+= hd.sc_data[SC_INCALLSTATUS].integer1();
			hd.int_+= hd.sc_data[SC_INCALLSTATUS].integer1();
			hd.dex+= hd.sc_data[SC_INCALLSTATUS].integer1();
			hd.luk+= hd.sc_data[SC_INCALLSTATUS].integer1();
		}

		if( hd.has_status(SC_INCREASEAGI) )	// 速度増加
			hd.agi += 2+hd.sc_data[SC_INCREASEAGI].integer1();

		if( hd.has_status(SC_DECREASEAGI) )	// 速度減少(agiはbattle.cで)
			hd.agi-= 2+hd.sc_data[SC_DECREASEAGI].integer1();

		if( hd.has_status(SC_BLESSING) )
		{	// ブレッシング
			hd.str+= hd.sc_data[SC_BLESSING].integer1();
			hd.dex+= hd.sc_data[SC_BLESSING].integer1();
			hd.int_+= hd.sc_data[SC_BLESSING].integer1();
		}
		if( hd.has_status(SC_SUITON) )
		{	// 水遁
			if(hd.sc_data[SC_SUITON].integer3())
				hd.agi+=hd.sc_data[SC_SUITON].integer3();
			if(hd.sc_data[SC_SUITON].integer4())
				hd.speed = hd.speed*2;
		}

		if( hd.has_status(SC_GLORIA) )	// グロリア
			hd.luk+= 30;
			
		if( hd.has_status(SC_QUAGMIRE) )
		{	// クァグマイア
			short subagi = 0;
			short subdex = 0;
			subagi = (hd.status.agi/2 < hd.sc_data[SC_QUAGMIRE].integer1()*10) ? hd.status.agi/2 : hd.sc_data[SC_QUAGMIRE].integer1()*10;
			subdex = (hd.status.dex/2 < hd.sc_data[SC_QUAGMIRE].integer1()*10) ? hd.status.dex/2 : hd.sc_data[SC_QUAGMIRE].integer1()*10;
			if(maps[hd.block_list::m].flag.pvp || maps[hd.block_list::m].flag.gvg){
				subagi/= 2;
				subdex/= 2;
			}
			hd.speed = hd.speed*4/3;
			hd.agi-= subagi;
			hd.dex-= subdex;
		}
	}

	dstr		= hd.str / 10;
	base_level	= hd.status.base_level;
	aspd_k		= homun_db[hd.status.class_-HOM_ID].aspd_k;
	
	hd.atk		+= hd.str * 2 + base_level + dstr * dstr;
	hd.matk		+= hd.int_+(hd.int_/ 5) * (hd.int_/ 5);
	hd.hit		+= hd.dex + base_level;
	hd.flee		+= hd.agi + base_level;
	hd.def		+= hd.vit + hd.vit / 5 + base_level / 10;
	hd.mdef		+= hd.int_/ 5 + base_level / 10;
	hd.critical	+= hd.luk / 3 + 1;
	hd.aspd		 = aspd_k - (aspd_k * hd.agi / 250 + aspd_k * hd.dex / 1000);
	hd.aspd		-= 200;
	
	//ディフェンス
	if( hd.has_status(SC_DEFENCE) )
		hd.def += hd.sc_data[SC_DEFENCE].integer1()*2;
	//オーバードスピード
	if( hd.has_status(SC_SPEED) )
		hd.flee = hd.flee + 10 + hd.sc_data[SC_SPEED].integer1()*10;
	//補正
	if(atk_rate!=100)
		hd.atk = hd.atk*atk_rate/100;
	if(matk_rate!=100)
		hd.matk = hd.matk*matk_rate/100;
	if(hit_rate!=100)
		hd.hit = hd.hit*hit_rate/100;
	if(flee_rate!=100)
		hd.flee = hd.flee*flee_rate/100;
	if(def_rate!=100)
		hd.def = hd.def*def_rate/100;
	if(mdef_rate!=100)
		hd.mdef = hd.mdef*mdef_rate/100;
	if(critical_rate!=100)
		hd.critical = hd.critical*critical_rate/100;
	if(hp_rate!=100)
		hd.max_hp = hd.max_hp*hp_rate/100;
	if(sp_rate!=100)
		hd.max_sp = hd.max_sp*sp_rate/100;		
	if(aspd_rate!=100)
		hd.aspd = hd.aspd*aspd_rate/100;
	if(speed_rate!=100)
		hd.speed = hd.speed*speed_rate/100;
		
	//メンタルチェンジ
	if( hd.has_status(SC_CHANGE) )
	{
		int atk_,hp_;
		//
		atk_= hd.atk;
		hd.atk = hd.matk;
		hd.matk = atk_;
		//
		hp_= hd.max_hp;
		hd.max_hp = hd.max_sp;
		hd.max_sp = hp_;
	}
	if(hd.max_hp<=0) hd.max_hp=1;	// mhp 0 だとクライアントエラー
	if(hd.max_sp<=0) hd.max_sp=1;
	//自然回復
	hd.nhealhp = hd.max_hp/100 + hd.vit/5 + 2;
	hd.nhealsp = (hd.int_/6)+(hd.max_sp/100)+1;
	if(hd.int_ >= 120)
		hd.nhealsp += ((hd.int_-120)>>1) + 4;
	if(hd.hprecov_rate!=100)
		hd.nhealhp = hd.nhealhp*hd.hprecov_rate/100;
	if(hd.sprecov_rate!=100)
		hd.nhealsp = hd.nhealsp*hd.sprecov_rate/100;
	
	hd.calc_skilltree();
}


void homun_data::recalc_status()
{
	homun_data &hd = *this;
	int lv,class_,hp,sp;
	class_ = hd.status.class_-HOM_ID;
	hd.status.max_hp = hd.status.hp = homun_db[class_].hp;
	hd.status.max_sp = hd.status.sp = homun_db[class_].sp;
	hd.status.str = homun_db[class_].str;
	hd.status.agi = homun_db[class_].agi;
	hd.status.vit = homun_db[class_].vit;
	hd.status.int_= homun_db[class_].int_;
	hd.status.dex = homun_db[class_].dex;
	hd.status.luk = homun_db[class_].luk;
	for(lv=1;lv<hd.status.base_level;lv++)
	{
		// 実測値の、最大値～最小値でランダム上昇
		hp = homun_db[hd.status.class_-HOM_ID].hp_kmax-homun_db[hd.status.class_-HOM_ID].hp_kmin;
		hd.status.max_hp += homun_db[hd.status.class_-HOM_ID].hp_kmin + rand()%hp;
		sp = homun_db[hd.status.class_-HOM_ID].sp_kmax-homun_db[hd.status.class_-HOM_ID].sp_kmin;
		hd.status.max_sp += homun_db[hd.status.class_-HOM_ID].sp_kmin + rand()%sp;
		//	homun_upstatus(&sd->hd.status);	// オートステ振り(statuspoint方式)
		homun_upstatus2(hd.status);		// ステアップ計算
		hd.calc_status();			// ステータス計算
	}
}




homun_data *homun_data::create_homunculus(map_session_data &sd, unsigned short homunid)
{
	// 作成初期値　新密度:2000/100000　満腹度:50/100
	int class_ = homunid-HOM_ID;
	if( !sd.hd )
	{
		sd.hd = new homun_data(sd);
	}

	sd.hd->status.class_ = homunid;
	sd.hd->status.account_id = sd.status.account_id;
	sd.hd->status.char_id = sd.status.char_id;
	memcpy(sd.hd->status.name, homun_db[class_].jname, 24);
	sd.hd->status.base_level = homun_db[class_].base_level;
	sd.hd->status.base_exp = 0;
	sd.hd->status.max_hp = 1;
	sd.hd->status.max_sp = 0;
	sd.hd->status.status_point = 0;
	sd.hd->status.skill_point = homun_db[class_].skillpoint; //初期スキルポイント導入するかも…成長しないホム用

	// 初期ステータスをDBから埋め込み
	sd.hd->status.max_hp = sd.hd->status.hp = homun_db[class_].hp;
	sd.hd->status.max_sp = sd.hd->status.sp = homun_db[class_].sp;

	sd.hd->status.str = homun_db[class_].str;
	sd.hd->status.agi = homun_db[class_].agi;
	sd.hd->status.vit = homun_db[class_].vit;
	sd.hd->status.int_= homun_db[class_].int_;
	sd.hd->status.dex = homun_db[class_].dex;
	sd.hd->status.luk = homun_db[class_].luk;

	sd.hd->status.equip =  0;
	sd.hd->status.intimate = 2000;
	sd.hd->status.hungry = 50;
	sd.hd->status.incubate = 0;
	sd.hd->status.rename_flag = 0;
	
//	if(config.save_homun_temporal_intimate)
//		pc_setglobalreg(sd,"HOM_TEMP_INTIMATE", sd.hd->intimate);

	intif_create_homdata(sd.status.account_id, sd.status.char_id, *sd.hd);

	return sd.hd;
}

bool homun_data::return_to_embryo()
{
	if( !this->is_dead() )
	{	//親密度保存
//		if(config.save_homun_temporal_intimate)
//			pc_setglobalreg(sd,"HOM_TEMP_INTIMATE", hd->intimate);
		this->status.incubate = 0;
		this->save_data();

		this->delblock();
		this->deliddb();
		this->freeblock();
		return true;
	}
	return false;
}


bool homun_data::revive(unsigned short skilllv)
{
	if( this->status.hp==0 )
	{	// 蘇生時HP = 死亡時HP（≦0）+ MAXHP * (Skill Lv * 0.2)
		this->status.hp = this->status.max_hp * skilllv / 5;
		if( this->status.hp>this->status.max_hp )
			this->status.hp = this->status.max_hp;
		homun_data::call_homunculus(this->msd);
		return true;
	}
	return false;
}


void homun_data::food()
{
	int i,t,food,class_,emotion;
	class_ = this->status.class_-HOM_ID;

	food = homun_db[class_].FoodID;

	i = pc_search_inventory(this->msd,food);
	if(i < 0)
	{
		clif_hom_food(this->msd,food,0);
	}
	else
	{
		pc_delitem(this->msd,i,1,0);
		t = this->status.hungry;
		if(t > 90)
		{
			this->status.intimate -= 50*config.homun_intimate_rate/100;
			this->intimate -= 50*config.homun_intimate_rate/100;
			emotion = 16;
		}
		else if(t > 75)
		{
			this->status.intimate -= 30*config.homun_intimate_rate/100;
			this->intimate -= 30*config.homun_intimate_rate/100;
			emotion = 19;
		}
		else if(t > 25)
		{
			this->status.intimate += 80*config.homun_intimate_rate/100;
			this->intimate += 80*config.homun_intimate_rate/100;
			emotion = 2;
		}
		else if(t > 10)
		{
			this->status.intimate +=100*config.homun_intimate_rate/100;
			this->intimate +=100*config.homun_intimate_rate/100;
			emotion = 2;
		}
		else
		{
			this->status.intimate += 50*config.homun_intimate_rate/100;
			this->intimate += 50*config.homun_intimate_rate/100;
			emotion = 2;
		}
		if( this->status.intimate <= 0 )
			this->status.intimate = 0;
		if( this->status.intimate > 100000 )
			this->status.intimate = 100000;
		if( this->intimate <= 0 )
			this->intimate = 0;
		if( this->intimate > 100000 )
			this->intimate = 100000;
		this->status.hungry += 10;
		if(this->status.hungry > 100)
			this->status.hungry = 100;

		if(this->hungry_cry_timer != -1)
		{
			delete_timer(this->hungry_cry_timer,homun_hungry_cry);
			this->hungry_cry_timer = -1;
		}

		clif_emotion(this->msd, emotion);
		clif_send_homdata(this->msd, 2, this->status.hungry);
		clif_send_homdata(this->msd, 0x100, this->intimate/100);
		clif_send_homstatus(this->msd, 0);
		clif_hom_food(this->msd, food, 1);
	}
}


void homun_data::menu(unsigned short menunum)
{
	switch(menunum)
	{
		case 0:
			clif_send_homstatus(this->msd,0);
			break;
		case 1:
			this->food();
			break;
		case 2:
			this->delete_data();
			break;
	}
}


void homun_data::return_to_master()
{
	this->random_walktarget(this->msd);
	this->walktoxy(this->walktarget.x,this->walktarget.y);
}


bool homun_data::change_name(const char *name)
{
	if( this->status.rename_flag == 0 || config.pet_rename == 1 )
	{
		int i;
		for(i=0;i<24 && name[i];i++)
		{
			if( !(name[i]&0xe0) || name[i]==0x7f )
				return 1;
		}
		this->stop_walking(1);
		safestrcpy(this->status.name, sizeof(this->status.name), name);
		this->save_data();
		clif_clearchar_area(*this,0);
		clif_spawnhom(*this);
		clif_send_homstatus(this->msd,1);
		clif_send_homstatus(this->msd,0);
		this->status.rename_flag = 1;
	//	clif_hom_equip(this->msd);
		clif_send_homstatus(this->msd,0);
		return true;
	}
	return false;
}


int homun_data::checkskill(unsigned short skill_id)
{
	if(skill_id >= HOM_SKILLID) skill_id -= HOM_SKILLID;
	if(skill_id >= MAX_HOMSKILL) return 0;
	if(this->status.skill[skill_id].id == skill_id+HOM_SKILLID)
		return (this->status.skill[skill_id].lv);
	return 0;
}


bool homun_data::skillup(unsigned short skill_id)
{
	unsigned short skill_num = skill_id;
	if(skill_id >= HOM_SKILLID) skill_id -= HOM_SKILLID;
	if(skill_id < MAX_HOMSKILL)
	{
		if( this->status.skill_point>0 &&
			this->status.skill[skill_id].id!=0 &&
			this->status.skill[skill_id].lv < skill_get_max(skill_num) )
		{
			++this->status.skill[skill_id].lv;
			--this->status.skill_point;
			this->calc_skilltree();

			clif_homskillup(this->msd,skill_num);
			clif_send_homstatus(this->msd,0);
			clif_homskillinfoblock(this->msd);
			return true;
		}
	}
	return false;
}


void homun_data::calc_skilltree()
{
	int i,id=0;
	int c=0,flag;

	c = this->status.class_-HOM_ID;

	for(i=0;i<MAX_HOMSKILL;i++)
		this->status.skill[i].id=0;
	do
	{
		flag=0;
		for(i=0;(id=homun_skill_tree[c][i].id)>0;i++){
			int j,f=1;
			for(j=0;j<5;j++) {
				if( homun_skill_tree[c][i].need[j].id &&
					this->checkskill(homun_skill_tree[c][i].need[j].id) < homun_skill_tree[c][i].need[j].lv)
					f=0;
			}
			if(this->status.base_level < homun_skill_tree[c][i].base_level)
				f=0;
			if(this->status.intimate < homun_skill_tree[c][j].intimate)
				f = 0;
			id=id-HOM_SKILLID;
			if(f && this->status.skill[id].id==0 && id>=0){
				this->status.skill[id].id=id+HOM_SKILLID;
				flag=1;
			}
		}
	}while(flag);
}


bool homun_data::check_baselevelup()
{
	int next = this->next_baseexp();
	int hp,sp;

	if( next > 0 && this->status.base_exp >= (uint32)next)
	{
		// base側レベルアップ処理
		this->status.base_exp -= next;

		this->status.base_level++;
	//	this->status.status_point += 15 + (hd->status.base_level+14)/3;	// 微調整してもうまくいかず･･･
		if(this->status.base_level%3==0)	// 3レベル毎にSkillPoint加算
			this->status.skill_point ++;

		// 実測値の、最大値～最小値でランダム上昇
		hp = homun_db[this->status.class_-HOM_ID].hp_kmax-homun_db[this->status.class_-HOM_ID].hp_kmin;
		this->status.max_hp += homun_db[this->status.class_-HOM_ID].hp_kmin + rand()%hp;
		sp = homun_db[this->status.class_-HOM_ID].sp_kmax-homun_db[this->status.class_-HOM_ID].sp_kmin;
		this->status.max_sp += homun_db[this->status.class_-HOM_ID].sp_kmin + rand()%sp;

	//	homun_upstatus(this->status);	// オートステ振り(statuspoint方式)
		homun_upstatus2(this->status);	// ステアップ計算
		this->calc_status();			// ステータス計算
		this->heal(this->max_hp,this->max_sp);
		clif_setareaeffect(*this,568);
		clif_send_homstatus(this->msd, 0);
		clif_homskillinfoblock(this->msd);

		return true;
	}

	return false;
}

void homun_data::gain_exp(uint32 base_exp, uint32 job_exp, block_list &obj)
{
	int next;
	uint64 bexp=base_exp, jexp=job_exp;
	int mbexp=0,mjexp=0;
	uint64 per;

	if( !this->is_on_map() || this->is_dead() )
		return;

	mob_data *md = obj.get_md();
	if(md && md->has_status(SC_RICHMANKIM) )
	{
		bexp = bexp*(125 + md->sc_data[SC_RICHMANKIM].integer1()*11)/100;
		jexp = jexp*(125 + md->sc_data[SC_RICHMANKIM].integer1()*11)/100;
	}
	base_exp = (bexp>0x7fffffff)? 0x7fffffff: (int)bexp;
	job_exp  = (jexp>0x7fffffff)? 0x7fffffff: (int)jexp;

	if(config.master_get_homun_base_exp)
		mbexp = base_exp;
	if(config.master_get_homun_job_exp)
		mjexp = job_exp;

	pc_gainexp(this->msd,mbexp,mjexp);

	per = 150;//config.next_exp_limit;
	if(base_exp>0)
	{
		next=this->next_baseexp();
		if(next>0)
		{
			while((this->status.base_exp + base_exp) >= (uint32)next)
			{	// LvUP
				int temp_exp = next - this->status.base_exp;
				if( (per-(100-((uint64)this->status.base_exp)*100/next)) <0)
					break;
				per -= (100-((uint64)this->status.base_exp)*100/next);
				this->status.base_exp = next;
				if(!this->check_baselevelup() || (next = this->next_baseexp())<=0) break;
				base_exp -= temp_exp;
			}
			if((next=this->next_baseexp())>0 && (((uint64)base_exp) * 100 / next) > per)
				this->status.base_exp = (uint32)( next * per / 100 );
			else
				this->status.base_exp += base_exp;

			if(this->status.base_exp < 0)
				this->status.base_exp = 0;
			this->check_baselevelup();
		}
		else
		{
			this->status.base_exp += base_exp;
		}
		clif_send_homstatus(this->msd, 0);
	}
}


int homun_data::next_baseexp() const
{
	int i;
	if( this->status.base_level>=MAX_LEVEL || this->status.base_level<=0 )
		return 0;
	i = homun_db[this->status.class_-HOM_ID].exp_table;
	return homun_exp_table[i][this->status.base_level-1];
}


int homun_data::damage(block_list &src, uint32 damage)
{
	homun_data &hd = *this;
	// 既に死んでいたら無効
	if( !hd.is_on_map() || hd.is_dead() )
		return 0;

	// 歩いていたら足を止める
	hd.stop_walking(2);

//	if(damage>0)
//		skill_stop_gravitation(&hd->bl);

	if(hd.status.hp > hd.max_hp)
		hd.status.hp = hd.max_hp;

	// over kill分は丸める
	if(damage>hd.status.hp)
		damage=hd.status.hp;

	hd.status.hp-=damage;

	if(hd.status.option&2 )
		status_change_end(&hd, SC_HIDING, -1);
	if(hd.status.option&4 )
		status_change_end(&hd, SC_CLOAKING, -1);

	clif_send_homstatus(hd.msd, 0);

	// 死亡していた
	if(hd.status.hp<=0)
	{
		this->set_dead();
	}
	return damage;
}


int homun_data::heal(int hp, int sp)
{
	homun_data &hd = *this;
	if( hd.has_status(SC_BERSERK) )
	{
		if (sp > 0) sp = 0;
		if (hp > 0) hp = 0;
	}

	if(hp+hd.status.hp > hd.max_hp)
		hp = hd.max_hp - hd.status.hp;
	else if(hp<0 && ((uint32)(-hp)) > hd.status.hp)
		hp = 0-hd.status.hp;

	if(sp+hd.status.sp > hd.max_sp)
		sp = hd.max_sp - hd.status.sp;
	else if(sp<0 && ((uint32)(-sp)) > hd.status.sp)
		sp = 0-hd.status.sp;

	hd.status.hp+=hp;
	hd.status.sp+=sp;

	if(hp || sp)
		clif_send_homstatus(hd.msd,0);

	if( hd.status.hp <= 0)
	{	// killed by negative heal
		this->set_dead();
	}
	return hp + sp;
}

bool homun_data::set_dead()
{
	homun_data &hd = *this;
//	if(config.save_homun_temporal_intimate && hd.msd)
//		pc_setglobalreg(*hd.msd,"HOM_TEMP_INTIMATE",hd.intimate);
	// スキルユニットからの離脱
	hd.status.hp = 1;
	skill_unit_move(hd,gettick(),0);
	hd.status.hp = 0;

	hd.status.incubate = 0;
	hd.save_data();

	hd.delblock();
	hd.deliddb();
	return true;
}

int homun_natural_heal_hp(int tid, unsigned long tick, int id, basics::numptr data)
{
	homun_data *hd = homun_data::from_blid(id);
	uint32 bhp;
	if(hd)
	{
		if(hd->natural_heal_hp_timer != tid)
		{
			if(config.error_log)
				ShowError("natural_heal_hp_timer %d != %d\n",hd->natural_heal_hp_timer, tid);
			return 0;
		}
		hd->natural_heal_hp_timer = -1;
		bhp=hd->status.hp;
		if(hd->walktimer == -1)
		{
			hd->status.hp += hd->nhealhp;
			if(hd->status.hp > hd->max_hp)
				hd->status.hp = hd->max_hp;
			if(bhp != hd->status.hp)
				clif_send_homstatus(hd->msd,0);
		}
		hd->natural_heal_hp_timer = add_timer(tick+NATURAL_HEAL_HP_INTERVAL, homun_natural_heal_hp, hd->block_list::id, 0);
	}
	return 0;
}

int homun_natural_heal_sp(int tid, unsigned long tick, int id, basics::numptr data)
{
	homun_data *hd = homun_data::from_blid(id);
	uint32 bsp;
	if(hd)
	{
		if(hd->natural_heal_sp_timer != tid)
		{
			if(config.error_log)
				ShowError("natural_heal_sp_timer %d != %d\n",hd->natural_heal_sp_timer,tid);
			return 0;
		}
		hd->natural_heal_sp_timer = -1;

		bsp = hd->status.sp;

		if(hd->intimate < hd->status.intimate)
		{
			hd->intimate+=config.homun_temporal_intimate_resilience;
			if(hd->status.intimate<hd->intimate)
				hd->intimate = hd->status.intimate;
			clif_send_homdata(hd->msd,0x100,hd->intimate/100);
		}

		if(hd->walktimer == -1)
		{
			hd->status.sp += hd->nhealsp;
			if(hd->status.sp > hd->max_sp)
				hd->status.sp = hd->max_sp;
			if(bsp != hd->status.sp)
				clif_send_homstatus(hd->msd,0);
		}
		hd->natural_heal_sp_timer = add_timer(tick+NATURAL_HEAL_SP_INTERVAL, homun_natural_heal_sp, hd->block_list::id, 0);
	}
	return 0;
}

void homun_data::delete_natural_heal_timer()
{
	if(this->natural_heal_hp_timer != -1)
	{
		delete_timer(this->natural_heal_hp_timer,homun_natural_heal_hp);
		this->natural_heal_hp_timer = -1;
	}
	if(this->natural_heal_sp_timer != -1)
	{
		delete_timer(this->natural_heal_sp_timer,homun_natural_heal_sp);
		this->natural_heal_sp_timer = -1;
	}
}

void homun_data ::delete_hungry_timer()
{
	if( this->hungry_timer != -1)
	{
		delete_timer(this->hungry_timer, homun_hungry);
		this->hungry_timer = -1;
	}
	if( this->hungry_cry_timer != -1)
	{
		delete_timer(this->hungry_cry_timer, homun_hungry_cry);
		this->hungry_cry_timer = -1;
	}
}

bool homun_data::call_homunculus(map_session_data &sd)
{
	int i;
	if( sd.hd )
	{	// data exists
		if( sd.hd->is_dead() )
		{	// object is dead
			sd.skill_failed(AM_CALLHOMUN);
			return false;
		}
		else if( !sd.hd->is_on_map() )
		{	// not spawned yet, so initialize and create map object
			size_t i;
			unsigned long tick = gettick();

			if(!sd.hd->block_list::id) sd.hd->block_list::id = sd.hd->status.homun_id;

			sd.hd->block_list::m = sd.block_list::m;
			sd.hd->block_list::x = sd.hd->walktarget.x = sd.block_list::x;
			sd.hd->block_list::y = sd.hd->walktarget.y = sd.block_list::y;
			sd.hd->random_walktarget(sd);
			sd.hd->block_list::x = sd.hd->walktarget.x;
			sd.hd->block_list::y = sd.hd->walktarget.y;
			sd.hd->set_dir(sd.get_dir());
			sd.hd->equip = 0;
			sd.hd->speed = sd.get_speed();	//歩行速度は、コール時の主人のspeedになる
			sd.hd->target_id = 0;
			sd.hd->atackable = 1;	// これを0にすると、クライアントから攻撃パケットを出さなくなる
			sd.hd->limits_to_growth = 0;


			//## range check
			sd.hd->view_class = homun_db[sd.hd->status.class_-HOM_ID].view_class;

			for(i=0;i<MAX_HOMSKILL;i++)
				sd.hd->homskillstatictimer[i] = tick;
			////親密度
//			if(config.save_homun_temporal_intimate)
//			{
//				hd->intimate = pc_readglobalreg(sd,"HOM_TEMP_INTIMATE");
//				if(hd->intimate==0)//旧互換
//					hd->intimate = hd->status.intimate;
//			}
//			else
			{
				sd.hd->intimate = sd.hd->status.intimate;
			}

			sd.hd->status.option&=OPTION_MASK;

			sd.hd->calc_status();			// ステータス計算

			sd.hd->delete_hungry_timer();
			sd.hd->delete_natural_heal_timer();

			sd.hd->natural_heal_hp_timer = add_timer(gettick()+NATURAL_HEAL_HP_INTERVAL,homun_natural_heal_hp,sd.hd->block_list::id,0);
			sd.hd->natural_heal_sp_timer = add_timer(gettick()+NATURAL_HEAL_SP_INTERVAL,homun_natural_heal_sp,sd.hd->block_list::id,0);
			sd.hd->hungry_timer = add_timer(gettick()+60*1000,homun_hungry,sd.hd->block_list::id,0);
			if(sd.hd->status.hungry < 10)
				sd.hd->hungry_cry_timer = add_timer(gettick()+20*1000,homun_hungry_cry,sd.hd->block_list::id,0);

			sd.hd->view_size =  0;

			sd.hd->addiddb();
			sd.hd->addblock();

			clif_spawnhom(*sd.hd);
			clif_send_homdata(sd,0,0);
			clif_send_homstatus(sd,1);
			clif_send_homstatus(sd,0);
			clif_homskillinfoblock(sd);
			sd.hd->status.incubate = 1;
			sd.hd->save_data();
			skill_unit_move(*sd.hd,gettick(),1);
		}
		return true;
	}
	else
	{	
		for(i=0;i<MAX_INVENTORY;++i)
		{
			if(sd.status.inventory[i].nameid==7142)
			{	// エンブリオ所持を確認
				pc_delitem(sd,i,1,0);	// エンブリオ消去
				if( config.homun_creation_rate >=100 || rand()%100<(int)config.homun_creation_rate )
					homun_data::create_homunculus(sd, create_homunculus_id());
				return true;
			}
		}
		sd.skill_failed(AM_CALLHOMUN);
		return false;
	}
}









int read_homundb(void)
{
	FILE *fp;
	char line[1024];
	int i;
	int j=0;
	int lines;
	char *filename[]={"db/homun_db.txt","db/homun_db2.txt"};

	// DB情報の初期化
	for(i=0; i<MAX_HOMUN_DB; i++)
	{
		if(homun_db[i].script)
		{
			homun_db[i].script->release();
			homun_db[i].script = NULL;
		}
	}
	memset(homun_db,0,sizeof(homun_db));
	homun_count = 0;

	for(i=0;i<2;i++)
	{
		fp=fopen(filename[i],"r");
		if(fp==NULL)
		{
			if(i>0)
				continue;
			ShowError("can't read %s\n",filename[i]);
			return -1;
		}
		lines=0;
		while( fgets(line,sizeof(line),fp) )
		{
			int nameid,i;
			char *str[50],*p,*np;
			lines++;

			if( !is_valid_line(line) )
				continue;

			for(i=0,p=line;i<32;i++)
			{
				if((np=strchr(p,','))!=NULL)
				{
					str[i]=p;
					*np=0;
					p=np+1;
				}
				else
				{
					str[i]=p;
					p+=strlen(p);
				}
			}

			nameid=atoi(str[0]);
			j = nameid-HOM_ID;

			if(j<0 || j>=MAX_HOMUN_DB)
				continue;

			homun_db[j].class_ = nameid;
			homun_db[j].view_class = atoi(str[1]);
			memcpy(homun_db[j].name,str[2],24);
			memcpy(homun_db[j].jname,str[3],24);
			homun_db[j].base_level=atoi(str[4]);
			homun_db[j].AcceID=atoi(str[5]);
			homun_db[j].FoodID=atoi(str[6]);
			homun_db[j].hp=atoi(str[7]);
			homun_db[j].sp=atoi(str[8]);
			homun_db[j].str=atoi(str[9]);
			homun_db[j].agi=atoi(str[10]);
			homun_db[j].vit=atoi(str[11]);
			homun_db[j].int_=atoi(str[12]);
			homun_db[j].dex=atoi(str[13]);
			homun_db[j].luk=atoi(str[14]);

			homun_db[j].base=atoi(str[15]);
			homun_db[j].hp_kmax=atoi(str[16]);
			homun_db[j].hp_kmin=atoi(str[17]);
			homun_db[j].sp_kmax=atoi(str[18]);
			homun_db[j].sp_kmin=atoi(str[19]);
			homun_db[j].str_k=atoi(str[20]);
			homun_db[j].agi_k=atoi(str[21]);
			homun_db[j].vit_k=atoi(str[22]);
			homun_db[j].int_k=atoi(str[23]);
			homun_db[j].dex_k=atoi(str[24]);
			homun_db[j].luk_k=atoi(str[25]);
			homun_db[j].aspd_k=atoi(str[26]);
			homun_db[j].size=atoi(str[27]);
			homun_db[j].race=atoi(str[28]);
			homun_db[j].element=atoi(str[29]);
			homun_db[j].evo_class=atoi(str[30]);
			homun_db[j].exp_table=atoi(str[31]);
			homun_db[j].skillpoint = homun_db[j].base_level/3; //予約 とりあえずベース/3
			
			if((np=strchr(p,'{'))==NULL)
				continue;

			if(homun_db[j].script)
				homun_db[j].script->release();

			homun_db[j].script = parse_script((uchar*)np,lines);
			homun_count++;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",homun_count,filename[i]);
	}
	return 0;
}
//
// 初期化物
//
int homun_readdb(void)
{
	int i,j,k,class_=0;
	FILE *fp;
	char line[1024],*p;

	// 必要経験値読み込み
	fp=fopen("db/exp_homun.txt","r");
	if(fp==NULL){
		ShowError("can't read db/exp_homun.txt\n");
		return 1;
	}
	i=0;
	while( fgets(line,sizeof(line),fp) )
	{
		int b0,b1,b2,b3,b4,b5;
		if( !is_valid_line(line) )
			continue;
		if(sscanf(line,"%d,%d,%d,%d,%d,%d",&b0,&b1,&b2,&b3,&b4,&b5)!=6)
			continue;
		homun_exp_table[0][i]=b0;
		homun_exp_table[1][i]=b1;
		homun_exp_table[2][i]=b2;
		homun_exp_table[3][i]=b3;
		homun_exp_table[4][i]=b4;
		homun_exp_table[5][i]=b5;
		i++;
		if(i > MAX_LEVEL)
			break;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", "db/exp_homun.txt");

	// スキルツリー
	memset(homun_skill_tree,0,sizeof(homun_skill_tree));
	fp=fopen("db/homun_skill_tree.txt","r");
	if(fp==NULL){
		ShowError("can't read db/homun_skill_tree.txt\n");
		return 1;
	}
	while( fgets(line,sizeof(line),fp) )
	{
		char *split[50];
		if( !is_valid_line(line) )
			continue;
		for(j=0,p=line;j<15 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j<15)
			continue;
		class_=atoi(split[0]);
		i=class_-HOM_ID;
		if(i<0 || i>=MAX_HOMUN_DB)
			continue;
		for(j=0;homun_skill_tree[i][j].id;j++);
		homun_skill_tree[i][j].id=atoi(split[1]);
		homun_skill_tree[i][j].max=atoi(split[2]);
		for(k=0;k<5;k++){
			homun_skill_tree[i][j].need[k].id=atoi(split[k*2+3]);
			homun_skill_tree[i][j].need[k].lv=atoi(split[k*2+4]);
		}
		homun_skill_tree[i][j].base_level=atoi(split[13]);
		homun_skill_tree[i][j].intimate=atoi(split[14]);
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n", "db/homun_skill_tree.txt");

	return 0;
}



int homun_read_embryodb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int homunid,j;
	char *str[10],*p;
	// 読み込む度、初期化
	ln = 0;
	embryo_count=0;
 	embryo_default=6001;

	if( (fp=fopen("db/embryo_db.txt","r"))==NULL )
	{
		ShowError("can't read db/embryo_db.txt");
		return 1;
	}

	while( fgets(line,sizeof(line),fp) )
	{
		if( !is_valid_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<3 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}

		if(str[0]==NULL)
			continue;

		homunid=atoi(str[0]);
		if(homunid == 0) {
			if(str[2])
				embryo_default = atoi(str[2]);
			continue;
		}
		if(homunid<6000)
			continue;
		if(str[2]){
			embryo_data[ln].homunid = homunid;
			embryo_data[ln].per = atoi(str[2]);
		}

		if(ln >= MAX_HOMUN_DB)
			break;
		ln++;
	}
	embryo_count = ln;
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",  embryo_count, "db/embryo_db.txt");

	return 0;
}

void homun_reload(void)
{
	read_homundb();
	homun_readdb();
	homun_read_embryodb();
}

void homun_id_db_final(void* kex, void*data)
{
	if(data)
	{
		homun_data *hd = (homun_data *)data;
		hd->freeblock();
	}
}

int do_init_homun(void)
{
	if(homun_id_db)
	{
		numdb_final(homun_id_db, homun_id_db_final);
		homun_id_db=NULL;
	}
	homun_id_db = numdb_init();

	memset(homun_db,0,sizeof(homun_db));
	read_homundb();
	homun_readdb();
	homun_read_embryodb();
	add_timer_func_list(homun_natural_heal_hp,"homun_natural_heal_hp");
	add_timer_func_list(homun_natural_heal_sp,"homun_natural_heal_sp");
	add_timer_func_list(homun_hungry,"homun_hungry");
	add_timer_func_list(homun_hungry_cry,"homun_hungry_cry");
	return 0;
}

int do_final_homun(void)
{
	int i;
	for(i = 0;i < MAX_HOMUN_DB; i++)
	{
		if(homun_db[i].script)
		{
			homun_db[i].script->release();
			homun_db[i].script=NULL;
		}
	}
	if(homun_id_db)
	{
		numdb_final(homun_id_db, homun_id_db_final);
		homun_id_db=NULL;
	}


	return 0;
}
