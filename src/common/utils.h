// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "basetypes.h"
#include "basefile.h"



//////////////////////////////
#ifdef WIN32
//////////////////////////////

#define RETCODE	"\r\n"
#define RET RETCODE

//////////////////////////////
#else/////////////////////////
//////////////////////////////

#define RETCODE "\n"
#define RET RETCODE

//////////////////////////////
#endif////////////////////////
//////////////////////////////


void dump(unsigned char *buffer, size_t num);

bool email_check(const char *email);
bool remove_control_chars(char *str);
const char *strcpytolower(char *tar, const char *str);
const char *strcpytolower(char *tar, size_t sz, const char *str);
const char *safestrcpy(char *tar, size_t sz, const char *src);
const char *replacecpy(char *tar, size_t sz, const char* src, char rplc='\t', char with=' ');
const char *mapname2buffer(unsigned char *buffer, size_t sz, const char *mapname);
const char *buffer2mapname(char *mapname, size_t sz, const char *buffer);
const char *is_valid_line(const char *line);
size_t prepare_line(char *line);










#endif//COMMON_UTILS_H
