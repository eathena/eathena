#ifndef _IF_LOGIN_H_
#define _IF_LOGIN_H_

int check_connect_login_server(int tid, unsigned int tick, int id, intptr data);
int ping_login_server(int tid, unsigned int tick, int id, intptr data);
int send_accounts_tologin(int tid, unsigned int tick, int id, intptr data);
int parse_fromlogin(int fd);

#endif /* _IF_LOGIN_H_ */
