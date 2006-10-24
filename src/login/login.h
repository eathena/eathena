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
                     	// It is 1 at the time of passwordencrypt.
                     	// It is made into 2 at the time of passwordencrypt2.
                     	// When it is made 3, it corresponds to both.

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
		this->md5key[this->md5keylen-1]=0;
	}
	virtual ~login_session_data()	{}
};






//////////////////////////////////////////////
/*
	stuff for universal login

	login holds char server sessions with
		ipset
		user count
		name
		flags
		list of accounts (on this specific char server)
		cryptkey

	account holds
		account_id
		userid
		passwd
		last_login
		last_ip
		login_count
		login_id1
		login_id2
		ban_until
		valid_until

	the protocol between login and char then could be reduced to
		"login/re-login+ack"	login<- char 
		"add account(s)"		login<->char (reverse when M/F creation is active)
		"update account(s)"		login<->char (reverse for login_count/lastip)
		"delete account(s)"		login<- char
*/



///////////////////////////////////////////////////////////////////////////////
class login_account
{
public:
	uint32							account_id;	// id of the account
	basics::string<>				userid;		// username
	basics::string<>				passwd;		// password hash
	uint32							login_id1;	// last id1
	uint32							login_id2;	// last id2
	time_t							ban_until;	// bantime if any
	time_t							valid_until;// validtime if any
public:
	login_account()	{}
	explicit login_account(const unsigned char* buf)
	{
		this->frombuffer(buf);
	}
	explicit login_account(char* name, char* pass) 
		: account_id(0), 
		  userid(name), passwd(pass), 
		  login_id1(0), login_id2(0), 
		  ban_until(0), valid_until(0)
	{}
	bool operator==(const login_account&a) const { return this->userid==a.userid; }
	bool operator< (const login_account&a) const { return this->userid< a.userid; }
	
	size_t frombuffer(const unsigned char* buf)
	{
		if(buf)
		{
			_L_frombuffer(this->account_id, buf);
			this->userid.assign((char*)buf, 24); buf+=24;
			this->passwd.assign((char*)buf, 24); buf+=24;
			_L_frombuffer(this->login_id1, buf);
			_L_frombuffer(this->login_id2, buf);
			_T_frombuffer(this->ban_until, buf);
			_T_frombuffer(this->valid_until, buf);
		}
	}
	size_t tobuffer(unsigned char* buf) const
	{
		if(buf)
		{
			_L_tobuffer(this->account_id, buf);
			_S_tobuffer(this->userid, buf, 24);
			_S_tobuffer(this->passwd, buf, 24);
			_L_tobuffer(this->login_id1, buf);
			_L_tobuffer(this->login_id2, buf);
			_T_tobuffer(this->ban_until, buf);
			_T_tobuffer(this->valid_until, buf);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
class server_connect : public basics::noncopyable
{
	int fd;
	basics::ipset address;
	size_t users;
};

///////////////////////////////////////////////////////////////////////////////
class charserver_connect : public server_connect
{
	basics::string<>				name;
	unsigned short					maintenance;
	unsigned short					new_display;
	uint32							cryptkey;
	basics::slist<login_account>	accounts;
public:

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
		_S_tobuffer(acc.passwd, buf, 24);
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
