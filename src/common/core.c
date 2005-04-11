// $Id: core.c,v 1.1.1.1 2004/09/10 17:44:49 MagicalTux Exp $
// original : core.c 2003/02/26 18:03:12 Rev 1.7

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <signal.h>
#include <string.h>
#ifdef DUMPSTACK
	#ifndef CYGWIN	// HAVE_EXECINFO_H
		#include <execinfo.h>
	#endif
#endif

#include "core.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/version.h"
#include "../common/showmsg.h"
#include "svnversion.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

char *argp;
int runflag = 1;
char SERVER_TYPE = SERVER_NONE;
unsigned long ticks = 0; // by MC Cameri
char pid_file[256];
static void (*term_func)(void)=NULL;

/*======================================
 *	CORE : Set function
 *--------------------------------------
 */
void set_termfunc(void (*termfunc)(void))
{
	term_func = termfunc;
}

// Added by Gabuzomeu
//
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//
#ifndef SIGPIPE
#define SIGPIPE SIGINT
#endif

#ifndef POSIX
#define compat_signal(signo, func) signal(signo, func)
#else
sigfunc *compat_signal(int signo, sigfunc *func)
{
  struct sigaction sact, oact;

  sact.sa_handler = func;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
#ifdef SA_INTERRUPT
  sact.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

  if (sigaction(signo, &sact, &oact) < 0)
    return (SIG_ERR);

  return (oact.sa_handler);
}
#endif

/*======================================
 *	CORE : Signal Sub Function
 *--------------------------------------
 */
// for handling certain signals ourselves, like SIGPIPE
static void sig_ignore(int sn) {
	ShowMessage ("Broken pipe found... closing socket\n");	// set to eof in socket.c
	return;	// does nothing here
}
static void sig_proc(int sn)
{
	static int is_called = 0;

	if(++is_called > 3)
		exit(0);

	switch(sn){
	case SIGINT:
	case SIGTERM:
                runflag = 0;
		break;
	}
}

/*=========================================
 *	Dumps the stack using glibc's backtrace
 *-----------------------------------------
 */
#ifndef DUMPSTACK
	#define sig_dump SIG_DFL
#else	
	#ifdef CYGWIN
		#define FOPEN_ freopen
		extern void cygwin_stackdump();
	#else
		#define FOPEN_(fn,m,s) fopen(fn,m)
	#endif
extern const char *strsignal(int);
void sig_dump(int sn)
{	
	FILE *fp;
	char file[256];
	int no = 0;
	
	#ifndef CYGWIN
		void* array[20];
		char **stack;
		size_t size;
	#endif

	// search for a usable filename
	do {
		sprintf (file, "log/%s%04d.stackdump", argp, ++no);
	} while((fp = fopen(file,"r")) && (fclose(fp), no < 9999));
	// dump the trace into the file

	if ((fp = FOPEN_(file, "w", stderr)) != NULL) {
		ShowMessage ("Dumping stack... ");
		fprintf(fp, "Exception: %s \n", strsignal(sn));
		fflush (fp);

	#ifdef CYGWIN
		cygwin_stackdump ();
	#else
		fprintf(fp, "Stack trace:\n");
		size = backtrace (array, 20);
		stack = backtrace_symbols (array, size);
		for (no = 0; no < size; no++) {
			fprintf(fp, "%s\n", stack[no]);
		}
		fprintf(fp,"End of stack trace\n");
		aFree(stack);
	#endif

		ShowMessage ("Done.\n");
		fflush(stdout);
		fclose(fp);
	}
	// Pass the signal to the system's default handler
	compat_signal(sn, SIG_DFL);
	raise(sn);
}
#endif

#ifdef SVNVERSION

#define xstringify(x)  stringify(x)
#define stringify(x)  #x

const char *get_svn_revision() {
       return xstringify(SVNVERSION);
}

#else

const char* get_svn_revision() {
	static char version[10];
	char line[1024];
	int rev = 0;
	FILE *fp;
	if ((fp = fopen(".svn/entries", "r")) == NULL) {
		return "Unknown";
	} else {
		while (fgets(line,1023,fp))
			if (strstr(line,"revision=")) break;
		fclose(fp);
		if (sscanf(line," %*[^\"]\"%d%*[^\n]",&rev) == 1) {
			sprintf(version, "%d", rev);
			return version;
		} else
			return "Unknown";
	}
	return 0;
}
#endif

/*======================================
 *	CORE : Display title
 *--------------------------------------
 */

static void display_title(void)
{
	const char *revision;
	ClearScreen(); // clear screen and go up/left (0, 0 position in text)
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n"); // white writing (37) on blue background (44), \033[K clean until end of file
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"        (c)2005 eAthena Development Team presents        "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // yellow writing (33)
	ShowMessage(""CL_XXBL"          ("CL_BOLD"       ______  __    __                                  "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      /\\  _  \\/\\ \\__/\\ \\                     v%2d.%02d.%02d   "CL_XXBL")"CL_CLL""CL_NORMAL"\n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"    __\\ \\ \\_\\ \\ \\ ,_\\ \\ \\___      __    ___      __      "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  /'__`\\ \\  __ \\ \\ \\/\\ \\  _ `\\  /'__`\\/' _ `\\  /'__`\\    "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD" /\\  __/\\ \\ \\/\\ \\ \\ \\_\\ \\ \\ \\ \\/\\  __//\\ \\/\\ \\/\\ \\_\\.\\_  "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD" \\ \\____\\\\ \\_\\ \\_\\ \\__\\\\ \\_\\ \\_\\ \\____\\ \\_\\ \\_\\ \\__/.\\_\\ "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  \\/____/ \\/_/\\/_/\\/__/ \\/_/\\/_/\\/____/\\/_/\\/_/\\/__/\\/_/ "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   _   _   _   _   _   _   _     _   _   _   _   _   _   "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  / \\ / \\ / \\ / \\ / \\ / \\ / \\   / \\ / \\ / \\ / \\ / \\ / \\  "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD" ( e | n | g | l | i | s | h ) ( A | t | h | e | n | a ) "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/  "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // 1: bold char, 0: normal char
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // yellow writing (33)
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"  Advanced Fusion Maps (c) 2003-2005 The Fusion Project  "CL_XXBL")"CL_CLL""CL_NORMAL"\n"); // yellow writing (33)
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n"); // reset color
	
	if ((revision = get_svn_revision())!=NULL) {
		ShowInfo("SVN Revision: '"CL_WHITE"%s"CL_RESET"'.\n",revision);
	}
}

/*======================================
 *	CORE : MAINROUTINE
 *--------------------------------------
 */

void pid_delete(void) {
	unlink(pid_file);
}

void pid_create (void) {
	FILE *fp;
	int len = strlen(argp);
	strcpy(pid_file, argp);
	if(len > 4 && pid_file[len - 4] == '.') {
		pid_file[len - 4] = 0;
	}
	strcat(pid_file,".pid");
	fp = fopen(pid_file,"w");
	if(fp) {
#ifdef _WIN32
		fprintf(fp,"%d",GetCurrentProcessId());
#else
		fprintf(fp,"%d",getpid());
#endif
		fclose(fp);
	}
}

#define LOG_UPTIME 0
void log_uptime(void)
{
#if LOG_UPTIME
	time_t curtime;
	char curtime2[24];
	FILE *fp;
	long seconds = 0, day = 24*60*60, hour = 60*60,
		minute = 60, days = 0, hours = 0, minutes = 0;

	fp = fopen("log/uptime.log","a");
	if (fp) {
		time(&curtime);
		strftime(curtime2, 24, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

		seconds = (gettick()-ticks)/CLOCKS_PER_SEC;
		days = seconds/day;
		seconds -= (seconds/day>0)?(seconds/day)*day:0;
		hours = seconds/hour;
		seconds -= (seconds/hour>0)?(seconds/hour)*hour:0;
		minutes = seconds/minute;
		seconds -= (seconds/minute>0)?(seconds/minute)*minute:0;

		fprintf(fp, "%s: %s uptime - %ld days, %ld hours, %ld minutes, %ld seconds.\n",
			curtime2, argp, days, hours, minutes, seconds);
		fclose(fp);
	}

	return;
#endif
}

int main(int argc,char **argv)
{
	int next;

	if ((argp = strstr(argv[0], "./")) != NULL)
		argp+=2;
	else argp = argv[0];

	display_title();
	
	do_init_malloc(); // àÍî‘ç≈èâÇ…é¿çsÇ∑ÇÈïKóvÇ™Ç†ÇÈ
	pid_create();
	Net_Init();
	do_socket();
	
	compat_signal(SIGPIPE, sig_ignore);
	compat_signal(SIGTERM,sig_proc);
	compat_signal(SIGINT,sig_proc);

	// Signal to create coredumps by system when necessary (crash)
	compat_signal(SIGSEGV, sig_dump);
	compat_signal(SIGFPE, sig_dump);
	compat_signal(SIGILL, sig_dump);
	#ifndef _WIN32
		compat_signal(SIGBUS, sig_dump);
		compat_signal(SIGTRAP, SIG_DFL);
	#endif

	tick_ = time(0);
	ticks = gettick();

	do_init(argc,argv);

	while(runflag){
		next=do_timer(gettick_nocache());
		do_sendrecv(next);
#ifndef TURBO
		do_parsepacket();
#endif
	}

	do_final();
	log_uptime();
	pid_delete();
	do_final_socket();
	do_final_malloc();

	return 0;
}

#ifdef BCHECK
unsigned int __invalid_size_argument_for_IOC;
#endif
