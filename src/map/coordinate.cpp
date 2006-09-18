// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "basetypes.h"

#include "coordinate.h"




///////////////////////////////////////////////////////////////////////////////
/// check for blocks in same direction.
/// appearently it's returning true when directions are same or 45degree
bool is_same_direction(dir_t s_dir,dir_t t_dir)
{
/*	if(s_dir == t_dir)
		return true;
	switch(s_dir) {
		case 0:
			if(t_dir == 7 || t_dir == 1 || t_dir == 0)
				return true;
			break;
		case 1:
			if(t_dir == 0 || t_dir == 2 || t_dir == 1)
				return true;
			break;
		case 2:
			if(t_dir == 1 || t_dir == 3 || t_dir == 2)
				return true;
			break;
		case 3:
			if(t_dir == 2 || t_dir == 4 || t_dir == 3)
				return true;
			break;
		case 4:
			if(t_dir == 3 || t_dir == 5 || t_dir == 4)
				return true;
			break;
		case 5:
			if(t_dir == 4 || t_dir == 6 || t_dir == 5)
				return true;
			break;
		case 6:
			if(t_dir == 5 || t_dir == 7 || t_dir == 6)
				return true;
			break;
		case 7:
			if(t_dir == 6 || t_dir == 0 || t_dir == 7)
				return true;
			break;
	}
	return false;
*/
	return (s_dir == t_dir || ((s_dir+1)&0x07)==t_dir || s_dir==((t_dir+1)&0x07));

/*
	static const char table[8]	= { 0x83,0x07,0x0E,0x1A,0x38,0x70,0xE0,0xA1 }
	return ( table[s_dir] & (1<<t_dir) )
	
*/

}

///////////////////////////////////////////////////////////////////////////////
/// 彼我の方向を計算
dir_t direction(int x0,int y0,int x1,int y1)
{
	dir_t dir=DIR_N;
	int dx,dy;
	dx = x1-x0;
	dy = y1-y0;
	if( dx==0 && dy==0 )
	{	// 彼我の場所一致
		dir=DIR_N;						// 上
	}
	else if( dx>=0 && dy>=0 )
	{	// 方向的に右上
		if( dx*2-1<dy )		dir=DIR_N;	// 上
		else if( dx>dy*2 )	dir=DIR_W;	// 右
		else				dir=DIR_NW;	// 右上
	}
	else if( dx>=0 && dy<=0 )
	{	// 方向的に右下
		if( dx*2-1<-dy )	dir=DIR_S;	// 下
		else if( dx>-dy*2 )	dir=DIR_W;	// 右
		else				dir=DIR_SW;	// 右下
	}
	else if( dx<=0 && dy<=0 )
	{	// 方向的に左下
		if( dx*2+1>dy )		dir=DIR_S;	// 下
		else if( dx<dy*2 )	dir=DIR_E;	// 左
		else				dir=DIR_SE;	// 左下
	}
	else
	{	// 方向的に左上
		if( -dx*2-1<dy )	dir=DIR_N;	// 上
		else if( -dx>dy*2 )	dir=DIR_E;	// 左
		else				dir=DIR_NE;	// 左上
	}
	return dir;
}

///////////////////////////////////////////////////////////////////////////////
/// calculate distance.
/// original code did manhattan formula, 
/// was changed to an euclidean distance approximation
int distance(int x0,int y0,int x1,int y1)
{
	int dx,dy;
	//dx=abs(x0-x1);
	//dy=abs(y0-y1);
	//return dx>dy ? dx : dy;

	// euclidean distance approximation with piecewise linear octagon
	// but calculated explicitely correct at the borders (PI/4 multiples)
	dx = (x0>x1)?x0-x1:x1-x0;
	dy = (y0>y1)?y0-y1:y1-y0;
	if(dx==0) return dy;
	if(dy==0) return dx;
	if(dy>=dx) if(dy>dx) basics::swap(dx,dy); else return (dx*362)>>8;
	//http://www.flipcode.com/articles/article_fastdistance.shtml
	return ( (( dx << 8 ) + ( dx << 3 ) - ( dx << 4 ) - ( dx << 1 ) +
			  ( dy << 7 ) - ( dy << 5 ) + ( dy << 3 ) - ( dy << 1 )) >> 8 );

}

