// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __INTERLOG_H_INCLUDED__
#define __INTERLOG_H_INCLUDED__


void interlog_log(const char* msg, ...);
bool interlog_init(void);
bool interlog_final(void);
bool interlog_config_read(const char* w1, const char* w2);


#endif // __INTERLOG_H_INCLUDED__
