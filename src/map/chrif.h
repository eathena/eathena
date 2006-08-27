// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHRIF_H_
#define _CHRIF_H_

#include "baseio.h"

void chrif_setuserid(const char* user);
void chrif_setpasswd(const char *pwd);
void chrif_setip(uint32 ip);
uint32 chrif_getip();
void chrif_setport(unsigned short port);

int chrif_isconnect(void);

//extern int chrif_connected;

int chrif_authreq(struct map_session_data &sd);
int chrif_save(struct map_session_data &sd);
int chrif_save_sc(struct map_session_data &sd);
int chrif_read_sc(struct map_session_data &sd);

int chrif_charselectreq(struct map_session_data &sd);

int chrif_changemapserver(struct map_session_data &sd, const char *name, unsigned short x, unsigned short y, basics::ipset& mapset);

int chrif_searchcharid(uint32 id);
int chrif_changegm(uint32 id,const char *pass,size_t len);
int chrif_changeemail(uint32 id, const char *actual_email, const char *new_email);
int chrif_char_ask_name(long id, const char *character_name, unsigned short operation_type, unsigned short year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second);
int chrif_saveaccountreg2(struct map_session_data &sd);
int chrif_reloadGMdb(void);
int chrif_ragsrvinfo(unsigned short base_rate, unsigned short job_rate, unsigned short drop_rate);
int chrif_char_offline(struct map_session_data &sd);
int chrif_char_reset_offline(void);
int chrif_char_online(struct map_session_data &sd);
int chrif_changesex(uint32 id, unsigned char sex);
int check_connect_char_server(int tid, unsigned long tick, int id, basics::numptr data);

int do_final_chrif(void);
int do_init_chrif(void);

int chrif_flush_fifo(void);

bool getAthentification(uint32 accid, CAuth& auth);



// fame stuff
const CFameList& chrif_getfamelist(fame_t type);
int chrif_updatefame(struct map_session_data &sd, fame_t type, int delta);
bool chrif_istop10fame(uint32 char_id, fame_t type);


// mail
void chrif_mail_cancel(struct map_session_data &sd);
bool chrif_mail_setitem(struct map_session_data &sd, ushort index, uint32 amount);
void chrif_mail_removeitem(struct map_session_data &sd, int flag);
bool chrif_mail_check(struct map_session_data &sd, bool showall=false);
bool chrif_mail_fetch(struct map_session_data &sd, bool unreadonly);
bool chrif_mail_read(struct map_session_data &sd, uint32 msgid);
bool chrif_mail_getappend(struct map_session_data &sd, uint32 msgid);
bool chrif_mail_delete(struct map_session_data &sd, uint32 msgid);
bool chrif_mail_send(struct map_session_data &sd, const char *target, const char *header, const char *body);

// irc
int chrif_irc_announce(const char* message, size_t sz);
int chrif_irc_announce_jobchange(struct map_session_data &sd);
int chrif_irc_announce_shop(struct map_session_data &sd, int flag);
int chrif_irc_announce_mvp(const map_session_data &sd, const mob_data &md);


// variables
int chrif_var_save(const char* name, const char* value);
int chrif_var_save(const char* name, uint32 value);


#endif
