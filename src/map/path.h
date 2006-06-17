#ifndef _PATH_H_
#define _PATH_H_

// strictly limited to that,
// if changing check all path code for storage limitations
#define MAX_WALKPATH 32	


///////////////////////////////////////////////////////////////////////////////
/// stores a walkpath.
/// len is the overall path, pos is the current position inside the path,
/// path_half marks when the path section is just used and the path itself
/// contains the directions that have to be walked to sequentially
/// also add the target destination here since it is always used
/// when a walkpath is used
struct walkpath_data
{										// necessary sizes:
//	unsigned char path_len;				// 0...MAX_WALKPATH-1	->currently 5bit
//	unsigned char path_pos;				// 0...MAX_WALKPATH-1	->currently 5bit
//	unsigned char path_half;			// 0/1					->1bit
//	unsigned char path[MAX_WALKPATH];	// 0..7					->3bit each
										//						-----------------
										//						107bit
										// currently used:		280bit

	unsigned char path_len		: 6;	// more than necessary but it fills the 
	unsigned char path_pos		: 6;	// struct to a 16bit boundary
	unsigned path_half			: 1;
	unsigned change_walk_target	: 1;
	unsigned walk_easy			: 1;
	unsigned _dummy				: 1;

	// -> change this to coordinate when coordinates get their own file
	struct _dummy
	{
		unsigned short x;
		unsigned short y;

		_dummy() : x(0),y(0)
		{}
	} target;

	unsigned char path[MAX_WALKPATH];

	// default constructor
	walkpath_data() : 
		path_len(0),
		path_pos(0),
		path_half(0),
		change_walk_target(0),
		walk_easy(0)
	{
		memset(path,0, sizeof(path));
		// might be not necessary to clean it
		// since there always is the valid path length given
	}

	const walkpath_data& operator=(const walkpath_data& wp)
	{	// only copy the path not the walk flags 
		// since it is still old behaviour on usage
		memcpy(this->path,wp.path, sizeof(this->path));
		this->path_len = wp.path_len;
		this->path_pos = wp.path_pos;
		this->path_half= wp.path_half;
		return *this;
	}


	bool path_search(unsigned short m,int x0,int y0,int x1,int y1,int flag);
	static bool is_possible(unsigned short m,int x0,int y0,int x1,int y1,int flag);

};

// path.c‚æ‚è
bool path_search_long(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1);
void path_blownpos(unsigned short m,int x0,int y0,int dx,int dy,int count, int& nx, int& ny);


#endif//_PATH_H_
