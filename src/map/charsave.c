#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/mmo.h"

#include "charsave.h"
#include "map.h"

#ifndef TXT_ONLY

struct mmo_charstatus *charsave_loadchar(int charid){
         int i, friends;
         struct mmo_charstatus *c;
         friends = 0;

         c = (struct mmo_charstatus *)aMalloc(sizeof(struct mmo_charstatus));

         if(charid <= 0){
         	eprintf("charsave_loadchar() charid <= 0! (%d)", charid);
                 aFree(c);
         	return NULL;
         }

	//Tested, Mysql 4.1.9+ has no problems with the long query, the buf is 65k big and the sql server needs for it 0.00009 secs on an athlon xp 2400+ WinXP (1GB Mem) ..  [Sirius]
         sprintf(charsql_tmpsql, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, `str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, `option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`, `clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, `last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`, `partner_id`, `father`, `mother`, `child`, `fame` FROM `char` WHERE `char_id` = '%d'", charid);
    	if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsave_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
         	return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(mysql_num_rows(charsql_res) <= 0){
         	printf("WARNING -> charsave_loadchar() -> CHARA NOT FOUND! (id: %d)\n", charid);
         	mysql_free_result(charsql_res);
                 aFree(c);
         	return NULL;
         }

         //fetch data
         charsql_row = mysql_fetch_row(charsql_res);

         //fill with data
         c->char_id = charid;
         c->account_id = atoi(charsql_row[1]);
         c->char_num = atoi(charsql_row[2]);
         strcpy(c->name, charsql_row[3]);
         c->class_ = atoi(charsql_row[4]);
         c->base_level = atoi(charsql_row[5]);
         c->job_level = atoi(charsql_row[6]);
         c->base_exp = atoi(charsql_row[7]);
         c->job_exp = atoi(charsql_row[8]);
         c->zeny = atoi(charsql_row[9]);
         c->str = atoi(charsql_row[10]);
         c->agi = atoi(charsql_row[11]);
         c->vit = atoi(charsql_row[12]);
         c->int_ = atoi(charsql_row[13]);
         c->dex = atoi(charsql_row[14]);
         c->luk = atoi(charsql_row[15]);
         c->max_hp = atoi(charsql_row[16]);
         c->hp = atoi(charsql_row[17]);
         c->max_sp = atoi(charsql_row[18]);
         c->sp = atoi(charsql_row[19]);
         c->status_point = atoi(charsql_row[20]);
         c->skill_point = atoi(charsql_row[21]);
         c->option = atoi(charsql_row[22]);
         c->karma = atoi(charsql_row[23]);
         c->manner = atoi(charsql_row[24]);
         c->party_id = atoi(charsql_row[25]);
         c->guild_id = atoi(charsql_row[26]);
         c->pet_id = atoi(charsql_row[27]);
         c->hair = atoi(charsql_row[28]);
         c->hair_color = atoi(charsql_row[29]);
         c->clothes_color = atoi(charsql_row[30]);
         c->weapon = atoi(charsql_row[31]);
         c->shield = atoi(charsql_row[32]);
         c->head_top = atoi(charsql_row[33]);
         c->head_mid = atoi(charsql_row[34]);
         c->head_bottom = atoi(charsql_row[35]);
    	strcpy(c->last_point.map, charsql_row[36]);
         c->last_point.x = atoi(charsql_row[37]);
         c->last_point.y = atoi(charsql_row[38]);
    	strcpy(c->save_point.map, charsql_row[39]);
         c->save_point.x = atoi(charsql_row[40]);
         c->save_point.y = atoi(charsql_row[41]);
         c->partner_id = atoi(charsql_row[42]);
         c->father = atoi(charsql_row[43]);
         c->mother = atoi(charsql_row[44]);
         c->child = atoi(charsql_row[45]);
         c->fame = atoi(charsql_row[46]);

        	mysql_free_result(charsql_res);

         //Check for '0' Savepoint / LastPoint
	if (c->last_point.x == 0 || c->last_point.y == 0 || c->last_point.map[0] == '\0'){
		strcpy(c->last_point.map, "prontera.gat");
                 c->last_point.x = 100;
                 c->last_point.y = 100;
         }

	if (c->save_point.x == 0 || c->save_point.y == 0 || c->save_point.map[0] == '\0'){
                strcpy(c->save_point.map, "prontera.gat");
                c->save_point.x = 100;
                c->save_point.y = 100;
         }


	//read the memo points
         sprintf(charsql_tmpsql, "SELECT `memo_id`, `char_id`, `map`, `x`, `y` FROM `memo` WHERE `char_id` = '%d' ORDER BY `memo_id`", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsave_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 strcpy(c->memo_point[i].map, charsql_row[2]);
	                 c->memo_point[i].x = atoi(charsql_row[3]);
	                 c->memo_point[i].y = atoi(charsql_row[4]);
	         }
		mysql_free_result(charsql_res);
         }


         //read inventory...
         sprintf(charsql_tmpsql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` FROM `inventory` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
	if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 c->inventory[i].id = atoi(charsql_row[0]);
	                 c->inventory[i].nameid = atoi(charsql_row[1]);
	                 c->inventory[i].amount = atoi(charsql_row[2]);
	                 c->inventory[i].equip = atoi(charsql_row[3]);
	                 c->inventory[i].identify = atoi(charsql_row[4]);
	                 c->inventory[i].refine = atoi(charsql_row[5]);
	                 c->inventory[i].attribute = atoi(charsql_row[6]);
	                 c->inventory[i].card[0] = atoi(charsql_row[7]);
	                 c->inventory[i].card[1] = atoi(charsql_row[8]);
	                 c->inventory[i].card[2] = atoi(charsql_row[9]);
	                 c->inventory[i].card[3] = atoi(charsql_row[10]);
	         }
	         mysql_free_result(charsql_res);
         }


         //cart inventory ..
         sprintf(charsql_tmpsql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` FROM `cart_inventory` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
	if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 c->cart[i].id = atoi(charsql_row[0]);
	                 c->cart[i].nameid = atoi(charsql_row[1]);
	                 c->cart[i].amount = atoi(charsql_row[2]);
	                 c->cart[i].equip = atoi(charsql_row[3]);
	                 c->cart[i].identify = atoi(charsql_row[4]);
	                 c->cart[i].refine = atoi(charsql_row[5]);
	                 c->cart[i].attribute = atoi(charsql_row[6]);
	                 c->cart[i].card[0] = atoi(charsql_row[7]);
	                 c->cart[i].card[1] = atoi(charsql_row[8]);
	                 c->cart[i].card[2] = atoi(charsql_row[9]);
	                 c->cart[i].card[3] = atoi(charsql_row[10]);
	         }
	         mysql_free_result(charsql_res);
         }


         //Skills...
         sprintf(charsql_tmpsql, "SELECT `char_id`, `id`, `lv` FROM `skill` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
         	while((charsql_row = mysql_fetch_row(charsql_res))){
                         i = atoi(charsql_row[1]);
                         c->skill[i].id = i;
                         c->skill[i].lv = atoi(charsql_row[2]);
                 }
                 mysql_free_result(charsql_res);
         }

         //Global REG
         sprintf(charsql_tmpsql, "SELECT `char_id`, `str`, `value` FROM `global_reg_value` WHERE `type` = '3' AND `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
         	for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
                 	strcpy(c->global_reg[i].str, charsql_row[1]);
                         c->global_reg[i].value = atoi(charsql_row[2]);
                 }
                 mysql_free_result(charsql_res);
         }


         //Friend list 'ids'
         sprintf(charsql_tmpsql, "SELECT `char_id`, `friend_id` FROM `friends` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, charsql_tmpsql)){
         	eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
                 aFree(c);
                 return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
         	for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
                 	c->friend_id[i] = atoi(charsql_row[1]);
                         //NEW ONE:
                         //c->friends[i].id = atoi(charsql_res[1]);
                 }
                 friends = i;
                 mysql_free_result(charsql_res);
         }


        	//Friend list 'names'
         for(i = 0; i < friends; i++){
         	sprintf(charsql_tmpsql, "SELECT `char_id`, `name` FROM `char` WHERE `char_id` = '%d'", c->friend_id[i]);
                 //NEW
                 //sprintf(charsql_tmpsql, "SELECT `char_id`, `name` FROM `char` WHERE `char_id` = '%d'", c->friends[i].id);
	        if(mysql_query(&charsql_handle, charsql_tmpsql)){
	                 eprintf("charsql_loadchar() SQL ERROR: %s\n", mysql_error(&charsql_handle));
	                 aFree(c);
	                 return NULL;
	        }

                 charsql_res = mysql_store_result(&charsql_handle);
                 if(charsql_res){
                 	charsql_row = mysql_fetch_row(charsql_res);
                         strcpy(c->friend_name[i], charsql_row[1]);
                         //NEW:
                         //strcpy(c->friends[i].name, charsql_row[1]);
                 	mysql_free_result(charsql_res);
                 }


         }

      	printf("charsql_loadchar(): loading of '%d' (%s) complete.\n", charid, c->name);

return c;
}





int charsave_savechar(int charid, struct mmo_charstatus *c){
	printf("SAVE REQ: %d NAME: %s\n", charid, c->name);
 	printf("currently charsave is incomplete ! the char will not be saved, sirius will continue it 18.6.05\n");

         return 0;
}

#endif

