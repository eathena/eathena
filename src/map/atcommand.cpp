// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "socket.h"
#include "timer.h"
#include "nullpo.h"
#include "mmo.h"
#include "db.h"
#include "core.h"
#include "showmsg.h"
#include "utils.h"


#include "log.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"


///////////////////////////////////////////////////////////////////////////////
//
// command functions
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///
/// addwarp
/// Create a new static warp point from current position to given position
/// addwarp mapname, x, y
///
bool command_addwarp(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()< 3 )
	{
		clif_displaymessage(fd, "Please, enter a valid parameters (usage: addwarp <mapname> <x> <y>).");
	}
	else
	{
		char output[128];
		char w1[64], w3[64], w4[64];
		
		const char *mapname=param[0];
		int x=param[1];
		int y=param[2];

		snprintf(w1,sizeof(w1), "%s,%d,%d", sd.mapname, sd.block_list::x, sd.block_list::y);
		snprintf(w3,sizeof(w3), "%s%d%d%d%d", mapname,sd.block_list::x, sd.block_list::y, x, y);
		snprintf(w4,sizeof(w4), "1,1,%s.gat,%d,%d", mapname, x, y);

		if( npc_parse_warp(w1, "warp", w3, w4) )
		{
			snprintf(output, sizeof(output), "New warp NPC => %s",w3);
			clif_displaymessage(fd, output);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// adjcmdlvl
/// Temp adjust the GM level required to use a GM command
/// Used during beta testing to allow players to use GM commands
/// for short periods of time
///
bool command_adjcmdlvl(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<2 )
	{
		clif_displaymessage(fd, "Usage: adjcmdlvl <command> <lvl>.");
	}
	else
	{
		const char *name = param[0];
		int newlev       = param[1];
		CommandInfo& cmd = CommandInfo::byname(name);
		if(cmd.func == NULL)
		{
			clif_displaymessage(fd, "command not found.");
		}
		else if( cmd.level < newlev && sd.isGM() < newlev && param[2]!="-force" )
		{
			clif_displaymessage(fd, "you try to increase the level over your priveledges.");
			clif_displaymessage(fd, "add \"-force\" to override.");
		}
		else if( sd.isGM() < cmd.level )
		{
			clif_displaymessage(fd, "your authorisation is not sufficient.");
		}
		else
		{
			cmd.level = newlev;
			clif_displaymessage(fd, "command level changed.");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// adjgmlvl
/// Change GM level
///
bool command_adjgmlvl(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int newlev      = param[0];
    const char *user= param[1];
	struct map_session_data *psd;

    if( param.size()<2 )
	{
		clif_displaymessage(fd, "Usage: adjgmlvl <lvl> <user>.");
	}
	else if( sd.status.gm_level <= newlev )
	{
		clif_displaymessage(fd, "You cannot grant higher or equal gm_level than your own.");
	}
	else if( NULL==(psd = CommandInfo::param2sd(user)) )
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
	}
	else if( psd==&sd || sd.isGM() <= psd->isGM() )
	{	// not authorized
		clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION));	
	}
	else
	{
		psd->status.gm_level = newlev;
		clif_displaymessage(psd->fd, "Your gm_level has been changed.");
		clif_displaymessage(fd, "gm_level changed.");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "adopt <player1> <player2> <player3>"
/// adopt a novice
///
bool command_adopt(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	struct map_session_data *pl_sd1 = NULL;
	struct map_session_data *pl_sd2 = NULL;
	struct map_session_data *pl_sd3 = NULL;
	const char *player1 = param[0];
	const char *player2 = param[1];
	const char *player3 = param[2];

	if( param.size()<3 )
	{
		clif_displaymessage(fd, "usage: adopt <player1> <player2> <player3>.");
	}
	else if( (pl_sd1=CommandInfo::param2sd(player1)) == NULL )
	{
		snprintf(output, sizeof(output), "Cannot find player %s online", player1);
		clif_displaymessage(fd, output);
	}
	else if( (pl_sd2=CommandInfo::param2sd(player2)) == NULL )
	{
		snprintf(output, sizeof(output), "Cannot find player %s online", player2);
		clif_displaymessage(fd, output);
	}
	else if( (pl_sd3=CommandInfo::param2sd(player3)) == NULL )
	{
		snprintf(output, sizeof(output), "Cannot find player %s online", player3);
		clif_displaymessage(fd, output);
	}
	else if((pl_sd1->status.base_level < 70) || (pl_sd2->status.base_level < 70))
	{
		clif_displaymessage(fd, "They are too young to be parents!");
	}
	else if( pc_adoption(*pl_sd1, *pl_sd2, *pl_sd3) )
	{
		ShowInfo("Adopting: --%s--%s--%s--\n", player1, player2, player3);
		clif_displaymessage(fd, "They are family.. wish them luck");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
///
bool command_agitend(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( !agit_flag )
	{
		clif_displaymessage(fd, msg_txt(MSG_NOT_SIEGE)); // Siege warfare hasn't started yet.
	}
	else
	{
		agit_flag = 0;
		guild_agit_end();
		clif_displaymessage(fd, msg_txt(MSG_SIEGE_END)); // Guild siege warfare end!
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
///
bool command_agitstart(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( agit_flag )
	{
		clif_displaymessage(fd, msg_txt(MSG_ALREADY_SIEGE)); // Already it has started siege warfare.
	}
	else
	{
		agit_flag = 1;
		guild_agit_start();
		clif_displaymessage(fd, msg_txt(MSG_SIEGE_START)); // Guild siege warfare start!
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
///
bool command_alive(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( sd.is_dead() )
	{
		sd.status.hp = sd.status.max_hp;
		sd.status.sp = sd.status.max_sp;
		clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
		sd.set_stand();
		if (config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, config.pc_invincible_time);
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		clif_resurrection(sd, 1);
		clif_displaymessage(sd.fd, msg_txt(MSG_YOUVE_BEEN_REVIVED)); // You've been revived! It's a miracle!
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_allskill(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_allskillup(sd); // all skills
	sd.status.skill_point = 0; // 0 skill points
	clif_updatestatus(sd, SP_SKILLPOINT); // update
	clif_displaymessage(sd.fd, msg_txt(MSG_YOU_RECEIVED_ALL_SKILLS)); // You have received all skills.
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Turns on/off AutoLoot
///
bool command_autoloot(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(sd.state.autoloot)
	{
		sd.state.autoloot = 0;
		clif_displaymessage(sd.fd, "Autoloot is now off.");
	}
	else
	{
		sd.state.autoloot = 1;
		clif_displaymessage(sd.fd, "Autoloot is now on.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_baselevelup(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int level=0, i;
	if( param.size()<1 || (level=param[0])==0 )
	{
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: lvup/blevel/baselvlup <number of levels>).");
	}
	else if( level >0 )
	{	
		if( sd.status.base_level >= config.maximum_level)
		{	// check for max level
			clif_displaymessage(fd, msg_txt(MSG_BASE_LV_MAX)); // Base level can't go any higher.
		}
		else
		{
			// fix positiv overflow
			if( (size_t)level > config.maximum_level || (size_t)level > (config.maximum_level - sd.status.base_level))
				level = config.maximum_level - sd.status.base_level;
			for(i=1; i<=level; ++i)
				sd.status.status_point += (sd.status.base_level + i + 14) / 5;
			sd.status.base_level += level;
			clif_updatestatus(sd, SP_BASELEVEL);
			clif_updatestatus(sd, SP_NEXTBASEEXP);
			clif_updatestatus(sd, SP_STATUSPOINT);
			status_calc_pc(sd, 0);
			sd.heal(sd.status.max_hp, sd.status.max_sp);
			clif_misceffect(sd, 0);
			clif_displaymessage(sd.fd, msg_txt(MSG_BASE_LV_RAISED)); // Base level raised.
			return true;
		}
	}
	else
	{
		if (sd.status.base_level <= 1)
		{
			clif_displaymessage(fd, msg_txt(MSG_BASE_LEVEL_CANT_GO_ANY_LOWER)); // Base level can't go any lower.
		}
		else
		{
			if((size_t)(-level) > config.maximum_level || sd.status.base_level < (size_t)(1 - level))
				level = 1 - sd.status.base_level;
			if( sd.status.status_point > 0 )
			{
				int sp = sd.status.status_point;
				for (i = 0; i > level; i--)
					sp -= (sd.status.base_level + i + 14) / 5;
				sd.status.status_point = (sp < 0) ? 0 : sp;
				clif_updatestatus(sd, SP_STATUSPOINT);
			} // to add: remove status points from stats
			sd.status.base_level += level;
			clif_updatestatus(sd, SP_BASELEVEL);
			clif_updatestatus(sd, SP_NEXTBASEEXP);
			status_calc_pc(sd, 0);
			clif_displaymessage(sd.fd, msg_txt(MSG_BASE_LV_LOWERED)); // Base level lowered.
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// broadcast
///
bool command_broadcast(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a message (usage: broadcast <message>).");
	}
	else
	{
		size_t sz=1+snprintf(output, sizeof(output), "%s : %s", sd.status.name, param.line());
		intif_GMmessage(output, sz,0);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// localbroadcast
///
bool command_localbroadcast(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a message (usage: localbroadcast <message>).");
		return false;
	}
	else
	{
		char output[128];
		size_t sz=1+snprintf(output, sizeof(output), "%s : %s", sd.status.name, param.line());
		clif_GMmessage(&sd, output, sz, 1);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  changelook
///
bool command_changelook(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	static int pos[6] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES };
	int j = param[0];
	int k = param[1];
	if( param.size() < 1 )
	{
		clif_displaymessage(fd, "Usage: changelook [<position>] <view id>");
		clif_displaymessage(fd, "Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield");
		return false;
	}
	else if (param.size() == 1)
	{	// position not defined, use HEAD_TOP as default
		k = j;	// swap
		j = LOOK_HEAD_TOP;
	}
	else if (param.size() >= 2)
	{
		if(j < 1)
			j = 1;
		else if (j > 6)
			j = 6;	// 6 = Shoes - for beta clients only perhaps
		j = pos[j - 1];
	}
	clif_changelook(sd,j,k);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "changesex [player_name]"
///
bool command_changesex(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char* name = (param.size()<1) ? sd.status.name : (const char*)param[0];
	if(strlen(name) < 4)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MIN_4_CHARS)); // Sorry, but a player name have at least 4 characters.
	}
	else if(strlen(name) > 23)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MAX_23_CHARS)); // Sorry, but a player name have 23 characters maximum.
	}
	else
	{
		chrif_char_ask_name(sd.status.account_id, name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_txt(MSG_NAME_SEND_TO_CHAR_SERVER)); // Character name sends to char-server to ask it.
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// cartlist: Displays the items list of a cart.
///
bool command_cartlist(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	char outputtmp[256];
	struct item_data *item_data, *item_temp;
	size_t i, j, count, counter, counter2;

	counter = 0;
	count = 0;
	for(i=0; i<MAX_CART; ++i)
	{
		if(sd.status.cart[i].nameid > 0 && (item_data = itemdb_exists(sd.status.cart[i].nameid)) != NULL)
		{
			counter = counter + sd.status.cart[i].amount;
			count++;
			if (count == 1) {
				snprintf(output, sizeof(output), "------ Cart items list of '%s' ------", sd.status.name);
				clif_displaymessage(fd, output);
			}
			if(sd.status.cart[i].refine)
				snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d)", sd.status.cart[i].amount, item_data->name, sd.status.cart[i].refine, item_data->jname, sd.status.cart[i].refine, sd.status.cart[i].nameid);
			else
				snprintf(output, sizeof(output), "%d %s (%s, id: %d)", sd.status.cart[i].amount, item_data->name, item_data->jname, sd.status.cart[i].nameid);
			clif_displaymessage(fd, output);
			*output = '\0';
			counter2 = 0;
			for (j = 0; j < item_data->flag.slot; ++j) {
				if (sd.status.cart[i].card[j]) {
					if ( (item_temp = itemdb_exists(sd.status.cart[i].card[j])) != NULL) {
						if(output[0] == '\0')
							snprintf(outputtmp, sizeof(outputtmp), " -> (card(s): #%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
						else
							snprintf(outputtmp, sizeof(outputtmp), "#%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
						strcat(output, outputtmp);
					}
				}
			}
			if(output[0] != '\0') {
				output[strlen(output) - 2] = ')';
				output[strlen(output) - 1] = '\0';
				clif_displaymessage(fd, output);
			}
		}
	}
	if (count == 0)
		clif_displaymessage(fd, "No item found in the cart of this player.");
	else
	{
		snprintf(output, sizeof(output), "%ld item(s) found in %ld kind(s) of items.", (unsigned long)(counter), (unsigned long)(count));
		clif_displaymessage(fd, output);
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// itemlist : Displays the list of items.
///
bool command_itemlist(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[256], equipstr[128], outputtmp[256];
	struct item_data *item_data, *item_temp;
	size_t i, j;
	size_t equip, count = 0, counter = 0, counter2;

	for(i=0; i<MAX_INVENTORY; ++i)
	{
		if( sd.status.inventory[i].nameid > 0 && (item_data = itemdb_exists(sd.status.inventory[i].nameid)) != NULL)
		{
			counter += sd.status.inventory[i].amount;
			++count;
			if (count == 1)
			{
				snprintf(output, sizeof(output), "------ Items list of '%s' ------", sd.status.name);
				clif_displaymessage(fd, output);
			}

			*equipstr='\0';
			if( (equip = sd.status.inventory[i].equip) )
			{
				strcpy(equipstr, "| equiped: ");
				if (equip & 4)
					strcat(equipstr, "robe/gargment, ");
				if (equip & 8)
					strcat(equipstr, "left accessory, ");
				if (equip & 16)
					strcat(equipstr, "body/armor, ");
				if ((equip & 34) == 2)
					strcat(equipstr, "right hand, ");
				if ((equip & 34) == 32)
					strcat(equipstr, "left hand, ");
				if ((equip & 34) == 34)
					strcat(equipstr, "both hands, ");
				if (equip & 64)
					strcat(equipstr, "feet, ");
				if (equip & 128)
					strcat(equipstr, "right accessory, ");
				if ((equip & 769) == 1)
					strcat(equipstr, "lower head, ");
				if ((equip & 769) == 256)
					strcat(equipstr, "top head, ");
				if ((equip & 769) == 257)
					strcat(equipstr, "lower/top head, ");
				if ((equip & 769) == 512)
					strcat(equipstr, "mid head, ");
				if ((equip & 769) == 512)
					strcat(equipstr, "lower/mid head, ");
				if ((equip & 769) == 769)
					strcat(equipstr, "lower/mid/top head, ");
				// remove final ', '
				equipstr[strlen(equipstr) - 2] = '\0';
			}
			if (sd.status.inventory[i].refine)
				snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d) %s", sd.status.inventory[i].amount, item_data->name, sd.status.inventory[i].refine, item_data->jname, sd.status.inventory[i].refine, sd.status.inventory[i].nameid, equipstr);
			else
				snprintf(output, sizeof(output), "%d %s (%s, id: %d) %s", sd.status.inventory[i].amount, item_data->name, item_data->jname, sd.status.inventory[i].nameid, equipstr);
			clif_displaymessage(fd, output);

			counter2 = 0;
			for(*output='\0',j=0; j<item_data->flag.slot; ++j)
			{
				if (sd.status.inventory[i].card[j])
				{
					if ((item_temp = itemdb_exists(sd.status.inventory[i].card[j])) != NULL)
					{
						if(output[0] == '\0')
							snprintf(outputtmp, sizeof(outputtmp), " -> (card(s): #%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
						else
							snprintf(outputtmp, sizeof(outputtmp), "#%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
						strcat(output, outputtmp);
					}
				}
			}
			if(*output)
			{
				output[strlen(output) - 2] = ')';
				output[strlen(output) - 1] = '\0';
				clif_displaymessage(fd, output);
			}
		}
	}
	if (count == 0)
		clif_displaymessage(fd, "No item found on this player.");
	else
	{
		snprintf(output, sizeof(output), "%ld item(s) found in %ld kind(s) of items.", (unsigned long)(counter), (unsigned long)(count));
		clif_displaymessage(fd, output);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// storagelist : Displays the list of items in storage.
///
bool command_storagelist(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct pc_storage *stor;
	
	struct item_data *item_data, *item_temp;
	size_t i, j, count, counter, counter2;
	char output[256], outputtmp[256];
	
	if( (stor=account2storage2(sd.status.account_id)) != NULL )
	{
		counter = 0;
		count = 0;
		for (i=0; i<MAX_STORAGE; ++i)
		{
			if (stor->storage[i].nameid > 0 && (item_data = itemdb_exists(stor->storage[i].nameid)) != NULL)
			{
				counter = counter + stor->storage[i].amount;
				count++;
				if (count == 1)
				{
					snprintf(output, sizeof(output), "------ Storage items list of '%s' ------", sd.status.name);
					clif_displaymessage(fd, output);
				}
				if (stor->storage[i].refine)
					snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d)", stor->storage[i].amount, item_data->name, stor->storage[i].refine, item_data->jname, stor->storage[i].refine, stor->storage[i].nameid);
				else
					snprintf(output, sizeof(output), "%d %s (%s, id: %d)", stor->storage[i].amount, item_data->name, item_data->jname, stor->storage[i].nameid);
				clif_displaymessage(fd, output);
				memset(output, '\0', sizeof(output));
				counter2 = 0;
				for (j = 0; j < item_data->flag.slot; ++j)
				{
					if (stor->storage[i].card[j])
					{
						if ((item_temp = itemdb_exists(stor->storage[i].card[j])) != NULL)
						{
							if (output[0] == '\0')
								snprintf(outputtmp, sizeof(outputtmp), " -> (card(s): #%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
							else
								snprintf(outputtmp, sizeof(outputtmp), "#%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
							strcat(output, outputtmp);
						}
					}
				}
				if (output[0] != '\0')
				{
					output[strlen(output) - 2] = ')';
					output[strlen(output) - 1] = '\0';
					clif_displaymessage(fd, output);
				}
			}
		}
		if (count == 0)
			clif_displaymessage(fd, "No item found in the storage of this player.");
		else {
			snprintf(output, sizeof(output), "%ld item(s) found in %ld kind(s) of items.", (unsigned long)(counter), (unsigned long)(count));
			clif_displaymessage(fd, output);
		}
	}
	else
	{
		clif_displaymessage(fd, "no storage.");
		return true;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charban command (usage: "charban <time> <player_name>")
/// This command do a limited ban on a player
/// Time is done as follows:
///   Adjustment value (-1, 1, +1, etc...)
///   Modified element:
///     a or y: year
///     m:  month
///     j or d: day
///     h:  hour
///     mn: minute
///     s:  second
/// <example> ban +1m-2mn1s-6y test_player
///           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
///
bool command_char_ban(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<2)
	{
		clif_displaymessage(fd, "Please, enter ban time and a player name (usage: charban/ban/banish/charbanish <time> <name>).");
		return false;
	}
	const char *ipp = param[0];
	const char *player_name= param[1];
	int year, month, day, hour, minute, second, value;

	year = month = day = hour = minute = second = 0;
	for(value=0; *ipp; ++ipp)
	{
		if(*ipp >= '0' && *ipp <= '9')
		{
			value = 10*value + *ipp - '0';
		}
		else
		{
			if (*ipp == 's')
			{
				second = value;
			}
			else if (*ipp == 'm' && ipp[1] == 'n')
			{
				minute = value;
			}
			else if (*ipp == 'h')
			{
				hour = value;
			}
			else if (*ipp == 'd' || *ipp == 'j')
			{
				day = value;
			}
			else if (*ipp == 'm')
			{
				month = value;
			}
			else if (*ipp == 'y' || *ipp == 'a')
			{
				year = value;
			}
			value = 0;
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_BAN_TIME)); // Invalid time for ban command.
		return false;
	}

	// check player name
	if(strlen(player_name) < 4)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MIN_4_CHARS)); // Sorry, but a player name have at least 4 characters.
		return false;
	}
	else if(strlen(player_name) > 23)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MAX_23_CHARS)); // Sorry, but a player name have 23 characters maximum.
		return false;
	}
	else
	{
		chrif_char_ask_name(sd.status.account_id, player_name, 2, year, month, day, hour, minute, second); // type: 2 - ban
		clif_displaymessage(fd, msg_txt(MSG_NAME_SEND_TO_CHAR_SERVER)); // Character name sends to char-server to ask it.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charblock command (usage: "charblock <player_name>")
/// This command do a definitiv ban on a player
///
bool command_char_block(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: charblock/block <name>).");
		return false;
	}
	const char *player_name = param[0];

	// check player name
	if(strlen(player_name) < 4)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MIN_4_CHARS)); // Sorry, but a player name have at least 4 characters.
		return false;
	}
	else if(strlen(player_name) > 23)
	{
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MAX_23_CHARS)); // Sorry, but a player name have 23 characters maximum.
		return false;
	}
	else
	{
		chrif_char_ask_name(sd.status.account_id, player_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
		clif_displaymessage(fd, msg_txt(MSG_NAME_SEND_TO_CHAR_SERVER)); // Character name sends to char-server to ask it.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "delitem <item_name_or_ID> <quantity>"
/// removes given number of item from a character
/// (item can be equiped or not).
///
bool command_delitem(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	int i, item_position, count;
	unsigned short item_id;
	struct item_data *item_data;

	const char *item_name  = param[0];
	int number             = param[1];
	
	if( param.size()<3 || number < 1)
	{
		clif_displaymessage(fd, "Please, enter an item name/id, a quantity and a player name (usage: delitem <item_name_or_ID> <quantity> [player]).");
		return false;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if( item_id<500 )
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_ITEM)); // Invalid item ID or name.
	}
	else if( (item_position = pc_search_inventory(sd, item_id)) < 0 )
	{
		clif_displaymessage(fd, msg_txt(MSG_CHARACTER_DOES_NOT_HAVE_THE_ITEM)); // Character does not have the item.
	}
	else
	{
		count = 0;
		for(i=0; i<number && item_position >= 0; ++i)
		{
			pc_delitem(sd, item_position, 1, 0);
			count++;
			item_position = pc_search_inventory(sd, item_id); // for next loop
		}
		snprintf(output, sizeof(output), msg_txt(MSG_D_ITEM_REMOVED_BY_A_GM), count); // %d item(s) removed by a GM.
		clif_displaymessage(sd.fd, output);
		if (number == count)
			snprintf(output, sizeof(output), msg_txt(MSG_D_ITEM_REMOVED_FROM_THE_PLAYER), count); // %d item(s) removed from the player.
		else
			snprintf(output, sizeof(output), msg_txt(MSG_D_ITEM_REMOVED_HAD_ONLY_D_ON_D), count, count, number); // %d item(s) removed. Player had only %d on %d items.
		clif_displaymessage(fd, output);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charunban command (usage: "charunban <player_name>")
///
bool command_char_unban(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: charunban <player_name>).");
		return false;
	}
	const char *player_name = param[0];

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MIN_4_CHARS)); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MAX_23_CHARS)); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd.status.account_id, player_name, 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban
		clif_displaymessage(fd, msg_txt(MSG_NAME_SEND_TO_CHAR_SERVER)); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charunblock command (usage: "charunblock <player_name>")
///
bool command_char_unblock(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: charunblock <player_name>).");
		return false;
	}
	const char *player_name = param[0];

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MIN_4_CHARS)); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_txt(MSG_PLY_NAME_MAX_23_CHARS)); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd.status.account_id, player_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
		clif_displaymessage(fd, msg_txt(MSG_NAME_SEND_TO_CHAR_SERVER)); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// cleanmap
///
class CAtCleanMap : public CMapProcessor
{
public:
	CAtCleanMap()	{}
	~CAtCleanMap()	{}
	virtual int process(block_list& bl) const
	{
		map_clearflooritem(bl.id);
		return 0;
	}
};

bool command_cleanmap(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	block_list::foreachinarea( CAtCleanMap(),
		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE*2, ((int)sd.block_list::y)-AREA_SIZE*2, ((int)sd.block_list::x)+AREA_SIZE*2, ((int)sd.block_list::y)+AREA_SIZE*2, BL_ITEM);
	clif_displaymessage(fd, "All dropped items have been cleaned up.");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clearing Weather Effects
///
bool command_clearweather(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		maps[m].flag.rain=0;
		maps[m].flag.snow=0;
		maps[m].flag.sakura=0;
		maps[m].flag.clouds=0;
		maps[m].flag.clouds2=0;
		maps[m].flag.fog=0;
		maps[m].flag.fireworks=0;
		maps[m].flag.leaves=0;
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clouds appear.
///
bool command_clouds(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.clouds)
		{
			maps[m].flag.clouds=0;
			clif_displaymessage(fd, "The clouds has gone.");
		}
		else
		{
			maps[m].flag.clouds=1;
			clif_displaymessage(fd, "Clouds appear.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_clouds2(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.clouds2)
		{
			maps[m].flag.clouds2=0;
			clif_displaymessage(fd, "The clouds has gone.");
		}
		else
		{
			maps[m].flag.clouds2=1;
			clif_displaymessage(fd, "Clouds appear.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// It is made to rain.
///
bool command_rain(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.rain)
		{
			maps[m].flag.rain=0;
			clif_displaymessage(fd, "The rain has stopped.");
		}
		else
		{
			maps[m].flag.rain=1;
			clif_displaymessage(fd, "It is made to rain.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// It is made to snow.
///
bool command_snow(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.snow)
		{
			maps[m].flag.snow=0;
			clif_displaymessage(fd, "Snow has stopped falling.");
		}
		else
		{
			maps[m].flag.snow=1;
			clif_displaymessage(fd, "It is made to snow.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Cherry tree snowstorm is made to fall. (Sakura)
///
bool command_sakura(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.sakura)
		{
			maps[m].flag.sakura=0;
			clif_displaymessage(fd, "Cherry tree leaves are gone.");
		}
		else
		{
			maps[m].flag.sakura=1;
			clif_displaymessage(fd, "Cherry tree leaves is made to fall.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Fog hangs over.
///
bool command_fog(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.fog)
		{
			maps[m].flag.fog=0;
			clif_displaymessage(fd, "The fog has gone.");
		}
		else
		{
			maps[m].flag.fog=1;
			clif_displaymessage(fd, "Fog hangs over.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Fallen leaves fall.
///
bool command_leaves(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.leaves)
		{
			maps[m].flag.leaves=0;
			clif_displaymessage(fd, "Leaves no longer fall.");
		}
		else
		{
			maps[m].flag.leaves=1;
			clif_displaymessage(fd, "Fallen leaves fall.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clouds appear.
///
bool command_fireworks(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char mapname[32];
	size_t m = ( param.size() ) ? (buffer2mapname(mapname, sizeof(mapname), param[0]),map_mapname2mapid(mapname)):sd.block_list::m;
	if(m<map_num)
	{
		if(maps[m].flag.fireworks)
		{
			maps[m].flag.fireworks=0;
			clif_displaymessage(fd, "Fireworks have burned down.");
		}
		else
		{
			maps[m].flag.fireworks=1;
			clif_displaymessage(fd, "Fireworks are launched.");
		}
		clif_updateweather(m);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_night(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(!daynight_flag)
	{
		map_daynight_timer(-1, 0, 0, 1);
		return true;
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_ALREADY_NIGHT)); // Sorry, it's already the night. Impossible to execute the command.
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_day(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(daynight_flag)
	{
		map_daynight_timer(-1, 0, 0, 0);
		return true;
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_ALREADY_DAY)); // Sorry, it's already the day. Impossible to execute the command.
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///
///  Mail System commands
///
bool command_checkmail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	return chrif_mail_check(sd, true);
}

bool command_listmail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	return chrif_mail_fetch(sd, true);
}

bool command_listnewmail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	return chrif_mail_fetch(sd, false);
}

bool command_readmail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int index;
	if (param.size()<1) {
		clif_displaymessage(sd.fd,"You must specify a message number.");
		return true;
	}
	index = param[0];
	if (index < 1) {
		clif_displaymessage(sd.fd,"Message number cannot be negative or zero.");
		return 0;
	}
	return chrif_mail_read(sd, index);
}

bool command_deletemail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int index;
	if (param.size()<1) {
		clif_displaymessage(sd.fd,"You must specify a message number.");
		return true;
	}
	index = param[0];
	if (index < 1) {
		clif_displaymessage(sd.fd,"Message number cannot be negative or zero.");
		return 0;
	}
	return chrif_mail_delete(sd, index);
}

bool command_sendmail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<2)
	{
		clif_displaymessage(sd.fd,"You must specify a recipient and a message.");
		return true;
	}
	
	const char* line = param.line();
	char name[32],text[128];
	if( (sscanf(line, "\"%32[^\"]\" %128[^\n]", name, text) < 2) &&
		(sscanf(line, "%32s %128[^\n]", name, text) < 2))
	{
		clif_displaymessage(fd,"You must specify a recipient and a message.");
		return true;
	}
	return chrif_mail_send(sd, name, "", text);
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_die(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	clif_specialeffect(sd,450,1);
	pc_damage(sd, sd.status.hp + 1,NULL);
	clif_displaymessage(sd.fd, msg_txt(MSG_A_PITY_YOUVE_DIED)); // A pity! You've died.
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// usage: "disguise <mob_id>"
///
bool command_disguise(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned short mob_id = 0;

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: disguise <monster_name_or_monster_ID>).");
		return false;
	}

	mob_id = mobdb_searchname(param[0]);
	if( mob_id == 0 ) // check name first (to avoid possible name begining
		mob_id = param[0];

	if( (mob_id >=  46 && mob_id <= 125) || // NPC
		(mob_id >= 700 && mob_id <= 858) || // NPC
	    (mob_id > 1000 && mob_id < 1582) ) // monsters
	{
		sd.stop_walking(0);
		clif_clearchar(sd, 0);
		sd.disguise_id = mob_id;
		clif_changeoption(sd);
		clif_spawnpc(sd);
		clif_displaymessage(fd, msg_txt(MSG_DISGUISE_APPLIED)); // Disguise applied.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG__NAMEID_HASNT_BEEN_FOUND)); // Monster/NPC name/id hasn't been found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// DisguiseAll
///
bool command_disguiseall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned short mob_id=0;
	size_t i=0;
	struct map_session_data *pl_sd;


	if (param.size()<1) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: disguiseall <monster_name_or_monster_ID>).");
		return false;
	}

	if ((mob_id = mobdb_searchname(param[0])) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = param[0];

	if ((mob_id >=  46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 718) || // NPC
	    (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || // NPC
	    (mob_id >= 813 && mob_id <= 834) || // NPC
	    (mob_id > 1000 && mob_id < 1582)) { // monsters
		for(i=0; i < fd_max; ++i) {
			if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
			{
				pl_sd->stop_walking(0);
				clif_clearchar(*pl_sd, 0);
				pl_sd->disguise_id = mob_id;
				clif_changeoption(*pl_sd);
				clif_spawnpc(*pl_sd);
			}
		}
		clif_displaymessage(fd, msg_txt(MSG_DISGUISE_APPLIED)); // Disguise applied.
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// undisguise
///
bool command_undisguise(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(sd.disguise_id)
	{
		sd.stop_walking(0);
		clif_clearchar(sd, 0);
		sd.disguise_id = 0;
		clif_changeoption(sd);
		clif_spawnpc(sd);
		clif_displaymessage(fd, msg_txt(MSG_UNDISGUISE_APPLIED)); // Undisguise applied.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_YOURE_NOT_DISGUISED)); // You're not disguised.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// UndisguiseAll
///
bool command_undisguiseall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;
	for(i=0; i < fd_max; ++i)
	{
		if(session[i] && (pl_sd = (struct map_session_data *)session[i]->user_session) && pl_sd->state.auth && pl_sd->disguise_id)
		{
			pl_sd->stop_walking(0);
			clif_clearchar(*pl_sd, 0);
			pl_sd->disguise_id = 0;
			clif_changeoption(*pl_sd);
			clif_spawnpc(*pl_sd);
		}
	}
	clif_displaymessage(fd, msg_txt(MSG_UNDISGUISE_APPLIED)); // Undisguise applied.

	return true;
}



///////////////////////////////////////////////////////////////////////////////
///
/// Hand a ring with partners name on it to this char
///
void getring(struct map_session_data &sd)
{
	int flag;
	unsigned short item_id = (sd.status.sex==0) ? 2635 : 2634;
	struct item item_tmp(item_id);
	
	item_tmp.card[0] = 0x00FF;
	item_tmp.card[1] = 0;
	item_tmp.card[2] = basics::GetWord(sd.status.partner_id, 0);
	item_tmp.card[3] = basics::GetWord(sd.status.partner_id, 1);

	flag = pc_additem(sd, item_tmp, 1);
	if( flag>0 )
	{
		clif_additem(sd,0,0,flag);
		map_addflooritem(item_tmp,1,sd.block_list::m,sd.block_list::x,sd.block_list::y,NULL,NULL,NULL,0);
	}
}

///////////////////////////////////////////////////////////////////////////////
///
/// marry
/// Marry two players
///
bool command_marry(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<2)
	{
		clif_displaymessage(fd, "Usage: marry <player1> <player2>.");
		return false;
	}

	struct map_session_data *pl_sd1, *pl_sd2;
	const char* player1=param[0];
	const char* player2=param[1];

	if( (pl_sd1=CommandInfo::param2sd(player1))== NULL )
	{	
		char output[128];
		snprintf(output, sizeof(output), "Cannot find player '%s' online", player1);
		clif_displaymessage(fd, output);
	}
	else if( (pl_sd2=CommandInfo::param2sd(player2))== NULL )
	{	
		char output[128];
		snprintf(output, sizeof(output), "Cannot find player '%s' online", player2);
		clif_displaymessage(fd, output);
	}
	else if( pc_marriage(*pl_sd1, *pl_sd2) )
	{
		clif_displaymessage(fd, "They are married.. wish them well");
		clif_wedding_effect(sd);	//wedding effect and music
		// Auto-give named rings (Aru)
		getring(*pl_sd1);
		getring(*pl_sd2);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// divorce
/// divorce two players
///
bool command_divorce(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Usage: divorce <player>.");
		return false;
	}
	
	char output[128];
	const char *player_name=param[0];
	struct map_session_data *pl_sd = NULL;

	if( (pl_sd=CommandInfo::param2sd(player_name)) != NULL )
	{
		if( !pc_divorce(*pl_sd) )
		{
			snprintf(output, sizeof(output), "The divorce has failed.. Cannot find player '%s' or his(her) partner online.", player_name);
			clif_displaymessage(fd, output);
			return false;
		}
		else
		{
			snprintf(output, sizeof(output), "'%s' and his(her) partner are now divorced.", player_name);
			clif_displaymessage(fd, output);
			return true;
		}
	}
	snprintf(output, sizeof(output), "Cannot find player '%s' online", player_name);
	clif_displaymessage(fd, output);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_doom(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;

	clif_specialeffect(sd,450,3);
	for(i = 0; i < fd_max; ++i)
	{
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && i != (size_t)fd &&
		    sd.isGM() >= pl_sd->isGM())
		{
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
			clif_displaymessage(pl_sd->fd, msg_txt(MSG_HAS_GIVEN_JUDGEMENT)); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_txt(MSG_JUDGEMENT_WAS_MADE)); // Judgement was made.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_doommap(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;

	clif_specialeffect(sd,450,2);
	for (i = 0; i < fd_max; ++i)
	{
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && i != (size_t)fd && 
			sd.block_list::m == pl_sd->block_list::m &&
		    sd.isGM() >= pl_sd->isGM())
		{
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
			clif_displaymessage(pl_sd->fd, msg_txt(MSG_HAS_GIVEN_JUDGEMENT)); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_txt(MSG_JUDGEMENT_WAS_MADE)); // Judgement was made.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// dropall
/// Drop all your possession on the ground
///
bool command_dropall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	size_t i;
	for (i = 0; i < MAX_INVENTORY; ++i)
	{
		if (sd.status.inventory[i].amount)
		{
			if(sd.status.inventory[i].equip != 0)
			pc_unequipitem(sd, i, 3);
			pc_dropitem(sd, i, sd.status.inventory[i].amount);
		}
	}
	if(fd != sd.fd )
	{
		clif_displaymessage(sd.fd, "Ever played 52 card pickup?");
		//clif_displaymessage(sd.fd, "It is offical.. your a jerk");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// dye && ccolor
///
bool command_dye(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned int cloth_color = param[0];

	if( param.size() < 1 )
	{
		char output[128];
		snprintf(output, sizeof(output), "Please, enter a clothes color (usage: dye/ccolor <clothes color: %ld-%ld>).", (unsigned long)MIN_CLOTH_COLOR, (unsigned long)MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
	}
	else if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR)
	{
		pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(fd, msg_txt(MSG_APPEARENCE_CHANGED)); // Appearence changed.
		return true;
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_NUMBER)); // An invalid number was specified.

	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_model(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	unsigned int hair_style = param[0];
	unsigned int hair_color = param[1];
	unsigned int cloth_color =param[2];

	if( param.size()<1 ||
		hair_style  < MIN_HAIR_STYLE  || hair_style  > MAX_HAIR_STYLE ||
		hair_color  < MIN_HAIR_COLOR  || hair_color  > MAX_HAIR_COLOR ||
		cloth_color < MIN_CLOTH_COLOR || cloth_color > MAX_CLOTH_COLOR )
	{
		snprintf(output, sizeof(output), "Please, enter at least a value (usage: model <hair ID: %ld-%ld> <hair color: %ld-%ld> <clothes color: %ld-%ld>).",
			(unsigned long)MIN_HAIR_STYLE, (unsigned long)MAX_HAIR_STYLE, (unsigned long)MIN_HAIR_COLOR, (unsigned long)MAX_HAIR_COLOR, (unsigned long)MIN_CLOTH_COLOR, (unsigned long)MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
	}
	else if (cloth_color != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 ||  sd.status.class_ == 17))
	{
		clif_displaymessage(fd, msg_txt(MSG_CMD_INVALID_CLASS)); // You can't use this command with this class_.
	}
	else
	{
		pc_changelook(sd, LOOK_HAIR, hair_style);
		pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
		pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(fd, msg_txt(MSG_APPEARENCE_CHANGED)); // Appearence changed.
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// hairstyle && hstyle
///
bool command_hair_style(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned int hair_style = param[0];

	if( param.size() < 1 )
	{
		char output[128];
		snprintf(output, sizeof(output), "Please, enter a hair style (usage: hairstyle/hstyle <hair ID: %ld-%ld>).", (unsigned long)MIN_HAIR_STYLE, (unsigned long)MAX_HAIR_STYLE);
		clif_displaymessage(fd, output);
	}
	else if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE)
	{
		if (hair_style != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 || sd.status.class_ == 17))
		{
			clif_displaymessage(fd, msg_txt(MSG_CMD_INVALID_CLASS)); // You can't use this command with this class_.
		}
		else
		{
			pc_changelook(sd, LOOK_HAIR, hair_style);
			clif_displaymessage(fd, msg_txt(MSG_APPEARENCE_CHANGED)); // Appearence changed.
			return true;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_NUMBER)); // An invalid number was specified.
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// haircolor && hcolor
bool command_hair_color(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned int hair_color = param[0];
	if( param.size() < 1 )
	{
		char output[128];
		snprintf(output, sizeof(output), "Please, enter a hair color (usage: haircolor/hcolor <hair color: %ld-%ld>).", (unsigned long)MIN_HAIR_COLOR, (unsigned long)MAX_HAIR_COLOR);
		clif_displaymessage(fd, output);
	}
	else if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR)
	{
		if (hair_color != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 || sd.status.class_ == 17))
		{
			clif_displaymessage(fd, msg_txt(MSG_CMD_INVALID_CLASS)); // You can't use this command with this class_.
		}
		else
		{
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			clif_displaymessage(fd, msg_txt(MSG_APPEARENCE_CHANGED)); // Appearence changed.
			return true;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_NUMBER)); // An invalid number was specified.
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// effect
///
bool command_effect(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *psd=&sd;
	bool all=false;
	int type = param[0];

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter at least a option (usage: effect <type+> [\"all\"/char name]).");
	}
	else if ( param.size()>=2 && !(all=(param[1]=="all")) && NULL==(psd = CommandInfo::param2sd(param[1])) )
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
	}
	else if( sd.isGM() <  psd->isGM())
	{	// Your GM level don't authorise you to do this action on this player.
		clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION));
	}
	else if( all )
	{
		clif_specialeffect(*psd, type, 3);	// 3 = all
	}
	else
	{
		clif_specialeffect(*psd, type, 1);	// 1 = self
		clif_displaymessage(psd->fd, msg_txt(MSG_YOUR_EFFECT_HAS_CHANGED)); // Your effect has changed.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_misceffect(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
		return false;
	clif_misceffect(sd, param[0]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "email <actual email> <new email>"
///
bool command_email(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()< 2)
	{
		clif_displaymessage(fd, "Please enter 2 emails (usage: email <actual@email> <new@email>).");
		return false;
	}

	const char *actual_email = param[0];
	const char *new_email    = param[1];


	if( !email_check(actual_email) )
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_ACTUAL_EMAIL)); // Invalid actual email. If you have default e-mail, give a@a.com.
	}
	else if( !email_check(new_email) )
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_NEW_EMAIL)); // Invalid new email. Please enter a real e-mail.
	}
	else if(strcasecmp(new_email, "a@a.com") == 0)
	{
		clif_displaymessage(fd, msg_txt(MSG_EMAIL_MUST_BE_VALID)); // New email must be a real e-mail.
	}
	else if(strcasecmp(actual_email, new_email) == 0)
	{
		clif_displaymessage(fd, msg_txt(MSG_EMAIL_MUST_BE_DIFFERENT)); // New email must be different of the actual e-mail.
	}
	else
	{
		chrif_changeemail(sd.status.account_id, actual_email, new_email);
		clif_displaymessage(fd, msg_txt(MSG_SENDED_TO_LOGIN_VIA_CHAR)); // Information sended to login-server via char-server.
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "fakename [char name/chrid/accid] [name]"
/// => sets fake name.
/// usage 
///
bool command_fakename(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char* msg;
	if( param.size()<1 )
	{
		if( sd.fakename[0] )
		{
			sd.fakename[0]='\0';
			pc_setpos(sd, sd.mapname, sd.block_list::x, sd.block_list::y, 3);
			msg="Returned to real name.";
		}
		else
		{
			msg="You must enter a name.";
		}
	}
	else
	{
		const char *name = param[0];
		if(strlen(name) < 2)
		{
			msg="Fake name must be at least two characters.";
		}
		else
		{
			safestrcpy(sd.fakename, sizeof(sd.fakename), name);
			clif_charnameack(-1, sd, true);
			msg="Fake name enabled.";
		}
	}
	clif_displaymessage(fd,msg);
	return true;

}

///////////////////////////////////////////////////////////////////////////////
///
/// follow
/// Follow a player .. staying no more then 5 spaces away
///
bool command_follow(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd = NULL;
	if (param.size()<1)
		return false;
	pl_sd = CommandInfo::param2sd(param[0]);
	if(pl_sd != NULL)
	{
		if (sd.followtarget == pl_sd->block_list::id)
			pc_stop_following(sd);
		else
			pc_follow(sd, pl_sd->block_list::id);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_gat(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int y;
	char output[128];


	for (y = 2; y >= -2; y--) {
		snprintf(output, sizeof(output), "%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
			maps[sd.block_list::m].mapname,sd.block_list::x - 2, sd.block_list::y + y,
 			map_getcell(sd.block_list::m, sd.block_list::x - 2, sd.block_list::y + y, CELL_GETTYPE),
 			map_getcell(sd.block_list::m, sd.block_list::x - 1, sd.block_list::y + y, CELL_GETTYPE),
 			map_getcell(sd.block_list::m, sd.block_list::x,     sd.block_list::y + y, CELL_GETTYPE),
 			map_getcell(sd.block_list::m, sd.block_list::x + 1, sd.block_list::y + y, CELL_GETTYPE),
 			map_getcell(sd.block_list::m, sd.block_list::x + 2, sd.block_list::y + y, CELL_GETTYPE));

		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_gm(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a password (usage: gm <password>).");
		return false;
	}
	if (sd.isGM())
	{	// a GM can not use this function. only a normal player (become gm is not for gm!)
		clif_displaymessage(fd, msg_txt(MSG_ALREADY_GM)); // You already have some GM powers.
		return false;
	}
	else
		chrif_changegm(sd.status.account_id, param[0], 1+strlen(param[0]));

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// gmotd (Global MOTD)
///
bool command_gmotd(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char buf[256];
	size_t sl;
	FILE *fp;

	if(	(fp = basics::safefopen(motd_txt, "r"))!=NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			sl = prepare_line(buf);
			if(sl)
				intif_GMmessage(buf, sl, 8);
		}
		fclose(fp);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "go [city_number/city_name]"
///
bool command_go(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int town;
	char output[128];
	int m;
	static const struct { const char *map; int x,   y; } data[] =
	{
		{ "prontera",   156, 191  },	//	 0=Prontera
		{ "morocc",     156,  93  },	//	 1=Morroc
		{ "geffen",     119,  59  },	//	 2=Geffen
		{ "payon",      162, 233  },	//	 3=Payon
		{ "alberta",    192, 147  },	//	 4=Alberta
		{ "izlude",     128, 114  },	//	 5=Izlude
		{ "aldebaran",  140, 131  },	//	 6=Al de Baran
		{ "xmas",       147, 134  },	//	 7=Lutie
		{ "comodo",     209, 143  },	//	 8=Comodo
		{ "yuno",       157,  51  },	//	 9=Yuno
		{ "amatsu",     198,  84  },	//	10=Amatsu
		{ "gonryun",    160, 120  },	//	11=Gon Ryun
		{ "umbala",      89, 157  },	//	12=Umbala
		{ "niflheim",    21, 153  },	//	13=Niflheim
		{ "louyang",    217,  40  },	//	14=Lou Yang
		{ "new_1-1",     53, 111  },	//	15=Training Grounds
		{ "sec_pri",     23,  61  },	//	16=Prison
		{ "jawaii",     249, 127  },	//  17=Jawaii
		{ "ayothaya",   151, 117  },	//  18=Ayothaya
		{ "einbroch",    64, 200  },	//  19=Einbroch
		{ "lighthalzen",158,  92  },	//  20=Lighthalzen
		{ "einbech.gat", 70,  95  },	//  21=Einbech
		{ "hugel",		 96, 145  },	//  22=Hugel
	};

	if(maps[sd.block_list::m].flag.nogo)
	{
		clif_displaymessage(sd.fd,"You can not use go on this map.");
		return true;
	}
	// get the number
	town = param[0];

	// if no value, display all value
	if (param.size() < 1 || town < -3 || town >= (int)(sizeof(data) / sizeof(data[0]))) {
		clif_displaymessage(fd, msg_txt(MSG_INVALID_LOCATION)); // Invalid location number or name.
		clif_displaymessage(fd, msg_txt(MSG_USE_ONE_OF_THIS)); // Please, use one of this number/name:
		clif_displaymessage(fd, "  0=Prontera         1=Morroc       2=Geffen");
		clif_displaymessage(fd, "  3=Payon            4=Alberta      5=Izlude");
		clif_displaymessage(fd, "  6=Aldebaran        7=Lutie        8=Comodo");
		clif_displaymessage(fd, "  9=Yuno            10=Amatsu      11=Gon Ryun");
		clif_displaymessage(fd, " 12=Umbala          13=Niflheim    14=Lou Yang");
		clif_displaymessage(fd, " 15=Novice Grounds  16=Prison      17=Jawaii");
		clif_displaymessage(fd, " 18=Ayothaya        19=Einbroch    20=Lighthalzen");
		clif_displaymessage(fd, " 21=Einbech         22=Hugel");

		return false;
	}
	else
	{	// map with different writings
		static const struct { const char *map; int ch; int no; } towns[] =
		{
			{"prontera",	3,	 0},
			{"morocc",		3,	 1},
			{"geffen",		3,	 2},
			{"payon",		3,	 3},
			{"paion",		3,	 3},
			{"alberta",		3,	 4},
			{"izlude",		3,	 5},
			{"islude",		3,	 5},
			{"aldebaran",	3,	 6},
			{"al de baran",	3,	 6},
			{"lutie",		3,	 7},
			{"christmas",	3,	 7},
			{"xmas",		3,	 7},
			{"x-mas",		3,	 7},
			{"comodo",		3,	 8},
			{"yuno",		3,	 9},
			{"juno",		3,	 9},
			{"amatsu",		3,	10},
			{"ammatsu",		3,	10},
			{"gonryun",		3,	11},
			{"umbala",		3,	12},
			{"niflheim",	3,	13},
			{"louyang",		3,	14},
			{"new_1-1",		3,	15},
			{"startpoint",	3,	15},
			{"begining",	3,	15},
			{"sec_pri",		3,	16},
			{"prison",		3,	16},
			{"jail",		3,	16},
			{"jawaii",		3,	17},
			{"ayothaya",	3,	18},
			{"einbroch",	6,	19},
			{"ainbroch",	3,	19},
			{"lighthalzen",	3,	20},
			{"reichthalzen",3,	20},
			{"einbech",     6,	21},
			{"hugel",       3,	22}
		};

		size_t i;
		char mapname[128]="", *ip;

		strcpytolower(mapname, sizeof(mapname), param[0]);
		ip=strchr(mapname, '.');
		if(ip) *ip=0;

		for(i=0; i<(sizeof(towns)/sizeof(towns[0])); ++i)
		{
			if( strncmp(mapname, towns[i].map, towns[i].ch) == 0)
				break;
		}
		if( i<(sizeof(towns)/sizeof(towns[0])) )
			town = towns[i].no;

		if (town >= -3 && town <= -1)
		{
			if (sd.status.memo_point[-town-1].mapname[0])
			{
				m = map_mapname2mapid(sd.status.memo_point[-town-1].mapname);
				if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
				{
					clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_TO));
					return false;
				}
				else if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
				{
					clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
					return false;
				}
				else if( pc_setpos(sd, sd.status.memo_point[-town-1].mapname, sd.status.memo_point[-town-1].x, sd.status.memo_point[-town-1].y, 3) )
				{
					clif_displaymessage(fd, msg_txt(MSG_WARPED));
				}
				else
				{
					clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND));
					return false;
				}
			}
			else
			{
				snprintf(output, sizeof(output), msg_txt(MSG_YOUR_MEMO_POINT_D_DOESNT_EXIST), -town-1); // Your memo point #%d doesn't exist.
				clif_displaymessage(fd, output);
				return false;
			}
		}
		else if (town >= 0 && town < (int)(sizeof(data) / sizeof(data[0])))
		{
			m = map_mapname2mapid(data[town].map);
			if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_TO));
				return false;
			}
			if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
				return false;
			}
			if( pc_setpos(sd, (char *)data[town].map, data[town].x, data[town].y, 3) ) {
				clif_displaymessage(fd, msg_txt(MSG_WARPED)); // Warped.
			} else {
				clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
				return false;
			}
		}
		else
		{	// if you arrive here, you have an error in town variable when reading of names
			clif_displaymessage(fd, msg_txt(MSG_INVALID_LOCATION)); // Invalid location number or name.
			return false;
		}
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// grind
///
bool command_grind(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum;
	int inf;
	const char *target = param[0];

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Usage: grind <target>");
		return false;
	}
	if((pl_sd=CommandInfo::param2sd(target)) == NULL)
		return false;
	for (skillnum = 1; skillnum < 500; skillnum++)
	{
		sd.status.sp = sd.status.max_sp;
		command_alive(fd, sd, command, param);
		inf = skill_get_inf(skillnum);
		if ((inf == 2) || (inf == 1))
			skill_use_pos(&sd, pl_sd->block_list::x+5, pl_sd->block_list::y+5, skillnum, 1);
		else
			skill_use_id(&sd, pl_sd->block_list::id, skillnum, 1);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// grind2
///
bool command_grind2(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int i, x, y, id;
	for(i=1000; i<2000; ++i)
	{
		x = sd.block_list::x + (rand() % 10 - 5);
		y = sd.block_list::y + (rand() % 10 - 5);
		id = mob_once_spawn(&sd, "this", x, y, "--ja--", i, 1, "");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_guild(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a guild name (usage: guild <guild_name>).");
		return false;
	}
	ulong prev = config.guild_emperium_check;
	config.guild_emperium_check = 0;
	guild_create(sd, param[0]);
	config.guild_emperium_check = prev;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_guildlevelup(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int level = 0;
	short added_level;
	struct guild *guild_info;


	if (param.size()<1 || (level=param[0]) == 0) {
		clif_displaymessage(fd, "Please, enter a valid level (usage: guildlvup/guildlvlup <# of levels>).");
		return false;
	}

	if (sd.status.guild_id <= 0 || (guild_info = guild_search(sd.status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_txt(MSG_NOT_IN_GUILD)); // You're not in a guild.
		return false;
	}
	if (strcmp(sd.status.name, guild_info->master) != 0) {
		clif_displaymessage(fd, msg_txt(MSG_NOT_GUILDMASTER)); // You're not the master of your guild.
		return false;
	}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, added_level);
		clif_displaymessage(fd, msg_txt(MSG_GUILD_LEVEL_CHANGED)); // Guild level changed.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_GLV_FAILED)); // Guild level change failed.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall online characters of a guild to your location
///
bool command_guildrecall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	const char *guild_name=param[0];
	char output[128];
	struct guild *g;

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: guildrecall <guild_name/id>).");
		return false;
	}

	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(guild_name))) != NULL)
	{
		count = 0;
		for (i = 0; i < fd_max; ++i) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
			    sd.status.account_id != pl_sd->status.account_id &&
			    pl_sd->status.guild_id == g->guild_id) {
				if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
					count++;
				else
					pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
			}
		}
		snprintf(output, sizeof(output), msg_txt(MSG_ALL_ON_CHARS_S_GUILD_NEAR), g->name); // All online characters of the %s guild are near you.
		clif_displaymessage(fd, output);
		if (count) {
			snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(MSG_NONE_OF_GUILD_ON)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_guildstorage(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct pc_storage *stor;
	if (sd.status.guild_id > 0)
	{
		if( sd.state.storage_flag == 1 ||
			((stor = account2storage2(sd.status.account_id)) != NULL && stor->storage_status == 1) )
		{
			clif_displaymessage(fd, msg_txt(MSG_GUILD_STORAGE_ALREADY_OPEN));
			return false;
		}
		storage_guild_storageopen(sd);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_NOT_IN_GUILD));
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_guildspy(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: guildspy <guild_name/id>).");
		return false;
	}

	const char *guild_name=param[0];
	char output[128];
	struct guild *g;

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(guild_name))) != NULL) {
		if (sd.guildspy == g->guild_id) {
			sd.guildspy = 0;
			snprintf(output, sizeof(output), msg_txt(MSG_NO_LONGER_SPYING_ON_THE_S_GUILD), g->name); // No longer spying on the %s guild.
			clif_displaymessage(fd, output);
		} else {
			sd.guildspy = g->guild_id;
			snprintf(output, sizeof(output), msg_txt(MSG_SPYING_ON_THE_S_GUILD), g->name); // Spying on the %s guild.
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(MSG_NONE_OF_GUILD_ON)); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_partyspy(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a party name/id (usage: partyspy <party_name/id>).");
		return false;
	}

	const char *party_name=param[0];
	char output[128];
	struct party *p;

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(party_name))) != NULL) {
		if (sd.partyspy == p->party_id) {
			sd.partyspy = 0;
			snprintf(output, sizeof(output), msg_txt(MSG_NO_LONGER_SPYING_ON_THE_S_PARTY), p->name); // No longer spying on the %s party.
			clif_displaymessage(fd, output);
		} else {
			sd.partyspy = p->party_id;
			snprintf(output, sizeof(output), msg_txt(MSG_SPYING_ON_THE_S_PARTY), p->name); // Spying on the %s party.
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(MSG_NONE_OF_PARTY_ON)); // Incorrect name or ID, or no one from the party is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_gvgoff(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	if (maps[sd.block_list::m].flag.gvg) {
		maps[sd.block_list::m].flag.gvg = 0;
		clif_send0199(sd.block_list::m, 0);
		clif_displaymessage(fd, msg_txt(MSG_GVG_OFF)); // GvG: Off.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_GVG_IS_ALREADY_OFF)); // GvG is already Off.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_gvgon(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	if (!maps[sd.block_list::m].flag.gvg) {
		maps[sd.block_list::m].flag.gvg = 1;
		clif_send0199(sd.block_list::m, 3);
		clif_displaymessage(fd, msg_txt(MSG_GVG_ON)); // GvG: On.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_GVG_IS_ALREADY_ON)); // GvG is already On.
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_help(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	FILE* fp;
	if((fp = basics::safefopen(help_txt, "r")) != NULL)
	{
		char line[2048], w1[2048], w2[2048];
		int gm_level = sd.isGM();
		clif_displaymessage(fd, msg_txt(MSG_HELP_COMMANDS)); // Help commands:

		while(fgets(line, sizeof(line), fp) != NULL)
		{
			if( !is_valid_line(line) )
				continue;
			if (sscanf(line, "%2048[^:=]%*[:=]%2048[^\r\n]", w1, w2) < 2)
				clif_displaymessage(fd, line);
			else if (gm_level >= atoi(w1))
				clif_displaymessage(fd, w2);
		}
		fclose(fp);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_HELPTXT_NOT_FOUND)); // File help.txt not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_hide(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (sd.status.option & OPTION_HIDE)
	{
		sd.status.option &= ~OPTION_HIDE;
		clif_displaymessage(fd, msg_txt(MSG_INVISIBLE_OFF)); // Invisible: Off
	}
	else
	{
		sd.status.option |= OPTION_HIDE;
		clif_displaymessage(fd, msg_txt(MSG_INVISIBLE_ON)); // Invisible: On
	}
	clif_changeoption(sd);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_hidenpc(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a NPC name (usage: npcoff <NPC_name>).");
		return false;
	}
	const char *NPCname=param[0];
	if (npc_name2id(NPCname) != NULL)
	{
		npc_enable(NPCname, 0);
		clif_displaymessage(fd, msg_txt(MSG_NPC_DISABLED)); // Npc Disabled.
	} 
	else 
	{
		clif_displaymessage(fd, msg_txt(MSG_THIS_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_shownpc(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a NPC name (usage: enablenpc <NPC_name>).");
		return false;
	}
	const char *NPCname=param[0];

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 1);
		clif_displaymessage(fd, msg_txt(MSG_NPC_ENABLED)); // Npc Enabled.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_THIS_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_loadnpc(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	FILE *fp;

	if (param.size()<1) {
		clif_displaymessage(fd, "Please, enter a script file name (usage: loadnpc <file name>).");
		return false;
	}

	// check if script file exists
	if ((fp = basics::safefopen(param[0], "r")) == NULL) {
		clif_displaymessage(fd, msg_txt(MSG_SCRIPT_NOT_LOADED));
		return false;
	}
	fclose(fp);

	// add to list of script sources and run it
	npc_addsrcfile(param[0]);
	npc_parsesrcfile(param[0]);

	clif_displaymessage(fd, msg_txt(MSG_SCRIPT_LOADED));

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_unloadnpc(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a NPC name (usage: npcoff <NPC_name>).");
		return false;
	}
	
	const char *NPCname = param[0];
	struct npc_data *nd = npc_name2id(NPCname);

	if( nd != NULL )
	{
		npc_remove_map(nd);
		clif_displaymessage(fd, msg_txt(MSG_NPC_DISABLED)); // Npc Disabled.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_THIS_NPC_DOESNT_EXIST)); // This NPC doesn't exist.
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// cause random emote on all online players
///
bool command_happyhappyjoyjoy(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i,e;
	for(i=0; i<fd_max; ++i)
	{
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
		{
			e=rand()%40;
			if(e==34)
				e = 0;
			clif_emotion(*pl_sd,e);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_hatch(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (sd.status.pet_id <= 0)
		clif_sendegg(sd);
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_YOU_ALREADY_HAVE_A_PET)); // You already have a pet.
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_heal(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	long hp = param[0], sp = param[1];
	if (hp == 0 && sp == 0)
	{
		hp = sd.status.max_hp - sd.status.hp;
		sp = sd.status.max_sp - sd.status.sp;
	}
	else
	{
		if(hp > 0 && (hp > (long)sd.status.max_hp || hp > (long)(sd.status.max_hp - sd.status.hp))) // fix positiv overflow
			hp = sd.status.max_hp - sd.status.hp;
		else if(hp < 0 && (hp < -(long)sd.status.max_hp || hp < (1 - (long)sd.status.hp))) // fix negativ overflow
			hp = 1 - sd.status.hp;
		if(sp > 0 && (sp > (long)sd.status.max_sp || sp > ((long)sd.status.max_sp - (long)sd.status.sp))) // fix positiv overflow
			sp = sd.status.max_sp - sd.status.sp;
		else if(sp < 0 && (sp < -(long)sd.status.max_sp || sp < (1 - (long)sd.status.sp))) // fix negativ overflow
			sp = 1 - (long)sd.status.sp;
	}

	if (hp > 0) // display like heal
		clif_heal(fd, SP_HP, hp);
	else if (hp < 0) // display like damage
		clif_damage(sd,sd, gettick(), 0, 0, -hp, 0 , 4, 0);
	if (sp > 0) // no display when we lost SP
		clif_heal(fd, SP_SP, sp);

	if (hp != 0 || sp != 0)
	{
		sd.heal(hp, sp);
		if (hp >= 0 && sp >= 0)
			clif_displaymessage(fd, msg_txt(MSG_HP_SP_RECOVERED)); // HP, SP recovered.
		else
			clif_displaymessage(fd, msg_txt(MSG_HP_SP_MODIFIED)); // HP or/and SP modified.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_HP_AND_SP_ARE_ALREADY_GOOD)); // HP and SP are already with the good value.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// identify
///
bool command_identify(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int i,num;
	for(i=num=0; i<MAX_INVENTORY; ++i)
	{
		if(sd.status.inventory[i].nameid > 0 && sd.status.inventory[i].identify!=1)
		{
			num++;
		}
	}
	if(num > 0)
	{
		clif_item_identify_list(sd);
	}
	else
	{
		clif_displaymessage(fd, "There are no items to appraise.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "idsearch <part_of_name>"
///
bool command_idsearch(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a part of item name (usage: idsearch <part_of_item_name>).");
		return false;
	}

	const char *item_name = param[0];
	char output[128];
	unsigned int i, match;
	struct item_data *item;

	snprintf(output, sizeof(output), msg_txt(MSG_REFERENCE_RESULT_S), item_name); // The reference result of '%s' (name: id):
	clif_displaymessage(fd, output);
	match = 0;
	for(i=0; i < MAX_ITEMS; ++i) {
		if ((item = itemdb_exists(i)) != NULL && strstr(item->jname, item_name) != NULL) {
			match++;
			snprintf(output, sizeof(output), msg_txt(MSG_S_D), item->jname, item->nameid); // %s: %d
			clif_displaymessage(fd, output);
		}
	}
	snprintf(output, sizeof(output), msg_txt(MSG_D_AFFAIR_ABOVE), match); // It is %d affair above.
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// item command (usage: "item <name/id_of_item> <quantity>")
///
static void  command_item_sub(struct map_session_data& sd, item& item_tmp, int pet_id, size_t number, size_t inc)
{
	size_t i;
	for(i=0; i<number; i+=inc)
	{	// if pet egg
		if (pet_id >= 0)
		{
			sd.catch_target_class = pet_db[pet_id].class_;
			intif_create_pet(sd.status.account_id, sd.status.char_id,
							 pet_db[pet_id].class_, mob_db[pet_db[pet_id].class_].lv,
							 pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
							 100, 0, 1, pet_db[pet_id].jname);

		}
		else
		{	// if not pet egg
			int flag = pc_additem(sd, item_tmp, inc);
			if( flag ) clif_additem(sd, 0, 0, flag);
		}
	}
}

bool command_item(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter an item name/id (usage: item <item name or ID> [quantity]).");
		clif_displaymessage(fd, "  [Identify_flag] [refine] [attribut] [Card1] [Card2] [Card3] [Card4]).");

		return false;
	}

	int item_id=0;
	struct item_data *item_data;
	
	int number = param[1];
	if (number <= 0)
		number = 1;

	if( (item_data = itemdb_searchname(param[0])) != NULL ||
		(item_data = itemdb_exists(atoi(param[0]))) != NULL )
		item_id = item_data->nameid;

	if(item_id >= 500)
	{
		int pet_id = search_petDB_index(item_id, PET_EGG);
		struct item item_tmp(item_id);

		item_tmp.identify = param[2];
		item_tmp.refine   = param[3];
		item_tmp.attribute= param[4];
		item_tmp.card[0]  = param[5];
		item_tmp.card[1]  = param[6];
		item_tmp.card[2]  = param[7];
		item_tmp.card[3]  = param[8];
		bool all     = 0==strcasecmp( param.last(), "all") || 0==strcasecmp( param.last(), "everyone");
		size_t inc = number;
		if( item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8)
		{
			inc = 1;
			if (item_data->type == 7)
			{
				item_tmp.identify = 1;
				item_tmp.refine = 0;
			}
			else if (item_data->type == 8)
				item_tmp.refine = 0;
			else  if (item_tmp.refine > 10)
				item_tmp.refine = 10;
		}
		else
		{
			item_tmp.identify = 1;
			item_tmp.refine = 0;
			item_tmp.attribute = 0;
		}

		if(all)
		{
			map_session_data* psd;
			size_t i;
			char output[128];
			for (i=0; i<fd_max; ++i)
			{
				if( session[i] && (psd = (map_session_data *)session[i]->user_session) )
				{
					command_item_sub(*psd, item_tmp, pet_id, number, inc);
					snprintf(output, sizeof(output), "You got %lu %s.", (unsigned long)number, item_data->jname);
					clif_displaymessage(psd->fd, output);
				}
			}
		}
		else
		{
			command_item_sub(sd, item_tmp, pet_id, number, inc);
		}
		clif_displaymessage(fd, msg_txt(MSG_ITEM_CREATED)); // Item created.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_ITEM)); // Invalid item ID or name.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_itemcheck(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_checkitem(sd);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// Show Items DB Info
///
bool command_iteminfo(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char *itype[12] = {"Potion/Food", "BUG!", "Usable", "Etc", "Weapon", "Protection", "Card", "Egg", "Pet Acessory", "BUG!", "Arrow", "Lure/Scroll"};
	char output[128];

	struct item_data *item_data;
	int item_id=0;

	if (param.size()<1) {
		clif_displaymessage(fd, "Please, enter Item name or its ID (usage: iteminfo <item_name_or_ID>).");
		return false;
	}

	if ((item_data = itemdb_searchname(param[0])) != NULL ||
	    (item_data = itemdb_exists(atoi(param[0]))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {

		snprintf(output, sizeof(output), "Item: '%s'/'%s'[%d] (%d) Type: %s | Extra Effect: %s",
			item_data->name,item_data->jname,item_data->flag.slot,item_id,
			item_data->type < 12 ? itype[item_data->type] : "BUG!",
			(item_data->use_script==NULL && item_data->equip_script==NULL) ? "None" : (item_data->use_script==NULL ? "On Equip" : "On Usage")
		);
		clif_displaymessage(fd, output);

		snprintf(output, sizeof(output), "NPC Buy:%ldz%s, Sell:%ldz%s | Weight: %ld ",
			(unsigned long)item_data->value_buy, item_data->flag.value_notdc ? "(No Discount!)":"",
			(unsigned long)item_data->value_sell, item_data->flag.value_notoc ? "(No Overcharge!)":"",
			(unsigned long)item_data->weight );
		clif_displaymessage(fd, output);

		return true;
	}

	clif_displaymessage(fd, "Item not found.");
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_itemreset(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	size_t i;
	for(i = 0; i < MAX_INVENTORY; ++i)
	{
		if (sd.status.inventory[i].amount && sd.status.inventory[i].equip == 0)
			pc_delitem(sd, i, sd.status.inventory[i].amount, 0);
	}
	clif_displaymessage(sd.fd, msg_txt(MSG_ITEMS_REMOVED)); // All of your items have been removed.
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// jail char_name
/// Special warp! No check with nowarp and nowarpto flag
///
bool command_jail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: jail <char_name>).");
		return false;
	}

	const char *player_name = param[0];
	struct map_session_data *pl_sd = CommandInfo::param2sd(player_name);
	int x, y;

	if(pl_sd != NULL)
	{
		if (sd.isGM() >= pl_sd->isGM())
		{	// you can jail only lower or same GM
			switch(rand() % 2)
			{
			case 0:
				x = 24;
				y = 75;
				break;
			default:
				x = 49;
				y = 75;
				break;
			}
			if( pc_setpos(*pl_sd, "sec_pri.gat", x, y, 3) )
			{
				pc_setsavepoint(*pl_sd, "sec_pri.gat", x, y); // Save Char Respawn Point in the jail room
				clif_displaymessage(pl_sd->fd, msg_txt(MSG_GM_HAS_SEND_YOU_IN_JAILS)); // GM has send you in jails.
				clif_displaymessage(fd, msg_txt(MSG_PLAYER_WARPED_IN_JAILS)); // Player warped in jails.
			}
			else
			{
				clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
				return false;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// unjail/discharge char_name
/// Special warp! No check with nowarp and nowarpto flag
///
bool command_unjail(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: unjail/discharge <char_name>).");
		return false;
	}
	const char *player_name = param[0];
	struct map_session_data *pl_sd = CommandInfo::param2sd(player_name);

	if( pl_sd != NULL )
	{
		if (sd.isGM() >= pl_sd->isGM())
		{	// you can jail only lower or same GM
			if (pl_sd->block_list::m != map_mapname2mapid("sec_pri.gat"))
			{
				clif_displaymessage(fd, msg_txt(MSG_THIS_PLAYER_IS_NOT_IN_JAILS)); // This player is not in jails.
				return false;
			}
			else if (pc_setpos(*pl_sd, "prontera.gat", 156,191, 3) == 0)
			{
				pc_setsavepoint(*pl_sd, "prontera.gat", 156,191); // Save char respawn point in Prontera
				clif_displaymessage(pl_sd->fd, msg_txt(MSG_GM_HAS_DISCHARGE_YOU)); // GM has discharge you.
				clif_displaymessage(fd, msg_txt(MSG_PLAYER_WARPED_TO_PRONTERA)); // Player warped to Prontera.
			}
			else
			{
				clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
				return false;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
///
bool command_jobchange(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int job = param[0];
	if(job == 0)
		job = job_id(param[0]);

	if( param.size()<1 || job < 0 || job > MAX_PC_CLASS )
	{
		clif_displaymessage(fd, "Please, enter a valid job ID (usage: job/jobchange <job ID> [char name/chrid/accid]).");
	}
	else
	{
		if (job == 37 || job == 45)
			return true;
		
		int j;
		// fix pecopeco display
		if ((job != 13 && job != 21 && job != 4014 && job != 4022 && job != 4030 && job != 4036 && job != 4037 && job != 4044 ))
		{
			if (pc_isriding(sd))
			{
				if (sd.status.class_ == 13)
					sd.status.class_ = sd.view_class = 7;
				else if (sd.status.class_ == 21)
					sd.status.class_ = sd.view_class = 14;
				else if (sd.status.class_ == 4014)
					sd.status.class_ = sd.view_class = 4008;
				else if (sd.status.class_ == 4022)
					sd.status.class_ = sd.view_class = 4015;
				else if (sd.status.class_ == 4036)
					sd.status.class_ = sd.view_class = 4030;
				else if (sd.status.class_ == 4044)
					sd.status.class_ = sd.view_class = 4037;
				sd.status.option &= ~0x0020;
				clif_changeoption(sd);
				status_calc_pc(sd, 0);
			}
		}
		else
		{
			if (!pc_isriding(sd))
			{
				if (job == 13)
					job = 7;
				else if (job == 21)
					job = 14;
				else if (job == 4014)
					job = 4008;
				else if (job == 4022)
					job = 4015;
				else if (job == 4036)
					job = 4030;
				else if (job == 4044)
					job = 4037;
			}
		}
		for (j=0; j < MAX_INVENTORY; ++j)
		{
			if(sd.status.inventory[j].nameid>0 && sd.status.inventory[j].equip!=0)
				pc_unequipitem(sd, j, 3);
		}
		if (pc_jobchange(sd, job, 0) == 0)
			clif_displaymessage(fd, "Job has been changed.");
		else
		{
			clif_displaymessage(fd, "Impossible to change job.");
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_joblevelup(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct pc_base_job s_class = pc_calc_base_job(sd.status.class_);
	int up_level = 50;
	int level=0;

	if (param.size()<1 || (level = param[0]) == 0)
	{
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: joblvup/jlevel/joblvlup <number of levels>).");
		return false;
	}

	if (s_class.job == 0)
		up_level -= 40;
	// super novices can go up to 99
	else if (s_class.job == 23)
		up_level += 49;
	else if (sd.status.class_ > 4007 && sd.status.class_ < 4023)
		up_level += 20;

	if (level > 0)
	{
		if (sd.status.job_level == up_level)
		{
			clif_displaymessage(fd, msg_txt(MSG_JOB_LV_MAX)); // Job level can't go any higher.
			return false;
		}
		if( level > up_level || level+sd.status.job_level > up_level) // fix positiv overflow
			level = up_level - sd.status.job_level;
		sd.status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		sd.status.skill_point += level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		status_calc_pc(sd, 0);
		clif_misceffect(sd, 1);
		clif_displaymessage(fd, msg_txt(MSG_JOB_LV_RAISED)); // Job level raised.
	}
	else
	{
		if (sd.status.job_level == 1)
		{
			clif_displaymessage(fd, msg_txt(MSG_JOB_LEVEL_CANT_GO_ANY_LOWER)); // Job level can't go any lower.
			return false;
		}
		if (level < -up_level || level < (1 - (int)sd.status.job_level)) // fix negativ overflow
			level = 1 - sd.status.job_level;
		sd.status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		if (sd.status.skill_point > 0)
		{
			int sp = sd.status.skill_point;
			sp += level;
			sd.status.skill_point = (sp<0) ? 0 : sp;

			clif_updatestatus(sd, SP_SKILLPOINT);
		} // to add: remove status points from skills
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(MSG_JOB_LV_LOWERED)); // Job level lowered.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_jump(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	int x = param[1], y = param[2];

	if( sd.block_list::m >= map_num ||
		maps[sd.block_list::m].flag.nowarp ||
		(maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) )
	{
		clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
		return false;
	}
	if (x <= 0)
		x = rand() % (maps[sd.block_list::m].xs-1) + 1;
	if (y <= 0)
		y = rand() % (maps[sd.block_list::m].ys-1) + 1;
	if (x > 0 && x < maps[sd.block_list::m].xs && y > 0 && y < maps[sd.block_list::m].ys)
	{
		pc_setpos(sd, sd.mapname, x, y, 3);
		snprintf(output, sizeof(output), msg_txt(MSG_JUMP_TO_D_D), x, y); // Jump to %d %d
		clif_displaymessage(fd, output);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_COORD_OUT_OF_RANGE)); // Coordinates out of range.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_jumpto(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];

	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: jumpto/warpto/goto <char name>/<account id>/<char id>).");
		return false;
	}

	struct map_session_data *pl_sd = CommandInfo::param2sd(param[0]);

	if( pl_sd != NULL )
	{
		if( sd.block_list::id == pl_sd->block_list::id ) //Yourself mate? Tsk tsk tsk.
			return false;

		if (pl_sd->block_list::m <map_num  && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
			clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_TO));
			return false;
		}
		if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
			clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
			return false;
		}
		pc_setpos(sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
		snprintf(output, sizeof(output), msg_txt(MSG_JUMP_TO_S), pl_sd->status.name); // Jump to %s
		clif_displaymessage(fd, output);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_kami(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a message (usage: kami <message>).");
		return false;
	}
	intif_GMmessage(param.line(), 1+strlen(param.line()), (command[5]=='b') ? 0x10 : 0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_kick(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: kick <char name>/<account id>/<char id>).");
		return false;
	}

	struct map_session_data *pl_sd = CommandInfo::param2sd(param[0]);

	if( pl_sd != NULL )
	{
		if( sd.block_list::id == pl_sd->block_list::id ) //Yourself mate? Tsk tsk tsk.
			return false;

		if (sd.isGM() >= pl_sd->isGM()) // you can kick only lower or same gm level
		{
			char output[128];
			clif_GM_kick(sd, *pl_sd, 1);
			snprintf(output, sizeof(output), "Kicked %s.", pl_sd->status.name);
			clif_displaymessage(fd, output);
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_kickall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;


	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
		    sd.isGM() >= pl_sd->isGM()) { // you can kick only lower or same gm level
			if (sd.status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, *pl_sd, 0);
			}
		}

	clif_displaymessage(fd, msg_txt(MSG_ALL_PLAYERS_HAVE_BEEN_KICKED)); // All players have been kicked!

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_kill(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: kill <char name>/<account id>/<char id>).");
		return false;
	}

	struct map_session_data *pl_sd = CommandInfo::param2sd(param[0]);

	if( pl_sd != NULL )
	{
		if( sd.block_list::id == pl_sd->block_list::id ) //Yourself mate? Tsk tsk tsk.
			return false;

		if (sd.isGM() >= pl_sd->isGM())
		{	// you can kill only lower or same level
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
			clif_displaymessage(fd, msg_txt(MSG_CHARACTER_KILLED)); // Character killed.
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// killable
/// enable other people killing you even when not in pvp
///
bool command_killable(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	sd.state.killable = ( param.size() )?(bool)param[0] : !sd.state.killable;
	clif_displaymessage(fd, (sd.state.killable)?"now killable":"no longer killable");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// killer
/// enable killing players even when not in pvp
///
bool command_killer(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	sd.state.killer = ( param.size() )?(bool)param[0] : !sd.state.killer;
	clif_displaymessage(fd, msg_txt((sd.state.killer)?MSG_YOU_BE_A_KILLA:MSG_YOU_GONNA_BE_OWN3D));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_killmonster(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	class CAtKillMonster : public CMapProcessor
	{
		int flag;
	public:
		CAtKillMonster(int f) : flag(f)	{}
			~CAtKillMonster()			{}
		virtual int process(block_list& bl) const
		{
			struct mob_data &md = (mob_data &)bl;
			if(flag)
				mob_damage(md, md.hp, 2, NULL);
			else
				mob_remove_map(md,1);
			return 0;
		}
	};

	int drop = param[0];
	int map_id=0;
	if( param.size()<1 || ((map_id = map_mapname2mapid(param[0])) < 0) )
		map_id = sd.block_list::m;

	if(map_id>0 && map_id<(int)map_num)
	{
		block_list::foreachinarea( CAtKillMonster(drop),
			map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB);
		clif_displaymessage(fd, msg_txt(MSG_ALL_MONSTERS_KILLED)); // All monsters killed!
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_load(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int m = map_mapname2mapid(sd.status.save_point.mapname);
	if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_SAVE));
		return false;
	}
	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
		return false;
	}

	pc_setpos(sd, sd.status.save_point.mapname, sd.status.save_point.x, sd.status.save_point.y, 0);
	clif_displaymessage(fd, msg_txt(MSG_WARPING_TO_RESPAWN));

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_lostskill(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	short skill_id=0;
	if (param.size()<1 || (skill_id = param[0]) < 0)
	{
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: lostskill <#:0+>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL)
	{
		if (skill_get_inf2(skill_id) & 0x01)
		{
			if (pc_checkskill(sd, skill_id) > 0)
			{
				sd.status.skill[skill_id].lv = 0;
				sd.status.skill[skill_id].flag = 0;
				clif_skillinfoblock(sd);
				clif_displaymessage(fd, msg_txt(MSG_YOU_FORGOT_SKILL)); // You have forgotten the skill.
			}
			else
			{
				clif_displaymessage(fd, msg_txt(MSG_YOU_DONT_HAVE_THIS_QUEST_SKILL)); // You don't have this quest skill.
				return false;
			}
		} 
		else 
		{
			clif_displaymessage(fd, msg_txt(MSG_SKILL_NOT_EXIST_OR_NO_QUEST_SKILL)); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_SKILL_NOT_EXIST)); // This skill number doesn't exist.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_makeegg(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct item_data *item_data;
	int id, pet_id;


	if (param.size()<1) {
		clif_displaymessage(fd, "Please, enter a monter/egg name/id (usage: makeegg <pet_id>).");
		return false;
	}

	if ((item_data = itemdb_searchname(param[0])) != NULL) // for egg name
		id = item_data->nameid;
	else if ((id = mobdb_searchname(param[0])) == 0) // for monster name
		id = param[0];

	pet_id = search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd.catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
			sd.status.account_id, sd.status.char_id,
			pet_db[pet_id].class_, mob_db[pet_db[pet_id].class_].lv,
			pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
			100, 0, 1, pet_db[pet_id].jname);
	} else {
		clif_displaymessage(fd, msg_txt(MSG_MONTER_EGG_NAME_DOESNT_EXIST)); // The monter/egg name/id doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// mapexit?A?}?b?v?T?[?o?[?d?I?1?3?1?e

bool command_mapexit(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;
	for (i = 0; i < fd_max; ++i)
	{
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
		{
			if (sd.status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, *pl_sd, 0);
		}
	}
	clif_GM_kick(sd, sd, 0);
	flush_fifos();
	core_stoprunning();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "mapflag [flagap name] [1|0|on|off] [map name]"
/// => Shows information about the map flags [map name]
/// Also set flags
///
bool command_mapflag(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{	// show flags of current map
	}
	else if( param.size()==1 )
	{	// could be:
		// mapflag [flagap name]	-> would display flag status on current map
		// mapflag [1|0|on|off]	-> useless
		// mapflag [map name]		-> would display all flags on given map
	}
	else if( param.size()==3 )
	{	// could be:
		// mapflag [flagap name] [1|0|on|off]	-> would set flag status on current map
		// mapflag [1|0|on|off] [map name]		-> useless
		// mapflag [flagap name] [map name]	-> would display specified flags on given map
	}
	else if( param.size()>=3 )
	{	// could be:
		// mapflag [flagap name] [1|0|on|off] [map name]	-> would set flag status on given map
	}

	// needs bool is_mapflag(const char*), bool is_mapname(const char*)
	// int get_mapflag(const char*), int set_mapflag(const char*, int), 

	clif_displaymessage(fd, "work in progress");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "mapinfo [map name] [0-3/player/npc/chat]"
/// => Shows information about the map [map name]
/// 0 = no additional information
/// 1 = Show users in that map and their location
/// 2 = Shows NPCs in that map
/// 3 = Shows the shops/chats in that map (not implemented)
///
bool command_mapinfo(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	struct npc_data *nd = NULL;
	chat_data *cd = NULL;
	char direction[12]="";
	char mapname[128]="";
	char output[128];
	int m_id, chat_num;
	size_t i;
	int list=0;

	if( param.size() )
	{	// map name or option
		if(basics::stringcheck::isdigit(param[0][0])) list = atoi(param[0]);
		else if(param[0]=="player") list = 1;
		else if(param[0]=="npc") list = 2;
		else if(param[0]=="chat") list = 3;
		else
			buffer2mapname(mapname, sizeof(mapname), param[0]);

		if( param.size()>=2 )
		{	// has also a second parameter
			if(*mapname)
			{	// is an option
				if(basics::stringcheck::isdigit(param[1][0])) list = atoi(param[1]);
				else if(param[1]=="player") list = 1;
				else if(param[1]=="npc") list = 2;
				else if(param[1]=="chat") list = 3;
			}
			else
			{	// is a mapname
				buffer2mapname(mapname, sizeof(mapname), param[1]);
			}
		}
	}
	if(list < 0 || list > 3)
	{
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: mapinfo [mapname] [0-3/player/npc/chat]).");
		return false;
	}


	if(mapname[0] == '\0')
	{	// use sd map, when none was given
		buffer2mapname(mapname, sizeof(mapname), sd.mapname);
	}


	if((m_id = map_mapname2mapid(mapname)) < 0)
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
		return false;
	}

	clif_displaymessage(fd, "------ Map Info ------");
	chat_num = 0;
	for (i = 0; i < fd_max; ++i)
	{
		if( session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
			pl_sd->chat )
		{
			++chat_num;
		}
	}
	snprintf(output, sizeof(output), "Map Name: %s | Players In Map: %ld | NPCs In Map: %ld | Chats In Map: %d", mapname, (unsigned long)(maps[m_id].users), (unsigned long)(maps[m_id].npc_num), chat_num);
	clif_displaymessage(fd, output);
	clif_displaymessage(fd, "------ Map Flags ------");
	strcpy(output,"PvP Flags: ");
	if (maps[m_id].flag.pvp)
		strcat(output, "Pvp ON | ");
	if (maps[m_id].flag.nopvp)
		strcat(output, "NoPvp | ");
	if (maps[m_id].flag.pvp_noguild)
		strcat(output, "NoGuild | ");
	if (maps[m_id].flag.pvp_noparty)
		strcat(output, "NoParty | ");
	if (maps[m_id].flag.pvp_nightmaredrop)
		strcat(output, "NightmareDrop | ");
	if (maps[m_id].flag.pvp_nocalcrank)
		strcat(output, "NoCalcRank | ");
	clif_displaymessage(fd, output);

	strcpy(output,"GvG Flags: ");
	if (maps[m_id].flag.gvg)
		strcat(output, "GvG ON | ");
	if (maps[m_id].flag.gvg_dungeon)
		strcat(output, "GvGDungeon | ");
	if (maps[m_id].flag.gvg_noparty)
		strcat(output, "NoParty | ");
	clif_displaymessage(fd, output);

	strcpy(output,"Teleport Flags: ");
	if (maps[m_id].flag.noteleport)
		strcat(output, "NoTeleport | ");
	if (maps[m_id].flag.monster_noteleport)
		strcat(output, "Monster NoTeleport | ");
	if (maps[m_id].flag.nowarp)
		strcat(output, "NoWarp | ");
	if (maps[m_id].flag.nowarpto)
		strcat(output, "NoWarpTo | ");
	if (maps[m_id].flag.noreturn)
		strcat(output, "NoReturn | ");
	if (maps[m_id].flag.nogo)
		strcat(output, "NoGo | ");
	if (maps[m_id].flag.nomemo)
		strcat(output, "NoMemo | ");
	clif_displaymessage(fd, output);

	snprintf(output, sizeof(output), "No Penalty: %s | No Zeny Penalty: %s", (maps[m_id].flag.nopenalty) ? "On" : "Off", (maps[m_id].flag.nozenypenalty) ? "On" : "Off");
	clif_displaymessage(fd, output);

	if (maps[m_id].flag.nosave) {
		if (maps[m_id].save.x == -1 || maps[m_id].save.y == -1 )
			snprintf(output, sizeof(output), "No Save, Save Point: %s,Random",maps[m_id].save.mapname);
		else
			snprintf(output, sizeof(output), "No Save, Save Point: %s,%d,%d",
				maps[m_id].save.mapname,maps[m_id].save.x,maps[m_id].save.y);
		clif_displaymessage(fd, output);
	}

	strcpy(output,"Weather Flags: ");
	if (maps[m_id].flag.snow)
		strcat(output, "Snow | ");
	if (maps[m_id].flag.fog)
		strcat(output, "Fog | ");
	if (maps[m_id].flag.sakura)
		strcat(output, "Sakura | ");
	if (maps[m_id].flag.clouds)
		strcat(output, "Clouds | ");
	if (maps[m_id].flag.fireworks)
		strcat(output, "Fireworks | ");
	if (maps[m_id].flag.leaves)
		strcat(output, "Leaves | ");
	if (maps[m_id].flag.rain)
		strcat(output, "Rain | ");
	if (maps[m_id].flag.indoors)
		strcat(output, "Indoors | ");
	clif_displaymessage(fd, output);

	strcpy(output,"Other Flags: ");
	if (maps[m_id].flag.nobranch)
		strcat(output, "NoBranch | ");
	if (maps[m_id].flag.notrade)
		strcat(output, "NoTrade | ");
	if (maps[m_id].flag.noskill)
		strcat(output, "NoSkill | ");
	if (maps[m_id].flag.noicewall)
		strcat(output, "NoIcewall | ");
	clif_displaymessage(fd, output);

	strcpy(output,"Other Flags: ");
	if (maps[m_id].flag.nobaseexp)
		strcat(output, "NoBaseEXP | ");
	if (maps[m_id].flag.nojobexp)
		strcat(output, "NoJobEXP | ");
	if (maps[m_id].flag.nomobloot)
		strcat(output, "NoMobLoot | ");
	if (maps[m_id].flag.nomvploot)
		strcat(output, "NoMVPLoot | ");
	clif_displaymessage(fd, output);


	switch (list) {
	case 0:
		// Do nothing. It's list 0, no additional display.
		break;
	case 1:
		clif_displaymessage(fd, "----- Players in Map -----");
		for (i = 0; i < fd_max; ++i) {
			if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && strcmp(pl_sd->mapname, mapname) == 0) {
				snprintf(output, sizeof(output), "Player '%s' (session #%ld) | Location: %d,%d",
				        pl_sd->status.name, (unsigned long)(i), pl_sd->block_list::x, pl_sd->block_list::y);
				clif_displaymessage(fd, output);
			}
		}
		break;
	case 2:
		clif_displaymessage(fd, "----- NPCs in Map -----");
		for (i = 0; i < maps[m_id].npc_num;) {
			nd = maps[m_id].npc[i];
			switch(nd->dir) {
			case 0:  strcpy(direction, "North"); break;
			case 1:  strcpy(direction, "North West"); break;
			case 2:  strcpy(direction, "West"); break;
			case 3:  strcpy(direction, "South West"); break;
			case 4:  strcpy(direction, "South"); break;
			case 5:  strcpy(direction, "South East"); break;
			case 6:  strcpy(direction, "East"); break;
			case 7:  strcpy(direction, "North East"); break;
			case 9:  strcpy(direction, "North"); break;
			default: strcpy(direction, "Unknown"); break;
			}
			snprintf(output, sizeof(output), "NPC %ld: %s | Direction: %s | Sprite: %d | Location: %d %d",
			        (unsigned long)(++i), nd->name, direction, nd->class_, nd->block_list::x, nd->block_list::y);
			clif_displaymessage(fd, output);
		}
		break;
	case 3:
		clif_displaymessage(fd, "----- Chats in Map -----");
		for (i = 0; i < fd_max; ++i)
		{
			if( session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
				(cd=pl_sd->chat) &&
			    strcmp(pl_sd->mapname, mapname) == 0 &&
			    cd->usersd[0] == pl_sd)
			{
				snprintf(output, sizeof(output), "Chat %ld: %s | Player: %s | Location: %d %d",
					(unsigned long)(i), cd->title, pl_sd->status.name, cd->block_list::x, cd->block_list::y);
				clif_displaymessage(fd, output);
				snprintf(output, sizeof(output), "   Users: %d/%d | Password: %s | Public: %s",
				        cd->users, cd->limit, cd->pass, (cd->pub) ? "Yes" : "No");
				clif_displaymessage(fd, output);
			}
		}
		break;
	default: // normally impossible to arrive here
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: mapinfo <0-3> [map]).");
		return false;
		break;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
/// mapmove.
///
/// parameters: mapname, x, y
///
bool command_mapmove(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char *mapname=param[0];
	int x = param[1], y = param[2];
	unsigned short m;

	if( param.size() < 3 )
	{
		clif_displaymessage(fd, "Please, enter a map (usage: mapmove/rura/warp <mapname> <x> <y>).");
	}
	else if( (m = map_mapname2mapid(mapname)) >= map_num )
	{
		clif_displaymessage(fd, msg_txt(MSG_MAP_NOT_FOUND)); // Map not found.
	}
	else if( maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM() )
	{
		clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_TO));
	}
	else if( sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM() )
	{
		clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_WARP_FROM));
	}
	else
	{
		if (x <= 0)
			x = 1+rand() % maps[m].xs;
		if (y <= 0)
			y = rand() % maps[m].ys;

		if (x > 0 && x < 400 && y > 0 && y < 400)
		{
			if( pc_setpos(sd, mapname, x, y, 3) )
			{
				clif_displaymessage(fd, msg_txt(MSG_WARPED)); // Warped.
				return true;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_COORD_OUT_OF_RANGE)); // Coordinates out of range.
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// me
/// => Displays the OUTPUT string on top of
///    the Visible players Heads.
///
bool command_me(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a message (usage: me <message>).");
		return false;
	}
	char output[256];
	snprintf(output, 256, "** %s %s **", sd.status.name, param.line());
    clif_disp_overhead(sd, output);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_memo(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	size_t i;
	bool ret = false;
	
	if( param.size() >=1 )
	{
		int position = param[0];
		if (position >= MIN_PORTAL_MEMO && position <= MAX_PORTAL_MEMO)
		{
			if( sd.block_list::m < map_num &&
				(maps[sd.block_list::m].flag.nowarpto || maps[sd.block_list::m].flag.nomemo) &&
				config.any_warp_GM_min_level > sd.isGM() )
			{
				clif_displaymessage(fd, msg_txt(MSG_NOT_AUTHORIZED_TO_MEMO));
				return false;
			}
			if (sd.status.memo_point[position].mapname[0]) {
				snprintf(output, sizeof(output), msg_txt(MSG_REPLACE_MEMO_D_S), position, sd.status.memo_point[position].mapname, sd.status.memo_point[position].x, sd.status.memo_point[position].y); // You replace previous memo position %d - %s (%d,%d).
				clif_displaymessage(fd, output);
			}
			memcpy(sd.status.memo_point[position].mapname, maps[sd.block_list::m].mapname, 24);
			sd.status.memo_point[position].x = sd.block_list::x;
			sd.status.memo_point[position].y = sd.block_list::y;
			clif_skill_memo(sd, 0);
			if (pc_checkskill(sd, AL_WARP) <= (position + 1))
				clif_displaymessage(fd, msg_txt(MSG_DONT_HAVE_WARP_SKILL)); // Note: you don't have the 'Warp' skill level to use it.
			ret = true;
		}
		else
		{
			snprintf(output, sizeof(output), "Please, enter a valid position (usage: memo <memo_position:%d-%d>).", MIN_PORTAL_MEMO, MAX_PORTAL_MEMO);
			clif_displaymessage(fd, output);
		}
	}

	// display actual memo points
	clif_displaymessage(sd.fd,  "Your actual memo positions are (except respawn point):");
	for (i = MIN_PORTAL_MEMO; i <= MAX_PORTAL_MEMO; ++i)
	{
		if (sd.status.memo_point[i].mapname[0])
			snprintf(output, sizeof(output), "%d - %s (%d,%d)", i, sd.status.memo_point[i].mapname, sd.status.memo_point[i].x, sd.status.memo_point[i].y);
		else
			snprintf(output, sizeof(output), msg_txt(MSG_D___VOID), i); // %d - void
		clif_displaymessage(sd.fd, output);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Show Monster DB Info
///
bool command_mobinfo(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned char msize[3][7] = {"Small", "Medium", "Large"};
	unsigned char mrace[12][11] = {"Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Boss", "Non-Boss"};
	unsigned char melement[11][8] = {"None", "Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead"};

	char output[128];
	char output2[256];
	struct item_data *item_data;
	struct mob_db *mob;
	int mob_id;
	int i, j;

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: mobinfo <monster_name_or_monster_ID>).");
		return false;
	}

	// If monster identifier/name argument is a name
	if( (mob_id = mobdb_searchname(param[0])) == 0 ) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(param[0]));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}

	mob = &mob_db[mob_id];

	// stats
	if (mob->mexp)
		snprintf(output, sizeof(output), "Monster (MVP): '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	else
		snprintf(output, sizeof(output), "Monster: '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	clif_displaymessage(fd, output);
	snprintf(output, sizeof(output), " Level:%d  HP:%d  SP:%d  Base EXP:%d  Job EXP:%d", mob->lv, mob->max_hp, mob->max_sp, mob->base_exp, mob->job_exp);
	clif_displaymessage(fd, output);
	snprintf(output, sizeof(output), " DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d", mob->def, mob->mdef, mob->str, mob->agi, mob->vit, mob->int_, mob->dex, mob->luk);
	clif_displaymessage(fd, output);
	if (mob->element < 20) {
		//Element - None, Level 0
		i = 0;
		j = 0;
	} else {
		i = mob->element % 20 + 1;
		j = mob->element / 20;
	}
	snprintf(output, sizeof(output), " ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)", mob->atk1, mob->atk2, mob->range, mob->range2 , mob->range3, msize[mob->size], mrace[mob->race], melement[i], j);
	clif_displaymessage(fd, output);
	// drops
	clif_displaymessage(fd, " Drops:");
	strcpy(output, " ");
	j = 0;
	for (i = 0; i < 10; ++i) {
		if (mob->dropitem[i].nameid <= 0 || (item_data = itemdb_exists(mob->dropitem[i].nameid)) == NULL)
			continue;
		if (mob->dropitem[i].p > 0) {
			snprintf(output2, sizeof(output2), " - %s  %02.02f%%", item_data->name, (float)mob->dropitem[i].p / 100);
			strcat(output, output2);
			if (++j % 3 == 0) {
				clif_displaymessage(fd, output);
				strcpy(output, " ");
			}
		}
	}
	if (j == 0)
		clif_displaymessage(fd, "This monster has no drop.");
	else if (j % 3 != 0)
		clif_displaymessage(fd, output);
	// mvp
	if (mob->mexp) {
		snprintf(output, sizeof(output), " MVP Bonus EXP:%d  %02.02f%%", mob->mexp, (float)mob->mexpper / 100);
		clif_displaymessage(fd, output);
		strcpy(output, " MVP Items:");
		j = 0;
		for (i = 0; i < 3; ++i) {
			if (mob->mvpitem[i].nameid <= 0 || (item_data = itemdb_exists(mob->mvpitem[i].nameid)) == NULL)
				continue;
			if (mob->mvpitem[i].p > 0) {
				j++;
				if (j == 1)
					snprintf(output2, sizeof(output2), " %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
				else
					snprintf(output2, sizeof(output2), " - %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
				strcat(output, output2);
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "This monster has no MVP drop.");
		else
			clif_displaymessage(fd, output);
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// Mob search
///
class CAtMobSearch : public CMapProcessor
{
	int mob_id;
	int fd;
	mutable int number;
	mutable char output[128];
public:
	CAtMobSearch(int m, int f) : mob_id(m), fd(f), number(0)	{}
	~CAtMobSearch()	{}
	virtual int process(block_list& bl) const
	{
		struct mob_data *md = bl.get_md();
		if( md && fd && (mob_id==-1 || (md->class_==mob_id)) )
		{
			snprintf(output, sizeof(output), "%2d[%3d:%3d] %s",++number,bl.x, bl.y,md->name);
			clif_displaymessage(fd, output);
		}
		return 0;
	}
};

bool command_mobsearch(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	const char *mob_name=param[0];
	int mob_id,map_id = 0;
	
	if( param.size()<1 )
		return false;
	if( (mob_id = atoi(mob_name)) == 0 )
		mob_id = mobdb_searchname(mob_name);
	if(mob_id !=-1 && (mob_id <= 1000 || mob_id >= 2000))
	{
		snprintf(output, sizeof output, "Invalid mob id %s!",mob_name);
		clif_displaymessage(fd, output);
		return true;
	}
	if(mob_id == atoi(mob_name) && mob_db[mob_id].jname)
				mob_name = mob_db[mob_id].jname;	// --ja--
//				mob_name = mob_db[mob_id].name;		// --en--

	map_id = sd.block_list::m;

	snprintf(output, sizeof output, "Mob Search... %s %s", mob_name, sd.mapname);
	clif_displaymessage(fd, output);

	block_list::foreachinarea( CAtMobSearch(mob_id, fd), map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_monster(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<2 )
	{
		clif_displaymessage(fd, msg_txt(MSG_NAME_MONSTER_ID)); // Give a display name and monster name/id please.
		return false;
	}

	char output[128];
	uint32 mob_id;
	unsigned int count;
	unsigned int i, j, k;
	unsigned int mx, my, range;

	const char *name	= param[0];
	const char *monster	= param[1];
	unsigned int number	= param[2];
	unsigned int x		= param[3];
	unsigned int y		= param[4];

	 // check name first (to avoid possible name begining by a number)
	if( (mob_id = mobdb_searchname(monster)) == 0 )
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0 || mob_id>2000)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}
	if (mob_id == MOBID_EMPERIUM)
	{
		clif_displaymessage(fd, msg_txt(MSG_CANNOT_SPAWN_EMPERIUM)); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;

	if( strlen(name) < 1 )
		name = "--ja--";
	
	// If value of command_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (config.atc_spawn_quantity_limit >= 1 && number > config.atc_spawn_quantity_limit)
		number = config.atc_spawn_quantity_limit;

	if (config.etc_log)
		ShowMessage("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, x, y);

	count = 0;
	range = (unsigned int)sqrt((double)number) + 5; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; ++i) {
		j = 0;
		k = 0;
		while(j++ < 8 && k == 0) { // try 8 times to spawn the monster (needed for close area)
			if (x <= 0)
				mx = sd.block_list::x + (rand() % range - (range / 2));
			else
				mx = x;
			if (y <= 0)
				my = sd.block_list::y + (rand() % range - (range / 2));
			else
				my = y;
			k = mob_once_spawn(&sd, "this", mx, my, name, mob_id, 1, "");
		}
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
		if (number == count)
			clif_displaymessage(fd, msg_txt(MSG_ALL_MONSTER_SUMMONED)); // All monster summoned!
		else {
			snprintf(output, sizeof(output), msg_txt(MSG_D_MONSTER_SUMMONED), count); // %d monster(s) summoned!
			clif_displaymessage(fd, output);
		}
	else {
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_spawn(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, msg_txt(MSG_GIVE_A_MONSTER_NAME)); // Give a monster name/id please.
		return false;
	}

	char output[128];
	uint32 mob_id;
	unsigned int count;
	unsigned int i, j, k;
	unsigned int mx, my, range;

	const char *monster = param[0];
	const bool has_name = basics::stringcheck::isalpha(*param[1]);
	const char *name	= (has_name) ? (const char *)param[1] : "--ja--";
	unsigned int number	= (has_name) ? param[2] : param[1];
	unsigned int x		= (has_name) ? param[3] : param[2];
	unsigned int y		= (has_name) ? param[4] : param[3];

	// check name first (to avoid possible name begining by a number)
	if( (mob_id = mobdb_searchname(monster)) == 0)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0 || mob_id>2000)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}
	if (mob_id == MOBID_EMPERIUM)
	{
		clif_displaymessage(fd, msg_txt(MSG_CANNOT_SPAWN_EMPERIUM)); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;
	if (strlen(name) < 1)
		name = "--ja--";

	// If value of command_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (config.atc_spawn_quantity_limit >= 1 && number > config.atc_spawn_quantity_limit)
		number = config.atc_spawn_quantity_limit;

	if (config.etc_log)
		ShowMessage("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, x, y);

	count = 0;
	range = (unsigned int)sqrt((double)number) + 5; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; ++i) {
		j = 0;
		k = 0;
		while(j++ < 8 && k == 0) { // try 8 times to spawn the monster (needed for close area)
			if (x <= 0)
				mx = sd.block_list::x + (rand() % range - (range / 2));
			else
				mx = x;
			if (y <= 0)
				my = sd.block_list::y + (rand() % range - (range / 2));
			else
				my = y;
			k = mob_once_spawn(&sd, "this", mx, my, name, mob_id, 1, "");
		}
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
	{
		if (number == count)
			clif_displaymessage(fd, msg_txt(MSG_ALL_MONSTER_SUMMONED)); // All monster summoned!
		else
		{
			snprintf(output, sizeof(output), msg_txt(MSG_D_MONSTER_SUMMONED), count); // %d monster(s) summoned!
			clif_displaymessage(fd, output);
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  big monster spawning
bool command_monsterbig(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, msg_txt(MSG_GIVE_A_MONSTER_NAME)); // Give a monster name/id please.
		return false;
	}

	uint32 mob_id;
	unsigned int count;
	unsigned int i;

	const char *monster = param[0];
	const bool has_name = basics::stringcheck::isalpha(*param[1]);
	const char *name	= (has_name) ? (const char *)param[1] : "--ja--";
	unsigned int number	= (has_name) ? param[2] : param[1];
	unsigned int x		= (has_name) ? param[3] : param[2];
	unsigned int y		= (has_name) ? param[4] : param[3];


	// check name first (to avoid possible name begining by a number)
	if( (mob_id = mobdb_searchname(monster)) == 0)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0 || mob_id>2000)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}
	if (mob_id == MOBID_EMPERIUM)
	{
		clif_displaymessage(fd, msg_txt(MSG_CANNOT_SPAWN_EMPERIUM)); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;
	if (strlen(name) < 1)
		name = "--ja--";

	// If value of command_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (config.atc_spawn_quantity_limit >= 1 && number > config.atc_spawn_quantity_limit)
		number = config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; ++i) {
		int mx, my;
		if (x <= 0)
			mx = sd.block_list::x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd.block_list::y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn(&sd, "this", mx, my, name, mob_id+4000, 1, "") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_txt(MSG_ALL_MONSTER_SUMMONED)); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid Monster ID.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  small monster spawning
bool command_monstersmall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, msg_txt(MSG_GIVE_A_MONSTER_NAME)); // Give a monster name/id please.
		return false;
	}
	uint32 mob_id;
	unsigned int count;
	unsigned int i;

	const char *monster = param[0];
	const bool has_name = basics::stringcheck::isalpha(*param[1]);
	const char *name	= (has_name) ? (const char *)param[1] : "--ja--";
	unsigned int number	= (has_name) ? param[2] : param[1];
	unsigned int x		= (has_name) ? param[3] : param[2];
	unsigned int y		= (has_name) ? param[4] : param[3];


	// check name first (to avoid possible name begining by a number)
	if( (mob_id = mobdb_searchname(monster)) == 0)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0 || mob_id>2000)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid monster ID or name.
		return false;
	}
	if (mob_id == MOBID_EMPERIUM)
	{
		clif_displaymessage(fd, msg_txt(MSG_CANNOT_SPAWN_EMPERIUM)); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;
	if (strlen(name) < 1)
		name = "--ja--";

	// If value of command_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (config.atc_spawn_quantity_limit >= 1 && number > config.atc_spawn_quantity_limit)
		number = config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; ++i) {
		int mx, my;
		if (x <= 0)
			mx = sd.block_list::x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd.block_list::y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn(&sd, "this", mx, my, name, mob_id+2000, 1, "") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_txt(MSG_ALL_MONSTER_SUMMONED)); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_txt(MSG_INVALID_MONSTER)); // Invalid Monster ID.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// monsterignore
/// => Makes monsters ignore you.
///
bool command_monsterignore(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	sd.state.monster_ignore = 
		(param.size())? (bool)param[0] : !sd.state.monster_ignore;

	clif_displaymessage(sd.fd, (sd.state.monster_ignore)?
		"Monsters will now ignore you.":"Monsters are no longer ignoring you.");
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_mount_peco(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (!pc_isriding(sd)) { // if actually no peco
		if (sd.status.class_ == 7 || sd.status.class_ == 14 || sd.status.class_ == 4008 || sd.status.class_ == 4015 || sd.status.class_ == 4030 || sd.status.class_ == 4036 || sd.status.class_ == 4037 || sd.status.class_ == 4044) {
			if (sd.status.class_ == 7)
				sd.status.class_ = sd.view_class = 13;
			else if (sd.status.class_ == 14)
				sd.status.class_ = sd.view_class = 21;
			else if (sd.status.class_ == 4008)
				sd.status.class_ = sd.view_class = 4014;
			else if (sd.status.class_ == 4015)
				sd.status.class_ = sd.view_class = 4022;
			else if (sd.status.class_ == 4030) //baby Knight
				sd.status.class_ = sd.view_class = 4036;
			else if (sd.status.class_ == 4037) //baby Crusader
				sd.status.class_ = sd.view_class = 4044;
			pc_setoption(sd, sd.status.option | 0x0020);
			clif_displaymessage(fd, msg_txt(MSG_MOUNTED_PECO)); // Mounted Peco.
		} else {
			clif_displaymessage(fd, msg_txt(MSG_YOU_NOT_MOUNT_WITH_JOB)); // You can not mount a peco with your job.
			return false;
		}
	} else {
		if (sd.status.class_ == 13)
			sd.status.class_ = sd.view_class = 7;
		else if (sd.status.class_ == 21)
			sd.status.class_ = sd.view_class = 14;
		else if (sd.status.class_ == 4014)
			sd.status.class_ = sd.view_class = 4008;
		else if (sd.status.class_ == 4022)
			sd.status.class_ = sd.view_class = 4015;
		else if (sd.status.class_ == 4036) //baby Knight
			sd.status.class_ = sd.view_class = 4030;
		else if (sd.status.class_ == 4044) //baby Crusader
			sd.status.class_ = sd.view_class = 4037;

		pc_setoption(sd, sd.status.option & ~0x0020);
		clif_displaymessage(fd, msg_txt(MSG_UNMOUNTED_PECO)); // Unmounted Peco.
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// mute - Mutes a player for a set amount of time
///
bool command_mute(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(!config.muting_players)
	{
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Usage: mute <time> [character name].");
		return false;
	}

	int manner = param[0];
	clif_GM_silence(sd, sd, 0);
	sd.status.manner -= manner;
	if(sd.status.manner < 0)
		status_change_start(&sd,SC_NOCHAT,0,0,0,0,0,0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// mutearea
///
class CAtMuteArea : public CMapProcessor
{
	uint32 id;
	int time;
public:
	CAtMuteArea(uint32 i, int t) : id(i), time(t)	{}
	~CAtMuteArea()	{}
	virtual int process(block_list& bl) const
	{
		struct map_session_data *sd = bl.get_sd();
		if(sd)
		{
			if( id != bl.id && !sd->isGM() )
			{
				sd->status.manner -= time;
				if(sd->status.manner < 0)
					status_change_start(sd,SC_NOCHAT,0,0,0,0,0,0);
			}
			return 0;
		}
		return 1;
	}
};
bool command_mutearea(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(!config.muting_players)
	{
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}
	int time = param[0];
	if (time <= 0)
		time = 15; // 15 minutes default
	block_list::foreachinarea( CAtMuteArea(sd.block_list::id, time),
		sd.block_list::m,  ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE,  ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_PC);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// npcmove
/// move a npc
///
bool command_npcmove(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<3)
	{
		clif_displaymessage(fd, "Usage: npcmove <X> <Y> <npc_name>");
		return false;
	}

	int x = param[0];
	int y = param[1];
	const char *name=param[2];
	struct npc_data *nd = npc_name2id(name);

	if(!nd)
	{
		clif_displaymessage(fd, "Usage: npcmove <X> <Y> <npc_name>");
		return false;
	}

	npc_enable(name, 0);
	nd->block_list::x = x;
	nd->block_list::y = y;
	npc_enable(name, 1);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// NPC/PET
///
bool command_npctalk(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<2 )
		return false;

	const char *name = param[0];
	const char *mes  = param[1];
	struct npc_data *nd = npc_name2id(name);
	
	if( !nd )
		return false;

	char output[128];
	snprintf(output, sizeof(output), "%s: %s", name, mes);
	clif_message(*nd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_pettalk(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(!sd.status.pet_id || !sd.pd)
		return false;

	char output[128];
	snprintf(output, sizeof(output), "%s : %s", sd.pd->pet.name, param.line());
	clif_message(*sd.pd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Removed nuke for now in favor of alchemist marine sphere skill
///
bool command_nuke(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: nuke <char name>).");
		return false;
	}
	const char *player_name=param[0];
	struct map_session_data *pl_sd;

	if((pl_sd = CommandInfo::param2sd(player_name)) != NULL)
	{
		if (sd.isGM() >= pl_sd->isGM())
		{	// you can kill only lower or same GM level
			skill_castend_damage_id(pl_sd, pl_sd, NPC_SELFDESTRUCTION, 99, gettick(), 0);
			clif_displaymessage(fd, msg_txt(MSG_PLAYER_HAS_BEEN_NUKED)); // Player has been nuked!
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_option(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter at least a option (usage: option <param1:0+> <param2:0+> <param3:0+>).");
		return false;
	}

	int param1 = param[0];
	int param2 = param[1];
	int param3 = param[2];


	sd.opt1 = param1;
	sd.opt2 = param2;
	if (!(sd.status.option & CART_MASK) && param3 & CART_MASK)
	{
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
	}
	sd.status.option = param3;
	// fix pecopeco display
	if (sd.status.class_ == 13 || sd.status.class_ == 21 || sd.status.class_ == 4014 || sd.status.class_ == 4022 || sd.status.class_ == 4030 || sd.status.class_ == 4036 || sd.status.class_ == 4037 || sd.status.class_ == 4044)
	{
		if (!pc_isriding(sd))
		{	// sd have the new value...
			if (sd.status.class_ == 13)
				sd.status.class_ = sd.view_class = 7;
			else if (sd.status.class_ == 21)
				sd.status.class_ = sd.view_class = 14;
			else if (sd.status.class_ == 4014)
				sd.status.class_ = sd.view_class = 4008;
			else if (sd.status.class_ == 4022)
				sd.status.class_ = sd.view_class = 4015;
			else if (sd.status.class_ == 4036) //baby Knight
				sd.status.class_ = sd.view_class = 4030;
			else if (sd.status.class_ == 4044) //baby Crusader
				sd.status.class_ = sd.view_class = 4037;
		}
	}
	else
	{
		if (pc_isriding(sd))
		{	// sd have the new value...
			if (sd.status.class_ == 7)
				sd.status.class_ = sd.view_class = 13;
			else if (sd.status.class_ == 14)
				sd.status.class_ = sd.view_class = 21;
			else if (sd.status.class_ == 4008)
				sd.status.class_ = sd.view_class = 4014;
			else if (sd.status.class_ == 4015)
				sd.status.class_ = sd.view_class = 4022;
			else if (sd.status.class_ == 4030) //baby Knight
				sd.status.class_ = sd.view_class = 4036;
			else if (sd.status.class_ == 4037) //baby Crusader
				sd.status.class_ = sd.view_class = 4044;
			else
				sd.status.option &= ~0x0020;
		}
	}
	clif_changeoption(sd);
	status_calc_pc(sd, 0);
	clif_displaymessage(fd, msg_txt(MSG_OPTIONS_CHANGED)); // Options changed.
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_packet(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	static int packet_mode = 0;
	if (strstr(command, "packetmode"))
	{
		packet_mode = param[0];
		clif_displaymessage(fd, "Packet mode changed.");
	}
	else if(param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a status type/flag (usage: packet <status type> <flag>).");
		return false;
	}
	else
	{
		int x = param[0];
		int y = (param.size() == 1) ? 1: (int)param[1];

		switch (packet_mode)
		{
		case 0:
			clif_status_change(sd, x, y);
			break;
		case 1:
			sd.status.skill[sd.cloneskill_id].id=0;
			sd.status.skill[sd.cloneskill_id].lv=0;
			sd.status.skill[sd.cloneskill_id].flag=0;
			sd.cloneskill_id = x;
			sd.status.skill[x].id = x;
			sd.status.skill[x].lv = skill_get_max(x);
			sd.status.skill[x].flag = 13;//cloneskill flag
			clif_skillinfoblock(sd);
			break;
		default:
			break;
			//added later
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_party(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a party name (usage: party <party_name>).");
		return false;
	}
	party_create(sd, param[0], 0, 0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall online characters of a party to your location
///
bool command_partyrecall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	size_t i, count;
	struct map_session_data *pl_sd;
	const char *party_name=param[0];
	char output[128];
	struct party *p;

	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter a party name/id (usage: partyrecall <party_name/id>).");
		return false;
	}

	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
	{
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(party_name))) != NULL)
	{
		count = 0;
		for (i = 0; i < fd_max; ++i) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
			    sd.status.account_id != pl_sd->status.account_id &&
			    pl_sd->status.party_id == p->party_id) {
				if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
					count++;
				else
					pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
			}
		}
		snprintf(output, sizeof(output), msg_txt(MSG_ALL_ON_CHARS_S_PARTY_NEAR), p->name); // All online characters of the %s party are near you.
		clif_displaymessage(fd, output);
		if (count) {
			snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
			clif_displaymessage(fd, output);
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_NONE_OF_PARTY_ON)); // Incorrect name or ID, or no one from the party is online.
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_petfriendly(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int friendly=0;
	int t;

	if (param.size()<1 || (friendly=param[0]) < 0) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: petfriendly <0-1000>).");
		return false;
	}
	if (sd.status.pet_id > 0 && sd.pd)
	{
		if (friendly >= 0 && friendly <= 1000)
		{
			if (friendly != sd.pd->pet.intimate)
			{
				t = sd.pd->pet.intimate;
				sd.pd->pet.intimate = friendly;
				clif_send_petstatus(sd);
				if (config.pet_status_support)
				{
					if ((sd.pd->pet.intimate > 0 && t <= 0) ||
					    (sd.pd->pet.intimate <= 0 && t > 0))
					{
						if( sd.is_on_map() )
							status_calc_pc(sd, 0);
						else
							status_calc_pc(sd, 2);
					}
				}
				clif_displaymessage(fd, msg_txt(MSG_PET_FRIENDLY_VALUE_CHANGED)); // Pet friendly value changed!
			}
			else
			{
				clif_displaymessage(fd, msg_txt(MSG_PET_FRIENDLY_GOOD)); // Pet friendly is already the good value.
				return false;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_INVALID_NUMBER)); // An invalid number was specified.
			return false;
		}
	} 
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_SORRY_BUT_YOU_HAVE_NO_PET)); // Sorry, but you have no pet.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_pethungry(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned hungry=0;
	if( param.size()<1 || (hungry=param[0]) > 100 )
	{
		clif_displaymessage(fd, "Please, enter a valid number (usage: pethungry <0-100>).");
		return false;
	}

	if (sd.status.pet_id > 0 && sd.pd)
	{
		if((short)hungry != sd.pd->pet.hungry)
		{
			sd.pd->pet.hungry = hungry;
			clif_send_petstatus(sd);
			clif_displaymessage(fd, msg_txt(MSG_PET_HUNGRY_VALUE_CHANGED)); // Pet hungry value changed!
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_PET_HUNGRY_GOOD)); // Pet hungry is already the good value.
			return false;
		}

	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_SORRY_BUT_YOU_HAVE_NO_PET)); // Sorry, but you have no pet.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_petrename(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (sd.status.pet_id > 0 && sd.pd)
	{
		if (sd.pd->pet.rename_flag != 0)
		{
			sd.pd->pet.rename_flag = 0;
			intif_save_petdata(sd.status.account_id, sd.pd->pet);
			clif_send_petstatus(sd);
			clif_displaymessage(sd.fd, msg_txt(MSG_YOU_CAN_NOW_RENAME_YOUR_PET)); // You can now rename your pet.
		}
		else
		{
			clif_displaymessage(sd.fd, msg_txt(MSG_YOU_CAN_ALREADY_RENAME_YOUR_PET)); // You can already rename your pet.
			return false;
		}
	}
	else 
	{
		clif_displaymessage(fd, msg_txt(MSG_SORRY_BUT_YOU_HAVE_NO_PET)); // Sorry, but you have no pet.
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// "petid <part of pet name>"
/// => Displays a list of matching pets.
///
bool command_petid(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
		return false;

	char searchtext[128];
	char temp0[128];
	char temp1[128];
	int cnt = 0, i = 0;
	
	strcpytolower(searchtext, sizeof(searchtext), param[0]);

	snprintf(temp0, sizeof(temp0), "Search results for: %s", searchtext);
	clif_displaymessage(fd,temp0);
	while (i < MAX_PET_DB)
	{
		strcpytolower(temp1, sizeof(temp1), pet_db[i].name);
		strcpytolower(temp0, sizeof(temp0), pet_db[i].jname);
		if( strstr(temp1, searchtext) || strstr(temp0, searchtext) )
		{
			snprintf(temp0, sizeof(temp0), "ID: %i -- Name: %s", pet_db[i].class_, pet_db[i].jname);
			if(cnt >= 100)
			{	// Only if there are custom pets
				clif_displaymessage(fd, "Be more specific, can't send more than 100 results.");
			}
			else
			{
				clif_displaymessage(fd, temp0);
			}
			cnt++;
		}
		i++;
	}
	snprintf(temp0, sizeof(temp0),"%i pets have '%s' in their name.", cnt, searchtext);
	clif_displaymessage(fd, temp0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_produce(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<1)
	{
		clif_displaymessage(fd, "Please, enter at least an item name/id (usage: produce <equip name or equip ID> [element] [# of very's]).");
		return false;
	}
	char output[128];
	int item_id = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;

	const char *item_name	= param[0];
	int attribute			= param[1];
	int star				= param[2];

	if( (item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (itemdb_exists(item_id) &&
	    (item_id <= 500 || item_id > 1099) &&
	    (item_id < 4001 || item_id > 4148) &&
	    (item_id < 7001 || item_id > 10019) &&
	    itemdb_isSingleStorage(item_id)) {
		if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
			attribute = ATTRIBUTE_NORMAL;
		if (star < MIN_STAR || star > MAX_STAR)
			star = 0;
		memset(&tmp_item, 0, sizeof tmp_item);
		tmp_item.nameid = item_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		tmp_item.card[0] = 0x00ff;
		tmp_item.card[1] = ((star * 5) << 8) + attribute;
		tmp_item.card[2] = basics::GetWord(sd.status.char_id, 0);
		tmp_item.card[3] = basics::GetWord(sd.status.char_id, 1);
		clif_produceeffect(sd, item_id, 0);
		clif_misceffect(sd, 3);
		if ((flag = pc_additem(sd, tmp_item, 1)))
			clif_additem(sd, 0, 0, flag);
	} 
	else
	{
		if (config.error_log)
			ShowMessage("produce NOT WEAPON [%d]\n", item_id);
		if (item_id != 0 && itemdb_exists(item_id))
			snprintf(output, sizeof(output), msg_txt(MSG_THIS_ITEM_D_S_IS_NOT_AN_EQUIPMENT), item_id, item_data->name); // This item (%d: '%s') is not an equipment.
		else
			snprintf(output, sizeof(output), msg_txt(MSG_THIS_ITEM_IS_NOT_AN_EQUIPMENT)); // This item is not an equipment.
		clif_displaymessage(fd, output);
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_pvpoff(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;


	if (config.pk_mode) { //disable command if server is in PK mode
		clif_displaymessage(fd, msg_txt(MSG_NOT_IN_PK)); // This option cannot be used in PK Mode.
		return false;
	}

	if (maps[sd.block_list::m].flag.pvp) {
		maps[sd.block_list::m].flag.pvp = 0;
		clif_send0199(sd.block_list::m, 0);
		for (i = 0; i < fd_max; ++i) {	//?l??h?a???[?v
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
				if (sd.block_list::m == pl_sd->block_list::m) {
					clif_pvpset(*pl_sd, 0, 0, 2);
					if (pl_sd->pvp_timer != -1) {
						delete_timer(pl_sd->pvp_timer, pc_calc_pvprank_timer);
						pl_sd->pvp_timer = -1;
					}
				}
			}
		}
		clif_displaymessage(fd, msg_txt(MSG_PVP_OFF)); // PvP: Off.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_PVP_IS_ALREADY_OFF)); // PvP is already Off.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_pvpon(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;


	if (config.pk_mode) { //disable command if server is in PK mode
		clif_displaymessage(fd, msg_txt(MSG_NOT_IN_PK)); // This option cannot be used in PK Mode.
		return false;
	}

	if (!maps[sd.block_list::m].flag.pvp && !maps[sd.block_list::m].flag.nopvp) {
		maps[sd.block_list::m].flag.pvp = 1;
		clif_send0199(sd.block_list::m, 1);
		for (i = 0; i < fd_max; ++i)
		{
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
			{
				if (sd.block_list::m == pl_sd->block_list::m && pl_sd->pvp_timer == -1)
				{
					pl_sd->pvp_timer = add_timer(gettick() + 200, pc_calc_pvprank_timer, pl_sd->block_list::id, 0);
					pl_sd->pvp_rank = 0;
					pl_sd->pvp_lastusers = 0;
					pl_sd->pvp_point = 5;
					pl_sd->pvp_won = 0;
					pl_sd->pvp_lost = 0;
				}
			}
		}
		clif_displaymessage(fd, msg_txt(MSG_PVP_ON)); // PvP: On.
	} else {
		clif_displaymessage(fd, msg_txt(MSG_PVP_IS_ALREADY_ON)); // PvP is already On.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_questskill(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	short skill_id=0;
	if (param.size()<1 || (skill_id = param[0]) < 0)
	{
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: questskill <#:0+>).");
		return false;
	}
	if (skill_id >= 0 && skill_id < MAX_SKILL_DB)
	{
		if (skill_get_inf2(skill_id) & 0x01)
		{
			if (pc_checkskill(sd, skill_id) == 0)
			{
				pc_skill(sd, skill_id, 1, 0);
				clif_displaymessage(fd, msg_txt(MSG_YOU_LEARNED_SKILL)); // You have learned the skill.
			}
			else
			{
				clif_displaymessage(fd, msg_txt(MSG_YOU_ALREADY_HAVE_THIS_QUEST_SKILL)); // You already have this quest skill.
				return false;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_SKILL_NOT_EXIST_OR_NO_QUEST_SKILL)); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_SKILL_NOT_EXIST)); // This skill number doesn't exist.
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
void command_raise_sub(struct map_session_data& sd)
{
	if( sd.state.auth && sd.is_dead() )
	{
		clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
		sd.status.hp = sd.status.max_hp;
		sd.status.sp = sd.status.max_sp;
		sd.set_stand();
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		clif_resurrection(sd, 1);
		if (config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, config.pc_invincible_time);
		clif_displaymessage(sd.fd, msg_txt(MSG_MERCY_HAS_BEEN_SHOWN)); // Mercy has been shown.
	}
}
bool command_raise(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i;
	for (i = 0; i < fd_max; ++i)
	{
		if(session[i] && (pl_sd=(struct map_session_data *)session[i]->user_session) && pl_sd->state.auth && sd.block_list::m == pl_sd->block_list::m)
			command_raise_sub(*pl_sd);
	}
	clif_displaymessage(fd, msg_txt(MSG_MERCY_HAS_BEEN_GRANTED)); // Mercy has been granted.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_rates(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "Experience rates: Base %lf.1x / Job %lf.1x",
		(double)config.base_exp_rate/100., (double)config.job_exp_rate/100.);
	clif_displaymessage(fd, buf);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool command_recall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];

	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: recall <char name>/<account id>/<char id>).");
		return false;
	}

	struct map_session_data *pl_sd = CommandInfo::param2sd(param[0]);
	
	if( pl_sd != NULL )
	{
		if( sd.block_list::id == pl_sd->block_list::id ) //Yourself mate? Tsk tsk tsk.
			return false;

		if (sd.isGM() >= pl_sd->isGM())
		{	// you can recall only lower or same level
			if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
			{
				clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
				return false;
			}
			if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
			{
				clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
				return false;
			}
			pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
			snprintf(output, sizeof(output), msg_txt(MSG_S_RECALLED), pl_sd->status.name); // %s recalled!
			clif_displaymessage(fd, output);
		}
		else
		{
			clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall All Characters Online To Your Location

bool command_recallall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	char output[128];
	size_t i, count;

	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
	{
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}
	count = 0;
	for (i = 0; i < fd_max; ++i)
	{
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && sd.status.account_id != pl_sd->status.account_id &&
			sd.isGM() >= pl_sd->isGM() )
		{
			if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
				count++;
			else
				pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
		}
	}
	clif_displaymessage(fd, msg_txt(MSG_ALL_CHARS_RECALLED)); // All characters recalled!
	if (count)
	{
		snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
		clif_displaymessage(fd, output);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_refine(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<2 )
	{
		clif_displaymessage(fd, "Please, enter a position and a amount (usage: refine <equip position> <+/- amount>).");
		return false;
	}
	int i, current_position, final_refine;
	int count;
	char output[128];

	int position = param[0];
	int refine   = param[1];

	if (refine < -10)
		refine = -10;
	else if (refine > 10)
		refine = 10;
	else if (refine == 0)
		refine = 1;

	count = 0;
	for (i = 0; i < MAX_INVENTORY; ++i)
	{
		if (sd.status.inventory[i].nameid &&	// ?Y?g??A???I?e??ho?d??C?B??E?e
		    (sd.status.inventory[i].equip & position ||
			(sd.status.inventory[i].equip && !position)))
		{
			final_refine = sd.status.inventory[i].refine + refine;
			if (final_refine > 10)
				final_refine = 10;
			else if (final_refine < 0)
				final_refine = 0;
			if (sd.status.inventory[i].refine != final_refine) {
				sd.status.inventory[i].refine = final_refine;
				current_position = sd.status.inventory[i].equip;
				pc_unequipitem(sd, i, 3);
				clif_refine(fd, sd, 0, i, sd.status.inventory[i].refine);
				clif_delitem(sd, i, 1);
				clif_additem(sd, i, 1, 0);
				pc_equipitem(sd, i, current_position);
				clif_misceffect(sd, 3);
				count++;
			}
		}
	}
	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_ITEM_HAS_BEEN_REFINED)); // No item has been refined!
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_1_ITEM_HAS_BEEN_REFINED)); // 1 item has been refined!
	else
	{
		snprintf(output, sizeof(output), msg_txt(MSG_D_ITEMS_HAVE_BEEN_REFINED), count); // %d items have been refined!
		clif_displaymessage(fd, output);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// refresh (like "jumpto <yourself>")
///
bool command_refresh(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	clif_refresh(sd);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Refresh online command for SQL
/// Will refresh and check online column of
/// players and set correctly.
///
bool command_refreshonline(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char_online_check();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// reloadatcommand command_athena.conf
///
bool command_reloadatcommand(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	CommandInfo::config_read(ATCOMMAND_CONF_FILENAME);
	clif_displaymessage(fd, msg_txt(MSG_GM_COMMANDS_RELOADED));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// reloadbattleconf battle_athena.conf
///
bool command_reloadbattleconf(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	config.read(BATTLE_CONF_FILENAME);
	clif_displaymessage(fd, msg_txt(MSG_BATTLE_CONFIG_RELOADED));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_reloaditemdb(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	itemdb_reload();
	clif_displaymessage(fd, msg_txt(MSG_ITEM_DATABASE_RELOADED)); // Item database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_reloadmobdb(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	mob_reload();
	do_final_pet();
	read_petdb();
	clif_displaymessage(fd, msg_txt(MSG_MONSTER_DATABASE_RELOADED)); // Monster database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// reloadpcdb
///  exp.txt skill_tree.txt attr_fix.txt
///
bool command_reloadpcdb(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_readdb();
	clif_displaymessage(fd, msg_txt(MSG_PLAYER_CONFIG_RELOADED));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_reloadscript(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	command_broadcast( fd, sd, "broadcast", basics::CParameterList("\"eAthena Server is Rehashing...\""));
	command_broadcast( fd, sd, "broadcast", basics::CParameterList("\"You will feel a bit of lag at this point !\""));
	command_broadcast( fd, sd, "broadcast", basics::CParameterList("\"Reloading NPCs...\""));

	flush_fifos();

	do_init_script();
	npc_reload();
	npc_event_do_oninit();

	clif_displaymessage(fd, msg_txt(MSG_REFUSE_ALL_WISPS)); // Scripts reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_reloadskilldb(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	skill_reload();
	clif_displaymessage(fd, msg_txt(MSG_SKILL_DATABASE_RELOADED)); // Skill database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// reloadstatusdb
///  job_db1.txt job_db2.txt job_db2-2.txt
///  refine_db.txt size_fix.txt
///  
bool command_reloadstatusdb(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	status_readdb();
	clif_displaymessage(fd, msg_txt(MSG_STATUS_CONFIG_RELOADED));
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// repairall
///
bool command_repairall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int count, i;
	count = 0;
	for(i=0; i<MAX_INVENTORY; ++i)
	{
		if (sd.status.inventory[i].nameid && sd.status.inventory[i].attribute == 1)
		{
			sd.status.inventory[i].attribute = 0;
			clif_produceeffect(sd, sd.status.inventory[i].nameid, 0);
			count++;
		}
	}
	if (count > 0)
	{
		clif_misceffect(sd, 3);
		clif_equiplist(sd);
		clif_displaymessage(fd, msg_txt(MSG_ALL_ITEMS_HAVE_BEEN_REPAIRED)); // All items have been repaired.
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_NO_ITEM_NEED_TO_BE_REPAIRED)); // No item need to be repaired.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_revive(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a player name (usage: revive <char name>/<account id>/<char id>).");
		return false;
	}

	struct map_session_data *pl_sd = CommandInfo::param2sd(param[0]);

	if( pl_sd != NULL )
	{
		if( pl_sd->is_dead() )
		{
			pl_sd->status.hp = pl_sd->status.max_hp;
			clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
			pl_sd->set_stand();
			if (config.pc_invincible_time > 0)
				pc_setinvincibletimer(*pl_sd, config.pc_invincible_time);
			clif_updatestatus(*pl_sd, SP_HP);
			clif_updatestatus(*pl_sd, SP_SP);
			clif_resurrection(*pl_sd, 1);
			clif_displaymessage(fd, msg_txt(MSG_CHAR_REVIVED)); // Character revived.
			return true;
		}
	}
	else
		clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_save(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	unsigned short m = sd.block_list::m;
	unsigned short x = sd.block_list::x;
	unsigned short y = sd.block_list::y;

	if( param.size() && 
		( param.size()<3 || (m = map_mapname2mapid(param[0]))>=map_num ||
		(x=param[1])<=0 || x>=maps[m].xs || (y=param[2])<=0 || y>=maps[m].ys ) )
	{
		clif_displaymessage(fd, "Please, enter a valid save point and a player name (usage: #save <map> <x> <y> <charname>).");
	}
	else if( m<map_num && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
	{
		clif_displaymessage(fd, "You are not authorised to set this map as a save map.");
	}
	else
	{
		pc_setsavepoint(sd, sd.mapname, sd.block_list::x, sd.block_list::y);
		if(sd.status.pet_id > 0 && sd.pd)
			intif_save_petdata(sd.status.account_id, sd.pd->pet);
		pc_makesavestatus(sd);
		chrif_save(sd);
		storage_storage_save(sd);
		clif_displaymessage(fd, msg_txt(MSG_CHAR_RESPAWN_SAVED)); // Character data respawn point saved.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// send. (used for testing packet sends from the client)
///
/// usage: "send <hex digit> <decimal digit>{20,20}"
///
bool command_send(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
   	if( param.size() < 1 )
	{
		clif_displaymessage(fd, "Please enter a packet number, and - if required - up to 20 additional values.");
		return false;
	}

	char output[128]="";
	int type = param[0];
	int info[20];
	size_t i;
	for(i=0; i<20; ++i)
		info[i] = param[i+1];

	if( clif_packetsend(fd, sd, type, info, sizeof(info)) )
	{
		snprintf(output, sizeof(output), msg_txt(MSG_SEND_PACKET_X_D), type, type);
		clif_displaymessage(fd, output);
	}
	else
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_PACKET));
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// time in txt for time command ()
/// 
const char* txt_time(char* buffer, size_t sz, unsigned long duration)
{
	if(buffer)
	{
		unsigned long days, hours, minutes, seconds;
		if(duration < 0)
			duration = 0;

		days = duration / (60 * 60 * 24);
		duration -= (60 * 60 * 24 * days);
		hours = duration / (60 * 60);
		duration -= (60 * 60 * hours);
		minutes = duration / 60;
		seconds = duration - (60 * minutes);

		snprintf(buffer, sz, "%ld %s %ld %s %ld %s %ld %s",
				days,		msg_txt((days==1)	?MSG_DAY   :MSG_DAYS   ),
				hours,		msg_txt((hours==1)	?MSG_HOUR  :MSG_HOURS  ),
				minutes,	msg_txt((minutes==1)?MSG_MINUTE:MSG_MINUTES),
				seconds,	msg_txt((seconds==1)?MSG_SECOND:MSG_SECONDS) );
		buffer[sz-1]=0;
	}
	return buffer;
}

///////////////////////////////////////////////////////////////////////////////
///
/// time/date/server_date/serverdate/server_time/servertime: Display the date/time of the server (
/// Calculation management of GM modification (day/night GM commands) is done
///
bool command_servertime(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct TimerData * timer_data;
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[256]="", temp2[256];

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_txt(MSG_SERVER_TIME__A_B_D_Y_X), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, temp);


	if (config.night_duration == 0 && config.day_duration == 0)
	{
		clif_displaymessage(fd, msg_txt((daynight_flag == 0)?MSG_PERMANENT_DAYLIGHT:MSG_PERMANENT_NIGHT)); 
		// Game time: The game is in permanent daylight.
		// Game time: The game is in permanent night.
	}
	else if (config.night_duration == 0)
	{
		if (daynight_flag == 1)
		{	// we have night
			timer_data = get_timer(daynight_timer_tid);
			snprintf(temp, sizeof(temp), msg_txt(MSG_CURRENTLY_IN_NIGHT_FOR_S), txt_time(temp2, sizeof(temp2), (timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_txt(MSG_AFTER_PERMANENT_DAYLIGHT)); // Game time: After, the game will be in permanent daylight.
		}
		else
			clif_displaymessage(fd, msg_txt(MSG_PERMANENT_DAYLIGHT)); // Game time: The game is in permanent daylight.
	}
	else if (config.day_duration == 0)
	{
		if (daynight_flag == 0)
		{	// we have day
			timer_data = get_timer(daynight_timer_tid);
			snprintf(temp, sizeof(temp), msg_txt(MSG_CURRENTLY_IN_DAYLIGHT_FOR_S), txt_time(temp2, sizeof(temp2), (timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_txt(MSG_AFTER_PERMANENT_NIGHT)); // Game time: After, the game will be in permanent night.
		} else
			clif_displaymessage(fd, msg_txt(MSG_PERMANENT_NIGHT)); // Game time: The game is in permanent night.
	}
	else
	{
		timer_data = get_timer(daynight_timer_tid);
		if (daynight_flag == 0)
		{	// we have day
			snprintf(temp, sizeof(temp), msg_txt(MSG_CURRENTLY_IN_DAYLIGHT_FOR_S), txt_time(temp2, sizeof(temp2), (timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_txt(MSG_AFTER_NIGHT_FOR_S), txt_time(temp2, sizeof(temp2), config.night_duration / 1000)); // Game time: After, the game will be in night for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_txt(MSG_DAY_CYCLE_DURATION_OF_S), txt_time(temp2, sizeof(temp2), (config.day_duration+config.night_duration) / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		}
		else
		{
			snprintf(temp, sizeof(temp), msg_txt(MSG_CURRENTLY_IN_NIGHT_FOR_S), txt_time(temp2, sizeof(temp2), (timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_txt(MSG_AFTER_DAYLIGHT_FOR_S), txt_time(temp2, sizeof(temp2), config.day_duration / 1000)); // Game time: After, the game will be in daylight for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_txt(MSG_DAY_CYCLE_DURATION_OF_S), txt_time(temp2, sizeof(temp2), (config.day_duration+config.night_duration) / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// setbattleflag
/// set a config flag without having to reboot
///
bool command_setbattleflag(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char *flag = param[0];
	const char *value= param[1];

	if (param.size()<2)
	{
		clif_displaymessage(fd, "Usage: setbattleflag <flag> <value>.");
		return false;
	}
	if( config.set_value(flag, value) )
	{
		clif_displaymessage(fd, "config set as requested");
		config.validate();
	}
	else
		clif_displaymessage(fd, "unknown config flag");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_showexp(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	sd.state.noexp = (param.size()) ?
		!((bool)param[0]) : !sd.state.noexp;

	clif_displaymessage(fd, (sd.state.noexp)?
		"Gained exp is now NOT shown":"Gained exp is now shown");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_showdelay(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	sd.state.nodelay = (param.size()) ?
		!((bool)param[0]) : !sd.state.nodelay;

	clif_displaymessage(fd, (sd.state.nodelay)?
		"Skill delay failure is NOT now shown":"Skill delay failure is now shown");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// shuffle
///
class CAtShuffle : public CMapProcessor
{
public:
	CAtShuffle()	{}
	~CAtShuffle()	{}
	virtual int process(block_list& bl) const
	{
		struct map_session_data *sd = bl.get_sd();
		if( sd && !sd->isGM() )
			pc_setpos(*sd, sd->mapname, rand() % maps[sd->block_list::m].xs, rand() % maps[sd->block_list::m].ys, 3);
		return 0;
	}
};
bool command_shuffle(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char* message = param[0];
	if(strcmp(message, "area")== 0)
	{
		block_list::foreachinarea( CAtShuffle(),
			sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE, ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_PC);
	}
	else if(strcmp(message, "map")== 0)
	{
		block_list::foreachinarea( CAtShuffle(),
			sd.block_list::m, 0, 0, maps[sd.block_list::m].xs, maps[sd.block_list::m].ys, BL_PC);
	}
	else if(strcmp(message, "world") == 0)
	{
		struct map_session_data *pl_sd;
		size_t i;
		CAtShuffle cs;
		for (i = 0; i < fd_max; ++i)
		{
			if(session[i] && (pl_sd = (struct map_session_data *)session[i]->user_session) != NULL && pl_sd->state.auth)
				cs.process(*pl_sd);
		}
	}
	else
		clif_displaymessage(fd, "options are <area>, <map>, or <world>");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// size
///
bool command_size(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int size=param[0];
	if(size==0 && sd.state.viewsize)
	{
		sd.state.viewsize=0;
		pc_setpos(sd, sd.mapname, sd.block_list::x, sd.block_list::y, 3);
	}
	else if(size==1)
	{
		sd.state.viewsize=1;
		clif_specialeffect(sd,420,0);
	}
	else if(size==2)
	{
		sd.state.viewsize=2;
		clif_specialeffect(sd,422,0);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// skillid
/// lookup a skill by name
///
bool command_skillid(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
		return false;
	
	size_t idx;
	const char*name= param[0];
	size_t len = strlen(name);
	
	for(idx = 0; idx < MAX_SKILL_DB; ++idx)
	{
		if( (skill_db[idx].name && strncasecmp(skill_db[idx].name, name, len) == 0) ||
			(skill_db[idx].desc && strncasecmp(skill_db[idx].desc, name, len) == 0) )
		{
			char output[256];
			snprintf(output, sizeof(output),"skill %s (id=%ld): %s", 
				skill_db[idx].name, (unsigned long)(idx), skill_db[idx].desc);
			clif_displaymessage(fd, output);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// skillon
/// turn skills on for the map
///
bool command_skillon(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	maps[sd.block_list::m].flag.noskill = 0;
	clif_displaymessage(fd, msg_txt(MSG_MAP_SKILLS_ARE_ON));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// skilloff
/// Turn skills off on the map
///
bool command_skilloff(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{

	maps[sd.block_list::m].flag.noskill = 1;
	clif_displaymessage(fd, msg_txt(MSG_MAP_SKILLS_ARE_OFF));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// skpoint
///
bool command_skillpoint(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int point=0;
	if( param.size()<1 || (point = param[0]) == 0 )
	{
		clif_displaymessage(fd, "Please, enter a number (usage: stpoint <number of points>).");
	}
	else
	{
		int new_skill_point = (int)sd.status.skill_point + point;
		if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
			new_skill_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
			new_skill_point = 0;
		if (new_skill_point != (int)sd.status.skill_point)
		{
			sd.status.skill_point = (short)new_skill_point;
			clif_updatestatus(sd, SP_SKILLPOINT);
			clif_displaymessage(fd, msg_txt(MSG_SKILL_POINTS_CHANGED)); // Number of skill points changed!
			return true;
		}
		else
		{
			// Impossible to decrease the number/value.
			// Impossible to increase the number/value.
			clif_displaymessage(fd, msg_txt((point < 0)?MSG_IMPOSSIBLE_TO_DECREASE:MSG_IMPOSSIBLE_TO_INCREASE)); 
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Skill Reset
///
bool command_skillreset(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_resetskill(sd);
	clif_displaymessage(fd, "skill points reseted");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Stat Reset
bool command_statusreset(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_resetstate(sd);
	clif_displaymessage(fd, "stats points reseted");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Reset
///
bool command_charreset(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	pc_resetstate(sd);
	pc_resetskill(sd);
	clif_displaymessage(fd, "stats & skill points reseted");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// skilltree
/// prints the skill tree for a player required to get to a skill
///
bool command_skilltree(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (param.size()<2)
		return false;

	int skillnum       = param[0];
	const char *target = param[1];
	struct map_session_data *pl_sd = CommandInfo::param2sd(target);
	if( pl_sd == NULL )
		return false;

	char output[128];
	int skillidx = -1;
	int meets = 1, j;
	struct skill_tree_entry *ent;
	int c = pc_calc_skilltree_normalize_job(*pl_sd);

	snprintf(output, sizeof(output), "Player is using %s skill tree (%d basic points)",
		job_name(c), pc_checkskill(*pl_sd, 1));
	clif_displaymessage(fd, output);

	for (j = 0; skill_tree[c][j].id != 0; ++j)
	{
		if (skill_tree[c][j].id == skillnum)
		{
			skillidx = j;
			break;
		}
	}
	if (skillidx == -1)
	{
		snprintf(output, sizeof(output), "I do not believe the player can use that skill");
		clif_displaymessage(fd, output);
		return false;
	}
	ent = &skill_tree[c][skillidx];
	for(j=0;j<5; ++j)
	{
		if( ent->need[j].id && pc_checkskill(sd,ent->need[j].id) < ent->need[j].lv)
		{
			int idx = 0;
			char *desc;
			while (skill_names[idx].id != 0 && skill_names[idx].id != ent->need[j].id)
				idx++;
			if (skill_names[idx].id == 0)
				desc = "Unknown skill";
			else
				desc = skill_names[idx].desc;
			snprintf(output, sizeof(output), "player requires level %d of skill %s",
				ent->need[j].lv,  desc);
			clif_displaymessage(fd, output);
			meets = 0;
			break;
		}
	}
	if (meets == 1)
	{
		snprintf(output, sizeof(output), "I believe the player meets all the requirements for that skill");
		clif_displaymessage(fd, output);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Sound Command - plays a sound for everyone!
///
bool command_sound(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a sound filename. (usage: sound <filename>)");
		return false;
	}
	char sound_file[128];
	safestrcpy(sound_file, sizeof(sound_file)-4, param[0]);
	if( strstr(sound_file, ".wav") == NULL )
		strcat(sound_file, ".wav");
	clif_soundeffectall(sd, sound_file,0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_speed(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int speed = param[0];
	char output[128];

	if (speed >= MIN_WALK_SPEED && speed <= MAX_WALK_SPEED)
	{
		sd.speed = speed;
		clif_updatestatus(sd, SP_SPEED);
		clif_displaymessage(fd, msg_txt(MSG_SPEED_CHANGED)); // Speed changed.
	}
	else
	{
		snprintf(output, sizeof(output), "Please, enter a valid speed value (usage: speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, output);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_spiritball(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int number = param[0];
	if( param.size()<1 )
	{
		clif_displaymessage(fd, "Please, enter a spirit ball number (usage: spiritball <number: 0-1000>).");
	}
	else if(number < 0 || number > 0x7FFF)
	{
		clif_displaymessage(fd, msg_txt(MSG_INVALID_NUMBER)); // An invalid number was specified.
	}
	else if (sd.spiritball == number)
	{
		clif_displaymessage(fd, msg_txt(MSG_ALREADY_HAS_SPIRITBALLS)); // You already have this number of spiritballs.
	}
	else
	{
		if (number > 500)
		{	 // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
			number = 500;
			clif_displaymessage(fd, msg_txt(MSG_TOO_MANY_SPIRITBALLS));
		}
		if (sd.spiritball > 0)
			pc_delspiritball(sd, sd.spiritball, 1);
		sd.spiritball = number;
		clif_spiritball(sd);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_status(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	size_t i;
	int value = 0, new_value;
	char output[128];
	const char* commandlist[] = { "str", "agi", "vit", "int", "dex", "luk" };
	unsigned short* status[]  = { &sd.status.str,  &sd.status.agi, &sd.status.vit,
								  &sd.status.int_, &sd.status.dex, &sd.status.luk };

	if( param.size()<1 || (value=param[0]) == 0 )
	{
		snprintf(output, sizeof(output), "Please, enter a valid value (usage: str,agi,vit,int,dex,luk <+/-adjustement>).");
		clif_displaymessage(fd, output);
		return false;
	}

	for (i=0; i<sizeof(commandlist)/sizeof(*commandlist); ++i)
	{
		if( 0==strcasecmp(command, commandlist[i]) )
		{
			new_value = *status[i] + value;
			if(value > 0 && (value > (int)config.max_parameter || new_value > (int)config.max_parameter)) // fix positiv overflow
				new_value = config.max_parameter;
			else if(value < 0 && (value < -((int)config.max_parameter) || new_value < 1)) // fix negativ overflow
				new_value = 1;

			if(new_value != (int)*status[i])
			{
				*status[i] = new_value;
				clif_updatestatus(sd, SP_STR + i);
				clif_updatestatus(sd, SP_USTR + i);
				status_calc_pc(sd, 0);
				clif_displaymessage(fd, msg_txt(MSG_STAT_CHANGED)); // Stat changed.
				return true;
			}
			else
			{
				// Impossible to decrease the number/value.
				// Impossible to increase the number/value.
				clif_displaymessage(fd, msg_txt((value < 0)?MSG_IMPOSSIBLE_TO_DECREASE:MSG_IMPOSSIBLE_TO_INCREASE));
				return false;
			}
		}
	}
	// fallen off the array
	snprintf(output, sizeof(output), "Please, enter a valid value (usage: str,agi,vit,int,dex,luk <+/-adjustement>).");
	clif_displaymessage(fd, output);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Stat all
bool command_stat_all(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int index, count, value = 0, new_value;
	unsigned short* status[] = {
		&sd.status.str,  &sd.status.agi, &sd.status.vit,
		&sd.status.int_, &sd.status.dex, &sd.status.luk
	};

	if (param.size()<1 || (value=param[0]) == 0)
		value = config.max_parameter;

	count = 0;
	for (index = 0; index < (int)(sizeof(status) / sizeof(status[0])); ++index) {

		new_value = (int)*status[index] + value;
		if(value > 0 && (value > (int)config.max_parameter || new_value > (int)config.max_parameter)) // fix positiv overflow
			new_value = config.max_parameter;
		else if(value < 0 && (value < -((int)config.max_parameter) || new_value < 1)) // fix negativ overflow
			new_value = 1;

		if (new_value != (int)*status[index]) {
			*status[index] = new_value;
			clif_updatestatus(sd, SP_STR + index);
			clif_updatestatus(sd, SP_USTR + index);
			status_calc_pc(sd, 0);
			count++;
		}
	}

	if (count > 0) // if at least 1 stat modified
		clif_displaymessage(fd, msg_txt(MSG_ALL_STATS_CHANGED)); // All stats changed!
	else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(MSG_IMPOSSIBLE_TO_DECREASE_A_STAT)); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_txt(MSG_IMPOSSIBLE_TO_INCREASE_A_STAT)); // Impossible to increase a stat.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
bool command_printstats(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[1024], gmlevel[1024];
	struct map_session_data *pl_sd;
	size_t i, count;

	if( param.size() && param[0]!="all" )
	{	// given a player name, so only display this
		pl_sd = CommandInfo::param2sd(param[0]);
		if( !pl_sd )
		{
			clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
			return false;
		}
		else
		{
			char job_jobname[128];
			struct
			{
				const char* format;
				int value;
			}
			output_table[] =
			{
				{ "Base Level - %d", pl_sd->status.base_level },
				{ job_jobname, pl_sd->status.job_level },
				{ "Hp - %d",    pl_sd->status.hp },
				{ "MaxHp - %d", pl_sd->status.max_hp },
				{ "Sp - %d",    pl_sd->status.sp },
				{ "MaxSp - %d", pl_sd->status.max_sp },
				{ "Str - %3d",  pl_sd->status.str },
				{ "Agi - %3d",  pl_sd->status.agi },
				{ "Vit - %3d",  pl_sd->status.vit },
				{ "Int - %3d",  pl_sd->status.int_ },
				{ "Dex - %3d",  pl_sd->status.dex },
				{ "Luk - %3d",  pl_sd->status.luk },
				{ "Zeny - %d",  pl_sd->status.zeny },
				{ NULL, 0 }
			};
			snprintf(job_jobname, sizeof(job_jobname), "Job - %s %s", job_name(pl_sd->status.class_), "(level %d)");
			snprintf(output, sizeof(output), msg_txt(MSG_S_STATS), pl_sd->status.name); // '%s' stats:
			clif_displaymessage(fd, output);
			for (i = 0; output_table[i].format != NULL; ++i)
			{
				snprintf(output, sizeof(output), output_table[i].format, output_table[i].value);
				clif_displaymessage(fd, output);
			}
		}
	}
	else
	{	// run through all online players otherwise
		for(count=0,i=0; i<fd_max; ++i)
		{
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
			{
				if(sd.isGM() && pl_sd->isGM())
					snprintf(gmlevel, sizeof(gmlevel), "| GM Lvl: %d", pl_sd->isGM());
				else
					snprintf(gmlevel, sizeof(gmlevel), " ");
				
				snprintf(output, sizeof(output), "Name: %s | BLvl: %d | Job: %s (Lvl: %d) | HP: %ld/%ld | SP: %ld/%ld", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level, (unsigned long)pl_sd->status.hp, (unsigned long)pl_sd->status.max_hp, (unsigned long)pl_sd->status.sp, (unsigned long)pl_sd->status.max_sp);
				clif_displaymessage(fd, output);
				snprintf(output, sizeof(output), "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %ld %s", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, (unsigned long)pl_sd->status.zeny, gmlevel);
				clif_displaymessage(fd, output);
				clif_displaymessage(fd, "--------");
				++count;
			}
		}
		if (count == 0)
		{
			clif_displaymessage(fd, msg_txt(MSG_NO_PLAYER_FOUND)); // No player found.
		}
		else if (count == 1)
		{
			clif_displaymessage(fd, msg_txt(MSG_ONE_PLAYER_FOUND)); // 1 player found.
		}
		else
		{
			snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_FOUND), count); // %d players found.
			clif_displaymessage(fd, output);
		}
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// stpoint
///
bool command_statuspoint(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	int point=0;
	if( param.size()<1 || (point = param[0]) == 0 )
	{
		clif_displaymessage(fd, "Please, enter a number (usage: stpoint <number of points>).");
	}
	else
	{
		int new_status_point = (int)sd.status.status_point + point;
		if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
			new_status_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
			new_status_point = 0;
		
		if (new_status_point != (int)sd.status.status_point)
		{
			sd.status.status_point = (short)new_status_point;
			clif_updatestatus(sd, SP_STATUSPOINT);
			clif_displaymessage(fd, msg_txt(MSG_STATUS_POINTS_CHANGED)); // Number of status points changed!
			return true;
		}
		else
		{
			// Impossible to decrease the number/value.
			// Impossible to increase the number/value.
			clif_displaymessage(fd, msg_txt((point < 0)?MSG_IMPOSSIBLE_TO_DECREASE:MSG_IMPOSSIBLE_TO_INCREASE)); 
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_storage(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct pc_storage *stor;
	if( sd.state.storage_flag == 1 ||
		((stor = account2storage2(sd.status.account_id)) != NULL && stor->storage_status == 1) )
	{
		clif_displaymessage(fd, msg_txt(MSG_STORAGE_ALREADY_OPEN));
		return false;
	}
	storage_storageopen(sd);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// usage: "storeall [charname/chrid/accid]"
/// Put everything into storage to simplify your inventory to make debugging easier
///
bool command_storeall(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if (storage_storageopen(sd) == 1)
	{
		clif_displaymessage(fd, "Had to open the characters storage window...");
		clif_displaymessage(fd, "run this command again..");
		return true;
	}
	size_t i;
	for(i=0; i<MAX_INVENTORY; ++i)
	{
		if (sd.status.inventory[i].amount)
		{
			if(sd.status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			storage_storageadd(sd,  i, sd.status.inventory[i].amount);
		}
	}
	storage_storageclose(sd);

	// initiator and working object are different
	if( sd.fd != fd )
	{
		clif_displaymessage(sd.fd, "Everything you own has been put away for safe keeping.");
		clif_displaymessage(sd.fd, "go to the nearest kafka to retrieve it..");
		clif_displaymessage(sd.fd, "   -- the management");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_summon(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<1)
		return false;
	
	uint32 mob_id = 0;
	int x = 0;
	int y = 0;
	uint32 id = 0;
	struct mob_data *md;
	unsigned long tick=gettick();

	const char *name = param[0];
	int duration = param[1];

	if (duration < 1)
		duration =1;
	else if (duration > 60)
		duration =60;

	if((mob_id = atoi(name)) == 0)
		mob_id = mobdb_searchname(name);
	if(mob_id == 0)
		return false;

	x = sd.block_list::x + (rand() % 10 - 5);
	y = sd.block_list::y + (rand() % 10 - 5);

	id = mob_once_spawn(&sd,"this", x, y, "--ja--", mob_id, 1, "");

	if( (md=mob_data::from_blid(id)) )
	{
		md->master_id=sd.block_list::id;
		md->state.special_mob_ai=1;
		md->mode = mob_db[md->class_].mode|0x04;
		md->deletetimer=add_timer(tick+duration*60000,mob_timer_delete,id,0);
		clif_setareaeffect(*md,344);
	}
	clif_skill_poseffect(sd,AM_CALLHOMUN,1,x,y,tick);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// trade
/// Open a trade window with a remote player
///
bool command_trade(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
    if (param.size()<1)
        return false;
    struct map_session_data *pl_sd =CommandInfo::param2sd(param[0]);
    if( pl_sd != NULL )
	{
		trade_traderequest(sd, pl_sd->block_list::id);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// unmute
///
bool command_unmute(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(!config.muting_players)
	{
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}
	if(sd.sc_data[SC_NOCHAT].timer!=-1)
	{
		sd.status.manner = 0; // have to set to 0 first
		status_change_end(&sd,SC_NOCHAT,-1);
		clif_displaymessage(fd,"unmuted");
	}
	else
		clif_displaymessage(fd,"not muted");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// uptime
///
bool command_uptime(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char output[128];
	unsigned long days, hours, minutes,seconds;

	uptime::getvalues(days, hours, minutes,seconds);
	snprintf(output, sizeof(output), msg_txt(MSG_SERVER_UPTIME), days, hours, minutes, seconds);
	clif_displaymessage(fd,output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// display number of users on maps
///
bool command_users(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char buf[256];
	size_t i, all;

	//## replace with querying the number of items in charid_db
	for(i=0, all=0; i<map_num; ++i)
	{
		all += maps[i].users;
	}
	for(i=0; i<map_num; ++i)
	{
		snprintf(buf, sizeof(buf), "%s : %ld (%ld%%)",maps[i].mapname, (unsigned long)maps[i].users,(unsigned long)(maps[i].users*100/all));
		clif_displaymessage(fd,buf);
	}
	snprintf(buf,sizeof(buf), "all : %ld",(unsigned long)all);
	clif_displaymessage(fd,buf);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// useskill
/// A way of using skills without having to find them in the skills menu
///
bool command_useskill(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	if(param.size()<3)
	{
		clif_displaymessage(fd, "Usage: useskill <skillnum> <skillv> <target>");
		return false;
	}

	int skillnum       = param[0];
	int skilllv        = param[1];
	const char *target = param[2];
	struct map_session_data *pl_sd = CommandInfo::param2sd(target);
	if( pl_sd )
	{
		int inf = skill_get_inf(skillnum);
		if( (inf == 2) || (inf == 1) )
			skill_use_pos(&sd, pl_sd->block_list::x, pl_sd->block_list::y, skillnum, skilllv);
		else
			skill_use_id(&sd, pl_sd->block_list::id, skillnum, skilllv);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_version(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char * revision;
	char tmp[200];

 	if( (revision = get_svn_revision()) != 0 )
	{
		snprintf(tmp,sizeof(tmp), "eAthena Version SVN r%s",revision);
		clif_displaymessage(fd,tmp);
	}
	else
          clif_displaymessage(fd,"Cannot determine SVN revision");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_where(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const char *player_name = param[1];
	char output[128]="";

	if( strncmp(sd.status.name, player_name, 24)==0 )
		player_name = "";

	if(player_name && *player_name)
	{
		map_session_data *pl_sd = CommandInfo::param2sd(player_name);

		if( !config.hide_GM_session || sd.isGM()<pl_sd->isGM() )
			snprintf(output, sizeof(output), "%s %s %d %d", player_name, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
		else
			snprintf(output, sizeof(output), "%s not found", player_name);
	}
	else
	{	// self position
		snprintf(output, sizeof(output), "%s %d %d", sd.mapname, sd.block_list::x, sd.block_list::y);
	}
	clif_displaymessage(fd, output);
	return true;
}



///////////////////////////////////////////////////////////////////////////////
///
///
bool command_who(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char output[128]="";
	char match_text[128];
	char player_name[32]="";

	strcpytolower(match_text, sizeof(match_text), param[0]);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i)
	{
		if( session[i] && (pl_sd = (struct map_session_data *)session[i]->user_session) && pl_sd->state.auth)
		{
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name, sizeof(player_name), pl_sd->status.name);
				if(strstr(player_name, match_text) != NULL)
				{	// search with no case sensitive
					if (config.who_display_aid > 0 && sd.isGM() >= config.who_display_aid) {
						if (pl_GM_level > 0)
							snprintf(output, sizeof(output), "(CID:%ld/AID:%ld) Name: %s (GM:%d) | Location: %s %d %d", (unsigned long)pl_sd->status.char_id, (unsigned long)pl_sd->status.account_id, pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
						else
							snprintf(output, sizeof(output), "(CID:%ld/AID:%ld) Name: %s | Location: %s %d %d", (unsigned long)pl_sd->status.char_id, (unsigned long)pl_sd->status.account_id, pl_sd->status.name, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
					}
					else {
						if (pl_GM_level > 0)
							snprintf(output, sizeof(output), "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
						else
							snprintf(output, sizeof(output), "Name: %s | Location: %s %d %d", pl_sd->status.name, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
					}
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_PLAYER_FOUND)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_ONE_PLAYER_FOUND)); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_FOUND), count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_who2(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char match_text[128]="";
	char output[128];
	char player_name[32];

	strcpytolower(match_text, sizeof(match_text), param[0]);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();

			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name, sizeof(player_name), pl_sd->status.name);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					if (pl_GM_level > 0)
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					else
						snprintf(output, sizeof(output), "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}
	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_PLAYER_FOUND)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_ONE_PLAYER_FOUND)); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_FOUND), count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_who3(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char temp0[128];
	char temp1[128];
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char match_text[128]="";
	char output[128];
	char player_name[24];
	struct guild *g;
	struct party *p;

	strcpytolower(match_text, sizeof(match_text), param.line());

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name, sizeof(player_name), pl_sd->status.name);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					g = guild_search(pl_sd->status.guild_id);
					if (g == NULL)
						snprintf(temp1, sizeof(temp1), "None");
					else
						snprintf(temp1, sizeof(temp1), "%s", g->name);
					p = party_search(pl_sd->status.party_id);
					if (p == NULL)
						snprintf(temp0, sizeof(temp0), "None");
					else
						snprintf(temp0, sizeof(temp0), "%s", p->name);
					if (pl_GM_level > 0)
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_GM_level, temp0, temp1);
					else
						snprintf(output, sizeof(output), "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, temp0, temp1);
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_PLAYER_FOUND)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_ONE_PLAYER_FOUND)); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_FOUND), count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_whomap(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id=-1;
	char output[128];

	if( param.size()<1 ||  ((map_id = map_mapname2mapid(param[0])) < 0) )
		map_id = sd.block_list::m;
	
	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->block_list::m == map_id) {
					if (pl_GM_level > 0)
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
					else
						snprintf(output, sizeof(output), "Name: %s | Location: %s %d %d", pl_sd->status.name, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		snprintf(output, sizeof(output), msg_txt(MSG_NO_PLAYER_IN_MAP_S), maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_txt(MSG_ONE_PLAYER_IN_MAP_S), maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_IN_MAP_S), count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_whomap2(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char output[128];

	if( param.size()<1 ||  ((map_id = map_mapname2mapid(param[0])) < 0) )
		map_id = sd.block_list::m;

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->block_list::m == map_id) {
					if (pl_GM_level > 0)
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					else
						snprintf(output, sizeof(output), "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		snprintf(output, sizeof(output), msg_txt(MSG_NO_PLAYER_IN_MAP_S), maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_txt(MSG_ONE_PLAYER_IN_MAP_S), maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_IN_MAP_S), count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_whomap3(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char temp0[128];
	char temp1[128];
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id=0;
	char output[128];
	struct guild *g;
	struct party *p;

	if( param.size()<1 ||  ((map_id = map_mapname2mapid(param[0])) < 0) )
		map_id = sd.block_list::m;

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->block_list::m == map_id) {
					g = guild_search(pl_sd->status.guild_id);
					if (g == NULL)
						snprintf(temp1, sizeof(temp1), "None");
					else
						snprintf(temp1, sizeof(temp1), "%s", g->name);
					p = party_search(pl_sd->status.party_id);
					if (p == NULL)
						snprintf(temp0, sizeof(temp0), "None");
					else
						snprintf(temp0, sizeof(temp0), "%s", p->name);
					if (pl_GM_level > 0)
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_GM_level, temp0, temp1);
					else
						snprintf(output, sizeof(output), "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, temp0, temp1);
					clif_displaymessage(fd, output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		snprintf(output, sizeof(output), msg_txt(MSG_NO_PLAYER_IN_MAP_S), maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_txt(MSG_ONE_PLAYER_IN_MAP_S), maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_IN_MAP_S), count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_whogm(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	char temp0[128];
	char temp1[128];
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char match_text[128]="";
	char output[128];
	char player_name[24];
	struct guild *g;
	struct party *p;

	strcpytolower(match_text, sizeof(match_text), param[0]);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (pl_GM_level > 0) {
				if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
					strcpytolower(player_name, sizeof(player_name), pl_sd->status.name);
					if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
						snprintf(output, sizeof(output), "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
						clif_displaymessage(fd, output);
						snprintf(output, sizeof(output), "       BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
						clif_displaymessage(fd, output);
						g = guild_search(pl_sd->status.guild_id);
						if (g == NULL)
							snprintf(temp1, sizeof(temp0), "None");
						else
							snprintf(temp1, sizeof(temp0), "%s", g->name);
						p = party_search(pl_sd->status.party_id);
						if (p == NULL)
							snprintf(temp0, sizeof(temp0), "None");
						else
							snprintf(temp0, sizeof(temp0), "%s", p->name);
						snprintf(output, sizeof(output), "       Party: '%s' | Guild: '%s'", temp0, temp1);
						clif_displaymessage(fd, output);
						count++;
					}
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_GM_FOUND)); // No GM found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_1_GM_FOUND)); // 1 GM found.
	else {
		snprintf(output, sizeof(output), msg_txt(MSG_D_GMS_FOUND), count); // %d GMs found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool command_whozeny(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	const uint MAX = 50; /// limit the list to 50 since only 50 get displayed anyway
	struct map_session_data *pl_sd;
	size_t i, count;
	char player_name[24];
	char output[128];
	bool all = param.size()==0 || param[0]=="all";
	
	struct zenysort
	{
		char	name[24];
		uint32	zeny;
	} zeny[MAX+1]; //+1 for clean insertion sort
	
	for(count=0, i=0; i<fd_max; ++i)
	{
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
		{
			if( all || strcpytolower(player_name, sizeof(player_name), pl_sd->status.name), NULL!=strstr(player_name, param[0]) )
			{
				zenysort *epp = zeny+(count>=MAX?MAX:count)-1;
				// insertion sort
				for( ; epp>=zeny && epp->zeny<pl_sd->status.zeny; epp[1]=epp[0],--epp );
				// prevent unnecessary writes
				if( epp<zeny+MAX-1 )
				{	
					epp[1].zeny = pl_sd->status.zeny;
					safestrcpy(epp[1].name, sizeof(epp[1].name), pl_sd->status.name);
				}
				++count;
			}
		}
	}
	for(i=0; i<count && i<MAX; ++i)
	{
		snprintf(output, sizeof(output), "Name: %s | Zeny: %ld", zeny[i].name, (unsigned long)zeny[i].zeny);
		clif_displaymessage(fd, output);
	}
	if (count == 0)
		clif_displaymessage(fd, msg_txt(MSG_NO_PLAYER_FOUND)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(MSG_ONE_PLAYER_FOUND)); // 1 player found.
	else
	{
		snprintf(output, sizeof(output), msg_txt(MSG_D_PLAYERS_FOUND), count); // %d players found.
		clif_displaymessage(fd, output);
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// zeny
///
bool command_zeny(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	long zeny=0;
	uint32 new_zeny;
	if( param.size()<1 || (zeny = param[0]) == 0 )
	{
		clif_displaymessage(fd, "Please, enter an amount (usage: zeny <amount>).");
	}
	else
	{
		new_zeny = sd.status.zeny + zeny;
		if( zeny>0 && (new_zeny<sd.status.zeny || new_zeny>MAX_ZENY) ) // pos overflow & max
			new_zeny = MAX_ZENY;
		else if( zeny<0 && new_zeny>sd.status.zeny ) // neg overflow
			new_zeny = 0;

		if (new_zeny != sd.status.zeny)
		{
			sd.status.zeny = new_zeny;
			clif_updatestatus(sd, SP_ZENY);
			clif_displaymessage(fd, msg_txt(MSG_ZENYS_CHANGED)); // Number of zenys changed!
			return true;
		}
		else
		{
			// Impossible to decrease the number/value.
			// Impossible to increase the number/value.
			clif_displaymessage(fd, msg_txt( (zeny<0)?MSG_IMPOSSIBLE_TO_DECREASE:MSG_IMPOSSIBLE_TO_INCREASE));
		}
	}
	return false;
}



#ifdef DMALLOC
staic unsigned long dmark_;
bool command_dmstart(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	dmark_ = dmalloc_mark();
	clif_displaymessage(fd, "debug mark set");
	return true;
}

bool command_dmtick(int fd, struct map_session_data& sd, const char* command, const basics::CParameterList& param)
{
	dmalloc_log_changed ( dmark_, 1, 0, 1 ) ;
	dmark_ = dmalloc_mark();
	clif_displaymessage(fd, "malloc changes logged");
	return true;
}
#endif






///////////////////////////////////////////////////////////////////////////////
//
// command control structures
//
///////////////////////////////////////////////////////////////////////////////



char CommandInfo::command_symbol = '@'; // first char of the commands



///////////////////////////////////////////////////////////////////////////////
/// CommandInfo.
/// First char of commands is configured in command_athena.conf. 
/// to set external level, load command_athena.conf.
static CommandInfo command_info[] =
{
	//<name>,<lvl>,<# param>,<optional sd>,<function>
	{ "addwarp",			20, 3, 0, command_addwarp				},
	{ "adjcmdlvl",			99, 2, 0, command_adjcmdlvl				},
	{ "adjgmlvl",			99, 2, 0, command_adjgmlvl				},
	{ "adopt",				40, 3, 0, command_adopt					},
	{ "agitend",			60, 0, 0, command_agitend				},
	{ "agitstart",			60, 0, 0, command_agitstart				},
	{ "alive",				60, 0, 1, command_alive					},
	{ "allskill",			60, 0, 1, command_allskill				},
	{ "allskills",			60, 0, 1, command_allskill				},
	{ "skillall",			60, 0, 1, command_allskill				},
	{ "skillsall",			60, 0, 1, command_allskill				},
	{ "autoloot",			10, 1, 1, command_autoloot				},
	{ "lvup",				60, 1, 1, command_baselevelup			},
	{ "blevel",				60, 1, 1, command_baselevelup			},
	{ "baselvlup",			60, 1, 1, command_baselevelup			},
	{ "broadcast",			40, 0, 0, command_broadcast				},
	{ "cartlist",			40, 0, 1, command_cartlist				},
	{ "changelook",			99, 1, 1, command_changelook			},
	{ "changesex",			 1, 1, 0, command_changesex				},
	{ "ban",				60, 1, 0, command_char_ban				},
	{ "banish",				60, 1, 0, command_char_ban				},
	{ "charban",			60, 1, 0, command_char_ban				},
	{ "charbanish",			60, 1, 0, command_char_ban				},
	{ "block",				60, 1, 0, command_char_block			},
	{ "charblock",			60, 1, 0, command_char_block			},
	{ "reset",				40, 0, 0, command_charreset				},
	{ "unban",				60, 1, 0, command_char_unban			},
	{ "unbanish",			60, 1, 0, command_char_unban			},
	{ "charunban",			60, 1, 0, command_char_unban			},
	{ "charunbanish",		60, 1, 0, command_char_unban			},
	{ "unblock",			60, 1, 0, command_char_unblock			},
	{ "charunblock",		60, 1, 0, command_char_unblock			},
	{ "checkmail",			 1, 0, 0, command_checkmail				},
	{ "cleanmap",			 0, 0, 0, command_cleanmap				},
	{ "clearweather",		99, 1, 0, command_clearweather			},
	{ "clouds",				99, 1, 0, command_clouds				},
	{ "clouds2",			99, 1, 0, command_clouds2				},
	{ "day",				80, 0, 0, command_day					},
	{ "deletemail",			 1, 1, 0, command_deletemail			},
	{ "delitem",			60, 1, 1, command_delitem				},
	{ "die",				 1, 0, 1, command_die					},
	{ "disguise",			20, 1, 1, command_disguise				},
	{ "disguiseall",		99, 1, 0, command_disguiseall			},
	{ "divorce",			40, 1, 0, command_divorce				},
	{ "doom",				80, 0, 0, command_doom					},
	{ "doommap",			80, 0, 0, command_doommap				},
	{ "dropall",			40, 0, 1, command_dropall				},
	{ "dye",				40, 1, 1, command_dye					},
	{ "ccolor",				40, 1, 1, command_dye					},
	{ "effect",				40, 1, 0, command_effect				},
	{ "email",				 0, 1, 0, command_email					},
	{ "fakename",			20, 1, 1, command_fakename				},
	{ "fireworks",			99, 1, 0, command_fireworks				},
	{ "fog",				99, 1, 0, command_fog					},
	{ "follow",				10, 1, 1, command_follow				},
	{ "gat",				99, 0, 1, command_gat					},
	{ "gm",					100, 0, 0, command_gm					},
	{ "gmotd",				 0, 0, 0, command_gmotd					},
	{ "go",					10, 1, 0, command_go					},
	{ "grind",				99, 1, 0, command_grind					},
	{ "grind2",				99, 0, 0, command_grind2				},
	{ "guild",				50, 1, 1, command_guild					},
	{ "guildlvup",			60, 1, 0, command_guildlevelup			},
	{ "guildlvlup",			60, 1, 0, command_guildlevelup			},
	{ "guildrecall",		60, 1, 0, command_guildrecall			},
	{ "guildspy",			60, 1, 0, command_guildspy				},
	{ "gstorage",			50, 0, 0, command_guildstorage			},
	{ "gvgoff",				40, 0, 0, command_gvgoff				},
	{ "gpvpoff",			40, 0, 0, command_gvgoff				},
	{ "gvgon",				40, 0, 0, command_gvgon					},
	{ "gpvpon",				40, 0, 0, command_gvgon					},
	{ "happyhappyjoyjoy",	40, 0, 0, command_happyhappyjoyjoy		},
	{ "hatch",				60, 0, 1, command_hatch					},
	{ "haircolor",			40, 1, 1, command_hair_color			},
	{ "hcolor",				40, 1, 1, command_hair_color			},
	{ "heal",				40, 1, 1, command_heal					},
	{ "h",					20, 0, 0, command_help					},
	{ "help",				20, 0, 0, command_help					},
	{ "hide",				40, 0, 1, command_hide					},
	{ "hidenpc",			80, 1, 0, command_hidenpc				},
	{ "hairstyle", 			40, 1, 1, command_hair_style			},
	{ "hstyle",				40, 1, 1, command_hair_style			},
	{ "identify",			40, 0, 1, command_identify				},
	{ "idsearch",			60, 0, 0, command_idsearch				},
	{ "item",				60, 1, 1, command_item					},
	{ "itemcheck",			60, 0, 1, command_itemcheck				},
	{ "iteminfo",			 1, 1, 0, command_iteminfo				},
	{ "ii",					 1, 1, 0, command_iteminfo				},
	{ "itemlist",			40, 0, 1, command_itemlist				},
	{ "itemreset",			40, 0, 1, command_itemreset				},
	{ "jail",				60, 1, 0, command_jail					},
	{ "jobchange",			40, 1, 1, command_jobchange				},
	{ "job",				40, 1, 1, command_jobchange				},
	{ "jlevel",				60, 1, 1, command_joblevelup			},
	{ "joblvup",			60, 1, 1, command_joblevelup			},
	{ "joblvlup",			60, 1, 1, command_joblevelup			},
	{ "jump",				40, 2, 0, command_jump					},
	{ "jumpto",				20, 1, 0, command_jumpto				},
	{ "warpto",				20, 1, 0, command_jumpto				},
	{ "goto",				20, 1, 0, command_jumpto				},
	{ "kami",				40, 0, 0, command_kami					},
	{ "kamib",				40, 0, 0, command_kami					},
	{ "kick",				20, 1, 0, command_kick					},
	{ "kickall",			99, 0, 0, command_kickall				},
	{ "kill",				60, 1, 0, command_kill					},
	{ "killable",			40, 1, 1, command_killable				},
	{ "killer",				60, 1, 1, command_killer				},
	{ "killmonster",		60, 1, 0, command_killmonster			},
	{ "leaves",				99, 1, 0, command_leaves				},
	{ "listmail",			 1, 1, 0, command_listmail				},
	{ "listnewmail",		 1, 1, 0, command_listnewmail			},
	{ "return",				40, 0, 0, command_load					},
	{ "load",				40, 0, 0, command_load					},
	{ "loadnpc",			80, 1, 0, command_loadnpc				},
	{ "localbroadcast",		40, 0, 0, command_localbroadcast		},
	{ "lostskill",			40, 1, 1, command_lostskill				},
	{ "makeegg",			60, 1, 1, command_makeegg				},
	{ "mapexit",			99, 0, 0, command_mapexit				},
	{ "mapflag",			99, 3, 0, command_mapflag				},
	{ "mapinfo",			99, 2, 0, command_mapinfo				},
	{ "mapmove",			40, 3, 1, command_mapmove				},
	{ "rura",				40, 3, 1, command_mapmove				},
	{ "warp",				40, 3, 1, command_mapmove				},
	{ "marry",				40, 1, 0, command_marry					},
	{ "me",					20, 0, 0, command_me					},
	{ "memo",				40, 1, 0, command_memo					},
	{ "misceffect",			50, 1, 1, command_misceffect			},
	{ "mobinfo",			 1, 1, 0, command_mobinfo				},
	{ "monsterinfo",		 1, 1, 0, command_mobinfo				},
	{ "mi",					 1, 1, 0, command_mobinfo				},
	{ "mobsearch",			 0, 1, 0, command_mobsearch				},
	{ "model",				20, 3, 1, command_model					},
	{ "monster2",			50, 5, 0, command_monster				},
	{ "monsterbig",			50, 5, 0, command_monsterbig			},
	{ "monsterignore",		99, 1, 1, command_monsterignore			},
	{ "monstersmall",		50, 5, 0, command_monstersmall			},
	{ "mountpeco",			20, 0, 1, command_mount_peco			},
	{ "mute",				99, 1, 1, command_mute					},
	{ "red",				99, 1, 1, command_mute					},
	{ "mutearea",			99, 1, 0, command_mutearea				},
	{ "stfu",				99, 1, 0, command_mutearea				},
	{ "night",				80, 0, 0, command_night					},
	{ "npcmove",			20, 1, 0, command_npcmove				},
	{ "npctalk",			 0, 2, 0, command_npctalk				},
	{ "nuke",				60, 1, 0, command_nuke					},
	{ "option",				40, 3, 1, command_option				},
	{ "packet",				99, 2, 0, command_packet				},
	{ "packetmode",			99, 1, 0, command_packet				},
	{ "party",				 1, 1, 1, command_party					},
	{ "partyrecall",		60, 1, 0, command_partyrecall			},
	{ "partyspy",			60, 1, 0, command_partyspy				},
	{ "petfriendly",		40, 1, 1, command_petfriendly			},
	{ "pethungry",			40, 1, 1, command_pethungry				},
	{ "petid",				40, 1, 0, command_petid					},
	{ "petrename",			40, 1, 1, command_petrename				},
	{ "pettalk",			 0, 0, 0, command_pettalk				},
	{ "printstats",			40, 1, 0, command_printstats			},
	{ "stats",				40, 1, 0, command_printstats			},
	{ "produce",			60, 3, 1, command_produce				},
	{ "pvpoff",				40, 0, 0, command_pvpoff				},
	{ "pvpon",				40, 0, 0, command_pvpon					},
	{ "questskill",			40, 1, 1, command_questskill			},
	{ "rain",				99, 1, 0, command_rain					},
	{ "raise",				80, 0, 0, command_raise					},
	{ "raisemap",			80, 0, 0, command_raise					},
	{ "rates",				10, 0, 0, command_rates					},
	{ "readmail",			 1, 1, 0, command_readmail				},
	{ "recall",				60, 1, 0, command_recall				},
	{ "recallall",			80, 0, 0, command_recallall				},
	{ "refine",				60, 2, 1, command_refine				},
	{ "refresh",			 0, 0, 0, command_refresh				},
	{ "refreshonline",		99, 0, 0, command_refreshonline			},
	{ "reloadatcommand",	99, 0, 0, command_reloadatcommand		},
	{ "reloadbattleconf",	99, 0, 0, command_reloadbattleconf		},
	{ "reloaditemdb",		99, 0, 0, command_reloaditemdb			},
	{ "reloadmobdb",		99, 0, 0, command_reloadmobdb			},
	{ "reloadpcdb",			99, 0, 0, command_reloadpcdb			},
	{ "reloadscript",		99, 0, 0, command_reloadscript			},
	{ "reloadskilldb",		99, 0, 0, command_reloadskilldb			},
	{ "reloadstatusdb",		99, 0, 0, command_reloadstatusdb		},
	{ "repairall",			60, 0, 1, command_repairall				},
	{ "revive",				60, 1, 0, command_revive				},
	{ "sakura",				99, 1, 0, command_sakura				},
	{ "save",				40, 3, 1, command_save					},
	{ "send",				60,20, 0, command_send					},
	{ "sendmail",			 1, 2, 0, command_sendmail				},
	{ "time",				 0, 0, 0, command_servertime			},
	{ "date",				 0, 0, 0, command_servertime			},
	{ "server_date",		 0, 0, 0, command_servertime			},
	{ "serverdate",			 0, 0, 0, command_servertime			},
	{ "server_time",		 0, 0, 0, command_servertime			},
	{ "servertime",			 0, 0, 0, command_servertime			},
	{ "setbattleflag",		60, 2, 0, command_setbattleflag			},
	{ "battleoption",		60, 2, 0, command_setbattleflag			},
	{ "showexp",			20, 1, 1, command_showexp				},
	{ "showdelay",			20, 1, 1, command_showdelay				},
	{ "shownpc",			80, 1, 0, command_shownpc				},
	{ "shuffle",			40, 1, 0, command_shuffle				},
	{ "size",				20, 1, 1, command_size					},
	{ "skillid",			40, 1, 0, command_skillid				},
	{ "skilloff",			20, 0, 0, command_skilloff				},
	{ "skillon",			20, 0, 0, command_skillon				},
	{ "skpoint",			60, 1, 1, command_skillpoint			},
	{ "skreset",			60, 0, 1, command_skillreset			},
	{ "skilltree",			40, 2, 0, command_skilltree				},
	{ "snow",				99, 1, 0, command_snow					},
	{ "sound",				40, 1, 0, command_sound					},
	{ "monster",			50, 5, 0, command_spawn					},
	{ "spawn",				50, 5, 0, command_spawn					},
	{ "speed",				40, 1, 1, command_speed					},
	{ "spiritball",			40, 1, 1, command_spiritball			},
	{ "statall",			60, 0, 1, command_stat_all				},
	{ "statsall",			60, 0, 1, command_stat_all				},
	{ "allstats",			60, 0, 1, command_stat_all				},
	{ "allstat",			60, 0, 1, command_stat_all				},
	{ "agi",				60, 1, 1, command_status				},
	{ "dex",				60, 1, 1, command_status				},
	{ "int",				60, 1, 1, command_status				},
	{ "luk",				60, 1, 1, command_status				},
	{ "str",				60, 1, 1, command_status				},
	{ "vit",				60, 1, 1, command_status				},
	{ "stpoint",			60, 1, 1, command_statuspoint			},
	{ "streset",			60, 0, 1, command_statusreset			},
	{ "storage",			 1, 0, 1, command_storage				},
	{ "storagelist",		40, 0, 1, command_storagelist			},
	{ "storeall",			40, 0, 1, command_storeall				},
	{ "summon",				60, 2, 1, command_summon				},
	{ "trade",				60, 1, 1, command_trade					},
	{ "undisguise",			20, 0, 1, command_undisguise			},
	{ "undisguiseall",		99, 0, 0, command_undisguiseall			},
	{ "unjail",				60, 1, 0, command_unjail				},
	{ "discharge",			60, 1, 0, command_unjail				},
	{ "unloadnpc",			80, 1, 0, command_unloadnpc				},
	{ "unmute",				60, 0, 1, command_unmute				},
	{ "uptime",				 0, 0, 0, command_uptime				},
	{ "users",				 0, 0, 0, command_users					},
	{ "useskill",			40, 3, 0, command_useskill				},
	{ "version",			 0, 0, 0, command_version				},
	{ "where",				 1, 1, 0, command_where					},
	{ "who",				20, 1, 0, command_who					},
	{ "whois",				20, 1, 0, command_who					},
	{ "who2",				20, 1, 0, command_who2					},
	{ "who3",				20, 1, 0, command_who3					},
	{ "whogm",				20, 1, 0, command_whogm					},
	{ "whomap",				20, 1, 0, command_whomap				},
	{ "whomap2",			20, 1, 0, command_whomap2				},
	{ "whomap3",			20, 1, 0, command_whomap3				},
	{ "whozeny",			20, 1, 0, command_whozeny				},
	{ "zeny",				60, 1, 1, command_zeny					},

#ifdef DMALLOC
	{ "dmstart",			99, 0, 0, command_dmstart				},
	{ "dmtick",				99, 0, 0, command_dmtick				},
#endif
// add new commands before this line
	{ NULL,					100,0, 0, NULL							}
};






///////////////////////////////////////////////////////////////////////////////
/// returns command requirement level.
unsigned char CommandInfo::get_level(command_function func)
{
	size_t i;
	for(i=0; command_info[i].func && i<sizeof(command_info)/sizeof(*command_info); ++i)
		if(command_info[i].func == func)
			return command_info[i].level;
	return 100; // 100: command can not be used
}
///////////////////////////////////////////////////////////////////////////////
/// returns command info by command name.
CommandInfo& CommandInfo::byname(const char* name)
{
	size_t i;
	for(i=0; command_info[i].func && i<sizeof(command_info)/sizeof(*command_info); ++i)
		if( strcasecmp(command_info[i].command, name) == 0 )
			break;
	return command_info[i];
}

///////////////////////////////////////////////////////////////////////////////
/// read configuration file.
bool CommandInfo::config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	
	FILE* fp;

	if((fp = basics::safefopen(cfgName, "r")) == NULL)
	{
		ShowError("At commands configuration file not found: %s\n", cfgName);
	}
	else
	{
		while (fgets(line, sizeof(line), fp))
		{
			if( prepare_line(line) && 2==sscanf(line, "%1023[^:=]%*[:=]%1023[^\r\n]", w1, w2) )
			{
				basics::itrim(w1);
				if(!*w1) continue;
				basics::itrim(w2);
				
				if(strcasecmp(w1, "import") == 0)
				{
					CommandInfo::config_read(w2);
				}
				else if(strcasecmp(w1, "command_symbol") == 0 && w2[0] > 31 &&
						w2[0] != '/' &&	// symbol of standard ragnarok GM commands
						w2[0] != '%' &&	// symbol of party chat speaking
						w2[0] != '$' )	// symbol of guild chat
				{
					CommandInfo::command_symbol = w2[0];
				}
				else
				{
					CommandInfo& cmd = CommandInfo::byname(w1);
					if(cmd.func)
					{
						const int i= strtol(w2,NULL,0);
						cmd.level = (i>=0 && i<100) ?i:100;
					}
					else
					{
						ShowWarning("command \"%s\" does not exist\n", w1);
					}
				}
			}
		}
		fclose(fp);
		ShowStatus("Command configuration file '%s' read.\n", cfgName);
		return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////
/// converts name or id to sd
map_session_data *CommandInfo::param2sd(const char* str)
{
	struct map_session_data *psd=NULL;
	if(str)
	{	// try id's when string starts with digits
		if( basics::stringcheck::isdigit(*str) )
		{	
			uint32 id = atoi(str);
			(psd = map_session_data::from_blid(id)) || (psd = map_session_data::charid2sd(id));
		}
		// do explicit stringsearch when no id or failed on exact match 
		if( !psd )
			psd = map_session_data::nick2sd(str);
	}
	return psd; 
}

///////////////////////////////////////////////////////////////////////////////
/// checks and executes a command.
bool CommandInfo::is_command(const int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl_override)
{
	if( !config.allow_atcommand_when_mute && sd.sc_data[SC_NOCHAT].timer != -1 )
	{	// return as processed
		return true;
	}
	else if( message && *message && (!config.atc_gmonly || gmlvl_override) )
	{	// format of the input string is "<name> : <command string>"
		char output[512];

		// compare the name
		const char *spp = message;
		const char *npp = sd.status.name;
		while( *npp && *spp && *npp==*spp ) ++npp, ++spp;
		if(*npp) // something wrong
		{
			snprintf(output,sizeof(output), "Hack on messages: character '%s' (account: %ld) uses another name.", sd.status.name, (unsigned long)sd.status.account_id);
			intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);
			ShowWarning(output);

			snprintf(output,sizeof(output), " Player sends: '%s'.", message);
			ShowWarning(output);
			intif_wis_message_to_gm(wisp_server_name, config.hack_info_GM_level, output);

			clif_ban_player(sd, config.ban_spoof_namer);

			// return as processed
			return true;
		}
		
		// skip until reached the colon
		while( *spp && *spp!=':' ) ++spp;
		// skip the colon
		if(*spp) ++spp;
		// skip the spaces until normal text starts
		while( *spp && basics::stringcheck::isspace(*spp) ) ++spp;

		if(*spp && *spp == CommandInfo::command_symbol)
		{	// command string starts with command symbol
			char command[128], *ipp=command;

			// skip the command symbol
			++spp;

			// copy out the command
			while( *spp && !basics::stringcheck::isspace(*spp) ) 
				*ipp = *spp++, (ipp>=(command+sizeof(command)-1)||++ipp);
			*ipp=0;

			// look up the command
			CommandInfo& cmd = CommandInfo::byname(command);

			log_atcommand(sd, message, cmd.level);

			gmlvl_override = gmlvl_override?gmlvl_override:sd.isGM();
			if( cmd.func == NULL  || gmlvl_override<cmd.level )
			{	// return false if player is normal player 
				// (display the text, not display: unknown command)
				if( gmlvl_override == 0 )
					return false;

				snprintf(output, sizeof(output), msg_txt(MSG_S_IS_UNKNOWN_COMMAND), command); // %s is Unknown Command.
				clif_displaymessage(fd, output);
			}
			else
			{	// build the parameter list
				basics::CParameterList param(spp);

				// default map_session to work on
				map_session_data *psd = &sd;
				
				if( cmd.option && param.size() )
				{	// for commands with optional input
					// check if the last parameter is a char identifier
					// and take it when valid 
					// or when there are more parameters than necessary for the current command
					map_session_data *tmp = CommandInfo::param2sd( param.last() );
					if(tmp || (param.size()>cmd.param) )
						psd = tmp;
				}

				if( !psd )
				{
					clif_displaymessage(fd, msg_txt(MSG_CHAR_NOT_FOUND)); // Character not found.
				}
				else if( psd != &sd && sd.isGM() <= psd->isGM() )
				{	// Your GM level don't authorise you to do this action on this player.
					clif_displaymessage(fd, msg_txt(MSG_GM_LV_TOO_LOW_FOR_PCACTION)); 
				}
				else if( !cmd.func(fd, *psd, command, param) )
				{	// Command could not be executed
					snprintf(output, sizeof(output), msg_txt(MSG_S_FAILED), command); // %s failed.
					clif_displaymessage(fd, output);
				}
				else
				{	// everything fine
					clif_displaymessage(fd, "done.");
				}
			}
			return true;
		}
	}
	return false;
}

