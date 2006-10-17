#include "basetypes.h"
#include "baseobjects.h"
#include "basesync.h"

NAMESPACE_BEGIN(basics)


#ifdef COUNT_GLOBALS
int global::sGlobalCount=0;
#ifdef DEBUG
global::_globalcount global::gc;
static bool _globalfinished=false;
global::_globalcount::_globalcount()
{
	fprintf(stderr, "init counting of global objects\n");
}
global::_globalcount::~_globalcount()
{
	fprintf(stderr, "\nterminating global objects.\n");
#ifndef SINGLETHREAD
	sleep(1000); //give some time to clean up
#endif
	if( global::getcount() > 0 )
	{
		fprintf(stderr, "still not dealloced everything, may be leaky or just static/global objects.\n");
		fprintf(stderr, "global object count: %i\n", global::getcount());
		fprintf(stderr, "waiting for final check.\n");
	}
	else
	{
		fprintf(stderr, "global objects clear.\n");
	}
	_globalfinished = true;
}
void global::_globalcount::finalcheck()
{
	if(_globalfinished && global::sGlobalCount==0)
		fprintf(stderr, "final check done, everything clear\n");
	else if( global::sGlobalCount==-1)
		fprintf(stderr, "error, global counter underflow\n");
}
#endif
void global::debugprint()
{
	fprintf(stderr, "global object count: %i\n", global::sGlobalCount);
}
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

//////////////////////////////////////////////////////////////////////////
/*

Known UNIX error codes:

EPERM         1          Not owner
ENOENT        2          No such file or directory
ESRCH         3          No such process
EINTR         4          Interrupted system call
EIO           5          I/O error
ENXIO         6          No such device or address
E2BIG         7          Argument list too long
ENOEXEC       8          Exec format error
EBADF         9          Bad file number
ECHILD       10          No spawned processes
EAGAIN       11          No more processes; not enough memory; maximum nesting level reached
ENOMEM       12          Not enough memory
EACCES       13          Permission denied
EFAULT       14          Bad address
ENOTBLK      15          Block device required
EBUSY        16          Mount device busy
EEXIST       17          File exists
EXDEV        18          Cross-device link
ENODEV       19          No such device
ENOTDIR      20          Not a directory
EISDIR       21          Is a directory
EINVAL       22          Invalid argument
ENFILE       23          File table overflow
EMFILE       24          Too many open files
ENOTTY       25          Not a teletype
ETXTBSY      26          Text file busy
EFBIG        27          File too large
ENOSPC       28          No space left on device
ESPIPE       29          Illegal seek
EROFS        30          Read-only file system
EMLINK       31          Too many links
EPIPE        32          Broken pipe
EDOM         33          Math argument
ERANGE       34          Result too large
EUCLEAN      35          File system needs cleaning
EDEADLK      36          Resource deadlock would occur
EDEADLOCK    37          Resource deadlock would occur
ENAMETOOLONG 38
ENOLCK       39
ENOSYS       40
ENOTEMPTY    41
EILSEQ       42
*/
//////////////////////////////////////////////////////////////////////////

const char* unixerrmsg(int code)
{
	switch(code)
	{
	case EPERM:      return "Not owner";
	case ENOENT:     return "No such file or directory";
	case ESRCH:      return "No such process";
	case EINTR:      return "Interrupted system call";
	case EIO:        return "I/O error";
	case ENXIO:      return "No such device or address";
	case E2BIG:      return "Argument list too long";
	case ENOEXEC:    return "Exec format error";
	case EBADF:      return "Invalid file descriptor";
	case ECHILD:     return "No spawned processes";
	case EAGAIN:     return "No more processes; not enough memory; maximum nesting level reached";
	case ENOMEM:     return "Not enough memory";
	case EACCES:     return "Access denied";
	case EFAULT:     return "Bad address";
#ifndef ENOTBLK
	case ENOTBLK:    return "Block device required";
#endif
	case EBUSY:      return "Mount device busy";
	case EEXIST:     return "File already exists";
	case EXDEV:      return "Cross-device link";
	case ENODEV:     return "No such device";
	case ENOTDIR:    return "Not a directory";
	case EISDIR:     return "Is a directory";
	case EINVAL:     return "Invalid argument";
	case ENFILE:     return "File table overflow";
	case EMFILE:     return "Too many open files";
	case ENOTTY:     return "Not a teletype";
#ifdef ETXTBSY
	case ETXTBSY:    return "Text file busy";
#endif
	case EFBIG:      return "File too large";
	case ENOSPC:     return "No space left on device";
	case ESPIPE:     return "Can not seek on this device";
	case EROFS:      return "Read-only file system";
	case EMLINK:     return "Too many links";
	case EPIPE:      return "Broken pipe";
	case EDOM:       return "Math argument";
	case ERANGE:     return "Result too large";
#ifdef EUCLEAN
	case EUCLEAN:    return "File system needs cleaning";
#endif
	case EDEADLK:    return "Resource deadlock would occur";
	default:         return "";
	}
}


NAMESPACE_END(basics)
