
#include "socket.h"
#include "timer.h"
#include "nullpo.h"

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
#include "charcommand.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"
#include "core.h"
#include "showmsg.h"
#include "utils.h"


static char command_symbol = '#';

#define CCMD_FUNC(x) bool charcommand_ ## x (int fd, struct map_session_data &sd,const char *command, const char *message)

CCMD_FUNC(jobchange);
CCMD_FUNC(petrename);
CCMD_FUNC(petfriendly);
CCMD_FUNC(stats);
CCMD_FUNC(option);
CCMD_FUNC(reset);
CCMD_FUNC(save);
CCMD_FUNC(stats_all);
CCMD_FUNC(spiritball);
CCMD_FUNC(itemlist);
CCMD_FUNC(effect);
CCMD_FUNC(storagelist);
CCMD_FUNC(item);
CCMD_FUNC(warp);
CCMD_FUNC(zeny);
CCMD_FUNC(warp);
CCMD_FUNC(showexp);
CCMD_FUNC(showdelay);

CCMD_FUNC(fakename);
CCMD_FUNC(baselevel);
CCMD_FUNC(joblevel);
CCMD_FUNC(questskill);
CCMD_FUNC(lostskill);
CCMD_FUNC(skreset);
CCMD_FUNC(streset);
CCMD_FUNC(model);
CCMD_FUNC(stpoint);
CCMD_FUNC(skpoint);
CCMD_FUNC(changesex);


/*==========================================
 *CharCommandInfo charcommand_info[]構造体の定義
 *------------------------------------------
 */

// First char of commands is configured in charcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read charcommand_athena.conf first please.
static CharCommandInfo charcommand_info[] = {
	{ CharCommandJobChange,				"#job",						60,	charcommand_jobchange },
	{ CharCommandJobChange,				"#jobchange",				60,	charcommand_jobchange },
	{ CharCommandPetRename,				"#petrename",				50, charcommand_petrename },
	{ CharCommandPetFriendly,			"#petfriendly",				50, charcommand_petfriendly },
	{ CharCommandStats,					"#stats",					40, charcommand_stats },
	{ CharCommandOption,				"#option",					60, charcommand_option },
	{ CharCommandReset,					"#reset",					60, charcommand_reset },
	{ CharCommandSave,					"#save",					60, charcommand_save },
	{ CharCommandStatsAll,				"#statsall",				40, charcommand_stats_all },
	{ CharCommandSpiritball,			"#spiritball",				40, charcommand_spiritball },
	{ CharCommandItemList,				"#itemlist",				40,	charcommand_itemlist },
	{ CharCommandEffect,				"#effect",					40, charcommand_effect },
	{ CharCommandStorageList,			"#storagelist",				40, charcommand_storagelist },
	{ CharCommandItem,					"#item",					60, charcommand_item },
	{ CharCommandWarp,					"#warp",					60, charcommand_warp },
	{ CharCommandWarp,					"#rura",					60, charcommand_warp },
	{ CharCommandWarp,					"#rura+",					60, charcommand_warp },
	{ CharCommandZeny,					"#zeny",					60, charcommand_zeny },
	{ CharCommandShowExp,				"#showexp",					 0, charcommand_showexp},
	{ CharCommandShowDelay,				"#showdelay",				 0, charcommand_showdelay},

	{ CharCommandFakeName,				"#fakename",				20, charcommand_fakename},
	{ CharCommandBaseLevel,				"#baselvl",					20, charcommand_baselevel},
	{ CharCommandBaseLevel,				"#blvl",					60, charcommand_baselevel},
	{ CharCommandBaseLevel,				"#baselvlup",				60, charcommand_baselevel},
	{ CharCommandJobLevel,				"#joblvl",					60, charcommand_joblevel},
	{ CharCommandJobLevel,				"#jlvl",					60, charcommand_joblevel},
	{ CharCommandJobLevel,				"#joblvlup",				60, charcommand_joblevel},
	{ CharCommandQuestSkill,			"#questskill",				60, charcommand_questskill },
	{ CharCommandLostSkill,				"#lostskill",				60, charcommand_lostskill },
	{ CharCommandSkReset,				"#skreset",					60, charcommand_skreset },
	{ CharCommandStReset,				"#streset",					60, charcommand_streset },
	{ CharCommandModel,					"#charmodel",				50, charcommand_model },
	{ CharCommandSKPoint,				"#skpoint",					60, charcommand_skpoint },
	{ CharCommandSTPoint,				"#stpoint",					60, charcommand_stpoint },
	{ CharCommandChangeSex,				"#changesex",				60, charcommand_changesex },


// add new commands before this line
	{ CharCommand_Unknown,             NULL,                1, NULL }
};

char chcmd_output[200];

unsigned char get_charcommand_level(const CharCommandType type)
{
	size_t i;
	for (i = 0; charcommand_info[i].type < CharCommand_Unknown; ++i)
		if (charcommand_info[i].type == type)
			return charcommand_info[i].level;
	return 100; // 100: command can not be used
}
/*==========================================
 *
 *------------------------------------------
 */
CharCommandInfo* get_charcommandinfo_byname(const char* name)
{
	size_t i;
	for (i = 0; charcommand_info[i].type != CharCommand_Unknown; ++i)
		if (strcasecmp(charcommand_info[i].command, name) == 0)
			return &charcommand_info[i];
	return NULL;
}

/*==========================================
 *
 *------------------------------------------
 */
bool charcommand_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	CharCommandInfo* p;
	FILE* fp;

	if ((fp = basics::safefopen(cfgName, "r")) == NULL) {
		ShowError("CharCommands configuration file not found: %s\n", cfgName);
		return false;
	}

	while (fgets(line, sizeof(line), fp))
	{
		if( !prepare_line(line) )
			continue;

		if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
			continue;

		if (strcasecmp(w1, "import") == 0)
			charcommand_config_read(w2);
		else if (strcasecmp(w1, "command_symbol") == 0 && w2[0] > 31 &&
				w2[0] != '/' && // symbol of standard ragnarok GM commands
				w2[0] != '%' && // symbol of party chat speaking
				w2[0] != '$' && // symbol of guild chat speaking
				w2[0] != '@')	// symbol of atcommand
			command_symbol = w2[0];
		else
		{
			p = get_charcommandinfo_byname(w1);
			if (p != NULL) {
				p->level = atoi(w2);
				if (p->level > 100)
					p->level = 100;
			}
		}
	}
	fclose(fp);
	return true;
}
/*==========================================
 *
 *------------------------------------------
 */
CharCommandType charcommand(struct CharCommandInfo &info, const char* message, unsigned int level)
{
	if (config.atc_gmonly != 0 && !level)
		return CharCommand_None;

	if (!message || !*message)
		return CharCommand_None;

	if( *message == command_symbol )
	{	// check first char.
		char command[101];

		sscanf(message, "%100s", command);
		command[sizeof(command)-1] = '\0';

		size_t i;
		for (i = 0; charcommand_info[i].type < CharCommand_Unknown; ++i)
			if( 0==strcasecmp(command+1, charcommand_info[i].command+1) )
				break;

		if(charcommand_info[i].type == CharCommand_Unknown || level < charcommand_info[i].level)
		{	// doesn't return Unknown if player is normal player (display the text, not display: unknown command)
			if(level == 0)
				return CharCommand_None;
			else
				return CharCommand_Unknown;
		}
		memcpy(&info, &charcommand_info[i], sizeof charcommand_info[i]);
		return info.type;
	}
	else
	{
		return CharCommand_None;
	}
	
}

/*==========================================
 *is_charcommand @コマンドに存在するかどうか確認する
 *------------------------------------------
 */
CharCommandType is_charcommand(int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl)
{
	const char* str = message;
	int s_flag = 0;
	CharCommandInfo info = {CharCommand_Unknown,NULL,0,NULL};
	CharCommandType type;

	if (!message || !*message)
		return CharCommand_None;

	str += strlen(sd.status.name);
	while (*str && (isspace((int)((unsigned char)*str)) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return CharCommand_None;

	type = charcommand(info, str, gmlvl > 0 ? gmlvl : sd.isGM());

	if (type != CharCommand_None)
	{
		char command[100]="";
		char output[200]="";
		const char* p = str;

		while (*p && !isspace((int)((unsigned char)*p)))
			p++;

		if( p >= str+sizeof(command) ) // too long
			return CharCommand_Unknown;

		memcpy(command, str, p - str);

		while (isspace((int)((unsigned char)*p)))
			p++;

		if (type == CharCommand_Unknown || info.proc == NULL)
		{
			snprintf(output, sizeof(output),msg_txt(153), command); // %s is Unknown Command.
			clif_displaymessage(fd, output);
		}
		else
		{
			if( !info.proc(fd, sd, command, p) )
			{
				// Command can not be executed
				snprintf(output, sizeof(output), msg_txt(154), command); // %s failed.
				clif_displaymessage(fd, output);
			}
		}
		return info.type;
	}

	return CharCommand_None;
}






/*==========================================
 * 対象キャラクターを転職させる upper指定で転生や養子も可能
 *------------------------------------------
 */
bool charcommand_jobchange(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char character[100]="";
	struct map_session_data* pl_sd;
	int job = 0, upper = -1;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
		return false;
	}

	if (sscanf(message, "%d %d %99[^\n]", &job, &upper, character) < 3) { //upper指定してある
		upper = -1;
		if (sscanf(message, "%d %99[^\n]", &job, character) < 2) { //upper指定してない上に何か足りない
			clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
			return false;
		}
	}

	if ((pl_sd = map_nick2sd(character)) != NULL)
	{
		size_t j;
		if (sd.isGM() >= pl_sd->isGM()) { // you can change job only to lower or same level
			if ((job >= 0 && job < MAX_PC_CLASS)) {

				// fix pecopeco display
				if ((job != 13 && job != 21 && job != 4014 && job != 4022 && job != 4030 && job != 4036 && job != 4037 && job != 4044 )) {
					if (pc_isriding(sd)) {
						if (pl_sd->status.class_ == 13)
							pl_sd->status.class_ = pl_sd->view_class = 7;
						if (pl_sd->status.class_ == 21)
							pl_sd->status.class_ = pl_sd->view_class = 14;
						if (pl_sd->status.class_ == 4014)
							pl_sd->status.class_ = pl_sd->view_class = 4008;
						if (pl_sd->status.class_ == 4022)
							pl_sd->status.class_ = pl_sd->view_class = 4015;
						if (pl_sd->status.class_ == 4036)
							pl_sd->status.class_ = pl_sd->view_class = 4030;
						if (pl_sd->status.class_ == 4044)
							pl_sd->status.class_ = pl_sd->view_class = 4037;
						pl_sd->status.option &= ~0x0020;
						clif_changeoption(*pl_sd);
						status_calc_pc(*pl_sd, 0);
					}
				} else {
					if (!pc_isriding(sd)) {
						if (job == 13)
							job = 7;
						if (job == 21)
							job = 14;
						if (job == 4014)
							job = 4008;
						if (job == 4022)
							job = 4015;
						if (job == 4036)
							job = 4030;
						if (job == 4044)
							job = 4037;
					}
				}
				for (j=0; j < MAX_INVENTORY; ++j) {
					if(pl_sd->status.inventory[j].nameid>0 && pl_sd->status.inventory[j].equip!=0)
						pc_unequipitem(*pl_sd, j, 3);
				}
				if (pc_jobchange(*pl_sd, job, upper) == 0)
					clif_displaymessage(fd, msg_txt(48)); // Character's job changed.
				else {
					clif_displaymessage(fd, msg_txt(192)); // Impossible to change the character's job.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_txt(49)); // Invalid job ID.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------
 */
bool charcommand_petrename(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char character[100];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #petrename <char name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pl_sd->status.pet_id > 0 && pl_sd->pd) {
			if (pl_sd->pd->pet.rename_flag != 0) {
				pl_sd->pd->pet.rename_flag = 0;
				intif_save_petdata(pl_sd->status.account_id, pl_sd->pd->pet);
				clif_send_petstatus(*pl_sd);
				clif_displaymessage(fd, msg_txt(189)); // This player can now rename his/her pet.
			} else {
				clif_displaymessage(fd, msg_txt(190)); // This player can already rename his/her pet.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_txt(191)); // Sorry, but this player has no pet.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}


/*==========================================
 * 
 *------------------------------------------
 */
bool charcommand_petfriendly(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	int friendly = 0;
	int t = 0;
	char character[100];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	if (!message || !*message || sscanf(message,"%d %s",&friendly,character) < 2) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: "
			"#petfriendly <0-1000> <player>).");
		return false;
	}

	if (((pl_sd = map_nick2sd(character)) != NULL) && sd.isGM()>pl_sd->isGM()) {
		if (pl_sd->status.pet_id > 0 && pl_sd->pd) {
			if (friendly >= 0 && friendly <= 1000) {
				if (friendly != pl_sd->pd->pet.intimate) {
					t = pl_sd->pd->pet.intimate;
					pl_sd->pd->pet.intimate = friendly;
					clif_send_petstatus(*pl_sd);
					clif_pet_emotion(*pl_sd->pd,0);
					if (config.pet_status_support) {
						if ((pl_sd->pd->pet.intimate > 0 && t <= 0) ||
						    (pl_sd->pd->pet.intimate <= 0 && t > 0)) {
							if (pl_sd->block_list::prev != NULL)
								status_calc_pc(*pl_sd, 0);
							else
								status_calc_pc(*pl_sd, 2);
						}
					}
					clif_displaymessage(pl_sd->fd, msg_txt(182)); // Pet friendly value changed!
					clif_displaymessage(sd.fd, msg_txt(182)); // Pet friendly value changed!
				} else {
					clif_displaymessage(fd, msg_txt(183)); // Pet friendly is already the good value.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
				return false;
			}
		} else {
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------
 */
bool charcommand_stats(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char character[128];
	char job_jobname[128];
	char output[256];
	struct map_session_data *pl_sd;
	int i;

	memset(character, '\0', sizeof(character));
	memset(job_jobname, '\0', sizeof(job_jobname));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #stats <char name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		struct {
			const char* format;
			int value;
		} output_table[] = {
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
		snprintf(output, sizeof(output), msg_txt(53), pl_sd->status.name); // '%s' stats:
		clif_displaymessage(fd, output);
		for (i = 0; output_table[i].format != NULL; ++i) {
			snprintf(output, sizeof(output), output_table[i].format, output_table[i].value);
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * Character Reset
 *------------------------------------------
 */
bool charcommand_reset(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char character[100];
	char output[200];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #reset <charname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can reset a character only for lower or same GM level
			pc_resetstate(*pl_sd);
			pc_resetskill(*pl_sd);
			snprintf(output, sizeof(output), msg_txt(208), character); // '%s' skill and stats points reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------
 */
bool charcommand_option(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char character[100];
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message ||
		sscanf(message, "%d %d %d %99[^\n]", &opt1, &opt2, &opt3, character) < 4 ||
		opt1 < 0 || opt2 < 0 || opt3 < 0) {
		clif_displaymessage(fd, "Please, enter valid options and a player name (usage: #option <param1> <param2> <param3> <charname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can change option only to lower or same level
			pl_sd->opt1 = opt1;
			pl_sd->opt2 = opt2;
			pl_sd->status.option = opt3;
			// fix pecopeco display
			if (pl_sd->status.class_ == 13 || pl_sd->status.class_ == 21 || pl_sd->status.class_ == 4014 || pl_sd->status.class_ == 4022 || pl_sd->status.class_ == 4030 || pl_sd->status.class_ == 4036 || pl_sd->status.class_ == 4037 || pl_sd->status.class_ == 4044) {
				if (!pc_isriding(*pl_sd)) { // pl_sd have the new value...
					if (pl_sd->status.class_ == 13)
						pl_sd->status.class_ = pl_sd->view_class = 7;
					else if (pl_sd->status.class_ == 21)
						pl_sd->status.class_ = pl_sd->view_class = 14;
					else if (pl_sd->status.class_ == 4014)
						pl_sd->status.class_ = pl_sd->view_class = 4008;
					else if (pl_sd->status.class_ == 4022)
						pl_sd->status.class_ = pl_sd->view_class = 4015;
					else if (pl_sd->status.class_ == 4036)
						pl_sd->status.class_ = pl_sd->view_class = 4030;
					else if (pl_sd->status.class_ == 4044)
						pl_sd->status.class_ = pl_sd->view_class = 4037;
				}
			} else {
				if (pc_isriding(*pl_sd)) { // pl_sd have the new value...
					if (pl_sd->status.class_ == 7)
						pl_sd->status.class_ = pl_sd->view_class = 13;
					else if (pl_sd->status.class_ == 14)
						pl_sd->status.class_ = pl_sd->view_class = 21;
					else if (pl_sd->status.class_ == 4008)
						pl_sd->status.class_ = pl_sd->view_class = 4014;
					else if (pl_sd->status.class_ == 4015)
						pl_sd->status.class_ = pl_sd->view_class = 4022;
					else if (pl_sd->status.class_ == 4030)
						pl_sd->status.class_ = pl_sd->view_class = 4036;
					else if (pl_sd->status.class_ == 4037)
						pl_sd->status.class_ = pl_sd->view_class = 4044;
					else
						pl_sd->status.option &= ~0x0020;
					}
				}
			clif_changeoption(*pl_sd);
			status_calc_pc(*pl_sd, 0);
			clif_displaymessage(fd, msg_txt(58)); // Character's options changed.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------
 */
bool charcommand_save(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char mapname[128], *ip;
	char character[128];
	struct map_session_data* pl_sd;
	int x = 0, y = 0;
	int m;

	memset(mapname, '\0', sizeof(mapname));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99s %d %d %99[^\n]", mapname, &x, &y, character) < 4 || x < 0 || y < 0) {
		clif_displaymessage(fd, "Please, enter a valid save point and a player name (usage: #save <map> <x> <y> <charname>).");
		return false;
	}
	ip = strchr(mapname, '.');
	if(ip) *ip=0;

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can change save point only to lower or same gm level
			m = map_mapname2mapid(mapname);
			if (m < 0) {
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return false;
			} else {
				if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
					clif_displaymessage(fd, "You are not authorised to set this map as a save map.");
					return false;
				}
				pc_setsavepoint(*pl_sd, mapname, x, y);
				clif_displaymessage(fd, msg_txt(57)); // Character's respawn point changed.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 *
 *------------------------------------------
 */
//** Character Stats All by fritz
bool charcommand_stats_all(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char output[1024], gmlevel[1024];
	size_t i, count;
	struct map_session_data *pl_sd;

	memset(output, '\0', sizeof(output));
	memset(gmlevel, '\0', sizeof(gmlevel));

	count = 0;
	for(i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {

			if (pl_sd->isGM() > 0)
				snprintf(gmlevel, sizeof(gmlevel), "| GM Lvl: %d", pl_sd->isGM());
			else
				snprintf(gmlevel, sizeof(gmlevel), " ");

			snprintf(output, sizeof(output), "Name: %s | BLvl: %d | Job: %s (Lvl: %d) | HP: %ld/%ld | SP: %ld/%ld", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level, (unsigned long)pl_sd->status.hp, (unsigned long)pl_sd->status.max_hp, (unsigned long)pl_sd->status.sp, (unsigned long)pl_sd->status.max_sp);
			clif_displaymessage(fd, output);
			snprintf(output, sizeof(output), "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %ld %s", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, (unsigned long)pl_sd->status.zeny, gmlevel);
			clif_displaymessage(fd, output);
			clif_displaymessage(fd, "--------");
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

/*==========================================
 * CharSpiritBall Function by PalasX
 *------------------------------------------
 */
bool charcommand_spiritball(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char character[100];
	int spirit = 0;

	memset(character, '\0', sizeof(character));

	if(!message || !*message || sscanf(message, "%d %99[^\n]", &spirit, character) < 2 || spirit < 0 || spirit > 1000) {
		clif_displaymessage(fd, "Usage: @spiritball <number: 0-1000>) <CHARACTER_NAME>.");
		return false;
	}

	if((pl_sd = map_nick2sd(character)) != NULL) {
		if (spirit >= 0 && spirit <= 0x7FFF) {
			if (pl_sd->spiritball != spirit || spirit > 999) {
				if (pl_sd->spiritball > 0)
					pc_delspiritball(*pl_sd, pl_sd->spiritball, 1);
				pl_sd->spiritball = spirit;
				clif_spiritball(*pl_sd);
				// no message, player can look the difference
				if (spirit > 1000)
					clif_displaymessage(fd, msg_txt(204)); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
			} else {
				clif_displaymessage(fd, msg_txt(205)); // You already have this number of spiritballs.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}
	return true;
}

/*==========================================
 * #itemlist <character>: Displays the list of a player's items.
 *------------------------------------------
 */
bool charcommand_itemlist(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	size_t i, j, equip, count, counter, counter2;
	char character[100], output[200], equipstr[100], outputtmp[200];

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(equipstr, '\0', sizeof(equipstr));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can look items only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_INVENTORY; ++i) {
				if (pl_sd->status.inventory[i].nameid > 0 && (item_data = itemdb_exists(pl_sd->status.inventory[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.inventory[i].amount;
					count++;
					if (count == 1) {
						snprintf(output, sizeof(output), "------ Items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, output);
					}
					if ((equip = pl_sd->status.inventory[i].equip)) {
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
					} else
						memset(equipstr, '\0', sizeof(equipstr));
					if (sd.status.inventory[i].refine)
						snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, pl_sd->status.inventory[i].refine, item_data->jname, pl_sd->status.inventory[i].refine, pl_sd->status.inventory[i].nameid, equipstr);
					else
						snprintf(output, sizeof(output), "%d %s (%s, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, item_data->jname, pl_sd->status.inventory[i].nameid, equipstr);
					clif_displaymessage(fd, output);
					memset(output, '\0', sizeof(output));
					counter2 = 0;
					for (j = 0; j < item_data->flag.slot; ++j) {
						if (pl_sd->status.inventory[i].card[j]) {
							if ((item_temp = itemdb_exists(pl_sd->status.inventory[i].card[j])) != NULL) {
								if (output[0] == '\0')
									snprintf(outputtmp, sizeof(outputtmp), " -> (card(s): #%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
								else
									snprintf(outputtmp, sizeof(outputtmp), "#%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
								strcat(output, outputtmp);
							}
						}
					}
					if (output[0] != '\0') {
						output[strlen(output) - 2] = ')';
						output[strlen(output) - 1] = '\0';
						clif_displaymessage(fd, output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found on this player.");
			else {
				snprintf(output, sizeof(output), "%ld item(s) found in %ld kind(s) of items.", (unsigned long)(counter), (unsigned long)(count));
				clif_displaymessage(fd, output);
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * #effect by [MouseJstr]
 *
 * Create a effect localized on another character
 *------------------------------------------
 */
bool charcommand_effect(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd = NULL;
	char target[255];
	int type = 0;

	if (!message || !*message || sscanf(message, "%d %s", &type, target) != 2) {
		clif_displaymessage(fd, "usage: #effect <type+> <target>.");
		return false;
	}

	if((pl_sd=map_nick2sd((char *) target)) == NULL)
		return false;

	clif_specialeffect(*pl_sd, type, 0);
	clif_displaymessage(fd, msg_txt(229)); // Your effect has changed.

	return true;
}

/*==========================================
 * #storagelist <character>: Displays the items list of a player's storage.
 *------------------------------------------
 */
bool charcommand_storagelist(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct pc_storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	size_t i, j, count, counter, counter2;
	char character[128], output[256], outputtmp[256];

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can look items only lower or same level
			if((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
				counter = 0;
				count = 0;
				for (i = 0; i < MAX_STORAGE; ++i) {
					if (stor->storage[i].nameid > 0 && (item_data = itemdb_exists(stor->storage[i].nameid)) != NULL) {
						counter = counter + stor->storage[i].amount;
						count++;
						if (count == 1) {
							snprintf(output, sizeof(output), "------ Storage items list of '%s' ------", pl_sd->status.name);
							clif_displaymessage(fd, output);
						}
						if (stor->storage[i].refine)
							snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d)", stor->storage[i].amount, item_data->name, stor->storage[i].refine, item_data->jname, stor->storage[i].refine, stor->storage[i].nameid);
						else
							snprintf(output, sizeof(output), "%d %s (%s, id: %d)", stor->storage[i].amount, item_data->name, item_data->jname, stor->storage[i].nameid);
						clif_displaymessage(fd, output);
						memset(output, '\0', sizeof(output));
						counter2 = 0;
						for (j = 0; j < item_data->flag.slot; ++j) {
							if (stor->storage[i].card[j]) {
								if ((item_temp = itemdb_exists(stor->storage[i].card[j])) != NULL) {
									if (output[0] == '\0')
										snprintf(outputtmp, sizeof(outputtmp), " -> (card(s): #%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
									else
										snprintf(outputtmp, sizeof(outputtmp), "#%ld %s (%s), ", (unsigned long)(++counter2), item_temp->name, item_temp->jname);
									strcat(output, outputtmp);
								}
							}
						}
						if (output[0] != '\0') {
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
			} else {
				clif_displaymessage(fd, "This player has no storage.");
				return true;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

static void 
charcommand_giveitem_sub(struct map_session_data &sd,struct item_data &item_data, size_t number)
{
	int flag = 0;
	int loop = 1, get_count = number,i;
	struct item item_tmp;

	if (item_data.type == 4 || item_data.type == 5 ||
		item_data.type == 7 || item_data.type == 8) {
			loop = number;
			get_count = 1;
		}
		for (i = 0; i < loop; ++i) {
			memset(&item_tmp, 0, sizeof(item_tmp));
		item_tmp.nameid = item_data.nameid;
			item_tmp.identify = 1;
		if( (flag = pc_additem(sd, item_tmp, get_count)) )
			clif_additem(sd, 0, 0, flag);
		}

	}
/*==========================================
 * #item command (usage: #item <name/id_of_item> <quantity> <player>)
 * by MC Cameri
 *------------------------------------------
 */
bool charcommand_item(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char item_name[100];
	char character[100];
	struct map_session_data *pl_sd;
	struct item item_tmp;
	struct item_data *item_data;
	int flag;
	uint32 item_id, pet_id;
	size_t i, get_count;
	int number = 0;

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d %99[^\n]", item_name, &number, character) < 3) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: #item <item name or ID> <quantity> <char name>).");
		return false;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		get_count = number;
		// check pet egg
		pet_id = search_petDB_index(item_id, PET_EGG);
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			get_count = 1;
		}
		if ((pl_sd = map_nick2sd(character)) != NULL) {
			if (sd.isGM() >= pl_sd->isGM()) { // you can look items only lower or same level
				for (i = 0; i < (size_t)number; i += get_count) {
					// if pet egg
					if (pet_id >= 0) {
						sd.catch_target_class = pet_db[pet_id].class_;
						intif_create_pet(
							sd.status.account_id, sd.status.char_id,           
							pet_db[pet_id].class_, mob_db[pet_db[pet_id].class_].lv,             
							pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,100, 
							0, 1, 
							pet_db[pet_id].jname);
					// if not pet egg
					} else {
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = item_id;
						item_tmp.identify = 1;
						if ((flag = pc_additem(*pl_sd, item_tmp, get_count)))
							clif_additem(*pl_sd, 0, 0, flag);
					}
				}
				clif_displaymessage(fd, msg_txt(18)); // Item created.
			} else {
				clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
				return false;
			}
		} else if(/* from jA's @giveitem */strcasecmp(character,"all")==0 || strcasecmp(character,"everyone")==0){
			char buf[256];
			for (i = 0; i < fd_max; ++i) {
				if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session)){
					charcommand_giveitem_sub( *pl_sd, *item_data, number);
					snprintf(buf, sizeof(buf), "You got %s %d.", item_name, number);
					clif_displaymessage(pl_sd->fd, buf);
				}
			}
			snprintf(buf, sizeof(buf), "%s received %s %d.","Everyone",item_name,number);
			clif_displaymessage(fd, buf);
		} else {
			clif_displaymessage(fd, msg_txt(3)); // Character not found.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return false;
	}

	return true;
}

/*==========================================
 * #warp/#rura/#rura+ <mapname> <x> <y> <char name>
 *------------------------------------------
 */
bool charcommand_warp(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char mapname[128], *ip;
	char character[128];
	int x = 0, y = 0;
	struct map_session_data *pl_sd;
	int m;

	memset(mapname, '\0', sizeof(mapname));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99s %d %d %99[^\n]", mapname, &x, &y, character) < 4) {
		clif_displaymessage(fd, "Usage: #warp/#rura/#rura+ <mapname> <x> <y> <char name>");
		return false;
	}

	if (x <= 0)
		x = rand() % 399 + 1;
	if (y <= 0)
		y = rand() % 399 + 1;
	ip = strchr(mapname, '.');
	if(ip) *ip=0;

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can rura+ only lower or same GM level
			if (x > 0 && x < 400 && y > 0 && y < 400) {
				m = map_mapname2mapid(mapname);
				if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
					clif_displaymessage(fd, "You are not authorised to warp someone to this map.");
					return false;
				}
				if(pl_sd->block_list::m <map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
					clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
					return false;
				}
				if(pc_setpos(*pl_sd, mapname, x, y, 3) == 0) {
					clif_displaymessage(pl_sd->fd, msg_txt(0)); // Warped.
					clif_displaymessage(fd, msg_txt(15)); // Player warped (message sends to player too).
				}
			else
{
					clif_displaymessage(fd, msg_txt(1)); // Map not found.
					return false;
	}
	}
			else
{
				clif_displaymessage(fd, msg_txt(2)); // Coordinates out of range.
				return false;
	}
				}
		else
{
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
	}
				}
	else
{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
				}

/*==========================================
 * #zeny <charname>
 *------------------------------------------
 */
bool charcommand_zeny(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char character[128]="";
	long zeny;
	unsigned long new_zeny;

	if (!message || !*message || sscanf(message, "%ld %99[^\n]", &zeny, character) < 2 || zeny == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: #zeny <zeny> <name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL)
	{
		new_zeny = sd.status.zeny + zeny;
		if( zeny>0 && (new_zeny<sd.status.zeny || new_zeny>MAX_ZENY) ) // pos overflow & max
			new_zeny = MAX_ZENY;
		else if( zeny<0 && new_zeny>sd.status.zeny ) // neg overflow
			new_zeny = 0;

		if (new_zeny != pl_sd->status.zeny)
		{
			pl_sd->status.zeny = new_zeny;
			clif_updatestatus(*pl_sd, SP_ZENY);
			clif_displaymessage(fd, msg_txt(211)); // Character's number of zenys changed!
		}
		else
		{
			if (zeny < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}
	return true;
}

bool charcommand_showexp(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	if( sd.state.noexp )
	{
		sd.state.noexp = 0;
		clif_displaymessage(fd, "Gained exp is now shown");
	}
	else
	{
		sd.state.noexp = 1;
		clif_displaymessage(fd, "Gained exp is now NOT shown");
		return true;
	}
	return true;
}

bool charcommand_showdelay(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	if( sd.state.nodelay )
	{
		sd.state.nodelay = 0;
		clif_displaymessage(fd, "Skill delay failure is now shown");
	}
	else
	{
		sd.state.nodelay = 1;
		clif_displaymessage(fd, "Skill delay failure is NOT now shown");
	}
	return true;
}

/*==========================================
 * #fakename <fake name> <char name>
 *------------------------------------------
 */

bool charcommand_fakename(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char name[64];
	char char_name[64];
	
	if (!message || !*message) {
		clif_displaymessage(sd.fd,"Usage: #fakename <fake name> <char name>.");
		clif_displaymessage(sd.fd,"Or: #fakename <char name> to disable.");
		return 0;
	}
	
	if (sscanf(message, "%23s %23[^\n]", name, char_name) < 1) {
		return 0;
	}
	
	if( strlen(char_name) < 1 )
	{
		if(!(pl_sd = map_nick2sd(name))) {
			clif_displaymessage(sd.fd,"Character not found.");
			return false;
		}
		else
		{
			if(strlen(pl_sd->fakename) > 1) {
				pl_sd->fakename[0]='\0';
				pc_setpos(*pl_sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
				clif_displaymessage(sd.fd,"Returned to real name.");
			} else {
				clif_displaymessage(sd.fd,"Usage: #fakename <fake name> <char name>.");
			}
		}
		return true;
	}

	if(!(pl_sd = map_nick2sd(char_name))) {
		clif_displaymessage(sd.fd,"Character not found.");
		return false;
	}
	
	if(strlen(name) < 2) {
		clif_displaymessage(sd.fd,"Fake name must be at least two characters.");
		return false;
	}
	
	safestrcpy(pl_sd->fakename,name, 24);
	pc_setpos(*pl_sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
	clif_displaymessage(sd.fd,"Fake name enabled.");
	
	return true;
}

/*==========================================
 * #baselvl <#> <nickname> 
 *------------------------------------------
*/
bool charcommand_baselevel(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	int level = 0, i;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: #baselvl <#> <nickname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can change base level only lower or same gm level

			if (level > 0) {
				if (pl_sd->status.base_level == config.max_base_level) {	// check for max level by Valaris
					clif_displaymessage(fd, msg_txt(91)); // Character's base level can't go any higher.
					return 0;
				}	// End Addition
				if ((unsigned int)level > config.max_base_level || (unsigned int)level > (config.max_base_level - pl_sd->status.base_level)) // fix positiv overflow
					level = config.max_base_level - pl_sd->status.base_level;
				for (i = 1; i <= level; ++i)
					pl_sd->status.status_point += (pl_sd->status.base_level + i + 14) / 5;
				pl_sd->status.base_level += level;
				clif_updatestatus(*pl_sd, SP_BASELEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTBASEEXP);
				clif_updatestatus(*pl_sd, SP_STATUSPOINT);
				status_calc_pc(*pl_sd, 0);
				pc_heal(*pl_sd, pl_sd->status.max_hp, pl_sd->status.max_sp);
				clif_misceffect(*pl_sd, 0);
				clif_displaymessage(fd, msg_txt(65)); // Character's base level raised.
			} else {
				if (pl_sd->status.base_level == 1) {
					clif_displaymessage(fd, msg_txt(193)); // Character's base level can't go any lower.
					return false;
				}
				if (level < -(int)config.max_base_level || level < (1 - (int)pl_sd->status.base_level)) // fix negativ overflow
					level = 1 - pl_sd->status.base_level;
				if (pl_sd->status.status_point > 0) {
					for (i = 0; i > level; i--)
					{
						if( pl_sd->status.status_point > (pl_sd->status.base_level + i + 14) / 5 )
							pl_sd->status.status_point -= (pl_sd->status.base_level + i + 14) / 5;
						else
							pl_sd->status.status_point = 0;
					}
					clif_updatestatus(*pl_sd, SP_STATUSPOINT);
				} // to add: remove status points from stats
				pl_sd->status.base_level += level;
				clif_updatestatus(*pl_sd, SP_BASELEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTBASEEXP);
				status_calc_pc(*pl_sd, 0);
				clif_displaymessage(fd, msg_txt(66)); // Character's base level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true; //?^(3)?i'?I―^(1)
}

/*==========================================
 * #jlvl <#> <nickname> 
 *------------------------------------------
 */
bool charcommand_joblevel(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	unsigned int max_level = config.max_job_level;
	char player[64];
	int level = 0;
	//“]?¶,a^―{Z(q,I`?e^?‡,I`OE^(3),I`?E<?,?Z(Z?o,?,e'
	struct pc_base_job pl_s_class;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: #joblvl <#> <nickname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		pl_s_class = pc_calc_base_job(pl_sd->status.class_);
		if (sd.isGM() >= pl_sd->isGM()) { // you can change job level only lower or same gm level
			if (pl_s_class.job == 0)
				max_level = 10; //Novice
			// super novices can go up to 99 [celest]
			else if (pl_s_class.job == 23)
				max_level = config.max_sn_level; //S. Novice
			else if (pl_sd->status.class_ > 4007 && pl_sd->status.class_ < 4023)
				max_level = config.max_adv_level; //Adv. Class

			if (level > 0) {
				if (pl_sd->status.job_level == max_level) {
					clif_displaymessage(fd, msg_txt(67)); // Character's job level can't go any higher.
					return false;
				}
				if((int)pl_sd->status.job_level + level > (int)max_level)
					level = max_level - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(*pl_sd, SP_JOBLEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTJOBEXP);
				pl_sd->status.skill_point += level;
				clif_updatestatus(*pl_sd, SP_SKILLPOINT);
				status_calc_pc(*pl_sd, 0);
				clif_misceffect(*pl_sd, 1);
				clif_displaymessage(fd, msg_txt(68)); // character's job level raised.
			}
			else //level<0
			{
				if (pl_sd->status.job_level == 1) {
					clif_displaymessage(fd, msg_txt(194)); // Character's job level can't go any lower.
					return false;
				}
				if((int)pl_sd->status.job_level + level < 1)
					level = 1 - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(*pl_sd, SP_JOBLEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTJOBEXP);
				if (pl_sd->status.skill_point > 0)
				{
					
					if (pl_sd->status.skill_point > - level)
						pl_sd->status.skill_point += level;
					else
						pl_sd->status.skill_point = 0;
					clif_updatestatus(*pl_sd, SP_SKILLPOINT);
				} // to add: remove status points from skills
				status_calc_pc(*pl_sd, 0);
				clif_displaymessage(fd, msg_txt(69)); // Character's job level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}


/*==========================================
 * #questskill <skill_#> <nickname>
 *------------------------------------------
 */
bool charcommand_questskill(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	int skill_id = 0;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: #questskill <#:0+> <nickname>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & INF2_QUEST_SKILL) {
			if ((pl_sd = map_nick2sd(player)) != NULL) {
				if (pc_checkskill(*pl_sd, skill_id) == 0) {
					pc_skill(*pl_sd, skill_id, 1, 0);
					clif_displaymessage(fd, msg_txt(199)); // This player has learned the skill.
				} else {
					clif_displaymessage(fd, msg_txt(200)); // This player already has this quest skill.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_txt(3)); // Character not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return false;
	}

	return true;
}


/*==========================================
 * #lostskill <skill_#> <nickname>
 *------------------------------------------
 */
bool charcommand_lostskill(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	int skill_id = 0;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charlostskill <#:0+> <char_name>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & INF2_QUEST_SKILL) {
			if ((pl_sd = map_nick2sd(player)) != NULL) {
				if (pc_checkskill(*pl_sd, skill_id) > 0) {
					pl_sd->status.skill[skill_id].lv = 0;
					pl_sd->status.skill[skill_id].flag = 0;
					clif_skillinfoblock(*pl_sd);
					clif_displaymessage(fd, msg_txt(202)); // This player has forgotten the skill.
				} else {
					clif_displaymessage(fd, msg_txt(203)); // This player doesn't have this quest skill.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_txt(3)); // Character not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return false;
	}

	return true;
}

/*==========================================
 * Character Skill Reset
 *------------------------------------------
 */
bool charcommand_skreset(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	char output[128];

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charskreset <charname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can reset skill points only lower or same gm level
			pc_resetskill(*pl_sd);
			snprintf(output, sizeof(output), msg_txt(206), player); // '%s' skill points reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * Character Stat Reset
 *------------------------------------------
 */
bool charcommand_streset(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	char output[128];

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charstreset <charname>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can reset stats points only lower or same gm level
			pc_resetstate(*pl_sd);
			snprintf(output, sizeof(output), msg_txt(207), player); // '%s' stats points reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * Character Model by chbrules
 *------------------------------------------
 */
bool charcommand_model(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	unsigned long hair_style = 0, hair_color = 0, cloth_color = 0;
	struct map_session_data *pl_sd;
	char player[64];
	char output[128];

	if (!message || !*message || sscanf(message, "%ld %ld %ld %23[^\n]", &hair_style, &hair_color, &cloth_color, player) < 4 || hair_style < 0 || hair_color < 0 || cloth_color < 0) {
		snprintf(output, sizeof(output), "Please, enter a valid model and a player name (usage: @charmodel <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d> <name>).",
				MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if(hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
			hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
			cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {

			if (cloth_color != 0 &&
				pl_sd->status.sex == 1 &&
				(pl_sd->status.class_ == 12 ||  pl_sd->status.class_ == 17)) {
				clif_displaymessage(fd, msg_txt(35)); // You can't use this command with this class.
				return false;
			} else {
				pc_changelook(*pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(*pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(*pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
			}
		} else {
			clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * Character Skill Point (Rewritten by [Yor])
 *------------------------------------------
 */
bool charcommand_skpoint(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	int new_skill_point;
	int point = 0;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &point, player) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charskpoint <amount> <name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		new_skill_point = (int)pl_sd->status.skill_point + point;
		if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
			new_skill_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
			new_skill_point = 0;
		if (new_skill_point != (int)pl_sd->status.skill_point) {
			pl_sd->status.skill_point = new_skill_point;
			clif_updatestatus(*pl_sd, SP_SKILLPOINT);
			clif_displaymessage(fd, msg_txt(209)); // Character's number of skill points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * Character Status Point (rewritten by [Yor])
 *------------------------------------------
 */
bool charcommand_stpoint(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	struct map_session_data *pl_sd;
	char player[64];
	int new_status_point;
	int point = 0;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &point, player) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charstpoint <amount> <name>).");
		return false;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		new_status_point = (int)pl_sd->status.status_point + point;
		if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
			new_status_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
			new_status_point = 0;
		if (new_status_point != (int)pl_sd->status.status_point) {
			pl_sd->status.status_point = new_status_point;
			clif_updatestatus(*pl_sd, SP_STATUSPOINT);
			clif_displaymessage(fd, msg_txt(210)); // Character's number of status points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return false;
	}

	return true;
}

/*==========================================
 * charchangesex command (usage: charchangesex <player_name>)
 *------------------------------------------
 */
bool charcommand_changesex(int fd, struct map_session_data &sd,const char *command, const char *message)
{
	char player[64];

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charchangesex <name>).");
		return false;
	}

	// check player name
	if (strlen(player) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if (strlen(player) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		chrif_char_ask_name(sd.status.account_id, player, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return true;
}
