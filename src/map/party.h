// $Id: party.h,v 1.3 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _PARTY_H_
#define _PARTY_H_

#include "map.h"

struct party;
struct map_session_data;
struct block_list;

void do_init_party(void);
void do_final_party(void);
struct party *party_search(uint32 party_id);
struct party* party_searchname(const char *str);

int party_create(struct map_session_data &sd,const char *name, int item, int item2);
int party_created(uint32 account_id,int fail,uint32 party_id,const char *name);
int party_request_info(uint32 party_id);
int party_invite(struct map_session_data &sd,uint32 account_id);
int party_member_added(uint32 party_id,uint32 account_id,int flag);
int party_leave(struct map_session_data &sd);
int party_removemember(struct map_session_data &sd,uint32 account_id,const char *name);
int party_member_leaved(uint32 party_id,uint32 account_id, const char *name);
int party_reply_invite(struct map_session_data &sd,uint32 account_id,int flag);
int party_recv_noinfo(uint32 party_id);
int party_recv_info(struct party &sp);
int party_recv_movemap(uint32 party_id,uint32 account_id,const char *map,int online,unsigned short lv);
int party_broken(uint32 party_id);
int party_optionchanged(uint32 party_id,uint32 account_id,unsigned short expshare,unsigned short itemshare,unsigned char flag);
int party_changeoption(struct map_session_data &sd,unsigned short expshare,unsigned short itemshare);

int party_send_movemap(struct map_session_data &sd);
int party_send_logout(struct map_session_data &sd);

int party_send_message(struct map_session_data &sd,const char *mes,size_t len);
int party_recv_message(uint32 party_id,uint32 account_id,const char *mes,size_t len);

int party_check_conflict(struct map_session_data &sd);

int party_send_xy_clear(struct party &p);
//int party_send_hp_check(struct block_list &bl,va_list &ap);
class CPartySendHP : public CMapProcessor
{
	uint32 party_id;
	int& flag;
public:
	CPartySendHP(uint32 p, int& f) : party_id(p), flag(f)	{}
	~CPartySendHP()	{}
	virtual int process(struct block_list& bl) const;
};
int party_exp_share(struct party &p,unsigned short map,uint32 base_exp,uint32 job_exp,uint32 zeny);
int party_send_dot_remove(struct map_session_data &sd);

#endif
