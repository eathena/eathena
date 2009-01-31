#ifndef _IF_CLIENT_H_
#define _IF_CLIENT_H_

int mmo_char_send006b(int fd, struct char_session_data* sd);
int parse_client(int fd);

#endif /* _IF_CLIENT_H_ */
