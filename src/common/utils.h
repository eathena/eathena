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

int config_switch(const char *str);
bool email_check(const char *email);
bool remove_control_chars(char *str);
const char *strcpytolower(char *tar, const char *str);
const char *strcpytolower(char *tar, size_t sz, const char *str);
const char *safestrcpy(char *tar, const char *src, size_t cnt);
const char *replacecpy(char *tar, const char* src, size_t sz, char rplc='\t', char with=' ');
const char *mapname2buffer(unsigned char *buffer, const char *mapname, size_t cnt);
const char *is_valid_line(const char *line);
size_t prepare_line(char *line);






#endif//COMMON_UTILS_H
