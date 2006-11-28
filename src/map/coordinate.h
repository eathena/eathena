// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __COORDINATE_H__
#define __COORDINATE_H__


/// directions.
/// 45degree enumerates with compass orientation
enum dir_t { DIR_N=0, DIR_NE, DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW };

/// calculates distance between two coordinates.
int distance(int x0,int y0,int x1,int y1);

/// calculates direction from first to second coordinate.
dir_t direction(int x0,int y0,int x1,int y1);

/// checks for same direction.
bool is_same_direction(dir_t s_dir, dir_t t_dir);


///////////////////////////////////////////////////////////////////////////////
/// 2 dimentional coordinate.
struct coordinate
{
	unsigned short x;
	unsigned short y;

	coordinate()
		: x(0), y(0)
	{}
	coordinate(unsigned short xi, unsigned short yi)
		: x(xi), y(yi)
	{}
	~coordinate()
	{}

	/// get the distance from this coordinate to the x/y.
	int get_distance(int x,int y) const
	{
		return distance(this->x,this->y, x, y);
	}
	/// get the distance from this coordinate to another.
	int get_distance(const coordinate& c) const
	{
		return distance(this->x,this->y, c.x, c.y);
	}
	/// get the direction from this coordinate to the x/y.
	dir_t get_direction(int x,int y) const
	{
		return direction(this->x,this->y, x, y);
	}
	/// get the direction from this coordinate to another.
	dir_t get_direction(const coordinate& c) const
	{
		return direction(this->x,this->y, c.x, c.y);
	}

	/// coordinate distance.
	friend inline int distance(const coordinate &c1, const coordinate &c2)
	{
		return distance(c1.x,c1.y,c2.x,c2.y);
	}
	/// coordinate direction.
	friend inline dir_t direction(const coordinate &c1, const coordinate &c2)
	{
		return direction(c1.x,c1.y,c2.x,c2.y);
	}

	/// eq compare operator.
	friend inline bool operator==(const coordinate &c1, const coordinate &c2)
	{
		return c1.x==c2.x && c1.y==c2.y;
	}
	/// ne compare operator.
	friend inline bool operator!=(const coordinate &c1, const coordinate &c2)
	{
		return c1.x!=c2.x || c1.y!=c2.y;
	}
};





#endif//__COORDINATE_H__
