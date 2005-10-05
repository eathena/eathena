// Testing main.c

#include "main.h"

// Standards in the lege_login_conf setup, add more as you please, and add it to the SQL conf read function.
int min_level_to_connect = 0; // minimum level of player/GM (0: player, 1-99: gm) to connect on the server

//---------
// Packet results affecting client:
//---------

	// For messages on 0x6a packet.

	// return 1  == 			Wrong password entered.
	// return 2  == line 9		ID is no longer valid
	// return 3  ==	line 10		Rejected from server, Server problem.
	// return 4  ==				Not validated email yet
	// return 5  == line 311	Server is down
	// return 6  == line 450	cant log in till.  -- Temp ban
	// return 7  == line 440	Banned for exploiting
	// return 8  == line 682	banned for an hour for too many password attempts
	// return 9  == line 704	banned for scamming.
	// return 10 == line 705	banned misc
	// return 11 == line 706	banned for breaking a rule
	// return 12 == line 707	temp banned for wrong password
	// return 13 == line 708
	// return 14 == line 709
	// return 15 == line 710	banned for botting
//		0x81
//		Packet information, must test with multiple results, and see what answers we get from client.
//
//		so far, fd,2 at setting 1 is closed.
//
//					WFIFOW(fd,0) = 0x81;
//					WFIFOL(fd,2) = 1; // 01 = Server closed
//					WFIFOSET(fd,3);

//-----------------------------------------------------
// MySQL setup.
//-----------------------------------------------------

int login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";


//-----------------------------------------------------
// global variable
//-----------------------------------------------------
int login_port = 6900;

int server_freezeflag[MAX_SERVERS]; // Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed
int login_fd;

//-----------------------------------------------------
//BANNED IP CHECK.
//-----------------------------------------------------
int ip_ban_check(int tid, unsigned int tick, int id, int data){
	sql_query("DELETE FROM `ipbanlist` WHERE `timer` <= UNIX_TIMESTAMP()",NULL);
	return 0;
}

//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
int remove_control_chars(unsigned char *str) {
	int i;
	int change = 0;

	for(i = 0; str[i]; i++) {
		if (str[i] < 32) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}


//-----------------------------------------------------
// Char server Anti freeze system
//-----------------------------------------------------
int char_anti_freeze_system(int tid, unsigned int tick, int id, int data) {
	int i;

	for(i = 0; i < MAX_SERVERS; i++) {
		if (server_fd[i] >= 0) {// if char-server is online
			if (server_freezeflag[i]-- < 1) {// Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed
				session[server_fd[i]]->eof = 1;
			}
		}
	}

	return 0;
}


//-------------------------------------------------
// Return numerical value of a switch configuration
// on/off, english, français, deutsch, español
//-------------------------------------------------
int config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0)
		return 0;

	return atoi(str);
}

void mmo_db_close(void) {
	sql_close();
	int i, fd;
	for (i = 0; i < MAX_SERVERS; i++) {	if ((fd = server_fd[i]) >= 0) delete_session(fd); }
	delete_session(login_fd);
}



//-----------------------------------------------------
// reading configuration
//-----------------------------------------------------
// all settings will either be set hardcoded or in the mysql database.
// if none is found in the database, it will input a table in the active databasse it is set to be in.
// if no database is found, and there is no login table, then it will error out. and let you know.

int login_config_read(int tid, unsigned int tick, int id, int data){
	sql_query("SELECT `value` FROM `login_settings` WHERE `setting`='min_level_to_connect'",NULL);
	if((sql_res = mysql_store_result(&mysql_handle)) && (sql_row = mysql_fetch_row(sql_res))){
		min_level_to_connect = atoi(sql_row[0]);
		printf("Set min_level_to_connect to %d \n",min_level_to_connect);
	}
	return 0;
}

void login_config_(void){ /* Kalaspuff, to get login_db */
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	printf("reading configure: %s\n", "login_athena.conf");

	if ((fp = fopen("login_athena.conf", "r")) == NULL) {
		printf("file not found: %s\n", "login_athena.conf");
		exit(1);
	}

	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if(strcmpi(w1, "login_server_ip") == 0) {
			strcpy(login_server_ip, w2);
		}else if(strcmpi(w1,"login_server_id")==0){
			strcpy(login_server_id,w2);
		}else if(strcmpi(w1,"login_server_pw")==0){
			strcpy(login_server_pw,w2);
		}else if(strcmpi(w1,"login_server_db")==0){
			strcpy(login_server_db,w2);
		}else if(strcmpi(w1,"login_server_port")==0){
			strcpy(login_server_port,w2);
		}

	}
	fclose(fp);
	printf("reading configure done.....\n");
}


int sql_cnt_check(int tid, unsigned int tick, int id, int data){

	printf("SQL Statistics: Interval of 1 minute\n");
	printf(
		"SQL Statistics: Inserts=%ld Selects=%ld Deletes:%ld Updates:%ld Replaces:%ld Total:%ld\n",
		sql_icnt, sql_scnt, sql_dcnt, sql_ucnt, sql_rcnt, (sql_icnt + sql_scnt + sql_dcnt + sql_ucnt + sql_rcnt + sql_cnt));

	sql_icnt=sql_scnt=sql_dcnt=sql_ucnt=sql_rcnt=sql_cnt=0;

	return 0;
}

int do_init(int argc,char **argv){
	int i;

	for(i=0;i<AUTH_FIFO_SIZE;i++)	auth_fifo[i].delflag=1;
	for(i=0;i<MAX_SERVERS;i++)		server_fd[i]=-1;

	login_config_();

	//server port open & binding
	login_fd = make_listen_port(login_port);

	//Start SQL server connection
	sql_connect(login_server_ip, login_server_id, login_server_pw, login_server_db, login_server_port);

	// Set to run when closing server
	set_termfunc(mmo_db_close);

	// Default packet parse when server starts up
	set_defaultparse(parse_login);

	add_timer_func_list(char_anti_freeze_system, "char_anti_freeze_system");

	// every 15 sec (users are sended every 5 sec)
	add_timer_interval(gettick()+1000, char_anti_freeze_system, 0, 0, 15 * 1000);

	add_timer_interval(gettick()+10, ip_ban_check,0,0,60*1000);

	// Check SQL conf for settings every 1 minute
	add_timer_interval(gettick()+10, login_config_read,0,0,60*1000);

	// Check SQL count and print out info
	add_timer_interval(gettick()+10, sql_cnt_check,0,0,60*1000);

	printf("\033[1;32mready\033[0m (Server is listening on the port %d).\n\n", login_port);

	return 0;
}

