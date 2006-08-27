// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_H_
#define _LOGIN_H_

#define MAX_SERVERS 30
#define LOGIN_CONF_NAME "conf/login_athena.conf"
#define PASSWORDENC 3	// A definition is given when making an encryption password correspond.
                     	// It is 1 at the time of passwordencrypt.
                     	// It is made into 2 at the time of passwordencrypt2.
                     	// When it is made 3, it corresponds to both.

#define START_ACCOUNT_NUM 2000000
#define END_ACCOUNT_NUM 100000000


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
//////////////////////////////////////////////
#endif
