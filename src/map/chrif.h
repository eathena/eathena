// $Id: chrif.h,v 1.3 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _CHRIF_H_
#define _CHRIF_H_
#include "base.h"
#include "baseio.h"

void chrif_setuserid(const char* user);
void chrif_setpasswd(const char *pwd);
void chrif_setip(unsigned long ip);
unsigned long chrif_getip();
void chrif_setport(unsigned short port);

int chrif_isconnect(void);

//extern int chrif_connected;

int chrif_authreq(struct map_session_data &sd);
int chrif_save(struct map_session_data &sd);
int chrif_save_sc(struct map_session_data &sd);
int chrif_read_sc(struct map_session_data &sd);

int chrif_charselectreq(struct map_session_data &sd);

int chrif_changemapserver(struct map_session_data &sd, const char *name, unsigned short x, unsigned short y, ipset& mapset);

int chrif_searchcharid(unsigned long id);
int chrif_changegm(unsigned long id,const char *pass,size_t len);
int chrif_changeemail(unsigned long id, const char *actual_email, const char *new_email);
int chrif_char_ask_name(long id, const char *character_name, unsigned short operation_type, unsigned short year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second);
int chrif_saveaccountreg2(struct map_session_data &sd);
int chrif_reloadGMdb(void);
int chrif_reqfamelist(void);
int chrif_ragsrvinfo(unsigned short base_rate, unsigned short job_rate, unsigned short drop_rate);
int chrif_char_offline(struct map_session_data &sd);
int chrif_char_reset_offline(void);
int chrif_char_online(struct map_session_data &sd);
int chrif_changesex(unsigned long id, unsigned char sex);
int check_connect_char_server(int tid, unsigned long tick, int id, int data);

int do_final_chrif(void);
int do_init_chrif(void);

int chrif_flush_fifo(void);

bool getAthentification(unsigned long accid, CAuth& auth);

#endif
