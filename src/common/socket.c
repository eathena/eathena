// original : core.c 2003/02/26 18:03:12 Rev 1.7

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <io.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef SIOCGIFCONF
#include <sys/sockio.h> // SIOCGIFCONF on Solaris, maybe others? [Shinomori]
#endif

#endif

#include <fcntl.h>
#include <string.h>

#include "socket.h"
#include "mmo.h"	// [Valaris] thanks to fov
#include "timer.h"
#include "malloc.h"
#include "showmsg.h"

fd_set readfds;
#ifdef TURBO
fd_set writefds;
#endif
int fd_max;
time_t tick_;
time_t stall_time_ = 60;
int ip_rules = 1;

#if defined(CYGWIN) || defined(_WIN32)
	#include "dll.h"
	#define UPNP

	Addon *upnp;
	int (*firewall_addport)(char *desc, int port);
	int	(*upnp_addport)(char *desc, unsigned char *ip, int port);
	extern char *argp;

	int release_mappings = 1;
	int close_ports = 1;
#endif

// values derived from freya
// a player that send more than 2k is probably a hacker without be parsed
// biggest known packet: S 0153 <len>.w <emblem data>.?B -> 24x24 256 color .bmp (0153 + len.w + 1618/1654/1756 bytes)
size_t rfifo_size = (16*1024);
size_t wfifo_size = (16*1024);

#ifndef TCP_FRAME_LEN
#define TCP_FRAME_LEN 1053
#endif

#define CONVIP(ip) ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,ip>>24

struct socket_data *session[FD_SETSIZE];

static int null_parse(int fd);
static int (*default_func_parse)(int) = null_parse;

static int null_console_parse(char *buf);
static int (*default_console_parse)(char*) = null_console_parse;
static int connect_check(unsigned int ip);

/*======================================
 *	CORE : Set function
 *--------------------------------------
 */
void set_defaultparse(int (*defaultparse)(int))
{
	default_func_parse = defaultparse;
}

void set_nonblocking(int fd, int yes) {
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);
}

static void setsocketopts(int fd)
{
	int yes = 1; // reuse fix

	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof yes);
#ifdef SO_REUSEPORT
	setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,(char *)&yes,sizeof yes);
#endif
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&yes,sizeof yes);

	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &wfifo_size , sizeof(rfifo_size ));
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &rfifo_size , sizeof(rfifo_size ));
}

/*======================================
 *	CORE : Socket Sub Function
 *--------------------------------------
 */

static int recv_to_fifo(int fd)
{
	int len;

	if( (fd<0) || (fd>=FD_SETSIZE) || (NULL==session[fd]) )
		return -1;

	//ShowMessage("recv_to_fifo : %d %d\n",fd,session[fd]->eof);
	if(session[fd]->eof)
		return -1;


#ifdef _WIN32
	len=recv(fd,session[fd]->rdata+session[fd]->rdata_size, RFIFOSPACE(fd), 0);
#else
	len=read(fd,session[fd]->rdata+session[fd]->rdata_size, RFIFOSPACE(fd));
#endif

//	ShowInfo (":::RECEIVE:::\n");
//	dump(session[fd]->rdata, len); ShowMessage ("\n");

	//{ int i; ShowMessage("recv %d : ",fd); for(i=0;i<len;i++){ ShowMessage("%02x ",RFIFOB(fd,session[fd]->rdata_size+i)); } ShowMessage("\n");}
	if(len>0){
		session[fd]->rdata_size+=len;
		session[fd]->rdata_tick = tick_;
	} else if(len<=0){
		// value of connection is not necessary the same
//		ShowMessage("set eof : connection #%d\n", fd);
		session[fd]->eof=1;
	}
	return 0;
}

static int send_from_fifo(int fd)
{
	int len;
	if( !session_isValid(fd) )
		return -1;

	// clear write buffer if not connected
	if( session[fd]->eof )
	{
		session[fd]->wdata_size = 0;
#ifdef TURBO
		FD_CLR(fd, &writefds);
#endif
		return -1;
	}
	
	//ShowMessage("send_from_fifo : %d\n",fd);
	if (session[fd]->wdata_size == 0)
	{
#ifdef TURBO
		FD_CLR(fd, &writefds);
#endif
		return 0;
	}


#ifdef _WIN32
	len=send(fd, session[fd]->wdata,session[fd]->wdata_size, 0);
#else
	len=write(fd,session[fd]->wdata,session[fd]->wdata_size);
#endif

//	ShowInfo (":::SEND:::\n");
//	dump(session[fd]->wdata, len); ShowMessage ("\n");

	//{ int i; ShowMessage("send %d : ",fd);  for(i=0;i<len;i++){ ShowMessage("%02x ",session[fd]->wdata[i]); } ShowMessage("\n");}
	if(len>0){
		if(len<session[fd]->wdata_size){
			memmove(session[fd]->wdata,session[fd]->wdata+len,session[fd]->wdata_size-len);
			session[fd]->wdata_size-=len;
		} else {
			session[fd]->wdata_size=0;
#ifdef TURBO
			FD_CLR(fd, &writefds);
#endif
		}
	} else if (errno != EAGAIN) {
//		ShowMessage("set eof :%d\n",fd);
		session[fd]->eof=1;
	}
	return 0;
}

void flush_fifos()
{
	int i;
	for(i=0;i<fd_max;i++)
		if(session[i] != NULL &&
		   session[i]->func_send == send_from_fifo)
			send_from_fifo(i);
}

static int null_parse(int fd)
{
	ShowMessage("null_parse : %d\n",fd);
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

/*======================================
 *	CORE : Socket Function
 *--------------------------------------
 */

static int connect_client(int listen_fd)
{
	int fd;
	struct sockaddr_in client_address;
	int len;
#ifndef _WIN32
	int result;
#endif

	//ShowMessage("connect_client : %d\n",listen_fd);

	len=sizeof(client_address);

	fd = accept(listen_fd,(struct sockaddr*)&client_address,(socklen_t*)&len);
	if(fd_max<=fd) fd_max=fd+1;

	setsocketopts(fd);

	if(fd==-1) {
		perror("accept");
		return -1;
	} else if (ip_rules && !connect_check(*(unsigned int*)(&client_address.sin_addr))) {
		close(fd);
		return -1;
	} else
		FD_SET(fd,&readfds);

#ifdef _WIN32
	{
		unsigned long val = 1;
		ioctlsocket(fd, FIONBIO, &val);
	}
#else
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	CREATE(session[fd], struct socket_data, 1);
	CREATE_A(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata   = rfifo_size;
	session[fd]->max_wdata   = wfifo_size;
	session[fd]->func_recv   = recv_to_fifo;
	session[fd]->func_send   = send_from_fifo;
	session[fd]->func_parse  = default_func_parse;
	session[fd]->client_addr = client_address;
	session[fd]->rdata_tick = tick_;

  //ShowMessage("new_session : %d %d\n",fd,session[fd]->eof);
	return fd;
}

int make_listen_port(int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(fd_max<=fd) fd_max=fd+1;

#ifdef _WIN32
        {
	  	unsigned long val = 1;
  		ioctlsocket(fd, FIONBIO, &val);
        }
#else
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl( INADDR_ANY );
	server_address.sin_port        = htons((unsigned short)port);

#ifdef UPNP
	if (upnp && upnp->state) {
		int localaddr = ntohl(addr_[0]);
		unsigned char *natip = (unsigned char *)&localaddr;
		char buf[16];
		sprintf(buf, "%d.%d.%d.%d", natip[0], natip[1], natip[2], natip[3]);
		//ShowMessage("natip=%d.%d.%d.%d\n", natip[0], natip[1], natip[2], natip[3]);
		if (firewall_addport(argp, port))
			ShowInfo ("Firewall port %d successfully opened\n", port);
		if (natip[0] == 192 && natip[1] == 168) {
			if (upnp_addport(argp, natip, port))
				ShowInfo ("UPnP mappings successfull\n");
		}
	}
#endif

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if( result == -1 ) {
		perror("bind");
		exit(1);
	}
	result = listen( fd, 5 );
	if( result == -1 ) { /* error */
		perror("listen");
		exit(1);
	}

	FD_SET(fd, &readfds );

	CREATE(session[fd], struct socket_data, 1);

	memset(session[fd],0,sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;

	return fd;
}

int make_listen_bind(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(fd_max<=fd) fd_max=fd+1;

#ifdef _WIN32
        {
	  	unsigned long val = 1;
  		ioctlsocket(fd, FIONBIO, &val);
        }
#else
	result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port        = htons((unsigned short)port);

#ifdef UPNP
	if (upnp && upnp->state) {
		int localaddr = ntohl(addr_[0]);
		unsigned char *natip = (unsigned char *)&localaddr;
		char buf[16];
		sprintf(buf, "%d.%d.%d.%d", natip[0], natip[1], natip[2], natip[3]);
		//ShowMessage("natip=%d.%d.%d.%d\n", natip[0], natip[1], natip[2], natip[3]);
		if (firewall_addport(argp, port))
			ShowInfo ("Firewall port %d successfully opened\n", port);
		if (natip[0] == 192 && natip[1] == 168) {
			if (upnp_addport(argp, natip, port))
				ShowInfo ("UPnP mappings successfull\n");
		}
	}
#endif

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if( result == -1 ) {
		perror("bind");
		exit(1);
	}
	result = listen( fd, 5 );
	if( result == -1 ) { /* error */
		perror("listen");
		exit(1);
	}

	FD_SET(fd, &readfds );

	CREATE(session[fd], struct socket_data, 1);

	memset(session[fd],0,sizeof(*session[fd]));
	session[fd]->func_recv = connect_client;

	ShowStatus("Open listen port on %d.%d.%d.%d:%i\n",
		(ip)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);

	return fd;
}

// Console Reciever [Wizputer]
int console_recieve(int i) {
	int n;
	char *buf;

	CREATE_A(buf, char, 64);
	memset(buf,0,sizeof(64));

	n = read(0, buf , 64);
	if ( n < 0 )
		ShowError("Console input read error\n");
	else
		session[0]->func_console(buf);

	aFree(buf);
	return 0;
}

void set_defaultconsoleparse(int (*defaultparse)(char*))
{
	default_console_parse = defaultparse;
}

static int null_console_parse(char *buf)
{
	ShowMessage("null_console_parse : %s\n",buf);
	return 0;
}

// Console Input [Wizputer]
int start_console(void) {
	FD_SET(0,&readfds);

	if (!session[0]) {	// dummy socket already uses fd 0
		CREATE(session[0], struct socket_data, 1);
	}
	memset(session[0],0,sizeof(*session[0]));

	session[0]->func_recv = console_recieve;
	session[0]->func_console = default_console_parse;

	return 0;
}

int make_connection(long ip,int port)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if (fd_max <= fd)
		fd_max = fd + 1;

	setsocketopts(fd);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons((unsigned short)port);

#ifdef _WIN32
        {
            unsigned long val = 1;
            ioctlsocket(fd, FIONBIO, &val);
        }
#else
        result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	ShowStatus("Connecting to %d.%d.%d.%d:%i\n",
		(ip)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);

	result = connect(fd, (struct sockaddr *)(&server_address), sizeof(struct sockaddr_in));

	// cool, what do you do if connect fails (result<0)?
	/*if (result == -1) {
		//perror("connect");
		return result;
	}*/

	FD_SET(fd,&readfds);

	CREATE(session[fd], struct socket_data, 1);
	CREATE_A(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = recv_to_fifo;
	session[fd]->func_send  = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	session[fd]->rdata_tick = tick_;

	return fd;
}

int delete_session(int fd)
{
	if (fd <= 0 || fd >= FD_SETSIZE)
		return -1;
	FD_CLR(fd, &readfds);
#ifdef TURBO
	FD_CLR(fd, &writefds);
#endif
	if (session[fd]){
		if (session[fd]->rdata)
			aFree(session[fd]->rdata);
		if (session[fd]->wdata)
			aFree(session[fd]->wdata);
		if (session[fd]->session_data)
			aFree(session[fd]->session_data);
		aFree(session[fd]);
		session[fd] = NULL;
	}
	//ShowMessage("delete_session:%d\n",fd);
	return 0;
}

int realloc_fifo(int fd,int rfifo_size,int wfifo_size)
{
	if( !session_isValid(fd) )
		return 0;

	if( session[fd]->max_rdata != rfifo_size && session[fd]->rdata_size < rfifo_size){
		RECREATE(session[fd]->rdata, unsigned char, rfifo_size);
		session[fd]->max_rdata  = rfifo_size;
	}
	
	if( session[fd]->max_wdata != wfifo_size && session[fd]->wdata_size < wfifo_size){
		RECREATE(session[fd]->wdata, unsigned char, wfifo_size);
		session[fd]->max_wdata  = wfifo_size;
	}
	return 0;
}

int realloc_writefifo(int fd, size_t addition)
{
	size_t newsize;

	if( !session_isValid(fd) ) // might not happen
		return 0;

	if( session[fd]->wdata_size + addition  > session[fd]->max_wdata )
	{	// grow rule; grow in multiples of wfifo_size
		newsize = wfifo_size;
		while( session[fd]->wdata_size + addition > newsize ) newsize += newsize;
	}
	else if( session[fd]->max_wdata>wfifo_size && (session[fd]->wdata_size+addition)*4 < session[fd]->max_wdata )
	{	// shrink rule, shrink by 2 when ony a quater of the fifo is used, don't shrink below 4*addition
		newsize = session[fd]->max_wdata/2;
	}
	else // no change
		return 0;

	RECREATE(session[fd]->wdata, unsigned char, newsize);
	session[fd]->max_wdata  = newsize;

	return 0;
}

int WFIFOSET(int fd,int len)
{
	size_t newreserve;
	struct socket_data *s = session[fd];

	if( !session_isValid(fd) || s->wdata == NULL )
		return 0;

	// we have written len bytes to the buffer already before calling WFIFOSET
	s->wdata_size += len;

	if(s->wdata_size > s->max_wdata)
	{	// actually there was a buffer overflow already
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		ShowMessage("socket: Buffer Overflow. Connection %d (%d.%d.%d.%d) has written %d bytes (%d allowed)\n", fd,
			sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], s->wdata_size + len, s->max_wdata);
		// no other chance, make a better fifo model
		exit(1);
	}

#ifdef TURBO
	FD_SET(fd,&writefds);
#endif


	// always keep a wfifo_size reserve in the buffer
	newreserve = s->wdata_size + wfifo_size;

	if (s->wdata_size > (TCP_FRAME_LEN))
		send_from_fifo(fd);

	// realloc after sending
	// readfifo does not need to be realloced at all
	realloc_writefifo(fd, newreserve);

	return 0;
}

int do_sendrecv(int next)
{
	fd_set rfd,wfd;
	struct timeval timeout;
	int ret,i;
#ifdef TURBO
	int j;
#endif

	tick_ = time(0);

	memcpy(&rfd, &readfds, sizeof(rfd));
#ifdef TURBO
	memcpy(&wfd, &writefds, sizeof(wfd));
#else
	FD_ZERO(&wfd);

	for (i = 0; i < fd_max; i++){
		if(!session[i] && FD_ISSET(i, &readfds)){
			ShowMessage("force clr fds %d\n", i);
			FD_CLR(i, &readfds);
			continue;
		}
		if(!session[i])
			continue;
		if(session[i]->wdata_size)
			FD_SET(i, &wfd);
	}
#endif

	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;
	ret = select(fd_max, &rfd, &wfd, NULL, &timeout);

#ifndef TURBO
	if (ret <= 0)
		return 0;
	for (i = 0; i < fd_max; i++){
#else

#ifndef __FDS_BITS
	#define __FDS_BITS(set) ((set)->fds_bits)
#endif

	for (i = 0; i < fd_max && ret > 0; i++){
		if ((i & (NFDBITS - 1)) == 0) {
			int off = i / NFDBITS;
			if ((__FDS_BITS(&wfd)[off] == 0) && (__FDS_BITS(&rfd)[off] == 0))
				i += NFDBITS;
		}
		for (j = 0; (j < NFDBITS) && (ret > 0); j++, i++) {
#endif

#if defined(TURBO) && defined(DEBUG)
			if(!session[i]) {
				if (FD_ISSET(i, &readfds))
					printf("FD_ISSET(i, &readfds) returned true with no session set\n");
				if (FD_ISSET(i, &writefds))
					printf("FD_ISSET(i, &writefds) returned true with no session set\n");
				continue;
			} else {
				if ((i != 0) && FD_ISSET(i, &readfds) == 0)
					printf("FD_ISSET(%d, &readfds) returned false with session set\n", i);
				if (session[i]->wdata_size == 0) {
					if (FD_ISSET(i, &writefds))
						printf("FD_ISSET(i, &writefds) returned true with session set and no data\n");
				} else {
					if (FD_ISSET(i, &writefds) == 0)
						printf("FD_ISSET(i, &writefds) returned false with session set and data\n");
				}
			}
#else
		if(!session[i]) 
			continue;
#endif

		if (FD_ISSET(i, &wfd)) {
			//ShowMessage("write:%d\n",i);
			if(session[i]->func_send)
				session[i]->func_send(i);
#ifdef TURBO
			if ((session[i]->rdata_tick != 0) && ((tick_ - session[i]->rdata_tick) > stall_time_)) {
				session[i]->eof = 1;
				if(session[i]->func_parse)
					session[i]->func_parse(i);
				continue;
			}
			ret--;
#endif
		}

		if(FD_ISSET(i,&rfd)){
			//ShowMessage("read:%d\n",i);
			if(session[i]->func_recv)
				session[i]->func_recv(i);
#ifdef TURBO
			if(session[i]->func_parse)
				session[i]->func_parse(i);
			ret--;
#endif
		}

#ifdef TURBO
		} // for (j = 0; 
#endif
	} // for (i = 0
	return 0;
}

#ifndef TURBO
int do_parsepacket(void)
{
	int i;
	for(i = 0; i < fd_max; i++){
		if(!session[i])
			continue;
		if ((session[i]->rdata_tick != 0) && DIFF_TICK(tick_, session[i]->rdata_tick) > stall_time_) {
			ShowInfo ("Session #%d timed out\n", i);
			session[i]->eof = 1;
		}
		if(session[i]->rdata_size == 0 && session[i]->eof == 0)
			continue;
		if(session[i]->func_parse){
			session[i]->func_parse(i);
			if(!session[i])
				continue;
		}
		RFIFOFLUSH(i);
	}
	return 0;
}
#endif

/* DDoS 攻撃対策 */

enum {
	ACO_DENY_ALLOW=0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
};

struct _access_control {
	unsigned int ip;
	unsigned int mask;
};

static struct _access_control *access_allow;
static struct _access_control *access_deny;
static int access_order=ACO_DENY_ALLOW;
static int access_allownum=0;
static int access_denynum=0;
static int access_debug=0;
static int ddos_count     = 10;
static int ddos_interval  = 3000;
static int ddos_autoreset = 600*1000;

struct _connect_history {
	struct _connect_history *next;
	struct _connect_history *prev;
	int    status;
	int    count;
	unsigned int ip;
	unsigned int tick;
};
static struct _connect_history *connect_history[0x10000];
static int connect_check_(unsigned int ip);

// 接続できるかどうかの確認
//   false : 接続OK
//   true  : 接続NG
static int connect_check(unsigned int ip) {
	int result = connect_check_(ip);
	if(access_debug) {
		ShowMessage("connect_check: Connection from %d.%d.%d.%d %s\n",
			CONVIP(ip),result ? "allowed." : "denied!");
	}
	return result;
}
	
static int connect_check_(unsigned int ip) {
	struct _connect_history *hist     = connect_history[ip & 0xFFFF];
	struct _connect_history *hist_new;
	int    i,is_allowip = 0,is_denyip = 0,connect_ok = 0;

	// allow , deny リストに入っているか確認
	for(i = 0;i < access_allownum; i++) {
		if((ip & access_allow[i].mask) == (access_allow[i].ip & access_allow[i].mask)) {
			if(access_debug) {
				ShowMessage("connect_check: Found match from allow list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_allow[i].ip),
					CONVIP(access_allow[i].mask));
			}
			is_allowip = 1;
			break;
		}
	}
	for(i = 0;i < access_denynum; i++) {
		if((ip & access_deny[i].mask) == (access_deny[i].ip & access_deny[i].mask)) {
			if(access_debug) {
				ShowMessage("connect_check: Found match from deny list:%d.%d.%d.%d IP:%d.%d.%d.%d Mask:%d.%d.%d.%d\n",
					CONVIP(ip),
					CONVIP(access_deny[i].ip),
					CONVIP(access_deny[i].mask));
			}
			is_denyip = 1;
			break;
		}
	}
	// コネクト出来るかどうか確認
	// connect_ok
	//   0 : 無条件に拒否
	//   1 : 田代砲チェックの結果次第
	//   2 : 無条件に許可
	switch(access_order) {
	case ACO_DENY_ALLOW:
	default:
		if(is_allowip) {
			connect_ok = 2;
		} else if(is_denyip) {
			connect_ok = 0;
		} else {
			connect_ok = 1;
		}
		break;
	case ACO_ALLOW_DENY:
		if(is_denyip) {
			connect_ok = 0;
		} else if(is_allowip) {
			connect_ok = 2;
		} else {
			connect_ok = 1;
		}
		break;
	case ACO_MUTUAL_FAILTURE:
		if(is_allowip) {
			connect_ok = 2;
		} else {
			connect_ok = 0;
		}
		break;
	}

	// 接続履歴を調べる
	while(hist) {
		if(ip == hist->ip) {
			// 同じIP発見
			if(hist->status) {
				// ban フラグが立ってる
				return (connect_ok == 2 ? 1 : 0);
			} else if(DIFF_TICK(gettick(),hist->tick) < ddos_interval) {
				// ddos_interval秒以内にリクエスト有り
				hist->tick = gettick();
				if(hist->count++ >= ddos_count) {
					// ddos 攻撃を検出
					hist->status = 1;
					ShowWarning("connect_check: DDOS Attack detected from %d.%d.%d.%d!\n",
						CONVIP(ip));
					return (connect_ok == 2 ? 1 : 0);
				} else {
					return connect_ok;
				}
			} else {
				// ddos_interval秒以内にリクエスト無いのでタイマークリア
				hist->tick  = gettick();
				hist->count = 0;
				return connect_ok;
			}
		}
		hist = hist->next;
	}
	// IPリストに無いので新規作成
	hist_new = (struct _connect_history *) aCalloc(1,sizeof(struct _connect_history));
	hist_new->ip   = ip;
	hist_new->tick = gettick();
	if(connect_history[ip & 0xFFFF] != NULL) {
		hist = connect_history[ip & 0xFFFF];
		hist->prev = hist_new;
		hist_new->next = hist;
	}
	connect_history[ip & 0xFFFF] = hist_new;
	return connect_ok;
}

static int connect_check_clear(int tid,unsigned int tick,int id,int data) {
	int i;
	int clear = 0;
	int list  = 0;
	struct _connect_history *hist , *hist2;
	for(i = 0;i < 0x10000 ; i++) {
		hist = connect_history[i];
		while(hist) {
			if ((DIFF_TICK(tick,hist->tick) > ddos_interval * 3 && !hist->status) ||
				(DIFF_TICK(tick,hist->tick) > ddos_autoreset && hist->status)) {
				// clear data
				hist2 = hist->next;
				if(hist->prev) {
					hist->prev->next = hist->next;
				} else {
					connect_history[i] = hist->next;
				}
				if(hist->next) {
					hist->next->prev = hist->prev;
				}
				aFree(hist);
				hist = hist2;
				clear++;
			} else {
				hist = hist->next;
			}
			list++;
		}
	}
	if(access_debug) {
		ShowMessage("connect_check_clear: Cleared %d of %d from IP list.\n", clear, list);
	}
	return list;
}

// IPマスクチェック
int access_ipmask(const char *str,struct _access_control* acc)
{
	unsigned int mask=0,i=0,m,ip, a0,a1,a2,a3;
	if( !strcmp(str,"all") ) {
		ip   = 0;
		mask = 0;
	} else {
		if( sscanf(str,"%d.%d.%d.%d%n",&a0,&a1,&a2,&a3,&i)!=4 || i==0) {
			ShowError("access_ipmask: Unknown format %s!\n",str);
			return 0;
		}
		ip = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;

		if(sscanf(str+i,"/%d.%d.%d.%d",&a0,&a1,&a2,&a3)==4 ){
			mask = (a3 << 24) | (a2 << 16) | (a1 << 8) | a0;
		} else if(sscanf(str+i,"/%d",&m) == 1) {
			for(i=0;i<m;i++) {
				mask = (mask >> 1) | 0x80000000;
			}
			mask = ntohl(mask);
		} else {
			mask = 0xFFFFFFFF;
		}
	}
	if(access_debug) {
		ShowMessage("access_ipmask: Loaded IP:%d.%d.%d.%d mask:%d.%d.%d.%d\n",
			CONVIP(ip), CONVIP(mask));
	}
	acc->ip   = ip;
	acc->mask = mask;
	return 1;
}

int socket_config_read(const char *cfgName) {
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName, "r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"stall_time")==0){
			stall_time_ = atoi(w2);
		} else if(strcmpi(w1,"enable_ip_rules")==0){
			if(strcmpi(w2,"yes")==0)
				ip_rules = 1;
			else if(strcmpi(w2,"no")==0)
				ip_rules = 0;
			else ip_rules = atoi(w2);
		} else if(strcmpi(w1,"order")==0){
			access_order=atoi(w2);
			if(strcmpi(w2,"deny,allow")==0) access_order=ACO_DENY_ALLOW;
			if(strcmpi(w2,"allow,deny")==0) access_order=ACO_ALLOW_DENY;
			if(strcmpi(w2,"mutual-failure")==0) access_order=ACO_MUTUAL_FAILTURE;
		} else if(strcmpi(w1,"allow")==0){
			access_allow = (struct _access_control *) aRealloc(access_allow,(access_allownum+1)*sizeof(struct _access_control));
			if(access_ipmask(w2,&access_allow[access_allownum])) {
				access_allownum++;
			}
		} else if(strcmpi(w1,"deny")==0){
			access_deny = (struct _access_control *) aRealloc(access_deny,(access_denynum+1)*sizeof(struct _access_control));
			if(access_ipmask(w2,&access_deny[access_denynum])) {
				access_denynum++;
			}
		} else if(!strcmpi(w1,"ddos_interval")){
			ddos_interval = atoi(w2);
		} else if(!strcmpi(w1,"ddos_count")){
			ddos_count = atoi(w2);
		} else if(!strcmpi(w1,"ddos_autoreset")){
			ddos_autoreset = atoi(w2);
		} else if(!strcmpi(w1,"debug")){
			if(strcmpi(w2,"yes")==0)
				access_debug = 1;
			else if(strcmpi(w2,"no")==0)
				access_debug = 0;
			else access_debug = atoi(w2);
	#ifdef UPNP
		} else if(!strcmpi(w1,"release_mappings")){
			if(strcmpi(w2,"yes")==0)
				release_mappings = 1;
			else if(strcmpi(w2,"no")==0)
				release_mappings = 0;
			else release_mappings = atoi(w2);
		} else if(!strcmpi(w1,"close_ports")){
			if(strcmpi(w2,"yes")==0)
				close_ports = 1;
			else if(strcmpi(w2,"no")==0)
				close_ports = 0;
			else close_ports = atoi(w2);
	#endif
		} else if (strcmpi(w1, "import") == 0)
			socket_config_read(w2);
	}
	fclose(fp);
	return 0;
}

int RFIFOSKIP(int fd,int len)
{
	struct socket_data *s;
	
	if ( !session_isActive(fd) ) //Nullpo error here[Kevin]
		return 0;

	s = session[fd];

	if (s->rdata_size-s->rdata_pos-len<0) {
		fprintf(stderr,"too many skip\n");
		exit(1);
	}

	s->rdata_pos = s->rdata_pos+len;

	return 0;
}


unsigned int addr_[16];   // ip addresses of local host (host byte order)
unsigned int naddr_ = 0;   // # of ip addresses

int  Net_Init(void)
{
#ifdef _WIN32
	char** a;
	unsigned int i;
	char fullhost[255];
	struct hostent* hent;

		/* Start up the windows networking */
	WSADATA wsaData;

	if ( WSAStartup(WINSOCK_VERSION, &wsaData) != 0 ) {
		ShowFatalError("SYSERR: WinSock not available!\n");
		exit(1);
	}

	if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR) {
		ShowError("Ugg.. no hostname defined!\n");
		return 0;
	}

	// XXX This should look up the local IP addresses in the registry
	// instead of calling gethostbyname. However, the way IP addresses
	// are stored in the registry is annoyingly complex, so I'll leave
	// this as T.B.D.
	hent = gethostbyname(fullhost);
	if (hent == NULL) {
		ShowError("Cannot resolve our own hostname to a IP address");
		return 0;
	}

	a = hent->h_addr_list;
	for(i = 0; a[i] != 0 && i < 16; ++i) {
		unsigned long addr1 = ntohl(*(unsigned long*) a[i]);
		addr_[i] = addr1;
	}
	naddr_ = i;
#else
	int pos;
	int fdes = socket(AF_INET, SOCK_STREAM, 0);
	char buf[16 * sizeof(struct ifreq)];
	struct ifconf ic;

	// The ioctl call will fail with Invalid Argument if there are more
	// interfaces than will fit in the buffer
	ic.ifc_len = sizeof(buf);
	ic.ifc_buf = buf;
	if(ioctl(fdes, SIOCGIFCONF, &ic) == -1) {
		ShowError("SIOCGIFCONF failed!\n");
		return 0;
	}

	for(pos = 0; pos < ic.ifc_len;)
	{
		struct ifreq * ir = (struct ifreq *) (ic.ifc_buf + pos);

		struct sockaddr_in * a = (struct sockaddr_in *) &(ir->ifr_addr);

		if(a->sin_family == AF_INET) {
			u_long ad = ntohl(a->sin_addr.s_addr);
			if(ad != INADDR_LOOPBACK) {
				addr_[naddr_ ++] = ad;
				if(naddr_ == 16)
					break;
			}
		}

#if defined(_AIX) || defined(__APPLE__)
		pos += ir->ifr_addr.sa_len;  // For when we port athena to run on Mac's :)
		pos += sizeof(ir->ifr_name);
#else
		pos += sizeof(struct ifreq);
#endif
	}

#endif

	return(0);
}

#ifdef UPNP
void do_init_upnp(void)
{
	int (*upnp_init)();
	int *_release_mappings;
	int *_close_ports;

	upnp = dll_open ("addons/upnp.dll");
	if (!upnp)
		return;
	DLL_SYM (upnp_init, upnp->dll, "do_init");
	DLL_SYM (firewall_addport, upnp->dll, "Firewall_AddPort");
	DLL_SYM (upnp_addport, upnp->dll, "UPNP_AddPort");
	if (!upnp_init || !firewall_addport || !upnp_addport) {
		ShowInfo ("Unable to load UPnP: %s\n", DLL_ERROR());
		upnp->state = 0;
		return;
	}

	DLL_SYM (_release_mappings, upnp->dll, "release_mappings");
	DLL_SYM (_close_ports, upnp->dll, "close_ports");
	if (_release_mappings)
		*_release_mappings = release_mappings;
	if (_close_ports)
		*_close_ports = close_ports;
	
	if (upnp_init() == 0)
		upnp->state = 0;

	return;
}
#endif

void do_final_socket(void)
{
	int i;
	struct _connect_history *hist , *hist2;
	for (i = 1; i < fd_max; i++) {
		if(session[i])
			delete_session(i);
	}
	for(i = 0; i < 0x10000; i++) {
		hist = connect_history[i];
		while(hist) {
			hist2 = hist->next;
			aFree(hist);
			hist = hist2;
		}
	}
	if (access_allow)
		aFree(access_allow);
	if (access_deny)
		aFree(access_deny);

	// session[0] のダミーデータを削除
	aFree(session[0]->rdata);
	aFree(session[0]->wdata);
	aFree(session[0]);
}

void do_socket(void)
{
	char *SOCKET_CONF_FILENAME = "conf/packet_athena.conf";

	FD_ZERO(&readfds);

#ifdef TURBO
	FD_ZERO(&writefds);
#endif

	socket_config_read(SOCKET_CONF_FILENAME);

	// session[0] にダミーデータを確保する
	CREATE(session[0], struct socket_data, 1);
	CREATE_A(session[0]->rdata, unsigned char, rfifo_size);
	CREATE_A(session[0]->wdata, unsigned char, wfifo_size);
	session[0]->max_rdata   = rfifo_size;
	session[0]->max_wdata   = wfifo_size;

	// とりあえず５分ごとに不要なデータを削除する
	add_timer_interval(gettick()+1000,connect_check_clear,0,0,300*1000);

#ifdef UPNP
	do_init_upnp();
#endif
}
