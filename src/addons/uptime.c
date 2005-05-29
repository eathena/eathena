// $Id: uptime.c 1 2005-3-10 3:17:17 PM Celestia $

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../common/dll.h"

ADDON_INFO = {
	"UptimeLog",
	ADDON_CORE,
	"0.1",
	DLL_VERSION,
	"Logs server uptime"
};

ADDON_EVENTS_TABLE = {
	{ "uptime_init", "DLL_Init" },
	{ "uptime_final", "DLL_Final" },
	{ NULL, NULL }
};

ADDON_CALL_TABLE = NULL;
unsigned int (*gettick)();
unsigned long tick = 0;
char *argp;

// Known Issues:
// - it somehow doesn't work in Linux, no idea why ^^;
// - it doesn't work if the server crashes, and using sig_proc
//   would mess up coredumping and my dump addon =/

int uptime_init ()
{
	IMPORT_SYMBOL(argp, 1);
	IMPORT_SYMBOL(gettick, 2);
	tick = gettick();
	return 1;
}

int uptime_final ()
{
	time_t curtime;
	char curtime2[24];	
	FILE *fp;
	long seconds = 0, day = 24*60*60, hour = 60*60,
		minute = 60, days = 0, hours = 0, minutes = 0;

	fp = fopen("log/uptime.log","a");
	if (fp) {
		time(&curtime);
		strftime(curtime2, 24, "%m/%d/%Y %H:%M:%S", localtime(&curtime));

		seconds = ((*gettick)()-tick)/CLOCKS_PER_SEC;
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

	return 1;
}
