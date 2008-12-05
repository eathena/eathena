// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __CHARLOG_H_INCLUDED__
#define __CHARLOG_H_INCLUDED__


void charlog_log(int char_id, int account_id, int slot, const char* name, const char* msg, ...);
bool charlog_init(void);
bool charlog_final(void);
bool charlog_config_read(const char* w1, const char* w2);


#endif // __CHARLOG_H_INCLUDED__
