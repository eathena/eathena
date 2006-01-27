#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"

#ifdef COUNT_GLOBALS
int global::sGlobalCount=0;
#ifdef DEBUG
global::_globalcount global::gc;
global::_globalcount::_globalcount()
{
	printf("init countint of global objects\n");
}
global::_globalcount::~_globalcount()
{
	printf("\nterminating global objects.\n");
#ifndef SINGLETHREAD
	sleep(1000); //give some time to clean up
#endif
	if( global::getcount() > 0 )
	{
		printf("global object count: %i\n", global::getcount());
		printf("still not dealloced everything, may be leaky or just static/global objects.\n");
	}
	else
	{
		printf("global objects clear.\n");
	}
}
#endif
#endif





int unixerrno() 
{
#ifdef WIN32
	switch( GetLastError() )
	{
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:      return ENOENT;
	case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
	case ERROR_ACCESS_DENIED:
	case ERROR_SHARING_VIOLATION:   return EACCES;
	case ERROR_INVALID_HANDLE:      return EBADF;
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:         return ENOMEM;
	case ERROR_INVALID_DRIVE:       return ENODEV;
	case ERROR_WRITE_PROTECT:       return EROFS;
	case ERROR_FILE_EXISTS:         return EEXIST;
	case ERROR_BROKEN_PIPE:         return EPIPE;
	case ERROR_DISK_FULL:           return ENOSPC;
	case ERROR_SEEK_ON_DEVICE:      return ESPIPE;
	default: return EIO;
	}
#else
	return errno;
#endif
}


//////////////////////////////////////////////////////////////////////////
// This function gives error messages for most frequently raising 
// IO errors. If the function returns NULL a generic message
// can be given, e.g. "I/O error".
//////////////////////////////////////////////////////////////////////////

const char* unixerrmsg(int code)
{
	switch(code)
	{
	case EBADF:  return "Invalid file descriptor";
	case ESPIPE: return "Can not seek on this device";
	case ENOENT: return "No such file or directory";
	case EMFILE: return "Too many open files";
	case EACCES: return "Access denied";
	case ENOMEM: return "Not enough memory";
	case ENODEV: return "No such device";
	case EROFS:  return "Read-only file system";
	case EEXIST: return "File already exists";
	case ENOSPC: return "Disk full";
	case EPIPE:  return "Broken pipe";
	default: return NULL;
	}
}
