// $Id: chrif.c,v 1.6 2004/09/25 11:39:17 MouseJstr Exp $
#include "base.h"
#include "socket.h"
#include "timer.h"
#include "map.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "nullpo.h"
#include "utils.h"
#include "showmsg.h"

//Updated table (only doc^^) [Sirius]
//Used Packets: U->2af8 
//Free Packets: F->2af8 

static const int packet_len_table[0x3d] = {
	60, 3,-1,27,22,-1, 6,-1,	// 2af8-2aff: U->2af8, U->2af9, U->2afa, U->2afb, U->2afc, U->2afd, U->2afe, U->2aff
	 6,-1,18, 7,-1,49,44, 0,	// 2b00-2b07: U->2b00, U->2b01, U->2b02, U->2b03, U->2b04, U->2b05, U->2b06, F->2b07
	 6,30,-1,10,86, 7,44,34,	// 2b08-2b0f: U->2b08, U->2b09, U->2b0a, U->2b0b, U->2b0c, U->2b0d, U->2b0e, U->2b0f
	-1,-1,10, 6,11,-1, 0, 0,	// 2b10-2b17: U->2b10, U->2b11, U->2b12, U->2b13, U->2b14, U->2b15, U->2b16, U->2b17
	-1,-1,-1,-1,-1,-1,-1, 7,	// 2b18-2b1f: U->2b18, U->2b19, U->2b1a, U->2b1b, F->2b1c, F->2b1d, F->2b1e, U->2b1f
	-1,-1,-1,-1,-1,-1,-1,-1,	// 2b20-2b27: U->2b20, F->2b21, F->2b22, F->2b23, F->2b24, F->2b25, F->2b26, F->2b27
};

//Used Packets:
//2af8: Outgoing, chrif_connect -> 'connect to charserver / auth @ charserver' 
//2af9: Incomming, chrif_connectack -> 'awnser of the 2af8 login(ok / fail)' 
//2afa: Outgoing, chrif_sendmap -> 'sending our maps'
//2afb: Incomming, chrif_sendmapack -> 'Maps recived successfully / or not ..'
//2afc: Outgoing, chrif_authreq -> 'validate the incomming client' ? (not sure)
//2afd: Incomming, pc_authok -> 'ok awnser of the 2afc'
//2afe: Incomming, pc_authfail -> 'fail awnser of the 2afc' ? (not sure)
//2aff: Outgoing, send_users_tochar -> 'sends all actual connected charactersids to charserver'
//2b00: Incomming, map_setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure)
//2b01: Outgoing, chrif_save -> 'charsave of char XY account XY (complete struct)' 
//2b02: Outgoing, chrif_charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure)
//2b03: Incomming, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure)
//2b04: Incomming, chrif_recvmap -> 'getting maps from charserver of other mapserver's'
//2b05: Outgoing, chrif_changemapserver -> 'Tell the charserver the mapchange / quest for ok...' 
//2b06: Incomming, chrif_changemapserverack -> 'awnser of 2b05, ok/fail, data: dunno^^'
//2b07: FREE
//2b08: Outgoing, chrif_searchcharid -> '...'
//2b09: Incomming, map_addchariddb -> 'dunno^^'
//2b0a: Outgoing, chrif_changegm -> 'level change of acc/char XY'
//2b0b: Incomming, chrif_changedgm -> 'awnser of 2b0a..'
//2b0c: Outgoing, chrif_changeemail -> 'change mail address ...'
//2b0d: Incomming, chrif_changedsex -> 'Change sex of acc XY'
//2b0e: Outgoing, chrif_char_ask_name -> 'Do some operations (change sex, ban / unban etc)'
//2b0f: Incomming, chrif_char_ask_name_answer -> 'awnser of the 2b0e'
//2b10: Outgoing, chrif_saveaccountreg2 -> dunno? (register an account??)
//2b11: Outgoing, chrif_changesex -> 'change sex of acc X'
//2b12: Incomming, chrif_divorce -> 'divorce a wedding of charid X and partner id X'
//2b13: Incomming, chrif_accountdeletion -> 'Delete acc XX, if the player is on, kick ....'
//2b14: Incomming, chrif_accountban -> 'not sure: kick the player with message XY'
//2b15: Incomming, chrif_recvgmaccounts -> 'recive gm accs from charserver (seems to be incomplete !)'
//2b16: Outgoing, chrif_ragsrvinfo -> 'sends motd / rates ....'
//2b17: Outgoing, chrif_char_offline -> 'tell the charserver that the char is now offline'
//2b18: Outgoing, chrif_char_reset_offline -> 'set all players OFF!'
//2b19: Outgoing, chrif_char_online -> 'tell the charserver that the char .. is online'
//2b1a: Outgoing, chrif_reqfamelist -> 'Request the fame list (top10)'
//2b1b: Incomming, chrif_recvfamelist -> 'awnser of 2b1a ..... the famelist top10^^'
//2b1c: FREE
//2b1d: FREE
//2b1e: FREE
//2b1f: Incomming, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius]
//2b20: Incomming, chrif_removemap -> 'remove maps of a server (sample: its going offline)' [Sirius]
//2b21-2b27: FREE



TslistDST<CAuth> cAuthList;

int chrif_connected;
int char_fd = -1;

static char userid[24];
static char passwd[24];
static int chrif_state = 0;



bool getAthentification(unsigned long accid, CAuth& auth)
{
	size_t pos;
	if( cAuthList.find(CAuth(accid),0,pos) )
	{
		auth = cAuthList[pos];
		return true;
	}
	return false;
}


// 設定ファイル読み込み関係
/*==========================================
 *
 *------------------------------------------
 */
void chrif_setuserid(const char *id)
{
	memcpy(userid, id, sizeof(userid));
	userid[sizeof(userid)-1]=0;
}

/*==========================================
 *
 *------------------------------------------
 */
void chrif_setpasswd(const char *pwd)
{
	memcpy(passwd, pwd, sizeof(passwd));
	passwd[sizeof(passwd)-1]=0;
}


/*==========================================
 *
 *------------------------------------------
 */
int chrif_isconnect(void)
{
	return chrif_state == 2;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_save(struct map_session_data &sd)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	pc_makesavestatus(sd);

	WFIFOW(char_fd,0) = 0x2b01;
	WFIFOW(char_fd,2) = 12+sizeof(sd.status);
	WFIFOL(char_fd,4) = sd.bl.id;
	WFIFOL(char_fd,8) = sd.status.char_id;
	mmo_charstatus_tobuffer( sd.status, WFIFOP(char_fd,12) );
	WFIFOSET(char_fd, 12+sizeof(sd.status));
	storage_storage_dirty(sd);
	storage_storage_save(sd);

	return 0;
}
int chrif_save_sc(struct map_session_data &sd)
{
	static const unsigned short sc_array[] = 
	{
		0
/*		// non-saveable because using pointers to other structures
		SC_BLADESTOP,
		SC_DANCING,
		SC_BASILICA,
		SC_GRAVITATION,
		SC_FOGWALL,
		SC_POISON,
		SC_QUAGMIRE,
		SC_PNEUMA,
		SC_SAFETYWALL,
*/
/*		// possibly saveable, even not yet checked
		SC_PROVOKE,
		SC_SIGHT,
		SC_FREEZE,
		SC_STONE,
		SC_RUWACH,
		SC_INCREASEAGI,
		SC_DECREASEAGI,
		SC_SIGNUMCRUCIS,
		SC_ANGELUS,
		SC_BLESSING,
		SC_CONCENTRATE,
		SC_HIDING,
		SC_TWOHANDQUICKEN,
		SC_AUTOCOUNTER,
		SC_IMPOSITIO,
		SC_SUFFRAGIUM,
		SC_ASPERSIO,
		SC_BENEDICTIO,
		SC_SLOWPOISON,
		SC_KYRIE,
		SC_MAGNIFICAT,
		SC_GLORIA,
		SC_DIVINA,
		SC_AETERNA,
		SC_ADRENALINE,
		SC_WEAPONPERFECTION,
		SC_OVERTHRUST,
		SC_MAXIMIZEPOWER,
		SC_CLOAKING,
		SC_STAN,
		SC_ENCPOISON,
		SC_POISONREACT,
		SC_SPLASHER,
		SC_TRICKDEAD,
		SC_AUTOBERSERK,
		SC_LOUD,
		SC_ENERGYCOAT,
		SC_KEEPING,
		SC_BARRIER,
		SC_HALLUCINATION,
		SC_STRIPWEAPON,
		SC_STRIPSHIELD,
		SC_STRIPARMOR,
		SC_STRIPHELM,
		SC_CP_WEAPON,
		SC_CP_SHIELD,
		SC_CP_ARMOR,
		SC_CP_HELM,
		SC_AUTOGUARD,
		SC_REFLECTSHIELD,
		SC_DEVOTION,
		SC_PROVIDENCE,
		SC_DEFENDER,
		SC_SPEARSQUICKEN,
		SC_STEELBODY,
		SC_BLADESTOP_WAIT,
		SC_EXPLOSIONSPIRITS,
		SC_EXTREMITYFIST,
		SC_MAGICROD,
		SC_FLAMELAUNCHER,
		SC_FROSTWEAPON,
		SC_LIGHTNINGLOADER,
		SC_SEISMICWEAPON,
		SC_VOLCANO,
		SC_DELUGE,
		SC_VIOLENTGALE,
		SC_LANDPROTECTOR,
		SC_LULLABY,
		SC_RICHMANKIM,
		SC_ETERNALCHAOS,
		SC_DRUMBATTLE,
		SC_NIBELUNGEN,
		SC_ROKISWEIL,
		SC_INTOABYSS,
		SC_SIEGFRIED,
		SC_DISSONANCE,
		SC_WHISTLE,
		SC_ASSNCROS,
		SC_POEMBRAGI,
		SC_APPLEIDUN,
		SC_UGLYDANCE,
		SC_HUMMING,
		SC_DONTFORGETME,
		SC_FORTUNE,
		SC_SERVICE4U,
		SC_AURABLADE,
		SC_PARRYING,
		SC_CONCENTRATION,
		SC_TENSIONRELAX,
		SC_BERSERK,
		SC_BERSERK,
		SC_ASSUMPTIO,
		SC_MAGICPOWER,
		SC_SACRIFICE,
		SC_GOSPEL,
		SC_EDP,
		SC_TRUESIGHT,
		SC_WINDWALK,
		SC_MELTDOWN,
		SC_CARTBOOST,
		SC_CHASEWALK,
		SC_REJECTSWORD,
		SC_MOONLIT,
		SC_MARIONETTE,
		SC_BLEEDING,
		SC_JOINTBEAT,
		SC_MINDBREAKER,
		SC_MEMORIZE,
		SC_SPIDERWEB,
		SC_BABY,
		SC_PRESERVE,
		SC_DOUBLECAST,
		SC_MAXOVERTHRUST,
		SC_LONGING,
		SC_HERMODE
*/			};
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;
	size_t i, p=12;
	struct TimerData *td;
	unsigned long tick = gettick();


	WFIFOB(char_fd,p)=0;	
	for(i=0; i<MAX_STATUSCHANGE; i++)
	{
		if(sd.sc_data[i].timer != -1)
		{
			td = get_timer(sd.sc_data[i].timer);
			if(td && tick > td->tick)
				p += sprintf( (char*)WFIFOP(char_fd,p), "(%u,%lu)", i, (tick-td->tick));
		}
	}
	WFIFOW(char_fd,0) = 0x2b22;
	WFIFOW(char_fd,2) = p+1;
	WFIFOL(char_fd,4) = sd.bl.id;
	WFIFOL(char_fd,8) = sd.status.char_id;
	WFIFOSET(char_fd, p+1);
	return 0;
}
int chrif_read_sc(int fd)
{
	if( !session_isActive(fd) || !chrif_isconnect() )
		return -1;

	struct map_session_data *sd = map_charid2sd( RFIFOL(fd,8) );
	if(sd && sd->bl.id==RFIFOL(fd,4))
	{
		//char *p = (char *)RFIFOP(char_fd,12);
		//"(%i,%i)"
		//status_change_start(&sd->bl, type, ....
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_connect(int fd)
{
	if( !session_isActive(fd) )
		return -1;

	WFIFOW(fd,0) = 0x2af8;
	memcpy(WFIFOP(fd,2), userid, 24);
	memcpy(WFIFOP(fd,26), passwd, 24);
	WFIFOL(fd,50) = 0;

	WFIFOLIP(fd,54) = getmapaddress().LANIP();
	WFIFOLIP(fd,58) = getmapaddress().LANMask();
	WFIFOW(fd,62)   = getmapaddress().LANPort();
	WFIFOLIP(fd,64) = getmapaddress().WANIP();
	WFIFOW(fd,68)   = getmapaddress().WANPort();

	WFIFOSET(fd,70);
	return 0;
}

/*==========================================
 * マップ送信
 *------------------------------------------
 */
int chrif_sendmap(int fd)
{
	size_t i;
	
	if( !session_isActive(fd) )
		return -1;
	
	WFIFOW(fd,0) = 0x2afa;
	for(i = 0; i < map_num; i++) {
		if (map[i].alias != '\0') // [MouseJstr] map aliasing
			memcpy(WFIFOP(fd,4+i*16), map[i].alias, 16);
		else
			memcpy(WFIFOP(fd,4+i*16), map[i].mapname, 16);
	}
	WFIFOW(fd,2) = 4 + i * 16;
	WFIFOSET(fd,WFIFOW(fd,2));
	
	return 0;
}

/*==========================================
 * マップ受信
 *------------------------------------------
 */
int chrif_recvmap(int fd)
{
	int i, j;
	unsigned long ip;
	unsigned short port;

	if( !session_isActive(char_fd) || !chrif_isconnect() )	// まだ準備中
		return -1;

	ipset mapset( RFIFOLIP(fd,4), RFIFOLIP(fd,8),RFIFOW(fd,12), RFIFOLIP(fd,14), RFIFOW(fd,18) );

	ip = RFIFOLIP(fd,4);
	port = RFIFOW(fd,8);
	for(i = 20, j = 0; i < RFIFOW(fd,2); i += 16, j++)
	{
		map_setipport((char*)RFIFOP(fd,i), mapset);
	}
	if (battle_config.etc_log)
		ShowStatus("recv maps from %s (%d maps)\n", mapset.getstring(), j);

	return 0;
}

/*==========================================
 * Delete maps of other servers, (if an other mapserver is going OFF)
 *------------------------------------------
 */
int chrif_removemap(int fd)
{
	int i, j;
//	unsigned long ip;
//	unsigned short port;
	

	if( !session_isActive(fd) || !chrif_isconnect() )
		return -1;
	
//	ip = RFIFOLIP(fd, 4);
//	port = RFIFOW(fd, 8);

	ipset mapset( RFIFOLIP(fd,4), RFIFOLIP(fd,8),RFIFOW(fd,12), RFIFOLIP(fd,14), RFIFOW(fd,18) );
	for(i=20, j=0; i<RFIFOW(fd, 2); i+=16, j++)
	{
		map_eraseipport((char*)RFIFOP(fd, i), mapset);
	}
	if(battle_config.etc_log)
		ShowStatus("remove maps of server %s (%d maps)\n", mapset.getstring(), j);
	
	
	return 0;	
}
	
/*==========================================
 * マップ鯖間移動のためのデータ準備要求
 *------------------------------------------
 */
int chrif_changemapserver(struct map_session_data &sd, const char *name, unsigned short x, unsigned short y, ipset& mapset)
{
	size_t i;
	unsigned long s_ip=0;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	for(i = 0; i < fd_max; i++)
	{
		if (session[i] && session[i]->session_data == &sd)
		{
			s_ip = session[i]->client_ip;
			break;
		}
	}

	WFIFOW(char_fd, 0) = 0x2b05;
	WFIFOL(char_fd, 2) = sd.bl.id;
	WFIFOL(char_fd, 6) = sd.login_id1;
	WFIFOL(char_fd,10) = sd.login_id2;
	WFIFOL(char_fd,14) = sd.status.char_id;
	memcpy(WFIFOP(char_fd,18), name, 16);
	WFIFOW(char_fd,34) = x;
	WFIFOW(char_fd,36) = y;

	if( mapset.isLAN(s_ip) )
	{
		WFIFOLIP(char_fd,38) = mapset.LANIP();
		WFIFOL(char_fd,42) = mapset.LANPort();
	}
	else
	{
		WFIFOLIP(char_fd,38) = mapset.WANIP();
		WFIFOL(char_fd,42) = mapset.WANPort();
	}
	WFIFOB(char_fd,44) = sd.status.sex;
	WFIFOLIP(char_fd,45) = s_ip;
	WFIFOSET(char_fd,49);

	return 0;
}

/*==========================================
 * マップ鯖間移動ack
 *------------------------------------------
 */
int chrif_changemapserverack(int fd)
{

	if( !session_isActive(fd)  )
		return -1;

	struct map_session_data *sd = map_id2sd(RFIFOL(fd,2));

	if( sd == NULL || RFIFOL(fd,14) != sd->status.char_id )
		return -1;

	if( RFIFOL(fd,6) == 1 )
	{
		if (battle_config.error_log)
			ShowMessage("map server change failed.\n");
		pc_authfail(sd->fd);
		return 0;
	}
	clif_changemapserver(*sd, (char*)RFIFOP(fd,18), RFIFOW(fd,34), RFIFOW(fd,36), RFIFOLIP(fd,38), RFIFOW(fd,42));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_connectack(int fd)
{
	static int char_init_done=0;

	if( !session_isActive(fd) )
		return -1;

	if (RFIFOB(fd,2)) {
		ShowMessage("Connected to char-server failed %d.\n", (unsigned char)RFIFOB(fd,2));
		exit(1);
	}
	ShowStatus("Successfully connected to Char Server (Connection: '"CL_WHITE"%d"CL_RESET"').\n",fd);
	chrif_state = 1;

	chrif_sendmap(fd);

	ShowStatus("Event '"CL_WHITE"OnCharIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnCharIfInit"));
	ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInit"));
	if(!char_init_done) {
		char_init_done = 1;
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInitOnce"));
		ShowStatus("Event '"CL_WHITE"OnAgitInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnAgitInit"));
	}


	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_sendmapack(int fd)
{

	if( !session_isActive(fd) )
		return -1;

	if (RFIFOB(fd,2))
	{
		ShowMessage("chrif : send map list to char server failed %d\n", (unsigned char)RFIFOB(fd,2));
		session_Remove(char_fd); 
	}
	else
	{
		memcpy(wisp_server_name, RFIFOP(fd,3), 24);
		chrif_state = 2;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_authreq(struct map_session_data &sd)
{
	size_t i;

	if(!sd.bl.id || !sd.login_id1)
		return -1;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	for(i = 0; i < fd_max; i++)
		if( session[i] && session[i]->session_data == &sd)
		{
			WFIFOW(char_fd, 0) = 0x2afc;
			WFIFOL(char_fd, 2) = sd.bl.id;
			WFIFOL(char_fd, 6) = sd.status.char_id;
			WFIFOL(char_fd,10) = sd.login_id1;
			WFIFOL(char_fd,14) = sd.login_id2;
			WFIFOLIP(char_fd,18) = session[i]->client_ip;
			WFIFOSET(char_fd,22);
			break;
		}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_charselectreq(struct map_session_data &sd)
{
	size_t i; 
	unsigned long s_ip;

	if( !sd.bl.id || !sd.login_id1 )
		return -1;
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	s_ip = 0;
	for(i = 0; i < fd_max; i++)
		if (session[i] && session[i]->session_data == &sd)
		{
			s_ip = session[i]->client_ip;
			break;
		}

	WFIFOW(char_fd, 0) = 0x2b02;
	WFIFOL(char_fd, 2) = sd.bl.id;
	WFIFOL(char_fd, 6) = sd.login_id1;
	WFIFOL(char_fd,10) = sd.login_id2;
	WFIFOLIP(char_fd,14) = s_ip;
	WFIFOSET(char_fd,18);

	return 0;
}

/*==========================================
 * キャラ名問い合わせ
 *------------------------------------------
 */
int chrif_searchcharid(unsigned long id)
{
	if( !id )
		return -1;
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b08;
	WFIFOL(char_fd,2) = id;
	WFIFOSET(char_fd,6);

	return 0;
}

/*==========================================
 * GMに変化要求
 *------------------------------------------
 */
int chrif_changegm(unsigned long id, const char *pass, size_t len)
{
	if (battle_config.etc_log)
		ShowMessage("chrif_changegm: account: %d, password: '%s'.\n", id, pass);

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b0a;
	WFIFOW(char_fd,2) = len + 8;
	WFIFOL(char_fd,4) = id;
	memcpy(WFIFOP(char_fd,8), pass, len);
	WFIFOSET(char_fd, len + 8);

	return 0;
}

/*==========================================
 * Change Email
 *------------------------------------------
 */
int chrif_changeemail(unsigned long id, const char *actual_email, const char *new_email)
{
	if (battle_config.etc_log)
		ShowMessage("chrif_changeemail: account: %d, actual_email: '%s', new_email: '%s'.\n", id, actual_email, new_email);

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b0c;
	WFIFOL(char_fd,2) = id;
	memcpy(WFIFOP(char_fd,6), actual_email, 40);
	memcpy(WFIFOP(char_fd,46), new_email, 40);
	WFIFOSET(char_fd,86);

	return 0;
}

/*==========================================
 * Send message to char-server with a character name to do some operations (by Yor)
 * Used to ask Char-server about a character name to have the account number to modify account file in login-server.
 * type of operation:
 *   1: block
 *   2: ban
 *   3: unblock
 *   4: unban
 *   5: changesex
 *------------------------------------------
 */
int chrif_char_ask_name(long id, const char *character_name, unsigned short operation_type, unsigned short year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second)
{
   	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd, 0) = 0x2b0e;
	WFIFOL(char_fd, 2) = id; // account_id of who ask (for answer) -1 if nobody
	memcpy(WFIFOP(char_fd,6), character_name, 24);
	WFIFOW(char_fd, 30) = operation_type; // type of operation
	if (operation_type == 2)
	{
		WFIFOW(char_fd, 32) = year;
		WFIFOW(char_fd, 34) = month;
		WFIFOW(char_fd, 36) = day;
		WFIFOW(char_fd, 38) = hour;
		WFIFOW(char_fd, 40) = minute;
		WFIFOW(char_fd, 42) = second;
	}
	ShowMessage("chrif : sended 0x2b0e\n");
	WFIFOSET(char_fd,44);

	return 0;
}

/*==========================================
 * 性別変化要求
 *------------------------------------------
 */
int chrif_changesex(unsigned long id, unsigned char sex)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b11;
	WFIFOW(char_fd,2) = 9;
	WFIFOL(char_fd,4) = id;
	WFIFOB(char_fd,8) = sex;
	ShowMessage("chrif : sent 0x3000(changesex)\n");
	WFIFOSET(char_fd,9);
	return 0;
}

/*==========================================
 * Answer after a request about a character name to do some operations (by Yor)
 * Used to answer of chrif_char_ask_name.
 * type of operation:
 *   1: block
 *   2: ban
 *   3: unblock
 *   4: unban
 *   5: changesex
 * type of answer:
 *   0: login-server resquest done
 *   1: player not found
 *   2: gm level too low
 *   3: login-server offline
 *------------------------------------------
 */
int chrif_char_ask_name_answer(int fd)
{
	int acc;
	struct map_session_data *sd;
	char output[256];
	char player_name[24];

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2); // account_id of who has asked (-1 if nobody)
	memcpy(player_name, RFIFOP(fd,6), sizeof(player_name));
	player_name[sizeof(player_name)-1] = '\0';

	sd = map_id2sd(acc);
	if (acc >= 0 && sd != NULL)
	{
		if (RFIFOW(fd, 32) == 1) // player not found
			sprintf(output, "The player '%s' doesn't exist.", player_name);
		else {
			switch(RFIFOW(fd, 30)) {
			case 1: // block
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					sprintf(output, "Login-server has been asked to block the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					sprintf(output, "Your GM level don't authorise you to block the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					sprintf(output, "Login-server is offline. Impossible to block the the player '%s'.", player_name);
					break;
				}
				break;
			case 2: // ban
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					sprintf(output, "Login-server has been asked to ban the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					sprintf(output, "Your GM level don't authorise you to ban the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					sprintf(output, "Login-server is offline. Impossible to ban the the player '%s'.", player_name);
					break;
				}
				break;
			case 3: // unblock
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					sprintf(output, "Login-server has been asked to unblock the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					sprintf(output, "Your GM level don't authorise you to unblock the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					sprintf(output, "Login-server is offline. Impossible to unblock the the player '%s'.", player_name);
					break;
				}
				break;
			case 4: // unban
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					sprintf(output, "Login-server has been asked to unban the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					sprintf(output, "Your GM level don't authorise you to unban the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					sprintf(output, "Login-server is offline. Impossible to unban the the player '%s'.", player_name);
					break;
				}
				break;
			case 5: // changesex
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					sprintf(output, "Login-server has been asked to change the sex of the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					sprintf(output, "Your GM level don't authorise you to change the sex of the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					sprintf(output, "Login-server is offline. Impossible to change the sex of the the player '%s'.", player_name);
					break;
				}
				break;
			}
		}
		if (output[0] != '\0') {
			output[sizeof(output)-1] = '\0';
			clif_displaymessage(sd->fd, output);
		}
	} else
		ShowMessage("chrif_char_ask_name_answer failed - player not online.\n");

	return 0;
}

/*==========================================
 * End of GM change (@GM) (modified by Yor)
 *------------------------------------------
 */
int chrif_changedgm(int fd)
{
	int acc, level;
	struct map_session_data *sd = NULL;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	level = RFIFOL(fd,6);

	sd = map_id2sd(acc);

	if (battle_config.etc_log)
		ShowMessage("chrif_changedgm: account: %d, GM level 0 -> %d.\n", acc, level);
	if (sd != NULL) {
		if (level > 0)
			clif_displaymessage(sd->fd, "GM modification success.");
		else
			clif_displaymessage(sd->fd, "Failure of GM modification.");
	}
	return 0;
}

/*==========================================
 * 性別変化終了 (modified by Yor)
 *------------------------------------------
 */
int chrif_changedsex(int fd)
{
	int acc, sex, i;
	struct map_session_data *sd;
	struct pc_base_job s_class;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	sex = RFIFOL(fd,6);
	if (battle_config.etc_log)
		ShowMessage("chrif_changedsex %d.\n", acc);
	sd = map_id2sd(acc);
	if (acc > 0)
	{
		if (sd != NULL && sd->status.sex != sex) {
			s_class = pc_calc_base_job(sd->status.class_);
			if (sd->status.sex == 0) {
				sd->status.sex = 1;
			} else if (sd->status.sex == 1) {
				sd->status.sex = 0;
			}
			// to avoid any problem with equipment and invalid sex, equipment is unequiped.
			for (i = 0; i < MAX_INVENTORY; i++) {
				if (sd->status.inventory[i].nameid && sd->status.inventory[i].equip)
					pc_unequipitem(*((struct map_session_data*)sd), i, 2);
			}
			// reset skill of some job
			if (s_class.job == 19 || s_class.job == 4020 || s_class.job == 4042 ||
			    s_class.job == 20 || s_class.job == 4021 || s_class.job == 4043) {
				// remove specifical skills of classes 19, 4020 and 4042
				for(i = 315; i <= 322; i++) {
					if (sd->status.skill[i].id > 0 && !sd->status.skill[i].flag) {
						sd->status.skill_point += sd->status.skill[i].lv;
						sd->status.skill[i].id = 0;
						sd->status.skill[i].lv = 0;
					}
				}
				// remove specifical skills of classes 20, 4021 and 4043
				for(i = 323; i <= 330; i++) {
					if (sd->status.skill[i].id > 0 && !sd->status.skill[i].flag) {
						sd->status.skill_point += sd->status.skill[i].lv;
						sd->status.skill[i].id = 0;
						sd->status.skill[i].lv = 0;
					}
				}
				clif_updatestatus(*sd, SP_SKILLPOINT);
				// change job if necessary
				if (s_class.job == 20 || s_class.job == 4021 || s_class.job == 4043)
					sd->status.class_ -= 1;
				else if (s_class.job == 19 || s_class.job == 4020 || s_class.job == 4042)
					sd->status.class_ += 1;
			}
			// save character
			chrif_save(*sd);
			sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
			                 // do same modify in login-server for the account, but no in char-server (it ask again login_id1 to login, and don't remember it)
			clif_displaymessage(sd->fd, "Your sex has been changed (need disconnection by the server)...");
			session_SetWaitClose(sd->fd, 5000); // forced to disconnect for the change
		}
	} else {
		if (sd != NULL) {
			ShowMessage("chrif_changedsex failed.\n");
		}
	}
	return 0;
}

/*==========================================
 * アカウント変数保存要求
 *------------------------------------------
 */
int chrif_saveaccountreg2(struct map_session_data &sd)
{
	unsigned short p;
	unsigned long j;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	p = 8;
	for(j = 0; j < sd.status.account_reg2_num; j++) {
		if( sd.status.account_reg2[j].str[0] && sd.status.account_reg2[j].value != 0) {
			memcpy(WFIFOP(char_fd,p), sd.status.account_reg2[j].str, 32);
			WFIFOL(char_fd,p+32) = sd.status.account_reg2[j].value;
			p += 36;
		}
	}
	WFIFOW(char_fd,0) = 0x2b10;
	WFIFOW(char_fd,2) = p;
	WFIFOL(char_fd,4) = sd.bl.id;
	WFIFOSET(char_fd,p);

	return 0;
}

/*==========================================
 * アカウント変数通知
 *------------------------------------------
 */
int chrif_accountreg2(int fd)
{
	int j, p;
	struct map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	if ((sd = map_id2sd(RFIFOL(fd,4))) == NULL)
		return 1;

	for(p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, j++) {
		memcpy(sd->status.account_reg2[j].str, RFIFOP(fd,p), 32);
		sd->status.account_reg2[j].value = RFIFOL(fd, p + 32);
	}
	sd->status.account_reg2_num = j;

	return 0;
}

/*==========================================
 * 離婚情報同期要求
 *------------------------------------------
 */
int chrif_divorce(unsigned long char_id, unsigned long partner_id)
{
	struct map_session_data *sd = NULL;

	if (!char_id || !partner_id)
		return 0;

	nullpo_retr(0, sd = map_nick2sd(map_charid2nick(partner_id)));
	if (sd->status.partner_id == char_id) {
		int i;
		//離婚(相方は既にキャラが消えている筈なので)
		sd->status.partner_id = 0;

		//相方の結婚指輪を剥奪
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(*sd, i, 1, 0);
	}

	return 0;
}

/*==========================================
 * Disconnection of a player (account has been deleted in login-server) by [Yor]
 *------------------------------------------
 */
int chrif_accountdeletion(int fd)
{
	int acc;
	struct map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	if (battle_config.etc_log)
		ShowMessage("chrif_accountdeletion %d.\n", acc);
	sd = map_id2sd(acc);
	if (acc > 0) {
		if (sd != NULL) {
			sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
			clif_displaymessage(sd->fd, "Your account has been deleted (disconnection)...");
			session_SetWaitClose(sd->fd, 5000); // forced to disconnect for the change
		}
	} else {
		if (sd != NULL)
			ShowMessage("chrif_accountdeletion failed - player not online.\n");
	}

	return 0;
}

/*==========================================
 * Disconnection of a player (account has been banned of has a status, from login-server) by [Yor]
 *------------------------------------------
 */
int chrif_accountban(int fd)
{
	int acc;
	struct map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	if (battle_config.etc_log)
		ShowMessage("chrif_accountban %d.\n", acc);
	sd = map_id2sd(acc);
	if (acc > 0) {
		if (sd != NULL) {
			sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
			if (RFIFOB(fd,6) == 0) { // 0: change of statut, 1: ban
				switch (RFIFOL(fd,7)) { // status or final date of a banishment
				case 1:   // 0 = Unregistered ID
					clif_displaymessage(sd->fd, "Your account has 'Unregistered'.");
					break;
				case 2:   // 1 = Incorrect Password
					clif_displaymessage(sd->fd, "Your account has an 'Incorrect Password'...");
					break;
				case 3:   // 2 = This ID is expired
					clif_displaymessage(sd->fd, "Your account has expired.");
					break;
				case 4:   // 3 = Rejected from Server
					clif_displaymessage(sd->fd, "Your account has been rejected from server.");
					break;
				case 5:   // 4 = You have been blocked by the GM Team
					clif_displaymessage(sd->fd, "Your account has been blocked by the GM Team.");
					break;
				case 6:   // 5 = Your Game's EXE file is not the latest version
					clif_displaymessage(sd->fd, "Your Game's EXE file is not the latest version.");
					break;
				case 7:   // 6 = Your are Prohibited to log in until %s
					clif_displaymessage(sd->fd, "Your account has been prohibited to log in.");
					break;
				case 8:   // 7 = Server is jammed due to over populated
					clif_displaymessage(sd->fd, "Server is jammed due to over populated.");
					break;
				case 9:   // 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
					clif_displaymessage(sd->fd, "Your account has not more authorised.");
					break;
				case 100: // 99 = This ID has been totally erased
					clif_displaymessage(sd->fd, "Your account has been totally erased.");
					break;
				default:
					clif_displaymessage(sd->fd, "Your account has not more authorised.");
					break;
				}
			} else if (RFIFOB(fd,6) == 1) { // 0: change of statut, 1: ban
				time_t timestamp;
				char tmpstr[2048];
				timestamp = (time_t)RFIFOL(fd,7); // status or final date of a banishment
				strcpy(tmpstr, "Your account has been banished until ");
				strftime(tmpstr + strlen(tmpstr), 24, "%d-%m-%Y %H:%M:%S", localtime(&timestamp));
				clif_displaymessage(sd->fd, tmpstr);
			}
			session_SetWaitClose(sd->fd, 5000); // forced to disconnect for the change
		}
	} else {
		if (sd != NULL)
			ShowMessage("chrif_accountban failed - player not online.\n");
	}

	return 0;
}

//Disconnect the player out of the game, simple packet
//packet.w AID.L WHY.B 2+4+1 = 7byte
int chrif_disconnectplayer(int fd)
{
	struct map_session_data *sd;
	
	sd = map_id2sd(RFIFOL(fd, 2));
	
	if(RFIFOL(fd, 2) <= 0 || sd == NULL){
		return -1;
	}
	
	//change sessid1 
	sd->login_id1++;

	switch(RFIFOB(fd, 6))
	{
	//clif_authfail_fd
	case 1: //server closed 
		clif_authfail_fd(fd, 1);	
		break;
	case 2: //someone else logged in
		clif_authfail_fd(fd, 2);		
		break;
	case 3: //server overpopulated
		clif_authfail_fd(fd, 4);
		break;
	case 4: //out of time payd for .. (avail)
		clif_authfail_fd(fd, 10);
		break;
	case 5: //forced to dc by gm
		clif_authfail_fd(fd, 15);
		break;
	}
	return 0;
}



/*==========================================
 * Request to reload GM accounts and their levels: send to char-server by [Yor]
 *------------------------------------------
 */
int chrif_reloadGMdb(void)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2af7;
	WFIFOSET(char_fd, 2);

	return 0;
}

/*==========================================
 * Receiving GM accounts and their levels from char-server by [Yor]
 *------------------------------------------
 */
int chrif_recvgmaccounts(int fd)
{
	ShowInfo("From login-server: receiving information of '"CL_WHITE"%d"CL_RESET"' GM accounts.\n", pc_read_gm_account(fd));
	return 0;
}

/*==========================================
 * Request/Receive top 10 Fame character list
 *------------------------------------------
 */
int chrif_reqfamelist(void)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b1a;
	WFIFOSET(char_fd, 2);

	return 0;
}
int chrif_recvfamelist(int fd)
{
	int id;
	size_t fame;
	size_t i, num, size;
	size_t total = 0;
	char *name;

	if( !session_isActive(fd) )
		return -1;

	// response from 0x2b1b
	memset (smith_fame_list, 0, sizeof(smith_fame_list));
	memset (chemist_fame_list, 0, sizeof(chemist_fame_list));

	size = RFIFOW(fd,4);
	for (i=6, num=0; i<size && num < MAX_FAMELIST; i+=32, num++)
	{
		id   = RFIFOL(fd,i);
		fame = RFIFOL(fd,i+4);
		name = (char*)RFIFOP(fd,i+8);
		if( id > 0 && fame > 0 && num < MAX_FAMELIST)
		{
			smith_fame_list[num].id = id;
			smith_fame_list[num].fame = fame;
			memcpy(smith_fame_list[num].name, name, 24);
			//ShowMessage("received : %s (id:%d) fame:%d\n", name, id, fame);
		}
	}
	total += num;

	i = size;
	size = RFIFOW(fd,2);
	for (num=0; i<size && num < MAX_FAMELIST; i+=32, num++)
	{
		id = RFIFOL(fd,i);
		fame = RFIFOL(fd,i+4);
		name = (char*)RFIFOP(fd,i+8);
		if( id > 0 && fame > 0 && num < MAX_FAMELIST)
		{
			chemist_fame_list[num].id = id;
			chemist_fame_list[num].fame = fame;
			memcpy(chemist_fame_list[num].name, name, 24);
			//ShowMessage("received : %s (id:%d) fame:%d\n", name, id, fame);
		}
	}
	total += num;

	if(battle_config.etc_log)
		ShowInfo("Receiving Fame List of '"CL_WHITE"%d"CL_RESET"' characters.\n", total);
	return 0;
}

/*==========================================
 * Send rates and motd to char server [Wizputer]
 *------------------------------------------
 */
 int chrif_ragsrvinfo(unsigned short base_rate, unsigned short job_rate, unsigned short drop_rate)
{
	char buf[256];
	FILE *fp;
	int i;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b16;
	WFIFOW(char_fd,2) = base_rate;
	WFIFOW(char_fd,4) = job_rate;
	WFIFOW(char_fd,6) = drop_rate;

	if( (fp = safefopen(motd_txt, "r")) != NULL) {
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			for(i = 0; buf[i]; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = 0;
					break;
				}
			}
			WFIFOW(char_fd,8) = sizeof(buf) + 10;
			memcpy(WFIFOP(char_fd,10), buf, sizeof(buf));
		}
		fclose(fp);
	} else {
		memset(buf, 0, sizeof(buf));
		WFIFOW(char_fd,8) = sizeof(buf) + 10;
		memcpy(WFIFOP(char_fd,10), buf, sizeof(buf));
	}
	WFIFOSET(char_fd,WFIFOW(char_fd,8));
	return 0;
}


/*=========================================
 * Tell char-server charcter disconnected [Wizputer]
 *-----------------------------------------
 */

int chrif_char_offline(struct map_session_data &sd)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b17;
	WFIFOL(char_fd,2) = sd.status.char_id;
	WFIFOL(char_fd,6) = sd.status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------
 */
int chrif_flush_fifo(void)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	flush_fifos();

	return 0;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------
 */
int chrif_char_reset_offline(void)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b18;
	WFIFOSET(char_fd,2);

	return 0;
}

/*=========================================
 * Tell char-server charcter is online [Wizputer]
 *-----------------------------------------
 */

int chrif_char_online(struct map_session_data &sd)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b19;
	WFIFOL(char_fd,2) = sd.status.char_id;
	WFIFOL(char_fd,6) = sd.status.account_id;
	WFIFOSET(char_fd,10);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
/*
int chrif_disconnect_sub(struct map_session_data& sd, va_list va)
{
	clif_authfail_fd(sd.fd,1);
	//map_quit(*sd);
	return 0;
}

int chrif_disconnect(int fd)
{
	if(fd == char_fd)
	{
		ShowWarning("Map Server disconnected from Char Server.\n\n");
		//clif_foreachclient(chrif_disconnect_sub);
		chrif_state = 0;
		char_fd = -1;
		session_Remove(fd);
		// 他のmap 鯖のデータを消す
		map_eraseallipport();

		// 倉庫キャッシュを消す
		do_final_storage();
		do_init_storage();
	}
	return 0;
}
*/
/*==========================================
 *
 *------------------------------------------
 */
int chrif_parse(int fd)
{
	int packet_len; 
	unsigned short cmd;
	// only char-server can have an access to here.
	// so, if it isn't the char-server, we disconnect the session (fd != char_fd).
	if (fd != char_fd)
	{
		session_Remove(fd);
		return 0;
	}
	// else it is the char_fd
	if( !session_isActive(fd) )
	{	// char server is disconnecting
		if(  chrif_isconnect() )
		{
			ShowWarning("Connection to Char Server dropped.\n");
			chrif_state=0;
		}
		char_fd = -1;
		session_Remove(fd);// have it removed by do_sendrecv

		// 他のmap 鯖のデータを消す
		map_eraseallipport();

		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		//ShowMessage("chrif_parse: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
		cmd = RFIFOW(fd,0);
		if (cmd < 0x2af8 || cmd >= 0x2af8 + (sizeof(packet_len_table) / sizeof(packet_len_table[0])) ||
		    packet_len_table[cmd-0x2af8] == 0) {

			int r = intif_parse(fd); // intifに渡す

			if (r == 1) continue;	// intifで処理した
			if (r == 2) return 0;	// intifで処理したが、データが足りない

			session_Remove(fd);
			ShowWarning("chrif_parse: session #%d, intif_parse failed -> disconnected.\n", fd);
			return 0;
		}
		packet_len = packet_len_table[cmd-0x2af8];
		if(packet_len < 0) {
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd,2);
		}
		if (RFIFOREST(fd) < packet_len)
			return 0;

		switch(cmd)
		{
		case 0x2af9: chrif_connectack(fd); break;
		case 0x2afb: chrif_sendmapack(fd); chrif_reqfamelist(); break;
		case 0x2afd: 
		{
			pc_authok(RFIFOL(fd,4), RFIFOL(fd,8), (time_t)RFIFOL(fd,12), RFIFOP(fd,16)); 
			// typecast is bad but only memcopied internally, so it might be ok
			break;
		}
        case 0x2afe: pc_authfail(RFIFOL(fd,2)); break;
		case 0x2b00: map_setusers(fd); break;
		case 0x2b03: clif_charselectok(RFIFOL(fd,2)); break;
		case 0x2b04: chrif_recvmap(fd); break;
		case 0x2b06: chrif_changemapserverack(fd); break;
		case 0x2b09: map_addchariddb(RFIFOL(fd,2), (char*)RFIFOP(fd,6)); break;
		case 0x2b0b: chrif_changedgm(fd); break;
		case 0x2b0d: chrif_changedsex(fd); break;
		case 0x2b0f: chrif_char_ask_name_answer(fd); break;
		case 0x2b11: chrif_accountreg2(fd); break;
		case 0x2b12: chrif_divorce(RFIFOL(fd,2), RFIFOL(fd,6)); break;
		case 0x2b13: chrif_accountdeletion(fd); break;
		case 0x2b14: chrif_accountban(fd); break;
		case 0x2b15: chrif_recvgmaccounts(fd); break;
		case 0x2b1b: chrif_recvfamelist(fd); break;
		case 0x2b1f: chrif_disconnectplayer(fd); break;
		case 0x2b20: chrif_removemap(fd); break; //Remove maps of a server [Sirius]

		case 0x2b21:
		{
			size_t pos;
			CAuth auth;
			auth.frombuffer(RFIFOP(fd,4));
			if( cAuthList.find(auth,0,pos) )
				cAuthList[pos]=auth;
			else
				cAuthList.insert(auth);

			break;
		}
		case 0x2b22: chrif_read_sc(fd); break;
		default:
			if (battle_config.error_log)
				ShowMessage("chrif_parse : unknown packet %d %d\n", fd, (unsigned short)RFIFOW(fd,0));
			session_Remove(fd);
			return 0;
		}
		RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

/*==========================================
 * timer関数
 * 今このmap鯖に繋がっているクライアント人数をchar鯖へ送る
 *------------------------------------------
 */
int send_users_tochar(int tid, unsigned long tick, int id, intptr data) {
	size_t i;
	unsigned short users = 0;
	struct map_session_data *sd;

	if( !session_isActive(char_fd) || !chrif_isconnect() ) // Thanks to Toster
		return 0;

	WFIFOW(char_fd,0) = 0x2aff;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (sd = (struct map_session_data*)session[i]->session_data) && sd->state.auth &&
		    !((battle_config.hide_GM_session || (sd->status.option & OPTION_HIDE)) && pc_isGM(*sd))) {
			WFIFOL(char_fd,6+4*users) = sd->status.char_id;
			users++;
		}
	}
	WFIFOW(char_fd,2) = 6 + 4 * users;
	WFIFOW(char_fd,4) = users;
	WFIFOSET(char_fd,6+4*users);

	return 0;
}

/*==========================================
 * timer関数
 * char鯖との接続を確認し、もし切れていたら再度接続する
 *------------------------------------------
 */
int check_connect_char_server(int tid, unsigned long tick, int id, intptr data)
{
	if( !session_isActive(char_fd) )
	{
		//clif_foreachclient(chrif_disconnect_sub);
		chrif_state = 0;

		ShowStatus("Attempting to connect to Char Server. Please wait.\n");
		char_fd = make_connection(getcharaddress().addr(), getcharaddress().port());
		if( session_isActive(char_fd) )
		{
			session[char_fd]->func_parse = chrif_parse;
			realloc_fifo(char_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

			chrif_connect(char_fd);
#ifndef TXT_ONLY
			chrif_ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
#endif /* not TXT_ONLY */
		}
	}
	return 0;
}
/*==========================================
 * 終了
 *------------------------------------------
 */
int do_final_chrif(void)
{
	session_Remove(char_fd);
	char_fd = -1;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int do_init_chrif(void)
{
	add_timer_func_list(check_connect_char_server, "check_connect_char_server");
	add_timer_func_list(send_users_tochar, "send_users_tochar");
	add_timer_interval(gettick() + 1000, 10 * 1000, check_connect_char_server, 0, 0);
	add_timer_interval(gettick() + 1000, 5 * 1000, send_users_tochar, 0, 0);

	return 0;
}
