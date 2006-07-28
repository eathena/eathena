// $Id: chat.c,v 1.2 2004/09/22 02:59:47 Akitasha Exp $
#include "nullpo.h"
#include "showmsg.h"
#include "utils.h"

#include "chat.h"
#include "battle.h"
#include "map.h"
#include "clif.h"
#include "pc.h"
#include "npc.h"

int chat_triggerevent(chat_data &cd);

/*==========================================
 * チャットルーム作成
 *------------------------------------------
 */
int chat_createchat(struct map_session_data &sd,unsigned short limit,unsigned char pub,char* pass,char* title,size_t titlelen)
{
	if( !sd.chatID )
	{
		chat_data *cd = new chat_data;

		cd->block_list::id	= map_addobject(*cd);
		if(cd->block_list::id==0)
		{
			clif_createchat(sd,1);
			delete cd;
			return 0;
		}

		cd->block_list::m		= sd.block_list::m;
		cd->block_list::x		= sd.block_list::x;
		cd->block_list::y		= sd.block_list::y;
		cd->block_list::type	= BL_CHAT;

		cd->owner	= &sd;
		cd->usersd[0] = &sd;
		
		cd->limit = limit;
		cd->pub = pub;
		cd->users = 1;

		memcpy(cd->pass,pass,sizeof(pass)-1);
		pass[sizeof(pass)-1]=0;

		if( titlelen+1>=sizeof(cd->title)) titlelen=sizeof(cd->title)-1;
		memcpy(cd->title,title,titlelen);
		cd->title[titlelen]=0;

		pc_setchatid(sd, cd->block_list::id);
		clif_createchat(sd,0);
		clif_dispchat(*cd,0);
	}
	return 0;
}

/*==========================================
 * 既存チャットルームに参加
 *------------------------------------------
 */
int chat_joinchat(struct map_session_data &sd,uint32 chatid,const char* pass)
{
	chat_data *cd = (chat_data*)map_id2bl(chatid);

	if( !cd || cd->block_list::m != sd.block_list::m || sd.chatID || cd->limit <= cd->users )
	{
		clif_joinchatfail(sd,0);
		return 0;
	}

	//Allows Gm access to protected room with any password they want by valaris
	if( (cd->pub == 0 && 0!=strncmp(pass, (char *)cd->pass, 8) && (sd.isGM()<config.gm_join_chat || !config.gm_join_chat)) 
		|| chatid == sd.chatID ) //Double Chat fix by Alex14, thx CHaNGeTe
	{
		clif_joinchatfail(sd,1);
		return 0;
	}

	cd->usersd[cd->users] = &sd;
	cd->users++;

	pc_setchatid(sd,cd->block_list::id);

	clif_joinchatok(sd,*cd);	// 新たに参加した人には全員のリスト
	clif_addchat(*cd,sd);	// 既に中に居た人には追加した人の報告
	clif_dispchat(*cd,0);	// 周囲の人には人数変化報告

	chat_triggerevent(*cd); // イベント

	return 0;
}

/*==========================================
 * チャットルームから抜ける
 *------------------------------------------
 */
int chat_leavechat(struct map_session_data &sd)
{
	chat_data *cd;
	size_t leavechar;

	cd=(chat_data*)map_id2bl(sd.chatID);
	if(cd==NULL)
		return 1;

	for(leavechar=0; leavechar<cd->users; ++leavechar)
	{
		if(cd->usersd[leavechar] == &sd)
		{
			break;
		}
	}
	if(leavechar>=cd->users)	// そのchatに所属していないらしい (バグ時のみ)
		return -1;

	if(leavechar==0 && cd->users>1 && cd->owner && cd->owner->type==BL_PC)
	{	// 所有者だった&他に人が居る&PCのチャット
		clif_changechatowner(*cd,*cd->usersd[1]);
		clif_clearchat(*cd,0);
	}

	// 抜けるPCにも送るのでusersを減らす前に実行
	clif_leavechat(*cd,sd);

	cd->users--;
	pc_setchatid(sd,0);

	if(cd->users == 0 && cd->owner && cd->owner->type==BL_PC)
	{	// 全員居なくなった&PCのチャットなので消す
		clif_clearchat(*cd,0);
		map_delobject(cd->block_list::id);	// freeまでしてくれる
	}
	else
	{
		size_t i;
		for(i=leavechar; i+1<cd->users; ++i)
			cd->usersd[i] = cd->usersd[i+1];
		if(leavechar==0 && cd->owner && cd->owner->type==BL_PC)
		{	// PCのチャットなので所有者が抜けたので位置変更
			cd->block_list::x = cd->owner->x;
			cd->block_list::y = cd->owner->y;
		}
		clif_dispchat(*cd,0);
	}
	return 0;
}

/*==========================================
 * チャットルームの持ち主を譲る
 *------------------------------------------
 */
int chat_changechatowner(struct map_session_data &sd, const char *nextownername)
{
	chat_data *cd;
	struct map_session_data *tmp_sd;
	size_t nextowner;

	cd=(chat_data*)map_id2bl(sd.chatID);
	if(cd==NULL || &sd!=cd->owner)
		return 1;

	for(nextowner=1; nextowner<cd->users; ++nextowner){
		if(strcmp(cd->usersd[nextowner]->status.name,nextownername)==0){
			break;
		}
	}
	if(nextowner>=cd->users) // そんな人は居ない
		return -1;

	clif_changechatowner(*cd,*cd->usersd[nextowner]);
	// 一旦消す
	clif_clearchat(*cd,0);

	// userlistの順番変更 (0が所有者なので)
	if( (tmp_sd = cd->usersd[0]) == NULL )
		return 1; //ありえるのかな？
	cd->usersd[0] = cd->usersd[nextowner];
	cd->usersd[nextowner] = tmp_sd;

	// 新しい所有者の位置へ変更
	cd->block_list::x=cd->usersd[0]->block_list::x;
	cd->block_list::y=cd->usersd[0]->block_list::y;

	// 再度表示
	clif_dispchat(*cd,0);

	return 0;
}

/*==========================================
 * チャットの状態(タイトル等)を変更
 *------------------------------------------
 */
int chat_changechatstatus(struct map_session_data &sd,unsigned short limit,unsigned char pub,const char* pass,const char* title, size_t titlelen)
{
	chat_data *cd;

	cd=(chat_data*)map_id2bl(sd.chatID);
	if(cd==NULL || &sd!=cd->owner)
		return 1;

	cd->limit = limit;
	cd->pub = pub;
	memcpy(cd->pass,pass,sizeof(pass)-1);
	cd->pass[sizeof(pass)-1]=0;

	if(titlelen+1>=sizeof(cd->title)) titlelen=sizeof(cd->title)-1;
	memcpy(cd->title,title,titlelen);
	cd->title[titlelen]=0;

	clif_changechatstatus(*cd);
	clif_dispchat(*cd,0);

	return 0;
}

/*==========================================
 * チャットルームから蹴り出す
 *------------------------------------------
 */
int chat_kickchat(struct map_session_data &sd, const char *kickusername)
{
	chat_data *cd;
	size_t kickuser;

	cd = (chat_data *)map_id2bl(sd.chatID);
	if( cd == NULL )
		return 1;

	for(kickuser=1; kickuser<cd->users; kickuser++)
	{
		if( cd->usersd[kickuser] && 
			0==strcmp(cd->usersd[kickuser]->status.name, kickusername) )
		{
			if( cd->usersd[kickuser]->isGM() < config.gm_kick_chat )
				chat_leavechat( *(cd->usersd[kickuser]) );
			return 0;
		}
	}

	return -1;
}

/*==========================================
 * npcチャットルーム作成
 *------------------------------------------
 */
int chat_createnpcchat(struct npc_data &nd,unsigned short limit,unsigned char pub, int trigger,const char* title, unsigned short titlelen,const char *ev)
{
	chat_data *cd;

	cd = new chat_data;

	cd->limit = cd->trigger = limit;
	if(trigger>0)
		cd->trigger = trigger;
	cd->pub = pub;
	cd->users = 0;
	memcpy(cd->pass,"",1);
	if((size_t)titlelen+1>=sizeof(cd->title)) titlelen=sizeof(cd->title)-1;
	memcpy(cd->title,title,titlelen);
	cd->title[titlelen]=0;

	cd->block_list::m = nd.block_list::m;
	cd->block_list::x = nd.block_list::x;
	cd->block_list::y = nd.block_list::y;
	cd->block_list::type = BL_CHAT;
	cd->owner = &nd;
	memcpy(cd->npc_event,ev,strlen(ev));

	cd->block_list::id = map_addobject(*cd);	
	if(cd->block_list::id==0)
	{
		delete cd;
		return 0;
	}
	nd.chat_id=cd->block_list::id;

	clif_dispchat(*cd,0);

	return 0;
}
/*==========================================
 * npcチャットルーム削除
 *------------------------------------------
 */
int chat_deletenpcchat(struct npc_data &nd)
{
	chat_data *cd;

	nullpo_retr(0, cd=(chat_data*)map_id2bl(nd.chat_id));
	
	cd->kickall();
	clif_clearchat(*cd,0);
	map_delobject(cd->block_list::id);	// freeまでしてくれる
	nd.chat_id=0;
	
	return 0;
}

/*==========================================
 * 規定人数以上でイベントが定義されてるなら実行
 *------------------------------------------
 */
int chat_triggerevent(chat_data &cd)
{
	if(cd.users>=cd.trigger && cd.npc_event[0])
		npc_event_do(cd.npc_event);
	return 0;
}

/*==========================================
 * イベントの有効化
 *------------------------------------------
 */
void chat_data::enable_event()
{
	this->trigger&=0x7f;
	chat_triggerevent(*this);
}
/*==========================================
 * イベントの無効化
 *------------------------------------------
 */
void chat_data::disable_event()
{
	this->trigger|=0x80;
}
/*==========================================
 * チャットルームから全員蹴り出す
 *------------------------------------------
 */
void chat_data::kickall()
{
	while(this->users>0 && this->usersd[this->users-1])
		chat_leavechat( *this->usersd[this->users-1] );
}
