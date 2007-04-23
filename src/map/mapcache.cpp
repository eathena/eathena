
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

struct map_cache {
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

bool map_cache_read(struct map_data &block_list)
{
	size_t i;
	if(!map_cache.fp) { return 0; }
	for(i=0; i<map_cache.head.nmaps; ++i)
	{
		if( map_cache.map[i].water_height == block_list.wh &&	// 水場の高さが違うので読み直し
			0==strcmp(block_list.mapname,map_cache.map[i].fn) )
			break;
	}
	if( i < map_cache.head.nmaps )	
	{
		if(map_cache.map[i].compressed == 0)
		{
				// 非圧縮ファイル
			const size_t dest_sz = map_cache.map[i].xs*map_cache.map[i].ys;
			block_list.xs = map_cache.map[i].xs;
			block_list.ys = map_cache.map[i].ys;
			block_list.gat = new struct mapgat[dest_sz];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if( map_cache.map[i].datalen > dest_sz*sizeof(struct mapgat) )
				map_cache.map[i].datalen = dest_sz*sizeof(struct mapgat);
			if( map_cache.map[i].datalen != fread(block_list.gat, 1, map_cache.map[i].datalen, map_cache.fp) )
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] block_list.gat;
				block_list.gat = NULL;
				block_list.xs = 0;
				block_list.ys = 0;
				return false;
			}
		}
		else if(map_cache.map[i].compressed == 1)
		{	//zlib compressed
				// 圧縮フラグ=1 : zlib
			unsigned char *buf;
			unsigned long size_compress = map_cache.map[i].datalen;
			unsigned long dest_len = map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat);
			block_list.xs = map_cache.map[i].xs;
			block_list.ys = map_cache.map[i].ys;
			buf = new unsigned char[size_compress];
			fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
			if(fread(buf,1,size_compress,map_cache.fp) != size_compress)
			{	// なぜかファイル後半が欠けてるので読み直し
				delete[] buf;
				buf = NULL;
				block_list.gat = NULL;
				block_list.xs = 0; 
				block_list.ys = 0; 
				return false;
			}
			block_list.gat = new struct mapgat[map_cache.map[i].xs * map_cache.map[i].ys];
			decode_zip((unsigned char*)block_list.gat, dest_len, buf, size_compress);
			if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys * sizeof(struct mapgat))
			{	// 正常に解凍が出来てない
				delete[] buf;
				buf=NULL;
				delete[] block_list.gat;
				block_list.gat = NULL;
				block_list.xs = 0; 
				block_list.ys = 0; 
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
		const size_t sz = block_list.xs*block_list.ys;
		bool warn=true;
		for(i=0; i<sz; ++i)
		{
			/////////////////////////////////////////////////////////
			//## remove at the latest by 12/2006
			if( warn && config.etc_log &&
				(block_list.gat[i].npc || block_list.gat[i].basilica || block_list.gat[i].moonlit || block_list.gat[i].regen) )
			{
				ShowWarning("Map Cache corrupted, please rebuild it\n");
				warn = false;
			}
			/////////////////////////////////////////////////////////

			block_list.gat[i].npc = 0;
			block_list.gat[i].basilica = 0;
			block_list.gat[i].moonlit = 0;
			block_list.gat[i].regen = 0;
		}
		return true;
	}
	return false;
}

bool map_cache_write(struct map_data &block_list)
{
	size_t i;
	unsigned long len_new, len_old;
	unsigned char *write_buf;

	if(!map_cache.fp)
		return false;

	for(i = 0;i < map_cache.head.nmaps ; ++i) 
	{
		if( (0==strcmp(block_list.mapname,map_cache.map[i].fn)) || (map_cache.map[i].fn[0] == 0) )
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
			len_new = 2 * block_list.xs * block_list.ys * sizeof(struct mapgat);
			write_buf = new unsigned char[len_new];
			encode_zip(write_buf,len_new,(unsigned char *)block_list.gat, block_list.xs*block_list.ys*sizeof(struct mapgat));
		}
		else
		{	// no compress
			len_new = block_list.xs * block_list.ys *sizeof(struct mapgat);
			write_buf = (unsigned char*)block_list.gat;
		}
		
		// now insert it
		if( (map_cache.map[i].fn[0] == 0) )
		{	// new map is inserted
			// write at the end of the mapcache file
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);

			// prepare the data header
			safestrcpy(map_cache.map[i].fn, sizeof(map_cache.map[i].fn), block_list.mapname);

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
		map_cache.map[i].xs  = block_list.xs;
		map_cache.map[i].ys  = block_list.ys;
		map_cache.map[i].water_height = block_list.wh;
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

bool map_readafm(struct map_data& block_list, const char *fn)
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

	int x,y,xs,ys;
	char afm_line[65535];
	int afm_size[2];
	FILE *afm_file;
	char *str;
	char buf[512];

	if(!fn)
	{
		char *ip;
		if(afm_dir && *afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, block_list.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", block_list.mapname);
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

		xs = block_list.xs = afm_size[0];
		ys = block_list.ys = afm_size[1];

		block_list.npc_num=0;
		block_list.users=0;
		memset(&block_list.flag,0,sizeof(block_list.flag));

		if(config.pk_mode) block_list.flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		block_list.gat = new struct mapgat[block_list.xs*block_list.ys];
		for (y = 0; y < ys; ++y)
		{
			str=fgets(afm_line, sizeof(afm_line), afm_file);
			for (x = 0; x < xs; ++x)
			{
				map_setcell(block_list.m,x,y, str[x] & CELL_MASK );
			}
		}
		fclose(afm_file);
		return true;
	}
	return false;
}

bool map_readaf2(struct map_data& block_list, const char*fn)
{
	FILE *af2_file, *dest;
	char buf[256];
	bool ret=false;

	if(!fn)
	{
		char *ip;
		if(afm_dir && *afm_dir)
			snprintf(buf, sizeof(buf),"%s%c%s",afm_dir, PATHSEP, block_list.mapname);
		else
			snprintf(buf, sizeof(buf),"%s", block_list.mapname);
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
			if(ret) ret = map_readafm(block_list, buf);
			remove(buf);
		}
	}
	return ret;
}


/*==========================================
 * マップ1枚読み甲ﾝ
 * ===================================================*/
bool map_readgrf(struct map_data& block_list, const char *fn)
{
	// read from grf
	int x,y;
	struct gat_1cell {float high[4]; uint32 type;};
	unsigned char *gat;
	char buf[512];
	
	if(!fn)
	{	char* ip;
		// have windows backslash as path seperator here
		snprintf(buf, sizeof(buf), "data\\%s", block_list.mapname);
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
		const unsigned char *p = gat+14;
		struct gat_1cell pp;	// make a real structure in memory

		block_list.xs= RBUFL(gat, 6);
		block_list.ys= RBUFL(gat,10);
		block_list.gat = new struct mapgat[block_list.xs*block_list.ys];

//float min=3.40282e+38, max=-3.40282e+38;
		
		for(y=0;y<block_list.ys;++y)
		for(x=0;x<block_list.xs;++x)
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
			if(block_list.wh!=NO_WATER && pp.type==0)
				map_setcell(block_list.m,x,y,(pp.high[0]>block_list.wh || pp.high[1]>block_list.wh || pp.high[2]>block_list.wh || pp.high[3]>block_list.wh) ? 3 : 0);
			else
				map_setcell(block_list.m,x,y,pp.type);
		}
//printf("\n%f, %f\n", min, max);
		delete[] gat;
		return true;
	}
	return false;
}









#ifdef TEST_MAP_STRUCT


struct __map : public basics::string<>
{
public:
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

	// move this out it disturbes map cloning
	// better implement those as mapobjects
	struct user_cell_t
	{
		unsigned char npc : 4;		// 4bit counter for npc touchups, can hold 15 touchups;
		unsigned char basilica : 1;	// 1bit for basilica (is on/off for basilica enough, what about two casting priests?)
		unsigned char moonlit : 1;	// 1bit for moonlit
		unsigned char regen : 1;	// 1bit for regen
		unsigned char _dummy : 1;
	};

private:
	typedef unsigned short index_t;
	typedef unsigned long storage_t;

	struct subscript_t
	{
		__map& obj;
		index_t x,y;

		subscript_t(__map& o, index_t xi, index_t yi)
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

	index_t cX;
	index_t cY;
	storage_t* cType;
	user_cell_t* cUser;
public:
	__map(const basics::string<>& name, const index_t x, const index_t y)
		: basics::string<>(name), cX(x), cY(y), cType(NULL), cUser(NULL)
	{
		//////////
		const size_t bits_per_stor = NBBY*sizeof(storage_t);
		const size_t num_stor = (3*x*y+bits_per_stor-1)/bits_per_stor;
		this->cType = new storage_t[num_stor];
		memset(this->cType, 0xFF, num_stor*sizeof(storage_t));
		//////////
		this->cUser = new user_cell_t[x*y];
		memset(this->cUser, 0, x*y*sizeof(user_cell_t));
	}
	~__map()
	{
		if( this->cType ) delete[] this->cType;
		if( this->cUser ) delete[] this->cUser;
	}
	const subscript_t operator()(index_t x, index_t y) const
	{
		return subscript_t(const_cast<__map&>(*this),x,y);
	}
	subscript_t operator()(index_t x, index_t y)
	{
		return subscript_t(*this,x,y);
	}

	// not-blocked
	bool is_passable(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_PASSMASK)==CELL_NORMAL);
	}
	// blocked && solid
	bool is_wall(index_t x, index_t y) const
	{
		return CELL_WALL==this->get_type(x,y);
	}
	// wet (passable and unpassable alike)
	bool is_water(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_TYPEMASK)==CELL_WATER);
	}
	// deep dry (passable and unpassable alike)
	bool is_quicksand(index_t x, index_t y) const
	{
		return ((this->get_type(x,y)&CELL_TYPEMASK)==CELL_QUICKSAND);
	}

private:
	cell_t get_type(const index_t x, const index_t y) const
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
	void set_type(const index_t x, const index_t y, cell_t type)
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
};

#endif//TEST_MAP_STRUCT


