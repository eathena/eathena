
#ifndef	_DLL_H_
#define _DLL_H_

#define DLL_VERSION "1.00"


int addon_event_trigger (char *);

void dll_load (const char *);
void dll_init (void);
void dll_final (void);

#endif	// _DLL_H_
