// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/version.h"
#include "chardb.h"
#include "charlog.h"
#include "inter.h"
#include "map.h"

//temporary imports
extern CharDB* chars;

extern int login_fd;
struct auth_node {
	int account_id;
	int char_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	int sex;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int gmlevel;
};
extern DBMap* auth_db;
struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server; // -2: unknown server, -1: not connected, 0+: id of server
};
extern DBMap* online_char_db;
extern int char_num, char_max;
#include "char.h"
extern struct mmo_charstatus *char_dat;
extern void set_char_offline(int char_id, int account_id);
extern int email_creation;
extern int search_mapserver(unsigned short map, uint32 ip, uint16 port);
extern int parse_frommap(int fd);
extern char userid[24];
extern char passwd[24];
extern void char_auth_ok(int fd, struct char_session_data *sd);
extern int lan_subnetcheck(uint32 ip);
extern void set_char_online(int map_id, int char_id, int account_id);
extern int make_new_char(struct char_session_data* sd, const char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style);
extern int mmo_char_tobuf(uint8* buf, struct mmo_charstatus* p);
extern int char_delete(struct mmo_charstatus *cs);
extern bool char_new;

#include "../common/sql.h"
extern DBMap* char_db_;
int make_new_char_sql(struct char_session_data* sd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style);
int delete_char_sql(int char_id);




int parse_char(int fd)
{
	int i;
	char email[40];
	unsigned short cmd;
	int map_fd;
	struct char_session_data* sd;
	uint32 ipl = session[fd]->client_addr;
	
	sd = (struct char_session_data*)session[fd]->session_data;

	// disconnect any player if no login-server.
	if(login_fd < 0)
		set_eof(fd);

	if(session[fd]->flag.eof)
	{
		if( sd != NULL && sd->auth )
		{
			struct online_char_data* data = (struct online_char_data*)idb_get(online_char_db, sd->account_id);
			if( data == NULL || data->server == -1) //If it is not in any server, send it offline. [Skotlex]
				set_char_offline(-1,sd->account_id);
			if( data != NULL && data->fd == fd)
				data->fd = -1;
		}
		do_close(fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		//For use in packets that depend on an sd being present [Skotlex]
		#define FIFOSD_CHECK(rest) { if(RFIFOREST(fd) < rest) return 0; if (sd==NULL || !sd->auth) { RFIFOSKIP(fd,rest); return 0; } }

		cmd = RFIFOW(fd,0);
		switch( cmd )
		{

		// request to connect
		// 0065 <account id>.L <login id1>.L <login id2>.L <???>.W <sex>.B
		case 0x65:
			if( RFIFOREST(fd) < 17 )
				return 0;
		{
			struct auth_node* node;

			int account_id = RFIFOL(fd,2);
			uint32 login_id1 = RFIFOL(fd,6);
			uint32 login_id2 = RFIFOL(fd,10);
			int sex = RFIFOB(fd,16);
			RFIFOSKIP(fd,17);

			ShowInfo("request connect - account_id:%d/login_id1:%d/login_id2:%d\n", account_id, login_id1, login_id2);

			if (sd) {
				//Received again auth packet for already authentified account?? Discard it.
				//TODO: Perhaps log this as a hack attempt?
				//TODO: and perhaps send back a reply?
				break;
			}
			
			CREATE(session[fd]->session_data, struct char_session_data, 1);
			sd = (struct char_session_data*)session[fd]->session_data;
			sd->account_id = account_id;
			sd->login_id1 = login_id1;
			sd->login_id2 = login_id2;
			sd->sex = sex;
			sd->auth = false; // not authed yet

			// send back account_id
			WFIFOHEAD(fd,4);
			WFIFOL(fd,0) = account_id;
			WFIFOSET(fd,4);

			// search authentification
			node = (struct auth_node*)idb_get(auth_db, account_id);
			if( node != NULL &&
			    node->account_id == account_id &&
				node->login_id1  == login_id1 &&
				node->login_id2  == login_id2 &&
				node->ip         == ipl )
			{// authentication found (coming from map server)
				idb_remove(auth_db, account_id);
				char_auth_ok(fd, sd);
			}
			else
			{// authentication not found (coming from login server)
				if (login_fd > 0) { // don't send request if no login-server
					WFIFOHEAD(login_fd,19);
					WFIFOW(login_fd,0) = 0x2712; // ask login-server to authentify an account
					WFIFOL(login_fd,2) = sd->account_id;
					WFIFOL(login_fd,6) = sd->login_id1;
					WFIFOL(login_fd,10) = sd->login_id2;
					WFIFOB(login_fd,14) = sd->sex;
					WFIFOL(login_fd,15) = htonl(ipl);
					WFIFOSET(login_fd,19);
				} else { // if no login-server, we must refuse connection
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x6c;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
		}
		break;

		// char select
		case 0x66:
			FIFOSD_CHECK(3);
		{
			struct mmo_charstatus cd;
			uint32 subnet_map_ip;
			struct auth_node* node;

			int slot = RFIFOB(fd,2);
			RFIFOSKIP(fd,3);

#ifdef TXT_ONLY
			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0) { // to modify an e-mail, login-server must be online
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}
#endif

			if( !chars->load_slot(chars, &cd, sd->account_id, slot) )
			{	//Not found?? May be forged packet.
				break;
			}

			cd.sex = sd->sex; //FIXME: is this a good idea?

#ifdef TXT_ONLY
			char_log("Character Selected, Account ID: %d, Character Slot: %d, Character Name: %s.\n", sd->account_id, slot, cd.name);
#else
			if (log_char) {
				char esc_name[NAME_LENGTH*2+1];

				Sql_EscapeStringLen(sql_handle, esc_name, cd.name, strnlen(cd.name, NAME_LENGTH));
				if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`time`, `account_id`,`char_num`,`name`) VALUES (NOW(), '%d', '%d', '%s')",
					charlog_db, sd->account_id, slot, esc_name) )
					Sql_ShowDebug(sql_handle);
			}
			ShowInfo("Selected char: (Account %d: %d - %s)\n", sd->account_id, slot, cd.name);
#endif
			// searching map server
			i = search_mapserver(cd.last_point.map,-1,-1);

			// if map is not found, we check major cities
			if (i < 0) {
				unsigned short j;
				//First check that there's actually a map server online.
				ARR_FIND( 0, MAX_MAP_SERVERS, j, server[j].fd >= 0 && server[j].map[0] );
				if (j == MAX_MAP_SERVERS) {
					ShowInfo("Connection Closed. No map servers available.\n");
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
					break;
				}
				if ((i = search_mapserver((j=mapindex_name2id(MAP_PRONTERA)),-1,-1)) >= 0) {
					cd.last_point.x = 273;
					cd.last_point.y = 354;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_GEFFEN)),-1,-1)) >= 0) {
					cd.last_point.x = 120;
					cd.last_point.y = 100;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_MORROC)),-1,-1)) >= 0) {
					cd.last_point.x = 160;
					cd.last_point.y = 94;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_ALBERTA)),-1,-1)) >= 0) {
					cd.last_point.x = 116;
					cd.last_point.y = 57;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_PAYON)),-1,-1)) >= 0) {
					cd.last_point.x = 87;
					cd.last_point.y = 117;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_IZLUDE)),-1,-1)) >= 0) {
					cd.last_point.x = 94;
					cd.last_point.y = 103;
				} else {
					ShowInfo("Connection Closed. No map server available that has a major city, and unable to find map-server for '%s'.\n", mapindex_id2name(cd.last_point.map));
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
					break;
				}
				ShowWarning("Unable to find map-server for '%s', sending to major city '%s'.\n", mapindex_id2name(cd.last_point.map), mapindex_id2name(j));
				cd.last_point.map = j;
			}

			//FIXME: is this case even possible? [ultramage]
			if ((map_fd = server[i].fd) < 1 || session[map_fd] == NULL)
			{
				ShowError("parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, i);
				server[i].fd = -1;
				memset(&server[i], 0, sizeof(struct mmo_map_server));
				//Send server closed.
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x81;
				WFIFOB(fd,2) = 1; // 01 = Server closed
				WFIFOSET(fd,3);
				break;
			}

			//Send player to map
			WFIFOHEAD(fd,28);
			WFIFOW(fd,0) = 0x71;
			WFIFOL(fd,2) = cd.char_id;
			mapindex_getmapname_ext(mapindex_id2name(cd.last_point.map), (char*)WFIFOP(fd,6));
			subnet_map_ip = lan_subnetcheck(ipl); // Advanced subnet check [LuzZza]
			WFIFOL(fd,22) = htonl((subnet_map_ip) ? subnet_map_ip : server[i].ip);
			WFIFOW(fd,26) = ntows(htons(server[i].port)); // [!] LE byte order here [!]
			WFIFOSET(fd,28);

			ShowInfo("Character selection '%s' (account: %d, slot: %d).\n", cd.name, sd->account_id, slot);

			// create temporary auth entry
			CREATE(node, struct auth_node, 1);
			node->account_id = sd->account_id;
			node->char_id = cd.char_id;
			node->login_id1 = sd->login_id1;
			node->login_id2 = sd->login_id2;
			node->sex = sd->sex;
			node->expiration_time = sd->expiration_time;
			node->gmlevel = sd->gmlevel;
			node->ip = ipl;
			idb_put(auth_db, sd->account_id, node);

			set_char_online(-2,node->char_id,sd->account_id);
		}
		break;

		// create new char
		// S 0067 <name>.24B <str>.B <agi>.B <vit>.B <int>.B <dex>.B <luk>.B <slot>.B <hair color>.W <hair style>.W
		case 0x67:
			FIFOSD_CHECK(37);
		{
			int result;

			char name[NAME_LENGTH];
			int str = RFIFOB(fd,26);
			int agi = RFIFOB(fd,27);
			int vit = RFIFOB(fd,28);
			int int_ = RFIFOB(fd,29);
			int dex = RFIFOB(fd,30);
			int luk = RFIFOB(fd,31);
			int slot = RFIFOB(fd,32);
			int haircolor = RFIFOW(fd,33);
			int hairstyle = RFIFOW(fd,35);
			safestrncpy(name, (const char*)RFIFOP(fd,2), NAME_LENGTH);
			RFIFOSKIP(fd,37);

			if( !char_new ) //turn character creation on/off [Kevin]
				result = -2;
			else
				result = make_new_char(sd, name, RFIFOB(fd,26),RFIFOB(fd,27),RFIFOB(fd,28),RFIFOB(fd,29),RFIFOB(fd,30),RFIFOB(fd,31),RFIFOB(fd,32),RFIFOW(fd,33),RFIFOW(fd,35));

			//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
			if (result < 0)
			{
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x6e;
				switch (result) {
				case -1: WFIFOB(fd,2) = 0x00; break;
				case -2: WFIFOB(fd,2) = 0x02; break;
				case -3: WFIFOB(fd,2) = 0x01; break;
				}
				WFIFOSET(fd,3);
			}
			else
			{
				struct mmo_charstatus ch;
				int len;

				// retrieve data
				chars->load_num(chars, &ch, result); //NOTE: only the short data is needed here

				// send to player
				WFIFOHEAD(fd,110);
				WFIFOW(fd,0) = 0x6d;
				len = 2 + mmo_char_tobuf(WFIFOP(fd,2), &ch);
				WFIFOSET(fd,len);
			}
		}
		break;

		// delete char
		case 0x68:
		// 2004-04-19aSakexe+ langtype 12 char deletion packet
		case 0x1fb:
			if (cmd == 0x68) FIFOSD_CHECK(46);
			if (cmd == 0x1fb) FIFOSD_CHECK(56);
		{
			int cid = RFIFOL(fd,2);
			struct mmo_charstatus* cs = NULL;
			ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, cid);
			memcpy(email, RFIFOP(fd,6), 40);
			RFIFOSKIP(fd,RFIFOREST(fd)); // hack to make the other deletion packet work

/*
#ifdef TXT_ONLY
			if (e_mail_check(email) == 0)
				strncpy(email, "a@a.com", 40); // default e-mail

			// BEGIN HACK: "change email using the char deletion 'confirm email' menu"
			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0)
			{ // to modify an e-mail, login-server must be online
				// if sended email is incorrect e-mail
				if (strcmp(email, "a@a.com") == 0) {
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x70;
					WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd,3);
					break;
				}
				// we change the packet to set it like selection.
				ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] != -1 && char_dat[sd->found_char[i]].char_id == cid );
				if( i < MAX_CHARS )
				{
					// we save new e-mail
					memcpy(sd->email, email, 40);
					// we send new e-mail to login-server ('online' login-server is checked before)
					WFIFOHEAD(login_fd,46);
					WFIFOW(login_fd,0) = 0x2715;
					WFIFOL(login_fd,2) = sd->account_id;
					memcpy(WFIFOP(login_fd, 6), email, 40);
					WFIFOSET(login_fd,46);

					// change value to put new packet (char selection)
					RFIFOSKIP(fd,-3); //FIXME: Will this work? Messing with the received buffer is ugly anyway... 
					RFIFOW(fd,0) = 0x66;
					RFIFOB(fd,2) = char_dat[sd->found_char[i]].slot;
					// not send packet, it's modify of actual packet
				} else {
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x70;
					WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd,3);
				}
				break;
			}
			// END HACK
			// otherwise, we delete the character
#endif
*/
			// Check if e-mail is correct
			if( strcmpi(email, sd->email) != 0 && //email does not match and 
				( strcmp("a@a.com", sd->email) != 0 || //it is not default email, or
				  ( strcmp("a@a.com", email) && strcmp("", email) ) //email sent does not matches default	
				)
			) {	//Fail
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}

			// check if this char exists
			//TODO: look up if char with 'cid' is on this sd->account_id
			if( i == MAX_CHARS )
			{ // Such a character does not exist in the account
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);
				break;
			}

#ifdef TXT_ONLY
			// deletion process
			cs = &char_dat[sd->found_char[i]];
			char_delete(cs);
#else
			/* Delete character */
			if(delete_char_sql(cid)<0){
				//can't delete the char
				//either SQL error or can't delete by some CONFIG conditions
				//del fail
				WFIFOW(fd, 0) = 0x70;
				WFIFOB(fd, 2) = 0;
				WFIFOSET(fd, 3);
				break;
			}
#endif

			/* Char successfully deleted.*/
			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x6f;
			WFIFOSET(fd,2);
		}
		break;

		// client keep-alive packet (every 12 seconds)
		// R 0187 <account ID>.l
		case 0x187:
			if (RFIFOREST(fd) < 6)
				return 0;
			RFIFOSKIP(fd,6);
		break;

		// char rename request
		// R 028d <account ID>.l <char ID>.l <new name>.24B
		case 0x28d:
			FIFOSD_CHECK(34);
			//not implemented
			RFIFOSKIP(fd,34);
		break;

		// login as map-server
		case 0x2af8:
			if (RFIFOREST(fd) < 60)
				return 0;
		{
			char* l_user = (char*)RFIFOP(fd,2);
			char* l_pass = (char*)RFIFOP(fd,26);
			l_user[23] = '\0';
			l_pass[23] = '\0';
			ARR_FIND( 0, MAX_MAP_SERVERS, i, server[i].fd <= 0 );
			if (i == MAX_MAP_SERVERS || strcmp(l_user, userid) || strcmp(l_pass, passwd)) {
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			} else {
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);

				server[i].fd = fd;
				server[i].ip = ntohl(RFIFOL(fd,54));
				server[i].port = ntohs(RFIFOW(fd,58));
				server[i].users = 0;
				memset(server[i].map, 0, sizeof(server[i].map));
				session[fd]->func_parse = parse_frommap;
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
				inter_mapif_init(fd);
			}

			RFIFOSKIP(fd,60);
		}
		return 0; // avoid processing of followup packets here

		// Athena info get
		case 0x7530:
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_INTER | ATHENA_SERVER_CHAR;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);

			RFIFOSKIP(fd,2);
		break;

		// unknown packet received
		default:
			ShowError("parse_char: Received unknown packet "CL_WHITE"0x%x"CL_RESET" from ip '"CL_WHITE"%s"CL_RESET"'! Disconnecting!\n", RFIFOW(fd,0), ip2str(ipl, NULL));
			set_eof(fd);
			return 0;
		}
	}

	RFIFOFLUSH(fd);
	return 0;
}
