#pragma once
#include <stdio.h>
#if defined(WIN32)
	#include <windows.h>
	#include <winsock2.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif
#include <time.h>
#include "malloc.h"

#if defined(__INTERIX) || defined(WIN32)
	#define FD_SETSIZE 4096
#endif

struct socket_data{
	int			eof;
	unsigned char		*rdata, *wdata;
	int			max_rdata,max_wdata;
	int			rdata_size,wdata_size;
	time_t			rdata_tick;
	int			rdata_pos;
	struct sockaddr_in 	client_addr;
	int			(*func_recv)(int);
	int			(*func_send)(int);
	int			(*func_parse)(int);
	int			(*func_console)(char*);
	void			*session_data;
};

class Socket {
	public:
		Socket(void);	// Constructor

		inline struct socket_data*	GetSession(int session_fd) { if(session[session_fd]) return session[session_fd] };
		int				DeleteSession(int session_fd);
		int				DelSession(int session_fd) { if(session[session_fd]) return DeleteSession(session_fd); };

		int				Do_SendRecv(int next);
		int				Do_ParsePacket(void);
		void				Do_Socket(void);

		void				Flush_Fifos(void);
		void				Set_NonBlocking(int fd, int yes);

		int				StartConsole(void);

		void				SetDefaultParse(int (*defaultparse)(int));
		void				SetDefaultConsoleParse(int (*defaultparse)(char*));

		int				ReallocFifo(int fd, int rfifo_size, int wfifo_size);
		int				WFIFOSET(int fd, int len);
		int				RFIFOSKIP(int fd, int len);

		inline int			GetRFIFOSize(void) { return rfifo_size; };
		inline int			GetWFIFOSize(void) { return wfifo_size };
		inline int			GetFDMax(void) { return fd_max };

		int				MakeListenPort(int listen_port);
		int				MakeConnection(long conncetion_ip);
	private:
		struct socket_data	*session[FD_SETSIZE];
		int rfifo_size, wfifo_size, fd_max;
		int port;
		unsigned int addr[16], naddr;	// Host Byte Order
		fd_set readfds;
		time_t tick_, stall_time_;
};
