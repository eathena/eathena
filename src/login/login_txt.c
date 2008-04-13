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

extern struct Login_Config login_config;
char login_log_filename[1024] = "log/login.log";
char account_txt[1024] = "save/account.txt";

// ladmin configuration
bool admin_state = false;
char admin_pass[24] = "";
uint32 admin_allowed_ip = 0;

int parse_admin(int fd);


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
	if(!strcmpi(w1, "login_log_filename"))
		safestrncpy(login_log_filename, w2, sizeof(login_log_filename));
	else if(!strcmpi(w1, "admin_state"))
		admin_state = (bool)config_switch(w2);
	else if(!strcmpi(w1, "admin_pass"))
		safestrncpy(admin_pass, w2, sizeof(admin_pass));
	else if(!strcmpi(w1, "admin_allowed_ip"))
		admin_allowed_ip = host2ip(w2);
	else if(!strcmpi(w1, "account_txt"))
		safestrncpy(account_txt, w2, sizeof(account_txt));
	else
		return false;

	return true;
}
