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
	uchar							md5keylen;		///< keylen
	uchar							md5key[23];		///< key data
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

///////////////////////////////////////////////////////////////////////////////
/// base class for a server connection.
/// add more generic stuff in here when going global with that
struct server_connect
{
	int fd;
	basics::ipset address;
	size_t users;
};

///////////////////////////////////////////////////////////////////////////////
/// login-char connection, login side.
struct charserver_connect : public server_connect
{
	basics::string<>				name;
	bool							allow_register;

	bool operator==(const charserver_connect&a) const { return this->name==a.name; }
	bool operator< (const charserver_connect&a) const { return this->name< a.name; }
};

///////////////////////////////////////////////////////////////////////////////
/// base class for a client connection.
/// add more generic stuff in here when going global with that
struct client_connect
{
	basics::ipaddress	addr;	///< socket connect, derive when possible
	ulong				tick;	///< tick of

	bool operator==(const client_connect&a) const { return this->addr==a.addr; }
	bool operator< (const client_connect&a) const { return this->addr< a.addr; }
};


///////////////////////////////////////////////////////////////////////////////
/// data of a login server.
struct login_server : public session_data
{
	socket_data&						socket;			///< where this session is assigned to
	basics::CParam<ushort>				port;			///< login server listen port
	basics::vector<charserver_connect>	char_servers;	///< char server connections
	basics::slist<client_connect>		last_connects;	///< list of last connections

	basics::CParam<bool>				mfreg_enabled;	///< M/F registration allowed
	basics::CParam<ulong>				mfreg_time;		///< time in seconds between registrations
	ulong								new_reg_tick;	///< internal tickcounter for M/F registration
	

	basics::smap< basics::string<>, basics::string<> > server_accounts;

	login_server(socket_data& s) :
		socket(s),
		port("loginport", 6900),
		mfreg_enabled("mfreg_enabled", false),
		mfreg_time("mfreg_time", 10),
		new_reg_tick(gettick())
	{}

	~login_server()
	{	// unregister
		this->socket.user_session = NULL;
	}
};

///////////////////////////////////////////////////////////////////////////////
class transmitt_packet
{
public:
	virtual ~transmitt_packet()	{}
	virtual size_t size() const=0;
	virtual operator const unsigned char*()=0;

	void send()	{}
};

///////////////////////////////////////////////////////////////////////////////
class packet_try_login : public transmitt_packet
{
	enum { packet_header = 0xFF00, packet_size = 4+3*24+1+2*4 };

	unsigned char buffer[packet_size];
public:
	explicit packet_try_login(const login_session& acc)
	{
		unsigned char *buf = this->buffer;
		_W_tobuffer((ushort)packet_header, buf);
		_W_tobuffer((ushort)packet_size, buf);
		_S_tobuffer(acc.userid, buf, 24);
		_X_tobuffer(acc.passwd, buf, 24);
		_B_tobuffer(acc.md5keylen, buf);
		_X_tobuffer(acc.md5key, buf, 24);
		_L_tobuffer(acc.login_id1, buf);
		_L_tobuffer(acc.login_id2, buf);
	}
	virtual ~packet_try_login()	{}
	virtual size_t size() const	{ return packet_size; }
	virtual const unsigned char* operator()() const	{ return buffer; }
	virtual unsigned char* operator()() { return buffer; }
};

///////////////////////////////////////////////////////////////////////////////
class packet_new_login : public transmitt_packet
{
	enum { packet_header = 0xFF01, packet_size = 4+3*24+1+2*4 };

	unsigned char buffer[packet_size];
public:
	explicit packet_new_login(const login_session& acc)
	{
		unsigned char *buf = this->buffer;
		_W_tobuffer((ushort)packet_header, buf);
		_W_tobuffer((ushort)packet_size, buf);
		_S_tobuffer(acc.userid, buf, 24);
		_X_tobuffer(acc.passwd, buf, 24);
		_B_tobuffer(acc.md5keylen, buf);
		_X_tobuffer(acc.md5key, buf, 24);
		_L_tobuffer(acc.login_id1, buf);
		_L_tobuffer(acc.login_id2, buf);
	}
	virtual ~packet_new_login()	{}
	virtual size_t size() const	{ return packet_size; }
	virtual const unsigned char* operator()() const	{ return buffer; }
	virtual unsigned char* operator()() { return buffer; }
};


//////////////////////////////////////////////
#endif
