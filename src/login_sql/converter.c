//Converter for NPC to SQL readout.

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "dbaccess.h"
#include "malloc.h"
#include "strlib.h"

struct npc_src_list {
	struct npc_src_list * next;
	struct npc_src_list * prev;
	char name[4];
} ;

struct npc_src_list *npc_src_first,*npc_src_last;

void parse_npcs(void){
	struct npc_src_list *nsl;
	FILE *fp;
	char line[1024];
	int lines;
	for(nsl=npc_src_first;nsl;nsl=nsl->next) {
		if(nsl->prev){
			free(nsl->prev);
			nsl->prev = NULL;
		}
		fp=fopen(nsl->name,"r");
		if (fp==NULL) {
			printf("file not found : %s\n",nsl->name);
			exit(1);
		}
		lines=0;
		while(fgets(line,1020,fp)) {
			char w1[1024],w2[1024],w3[1024],w4[1024],mapname[1024];
			int i,j,w4pos,count;
			lines++;

			if (line[0] == '/' && line[1] == '/')
				continue;
			// 不要なスペースやタブの連続は詰める
			for(i=j=0;line[i];i++) {
				if (line[i]==' ') {
					if (!((line[i+1] && (isspace(line[i+1]) || line[i+1]==',')) ||
						 (j && line[j-1]==',')))
						line[j++]=' ';
				} else if (line[i]=='\t') {
					if (!(j && line[j-1]=='\t'))
						line[j++]='\t';
				} else
 					line[j++]=line[i];
			}
			// 最初はタブ区切りでチェックしてみて、ダメならスペース区切りで確認
			if ((count=sscanf(line,"%[^\t]\t%[^\t]\t%[^\t\r\n]\t%n%[^\t\r\n]",w1,w2,w3,&w4pos,w4)) < 3 &&
			   (count=sscanf(line,"%s%s%s%n%s",w1,w2,w3,&w4pos,w4)) < 3) {
				continue;
			}
			// マップの存在確認
			if( strcmp(w1,"-")!=0 && strcasecmp(w1,"function")!=0 ){
				sscanf(w1,"%[^,]",mapname);
				if (strlen(mapname)>16) {
					// "mapname" is not assigned to this server
					continue;
				}
			}
			if (strcasecmp(w2,"warp")==0 && count > 3) {
				printf("\r warp found");
				/*------------------------
				NPC Warps >> SQL
				------------------------*/
				//wc1					wc2		wc3		wc4
				//alberta.gat,15,234,0	warp	alb001	2,5,pay_fild03.gat,388,63
				int x,y,xs,ys,to_x,to_y;
				char mapname[24],to_mapname[24];
				if (sscanf(w1,"%[^,],%d,%d",mapname,&x,&y) != 3 ||
				   sscanf(w4,"%d,%d,%[^,],%d,%d",&xs,&ys,to_mapname,&to_x,&to_y) != 5) {
					printf("bad warp line : %s\n",w3);
					continue;
				}
				printf("\r running query for warp");
				sql_query(
					"INSERT INTO `lege_npc_warps` (`mapname`,`x`,`y`,`xs`,`ys`,`to_mapname`,`to_x`,`to_y`) "
					"values ('%s','%d','%d','%d','%d','%s','%d','%d')",
					mapname,x,y,xs,ys,to_mapname,to_x,to_y);
			} else if (strcasecmp(w2,"shop")==0 && count > 3) {
				printf("\r shop found");
				/*------------------------
				NPC Shops >> SQL
				------------------------*/
				// wc1						wc2		wc3				wc4
				//prontera.gat,155,211,5	shop	2-2 Class Shop	86,1950:-1,1952:-1
				int x,y,dir,sprite;
				char mapname[24],shop_name[24];
				char *p;
				int t_nameid,t_value;
				int max = 100, pos = 0, l = 0;

				struct {
					int nameid;
					int value;
				} shop_item[101];

				if (sscanf(w1, "%[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
					strchr(w4, ',') == NULL) {
					printf("bad shop line : %s\n", w3);
					continue;
				}

				p = strchr(w4, ',');

				while (p && pos < max) {
					p++;
					if (sscanf(p, "%d:%d", &t_nameid, &t_value) != 2)
						break;
					shop_item[pos].nameid = t_nameid;
					shop_item[pos].value = t_value;
					pos++;
					p=strchr(p,',');
				}

				memcpy(shop_name, w3, 24);
				sprite = atoi(w4);

				sprintf(tmp_sql,"INSERT INTO `lege_npc_shops` (`mapname`,`x`,`y`,`dir`,`shop_name`,`sprite`");

				for (l=1;l<pos+1;l++)
					sprintf(tmp_sql,"%s,`item%d_id`,`item%d_value`",tmp_sql,l,l);

				sprintf(tmp_sql,"%s) VALUES ('%s','%d','%d','%d','%s','%d'",tmp_sql,mapname,x,y,dir,shop_name,sprite);

				for (l=0;l<pos;l++)
					sprintf(tmp_sql,"%s,'%d','%d'",tmp_sql,shop_item[l].nameid,shop_item[l].value);
				printf("\r running query for shop");
				sql_query("%s)",tmp_sql);

			} else if (strcasecmp(w2,"monster")==0 && count > 3) {
				/*------------------------
				NPC Mob Spawns>> SQL
				------------------------*/
				// wc1						wc2		wc3		wc4
				// prt_sewb1.gat,0,0,0,0	monster	Tarou	1175,20,0,0,0
				char mapname[24],mobname[24],eventname[24];
				int x,y,xs,ys,num,class,delay1,delay2;

				if (sscanf(w1,"%[^,],%d,%d,%d,%d",mapname,&x,&y,&xs,&ys) < 3 ||
				   sscanf(w4,"%d,%d,%d,%d,%s",&class,&num,&delay1,&delay2,eventname) < 2 ) {
					printf("bad monster line : %s\n",w3);
					continue;
				}
				memcpy(mobname,w3,24);
				sql_query(
					"INSERT INTO lege_npc_mobspawns "
					"(`mapname`,`x`,`y`,`xs`,`ys`,`mob_name`,`mob_id`,`num`,`delay1`,`delay2`,`eventname`) "
					"VALUES ('%s','%d','%d','%d','%d','%s','%d','%d','%d','%d','%s')",
					  mapname,x,y,xs,ys,jstrescape(mobname),class,num,delay1,delay2,eventname);

			} else if (strcasecmp(w2,"mapflag")==0 && count >= 3) {
				/*------------------------
				NPC Map Flags>> SQL
				------------------------*/
				//char mapname[24],flag[24];
				// wc1			wc2		wc3		wc4
				// cmd_in01.gat	mapflag	nomemo	{blank}
				//if (sscanf(w1,"%[^,]",mapname)!= 1)
					continue;
				//memcpy(flag,w3,24);
			//	sql_query("INSERT INTO `lege_npc_mapflags` (`mapname`,`flag`) VALUES ('%s','%s')",mapname,flag);
			}
		}
		fclose(fp);
		fflush(stdout);
	}
}

void npc_addsrcfile(char *name)
{
	printf("\r adding file to list: %s",name);
	struct npc_src_list *new;
	size_t len;

	len = sizeof(*new) + strlen(name);
	new=(struct npc_src_list *)aCalloc(1,len);
	new->next = NULL;
	strncpy(new->name,name,strlen(name)+1);
	if (npc_src_first==NULL)
		npc_src_first = new;
	if (npc_src_last)
		npc_src_last->next = new;

	npc_src_last=new;
}

int map_config_read(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if (fp == NULL) {
		printf("Map configuration file not found at: %s\n", cfgName);
		exit(1);
	}
	printf("Map configuration file found at: %s\n", cfgName);
	while(fgets(line, sizeof(line) -1, fp)) {
		if (line[0] == '/' && line[1] == '/') continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2){
			if (strcasecmp(w1, "npc") == 0){
				npc_addsrcfile(w2);
			} else if (strcasecmp(w1, "import") == 0) {
				map_config_read(w2);
			}
		}

	}
	fclose(fp);

	return 0;
}


int main(int argc,char **argv){

	int login_server_port = 3306;
	char login_server_ip[32] = "database.clownphobia.com";
	char login_server_id[32] = "admin";
	char login_server_pw[32] = "tehsux0rs";
	char login_server_db[32] = "lightside";

	//Start SQL server connection
	sql_connect(login_server_ip, login_server_id, login_server_pw, login_server_db, login_server_port);

	int l;
	sql_query("DROP TABLE IF EXISTS `lege_npc_shops`",NULL);
	sql_query("delete from lege_npc_mobspawns",NULL);
	sql_query("delete from lege_npc_warps",NULL);
	sql_query("DROP TABLE IF EXISTS `lege_npc_mapflags`",NULL);

	sprintf(tmp_sql,
		"CREATE TABLE `lege_npc_shops` ("
			"`mapname` VARCHAR(24) NOT NULL,"
			"`x` INTEGER UNSIGNED NOT NULL,"
			"`y` INTEGER UNSIGNED NOT NULL,"
			"`dir` INTEGER UNSIGNED NOT NULL,"
			"`shop_name` VARCHAR(24) NOT NULL,"
			"`sprite` INTEGER UNSIGNED NOT NULL,");
	for (l=1;l<101;l++)
		sprintf(tmp_sql,"%s `item_id` INTEGER UNSIGNED NOT NULL,`item%d_value` INTEGER UNSIGNED NOT NULL,",tmp_sql,l,l);
	sprintf(tmp_sql,
		"%s PRIMARY KEY(`mapname`, `x`, `y`)"
		")TYPE = InnoDB",tmp_sql);
	sql_query(tmp_sql,NULL);


	sql_query(
		"CREATE TABLE `lege_npc_mapflags` ("
			"`mapname` VARCHAR(24) NOT NULL,"
			"`flag` VARCHAR(24) NOT NULL,"
			"PRIMARY KEY(`mapname`,`flag`)"
		")TYPE = InnoDB",NULL);



	printf("Starting Conversion of NPCs to connect to SQL database\n");
	map_config_read("conf/map_athena.conf");
	parse_npcs();

	return 0;
}


