#ifndef _MAIN_H_
#define _MAIN_H_
//#define LOGIN_DEBUG
//#define CHAR_DEBUG

#include <stdio.h> // basic functions
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>
#include <stdarg.h> // For custom Query

#include "md5calc.h"
#include "strlib.h"
#include "timer.h"
#include "core.h"
#include "mmo.h"
#include "version.h"
#include "db.h"
#include "dbaccess.h"
#include "int_char.h"
#include "int_login.h"
#include "socket.h"


#define MAX_SERVERS 30

struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

extern int min_level_to_connect;


struct {
	int account_id,login_id1,login_id2;
	int ip,sex,delflag;
} auth_fifo[AUTH_FIFO_SIZE];

#endif
