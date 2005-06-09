// $Id: party.h,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _PARTY_H_
#define _PARTY_H_

#include <stdarg.h>

struct party;
struct map_session_data;
struct block_list;

void do_init_party(void);
void do_final_party(void);
struct party *party_search(unsigned long party_id);
struct party* party_searchname(const char *str);

int party_create(struct map_session_data &sd,const char *name, int item, int item2);
int party_created(unsigned long account_id,int fail,unsigned long party_id,const char *name);
int party_request_info(unsigned long party_id);
int party_invite(struct map_session_data &sd,unsigned long account_id);
int party_member_added(unsigned long party_id,unsigned long account_id,int flag);
int party_leave(struct map_session_data &sd);
int party_removemember(struct map_session_data &sd,unsigned long account_id,const char *name);
int party_member_leaved(unsigned long party_id,unsigned long account_id, const char *name);
int party_reply_invite(struct map_session_data &sd,unsigned long account_id,int flag);
int party_recv_noinfo(unsigned long party_id);
int party_recv_info(struct party &sp);
int party_recv_movemap(unsigned long party_id,unsigned long account_id,const char *map,int online,unsigned short lv);
int party_broken(unsigned long party_id);
int party_optionchanged(unsigned long party_id,unsigned long account_id,int exp,int item,int flag);
int party_changeoption(struct map_session_data &sd,int exp,int item);

int party_send_movemap(struct map_session_data &sd);
int party_send_logout(struct map_session_data &sd);

int party_send_message(struct map_session_data &sd,const char *mes,size_t len);
int party_recv_message(unsigned long party_id,unsigned long account_id,const char *mes,size_t len);

int party_check_conflict(struct map_session_data &sd);

int party_send_xy_clear(struct party &p);
int party_send_hp_check(struct block_list &bl,va_list ap);

int party_exp_share(struct party &p,unsigned short map,unsigned long base_exp,unsigned long job_exp,unsigned long zeny);

void party_foreachsamemap(int (*func)(struct block_list&,va_list),struct map_session_data &sd,int type,...);

int party_send_dot_remove(struct map_session_data &sd);

#endif
