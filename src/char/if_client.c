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
#include "if_login.h"
#include "if_map.h"
#include "online.h"

//temporary imports
extern CharServerDB* charserver;

extern DBMap* auth_db;
#include "char.h"
extern int email_creation;
extern int search_mapserver(unsigned short map, uint32 ip, uint16 port);
extern void char_auth_ok(int fd, struct char_session_data *sd);
extern int lan_subnetcheck(uint32 ip);
#define MAX_CHAR_BUF 110 //Max size (for WFIFOHEAD calls)
int mmo_char_tobuf(uint8* buf, struct mmo_charstatus* p);


int parse_client(int fd)
{
	CharDB* chars = charserver->chardb(charserver);
	int i;
	char email[40];
	unsigned short cmd;
	int map_fd;
	struct char_session_data* sd;
	uint32 ipl = session[fd]->client_addr;
	
	sd = (struct char_session_data*)session[fd]->session_data;

	// disconnect any player if no login-server.
	if( !loginif_is_connected() )
		set_eof(fd);

	if(session[fd]->flag.eof)
	{
		if( sd != NULL && sd->auth )
		{
			struct online_char_data* data = onlinedb_get(sd->account_id);
			if( data != NULL && data->fd == fd)
				data->fd = -1;
			if( data == NULL || data->server == -1) //If it is not in any server, send it offline. [Skotlex]
				set_char_offline(-1,sd->account_id);
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

			if( runflag != CHARSERVER_ST_RUNNING )
			{
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x6c;
				WFIFOB(fd,2) = 0;// rejected from server
				WFIFOSET(fd,3);
				break;
			}

			// search authentification
			node = (struct auth_node*)idb_get(auth_db, account_id);
			if( node != NULL &&
			    node->account_id == account_id &&
				node->login_id1  == login_id1 &&
				node->login_id2  == login_id2 /*&&
				node->ip         == ipl*/ )
			{// authentication found (coming from map server)
				idb_remove(auth_db, account_id);
				char_auth_ok(fd, sd);
			}
			else
			{// authentication not found (coming from login server)
				if( loginif_is_connected() )
					loginif_auth_request(sd->account_id, sd->login_id1, sd->login_id2, sd->sex, htonl(ipl), fd);
				else
				{// if no login-server, we must refuse connection
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
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0) {
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}
#endif

			if( !chars->load_num(chars, &cd, sd->slots[slot]) )
			{	//Not found?? May be forged packet.
				break;
			}

			cd.sex = sd->sex; //FIXME: is this a good idea?

			charlog_log(cd.char_id, cd.account_id, cd.slot, cd.name, "char select");
			ShowInfo("Selected char: (Account %d: %d - %s)\n", sd->account_id, slot, cd.name);

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
				ShowError("parse_client: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, i);
				memset(&server[i], 0, sizeof(struct mmo_map_server));
				server[i].fd = -1;
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
			struct mmo_charstatus cd;
			int result;
			int char_id;

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

			if( !char_config.char_new
			|| (slot < 0 || slot >= MAX_CHARS || sd->slots[slot] != 0) // invalid slot or in use
			|| (char_config.chars_per_account > 0 && sd->chars_num >= char_config.chars_per_account) // maximum number of chars reached
			|| (hairstyle < 0 || hairstyle >= 24) // hair style
			|| (haircolor < 0 || haircolor >= 9) // hair color
			|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
			|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
			|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
				result = -2;// reject
			else
				result = char_create(sd->account_id, name, str, agi, vit, int_, dex, luk, slot, haircolor, hairstyle, &char_id);

			//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
			if( result < 0 || !chars->load_num(chars, &cd, char_id) )
			{
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x6e;
				switch (result) {
				case -1: WFIFOB(fd,2) = 0x00; break;
				default:
				case -2: WFIFOB(fd,2) = 0xFF; break;
				case -3: WFIFOB(fd,2) = 0x01; break;
				}
				WFIFOSET(fd,3);
			}
			else
			{
				int len;

				cd.slot = slot; // XXX if different, update slot in the database?
				sd->slots[slot] = cd.char_id;
				++sd->chars_num;
				// send to player
				WFIFOHEAD(fd,2+MAX_CHAR_BUF);
				WFIFOW(fd,0) = 0x6d;
				len = 2 + mmo_char_tobuf(WFIFOP(fd,2), &cd);
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
			int char_id = RFIFOL(fd,2);
			safestrncpy(email, RFIFOP(fd,6), sizeof(email));
			RFIFOSKIP(fd, cmd==0x68?46:56);

			ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, char_id);
/*
#ifdef TXT_ONLY
			if (e_mail_check(email) == 0)
				strncpy(email, "a@a.com", 40); // default e-mail

			// BEGIN HACK: "change email using the char deletion 'confirm email' menu"
			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && loginif_is_connected())
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
				ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] != -1 && char_dat[sd->found_char[i]].char_id == char_id );
				if( i < MAX_CHARS )
				{
					// we save new e-mail
					memcpy(sd->email, email, 40);

					// we send new e-mail to login-server ('online' login-server is checked before)
					loginif_change_email(sd->account_id, "a@a.com", sd->email);

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
				WFIFOB(fd,2) = 1; // Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}

			ARR_FIND(0, MAX_CHARS, i, sd->slots[i] == char_id);
			if( i == MAX_CHARS || char_delete(char_id) != 0 )
			{// not in the client view or failed to delete
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // character deletion denied
				WFIFOSET(fd,3);
				break;
			}

			// Char successfully deleted.
			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x6f;
			WFIFOSET(fd,2);
			// update client view
			sd->slots[i] = 0;
			if( sd->chars_num > MAX_CHARS )
			{// fill free slot with extra character
				struct mmo_charstatus cd;
				int char_id;
				CSDBIterator* it;

				it = chars->characters(chars, sd->account_id);
				while( it->next(it, &char_id) )
				{
					int j;
					ARR_FIND(0, MAX_CHARS, j, sd->slots[j] == char_id);
					if( j < MAX_CHARS )
						continue;// already displayed

					// send character
					chars->load_num(chars, &cd, char_id);
					cd.slot = i; // XXX if different, update slot in the database?
					WFIFOHEAD(fd,2+108);
					WFIFOW(fd,0) = 0x6f;
					j = 2 + mmo_char_tobuf(WFIFOP(fd,2), &cd);
					WFIFOSET(fd,j);
					sd->slots[i] = cd.char_id;
					break;
				}
				it->destroy(it);
			}
			--sd->chars_num;
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

		// captcha code request (not implemented)
		// R 07e5 <?>.w <aid>.l
		case 0x7e5:
			WFIFOHEAD(fd,5);
			WFIFOW(fd,0) = 0x7e9;
			WFIFOW(fd,2) = 5;
			WFIFOB(fd,4) = 1;
			WFIFOSET(fd,5);
			RFIFOSKIP(fd,8);
		break;

		// captcha code check (not implemented)
		// R 07e7 <len>.w <aid>.l <code>.b10 <?>.b14
		case 0x7e7:
			WFIFOHEAD(fd,5);
			WFIFOW(fd,0) = 0x7e9;
			WFIFOW(fd,2) = 5;
			WFIFOB(fd,4) = 1;
			WFIFOSET(fd,5);
			RFIFOSKIP(fd,32);
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
			ARR_FIND( 0, ARRAYLENGTH(server), i, server[i].fd <= 0 );
			if( runflag != CHARSERVER_ST_RUNNING ||
				i == ARRAYLENGTH(server) ||
				strcmp(l_user, char_config.userid) != 0 ||
				strcmp(l_pass, char_config.passwd) != 0 )
			{
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			}
			else
			{
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

				mapif_cookie_generate(i);
				inter_mapif_init(fd);
			}

			RFIFOSKIP(fd,60);
		}
		return 0; // avoid processing of followup packets here

		case 0x2b28:	// Reconnection request of a map-server
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		{
			uint16 cookielen;
			const char* cookiedata;
			int id;

			cookielen = RFIFOW(fd,2)-4;
			cookiedata = (const char*)RFIFOP(fd,4);

			ARR_FIND(0, ARRAYLENGTH(server), id, cookie_compare(&server[id].cookie, cookielen, cookiedata) == 0);
			if( cookielen == 0 || id == ARRAYLENGTH(server) )
			{// invalid, reject
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			}
			else if( session_isValid(server[id].fd) )
			{// already connected, reject
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
				// new cookie... not required, but better safe than sorry
				if( session_isActive(server[id].fd) )
					mapif_cookie_generate(id);
			}
			else
			{// all ok, accept
				server[id].fd = fd;

				session[fd]->func_parse = parse_frommap;
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

				// send connection success
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);

				cookie_timeout_stop(&server[id].cookie);
				inter_mapif_init(fd);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		}
		return 0; // processing will continue elsewhere

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
			ShowError("parse_client: Received unknown packet "CL_WHITE"0x%x"CL_RESET" from ip '"CL_WHITE"%s"CL_RESET"'! Disconnecting!\n", RFIFOW(fd,0), ip2str(ipl, NULL));
			set_eof(fd);
			return 0;
		}
	}

	return 0;
}


//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int mmo_char_send006b(int fd, struct char_session_data* sd)
{
	CharDB* chars = charserver->chardb(charserver);
	struct mmo_charstatus cd_arr[MAX_CHARS];
	struct mmo_charstatus cd;
	CSDBIterator* it;
	int char_id;
	int i,j;

	// load characters
	memset(cd_arr, 0, sizeof(cd_arr));
	sd->chars_num = 0;
	it = chars->characters(chars, sd->account_id);
	while( it->next(it, &char_id) )
	{
		chars->load_num(chars, &cd, char_id);
		++sd->chars_num;
		if( cd.slot < MAX_CHARS )
		{// use slot position
			if( cd_arr[cd.slot].account_id == sd->account_id )
			{// move occupant to first free slot
				ARR_FIND(0, MAX_CHARS, i, cd_arr[i].account_id != sd->account_id);
				if( i < MAX_CHARS )
					memcpy(&cd_arr[i], &cd_arr[cd.slot], sizeof(struct mmo_charstatus));
			}
			memcpy(&cd_arr[cd.slot], &cd, sizeof(struct mmo_charstatus));
		}
		else
		{// use first free slot
			ARR_FIND(0, MAX_CHARS, i, cd_arr[i].account_id != sd->account_id);
			if( i < MAX_CHARS )
				memcpy(&cd_arr[i], &cd, sizeof(struct mmo_charstatus));
		}
	}
	it->destroy(it);

	// update client view
	for( i = 0; i < MAX_CHARS; ++i )
	{
		cd_arr[i].slot = i; // XXX if different, update slot in the database?
		sd->slots[i] = cd_arr[i].char_id;
	}

	j = 24; // offset
	WFIFOHEAD(fd,j + MAX_CHARS*MAX_CHAR_BUF);
	WFIFOW(fd,0) = 0x6b;
	memset(WFIFOP(fd,4), 0, 20); // unknown bytes
	for( i = 0; i < MAX_CHARS; ++i )
		if( cd_arr[i].account_id == sd->account_id )
			j += mmo_char_tobuf(WFIFOP(fd,j), &cd_arr[i]);
	WFIFOW(fd,2) = j; // packet len
	WFIFOSET(fd,j);

	return 0;
}

/// Writes char data to the buffer in the format used by the client.
/// Used in packets 0x6b (chars info) and 0x6d (new char info).
/// Returns the size.
#define MAX_CHAR_BUF 110 //Max size (for WFIFOHEAD calls)
int mmo_char_tobuf(uint8* buffer, struct mmo_charstatus* p)
{
	unsigned short offset = 0;
	uint8* buf;

	if( buffer == NULL || p == NULL )
		return 0;

	buf = WBUFP(buffer,0);
	WBUFL(buf,0) = p->char_id;
	WBUFL(buf,4) = min(p->base_exp, LONG_MAX);
	WBUFL(buf,8) = p->zeny;
	WBUFL(buf,12) = min(p->job_exp, LONG_MAX);
	WBUFL(buf,16) = p->job_level;
	WBUFL(buf,20) = 0; // probably opt1
	WBUFL(buf,24) = 0; // probably opt2
	WBUFL(buf,28) = p->option;
	WBUFL(buf,32) = p->karma;
	WBUFL(buf,36) = p->manner;
	WBUFW(buf,40) = min(p->status_point, SHRT_MAX);
#if PACKETVER > 20081217
	WBUFL(buf,42) = p->hp;
	WBUFL(buf,46) = p->max_hp;
	offset+=4;
	buf = WBUFP(buffer,offset);
#else
	WBUFW(buf,42) = min(p->hp, SHRT_MAX);
	WBUFW(buf,44) = min(p->max_hp, SHRT_MAX);
#endif
	WBUFW(buf,46) = min(p->sp, SHRT_MAX);
	WBUFW(buf,48) = min(p->max_sp, SHRT_MAX);
	WBUFW(buf,50) = DEFAULT_WALK_SPEED; // p->speed;
	WBUFW(buf,52) = p->class_;
	WBUFW(buf,54) = p->hair;
	WBUFW(buf,56) = 0; // p->weapon;
	WBUFW(buf,58) = p->base_level;
	WBUFW(buf,60) = min(p->skill_point, SHRT_MAX);
	WBUFW(buf,62) = p->head_bottom;
	WBUFW(buf,64) = 0; // p->shield;
	WBUFW(buf,66) = p->head_top;
	WBUFW(buf,68) = p->head_mid;
	WBUFW(buf,70) = p->hair_color;
	WBUFW(buf,72) = p->clothes_color;
	memcpy(WBUFP(buf,74), p->name, NAME_LENGTH);
	WBUFB(buf,98) = min(p->str, UCHAR_MAX);
	WBUFB(buf,99) = min(p->agi, UCHAR_MAX);
	WBUFB(buf,100) = min(p->vit, UCHAR_MAX);
	WBUFB(buf,101) = min(p->int_, UCHAR_MAX);
	WBUFB(buf,102) = min(p->dex, UCHAR_MAX);
	WBUFB(buf,103) = min(p->luk, UCHAR_MAX);
	WBUFW(buf,104) = p->slot;
#if PACKETVER >= 20061023
	WBUFW(buf,106) = 1;
	offset += 2;
#endif

	return 106+offset;
}
