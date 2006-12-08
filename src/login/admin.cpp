#include "login.h"
#include "admin.h"


///////////////////////////////////////////////////////////////////////////////
// admin/gm stuff
bool admin_state = false;
char admin_pass[24] = "";
bool display_parse_admin = false;



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
		if (display_parse_admin)
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
				
				safestrcpy(account.passwd, sizeof(account.passwd), pass);
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
					safestrcpy(account.error_message, sizeof(account.error_message), error_message);
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
					safestrcpy(account.email, sizeof(account.email), email);
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
//					safestrcpy(account.memo, sizeof(account.memo), buf);
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
					char message[32000];
					size_t sz = RFIFOL(fd,4);
					if(sz > sizeof(message)) sz = sizeof(message)-1;
					WFIFOW(fd,2) = 0;
					safestrcpy(message, sizeof(message), (char*)RFIFOP(fd,8));
					message[sizeof(message)-1] = '\0';
					remove_control_chars(message);
					if (RFIFOW(fd,2) == 0)
						login_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s, ip: %s)" RETCODE,
							message, ip_str);
					else
						login_log("'ladmin': Receiving a message for broadcast (message (in blue): %s, ip: %s)" RETCODE,
							message, ip_str);
					// send same message to all char-servers (no answer)
					// send directly from receive buffer, no copy
					RFIFOW(fd,0) = 0x2726;
					charif_sendallwos(-1, RFIFOP(fd,0), 8+sz);
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
				if( timestamp == 0 )
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
				safestrcpy((char*)WFIFOP(fd,7), 24, "");
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


