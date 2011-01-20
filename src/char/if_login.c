// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/cookie.h"
#include "../common/db.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "chardb.h"
#include "int_guild.h"
#include "int_storage.h" // inter_storage_delete()
#include "inter.h"
#include "if_client.h"
#include "if_login.h"
#include "if_map.h"
#include "charserverdb.h"
#include "online.h"
#include <stdio.h>

static int send_accounts_tologin(int tid, unsigned int tick, int id, intptr data);

//temporary imports
extern CharServerDB* charserver;
int login_fd = -1;
static enum{LOGINIF_NOT_READY, LOGINIF_LOGIN, LOGINIF_READY} loginif_state = LOGINIF_NOT_READY;
static int loginif_connect_timer = INVALID_TIMER; //< check_connect_login_server
static struct s_cookie cookie; //< session cookie from login-server

#include "char.h"
extern uint32 login_ip;
extern uint32 char_ip;
extern DBMap* auth_db;
extern void char_auth_ok(int fd, struct char_session_data *sd);
extern int disconnect_player(int account_id);
extern int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);
extern int count_users(void);


/// Cookie timeout timer.
/// Starts when the char-server disconnects, if we have a session cookie.
/// Canceled when a successfull reconnect is achieved.
void loginif_cookie_timeout_callback(intptr data)
{
	ShowWarning("Login-server session expired...");
	cookie_set(&cookie, 0, NULL);
	loginif_reset();
}


/// Sends a cookie-related message to the login-server.
/// 0=ack, 1=request, 2=release
void loginif_send_cookie_msg(int fd, int msg)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x271c;
	WFIFOB(fd,2) = msg;
	WFIFOSET(fd,3);
}


/// Resets all the data.
void loginif_reset(void)
{
	int id;
	cookie_set(&cookie, 0, NULL);
	// TODO kick everyone out and reset everything or wait for connect and try to reaquire locks [FlavioJS]
	for( id = 0; id < ARRAYLENGTH(server); ++id )
		mapif_server_reset(id);
	flush_fifos();
	exit(EXIT_FAILURE);
}


/// Checks the conditions for the server to stop.
/// Releases the cookie when all characters are saved.
/// If all the conditions are met, it stops the core loop.
void loginif_check_shutdown(void)
{
	if( runflag != CHARSERVER_ST_SHUTDOWN )
		return;
	// TODO wait for locks to be released [FlavioJS]
	if( !cookie_expired(&cookie) )
	{
		if( loginif_is_connected() )
			loginif_send_cookie_msg(login_fd,2);// release
		return;
	}
	runflag = CORE_ST_STOP;
}


/// Called when the connection to Login Server is disconnected.
void loginif_on_disconnect(void)
{
	loginif_state = LOGINIF_NOT_READY;

	loginif_connect_timer_start();
	if( cookie_expired(&cookie) )
	{
		ShowStatus("Connection to Login Server lost.\n");
		loginif_reset();
	}
	else
	{
		ShowWarning("Connection to Login Server lost, trying to resume...\n");
		cookie_timeout_start(&cookie);
	}
}


/// Called when all the connection steps are completed.
void loginif_on_ready(void)
{
	int i;
	loginif_state = LOGINIF_READY;
	loginif_connect_timer_stop();
	loginif_check_shutdown();

	//Send online accounts to login server.
	send_accounts_tologin(-1, gettick(), 0, 0);

	// if no map-server already connected, display a message...
	ARR_FIND( 0, MAX_MAP_SERVERS, i, server[i].fd > 0 && server[i].map[0] );
	if( i == MAX_MAP_SERVERS )
		ShowStatus("Awaiting maps from map-server.\n");
}


int parse_fromlogin(int fd)
{
	struct char_session_data* sd = NULL;
	int i;

	// only process data from the login-server
	if( fd != login_fd )
	{
		ShowDebug("parse_fromlogin: Disconnecting invalid session #%d (is not the login-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		login_fd = -1;
		loginif_on_disconnect();
		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{

		// acknowledgement of connect-to-loginserver request
		case 0x2711:
			if (RFIFOREST(fd) < 3)
				return 0;

			if( RFIFOB(fd,2) ) {
				if( cookie_expired(&cookie) ) {
					ShowError("Connection to login-server failed (error=%d).\n"
						"Please make sure that\n"
						"- the char-server's userid/passwd settings match an existing account\n"
						"- the account's gender is set to 'S'\n"
						"- the account's id is less than MAX_SERVERS (default:30)\n", RFIFOB(fd,2));
				}
				else
				{
					ShowError("Reconnection to login-server failed (error=%d).\n", RFIFOB(fd,2));
				}
				loginif_state = LOGINIF_NOT_READY;
				set_eof(fd);
				return 0;
			} else {
				ShowStatus("Connected to login-server (connection #%d).\n", fd);
				cookie_timeout_stop(&cookie);
				loginif_on_ready();
			}
			RFIFOSKIP(fd,3);
		break;

		// acknowledgement of account authentication request
		case 0x2713:
			if (RFIFOREST(fd) < 25)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			uint32 login_id1 = RFIFOL(fd,6);
			uint32 login_id2 = RFIFOL(fd,10);
			uint8 sex = RFIFOB(fd,14);
			uint8 result = RFIFOB(fd,15);
			int request_id = RFIFOL(fd,16);
			uint32 version = RFIFOL(fd,20);
			uint8 clienttype = RFIFOB(fd,24);
			RFIFOSKIP(fd,25);

			if( session_isActive(request_id) && (sd=(struct char_session_data*)session[request_id]->session_data) &&
				!sd->auth && sd->account_id == account_id && sd->login_id1 == login_id1 && sd->login_id2 == login_id2 && sd->sex == sex )
			{
				int client_fd = request_id;
				sd->version = version;
				sd->clienttype = clienttype;
				switch( result )
				{
				case 0:// ok
					char_auth_ok(client_fd, sd);
					break;
				case 1:// auth failed
					WFIFOHEAD(client_fd,3);
					WFIFOW(client_fd,0) = 0x6c;
					WFIFOB(client_fd,2) = 0;// rejected from server
					WFIFOSET(client_fd,3);
					break;
				}
			}
		}
		break;

		case 0x2717: // account data
			if (RFIFOREST(fd) < 51)
				return 0;

			// find the authenticated session with this account id
			ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->auth && sd->account_id == RFIFOL(fd,2) );
			if( i < fd_max )
			{
				memcpy(sd->email, RFIFOP(fd,6), 40);
				sd->expiration_time = (time_t)RFIFOL(fd,46);
				sd->gmlevel = RFIFOB(fd,50);

				// continued from char_auth_ok...
				if( char_config.max_connect_user != 0 && count_users() >= char_config.max_connect_user && sd->gmlevel < char_config.gm_allow_level )
				{
					// refuse connection (over populated)
					WFIFOW(i,0) = 0x6c;
					WFIFOW(i,2) = 0;
					WFIFOSET(i,3);
				}
				else
				{
					// send characters to player
					mmo_char_send006b(i, sd);
				}
			}
			RFIFOSKIP(fd,51);
		break;

		// login-server alive packet
		case 0x2718:
			if (RFIFOREST(fd) < 2)
				return 0;
			RFIFOSKIP(fd,2);
		break;

		case 0x271b: // receive session cookie
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
			if( RFIFOW(fd,2) < 8 )
			{// invalid packet
				set_eof(fd);
				return 0;
			}
		{
			uint16 len = RFIFOW(fd,2)-8;
			uint32 timeout = RFIFOL(fd,4);
			const char* data = (const char*)RFIFOP(fd,8);

			if( len > MAX_COOKIE_LEN )
			{
				ShowDebug("parse_fromlogin: cookie is too long (len=%d max=%d).\n", len, MAX_COOKIE_LEN);
				cookie.timeout = 0;
				cookie_set(&cookie, 0, NULL);
				loginif_send_cookie_msg(fd,1);// request
			}
			else
			{
				cookie.timeout = timeout;
				cookie_set(&cookie, len, data);
				loginif_send_cookie_msg(fd,0);// ack
				loginif_check_shutdown();
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		}
		break;

		// changesex reply
		case 0x2723:
			if (RFIFOREST(fd) < 7)
				return 0;
		{
			unsigned char buf[7];

			int acc = RFIFOL(fd,2);
			int sex = RFIFOB(fd,6);
			RFIFOSKIP(fd,7);

			if( acc > 0 )
			{// TODO: Is this even possible?
				CharDB* chars = charserver->chardb(charserver);
				SkillDB* skills = charserver->skilldb(charserver);
				StorageDB* storages = charserver->storagedb(charserver);
				struct mmo_charstatus cd;
				CSDBIterator* it;
				int char_id;
				int j;

				struct auth_node* node = (struct auth_node*)idb_get(auth_db, acc);
				if( node != NULL )
					node->sex = sex;

				// process every char on this account
				it = chars->characters(chars, acc);
				while( it->next(it, &char_id) )
				{
					chars->load_num(chars, &cd, char_id);
					skills->load(skills, &cd.skill, char_id);
					storages->load(storages, cd.inventory, MAX_INVENTORY, STORAGE_INVENTORY, char_id);
					cd.sex = sex;

					if (cd.class_ == JOB_BARD || cd.class_ == JOB_DANCER ||
					    cd.class_ == JOB_CLOWN || cd.class_ == JOB_GYPSY ||
					    cd.class_ == JOB_BABY_BARD || cd.class_ == JOB_BABY_DANCER)
					{
						// job modification
						if( cd.class_ == JOB_BARD || cd.class_ == JOB_DANCER )
							cd.class_ = (sex) ? JOB_BARD : JOB_DANCER;
						else
						if( cd.class_ == JOB_CLOWN || cd.class_ == JOB_GYPSY )
							cd.class_ = (sex) ? JOB_CLOWN : JOB_GYPSY;
						else
						if( cd.class_ == JOB_BABY_BARD || cd.class_ == JOB_BABY_DANCER )
							cd.class_ = (sex) ? JOB_BABY_BARD : JOB_BABY_DANCER;
						
						// reset points in bard/dancer song skills
						for( j = 315; j <= 330; j++ )
						{
							if( cd.skill[j].id > 0 && !cd.skill[j].flag )
							{
								cd.skill_point = max(cd.skill_point, cd.skill_point + cd.skill[j].lv); // overflow check
								cd.skill[j].id = 0;
								cd.skill[j].lv = 0;
							}
						}
					}

					// to avoid any problem with equipment and invalid sex, equipment is unequiped.
					for( j = 0; j < MAX_INVENTORY; j++ )
						cd.inventory[j].equip = 0;

					cd.weapon = 0;
					cd.shield = 0;
					cd.head_top = 0;
					cd.head_mid = 0;
					cd.head_bottom = 0;

					//If there is a guild, update the guild_member data [Skotlex]
					if( cd.guild_id > 0 )
						inter_guild_sex_changed(cd.guild_id, acc, cd.char_id, sex);

					chars->save(chars, &cd);
					skills->save(skills, &cd.skill, char_id);
					storages->save(storages, cd.inventory, MAX_INVENTORY, STORAGE_INVENTORY, char_id);
				}
				it->destroy(it);

				// disconnect player if online on char-server
				disconnect_player(acc);
			}

			// notify all mapservers about this change
			WBUFW(buf,0) = 0x2b0d;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			mapif_sendall(buf, 7);
		}
		break;

		// reply to an account_reg2 registry request
		case 0x2729:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

		{	//Receive account_reg2 registry, forward to map servers.
			unsigned char buf[13+ACCOUNT_REG2_NUM*sizeof(struct global_reg)];
			memcpy(buf,RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x3804; //Map server can now receive all kinds of reg values with the same packet. [Skotlex]
			mapif_sendall(buf, WBUFW(buf,2));
		}
			RFIFOSKIP(fd, RFIFOW(fd,2));
		break;

		// State change of account/ban notification (from login-server)
		case 0x2731:
			if (RFIFOREST(fd) < 11)
				return 0;
			
		{	// send to all map-servers to disconnect the player
			unsigned char buf[11];
			WBUFW(buf,0) = 0x2b14;
			WBUFL(buf,2) = RFIFOL(fd,2);
			WBUFB(buf,6) = RFIFOB(fd,6); // 0: change of statut, 1: ban
			WBUFL(buf,7) = RFIFOL(fd,7); // status or final date of a banishment
			mapif_sendall(buf, 11);
		}
			// disconnect player if online on char-server
			disconnect_player(RFIFOL(fd,2));

			RFIFOSKIP(fd,11);
		break;

		// Login server request to kick a character out. [Skotlex]
		case 0x2734:
			if (RFIFOREST(fd) < 6)
				return 0;
		{
			int aid = RFIFOL(fd,2);
			struct online_char_data* character = onlinedb_get(aid);
			RFIFOSKIP(fd,6);
			if( character != NULL )
			{// account is already marked as online!
				if( character->server > -1 )
				{	//Kick it from the map server it is on.
					mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
					set_char_waitdisconnect(character->account_id, AUTH_TIMEOUT);
				}
				else
				{// Manual kick from char server.
					struct char_session_data *tsd;
					int i;
					ARR_FIND( 0, fd_max, i, session[i] && (tsd = (struct char_session_data*)session[i]->session_data) && tsd->account_id == aid );
					if( i < fd_max )
					{
						WFIFOHEAD(i,3);
						WFIFOW(i,0) = 0x81;
						WFIFOB(i,2) = 2; // "Someone has already logged in with this id"
						WFIFOSET(i,3);
						set_eof(i);
					}
					else // still moving to the map-server
 						set_char_offline(-1, aid);
				}
			}
			idb_remove(auth_db, aid);// reject auth attempts from map-server
		}
		break;
		
		// ip address update signal from login server
		case 0x2735:
		{
			unsigned char buf[2];
			uint32 new_ip = 0;

			WBUFW(buf,0) = 0x2b1e;
			mapif_sendall(buf, 2);

			new_ip = host2ip(char_config.login_ip);
			if (new_ip && new_ip != login_ip)
				login_ip = new_ip; //Update login up.

			new_ip = host2ip(char_config.char_ip);
			if (new_ip && new_ip != char_ip)
			{	//Update ip.
				char_ip = new_ip;
				ShowInfo("Updating IP for [%s].\n", char_config.char_ip);
				// notify login server about the change
				WFIFOHEAD(fd,6);
				WFIFOW(fd,0) = 0x2736;
				WFIFOL(fd,2) = htonl(char_ip);
				WFIFOSET(fd,6);
			}
		}

			RFIFOSKIP(fd,2);
		break;

		default:
			ShowError("Unknown packet 0x%04x received from login-server, disconnecting.\n", command);
			set_eof(fd);
			return 0;
		}
	}

	return 0;
}


/// Timer function.
/// When triggered, it tries to connect/reconnect to the login-server.
static int check_connect_login_server(int tid, unsigned int tick, int id, intptr data)
{
	if( loginif_connect_timer != tid ) {
		ShowDebug("check_connect_login_server: invalid tid (%d != %d)\n", tid, loginif_connect_timer);
		return 0;// invalid
	}

	if( loginif_is_connected() )
		loginif_connect_timer = INVALID_TIMER;
	else// try again in 10 seconds
		loginif_connect_timer = add_timer(gettick() + 10 * 1000, check_connect_login_server, 0, 0);

	if( login_fd == -1 )
	{
		login_fd = make_connection(login_ip, char_config.login_port);
		if( login_fd == -1 )
			return 0;

		session[login_fd]->func_parse = parse_fromlogin;
		session[login_fd]->flag.server = 1;
		realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		loginif_charserver_login();
	}
	return 0;
}


/// Starts the connect timer.
void loginif_connect_timer_start(void)
{
	static bool is_registered = false;
	if( !is_registered )
	{
		add_timer_func_list(check_connect_login_server, "check_connect_login_server");
		is_registered = true;
	}
	if( loginif_connect_timer == INVALID_TIMER )
		loginif_connect_timer = add_timer(gettick() + 1000, check_connect_login_server, 0, 0);
}


/// Stops the connect timer.
void loginif_connect_timer_stop(void)
{
	if( loginif_connect_timer != INVALID_TIMER )
	{
		delete_timer(loginif_connect_timer, check_connect_login_server);
		loginif_connect_timer = INVALID_TIMER;
	}
}


static int send_accounts_tologin(int tid, unsigned int tick, int id, intptr data)
{
	loginif_online_accounts_list();
	return 0;
}

static int ping_login_server(int tid, unsigned int tick, int id, intptr data)
{
	loginif_ping();
	return 0;
}

bool loginif_is_connected(void)
{
	return( session_isActive(login_fd) && loginif_state == LOGINIF_READY );
}


/// Connect/reconnect request to the login server.
/// Will receive answer 0x2711 (0: success, 3: rejected).
void loginif_charserver_login(void)
{
	loginif_state = LOGINIF_LOGIN;
	if( cookie_expired(&cookie) )
	{// connect
		ShowStatus("Connecting to Login Server...\n");
		WFIFOHEAD(login_fd,86);
		WFIFOW(login_fd,0) = 0x2710;
		memcpy(WFIFOP(login_fd,2), char_config.userid, 24);
		memcpy(WFIFOP(login_fd,26), char_config.passwd, 24);
		WFIFOL(login_fd,50) = 0;
		WFIFOL(login_fd,54) = htonl(char_ip);
		WFIFOW(login_fd,58) = htons(char_config.char_port);
		memcpy(WFIFOP(login_fd,60), char_config.server_name, 20);
		WFIFOW(login_fd,80) = 0;
		WFIFOW(login_fd,82) = char_config.char_maintenance;
		WFIFOW(login_fd,84) = char_config.char_new_display;
		WFIFOSET(login_fd,86);
	}
	else
	{// reconnect
		ShowStatus("Reconnecting to Login Server...\n");
		WFIFOHEAD(login_fd,4+cookie.len);
		WFIFOW(login_fd,0) = 0x271a;
		WFIFOW(login_fd,2) = 4+cookie.len;
		memcpy(WFIFOP(login_fd,4), cookie.data, cookie.len);
		WFIFOSET(login_fd,4+cookie.len);
	}
}


/// Ask login-server to authenticate this account.
void loginif_auth_request(int account_id, int login_id1, int login_id2, int sex, int ip, int fd)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,23);
	WFIFOW(login_fd,0) = 0x2712; 
	WFIFOL(login_fd,2) = account_id;
	WFIFOL(login_fd,6) = login_id1;
	WFIFOL(login_fd,10) = login_id2;
	WFIFOB(login_fd,14) = sex;
	WFIFOL(login_fd,15) = ip;
	WFIFOL(login_fd,19) = fd;
	WFIFOSET(login_fd,23);
}


/// Send number of online users to login server.
/// Does not include people connected to the charserver.
void loginif_user_count(int users)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x2714;
	WFIFOL(login_fd,2) = users;
	WFIFOSET(login_fd,6);
}


/// Request account data (email, expiration time, gm level).
/// Will receive answer 0x2717.
void loginif_request_account_data(int account_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x2716;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}


/// Sends a ping packet to login server.
/// Will receive pong 0x2718.
void loginif_ping(void)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,2);
	WFIFOW(login_fd,0) = 0x2719;
	WFIFOSET(login_fd,2);
}


/// Forward email change request to login server.
void loginif_change_email(int account_id, const char* old_email, const char* new_email)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,86);
	WFIFOW(login_fd,0) = 0x2722;
	WFIFOL(login_fd,2) = account_id;
	safestrncpy((char*)WFIFOP(login_fd,6), old_email, 40);
	safestrncpy((char*)WFIFOP(login_fd,46), new_email, 40);
	WFIFOSET(login_fd,86);
}


/// Change an account's status code.
void loginif_account_status(int account_id, int status)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,10);
	WFIFOW(login_fd,0) = 0x2724;
	WFIFOL(login_fd,2) = account_id;
	WFIFOL(login_fd,6) = status;
	WFIFOSET(login_fd,10);
}


/// Ban an account for the specified duration.
void loginif_account_ban(int account_id, short year, short month, short day, short hour, short minute, short second)
{
	if( !session_isActive(login_fd) )
		return;
	
	WFIFOHEAD(login_fd,18);
	WFIFOW(login_fd, 0) = 0x2725;
	WFIFOL(login_fd, 2) = account_id;
	WFIFOW(login_fd, 6) = year;
	WFIFOW(login_fd, 8) = month;
	WFIFOW(login_fd,10) = day;
	WFIFOW(login_fd,12) = hour;
	WFIFOW(login_fd,14) = minute;
	WFIFOW(login_fd,16) = second;
	WFIFOSET(login_fd,18);
}


/// Change an account's gender.
void loginif_account_changesex(int account_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x2727;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}


/// Send account registry save request to login server.
void loginif_save_accreg2(unsigned char* buf, int len)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,len+4);
	WFIFOW(login_fd,0) = 0x2728;
	WFIFOW(login_fd,2) = len+4;
	memcpy(WFIFOP(login_fd,4), buf, len);
	WFIFOSET(login_fd,len+4);
}


/// Unban a banned account.
void loginif_account_unban(int account_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x272a;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}


/// Mark account as online on login server.
void loginif_char_online(int account_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x272b;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}


/// Mark account as offline on login server.
void loginif_char_offline(int account_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x272c;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}


/// Send online accounts list to login server.
void loginif_online_accounts_list(void)
{
	unsigned int users;
	DBIterator* iter;
	DBKey key;
	int i;

	if( !session_isActive(login_fd) )
		return;

	users = onlinedb_size();

	WFIFOHEAD(login_fd,8+users*4);
	WFIFOW(login_fd,0) = 0x272d;

	iter = onlinedb_iterator();
	i = 0;
	while( iter->next(iter, &key) )
	{
		int account_id = key.i;
		struct online_char_data* character = onlinedb_get(account_id);
		if( character->server > -1 )
		{
			WFIFOL(login_fd,8+i*4) = account_id;
			i++;
		}
	}
	iter->destroy(iter);

	WFIFOW(login_fd,2) = 8+ i*4;
	WFIFOL(login_fd,4) = i;
	WFIFOSET(login_fd,WFIFOW(login_fd,2));
}


/// Request account registry from login server.
/// Will receive answer 0x2729.
void loginif_request_accreg2(int account_id, int char_id)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,10);
	WFIFOW(login_fd,0) = 0x272e;
	WFIFOL(login_fd,2) = account_id;
	WFIFOL(login_fd,6) = char_id;
	WFIFOSET(login_fd,10);
}


/// Tell login server to mark all users on this charserver as offline.
void loginif_all_offline(void)
{
	if( !session_isActive(login_fd) )
		return;

	WFIFOHEAD(login_fd,2);
	WFIFOW(login_fd,0) = 0x2737;
	WFIFOSET(login_fd,2);
}

void do_init_loginif(void)
{

	// keep the char-login connection alive
	add_timer_func_list(ping_login_server, "ping_login_server");
	add_timer_interval(gettick() + 1000, ping_login_server, 0, 0, ((int)stall_time-2) * 1000);

	// send a list of all online account IDs to login server
	add_timer_func_list(send_accounts_tologin, "send_accounts_tologin");
	add_timer_interval(gettick() + 1000, send_accounts_tologin, 0, 0, 3600 * 1000); //Sync online accounts every hour

	cookie_init(&cookie);
	cookie.on_timeout = loginif_cookie_timeout_callback;
	loginif_connect_timer_start();
}

void do_final_loginif(void)
{
	loginif_connect_timer_stop();
	cookie_destroy(&cookie);
	if( login_fd != -1 )
	{
		do_close(login_fd);
		login_fd = -1;
	}
}
