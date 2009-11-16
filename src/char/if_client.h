// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _IF_CLIENT_H_
#define _IF_CLIENT_H_

int mmo_char_send006b(int fd, struct char_session_data* sd);
int parse_client(int fd);

#endif /* _IF_CLIENT_H_ */
