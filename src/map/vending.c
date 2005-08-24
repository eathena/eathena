// $Id: vending.c,v 1.2 2004/09/25 05:32:19 MouseJstr Exp $
#include <stdio.h>
#include <string.h>

#include "../common/nullpo.h"
#include "clif.h"
#include "itemdb.h"
#include "atcommand.h"
#include "map.h"
#include "chrif.h"
#include "vending.h"
#include "pc.h"
#include "skill.h"
#include "battle.h"
#include "log.h"

/*==========================================
 * 露店閉鎖
 *------------------------------------------
*/
void vending_closevending(struct map_session_data *sd)
{

	nullpo_retv(sd);

	sd->vender_id=0;
	clif_closevendingboard(&sd->bl,0);
}

/*==========================================
 * 露店アイテムリスト要求
 *------------------------------------------
 */
void vending_vendinglistreq(struct map_session_data *sd,int id)
{
	struct map_session_data *vsd;

	nullpo_retv(sd);

	if( (vsd=map_id2sd(id)) == NULL )
		return;
	if(vsd->vender_id==0)
		return;
	clif_vendinglist(sd,id,vsd->vending);
}

/*==========================================
 * 露店アイテム購入
 *------------------------------------------
 */
void vending_purchasereq(struct map_session_data *sd,int len,int id,unsigned char *p)
{
	int i, j, w, z, new_ = 0, blank, vend_list[12];
	short amount, index;
	struct map_session_data *vsd = map_id2sd(id);

	nullpo_retv(sd);

	blank = pc_inventoryblank(sd);

	if (vsd == NULL)
		return;
	if (vsd->vender_id == 0)
		return;
	if (vsd->vender_id == sd->bl.id)
		return;
	for(i = 0, w = z = 0; 8 + 4 * i < len; i++) {
		amount = *(short*)(p + 4 * i);
		index = *(short*)(p + 2 + 4 * i) - 2;

		if (amount < 0) return; // exploit
/*		for(j = 0; j < vsd->vend_num; j++)
			if (0 < vsd->vending[j].amount && amount <= vsd->vending[j].amount && vsd->vending[j].index == index)
				break;
*/
//ADD_start
		for(j = 0; j < vsd->vend_num; j++) {
			if (0 < vsd->vending[j].amount && vsd->vending[j].index == index) {
				if (amount > vsd->vending[j].amount || amount <= 0) {
					clif_buyvending(sd,index,vsd->vending[j].amount, 4);
					return;
				}
				break;
			}
		}
//ADD_end
		if (j == vsd->vend_num)
			return; // 売り切れ
		vend_list[i] = j;
		z += vsd->vending[j].value * amount;
		if (z > sd->status.zeny){
			clif_buyvending(sd, index, amount, 1);
			return; // zeny不足
		}
		w += itemdb_weight(vsd->status.cart[index].nameid) * amount;
		if (w + sd->weight > sd->max_weight) {
			clif_buyvending(sd, index, amount, 2);
			return; // 重量超過
		}
		switch(pc_checkadditem(sd, vsd->status.cart[index].nameid, amount)) {
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
		clif_tradecancelled(vsd);
		return;
	}

	pc_payzeny(sd, z);
	pc_getzeny(vsd, z);
	for(i = 0; 8 + 4 * i < len; i++) {
		amount = *(short*)(p + 4 *i);
		index = *(short*)(p + 2 + 4 * i) - 2;
		//if (amount < 0) break; // tested at start of the function
		pc_additem(sd,&vsd->status.cart[index],amount);
		vsd->vending[vend_list[i]].amount -= amount;
		pc_cart_delitem(vsd, index, amount, 0);
		clif_vendingreport(vsd, index, amount);
		if(battle_config.buyer_name) {
			char temp[256];
			sprintf(temp, msg_txt(265), sd->status.name);
			clif_disp_onlyself(vsd,temp,strlen(temp));
		}

		//log added by Lupus
		if(log_config.vend > 0) {
			log_vend(sd,vsd, index, amount, z); // for Item + Zeny. log.
			//we log ZENY only with the 1st item. Then zero it for the rest items 8).
			z = 0;
		}
	}

	if (sd->state.autotrade)
	{	//check for @AUTOTRADE users [durf]
		chrif_save(vsd);
		storage_storage_save(vsd);
	}
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
void vending_openvending(struct map_session_data *sd,int len,char *message,int flag,unsigned char *p)
{
	int i, j;

	nullpo_retv(sd);

	if(!pc_checkskill(sd,MC_VENDING) || !pc_iscarton(sd)) {	// cart skill and cart check [Valaris]
		clif_skill_fail(sd,MC_VENDING,0,0);
		return;
	}

	if (flag) {
		for(i = 0, j = 0; (85 + 8 * j < len) && (i < MAX_VENDING); i++, j++) {
			sd->vending[i].index = *(short*)(p+8*j)-2;
			if (sd->vending[i].index < 0 || sd->vending[i].index >= MAX_INVENTORY ||
				!itemdb_cantrade(sd->status.inventory[sd->vending[i].index].nameid, pc_isGM(sd), pc_isGM(sd)))
			{
				i--; //Preserve the vending index, skip to the next item.
				continue;
			}
			sd->vending[i].amount = *(short*)(p+2+8*j);
			sd->vending[i].value = *(int*)(p+4+8*j);
			if(sd->vending[i].value > battle_config.vending_max_value)
				sd->vending[i].value=battle_config.vending_max_value;
			else if(sd->vending[i].value == 0)
				sd->vending[i].value = 1000000;	// auto set to 1 million [celest]
			// カート内のアイテム数と販売するアイテム数に相違があったら中止
			if(pc_cartitem_amount(sd, sd->vending[i].index, sd->vending[i].amount) < 0 || sd->vending[i].value < 0) { // fixes by Valaris and fritz
				clif_skill_fail(sd, MC_VENDING, 0, 0);
				return;
			}
		}
		if (i != j)
		{	//Some items were not vended. [Skotlex]
			clif_displaymessage (sd->fd, msg_txt(266));
		}
		sd->vender_id = sd->bl.id;
		sd->vend_num = i;
		memcpy(sd->message,message, MESSAGE_SIZE-1);
		if (clif_openvending(sd,sd->vender_id,sd->vending) > 0)
			clif_showvendingboard(&sd->bl,message,0);
		else
			sd->vender_id = 0;
	}
}

