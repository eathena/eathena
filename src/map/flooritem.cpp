#include "showmsg.h"

#include "flooritem.h"
#include "intif.h"
#include "map.h"
#include "clif.h"

///////////////////////////////////////////////////////////////////////////////
/// (m,x,y)を中心に3x3以?に床アイテム設置
/// item_dataはamount以外をcopyする
int flooritem_data::create(const struct item &item_data,unsigned short amount,
						   unsigned short m,unsigned short x,unsigned short y,
						   const block_list* first_sd, const block_list* second_sd, const block_list* third_sd,
						   int type)
{
	int xy,r;
	unsigned long tick;
	flooritem_data *fitem=NULL;

	if((xy=maps[m].searchrandfreecell(x,y,2))<0)
		return 0;
	r=rand();

	fitem = new flooritem_data;
	fitem->block_list::m=m;
	fitem->block_list::x= xy&0xffff;
	fitem->block_list::y=(xy>>16)&0xffff;
	fitem->first_get_id = 0;
	fitem->first_get_tick = 0;
	fitem->second_get_id = 0;
	fitem->second_get_tick = 0;
	fitem->third_get_id = 0;
	fitem->third_get_tick = 0;

	if( 0==fitem->register_id() )
	{
		delete fitem;
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->block_list::id;
		if(type)
			fitem->first_get_tick = tick + config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->block_list::id;
		if(type)
			fitem->second_get_tick = tick + config.mvp_item_first_get_time + config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + config.item_first_get_time + config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->block_list::id;
		if(type)
			fitem->third_get_tick = tick + config.mvp_item_first_get_time + config.mvp_item_second_get_time + config.mvp_item_third_get_time;
		else
			fitem->third_get_tick = tick + config.item_first_get_time + config.item_second_get_time + config.item_third_get_time;
	}

	fitem->item_data = item_data;
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->clear_tid = add_timer(gettick()+config.flooritem_lifetime, flooritem_data::clear_timer, fitem->block_list::id, basics::numptr(fitem));
	fitem->addblock();
	clif_dropflooritem(*fitem);

	return fitem->block_list::id;
}


///////////////////////////////////////////////////////////////////////////////
/// 床アイテムを消す
int flooritem_data::clear_timer(int tid, unsigned long tick, int id, basics::numptr data)
{
	//flooritem_data *fitem = flooritem_data::from_blid(id);
	flooritem_data *fitem = static_cast<flooritem_data*>(data.ptr);
	if( fitem==NULL || (!data.num && fitem->clear_tid != tid))
	{
		if(config.error_log)
			ShowMessage("map_clearflooritem_timer : error\n");
		return 1;
	}
	fitem->clear_tid = -1;
	delete fitem;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// 
flooritem_data::~flooritem_data()
{
	if( this->clear_tid !=-1 )
		delete_timer(this->clear_tid, flooritem_data::clear_timer);

	if( this->item_data.card[0] == 0xff00 )
		intif_delete_petdata( basics::MakeDWord(this->item_data.card[1],this->item_data.card[2]) );

	clif_clearflooritem(*this);
	this->freeblock();
}

