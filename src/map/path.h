#ifndef _PATH_H_
#define _PATH_H_


#define MAX_WALKPATH 32


///////////////////////////////////////////////////////////////////////////////
/// stores a walkpath.
/// len is the overall path, pos is the current position inside the path,
/// path_half marks when the path section is just used and the path itself
/// contains the directions that have to be walked to sequentially
//## maybe also add the target destination here since the to_x/to_y is always used
//## when a walkpath is used
struct walkpath_data
{										// necessary sizes:
	unsigned char path_len;				// 0...MAX_WALKPATH-1	->currently 5bit
	unsigned char path_pos;				// 0...MAX_WALKPATH-1	->currently 5bit
	unsigned char path_half;			// 0/1					->1bit
	unsigned char path[MAX_WALKPATH];	// 0..7					->3bit each
										//						-----------------
										//						107bit
										// currently used:		280bit
	// default constructor
	walkpath_data() :
		path_len(0),
		path_pos(0),
		path_half(0)
	{
		memset(path,0, sizeof(path));
		// might be not necessary to clean it
		// since there always is the valid length given
	}


	bool path_search(unsigned short m,int x0,int y0,int x1,int y1,int flag);

};

// path.c‚æ‚è
bool path_search_long(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1);
void path_blownpos(unsigned short m,int x0,int y0,int dx,int dy,int count, int& nx, int& ny);


#endif//_PATH_H_
