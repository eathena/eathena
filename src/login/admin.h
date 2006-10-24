// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ADMIN_H_
#define _ADMIN_H_


extern bool admin_state;
extern char admin_pass[24];
extern bool display_parse_admin;


int parse_admin(int fd);

#endif//_ADMIN_H_

