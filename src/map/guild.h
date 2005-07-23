// $Id: guild.h,v 1.4 2004/09/25 05:32:18 MouseJstr Exp $
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

void do_init_guild(void);
struct guild *guild_search(unsigned long guild_id);
struct guild *guild_searchname(const char *str);
struct guild_castle *guild_castle_search(unsigned long gcid);

struct guild_castle *guild_mapname2gc(const char *mapname);

struct map_session_data *guild_getavailablesd(struct guild &g);
int guild_getindex(struct guild &g,unsigned long account_id,unsigned long char_id);
int guild_getposition(struct map_session_data &sd,struct guild &g);
int guild_payexp(struct map_session_data &sd,unsigned long exp);
int guild_getexp(struct map_session_data &sd,int exp); // [Celest]

int guild_create(struct map_session_data &sd,const char *name);
int guild_created(unsigned long account_id,unsigned long guild_id);
int guild_request_info(unsigned long guild_id);
int guild_recv_noinfo(unsigned long guild_id);
int guild_recv_info(struct guild &sg);
int guild_npc_request_info(unsigned long guild_id,const char *ev);
int guild_invite(struct map_session_data &sd,unsigned long account_id);
int guild_reply_invite(struct map_session_data &sd,unsigned long guild_id,int flag);
int guild_member_added(unsigned long guild_id,unsigned long account_id,unsigned long char_id,int flag);
int guild_leave(struct map_session_data &sd,unsigned long guild_id,unsigned long account_id,unsigned long char_id,const char *mes);
int guild_member_leaved(unsigned long guild_id,unsigned long account_id,unsigned long char_id,int flag, const char *name,const char *mes);
int guild_explusion(struct map_session_data &sd,unsigned long guild_id,unsigned long account_id,unsigned long char_id,const char *mes);
int guild_skillup(struct map_session_data &sd,unsigned short skill_num,int flag);
int guild_reqalliance(struct map_session_data &sd,unsigned long account_id);
int guild_reply_reqalliance(struct map_session_data &sd,unsigned long account_id,int flag);
int guild_alliance(unsigned long guild_id1,unsigned long guild_id2,unsigned long account_id1,unsigned long account_id2);
int guild_allianceack(unsigned long guild_id1,unsigned long guild_id2,unsigned long account_id1,unsigned long account_id2,int flag,const char *name1,const char *name2);
int guild_delalliance(struct map_session_data &sd,unsigned long guild_id,int flag);
int guild_opposition(struct map_session_data &sd,unsigned long char_id);
int guild_check_alliance(unsigned long guild_id1, unsigned long guild_id2, int flag);

int guild_send_memberinfoshort(struct map_session_data &sd,int online);
int guild_recv_memberinfoshort(unsigned long guild_id,unsigned long account_id,unsigned long char_id,int online,int lv,int class_);
int guild_change_memberposition(unsigned long guild_id,unsigned long account_id,unsigned long char_id,unsigned long idx);
int guild_memberposition_changed(struct guild &g,unsigned short idx,unsigned short pos);
int guild_change_position(struct map_session_data &sd,unsigned long idx,int mode,int exp_mode,const char *name);
int guild_position_changed(unsigned long guild_id,unsigned long idx,struct guild_position &p);
int guild_change_notice(struct map_session_data &sd,unsigned long guild_id,const char *mes1,const char *mes2);
int guild_notice_changed(unsigned long guild_id,const char *mes1,const char *mes2);
int guild_change_emblem(struct map_session_data &sd,int len,const unsigned char *data);
int guild_emblem_changed(unsigned short len,unsigned long guild_id,unsigned long emblem_id,const unsigned char *data);
int guild_send_message(struct map_session_data &sd,const char *mes,size_t len);
int guild_recv_message(unsigned long guild_id,unsigned long account_id,const char *mes,size_t len);
int guild_skillupack(unsigned long guild_id,unsigned long skillid,unsigned long account_id);
int guild_break(struct map_session_data &sd,const char *name);
int guild_broken(unsigned long guild_id,int flag);

int guild_addcastleinfoevent(unsigned short castle_id,int index,const char *name);
int guild_castledataload(unsigned short castle_id,int index);
int guild_castledataloadack(unsigned short castle_id,int index,int value);
int guild_castledatasave(unsigned short castle_id,int index,int value);
int guild_castledatasaveack(unsigned short castle_id,int index,int value);
int guild_castlealldataload(int len, unsigned char *buf);

int guild_agit_start(void);
int guild_agit_end(void);
int guild_agit_break(struct mob_data &md);

void do_final_guild(void);

#endif
