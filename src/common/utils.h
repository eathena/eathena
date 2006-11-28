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


/// dumps an uchar array as hex and ascii
void dump(const unsigned char *buf, const size_t sz, FILE* file=stdout);
/// test for valid email structure
bool email_check(const char *email);
/// suppress control characters in a string.
bool remove_control_chars(char *str);
/// create a lowercase copy of a string.
const char *strcpytolower(char *tar, size_t sz, const char *src);
/// create a copy of a string with sizecheck.
const char *safestrcpy(char *tar, size_t sz, const char *src);
/// create a copy of a string and do single char replacements.
const char *replacecpy(char *tar, size_t sz, const char* src, char rplc='\t', char with=' ');
/// copy a mapname to transmission buffer and add the client extension.
const char *mapname2buffer(unsigned char *buf, size_t sz, const char *mapname);
/// copy a mapname from (transmission) buffer and remove the extension.
const char *buffer2mapname(char *mapname, size_t sz, const char *buf);
/// check if line is empty or a comment line.
const char *is_valid_line(const char *line);
/// remove whitespaces and comments.
size_t prepare_line(char *line);



#endif//COMMON_UTILS_H
