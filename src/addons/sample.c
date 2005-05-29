// Sample Athena plugin

#include <stdio.h>
#include <string.h>
#include "../common/dll.h"

////// Plugin information ////////
//
ADDON_INFO = {
// change only the following area
	"Test",			// Plugin name
	ADDON_ALL,		// Which servers is this plugin for
	"0.1",			// Plugin version
	DLL_VERSION,		// Minimum plugin engine version to run
	"A sample plugin"	// Short description of plugin
};

////// Plugin event list //////////
// Format: <plugin function>,<event name>
// All registered functions to a event gets executed
// (In descending order) when its called.
// Multiple functions can be called by multiple events too,
// So it's up to your creativity ^^
//
ADDON_EVENTS_TABLE = {
// change only the following area
	{ "test_me", "DLL_Test" },	// when the plugin is tested for compatibility
	{ "do_init", "DLL_Init" },	// when plugins are loaded
	{ "do_final", "DLL_Final" },	// when plugins are unloaded
	{ "some_function", "some_event" },
	{ "some_function", "another_event" },
	{ NULL, NULL }
};

///// Import tables and variables /////
ADDON_CALL_TABLE = NULL;
char *server_type;
char *argp;

//////// Plugin functions //////////
int do_init ()
{
	// import symbols from the server
	IMPORT_SYMBOL(server_type, 0);
	IMPORT_SYMBOL(argp, 1);

	printf ("Server type is ");
	switch (*server_type) {
		case ADDON_LOGIN: printf ("Login\n"); break;
		case ADDON_CHAR: printf ("Char\n"); break;
		case ADDON_MAP: printf ("Map\n"); break;
	}
	printf ("Filename is %s\n", argp);

	return 1;
}

int do_final ()
{
	printf ("Bye world\n");

	return 1;
}

int some_function ()
{
	printf ("Some function\n");
	return 0;
}

// return 1 if the testing passes, otherwise 0
// (where the plugin will be deactivated)
int test_me ()
{
	if (1 + 1 == 2)
		return 1;
	return 0;
}
