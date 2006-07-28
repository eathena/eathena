// $Id: char.h,v 1.1.1.1 2004/09/10 17:26:50 MagicalTux Exp $
#ifndef _CHAR_H_
#define _CHAR_H_


#include "baseio.h"
#include "mmo.h"


#define MAX_MAP_SERVERS 30
#define CHAR_CONF_NAME	"conf/char_athena.conf"
#define DEFAULT_AUTOSAVE_INTERVAL 300*1000


///////////////////////////////////////////////////////////////////////////////
/// char session data.
/// used for client connections and holds a char-char-account
/// (which is the account data that references all characters of this account)
class char_session_data : public session_data, public CCharCharAccount
{
public:
	char_session_data()				{}
	virtual ~char_session_data()	{}

};






int mapif_sendall(unsigned char *buf, unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf, unsigned int len);
int mapif_send(int fd,unsigned char *buf, unsigned int len);

bool char_married(uint32 p1_id, uint32 p2_id);
bool char_married(const char* p1_name, const char* p2_name);
bool char_child(uint32 parent_id, uint32 child_id);
bool char_child(const char* parent_name, const char* child_name);
bool char_family(uint32 id1, uint32 id2, uint32 id3);
bool char_family(const char* name1, const char* name2, const char* name3);
bool char_exist(uint32 id);
bool char_exist(const char* name);

int char_log(char *fmt, ...);



extern char server_name[20];





#endif
