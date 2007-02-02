// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILD_H_
#define _GUILD_H_

#include "basetypes.h"

struct map_session_data;
struct mob_data;
struct guild;
struct guild_member;
struct guild_position;
struct guild_castle;

int guild_skill_get_inf(unsigned short id);
int guild_skill_get_max(unsigned short id);

int guild_checkskill(const guild &g,unsigned short id);
unsigned int guild_checkcastles(guild &g); // [MouseJstr]
bool guild_isallied(guild &g, guild_castle &gc);
bool guild_isallied(uint32 guild_id, uint32 guild_id2);

void do_init_guild(void);
guild *guild_search(uint32 guild_id);
guild *guild_searchname(const char *str);
guild_castle *guild_castle_search(uint32 gcid);

guild_castle *guild_mapname2gc(const char *mapname);

map_session_data *guild_getavailablesd(guild &g);
int guild_getindex(guild &g,uint32 account_id,uint32 char_id);
int guild_getposition(map_session_data &sd,guild &g);
int guild_payexp(map_session_data &sd,uint32 exp);
int guild_getexp(map_session_data &sd,int exp); // [Celest]

int guild_create(map_session_data &sd,const char *name);
int guild_created(uint32 account_id,uint32 guild_id);
int guild_request_info(uint32 guild_id);
int guild_recv_noinfo(uint32 guild_id);
int guild_recv_info(guild &sg);
int guild_npc_request_info(uint32 guild_id,const char *ev);
int guild_invite(map_session_data &sd,uint32 account_id);
int guild_reply_invite(map_session_data &sd,uint32 guild_id,int flag);
int guild_member_added(uint32 guild_id,uint32 account_id,uint32 char_id,int flag);
int guild_leave(map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes);
int guild_member_leaved(uint32 guild_id,uint32 account_id,uint32 char_id,int flag, const char *name,const char *mes);
int guild_explusion(map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes);
int guild_skillup(map_session_data &sd,unsigned short skill_num,int flag);
int guild_reqalliance(map_session_data &sd,uint32 account_id);
int guild_reply_reqalliance(map_session_data &sd,uint32 account_id,int flag);
int guild_alliance(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2);
int guild_allianceack(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2,int flag,const char *name1,const char *name2);
int guild_delalliance(map_session_data &sd,uint32 guild_id,int flag);
int guild_opposition(map_session_data &sd,uint32 char_id);
int guild_check_alliance(uint32 guild_id1, uint32 guild_id2, int flag);

int guild_send_memberinfoshort(map_session_data &sd,int online);
int guild_recv_memberinfoshort(uint32 guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_);
int guild_change_memberposition(uint32 guild_id,uint32 account_id,uint32 char_id,uint32 idx);
int guild_memberposition_changed(guild &g,unsigned short idx,unsigned short pos);
int guild_change_position(map_session_data &sd,uint32 idx,int mode,int exp_mode,const char *name);
int guild_position_changed(uint32 guild_id,uint32 idx,guild_position &p);
int guild_change_notice(map_session_data &sd,uint32 guild_id,const char *mes1,const char *mes2);
int guild_notice_changed(uint32 guild_id,const char *mes1,const char *mes2);
int guild_change_emblem(map_session_data &sd,int len,const unsigned char *data);
int guild_emblem_changed(unsigned short len,uint32 guild_id,uint32 emblem_id,const unsigned char *data);
int guild_send_message(map_session_data &sd,const char *mes,size_t len);
int guild_recv_message(uint32 guild_id,uint32 account_id,const char *mes,size_t len);
int guild_skillupack(uint32 guild_id,uint32 skillid,uint32 account_id);
int guild_break(map_session_data &sd,const char *name);
int guild_broken(uint32 guild_id,int flag);

int guild_addcastleinfoevent(unsigned short castle_id,int index,const char *name);
int guild_castledataload(unsigned short castle_id,int index);
int guild_castledataloadack(unsigned short castle_id,int index,int value);
int guild_castledatasave(unsigned short castle_id,int index,int value);
int guild_castledatasaveack(unsigned short castle_id,int index,int value);
int guild_castlealldataload(int len, unsigned char *buf);

int guild_agit_start(void);
int guild_agit_end(void);
int guild_agit_break(const char* eventname);

void guild_send_xy(unsigned long tick);
void do_final_guild(void);

#endif
