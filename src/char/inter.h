// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTER_H_
#define _INTER_H_

int inter_init(void);
void inter_final(void);
int inter_save(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);

int inter_check_length(int fd,int length);

int inter_log(char *fmt,...);

#endif
