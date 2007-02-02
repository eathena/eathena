// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "db.h"
#include "timer.h"
#include "socket.h"
#include "nullpo.h"
#include "malloc.h"
#include "showmsg.h"
#include "utils.h"


#include "guild.h"
#include "storage.h"
#include "battle.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "intif.h"
#include "clif.h"
#include "skill.h"
#include "log.h"

static struct dbt *guild_db;
static struct dbt *castle_db;
static struct dbt *guild_expcache_db;
static struct dbt *guild_infoevent_db;
static struct dbt *guild_castleinfoevent_db;

struct eventlist
{
	char name[50];
	struct eventlist *next;

	eventlist(const char *n, struct eventlist *nx) :
		next(nx)
	{
		safestrcpy(name,sizeof(name),n);
	}
};

// ギルドのEXPキャッシュのフラッシュに関連する定数
#define GUILD_PAYEXP_INVERVAL 10000	// 間隔(キャッシュの最大生存時間、ミリ秒)
#define GUILD_PAYEXP_LIST 8192	// キャッシュの最大数
#define GUILD_SEND_XYHP_INVERVAL	1000	// 座標やＨＰ送信の間隔

// ギルドのEXPキャッシュ
struct guild_expcache {
	uint32 guild_id;
	uint32 account_id;
	uint32 char_id;
	uint32 exp;
};

// timer for auto saving guild data during WoE
#define GUILD_SAVE_INTERVAL 300000
int guild_save_timer = -1;

int guild_payexp_timer(int tid, unsigned long tick, int id, basics::numptr data);
int guild_gvg_eliminate_timer(int tid, unsigned long tick, int id, basics::numptr data);
int guild_save_sub(int tid, unsigned long tick, int id, basics::numptr data);

// ギルドスキルdbのアクセサ（今は直打ちで代用）

// Modified for new skills [Sara]
int guild_skill_get_inf(unsigned short id)
{
	switch(id) {
		case GD_BATTLEORDER:
		case GD_REGENERATION:
		case GD_RESTORE:
		case GD_EMERGENCYCALL:
			return 4;
	}
	return 0;
}

int guild_skill_get_max(unsigned short id)
{	// Modified for new skills [Sara]
	switch (id) {
	case GD_GUARDUP:
		return 3;
	case GD_EXTENSION:
		return 10;
	case GD_LEADERSHIP:
	case GD_GLORYWOUNDS:
	case GD_SOULCOLD:
	case GD_HAWKEYES:
		return 5;
	case GD_REGENERATION:
		return 3;
	default:
		return 1;
	}
}

// ギルドスキルがあるか確認
int guild_checkskill(const guild &g,unsigned short skillid)
{
	unsigned short idx = skillid-GD_SKILLBASE;
	if(idx<MAX_GUILDSKILL)
		return g.skill[idx].lv;
	return 0;
}

int guild_read_castledb(void)
{
	FILE *fp;
	char line[1024];
	int j,ln=0;
	char *str[32],*p;
	struct guild_castle *gc;

	if( (fp=basics::safefopen("db/castle_db.txt","r"))==NULL ){
		ShowError("can't read %s\n", "db/castle_db.txt");
		return -1;
	}

	while(fgets(line,sizeof(line),fp))
	{
		if( !is_valid_line(line) )
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<6 && p; ++j)
		{
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}

		if( str[0] )	// we have at least something for an ID string
		{
			uint32 id = atoi(str[0]);
			gc = (struct guild_castle *)numdb_search(castle_db, id);
			if(gc==NULL)
			{
				gc= new struct guild_castle(id);
				numdb_insert(castle_db, id, gc);
			}
			if(str[0]) gc->castle_id=id;
			if(str[1]) memcpy(gc->mapname,str[1],24); 
			char*ip=strchr(gc->mapname,'.');
			if(ip) *ip=0;
			if(str[2]) memcpy(gc->castle_name,str[2],24);
			if(str[3]) memcpy(gc->castle_event,str[3],24);
		}
		ln++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n",ln,"db/castle_db.txt");
	return 0;
}

// 検索
struct guild *guild_search(uint32 guild_id)
{
	return (struct guild *)numdb_search(guild_db,guild_id);
}

// ギルド名検索
struct guild* guild_searchname(const char *str)
{
	db_iterator<size_t, guild *> iter(guild_db);
	for(; iter; ++iter)
	{
		guild *g=iter.data();
		if( g && 0==strcasecmp(g->name,str) )
		{
			return g;
		}
	}
	return NULL;
}
struct guild_castle *guild_castle_search(uint32 gcid)
{
	return (struct guild_castle *) numdb_search(castle_db,gcid);
}

// mapnameに対応したアジトのgcを返す
struct guild_castle *guild_mapname2gc(const char *mapname)
{
	int i;
	struct guild_castle *gc=NULL;
	for(i=0;i<MAX_GUILDCASTLE;++i){
		gc=guild_castle_search(i);
		if(!gc) continue;
		if(strcmp(gc->mapname,mapname)==0) return gc;
	}
	return NULL;
}

// ログイン中のギルドメンバーの１人のsdを返す
map_session_data *guild_getavailablesd(struct guild &g)
{
	int i;
	for(i=0;i<g.max_member && i<MAX_GUILD;++i)
		if(g.member[i].sd!=NULL)
			return g.member[i].sd;
	return NULL;
}

// ギルドメンバーのインデックスを返す
int guild_getindex(struct guild &g,uint32 account_id,uint32 char_id)
{
	size_t i;
	for(i=0;i<g.max_member && i<MAX_GUILD;++i)
		if( g.member[i].account_id==account_id &&
			g.member[i].char_id==char_id )
			return i;
	return -1;
}
// ギルドメンバーの役職を返す
int guild_getposition(map_session_data &sd,struct guild &g)
{
	size_t i;
	for(i=0; i<g.max_member && i<MAX_GUILD;++i)
		if( g.member[i].account_id==sd.status.account_id &&
			g.member[i].char_id==sd.status.char_id )
			return g.member[i].position;
		return -1;
}

// メンバー情報の作成
void guild_makemember(struct guild_member &m,map_session_data &sd)
{
	memset(&m,0,sizeof(struct guild_member));
	m.account_id	=sd.status.account_id;
	m.char_id		=sd.status.char_id;
	m.hair			=sd.status.hair;
	m.hair_color	=sd.status.hair_color;
	m.gender		=sd.status.sex;
	m.class_		=sd.status.class_;
	m.lv			=sd.status.base_level;
	m.exp			=0;
	m.exp_payper	=0;
	m.online		=1;
	m.position		=MAX_GUILDPOSITION-1;
	memcpy(m.name,sd.status.name,24);
	return;
}
// ギルド競合確認
int guild_check_conflict(map_session_data &sd)
{
	return intif_guild_checkconflict(sd.status.guild_id,sd.status.account_id,sd.status.char_id);
}

// ギルドのEXPキャッシュをinter鯖にフラッシュする
int guild_payexp_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	size_t delp=0;

	db_iterator<ssize_t, guild_expcache*> iter(guild_expcache_db);
	for(; iter; ++iter)
	{
		ssize_t dataid = iter.key();
		struct guild_expcache *c=iter.data();
		int i;
		struct guild *g;

		numdb_erase(guild_expcache_db, dataid);

		if( c &&
			(g = guild_search(c->guild_id)) &&
			(i = guild_getindex(*g, c->account_id, c->char_id)) >= 0)
		{
			uint64 exp = (uint64)g->member[i].exp + (uint64)c->exp;
			g->member[i].exp = (exp > INT_MAX) ? INT_MAX : exp;

			intif_guild_change_memberinfo(g->guild_id,c->account_id,c->char_id,GMI_EXP,g->member[i].exp);

			delete c;

			++delp;
		}
	}


	if(config.etc_log && delp)
		ShowMessage("guild exp %d charactor's exp flushed !\n",delp);
	return 0;
}

//------------------------------------------------------------------------

// 作成要求
int guild_create(map_session_data &sd,const char *name)
{
	if(sd.status.guild_id==0)
	{
		if(!config.guild_emperium_check || pc_search_inventory(sd,714) >= 0)
		{
			struct guild_member m;
			guild_makemember(m,sd);
			m.position=0;
			intif_guild_create(name,m);
		}
		else
			clif_guild_created(sd,3);	// エンペリウムがいない
	}
	else
		clif_guild_created(sd,1);	// すでに所属している

	return 0;
}

// 作成可否
int guild_created(uint32 account_id,uint32 guild_id)
{
	map_session_data *sd=map_session_data::from_blid(account_id);

	if(sd==NULL)
		return 0;

	if(guild_id>0)
	{
		const struct guild *g=(struct guild *) numdb_search(guild_db,guild_id);
		if(g!=NULL)
		{
			ShowMessage("guild: id already exists!\n");
			return 0;
		}
		sd->status.guild_id=guild_id;
		sd->guild_sended=0;

		clif_guild_created(*sd,0);
		if(config.guild_emperium_check)
			pc_delitem(*sd,pc_search_inventory(*sd,714),1,0);	// エンペリウム消耗

		guild_request_info(guild_id);
	}
	else
	{
		clif_guild_created(*sd,2);	// 作成失敗（同名ギルド存在）
	}
	return 0;
}

// 情報要求
int guild_request_info(uint32 guild_id)
{
//	if(config.etc_log)
//		ShowMessage("guild_request_info\n");
	return intif_guild_request_info(guild_id);
}
// イベント付き情報要求
int guild_npc_request_info(uint32 guild_id,const char *event)
{
	struct eventlist *ev;

	if( guild_search(guild_id) ){
		if(event && *event)
			npc_data::event(event);
		return 0;
	}

	if(event==NULL || *event==0)
		return guild_request_info(guild_id);

	// get the previously inserted element from the database (is NULL when empty),
	// to put it as next element into this one
	ev = new struct eventlist(event, (struct eventlist *)numdb_search(guild_infoevent_db,guild_id));
	// and overwrite the old database entry with the new one
	numdb_insert(guild_infoevent_db,guild_id,ev);
	return guild_request_info(guild_id);
}

// 所属キャラの確認
bool guild_check_member(const struct guild &g)
{
	size_t i;
	map_session_data *sd;
	
	for(i=0;i<fd_max;++i)
	{
		if( session[i] && 
			(sd=(map_session_data *)session[i]->user_session) && 
			sd->state.auth &&
			sd->status.guild_id==g.guild_id )
		{
			size_t j;
			for(j=0;j<g.max_member && j<MAX_GUILD; ++j)
			{	// データがあるか
				if(	g.member[j].account_id==sd->status.account_id &&
					g.member[j].char_id==sd->status.char_id)
				{	// found it
					return true;
				}
			}
			if(j>=MAX_GUILD || j>=g.max_member)// always valid
			{
				sd->status.guild_id=0;
				sd->guild_sended=0;
				sd->guild_emblem_id=0;
				if(config.error_log)
					ShowMessage("guild: check_member %d[%s] is not member\n",sd->status.account_id,sd->status.name);
			}
		}
	}
	return false;
}
// 情報所得失敗（そのIDのキャラを全部未所属にする）
int guild_recv_noinfo(uint32 guild_id)
{
	size_t i;
	map_session_data *sd;
	for(i=0;i<fd_max;++i){
		if(session[i] && (sd=(map_session_data *)session[i]->user_session) && sd->state.auth){
			if(sd->status.guild_id==guild_id)
				sd->status.guild_id=0;
		}
	}
	return 0;
}
// 情報所得
int guild_recv_info(struct guild &sg)
{
	struct guild *g, before;
	int i,bm,m;
	struct eventlist *ev,*ev2;

	if((g=(struct guild *) numdb_search(guild_db,sg.guild_id))==NULL )
	{	// 最初のロードなのでユーザーのチェックを行う
		map_session_data* sd;
		guild_check_member(sg);

		g = new struct guild;
		memset(g,0,sizeof(struct guild));
		numdb_insert(guild_db,sg.guild_id,g);
		before = *g;
		//If the guild master is online the first time the guild_info is received, that means he was the first to join,
		//and as such, his guild skills should be blocked to avoid login/logout abuse [Skotlex]
		if( (sd = map_session_data::nick2sd(sg.master)) != NULL )
		{
			int skill_num[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
			for (i = 0; i < 4; ++i)
				if (guild_checkskill(sg, skill_num[i]))
					pc_blockskill_start(*sd, skill_num[i], 300000);
		}
	}
	else
	{
		before = *g;
	}
	*g = sg;


	for(i=bm=m=0;i<g->max_member;++i)
	{	// sdの設定と人数の確認
		if(g->member[i].account_id>0)
		{
			map_session_data *sd = map_session_data::from_blid(g->member[i].account_id);
			if (sd && sd->status.char_id == g->member[i].char_id &&
				sd->status.guild_id == g->guild_id &&
				!sd->state.waitingdisconnect)
			{
				g->member[i].sd = sd;
			}
			else
				g->member[i].sd = NULL;
			m++;
		}
		else
			g->member[i].sd=NULL;
		if(before.member[i].account_id>0)
			bm++;
	}

	for(i=0;i<g->max_member;++i)
	{	// 情報の送信
		map_session_data *sd = g->member[i].sd;
		if( sd==NULL )
			continue;

		if(	before.guild_lv!=g->guild_lv || bm!=m ||
			before.max_member!=g->max_member )
		{
			clif_charnameack(-1, *sd);
			clif_guild_basicinfo(*sd);	// 基本情報送信
			clif_guild_emblem(*sd,*g);	// エンブレム送信
		}

		if(bm!=m)
		{	// メンバー情報送信
			clif_guild_memberlist(*g->member[i].sd);
		}

		if( before.skill_point!=g->skill_point)
			clif_guild_skillinfo(*sd);	// スキル情報送信

		if( sd->guild_sended==0)
		{	// 未送信なら所属情報も送る
			clif_guild_belonginfo(*sd,*g);
			clif_guild_notice(*sd,*g);
			sd->guild_emblem_id=g->emblem_id;
			sd->guild_sended=1;
		}
	}

	// イベントの発生
	if( (ev=(struct eventlist *)numdb_search(guild_infoevent_db,sg.guild_id))!=NULL )
	{
		numdb_erase(guild_infoevent_db,sg.guild_id);
		while(ev)
		{
			npc_data::event(ev->name);
			ev2=ev->next;
			delete ev;
			ev=ev2;
		}
	}

	return 0;
}


// ギルドへの勧誘
int guild_invite(map_session_data &sd,uint32 account_id)
{
	map_session_data *tsd;
	struct guild *g;
	size_t i;

	tsd= map_session_data::from_blid(account_id);
	g=guild_search(sd.status.guild_id);
	if(tsd==NULL || g==NULL)
		return 0;

	if(!config.invite_request_check)
	{
		if (tsd->party_invite>0 || tsd->trade_partner)
		{	// 相手が取引中かどうか
			clif_guild_inviteack(sd,0);
			return 0;
		}
	}
	if( tsd->status.guild_id>0 || tsd->guild_invite>0 ){	// 相手の所属確認
		clif_guild_inviteack(sd,0);
		return 0;
	}

	// 定員確認
	for(i=0;i<g->max_member;++i)
		if(g->member[i].account_id==0)
			break;
	if(i==g->max_member){
		clif_guild_inviteack(sd,3);
		return 0;
	}

	tsd->guild_invite=sd.status.guild_id;
	tsd->guild_invite_account=sd.status.account_id;

	clif_guild_invite(*tsd,*g);
	return 0;
}
// ギルド勧誘への返答
int guild_reply_invite(map_session_data &sd,uint32 guild_id,int flag)
{
	map_session_data *tsd = map_session_data::from_blid( sd.guild_invite_account );
	nullpo_retr(0, tsd);

	if(sd.guild_invite!=guild_id)	// 勧誘とギルドIDが違う
		return 0;

	if(flag==1)
	{	// 承諾
		struct guild_member m;
		struct guild *g;
		size_t i;

		// 定員確認
		if( (g=guild_search(tsd->status.guild_id))==NULL ){
			sd.guild_invite=0;
			sd.guild_invite_account=0;
			return 0;
		}
		for(i=0;i<g->max_member;++i)
			if(g->member[i].account_id==0)
				break;
		if(i==g->max_member){
			sd.guild_invite=0;
			sd.guild_invite_account=0;
			clif_guild_inviteack(*tsd,3);
			return 0;
		}
		//inter鯖へ追加要求
		guild_makemember(m,sd);
		intif_guild_addmember( sd.guild_invite, m );
		return 0;
	}
	else
	{		// 拒否
		sd.guild_invite=0;
		sd.guild_invite_account=0;
		if(tsd==NULL)
			return 0;
		clif_guild_inviteack(*tsd,1);
	}
	return 0;
}
// ギルドメンバが追加された
int guild_member_added(uint32 guild_id,uint32 account_id,uint32 char_id,int flag)
{
	map_session_data *sd= map_session_data::from_blid(account_id),*sd2;
	struct guild *g=guild_search(guild_id);

	if( g==NULL )
		return 0;

	if(sd==NULL || sd->guild_invite==0){ // キャラ側に登録できなかったため脱退要求を出す
		if (flag == 0) {
			if(config.error_log)
				ShowMessage("guild: member added error %d is not online\n",account_id);
 			intif_guild_leave(guild_id,account_id,char_id,0,"**登録失敗**");
		}
		return 0;
	}
	sd2 = map_session_data::from_blid(sd->guild_invite_account);
	if(flag==1){	// 失敗
		if( sd2!=NULL )
			clif_guild_inviteack(*sd2,3);
		return 0;
	}

		// 成功
	sd->guild_invite=0;
	sd->guild_invite_account=0;
	sd->guild_sended=0;
	sd->status.guild_id=guild_id;

	if( sd2!=NULL )
		clif_guild_inviteack(*sd2,2);

	// いちおう競合確認
	guild_check_conflict(*sd);
	clif_charnameack(-1, *sd); //Update display name [Skotlex]
	return 0;
}

// ギルド脱退要求
int guild_leave(map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes)
{
	struct guild *g;
	int i;

	g = guild_search(sd.status.guild_id);

	if(g==NULL)
		return 0;

	if(	sd.status.account_id!=account_id ||
		sd.status.char_id!=char_id || sd.status.guild_id!=guild_id)
		return 0;

	for(i=0;i<g->max_member;++i){	// 所属しているか
		if(	g->member[i].account_id==sd.status.account_id &&
			g->member[i].char_id==sd.status.char_id ){
			intif_guild_leave(g->guild_id,sd.status.account_id,sd.status.char_id,0,mes);
			return 0;
		}
	}
	return 0;
}
// ギルド追放要求
int guild_explusion(map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes)
{
	struct guild *g;
	int i,ps;

	g = guild_search(sd.status.guild_id);

	if(g==NULL)
		return 0;

	if(	sd.status.guild_id!=guild_id)
		return 0;

	if( (ps=guild_getposition(sd,*g))<0 || !(g->position[ps].mode&0x0010) )
		return 0;	// 処罰権限無し

	for(i=0;i<g->max_member;++i){	// 所属しているか
		if(	g->member[i].account_id==account_id &&
			g->member[i].char_id==char_id )
		{
			intif_guild_leave(g->guild_id,account_id,char_id,1,mes);
			memset(&g->member[i],0,sizeof(struct guild_member));
			return 0;
		}
	}
	return 0;
}
// ギルドメンバが脱退した
int guild_member_leaved(uint32 guild_id,uint32 account_id,uint32 char_id,int flag,const char *name,const char *mes)
{
	map_session_data *sd=map_session_data::charid2sd(char_id);
	struct guild *g=guild_search(guild_id);

	if(g!=NULL){
		int i;
		for(i=0;i<g->max_member;++i) {
			if(	g->member[i].account_id==account_id &&
				g->member[i].char_id==char_id ){
				map_session_data *sd2=sd;
				if(sd2==NULL)
					sd2=guild_getavailablesd(*g);
				else
				{
					if(flag==0)
						clif_guild_leave(*sd2,name,mes);
					else
						clif_guild_explusion(*sd2,name,mes,account_id);
				}
				memset(&g->member[i],0,sizeof(struct guild_member));
			}
		}
			// メンバーリストを全員に再通知
			for(i=0;i<g->max_member;++i){
				if( g->member[i].sd!=NULL )
					clif_guild_memberlist(*g->member[i].sd);
			}
		}

	if(sd!=NULL && sd->status.guild_id==guild_id){
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
			sd->guild_sended=0;
			clif_charnameack(-1, *sd,true); //Update display name [Skotlex]
		}
	return 0;
}
// ギルドメンバのオンライン状態/Lv更新送信
int guild_send_memberinfoshort(map_session_data &sd,int online)
{
	struct guild *g;

	if(sd.status.guild_id<=0)
		return 0;
	g=guild_search(sd.status.guild_id);
	if(g==NULL)
		return 0;

	intif_guild_memberinfoshort(g->guild_id,
		sd.status.account_id,sd.status.char_id,online,sd.status.base_level,sd.status.class_);

	if( !online ){	// ログアウトするならsdをクリアして終了
		int i=guild_getindex(*g,sd.status.account_id,sd.status.char_id);
		if(i>=0)
			g->member[i].sd=NULL;
		return 0;
	}

	if( sd.guild_sended!=0 )	// ギルド初期送信データは送信済み
		return 0;

	// 競合確認
	guild_check_conflict(sd);

	// あるならギルド初期送信データ送信
	if( (g=guild_search(sd.status.guild_id))!=NULL && guild_check_member(*g) )
	{	// 所属を確認する
		if(sd.status.guild_id==g->guild_id){
			clif_guild_belonginfo(sd,*g);
			clif_guild_notice(sd,*g);
			sd.guild_sended=1;
			sd.guild_emblem_id=g->emblem_id;
		}
	}
	return 0;
}
// ギルドメンバのオンライン状態/Lv更新通知
int guild_recv_memberinfoshort(uint32 guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_)
{
	int i,alv,c,idx=-1,om=0,oldonline=-1;
	struct guild *g=guild_search(guild_id);
	if(g==NULL)
		return 0;
	for(i=0,alv=0,c=0,om=0;i<g->max_member;++i){
		struct guild_member *m=&g->member[i];
		if(m->account_id==account_id && m->char_id==char_id ){
			oldonline=m->online;
			m->online=online;
			m->lv=lv;
			m->class_=class_;
			idx=i;
		}
		if(m->account_id>0){
			alv+=m->lv;
			c++;
		}
		if(m->online)
			om++;
	}
	if(idx == -1 || c == 0) {
		// ギルドのメンバー外なので追放扱いする
		map_session_data *sd = map_session_data::from_blid(account_id);
		if(sd && sd->status.char_id == char_id) {
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
			sd->guild_sended=0;
		}
		if(config.error_log)
			ShowError("guild: not found member %d,%d on %d[%s]\n",	account_id,char_id,guild_id,g->name);
		return 0;
	}
	g->average_lv=alv/c;
	g->connect_member=om;

	if(oldonline!=online)	// オンライン状態が変わったので通知
		clif_guild_memberlogin_notice(*g,idx,online);

	for(i=0;i<g->max_member;++i){	// sd再設定
		map_session_data *sd= map_session_data::from_blid(g->member[i].account_id);
		if (sd && sd->status.char_id == g->member[i].char_id &&
			sd->status.guild_id == g->guild_id &&
			!sd->state.waitingdisconnect)
			g->member[i].sd = sd;
		else sd = NULL;
	}

	// ここにクライアントに送信処理が必要

	return 0;
}
// ギルド会話送信
int guild_send_message(map_session_data &sd,const char *mes, size_t len)
{
	if(sd.status.guild_id==0)
		return 0;
	intif_guild_message(sd.status.guild_id,sd.status.account_id,mes,len);
	guild_recv_message(sd.status.guild_id,sd.status.account_id,mes,len);

	//Chatlogging type 'G'
	if(log_config.chat&1 //we log everything then
		|| ( log_config.chat&8 //if Guild bit is on
		&& ( !agit_flag || !(log_config.chat&16) ))) //if WOE ONLY flag is off or AGIT is OFF
		log_chat("G", sd.status.guild_id, sd.status.char_id, sd.status.account_id, sd.mapname, sd.block_list::x, sd.block_list::y, "", mes);
	

	return 0;
}
// ギルド会話受信
int guild_recv_message(uint32 guild_id,uint32 account_id,const char *mes,size_t len)
{
	struct guild *g;
	if( (g=guild_search(guild_id))==NULL)
		return 0;
	clif_guild_message(*g,account_id,mes,len);
	return 0;
}
// ギルドメンバの役職変更
int guild_change_memberposition(uint32 guild_id,uint32 account_id,uint32 char_id, uint32 idx)
{
	return intif_guild_change_memberinfo(guild_id,account_id,char_id,GMI_POSITION,idx);
}
// ギルドメンバの役職変更通知
int guild_memberposition_changed(struct guild &g,unsigned short idx,unsigned short pos)
{
	if(idx<MAX_GUILD)
	{
		g.member[idx].position=pos;
		clif_guild_memberpositionchanged(g,idx);

		if( g.member[idx].sd )
			clif_charnameack(-1, *g.member[idx].sd);
	}
	return 0;
}
// ギルド役職変更
int guild_change_position(map_session_data &sd,uint32 idx,int mode,int exp_mode,const char *name)
{
	struct guild_position p;
	if(exp_mode > (int)config.guild_exp_limit)
		exp_mode=config.guild_exp_limit;
	if(exp_mode<0)exp_mode=0;
	p.mode=mode;
	p.exp_mode=exp_mode;
	memcpy(p.name,name,24);
	return intif_guild_position(sd.status.guild_id,idx,p);
}
// ギルド役職変更通知
int guild_position_changed(uint32 guild_id,uint32 idx,struct guild_position &p)
{
	size_t i;
	struct guild *g=guild_search(guild_id);
	if(g!=NULL && idx<MAX_GUILDPOSITION)
	{
		memcpy(&g->position[idx],&p,sizeof(struct guild_position));
		clif_guild_positionchanged(*g,idx);

		for(i=0; i<g->max_member; ++i)
		{	// update all members with that position
			if( g->member[i].sd && g->member[i].position==idx )
				clif_charnameack(-1, *g->member[i].sd);
		}
	}
	return 0;
}
// ギルド告知変更
int guild_change_notice(map_session_data &sd,uint32 guild_id,const char *mes1,const char *mes2)
{
	if(guild_id!=sd.status.guild_id)
		return 0;
	return intif_guild_notice(guild_id,mes1,mes2);
}
// ギルド告知変更通知
int guild_notice_changed(uint32 guild_id,const char *mes1,const char *mes2)
{
	int i;
	map_session_data *sd;
	struct guild *g=guild_search(guild_id);
	if(g==NULL)
		return 0;

	memcpy(g->mes1,mes1,60);
	memcpy(g->mes2,mes2,120);

	for(i=0;i<g->max_member;++i){
		if((sd=g->member[i].sd)!=NULL)
			clif_guild_notice(*sd,*g);
	}
	return 0;
}
// ギルドエンブレム変更
int guild_change_emblem(map_session_data &sd,int len,const unsigned char *data)
{
	struct guild *g;
	if (config.require_glory_guild &&
		!((g = guild_search(sd.status.guild_id)) &&
		guild_checkskill(*g, GD_GLORYGUILD)>0) )
	{
		sd.skill_failed(GD_GLORYGUILD);
		return 0;
	}
	return intif_guild_emblem(sd.status.guild_id,data,len);
}
// ギルドエンブレム変更通知
int guild_emblem_changed(unsigned short len,uint32 guild_id,uint32 emblem_id,const unsigned char *data)
{
	int i;
	map_session_data *sd;
	struct guild *g=guild_search(guild_id);
	if(g==NULL)
		return 0;

	memcpy(g->emblem_data,data,len);
	g->emblem_len=len;
	g->emblem_id=emblem_id;

	for(i=0;i<g->max_member;++i){
		if((sd=g->member[i].sd)!=NULL){
			sd->guild_emblem_id=emblem_id;
			clif_guild_belonginfo(*sd,*g);
			clif_guild_emblem(*sd,*g);
		}
	}
	return 0;
}

// ギルドのEXP上納
int guild_payexp(map_session_data &sd, uint32 exp)
{
	struct guild *g;
	struct guild_expcache *c;
	uint32 per, exp2;
	
	if (sd.status.guild_id == 0 ||
		(g = guild_search(sd.status.guild_id)) == NULL ||
		(per = g->position[guild_getposition(sd,*g)].exp_mode) <= 0)
		return 0;

	if (per > 100) per = 100;

	if ((exp2 = exp * per / 100) <= 0)
		return 0;

	if( (c=(struct guild_expcache *) numdb_search(guild_expcache_db,sd.status.char_id))==NULL )
	{
		c = new struct guild_expcache;
		c->guild_id=sd.status.guild_id;
		c->account_id=sd.status.account_id;
		c->char_id=sd.status.char_id;
		c->exp = exp2;
		numdb_insert(guild_expcache_db, c->char_id, c);
	}
	else
	{
		double tmp = c->exp + exp2;
		c->exp = (tmp > double(INT_MAX)) ? INT_MAX : (int)tmp;
	}
	return exp2;
}

// Celest
int guild_getexp(map_session_data &sd,int exp)
{
	if(sd.status.guild_id==0 || guild_search(sd.status.guild_id)==NULL )
		return 0;
	struct guild_expcache *c=(struct guild_expcache *) numdb_search(guild_expcache_db,sd.status.char_id);
	if( c==NULL )
	{
		c = new struct guild_expcache;
		c->guild_id=sd.status.guild_id;
		c->account_id=sd.status.account_id;
		c->char_id=sd.status.char_id;
		c->exp = exp;
		numdb_insert(guild_expcache_db,c->char_id,c);
	}
	else
	{
		double tmp = c->exp + exp;
		c->exp = (tmp > double(INT_MAX)) ? INT_MAX : (int)tmp;
	}
	return exp;
}

// スキルポイント割り振り
int guild_skillup(map_session_data &sd, unsigned short skillid,int flag)
{
	struct guild *g;
	unsigned short idx = skillid - GD_SKILLBASE;

	if(idx >= MAX_GUILDSKILL)
		return 0;

	if(sd.status.guild_id==0 || (g=guild_search(sd.status.guild_id))==NULL)
		return 0;
	if( 0 != strcmp(sd.status.name,g->master) )
		return 0;

	if( (g->skill_point>0 || flag&1) &&
		g->skill[idx].id!=0 &&
		g->skill[idx].lv < guild_skill_get_max(skillid) )
	{
		intif_guild_skillup(g->guild_id, skillid, sd.status.account_id, flag);
	}
	status_calc_pc (sd, 0); // Celest

	return 0;
}
// スキルポイント割り振り通知
int guild_skillupack(uint32 guild_id,uint32 skillid,uint32 account_id)
{
	map_session_data *sd=map_session_data::from_blid(account_id);
	struct guild *g=guild_search(guild_id);
	int i;
	if(g==NULL)
		return 0;
	if(sd!=NULL)
		clif_guild_skillup(*sd,(unsigned short)skillid,g->skill[skillid-GD_SKILLBASE].lv);
	// 全員に通知
	for(i=0;i<g->max_member;++i)
		if((sd=g->member[i].sd)!=NULL)
			clif_guild_skillinfo(*sd);
	return 0;
}

// ギルド同盟数所得
int guild_get_alliance_count(struct guild *g,int flag)
{
	int i,c;

	nullpo_retr(0, g);

	for(i=c=0;i<MAX_GUILDALLIANCE;++i){
		if(	g->alliance[i].guild_id>0 &&
			g->alliance[i].opposition==flag )
			c++;
	}
	return c;
}
// 同盟関係かどうかチェック
// 同盟なら1、それ以外は0
int guild_check_alliance(uint32 guild_id1, uint32 guild_id2, int flag)
{
	struct guild *g;
	int i;

	g = guild_search(guild_id1);
	if (g == NULL)
		return 0;

	for (i=0; i<MAX_GUILDALLIANCE; ++i)
		if ((g->alliance[i].guild_id == guild_id2) && (g->alliance[i].opposition == flag))
			return 1;

	return 0;
}
// ギルド同盟要求
int guild_reqalliance(map_session_data &sd,uint32 account_id)
{
	map_session_data *tsd= map_session_data::from_blid(account_id);
	struct guild *g[2];
	int i;

	if(agit_flag)	{	// Disable alliance creation during woe [Valaris]
		clif_displaymessage(sd.fd,"Alliances cannot be made during Guild Wars!");
		return 0;
	}	// end addition [Valaris]

	if(tsd==NULL || tsd->status.guild_id<=0)
		return 0;

	g[0]=guild_search(sd.status.guild_id);
	g[1]=guild_search(tsd->status.guild_id);

	if(g[0]==NULL || g[1]==NULL)
		return 0;

	if( guild_get_alliance_count(g[0],0)>3 )	// 同盟数確認
		clif_guild_allianceack(sd,4);
	if( guild_get_alliance_count(g[1],0)>3 )
		clif_guild_allianceack(sd,3);

	if( tsd->guild_alliance>0 )
	{	// 相手が同盟要請状態かどうか確認
		clif_guild_allianceack(sd,1);
		return 0;
	}

	for(i=0;i<MAX_GUILDALLIANCE;++i){	// すでに同盟状態か確認
		if(	g[0]->alliance[i].guild_id==tsd->status.guild_id &&
			g[0]->alliance[i].opposition==0)
		{
			clif_guild_allianceack(sd,0);
			return 0;
		}
	}

	tsd->guild_alliance=sd.status.guild_id;
	tsd->guild_alliance_account=sd.status.account_id;

	clif_guild_reqalliance(*tsd,sd.status.account_id,g[0]->name);
	return 0;
}
// ギルド勧誘への返答
int guild_reply_reqalliance(map_session_data &sd,uint32 account_id,int flag)
{
	map_session_data *tsd;
	nullpo_retr(0, tsd= map_session_data::from_blid( account_id ));

	if(sd.guild_alliance!=tsd->status.guild_id)	// 勧誘とギルドIDが違う
		return 0;

	if(flag==1)
	{	// 承諾
		size_t i;

		struct guild *g;	// 同盟数再確認
		if( (g=guild_search(sd.status.guild_id))==NULL ||
			guild_get_alliance_count(g,0)>3 ){
			clif_guild_allianceack(sd,4);
			clif_guild_allianceack(*tsd,3);
			return 0;
		}
		if( (g=guild_search(tsd->status.guild_id))==NULL ||
			guild_get_alliance_count(g,0)>3 ){
			clif_guild_allianceack(sd,3);
			clif_guild_allianceack(*tsd,4);
			return 0;
		}

		// 敵対関係なら敵対を止める
		if((g=guild_search(sd.status.guild_id)) == NULL)
			return 0;
		for(i=0;i<MAX_GUILDALLIANCE;++i){
			if(	g->alliance[i].guild_id==tsd->status.guild_id &&
				g->alliance[i].opposition==1)
				intif_guild_alliance( sd.status.guild_id,tsd->status.guild_id,
					sd.status.account_id,tsd->status.account_id,9 );
		}
		if((g=guild_search(tsd->status.guild_id)) == NULL)
			return 0;
		for(i=0;i<MAX_GUILDALLIANCE;++i){
			if(	g->alliance[i].guild_id==sd.status.guild_id &&
				g->alliance[i].opposition==1)
				intif_guild_alliance( tsd->status.guild_id,sd.status.guild_id,
					tsd->status.account_id,sd.status.account_id,9 );
		}

		// inter鯖へ同盟要請
		intif_guild_alliance( sd.status.guild_id,tsd->status.guild_id,
			sd.status.account_id,tsd->status.account_id,0 );
		return 0;
	}else{		// 拒否
		sd.guild_alliance=0;
		sd.guild_alliance_account=0;
		if(tsd!=NULL)
			clif_guild_allianceack(*tsd,3);
	}
	return 0;
}
// ギルド関係解消
int guild_delalliance(map_session_data &sd,uint32 guild_id,int flag)
{
	if(agit_flag)	{	// Disable alliance breaking during woe [Valaris]
		clif_displaymessage(sd.fd,"Alliances cannot be broken during Guild Wars!");
		return 0;
	}	// end addition [Valaris]


	intif_guild_alliance( sd.status.guild_id,guild_id,
		sd.status.account_id,0,flag|8 );
	return 0;
}
// ギルド敵対
int guild_opposition(map_session_data &sd,uint32 char_id)
{
	map_session_data *tsd=map_session_data::from_blid(char_id);
	struct guild *g;
	int i;

	g=guild_search(sd.status.guild_id);
	if(g==NULL || tsd==NULL)
		return 0;

	if( guild_get_alliance_count(g,1)>3 )	// 敵対数確認
		clif_guild_oppositionack(sd,1);

	for(i=0;i<MAX_GUILDALLIANCE;++i){	// すでに関係を持っているか確認
		if(g->alliance[i].guild_id==tsd->status.guild_id){
			if(g->alliance[i].opposition==1){	// すでに敵対
				clif_guild_oppositionack(sd,2);
				return 0;
			}else	// 同盟破棄
				intif_guild_alliance( sd.status.guild_id,tsd->status.guild_id,
					sd.status.account_id,tsd->status.account_id,8 );
		}
	}

	// inter鯖に敵対要請
	intif_guild_alliance( sd.status.guild_id,tsd->status.guild_id,
			sd.status.account_id,tsd->status.account_id,1 );
	return 0;
}
// ギルド同盟/敵対通知
int guild_allianceack(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2,int flag,const char *name1,const char *name2)
{
	struct guild *g[2];
	uint32 guild_id[2];
	const char *guild_name[2];
	map_session_data *sds[2];
	int j,i;

	guild_id[0] = guild_id1;
	guild_id[1] = guild_id2;
	guild_name[0] = name1;
	guild_name[1] = name2;
	sds[0] = map_session_data::from_blid(account_id1);
	sds[1] = map_session_data::from_blid(account_id2);

	g[0]=guild_search(guild_id1);
	g[1]=guild_search(guild_id2);

	if(sds[0]!=NULL && (flag&0x0f)==0){
		sds[0]->guild_alliance=0;
		sds[0]->guild_alliance_account=0;
	}

	if(flag&0x70){	// 失敗
		for(i=0;i<2-(flag&1);++i)
			if( sds[i]!=NULL )
				clif_guild_allianceack(*sds[i],((flag>>4)==i+1)?3:4);
		return 0;
	}
//	if(config.etc_log)
//		ShowMessage("guild alliance_ack %d %d %d %d %d %s %s\n",guild_id1,guild_id2,account_id1,account_id2,flag,name1,name2);

	if(!(flag&0x08)){	// 関係追加
		for(i=0;i<2-(flag&1);++i)
			if(g[i]!=NULL)
				for(j=0;j<MAX_GUILDALLIANCE; ++j)
					if(g[i]->alliance[j].guild_id==0){
						g[i]->alliance[j].guild_id=guild_id[1-i];
						memcpy(g[i]->alliance[j].name,guild_name[1-i],24);
						g[i]->alliance[j].opposition=flag&1;
						break;
					}
	}else{				// 関係解消
		for(i=0;i<2-(flag&1);++i){
			if(g[i]!=NULL)
				for(j=0;j<MAX_GUILDALLIANCE; ++j)
					if(	g[i]->alliance[j].guild_id==guild_id[1-i] &&
						g[i]->alliance[j].opposition==(flag&1)){
						g[i]->alliance[j].guild_id=0;
						break;
					}
			if( sds[i]!=NULL )	// 解消通知
				clif_guild_delalliance(*sds[i],guild_id[1-i],(flag&1));
		}
	}

	if((flag&0x0f)==0){			// 同盟通知
		if( sds[1]!=NULL )
			clif_guild_allianceack(*sds[1],2);
	}else if((flag&0x0f)==1){	// 敵対通知
		if( sds[0]!=NULL )
			clif_guild_oppositionack(*sds[0],0);
	}


	for(i=0;i<2-(flag&1);++i)
	{	// 同盟/敵対リストの再送信
		if(g[i]!=NULL)
		{
			for(j=0;j<g[i]->max_member; ++j)
			{
				if( g[i]->member[j].sd != NULL )
					clif_guild_allianceinfo(*g[i]->member[j].sd);
			}
		}
	}
	return 0;
}

// ギルド解散通知
int guild_broken(uint32 guild_id,int flag)
{
	guild *gg;
	if( flag==0 && (gg=guild_search(guild_id)) )
	{
		map_session_data *sd;
		int i,j;

		// clear members
		for(i=0;i<gg->max_member;++i)
		{	// ギルド解散を通知
			if((sd=gg->member[i].sd)!=NULL)
			{
				if(sd->state.storage_flag)
					storage_guild_storage_quit(*sd,1);
				sd->status.guild_id=0;
				sd->guild_sended=0;
				clif_charnameack(-1, *sd,true);
				clif_guild_broken(*sd,0);
			}
		}
		// break guild
		db_iterator<size_t, guild*> giter(guild_db);
		for(; giter; ++giter)
		{
			guild *gi=giter.data();
			if(gi)
			{
				for(i=0;i<MAX_GUILDALLIANCE;++i)
				{	// 関係を破棄
					if(gi->alliance[i].guild_id==guild_id)
					{
						for(j=0;j<gi->max_member; ++j)
						{
							if( (sd=gi->member[j].sd)!=NULL )
								clif_guild_delalliance(*sd,guild_id,gi->alliance[i].opposition);
						}
						intif_guild_alliance(gi->guild_id, guild_id,0,0,gi->alliance[i].opposition|8);
						gi->alliance[i].guild_id=0;
					}
				}
			}
		}

		// break owned castles
		db_iterator<size_t, guild_castle*> citer(castle_db);
		for(; giter; ++giter)
		{
			struct guild_castle *gc=citer.data();
			if( gc && gc->guild_id == guild_id)
			{	// Save the new 'owner', this should invoke guardian clean up and other such things.
				gc->guild_id = 0;
				guild_castledatasave(gc->castle_id, 1, 0);
			}
		}

		// remove storage
		guild_storage_delete(guild_id);

		// 
		numdb_erase(guild_db,guild_id);
		delete gg;
	}
	return 0;
}

// ギルド解散
int guild_break(map_session_data &sd, const char *name)
{
	struct guild *g;
	int i;

	if( (g=guild_search(sd.status.guild_id))==NULL )
		return 0;
	if(strcmp(g->name,name)!=0)
		return 0;
	if( 0 != strcmp(sd.status.name,g->master) )
		return 0;
	for(i=0;i<g->max_member;++i){
		if(	g->member[i].account_id>0 && (
			g->member[i].account_id!=sd.status.account_id ||
			g->member[i].char_id!=sd.status.char_id ))
			break;
	}
	if(i<g->max_member)
	{
		clif_guild_broken(sd,2);
		return 0;
	}
	intif_guild_break(g->guild_id);
	return 0;
}

// ギルド城データ要求
int guild_castledataload(unsigned short castle_id,int index)
{
	return intif_guild_castle_dataload(castle_id,index);
}
// ギルド城情報所得時イベント追加
int guild_addcastleinfoevent(unsigned short castle_id,int index,const char *name)
{
	struct eventlist *ev;
	int code=castle_id|(index<<16);
	if( name && *name )
	{
		ev = new struct eventlist(name,	(struct eventlist *) numdb_search(guild_castleinfoevent_db,code));
		numdb_insert(guild_castleinfoevent_db,code,ev);
	}
	return 0;
}

// ギルド城データ要求返信
int guild_castledataloadack(unsigned short castle_id,int index,int value)
{
	struct guild_castle *gc=guild_castle_search(castle_id);
	int code=castle_id|(index<<16);
	struct eventlist *ev,*ev2;

	if(gc==NULL){
		return 0;
	}
	switch(index){
	case 1:
	{
		gc->guild_id = value;
		//Request guild data which will be required for spawned guardians. [Skotlex]
		if (value && guild_search(value)==NULL)
			guild_request_info(value);
		break;
	}
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
		gc->guardian[index-10].visible = (value!=0); break;
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
		gc->guardian[index-18].guardian_hp = value; break;
	default:
		ShowError("guild_castledataloadack ERROR!! (Not found index=%d, castle %d)\n", index,castle_id);
		return 0;
	}
	ev=(struct eventlist *)numdb_search(guild_castleinfoevent_db,code);
	if( ev != NULL ){
		numdb_erase(guild_castleinfoevent_db,code);
		while(ev)
		{
			npc_data::event(ev->name);
			ev2=ev->next;
			delete ev;
			ev=ev2;
		}
	}
	return 1;
}
// ギルド城データ変更要求
int guild_castledatasave(unsigned short castle_id,int index,int value)
{
	return intif_guild_castle_datasave(castle_id,index,value);
}

// ギルド城データ変更通知
int guild_castledatasaveack(unsigned short castle_id,int index,int value)
{
	struct guild_castle *gc=guild_castle_search(castle_id);
	if(gc==NULL){
		return 0;
	}
	switch(index){
	case 1: gc->guild_id = value; break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
		gc->guardian[index-10].visible = (value!=0); break;
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
		gc->guardian[index-18].guardian_hp = value; break;
	default:
		ShowError("guild_castledatasaveack ERROR!! (Not found index=%d)\n", index);
		return 0;
	}
	return 1;
}

// ギルドデータ一括受信（初期化時）
int guild_castlealldataload(int len, unsigned char *buf)
{
	int i;
	int n = (len-4) / sizeof(struct guild_castle), ev = -1;
	static struct guild_castle gctmp;

	nullpo_retr(0, buf);

	// イベント付きで要求するデータ位置を探す(最後の占拠データ)
	for(i = 0; i < n; ++i) {
//		if ((gc + i)->guild_id) // this access might be unalligned
//		memcpy(&gctmp,(gc+i),sizeof(struct guild_castle)); // no memcopy, use access function
		guild_castle_frombuffer(gctmp, buf+i*sizeof(struct guild_castle));
		if ( gctmp.guild_id )
			ev = i;
	}

	// 城データ格納とギルド情報要求
	for(i = 0; i < n; ++i) {
		struct guild_castle *c;

		guild_castle_frombuffer(gctmp, buf+i*sizeof(struct guild_castle));
		c = guild_castle_search(gctmp.castle_id);
		if (!c) {
			ShowMessage("guild_castlealldataload ??\n");
			continue;
		}
		// copy values of the struct guildcastle that are included in the char server packet
		memcpy(gctmp.mapname,c->mapname,24 );
		memcpy(gctmp.castle_name,c->castle_name,24 );
		memcpy(gctmp.castle_event,c->castle_event,24 );
		// and copy the whole thing back
		memcpy(c,&gctmp,sizeof(struct guild_castle) );

		if( c->guild_id )
		{
			if(i!=ev)
				guild_request_info(c->guild_id);
			else
				guild_npc_request_info(c->guild_id, "::OnAgitInit");
		}
	}
	if (ev == -1)
		npc_data::event("OnAgitInit");
	return 0;
}

int guild_agit_start(void)
{	// Run All NPC_Event[OnAgitStart]
	int c = npc_data::event("OnAgitStart");
	ShowMessage("NPC_Event:[OnAgitStart] Run (%d) Events by @AgitStart.\n",c);
	// Start auto saving
	if(guild_save_timer!=-1)
		delete_timer (guild_save_timer, guild_save_sub);
	guild_save_timer = add_timer_interval(gettick() + GUILD_SAVE_INTERVAL, GUILD_SAVE_INTERVAL, guild_save_sub, 0, 0);
	return 0;
}

int guild_agit_end(void)
{	// Run All NPC_Event[OnAgitEnd]
	int c = npc_data::event("OnAgitEnd");
	ShowMessage("NPC_Event:[OnAgitEnd] Run (%d) Events by @AgitEnd.\n",c);
	// Stop auto saving
	if(guild_save_timer!=-1)
	{
		delete_timer (guild_save_timer, guild_save_sub);
		guild_save_timer=-1;
	}
	return 0;
}

int guild_gvg_eliminate_timer(int tid, unsigned long tick, int id, basics::numptr data)
{	// Run One NPC_Event[OnAgitEliminate]
	char *name = (char*)data.ptr;
	if(name)
	{
		if(agit_flag)	// Agit not already End
		{
			const size_t len = strlen(name); 
			char* evname = new char[len+5];
			memcpy(evname,name,len-5);
			strcpy(evname+len-5,"Eliminate");
			ShowMessage("NPC_Event:[%s] Run (%d) Events.\n", evname, npc_data::event(evname) );
			delete[] evname;
		}
		delete[] name;
		get_timer(tid)->data=0;
	}
	return 0;
}

int guild_save_sub(int tid, unsigned long tick, int id, basics::numptr data)
{
	static uint32 Ghp[MAX_GUILDCASTLE][MAX_GUARDIAN];	// so save only if HP are changed // experimental code [Yor]
	static uint32 Gid[MAX_GUILDCASTLE];
	struct guild_castle *gc;
	int i, k;

	for(i = 0; i < MAX_GUILDCASTLE; ++i)
	{	// [Yor]
		gc = guild_castle_search(i);
		if (!gc) continue;
		if (gc->guild_id != Gid[i]) {
			// Re-save guild id if its owner guild has changed
			// This should already be done in gldfunc_ev_agit.txt,
			// but since people have complained... Well x3
			guild_castledatasave(gc->castle_id, 1, gc->guild_id);
			Gid[i] = gc->guild_id;
		}
		for(k=0; k<MAX_GUARDIAN; ++k)
		{
			if(gc->guardian[k].visible && Ghp[i][0] != gc->guardian[k].guardian_hp)
			{
				guild_castledatasave(gc->castle_id, 18+k, gc->guardian[k].guardian_hp);
				Ghp[i][k] = gc->guardian[k].guardian_hp;
			}
		}
	}
	return 0;
}

int guild_agit_break(const char* eventname)
{	// Run One NPC_Event[OnAgitBreak]
	if(agit_flag)
	{
		char *evname= new char[1+strlen(eventname)];
		memcpy(evname, eventname, 1+strlen(eventname));
		add_timer(gettick()+config.gvg_eliminate_time,guild_gvg_eliminate_timer,0, basics::numptr(evname), false);
	}
	return 0;
}

// [MouseJstr]
//   How many castles does this guild have?
unsigned int guild_checkcastles(struct guild &g)
{
	size_t i, nb_cas=0;

	struct guild_castle *gc;
		
	for(i=0;i<MAX_GUILDCASTLE;++i)
	{
		gc=guild_castle_search(i);
		if(gc && gc->guild_id == g.guild_id)
			nb_cas=nb_cas+1;
		} //end for
	return nb_cas;
}

// [MouseJstr]
//    is this guild allied with this castle?
bool guild_isallied(struct guild &g, struct guild_castle &gc)
{
	size_t i;

	if(gc.guild_id == 0 || g.guild_id == 0)
		return false;

	if(g.guild_id == gc.guild_id)
		return true;

	for(i=0;i<MAX_GUILDALLIANCE;++i)
	{
		if(g.alliance[i].guild_id == gc.guild_id)
		{
			if(g.alliance[i].opposition == 0)
				return true;
			else
				return false;
		}
	}
	return false;
}
bool guild_isallied(uint32 guild_id, uint32 guild_id2)
{
	int i;
	struct guild *g1, *g2;

	if (guild_id <= 0 || guild_id2 <= 0)
		return false;
	
	g1 = guild_search(guild_id);
	g2 = guild_search(guild_id2);

	if(!g1 || !g2)
		return false;

	if(g1->guild_id == g2->guild_id)
		return true;

	for(i=0;i<MAX_GUILDALLIANCE;++i)
	{
		if(g1->alliance[i].guild_id == g2->guild_id)
		{
			if(g1->alliance[i].opposition == 0)
				return true;
			else
				return false;
		}
	}
	return false;
}

void guild_send_xy(unsigned long tick)
{
	db_iterator<size_t, guild*> iter(guild_db);
	for(; iter; ++iter)
	{
		guild *g=iter.data();
		if(g)
		{
			size_t i;
			for(i=0; i<MAX_GUILD; ++i)
			{
				map_session_data *sd = g->member[i].sd;
				if( sd )
				{	// 座標通知
					if(sd->party_x!=sd->block_list::x || sd->party_y!=sd->block_list::y)
					{
						clif_guild_xy(*sd);
						sd->party_x=sd->block_list::x;
						sd->party_y=sd->block_list::y;
					}
				}
			}
		}
	}
}

void guild_db_final(void *key,void *data)
{
	struct guild *g=(struct guild *) data;
	delete g;
}
void castle_db_final(void *key,void *data)
{
	struct guild_castle *gc=(struct guild_castle *) data;
	delete gc;
}
void guild_expcache_db_final(void *key,void *data)
{
	struct guild_expcache *c=(struct guild_expcache *) data;
	delete c;
}
void guild_infoevent_db_final(void *key,void *data)
{
	struct eventlist *ev, *ev2;
	ev =(struct eventlist *) data;
	while(ev)
	{
		ev2=ev->next;
		delete ev;
		ev=ev2;
	}
}

void do_final_guild(void)
{
	if(guild_db)
	{
		numdb_final(guild_db,guild_db_final);
		guild_db=NULL;
	}
	if(castle_db)
	{
		numdb_final(castle_db,castle_db_final);
		castle_db=NULL;
	}
	if(guild_expcache_db)
	{
		numdb_final(guild_expcache_db,guild_expcache_db_final);
		guild_expcache_db=NULL;
	}
	if(guild_infoevent_db)
	{
		numdb_final(guild_infoevent_db,guild_infoevent_db_final);
		guild_infoevent_db=NULL;
	}
	if(guild_castleinfoevent_db)
	{
		numdb_final(guild_castleinfoevent_db,guild_infoevent_db_final);
		guild_castleinfoevent_db=NULL;
	}
}

// 初期化
void do_init_guild(void)
{
	guild_db=numdb_init();
	castle_db=numdb_init();
	guild_expcache_db=numdb_init();
	guild_infoevent_db=numdb_init();
	guild_castleinfoevent_db=numdb_init();

	guild_read_castledb();

	add_timer_func_list(guild_gvg_eliminate_timer,"guild_gvg_eliminate_timer");
	add_timer_func_list(guild_payexp_timer,"guild_payexp_timer");
	add_timer_func_list(guild_save_sub, "guild_save_sub");
	add_timer_interval(gettick()+GUILD_PAYEXP_INVERVAL,GUILD_PAYEXP_INVERVAL,guild_payexp_timer,0,0);
}
