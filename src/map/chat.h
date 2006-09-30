// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAT_H_
#define _CHAT_H_

#include "pc.h"


///////////////////////////////////////////////////////////////////////////////
/// a (player) chat object
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
	unsigned char pub;		// room attribute
	map_session_data *usersd[20];
	block_list *owner;
protected:
	/////////////////////////////////////////////////////////////////
	/// constructor.
	/// cannot be only constructed via create function
	chat_data() :
		limit(0), users(0), pub(0), owner(NULL)
	{
		memset(usersd,0,sizeof(usersd));
	}
public:
	/////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~chat_data()	{}

	/////////////////////////////////////////////////////////////////
	/// upcasting overloads.
	virtual bool is_type(object_t t) const
	{
		return (t==BL_ALL) || (t==BL_CHAT);
	}
	virtual chat_data*				get_cd()				{ return this; }
	virtual const chat_data*		get_cd() const			{ return this; }


	/////////////////////////////////////////////////////////////////
	/// check ownership
	bool is_owner(map_session_data const & sd) const
	{
		return (owner)?owner->id==sd.block_list::id:false;
	}

	/////////////////////////////////////////////////////////////////
	/// create a chat.
	static bool create(map_session_data& sd, unsigned short limit, unsigned char pub, const char* pass, const char* title);
	static bool join(map_session_data &sd, uint32 chatid, const char* pass);


	/////////////////////////////////////////////////////////////////
	/// kick user from chat
	bool kick(const char *kickusername);
	/////////////////////////////////////////////////////////////////
	/// 
	void kickall();
	/////////////////////////////////////////////////////////////////
	/// removes session from chat
	bool remove(map_session_data &sd);
	/////////////////////////////////////////////////////////////////
	/// change the owner.
	bool change_owner(map_session_data &sd, const char *nextownername);
	/////////////////////////////////////////////////////////////////
	/// change the status.
	bool change_status(map_session_data &sd, unsigned short limit, unsigned char pub, const char* pass, const char* title);
	/////////////////////////////////////////////////////////////////
	///
	virtual void enable_event()		{}
	/////////////////////////////////////////////////////////////////
	///
	virtual void disable_event()	{}
	/////////////////////////////////////////////////////////////////
	///
	virtual void trigger_event()	{}
};



///////////////////////////////////////////////////////////////////////////////
/// a npc chat object.
/// created via script, inherits from chat_data
class npcchat_data : public chat_data
{
public:
	char npc_event[50];
	unsigned char trigger;
	
private:
	/////////////////////////////////////////////////////////////////
	/// constructor.
	/// cannot be only constructed via create function
	npcchat_data() : trigger(0)
	{
		memset(npc_event,0,sizeof(npc_event));
	}
public:
	/////////////////////////////////////////////////////////////////
	/// destructor.
	virtual ~npcchat_data()	{}

	/////////////////////////////////////////////////////////////////
	///
	static bool create(npcscript_data &nd, unsigned short limit, unsigned char pub, int trigger, const char* title, const char *ev);
	/////////////////////////////////////////////////////////////////
	///
	static bool erase(npcscript_data &nd);


	/////////////////////////////////////////////////////////////////
	///
	virtual void enable_event();
	/////////////////////////////////////////////////////////////////
	///
	virtual void disable_event();
	/////////////////////////////////////////////////////////////////
	///
	virtual void trigger_event();
};


#endif
