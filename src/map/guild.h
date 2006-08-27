// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILD_H_
#define _GUILD_H_

struct map_session_data;
struct mob_data;
struct guild;
struct guild_member;
struct guild_position;
struct guild_castle;

int guild_skill_get_inf(unsigned short id);
int guild_skill_get_max(unsigned short id);

int guild_checkskill(struct guild &g,unsigned short id);
unsigned int guild_checkcastles(struct guild &g); // [MouseJstr]
bool guild_isallied(struct guild &g, struct guild_castle &gc);
bool guild_isallied(uint32 guild_id, uint32 guild_id2);

void do_init_guild(void);
struct guild *guild_search(uint32 guild_id);
struct guild *guild_searchname(const char *str);
struct guild_castle *guild_castle_search(uint32 gcid);

struct guild_castle *guild_mapname2gc(const char *mapname);

struct map_session_data *guild_getavailablesd(struct guild &g);
int guild_getindex(struct guild &g,uint32 account_id,uint32 char_id);
int guild_getposition(struct map_session_data &sd,struct guild &g);
int guild_payexp(struct map_session_data &sd,uint32 exp);
int guild_getexp(struct map_session_data &sd,int exp); // [Celest]

int guild_create(struct map_session_data &sd,const char *name);
int guild_created(uint32 account_id,uint32 guild_id);
int guild_request_info(uint32 guild_id);
int guild_recv_noinfo(uint32 guild_id);
int guild_recv_info(struct guild &sg);
int guild_npc_request_info(uint32 guild_id,const char *ev);
int guild_invite(struct map_session_data &sd,uint32 account_id);
int guild_reply_invite(struct map_session_data &sd,uint32 guild_id,int flag);
int guild_member_added(uint32 guild_id,uint32 account_id,uint32 char_id,int flag);
int guild_leave(struct map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes);
int guild_member_leaved(uint32 guild_id,uint32 account_id,uint32 char_id,int flag, const char *name,const char *mes);
int guild_explusion(struct map_session_data &sd,uint32 guild_id,uint32 account_id,uint32 char_id,const char *mes);
int guild_skillup(struct map_session_data &sd,unsigned short skill_num,int flag);
int guild_reqalliance(struct map_session_data &sd,uint32 account_id);
int guild_reply_reqalliance(struct map_session_data &sd,uint32 account_id,int flag);
int guild_alliance(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2);
int guild_allianceack(uint32 guild_id1,uint32 guild_id2,uint32 account_id1,uint32 account_id2,int flag,const char *name1,const char *name2);
int guild_delalliance(struct map_session_data &sd,uint32 guild_id,int flag);
int guild_opposition(struct map_session_data &sd,uint32 char_id);
int guild_check_alliance(uint32 guild_id1, uint32 guild_id2, int flag);

int guild_send_memberinfoshort(struct map_session_data &sd,int online);
int guild_recv_memberinfoshort(uint32 guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_);
int guild_change_memberposition(uint32 guild_id,uint32 account_id,uint32 char_id,uint32 idx);
int guild_memberposition_changed(struct guild &g,unsigned short idx,unsigned short pos);
int guild_change_position(struct map_session_data &sd,uint32 idx,int mode,int exp_mode,const char *name);
int guild_position_changed(uint32 guild_id,uint32 idx,struct guild_position &p);
int guild_change_notice(struct map_session_data &sd,uint32 guild_id,const char *mes1,const char *mes2);
int guild_notice_changed(uint32 guild_id,const char *mes1,const char *mes2);
int guild_change_emblem(struct map_session_data &sd,int len,const unsigned char *data);
int guild_emblem_changed(unsigned short len,uint32 guild_id,uint32 emblem_id,const unsigned char *data);
int guild_send_message(struct map_session_data &sd,const char *mes,size_t len);
int guild_recv_message(uint32 guild_id,uint32 account_id,const char *mes,size_t len);
int guild_skillupack(uint32 guild_id,uint32 skillid,uint32 account_id);
int guild_break(struct map_session_data &sd,const char *name);
int guild_broken(uint32 guild_id,int flag);

int guild_addcastleinfoevent(unsigned short castle_id,int index,const char *name);
int guild_castledataload(unsigned short castle_id,int index);
int guild_castledataloadack(unsigned short castle_id,int index,int value);
int guild_castledatasave(unsigned short castle_id,int index,int value);
int guild_castledatasaveack(unsigned short castle_id,int index,int value);
int guild_castlealldataload(int len, unsigned char *buf);

int guild_agit_start(void);
int guild_agit_end(void);
int guild_agit_break(struct mob_data &md);

void guild_send_xy(unsigned long tick);
void do_final_guild(void);

#endif
