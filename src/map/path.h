#ifndef _PATH_H_
#define _PATH_H_

// strictly limited to that,
// if changing check all path code for storage limitations
#define MAX_WALKPATH 32	


///////////////////////////////////////////////////////////////////////////////
/// stores a walkpath.
/// len is the overall path, pos is the current position inside the path, 
/// the path itself contains the directions that have to be walked to sequentially, 
/// also add the target destination here since it is always used when a walkpath is used
struct walkpath_data
{										// necessary sizes:

private:
	unsigned char path_len		: 6;	// more than necessary but it fills the 
	unsigned char path_pos		: 6;	// struct to a 16bit boundary
public:
	unsigned char change_target	: 1;	// need to recalculate
	unsigned char walk_easy		: 1;	// only walk in line-of-sight
	unsigned char _dummy		: 2;

	unsigned char path[MAX_WALKPATH];

	// default constructor
	walkpath_data() : 
		path_len(0),
		path_pos(0),
		change_target(0),
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
		return *this;
	}


	bool path_search(unsigned short m,int x0,int y0,int x1,int y1,int flag);
	static bool is_possible(unsigned short m,int x0,int y0,int x1,int y1,int flag);

	bool first_step() const 		{ return this->path_pos == 0; }
	bool finished() const			{ return this->path_pos >= this->path_len; }
	dir_t get_current_step() const	{ return dir_t(this->path[this->path_pos]&0x07); }

	void clear()
	{
		this->path_pos = this->path_len = change_target = 0;
	}

	bool next()
	{
		++this->path_pos;
		if( this->path_pos >= this->path_len )
		{
			this->path_pos = this->path_len = 0;
			return false;
		}
		return true;
	}
	bool operator++(int)	{ return this->next(); }
	bool operator++()		{ return this->next(); }


	unsigned long get_path_time() const
	{
		unsigned long c,i;
		for(c=0, i=this->path_pos; i<this->path_len; ++i)
		{	// diagonals count 14, orthogonals 10
			c += (this->path[i]&1) ? 14 : 10;
		}
		return c;
	}

};

// path.c‚æ‚è
bool path_search_long(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1);
void path_blownpos(unsigned short m,int x0,int y0,int dx,int dy,int count, int& nx, int& ny);


#endif//_PATH_H_
