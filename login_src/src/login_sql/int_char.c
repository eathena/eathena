// Testing int_char.c

#include "main.h"

int server_freezeflag[MAX_SERVERS]; // Char-server anti-freeze system. Counter. 5 ok, 4...0 freezed

//-------------------------------------------
// Send to all character servers.
//-------------------------------------------
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;
	c = 0;
	for(i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) > 0 && fd != sfd) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}
	return c;
}

//-----------------------------------------------------
// Char server packet parsing functions.
//-----------------------------------------------------
void parse_char_charserver(int fd){
	if (RFIFOREST(fd) < 19)	return;

	int account_id, i;

	account_id = RFIFOL(fd,2); // speed up

	for(i=0;i<AUTH_FIFO_SIZE;i++){
		if (
		auth_fifo[i].account_id == RFIFOL(fd,2) &&
		auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
		auth_fifo[i].login_id2 == RFIFOL(fd,10) &&
		auth_fifo[i].sex == RFIFOB(fd,14) &&
		auth_fifo[i].ip == RFIFOL(fd,15) &&
		!auth_fifo[i].delflag
		) {
			auth_fifo[i].delflag = 1;
			#ifdef CHAR_DEBUG
			printf("auth -> %d\n", i);
			#endif
			break;
		}
	}
	if (i != AUTH_FIFO_SIZE) { // send account_reg
		#ifdef CHAR_DEBUG
		printf("sending account_reg for %d",RFIFOL(fd,2));
		#endif
		int p;
		time_t connect_until_time = 0;
		char email[40] = "";
		account_id=RFIFOL(fd,2);
		sql_query("SELECT `email` FROM `ibf_members` WHERE `id`='%d'", account_id);
		sql_res = mysql_store_result(&mysql_handle) ;
		if (sql_res) {
			sql_row = mysql_fetch_row(sql_res);
//			connect_until_time = atol(sql_row[1]);
			strcpy(email, sql_row[0]);
		}
		mysql_free_result(sql_res);
		if (RFIFOL(fd,2) > 0) {
			sql_query("SELECT `str`,`value` FROM `global_reg_value` WHERE `type`='1' AND `account_id`='%d'",account_id);
			sql_res = mysql_store_result(&mysql_handle) ;
			if (sql_res) {
				WFIFOW(fd,0) = 0x2729;
				WFIFOL(fd,4) = RFIFOL(fd,2);
				for(p = 8; (sql_row = mysql_fetch_row(sql_res));p+=36){
					memcpy(WFIFOP(fd,p), sql_row[0], 32);
					WFIFOL(fd,p+32) = atoi(sql_row[1]);
				}
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,p);
				#ifdef CHAR_DEBUG
				printf("account_reg2 send : login->char (auth fifo)\n");
				#endif
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = RFIFOL(fd,2);
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), email, 40);
				WFIFOL(fd,47) = (unsigned long) connect_until_time;
				WFIFOSET(fd,51);
			}
			mysql_free_result(sql_res);
		}
	} else {
		WFIFOW(fd,0) = 0x2713;
		WFIFOL(fd,2) = RFIFOL(fd,2);
		WFIFOB(fd,6) = 1;
		WFIFOSET(fd,51);
	}
	RFIFOSKIP(fd,19);
}
void parse_char_users(int fd, int id){
	if (RFIFOREST(fd) < 6) return;
	if (server[id].users != RFIFOL(fd,2)){
		printf("set users %s : %d  Server ID: %d\n", server[id].name, RFIFOL(fd,2), id);
		server[id].users = RFIFOL(fd,2);
		server_freezeflag[id] = 5; // Char anti-freeze system. Counter. 5 ok, 4...0 freezed
		sql_query("UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id);
	}
	RFIFOSKIP(fd,6);
}

void parse_char_return(int fd, int id){

	if (RFIFOREST(fd) < 6)return;
	int account_id;

	char email[40] = "";
	account_id=RFIFOL(fd,2);
	sql_query("SELECT `email` FROM `ibf_members` WHERE `id`='%d'",account_id);
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);
		strcpy(email, sql_row[0]);
	}
	mysql_free_result(sql_res);
	#ifdef CHAR_DEBUG
	printf("parse_fromchar: E-mail request from '%s' server (concerned account: %d)\n", server[id].name, RFIFOL(fd,2));
	#endif
	WFIFOW(fd,0) = 0x2717;
	WFIFOL(fd,2) = RFIFOL(fd,2);
	memcpy(WFIFOP(fd, 6), email, 40);
	WFIFOL(fd,46) = 0;
	WFIFOSET(fd,50);
	RFIFOSKIP(fd,6);
}

void parse_char_save_reg(int fd){
	if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2)) return;
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void parse_char_packet(int fd){
	printf("parse_fromchar: unknown packet %x! .\n", RFIFOW(fd,0));
	session[fd]->eof = 1;
}


//-----------------------------------------------------
// char-server packet parse
//-----------------------------------------------------
int parse_fromchar(int fd){
	int id;
	for(id = 0; id < MAX_SERVERS; id++){
		if (server_fd[id] == fd) {
			#ifdef CHAR_DEBUG
			printf("Char server ID: %d NAME: %s has fully connected\n",id,server[id].name);
			#endif
			break;
		}
	}

	if (id == MAX_SERVERS || session[fd]->eof) {
		if (id < MAX_SERVERS) {
			#ifdef CHAR_DEBUG
			printf("Char-server '%s' has disconnected.\n", server[id].name);
			#endif
			server_fd[id] = -1;
			memset(&server[id], 0, sizeof(struct mmo_char_server));
			sql_query("DELETE FROM `sstatus` WHERE `index`='%d'", id);
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
		#ifdef CHAR_DEBUG
		printf("char_parse: %d %d packet case=%x of MAP SERVER NAME: %s \n", fd, RFIFOREST(fd), RFIFOW(fd, 0), server[id].name);
		#endif
		switch (RFIFOW(fd,0)) {
			case 0x2712: parse_char_charserver(fd);	break;
			case 0x2714: parse_char_users(fd,id);	break;
			case 0x2716: parse_char_return(fd,id);	break;
			case 0x2728: parse_char_save_reg(fd);	break;
			default:
				parse_char_packet(fd);
				return 0;
				break;
		}
	}

	return 0;
}
