// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAPCACHE_H_
#define _MAPCACHE_H_


extern char afm_dir[1024];

/*==========================================
 * êÖèÍçÇÇ≥ê›íË
 *------------------------------------------
 */
#define NO_WATER 1000000

class CWaterlist
{
	basics::smap<basics::string<>,int> cMap;
public:
	CWaterlist()
	{}
	CWaterlist(const char *watertxt)
	{
		this->open(watertxt);
	}
	~CWaterlist()
	{}
	bool open(const char *watertxt)
	{
		char line[1024],w1[1024];
		FILE *fp=NULL;

		fp=basics::safefopen(watertxt,"r");
		if(fp==NULL)
		{
			ShowError("waterheight file not found: %s\n",watertxt);
			return false;
		}
		while( fgets(line,sizeof(line),fp) )
		{
			int wh, count;
			if( !is_valid_line(line) )
				continue;
			if((count=sscanf(line,"%1024s %d",w1,&wh)) < 1)
				continue;
			basics::itrim(w1);
			if(!*w1)
				continue;
			buffer2mapname(line, sizeof(line), w1);
			line[sizeof(line)-1]=0;
			this->cMap[line] = (count >= 2)?wh:3;
		}
		fclose(fp);
		return true;
	}
	int operator[](const char *mapname) const
	{
		const int* ptr = this->cMap.search(mapname);
		return ptr?*ptr:NO_WATER;
	}

	// map_readwaterheight
	// Reads from the .rsw for each map
	// Returns water height (or NO_WATER if file doesn't exist)or other error is encountered.
	// This receives a map-name, and changes the extension to rsw if it isn't set already.
	// assumed path for file is data/mapname.rsw
	// Credits to LittleWolf
	static int map_waterheight_from_grf(const char *mapname)
	{
		if(mapname)
		{
			char buf[512];
			char* ip;

			// have windows backslash as path seperator here
			snprintf(buf, sizeof(buf), "data\\%s", mapname);
			ip = strrchr(buf,'.');
			if(ip) *ip=0;
			// append ".rsw" for reading in rsw's
			strcat(buf, ".rsw");

			//Load water height from file
			uchar *dat = grfio_read(buf);
			if(dat)
			{	
				const uchar *p = dat+166;
				float whtemp;
				_F_frombuffer(whtemp, p);
				delete[] dat;
				return (int)whtemp;
			}
		}
		return NO_WATER;
	}

};




bool map_cache_open(const char *fn);
bool map_cache_close(void);
bool map_cache_read(struct map_data &m);
bool map_cache_write(struct map_data &m);
bool map_readafm(struct map_data& block_list, const char *fn=NULL);
bool map_readaf2(struct map_data& block_list, const char*fn=NULL);
bool map_readgrf(struct map_data& block_list, const char *fn=NULL);

#endif//_MAPCACHE_H_
