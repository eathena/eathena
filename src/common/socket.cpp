#include "socket.h"	// Socket Header
#include <stdlib.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifndef SIOCGIFCONF
	#include <sys/sockio.h>
#endif
#include <fcntl.h>
#include <string.h>
#include "mmo.h"
#include "utils."

#if defined(MEMWATCH)
	#include "memwatch.h"
#endif
#ifndef TCP_FRAME_LEN
	#define TCP_FRAME_LEN 1053
#endif

static int null_parse(int fd);
static int null_console_parse(char *buf);
static int (*default_func_parse)(int) = null_parse;
static int (*default_console_parse)(char*) = null_console_parse;

Socket::Socket(void)
{
	rfifo_size = wfifo_size = 65536;
	stall_time_ = 60;

	#if defined(WIN32)
		char** a;
		unsigned int i;
		char fullhost[255];
		struct hostent *hent;
    
		// Start up the windows networking
		WSADATA wsaData;

		if(WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
		{
			printf("SYSERR: WinSock not available!\n");
			exit(1);
		}

		if(gethostname(fullhost, sizeof(fullhost)) == SOCKET_ERROR)
		{
			printf("No hostname defined!\n");
			return -1;
		} 

		// XXX This should look up the local IP addresses in the registry
		// instead of calling gethostbyname. However, the way IP addresses
		// are stored in the registry is annoyingly complex, so I'll leave
		// this as T.B.D.
		hent = gethostbyname(fullhost);
		if(!hent)
		{
			printf("Cannot resolve hostname to an IP address.\n");
			return -1;
		}

		a = hent->h_addr_list;
		for(i = 0; (a[i] != 0) && (i < 16); ++i)
		{
			addr[i] = (unsigned int)ntohl(*(unsigned long*) a[i]);
		}
		naddr = i;
	#else
		int pos;
		int fdes = socket(AF_INET, SOCK_STREAM, 0);
		char buf[16 * sizeof(struct ifreq)];
		struct ifconf ic;

		// The ioctl call will fail with Invalid Argument if there are more
		// interfaces than will fit in the buffer
		ic.ifc_len = sizeof(buf);
		ic.ifc_buf = buf;
		if(ioctl(fdes, SIOCGIFCONF, &ic) == -1)
		{
			printf("SIOCGIFCONF failed!\n");
			return -1;
		}

		for(pos=0; pos < ic.ifc_len;) 
		{
			struct ifreq *ir = (struct ifreq*) (ic.ifc_buf + pos);
			struct sockaddr_in *a = (struct sockaddr_in*) &(ir->ifr_addr);

			if(a->sin_family == AF_INET)
			{
				u_long ad = ntohl(a->sin_addr.s_addr);
				if(ad != INADDR_LOOPBACK)
				{
					addr_[naddr_ ++] = ad;
					if(naddr_ == 16)
						break;
				}
			}

			#if defined(_AIX) || defined(__APPLE__)	// MacOS portability
				pos += ir->ifr_addr.sa_len;
				pos += sizeof(ir->ifr_name);
			#else
				pos += sizeof(struct ifreq);
			#endif
		}
	#endif

	return 0;
}

int	Socket::DeleteSession(int session_fd)
{
	if(session_fd<0 || session_fd>=FD_SETSIZE)
		return -1;
	FD_CLR(session_fd, &readfds);
	if(session[session_fd)
	{
		if(session[session_fd]->rdata)
			free(session[session_fd]->rdata);
		if(session[session_fd]->wdata)
			free(session[session_fd]->wdata);
		if(session[session_fd]->session_data)
			free(session[session_fd]->session_data);
		free(session[session_fd]);
	}
	session[fd] = NULL;
	return 0;
}

int	Socket::Do_SendRecv(int next)
{
	fd_set rfd, wfd;
	struct timeval timeout;
	int ret, i;

	tick_ = time(0);

	memcpy(&rfd, &readfds, sizeof(rfd));

	FD_ZERO(&wfd);
	for(i=0;i<fd_max;i++)
	{
		if(!session[i] && FD_ISSET(i, &readfds))
		{
			printf("force clr fds %d\n",i);
			FD_CLR(i,&readfds);
			continue;
		}
		if(!session[i])
			continue;
		if(session[i]->wdata_size)
			FD_SET(i,&wfd);
	}
	timeout.tv_sec  = next/1000;
	timeout.tv_usec = next%1000*1000;
	ret = select(fd_max,&rfd,&wfd,NULL,&timeout);
	if(ret<=0)
		return 0;
	for(i=0;i<fd_max;i++)
	{
		if(!session[i])
			continue;
		if(FD_ISSET(i,&wfd) && session[i]->func_send)
			session[i]->func_send(i);
		if(FD_ISSET(i,&rfd) && session[i]->func_recv)
			session[i]->func_recv(i);
	}
	return 0;
}

int	Socket::Do_ParsePacket(void)
{
	for(int i=0;i<fd_max;i++)
	{
		if(!session[i])
			continue;
		if((session[i]->rdata_tick != 0) && ((tick_ - session[i]->rdata_tick) > stall_time_)) 
			session[i]->eof = 1;
		if(session[i]->rdata_size==0 && session[i]->eof==0)
			continue;
		if(session[i]->func_parse)
		{
			session[i]->func_parse(i);
			if(!session[i])
				continue;
		}
		RFIFOFLUSH(i);
	}
	return 0;
}

void	Socket::Do_Socket(void)
{
	FD_ZERO(&readfds);
	return;
}

void	Socket::Flush_Fifos(void)
{
	for(int i=0;i<fd_max;i++)
		if(session[i] != NULL && session[i]->func_send == send_from_fifo)
			send_from_fifo(i);
	return;
}

void	Socket::Set_NonBlocking(int fd, int yes)
{
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
	return;
}

int	Socket::StartConsole(void)
{
	FD_SET(0,&readfds);
    
	CREATE(session[0], struct socket_data, 1);
	if(!session[0])
	{
		printf("out of memory : start_console\n");
		exit(1);
	}
	
	memset(session[0], 0, sizeof(*session[0]));
	
	session[0]->func_recv = console_recieve;
	session[0]->func_console = default_console_parse;
	
	return 0;
	return;
}

void	Socket::SetDefaultParse(int (*defaultparse)(int))
{
	default_func_parse = defaultparse;
	return;
}

void	Socket::SetDefaultConsole(int (*defaultparse)(char*))
{
	default_console_parse = defaultparse;
	return;
}

int	Socket::ReallocFifo(int fd, int rfifo_size, int wfifo_size)
{
	struct socket_data *s = session[fd];

	if(s->max_rdata != rfifo_size && s->rdata_size < rfifo_size)
	{
		RECREATE(s->rdata, unsigned char, rfifo_size);
		s->max_rdata  = rfifo_size;
	}

	if(s->max_wdata != wfifo_size && s->wdata_size < wfifo_size)
	{
		RECREATE(s->wdata, unsigned char, wfifo_size);
		s->max_wdata  = wfifo_size;
	}

	return 0;
}

int	Socket::WFIFOSET(int fd, int len)
{
	struct socket_data *s = session[fd];
	if(!s || !s->wdata)
		return 0;
	if(s->wdata_size + len + 16384 > s->max_wdata )
	{
		unsigned char *sin_addr = (unsigned char *)&s->client_addr.sin_addr;
		realloc_fifo(fd,s->max_rdata, s->max_wdata << 1);
		printf("socket: %d (%d.%d.%d.%d) wdata expanded to %d bytes.\n",fd, sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], s->max_wdata);
	}
	s->wdata_size = (s->wdata_size+(len)+2048 < s->max_wdata) ? s->wdata_size+len : (printf("socket: %d wdata lost !!\n",fd),s->wdata_size);
	if (s->wdata_size > (TCP_FRAME_LEN)) 
		send_from_fifo(fd);
	return 0;
}

int	Socket::RFIFOSKIP(int fd, int len)
{
	struct socket_data *s = session[fd];
	if(s->rdata_size-s->rdata_pos-len < 0)
	{
		fprintf(stderr,"too many skip\n");
		exit(1);
	}

	s->rdata_pos = s->rdata_pos + len;

	return 0;
}

int	Socket::MakeListenPort(int listen_port)
{
	struct sockaddr_in server_address;
	int fd, result;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_max <= fd)
		fd_max = fd + 1;

#if defined(WIN32)
		unsigned long val = 1;
		ioctlsocket(fd, FIONBIO, &val);
#else
		result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	setsocketopts(fd);

	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port        = htons(listen_port);

	result = bind(fd, (struct sockaddr*)&server_address, sizeof(server_address));
	if(result == -1)
	{
		perror("bind");
		exit(1);
	}

	result = listen(fd, 5);

	if(result == -1)
	{
		perror("listen");
		exit(1);
	}

	FD_SET(fd, &readfds);

	CREATE(session[fd], struct socket_data, 1);

	if(!session[fd])
	{
		printf("out of memory : make_listen_port\n");
		exit(1);
	}

	memset(session[fd],0,sizeof(*session[fd]));

	session[fd]->func_recv = connect_client;

	return fd;
}

int	Socket::MakeConnection(long conncetion_ip)
{
	struct sockaddr_in server_address;
	int fd;
	int result;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_max <= fd) 
		fd_max=fd+1;

	setsocketopts(fd);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = ip;
	server_address.sin_port = htons(port);

#if defined(WIN32)
		unsigned long val = 1;
		ioctlsocket(fd, FIONBIO, &val);
#else
		result = fcntl(fd, F_SETFL, O_NONBLOCK);
#endif

	result = connect(fd, (struct sockaddr *)(&server_address),sizeof(struct sockaddr_in));

	FD_SET(fd,&readfds);

	CREATE(session[fd], struct socket_data, 1);
	CREATE(session[fd]->rdata, unsigned char, rfifo_size);
	CREATE(session[fd]->wdata, unsigned char, wfifo_size);

	session[fd]->max_rdata  = rfifo_size;
	session[fd]->max_wdata  = wfifo_size;
	session[fd]->func_recv  = recv_to_fifo;
	session[fd]->func_send  = send_from_fifo;
	session[fd]->func_parse = default_func_parse;
	session[fd]->rdata_tick = tick_;

	return fd;
}
