// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "core.h"
#include "../common/db.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/plugins.h"
#include "../common/utils.h" // filesize()
#ifndef _WIN32
#include "svnversion.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif


int runflag = SERVER_STATE_RUN;
int arg_c = 0;
char **arg_v = NULL;

char *SERVER_NAME = NULL;
char SERVER_TYPE = ATHENA_SERVER_NONE;


// Added by Gabuzomeu
//
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

/*======================================
 *	CORE : Signal Sub Function
 *--------------------------------------*/
static void sig_proc(int sn)
{
	static int is_called = 0;

	switch (sn) {
	case SIGINT:
	case SIGTERM:
		if (++is_called > 3)
			exit(EXIT_SUCCESS);
		do_shutdown();
		break;
	case SIGSEGV:
	case SIGFPE:
		do_abort();
		// Pass the signal to the system's default handler
		compat_signal(sn, SIG_DFL);
		raise(sn);
		break;
#ifndef _WIN32
	case SIGXFSZ:
		// ignore and allow it to set errno to EFBIG
		ShowWarning ("Max file size reached!\n");
		//run_flag = 0;	// should we quit?
		break;
	case SIGPIPE:
		//ShowInfo ("Broken pipe found... closing socket\n");	// set to eof in socket.c
		break;	// does nothing here
#endif
	}
}

void signals_init (void)
{
	compat_signal(SIGTERM, sig_proc);
	compat_signal(SIGINT, sig_proc);
#ifndef _DEBUG // need unhandled exceptions to debug on Windows
	compat_signal(SIGSEGV, sig_proc);
	compat_signal(SIGFPE, sig_proc);
#endif
#ifndef _WIN32
	compat_signal(SIGILL, SIG_DFL);
	compat_signal(SIGXFSZ, sig_proc);
	compat_signal(SIGPIPE, sig_proc);
	compat_signal(SIGBUS, SIG_DFL);
	compat_signal(SIGTRAP, SIG_DFL);
#endif
}

#ifdef SVNVERSION
	const char *get_svn_revision(void)
	{
		return EXPAND_AND_QUOTE(SVNVERSION);
	}
#else// not SVNVERSION
const char* get_svn_revision(void)
{
	static char svn_version_buffer[16] = "";
	FILE *fp;

	if( svn_version_buffer[0] != '\0' )
		return svn_version_buffer;

	// subversion 1.7 uses a sqlite3 database
	// FIXME this is hackish at best...
	// - ignores database file structure
	// - assumes the data in NODES.dav_cache column ends with "!svn/ver/<revision>/<path>)"
	// - since it's a cache column, the data might not even exist
	if( (fp = fopen(".svn"PATHSEP_STR"wc.db", "rb")) != NULL || (fp = fopen(".."PATHSEP_STR".svn"PATHSEP_STR"wc.db", "rb")) != NULL )
	{
	#ifndef SVNNODEPATH
		//not sure how to handle branches, so i'll leave this overridable define until a better solution comes up
		#define SVNNODEPATH trunk
	#endif
		const char* prefix = "!svn/ver/";
		const char* postfix = "/"EXPAND_AND_QUOTE(SVNNODEPATH)")"; // there should exist only 1 entry like this
		size_t prefix_len = strlen(prefix);
		size_t postfix_len = strlen(postfix);
		size_t i,j,len;
		char* buffer;

		// read file to buffer
		len = filesize(fp);
		buffer = (char*)aMalloc(len + 1);
		len = fread(buffer, 1, len, fp);
		buffer[len] = '\0';
		fclose(fp);

		// parse buffer
		for( i = prefix_len + 1; i + postfix_len <= len; ++i )
		{
			if( buffer[i] != postfix[0] || memcmp(buffer + i, postfix, postfix_len) != 0 )
				continue; // postfix missmatch
			for( j = i; j > 0; --j )
			{// skip digits
				if( !ISDIGIT(buffer[j - 1]) )
					break;
			}
			if( memcmp(buffer + j - prefix_len, prefix, prefix_len) != 0 )
				continue; // prefix missmatch
			// done
			snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(buffer + j));
			break;
		}
		aFree(buffer);

		if( svn_version_buffer[0] != '\0' )
			return svn_version_buffer;
	}

	// subversion 1.6 and older?
	if ((fp = fopen(".svn/entries", "r")) != NULL)
	{
		char line[1024];
		int rev;
		// Check the version
		if (fgets(line, sizeof(line), fp))
		{
			if(!ISDIGIT(line[0]))
			{
				// XML File format
				while (fgets(line,sizeof(line),fp))
					if (strstr(line,"revision=")) break;
				if (sscanf(line," %*[^\"]\"%d%*[^\n]", &rev) == 1) {
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", rev);
				}
			}
			else
			{
				// Bin File format
				fgets(line, sizeof(line), fp); // Get the name
				fgets(line, sizeof(line), fp); // Get the entries kind
				if(fgets(line, sizeof(line), fp)) // Get the rev numver
				{
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(line));
				}
			}
		}
		fclose(fp);

		if( svn_version_buffer[0] != '\0' )
			return svn_version_buffer;
	}

	// fallback
	snprintf(svn_version_buffer, sizeof(svn_version_buffer), "Unknown");
	return svn_version_buffer;
}
#endif

/*======================================
 *	CORE : Display title
 *--------------------------------------*/
static void display_title(void)
{
	//ClearScreen(); // clear screen and go up/left (0, 0 position in text)
	ShowMessage("\n");
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"            eAthena Development Team presents            "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"       ______  __    __                                  "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"      /\\  _  \\/\\ \\__/\\ \\                                 "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"    __\\ \\ \\_\\ \\ \\ ,_\\ \\ \\___      __    ___      __      "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  /'__`\\ \\  __ \\ \\ \\/\\ \\  _ `\\  /'__`\\/' _ `\\  /'__`\\    "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD" /\\  __/\\ \\ \\/\\ \\ \\ \\_\\ \\ \\ \\ \\/\\  __//\\ \\/\\ \\/\\ \\_\\.\\_  "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD" \\ \\____\\\\ \\_\\ \\_\\ \\__\\\\ \\_\\ \\_\\ \\____\\ \\_\\ \\_\\ \\__/.\\_\\ "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  \\/____/ \\/_/\\/_/\\/__/ \\/_/\\/_/\\/____/\\/_/\\/_/\\/__/\\/_/ "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   _   _   _   _   _   _   _     _   _   _   _   _   _   "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  / \\ / \\ / \\ / \\ / \\ / \\ / \\   / \\ / \\ / \\ / \\ / \\ / \\  "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD" ( e | n | g | l | i | s | h ) ( A | t | h | e | n | a ) "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"  \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/  "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n");

	ShowInfo("SVN Revision: '"CL_WHITE"%s"CL_RESET"'.\n", get_svn_revision());
}

// Warning if logged in as superuser (root)
void usercheck(void)
{
#ifndef _WIN32
    if ((getuid() == 0) && (getgid() == 0)) {
	ShowWarning ("You are running eAthena as the root superuser.\n");
	ShowWarning ("It is unnecessary and unsafe to run eAthena with root privileges.\n");
	sleep(3);
    }
#endif
}

/*======================================
 *	CORE : MAINROUTINE
 *--------------------------------------*/
int main (int argc, char **argv)
{
	{// initialize program arguments
		char *p1 = SERVER_NAME = argv[0];
		char *p2 = p1;
		while ((p1 = strchr(p2, '/')) != NULL || (p1 = strchr(p2, '\\')) != NULL)
		{
			SERVER_NAME = ++p1;
			p2 = p1;
		}
		arg_c = argc;
		arg_v = argv;
	}

	malloc_init();// needed for Show* in display_title() [FlavioJS]

	set_server_type();
	display_title();
	usercheck();

	db_init();
	signals_init();
	timer_init();
	socket_init();
	plugins_init();

	do_init(argc,argv);
	plugin_event_trigger(EVENT_ATHENA_INIT);

	{// Main runtime cycle
		int next;
		while (runflag != SERVER_STATE_STOP) {
			next = do_timer(gettick_nocache());
			do_sockets(next);
		}
	}

	plugin_event_trigger(EVENT_ATHENA_FINAL);
	do_final();

	timer_final();
	plugins_final();
	socket_final();
	db_final();
	malloc_final();

	return 0;
}
