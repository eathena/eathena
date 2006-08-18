// $Id: chat.h,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _CHAT_H_
#define _CHAT_H_

#include "map.h"




///////////////////////////////////////////////////////////////////////////////
class chat_data : public block_list
{
public:

	char pass[9];			// password (max 8)
	char title[61];			// room title (max 60)
	unsigned short limit;	// join limit
	unsigned short users;	// current users
	unsigned char trigger;
	unsigned char pub;		// room attribute
	struct map_session_data *usersd[20];
	struct block_list *owner;
	char npc_event[50];

	chat_data() :
		limit(0), users(0), trigger(0), pub(0), owner(NULL)
	{
		memset(usersd,0,sizeof(usersd));
		memset(npc_event,0,sizeof(npc_event));
	}
	virtual ~chat_data()	{}

	///////////////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual chat_data*				get_cd()				{ return this; }
	virtual const chat_data*		get_cd() const			{ return this; }

	void kickall();
	void enable_event();
	void disable_event();
};


int chat_createchat(struct map_session_data &sd,unsigned short limit,unsigned char pub,char* pass,char* title,size_t titlelen);
int chat_joinchat(struct map_session_data &sd,uint32 chatid,const char* pass);
int chat_leavechat(struct map_session_data &sd);
int chat_changechatowner(struct map_session_data &sd,const char *nextownername);
int chat_changechatstatus(struct map_session_data &sd,unsigned short limit,unsigned char pub,const char* pass,const char* title, size_t titlelen);
int chat_kickchat(struct map_session_data &sd,const char *kickusername);

int chat_createnpcchat(struct npc_data &nd,unsigned short limit,unsigned char pub, int trigger,const char* title, unsigned short titlelen,const char *ev);
int chat_deletenpcchat(struct npc_data &nd);

#endif
