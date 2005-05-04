
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "dll.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/utils.h"
#include "../common/malloc.h"
#include "../common/version.h"
#include "../common/showmsg.h"

//////////////////////////////////////////////

struct Addon_Event {
	void (*func)();
	struct Addon_Event *next;
};
struct Addon_Event_List {
	char *name;
	struct Addon_Event_List *next;
	struct Addon_Event *events;
};

int auto_search = 1;
struct Addon_Event_List *event_head = NULL;
struct Addon *addon_head = NULL;

////// Plugin Events Functions //////////////////

int register_addon_func (char* name)
{
	struct Addon_Event_List *evl;
	if (name) {
		evl = (struct Addon_Event_List *) aMalloc(sizeof(struct Addon_Event_List));
		evl->name = (char *) aMalloc (strlen(name) + 1);

		evl->next = event_head;
		strcpy(evl->name, name);
		evl->events = NULL;
		event_head = evl;
	}
	return 0;
}

struct Addon_Event_List *search_addon_func (char *name)
{
	struct Addon_Event_List *evl = event_head;
	while (evl) {
		if (strcmpi(evl->name, name) == 0)
			return evl;
		evl = evl->next;
	}
	return NULL;
}

int register_addon_event (void (*func)(), char* name)
{
	struct Addon_Event_List *evl = search_addon_func(name);
	if (evl) {
		struct Addon_Event *ev;

		ev = (struct Addon_Event *) aMalloc(sizeof(struct Addon_Event));
		ev->func = func;
		ev->next = NULL;

		if (evl->events == NULL)
			evl->events = ev;
		else {
			struct Addon_Event *ev2 = evl->events;
			while (ev2) {
				if (ev2->next == NULL) {
					ev2->next = ev;
					break;
				}
				ev2 = ev2->next;
			}
		}
	}
	return 0;
}

int addon_event_trigger (char *name)
{
	int c = 0;
	struct Addon_Event_List *evl = search_addon_func(name);
	if (evl) {
		struct Addon_Event *ev = evl->events;
		while (ev) {
			ev->func();
			ev = ev->next;
			c++;
		}
	}
	return c;
}

////// Plugins Core /////////////////////////

void dll_open (const char *filename)
{
	struct Addon *addon;
	struct Addon_Info *info;
	struct Addon_Event_Table *events;

	//printf ("loading %s\n", filename);
	
	// Check if the plugin has been loaded before
	addon = addon_head;
	while (addon) {
		if (strcmpi(addon->filename, filename) == 0)
			return;
		addon = addon->next;
	}

	addon = aMallocA(sizeof(struct Addon));
	addon->state = -1;	// not loaded

	addon->dll = DLL_OPEN(filename);
	if (!addon->dll) {
		//printf ("not loaded (invalid file) : %s\n", filename);
		goto err;
	}
	
	// Retrieve plugin information
	addon->state = 0;	// initialising
	DLL_SYM (info, addon->dll, "Addon_Info");
	if (!info ||
		(atof(info->req_version) < DLL_VERSION) ||	// plugin is based on older code
		(info->type != ATHENA_SERVER_ALL && info->type != SERVER_TYPE))
	{
		//printf ("not loaded (incompatible) : %s\n", filename);		
		goto err;
	}
	addon->info = info;

	addon->filename = (char *) aMalloc (strlen(filename) + 1);
	strcpy(addon->filename, filename);
	
	// Register plugin events
	DLL_SYM (events, addon->dll, "Addon_Event_Table");
	if (events) {
		int i = 0;
		while (events[i].func_name) {
			void (*func)(void);
			DLL_SYM (func, addon->dll, events[i].func_name);
			if (func)
				register_addon_event (func, events[i].event_name);
			i++;
		}
	}

	if (addon_head == NULL) {
		addon->next = NULL;
		addon_head = addon;
	} else {
		addon->next = addon_head;
		addon_head = addon;
	}

	addon->state = 1;	// fully loaded
	ShowStatus ("Done loading plugin '"CL_WHITE"%s"CL_RESET"'\n", addon->info->name);
	return;

err:
	if (addon->filename) aFree(addon->filename);
	if (addon->dll) DLL_CLOSE(addon->dll);
	if (addon) aFree(addon);
	return;
}

#ifdef _WIN32
char *DLL_ERROR(void)
{
	static char dllbuf[80];
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, dllbuf, 80, NULL);
	return dllbuf;
}
#endif

////// Initialize/Finalize ////////////////////

int dll_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while (fgets(line, 1020, fp)) {
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "auto_search") == 0) {
			if(strcmpi(w2, "yes")==0)
				auto_search = 1;
			else if(strcmpi(w2, "no")==0)
				auto_search = 0;
			else auto_search = atoi(w2);
		} else if (strcmpi(w1, "addon") == 0) {
			char filename[128];
			sprintf (filename, "addons/%s%s", w2, DLL_EXT);
			dll_open(filename);
		} else if (strcmpi(w1, "import") == 0)
			dll_config_read(w2);
	}
	fclose(fp);
	return 0;
}

void dll_init (void)
{
	char *DLL_CONF_FILENAME = "conf/addon_athena.conf";
	register_addon_func("DLL_Init");
	register_addon_func("DLL_Final");
	register_addon_func("Athena_Init");
	register_addon_func("Athena_Final");

	dll_config_read (DLL_CONF_FILENAME);

	if (auto_search)
		findfile("addons", DLL_EXT, dll_open);

	addon_event_trigger("DLL_Init");

	return;
}

void dll_final (void)
{
	struct Addon *addon = addon_head, *addon2;
	struct Addon_Event_List *evl = event_head, *evl2;
	struct Addon_Event *ev, *ev2;

	addon_event_trigger("DLL_Final");

	while (addon) {
		addon2 = addon->next;
		addon->state = 0;
		aFree(addon->filename);
		DLL_CLOSE(addon->dll);
		aFree(addon);
		addon = addon2;
	}

	while (evl) {
		ev = evl->events;
		while (ev) {
			ev2 = ev->next;
			aFree(ev);
			ev = ev2;
		}
		evl2 = evl->next;
		aFree(evl->name);
		aFree(evl);
		evl = evl2;
	}

	return;
}
