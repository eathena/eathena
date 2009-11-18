// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "char.h"
#include "chardb.h"
#include "int_rank.h"
#include "int_status.h"
#include "int_storage.h"
#include "inter.h"
#include "if_login.h"
#include "if_map.h"
#include "online.h"
#include <string.h>

//temporary imports
extern CharServerDB* charserver;

#include "char.h"
extern DBMap* auth_db; // int account_id -> struct auth_node*


struct mmo_map_server server[MAX_MAP_SERVERS];


/// Triggered when the cookie expires.
/// Resets the target server.
void mapif_cookie_timeout_callback(intptr data)
{
	int id = (int)data;
	if( id < 0 || id >= ARRAYLENGTH(server) )
		return;// invalid

	ShowWarning("Session expired for map-server %d.\n", id);
	cookie_set(&server[id].cookie, 0, NULL);
	mapif_server_reset(id);
}


/// Initializes a server structure.
void mapif_server_init(int id)
{
	memset(&server[id], 0, sizeof(server[id]));
	server[id].fd = -1;
	cookie_init(&server[id].cookie);
	server[id].cookie.timeout = MAPSERVER_TIMEOUT;
	server[id].cookie.callback_data = (intptr)id;
	server[id].cookie.on_timeout = mapif_cookie_timeout_callback;
}


/// Destroys a server structure.
void mapif_server_destroy(int id)
{
	if( server[id].fd == -1 )
	{
		do_close(server[id].fd);
		server[id].fd = -1;
	}
	cookie_destroy(&server[id].cookie);
}


/// Resets all the data related to a server.
void mapif_server_reset(int id)
{
	int i,j;
	unsigned char buf[16384];
	int fd = server[id].fd;
	//Notify other map servers that this one is gone. [Skotlex]
	WBUFW(buf,0) = 0x2b20;
	WBUFL(buf,4) = htonl(server[id].ip);
	WBUFW(buf,8) = htons(server[id].port);
	j = 0;
	for(i = 0; i < MAX_MAP_PER_SERVER; i++)
		if (server[id].map[i])
			WBUFW(buf,10+(j++)*4) = server[id].map[i];
	if (j > 0) {
		WBUFW(buf,2) = j * 4 + 10;
		mapif_sendallwos(fd, buf, WBUFW(buf,2));
	}
	onlinedb_mapserver_unknown(id); //Tag relevant chars as 'in disconnected' server.
	onlinedb_sync(); // update online list
	mapif_cookie_clear(id);
	mapif_server_destroy(id);
	mapif_server_init(id);
}


/// Sends a cookie.
void mapif_cookie_send(int id)
{
	int fd = server[id].fd;
	uint32 timeout = server[id].cookie.timeout;
	uint16 cookielen = server[id].cookie.len;
	const char* cookie = server[id].cookie.data;

	uint16 packet_len = 8 + cookielen;
	WFIFOHEAD(fd,packet_len);
	WFIFOW(fd,0) = 0x2b29;
	WFIFOW(fd,2) = packet_len;
	WFIFOL(fd,4) = timeout;
	memcpy(WFIFOP(fd,8), cookie, cookielen);
	WFIFOSET(fd,packet_len);
}


/// Generates a new unique cookie and assigns it to a server.
/// The cookie allows the server to reconnect and continue normally.
void mapif_cookie_generate(int id)
{
	size_t i;
	do
	{
		cookie_generate(&server[id].cookie);
		ARR_FIND(0, ARRAYLENGTH(server), i, i != id && cookie_compare(&server[i].cookie, server[id].cookie.len, server[id].cookie.data) == 0);
	} while( i != ARRAYLENGTH(server) );// cookie must be unique

	if( session_isActive(server[id].fd) )
		mapif_cookie_send(id);
}


/// Clears the cookie of a server.
void mapif_cookie_clear(int id)
{
	cookie_set(&server[id].cookie, 0, NULL);

	if( session_isActive(server[id].fd) )
		mapif_cookie_send(id);
}


/// Called when the connection to Map Server is disconnected.
void mapif_on_disconnect(int id)
{
	if( cookie_expired(&server[id].cookie) )
	{
		ShowStatus("Map-server #%d has disconnected.\n", id);
		mapif_server_reset(id);
	}
	else
	{
		ShowWarning("Map-server #%d has disconnected, waiting for reconnect...\n", id);
		cookie_timeout_start(&server[id].cookie);
	}
}


// sends data to all mapservers
int mapif_sendall(const void* buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

// sends data to all mapservers other than the one specified
int mapif_sendallwos(int sfd, const void* buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

// send data to a single mapserver
int mapif_send(int fd, const void* buf, unsigned int len)
{
	int i;

	if (fd >= 0) {
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (fd == server[i].fd) {
				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}


// Searches for the mapserver that has a given map (and optionally ip/port, if not -1).
// If found, returns the server's index in the 'server' array (otherwise returns -1).
int search_mapserver(unsigned short map, uint32 ip, uint16 port)
{
	int i, j;
	
	for(i = 0; i < MAX_MAP_SERVERS; i++)
	{
		if (server[i].fd > 0
		&& (ip == (uint32)-1 || server[i].ip == ip)
		&& (port == (uint16)-1 || server[i].port == port))
		{
			for (j = 0; server[i].map[j]; j++)
				if (server[i].map[j] == map)
					return i;
		}
	}

	return -1;
}


int parse_frommap(int fd)
{
	CharDB* chars = charserver->chardb(charserver);
	FriendDB* friends = charserver->frienddb(charserver);
	HotkeyDB* hotkeys = charserver->hotkeydb(charserver);
	MemoDB* memos = charserver->memodb(charserver);
	RankDB* ranks = charserver->rankdb(charserver);
	SkillDB* skills = charserver->skilldb(charserver);
	StorageDB* storages = charserver->storagedb(charserver);
	int i, j;
	int id;

	ARR_FIND( 0, ARRAYLENGTH(server), id, server[id].fd == fd );
	if( id == ARRAYLENGTH(server) )
	{// not a map server
		ShowDebug("parse_frommap: Disconnecting invalid session #%d (is not a map-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		server[id].fd = -1;
		mapif_on_disconnect(id);
		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		//ShowDebug("Received packet 0x%4x (%d bytes) from map-server (connection %d)\n", RFIFOW(fd, 0), RFIFOREST(fd), fd);

		switch(RFIFOW(fd,0))
		{

		case 0x2afa: // Receiving map names list from the map-server
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			memset(server[id].map, 0, sizeof(server[id].map));
			j = 0;
			for(i = 4; i < RFIFOW(fd,2); i += 4) {
				server[id].map[j] = RFIFOW(fd,i);
				j++;
			}

			ShowStatus("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d.\n", id, j, CONVIP(server[id].ip), server[id].port);
			log_char("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d.\n", id, j, CONVIP(server[id].ip), server[id].port);

			// send name for wisp to player
			WFIFOHEAD(fd, 3 + NAME_LENGTH);
			WFIFOW(fd,0) = 0x2afb;
			WFIFOB(fd,2) = 0;
			memcpy(WFIFOP(fd,3), char_config.wisp_server_name, NAME_LENGTH);
			WFIFOSET(fd,3+NAME_LENGTH);

			{
			unsigned char buf[16384];
			int x;
			if (j == 0) {
				ShowWarning("Map-server %d has NO maps.\n", id);
				log_char("WARNING: Map-server %d has NO maps.\n", id);
			} else {
				// Transmitting maps information to the other map-servers
				WBUFW(buf,0) = 0x2b04;
				WBUFW(buf,2) = j * 4 + 10;
				WBUFL(buf,4) = htonl(server[id].ip);
				WBUFW(buf,8) = htons(server[id].port);
				memcpy(WBUFP(buf,10), RFIFOP(fd,4), j * 4);
				mapif_sendallwos(fd, buf, WBUFW(buf,2));
			}
			// Transmitting the maps of the other map-servers to the new map-server
			for(x = 0; x < MAX_MAP_SERVERS; x++) {
				if (server[x].fd > 0 && x != id) {
					WFIFOHEAD(fd,10 +4*MAX_MAP_PER_SERVER);
					WFIFOW(fd,0) = 0x2b04;
					WFIFOL(fd,4) = htonl(server[x].ip);
					WFIFOW(fd,8) = htons(server[x].port);
					j = 0;
					for(i = 0; i < MAX_MAP_PER_SERVER; i++)
						if (server[x].map[i])
							WFIFOW(fd,10+(j++)*4) = server[x].map[i];
					if (j > 0) {
						WFIFOW(fd,2) = j * 4 + 10;
						WFIFOSET(fd,WFIFOW(fd,2));
					}
				}
			}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x2afe: //set MAP user count
			if (RFIFOREST(fd) < 4)
				return 0;
			if (RFIFOW(fd,2) != server[id].users) {
				server[id].users = RFIFOW(fd,2);
				ShowInfo("User Count: %d (Server: %d)\n", server[id].users, id);
			}
			RFIFOSKIP(fd, 4);
			break;

		case 0x2aff: //set MAP users
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			//TODO: update guild/party online/offline states.

			onlinedb_mapserver_unknown(id); //Set all chars from this server as 'unknown'

			for(i = 0; i < RFIFOW(fd,4); i++)
			{
				int aid = RFIFOL(fd,6+i*8);
				int cid = RFIFOL(fd,6+i*8+4);

				set_char_online(id, cid, aid);
			}
			//If any chars remain in -2, they will be cleaned in the cleanup timer.
			RFIFOSKIP(fd,6+i*8);
		break;

		case 0x2b01: // Receive character data from map-server for saving
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		{
			struct online_char_data* character;
			struct mmo_charstatus cd;

			int size = RFIFOW(fd,2);
			int aid = RFIFOL(fd,4);
			int cid = RFIFOL(fd,8);
			bool finalsave = (bool)RFIFOB(fd,12);

			// verify data size
			if (size - 13 != sizeof(struct mmo_charstatus))
			{
				ShowError("parse_from_map (save-char): Size mismatch! %d != %d\n", size-13, sizeof(struct mmo_charstatus));
				RFIFOSKIP(fd,size);
				break;
			}

			memcpy(&cd, RFIFOP(fd,13), sizeof(struct mmo_charstatus));
			RFIFOSKIP(fd,size);

			//Check account only if this ain't final save. Final-save goes through because of the char-map reconnect
			if( finalsave || ((character = onlinedb_get(aid)) != NULL && character->char_id == cid) )
			{
				//TODO: perhaps check if account id matches

				if( chars->save(chars, &cd) )
				{
					storages->save(storages, cd.inventory, MAX_INVENTORY, STORAGE_INVENTORY, cd.char_id);
					storages->save(storages, cd.cart, MAX_CART, STORAGE_CART, cd.char_id);
					storages->save(storages, cd.storage, MAX_STORAGE, STORAGE_KAFRA, cd.account_id);
					memos->save(memos, &cd.memo_point, cd.char_id);
					skills->save(skills, &cd.skill, cd.char_id);
					friends->save(friends, &cd.friends, cd.char_id);
					hotkeys->save(hotkeys, &cd.hotkeys, cd.char_id);
				}
				//TODO: error handling
			}
			else
			{// This may be valid on char-server reconnection, when re-sending characters that already logged off.
				ShowError("parse_from_map (save-char): Received data for non-existant/offline character (%d:%d).\n", aid, cid);
				set_char_online(id, cid, aid);
			}

			if (finalsave)
			{// Save ack only needed on final save.
				set_char_offline(cid,aid);
				WFIFOHEAD(fd,10);
				WFIFOW(fd,0) = 0x2b21; 
				WFIFOL(fd,2) = aid;
				WFIFOL(fd,6) = cid;
				WFIFOSET(fd,10);
			}
		}
		break;

		case 0x2b02: // req char selection
			if( RFIFOREST(fd) < 18 )
				return 0;
		{
			struct auth_node* node;

			int account_id = RFIFOL(fd,2);
			uint32 login_id1 = RFIFOL(fd,6);
			uint32 login_id2 = RFIFOL(fd,10);
			uint32 ip = RFIFOL(fd,14);
			RFIFOSKIP(fd,18);

			if( false )
			{
				WFIFOHEAD(fd,7);
				WFIFOW(fd,0) = 0x2b03;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 0;// not ok
				WFIFOSET(fd,7);
			}
			else
			{
				// create temporary auth entry
				CREATE(node, struct auth_node, 1);
				node->account_id = account_id;
				node->char_id = 0;
				node->login_id1 = login_id1;
				node->login_id2 = login_id2;
				//node->sex = 0;
				node->ip = ntohl(ip);
				//node->expiration_time = 0; // unlimited/unknown time by default (not display in map-server)
				//node->gmlevel = 0;
				idb_put(auth_db, account_id, node);

				//Set char to "@ char select" in online db [Kevin]
				set_char_charselect(account_id);

				WFIFOHEAD(fd,7);
				WFIFOW(fd,0) = 0x2b03;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 1;// ok
				WFIFOSET(fd,7);
			}
		}
		break;

		case 0x2b05: // request "change map server"
			if (RFIFOREST(fd) < 35)
				return 0;
		{
			int map_id, map_fd = -1;
			struct online_char_data* data;
			struct mmo_charstatus cd;

			int account_id = RFIFOL(fd,2);
			int login_id1 = RFIFOL(fd,6);
			int login_id2 = RFIFOL(fd,10);
			int char_id = RFIFOL(fd,14);
			unsigned short mapindex = RFIFOW(fd,18);
			short x = RFIFOW(fd,20);
			short y = RFIFOW(fd,22);
			uint32 server_ip = RFIFOL(fd,24);
			uint16 server_port = RFIFOW(fd,28);
			char sex = RFIFOB(fd,30);
			uint32 client_ip = RFIFOL(fd,31);
			RFIFOSKIP(fd,35);

			map_id = search_mapserver(mapindex, ntohl(server_ip), ntohs(server_port)); //Locate mapserver by ip and port.
			if (map_id >= 0)
				map_fd = server[map_id].fd;

			if( session_isActive(map_fd) &&
				chars->load_num(chars, &cd, char_id) )
			{	//Send the map server the auth of this player.
				struct auth_node* node;

				//Update the "last map" as this is where the player must be spawned on the new map server.
				cd.last_point.map = mapindex;
				cd.last_point.x = x;
				cd.last_point.y = y;
				cd.sex = sex;

				// create temporary auth entry
				CREATE(node, struct auth_node, 1);
				node->account_id = account_id;
				node->char_id = char_id;
				node->login_id1 = login_id1;
				node->login_id2 = login_id2;
				node->sex = sex;
				//node->expiration_time = 0; // FIXME
				node->ip = ntohl(client_ip);
				idb_put(auth_db, account_id, node);

				data = onlinedb_ensure(account_id);
				data->char_id = cd.char_id;
				data->server = map_id; //Update server where char is.

				//Reply with an ack.
				WFIFOHEAD(fd,30);
				WFIFOW(fd,0) = 0x2b06;
				memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 28);
				WFIFOSET(fd,30);
			} else { //Reply with nak
				WFIFOHEAD(fd,30);
				WFIFOW(fd,0) = 0x2b06;
				memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 28);
				WFIFOL(fd,6) = 0; //Set login1 to 0.
				WFIFOSET(fd,30);
			}
		}
		break;

		case 0x2b08: // char name request
			if (RFIFOREST(fd) < 6)
				return 0;

			WFIFOHEAD(fd,30);
			WFIFOW(fd,0) = 0x2b09;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			if( !chars->id2name(chars, (int)RFIFOL(fd,2), (char*)WFIFOP(fd,6), NAME_LENGTH) )
				safestrncpy((char*)WFIFOP(fd,6), char_config.unknown_char_name, NAME_LENGTH);
			WFIFOSET(fd,30);

			RFIFOSKIP(fd,6);
		break;

		case 0x2b0c: // Map server send information to change an email of an account -> login-server
			if (RFIFOREST(fd) < 86)
				return 0;
			loginif_change_email(RFIFOL(fd,2), (const char*)RFIFOP(fd,6), (const char*)RFIFOP(fd,46));
			RFIFOSKIP(fd, 86);
		break;

		case 0x2b0e: // Request from map-server to change an account's status (will just be forwarded to login server)
			if (RFIFOREST(fd) < 44)
				return 0;
		{
			int result = 0; // 0-login-server request done, 1-player not found, 2-gm level too low, 3-login-server offline
			struct mmo_charstatus cd;

			char name[NAME_LENGTH];
			int acc = RFIFOL(fd,2); // account_id of who ask (-1 if server itself made this request)
			int type = RFIFOW(fd,30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban
			short year = RFIFOW(fd,32);
			short month = RFIFOW(fd,34);
			short day = RFIFOW(fd,36);
			short hour = RFIFOW(fd,38);
			short minute = RFIFOW(fd,40);
			short second = RFIFOW(fd,42);
			safestrncpy(name, (char*)RFIFOP(fd,6), NAME_LENGTH); // name of the target character
			RFIFOSKIP(fd,44);

			if( !chars->load_str(chars, &cd, name, char_config.character_name_case_sensitive) )
				result = 1; // 1-player not found
			else
			{
				int account_id = cd.account_id;
				safestrncpy(name, cd.name, NAME_LENGTH);

				if( !loginif_is_connected() )
					result = 3; // 3-login-server offline
				//FIXME: need to move this check to login server [ultramage]
//				else
//				if( acc != -1 && isGM(acc) < isGM(account_id) )
//					result = 2; // 2-gm level too low
				else
				switch( type ) {
				case 1: loginif_account_status(account_id, 5); break; // block
				case 2: loginif_account_ban(account_id, year, month, day, hour, minute, second); break; // ban
				case 3: loginif_account_status(account_id, 0); break; // unblock
				case 4: loginif_account_unban(account_id); break; // unban
				case 5: loginif_account_changesex(account_id); break; // changesex
				}
			}

			// send answer if a player ask, not if the server ask
			// Don't send answer for changesex
			if( acc != -1 && type != 5 )
			{
				WFIFOHEAD(fd,34);
				WFIFOW(fd, 0) = 0x2b0f;
				WFIFOL(fd, 2) = acc;
				safestrncpy((char*)WFIFOP(fd,6), name, NAME_LENGTH);
				WFIFOW(fd,30) = type;
				WFIFOW(fd,32) = result;
				WFIFOSET(fd,34);
			}
		}
		break;

		// Divorce chars
		case 0x2b11:
			if( RFIFOREST(fd) < 10 )
				return 0;

			char_divorce(RFIFOL(fd,2), RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
		break;

		case 0x2b17: // Character disconnected set online 0 [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
		break;

		case 0x2b18: // Reset all chars to offline [Wizputer]
			onlinedb_mapserver_offline(id);
			RFIFOSKIP(fd,2);
		break;
		
		case 0x2b19: // Character set online [Wizputer]
			if (RFIFOREST(fd) < 10)
				return 0;
			set_char_online(id, RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
		break;

		case 0x2b23: // map-server alive packet
			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2b24;
			WFIFOSET(fd,2);
			RFIFOSKIP(fd,2);
		break;

		case 0x2b26: // auth request from map-server
			if (RFIFOREST(fd) < 19)
				return 0;
		{
			int account_id;
			int char_id;
			int login_id1;
			char sex;
			uint32 ip;
			struct auth_node* node;
			struct mmo_charstatus cd;

			account_id = RFIFOL(fd,2);
			char_id    = RFIFOL(fd,6);
			login_id1  = RFIFOL(fd,10);
			sex        = RFIFOB(fd,14);
			ip         = ntohl(RFIFOL(fd,15));
			RFIFOSKIP(fd,19);

			node = (struct auth_node*)idb_get(auth_db, account_id);

			if( chars->load_num(chars, &cd, char_id) && //TODO: verify account_id?
				node != NULL &&
				node->account_id == account_id &&
				node->char_id == char_id &&
				node->login_id1 == login_id1 &&
				node->sex == sex &&
				node->ip == ip )
			{// auth ok
				cd.sex = sex; //FIXME: is this ok?

				// load auxiliary data
				storages->load(storages, cd.inventory, MAX_INVENTORY, STORAGE_INVENTORY, char_id);
				storages->load(storages, cd.cart, MAX_CART, STORAGE_CART, char_id);
				storages->load(storages, cd.storage, MAX_STORAGE, STORAGE_KAFRA, account_id);
				memos->load(memos, &cd.memo_point, cd.char_id);
				skills->load(skills, &cd.skill, cd.char_id);
				friends->load(friends, &cd.friends, char_id);
				hotkeys->load(hotkeys, &cd.hotkeys, char_id);
				cd.fame = ranks->get_points(ranks, inter_rank_class2rankid(cd.class_), char_id);

				WFIFOHEAD(fd,24 + sizeof(struct mmo_charstatus));
				WFIFOW(fd,0) = 0x2afd;
				WFIFOW(fd,2) = 24 + sizeof(struct mmo_charstatus);
				WFIFOL(fd,4) = account_id;
				WFIFOL(fd,8) = node->login_id1;
				WFIFOL(fd,12) = node->login_id2;
				WFIFOL(fd,16) = (uint32)node->expiration_time; // FIXME: will wrap to negative after "19-Jan-2038, 03:14:07 AM GMT"
				WFIFOL(fd,20) = node->gmlevel;
				memcpy(WFIFOP(fd,24), &cd, sizeof(struct mmo_charstatus));
				WFIFOSET(fd, WFIFOW(fd,2));

				// only use the auth once and mark user online
				idb_remove(auth_db, account_id);
				set_char_online(id, char_id, account_id);
			}
			else
			{// auth failed
				WFIFOHEAD(fd,19);
				WFIFOW(fd,0) = 0x2b27;
				WFIFOL(fd,2) = account_id;
				WFIFOL(fd,6) = char_id;
				WFIFOL(fd,10) = login_id1;
				WFIFOB(fd,14) = sex;
				WFIFOL(fd,15) = htonl(ip);
				WFIFOSET(fd,19);
			}
		}
		break;

		case 0x2b2a:	// cookie ack/request/release
			if (RFIFOREST(fd) < 3)
				return 0;
		{
			uint8 type = RFIFOB(fd,2);
			RFIFOSKIP(fd,3);

			if( type == 0 )
			{// ack
			}
			else if( type == 1 )
			{// request
				mapif_cookie_generate(id);
			}
			else if( type == 2 )
			{// release
				mapif_cookie_clear(id);
			}
		}
		break;

		case 0x2736: // ip address update
			if (RFIFOREST(fd) < 6) return 0;
			server[id].ip = ntohl(RFIFOL(fd, 2));
			ShowInfo("Updated IP address of map-server #%d to %d.%d.%d.%d.\n", id, CONVIP(server[id].ip));
			RFIFOSKIP(fd,6);
		break;

		default:
		{
			// inter server - packet
			int r = inter_parse_frommap(fd);
			if (r == 1) break;		// processed
			if (r == 2) return 0;	// need more packet

			// no inter server packet. no char server packet -> disconnect
			ShowError("Unknown packet 0x%04x from map server, disconnecting.\n", RFIFOW(fd,0));
			set_eof(fd);
			return 0;
		}
		} // switch
	} // while
	
	return 0;
}

void do_init_mapif(void)
{
	int i;
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		mapif_server_init(i);
}

void do_final_mapif(void)
{
	int i;
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		mapif_server_destroy(i);
}
