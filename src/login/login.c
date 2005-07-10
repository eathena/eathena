// $Id: login.c,v 1.1.1.1 2004/09/10 17:26:53 MagicalTux Exp $
// new version of the login-server by [Yor]

#include "base.h"
#include "core.h"
#include "socket.h"
#include "timer.h"
#include "mmo.h"
#include "version.h"
#include "db.h"
#include "md5calc.h"
#include "lock.h"
#include "malloc.h"
#include "utils.h"
#include "showmsg.h"
#include "strlib.h"

#ifdef PASSWORDENC
#include "md5calc.h"
#endif

#include "login.h"


unsigned long account_id_count = START_ACCOUNT_NUM;
unsigned short server_num;
unsigned long new_account_flag = 0;

unsigned long	login_ip	= INADDR_ANY;	// bind on all addresses by default
unsigned short	login_port	= 6900;

//Added for lan support
unsigned long	lan_char_ip	= INADDR_LOOPBACK;	//127.0.0.1
unsigned long	subnet_ip	= INADDR_LOOPBACK;	//127.0.0.1
unsigned long	subnet_mask	= INADDR_BROADCAST;	//255.255.255.255


char account_filename[1024] = "save/account.txt";
char GM_account_filename[1024] = "conf/GM_account.txt";
char login_log_filename[1024] = "log/login.log";
FILE *log_fp = NULL;
char login_log_unknown_packets_filename[1024] = "log/login_unknown_packets.log";
char date_format[32] = "%Y-%m-%d %H:%M:%S";
int save_unknown_packets = 0;
long creation_time_GM_account_file;
int gm_account_filename_check_timer = 15; // Timer to check if GM_account file has been changed and reload GM account automaticaly (in seconds; default: 15)

int log_login = 1;

int display_parse_login = 0; // 0: no, 1: yes
int display_parse_admin = 0; // 0: no, 1: yes
int display_parse_fromchar = 0; // 0: no, 1: yes (without packet 0x2714), 2: all packets

struct mmo_char_server server[MAX_SERVERS];

int login_fd;

unsigned int new_reg_tick=0;
int allowed_regs=1;
int num_regs=0;
int time_allowed=10; //Init this to 10 seconds. [Skotlex]

enum {
	ACO_DENY_ALLOW = 0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
	ACO_STRSIZE = 128,
};

int access_order = ACO_DENY_ALLOW;
int access_allownum = 0;
int access_denynum = 0;

char *access_allow = NULL;
char *access_deny = NULL;

int access_ladmin_allownum = 0;
char *access_ladmin_allow = NULL;


int min_level_to_connect = 0; // minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int add_to_unlimited_account = 0; // Give possibility or not to adjust (ladmin command: timeadd) the time of an unlimited account.
int start_limited_time = -1; // Starting additional sec from now for the limited time at creation of accounts (-1: unlimited time, 0 or more: additional sec from now)
int check_ip_flag = 1; // It's to check IP of a player between login-server and char-server (part of anti-hacking system)

int check_client_version = 0; //Client version check ON/OFF .. (sirius)
int client_version_to_connect = 20; //Client version needed to connect ..(sirius)



struct login_session_data {
	int md5keylen;
	char md5key[20];
};

#define AUTH_FIFO_SIZE 256
struct {
	unsigned long account_id;
	unsigned long login_id1;
	unsigned long login_id2;
	unsigned long ip;
	int sex;
	int delflag;
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

struct auth_dat {
	unsigned long account_id;
	int sex;
	char userid[24];
	char pass[33]; // 33 for 32 + NULL terminated
	char lastlogin[24]; 
	unsigned long logincount;
	unsigned long state; // packet 0x006a value + 1 (0: compte OK)
	char email[40]; // e-mail (by default: a@a.com)
	char error_message[20]; // Message of error code #6 = Your are Prohibited to log in until %s (packet 0x006a)
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	char last_ip[16]; // save of last IP of connection
	char memo[255]; // a memo field
	long account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];
} *auth_dat = NULL;

int auth_num = 0;
int auth_max = 0;

// define the number of times that some players must authentify them before to save account file.
// it's just about normal authentification. If an account is created or modified, save is immediatly done.
// An authentification just change last connected IP and date. It already save in log file.
// set minimum auth change before save:
#define AUTH_BEFORE_SAVE_FILE 10
// set divider of auth_num to found number of change before save
#define AUTH_SAVE_FILE_DIVIDER 50
int auth_before_save_file = 0; // Counter. First save when 1st char-server do connection.

int admin_state = 0;
char admin_pass[24] = "";
int GM_num;
int GM_max=256;
char gm_pass[64] = "";
int level_new_gm = 60;

struct gm_account *gm_account_db;

static struct dbt *online_db;

int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 1;

int use_md5_passwds = 0;

int console = 0;

/*
class Globals : public CConfig
{
public:

	virtual bool ProcessConfig(const char*w1,const char*w2)
	{
		if (strcasecmp(w1, "admin_state") == 0)
		{
			admin_state = isSwitch(w2);
		}
		else if (strcasecmp(w1, "admin_pass") == 0)
		{
			memset(admin_pass, 0, sizeof(admin_pass));
			safestrcpy(admin_pass, w2,sizeof(admin_pass));
			gm_pass[sizeof(admin_pass)-1] = '\0';
		}
		else if (strcasecmp(w1, "ladminallowip") == 0)
		{
			if (strcasecmp(w2, "clear") == 0)
			{
				if (access_ladmin_allow)
				{	// clear and initialize
					aFree(access_ladmin_allow);
					access_ladmin_allow = NULL;
					access_ladmin_allownum = 0;
				}
			}
			else if (strcasecmp(w2, "all") == 0)
			{	// reset all previous values
				if (access_ladmin_allow)
					aFree(access_ladmin_allow);
				// set to all
				access_ladmin_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
				access_ladmin_allownum = 1;
				access_ladmin_allow[0] = '\0';
			}
			else if (w2[0] && !(access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0'))
			{	// don't add IP if already 'all'
				if (access_ladmin_allow)
					access_ladmin_allow = (char*)aRealloc(access_ladmin_allow, (access_ladmin_allownum+1) * ACO_STRSIZE);
				else
					access_ladmin_allow = (char*)aMalloc(ACO_STRSIZE * sizeof(char));
				safestrcpy(access_ladmin_allow + (access_ladmin_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
				access_ladmin_allow[access_ladmin_allownum * ACO_STRSIZE - 1] = '\0';
			}
		}
		else if (strcasecmp(w1, "gm_pass") == 0)
		{
			memset(gm_pass, 0, sizeof(gm_pass));
			safestrcpy(gm_pass, w2, sizeof(gm_pass));
			gm_pass[sizeof(gm_pass)-1] = '\0';
		}
		else if (strcasecmp(w1, "level_new_gm") == 0)
		{
			level_new_gm = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "new_account") == 0)
		{
			new_account_flag = isSwitch(w2);
		}
		else if (strcasecmp(w1, "login_ip") == 0)
		{
			login_ip = String2IP(w2);
		}
		else if (strcasecmp(w1, "login_port") == 0)
		{
			login_port = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "account_filename") == 0)
		{
			memset(account_filename, 0, sizeof(account_filename));
			safestrcpy(account_filename, w2, sizeof(account_filename));
			account_filename[sizeof(account_filename)-1] = '\0';
		}
		else if (strcasecmp(w1, "gm_account_filename") == 0)
		{
			memset(GM_account_filename, 0, sizeof(GM_account_filename));
			safestrcpy(GM_account_filename, w2, sizeof(GM_account_filename));
			GM_account_filename[sizeof(GM_account_filename)-1] = '\0';
		}
		else if (strcasecmp(w1, "gm_account_filename_check_timer") == 0)
		{
			gm_account_filename_check_timer = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "use_MD5_passwords") == 0)
		{
			use_md5_passwds = isSwitch(w2);
		}
		else if (strcasecmp(w1, "login_log_filename") == 0)
		{
			memset(login_log_filename, 0, sizeof(login_log_filename));
			safestrcpy(login_log_filename, w2, sizeof(login_log_filename));
			login_log_filename[sizeof(login_log_filename)-1] = '\0';
		}
		else if (strcasecmp(w1, "log_login") == 0)
		{
			log_login = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "login_log_unknown_packets_filename") == 0)
		{
			memset(login_log_unknown_packets_filename, 0, sizeof(login_log_unknown_packets_filename));
			safestrcpy(login_log_unknown_packets_filename, w2, sizeof(login_log_unknown_packets_filename));
			login_log_unknown_packets_filename[sizeof(login_log_unknown_packets_filename)-1] = '\0';
		}
		else if (strcasecmp(w1, "save_unknown_packets") == 0)
		{
			save_unknown_packets = isSwitch(w2);
		}
		else if (strcasecmp(w1, "display_parse_login") == 0)
		{
			display_parse_login = isSwitch(w2);
		}
		else if (strcasecmp(w1, "display_parse_admin") == 0)
		{
			display_parse_admin = isSwitch(w2);
		}
		else if (strcasecmp(w1, "display_parse_fromchar") == 0)
		{	// 0: no, 1: yes (without packet 0x2714), 2: all packets
			display_parse_fromchar = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "date_format") == 0)
		{	// note: never have more than 19 char for the date!
			memset(date_format, 0, sizeof(date_format));
			switch ( isSwitchValue(w2) )
			{
			case 0:
				strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
				break;
			case 1:
				strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
				break;
			case 2:
				strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
				break;
			case 3:
				strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
				break;
			}
		}
		else if (strcasecmp(w1, "min_level_to_connect") == 0)
		{
			min_level_to_connect = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "add_to_unlimited_account") == 0)
		{
			add_to_unlimited_account = isSwitch(w2);
		}
		else if (strcasecmp(w1, "start_limited_time") == 0)
		{
			start_limited_time = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "check_ip_flag") == 0)
		{
			check_ip_flag = isSwitch(w2);
		}
		else if (strcasecmp(w1, "order") == 0)
		{
			if( strcasecmp(w2, "deny,allow") == 0 || strcasecmp(w2, "deny, allow") == 0)
				access_order = ACO_DENY_ALLOW;
			else if( strcasecmp(w2, "allow,deny") == 0 || strcasecmp(w2, "allow, deny") == 0)
				access_order = ACO_ALLOW_DENY;
			else if( strcasecmp(w2, "mutual-failture") == 0 || strcasecmp(w2, "mutual-failure") == 0)
				access_order = ACO_MUTUAL_FAILTURE;
			else
				access_order = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "allow") == 0)
		{
			if (strcasecmp(w2, "clear") == 0)
			{
				if (access_allow)
				{
					aFree(access_allow);
					access_allow = NULL;
					access_allownum = 0;
				}
			}
			else if (strcasecmp(w2, "all") == 0)
			{	// reset all previous values
				if (access_allow)
					aFree(access_allow);
				// set to all
				access_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
				access_allownum = 1;
				access_allow[0] = '\0';
			}
			else if( w2[0] && !(access_allownum == 1 && access_allow[0] == '\0') )
			{	// don't add IP if already 'all'
				if (access_allow)
					access_allow = (char*)aRealloc(access_allow, (access_allownum+1) * ACO_STRSIZE);
				else
					access_allow = (char*)aMalloc(ACO_STRSIZE * sizeof(char));
				safestrcpy(access_allow + (access_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
				access_allow[access_allownum * ACO_STRSIZE - 1] = '\0';
			}
		}
		else if (strcasecmp(w1, "deny") == 0)
		{
			if (strcasecmp(w2, "clear") == 0)
			{
				if (access_deny)
				{
					aFree(access_deny);
					access_deny = NULL;
					access_denynum = 0;
				}
			}
			else if (strcasecmp(w2, "all") == 0)
			{	// reset all previous values
				if (access_deny)
					aFree(access_deny);
				// set to all
				access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
				access_denynum = 1;
				access_deny[0] = '\0';
			}
			else if( w2[0] && !(access_denynum == 1 && access_deny[0] == '\0') )
			{	// don't add IP if already 'all'
				if (access_deny)
					access_deny = (char*)aRealloc(access_deny, (access_denynum+1) * ACO_STRSIZE);
				else
					access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
				safestrcpy(access_deny + (access_denynum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
				access_deny[access_denynum * ACO_STRSIZE - 1] = '\0';
			}
		}
		else if (strcasecmp(w1, "dynamic_pass_failure_ban") == 0)
		{	// dynamic password error ban
			dynamic_pass_failure_ban = isSwitch(w2);
		}
		else if (strcasecmp(w1, "dynamic_pass_failure_ban_time") == 0)
		{
			dynamic_pass_failure_ban_time = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_many") == 0)
		{
			dynamic_pass_failure_ban_how_many = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_long") == 0)
		{
			dynamic_pass_failure_ban_how_long = isSwitchValue(w2);
		}
		else if(strcasecmp(w1,"imalive_on")==0)
		{	//Added by Mugendai for I'm Alive mod
			imalive_on = isSwitchValue(w2);
		}
		else if(strcasecmp(w1,"imalive_time")==0)
		{	//Added by Mugendai for I'm Alive mod
			imalive_time = isSwitchValue(w2);
		}
		else if(strcasecmp(w1,"flush_on")==0)
		{	//Added by Mugendai for GUI
			flush_on = isSwitchValue(w2);
		}
		else if(strcasecmp(w1,"flush_time")==0)
		{	//Added by Mugendai for GUI
			flush_time = isSwitchValue(w2);
		}
		else if(strcasecmp(w1, "check_client_version") == 0)
		{	//Added by Sirius for client version check
			check_client_version = isSwitch(w2);
		}
		else if(strcasecmp(w1, "client_version_to_connect") == 0)
		{	//Added by Sirius for client version check
			client_version_to_connect = isSwitchValue(w2);
		}
		else if (strcasecmp(w1, "console") == 0)
		{
			console = isSwitch(w2);
		}
		return 0;
	}
};
*/


//------------------------------
// Writing function of logs file
//------------------------------
int login_log(char *fmt, ...)
{
	if (log_login)
	{
		va_list ap;
		time_t unixtime;
		struct timeval tv;
		char tmpstr[2048];

		if(!log_fp)
			log_fp = savefopen(login_log_filename, "a");

		if (log_fp) {
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(log_fp, RETCODE);
			else {
				va_start(ap, fmt);
				gettimeofday(&tv, NULL);
				unixtime = tv.tv_sec;
				strftime(tmpstr, 24, date_format, localtime(&unixtime));
				sprintf(tmpstr + strlen(tmpstr), ".%03ld: %s", tv.tv_usec / 1000, fmt);
				vfprintf(log_fp, tmpstr, ap);
				va_end(ap);
			}
			fflush(log_fp); // under cygwin or windows, if software is stopped, data are not written in the file -> fflush at every line
		}
	}

	return 0;
}

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

int online_db_final(void *key,void *data,va_list ap)
{
	unsigned long *p = (unsigned long *) data;
	if (p) aFree(p);
	return 0;
}
void add_online_user (unsigned long account_id) {
	unsigned long *p = (unsigned long*)numdb_search(online_db, account_id);
	if (p == NULL) {
		p = (unsigned long *)aMalloc(sizeof(unsigned long));
		*p = account_id;
		numdb_insert(online_db, account_id, p);
	}
}
int is_user_online (unsigned long account_id) {
	unsigned long *p = (unsigned long*)numdb_search(online_db, account_id);
	return (p != NULL);
}
void remove_online_user (unsigned long account_id) {
	unsigned long *p;
	if (account_id == 99) {	// reset all to offline
		numdb_final(online_db, online_db_final);	// purge db
		online_db = numdb_init();	// reinitialise
	}
	p = (unsigned long*)numdb_erase(online_db, account_id);
	aFree(p);
}

//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
int isGM(unsigned long account_id) {
	int i;
	for(i=0; i < GM_num; i++)
		if(gm_account_db[i].account_id == account_id)
			return gm_account_db[i].level;
	return 0;
}

//----------------------------------------------------------------------
// Adds a new GM using acc id and level
//----------------------------------------------------------------------
void addGM(unsigned long account_id, unsigned long level) {
	int i;
	int do_add = 0;
	for(i = 0; i < auth_num; i++) {
		if (auth_dat[i].account_id==account_id) {
			do_add = 1;
			break;
		}
	}
	for(i = 0; i < GM_num; i++)
		if (gm_account_db[i].account_id == account_id) {
			if (gm_account_db[i].level == level)
				ShowMessage("addGM: GM account %d defined twice (same level: %d).\n", account_id, level);
			else {
				ShowMessage("addGM: GM account %d defined twice (levels: %d and %d).\n", account_id, gm_account_db[i].level, level);
				gm_account_db[i].level = level;
			}
			return;
		}

	// if new account
	if (i == GM_num && do_add) {
		if (GM_num >= GM_max) {
			GM_max += 256;
			gm_account_db = (struct gm_account*)aRealloc(gm_account_db, sizeof(struct gm_account) * GM_max);
			memset(gm_account_db + (GM_max - 256), 0, 256 * sizeof(struct gm_account));
		}
		gm_account_db[GM_num].account_id = account_id;
		gm_account_db[GM_num].level = level;
		GM_num++;
		if (GM_num >= 4000) {
			ShowMessage("***WARNING: 4000 GM accounts found. Next GM accounts are not read.\n");
			login_log("***WARNING: 4000 GM accounts found. Next GM accounts are not read." RETCODE);
		}
	}
}

//-------------------------------------------------------
// Reading function of GM accounts file (and their level)
//-------------------------------------------------------
int read_gm_account(void) {
	char line[512];
	FILE *fp;
	int account_id;
	int level;
	size_t line_counter;
	struct stat file_stat;
	size_t start_range = 0, end_range = 0, is_range = 0, current_id = 0;

	if(gm_account_db) aFree(gm_account_db);
	GM_num = 0;
	if(GM_max <= 0)	GM_max = 256;
	gm_account_db = (struct gm_account*)aCalloc(GM_max, sizeof(struct gm_account));

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		creation_time_GM_account_file = 0; // error
	else
		creation_time_GM_account_file = file_stat.st_mtime;

	if ((fp = savefopen(GM_account_filename, "r")) == NULL) {
		ShowMessage("read_gm_account: GM accounts file [%s] not found.\n", GM_account_filename);
		ShowMessage("                 Actually, there is no GM accounts on the server.\n");
		login_log("read_gm_account: GM accounts file [%s] not found." RETCODE, GM_account_filename);
		login_log("                 Actually, there is no GM accounts on the server." RETCODE);
		return 1;
	}

	line_counter = 0;
	// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
	// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
	while(fgets(line, sizeof(line)-1, fp) && GM_num < 4000) {
		line_counter++;
		if( !skip_empty_line(line) )
			continue;
		is_range = (sscanf(line, "%d%*[-~]%d %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
		if (!is_range && sscanf(line, "%d %d", &account_id, &level) != 2 && sscanf(line, "%d: %d", &account_id, &level) != 2)
			ShowMessage("read_gm_account: file [%s], invalid 'acount_id|range level' format (line #%d).\n", GM_account_filename, line_counter);
		else if (level <= 0)
			ShowMessage("read_gm_account: file [%s] %dth account (line #%d) (invalid level [0 or negative]: %d).\n", GM_account_filename, GM_num+1, line_counter, level);
		else {
			if (level > 99) {
				ShowMessage("read_gm_account: file [%s] %dth account (invalid level, but corrected: %d->99).\n", GM_account_filename, GM_num+1, level);
				level = 99;
			}
			if (is_range) {
				if (start_range==end_range)
					ShowMessage("read_gm_account: file [%s] invalid range, beginning of range is equal to end of range (line #%d).\n", GM_account_filename, line_counter);
				else if (start_range>end_range)
					ShowMessage("read_gm_account: file [%s] invalid range, beginning of range must be lower than end of range (line #%d).\n", GM_account_filename, line_counter);
				else
					for (current_id = start_range;current_id<=end_range;current_id++)
						addGM(current_id,level);
			} else {
				addGM(account_id,level);
			}
		}
	}
	fclose(fp);

	ShowMessage("read_gm_account: file '%s' read (%d GM accounts found).\n", GM_account_filename, GM_num);
	login_log("read_gm_account: file '%s' read (%d GM accounts found)." RETCODE, GM_account_filename, GM_num);

	return 0;
}

//--------------------------------------------------------------
// Test of the IP mask
// (ip: IP to be tested, str: mask x.x.x.x/# or x.x.x.x/y.y.y.y)
//--------------------------------------------------------------
int check_ipmask(unsigned long ip, const unsigned char *str) {
	unsigned int mask = 0, i = 0, m, ip2, a0, a1, a2, a3;
	unsigned char *p = (unsigned char *)&ip2, *p2 = (unsigned char *)&mask;

	if (sscanf((char *)str, "%d.%d.%d.%d/%n", &a0, &a1, &a2, &a3, &i) != 4 || i == 0)
		return 0;
	p[0] = a0; p[1] = a1; p[2] = a2; p[3] = a3;

	if (sscanf((char *)(str+i), "%d.%d.%d.%d", &a0, &a1, &a2, &a3) == 4) {
		p2[0] = a0; p2[1] = a1; p2[2] = a2; p2[3] = a3;
		mask = ntohl(mask);
	} else if (sscanf((char *)(str+i), "%d", &m) == 1 && m >= 0 && m <= 32) {
		for(i = 0; i < m && i < 32; i++)
			mask = (mask >> 1) | 0x80000000;
	} else {
		ShowMessage("check_ipmask: invalid mask [%s].\n", str);
		return 0;
	}

//	ShowMessage("Tested IP: %08x, network: %08x, network mask: %08x\n",
//	       (unsigned int)ntohl(ip), (unsigned int)ntohl(ip2), (unsigned int)mask);
	return ((ntohl(ip) & mask) == (ntohl(ip2) & mask));
}

//---------------------
// Access control by IP
//---------------------
int check_ip(unsigned long ip) {
	int i;
	unsigned char *p = (unsigned char *)&ip;
	char buf[16];
	char * access_ip;
	enum { ACF_DEF, ACF_ALLOW, ACF_DENY } flag = ACF_DEF;

	if (access_allownum == 0 && access_denynum == 0)
		return 1; // When there is no restriction, all IP are authorised.

//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	sprintf(buf, "%d.%d.%d.%d.", p[0], p[1], p[2], p[3]);

	for(i = 0; i < access_allownum; i++) {
		access_ip = access_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			if(access_order == ACO_ALLOW_DENY)
				return 1; // With 'allow, deny' (deny if not allow), allow has priority
			flag = ACF_ALLOW;
			break;
		}
	}

	for(i = 0; i < access_denynum; i++) {
		access_ip = access_deny + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			//flag = ACF_DENY; // not necessary to define flag
			return 0; // At this point, if it's 'deny', we refuse connection.
		}
	}

	return (flag == ACF_ALLOW || access_order == ACO_DENY_ALLOW) ? 1:0;
		// With 'mutual-failture', only 'allow' and non 'deny' IP are authorised.
		//   A non 'allow' (even non 'deny') IP is not authorised. It's like: if allowed and not denied, it's authorised.
		//   So, it's disapproval if you have no description at the time of 'mutual-failture'.
		// With 'deny,allow' (allow if not deny), because here it's not deny, we authorise.
}

//--------------------------------
// Access control by IP for ladmin
//--------------------------------
int check_ladminip(unsigned long ip) {
	int i;
	unsigned char *p = (unsigned char *)&ip;
	char buf[16];
	char * access_ip;

	if (access_ladmin_allownum == 0)
		return 1; // When there is no restriction, all IP are authorised.

//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	sprintf(buf, "%d.%d.%d.%d.", p[0], p[1], p[2], p[3]);

	for(i = 0; i < access_ladmin_allownum; i++) {
		access_ip = access_ladmin_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			return 1;
		}
	}

	return 0;
}

//-----------------------------------------------
// Search an account id
//   (return account index or -1 (if not found))
//   If exact account name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 account is found
//   and similar to the searched name.
//-----------------------------------------------
int search_account_index(char* account_name) {
	int i, quantity, index;

	quantity = 0;
	index = -1;

	for(i = 0; i < auth_num; i++) {
		// Without case sensitive check (increase the number of similar account names found)
		if (strcasecmp(auth_dat[i].userid, account_name) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(auth_dat[i].userid, account_name) == 0)
				return i;
			quantity++;
			index = i;
		}
	}
	// Here, the exact account name is not found
	// We return the found index of a similar account ONLY if there is 1 similar account
	if (quantity == 1)
		return index;

	// Exact account name is not found and 0 or more than 1 similar accounts have been found ==> we say not found
	return -1;
}

//--------------------------------------------------------
// Create a string to save the account in the account file
//--------------------------------------------------------
int mmo_auth_tostr(char *str, struct auth_dat *p) {
	int i;
	char *str_p = str;

	str_p += sprintf(str_p, "%ld\t%s\t%s\t%s\t%c\t%ld\t%ld\t"
	                 "%s\t%s\t%ld\t%s\t%s\t%ld\t",
	                 p->account_id, p->userid, p->pass, p->lastlogin,
	                 (p->sex == 2) ? 'S' : (p->sex ? 'M' : 'F'),
	                 p->logincount, p->state,
							p->email, p->error_message, (unsigned long)p->connect_until_time,
							p->last_ip, p->memo, (unsigned long)p->ban_until_time);

	for(i = 0; i < p->account_reg2_num; i++)
		if (p->account_reg2[i].str[0])
			str_p += sprintf(str_p, "%s,%ld ", p->account_reg2[i].str, p->account_reg2[i].value);

	return 0;
}

//---------------------------------
// Reading of the accounts database
//---------------------------------
int mmo_auth_init(void) {
	FILE *fp;
	unsigned long account_id;
	int logincount, state, n, i, j, v;
	char line[2048], *p, userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
	unsigned long ban_until_time;
	unsigned long connect_until_time;
	char str[2048];
	int GM_count = 0;
	int server_count = 0;

	auth_max = 256;
	auth_dat = (struct auth_dat*)aCalloc(auth_max, sizeof(struct auth_dat));

	if ((fp = savefopen(account_filename, "r")) == NULL) {
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowMessage(CL_BT_RED"mmo_auth_init: Accounts file [%s] not found.\n"CL_RESET, account_filename);
		return 0;
	}

	while(fgets(line, sizeof(line)-1, fp) != NULL) {
		if( !skip_empty_line(line) )
			continue;
		line[sizeof(line)-1] = '\0';
		// remove carriage return if exist
		while(line[0] != '\0' && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'))
			line[strlen(line)-1] = '\0';
		p = line;

		memset(userid, 0, sizeof(userid));
		memset(pass, 0, sizeof(pass));
		memset(lastlogin, 0, sizeof(lastlogin));
		memset(email, 0, sizeof(email));
		memset(error_message, 0, sizeof(error_message));
		memset(last_ip, 0, sizeof(last_ip));
		memset(memo, 0, sizeof(memo));

		// database version reading (v2)
		if (((i = sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n)) == 13 && line[n] == '\t') ||
		    ((i = sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &n)) == 12 && line[n] == '\t')) {
			n = n + 1;

			// Some checks
			if (account_id > END_ACCOUNT_NUM) {
				ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowMessage("               account id #%d -> account not read (saved in log file).\n"CL_RESET, account_id);
				login_log("mmmo_auth_init: ******Error: an account has an id higher than %d." RETCODE, END_ACCOUNT_NUM);
				login_log("               account id #%d -> account not read (saved in next line):" RETCODE, account_id);
				login_log("%s", line);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars(userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: an account has an identical id to another.\n");
					ShowMessage("               account id #%d -> new account not read (saved in log file).\nCL_RESET", account_id);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: account name already exists.\n");
					ShowMessage("               account name '%s' -> new account not read.\n", userid); // 2 lines, account name can be long.
					ShowMessage("               Account saved in log file.\n"CL_RESET);
					login_log("mmmo_auth_init: ******Error: an account has an identical name to another." RETCODE);
					login_log("               account name '%s' -> new account not read (saved in next line):" RETCODE, userid);
					login_log("%s", line);
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = (struct auth_dat*)aRealloc(auth_dat, auth_max*sizeof(struct auth_dat));
				memset(auth_dat+(auth_max-256), 0, 256*sizeof(struct auth_dat) );
			}

			auth_dat[auth_num].account_id = account_id;

			memcpy(auth_dat[auth_num].userid, userid, 24);
			auth_dat[auth_num].userid[24-1]='\0';

			pass[23] = '\0';
			remove_control_chars(pass);
			memcpy(auth_dat[auth_num].pass, pass, 24);
			auth_dat[auth_num].pass[24-1]='\0';

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			memcpy(auth_dat[auth_num].lastlogin, lastlogin, 24);
			auth_dat[auth_num].lastlogin[24-1]='\0';

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');

			if (logincount >= 0)
				auth_dat[auth_num].logincount = logincount;
			else
				auth_dat[auth_num].logincount = 0;

			if (state > 255)
				auth_dat[auth_num].state = 100;
			else if (state < 0)
				auth_dat[auth_num].state = 0;
			else
				auth_dat[auth_num].state = state;

			if (e_mail_check(email) == 0) {
				ShowMessage("Account %s (%d): invalid e-mail (replaced par a@a.com).\n", auth_dat[auth_num].userid, auth_dat[auth_num].account_id);
				safestrcpy(auth_dat[auth_num].email, "a@a.com", 40);
			} else {
				remove_control_chars(email);
				safestrcpy(auth_dat[auth_num].email, email, 40);
			}

			error_message[19] = '\0';
			remove_control_chars(error_message);
			if (error_message[0] == '\0' || state != 7) { // 7, because state is packet 0x006a value + 1
				safestrcpy(auth_dat[auth_num].error_message, "-", 20);
			} else {
				safestrcpy(auth_dat[auth_num].error_message, error_message, 20);
			}

			if (i == 13)
				auth_dat[auth_num].ban_until_time = ban_until_time;
			else
				auth_dat[auth_num].ban_until_time = 0;

			auth_dat[auth_num].connect_until_time = connect_until_time;

			last_ip[15] = '\0';
			remove_control_chars(last_ip);
			safestrcpy(auth_dat[auth_num].last_ip, last_ip, 16);

			memo[254] = '\0';
			remove_control_chars(memo);
			safestrcpy(auth_dat[auth_num].memo, memo, 255);

			for(j = 0; j < ACCOUNT_REG2_NUM; j++) {
				p += n;
				if (sscanf(p, "%[^\t,],%d %n", str, &v, &n) != 2) {
					// We must check if a str is void. If it's, we can continue to read other REG2.
					// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
					if (p[0] == ',' && sscanf(p, ",%d %n", &v, &n) == 1) {
						j--;
						continue;
					} else
						break;
				}
				str[31] = '\0';
				remove_control_chars(str);
				safestrcpy(auth_dat[auth_num].account_reg2[j].str, str, 32);
				auth_dat[auth_num].account_reg2[j].value = v;
			}
			auth_dat[auth_num].account_reg2_num = j;

			if (isGM(account_id) > 0)
				GM_count++;
			if (auth_dat[auth_num].sex == 2)
				server_count++;

			auth_num++;
			if (account_id >= account_id_count)
				account_id_count = account_id + 1;

		// Old athena database version reading (v1)
		} else if ((i = sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%n",
		           &account_id, userid, pass, lastlogin, &sex, &logincount, &state, &n)) >= 5) {
			if (account_id > END_ACCOUNT_NUM) {
				ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowMessage("               account id #%d -> account not read (saved in log file).\n"CL_RESET, account_id);
				login_log("mmmo_auth_init: ******Error: an account has an id higher than %d." RETCODE, END_ACCOUNT_NUM);
				login_log("               account id #%d -> account not read (saved in next line):" RETCODE, account_id);
				login_log("%s", line);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars(userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: an account has an identical id to another.\n");
					ShowMessage("               account id #%d -> new account not read (saved in log file).\n"CL_RESET, account_id);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowMessage(CL_BT_RED"mmo_auth_init: ******Error: account name already exists.\n");
					ShowMessage("               account name '%s' -> new account not read.\n", userid); // 2 lines, account name can be long.
					ShowMessage("               Account saved in log file.\n"CL_RESET);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = (struct auth_dat*)aRealloc(auth_dat, auth_max*sizeof(struct auth_dat) );
				memset(auth_dat+(auth_max-256), 0, 256*sizeof(struct auth_dat) );
			}

			auth_dat[auth_num].account_id = account_id;

			safestrcpy(auth_dat[auth_num].userid, userid, 24);

			pass[23] = '\0';
			remove_control_chars(pass);
			safestrcpy(auth_dat[auth_num].pass, pass, 24);

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			safestrcpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');

			if (i >= 6) {
				if (logincount >= 0)
					auth_dat[auth_num].logincount = logincount;
				else
					auth_dat[auth_num].logincount = 0;
			} else
				auth_dat[auth_num].logincount = 0;

			if (i >= 7) {
				if (state > 255)
					auth_dat[auth_num].state = 100;
				else if (state < 0)
					auth_dat[auth_num].state = 0;
				else
					auth_dat[auth_num].state = state;
			} else
				auth_dat[auth_num].state = 0;

			// Initialization of new data
			safestrcpy(auth_dat[auth_num].email, "a@a.com", 40);
			safestrcpy(auth_dat[auth_num].error_message, "-", 20);
			auth_dat[auth_num].ban_until_time = 0;
			auth_dat[auth_num].connect_until_time = 0;
			safestrcpy(auth_dat[auth_num].last_ip, "-", 16);
			safestrcpy(auth_dat[auth_num].memo, "-", 255);

			for(j = 0; j < ACCOUNT_REG2_NUM; j++) {
				p += n;
				if (sscanf(p, "%[^\t,],%d %n", str, &v, &n) != 2) {
					// We must check if a str is void. If it's, we can continue to read other REG2.
					// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
					if (p[0] == ',' && sscanf(p, ",%d %n", &v, &n) == 1) {
						j--;
						continue;
					} else
						break;
				}
				str[31] = '\0';
				remove_control_chars(str);
				safestrcpy(auth_dat[auth_num].account_reg2[j].str, str, 32);
				auth_dat[auth_num].account_reg2[j].value = v;
			}
			auth_dat[auth_num].account_reg2_num = j;

			if (isGM(account_id) > 0)
				GM_count++;
			if (auth_dat[auth_num].sex == 2)
				server_count++;

			auth_num++;
			if (account_id >= account_id_count)
				account_id_count = account_id + 1;

		} else {
			i = 0;
			if (sscanf(line, "%ld\t%%newid%%\n%n", &account_id, &i) == 1 &&
			    i > 0 && account_id > account_id_count)
				account_id_count = account_id;
		}
	}
	fclose(fp);

	if (auth_num == 0) {
		ShowMessage("mmo_auth_init: No account found in %s.\n", account_filename);
		sprintf(line, "No account found in %s.", account_filename);
	} else {
		if (auth_num == 1) {
			ShowMessage("mmo_auth_init: 1 account read in %s,\n", account_filename);
			sprintf(line, "1 account read in %s,", account_filename);
		} else {
			ShowMessage("mmo_auth_init: %d accounts read in %s,\n", auth_num, account_filename);
			sprintf(line, "%d accounts read in %s,", auth_num, account_filename);
		}
		if (GM_count == 0) {
			ShowMessage("               of which is no GM account, and ");
			sprintf(str, "%s of which is no GM account and", line);
		} else if (GM_count == 1) {
			ShowMessage("               of which is 1 GM account, and ");
			sprintf(str, "%s of which is 1 GM account and", line);
		} else {
			ShowMessage("               of which is %d GM accounts, and ", GM_count);
			sprintf(str, "%s of which is %d GM accounts and", line, GM_count);
		}
		if (server_count == 0) {
			ShowMessage("no server account ('S').\n");
			sprintf(line, "%s no server account ('S').", str);
		} else if (server_count == 1) {
			ShowMessage("1 server account ('S').\n");
			sprintf(line, "%s 1 server account ('S').", str);
		} else {
			ShowMessage("%d server accounts ('S').\n", server_count);
			sprintf(line, "%s %d server accounts ('S').", str, server_count);
		}
	}
	login_log("%s" RETCODE, line);

	return 0;
}

//------------------------------------------
// Writing of the accounts database file
//   (accounts are sorted by id before save)
//------------------------------------------
void mmo_auth_sync(void) {
	FILE *fp;
	int i, j, k, lock;
	unsigned long account_id;
	char line[65536];
	CREATE_BUFFER(id, int, auth_num);

	// Sorting before save
	for(i = 0; i < auth_num; i++) {
		id[i] = i;
		account_id = auth_dat[i].account_id;
		for(j = 0; j < i; j++) {
			if (account_id < auth_dat[id[j]].account_id) {
				for(k = i; k > j; k--)
					id[k] = id[k-1];
				id[j] = i; // id[i]
				break;
			}
		}
	}

	// Data save
	if ((fp = lock_fopen(account_filename, &lock)) == NULL) {
		DELETE_BUFFER(id);
		return;
	}
	fprintf(fp, "// Accounts file: here are saved all information about the accounts.\n");
	fprintf(fp, "// Structure: ID, account name, password, last login time, sex, # of logins, state, email, error message for state 7, validity time, last (accepted) login ip, memo field, ban timestamp, repeated(register text, register value)\n");
	fprintf(fp, "// Some explanations:\n");
	fprintf(fp, "//   account name    : between 4 to 23 char for a normal account (standard client can't send less than 4 char).\n");
	fprintf(fp, "//   account password: between 4 to 23 char\n");
	fprintf(fp, "//   sex             : M or F for normal accounts, S for server accounts\n");
	fprintf(fp, "//   state           : 0: account is ok, 1 to 256: error code of packet 0x006a + 1\n");
	fprintf(fp, "//   email           : between 3 to 39 char (a@a.com is like no email)\n");
	fprintf(fp, "//   error message   : text for the state 7: 'Your are Prohibited to login until <text>'. Max 19 char\n");
	fprintf(fp, "//   valitidy time   : 0: unlimited account, <other value>: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
	fprintf(fp, "//   memo field      : max 254 char\n");
	fprintf(fp, "//   ban time        : 0: no ban, <other value>: banned until the date: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
	for(i = 0; i < auth_num; i++) {
		k = id[i]; // use of sorted index
		if (auth_dat[k].account_id < 0)
			continue;

		mmo_auth_tostr(line, &auth_dat[k]);
		fprintf(fp, "%s" RETCODE, line);
	}
	fprintf(fp, "%ld\t%%newid%%\n", account_id_count);

	lock_fclose(fp, account_filename, &lock);

	// set new counter to minimum number of auth before save
	auth_before_save_file = auth_num / AUTH_SAVE_FILE_DIVIDER; // Re-initialise counter. We have save.
	if (auth_before_save_file < AUTH_BEFORE_SAVE_FILE)
		auth_before_save_file = AUTH_BEFORE_SAVE_FILE;

	DELETE_BUFFER(id);
	return;
}

//-----------------------------------------------------
// Check if we must save accounts file or not
//   every minute, we check if we must save because we
//   have do some authentifications without arrive to
//   the minimum of authentifications for the save.
// Note: all other modification of accounts (deletion,
//       change of some informations excepted lastip/
//       lastlogintime, creation) are always save
//       immediatly and set  the minimum of
//       authentifications to its initialization value.
//-----------------------------------------------------
int check_auth_sync(int tid, unsigned long tick, int id, int data)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if (auth_before_save_file < AUTH_BEFORE_SAVE_FILE ||
	    auth_before_save_file*AUTH_SAVE_FILE_DIVIDER < auth_num )
		mmo_auth_sync();

	return 0;
}

//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c, fd;

	for(i = 0, c = 0; i < MAX_SERVERS; i++)
	{
		fd = server[i].fd;
		if( fd >= 0 && fd != sfd) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}

//-----------------------------------------------------
// Send GM accounts to all char-server
//-----------------------------------------------------
void send_GM_accounts() {
	int i;
	unsigned char buf[32767];
	int len;

	len = 4;
	WBUFW(buf,0) = 0x2732;
	for(i = 0; i < GM_num; i++)
		// send only existing accounts. We can not create a GM account when server is online.
		if (gm_account_db[i].level > 0) {
			WBUFL(buf,len) = gm_account_db[i].account_id;
			WBUFB(buf,len+4) = (unsigned char)gm_account_db[i].level;
			len += 5;
		}
	WBUFW(buf,2) = len;
	charif_sendallwos(-1, (unsigned char*)buf, len);

	return;
}

//-----------------------------------------------------
// Check if GM file account have been changed
//-----------------------------------------------------
int check_GM_file(int tid, unsigned long tick, int id, int data)
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
		new_time = file_stat.st_mtime;

	if (new_time != creation_time_GM_account_file)
	{
		read_gm_account();
		send_GM_accounts();
	}

	return 0;
}

//-------------------------------------
// Account creation (with e-mail check)
//-------------------------------------
int mmo_auth_new(struct mmo_account* account, char sex, char* email) {
	time_t timestamp, timestamp_temp;
	struct tm *tmtime;
	int i = auth_num;

	if (auth_num >= auth_max) {
		auth_max += 256;
		auth_dat = (struct auth_dat*)aRealloc(auth_dat, auth_max*sizeof(struct auth_dat) );
		memset(auth_dat+(auth_max-256), 0, 256*sizeof(struct auth_dat) );
	}

	while (isGM(account_id_count) > 0)
		account_id_count++;

	auth_dat[i].account_id = account_id_count++;

	safestrcpy(auth_dat[i].userid, account->userid, 24);
	auth_dat[i].userid[23] = '\0';

	safestrcpy(auth_dat[i].pass, account->passwd, 32);
	auth_dat[i].pass[23] = '\0';

	memcpy(auth_dat[i].lastlogin, "-", 2);

	auth_dat[i].sex = (sex == 'M');

	auth_dat[i].logincount = 0;

	auth_dat[i].state = 0;

	if (e_mail_check(email) == 0)
		safestrcpy(auth_dat[i].email, "a@a.com", 40);
	else
		safestrcpy(auth_dat[i].email, email, 40);

	safestrcpy(auth_dat[i].error_message, "-", 20);

	auth_dat[i].ban_until_time = 0;

	if (start_limited_time < 0)
		auth_dat[i].connect_until_time = 0; // unlimited
	else { // limited time
		timestamp = time(NULL) + start_limited_time;
		// double conversion to be sure that it is possible
		tmtime = localtime(&timestamp);
		timestamp_temp = mktime(tmtime);
		if (timestamp_temp != -1 && (timestamp_temp + 3600) >= timestamp) // check possible value and overflow (and avoid summer/winter hour)
			auth_dat[i].connect_until_time = timestamp_temp;
		else
			auth_dat[i].connect_until_time = 0; // unlimited
	}

	safestrcpy(auth_dat[i].last_ip, "-", 16);
	safestrcpy(auth_dat[i].memo, "-", 255);

	auth_dat[i].account_reg2_num = 0;

	auth_num++;

	return (account_id_count - 1);
}

//---------------------------------------
// Check/authentification of a connection
//---------------------------------------
int mmo_auth(struct mmo_account* account, int fd)
{
	int i;
	struct timeval tv;
	time_t unixtime;
	char tmpstr[256];
	int len, newaccount = 0;
#ifdef PASSWORDENC
	struct login_session_data *ld;
#endif
	int encpasswdok;
	char md5str[64], md5bin[32];
	char user_password[256];
	char ip_str[16];
	unsigned long client_ip = session[fd]->client_ip;

	sprintf(ip_str, "%ld.%ld.%ld.%ld", (client_ip>>24)&0xFF, (client_ip>>16)&0xFF, (client_ip>>8)&0xFF, (client_ip)&0xFF);

	//EXE Version check [Sirius]
	if( (check_client_version == 1) && 
		(account->version != 0) &&
		(account->version != client_version_to_connect) )
	{
		return 5;
	}

	len = strlen(account->userid) - 2;
	// Account creation with _M/_F
	if (account->passwdenc == 0 && account->userid[len] == '_' &&
	    (account->userid[len+1] == 'F' || account->userid[len+1] == 'M') && new_account_flag == 1 &&
	    account_id_count <= END_ACCOUNT_NUM && len >= 4 && strlen(account->passwd) >= 4) {
						
		// only continue if amount in this time limit is allowed
		if(gettick() <= new_reg_tick && num_regs >= allowed_regs) {
			ShowMessage("Notice: Account registration in disallowed time from: %s!", ip_str);
			login_log("Notice: Account registration in disallowed time from: %s!", ip_str);
			return 3;
		} else {
			num_regs=0;
		}
		
			newaccount = 1;
		account->userid[len] = '\0';
	}
	
	//EXE Version check [Sirius]
	if (check_client_version == 1 && account->version != 0 &&
		account->version != client_version_to_connect)
		return 5;
	
	// Strict account search
	for(i = 0; i < auth_num; i++) {
		if (strcmp(account->userid, auth_dat[i].userid) == 0)
			break;
	}

	// if there is no creation request and strict account search fails, 
	// we do a no sensitive case research for index
	if (newaccount == 0 && i == auth_num) {
		i = search_account_index(account->userid);
		if (i == -1)
			i = auth_num;
		else
			memcpy(account->userid, auth_dat[i].userid, 24); // for the possible tests/checks afterwards (copy correcte sensitive case).
	}

	if (i != auth_num) {
		if (newaccount) {
			login_log("Attempt of creation of an already existant account (account: %s_%c, pass: %s, received pass: %s, ip: %s)" RETCODE,
			          account->userid, account->userid[len+1], auth_dat[i].pass, account->passwd, ip_str);
			return 1; // 1 = Incorrect Password
		}
		if(use_md5_passwds)
			MD5_String(account->passwd, user_password);
		else
			memcpy(user_password, account->passwd, 25);
		encpasswdok = 0;
#ifdef PASSWORDENC
		ld = (struct login_session_data*)session[fd]->session_data;
		if (account->passwdenc > 0) {
			int j = account->passwdenc;
			if (!ld) {
				login_log("Md5 key not created (account: %s, ip: %s)" RETCODE, account->userid, ip_str);
				return 1; // 1 = Incorrect Password
			}
			if (j > 2)
				j = 1;
			do {
				if (j == 1) {
					sprintf(md5str, "%s%s", ld->md5key, auth_dat[i].pass); // 20 + 24
				} else if (j == 2) {
					sprintf(md5str, "%s%s", auth_dat[i].pass, ld->md5key); // 24 + 20
				} else
					md5str[0] = '\0';
				md5str[sizeof(md5str)-1] = '\0'; // 64
				MD5_String2binary(md5str, md5bin);
				encpasswdok = (memcmp(account->passwd, md5bin, 16) == 0);
			} while (j < 2 && !encpasswdok && (j++) != account->passwdenc);
//			ShowMessage("key[%s] md5 [%s] ", md5key, md5);
//			ShowMessage("client [%s] accountpass [%s]\n", account->passwd, auth_dat[i].pass);
		}
#endif
		if ((strcmp(account->passwd, auth_dat[i].pass) && !encpasswdok)) {
			if (account->passwdenc == 0)
				login_log("Invalid password (account: %s, pass: %s, received pass: %s, ip: %s)" RETCODE, account->userid, auth_dat[i].pass, account->passwd, ip_str);
#ifdef PASSWORDENC
			else {
				char logbuf[512], *p = logbuf;
				int j;
				p += sprintf(p, "Invalid password (account: %s, received md5[", account->userid);
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)account->passwd)[j]);
				p += sprintf(p,"] calculated md5[");
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)md5bin)[j]);
				p += sprintf(p, "] md5 key[");
				for(j = 0; j < ld->md5keylen; j++)
					p += sprintf(p, "%02x", ((unsigned char *)ld->md5key)[j]);
				p += sprintf(p, "], ip: %s)" RETCODE, ip_str);
				login_log(logbuf);
			}
#endif
			return 1; // 1 = Incorrect Password
		}

		if (auth_dat[i].state) {
			login_log("Connection refused (account: %s, pass: %s, state: %d, ip: %s)" RETCODE,
			          account->userid, account->passwd, auth_dat[i].state, ip_str);
			switch(auth_dat[i].state) { // packet 0x006a value + 1
			case 1:   // 0 = Unregistered ID
			case 2:   // 1 = Incorrect Password
			case 3:   // 2 = This ID is expired
			case 4:   // 3 = Rejected from Server
			case 5:   // 4 = You have been blocked by the GM Team
			case 6:   // 5 = Your Game's EXE file is not the latest version
			case 7:   // 6 = Your are Prohibited to log in until %s
			case 8:   // 7 = Server is jammed due to over populated
			case 9:   // 8 = No MSG (actually, all states after 9 except 99 are No MSG, use only this)
			case 100: // 99 = This ID has been totally erased
				return auth_dat[i].state - 1;
			default:
				return 99; // 99 = ID has been totally erased
			}
		}

		if (is_user_online(auth_dat[i].account_id)) {
			return 3; // Rejected
		}

		if (auth_dat[i].ban_until_time != 0) { // if account is banned
			strftime(tmpstr, 20, date_format, localtime(&auth_dat[i].ban_until_time));
			tmpstr[19] = '\0';
			if (auth_dat[i].ban_until_time > time(NULL)) { // always banned
				login_log("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)" RETCODE,
				          account->userid, account->passwd, tmpstr, ip_str);
				return 6; // 6 = Your are Prohibited to log in until %s
			} else { // ban is finished
				login_log("End of ban (account: %s, pass: %s, previously banned until %s -> not more banned, ip: %s)" RETCODE,
				          account->userid, account->passwd, tmpstr, ip_str);
				auth_dat[i].ban_until_time = 0; // reset the ban time
			}
		}

		if (auth_dat[i].connect_until_time != 0 && auth_dat[i].connect_until_time < time(NULL)) {
			login_log("Connection refused (account: %s, pass: %s, expired ID, ip: %s)" RETCODE,
			          account->userid, account->passwd, ip_str);
			return 2; // 2 = This ID is expired
		}

		login_log("Authentification accepted (account: %s (id: %d), ip: %s)" RETCODE, account->userid, auth_dat[i].account_id, ip_str);
	} else {
		if (newaccount == 0) {
			login_log("Unknown account (account: %s, received pass: %s, ip: %s)" RETCODE,
			          account->userid, account->passwd, ip_str);
			return 0; // 0 = Unregistered ID
		} else {
			int new_id = mmo_auth_new(account, account->userid[len+1], "a@a.com");
			login_log("Account creation and authentification accepted (account %s (id: %d), pass: %s, sex: %c, connection with _F/_M, ip: %s)" RETCODE,
			          account->userid, new_id, account->passwd, account->userid[len+1], ip_str);
			auth_before_save_file = 0; // Creation of an account -> save accounts file immediatly
			//restart ticker
			if(num_regs==0) {
				new_reg_tick=gettick()+time_allowed*1000;
			}
			num_regs++;
		}
	}

	gettimeofday(&tv, NULL);
	unixtime=tv.tv_sec;
	strftime(tmpstr, 24, date_format, localtime(&unixtime));
	sprintf(tmpstr + strlen(tmpstr), ".%03ld", tv.tv_usec / 1000);

	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	memcpy(auth_dat[i].lastlogin, tmpstr, 24);
	account->sex = auth_dat[i].sex;
	safestrcpy(auth_dat[i].last_ip, ip_str, 16);
	auth_dat[i].logincount++;

	// Save until for change ip/time of auth is not very useful => limited save for that
	// Save there informations isnot necessary, because they are saved in log file.
	if (--auth_before_save_file <= 0) // Reduce counter. 0 or less, we save
		mmo_auth_sync();

	return -1; // account OK
}



//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd) {
	int i, j, id;
	unsigned long acc;
	char ip_str[16];
	unsigned long client_ip = session[fd]->client_ip;
	sprintf(ip_str, "%ld.%ld.%ld.%ld", (client_ip>>24)&0xFF, (client_ip>>16)&0xFF, (client_ip>>8)&0xFF, (client_ip)&0xFF);


	for(id = 0; id < MAX_SERVERS; id++)
		if (server[id].fd == fd)
			break;
	if (id == MAX_SERVERS) {	
		session_Remove(fd);
		return 0;
		}
	// else it is char server id
	if( !session_isActive(fd) ) {
		// check if a char server is disconnecting
		ShowMessage("Char-server '%s' has disconnected.\n", server[id].name);
		login_log("Char-server '%s' has disconnected (ip: %s)." RETCODE,server[id].name, ip_str);
		server[id].fd = -1;
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}


	while (RFIFOREST(fd) >= 2) {
		if (display_parse_fromchar == 2 || (display_parse_fromchar == 1 && RFIFOW(fd,0) != 0x2714)) // 0x2714 is done very often (number of players)
			ShowMessage("parse_fromchar: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));

		switch (RFIFOW(fd,0)) {
		// request from map-server via char-server to reload GM accounts (by Yor).
		case 0x2709:
			login_log("Char-server '%s': Request to re-load GM configuration file (ip: %s)." RETCODE, server[id].name, ip_str);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts();
			RFIFOSKIP(fd,2);
			break;

		case 0x2712: // request from char-server to authentify an account
			if (RFIFOREST(fd) < 19)
				return 0;
		  {
			unsigned long acc = RFIFOL(fd,2);
			for(i = 0; i < AUTH_FIFO_SIZE; i++) 
			{
				if (auth_fifo[i].account_id == acc &&
				    auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				    auth_fifo[i].login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18
#endif
				    auth_fifo[i].sex == RFIFOB(fd,14) &&
				    (!check_ip_flag || auth_fifo[i].ip == RFIFOLIP(fd,15)) &&
				    !auth_fifo[i].delflag) 
				{
					int p, k;
					auth_fifo[i].delflag = 1;
					login_log("Char-server '%s': authentification of the account %d accepted (ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
					//ShowMessage("%d\n", i);

					for(k = 0; k < auth_num; k++) {
						if (auth_dat[k].account_id == acc) {
							WFIFOW(fd,0) = 0x2729;	// Sending of the account_reg2
							WFIFOL(fd,4) = acc;
							for(p = 8, j = 0; j < auth_dat[k].account_reg2_num; p += 36, j++) {
								memcpy(WFIFOP(fd,p), auth_dat[k].account_reg2[j].str, 32);
								WFIFOL(fd,p+32) = auth_dat[k].account_reg2[j].value;
							}
							WFIFOW(fd,2) = p;
							WFIFOSET(fd,p);
//							ShowMessage("parse_fromchar: Sending of account_reg2: login->char (auth fifo)\n");
							WFIFOW(fd,0) = 0x2713;
							WFIFOL(fd,2) = acc;
							WFIFOB(fd,6) = 0;
							memcpy(WFIFOP(fd, 7), auth_dat[k].email, 40);
							WFIFOL(fd,47) = (unsigned long)auth_dat[k].connect_until_time;
							WFIFOSET(fd,51);
							break;
						}
					}
					break;
				}
			}
			// authentification not found
			if (i == AUTH_FIFO_SIZE) {
				login_log("Char-server '%s': authentification of the account %d REFUSED (ip: %s)." RETCODE,
				          server[id].name, acc, ip_str);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = acc;
				WFIFOB(fd,6) = 1;
				// It is unnecessary to send email
				// It is unnecessary to send validity date of the account
				WFIFOSET(fd,51);
			}
		  }
			RFIFOSKIP(fd,19);
			break;

		case 0x2714:
			if (RFIFOREST(fd) < 6)
				return 0;
			//ShowMessage("parse_fromchar: Receiving of the users number of the server '%s': %ld\n", server[id].name, (unsigned long)RFIFOL(fd,2));
			server[id].users = RFIFOL(fd,2);

			// send some answer
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);

			RFIFOSKIP(fd,6);
			break;

		// we receive a e-mail creation of an account with a default e-mail (no answer)
		case 0x2715:
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			char email[40];
			acc = RFIFOL(fd,2); // speed up
			memcpy(email, RFIFOP(fd,6), 40);
			email[39] = '\0';
			remove_control_chars(email);
			//ShowMessage("parse_fromchar: an e-mail creation of an account with a default e-mail: server '%s', account: %d, e-mail: '%s'.\n", server[id].name, acc, RFIFOP(fd,6));
			if (e_mail_check(email) == 0)
				login_log("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - e-mail is invalid (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip_str);
			else {
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc && (strcmp(auth_dat[i].email, "a@a.com") == 0 || auth_dat[i].email[0] == '\0')) {
						memcpy(auth_dat[i].email, email, 40);
						login_log("Char-server '%s': Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s)." RETCODE,
						          server[id].name, acc, email, ip_str);
						// Save
						mmo_auth_sync();
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - account doesn't exist or e-mail of account isn't default e-mail (account: %d, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
			}
		}
			RFIFOSKIP(fd,46);
			break;

		// We receive an e-mail/limited time request, because a player comes back from a map-server to the char-server
		case 0x2716:
			if (RFIFOREST(fd) < 6)
				return 0;
			//ShowMessage("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %ld)\n", server[id].name, (unsigned long)RFIFOL(fd,2));
			for(i = 0; i < auth_num; i++)
			{
				if (auth_dat[i].account_id == RFIFOL(fd,2))
				{
					login_log("Char-server '%s': e-mail of the account %d found (ip: %s)." RETCODE,
					          server[id].name, (unsigned long)RFIFOL(fd,2), ip_str);
					WFIFOW(fd,0) = 0x2717;
					WFIFOL(fd,2) = RFIFOL(fd,2);
					memcpy(WFIFOP(fd, 6), auth_dat[i].email, 40);
					WFIFOL(fd,46) = (unsigned long)auth_dat[i].connect_until_time;
					WFIFOSET(fd,50);
					break;
				}
			}
			if (i == auth_num)
				login_log("Char-server '%s': e-mail of the account %d NOT found (ip: %s)." RETCODE,
				          server[id].name, (unsigned long)RFIFOL(fd,2), ip_str);
			RFIFOSKIP(fd,6);
			break;

		// login-server alive packet reply
		case 0x2718:
			if (RFIFOREST(fd) < 2)
				return 0;
			// do whatever it's supposed to do here?
			RFIFOSKIP(fd,2);
			break;

		case 0x2720:	// To become GM request
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < (int)RFIFOW(fd,2))
				return 0;
			  {
				unsigned char buf[10];
				FILE *fp;
				acc = RFIFOL(fd,4);
				//ShowMessage("parse_fromchar: Request to become a GM acount from %d account.\n", acc);
				WBUFW(buf,0) = 0x2721;
				WBUFL(buf,2) = acc;
				WBUFL(buf,6) = 0;
				if (strcmp((char*)RFIFOP(fd,8), gm_pass) == 0) {
					// only non-GM can become GM
					if (isGM(acc) == 0) {
						// if we autorise creation
						if (level_new_gm > 0) {
							// if we can open the file to add the new GM
							if ((fp = savefopen(GM_account_filename, "a")) != NULL) {
								char tmpstr[24];
								time_t unixtime = time(NULL);
								strftime(tmpstr, 23, date_format, localtime(&unixtime));
								fprintf(fp, RETCODE "// %s: @GM command on account %ld" RETCODE "%ld %d" RETCODE, tmpstr, acc, acc, level_new_gm);
								fclose(fp);
								WBUFL(buf,6) = level_new_gm;
								read_gm_account();
								send_GM_accounts();
								ShowMessage("GM Change of the account %d: level 0 -> %d.\n", acc, level_new_gm);
								login_log("Char-server '%s': GM Change of the account %d: level 0 -> %d (ip: %s)." RETCODE,
								          server[id].name, acc, level_new_gm, ip_str);
							} else {
								ShowMessage("Error of GM change (suggested account: %d, correct password, unable to add a GM account in GM accounts file)\n", acc);
								login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, unable to add a GM account in GM accounts file, ip: %s)." RETCODE,
								          server[id].name, acc, ip_str);
							}
						} else {
							ShowMessage("Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0))\n", acc);
							login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0), ip: %s)." RETCODE,
							          server[id].name, acc, ip_str);
						}
					} else {
						ShowMessage("Error of GM change (suggested account: %d (already GM), correct password).\n", acc);
						login_log("Char-server '%s': Error of GM change (suggested account: %d (already GM), correct password, ip: %s)." RETCODE,
						          server[id].name, acc, ip_str);
					}
				} else {
					ShowMessage("Error of GM change (suggested account: %d, invalid password).\n", acc);
					login_log("Char-server '%s': Error of GM change (suggested account: %d, invalid password, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
				}
				charif_sendallwos(-1, buf, 10);
			  }
			RFIFOSKIP(fd, RFIFOW(fd,2));
			break;;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		  {
			char actual_email[40], new_email[40];
			acc = RFIFOL(fd,2);
			memcpy(actual_email, RFIFOP(fd,6), 40);
			actual_email[39] = '\0';
			remove_control_chars(actual_email);
			memcpy(new_email, RFIFOP(fd,46), 40);
			new_email[39] = '\0';
			remove_control_chars(new_email);
			if (e_mail_check(actual_email) == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip_str);
			else if (e_mail_check(new_email) == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip_str);
			else if (strcasecmp(new_email, "a@a.com") == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip_str);
			else {
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (strcasecmp(auth_dat[i].email, actual_email) == 0) {
							memcpy(auth_dat[i].email, new_email, 40);
							login_log("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].userid, new_email, ip_str);
							// Save
							mmo_auth_sync();
						} else
							login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].userid, auth_dat[i].email, actual_email, ip_str);
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
			}
		  }
			RFIFOSKIP(fd, 86);
			break;

		// Receiving of map-server via char-server a status change resquest (by Yor)
		case 0x2724:
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				unsigned long acc, statut;
				acc = RFIFOL(fd,2);
				statut = RFIFOL(fd,6);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].state != statut) {
							login_log("Char-server '%s': Status change (account: %d, new status %d, ip: %s)." RETCODE,
							          server[id].name, acc, statut, ip_str);
							if (statut != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = acc;
								WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
								WBUFL(buf,7) = statut; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
								for(j = 0; j < AUTH_FIFO_SIZE; j++)
									if (auth_fifo[j].account_id == acc)
										auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							}
							auth_dat[i].state = statut;
							// Save
							mmo_auth_sync();
						} else
							login_log("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s)." RETCODE,
							          server[id].name, acc, statut, ip_str);
						break;
					}
				}
				if (i == auth_num) {
					login_log("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s)." RETCODE,
					          server[id].name, acc, statut, ip_str);
				}
				RFIFOSKIP(fd,10);
			}
			break;

		case 0x2725:	// Receiving a ban request from map-server via char-server (by Yor)
			if (RFIFOREST(fd) < 18)
				return 0;
			{
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						time_t timestamp;
						struct tm *tmtime;
						if (auth_dat[i].ban_until_time == 0 || auth_dat[i].ban_until_time < time(NULL))
							timestamp = time(NULL);
						else
							timestamp = auth_dat[i].ban_until_time;
						tmtime = localtime(&timestamp);
						tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
						tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
						tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
						tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
						tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
						tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);
						timestamp = mktime(tmtime);
						if (timestamp != -1) {
							if (timestamp <= time(NULL))
								timestamp = 0;
							if (auth_dat[i].ban_until_time != timestamp) {
								if (timestamp != 0) {
									unsigned char buf[16];
									char tmpstr[2048];
									strftime(tmpstr, 24, date_format, localtime(&timestamp));
									login_log("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s)." RETCODE,
									          server[id].name, acc, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
									WBUFW(buf,0) = 0x2731;
									WBUFL(buf,2) = auth_dat[i].account_id;
									WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
									WBUFL(buf,7) = timestamp; // status or final date of a banishment
									charif_sendallwos(-1, buf, 11);
									for(j = 0; j < AUTH_FIFO_SIZE; j++)
										if (auth_fifo[j].account_id == acc)
											auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
								} else {
									login_log("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s)." RETCODE,
									          server[id].name, acc, ip_str);
								}
								auth_dat[i].ban_until_time = timestamp;
								// Save
								mmo_auth_sync();
							} else {
								login_log("Char-server '%s': Error of ban request (account: %d, no change for ban date, ip: %s)." RETCODE,
								          server[id].name, acc, ip_str);
							}
						} else {
							login_log("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s)." RETCODE,
							          server[id].name, acc, ip_str);
						}
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Error of ban request (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
				RFIFOSKIP(fd,18);
			}
			break;

		case 0x2727:	// Change of sex (sex is reversed)
			if (RFIFOREST(fd) < 6)
				return 0;
			{
				int sex;
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
//					ShowMessage("%d,", auth_dat[i].account_id);
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].sex == 2)
							login_log("Char-server '%s': Error of sex change - Server account (suggested account: %d, actual sex %d (Server), ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].sex, ip_str);
						else {
							unsigned char buf[16];
							if (auth_dat[i].sex == 0)
								sex = 1;
							else
								sex = 0;
							login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
							          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip_str);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == acc)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							auth_dat[i].sex = sex;
							WBUFW(buf,0) = 0x2723;
							WBUFL(buf,2) = acc;
							WBUFB(buf,6) = sex;
							charif_sendallwos(-1, buf, 7);
							// Save
							mmo_auth_sync();
						}
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Error of sex change (account: %d not found, sex would be reversed, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
				RFIFOSKIP(fd,6);
			}
			break;

		case 0x2728:	// We receive account_reg2 from a char-server, and we send them to other char-servers.
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				int p;
				CREATE_BUFFER(buf,unsigned char,RFIFOW(fd,2)+1);

				acc = RFIFOL(fd,4);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						login_log("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d, ip: %s)." RETCODE,
						          server[id].name, acc, ip_str);
						for(p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, j++) {
							memcpy(auth_dat[i].account_reg2[j].str, RFIFOP(fd,p), 32);
							auth_dat[i].account_reg2[j].str[31] = '\0';
							remove_control_chars((auth_dat[i].account_reg2[j].str));
							auth_dat[i].account_reg2[j].value = RFIFOL(fd,p+32);
						}
						auth_dat[i].account_reg2_num = j;
						// Sending information towards the other char-servers.
						memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
						WBUFW(buf,0) = 0x2729;
						charif_sendallwos(fd, buf, WBUFW(buf,2));
						// Save
						mmo_auth_sync();
//						ShowMessage("parse_fromchar: receiving (from the char-server) of account_reg2 (account id: %d).\n", acc);
						break;
					}
				}
				if (i == auth_num) {
//					ShowMessage("parse_fromchar: receiving (from the char-server) of account_reg2 (unknwon account id: %d).\n", acc);
					login_log("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
				}
				DELETE_BUFFER(buf);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		case 0x272a:	// Receiving of map-server via char-server a unban resquest (by Yor)
			if (RFIFOREST(fd) < 6)
				return 0;
			{
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].ban_until_time != 0) {
							auth_dat[i].ban_until_time = 0;
							login_log("Char-server '%s': UnBan request (account: %d, ip: %s)." RETCODE,
							          server[id].name, acc, ip_str);
						} else {
							login_log("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s)." RETCODE,
							          server[id].name, acc, ip_str);
						}
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip_str);
				RFIFOSKIP(fd,6);
			}
			break;


		case 0x272b:    // Set account_id to online [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			add_online_user(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
			break;
		
		case 0x272c:   // Set account_id to offline [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			remove_online_user(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
			break;

		case 0x3000: //change sex for chrif_changesex()
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				int sex, i = 0;
				acc = RFIFOL(fd,4);
				sex = RFIFOB(fd,8);
				if (sex != 0 && sex != 1)
					sex = 0;
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						unsigned char buf[16];
						login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
						          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip_str);
						auth_fifo[i].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						auth_dat[i].sex = sex;
						WBUFW(buf,0) = 0x2723;
						WBUFL(buf,2) = acc;
						WBUFB(buf,6) = sex;
						charif_sendallwos(-1, buf, 7);
						break;
					}
				}
				if (i == auth_num) {
					login_log("Char-server '%s': Error of Sex change (account: %d not found, suggested sex %c, ip: %s)." RETCODE,
					          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip_str);
				}
				RFIFOSKIP(fd,RFIFOW(fd,2));
			}
			break;

		default:
			{
				FILE *logfp;
				char tmpstr[24];
				struct timeval tv;
				time_t unixtime;
				logfp = savefopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					gettimeofday(&tv, NULL);
					unixtime = tv.tv_sec;
					strftime(tmpstr, 23, date_format, localtime(&unixtime));
					fprintf(logfp, "%s.%03ld: receiving of an unknown packet -> disconnection" RETCODE, tmpstr, tv.tv_usec / 1000);
					fprintf(logfp, "parse_fromchar: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip_str, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
					memset(tmpstr, '\0', sizeof(tmpstr));
					for(i = 0; i < RFIFOREST(fd); i++) {
						if ((i & 15) == 0)
							fprintf(logfp, "%04X ",i);
						fprintf(logfp, "%02x ", (unsigned char)RFIFOB(fd,i));
						if (RFIFOB(fd,i) > 0x1f)
							tmpstr[i % 16] = RFIFOB(fd,i);
						else
							tmpstr[i % 16] = '.';
						if ((i - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
						else if ((i + 1) % 16 == 0) {
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			ShowMessage("parse_fromchar: Unknown packet 0x%x (from a char-server)! -> disconnection.\n", (unsigned short)RFIFOW(fd,0));
			session_Remove(fd);
			ShowMessage("Char-server has been disconnected (unknown packet).\n");
			return 0;
		}//end case
	}// end while
	return 0;
}

//---------------------------------------
// Packet parsing for administation login
//---------------------------------------
int parse_admin(int fd) {
	int i, j;
	char* account_name;
	char ip_str[16];
	unsigned long client_ip = session[fd]->client_ip;
	sprintf(ip_str, "%ld.%ld.%ld.%ld", (client_ip>>24)&0xFF, (client_ip>>16)&0xFF, (client_ip>>8)&0xFF, (client_ip)&0xFF);

	if( !session_isActive(fd) ) {
		ShowMessage("Remote administration has disconnected (session #%d).\n", fd);
		session_Remove(fd);// have it removed by do_sendrecv
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
		if (display_parse_admin == 1)
			ShowMessage("parse_admin: connection #%d, packet: 0x%x (with being read: %d).\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));

		switch(RFIFOW(fd,0)) {
		case 0x7530:	// Request of the server version
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

		case 0x7532:	// Request of end of connection
			login_log("'ladmin': End of connection (ip: %s)" RETCODE, ip_str);
			RFIFOSKIP(fd,2);
			session_Remove(fd);
			break;

		case 0x7920:	// Request of an accounts list
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				unsigned long st, ed;
				size_t len;
				CREATE_BUFFER(id, unsigned long, auth_num);

				st = RFIFOL(fd,2);
				ed = RFIFOL(fd,6);
				RFIFOSKIP(fd,10);
				WFIFOW(fd,0) = 0x7921;
				if (st < 0)
					st = 0;
				if (ed > END_ACCOUNT_NUM || ed < st || ed <= 0)
					ed = END_ACCOUNT_NUM;
				login_log("'ladmin': Sending an accounts list (ask: from %d to %d, ip: %s)" RETCODE, st, ed, ip_str);
				// Sort before send
				for(i = 0; i < auth_num; i++) {
					int k;
					id[i] = i;
					for(j = 0; j < i; j++) {
						if (auth_dat[id[i]].account_id < auth_dat[id[j]].account_id) {
							for(k = i; k > j; k--) {
								id[k] = id[k-1];
							}
							id[j] = i; // id[i]
							break;
						}
					}
				}
				// Sending accounts information
				len = 4;
				for(i = 0; i < auth_num && len < 30000; i++) {
					unsigned long account_id = auth_dat[id[i]].account_id; // use sorted index
					if (account_id >= st && account_id <= ed) {
						j = id[i];
						WFIFOL(fd,len) = account_id;
						WFIFOB(fd,len+4) = (unsigned char)isGM(account_id);
						memcpy(WFIFOP(fd,len+5), auth_dat[j].userid, 24);
						WFIFOB(fd,len+29) = auth_dat[j].sex;
						WFIFOL(fd,len+30) = auth_dat[j].logincount;
						if (auth_dat[j].state == 0 && auth_dat[j].ban_until_time != 0) // if no state and banished
							WFIFOL(fd,len+34) = 7; // 6 = Your are Prohibited to log in until %s
						else
							WFIFOL(fd,len+34) = auth_dat[j].state;
						len += 38;
					}
				}
				WFIFOW(fd,2) = len;
				WFIFOSET(fd,len);

				DELETE_BUFFER(id);
			}
			break;

		case 0x7930:	// Request for an account creation
			if (RFIFOREST(fd) < 91)
				return 0;
			{
				struct mmo_account ma;
				ma.userid = (char*)RFIFOP(fd, 2);
				memcpy(ma.passwd, RFIFOP(fd, 26), 24);
				ma.passwd[24] = '\0';
				memcpy(ma.lastlogin, "-", 2);
				ma.sex = RFIFOB(fd,50);
				WFIFOW(fd,0) = 0x7931;
				WFIFOL(fd,2) = ~0;
				memcpy(WFIFOP(fd,6), RFIFOP(fd,2), 24);
				if (strlen(ma.userid) > 23 || strlen(ma.passwd) > 23) {
					login_log("'ladmin': Attempt to create an invalid account (account or pass is too long, ip: %s)" RETCODE,
					          ip_str);
				} else if (strlen(ma.userid) < 4 || strlen(ma.passwd) < 4) {
					login_log("'ladmin': Attempt to create an invalid account (account or pass is too short, ip: %s)" RETCODE,
					          ip_str);
				} else if (ma.sex != 'F' && ma.sex != 'M') {
					login_log("'ladmin': Attempt to create an invalid account (account: %s, received pass: %s, invalid sex, ip: %s)" RETCODE,
					          ma.userid, ma.passwd, ip_str);
				} else if (account_id_count > END_ACCOUNT_NUM) {
					login_log("'ladmin': Attempt to create an account, but there is no more available id number (account: %s, pass: %s, sex: %c, ip: %s)" RETCODE,
					          ma.userid, ma.passwd, ma.sex, ip_str);
				} else {
					remove_control_chars(ma.userid);
					remove_control_chars(ma.passwd);
					for(i = 0; i < auth_num; i++) {
						if (strncmp(auth_dat[i].userid, ma.userid, 24) == 0) {
							login_log("'ladmin': Attempt to create an already existing account (account: %s, pass: %s, received pass: %s, ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].pass, ma.passwd, ip_str);
							break;
						}
					}
					if (i == auth_num) {
						int new_id;
						char email[40];
						memcpy(email, RFIFOP(fd,51), 40);
						email[39] = '\0';
						remove_control_chars(email);
						new_id = mmo_auth_new(&ma, (char)ma.sex, email);
						login_log("'ladmin': Account creation (account: %s (id: %d), pass: %s, sex: %c, email: %s, ip: %s)" RETCODE,
						          ma.userid, new_id, ma.passwd, ma.sex, auth_dat[i].email, ip_str);
						WFIFOL(fd,2) = new_id;
						mmo_auth_sync();
					}
				}
				WFIFOSET(fd,30);
				RFIFOSKIP(fd,91);
			}
			break;

		case 0x7932:	// Request for an account deletion
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7933;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				// Char-server is notified of deletion (for characters deletion).
				unsigned char buf[65535];
				WBUFW(buf,0) = 0x2730;
				WBUFL(buf,2) = auth_dat[i].account_id;
				charif_sendallwos(-1, buf, 6);
				// send answer
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				// save deleted account in log file
				login_log("'ladmin': Account deletion (account: %s, id: %d, ip: %s) - saved in next line:" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip_str);
				mmo_auth_tostr((char*)buf, &auth_dat[i]);
				login_log("%s" RETCODE, buf);
				// delete account
				memset(auth_dat[i].userid, '\0', sizeof(auth_dat[i].userid));
				auth_dat[i].account_id = ~0;
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to delete an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7934:	// Request to change a password
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x7935;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(auth_dat[i].pass, RFIFOP(fd,26), 24);
				auth_dat[i].pass[23] = '\0';
				remove_control_chars((auth_dat[i].pass));
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Modification of a password (account: %s, new password: %s, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].pass, ip_str);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to modify the password of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x7936:	// Request to modify a state
			if (RFIFOREST(fd) < 50)
				return 0;
			{
				char error_message[20];
				unsigned long statut;
				WFIFOW(fd,0) = 0x7937;
				WFIFOL(fd,2) = ~0;
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				statut = RFIFOL(fd,26);
				memcpy(error_message, RFIFOP(fd,30), 20);
				error_message[19] = '\0';
				remove_control_chars(error_message);
				if (statut != 7 || error_message[0] == '\0') { // 7: // 6 = Your are Prohibited to log in until %s
					strcpy(error_message, "-");
				}
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					if (auth_dat[i].state == statut && strcmp(auth_dat[i].error_message, error_message) == 0)
						login_log("'ladmin': Modification of a state, but the state of the account is already the good state (account: %s, received state: %d, ip: %s)" RETCODE,
						          account_name, statut, ip_str);
					else {
						if (statut == 7)
							login_log("'ladmin': Modification of a state (account: %s, new state: %d - prohibited to login until '%s', ip: %s)" RETCODE,
							          auth_dat[i].userid, statut, error_message, ip_str);
						else
							login_log("'ladmin': Modification of a state (account: %s, new state: %d, ip: %s)" RETCODE,
							          auth_dat[i].userid, statut, ip_str);
						if (auth_dat[i].state == 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
							WBUFL(buf,7) = statut; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						auth_dat[i].state = statut;
						memcpy(auth_dat[i].error_message, error_message, 20);
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to modify the state of an unknown account (account: %s, received state: %d, ip: %s)" RETCODE,
					          account_name, statut, ip_str);
				}
				WFIFOL(fd,30) = statut;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,50);
			break;

		case 0x7938:	// Request for servers list and # of online players
			login_log("'ladmin': Sending of servers list (ip: %s)" RETCODE, ip_str);
			server_num = 0;
			for(i = 0; i < MAX_SERVERS; i++)
			{
				if (server[i].fd >= 0)
				{
					WFIFOLIP(fd,4+server_num*32) = server[i].lanip;
					WFIFOW(fd,4+server_num*32+4) = server[i].lanport;
					memcpy(WFIFOP(fd,4+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,4+server_num*32+26) = server[i].users;
					WFIFOW(fd,4+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,4+server_num*32+30) = server[i].new_;
					server_num++;
				}
			}
			WFIFOW(fd,0) = 0x7939;
			WFIFOW(fd,2) = 4 + 32 * server_num;
			WFIFOSET(fd,4+32*server_num);
			RFIFOSKIP(fd,2);
			break;

		case 0x793a:	// Request to password check
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x793b;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				char pass[25];
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(pass, RFIFOP(fd,26), 24);
				pass[24] = '\0';
				remove_control_chars(pass);
				if (strcmp(auth_dat[i].pass, pass) == 0) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					login_log("'ladmin': Check of password OK (account: %s, password: %s, ip: %s)" RETCODE,
					          auth_dat[i].userid, auth_dat[i].pass, ip_str);
				} else {
					login_log("'ladmin': Failure of password check (account: %s, proposed pass: %s, ip: %s)" RETCODE,
					          auth_dat[i].userid, pass, ip_str);
				}
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to check the password of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x793c:	// Request to modify sex
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793d;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char sex;
				sex = RFIFOB(fd,26);
				if (sex != 'F' && sex != 'M') {
					if (sex > 31)
						login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: %c, ip: %s)" RETCODE,
						          account_name, sex, ip_str);
					else
						login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: 'control char', ip: %s)" RETCODE,
						          account_name, ip_str);
				} else {
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (auth_dat[i].sex != ((sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm'))) {
							unsigned char buf[16];
							WFIFOL(fd,2) = auth_dat[i].account_id;
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							auth_dat[i].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');
							login_log("'ladmin': Modification of a sex (account: %s, new sex: %c, ip: %s)" RETCODE,
							          auth_dat[i].userid, sex, ip_str);
							mmo_auth_sync();
							// send to all char-server the change
							WBUFW(buf,0) = 0x2723;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = auth_dat[i].sex;
							charif_sendallwos(-1, buf, 7);
						} else {
							login_log("'ladmin': Modification of a sex, but the sex is already the good sex (account: %s, sex: %c, ip: %s)" RETCODE,
							          auth_dat[i].userid, sex, ip_str);
						}
					} else {
						login_log("'ladmin': Attempt to modify the sex of an unknown account (account: %s, received sex: %c, ip: %s)" RETCODE,
						          account_name, sex, ip_str);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x793e:	// Request to modify GM level
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793f;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char new_gm_level;
				new_gm_level = RFIFOB(fd,26);
				if (new_gm_level < 0 || new_gm_level > 99) {
					login_log("'ladmin': Attempt to give an invalid GM level (account: %s, received GM level: %d, ip: %s)" RETCODE,
					          account_name, (int)new_gm_level, ip_str);
				} else {
					i = search_account_index(account_name);
					if (i != -1) {
						int acc = auth_dat[i].account_id;
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (isGM(acc) != new_gm_level) {
							// modification of the file
							FILE *fp, *fp2;
							int lock;
							char line[512];
							int GM_account, GM_level;
							int modify_flag;
							char tmpstr[24];
							if ((fp2 = lock_fopen(GM_account_filename, &lock)) != NULL) {
								if ((fp = savefopen(GM_account_filename, "r")) != NULL) {
									time_t unixtime=time(NULL);
									strftime(tmpstr, 23, date_format, localtime(&unixtime));
									modify_flag = 0;
									// read/write GM file
									while(fgets(line, sizeof(line)-1, fp)) {
										while(line[0] != '\0' && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'))
											line[strlen(line)-1] = '\0';
										if( !skip_empty_line(line) )
											fprintf(fp2, "%s" RETCODE, line);
										else {
											if (sscanf(line, "%d %d", &GM_account, &GM_level) != 2 && sscanf(line, "%d: %d", &GM_account, &GM_level) != 2)
												fprintf(fp2, "%s" RETCODE, line);
											else if (GM_account != acc)
												fprintf(fp2, "%s" RETCODE, line);
											else if (new_gm_level < 1) {
												fprintf(fp2, "// %s: 'ladmin' GM level removed on account %d '%s' (previous level: %d)" RETCODE "//%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											} else {
												fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: %d)" RETCODE "%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											}
										}
									}
									if (modify_flag == 0)
										fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: 0)" RETCODE "%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, acc, new_gm_level);
									fclose(fp);
								} else {
									login_log("'ladmin': Attempt to modify of a GM level - impossible to read GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip_str);
								}
								if (lock_fclose(fp2, GM_account_filename, &lock) == 0) {
									WFIFOL(fd,2) = acc;
									login_log("'ladmin': Modification of a GM level (account: %s (%d), new GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip_str);
									// read and send new GM informations
									read_gm_account();
									send_GM_accounts();
								} else {
									login_log("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip_str);
								}
							} else {
								login_log("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
								          auth_dat[i].userid, acc, (int)new_gm_level, ip_str);
							}
						} else {
							login_log("'ladmin': Attempt to modify of a GM level, but the GM level is already the good GM level (account: %s (%d), GM level: %d, ip: %s)" RETCODE,
							          auth_dat[i].userid, acc, (int)new_gm_level, ip_str);
						}
					} else {
						login_log("'ladmin': Attempt to modify the GM level of an unknown account (account: %s, received GM level: %d, ip: %s)" RETCODE,
						          account_name, (int)new_gm_level, ip_str);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x7940:	// Request to modify e-mail
			if (RFIFOREST(fd) < 66)
				return 0;
			WFIFOW(fd,0) = 0x7941;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char email[40];
				memcpy(email, RFIFOP(fd,26), 40);
				if (e_mail_check(email) == 0) {
					login_log("'ladmin': Attempt to give an invalid e-mail (account: %s, ip: %s)" RETCODE,
					          account_name, ip_str);
				} else {
					remove_control_chars(email);
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						memcpy(auth_dat[i].email, email, 40);
						WFIFOL(fd,2) = auth_dat[i].account_id;
						login_log("'ladmin': Modification of an email (account: %s, new e-mail: %s, ip: %s)" RETCODE,
						          auth_dat[i].userid, email, ip_str);
						mmo_auth_sync();
					} else {
						login_log("'ladmin': Attempt to modify the e-mail of an unknown account (account: %s, received e-mail: %s, ip: %s)" RETCODE,
						          account_name, email, ip_str);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,66);
			break;

		case 0x7942:	// Request to modify memo field
			if (RFIFOREST(fd) < 28 || RFIFOREST(fd) < (28 + RFIFOW(fd,26)))
				return 0;
			WFIFOW(fd,0) = 0x7943;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				if (RFIFOW(fd,26) == 0) {
					strcpy(auth_dat[i].memo, "-");
				} else if ((size_t)(RFIFOW(fd,26)+1) > sizeof(auth_dat[i].memo)) {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), sizeof(auth_dat[i].memo) - 1);
				} else {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), RFIFOW(fd,26));
				}
				auth_dat[i].memo[ sizeof(auth_dat[i].memo)-1 ] =0;

				remove_control_chars((auth_dat[i].memo));
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Modification of a memo field (account: %s, new memo: %s, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].memo, ip_str);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to modify the memo field of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,28 + RFIFOW(fd,26));
			break;

		case 0x7944:	// Request to found an account id
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7945;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Request (by the name) of an account id (account: %s, id: %d, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip_str);
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': ID request (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7946:	// Request to found an account name
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7947;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,6), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == RFIFOL(fd,2)) {
					safestrcpy((char*)WFIFOP(fd,6), auth_dat[i].userid, 24);
					login_log("'ladmin': Request (by id) of an account name (account: %s, id: %d, ip: %s)" RETCODE,
					          auth_dat[i].userid, (unsigned long)RFIFOL(fd,2), ip_str);
					break;
				}
			}
			if (i == auth_num) {
				login_log("'ladmin': Name request (by id) of an unknown account (id: %d, ip: %s)" RETCODE,
				          (unsigned long)RFIFOL(fd,2), ip_str);
				safestrcpy((char*)WFIFOP(fd,6), "", 24);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;

		case 0x7948:	// Request to change the validity limit (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x7949;
				WFIFOL(fd,2) = ~0;
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				strftime(tmpstr, 24, date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					login_log("'ladmin': Change of a validity limit (account: %s, new validity: %d (%s), ip: %s)" RETCODE,
					          auth_dat[i].userid, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip_str);
					auth_dat[i].connect_until_time = timestamp;
					WFIFOL(fd,2) = auth_dat[i].account_id;
					mmo_auth_sync();
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to change the validity limit of an unknown account (account: %s, received validity: %d (%s), ip: %s)" RETCODE,
					          account_name, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip_str);
				}
				WFIFOL(fd,30) = timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794a:	// Request to change the final date of a banishment (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794b;
				WFIFOL(fd,2) = ~0;
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				if (timestamp <= time(NULL))
					timestamp = 0;
				strftime(tmpstr, 24, date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					login_log("'ladmin': Change of the final date of a banishment (account: %s, new final date of banishment: %d (%s), ip: %s)" RETCODE,
					          auth_dat[i].userid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
					if (auth_dat[i].ban_until_time != timestamp) {
						if (timestamp != 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						auth_dat[i].ban_until_time = timestamp;
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to change the final date of a banishment of an unknown account (account: %s, received final date of banishment: %d (%s), ip: %s)" RETCODE,
					          account_name, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
				}
				WFIFOL(fd,30) = timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794c:	// Request to change the final date of a banishment (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794d;
				WFIFOL(fd,2) = ~0;
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				i = search_account_index(account_name);
				if (i != -1) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					if (auth_dat[i].ban_until_time == 0 || auth_dat[i].ban_until_time < time(NULL))
						timestamp = time(NULL);
					else
						timestamp = auth_dat[i].ban_until_time;
					tmtime = localtime(&timestamp);
					tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
					tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
					tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
					tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
					tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
					tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
					timestamp = mktime(tmtime);
					if (timestamp != -1) {
						if (timestamp <= time(NULL))
							timestamp = 0;
						strftime(tmpstr, 24, date_format, localtime(&timestamp));
						login_log("'ladmin': Adjustment of a final date of a banishment (account: %s, (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
						          auth_dat[i].userid, (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip_str);
						if (auth_dat[i].ban_until_time != timestamp) {
							if (timestamp != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = auth_dat[i].account_id;
								WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
								WBUFL(buf,7) = timestamp; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
								for(j = 0; j < AUTH_FIFO_SIZE; j++)
									if (auth_fifo[j].account_id == auth_dat[i].account_id)
										auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							}
							auth_dat[i].ban_until_time = timestamp;
							mmo_auth_sync();
						}
					} else {
						strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].ban_until_time));
						login_log("'ladmin': Impossible to adjust the final date of a banishment (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
						          auth_dat[i].userid, auth_dat[i].ban_until_time, (auth_dat[i].ban_until_time == 0 ? "no banishment" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip_str);
					}
					WFIFOL(fd,30) = (unsigned long)auth_dat[i].ban_until_time;
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to adjust the final date of a banishment of an unknown account (account: %s, ip: %s)" RETCODE,
					          account_name, ip_str);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x794e:	// Request to send a broadcast message
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + (int)RFIFOL(fd,4)))
				return 0;
			WFIFOW(fd,0) = 0x794f;
			WFIFOW(fd,2) = ~0;
			if (RFIFOL(fd,4) < 1)
			{
				login_log("'ladmin': Receiving a message for broadcast, but message is void (ip: %s)" RETCODE,
				          ip_str);
			}
			else
			{
				// at least 1 char-server
				for(i = 0; i < MAX_SERVERS; i++)
					if (server[i].fd >= 0)
						break;
				if (i == MAX_SERVERS)
				{
					login_log("'ladmin': Receiving a message for broadcast, but no char-server is online (ip: %s)" RETCODE,
					          ip_str);
				}
				else
				{
					unsigned char buf[32000];
					char message[32000];
					WFIFOW(fd,2) = 0;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), RFIFOL(fd,4));
					message[sizeof(message)-1] = '\0';
					remove_control_chars(message);
					if (RFIFOW(fd,2) == 0)
						login_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s, ip: %s)" RETCODE,
						          message, ip_str);
					else
						login_log("'ladmin': Receiving a message for broadcast (message (in blue): %s, ip: %s)" RETCODE,
						          message, ip_str);
					// send same message to all char-servers (no answer)
					memcpy(WBUFP(buf,0), RFIFOP(fd,0), 8 + RFIFOL(fd,4));
					WBUFW(buf,0) = 0x2726;
					charif_sendallwos(-1, (unsigned char*)buf, 8 + RFIFOL(fd,4));
				}
			}
			WFIFOSET(fd,4);
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
			break;

		case 0x7950:	// Request to change the validity limite (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				char tmpstr2[2048];
				WFIFOW(fd,0) = 0x7951;
				WFIFOL(fd,2) = ~0;
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars(account_name);
				i = search_account_index(account_name);
				if (i != -1) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					timestamp = auth_dat[i].connect_until_time;
					if (add_to_unlimited_account == 0 && timestamp == 0) {
						login_log("'ladmin': Attempt to adjust the validity limit of an unlimited account (account: %s, ip: %s)" RETCODE,
						          auth_dat[i].userid, ip_str);
						WFIFOL(fd,30) = 0;
					} else {
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
						if (timestamp != -1) {
							strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].connect_until_time));
							strftime(tmpstr2, 24, date_format, localtime(&timestamp));
							login_log("'ladmin': Adjustment of a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "unlimited" : tmpstr2), ip_str);
							auth_dat[i].connect_until_time = timestamp;
							mmo_auth_sync();
							WFIFOL(fd,30) = (unsigned long)auth_dat[i].connect_until_time;
						} else {
							strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].connect_until_time));
							login_log("'ladmin': Impossible to adjust a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip_str);
							WFIFOL(fd,30) = 0;
						}
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to adjust the validity limit of an unknown account (account: %s, ip: %s)" RETCODE,
					          account_name, ip_str);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x7952:	// Request about informations of an account (by account name)
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = ~0;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars(account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				WFIFOL(fd,2) = auth_dat[i].account_id;
				WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
				memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
				WFIFOB(fd,31) = auth_dat[i].sex;
				WFIFOL(fd,32) = auth_dat[i].logincount;
				WFIFOL(fd,36) = auth_dat[i].state;
				memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
				memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
				memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
				memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
				WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
				WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
				WFIFOW(fd,148) = strlen(auth_dat[i].memo);
				if (auth_dat[i].memo[0]) {
					memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
				}
				login_log("'ladmin': Sending information of an account (request by the name; account: %s, id: %d, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip_str);
				WFIFOSET(fd,150+strlen(auth_dat[i].memo));
			} else {
				memcpy(WFIFOP(fd,7), account_name, 24);
				WFIFOW(fd,148) = 0;
				login_log("'ladmin': Attempt to obtain information (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip_str);
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,26);
			break;

		case 0x7954:	// Request about information of an account (by account id)
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,7), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == RFIFOL(fd,2)) {
					login_log("'ladmin': Sending information of an account (request by the id; account: %s, id: %d, ip: %s)" RETCODE,
					          auth_dat[i].userid, (unsigned long)RFIFOL(fd,2), ip_str);
					WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
					memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
					WFIFOB(fd,31) = auth_dat[i].sex;
					WFIFOL(fd,32) = auth_dat[i].logincount;
					WFIFOL(fd,36) = auth_dat[i].state;
					memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
					memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
					memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
					memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
					WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
					WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
					WFIFOW(fd,148) = strlen(auth_dat[i].memo);
					if (auth_dat[i].memo[0]) {
						memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
					}
					WFIFOSET(fd,150+strlen(auth_dat[i].memo));
					break;
				}
			}
			if (i == auth_num) {
				login_log("'ladmin': Attempt to obtain information (by the id) of an unknown account (id: %d, ip: %s)" RETCODE,
				          (unsigned long)RFIFOL(fd,2), ip_str);
				safestrcpy((char*)WFIFOP(fd,7), "", 24);
				WFIFOW(fd,148) = 0;
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,6);
			break;

		case 0x7955:	// Request to reload GM file (no answer)
			login_log("'ladmin': Request to re-load GM configuration file (ip: %s)." RETCODE, ip_str);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts();
			RFIFOSKIP(fd,2);
			break;

		default:
			{
				FILE *logfp;
				char tmpstr[24];
				struct timeval tv;
				time_t unixtime;
				logfp = savefopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					gettimeofday(&tv, NULL);
					unixtime = tv.tv_sec;
					strftime(tmpstr, 23, date_format, localtime(&unixtime));
					fprintf(logfp, "%s.%03ld: receiving of an unknown packet -> disconnection" RETCODE, tmpstr, tv.tv_usec / 1000);
					fprintf(logfp, "parse_admin: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip_str, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
					memset(tmpstr, '\0', sizeof(tmpstr));
					for(i = 0; i < RFIFOREST(fd); i++) {
						if ((i & 15) == 0)
							fprintf(logfp, "%04X ",i);
						fprintf(logfp, "%02x ", (unsigned char)RFIFOB(fd,i));
						if (RFIFOB(fd,i) > 0x1f)
							tmpstr[i % 16] = RFIFOB(fd,i);
						else
							tmpstr[i % 16] = '.';
						if ((i - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
						else if ((i + 1) % 16 == 0) {
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			login_log("'ladmin': End of connection, unknown packet (ip: %s)" RETCODE, ip_str);
			session_Remove(fd);
			ShowMessage("Remote administration has been disconnected (unknown packet).\n");
			return 0;
		}
		//WFIFOW(fd,0) = 0x791f;
		//WFIFOSET(fd,2);
	}
	return 0;
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_ip_check(unsigned long ip){
	int lancheck;
//	ShowMessage("lan_ip_check: to compare: %X, network: %X/%X\n", ip, subnet_ip, subnet_mask);

	lancheck = (subnet_ip & subnet_mask) == (ip & subnet_mask);
	ShowMessage("LAN test (result): %s source.\n"CL_NORM, (lancheck) ? CL_BT_CYAN"LAN" : CL_BT_GREEN"WAN");
	return lancheck;
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd) {
	struct mmo_account account;
	int result, i, j;
	char ip_str[16];
	unsigned long client_ip = session[fd]->client_ip;
	sprintf(ip_str, "%ld.%ld.%ld.%ld", (client_ip>>24)&0xFF, (client_ip>>16)&0xFF, (client_ip>>8)&0xFF, (client_ip)&0xFF);

	if ( !session_isActive(fd) ) {
		session_Remove(fd);// have it removed by do_sendrecv
        memset(&account, 0, sizeof(account));
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
		if (display_parse_login == 1) {
			if (RFIFOW(fd,0) == 0x64 || RFIFOW(fd,0) == 0x01dd) {
				if (RFIFOREST(fd) >= ((RFIFOW(fd,0) == 0x64) ? 55 : 47))
					ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d), account: %s.\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd), RFIFOP(fd,6));
			} else if (RFIFOW(fd,0) == 0x2710) {
				if (RFIFOREST(fd) >= 86)
					ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d), server: %s.\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd), RFIFOP(fd,60));
			} else
				ShowMessage("parse_login: connection #%d, packet: 0x%x (with being read: %d).\n", fd, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
		}

		switch(RFIFOW(fd,0)) {
		case 0x200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
			break;

		case 0x204:		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
			if (RFIFOREST(fd) < 18)
				return 0;
			RFIFOSKIP(fd,18);
			break;

		case 0x64:		// Ask connection of a client
		case 0x01dd:	// Ask connection of a client (encryption mode)
			if (RFIFOREST(fd) < ((RFIFOW(fd,0) == 0x64) ? 55 : 47))
				return 0;
			
			account.version = RFIFOL(fd, 2);	//for exe version check [Sirius]
			account.userid = (char*)RFIFOP(fd,6);
			account.userid[23] = '\0';
			remove_control_chars(account.userid);
			if (RFIFOW(fd,0) == 0x64) {
				login_log("Request for connection (non encryption mode) of %s (ip: %s)." RETCODE, account.userid, ip_str);
				memcpy(account.passwd, RFIFOP(fd,30), 24);
				account.passwd[24] = '\0';
				remove_control_chars(account.passwd);
			} else {
				login_log("Request for connection (encryption mode) of %s (ip: %s)." RETCODE, account.userid, ip_str);
				 // If remove control characters from received password encrypted by md5,
				// there would be a wrong result and failed to authentication. [End_of_exam]
				memcpy(account.passwd, RFIFOP(fd,30), 16);
				account.passwd[16] = '\0';
			}
#ifdef PASSWORDENC
			account.passwdenc = (RFIFOW(fd,0) == 0x64) ? 0 : PASSWORDENC;
#else
			account.passwdenc = 0;
#endif

			if ( !check_ip(client_ip) ) {
				login_log("Connection refused: IP isn't authorised (deny/allow, ip: %s)." RETCODE, ip_str);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = 3; // 3 = Rejected from Server
				WFIFOSET(fd,23);
				RFIFOSKIP(fd,(RFIFOW(fd,0) == 0x64) ? 55 : 47);
				break;
			}

			result = mmo_auth(&account, fd);
			if (result == -1) {
				int gm_level = isGM(account.account_id);
				if (min_level_to_connect > gm_level) {
					login_log("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d, ip: %s)." RETCODE,
					          min_level_to_connect, account.userid, gm_level, ip_str);
					WFIFOW(fd,0) = 0x81;
					WFIFOL(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
				} else {
					if (gm_level)
						ShowMessage("Connection of the GM (level:%d) account '%s' accepted.\n", gm_level, account.userid);
					else
						ShowMessage("Connection of the account '%s' accepted.\n", account.userid);
					server_num = 0;
					for(i = 0; i < MAX_SERVERS; i++)
					{
						if( server[i].fd >= 0)
						{
							if (lan_ip_check(client_ip))
								WFIFOLIP(fd,47+server_num*32) = lan_char_ip;
							else
								WFIFOLIP(fd,47+server_num*32) = server[i].lanip;
							WFIFOW(fd,47+server_num*32+4) = server[i].lanport;
							memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
							WFIFOW(fd,47+server_num*32+26) = server[i].users;
							WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
							WFIFOW(fd,47+server_num*32+30) = server[i].new_;
							server_num++;
						}
					}
					// if at least 1 char-server
					if (server_num > 0) {
						WFIFOW(fd,0) = 0x69;
						WFIFOW(fd,2) = 47+32*server_num;
						WFIFOL(fd,4) = account.login_id1;
						WFIFOL(fd,8) = account.account_id;
						WFIFOL(fd,12) = account.login_id2;
						WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
						memcpy(WFIFOP(fd,20), account.lastlogin, 24); // in old version, that was for name (not more used)
						WFIFOB(fd,46) = account.sex;
						WFIFOSET(fd,47+32*server_num);
						if (auth_fifo_pos >= AUTH_FIFO_SIZE)
							auth_fifo_pos = 0;
						auth_fifo[auth_fifo_pos].account_id = account.account_id;
						auth_fifo[auth_fifo_pos].login_id1 = account.login_id1;
						auth_fifo[auth_fifo_pos].login_id2 = account.login_id2;
						auth_fifo[auth_fifo_pos].sex = account.sex;
						auth_fifo[auth_fifo_pos].delflag = 0;
						auth_fifo[auth_fifo_pos].ip = client_ip;
						auth_fifo_pos++;
					// if no char-server, don't send void list of servers, just disconnect the player with proper message
					} else {
						login_log("Connection refused: there is no char-server online (account: %s, ip: %s)." RETCODE,
						          account.userid, ip_str);
						WFIFOW(fd,0) = 0x81;
						WFIFOL(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
					}
				}
			} else {
				memset(WFIFOP(fd,0), '\0', 23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = result;
				if (result == 6) { // 6 = Your are Prohibited to log in until %s
					i = search_account_index(account.userid);
					if (i != -1) {
						if (auth_dat[i].ban_until_time != 0) { // if account is banned, we send ban timestamp
							char tmpstr[256];
							strftime(tmpstr, 20, date_format, localtime(&auth_dat[i].ban_until_time));
							tmpstr[19] = '\0';
							memcpy(WFIFOP(fd,3), tmpstr, 20);
						} else { // we send error message
							memcpy(WFIFOP(fd,3), auth_dat[i].error_message, 20);
						}
					}
				}
				WFIFOSET(fd,23);
			}
			RFIFOSKIP(fd,(RFIFOW(fd,0) == 0x64) ? 55 : 47);
			break;

		case 0x01db:	// Sending request of the coding key
		case 0x791a:	// Sending request of the coding key (administration packet)
			{
				struct login_session_data *ld;
				if (session[fd]->session_data) {
					ShowMessage("login: abnormal request of MD5 key (already opened session).\n");
					session_Remove(fd);
					return 0;
				}
				ld = (struct login_session_data*)(session[fd]->session_data = aCalloc(1, sizeof(struct login_session_data)));
				if (!ld) {
					ShowMessage("login: Request for md5 key: memory allocation failure (malloc)!\n");
					session_Remove(fd);
					return 0;
				}
				if (RFIFOW(fd,0) == 0x01db)
					login_log("Sending request of the coding key (ip: %s)" RETCODE, ip_str);
				else
					login_log("'ladmin': Sending request of the coding key (ip: %s)" RETCODE, ip_str);
				// Creation of the coding key
				memset(ld->md5key, '\0', sizeof(ld->md5key));
				ld->md5keylen = rand() % 4 + 12;
				for(i = 0; i < ld->md5keylen; i++)
					ld->md5key[i] = rand() % 255 + 1;
				WFIFOW(fd,0) = 0x01dc;
				WFIFOW(fd,2) = 4 + ld->md5keylen;
				memcpy(WFIFOP(fd,4), ld->md5key, ld->md5keylen);
				WFIFOSET(fd,WFIFOW(fd,2));
				RFIFOSKIP(fd,2);
			}
			break;

		case 0x2710:	// Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
			{
				int GM_value, len;
				char* server_name;
				account.userid = (char*)RFIFOP(fd,2);
				account.userid[23] = '\0';
				remove_control_chars(account.userid);
				memcpy(account.passwd, RFIFOP(fd,26), 24);
				account.passwd[24] = '\0';
				remove_control_chars(account.passwd);
				account.passwdenc = 0;
				server_name = (char*)RFIFOP(fd,60);
				server_name[19] = '\0';
				remove_control_chars(server_name);
				login_log("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (ip: %s)" RETCODE,
				          server_name, (unsigned char)RFIFOB(fd,54), (unsigned char)RFIFOB(fd,55), (unsigned char)RFIFOB(fd,56), (unsigned char)RFIFOB(fd,57), (unsigned short)RFIFOW(fd,58), ip_str);
				result = mmo_auth(&account, fd);
				if (result == -1 && account.sex == 2 && account.account_id < MAX_SERVERS && server[account.account_id].fd == -1)
				{
					login_log("Connection of the char-server '%s' accepted (account: %s, pass: %s, ip: %s)" RETCODE,
					          server_name, account.userid, account.passwd, ip_str);
					ShowMessage("Connection of the char-server '%s' accepted.\n", server_name);
					memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
					server[account.account_id].lanip = RFIFOLIP(fd,54);
					server[account.account_id].lanport = RFIFOW(fd,58);
					memcpy(server[account.account_id].name, server_name, 20);
					server[account.account_id].users = 0;
					server[account.account_id].maintenance = RFIFOW(fd,82);
					server[account.account_id].new_ = RFIFOW(fd,84);
					server[account.account_id].fd = fd;

					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd,3);
					session[fd]->func_parse = parse_fromchar;
					realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
					// send GM account to char-server
					len = 4;
					WFIFOW(fd,0) = 0x2732;
					for(i = 0; i < auth_num; i++)
						// send only existing accounts. We can not create a GM account when server is online.
						if ((GM_value = isGM(auth_dat[i].account_id)) > 0) {
							WFIFOL(fd,len) = auth_dat[i].account_id;
							WFIFOB(fd,len+4) = (unsigned char)GM_value;
							len += 5;
						}
					WFIFOW(fd,2) = len;
					WFIFOSET(fd,len);
				} else {
					if (server[account.account_id].fd != -1) {
						ShowMessage("Connection of the char-server '%s' REFUSED - already connected (account: %ld-%s, pass: %s, ip: %s)\n",
					        server_name, account.account_id, account.userid, account.passwd, ip_str);
						ShowMessage("You must probably wait that the freeze system detect the disconnection.\n");
						login_log("Connexion of the char-server '%s' REFUSED - already connected (account: %ld-%s, pass: %s, ip: %s)" RETCODE,
					          server_name, account.account_id, account.userid, account.passwd, ip_str);
					} else {
						ShowMessage("Connection of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s).\n", server_name, account.userid, account.passwd, ip_str);
						login_log("Connexion of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s)" RETCODE,
					          	server_name, account.userid, account.passwd, ip_str);
					}
					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 3;
					WFIFOSET(fd,3);
				}
			}
			RFIFOSKIP(fd,86);
			return 0;

		case 0x7530:	// Request of the server version
			login_log("Sending of the server version (ip: %s)" RETCODE, ip_str);
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

		case 0x7532:	// Request to end connection
			login_log("End of connection (ip: %s)" RETCODE, ip_str);
			session_Remove(fd);
			return 0;

		case 0x7918:	// Request for administation login
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < ((RFIFOW(fd,2) == 0) ? 28 : 20))
				return 0;
			WFIFOW(fd,0) = 0x7919;
			WFIFOB(fd,2) = 1;
			if (!check_ladminip( client_ip )) {
				login_log("'ladmin'-login: Connection in administration mode refused: IP isn't authorised (ladmin_allow, ip: %s)." RETCODE, ip_str);
			} else {
				struct login_session_data *ld = (struct login_session_data*)session[fd]->session_data;
				if (RFIFOW(fd,2) == 0) {	// non encrypted password
					char* password="";
					memcpy(password, RFIFOP(fd,4), 24);
					password[24] = '\0';
					remove_control_chars(password);
					// If remote administration is enabled and password sent by client matches password read from login server configuration file
					if ((admin_state == 1) && (strcmp(password, admin_pass) == 0)) {
						login_log("'ladmin'-login: Connection in administration mode accepted (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
						ShowMessage("Connection of a remote administration accepted (non encrypted password).\n");
						WFIFOB(fd,2) = 0;
						session[fd]->func_parse = parse_admin;
					} else if (admin_state != 1)
						login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
					else
						login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (non encrypted password: %s, ip: %s)" RETCODE, password, ip_str);
				} else {	// encrypted password
					if (!ld)
						ShowMessage("'ladmin'-login: error! MD5 key not created/requested for an administration login.\n");
					else {
						char md5str[64] = "", md5bin[32];
						if (RFIFOW(fd,2) == 1) {
							sprintf(md5str, "%s%s", ld->md5key, admin_pass); // 20 24
						} else if (RFIFOW(fd,2) == 2) {
							sprintf(md5str, "%s%s", admin_pass, ld->md5key); // 24 20
						}
						MD5_String2binary(md5str, md5bin);
						// If remote administration is enabled and password hash sent by client matches hash of password read from login server configuration file
						if ((admin_state == 1) && (memcmp(md5bin, RFIFOP(fd,4), 16) == 0)) {
							login_log("'ladmin'-login: Connection in administration mode accepted (encrypted password, ip: %s)" RETCODE, ip_str);
							ShowMessage("Connection of a remote administration accepted (encrypted password).\n");
							WFIFOB(fd,2) = 0;
							session[fd]->func_parse = parse_admin;
						} else if (admin_state != 1)
							login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (encrypted password, ip: %s)" RETCODE, ip_str);
						else
							login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (encrypted password, ip: %s)" RETCODE, ip_str);
					}
				}
			}
			WFIFOSET(fd,3);
			RFIFOSKIP(fd, (RFIFOW(fd,2) == 0) ? 28 : 20);
			break;

		default:
			if (save_unknown_packets) {
				FILE *logfp;
				char tmpstr[24];
				struct timeval tv;
				time_t unixtime;
				logfp = savefopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					gettimeofday(&tv, NULL);
					unixtime=tv.tv_sec;
					strftime(tmpstr, 23, date_format, localtime(&unixtime));
					fprintf(logfp, "%s.%03ld: receiving of an unknown packet -> disconnection" RETCODE, tmpstr, tv.tv_usec / 1000);
					fprintf(logfp, "parse_login: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip_str, (unsigned short)RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
					memset(tmpstr, '\0', sizeof(tmpstr));
					for(i = 0; i < RFIFOREST(fd); i++) {
						if ((i & 15) == 0)
							fprintf(logfp, "%04X ",i);
						fprintf(logfp, "%02x ", (unsigned char)RFIFOB(fd,i));
						if (RFIFOB(fd,i) > 0x1f)
							tmpstr[i % 16] = RFIFOB(fd,i);
						else
							tmpstr[i % 16] = '.';
						if ((i - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
						else if ((i + 1) % 16 == 0) {
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			login_log("End of connection, unknown packet (ip: %s)" RETCODE, ip_str);
			session_Remove(fd);
			return 0;
		}
	}
	return 0;
}

//-----------------------
// Console Command Parser [Wizputer]
//-----------------------
int parse_console(char *buf) {
	char command[256];

	memset(command,0,sizeof(command));

	sscanf(buf, "%[^\n]", command);

	login_log("Console command :%s" RETCODE, command);

	if(strcasecmp("shutdown", command) == 0 ||
	    strcasecmp("exit", command) == 0 ||
	    strcasecmp("quit", command) == 0 ||
	    strcasecmp("end", command) == 0)
		core_stoprunning();
	else if(strcasecmp("alive", command) == 0 ||
	         strcasecmp("status", command) == 0)
		ShowConsole(CL_BOLD"I'm Alive.\n"CL_NORM);

	else if(strcasecmp("help", command) == 0) {
		ShowMessage(CL_GREEN"Help of commands:\n"CL_NORM);
		ShowMessage("  To shutdown the server:\n");
		ShowMessage("  'shutdown|exit|qui|end'\n");
		ShowMessage("  To know if server is alive:\n");
		ShowMessage("  'alive|status'\n");
	}

	return 0;
}


//----------------------------------
// Reading Lan Support configuration
//----------------------------------
int login_lan_config_read(const char *lancfgName) {
	struct hostent * h = NULL;
	char ip_str[16];
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = savefopen(lancfgName, "r");
	if (fp == NULL) {
		ShowMessage("***WARNING: LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	ShowMessage("---Start reading Lan Support configuration file\n");

	while(fgets(line, sizeof(line)-1, fp)) {
		if( !skip_empty_line(line) )
			continue;

		line[sizeof(line)-1] = '\0';
		memset(w2, 0, sizeof(w2));
		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);
		if (strcasecmp(w1, "lan_char_ip") == 0) { // Read Char-Server Lan IP Address
			h = gethostbyname(w2);
			if (h != NULL) {
				sprintf(ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				strcpy(ip_str, w2);
			lan_char_ip  = ntohl(inet_addr(ip_str));
			ShowMessage("LAN IP of map-server: %d.%d.%d.%d.\n", (lan_char_ip>>24)&0xFF,(lan_char_ip>>16)&0xFF,(lan_char_ip>>8)&0xFF,(lan_char_ip)&0xFF);
		} else if (strcasecmp(w1, "subnet") == 0) { // Read Subnetwork
			h = gethostbyname(w2);
			if (h != NULL) {
				sprintf(ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				strcpy(ip_str, w2);
			subnet_ip = ntohl(inet_addr(ip_str));
			ShowMessage("Sub-network of the char-server: %d.%d.%d.%d.\n", (subnet_ip>>24)&0xFF,(subnet_ip>>16)&0xFF,(subnet_ip>>8)&0xFF,(subnet_ip)&0xFF);
		} else if (strcasecmp(w1, "subnetmask") == 0) { // Read Subnetwork Mask
			h = gethostbyname(w2);
			if (h != NULL) {
				sprintf(ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				strcpy(ip_str, w2);
			subnet_mask = ntohl(inet_addr(ip_str));
			ShowMessage("Sub-network mask of the char-server: %d.%d.%d.%d.\n", (subnet_mask>>24)&0xFF,(subnet_mask>>16)&0xFF,(subnet_mask>>8)&0xFF,(subnet_mask)&0xFF);
			}
		}
	fclose(fp);

	// log the LAN configuration
	login_log("The LAN configuration of the server is set:" RETCODE);
	login_log("- with LAN IP of char-server: %X." RETCODE, lan_char_ip);
	login_log("- with the sub-network of the char-server: %d.%d.%d.%d/%d.%d.%d.%d." RETCODE,
	   (subnet_ip>>24)&0xFF, (subnet_ip>>16)&0xFF, (subnet_ip>>8)&0xFF, (subnet_ip)&0xFF, 
	   (subnet_mask>>24)&0xFF, (subnet_mask>>16)&0xFF, (subnet_mask>>8)&0xFF, (subnet_mask)&0xFF);

	// sub-network check of the char-server
	{
		if (lan_ip_check(lan_char_ip) == 0) {
			ShowMessage(CL_BT_RED"***ERROR: LAN IP of the char-server doesn't belong to the specified Sub-network\n"CL_NORM);
			login_log("***ERROR: LAN IP of the char-server doesn't belong to the specified Sub-network." RETCODE);
		}
	}

	ShowMessage("---End reading of Lan Support configuration file\n");

	return 0;
}

//-----------------------------------
// Reading general configuration file
//-----------------------------------
int login_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	struct hostent *h = NULL;

	if ((fp = savefopen(cfgName, "r")) == NULL) {
		ShowMessage("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}

	ShowMessage("---Start reading of Login Server configuration file (%s)\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)) {
		if( !skip_empty_line(line) )
			continue;

		line[sizeof(line)-1] = '\0';
		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			remove_control_chars(w1);
			remove_control_chars(w2);

			if (strcasecmp(w1, "admin_state") == 0) {
				admin_state = config_switch(w2);
			} else if (strcasecmp(w1, "admin_pass") == 0) {
				safestrcpy(admin_pass, w2, sizeof(admin_pass));
			} else if (strcasecmp(w1, "ladminallowip") == 0) {
				if (strcasecmp(w2, "clear") == 0) {
					if (access_ladmin_allow)
					{
						aFree(access_ladmin_allow);
					access_ladmin_allow = NULL;
					access_ladmin_allownum = 0;
					}
				} else {
					if (strcasecmp(w2, "all") == 0) {
						// reset all previous values
						if (access_ladmin_allow)
							aFree(access_ladmin_allow);
						// set to all
						access_ladmin_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						access_ladmin_allownum = 1;
						access_ladmin_allow[0] = '\0';
					} else if (w2[0] && !(access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0')) { // don't add IP if already 'all'
						if (access_ladmin_allow)
							access_ladmin_allow = (char*)aRealloc(access_ladmin_allow, (access_ladmin_allownum+1) * ACO_STRSIZE);
						else
							access_ladmin_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						safestrcpy(access_ladmin_allow + (access_ladmin_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
			} else if (strcasecmp(w1, "gm_pass") == 0) {
				safestrcpy(gm_pass, w2, sizeof(gm_pass));
			} else if (strcasecmp(w1, "level_new_gm") == 0) {
				level_new_gm = atoi(w2);
			} else if (strcasecmp(w1, "new_account") == 0) {
				new_account_flag = config_switch(w2);
			} else if (strcasecmp(w1, "login_ip") == 0) {
				char login_ip_str[16];
				h = gethostbyname (w2);
				if (h != NULL) {
					ShowMessage("Login server binding IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					sprintf(login_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				} else
					memcpy(login_ip_str,w2,16);
				login_ip = ntohl(inet_addr(login_ip_str));
			} else if (strcasecmp(w1, "login_port") == 0) {
				login_port = atoi(w2);
			} else if (strcasecmp(w1, "account_filename") == 0) {
				safestrcpy(account_filename, w2, sizeof(account_filename));
			} else if (strcasecmp(w1, "gm_account_filename") == 0) {
				safestrcpy(GM_account_filename, w2, sizeof(GM_account_filename));
			} else if (strcasecmp(w1, "gm_account_filename_check_timer") == 0) {
				gm_account_filename_check_timer = atoi(w2);
			} else if (strcasecmp(w1, "use_MD5_passwords") == 0) {
				use_md5_passwds = config_switch(w2);
			} else if (strcasecmp(w1, "login_log_filename") == 0) {
				safestrcpy(login_log_filename, w2, sizeof(login_log_filename));
			} else if (strcasecmp(w1, "log_login") == 0) {
				log_login = atoi(w2);
			} else if (strcasecmp(w1, "login_log_unknown_packets_filename") == 0) {
				safestrcpy(login_log_unknown_packets_filename, w2, sizeof(login_log_unknown_packets_filename));
			} else if (strcasecmp(w1, "save_unknown_packets") == 0) {
				save_unknown_packets = config_switch(w2);
			} else if (strcasecmp(w1, "display_parse_login") == 0) {
				display_parse_login = config_switch(w2); // 0: no, 1: yes
			} else if (strcasecmp(w1, "display_parse_admin") == 0) {
				display_parse_admin = config_switch(w2); // 0: no, 1: yes
			} else if (strcasecmp(w1, "display_parse_fromchar") == 0) {
				display_parse_fromchar = config_switch(w2); // 0: no, 1: yes (without packet 0x2714), 2: all packets
			} else if (strcasecmp(w1, "date_format") == 0) { // note: never have more than 19 char for the date!
				memset(date_format, 0, sizeof(date_format));
				switch (atoi(w2)) {
				case 0:
					strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
				}
			} else if (strcasecmp(w1, "min_level_to_connect") == 0) {
				min_level_to_connect = atoi(w2);
			} else if (strcasecmp(w1, "add_to_unlimited_account") == 0) {
				add_to_unlimited_account = config_switch(w2);
			} else if (strcasecmp(w1, "start_limited_time") == 0) {
				start_limited_time = atoi(w2);
			} else if (strcasecmp(w1, "check_ip_flag") == 0) {
				check_ip_flag = config_switch(w2);
			} else if (strcasecmp(w1, "order") == 0) {
				access_order = atoi(w2);
				if (strcasecmp(w2, "deny,allow") == 0 ||
				    strcasecmp(w2, "deny, allow") == 0) access_order = ACO_DENY_ALLOW;
				if (strcasecmp(w2, "allow,deny") == 0 ||
				    strcasecmp(w2, "allow, deny") == 0) access_order = ACO_ALLOW_DENY;
				if (strcasecmp(w2, "mutual-failture") == 0 ||
				    strcasecmp(w2, "mutual-failure") == 0) access_order = ACO_MUTUAL_FAILTURE;
			} else if (strcasecmp(w1, "allow") == 0) {
				if (strcasecmp(w2, "clear") == 0) {
					if (access_allow)
						aFree(access_allow);
					access_allow = NULL;
					access_allownum = 0;
				} else {
					if (strcasecmp(w2, "all") == 0) {
						// reset all previous values
						if (access_allow)
							aFree(access_allow);
						// set to all
						access_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						access_allownum = 1;
						access_allow[0] = '\0';
					} else if (w2[0] && !(access_allownum == 1 && access_allow[0] == '\0')) { // don't add IP if already 'all'
						if (access_allow)
							access_allow = (char*)aRealloc(access_allow, (access_allownum+1) * ACO_STRSIZE);
						else
							access_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						safestrcpy(access_allow + (access_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
			} else if (strcasecmp(w1, "deny") == 0) {
				if (strcasecmp(w2, "clear") == 0) {
					if (access_deny)
					{
						aFree(access_deny);
					access_deny = NULL;
					access_denynum = 0;
					}
				} else {
					if (strcasecmp(w2, "all") == 0) {
						// reset all previous values
						if (access_deny)
							aFree(access_deny);
						// set to all
						access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						access_denynum = 1;
						access_deny[0] = '\0';
					} else if (w2[0] && !(access_denynum == 1 && access_deny[0] == '\0')) { // don't add IP if already 'all'
						if (access_deny)
							access_deny = (char*)aRealloc(access_deny, (access_denynum+1) * ACO_STRSIZE);
						else
							access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
						safestrcpy(access_deny + (access_denynum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					}
				}
				// dynamic password error ban
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban") == 0) {
				dynamic_pass_failure_ban = config_switch(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_time") == 0) {
				dynamic_pass_failure_ban_time = atoi(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_many") == 0) {
				dynamic_pass_failure_ban_how_many = atoi(w2);
			} else if (strcasecmp(w1, "dynamic_pass_failure_ban_how_long") == 0) {
				dynamic_pass_failure_ban_how_long = atoi(w2);
			} else if (strcasecmp(w1, "import") == 0) {
				login_config_read(w2);
			} else if(strcasecmp(w1, "check_client_version") == 0){	//Added by Sirius for client version check
				check_client_version = config_switch(w2);
			}else if(strcasecmp(w1, "client_version_to_connect") == 0){	//Added by Sirius for client version check
				client_version_to_connect = atoi(w2);			//Added by Sirius for client version check
			} else if (strcasecmp(w1, "console") == 0) {
				console = config_switch(w2);
			} else if (strcasecmp(w1, "allowed_regs") == 0) {			
				allowed_regs = atoi(w2);
			} else if (strcasecmp(w1, "time_allowed") == 0) {
				
				time_allowed = atoi(w2);
				
			}
		}
	}
	fclose(fp);

	ShowMessage("---End reading of Login Server configuration file.\n");

	return 0;
}

//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void) {
	if (admin_state != 0 && admin_state != 1) {
		ShowMessage("***WARNING: Invalid value for admin_state parameter -> set to 0 (no remote admin).\n");
		admin_state = 0;
	}

	if (admin_state == 1) {
		if (admin_pass[0] == '\0') {
			ShowMessage("***WARNING: Administrator password is void (admin_pass).\n");
		} else if (strcmp(admin_pass, "admin") == 0) {
			ShowMessage("***WARNING: You are using the default administrator password (admin_pass).\n");
			ShowMessage("            We highly recommend that you change it.\n");
		}
	}

	if (gm_pass[0] == '\0') {
		ShowMessage("***WARNING: 'To GM become' password is void (gm_pass).\n");
		ShowMessage("            We highly recommend that you set one password.\n");
	} else if (strcmp(gm_pass, "gm") == 0) {
		ShowMessage("***WARNING: You are using the default GM password (gm_pass).\n");
		ShowMessage("            We highly recommend that you change it.\n");
	}

	if (level_new_gm < 0 || level_new_gm > 99) {
		ShowMessage("***WARNING: Invalid value for level_new_gm parameter -> set to 60 (default).\n");
		level_new_gm = 60;
	}

	if (new_account_flag != 0 && new_account_flag != 1) {
		ShowMessage("***WARNING: Invalid value for new_account parameter -> set to 0 (no new account).\n");
		new_account_flag = 0;
	}

	if (login_port < 1024) {
		ShowMessage("***WARNING: Invalid value for login_port parameter -> set to 6900 (default).\n");
		login_port = 6900;
	}

	if (gm_account_filename_check_timer < 0) {
		ShowMessage("***WARNING: Invalid value for gm_account_filename_check_timer parameter.\n");
		ShowMessage("            -> set to 15 sec (default).\n");
		gm_account_filename_check_timer = 15;
	} else if (gm_account_filename_check_timer == 1) {
		ShowMessage("***WARNING: Invalid value for gm_account_filename_check_timer parameter.\n");
		ShowMessage("            -> set to 2 sec (minimum value).\n");
		gm_account_filename_check_timer = 2;
	}

	if (save_unknown_packets != 0 && save_unknown_packets != 1) {
		ShowMessage("WARNING: Invalid value for save_unknown_packets parameter -> set to 0-no save.\n");
		save_unknown_packets = 0;
	}

	if (display_parse_login != 0 && display_parse_login != 1) { // 0: no, 1: yes
		ShowMessage("***WARNING: Invalid value for display_parse_login parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_login = 0;
	}

	if (display_parse_admin != 0 && display_parse_admin != 1) { // 0: no, 1: yes
		ShowMessage("***WARNING: Invalid value for display_parse_admin parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_admin = 0;
	}

	if (display_parse_fromchar < 0 || display_parse_fromchar > 2) { // 0: no, 1: yes (without packet 0x2714), 2: all packets
		ShowMessage("***WARNING: Invalid value for display_parse_fromchar parameter\n");
		ShowMessage("            -> set to 0 (no display).\n");
		display_parse_fromchar = 0;
	}

	if (min_level_to_connect < 0) { // 0: all players, 1-99 at least gm level x
		ShowMessage("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 0 (any player).\n");
		min_level_to_connect = 0;
	} else if (min_level_to_connect > 99) { // 0: all players, 1-99 at least gm level x
		ShowMessage("***WARNING: Invalid value for min_level_to_connect (%d) parameter\n", min_level_to_connect);
		ShowMessage("            -> set to 99 (only GM level 99).\n");
		min_level_to_connect = 99;
	}

	if (add_to_unlimited_account != 0 && add_to_unlimited_account != 1) { // 0: no, 1: yes
		ShowMessage("***WARNING: Invalid value for add_to_unlimited_account parameter\n");
		ShowMessage("            -> set to 0 (impossible to add a time to an unlimited account).\n");
		add_to_unlimited_account = 0;
	}

	if (start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowMessage("***WARNING: Invalid value for start_limited_time parameter\n");
		ShowMessage("            -> set to -1 (new accounts are created with unlimited time).\n");
		start_limited_time = -1;
	}

	if (check_ip_flag != 0 && check_ip_flag != 1) { // 0: no, 1: yes
		ShowMessage("***WARNING: Invalid value for check_ip_flag parameter\n");
		ShowMessage("            -> set to 1 (check players ip between login-server & char-server).\n");
		check_ip_flag = 1;
	}

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowMessage("***WARNING: The IP security order is 'deny,allow' (allow if not deny).\n");
			ShowMessage("            And you refuse ALL IP.\n");
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			ShowMessage("***WARNING: The IP security order is 'allow,deny' (deny if not allow).\n");
			ShowMessage("            But, NO IP IS AUTHORISED!\n");
		}
	} else { // ACO_MUTUAL_FAILTURE
		if (access_allownum == 0) {
			ShowMessage("***WARNING: The IP security order is 'mutual-failture'\n");
			ShowMessage("            (allow if in the allow list and not in the deny list).\n");
			ShowMessage("            But, NO IP IS AUTHORISED!\n");
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowMessage("***WARNING: The IP security order is mutual-failture\n");
			ShowMessage("            (allow if in the allow list and not in the deny list).\n");
			ShowMessage("            But, you refuse ALL IP!\n");
		}
	}

	if (dynamic_pass_failure_ban != 0) {
		if (dynamic_pass_failure_ban_time < 1) {
			ShowMessage("***WARNING: Invalid value for dynamic_pass_failure_ban_time (%d) parameter\n", dynamic_pass_failure_ban_time);
			ShowMessage("            -> set to 5 (5 minutes to look number of invalid passwords.\n");
			dynamic_pass_failure_ban_time = 5;
		}
		if (dynamic_pass_failure_ban_how_many < 1) {
			ShowMessage("***WARNING: Invalid value for dynamic_pass_failure_ban_how_many (%d) parameter\n", dynamic_pass_failure_ban_how_many);
			ShowMessage("            -> set to 3 (3 invalid passwords before to temporarily ban.\n");
			dynamic_pass_failure_ban_how_many = 3;
		}
		if (dynamic_pass_failure_ban_how_long < 1) {
			ShowMessage("***WARNING: Invalid value for dynamic_pass_failure_ban_how_long (%d) parameter\n", dynamic_pass_failure_ban_how_long);
			ShowMessage("            -> set to 1 (1 minute of temporarily ban.\n");
			dynamic_pass_failure_ban_how_long = 1;
		}
	}

	return;
}

//-------------------------------
// Save configuration in log file
//-------------------------------
void save_config_in_log(void) {
	int i;

	// a newline in the log...
	login_log("");
	login_log("The login-server starting..." RETCODE);

	// save configuration in log file
	login_log("The configuration of the server is set:" RETCODE);

	if (admin_state != 1)
		login_log("- with no remote administration." RETCODE);
	else if (admin_pass[0] == '\0')
		login_log("- with a remote administration with a VOID password." RETCODE);
	else if (strcmp(admin_pass, "admin") == 0)
		login_log("- with a remote administration with the DEFAULT password." RETCODE);
	else
		login_log("- with a remote administration with the password of %d character(s)." RETCODE, strlen(admin_pass));
	if (access_ladmin_allownum == 0 || (access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0')) {
		login_log("- to accept any IP for remote administration" RETCODE);
	} else {
		login_log("- to accept following IP for remote administration:" RETCODE);
		for(i = 0; i < access_ladmin_allownum; i++)
			login_log("  %s" RETCODE, (char *)(access_ladmin_allow + i * ACO_STRSIZE));
	}

	if (gm_pass[0] == '\0')
		login_log("- with a VOID 'To GM become' password (gm_pass)." RETCODE);
	else if (strcmp(gm_pass, "gm") == 0)
		login_log("- with the DEFAULT 'To GM become' password (gm_pass)." RETCODE);
	else
		login_log("- with a 'To GM become' password (gm_pass) of %d character(s)." RETCODE, strlen(gm_pass));
	if (level_new_gm == 0)
		login_log("- to refuse any creation of GM with @gm." RETCODE);
	else
		login_log("- to create GM with level '%d' when @gm is used." RETCODE, level_new_gm);

	if (new_account_flag == 1)
		login_log("- to ALLOW new users (with _F/_M)." RETCODE);
	else
		login_log("- to NOT ALLOW new users (with _F/_M)." RETCODE);
	login_log("- with port: %d." RETCODE, login_port);
	login_log("- with the accounts file name: '%s'." RETCODE, account_filename);
	login_log("- with the GM accounts file name: '%s'." RETCODE, GM_account_filename);
	if (gm_account_filename_check_timer == 0)
		login_log("- to NOT check GM accounts file modifications." RETCODE);
	else
		login_log("- to check GM accounts file modifications every %d seconds." RETCODE, gm_account_filename_check_timer);

	if (use_md5_passwds == 0)
		login_log("- to save password in plain text." RETCODE);
	else
		login_log("- to save password with MD5 encrypting." RETCODE);

	// not necessary to log the 'login_log_filename', we are inside :)

	login_log("- with the unknown packets file name: '%s'." RETCODE, login_log_unknown_packets_filename);
	if (save_unknown_packets)
		login_log("- to SAVE all unkown packets." RETCODE);
	else
		login_log("- to SAVE only unkown packets sending by a char-server or a remote administration." RETCODE);
	if (display_parse_login)
		login_log("- to display normal parse packets on console." RETCODE);
	else
		login_log("- to NOT display normal parse packets on console." RETCODE);
	if (display_parse_admin)
		login_log("- to display administration parse packets on console." RETCODE);
	else
		login_log("- to NOT display administration parse packets on console." RETCODE);
	if (display_parse_fromchar)
		login_log("- to display char-server parse packets on console." RETCODE);
	else
		login_log("- to NOT display char-server parse packets on console." RETCODE);

	if (min_level_to_connect == 0) // 0: all players, 1-99 at least gm level x
		login_log("- with no minimum level for connection." RETCODE);
	else if (min_level_to_connect == 99)
		login_log("- to accept only GM with level 99." RETCODE);
	else
		login_log("- to accept only GM with level %d or more." RETCODE, min_level_to_connect);

	if (add_to_unlimited_account)
		login_log("- to authorize adjustment (with timeadd ladmin) on an unlimited account." RETCODE);
	else
		login_log("- to refuse adjustment (with timeadd ladmin) on an unlimited account. You must use timeset (ladmin command) before." RETCODE);

	if (start_limited_time < 0)
		login_log("- to create new accounts with an unlimited time." RETCODE);
	else if (start_limited_time == 0)
		login_log("- to create new accounts with a limited time: time of creation." RETCODE);
	else
		login_log("- to create new accounts with a limited time: time of creation + %d second(s)." RETCODE, start_limited_time);

	if (check_ip_flag)
		login_log("- with control of players IP between login-server and char-server." RETCODE);
	else
		login_log("- to not check players IP between login-server and char-server." RETCODE);

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 0) {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse no IP." RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; i++)
				login_log("  %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_allownum == 1 && access_allow[0] == '\0') {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). You authorise ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). Authorised IP are:" RETCODE);
			for(i = 0; i < access_allownum; i++)
				login_log("  %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
		}
	} else { // ACO_MUTUAL_FAILTURE
		login_log("- with the IP security order: 'mutual-failture' (allow if in the allow list and not in the deny list)." RETCODE);
		if (access_allownum == 0) {
			login_log("  But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("  But, you refuse ALL IP!" RETCODE);
		} else {
			if (access_allownum == 1 && access_allow[0] == '\0') {
				login_log("  You authorise ALL IP." RETCODE);
			} else {
				login_log("  Authorised IP are:" RETCODE);
				for(i = 0; i < access_allownum; i++)
					login_log("    %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
			}
			login_log("  Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; i++)
				login_log("    %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}

		// dynamic password error ban
		if (dynamic_pass_failure_ban == 0)
			login_log("- with NO dynamic password error ban." RETCODE);
		else {
			ShowMessage("- with a dynamic password error ban:" RETCODE);
			ShowMessage("  After %d invalid password in %d minutes" RETCODE, dynamic_pass_failure_ban_how_many, dynamic_pass_failure_ban_time);
			ShowMessage("  IP is banned for %d minutes" RETCODE, dynamic_pass_failure_ban_how_long);
		}
	}
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void) {
	size_t i;
	ShowStatus("Terminating...\n");
	///////////////////////////////////////////////////////////////////////////
	mmo_auth_sync();
	numdb_final(online_db, online_db_final);

	if(auth_dat) aFree(auth_dat);
	if(gm_account_db) aFree(gm_account_db);
	if(access_ladmin_allow) aFree(access_ladmin_allow);
	if(access_allow) aFree(access_allow);
	if(access_deny) aFree(access_deny);

	///////////////////////////////////////////////////////////////////////////
	// delete sessions
	for (i = 0; i < fd_max; i++) {
		if (session[i])
			session_Delete(i);
	}
	// clear externaly stored fd's
	for (i = 0; i < MAX_SERVERS; i++) {
			server[i].fd = -1;
		}
	login_fd = -1;
	///////////////////////////////////////////////////////////////////////////
	login_log("----End of login-server (normal end with closing of all files)." RETCODE);
	if(log_fp)
		fclose(log_fp);
	///////////////////////////////////////////////////////////////////////////
	ShowStatus("Successfully terminated.\n");
}

//------------------------------
// Main function of login-server
//------------------------------
unsigned char getServerType()
{
	return ATHENA_SERVER_LOGIN | ATHENA_SERVER_CORE;
}
int do_init(int argc, char **argv) {
	int i, j;

	// read login-server configuration
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
	save_config_in_log(); // not before, because log file name can be changed
	login_lan_config_read((argc > 1) ? argv[1] : LAN_CONF_NAME);

	srand(time(NULL)^3141592654UL);

	for(i = 0; i< AUTH_FIFO_SIZE; i++)
		auth_fifo[i].delflag = 1;
	for(i = 0; i < MAX_SERVERS; i++)
		server[i].fd = -1;

	//gm_account_db = numdb_init();
	gm_account_db = NULL;
	GM_num = 0;
	GM_max = 0;
	mmo_auth_init();
	read_gm_account();
	set_defaultparse(parse_login);
	// Online user database init
    online_db = numdb_init();

	login_fd = make_listen(login_ip,login_port);

	add_timer_func_list(check_auth_sync, "check_auth_sync");
	add_timer_interval(gettick() + 60000, 60000, check_auth_sync, 0, 0); // every 60 sec we check if we must save accounts file (only if necessary to save)



	// add timer to check GM accounts file modification
	j = gm_account_filename_check_timer;
	if (j <= 0) // if we would not to check, we check every 60 sec, just to have timer (if we change timer, is was not necessary to check if timer already exists)
		j = 60;

	add_timer_func_list(check_GM_file, "check_GM_file");
	add_timer_interval(gettick() + j * 1000, j * 1000, check_GM_file, 0, 0); // every x sec we check if gm file has been changed

	if(console) {
		set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	login_log("The login-server is ready (Server is listening on the port %d)." RETCODE, login_port);
	ShowMessage("The login-server is "CL_BT_GREEN"ready"CL_NORM" (Server is listening on the port %d).\n\n", login_port);

	return 0;
}
