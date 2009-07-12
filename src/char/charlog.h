// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __CHARLOG_H_INCLUDED__
#define __CHARLOG_H_INCLUDED__


void charlog_create(void);
bool charlog_init(void);
void charlog_final(void);
void charlog_config_read(const char* w1, const char* w2);
void charlog_log(int char_id, int account_id, int slot, const char* name, const char* msg, ...);


#endif // __CHARLOG_H_INCLUDED__
