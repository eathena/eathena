// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "baselib.h"
#include "baseparam.h"
#include "core.h"
#include "utils.h"
#include "version.h"
#include "timer.h"
#include "showmsg.h"
#include "malloc.h"

#include "plugin.h"
#include "plugins.h"

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



#else

#  include <dlfcn.h>

// implement a windows like loading environment
#ifdef CYGWIN
	#define DLL_EXT		".dll"
#else
	#define DLL_EXT		".so"
#endif

typedef void* DLL;


#endif

//////////////////////////////////////////////////////////////////////////
// storage for loading dlls

typedef struct _Plugin
{
	DLL dll;
	char state;
	basics::string<> filename;
	Plugin_Info *info;
	struct _Plugin *next;

	_Plugin(const basics::string<>fn, const DLL& d) :
		dll(d),
		state(0),
		filename(fn),
		info(NULL),
		next(NULL)
	{}

	~_Plugin()
	{
		this->close();
	}
	void close()
	{
		this->filename.clear();
		if(this->dll)
			FreeLibrary(this->dll);
	}

} Plugin;

typedef struct _Plugin_Event
{
	void (*func)();
	struct _Plugin_Event *next;

	_Plugin_Event() :
		func(NULL),
		next(NULL)
	{}
} Plugin_Event;

typedef struct _Plugin_Event_List
{
	char *name;
	struct _Plugin_Event_List *next;
	struct _Plugin_Event *events;

	_Plugin_Event_List() : 
		name(NULL),
		next(NULL),
		events(NULL)
	{}
} Plugin_Event_List;

int auto_search	= 1;

Plugin_Event_List *event_head = NULL;

Plugin *plugin_head = NULL;

void** plugin_call_table=NULL;
size_t call_table_size	= 0;
size_t call_table_max	= 0;

Plugin_Info default_info = { "Unknown", PLUGIN_ALL, "0", PLUGIN_VERSION, "Unknown" };


////// Plugin Events Functions //////////////////

int register_plugin_func (char *name)
{
	Plugin_Event_List *evl;
	if (name)
	{
		evl = new Plugin_Event_List;
		evl->next = event_head;
		evl->name = new char[1+strlen(name)];
		strcpy(evl->name, name);
		evl->events = NULL;
		event_head = evl;
	}
	return 0;
}

Plugin_Event_List *search_plugin_func (char *name)
{
	Plugin_Event_List *evl = event_head;
	while (evl)
	{
		if (strcasecmp(evl->name, name) == 0)
			return evl;
		evl = evl->next;
	}
	return NULL;
}

int register_plugin_event (void (*func)(), char* name)
{
	Plugin_Event_List *evl = search_plugin_func(name);
	if(!evl)
	{
		// register event if it doesn't exist already
		register_plugin_func(name);
		// relocate the new event list
		evl = search_plugin_func(name);
	}
	if(evl)
	{
		Plugin_Event *ev = new Plugin_Event;
		ev->func = func;
		ev->next = NULL;

		if(evl->events == NULL)
			evl->events = ev;
		else {
			Plugin_Event *ev2 = evl->events;
			while(ev2)
			{
				if(ev2->next == NULL)
				{
					ev2->next = ev;
					break;
				}
				ev2 = ev2->next;
			}
		}
	}
	return 0;
}

int plugin_event_trigger (char *name)
{
	int c = 0;
	Plugin_Event_List *evl = search_plugin_func(name);
	if (evl) {
		Plugin_Event *ev = evl->events;
		while (ev)
		{
			ev->func();
			ev = ev->next;
			++c;
		}
	}
	return c;
}

////// Plugins Call Table Functions /////////

int export_symbol (void *var, int offset)
{
	//printf ("0x%x\n", var);
	
	// add to the end of the list
	if (offset < 0)
		offset = call_table_size;
	
	// realloc if not large enough  
	if( (size_t)offset >= call_table_max )
	{
		call_table_max = new_realloc(plugin_call_table, call_table_max, (offset+1)-call_table_max);
	}

	// the new table size is delimited by the new element at the end
	if ((size_t)offset >= call_table_size)
		call_table_size = offset+1;
	
	// put the pointer at the selected place
	plugin_call_table[offset] = var;
	return 0;
}

////// Plugins Core /////////////////////////


Plugin *plugin_open (const char *filename, bool force=false)
{
	Plugin *plugin;
	Plugin_Info *info;
	Plugin_Event_Table *events;
	void **procs;
	int init_flag = 1;

	//printf ("loading %s\n", filename);
	
	// Check if the plugin has been loaded before
	plugin = plugin_head;
	while (plugin)
	{
		// returns handle to the already loaded plugin
		if (plugin->state && strcasecmp(plugin->filename, filename) == 0)
		{
			//printf ("not loaded (duplicate) : %s\n", filename);
			return plugin;
		}
		plugin = plugin->next;
	}

	DLL dll = LoadLibrary(filename);
	if( !dll )
	{
		ShowError("Plugin not loaded (invalid file) : %s\n", filename);
		return NULL;
	}
	plugin = new Plugin(filename, dll);

	// Retrieve plugin information
	info = (Plugin_Info *)GetProcAddress(plugin->dll, "plugin_info");
	// For high priority plugins (those that are explicitly loaded from the conf file)
	// we'll ignore them even (could be a 3rd party dll file)
	if( (!info && !force) ||
		// plugin is based on older code
		(info && (atof(info->req_version) < atof(PLUGIN_VERSION))) ||	
		// plugin is not for this server
		!(getServerType() & info->type) )
	{
		//printf ("not loaded (incompatible) : %s\n", filename);
		delete plugin;
		return NULL;
	}
	plugin->info = (info) ? info : &default_info;

	// Initialise plugin call table (For exporting procedures)
	procs = (void**)GetProcAddress(plugin->dll, "plugin_call_table");
	if (procs) *procs = plugin_call_table;
	
	// Register plugin events
	events = (Plugin_Event_Table*)GetProcAddress(plugin->dll, "plugin_event_table");
	if (events)
	{
		int i = 0;
		while (events[i].func_name)
		{
			if (strcasecmp(events[i].event_name, "Plugin_Test") == 0)
			{
				int (*test_func)(void);
				test_func = (int (*)(void))GetProcFunction(plugin->dll, events[i].func_name);
				if (test_func && test_func() == 0)
				{
					// plugin has failed test, disabling
					//printf ("disabled (failed test) : %s\n", filename);
					init_flag = 0;
				}
			}
			else
			{
				void (*func)(void);
				func = (void (*)(void))GetProcFunction(plugin->dll, events[i].func_name);
				if (func) register_plugin_event (func, events[i].event_name);
			}
			++i;
		}
	}

	plugin->next = plugin_head;
	plugin_head = plugin;

	plugin->state = init_flag;	// fully loaded
	ShowStatus ("Done loading plugin '"CL_WHITE"%s"CL_RESET"'\n", (info) ? plugin->info->name : filename);

	return plugin;
}

// filesearch callback
void plugin_load (const char *filename)
{
	plugin_open(filename);
}



////// Initialize/Finalize ////////////////////

int plugins_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = basics::safefopen(cfgName, "r");
	if (fp == NULL)
	{
		ShowError("Plugin Configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
		return 1;
	}
	
	while (fgets(line, sizeof(line), fp))
	{
		if( prepare_line(line) && 2==sscanf(line,"%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
		{
			basics::itrim(w1);
			if(!*w1) continue;
			
			basics::itrim(w2);
			
			if (strcasecmp(w1, "auto_search") == 0)
			{
				auto_search = basics::config_switch<bool>(w2);
			}
			else if (strcasecmp(w1, "plugin") == 0)
			{
				char filename[128];
				snprintf(filename, sizeof(filename), "plugins%c%s%s", PATHSEP, w2, DLL_EXT);
				plugin_open(filename, true);
			}
			else if (strcasecmp(w1, "import") == 0)
			{
				plugins_config_read(w2);
			}
		}
	}
	fclose(fp);
	ShowStatus("Done reading Plugin Configuration '"CL_WHITE"%s"CL_RESET"'.\n", cfgName);
	return 0;
}

void plugin_init (void)
{
	static unsigned char st = getServerType();
	static char *argp="";
	
	char *PLUGIN_CONF_FILENAME = "conf/plugin_athena.conf";
	register_plugin_func("Plugin_Init");
	register_plugin_func("Plugin_Final");
	register_plugin_func("Athena_Init");
	register_plugin_func("Athena_Final");

//	export_symbol (addr_,										12);
//	export_symbol(GetProcAddress(get_uptime),					11);
	export_symbol((void*)NULL,									11);
	export_symbol(GetProcAddress((FARPROC)delete_timer),		10);
	export_symbol(GetProcAddress((FARPROC)add_timer_func_list),	 9);
//	export_symbol(GetProcAddress((FARPROC)add_timer_interval),	 8);
//	export_symbol(GetProcAddress((FARPROC)add_timer),			 7);
	export_symbol(GetProcAddress((FARPROC)get_revision),		 6);
	export_symbol(GetProcAddress((FARPROC)gettick),				 5);
//	export_symbol(&runflag,										 4);
	export_symbol(NULL,											 4);
//	export_symbol(arg_v,										 3);
	export_symbol(NULL,											 3);
//	export_symbol(&arg_c,										 2);
	export_symbol(NULL,											 2);
	export_symbol(argp,											 1);
	export_symbol(&st,											 0);

	plugins_config_read (PLUGIN_CONF_FILENAME);


	if (auto_search)
		basics::findFiles("plugins", "*"DLL_EXT, plugin_load);

	plugin_event_trigger("Plugin_Init");

	return;
}

void plugin_final (void)
{
	Plugin *plugin, *plugin2;
	Plugin_Event_List *evl, *evl2;
	Plugin_Event *ev, *ev2;

	plugin_event_trigger("Plugin_Final");

	evl = event_head;
	while (evl)
	{
		ev = evl->events;
		while (ev)
		{
			ev2 = ev->next;
			delete ev;
			ev = ev2;
		}
		evl2 = evl->next;
		delete[] evl->name;
		delete evl;
		evl = evl2;
	}
	event_head = NULL;

	plugin = plugin_head;
	while (plugin)
	{
		plugin2 = plugin->next;
		delete plugin;
		plugin = plugin2;
	}
	plugin_head = NULL;

	delete[] plugin_call_table;
	plugin_call_table = NULL;

	return;
}
