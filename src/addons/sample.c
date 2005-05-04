// Sample Athena plugin

#include <stdio.h>
#include "../common/dll.h"
#include "../common/version.h"

// Plugin information
//
struct Addon_Info addon_info = {
// change only the following area
	"Test",			// Plugin name
	ATHENA_SERVER_ALL,	// Which servers is this plugin for
	"0.1",			// Plugin version
	DLL_VERSION,		// Minimum plugin engine version to run
	"A sample plugin"	// Short description of plugin
};

// Plugin event list
// Format: <plugin function>,<event name>
// All registered functions to a event gets executed
// (In descending order) when its called.
// Multiple functions can be called by multiple events too,
// So it's up to your creativity ^^
//
struct Addon_Event_Table addon_event_table[] = {
// change only the following area
	{ "do_init", "DLL_Init" },
	{ "do_final", "DLL_Final" },
	{ "some_function", "some_event" },
	{ "some_function", "another_event" },
	{ NULL, NULL }
};

// Plugin functions
int do_init ()
{
	printf ("Hello world\n");
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
