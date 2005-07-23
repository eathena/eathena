// $Id: npc.h,v 1.5 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _NPC_H_
#define _NPC_H_

#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define INVISIBLE_CLASS 32767


void npc_chat_finalize(struct npc_data *nd);

int npc_chat_sub(struct block_list &bl, va_list ap);
int npc_event_dequeue(struct map_session_data &sd);
int npc_event_timer(int tid,unsigned long tick,int id,int data);
int npc_event(struct map_session_data &sd,const char *npcname,int);
int npc_timer_event(const char *eventname);				// Added by RoVeRT
int npc_command(struct map_session_data &sd,const char *npcname, const char *command);
int npc_touch_areanpc(struct map_session_data &sd,unsigned short m,int x,int y);
int npc_click(struct map_session_data &sd,unsigned long npcid);
int npc_scriptcont(struct map_session_data &sd,unsigned long id);
bool npc_icNear(struct map_session_data &sd,unsigned long id);
int npc_buysellsel(struct map_session_data &sd,unsigned long id,int type);
int npc_buylist(struct map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_selllist(struct map_session_data &sd,unsigned short n,unsigned char *buffer);
int npc_parse_mob(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_parse_mob2(struct mob_list &mob);
bool npc_parse_warp(const char *w1,const char *w2,const char *w3,const char *w4);
int npc_globalmessage(const char *name,const char *mes);

int npc_enable(const char *name,int flag);
int npc_changename(const char *name, const char *newname, unsigned short look);
struct npc_data* npc_name2id(const char *name);

int npc_walktoxy(struct npc_data &nd,int x,int y,int easy); // npc walking [Valaris]
int npc_stop_walking(struct npc_data &nd,int type);
int npc_changestate(struct npc_data &nd,int state,int type);

int npc_get_new_npc_id(void);

void npc_addsrcfile(const char *);
void npc_delsrcfile(const char *);
void npc_printsrcfile();
void npc_parsesrcfile(const char *);
int do_final_npc(void);
int do_init_npc(void);
int npc_event_do_oninit(void);
int npc_do_ontimer(unsigned long npc_id, struct map_session_data &sd, int option);

int npc_event_doall(const char *name);
int npc_event_do(const char *name);
int npc_event_doall_id(const char *name, int id);

int npc_timerevent_start(struct npc_data &nd, unsigned long rid);
int npc_timerevent_stop(struct npc_data &nd);
int npc_gettimerevent_tick(struct npc_data &nd);
int npc_settimerevent_tick(struct npc_data &nd,int newtimer);
int npc_remove_map(struct npc_data *nd);
int npc_unload(struct npc_data *nd, bool erase_strdb=true);
int npc_reload(void);

// ============================================
// ADDITION Qamera death/disconnect/connect event mod
int npc_event_doall_attached_sub(void *key,void *data,va_list ap);
int npc_event_doall_attached(const char *name, struct map_session_data &sd);
struct npc_att_data {
	struct map_session_data * sd;
	char buf[64];
} ;
// END ADDITION
// ============================================ 

#endif

