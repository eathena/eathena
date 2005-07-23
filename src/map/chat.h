// $Id: chat.h,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _CHAT_H_
#define _CHAT_H_

#include "map.h"

int chat_createchat(struct map_session_data &sd,unsigned short limit,unsigned char pub,char* pass,char* title,size_t titlelen);
int chat_joinchat(struct map_session_data &sd,unsigned long chatid,const char* pass);
int chat_leavechat(struct map_session_data &sd);
int chat_changechatowner(struct map_session_data &sd,const char *nextownername);
int chat_changechatstatus(struct map_session_data &sd,unsigned short limit,unsigned char pub,const char* pass,const char* title, size_t titlelen);
int chat_kickchat(struct map_session_data &sd,const char *kickusername);

int chat_createnpcchat(struct npc_data &nd,unsigned short limit,unsigned char pub, int trigger,const char* title, unsigned short titlelen,const char *ev);
int chat_deletenpcchat(struct npc_data &nd);
int chat_enableevent(struct chat_data &cd);
int chat_disableevent(struct chat_data &cd);
int chat_npckickall(struct chat_data &cd);

#endif
