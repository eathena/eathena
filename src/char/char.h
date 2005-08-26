// $Id: char.h,v 1.1.1.1 2004/09/10 17:26:50 MagicalTux Exp $
#ifndef _CHAR_H_
#define _CHAR_H_

#define MAX_MAP_SERVERS 30

#define CHAR_CONF_NAME	"conf/char_athena.conf"

#define LOGIN_LAN_CONF_NAME	"conf/lan_support.conf"

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000



int mapif_sendall(unsigned char *buf, unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf, unsigned int len);
int mapif_send(int fd,unsigned char *buf, unsigned int len);

bool char_married(unsigned long p1_id, unsigned long p2_id);
bool char_married(const char* p1_name, const char* p2_name);
bool char_child(unsigned long parent_id, unsigned long child_id);
bool char_child(const char* parent_name, const char* child_name);
bool char_family(unsigned long id1, unsigned long id2, unsigned long id3);
bool char_family(const char* name1, const char* name2, const char* name3);
bool char_exist(unsigned long id);
bool char_exist(const char* name);

int char_log(char *fmt, ...);

//extern int autosave_interval;
extern char db_path[];

extern int party_modus;

#endif
