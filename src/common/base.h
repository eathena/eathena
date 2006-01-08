#ifndef	_BASE_H_
#define _BASE_H_

//////////////////////////////////////////////////////////////////////////
// temporarily include all necessary base files here here
//////////////////////////////////////////////////////////////////////////
#include "basetypes.h"
#include "basememory.h"
#include "basecharset.h"
#include "basestring.h"
#include "basearray.h"
#include "basesync.h"
#include "basetime.h"
#include "basefile.h"
#include "basepool.h"

#include "baseministring.h"




//////////////////////////////////////////////////////////////////////////
// leftovers from base files currently not included in this package
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// #include "baseinet.h"
//////////////////////////////////////////////////////////////////////////


//////////////////////////////
// socketlen type definition
//////////////////////////////
#ifndef __socklen_t_defined
#define __socklen_t_defined	// define socklen_t if not already defined

#ifdef WIN32
  typedef int socklen_t;	// type in an int on windows
#elif CYGWIN				// stupid cygwin is not standard conform
  #ifdef socklen_t			// it has a define for this instead a typedef as it should have
  #undef socklen_t
  #endif
  typedef int socklen_t;	// define it pr
#else// normal unix with undefined socklen_t
  typedef unsigned int socklen_t;
#endif
  
#endif//__socklen_t_defined


//////////////////////////////
#ifdef WIN32
//////////////////////////////

// defines

#define RETCODE	"\r\n"
#define RET RETCODE

// functions
extern inline int read(SOCKET fd, char*buf, int sz)		
{
	return recv(fd,buf,sz,0); 
}
extern inline int write(SOCKET fd, char*buf, int sz)	
{
	return send(fd,buf,sz,0); 
}

//////////////////////////////
#else/////////////////////////
//////////////////////////////

// defines
#define RETCODE "\n"
#define RET RETCODE

// typedefs
typedef int		SOCKET;

// functions
static inline int closesocket(SOCKET fd)		
{
	return close(fd); 
}
static inline int ioctlsocket(SOCKET fd, long cmd, unsigned long *arg)		
{
	return ioctl(fd,cmd,arg); 
}

//////////////////////////////
#endif////////////////////////
//////////////////////////////








#endif//_BASE_H_

