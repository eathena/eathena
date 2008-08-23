// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __CHARLOG_H_INCLUDED__
#define __CHARLOG_H_INCLUDED__


void char_log(char *fmt, ...);
bool charlog_init(void);
bool charlog_final(void);
bool charlog_config_read(const char* w1, const char* w2);


#endif // __CHARLOG_H_INCLUDED__
