// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "db.h"
#include "nullpo.h"
#include "showmsg.h"
#include "utils.h"

#include "party.h"
#include "guild.h" // sharing code for xy sending
#include "pc.h"
#include "map.h"
#include "battle.h"
#include "intif.h"
#include "clif.h"
#include "log.h"

#define PARTY_SEND_XYHP_INVERVAL	1000	// 座標やＨＰ送信の間隔

static struct dbt* party_db;

int party_send_xyhp_timer(int tid, unsigned long tick, int id, basics::numptr data);
/*==========================================
 * 終了
 *------------------------------------------
 */
int party_db_final(void *key,void *data)
{
	delete ((struct party*)data);
	return 0;
}
void do_final_party(void)
{
	if(party_db)
	{
		numdb_final(party_db,party_db_final);
		party_db=NULL;
	}
}
// 初期化
void do_init_party(void)
{
	party_db=numdb_init();
	add_timer_func_list(party_send_xyhp_timer,"party_send_xyhp_timer");
	add_timer_interval(gettick()+PARTY_SEND_XYHP_INVERVAL,PARTY_SEND_XYHP_INVERVAL,party_send_xyhp_timer,0,0);
}

// 検索
struct party *party_search(uint32 party_id)
{
	return (struct party *)numdb_search(party_db,party_id);
}


class CDBPartySearchname : public CDBProcessor
{
	const char *str;
	struct party*& dsp;
public:
	CDBPartySearchname(const char *s, struct party*& p) : str(s),dsp(p)	{}
	virtual ~CDBPartySearchname()	{}
	virtual bool process(void *key, void *data) const
	{
		struct party *p=(struct party *)data;
		if(strcasecmp(p->name,str)==0)
		{
			dsp=p;
			// found -> stop db traverse
			return false;
		}
		return true;
	}

};
// パーティ名検索
struct party* party_searchname(const char *str)
{
	struct party *p=NULL;
	numdb_foreach(party_db, CDBPartySearchname(str,p) );
	//numdb_foreach(party_db,party_searchname_sub,str,&p);
	return p;
}
// 作成要求
int party_create(struct map_session_data &sd,const char *name,int item,int item2)
{
	if(sd.status.party_id==0)
		intif_create_party(sd,name,item,item2);
	else
		clif_party_created(sd,2);
	return 0;
}

// 作成可否
int party_created(uint32 account_id,int fail,uint32 party_id,const char *name)
{
	struct map_session_data *sd = map_session_data::from_blid(account_id);

	nullpo_retr(0, sd);
	if(fail==0)
	{
		if( NULL != numdb_search(party_db,party_id) )
		{
			ShowMessage("party: id already exists!\n");
		}
		else
		{
			struct party *p = new struct party(party_id, name);
			sd->status.party_id=party_id;
		
			numdb_insert(party_db,party_id,p);
			clif_party_created(*sd,0);
			clif_charnameack(-1, *sd); //Update other people's display. [Skotlex]
			return 0;
		}
	}
	// failed
	clif_party_created(*sd,1);
	return 0;
}

// 情報要求
int party_request_info(uint32 party_id)
{
	return intif_request_partyinfo(party_id);
}

// 所属キャラの確認
int party_check_member(struct party &p)
{
	size_t i;
	struct map_session_data *sd;

	for(i=0;i<fd_max; ++i)
	{
		if(session[i] && (sd=(struct map_session_data *)session[i]->user_session) && sd->state.auth)
		{
			if(sd->status.party_id==p.party_id)
			{
				size_t j, f=1;
				for(j=0;j<MAX_PARTY; ++j)
				{	// パーティにデータがあるか確認
					if(	p.member[j].account_id==sd->status.account_id)
					{
						if(	strcmp(p.member[j].name,sd->status.name)==0 )
							f=0;	// データがある
						else
							p.member[j].sd=NULL;	// 同垢別キャラだった
					}
				}
				if(f)
				{
					sd->status.party_id=0;
					if(config.error_log)
						ShowMessage("party: check_member %d[%s] is not member\n",sd->status.account_id,sd->status.name);
				}
			}
		}
	}
	return 0;
}

// 情報所得失敗（そのIDのキャラを全部未所属にする）
int party_recv_noinfo(uint32 party_id)
{
	size_t i;
	struct map_session_data *sd;
	for(i=0;i<fd_max; ++i){
		if(session[i] && (sd=(struct map_session_data *) session[i]->user_session) && sd->state.auth){
			if(sd->status.party_id==party_id)
				sd->status.party_id=0;
		}
	}
	return 0;
}
// 情報所得
int party_recv_info(struct party &sp)
{
	int i;
	struct party *p=(struct party *)numdb_search(party_db, sp.party_id);
	if(p==NULL)
	{	//does not exist, so create a new one as direct copy
		p= new struct party(sp);
		numdb_insert(party_db, sp.party_id, p);

		party_check_member(sp);
	}
	else
	{	// just copy
		*p = sp;
	}
	
	for(i=0;i<MAX_PARTY; ++i)
	{	// sdの設定
		if( p->member[i].account_id )
		{
			struct map_session_data *sd = map_session_data::from_blid(p->member[i].account_id);
			p->member[i].sd=(sd!=NULL && sd->status.party_id==p->party_id && !sd->state.waitingdisconnect)?sd:NULL;
		}
	}

	clif_party_info(*p,-1);

	for(i=0;i<MAX_PARTY; ++i)
	{	// 設定情報の送信
		struct map_session_data *sd = p->member[i].sd;
		if(sd!=NULL && sd->party_sended==0){
			clif_party_option(*p,sd,0x100);
			sd->party_sended=1;
		}
	}
	return 0;
}

// パーティへの勧誘
int party_invite(struct map_session_data &sd,uint32 account_id)
{
	struct map_session_data *tsd= map_session_data::from_blid(account_id);
	struct party *p=party_search(sd.status.party_id);
	int i;
	
	if(tsd==NULL || p==NULL)
		return 0;
	if(!config.invite_request_check) {
		if (tsd->guild_invite>0 || tsd->trade_partner) {	// 相手が取引中かどうか
			clif_party_inviteack(sd,tsd->status.name,0);
			return 0;
		}
	}
	if( tsd->status.party_id>0 || tsd->party_invite>0 ){	// 相手の所属確認
		clif_party_inviteack(sd,tsd->status.name,0);
		return 0;
	}
	for(i=0;i<MAX_PARTY; ++i){	// 同アカウント確認
		if(p->member[i].account_id==account_id){
			clif_party_inviteack(sd,tsd->status.name,0);
			return 0;
		}
	}
	
	tsd->party_invite=sd.status.party_id;
	tsd->party_invite_account=sd.status.account_id;

	clif_party_invite(sd,*tsd);
	return 0;
}
// パーティ勧誘への返答
int party_reply_invite(struct map_session_data &sd,uint32 account_id,int flag)
{
	struct map_session_data *tsd= map_session_data::from_blid(account_id);

	if(flag==1){	// 承諾
		//inter鯖へ追加要求
		intif_party_addmember( sd.party_invite, sd.status.account_id );
		return 0;
	}
	else {		// 拒否
		sd.party_invite=0;
		sd.party_invite_account=0;
		if(tsd==NULL)
			return 0;
		clif_party_inviteack(*tsd,sd.status.name,1);
	}
	return 0;
}
// パーティが追加された
int party_member_added(uint32 party_id,uint32 account_id,int flag)
{
	struct map_session_data *sd = map_session_data::from_blid(account_id),*sd2;
	if(sd == NULL){
		if(flag==0)
		{
			if(config.error_log)
				ShowMessage("party: member added error %d is not online\n",account_id);
			intif_party_leave(party_id,account_id); // キャラ側に登録できなかったため脱退要求を出す
		}
		return 0;
	}
	sd2=map_session_data::from_blid(sd->party_invite_account);
	sd->party_invite=0;
	sd->party_invite_account=0;
	
	if(flag==1){	// 失敗
		if( sd2!=NULL )
			clif_party_inviteack(*sd2,sd->status.name,0);
		return 0;
	}
	
		// 成功
	sd->party_sended=0;
	sd->status.party_id=party_id;
	
	if( sd2!=NULL)
		clif_party_inviteack(*sd2,sd->status.name,2);

	// いちおう競合確認
	party_check_conflict(*sd);
	clif_charnameack(-1, *sd); //Update char name's display [Skotlex]
	return 0;
}
// パーティ除名要求
int party_removemember(struct map_session_data &sd,uint32 account_id,const char *name)
{
	struct party *p;
	int i;

	if( (p = party_search(sd.status.party_id)) == NULL )
		return 0;

	for(i=0;i<MAX_PARTY; ++i){	// リーダーかどうかチェック
		if(p->member[i].account_id==sd.status.account_id)
			if(p->member[i].leader==0)
				return 0;
	}

	for(i=0;i<MAX_PARTY; ++i){	// 所属しているか調べる
		if(p->member[i].account_id==account_id){
			intif_party_leave(p->party_id,account_id);
			return 0;
		}
	}
	return 0;
}

// パーティ脱退要求
int party_leave(struct map_session_data &sd)
{
	struct party *p;
	int i;

	if( (p = party_search(sd.status.party_id)) == NULL )
		return 0;
	
	for(i=0;i<MAX_PARTY; ++i){	// 所属しているか
		if(p->member[i].account_id==sd.status.account_id){
			intif_party_leave(p->party_id,sd.status.account_id);
			return 0;
		}
	}
	
	return 0;
}
// パーティメンバが脱退した
int party_member_leaved(uint32 party_id,uint32 account_id,const char *name)
{
	struct map_session_data *sd=map_session_data::from_blid(account_id);
	struct party *p=party_search(party_id);
	if(p!=NULL){
		int i;
		for(i=0;i<MAX_PARTY; ++i)
			if(p->member[i].account_id==account_id){
				clif_party_leaved(*p,sd,account_id,name,0x00);
				p->member[i].account_id=0;
				p->member[i].sd=NULL;
			}
	}
	if(sd!=NULL && sd->status.party_id==party_id){
		sd->status.party_id=0;
		sd->party_sended=0;
		clif_charnameack(-1, *sd, true); //Update name display [Skotlex]
	}
	return 0;
}
// パーティ解散通知
int party_broken(uint32 party_id)
{
	struct party *p;
	int i;
	if( (p=party_search(party_id))==NULL )
		return 0;
	
	for(i=0;i<MAX_PARTY; ++i){
		if(p->member[i].sd!=NULL){
			clif_party_leaved(*p,p->member[i].sd,
				p->member[i].account_id,p->member[i].name,0x10);
			p->member[i].sd->status.party_id=0;
			p->member[i].sd->party_sended=0;
		}
	}
	numdb_erase(party_db,party_id);
	return 0;
}
// パーティの設定変更要求
int party_changeoption(struct map_session_data &sd,unsigned short expshare,unsigned short itemshare)
{
	size_t i;
	struct party *p;

	if( sd.status.party_id==0 || (p=party_search(sd.status.party_id))==NULL )
		return 0;

	for(i=0; i<MAX_PARTY; ++i)
	{	// only leader can change party options
		if(p->member[i].account_id == sd.status.account_id)
		{
			if( p->member[i].leader )
				intif_party_changeoption(sd.status.party_id, sd.status.account_id, expshare, itemshare);
			break;
		}
	}
	return 0;
}
// パーティの設定変更通知
int party_optionchanged(uint32 party_id,uint32 account_id,unsigned short expshare,unsigned short itemshare,unsigned char flag)
{
	struct party *p;
	struct map_session_data *sd=map_session_data::from_blid(account_id);
	if( (p=party_search(party_id))==NULL)
		return 0;

	p->expshare = expshare;
	p->itemshare = itemshare;
	clif_party_option(*p,sd,flag);
	return 0;
}

// パーティメンバの移動通知
int party_recv_movemap(uint32 party_id,uint32 account_id,const char *mapname,int online,unsigned short lv)
{
	struct party *p;
	int i;
	if( (p=party_search(party_id))==NULL)
		return 0;
	for(i=0;i<MAX_PARTY; ++i)
	{
		struct party_member *m=&p->member[i];

		if(m->account_id==account_id){
			memcpy(m->mapname,mapname,24);
			m->mapname[23]=0;
			m->online=online;
			m->lv=lv;
			break;
		}

	}
	if(i==MAX_PARTY){
		if(config.error_log)
			ShowError("party: not found member %d on %d[%s]",account_id,party_id,p->name);
		return 0;
	}
	
	for(i=0;i<MAX_PARTY; ++i){	// sd再設定
		struct map_session_data *sd= map_session_data::from_blid(p->member[i].account_id);
		p->member[i].sd=(sd!=NULL && sd->status.party_id==p->party_id && !sd->state.waitingdisconnect)?sd:NULL;
	}

	party_send_xy_clear(*p);	// 座標再通知要請
	
	clif_party_info(*p,-1);
	return 0;
}

// パーティメンバの移動
int party_send_movemap(struct map_session_data &sd)
{
	struct party *p;

	if( sd.status.party_id==0 )
		return 0;
	intif_party_changemap(&sd,1);

	if( sd.party_sended!=0 )	// もうパーティデータは送信済み
		return 0;

	// 競合確認	
	party_check_conflict(sd);
	
	// あるならパーティ情報送信
	if( (p=party_search(sd.status.party_id))!=NULL )
	{
		party_check_member(*p);	// 所属を確認する
		if(sd.status.party_id==p->party_id){
			clif_party_info(*p,sd.fd);
			clif_party_option(*p,&sd,0x100);
			sd.party_sended=1;
		}
	}
	return 0;
}
// パーティメンバのログアウト
int party_send_logout(struct map_session_data &sd)
{
	struct party *p;

	if( sd.status.party_id>0 )
		intif_party_changemap(&sd,0);

	// sdが無効になるのでパーティ情報から削除
	if( (p=party_search(sd.status.party_id))!=NULL ){
		int i;
		for(i=0;i<MAX_PARTY; ++i)
			if(p->member[i].sd==&sd)
				p->member[i].sd=NULL;
	}
	
	return 0;
}
// パーティメッセージ送信
int party_send_message(struct map_session_data &sd,const char *mes,size_t len)
{
	if(sd.status.party_id==0)
		return 0;
	intif_party_message(sd.status.party_id,sd.status.account_id,mes,len);
	party_recv_message (sd.status.party_id,sd.status.account_id,mes,len);
	//Chat Logging support Type 'P'
	if(log_config.chat&1 //we log everything then
		|| ( log_config.chat&4 //if Party bit is on
		&& ( !agit_flag || !(log_config.chat&16) ))) //if WOE ONLY flag is off or AGIT is OFF
		log_chat("P", sd.status.party_id, sd.status.char_id, sd.status.account_id, sd.mapname, sd.block_list::x, sd.block_list::y, "", mes);
	
	return 0;
}

// パーティメッセージ受信
int party_recv_message(uint32 party_id,uint32 account_id,const char *mes,size_t len)
{
	struct party *p;
	if( (p=party_search(party_id))==NULL)
		return 0;
	clif_party_message(*p,account_id,mes,len);
	return 0;
}
// パーティ競合確認
int party_check_conflict(struct map_session_data &sd)
{
	intif_party_checkconflict(sd.status.party_id,sd.status.account_id,sd.status.name);
	return 0;
}


// 位置やＨＰ通知用
/*
int party_send_xyhp_timer_sub(void *key,void *data,va_list &ap)
{
	struct party *p=(struct party *)data;
	int i;

	nullpo_retr(0, p);

	for(i=0;i<MAX_PARTY; ++i)
	{
		struct map_session_data *sd=p->member[i].sd;
		if(sd!=NULL)
		{
			// 座標通知
			if(sd->party_x!=sd->block_list::x || sd->party_y!=sd->block_list::y)
			{
				clif_party_xy(*p,*sd);
				if(sd->status.guild_id)
					clif_guild_xy(*sd);
				sd->party_x=sd->block_list::x;
				sd->party_y=sd->block_list::y;
			}
			// ＨＰ通知
			if(sd->party_hp!=sd->status.hp)
			{
				clif_party_hp(*p,*sd);
				sd->party_hp=sd->status.hp;
			}
		}
	}
	return 0;
}
*/
class CDBPartySendXYHP : public CDBProcessor
{
	unsigned long tick;
public:
	CDBPartySendXYHP(unsigned long t) : tick(t)	{}
	virtual ~CDBPartySendXYHP()	{}
	virtual bool process(void *key, void *data) const
	{
		struct party *p=(struct party *)data;
		int i;

		nullpo_retr(true, p);

		for(i=0;i<MAX_PARTY; ++i)
		{
			struct map_session_data *sd=p->member[i].sd;
			if(sd!=NULL)
			{
				// 座標通知
				if(sd->party_x!=sd->block_list::x || sd->party_y!=sd->block_list::y)
				{
					clif_party_xy(*p,*sd);
					if(sd->status.guild_id)
						clif_guild_xy(*sd);
					sd->party_x=sd->block_list::x;
					sd->party_y=sd->block_list::y;
				}
				// ＨＰ通知
				if(sd->party_hp!=sd->status.hp)
				{
					clif_party_hp(*p,*sd);
					sd->party_hp=sd->status.hp;
				}
			}
		}
		return true;
	}
};
// 位置やＨＰ通知
int party_send_xyhp_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	numdb_foreach(party_db, CDBPartySendXYHP(tick) );
//	numdb_foreach(party_db,party_send_xyhp_timer_sub,tick);
	guild_send_xy(tick);
	return 0;
}

// 位置通知クリア
int party_send_xy_clear(struct party &p)
{
	int i;
	for(i=0;i<MAX_PARTY; ++i){
		struct map_session_data *sd=p.member[i].sd;
		if(sd!=NULL){
			sd->party_x=0xFFFF;
			sd->party_y=0xFFFF;
			sd->party_hp=-1;
		}
	}
	return 0;
}
// HP通知の必要性検査用（map_foreachinmoveareaから呼ばれる）
int CPartySendHP::process(block_list& bl) const
{
	struct map_session_data *sd = bl.get_sd();
	if(sd && sd->status.party_id==party_id)
	{
		flag=1;
		sd->party_hp=-1;
	}
	return 0;
}

// exp share and added zeny share [Valaris]
int party_exp_share(party &p,unsigned short map, uint32 base_exp,uint32 job_exp,uint32 zeny)
{
	struct map_session_data *sd;
	size_t i;
	unsigned short c=0;
	unsigned short memberpos[MAX_PARTY];

	for (i=c=0; i < MAX_PARTY; ++i)
	{	
		if((sd=p.member[i].sd)!=NULL && p.member[i].online && sd->block_list::m==map && session[sd->fd] != NULL )
		{
			if( !( sd->chat                              && config.party_share_mode>=2 ) &&	// don't count chatting
				!( difftime(last_tick, sd->idletime)>120 && config.party_share_mode>=1) &&	// don't count idle
				!sd->is_dead() )
				memberpos[c++] = i;
		}
	}

	if(c==0)
		return 0;
	
	if( config.party_bonus )
	{	// bonus formula, originally [Valaris]
		// 1    2    3    4    5    6    7    8    9    10   11   12
		// 1.00 1.05 1.15 1.30 1.50 1.75 2.05 2.40 2.80 3.25 3.75 4.30
		//
		// will be calculated as E' = E*( 1+0.05*(c-1)*c/2 )
		// E' will be then distributed equally among the members
		// so E'' = E'/c 
		//        = E*( 1+0.05*(c-1)*c/2 )/c 
		//        = E*( 1+0.05*(c-1)  /2 )
		//        = E*( 1+1/20*(c-1)  /2 )
		//        = E*( 1+     (c-1)  /40)
		//        =     E+   E*(c-1)  /40  (+1 to prevent 0 exp)

		base_exp += base_exp * (c-1) /40+1;	//base_exp * (c-1)*c /40;
		job_exp  += job_exp  * (c-1) /40+1;	//job_exp  * (c-1)*c /40;
		zeny     += zeny     * (c-1) /40;	//zeny     * (c-1)*c /40;
	}
	else
	{
		base_exp = base_exp/c+1;
		job_exp  = job_exp /c+1;
		zeny     = zeny    /c;
	}

	for(i = 0; i < c; ++i)
	{
		if( (sd=p.member[memberpos[i]].sd)!=NULL )
		{
			pc_gainexp(*sd,base_exp,job_exp);
			if (config.zeny_from_mobs) // zeny from mobs [Valaris]
				pc_getzeny(*sd,zeny);
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// other calculation method
// pc's get experience according to their lvl
// so if a lvl 99 and a lvl 1 pc share exp, 
// the lvl 99 gets 99 and the lvl 1 get 1 point
// this way it won't be necessary to block exp sharing of lvl differences
///////////////////////////////////////////////////////////////////////////////
int party_exp_share2(struct party &p, unsigned short map, uint32 base_exp, uint32 job_exp, uint32 zeny)
{
	struct map_session_data *sd;
	size_t i;
	size_t lvlsum = 0, c=0;
	unsigned short memberpos[MAX_PARTY];
	double base_exp_div,job_exp_div,zeny_div;


	for (i=c=0; i < MAX_PARTY; ++i)
	{	
		if((sd=p.member[i].sd)!=NULL && p.member[i].online && sd->block_list::m==map && session[sd->fd] != NULL)
		{
			if( !( sd->chat                               && config.party_share_mode>=2 ) &&	// don't count chatting
				!( difftime(last_tick, sd->idletime)>120  && config.party_share_mode>=1) &&	// don't count idle
				!sd->is_dead() )
				memberpos[c++] = i;
				lvlsum += p.member[i].lv;
		}
	}

	if(lvlsum==0)
		return 0;

	base_exp_div = (double)base_exp /(double)lvlsum;
	job_exp_div  = (double)job_exp  /(double)lvlsum;
	zeny_div     = (double)zeny     /(double)lvlsum;

	if( config.party_bonus )
	{	// bonus formula
		// 1    2    3    4    5    6    7    8    9    10   11   12
		// 1.00 1.05 1.15 1.30 1.50 1.75 2.05 2.40 2.80 3.25 3.75 4.30
		//
		// will be calculated as E*( 1+0.05*(c-1)*c/2 )
		base_exp_div += base_exp_div * (c-1)*c /40.;
		job_exp_div  += job_exp_div  * (c-1)*c /40.;
		zeny_div     += zeny_div     * (c-1)*c /40.;
	}


	for(i = 0; i < c; ++i)
	{
		if( (sd=p.member[memberpos[i]].sd)!=NULL )
		{
			pc_gainexp(*sd,(uint32)(base_exp_div * p.member[i].lv),(uint32)(job_exp_div * p.member[i].lv));
			if(config.zeny_from_mobs) // zeny from mobs [Valaris]
				pc_getzeny(*sd,(uint32)(zeny_div*p.member[i].lv));
		}
	}
	return 0;
}

int party_send_dot_remove(struct map_session_data &sd)
{
	return clif_party_xy_remove(sd) && clif_guild_xy_remove(sd);
}
