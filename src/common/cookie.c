// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/md5calc.h"
#include "../common/timer.h"
#include "cookie.h"

#include <stdlib.h> // rand
#include <string.h> // memset,memcpy,memcmp


/// Timeout timer.
/// Triggers when the cookie expires.
/// Either calls the callback or resets the cookie data.
static int cookie_timeout_timer(int tid, unsigned int tick, int id, intptr data)
{
	struct s_cookie* cookie = (struct s_cookie*)data;
	if( cookie == NULL || cookie->timeout_timer != tid )
		return 0;// invalid
	if( cookie->len == 0 )
		return 0;// already expired

	cookie->timeout_timer = INVALID_TIMER;
	if( cookie->on_timeout != NULL )
		cookie->on_timeout(cookie->callback_data);
	else
	{
		cookie->len = 0;
		memset(cookie->data, 0, MAX_COOKIE_LEN);
	}
	return 0;
}


/// Initializes the cookie.
void cookie_init(struct s_cookie* cookie)
{
	cookie->len = 0;
	memset(cookie->data, 0, MAX_COOKIE_LEN);
	cookie->timeout = 0;
	cookie->timeout_timer = INVALID_TIMER;
	cookie->callback_data = 0;
	cookie->on_timeout = NULL;
}


/// Destroys the cookie.
void cookie_destroy(struct s_cookie* cookie)
{
	cookie_timeout_stop(cookie);
	cookie_init(cookie);
}


/// Generates random cookie data.
void cookie_generate(struct s_cookie* cookie)
{
	cookie->len = (uint16)(MAX_COOKIE_LEN/2 + rand() % (MAX_COOKIE_LEN/2));
	memset(cookie->data, 0, MAX_COOKIE_LEN);
	MD5_Salt(cookie->len, cookie->data);
}


/// Sets the cookie data.
/// Truncates the data if too big.
void cookie_set(struct s_cookie* cookie, uint16 len, const char* data)
{
	if( len > MAX_COOKIE_LEN )
		len = MAX_COOKIE_LEN;
	cookie->len = len;
	memset(cookie->data, 0, MAX_COOKIE_LEN);
	memcpy(cookie->data, data, len);
}


/// Starts the timeout timer.
void cookie_timeout_start(struct s_cookie* cookie)
{
	static bool is_registered = false;
	if( !is_registered )
	{
		add_timer_func_list(cookie_timeout_timer, "cookie_timeout_timer");
		is_registered = true;
	}

	if( cookie->timeout_timer != INVALID_TIMER )
		return;// already running
	cookie->timeout_timer = add_timer(gettick() + cookie->timeout, cookie_timeout_timer, 0, (intptr)cookie);
}


/// Stops the timeout timer.
void cookie_timeout_stop(struct s_cookie* cookie)
{
	if( cookie->timeout_timer == INVALID_TIMER )
		return;// not running
	delete_timer(cookie->timeout_timer, cookie_timeout_timer);
	cookie->timeout = INVALID_TIMER;
}


/// Compares the data of the cookie.
int cookie_compare(struct s_cookie* cookie, uint16 len, const char* data)
{
	uint16 n = (cookie->len < len? cookie->len: len);
	int ret = memcmp(cookie->data, data, n);
	if( ret == 0 )
		ret = (int)len - (int)cookie->len;
	return ret;
}
