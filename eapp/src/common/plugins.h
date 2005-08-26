#ifndef	_PLUGIN_H_
#define _PLUGIN_H_


int plugin_event_trigger (char *);

void plugin_load (const char *);

void plugin_init (void);
void plugin_final (void);


#endif	// _PLUGINS_H_
