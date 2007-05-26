// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAPCACHE_H_
#define _MAPCACHE_H_


extern char afm_dir[1024];


bool map_waterlist_open(const char *fn);
bool map_cache_open(const char *fn);
bool map_cache_close(void);
bool map_cache_read(struct map_intern &mm);
bool map_cache_write(struct map_intern &mm);
bool map_readafm(struct map_intern& mm, const char *fn=NULL);
bool map_readaf2(struct map_intern& mm, const char *fn=NULL);
bool map_readgrf(struct map_intern& mm, const char *fn=NULL);

#endif//_MAPCACHE_H_
