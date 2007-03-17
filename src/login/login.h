// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#include "basestring.h"
#include "baseio.h"
#include "core.h"
#include "socket.h"
#include "timer.h"
#include "mmo.h"
#include "version.h"
#include "db.h"
#include "md5calc.h"
#include "lock.h"
#include "malloc.h"
#include "utils.h"
#include "showmsg.h"


#include "md5calc.h"


#define MAX_SERVERS 30
#define LOGIN_CONF_NAME "conf/login_athena.conf"
#define PASSWORDENC 3	// A definition is given when making an encryption password correspond.
                     	// 1 for passwordencrypt.
                     	// 2 for passwordencrypt2.
                     	// 3 for both.

#define START_ACCOUNT_NUM 2000000
#define END_ACCOUNT_NUM 100000000


extern char date_format[32];
extern struct mmo_char_server server[MAX_SERVERS];
extern unsigned short server_num;
extern CAccountDB account_db;


int login_log(char *fmt, ...);
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len);
int save_packet(int fd, const char* str, const char* ip_str);



///////////////////////////////////////////////////////////////////////////////
/// login session data.
/// only used for holding a md5key
class login_session_data : public session_data
{
public:
	uchar md5keylen;
	char md5key[23];

	login_session_data() : md5keylen( rand() % 4 + 12 )
	{	// create a coding key, 
		// all chars exept zero allowed
		size_t i;
		for(i=0; i<this->md5keylen; ++i)
			this->md5key[i] = rand() % 255 + 1;
		// terminate with zero
		this->md5key[this->md5keylen]=0;
	}
	virtual ~login_session_data()	{}
};








///////////////////////////////////////////////////////////////////////////////
/// login object
class login_session : public session_data
{
public:
	socket_data&					socket;			///< where this session is assigned to
	basics::string<>				userid;			///< username
	uchar							passwd[24];		///< password
	uchar							md5key[23];		///< key data
	uchar							md5keylen;		///< keylen
	uint32							login_id1;		///< last id1
	uint32							login_id2;		///< last id2

	/// default constructor.
	login_session(socket_data& s)
		: socket(s), md5keylen(0)
	{	// register
		this->socket.user_session = this;
		// init values
		*this->passwd=0;
		*this->md5key=0;
		this->login_id1 = rand()<<16 | rand();
		this->login_id2 = rand()<<16 | rand();
	}
	~login_session()
	{	// unregister
		this->socket.user_session = NULL;
	}

	/// create a coding key.
	void create_key()
 	{	// all chars exept zero allowed
		this->md5keylen = rand()%4 + 12; // 4..16
		size_t i;
		for(i=0; i<this->md5keylen; ++i)
			this->md5key[i] = rand() % 255 + 1;
		// terminate with zero
		this->md5key[this->md5keylen]=0;
	}

	/// test if key exists.
	bool is_encryped() const { return md5keylen!=0; }
};


struct server_auth
{
	basics::smap< basics::string<>,basics::string<> > passmap;

	server_auth()
	{}

	void add(const char* str)
	{
		while(str && *str)
		{
			const char* kp = strchr(str,',');
			basics::string<> user = (kp)?basics::string<>(str, kp-str):basics::string<>(str);
			str=(kp)?kp+1:kp;

			const char* ip = strchr(user.c_str(),':');
			if(!ip)
			{
				ShowError("server_auth: invalid specification, should be <user>:<pass>\n");
				continue;
			}

			basics::string<> pass(ip+1);
			user.truncate(ip-user.c_str());
			user.trim();
			pass.trim();
			if( user.size() == 0 )
			{
				ShowError("server_auth: empty user ignored\n");
			}
			else if( pass.size()==0 )
			{
				ShowError("server_auth: empty password ignored\n");
			}
			else
			{
				if( passmap.exists(user) )
				{
					ShowWarning("server_auth: replacing password for user '%s'\n", user.c_str());
				}
				this->passmap[user] = pass;
			}
		}
	}
	void add(const basics::string<> str)
	{
		this->add((const char*)str);
	}

	bool exists(const char* user, const char* pass) const
	{
		const basics::string<>* entry = this->passmap.search(user);
		return ( entry && *entry==pass );
	}

	///////////////////////////////////////////////////////////////////////////
	/// assign string.
	const server_auth& operator=(const char* str)				{ this->add(str); return *this; }
	const server_auth& operator=(const basics::string<>& str)	{ this->add(str); return *this; }

	///////////////////////////////////////////////////////////////////////////
	/// explicit compare operator.
	/// retuns always false for usage in parameters
	friend bool operator==(const server_auth& a, const server_auth& b)	{ return false; } 
};




//////////////////////////////////////////////
#endif
