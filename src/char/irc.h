#ifndef IRC_H
#define IRC_H

#include "socket.h"


// IRC Access levels [Zido]
#define	ACCESS_OWNER	5
#define	ACCESS_SOP		4
#define	ACCESS_OP		3
#define	ACCESS_HOP		2
#define	ACCESS_VOICE	1
#define ACCESS_NORM		0



void irc_announce(int fd);

void irc_final(void);
void irc_init(void);


#endif//IRC_H
