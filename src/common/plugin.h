// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef	_PLUGIN_H_
#define _PLUGIN_H_

////// Plugin functions ///////////////

#define PLUGIN_VERSION "1.02"


//////////////////////////////////////////////////////////////////////////
// data definitions

typedef struct _Plugin_Info {
	char *name;
	unsigned char type;
	char *version;
	char *req_version;
	char *description;
} Plugin_Info;

typedef struct _Plugin_Event_Table {
	char *func_name;
	char *event_name;
} Plugin_Event_Table;

//////////////////////////////////////////////////////////////////////////
////// Plugin Export functions /////////////

// same layout than ServerType from version.h
typedef enum 
{
	PLUGIN_NONE	=	0x00,
	PLUGIN_LOGIN=	0x01,
	PLUGIN_CHAR	=	0x02,
	PLUGIN_INTER=	0x04,
	PLUGIN_MAP	=	0x08,
	PLUGIN_CORE	=	0x10,
	PLUGIN_xxx1	=	0x20,
	PLUGIN_xxx2	=	0x40,
	PLUGIN_xxx3	=	0x80,
	PLUGIN_ALL	=	0xFF
} AddonType;


#endif	// _PLUGIN_H_
