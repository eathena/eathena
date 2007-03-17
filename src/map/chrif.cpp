// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "socket.h"
#include "timer.h"
#include "nullpo.h"
#include "utils.h"
#include "showmsg.h"

#include "atcommand.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "mob.h"
#include "pet.h"
#include "status.h"
#include "itemdb.h"

static const int packet_len_table[] = {
	70,	// 2af8: Outgoing, chrif_connect -> 'connect to charserver / auth @ charserver' 
	 3,	// 2af9: Incomming, chrif_connectack -> 'awnser of the 2af8 login(ok / fail)' 
	-1,	// 2afa: Outgoing, chrif_sendmap -> 'sending our maps'
	27,	// 2afb: Incomming, chrif_sendmapack -> 'Maps recived successfully / or not ..'
	22,	// 2afc: Outgoing, chrif_authreq -> 'validate the incomming client' ? (not sure)
	-1,	// 2afd: Incomming, pc_authok -> 'ok awnser of the 2afc'
	 6,	// 2afe: Incomming, pc_authfail -> 'fail awnser of the 2afc' ? (not sure)
	-1,	// 2aff: Outgoing, send_users_tochar -> 'sends all actual connected charactersids to charserver'
	 6,	// 2b00: Incomming, map_setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure)
	-1,	// 2b01: Outgoing, chrif_save -> 'charsave of char XY account XY (complete struct)' 
	18,	// 2b02: Outgoing, chrif_charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure)
	 7,	// 2b03: Incomming, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure)
	-1,	// 2b04: Incomming, chrif_recvmap -> 'getting maps from charserver of other mapserver's'
	49,	// 2b05: Outgoing, chrif_changemapserver -> 'Tell the charserver the mapchange / quest for ok...' 
	44,	// 2b06: Incomming, chrif_changemapserverack -> 'awnser of 2b05, ok/fail, data: dunno^^'
	 0,	// 2b07: FREE
	 6,	// 2b08: Outgoing, chrif_searchcharid -> '...'
	30,	// 2b09: Incomming, map_addchariddb -> 'dunno^^'
	-1,	// 2b0a: Outgoing, chrif_changegm -> 'level change of acc/char XY'
	10,	// 2b0b: Incomming, chrif_changedgm -> 'awnser of 2b0a..'
	86,	// 2b0c: Outgoing, chrif_changeemail -> 'change mail address ...'
	 7,	// 2b0d: Incomming, chrif_changedsex -> 'Change sex of acc XY'
	44,	// 2b0e: Outgoing, chrif_char_ask_name -> 'Do some operations (change sex, ban / unban etc)'
	34,	// 2b0f: Incomming, chrif_char_ask_name_answer -> 'awnser of the 2b0e'
	-1,	// 2b10: Outgoing, chrif_saveaccountreg2 -> dunno? (register an account??)
	-1,	// 2b11: Outgoing, chrif_changesex -> 'change sex of acc X'
	10,	// 2b12: Incomming, chrif_divorce -> 'divorce a wedding of charid X and partner id X'
	 6,	// 2b13: Incomming, chrif_accountdeletion -> 'Delete acc XX, if the player is on, kick ....'
	11,	// 2b14: Incomming, chrif_accountban -> 'not sure: kick the player with message XY'
	-1,	// 2b15: Incomming, 
	 0,	// 2b16: Outgoing, chrif_ragsrvinfo -> 'sends motd / rates ....'
	 0,	// 2b17: Outgoing, chrif_char_offline -> 'tell the charserver that the char is now offline'
	-1,	// 2b18: Outgoing, chrif_char_reset_offline -> 'set all players OFF!'
	-1,	// 2b19: Outgoing, chrif_char_online -> 'tell the charserver that the char .. is online'
	-1,	// 2b1a: Outgoing, chrif_updatefame
	-1,	// 2b1b: Incomming, chrif_recvfamelist -> 'awnser of 2b1a ..... the famelist top10^^'
	-1,	// 2b1c: FREE
	-1,	// 2b1d: FREE
	-1,	// 2b1e: FREE
	 7,	// 2b1f: Incomming, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius]
	-1,	// 2b20: Incomming, chrif_removemap -> 'remove maps of a server (sample: its going offline)' [Sirius]
	-1,	// 2b21: Auth in/out (out not used)
	-1,	// 2b22: SC in/out
	-1,	// 2b23: check mail in/out
	-1,	// 2b24: fetch mail in/out
	-1,	// 2b25: read mail in/out
	-1,	// 2b26: delete mail in/out
	-1,	// 2b27: send mail in/out

	-1,	// 2b28: get mail appendix in/out (in not used)
	-1,	// 2b29:
	-1,	// 2b2a:
	-1,	// 2b2b:
	-1,	// 2b2c:
	-1,	// 2b2d:
	-1,	// 2b2e:
	-1,	// 2b2f:

	-1,	// 2b30: req variable out
	-1,	// 2b31: transports variables in/out
	-1,	// 2b32: 
	-1,	// 2b33: 
	-1,	// 2b34: 
	-1,	// 2b35: 
	-1,	// 2b36: 
	-1,	// 2b37: 

	-1,	// 2b38: irc announce in/out
	-1,	// 2b39:
	-1,	// 2b3a:
	-1,	// 2b3b:
	-1,	// 2b3c:
	-1,	// 2b3d:
	-1,	// 2b3e:
	-1,	// 2b3f:
};


basics::slist<CAuth> cAuthList;

CFameList famelists[4];


int chrif_connected;
int char_fd = -1;

static char userid[24];
static char passwd[24];
static int chrif_state = 0;





bool getAthentification(uint32 accid, CAuth& auth)
{
	size_t pos;
	if( cAuthList.find(CAuth(accid),0,pos) )
	{
		auth = cAuthList[pos];
		return true;
	}
	return false;
}
void chrif_parse_StoreAthentification(int fd)
{
	if( session_isActive(fd) )
	{
		size_t pos;
		CAuth auth;
		auth.frombuffer(RFIFOP(fd,4));
		if( cAuthList.find(auth,0,pos) )
			cAuthList[pos]=auth;
		else
			cAuthList.insert(auth);
	}
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
int chrif_save(map_session_data &sd)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	pc_makesavestatus(sd);

	WFIFOW(char_fd,0) = 0x2b01;
	WFIFOW(char_fd,2) = 12+sizeof(sd.status);
	WFIFOL(char_fd,4) = sd.block_list::id;
	WFIFOL(char_fd,8) = sd.status.char_id;
	mmo_charstatus_tobuffer( sd.status, WFIFOP(char_fd,12) );
	WFIFOSET(char_fd, 12+sizeof(sd.status));
	storage_storage_dirty(sd);
	storage_storage_save(sd);

	return 0;
}
int chrif_save_sc(map_session_data &sd)
{
/*	static const unsigned short sc_array[] = 
	{
		0
		// non-saveable because using pointers to other structures
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
		SC_STUN,
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
	};
*/	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;
	size_t i, cnt, p;
	unsigned long tick = gettick();

	for(i=0, cnt=0, p=14; i<MAX_STATUSCHANGE; ++i)
	{
		if( sd.has_status((status_t)i) )
		{
			const ulong remaining_time = sd.get_status_remaining((status_t)i,tick);
			if( remaining_time &&
				!sd.get_statusvalue1((status_t)i).pointer() &&
				!sd.get_statusvalue2((status_t)i).pointer() &&
				!sd.get_statusvalue3((status_t)i).pointer() &&
				!sd.get_statusvalue4((status_t)i).pointer() )
			{
				sc_data data;
				data.type = i;
				data.val1 = sd.get_statusvalue1((status_t)i).integer();
				data.val2 = sd.get_statusvalue2((status_t)i).integer();
				data.val3 = sd.get_statusvalue3((status_t)i).integer();
				data.val4 = sd.get_statusvalue4((status_t)i).integer();
				data.tick = remaining_time; //Duration that is left before ending.
				scdata_tobuffer(data, RFIFOP(char_fd,p));
				p+=sizeof(struct sc_data);
				cnt++;
			}
		}
	}
	WFIFOW(char_fd, 0) = 0x2b22;
	WFIFOW(char_fd, 2) = p;
	WFIFOL(char_fd, 4) = sd.block_list::id;
	WFIFOL(char_fd, 8) = sd.status.char_id;
	WFIFOW(char_fd,12) = cnt;
	WFIFOSET(char_fd, p);
	
	return 0;
}

int chrif_parse_ReadSC(int fd)
{
	if( !session_isActive(fd) || !chrif_isconnect() )
		return -1;

	map_session_data *sd = map_session_data::charid2sd( RFIFOL(fd,8) );
	if(sd && sd->block_list::id==RFIFOL(fd,4))
	{
		size_t i, p, count = RFIFOW(fd,12); //sc_count
		struct sc_data data;
		for (i=0, p=14; i<count; i++, p+=14+sizeof(struct sc_data))
		{
			scdata_frombuffer(data, RFIFOP(fd,p));
			status_change_start(sd, (status_t)data.type, data.val1, data.val2, data.val3, data.val4, data.tick, 3);
			//Flag 3 is 1&2, 1: Force status start, 2: Do not modify the tick value sent.
		}
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

	WFIFOLIP(fd,54) = getmapaddress().RealLANIP();
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
	for(i = 0; i < map_num; ++i)
		memcpy(WFIFOP(fd,4+i*16), maps[i].mapname, 16);
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

	if( !session_isActive(char_fd) || !chrif_isconnect() )	// まだ準備中
		return -1;

	basics::ipaddress ip = RFIFOLIP(fd,4);
	unsigned short port = RFIFOW(fd,8);
	basics::ipset mapset( ip, port, RFIFOW(fd,12), RFIFOLIP(fd,14), RFIFOW(fd,18) );

	for(i = 20, j = 0; i < RFIFOW(fd,2); i += 16, ++j)
	{
		map_setipport((char*)RFIFOP(fd,i), mapset);
	}
	if (config.etc_log)
		ShowStatus("recv maps from %s (%d maps)\n", mapset.tostring(NULL), j);

	return 0;
}

/*==========================================
 * Delete maps of other servers, (if an other mapserver is going OFF)
 *------------------------------------------
 */
int chrif_removemap(int fd)
{
	int i, j;
	if( !session_isActive(fd) || !chrif_isconnect() )
		return -1;
	
	basics::ipset mapset( RFIFOLIP(fd,4), RFIFOLIP(fd,8),RFIFOW(fd,12), RFIFOLIP(fd,14), RFIFOW(fd,18) );

	for(i=20, j=0; i<RFIFOW(fd, 2); i+=16, ++j)
	{
		map_eraseipport((char*)RFIFOP(fd, i), mapset);
	}
	if(config.etc_log)
		ShowStatus("remove maps of server %s (%d maps)\n", mapset.tostring(NULL), j);

	return 0;	
}
	
/*==========================================
 * マップ鯖間移動のためのデータ準備要求
 *------------------------------------------
 */
int chrif_changemapserver(map_session_data &sd, const char *name, unsigned short x, unsigned short y, basics::ipset& mapset)
{
	size_t i;
	basics::ipaddress s_ip=(uint32)INADDR_ANY;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	for(i = 0; i < fd_max; ++i)
	{
		if (session[i] && session[i]->user_session == &sd)
		{
			s_ip = session[i]->client_ip;
			break;
		}
	}

	WFIFOW(char_fd, 0) = 0x2b05;
	WFIFOL(char_fd, 2) = sd.block_list::id;
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

	map_session_data *sd = map_session_data::from_blid(RFIFOL(fd,2));

	if( sd == NULL || RFIFOL(fd,14) != sd->status.char_id )
		return -1;

	if( RFIFOL(fd,6) == 1 )
	{
		if (config.error_log)
			ShowMessage("map server change failed.\n");
		pc_authfail(sd->fd);
		return 0;
	}
	clif_changemapserver(*sd, (char*)RFIFOP(fd,18), RFIFOW(fd,34), RFIFOW(fd,36), RFIFOLIP(fd,38), RFIFOW(fd,42));

	return 0;
}
/*==========================================
 * Delayed execution of the OnAgitInit to allow castle data
 * to the retrieved from the char-server. [Skotlex]
 *------------------------------------------
 */
int chrif_do_onagitinit(int tid, unsigned long tick, int id, basics::numptr data)
{
	ShowStatus("Event '"CL_WHITE"OnAgitInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_data::event("OnAgitInit"));
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

	if (RFIFOB(fd,2))
	{
		ShowError("Connection to char-server failed (%d).\n"
					CL_SPACE"char server is full or using wrong username or password.\n"
					CL_SPACE"error not recoverable, quitting.\n",
					(unsigned char)RFIFOB(fd,2));
		exit(1);
	}
	ShowStatus("Successfully connected to Char Server (Connection: '"CL_WHITE"%d"CL_RESET"').\n",fd);
	chrif_state = 1;

	chrif_sendmap(fd);

	ShowStatus("Event '"CL_WHITE"OnCharIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_data::event("OnCharIfInit"));
	ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_data::event("OnInterIfInit"));
	if(!char_init_done) {
		char_init_done = 1;
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_data::event("OnInterIfInitOnce"));
		add_timer(gettick() + 10000, chrif_do_onagitinit, 0, 0);
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
		safestrcpy(wisp_server_name, 24, (char*)RFIFOP(fd,3));
		chrif_state = 2;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_authreq(map_session_data &sd)
{
	size_t i;

	if(!sd.block_list::id || !sd.login_id1)
		return -1;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	if( session_isActive(sd.fd) && session[sd.fd]->user_session == &sd)
		i = sd.fd;
	else
	{	// totally unnecessary
		for(i = 0; i < fd_max; ++i)
			if( session[i] && session[i]->user_session == &sd)
				break;
	}

	if(i < fd_max)
	{
		WFIFOW(char_fd, 0) = 0x2afc;
		WFIFOL(char_fd, 2) = sd.status.account_id;
		WFIFOL(char_fd, 6) = sd.status.char_id;
		WFIFOL(char_fd,10) = sd.login_id1;
		WFIFOL(char_fd,14) = sd.login_id2;
		WFIFOLIP(char_fd,18) = session[i]->client_ip;
		WFIFOSET(char_fd,22);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int chrif_charselectreq(map_session_data &sd)
{
	basics::ipaddress s_ip=(uint32)INADDR_ANY;

	if( !sd.block_list::id || !sd.login_id1 )
		return -1;
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

//	for(i = 0; i < fd_max; ++i)
//		if (session[i] && session[i]->user_session == &sd)
//		{
//			s_ip = session[i]->client_ip;
//			break;
//		}
	if( session_isActive(sd.fd) && session[sd.fd] && session[sd.fd]->user_session == &sd)
		s_ip = session[sd.fd]->client_ip;


	WFIFOW(char_fd, 0) = 0x2b02;
	WFIFOL(char_fd, 2) = sd.block_list::id;
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
int chrif_searchcharid(uint32 id)
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
int chrif_changegm(uint32 id, const char *pass, size_t len)
{
	if (config.etc_log)
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
int chrif_changeemail(uint32 id, const char *actual_email, const char *new_email)
{
	if (config.etc_log)
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
int chrif_changesex(uint32 id, unsigned char sex)
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
	map_session_data *sd;
	char output[256];
	char player_name[24];

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2); // account_id of who has asked (-1 if nobody)
	safestrcpy(player_name, sizeof(player_name), (char*)RFIFOP(fd,6));

	sd = map_session_data::from_blid(acc);
	if (acc >= 0 && sd != NULL)
	{
		if (RFIFOW(fd, 32) == 1) // player not found
			snprintf(output, sizeof(output), "The player '%s' doesn't exist.", player_name);
		else {
			switch(RFIFOW(fd, 30)) {
			case 1: // block
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					snprintf(output, sizeof(output), "Login-server has been asked to block the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					snprintf(output, sizeof(output), "Your GM level don't authorise you to block the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					snprintf(output, sizeof(output), "Login-server is offline. Impossible to block the the player '%s'.", player_name);
					break;
				}
				break;
			case 2: // ban
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					snprintf(output, sizeof(output), "Login-server has been asked to ban the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					snprintf(output, sizeof(output), "Your GM level don't authorise you to ban the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					snprintf(output, sizeof(output), "Login-server is offline. Impossible to ban the the player '%s'.", player_name);
					break;
				}
				break;
			case 3: // unblock
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					snprintf(output, sizeof(output), "Login-server has been asked to unblock the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					snprintf(output, sizeof(output), "Your GM level don't authorise you to unblock the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					snprintf(output, sizeof(output), "Login-server is offline. Impossible to unblock the the player '%s'.", player_name);
					break;
				}
				break;
			case 4: // unban
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					snprintf(output, sizeof(output), "Login-server has been asked to unban the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					snprintf(output, sizeof(output), "Your GM level don't authorise you to unban the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					snprintf(output, sizeof(output), "Login-server is offline. Impossible to unban the the player '%s'.", player_name);
					break;
				}
				break;
			case 5: // changesex
				switch(RFIFOW(fd, 32)) {
				case 0: // login-server resquest done
					snprintf(output, sizeof(output), "Login-server has been asked to change the sex of the player '%s'.", player_name);
					break;
				//case 1: // player not found
				case 2: // gm level too low
					snprintf(output, sizeof(output), "Your GM level don't authorise you to change the sex of the player '%s'.", player_name);
					break;
				case 3: // login-server offline
					snprintf(output, sizeof(output), "Login-server is offline. Impossible to change the sex of the the player '%s'.", player_name);
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
	map_session_data *sd = NULL;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	level = RFIFOL(fd,6);

	sd = map_session_data::from_blid(acc);

	if (config.etc_log)
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
	map_session_data *sd;
	struct pc_base_job s_class;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	sex = RFIFOL(fd,6);
	if (config.etc_log)
		ShowMessage("chrif_changedsex %d.\n", acc);
	sd = map_session_data::from_blid(acc);
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
			for (i = 0; i < MAX_INVENTORY; ++i) {
				if (sd->status.inventory[i].nameid && sd->status.inventory[i].equip)
					pc_unequipitem(*sd->get_sd(), i, 2);
			}
			// reset skill of some job
			if (s_class.job == 19 || s_class.job == 4020 || s_class.job == 4042 ||
			    s_class.job == 20 || s_class.job == 4021 || s_class.job == 4043) {
				// remove specifical skills of classes 19, 4020 and 4042
				for(i = 315; i <= 322; ++i) {
					if (sd->status.skill[i].id > 0 && !sd->status.skill[i].flag) {
						sd->status.skill_point += sd->status.skill[i].lv;
						sd->status.skill[i].id = 0;
						sd->status.skill[i].lv = 0;
					}
				}
				// remove specifical skills of classes 20, 4021 and 4043
				for(i = 323; i <= 330; ++i) {
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
int chrif_saveaccountreg2(map_session_data &sd)
{
	unsigned short p;
	uint32 j;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	p = 8;
	for(j = 0; j < sd.status.account_reg2_num; ++j) {
		if( sd.status.account_reg2[j].str[0] && sd.status.account_reg2[j].value != 0) {
			memcpy(WFIFOP(char_fd,p), sd.status.account_reg2[j].str, 32);
			WFIFOL(char_fd,p+32) = sd.status.account_reg2[j].value;
			p += 36;
		}
	}
	WFIFOW(char_fd,0) = 0x2b10;
	WFIFOW(char_fd,2) = p;
	WFIFOL(char_fd,4) = sd.block_list::id;
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
	map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	if ((sd = map_session_data::from_blid(RFIFOL(fd,4))) == NULL)
		return 1;

	for(p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, ++j)
	{
		safestrcpy(sd->status.account_reg2[j].str, sizeof(sd->status.account_reg2[j].str), (char*)RFIFOP(fd,p));
		sd->status.account_reg2[j].value = RFIFOL(fd, p + 32);
	}
	sd->status.account_reg2_num = j;

	return 0;
}

/*==========================================
 * 離婚情報同期要求
 *------------------------------------------
 */
int chrif_divorce(uint32 char_id, uint32 partner_id)
{
	map_session_data *sd = NULL;

	if (!char_id || !partner_id)
		return 0;

	nullpo_retr(0, sd = map_session_data::charid2sd(partner_id));
	if (sd->status.partner_id == char_id) {
		int i;
		//離婚(相方は既にキャラが消えている筈なので)
		sd->status.partner_id = 0;

		//相方の結婚指輪を剥奪
		for(i = 0; i < MAX_INVENTORY; ++i)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc_delitem(*sd, i, 1, 0);
	}

	return 0;
}

/*==========================================
 * Disconnection of a player
 *------------------------------------------
 */
int chrif_accountdeletion(int fd)
{
	int acc;
	map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	if (config.etc_log)
		ShowMessage("chrif_accountdeletion %d.\n", acc);
	sd = map_session_data::from_blid(acc);
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
	map_session_data *sd;

	if( !session_isActive(fd) )
		return -1;

	acc = RFIFOL(fd,2);
	if (config.etc_log)
		ShowMessage("chrif_accountban %d.\n", acc);
	sd = map_session_data::from_blid(acc);
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
	map_session_data *sd;
	
	sd = map_session_data::from_blid(RFIFOL(fd, 2));
	
	if(RFIFOL(fd, 2) <= 0 || sd == NULL){
		return -1;
	}
	
	//change sessid1 
	sd->login_id1++;

	switch(RFIFOB(fd, 6))
	{
	//clif_authfail
	case 1: //server closed 
		clif_authfail(*sd, 1);	
		break;
	case 2: //someone else logged in
		clif_authfail(*sd, 2);		
		break;
	case 3: //server overpopulated
		clif_authfail(*sd, 4);
		break;
	case 4: //out of time payd for .. (avail)
		clif_authfail(*sd, 10);
		break;
	case 5: //forced to dc by gm
		clif_authfail(*sd, 15);
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
 * return the requested fame list
 * returns pklist on out-of-bound
 *------------------------------------------
 */
const CFameList& chrif_getfamelist(fame_t type)
{
	if( type==FAME_PK || type==FAME_SMITH || type==FAME_CHEM || type==FAME_TEAK )
		return famelists[type];
	return famelists[FAME_PK];
}
/*==========================================
 * send an update for famelist, char then decides on sending an update back
 *------------------------------------------
 */
int chrif_updatefame(map_session_data &sd, fame_t type, int delta)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return 0;

	static const char* regnames[4] = 
	{"PC_PK_FAME","PC_SMITH_FAME","PC_CHEM_FAME","PC_TEAK_FAME"};

	uint32 total = pc_readglobalreg(sd,regnames[type]);
	// saturated operation within 0..INT_MAX
	if( delta<0 && total<(uint32)(-delta))
		total=0;	
	else if( delta>0 && total+delta>INT_MAX)
		total=INT_MAX;	
	else
		total+=delta;
	pc_setglobalreg(sd,regnames[type],total);

	switch(type)
	{
		case FAME_PK:	// PK
			clif_pk_fame(sd, total, delta);
			break;
		case FAME_SMITH: // Blacksmith
			clif_blacksmith_fame(sd, total, delta);
			break;
		case FAME_CHEM: // Alchemist
			clif_alchemist_fame(sd, total, delta);
			break;
		case FAME_TEAK: // Taekwon
			clif_taekwon_fame(sd, total, delta);
			break;
	}

	//send stuff to char and char decides if a famelist update is necessary
	WFIFOW(char_fd,0) = 0x2b1a;
	WFIFOW(char_fd,2) = type;
	WFIFOL(char_fd,4) = sd.status.char_id;
	memcpy(WFIFOP(char_fd,8), sd.status.name,24);
	WFIFOL(char_fd,32) = total;
	WFIFOSET(char_fd, 36);

	// return new total number of points
	return total;
}

// Check whether a player ID is in the top10 fame list
bool chrif_istop10fame(uint32 char_id, fame_t type)
{
	return chrif_getfamelist(type).exists(char_id);
}

/*==========================================
 * Receive top 10 Fame character list
 *------------------------------------------
 */
int chrif_recvfamelist(int fd)
{
	if( !session_isActive(fd) )
		return -1;

	// 0 w -> command
	// 2 w -> packet size
	fame_t type = (fame_t)((ushort)RFIFOW(fd,4));

	if( type==FAME_PK || type==FAME_SMITH || type==FAME_CHEM || type==FAME_TEAK )
	{
		static const char* which[] = {"PK", "Smith", "Chemist", "Teakon"};
		CFameList &fl = famelists[type];

		fl.frombuffer(RFIFOP(fd,6));
		if(config.etc_log)
			ShowInfo("Receiving %s Fame List of '"CL_WHITE"%d"CL_RESET"' characters.\n", 
				which[type], fl.count() );
	}
	return 0;
}

/*==========================================
 * Send rates and motd to char server [Wizputer]
 *------------------------------------------
 */
int chrif_ragsrvinfo(unsigned short base_rate, unsigned short job_rate, unsigned short drop_rate)
{
	char strbuf[256];
	FILE *fp;
	size_t sl, sz = 10;

	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b16;
	WFIFOW(char_fd,2) = base_rate;
	WFIFOW(char_fd,4) = job_rate;
	WFIFOW(char_fd,6) = drop_rate;

	if( (fp = basics::safefopen(motd_txt, "r")) != NULL)
	{
		while( fgets(strbuf, sizeof(strbuf), fp) )
		{
			sl = prepare_line(strbuf);
			if(sl)
			{
				memcpy(WFIFOP(char_fd,sz), strbuf, sl);
				sz += sl;
				WFIFOB(char_fd,sz)='\n';
				++sz;
			}
		}
		if(sz>0) --sz;
		WFIFOB(char_fd,sz)=0;
		++sz;
		fclose(fp);
	}
	else
	{
		WFIFOB(char_fd,10) = 0;
	}
	++sz;// eof
	WFIFOW(char_fd,8) = sz;
	WFIFOSET(char_fd, sz);
	return 0;
}


/*=========================================
 * Tell char-server charcter disconnected [Wizputer]
 *-----------------------------------------
 */

int chrif_char_offline(map_session_data &sd)
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
 * Tell char-server charcter is online [Wizputer]
 *-----------------------------------------
 */

int chrif_char_online(map_session_data &sd)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	WFIFOW(char_fd,0) = 0x2b19;
	WFIFOL(char_fd,2) = sd.status.char_id;
	WFIFOL(char_fd,6) = sd.status.account_id;
	WFIFOSET(char_fd,10);

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

void chrif_char_online_check(void)
{
	chrif_char_reset_offline();

	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		map_session_data *sd = iter.data();
		if( sd &&
			!(config.hide_GM_session && sd->isGM()) )
		{
			chrif_char_online(*sd);
		}
	}
}
/*=========================================
 * 
 *-----------------------------------------
 */
int chrif_flush_fifo(void)
{
	if( !session_isActive(char_fd) || !chrif_isconnect() )
		return -1;

	flush_fifos();

	return 0;
}












///////////////////////////////////////////////////////////////////////////////
// chrif mail system
///////////////////////////////////////////////////////////////////////////////
//	mail packets
//	2b23: check mail => returns number of unread
//	2b24: fetch mail => returns the mail headers/fail, param: box
//	2b25: read mail  => returns specified mail/fail, param: msg_id
//	2b26: delete mail=> deletes specified mail returns ok/fail, param: msg_id
//	2b27: send mail  => sends mail returns ok/fail, param: target, header, body
///////////////////////////////////////////////////////////////////////////////

/// temporary mail storage helper
class CMailDummy
{
public:
	uint32 zeny;
	struct item item;
	ushort index;

	CMailDummy() : zeny(0), index(0xFFFF)
	{ }
};

basics::map<uint32, CMailDummy> maildb;	///< temp mailstorage
basics::Mutex mailmx;					///< access mutex


/// clears a mail from temp storage
void chrif_mail_cancel(map_session_data &sd)
{
	basics::ScopeLock sl(mailmx);
	if( maildb.exists(sd.status.char_id) )
	{
		maildb.erase(sd.status.char_id);
	}
}

/// set item or zeny to a mail
bool chrif_mail_setitem(map_session_data &sd, ushort index, uint32 amount)
{
	basics::ScopeLock sl(mailmx);
	CMailDummy &mail = maildb[sd.status.char_id];

	if(index == 0)
	{	// zeny
		mail.zeny = (sd.status.zeny < amount) ? sd.status.zeny :  amount;
		return true;
	}
	// item with client side indexing
	index-=2;
	if(index < MAX_INVENTORY)
	{	
		if( mail.item.nameid > 0 && mail.item.amount > amount )
		{
			mail.item.amount -= amount;
			return true;
		}
		else if( sd.status.inventory[index].amount >= amount )
		{
			if( itemdb_isdropable(sd.status.inventory[index].nameid, sd.status.gm_level) )
			{
				mail.item = sd.status.inventory[index];
				mail.item.amount = amount;
				mail.index = index;
				return true;
			}
		}
	}
	return false;
}
/// remove item from mail
void chrif_mail_removeitem(map_session_data &sd, int flag)
{
	basics::ScopeLock sl(mailmx);
	if( maildb.exists(sd.status.char_id) )
	{
		CMailDummy &mail = maildb[sd.status.char_id];
		if( flag & 0x01 )
		{	// clear item
			mail.item.nameid = 0;
			mail.item.amount = 0;
		}
		if( flag & 0x02 )
		{	// clear zeny
			mail.zeny = 0;
		}
	}
}

bool chrif_mail_check(map_session_data &sd, bool showall)
{
	if( session_isActive(char_fd) )
	{
		WFIFOW(char_fd, 0) = 0x2b23;
		WFIFOW(char_fd, 2) = 9;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		WFIFOB(char_fd, 8) = showall;
		WFIFOSET(char_fd, 9);
	}
	return true;
}
int chrif_parse_mail_check(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		uint32 all    = RFIFOL(fd,8);
		uint32 unread = RFIFOL(fd,12);
		uchar showall = RFIFOB(fd,16);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd)
		{
			char message[512];
			if(showall && all>0)
			{
				snprintf(message, sizeof(message), "You have %u unread of %u mails", unread, all);
				clif_disp_onlyself(*sd, message);
			}
			else if(unread>0)
			{
				snprintf(message, sizeof(message), msg_txt(MSG_YOU_HAVE_D_NEW_MESSAGES), unread);
				clif_disp_onlyself(*sd, message);
			}
			else
			{
				clif_disp_onlyself(*sd, msg_txt(MSG_YOU_HAVE_NO_NEW_MESSAGES));
			}
		}
	}
	return 0;
}
bool chrif_mail_fetch(map_session_data &sd, bool all)
{
	if( session_isActive(char_fd) )
	{
		WFIFOW(char_fd, 0) = 0x2b24;
		WFIFOW(char_fd, 2) = 9;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		WFIFOB(char_fd, 8) = all;
		WFIFOSET(char_fd, 9);
	}
	return true;
}
int chrif_parse_mail_fetch(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		uint32 count  = RFIFOL(fd,8);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd)
		{
			clif_send_mailbox(*sd, count, RFIFOP(fd,12));
		}
	}
	return 0;
}
bool chrif_mail_read(map_session_data &sd, uint32 msgid)
{
	if( session_isActive(char_fd) )
	{
		WFIFOW(char_fd, 0) = 0x2b25;
		WFIFOW(char_fd, 2) = 12;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		WFIFOL(char_fd, 8) = msgid;
		WFIFOSET(char_fd, 12);
	}
	return true;
}
int chrif_parse_mail_read(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd)
		{
			CMail mail;
			char message[512], *ip=mail.body, *kp;
			mail.frombuffer( RFIFOP(fd, 8) );

			// automatically get appends when reading
			// should be checked if suitable
			if(mail.item.nameid>0 && mail.item.amount>0 )
				pc_additem(*sd, mail.item, mail.item.amount);
			if( mail.zeny>0 )
			{
				sd->status.zeny += mail.zeny;
				clif_updatestatus(*sd, SP_ZENY);
			}


			if( *mail.body )
				snprintf(message, sizeof(message), "%c %-8lu %-24s %s", mail.read?' ':'*', (unsigned long)mail.msgid, mail.name, mail.head);
			else
				snprintf(message, sizeof(message), "mail not found");
			clif_disp_onlyself(*sd, message);
			// linewise print of mail body
			while(ip)
			{
				kp = strchr(ip, '\n');
				if(kp) *kp++=0;
				clif_disp_onlyself(*sd, ip);
				ip = kp;
			}
		}
	}
	return 0;
}
bool chrif_mail_delete(map_session_data &sd, uint32 msgid)
{
	if( session_isActive(char_fd) )
	{
		WFIFOW(char_fd, 0) = 0x2b26;
		WFIFOW(char_fd, 2) = 12;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		WFIFOL(char_fd, 8) = msgid;
		WFIFOSET(char_fd, 12);
	}
	return true;
}
int chrif_parse_mail_delete(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		uint32 msgid  = RFIFOL(fd,8);
		uchar ok      = RFIFOB(fd,12);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd) clif_deletemail_res(*sd, msgid, ok);
	}
	return 0;
}

bool chrif_mail_send(map_session_data &sd, const char *target, const char *header, const char *body)
{
	if(sd.isGM() < 80 && DIFF_TICK(gettick(), sd.mail_tick) < 10*60*1000)
	{
		//clif_displaymessage(sd.fd,"You must wait 10 minutes before sending another message");
		clif_disp_onlyself(sd,msg_txt(MSG_10_MIN_BEFORE_SENDING));
	}
	else if( (sd.isGM() < 80 && 0==strcmp(target,"*")) || (0==strcmp(target,sd.status.name)) )
	{
		//clif_displaymessage(sd.fd, "Access Denied.");
		clif_disp_onlyself(sd, msg_txt(MSG_ACCESS_DENIED));
	}
	else if( session_isActive(char_fd) )
	{
		const size_t len = 4 +24+24+40+512+ 4+sizeof(struct item);
		WFIFOW(char_fd, 0) = 0x2b27;
		WFIFOW(char_fd, 2) = len;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		safestrcpy((char*)WFIFOP(char_fd,  8), 24, sd.status.name);
		safestrcpy((char*)WFIFOP(char_fd, 32), 24, target);
		safestrcpy((char*)WFIFOP(char_fd, 56), 40, header);
		safestrcpy((char*)WFIFOP(char_fd, 96), 512, body);

		// put zeny and item to buffer, do final checking
		basics::ScopeLock sl(mailmx);
		if( maildb.exists(sd.status.char_id) )
		{
			CMailDummy &mail = maildb[sd.status.char_id];
			
			if( mail.index<MAX_INVENTORY && mail.item.nameid == sd.status.inventory[mail.index].nameid )
			{
				if( mail.item.amount > sd.status.inventory[mail.index].amount )
					mail.item.amount = sd.status.inventory[mail.index].amount;
			}

			if( mail.zeny )
			{
				if( mail.zeny > sd.status.zeny )
					mail.zeny = sd.status.zeny;
			}
			WFIFOL(char_fd, 608) = mail.zeny;
			item_tobuffer(mail.item, WFIFOP(char_fd, 612));
		}

		WFIFOSET(char_fd, len);
		sd.mail_tick = gettick();
	}
	return true;
}
int chrif_parse_mail_send(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		// uint32 msgid  = RFIFOL(fd,8); // not needed
		char ok       = RFIFOB(fd,12);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd)
		{
			basics::ScopeLock sl(mailmx);
			if( ok )
			{	// take item/zeny away when send was successful
				CMailDummy &mail = maildb[sd->status.char_id];

				pc_delitem(*sd, mail.index, mail.item.amount, 0);
				sd->status.zeny -= mail.zeny;
				clif_updatestatus(*sd, SP_ZENY);
			}
			maildb.erase(sd->status.char_id);

			clif_res_sendmail(*sd, ok);
		}
	}
	return 0;
}
bool chrif_mail_getappend(map_session_data &sd, uint32 msgid)
{
	if( session_isActive(char_fd) )
	{
		WFIFOW(char_fd, 0) = 0x2b28;
		WFIFOW(char_fd, 2) = 12;
		WFIFOL(char_fd, 4) = sd.status.char_id;
		WFIFOL(char_fd, 8) = msgid;
		WFIFOSET(char_fd, 12);
	}
	return true;
}
int chrif_parse_mail_getappend(int fd)
{
	if( session_isActive(fd) )
	{
		uint32 charid = RFIFOL(fd,4);
		map_session_data *sd = map_session_data::charid2sd(charid);
		if(sd)
		{
			CMail mail;
			mail.frombuffer( RFIFOP(fd, 8) );

			if(mail.item.nameid>0 && mail.item.amount>0 )
				pc_additem(*sd, mail.item, mail.item.amount);
			if( mail.zeny>0 )
			{
				sd->status.zeny += mail.zeny;
				clif_updatestatus(*sd, SP_ZENY);
			}
		}
	}
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// chrif variable system

///////////////////////////////////////////////////////////////////////////////
//	2b30: request a variable
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	2b31: transfer a variable
///////////////////////////////////////////////////////////////////////////////
int chrif_var_save(const char* name, const char* value)
{
	if( session_isActive(char_fd) )
	{
		CVar var(name, value);
//		ShowMessage("saving var '%s' = '%s'\n", (const char*)var.name(), (const char*)var.value());

		size_t sz = var.to_buffer(WFIFOP(char_fd,4), 0); //## len parameter is ignored here
		WFIFOW(char_fd, 0) = 0x2b31;
		WFIFOW(char_fd, 2) = 4+sz;
		WFIFOSET(char_fd, 4+sz);
	}
	return 0;
}
int chrif_var_save(const char* name, uint32 value)
{
	if( session_isActive(char_fd) )
	{
		CVar var(name, basics::string<>(value));
//		ShowMessage("saving var '%s' = '%s'\n", (const char*)var.name(), (const char*)var.value());

		size_t sz = var.to_buffer(WFIFOP(char_fd,4), 0); //## len parameter is ignored here
		WFIFOW(char_fd, 0) = 0x2b31;
		WFIFOW(char_fd, 2) = 4+sz;
		WFIFOSET(char_fd, 4+sz);
	}
	return 0;
}
int chrif_parse_var_recv(int fd)
{
	if( session_isActive(fd) )
	{
		CVar var;
		var.from_buffer( RFIFOP(fd, 4) );

//		ShowMessage("receiving var '%s' = '%s'\n", (const char*)var.name(), (const char*)var.value());
		
		const char postfix = var.name().size() ? var.name()[var.name().size()-1] : '\0';
		set_var((const char *)var.name(), 		
			(postfix=='$') ?
			((void *)(const char*)var.value()) :
			((void *)(size_t)(ssize_t)atol(var.value())) );

	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// chrif irc system
///////////////////////////////////////////////////////////////////////////////
// 2b28: irc_announce in/out
///////////////////////////////////////////////////////////////////////////////
int chrif_irc_announce(const char* message, size_t sz)
{
	if( message && session_isActive(char_fd) )
	{
		if(sz==0) sz = 1+strlen(message);
		if(sz>1)
		{
			session_checkbuffer(char_fd, 4+sz);
			WFIFOW(char_fd, 0) = 0x2b38;
			WFIFOW(char_fd, 2) = 4+sz;
			memcpy(WFIFOP(char_fd, 4),message, sz);
			WFIFOSET(char_fd, 4+sz);
		}
	}
	return 0;
}
int chrif_parse_irc_announce(int fd)
{
	if( session_isActive(fd) )
	{
		unsigned short len = RFIFOW(char_fd, 2);
		if(len>5) // 4 byte header + 1 byte terminator
		{
			char* str = (char*)RFIFOP(char_fd, 4);
			len -= 4;
			str[len-1] = 0; // force a terminator
			intif_GMmessage(str,len,0);
		}
	}
	return 0;
}
int chrif_irc_announce_jobchange(map_session_data &sd)
{
	char message[1024];
	int len = 1+snprintf(message, sizeof(message), "%s has changed into a %s.", sd.status.name, job_name(sd.status.class_));
	return chrif_irc_announce(message,len);
}
int chrif_irc_announce_shop(map_session_data &sd, int flag)
{
	char message[1024];
	char mapname[32];
	safestrcpy(mapname, sizeof(mapname), maps[sd.block_list::m].mapname);
	mapname[0] = basics::upcase(mapname[0]);
	size_t len = (flag) ?
		snprintf(message, sizeof(message), "%s has opened the shop '%s' at <%d,%d> in %s.", sd.status.name, sd.message, sd.block_list::x,sd.block_list::y, mapname) :
		snprintf(message, sizeof(message), "%s has closed their shop in %s.", sd.status.name, mapname);
	return chrif_irc_announce(message,len);
}
int chrif_irc_announce_mvp(const map_session_data &sd, const mob_data &md)
{
	char message[1024];
	char mapname[32];
	safestrcpy(mapname, sizeof(mapname), maps[sd.block_list::m].mapname);
	mapname[0] = basics::upcase(mapname[0]);

	size_t len = 1+sprintf(message,"%s the %s has MVP'd %s in %s.", sd.status.name, job_name(sd.status.class_), md.name, mapname);
	return chrif_irc_announce(message,len);
}


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
		if(cmd < 0x2af8 || cmd >= 0x2af8 + (sizeof(packet_len_table) / sizeof(packet_len_table[0])) ||
		    packet_len_table[cmd-0x2af8] == 0)
		{

			int r = intif_parse(fd); // intifに渡す

			if (r == 1) continue;	// intifで処理した
			if (r == 2) return 0;	// intifで処理したが、データが足りない

			session_Remove(fd);
			ShowWarning("chrif_parse: session #%d, cmd 0x%04X intif_parse failed -> disconnected.\n", fd, cmd);
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
		case 0x2afb: chrif_sendmapack(fd); break;
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
		case 0x2b09: map_add_namedb(RFIFOL(fd,2), (char*)RFIFOP(fd,6)); break;
		case 0x2b0b: chrif_changedgm(fd); break;
		case 0x2b0d: chrif_changedsex(fd); break;
		case 0x2b0f: chrif_char_ask_name_answer(fd); break;
		case 0x2b11: chrif_accountreg2(fd); break;
		case 0x2b12: chrif_divorce(RFIFOL(fd,2), RFIFOL(fd,6)); break;
		case 0x2b13: chrif_accountdeletion(fd); break;
		case 0x2b14: chrif_accountban(fd); break;
case 0x2b15: break;
		case 0x2b1b: chrif_recvfamelist(fd); break;
		case 0x2b1f: chrif_disconnectplayer(fd); break;
		case 0x2b20: chrif_removemap(fd); break; //Remove maps of a server [Sirius]

		// new authentification system
		case 0x2b21: chrif_parse_StoreAthentification(fd); break;
		// status saving
		case 0x2b22: chrif_parse_ReadSC(fd); break;
		// new mail system
		case 0x2b23: chrif_parse_mail_check(fd); break;
		case 0x2b24: chrif_parse_mail_fetch(fd); break;
		case 0x2b25: chrif_parse_mail_read(fd); break;
		case 0x2b26: chrif_parse_mail_delete(fd); break;
		case 0x2b27: chrif_parse_mail_send(fd); break;
		//case 0x2b28: inbound get appendix is not used

		case 0x2b31: chrif_parse_var_recv(fd); break;

		case 0x2b38: chrif_parse_irc_announce(fd); break;

		default:
			if (config.error_log)
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
int send_users_tochar(int tid, unsigned long tick, int id, basics::numptr data)
{
	unsigned short users = 0;
	map_session_data *sd;

	if( !session_isActive(char_fd) || !chrif_isconnect() ) // Thanks to Toster
		return 0;

	WFIFOW(char_fd,0) = 0x2aff;

	map_session_data::iterator iter(map_session_data::nickdb());
	for(; iter; ++iter)
	{
		sd = iter.data();
		if( sd &&
			!((config.hide_GM_session || (sd->status.option & OPTION_HIDE)) && sd->isGM()))
		{
			WFIFOL(char_fd,6+4*users) = sd->status.char_id;
			++users;
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
int check_connect_char_server(int tid, unsigned long tick, int id, basics::numptr data)
{
	if( !session_isActive(char_fd) )
	{
		chrif_state = 0;

		ShowStatus("Attempting to connect to Char Server. Please wait.\n");
		char_fd = make_connection(getcharaddress().addr(), getcharaddress().port());
		if( session_isActive(char_fd) )
		{
			session[char_fd]->func_parse = chrif_parse;
			realloc_fifo(char_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

			chrif_connect(char_fd);
			chrif_ragsrvinfo(config.base_exp_rate, config.job_exp_rate, config.item_rate_common);
		}
	}
	check_connect_map_port();
	return 0;
}
int check_dropped_mapwan(int tid, unsigned long tick, int id, basics::numptr data)
{
	static basics::CParam<bool> automatic_wan_setup("automatic_wan_setup",false);
	if( automatic_wan_setup )
	{
		basics::ipset &mapaddress = getmapaddress();

		if( dropped_WAN(mapaddress.WANIP(), mapaddress.WANPort()) )
		{
			// when we had a wanip then it is gone now
			if( mapaddress.WANIP() != basics::ipany )
				ShowWarning("WAN connection dropped.\n");

			// and try to find the new wanip
			if( initialize_WAN(mapaddress, 6121) )
			{
				if( dropped_WAN(mapaddress.WANIP(), mapaddress.WANPort()) )
				{	// most likely the router/firewall settings are wrong
					// give a warning and disable the automatich check
					char buf1[16],buf2[16];
					ShowWarning("WAN connection not available or router with wrong configuration.\n"
								CL_SPACE"expecting correct port forward from router to local machine\n"
								CL_SPACE"router is on %s:%i, local machine on %s:%i\n"
								, mapaddress.WANIP().tostring(buf1), mapaddress.WANPort()
								, mapaddress.LANIP().tostring(buf2), mapaddress.LANPort()
								);

					ShowWarning("automatic WAN detection will be disabled now.\n");
					automatic_wan_setup = false;

					// reset the wanip to default
					mapaddress.LANMask() = basics::ipany;
					mapaddress.SetWANIP(basics::ipany);
				}
				// and send it to char
				chrif_connect(char_fd);
			}
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
	add_timer_interval(gettick() + 1000, 10 * 1000, check_connect_char_server, 0, 0);

	add_timer_func_list(check_dropped_mapwan, "check_dropped_mapwan");
	add_timer_interval(gettick() + 10000, 600 * 1000, check_dropped_mapwan, 0, 0);

	add_timer_func_list(send_users_tochar, "send_users_tochar");
	add_timer_interval(gettick() + 1000, 5 * 1000, send_users_tochar, 0, 0);
	return 0;
}
