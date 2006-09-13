// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "socket.h"
#include "showmsg.h"
#include "utils.h"

#include "map.h"
#include "atcommand.h"
#include "clif.h"
#include "itemdb.h"
#include "vending.h"
#include "pc.h"
#include "skill.h"
#include "battle.h"
#include "nullpo.h"
#include "log.h"

/*==========================================
 * 露店閉鎖
 *------------------------------------------
*/
void vending_closevending(struct map_session_data &sd)
{
	sd.vender_id=0;
	clif_closevendingboard(sd,0);
}

/*==========================================
 * 露店アイテムリスト要求
 *------------------------------------------
 */
void vending_vendinglistreq(struct map_session_data &sd, uint32 id)
{
	struct map_session_data *vsd=map_session_data::from_blid(id);
	if( vsd && vsd->vender_id )
		clif_vendinglist(sd,id,vsd->vend_list);
}

/*==========================================
 * 露店アイテム購入
 *------------------------------------------
 */
void vending_purchasereq(struct map_session_data &sd,unsigned short len,uint32 id,unsigned char *buffer)
{
	size_t i, j, w;
	uint32 z;
	unsigned short blank, vend_ids[MAX_VENDING];
	unsigned short amount, index, new_ = 0;
	vending_element vend_list[MAX_VENDING]; // against duplicate packets
	map_session_data *vsd = map_session_data::from_blid(id);

	if(vsd == NULL)
		return;
	if(vsd->vender_id == 0)
		return;
	if(vsd->vender_id == sd.block_list::id)
		return;
	if( vsd->vend_num>MAX_VENDING )
		vsd->vend_num=MAX_VENDING;

	// duplicate item in vend_list to check hacker with multiple packets
	memcpy(&vend_list, &vsd->vend_list, sizeof(vend_list));

	// number of blank entries in inventory
	blank = pc_inventoryblank(sd);

	for(i = 0, w = z = 0; 8 + 4 * i < len; ++i) {
		amount = RBUFW(buffer, 4 * i);
		index =  RBUFW(buffer, 2 + 4 * i) - 2;

		if(amount > MAX_AMOUNT) return; // exploit
			
		for(j=0; j<vsd->vend_num; ++j) {
			if( vsd->vend_list[j].amount>0 && vsd->vend_list[j].index == index )
			{
				if (amount > vsd->vend_list[j].amount)
				{	
					clif_buyvending(sd,index,vsd->vend_list[j].amount, 4);
					return;
				}
				else 
					break;
			}
		}
		if (j == vsd->vend_num)
			return; // 売り切れ

		vend_ids[i] = j;
		z += vsd->vend_list[j].value * amount;
		if (z > sd.status.zeny){
			clif_buyvending(sd, index, amount, 1);
			return; // zeny不足
		}
		w += itemdb_weight(vsd->status.cart[index].nameid) * amount;
		if (w + sd.weight > sd.max_weight) {
			clif_buyvending(sd, index, amount, 2);
			return; // 重量超過
		}
		
		if( vend_list[j].amount > vsd->status.cart[index].amount ) //Check to see if cart/vend info is in sync.
			vend_list[j].amount = vsd->status.cart[index].amount;

		// if they try to add packets (example: get twice or more 2 apples if marchand has only 3 apples).
		// here, we check cumulativ amounts
		if( vend_list[j].amount < amount )
		{	// send more quantity is not a hack (an other player can have buy items just before)
			clif_buyvending(sd, index, vsd->vend_list[j].amount, 4); // not enough quantity
			return;
		}
		else
			vend_list[j].amount -= amount;

		switch( pc_checkadditem(sd, vsd->status.cart[index].nameid, amount) ) {
		case ADDITEM_EXIST:
			break;
		case ADDITEM_NEW:
			new_++;
			if (new_ > blank)
				return;	// 種類数超過
			break;
		case ADDITEM_OVERAMOUNT:
			return; // アイテム数超過
		}
	}
	if (z < 0 || z > MAX_ZENY) { // Zeny Bug Fixed by Darkchild
		clif_tradecancelled(sd);
		clif_tradecancelled(*vsd);
		return;
	}

	pc_payzeny(sd, z);
	pc_getzeny(*vsd, z);
	for(i = 0; 8 + 4 * i < len; ++i) {
		amount = RBUFW(buffer, 4 *i);
		index =  RBUFW(buffer, 2 + 4 * i) - 2;
		//if (amount < 0) break; // tested at start of the function
		pc_additem(sd,vsd->status.cart[index],amount);
		vsd->vend_list[vend_ids[i]].amount -= amount;
		pc_cart_delitem(*vsd, index, amount, 0);
		clif_vendingreport(*vsd, index, amount);
		if(config.buyer_name)
		{
			char temp[256];
			snprintf(temp, sizeof(temp),msg_txt(MSG_S_HAS_BOUGHT), sd.status.name);
			clif_disp_onlyself(*vsd,temp);
		}
		//log added by Lupus
		if(log_config.vend > 0) {
			log_vend(sd, *vsd, index, amount, z); // for Item + Zeny. log.
			//we log ZENY only with the 1st item. Then zero it for the rest items 8).
			z = 0;
		}
	}
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
void vending_openvending(struct map_session_data &sd,unsigned short len,const char *message,int flag, unsigned char *buffer)
{
	size_t i;

	if(!pc_checkskill(sd,MC_VENDING) || !pc_iscarton(sd)) {	// cart skill and cart check [Valaris]
		sd.clif_skill_failed(MC_VENDING,0,0);
		return;
	}

	if (flag) {
		for(i = 0; (85 + 8 * i < len) && (i < MAX_VENDING); ++i) {
			sd.vend_list[i].index = RBUFW(buffer,8*i) - 2;
			sd.vend_list[i].amount= RBUFW(buffer,2+8*i);
			sd.vend_list[i].value = RBUFL(buffer,4+8*i);
			if( sd.vend_list[i].value>config.vending_max_value )
				sd.vend_list[i].value=config.vending_max_value;
			else if(sd.vend_list[i].value == 0)
				sd.vend_list[i].value = 1000000;	// auto set to 1 million [celest]
			// カート内のアイテム数と販売するアイテム数に相違があったら中止
			if(pc_cartitem_amount(sd, sd.vend_list[i].index, sd.vend_list[i].amount) < 0 || sd.vend_list[i].value < 0)
			{
				sd.clif_skill_failed(MC_VENDING, 0, 0);
				return;
			}
		}
		sd.vender_id = sd.block_list::id;
		sd.vend_num = i;
		strcpy(sd.message,message);
		if (clif_openvending(sd,sd.vender_id,sd.vend_list) > 0)
			clif_showvendingboard(sd,message,0);
		else
			sd.vender_id = 0;
	}
}

