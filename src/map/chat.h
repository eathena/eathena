// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAT_H_
#define _CHAT_H_

#include "map.h"




///////////////////////////////////////////////////////////////////////////////
class chat_data : public block_list
{
public:
	/////////////////////////////////////////////////////////////////
	static chat_data* from_blid(uint32 id)
	{
		block_list *bl = block_list::from_blid(id);
		return (bl)?bl->get_cd():NULL;
	}
	/////////////////////////////////////////////////////////////////
	char pass[9];			// password (max 8)
	char title[61];			// room title (max 60)
	unsigned short limit;	// join limit
	unsigned short users;	// current users
	unsigned char trigger;
	unsigned char pub;		// room attribute
	map_session_data *usersd[20];
	block_list *owner;
	char npc_event[50];

	/////////////////////////////////////////////////////////////////
	/// constructor.
	chat_data() :
		limit(0), users(0), trigger(0), pub(0), owner(NULL)
	{
		memset(usersd,0,sizeof(usersd));
		memset(npc_event,0,sizeof(npc_event));
	}
	/////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~chat_data()	{}

	/////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual chat_data*				get_cd()				{ return this; }
	virtual const chat_data*		get_cd() const			{ return this; }


	/////////////////////////////////////////////////////////////////
	/// check ownership
	bool is_owner(map_session_data const & sd) const
	{
		return (owner)?owner->id==sd.block_list::id:false;
	}
	/////////////////////////////////////////////////////////////////
	/// 
	void kickall();
	/////////////////////////////////////////////////////////////////
	///
	void enable_event();
	/////////////////////////////////////////////////////////////////
	///
	void disable_event();
	/////////////////////////////////////////////////////////////////
	/// create a chat.
	static bool create(map_session_data& sd, unsigned short limit, unsigned char pub, const char* pass, const char* title);
	/////////////////////////////////////////////////////////////////
	/// kick user from chat
	bool kick(const char *kickusername);
	/////////////////////////////////////////////////////////////////
	/// removes session from chat
	bool remove(map_session_data &sd);




};



int chat_joinchat(map_session_data &sd,uint32 chatid,const char* pass);
int chat_changechatowner(map_session_data &sd,const char *nextownername);
int chat_changechatstatus(map_session_data &sd,unsigned short limit,unsigned char pub,const char* pass,const char* title, size_t titlelen);
int chat_createnpcchat(npc_data &nd,unsigned short limit,unsigned char pub, int trigger,const char* title, unsigned short titlelen,const char *ev);
int chat_deletenpcchat(npc_data &nd);

#endif
