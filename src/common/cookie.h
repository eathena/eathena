// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _COMMON_COOKIE_H_
#define _COMMON_COOKIE_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif

typedef void (*f_cookie_callback)(intptr data);

#define MAX_COOKIE_LEN 256
struct s_cookie
{
	uint16 len;
	char data[MAX_COOKIE_LEN];

	uint32 timeout; //< How long the cookie lasts for before expiring
	int timeout_timer;

	intptr callback_data; //< user-defined data
	f_cookie_callback on_timeout; //< Called when the cookie expires. If NULL, the cookie data is reset instead.
};

void cookie_init(struct s_cookie* cookie);
void cookie_destroy(struct s_cookie* cookie);

void cookie_generate(struct s_cookie* cookie);
void cookie_set(struct s_cookie* cookie, uint16 len, const char* data);
void cookie_timeout_start(struct s_cookie* cookie);
void cookie_timeout_stop(struct s_cookie* cookie);
bool cookie_expired(struct s_cookie* cookie);
int cookie_compare(struct s_cookie* cookie, uint16 len, const char* data);

#endif /* _COMMON_COOKIE_H_ */
