#include "base.h"

#include "mmo.h"
#include "core.h"
#include "utils.h"
#include "malloc.h"
#include "version.h"
#include "showmsg.h"

#include "dll.h"




//////////////////////////////////////////////////////////////////////////
// defines 
// would be suitable for a header, but does not affect any other file, 
// so keep it away from global namespace
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
// not much necessary here
// we change the unix to equal windows style

#define DLL_EXT		".dll"

typedef HINSTANCE DLL;

const char *LibraryError(void);

#else

#  include <dlfcn.h>

// implement a windows like loading environment
#ifdef CYGWIN
	#define DLL_EXT		".dll"
#else
	#define DLL_EXT		".so"
#endif

typedef void* DLL;

extern inline DLL LoadLibrary(const char* lpLibFileName)
{
	return dlopen(lpLibFileName,RTLD_LAZY);
}
extern inline bool FreeLibrary(DLL hLibModule)
{
	return 0==dlclose(hLibModule);
}
extern inline void *GetProcAddress(DLL hModule, const char* lpProcName)
{
	return dlsym(hModule, lpProcName);
}
extern inline const char *LibraryError(void)
{
	return dlerror();
}
#endif



#ifdef WIN32
const char *LibraryError(void)
{
	static char dllbuf[80];
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, dllbuf, 80, NULL);
	return dllbuf;
}
#endif


//////////////////////////////////////////////////////////////////////////
// data definitions


//////////////////////////////////////////////////////////////////////////
////// Plugin Export functions /////////////

// same layout than ServerType from version.h
typedef enum 
{
	ADDON_NONE	=	0x00,
	ADDON_LOGIN	=	0x01,
	ADDON_CHAR	=	0x02,
	ADDON_INTER	=	0x04,
	ADDON_MAP	=	0x08,
	ADDON_CORE	=	0x10,
	ADDON_xxx1	=	0x20,
	ADDON_xxx2	=	0x40,
	ADDON_xxx3	=	0x80,
	ADDON_ALL	=	0xFF
} AddonType;


struct Addon_Info {
	char *name;
	unsigned char type;
	char *version;
	char *req_version;
	char *description;
};

Addon_Info default_info = { "Unknown", (unsigned char)ADDON_ALL, "0", DLL_VERSION, "Unknown" };

struct Addon_Event_Table {
	char *func_name;
	char *event_name;
};




//////////////////////////////////////////////////////////////////////////
// storage for loading dlls
struct Addon {
	DLL dll;
	char state;
	char *filename;
	struct Addon_Info *info;
	struct Addon *next;	
};

struct Addon_Event {
	void (*func)();
	struct Addon_Event *next;
};

struct Addon_Event_List {
	char *name;
	struct Addon_Event_List *next;
	struct Addon_Event *events;
};



//////////////////////////////////////////////
// file global data

int auto_search = 1;
struct Addon_Event_List *event_head = NULL;
struct Addon *addon_head = NULL;


void **call_table = NULL;
size_t call_table_size	= 0;
size_t max_call_table	= 0;


//////////////////////////////////////////////
////// Plugin Events Functions //////////////////

int register_addon_func (char *name)
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
		if (strcasecmp(evl->name, name) == 0)
			return evl;
		evl = evl->next;
	}
	return NULL;
}

int register_addon_event (void (*func)(), char* name)
{
	struct Addon_Event_List *evl = search_addon_func(name);
	if (!evl) {
		// register event if it doesn't exist already
		register_addon_func(name);
		// relocate the new event list
		evl = search_addon_func(name);
	}
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
	if (evl)
	{
		struct Addon_Event *ev = evl->events;
		while (ev)
		{
			ev->func();
			ev = ev->next;
			c++;
		}
	}
	return c;
}

////// Plugins Call Table Functions /////////

int export_symbol(void *var, int offset)
{
	// add to the end of the list
	if (offset < 0)
		offset = call_table_size;

	// realloc if not large enough
	if((size_t)offset >= max_call_table)
	{
		max_call_table = 1+offset;
		call_table = (void**)aRealloc(call_table, max_call_table*sizeof(void*) );
		// clear the new alloced block
		memset(call_table+call_table_size, 0, (max_call_table-call_table_size)*sizeof(void*));
	}

	// the new table size is delimited by the new element at the end
	if( (size_t)offset > call_table_size )
		call_table_size = offset+1;

	// put the pointer at the selected place
	call_table[offset] = var;
	return 0;
}

////// Plugins Core /////////////////////////

void dll_close(Addon *addon)
{
	if(addon)
	{
		if (addon->dll)	FreeLibrary(addon->dll);
		if (addon->filename) aFree(addon->filename);
		aFree(addon);
	}
}

Addon *dll_open (const char *filename, bool force=false)
{
	struct Addon *addon;
	struct Addon_Info *info;
	struct Addon_Event_Table *events;
	void **procs;
	int init_flag = 1;

	//ShowMessage ("loading %s\n", filename);
	
	// Check if the plugin has been loaded before
	addon = addon_head;
	while (addon) {
		// returns handle to the already loaded plugin
		if (addon->state && strcasecmp(addon->filename, filename) == 0) {
			//ShowMessage ("not loaded (duplicate) : %s\n", filename);
			return addon;
		}
		addon = addon->next;
	}

	addon = (struct Addon*)aMallocA(sizeof(struct Addon));
	addon->state = -1;	// not loaded

	addon->dll = LoadLibrary(filename);
	if (!addon->dll) {
		//ShowMessage ("not loaded (invalid file) : %s\n", filename);
		dll_close(addon);
		return NULL;
	}
	
	// Retrieve plugin information
	addon->state = 0;	// initialising
	info = (struct Addon_Info *)GetProcAddress(addon->dll, "addon_info");
	// For high priority plugins (those that are explicitly loaded from the conf file)
	// we'll ignore them even (could be a 3rd party dll file)
	if( (!info && !force) ||
		// plugin is based on older code
		(info && ((atof(info->req_version) < atof(DLL_VERSION)) ||	
		// plugin is not for this server 
		!(getServerType() & info->type))) )
	{
		//ShowMessage ("not loaded (incompatible) : %s\n", filename);
		dll_close(addon);
		return NULL;
	}
	addon->info = (info) ? info : &default_info;

	addon->filename = (char *) aMalloc (strlen(filename) + 1);
	strcpy(addon->filename, filename);
	
	// Initialise plugin call table (For exporting procedures)
	procs = (void**)GetProcAddress(addon->dll, "addon_call_table");
	if(procs) *procs = call_table;
	
	// Register plugin events
	events = (struct Addon_Event_Table*)GetProcAddress(addon->dll, "addon_event_table");
	if (events) {
		int i = 0;
		while (events[i].func_name) {
			if (strcasecmp(events[i].event_name, "DLL_Test") == 0) {
				int (*test_func)(void);
				test_func = (int (*)(void))GetProcAddress(addon->dll, events[i].func_name);
				if (test_func && test_func() == 0) {
					// plugin has failed test, disabling
					//ShowMessage ("disabled (failed test) : %s\n", filename);
					init_flag = 0;
				}
			} else {
				void (*func)(void);
				func = (void (*)(void))GetProcAddress(addon->dll, events[i].func_name);
				if (func) register_addon_event (func, events[i].event_name);
			}
			i++;
		}
	}

	addon->next = addon_head;
	addon_head = addon;

	addon->state = init_flag;	// fully loaded
	ShowStatus ("Done loading plugin '"CL_WHITE"%s"CL_RESET"'\n", (info) ? addon->info->name : filename);

	return addon;
}


void dll_load (const char *filename)
{
	dll_open(filename);
}


// Find a previously loaded plugin
Addon *dll_findloaded (const char *name)
{
	char path[256];
	Addon *addon = addon_head;
	sprintf (path, "addons%c%s%s", PATHSEP,name, DLL_EXT);

	while (addon) {
		if (addon->state && strcasecmp(addon->filename, path) == 0)
			return addon;
		addon = addon->next;
	}

	return NULL;
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

	fp = savefopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while (fgets(line, 1020, fp)) {
		if( !skip_empty_line(line) )
			continue;
		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcasecmp(w1, "auto_search") == 0) {
			if(strcasecmp(w2, "yes")==0)
				auto_search = 1;
			else if(strcasecmp(w2, "no")==0)
				auto_search = 0;
			else auto_search = atoi(w2);
		} else if (strcasecmp(w1, "addon") == 0) {
			char filename[128];
			sprintf (filename, "addons%c%s%s", PATHSEP, w2, DLL_EXT);
			dll_open(filename, true);
		} else if (strcasecmp(w1, "import") == 0)
			dll_config_read(w2);
	}
	fclose(fp);
	return 0;
}

void dll_init (void)
{
	static unsigned char st = getServerType();
	static char *argp="";
	char *DLL_CONF_FILENAME = "conf/addon_athena.conf";
	register_addon_func("DLL_Init");
	register_addon_func("DLL_Final");
	register_addon_func("Athena_Init");
	register_addon_func("Athena_Final");

	// on first access the symbol array is alloced with correct size
	// and the following access can just put in the values

	export_symbol((void*)delete_timer, 7);
	export_symbol((void*)add_timer_func_list, 6);
	export_symbol((void*)add_timer_interval, 5);
	export_symbol((void*)add_timer, 4);
	export_symbol((void*)get_svn_revision, 3);
	export_symbol((void*)gettick, 2);

	export_symbol(argp, 1);
	export_symbol(&st, 0);
	dll_config_read (DLL_CONF_FILENAME);

	if (auto_search)
		findfile("addons", DLL_EXT, dll_load);

	addon_event_trigger("DLL_Init");

	return;
}

void dll_final (void)
{
	Addon *addon = addon_head, *addon2;
	Addon_Event_List *evl = event_head, *evl2;
	Addon_Event *ev, *ev2;

	addon_event_trigger("DLL_Final");

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
	while (addon) {
		addon2 = addon->next;
		dll_close(addon);
		addon = addon2;
	}

	aFree(call_table);

	return;
}
