
// $Id: login.c,v 1.1.1.1 2004/09/10 17:26:53 MagicalTux Exp $
// new version of the login-server by [Yor]

#include "baseparam.h"
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

#include "login.h"


///////////////////////////////////////////////////////////////////////////////
// server address
basics::netaddress loginaddress((uint32)INADDR_ANY, 6900);
int login_fd;


///////////////////////////////////////////////////////////////////////////////
// char connections
struct mmo_char_server server[MAX_SERVERS];
unsigned short server_num;


///////////////////////////////////////////////////////////////////////////////
// the account data base
CAccountDB account_db;


///////////////////////////////////////////////////////////////////////////////
// account registration
bool new_account_flag = false;		///< creation of new accounts allowed
bool allowed_regs = false;			///< M/F registration allowed
unsigned long new_reg_tick=0;		///< internal tickcounter for M/F registration
unsigned long time_allowed=10;		///< time in seconds between registrations (10)




///////////////////////////////////////////////////////////////////////////////
// access stuff 
//!! todo move to socket
enum
{
	ACO_DENY_ALLOW = 0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
	ACO_STRSIZE = 128,
};

int access_order = ACO_DENY_ALLOW;
int access_allownum = 0;
int access_denynum = 0;

char *access_allow = NULL;
char *access_deny = NULL;

int access_ladmin_allownum = 0;
char *access_ladmin_allow = NULL;

int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 1;


///////////////////////////////////////////////////////////////////////////////
// connection stuff
int min_level_to_connect = 0;		// minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int add_to_unlimited_account = 0;	// Give possibility or not to adjust (ladmin command: timeadd) the time of an unlimited account.
int start_limited_time = -1;		// Starting additional sec from now for the limited time at creation of accounts (-1: unlimited time, 0 or more: additional sec from now)
int check_ip_flag = 1;				// It's to check IP of a player between login-server and char-server (part of anti-hacking system)
int use_md5_passwds = 0;

///////////////////////////////////////////////////////////////////////////////
// enable console
int console = 0;

///////////////////////////////////////////////////////////////////////////////
// admin/gm stuff
int admin_state = 0;
char admin_pass[24] = "";
char gm_pass[64] = "";
int level_new_gm = 60;


///////////////////////////////////////////////////////////////////////////////
// logging (txt)
//!! todo compound to class with sql logging
FILE *log_fp = NULL;
int log_login = 1;
char login_log_filename[1024] = "log/login.log";
char date_format[32] = "%Y-%m-%d %H:%M:%S";

int display_parse_login = 0; // 0: no, 1: yes
int display_parse_admin = 0; // 0: no, 1: yes
int display_parse_fromchar = 0; // 0: no, 1: yes (without packet 0x2714), 2: all packets

//------------------------------
// Writing function of logs file
//------------------------------
int login_log(char *fmt, ...)
{
	if (log_login)
	{		
		time_t unixtime;
		struct timeval tv;
		char tmpstr[2048];

		if(!log_fp)
			log_fp = basics::safefopen(login_log_filename, "a");

		if (log_fp) {
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(log_fp, RETCODE);
			else {
				
				gettimeofday(&tv, NULL);
				unixtime = tv.tv_sec;
				strftime(tmpstr, 24, date_format, localtime(&unixtime));
				sprintf(tmpstr + strlen(tmpstr), ".%03ld: %s", tv.tv_usec / 1000, fmt);

				va_list ap;
				va_start(ap, fmt);
				vfprintf(log_fp, tmpstr, ap);
				va_end(ap);
			}
			fflush(log_fp); // under cygwin or windows, if software is stopped, data are not written in the file -> fflush at every line
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// unknown packets
int save_unknown_packets = 0;
char login_log_unknown_packets_filename[1024] = "log/login_unknown_packets.log";

int save_packet(int fd, const char* str, const char* ip_str)
{
	if(save_unknown_packets)
	{	size_t i,j;
		char tmpstr[24];
		struct timeval tv;
		time_t unixtime;
		FILE *logfp = basics::safefopen(login_log_unknown_packets_filename, "a");
		if (logfp)
		{
			gettimeofday(&tv, NULL);
			unixtime = tv.tv_sec;
			strftime(tmpstr, 23, date_format, localtime(&unixtime));
			fprintf(logfp, "%s.%03ld: receiving of an unknown packet -> disconnection" RETCODE, tmpstr, tv.tv_usec / 1000);
			fprintf(logfp, "%s: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, str, fd, ip_str, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
			fprintf(logfp, "Detail (in hex):" RETCODE);
			fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
			memset(tmpstr, '\0', sizeof(tmpstr));
			for(i = 0; i < (size_t)RFIFOREST(fd); ++i)
			{
				if ((i & 15) == 0)
					fprintf(logfp, "%04X ", (int)i);
				fprintf(logfp, "%02x ", (unsigned char)RFIFOB(fd,i));
				if (RFIFOB(fd,i) > 0x1f)
					tmpstr[i % 16] = RFIFOB(fd,i);
				else
					tmpstr[i % 16] = '.';
				if ((i - 7) % 16 == 0) // -8 + 1
					fprintf(logfp, " ");
				else if ((i + 1) % 16 == 0)
				{
					fprintf(logfp, " %s" RETCODE, tmpstr);
					memset(tmpstr, '\0', sizeof(tmpstr));
				}
			}
			if (i % 16 != 0)
			{
				for(j = i; j % 16 != 0; ++j)
				{
					fprintf(logfp, "   ");
					if ((j - 7) % 16 == 0) // -8 + 1
						fprintf(logfp, " ");
				}
				fprintf(logfp, " %s" RETCODE, tmpstr);
			}
			fprintf(logfp, RETCODE);
			fclose(logfp);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// encrypted/unencrypted password check
bool check_password(login_session_data* ld, int passwdenc, const char* passwd, const char* refpass)
{	
	if(passwdenc == 0)
	{
		return (0==strcmp(passwd, refpass));
	}
	else if (ld)
	{
		char md5str[64], md5bin[32];
		bool encpasswdok = false;
		if(passwdenc == 2)
			snprintf(md5str, sizeof(md5str), "%s%s", refpass, ld->md5key); // 24 + 20
		else // otherwise take passwdenc == 1 as defaule
			snprintf(md5str, sizeof(md5str), "%s%s", ld->md5key, refpass); // 20 + 24

		md5str[sizeof(md5str)-1] = '\0';
		MD5_String2binary(md5str, md5bin);
		encpasswdok = (memcmp(passwd, md5bin, 16) == 0);
		return encpasswdok;
	}
	return false;
}

const char* timestamp2string(char* str, size_t sz)
{
	if(str && sz)
	{
		size_t csz;
		timeval tv;
		time_t unixtime;
		gettimeofday(&tv, NULL);										// get time
		unixtime = (time_t)tv.tv_sec;									// pick out seconds from unix epoch 
		csz=strftime(str, sz, "%Y-%m-%d %H:%M:%S", localtime(&unixtime));// print string with given dateformat
		snprintf(str+csz, sz-csz, ".%03lu", tv.tv_usec / 1000);			// add milliseconds
		str[sz-1] = 0;	// system snprintf does not add EOS when buffer limit is reached
	}
	return str;
}


//--------------------------------------------------------------
// Test of the IP mask
// (ip: IP to be tested, str: mask x.x.x.x/# or x.x.x.x/y.y.y.y)
//--------------------------------------------------------------
bool check_ipmask(uint32 ip, const unsigned char *str)
{
	unsigned int mask = 0, i = 0, m, ip2, a0, a1, a2, a3;
	unsigned char *p = (unsigned char *)&ip2, *p2 = (unsigned char *)&mask;

	if (sscanf((char *)str, "%d.%d.%d.%d/%n", &a0, &a1, &a2, &a3, &i) != 4 || i == 0)
		return false;
	p[0] = a0; p[1] = a1; p[2] = a2; p[3] = a3;

	if (sscanf((char *)(str+i), "%d.%d.%d.%d", &a0, &a1, &a2, &a3) == 4) {
		p2[0] = a0; p2[1] = a1; p2[2] = a2; p2[3] = a3;
		mask = ntohl(mask);
	} else if (sscanf((char *)(str+i), "%d", &m) == 1 && m >= 0 && m <= 32) {
		for(i = 0; i < m && i < 32; ++i)
			mask = (mask >> 1) | 0x80000000;
	} else {
		ShowWarning("check_ipmask: invalid mask [%s].\n", str);
		return false;
	}
	return ((ntohl(ip) & mask) == (ntohl(ip2) & mask));
}

//---------------------
// Access control by IP
//---------------------
bool check_ip(uint32 ip)
{
	int i;
	char buf[16];
	char * access_ip;
	enum { ACF_DEF, ACF_ALLOW, ACF_DENY } flag = ACF_DEF;
	
	if (access_allownum == 0 && access_denynum == 0)
		return 1; // When there is no restriction, all IP are authorised.


//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	((basics::ipaddress)ip).tostring(buf,sizeof(buf));

	for(i = 0; i < access_allownum; ++i) {
		access_ip = access_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			if(access_order == ACO_ALLOW_DENY)
				return true; // With 'allow, deny' (deny if not allow), allow has priority
			flag = ACF_ALLOW;
			break;
		}
	}

	for(i = 0; i < access_denynum; ++i) {
		access_ip = access_deny + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			//flag = ACF_DENY; // not necessary to define flag
			return false; // At this point, if it's 'deny', we refuse connection.
		}
	}

	return (flag == ACF_ALLOW || access_order == ACO_DENY_ALLOW);
		// With 'mutual-failture', only 'allow' and non 'deny' IP are authorised.
		//   A non 'allow' (even non 'deny') IP is not authorised. It's like: if allowed and not denied, it's authorised.
		//   So, it's disapproval if you have no description at the time of 'mutual-failture'.
		// With 'deny,allow' (allow if not deny), because here it's not deny, we authorise.
}

//--------------------------------
// Access control by IP for ladmin
//--------------------------------
bool check_ladminip(uint32 ip)
{
	int i;
	char buf[16];
	char * access_ip;

	if (access_ladmin_allownum == 0)
		return true; // When there is no restriction, all IP are authorised.

//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	((basics::ipaddress)ip).tostring(buf,sizeof(buf));

	for(i = 0; i < access_ladmin_allownum; ++i)
	{
		access_ip = access_ladmin_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip))
			return true;
	}
	return false;
}


//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len)
{
	size_t i, c;
	for(i=0, c=0; i<MAX_SERVERS; ++i)
	{
		if( server[i].fd != sfd && session_isActive(server[i].fd) ) 
		{
			memcpy(WFIFOP(server[i].fd,0), buf, len);
			WFIFOSET(server[i].fd, len);
			c++;
		}
	}
	return c;
}





//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd)
{
	size_t id;
	char ip_str[16]="unknown";
	if(session[fd]) session[fd]->client_ip.tostring(ip_str, sizeof(ip_str));


	for(id = 0; id < MAX_SERVERS; ++id)
		if (server[id].fd == fd)
			break;
	if (id == MAX_SERVERS)
	{	// not a char server
		session_Remove(fd);
		return 0;
	}
	// else it is char server id
	if( !session_isActive(fd) )
	{	// check if a char server is disconnecting
		ShowWarning("Char-server '%s' has disconnected.\n", server[id].name);
		login_log("Char-server '%s' has disconnected (ip: %s)." RETCODE,server[id].name, ip_str);
		server[id].fd = -1;
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}


	while (RFIFOREST(fd) >= 2)
	{
		unsigned short command = RFIFOW(fd,0);

		if( display_parse_fromchar == 2 || (display_parse_fromchar == 1 && command != 0x2714)) // 0x2714 is done very often (number of players)
			ShowMessage("parse_fromchar: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, command, RFIFOREST(fd));
		switch(command)
		{
		///////////////////////////////////////////////////////////////////////
		// request from map-server via char-server to reload GM accounts (by Yor).
//obsolete
		case 0x2709:
		{
			login_log("Char-server '%s': Request to re-load GM configuration file (ip: %s)." RETCODE, server[id].name, ip_str);
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// char server reconnection, update the ip's
		case 0x2710:
		{
			if (RFIFOREST(fd) < 90)
					return 0;

			char* server_name=(char*)RFIFOP(fd,50);
			server_name[19] = '\0';
			remove_control_chars(server_name);

			memcpy(server[id].name,server_name,20);
			server[id].maintenance=RFIFOB(fd,70);
			server[id].new_display=RFIFOB(fd,71);
			// wanip,wanport,lanip,lanmask,lanport
			server[id].address = basics::ipset(	RFIFOLIP(fd,74), RFIFOLIP(fd,78), RFIFOW(fd,82), RFIFOLIP(fd,84), RFIFOW(fd,88) );

			
			login_log("Connection update of the char-server '%s' @ %s" RETCODE,
				      server_name, server[id].address.tostring(NULL) );
			ShowStatus("Connection update of the char-server '%s' @ %s\n",
				      server_name, server[id].address.tostring(NULL) );

			RFIFOSKIP(fd,90);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// request from char-server to authentify an account
		case 0x2712: 
		{
			if (RFIFOREST(fd) < 19)
				return 0;

			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;

			if( account_db.searchAccount(accid, account) &&
				// maybe remove this checks too, char server should do the auth checking by its own
				account.account_id == accid &&
				account.login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				account.login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18
#endif
				account.sex == RFIFOB(fd,14) &&
				(!check_ip_flag || account.client_ip == RFIFOLIP(fd,15)) )
			{	// found and verified
				login_log("Char-server '%s': authentification of the account %d accepted (ip: %s)." RETCODE, server[id].name, accid, ip_str);

				// send authentification to char
				WFIFOW(fd,0) = 0x2750;
				account.CCharAccount::tobuffer( WFIFOP(fd, 2) );
				WFIFOSET(fd, 2+account.CCharAccount::size() );

//obsolete
				WFIFOW(fd,0) = 0x2729;	// Sending of the account_reg2
				WFIFOL(fd,4) = accid;

				size_t p,j;
				for(p = 8, j = 0; j < account.account_reg2_num; p += 36, ++j) {
					memcpy(WFIFOP(fd,p), account.account_reg2[j].str, 32);
					WFIFOL(fd,p+32) = account.account_reg2[j].value;
				}
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,p);

				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = accid;
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), account.email, 40);
				WFIFOL(fd,47) = (uint32)account.valid_until;
				WFIFOSET(fd,51);
/////
			}
			else
			{	// authentification not found
				login_log("Char-server '%s': authentification of the account %d REFUSED (ip: %s)." RETCODE, server[id].name, accid, ip_str);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = accid;
				WFIFOB(fd,6) = 1;
				// It is unnecessary to send email
				// It is unnecessary to send validity date of the account
				WFIFOSET(fd,51);
			}
			RFIFOSKIP(fd,19);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// incoming account data from char with changes
		case 0x2750:
		{
			CLoginAccount account;
			CCharAccount  characcount;

			if( (size_t)RFIFOREST(fd) < 2+account.CCharAccount::size() )
				return 0;
			// read data from buffer
			characcount.CCharAccount::frombuffer( RFIFOP(fd,2) );
			// fetch the loginaccount data
			if( account_db.searchAccount(characcount.account_id, account) )
			{	// store the resceived data
				account = characcount;
				// Save
				account_db.saveAccount(account);

				// send to other charservers
				charif_sendallwos(fd, RFIFOP(fd,0), 2+account.CCharAccount::size() );
			}
			RFIFOSKIP(fd,2+account.CCharAccount::size() );
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Receiving of the users number
		case 0x2714:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			//ShowMessage("parse_fromchar: Receiving of the users number of the server '%s': %ld\n", server[id].name, (unsigned long)RFIFOL(fd,2));
			server[id].users = RFIFOL(fd,2);

			// send some answer
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);

			RFIFOSKIP(fd,6);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// login-server alive packet reply
		case 0x2718:
		{
			if (RFIFOREST(fd) < 2)
				return 0;
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// request to become GM
// obsolete, use sending the account data with 2750 instead
		case 0x2720:	
		{
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < (int)RFIFOW(fd,2))
				return 0;

			unsigned char buf[10];
			uint32 accid = RFIFOL(fd,4);
			CLoginAccount account;
			//ShowMessage("parse_fromchar: Request to become a GM acount from %d account.\n", accid);
			
			if( account_db.searchAccount(accid, account) )
			{
				if( 0!=strcmp((char*)RFIFOP(fd,8), gm_pass) )
				{	// password correct
					ShowError("Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0))\n", accid);
					login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0), ip: %s)." RETCODE,
							  server[id].name, accid, ip_str);
				}
				else if( account.gm_level > 0)
				{	// only non-GM can become GM
					ShowError("Error of GM change (suggested account: %d, invalid password).\n", accid);
					login_log("Char-server '%s': Error of GM change (suggested account: %d, invalid password, ip: %s)." RETCODE,
							  server[id].name, accid, ip_str);
				}
				else if( level_new_gm == 0 )
				{	// gm creation is enabled
					ShowError("Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0))\n", accid);
					login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0), ip: %s)." RETCODE,
							  server[id].name, accid, ip_str);

				}
				else
				{	// everything ok
					account.gm_level = level_new_gm;
					account_db.saveAccount(account);

					WBUFW(buf,0) = 0x2721;
					WBUFL(buf,2) = accid;
					WBUFL(buf,6) = level_new_gm;
					charif_sendallwos(-1, buf, 10);
					
					ShowMessage("GM Change of the account %d: level 0 -> %d.\n", accid, level_new_gm);
					login_log("Char-server '%s': GM Change of the account %d: level 0 -> %d (ip: %s)." RETCODE,
							  server[id].name, accid, level_new_gm, ip_str);
				}
			}
			RFIFOSKIP(fd, RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Map server send information to change an email of an account via char-server
// obsolete, use sending the account data with 2750 instead
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
		{
			if (RFIFOREST(fd) < 86)
				return 0;

			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;
			
			char* cur_email = (char*)RFIFOP(fd, 6);
			char* new_email = (char*)RFIFOP(fd,46);

			cur_email[39] = '\0';
			new_email[39] = '\0';
			remove_control_chars(cur_email);
			remove_control_chars(new_email);
			
			if( !email_check(cur_email) )
			{
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
						  server[id].name, accid, ip_str);
			}
			else if( !email_check(new_email) )
			{
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
						  server[id].name, accid, ip_str);
			}
			else if (strcasecmp(new_email, "a@a.com") == 0)
			{
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
						  server[id].name, accid, ip_str);
			}
			else if( !account_db.searchAccount(accid, account) )
			{
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s)." RETCODE,
					      server[id].name, accid, ip_str);
			}
			else if( 0!=strcasecmp(account.email, cur_email) )
			{
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s)." RETCODE,
						  server[id].name, accid, account.userid, account.email, cur_email, ip_str);
			}
			else
			{	// everything ok
				safestrcpy(account.email, new_email, sizeof(account.email));
				login_log("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
					server[id].name, accid, account.userid, new_email, ip_str);
				// Save
				account_db.saveAccount(account);
			}
			RFIFOSKIP(fd, 86);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// Receiving a ban request from map-server via char-server (by Yor)
// obsolete, use sending the account data with 2750 instead
		case 0x2725:	
		{
			if (RFIFOREST(fd) < 18)
				return 0;

			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;

			if( account_db.searchAccount(accid, account) )
			{
				time_t timestamp;
				struct tm *tmtime;
				if( account.ban_until == 0 || account.ban_until < time(NULL) )
					timestamp = time(NULL);
				else
					timestamp = account.ban_until;

				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
				tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
				tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
				tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
				tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
				tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);
				timestamp = mktime(tmtime);
				
				if( timestamp != -1 )
				{
					if( timestamp <= time(NULL) )
						timestamp = 0;
					if( account.ban_until != timestamp )
					{
						if(timestamp != 0)
						{
							unsigned char buf[16];
							char tmpstr[2048];
							strftime(tmpstr, 24, date_format, localtime(&timestamp));
							login_log("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s)." RETCODE,
								server[id].name, accid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = account.account_id;
							WBUFB(buf,6) = 1; // 0: change of status, 1: ban
							WBUFL(buf,7) = timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							
							account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						else
						{
							login_log("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s)." RETCODE,
								server[id].name, accid, ip_str);
						}
						account.ban_until = timestamp;
						// Save
						account_db.saveAccount(account);
					}
					else
					{
						login_log("Char-server '%s': Error of ban request (account: %d, no change for ban date, ip: %s)." RETCODE,
							server[id].name, accid, ip_str);
					}
				}
				else
				{
					login_log("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s)." RETCODE,
						server[id].name, accid, ip_str);
				}
			}
			else
			{
				login_log("Char-server '%s': Error of ban request (account: %d not found, ip: %s)." RETCODE,
					server[id].name, accid, ip_str);
			}
			RFIFOSKIP(fd,18);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Change of sex (sex is reversed)
// obsolete, use sending the account data with 2750 instead
		case 0x2727:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			
			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;

			if( account_db.searchAccount(accid, account) )
			{
				if( account.sex == 2 )
				{
					login_log("Char-server '%s': Error of sex change - Server account (suggested account: %d, actual sex %d (Server), ip: %s)." RETCODE,
						server[id].name, accid, account.sex, ip_str);
				}
				else
				{
					unsigned char buf[16];
					account.sex = (account.sex == 0) ? 1:0;
					// Save
					account_db.saveAccount(account);
					
					login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
						server[id].name, accid, (account.sex == 2) ? 'S' : (account.sex ? 'M' : 'F'), ip_str);
					account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)

					WBUFW(buf,0) = 0x2723;
					WBUFL(buf,2) = accid;
					WBUFB(buf,6) = account.sex;
					charif_sendallwos(-1, buf, 7);
				}
			}
			else
			{
				login_log("Char-server '%s': Error of sex change (account: %d not found, sex would be reversed, ip: %s)." RETCODE,
					      server[id].name, accid, ip_str);
			}
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// We receive account_reg2 from a char-server, and we send them to other char-servers.
// obsolete, use sending the account data with 2750 instead
		case 0x2728:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			size_t p, j;
			size_t sz = RFIFOW(fd,2);
			uint32 accid = RFIFOL(fd,4);
			CLoginAccount account;

			if( account_db.searchAccount(accid, account) )
			{
				login_log("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d, ip: %s)." RETCODE,
					server[id].name, accid, ip_str);
				
				for(p=8, j=0; p<sz && j<ACCOUNT_REG2_NUM; p += 36, ++j)
				{
					memcpy(account.account_reg2[j].str, RFIFOP(fd,p), 32);
					account.account_reg2[j].str[31] = '\0';
					remove_control_chars((account.account_reg2[j].str));
					account.account_reg2[j].value = RFIFOL(fd,p+32);
				}
				account.account_reg2_num = j;
				// Save
				account_db.saveAccount(account);
				
				// Sending information towards the other char-servers.
				RFIFOW(fd,0) = 0x2729;
				charif_sendallwos(fd, RFIFOP(fd,0), sz);
//				ShowMessage("parse_fromchar: receiving (from the char-server) of account_reg2 (account id: %d).\n", accid);
			}
			else
			{
//				ShowMessage("parse_fromchar: receiving (from the char-server) of account_reg2 (unknwon account id: %d).\n", accid);
				login_log("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s)." RETCODE,
					server[id].name, accid, ip_str);
			}
			RFIFOSKIP(fd, sz);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Receiving of map-server via char-server a unban resquest (by Yor)
// obsolete, use sending the account data with 2750 instead
		case 0x272a:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			
			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;

			if( account_db.searchAccount(accid, account) )
			{
				if( account.ban_until != 0 )
				{
					account.ban_until = 0;
					// Save
					account_db.saveAccount(account);

					login_log("Char-server '%s': UnBan request (account: %d, ip: %s)." RETCODE,
						server[id].name, accid, ip_str);
				}
				else
				{
					login_log("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s)." RETCODE,
						server[id].name, accid, ip_str);
				}
			}
			else
			{
				login_log("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s)." RETCODE,
					server[id].name, accid, ip_str);
			}
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Set account_id to online [Wizputer]
// obsolete, use sending the account data with 2750 instead
		case 0x272b:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;
			if( account_db.searchAccount(accid, account) )
			{
				account.online=1;
				// Save
				account_db.saveAccount(account);
			}
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Set account_id to offline [Wizputer]
// obsolete, use sending the account data with 2750 instead
		case 0x272c:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			uint32 accid = RFIFOL(fd,2);
			CLoginAccount account;
			if( account_db.searchAccount(accid, account) )
			{
				account.online=0;
				// Save
				account_db.saveAccount(account);
			}
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		//change sex for chrif_changesex()
// obsolete, use sending the account data with 2750 instead
		case 0x3000:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			uint32 accid = RFIFOL(fd,4);
			unsigned char sex = RFIFOB(fd,8);
			CLoginAccount account;
			if( account_db.searchAccount(accid, account) )
			{
				login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
						  server[id].name, accid, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip_str);
				account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
				account.sex = sex;
				// Save
				account_db.saveAccount(account);

				unsigned char buf[16];
				WBUFW(buf,0) = 0x2723;
				WBUFL(buf,2) = accid;
				WBUFB(buf,6) = sex;
				charif_sendallwos(-1, buf, 7);
			}
			else
			{
				login_log("Char-server '%s': Error of Sex change (account: %d not found, suggested sex %c, ip: %s)." RETCODE,
					server[id].name, accid, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip_str);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		default:
			save_packet(fd, "parse_fromchar", ip_str);
			ShowWarning("parse_fromchar: Unknown packet 0x%x (from a char-server)! -> disconnection.\n", command);
			session_Remove(fd);
			server[id].fd = -1;
			return 0;
		}//end case
	}// end while
	return 0;
}

//---------------------------------------
// Packet parsing for administation login
//---------------------------------------
int parse_admin(int fd)
{
	char ip_str[16]="";
	if(session[fd]) session[fd]->client_ip.tostring(ip_str, sizeof(ip_str));

	if( !session_isActive(fd) )
	{
		ShowWarning("Remote administration has disconnected (session #%d).\n", fd);
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		unsigned short command = RFIFOW(fd,0);
		if (display_parse_admin == 1)
			ShowMessage("parse_admin: connection #%d, packet: 0x%x (with being read: %d).\n", fd, command, RFIFOREST(fd));

		switch( command )
		{
		///////////////////////////////////////////////////////////////////////
		// Request of the server version
		case 0x7530:
		{
			login_log("'ladmin': Sending of the server version (ip: %s)" RETCODE, ip_str);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_LOGIN;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request of end of connection
		case 0x7532:
		{
			login_log("'ladmin': End of connection (ip: %s)" RETCODE, ip_str);
			RFIFOSKIP(fd,2);
			session_Remove(fd);
			return 0;
		}
		///////////////////////////////////////////////////////////////////////
		// Request of an accounts list (message size max 65k)
		case 0x7920:
		{
			if (RFIFOREST(fd) < 10)
				return 0;
			size_t len;
			uint32 st = RFIFOL(fd,2);
			uint32 ed = RFIFOL(fd,6);

			if(st>ed) basics::swap(st,ed);
			if(ed > END_ACCOUNT_NUM || ed < st || ed <= 0)
				ed = END_ACCOUNT_NUM;

			login_log("'ladmin': Sending an accounts list (ask: from %d to %d, ip: %s)" RETCODE, st, ed, ip_str);
			// Sending accounts information

			WFIFOW(fd,0) = 0x7921;
			len = 4;
/*
			// lock the database
			account_db.aquire();
			// go through accounts
			while( account_db && len < 65536 )
			{
				if(account_db().account_id >= st && account_db().account_id <= ed)
				{
					WFIFOL(fd,len) = account_db().account_id;
					WFIFOB(fd,len+4) = account_db().gm_level;
					memcpy(WFIFOP(fd,len+5), account_db().userid, 24);
					WFIFOB(fd,len+29) = account_db().sex;
					WFIFOL(fd,len+30) = account_db().login_count;
					if(account_db().ban_until != 0)
						WFIFOL(fd,len+34) = 7; // 6 = Your are Prohibited to log in until %s
					else
						WFIFOL(fd,len+34) = 0;
					len += 38;
				}
				// next account
				account_db++;
			}
			// release the database
			account_db.release();
*/

			size_t i;
			for(i=0; i<account_db.size() && len < 65536; ++i)
			{
				CLoginAccount &account = account_db[i]; // sorted by index 0
				if(account.account_id >= st && account.account_id <= ed)
				{
					WFIFOL(fd,len) = account.account_id;
					WFIFOB(fd,len+4) = account.gm_level;
					memcpy(WFIFOP(fd,len+5), account.userid, 24);
					WFIFOB(fd,len+29) = account.sex;
					WFIFOL(fd,len+30) = account.login_count;
					if(account.ban_until != 0)
						WFIFOL(fd,len+34) = 7; // 6 = Your are Prohibited to log in until %s
					else
						WFIFOL(fd,len+34) = 0;
					len += 38;
				}
			}

			WFIFOW(fd,2) = len;
			WFIFOSET(fd,len);

			RFIFOSKIP(fd,10);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request for an account creation
		case 0x7930:
		{
			if (RFIFOREST(fd) < 91)
				return 0;

			char* userid = (char*)RFIFOP(fd, 2);
			char* passwd = (char*)RFIFOP(fd, 26);
			passwd[23] = '\0';
			unsigned char sex = RFIFOB(fd,50);

			remove_control_chars(userid);
			remove_control_chars(passwd);

			WFIFOW(fd,0) = 0x7931;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			memcpy(WFIFOP(fd,6), userid, 24);

			if( strlen(userid) > 23 || strlen(passwd) > 23 )
			{
				login_log("'ladmin': Attempt to create an invalid account (account or pass is too long, ip: %s)" RETCODE,
					ip_str);
			}
			else if( strlen(userid) < 4 || strlen(passwd) < 4 )
			{
				login_log("'ladmin': Attempt to create an invalid account (account or pass is too short, ip: %s)" RETCODE,
					ip_str);
			}
			else if( basics::upcase(sex) != 'F' && basics::upcase(sex) != 'M' )
			{
				login_log("'ladmin': Attempt to create an invalid account (account: %s, received pass: %s, invalid sex, ip: %s)" RETCODE,
					userid, passwd, ip_str);
			}
			else if( account_db.existAccount(userid) )
			{
				login_log("'ladmin': Attempt to create an already existing account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
			}
			else
			{
				CLoginAccount account;
				char* email = (char*)RFIFOP(fd,51);
				remove_control_chars(email);

				if( account_db.insertAccount(userid, passwd, (basics::upcase(sex)=='M')?1:0, email, account) )
				{
					login_log("'ladmin': Account creation (account: %s (id: %d), pass: %s, sex: %c, email: %s, ip: %s)" RETCODE,
						      account.userid, account.account_id, account.passwd, account.sex, account.email, ip_str);
					WFIFOL(fd,2) = account.account_id;
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,91);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request for an account deletion
		case 0x7932:
		{
			if (RFIFOREST(fd) < 26)
				return 0;
			CLoginAccount account;
			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7933;
			WFIFOL(fd,2) = 0xFFFFFFFF;

			if( account_db.searchAccount(userid, account) )
			{	// save deleted account in log file
				login_log("'ladmin': Account deletion (account: %s, id: %d, ip: %s) - saved in next line:" RETCODE,
				          account.userid, account.account_id, ip_str);
				// delete account

				account_db.removeAccount(account.account_id);

				// send answer
				memcpy(WFIFOP(fd,6), account.userid, 24);
				WFIFOL(fd,2) = account.account_id;

				// Char-server is notified of deletion (for characters deletion).
				unsigned char buf[16];
				WBUFW(buf,0) = 0x2730;
				WBUFL(buf,2) = account.account_id;
				charif_sendallwos(-1, buf, 6);
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to delete an unknown account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to change a password
		case 0x7934:
		{
			if (RFIFOREST(fd) < 50)
				return 0;

			CLoginAccount account;
			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7935;
			WFIFOL(fd,2) = 0xFFFFFFFF;

			if( account_db.searchAccount(userid, account) )
			{
				char* pass = (char*)RFIFOP(fd,26);
				pass[23] = '\0';
				remove_control_chars(pass);
				
				safestrcpy(account.passwd, pass, sizeof(account.passwd));
				account_db.saveAccount(account);
				
				memcpy(WFIFOP(fd,6), account.userid, 24);
				WFIFOL(fd,2) = account.account_id;
				login_log("'ladmin': Modification of a password (account: %s, new password: %s, ip: %s)" RETCODE,
				          account.userid, account.passwd, ip_str);
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to modify the password of an unknown account (account: %s, ip: %s)" RETCODE,
				          userid, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to modify a state
		case 0x7936:
		{
			if (RFIFOREST(fd) < 50)
				return 0;
			{
				CLoginAccount account;
				char*userid = (char*)RFIFOP(fd,2);
				userid[23] = '\0';
				remove_control_chars(userid);
				uint32 status = RFIFOL(fd,26);
				char* error_message = (char*)RFIFOP(fd,30);
				error_message[19] = '\0';
				remove_control_chars(error_message);

				if(status != 7 || error_message[0] == '\0')
				{	// 7: // 6 = Your are Prohibited to log in until %s
					error_message = (char*)"-";
				}

				WFIFOW(fd,0) = 0x7937;
				WFIFOL(fd,2) = 0xFFFFFFFF;

				if( account_db.searchAccount(userid, account) )
				{
					memcpy(WFIFOP(fd,6), account.userid, 24);
					WFIFOL(fd,2) = account.account_id;

					if (status == 7)
					{
						login_log("'ladmin': Modification of a state (account: %s, new state: %d - prohibited to login until '%s', ip: %s)" RETCODE,
							account.userid, status, error_message, ip_str);
					}
					else
					{
						login_log("'ladmin': Modification of a state (account: %s, new state: %d, ip: %s)" RETCODE,
							account.userid, status, ip_str);
					}
					/*
					if( account.state == 0)
					{
						unsigned char buf[16];
						WBUFW(buf,0) = 0x2731;
						WBUFL(buf,2) = account.account_id;
						WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
						WBUFL(buf,7) = status; // status or final date of a banishment
						charif_sendallwos(-1, buf, 11);
						account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
					}
					account.state = status;
					safestrcpy(account.error_message, error_message, sizeof(account.error_message));
					account_db.saveAccount(account);
					*/
				}
				else
				{
					memcpy(WFIFOP(fd,6), userid, 24);
					login_log("'ladmin': Attempt to modify the state of an unknown account (account: %s, received state: %d, ip: %s)" RETCODE,
						userid, status, ip_str);
				}
				WFIFOL(fd,30) = status;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,50);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request for servers list and # of online players
		case 0x7938:
		{
			size_t i;
			login_log("'ladmin': Sending of servers list (ip: %s)" RETCODE, ip_str);
			server_num = 0;
			for(i = 0; i < MAX_SERVERS; ++i)
			{
				if (server[i].fd >= 0)
				{
					WFIFOLIP(fd,4+server_num*32) = server[i].address.WANIP();
					WFIFOW(fd,4+server_num*32+4) = server[i].address.WANPort();
					memcpy(WFIFOP(fd,4+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,4+server_num*32+26) = server[i].users;
					WFIFOW(fd,4+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,4+server_num*32+30) = server[i].new_display;
					server_num++;
				}
			}
			WFIFOW(fd,0) = 0x7939;
			WFIFOW(fd,2) = 4 + 32 * server_num;
			WFIFOSET(fd,4+32*server_num);
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to password check
		case 0x793a:
		{
			if (RFIFOREST(fd) < 50)
				return 0;

			CLoginAccount account;

			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x793b;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount(userid, account) )
			{
				char* pass = (char*)RFIFOP(fd,26);
				pass[23] = '\0';
				remove_control_chars(pass);

				memcpy(WFIFOP(fd,6), account.userid, 24);
	
				if( 0==strcmp(account.passwd, pass) )
				{
					WFIFOL(fd,2) = account.account_id;
					login_log("'ladmin': Check of password OK (account: %s, password: %s, ip: %s)" RETCODE,
						account.userid, account.passwd, ip_str);
				}
				else
				{
					login_log("'ladmin': Failure of password check (account: %s, proposed pass: %s, ip: %s)" RETCODE,
						account.userid, pass, ip_str);
				}
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to check the password of an unknown account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to modify sex
		case 0x793c:
		{
			if (RFIFOREST(fd) < 27)
				return 0;
			CLoginAccount account;

			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x793d;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			memcpy(WFIFOP(fd,6), userid, 24);
			
			char sex = basics::upcase( (char)RFIFOB(fd,26) );

			if( sex != 'F' && sex != 'M')
			{
				if (sex > 31)
					login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: %c, ip: %s)" RETCODE,
						userid, sex, ip_str);
				else
					login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: 'control char', ip: %s)" RETCODE,
						userid, ip_str);
			}
			else if( account_db.searchAccount(userid, account) )
			{
				memcpy(WFIFOP(fd,6), account.userid, 24);
				if( account.sex != (sex=='S') ? 2 : (sex=='M') )
				{
					unsigned char buf[16];
					WFIFOL(fd,2) = account.account_id;
					
					account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
					account.sex = (sex == 'S') ? 2 : (sex == 'M');
					login_log("'ladmin': Modification of a sex (account: %s, new sex: %c, ip: %s)" RETCODE,
						account.userid, sex, ip_str);
					account_db.saveAccount(account);
					
					// send to all char-server the change
					WBUFW(buf,0) = 0x2723;
					WBUFL(buf,2) = account.account_id;
					WBUFB(buf,6) = account.sex;
					charif_sendallwos(-1, buf, 7);
				}
				else
				{
					login_log("'ladmin': Modification of a sex, but the sex is already the good sex (account: %s, sex: %c, ip: %s)" RETCODE,
						account.userid, sex, ip_str);
				}
			}
			else
			{
				login_log("'ladmin': Attempt to modify the sex of an unknown account (account: %s, received sex: %c, ip: %s)" RETCODE,
					userid, sex, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to modify GM level
		case 0x793e:
		{
			if (RFIFOREST(fd) < 27)
				return 0;

			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);
			
			WFIFOW(fd,0) = 0x793f;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			memcpy(WFIFOP(fd,6), userid, 24);


			unsigned char new_gm_level = RFIFOB(fd,26);
			
			if(new_gm_level > 99)
			{
				login_log("'ladmin': Attempt to give an invalid GM level (account: %s, received GM level: %d, ip: %s)" RETCODE,
					userid, new_gm_level, ip_str);
			}
			else
			{
				CLoginAccount account;
				if( account_db.searchAccount(userid, account) )
				{
					memcpy(WFIFOP(fd,6), account.userid, 24);
					if( account.gm_level != new_gm_level )
					{	// modification 
						account.gm_level = new_gm_level;
						account_db.saveAccount(account);
					}
					else
					{
						login_log("'ladmin': Attempt to modify of a GM level, but the GM level is already set GM level (account: %s (%d), GM level: %d, ip: %s)" RETCODE,
							account.userid, account.account_id, new_gm_level, ip_str);
					}
				}
				else
				{
					login_log("'ladmin': Attempt to modify the GM level of an unknown account (account: %s, received GM level: %d, ip: %s)" RETCODE,
						userid, new_gm_level, ip_str);
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to modify e-mail
		case 0x7940:
		{
			if (RFIFOREST(fd) < 66)
				return 0;
			char *userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);
			memcpy(WFIFOP(fd,6), userid, 24);

			char *email = (char*)RFIFOP(fd,26);
			userid[39] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7941;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( !email_check(email) )
			{
				login_log("'ladmin': Attempt to give an invalid e-mail (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
			}
			else
			{
				CLoginAccount account;
				if( account_db.searchAccount(userid, account) )
				{
					safestrcpy(account.email, email, sizeof(account.email));
					account_db.saveAccount(account);
					
					WFIFOL(fd,2) = account.account_id;
					memcpy(WFIFOP(fd,6), account.userid, 24);
					
					login_log("'ladmin': Modification of an email (account: %s, new e-mail: %s, ip: %s)" RETCODE,
						account.userid, email, ip_str);

				}
				else
				{
					login_log("'ladmin': Attempt to modify the e-mail of an unknown account (account: %s, received e-mail: %s, ip: %s)" RETCODE,
						userid, email, ip_str);
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,66);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to modify memo field
		case 0x7942:
		{
			if (RFIFOREST(fd) < 28 || RFIFOREST(fd) < (28 + RFIFOW(fd,26)))
				return 0;

			CLoginAccount account;
			
			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7943;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount(userid, account) )
			{
				if(RFIFOW(fd,26) == 0)
				{
//					strcpy(account.memo, "-");
				}
				else
				{
					char *buf = (char*)RFIFOP(fd,28);
					remove_control_chars(buf);
//					safestrcpy(account.memo, buf, sizeof(account.memo));
				}
//				account_db.saveAccount(account);
				
				WFIFOL(fd,2) = account.account_id;
				memcpy(WFIFOP(fd,6), account.userid, 24);
//				login_log("'ladmin': Modification of a memo field (account: %s, new memo: %s, ip: %s)" RETCODE,
//				          account.userid, account.memo, ip_str);
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to modify the memo field of an unknown account (account: %s, ip: %s)" RETCODE,
				          userid, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,28 + RFIFOW(fd,26));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to find an account id
		case 0x7944:
		{
			if (RFIFOREST(fd) < 26)
				return 0;

			CLoginAccount account;
			char *userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);
			
			WFIFOW(fd,0) = 0x7945;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount(userid, account) )
			{
				
				WFIFOL(fd,2) = account.account_id;
				memcpy(WFIFOP(fd,6), account.userid, 24);
				login_log("'ladmin': Request (by the name) of an account id (account: %s, id: %d, ip: %s)" RETCODE,
					account.userid, account.account_id, ip_str);
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': ID request (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to find an account name
		case 0x7946:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			CLoginAccount account;
			
			WFIFOW(fd,0) = 0x7947;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,6), '\0', 24);

			if( account_db.searchAccount( RFIFOL(fd,2), account) )
			{
				memcpy(WFIFOP(fd,6), account.userid, 24);
				login_log("'ladmin': Request (by id) of an account name (account: %s, id: %d, ip: %s)" RETCODE,
					account.userid, (uint32)RFIFOL(fd,2), ip_str);
			}
			else
			{
				login_log("'ladmin': Name request (by id) of an unknown account (id: %d, ip: %s)" RETCODE,
					(uint32)RFIFOL(fd,2), ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to change the validity limit (timestamp) (absolute value)
		case 0x7948:
		{
			if (RFIFOREST(fd) < 30)
				return 0;

			CLoginAccount account;
			
			char tmpstr[2048];
			time_t timestamp = (time_t)RFIFOL(fd,26);
			char*userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			strftime(tmpstr, 24, date_format, localtime(&timestamp));

			WFIFOW(fd,0) = 0x7949;
			WFIFOL(fd,2) = 0xFFFFFFFF;

			if( account_db.searchAccount( userid, account) )
			{
				
				login_log("'ladmin': Change of a validity limit (account: %s, new validity: %d (%s), ip: %s)" RETCODE,
					account.userid, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip_str);
				account.valid_until = timestamp;

				account_db.saveAccount(account);

				WFIFOL(fd,2) = account.account_id;
				memcpy(WFIFOP(fd,6), account.userid, 24);
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to change the validity limit of an unknown account (account: %s, received validity: %d (%s), ip: %s)" RETCODE,
					userid, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip_str);
			}
			WFIFOL(fd,30) = (uint32)timestamp;
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to change the final date of a banishment (timestamp) (absolute value)
		case 0x794a:
		{
			if (RFIFOREST(fd) < 30)
				return 0;
			
			CLoginAccount account;
			
			char tmpstr[2048];
			time_t timestamp = (time_t)RFIFOL(fd,26);
			
			char*userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);
			
			if (timestamp <= time(NULL))
				timestamp = 0;
			strftime(tmpstr, 24, date_format, localtime(&timestamp));

			WFIFOW(fd,0) = 0x794b;
			WFIFOL(fd,2) = 0xFFFFFFFF;

			if( account_db.searchAccount( userid, account) )
			{
				memcpy(WFIFOP(fd,6), account.userid, 24);
				WFIFOL(fd,2) = account.account_id;
				
				login_log("'ladmin': Change of the final date of a banishment (account: %s, new final date of banishment: %d (%s), ip: %s)" RETCODE,
					account.userid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
				
				if(account.ban_until != timestamp)
				{
					if (timestamp != 0)
					{
						unsigned char buf[16];
						WBUFW(buf,0) = 0x2731;
						WBUFL(buf,2) = account.account_id;
						WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
						WBUFL(buf,7) = timestamp; // status or final date of a banishment
						charif_sendallwos(-1, buf, 11);
						account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						account.ban_until = timestamp;
						account_db.saveAccount(account);
					}
				}
				else
				{
					memcpy(WFIFOP(fd,6), userid, 24);
					login_log("'ladmin': Attempt to change the final date of a banishment of an unknown account (account: %s, received final date of banishment: %d (%s), ip: %s)" RETCODE,
					          userid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
				}
				WFIFOL(fd,30) = timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to change the final date of a banishment (timestamp) (relative change)
		case 0x794c:
		{
			if (RFIFOREST(fd) < 38)
				return 0;

			CLoginAccount account;
			char *userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);
			
			WFIFOW(fd,0) = 0x794d;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount( userid, account) )
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];

				WFIFOL(fd,2) = account.account_id;
				memcpy(WFIFOP(fd,6), account.userid, 24);
				
				if(account.ban_until == 0 || account.ban_until < time(NULL))
					timestamp = time(NULL);
				else
					timestamp = account.ban_until;
				
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
				tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
				tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
				tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
				tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
				tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
				timestamp = mktime(tmtime);
				
				if (timestamp != -1)
				{
					if (timestamp <= time(NULL))
						timestamp = 0;
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					login_log("'ladmin': Adjustment of a final date of a banishment (account: %s, (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
						account.userid, (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
					if (account.ban_until != timestamp)
					{
						if (timestamp != 0)
						{
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = account.account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							account.login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						account.ban_until = timestamp;
						account_db.saveAccount(account);
					}
					else
					{
						strftime(tmpstr, 24, date_format, localtime(&account.ban_until));
						login_log("'ladmin': Impossible to adjust the final date of a banishment (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
						          account.userid, account.ban_until, (account.ban_until == 0 ? "no banishment" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip_str);
					}
					WFIFOL(fd,30) = (uint32)account.ban_until;
				}
				else
				{
					memcpy(WFIFOP(fd,6), userid, 24);
					login_log("'ladmin': Attempt to adjust the final date of a banishment of an unknown account (account: %s, ip: %s)" RETCODE,
					          userid, ip_str);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to send a broadcast message
		case 0x794e:
		{
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + (int)RFIFOL(fd,4)))
				return 0;
			WFIFOW(fd,0) = 0x794f;
			WFIFOW(fd,2) = 0xFFFF;
			if (RFIFOL(fd,4) < 1)
			{
				login_log("'ladmin': Receiving a message for broadcast, but message is void (ip: %s)" RETCODE,
					ip_str);
			}
			else
			{
				size_t i;
				for(i = 0; i < MAX_SERVERS; ++i)
					if (server[i].fd >= 0)
						break;
				if (i == MAX_SERVERS)
				{
					login_log("'ladmin': Receiving a message for broadcast, but no char-server is online (ip: %s)" RETCODE,
						ip_str);
				}
				else
				{	// at least 1 char-server
					unsigned char buf[32016];
					char message[32000];
					size_t sz = RFIFOL(fd,4);
					if(sz > sizeof(message)) sz = sizeof(message)-1;
					WFIFOW(fd,2) = 0;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), sz);
					message[sizeof(message)-1] = '\0';
					remove_control_chars(message);
					if (RFIFOW(fd,2) == 0)
						login_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s, ip: %s)" RETCODE,
							message, ip_str);
					else
						login_log("'ladmin': Receiving a message for broadcast (message (in blue): %s, ip: %s)" RETCODE,
							message, ip_str);
					// send same message to all char-servers (no answer)
					memcpy(buf, RFIFOP(fd,0), 8+sz);
					WBUFW(buf,0) = 0x2726;
					charif_sendallwos(-1, buf, 8+sz);
				}
			}
			WFIFOSET(fd,4);
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to change the validity limite (timestamp) (relative change)
		case 0x7950:
		{
			if (RFIFOREST(fd) < 38)
				return 0;
			CLoginAccount account;
			char* userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7951;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount( userid, account) )
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				char tmpstr2[2048];
				
				WFIFOL(fd,2) = account.account_id;
				memcpy(WFIFOP(fd,6), account.userid, 24);
				timestamp = account.valid_until;
				if(add_to_unlimited_account == 0 && timestamp == 0)
				{
					login_log("'ladmin': Attempt to adjust the validity limit of an unlimited account (account: %s, ip: %s)" RETCODE,
						account.userid, ip_str);
					WFIFOL(fd,30) = 0;
				}
				else
				{
					if (timestamp == 0 || timestamp < time(NULL))
						timestamp = time(NULL);
					tmtime = localtime(&timestamp);
					tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
					tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
					tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
					tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
					tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
					tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
					timestamp = mktime(tmtime);
					if (timestamp != -1)
					{
						strftime(tmpstr, 24, date_format, localtime(&account.valid_until));
						strftime(tmpstr2, 24, date_format, localtime(&timestamp));
						login_log("'ladmin': Adjustment of a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
							account.userid, account.valid_until, (account.valid_until == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "unlimited" : tmpstr2), ip_str);
						account.valid_until = timestamp;
						
						account_db.saveAccount(account);
						
						WFIFOL(fd,30) = (uint32)timestamp;
					}
					else
					{
						strftime(tmpstr, 24, date_format, localtime(&account.valid_until));
						login_log("'ladmin': Impossible to adjust a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
							account.userid, account.valid_until, (account.valid_until == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip_str);
						WFIFOL(fd,30) = 0;
					}
				}
			}
			else
			{
				memcpy(WFIFOP(fd,6), userid, 24);
				login_log("'ladmin': Attempt to adjust the validity limit of an unknown account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
				WFIFOL(fd,30) = 0;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request information of an account (by account name)
		case 0x7952:
		{
			if (RFIFOREST(fd) < 26)
				return 0;

			CLoginAccount account;
			char *userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			
			if( account_db.searchAccount(userid, account) )
			{
				WFIFOL(fd,2) = account.account_id;
				WFIFOB(fd,6) = account.gm_level;
				memcpy(WFIFOP(fd,7), account.userid, 24);
				WFIFOB(fd,31) = account.sex;
				WFIFOL(fd,32) = account.login_count;
				WFIFOL(fd,36) = 0;//account.state;
				WFIFOB(fd,40) = 0;//memcpy(WFIFOP(fd,40), account.error_message, 20);
				memcpy(WFIFOP(fd,60), account.last_login, 24);
				memcpy(WFIFOP(fd,84), account.last_ip, 16);
				memcpy(WFIFOP(fd,100), account.email, 40);
				WFIFOL(fd,140) = (uint32)account.valid_until;
				WFIFOL(fd,144) = (uint32)account.ban_until;
				WFIFOW(fd,148) = 0;//strlen(account.memo);
				//if(account.memo[0])
				//{
				//	memcpy(WFIFOP(fd,150), account.memo, strlen(account.memo));
				//}
				login_log("'ladmin': Sending information of an account (request by the name; account: %s, id: %d, ip: %s)" RETCODE,
					account.userid, account.account_id, ip_str);
				//WFIFOSET(fd,150+strlen(account.memo));
				WFIFOSET(fd,150);
			}
			else
			{
				memcpy(WFIFOP(fd,7), userid, 24);
				WFIFOW(fd,148) = 0;
				login_log("'ladmin': Attempt to obtain information (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
					userid, ip_str);
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,26);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request about information of an account (by account id)
		case 0x7954:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			CLoginAccount account;

			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,7), '\0', 24);
			
			if( account_db.searchAccount( RFIFOL(fd,2), account) )
			{
				
				login_log("'ladmin': Sending information of an account (request by the id; account: %s, id: %d, ip: %s)" RETCODE,
					account.userid, (uint32)RFIFOL(fd,2), ip_str);
				WFIFOB(fd,6) = account.gm_level;
				memcpy(WFIFOP(fd,7), account.userid, 24);
				WFIFOB(fd,31) = account.sex;
				WFIFOL(fd,32) = account.login_count;
				WFIFOL(fd,36) = 0;//account.state;
				WFIFOB(fd,40) = 0;//memcpy(WFIFOP(fd,40), account.error_message, 20);
				memcpy(WFIFOP(fd,60), account.last_login, 24);
				memcpy(WFIFOP(fd,84), account.last_ip, 16);
				memcpy(WFIFOP(fd,100), account.email, 40);
				WFIFOL(fd,140) = (uint32)account.valid_until;
				WFIFOL(fd,144) = (uint32)account.ban_until;
				WFIFOW(fd,148) = 0;//strlen(account.memo);
				//if(account.memo[0])
				//{
				//	memcpy(WFIFOP(fd,150), account.memo, strlen(account.memo));
				//}
				//WFIFOSET(fd,150+strlen(account.memo));
				WFIFOSET(fd,150);
			}
			else
			{
				login_log("'ladmin': Attempt to obtain information (by the id) of an unknown account (id: %d, ip: %s)" RETCODE,
					(uint32)RFIFOL(fd,2), ip_str);
				safestrcpy((char*)WFIFOP(fd,7), "", 24);
				WFIFOW(fd,148) = 0;
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to reload GM file (no answer)
		case 0x7955:
		{
			login_log("'ladmin': Request to re-load GM configuration file (ip: %s)." RETCODE, ip_str);
			//read_gm_account();
			// send GM accounts to all char-servers
			//send_GM_accounts();
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		default:
			save_packet(fd, "parse_admin", ip_str);
			ShowWarning("parse_admin: Unknown packet 0x%x (from a ladmin)! -> disconnection.\n", command);
			session_Remove(fd);
			return 0;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd)
{
	char ip_str[16];
	if(session[fd]) session[fd]->client_ip.tostring(ip_str, sizeof(ip_str));

	if ( !session_isActive(fd) )
	{
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}
	
	while(RFIFOREST(fd) >= 2)
	{
		unsigned short command = RFIFOW(fd,0);
		if (display_parse_login == 1)
		{
			if( command == 0x64 || command == 0x01dd)
			{
				if( RFIFOREST(fd) >= ((command==0x64) ? 55 : 47) )
					ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d), account: %s.\n", fd, command, RFIFOREST(fd), RFIFOP(fd,6));
			}
			else if( command == 0x2710 )
			{
				if( RFIFOREST(fd) >= 90 )
					ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d), server: %s.\n", fd, command, RFIFOREST(fd), RFIFOP(fd,60));
			}
			else
				ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d).\n", fd, command, RFIFOREST(fd));
		}

		switch(command)
		{
		///////////////////////////////////////////////////////////////////////
		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
		case 0x200:	
		{
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
		case 0x204:
		{
			if (RFIFOREST(fd) < 18)
				return 0;
			RFIFOSKIP(fd,18);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		case 0x64:		// Ask connection of a client
		case 0x277:		// new login packet (layout is same as 0x64 but different length)
		case 0x01dd:	// Ask connection of a client (encryption mode)
		{
			// 0 = Unregistered ID
			// 1 = Incorrect Password
			// 2 = This ID is expired
			// 3 = Rejected from Server
			// 4 = You have been blocked by the GM Team
			// 5 = Your Game's EXE file is not the latest version
			// 6 = Your are Prohibited to log in until %s
			// 7 = Server is jammed due to over populated
			// 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
			// 99 = This ID has been totally erased

			size_t len = ((command==0x64) ? 55 : (command==0x01dd) ? 47 : 84);

			if( (size_t)RFIFOREST(fd) < len )
				return 0;

			if( !check_ip(session[fd]->client_ip) )
			{
				login_log("Connection refused: IP isn't authorised (deny/allow, ip: %s)." RETCODE, ip_str);

				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = 3; // 3 = Rejected from Server
				WFIFOB(fd,3) = 0;
				WFIFOSET(fd,23);
			}
			else 
			{
				CLoginAccount account;
				int passwdenc = (command == 0x64 || command == 0x277) ? 0 : PASSWORDENC;
				char* userid = (char*)RFIFOP(fd,6);
				userid[23] = '\0';
				remove_control_chars(userid);
				char *passwd = (char*)RFIFOP(fd,30);
				if( command == 0x64 )
				{
					passwd[23] = '\0';
					remove_control_chars(passwd);
				}
				else
				{	// If remove control characters from received password encrypted by md5,
					// there would be a wrong result and failed to authentication. [End_of_exam]
					passwd[16] = '\0';
				}
				
				bool ok = account_db.searchAccount(userid, account);
				if( !ok )
				{	// try for account creation with _M/_F
					int len = strlen(userid) - 2;
					if( new_account_flag && allowed_regs &&
						len >= 4 &&
						passwdenc == 0 && 
						userid[len] == '_' &&
						(basics::upcase(userid[len+1]) == 'F' || 
						 basics::upcase(userid[len+1]) == 'M' ) &&
						strlen(passwd) >= 4)
					{
						//only continue if amount in this time limit is allowed (account registration flood protection)[Kevin]
						if( gettick() <= new_reg_tick )
						{
							ShowNotice("Account registration denied (registration limit exceeded) to %s!\n", ip_str);
							login_log("Notice: Account registration denied (registration limit exceeded) to %s!\n", ip_str);
						}
						else
						{
							new_reg_tick = gettick() + 1000*time_allowed;
							userid[len] = '\0';
							ok = !account_db.existAccount(userid) &&
								account_db.insertAccount(userid, passwd, (basics::upcase(userid[len+1]) == 'M'),"a@a.com", account);
							if(ok)
							{
								ShowNotice("Account registration successful account: %s, ip %s!\n", userid, ip_str);
								login_log("Account registration successful account: %s, ip %s!\n", userid, ip_str);
							}
						}
					}
				}
				if(!ok)
				{
					WFIFOW(fd,0) = 0x6a;
					WFIFOB(fd,2) = 0; // 0 = Unregistered ID
					WFIFOB(fd,3) = 0;
					WFIFOSET(fd,23);
				}
				else
				{
					login_session_data *ld = (login_session_data*)session[fd]->user_session;

					login_log("Request for connection %s of %s (ip: %s)."RETCODE, 
						( command == 0x64 )?"(non encryption mode)":"(encryption mode)",account.userid, ip_str);

					if( !check_password(ld, passwdenc, passwd, account.passwd) )
					{
						WFIFOW(fd,0) = 0x6a;
						WFIFOB(fd,2) = 1; // 1 = Incorrect Password
						WFIFOB(fd,3) = 0;
						WFIFOSET(fd,23);
					}
					else if (min_level_to_connect > account.gm_level)
					{
						login_log("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d, ip: %s)." RETCODE,
								  min_level_to_connect, account.userid, account.gm_level, ip_str);
						WFIFOW(fd,0) = 0x81;
						WFIFOL(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
					}
					else if( account.valid_until && account.valid_until<time(NULL) )
					{
						WFIFOW(fd,0) = 0x6a;
						WFIFOB(fd,2) = 2; // 2 = This ID is expired
						WFIFOB(fd,3) = 0;
						WFIFOSET(fd,23);
					}
					else if( account.ban_until && account.ban_until>time(NULL) )
					{
						char tmpstr[32];
						strftime(tmpstr, 20, date_format, localtime(&account.ban_until));
						tmpstr[19] = '\0';
						
						WFIFOW(fd,0) = 0x6a;
						WFIFOB(fd,2) = 6; // 6 = Your are Prohibited to log in until %s
						memcpy(WFIFOP(fd,3), tmpstr, 20);
						WFIFOSET(fd,23);
					}
					else
					{
						size_t i, server_num=0;
						for(i = 0; i < MAX_SERVERS; ++i)
						{
							if( session_isActive(server[i].fd) )
							{
								if( server[i].address.isLAN(session[fd]->client_ip) )
								{
									ShowMessage("Send IP of char-server: %s:%d (%s)\n", server[i].address.LANIP().tostring(NULL), server[i].address.LANPort(), CL_BT_GREEN"LAN"CL_NORM);
									WFIFOLIP(fd,47+server_num*32) = server[i].address.LANIP();
									WFIFOW(fd,47+server_num*32+4) = server[i].address.LANPort();
								}
								else
								{
									ShowMessage("Send IP of char-server: %s:%d (%s)\n", server[i].address.WANIP().tostring(NULL), server[i].address.WANPort(), CL_BT_CYAN"WAN"CL_NORM);
									WFIFOLIP(fd,47+server_num*32) = server[i].address.WANIP();
									WFIFOW(fd,47+server_num*32+4) = server[i].address.WANPort();
								}
								memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
								WFIFOW(fd,47+server_num*32+26) = server[i].users;
								WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
								WFIFOW(fd,47+server_num*32+30) = server[i].new_display;
								server_num++;
							}
						}
						// if at least 1 char-server
						if (server_num > 0)
						{
							if(account.gm_level)
								ShowStatus("Connection of the GM (level:%d) account '%s' accepted.\n", account.gm_level, account.userid);
							else
								ShowStatus("Connection of the account '%s' accepted.\n", account.userid);

							// build authentification data
							account.login_id1 = rand();
							account.login_id2 = rand();
							account.client_ip = session[fd]->client_ip;
							// update account information
							timestamp2string(account.last_login, sizeof(account.last_login));
							session[fd]->client_ip.tostring(account.last_ip, sizeof(account.last_ip));
							account.login_count++;
							//account.state = 0;

							// save
							account_db.saveAccount(account);

							// send authentification to char
							unsigned char buf[16+sizeof(account)]; // a bit larger then necessary
							WBUFW(buf,0) = 0x2750;
							account.CCharAccount::tobuffer(WBUFP(buf, 2));
							charif_sendallwos(-1, buf, 2+account.CCharAccount::size() );

							// send server list to client
							WFIFOW(fd,0) = 0x69;
							WFIFOW(fd,2) = 47+32*server_num;
							WFIFOL(fd,4) = account.login_id1;
							WFIFOL(fd,8) = account.account_id;
							WFIFOL(fd,12) = account.login_id2;
							WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
							memcpy(WFIFOP(fd,20), account.last_login, 24); // in old version, that was for name (not more used)
							WFIFOB(fd,46) = account.sex;
							WFIFOSET(fd,47+32*server_num);
						}
						else
						{	// if no char-server, don't send void list of servers, just disconnect the player with proper message
							login_log("Connection refused: there is no char-server online (account: %s, ip: %s)." RETCODE,
								account.userid, ip_str);
							WFIFOW(fd,0) = 0x81;
							WFIFOL(fd,2) = 1; // 01 = Server closed
							WFIFOSET(fd,3);
						}
					}
				}
			}
			RFIFOSKIP(fd,len);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		case 0x01db:	// Sending request of the coding key
		case 0x791a:	// Sending request of the coding key (administration packet)
		{
			login_session_data *ld;
			if (session[fd]->user_session)
			{
				ShowError("login: abnormal request of MD5 key (already opened session).\n");
				session_Remove(fd);
				return 0;
			}
			session[fd]->user_session = ld = new login_session_data;
			if(!ld)
			{
				ShowError("login: Request for md5 key: memory allocation failure (malloc)!\n");
				session_Remove(fd);
				return 0;
			}
			if(command == 0x01db)
				login_log("Sending request of the coding key (ip: %s)" RETCODE, ip_str);
			else
				login_log("'ladmin': Sending request of the coding key (ip: %s)" RETCODE, ip_str);
			WFIFOW(fd,0) = 0x01dc;
			WFIFOW(fd,2) = 4 + ld->md5keylen;
			memcpy(WFIFOP(fd,4), ld->md5key, ld->md5keylen);
			WFIFOSET(fd,WFIFOW(fd,2));
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Connection request of a char-server
		case 0x2710:
		{
			if (RFIFOREST(fd) < 90)
				return 0;

			char *userid = (char*)RFIFOP(fd,2);
			userid[23] = '\0';
			remove_control_chars(userid);

			char *passwd = (char *)RFIFOP(fd,26);
			passwd[23] = '\0';
			remove_control_chars(passwd);

			char* server_name=(char*)RFIFOP(fd,50);
			server_name[19] = '\0';
			remove_control_chars(server_name);

			login_log("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (ip: %s)" RETCODE,
				      server_name, (unsigned char)RFIFOB(fd,74), (unsigned char)RFIFOB(fd,75), (unsigned char)RFIFOB(fd,76), (unsigned char)RFIFOB(fd,77), (unsigned short)RFIFOW(fd,82), ip_str);

			CLoginAccount account;
			if( !account_db.searchAccount(userid, account) || 
				account.sex != 2 || 
				0!=strcmp(passwd, account.passwd) )
			{
				ShowError("Connection of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s).\n", server_name, userid, passwd, ip_str);
				login_log("Connexion of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s)" RETCODE,
					    server_name, userid, passwd, ip_str);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			}
			else
			{
				size_t i;
				for(i=0; i<MAX_SERVERS; ++i)
				{
					if(server[i].fd==-1)
						break;
				}
				if(i>=MAX_SERVERS)
				{
					login_log("Connection of the char-server '%s' refused, we are full (account: %s, pass: %s, ip: %s)" RETCODE,
							  server_name, account.userid, account.passwd, ip_str);
					ShowStatus("Connection of the char-server '%s' refused, we are full \n", server_name);
					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 3;
					WFIFOSET(fd,3);
				}
				else
				{
					memcpy(server[i].name,server_name,20);
					server[i].maintenance=RFIFOB(fd,70);
					server[i].new_display=RFIFOB(fd,71);
					// wanip,wanport,lanip,lanmask,lanport
					server[i].address = basics::ipset(	RFIFOLIP(fd,74), RFIFOLIP(fd,78), RFIFOW(fd,82), RFIFOLIP(fd,84), RFIFOW(fd,88) );
					server[i].users = 0;
					server[i].fd = fd;
					
					session[fd]->func_parse = parse_fromchar;
					realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

					login_log("Connection of the char-server '%s' accepted (account: %s, pass: %s, ip: %s)" RETCODE,
							server[i].name, account.userid, account.passwd, server[i].address.tostring(NULL));
					ShowStatus("Connection of the char-server '%s' (%s) accepted.\n", 
							server[i].name, server[i].address.tostring(NULL));
					
					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
			RFIFOSKIP(fd,90);
			return 0; 
		}
		///////////////////////////////////////////////////////////////////////
		// Request of the server version
		case 0x7530:
		{
			login_log("Sending of the server version (ip: %s)" RETCODE, ip_str);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_LOGIN;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		case 0x7532:	// Request to end connection
		{
			login_log("End of connection (ip: %s)" RETCODE, ip_str);
			session_Remove(fd);
			return 0;
		}
		///////////////////////////////////////////////////////////////////////
		case 0x7918:	// Request for administation login
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < ((RFIFOW(fd,2) == 0) ? 28 : 20))
				return 0;

			WFIFOW(fd,0) = 0x7919;
			WFIFOB(fd,2) = 1;
			if (!check_ladminip( session[fd]->client_ip ))
			{
				login_log("'ladmin'-login: Connection in administration mode refused: IP isn't authorised (ladmin_allow, ip: %s)." RETCODE, ip_str);
			}
			else
			{
				login_session_data *ld = (login_session_data*)session[fd]->user_session;
				if (RFIFOW(fd,2) == 0)
				{	// non encrypted password
					char password[64];
					memcpy(password, RFIFOP(fd,4), 24);
					password[23] = '\0';
					remove_control_chars(password);
					// If remote administration is enabled and password sent by client matches password read from login server configuration file
					if ((admin_state == 1) && (strcmp(password, admin_pass) == 0))
					{
						login_log("'ladmin'-login: Connection in administration mode accepted (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
						ShowStatus("Connection of a remote administration accepted (non encrypted password).\n");
						WFIFOB(fd,2) = 0;
						session[fd]->func_parse = parse_admin;
						session[fd]->rdata_tick = 0;
					}
					else if (admin_state != 1)
						login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
					else
						login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
				}
				else
				{	// encrypted password
					if (!ld)
						ShowError("'ladmin'-login: error! MD5 key not created/requested for an administration login.\n");
					else
					{
						char md5str[64] = "", md5bin[32];
						if (RFIFOW(fd,2) == 1)
						{
							snprintf(md5str, sizeof(md5str),"%s%s", ld->md5key, admin_pass); // 20 24
						}
						else if (RFIFOW(fd,2) == 2)
						{
							snprintf(md5str, sizeof(md5str),"%s%s", admin_pass, ld->md5key); // 24 20
						}
						MD5_String2binary(md5str, md5bin);
						// If remote administration is enabled and password hash sent by client matches hash of password read from login server configuration file
						if ((admin_state == 1) && (memcmp(md5bin, RFIFOP(fd,4), 16) == 0))
						{
							login_log("'ladmin'-login: Connection in administration mode accepted (encrypted password, ip: %s)" RETCODE, ip_str);
							ShowStatus("Connection of a remote administration accepted (encrypted password).\n");
							WFIFOB(fd,2) = 0;
							session[fd]->func_parse = parse_admin;
							session[fd]->rdata_tick = 0;
						}
						else if (admin_state != 1)
							login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (encrypted password, ip: %s)" RETCODE, ip_str);
						else
							login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (encrypted password, ip: %s)" RETCODE, ip_str);
					}
				}
			}
			WFIFOSET(fd,3);
			RFIFOSKIP(fd, (RFIFOW(fd,2) == 0) ? 28 : 20);
			return 0;
		}
		///////////////////////////////////////////////////////////////////////
		default:
			save_packet(fd, "parse_login", ip_str);
			ShowWarning("parse_login: Unknown packet 0x%x! -> disconnection.\n", command);
			session_Remove(fd);
			return 0;
		}
	}
	return 0;
}

//-----------------------
// Console Command Parser [Wizputer]
//-----------------------
int parse_console(const char *buf)
{
	char command[256];

	memset(command,0,sizeof(command));

	sscanf(buf, "%256[^\n]", command);
	basics::itrim(command);

	login_log("Console command :%s" RETCODE, command);

	if(strcasecmp("shutdown", command) == 0 ||
	    strcasecmp("exit", command) == 0 ||
	    strcasecmp("quit", command) == 0 ||
	    strcasecmp("end", command) == 0)
		core_stoprunning();
	else if(strcasecmp("alive", command) == 0 ||
	         strcasecmp("status", command) == 0)
		ShowConsole(CL_BOLD"I'm Alive.\n"CL_NORM);

	else if(strcasecmp("help", command) == 0) {
		ShowMessage(CL_BT_GREEN"Help of commands:\n"CL_NORM);
		ShowMessage("  To shutdown the server:\n");
		ShowMessage("  'shutdown|exit|qui|end'\n");
		ShowMessage("  To know if server is alive:\n");
		ShowMessage("  'alive|status'\n");
	}
	return 0;
}

//-----------------------------------
// Reading general configuration file
//-----------------------------------
int login_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	
	if ((fp = basics::safefopen(cfgName, "r")) == NULL)
	{
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	ShowStatus("Reading Login Configuration %s\n", cfgName);
	while( fgets(line, sizeof(line), fp) )
	{
		if( !prepare_line(line) )
			continue;
		
		line[sizeof(line)-1] = '\0';
		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) == 2)
		{
			remove_control_chars(w1);
			remove_control_chars(w2);
			basics::itrim(w1);
			basics::itrim(w2);
			
			if (strcasecmp(w1, "admin_state") == 0)
			{
				admin_state = config_switch(w2);
			}
			else if (strcasecmp(w1, "admin_pass") == 0)
			{
				safestrcpy(admin_pass, w2, sizeof(admin_pass));
			}
			else if (strcasecmp(w1, "ladminallowip") == 0)
			{
				if (strcasecmp(w2, "clear") == 0)
				{
					if (access_ladmin_allow)
					{
						delete[] access_ladmin_allow;
						access_ladmin_allow = NULL;
						access_ladmin_allownum = 0;
					}
				}
				else
				{
					if (strcasecmp(w2, "all") == 0)
					{	// reset all previous values
						if(access_ladmin_allow)
							delete[] access_ladmin_allow;
						// set to all
						access_ladmin_allow = new char[ACO_STRSIZE];
						access_ladmin_allownum = 1;
						access_ladmin_allow[0] = '\0';
					}
					else if (w2[0] && !(access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0'))
					{	// don't add IP if already 'all'
						new_realloc(access_ladmin_allow, access_ladmin_allownum*ACO_STRSIZE, ACO_STRSIZE);
						safestrcpy(access_ladmin_allow + (access_ladmin_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
			} else if (strcasecmp(w1, "gm_pass") == 0)
			{
				safestrcpy(gm_pass, w2, sizeof(gm_pass));
			}
			else if (strcasecmp(w1, "level_new_gm") == 0)
			{
				level_new_gm = atoi(w2);
			}
			else if (strcasecmp(w1, "new_account") == 0)
			{
				new_account_flag = config_switch(w2);
			}
			else if (strcasecmp(w1, "login_ip") == 0) {
				loginaddress = w2;
				ShowMessage("Login server IP address : %s -> %s\n", w2, loginaddress.tostring(NULL));
			}
			else if (strcasecmp(w1, "login_port") == 0)
			{
				loginaddress.port() = atoi(w2);
			}
			else if (strcasecmp(w1, "use_MD5_passwords") == 0)
			{
				use_md5_passwds = config_switch(w2);
			}
			else if (strcasecmp(w1, "login_log_filename") == 0)
			{
				safestrcpy(login_log_filename, w2, sizeof(login_log_filename));
			}
			else if (strcasecmp(w1, "log_login") == 0)
			{
				log_login = atoi(w2);
			}
			else if (strcasecmp(w1, "login_log_unknown_packets_filename") == 0)
			{
				safestrcpy(login_log_unknown_packets_filename, w2, sizeof(login_log_unknown_packets_filename));
			}
			else if (strcasecmp(w1, "save_unknown_packets") == 0)
			{
				save_unknown_packets = config_switch(w2);
			}
			else if (strcasecmp(w1, "display_parse_login") == 0)
			{
				display_parse_login = config_switch(w2); // 0: no, 1: yes
			}
			else if (strcasecmp(w1, "display_parse_admin") == 0)
			{
				display_parse_admin = config_switch(w2); // 0: no, 1: yes
			}
			else if (strcasecmp(w1, "display_parse_fromchar") == 0)
			{
				display_parse_fromchar = config_switch(w2); // 0: no, 1: yes (without packet 0x2714), 2: all packets
			}
			else if (strcasecmp(w1, "date_format") == 0)
			{	// note: never have more than 19 char for the date!
				memset(date_format, 0, sizeof(date_format));
				switch (atoi(w2))
				{
				case 0:
					strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
				}
			}
			else if (strcasecmp(w1, "min_level_to_connect") == 0)
			{
				min_level_to_connect = atoi(w2);
			}
			else if (strcasecmp(w1, "add_to_unlimited_account") == 0)
			{
				add_to_unlimited_account = config_switch(w2);
			} 
			else if (strcasecmp(w1, "start_limited_time") == 0)
			{
				start_limited_time = atoi(w2);
			}
			else if (strcasecmp(w1, "check_ip_flag") == 0)
			{
				check_ip_flag = config_switch(w2);
			} 
			else if (strcasecmp(w1, "order") == 0)
			{
				access_order = atoi(w2);
				if (strcasecmp(w2, "deny,allow") == 0 ||
				    strcasecmp(w2, "deny, allow") == 0) access_order = ACO_DENY_ALLOW;
				if (strcasecmp(w2, "allow,deny") == 0 ||
				    strcasecmp(w2, "allow, deny") == 0) access_order = ACO_ALLOW_DENY;
				if (strcasecmp(w2, "mutual-failture") == 0 ||
				    strcasecmp(w2, "mutual-failure") == 0) access_order = ACO_MUTUAL_FAILTURE;
			}
			else if (strcasecmp(w1, "allow") == 0)
			{
				if (strcasecmp(w2, "clear") == 0)
				{
					if (access_allow)
						delete[] access_allow;
					access_allow = NULL;
					access_allownum = 0;
				}
				else
				{
					if (strcasecmp(w2, "all") == 0)
					{	// reset all previous values
						if (access_allow)
							delete[] access_allow;
						// set to all
						access_allow = new char[ACO_STRSIZE];
						memset(access_allow,0,ACO_STRSIZE*sizeof(char));
						access_allownum = 1;
						access_allow[0] = '\0';
					} else if (w2[0] && !(access_allownum == 1 && access_allow[0] == '\0'))
					{	// don't add IP if already 'all'
						new_realloc(access_allow,access_allownum*ACO_STRSIZE,ACO_STRSIZE);
						safestrcpy(access_allow + (access_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
			} else if (strcasecmp(w1, "deny") == 0)
			{
				if (strcasecmp(w2, "clear") == 0)
				{
					if (access_deny)
					{
						delete[] access_deny;
					access_deny = NULL;
					access_denynum = 0;
					}
				}
				else
				{
					if (strcasecmp(w2, "all") == 0)
					{	// reset all previous values
						if (access_deny)
							delete[] access_deny;
						// set to all
						access_deny = new char[ACO_STRSIZE];
						access_denynum = 1;
						access_deny[0] = '\0';
					}
					else if (w2[0] && !(access_denynum == 1 && access_deny[0] == '\0'))
					{	// don't add IP if already 'all'
						new_realloc(access_deny, access_denynum*ACO_STRSIZE,ACO_STRSIZE);
						safestrcpy(access_deny + (access_denynum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
				// dynamic password error ban
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban") == 0) {
				dynamic_pass_failure_ban = config_switch(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_time") == 0) {
				dynamic_pass_failure_ban_time = atoi(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_many") == 0) {
				dynamic_pass_failure_ban_how_many = atoi(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_long") == 0) {
				dynamic_pass_failure_ban_how_long = atoi(w2);
			} else if (strcasecmp(w1, "import") == 0) {
				login_config_read(w2);
			} else if (strcasecmp(w1, "console") == 0) {
				console = config_switch(w2);
			} else if (strcasecmp(w1, "allowed_regs") == 0) {			
				allowed_regs = atoi(w2);
			} else if (strcasecmp(w1, "time_allowed") == 0) {
				time_allowed = atoi(w2);			
			}
		}
	}
	fclose(fp);
	return 0;
}

//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void) {
	if (admin_state != 0 && admin_state != 1) {
		ShowWarning("***WARNING: Invalid value for admin_state parameter -> set to 0 (no remote admin).\n");
		admin_state = 0;
	}

	if (admin_state == 1) {
		if (admin_pass[0] == '\0') {
			ShowWarning("***WARNING: Administrator password is void (admin_pass).\n");
		} else if (strcmp(admin_pass, "admin") == 0) {
			ShowWarning("***WARNING: You are using the default administrator password (admin_pass).\n");
			ShowMessage("            We highly recommend that you change it.\n");
		}
	}

	if (gm_pass[0] == '\0') {
		ShowWarning("***WARNING: 'To GM become' password is void (gm_pass).\n");
		ShowMessage("            We highly recommend that you set one password.\n");
	} else if (strcmp(gm_pass, "gm") == 0) {
		ShowWarning("***WARNING: You are using the default GM password (gm_pass).\n");
		ShowMessage("            We highly recommend that you change it.\n");
	}

	if (level_new_gm < 0 || level_new_gm > 99) {
		ShowWarning("***WARNING: Invalid value for level_new_gm parameter -> set to 60 (default).\n");
		level_new_gm = 60;
	}

	if (new_account_flag != 0 && new_account_flag != 1) {
		ShowWarning("***WARNING: Invalid value for new_account parameter -> set to 0 (no new account).\n");
		new_account_flag = 0;
	}

	if (loginaddress.port() < 1024) {
		ShowWarning("***WARNING: Invalid value for login_port parameter -> set to 6900 (default).\n");
		loginaddress.port() = 6900;
	}

	if (save_unknown_packets != 0 && save_unknown_packets != 1) {
		ShowWarning("WARNING: Invalid value for save_unknown_packets parameter -> set to 0-no save.\n");
		save_unknown_packets = 0;
	}

	if (display_parse_login != 0 && display_parse_login != 1) { // 0: no, 1: yes
		ShowWarning("***WARNING: Invalid value for display_parse_login parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_login = 0;
	}

	if (display_parse_admin != 0 && display_parse_admin != 1) { // 0: no, 1: yes
		ShowWarning("***WARNING: Invalid value for display_parse_admin parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_admin = 0;
	}

	if (display_parse_fromchar < 0 || display_parse_fromchar > 2) { // 0: no, 1: yes (without packet 0x2714), 2: all packets
		ShowWarning("***WARNING: Invalid value for display_parse_fromchar parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_fromchar = 0;
	}

	if (min_level_to_connect < 0) { // 0: all players, 1-99 at least gm level x
		ShowWarning("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 0 (any player).\n");
		min_level_to_connect = 0;
	} else if (min_level_to_connect > 99) { // 0: all players, 1-99 at least gm level x
		ShowWarning("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 99 (only GM level 99).\n");
		min_level_to_connect = 99;
	}

	if (add_to_unlimited_account != 0 && add_to_unlimited_account != 1) { // 0: no, 1: yes
		ShowWarning("***WARNING: Invalid value for add_to_unlimited_account parameter\n");
		ShowMessage("            -> set to 0 (impossible to add a time to an unlimited account).\n");
		add_to_unlimited_account = 0;
	}

	if (start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowWarning("***WARNING: Invalid value for start_limited_time parameter\n");
		ShowMessage("            -> set to -1 (new accounts are created with unlimited time).\n");
		start_limited_time = -1;
	}

	if (check_ip_flag != 0 && check_ip_flag != 1) { // 0: no, 1: yes
		ShowWarning("***WARNING: Invalid value for check_ip_flag parameter\n");
		ShowMessage("            -> set to 1 (check players ip between login-server & char-server).\n");
		check_ip_flag = 1;
	}

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowWarning("***WARNING: The IP security order is 'deny,allow' (allow if not deny).\n");
			ShowMessage("            And you refuse ALL IP.\n");
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			ShowWarning("***WARNING: The IP security order is 'allow,deny' (deny if not allow).\n");
			ShowMessage("            But, NO IP IS AUTHORISED!\n");
		}
	} else { // ACO_MUTUAL_FAILTURE
		if (access_allownum == 0) {
			ShowWarning("***WARNING: The IP security order is 'mutual-failture'\n");
			ShowMessage("            (allow if in the allow list and not in the deny list).\n");
			ShowMessage("            But, NO IP IS AUTHORISED!\n");
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowWarning("***WARNING: The IP security order is mutual-failture\n");
			ShowMessage("            (allow if in the allow list and not in the deny list).\n");
			ShowMessage("            But, you refuse ALL IP!\n");
		}
	}

	if (dynamic_pass_failure_ban != 0) {
		if (dynamic_pass_failure_ban_time < 1) {
			ShowWarning("***WARNING: Invalid value for dynamic_pass_failure_ban_time (%d) parameter\n", dynamic_pass_failure_ban_time);
			ShowMessage("            -> set to 5 (5 minutes to look number of invalid passwords.\n");
			dynamic_pass_failure_ban_time = 5;
		}
		if (dynamic_pass_failure_ban_how_many < 1) {
			ShowWarning("***WARNING: Invalid value for dynamic_pass_failure_ban_how_many (%d) parameter\n", dynamic_pass_failure_ban_how_many);
			ShowMessage("            -> set to 3 (3 invalid passwords before to temporarily ban.\n");
			dynamic_pass_failure_ban_how_many = 3;
		}
		if (dynamic_pass_failure_ban_how_long < 1) {
			ShowWarning("***WARNING: Invalid value for dynamic_pass_failure_ban_how_long (%d) parameter\n", dynamic_pass_failure_ban_how_long);
			ShowMessage("            -> set to 1 (1 minute of temporarily ban.\n");
			dynamic_pass_failure_ban_how_long = 1;
		}
	}

	return;
}

//-------------------------------
// Save configuration in log file
//-------------------------------
void save_config_in_log(void) {
	int i;

	// a newline in the log...
	login_log("");
	login_log("The login-server starting..." RETCODE);

	// save configuration in log file
	login_log("The configuration of the server is set:" RETCODE);

	if (admin_state != 1)
		login_log("- with no remote administration." RETCODE);
	else if (admin_pass[0] == '\0')
		login_log("- with a remote administration with a VOID password." RETCODE);
	else if (strcmp(admin_pass, "admin") == 0)
		login_log("- with a remote administration with the DEFAULT password." RETCODE);
	else
		login_log("- with a remote administration with the password of %d character(s)." RETCODE, strlen(admin_pass));
	if (access_ladmin_allownum == 0 || (access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0')) {
		login_log("- to accept any IP for remote administration" RETCODE);
	} else {
		login_log("- to accept following IP for remote administration:" RETCODE);
		for(i = 0; i < access_ladmin_allownum; ++i)
			login_log("  %s" RETCODE, (char *)(access_ladmin_allow + i * ACO_STRSIZE));
	}

	if (gm_pass[0] == '\0')
		login_log("- with a VOID 'To GM become' password (gm_pass)." RETCODE);
	else if (strcmp(gm_pass, "gm") == 0)
		login_log("- with the DEFAULT 'To GM become' password (gm_pass)." RETCODE);
	else
		login_log("- with a 'To GM become' password (gm_pass) of %d character(s)." RETCODE, strlen(gm_pass));
	if (level_new_gm == 0)
		login_log("- to refuse any creation of GM with @gm." RETCODE);
	else
		login_log("- to create GM with level '%d' when @gm is used." RETCODE, level_new_gm);

	if (new_account_flag == 1)
		login_log("- to ALLOW new users (with _F/_M)." RETCODE);
	else
		login_log("- to NOT ALLOW new users (with _F/_M)." RETCODE);
	login_log("- with port: %d." RETCODE, loginaddress.port());

	if (use_md5_passwds == 0)
		login_log("- to save password in plain text." RETCODE);
	else
		login_log("- to save password with MD5 encrypting." RETCODE);

	// not necessary to log the 'login_log_filename', we are inside :)

	login_log("- with the unknown packets file name: '%s'." RETCODE, login_log_unknown_packets_filename);
	if (save_unknown_packets)
		login_log("- to SAVE all unkown packets." RETCODE);
	else
		login_log("- to SAVE only unkown packets sending by a char-server or a remote administration." RETCODE);
	if (display_parse_login)
		login_log("- to display normal parse packets on console." RETCODE);
	else
		login_log("- to NOT display normal parse packets on console." RETCODE);
	if (display_parse_admin)
		login_log("- to display administration parse packets on console." RETCODE);
	else
		login_log("- to NOT display administration parse packets on console." RETCODE);
	if (display_parse_fromchar)
		login_log("- to display char-server parse packets on console." RETCODE);
	else
		login_log("- to NOT display char-server parse packets on console." RETCODE);

	if (min_level_to_connect == 0) // 0: all players, 1-99 at least gm level x
		login_log("- with no minimum level for connection." RETCODE);
	else if (min_level_to_connect == 99)
		login_log("- to accept only GM with level 99." RETCODE);
	else
		login_log("- to accept only GM with level %d or more." RETCODE, min_level_to_connect);

	if (add_to_unlimited_account)
		login_log("- to authorize adjustment (with timeadd ladmin) on an unlimited account." RETCODE);
	else
		login_log("- to refuse adjustment (with timeadd ladmin) on an unlimited account. You must use timeset (ladmin command) before." RETCODE);

	if (start_limited_time < 0)
		login_log("- to create new accounts with an unlimited time." RETCODE);
	else if (start_limited_time == 0)
		login_log("- to create new accounts with a limited time: time of creation." RETCODE);
	else
		login_log("- to create new accounts with a limited time: time of creation + %d second(s)." RETCODE, start_limited_time);

	if (check_ip_flag)
		login_log("- with control of players IP between login-server and char-server." RETCODE);
	else
		login_log("- to not check players IP between login-server and char-server." RETCODE);

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 0) {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse no IP." RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; ++i)
				login_log("  %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_allownum == 1 && access_allow[0] == '\0') {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). You authorise ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). Authorised IP are:" RETCODE);
			for(i = 0; i < access_allownum; ++i)
				login_log("  %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
		}
	} else { // ACO_MUTUAL_FAILTURE
		login_log("- with the IP security order: 'mutual-failture' (allow if in the allow list and not in the deny list)." RETCODE);
		if (access_allownum == 0) {
			login_log("  But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("  But, you refuse ALL IP!" RETCODE);
		} else {
			if (access_allownum == 1 && access_allow[0] == '\0') {
				login_log("  You authorise ALL IP." RETCODE);
			} else {
				login_log("  Authorised IP are:" RETCODE);
				for(i = 0; i < access_allownum; ++i)
					login_log("    %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
			}
			login_log("  Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; ++i)
				login_log("    %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}

		// dynamic password error ban
		if (dynamic_pass_failure_ban == 0)
			login_log("- with NO dynamic password error ban." RETCODE);
		else {
			ShowMessage("- with a dynamic password error ban:" RETCODE);
			ShowMessage("  After %d invalid password in %d minutes" RETCODE, dynamic_pass_failure_ban_how_many, dynamic_pass_failure_ban_time);
			ShowMessage("  IP is banned for %d minutes" RETCODE, dynamic_pass_failure_ban_how_long);
		}
	}
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void) {
	size_t i;
	ShowStatus("Terminating...\n");
	///////////////////////////////////////////////////////////////////////////

	account_db.close();

	if(access_ladmin_allow) delete[] access_ladmin_allow;
	if(access_allow) delete[] access_allow;
	if(access_deny) delete[] access_deny;

	///////////////////////////////////////////////////////////////////////////
	// delete sessions
	for (i = 0; i < fd_max; ++i) {
		if (session[i])
			session_Delete(i);
	}
	// clear externaly stored fd's
	for (i = 0; i < MAX_SERVERS; ++i) {
			server[i].fd = -1;
		}
	login_fd = -1;
	///////////////////////////////////////////////////////////////////////////
	login_log("----End of login-server (normal end with closing of all files)." RETCODE);
	if(log_fp)
		fclose(log_fp);
	///////////////////////////////////////////////////////////////////////////
	ShowStatus("Successfully terminated.\n");
}

//------------------------------
// Main function of login-server
//------------------------------
unsigned char getServerType()
{
	return ATHENA_SERVER_LOGIN | ATHENA_SERVER_CORE;
}


int check_connect_login_port(int tid, unsigned long tick, int id, basics::numptr data)
{
	if( !session_isActive(login_fd) )
	{	// the listen port was dropped, open it new
		login_fd = make_listen(loginaddress.addr(),loginaddress.port());

		if(login_fd>=0)
		{
			login_log("The login-server is ready (Server is listening on port %d)." RETCODE, loginaddress.port());
			ShowStatus("The login-server is "CL_BT_GREEN"ready"CL_NORM" (Server is listening on port %d).\n", loginaddress.port());
		}
		else
			ShowError("open listening socket on port '"CL_WHITE"%d"CL_RESET"' failed.\n\n", loginaddress.port());
	}
	return 0;
}

int do_init(int argc, char **argv)
{
	int i;
	// read login-server configuration
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	basics::CParamBase::loadFile((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
	save_config_in_log(); // not before, because log file name can be changed
	
	if (!account_db.init( (argc > 1) ? argv[1] : LOGIN_CONF_NAME ))
	{
		core_stoprunning();
		return 0;
	}

	for(i = 0; i < MAX_SERVERS; ++i)
		server[i].fd = -1;

	set_defaultparse(parse_login);

	if(console) {
		set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	add_timer_func_list(check_connect_login_port, "check_connect_login_port");
	add_timer_interval(gettick() + 1000, 10 * 1000, check_connect_login_port, 0, 0);

	return 0;
}





