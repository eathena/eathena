// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_SOCKET_H_
#define _SOCKET_H_

#include "basetypes.h"
#include "baseinet.h"
#include "basesocket.h"
#include "basebuffer.h"
#include "basetime.h"

#include "malloc.h"
#include "timer.h"



extern time_t last_tick;

typedef int (*socket_function)(int);
typedef int (*console_function )(const char*);





/// abstract session data declaration.
/// user defined session data have to be derived from this class
class session_data
{
protected:
	session_data()				{}
public:
	virtual ~session_data()		{}
};

// socket data declaration
struct socket_data
{
	struct _flag{
		bool connected : 1;		// true when connected
		bool remove : 1;		// true when to be removed
		bool marked : 1;		// true when deleayed removal is initiated (optional)

		_flag() : connected(true),remove(false),marked(false)
		{}
	}flag;

	time_t rdata_tick;			// tick of last read, zero when timout restriction is disabled

	unsigned char *rdata;		// buffer
	size_t rdata_max;			// size of buffer
	size_t rdata_size;			// size of data
	size_t rdata_pos;			// iterator within data

	unsigned char *wdata;		// buffer
	size_t wdata_max;			// size of buffer
	size_t wdata_size;			// size of data

	basics::ipaddress client_ip;

	socket_function func_recv;
	socket_function func_send;
	socket_function func_parse;
	socket_function func_term;
	console_function func_console;

	session_data*	user_session;
};

// Data prototype declaration

extern struct socket_data *session[FD_SETSIZE];
extern size_t fd_max;


static inline bool session_isValid(int fd)
{
	return ( (fd>=0) && (fd<FD_SETSIZE) && (NULL!=session[fd]) );
}
static inline bool session_isActive(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.connected );
}	
	
static inline bool session_isRemoved(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.remove );
}
static inline bool session_isMarked(int fd)
{
	return ( session_isValid(fd) && session[fd]->flag.marked );
}
static inline bool session_Remove(int fd)
{	// force removal
	if( session_isValid(fd)	)
	{
		session[fd]->flag.connected = false;
		session[fd]->flag.marked	= false;
		session[fd]->flag.remove	= true;
	}
	return true;
}
static inline bool session_SetTermFunction(int fd, int (*term)(int))
{
	if( session_isValid(fd) )
		session[fd]->func_term = term;
	return true;
}

bool session_SetWaitClose(int fd, unsigned long timeoffset);
bool session_Delete(int fd);

bool session_checkbuffer(int fd, size_t sz);


// Function prototype declaration

int make_listen    (unsigned long ip, unsigned short port);
int make_connection(unsigned long ip, unsigned short port);

int realloc_fifo(int fd, size_t rfifo_size,size_t wfifo_size);

int WFIFOSET(int fd, size_t len);
int RFIFOSKIP(int fd, size_t len);

int do_sendrecv(int next);
void do_final_socket(void);

void socket_init(void);
void socket_final(void);

void flush_fifos();

int start_console(void);

void set_defaultparse(int (*defaultparse)(int));
void set_defaultconsoleparse(int (*defaultparse)(const char*));

bool detect_WAN(basics::ipaddress& wanip);
bool dropped_WAN(const basics::ipaddress& wanip, const ushort wanport);
bool initialize_WAN(basics::ipset& set, const ushort defaultport);














// Class for assigning/reading words from a buffer
class objW
{
	unsigned char *ip;
public:
	objW():ip(NULL)												{}
	objW(unsigned char* x):ip(x)								{}
	objW(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objW(char* x):ip((unsigned char*)x)							{}
	objW(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objW& init(unsigned char* x)			{ip=x;return *this;}
	objW& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objW& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objW& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned short() const
	{	return this->operator()();
	}
	unsigned short operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned short)(ip[0]))        )
					|( ((unsigned short)(ip[1])) << 0x08);
			
		}
		return 0;
	}

	objW& operator=(const objW& objw)
	{
		if(ip && objw.ip)
		{	
			memcpy(ip, objw.ip, 2);
		}
		return *this;
	}
	unsigned short operator=(unsigned short valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x00FF)          );
			ip[1] = (unsigned char)((valin & 0xFF00)  >> 0x08 );
		}
		return valin;
	}
};

// Class for assigning/reading dwords from a buffer
class objL
{
	unsigned char *ip;
public:
	objL():ip(NULL)												{}
	objL(unsigned char* x):ip(x)								{}
	objL(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objL(char* x):ip((unsigned char*)x)							{}
	objL(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objL& init(unsigned char* x)			{ip=x;return *this;}
	objL& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objL& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objL& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}


	operator unsigned long() const
	{	return this->operator()();
	}
	unsigned long operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[0]))        )
					|( ((unsigned long)(ip[1])) << 0x08)
					|( ((unsigned long)(ip[2])) << 0x10)
					|( ((unsigned long)(ip[3])) << 0x18);
			
		}
		return 0;
	}

	objL& operator=(const objL& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	unsigned long operator=(unsigned long valin)
	{	
		if(ip)
		{
			ip[0] = (unsigned char)((valin & 0x000000FF)          );
			ip[1] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[2] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[3] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

// Class for assigning/reading words from a buffer 
// without changing byte order
// necessary for IP numbers, which are always in network byte order
// it is implemented this way and I currently won't interfere with that
// even if it is quite questionable

// changes: 
// transfered all IP addresses in the programms to host byte order
// since IPs are transfered in network byte order
// we cannot use objL but its complementary here
class objLIP	
{
	unsigned char *ip;
public:
	objLIP():ip(NULL)											{}
	objLIP(unsigned char* x):ip(x)								{}
	objLIP(unsigned char* x,int pos):ip(x?x+pos:NULL)			{}
	objLIP(char* x):ip((unsigned char*)x)						{}
	objLIP(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objLIP& init(unsigned char* x)			{ip=x;return *this;}
	objLIP& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objLIP& init(char* x)					{ip=(unsigned char*)x;return *this;}
	objLIP& init(char* x,int pos)			{if(x) ip=(unsigned char*)(x+pos);return *this;}

	
	operator basics::ipaddress() const
	{
		return this->operator()();
	}
	basics::ipaddress operator()()	const
	{
		if(ip)
		{	
			return	 ( ((unsigned long)(ip[3]))        )
					|( ((unsigned long)(ip[2])) << 0x08)
					|( ((unsigned long)(ip[1])) << 0x10)
					|( ((unsigned long)(ip[0])) << 0x18);
		}
		return (uint32)INADDR_ANY;
	}
	objLIP& operator=(const objLIP& objl)
	{
		if(ip && objl.ip)
		{	
			memcpy(ip, objl.ip, 4);
		}
		return *this;
	}
	basics::ipaddress operator=(basics::ipaddress valin)
	{	
		if(ip)
		{
			ip[3] = (unsigned char)((valin & 0x000000FF)          );
			ip[2] = (unsigned char)((valin & 0x0000FF00)  >> 0x08 );
			ip[1] = (unsigned char)((valin & 0x00FF0000)  >> 0x10 );
			ip[0] = (unsigned char)((valin & 0xFF000000)  >> 0x18 );
		}
		return valin;
	}
};

class objB
{
	unsigned char *ip;
public:
	objB():ip(NULL)												{}
	objB(unsigned char* x):ip(x)								{}
	objB(unsigned char* x,int pos):ip(x?x+pos:NULL)				{}
	objB(char* x):ip((unsigned char*)x)							{}
	objB(char* x,int pos):ip((unsigned char*)(x?x+pos:NULL))	{}

	objB& init(unsigned char* x)			{ip=x;return *this;}
	objB& init(unsigned char* x,int pos)	{if(x) ip=x+pos;return *this;}
	objB& init(char* x)						{ip=(unsigned char*)x;return *this;}
	objB& init(char* x,int pos)				{if(x) ip=(unsigned char*)(x+pos);return *this;}

	operator unsigned char() const
	{	return this->operator()();
	}
	unsigned char operator()()	const
	{
		if(ip)
		{	
			return ip[0];
		}
		return 0;
	}
	objB& operator=(const objB& objb)
	{
		if(ip && objb.ip)
		{	
			ip[0] = objb.ip[0];
		}
		return *this;
	}
	unsigned char operator=(unsigned char valin)
	{	
		if(ip)
		{
			ip[0] = valin;
		}
		return valin;
	}
};



// define declaration

#define RFIFOP(fd,pos) ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL))
#define RFIFOB(fd,pos) ((objB( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) )))
#define RFIFOW(fd,pos) ((objW( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) )))
#define RFIFOL(fd,pos) ((objL( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) )))
#define RFIFOLIP(fd,pos) ((objLIP( ((session[fd]&&session[fd]->rdata)?(session[fd]->rdata+session[fd]->rdata_pos+(pos)):(NULL)) )))
#define RFIFOREST(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_size-session[fd]->rdata_pos):0))
#define RFIFOSPACE(fd) ((int)((session[fd]&&session[fd]->rdata)?(session[fd]->rdata_max-session[fd]->rdata_size):0))
#define RBUFP(p,pos) ((((unsigned char*)(p))+(pos)))
#define RBUFB(p,pos) ((*((unsigned char*)RBUFP((p),(pos)))))
#define RBUFW(p,pos) ((objW((p),(pos))))
#define RBUFL(p,pos) ((objL((p),(pos))))
#define RBUFLIP(p,pos) ((objLIP((p),(pos))))

#define WFIFOSPACE(fd) (((session[fd]&&session[fd]->wdata)?(session[fd]->wdata_max-session[fd]->wdata_size):0))

#define WFIFOP(fd,pos) (((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)))
#define WFIFOB(fd,pos) ((objB( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) )))
#define WFIFOW(fd,pos) ((objW( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) )))
#define WFIFOL(fd,pos) ((objL( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) )))
#define WFIFOLIP(fd,pos) ((objLIP( ((session[fd]&&session[fd]->wdata)?(session[fd]->wdata+session[fd]->wdata_size+(pos)):(NULL)) )))
#define WBUFP(p,pos) (((unsigned char*)(p))+(pos))
#define WBUFB(p,pos) (*((unsigned char*)WBUFP((p),(pos))))
#define WBUFW(p,pos) ((objW((p),(pos))))
#define WBUFL(p,pos) ((objL((p),(pos))))
#define WBUFLIP(p,pos) ((objLIP((p),(pos))))

// explicitly using doubled parenthesis here because of an ugly VC7 bug
// which permits the usage of inheritance specification in assignment following this macro













#endif	// _SOCKET_H_
