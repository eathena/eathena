// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// original : char2.c 2003/03/14 11:58:35 Rev.1.5

#include "core.h"
#include "socket.h"
#include "timer.h"
#include "db.h"
#include "malloc.h"
#include "version.h"
#include "lock.h"
#include "char.h"
#include "utils.h"
#include "showmsg.h"

#include "inter.h"
#include "int_pet.h"
#include "int_guild.h"
#include "int_party.h"
#include "int_storage.h"
#include "irc.h"


///////////////////////////////////////////////////////////////////////////////
/// chardb
CCharDB		char_db;
CVarDB		var_db;

///////////////////////////////////////////////////////////////////////////////
basics::netaddress	loginaddress(basics::ipaddress::GetSystemIP(0), 6900);	 // first lanip as default
basics::ipset		charaddress(6121);								 // automatic setup as default

int login_fd= -1;
int char_fd = -1;
struct mmo_map_server server[MAX_MAP_SERVERS];

char userid[24]="";
char passwd[24]="";
char server_name[20] = "Server";
char wisp_server_name[24] = "Server";



///////////////////////////////////////////////////////////////////////////////
int char_maintenance = 0;
int char_new_display;
int email_creation = 0; // disabled by default
char unknown_char_name[1024] = "Unknown";

bool check_ip_flag = true;

int max_connect_user = 0;
int gm_allow_level = 99;
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;


///////////////////////////////////////////////////////////////////////////////
// online players by [Yor]
char online_txt_filename[1024] = "online.txt";
char online_html_filename[1024] = "online.html";
int online_sorting_option = 0; // sorting option to display online players in online files
int online_display_option = 1; // display options: to know which columns must be displayed
size_t online_players_max;
size_t online_refresh_html = 20; // refresh time (in sec) of the html file in the explorer
size_t online_gm_display_min_level = 20; // minimum GM level to display 'GM' when we want to display it



time_t update_online; // to update online files when we receiving information from a server (not less than 8 seconds)

bool console = false;



///////////////////////////////////////////////////////////////////////////////
// Writing function of logs file
//!!
int log_char = 1;	// loggin char or not [devil]
int log_inter = 1;	// loggin inter or not [devil]
char char_log_filename[1024] = "log/char.log";

int char_log(char *fmt, ...)
{
	if(log_char)
	{
		struct timeval tv;
		time_t unixtime;
		char tmpstr[2048];
		FILE *logfp = basics::safefopen(char_log_filename, "a");
		if (logfp)
		{
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(logfp, RETCODE);
			else
			{
				va_list ap;

				gettimeofday(&tv, NULL);
				unixtime = tv.tv_sec;
				strftime(tmpstr, 24, "%d-%m-%Y %H:%M:%S", localtime(&unixtime));
				sprintf(tmpstr + 19, ".%03ld: %s", tv.tv_usec / 1000, fmt);
				
				va_start(ap, fmt);
				vfprintf(logfp, tmpstr, ap);
				va_end(ap);
			}
			fclose(logfp);
		}
		
	}
	return 0;
}




///////////////////////////////////////////////////////////////////////////////
// Set Character online/offline [Wizputer]
void set_char_online(uint32 char_id, uint32 account_id)
{
	if( !session_isActive(login_fd) )
		return;
	WFIFOW(login_fd,0) = 0x272b;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
	//ShowMessage ("set online\n");
}
void set_char_offline(uint32 char_id, uint32 account_id)
{
	if( !session_isActive(login_fd) )
		return;
	WFIFOW(login_fd,0) = 0x272c;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
	//ShowMessage ("set offline\n");
}
void set_all_offline(void)
{
	if( !session_isActive(login_fd) )
		return;
	WFIFOW(login_fd,0) = 0x272c;
	WFIFOL(login_fd,2) = 99;
	WFIFOSET(login_fd,6);
	//ShowMessage ("set all offline\n");
}

///////////////////////////////////////////////////////////////////////////////
// Function to save (in a periodic way) datas in files
int mmo_char_sync_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	inter_save();
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// Function to create the online files (txt and html). by [Yor]
void create_online_files(void)
{
//!! rewrite
/*	size_t i, j, k, l; // for loops
	uint32 sv;
	size_t players;    // count the number of players
	FILE *fptext;       // for the txt file
	FILE *fphtm・      // for the html file
	char temp[256];      // to prepare what we must display
	time_t time_server;  // for number of seconds
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	CREATE_BUFFER(id,int,online_players_max);

	// don't return here if we display nothing, because server[j].users is updated in the first loop.

	// Get number of online players, id of each online players, and verify if a server is offline
	players = 0;
	for (i = 0; i < online_players_max; ++i) {
		if( online_chars[i].char_id != 0xFFFFFFFF ) {
			// check if map-server is online
			sv = online_chars[i].server;
			if(sv == 0xFFFFFFFF) {
				online_chars[i].char_id = 0xFFFFFFFF;
				continue;
			} else if( server[sv].fd < 0) {
				server[sv].users = 0;
				online_chars[i].char_id = 0xFFFFFFFF;
				online_chars[i].server  = 0xFFFFFFFF;
				continue;
			}
			// check if the character is twice or more in the list
			// (multiple map-servers and player have successfully connected twice!)
			for(j = i + 1; j < online_players_max; ++j)
			{
				if (online_chars[i].char_id == online_chars[j].char_id) {
					sv = online_chars[j].server;
					if(sv != 0xFFFFFFFF && server[sv].fd >= 0 && server[sv].users > 0)
						server[sv].users--;
					online_chars[j].char_id = 0xFFFFFFFF;
					online_chars[j].server  = 0xFFFFFFFF;
				}
			}
			// search position of character in char_dat and sort online characters.
			for(j = 0; j < char_num; ++j)
			{
				if( char_dat[j].char_id == online_chars[i].char_id )
				{
					id[players] = j;
					// use sorting option
					switch (online_sorting_option) {
					case 1: // by name (without case sensitive)
						for(k = 0; k < players; ++k)
							if(strcasecmp(char_dat[j].name, char_dat[id[k]].name) < 0 ||
							   // if same name, we sort with case sensitive.
							   (strcasecmp(char_dat[j].name, char_dat[id[k]].name) == 0 &&
							    strcmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 2: // by zeny
						for(k = 0; k < players; ++k)
							if (char_dat[j].zeny < char_dat[id[k]].zeny ||
							   // if same number of zenys, we sort by name.
							   (char_dat[j].zeny == char_dat[id[k]].zeny &&
							    strcasecmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 3: // by base level
						for(k = 0; k < players; ++k)
							if (char_dat[j].base_level < char_dat[id[k]].base_level ||
							   // if same base level, we sort by base exp.
							   (char_dat[j].base_level == char_dat[id[k]].base_level &&
							    char_dat[j].base_exp < char_dat[id[k]].base_exp)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 4: // by job (and job level)
						for(k = 0; k < players; ++k)
							if (char_dat[j].class_ < char_dat[id[k]].class_ ||
							   // if same job, we sort by job level.
							   (char_dat[j].class_ == char_dat[id[k]].class_ &&
							    char_dat[j].job_level < char_dat[id[k]].job_level) ||
							   // if same job and job level, we sort by job exp.
							   (char_dat[j].class_ == char_dat[id[k]].class_ &&
							    char_dat[j].job_level == char_dat[id[k]].job_level &&
							    char_dat[j].job_exp < char_dat[id[k]].job_exp)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 5: // by location map name
						for(k = 0; k < players; ++k)
							if(strcasecmp(char_dat[j].last_point.map, char_dat[id[k]].last_point.map) < 0 ||
							   // if same map name, we sort by name.
							   (strcasecmp(char_dat[j].last_point.map, char_dat[id[k]].last_point.map) == 0 &&
							    strcasecmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					default: // 0 or invalid value: no sorting
						break;
					}
					players++;
					break;
				}
			}
		}
	}

	if (online_display_option == 0) // we display nothing, so return
	{
		DELETE_BUFFER(id);
		return;
	}

	// write files
	fp = safefopen(online_txt_filename, "w");
	if (fp != NULL) {
		fp2 = safefopen(online_html_filename, "w");
		if (fp2 != NULL) {
			// get time
			time(&time_server); // get time in seconds since 1/1/1970
			datetime = localtime(&time_server); // convert seconds in structure
			strftime(temp, sizeof(temp), "%d %b %Y %X", datetime); // like sprintf, but only for date/time (05 dec 2003 15:12:52)
			// write heading
			fprintf(fp2, "<HTML>\n");
			fprintf(fp2, "  <META http-equiv=\"Refresh\" content=\"%d\">\n", online_refresh_html); // update on client explorer every x seconds
			fprintf(fp2, "  <HEAD>\n");
			fprintf(fp2, "    <TITLE>Online Players on %s</TITLE>\n", server_name);
			fprintf(fp2, "  </HEAD>\n");
			fprintf(fp2, "  <BODY>\n");
			fprintf(fp2, "    <H3>Online Players on %s (%s):</H3>\n", server_name, temp);
			fprintf(fp, "Online Players on %s (%s):\n", server_name, temp);
			fprintf(fp, "\n");

			for (i = 0; i < players; ++i) {
				// if it's the first player
				if (i == 0) {
					j = 0; // count the number of characters for the txt version and to set the separate line
					fprintf(fp2, "    <table border=\"1\" cellspacing=\"1\">\n");
					fprintf(fp2, "      <tr>\n");
					if ((online_display_option & 1) || (online_display_option & 64)) {
						fprintf(fp2, "        <td><b>Name</b></td>\n");
						if (online_display_option & 64) {
							fprintf(fp, "Name                          "); // 30
							j += 30;
						} else {
							fprintf(fp, "Name                     "); // 25
							j += 25;
						}
					}
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td><b>Job (levels)</b></td>\n");
						fprintf(fp, "Job                 Levels "); // 27
						j += 27;
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td><b>Job</b></td>\n");
						fprintf(fp, "Job                "); // 19
						j += 19;
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td><b>Levels</b></td>\n");
						fprintf(fp, " Levels "); // 8
						j += 8;
					}
					if (online_display_option & 24) { // 8 or 16
						fprintf(fp2, "        <td><b>Location</b></td>\n");
						if (online_display_option & 16) {
							fprintf(fp, "Location     ( x , y ) "); // 23
							j += 23;
						} else {
							fprintf(fp, "Location     "); // 13
							j += 13;
						}
					}
					if (online_display_option & 32) {
						fprintf(fp2, "        <td ALIGN=CENTER><b>zenys</b></td>\n");
						fprintf(fp, "          Zenys "); // 16
						j += 16;
					}
					fprintf(fp2, "      </tr>\n");
					fprintf(fp, "\n");
					for (k = 0; k < j; ++k)
						fprintf(fp, "-");
					fprintf(fp, "\n");
				}
				fprintf(fp2, "      <tr>\n");
				// get id of the character (more speed)
				j = id[i];
				// displaying the character name
				if ((online_display_option & 1) || (online_display_option & 64)) { // without/with 'GM' display
					strcpy(temp, char_dat[j].name);
					l = char_dat[j].gm_level;
					if (online_display_option & 64) {
						if (l >= online_gm_display_min_level)
							fprintf(fp, "%-24s (GM) ", temp);
						else
							fprintf(fp, "%-24s      ", temp);
					} else
						fprintf(fp, "%-24s ", temp);
					// name of the character in the html (no < >, because that create problem in html code)
					fprintf(fp2, "        <td>");
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
						fprintf(fp2, "<b>");
					for (k = 0; k < strlen(temp); ++k) {
						switch(temp[k]) {
						case '<': // <
							fprintf(fp2, "&lt;");
							break;
						case '>': // >
							fprintf(fp2, "&gt;");
							break;
						default:
							fprintf(fp2, "%c", temp[k]);
							break;
						};
					}
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
						fprintf(fp2, "</b> (GM)");
					fprintf(fp2, "</td>\n");
				}
				// displaying of the job
				if (online_display_option & 6) {
					const char* jobname = job_name(char_dat[j].class_);
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td>%s %d/%d</td>\n", jobname, char_dat[j].base_level, char_dat[j].job_level);
						fprintf(fp, "%-18s %3d/%3d ", jobname, char_dat[j].base_level, char_dat[j].job_level);
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td>%s</td>\n", jobname);
						fprintf(fp, "%-18s ", jobname);
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td>%d/%d</td>\n", char_dat[j].base_level, char_dat[j].job_level);
						fprintf(fp, "%3d/%3d ", char_dat[j].base_level, char_dat[j].job_level);
					}
				}
				// displaying of the map
				if (online_display_option & 24) { // 8 or 16
					// prepare map name
					memset(temp, 0, 17);
					memcpy(temp, char_dat[j].last_point.map, 16);
					if (strstr(temp, ".gat") != NULL) {
						temp[strstr(temp, ".gat") - temp] = 0; // suppress the '.gat'
					}
					// write map name
					if (online_display_option & 16) { // map-name AND coordonates
						fprintf(fp2, "        <td>%s (%d, %d)</td>\n", temp, char_dat[j].last_point.x, char_dat[j].last_point.y);
						fprintf(fp, "%-12s (%3d,%3d) ", temp, char_dat[j].last_point.x, char_dat[j].last_point.y);
					} else {
						fprintf(fp2, "        <td>%s</td>\n", temp);
						fprintf(fp, "%-12s ", temp);
					}
				}
				// displaying number of zenys
				if (online_display_option & 32) {
					// write number of zenys
					if (char_dat[j].zeny == 0) { // if no zeny
						fprintf(fp2, "        <td ALIGN=RIGHT>no zeny</td>\n");
						fprintf(fp, "        no zeny ");
					} else {
						fprintf(fp2, "        <td ALIGN=RIGHT>%ld z</td>\n", char_dat[j].zeny);
						fprintf(fp, "%13ld z ", char_dat[j].zeny);
					}
				}
				fprintf(fp, "\n");
				fprintf(fp2, "      </tr>\n");
			}
			// If we display at least 1 player
			if (players > 0) {
				fprintf(fp2, "    </table>\n");
				fprintf(fp, "\n");
			}

			// Displaying number of online players
			if (players == 0) {
				fprintf(fp2, "    <p>No user is online.</p>\n");
				fprintf(fp, "No user is online.\n");
			} else if (players == 1) {
				fprintf(fp2, "    <p>%d user is online.</p>\n", players);
				fprintf(fp, "%d user is online.\n", players);
			} else {
				fprintf(fp2, "    <p>%d users are online.</p>\n", players);
				fprintf(fp, "%d users are online.\n", players);
			}
			fprintf(fp2, "  </BODY>\n");
			fprintf(fp2, "</HTML>\n");
			fclose(fp2);
		}
		fclose(fp);
	}
	DELETE_BUFFER(id);
	return;
*/
}

///////////////////////////////////////////////////////////////////////////////
// This function return the number of online players in all map-servers
int count_users(void)
{
	size_t i, users = 0;
	for(i = 0; i < MAX_MAP_SERVERS; ++i)
		if(server[i].fd >= 0)
			users += server[i].users;
	return users;
}


///////////////////////////////////////////////////////////////////////////////
// Function to send characters to a player
int mmo_char_send006b(int fd, char_session_data &sd)
{
	size_t i, j, found_num = 0;
	const int offset = 24;
	CCharCharacter character;

	if( !session_isActive(fd) )
		return 0;
	set_char_online(99, sd.account_id);

	memset(WFIFOP(fd,0), 0, offset + 9*106);
	WFIFOW(fd,0) = 0x6b;
	for(i=0; i<9; ++i)
	{
		if( sd.charlist[i]!=0 &&
			char_db.searchChar(sd.charlist[i], character) )
		{
			j = offset + (found_num*106);

			WFIFOL(fd,j) = character.char_id;
			WFIFOL(fd,j+4) = character.base_exp;
			WFIFOL(fd,j+8) = character.zeny;
			WFIFOL(fd,j+12) = character.job_exp;
			WFIFOL(fd,j+16) = character.job_level;

			WFIFOL(fd,j+20) = 0;
			WFIFOL(fd,j+24) = 0;
			WFIFOL(fd,j+28) = character.option;

			WFIFOL(fd,j+32) = character.karma;
			WFIFOL(fd,j+36) = character.manner;

			WFIFOW(fd,j+40) = character.status_point;
			WFIFOW(fd,j+42) = (unsigned short)((character.hp > 0x7fff) ? 0x7fff : character.hp);
			WFIFOW(fd,j+44) = (unsigned short)((character.max_hp > 0x7fff) ? 0x7fff : character.max_hp);
			WFIFOW(fd,j+46) = (unsigned short)((character.sp > 0x7fff) ? 0x7fff : character.sp);
			WFIFOW(fd,j+48) = (unsigned short)((character.max_sp > 0x7fff) ? 0x7fff : character.max_sp);
			WFIFOW(fd,j+50) = DEFAULT_WALK_SPEED; // p->speed;
			WFIFOW(fd,j+52) = character.class_;
			WFIFOW(fd,j+54) = character.hair;

			// pecopeco knights/crusaders crash fix
			if (character.class_ == 13 || character.class_ == 21 ||
				character.class_ == 4014 || character.class_ == 4022 ||
				character.class_ == 4036 || character.class_ == 4044)
				WFIFOW(fd,j+56) = 0;
			else 
				WFIFOW(fd,j+56) = character.weapon;

			WFIFOW(fd,j+58) = character.base_level;
			WFIFOW(fd,j+60) = character.skill_point;
			WFIFOW(fd,j+62) = character.head_bottom;
			WFIFOW(fd,j+64) = character.shield;
			WFIFOW(fd,j+66) = character.head_top;
			WFIFOW(fd,j+68) = character.head_mid;
			WFIFOW(fd,j+70) = character.hair_color;
			WFIFOW(fd,j+72) = character.clothes_color;

			memcpy(WFIFOP(fd,j+74), character.name, 24);

			WFIFOB(fd,j+98) = (character.str > 255) ? 255 : character.str;
			WFIFOB(fd,j+99) = (character.agi > 255) ? 255 : character.agi;
			WFIFOB(fd,j+100) = (character.vit > 255) ? 255 : character.vit;
			WFIFOB(fd,j+101) = (character.int_ > 255) ? 255 : character.int_;
			WFIFOB(fd,j+102) = (character.dex > 255) ? 255 : character.dex;
			WFIFOB(fd,j+103) = (character.luk > 255) ? 255 : character.luk;
			WFIFOB(fd,j+104) = character.slot;

			found_num++;
		}
	}
	WFIFOW(fd,2) = offset+found_num*106;
	WFIFOSET(fd, offset+found_num*106);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// 離婚(char削除時に使用)
bool char_divorce(CCharCharacter &character)
{
	if(character.partner_id != 0)
	{
		size_t j;
		CCharCharacter partner;
		if( char_db.searchChar( character.partner_id, partner ) &&
			partner.partner_id==character.char_id )
		{
			partner.partner_id=0;
			for(j=0; j<MAX_INVENTORY; ++j)
			{
				if(partner.inventory[j].nameid == WEDDING_RING_M || partner.inventory[j].nameid == WEDDING_RING_F)
				{
					partner.inventory[j] = item();
					break;
				}
			}
			char_db.saveChar(partner);
		}
		character.partner_id = 0;
		for(j=0; j<MAX_INVENTORY; ++j)
		{
			if(character.inventory[j].nameid == WEDDING_RING_M || character.inventory[j].nameid == WEDDING_RING_F)
			{
				character.inventory[j] = item();
				break;
			}
		}
		char_db.saveChar(character);
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
bool char_married(uint32 p1_id, uint32 p2_id)
{
	CCharCharacter partner1, partner2;
	return ( char_db.searchChar(p1_id, partner1) &&
			 char_db.searchChar(p2_id, partner2) &&
			 p1_id == partner2.partner_id && 
			 p2_id == partner1.partner_id );
}
bool char_married(const char* p1_name, const char* p2_name)
{
	CCharCharacter partner1, partner2;
	return ( char_db.searchChar(p1_name, partner1) &&
			 char_db.searchChar(p2_name, partner2) &&
			 partner1.char_id == partner2.partner_id && 
			 partner2.char_id == partner1.partner_id );
}
///////////////////////////////////////////////////////////////////////////////
bool char_child(uint32 parent_id, uint32 child_id)
{
	CCharCharacter parent, child;
	return ( char_db.searchChar(parent_id, parent) &&
			 char_db.searchChar(child_id, child) &&
			 parent.child_id == child_id &&
			 (parent_id == child.father_id || parent_id == child.mother_id) );
}
bool char_child(const char* parent_name, const char* child_name)
{
	CCharCharacter parent, child;
	return ( char_db.searchChar(parent_name, parent) &&
			 char_db.searchChar(child_name, child) &&
			 parent.child_id == child.char_id &&
			 (parent.char_id == child.father_id || parent.char_id == child.mother_id) );
}
///////////////////////////////////////////////////////////////////////////////
bool char_family(uint32 id1, uint32 id2, uint32 id3)
{
	CCharCharacter ch1, ch2, ch3;
	return ( char_db.searchChar(id1, ch1) &&
			 char_db.searchChar(id2, ch2) &&
			 char_db.searchChar(id3, ch3) &&
			 ( (ch1.char_id==ch2.partner_id && ch2.char_id==ch1.partner_id && ch3.char_id==ch1.child_id && ch3.char_id==ch2.child_id && ((ch1.char_id==ch3.father_id && ch2.char_id==ch3.mother_id)||(ch1.char_id==ch3.mother_id && ch2.char_id==ch3.father_id)) ) ||
			   (ch1.char_id==ch3.partner_id && ch3.char_id==ch1.partner_id && ch2.char_id==ch1.child_id && ch2.char_id==ch3.child_id && ((ch1.char_id==ch2.father_id && ch3.char_id==ch2.mother_id)||(ch1.char_id==ch2.mother_id && ch3.char_id==ch2.father_id)) ) ||
			   (ch3.char_id==ch2.partner_id && ch2.char_id==ch3.partner_id && ch1.char_id==ch3.child_id && ch1.char_id==ch2.child_id && ((ch3.char_id==ch1.father_id && ch2.char_id==ch1.mother_id)||(ch3.char_id==ch1.mother_id && ch2.char_id==ch1.father_id)) ) ) );
}
bool char_family(const char* name1, const char* name2, const char* name3)
{
	CCharCharacter ch1, ch2, ch3;
	return ( char_db.searchChar(name1, ch1) &&
			 char_db.searchChar(name2, ch2) &&
			 char_db.searchChar(name3, ch3) &&
			 ( (ch1.char_id==ch2.partner_id && ch2.char_id==ch1.partner_id && ch3.char_id==ch1.child_id && ch3.char_id==ch2.child_id && ((ch1.char_id==ch3.father_id && ch2.char_id==ch3.mother_id)||(ch1.char_id==ch3.mother_id && ch2.char_id==ch3.father_id)) ) ||
			   (ch1.char_id==ch3.partner_id && ch3.char_id==ch1.partner_id && ch2.char_id==ch1.child_id && ch2.char_id==ch3.child_id && ((ch1.char_id==ch2.father_id && ch3.char_id==ch2.mother_id)||(ch1.char_id==ch2.mother_id && ch3.char_id==ch2.father_id)) ) ||
			   (ch3.char_id==ch2.partner_id && ch2.char_id==ch3.partner_id && ch1.char_id==ch3.child_id && ch1.char_id==ch2.child_id && ((ch3.char_id==ch1.father_id && ch2.char_id==ch1.mother_id)||(ch3.char_id==ch1.mother_id && ch2.char_id==ch1.father_id)) ) ) );
}
///////////////////////////////////////////////////////////////////////////////
bool char_exist(const char* name)
{
	return ( char_db.existChar(name) );
}
bool char_exist(uint32 id)
{
	CCharCharacter ch;
	return ( char_db.searchChar(id, ch) );
}



///////////////////////////////////////////////////////////////////////////////
// Force disconnection of an online player (with account value) by [Yor]
bool disconnect_player(uint32 accound_id)
{
	size_t fd;
	char_session_data *sd;
	// disconnect player if online on char-server
	for(fd = 0; fd < fd_max; ++fd)
	{
		if(session[fd] && (sd = (char_session_data *)session[fd]->user_session))
		{
			if(sd->account_id == accound_id)
			{
				session_Remove(fd);
				return true;
			}
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
// キャラ削除に伴うデータ削除
bool char_delete(uint32 char_id)
{
	CCharCharacter character;
	if( char_db.searchChar(char_id,character) )
	{
		int j;

		// ペット削除
		if(character.pet_id)
			inter_pet_delete(character.pet_id);
		for(j=0; j<MAX_INVENTORY; ++j)
			if(character.inventory[j].card[0] == 0xff00)
				inter_pet_delete(basics::MakeDWord(character.inventory[j].card[1],character.inventory[j].card[2]));
		for(j=0; j<MAX_CART; ++j)
			if(character.cart[j].card[0] == 0xff00)
				inter_pet_delete(basics::MakeDWord(character.cart[j].card[1],character.cart[j].card[2]));
		// ギルド脱退
		if(character.guild_id)
			inter_guild_leave(character.guild_id, character.account_id, character.char_id);
		// パーティー脱退
		if(character.party_id)
			inter_party_leave(character.party_id, character.account_id);
		// 離婚
		if(character.partner_id)
		{
			// 離婚情報をmapに通知
			unsigned char buf[10];
			WBUFW(buf,0) = 0x2b12;
			WBUFL(buf,2) = character.char_id;
			WBUFL(buf,6) = character.partner_id;
			mapif_sendall(buf,10);
			// 離婚
			char_divorce(character);
		}
		return char_db.removeChar(char_id);
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
int parse_tologin(int fd)
{
	size_t i;
	///////////////////////////////////////////////////////////////////////////
	// only login-server can have an access to here.
	// so, if it isn't the login-server, we disconnect the session.
	if (fd != login_fd)
	{
		session_Remove(fd);
		return 0;
	}
	// else it is the login
	if( !session_isActive(fd) )
	{	// login is disconnecting
		ShowMessage("Connection to login-server dropped (connection #%d).\n", fd);
		session_Remove(fd);// have it removed by do_sendrecv
		login_fd = -1;
		return 0;
	}
	while(RFIFOREST(fd) >= 2)
	{
		unsigned short command = RFIFOW(fd,0);
//		ShowMessage("parse_tologin: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, command, RFIFOREST(fd));
		switch(command)
		{
		///////////////////////////////////////////////////////////////////////
		// connect reply from login 
		case 0x2711:
		{
			if (RFIFOREST(fd) < 3)
				return 0;
			if( 0==RFIFOB(fd, 2) )
			{
				ShowStatus("Connected to login-server (connection #%d).\n", fd);
				// if no map-server already connected, display a message...
				for(i = 0; i < MAX_MAP_SERVERS; ++i)
					if(server[i].fd >= 0 && server[i].map[0][0]) // if map-server online and at least 1 map
						break;
				if (i == MAX_MAP_SERVERS)
					ShowStatus("Awaiting maps from map-server.\n");
			}
			else
			{
				//ShowMessage("connect login server error : %d\n", (unsigned char)RFIFOB(fd,2));
				ShowMessage("Can not connect to login-server.\n");
				ShowMessage("The server communication passwords (default s1/p1) is probably invalid.\n");
				ShowMessage("Also, please make sure your accounts file (default: accounts.txt) has those values present.\n");
				ShowMessage("If you changed the communication passwords, change them back at map_athena.conf and char_athena.conf\n");
				core_stoprunning();
			}
			RFIFOSKIP(fd, 3);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// get authentification from login
// might be obsolete, covered by 0x2750
		case 0x2713:
		{
			if (RFIFOREST(fd) < 51)
				return 0;

			char_session_data *sd;
			CCharCharAccount account;
			// search session 
			//!! add a connection db
			for(i = 0; i < fd_max; ++i)
			{
				if(session[i] && (sd = (char_session_data *)session[i]->user_session) && sd->account_id == RFIFOL(fd,2))
				{
					// check authentification if login authentified and send the data
					if( RFIFOB(fd,6) == 0 &&
						char_db.searchAccount(RFIFOL(fd,2), account) &&
						account.client_ip   == sd->client_ip &&
						account.account_id  == sd->account_id &&
						account.login_id1   == sd->login_id1 &&
						account.login_id2   == sd->login_id2 && 
						account.sex         == sd->sex )
					{
						if(max_connect_user == 0 || count_users() < max_connect_user || account.gm_level >= gm_allow_level)
						{
							// make a complete copy of the account data at the session
							sd->CCharCharAccount::operator=(account);

							if( sd->gm_level )
								ShowInfo("Account Logged On; Account ID: %ld (GM level %d).\n", (unsigned long)sd->account_id, sd->gm_level);
							else
								ShowInfo("Account Logged On; Account ID: %ld.\n", (unsigned long)sd->account_id);

							// send characters to player
							mmo_char_send006b(fd, *sd);
						}
						else
						{
							// refuse connection (over populated)
							WFIFOW(fd,0) = 0x6c;
							WFIFOW(fd,2) = 0;
							WFIFOSET(fd,3);
						}
					}
					else
					{	// reject, not authentified
						WFIFOW(i,0) = 0x6c;
						WFIFOB(i,2) = 0x42;
						WFIFOSET(i,3);
					}
					break;
				}
			}// end for
			RFIFOSKIP(fd,51);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// login-server alive packet
		case 0x2718:
		{
			if (RFIFOREST(fd) < 2)
				return 0;

			RFIFOSKIP(fd,2);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// Receiving authentification from Freya-type login server (to avoid char->login->char)
// obsolete
		case 0x2719:
			if (RFIFOREST(fd) < 18)
				return 0;
/*
			// to conserv a maximum of authentification, search if account is already authentified and replace it
			// that will reduce multiple connection too
			for(i = 0; i < AUTH_FIFO_SIZE; ++i)
				if (auth_fifo[i].account_id == RFIFOL(fd,2))
					break;
			// if not found, use next value
			if (i == AUTH_FIFO_SIZE) {
				if (auth_fifo_pos >= AUTH_FIFO_SIZE)
					auth_fifo_pos = 0;
				i = auth_fifo_pos;
				auth_fifo_pos++;
			}
			//ShowMessage("auth_fifo set (auth #%d) - account: %d, secure: %08x-%08x\n", i, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
			auth_fifo[i].account_id = RFIFOL(fd,2);
			auth_fifo[i].char_id = 0;
			auth_fifo[i].login_id1 = RFIFOL(fd,6);
			auth_fifo[i].login_id2 = RFIFOL(fd,10);
			auth_fifo[i].delflag = 2; // 0: auth_fifo canceled/void, 2: auth_fifo received from login/map server in memory, 1: connection authentified
			auth_fifo[i].char_pos = 0;
			auth_fifo[i].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[i].client_ip = RFIFOLIP(fd,14);
			//auth_fifo[i].map_auth = 0;
*/
			RFIFOSKIP(fd,18);
			break;

		///////////////////////////////////////////////////////////////////////
		// gm reply
// obsolete, 0x2750
		case 0x2721:	
		{
			if (RFIFOREST(fd) < 10)
				return 0;
		  {
			unsigned char buf[10];
			WBUFW(buf,0) = 0x2b0b;
			WBUFL(buf,2) = RFIFOL(fd,2); // account
			WBUFL(buf,6) = RFIFOL(fd,6); // GM level
			mapif_sendall(buf,10);
//			ShowMessage("parse_tologin: To become GM answer: char -> map.\n");
		  }
			RFIFOSKIP(fd,10);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// changesex reply (modified by [Yor])
// obsolete in the current state, merge with 0x2750
		case 0x2723:	
		{
			if (RFIFOREST(fd) < 7)
				return 0;
/*
			uint32 acc;
			int sex;
			size_t i, j;
			unsigned char buf[7];
			acc = RFIFOL(fd,2);
			sex = RFIFOB(fd,6);
			
			if (acc > 0) {
				for (i = 0; i < char_num; ++i) {
					if (char_dat[i].account_id == acc) {
						int jobclass = char_dat[i].class_;
						char_dat[i].sex = sex;
						auth_fifo[i].sex = sex;
						if (jobclass == 19 || jobclass == 20 ||
						    jobclass == 4020 || jobclass == 4021 ||
						    jobclass == 4042 || jobclass == 4043) {
							// job modification
							if (jobclass == 19 || jobclass == 20) {
								char_dat[i].class_ = (sex) ? 19 : 20;
							} else if (jobclass == 4020 || jobclass == 4021) {
								char_dat[i].class_ = (sex) ? 4020 : 4021;
							} else if (jobclass == 4042 || jobclass == 4043) {
								char_dat[i].class_ = (sex) ? 4042 : 4043;
							}
							// remove specifical skills of classes 19, 4020 and 4042
							for(j = 315; j <= 322; ++j) {
								if (char_dat[i].skill[j].id > 0 && !char_dat[i].skill[j].flag) {
									char_dat[i].skill_point += char_dat[i].skill[j].lv;
									char_dat[i].skill[j].id = 0;
									char_dat[i].skill[j].lv = 0;
								}
							}
							// remove specifical skills of classes 20, 4021 and 4043
							for(j = 323; j <= 330; ++j) {
								if (char_dat[i].skill[j].id > 0 && !char_dat[i].skill[j].flag) {
									char_dat[i].skill_point += char_dat[i].skill[j].lv;
									char_dat[i].skill[j].id = 0;
									char_dat[i].skill[j].lv = 0;
								}
							}
						}
						// to avoid any problem with equipment and invalid sex, equipment is unequiped.
						for (j = 0; j < MAX_INVENTORY; ++j) {
							if (char_dat[i].inventory[j].nameid && char_dat[i].inventory[j].equip)
								char_dat[i].inventory[j].equip = 0;
						}
						char_dat[i].weapon = 0;
						char_dat[i].shield = 0;
						char_dat[i].head_top = 0;
						char_dat[i].head_mid = 0;
						char_dat[i].head_bottom = 0;
					}
				}
				// disconnect player if online on char-server
				disconnect_player(acc);
			}
			WBUFW(buf,0) = 0x2b0d;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			mapif_sendall(buf, 7);
*/
			RFIFOSKIP(fd, 7);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Request to send a broadcast message (no answer)
		case 0x2726:
		{
			if(RFIFOREST(fd) < 8 || (size_t)RFIFOREST(fd) < (8 + RFIFOL(fd,4)))
				return 0;
			if (RFIFOL(fd,4) < 1)
				char_log("Receiving a message for broadcast, but message is void." RETCODE);
			else
			{	// at least 1 map-server
				for(i = 0; i < MAX_MAP_SERVERS; ++i)
					if(server[i].fd >= 0)
						break;
				if (i == MAX_MAP_SERVERS)
					char_log("'ladmin': Receiving a message for broadcast, but no map-server is online." RETCODE);
				else
				{
					int lp;
					char *p;
					unsigned char buf[128];
					char *message = (char*)RFIFOP(fd,8);
					
					message[RFIFOL(fd,4)] = 0;
					remove_control_chars(message);
					// remove all first spaces
					p = message;
					while(p[0] == ' ')
						p++;
					// if message is only composed of spaces
					if(p[0] == '\0')
						char_log("Receiving a message for broadcast, but message is only a lot of spaces." RETCODE);
					// else send message to all map-servers
					else
					{
						if(RFIFOW(fd,2) == 0)
						{
							char_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s)" RETCODE,
								message);
							lp = 4;
						}
						else
						{
							char_log("'ladmin': Receiving a message for broadcast (message (in blue): %s)" RETCODE,
								message);
							lp = 8;
						}
						// split message to max 80 char
						while(p[0] != '\0')
						{	// if not finish
							if (p[0] == ' ') // jump if first char is a space
								p++;
							else
							{
								char split[80];
								char* last_space;
								sscanf(p, "%79[^\t]", split); // max 79 char, any char (\t is control char and control char was removed before)
								split[sizeof(split)-1] = '\0'; // last char always \0
								if ((last_space = strrchr(split, ' ')) != NULL)
								{	// searching space from end of the string
									last_space[0] = '\0'; // replace it by NULL to have correct length of split
									p++; // to jump the new NULL
								}
								p += strlen(split);
								// send broadcast to all map-servers
								WBUFW(buf,0) = 0x3800;
								WBUFW(buf,2) = lp + strlen(split) + 1;
								WBUFL(buf,4) = 0x65756c62; // only write if in blue (lp = 8)
								memcpy(WBUFP(buf,lp), split, strlen(split) + 1);
								mapif_sendall(buf, WBUFW(buf,2));
							}
						}
					}
				}
			}
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// account_reg2変更通知
// will be obsolete
		case 0x2729:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			size_t j,p,sz = RFIFOW(fd,2);
			uint32 accid = RFIFOL(fd,4);
			CCharCharAccount account;
			if( char_db.searchAccount(accid,account) )
			{
				for(p = 8, j = 0; p < sz && j < ACCOUNT_REG2_NUM; p += 36, ++j)
				{
					memcpy(account.account_reg2[j].str, RFIFOP(fd,p), 32);
					account.account_reg2[j].value = RFIFOL(fd,p+32);
				}
				char_db.saveAccount(account);
			}
			// 同垢ログインを禁止していれば送る必要は無い
			// modify and write directly out of the readfifo 
			RFIFOW(fd,0) = 0x2b11;
			mapif_sendall(RFIFOP(fd,0), sz);
			RFIFOSKIP(fd, sz);
//			ShowMessage("char: save_account_reg_reply\n");

			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Account deletion notification (from login-server)
		case 0x2730:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			// disconnect player if online on char-server
			disconnect_player(RFIFOL(fd,2));

			// Deletion of all characters of the account
			CCharCharAccount account;
			if( char_db.searchAccount(RFIFOL(fd,2), account) )
			{
				size_t i;
				for(i=0; i<9; ++i)
				{
					if( account.charlist[i] != 0)
						char_delete(account.charlist[i]);
				}
				char_db.removeAccount(account.account_id);
			}
			// Deletion of the storage
			inter_storage_delete(RFIFOL(fd,2));
			// send to all map-servers to disconnect the player
			unsigned char buf[6];
			WBUFW(buf,0) = 0x2b13;
			WBUFL(buf,2) = RFIFOL(fd,2);
			mapif_sendall(buf, 6);

			RFIFOSKIP(fd,6);
			break;
		}
		// State change of account/ban notification (from login-server) by [Yor]
// might need a change
		case 0x2731:
		{
			if (RFIFOREST(fd) < 11)
				return 0;
			// disconnect player if online on char-server
			disconnect_player(RFIFOL(fd,2));

			// send to all map-servers to disconnect the player
			unsigned char buf[11];
			WBUFW(buf,0) = 0x2b14;
			WBUFL(buf,2) = RFIFOL(fd,2);
			WBUFB(buf,6) = RFIFOB(fd,6); // 0: change of statut, 1: ban
			WBUFL(buf,7) = RFIFOL(fd,7); // final date of a banishment
			mapif_sendall(buf, 11);

			RFIFOSKIP(fd,11);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Receiving GM acounts info from login-server (by [Yor])
// obsolete
		case 0x2732:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
/*		  {
			unsigned char buf[32000];
			if (gm_account != NULL)
				aFree(gm_account);
			gm_account = (struct gm_account*)aCalloc(1, sizeof(struct gm_account) * ((RFIFOW(fd,2) - 4) / 5));
			GM_num = 0;
			for (i = 4; i < (size_t)RFIFOW(fd,2); i = i + 5) {
				gm_account[GM_num].account_id = RFIFOL(fd,i);
				gm_account[GM_num].level = (int)RFIFOB(fd,i+4);
				//ShowMessage("GM account: %d -> level %d\n", gm_account[GM_num].account_id, gm_account[GM_num].level);
				GM_num++;
			}
			ShowInfo("From login-server: receiving of %d GM accounts information.\n", GM_num);
			char_log("From login-server: receiving of %d GM accounts information." RETCODE, GM_num);
			create_online_files(); // update online players files (perhaps some online players change of GM level)
			// send new gm acccounts level to map-servers
			memcpy(buf, RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x2b15;
			mapif_sendall((unsigned char*)buf, RFIFOW(fd,2));
		  }
*/
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Receive GM accounts [Freya login server packet by Yor]
// obsolete
		case 0x2733:
		// add test here to remember that the login-server is Freya-type
		// sprintf (login_server_type, "Freya");
			if (RFIFOREST(fd) < 7)
				return 0;
/*			{
				unsigned char buf[32000];
				int new_level = 0;
				for(i = 0; i < GM_num; ++i)
					if (gm_account[i].account_id == RFIFOL(fd,2)) {
						if(gm_account[i].level != RFIFOB(fd,6)) {
							gm_account[i].level = RFIFOB(fd,6);
							new_level = 1;
						}
						break;
					}
				// if not found, add it
				if (i == GM_num) {
					// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
					// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
					if (((int)RFIFOB(fd,6)) > 0 && GM_num < 4000) {
						if (GM_num == 0) {
							gm_account = (struct gm_account*)aMalloc(sizeof(struct gm_account));
						} else {
							gm_account = (struct gm_account*)aRealloc(gm_account, sizeof(struct gm_account) * (GM_num + 1));						
						}
						gm_account[GM_num].account_id = RFIFOL(fd,2);
						gm_account[GM_num].level = (int)RFIFOB(fd,6);
						new_level = 1;
						GM_num++;
						if (GM_num >= 4000) {
							ShowMessage("***WARNING: 4000 GM accounts found. Next GM accounts are not readed.\n");
							char_log("***WARNING: 4000 GM accounts found. Next GM accounts are not readed." RETCODE);
						}
					}
				}
				if (new_level == 1) {
					int len;
					ShowInfo("From login-server: receiving a GM account information (%d: level %d).\n", (uint32)RFIFOL(fd,2), (unsigned char)RFIFOB(fd,6));
					char_log("From login-server: receiving a GM account information (%d: level %d)." RETCODE, (uint32)RFIFOL(fd,2), (unsigned char)RFIFOB(fd,6));
					//create_online_files(); // not change online file for only 1 player (in next timer, that will be done
					// send gm acccounts level to map-servers
					len = 4;
					WBUFW(buf,0) = 0x2b15;
				
					for(i = 0; i < GM_num; ++i) {
						WBUFL(buf, len) = gm_account[i].account_id;
						WBUFB(buf, len+4) = (unsigned char)gm_account[i].level;
						len += 5;
					}
					WBUFW(buf, 2) = len;
					mapif_sendall(buf, len);
				}
			}
*/
			RFIFOSKIP(fd,7);
			break;

		///////////////////////////////////////////////////////////////////////
		// receive auth information
		case 0x2750:
		{
			CCharAccount account;
			
			if( (size_t)RFIFOREST(fd) < 2+account.CCharAccount::size() )
				return 0;
			
			account.CCharAccount::frombuffer( RFIFOP(fd,2) );
			char_db.saveAccount(account);

			// send authentification down to maps
			unsigned char buf[16+sizeof(account)]; // larger then necessary
			WBUFW(buf,0) = 0x2b21;
			// VC7.1 bug, does not determin correct inheritance
			WBUFW(buf,2) = 4+account.CAuth::size();
			account.CAuth::tobuffer(WBUFP(buf, 4));
			mapif_sendall(buf, 4+account.CAuth::size() );
			

			RFIFOSKIP( fd,2+account.CCharAccount::size() );
			break;
		}
		default:
			ShowMessage("parse_tologin: unknown packet %x! \n", (unsigned short)RFIFOW(fd,0));
			session_Remove(fd);
			login_fd = -1;
			return 0;
		}
	}
	return 0;
}

int parse_frommap(int fd)
{
	size_t i, j;
	size_t id;

	for(id = 0; id < MAX_MAP_SERVERS; ++id)
		if(server[id].fd == fd)
			break;
	if(id==MAX_MAP_SERVERS) {
		// not a map server
		session_Remove(fd);
		return 0;
	}
	// else it is a valid map server
	if( !session_isActive(fd) )
	{
		// a map server is disconnecting
		ShowWarning("Map-server %d has disconnected.\n", id);
		server[id].fd = -1;
		session_Remove(fd);// have it removed by do_sendrecv
		create_online_files();
		
		// inform the other map servers of the loss
		unsigned char buf[16384];
		WBUFW(buf,0) = 0x2b20;
		WBUFW(buf,2) = server[id].maps * 16 + 20;
		WBUFLIP(buf,4) = server[id].address.LANIP();
		WBUFLIP(buf,8) = server[id].address.LANMask();
		WBUFW(buf,12) = server[id].address.LANPort();
		WBUFLIP(buf,14) = server[id].address.WANIP();
		WBUFW(buf,18) = server[id].address.WANPort();

		for(i=20, j=0; j<server[id].maps; i+=16, ++j)
			memcpy(RBUFP(buf,i), server[id].map[j], 16);
		mapif_sendallwos(fd, buf, server[id].maps * 16 + 20);
		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		unsigned short command=RFIFOW(fd,0);
//		ShowMessage("parse_frommap: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, command, RFIFOREST(fd));
		switch(command)
		{
		///////////////////////////////////////////////////////////////////////
		// map-server alive packet
		case 0x2718:
		{
			if (RFIFOREST(fd) < 2)
				return 0;
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// request from map-server to reload GM accounts. Transmission to login-server (by Yor)
//obsolete
		case 0x2af7:
		{
			if( session_isActive(login_fd) )
			{	// don't send request if no login-server
				WFIFOW(login_fd,0) = 0x2709;
				WFIFOSET(login_fd, 2);
//				ShowMessage("char : request from map-server to reload GM accounts -> login-server.\n");
			}
			RFIFOSKIP(fd,2);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// login as map-server; update ip addresses
		case 0x2af8: 
		{
			if(RFIFOREST(fd) < 70)
				return 0;
			server[id].address = basics::ipset(RFIFOLIP(fd,54), RFIFOLIP(fd,58), RFIFOW(fd,62),RFIFOLIP(fd,64), RFIFOW(fd,68));
			// send new ipset to mapservers for update
			unsigned char buf[16384];
			WBUFLIP(buf,4) = server[id].address.LANIP();
			WBUFLIP(buf,8) = server[id].address.LANMask();
			WBUFW(buf,12) = server[id].address.LANPort();
			WBUFLIP(buf,14) = server[id].address.WANIP();
			WBUFW(buf,18) = server[id].address.WANPort();
		
			for(i=0, j=0; i<MAX_MAP_PER_SERVER; ++i)
				if(server[id].map[i][0])
					memcpy(WBUFP(buf,20+(j++)*16), server[id].map[i], 16);
			if (j > 0) {
				WBUFW(buf,0) = 0x2b04;
				WBUFW(buf,2) = j*16+20;
				mapif_sendallwos(fd, buf, j*16+20);
			}

			ShowStatus("Map-Server %d updated: %s (%d maps)\n",
				id, server[id].address.tostring(NULL), j);

			RFIFOSKIP(fd,70);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Receiving map names list from the map-server
		case 0x2afa:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			memset(server[id].map, 0, sizeof(server[id].map));
			j = 0;
			for(i = 4; i < (size_t)RFIFOW(fd,2); i += 16) {
				memcpy(server[id].map[j], RFIFOP(fd,i), 16);
				j++;
			}
			server[id].maps = j;
			ShowStatus("Map-Server %d connected from %s (%d maps)\n",
				id, server[id].address.tostring(NULL), j);
			
			char_log("Map-Server %d connected from %s (%d maps)"RETCODE,
				id, server[id].address.tostring(NULL), j);

			WFIFOW(fd,0) = 0x2afb;
			WFIFOB(fd,2) = 0;
			memcpy(WFIFOP(fd,3), wisp_server_name, 24); // name for wisp to player
			WFIFOSET(fd,27);
			{
				unsigned char buf[16384];
				size_t x;
				if (j == 0) {
					ShowMessage("WARNING: Map-Server %d have NO map.\n", id);
					char_log("WARNING: Map-Server %d have NO map." RETCODE, id);
				// Transmitting maps information to the other map-servers
				} else {
					WBUFW(buf,0) = 0x2b04;
					WBUFW(buf,2) = j*16+20;
					WBUFLIP(buf,4) = server[id].address.LANIP();
					WBUFLIP(buf,8) = server[id].address.LANMask();
					WBUFW(buf,12) = server[id].address.LANPort();
					WBUFLIP(buf,14) = server[id].address.WANIP();
					WBUFW(buf,18) = server[id].address.WANPort();

					memcpy(WBUFP(buf,20), RFIFOP(fd,4), j*16);
					mapif_sendallwos(fd, buf, j*16+20);
				}
				// Transmitting the maps of the other map-servers to the new map-server
				for(x = 0; x < MAX_MAP_SERVERS; ++x) {
					if(server[x].fd >= 0 && x != id) {
						WFIFOW(fd,0) = 0x2b04;
						WFIFOLIP(fd,4) = server[x].address.LANIP();
						WFIFOLIP(fd,8) = server[x].address.LANMask();
						WFIFOW(fd,12) = server[x].address.LANPort();
						WFIFOLIP(fd,14) = server[x].address.WANIP();
						WFIFOW(fd,18) = server[x].address.WANPort();
						
						for(i=0, j=0; i<MAX_MAP_PER_SERVER; ++i)
							if (server[x].map[i][0])
								memcpy(WFIFOP(fd,20+(j++)*16), server[x].map[i], 16);
						if (j > 0) {
							WFIFOW(fd,2) = j*16+20;
							WFIFOSET(fd,j*16+20);
						}
					}
				}
				// send all fame lists
				for(x=0; x<4; x++)
				{	
					CFameList &fl = char_db.getfamelist(x);
					size_t len = 6+fl.tobuffer(WBUFP(buf,6));
					WBUFW(buf,0) = 0x2b1b;
					WBUFW(buf,4) = x;
					WBUFW(buf,2) = len;
					// send to all maps
					mapif_sendall(buf, len);
				}

				// send vars
				size_t cnt=var_db.size();
				for(x=0; x<cnt; ++x)
				{
					const CVar &var = var_db[x];
					const size_t sz = var.to_buffer(WFIFOP(fd,4), 0); //## len curently ignored
					WFIFOW(fd,0) = 0x2b31;
					WFIFOW(fd,2) = 4+sz;
					WFIFOSET(fd, 4+sz);
				}
				

			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// 認証要・
		// Send character data to map-server・
		case 0x2afc:
		{
			if (RFIFOREST(fd) < 22)
				return 0;

			CCharCharAccount account;
			CCharCharacter character;

			//ShowMessage("auth_fifo search: account: %ld, char: %ld, secure: %08lX-%08lX\n", (unsigned long)RFIFOL(fd,2), (unsigned long)RFIFOL(fd,6), (unsigned long)RFIFOL(fd,10), (unsigned long)RFIFOL(fd,14));

			// not yet combined in db
			if( char_db.searchAccount(RFIFOL(fd,2), account) &&
				char_db.searchChar(RFIFOL(fd,6), character) &&
				account.account_id == RFIFOL(fd,2) &&
				account.login_id1 == RFIFOL(fd,10) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				// here, it's the only area where it's possible that we doesn't know login_id2 (map-server asks just after 0x72 packet, that doesn't given the value)
				(RFIFOL(fd,14) == 0 || account.login_id2 == RFIFOL(fd,14) ) && // relate to the versions higher than 18
#endif
				(!check_ip_flag || account.client_ip == RFIFOLIP(fd,18)) )
			{
				WFIFOW(fd,0) = 0x2afd;
				WFIFOW(fd,2) = 16 + sizeof(struct mmo_charstatus);
				WFIFOL(fd,4) = RFIFOL(fd,6); // send char_id!!
				WFIFOL(fd,8) = account.login_id2;
				WFIFOL(fd,12) = (uint32)account.valid_until;
				character.sex = account.sex;
				character.gm_level = account.gm_level;

				mmo_charstatus_tobuffer(character, WFIFOP(fd,16));
				WFIFOSET(fd, 16 + sizeof(struct mmo_charstatus));
				//ShowMessage("auth_fifo search success (auth #%d, account %ld, character: %ld %i).\n", i, (unsigned long)RFIFOL(fd,2), (unsigned long)RFIFOL(fd,6),gm_level);
			}
			else
			{
				WFIFOW(fd,0) = 0x2afe;
				WFIFOL(fd,2) = RFIFOL(fd,2);
				WFIFOSET(fd,6);
				ShowMessage("auth_fifo search error! account %ld not authentified.\n", (unsigned long)RFIFOL(fd,2));
			}
			RFIFOSKIP(fd,22);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// MAPサーバー上のユーザー数受信
		// Recieve alive message from map-server
		case 0x2aff:
		{
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			size_t sz = RFIFOW(fd,2);
			
			server[id].users = RFIFOW(fd,4);
/*
//!! rewrite
			// remove all previously online players of the server
			for(i = 0; i < online_players_max; ++i)
				if (online_chars[i].server == id) {
					online_chars[i].char_id = 0xFFFFFFFF;
					online_chars[i].server  = 0xFFFFFFFF;
				}
			// add online players in the list by [Yor]
			j = 0;
			for(i = 0; i < server[id].users; ++i) {
				for(; j < online_players_max; ++j)
					if(online_chars[j].char_id == 0xFFFFFFFF) {
						online_chars[j].char_id = RFIFOL(fd,6+i*4);
						online_chars[j].server = id;
						//ShowMessage("%d\n", online_chars[j].char_id);
						break;
					}
				// no available slots...
				if (j == online_players_max) {
					// create 256 new slots
					online_players_max += 256;
					online_chars = (struct online_chars*)aRealloc(online_chars, online_players_max*sizeof(struct online_chars) );
					for( ; j < online_players_max; ++j) {
						online_chars[j].char_id = 0xFFFFFFFF;
						online_chars[j].server  = 0xFFFFFFFF;
					}
					// save data
					j = online_players_max - 256;
					online_chars[j].char_id = RFIFOL(fd,6+i*4);
					online_chars[j].server = id;
				}
			}
			if (update_online < time(NULL)) { // Time is done
				update_online = time(NULL) + 8;
				create_online_files(); // only every 8 sec. (normally, 1 server send users every 5 sec.) Don't update every time, because that takes time, but only every 2 connection.
				                       // it set to 8 sec because is more than 5 (sec) and if we have more than 1 map-server, informations can be received in shifted.
			}
*/
			RFIFOSKIP(fd,sz);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// キャラデータ保存
		// Recieve character data from map-server
		case 0x2b01:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			CCharCharacter character;
			mmo_charstatus_frombuffer(character, RFIFOP(fd,12));
			char_db.saveChar(character);
			
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// キャラセレ要求
// might be obsolete, authentification will be considered up-down, not down-up
		case 0x2b02:
		{
			if (RFIFOREST(fd) < 18)
				return 0;
			CCharCharAccount account;

			//ShowMessage("auth_fifo set (auth #%d) - account: %d, secure: %08x-%08x\n", auth_fifo_pos, (uint32)RFIFOL(fd,2), (uint32)RFIFOL(fd,6), (uint32)RFIFOL(fd,10));
			if( char_db.searchAccount( RFIFOL(fd,2), account) )
			{
				account.account_id = RFIFOL(fd,2);
				account.login_id1 = RFIFOL(fd,6);
				account.login_id2 = RFIFOL(fd,10);
				account.client_ip = RFIFOLIP(fd,14);
			}
			else
			{
				account.account_id = RFIFOL(fd,2);
				account.login_id1 = RFIFOL(fd,6);
				account.login_id2 = RFIFOL(fd,10);
				account.client_ip = RFIFOLIP(fd,14);
			}
			char_db.saveAccount(account);

			WFIFOW(fd,0) = 0x2b03;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			WFIFOB(fd,6) = 0;
			WFIFOSET(fd,7);

			RFIFOSKIP(fd,18);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// マップサーバー間移動要求
// might be obsolete, authentification will be considered up-down, not down-up
		case 0x2b05:
		{
			if (RFIFOREST(fd) < 49)
				return 0;

			CCharCharAccount account;
			//ShowMessage("auth_fifo set (auth#%d) - account: %d, secure: 0x%08x-0x%08x\n", auth_fifo_pos, (uint32)RFIFOL(fd,2), (uint32)RFIFOL(fd,6), (uint32)RFIFOL(fd,10));

			if( char_db.searchAccount( RFIFOL(fd,2), account) )
			{
				account.account_id = RFIFOL(fd,2);
				//auth_fifo[auth_fifo_pos].char_id = RFIFOL(fd,14);
				// not used here
				account.login_id1 = RFIFOL(fd,6);
				account.login_id2 = RFIFOL(fd,10);
				account.sex = RFIFOB(fd,44);
				account.valid_until = 0; // unlimited/unknown time by default (not display in map-server)
				account.client_ip = RFIFOLIP(fd,45);
			}
			else
			{
				account.account_id = RFIFOL(fd,2);
				account.login_id1 = RFIFOL(fd,6);
				account.login_id2 = RFIFOL(fd,10);
				account.client_ip = RFIFOLIP(fd,14);
			}
			char_db.saveAccount(account);

			WFIFOW(fd,0) = 0x2b06;
			memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 42);

			CCharCharacter character;
			if( char_db.searchChar(RFIFOL(fd,14),character) &&
				character.account_id == RFIFOL(fd,2) )
				WFIFOL(fd,6) = 0;
			else
				WFIFOL(fd,6) = 1;

			WFIFOSET(fd,44);

			RFIFOSKIP(fd,49);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// キャラ名検索
		case 0x2b08:
		{
			if (RFIFOREST(fd) < 6)
				return 0;
			CCharCharacter character;

			WFIFOW(fd,0) = 0x2b09;
			WFIFOL(fd,2) = RFIFOL(fd,2);

			if( char_db.searchChar(RFIFOL(fd,2),character) )
				memcpy(WFIFOP(fd,6), character.name, 24);
			else
				memcpy(WFIFOP(fd,6), unknown_char_name, 24);
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// it is a request to become GM
// obsolete, integrate to 0x2750
		case 0x2b0a:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
//			ShowMessage("parse_frommap: change gm -> login, account: %d, pass: '%s'.\n", RFIFOL(fd,4), RFIFOP(fd,8));
			if( session_isActive(login_fd) )
			{	// don't send request if no login-server
				WFIFOW(login_fd,0) = 0x2720;
				memcpy(WFIFOP(login_fd,2), RFIFOP(fd,2), RFIFOW(fd,2)-2);
				WFIFOSET(login_fd, RFIFOW(fd,2));
			}
			else
			{
				WFIFOW(fd,0) = 0x2b0b;
				WFIFOL(fd,2) = RFIFOL(fd,4);
				WFIFOL(fd,6) = 0;
				WFIFOSET(fd, 10);
			}
			RFIFOSKIP(fd, RFIFOW(fd,2));
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Map server send information to change an email of an account -> login-server
// obsolete, integrate to 0x2750
		case 0x2b0c:
			if (RFIFOREST(fd) < 86)
				return 0;
			if( session_isActive(login_fd) )
			{	// don't send request if no login-server
				 // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), 86);
				WFIFOW(login_fd,0) = 0x2722;
				WFIFOSET(login_fd, 86);
			}
			RFIFOSKIP(fd, 86);
			break;
		///////////////////////////////////////////////////////////////////////
		// Map server ask char-server about a character name to do some operations (all operations are transmitted to login-server)
// obsolete, integrate to 0x2750
		case 0x2b0e:
		{
			if (RFIFOREST(fd) < 44)
				return 0;

			uint32 accid = RFIFOL(fd,2); // account_id of who ask (-1 if nobody)
			char *character_name = (char *)RFIFOP(fd,6);
			character_name[24] = '\0';
			
			// prepare answer
			WFIFOW(fd,0) = 0x2b0f; // answer
			WFIFOL(fd,2) = accid; // who want do operation
			WFIFOW(fd,30) = RFIFOW(fd, 30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban, 5-changesex

			// search character
			CCharCharacter character;
			if( !char_db.searchChar(character_name, character) )
			{
				memcpy(WFIFOP(fd,6), character_name, 24);
				WFIFOW(fd,32) = 1; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
			}
			else
			{
				memcpy(WFIFOP(fd,6), character.name, 24); // put correct name if found
				WFIFOW(fd,32) = 0; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline


				CCharCharAccount askingaccount, targetaccount;
				if( !char_db.searchAccount(accid, askingaccount) )
					askingaccount.gm_level=0;
				if( !char_db.searchAccount(character.account_id, targetaccount ) )
					targetaccount.gm_level=0;
			
				if( accid != 0xFFFFFFFF && askingaccount.gm_level < targetaccount.gm_level )
				{
					WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
				}
				else if( !session_isActive(login_fd) )
				{
					WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
				}
				else
				{

					switch(RFIFOW(fd, 30))
					{
					case 1: // block
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = character.account_id; // account value
						WFIFOL(login_fd,6) = 5; // status of the account
						WFIFOSET(login_fd, 10);
//						ShowMessage("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 5);
						break;
					case 2: // ban
						WFIFOW(login_fd, 0) = 0x2725;
						WFIFOL(login_fd, 2) = character.account_id; // account value
						WFIFOW(login_fd, 6) = RFIFOW(fd,32); // year
						WFIFOW(login_fd, 8) = RFIFOW(fd,34); // month
						WFIFOW(login_fd,10) = RFIFOW(fd,36); // day
						WFIFOW(login_fd,12) = RFIFOW(fd,38); // hour
						WFIFOW(login_fd,14) = RFIFOW(fd,40); // minute
						WFIFOW(login_fd,16) = RFIFOW(fd,42); // second
						WFIFOSET(login_fd,18);
//						ShowMessage("char : status -> login: account %d, ban: %dy %dm %dd %dh %dmn %ds\n",
//						       char_dat[i].account_id, (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), (short)RFIFOW(fd,38), (short)RFIFOW(fd,40), (short)RFIFOW(fd,42));
						break;
					case 3: // unblock
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = character.account_id; // account value
						WFIFOL(login_fd,6) = 0; // status of the account
						WFIFOSET(login_fd, 10);
//						ShowMessage("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 0);
						break;
					case 4: // unban
						WFIFOW(login_fd, 0) = 0x272a;
						WFIFOL(login_fd, 2) = character.account_id; // account value
						WFIFOSET(login_fd, 6);
//						ShowMessage("char : status -> login: account %d, unban request\n", char_dat[i].account_id);
						break;
					case 5: // changesex
						WFIFOW(login_fd, 0) = 0x2727;
						WFIFOL(login_fd, 2) = character.account_id; // account value
						WFIFOSET(login_fd, 6);
//						ShowMessage("char : status -> login: account %d, change sex request\n", char_dat[i].account_id);
						break;
					}// end switch
				}
			}
			// send answer if a player ask, not if the server ask
			if(accid != 0xFFFFFFFF)
			{
				WFIFOSET(fd, 34);
			}
			RFIFOSKIP(fd, 44);
			break;
		}
		///////////////////////////////////////////////////////////////////////
//		case 0x2b0f: not more used (available for futur usage)

		///////////////////////////////////////////////////////////////////////
		// account_reg保存要求
// obsolete, integrate to 0x2750
		case 0x2b10:
		{
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			size_t j, p, sz=RFIFOW(fd,2);
			uint32 accid = RFIFOL(fd,4);
			CCharCharAccount account;
			if( char_db.searchAccount(accid,account) )
			{
				for(p = 8, j = 0; p < sz && j < ACCOUNT_REG2_NUM; p += 36, ++j)
				{
					memcpy(account.account_reg2[j].str, RFIFOP(fd,p), 32);
					account.account_reg2[j].value = RFIFOL(fd,p+32);
				}
				char_db.saveAccount(account);
			}
			// loginサーバーへ送る
			if( session_isActive(login_fd) )
			{	// don't send request if no login-server

				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), sz);
				WFIFOW(login_fd, 0) = 0x2728;
				WFIFOSET(login_fd, sz);
			}
			// ワールドへの同垢ログインがなければmapサーバーに送る必要はない
			RFIFOW(fd,0) = 0x2b11;
			mapif_sendall(RFIFOP(fd,0), sz);
			RFIFOSKIP(fd, sz);
//			ShowMessage("char: save_account_reg (from map)\n");
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// Recieve rates [Wizputer]
// rethink those generally, might be obsolete
		case 0x2b16:
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,8))
				return 0;
			// Txt doesn't need this packet, so just skip it
			RFIFOSKIP(fd,RFIFOW(fd,8));
			break;

		// Character disconnected set online 0 [Wizputer]
		case 0x2b17:
			if (RFIFOREST(fd) < 6)
				return 0;
			//ShowMessage("Setting %d char offline\n",RFIFOL(fd,2));
			set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
			break;

		// Reset all chars to offline [Wizputer]
		case 0x2b18:
		    set_all_offline();
			RFIFOSKIP(fd,2);
			break;

		// Character set online [Wizputer]
		case 0x2b19:
			if (RFIFOREST(fd) < 6)
				return 0;
			//ShowMessage("Setting %d char online\n",RFIFOL(fd,2));
			set_char_online(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
			break;
		///////////////////////////////////////////////////////////////////////
		// Request for updating the fame list
		case 0x2b1a:
		{
			if (RFIFOREST(fd) < 36)
				return 0;

			const int type = RFIFOW(fd,2);
			const uint32 charid = RFIFOL(fd,4);
			const char* name    = (const char*)RFIFOP(fd,8);
			const uint32 points = RFIFOL(fd,32);
			CFameList& fl = char_db.getfamelist(type);

			if( fl.update(charid, name, points) )
			{	// has changed so we send it
				unsigned char buf[32+sizeof(CFameList)];
				size_t len = 6+fl.tobuffer(WBUFP(buf,6));
				WBUFW(buf,0) = 0x2b1b;
				WBUFW(buf,4) = type;
				WBUFW(buf,2) = len;
				// sending to all maps
				mapif_sendall(buf, len);
			}
			RFIFOSKIP(fd,36);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// status changes
		// for testing purpose
		case 0x2b22:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;

			RFIFOSKIP(fd,sz);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// mail system
		// check
		case 0x2b23:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;
			uint32 charid = RFIFOL(fd,4);
			uchar showall = RFIFOB(fd,8);
			uint32 all, unread;
			char_db.getMailCount(charid, all, unread);	// check unread mail count

			WFIFOW(fd,0) = 0x2b23;
			WFIFOW(fd,2) = 17;
			WFIFOL(fd,4) = charid;
			WFIFOL(fd,8) = all;
			WFIFOL(fd,12) = unread;
			WFIFOL(fd,16) = showall;
			WFIFOSET(fd, 17);

			RFIFOSKIP(fd,sz);
			break;
		}
		// fetch
		case 0x2b24:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;

			CMailHead dummy;
			uint32 charid = RFIFOL(fd,4);
			unsigned char box = RFIFOB(fd, 8);
			uint32 count  = char_db.listMail(charid, box, WFIFOP(fd,12));
			WFIFOW(fd,0) = 0x2b24;
			WFIFOW(fd,2) = 12 + count*dummy.size();
			WFIFOL(fd,4) = charid;
			WFIFOL(fd,8) = count;
			WFIFOSET(fd, 12 + count*dummy.size());

			RFIFOSKIP(fd,sz);
			break;
		}
		// read/getappend
		case 0x2b25:
		case 0x2b28:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;
			uint32 charid = RFIFOL(fd,4);
			uint32 msgid  = RFIFOL(fd,8);
			CMail mail;
			char_db.readMail(charid, msgid, mail);
			// send wit same command
			WFIFOW(fd, 0) = command;//0x2b25;
			WFIFOW(fd, 2) = 8+mail.size();
			WFIFOL(fd, 4) = charid;
			mail.tobuffer( WFIFOP(fd, 8) );
			WFIFOSET(fd, 8+mail.size());

			RFIFOSKIP(fd,sz);
			break;
		}
		// delete
		case 0x2b26:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;
			uint32 charid = RFIFOL(fd,4);
			uint32 msgid  = RFIFOL(fd,8);
			uchar  ok     = char_db.deleteMail(charid, msgid);

			WFIFOW(fd, 0) = 0x2b26;
			WFIFOW(fd, 2) = 13;
			WFIFOL(fd, 4) = charid;
			WFIFOL(fd, 8) = msgid;
			WFIFOB(fd,12) = ok;
			WFIFOSET(fd, 13);

			RFIFOSKIP(fd,sz);
			break;
		}
		// send
		case 0x2b27:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;
			bool ok = false;
			uint32 tid, msgid=0;
			uint32 senderid = RFIFOL(fd,4);
			char* sender  = (char*)RFIFOP(fd,8);
			char* target  = (char*)RFIFOP(fd,32);
			char* head    = (char*)RFIFOP(fd,56);
			char* body    = (char*)RFIFOP(fd,96);
			uint32 zeny   = RFIFOL(fd,608);
			struct item it;

			item_frombuffer(it, RFIFOP(fd, 612));

			// store mail
			ok = char_db.sendMail(senderid, sender, target, head, body, zeny, it, msgid, tid);
			if(ok)
			{	// send "you have new mail" to addressee
				unsigned char buf[16];

				WBUFW(buf,0) = 0x2b23;
				WBUFW(buf,2) = 12;
				WBUFL(buf,4) = tid;
				WBUFL(buf,8) = 1;
				mapif_sendallwos(-1, buf, 12);
			}
			// send answer back to sender
			WFIFOW(fd,0) = 0x2b27;
			WFIFOW(fd,2) = 13;
			WFIFOL(fd,4) = senderid;
			WFIFOL(fd,8) = msgid;
			WFIFOB(fd,12) = ok;
			WFIFOSET(fd, 13);

			RFIFOSKIP(fd,sz);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// variable system
		// req variable in/out
		case 0x2b30:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;

			// nothing here right now

			RFIFOSKIP(fd,sz);
			break;
		}
		// save variable in/out
		case 0x2b31:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;

			const char*n =(const char*)RFIFOP(fd,4);
			const char*v =(const char*)RFIFOP(fd,36); 
			// putting Macros into indirect conversions results in problems on M$
			CVar var( n, v );
			if( var_db.saveVar(var) )
			{
				// and send it down to all other maps for map/party/guild vars
				mapif_sendallwos(fd, RFIFOP(fd,0), sz);
				// do not send down for account/char vars
			}

			RFIFOSKIP(fd,sz);
			break;
		}

		///////////////////////////////////////////////////////////////////////
		// irc system
		// announce
		case 0x2b38:
		{	size_t sz;
			if (RFIFOREST(fd) < 4 || (size_t)RFIFOREST(fd) < (sz=RFIFOW(fd,2)))
				return 0;
			irc_announce(fd);
			break;
		}
		///////////////////////////////////////////////////////////////////////
//!! reorder for proper returns or just integrate
		default:
			// inter server処理に渡す
			{
				int r = inter_parse_frommap(fd);
				if (r == 1) // 処理できた
					break;
				if (r == 2) // パケット長が足りない
					return 0;
			}
			// inter server処理でもない場合は切断
			ShowMessage("char: unknown packet 0x%04x (%d bytes to read in buffer)! (from map).\n", (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
			session_Remove(fd);
			server[id].fd = -1;
			return 0;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int search_mapserver(const char *mapname)
{
	size_t i, j;
	char temp_map[16];
	size_t temp_map_len;

	if( !mapname )
		return -1;

//	ShowMessage("Searching the map-server for map '%s'... ", map);
	temp_map_len = 1+strlen(mapname);
	if( temp_map_len>sizeof(temp_map) ) temp_map_len = sizeof(temp_map);
	memcpy(temp_map, mapname, temp_map_len);
	temp_map[sizeof(temp_map)-1] = '\0';
	if (strchr(temp_map, '.') != NULL)
		temp_map[strchr(temp_map, '.') - temp_map + 1] = '\0'; // suppress the '.gat', but conserve the '.' to be sure of the name of the map

	temp_map_len = strlen(temp_map);
	for(i = 0; i < MAX_MAP_SERVERS; ++i)
	{
		if( session_isActive( server[i].fd ) )
		{
			for (j = 0; server[i].map[j][0]; ++j)
			{
				//ShowMessage("%s : %s = %d\n", server[i].map[j], map, strncmp(server[i].map[j], temp_map, temp_map_len));
				if(strncmp(server[i].map[j], temp_map, temp_map_len) == 0)
				{
//					ShowMessage("found -> server #%d.\n", i);
					return i;
				}
			}
		}
	}
//	ShowMessage("not found.\n");
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
// char_mapifの初期化処理（現在はinter_mapif初期化のみ）
int char_mapif_init(int fd) {
	return inter_mapif_init(fd);
}
///////////////////////////////////////////////////////////////////////////////
int parse_char(int fd)
{
	if( !session_isActive(fd) ) 
	{	// is disconnecting
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}
	if( !session_isActive(login_fd) )
	{	// no login server available, reject connection
		session_Remove(fd);
		return 0;
	}

	unsigned short cmd;
	uint32 client_ip = session[fd]->client_ip;
	char_session_data *sd = (char_session_data *)session[fd]->user_session;

	while (RFIFOREST(fd) >= 2)
	{
		cmd = RFIFOW(fd,0);
		// crc32のスキップ用
		if(	sd==NULL			&&	// 未ログインor管理パケット
			RFIFOREST(fd)>=4	&&	// 最低バイト数制限 ＆ 0x7530,0x7532管理パケ除去
			RFIFOREST(fd)<=21	&&	// 最大バイト数制限 ＆ サーバーログイン除去
			cmd!=0x20b	&&	// md5通知パケット除去
			(RFIFOREST(fd)<6 || RFIFOW(fd,4)==0x65)	)
		{	// 次に何かパケットが来てるなら、接続でないとだめ
			RFIFOSKIP(fd,4);
			cmd = RFIFOW(fd,0);
			ShowMessage("parse_char : %d crc32 skipped\n",fd);
			if(RFIFOREST(fd)==0)
				return 0;
		}

//		if(cmd<30000 && cmd!=0x187)
//			ShowMessage("parse_char : %d %d %d\n",fd,RFIFOREST(fd),cmd);

		// 不正パケットの処理
//		if (sd == NULL && cmd != 0x65 && cmd != 0x20b && cmd != 0x187 &&
//					 cmd != 0x2af8 && cmd != 0x7530 && cmd != 0x7532)
//			cmd = 0xffff;	// パケットダンプを表示させる

		switch(cmd)
		{
		///////////////////////////////////////////////////////////////////////
		//20040622暗号化ragexe対応
		case 0x20b:	
		{
			if (RFIFOREST(fd) < 19)
				return 0;
			RFIFOSKIP(fd,19);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// 接続要求
		case 0x65:
		{
			if (RFIFOREST(fd) < 17)
				return 0;
			if (sd == NULL)
			{
				session[fd]->user_session = sd = new char_session_data;
				safestrcpy(sd->email, sizeof(sd->email), "no mail"); // put here a mail without '@' to refuse deletion if we don't receive the e-mail
				sd->valid_until = 0; // unknow or unlimited (not displaying on map-server)
			}
			// send back account_id
			WFIFOL(fd,0) = RFIFOL(fd,2);
			WFIFOSET(fd,4);

			// search authentification, does also fill the char_session_data
			if( char_db.searchAccount(RFIFOL(fd,2), *sd) &&
				client_ip     == sd->client_ip &&
				RFIFOL(fd,2)  == sd->account_id &&
				RFIFOL(fd,6)  == sd->login_id1 &&
				RFIFOL(fd,10) == sd->login_id2 && 
				RFIFOB(fd,16) == sd->sex ) 
			{	// send characters to player

				if(max_connect_user == 0 || count_users() < max_connect_user || sd->gm_level >= gm_allow_level)
				{
					// make a local copy of the whole account data
					if( sd->gm_level )
						ShowInfo("Account Logged On; Account ID: %ld (GM level %d).\n", (unsigned long)sd->account_id, sd->gm_level);
					else
						ShowInfo("Account Logged On; Account ID: %ld.\n", (unsigned long)sd->account_id);

					// send characters to player
					mmo_char_send006b(fd, *sd);
				}
				else
				{
					// refuse connection (over populated)
					WFIFOW(fd,0) = 0x6c;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
			else
			{
				if( session_isActive(login_fd) )
				{	// store data in the session data
					sd->account_id= RFIFOL(fd,2);
					sd->login_id1 = RFIFOL(fd,6);
					sd->login_id2 = RFIFOL(fd,10);
					sd->sex       = RFIFOB(fd,16);
					sd->client_ip = client_ip;
					
					// ask login-server to authentify an account
					WFIFOW(login_fd,0) = 0x2712; 
					WFIFOL(login_fd,2) = sd->account_id;
					WFIFOL(login_fd,6) = sd->login_id1;
					WFIFOL(login_fd,10) = sd->login_id2; // relate to the versions higher than 18
					WFIFOB(login_fd,14) = sd->sex;
					WFIFOLIP(login_fd,15) = client_ip;
					WFIFOSET(login_fd,19);
				}
				else
				{	// if no login-server, we must refuse connection
					WFIFOW(fd,0) = 0x6c;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}		
			RFIFOSKIP(fd,17);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// キャラ選択
		case 0x66:
		{
			if (RFIFOREST(fd) < 3)
				return 0;
			if(sd)
			{	size_t slot;
				// if we activated email creation and email is default email
				if(email_creation && 0==strcmp(sd->email, "a@a.com") && session_isActive(login_fd) )
				{	
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd, 3);
				}
				else if( (slot=RFIFOB(fd,2)) < 9 )
				{	// otherwise, load the character
					CCharCharAccount account;
					CCharCharacter character;
					if( char_db.searchAccount(sd->account_id, account) &&
						account.charlist[slot] != 0 &&
						char_db.searchChar(account.charlist[slot], character) )
					{
						char_log("Character Selected, Account ID: %d, Character Slot: %d, Character ID: %ld, Name: %s." RETCODE,
							(unsigned long)sd->account_id, slot, (unsigned long)character.char_id, character.name);
						// searching map server
						int j = search_mapserver(character.last_point.mapname);
						// if map is not found, we check major cities
						if(j < 0)
						{
							if((j = search_mapserver("prontera")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "prontera");
								character.last_point.x = 273; // savepoint coordonates
								character.last_point.y = 354;
							} else if((j = search_mapserver("geffen")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "geffen");
								character.last_point.x = 120; // savepoint coordonates
								character.last_point.y = 100;
							} else if((j = search_mapserver("morocc")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "morocc");
								character.last_point.x = 160; // savepoint coordonates
								character.last_point.y = 94;
							} else if((j = search_mapserver("alberta")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "alberta");
								character.last_point.x = 116; // savepoint coordonates
								character.last_point.y = 57;
							} else if((j = search_mapserver("payon")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "payon");
								character.last_point.x = 87; // savepoint coordonates
								character.last_point.y = 117;
							} else if((j = search_mapserver("izlude")) >= 0) { // check is done without 'gat'.
								safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), "izlude");
								character.last_point.x = 94; // savepoint coordonates
								character.last_point.y = 103;
							} else {
								// get first online server (with a map)
								for(j = 0; j < MAX_MAP_SERVERS; ++j)
								{
									if( session_isActive(server[j].fd) && server[j].map[0][0])
									{	// change save point to one of map found on the server (the first)
										safestrcpy(character.last_point.mapname, sizeof(character.last_point.mapname), server[j].map[0]);
										ShowMessage("Map-server #%d found with a map: '%s'.\n", j, server[j].map[0]);
										// coordonates are unknown
										break;
									}
								}
								// if no map-server is connected, we send: server closed
								if(j >= MAX_MAP_SERVERS)
								{
									WFIFOW(fd,0) = 0x81;
									WFIFOL(fd,2) = 1; // 01 = Server closed
									WFIFOSET(fd,3);
									RFIFOSKIP(fd,3);
									break;
								}
							}
						}

						WFIFOW(fd,0) = 0x71;
						WFIFOL(fd,2) = character.char_id;
						mapname2buffer(WFIFOP(fd,6), 16, character.last_point.mapname);


						ShowMessage("Character selection '%s' (account: %ld, charid: %ld, slot: %d).\n", 
							character.name, (unsigned long)character.account_id, (unsigned long)character.char_id, slot);
						
						if( server[j].address.isLAN(client_ip) )
						{
							ShowMessage("Send IP of map-server: %s:%d (%s)\n", server[j].address.LANIP().tostring(NULL), server[j].address.LANPort(), CL_BT_GREEN"LAN"CL_NORM);
							WFIFOLIP(fd, 22) = server[j].address.LANIP();
							WFIFOW(fd,26)    = server[j].address.LANPort();
						}
						else
						{
							ShowMessage("Send IP of map-server: %s:%d (%s)\n", server[j].address.WANIP().tostring(NULL), server[j].address.WANPort(), CL_BT_CYAN"WAN"CL_NORM);
							WFIFOLIP(fd, 22) = server[j].address.WANIP();
							WFIFOW(fd,26)    = server[j].address.WANPort();
						}
						WFIFOSET(fd,28);
					}
				}
			}
			RFIFOSKIP(fd,3);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// 作成
		case 0x67:
		{
			if (RFIFOREST(fd) < 37)
				return 0;
				
			if(sd)
			{	
				CCharCharacter character;
				char* name = (char*)RFIFOP(fd,2);
				const unsigned char str	= RFIFOB(fd,26);
				const unsigned char agi = RFIFOB(fd,27);
				const unsigned char vit = RFIFOB(fd,28);
				const unsigned char int_= RFIFOB(fd,29);
				const unsigned char dex = RFIFOB(fd,30);
				const unsigned char luk = RFIFOB(fd,31);
				const unsigned char slot= RFIFOB(fd,32);
				const unsigned char hair_style = RFIFOB(fd,35);
				const unsigned char hair_color = RFIFOB(fd,33);

				if( !char_db.testChar( *sd, name, str,agi,vit,int_,dex,luk,slot,hair_style,hair_color) )
				{
					//denied
					WFIFOW(fd, 0) = 0x6e;
					WFIFOB(fd, 2) = 0x02;
					WFIFOSET(fd, 3); 
				}
				else if( !char_db.insertChar( *sd, name, str,agi,vit,int_,dex,luk,slot,hair_style,hair_color, character) )
				{	//already exists
					WFIFOW(fd, 0) = 0x6e;
					WFIFOB(fd, 2) = 0x00;
					WFIFOSET(fd, 3);
					/*
					useless error message
					else if(ret == -3)
					{	//underaged XD
						WFIFOW(fd, 0) = 0x6e;
						WFIFOB(fd, 2) = 0x01;
						WFIFOSET(fd, 3);
					}
					*/
				}
				else
				{	// ok
					WFIFOW(fd,0) = 0x6d;
					memset(WFIFOP(fd,2), 0, 106);

					WFIFOL(fd,2) = character.char_id;
					WFIFOL(fd,2+4) = character.base_exp;
					WFIFOL(fd,2+8) = character.zeny;
					WFIFOL(fd,2+12) = character.job_exp;
					WFIFOL(fd,2+16) = character.job_level;

					WFIFOL(fd,2+28) = character.karma;
					WFIFOL(fd,2+32) = character.manner;

					WFIFOW(fd,2+40) = 0x30;
					WFIFOW(fd,2+42) = (unsigned short)((character.hp > 0x7fff) ? 0x7fff : character.hp);
					WFIFOW(fd,2+44) = (unsigned short)((character.max_hp > 0x7fff) ? 0x7fff : character.max_hp);
					WFIFOW(fd,2+46) = (unsigned short)((character.sp > 0x7fff) ? 0x7fff : character.sp);
					WFIFOW(fd,2+48) = (unsigned short)((character.max_sp > 0x7fff) ? 0x7fff : character.max_sp);
					WFIFOW(fd,2+50) = DEFAULT_WALK_SPEED; // char_dat[i].speed;
					WFIFOW(fd,2+52) = character.class_;
					WFIFOW(fd,2+54) = character.hair;

					WFIFOW(fd,2+58) = character.base_level;
					WFIFOW(fd,2+60) = character.skill_point;

					WFIFOW(fd,2+64) = character.shield;
					WFIFOW(fd,2+66) = character.head_top;
					WFIFOW(fd,2+68) = character.head_mid;
					WFIFOW(fd,2+70) = character.hair_color;

					memcpy(WFIFOP(fd,2+74), character.name, 24);

					WFIFOB(fd,2+98) = (character.str > 255) ? 255 : character.str;
					WFIFOB(fd,2+99) = (character.agi > 255) ? 255 : character.agi;
					WFIFOB(fd,2+100) = (character.vit > 255) ? 255 : character.vit;
					WFIFOB(fd,2+101) = (character.int_ > 255) ? 255 : character.int_;
					WFIFOB(fd,2+102) = (character.dex > 255) ? 255 : character.dex;
					WFIFOB(fd,2+103) = (character.luk > 255) ? 255 : character.luk;
					WFIFOB(fd,2+104) = character.slot;

					WFIFOSET(fd,108);
				}
			}
			// else there is no sd
			RFIFOSKIP(fd,37);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// delete char
		case 0x68:	
		{
			if (RFIFOREST(fd) < 46)
				return 0;

			char* email = (char*)RFIFOP(fd,6);
			email[39]=0;
			if( !email_check(email) )
				safestrcpy(email, 40, "a@a.com"); // default e-mail if incorrect
			
			if( email_creation && session_isActive(login_fd) &&
				0!=strcmp(email, "a@a.com") && 0==strcmp(sd->email, "a@a.com") )
			{	// enable changeing the email when email creation is activated
				// and a non-default email comes in
				
				// we save new e-mail
				safestrcpy(sd->email, sizeof(sd->email), email);

				// send changed authentification to login
				WFIFOW(fd,0) = 0x2750;
				sd->CCharAccount::tobuffer( WFIFOP(fd, 2) );
				WFIFOSET(fd, 2+sd->CCharAccount::size() );


				// modify packet to simulate a char selection
				CCharCharacter character;
				if( char_db.searchChar(RFIFOL(fd,2),character) )
				{
					//////////////////
					//## attention, two unusual schemes at once here

					// skip part of the packet! (46, but leave the size of select packet: 3)
					RFIFOSKIP(fd,43);
					// change value to put new packet (char selection)
					RFIFOW(fd, 0) = 0x66;
					RFIFOB(fd, 2) = character.slot;
					// not skip packet, it's modify of actual packet
					break;

					//## attention, two unusual schemes at once here
					//////////////////
				}
				else
				{
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd, 3);
					char_log("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - e-mail is invalid (account: %d, ip: %s)" RETCODE, server_name, sd->account_id, basics::ipaddress(client_ip).tostring(NULL));
				}
			}
			// otherwise, we delete the character
			else if( email && sd->email && 0==strcasecmp(email, sd->email) &&
					char_delete(RFIFOL(fd,2)) )
			{
				WFIFOW(fd,0) = 0x6f;
				WFIFOSET(fd,2);
			}
			else
			{	// semething went wrong / most possibly an invalid email
				WFIFOW(fd, 0) = 0x70;
				WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd, 3);
			}
			RFIFOSKIP(fd,46);
			break;
		}
		///////////////////////////////////////////////////////////////////////
		// マップサーバーログイン
		case 0x2af8:
		{
			size_t i;
			if (RFIFOREST(fd) < 70)
				return 0;
			WFIFOW(fd,0) = 0x2af9;
			for(i = 0; i < MAX_MAP_SERVERS; ++i) {
				if(server[i].fd < 0)
					break;
			}
			if (i == MAX_MAP_SERVERS || strcmp((char*)RFIFOP(fd,2), userid) || strcmp((char*)RFIFOP(fd,26), passwd)){
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			} else {
				WFIFOB(fd,2) = 0;
				session[fd]->func_parse = parse_frommap;

				server[i].address = basics::ipset(RFIFOLIP(fd,54), RFIFOLIP(fd,58), RFIFOW(fd,62),RFIFOLIP(fd,64), RFIFOW(fd,68));
				server[i].fd    = fd;
				server[i].users = 0;
				memset(server[i].map, 0, sizeof(server[i].map));

				WFIFOSET(fd,3);
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
				char_mapif_init(fd);
			}
			RFIFOSKIP(fd,70);
			return parse_frommap(fd);
		}
		///////////////////////////////////////////////////////////////////////
		case 0x187:	// Alive信号？
			if (RFIFOREST(fd) < 6)
				return 0;
			RFIFOSKIP(fd, 6);
			break;
		///////////////////////////////////////////////////////////////////////
		// Athena情報所得
		case 0x7530:
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
			return 0;
		///////////////////////////////////////////////////////////////////////
		// 接続の切断(defaultと処理は一緒だが明示的にするため)
		case 0x7532:
			session_Remove(fd);
			return 0;
		///////////////////////////////////////////////////////////////////////
		default:
			session_Remove(fd);
			return 0;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// Console Command Parser [Wizputer]
int parse_console(const char *buf)
{
    char type[64]="", command[64]="";

    ShowMessage("Console: %s\n",buf);

    if( sscanf(buf, "%64[^:]:%64[^\n]", type , command ) < 2 )
        sscanf(buf,"%64[^\n]",type);

    ShowMessage("Type of command: %s || Command: %s \n",type,command);

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
// 全てのMAPサーバーにデータ送信（送信したmap鯖の数を返す）
int mapif_sendall(unsigned char *buf, unsigned int len) {
	int i, c=0;
	int fd;

	if(buf)
	for(i = 0; i < MAX_MAP_SERVERS; ++i)
	{
		fd = server[i].fd;
		if( session_isActive(fd) )
		{
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}
	return c;
}
///////////////////////////////////////////////////////////////////////////////
// 自分以外の全てのMAPサーバーにデータ送信（送信したmap鯖の数を返す）
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c=0;
	int fd;

	if(buf)
	for(i = 0; i < MAX_MAP_SERVERS; ++i)
	{
		fd = server[i].fd;
		if( session_isActive(fd) && fd != sfd )
		{
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}
///////////////////////////////////////////////////////////////////////////////
// MAPサーバーにデータ送信（map鯖生存確認有り）
int mapif_send(int fd, unsigned char *buf, unsigned int len) {
	int i;
	if( buf && session_isActive(fd) )
	{
		for(i = 0; i < MAX_MAP_SERVERS; ++i)
		{
			if(fd == server[i].fd)
			{
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int send_users_tologin(int tid, unsigned long tick, int id, basics::numptr data) {
	int users = count_users();
	unsigned char buf[16];

	if( session_isActive(login_fd) )
	{
		// send number of user to login server
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}
	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf, 6);

	return 0;
}
void connect_login(void)
{
	if( session_isActive(login_fd) )
	{
		session[login_fd]->func_parse = parse_tologin;
		realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		WFIFOW(login_fd,0) = 0x2710;
		memcpy(WFIFOP(login_fd,2), userid, strlen(userid) < 24 ? 1+strlen(userid) : 24);
		WFIFOB(login_fd,25) = 0;
		memcpy(WFIFOP(login_fd,26), passwd, strlen(passwd) < 24 ? 1+strlen(passwd) : 24);
		WFIFOB(login_fd,49) = 0;
		memcpy(WFIFOP(login_fd,50), server_name, strlen(server_name) < 20 ? 1+strlen(server_name) : 20);
		WFIFOB(login_fd,69) = 0;
		WFIFOB(login_fd,70) = char_maintenance;
		WFIFOB(login_fd,71) = char_new_display;
		WFIFOB(login_fd,72) = 0;
		WFIFOB(login_fd,73) = 0;
		// ip's
		WFIFOLIP(login_fd,74) = charaddress.LANIP();
		WFIFOLIP(login_fd,78) = charaddress.LANMask();
		WFIFOW(login_fd,82)   = charaddress.LANPort();
		WFIFOLIP(login_fd,84) = charaddress.WANIP();
		WFIFOW(login_fd,88)   = charaddress.WANPort();

		WFIFOSET(login_fd,90);
	}
}
///////////////////////////////////////////////////////////////////////////////
int check_connect_login_server(int tid, unsigned long tick, int id, basics::numptr data)
{
	if( !session_isActive(login_fd) )
	{
		ShowStatus("Attempt to connect to login-server...(%s)\n", server_name);
		login_fd = make_connection(loginaddress.addr(), loginaddress.port());

		connect_login();
	}
	if( !session_isActive(char_fd) )
	{	// the listen port was dropped, open it new
		char_fd = make_listen(charaddress.LANIP(),charaddress.LANPort());
		if(char_fd>=0)
		{
			char_log("The char-server is ready (Server is listening on %s:%d)." RETCODE, charaddress.LANIP().tostring(NULL),charaddress.LANPort());
			ShowStatus("The char-server is "CL_BT_GREEN"ready"CL_NORM" (listening on '"CL_WHITE"%s:%d"CL_RESET"').\n", charaddress.LANIP().tostring(NULL),charaddress.LANPort());
		}
		else
			ShowStatus("open listen socket on '"CL_WHITE"%s:%d"CL_RESET"' failed.\n", charaddress.LANIP().tostring(NULL),charaddress.LANPort());
	}
	return 0;
}
int check_dropped_charwan(int tid, unsigned long tick, int id, basics::numptr data)
{
	static basics::CParam<bool> automatic_wan_setup("automatic_wan_setup", false);

	if(	automatic_wan_setup )
	{
		if( dropped_WAN(charaddress.WANIP(), charaddress.WANPort()) )
		{
			if( charaddress.WANIP() != basics::ipany )
				ShowWarning("WAN connection dropped.\n");

			if( initialize_WAN(charaddress, 6121) )
			{	// if ok, then check if we can connect to it
				if( dropped_WAN(charaddress.WANIP(), charaddress.WANPort()) )
				{	// most likely the router/firewall settings are wrong
					// give a warning and disable the automatic wan setup
					char buf1[16],buf2[16];
					ShowWarning("WAN connection not available or router with wrong configuration.\n"
								CL_SPACE"expecting correct port forward from router to local machine\n"
								CL_SPACE"router is on %s:%i, local machine on %s:%i\n"
								, charaddress.WANIP().tostring(buf1), charaddress.WANPort()
								, charaddress.LANIP().tostring(buf2), charaddress.LANPort()
								);

					ShowWarning("automatic WAN detection will be disabled now.\n");
					automatic_wan_setup = false;

					// reset the wanip to default
					// only accept lan ip's from now
					charaddress.LANMask() = basics::ipany;
					charaddress.SetWANIP(basics::ipany);
				}
				// send changes to login
				connect_login();
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
int char_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];

	FILE *fp = basics::safefopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return 0;
	}

	while(fgets(line, sizeof(line), fp))
	{
		if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
		{
			remove_control_chars(w1);
			basics::itrim(w1);
			if(!*w1) continue;

			remove_control_chars(w2);
			basics::itrim(w2);

			if(strcasecmp(w1, "userid") == 0)
			{
				safestrcpy(userid, sizeof(userid), w2);
			}
			else if(strcasecmp(w1, "passwd") == 0)
			{
				safestrcpy(passwd, sizeof(passwd), w2);
			}
			else if(strcasecmp(w1, "server_name") == 0)
			{
				safestrcpy(server_name, sizeof(server_name), w2);
				ShowStatus("%s server has been initialized\n", server_name);
			}
			else if(strcasecmp(w1, "wisp_server_name") == 0)
			{
				if(strlen(w2) >= 4)
					safestrcpy(wisp_server_name, sizeof(wisp_server_name), w2);
			}
			else if(strcasecmp(w1, "login_ip") == 0)
			{
				loginaddress = w2;
				ShowInfo("Expecting login server at %s\n", loginaddress.tostring(NULL));
			}
			else if(strcasecmp(w1, "login_port") == 0)
			{
				loginaddress.port() = basics::config_switch<ushort>(w2);
			}
			else if(strcasecmp(w1, "char_ip") == 0)
			{
				charaddress = w2;
				ShowInfo("Using char server with %s\n", loginaddress.tostring(NULL));
			}
			else if(strcasecmp(w1, "char_port") == 0)
			{
				charaddress.LANPort() = basics::config_switch<ushort>(w2);
			}
			else if(strcasecmp(w1, "char_maintenance") == 0)
			{
				char_maintenance = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "char_new_display") == 0)
			{
				char_new_display = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "email_creation") == 0)
			{
				email_creation = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1, "max_connect_user") == 0)
			{
				max_connect_user = basics::config_switch<int>(w2,0);
			}
			else if(strcasecmp(w1, "gm_allow_level") == 0)
			{
				gm_allow_level = basics::config_switch<int>(w2,0,99);
			}
			else if(strcasecmp(w1, "check_ip_flag") == 0)
			{
				check_ip_flag = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1, "autosave_time") == 0)
			{
				autosave_interval = basics::config_switch<int>(w2)*1000;
				if (autosave_interval <= 0)
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			}
			else if(strcasecmp(w1,"log_char")==0)
			{	//log char or not [devil]
				log_char = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1, "unknown_char_name") == 0)
			{
				safestrcpy(unknown_char_name, sizeof(unknown_char_name), w2);
			}
			else if(strcasecmp(w1, "char_log_filename") == 0)
			{
				safestrcpy(char_log_filename, sizeof(char_log_filename), w2);
			}
			else if(strcasecmp(w1, "online_txt_filename") == 0)
			{
				safestrcpy(online_txt_filename, sizeof(online_txt_filename), w2);
			}
			else if(strcasecmp(w1, "online_html_filename") == 0)
			{
				safestrcpy(online_html_filename, sizeof(online_html_filename), w2);
			}
			else if(strcasecmp(w1, "online_sorting_option") == 0)
			{
				online_sorting_option = atoi(w2);
			}
			else if(strcasecmp(w1, "online_display_option") == 0)
			{
				online_display_option = atoi(w2);
			}
			else if(strcasecmp(w1, "online_gm_display_min_level") == 0)
			{	// minimum GM level to display 'GM' when we want to display it
				online_gm_display_min_level = atoi(w2);
				// send online file every 5 seconds to player is enough
				if(online_gm_display_min_level < 5)
					online_gm_display_min_level = 5;
			}
			else if(strcasecmp(w1, "online_refresh_html") == 0)
			{
				online_refresh_html = atoi(w2);
				if (online_refresh_html < 1)
					online_refresh_html = 1;
			}
			else if(strcasecmp(w1, "console") == 0)
			{
				console = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1, "import") == 0)
			{
				char_config_read(w2);
			}
		}
	}
	fclose(fp);

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
void do_final(void)
{
	size_t i;
	ShowStatus("Terminating server.\n");

	char_db.close();

	create_online_files();

	inter_save();
	set_all_offline();

	inter_final();
	irc_final();


	///////////////////////////////////////////////////////////////////////////
	// delete sessions
	for(i = 0; i < (size_t)fd_max; ++i)
		if(session[i] != NULL) 
			session_Delete(i);
	// clear externaly stored fd's
	login_fd=-1;
	char_fd=-1;
	///////////////////////////////////////////////////////////////////////////
	char_log("----End of char-server (normal end with closing of all files)." RETCODE);
	ShowStatus("Successfully terminated.\n");
}
///////////////////////////////////////////////////////////////////////////////
unsigned char getServerType()
{
	return ATHENA_SERVER_CHAR | ATHENA_SERVER_INTER | ATHENA_SERVER_CORE;
}
///////////////////////////////////////////////////////////////////////////////
int do_init(int argc, char **argv)
{
	int i;

	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_db.init( (argc < 2) ? CHAR_CONF_NAME : argv[1] );
	var_db.init( (argc < 2) ? CHAR_CONF_NAME : argv[1] );


	// a newline in the log...
	char_log("");
	// moved behind char_config_read in case we changed the filename [celest]
	char_log("The char-server starting..." RETCODE);

	for(i=0; i<MAX_MAP_SERVERS; ++i)
	{
		server[i].fd = -1;
	}

	update_online = time(NULL);
	create_online_files(); // update online players files at start of the server

	inter_init((argc > 2) ? argv[2] : inter_cfgName);	// inter server 初期化
	
	set_defaultparse(parse_char);

	basics::CParam<bool> automatic_wan_setup("automatic_wan_setup",false);
	if( automatic_wan_setup )
		initialize_WAN(charaddress, 6121);


	add_timer_func_list(check_connect_login_server, "check_connect_login_server");
	add_timer_interval(gettick() + 1000, 10 * 1000, check_connect_login_server, 0, 0);

	add_timer_func_list(check_dropped_charwan, "check_dropped_charwan");
	add_timer_interval(gettick() + 10000, 600 * 1000, check_dropped_charwan, 0, 0);

	add_timer_func_list(send_users_tologin, "send_users_tologin");
	add_timer_interval(gettick() + 1000, 5 * 1000, send_users_tologin, 0, 0);
	add_timer_func_list(mmo_char_sync_timer, "mmo_char_sync_timer");
	add_timer_interval(gettick() + autosave_interval, autosave_interval, mmo_char_sync_timer, 0, 0);

	if(console)
	{
	    set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	irc_init();
	return 0;
}
