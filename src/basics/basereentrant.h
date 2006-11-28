#ifndef __BASEREENTRANT_H__
#define __BASEREENTRANT_H__

///////////////////////////////////////////////////////////////////////////////
// collection of reentrant function for systems that don't support them
#include <stdio.h>

const char* strerror(int err, char* buf, size_t size);


#ifdef WIN32

char* strtok_r( char *strToken, const char *strDelimit, char**pt);

inline const char* strerror_r(int err, char* buf, size_t size)
{
	return strerror(err, buf, size);
}

#endif


#endif//__BASEREENTRANT_H__

