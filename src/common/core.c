#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
		#include <unistd.h>
#endif
#include <signal.h>
#include <string.h>
#include "core.h"	// Core Header
#include "socket.h"	// Socket Header
#include "timer.h"	// Timer Header
#include "version.h"	// Version Header
#include "showmsg.h"	// ShowMsg Header
#if defined(MEMWATCH)
		#include "memwatch.h"
#endif

static void (*term_func)(void) = NULL;

Core::Core(void)
{
	Core(0, NULL);
	return;
}

Core::Core(int argc, char **argv)
{
	int next;

	Socket *CoreSocket = new Socket;
	CoreSocket->Do_Socket();
	
	compat_signal(SIGPIPE,SIG_IGN);
	compat_signal(SIGTERM,sig_proc);
	compat_signal(SIGINT,sig_proc);
	
	// Signal to create coredumps by system when necessary (crash)
	compat_signal(SIGSEGV, SIG_DFL);
#ifndef WIN32
	compat_signal(SIGBUS, SIG_DFL);
	compat_signal(SIGTRAP, SIG_DFL); 
#endif
	compat_signal(SIGILL, SIG_DFL);

	display_title();

	tick_ = time(0);

	do_init(argc,argv);
	while(runflag){
		next=do_timer(gettick_nocache());
		do_sendrecv(next);
		do_parsepacket();
	}

	if(CoreSocket)
		free(CoreSocket);
	CoreSocket = NULL;

	return;
}

void	Core::SetTermFunc(void (*termfunc)(void))
{
	term_func = termfunc;
	return;
}

static void	Core::Sig_Proc(int sn)
{
	int i;
	switch(sn)
	{
		case SIGINT:
		case SIGTERM:
			if(term_func)
				term_func();
			for(i=0; i<fd_max; i++){
				if(!session[i])
					continue;
				close(i);
			}
		exit(0);
		break;
	}
	return;
}

static int	Core::Get_SVNRev(char *svnentry)
{
	char line[1024];
	int rev=0;
	FILE *fp;
	if(!(fp = fopen(svnentry, "r")))
	{
		return 0;
	} else {
		while(fgets(line, (sizeof(line))-1, fp))
			if(strstr(line,"revision="))
				break;
		fclose(fp);
		if(sscanf(line," %*[^\"]\"%d%*[^\n]",&rev) ==1) 
			return rev;
		else
			return 0;
	}
	return 0;
}

static void	Core::Display_Title(void)
{
	int revision;

	// for help with the console colors look here:
	// http://www.edoceo.com/liberum/?doc=printf-with-color
	// some code explanation (used here):
	// \033[2J : clear screen and go up/left (0, 0 position)
	// \033[K  : clear line from actual position to end of the line
	// \033[0m : reset color parameter
	// \033[1m : use bold for font

	printf("\033[2J"); // clear screen and go up/left (0, 0 position in text)
	printf("\033[37;44m          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)\033[K\033[0m\n"); // white writing (37) on blue background (44), \033[K clean until end of file
	printf("\033[0;44m          (\033[1;33m        (c)2004 eAthena Development Team presents        \033[0;44m)\033[K\033[0m\n"); // yellow writing (33)
	printf("\033[0;44m          (\033[1m       ______  __    __                                  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m      /\\  _  \\/\\ \\__/\\ \\                     v%2d.%02d.%02d   \033[0;44m)\033[K\033[0m\n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m    __\\ \\ \\_\\ \\ \\ ,_\\ \\ \\___      __    ___      __      \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  /'__`\\ \\  __ \\ \\ \\/\\ \\  _ `\\  /'__`\\/' _ `\\  /'__`\\    \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m /\\  __/\\ \\ \\/\\ \\ \\ \\_\\ \\ \\ \\ \\/\\  __//\\ \\/\\ \\/\\ \\_\\.\\_  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m \\ \\____\\\\ \\_\\ \\_\\ \\__\\\\ \\_\\ \\_\\ \\____\\ \\_\\ \\_\\ \\__/.\\_\\ \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  \\/____/ \\/_/\\/_/\\/__/ \\/_/\\/_/\\/____/\\/_/\\/_/\\/__/\\/_/ \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m   _   _   _   _   _   _   _     _   _   _   _   _   _   \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  / \\ / \\ / \\ / \\ / \\ / \\ / \\   / \\ / \\ / \\ / \\ / \\ / \\  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m ( e | n | g | l | i | s | h ) ( A | t | h | e | n | a ) \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m                                                         \033[0;44m)\033[K\033[0m\n"); // yellow writing (33)
	printf("\033[0;44m          (\033[1;33m  Advanced Fusion Maps (c) 2003-2004 The Fusion Project  \033[0;44m)\033[K\033[0m\n"); // yellow writing (33)
	printf("\033[37;44m          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)\033[K\033[0m\n\n"); // reset color
	
	if((revision = Get_SVNRev(".svn\\entries")) >0)
	{
		snprintf(ShowMsgtmp_output, sizeof(tmp_output), "SVN Revision: '"CL_WHITE"%d"CL_RESET"'.\n", revision);
		ShowMsg.ShowInfo(tmp_output);
	}
	return;
}

sigfunc*	Core::compat_signal(int signo, sigfunc *func)
{
#ifndef POSIX
	signal(signo, func);
#else
	struct sigaction sact, oact;

  	sact.sa_handler = func;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
	#if defined(SA_INTERRUPT)
		sact.sa_flags |= SA_INTERRUPT;	// SunOS
	#endif

	if(sigaction(signo, &sact, &oact) < 0)
		return (SIG_ERR);

	return (oact.sa_handler);
#endif
}