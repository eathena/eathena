// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_PLUGIN_H_
#define _PLUGIN_H_


int plugin_event_trigger (char *);

void plugin_load (const char *);

void plugin_init (void);
void plugin_final (void);


#endif	// _PLUGINS_H_
