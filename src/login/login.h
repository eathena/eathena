// $Id: login.h,v 1.1.1.1 2004/09/10 17:26:53 MagicalTux Exp $
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
		"add account(s)"		login<->char
		"update account(s)"		login<->char
		"delete account(s)"		login<- char

*/
//////////////////////////////////////////////
#endif
