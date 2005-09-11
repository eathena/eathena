#include "dbaccess.h"
#include <stdio.h>
#include <stdlib.h>
/*
SELECT i.subtrans_member_id,i.subtrans_cumulative FROM `ibf_subscription_trans` i, where i.subtrans_method = 'paypal' and i.subtrans_state = 'paid' order by i.subtrans_cumulative DESC
SELECT i.subtrans_member_id,i.subtrans_cumulative,im.name FROM `ibf_subscription_trans` i, `ibf_members` im where i.subtrans_member_id = im.id and i.subtrans_method = 'paypal' and i.subtrans_state = 'paid'
SELECT i.subtrans_member_id,i.subtrans_cumulative,im.name FROM `ibf_subscription_trans` i, `ibf_members` im where i.subtrans_member_id = im.id and i.subtrans_method = 'paypal' order by i.subtrans_cumulative DESC

*/


main(){
	int i;
	char stuff[256];

	FILE *rp;
	char npc_list[256] = "npc_list.c";

	FILE *fp;
	char donor_list[256] = "donorlist.sql";

	sql_connect("127.0.0.1","admin","tehsux0rs","lightside",3306);
	fp = fopen(donor_list, "a");
	rp = fopen(npc_list, "a");

	if (fp == NULL || rp == NULL) {
		printf("Donor maker cant make donations  heheheh oops?!!!\n");
		exit(1);
	} else {

		sql_query("SELECT i.subtrans_member_id,i.subtrans_cumulative,im.name FROM `ibf_subscription_trans` i, `ibf_members` im where i.subtrans_member_id = im.id and i.subtrans_method = 'paypal' order by i.subtrans_cumulative DESC",NULL);


		if ((sql_res = mysql_store_result(&mysql_handle))) {
			for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){

				if (atoi(sql_row[1])>=1 && atoi(sql_row[1])<10) sprintf(stuff,"(%d,603,2,1)",atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=10 && atoi(sql_row[1])<20)
					sprintf(stuff,"(%d,603,2,1),(%d,617,1,1)",atoi(sql_row[0]),atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=20 && atoi(sql_row[1])<30)
					sprintf(stuff,"(%d,617,2,1),(%d,616,1,1)",atoi(sql_row[0]),atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=30 && atoi(sql_row[1])<40)
					sprintf(stuff,"(%d,617,2,1),(%d,616,1,1)",atoi(sql_row[0]),atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=40 && atoi(sql_row[1])<50)
					sprintf(stuff,"(%d,616,2,1),(%d,11500,1,1)",atoi(sql_row[0]),atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=50 && atoi(sql_row[1])<100)
					sprintf(stuff,"(%d,616,3,1),(%d,11500,1,1)",atoi(sql_row[0]),atoi(sql_row[0]));
				else if (atoi(sql_row[1])>=100)
					sprintf(stuff,"(%d,616,5,1),(%d,11500,2,1)",atoi(sql_row[0]),atoi(sql_row[0]));

				else { printf("something went wrong, id = %d accumulated amount = %d \n",atoi(sql_row[0]),atoi(sql_row[1])); exit(1);}

				fprintf(fp, "INSERT INTO `lege_storage` (`account_id`,`nameid`,`amount`,`identify`) VALUES %s; \n",stuff);
				fprintf(rp, "%d \t %d \t %s\n",atoi(sql_row[1]), atoi(sql_row[0]),sql_row[2] );


			}
		}

		fclose(fp);
		fclose(rp);
	}

	sql_close();
	return 0;
}

// 603 obb
// 617 opb
// 616 oca
// 11500 prize bag

