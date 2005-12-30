#include "mmo.h"
#include <string.h>
#include <stdio.h>
#include <showmsg.h>

#define MAX_MAPINDEX 2000

//Leave an extra char of space to hold the terminator, in case for the strncpy(mapindex_id2name()) calls.
char indexes[MAX_MAPINDEX][MAP_NAME_LENGTH+1];
static unsigned short max_index = 0;

char mapindex_cfgfile[80] = "db/map_index.txt";

unsigned short mapindex_name2id(char* name) {
	//TODO: Perhaps use a db to speed this up? [Skotlex]
	int i;
	for (i = 1; i < max_index; i++)
	{
		if (indexes[i][0] && strncmp(indexes[i],name,MAP_NAME_LENGTH)==0)
			return i;
	}
#ifdef MAPINDEX_AUTOADD
	if (i < MAX_MAPINDEX) {
		strncpy(indexes[i], name, MAP_NAME_LENGTH);
		ShowDebug("mapindex_name2id: Added map \"%s\" to position %d\n", indexes[i], i);
		return i;
	}
#endif
	ShowDebug("mapindex_name2id: Map \"%s\" not found in index list!\n", name);
	return 0;
}

char* mapindex_id2name(unsigned short id) {
	if (id > MAX_MAPINDEX || !indexes[id][0]) {
		ShowDebug("mapindex_id2name: Requested name for non-existant map index [%d] in cache.\n", id);
		return indexes[0]; //Theorically this should never happen, hence we return this string to prevent null pointer crashes.
	}
	return indexes[id];
}

void mapindex_init() {
	FILE *fp;
	char line[1024];
	int last_index = -1;
	int index;
	char map_name[1024];
	
	memset (&indexes, 0, sizeof (indexes));
	fp=fopen(mapindex_cfgfile,"r");
	if(fp==NULL){
		ShowError("Unable to read mapindex config file %s!\n", mapindex_cfgfile);
		return;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		switch (sscanf(line,"%s\t%d",map_name,&index)) {
			case 1: //Map with no ID given, auto-assign
				index = last_index+1;
			case 2: //Map with ID given
				if (index < 0 || index >= MAX_MAPINDEX) {
					ShowError("(mapindex_init) Map index (%d) for \"%s\" out of range (max is %d)\n", index, map_name, MAX_MAPINDEX);
					continue;
				}
				if (indexes[index][0])
					ShowWarning("(mapindex_init) Overriding index %d: map \"%s\" -> \"%s\"\n", indexes[index], map_name);
				strncpy(indexes[index], map_name, MAP_NAME_LENGTH);
				if (max_index < index)
					max_index = index;
				break;
			default:
				continue;
		}
		last_index = index;
	}
}

void mapindex_final() {
}

