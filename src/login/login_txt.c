// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/md5calc.h"
#include "../common/lock.h"
#include "account.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for stat/lstat/fstat


// GM account management
struct gm_account* gm_account_db = NULL;
unsigned int GM_num = 0; // number of gm accounts
char GM_account_filename[1024] = "conf/GM_account.txt";
long creation_time_GM_account_file; // tracks the last-changed timestamp of the gm accounts file
int gm_account_filename_check_timer = 15; // Timer to check if GM_account file has been changed and reload GM account automaticaly (in seconds; default: 15)

char login_log_filename[1024] = "log/login.log";

// TXT-specifics
char account_txt[1024] = "save/account.txt";


// ladmin configuration
bool admin_state = false;
char admin_pass[24] = "";
uint32 admin_allowed_ip = 0;

int parse_admin(int fd);


// temporary external imports
extern AccountDB* accounts;
extern struct Login_Config login_config;
extern int charif_sendallwos(int sfd, uint8* buf, size_t len);


//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
int isGM(int account_id)
{
	unsigned int i;
	ARR_FIND( 0, GM_num, i, gm_account_db[i].account_id == account_id );
	return ( i < GM_num ) ? gm_account_db[i].level : 0;
}

//----------------------------------------------------------------------
// Adds a new GM using acc id and level
//----------------------------------------------------------------------
void addGM(int account_id, int level)
{
	static unsigned int GM_max = 0;
	unsigned int i;
	struct mmo_account acc;

	if( !accounts->load_num(accounts, &acc, account_id) )
		return; // no such account

	ARR_FIND( 0, GM_num, i, gm_account_db[i].account_id == account_id );
	if( i < GM_num )
	{
		if (gm_account_db[i].level == level)
			ShowWarning("addGM: GM account %d defined twice (same level: %d).\n", account_id, level);
		else {
			ShowWarning("addGM: GM account %d defined twice (levels: %d and %d).\n", account_id, gm_account_db[i].level, level);
			gm_account_db[i].level = level;
		}
		return; // entry already present
	}

	// new account
	if (GM_num >= GM_max) {
		GM_max += 256;
		RECREATE(gm_account_db, struct gm_account, GM_max);
	}
	gm_account_db[GM_num].account_id = account_id;
	gm_account_db[GM_num].level = level;
	GM_num++;
	if (GM_num >= 4000)
		ShowWarning("4000 GM accounts found. Next GM accounts are not read.\n");
}

//-------------------------------------------------------
// Reading function of GM accounts file (and their level)
//-------------------------------------------------------
int read_gm_account(void)
{
	char line[512];
	FILE *fp;
	int account_id, level;
	int line_counter;
	struct stat file_stat;
	int start_range = 0, end_range = 0, is_range = 0, current_id = 0;

	if(gm_account_db) aFree(gm_account_db);
	CREATE(gm_account_db, struct gm_account, 1);
	GM_num = 0;

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		creation_time_GM_account_file = 0; // error
	else
		creation_time_GM_account_file = (long)file_stat.st_mtime;

	if ((fp = fopen(GM_account_filename, "r")) == NULL) {
		ShowError("read_gm_account: GM accounts file [%s] not found.\n", GM_account_filename);
		return 1;
	}

	line_counter = 0;
	// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
	// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
	while(fgets(line, sizeof(line), fp) && GM_num < 4000)
	{
		line_counter++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		is_range = (sscanf(line, "%d%*[-~]%d %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
		if (!is_range && sscanf(line, "%d %d", &account_id, &level) != 2 && sscanf(line, "%d: %d", &account_id, &level) != 2)
			ShowError("read_gm_account: file [%s], invalid 'acount_id|range level' format (line #%d).\n", GM_account_filename, line_counter);
		else if (level <= 0)
			ShowError("read_gm_account: file [%s] %dth account (line #%d) (invalid level [0 or negative]: %d).\n", GM_account_filename, GM_num+1, line_counter, level);
		else {
			if (level > 99) {
				ShowNotice("read_gm_account: file [%s] %dth account (invalid level, but corrected: %d->99).\n", GM_account_filename, GM_num+1, level);
				level = 99;
			}
			if (is_range) {
				if (start_range==end_range)
					ShowError("read_gm_account: file [%s] invalid range, beginning of range is equal to end of range (line #%d).\n", GM_account_filename, line_counter);
				else if (start_range>end_range)
					ShowError("read_gm_account: file [%s] invalid range, beginning of range must be lower than end of range (line #%d).\n", GM_account_filename, line_counter);
				else
					for (current_id = start_range;current_id<=end_range;current_id++)
						addGM(current_id,level);
			} else {
				addGM(account_id,level);
			}
		}
	}
	fclose(fp);

	ShowStatus("read_gm_account: file '%s' read (%d GM accounts found).\n", GM_account_filename, GM_num);

	return 0;
}

//-----------------------------------------------------
// Send GM accounts to one or all char-servers
//-----------------------------------------------------
void send_GM_accounts(int fd)
{
	unsigned int i;
	uint8 buf[32767];
	uint16 len;

	len = 4;
	WBUFW(buf,0) = 0x2732;
	for(i = 0; i < GM_num; i++)
		// send only existing accounts. We can not create a GM account when server is online.
		if (gm_account_db[i].level > 0) {
			WBUFL(buf,len) = gm_account_db[i].account_id;
			WBUFB(buf,len+4) = (uint8)gm_account_db[i].level;
			len += 5;
			if (len >= 32000) {
				ShowWarning("send_GM_accounts: Too many accounts! Only %d out of %d were sent.\n", i, GM_num);
				break;
			}
		}

	WBUFW(buf,2) = len;
	if (fd == -1) // send to all charservers
		charif_sendallwos(-1, buf, len);
	else { // send only to target
		WFIFOHEAD(fd,len);
		memcpy(WFIFOP(fd,0), buf, len);
		WFIFOSET(fd,len);
	}

	return;
}

//-----------------------------------------------------
// Check if GM file account have been changed
//-----------------------------------------------------
int check_GM_file(int tid, unsigned int tick, int id, int data)
{
	struct stat file_stat;
	long new_time;

	// if we would not check
	if (gm_account_filename_check_timer < 1)
		return 0;

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		new_time = 0; // error
	else
		new_time = (long)file_stat.st_mtime;

	if (new_time != creation_time_GM_account_file) {
		read_gm_account();
		send_GM_accounts(-1);
	}

	return 0;
}

/*=============================================
 * Records an event in the login log
 *---------------------------------------------*/
void login_log(uint32 ip, const char* username, int rcode, const char* message)
{
	FILE* log_fp;

	if( !login_config.log_login )
		return;
	
	log_fp = fopen(login_log_filename, "a");
	if( log_fp != NULL )
	{
		char esc_username[NAME_LENGTH*4+1];
		char esc_message[255*4+1];
		time_t raw_time;
		char str_time[24];

		sv_escape_c(esc_username, username, NAME_LENGTH, NULL);
		sv_escape_c(esc_message, message, 255, NULL);

		time(&raw_time);
		strftime(str_time, 24, login_config.date_format, localtime(&raw_time));
		str_time[23] = '\0';

		fprintf(log_fp, "%s\t%s\t%s\t%d\t%s\n", str_time, ip2str(ip,NULL), esc_username, rcode, esc_message);

		fclose(log_fp);
	}
}


//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void)
{
	if( admin_state ) {
		if (admin_pass[0] == '\0') {
			ShowWarning("Administrator password is void (admin_pass).\n");
		} else if (strcmp(admin_pass, "admin") == 0) {
			ShowWarning("You are using the default administrator password (admin_pass).\n");
			ShowWarning("  We highly recommend that you change it.\n");
		}
	}

	if (gm_account_filename_check_timer < 0) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 15 sec (default).\n");
		gm_account_filename_check_timer = 15;
	} else if (gm_account_filename_check_timer == 1) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 2 sec (minimum value).\n");
		gm_account_filename_check_timer = 2;
	}

	if (login_config.min_level_to_connect < 0) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting 0 (any player).\n", login_config.min_level_to_connect);
		login_config.min_level_to_connect = 0;
	} else if (login_config.min_level_to_connect > 99) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting to 99 (only GM level 99)\n", login_config.min_level_to_connect);
		login_config.min_level_to_connect = 99;
	}

	if (login_config.start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowWarning("Invalid value for start_limited_time parameter\n");
		ShowWarning("  -> setting to -1 (new accounts are created with unlimited time).\n");
		login_config.start_limited_time = -1;
	}

	return;
}

bool login_config_read_txt(const char* w1, const char* w2)
{
	if(!strcmpi(w1, "login_log_filename") == 0)
		safestrncpy(login_log_filename, w2, sizeof(login_log_filename));
	else if(!strcmpi(w1, "admin_state") == 0)
		admin_state = (bool)config_switch(w2);
	else if(!strcmpi(w1, "admin_pass") == 0)
		safestrncpy(admin_pass, w2, sizeof(admin_pass));
	else if(!strcmpi(w1, "admin_allowed_ip") == 0)
		admin_allowed_ip = host2ip(w2);
	else if(!strcmpi(w1, "account_txt") == 0)
		safestrncpy(account_txt, w2, sizeof(account_txt));
	else if(!strcmpi(w1, "gm_account_filename") == 0)
		safestrncpy(GM_account_filename, w2, sizeof(GM_account_filename));
	else if(!strcmpi(w1, "gm_account_filename_check_timer") == 0)
		gm_account_filename_check_timer = atoi(w2);
	else
		return false;

	return true;
}
