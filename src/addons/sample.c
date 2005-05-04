
#include <stdio.h>

struct {
	char *name;
	char type;
	char *version;
	char *req_version;
	char *description;
} Addon_Info = {
	"Test",
	15,
	"0.1",
	"1.00",
	"A sample plugin"
};

struct {
	char *func_name;
	char *event_name;
} Addon_Event_Table[] = {
	{ "do_init", "DLL_Init" },
	{ "do_final", "DLL_Final" },
	{ NULL, NULL }
};

int do_init ()
{
	printf ("init\n");	
	return 1;
}

int do_final ()
{
	printf ("final\n");
	return 1;
}
