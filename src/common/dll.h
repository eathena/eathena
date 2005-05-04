
#ifndef	_DLL_H_
#define _DLL_H_

#define DLL_VERSION 1.00

#ifdef _WIN32

	#include <windows.h>
	#define DLL_OPEN(x)		LoadLibrary(x)
	#define DLL_SYM(x,y,z)	(FARPROC)(x) = GetProcAddress(y,z)
	#define DLL_CLOSE(x)	FreeLibrary(x)
	#define DLL_EXT			".dll"
	#define DLL				HINSTANCE
	char *DLL_ERROR(void);

#else

	#include <dlfcn.h>
	#define DLL_OPEN(x)		dlopen(x,RTLD_NOW)
	#define DLL_SYM(x,y,z)	(x) = (void *)dlsym(y,z)
	#define DLL_CLOSE(x)	dlclose(x)
	#define DLL_ERROR		dlerror

	#ifdef CYGWIN
		#define DLL_EXT		".dll"
	#else
		#define DLL_EXT		".so"
	#endif
	#define DLL				void *

#endif

struct Addon {
	DLL dll;
	char state;
	char *filename;
	struct Addon_Info *info;
	struct Addon *next;	
};

struct Addon_Info {
	char *name;
	char type;
	char *version;
	char *req_version;
	char *description;
};

struct Addon_Event_Table {
	char *func_name;
	char *event_name;
};

int register_addon_func (char *);
int register_addon_event (void (*)(), char *);
int addon_event_trigger (char *);

void dll_open (const char *);
void dll_init (void);
void dll_final (void);

#endif	// _DLL_H_
