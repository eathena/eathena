// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cbasetypes.h"
#include "db.h"
#include "malloc.h"
#include "mapindex.h"
#include "mmo.h"
#include "showmsg.h"
#include "strlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DBMap* mapindex_db = NULL;  // char* -> unsigned short
static char* mapindex_indexes[MAX_MAPINDEX] = { 0 };  // unsigned short -> char*
static unsigned short max_index = 0;

char mapindex_cfgfile[80] = "db/map_index.txt";

/// Validates a map index.
static inline bool mapindex_exists(unsigned short index)
{
	return index && index<MAX_MAPINDEX && mapindex_indexes[index]!=NULL;
}

/// Retrieves the map name from 'string' (removing .gat extension if present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname(const char* string, char* output)
{
	static char buf[MAP_NAME_LENGTH];
	char* dest = (output != NULL) ? output : buf;

	const char* match;
	size_t len = strlen(string);

	if( ( match = strrchr(string, '.') ) != NULL && stricmp(match+1, "gat") == 0 )
	{// has an extension
		if( len >= MAP_NAME_LENGTH_EXT )
		{
			ShowWarning("mapindex_getmapname: Map name '%s' is too long (len=%u, max=%u)!\n", string, len, MAP_NAME_LENGTH_EXT-1);
			len = MAP_NAME_LENGTH_EXT-1;  // cripple the name so it fits the buffer
		}

		len -= 4;  // strip .gat extension
	}
	else
	{// already without extension
		if( len >= MAP_NAME_LENGTH )
		{
			ShowWarning("mapindex_getmapname: Map name '%s' is too long (len=%u, max=%u)!\n", string, len, MAP_NAME_LENGTH-1);
			len = MAP_NAME_LENGTH-1;  // cripple the name so it fits the buffer
		}
	}

	safestrncpy(dest, string, len+1);

	// wipe the remaining buffer to prevent stale data from leaking
	memset(&dest[len], 0, MAP_NAME_LENGTH-len);

	return dest;
}

/// Retrieves the map name from 'string' (adding .gat extension if not already present).
/// Result gets placed either into 'buf' or in a static local buffer.
const char* mapindex_getmapname_ext(const char* string, char* output)
{
	static char buf[MAP_NAME_LENGTH_EXT];
	char* dest = ( output != NULL ) ? output : buf;

	const char* match;
	size_t len;

	if( ( match = strchr(string, '#' ) ) != NULL )
	{// skip the instance part of the name
		string = match+1;
	}

	len = strlen(string);

	if( ( match = strrchr(string, '.') ) != NULL && stricmp(match+1, "gat") == 0 )
	{// already has an extension
		if( len >= MAP_NAME_LENGTH_EXT )
		{
			ShowWarning("mapindex_getmapname_ext: Map name '%s' is too long (len=%u, max=%u)!\n", string, len, MAP_NAME_LENGTH_EXT-1);
			len = MAP_NAME_LENGTH_EXT-1;  // cripple the name so it fits the buffer
		}

		safestrncpy(dest, string, len+1);
	}
	else
	{// without extension
		if( len >= MAP_NAME_LENGTH )
		{
			ShowWarning("mapindex_getmapname_ext: Map name '%s' is too long (len=%u, max=%u)!\n", string, len, MAP_NAME_LENGTH-1);
			len = MAP_NAME_LENGTH-1;  // cripple the name so it fits the buffer
		}

		safestrncpy(dest, string, len+1);
		strcpy(&dest[len], ".gat");  // add .gat extension
		len+= 4;
	}

	// wipe the remaining buffer to prevent stale data from leaking
	memset(&dest[len], 0, MAP_NAME_LENGTH_EXT-len);

	return dest;
}

/// Adds a map to the specified index.
/// If 0 is provided as index, temporary map index is created.
/// Returns map index if successful, 0 oherwise.
unsigned short mapindex_addmap(unsigned short index, const char* name)
{
	char map_name[MAP_NAME_LENGTH];

	mapindex_getmapname(name, map_name);

	if( !map_name[0] )
	{
		ShowError("mapindex_addmap: Attempted to add a map without a name (index=%u).\n", index);
		return 0;
	}

	if( !index )
	{// request for temporary map index
		if( !max_index )
		{
			ShowError("mapindex_addmap: Request for temporary map index for '%s' with no maps loaded!\n", map_name);
			return 0;
		}

		for( index = max_index+1; index < MAX_MAPINDEX; index++ )
		{
			if( mapindex_indexes[index] == NULL )
			{
				break;
			}
		}
		if( index == MAX_MAPINDEX )
		{
			ShowError("mapindex_addmap: Out of temporary map indexes, increase MAX_MAPINDEX (current=%u, max=%u).\n", MAX_MAPINDEX, ~((unsigned short)0));
			return 0;
		}
	}
	else if( index > MAX_MAPINDEX )
	{
		ShowError("mapindex_addmap: Map index for '%s' is out of range (index=%u, max=%u).\n", map_name, index, MAX_MAPINDEX);
		return 0;
	}
	else if( index > max_index )
	{// normal map index max
		max_index = index;
	}

	if( mapindex_exists(index) )
	{
		ShowWarning("mapindex_addmap: Overriding existing index [%u]: '%s' -> '%s'\n", index, mapindex_indexes[index], map_name);
	}

	mapindex_indexes[index] = aStrdup(map_name);
	strdb_put(mapindex_db, mapindex_indexes[index], (void*)index);

	return index;
}

unsigned short mapindex_name2id(const char* name)
{
	char map_name[MAP_NAME_LENGTH];
	unsigned short index;

	mapindex_getmapname(name, map_name);

	if( ( index = (unsigned short)strdb_get(mapindex_db, map_name) ) == 0 )
	{
		ShowWarning("mapindex_name2id: Requested id for non-existant map name '%s'.\n", map_name);
		return 0;
	}

	return index;
}

const char* mapindex_id2name(unsigned short index)
{
	if( !mapindex_exists(index) )
	{
		ShowWarning("mapindex_id2name: Requested name for non-existant map index [%u].\n", index);
		return ""; // dummy empty string so that the callee doesn't crash
	}

	return mapindex_indexes[index];
}

void mapindex_init(void)
{
	char line[1024], map_name[1024];
	unsigned int count = 0, lines = 0;
	unsigned short index, last_index = 0;
	FILE* fp;

	mapindex_db = stridb_alloc(DB_OPT_RELEASE_KEY, MAP_NAME_LENGTH);

	if( ( fp = fopen(mapindex_cfgfile, "r") ) == NULL)
	{
		ShowFatalError("Failed to read '"CL_WHITE"%s"CL_RESET"'.\n", mapindex_cfgfile);
		exit(EXIT_FAILURE); //Server can't really run without this file.
	}

	while( fgets(line, sizeof(line), fp) )
	{
		lines++;

		if( !line[0] || ( line[0] == '/' && line[1] == '/' ) || line[0] == '\n' || line[0] == '\r' )
		{// eof, comment, empty line
			continue;
		}

		switch( sscanf(line, "%1023s\t%hu", map_name, &index) )
		{
			case 1: //Map with no ID given, auto-assign
				index = last_index+1;
				// fall through
			case 2: //Map with ID given
				mapindex_addmap(index, map_name);
				break;
			default:
				ShowWarning("mapindex_init: Not recognized data on line '"CL_WHITE"%u"CL_RESET"' in '"CL_WHITE"%s"CL_RESET"'.\n", lines, mapindex_cfgfile);
				continue;
		}

		last_index = index;

		count++;
	}

	fclose(fp);

	ShowStatus("Done reading '"CL_WHITE"%lu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, mapindex_cfgfile);
}

void mapindex_removemap(unsigned short index)
{
	if( mapindex_exists(index) )
	{
		strdb_remove(mapindex_db, mapindex_indexes[index]);
		mapindex_indexes[index] = NULL;
	}
}

void mapindex_final(void)
{
	mapindex_db->destroy(mapindex_db, NULL);
	mapindex_db = NULL;
	memset(mapindex_indexes, 0, sizeof(mapindex_indexes));  // wipe stale pointers
}
