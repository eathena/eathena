
#include "basefile.h"
#include "grfio.h"
#include "mmo.h"
#include "showmsg.h"
#include "utils.h"
#include "mapcache.h"
#include "map.h"




///////////////////////////////////////////////////////////////////////////////
// 初期化周り
char afm_dir[1024] = "";









/*==========================================
 * 水場高さ設定
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

CWaterlist waterlist;
bool map_waterlist_open(const char *fn)
{
	return waterlist.open(fn);
}










/*==========================================
* マップキャッシュに追加する
*===========================================*/
///////////////////////////////////////////////////////////////////////////////
// マップキャッシュの最大値
#define MAX_MAP_CACHE 768

//各マップごとの最小限情報を入れるもの、READ_FROM_BITMAP用
struct map_cache_info
{
	char fn[24];			//ファイル名
	unsigned short xs;
	unsigned short ys;		//幅と高さ
	int water_height;
	size_t pos;				// データが入れてある場所
	int compressed;     // zilb通せるようにする為の予約
	size_t datalen;
}; // 40 byte

struct map_cache_head {
	size_t sizeof_header;
	size_t sizeof_map;
	// 上の２つ改変不可
	size_t nmaps;			// マップの個数
	long filesize;
};

struct map_cache
{
	struct map_cache_head head;
	struct map_cache_info *map;
	FILE *fp;
	int dirty;
};

struct map_cache map_cache;



bool map_cache_open(const char *fn)
{
	if(map_cache.fp)
		map_cache_close();

	map_cache.fp = basics::safefopen(fn,"r+b");
	if(map_cache.fp)
	{
		fread(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fseek(map_cache.fp,0,SEEK_END);
		if(
			map_cache.head.sizeof_header == sizeof(struct map_cache_head) &&
			map_cache.head.sizeof_map    == sizeof(struct map_cache_info) &&
			map_cache.head.filesize      == ftell(map_cache.fp) )
		{
			// キャッシュ読み甲ﾝ成功
			map_cache.map = new struct map_cache_info[map_cache.head.nmaps];
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map, sizeof(struct map_cache_info), map_cache.head.nmaps, map_cache.fp);
			return true;
		}
		fclose(map_cache.fp);
	}
	// 読み甲ﾝに失敗したので新規に作成する
	map_cache.fp = basics::safefopen(fn,"wb");
	if(map_cache.fp)
	{
		memset(&map_cache.head,0,sizeof(struct map_cache_head));
		map_cache.map   = new struct map_cache_info[MAX_MAP_CACHE];
		memset(map_cache.map,0, MAX_MAP_CACHE*sizeof(struct map_cache_info));
		map_cache.head.nmaps         = MAX_MAP_CACHE;
		map_cache.head.sizeof_header = sizeof(struct map_cache_head);
		map_cache.head.sizeof_map    = sizeof(struct map_cache_info);

		map_cache.head.filesize  = sizeof(struct map_cache_head);
		map_cache.head.filesize += sizeof(struct map_cache_info) * map_cache.head.nmaps;

		map_cache.dirty = 1;
		return true;
	}
	return false;
}

bool map_cache_close(void)
{
	if(!map_cache.fp)
		return false;
	if(map_cache.dirty)
	{
		fseek(map_cache.fp,0,SEEK_SET);
		fwrite(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fwrite(map_cache.map,map_cache.head.nmaps,sizeof(struct map_cache_info),map_cache.fp);
	}
	fclose(map_cache.fp);
	map_cache.fp = NULL;
	delete[] map_cache.map;
	map_cache.map=NULL;
	return true;
}

bool map_cache_read(struct map_intern &mm)
{
	size_t i;
	if(!map_cache.fp)
		return 0;
	for(i=0; i<map_cache.head.nmaps; ++i)
	{
		if( map_cache.map[i].water_height == waterlist[mm.mapname] &&	// 水場の高さが違うので読み直し
			0==strcmp(mm.mapname,map_cache.map[i].fn) )
			break;
	}
	if( i < map_cache.head.nmaps )	
	{
		if(map_cache.map[i].compressed == 0)
		{
				// 非圧縮ファイル
			const size_t dest_sz = map_cache.map[i].xs*map_cache.map[i].ys;
			mm.xs = map_cache.map[i].xs;
			mm.ys = map_cache.map[i].ys;
			mm.gat = new mapcell_t[dest_sz];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if( map_cache.map[i].datalen > dest_sz*sizeof(mapcell_t) )
				map_cache.map[i].datalen = dest_sz*sizeof(mapcell_t);
			if( map_cache.map[i].datalen != fread(mm.gat, 1, map_cache.map[i].datalen, map_cache.fp) )
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] mm.gat;
				mm.gat = NULL;
				mm.xs = 0;
				mm.ys = 0;
				return false;
			}
		}
		else if(map_cache.map[i].compressed == 1)
		{	//zlib compressed
				// 圧縮フラグ=1 : zlib
			unsigned char *buf;
			unsigned long size_compress = map_cache.map[i].datalen;
			unsigned long dest_len = map_cache.map[i].xs * map_cache.map[i].ys * sizeof(mapcell_t);
			mm.xs = map_cache.map[i].xs;
			mm.ys = map_cache.map[i].ys;
			buf = new unsigned char[size_compress];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if(fread(buf,1,size_compress,map_cache.fp) != size_compress)
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] buf;
				buf = NULL;
				mm.gat = NULL;
				mm.xs = 0; 
				mm.ys = 0; 
				return false;
			}
			mm.gat = new mapcell_t[map_cache.map[i].xs * map_cache.map[i].ys];
			decode_zip((unsigned char*)mm.gat, dest_len, buf, size_compress);
			if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys * sizeof(mapcell_t))
			{	// 正常に解凍が出来てない
				delete[] buf;
				buf=NULL;
				delete[] mm.gat;
				mm.gat = NULL;
				mm.xs = 0; 
				mm.ys = 0; 
				return false;
			}
			if(buf)
			{	// might be ok without this check
				delete[] buf;
				buf=NULL;
			}
		}
		// might add other compressions here

		// clear dynamic field entries
		const size_t sz = mm.xs*mm.ys;
		bool warn=true;
		for(i=0; i<sz; ++i)
		{
			/////////////////////////////////////////////////////////
			//## remove at the latest by 12/2006
			if( warn && config.etc_log &&
				(mm.gat[i].npc || mm.gat[i].basilica || mm.gat[i].moonlit || mm.gat[i].regen) )
			{
				ShowWarning("Map Cache corrupted, please rebuild it\n");
				warn = false;
			}
			/////////////////////////////////////////////////////////

			mm.gat[i].npc = 0;
			mm.gat[i].basilica = 0;
			mm.gat[i].moonlit = 0;
			mm.gat[i].regen = 0;
		}
		return true;
	}
	return false;
}

bool map_cache_write(struct map_intern &mm)
{
	size_t i;
	unsigned long len_new, len_old;
	unsigned char *write_buf;

	if(!map_cache.fp)
		return false;

	for(i = 0;i < map_cache.head.nmaps ; ++i) 
	{
		if( (0==strcmp(mm.mapname,map_cache.map[i].fn)) || (map_cache.map[i].fn[0] == 0) )
			break;
	}
	if(i<map_cache.head.nmaps) 
	{	// should always be valid but better check it
		int compress = 0;
		if(map_read_flag == READ_FROM_BITMAP_COMPRESSED)
			compress = 1;	// zlib compress
							// might add other compressions here

		// prepare write_buf and len_new
		if(compress == 1)
		{	// zlib compress
			// 圧縮保存
			// さすがに２倍に膨れる事はないという事で
			len_new = 2 * mm.xs * mm.ys * sizeof(mapcell_t);
			write_buf = new unsigned char[len_new];
			encode_zip(write_buf,len_new,(unsigned char *)mm.gat, mm.xs*mm.ys*sizeof(mapcell_t));
		}
		else
		{	// no compress
			len_new = mm.xs * mm.ys *sizeof(mapcell_t);
			write_buf = (unsigned char*)mm.gat;
		}
		
		// now insert it
		if( (map_cache.map[i].fn[0] == 0) )
		{	// new map is inserted
			// write at the end of the mapcache file
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

			// prepare the data header
			safestrcpy(map_cache.map[i].fn, sizeof(map_cache.map[i].fn), mm.mapname);

			// update file header
			map_cache.map[i].pos = map_cache.head.filesize;
			map_cache.head.filesize += len_new;
		}
		else
		{	// update an existing map
			len_old = map_cache.map[i].datalen;

			if(len_new <= len_old) // size is ok
			{	// サイズが同じか小さくなったので場所は変わらない
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
	
			}
			else 
			{	// new len is larger then the old space -> write at file end
				// 新しい場所に登録
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

					// update file header
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
		}
		// just make sure that everything gets updated
		map_cache.map[i].xs  = mm.xs;
		map_cache.map[i].ys  = mm.ys;
		map_cache.map[i].water_height = waterlist[mm.mapname];
		map_cache.map[i].compressed   = compress;
		map_cache.map[i].datalen      = len_new;
		map_cache.dirty = 1;

		if(compress == 1)
		{	// zlib compress has alloced an additional buffer
			delete[] write_buf;
			write_buf = NULL;
		}
		return true;
	}
	// 書き甲ﾟなかった
	return false;
}

bool map_readafm(struct map_intern& mm, const char *fn)
{
	/*
	Advanced Fusion Maps Support
	(c) 2003-2004, The Fusion Project
	- AlexKreuz

	The following code has been provided by me for eAthena
	under the GNU GPL.  It provides Advanced Fusion
	Map, the map format desgined by me for Fusion, support
	for the eAthena emulator.

	I understand that because it is under the GPL
	that other emulators may very well use this code in their
	GNU project as well.

	The AFM map format was not originally a part of the GNU
	GPL. It originated from scratch by my own hand.  I understand
	that distributing this code to read the AFM maps with eAthena
	causes the GPL to apply to this code.  But the actual AFM
	maps are STILL copyrighted to the Fusion Project.  By choosing

	In exchange for that 'act of faith' I ask for the following.

	A) Give credit where it is due.  If you use this code, do not
	   place your name on the changelog.  Credit should be given
	   to AlexKreuz.
	B) As an act of courtesy, ask me and let me know that you are putting
	   AFM support in your project.  You will have my blessings if you do.
	C) Use the code in its entirety INCLUDING the copyright message.
	   Although the code provided may now be GPL, the AFM maps are not
	   and so I ask you to display the copyright message on the STARTUP
	   SCREEN as I have done here. (refer to core.c)
	   "Advanced Fusion Maps (c) 2003-2004 The Fusion Project"

	Without this copyright, you are NOT entitled to bundle or distribute
	the AFM maps at all.  On top of that, your "support" for AFM maps
	becomes just as shady as your "support" for Gravity GRF files.

	The bottom line is this.  I know that there are those of you who
	would like to use this code but aren't going to want to provide the
	proper credit.  I know this because I speak frome experience.  If
	you are one of those people who is going to try to get around my
	requests, then save your breath because I don't want to hear it.

	I have zero faith in GPL and I know and accept that if you choose to
	not display the copyright for the AFMs then there is absolutely nothing
	I can do about it.  I am not about to start a legal battle over something
	this silly.

	Provide the proper credit because you believe in the GPL.  If you choose
	not to and would rather argue about it, consider the GPL failed.

	October 18th, 2004
	- AlexKreuz
	- The Fusion Project
	*/

	unsigned short x,y,xs,ys;
	char afm_line[65535];
	int afm_size[2];
	FILE *afm_file;
	char *str;
	char buf[512];

	if(!fn)
	{
		char *ip;
		if(*afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, mm.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", mm.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".afm");
	}
	else
		safestrcpy(buf, sizeof(buf), fn);

	afm_file = basics::safefopen(buf, "r");
	if (afm_file != NULL)
	{
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		str=fgets(afm_line, sizeof(afm_line), afm_file);
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		xs = mm.xs = (unsigned short)afm_size[0];
		ys = mm.ys = (unsigned short)afm_size[1];

		mm.npc_num=0;
		mm.users=0;
		memset(&mm.flag,0,sizeof(mm.flag));

		if(config.pk_mode) mm.flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		mm.gat = new mapcell_t[mm.xs*mm.ys];
		for (y = 0; y < ys; ++y)
		{
			str=fgets(afm_line, sizeof(afm_line), afm_file);
			for (x = 0; x < xs; ++x)
			{
				mm.set_type(x,y, str[x]&0x07);
			}
		}
		fclose(afm_file);
		return true;
	}
	return false;
}

bool map_readaf2(struct map_intern& mm, const char*fn)
{
	FILE *af2_file, *dest;
	char buf[256];
	bool ret=false;

	if(!fn)
	{
		char *ip;
		if(*afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, mm.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", mm.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		strcat(buf, ".af2");
	}
	else
		safestrcpy(buf, sizeof(buf), fn);

	af2_file = basics::safefopen(buf, "r");
	if( af2_file != NULL )
	{
		memcpy(buf+strlen(buf)-4, ".out", 5);
		dest = basics::safefopen(buf, "w");
		if (dest == NULL)
		{
			ShowMessage("can't open\n");
			fclose(af2_file);
		}
		else
		{
			ret = 0!=decode_file(af2_file, dest);
			fclose(af2_file);
			fclose(dest);
			if(ret) ret = map_readafm(mm, buf);
			remove(buf);
		}
	}
	return ret;
}


/*==========================================
 * マップ1枚読み甲ﾝ
 * ===================================================*/
bool map_readgrf(struct map_intern& mm, const char *fn)
{
	// read from grf
	unsigned short x,y;
	struct gat_1cell {float high[4]; uint32 type;};
	unsigned char *gat;
	char buf[512];
	
	if(!fn)
	{	char* ip;
		// have windows backslash as path seperator here
		snprintf(buf, sizeof(buf), "data\\%s", mm.mapname);
		ip = strrchr(buf,'.');
		if(ip) *ip=0;
		// append ".gat" for reading in grf's
		strcat(buf, ".gat");
	}
	else
	{	
		safestrcpy(buf, sizeof(buf), fn);
		// append ".gat" if not already exist
		if( NULL == strstr(buf,".gat") )
			strcat(buf, ".gat");
	}
	gat = grfio_read(buf);
	if( gat )
	{
		const int wh = waterlist[mm.mapname];
		const unsigned char *p = gat+14;
		struct gat_1cell pp;	// make a real structure in memory

		mm.xs= RBUFL(gat, 6);
		mm.ys= RBUFL(gat,10);
		mm.gat = new mapcell_t[mm.xs*mm.ys];

//float min=3.40282e+38, max=-3.40282e+38;
		
		for(y=0;y<mm.ys;++y)
		for(x=0;x<mm.xs;++x)
		{	// faster and typesafe
			_F_frombuffer(pp.high[0], p);
			_F_frombuffer(pp.high[1], p);
			_F_frombuffer(pp.high[2], p);
			_F_frombuffer(pp.high[3], p);
			_L_frombuffer(pp.type, p);
			// buffer increment is done automatically 
/*
if(pp.type!=1 && pp.type!=5)
{
	if(pp.high[0]<min) min = pp.high[0]; else if(pp.high[0]>max) max = pp.high[0];
	if(pp.high[1]<min) min = pp.high[1]; else if(pp.high[1]>max) max = pp.high[1];
	if(pp.high[2]<min) min = pp.high[2]; else if(pp.high[2]>max) max = pp.high[2];
	if(pp.high[3]<min) min = pp.high[3]; else if(pp.high[3]>max) max = pp.high[3];
}
*/
			if(wh!=NO_WATER && pp.type==0)
				mm.set_type(x,y, (pp.high[0]>wh || pp.high[1]>wh || pp.high[2]>wh || pp.high[3]>wh) ? 3 : 0 );
			else
				mm.set_type(x,y, pp.type );
		}
//printf("\n%f, %f\n", min, max);
		delete[] gat;
		return true;
	}
	return false;
}




#define TEST_MAP_STRUCT
#ifdef TEST_MAP_STRUCT

#include "basebinstream.h"
#include "basesafeptr.h"
#include "basesync.h"
#include "basefile.h"
#include "basezlib.h"


///////////////////////////////////////////////////////////////////////////////
// predeclaration
struct mapcache;
struct real_map;

///////////////////////////////////////////////////////////////////////////////
/// storage for terrain data.
// currently configuration only holds flat data,
// might add simplified height information.
// this would remove the water decision from the cell_t
// leaving it with 2bits so the remaining 6 bits could be used for enough height,
// i don't read the board anymore, leave comments in my pt box [Hinoko]
struct terrain
{
	///////////////////////////////////////////////////////////////////////////
	friend struct mapcache;
public:
	///////////////////////////////////////////////////////////////////////////
	// use 3 bits per cell
	enum cell_t
	{
		CELL_NORMAL=0,     // pass     normal    0 00
		CELL_WATER=1,      //          water     0 01
		CELL_QUICKSAND=2,  //          quicksand 0 10
		CELL_SHIELD=3,     //          shield    0 11
		CELL_BLOCKED=4,    // nonpass  normal    1 00
		CELL_DEEPWATER=5,  //          water     1 01
		CELL_ABYSS=6,      //          abyss     1 10
		CELL_WALL=7,       //          wall      1 11
		CELL_PASSMASK=4,
		CELL_TYPEMASK=3
	};
	///////////////////////////////////////////////////////////////////////////
	typedef unsigned short index_t;
	typedef unsigned long storage_t;
private:
	///////////////////////////////////////////////////////////////////////////
	// subscript type.
	struct subscript_t
	{
		terrain& obj;
		index_t x,y;

		subscript_t(terrain& o, index_t xi, index_t yi)
			: obj(o), x(xi), y(yi)
		{}
		operator cell_t() const
		{
			return obj.get_type(x,y);
		}
		cell_t operator=(const cell_t type)
		{
			obj.set_type(x,y,type);
			return type;
		}
	};
	friend struct subscript_t;

	///////////////////////////////////////////////////////////////////////////
	basics::string<> cName;		///< name of the terrain
	index_t cX;					///< size of x dimension
	index_t cY;					///< size of y dimension
	storage_t* cType;			///< pointer to the cell_t array
	size_t cRefCount;			///< reference counter

public:
	///////////////////////////////////////////////////////////////////////////
	terrain(const basics::string<>& name, const index_t x, const index_t y);
	~terrain();
	///////////////////////////////////////////////////////////////////////////
	/// size of the array in numbers of storage_t.
	size_t size() const
	{
		const size_t bits_per_stor = NBBY*sizeof(storage_t);
		return (3*this->cX*this->cY+bits_per_stor-1)/bits_per_stor;
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// forbid copy/assignment
	terrain(const terrain&);
	const terrain& operator=(const terrain&);
public:
	///////////////////////////////////////////////////////////////////////////
	/// subscript operator.
	const subscript_t operator()(index_t x, index_t y) const
	{
		return subscript_t(const_cast<terrain&>(*this),x,y);
	}
	subscript_t operator()(index_t x, index_t y)
	{
		return subscript_t(*this,x,y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// not-blocked.
	bool is_passable(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_PASSMASK)==CELL_NORMAL);
	}
	///////////////////////////////////////////////////////////////////////////
	/// blocked && solid.
	bool is_wall(index_t x, index_t y) const
	{
		return CELL_WALL==this->get_type(x,y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// wet (passable and unpassable alike).
	bool is_water(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_TYPEMASK)==CELL_WATER);
	}
	///////////////////////////////////////////////////////////////////////////
	/// deep dry (passable and unpassable alike).
	bool is_quicksand(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_TYPEMASK)==CELL_QUICKSAND);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// access to the array
	cell_t get_type(const index_t x, const index_t y) const;
	void set_type(const index_t x, const index_t y, cell_t type);
};




///////////////////////////////////////////////////////////////////////////////
/// entry for mapcache and map instanciation.
/// only holds data, access is done through mapcache itself
struct mapcacheentry
{
	///////////////////////////////////////////////////////////////////////////
	friend struct mapcache;

	///////////////////////////////////////////////////////////////////////////
	typedef terrain::index_t	index_t;
	typedef terrain::storage_t	storage_t;

	///////////////////////////////////////////////////////////////////////////
	basics::string<> name;	///< name of the map
	index_t		x;			///< x dimension
	index_t		y;			///< y dimension
private:
	uint32		csz;		///< compressed size of the terrain data
	uint32		ofs;		///< offset to the terrain data
	terrain*	ifo;		///< pointer to the terrain when active

public:
	///////////////////////////////////////////////////////////////////////////
	mapcacheentry()
		: x(0), y(0), csz(0), ofs(0), ifo(NULL)
	{
		printf("create mapcacheentry %p\n", static_cast<void*>(this));
	}
	~mapcacheentry()
	{
		printf("delete mapcacheentry %p\n", static_cast<void*>(this));
	}
	///////////////////////////////////////////////////////////////////////////
	mapcacheentry(const mapcacheentry& a)
		: name(a.name)
		, x(a.x)
		, y(a.y)
		, csz(a.csz)
		, ofs(a.ofs)
		, ifo(a.ifo)
	{
		printf("create mapcacheentry %p from %p\n", static_cast<void*>(this), static_cast<const void*>(&a));
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// only assignment is forbidden
	const mapcacheentry& operator=(const mapcacheentry&);
};

typedef basics::TObjPtrCount<mapcacheentry> mapcacheentry_ptr;



///////////////////////////////////////////////////////////////////////////////
/// mapcache.
/// holds a list of mapcache entries and a list of name references.
/// only exists for at startup, can be removed when loading is finished
struct mapcache
{
	///////////////////////////////////////////////////////////////////////////
	typedef terrain::index_t	index_t;
	typedef terrain::storage_t	storage_t;

	///////////////////////////////////////////////////////////////////////////
	static basics::string<> cachefilename;						///< name of the cache file
	static basics::string<> waterfilename;						///< name of the waterlist file

	///////////////////////////////////////////////////////////////////////////
	basics::smap< basics::string<>, basics::string<> > cpydata;	///< name references
	basics::smap< basics::string<>, float> waterdata;			///< water height list from waterlist file
	basics::smap< basics::string<>, mapcacheentry_ptr > mapdata;///< mapcache entries

	///////////////////////////////////////////////////////////////////////////
	mapcache(const char* mcfile=NULL, const char* whfile=NULL)
	{
		printf("create mapcache %p\n", static_cast<void*>(this));
		this->initialize(mcfile, whfile);
	}
	~mapcache()
	{
		printf("delete mapcache %p\n", static_cast<void*>(this));
	}
	///////////////////////////////////////////////////////////////////////////
	mapcache(const mapcache& a)
		: cpydata(a.cpydata)
		, waterdata(a.waterdata)
		, mapdata(a.mapdata)
	{
		printf("create mapcache copy %p from %p\n", static_cast<void*>(this), static_cast<const void*>(&a));
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// only assignment is forbidden.
	const mapcache& operator=(const mapcache&);
public:
	///////////////////////////////////////////////////////////////////////////
	/// initialize the cache.
	void initialize(const char* mcfile=NULL, const char* whfile=NULL, bool rebuild=false);
	///////////////////////////////////////////////////////////////////////////
	/// aquire an entry from the cache by name.
	mapcacheentry_ptr aquire(const basics::string<>& name) const;
	///////////////////////////////////////////////////////////////////////////
	/// load a new entry by name and aquire it.
	mapcacheentry_ptr load(const basics::string<>& name);
	///////////////////////////////////////////////////////////////////////////
	/// create a new name reference.
	bool create_clone(const basics::string<>& newname, const basics::string<>& oldname);
	///////////////////////////////////////////////////////////////////////////
	/// aquire a terrain from a mapcahce entry.
	static terrain const * aquire(mapcacheentry_ptr& entry);
	///////////////////////////////////////////////////////////////////////////
	/// release a terrain from a mapcahce entry.
	static terrain const * release(terrain const * terr, mapcacheentry_ptr& entry);
private:
	///////////////////////////////////////////////////////////////////////////
	/// open cache file and scan content.
	bool init_cache();
	///////////////////////////////////////////////////////////////////////////
	/// open waterlist file and scan content.
	bool init_waterheight();
	///////////////////////////////////////////////////////////////////////////
	/// resnametable.txt from grf and initiate name reference list.
	bool init_grf();
	///////////////////////////////////////////////////////////////////////////
	/// return the waterheight for mapname. prefere grf reading
	float waterheight(const char *mapname) const;
	///////////////////////////////////////////////////////////////////////////
	/// read waterheight from grf.
	static float waterheight_from_grf(const char *mapname);
	///////////////////////////////////////////////////////////////////////////
	/// load terrain from grf.
	terrain* read_from_grf(const char* name) const;
};

typedef basics::TObjPtrCount<mapcache> mapcache_ptr;


///////////////////////////////////////////////////////////////////////////////
/// actual map.
/// holds a pointer to the terrain and a pointer to the cache entry.
/// and implements the additional interfaces (tbd)
struct real_map : public basics::string<>
{
	///////////////////////////////////////////////////////////////////////////
	typedef basics::string<>	mybase;
	typedef terrain::index_t	index_t;


	///////////////////////////////////////////////////////////////////////////
	struct cell_t
	{
		friend struct real_map;
		real_map& m;
		index_t x,y;
	private:
		cell_t(const real_map& mi, index_t xi, index_t yi)
			: m(const_cast<real_map&>(mi)), x(xi), y(yi)
		{}
	public:
		void set_type(int t)
		{
		}
		int get_type() const
		{
			return 0;
		}

		bool is_passable() const
		{
			return false;
		}
		bool is_wall() const
		{
			return false;
		}
		bool is_ground() const
		{
			return false;
		}
		bool is_water() const
		{
			return false;
		}
		bool is_quicksand() const
		{
			return false;
		}

		bool is_npc() const
		{
			return false;
		}
		void set_npc()
		{
		}
		void clr_npc()
		{
		}

		bool is_basilica() const
		{
			return false;
		}
		void set_basilica()
		{
		}
		void clr_basilica()
		{
		}
		
		bool is_moonlit() const
		{
			return false;
		}
		void set_moonlit()
		{
		}
		void clr_moonlit()
		{
		}

		bool is_regen() const
		{
			return false;
		}
		void set_regen()
		{
		}
		void clr_regen()
		{
		}

		bool is_icewall() const
		{
			return false;
		}
		void set_icewall()
		{
		}
		void clr_icewall()
		{
		}
	};





private:
	///////////////////////////////////////////////////////////////////////////
	// none-static cell types.
	struct user_cell_t
	{
		unsigned char npc : 4;		// 4bit counter for npc touchups, can hold 15 touchups;
		unsigned char basilica : 1;	// 1bit for basilica (is on/off for basilica enough, what about two casting priests?)
		unsigned char moonlit : 1;	// 1bit for moonlit
		unsigned char regen : 1;	// 1bit for regen
		unsigned char _dummy : 1;
	};

	///////////////////////////////////////////////////////////////////////////
	static mapcache_ptr cache;			///< pointer to the mapcache.
	///////////////////////////////////////////////////////////////////////////
	basics::string<> original_name;		///< name of the original terrain
	mapcacheentry_ptr cacheentry;		///< pointer to the curent mapcache entry
	terrain const * terrain_map;		///< pointer to the terrain
	user_cell_t* modify_map;			///< array of user_cell_t

public:
	///////////////////////////////////////////////////////////////////////////
	real_map()
		: terrain_map(NULL)
		, modify_map(NULL)
	{
		printf("create empty map %p\n", static_cast<void*>(this));
	}
	///////////////////////////////////////////////////////////////////////////
	~real_map()
	{
		printf("delete map %p\n", static_cast<void*>(this));
		this->release();
	}
	///////////////////////////////////////////////////////////////////////////
	real_map(const real_map& a)
		: basics::string<>(a)
		, original_name(a.original_name)
		, cacheentry(a.cacheentry)
		, terrain_map(NULL)
		, modify_map(NULL)
	{
		printf("create map %p from %p\n", static_cast<void*>(this), static_cast<const void*>(&a));
		this->initialize();
	}
	///////////////////////////////////////////////////////////////////////////
	const real_map& operator=(const real_map& a)
	{
		this->mybase::operator=(a);
		this->original_name = a.original_name;
		this->cacheentry = a.cacheentry;
		this->initialize();
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	real_map(const basics::string<>& name)
		: terrain_map(NULL)
		, modify_map(NULL)
	{
		printf("create map %p from string\n", static_cast<void*>(this));
		this->load(name);
	}
	///////////////////////////////////////////////////////////////////////////
	const real_map& operator=(const basics::string<>& name)
	{
		this->load(name);
		return *this;
	}
	///////////////////////////////////////////////////////////////////////////
	/// assign a mapname to this map
	bool load(const basics::string<>& name);
	///////////////////////////////////////////////////////////////////////////
	/// initialize the map, create the internal structures
	bool initialize();
	///////////////////////////////////////////////////////////////////////////
	/// free the currently aquired terrain.
	void release() const
	{
		real_map* mynonconst = const_cast<real_map*>(this);
		mynonconst->terrain_map = mapcache::release(mynonconst->terrain_map, mynonconst->cacheentry);
	}
private:
	///////////////////////////////////////////////////////////////////////////
	/// aquire the terrain if not yet existing.
	const terrain& aquire() const
	{
		if( !this->terrain_map )
		{
			real_map* mynonconst = const_cast<real_map*>(this);
			mynonconst->terrain_map = mapcache::aquire(mynonconst->cacheentry);
		}
		return *this->terrain_map;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// cell access
	cell_t operator()(index_t x, index_t y) const
	{
		return cell_t(*this,x,y);
	}
	cell_t operator()(index_t x, index_t y)
	{
		return cell_t(*this,x,y);
	}

	///////////////////////////////////////////////////////////////////////////
	/// not-blocked.
	bool is_passable(index_t x, index_t y) const
	{
		return this->aquire().is_passable(x, y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// blocked && solid.
	bool is_wall(index_t x, index_t y) const
	{
		return this->aquire().is_wall(x, y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// wet (passable and unpassable alike).
	bool is_water(index_t x, index_t y) const
	{
		return this->aquire().is_water(x, y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// deep dry (passable and unpassable alike).
	bool is_quicksand(index_t x, index_t y) const
	{
		return this->aquire().is_quicksand(x, y);
	}
	///////////////////////////////////////////////////////////////////////////
	/// create and initialize the mapcache.
	static void init_cache(const char*cachefile=NULL, const char*waterfile=NULL, bool rebuild=false)
	{
		real_map::cache = mapcache_ptr(cachefile, waterfile);
	}
	///////////////////////////////////////////////////////////////////////////
	/// close and clear the mapcache.
	static void finalize_cache()
	{
		real_map::cache.clear();
	}
};




///////////////////////////////////////////////////////////////////////////////
terrain::terrain(const basics::string<>& name, const terrain::index_t x, const terrain::index_t y)
	: cName(name), cX(x), cY(y), cType(NULL), cRefCount(0)
{
	printf("create terrain %p\n", static_cast<void*>(this));
	const size_t num_stor = this->size();
	if(num_stor)
	{
		this->cType = new storage_t[num_stor];
		memset(this->cType, 0xFF, num_stor*sizeof(storage_t));
	}
}
///////////////////////////////////////////////////////////////////////////////
terrain::~terrain()
{
	printf("delete terrain %p\n", static_cast<void*>(this));
	if( this->cType ) delete[] this->cType;
}
///////////////////////////////////////////////////////////////////////////////
terrain::cell_t terrain::get_type(const terrain::index_t x, const terrain::index_t y) const
{
	if( x<cX && y<cY)
	{
		const size_t bits_per_stor = NBBY*sizeof(storage_t);
		const ldiv_t divres = ldiv(3*(x+y*cX),bits_per_stor);
		storage_t ret = this->cType[divres.quot]>>divres.rem;
		if( divres.rem+3>(long)bits_per_stor )
		{	// apend the upper bits when out of the current storage
			ret |= this->cType[divres.quot+1] << (bits_per_stor-divres.rem);
		}
		return cell_t(ret&0x7);
	}
	else
	{
		return CELL_WALL;
	}
}
///////////////////////////////////////////////////////////////////////////////
void terrain::set_type(const terrain::index_t x, const terrain::index_t y, terrain::cell_t type)
{
	if( x<cX || y<cY)
	{
		const size_t bits_per_stor = NBBY*sizeof(storage_t);
		const ldiv_t divres = ldiv(3*(x+y*cX),bits_per_stor);

		const storage_t v = type&0x07;
		this->cType[divres.quot] &= ~(0x7ul<< divres.rem);
		this->cType[divres.quot] |=  (v    << divres.rem);
		if( divres.rem+3>(long)bits_per_stor )
		{	// when out of the current storage set upper bits to next 
			this->cType[divres.quot+1] &= ~(0x07ul>> (bits_per_stor-divres.rem));
			this->cType[divres.quot+1] |=  (v     >> (bits_per_stor-divres.rem));
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
basics::string<> mapcache::cachefilename="map.info";
basics::string<> mapcache::waterfilename="water_height.txt";


///////////////////////////////////////////////////////////////////////////////
void mapcache::initialize(const char* mcfile, const char* whfile, bool rebuild)
{
	if(mcfile)
		this->cachefilename = mcfile;
	if(whfile && basics::file_exists(whfile) )
		this->waterfilename = whfile;
	this->cpydata.clear();
	this->waterdata.clear();
	this->mapdata.clear();
	if( rebuild && basics::file_exists(mcfile) )
		basics::file_delete(mcfile);

	if( !this->init_cache() )
		this->init_grf();
	this->init_waterheight();
}
///////////////////////////////////////////////////////////////////////////////
mapcacheentry_ptr mapcache::aquire(const basics::string<>& name) const
{
	const basics::string<>* trans = this->cpydata.search(name);
	const basics::string<>& realname = trans?*trans:name;

	const mapcacheentry_ptr *entry = this->mapdata.search(realname);
	return (entry)?*const_cast<mapcacheentry_ptr *>(entry):mapcacheentry_ptr();
}
///////////////////////////////////////////////////////////////////////////////
mapcacheentry_ptr mapcache::load(const basics::string<>& name)
{
	mapcacheentry_ptr newe;
	terrain* ifo = NULL;
	const basics::string<>* trans = this->cpydata.search(name);
	const basics::string<>& realname = trans?*trans:name;
	mapcacheentry_ptr* entry = this->mapdata.search(realname);
	if( entry )
	{
		return *entry;
	}
	else if( (ifo=mapcache::read_from_grf(realname)) )
	{	// create a new terrain and write to cache
		
		const size_t num_stor = ifo->size();
		if( num_stor )
		{
			basics::binaryfile bf(this->cachefilename, "rb+");
			if( !bf.is_open() )
				bf.open(this->cachefilename, "wb+");
			if( bf.is_open() )
			{
				const size_t sz = num_stor * sizeof(storage_t);

				unsigned char* buffer1 = new unsigned char[sz];
				unsigned char* buffer2 = new unsigned char[2*sz];
				// copy to buffer
				unsigned char *sp;
				storage_t * tp;
				size_t i;
				for(tp=ifo->cType, sp=buffer2; sp<buffer2+sz; ++tp)
				for(i=sizeof(storage_t); i; ++sp)
				{
					--i;
					*sp = (unsigned char)(*tp>>(8*i));
				}
				basics::CZlib zlib;
				unsigned long resultsize=2*sz;
				if( 0==zlib.encode(buffer1, resultsize, buffer2, sz) )
				{
					// write cache header
					bf.seekg(bf.size());
					bf << 0 << realname << ifo->cX << ifo->cY << resultsize;
					
					// build cache entry
					mapcacheentry e;
					e.x=ifo->cX;
					e.y=ifo->cY;
					e.csz = resultsize;
					e.ofs = bf.tellg();
					newe = mapcacheentry_ptr(e);
					this->mapdata.insert(name, newe);

					// write cache data
					bf.write(buffer1, e.csz);
				}
				delete[] buffer1;
				delete[] buffer2;
			}
		}
		delete ifo;
	}
	return newe;
}
///////////////////////////////////////////////////////////////////////////////
bool mapcache::create_clone(const basics::string<>& newname, const basics::string<>& oldname)
{
	basics::string<>* ptr = this->cpydata.search(newname);
	if( !ptr || *ptr!=oldname )
	{
		this->cpydata.insert(newname, oldname);
		// write to cache
		basics::binaryfile bf(this->cachefilename, "rb+");
		if( !bf.is_open() )
			bf.open(this->cachefilename, "wb+");
		if( bf.is_open() )
		{
			bf.seekg(bf.size());
			bf << 1 << newname << oldname;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
terrain const * mapcache::aquire(mapcacheentry_ptr& entry)
{
	if( entry.exists() )
	{
		if( !entry->ifo )
		{	// create new
			entry->ifo = new terrain(entry->name, entry->x, entry->y);
			basics::binaryfile bf(mapcache::cachefilename, "rb");
			if( bf.is_open() )
			{
				const size_t sz = entry->ifo->size()*sizeof(storage_t);
				bf.seekg(entry->ofs);
				unsigned char* buffer1 = new unsigned char[entry->csz];
				bf.read(buffer1, entry->csz);

				unsigned char* buffer2 = new unsigned char[sz];
				basics::CZlib zlib;
				unsigned long resultsize = sz;
				if( 0==zlib.decode(buffer2, resultsize, buffer1, entry->csz) )
				{	// copy to storage_t
					unsigned char *sp;
					storage_t *tp;
					size_t i;
					for(tp=entry->ifo->cType, sp=buffer2; sp<buffer2+sz; ++tp)
					for(i=sizeof(storage_t); i; --i, ++sp)
					{
						*tp = (*tp<<8) | *sp;
					}
				}
				// else decompression error
				delete[] buffer1;
				delete[] buffer2;
			}
			// else file open error
		}
		++entry->ifo->cRefCount;
		return entry->ifo;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
terrain const * mapcache::release(terrain const * terr, mapcacheentry_ptr& entry)
{
	if(terr)
	{
		terrain* ptr = const_cast<terrain*>(terr);
		--ptr->cRefCount;
		if( 0==ptr->cRefCount )
		{
			if( entry.exists() && ptr==entry->ifo )
				entry->ifo=NULL;
			delete ptr;
		}
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
bool mapcache::init_cache()
{
	bool ret = false;
	this->mapdata.clear();
	basics::binaryfile bf(this->cachefilename, "rb");
	if( bf.is_open() )
	{
		basics::string<> name, clone;
		mapcacheentry e;
		int val=0;
		ret = true;
		while( !bf.eof() )
		{
			bf >> val;
			if( val==0 )
			{	// data
				
				bf >> e.name;
				bf >> e.x >> e.y >> e.csz;
				e.ofs = bf.tellg();
				e.ifo=NULL;
				if( !e.x || !e.y || !e.csz || !bf.seekg(e.ofs+e.csz) )
				{
					ret = false;
					break;
				}
				this->mapdata.insert(e.name, mapcacheentry_ptr(e) );
			}
			else if( val==1 )
			{
				
				bf >> clone >> name;
				this->cpydata.insert(clone,name);
			}
			else
			{
				ret = false;
				break;
			}
		}
		if(!ret)
		{
			fprintf(stderr, "'%s' inconsistent, deleting\n", this->cachefilename.c_str());
			this->mapdata.clear();
			this->cpydata.clear();
			basics::file_delete(this->cachefilename);
		}
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
bool mapcache::init_waterheight()
{
	char line[1024],w1[1024];
	FILE *fp=NULL;

	fp=basics::safefopen(this->waterfilename,"r");
	if( fp )
	{
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
			this->waterdata.insert(line, (count>=2)?wh:3.f);
		}
		fclose(fp);
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool mapcache::init_grf()
{	// read and parse resnametable
	int size=0;
	char *buf = (char*)grfio_read("data\\resnametable.txt",size);
	if(buf)
	{
		char *ptr;
		char w1[256],w2[256];
		char a[256],b[256];
		// ensure string termination
		buf[size] = 0;
		for(ptr=buf; ptr-buf<size;)
		{
			if( 2==sscanf(ptr,"%256[^#]#%256[^#]#",w1,w2) )
			{
				basics::itrim(w1);
				basics::itrim(w2);
				if( *w1 && *w2 && strstr(w1,"gat") )
				{
					buffer2mapname(a,sizeof(a),w1);
					buffer2mapname(b,sizeof(b),w2);
					
					this->create_clone(a,b);
				}
			}
			ptr = strchr(ptr,'\n');	// Next line
			if (!ptr) break;
			++ptr;
		}
		delete[] buf;
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
float mapcache::waterheight(const char *mapname) const
{
	float ret = mapcache::waterheight_from_grf(mapname);
	if( ret==NO_WATER )
	{
		const float* ptr = this->waterdata.search(mapname);
		return ptr?*ptr:NO_WATER;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
float mapcache::waterheight_from_grf(const char *mapname)
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
			return whtemp;
		}
	}
	return NO_WATER;
}
///////////////////////////////////////////////////////////////////////////////
terrain* mapcache::read_from_grf(const char* name) const
{	// read from grf
	terrain* ifo=NULL;
	unsigned short x,y;
	struct gat_1cell {float high[4]; uint32 type;};
	unsigned char *gat;
	char buf[512];
	snprintf(buf, sizeof(buf), "data\\%s", name);
	char* ip = strrchr(buf,'.');
	if(ip) *ip=0;
	// append ".gat" for reading in grf's
	strcat(buf, ".gat");

	gat = grfio_read(buf);
	if( gat )
	{
		const unsigned char *p = gat+14;
		struct gat_1cell pp;	// make a real structure in memory
		const unsigned short xs= RBUFL(gat, 6);
		const unsigned short ys= RBUFL(gat,10);
		const float wh = mapcache::waterheight(name);
		ifo = new terrain(name, xs, ys);
/*
		float minh=3.40282e+38, maxh=-3.40282e+38;
		for(y=0;y<ys;++y)
		for(x=0;x<xs;++x)
		{
			_F_frombuffer(pp.high[0], p);
			_F_frombuffer(pp.high[1], p);
			_F_frombuffer(pp.high[2], p);
			_F_frombuffer(pp.high[3], p);
			_L_frombuffer(pp.type, p);
			if(pp.type!=1 && pp.type!=5)
			{
				if(pp.high[0]<minh) minh = pp.high[0]; else if(pp.high[0]>maxh) maxh = pp.high[0];
				if(pp.high[1]<minh) minh = pp.high[1]; else if(pp.high[1]>maxh) maxh = pp.high[1];
				if(pp.high[2]<minh) minh = pp.high[2]; else if(pp.high[2]>maxh) maxh = pp.high[2];
				if(pp.high[3]<minh) minh = pp.high[3]; else if(pp.high[3]>maxh) maxh = pp.high[3];
			}
		}
		printf("\n%f, %f\n", minh, maxh);
*/
		for(y=0;y<ys;++y)
		for(x=0;x<xs;++x)
		{	// faster and typesafe
			_F_frombuffer(pp.high[0], p);
			_F_frombuffer(pp.high[1], p);
			_F_frombuffer(pp.high[2], p);
			_F_frombuffer(pp.high[3], p);
			_L_frombuffer(pp.type, p);
			// buffer increment is done automatically 

			const bool wet = (pp.high[0]<wh && pp.high[1]<wh && pp.high[2]<wh && pp.high[3]<wh);
			static const terrain::cell_t cell_types[2][8] =
			{
				{terrain::CELL_NORMAL, terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_BLOCKED,   terrain::CELL_QUICKSAND, terrain::CELL_WALL},
				{terrain::CELL_WATER,  terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_WALL, terrain::CELL_DEEPWATER, terrain::CELL_WATER,     terrain::CELL_WALL}
			};
			ifo->set_type(x,y, cell_types[wet][pp.type&0x07]);
		}
		delete[] gat;
	}
	return ifo;
}



///////////////////////////////////////////////////////////////////////////////
mapcache_ptr real_map::cache;


///////////////////////////////////////////////////////////////////////////////
bool real_map::load(const basics::string<>& name)
{
	size_t p = name.find_first_of('#');
	if(p!=basics::string<>::npos)
	{	// clone the name
		this->mybase::operator=( name(0,p) );
		this->mybase::trim();
		this->original_name = name(p+1,name.size());
		this->original_name.trim();
	}
	else
	{	// use the given name for both
		this->original_name = name;
		this->original_name.trim();
		this->mybase::operator=( this->original_name );
	}
	this->cacheentry = real_map::cache->load(this->original_name);
	return this->initialize();
}
///////////////////////////////////////////////////////////////////////////////
bool real_map::initialize()
{
	this->release();
	if(this->modify_map)
		delete this->modify_map;
	if( this->cacheentry.exists() )
	{
		if( this->original_name==*this )
			this->mybase::operator=( this->cacheentry->name );
		this->original_name = this->cacheentry->name;

		const size_t sz = this->cacheentry->x * this->cacheentry->y;
		if( sz )
			this->modify_map = new user_cell_t[sz];
		return true;
	}
	return false;
}


#endif//TEST_MAP_STRUCT
