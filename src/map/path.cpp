// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "nullpo.h"
#include "showmsg.h"
#include "utils.h"
#include "map.h"
#include "battle.h"
#include "path.h"

//#define PATH_STANDALONETEST

#define MAX_HEAP 150
struct tmp_path 
{
	short x;
	short y;
	short dist;
	short before;
	short cost;
	char dir;
	char flag;
};

/*==========================================
 * åoòHíTçıï‚èïheap push
 *------------------------------------------
 */
void push_heap_path(int heap[], struct tmp_path tp[], int index)
{
	int i,h;

	if( heap == NULL || tp == NULL ){
		ShowMessage("push_heap_path nullpo\n");
		return;
	}

	// looks like a binary heap with the heap size on element 0, 
	// stores indexes to the temp path elements and sorting them according to cost
	++heap[0];

	for( h=heap[0]-1, i=(h-1)/2;
		 h>0 && tp[index].cost<tp[heap[i+1]].cost;
		 i=(h-1)/2)
		heap[h+1]=heap[i+1],h=i;

	heap[h+1]=index;
}

/*==========================================
 * åoòHíTçıï‚èïheap update
 * costÇ™å∏Ç¡ÇΩÇÃÇ≈ç™ÇÃï˚Ç÷à⁄ìÆ
 *------------------------------------------
 */
void update_heap_path(int heap[], struct tmp_path tp[], int index)
{
	int i,h;

	nullpo_retv(heap);
	nullpo_retv(tp);

	// find the element in the heap which needs an update
	for(h=0; h<heap[0]; ++h)
		if(heap[h+1]==index)
			break;

	if(h>=heap[0])
	{	// maybe a bit hard, might be possible to just ignore that
		fprintf(stderr,"update_heap_path bug\n");
		exit(1);
	}
	// one sided update of the node 
	// since apperently costs only gets larger, so the upheap part can be skipped
	for(i=(h-1)/2;
		h>0 && tp[index].cost<tp[heap[i+1]].cost;
		i=(h-1)/2)
		heap[h+1]=heap[i+1],h=i;
	heap[h+1]=index;
}

/*==========================================
 * åoòHíTçıï‚èïheap pop
 *------------------------------------------
 */
int pop_heap_path(int heap[], struct tmp_path tp[])
{
	int i,h,k;
	int ret,last;

	nullpo_retr(-1, heap);
	nullpo_retr(-1, tp);

	// nothing in the heap
	if(heap[0]<=0)
		return -1;
	// take out the first element
	ret=heap[1];
	// get the last element
	// since the size is at index 0, the array is numbered from 1 to size
	last=heap[heap[0]];
	// we are taking out an element
	--heap[0];

	// move the hole at the top of the tree down until a leaf is reached
	for(h=0,k=2;k<heap[0];k=k*2+2){
		if(tp[heap[k+1]].cost>tp[heap[k]].cost)
			k--;
		heap[h+1]=heap[k+1], h=k;
	}
	if(k==heap[0])
		heap[h+1]=heap[k], h=k-1;
	
	// now find a suitable position for the last element by doing a upheap
	for(i=(h-1)/2;
		h>0 && tp[heap[i+1]].cost>tp[last].cost;
		i=(h-1)/2)
		heap[h+1]=heap[i+1],h=i;
	// and put it into the new position
	heap[h+1]=last;
	return ret;
}

/*==========================================
 * åªç›ÇÃì_ÇÃcoståvéZ
 *------------------------------------------
 */
int calc_cost(const struct tmp_path &tp, const int x1, const int y1)
{
	const int abs_dx = (x1>tp.x)?(x1-tp.x):(tp.x-x1);
	const int abs_dy = (y1>tp.y)?(y1-tp.y):(tp.y-y1);
	return (abs_dx+abs_dy)*10 + tp.dist;
	// this is using simple manhattan distance
	// might be not that suitable since diagonal movement is allowed but not considered here
	// since the cost for a diagonal move is higher than it should be (20 instead of 14.14213)
	//
	// so a better heuristic might be:
	// h = D * max(abs_dx, abs_dy);
	// which on the other hand prefers diagonal movements when possible
	// since costs for orthogonal and diagonal are D for both
	//
	// therefore a more sophisticaed heuristic might be:
	// diagonal_steps = min(abs_dx, abs_dy);
	// orthogon_steps = abs_dx + abs_dy;
	// h = D2 * diagonal_steps + D * (orthogon_steps - 2*diagonal_steps)
	//   = D * orthogon_steps - (2*D-D2) * diagonal_steps 
	// which still slightly favours orthogonal movement
	//
	// exact would be eucledian distance which however has other disadvantages
	//
	// with D = cost for an orthogonal step (here 10)
	// and  D2= cost for a diagonal step (here ~14)
}
inline int calc_index(const int x, const int y)
{	// this calculation is only valid when MAX_WALKPATH is multiple of 2
	return (((x)+(y)*MAX_WALKPATH) & (MAX_WALKPATH*MAX_WALKPATH-1));

	// it defines a window of MAX_WALKPATH sqared inside the actual map
	// start point is somewhere inside this the window,
	// which automatically wraps around on the window border.
	// it will overwrite existing data when path is larger than the window size
}


/*==========================================
 * ïKóvÇ»ÇÁpathÇí«â¡/èCê≥Ç∑ÇÈ
 *------------------------------------------
 */
int add_path(int heap[],struct tmp_path tp[],int x,int y,int dist,int dir,int before,int x1,int y1)
{
	int i;

	nullpo_retr(0, heap);
	nullpo_retr(0, tp);

	i=calc_index(x,y);

	
	if(tp[i].x==x && tp[i].y==y)
	{	// already have calculated this node
		if(tp[i].dist>dist)
		{	// new distance is better, 
			// so overwrite the previous
			// it would be actually better to compare the costs, 
			// comparing distance might not do the job, 
			// in this case here where distance equals the cost it might be ok 
			// but not for the scenario I have in mind
			tp[i].dist=dist;
			tp[i].dir=dir;
			tp[i].before=before;
			tp[i].cost=calc_cost(tp[i],x1,y1);
			if(tp[i].flag)
				push_heap_path(heap,tp,i);		// if it is an already processed node, push it new
			else
				update_heap_path(heap,tp,i);	// otherwise it is still on the heap so just update it
			tp[i].flag=0;
		}
		return 0;
	}

	if(tp[i].x || tp[i].y)
		return 1;

	tp[i].x=x;
	tp[i].y=y;
	tp[i].dist=dist;
	tp[i].dir=dir;
	tp[i].before=before;
	tp[i].cost=calc_cost(tp[i],x1,y1);
	tp[i].flag=0;
	push_heap_path(heap,tp,i);

	return 0;
}


/*==========================================
 * (x,y)Ç™à⁄ìÆïsâ¬î\ínë—Ç©Ç«Ç§Ç©
 * flag 0x10000 âìãóó£çUåÇîªíË
 *------------------------------------------
 */
inline bool can_place(struct map_data &m, int x, int y, int flag)
{	// cell is passable when it is passable by itself
	// or when it is a ground cell in case it's flagged
	return map_getcellp(m,x,y,CELL_CHKPASS) ||
			((flag&0x10000) && map_getcellp(m,x,y,CELL_CHKGROUND));
}

/*==========================================
 * (x0,y0)Ç©ÇÁ(x1,y1)Ç÷1ï‡Ç≈à⁄ìÆâ¬î\Ç©åvéZ
 *------------------------------------------
 */
bool can_move(struct map_data &m, const int x0, const int y0, const int x1, const int y1, const int flag)
{	// check if the position is valid
	if( x0-x1<-1 || x0-x1>1 || y0-y1<-1 || y0-y1>1 ||
		x1<0 || y1<0 || x1>=m.xs || y1>=m.ys)
		return false;
	// check if source and target postion are passable
	if( !can_place(m,x0,y0,flag) || !can_place(m,x1,y1,flag) )
		return false;
	// can finished successfully when going orthogonal
	if(x0==x1 || y0==y1)
		return true;
	// otherwise also check the neighbour cells on the diagonal path
	return (can_place(m,x0,y1,flag) && can_place(m,x1,y0,flag));
}
/*==========================================
 * (x0,y0)Ç©ÇÁ(dx,dy)ï˚å¸Ç÷countÉZÉãï™
 * êÅÇ´îÚÇŒÇµÇΩÇ†Ç∆ÇÃç¿ïWÇèäìæ
 *------------------------------------------
 */
void path_blownpos(unsigned short m,int x0,int y0,int dx,int dy,int count, int& nx, int& ny)
{
	if( count )
	{
		if(m >= map_num || !maps[m].gat)
		{
			nx=ny=0xFFFF;
			return;
		}

		if(count>15){	// ç≈ëÂ10É}ÉXÇ…êßå¿
			if(config.error_log)
				ShowMessage("path_blownpos: count too many %d !\n",count);
			count=15;
		}
		if(dx>1 || dx<-1 || dy>1 || dy<-1){
			if(config.error_log)
				ShowMessage("path_blownpos: illeagal dx=%d or dy=%d !\n",dx,dy);
			dx=(dx>=0)?1:((dx<0)?-1:0);
			dy=(dy>=0)?1:((dy<0)?-1:0);
		}
		
		while( (count--)>0 && (dx!=0 || dy!=0) ){
			if( !can_move(maps[m],x0,y0,x0+dx,y0+dy,0) ){
				int fx=(dx!=0 && can_move(maps[m],x0,y0,x0+dx,y0,0));
				int fy=(dy!=0 && can_move(maps[m],x0,y0,x0,y0+dy,0));
				if( fx && fy ){
					if(rand()&1) dx=0;
					else		 dy=0;
				}
				if( !fx )		dx=0;
				if( !fy )		dy=0;
			}
			x0+=dx;
			y0+=dy;
		}
	}
	nx = x0;
	ny = y0;
}

/*==========================================
 *  Í¿ÀÂ◊ÓÕÙ?™¨ ¶“ˆ™´™…™¶™´™Ú⁄˜™π
 *------------------------------------------
 */

bool path_search_long(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1)
{
	int dx, dy;
	int wx = 0, wy = 0;
	int weight;

	if(m >= map_num || !maps[m].gat)
		return false;

	dx = ((int)x1 - (int)x0);
	if (dx < 0) {
		basics::swap(x0, x1);
		basics::swap(y0, y1);
		dx = -dx;
	}
	dy = ((int)y1 - (int)y0);


	if (map_getcellp(maps[m],x1,y1,CELL_CHKWALL))
		return false;

	if (dx > abs(dy)) {
		weight = dx;
	} else {
		weight = abs(y1 - y0);
	}

	while (x0 != x1 || y0 != y1) {
		if (map_getcellp(maps[m],x0,y0,CELL_CHKWALL))
			return false;
		wx += dx;
		wy += dy;
		if (wx >= weight) {
			wx -= weight;
			x0 ++;
		}
		if (wy >= weight) {
			wy -= weight;
			y0 ++;
		} else if (wy < 0) {
			wy += weight;
			y0 --;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
// a bit rearranged and spliting to two loops = 12% faster
///////////////////////////////////////////////////////////////////////////////
bool path_search_long2(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1)
{
	int dx, dy,x,y;
	int w = 0;

	if(m >=map_num || !maps[m].gat)
		return false;

	dx = ((int)x1 - (int)x0);
	dy = ((int)y1 - (int)y0);

	if (abs(dx) > abs(dy))
	{
		if(dx<0)
		{
			basics::swap(x0,x1);
			basics::swap(y0,y1);
			dx=-dx;
			dy=-dy;
		}

		for(x=x0,y=y0; x<=x1; ++x)
		{
			if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
				return false;

			// next point on smaller axis
			w += dy;
			if(w >= dx)
			{
				w -= dx;
				y++;
			}
			else if(w <= -dx)
			{
				w += dx;
				y--;
			}
		}
	}
	else
	{
		if(dy<0)
		{
			basics::swap(x0,x1);
			basics::swap(y0,y1);
			dx=-dx;
			dy=-dy;
		}

		for(x=x0,y=y0; y<=y1; ++y)
		{
			if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
				return false;

			// next point on smaller axis
			w += dx;
			if(w >= dy)
			{
				w -= dy;
				x++;
			}
			if(w <= -dy)
			{
				w += dy;
				x--;
			}
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
// splitting to 4 loops = 20% faster
///////////////////////////////////////////////////////////////////////////////
bool path_search_long3(unsigned short m,unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1)
{
	int dx, dy,x,y;
	int w = 0;

	if(m >= map_num || !maps[m].gat)
		return false;

	dx = ((int)x1 - (int)x0);
	dy = ((int)y1 - (int)y0);

	if (abs(dx) > abs(dy))
	{
		if(dx<0)
		{
			basics::swap(x0,x1);
			basics::swap(y0,y1);
			dx=-dx;
			dy=-dy;
		}

		if(dy>0)
		{
			for(x=x0,y=y0; x<=x1; ++x)
			{
				if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
					return false;
				// next point on smaller axis
				w += dy;
				if(w >= dx)
				{
					w -= dx;
					y++;
				}
			}
		}
		else
		{
			for(x=x0,y=y0; x<=x1; ++x)
			{
				if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
					return false;
				// next point on smaller axis
				w += dy;
				if(w <= dx)
				{
					w += dx;
					y--;
				}
			}
		}

	}
	else
	{
		if(dy<0)
		{
			basics::swap(x0,x1);
			basics::swap(y0,y1);
			dx=-dx;
			dy=-dy;
		}
		if(dx>0)
		{
			for(x=x0,y=y0; y<=y1; ++y)
			{
				if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
					return false;

				// next point on smaller axis
				w += dx;
				if(w >= dy)
				{
					w -= dy;
					x++;
				}
			}
		}
		else
		{
			for(x=x0,y=y0; y<=y1; ++y)
			{
				if (map_getcellp(maps[m],x,y,CELL_CHKWALL))
					return false;

				// next point on smaller axis
				w += dx;
				if(w <= -dy)
				{
					w += dy;
					x--;
				}
			}		
		}
	}

	return true;
}

/*==========================================
 * pathíTçı (x0,y0)->(x1,y1)
 *------------------------------------------
 */
bool walkpath_data::path_search(unsigned short m,int x0,int y0,int x1,int y1,int flag)
{
	int heap[MAX_HEAP+1];
	struct tmp_path tp[MAX_WALKPATH*MAX_WALKPATH];
	size_t i;
	int rp,x,y;

	if(m > MAX_MAP_PER_SERVER || !maps[m].gat)
		return false;

	if( x1<0 || x1>=maps[m].xs || y1<0 || y1>=maps[m].ys || 
		(x0==x1 && y0==y1) ||
		!can_place(maps[m],x0,y0,flag) || !can_place(maps[m],x1,y1,flag) )
		return false;


	// reuse the current path when it is used, 
	// could be added when the walk process has been restructured
//	if( this->path_pos >= this->path_len )
//	{	// copy only the actual position, when pos!=0
//		if(this->path_pos)
//		{
//			this->path[0] = this->path[this->path_pos];
//			this->path_pos=0;
//		}
//		this->path_len=1;
//	}
//	else
	{	// start a new path
		this->path_len=0;
		this->path_pos=0;
	}

	// check if going straight is possible

	const int dx = (x1<x0) ? -1 : 1;
	const int dy = (y1<y0) ? -1 : 1;
	for(x=x0,y=y0,i=this->path_len; ; ++i)
	{
		if( i >= sizeof(this->path)/sizeof(this->path[0]))
			return false;
		
		if(x==x1 && y==y1)
		{	//reached the aim
			this->path_len=i;
			return true;
		}
		else
		{
			if(x!=x1 && y!=y1)
			{	// diagonal
				// target must be valid and either one of the diagonal neighbours
				if( !can_place(maps[m],x+dx,y+dy,flag) || 
					(!can_place(maps[m],x,y+dy,flag) && !can_place(maps[m],x+dx,y,flag)) )
					break;
				x+=dx;
				y+=dy;
				this->path[i]=(dx>0) ? ((dy>0)? 1 : 3) : ((dy<0)? 5 : 7);
			}
			else if(x!=x1)
			{	// orthogonal x
				if(!can_place(maps[m],x+dx,y   ,flag))
					break;
				x+=dx;
				this->path[i]=(dx>0) ? 2 : 6;
			}
			else if(y!=y1)
			{	// orthogonal y
				if(!can_place(maps[m],x    ,y+dy,flag))
					break;
				y+=dy;
				this->path[i]=(dy>0) ? 0 : 4;
			}
		}
	}

	if(flag&1)
		return false;

	// real path search
	// doing a A* algorithm with simple heuristic
	// using a binary heap as node queue

	memset(tp,0,sizeof(tp));

	// build starting point
	i=calc_index(x0,y0);
	tp[i].x=x0;
	tp[i].y=y0;
	tp[i].dist=0;
	tp[i].dir=0;
	tp[i].before=0;
	tp[i].cost=calc_cost(tp[i],x1,y1);
	tp[i].flag=0;
	heap[0]=0;
	push_heap_path(heap,tp,calc_index(x0,y0));


	// have a randomizer to randomly seperate path of same costs
	// a real tie breaker in the cost function would be better 
	// but the current number scales don't allow this, and I don't want to rescale
	const unsigned long randomizer = (rand()<<16) | rand();

	while(1)
	{
		int e=0;

		// fail when nothing on the heap
		if(heap[0]==0)
			return false;

		// get the element with the lowest cost
		rp=pop_heap_path(heap,tp);

		x=tp[rp].x;
		y=tp[rp].y;
		if(x==x1 && y==y1)
		{	// reached the destination, therefore found a path
			size_t len;
			int j;

			// find the first node and count the length
			//for(len=0,i=rp; len<sizeof(wpd.path)/sizeof(wpd.path[0]) && i!=(size_t)calc_index(x0,y0); i=tp[i].before, ++len);
			// break if too long
			//if(len>=sizeof(wpd.path)/sizeof(wpd.path[0]))
			//	return -1;
			/////////////////////////////////////////////////
			// other option:
			// find the first node and count the length
			for(len=this->path_len,i=rp; i!=(size_t)calc_index(x0,y0); i=tp[i].before, ++len);
			// limit length when too long
			if( len>=sizeof(this->path)/sizeof(this->path[0])) 
				len=sizeof(this->path)/sizeof(this->path[0]);
			/////////////////////////////////////////////////
			// fill the walkpath structure, 
			// only take the directions that have to be walked to at each position
			for(i=rp,j=len-1; j>=this->path_len; i=tp[i].before,--j)
				this->path[j] = tp[i].dir;
			this->path_len=len;

			// and return success
			return true;
		}
		// cut out a single bit of the randomizer depending on current x/y
		const int randbit = 0x1 & (randomizer>>((x*y)&0x1F)); 

		// check surrounding in order N, NE, E, SE, S, SW, W, NW and add it to the heap
		// have weight 10 for orthogonals, and 14 for diagonals (should be 10*sqr(2) but 14 is good enough)
		if(can_move(maps[m],x,y,x  ,y+1,flag)) e+=add_path(heap,tp,x  ,y+1,tp[rp].dist+10+randbit,0,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y+1,flag)) e+=add_path(heap,tp,x+1,y+1,tp[rp].dist+14+randbit,1,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y  ,flag)) e+=add_path(heap,tp,x+1,y  ,tp[rp].dist+10+randbit,2,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y-1,flag)) e+=add_path(heap,tp,x+1,y-1,tp[rp].dist+14+randbit,3,rp,x1,y1);
		if(can_move(maps[m],x,y,x  ,y-1,flag)) e+=add_path(heap,tp,x  ,y-1,tp[rp].dist+10+randbit,4,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y-1,flag)) e+=add_path(heap,tp,x-1,y-1,tp[rp].dist+14+randbit,5,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y  ,flag)) e+=add_path(heap,tp,x-1,y  ,tp[rp].dist+10+randbit,6,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y+1,flag)) e+=add_path(heap,tp,x-1,y+1,tp[rp].dist+14+randbit,7,rp,x1,y1);

		// set the node as handled
		tp[rp].flag=1;

		// break when error or heap almost full 
		// but no idea why it's limited to 5 below max heapsize
		// it instead could overwrite the node which is most unlikely to be on the sortest path
		if(e || heap[0]>=MAX_HEAP-5)
			return false;
	}
	// return failure
	return false;
}

/// same as pathsearch but does not fill the path structure
bool walkpath_data::is_possible(unsigned short m,int x0,int y0,int x1,int y1,int flag)
{
	int x,y;

	if(m > MAX_MAP_PER_SERVER || !maps[m].gat)
		return false;

	if( x1<0 || x1>=maps[m].xs || y1<0 || y1>=maps[m].ys || 
		!can_place(maps[m],x0,y0,flag) || !can_place(maps[m],x1,y1,flag) )
		return false;

	// check if going straight is possible

	const int dx = (x1<x0) ? -1 : 1;
	const int dy = (y1<y0) ? -1 : 1;
	for(x=x0,y=y0; ; )
	{
		
		if(x==x1 && y==y1)
		{	//reached the aim
			return true;
		}
		else
		{
			if(x!=x1 && y!=y1)
			{	// diagonal
				// target must be valid and either one of the diagonal neighbours
				if( !can_place(maps[m],x+dx,y+dy,flag) || 
					(!can_place(maps[m],x,y+dy,flag) && !can_place(maps[m],x+dx,y,flag)) )
					break;
				x+=dx;
				y+=dy;
			}
			else if(x!=x1)
			{	// orthogonal x
				if(!can_place(maps[m],x+dx,y   ,flag))
					break;
				x+=dx;
			}
			else if(y!=y1)
			{	// orthogonal y
				if(!can_place(maps[m],x    ,y+dy,flag))
					break;
				y+=dy;
			}
		}
	}

	if(flag&1)
		return false;


	// real path search
	// doing a A* algorithm with simple heuristic
	// using a binary heap as node queue
	int heap[MAX_HEAP+1];
	struct tmp_path tp[MAX_WALKPATH*MAX_WALKPATH];
	size_t i;
	int rp;

	//memset(tp,0,sizeof(tp));

	// build starting point
	i=calc_index(x0,y0);
	tp[i].x=x0;
	tp[i].y=y0;
	tp[i].dist=0;
	tp[i].dir=0;
	tp[i].before=0;
	tp[i].cost=calc_cost(tp[i],x1,y1);
	tp[i].flag=0;
	heap[0]=0;
	push_heap_path(heap,tp,calc_index(x0,y0));


	// have a randomizer to randomly seperate path of same costs
	// a real tie breaker in the cost function would be better 
	// but the current number scales don't allow this, and I don't want to rescale
	const unsigned long randomizer = (rand()<<16) | rand();

	while(1)
	{
		int e=0;

		// fail when nothing on the heap
		if(heap[0]==0)
			return false;

		// get the element with the lowest cost
		rp=pop_heap_path(heap,tp);

		x=tp[rp].x;
		y=tp[rp].y;
		if(x==x1 && y==y1)
		{	// reached the destination, therefore found a path
			return true;
		}
		// cut out a single bit of the randomizer depending on current x/y
		const int randbit = 0x1 & (randomizer>>((x*y)&0x1F)); 

		// check surrounding in order N, NE, E, SE, S, SW, W, NW and add it to the heap
		// have weight 10 for orthogonals, and 14 for diagonals (should be 10*sqr(2) but 14 is good enough)
		if(can_move(maps[m],x,y,x  ,y+1,flag)) e+=add_path(heap,tp,x  ,y+1,tp[rp].dist+10+randbit,0,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y+1,flag)) e+=add_path(heap,tp,x+1,y+1,tp[rp].dist+14+randbit,1,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y  ,flag)) e+=add_path(heap,tp,x+1,y  ,tp[rp].dist+10+randbit,2,rp,x1,y1);
		if(can_move(maps[m],x,y,x+1,y-1,flag)) e+=add_path(heap,tp,x+1,y-1,tp[rp].dist+14+randbit,3,rp,x1,y1);
		if(can_move(maps[m],x,y,x  ,y-1,flag)) e+=add_path(heap,tp,x  ,y-1,tp[rp].dist+10+randbit,4,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y-1,flag)) e+=add_path(heap,tp,x-1,y-1,tp[rp].dist+14+randbit,5,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y  ,flag)) e+=add_path(heap,tp,x-1,y  ,tp[rp].dist+10+randbit,6,rp,x1,y1);
		if(can_move(maps[m],x,y,x-1,y+1,flag)) e+=add_path(heap,tp,x-1,y+1,tp[rp].dist+14+randbit,7,rp,x1,y1);

		// set the node as handled
		tp[rp].flag=1;

		// break when error or heap almost full 
		// but no idea why it's limited to 5 below max heapsize
		// it instead could overwrite the node which is most unlikely to be on the sortest path
		if(e || heap[0]>=MAX_HEAP-5)
			return false;
	}
	// return failure
	return false;
}

#ifdef PATH_STANDALONETEST

char gat[64][64]={
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0},
};
struct map_data maps[1];

/*==========================================
 * åoòHíTçıÉãÅ[É`ÉìíPëÃÉeÉXÉgópmainä÷êî
 *------------------------------------------
 */
void main(int argc,char *argv[])
{
	struct walkpath_data wpd;

	maps[0].gat=gat;
	maps[0].xs=64;
	maps[0].ys=64;

	wpd.path_search(0,3,4,5,4,0);
	wpd.path_search(0,5,4,3,4,0);
	wpd.path_search(0,6,4,3,4,0);
	wpd.path_search(0,7,4,3,4,0);
	wpd.path_search(0,4,3,4,5,0);
	wpd.path_search(0,4,2,4,5,0);
	wpd.path_search(0,4,1,4,5,0);
	wpd.path_search(0,4,5,4,3,0);
	wpd.path_search(0,4,6,4,3,0);
	wpd.path_search(0,4,7,4,3,0);
	wpd.path_search(0,7,4,3,4,0);
	wpd.path_search(0,8,4,3,4,0);
	wpd.path_search(0,9,4,3,4,0);
	wpd.path_search(0,10,4,3,4,0);
	wpd.path_search(0,11,4,3,4,0);
	wpd.path_search(0,12,4,3,4,0);
	wpd.path_search(0,13,4,3,4,0);
	wpd.path_search(0,14,4,3,4,0);
	wpd.path_search(0,15,4,3,4,0);
	wpd.path_search(0,16,4,3,4,0);
	wpd.path_search(0,17,4,3,4,0);
	wpd.path_search(0,18,4,3,4,0);
}
#endif


