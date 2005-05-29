
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "../common/dll.h"

ADDON_INFO = {
	"ProcessId",
	ADDON_ALL,
	"1.0",
	DLL_VERSION,
	"Logs the process ID"
};

ADDON_EVENTS_TABLE = {
	{ "pid_create", "DLL_Init" },
	{ "pid_delete", "DLL_Final" },
	{ NULL, NULL }
};

ADDON_CALL_TABLE = NULL;
char pid_file[256];
char *argp;

void pid_create ()
{
	FILE *fp;
	int len;
	
	IMPORT_SYMBOL(argp, 1);
	len = strlen(argp);
	strcpy(pid_file, argp);
	if(len > 4 && pid_file[len - 4] == '.') {
		pid_file[len - 4] = 0;
	}
	strcat(pid_file, ".pid");
	fp = fopen(pid_file, "w");
	if (fp) {
#ifdef _WIN32
		fprintf(fp, "%d", GetCurrentProcessId());
#else
		fprintf(fp, "%d", getpid());
#endif
		fclose(fp);
	}
}

void pid_delete ()
{
	unlink(pid_file);
}
