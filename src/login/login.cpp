// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "baseparam.h"
#include "baseipfilter.h"

#include "login.h"
#include "admin.h"

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
bool mfreg_enabled = false;			///< M/F registration allowed
unsigned long mfreg_time=10;		///< time in seconds between registrations (10)
unsigned long new_reg_tick=0;		///< internal tickcounter for M/F registration



///////////////////////////////////////////////////////////////////////////////
// ladmin access
basics::CParam<basics::iprulelist> ladminallowip("ladminallowip");

///////////////////////////////////////////////////////////////////////////////
// connection stuff
int min_level_to_connect = 0;			// minimum level of player/GM (0: player, 1-99: gm) to connect on the server
bool check_ip_flag = true;				// It's to check IP of a player between login-server and char-server (part of anti-hacking system)

///////////////////////////////////////////////////////////////////////////////
// enable console
bool console = false;

///////////////////////////////////////////////////////////////////////////////
// gm stuff
char gm_pass[64] = "";
int level_new_gm = 60;


///////////////////////////////////////////////////////////////////////////////
// logging (txt)
//!! todo compound to class with sql logging
FILE *log_fp = NULL;
bool log_login = true;
char login_log_filename[1024] = "log/login.log";
char date_format[32] = "%Y-%m-%d %H:%M:%S";

bool display_parse_login = 0;
int display_parse_fromchar = 0; // 0: no, 1: yes (without packet 0x2714), 2: all packets

//------------------------------
// Writing function of logs file
//------------------------------
int login_log(char *fmt, ...)
{
	if(log_login)
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
				sprintf(tmpstr + strlen(tmpstr), ".%03ld: %s", (long)(tv.tv_usec / 1000), fmt);

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
bool save_unknown_packets = false;
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
			fprintf(logfp, "%s.%03ld: receiving of an unknown packet -> disconnection" RETCODE, tmpstr, (long)(tv.tv_usec / 1000));
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

bool check_encryped(const char* str1, const char* str2, const char* passwd)
{
	char md5str[64], md5bin[32];
	snprintf(md5str, sizeof(md5str), "%s%s", str1, str2);
	md5str[sizeof(md5str)-1] = '\0';
	MD5_String2binary(md5str, md5bin);

	return (0==memcmp(passwd, md5bin, 16));
}

bool check_password(login_session_data* ld, int passwdenc, const char* passwd, const char* refpass)
{	
	if(passwdenc == 0)
	{
		return (0==strcmp(passwd, refpass));
	}
	else if (ld)
	{
		// password mode set to 1
		// (md5key, refpass) enable with <passwordencrypt></passwordencrypt>
		// password mode set to 2
		// (refpass, md5key) enable with <passwordencrypt2></passwordencrypt2>
		
		bool encpasswdok = 
			((passwdenc&0x01) && check_encryped(ld->md5key, refpass, passwd)) ||
			((passwdenc&0x02) && check_encryped(refpass, ld->md5key, passwd));
	
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
		snprintf(str+csz, sz-csz, ".%03ld", (long)(tv.tv_usec / 1000));			// add milliseconds
		str[sz-1] = 0;	// system snprintf does not add EOS when buffer limit is reached
	}
	return str;
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
				(!check_ip_flag || account.client_ip == RFIFOLIP(fd,15) || 
				(account.client_ip.isBindable() && basics::ipaddress(RFIFOLIP(fd,15)).isBindable()) ) )
			{	// found and verified
				// have a bypass for connections from local machine
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
				safestrcpy(account.email, sizeof(account.email), new_email);
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
					safestrcpy(account.account_reg2[j].str, sizeof(account.account_reg2[j].str), (char*)RFIFOP(fd,p));
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
		if (display_parse_login)
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
				int namelen = strlen(userid) - 2;
				if( new_account_flag && mfreg_enabled &&
					namelen >= 4 &&
					passwdenc == 0 && 
					userid[namelen] == '_' &&
					(basics::upcase(userid[namelen+1]) == 'F' || 
					 basics::upcase(userid[namelen+1]) == 'M' ) &&
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
						new_reg_tick = gettick() + 1000*mfreg_time;
						userid[namelen] = '\0';
						ok = !account_db.existAccount(userid) &&
							account_db.insertAccount(userid, passwd, (basics::upcase(userid[namelen+1]) == 'M'),"a@a.com", account);
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
			if( !ladminallowip->exists( session[fd]->client_ip ) )
			{
				login_log("'ladmin'-login: Connection in administration mode refused: IP isn't authorised (ladmin_allow, ip: %s)." RETCODE, ip_str);
			}
			else
			{
				login_session_data *ld = (login_session_data*)session[fd]->user_session;
				if (RFIFOW(fd,2) == 0)
				{	// non encrypted password
					char password[64];
					safestrcpy(password, sizeof(password), (char*)RFIFOP(fd,4));
					remove_control_chars(password);
					// If remote administration is enabled and password sent by client matches password read from login server configuration file
					if( !admin_state )
						login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
					else if( 0!=strcmp(password, admin_pass) )
						login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
					else
					{
						login_log("'ladmin'-login: Connection in administration mode accepted (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
						ShowStatus("Connection of a remote administration accepted (non encrypted password).\n");
						WFIFOB(fd,2) = 0;
						session[fd]->func_parse = parse_admin;
						session[fd]->rdata_tick = 0;
					}					
				}
				else
				{	// encrypted password
					if (!ld)
						ShowError("'ladmin'-login: error! MD5 key not created/requested for an administration login.\n");
					else
					{
						char md5str[64], md5bin[32];
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
						if( !admin_state )
							login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (encrypted password, ip: %s)" RETCODE, ip_str);
						else if( 0!=memcmp(md5bin, RFIFOP(fd,4), 16) )
							login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (encrypted password, ip: %s)" RETCODE, ip_str);
						else
						{
							login_log("'ladmin'-login: Connection in administration mode accepted (encrypted password, ip: %s)" RETCODE, ip_str);
							ShowStatus("Connection of a remote administration accepted (encrypted password).\n");
							WFIFOB(fd,2) = 0;
							session[fd]->func_parse = parse_admin;
							session[fd]->rdata_tick = 0;
						}
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

void send_reject_packet(int fd)
{
	//login_log("Connection refused: IP isn't authorised (deny/allow, ip: %s)." RETCODE, ip_str);
	WFIFOW(fd,0) = 0x6a;
	WFIFOB(fd,2) = 3; // 3 = Rejected from Server
	WFIFOB(fd,3) = 0;
	WFIFOSET(fd,23);
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
		ShowError("Login Configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
		return 1;
	}
	
	while( fgets(line, sizeof(line), fp) )
	{
		if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
		{
			remove_control_chars(w1);
			basics::itrim(w1);
			if(!*w1) continue;
			
			remove_control_chars(w2);
			basics::itrim(w2);
			
			if (strcasecmp(w1, "admin_state") == 0)
			{
				admin_state = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "admin_pass") == 0)
			{
				safestrcpy(admin_pass, sizeof(admin_pass), w2);
			}
			else if (strcasecmp(w1, "gm_pass") == 0)
			{
				safestrcpy(gm_pass, sizeof(gm_pass), w2);
			}
			else if (strcasecmp(w1, "level_new_gm") == 0)
			{
				level_new_gm = atoi(w2);
			}
			else if (strcasecmp(w1, "new_account") == 0)
			{
				new_account_flag = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "login_ip") == 0)
			{
				loginaddress = w2;
				ShowMessage("Login server IP address : %s -> %s\n", w2, loginaddress.tostring(NULL));
			}
			else if (strcasecmp(w1, "login_port") == 0)
			{
				loginaddress.port() = basics::config_switch<ushort>(w2);
			}
			else if (strcasecmp(w1, "login_log_filename") == 0)
			{
				safestrcpy(login_log_filename, sizeof(login_log_filename), w2);
			}
			else if (strcasecmp(w1, "log_login") == 0)
			{
				log_login = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "login_log_unknown_packets_filename") == 0)
			{
				safestrcpy(login_log_unknown_packets_filename, sizeof(login_log_unknown_packets_filename), w2);
			}
			else if (strcasecmp(w1, "save_unknown_packets") == 0)
			{
				save_unknown_packets = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "display_parse_login") == 0)
			{
				display_parse_login = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "display_parse_admin") == 0)
			{
				display_parse_admin = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "display_parse_fromchar") == 0)
			{
				display_parse_fromchar = basics::config_switch<int>(w2,0,2); // 0: no, 1: yes (without packet 0x2714), 2: all packets
			}
			else if (strcasecmp(w1, "date_format") == 0)
			{	// note: never have more than 19 char for the date!
				memset(date_format, 0, sizeof(date_format));
				switch (atoi(w2))
				{
				case 0:
					safestrcpy(date_format, sizeof(date_format), "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					safestrcpy(date_format, sizeof(date_format), "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					safestrcpy(date_format, sizeof(date_format), "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					safestrcpy(date_format, sizeof(date_format), "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
				}
			}
			else if (strcasecmp(w1, "min_level_to_connect") == 0)
			{
				min_level_to_connect = basics::config_switch<int>(w2);
			}
			else if (strcasecmp(w1, "check_ip_flag") == 0)
			{
				check_ip_flag = basics::config_switch<bool>(w2);
			} 
			else if (strcasecmp(w1, "import") == 0)
			{
				login_config_read(w2);
			}
			else if (strcasecmp(w1, "console") == 0)
			{
				console = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "mfreg_enabled") == 0)
			{			
				mfreg_enabled = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "mfreg_time") == 0)
			{
				mfreg_time = basics::config_switch<unsigned long>(w2);			
			}
		}
	}
	fclose(fp);
	ShowStatus("Done reading Login Configuration '"CL_WHITE"%s"CL_RESET"'.\n", cfgName);
	return 0;
}

//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void)
{
	if(admin_state)
	{
		if (admin_pass[0] == '\0')
		{
			ShowWarning("***WARNING: Administrator password is void (admin_pass).\n");
		}
		else if (strcmp(admin_pass, "admin") == 0)
		{
			ShowWarning("***WARNING: You are using the default administrator password (admin_pass).\n");
			ShowMessage("            We highly recommend that you change it.\n");
		}
	}

	if (gm_pass[0] == '\0')
	{
		ShowWarning("***WARNING: 'To GM become' password is void (gm_pass).\n");
		ShowMessage("            We highly recommend that you set one password.\n");
	} else if (strcmp(gm_pass, "gm") == 0)
	{
		ShowWarning("***WARNING: You are using the default GM password (gm_pass).\n");
		ShowMessage("            We highly recommend that you change it.\n");
	}

	if (level_new_gm < 0 || level_new_gm > 99)
	{
		ShowWarning("***WARNING: Invalid value for level_new_gm parameter -> set to 60 (default).\n");
		level_new_gm = 60;
	}

	if (loginaddress.port() < 1024)
	{
		ShowWarning("***WARNING: Invalid value for login_port parameter -> set to 6900 (default).\n");
		loginaddress.port() = 6900;
	}

	if (min_level_to_connect < 0)
	{	// 0: all players, 1-99 at least gm level x
		ShowWarning("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 0 (any player).\n");
		min_level_to_connect = 0;
	}
	else if (min_level_to_connect > 99)
	{	// 0: all players, 1-99 at least gm level x
		ShowWarning("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 99 (only GM level 99).\n");
		min_level_to_connect = 99;
	}

	return;
}

//-------------------------------
// Save configuration in log file
//-------------------------------
void save_config_in_log(void)
{
	// a newline in the log...
	login_log("");
	login_log("The login-server starting..." RETCODE);

	// save configuration in log file
	login_log("The configuration of the server is set:" RETCODE);

	if( !admin_state )
		login_log("- with no remote administration." RETCODE);
	else if (admin_pass[0] == '\0')
		login_log("- with a remote administration with a VOID password." RETCODE);
	else if (strcmp(admin_pass, "admin") == 0)
		login_log("- with a remote administration with the DEFAULT password." RETCODE);
	else
		login_log("- with a remote administration with the password of %d character(s)." RETCODE, strlen(admin_pass));

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

	if( new_account_flag )
		login_log("- to ALLOW new users (with _F/_M)." RETCODE);
	else
		login_log("- to NOT ALLOW new users (with _F/_M)." RETCODE);
	login_log("- with port: %d." RETCODE, loginaddress.port());


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

	if (check_ip_flag)
		login_log("- with control of players IP between login-server and char-server." RETCODE);
	else
		login_log("- to not check players IP between login-server and char-server." RETCODE);
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void) {
	size_t i;
	ShowStatus("Terminating...\n");
	///////////////////////////////////////////////////////////////////////////

	account_db.close();

	///////////////////////////////////////////////////////////////////////////
	// delete sessions
	for (i = 0; i < fd_max; ++i)
	{
		if (session[i])
			session_Delete(i);
	}
	// clear externaly stored fd's
	for (i = 0; i < MAX_SERVERS; ++i)
	{
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
	login_config_read((argc > 1 && basics::is_file(argv[1])) ? argv[1] : LOGIN_CONF_NAME);
	basics::CParamBase::loadFile((argc > 1 && basics::is_file(argv[1])) ? argv[1] : LOGIN_CONF_NAME);
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
	save_config_in_log(); // not before, because log file name can be changed
	
	if (!account_db.init( (argc > 1 && basics::is_file(argv[1])) ? argv[1] : LOGIN_CONF_NAME ))
	{
		core_stoprunning();
		return 0;
	}

	for(i = 0; i < MAX_SERVERS; ++i)
		server[i].fd = -1;

	set_defaultparse(parse_login);

	if(console)
	{
		set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	add_timer_func_list(check_connect_login_port, "check_connect_login_port");
	add_timer_interval(gettick() + 1000, 10 * 1000, check_connect_login_port, 0, 0);

	return 0;
}
