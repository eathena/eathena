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
extern bool add_to_unlimited_account;
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
/// login object for an account entry.
/// holds user data and account specific options
class login_account
{
public:
	uint32							account_id;		///< id of the account
	uchar							passwd[16];		///< password hash
	basics::string<>				userid;			///< username
	uint32							login_id1;		///< last id1
	uint32							login_id2;		///< last id2
	uint32							passfails;		///< # offailed password trials
	time_t							ban_until;		///< bantime if any
	time_t							valid_until;	///< validtime if any
	
public:
	/// default constructor.
	//## TODO remove when combining with construct support
	login_account()	{}

	// can use default copy/assign

	/// create constructor with explicit name.
	//## TODO remove when combining with construct support (iterator_find uses direct compare)
	explicit login_account(char* name) 
		: account_id(0), userid(name), 
		  login_id1(0), login_id2(0), passfails(0),
		  ban_until(0), valid_until(0)
	{}

	/// create from buffer constructor.
	/// constructs object from transfer buffer
	explicit login_account(const unsigned char* buf)
	{
		this->frombuffer(buf);
	}

	//@{
	/// compare operators. 
	/// only have the string types as right hand token
	bool operator==(const login_account&a) const { return this->userid==a.userid; }
	bool operator< (const login_account&a) const { return this->userid< a.userid; }

	bool operator==(const basics::string<>&a) const { return this->userid==a; }
	bool operator< (const basics::string<>&a) const { return this->userid< a; }
	bool operator==(const char*a) const				{ return this->userid==a; }
	bool operator< (const char*a) const				{ return this->userid< a; }
	//@}
	
	/// old style buffer converter. loads object from buffer
	size_t frombuffer(const unsigned char* buf)
	{
		if(buf)
		{
			_L_frombuffer(this->account_id, buf);
			this->userid.assign((char*)buf, 24); buf+=24;
			_X_frombuffer(this->passwd, buf, 16);
			_L_frombuffer(this->login_id1, buf);
			_L_frombuffer(this->login_id2, buf);
			_T_frombuffer(this->ban_until, buf);
			_T_frombuffer(this->valid_until, buf);
			return 4+24+16+4+4+4+4;
		}
		return 0;
	}
	/// old style buffer converter. stores object to buffer
	size_t tobuffer(unsigned char* buf) const
	{
		if(buf)
		{
			_L_tobuffer(this->account_id, buf);
			_S_tobuffer(this->userid, buf, 24);
			_X_tobuffer(this->passwd, buf, 16);
			_L_tobuffer(this->login_id1, buf);
			_L_tobuffer(this->login_id2, buf);
			_T_tobuffer(this->ban_until, buf);
			_T_tobuffer(this->valid_until, buf);
			return 4+24+16+4+4+4+4;
		}
		return 0;
	}
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
/// mode needs to be specified
struct charserver_connect : public server_connect
{
	basics::string<>				name;
	unsigned int					mode;
	uint32							cryptkey;
	basics::slist<login_account>	accounts;

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
struct login_server
{
	int									fd;				///< socket connect, derive when possible
	basics::CParam<ushort>				port;			///< login server listen port
	basics::vector<charserver_connect>	char_servers;	///< char server connections
	basics::slist<client_connect>		last_connects;	///< list of last server connections

	basics::CParam<bool>				allowed_regs;	///< M/F registration allowed
	ulong								new_reg_tick;	///< internal tickcounter for M/F registration
	basics::CParam<ulong>				time_allowed;	///< time in seconds between registrations

	login_server() :
		fd(-1),
		port("loginport", 6900),
		allowed_regs("allowed_regs", false),
		new_reg_tick(gettick()),
		time_allowed("time_allowed", 10)
		{}

};

///////////////////////////////////////////////////////////////////////////////
class transmitt_packet
{
public:
	virtual size_t size() const=0;
	virtual operator const unsigned char*()=0;

	void send()	{}
};

///////////////////////////////////////////////////////////////////////////////
class packet_login_ok : public transmitt_packet
{
	enum { packet_header = 0xFF00, packet_size = 4+3*sizeof(uint32) };

	unsigned char buffer[packet_size];
public:
	explicit packet_login_ok(const login_account& acc)
	{
		unsigned char *buf = this->buffer;
		_W_tobuffer((ushort)packet_header, buf);
		_W_tobuffer((ushort)packet_size, buf);
		_L_tobuffer(acc.account_id, buf);
		_L_tobuffer(acc.login_id1, buf);
		_L_tobuffer(acc.login_id2, buf);
	}
	virtual size_t size() const	{ return packet_size; }
	virtual operator const unsigned char*()	{ return buffer; }
};

///////////////////////////////////////////////////////////////////////////////
class packet_add_account : public transmitt_packet
{
	enum { packet_header = 0xFF01, packet_size = 4+2*24+3*sizeof(uint32)+2*sizeof(uint32) };

	unsigned char buffer[packet_size];
public:
	explicit packet_add_account(const login_account& acc)
	{
		unsigned char *buf = this->buffer;
		_W_tobuffer((ushort)packet_header, buf);
		_W_tobuffer((ushort)packet_size, buf);

		_L_tobuffer(acc.account_id, buf);
		_S_tobuffer(acc.userid, buf, 24);
		_X_tobuffer(acc.passwd, buf, 16);
		_L_tobuffer(acc.login_id1, buf);
		_L_tobuffer(acc.login_id2, buf);
		_T_tobuffer(acc.ban_until, buf);
		_T_tobuffer(acc.valid_until, buf);
	}
	virtual size_t size() const	{ return packet_size; }
	virtual operator const unsigned char*()	{ return buffer; }
};

///////////////////////////////////////////////////////////////////////////////
class packet_del_account : public transmitt_packet
{
	enum { packet_header = 0xFF02, packet_size = 4+sizeof(uint32) };

	unsigned char buffer[packet_size];
public:
	explicit packet_del_account(const login_account& acc)
	{
		unsigned char *buf = this->buffer;
		_W_tobuffer((ushort)packet_header, buf);
		_W_tobuffer((ushort)packet_size, buf);
		_L_tobuffer(acc.account_id, buf);
	}
	virtual size_t size() const	{ return packet_size; }
	virtual operator const unsigned char*()	{ return buffer; }
};

//////////////////////////////////////////////
#endif
