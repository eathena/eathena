// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// Logging functions by Azndragon & Codemaster
#include "baseparam.h"
#include "nullpo.h"
#include "itemdb.h"
#include "pc.h"
#include "showmsg.h"
#include "utils.h"
#include "log.h"

struct LogConfig log_config;


#if defined(WITH_MYSQL)
#include <mysql.h>


static inline int mysql_SendQuery(MYSQL *mysql, const char* q)
{
#ifdef TWILIGHT
	ShowSQL("%s:%d# %s\n", __FILE__, __LINE__, q);
#endif
	return mysql_real_query(mysql, q, strlen(q));
}


MYSQL logmysql_handle; //For the log database - fix by [Maeki]
MYSQL_RES* logsql_res ;
MYSQL_ROW  logsql_row ;

char log_db[32] = "log";
char log_db_ip[16] = "127.0.0.1";
char log_db_id[32] = "ragnarok";
char log_db_pw[32] = "ragnarok";
int log_db_port = 3306;




inline const char* escape_string(char *target, const char* source)
{
	if(source && target)
	{	// no overflow check
		mysql_real_escape_string(&logmysql_handle, target, source, basics::hstrlen(source));
		return target;
	}
	else
	{
		return "";
	}
}
#endif
//FILTER OPTIONS
//0 = Don't log
//1 = Log any item
//Bits: ||
//2 - Healing items (0)
//3 - Etc Items(3) + Arrows (10)
//4 - Usable Items(2) + Scrolls,Lures(11)
//5 - Weapon(4)
//6 - Shields,Armor,Headgears,Accessories,etc(5)
//7 - Cards(6)
//8 - Pet Accessories(8) + Eggs(7) (well, monsters don't drop 'em but we'll use the same system for ALL logs)
//9 - Log expensive items ( >= price_log)
//10 - Log big amount of items ( >= amount_log)
//11 - Log refined items (if their refine >= refine_log )
//12 - Log rare items (if their drop chance <= rare_log )

//check if this item should be logger according the settings
int should_log_item(int filter, unsigned short nameid)
{
	struct item_data *item_data;
	if(nameid<512 || (item_data= itemdb_exists(nameid)) == NULL) return 0;
	if( (filter&1) || // Filter = 1, we log any item
		(filter&2 && item_data->type == 0 ) ||	//healing items
		(filter&4 && (item_data->type == 3 || item_data->type == 10) ) ||	//etc+arrows
		(filter&8 && (item_data->type == 2 || item_data->type == 11) ) ||	//usable
		(filter&16 && item_data->type == 4 ) ||	//weapon
		(filter&32 && item_data->type == 5 ) ||	//armor
		(filter&64 && item_data->type == 6 ) ||	//cards
		(filter&128 && (item_data->type == 7 || item_data->type == 8) ) ||	//eggs+pet access
		(filter&256 && item_data->value_buy >= log_config.price_items_log )
		//|| (filter&512 && item_data->refine >= log_config.refine_items_log )
	) return item_data->nameid;

	return 0;
}

int log_branch(map_session_data &sd)
{
	if( log_config.enable_logs )
	{
#if defined(WITH_MYSQL)
		if(log_config.sql_logs)
		{
			char t_name[64];
			char tmp_sql[16384];
			snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%ld', '%ld', '%s', '%s')",
				log_config.log_branch_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), sd.mapname);
			if(mysql_SendQuery(&logmysql_handle, tmp_sql))
				ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
		}
		else
#endif
		{
			FILE *logfp;
			if((logfp=basics::safefopen(log_config.log_branch,"a+")) != NULL)
			{
				char timestring[128];
				time_t curtime;

				time(&curtime);
				strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
				fprintf(logfp,"%s - %s[%ld:%ld]\t%s"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, sd.mapname);
				fclose(logfp);
			}
		}
	}
	return 0;
}

int log_drop(map_session_data &sd, uint32 monster_id, int log_drop[])
{
	FILE *logfp;
	int i,flag = 0;

	if( !log_config.enable_logs )
		return 0;

	for (i = 0; i<10; ++i) { //Should we log these items? [Lupus]
		flag += should_log_item(log_config.drop,log_drop[i]);
	}
	if (flag==0) return 0; //we skip logging this items set - they doesn't met our logging conditions [Lupus]

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`drop_date`, `kill_char_id`, `monster_id`, `item1`, `item2`, `item3`, `item4`, `item5`, `item6`, `item7`, `item8`, `item9`, `itemCard`, `map`) VALUES (NOW(), '%ld', '%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s') ", log_config.log_drop_db, (unsigned long)sd.status.char_id, (unsigned long)monster_id, log_drop[0], log_drop[1], log_drop[2], log_drop[3], log_drop[4], log_drop[5], log_drop[6], log_drop[7], log_drop[8], log_drop[9], sd.mapname);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_drop,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;
			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%ld\t%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, (unsigned long)monster_id, log_drop[0], log_drop[1], log_drop[2], log_drop[3], log_drop[4], log_drop[5], log_drop[6], log_drop[7], log_drop[8], log_drop[9]);
			fclose(logfp);
		}
	}
	return 1; //Logged
}

int log_mvpdrop(map_session_data &sd, uint32 monster_id, int log_mvp[])
{
	FILE *logfp;

	if( !log_config.enable_logs )
		return 0;

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%ld', '%ld', '%d', '%d', '%s') ", log_config.log_mvpdrop_db, (unsigned long)sd.status.char_id, (unsigned long)monster_id, log_mvp[0], log_mvp[1], sd.mapname);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_mvpdrop,"a+")) != NULL) {
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%ld\t%d,%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, (unsigned long)monster_id, log_mvp[0], log_mvp[1]);
			fclose(logfp);
		}
	}
	return 0;
}

int log_present(map_session_data &sd, int source_type, unsigned short nameid)
{
	FILE *logfp;

	if( !log_config.enable_logs )
		return 0;

	if(!should_log_item(log_config.present,nameid)) return 0;	//filter [Lupus]
#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`present_date`, `src_id`, `account_id`, `char_id`, `char_name`, `nameid`, `map`) VALUES (NOW(), '%d', '%ld', '%ld', '%s', '%d', '%s') ",
			log_config.log_present_db, source_type, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), nameid, sd.mapname);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_present,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%d\t%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, source_type, nameid);
			fclose(logfp);
		}
	}
	return 0;
}

int log_produce(map_session_data &sd, unsigned short nameid, int slot1, int slot2, int slot3, int success)
{
	FILE *logfp;

	if( !log_config.enable_logs )
		return 0;

	if(!should_log_item(log_config.produce,nameid)) return 0;	//filter [Lupus]
#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`produce_date`, `account_id`, `char_id`, `char_name`, `nameid`, `slot1`, `slot2`, `slot3`, `map`, `success`) VALUES (NOW(), '%ld', '%ld', '%s', '%d', '%d', '%d', '%d', '%s', '%d') ",
			log_config.log_produce_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), nameid, slot1, slot2, slot3, sd.mapname, success);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_produce,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%d\t%d,%d,%d\t%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, nameid, slot1, slot2, slot3, success);
			fclose(logfp);
		}
	}
	return 0;
}

int log_refine(map_session_data &sd, int n, int success)
{
	FILE *logfp;
	int log_card[4];
	int item_level;
	int i;

	if( !log_config.enable_logs )
		return 0;

	if(success == 0)
		item_level = 0;
	else
		item_level = sd.status.inventory[n].refine + 1;
	if(!should_log_item(log_config.refine,sd.status.inventory[n].nameid)) return 0;	//filter [Lupus]
	for(i=0;i<4; ++i)
		log_card[i] = sd.status.inventory[n].card[i];

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`refine_date`, `account_id`, `char_id`, `char_name`, `nameid`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`, `success`, `item_level`) VALUES (NOW(), '%ld', '%ld', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%d', '%d')",
			log_config.log_refine_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), sd.status.inventory[n].nameid, sd.status.inventory[n].refine, log_card[0], log_card[1], log_card[2], log_card[3], sd.mapname, success, item_level);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_refine,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%d,%d\t%d%d%d%d\t%d,%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, sd.status.inventory[n].nameid, sd.status.inventory[n].refine, log_card[0], log_card[1], log_card[2], log_card[3], success, item_level);
			fclose(logfp);
		}
	}
	return 0;
}

int log_tostorage(map_session_data &sd,int n, int guild) 
{
  FILE *logfp;

  if(!log_config.enable_logs || log_config.storage == 0 || log_config.log_storage[0] == '\0')
    return 0;
	if(sd.status.inventory[n].nameid==0 || sd.inventory_data[n] == NULL)
    return 1;

	if( sd.status.inventory[n].amount > MAX_AMOUNT )//sd.status.inventory[n].amount < 0 )
    return 1;

	if((logfp=basics::safefopen(log_config.log_trade,"a+")) != NULL) {
		char timestring[128];
		time_t curtime;

    time(&curtime);
		strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - to %s: %s[%ld:%ld]\t%d\t%d\t%d\t%d,%d,%d,%d"RETCODE, 
			timestring, guild ? "guild_storage": "storage", 
			sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, 
			sd.status.inventory[n].nameid,
			sd.status.inventory[n].amount,
			sd.status.inventory[n].refine,
			sd.status.inventory[n].card[0],
			sd.status.inventory[n].card[1],
			sd.status.inventory[n].card[2],
			sd.status.inventory[n].card[3] );
    fclose(logfp);
  }
  return 0;
}

int log_fromstorage(map_session_data &sd,int n, int guild) 
{
  FILE *logfp;

  if( !log_config.enable_logs || log_config.storage == 0 || log_config.log_storage[0] == '\0')
    return 0;
	if(sd.status.inventory[n].nameid==0 || sd.inventory_data[n] == NULL)
    return 1;

	if( sd.status.inventory[n].amount > MAX_AMOUNT )//sd.status.inventory[n].amount < 0 )
    return 1;

	if((logfp=basics::safefopen(log_config.log_trade,"a+")) != NULL) {
		char timestring[128];
		time_t curtime;

    time(&curtime);
		strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		fprintf(logfp,"%s - from %s: %s[%ld:%ld]\t%d\t%d\t%d\t%d,%d,%d,%d"RETCODE, 
			timestring, guild ? "guild_storage": "storage", 
			sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, 
			sd.status.inventory[n].nameid,
			sd.status.inventory[n].amount,
			sd.status.inventory[n].refine,
			sd.status.inventory[n].card[0],
			sd.status.inventory[n].card[1],
			sd.status.inventory[n].card[2],
			sd.status.inventory[n].card[3]);
    fclose(logfp);
  }
  return 0;
}

int log_trade(map_session_data &sd, map_session_data &target_sd, int n,int amount)
{
	FILE *logfp;
	int log_nameid, log_amount, log_refine, log_card[4];
	int i;
	if( !log_config.enable_logs )
		return 0;


	if(sd.status.inventory[n].nameid==0 || amount <= 0 || sd.status.inventory[n].amount<amount || sd.inventory_data[n] == NULL)
		return 1;

	if( sd.status.inventory[n].amount > MAX_AMOUNT )//sd.status.inventory[n].amount < 0 )
		return 1;
	if(!should_log_item(log_config.trade,sd.status.inventory[n].nameid)) return 0;	//filter [Lupus]
	log_nameid = sd.status.inventory[n].nameid;
	log_amount = sd.status.inventory[n].amount;
	log_refine = sd.status.inventory[n].refine;

	for(i=0;i<4; ++i)
		log_card[i] = sd.status.inventory[n].card[i];

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64],t_name2[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`trade_date`, `src_account_id`, `src_char_id`, `src_char_name`, `des_account_id`, `des_char_id`, `des_char_name`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`) VALUES (NOW(), '%ld', '%ld', '%s', '%ld', '%ld', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s')",
			log_config.log_trade_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), (unsigned long)target_sd.status.account_id, (unsigned long)target_sd.status.char_id, escape_string(t_name2, target_sd.status.name), log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], sd.mapname);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_trade,"a+")) != NULL) {
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%s[%ld:%ld]\t%d\t%d\t%d\t%d,%d,%d,%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, target_sd.status.name, (unsigned long)target_sd.status.account_id, (unsigned long)target_sd.status.char_id, log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3]);
			fclose(logfp);
		}
	}
	return 0;
}

int log_vend(map_session_data &sd,map_session_data &vsd,int n,int amount, int zeny)
{
	FILE *logfp;
	int log_nameid, log_amount, log_refine, log_card[4];
	int i;
	if( !log_config.enable_logs )
		return 0;

	if(sd.status.inventory[n].nameid==0 || amount <= 0 || sd.status.inventory[n].amount<amount || sd.inventory_data[n] == NULL)
		return 1;
	if( sd.status.inventory[n].amount > MAX_AMOUNT )//sd.status.inventory[n].amount < 0 )
		return 1;
	if(!should_log_item(log_config.vend,sd.status.inventory[n].nameid)) return 0;	//filter [Lupus]
	log_nameid = sd.status.inventory[n].nameid;
	log_amount = sd.status.inventory[n].amount;
	log_refine = sd.status.inventory[n].refine;
	for(i=0;i<4; ++i)
		log_card[i] = sd.status.inventory[n].card[i];

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64],t_name2[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`vend_date`, `vend_account_id`, `vend_char_id`, `vend_char_name`, `buy_account_id`, `buy_char_id`, `buy_char_name`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`, `zeny`) VALUES (NOW(), '%ld', '%ld', '%s', '%ld', '%ld', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%d')",
			log_config.log_vend_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), (unsigned long)vsd.status.account_id, (unsigned long)vsd.status.char_id, escape_string(t_name2, vsd.status.name), log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], sd.mapname, zeny);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_vend,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld:%ld]\t%s[%ld:%ld]\t%d\t%d\t%d\t%d,%d,%d,%d\t%d"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, vsd.status.name, (unsigned long)vsd.status.account_id, (unsigned long)vsd.status.char_id, log_nameid, log_amount, log_refine, log_card[0], log_card[1], log_card[2], log_card[3], zeny);
			fclose(logfp);
		}
	}
	return 0;
}

int log_zeny(map_session_data &sd, map_session_data &target_sd,int amount)
{
	FILE *logfp;

	if( !log_config.enable_logs )
		return 0;

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64],t_name2[64];
		snprintf(tmp_sql, sizeof(tmp_sql),"INSERT DELAYED INTO `%s` (`trade_date`, `src_account_id`, `src_char_id`, `src_char_name`, `des_account_id`, `des_char_id`, `des_char_name`, `map`, `zeny`) VALUES (NOW(), '%ld', '%ld', '%s', '%ld', '%ld', '%s', '%s', '%ld')",
			log_config.log_trade_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), (unsigned long)target_sd.status.account_id, (unsigned long)target_sd.status.char_id, escape_string(t_name2, target_sd.status.name), sd.mapname, (unsigned long)sd.deal_zeny);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_trade,"a+")) != NULL)
		{
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld]\t%s[%ld]\t%ld"RETCODE, timestring, sd.status.name, (unsigned long)sd.status.account_id, target_sd.status.name, (unsigned long)target_sd.status.account_id, (unsigned long)sd.deal_zeny);
			fclose(logfp);
		}
	}
	return 0;
}

int log_atcommand(const map_session_data &sd, const char *message, unsigned cmdlvl)
{
	if( !log_config.enable_logs || cmdlvl < log_config.gmlevel )
		return 0;

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64];
		char t_msg[100]; //These are the contents of an @ call, so there shouldn't be overflow danger here?

		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES(NOW(), '%ld', '%ld', '%s', '%s', '%s') ",
			log_config.log_gm_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), sd.mapname, escape_string(t_msg, (char *)message));
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		FILE *logfp;
		if((logfp=basics::safefopen(log_config.log_gm,"a+")) != NULL) {
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld]: %s"RETCODE,timestring,sd.status.name,(unsigned long)sd.status.account_id,message);
			fclose(logfp);
		}
	}
	return 0;
}

int log_npc(map_session_data &sd, const char *message)
{	//[Lupus]
	FILE *logfp;

	if( !log_config.enable_logs )
		return 0;

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_name[64];
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES(NOW(), '%ld', '%ld', '%s', '%s', '%s') ",
			log_config.log_npc_db, (unsigned long)sd.status.account_id, (unsigned long)sd.status.char_id, escape_string(t_name, sd.status.name), sd.mapname, message);
		if(mysql_SendQuery(&logmysql_handle, tmp_sql))
			ShowError("DB server Error - %s\n",mysql_error(&logmysql_handle));
	}
	else
#endif
	{
		if((logfp=basics::safefopen(log_config.log_npc,"a+")) != NULL) {
			char timestring[128];
			time_t curtime;

			time(&curtime);
			strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(logfp,"%s - %s[%ld]: %s"RETCODE,timestring,sd.status.name,(unsigned long)sd.status.account_id,message);
			fclose(logfp);
		}
	}
	return 0;
}

//ChatLogging
// Log CHAT (currently only: Party, Guild, Whisper)
// LOGGING FILTERS [Lupus]
//=============================================================
//0 = Don't log at all
//1 = Log any chat messages
//Advanced Filter Bits: ||
//2 - Log Whisper messages
//3 - Log Party messages
//4 - Log Guild messages
//5 - Log Common messages (not implemented)
//6 - Don't log when WOE is on
//Example:
//log_chat: 1	= logs ANY messages
//log_chat: 6	= logs both Whisper & Party messages
//log_chat: 8	= logs only Guild messages
//log_chat: 18	= logs only Whisper, when WOE is off
int log_chat(const char *type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char *dst_charname, const char *message)
{
	FILE *logfp;
	//Check ON/OFF
	if(log_config.chat <= 0)
		return 0; //Deactivated

#if defined(WITH_MYSQL)
	if(log_config.sql_logs)
	{
		char tmp_sql[16384];
		char t_msg[100]; //The chat line, 100 should be high enough above overflow...
		snprintf(tmp_sql, sizeof(tmp_sql), "INSERT DELAYED INTO `%s` (`time`, `type`, `type_id`, `src_charid`, `src_accountid`, `src_map`, `src_map_x`, `src_map_y`, `dst_charname`, `message`) VALUES (NOW(), '%s', '%d', '%d', '%d', '%s', '%d', '%d', '%s', '%s')", 
		 	log_config.log_chat_db, type, type_id, src_charid, src_accid, mapname, x, y, dst_charname, escape_string(t_msg, message));
	
		if(mysql_SendQuery(&logmysql_handle, tmp_sql)){
			ShowError("log_chat() -> SQL ERROR / FAIL: %s\n", mysql_error(&logmysql_handle));
			return -1;	
		}else{
			return 0;
		}
	}
	else if( (logfp = fopen(log_config.log_chat, "a+")) != NULL)
#else
	if( (logfp = fopen(log_config.log_chat, "a+")) != NULL )
#endif
	{	
		char timestring[128];
		time_t curtime;
		time(&curtime);
		strftime(timestring, 127, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
		//DATE - type,type_id,src_charid,src_accountid,src_map,src_x,src_y,dst_charname,message
		fprintf(logfp, "%s - %s,%d,%d,%d,%s,%d,%d,%s,%s%s", 
			timestring, type, type_id, src_charid, src_accid, mapname, x, y, dst_charname, message, RETCODE);
		fclose(logfp);
		return 0;
	}
	return -1;
}















void log_set_defaults(void)
{
	memset(&log_config, 0, sizeof(log_config));

	//LOG FILTER Default values
	log_config.refine_items_log = 7; //log refined items, with refine >= +7
	log_config.rare_items_log = 100; //log rare items. drop chance <= 1%
	log_config.price_items_log = 1000; //1000z
	log_config.amount_items_log = 100;	
}

int log_config_read(const char *cfgName)
{
	static int count = 0;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if ((count++) == 0)
		log_set_defaults();		

	if((fp = basics::safefopen(cfgName, "r")) == NULL)
	{
		ShowError("Log configuration '"CL_WHITE"%s"CL_RESET"' not found.\n", cfgName);
		return 1;
	}
	
	while(fgets(line, sizeof(line), fp))
	{
		if( prepare_line(line) && 2==sscanf(line, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) )
		{
			basics::itrim(w1);
			if(!*w1) continue;
			basics::itrim(w2);

			if(strcasecmp(w1,"enable_logs") == 0)
			{
				log_config.enable_logs = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1,"sql_logs") == 0)
			{
				log_config.sql_logs = basics::config_switch<bool>(w2);
//start of common filter settings
			}
			else if(strcasecmp(w1,"rare_items_log") == 0)
			{
				log_config.rare_items_log = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"refine_items_log") == 0)
			{
				log_config.refine_items_log = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"price_items_log") == 0)
			{
				log_config.price_items_log = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"amount_items_log") == 0)
			{
				log_config.amount_items_log = basics::config_switch<int>(w2);
//end of common filter settings
			}
			else if(strcasecmp(w1,"log_branch") == 0)
			{
				log_config.branch = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_drop") == 0)
			{
				log_config.drop = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_mvpdrop") == 0)
			{
				log_config.mvpdrop = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_present") == 0)
			{
				log_config.present = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_produce") == 0)
			{
				log_config.produce = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_refine") == 0)
			{
				log_config.refine = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_trade") == 0)
			{
				log_config.trade = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_storage") == 0)
			{
				log_config.storage = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_vend") == 0)
			{
				log_config.vend = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_zeny") == 0)
			{
				log_config.zeny = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"log_gm") == 0)
			{				
				log_config.gmlevel = basics::config_switch<int>(w2,0,100);
			}
			else if(strcasecmp(w1,"log_npc") == 0)
			{
				log_config.npc = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1, "log_chat") == 0)
			{
				log_config.chat = basics::config_switch<int>(w2);
			}

#if defined(WITH_MYSQL)
			else if(strcasecmp(w1, "log_branch_db") == 0)
			{
				safestrcpy(log_config.log_branch_db, sizeof(log_config.log_branch_db), w2);
				if(log_config.branch == 1)
					ShowInfo("Logging Dead Branch Usage to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_drop_db") == 0)
			{
				safestrcpy(log_config.log_drop_db, sizeof(log_config.log_drop_db), w2);
				if(log_config.drop == 1)
					ShowInfo("Logging Item Drops to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_mvpdrop_db") == 0)
			{
				safestrcpy(log_config.log_mvpdrop_db, sizeof(log_config.log_mvpdrop_db), w2);
				if(log_config.mvpdrop == 1)
					ShowInfo("Logging MVP Drops to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_present_db") == 0)
			{
				safestrcpy(log_config.log_present_db, sizeof(log_config.log_present_db), w2);
				if(log_config.present == 1)
					ShowInfo("Logging Present Usage & Results to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_produce_db") == 0)
			{
				safestrcpy(log_config.log_produce_db, sizeof(log_config.log_produce_db), w2);
				if(log_config.produce == 1)
					ShowInfo("Logging Producing to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_refine_db") == 0)
			{
				safestrcpy(log_config.log_refine_db, sizeof(log_config.log_refine_db), w2);
				if(log_config.refine == 1)
					ShowInfo("Logging Refining to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_trade_db") == 0)
			{
				safestrcpy(log_config.log_trade_db, sizeof(log_config.log_trade_db), w2);
				if(log_config.trade == 1)
				{
					ShowInfo("Logging Item Trades");
					if(log_config.zeny == 1)
						ShowMessage("and Zeny Trades");
					ShowMessage(" to table `%s`\n", w2);
				}
			}
//			else if(strcasecmp(w1, "log_storage_db") == 0)
//			{
//				safestrcpy(log_config.log_storage_db, sizeof(log_config.log_storage_db), w2);
//				if(log_config.storage == 1)
//				{
//					ShowInfo("Logging Item Storages");
//					ShowMessage(" to table `%s`\n", w2);
//				}
//			}
			else if(strcasecmp(w1, "log_vend_db") == 0)
			{
				safestrcpy(log_config.log_vend_db, sizeof(log_config.log_vend_db), w2);
				if(log_config.vend == 1)
					ShowInfo("Logging Vending to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_gm_db") == 0)
			{
				safestrcpy(log_config.log_gm_db, sizeof(log_config.log_gm_db), w2);
				if(log_config.gmlevel < 100)
					ShowInfo("Logging GM Level %d Commands to table `%s`\n", log_config.gmlevel, w2);
			}
			else if(strcasecmp(w1, "log_npc_db") == 0)
			{
				safestrcpy(log_config.log_npc_db, sizeof(log_config.log_npc_db), w2);
				if(log_config.npc > 0)
					ShowInfo("Logging NPC 'logmes' to table `%s`\n", w2);
			}
			else if(strcasecmp(w1, "log_chat_db") == 0)
			{
				safestrcpy(log_config.log_chat_db, sizeof(log_config.log_chat_db), w2);
				if(log_config.chat > 0)
					ShowInfo("Logging CHAT to table `%s`\n", w2);
			}
			else if(strcasecmp(w1,"log_db")==0)
			{
				safestrcpy(log_db, sizeof(log_db), w2);
			}
			else if(strcasecmp(w1,"log_db_ip")==0)
			{
				safestrcpy(log_db_ip, sizeof(log_db_ip), w2);
			}
			else if(strcasecmp(w1,"log_db")==0)
			{
				safestrcpy(log_db, sizeof(log_db), w2);
			}
			else if(strcasecmp(w1,"log_db_id")==0)
			{
				safestrcpy(log_db_id, sizeof(log_db_id), w2);
			}
			else if(strcasecmp(w1,"log_db_pw")==0)
			{
				safestrcpy(log_db_pw, sizeof(log_db_pw), w2);
			}
			else if(strcasecmp(w1,"log_db_port")==0)
			{
				log_db_port = atoi(w2);
			}
#endif
			else if(strcasecmp(w1, "log_branch_file") == 0)
			{
				safestrcpy(log_config.log_branch, sizeof(log_config.log_branch), w2);
				if(log_config.branch > 0 && !log_config.sql_logs )
					ShowInfo("Logging Dead Branch Usage to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_drop_file") == 0)
			{
				safestrcpy(log_config.log_drop, sizeof(log_config.log_drop), w2);
				if(log_config.drop > 0 && !log_config.sql_logs )
					ShowInfo("Logging Item Drops to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_mvpdrop_file") == 0)
			{
				safestrcpy(log_config.log_mvpdrop, sizeof(log_config.log_mvpdrop), w2);
				if(log_config.mvpdrop > 0 && !log_config.sql_logs )
					ShowInfo("Logging MVP Drops to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_present_file") == 0)
			{
				safestrcpy(log_config.log_present, sizeof(log_config.log_present), w2);
				if(log_config.present > 0 && !log_config.sql_logs )
					ShowInfo("Logging Present Usage & Results to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_produce_file") == 0)
			{
				safestrcpy(log_config.log_produce, sizeof(log_config.log_produce), w2);
				if(log_config.produce > 0 && !log_config.sql_logs )
					ShowInfo("Logging Producing to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_refine_file") == 0)
			{
				safestrcpy(log_config.log_refine, sizeof(log_config.log_refine), w2);
				if(log_config.refine > 0 && !log_config.sql_logs )
					ShowInfo("Logging Refining to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_trade_file") == 0)
			{
				safestrcpy(log_config.log_trade, sizeof(log_config.log_trade), w2);
				if(log_config.trade > 0 && !log_config.sql_logs )
				{
					ShowInfo("Logging Item Trades");
					if(log_config.zeny > 0)
						ShowMessage("and Zeny Trades");
					ShowMessage(" to file `%s`.txt\n", w2);
				}
			}
			else if(strcasecmp(w1, "log_storage_file") == 0)
			{
				safestrcpy(log_config.log_storage, sizeof(log_config.log_storage), w2);
				if(log_config.storage > 0 && !log_config.sql_logs )
				{
					ShowInfo("Logging Item Storages");
					ShowMessage(" to file `%s`.txt\n", w2);
				}
			}
			else if(strcasecmp(w1, "log_vend_file") == 0)
			{
				safestrcpy(log_config.log_vend, sizeof(log_config.log_vend), w2);
				if(log_config.vend > 0  && !log_config.sql_logs)
					ShowInfo("Logging Vending to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_gm_file") == 0)
			{
				safestrcpy(log_config.log_gm, sizeof(log_config.log_gm), w2);
				if(log_config.gmlevel < 100 && !log_config.sql_logs )
					ShowInfo("Logging GM Level %d Commands to file `%s`.txt\n", log_config.gmlevel, w2);
			}
			else if(strcasecmp(w1, "log_npc_file") == 0)
			{
				safestrcpy(log_config.log_npc, sizeof(log_config.log_npc), w2);
				if(log_config.npc > 0 && !log_config.sql_logs )
					ShowInfo("Logging NPC 'logmes' to file `%s`.txt\n", w2);
			}
			else if(strcasecmp(w1, "log_chat_file") == 0)
			{
				safestrcpy(log_config.log_chat, sizeof(log_config.log_chat), w2);
				if(log_config.chat > 0 && !log_config.sql_logs )
					ShowInfo("Logging CHAT to file `%s`.txt\n", w2);
			//support the import command, just like any other config
			}
			else if(strcasecmp(w1,"import") == 0)
			{
				log_config_read(w2);
			}
		}
	}
	fclose(fp);
	ShowStatus("Done Reading Log Configuration '"CL_WHITE"%s"CL_RESET"'.\n", cfgName);
	return 0;
}






int log_final(void)
{
#if defined(WITH_MYSQL)
	mysql_close(&logmysql_handle);
	ShowMessage("Close Log DB Connection....\n");
#endif
	return 0;
}

int log_init(const char *cfgName)
{
	log_config_read(cfgName);

#if defined(WITH_MYSQL)
	if( log_config.sql_logs && (log_config.branch || log_config.drop || log_config.mvpdrop ||
		log_config.present || log_config.produce || log_config.refine || log_config.trade))
	{
		mysql_init(&logmysql_handle);

		//DB connection start
		ShowMessage(""CL_WHITE"[SQL]"CL_RESET": Connecting to Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",log_db,log_db_ip);
		if(!mysql_real_connect(&logmysql_handle, log_db_ip, log_db_id, log_db_pw,
			log_db ,log_db_port, (char *)NULL, 0)) {
				//pointer check
				ShowError(""CL_WHITE"[SQL Error]"CL_RESET": %s\n"
						CL_SPACE"error not recoverable, quitting.\n",
						mysql_error(&logmysql_handle));
				exit(1);
		} else {
			ShowStatus(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_BT_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", log_db);
		}
	}
#endif
	return 0;
}


