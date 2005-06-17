// $Id: npc.h,v 1.5 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _NPC_H_
#define _NPC_H_

#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define INVISIBLE_CLASS 32767

#ifdef PCRE_SUPPORT
void npc_chat_finalize(struct npc_data *nd);
#endif
int npc_chat_sub(struct block_list *bl, va_list ap);
int npc_click(struct map_session_data *,int);
int npc_touch_areascript(struct map_session_data *,int,int,int);
int npc_touch_warp(struct map_session_data *,int,int,int);
int npc_scriptend(struct map_session_data *,int);
int npc_scriptnext(struct map_session_data *,int);
int npc_checknear(struct map_session_data *,int);
int npc_buysellsel(struct map_session_data *,int,int);
int npc_buylist(struct map_session_data *,int,unsigned short *);
int npc_selllist(struct map_session_data *,int,unsigned short *);
int npc_parse_mob(char *w1,char *w2,char *w3,char *w4);
int npc_parse_mob2 (struct mob_list *, int cached); // [Wizputer]
int npc_globalmessage(const char *name,char *mes);

int npc_enable(const char *name,int flag);
struct npc_data* npc_name2id(const char *name);

int npc_walktoxy(struct npc_data *nd,int x,int y,int easy); // npc walking [Valaris]
int npc_stop_walking(struct npc_data *nd,int type);
int npc_changestate(struct npc_data *nd,int state,int type);

int npc_get_new_npc_id(void);

void npc_addsrcfile(char *);
void npc_delsrcfile(char *);
void npc_parsesrcfile(char *);
int do_final_npc(void);
int do_init_npc(void);

int npc_add(char *,char *,short,short,short,short,short,char *);
int areascript_add (char *,short,short,short,short,short,char *);
int warp_add (char *,short,short,short,char *,short,short,short,short);
int npc_unload(struct npc_data *nd);
int warp_unload(struct warp_data *wd);
int areascript_unload(struct areascript_data *ad);
int npc_reload(void);

extern char *current_file;

#endif

