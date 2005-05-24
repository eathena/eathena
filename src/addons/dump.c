// $Id: dump.c 1 2005-3-10 3:17:17 PM Celestia $

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#if !defined(CYGWIN) && !defined(_WIN32) && !defined(__NETBSD__)	// HAVE_EXECINFO_H
	#include <execinfo.h>
#endif
#include "../common/dll.h"
#include "../common/version.h"
#include "../common/showmsg.h"

ADDON_INFO = {
	"StackDump",
	ADDON_CORE,
	"1.0",
	DLL_VERSION,
	"Creates a stack dump upon crash"
};

ADDON_EVENTS_TABLE = {
	{ "sig_init", "DLL_Init" },
	{ NULL, NULL }
};

//////////////////////////////////////

ADDON_CALL_TABLE = NULL;
extern const char *strsignal(int);
const char* (*getrevision)();
char *argp;

// by Gabuzomeu
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//
#ifdef WIN32	// windows don't have SIGPIPE
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

/*=========================================
 *	Dumps the stack using glibc's backtrace
 *-----------------------------------------
 */
#ifdef CYGWIN
	#define FOPEN_ freopen
	extern void cygwin_stackdump();
#else
	#define FOPEN_(fn,m,s) fopen(fn,m)
#endif
void sig_dump(int sn)
{	
	FILE *fp;
	char file[256];
	int no = 0;
	
	// search for a usable filename
	do {
		sprintf (file, "log/%s%04d.stackdump", argp, ++no);
	} while((fp = fopen(file,"r")) && (fclose(fp), no < 9999));
	// dump the trace into the file

	if ((fp = FOPEN_(file, "w", stderr)) != NULL) {
		const char *revision;
	#ifndef CYGWIN
		void* array[20];
		char **stack;
		size_t size;
	#endif

		printf ("Dumping stack to '"CL_WHITE"%s"CL_RESET"'... ", file);
		if ((revision = getrevision()) != NULL)
			fprintf(fp, "Version: svn%s \n", revision);
		else
			fprintf(fp, "Version: %2d.%02d.%02d mod%02d \n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION, ATHENA_MOD_VERSION);
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
		free(stack);
	#endif

		printf ("Done.\n");
		fflush(stdout);
		fclose(fp);
	}
	// Pass the signal to the system's default handler
	compat_signal(sn, SIG_DFL);
	raise(sn);
}

/*=========================================
 *	Register the signal handlers
 *-----------------------------------------
 */
int sig_init ()
{
	void (*func) = sig_dump;
#ifdef CYGWIN	// test if dumper is enabled
	char *buf = getenv ("CYGWIN");
	if (buf && strstr(buf, "error_start") != NULL)
		func = SIG_DFL;
#endif

	IMPORT_SYMBOL(argp, 4);
	IMPORT_SYMBOL(getrevision, 12);

	compat_signal(SIGSEGV, func);
	compat_signal(SIGFPE, func);
	compat_signal(SIGILL, func);
	#ifndef __WIN32
		compat_signal(SIGBUS, func);
	#endif

	return 1;
}
